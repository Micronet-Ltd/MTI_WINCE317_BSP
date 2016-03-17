/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
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

    This module contains the Device Driver Framework functions to read sectors
    from, and write sectors to a FlashFX disk.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvio.c $
    Revision 1.34  2012/03/06 20:22:06Z  billr
    Remove assertion of successful read to permit fault injection tests.
    Revision 1.33  2010/11/09 13:56:42Z  garyp
    Debug messages updated -- no functional changes.
    Revision 1.32  2010/06/10 20:27:39Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.31  2010/04/19 19:11:16Z  garyp
    Minor tweaks to the statistics gathering code.
    Revision 1.30  2010/01/27 03:41:52Z  glenns
    Repair issues exposed by turning on a compiler option to warn of possible
    data loss resulting from implicit typecasts between integer data types.
    Revision 1.29  2010/01/23 21:42:12Z  garyp
    Added FfxDriverSectorFlush().  Enhanced the statistics gathering.
    Revision 1.28  2009/11/24 16:45:11Z  garyp
    Updated to protect the stats gathering with a critical section.  Updated
    to track the sequentialness of operations.
    Revision 1.27  2009/07/31 20:06:19Z  garyp
    Eliminated the ReadAhead feature.
    Revision 1.26  2009/07/22 18:04:35Z  garyp
    Removed an unreferenced label.
    Revision 1.25  2009/07/21 21:17:23Z  garyp
    Merged from the v4.0 branch.  Eliminated a deprecated header.  Eliminated
    code from rev 1.17 which allowed reads from a Disk with no allocator to
    be redirected to the FML.  When this is necesssary it is now handled at
    a higher level.
    Revision 1.24  2009/04/09 21:26:36Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.23  2009/04/07 20:23:16Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.22  2009/03/31 19:19:56Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.21  2009/02/19 19:11:04Z  billr
    bug 2447: compiler warnings if FFXCONF_ALLOCATORSUPPORT is FALSE
    Revision 1.20  2008/05/03 02:55:38Z  garyp
    Cleaned up a handle dereference and updated debug code.
    Revision 1.19  2008/04/10 14:52:09Z  billr
    Make previous work in a debug build.
    Revision 1.18  2008/04/08 19:44:50Z  billr
    Fix warning about unused variable.
    Revision 1.17  2008/03/25 06:53:56Z  Garyp
    Updated to compile and run in instances where an allocator is not included
    at compile time, or perhaps is disabled at run-time.  FfxDriverSectorRead()
    will use FML level reads in that event.
    Revision 1.16  2008/01/13 07:26:27Z  keithg
    Function header updates to support autodoc.
    Revision 1.15  2007/11/02 03:19:57Z  Garyp
    Modified FfxDriverReadData() to return a sector count rather than a Boolean
    TRUE/FALSE.
    Revision 1.14  2007/09/26 23:52:51Z  jeremys
    Renamed a header file.
    Revision 1.13  2006/11/09 00:44:11Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.12  2006/10/09 18:20:04Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.11  2006/08/20 00:10:29Z  Garyp
    Added debug code.
    Revision 1.10  2006/06/12 11:59:18Z  Garyp
    Eliminated the use of the deprecated vbfclientsize().
    Revision 1.9  2006/02/21 22:00:51Z  Pauli
    Added missing status variable.
    Revision 1.8  2006/02/21 04:07:43Z  Garyp
    Updated to use the new FfxVbfRead/WritePages() functions.
    Revision 1.7  2006/02/06 19:40:05Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.6  2006/01/17 03:27:43Z  Garyp
    Updated debugging code.
    Revision 1.5  2006/01/08 02:52:17Z  Garyp
    Modified so ulMaxTransferSize is now a D_UINT32 instead of D_UINT16.
    Revision 1.4  2006/01/05 03:36:33Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.3  2005/12/14 20:50:08Z  Pauli
    Fixed usage of obsolete assertion macros.
    Revision 1.2  2005/12/09 01:32:05Z  Pauli
    Updated to set the size of the disk based on the VBF client size when
    an unrecognized MBR is written.
    Revision 1.1  2005/10/22 06:55:42Z  Pauli
    Initial revision
    Revision 1.2  2005/10/22 07:55:41Z  garyp
    Minor cleanup, removed some dead code.  No functional changes.
    Revision 1.1  2005/10/02 03:03:20Z  Garyp
    Initial revision
    Revision 1.3  2005/09/18 05:26:55Z  garyp
    Modified so that last read/write access times are recorded in VBF rather
    than the driver framework, and are not conditional on the background
    compaction feature being enabled.
    Revision 1.2  2005/08/14 18:52:42Z  garyp
    Fixed FfxDriverReadData() to take a D_UINT32 length parameter
    as the function is structured to take.
    Revision 1.1  2005/08/03 19:30:48Z  pauli
    Initial revision
    Revision 1.4  2005/08/03 19:30:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 03:52:38Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/07/25 21:11:08Z  pauli
    Initial revision
    Revision 1.21  2005/05/05 03:14:48Z  garyp
    Minor debug code changes.
    Revision 1.20  2005/04/06 22:40:08Z  GaryP
    Improved error handling code.
    Revision 1.19  2005/02/04 03:33:37Z  GaryP
    Removed FFXCONF_FORCEALIGNEDIO support (now implemented in the FMSL).
    Revision 1.18  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.17  2004/12/23 22:42:53Z  GaryP
    Minor debug code changes.
    Revision 1.16  2004/12/18 23:23:37Z  GaryP
    In the event of an MBR write, updated so that we default to using the
    original sector size, rather than VBF_BLOCK_SIZE.
    Revision 1.15  2004/11/19 20:23:49Z  GaryP
    Updated to use refactored header files.
    Revision 1.14  2004/11/08 20:47:50Z  GaryP
    Modified to use the updated background compaction API.
    Revision 1.13  2004/10/29 00:57:48Z  GaryP
    Changed garbage collection terminology to compaction.
    Revision 1.12  2004/09/29 00:22:31Z  GaryP
    Minor debug code update.
    Revision 1.11  2004/09/23 03:36:55Z  GaryP
    Changed the device sector length field to be a 32-bit value.
    Revision 1.10  2004/09/17 04:16:52Z  GaryP
    Modified to record the last access time upon completion of the I/O.  Fixed
    to deinit and re-init FATMON if the MBR is written to.
    Revision 1.9  2004/08/16 17:38:25Z  Pauli
    Modified to return number of sectors when forcing an aligned read/write.
    Revision 1.8  2004/08/13 19:44:46Z  GaryP
    General module cleanup.  Modified the return values to be in sectors rather
    than the non-intuitive bytes.  Functions renamed.
    Revision 1.7  2004/08/10 07:30:05Z  garyp
    Added a missing header.
    Revision 1.6  2004/08/06 03:13:27Z  GaryP
    Updated to use the new FAT Monitor functions.
    Revision 1.5  2004/02/01 18:59:12Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.4  2004/02/01 18:59:12  garyp
    Split the alignment specific code out into the module drvioaln.c.
    Revision 1.2.1.3  2003/12/25 00:03:58Z  garyp
    Changed all uses of VBF_API_BLOCK_SIZE to VBF_BLOCK_SIZE and eliminated
    any conditional code based on different values for those settings.
    Revision 1.2.1.2  2003/10/04 18:11:38Z  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/10/04 18:11:38Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/04/21 04:58:34Z  garyp
    Added typecasts and parens to mollify various compilers.
    Revision 1.1  2003/04/15 17:49:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include "drvio.h"

