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

    This module is an application framework for creating standalone
    executables for the given OS Layer.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmsltest.c $
    Revision 1.9  2009/10/01 18:20:31Z  garyp
    Eliminated the use of ffxapi.h.
    Revision 1.8  2009/07/19 04:44:36Z  garyp
    Merged from the v4.0 branch.  Updated to use the DCL instance handle
    returned by FlashFX_Open().
    Revision 1.7  2009/02/09 07:10:35Z  garyp
    Merged from the v4.0 branch.  Updated to use FlashFX_Open/Close() at
    the outermost layer.  Updated to properly use flashfx_wince.h has the top
    level header.
    Revision 1.6  2007/11/28 22:45:06Z  Garyp
    Updated to use the new DCL functions to support output redirection.
    Revision 1.5  2007/11/03 23:50:13Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2007/08/05 22:25:10Z  garyp
    Updated to return standard errorlevel values as defined in dlerrlev.h
    (where possible).
    Revision 1.3  2007/03/06 22:31:51Z  Garyp
    Modified to dismount the store before running the test.
    Revision 1.2  2006/02/12 20:07:23Z  Garyp
    Updated to new Devices and Disk configuration model.
    Revision 1.1  2005/10/03 13:30:20Z  Pauli
    Initial revision
    Revision 1.7  2005/06/12 04:40:30Z  PaulI
    Set DCL output function to redirect output to the extapi function.
    Revision 1.6  2004/12/30 23:08:27Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/07/20 00:30:43Z  GaryP
    Minor include file updates.
    Revision 1.4  2003/12/05 07:05:24Z  garys
    Merge from FlashFXMT
    Revision 1.3.1.4  2003/12/05 07:05:24  garyp
    Modified to use the new oemmount() function.
    Revision 1.3.1.3  2003/11/23 20:32:12Z  garyp
    Added some missing headers.
    Revision 1.3.1.2  2003/11/19 02:54:08Z  garyp
    Modified to work with FFX-MT and use the new external API.
    Revision 1.3  2003/05/08 18:43:42Z  garyp
    Minor output formatting changes.
    Revision 1.2  2003/04/15 17:34:10Z  garyp
    Updated to use the Tools Framework.
    Revision 1.1  2002/12/05 00:37:40Z  garyp
    Initial revision
    07/17/02 gp  Created
---------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <storemgr.h>

#include <flashfx_wince.h>
#include <fxtools.h>
#include <fxver.h>
#include <oecommon.h>
#include <fxfmlapi.h>
#include <dlerrlev.h>
#include <dlceutil.h>

#define PROGRAM_NAME    "FMSLTEST"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    #define                 BUFFLEN     (256)
    #define                 DEVNAMELEN   (16)
    char                    szCmdLine[BUFFLEN];
    char                    achBuffer[BUFFLEN];
    D_UINT16                uDiskNum = 0;
    D_UINT16                uArgCount;
    FFXTOOLPARAMS           tp = {0};
    TCHAR                   awcDevName[DEVNAMELEN];
    HANDLE                  hStore;
    DCLREDIRECTIONHANDLE    hRedirect;
    int                     iReturn = 0;
    DCLSTATUS               dclStat;
    DCLINSTANCEHANDLE       hDclInst = NULL;
    FFXREQHANDLE            hReq = 0;

    /*  Open a requestor handle to the FlashFX driver.  This also
        validates that FlashFX driver version matches that of this
        utility, as well as creates a DCL Instance.
    */
    dclStat = FlashFX_Open(UINT_MAX, &hDclInst, &hReq);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        printf(PROGRAM_NAME": Unable to open a requestor handle to FlashFX, Status=0x%08lX\n", dclStat);
        goto Cleanup;
    }

    tp.dtp.hDclInst     = hDclInst;
    tp.dtp.pszCmdName   = PROGRAM_NAME;
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = FfxGetDriveFormatString();

    /*  Convert the string from Unicode to ANSI
    */
    WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, BUFFLEN, NULL, NULL);

    hRedirect = DclCeOutputRedirectionBegin(hDclInst, szCmdLine);

    uArgCount = DclArgCount(szCmdLine);

    if(!uArgCount)
        goto DisplayHelp;

    if(DclArgRetrieve(szCmdLine, 1, sizeof(achBuffer), achBuffer))
    {
        if(!strcmp(achBuffer, "?") || !strcmp(achBuffer, "/?") ||
           !strcmp(achBuffer, "/h") || !strcmp(achBuffer, "/H"))
        {
            goto DisplayHelp;
        }

        /*  Retrieve the drive number
        */
        if(!FfxParseDriveNumber(achBuffer, &uDiskNum, 0))
        {
            DclPrintf(PROGRAM_NAME": Unable to determine the Disk number from '%s'.\n", achBuffer);
            DclPrintf("The Disk must be specified in the form: %s\n\n", tp.pszDriveForms);

            dclStat = DCLSTAT_INVALIDDEVICE;
            goto Cleanup;
        }
        else
        {
            /*  The device name must be specified in Unicode TCHAR format
                so convert it now.
            */
            MultiByteToWideChar(CP_ACP, 0, achBuffer, -1, awcDevName, sizeof(awcDevName));
        }

        /*  clear the drive designation parameter
        */
        DclArgClear(szCmdLine, 1);
    }
    else
    {
        goto DisplayHelp;
    }

    tp.dtp.pszCmdLine = &szCmdLine[0];
    tp.nDeviceNum = 0;
    tp.nDiskNum = uDiskNum;

    hStore = OpenStore(awcDevName);
    if(hStore == INVALID_HANDLE_VALUE)
    {
        DclPrintf(PROGRAM_NAME": Unable to open the store, Error=0x%lx\n", GetLastError());

        dclStat = DCLSTAT_INVALIDDEVICE;

        goto Cleanup;
    }

    if(!DismountStore(hStore))
    {
        DclPrintf(PROGRAM_NAME": Unable to dismount the store, Error=0x%lx\n", GetLastError());

        dclStat = DCLSTAT_ACCESSDENIED;

        goto Cleanup;
    }

    CloseHandle(hStore);
    goto RunCommand;

  DisplayHelp:

    strcpy(achBuffer, "/?");
    tp.dtp.pszCmdLine = achBuffer;

  RunCommand:

    iReturn = FfxTestFMSL(&tp);

  Cleanup:

    if(hRedirect)
        DclCeOutputRedirectionEnd(hRedirect);

    if(!iReturn)
        iReturn = DclStatusToErrorlevel(dclStat);

    if(hReq)
        FlashFX_Close(hReq);

    return iReturn;
}


