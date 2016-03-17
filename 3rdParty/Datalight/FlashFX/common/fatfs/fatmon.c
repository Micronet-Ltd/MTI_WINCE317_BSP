/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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
 
    Unlike other storage media such as a hard disk, FlashFX must manage
    data on the virtual disk and periodically move things around.  To
    effectively do this with reasonable performance, FlashFX must "know"
    when it is no longer necessary to track a given piece of data.  Without
    this knowledge, FlashFX will continue to preserve and move around the
    dead data, until such time as it is overwritten with new data.  This
    results in extremely poor performance.
 
    Some file systems such as Datalight Reliance will inform FlashFX when
    data is being discarded via an ioctl function, however some common FAT
    implementations do not have this functionality.
 
    The FAT Monitor is a set of functionality that watches writes to the
    FAT table and examines them for clusters that are being freed.  These
    freed clusters are then discarded by calling vbfdiscard().
---------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------
                                Revision History
    $Log: fatmon.c $
    Revision 1.25  2011/11/17 02:07:51Z  garyp
    Updated to support sector sizes other than 512 bytes.
    Revision 1.24  2011/02/09 00:48:39Z  garyp
    Updated to use DCLDIMENSIONOF() rather than a predefined symbol.
    Revision 1.23  2009/07/21 22:01:23Z  garyp
    Merged from the v4.0 branch.  Modified to use the cleaned up and refactored
    BPB structures.
    Revision 1.22  2009/03/31 19:51:48Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.21  2009/02/13 00:05:06Z  garyp
    Updated to use the enhanced DclFatBPBMove() function.
    Revision 1.20  2009/02/09 01:20:33Z  garyp
    Updated to include dlpartid.h.
    Revision 1.19  2008/03/22 18:45:27Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Minor data type changes.
    Revision 1.18  2008/01/13 07:26:47Z  keithg
    Function header updates to support autodoc.
    Revision 1.17  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/09/27 00:01:09Z  jeremys
    Renamed several header files and function names.
    Revision 1.15  2007/05/08 21:14:55Z  rickc
    Fixed memory leak introduced in rev 1.14
    Revision 1.14  2007/05/01 19:48:12Z  rickc
    Removed deprecated calls to vbfread() and vbfclientsize()
    Revision 1.13  2006/10/16 20:32:14Z  Garyp
    Minor type changes.
    Revision 1.12  2006/10/11 19:11:10Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.11  2006/10/04 02:30:14Z  Garyp
    Updated to use DclMemAllocZero().  Updated to use the new printf macros.
    Revision 1.10  2006/07/27 20:12:01Z  johnb
    Added check for partition type of BIGFAT (FAT16 > 32MB)
    Revision 1.9  2006/03/17 22:39:04Z  Garyp
    Fixed to politely avoid initializing on disks with non-512-byte sectors.
    Revision 1.8  2006/03/16 03:54:19Z  Garyp
    Documentation fixes.
    Revision 1.7  2006/02/21 03:46:28Z  Garyp
    Modified to operate in terms of sectors rather than bytes.
    Revision 1.6  2006/02/12 20:26:24Z  Garyp
    Minor type change.
    Revision 1.5  2006/02/10 23:26:14Z  Garyp
    Updated to use new FML interface.
    Revision 1.4  2006/02/03 16:31:55Z  johnb
    Added FAT32 Support
    Revision 1.3  2006/01/12 02:22:20Z  Garyp
    Documentation, debug code, and general cleanup -- no functional changes.
    Revision 1.2  2006/01/05 03:36:34Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/10/22 07:04:14Z  Pauli
    Initial revision
    Revision 1.21  2005/07/27 21:19:20Z  Garyp
    Fixed FatMonWriteSectors() to avoid a length truncation problem that would
    result in some data not being discarded when it should be.
    Revision 1.20  2005/02/16 23:07:38Z  billr
    Different operating systems don't use the "hidden sectors" field in the BPB
    consistently, so it's not a reliable way to find out where the partition
    starts.  Get this information from the partition table instead.
    Revision 1.19  2004/12/30 17:32:43Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.18  2004/11/19 20:24:46Z  GaryP
    Updated to use refactored header files.
    Revision 1.17  2004/09/23 05:50:49Z  GaryP
    Updated to use the FAT_SECTORLEN value.
    Revision 1.16  2004/09/17 06:47:06Z  GaryP
    Moved some debugging code around.
    Revision 1.15  2004/08/30 20:26:45Z  GaryP
    Changed the file number.
    Revision 1.14  2004/08/24 03:52:32Z  GaryP
    Fixed FfxFatMonInit() to allow more than one initialization attempt.
    Revision 1.13  2004/08/13 22:58:40Z  GaryP
    Fixed so the various FATMON instances are initialized at FATMON init time
    rather than waiting for the first write.
    Revision 1.12  2004/08/13 19:23:21Z  GaryP
    Updated to use FfxDriverSectorDiscard() rather than discrete vbfdiscard()
    calls.
    Revision 1.11  2004/08/12 21:08:08Z  billr
    PFATMONINST is already defined in include/fxdriver.h, don't redefine it.
    Revision 1.10  2004/08/11 00:51:53Z  GaryP
    Updated the locking sequences and fixed a cut-and-paste bug.
    Revision 1.9  2004/08/10 07:20:38Z  garyp
    Added FfxFatMonGetBPB() and fixed some ADS specific warnings.
    Revision 1.8  2004/08/06 23:48:10Z  GaryP
    Updated to support multiple drives and multithreaded access.
    Revision 1.7  2004/01/14 02:49:28Z  garys
    Merge from FlashFXMT
    Revision 1.4.1.3  2004/01/14 02:49:28  garyp
    Updated to accomodate some renamed structure fields.
    Revision 1.4.1.2  2003/11/01 02:21:22Z  garyp
    Re-checked into variant sandbox.
    Revision 1.5  2003/11/01 02:21:22Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.4  2003/05/29 17:48:06Z  garyp
    Updated to use VBF_MAX_TRANSFER_SIZE.
    Revision 1.3  2003/05/01 22:53:14Z  garyp
    Updated debugging code.
    Revision 1.2  2003/04/16 02:19:02Z  garyp
    Updated the file number.
    Revision 1.1  2003/03/16 22:14:00Z  garyp
    Initial revision
    Revision 1.4  2003/03/16 23:14:02Z  garyp
    Minor header file changes.
    Revision 1.3  2003/02/07 22:07:44Z  garyp
    Added debugging messages.
    08/29/02 gp  Fixed a problem where we were indexing past the end of the
                 first FAT sector due to the two reserved entries.  Fixed a
                 problem on FAT12 systems where we were processing one extra
                 FAT entry on some sectors.  Added a bunch of asserts to
                 ensure that doesn't happen again.
    08/21/02 gp  Fixed a compiler warning that only shows up on BCC32.
    08/06/02 gp  Major FATMon update so that we only call vbfdiscard() for
                 clusters that went from being allocated to being freed.
                 Additionally, concatenate adjacent freed clusters so we
                 call vbfdiscard() as few times as possible.
    06/19/01 gp  Added FFXGetFileSystem().
    07/11/00 mrm Added conditional compile test to this file.
    06/24/99 PKQ Corrected overflow in calculation for ulClientAddress
                 in FfxFatMonWrite()
    12/22/98 PKG Corrected multi-sector FAT write parsing
                 that caused valid data to be discarded when it shouldn't.
                 Problem exercised by CHKDSK when deleting lost clusters
    02/19/98 PKG Now 100% portable, uses byte offsets and values
    01/12/98 PKG Fixed the byte offset used to discard cluster data
                 that is larger than a D_UINT16
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    08/26/97 PKG Now uses new Datalight specific types
    08/07/97 PKG No longer uses BLOCK_SIZE since it is not available
    06/19/97 PKG Ouch!  The client address discarded was not correctly
                 calculated causing the wrong data to be discarded in
                 multi-sector writes to the FAT area.
    03/18/97 PKG Ported here from v3.01, major code cleanup
