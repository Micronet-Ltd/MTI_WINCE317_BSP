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

    This module is an application framework for creating standalone
    executables for this given OS.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlpart.c $
    Revision 1.2  2009/06/27 22:41:49Z  garyp
    Updated to create a DCL Instance for use by the program.
    Revision 1.1  2007/12/14 17:49:32Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <storemgr.h>

#include <dcl.h>
#include <dlerrlev.h>
#include <dlceutil.h>
#include <dlcetools.h>

#define PROGRAM_NAME    "DLPART"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE               hInstance,
    HINSTANCE               hPrevInstance,
    LPTSTR                  ptzCmdLine,
    int                     nCmdShow)
{
    DCLSTATUS               dclStat;
    DCLINSTANCEHANDLE       hDclInst;

    dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        #define                 BUFFLEN     (256)
        char                    szCmdLine[BUFFLEN];
        DCLREDIRECTIONHANDLE    hRedirect;

        /*  Convert the string from Unicode to ANSI
        */
        WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, szCmdLine, BUFFLEN, NULL, NULL);

        hRedirect = DclCeOutputRedirectionBegin(hDclInst, szCmdLine);

        dclStat = DclCePartition(PROGRAM_NAME, szCmdLine);

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


