/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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
    $Log: fxdump.c $
    Revision 1.7  2010/12/12 07:26:16Z  garyp
    Updated for the new FXDUMP calling convention.
    Revision 1.6  2009/10/01 18:20:31Z  garyp
    Eliminated the use of ffxapi.h.
    Revision 1.5  2009/07/19 04:49:59Z  garyp
    Merged from the v4.0 branch (again).  Modified to create a DCL instance
    before invoking the application.
    Revision 1.4  2009/02/09 07:15:59Z  garyp
    Merged from the v4.0 branch.  Use the new FFXTOOLPARAMS structure.
    Revision 1.3  2007/11/28 22:45:05Z  Garyp
    Updated to use the new DCL functions to support output redirection.
    Revision 1.2  2007/11/03 23:50:13Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2005/10/03 13:30:18Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 17:51:18Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/06/12 21:13:26Z  pauli
    Initial revision
    Revision 1.3  2005/06/12 21:13:26Z  PaulI
    Set DCL output function to redirect output to the extapi function.
    Revision 1.2  2004/12/30 23:14:05Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/10/27 23:25:44Z  jaredw
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx_wince.h>
#include <fxtools.h>
#include <oecommon.h>
#include <dlceutil.h>
#include <dlerrlev.h>

#define PROGRAM_NAME    "FXDUMP"


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

        dclStat = FfxDump(&tp);

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

