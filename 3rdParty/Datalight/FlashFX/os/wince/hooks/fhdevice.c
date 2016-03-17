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

    This module contains the Windows CE Hooks Layer default implementations
    for:

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
    Revision 1.9  2009/07/17 20:42:26Z  garyp
    Merged from the v4.0 branch.  Eliminated use of an obsolete header.
    Revision 1.8  2009/04/08 20:32:45Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.7  2008/03/23 04:11:11Z  Garyp
    Updated to the new-and-improved method of DEVICE creation, where the hook
    function fills in a FFXDEVCONFIG structure, rather than touching the FFXDEVINFO
    structure directly.  Any configuration options not supplied will be automatically
    filled in with the default values.
    Revision 1.6  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/08/03 00:14:10Z  timothyj
    Changed units of reserved space and maximum size to be in terms
    of KB instead of bytes.
    Revision 1.4  2006/10/04 01:07:20Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.3  2006/08/14 17:43:14Z  timothyj
    Fixed FFXPRINTF calls that were missing device number, and
    changed %p specifier to %P.
    Revision 1.2  2006/02/15 00:07:51Z  Garyp
    Updated to properly stuff the pDev->uDeviceNum field on successful
    completion.
    Revision 1.1  2006/02/10 01:14:24Z  Garyp
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
    FFXSTATUS               ffxStat;

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

    /*  For this OS, simply use the settings as defined in the ffxconf.h
        file.
    */
    ffxStat = FfxDriverDeviceSettings(pConf);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("DEV%u settings initialization failed, Status=%lX\n", pDevData->nDevNum, ffxStat));

        DclMemFree(pHook);
        return NULL;
    }

    /*  For NAND we don't care about virtual mapping and the base flash
        address is meaningless.  The MaxArraySize is therefore not used
        (at this time) for NAND.

        If the BaseAddress is BAD_FLASH_ADDRESS (0xFFFFFFFF), then assume
        this device is NAND, and skip the mapping.
    */
    if((D_UINT32)pConf->DevSettings.pBaseFlashAddress != BAD_FLASH_ADDRESS)
    {
        D_UINT32    ulLength;

        /*  For NOR, we must map the flash into our address space (and it
            could be mapped at address 0.  We must have a MaxArraySize
            value in order to know how much to map -- we can't just rely
            on the default "use everything" value of 0, since we don't yet
            know at this time how much "everything" is.
        */
        if(!pConf->DevSettings.ulMaxArraySizeKB)
        {
            FFXPRINTF(1, ("DEV%u MaxArraySize is not initialized, BaseAddress=%P\n",
                pDevData->nDevNum, pConf->DevSettings.pBaseFlashAddress));

            DclMemFree(pHook);
            return NULL;
        }

        /*  Determined, based on the configuration settings, what the
            maximum amount of address space is, which we have to map.

            This address space must include the reserved space.
        */
        ulLength = (pConf->DevSettings.ulReservedBottomKB + pConf->DevSettings.ulReservedTopKB) * 1024UL;
        if(D_UINT32_MAX - ulLength < pConf->DevSettings.ulMaxArraySizeKB * 1024)
            ulLength = D_UINT32_MAX;
        else
            ulLength += pConf->DevSettings.ulMaxArraySizeKB * 1024UL;

        pHook->pbFlashMedia = FfxCeMapFlash(pConf->DevSettings.pBaseFlashAddress, ulLength);
        if(!pHook->pbFlashMedia)
        {
            FFXPRINTF(1, ("DEV%u Flash mapping failed, Address=%P Length=%lX\n",
                pDevData->nDevNum, pConf->DevSettings.pBaseFlashAddress, ulLength));

            DclMemFree(pHook);
            return NULL;
        }
    }

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

    if(pHook->pbFlashMedia)
        FfxCeUnmapFlash(pHook->pbFlashMedia);

    DclMemFree(pHook);

    return;
}


