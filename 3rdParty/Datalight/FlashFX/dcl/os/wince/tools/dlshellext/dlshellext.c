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
  jurisdictions.  Patents may be pending.

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

    This module implements a Windows CE Target Control extension DLL which
    allows the DCL Shell to be run from within the Target Control window.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshellext.c $
    Revision 1.5  2009/06/27 23:10:40Z  garyp
    Updated to redirect I/O using services.
    Revision 1.4  2009/02/08 17:29:36Z  garyp
    Minor cleanup.
    Revision 1.3  2009/02/08 01:45:30Z  garyp
    Merged from the v4.0 branch.  Updated to use the new DclInputFunction()
    to remap how input is supplied, and avoid using a global.
    Revision 1.2  2008/01/18 21:07:51Z  Garyp
    Updated to handle long output requests.
    Revision 1.1  2007/12/14 17:49:34Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dltools.h>
#include <dlcetools.h>
#include <dlservice.h>

/*  Bad form, but these are also hard-coded in the MS sample code...
*/
typedef int     (*PFNPRINTF)(TCHAR *ptzFmt, ...);
typedef TCHAR * (*PFNINPUT) (TCHAR *pBuffer, int iBuffLen);

static DCLSTATUS ShellExtInput(void *pContext, D_UCHAR *puChar, unsigned nFlags);
static void      ShellExtOutput(void *pContext, const char *pszStr);


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
BOOL __stdcall DLLEntry(
    HANDLE  hInstance,
    DWORD   dwOp,
    LPVOID  pReserved)
{
    switch(dwOp)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls((HMODULE)hInstance);
            break;

        case DLL_PROCESS_DETACH:
            break;

        default:
            break;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
BOOL ParseCommand(
    const LPTSTR    ptzCmd,
    const LPTSTR    ptzCmdLine,
    PFNPRINTF       pfnPrintf,
    PFNINPUT        pfnInput)
{
  #if DCLCONF_COMMAND_SHELL

    if (!_tcsicmp(ptzCmd, TEXT("?")))
    {
        pfnPrintf(TEXT("    DLShell  - Run the Datalight Command Shell\r\n"));
        return TRUE;
    }
    else if (!_tcsicmp(ptzCmd, TEXT("dlshell")))
    {
        #define             BUFFLEN     (256)
        DCLINPUTINFO        ii = {pfnInput, ShellExtInput};
        DCLOUTPUTINFO       oi = {pfnPrintf, ShellExtOutput};
        DCLDECLARESERVICE   (SrvIn, "DLCESHINPUT", DCLSERVICE_INPUT, NULL, &ii, 0);
        DCLDECLARESERVICE   (SrvOut, "DLCESHOUTPUT", DCLSERVICE_OUTPUT, NULL, &oi, 0);
        char                szCmdLine[BUFFLEN];
        DCLSTATUS           dclStat;
        DCLINSTANCEHANDLE   hDclInst;
        DCLTOOLPARAMS       tp = {0};

        dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
        if(dclStat != DCLSTAT_SUCCESS)
            return TRUE;

        tp.hDclInst     = hDclInst;
        tp.pszCmdName   = "DLSHELL";
        tp.pszCmdLine   = szCmdLine;

        /*  Register the output service to use
        */
        if(DclServiceCreate(hDclInst, &SrvOut) == DCLSTAT_SUCCESS)
        {
            /*  Register the input service to use
            */
            if(DclServiceCreate(hDclInst, &SrvIn) == DCLSTAT_SUCCESS)
            {
                if(ptzCmdLine)
                {
                    /*  Convert the string from Unicode to ANSI
                    */
                    WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, sizeof(szCmdLine), NULL, NULL);
                }
                else
                {
                    szCmdLine[0] = 0;
                }

                DclCeShell(&tp);

                /*  Remove the input service
                */
                DclServiceDestroy(&SrvIn);
            }

            /*  Remove the output service
            */
            DclServiceDestroy(&SrvOut);
        }

        DclInstanceDestroy(hDclInst);

        return TRUE;
    }
    else
    {
        return FALSE;
    }

  #else

    (void)ptzCmd;
    (void)ptzCmdLine;
    (void)pfnPrintf;
    (void)pfnInput;

    return FALSE;

  #endif

}


