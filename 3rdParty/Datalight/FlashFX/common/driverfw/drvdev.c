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

    This module contains Driver Framework functions to manage Devices.
    Typically these functions are used by the OS-specific device driver.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvdev.c $
    Revision 1.29  2009/11/06 22:28:08Z  garyp
    Updated the enumerate function to more politely handle the case
    where the driver is not loaded.
    Revision 1.28  2009/08/27 20:27:48Z  billr
    Fix indentation.
    Revision 1.27  2009/07/21 20:59:57Z  garyp
    Merged from the v4.0 branch.  Eliminated a deprecated header.  Updated
    FfxDriverDeviceGeometry() to report whether BBM is loaded for a Device
    or not.  Minorcleanup in the Create function -- not functionally changed.  
    Revision 1.26  2009/04/07 19:57:07Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.25  2009/04/03 19:31:17Z  davidh
    Function header errors corrected for AutoDoc.
    Revision 1.24  2009/03/31 18:02:50Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.23  2009/02/06 07:57:30Z  keithg
    Replaced asserts with error handling; made several function headers
    protected since they are not intended to be used outside this module.
    Revision 1.22  2008/12/23 21:36:03  keithg
    Removed obsolete macro FFX_BBMFORMAT_REUSE_DISCOVEREDBADBLOCKS
    Revision 1.21  2008/05/21 02:13:37Z  garyp
    Merged from the WinMobile branch.
    Revision 1.20.1.2  2008/05/21 02:13:37Z  garyp
    Updated to use DCLISALIGNED().
    Revision 1.20  2008/05/03 03:48:22Z  garyp
    Fixed to build properly when BBM support is disabled.
    Revision 1.19  2008/03/26 00:58:38Z  Garyp
    Major update to streamline DEVICE management.  Refactored the DEVICE
    creation function into two functions -- one to provide the original
    functionality, and one to facilitate automated DEVICE creation from
    generic code where the hooks are not used.  Added functions to enumerate
    DEVICES and to report DEVICE geometry.
    Revision 1.18  2008/01/22 04:29:36Z  Garyp
    BZ 1760 -- Cleaned up error handling logic to not assert for some types
    of errors during device creation.
    Revision 1.17  2008/01/13 07:26:21Z  keithg
    Function header updates to support autodoc.
    Revision 1.16  2007/12/03 01:59:08Z  Garyp
    Added FfxDriverDevicePtr().  Fixed a broken error message.
    Revision 1.15  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.14  2007/01/31 20:54:16Z  Garyp
    Added support for handling device timing and latency settings.
    Revision 1.13  2006/04/26 23:40:07Z  billr
    Pointers can be wider than 32 bits.
    Revision 1.12  2006/03/15 00:30:27Z  Garyp
    Fixed to build cleanly if BBMSUPPORT is on but BBMFORMAT support is off.
    Revision 1.11  2006/03/08 01:26:48Z  Garyp
    Added a check to avoid creating the same device twice.
    Revision 1.10  2006/02/17 22:43:34Z  timothyj
    Added two separate DclAssert() to an error case to indicate the specific
    cause of the error.
    Revision 1.9  2006/02/15 01:42:32Z  Garyp
    Corrected a typo.
    Revision 1.8  2006/02/15 00:15:55Z  Garyp
    Corrected compiler warnings.
    Revision 1.7  2006/02/14 19:53:58Z  Garyp
    Improved error handling logic.
    Revision 1.6  2006/02/13 08:58:54Z  Garyp
    Updated debugging code.
    Revision 1.5  2006/02/12 03:59:28Z  Garyp
    Added FfxDriverDeviceHandle.
    Revision 1.4  2006/02/11 04:39:54Z  Garyp
    Removed some dead code.
    Revision 1.3  2006/02/10 07:49:48Z  Garyp
    Completely revamped the device initialization process.
    Revision 1.2  2006/01/27 00:01:57Z  Pauli
    Removed a memset call in device unmount to prevent clearing data
    that is still in use.
    Revision 1.1  2005/10/06 05:50:54Z  Pauli
    Initial revision
    Revision 1.9  2004/12/30 17:32:41Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2004/11/19 21:04:13Z  GaryP
    Added FfxDriverDevicePointer().
    Revision 1.7  2004/09/22 16:56:19Z  GaryP
    Changed the device sector length field to be a 32-bit value.
    Revision 1.6  2004/08/13 22:53:20Z  GaryP
    Moved the FAT Monitor init/deinit logic so it happens after the allocator
    is mounted.
    Revision 1.5  2004/08/11 19:26:52Z  GaryP
    Modified to zero the pDev structure at init time.  Added some debugging
    code.
    Revision 1.4  2004/08/10 07:28:38Z  garyp
    Added a missnig header.
    Revision 1.3  2004/08/06 02:51:03Z  GaryP
    Added FATMON init and deinit code.
    Revision 1.2  2004/07/21 21:17:18Z  GaryP
    Added debug code.
    Revision 1.1  2003/12/04 23:17:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxdevapi.h>

