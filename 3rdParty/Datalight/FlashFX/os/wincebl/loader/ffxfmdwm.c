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

    This module contains code which may be used to plug FlashFX into a
    Windows CE or WinMobile boot loader as an FMD.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxfmdwm.c $
    Revision 1.9  2010/07/17 23:50:22Z  garyp
    Updated so the "AltUnlock" functionality is optional, and must be
    explicitly turned on.
    Revision 1.8  2010/07/06 02:37:39Z  garyp
    Updated to use some modified Loader functions.
    Revision 1.7  2009/12/11 23:12:35Z  garyp
    Eliminated the AUTO_UNLOCK_BLOCKS feature.  The capability is now
    specified at a higher level, and implemented at a lower level.
    Revision 1.6  2009/11/11 00:46:54Z  garyp
    Eliminated use of the C library memset() function.
    Revision 1.5  2009/07/22 01:21:49Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FfxLoaderDiskCreate()
    which now takes a Device parameter. Updated the FfxWM_FMD_OEMIoControl()
    function to handle IOCTL_FMD_FLASHFX to allow FMD clients to issue FlashFX
    specific IOCTL requests.  Updated the IOCTL_FMD_LOCK_BLOCKS command to issue
    a lock first, then the lock-freeze.  Updated to use the generalized FlashFX
    Loader interface, rather than the deprecated CE Loader.  Updated to allow
    multiple Devices to be created in the event that the Disk is specified with
    the "SpanDevices" flag.  Added the AUTO_UNLOCK_BLOCKS setting.  Updated
    to create a DCL instance for the driver.  Redirect input and output using 
    services.  Modified so multi-sector read/write requests are supported.  
    Modified FfxWM_FMD_GetBlockStatus() so the block remapping is done outside
    the code which is conditioned with FFXCONF_USESECTORINFO.  Modified the
    "lock" IOCTL to use the new "lock-freeze" API.  Modified to honor the 
    FFXCONF_USESECTORINFO setting and ignore SectorInfo if it is FALSE.  Updated
    to support writes where pSectorInfo is NULL.  Implemented the lock/unlock 
    blocks IOCTLs.  Updated to unlock all the flash when EBOOT first comes up.
    Revision 1.4  2009/02/09 02:15:31Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables 
    -- no functional changes.
    Revision 1.3  2008/05/09 03:22:04Z  garyp
    Updated to work with output disabled.
    Revision 1.2  2008/05/03 21:44:13Z  garyp
    Modified to build only with CE versions later than 5.0.
    Revision 1.1  2008/03/18 20:31:38Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

/*  This code only works with CE and WinMobile versions later than 5.0
*/
#if _WIN32_WCE > 500

#include <fmd.h>
#include <nkintr.h>     /* for OEMWriteDebugString() */

#include <flashfx.h>
#include <flashfx_wincebl.h>
#include <dlinstance.h>
#include <dlservice.h>
#include <fxwincommon.h>
#include <fxloaderapi.h>
#include <fxapireq.h>

#define RANGE_NONE      (0)
#define RANGE_1         (1)
#define RANGE_2         (2)

typedef struct
{
    DCLINSTANCEHANDLE   hDclInst;
  #if DCLCONF_OUTPUT_ENABLED
	DCLOUTPUTINFO       oi;
    DCLSERVICEHEADER    SrvOut;
  #endif
    FFXLOADERHANDLE     hLdr;
    FFXLOADERDEVHANDLE  ahDev[FFX_MAX_DEVICES];
    FFXLOADERDISKHANDLE hDisk;
    FFXWMFMDRANGE       RangeInfo1;
    FFXWMFMDRANGE       RangeInfo2;
    FFXWMFMDRANGE      *pCurrentRange;
    FFXLOADERDISKINFO   DiskInfo;           /* Disk info for the full disk */
    FFXLOADERDISKINFO   CurrentDiskInfo;    /* Disk info for the current range */
    D_UINT32            ulRange2ReservedSectors;
    D_UINT32            ulRange2TotalSectors;
    unsigned            nDeviceCount;
    unsigned            nUsageCount;
    unsigned            nCurrentRange;
    unsigned            fRange1Inited : 1;
    unsigned            fRange2Inited : 1;
    unsigned            fIsEBOOT : 1;
} DLFMDINFO;

/*  Silliness that the FMD inferace does not pass this context
    to each function...
*/
static DLFMDINFO   *gpDLFMD;

static void     OutputAString(  void *pContext, const char *pszString);
static void     OutputIPLString(void *pContext, const char *pszString);
static D_UINT32 RemapBlock(    DLFMDINFO *pDLFMD, D_UINT32 ulBlockNum);
static D_UINT32 RemapSector(   DLFMDINFO *pDLFMD, D_UINT32 ulSectorNum);
static D_BOOL   SelectRange(   DLFMDINFO *pDLFMD, D_UINT32 ulSectorNum);
static D_BOOL   CreateDevices( DLFMDINFO *pDLFMD, FFXWMFMDRANGE *pRange);
static D_BOOL   DestroyDevices(DLFMDINFO *pDLFMD);
#if D_DEBUG
static void     DumpRangeInfo(unsigned nRangeNum, FFXWMFMDRANGE *pRange);
#endif


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_Init()

    This function is called to initialize flash subsystem.