#if DCLCONF_COMMAND_SHELL

/*-------------------------------------------------------------------
    Local: ShellExtInput()

    This is the workhorse function for obtaining character input.

    When this function is used by common, OS independent code, it
    <must> be assumed that the OS interface may wait for a character,
    or even the [Enter] key before returning from this function.

    When this function is used from OS specific code, where the OS
    characteristics are known, it may return immediately with a
    DCLSTAT_INPUT_IDLE status.

    Parameters:
        pContext - A pointer to any context data which may have been
                   specified when DclInputFunction() was called.
        puChar   - A pointer to a buffer to receive the character.
                   The contents of this field will only be modified
                   if DCLSTAT_SUCCESS is returned.
        nFlags   - This parameter is reserved for future use and must
                   be 0.

    Return Value:
        Returns one of the following DCLSTATUS values:

          DCLSTAT_SUCCESS         - A character was returned
          DCLSTAT_INPUT_DISABLED  - The input interface is disabled
          DCLSTAT_INPUT_IDLE      - There is no input ready
          DCLSTAT_INPUT_TERMINATE - The OS is terminating the program
-------------------------------------------------------------------*/
static DCLSTATUS ShellExtInput(
    void           *pContext,
    D_UCHAR        *puChar,
    unsigned        nFlags)
{
    PFNINPUT        pfnInput = (PFNINPUT)pContext;
    TCHAR           awcBuffer[DCLOUTPUT_BUFFERLEN];
    static char     achBuffer[DCLOUTPUT_BUFFERLEN];
    static unsigned nRemaining = 0;
    static unsigned nIndex = 0;

    (void)pContext;

    DclAssert(puChar);
    DclAssert(nFlags == 0);

    if(!nRemaining)
    {
        pfnInput(awcBuffer, sizeof(awcBuffer));

        /*  Convert the string from Unicode to ANSI
        */
        WideCharToMultiByte(CP_ACP, 0, awcBuffer, -1, achBuffer, sizeof(achBuffer)-1, NULL, NULL);

        nRemaining = strlen(achBuffer);
        nIndex = 0;

        if(achBuffer[nIndex] == '\n')
            nIndex++;

        achBuffer[nRemaining] = '\n';
    }

    if(nRemaining)
    {
        nRemaining--;
        nIndex++;

        *puChar = achBuffer[nIndex-1];

        return DCLSTAT_SUCCESS;
    }
    else
    {
        return DCLSTAT_INPUT_DISABLED;
    }
}


/*-------------------------------------------------------------------
    This is a run-time replacement for DclOsOutputString().
-------------------------------------------------------------------*/
static void ShellExtOutput(
    void       *pContext,
    const char *pszStr)
{
    PFNPRINTF   pfnPrintf = (PFNPRINTF)pContext;

    if(pfnPrintf)
    {
        TCHAR       awcBuffer[DCLOUTPUT_BUFFERLEN];
        unsigned    nLen = 0;
        unsigned    nIndex = 0;

        while(pszStr[nIndex])
        {
            nLen = DCLMIN(DclStrLen(pszStr+nIndex), DCLOUTPUT_BUFFERLEN-1);

            MultiByteToWideChar(CP_ACP, 0, pszStr+nIndex, nLen, awcBuffer, sizeof(awcBuffer));

            awcBuffer[nLen] = 0;

            pfnPrintf(TEXT("%s"), awcBuffer);

            nIndex += nLen;
            nLen = 0;
        }
    }

    return;
}


#endif  /* DCLCONF_COMMAND_SHELL */