#if FFXCONF_FATMONITORSUPPORT
#include <fatmon.h>
#endif

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>

static D_BOOL FfxDriverWritePages(FFXDISKHANDLE hDisk, D_UINT32 ulStartPage, D_UINT32 ulPageCount, const D_BUFFER *pSrcBuf);
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverSectorRead()

    Read sectors from a FlashFX Disk.

    Parameters:
        hDisk         - The Disk handle
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to read
        pBuffer       - A pointer to the buffer to fill

    Return Value:
        Returns the total number of sectors read.
-------------------------------------------------------------------*/
D_UINT32 FfxDriverSectorRead(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount,
    D_UCHAR        *pBuffer)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_UINT32        ulTotalSectorsRead = 0;
  #if FFXCONF_STATS_DRIVERIO
    D_UINT32        ulElapsedUS;
    D_UINT32        ulAverageUS;
    DCLTIMESTAMP    ts = DclTimeStamp();
  #endif

    DclAssert(pDsk);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverSectorRead() hDisk=%P DISK%u Start=%lU Count=%lU pBuff=%P\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount, pBuffer));

    DclProfilerEnter("FfxDriverSectorRead", 0, ulCount);

    DclAssert(pDsk->ulSectorLength);
    DclAssert(ulCount);
    DclAssert(pBuffer);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(!pDsk->hVBF)
    {
        FFXPRINTF(1, ("FfxDriverSectorRead() can't read from a Disk with no allocator, hDisk=%lX DISK%u Start=%lU Count=%lU\n",
            hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));

        /*  No assert here.  The Disk may be configured with no allocator
        */
    }
    else
    {
        ulTotalSectorsRead = FfxDriverReadData(hDisk, ulStartSector, ulCount, pBuffer);
    }
    
  #else
  
    /*  Unused if FFXCONF_ALLOCATORSUPPORT and FFXCONF_STATS_DRIVERIO are FALSE
    */  
    (void)pDsk;

    FFXPRINTF(1, ("FfxDriverSectorRead() allocator support is disabled, hDisk=%lX DISK%u Start=%lU Count=%lU\n",
         hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));

    /*  No assert here.  Allocator support may be disabled.
    */
  #endif

  #if FFXCONF_STATS_DRIVERIO
    ulElapsedUS = DclTimePassedUS(ts);
    ulAverageUS = ulTotalSectorsRead ? ulElapsedUS / ulTotalSectorsRead : 0;
    
    if(DclCriticalSectionEnter(&pDsk->ulAtomicReadStatsGate, 10000, TRUE))
    {
        pDsk->stats.Read.ulCalls++;
        pDsk->stats.Read.ulTotalSectors += ulTotalSectorsRead;
        pDsk->stats.Read.ullTotalUS += ulElapsedUS;
        
        if(pDsk->stats.Read.ulMaxAverageUS < ulAverageUS)
            pDsk->stats.Read.ulMaxAverageUS = ulAverageUS;

        if(pDsk->stats.Read.ulMaxSectors < ulTotalSectorsRead)
            pDsk->stats.Read.ulMaxSectors = ulTotalSectorsRead;

        if(pDsk->ulNextSeqReadSectorNum == ulStartSector)
        {
            pDsk->stats.Read.ulSeqCalls++;
            pDsk->stats.Read.ulSeqTotalSectors += ulTotalSectorsRead;
        }
        else
        {
            pDsk->stats.Read.ulSeqTotalSectors += ulTotalSectorsRead - 1;
        }
        
        pDsk->ulNextSeqReadSectorNum = ulStartSector + ulTotalSectorsRead;
        
        DclCriticalSectionLeave(&pDsk->ulAtomicReadStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
    }
  #endif

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverSectorRead() returning %lU\n", ulTotalSectorsRead));

     return ulTotalSectorsRead;
}