#define BAD_DEVICENUM   (UINT_MAX)

#if FFXCONF_BBMSUPPORT && D_DEBUG
static FFXSTATUS ValidateBbmSettings(FFXDEVCONFIG *pConf);
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceCreate()

    Create a physical Device instance.  This function calls
    FfxHookDeviceCreate() with the pDevCreateData to obtain the
    Device parameters.

    NOTE: This function is NOT available through the external
          interface.  Use FfxDriverDeviceCreateParam() to create
          Devices through the external interface.

    Parameters:
        pFDI           - A pointer to the FFXDRIVERINFO structure
                         to use.
        pDevCreateData - A pointer to the FFXDEVINITDATA structure
                         to use.  This value may only be NULL if
                         FfxHookDeviceCreate() does not use it.

    Return Value:
        Returns a FFXDEVHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceCreate(
    FFXDRIVERINFO          *pFDI,
    FFXDEVINITDATA         *pDevCreateData)
 {
    FFXDEVCONFIG            conf;
    FFXDEVHANDLE            hDev = NULL;
    FFXDEVHOOK             *pHook;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDeviceCreate() pDrv=%P pCreateData=%P\n", pFDI, pDevCreateData));

    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    if(!pFDI || !pDevCreateData)
    {
        return NULL;
    }

    DclMemSet(&conf, 0, sizeof(conf));
    conf.nDevNum = BAD_DEVICENUM;

    /*  Call the Project Hooks code to allow any project specific
        configuration to occur.  It is the responsibility of the
        hook code to tell us which DEVn we are configuring.
    */
    pHook = FfxHookDeviceCreate(pFDI, pDevCreateData, &conf);
    if(pHook)
    {
        DclAssert(conf.nDevNum < FFX_MAX_DEVICES);

        /*  Now do the real Device initialization
        */
        hDev = FfxDriverDeviceCreateParam(pFDI, &conf, pHook);
        if(!hDev)
            FfxHookDeviceDestroy(pFDI, pHook);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverDeviceCreate() for DEV%u returning hDev=%P\n", conf.nDevNum, hDev));

    return hDev;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceCreateParam()

    Create a physical Device instance, using the parameters in the
    supplied FFXDEVCONFIG structure.

    The supplied FFXDEVCONFIG structure may be fully, partially, or
    not at all configured.  The various "fInitedxxxSettings" flags
    must be set to indicate which settings have already been
    initialized.  Any settings which have not been initialized will
    be set to use default values, typically from ffxconf.h.

    The contents of the FFXDEVCONFIG structure may be modified during
    the course of this create operation, and on exit it will reflect
    the configuration settings which were used.

    Parameters:
        pFDI  - A pointer to the FFXDRIVERINFO structure to use.
        pConf - A pointer to the FFXDEVCONFIG structure to use.
        pHook - A pointer to the FFXDEVHOOK structure returned
                by FfxHookDeviceCreate().  This value may be NULL
                if the DeviceCreate/Destroy() Project Hooks are
                not being used.

    Return Value:
        Returns a FFXDEVHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceCreateParam(
    FFXDRIVERINFO          *pFDI,
    FFXDEVCONFIG           *pConf,
    FFXDEVHOOK             *pHook)
{
    FFXDEVHANDLE            hDev = NULL;
    FFXDEVINFO             *pDev;
    FFXSTATUS               ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDeviceCreateParam() pDrv=%P pConf=%P pHook=%P\n", pFDI, pConf, pHook));

    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    DclAssert(pFDI);
    DclAssert(pConf);

    if(pConf->nDevNum >= FFX_MAX_DEVICES)
    {
        DclError();
        goto Cleanup;
    }

    if(pFDI->apDev[pConf->nDevNum])
    {
        FFXPRINTF(1, ("DEV%u already initialized\n", pConf->nDevNum));
        goto Cleanup;
    }

    if(!pConf->fInitedDevSettings)
    {
        ffxStat = FfxDriverDeviceSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedDevSettings);
    }

  #if FFXCONF_BBMSUPPORT
    if(!pConf->fInitedBbmSettings)
    {
        ffxStat = FfxDriverDeviceBbmSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedBbmSettings);
    }

  #if D_DEBUG
    ffxStat = ValidateBbmSettings(pConf);
    if(ffxStat != FFXSTAT_SUCCESS)
        goto Cleanup;
  #endif
  #endif

  #if FFXCONF_LATENCYREDUCTIONENABLED
    if(!pConf->fInitedLatencySettings)
    {
        ffxStat = FfxDriverDeviceLatencySettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedLatencySettings);
    }
  #endif

    pDev = DclMemAllocZero(sizeof(*pDev));
    if(pDev)
    {
      #if D_DEBUG
        DclMemCpy(pDev->acSignature, FFX_DEVICE_SIGNATURE, sizeof pDev->acSignature);
      #endif

        pDev->pDriverInfo = pFDI;
        pDev->pDevHook = pHook;
        pDev->Conf = *pConf;

        pFDI->apDev[pConf->nDevNum] = pDev;

        hDev = &pFDI->apDev[pConf->nDevNum];

        pDev->hFimDev = FfxDevCreate(hDev, (D_UINT16)pConf->nDevNum);
        if(!pDev->hFimDev)
        {
            pFDI->apDev[pConf->nDevNum] = NULL;

            DclMemFree(pDev);

            hDev = NULL;
        }
    }

  Cleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverDeviceCreateParam() returning hDev=%P\n", hDev));

    return hDev;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceDestroy()

    Destroy a Device instance.

    Parameters:
        hDev - The FFXDEVHANDLE to use

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceDestroy(
    FFXDEVHANDLE    hDev)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADHANDLE;
    unsigned        nDevNum = UINT_MAX;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDeviceDestroy() hDev=%P\n", hDev));

    if(FfxDriverDeviceHandleValidate(hDev))
    {
        FFXDEVINFO     *pDev = *hDev;

        nDevNum = pDev->Conf.nDevNum;

        DclAssert(pDev->hFimDev);

        ffxStat = FfxDevDestroy(pDev->hFimDev);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FFXDRIVERINFO  *pFDI = pDev->pDriverInfo;
            FFXDEVHOOK     *pHook = pDev->pDevHook;

            DclMemFree(pDev);

            if(pHook)
                FfxHookDeviceDestroy(pFDI, pHook);

            /*  Remove this DEVICE from the driver's list of DEVICEs.
            */
            pFDI->apDev[nDevNum] = NULL;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverDeviceDestroy() DEV%u returning Status=%lX\n", nDevNum, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceEnumerate()

    Enumerate the Devices in the system.  They are enumerated in
    "Device number" order.

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.  This
               value may be null to use the current driver
               data structure (this is done only to accommodate
               external API needs).
        hDev     - The handle for the last Device enumerated.  Set
               this to NULL to start the enumeration.

    Return Value:
        Returns the FFXDEVICEHANDLE value for the next Device, or
        NULL if the enumeration is complete.
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceEnumerate(
    FFXDRIVERINFO  *pFDI,
    FFXDEVHANDLE    hDev)
{
    unsigned        nDevNum = 0;

    if(!pFDI)
    {
        pFDI = FfxDriverHandle(1);
        if(!pFDI)
        {
            DCLPRINTF(1, ("A FlashFX driver for instance 1 is not loaded\n"));
            return NULL;
        }
    }

    if(hDev)
    {
        FFXDEVINFO     *pDev;

        pDev = FfxDriverDevicePtr(hDev);
        if(!pDev)
        {
            DclError();
            return NULL;
        }

        nDevNum = pDev->Conf.nDevNum + 1;
    }

    for(; nDevNum < FFX_MAX_DEVICES; nDevNum++)
    {
        hDev = FfxDriverDeviceHandle(pFDI, nDevNum);
        if(hDev)
            return hDev;
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceHandle()

    Return the Device handle for a given Device number.

    Parameters:
        pFDI    - A pointer to the FFXDRIVERINFO structure.
        nDevNum    - The Device number

    Return Value:
        Returns an FFXDEVHANDLE value if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDEVHANDLE FfxDriverDeviceHandle(
    FFXDRIVERINFO  *pFDI,
    unsigned        nDevNum)
{
    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    if(pFDI == NULL)
        return NULL;

    if(nDevNum >= FFX_MAX_DEVICES)
        return NULL;

    if(!pFDI->apDev[nDevNum])
        return NULL;

    if(pFDI->apDev[nDevNum]->Conf.nDevNum != nDevNum)
        return NULL;

    return &pFDI->apDev[nDevNum];
}


/*-------------------------------------------------------------------
    Public: FfxDriverDevicePtr()

    Return a pointer to the FFXDEVINFO structure, given a Device
    handle.

    Note that typically the driver framework code simply
    dereferences the handle.  This function is provided as
    a convenience.

    Parameters:
        hDev - The FFXDEVHANDLE to use

    Return Value:
        If successful, returns a pointer to the FFXDEVINFO structure
        for the Device, or NULL otherwise.
-------------------------------------------------------------------*/
FFXDEVINFO * FfxDriverDevicePtr(
    FFXDEVHANDLE    hDev)
{
    DclAssert(FfxDriverDeviceHandleValidate(hDev));

    return *hDev;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceGeometry()

    Get the geometry for a Device.

    Parameters:
        hDev        - The Device handle
        pGeometry - A pointer to the FFXDEVGEOMETRY structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceGeometry(
    FFXDEVHANDLE        hDev,
    FFXDEVGEOMETRY     *pGeometry)
{
    FFXFIMDEVINFO       DevInfo;

    DclAssert(FfxDriverDeviceHandleValidate(hDev));
    DclAssert(pGeometry);

    if(!FfxDevInfo((*hDev)->hFimDev, &DevInfo))
        return FFXSTAT_DEVMGR_INFOFAILED;

    DclMemSet(pGeometry, 0, sizeof(*pGeometry));

    pGeometry->nDevNum          = (*hDev)->Conf.nDevNum;
    pGeometry->nDevType         = (unsigned)DevInfo.uDeviceType;    /* Device type code prefixed with DEVTYPE_*     */
    pGeometry->nPageSize        = (unsigned)DevInfo.uPageSize;      /* Device page size                             */
    pGeometry->ulReservedBlocks = DevInfo.ulReservedBlocks;         /* Blocks reserved (inaccessible)               */
    pGeometry->ulTotalBlocks    = DevInfo.ulTotalBlocks;            /* Total usable blocks in the array             */
    pGeometry->ulBlockSize      = DevInfo.ulBlockSize;              /* Physical erase block size                    */

    if(DevInfo.uDeviceFlags & DEV_USING_BBM)
        pGeometry->fBBMLoaded = TRUE;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceHandleValidate()

    Validate a Device handle.

    Parameters:
        hDev - The DEVICE handle

    Return Value:
        Returns TRUE if the handle is a valid Device handle,
        otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverDeviceHandleValidate(
    FFXDEVHANDLE    hDev)
{
    FFXDEVINFO     *pDev;
    FFXDRIVERINFO  *pFDI;

    if(!hDev)
        return FALSE;

    pDev = *hDev;

    if(!pDev)
        return FALSE;

    if(!DCLISALIGNED((D_UINTPTR)pDev, DCL_ALIGNSIZE))
        return FALSE;

  #if D_DEBUG
    if(DclMemCmp(pDev->acSignature, FFX_DEVICE_SIGNATURE, sizeof pDev->acSignature))
        return FALSE;
  #endif

    if(!pDev->pDriverInfo)
        return FALSE;

    if(pDev->Conf.nDevNum >= FFX_MAX_DEVICES)
        return FALSE;

    pFDI = pDev->pDriverInfo;

  #if D_DEBUG
    if(DclMemCmp(pFDI->acSignature, FFX_DRIVER_SIGNATURE, sizeof pFDI->acSignature))
        return FALSE;
  #endif

    if(pFDI->apDev[pDev->Conf.nDevNum] != pDev)
        return FALSE;

    if(&pFDI->apDev[pDev->Conf.nDevNum] != hDev)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDeviceSettings()

    Initializes the Device settings from the developer specified
    configuration information in ffxconf.h.

    This function supports up to 8 Devices by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DEVICES > 8).

    Note that in some environments portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Device creation, we must pass in a pointer to
    the FFXDEVINFO structure to use, rather than a Device
    handle (since it is not known yet).

    Parameters:
        pConf - A pointer to the FFXDEVCONFIG structure to use.
                The nDevNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceSettings(
    FFXDEVCONFIG   *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DEVICES > 8
#error "FlashFX DEVICE settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedDevSettings);

    switch(pConf->nDevNum)
    {
      #if FFX_MAX_DEVICES > 0 && defined(FFX_DEV0_SETTINGS)
        case 0:
        {
            FFXDEVSETTINGS  ds = FFX_DEV0_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 1 && defined(FFX_DEV1_SETTINGS)
        case 1:
        {
            FFXDEVSETTINGS  ds = FFX_DEV1_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 2 && defined(FFX_DEV2_SETTINGS)
        case 2:
        {
            FFXDEVSETTINGS  ds = FFX_DEV2_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 3 && defined(FFX_DEV3_SETTINGS)
        case 3:
        {
            FFXDEVSETTINGS  ds = FFX_DEV3_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 4 && defined(FFX_DEV4_SETTINGS)
        case 4:
        {
            FFXDEVSETTINGS  ds = FFX_DEV4_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 5 && defined(FFX_DEV5_SETTINGS)
        case 5:
        {
            FFXDEVSETTINGS  ds = FFX_DEV5_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif
      #if FFX_MAX_DEVICES > 6 && defined(FFX_DEV6_SETTINGS)
        case 6:
        {
            FFXDEVSETTINGS  ds = FFX_DEV6_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif
      #if FFX_MAX_DEVICES > 7 && defined(FFX_DEV7_SETTINGS)
        case 7:
        {
            FFXDEVSETTINGS  ds = FFX_DEV7_SETTINGS;

            pConf->DevSettings = ds;
            pConf->fInitedDevSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDevNum >= FFX_MAX_DEVICES)
            {
                DclPrintf("FFX: DEV%u out of range or not configured (FFX_MAX_DEVICES=%u)\n", pConf->nDevNum, FFX_MAX_DEVICES);

                ffxStat = FFXSTAT_DEVICENUMBERINVALID;
            }
            else
            {
                FFXDEVSETTINGS     ds = {FFX_DEFAULT_DEV_SETTINGS};

                FFXPRINTF(1, ("Basic settings are missing for DEV%u, using default values\n", pConf->nDevNum));

                pConf->DevSettings = ds;
                pConf->fInitedDevSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDeviceSettings() for DEV%u returning Status=%lX\n", pConf->nDevNum, ffxStat));

    return ffxStat;
}


#if FFXCONF_BBMSUPPORT

/*-------------------------------------------------------------------
    Public: FfxDriverDeviceBbmSettings()

    Initializes the Device BBM settings from the developer specified
    configuration information in ffxconf.h.

    This function supports up to 8 Devices by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DEVICES > 8).

    Note that in some environments portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Device creation, we must pass in a pointer to
    the FFXDEVINFO structure to use, rather than a Device
    handle (since it is not known yet).

    Parameters:
        pConf - A pointer to the FFXDEVCONFIG structure to use.
                The nDevNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceBbmSettings(
    FFXDEVCONFIG   *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DEVICES > 8
#error "FlashFX DEVICE settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedBbmSettings);

    switch(pConf->nDevNum)
    {
      #if FFX_MAX_DEVICES > 0 && defined(FFX_DEV0_SETTINGS)
        case 0:
        {
            FFXBBMSETTINGS      bs = FFX_DEV0_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 1 && defined(FFX_DEV1_SETTINGS)
        case 1:
        {
            FFXBBMSETTINGS      bs = FFX_DEV1_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
         }
      #endif

      #if FFX_MAX_DEVICES > 2 && defined(FFX_DEV2_SETTINGS)
        case 2:
        {
            FFXBBMSETTINGS      bs = FFX_DEV2_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 3 && defined(FFX_DEV3_SETTINGS)
        case 3:
        {
            FFXBBMSETTINGS      bs = FFX_DEV3_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 4 && defined(FFX_DEV4_SETTINGS)
        case 4:
        {
            FFXBBMSETTINGS      bs = FFX_DEV4_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 5 && defined(FFX_DEV5_SETTINGS)
        case 5:
        {
            FFXBBMSETTINGS      bs = FFX_DEV5_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 6 && defined(FFX_DEV6_SETTINGS)
        case 6:
        {
            FFXBBMSETTINGS      bs = FFX_DEV6_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 7 && defined(FFX_DEV7_SETTINGS)
        case 7:
        {
            FFXBBMSETTINGS      bs = FFX_DEV7_BBM;

            pConf->BbmSettings = bs;
            pConf->fInitedBbmSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDevNum >= FFX_MAX_DEVICES)
            {
                DclPrintf("FFX: DEV%u out of range or not configured (FFX_MAX_DEVICES=%u)\n", pConf->nDevNum, FFX_MAX_DEVICES);

                ffxStat = FFXSTAT_DEVICENUMBERINVALID;
            }
            else
            {
                FFXBBMSETTINGS     bs = {FFX_DEFAULT_BBM_SETTINGS};

                FFXPRINTF(1, ("BBM settings are missing for DEV%u, using default values\n", pConf->nDevNum));

                pConf->BbmSettings = bs;
                pConf->fInitedBbmSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDeviceBbmSettings() for DEV%u returning Status=%lX\n", pConf->nDevNum, ffxStat));

    return ffxStat;
}


#if D_DEBUG

/*-------------------------------------------------------------------
    Local: ValidateBbmSettings()

    Parameters:
        pConf - A pointer to the FFXDEVCONFIG structure to use.
                The nDevNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ValidateBbmSettings(
    FFXDEVCONFIG   *pConf)
{
    if(pConf->BbmSettings.nFormatState != FFX_FORMAT_NEVER &&
       pConf->BbmSettings.nFormatState != FFX_FORMAT_ONCE &&
       pConf->BbmSettings.nFormatState != FFX_FORMAT_ALWAYS &&
       pConf->BbmSettings.nFormatState != FFX_FORMAT_UNFORMAT)
    {
        FFXPRINTF(1, ("BBM Format State setting invalid for DEV%u\n", pConf->nDevNum));
        DclError();
        return FFXSTAT_INVALIDFORMATSTATE;
    }

    if(pConf->BbmSettings.nFormatType != FFX_BBMFORMAT_PRESERVE_BADBLOCKS &&
       pConf->BbmSettings.nFormatType != FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS)
    {
        FFXPRINTF(1, ("BBM Format Type setting invalid for DEV%u\n", pConf->nDevNum));
        DclError();
        return FFXSTAT_INVALIDFORMATTYPE;
    }

    return FFXSTAT_SUCCESS;
}
#endif
#endif


#if FFXCONF_LATENCYREDUCTIONENABLED

/*-------------------------------------------------------------------
    Public: FfxDriverDeviceLatencySettings()

    Initialize the Device latency settings from the developer
    specified configuration information in ffxconf.h.

    This function supports up to 8 Devices by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DEVICES > 8).

    Note that in some environments portions of these settings
    may be specified via other means, and therefore #ifdef
    is used to ensure that only valid settings are configured.

    Note also that because this function is used during the
    course of Device creation, we must pass in a pointer to
    the FFXDEVINFO structure to use, rather than a Device
    handle (since it is not known yet).

    Parameters:
        pConf - A pointer to the FFXDEVCONFIG structure to use.
                The nDevNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDeviceLatencySettings(
    FFXDEVCONFIG   *pConf)
 {
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DEVICES > 8
#error "FlashFX DEVICE latency settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedLatencySettings);

    switch(pConf->nDevNum)
    {
      #if FFX_MAX_DEVICES > 0 && defined(FFX_DEV0_TIMING) && defined(FFX_DEV0_LATENCY)
        case 0:
        {
            FFXDEVTIMING       dt = FFX_DEV0_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV0_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 1 && defined(FFX_DEV1_TIMING) && defined(FFX_DEV1_LATENCY)
        case 1:
        {
            FFXDEVTIMING       dt = FFX_DEV1_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV1_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 2 && defined(FFX_DEV2_TIMING) && defined(FFX_DEV2_LATENCY)
        case 2:
        {
            FFXDEVTIMING       dt = FFX_DEV2_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV2_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 3 && defined(FFX_DEV3_TIMING) && defined(FFX_DEV3_LATENCY)
        case 3:
        {
            FFXDEVTIMING       dt = FFX_DEV3_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV3_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 4 && defined(FFX_DEV4_TIMING) && defined(FFX_DEV4_LATENCY)
        case 4:
        {
            FFXDEVTIMING       dt = FFX_DEV4_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV4_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 5 && defined(FFX_DEV5_TIMING) && defined(FFX_DEV5_LATENCY)
        case 5:
        {
            FFXDEVTIMING       dt = FFX_DEV5_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV5_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 6 && defined(FFX_DEV6_TIMING) && defined(FFX_DEV6_LATENCY)
        case 6:
        {
            FFXDEVTIMING       dt = FFX_DEV6_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV6_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DEVICES > 7 && defined(FFX_DEV7_TIMING) && defined(FFX_DEV7_LATENCY)
        case 7:
        {
            FFXDEVTIMING       dt = FFX_DEV7_TIMING;
            FFXDEVLATENCY      dl = FFX_DEV7_LATENCY;

            pConf->DevTiming = dt;
            pConf->DevLatency = dl;
            pConf->fInitedLatencySettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDevNum >= FFX_MAX_DEVICES)
            {
                DclPrintf("FFX: DEV%u out of range or not configured (FFX_MAX_DEVICES=%u)\n", pConf->nDevNum, FFX_MAX_DEVICES);

                ffxStat = FFXSTAT_DEVICENUMBERINVALID;
            }
            else
            {
                FFXDEVTIMING     dt = {FFX_DEFAULT_TIMING_SETTINGS};
                FFXDEVLATENCY    dl = {FFX_DEFAULT_LATENCY_SETTINGS};

                FFXPRINTF(1, ("Timing and/or Latency settings are missing for DEV%u, using default values\n", pConf->nDevNum));

                pConf->DevTiming = dt;
                pConf->DevLatency = dl;
                pConf->fInitedLatencySettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDeviceLatencySettings() for DEV%u returning Status=%lX\n", pConf->nDevNum, ffxStat));

    return ffxStat;
}


#endif



