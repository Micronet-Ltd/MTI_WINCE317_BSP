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

    This module contains Driver Framework functions to manage Disks.
    Typically these functions are used by the OS-specific device driver.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvdisk.c $
    Revision 1.29  2010/09/23 05:55:23Z  garyp
    Updated documentation and debug messages -- no functional changes.
    Revision 1.28  2010/09/19 03:11:46Z  garyp
    Corrected some documentation, comments, and debug messages --
    no functional changes.
    Revision 1.27  2010/07/06 01:24:08Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.26  2009/11/11 00:21:11Z  garyp
    Updated the enumerate function to more politely handle the case
    where the driver is not loaded.
    Revision 1.25  2009/07/21 21:03:24Z  garyp
    Merged from the v4.0 branch.  Eliminated a deprecated header.  Tweaked
    some debug messages.  Added an fIsNAND field to the FFXDISKINFO structure.  
    Improved error handling in FfxDriverDiskCreateParam().  Modified to handle
    special case Disk creation inside the FML, and simply record the results
    once the Disk has been created.  Minor cleanup in the Create function --
    not functionally changed.  Updated to ensure that the uPageSize and 
    uSectorToPageShift fields are initialized even in the event that an 
    allocator is not being used.
    Revision 1.24  2009/04/07 20:01:50Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.23  2009/03/31 18:06:14Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.22  2009/02/09 02:15:29Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables
    -- no functional changes.
    Revision 1.21  2008/05/21 02:13:35Z  garyp
    Merged from the WinMobile branch.
    Revision 1.20.1.2  2008/05/21 02:13:35Z  garyp
    Updated to use DCLISALIGNED().
    Revision 1.20  2008/03/22 20:00:35Z  Garyp
    Major update to streamline DISK management.  Refactored the DISK creation
    function into two functions -- one to provide the original functionality,
    and one to facilitate automated DISK creation from generic code where
    the hooks are not used.  Added a functions to enumerate DISKs.
    Revision 1.19  2008/01/13 07:26:23Z  keithg
    Function header updates to support autodoc.
    Revision 1.18  2007/12/03 01:59:09Z  Garyp
    Added FfxDriverDiskInfo().  Cleaned up FfxDriverDiskAdjust().
    Revision 1.17  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/07/13 23:28:27Z  timothyj
    Converted byte-wise start and offset logic in FfxDriverDiskCreate() to
    operate on block boundaries.  Created FfxDriverDiskAdjust() helper to
    adjust the start and length of a disk to align on flash erase block
    boundaries.  Updated FfxDriverDiskSettings() to use the new function
    FfxDriverDiskAdjust() helper.
    Revision 1.15  2007/05/23 01:11:26Z  Garyp
    Updated to support hrFs.
    Revision 1.14  2006/11/09 04:36:26Z  Garyp
    Updated to allow FfxDriverDiskHandle() to accept a NULL pointer to the
    FFXDRIVERINFO structure, to better accommodate external API requirements.
    Revision 1.13  2006/10/17 02:05:38Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.12  2006/10/11 18:20:01Z  Garyp
    Major update to allow format settings to be specified on a per-disk basis.
    Revision 1.11  2006/06/12 11:59:17Z  Garyp
    Added FfxDriverDiskGeometry().
    Revision 1.10  2006/04/26 23:40:56Z  billr
    Pointers can be wider than 32 bits.
    Revision 1.9  2006/04/17 22:37:44Z  tonyq
    Ensure locals in FfxDriverDiskCreate() were initialized
    Revision 1.8  2006/03/08 01:14:00Z  Garyp
    Added a check to avoid creating the same disk twice.
    Revision 1.7  2006/03/06 00:13:02Z  Garyp
    Updated to allow RAW disks to be mapped.
    Revision 1.6  2006/02/15 03:01:29Z  Garyp
    Split out the compaction settings code.
    Revision 1.5  2006/02/14 19:55:34Z  Garyp
    Improved error handling logic.
    Revision 1.4  2006/02/13 09:00:54Z  Garyp
    Corrected some symbol names.
    Revision 1.3  2006/02/12 03:59:28Z  Garyp
    Added FfxDriverDiskHandle().
    Revision 1.2  2006/02/11 04:25:47Z  Garyp
    Removed dead code.  Modified the disk range logic to be more tolerant
    and round up or down to erase block boundaries as needed.
    Revision 1.1  2006/02/10 07:56:20Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>

#define BAD_DISKNUM   (UINT_MAX)

static FFXSTATUS AdjustDiskSettings(FFXDISKINFO *pDisk, const FFXDISKCONFIG *pConf);

