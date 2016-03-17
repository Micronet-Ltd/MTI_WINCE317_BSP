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

    This module contains the Loader Hooks Layer default implementations for:

       FfxHookDiskCreate()
       FfxHookDiskDestroy()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhdisk.c $
    Revision 1.5  2009/07/17 19:10:50Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.4  2009/04/08 19:38:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2008/03/23 04:18:30Z  Garyp
    Updated to the new-and-improved method of DISK creation, where the hook
    function fills in a FFXDISKCONFIG structure, rather than touching the FFXDISKINFO
    structure directly.  Any configuration options not supplied will be automatically
    filled in with the default values.
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/10/13 01:42:30Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxdrv.h>

/*-------------------------------------------------------------------
    Public: FfxHookDiskCreate()

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

    Return Value:
        If successful, returns an opaque pointer/handle for the
        newly created FFXDISKHOOK object, or NULL otherwise.
-------------------------------------------------------------------*/
FFXDISKHOOK *FfxHookDiskCreate(
    const FFXDRIVERINFO    *pDI,
    const FFXDISKINITDATA  *pDiskData,
    FFXDISKCONFIG          *pConf)
{
    FFXDISKHOOK            *pHook;

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

    pHook = DclMemAllocZero(sizeof *pHook);
    if(pHook)
    {
        /*  The FFXDISKHOOK needs to record the disk number.
        */
        pHook->nDiskNum = pDiskData->nDiskNum;

        /*  Here's how the project code communicates to the driver framework
            which disk number this is.
        */
        pConf->nDiskNum = pDiskData->nDiskNum;

        /*******************************************************************\
         *  If necessary, any or all DISK configuration parameters may     *
         *  be placed into pConf at this point, setting the appropriate    *
         *  "fInitedxxxSettings" flags along the way.                      *
         *                                                                 *
         *  However, for this OS, we simply use the settings as defined in *
         *  the ffxconf.h file.  The only required value which must be set *
         *  is the DISK number.                                            *
        \*******************************************************************/

        return pHook;
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxHookDiskDestroy()

    This function destroys a FFXDISKHOOK object that was created
    with FfxHookDiskCreate().

    Parameters:
        pDI   - A pointer to the FFXDRIVERINFO structure to use
        pHook - The opaque pointer/handle for the FFXDISKHOOK
                object to destroy

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookDiskDestroy(
    const FFXDRIVERINFO    *pDI,
    FFXDISKHOOK            *pHook)
{
    DclAssert(pDI);
    DclAssert(pHook);

    (void)pDI;

    DclMemFree(pHook);

    return;
}



