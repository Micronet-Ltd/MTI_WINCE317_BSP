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

    This program loads the DLL specified in the supplied registry file, using
    the settings contained in that registry file.  ActivateDevice() loads the
    DLL, then adds it to the active driver list.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: loaddrv.c $
    Revision 1.7  2009/06/27 22:41:50Z  garyp
    Updated to create a DCL Instance for use by the program.
    Revision 1.6  2007/11/29 19:18:02Z  Garyp
    Modified to use the new generalized CE output redirection functions.
    Updated to use the refactored "CE driver load" functionality.
    Revision 1.5  2007/11/03 23:31:37Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/07/18 21:54:38Z  keithg
    Now treat the handle passes as unsigned to avoid overflow.
    This tool now works with Win CE 6.0
    Revision 1.3  2007/05/09 16:26:52Z  keithg
    Fixed quiet option to work with the unload option
    Revision 1.2  2007/04/26 22:44:49Z  keithg
    Added support to unload a driver and a /quiet option - improved help screen.
    Revision 1.1  2007/01/21 19:07:30Z  Garyp
    Initial revision
    Revision 1.2  2006/12/14 00:52:05Z  Garyp
    Added a standard header and updated the code formatting to current
    standards -- no functional changes.
    11/06/01 TWQ Created from Savereg.cpp
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlerrlev.h>
#include <dlceutil.h>
#include <dlcetools.h>

#define PROGRAM_NAME    "LOADDRV"

/*-------------------------------------------------------------------
    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    DCLSTATUS               dclStat;
    DCLINSTANCEHANDLE       hDclInst;

    (void)hInstance;
    (void)hPrevInstance;
    (void)nCmdShow;

    dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        #define                 BUFFLEN (256)
        char                    szCmdLine[BUFFLEN];
        DCLREDIRECTIONHANDLE    hRedirect;

        /*  Convert the string from Unicode to ANSI
        */
        WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, sizeof(szCmdLine), NULL, NULL);

        hRedirect = DclCeOutputRedirectionBegin(hDclInst, szCmdLine);

        dclStat = DclCeDriverLoad(PROGRAM_NAME, szCmdLine);

        if(hRedirect)
            DclCeOutputRedirectionEnd(hRedirect);

        if(DclInstanceDestroy(hDclInst) != DCLSTAT_SUCCESS)
            printf(PROGRAM_NAME": Instance destruction failed\n");
    }
    else
    {
        printf(PROGRAM_NAME": Instance creation failed with status code 0x%08lX\n", dclStat);
    }

    return DclStatusToErrorlevel(dclStat);
}


