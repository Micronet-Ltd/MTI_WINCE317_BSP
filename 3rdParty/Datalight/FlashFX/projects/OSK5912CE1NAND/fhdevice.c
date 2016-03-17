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

    This module contains the Windows CE Hooks Layer implementations for:

       FfxHookDeviceCreate()
       FfxHookDeviceDestroy()

    This is the FlashFX Hook for creating and initializing project-specific
    data structures for a device.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhdevice.c $
    Revision 1.5  2009/07/17 20:48:00Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.4  2008/03/23 04:11:17Z  Garyp
    Updated to the new-and-improved method of DEVICE creation, where the hook
    function fills in a FFXDEVCONFIG structure, rather than touching the FFXDEVINFO
    structure directly.  Any configuration options not supplied will be automatically
    filled in with the default values.
    Revision 1.3  2007/11/03 23:50:34Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/10/04 02:19:44Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.1  2006/03/17 01:49:58Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxwce.h>
#include <oecommon.h>


/*-------------------------------------------------------------------
    Public: FfxHookDeviceCreate()

    Hook the creation of a Device, allowing project specific,
    run-time customization of the Device configuration parameters.

    Upon successful completion, this function must fill in the
    pConf->nDevNum field with the appropriate DEVn number.

    Parameters:
        pDI      - A pointer to the FFXDRIVERINFO structure to use.
        pDevData - A pointer to the FFXDEVINITDATA structure to use.
        pConf    - A pointer to the FFXDEVCONFIG structure to use.

    Return Value:
        A pointer to the FFXDEVHOOK object if successful, or NULL 
        otherwise.
-------------------------------------------------------------------*/
FFXDEVHOOK *FfxHookDeviceCreate(
    const FFXDRIVERINFO    *pDI,
    const FFXDEVINITDATA   *pDevData,
    FFXDEVCONFIG           *pConf)
{
    FFXDEVHOOK             *pHook;

    (void)pDI;

    DclAssert(pDI);
    DclAssert(pConf);
    DclAssert(pDevData);

    if(pDevData->nDevNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("DEV%u is out of range (max=%u)\n", pDevData->nDevNum, FFX_MAX_DEVICES-1));
        return NULL;
    }

    pHook = DclMemAllocZero(sizeof *pHook);
    if(!pHook)
        return NULL;

    pHook->nDevNum = pDevData->nDevNum;
    pConf->nDevNum = pDevData->nDevNum;

        /*******************************************************************\
         *  If necessary, any or all DEVICE configuration parameters may   *
         *  be placed into pConf at this point, setting the appropriate    *
         *  "fInitedxxxSettings" flags along the way.                      *
         *                                                                 *
         *  However, for this OS, we simply use the settings as defined    *
         *  in the ffxconf.h file.  The only required value which must be  *
         *  set is the DEVICE number.                                      *
        \*******************************************************************/

    return pHook;
}


/*-------------------------------------------------------------------
    Public: FfxHookDeviceDestroy()

    Hook the destruction of a Device.  This function should release
    any resources which were allocated by FfxHookDeviceCreate().

    Parameters:
        pDI   - A pointer to the FFXDRIVERINFO structure to use
        pHook - A pointer to the FFXDEVHOOK object to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookDeviceDestroy(
    const FFXDRIVERINFO    *pDI,
    FFXDEVHOOK             *pHook)
{
    DclAssert(pDI);
    DclAssert(pHook);

    (void)pDI;

    DclMemFree(pHook);

    return;
}