-------------------------------------------------------------------*/
PVOID FfxWM_FMD_Init(
    LPCTSTR                 pActive,
    PPCI_REG_INFO           pRegIn,
    PPCI_REG_INFO           pRegOut)
{
    DLFMDINFO              *pDLFMD;
    DCLINSTANCEHANDLE       hDclInst;

    if(gpDLFMD)
    {
        DclPrintf("Datalight FlashFX FMD Interface already initialized\n");

        gpDLFMD->nUsageCount++;

        return gpDLFMD;
    }

    /*  Create a DCL Instance before doing <anything> else.  This will
        initialize the default services, as defined in the project.
    */
    if(DclInstanceCreate(0, DCLFLAG_DRIVER, &hDclInst) != DCLSTAT_SUCCESS)
        return NULL;

    pDLFMD = DclMemAllocZero(sizeof(*pDLFMD));
    if(!pDLFMD)
    {
        DclInstanceDestroy(hDclInst);
        return NULL;
    }

    pDLFMD->hDclInst = hDclInst;
    pDLFMD->nUsageCount++;

    if(pActive)
        pDLFMD->fIsEBOOT = TRUE;

  #if DCLCONF_OUTPUT_ENABLED
    {
        DCLSTATUS           dclStat;
        DCLSERVICEHEADER   *pSrv;

        if(pActive)
            pDLFMD->oi.pfnOutputString = OutputAString;
        else
            pDLFMD->oi.pfnOutputString = OutputIPLString;

        /*  Register the output service to use
        */
        pSrv = &pDLFMD->SrvOut;
        dclStat = DclServiceHeaderInit(&pSrv, "DLFMDOUTPUT", DCLSERVICE_OUTPUT, NULL, &pDLFMD->oi, 0);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            dclStat = DclServiceCreate(hDclInst, pSrv);
        }

        /*  If it failed, try to print out a message using the previous service
        */
        if(dclStat != DCLSTAT_SUCCESS)
            DclPrintf("Output service initialization failed with status code %lX\n", dclStat);

        /*  Even though the output service failed to initialize, go ahead
            and keep trying to load...
        */
    }
  #endif

    DclPrintf("Datalight FlashFX FMD Interface for Windows CE and WinMobile\n");

  #if D_DEBUG
    if(pActive)
    {
        /*  The string to print is unicode, but we don't have any unicode-
            to-ascii support in this mode, so manually convert it ourselves.
        */
        FFXPRINTF(1, ("pActive="));

        while(*pActive)
        {
            char    acChr[2];

            acChr[0] = *(char*)pActive;
            acChr[1] = 0;

            FFXPRINTF(1, ("%s", acChr));

            pActive++;
        }

        FFXPRINTF(1, (" pRegIn=%P pRegOut=%P\n", pRegIn, pRegOut));


    }
    else
    {
        FFXPRINTF(1, ("pActive=IPL pRegIn=%P pRegOut=%P\n", pRegIn, pRegOut));
    }
  #endif

    pDLFMD->hLdr = FfxLoaderCreate(hDclInst);
    if(pDLFMD->hLdr)
    {
        FFXPRINTF(1, ("WM FMD initialization successful\n"));

        gpDLFMD = pDLFMD;

        return pDLFMD;
    }
    else
    {
        FfxWM_FMD_Deinit(pDLFMD);

        return NULL;
    }
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_InitRanges()

    This function is called to initialize flash subsystem.
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_InitRanges(
    VOID                   *pContext,
    FFXWMFMDRANGE          *pWMFMDRange1,
    FFXWMFMDRANGE          *pWMFMDRange2)
{
    DLFMDINFO              *pDLFMD = pContext;
    BOOL                    fSuccess = FALSE;
    D_UINT32                ulFlags = 0;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxWM_FMD_InitRanges() pFMD=%P pRange1=%P pRange2=%P\n", pDLFMD, pWMFMDRange1, pWMFMDRange2));

    DclAssert(pDLFMD);
    DclAssert(pDLFMD == gpDLFMD);
    DclAssert(pWMFMDRange1);

    /*  Can't be using a remap table AND BBM.  One or the other, (or both)
        must be disabled.
    */
    DclAssert(!pWMFMDRange1->nRemapTableSize || !pWMFMDRange1->fUseBBM);

    /*  Destroy any previous Disk and Device instances we may have
        already had set up.
    */
    if(pDLFMD->nCurrentRange != RANGE_NONE)
    {
        FFXPRINTF(1, ("FfxWM_FMD_InitRanges() destroying current Range%u Disks and Devices\n",
            pDLFMD->nCurrentRange));

        DclAssert(pDLFMD->hDisk);
        DclAssert(pDLFMD->ahDev[0]);

        FfxLoaderDiskDestroy(pDLFMD->hDisk);
        pDLFMD->hDisk = NULL;

        DestroyDevices(pDLFMD);

        pDLFMD->nCurrentRange = RANGE_NONE;
    }

  #if D_DEBUG
    DumpRangeInfo(1, pWMFMDRange1);
  #endif

    pDLFMD->RangeInfo1 = *pWMFMDRange1;
    pDLFMD->fRange1Inited = TRUE;

    if(pWMFMDRange2)
    {
      #if D_DEBUG
        DumpRangeInfo(2, pWMFMDRange2);
      #endif

        /*  Can't be using a remap table AND BBM.  One or the other, (or both)
            must be disabled.
        */
        DclAssert(!pWMFMDRange2->nRemapTableSize || !pWMFMDRange2->fUseBBM);

        pDLFMD->RangeInfo2 = *pWMFMDRange2;
        pDLFMD->fRange2Inited = TRUE;
    }
    else
    {
        /*  Since the InitRanges() function may be called multiple
            times, we must not assume that these fields are already
            set to FALSE or 0.
        */
        pDLFMD->fRange2Inited = FALSE;
        pDLFMD->ulRange2ReservedSectors = 0;
        pDLFMD->ulRange2TotalSectors = 0;
    }

    if(!CreateDevices(pDLFMD, pWMFMDRange1))
        goto InitRangeCleanup;

    if(pWMFMDRange1->fUseAllocator)
        ulFlags |= FFXLOADER_DISK_USEALLOCATOR;

    if(pWMFMDRange1->fSpanDevices)
        ulFlags |= FFXLOADER_DISK_SPANDEVICES;

    if(pWMFMDRange1->fAltUnlock)
        ulFlags |= FFXLOADER_DISK_ALTUNLOCK;

    pDLFMD->hDisk = FfxLoaderDiskCreate(pDLFMD->hLdr, pWMFMDRange1->nDiskNum, pWMFMDRange1->nDeviceNum, ulFlags);
    if(!pDLFMD->hDisk)
        goto InitRangeCleanup;

    if(FfxLoaderGetDiskInfo(pDLFMD->hDisk, &pDLFMD->DiskInfo) != FFXSTAT_SUCCESS)
        goto InitRangeCleanup;

    pDLFMD->CurrentDiskInfo = pDLFMD->DiskInfo;

    if(pDLFMD->fRange2Inited)
    {
        D_UINT32    ulTempBlocks;

        /*  Calculate the reserved sectors preceding RANGE2
        */
        pDLFMD->ulRange2ReservedSectors = (pWMFMDRange2->ulExtraReservedLowKB * 1024) / pDLFMD->DiskInfo.nBytesPerSector;

        ulTempBlocks = ((pWMFMDRange2->ulExtraReservedLowKB + pWMFMDRange2->ulExtraReservedHighKB) * 1024) / pDLFMD->DiskInfo.ulBytesPerBlock;
        ulTempBlocks = pDLFMD->DiskInfo.ulTotalBlocks - ulTempBlocks;

        /*  Calculate the total sectors in RANGE2
        */
        pDLFMD->ulRange2TotalSectors = ulTempBlocks * pDLFMD->DiskInfo.nSectorsPerBlock;

        FFXPRINTF(1, ("FfxWM_FMD_InitRanges() Range2 ReservedSectors=%lU TotalSectors=%lU\n",
            pDLFMD->ulRange2ReservedSectors, pDLFMD->ulRange2TotalSectors));
    }

    pDLFMD->nCurrentRange = RANGE_1;
    pDLFMD->pCurrentRange = &pDLFMD->RangeInfo1;

    fSuccess = TRUE;

  InitRangeCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FfxWM_FMD_InitRanges() returning %u\n", fSuccess));

    return fSuccess;
}


