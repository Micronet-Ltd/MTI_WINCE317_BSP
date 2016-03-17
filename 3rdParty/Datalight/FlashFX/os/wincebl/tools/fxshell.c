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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshell.c $
    Revision 1.6  2009/11/06 03:03:09Z  garyp
    Updated to create a DCL instance for use by the program.
    Revision 1.5  2009/10/09 01:19:13Z  garyp
    Removed the logic conditional on DCLCONF_COMMAND_SHELL, which
    is now handled at the lower level.
    Revision 1.4  2009/10/01 19:00:31Z  garyp
    Eliminated the use of ffxapi.h.
    Revision 1.3  2009/08/04 17:07:46Z  garyp
    Use more robust structure initialization.
    Revision 1.2  2009/02/09 18:53:24Z  thomd
    Corrected affected members of FFXTOOLPARAMS.
    Revision 1.1  2008/02/24 19:04:56Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <flashfx.h>
#include <flashfx_wincebl.h>
#include <fxtools.h>

#define PROGRAM_NAME    "FXSHELL"


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
int FlashFXShell(
    const char         *pszCmdLine)
{
    DCLINSTANCEHANDLE   hDclInst;
    DCLSTATUS           dclStat;    
    int                 iError = 0;

    dclStat = DclInstanceCreate(0, DCLFLAG_APP, &hDclInst);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        DCLSTATUS       dclStat2;
        FFXTOOLPARAMS   tp = {{0}};

        tp.dtp.pszCmdName   = PROGRAM_NAME;
        tp.dtp.pszCmdLine   = pszCmdLine;
        tp.dtp.hDclInst     = hDclInst;

        iError = FfxShell(&tp);
        if(iError != 0)
            dclStat = DCLSTAT_FAILURE;

        dclStat2 = DclInstanceDestroy(hDclInst);
        
        if(dclStat2 != DCLSTAT_SUCCESS && dclStat2 != DCLSTAT_INST_USAGECOUNTREDUCED)
        {
            DclPrintf(PROGRAM_NAME": Instance destruction failed with status %lX\n", dclStat2);

            /*  Preserve any original status code
            */
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(!iError)
            iError = DclStatusToErrorlevel(dclStat);

        DclPrintf(PROGRAM_NAME": Terminated abnormally with status %lX (ERRORLEVEL=%u)\n", dclStat, iError);
    }        
    
    return iError;
}







