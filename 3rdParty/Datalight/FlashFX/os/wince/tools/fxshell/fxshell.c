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

    This module implements the FlashFX Shell for WinCE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshell.c $
    Revision 1.7  2009/10/09 02:34:55Z  garyp
    Removed the logic conditional on DCLCONF_COMMAND_SHELL, which
    is now handled at the lower level.
    Revision 1.6  2009/10/01 18:20:29Z  garyp
    Eliminated the use of ffxapi.h.
    Revision 1.5  2009/07/19 04:47:47Z  garyp
    Merged from the v4.0 branch.  Updated to use the DCL instance handle
    returned by FlashFX_Open().
    Revision 1.4  2009/02/09 07:13:13Z  garyp
    Merged from the v4.0 branch.  Updated to use FlashFX_Open/Close() at
    the outermost layer.  Updated to properly use flashfx_wince.h has the top
    level header.
    Revision 1.3  2007/11/28 23:12:07Z  Garyp
    Updated to use the new DCL functions to support output redirection.
    Revision 1.2  2007/11/03 23:50:14Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/03/16 01:58:30Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>

#include <flashfx_wince.h>
#include <oecommon.h>
#include <fxtools.h>
#include <dlceutil.h>
#include <dlerrlev.h>

#define PROGRAM_NAME    "FXSHELL"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    int                     iReturn = 0;
    DCLSTATUS               dclStat;
    DCLINSTANCEHANDLE       hDclInst = NULL;
    FFXREQHANDLE            hReq;

    /*  Open a requestor handle to the FlashFX driver.  This also
        validates that FlashFX driver version matches that of this
        utility, as well as creates a DCL Instance.
    */
    dclStat = FlashFX_Open(UINT_MAX, &hDclInst, &hReq);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        #define                 BUFFLEN (256)
        char                    achBuffer[BUFFLEN];
        FFXTOOLPARAMS           tp = {0};
        DCLREDIRECTIONHANDLE    hRedirect;

        tp.dtp.hDclInst   = hDclInst;
        tp.dtp.pszCmdName = PROGRAM_NAME;
        tp.dtp.pszCmdLine = achBuffer;

        /*  Convert the string from Unicode to ANSI
        */
        WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, achBuffer, BUFFLEN, NULL, NULL);

        hRedirect = DclCeOutputRedirectionBegin(hDclInst, achBuffer);

        iReturn = FfxShell(&tp);

        if(hRedirect)
            DclCeOutputRedirectionEnd(hRedirect);

        FlashFX_Close(hReq);
    }
    else
    {
        printf(PROGRAM_NAME": Unable to open a requestor handle to FlashFX, Status=0x%08lX\n", dclStat);
    }

    if(!iReturn)
        iReturn = DclStatusToErrorlevel(dclStat);

    return iReturn;
}