/*-------------------------------------------------------------------
    Public:  FMD_Deinit()
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_Deinit(
    VOID               *pContext)
{
    DLFMDINFO          *pDLFMD = pContext;
    DCLINSTANCEHANDLE   hDclInst;

    FFXPRINTF(1, ("FfxWM_FMD_Deinit()\n"));

    DclAssert(pDLFMD == gpDLFMD);

    pDLFMD->nUsageCount--;

    if(pDLFMD->nUsageCount)
    {
        FFXPRINTF(1, ("Reduced usage count, loader still installed\n"));

        return TRUE;
    }

    /*  Deallocate resources in reverse order of allocation
    */

    if(pDLFMD->hDisk)
        FfxLoaderDiskDestroy(pDLFMD->hDisk);

    if(pDLFMD->ahDev[0])
        DestroyDevices(pDLFMD);

    if(pDLFMD->hLdr)
        FfxLoaderDestroy(pDLFMD->hLdr);

    /*  Destroy any transient services we may have created.  Project
        services will be destroyed when the DCL Instance is destroyed.
    */
    DclServiceDestroyAll(pDLFMD->hDclInst, TRUE);

    /*  Save handle in a temporary buffer so we can free the memory
    */
    hDclInst = pDLFMD->hDclInst;

    DclMemFree(pDLFMD);

    /*  Destroy the DCL Instance and any project level services
    */
    DclInstanceDestroy(hDclInst);

    gpDLFMD = NULL;

    return TRUE;
}


/*-------------------------------------------------------------------
    Public:  FMD_GetInfo()

    This function is call to get flash information
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_GetInfo(
    FlashInfo          *pFlashInfo)
{
    if(gpDLFMD->DiskInfo.fIsNAND)
        pFlashInfo->flashType       = NAND;
    else
        pFlashInfo->flashType       = NOR;

    pFlashInfo->dwNumBlocks         = gpDLFMD->DiskInfo.ulTotalBlocks;
    pFlashInfo->wSectorsPerBlock    = gpDLFMD->DiskInfo.nSectorsPerBlock;
    pFlashInfo->wDataBytesPerSector = gpDLFMD->DiskInfo.nBytesPerSector;
    pFlashInfo->dwBytesPerBlock     = gpDLFMD->DiskInfo.ulBytesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_ALWAYS, 1, 0),
        "FfxWM_FMD_GetInfo() returning TotalBlocks %lU\n", pFlashInfo->dwNumBlocks));

    return TRUE;
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_ReadSector()

    Read the content of the sector.
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_ReadSector(
    SECTOR_ADDR         ulStartSector,
    UCHAR              *pClientBuffer,
    SectorInfo         *pSectorInfo,
    DWORD               ulCount)
{
    SECTOR_ADDR         ulSector;
    BOOL                rc = TRUE;
    D_UINT32            ulRead;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxWM_FMD_ReadSector() Start=%lU Count=%lU pBuff=%P pSpare=%P\n",
        ulStartSector, ulCount, pClientBuffer, pSectorInfo));

    /*  We do not assert pClientBuffer or pSectorInfo here because either
        or both may be NULL, strange as it may seem...
    */

  #if !FFXCONF_USESECTORINFO
    /*  Fill the SectorInfo buffer with zeros if we are not using it.
    */
    if(pSectorInfo)
    {
        DclMemSet(pSectorInfo, 0, sizeof(*pSectorInfo));
        pSectorInfo = NULL;
    }
  #endif

    ulSector = RemapSector(gpDLFMD, ulStartSector);
    if(ulSector == D_UINT32_MAX)
    {
        rc = FALSE;
	}
    else
    {
        /*  This code does not currently handle multi-sector requests
            if any of them are subject to being remapped <differently>.
            Check to see that the difference between the remapped
            first sector and the original is the same as the difference
            between the remapped last sector and the original.  If so,
            the multi-sector request will work fine.  If not, then
            just fail -- we can't handle the request.  (We really don't
            expect to ever see this in practice since the standard MS
            FMD usage always does single sector I/O, <and> the use of
            the remap feature is optional.)
        */
        if(ulCount > 1)
        {
            SECTOR_ADDR ulLastSector = ulStartSector + ulCount - 1;

            if(ulSector - ulStartSector != RemapSector(gpDLFMD, ulLastSector) - ulLastSector)
            {
                FFXPRINTF(1, ("FfxWM_FMD_ReadSector() Unhandled request!\n"));
                DclProductionError();
                rc = FALSE;
                goto Cleanup;
            }
        }

        if(pClientBuffer)
        {
            ulRead = FfxLoaderReadFmlPages(gpDLFMD->hDisk, ulSector, ulCount,
                                            pClientBuffer, (D_BUFFER*)pSectorInfo,
                                            pSectorInfo ? sizeof(SectorInfo) : 0);
            if(ulRead != ulCount)
                rc = FALSE;
        }
    	else if(pSectorInfo)
    	{
    		ulRead = FfxLoaderReadTags(gpDLFMD->hDisk, ulSector, ulCount, (D_BUFFER*)pSectorInfo, sizeof(SectorInfo));
            if(ulRead != ulCount)
                rc = FALSE;
    	}
    }

  Cleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEUNDENT),
        "FfxWM_FMD_ReadSector() returning %u\n", rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_WriteSector()
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_WriteSector(
    SECTOR_ADDR         ulStartSector,
    UCHAR              *pClientBuffer,
    SectorInfo         *pSectorInfo,
    DWORD               ulCount)
{
    BOOL                rc = TRUE;
    D_UINT32            ulWritten;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEINDENT),
        "FfxWM_FMD_WriteSector() Start=%lU Count=%lU pBuff=%P pSpare=%P\n",
        ulStartSector, ulCount, pClientBuffer, pSectorInfo));

  #if !FFXCONF_USESECTORINFO
    /*  ignore sectorinfo
    */
    pSectorInfo = NULL;
  #endif

    if(pClientBuffer)
    {
        D_UINT32 ulNewSector;

        ulNewSector = RemapSector(gpDLFMD, ulStartSector);
        if(ulNewSector == D_UINT32_MAX)
        {
            rc = FALSE;
    	}
        else
        {
            unsigned    nTagSize;

            /*  This code does not currently handle multi-sector requests
                if any of them are subject to being remapped <differently>.
                Check to see that the difference between the remapped
                first sector and the original is the same as the difference
                between the remapped last sector and the original.  If so,
                the multi-sector request will work fine.  If not, then
                just fail -- we can't handle the request.  (We really don't
                expect to ever see this in practice since the standard MS
                FMD usage always does single sector I/O, <and> the use of
                the remap feature is optional.)
            */
            if(ulCount > 1)
            {
                SECTOR_ADDR ulLastSector = ulStartSector + ulCount - 1;

                if(ulNewSector - ulStartSector != RemapSector(gpDLFMD, ulLastSector) - ulLastSector)
                {
                    FFXPRINTF(1, ("FfxWM_FMD_WriteSector() Unhandled request!\n"));
                    DclProductionError();
                    rc = FALSE;
                    goto Cleanup;
                }
            }

            if(pSectorInfo &&
                DclMemCmp(&pSectorInfo->dwReserved1, FFXWIN_TAG_SIGNATURE, sizeof(pSectorInfo->dwReserved1)) == 0)
            {
                WORD    wTag;

                /*  High byte of the tag is in the high byte of wReserved2,
                    and the low byte is in bOEMReserved.

                    The low byte of wReserved2 is cannot be used because
                    the EBOOT downloader may have modified it by tweaking
                    the SECTOR_WRITE_IN_PROGRESS and SECTOR_WRITE_COMPLETED
                    flags.
                */
                wTag = (pSectorInfo->wReserved2 & 0xFF00) | pSectorInfo->bOEMReserved;

                pSectorInfo = (SectorInfo*)&wTag;
                nTagSize = sizeof(wTag);

            	FFXPRINTF(1, ("FfxWM_FMD_WriteSector() writing VBF data at page %lU with tag=%X\n", ulNewSector, wTag));
            }
            else
            {
                nTagSize = pSectorInfo ? sizeof(SectorInfo) : 0;
            }

    		ulWritten = FfxLoaderWriteFmlPages(gpDLFMD->hDisk, ulNewSector, ulCount,
                                                pClientBuffer, (const D_BUFFER*)pSectorInfo, nTagSize);
            if(ulWritten != ulCount)
                rc = FALSE;
        }
    }
    else
    {
/*        DclAssert(pSectorInfo); */
        FFXPRINTF(2, ("Ignoring Spare-only write for sector %lU\n", ulStartSector));

      #if D_DEBUG > 1
        if(pSectorInfo)
            DclHexDump("IgnoredSpareData: ", HEXDUMP_UINT8|HEXDUMP_NOOFFSET, sizeof(*pSectorInfo), sizeof(*pSectorInfo), pSectorInfo);
      #endif

        rc = TRUE;
    }

  Cleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEUNDENT),
        "FfxWM_FMD_WriteSector() returning %u\n", rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public:  FMD_GetBlockStatus()
