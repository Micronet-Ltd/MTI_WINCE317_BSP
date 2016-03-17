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

    This module is interface to the StressMT test.

    See the comments included in the respective tests/tools code for
    the syntax for this utility.
---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                              Revision History
    $Log: mtstress.c $
    Revision 1.8  2009/02/09 18:53:20Z  thomd
    Corrected affected members of FFXTOOLPARAMS
    Revision 1.7  2008/01/13 07:28:43Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:50:02Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/05 22:08:53Z  garyp
    Updated to return standard errorlevel values as defined in dlerrlev.h
    (where possible).
    Revision 1.4  2007/06/12 18:20:03Z  timothyj
    Account for RTOS's without threading capability (BZ 1206, 1207)
    Revision 1.3  2006/10/10 16:46:16Z  Pauli
    Merged from v310_Release branch.
    Revision 1.2.1.2  2006/10/10 16:46:16Z  Pauli
    MTStress parses the parameters string for the device specifier.  The
    TX tool framework routine must be bypassed to make this work because
    it finds and removes the device specifier.
    Revision 1.2.1.1  2006/05/06 00:17:29Z  Pauli
    Duplicate revision
    Revision 1.2  2006/05/06 00:17:29Z  Pauli
    Added "const" to input parameter.
    Revision 1.1  2006/01/25 17:25:14Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <toolfw.h>
#include <oecommon.h>
#include <dlerrlev.h>

#if DCL_OSFEATURE_THREADS

static char szCmdName[] = "FlashFXStressMT";

extern FFXDRIVERINFO *pDI;

/*-------------------------------------------------------------------
    Public: FlashFXStressMT

    Parameters:

    Return:

-------------------------------------------------------------------*/
int FlashFXStressMT(
    const char     *pszCmdLine)
{
    FFXTOOLPARAMS   tp;
    int             iReturn;

    tp.dtp.pszCmdName = szCmdName;
    tp.dtp.pszCmdLine = pszCmdLine;
    tp.dtp.pszExtraHelp = 0;
    tp.pszDriveForms = FfxGetDriveFormatString();
    tp.nDeviceNum = 0;
    tp.nDiskNum = 0;

    if(!FfxDriverLock(pDI))
    {
        DclPrintf("%s: Unable to lock device driver\n", szCmdName);

        return DCLERRORLEVEL_DRIVERLOCKFAILED;
    }

    iReturn = (int)FfxStressMT(&tp);

    FfxDriverUnlock(pDI);

    return iReturn;
}



#endif /* DCL_OSFEATURE_THREADS */


