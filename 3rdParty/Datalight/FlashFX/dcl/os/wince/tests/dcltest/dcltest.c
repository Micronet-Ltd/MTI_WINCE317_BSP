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
    executables for this OS.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dcltest.c $
    Revision 1.8  2009/11/02 18:27:41Z  garyp
    Enhanced the error handling logic.
    Revision 1.7  2009/06/27 22:41:49Z  garyp
    Updated to create a DCL Instance for use by the program.
    Revision 1.6  2009/02/09 06:24:51Z  garyp
    Removed a reference to an obsolete field.
    Revision 1.5  2007/11/28 22:45:08Z  Garyp
    Modified to use the new generalized CE output redirection functions.
    Revision 1.4  2007/11/03 23:31:37Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/16 20:03:49Z  Garyp
    Fixed to use a proper printf() format string.
    Revision 1.2  2007/08/06 00:43:47Z  garyp
    Modified to always return a valid errorlevel value as defined in dlerrlev.h.
    Revision 1.1  2005/10/20 02:57:08Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dltools.h>
#include <dlerrlev.h>
#include <dlceutil.h>

#define PROGRAM_NAME    "DCLTEST"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int WINAPI WinMain(
    HINSTANCE           hInstance,
    HINSTANCE           hPrevInstance,
    LPTSTR              ptzCmdLine,
    int                 nCmdShow)
{
    DCLSTATUS           dclStat;
    DCLINSTANCEHANDLE   hDclInst;

    dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        #define                 BUFFLEN (256)
        char                    achBuffer[BUFFLEN];
        DCLREDIRECTIONHANDLE    hRedirect;
        DCLTOOLPARAMS           tp = {0};

        tp.hDclInst   = hDclInst;
        tp.pszCmdName = PROGRAM_NAME;
        tp.pszCmdLine = achBuffer;

        /*  Convert the string from Unicode to ANSI
        */
        WideCharToMultiByte(CP_ACP, 0, ptzCmdLine, -1, achBuffer, BUFFLEN, NULL, NULL);

        hRedirect = DclCeOutputRedirectionBegin(hDclInst, achBuffer);

        dclStat = DclTestMain(&tp);

        if(hRedirect)
            DclCeOutputRedirectionEnd(hRedirect);

        if(DclInstanceDestroy(hDclInst) != DCLSTAT_SUCCESS)
            printf(PROGRAM_NAME": Instance destruction failed\n");
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        int iErrLev = DclStatusToErrorlevel(dclStat);
        
        if(dclStat != DCLSTAT_HELPREQUEST)
            printf(PROGRAM_NAME": Terminated with DL status code 0x%08lX (ERRORLEVEL=%u)\n", dclStat, iErrLev);
        
        return iErrLev;
    }
    else
    {
        return 0;
    }
}

