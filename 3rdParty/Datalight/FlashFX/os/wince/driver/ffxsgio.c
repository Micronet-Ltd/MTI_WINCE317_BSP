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

    This module contains the functions used to support the Scatter/Gather
    reading and writing functionality in the FlashFX device driver for
    Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxsgio.c $
    Revision 1.31  2011/02/09 00:48:39Z  garyp
    Updated to use DCLDIMENSIONOF() rather than a predefined symbol.
    Revision 1.30  2010/11/16 20:55:54Z  johnb
    Cleaned up error checking to correct CETK failures.  Added a 
    specific check for out of memory on buffer marshalling, and 
    changed the buffer marshalling clean up failure to parameter error 
    from general error.
    Revision 1.29  2010/09/23 07:28:25Z  garyp
    Corrected problems with the version checks stemming from rev 1.27.
    Revision 1.28  2010/09/18 01:22:12Z  garyp
    Updated to call LockPages() for all read buffers to prevent deadlocks
    when multiple page faults occur.
    Revision 1.27  2010/09/09 15:38:06Z  johnb
    Added various checks to validate parameters in order to get the CETK to 
    fully pass.  Made some of the tests dependent on CE 6.0 or newer.
    Revision 1.26  2010/06/10 20:27:39Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.25  2010/04/19 19:19:01Z  garyp
    Updated so the QUEUE_PENDING_DISCARDS feature is configurable at
    compile time.  Added profiler instrumentation.
    Revision 1.24  2010/01/23 21:40:46Z  garyp
    Updated to support flush.
    Revision 1.23  2009/11/24 16:45:11Z  garyp
    Corrected to ensure that FFXSTAT_FIM_WRITEPROTECTEDBLOCK errors get
    reported as ERROR_WRITE_PROTECT.
    Revision 1.22  2009/07/22 01:00:53Z  garyp
    Merged from the v4.0 branch.  Updated to use some reorganized FAT 
    functionality.  Conditioned some code on DCL_OSTARGET_WINMOBILE. 
    Updated to allow SGIO buffers to be allocated on the stack.  Updated
    to pass error information back up the stack so that write-protect 
    errors can be accurately returned to CE.  Updated to support the 
    concept of a CE store which is split across two FlashFX Disks.  
    Updated to use standard DCL memory management routines rather than
    using VirtualAlloc/Free().  Conditioned the "hidden sector issue" 
    with CORRECT_HIDDEN_SECTORS, and disabled it by default.  Fixed that
    same code to function correctly when structure packing is disabled.
    Modified to support redirecting sector I/O requests to the Driver
    Framework abstraction for the FML if allocator support is disabled.
    Added support for memory validation.
    Revision 1.21  2009/02/13 00:07:24Z  garyp
    Partial merge from the v4.0 branch.  Resolved bug 2351 -- Dependencies on
    structure packing.  Conditioned the "hidden sector issue" with the setting
    CORRECT_HIDDEN_SECTORS, and disabled it by default. 
    Revision 1.20  2009/02/09 02:15:31Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables
    -- no functional changes.
    Revision 1.19  2008/05/04 23:59:22Z  garyp
    Debug code updated.
    Revision 1.18  2008/05/03 03:33:10Z  garyp
    Added debug code.
    Revision 1.17  2008/03/27 03:22:39Z  Garyp
    Removed DRK remnants.  Updated to work when allocator support is disabled.
    Revision 1.16  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/28 22:51:03Z  jeremys
    Renamed functions.
    Revision 1.14  2007/09/27 02:02:46Z  jeremys
    Renamed some header files and a function name.
    Revision 1.13  2007/04/02 16:18:26Z  keithg
    FSINTTEST has been removed from FFX, it is now part of DCL.
    Revision 1.12  2007/03/19 17:12:26Z  keithg
    Added debug code to dump sectors in the FSINTTEST conditional code.
    Defaulted to off.
    Revision 1.11  2007/03/08 20:11:06Z  keithg
    Corrected pointer advancement in FSINTTEST_ENABLED conditional.
    Revision 1.10  2007/03/04 02:54:50Z  Garyp
    Removed CE 3.0 support.  Added CE 6.0 support.
    Revision 1.9  2007/01/23 23:12:47Z  Garyp
    Added FSIntTest support.
    Revision 1.8  2006/11/30 03:32:40Z  Garyp
    Minor type changes -- nothing functional.
    Revision 1.7  2006/10/04 00:13:59Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.6  2006/03/07 23:39:11Z  rickc
    Updated scatter gather change to new pDisk type.
    Revision 1.5  2006/03/06 19:37:35Z  rickc
    Fixed issue with scatter gather buffers with length less than sector size
    Revision 1.4  2006/02/13 10:38:03Z  Garyp
    Fixed to build cleanly.
    Revision 1.3  2006/02/09 23:05:59Z  Garyp
    Updated to use the new DeviceNum/DiskNum concepts.
    Revision 1.2  2006/02/03 16:26:01Z  johnb
    Added FATMON FAT32 Support
    Revision 1.1  2005/10/02 01:59:28Z  Pauli
    Initial revision
    Revision 1.10  2004/12/30 17:33:25Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.9  2004/12/24 23:29:28Z  GaryP
    Minor debug code changes.
    Revision 1.8  2004/12/07 22:51:28Z  GaryP
    Updated to use the driver framework read/write functions rather than
    calling vbfread/wriite() directly.
    Revision 1.7  2004/11/20 04:36:07Z  GaryP
    Updated to use refactored header files.
    Revision 1.6  2004/09/22 16:58:47Z  GaryP
    Updated to use the FAT_SECTORLEN value.
    Revision 1.5  2004/08/11 00:51:53Z  GaryP
    Fixed a documentation problem.
    Revision 1.4  2004/08/06 23:34:20Z  GaryP
    In-progress checkin.  Commented out some apparently unnecessary functions
    now that FATMON has been refactored.
    Revision 1.3  2004/03/30 00:08:22Z  billr
    Fix typo so it compiles.
    Revision 1.2  2004/01/24 23:29:32Z  garys
    Revision 1.1.1.4  2004/01/24 23:29:32  garyp
    Updated to use the new FFXCONF_... style configuration settings.
    Revision 1.1.1.3  2004/01/03 01:15:14Z  garyp
    Include changes.
    Revision 1.1.1.2  2003/12/25 00:08:38Z  garyp
    Changed all uses of VBF_API_BLOCK_SIZE to pDev->ulSectorLength and
    eliminated any conditional code based on different values for those settings.
    Revision 1.1  2003/12/05 07:16:28Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <windev.h>