#if FFXCONF_FORMATSUPPORT
    static FFXSTATUS ValidateFormatSettings(FFXDISKCONFIG *pConf);
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCreate()

    Create a Disk instance, calling the function FfxHookDiskCreate()
    to use the pDiskCreateData to obtain the Disk parameters.

    Parameters:
        pFDI            - A pointer to the FFXDRIVERINFO structure
                          to use.
        pDiskCreateData - A pointer to the FFXDISKINITDATA structure
                          to use.  This value may only be NULL if
                          FfxHookDiskCreate() does not use it.

    Return Value:
        Returns a FFXDISKHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskCreate(
    FFXDRIVERINFO          *pFDI,
    FFXDISKINITDATA        *pDiskCreateData)
{
    FFXDISKCONFIG           conf = {0};
    FFXDISKHANDLE           hDisk = NULL;
    FFXDISKHOOK            *pHook;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDiskCreate() pDrv=%P pCreateData=%P\n", pFDI, pDiskCreateData ));

    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    DclAssert(pFDI);

    conf.nDiskNum = BAD_DISKNUM;

    /*  Call the Project Hooks code to allow any project specific
        configuration to occur.  It is the responsibility of the
        hook code to tell us which DISKn we are configuring.
    */
    pHook = FfxHookDiskCreate(pFDI, pDiskCreateData, &conf);
    if(pHook)
    {
        DclAssert(conf.nDiskNum < FFX_MAX_DISKS);

        /*  Now do the real Disk initialization
        */
        hDisk = FfxDriverDiskCreateParam(pFDI, &conf, pHook);
        if(!hDisk)
            FfxHookDiskDestroy(pFDI, pHook);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
        "FfxDriverDiskCreate() for DISK%u returning hDisk %P\n", conf.nDiskNum, hDisk));

    return hDisk;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskCreateParam()

    Create a physical Disk instance, using the parameters in the
    supplied FFXDISKCONFIG structure.

    The supplied FFXDISKCONFIG structure may be fully, partially,
    or not at all configured.  The various "fInitedxxxSettings"
    flags must be set to indicate which settings have already been
    initialized.  Any settings which have not been initialized will
    be set to use default values, typically from ffxconf.h.

    The contents of the FFXDISKCONFIG structure may be modified
    during the course of this create operation, and on exit it will
    reflect the configuration settings which were used.

    Parameters:
        pFDI  - A pointer to the FFXDRIVERINFO structure to use.
        pConf - A pointer to the FFXDISKCONFIG structure to use.
        pHook - A pointer to the FFXDISKHOOK structure returned
                by FfxHookDiskCreate().  This value may be NULL
                if the DiskCreate/Destroy() Project Hooks are
                not being used.

    Return Value:
        Returns a FFXDISKHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskCreateParam(
    FFXDRIVERINFO          *pFDI,
    FFXDISKCONFIG          *pConf,
    FFXDISKHOOK            *pHook)
{
    FFXFIMDEVINFO           DevInfo;
    FFXDISKHANDLE           hDisk = NULL;
    FFXDISKINFO            *pDisk = NULL;
    FFXSTATUS               ffxStat;
    unsigned                fNewDisk = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDiskCreateParam() pDrv=%P pConf=%P pHook=%P\n", pFDI, pConf, pHook));

    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    DclAssert(pFDI);
    DclAssert(pConf);

    if(pConf->nDiskNum >= FFX_MAX_DISKS)
    {
        ffxStat = FFXSTAT_DISKNUMBERINVALID;
        goto Cleanup;
    }

    if(pFDI->apDisk[pConf->nDiskNum])
    {
        FFXPRINTF(1, ("DISK%u already initialized\n", pConf->nDiskNum));
        ffxStat = FFXSTAT_DISK_ALREADYINITIALIZED;
        goto Cleanup;
    }

    if(!pConf->fInitedDiskSettings)
    {
        ffxStat = FfxDriverDiskSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedDiskSettings);
    }

  #if FFXCONF_FORMATSUPPORT
    if(!pConf->fInitedFormatSettings)
    {
        ffxStat = FfxDriverDiskFormatSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedFormatSettings);
    }

    ffxStat = ValidateFormatSettings(pConf);
    if(ffxStat != FFXSTAT_SUCCESS)
        goto Cleanup;
  #endif

  #if FFXCONF_ALLOCATORSUPPORT
    if(!pConf->fInitedAllocatorSettings)
    {
        ffxStat = FfxDriverDiskAllocatorSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedAllocatorSettings);
    }
  #endif

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    if(!pConf->fInitedCompactionSettings)
    {
        ffxStat = FfxDriverDiskCompSettings(pConf);
        if(ffxStat != FFXSTAT_SUCCESS)
            goto Cleanup;

        DclAssert(pConf->fInitedCompactionSettings);
    }
  #endif

    pDisk = DclMemAllocZero(sizeof(*pDisk));
    if(!pDisk)
        goto Cleanup;

    ffxStat = AdjustDiskSettings(pDisk, pConf);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("DISK%u settings adjustment failed, Status=%lX\n", pConf->nDiskNum, ffxStat));
        goto Cleanup;
    }

  #if D_DEBUG
    DclMemCpy(pDisk->acSignature, FFX_DISK_SIGNATURE, sizeof pDisk->acSignature);
  #endif

    pDisk->pDriverInfo = pFDI;
    pDisk->pDiskHook = pHook;
    pDisk->Conf = *pConf;

    /*  Set a flag so we know that we must zero out the pointer should
        we fail to completely initialize later in this process.  We
        can't just arbitrarily zero it on failure since the Device
        may have previously existed.
    */
    fNewDisk = TRUE;

    pFDI->apDisk[pConf->nDiskNum] = pDisk;

    hDisk = &pFDI->apDisk[pConf->nDiskNum];

    if(!pFDI->apDev[pConf->DiskSettings.nDevNum])
    {
        ffxStat = FFXSTAT_DISK_DEVICENOTINITIALIZED;
        goto Cleanup;
    }

    pDisk->pDev = pFDI->apDev[pConf->DiskSettings.nDevNum];

    if(!FfxDevInfo(pDisk->pDev->hFimDev, &DevInfo))
    {
        ffxStat = FFXSTAT_INVALIDDEVICE;
        goto Cleanup;
    }

  #if FFXCONF_NANDSUPPORT
    if(DevInfo.uDeviceType == DEVTYPE_NAND)
        pDisk->fIsNAND = TRUE;
  #endif

    /*  For now the sector length and page size are the same
    */
    pDisk->ulSectorLength = DevInfo.uPageSize;
    pDisk->uPageSize        = DevInfo.uPageSize;
    pDisk->ulBlockSize = DevInfo.ulBlockSize;
    pDisk->uSectorToPageShift = 0;

    pDisk->hFML = FfxFmlCreate(hDisk, pDisk->pDev->hFimDev, pConf->nDiskNum,
                              pDisk->ulBlockOffset, pDisk->ulBlockCount,
                              pConf->DiskSettings.nFlags);
    if(pDisk->hFML)
    {
        FFXFMLINFO  fi;

        ffxStat = FfxFmlDiskInfo(pDisk->hFML, &fi);
        if(ffxStat == FFXSTAT_SUCCESS)
            pDisk->ulBlockCount = fi.ulTotalBlocks;
    else
            FfxFmlDestroy(pDisk->hFML);
    }
    else
    {
        ffxStat = FFXSTAT_DISKMAPPINGFAILED;
    }

  Cleanup:

    if(ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Wipe this out only if we were the ones to set it...
        */
        if(fNewDisk)
            pFDI->apDisk[pConf->nDiskNum] = NULL;

        if(pDisk)
            DclMemFree(pDisk);

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
            "FfxDriverDiskCreateParam() failed, Status=%lX\n", ffxStat));

        return NULL;
    }
    else
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEUNDENT),
            "FfxDriverDiskCreateParam() successfully created DISK%u, hDisk=%P\n", pConf->nDiskNum, hDisk));

        return hDisk;
    }
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskDestroy()

    Destroy a Disk instance.

    Parameters:
        hDisk - The FFXDISKKANDLE to use

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskDestroy(
    FFXDISKHANDLE   hDisk)
 {
    FFXSTATUS       ffxStat = FFXSTAT_BADHANDLE;
    unsigned        nDiskNum = UINT_MAX;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverDiskDestroy() hDisk=%P\n", hDisk));

    if(FfxDriverDiskHandleValidate(hDisk))
    {
        FFXDISKINFO    *pDisk = *hDisk;

        nDiskNum = pDisk->Conf.nDiskNum;

        DclAssert(pDisk->hFML);

      #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
        if(pDisk->hVBF)
        {
            FFXPRINTF(1, ("Cannot destroy a Disk while the allocator is initialized for it\n"));
            ffxStat = FFXSTAT_DISK_ALLOCATORINITED;
        }
        else
      #endif
        {
            ffxStat = FfxFmlDestroy(pDisk->hFML);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                FFXDRIVERINFO  *pFDI = pDisk->pDriverInfo;
                FFXDISKHOOK    *pHook = pDisk->pDiskHook;

                DclMemFree(pDisk);

                if(pHook)
                    FfxHookDiskDestroy(pFDI, pHook);

                /*  Remove this Disk from the driver's list of Disks.
                */
                pFDI->apDisk[nDiskNum] = NULL;
            }
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_ALWAYS, 1, TRACEUNDENT),
        "FfxDriverDiskDestroy() DISK%u returning Status=%lX\n", nDiskNum, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskEnumerate()

    Enumerates the Disks in the system.  They are enumerated in
    "Disk number" order.

    Parameters:
        pFDI  - A pointer to the FFXDRIVERINFO structure.  This
                value may be null to use the current driver
                data structure (this is done only to accommodate
                external API needs).
        hDisk - The handle for the last Disk enumerated.  Set
                this to NULL to start the enumeration.

    Return Value:
        Returns the FFXDISKHANDLE value for the next Disk, or NULL
        if the enumeration is complete.
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskEnumerate(
    FFXDRIVERINFO  *pFDI,
    FFXDISKHANDLE   hDisk)
{
    unsigned        nDiskNum = 0;

    if(!pFDI)
    {
        pFDI = FfxDriverHandle(1);
        if(!pFDI)
        {
            DCLPRINTF(1, ("A FlashFX driver for instance 1 is not loaded\n"));
            return NULL;
        }
    }

    if(hDisk)
    {
        FFXDISKINFO    *pDisk;

        pDisk = FfxDriverDiskPtr(hDisk);
        if(!pDisk)
        {
            DclError();
            return NULL;
        }

        nDiskNum = pDisk->Conf.nDiskNum + 1;
    }

    for(; nDiskNum < FFX_MAX_DISKS; nDiskNum++)
    {
        hDisk = FfxDriverDiskHandle(pFDI, nDiskNum);
        if(hDisk)
            return hDisk;
    }

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskHandle()

    Return the Disk handle for a given Disk number.

    Parameters:
        pFDI     - A pointer to the FFXDRIVERINFO structure.  This
                   value may be null to use the current driver
                   data structure (this is done only to accommodate
                   external API needs).
        nDiskNum - The Disk number

    Return Value:
        Returns an FFXDISKHANDLE value if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxDriverDiskHandle(
    FFXDRIVERINFO  *pFDI,
    unsigned        nDiskNum)
{
    if(pFDI == NULL)
        pFDI = FfxDriverHandle(1);

    DclAssert(pFDI);

    if(nDiskNum >= FFX_MAX_DISKS)
        return NULL;

    if(!pFDI->apDisk[nDiskNum])
        return NULL;

    DclAssert(pFDI->apDisk[nDiskNum]->Conf.nDiskNum == nDiskNum);

    return &pFDI->apDisk[nDiskNum];
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskPtr()

    Return a pointer to the FFXDISKINFO structure, given a Disk
    handle.

    Note that typically the driver framework code simply
    dereferences the handle.  This function is provide as
    a convenience.

    Parameters:
        hDisk - The FFXDISKHANDLE to use

    Return Value:
        If successful, returns a pointer to the FFXDISKINFO structure
        for the Disk, or NULL otherwise.
-------------------------------------------------------------------*/
FFXDISKINFO * FfxDriverDiskPtr(
    FFXDISKHANDLE    hDisk)
{
    DclAssert(FfxDriverDiskHandleValidate(hDisk));

    return *hDisk;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskGeometry()

    Report physical and logical geometry information for a Disk.

    This geometry information includes physical information such as
    the erase block size, and the location within a Device a the Disk
    resides.

    Emulated CHS values will be returned when they are available --
    check the fCHSValid flag to ensure they are valid.

    Note that if an allocator is being used, the ulLogicalSectorCount
    value will be different than the total physical sectors you can
    calculate by using the the ulBlockCount, ulBlockSize, and
    nBytesPerSector fields.

    If no allocator is being used then ulLogicalSectorCount will
    match the calculated physical sector count.

    Note also that when an allocator is being used, that the
    ulLogicalSectorCount value may differ from the total sectors
    reported available by VBF due to the need to round down to make
    the CHS calculations work.

    Parameters:
        hDisk     - The Disk handle
        pGeometry - A pointer to the FFXDISKGEOMETRY structure

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskGeometry(
    FFXDISKHANDLE       hDisk,
    FFXDISKGEOMETRY    *pGeometry)
{
    DclAssert(FfxDriverDiskHandleValidate(hDisk));
    DclAssert(pGeometry);

    DclMemSet(pGeometry, 0, sizeof(*pGeometry));

    pGeometry->nDiskNum         = (*hDisk)->Conf.nDiskNum;
    pGeometry->nDevNum          = (*hDisk)->Conf.DiskSettings.nDevNum;
    pGeometry->ulBlockOffset    = (*hDisk)->ulBlockOffset;
    pGeometry->ulBlockCount     = (*hDisk)->ulBlockCount;
    pGeometry->ulBlockSize      = (*hDisk)->ulBlockSize;

    /*  Cases where these fields may not be initialized are when the
        allocator is not being used (or initialized yet), or where
        CHS cannot accommodate LFA.
    */
    if((*hDisk)->fpdi.uSectorsPerTrack &&
        (*hDisk)->fpdi.uHeads &&
        (*hDisk)->fpdi.uCylinders)
    {
        pGeometry->nSectorsPerTrack     = (unsigned)(*hDisk)->fpdi.uSectorsPerTrack;
        pGeometry->nHeads               = (unsigned)(*hDisk)->fpdi.uHeads;
        pGeometry->nCylinders           = (unsigned)(*hDisk)->fpdi.uCylinders;

        pGeometry->fCHSValid = TRUE;
    }

    /*  If these are non-zero, then we are good to go.
    */
    if((*hDisk)->fpdi.ulBytesPerSector && (*hDisk)->fpdi.ulTotalSectors)
    {
        pGeometry->nBytesPerSector      = (unsigned)(*hDisk)->fpdi.ulBytesPerSector;
        pGeometry->ulLogicalSectorCount = (*hDisk)->fpdi.ulTotalSectors;
    }
    else
    {
        /*  If there is no allocator, or things aren't fully initialized
            yet, return the physical characteristics for the logical.
        */
        pGeometry->nBytesPerSector = (unsigned)(*hDisk)->ulSectorLength;
        pGeometry->ulLogicalSectorCount =
            (pGeometry->ulBlockSize / pGeometry->nBytesPerSector) *
             pGeometry->ulBlockCount;
    }

  #if FFXCONF_ALLOCATORSUPPORT
    if((*hDisk)->hVBF)
        pGeometry->fAllocatorLoaded = TRUE;
  #endif

    FFXPRINTF(2, ("DISK%u Geometry:\n", pGeometry->nDiskNum));
    FFXPRINTF(2, ("  DEV%u Block Offset / Count %5lU /%5lU\n", pGeometry->nDevNum, pGeometry->ulBlockOffset, pGeometry->ulBlockCount));
    FFXPRINTF(2, ("  BlockSize / SectorSize  %5lUKB /%5u\n",   pGeometry->ulBlockSize/1024, pGeometry->nBytesPerSector));
    FFXPRINTF(2, ("  Logical Sector Count        %10lU\n",     pGeometry->ulLogicalSectorCount));
    FFXPRINTF(2, ("  CHS Info           %7u / %3u / %3u\n",    pGeometry->nCylinders, pGeometry->nHeads, pGeometry->nSectorsPerTrack));
    FFXPRINTF(2, ("  CHS Valid=%u Allocator Loaded=%u\n",      pGeometry->fCHSValid, pGeometry->fAllocatorLoaded));

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskHandleValidate()

    Determine if the specified handle is a valid Disk handle.

    Parameters:
        hDisk        - The Disk handle

    Return Value:
        Returns TRUE if the handle is a valid Disk handle, otherwise
        FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverDiskHandleValidate(
    FFXDISKHANDLE   hDisk)
{
    FFXDISKINFO    *pDisk;
    FFXDRIVERINFO  *pFDI;

    if(!hDisk)
        return FALSE;

    pDisk = *hDisk;

    if(!pDisk)
        return FALSE;

    if(!DCLISALIGNED((D_UINTPTR)pDisk, DCL_ALIGNSIZE))
        return FALSE;

  #if D_DEBUG
    if(DclMemCmp(pDisk->acSignature, FFX_DISK_SIGNATURE, sizeof pDisk->acSignature))
        return FALSE;
  #endif

    if(!pDisk->pDriverInfo)
        return FALSE;

    if(pDisk->Conf.nDiskNum >= FFX_MAX_DISKS)
        return FALSE;

    pFDI = pDisk->pDriverInfo;

  #if D_DEBUG
    if(DclMemCmp(pFDI->acSignature, FFX_DRIVER_SIGNATURE, sizeof pFDI->acSignature))
        return FALSE;
  #endif

    if(pFDI->apDisk[pDisk->Conf.nDiskNum] != pDisk)
        return FALSE;

    if(&pFDI->apDisk[pDisk->Conf.nDiskNum] != hDisk)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskSettings()

    Initialize the Disk settings from the developer specified
    configuration information in ffxconf.h.

    This function supports up to 8 Disks by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DISKS > 8).

    Note that in some environments portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Disk creation, we must pass in a pointer to the
    FFXDISKINFO structure to use, rather than a Disk handle
    (since it is not known yet).

    Parameters:
        pConf - A pointer to the FFXDISKCONFIG structure to use.
                The nDiskNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskSettings(
    FFXDISKCONFIG  *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DISKS > 8
#error "FlashFX Disk settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedDiskSettings);

    switch(pConf->nDiskNum)
    {
      #if FFX_MAX_DISKS > 0 && defined(FFX_DISK0_SETTINGS)
        case 0:
        {
            FFXDISKSETTINGS     ds = FFX_DISK0_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 1 && defined(FFX_DISK1_SETTINGS)
        case 1:
        {
            FFXDISKSETTINGS     ds = FFX_DISK1_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 2 && defined(FFX_DISK2_SETTINGS)
        case 2:
        {
            FFXDISKSETTINGS     ds = FFX_DISK2_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 3 && defined(FFX_DISK3_SETTINGS)
        case 3:
        {
            FFXDISKSETTINGS     ds = FFX_DISK3_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 4 && defined(FFX_DISK4_SETTINGS)
        case 4:
        {
            FFXDISKSETTINGS     ds = FFX_DISK4_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 5 && defined(FFX_DISK5_SETTINGS)
        case 5:
        {
            FFXDISKSETTINGS     ds = FFX_DISK5_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 6 && defined(FFX_DISK6_SETTINGS)
        case 6:
        {
            FFXDISKSETTINGS     ds = FFX_DISK6_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 7 && defined(FFX_DISK7_SETTINGS)
        case 7:
        {
            FFXDISKSETTINGS     ds = FFX_DISK7_SETTINGS;

            pConf->DiskSettings = ds;
            pConf->fInitedDiskSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDiskNum >= FFX_MAX_DISKS)
            {
                DclPrintf("FFX: DISK%u out of range or not configured (FFX_MAX_DISKS=%u)\n", pConf->nDiskNum, FFX_MAX_DISKS);

                ffxStat = FFXSTAT_DISKNUMBERINVALID;
            }
            else
            {
                FFXDISKSETTINGS     ds = {FFX_DEFAULT_DISK_SETTINGS};

                FFXPRINTF(1, ("Basic settings are missing for DISK%u, using default values\n", pConf->nDiskNum));

                pConf->DiskSettings = ds;
                pConf->fInitedDiskSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDiskSettings() for DISK%u returning Status=%lX\n", pConf->nDiskNum, ffxStat));

    return ffxStat;
}


#if FFXCONF_FORMATSUPPORT

/*-------------------------------------------------------------------
    Public: FfxDriverDiskFormatSettings()

    Initialize the Disk format settings from the developer specified
    configuration information in ffxconf.h.

    This function supports up to 8 Disks by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DISKS > 8).

    Note that in some environments, portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Disk creation, we must pass in a pointer to the
    FFXDISKINFO structure to use, rather than a Disk handle
    (since it is not known yet).

    Parameters:
        pConf - A pointer to the FFXDISKCONFIG structure to use.
                The nDiskNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskFormatSettings(
    FFXDISKCONFIG  *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DISKS > 8
#error "FlashFX Disk format settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedFormatSettings);

    switch(pConf->nDiskNum)
    {
      #if FFX_MAX_DISKS > 0 && defined(FFX_DISK0_FORMAT)
        case 0:
        {
            FFXFORMATSETTINGS       fs = FFX_DISK0_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 1 && defined(FFX_DISK1_FORMAT)
        case 1:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK1_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 2 && defined(FFX_DISK2_FORMAT)
        case 2:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK2_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 3 && defined(FFX_DISK3_FORMAT)
        case 3:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK3_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 4 && defined(FFX_DISK4_FORMAT)
        case 4:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK4_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 5 && defined(FFX_DISK5_FORMAT)
        case 5:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK5_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 6 && defined(FFX_DISK6_FORMAT)
        case 6:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK6_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 7 && defined(FFX_DISK7_FORMAT)
        case 7:
        {
            FFXFORMATSETTINGS   fs = FFX_DISK7_FORMAT;

            pConf->FormatSettings = fs;
            pConf->fInitedFormatSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDiskNum >= FFX_MAX_DISKS)
            {
                DclPrintf("FFX: DISK%u out of range or not configured (FFX_MAX_DISKS=%u)\n", pConf->nDiskNum, FFX_MAX_DISKS);

                ffxStat = FFXSTAT_DISKNUMBERINVALID;
            }
            else
            {
                FFXFORMATSETTINGS     fs = {FFX_DEFAULT_FORMAT_SETTINGS};

                FFXPRINTF(1, ("Format settings are missing for DISK%u, using default values\n", pConf->nDiskNum));

                pConf->FormatSettings = fs;
                pConf->fInitedFormatSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDiskFormatSettings() for DISK%u returning Status=%lX\n", pConf->nDiskNum, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ValidateFormatSettings()

    Validates that the user specified format settings for a given
    Disk are valid.

    Parameters:
        pConf        - A pointer to the FFXDISKCONFIG structure

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS ValidateFormatSettings(
    FFXDISKCONFIG  *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    DclAssert(pConf);

    switch(pConf->FormatSettings.uFileSystem)
    {
        case FFX_FILESYS_FAT:
          #if !FFXCONF_FATSUPPORT
            DclPrintf("DISK%u format type is set to FAT, but FFXCONF_FATSUPPORT is FALSE\n",
                pConf->nDiskNum);

            ffxStat = FFXSTAT_DISK_FATUNSUPPORTED;
          #endif

            break;

        case FFX_FILESYS_RELIANCE:
          #if !FFXCONF_RELIANCESUPPORT
            DclPrintf("DISK%u format type is set to Reliance, but FFXCONF_RELIANCESUPPORT is FALSE\n",
                pConf->nDiskNum);

            ffxStat = FFXSTAT_DISK_RELIANCEUNSUPPORTED;
          #endif

            break;

        case FFX_FILESYS_HRFS:
          #if !FFXCONF_HRFSSUPPORT
            DclPrintf("DISK%u format type is set to hrFs, but FFXCONF_HRFSSUPPORT is FALSE\n",
                pConf->nDiskNum);

            ffxStat = FFXSTAT_DISK_HRFSUNSUPPORTED;
          #endif

            break;

        case FFX_FILESYS_UNKNOWN:
          #if FFXCONF_MBRFORMAT
            if(pConf->FormatSettings.fUseMBR)
            {
                DclPrintf("DISK%u format type is set to 'Unknown', but 'UseMBR' is TRUE\n",
                    pConf->nDiskNum);

                ffxStat = FFXSTAT_DISK_BADMBRSETTING;
            }
          #endif

            break;

        default:
            DclPrintf("DISK%u file system type %U is not recognized\n",
                pConf->nDiskNum, pConf->FormatSettings.uFileSystem);

            ffxStat = FFXSTAT_DISK_BADFILESYSSETTING;

            break;
    }

  #if D_DEBUG && !FFXCONF_MBRSUPPORT
    if(pConf->FormatSettings.fUseMBR)
    {
        DclPrintf("DISK%u 'UseMBR' setting is TRUE, however FFXCONF_MBRSUPPORT is FALSE\n",
            pConf->nDiskNum);

        ffxStat = FFXSTAT_DISK_MBRSUPPORTDISABLED;
    }
  #endif

  #if D_DEBUG && !FFXCONF_MBRFORMAT
    if(pConf->FormatSettings.fUseMBR)
    {
        DclPrintf("DISK%u 'UseMBR' setting is TRUE, however FFXCONF_MBRFORMAT is FALSE\n",
            pConf->nDiskNum);

        ffxStat = FFXSTAT_DISK_MBRFORMATDISABLED;
    }
  #endif

    return ffxStat;
}


#endif /* FFXCONF_FORMATSUPPORT */


#if FFXCONF_ALLOCATORSUPPORT

/*-------------------------------------------------------------------
    Public: FfxDriverDiskAllocatorSettings()

    Initialize the Disk allocator settings from the developer
    specified configuration information in ffxconf.h.

    This function supports up to 8 Disks by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DISKS > 8).

    Note that in some environments, portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Disk creation, we must pass in a pointer to the
    FFXDISKINFO structure to use, rather than a Disk handle
    (since it is not known yet).

    If the allocator settings for the given Disk are not
    specified, default settings will be used.

    Parameters:
        pConf - A pointer to the FFXDISKCONFIG structure to use.
                The nDiskNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskAllocatorSettings(
    FFXDISKCONFIG  *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DISKS > 8
#error "FlashFX Disk allocator settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedAllocatorSettings);

    switch(pConf->nDiskNum)
    {
      #if FFX_MAX_DISKS > 0 && defined(FFX_DISK0_ALLOCATOR)
        case 0:
        {
            FFXALLOCSETTINGS as = FFX_DISK0_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 1 && defined(FFX_DISK1_ALLOCATOR)
        case 1:
        {
            FFXALLOCSETTINGS as = FFX_DISK1_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 2 && defined(FFX_DISK2_ALLOCATOR)
        case 2:
        {
            FFXALLOCSETTINGS as = FFX_DISK2_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 3 && defined(FFX_DISK3_ALLOCATOR)
        case 3:
        {
            FFXALLOCSETTINGS as = FFX_DISK3_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 4 && defined(FFX_DISK4_ALLOCATOR)
        case 4:
        {
            FFXALLOCSETTINGS as = FFX_DISK4_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 5 && defined(FFX_DISK5_ALLOCATOR)
        case 5:
        {
            FFXALLOCSETTINGS as = FFX_DISK5_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 6 && defined(FFX_DISK6_ALLOCATOR)
        case 6:
        {
            FFXALLOCSETTINGS as = FFX_DISK6_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 7 && defined(FFX_DISK7_ALLOCATOR)
        case 7:
        {
            FFXALLOCSETTINGS as = FFX_DISK7_ALLOCATOR;

            pConf->AllocSettings = as;
            pConf->fInitedAllocatorSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDiskNum >= FFX_MAX_DISKS)
            {
                DclPrintf("FFX: DISK%u out of range or not configured (FFX_MAX_DISKS=%u)\n", pConf->nDiskNum, FFX_MAX_DISKS);

                ffxStat = FFXSTAT_DISKNUMBERINVALID;
            }
            else
            {
                FFXALLOCSETTINGS     as = {FFX_DEFAULT_ALLOCATOR_SETTINGS};

                FFXPRINTF(1, ("Allocator settings are missing for DISK%u, using default values\n", pConf->nDiskNum));

                pConf->AllocSettings = as;
                pConf->fInitedAllocatorSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDiskAllocatorSettings() for DISK%u returning Status=%lX\n", pConf->nDiskNum, ffxStat));

    return ffxStat;
}

#endif


#if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS

/*-------------------------------------------------------------------
    Public: FfxDriverDiskCompSettings()

    Initialize the Disk compaction settings from the developer
    specified configuration information in ffxconf.h.

    This function supports up to 8 Disks by default, and is
    easily modified if the system needs to support more than
    that (FFX_MAX_DISKS > 8).

    Note that in some environments, portions of these settings
    may be specified via other means (such as the registry in
    Windows CE), and therefore #ifdef is used to ensure that
    only valid settings are configured.

    Note also that because this function is used during the
    course of Disk creation, we must pass in a pointer to the
    FFXDISKINFO structure to use, rather than a Disk handle
    (since it is not known yet).

    If the compaction settings for the given Disk are not
    specified, default settings will be used.

    Parameters:
        pConf - A pointer to the FFXDISKCONFIG structure to use.
                The nDiskNum field must be filled in prior to
                calling this function.

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskCompSettings(
    FFXDISKCONFIG  *pConf)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

#if FFX_MAX_DISKS > 8
#error "FlashFX Disk compaction settings initialization maxed out at 8"
#endif

    /*  We shouldn't be in this code if the flag indicates that the
        settings have already been initialized.
    */
    DclAssert(!pConf->fInitedCompactionSettings);

    switch(pConf->nDiskNum)
    {
      #if FFX_MAX_DISKS > 0 && defined(FFX_DISK0_COMPACTION)
        case 0:
        {
            FFXCOMPSETTINGS cs = FFX_DISK0_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 1 && defined(FFX_DISK1_COMPACTION)
        case 1:
        {
            FFXCOMPSETTINGS cs = FFX_DISK1_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 2 && defined(FFX_DISK2_COMPACTION)
        case 2:
        {
            FFXCOMPSETTINGS cs = FFX_DISK2_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 3 && defined(FFX_DISK3_COMPACTION)
        case 3:
        {
            FFXCOMPSETTINGS cs = FFX_DISK3_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 4 && defined(FFX_DISK4_COMPACTION)
        case 4:
        {
            FFXCOMPSETTINGS cs = FFX_DISK4_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 5 && defined(FFX_DISK5_COMPACTION)
        case 5:
        {
            FFXCOMPSETTINGS cs = FFX_DISK5_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 6 && defined(FFX_DISK6_COMPACTION)
        case 6:
        {
            FFXCOMPSETTINGS cs = FFX_DISK6_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

      #if FFX_MAX_DISKS > 7 && defined(FFX_DISK7_COMPACTION)
        case 7:
        {
            FFXCOMPSETTINGS cs = FFX_DISK7_COMPACTION;

            pConf->CompSettings = cs;
            pConf->fInitedCompactionSettings = TRUE;

            break;
        }
      #endif

        default:
            if(pConf->nDiskNum >= FFX_MAX_DISKS)
            {
                DclPrintf("FFX: DISK%u out of range or not configured (FFX_MAX_DISKS=%u)\n", pConf->nDiskNum, FFX_MAX_DISKS);

                ffxStat = FFXSTAT_DISKNUMBERINVALID;
            }
            else
            {
                FFXCOMPSETTINGS     cs = {FFX_DEFAULT_COMPACTION_SETTINGS};

                FFXPRINTF(1, ("Compaction settings are missing for DISK%u, using default values\n", pConf->nDiskNum));

                pConf->CompSettings = cs;
                pConf->fInitedCompactionSettings = TRUE;
            }
    }

    FFXPRINTF(2, ("FfxDriverDiskCompSettings() for DISK%u returning Status=%lX\n", pConf->nDiskNum, ffxStat));

    return ffxStat;
}

#endif


/*-------------------------------------------------------------------
    Local: AdjustDiskSettings()

    Initialize the Disk settings from a caller-provided
    FFXDISKSETTINGS structure, and adjusts the offset and
    length to flash block boundries.

    Note also that because this function is used during the
    course of Disk creation, we must pass in a pointer to the
    FFXDISKINFO structure to use, rather than a Disk handle
    (since it is not known yet).

    Parameters:
        pDisk - A pointer to the FFXDISKINFO structure
        pConf - A pointer to the FFXDISKCONFIG structure

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS AdjustDiskSettings(
    FFXDISKINFO            *pDisk,
    const FFXDISKCONFIG    *pConf)
{
    FFXFIMDEVINFO           DevInfo;
    FFXFIMDEVHANDLE         hFimDev;
    D_UINT32                ulBlockSizeKB;
    const FFXDISKSETTINGS  *pDiskSettings = &pConf->DiskSettings;

    DclAssert(pDisk);
    DclAssert(pConf);

    hFimDev = FfxDevHandle((D_UINT16)pConf->DiskSettings.nDevNum);
    if (!hFimDev)
    {
        FFXPRINTF(1, ("Failed to get handle for DEV%u\n", pConf->DiskSettings.nDevNum));

        return FFXSTAT_INVALIDDEVICE;
    }

    if(!FfxDevInfo(hFimDev, &DevInfo))
    {
        FFXPRINTF(1, ("Failed to get Device info for DEV%u\n", pConf->DiskSettings.nDevNum));

        return FFXSTAT_INVALIDDEVICE;
    }

    DclAssert((DevInfo.ulBlockSize % 1024UL) == 0);
    ulBlockSizeKB = DevInfo.ulBlockSize / 1024UL;

    /*  For the offset, if not aligned on an erase block boundary,
        round up to the nearest block boundary, and print a warning.
    */
    pDisk->ulBlockOffset = pDiskSettings->ulOffsetKB / ulBlockSizeKB;
    if((pDiskSettings->ulOffsetKB % ulBlockSizeKB) != 0)
    {
        FFXPRINTF(1, ("Offset is not on an erase block boundary - increasing.\n"));
        pDisk->ulBlockOffset++;
    }

    if(pDiskSettings->ulLengthKB >= FFX_SPECIAL_CASE_MINIMUM)
    {
        /*  Handle special cases separately
        */
        pDisk->ulBlockCount = pDiskSettings->ulLengthKB;
    }
    else
    {
        /*  For the length, if not aligned on an erase block boundary,
            round down to the nearest block boundary, and print a warning.
            Note the divide truncates for us, so we only have to do the
            check to print the warning on the debug build.
        */
        pDisk->ulBlockCount = pDiskSettings->ulLengthKB / ulBlockSizeKB;

      #if D_DEBUG > 0
        if((pDiskSettings->ulLengthKB % ulBlockSizeKB) != 0)
        {
            FFXPRINTF(1, ("Length is not on an erase block boundary - reducing.\n"));
        }
      #endif
    }

    return FFXSTAT_SUCCESS;
}

