/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module provides an interface for a low-level read/write page, erase
    block level version of FlashFX, typically in a boot loader environment.

    This interface is designed such that client code calling these functions
    do NOT need to include flashfx.h, but rather need only include
    flashfx_osname.h along with fxloaderapi.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvloader.c $
    Revision 1.12  2010/07/17 23:55:56Z  garyp
    Updated so the "AltUnlock" functionality is optional, and must be
    explicitly turned on.
    Revision 1.11  2010/07/06 02:06:18Z  garyp
    Major update so that Loader Device and Disk handles are now separate
    objects rather than pseudo Driver Device and Disk handles.  Updated
    FfxLoaderUnlockBlocks(), so that when using flash which only supports
    unlocking a single range, the lowest unlocked block will be tracked.
    and subsequent requests to unlock higher blocks will be ignored.
    Revision 1.10  2010/06/20 02:01:53Z  garyp
    Fixed to compile cleanly if allocator support is disabled.
    Revision 1.9  2010/06/18 17:40:51Z  garyp
    Updated to build properly with allocator support disabled.  Docs updated.
    Revision 1.8  2010/06/17 18:16:09Z  garyp
    Fixed a Bug 3132 in the Disk initialization code which would prevent things
    from working properly when the Loader is used for multiple Disks.  Added
    FfxLoaderReadSectors() and FfxLoaderWriteSectors() which provide allocator
    level read/write sector functionality, assuming the allocator is enabled.
    Updated documentation and debug code.
    Revision 1.7  2010/04/29 00:04:20Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.6  2010/01/07 23:51:42Z  garyp
    Modified to use the updated DriverAutoTest interface.
    Revision 1.5  2009/12/09 19:59:21Z  garyp
    Added support for automatically unlocking the flash at Device creation
    time.  Documentation updated.
    Revision 1.4  2009/11/17 18:43:21Z  garyp
    Updated some debug messages -- no functional changes.
    Revision 1.3  2009/11/06 21:21:33Z  garyp
    Updated the Device and Disk creation functions to provide more useful
    error code information on failure.
    Revision 1.2  2009/07/16 01:20:55Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.8  2009/07/16 01:20:55Z  garyp
    Updated to build cleanly in alternate configurations.
    Revision 1.1.1.7  2009/03/22 21:37:32Z  garyp
    Updated to use a structure which was renamed to accommodate the
    auto-documentation system.
    Revision 1.1.1.6  2009/03/09 02:12:12Z  garyp
    Eliminated a deprecated header.
    Revision 1.1.1.5  2009/03/02 19:41:33Z  garyp
    Updated FfxLoaderDiskCreate() to allow the Device number to be specified.
    Revision 1.1.1.4  2009/02/27 20:02:55Z  garyp
    Updated the lock-freeze function to take a start block and count.
    Revision 1.1.1.3  2009/02/17 21:34:26Z  garyp
    Updated to use the new lock-freeze functionality which takes a block
    number and count.
    Revision 1.1.1.2  2009/02/05 21:24:06Z  garyp
    Cleaned up the interface so it is better suited for long-term general use.
    Revision 1.1  2009/02/04 23:21:08Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <fxloaderapi.h>

#define ALL_LOCKED      D_UINT32_MAX

typedef struct sFFXLDRINFO
{
    FFXDRIVERINFO  *pDI;
    D_BOOL          fLocalDcl;
} FFXLDRINFO;

typedef struct sFFXLDRDISKINFO
{
    FFXLOADERHANDLE hLdr;
    FFXDISKHANDLE   hDisk;
    D_BOOL          fAltUnlock;
    D_UINT32        ulLowestUnlockedBlock;  /* only used if fAltUnlock is TRUE */
} FFXLDRDISKINFO;

typedef struct sFFXLDRDEVICEINFO
{
    FFXLOADERHANDLE hLdr;
    FFXDEVHANDLE    hDev;
} FFXLDRDEVICEINFO;

typedef struct
{
    D_UINT32        ulExtraReservedLowKB;
    D_UINT32        ulExtraReservedHighKB;
    D_UINT32        ulMaxArraySizeKB;
    unsigned        nDevNum;
    D_BOOL          fUseBBM;
} DEVCREATEDATA;

typedef struct
{
    unsigned        nDiskNum;
    unsigned        nDevNum;
    D_BOOL          fSpanDevices;
} DISKCREATEDATA;

/*  Global!
*/
static FFXPFNOPTIONGET pfnOldOptionHandler;

static FFXSTATUS InitDevSettings(FFXDEVCONFIG *pConf, const DEVCREATEDATA *pDevData);
static FFXSTATUS InitDiskSettings(FFXDISKCONFIG *pConf, DISKCREATEDATA *pDiskData);
static D_BOOL    LoaderOptionGet(FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);


