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

    This module contains helper routines that are used by the OS level
    device driver.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: drvlock.c $
    Revision 1.8  2009/07/18 00:52:21Z  garyp
    Eliminated use of an obsolete header.
    Revision 1.7  2009/03/31 19:19:58Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.6  2008/03/04 02:50:15Z  Garyp
    Documentation updated.
    Revision 1.5  2008/01/13 07:26:29Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/03/13 20:26:33Z  keithg
    Moved assertion and reset of fInFFX to within the protected code path.
    Revision 1.2  2006/02/06 19:18:15Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.1  2005/10/01 11:04:08Z  Pauli
    Initial revision
    Revision 1.2  2005/08/03 19:30:50Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/01 02:30:54Z  pauli
    Initial revision
    Revision 1.3  2004/12/30 21:37:57Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/11/19 20:23:50Z  GaryP
    Updated to use refactored header files.
    Revision 1.1  2003/04/10 18:23:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>


/*-------------------------------------------------------------------
    Public: FfxDriverLock()

    This function acquires a global, driver-level lock, which must
    be released with FfxDriverUnlock.

    Parameters:
        pDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns TRUE if successful or FALSE otherwise.
-------------------------------------------------------------------*/
D_BOOL FfxDriverLock(
    FFXDRIVERINFO *pDI)
{
    if(!DclMutexAcquire(pDI->pMutex))
    {
        DclProductionError();

        return FALSE;
    }

    DclAssert(pDI->fInFFX == FALSE);

    pDI->fInFFX = TRUE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FfxDriverUnlock()

    This function release a global, driver-level lock, which was
    acquired with FfxDriverLock.

    Parameters:
        pDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns nothing
-------------------------------------------------------------------*/
void FfxDriverUnlock(
    FFXDRIVERINFO *pDI)
{
    DclAssert(pDI->fInFFX == TRUE);

    pDI->fInFFX = FALSE;

    if(!DclMutexRelease(pDI->pMutex))
    {
        DclError();

        return;
    }

    return;
}