-------------------------------------------------------------------*/
DWORD FfxWM_FMD_GetBlockStatus(
    BLOCK_ID            blockId)
{
    DWORD               rc = 0;
    D_UINT32            ulBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEINDENT),
        "FfxWM_FMD_GetBlockStatus() Block=%lU\n", blockId));

    ulBlock = RemapBlock(gpDLFMD, blockId);
    if(ulBlock == D_UINT32_MAX)
    {
        rc = BLOCK_STATUS_UNKNOWN;
        FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() error remapping block %lU\n", blockId));
        goto Cleanup;
    }

    if(gpDLFMD->pCurrentRange->fUseBBM)
    {
        /*  When we are using BBM, we may get requests for BlockStatus on
            blocks which are already formatted with BBM.  (IPL does not
            know this).  Just report them as "normal" blocks, since that
            is what the IPL code expects.
        */
        if(ulBlock >= gpDLFMD->CurrentDiskInfo.ulTotalBlocks)
        {
            /* rc |= BLOCK_STATUS_RESERVED; */
            FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() Reporting BBM block %lU as a NORMAL block\n", blockId));
            goto Cleanup;
        }
    }
    else
    {
        unsigned    nStatus;

        nStatus = FfxLoaderGetBlockStatus(gpDLFMD->hDisk, ulBlock);
        if(nStatus == UINT_MAX)
        {
            rc = BLOCK_STATUS_UNKNOWN;
            FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() error getting block status for block %lU\n", blockId));
            goto Cleanup;
        }

        if(nStatus == 0)
        {
            /*  If the range is NOT using BBM, then report any bad block we
                might have encountered with the GetRawBlockStatus() call.
                (If we are using BBM, then it is not relevant since the block
                will have been remapped.)
            */
            rc |= BLOCK_STATUS_BAD;
            goto Cleanup;
        }
    }

  #if !FFXCONF_USESECTORINFO

    rc = BLOCK_STATUS_UNKNOWN;

  #else
    {
        D_UINT32    ulSector;
        SectorInfo  si;
        SectorInfo  DummySI = {0};

        ulSector = ulBlock * gpDLFMD->DiskInfo.nSectorsPerBlock;

    	if(FfxLoaderReadTags(gpDLFMD->hDisk, ulSector, 1, (unsigned char *)&si, sizeof(si)) != 1)
        {
            rc = BLOCK_STATUS_UNKNOWN;
            FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() error reading tags for block %lU\n", blockId));
            goto Cleanup;
        }

        /*  If a tag was never written to the page, the returned
            value will be entirely null.  If this is the case, we
            do NOT want to examine the contents -- everything is
            considered "normal".
        */
        if(DclMemCmp(&si, &DummySI, sizeof(si)) != 0)
        {
            if(si.bBadBlock != 0xFF)
            {
                /*  If we ARE using BBM, we should not find any blocks
                    which WM has marked as bad in the tag area.  Assert
                    it so.
                */
                DclAssert(!gpDLFMD->pCurrentRange->fUseBBM);

                rc |= BLOCK_STATUS_BAD;
                FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() BAD block at block %lU\n", blockId));
            }

            if((si.bOEMReserved & OEM_BLOCK_READONLY) == 0)
            {
                rc |= BLOCK_STATUS_READONLY;
                FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() READONLY block at block %lU\n", blockId));
            }

            if((si.bOEMReserved & OEM_BLOCK_RESERVED) == 0)
            {
                rc |= BLOCK_STATUS_RESERVED;
                FFXPRINTF(1, ("FfxWM_FMD_GetBlockStatus() RESERVED block at block %lU\n", blockId));
            }
        }
    }
  #endif

  Cleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FfxWM_FMD_GetBlockStatus() Block=%lu returning %lX\n", blockId, rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_SetBlockStatus()
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_SetBlockStatus(
    BLOCK_ID            blockId,
    DWORD               status)
{
    /*  Doesn't matter whether FFXCONF_USESECTORINFO support is
        enabled or not (currently), since we are always failing
        this call.
    */
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, 0),
        "FfxWM_FMD_SetBlockStatus() Block=%lU Status=%lX (STUBBED!)\n", blockId, status));

    return FALSE;
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_EraseBlock()

    Erase the given block
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_EraseBlock(
    BLOCK_ID            blockId)
{
    BOOL                rc = TRUE;
    D_UINT32            ulBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEINDENT),
        "FfxWM_FMD_EraseBlock() Block=%lU\n", blockId));

    ulBlock = RemapBlock(gpDLFMD, blockId);
    if(ulBlock == D_UINT32_MAX)
    {
        rc = FALSE;
        FFXPRINTF(1, ("FfxWM_FMD_EraseBlock() error remapping block %lU\n", blockId));
	}
    else
    {
        if(ulBlock < gpDLFMD->CurrentDiskInfo.ulTotalBlocks)
        {
            if(!FfxLoaderEraseBlocks(gpDLFMD->hDisk, ulBlock, 1))
            {
                rc = FALSE;
            }
        }
        else
        {
            FFXPRINTF(1, ("Ignoring erase request for out-of-range block %lU\n", blockId));
            rc = TRUE;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, TRACEUNDENT),
        "FfxWM_FMD_EraseBlock() Block=%lU returning %u\n", blockId, rc));

    return rc;
}