---------------------------------------------------------------------------*/
 
#include <flashfx.h>
 
#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT
#if FFXCONF_FATMONITORSUPPORT
 
#include <fxdriver.h>
#include <dlfat.h>
#include <dlpartid.h>
#include <fxfatapi.h>
#include <fatmon.h>
#include <vbf.h>
 
#if D_DEBUG == 0
  #define FATMON_DEBUGGING FALSE  /* must be FALSE for checkin */
#else
  #define FATMON_DEBUGGING FALSE  /* set to TRUE to enable FATMON messages */
#endif
 
typedef struct tagFATMONINST
{
    PFATMONINST     pNextInst;
    PDCLMUTEX       pMutex;
    VBFDISKINFO     VbfDiskInfo;
    D_UINT16        uPartNum;
    DCLFATBPB       bpb;
    D_BYTE          bFSID;
    D_UINT32        ulCachedSectorNum;
    D_BUFFER       *pCachedFATSector;
} FATMONINST;
 
/*  Following are some useful macros used to examine FAT entries
*/
#define FATENTRY2CLUSTER(offset, nibbles) ( (offset) / (nibbles) )
#define CLUSTER2FATENTRY(clus, nibbles) ( (clus) * (nibbles) )
 
 
/*-------------------------------------------------------------------
------------------- Prototypes for private functions  ---------------
-------------------------------------------------------------------*/
 