/*-------------------------------------------------------------------
    Public: FfxDriverSectorWrite()

    Write sectors to a FlashFX Disk.

    Parameters:
        hDisk         - The Disk handle
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to write
        pBuffer       - A pointer to the data to write

    Return Value:
        Returns the total number of sectors written.
-------------------------------------------------------------------*/
D_UINT32 FfxDriverSectorWrite(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount,
    const D_BUFFER *pBuffer)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_UINT32        ulSectorsWritten = 0;
  #if FFXCONF_STATS_DRIVERIO
    D_UINT32        ulElapsedUS;
    D_UINT32        ulAverageUS;
    DCLTIMESTAMP    ts = DclTimeStamp();
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverSectorWrite() hDisk=%P DISK%u Start=%lU Count=%lU pBuff=%P\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount, pBuffer));

    DclProfilerEnter("FfxDriverSectorWrite", 0, ulCount);

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);
    DclAssert(ulCount);
    DclAssert(pBuffer);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(!pDsk->hVBF)
    {
        FFXPRINTF(1, ("FfxDriverSectorWrite() can't write to a Disk with no allocator, hDisk=%lX DISK%u Start=%lU Count=%lU\n",
            hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));

        /*  No assert here.  The Disk may be configured with no allocator.
        */
    }
    else
    {
        D_UINT32    ulStartPage = ulStartSector << pDsk->uSectorToPageShift;
        D_UINT32    ulPageCount = ulCount << pDsk->uSectorToPageShift;

        DclAssert(ulStartPage >> pDsk->uSectorToPageShift == ulStartSector);
        DclAssert(ulPageCount >> pDsk->uSectorToPageShift == ulCount);

        if(FfxDriverWritePages(hDisk, ulStartPage, ulPageCount, pBuffer))
            ulSectorsWritten = ulCount;
    }

  #else

    /*  Unused if FFXCONF_ALLOCATORSUPPORT and FFXCONF_STATS_DRIVERIO are FALSE
    */  
    (void)pDsk; 

    FFXPRINTF(1, ("FfxDriverSectorWrite() allocator support is disabled, hDisk=%lX DISK%u Start=%lU Count=%lU\n",
         hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));

    /*  No assert here.  Allocator support may be disabled.
    */
  #endif

  #if FFXCONF_STATS_DRIVERIO
    ulElapsedUS = DclTimePassedUS(ts);
    ulAverageUS = ulSectorsWritten ? ulElapsedUS / ulSectorsWritten : 0;
        
    if(DclCriticalSectionEnter(&pDsk->ulAtomicWriteStatsGate, 10000, TRUE))
    {
        pDsk->stats.Write.ulCalls++;
        pDsk->stats.Write.ulTotalSectors += ulSectorsWritten;
        pDsk->stats.Write.ullTotalUS += ulElapsedUS;
        
        if(pDsk->stats.Write.ulMaxAverageUS < ulAverageUS)
            pDsk->stats.Write.ulMaxAverageUS = ulAverageUS;

        if(pDsk->stats.Write.ulMaxSectors < ulSectorsWritten)
            pDsk->stats.Write.ulMaxSectors = ulSectorsWritten;

        if(pDsk->ulNextSeqWriteSectorNum == ulStartSector)
        {
            pDsk->stats.Write.ulSeqCalls++;
            pDsk->stats.Write.ulSeqTotalSectors += ulSectorsWritten;
        }
        else
        {
            pDsk->stats.Write.ulSeqTotalSectors += ulSectorsWritten - 1;
        }
        
        pDsk->ulNextSeqWriteSectorNum = ulStartSector + ulSectorsWritten;
        
        DclCriticalSectionLeave(&pDsk->ulAtomicWriteStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
    }
  #endif

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverSectorWrite() returning %lU \n", ulSectorsWritten));

    return ulSectorsWritten;
}