/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_PowerUp()
-------------------------------------------------------------------*/
VOID FfxWM_FMD_PowerUp(VOID)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, 0),
        "FfxWM_FMD_PowerUp()\n"));

    return;
}

/*-------------------------------------------------------------------
    Public:  FfxWM_FMD_PowerDown()
-------------------------------------------------------------------*/
VOID FfxWM_FMD_PowerDown(VOID)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 1, 0),
        "FfxWM_FMD_PowerDown()\n"));

    return;
}

/*-------------------------------------------------------------------
    Public:  FMD_OEMIoControl()
-------------------------------------------------------------------*/
BOOL FfxWM_FMD_OEMIoControl(
    DWORD   dwIoControlCode,
    UCHAR  *pInBuf,
    DWORD   dwInBufSize,
    UCHAR  *pOutBuf,
    DWORD   dwOutBufSize,
    DWORD  *pBytesReturned)
{
    BOOL    rc = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEINDENT),
        "FfxWM_FMD_OEMIoControl() Code=%lX pInBuff=%P InBuffSize=%lU pOutBuff=%P OutBuffSize=%lU pBytesReturned=%P\n",
        dwIoControlCode, pInBuf, dwInBufSize, pOutBuf, dwOutBufSize, pBytesReturned));

    switch(dwIoControlCode)
    {
        case IOCTL_FMD_LOCK_BLOCKS:
        {
            PBlockLockInfo  pLockInfo = (PBlockLockInfo)pInBuf;
            FFXSTATUS       ffxStat;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_LOCK_BLOCKS) Start=%lU Count=%lU\n",
                pLockInfo->StartBlock, pLockInfo->NumBlocks));

            /*  This interface is (presumably) not used to lock "all" the
                blocks in the device, but rather is used to put the device
                into the "lock-freeze" mode.
            */

            /*  OneNAND requires blocks to be locked before locked-tight/freeze.
            */

            ffxStat = FfxLoaderLockBlocks(gpDLFMD->hDisk, pLockInfo->StartBlock, pLockInfo->NumBlocks);

    		ffxStat = FfxLoaderLockFreeze(gpDLFMD->hDisk, pLockInfo->StartBlock, pLockInfo->NumBlocks);
            if(ffxStat == FFXSTAT_SUCCESS)
                rc = TRUE;
        }
        break;

        case IOCTL_FMD_UNLOCK_BLOCKS:
        {
            PBlockLockInfo  pLockInfo = (PBlockLockInfo)pInBuf;
            FFXSTATUS       ffxStat;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_UNLOCK_BLOCKS) Start=%lU Count=%lU\n",
                pLockInfo->StartBlock, pLockInfo->NumBlocks));

    		ffxStat = FfxLoaderUnlockBlocks(gpDLFMD->hDisk, pLockInfo->StartBlock, pLockInfo->NumBlocks, FALSE);
            if(ffxStat == FFXSTAT_SUCCESS)
                rc = TRUE;
        }
        break;

        case IOCTL_FMD_GET_INTERFACE:
        {
            PFMDInterface pInterface = (PFMDInterface)pOutBuf;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_INTERFACE)\n"));

            if (!pOutBuf || dwOutBufSize < sizeof(FMDInterface))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_INTERFACE) Bad parameters\n"));
            }
            else
            {
                pInterface->cbSize              = sizeof(FMDInterface);
                pInterface->pInit               = FfxWM_FMD_Init;
                pInterface->pDeInit             = FfxWM_FMD_Deinit;
                pInterface->pGetInfo            = FfxWM_FMD_GetInfo;
                pInterface->pGetInfoEx          = NULL;
                pInterface->pGetBlockStatus     = FfxWM_FMD_GetBlockStatus;
                pInterface->pSetBlockStatus     = FfxWM_FMD_SetBlockStatus;
                pInterface->pReadSector         = FfxWM_FMD_ReadSector;
                pInterface->pWriteSector        = FfxWM_FMD_WriteSector;
                pInterface->pEraseBlock         = FfxWM_FMD_EraseBlock;
                pInterface->pPowerUp            = FfxWM_FMD_PowerUp;
                pInterface->pPowerDown          = FfxWM_FMD_PowerDown;
                pInterface->pGetPhysSectorAddr  = NULL;
                rc = TRUE;
            }
        }
        break;

        case IOCTL_FMD_SET_XIPMODE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_SET_XIPMODE) mode=%u is UNSUPPORTED\n", *(PBOOLEAN)pInBuf));

            if (!pInBuf || dwInBufSize < sizeof(BOOLEAN))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_SET_XIPMODE) Bad parameters\n"));
            }
        }
        break;

        case IOCTL_FMD_GET_XIPMODE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_XIPMODE) is UNSUPPORTED\n"));

            if (!pOutBuf || dwOutBufSize < sizeof(BOOLEAN))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_XIPMODE) Bad parameters\n"));
            }
        }
        break;

        case IOCTL_FMD_READ_RESERVED:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_READ_RESERVED) is UNSUPPORTED\n"));
        }
        break;

        case IOCTL_FMD_WRITE_RESERVED:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_WRITE_RESERVED) is UNSUPPORTED\n"));
        }
        break;

        case IOCTL_FMD_GET_RESERVED_TABLE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_RESERVED_TABLE) is UNSUPPORTED\n"));
        }
        break;