/*-------------------------------------------------------------------
    Public: FfxLoaderCreate()

    Initialize an instance of the FlashFX loader.

    Parameters:
        hDclInst - The DCL Instance handle to use.  This value may be
                   NULL to cause the DCL Instance to be automatically
                   initialized.

    Return Value:
        Returns the loader handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
FFXLOADERHANDLE FfxLoaderCreate(
    DCLINSTANCEHANDLE   hDclInst)
{
    FFXLDRINFO         *pLdr = NULL;
    DCLINSTANCEHANDLE   hLocalDclInst = NULL;

    if(!hDclInst)
    {
        /*  Create a DCL Instance before doing <anything> else.  This will
            initialize the default services, as defined in the project.
        */
        if(DclInstanceCreate(0, DCLFLAG_DRIVER, &hLocalDclInst) != DCLSTAT_SUCCESS)
            return NULL;

        if(!hLocalDclInst)
            return NULL;

        hDclInst = hLocalDclInst;
    }

    if(!FfxProjMain())
    {
        FFXPRINTF(1, ("FfxLoaderCreate() init cancelled by FfxProjMain\n"));
        goto Cleanup;
    }

    FfxSignOn(FALSE);

  #if FFXCONF_DRIVERAUTOTEST
     FfxDriverUnitTestDCL(hDclInst);
  #endif

    pLdr = DclMemAllocZero(sizeof *pLdr);
    if(!pLdr)
    {
        FFXPRINTF(1, ("FfxLoaderCreate() Out of memory\n"));
        goto Cleanup;
    }

    if(hLocalDclInst)
        pLdr->fLocalDcl = TRUE;

    pLdr->pDI = FfxDriverCreate(1, hDclInst);
    if(pLdr->pDI)
        return pLdr;

    FFXPRINTF(1, ("FfxLoaderCreate() Initialization failed\n"));

  Cleanup:
    if(pLdr)
        DclMemFree(pLdr);

    if(hLocalDclInst)
        DclInstanceDestroy(hLocalDclInst);

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderDestroy()

    Destroy an instance of the FlashFX Loader.  

    *Note* -- The caller must ensure that all Loader Devices and Disks
              have been destroyed prior to calling this function.

    Parameters:
        hLdr    - The handle of the loader instance to destroy

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderDestroy(
    FFXLOADERHANDLE     hLdr)
{
    DCLINSTANCEHANDLE   hLocalDclInst = NULL;
    FFXSTATUS           ffxStat;

    FFXPRINTF(1, ("FfxLoaderDestroy(%P)\n", hLdr));

    /*  We must destroy the DCL Instance if we were the one to create
        it.  Use a temporary variable to ensure we are releasing
        resources in reverse order of allocation/creation.
    */
    if(hLdr->fLocalDcl)
        hLocalDclInst = hLdr->pDI->hDclInst;

    ffxStat = FfxDriverDestroy(hLdr->pDI);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = DclMemFree(hLdr);
        DclAssert(ffxStat == FFXSTAT_SUCCESS);

        if(hLocalDclInst)
        {
            ffxStat = DclInstanceDestroy(hLocalDclInst);
            DclAssert(ffxStat == FFXSTAT_SUCCESS);
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderDeviceCreate()

    Create a FlashFX Loader Device object using the supplied char-
    acteristics.  Note that Devices created with this interface
    do not use the standard FlashFX Device Project Hooks functions 
    FfxHookDeviceCreate() and FfxHookDeviceDestroy() as implemented
    in fhdevice.c.  All Device configuration is done with this 
    function, in combination with the settings in ffxconf.h, as
    described below.

    *Note!* -- The basic Device settings values are retrieved
    from the ffxconf.h file, however the ulMaxArraySizeKB and 
    FFXLOADER_DEV_USEBBM parameters specified in this interface
    will override the equivalent settings from the ffxconf.h file.

    Parameters:
        hLdr                  - The loader instance handle
        nDevNum               - Number of the Device to create
        ulExtraReservedLowKB  - This specifies any "extra" reserved
                                space to use, *above and beyond* any
                                which may be specified in ffxconf.h.
        ulExtraReservedHighKB - This specifies any "extra" reserved
                                space to use, *above and beyond* any
                                which may be specified in ffxconf.h.
        ulMaxArraySizeKB      - This specifies the maximum size of
                                the flash array.  If specified, this
                                value *overrides* any which might be
                                specified in ffxconf.h.
        ulFlags               - The Device flags to use, which must
                                be zero or more of the following:
                              + FFXLOADER_DEV_USEBBM - Indicates that
                                the Device should be initialized with
                                BBM.  BBM will not be used otherwise.
                                This flag is only valid with NAND.
                              + FFXLOADER_DEV_UNLOCK - Indicates that
                                the Device should be automatically
                                unlocked at creation time.


    Return Value:
        Returns an FFXLOADERDEVHANDLE if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
FFXLOADERDEVHANDLE FfxLoaderDeviceCreate(
    FFXLOADERHANDLE     hLdr,
    unsigned            nDevNum,
    D_UINT32            ulExtraReservedLowKB,
    D_UINT32            ulExtraReservedHighKB,
    D_UINT32            ulMaxArraySizeKB,
    D_UINT32            ulFlags)
{
    DEVCREATEDATA       dcd = {0};
    FFXDEVCONFIG        conf;
    FFXSTATUS           ffxStat;
    FFXLDRDEVICEINFO   *pDev;

    FFXPRINTF(1, ("FfxLoaderDeviceCreate() hLdr=%P DEV%u ExtraLoResvKB=%lU ExtraHiResvKB=%lU MaxKB=%lU Flags=%lX\n",
        hLdr, nDevNum, ulExtraReservedLowKB, ulExtraReservedHighKB, ulMaxArraySizeKB, ulFlags));

    DclAssert(hLdr);

    if(ulFlags & FFXLOADER_DEV_RESERVEDMASK)
    {
        FFXPRINTF(1, ("FfxLoaderDeviceCreate() Invalid flags value %lX\n", ulFlags));
        return NULL;
    }

    dcd.ulExtraReservedLowKB    = ulExtraReservedLowKB;
    dcd.ulExtraReservedHighKB   = ulExtraReservedHighKB;
    dcd.ulMaxArraySizeKB        = ulMaxArraySizeKB;
    dcd.nDevNum                 = nDevNum;

    if(ulFlags & FFXLOADER_DEV_USEBBM)
    {
      #if FFXCONF_NANDSUPPORT
        dcd.fUseBBM = TRUE;
      #else
        FFXPRINTF(1, ("FfxLoaderDeviceCreate() FFXLOADER_DEV_USEBBM specified, but NAND is disabled\n"));
        return NULL;
      #endif
    }

    pDev = DclMemAllocZero(sizeof(*pDev));
    if(!pDev)
    {
        FFXPRINTF(1, ("FfxLoaderDeviceCreate() out of memory\n"));
        return NULL;
    }

    pDev->hLdr = hLdr;

    ffxStat = InitDevSettings(&conf, &dcd);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        FFXDEVHANDLE    hDev;

        FfxDriverLock(hLdr->pDI);

        if(ulFlags & FFXLOADER_DEV_UNLOCK)
        {
            /*  If we are being instructed to automatically unlock this
                Device, hook into the "options" interface chain at run-
                time to cause the option request to return TRUE.
            */                
            pfnOldOptionHandler = FfxDriverOptionHook(LoaderOptionGet);
            DclAssert(pfnOldOptionHandler);
        }
        
        hDev = FfxDriverDeviceCreateParam(hLdr->pDI, &conf, NULL);

        if((ulFlags & FFXLOADER_DEV_UNLOCK) && pfnOldOptionHandler)
        {
            /*  Uninstall the run-time hook
            */
            FfxDriverOptionHook(pfnOldOptionHandler);
        }

        FfxDriverUnlock(hLdr->pDI);

        if(hDev)
        {
            pDev->hDev = hDev;
            return pDev;
        }

        ffxStat = FFXSTAT_DEVICE_CREATEFAILED;
    }

    DclMemFree(pDev);

    FFXPRINTF(1, ("FfxLoaderDeviceCreate() DEV%u creation failed with Status %lX\n", nDevNum, ffxStat));

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderDeviceDestroy()

    Destroy a FlashFX Loader Device.

    Parameters:
        hLdrDev        - The Device handle

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderDeviceDestroy(
    FFXLOADERDEVHANDLE  hLdrDev)
{
    FFXSTATUS           ffxStat;
    
    DclAssert(hLdrDev);
    DclAssert(hLdrDev->hDev);

    FFXPRINTF(1, ("FfxLoaderDeviceDestroy() destroying LdrDev=%P Device=%P\n", hLdrDev, hLdrDev->hDev));

    ffxStat = FfxDriverDeviceDestroy(hLdrDev->hDev);

    DclMemFree(hLdrDev);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderDiskCreate()

    Create a FlashFX Loader Disk object using the supplied char-
    acteristics.  Note that Disks created with this interface do
    not use the standard FlashFX Device Project Hooks functions 
    FfxHookDiskCreate() and FfxHookDiskDestroy() as implemented in
    fhdisk.c.  All Disk configuration is done with this function,
    in combination with the settings in ffxconf.h, as described
    below.

    *Note!* -- The basic Disk settings values are retrieved from the
    ffxconf.h file, however the nDevNum value and ulFlags parameters
    specified in this interface will override the equivalent settings
    from the ffxconf.h file.

    Parameters:
        hLdr     - The loader instance handle
        nDiskNum - Number of the Disk to create
        nDevNum  - The Device number on which the Disk is to reside.
        ulFlags  - The Disk flags to use, which must be zero or
                   more of the following:
                 + FFXLOADER_DISK_USEALLOCATOR - Indicates that the
                   Disk should be initialized with the allocator.
                   It will not be used otherwise.
                 + FFXLOADER_DISK_SPANDEVICES - Indicates that the
                   Disk is allowed to span Devices if necessary.
                   This flag overrides the "SpanDevices" flag which
                   may be specified in ffxconf.h.
                 + FFXLOADER_DISK_ALTUNLOCK - Indicates that the
                   unlock command should ignore unlock requests for
                   blocks which are higher than previously requested
                   blocks.  See the notes for FfxLoaderUnlockBlocks().


    Return Value:
        Returns an FFXLOADERDISKHANDLE if successful, or NULL
        otherwise.
-------------------------------------------------------------------*/
FFXLOADERDISKHANDLE FfxLoaderDiskCreate(
    FFXLOADERHANDLE     hLdr,
    unsigned            nDiskNum,
    unsigned            nDevNum,
    D_UINT32            ulFlags)
{
    DISKCREATEDATA      dcd = {0};
    FFXDISKCONFIG       conf;
    FFXSTATUS           ffxStat;
    FFXLDRDISKINFO     *pDisk;

    FFXPRINTF(1, ("FfxLoaderDiskCreate() hLdr=%P DISK%u Flags=%lX\n", hLdr, nDiskNum, ulFlags));

    DclAssert(hLdr);

    if(ulFlags & FFXLOADER_DISK_RESERVEDMASK)
    {
        FFXPRINTF(1, ("FfxLoaderDiskCreate() Invalid flags value %lX\n", ulFlags));
        return NULL;
    }

    pDisk = DclMemAllocZero(sizeof(*pDisk));
    if(!pDisk)
    {
        FFXPRINTF(1, ("FfxLoaderDiskCreate() out of memory\n"));
        return NULL;
    }
    
    pDisk->hLdr = hLdr;

    if(ulFlags & FFXLOADER_DISK_ALTUNLOCK)
        pDisk->fAltUnlock = TRUE;

    if(ulFlags & FFXLOADER_DISK_SPANDEVICES)
        dcd.fSpanDevices = TRUE;

    dcd.nDiskNum = nDiskNum;
    dcd.nDevNum  = nDevNum;

    ffxStat = InitDiskSettings(&conf, &dcd);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        FFXDISKHANDLE   hDisk;

        FfxDriverLock(hLdr->pDI);

        hDisk = FfxDriverDiskCreateParam(hLdr->pDI, &conf, NULL);
        if(hDisk)
        {
          #if FFXCONF_DRIVERAUTOTEST
             FfxDriverUnitTest(hLdr->pDI->hDclInst, hDisk);
          #endif

          #if FFXCONF_ALLOCATORSUPPORT
            if(ulFlags & FFXLOADER_DISK_USEALLOCATOR)
            {
                /*-----------------------------------------------------------
                    Mount the allocator
                -----------------------------------------------------------*/
                ffxStat = FfxDriverAllocatorCreate(hDisk);
                if(ffxStat != FFXSTAT_SUCCESS)
                {
                    FFXPRINTF(1, ("FfxLoaderDiskCreate() Allocator initialization failed\n"));

                    FfxDriverDiskDestroy(hDisk);

                    hDisk = NULL;
                }
            }
          #else
            DclAssert(!(ulFlags & FFXLOADER_DISK_USEALLOCATOR));
          #endif
        }
        else
        {
            ffxStat = FFXSTAT_DISK_CREATEFAILED;
        }

        FfxDriverUnlock(hLdr->pDI);

        if(hDisk)
        {
            if(pDisk->fAltUnlock)
            {
                FFXFMLDEVINFO       fdi;
                
                ffxStat = FfxFmlDeviceInfo(nDevNum, &fdi);
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    if(fdi.nLockFlags & (FFXLOCKFLAGS_UNLOCKSINGLERANGE | FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
                    {
                        DclAssert(fdi.nLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS);
                        
                        pDisk->ulLowestUnlockedBlock = ALL_LOCKED;
                    }
                    else
                    {
                        /*  Turn the feature off if this is not the type of
                            locking scheme which requires it.
                        */    
                        pDisk->fAltUnlock = FALSE;
                    }
                }
                else
                {
                    DclError();
                    pDisk->fAltUnlock = FALSE;
                }
            }
            
            pDisk->hDisk = hDisk;
            return pDisk;
        }
    }

    DclMemFree(pDisk);

    FFXPRINTF(1, ("FfxLoaderDiskCreate() DISK%u creation failed with Status %lX\n", nDiskNum, ffxStat));

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderDiskDestroy()

    Destroy a FlashFX Loader Disk.

    Parameters:
        hLdrDisk       - The Loader Disk handle

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderDiskDestroy(
    FFXLOADERDISKHANDLE hLdrDisk)
{
    FFXSTATUS           ffxStat;
    
    DclAssert(hLdrDisk);
    DclAssert(hLdrDisk->hDisk);

    FFXPRINTF(1, ("FfxLoaderDiskDestroy() destroying LdrDisk=%P Disk=%P\n", hLdrDisk, hLdrDisk->hDisk));

    ffxStat = FfxDriverDiskDestroy(hLdrDisk->hDisk);

    DclMemFree(hLdrDisk);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderEraseBlocks()

    Erases blocks from the flash array at the specified logical
    block.

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartBlock    - Logical block number for first block
        ulCount         - Count of the blocks

    Return Value:
        Returns number of blocks erased.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderEraseBlocks(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStatus;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "FfxLoaderEraseBlocks() hLdrDisk=%P Erasing %lU blocks at %lU\n", 
        hLdrDisk, ulCount, ulStartBlock));

    FMLERASE_BLOCKS(pDisk->hFML, ulStartBlock, ulCount, ioStatus);

    return ioStatus.ulCount;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderLockFreeze()

    Freeze the lock state for a range of erase blocks.

    *Warning!* -- Locking APIs must be used with extreme caution, as
                  some flash types apply locking/unlocking operations
                  to the entire chip.  FlashFX Disk boundaries cannot
                  apply in this case.  

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartBlock    - Logical block number for first block
        ulCount         - Count of the blocks

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderLockFreeze(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount)
 {
    FFXIOSTATUS         ioStatus;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
        "FfxLoaderLockFreeze() hLdrDisk=%P locking %lU blocks at %lU\n", hLdrDisk, ulCount, ulStartBlock));

    FMLLOCK_FREEZE(pDisk->hFML, ulStartBlock, ulCount, ioStatus);

    return ioStatus.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderLockBlocks()

    Lock a range of erase blocks.

    *Warning!* -- Locking APIs must be used with extreme caution, as
                  some flash types apply locking/unlocking operations
                  to the entire chip.  FlashFX Disk boundaries cannot
                  apply in this case.  

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartBlock    - Logical block number for first block
        ulCount         - Count of the blocks

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderLockBlocks(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount)
 {
    FFXIOSTATUS         ioStatus;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
        "FfxLoaderLockBlocks() hLdrDisk=%P locking %lU blocks at %lU\n", 
        hLdrDisk, ulCount, ulStartBlock));

    FMLLOCK_BLOCKS(pDisk->hFML, ulStartBlock, ulCount, ioStatus);

    if(hLdrDisk->fAltUnlock && (ioStatus.ffxStat == FFXSTAT_SUCCESS))
        hLdrDisk->ulLowestUnlockedBlock = ALL_LOCKED;

    return ioStatus.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderUnlockBlocks()

    Unlock a range of erase blocks.

    *Warning!* -- Locking APIs must be used with extreme caution, as
                  some flash types apply locking/unlocking operations
                  to the entire chip.  FlashFX Disk boundaries cannot
                  apply in this case.  

    For flash which only supports unlocking a single range of erase
    blocks, this function tracks the lowest erase block that is 
    unlocked.  Some software (unnamed) has no knowledge of the single
    range erase characteristic and iteratively tries unlocking most 
    if not all the flash, one block at a time, resulting in all the
    flash blocks being locked, except the very last block.

    This depends on the behavior of FlashFX which requires that all
    BBM blocks be unlocked, and therefore, since only one range can
    be unlocked, everything from that lowest block to the end of the
    flash must be unlocked.  

    Note that this special behavior is only performed if the fInvert
    flag is FALSE.

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartBlock    - Logical block number for first block
        ulCount         - Count of the blocks
        fInvert         - Indicates that the inverse of the specified
                          range should be unlocked.

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderUnlockBlocks(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartBlock,
    D_UINT32            ulCount,
    D_BOOL              fInvert)
{
    FFXIOSTATUS         ioStatus;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, 0),
        "FfxLoaderUnlockBlocks() hLdrDisk=%P Unlocking %lU blocks at %lU, Invert=%u\n", 
        hLdrDisk, ulCount, ulStartBlock, fInvert));

    if(hLdrDisk->fAltUnlock && !fInvert)
    {
        if((hLdrDisk->ulLowestUnlockedBlock != ALL_LOCKED) && (ulStartBlock >= hLdrDisk->ulLowestUnlockedBlock))
        {
            FFXPRINTF(1, ("FfxLoaderUnlockBlocks() Ignoring unlock request for an already unlocked range (Start=%lU Count=%lU)\n", ulStartBlock, ulCount));
            return FFXSTAT_SUCCESS;
        }
        else
        {
            FMLUNLOCK_BLOCKS(pDisk->hFML, ulStartBlock, ulCount, fInvert, ioStatus);

            if(ioStatus.ffxStat == FFXSTAT_SUCCESS) 
                hLdrDisk->ulLowestUnlockedBlock = ulStartBlock;
        }
    }
    else
    {
        FMLUNLOCK_BLOCKS(pDisk->hFML, ulStartBlock, ulCount, fInvert, ioStatus);
    }
    
    return ioStatus.ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderGetDeviceInfo()

    Get information about a FlashFX Device.

    Parameters:
        hLdrDev  - The Loader Device handle
        pDevInfo - A pointer to the FFXLOADERDEVINFO structure to fill.

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderGetDeviceInfo(
    FFXLOADERDEVHANDLE  hLdrDev,
    FFXLOADERDEVINFO   *pDevInfo)
{
    FFXFMLDEVINFO       fdi;
    FFXDEVHANDLE        hIntDev = hLdrDev->hDev;
    FFXSTATUS           ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "FfxLoaderGetDeviceInfo() hLdrDev=%P getting info for Device %P\n",
        hLdrDev, hIntDev));

    ffxStat = FfxFmlDeviceInfo((*hIntDev)->Conf.nDevNum, &fdi);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxLoaderGetDeviceInfo() failed with Status %lX\n", ffxStat));
    }
    else
    {
        DclMemSet(pDevInfo, 0, sizeof(*pDevInfo));
        
        pDevInfo->ulBlockSize       = fdi.ulBlockSize;
        pDevInfo->ulReservedBlocks  = fdi.ulReservedBlocks;
        pDevInfo->ulRawBlocks       = fdi.ulRawBlocks;
        pDevInfo->ulTotalBlocks     = fdi.ulTotalBlocks;
        pDevInfo->ulChipBlocks      = fdi.ulChipBlocks;
        pDevInfo->nPageSize         = (unsigned)fdi.uPageSize;
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderGetDiskInfo()

    Get information about a FlashFX Disk.

    Parameters:
        hLdrDisk    - The Loader Disk handle.
        pInfo       - The Disk information

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxLoaderGetDiskInfo(
    FFXLOADERDISKHANDLE hLdrDisk,
    FFXLOADERDISKINFO  *pInfo)
{
    FFXFMLINFO          fmlInfo;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;
    FFXSTATUS           ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, 0),
        "FfxLoaderGetDiskInfo() hLdrDisk=%P Getting Disk info\n", hLdrDisk));

    ffxStat = FfxFmlDiskInfo(pDisk->hFML, &fmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxLoaderGetDiskInfo() failed with Status %lX\n", ffxStat));
    }
    else
    {
        DclMemSet(pInfo, 0, sizeof(*pInfo));
        
        pInfo->ulTotalBlocks = fmlInfo.ulTotalBlocks;
        pInfo->ulBytesPerBlock = fmlInfo.ulBlockSize;

        /*  For now, we're assuming that one sector is equal to one page
        */
        pInfo->nSectorsPerBlock = (unsigned)(fmlInfo.ulBlockSize / fmlInfo.uPageSize);
        pInfo->nBytesPerSector = fmlInfo.uPageSize;

        if(fmlInfo.uDeviceType & DEVTYPE_NAND)
            pInfo->fIsNAND = TRUE;

      #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
        /*  ulAlocatorSectors is only valid if an allocator is being used.
            Otherwise it is zero, and the total FML pages is determined by
            multiplying ulTotalBlocks * nSectorsPerBlock.
        */            
        if(pDisk->hVBF && pDisk->fpdi.ulTotalSectors)
            pInfo->ulAllocatorSectors = pDisk->fpdi.ulTotalSectors;
      #endif
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderGetBlockStatus()

    Get the block status for the specified erase block.  This function
    is only available on NAND, and will return UINT_MAX if it is some
    other flash type.

    *Note* -- This returns status of "Raw" blocks -- unmapped by BBM.

    Parameters:
        hLdrDisk     - The Loader Disk handle.
        ulBlockNum   - Block number

    Return Value:
        Returns 1 if the block is good, 0 if the block is bad, and
        UINT_MAX if the block status could not be read.
-------------------------------------------------------------------*/
unsigned FfxLoaderGetBlockStatus(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulBlockNum)
{
    unsigned            nBlockStatus = UINT_MAX;
  #if FFXCONF_NANDSUPPORT
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    if(pDisk->fIsNAND)
    {
        FFXIOSTATUS ioStat;

        FML_GET_RAW_BLOCK_STATUS(pDisk->hFML, ulBlockNum, ioStat);
        if(IOSUCCESS(ioStat, 1))
        {
            if((ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) == BLOCKSTATUS_NOTBAD)
                nBlockStatus = 1;
            else
                nBlockStatus = 0;
        }
    }
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, 0),
        "FfxLoaderGetBlockStatus() hLdrDisk=%P for block %lU returning %u\n", 
        hLdrDisk, ulBlockNum, nBlockStatus));

    return nBlockStatus;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderReadFmlPages()

    Read sectors from the flash array at the specified logical
    sector location.

    Parameters:
        hLdrDisk        - The Loader Disk handle.
        ulStartPage     - The starting page
        ulCount         - Count of the pages
        pBuffer         - Buffer to store the read data
        pTags           - An optional pointer to a buffer in which
                          tags will be stored.  Only used if the
                          flash is NAND, and must be NULL otherwise.
        nTagSize        - The size of each tag.  Must be non-zero
                          if pTags is not NULL.

    Return Value:
        Returns number of pages read.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderReadFmlPages(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    D_BUFFER           *pBuffer,
	D_BUFFER           *pTags,
	unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxLoaderReadFmlPages() hLdrDisk=%P Start=%lU Count=%lU pBuff=%P pTags=%P TagSize=%u\n",
        hLdrDisk, ulStartPage, ulCount, pBuffer, pTags, nTagSize));

  #if FFXCONF_NANDSUPPORT
    if(pDisk->fIsNAND && pTags)
    {
        DclAssert(nTagSize);

        FMLREAD_TAGGEDPAGES(pDisk->hFML, ulStartPage, ulCount, pBuffer, pTags, nTagSize, ioStat);
    }
    else
  #endif
    {
        DclAssert(!nTagSize && !pTags);

        FMLREAD_PAGES(pDisk->hFML, ulStartPage, ulCount, pBuffer, ioStat);
    }
        
    FFXTRACEPRINTF(
        (ioStat.ffxStat == FFXSTAT_SUCCESS ? MAKETRACEFLAGS( FFXTRACE_DRIVER, 2, TRACEUNDENT)
                                           : MAKETRACEFLAGS( FFXTRACE_ALWAYS, 1, TRACEUNDENT),
        "FfxLoaderReadFmlPages() Start=%lU returning %s\n", ulStartPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat.ulCount;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderWriteFmlPages()

    Write sectors to the flash array at the specified logical
    sector location.

    Parameters:
        hLdrDisk        - The Loader Disk handle.
        ulStartPage     - The starting page
        ulCount         - Count of the pages
        pBuffer         - Data to write
        pTags           - An optional pointer to a buffer containing
                          the tags.  Only used if the flash is NAND,
                          and must be NULL otherwise.
        nTagSize        - The size of each tag.  Must be non-zero
                          if pTags is not NULL.

    Return Value:
        Returns number of pages written.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderWriteFmlPages(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    const D_BUFFER     *pBuffer,
	const D_BUFFER     *pTags,
	unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat;
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxLoaderWriteFmlPages() hLdrDisk=%P Start=%lU Count=%lU pBuff=%P pTags=%P TagSize=%u\n",
        hLdrDisk, ulStartPage, ulCount, pBuffer, pTags, nTagSize));

  #if FFXCONF_NANDSUPPORT
    if(pDisk->fIsNAND && pTags)
    {
        DclAssert(nTagSize);

        FMLWRITE_TAGGEDPAGES(pDisk->hFML, ulStartPage, ulCount, pBuffer, pTags, nTagSize, ioStat);
    }
    else
  #endif
    {
        DclAssert(!nTagSize && !pTags);

        FMLWRITE_PAGES(pDisk->hFML, ulStartPage, ulCount, pBuffer, ioStat);
    }

    FFXTRACEPRINTF(
        (ioStat.ffxStat == FFXSTAT_SUCCESS ? MAKETRACEFLAGS( FFXTRACE_DRIVER, 2, TRACEUNDENT)
                                           : MAKETRACEFLAGS( FFXTRACE_ALWAYS, 1, TRACEUNDENT),
        "FfxLoaderWriteFmlPages() Start=%lU returning %s\n", ulStartPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat.ulCount;
}


/*-------------------------------------------------------------------
    FfxLoaderReadTags()

    Read one or more tags into a buffer.  This function is only
    valid when used on NAND flash.

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartPage     - The start page
        ulCount         - Count of the tags
        pTagBuffer      - A pointer to a buffer in which tags will
                          be stored.
        nTagSize        - The size of each tag.

    Return Value:
        Returns number of tags read.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderReadTags(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
	D_BUFFER           *pTagBuffer,
	unsigned            nTagSize)
{
  #if FFXCONF_NANDSUPPORT
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;
  #endif
    D_UINT32            ulRead = 0;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxLoaderReadTags() hLdrDisk=%P Start=%lU Count=%lU TagSize=%u\n",
        hLdrDisk, ulStartPage, ulCount, nTagSize));

    DclAssert(hLdrDisk);
    DclAssert(ulCount);
    DclAssert(pTagBuffer);
    DclAssert(nTagSize);

  #if FFXCONF_NANDSUPPORT
    if(pDisk->fIsNAND)
    {
        FFXIOSTATUS ioStat;

        FMLREAD_TAGS(pDisk->hFML, ulStartPage, ulCount, pTagBuffer, nTagSize, ioStat);

        ulRead = ioStat.ulCount;
    }
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEUNDENT),
        "FfxLoaderReadTags() returning %lU\n", ulRead));

    return ulRead;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderReadSectors()

    Read sectors from the flash array at the specified logical
    sector location.

    *Note* -- This function is usable only if the allocator (VBF)
              is enabled, and the Disk is formatted and mounted.
              Allocator support (FFXCONF_ALLOCATORSUPPORT) is often
              disabled in boot loaders.

    Parameters:
        hLdrDisk        - The Loader Disk handle.
        ulStartSector   - The starting sector
        ulCount         - Count of the sectors
        pBuffer         - Buffer to store the read data

    Return Value:
        Returns number of sectors read.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderReadSectors(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartSector,
    D_UINT32            ulCount,
    D_BUFFER           *pBuffer)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxLoaderReadSectors() hLdrDisk=%P Start=%lU Count=%lU pBuff=%P\n", 
        hLdrDisk, ulStartSector, ulCount, pBuffer));

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDisk->hVBF)
    {
        ioStat.ulCount = FfxDriverSectorRead(hLdrDisk->hDisk, ulStartSector, ulCount, pBuffer);
    }
    else
  #endif
    {
        /*  Code which uses this variable may be conditioned out.
        */
        (void)pDisk;

        ioStat.ffxStat = FFXSTAT_LOADER_NOALLOCATOR;
    }
      
    FFXTRACEPRINTF(
        (ioStat.ffxStat == FFXSTAT_SUCCESS ? MAKETRACEFLAGS( FFXTRACE_DRIVER, 2, TRACEUNDENT)
                                           : MAKETRACEFLAGS( FFXTRACE_ALWAYS, 1, TRACEUNDENT),
        "FfxLoaderReadSectors() Start=%lU returning %s\n", ulStartSector, FfxDecodeIOStatus(&ioStat)));

    return ioStat.ulCount;
}


