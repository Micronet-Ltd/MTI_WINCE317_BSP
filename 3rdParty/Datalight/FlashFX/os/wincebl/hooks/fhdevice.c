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

    This module contains the WinCE BootLoader Hooks Layer default
    implementations for:

       FfxHookDeviceCreate()
       FfxHookDeviceDestroy()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    This is the FlashFX Hook for creating and initializing project-specific
    data structures for a device.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhdevice.c $
    Revision 1.9  2009/07/17 20:44:23Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.8  2008/03/20 18:53:11Z  Garyp
    Updated to the new-and-improved method of DEVICE creation, where the hook
    function fills in a FFXDEVCONFIG structure, rather than touching the
    FFXDEVINFO structure directly.  Any configuration options not supplied
    will be automatically filled in with the default values.
    Revision 1.7  2008/01/25 07:43:05Z  keithg
    Comment style updates to support autodoc.
    Revision 1.6  2007/11/03 23:50:17Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2006/03/15 03:07:46Z  Garyp
    Updated to build cleanly.
    Revision 1.4  2006/03/09 20:49:13Z  Garyp
    Modified to use a default device number of the FFXDEVINITDATA
    structure is not supplied.
    Revision 1.3  2006/02/26 21:21:25Z  Garyp
    Modified to protect again potentially referencing a NULL pointer.
    Revision 1.2  2006/02/15 00:07:49Z  Garyp
    Updated to properly stuff the pDev->uDeviceNum field on successful
    completion.
    Revision 1.1  2006/02/10 01:28:08Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <ffxldr.h>

#define DEFAULT_DEVICE_NUMBER   (0)


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
    unsigned                nDevNum = DEFAULT_DEVICE_NUMBER;

    (void)pDI;

    DclAssert(pDI);
    DclAssert(pConf);

    /*  If a FFXDEVINITDATA structure is supplied, use the Device number
        specified therein, otherwise use a default device number.
    */
    if(pDevData)
        nDevNum = pDevData->nDevNum;

    if(nDevNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("DEV%u is out of range (max=%u)\n", nDevNum, FFX_MAX_DEVICES-1));
        return NULL;
    }

        /*******************************************************************\
         *  If necessary, any or all Device configuration parameters may   *
         *  be placed into pConf at this point, setting the appropriate    *
         *  "fInitedxxxSettings" flags along the way.                      *
         *                                                                 *
         *  However, for this OS, we simply use the settings as defined    *
         *  in the ffxconf.h file.  The only required value which must be  *
         *  set is the Device number.                                      *
        \*******************************************************************/

    /*  If successful, set the DeviceNum into the pConf structure, and
        return a pointer to the hook structure.
    */
    pConf->nDevNum = nDevNum;

    return (FFXDEVHOOK*)(nDevNum+1);
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

    return;
}


