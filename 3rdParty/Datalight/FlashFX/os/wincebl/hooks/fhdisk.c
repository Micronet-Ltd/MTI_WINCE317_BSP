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

    This module contains the Windows CE BootLoader Hooks Layer default
    implementations for:

       FfxHookDiskCreate()
       FfxHookDiskDestroy()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhdisk.c $
    Revision 1.10  2009/07/17 20:47:56Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.9  2008/03/21 02:12:45Z  Garyp
    Updated to the new-and-improved method of DISK creation, where the hook
    function fills in a FFXDISKCONFIG structure, rather than touching the FFXDISKINFO
    structure directly.  Any configuration options not supplied will be automatically
    filled in with the default values.
    Revision 1.8  2008/01/25 07:43:06Z  keithg
    Comment style updates to support autodoc.
    Revision 1.7  2007/11/03 23:50:17Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/05/21 21:05:23Z  rickc
    Bug 1162 - In FfxHookDiskCreate(), pDiskData is optional arg, changed to
    account for this.
    Revision 1.5  2006/10/13 01:42:27Z  Garyp
    Updated to initialize the disk format settings.
    Revision 1.4  2006/03/15 03:11:53Z  Garyp
    Updated to build cleanly.
    Revision 1.3  2006/03/09 20:49:11Z  Garyp
    Modified to use a default disk number of the FFXDISKINITDATA structure is
    not supplied.
    Revision 1.2  2006/03/06 18:08:50Z  Garyp
    Updated to the new Devices and Disks model.
    Revision 1.1  2006/02/04 19:55:26Z  Garyp
    Initial revision
    Revision 1.3  2005/03/29 03:53:49Z  GaryP
    Minor cleanup, no functional changes.
    Revision 1.2  2004/12/30 23:34:32Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2004/08/17 18:33:26Z  garyp
    Initial revision
    ---------------------
    Bill Roman 2004-06-18
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxldr.h>

#define DEFAULT_DISK_NUMBER   (0)


/*-------------------------------------------------------------------
    Public: FfxHookDiskCreate

    This function is used to create a FFXDISKHOOK object.

    Upon successful completion, this function must fill in the
    pConf->nDiskNum field with the appropriate DISKn number.

    Other configuration parameters may be filled in as well.  Any
    parameters which are not set by this function will use default
    values.

    Parameters:
        pDI       - A pointer to the FFXDRIVERINFO structure to use
        pDiskData - A pointer to the FFXDISKINITDATA structure to use
        pConf     - A pointer to the FFXDISKCONFIG structure to use

    Return:
        If successful, returns an opaque pointer/handle for the
        newly created FFXDISKHOOK object, or NULL otherwise.
-------------------------------------------------------------------*/
FFXDISKHOOK *FfxHookDiskCreate(
    const FFXDRIVERINFO    *pDI,
    const FFXDISKINITDATA  *pDiskData,
    FFXDISKCONFIG          *pConf)
{
    unsigned                nDiskNum = DEFAULT_DISK_NUMBER;
/*  FFXDISKHOOK            *pHook; */

    /*  Not currently used.
    */
    (void)pDI;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, 0),
        "FfxHookDiskCreate() pDrv=%P pDiskInit=%P pConf=%P\n", pDI, pDiskData, pConf));

    DclAssert(pDI);
    DclAssert(pDiskData);
    DclAssert(pConf);

    if(pDiskData->nDiskNum >= FFX_MAX_DISKS)
    {
        FFXPRINTF(1, ("DISK%u is out of range (max=%u)\n", pDiskData->nDiskNum, FFX_MAX_DISKS-1));
        return NULL;
    }

    /*  If a FFXDISKINITDATA structure is supplied, use the disk number
        specified therein, otherwise use a default disk number.
    */
    if(pDiskData)
        nDiskNum = pDiskData->nDiskNum;

/*
    pHook = DclMemAllocZero(sizeof *pHook);
    if(!pHook)
        return NULL;

    pHook->nDiskNum = nDiskNum;
*/
    pConf->nDiskNum = nDiskNum;

        /*******************************************************************\
         *  If necessary, any or all DISK configuration parameters may     *
         *  be placed into pConf at this point, setting the appropriate    *
         *  "fInitedxxxSettings" flags along the way.                      *
         *                                                                 *
         *  However, for this OS, we simply use the settings as defined in *
         *  the ffxconf.h file.  The only required value which must be set *
         *  is the DISK number.                                            *
        \*******************************************************************/

/*  return pHook;   */
    return (FFXDISKHOOK*)(nDiskNum+1);
}


/*-------------------------------------------------------------------
    Public: FfxHookDiskDestroy

    This function destroys a FFXDISKHOOK object that was created
    with FfxHookDiskCreate().

    Parameters:
        pDI     - A pointer to the FFXDRIVERINFO structure to use
        pHook   - The opaque pointer/handle for the FFXDISKHOOK
                  object to destroy

    Return:
        None
-------------------------------------------------------------------*/
void FfxHookDiskDestroy(
    const FFXDRIVERINFO    *pDI,
    FFXDISKHOOK            *pHook)
{
    DclAssert(pDI);
    DclAssert(pHook);

    (void)pDI;
    (void)pHook;

/*    DclMemFree(pHook); */

    return;
}



