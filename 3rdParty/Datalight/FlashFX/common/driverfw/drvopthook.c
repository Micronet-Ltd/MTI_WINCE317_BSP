/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module contains logic to allow the "OptionGet" function hierarchy
    to be hooked into at run time.

    ToDo:
    - Fix this interface to supply a context to go along with the hooked
      option function, to facilitate eliminating the need for any global
      data.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvopthook.c $
    Revision 1.5  2009/12/09 19:51:30Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/07/18 00:53:31Z  garyp
    Eliminated use of an obsolete header.
    Revision 1.3  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.2  2009/03/31 19:20:01Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.1  2008/03/27 16:07:38Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverOptionHook()

    This function contains logic to allow the "OptionGet" function
    hierarchy to be hooked into at run time.  Do not confuse this
    use of the term "hook" with the project hooks -- they are
    distinctly different.

    The hierarchy is structured as follows:

    FfxHookOptionGet()           // function called by requestor
     ->FfxDriverOptionGet()      // optionally call function hooks
        ->FfxDriverFWOptionGet() // default Driver Framework handler
           ->FfxOsOptionGet()    // default OS driver handler
              ->FfxOptionGet()   // default catch-call handler

    Notes:
    1) The project level hook code (fhoption.c) always gets first
       crack at satisfying the request (for now).
    2) Any hook function put in place by this code <MUST> pass all
       unhandled requests to the original hook function.  This will
       usually be FfxDriverFWOptionGet(), but could be something
       else if more than one client has hooked things.
    3) In the event that multiple clients have hooked this interface,
       the last one installed will be called first.
    4) In the event that multiple clients have hooked this interface,
       they must be unhooked in reverse order.

    Parameters:
        pfnOptionGet  - A function pointer to the hook function
                        to use.

    Return Value:
        If successful, returns a pointer to the previous hook
        function, or NULL otherwise.
-------------------------------------------------------------------*/
FFXPFNOPTIONGET FfxDriverOptionHook(
    FFXPFNOPTIONGET     pfnOptionGet)
{
    FFXDRIVERINFO      *pFDI;
    FFXPFNOPTIONGET     pfnOldFunc = NULL;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, 0),
        "FfxDriverOptionHook() NewFunc=%P\n", pfnOptionGet));

    DclAssert(pfnOptionGet);

    pFDI = FfxDriverHandle(1);
    if(!pFDI)
    {
        DclError();
        return NULL;
    }

    /*  Save the current function pointer value
    */
    pfnOldFunc = pFDI->pfnOptionGet;

    if(pfnOptionGet == FfxDriverFWOptionGet)
    {
        /*  If the new function is a pointer to the normal handler in the
            chain, set the hook pointer to NULL, since we are effectively
            removing whatever the old handler was.
        */
        pFDI->pfnOptionGet = NULL;
    }
    else
    {
        /*  Otherwise, just stuff in the new function pointer...
        */
        pFDI->pfnOptionGet = pfnOptionGet;
    }

    /*  If the old function pointer was NULL, then we know that the original
        handler was in place -- return a pointer to it, rather than returning
        NULL, which would indicate an error.
    */
    if(!pfnOldFunc)
        pfnOldFunc = FfxDriverFWOptionGet;

    return pfnOldFunc;
}


/*-------------------------------------------------------------------
    Protected: FfxDriverOptionGet()

    Get a configuration option.  This function is normally only
    called from FfxHookOptionGet() to obtain an option that is
    standard for all projects using a given OS.

    This function will call any installed options hook functions
    which may be in place.  If none have been installed then the
    function FfxDriverFWOptionGet() will be called.

    Parameters:
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return Value:
        TRUE if the option identifier is valid, the option value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverOptionGet(
    FFXOPTION       opt,
    void           *handle,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    D_BOOL          fResult = FALSE;
    FFXDRIVERINFO  *pFDI;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDriverOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    pFDI = FfxDriverHandle(1);
    if(pFDI && pFDI->pfnOptionGet)
    {
        /*  The interface has been hooked, so call the last hook
            installed.  It is the responsibility of the hook
            function to call the original handlers.
        */
        fResult = (*pFDI->pfnOptionGet)(opt, handle, pBuffer, ulBuffLen);
    }
    else
    {
        /*  The interface has not been hooked, so simply call
            the standard handler.
        */
        fResult = FfxDriverFWOptionGet(opt, handle, pBuffer, ulBuffLen);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDriverOptionGet() returning %U\n", fResult));

    return fResult;
}