#include <devload.h>
#include <diskio.h>
#if _WIN32_WCE >= 600
#include <pkfuncs.h>
#endif

#include <flashfx.h>
#include <fxdriver.h>
#include <fxapireq.h>
#include <ffxwce.h>
#include "ffxdrv.h"

/*  Set this to FALSE to disable putting partial SG buffers on the stack.
    If this is FALSE, buffers will be allocated on the fly as needed.
    Reminder that this cannot just use a static buffer because this code
    is executing in a multithreaded context.

    Defaults to TRUE, for best performance, at the cost of some stack space.
*/
#define SGIO_STACK_BUFFERS          TRUE

#if FFXCONF_FATSUPPORT
    /*  CE is screwed up in that it writes BPBs with a zero hidden
        sectors value even when using an MBR.  In order to maintain
        cross-system compatibility, set CORRECT_HIDDEN_SECTORS to
        TRUE.
    */
    #define CORRECT_HIDDEN_SECTORS  FALSE   /* FALSE for checkin */
#else
    /*  If FAT support is disabled, then the hidden sectors issue
        is not relevant.
    */
    #define CORRECT_HIDDEN_SECTORS  FALSE
#endif

#if CORRECT_HIDDEN_SECTORS
    #include <dlfatapi.h>
    #include <dlpartid.h>
#endif

static DWORD    ScatterGatherRead(FFXDISKHANDLE  hDisk, PSG_REQ pSgr);
static DWORD    ScatterGatherWrite(FFXDISKHANDLE hDisk, PSG_REQ pSgr);
static DWORD    ReadData(FFXDISKHANDLE hDisk, PUCHAR pDestBuf, ULONG ulLength, DWORD dwFlashOffset);
static DWORD    WriteData(FFXDISKHANDLE hDisk, PUCHAR pSrcBuf, ULONG ulLength, DWORD dwFlashOffset, FFXIOSTATUS *pIOStat);


/*-------------------------------------------------------------------
    Private: FfxCeIoctlReadWrite()

    Dispatch an Ioctl read or write operation for a CE Store.

    Parameters:
        hDisk - The FlashFX Disk handle for the CE Store.
        dwIOCTLCode  - IOCTL code/operation to perform (READ or WRITE)
        pSgr         - Pointer into the scatter gather buffer

    Return Value:
        Returns the total number of bytes read or written, and
        pSgr->sr_status will be up to date.
-------------------------------------------------------------------*/
DWORD FfxCeIoctlReadWrite(
    FFXDISKHANDLE   hDisk,
    DWORD           dwIOCTLCode,
    PSG_REQ         pSgr)
{
    DWORD           dwTotalBytes = 0;
    FFXDISKINFO    *pDisk = *hDisk;
  #if _WIN32_WCE >= 600
    DWORD           dwNumSg;
    PSG_BUF         pSg;
    PUCHAR          pSaveOldSGPtrs[MAX_SG_BUF];
    DWORD           dwNumMapped = 0, i;
    HRESULT         hResult;
  #endif

  #if D_DEBUG
    if(dwIOCTLCode == DISK_IOCTL_READ)
    {
        FFXPRINTF(2, ("IOCTL Read  StartSec: %lX NumSec: %lX\n", pSgr->sr_start, pSgr->sr_num_sec));
    }
    else
    {
        DclAssert(dwIOCTLCode == DISK_IOCTL_WRITE);
        FFXPRINTF(2, ("IOCTL Write StartSec: %lX NumSec: %lX\n", pSgr->sr_start, pSgr->sr_num_sec));
    }

    FFXPRINTF(2, ("                   Status: %lX NumSG:  %lX\n", pSgr->sr_status, pSgr->sr_num_sg));
  #endif

    DclProfilerEnter("FfxCeIoctlReadWrite", 0, 0);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
    FfxCeFlushPendingDiscards(hDisk);
  #endif

    /*  Still not complete yet
    */
    pSgr->sr_status = ERROR_IO_PENDING;

    /*  Ensure the number of scatter gathers are reasonable
    */
    if(pSgr->sr_num_sg > MAX_SG_BUF)
    {
        FFXPRINTF(1, ("ERROR_INVALID_PARAMETER\n"));

        DclError();

        pSgr->sr_status = ERROR_INVALID_PARAMETER;

        goto ReadWriteCleanup;
    }

    /*  Make sure request doesn't exceed the disk
    */
    if(pSgr->sr_start + pSgr->sr_num_sec > pDisk->pDiskHook->CeDiskInfo.di_total_sectors)
    {
        FFXPRINTF(1, ("ERROR_SECTOR_NOT_FOUND\n"));

        pSgr->sr_status = ERROR_SECTOR_NOT_FOUND;

        DclError();

        goto ReadWriteCleanup;
    }

    /*  Fail if it's not already opened!
    */
    if(!(pDisk->pDiskHook->dwFlags & STATE_OPENED))
    {
        FFXPRINTF(1, ("DEVICE_NOT_OPENED\n"));

        DclError();

        pSgr->sr_status = ERROR_SECTOR_NOT_FOUND;

        goto ReadWriteCleanup;
    }

  #if _WIN32_WCE >= 600
    dwNumSg = pSgr->sr_num_sg;
    pSg = &(pSgr->sr_sglist[0]);
    for(dwNumMapped = 0; dwNumMapped < dwNumSg; dwNumMapped++)
    {
        pSaveOldSGPtrs[dwNumMapped] = pSg[dwNumMapped].sb_buf;
        hResult = CeOpenCallerBuffer(
                     &pSg[dwNumMapped].sb_buf,
                     pSaveOldSGPtrs[dwNumMapped],
                     pSg[dwNumMapped].sb_len,
                     ((dwIOCTLCode == DISK_IOCTL_READ)?ARG_O_PTR:ARG_I_PTR),
                     FALSE);
        
        if (hResult == E_OUTOFMEMORY)
        {
            pSgr->sr_status = ERROR_OUTOFMEMORY;
            goto FFXCEIORW_EXIT;
        }
        else if (FAILED(hResult))
        {
            pSgr->sr_status = ERROR_INVALID_PARAMETER;
            goto FFXCEIORW_EXIT;
        }
    }
  #endif

    if(dwIOCTLCode == DISK_IOCTL_READ)
        dwTotalBytes = ScatterGatherRead(hDisk, pSgr);
    else
        dwTotalBytes = ScatterGatherWrite(hDisk, pSgr);

  #if _WIN32_WCE >= 600
  FFXCEIORW_EXIT:

    for (i = 0; i < dwNumMapped; i++)
    {
        hResult = CeCloseCallerBuffer(
                     pSg[i].sb_buf,
                     pSaveOldSGPtrs[i],
                     pSg[i].sb_len,
                     ((dwIOCTLCode== DISK_IOCTL_READ)?ARG_O_PTR:ARG_I_PTR));
        if(FAILED(hResult))
        {
            pSgr->sr_status = ERROR_INVALID_PARAMETER;
            break;
        }
    }
  #endif

    /*  If the access is still pending, it worked!
    */
    if(pSgr->sr_status == ERROR_IO_PENDING)
        pSgr->sr_status = ERROR_SUCCESS;

  ReadWriteCleanup:
    DclProfilerLeave(dwTotalBytes);
    FFXPRINTF(2, ("IOCTL Read/Write returning %lX bytes, Error code: %lX\n",
               dwTotalBytes, pSgr->sr_status));

    return dwTotalBytes;
}