/*-------------------------------------------------------------------
    Public: FfxLoaderWriteSectors()

    Write sectors to the flash array at the specified logical
    sector location.

    *Note* -- This function is usable only if the allocator (VBF)
              is enabled, and the Disk is formatted and mounted.
              Allocator support (FFXCONF_ALLOCATORSUPPORT) is often
              disabled in boot loaders.

    Parameters:
        hLdrDisk        - The Loader Disk handle
        ulStartSector   - The starting sector
        ulCount         - Count of the sectors
        pBuffer         - Data to write

    Return Value:
        Returns number of sectors written.
-------------------------------------------------------------------*/
D_UINT32 FfxLoaderWriteSectors(
    FFXLOADERDISKHANDLE hLdrDisk,
    D_UINT32            ulStartSector,
    D_UINT32            ulCount,
    const D_BUFFER     *pBuffer)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    FFXDISKINFO        *pDisk = *hLdrDisk->hDisk;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxLoaderWriteSectors() hLdrDisk=%P Start=%lU Count=%lU pBuff=%P\n", 
        hLdrDisk, ulStartSector, ulCount, pBuffer));

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDisk->hVBF)
    {
        ioStat.ulCount = FfxDriverSectorWrite(hLdrDisk->hDisk, ulStartSector, ulCount, pBuffer);
    }
    else
  #endif
    {
        /*  Code which uses this variable may be conditioned out.
        */
        (void)pDisk;

        ioStat.ffxStat = FFXSTAT_LOADER_NOALLOCATOR;
    }
      
    FFXTRACEPRINTF(
        (ioStat.ffxStat == FFXSTAT_SUCCESS ? MAKETRACEFLAGS( FFXTRACE_DRIVER, 2, TRACEUNDENT)
                                           : MAKETRACEFLAGS( FFXTRACE_ALWAYS, 1, TRACEUNDENT),
        "FfxLoaderWriteSectors() Start=%lU returning %s\n", ulStartSector, FfxDecodeIOStatus(&ioStat)));

    return ioStat.ulCount;
}