/*-------------------------------------------------------------------
    Public: FfxDriverSectorFlush()

    Flush a range of sectors on a FlashFX Disk.  This API supports
    flushing a range of sectors, however it must be assumed that
    specifying a range could result in all dirty sectors being 
    flushed.

    *Note* -- At this time, FlashFX does not cache any unwritten,
    data, and this function will do nothing.

    Parameters:
        hDisk         - The Disk handle
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to flush.  Set this 
                        value to D_UINT32_MAX to flush all the 
                        remaining sectors.

    Return Value:
        Returns the total number of dirty sectors which were flushed.
-------------------------------------------------------------------*/
D_UINT32 FfxDriverSectorFlush(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_UINT32        ulSectorsFlushed = 0;
  #if FFXCONF_STATS_DRIVERIO
    D_UINT32        ulElapsedUS;
    D_UINT32        ulAverageUS;
    DCLTIMESTAMP    ts = DclTimeStamp();
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverSectorFlush() hDisk=%P DISK%u Start=%lU Count=%lU\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));
        
    DclProfilerEnter("FfxDriverSectorFlush", 0, 0);

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);
    DclAssert(ulCount);

    /*
        if(ulCount == D_UINT32_MAX)
            ulCount = pDsk->fpdi.ulTotalSectors - ulStartSector;

        ---> If we ever actually need to flush something, do it here <---
    */

  #if FFXCONF_STATS_DRIVERIO
    ulElapsedUS = DclTimePassedUS(ts);
    ulAverageUS = ulSectorsFlushed ? ulElapsedUS / ulSectorsFlushed : 0;

    if(DclCriticalSectionEnter(&pDsk->ulAtomicFlushStatsGate, 10000, TRUE))
    {
        pDsk->stats.Flush.ulCalls++;
        pDsk->stats.Flush.ulTotalSectors += ulSectorsFlushed;
        pDsk->stats.Flush.ullTotalUS += ulElapsedUS;
        
        if(pDsk->stats.Flush.ulMaxAverageUS < ulAverageUS)
            pDsk->stats.Flush.ulMaxAverageUS = ulAverageUS;

        if(pDsk->stats.Flush.ulMaxSectors < ulSectorsFlushed)
            pDsk->stats.Flush.ulMaxSectors = ulSectorsFlushed;
        
        /*  Record the max actually flushed for any individual call
        */
        if(pDsk->stats.ulFlushActualMax < ulSectorsFlushed)
            pDsk->stats.ulFlushActualMax = ulSectorsFlushed;

        /*  These value will always be zero for now, but at whatever point
            we really do have data to flush, the ulFlushActual value should
            accumulate the total.
        */            
        pDsk->stats.ulFlushActualTotal += ulSectorsFlushed;
         
        DclCriticalSectionLeave(&pDsk->ulAtomicFlushStatsGate);
    }
    else
    {
        /*  Really should not ever time out...
        */
        DclError();
    }
  #else

    /*  Unused in release mode 
    */
    (void)pDsk; 
  
  #endif

    DclProfilerLeave(ulSectorsFlushed);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverSectorFlush() returning %lU \n", ulSectorsFlushed));

    return ulSectorsFlushed;
}