static PFATMONINST  FatMonInstanceCreate(   FFXDISKHANDLE hDisk, D_UINT16 uPartitionNum, const D_BUFFER *pBuffer);
static void         FatMonInstanceDestroy(  FFXDISKHANDLE hDisk, PFATMONINST pFMI);
static D_BOOL       FatMonWriteSectors(     FFXDISKHANDLE hDisk, PFATMONINST pFMI, D_UINT32 ulStartPage, D_UINT32 ulPageCount, const D_BUFFER *pBuffer);
static D_BOOL       FatMonWriteFatSectors(  FFXDISKHANDLE hDisk, PFATMONINST pFMI, D_UINT32 ulStartPage, D_UINT32 ulPageCount, const D_BUFFER *pBuffer);
static void         FatMonDiscardClusters(  FFXDISKHANDLE hDisk, PFATMONINST pFMI, D_UINT32 ulSectorNum, D_BUFFER *pOldSector, const D_BUFFER *pNewSector);
static D_UINT16     FatMonGetFatEntry(      const D_BUFFER *pBuffer, D_UINT16 uNibbleOffset, D_UINT16 uNumNibbles);
static D_UINT32     FatMonGetFat32Entry(    const D_BUFFER *pcBuffer, D_UINT16 uNibbleOffset, D_UINT16 uNumNibbles);
 
 
/*-------------------------------------------------------------------
-------------------     Public FATMON Functions     -----------------
-------------------------------------------------------------------*/
 
 
/*-------------------------------------------------------------------
    Public: FfxFatMonInit()
 
    Initialize the FAT Monitor functionality.  This is a one-time
    initialization for each FlashFX Disk, that should be paired
    with a corresponding call to FfxFatMonDeinit().
 
    Parameters:
        hDisk   - The disk handle
 
    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFatMonInit(
    FFXDISKHANDLE   hDisk)
{
    char            szMutex[DCL_MUTEXNAMELEN];
    D_BOOL          fUseFM;
    D_UINT16        k;
    VBFDISKINFO     VbfInfo;
    FFXSTATUS       ffxStat;
    FFXIOSTATUS     ioStat;
    D_BUFFER       *pSector = NULL;
 
  #if FATMON_DEBUGGING
    FFXPRINTF(1,("FfxFatMonInit()\n"));
  #endif
 
    /*  If already loaded, just return success...
    */
    if((*hDisk)->pFatMonMutex)
        return FFXSTAT_SUCCESS;
 
    /*  Even though the OS Layer is configured to compile in the FAT Monitor
        logic, it may be disabled at run-time.
    */
    if(FfxHookOptionGet(FFXOPT_USEFATMONITOR, hDisk, &fUseFM, sizeof fUseFM))
    {
        if(!fUseFM)
            return FFXSTAT_FATMONDISABLED;
    }
 
    FFXPRINTF(1,("FATMON initializing for DISK%u\n", (*hDisk)->Conf.nDiskNum));
 
    ffxStat = FfxVbfDiskInfo((*hDisk)->hVBF, &VbfInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclAssert(VbfInfo.uPageSize == (*hDisk)->ulSectorLength);
 
    /*  Allocate a mutex protecting the FATMON instance for
        this device.
    */
    DclAssert((*hDisk)->Conf.nDiskNum <= 0xFF);
    DclSNPrintf(szMutex, sizeof(szMutex), "FMFMD%02x", (*hDisk)->Conf.nDiskNum);
 
    (*hDisk)->pFatMonMutex = DclMutexCreate(szMutex);
    if(!(*hDisk)->pFatMonMutex)
    {
        DclError();
 
        return DCLSTAT_MUTEXCREATEFAILED;
    }
 
    /*  Loop through our partition entries to see if we need to create any
        FATMON instances.
    */
    for(k = 0; k < DCLDIMENSIONOF((*hDisk)->fpdi.fpi); k++)
    {
        /*  Is the boot record for this partition being written to?
        */
        if(((*hDisk)->fpdi.fpi[k].bFSID == FSID_FAT12) ||
           ((*hDisk)->fpdi.fpi[k].bFSID == FSID_FAT16) ||
           ((*hDisk)->fpdi.fpi[k].bFSID == FSID_BIGFAT) ||
           ((*hDisk)->fpdi.fpi[k].bFSID == FSID_FAT32))
        {
            D_BYTE              bFSID;
            FFXPHYSICALDISKINFO pdi;
 
            if(!pSector)
            {
                pSector = DclMemAlloc(VbfInfo.uPageSize);
                if(!pSector)
                    return DCLSTAT_MEMALLOCFAILED;
            }
 
            /*  Read the boot record for the partition
            */
            ioStat = FfxVbfReadPages((*hDisk)->hVBF, (*hDisk)->fpdi.fpi[k].ulStartSector, 1, pSector);
            if(!IOSUCCESS(ioStat, 1))
            {
                FFXPRINTF(1, ("Unable to read boot record\n"));
                continue;
            }
 
            /*  If this is a FAT12 or FAT16 boot record, and the
                BytesPerSector value is 512, create a new FATMON instance.
            */
            bFSID = FfxFatGetDiskParamsFromBootRecord(pSector, &pdi);
            if((bFSID == FSID_FAT12) || (bFSID == FSID_FAT16) ||
               (bFSID == FSID_BIGFAT) || (bFSID == FSID_FAT32))
            {
                if(!FatMonInstanceCreate(hDisk, k, pSector))
                {
                    DclError();
                }
            }
            else
            {
                FFXPRINTF(1, ("MBR describes a FAT partition but boot record does not match\n"));
            }
        }
    }
 
    if(pSector)
        DclMemFree(pSector);
 
    return FFXSTAT_SUCCESS;
}
 
 
/*-------------------------------------------------------------------
    Public: FfxFatMonDeinit()
 
    Deinitialize the FAT Monitor functionality.
 
    Parameters:
        hDisk   - The disk handle
 
    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFatMonDeinit(
    FFXDISKHANDLE   hDisk)
{
    D_BOOL          fSuccess;
 
    if(!(*hDisk)->pFatMonMutex)
        return FFXSTAT_SUCCESS;
 
    FFXPRINTF(1, ("FATMON deinitializing for DISK%u\n", (*hDisk)->Conf.nDiskNum));
 
    /*  Destroy any instances that were created for this device
    */
    DclMutexAcquire((*hDisk)->pFatMonMutex);
    {
        while((*hDisk)->pFMI)
            FatMonInstanceDestroy(hDisk, (*hDisk)->pFMI);
    }
    DclMutexRelease((*hDisk)->pFatMonMutex);
 
    fSuccess = DclMutexDestroy((*hDisk)->pFatMonMutex);
 
    (*hDisk)->pFatMonMutex = 0;
 
    if(fSuccess)
        return FFXSTAT_SUCCESS;
    else
        return DCLSTAT_MUTEXDESTROYFAILED;
}
 
 
/*-------------------------------------------------------------------
    Public: FfxFatMonWrite()

    Write data using FAT monitor. Write some client data into
    the media.  The write may start in the FAT.  Any writes
    to the FAT will be identified and any FAT entries that
    mark the data as available for allocation will be discarded.

    Parameters:
        hDisk    - The disk handle
        ulStart  - Starting offset in bytes to begin the write.
        uLength  - Length of the write in bytes.
        pcBuffer - Pointer into the client data buffer.
 
    Return Value:
        TRUE - If the write and discards were successful.
        FALSE - If the write failed.
-------------------------------------------------------------------*/
D_BOOL FfxFatMonWrite(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    const D_BUFFER *pBuffer)
{
    FFXIOSTATUS     ioStat;
 
    /*  Normally this code will always get entered when FAT Monitor is
        compiled into the code base.  However it can be disabled at
        run-time in certain OESLs.  In this case, the pFatMonMutex
        value will be NULL.
    */
    if((*hDisk)->pFatMonMutex)
    {
        D_UINT32        ulStartSector = ulStartPage;
        D_UINT16        k;
        PFATMONINST     pFMI;
 
        /*  TODO: We should be examining any write to the MBR to determine
            if partitions are being created or destroyed, and fix our
            tables accordingly.
        */
 
        /*  Loop through our partition entries to see if the first sector
            of any partition is being written to.
        */
        for(k = 0; k < DCLDIMENSIONOF((*hDisk)->fpdi.fpi); k++)
        {
            /*  Is the boot record for this partition being written to?
            */
            if(((*hDisk)->fpdi.fpi[k].bFSID != FSID_NONE) &&
               ((*hDisk)->fpdi.fpi[k].ulStartSector == ulStartSector))
            {
                D_BYTE              bFSID;
                FFXPHYSICALDISKINFO pdi;
 
                /*  Yes, so delete any previous FATMON instance for
                    this partition.
                */
                DclMutexAcquire((*hDisk)->pFatMonMutex);
                {
                    pFMI = (*hDisk)->pFMI;
                    while(pFMI)
                    {
                        if(pFMI->uPartNum == k)
                        {
                            FatMonInstanceDestroy(hDisk, pFMI);
                            break;
                        }
 
                        pFMI = pFMI->pNextInst;
                    }
                }
                DclMutexRelease((*hDisk)->pFatMonMutex);
 
                /*  If this is a FAT boot record we can handle, create 
                    a new FATMON instance.
                */
                bFSID = FfxFatGetDiskParamsFromBootRecord(pBuffer, &pdi);
                if((bFSID == FSID_FAT12) || (bFSID == FSID_FAT16) ||
                   (bFSID == FSID_BIGFAT) || (bFSID == FSID_FAT32))
                {
                    if(!FatMonInstanceCreate(hDisk, k, pBuffer))
                    {
                        DclError();
                    }
                }
 
                break;
            }
        }
 
        /*  Now iterate through our chain of FATMON instances, and if this write
            falls within a logical partition that is managed by a FATMON instance,
            write the sectors using FATMON.
        */
        DclMutexAcquire((*hDisk)->pFatMonMutex);
        {
            pFMI = (*hDisk)->pFMI;
            while(pFMI)
            {
                FFXPARTITIONINFO *pFPI = &(*hDisk)->fpdi.fpi[pFMI->uPartNum];
 
                if((ulStartSector > pFPI->ulStartSector) &&
                   (ulStartSector < pFPI->ulStartSector + pFPI->ulSectorCount))
                {
                    D_BOOL          fSuccess;
 
                    /*  Ensure nobody else is trying to write to this FAT partition
                        while we are mucking with it.  The granularity of this
                        locking could be improved by moving these locks closer to
                        the relevant objects...
                    */
                    DclMutexAcquire(pFMI->pMutex);
 
                    /*  No longer need to protect the FATMON instance chain
                    */
                    DclMutexRelease((*hDisk)->pFatMonMutex);
 
                    fSuccess = FatMonWriteSectors(hDisk, pFMI, ulStartPage, ulPageCount, pBuffer);
 
                    DclMutexRelease(pFMI->pMutex);
 
                    return fSuccess;
                }
 
                pFMI = pFMI->pNextInst;
            }
        }
        DclMutexRelease((*hDisk)->pFatMonMutex);
    }
 
    /*  If we get here, the area of the disk is not being managed by FATMON,
        so just write the data using VBF.
    */
    ioStat = FfxVbfWritePages((*hDisk)->hVBF, ulStartPage, ulPageCount, pBuffer);
    if(IOSUCCESS(ioStat, ulPageCount))
        return TRUE;
    else
        return FALSE;
}
 
 
/*-------------------------------------------------------------------
    Public: FfxFatMonGetBPB()

    Get the BPB for a logical partition.

    Parameters:
        hDisk    - The disk handle
        uPartNum - The logical partition number.
        pBPB     - A pointer to the DCLFATBPB structure to use.
 
    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFatMonGetBPB(
    FFXDISKHANDLE   hDisk,
    D_UINT16        uPartNum,
    DCLFATBPB      *pBPB)
{
    PFATMONINST     pFMI;
 
    DclAssert((*hDisk));
    DclAssert(pBPB);
 
    if(!(*hDisk)->pFatMonMutex)
        return FFXSTAT_FATMONDISABLED;
 
    DclMutexAcquire((*hDisk)->pFatMonMutex);
    {
        pFMI = (*hDisk)->pFMI;
        while(pFMI)
        {
            if(pFMI->uPartNum == uPartNum)
            {
                DclMutexAcquire(pFMI->pMutex);
 
                /*  No longer need to protect the FATMON instance chain
                */
                DclMutexRelease((*hDisk)->pFatMonMutex);
 
                *pBPB = pFMI->bpb;
 
                DclMutexRelease(pFMI->pMutex);
 
                return FFXSTAT_SUCCESS;
            }
 
            pFMI = pFMI->pNextInst;
        }
    }
    DclMutexRelease((*hDisk)->pFatMonMutex);
 
    return FFXSTAT_FATMONPARTITIONNOTFOUND;
}
 
 
 
