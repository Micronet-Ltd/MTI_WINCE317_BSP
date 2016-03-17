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

    This module contains the Device Driver Framework functions to discard
    sectors from a FlashFX disk.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvdisc.c $
    Revision 1.21  2010/11/09 13:56:42Z  garyp
    Debug messages updated -- no functional changes.
    Revision 1.20  2010/04/19 19:11:03Z  garyp
    Minor tweaks to the statistics gathering code.
    Revision 1.19  2010/01/23 21:42:33Z  garyp
    Enhanced the statistics gathering.
    Revision 1.18  2009/11/24 16:45:11Z  garyp
    Updated to protect the stats gathering with a critical section.  Updated
    to track the sequentialness of operations.
    Revision 1.17  2009/07/31 20:06:20Z  garyp
    Eliminated the ReadAhead feature.
    Revision 1.16  2009/07/18 00:54:41Z  garyp
    Eliminated use of an obsolete header.
    Revision 1.15  2009/02/19 19:11:10Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.14  2009/02/19 19:11:10Z  billr
    bug 2447: compiler warnings if FFXCONF_ALLOCATORSUPPORT is FALSE
    Revision 1.13  2009/02/08 08:41:15Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.12  2008/03/31 16:09:40Z  garyp
    Debug code updated.
    Revision 1.11  2008/03/25 02:30:41Z  Garyp
    Updated to compile when allocator support is disabled.
    Revision 1.10  2008/03/22 20:25:49Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.9  2008/01/13 07:26:22Z  keithg
    Function header updates to support autodoc.
    Revision 1.8  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2006/11/09 00:49:25Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.6  2006/10/17 00:31:34Z  Garyp
    Updated to use the VBF page oriented interface.
    Revision 1.5  2006/02/06 18:08:26Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.4  2006/01/07 22:02:45Z  Garyp
    Modified so ulMaxTransferSize is now a D_UINT32 instead of D_UINT16.
    Revision 1.3  2006/01/04 23:39:36Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/25 05:14:35Z  Garyp
    Added/updated debugging code -- no functional changes.
    Revision 1.1  2005/10/22 06:44:46Z  Pauli
    Initial revision
    Revision 1.2  2005/09/18 05:26:55Z  garyp
    Modified so that last read/write access times are recorded in VBF rather
    than the driver framework, and are not conditional on the background
    compaction feature being enabled.
    Revision 1.1  2005/08/03 19:30:44Z  pauli
    Initial revision
    Revision 1.4  2005/08/03 19:30:44Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 05:41:16Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/05/05 03:14:48Z  pauli
    Initial revision
    Revision 1.9  2005/05/05 03:14:48Z  garyp
    Minor debug code changes.
    Revision 1.8  2004/12/30 17:32:41Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.7  2004/12/23 22:42:53Z  GaryP
    Minor debug code changes.
    Revision 1.6  2004/11/19 20:23:49Z  GaryP
    Updated to use refactored header files.
    Revision 1.5  2004/11/08 20:47:50Z  GaryP
    Modified to use the updated background compaction API.
    Revision 1.4  2004/10/29 00:57:48Z  GaryP
    Changed garbage collection terminology to compaction.
    Revision 1.3  2004/09/23 03:36:55Z  GaryP
    Changed the device sector length field to be a 32-bit value.
    Revision 1.2  2004/09/17 04:16:52Z  GaryP
    Modified to record the last access time upon completion of the discard.
    Revision 1.1  2004/08/13 19:55:08Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxstats.h>

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif


/*-------------------------------------------------------------------
    Public: FfxDriverSectorDiscard()

    Discard sectors on a FlashFX Disk.

    Parameters:
        pDev          - A pointer to the FFXDEVINFO structure to use
        ulStartSector - The starting sector number
        ulCount       - The number of sectors to discard

    Return Value:
        Returns an FFXSTATUS code indicating the success or failure
        of the operation.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverSectorDiscard(
    FFXDISKHANDLE   hDisk,
    D_UINT32        ulStartSector,
    D_UINT32        ulCount)
{
    FFXDISKINFO    *pDsk = *hDisk;
    FFXSTATUS       ffxStat = FFXSTAT_DRV_SECTORRANGEINVALID;
  #if FFXCONF_STATS_DRIVERIO
    D_UINT32        ulElapsedUS;
    D_UINT32        ulAverageUS;
    DCLTIMESTAMP    ts = DclTimeStamp();
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DRIVERFW, 1, TRACEINDENT),
        "FfxDriverSectorDiscard() hDisk=%P DISK%u Start=%lU Count=%lU\n",
        hDisk, pDsk->Conf.nDiskNum, ulStartSector, ulCount));

    DclProfilerEnter("FfxDriverSectorDiscard", 0, ulCount);

    DclAssert(pDsk);
    DclAssert(pDsk->ulSectorLength);

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    /*  No assert here.  The driver may be configured with no allocator
    */
    if(!pDsk->hVBF)
        goto DiscardCleanup;

    {
        FFXIOSTATUS ioStat;
        D_UINT32    ulStartPage = ulStartSector << pDsk->uSectorToPageShift;
        D_UINT32    ulPageCount = ulCount << pDsk->uSectorToPageShift;

        if(ulStartPage >> pDsk->uSectorToPageShift != ulStartSector)
        {
            DclError();
            goto DiscardCleanup;
        }

        if(ulPageCount >> pDsk->uSectorToPageShift != ulCount)
        {
            DclError();
            goto DiscardCleanup;
        }

        ioStat = FfxVbfDiscardPages(pDsk->hVBF, ulStartPage, ulPageCount);
        ffxStat = ioStat.ffxStat;
    }

  DiscardCleanup:

  #else
  
    (void)pDsk; 
  
    ffxStat = FFXSTAT_DRV_NOALLOCATOR;
  #endif

  #if FFXCONF_STATS_DRIVERIO
    ulElapsedUS = DclTimePassedUS(ts);
    ulAverageUS = ulCount ? ulElapsedUS / ulCount : 0;

    if(DclCriticalSectionEnter(&pDsk->ulAtomicDiscardStatsGate, 10000, TRUE))
    {
        pDsk->stats.Discard.ulCalls++;
        pDsk->stats.Discard.ulTotalSectors += ulCount;
        pDsk->stats.Discard.ullTotalUS += ulElapsedUS;
        
        if(pDsk->stats.Discard.ulMaxAverageUS < ulAverageUS)
            pDsk->stats.Discard.ulMaxAverageUS = ulAverageUS;
        
        if( pDsk->stats.Discard.ulMaxSectors < ulCount)
            pDsk->stats.Discard.ulMaxSectors = ulCount;

        if( pDsk->ulNextSeqDiscardSectorNum == ulStartSector)
        {
            pDsk->stats.Discard.ulSeqCalls++;
            pDsk->stats.Discard.ulSeqTotalSectors += ulCount;
        }
        else
        {
            pDsk->stats.Discard.ulSeqTotalSectors += ulCount - 1;
        }
        
        pDsk->ulNextSeqDiscardSectorNum = ulStartSector + ulCount;
        
        DclCriticalSectionLeave(&pDsk->ulAtomicDiscardStatsGate);
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
        "FfxDriverSectorDiscard() returning Status %lX\n", ffxStat));

    return ffxStat;
}