#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Private: FfxDriverReadData()

    This function is used to read sectors from a FlashFX Disk.

    Parameters:
        hDisk         - The Disk handle
        ulStartSector - The starting sector
        ulSectorCount - The number of sectors to read
        pBuffer       - A pointer to the buffer to fill

    Return Value:
        Returns the total number of sectors read.
 -------------------------------------------------------------------*/
D_UINT32 FfxDriverReadData(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulSectorCount,
    D_BUFFER       *pBuffer)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_UINT32        ulSectorsRead = 0;
    D_UINT32        ulStartPage;
    D_UINT32        ulPageCount;
    FFXIOSTATUS     ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEINDENT),
        "FfxDriverReadData() hDisk=%P StartSector=%lX Count=%lX pBuff=%P\n",
        hDisk, ulStartSector, ulSectorCount, pBuffer));

    DclProfilerEnter("FfxDriverReadData", 0, ulSectorCount);

    DclAssert(pDsk);
    DclAssert(ulSectorCount);
    DclAssert(pBuffer);

    ulStartPage = ulStartSector << pDsk->uSectorToPageShift;
    ulPageCount = ulSectorCount << pDsk->uSectorToPageShift;

    DclAssert(ulStartPage >> pDsk->uSectorToPageShift == ulStartSector);
    DclAssert(ulPageCount >> pDsk->uSectorToPageShift == ulSectorCount);

    ioStat = FfxVbfReadPages(pDsk->hVBF, ulStartPage, ulPageCount, pBuffer);
    if(!IOSUCCESS(ioStat, ulPageCount))
    {
        FFXPRINTF(1, ("FfxDriverReadData() Start=%lU Count=%lU failed with status %s\n",
            ulStartPage, ulPageCount, FfxDecodeIOStatus(&ioStat)));
    }

    ulSectorsRead = ioStat.ulCount >> pDsk->uSectorToPageShift;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 2, TRACEUNDENT),
        "FfxDriverReadData() returning %lU \n", ulSectorsRead));

     return ulSectorsRead;
}