/*-------------------------------------------------------------------
----------------     Private FATMON Functions     -------------------
-------------------------------------------------------------------*/
 
 
 
/*-------------------------------------------------------------------
    Local: FatMonInstanceCreate()

    Create an instance of the FAT monitor.

    Parameters:
        hDisk    - The disk handle
        uPartNum - The logical partition number (0-n).
        pBuffer  - A pointer to the boot record.
 
    Return Value:
        Returns a pointer to the FATMONINST structure if successful,
        else NULL.
-------------------------------------------------------------------*/
static PFATMONINST FatMonInstanceCreate(
    FFXDISKHANDLE   hDisk,
    D_UINT16        uPartitionNum,
    const D_BUFFER *pBuffer)
{
    PFATMONINST     pFMI;
    char            szMutex[DCL_MUTEXNAMELEN];
    FFXSTATUS       ffxStat;
 
    FFXPRINTF(1, ("FATMON creating instance for partition %U\n", uPartitionNum));
 
  #if D_DEBUG
    /*  We should never be creating an instance for a logical partition
        that already has one.
    */
    DclMutexAcquire((*hDisk)->pFatMonMutex);
    {
        pFMI = (*hDisk)->pFMI;
        while(pFMI)
        {
            DclAssert(pFMI->uPartNum != uPartitionNum);
 
            pFMI = pFMI->pNextInst;
        }
    }
    DclMutexRelease((*hDisk)->pFatMonMutex);
  #endif
 
    pFMI = DclMemAllocZero(sizeof *pFMI);
    if(!pFMI)
        return NULL;
    
    ffxStat = FfxVbfDiskInfo((*hDisk)->hVBF, &pFMI->VbfDiskInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclMemFree(pFMI);
        return NULL;
    }

    DclAssert(pFMI->VbfDiskInfo.uPageSize == (*hDisk)->ulSectorLength);
    
    DclAssert((*hDisk)->Conf.nDiskNum <= 0xFF);
    DclAssert(uPartitionNum <= 0xF);
    DclSNPrintf(szMutex, sizeof(szMutex), "FMFM%02x%1X", (*hDisk)->Conf.nDiskNum, uPartitionNum);
 
    pFMI->pMutex = DclMutexCreate(szMutex);
    if(!pFMI->pMutex)
    {
        DclMemFree(pFMI);
        return NULL;
    }
  
    pFMI->pCachedFATSector = DclMemAlloc(pFMI->VbfDiskInfo.uPageSize);
    if(!pFMI->pCachedFATSector)
    {
       DclMutexDestroy(pFMI->pMutex);
       DclMemFree(pFMI);
       return NULL;
    }
  
    pFMI->uPartNum = uPartitionNum;
 
    /*  WARNING! Even though the partition detection code accommodates
                 FSID_BIGFAT, this function will never return that value,
                 and therefore the meat of the FATMON code works OK even
                 though it does not explicitly check for FSID_BIGFAT.
                 If this function is ever updated to return FSID_BIGFAT,
                 then the remainder of FATMON needs to be updated do
                 recognize that.
    */
    pFMI->bFSID = DclFatBPBMove(&pFMI->bpb, pBuffer);
 
  #if D_DEBUG
    DclFatBPBDisplay(&pFMI->bpb);
  #endif
 
    /*  Hook ourselves at the beginning of the chain.
    */
    DclMutexAcquire((*hDisk)->pFatMonMutex);
    pFMI->pNextInst = (*hDisk)->pFMI;
    (*hDisk)->pFMI = pFMI;
    DclMutexRelease((*hDisk)->pFatMonMutex);
 
    return pFMI;
}
 
 
/*-------------------------------------------------------------------
    Local: FatMonInstanceDestroy()

    Destroy an instance of the FAT monitor.

    Parameters:
        hDisk    - The disk handle
        pFMI     - A pointer to the FATMONINST stucture to use.
 
    Return Value:
        Returns a pointer to the FATMONINST structure if successful,
        else NULL.
-------------------------------------------------------------------*/
static void FatMonInstanceDestroy(
    FFXDISKHANDLE   hDisk,
    PFATMONINST     pFMI)
{
    DclAssert((*hDisk));
    DclAssert(pFMI);
    DclAssert((*hDisk)->pFMI);
 
    /*  Acquire access to the object to ensure nobody
        else is using it.
    */
    DclMutexAcquire(pFMI->pMutex);
 
    FFXPRINTF(1, ("FATMON destroying instance for partition %U\n", pFMI->uPartNum));
 
    if((*hDisk)->pFMI == pFMI)
    {
        /*  Special case if we are at the head of the chain.
        */
        (*hDisk)->pFMI = pFMI->pNextInst;
    }
    else
    {
        PFATMONINST     pThisInst;
 
        /*  Otherwise, loop through the chain until we find the instance
            prior to us and patch him to point past us.
        */
        pThisInst = (*hDisk)->pFMI;
        while(pThisInst)
        {
            if(pThisInst->pNextInst == pFMI)
            {
                pThisInst->pNextInst = pFMI->pNextInst;
 
                break;
            }
            else
            {
                pThisInst = pThisInst->pNextInst;
 
                /*  We should only assert if we are not in the chain at all
                    which should never happen.
                */
                DclAssert(pThisInst);
            }
        }
    }
 
    DclMemFree(pFMI->pCachedFATSector);
 
    DclMutexRelease(pFMI->pMutex);
    DclMutexDestroy(pFMI->pMutex);
 
    DclMemFree(pFMI);
 
    return;
}
 
 
/*-------------------------------------------------------------------
    Local: FatMonWriteSectors()

    Write sectors using FAT monitor. Write some client data
    into the media.  The write may start in the FAT.  Any
    writes to the FAT will be identified and any FAT entries
    that mark the data as available for allocation will be
    discarded.

    Parameters:
        hDisk    - The disk handle
        pFMI     - A pointer to the FATMONINST structure to use
        ulStart  - Starting offset in bytes to begin the write.
        uLength  - Length of the write in bytes.
        pcBuffer - Pointer into the client data buffer.
 
    Return Value:
        TRUE - If the write and discards were successful.
        FALSE - If the write failed.
-------------------------------------------------------------------*/
static D_BOOL FatMonWriteSectors(
    FFXDISKHANDLE   hDisk,
    PFATMONINST     pFMI,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    const D_BUFFER *pcBuffer)
{
    D_UINT32        ulFat1SecStart;
    D_UINT32        ulFat2SecStart;
    D_UINT32        ulThisSector;
    D_BOOL          fReturn = FALSE;
    FFXIOSTATUS     ioStat;
    D_UINT32        ulCount;
 
  #if FATMON_DEBUGGING
    FFXPRINTF(3,("FATMON: FfxFatMonWriteSectors StartPage=%lX Count=%lX\n",
        ulStartPage, ulPageCount));
  #endif
 
    DclAssert(pFMI);
    DclAssert(pcBuffer);
    DclAssert(ulPageCount);
 
    /*  Determine if we are writing into the FAT
    */
    ulFat1SecStart = pFMI->bpb.uReservedSectors + (*hDisk)->fpdi.fpi[pFMI->uPartNum].ulStartSector;
    if (pFMI->bFSID == FSID_FAT12 || pFMI->bFSID == FSID_FAT16)
    {
        ulFat2SecStart = ulFat1SecStart + pFMI->bpb.uSecPerFAT;
    }
    else
    {
        DclAssert(pFMI->bFSID == FSID_FAT32);
        ulFat2SecStart = ulFat1SecStart + pFMI->bpb.ext.bpb32.ulSectorsPerFat;
    }

    ulThisSector = ulStartPage;
 
    DclAssert(ulFat1SecStart);
    DclAssert(ulFat2SecStart);
 
    /*  Process any writes that preceed the first FAT
    */
    if(ulThisSector < ulFat1SecStart)
    {
        /*  Invalidate any saved FAT sector
        */
        pFMI->ulCachedSectorNum = 0;
 
        /*  Calc pages to write
        */
        ulCount = DCLMIN(ulFat1SecStart - ulThisSector, ulPageCount);
 
        /*  Write the data
        */
        ioStat = FfxVbfWritePages((*hDisk)->hVBF, ulStartPage, ulCount, pcBuffer);
        if(!IOSUCCESS(ioStat, ulCount))
            goto FSWCleanup;
 
        /*  Adjust pointers and remaining length, if any
        */
        ulPageCount -= ulCount;
 
        /*  Exit if nothing left to write
        */
        if(!ulPageCount)
            goto FSWSuccess;
 
        ulThisSector    += ulCount;
        ulStartPage     += ulCount;
        pcBuffer        += ulCount * pFMI->VbfDiskInfo.uPageSize;
    }
 
    /*  Now process any data that we are writing into the first FAT
    */
    if(ulThisSector < ulFat2SecStart)
    {
        DclAssert(ulThisSector >= ulFat1SecStart);
 
        /*  Calc pages to write
        */
        ulCount = DCLMIN(ulFat2SecStart - ulThisSector, ulPageCount);
 
        if(!(FatMonWriteFatSectors(hDisk, pFMI, ulStartPage, ulCount, pcBuffer)))
            goto FSWCleanup;
 
        /*  Adjust pointers and remaining length, if any
        */
        ulPageCount -= ulCount;
 
        /*  Exit if nothing left to write
        */
        if(!ulPageCount)
            goto FSWSuccess;
 
        ulStartPage     += ulCount;
        pcBuffer        += ulCount * pFMI->VbfDiskInfo.uPageSize;
    }
 
    /*  Process any writes that follow the first FAT
    */
    if(ulPageCount)
    {
        /*  Invalidate any saved FAT sector
        */
        pFMI->ulCachedSectorNum = 0;
 
        /*  Write any remaining data
        */
        ioStat = FfxVbfWritePages((*hDisk)->hVBF, ulStartPage, ulPageCount, pcBuffer);
        if(!IOSUCCESS(ioStat, ulPageCount))
            goto FSWCleanup;
    }
 
  FSWSuccess:
    fReturn = TRUE;
 
  FSWCleanup:
 
    return fReturn;
}
 
 
/*-------------------------------------------------------------------
    Local: FatMonWriteFatSectors()

    Write sectors to the FAT portion of the disk. Write some
    client data into the media.  The write may start in the FAT.
    Any writes to the FAT will be identified and any FAT entries
    that mark the data as available for allocation will be discarded.

    Parameters:
        hDisk    - The disk handle
        pFMI     - A pointer to the FATMONINST structure to use
        ulStart  - Starting offset in bytes to begin the write.
        uLength  - Length of the write in bytes.
        pcBuffer - Pointer into the client data buffer.
 
    Return Value:
        TRUE - If the write and discards were successful.
        FALSE - If the write failed.
-------------------------------------------------------------------*/
static D_BOOL FatMonWriteFatSectors(
    FFXDISKHANDLE   hDisk,
    PFATMONINST     pFMI,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    const D_BUFFER *pcBuffer)
{
    D_UINT32        ulThisSector;
    D_UINT32        ulSectorCount;
    FFXIOSTATUS     ioStat;
 
  #if FATMON_DEBUGGING
    FFXPRINTF(2, ("FATMON: FatMonWriteFatSectors StartPage=%lX Count=%lX\n",
        ulStartPage, ulPageCount));
  #endif
 
    DclAssert(pFMI);
    DclAssert(pcBuffer);
    DclAssert(ulPageCount);
 
    ulThisSector = ulStartPage;
    ulSectorCount = ulPageCount;
 
    while(ulSectorCount)
    {
        /*  Determine if the sector in our cache is the same
            one we are now writing.  If not load the sector.
        */
        if(ulThisSector != pFMI->ulCachedSectorNum)
        {
            /*  Refresh our cached sector with the original data
                from the new FAT sector we are about to write.
            */
            ioStat = FfxVbfReadPages((*hDisk)->hVBF, ulStartPage, 1, pFMI->pCachedFATSector);
            if(IOSUCCESS(ioStat, 1))
            {
                /*  indicate that we have something in the cache
                */
                pFMI->ulCachedSectorNum = ulThisSector;
            }
            else
            {
                /*  What happens if FfxVbfReadPages() fails?  For now we just
                    let it, and the cache remains empty.
                */
                DclError();
 
                pFMI->ulCachedSectorNum = 0;
            }
        }
 
        /*  Write the new FAT sector to disk.  The original FAT
            sector is in our cache.
        */
        ioStat = FfxVbfWritePages((*hDisk)->hVBF, ulStartPage, 1, pcBuffer);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;
 
        /*  If we have the original sector in our cache, iterate
            through the FAT entries, discarding any clusters that
            have gone from allocated to free.
        */
        if(pFMI->ulCachedSectorNum)
        {
            FatMonDiscardClusters(hDisk, pFMI, pFMI->ulCachedSectorNum,
                                  pFMI->pCachedFATSector, pcBuffer);
        }
 
        /*  The new FAT sector we just wrote now becomes the
            data in our cache.
        */
        DclMemCpy(pFMI->pCachedFATSector, pcBuffer, pFMI->VbfDiskInfo.uPageSize);
 
        ulSectorCount--;
        ulThisSector++;
 
        ulStartPage++;
        pcBuffer += pFMI->VbfDiskInfo.uPageSize;
    }
 
    return TRUE;
}
 
 
/*-------------------------------------------------------------------
    Local: FatMonDiscardClusters()
 
    Discard unused clusters detected by FATMON. Write some client
    data into the media.  The write may start in the FAT.  Any
    writes to the FAT will be identified and any FAT entries that
    mark the data as available for allocation will be discarded.
 
    Note that for FAT12 disks, there will be FAT entries that span
    FAT sectors, these entries will not be processed.
 
    Parameters:
        hDisk       - The disk handle
        pFMI        - A pointer to the FATMONINST structure to use
        ulSectorNum - The disk sector being processed.
        pcOldSector - A pointer to The original FAT sector data.
        pcNewSector - A pointer to the new FAT sector data.
 
    Return Value:
        None
-------------------------------------------------------------------*/
static void FatMonDiscardClusters(
    FFXDISKHANDLE   hDisk,
    PFATMONINST     pFMI,
    D_UINT32        ulSectorNum,
    D_BUFFER       *pcOldSector,
    const D_BUFFER *pcNewSector)
{
    D_UINT16        uNibble;    /* absolute nibble within the FAT */
    D_UINT32        ulOldFatEntry;
    D_UINT32        ulNewFatEntry;
    D_UINT16        uNibblesPerEntry;   /* nibbles per each FAT entry */
    D_UINT32        ulStartCluster;
    D_UINT32        ulMaxClusterNum;
    D_UINT16        uSectorsPerCluster;
    D_UINT16        uFatEntriesLeft;    /* per sector */
    D_UINT16        uEntriesFound;
    D_UINT32        ulNibbleOffset;
    D_UINT32        ulFat1SecStart;
    D_UINT32        ulDataSecStart;
    D_UINT32        ulTotalDiskSectors;
    D_UINT32        ulClientPage;
    D_UINT32        ulSectorCount;
 
  #if FATMON_DEBUGGING
    FFXPRINTF(1, ("FATMON: FatMonDiscardClusters SectorNum=%08lX\n", ulSectorNum));
  #endif
 
    DclAssert(pFMI);
 
    ulFat1SecStart = pFMI->bpb.uReservedSectors + (*hDisk)->fpdi.fpi[pFMI->uPartNum].ulStartSector;
 
    /*  Assert that we are writing into FAT1 and only into FAT1
    */
    DclAssert(ulFat1SecStart);
    DclAssert(ulSectorNum >= ulFat1SecStart);
 
    /*  Some more useful information
    */
    if (pFMI->bFSID == FSID_FAT12 || pFMI->bFSID == FSID_FAT16)
    {
        DclAssert(ulSectorNum < ulFat1SecStart + pFMI->bpb.uSecPerFAT);
        ulDataSecStart =  ulFat1SecStart + (pFMI->bpb.uSecPerFAT * pFMI->bpb.bNumFats) +
            (pFMI->bpb.uMaxDirs * 0x20 / pFMI->VbfDiskInfo.uPageSize);
    }
    else
    {
        DclAssert(pFMI->bFSID == FSID_FAT32);
        DclAssert(ulSectorNum < ulFat1SecStart + pFMI->bpb.ext.bpb32.ulSectorsPerFat);
 
        /*  FAT32 -- uMaxDirs is 0
        */
        ulDataSecStart = ulFat1SecStart + (pFMI->bpb.ext.bpb32.ulSectorsPerFat * pFMI->bpb.bNumFats);
    }
 
    uSectorsPerCluster = (D_UINT16)pFMI->bpb.bSecPerCluster;
 
    /*  Determine the max cluster number
    */
    ulTotalDiskSectors = (D_UINT16)pFMI->bpb.uTotalSecs;
 
    if(ulTotalDiskSectors == 0L)
        ulTotalDiskSectors = pFMI->bpb.ulTotalSecs;
 
    /*  We add 2 to the max cluster number because the first two cluster
        entries are reserved.  The first cluster number is always 2.
    */
    ulMaxClusterNum = ((ulTotalDiskSectors - ulDataSecStart) / uSectorsPerCluster) + 2;
 
    /*  Determine size of a single FAT entry
    */
    if (pFMI->bFSID == FSID_FAT12 || pFMI->bFSID == FSID_FAT16)
    {
        if(ulMaxClusterNum < FAT12_MAXCLUSTERS)
            uNibblesPerEntry = 3;
        else
            uNibblesPerEntry = 4;
    }
    else
    {
        DclAssert(pFMI->bFSID == FSID_FAT32);
        uNibblesPerEntry = 8;
    }
 
    ulNibbleOffset = (ulSectorNum - ulFat1SecStart) * pFMI->VbfDiskInfo.uPageSize * 2;
 
    ulStartCluster = FATENTRY2CLUSTER((ulNibbleOffset + uNibblesPerEntry - 1), uNibblesPerEntry);
 
  #if FATMON_DEBUGGING
    FFXPRINTF(3, ("FATMON: FAT1StartSec=%lU DataStartSec=%lU TotalSecs=%lU SecPerClus=%U MaxClusterNum=%lU\n",
               ulFat1SecStart, ulDataSecStart, ulTotalDiskSectors,
               uSectorsPerCluster, ulMaxClusterNum));
    FFXPRINTF(3, ("FATMON: NibblesPerEntry=%U NibbleOffset=%lU StartClus=%lU\n",
               uNibblesPerEntry, ulNibbleOffset, ulStartCluster));
  #endif
 
    /*  Check for out of bounds entries
    */
    if(ulStartCluster >= ulMaxClusterNum)
    {
        /*  We have seen certain file systems such as the DOSFS file
            system in VxWorks that will allocate more sectors to the FAT
            than is necessary to represent all the clusters.  The above
            test will catch this.  Don't assert, just return.
        */
 
        /*  DclError();
        */
 
        return;
    }
 
    if(ulStartCluster < 2)
        ulStartCluster = 2;
 
    /*  Determine the starting nibble within the given FAT sector
    */
    uNibble = (D_UINT16)CLUSTER2FATENTRY(ulStartCluster, uNibblesPerEntry);
        uNibble &= (pFMI->VbfDiskInfo.uPageSize * 2 - 1);
 
 
    ulClientPage = ulDataSecStart + ((D_UINT32) (ulStartCluster - 2) * uSectorsPerCluster);

    DclAssert(ulClientPage >= ulDataSecStart);
    DclAssert(ulClientPage < (*hDisk)->pFMI->VbfDiskInfo.ulTotalPages);
 
    /*  Determine the number of FAT entries in this sector
    */
    uFatEntriesLeft = (pFMI->VbfDiskInfo.uPageSize * 2) / uNibblesPerEntry;
 
    if(ulSectorNum == ulFat1SecStart)
    {
        /*  The first two FAT entries in the first FAT sector are reserved.
            We must adjust uFatEntriesLeft for the first FAT sector because
            our master index "uNibble" has already been adjusted to skip
            those entries (based on uStartCluster above).
        */
 
        uFatEntriesLeft -= 2;
    }
    else
    {
        /*  In all FAT sectors other than the first one, uNibble will point
            to the nibble for the first complete FAT entry.  For FAT16 disks
            this will always be 0, and we don't need to do anything further.
 
            However, for FAT12 disks, things are not so simple:
 
            A) For sectors where the first FAT entry falls at the beginning
            of the sector, or one nibble into the sector, the sector will
            be able to contain 341 complete entries, with zero of one
            trailing nibbles.
 
            B) For those sectors where the first FAT entry falls two nibbles
            into the sector, there will always be two nibbles at the trailing
            edge of the sector as well, meaning that there are only 340
            complete FAT entries contained in the sector.
 
            Since we can only process complete FAT entries (those spanning
            FAT sectors will be ignored) we must determine which case we fall
            into and potentially decrement our uFatEntriesLeft value.
        */
 
        if (uNibblesPerEntry == 8)
        {
            /*  Assert that what we stated regarding FAT16 disks above is true.
            */
            DclAssert(uFatEntriesLeft == pFMI->VbfDiskInfo.uPageSize / 4);
            DclAssert(uNibble == 0);
        }
        else if(uNibblesPerEntry == 4)
        {
            /*  Assert that what we stated regarding FAT16 disks above is true.
            */
            DclAssert(uFatEntriesLeft == pFMI->VbfDiskInfo.uPageSize / 2);
            DclAssert(uNibble == 0);
        }
        else
        {
            /*  Assert that what we stated regarding FAT12 disks above is true.
            */
            DclAssert(uFatEntriesLeft == (pFMI->VbfDiskInfo.uPageSize * 2) / 3);
            DclAssert(uNibblesPerEntry == 3);
            DclAssert(uNibble <= 2);
 
            /*  If we're starting by skipping a partial entry of two nibbles,
                adjust the total count of complete FAT entries to process (in
                this sector).
            */
            if(uNibble == 2)
                uFatEntriesLeft--;
        }
    }
 
    if(uFatEntriesLeft > ulMaxClusterNum - ulStartCluster)
        uFatEntriesLeft = (D_UINT16)(ulMaxClusterNum - ulStartCluster);
 
    while(uFatEntriesLeft)
    {
      #if FATMON_DEBUGGING
        FFXPRINTF(2, ("FATMON:uFatEntriesLeft = %u)\n", uFatEntriesLeft));
      #endif
      
        if (uNibblesPerEntry != 8)
        {
            ulNewFatEntry = FatMonGetFatEntry(pcNewSector, uNibble, uNibblesPerEntry);
            ulOldFatEntry = FatMonGetFatEntry(pcOldSector, uNibble, uNibblesPerEntry);
        }
        else
        {
            ulNewFatEntry = FatMonGetFat32Entry(pcNewSector, uNibble, uNibblesPerEntry);
            ulOldFatEntry = FatMonGetFat32Entry(pcOldSector, uNibble, uNibblesPerEntry);
        }
 
        /*  Bump through the FAT entries until we find one that went
            from non-zero to zero.
        */
        if(!((ulOldFatEntry != 0) && (ulNewFatEntry == 0)))
        {
            /*  This entry did NOT meet our criteria, so
                adjust our counts and go on to the next
                entry.
            */
            uNibble += uNibblesPerEntry;
 
            ulClientPage += uSectorsPerCluster;
 
            uFatEntriesLeft--;
 
            /*  Try the next FAT entry
            */
            continue;
        }
 
        /*  We only get to this point if we DID find an entry
            that went from non-zero to zero.
 
            Now see if there are multiple consecutive entries
            that did the same.  If so, we'll discard them with
            as few calls as possible.
        */
 
        uEntriesFound = 1;
        uNibble += uNibblesPerEntry;
 
        while(uFatEntriesLeft > uEntriesFound)
        {
            if (uNibblesPerEntry != 8)
            {
                ulNewFatEntry = FatMonGetFatEntry(pcNewSector, uNibble, uNibblesPerEntry);
                ulOldFatEntry = FatMonGetFatEntry(pcOldSector, uNibble, uNibblesPerEntry);
            }
            else
            {
                ulNewFatEntry = FatMonGetFat32Entry(pcNewSector, uNibble, uNibblesPerEntry);
                ulOldFatEntry = FatMonGetFat32Entry(pcOldSector, uNibble, uNibblesPerEntry);
            }
 
            /*  Did the next entry go from non-zero to zero?
            */
            if((ulOldFatEntry != 0) && (ulNewFatEntry == 0))
            {
                /*  Yes, flag that we found it and try the next entry
                */
 
                uEntriesFound++;
 
                uNibble += uNibblesPerEntry;
            }
            else
            {
                /*  No, so we've counted all the contiguous entries
                    that were cleared.
                */
 
                break;
            }
        }
 
        /*  We know we have at least one cluster to discard...
            maybe several.
        */
        ulSectorCount = uEntriesFound * uSectorsPerCluster;
 
      #if FATMON_DEBUGGING
        FFXPRINTF(3, ("FatMonDiscardClusters (start: %08X, length %08X\n",
                            ulClientPage, ulSectorCount ));
      #endif
 
        FfxDriverSectorDiscard(hDisk, ulClientPage, ulSectorCount);
 
        /*  Point to the next client address, past what
            we just discarded.
        */
        ulClientPage += ulSectorCount;
 
        DclAssert(uEntriesFound <= uFatEntriesLeft);
 
        uFatEntriesLeft -= uEntriesFound;
    }
 
    return;
}
 
 
/*-------------------------------------------------------------------
    Local: FatMonGetFatEntry()
 
    Calculate the FAT entry of uNumNibbles length at the given
    offset into the FAT sector.

    Parameters:
        pcBuffer        - Array of nibbles.
        uNibbleOffset   - The nibble offset into the array.
        uNumNibbles     - Number of nibbles in each entry.
 
    Return Value:
        The entry at the given nibble offset that is
-------------------------------------------------------------------*/
static D_UINT16 FatMonGetFatEntry(
    const D_BUFFER *pcBuffer,
    D_UINT16        uNibbleOffset,
    D_UINT16        uNumNibbles)
{
    D_UINT16        uEntry;
 
    DclAssert(pcBuffer);
    DclAssert(uNumNibbles == 3 || uNumNibbles == 4);
 
    /*  Get the word from the FAT sector.  Do not attempt to read
        a word from an odd offset.  Instead read two bytes.
    */
    uEntry = (D_UINT16) * (pcBuffer + (uNibbleOffset / 2) + 1) << 8;
    uEntry += (D_UINT16) * (pcBuffer + (uNibbleOffset / 2));
 
    /*  4 nibbles is one word, we already have the entry
    */
    if(uNumNibbles == 4)
        return uEntry;
 
    /*  The odd entries are in the high 3 nibbles.
        Even entries are in the low 3 nibbles.
    */
    if(uNibbleOffset & 1)
        return (uEntry >> 4);
    else
        return (uEntry & 0xFFF);
}
 
 
/*----------------------------------------------------------------------
    Local: FatMonGetFat32Entry()
 
    Calculats the FAT32 entry of uNumNibbles length at the given
    offset into the FAT32 sector.
 
    Parameters:
        pcBuffer      - Array of nibbles.
        uNibbleOffset - The nibble offset into the array.
        uNumNibbles   - Number of nibbles in each entry.

    Return Value:
        The entry at the given nibble offset that is
----------------------------------------------------------------------*/
static D_UINT32 FatMonGetFat32Entry(
    const D_BUFFER *pcBuffer,
    D_UINT16        uNibbleOffset,
    D_UINT16        uNumNibbles)
{
    D_UINT32        ulEntry;
    D_UINT32       *pTmp;
 
  #if FATMON_DEBUGGING
    FFXPRINTF(2, ("FatMonGetFat32Entry(%u, %u)\n", uNibbleOffset, uNumNibbles));
  #endif
 
    DclAssert(pcBuffer);
    DclAssert(uNumNibbles == 8);

    /*  Get the dword from the FAT sector.  Do not attempt to read
        a dword from an odd offset.  Instead read four bytes.
    */
    pTmp = (D_UINT32 *)(pcBuffer + (uNibbleOffset /2 ));
    ulEntry =  *pTmp;
 
    return ulEntry;
}
 
 
#endif  /* FFXCONF_FATMONITORSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */


