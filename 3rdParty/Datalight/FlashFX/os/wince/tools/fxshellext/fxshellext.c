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

    This module implements a Windows CE Target Control extension DLL which
    allows the FlashFX Shell to be run from within the Target Control
    window.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshellext.c $
    Revision 1.9  2009/10/01 18:20:30Z  garyp
    Eliminated the use of ffxapi.h.
    Revision 1.8  2009/07/29 17:17:05Z  garyp
    Merged from the v4.0 branch.  Updated to use the new service initialization 
    process.
    Revision 1.7  2009/04/17 02:24:28Z  michaelm
    Removed commas from between TEXT() macros.
    Revision 1.6  2009/04/16 16:13:49Z  michaelm
    Broke up printf in ParseCommand() to address CE build error.
    Revision 1.5  2009/04/14 20:57:06Z  thomd
    Updated to use D_PRODUCTNAME macro
    Revision 1.4  2009/02/09 07:42:15Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FlashFX_Open() 
    function.  Use the newly abstracted interface for replacing the input
    function -- no more globals.
    Revision 1.3  2008/01/26 02:23:49Z  Garyp
    Updated the output function to properly handle long strings.
    Revision 1.2  2008/01/25 07:22:53Z  keithg
    Comment style updates to support autodoc.
    Revision 1.1  2007/12/14 01:19:58Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx.h>            /* Necessary due to use of internal FlashFX stuff */
#include <flashfx_wince.h>
#include <oecommon.h>
#include <fxtools.h>
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

        default :
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
        pfnPrintf(TEXT("    FlashFX  - Run the Datalight ") TEXT(D_PRODUCTNAME) TEXT(" Command Shell\r\n") );
        return TRUE;
    }
    else if (!_tcsicmp(ptzCmd, TEXT("flashfx")))
    {
        #define             BUFFLEN     (256)
        DCLINPUTINFO        ii = {pfnInput, ShellExtInput};
        DCLOUTPUTINFO       oi = {pfnPrintf, ShellExtOutput};
        DCLDECLARESERVICE   (SrvIn, "FXCESHINPUT", DCLSERVICE_INPUT, NULL, &ii, 0);
        DCLDECLARESERVICE   (SrvOut, "FXCESHOUTPUT", DCLSERVICE_OUTPUT, NULL, &oi, 0);
        char                szCmdLine[BUFFLEN];
        FFXTOOLPARAMS       tp = {0};
        DCLSTATUS           dclStat;

        dclStat = DclInstanceCreate(0, DCLFLAG_APP, &tp.dtp.hDclInst);
        if(dclStat != DCLSTAT_SUCCESS)
            return TRUE;

        /*  Register the output service to use
        */
        if(DclServiceCreate(tp.dtp.hDclInst, &SrvOut) == DCLSTAT_SUCCESS)
        {
            /*  Register the input service to use
            */
            if(DclServiceCreate(tp.dtp.hDclInst, &SrvIn) == DCLSTAT_SUCCESS)
            {
                FFXREQHANDLE    hReq;

                tp.dtp.pszCmdName = "FLASHFX";
                tp.dtp.pszCmdLine = &szCmdLine[0];
                tp.pszDriveForms = FfxGetDriveFormatString();

                if(ptzCmdLine)
                {
                    /*  Convert the string from Unicode to ANSI
                    */
                    WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, BUFFLEN, NULL, NULL);
                }
                else
                {
                    szCmdLine[0] = 0;
                }

                /*  Open a requestor handle to the FlashFX driver.  This also
                    validates that FlashFX driver version matches that of this
                    utility.
                */
                dclStat = FlashFX_Open(UINT_MAX, &tp.dtp.hDclInst, &hReq);
                if(dclStat == DCLSTAT_SUCCESS)
                {
                    FfxShell(&tp);

                    FlashFX_Close(hReq);
                }
                else
                {
                    DclPrintf("FXSHELLEXT: Unable to open a requestor handle, Status=%lX\n", dclStat);
                }

                /*  Remove the input service
                */
                DclServiceDestroy(&SrvIn);
            }

            /*  Remove the output service
            */
            DclServiceDestroy(&SrvOut);
        }

        DclInstanceDestroy(tp.dtp.hDclInst);

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

    Returns:
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