/*-------------------------------------------------------------------
    Local: InitDevSettings()

    This function is used to initialize Device parameters.

    Upon successful completion, this function will fill in the
    pConf->nDevNum field with the appropriate DEVn number.

    Parameters:
        pConf     - A pointer to a FFXDEVCONFIG structure to fill.
        pDevData  - A pointer to the DEVCREATEDATA structure to use

    Return Value:
        Returns a standard FFXSTATUS code.
-------------------------------------------------------------------*/
static FFXSTATUS InitDevSettings(
    FFXDEVCONFIG           *pConf,
    const DEVCREATEDATA    *pDevData)
{
    unsigned                nDevNum;
    FFXSTATUS               ffxStat;

    DclAssert(pConf);
    DclAssert(pDevData);

    DclMemSet(pConf, 0, sizeof(*pConf));

    nDevNum = pDevData->nDevNum;
    if(nDevNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("DEV%u is out of range (max=%u)\n", nDevNum, FFX_MAX_DEVICES-1));
        return FFXSTAT_DEVICENUMBERINVALID;
    }

    /*  The pConf structure must contain the Device number being configured.
    */
    pConf->nDevNum = pDevData->nDevNum;

    /*  Grab the DEV settings, as defined in ffxconf.h
    */
    ffxStat = FfxDriverDeviceSettings(pConf);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Now overlay the special settings we want for this Device
        */
        pConf->DevSettings.ulReservedBottomKB += pDevData->ulExtraReservedLowKB;
        pConf->DevSettings.ulReservedTopKB    += pDevData->ulExtraReservedHighKB;

        /*  If the value is zero, just use the value as specified in the
            ffxconf.h file.
        */
        if(pDevData->ulMaxArraySizeKB)
            pConf->DevSettings.ulMaxArraySizeKB = pDevData->ulMaxArraySizeKB;

      #if FFXCONF_BBMSUPPORT
        /*  Grab the BBM settings, as defined in ffxconf.h
        */
        ffxStat = FfxDriverDeviceBbmSettings(pConf);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            /*  Now overlay the special settings we want for this Device
            */
            pConf->BbmSettings.fEnableBBM = pDevData->fUseBBM;
        }
      #endif
    }

    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("DEV%u settings initialization failed, Status=%lX\n", nDevNum, ffxStat));
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: InitDiskSettings()

    This function is used to initialize Disk parameters.

    Upon successful completion, this function will fill in the
    pConf->nDiskNum field with the appropriate DISKn number.

    Parameters:
        pConf     - A pointer to a FFXDISKCONFIG structure to fill.
        pDiskData - A pointer to the DISKCREATEDATA structure to use

    Return Value:
        Returns a standard FFXSTATUS code.