/*-------------------------------------------------------------------
    Private: FfxCeIoctlFlush()

    Issue a flush for the CE Store.

    Parameters:
        hDisk - The FlashFX Disk handle for the CE Store.

    Return Value:
        Returns an FFXSTATUS code indicating the result.
-------------------------------------------------------------------*/
FFXSTATUS FfxCeIoctlFlush(
    FFXDISKHANDLE   hDisk)
{
    D_UINT32        ulCount;
    FFXDISKINFO    *pDisk = *hDisk;

    DclProfilerEnter("FfxCeIoctlFlush", 0, 0);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
    FfxCeFlushPendingDiscards(hDisk);
  #endif

    ulCount = FfxDriverSectorFlush(hDisk, 0, D_UINT32_MAX);

    /*  If the CE Store spans multiple FlashFX Disks, then issue the flush
        for both Disks.
    */        
    if(pDisk->pDiskHook->hNextDisk)
        ulCount += FfxDriverSectorFlush(pDisk->pDiskHook->hNextDisk, 0, D_UINT32_MAX);

    DclProfilerLeave(ulCount);
    FFXPRINTF(2, ("FfxCeIoctlFlush() flushed %lU sectors\n", ulCount));

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Private: FfxCeMapPtr()

    This function maps a pointer to a process.  It is only used
    in CE versions prior to v6.0.

    The pointer should be unmapped with FfxCeUnMapPtr() when it
    is no longer needed.

    Parameters:
        ptr    - The pointer to map

    Return Value:
        Returns the mapped pointer.
-------------------------------------------------------------------*/
void * FfxCeMapPtr(
    void       *ptr)
{
    DclAssert(ptr);

  #if _WIN32_WCE < 600
    return MapPtrToProcess((LPVOID)ptr, GetCallerProcess());
  #else
    return ptr;
  #endif
}


/*-------------------------------------------------------------------
    Private: FfxCeUnMapPtr()

    This function unmaps a pointer that was mapped with
    FfxCeMapPtr().  It is only used in CE versions prior
    to v6.0.

    Parameters:
        ptr    - The pointer to unmap

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxCeUnMapPtr(
    void       *ptr)
{
    DclAssert(ptr);

  #if _WIN32_WCE < 600
    UnMapPtr(ptr);
  #else
    (void)ptr;
  #endif

    return;
}


/*-------------------------------------------------------------------
    Local: ScatterGatherRead()

    The scatter/gather functionality allows a single contiguous block
    of flash to be read in distributed into one or more buffers.
    These buffers do not have to be sized in multiples of 512, however
    the overall size of the block must still be sized as such.

    Parameters:
        hDisk - The Disk handle
        pSgr  - A pointer to the PSG_REQ structure to use

    Return Value:
        Returns the total number of bytes read.
-------------------------------------------------------------------*/
static DWORD ScatterGatherRead(
    FFXDISKHANDLE   hDisk,
    PSG_REQ         pSgr)
{
    FFXDISKHANDLE   hDiskIO;
    FFXDISKINFO    *pDisk = *hDisk;
    FFXDISKINFO    *pDiskIO;
    DWORD           dwNumSG;
    DWORD           dwFlashOffset;
    DWORD           dwTotalBytesRead = 0;
    DWORD           dwStartSector;
    PSG_BUF         pSg;
  #if SGIO_STACK_BUFFERS
    DCLALIGNEDBUFFER(sgr,data,FFX_MAX_PAGESIZE);
    PBYTE           pTailBuff = sgr.data;
  #else
    PBYTE           pTailBuff = NULL;       /* Non-zero if must be freed on exit */
  #endif
    ULONG           ulTailBuffLen = 0;      /* Remaining available length in the buffer */
    ULONG           ulTailBuffIndex = 0;    /* Non-zero indicates something in the buffer */

#if 0
#if D_DEBUG == 0

    /*  This code exists solely so that the benchmark test in the
        WinCE Test Kit (WCETK) will pass.  Completely bogus...
        Microsoft has acknowleged that the benchmark test in the
        CE 4.0 test kit will fail if the test finishes "too fast".

        Adding a simple do nothing RETAILMSG solves the problem,
        though theoretically faster CPUs might require even more
        delay than this.  If we're not running CE.net, or if we're
        running in DEBUG mode, there is no need for the delay.

        Just comment out the outer "#if 0" to enable this code.

        Microsoft has fixed this bug in the 4.2 CETK.
    */
    DclPrintf("");
#endif
#endif

    DclProfilerEnter("FfxCe:ScatterGatherRead", 0, 0);
    DclAssert(pSgr);
    DclAssert(pDisk);

    dwStartSector = pSgr->sr_start;

    /*  We might have one CE Store which is split across two FlashFX
        Disks.  Adjust the starting sector and the Disk handle if
        necessary (an individual I/O call can't cross the boundary).
    */
    hDiskIO = FfxCeAdjustIODiskAndOffset(hDisk, &dwStartSector);

    pDiskIO = *hDiskIO;

    /*  Convert sectors to bytes
    */
    dwFlashOffset = dwStartSector * pDisk->pDiskHook->CeDiskInfo.di_bytes_per_sect;

    dwNumSG = pSgr->sr_num_sg;

    pSg = &(pSgr->sr_sglist[0]);

    DclAssert(dwNumSG);
    DclAssert(pSgr->sr_status == ERROR_IO_PENDING);

    while(dwNumSG)
    {
        DWORD           dwThisSGLen;
        ULONG           ulRemaining;
        PUCHAR          pBuf;
        PUCHAR          pOriginalMappedBuf;
        DWORD           dwBytesRead;

        /* check the parameters
        */
        if (pSg->sb_len == 0 || pSg->sb_buf == NULL)
        {
            pSgr->sr_status = ERROR_INVALID_PARAMETER;
            break;
        }

      #if DCLCONF_MEMORYVALIDATION
        DclMemValBufferAdd(NULL, pSg->sb_buf, pSg->sb_len, DCL_MVFLAG_TYPEEXPLICIT);
      #endif

        pOriginalMappedBuf = FfxCeMapPtr(pSg->sb_buf);
        pBuf = pOriginalMappedBuf;

        dwThisSGLen = pSg->sb_len;

        FFXPRINTF(2, ("SG read: pBuf=%P Len=%lX\n", pBuf, dwThisSGLen));

        DclAssert(dwThisSGLen);

        /*  If there was something left from a previous read, fill as much
            of this SG buffer as we can with it, and adjust the pointers
            and remaining lengths.
        */
        if(ulTailBuffIndex && ulTailBuffLen)
        {
            ULONG           ulLen;

            DclAssert(pTailBuff);
            DclAssert(ulTailBuffIndex + ulTailBuffLen == pDiskIO->ulSectorLength);

            if(dwThisSGLen >= ulTailBuffLen)
                ulLen = ulTailBuffLen;
            else
                ulLen = dwThisSGLen;

            DclMemCpy(pBuf, pTailBuff + ulTailBuffIndex, ulLen);

            dwTotalBytesRead    += ulLen;
            pBuf                += ulLen;   /* adjust client buffer pointer */
            ulTailBuffIndex     += ulLen;
            ulTailBuffLen       -= ulLen;
            dwThisSGLen         -= ulLen;

            /*  If we completely filled this SG buffer, skip to the next...
            */
            if(!dwThisSGLen)
                goto NextScatterGatherRead;
        }

        /*----------------------------------------------------------------
            We handled any leftover data from a previous iteration, so now
            we are ready to handle full sectors that can fit within this
            scatter/gather buffer.
        ----------------------------------------------------------------*/
        if(dwThisSGLen >= pDiskIO->ulSectorLength)
        {
            if(dwThisSGLen % pDiskIO->ulSectorLength != 0)
            {
                /*  The CE.net Test Kit will try to do this during the "API
                    Compatibility" tests.
                */

                /*  Calc remaining tail bytes in this SG, and shorten this length
                */
                ulRemaining = dwThisSGLen & (pDiskIO->ulSectorLength - 1);
                dwThisSGLen -= ulRemaining;
            }
            else
            {
                /*  This block was sized OK, so no tail to deal with.
                */
                ulRemaining = 0;
            }

            /*  We've now trimmed down this SG len to be evenly divisible by
                a sector size.  Anything that might remain is indicated by
                "ulRemaining" be non-zero.
            */
            DclAssert(dwFlashOffset % pDiskIO->ulSectorLength == 0);
            DclAssert(dwThisSGLen % pDiskIO->ulSectorLength == 0);

            dwBytesRead = ReadData(hDiskIO, pBuf, dwThisSGLen, dwFlashOffset);

            dwTotalBytesRead += dwBytesRead;
            dwFlashOffset    += dwBytesRead;
            pBuf             += dwBytesRead;

            if(dwBytesRead != dwThisSGLen)
            {
                FFXPRINTF(1, ("ReadData(a) failed to read %lU sector(s) at sector offset %lX (Read %lU)\n",
                    dwThisSGLen / pDiskIO->ulSectorLength,
                    dwFlashOffset / pDiskIO->ulSectorLength,
                    dwBytesRead / pDiskIO->ulSectorLength));
                    

                DclError();

                pSgr->sr_status = ERROR_READ_FAULT;

                goto NextScatterGatherRead;
            }
        }
        else
        {
            ulRemaining = dwThisSGLen;
        }

        /*----------------------------------------------------------------
            Now if there is still some area left to fill in the SG buffer,
            read a full sector into our allocated "TailBuff" area and fill
            out the SG from it.  Whatever remains will be used to start
            filling in the next SG buffer.
        ----------------------------------------------------------------*/
        if(ulRemaining)
        {
            /*  Darn well better be nothing remaining from any previous
                tail buffer.
            */
            DclAssert(!ulTailBuffLen);
            DclAssert(ulRemaining < pDiskIO->ulSectorLength);

            FFXPRINTF(2, ("ScatterGatherRead() Partial sector remaining=%lU\n", ulRemaining));

          #if !SGIO_STACK_BUFFERS
            /*  Allocate the tail buffer if we've not already gotten one
            */
            if(!pTailBuff)
            {
                pTailBuff = DclMemAlloc(pDiskIO->ulSectorLength);

                DclAssert(pTailBuff);
            }
          #endif

            dwBytesRead = ReadData(hDiskIO, pTailBuff, pDiskIO->ulSectorLength, dwFlashOffset);
            if(dwBytesRead != pDiskIO->ulSectorLength)
            {
                FFXPRINTF(1, ("ReadData(b) failed to read 1 sector at sector offset %lX\n",
                    dwFlashOffset / pDiskIO->ulSectorLength));

                DclError();

                pSgr->sr_status = ERROR_READ_FAULT;

                goto NextScatterGatherRead;
            }

            /*  Fill the remainder of this SG buffer
            */
            DclMemCpy(pBuf, pTailBuff, ulRemaining);

            dwFlashOffset    += pDiskIO->ulSectorLength;
            dwTotalBytesRead += ulRemaining;
            pBuf             += ulRemaining;

            /*  Calc length and index of data that is remaining in the
                tail buffer, which will be used to start filling the
                next SG buffer.
            */
            ulTailBuffIndex = ulRemaining;
            ulTailBuffLen = pDiskIO->ulSectorLength - ulRemaining;
        }

      NextScatterGatherRead:

        FfxCeUnMapPtr(pOriginalMappedBuf);

      #if DCLCONF_MEMORYVALIDATION
        DclMemValBufferRemove(NULL, pSg->sb_buf);
      #endif

        /*  Don't continue if there was an error!
        */
        if(pSgr->sr_status != ERROR_IO_PENDING)
            break;

        dwNumSG--;

        /*  Use the next scatter/gather buffer
        */
        pSg++;
    }

    if(pSgr->sr_status == ERROR_IO_PENDING)
    {
        /*  When all is said and done, there should be no space left in
            our tail buffer.
        */
        DclAssert(!ulTailBuffLen);

      #if CORRECT_HIDDEN_SECTORS
        {
            unsigned    kk;
            unsigned    nParts = DCLDIMENSIONOF(pDiskIO->fpdi.fpi);

            /*  If we just read the BPB, check to see if it is a type we
                recognize, and lie to FATFS regarding the hidden sectors value.
            */

            /*  See if we are reading a boot record
            */
            for(kk = 0; kk < nParts; kk++)
            {
                /*  Is the boot record for this partition being read
                */
                if((pDiskIO->fpdi.fpi[kk].bFSID != FSID_NONE) &&
                   (pDiskIO->fpdi.fpi[kk].ulStartSector == dwStartSector))
                {
                    break;
                }
            }

            if(kk < nParts)
            {
                D_BUFFER   *pBuff;
                D_BYTE      bFSID;
                DCLFATBPB   bpb;

                /*  We don't allow any weird stuff here.  Any BPB reads must not
                    be scattered using multiple SG buffers, and the read must be
                    at least FAT_SECTORLEN bytes long.
                */
                DclAssert(pSgr->sr_num_sg == 1);

                pSg = &(pSgr->sr_sglist[0]);

                DclAssert(pSg->sb_len >= FAT_SECTORLEN);

                FFXPRINTF(1, ("Detected Boot Record read, checking hidden sectors\n"));

                pBuff = FfxCeMapPtr(pSg->sb_buf);
                DclAssert(pBuff);

                bFSID = DclFatBPBMove(&bpb, pBuff);

                if(bFSID == FSID_FAT12 || bFSID == FSID_FAT16 || bFSID == FSID_FAT32)
                {
                    if(bpb.ulHidden != 0)
                    {
                        /*  If the hidden sectors value is correct, it will exactly
                            match the pDiskIO->fpdi.fpi[kk].ulStartSector value --
                            even when we are not using an MBR.

                            Therefore, do a little reality check here...
                        */
                        DclAssert(bpb.ulHidden == (*hDiskIO)->fpdi.fpi[kk].ulStartSector);

                        DclMemSet(pBuff[BT_HIDN_OFF], 0, sizeof(bpb.ulHidden));

                        FFXPRINTF(1, ("Lying to FATFS and telling it hidden sectors is 0\n"));
                    }
                }

                FfxCeUnMapPtr(pBuff);
            }
        }
      #endif
    }

  #if !SGIO_STACK_BUFFERS
    /*  Free any memory we allocated
    */
    if(pTailBuff)
        DclMemFree(pTailBuff);
  #endif

    DclProfilerLeave(dwTotalBytesRead);
    return dwTotalBytesRead;
}


/*-------------------------------------------------------------------
    Local: ScatterGatherWrite()

    The scatter/gather functionality allows a single contiguous block
    of flash to be written from one or more source buffers.  These
    buffers do not have to be sized in multiples of 512, however the
    overall size of the block must still be sized as such.

    Parameters:
        hDisk - The Disk handle
        pSgr  - A pointer to the PSG_REQ structure to use

    Return Value:
        Returns the total number of bytes written.
-------------------------------------------------------------------*/
static DWORD ScatterGatherWrite(
    FFXDISKHANDLE   hDisk,
    PSG_REQ         pSgr)
{
    FFXDISKHANDLE   hDiskIO;
    FFXDISKINFO    *pDisk = *hDisk;
    FFXDISKINFO    *pDiskIO;
    DWORD           dwNumSG;
    DWORD           dwFlashOffset;
    DWORD           dwThisSGLen;
    DWORD           dwTotalBytesWritten = 0;
    DWORD           dwStartSector;
    PSG_BUF         pSg;
  #if SGIO_STACK_BUFFERS
    DCLALIGNEDBUFFER(sgw,data,FFX_MAX_PAGESIZE);
    PBYTE           pTailBuff = sgw.data;
  #else
    PBYTE           pTailBuff = NULL;   /* Non-zero if must be freed on exit */
  #endif
    ULONG           ulTailBuffLen = 0;  /* Remaining available length in the buffer   */
    ULONG           ulTailBuffIndex = 0;/* Non-zero indicates something in the buffer */

    FFXIOSTATUS     ioStat;

    DclProfilerEnter("FfxCe:ScatterGatherWrite", 0, 0);
    DclAssert(pSgr);
    DclAssert(pDisk);

    dwStartSector = pSgr->sr_start;

    /*  We might have one CE Store which is split across two FlashFX
        Disks.  Adjust the starting sector and the Disk handle if
        necessary (an individual I/O call can't cross the boundary).
    */
    hDiskIO = FfxCeAdjustIODiskAndOffset(hDisk, &dwStartSector);

    pDiskIO = *hDiskIO;

    /*  Convert sectors to bytes
    */
    dwFlashOffset = dwStartSector * pDisk->pDiskHook->CeDiskInfo.di_bytes_per_sect;

    dwNumSG = pSgr->sr_num_sg;

    pSg = &(pSgr->sr_sglist[0]);

    /* check the parameters
    */
    if (pSg->sb_len == 0 || pSg->sb_buf == NULL)
    {
        pSgr->sr_status = ERROR_INVALID_PARAMETER;
        goto ScatterGatherWriteCleanup;
    }

    DclAssert(dwNumSG);
    DclAssert(pSgr->sr_status == ERROR_IO_PENDING);

  #if CORRECT_HIDDEN_SECTORS
    {
        unsigned    kk;
        unsigned    nParts = DCLDIMENSIONOF(pDiskIO->fpdi.fpi);

        /*  See if we are rewriting a boot record
        */
        for(kk = 0; kk < nParts; kk++)
        {
            /*  Is the boot record for this partition being written to?
            */
            if((pDiskIO->fpdi.fpi[kk].bFSID != FSID_NONE) &&
               (pDiskIO->fpdi.fpi[kk].ulStartSector == dwStartSector))
            {
                break;
            }
        }

        if(kk < nParts)
        {
            D_BUFFER   *pBuff;
            D_BYTE      bFSID;
            DCLFATBPB   bpb;

            FFXPRINTF(2, ("Detected Boot Record write, checking hidden sectors\n"));

            /*  We don't allow any weird stuff here.  Any BPB writes must not
                be scattered using multiple SG buffers, and the write must be
                at least FAT_SECTORLEN bytes long.
            */
            DclAssert(pSgr->sr_num_sg == 1);
            DclAssert(pSg->sb_len >= FAT_SECTORLEN);

          #if DCLCONF_MEMORYVALIDATION
            DclMemValBufferAdd(NULL, pSg->sb_buf, pSg->sb_len, DCL_MVFLAG_TYPEEXPLICIT);
          #endif

            pBuff = FfxCeMapPtr(pSg->sb_buf);
            DclAssert(pBuff);

            /*  CE.net is screwed up in that it writes BPBs with a zero hidden
                sectors value even when we are using an MBR.  In order to
                maintain cross-system compatibility, we hack the hidden sectors
                value to be correct.
            */

            bFSID = DclFatBPBMove(&bpb, pBuff);

            /*  Only do this if it really is BPB data that is being written...
            */
            if(bFSID == FSID_FAT12 || bFSID == FSID_FAT16 || bFSID == FSID_FAT32)
            {
                if(bpb.ulHidden != pDiskIO->fpdi.fpi[kk].ulStartSector)
                {
                    /*  If the hidden sectors value is correct, it will exactly
                        match the pDiskIO->fpdi.fpi[kk].ulStartSector value --
                        even when we are not using an MBR.

                        Therefore, do a little reality check here...
                    */
                    DclAssert(bpb.ulHidden == 0);

                    /*  Hack it to be correct.
                    */
                    DCLNE2LE(pBuff[BT_HIDN_OFF], &pDiskIO->fpdi.fpi[kk].ulStartSector, sizeof(bpb.ulHidden));

                    FFXPRINTF(1, ("Hacked BPB to use correct hidden sectors value\n"));
                }
            }

            FfxCeUnMapPtr(pBuff);

          #if DCLCONF_MEMORYVALIDATION
            DclMemValBufferRemove(NULL, pSg->sb_buf);
          #endif
        }
    }
  #endif

    while(dwNumSG)
    {
        PUCHAR          pBuf;
        PUCHAR          pOriginalMappedBuf;
        ULONG           ulBytesWritten;
        ULONG           ulRemaining;

      #if DCLCONF_MEMORYVALIDATION
        DclMemValBufferAdd(NULL, pSg->sb_buf, pSg->sb_len, DCL_MVFLAG_TYPEEXPLICIT|DCL_MVFLAG_READONLY);
      #endif

        pOriginalMappedBuf = FfxCeMapPtr(pSg->sb_buf);
        pBuf = pOriginalMappedBuf;

        dwThisSGLen = pSg->sb_len;

        FFXPRINTF(2, ("SG write: pBuf=%P Len=%lX\n", pBuf, dwThisSGLen));

        DclAssert(dwThisSGLen);

        /*  If there is already something in the tail buffer, then we need
            to continue filling it, and write it out when it is full.
        */
        if(ulTailBuffIndex)
        {
            ULONG           ulLen;

            DclAssert(pTailBuff);
            DclAssert(ulTailBuffIndex + ulTailBuffLen == pDiskIO->ulSectorLength);

            if(dwThisSGLen >= ulTailBuffLen)
                ulLen = ulTailBuffLen;
            else
                ulLen = dwThisSGLen;

            DclMemCpy(pTailBuff + ulTailBuffIndex, pBuf, ulLen);

            pBuf += ulLen;      /* adjust client buffer pointer */
            ulTailBuffIndex += ulLen;
            ulTailBuffLen -= ulLen;
            dwThisSGLen -= ulLen;

            /*  If there is no length left in the tail buffer, write the
                sector to disk.
            */
            if(!ulTailBuffLen)
            {
                DclAssert(ulTailBuffIndex == pDiskIO->ulSectorLength);

                ulBytesWritten = WriteData(hDiskIO, pTailBuff, pDiskIO->ulSectorLength, dwFlashOffset, &ioStat);

                if(ulBytesWritten != pDiskIO->ulSectorLength)
                {
                    FFXPRINTF(1, ("write FAILED! (C)\n"));

                    /*  It is critical to return the proper error code here, or 
                        things like the WinMobile LTK will spit out loads of
                        warnings.
                    */
                    if(ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDPAGE ||
                       ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDBLOCK ||
                       ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDSOFTLOCK)
                        pSgr->sr_status = ERROR_WRITE_PROTECT;
                    else
                        pSgr->sr_status = ERROR_WRITE_FAULT;

                    goto NextScatterGatherWrite;
                }

                dwFlashOffset += pDiskIO->ulSectorLength;
                dwTotalBytesWritten += pDiskIO->ulSectorLength;

                /*  Reset to indicate an empty tail buffer
                */
                ulTailBuffIndex = 0;
            }

            /*  If we completely emptied this SG buffer, skip to the next...
            */
            if(!dwThisSGLen)
                goto NextScatterGatherWrite;
        }

        /*----------------------------------------------------------------
            We handled any leftover data from a previous iteration, so now
            we are ready to handle full sectors that can fit within this
            scatter/gather buffer.
        ----------------------------------------------------------------*/
        if(dwThisSGLen >= pDiskIO->ulSectorLength)
        {
            if(dwThisSGLen % pDiskIO->ulSectorLength != 0)
            {
                /*  calc remaining tail bytes in this SG
                */
                ulRemaining = dwThisSGLen & (pDiskIO->ulSectorLength - 1);
                dwThisSGLen -= ulRemaining;
            }
            else
            {
                /*  This block was sized OK, so no tail to deal with.
                */
                ulRemaining = 0;
            }

            /*  We've now trimmed down this SG len to be evenly divisible by
                a sector size.  Anything that might remain is indicated by
                "ulRemaining" be non-zero.
            */
            DclAssert(dwThisSGLen % pDiskIO->ulSectorLength == 0);

            ulBytesWritten = WriteData(hDiskIO, pBuf, dwThisSGLen, dwFlashOffset, &ioStat);

            dwFlashOffset += ulBytesWritten;
            pBuf += ulBytesWritten;
            dwTotalBytesWritten += ulBytesWritten;

            if(ulBytesWritten != dwThisSGLen)
            {
                FFXPRINTF(1, ("write FAILED! (A)\n"));

                /*  It is critical to return the proper error code here, or 
                    things like the WinMobile LTK will spit out loads of
                    warnings.
                */
                if(ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDPAGE ||
                   ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDBLOCK ||
                   ioStat.ffxStat == FFXSTAT_FIM_WRITEPROTECTEDSOFTLOCK)
                    pSgr->sr_status = ERROR_WRITE_PROTECT;
                else
                    pSgr->sr_status = ERROR_WRITE_FAULT;

                goto NextScatterGatherWrite;
            }
        }
        else
        {
            ulRemaining = dwThisSGLen;
        }

        /*----------------------------------------------------------------
            Now if there is still some data left to get from the SG buffer,
            save it off in our tail buffer so that we can process it as part
            of the next SG buffer.
        ----------------------------------------------------------------*/
        if(ulRemaining)
        {
            /*  Darn well better be nothing remaining from any previous
                tail buffer.
            */
            DclAssert(!ulTailBuffIndex);
            DclAssert(ulRemaining < pDiskIO->ulSectorLength);

            FFXPRINTF(2, ("ScatterGatherWrite() Partial sector remaining=%lU\n", ulRemaining));

          #if !SGIO_STACK_BUFFERS
            /*  Allocate the tail buffer if we've not already gotten one
            */
            if(!pTailBuff)
            {
                pTailBuff = DclMemAlloc(pDiskIO->ulSectorLength);

                DclAssert(pTailBuff);
            }
          #endif

            /*  Empty the remainder of this SG buffer
            */
            DclMemCpy(pTailBuff, pBuf, ulRemaining);

            pBuf += ulRemaining;

            /*  Calc length and index of area that is remaining in the
                tail buffer, which will be used to start emptying the
                next SG buffer.
            */
            ulTailBuffIndex = ulRemaining;
            ulTailBuffLen = pDiskIO->ulSectorLength - ulRemaining;
        }

      NextScatterGatherWrite:

        FfxCeUnMapPtr(pOriginalMappedBuf);

      #if DCLCONF_MEMORYVALIDATION
        DclMemValBufferRemove(NULL, pSg->sb_buf);
      #endif

        /*  Don't continue if there was an error!
        */
        if(pSgr->sr_status != ERROR_IO_PENDING)
            break;

        dwNumSG--;

        /*  Use the next scatter/gather buffer
        */
        pSg++;
    }

    /*  When all is said and done, there should be no space left in
        our tail buffer if there was no error.
    */
    DclAssert(ulTailBuffLen == 0 || pSgr->sr_status != ERROR_IO_PENDING);

ScatterGatherWriteCleanup:

  #if !SGIO_STACK_BUFFERS
    /*  Free any memory we allocated
    */
    if(pTailBuff)
        DclMemFree(pTailBuff);
  #endif

    DclProfilerLeave(dwTotalBytesWritten);
    return dwTotalBytesWritten;
}


/*-------------------------------------------------------------------
    Private: FfxCeAdjustIODiskAndOffset()

    We might have one CE Store which is split across two FlashFX
    Disks.  Adjust the starting sector and the Disk handle if
    necessary (an individual I/O call can't cross the boundary).

    Additionally, we may need to adjust for CE "Compaction Blocks"
    which may be embedded in the middle of what would otherwise
    be a 1 to 1 linear sector mapping.

    Parameters:
        hDisk     - The Disk handle
        pulSector - Specifies a pointer to the starting sector
                    value, which may be adjusted on exit.

    Return Value:
        Returns the actual Disk handle to use.
-------------------------------------------------------------------*/
FFXDISKHANDLE FfxCeAdjustIODiskAndOffset(
    FFXDISKHANDLE   hDisk,
    D_UINT32       *pulSector)
{
    FFXDISKINFO    *pDisk = *hDisk;

    DclAssert(pDisk);
    DclAssert(pulSector);

    if(pDisk->pDiskHook->ulNextDiskSector && *pulSector >= pDisk->pDiskHook->ulNextDiskSector)
    {
        /*  There is a subsequent Disk, and we have determined that the
            starting logical sector number is past the threshhold for the
            start of that Disk.  Change the Disk handle and adjust the
            starting sector number.  In this scenario we do not need to
            concern ourselves with any compaction blocks, as that was
            already taken into account when the threshhold was calculated.
        */

        DclAssert(pDisk->pDiskHook->hNextDisk);

        *pulSector -= pDisk->pDiskHook->ulNextDiskSector;
        hDisk = pDisk->pDiskHook->hNextDisk;
    }
    else
    {
    }

    return hDisk;
}


/*-------------------------------------------------------------------
    Local: ReadData()

    Read data from flash.

    Parameters:
        hDisk         - The Disk Handle
        pDestBuff     - The client buffer to fill
        ulLength      - The number of bytes to read
        dwFlashOffset - The byte offset to read from

    Return Value:
        Returns the number of bytes read.
-------------------------------------------------------------------*/
static DWORD ReadData(
    FFXDISKHANDLE   hDisk,
    PUCHAR          pDestBuf,
    ULONG           ulLength,
    DWORD           dwFlashOffset)
{
    FFXDISKINFO    *pDisk = *hDisk;
    ULONG           ulSectorsRead;
    D_BOOL          fLocked = TRUE;

    DclProfilerEnter("FfxCe:ReadData", 0, 0);
    
    DclAssert(pDisk);
    DclAssert(pDestBuf);
    DclAssert(ulLength);
    DclAssert(ulLength % pDisk->ulSectorLength == 0);
    DclAssert(dwFlashOffset % pDisk->ulSectorLength == 0);

    /*  We must avoid accessing uncommitted pages while in FlashFX, since
        FlashFX may be used to demand page stuff into memory, and we could
        get into a deadlock situation.  This does not seem to be a problem
        when using FAT, because it appears that FAT always does I/O to its
        internal buffers.  However Reliance may do I/O directly to client
        buffers, which could be uncommitted.  This has been observed on 
        WinMobile 6.5 when running battlife.wma from the LTK.  One would 
        think that this would be automatically handled internally by CE...

        The problem seemingly can be resolved by using VirtualAlloc() with
        the MEM_COMMIT option, however the Lock/UnlockPages() APIs seem 
        like they might be better since the original state can be restored.
        Note also, that from reading the docs, it is not apparent that
        LockPages() will force reserved pages to be committed, but that 
        seems to be the case.
        
        This should not be an issue with writes, since one would not try
        writing from an uncommitted buffer (right?)...
    */        
    if(!LockPages(pDestBuf, ulLength, NULL, LOCKFLAG_WRITE))
    {
        DCLPRINTF(1, ("FfxCe:ReadData() LockPages() failed with error %lX\n", GetLastError()));
        DclError();

        fLocked = FALSE;
    }

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if((*hDisk)->hVBF)
    {
        ulSectorsRead = FfxDriverSectorRead(hDisk,
                                            dwFlashOffset / pDisk->ulSectorLength,
                                            ulLength / pDisk->ulSectorLength,
                                            pDestBuf);
    }
    else
  #endif
    {
        FFXIOSTATUS ioStat;

        ioStat = FfxDriverFmlSectorRead(hDisk,
                                        dwFlashOffset / pDisk->ulSectorLength,
                                        ulLength / pDisk->ulSectorLength,
                                        pDestBuf);

        ulSectorsRead = ioStat.ulCount;
    }

    if(fLocked)
        UnlockPages(pDestBuf, ulLength); 

    DclProfilerLeave(ulSectorsRead);
    
    return ulSectorsRead * pDisk->ulSectorLength;
}


/*-------------------------------------------------------------------
    Local: WriteData()

    Write data to flash.

    Parameters:
        hDisk         - The Disk Handle
        pSrcBuf       - The client buffer to write from
        ulLength      - The number of bytes to write
        dwFlashOffset - The byte offset to read from

    Return Value:
        Returns the number of bytes written.
-------------------------------------------------------------------*/
static DWORD WriteData(
    FFXDISKHANDLE   hDisk,
    PUCHAR          pSrcBuf,
    ULONG           ulLength,
    DWORD           dwFlashOffset,
    FFXIOSTATUS    *pIOStat)
{
    FFXDISKINFO    *pDisk = *hDisk;
    ULONG           ulSectorsWritten = 0;
    
    DclProfilerEnter("FfxCe:WriteData", 0, 0);

    DclAssert(pDisk);
    DclAssert(pSrcBuf);
    DclAssert(ulLength);
    DclAssert(ulLength % pDisk->ulSectorLength == 0);
    DclAssert(dwFlashOffset % pDisk->ulSectorLength == 0);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if((*hDisk)->hVBF)
    {
        ulSectorsWritten = FfxDriverSectorWrite(hDisk,
                                                dwFlashOffset / pDisk->ulSectorLength,
                                                ulLength / pDisk->ulSectorLength,
                                                pSrcBuf);
    }
    else
  #endif
    {
        D_BUFFER *pBlockBuff = DclMemAlloc(pDisk->ulBlockSize);
        if(pBlockBuff)
        {
            *pIOStat = FfxDriverFmlSectorWrite(hDisk,
                                               dwFlashOffset / pDisk->ulSectorLength,
                                               ulLength / pDisk->ulSectorLength,
                                               pSrcBuf, pBlockBuff);

            ulSectorsWritten = pIOStat->ulCount;
            DclMemFree(pBlockBuff);
        }
        else
        {
            DclError();
        }
    }

    DclProfilerLeave(ulSectorsWritten);

    return ulSectorsWritten * pDisk->ulSectorLength;
}




/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 1/0 instances containing 26 lines.
    Modified 0 lines.
---------------------------------------------------------*/