/*
        case IOCTL_SET_REGION_TABLE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_SET_REGION_TABLE) is UNSUPPORTED\n"));
        }
        break;
*/
        case IOCTL_DISK_DELETE_SECTORS:
        {
            DELETE_SECTOR_INFO* pDeleteInfo = (DELETE_SECTOR_INFO*) pInBuf;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_DISK_DELETE_SECTORS) Size=%lU Start=%lU Count=%lU is UNSUPPORTED\n",
                pDeleteInfo->cbSize, pDeleteInfo->startsector, pDeleteInfo->numsectors));
        }
        break;

        case IOCTL_DISK_GET_STORAGEID:
        {
            STORAGE_IDENTIFICATION* pStorageInfo = (STORAGE_IDENTIFICATION*) pOutBuf;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_DISK_GET_STORAGEID)\n"));

            if (!pOutBuf || dwOutBufSize < sizeof(STORAGE_IDENTIFICATION))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_DISK_GET_STORAGEID) Bad parameters\n"));
            }
            else
            {
                pStorageInfo->dwSize = sizeof(STORAGE_IDENTIFICATION);
                pStorageInfo->dwFlags = MANUFACTUREID_INVALID | SERIALNUM_INVALID;
                pStorageInfo->dwManufactureIDOffset = 0;
                pStorageInfo->dwSerialNumOffset = 0;

                rc = TRUE;
            }
        }
        break;

        case IOCTL_FMD_SET_SECTORSIZE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_SET_SECTORSIZE) is UNSUPPORTED\n"));

            if (!pInBuf || dwInBufSize < sizeof(DWORD))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_SET_SECTORSIZE) bad parameters\n"));
            }
        }
        break;

        case IOCTL_FMD_RAW_WRITE_BLOCKS:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_RAW_WRITE_BLOCKS) is UNSUPPORTED\n"));
        }
        break;

        case IOCTL_FMD_GET_RAW_BLOCK_SIZE:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_RAW_BLOCK_SIZE)\n"));

            if (!pOutBuf || dwOutBufSize < sizeof(DWORD))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_RAW_BLOCK_SIZE) bad parameters\n"));
            }
            else
            {
                *((PDWORD)pOutBuf) = gpDLFMD->DiskInfo.ulBytesPerBlock;
                rc = TRUE;
            }
        }
        break;

        case IOCTL_FMD_GET_INFO:
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_INFO)\n"));

            if (!pOutBuf || dwOutBufSize < sizeof(FMDInfo))
            {
                FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl(IOCTL_FMD_GET_INFO) bad parameters\n"));
            }
            else
            {
                PFMDInfo pInfo = (PFMDInfo)pOutBuf;
                pInfo->flashType = gpDLFMD->DiskInfo.fIsNAND ? NAND : NOR;
                pInfo->dwBaseAddress = 0;
                pInfo->dwNumRegions = gpDLFMD->DiskInfo.ulTotalBlocks;
                pInfo->dwNumReserved = 0;
                rc = TRUE;
            }
        }
        break;

        case IOCTL_FMD_FLASHFX:
        {
            FFXSTATUS   ffxStat;

            FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, 0),
                "FfxWM_FMD_OEMIoControl(IOCTL_FMD_FLASHFX)\n"));

            ffxStat = FfxDriverIoctlMasterDispatch((FFXIOREQUEST*)pInBuf);
            if(ffxStat == FFXSTAT_SUCCESS)
                rc = TRUE;
        }
        break;

        default:
            FFXPRINTF(1, ("FfxWM_FMD_OEMIoControl() Unrecognized IOCTL (%lX)\n", dwIoControlCode));
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVER, 2, TRACEUNDENT),
        "FfxWM_FMD_OEMIoControl() returning %u\n", rc));

    return rc;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void OutputAString(
    void       *pContext,
    const char *pszString)
{
    char        buff[512];
    unsigned    nn = 0;

    while(*pszString)
    {
        if((*pszString == '\n') || (nn >= sizeof(buff)-3))
        {
            buff[nn] = '\r';
            buff[nn+1] = '\n';
            buff[nn+2] = 0;

	        RETAILMSG(1,(TEXT("%S"), &buff[0]));

            nn = 0;

            if(*pszString == '\n')
                pszString++;
        }
        else
        {
            buff[nn] = *pszString;
            nn++;
            pszString++;
        }
    }

    if(nn)
    {
        buff[nn+0] = 0;

        RETAILMSG(1,(TEXT("%S"), &buff[0]));
    }

    return;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void OutputIPLString(
    void       *pContext,
    const char *pszString)
{
	wchar_t     acharacter[2];

	acharacter[1] = 0;
	while(*pszString)
	{
		acharacter[0] = *pszString;
		OEMWriteDebugString(acharacter);
		pszString++;
	}
	OEMWriteDebugString(L"\r");

    return;
}


/*-------------------------------------------------------------------
    This function translates a logical FMD block offset to a block
    offset which is relative to the current Range.  We adjust both
    the offset into the Disk if there is any reserved area, and if
    necessary translate the block number if it is in the remap table.

    Returns the tranlated block number, or D_UINT32_MAX if an error
    occurred.
-------------------------------------------------------------------*/
static D_UINT32 RemapBlock(
    DLFMDINFO      *pDLFMD,
    D_UINT32        ulBlockNum)
{
    FFXWMFMDRANGE  *pRange;

    if(!SelectRange(pDLFMD, ulBlockNum * pDLFMD->DiskInfo.nSectorsPerBlock))
    {
        FFXPRINTF(1, ("FfxFmdWm:RemapBlock() failed for block %lU\n", ulBlockNum));
        return D_UINT32_MAX;
    }

    if(pDLFMD->nCurrentRange == RANGE_1)
    {
        pRange = &pDLFMD->RangeInfo1;
    }
    else
    {
        D_UINT32    ulReservedBlocks;

        DclAssert(pDLFMD->fRange2Inited);
        DclAssert(pDLFMD->nCurrentRange == RANGE_2);

        pRange = &pDLFMD->RangeInfo2;

        ulReservedBlocks = pDLFMD->ulRange2ReservedSectors / pDLFMD->DiskInfo.nSectorsPerBlock;

        /*  If Range2 is current, then higher levels must have assured
            that the block number is past any reserved blocks we might
            have.
        */
        DclAssert(ulBlockNum >= ulReservedBlocks);

        /*  Adjust the block number to be relative to the reserved space.
        */
        ulBlockNum -= ulReservedBlocks;
    }

    /*  Typically any remap table will only be used for Range1, but it is
        technically possible to use it for Range2 as well.
    */
    if(pRange->nRemapTableSize)
    {
        unsigned    nn;

        DclAssert(!pRange->fUseBBM);

        for(nn=0; nn<(pRange->nRemapTableSize*2); nn+=2)
        {
            if(pRange->puRemapTable[nn] == ulBlockNum)
            {
                FFXPRINTF(1, ("FfxFmdWm:RemapBlock() Block %lU mapped to block %U\n", ulBlockNum, pRange->puRemapTable[nn+1]));
                return pRange->puRemapTable[nn+1];
            }
        }
    }

    return ulBlockNum;
}


/*-------------------------------------------------------------------
    This function translates a logical FMD sector offset to a sector
    offset which is relative to the current Range.  We adjust both
    the offset into the Disk if there is any reserved area, and if
    necessary translate the sector number if it is in the remap
    table.

    Returns the tranlated sector number, or D_UINT32_MAX if an error
    occurred.
-------------------------------------------------------------------*/
static D_UINT32 RemapSector(
    DLFMDINFO      *pDLFMD,
    D_UINT32        ulSectorNum)
{
    FFXWMFMDRANGE  *pRange;

    if(!SelectRange(pDLFMD, ulSectorNum))
    {
        FFXPRINTF(1, ("FfxFmdWm:RemapSector() failed for sector %lU\n", ulSectorNum));
        return D_UINT32_MAX;
    }

    if(pDLFMD->nCurrentRange == RANGE_1)
    {
        pRange = &pDLFMD->RangeInfo1;
    }
    else
    {
        DclAssert(pDLFMD->fRange2Inited);
        DclAssert(pDLFMD->nCurrentRange == RANGE_2);
        DclAssert(ulSectorNum >= pDLFMD->ulRange2ReservedSectors);

        /*  If Range2 is current, then higher levels must have assured
            that the sector number is past any reserved blocks we might
            have.
        */

        pRange = &pDLFMD->RangeInfo2;

        /*  Adjust the sector number to be relative to the reserved space.
        */
        ulSectorNum -= pDLFMD->ulRange2ReservedSectors;
    }

    /*  Typically any remap table will only be used for Range1, but it is
        technically possible to use it for Range2 as well.
    */
    if(pRange->nRemapTableSize)
    {
        D_UINT32    ulBlock = ulSectorNum / pDLFMD->DiskInfo.nSectorsPerBlock;
        D_UINT32    ulSectorInBlock = ulSectorNum % pDLFMD->DiskInfo.nSectorsPerBlock;
        unsigned    nn;

        DclAssert(!pRange->fUseBBM);

        for(nn=0; nn<(pRange->nRemapTableSize*2); nn+=2)
        {
            if(pRange->puRemapTable[nn] == ulBlock)
            {
                FFXPRINTF(1, ("FfxFmdWm:RemapSector() Block %lU mapped to block %U\n", ulBlock, pRange->puRemapTable[nn+1]));
                return (pRange->puRemapTable[nn+1] * pDLFMD->DiskInfo.nSectorsPerBlock) + ulSectorInBlock;
            }
        }
    }

    return ulSectorNum;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL SelectRange(
    DLFMDINFO  *pDLFMD,
    D_UINT32    ulSectorNum)
{
    unsigned    nNewRange;

    /*  If there is only one range supported, just do a few validity
        checks, and return successfully.
    */
    if(!pDLFMD->fRange2Inited)
    {
        DclAssert(pDLFMD->nCurrentRange == RANGE_1);
        DclAssert(pDLFMD->ulRange2ReservedSectors == 0);
        DclAssert(pDLFMD->ulRange2TotalSectors == 0);

        return TRUE;
    }

    DclAssert(pDLFMD->ulRange2TotalSectors);

    if((ulSectorNum < pDLFMD->ulRange2ReservedSectors) ||
        (ulSectorNum >= pDLFMD->ulRange2ReservedSectors + pDLFMD->ulRange2TotalSectors))
    {
        nNewRange = RANGE_1;
        pDLFMD->pCurrentRange = &pDLFMD->RangeInfo1;
    }
    else
    {
        DclAssert(pDLFMD->fRange2Inited);

        nNewRange = RANGE_2;
        pDLFMD->pCurrentRange = &pDLFMD->RangeInfo2;
    }

    if(nNewRange != pDLFMD->nCurrentRange)
    {
        D_UINT32    ulFlags = 0;

        if(pDLFMD->nCurrentRange != RANGE_NONE)
        {
    		FFXPRINTF(1, ("FfxFmdWm:Destroying Disks and Devices for Range %u\n", pDLFMD->nCurrentRange));

            /*  Deinitialize the current Disk and Device
            */
            FfxLoaderDiskDestroy(pDLFMD->hDisk);
            pDLFMD->hDisk = NULL;

            DestroyDevices(pDLFMD);

            pDLFMD->nCurrentRange = RANGE_NONE;
        }

 	    FFXPRINTF(1, ("FfxFmdWm:Creating Disks and Devices for Range %u\n", nNewRange));

        /*  Initialize the new Device and Disk
        */
        if(!CreateDevices(pDLFMD, pDLFMD->pCurrentRange))
        {
    		FFXPRINTF(1, ("FfxFmdWm:SelectRange() failed creating DEV%u for Range %u\n",
                                                pDLFMD->pCurrentRange->nDeviceNum, nNewRange));
            return FALSE;
        }

        if(pDLFMD->pCurrentRange->fUseAllocator)
            ulFlags |= FFXLOADER_DISK_USEALLOCATOR;

        if(pDLFMD->pCurrentRange->fSpanDevices)
            ulFlags |= FFXLOADER_DISK_SPANDEVICES;

        if(pDLFMD->pCurrentRange->fAltUnlock)
            ulFlags |= FFXLOADER_DISK_ALTUNLOCK;

        pDLFMD->hDisk = FfxLoaderDiskCreate(pDLFMD->hLdr, pDLFMD->pCurrentRange->nDiskNum, pDLFMD->pCurrentRange->nDeviceNum, ulFlags);
        if(!pDLFMD->hDisk)
        {
    		FFXPRINTF(1, ("FfxFmdWm:SelectRange() failed creating DISK%u for Range %u\n",
                                                pDLFMD->pCurrentRange->nDiskNum, nNewRange));
            return FALSE;
        }

        if(FfxLoaderGetDiskInfo(pDLFMD->hDisk, &pDLFMD->CurrentDiskInfo) != FFXSTAT_SUCCESS)
        {
    		FFXPRINTF(1, ("FfxLoaderGetDiskInfo() failed\n"));
            return FALSE;
        }

        pDLFMD->nCurrentRange = nNewRange;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL CreateDevices(
    DLFMDINFO      *pDLFMD,
    FFXWMFMDRANGE  *pRange)
{
    unsigned        nn;
    FFXWMFMDRANGE   TmpRange;

    DclAssert(pDLFMD);
    DclAssert(pRange);

    TmpRange = *pRange;
    pDLFMD->nDeviceCount = 0;

    for(nn=0; nn<FFX_MAX_DEVICES; nn++)
    {
        FFXLOADERDEVINFO    DevInfo;
        D_UINT32            ulFlags;

        ulFlags  = TmpRange.fUseBBM ? FFXLOADER_DEV_USEBBM : 0;
        ulFlags |= TmpRange.fAutoUnlock ? FFXLOADER_DEV_UNLOCK : 0;

        pDLFMD->ahDev[nn] = FfxLoaderDeviceCreate(pDLFMD->hLdr,
                                            TmpRange.nDeviceNum,
                                            TmpRange.ulExtraReservedLowKB,
                                            TmpRange.ulExtraReservedHighKB,
                                            TmpRange.ulMaxArraySizeKB,
                                            ulFlags);

        /*  Device failed to initialize
        */
        if(!pDLFMD->ahDev[nn])
        {
            /*  If it was the first Device which failed, that is a fatal error,
                however subsequent Devices are allowed to benignly fail.
            */
            if(nn == 0)
                return FALSE;
            else
                return TRUE;
        }

        if(FfxLoaderGetDeviceInfo(pDLFMD->ahDev[nn], &DevInfo) != FFXSTAT_SUCCESS)
            return FALSE;

        FFXPRINTF(1, ("DevInfo: DEV%u BlockSizeKB=%lU RawBlocks=%lU ChipBlocks=%lU ResvBlocks=%lU UsableBlocks=%lU PageSize=%u\n",
            TmpRange.nDeviceNum,
            DevInfo.ulBlockSize/1024,   DevInfo.ulRawBlocks,
            DevInfo.ulChipBlocks,       DevInfo.ulReservedBlocks,
            DevInfo.ulTotalBlocks,      DevInfo.nPageSize));

        pDLFMD->nDeviceCount++;

        /*  If we are not trying to span Devices, then just quit after the
            first one is initialized.
        */
        if(!pRange->fSpanDevices)
            break;

        /*  On to try the next Device.  Extra reserved space and max size
            limitations are never used on subsequent Devices.
        */
        TmpRange.nDeviceNum++;
        TmpRange.ulExtraReservedLowKB = 0;
        TmpRange.ulExtraReservedHighKB = 0;
        TmpRange.ulMaxArraySizeKB = ULONG_MAX;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL DestroyDevices(
    DLFMDINFO      *pDLFMD)
{
    unsigned        nn;

    DclAssert(pDLFMD);
    DclAssert(pDLFMD->hLdr);
    DclAssert(pDLFMD->nDeviceCount);

    for(nn = pDLFMD->nDeviceCount; nn > 0; nn--)
    {
        DclAssert(pDLFMD->ahDev[nn-1]);

        FfxLoaderDeviceDestroy(pDLFMD->ahDev[nn-1]);

        pDLFMD->ahDev[nn-1] = NULL;
    }

    pDLFMD->nDeviceCount = 0;

    return TRUE;
}




#if D_DEBUG

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static void DumpRangeInfo(
    unsigned        nRangeNum,
    FFXWMFMDRANGE  *pRange)
{
    DclPrintf("Range%u Info: DEV%u DISK%u ExtraResvLoKB=%lU ExtraResvHiKB=%lU MaxArrayKB=%lU\n",
        nRangeNum,
        pRange->nDeviceNum,           pRange->nDiskNum,
        pRange->ulExtraReservedLowKB, pRange->ulExtraReservedHighKB,
        pRange->ulMaxArraySizeKB);

    DclPrintf("             RemapSize=%U pRemap=%P UseBBM=%u UseAllocator=%u AutoUnlock=%u\n",
        pRange->nRemapTableSize,      pRange->puRemapTable,
        pRange->fUseBBM,              pRange->fUseAllocator,
        pRange->fAutoUnlock);

    if(pRange->nRemapTableSize)
    {
        unsigned nn;

        DclAssert(pRange->puRemapTable);

        DclPrintf("             RemapTable Contents:  From -    To\n");
        for(nn=0; nn<(pRange->nRemapTableSize*2); nn+=2)
        {
            DclPrintf("                                  %5U - %5U\n", pRange->puRemapTable[nn], pRange->puRemapTable[nn+1]);
        }
    }

    return;
}

#endif


#endif  /* _WIN32_WCE > 500 */