/*-------------------------------------------------------------------
    Local: FfxDriverWritePages()

    This function is used to write sectors to a FlashFX Disk.

    If FAT Monitor is enabled, writes will be done using
    FfxFatMonWrite().

    If FAT Monitor is enabled and a write to the boot record is
    detected, the BPB will be revalidated and loaded into the
    FFXDEVINFO structure.

    Parameters:
        hDisk       - The Disk handle
        ulStartPage - The starting page to write
        ulPageCount - The number of pages to write
        pBuffer     - A pointer to the buffer to use

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL FfxDriverWritePages(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount,
    const D_BUFFER *pSrcBuf)
{
    FFXDISKINFO    *pDsk = *hDisk;
    D_BOOL          fSuccess;

    DclProfilerEnter("FfxDriverWritePages", 0, ulPageCount);

  #if FFXCONF_FATMONITORSUPPORT
    {
        /*  If we are writing the first sector, deinitalize FAT Monitor
        */
        if(ulStartPage == 0)
            FfxFatMonDeinit(hDisk);

        fSuccess = FfxFatMonWrite(hDisk, ulStartPage, ulPageCount, pSrcBuf);

        if(!fSuccess)
        {
            FFXPRINTF(1, ("FfxFatMonWrite FAILED!\n"));
        }
    }
  #else
    {
        FFXIOSTATUS ioStat;

        ioStat = FfxVbfWritePages(pDsk->hVBF, ulStartPage, ulPageCount, pSrcBuf);
        if(IOSUCCESS(ioStat, ulPageCount))
            fSuccess = TRUE;
        else
            fSuccess = FALSE;
    }
  #endif

    if(fSuccess && ulStartPage == 0)
    {
        FFXPRINTF(1, ("Detected sector 0 write\n"));

        /*  If we are rewriting the MBR, reload all the physical disk
            information we need.  It is possible that this sector being
            written is not really an MBR, but rather a boot record, if
            the system is configured to NOT use an MBR.  In this case
            we create a pseudo partition table entry.
        */
        if(!FfxGetPhysicalDiskParams(&pDsk->fpdi, pSrcBuf, pDsk->ulSectorLength))
        {
            DclError();
            fSuccess = FALSE;
            goto WritePagesCleanup;
        }

        if(!pDsk->fpdi.ulTotalSectors)
        {
            VBFDISKINFO vdi;

            /*  This flag should always have been turned off if we
                couldn't calculate the total sectors.
            */
            DclAssert(!pDsk->fpdi.fFoundValidPartitions);

            if(FfxVbfDiskInfo(pDsk->hVBF, &vdi) != FFXSTAT_SUCCESS)
            {
                DclError();
                fSuccess = FALSE;
                goto WritePagesCleanup;
            }

            /*  Since there's nothing on the disk that we can use to determine
                the size, use the VBF client size (since we know that VBF is
                mounted).
            */
            if(!FfxCalcPhysicalDiskParams(&pDsk->fpdi, vdi.ulTotalPages))
            {
                DclError();
                fSuccess = FALSE;
                goto WritePagesCleanup;
            }

            /*  After successfully having executed the code above, the
                total sectors value MUST now be initialized
            */
            DclAssert(pDsk->fpdi.ulTotalSectors);
        }

      #if FFXCONF_FATMONITORSUPPORT
        FfxFatMonInit(hDisk);
      #endif
    }

  WritePagesCleanup:

    DclProfilerLeave(0UL);

    return fSuccess;
}

#endif