-------------------------------------------------------------------*/
static FFXSTATUS InitDiskSettings(
    FFXDISKCONFIG  *pConf,
    DISKCREATEDATA *pDiskData)
{
    unsigned        nDiskNum;
    FFXSTATUS       ffxStat;

    DclAssert(pConf);
    DclAssert(pDiskData);

    DclMemSet(pConf, 0, sizeof(*pConf));

    nDiskNum = pDiskData->nDiskNum;

    if(nDiskNum >= FFX_MAX_DISKS)
    {
        FFXPRINTF(1, ("DISK%u is out of range (max=%u)\n", nDiskNum, FFX_MAX_DISKS-1));
        return FFXSTAT_DISKNUMBERINVALID;
    }

    /*  Stuff the DiskNum into the pConf structure, which is necessary so
        FfxDriverDiskSettings() knows which Disk is being initialized.
    */
    pConf->nDiskNum = nDiskNum;

    /*  Grab the Disk settings, as defined in ffxconf.h
    */
    ffxStat = FfxDriverDiskSettings(pConf);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Now overlay the special settings we want for this Disk
        */
        pConf->DiskSettings.nDevNum = pDiskData->nDevNum;

        /*  The supplied "SpanDevices" flag overrides any value which
            may be specified in the ffxconf.h file.
        */
        if(pDiskData->fSpanDevices)
            pConf->DiskSettings.nFlags |= FFX_DISK_SPANDEVICES;
        else
            pConf->DiskSettings.nFlags &= ~FFX_DISK_SPANDEVICES;
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: LoaderOptionGet()

    Get a configuration option.  This function is normally only
    called from FfxHookOptionGet() to obtain an option that is
    standard for all projects using a given OS.

    This function will call any installed options hook functions
    which may be in place.  If none have been installed then the
    function FfxDriverFWOptionGet() will be called.

    Parameters:
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return Value:
        TRUE if the option identifier is valid, the option value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL LoaderOptionGet(
    FFXOPTION       opt,
    void           *handle,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    D_BOOL          fResult = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "LoaderOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    switch (opt)
    {
        case FFXOPT_DEVICE_AUTOUNLOCK:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                DclAssert(ulBuffLen == sizeof(D_BOOL));
                *(D_BOOL*)pBuffer = TRUE;
            }
            fResult = TRUE;
            break;

        default:

            /*  Other parameter codes may be recognized by FlashFX.
            */
            DclAssert(pfnOldOptionHandler);
            fResult = (*pfnOldOptionHandler)(opt, handle, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "LoaderOptionGet() returning %U\n", fResult));

    return fResult;
}

