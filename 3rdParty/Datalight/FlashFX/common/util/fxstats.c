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

    This module contains routines to display FlashFX statistics.

    Note that this module explicitly resides in a location where it is built
    into a separate library such that it may be linked into an application
    completely separate from the device driver code.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxstats.c $
    Revision 1.22  2010/07/31 19:47:13Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.21  2010/07/13 00:19:16Z  garyp
    Minor stats updates -- no functional changes (to non-stats stuff).
    Revision 1.20  2010/01/23 21:44:10Z  garyp
    Enhanced the driver statistics.  Now support flush.
    Revision 1.19  2009/12/02 21:48:10Z  garyp
    Added more driver I/O stats for tracking sequentialness.
    Revision 1.18  2009/07/22 00:15:59Z  garyp
    Merged from the v4.0 branch.  Updated to handle some additional stats.
    Documentation updated.
    Revision 1.17  2009/04/01 15:43:46Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.16  2009/01/19 04:34:40Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.15  2008/12/19 06:19:39Z  keithg
    Conditioned statistics per BBM support.
    Revision 1.14  2008/05/08 19:01:00Z  garyp
    Further enhancement of the FML block statistics information.
    Revision 1.13  2008/05/08 01:12:12Z  garyp
    Updated to work with the new format of the FML block statistics.
    Revision 1.12  2008/01/13 07:27:15Z  keithg
    Function header updates to support autodoc.
    Revision 1.11  2007/12/13 20:46:45Z  Garyp
    Updated to support displaying FML erase counts.
    Revision 1.10  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/09/12 13:30:38Z  thomd
    Removed unused code - cleanup of ffxconf.h
    Revision 1.8  2007/06/21 18:54:20Z  Garyp
    Refactored the PerfLog output stuff so that it is separate from the stats
    display stuff.
    Revision 1.7  2007/04/15 17:51:55Z  Garyp
    Updated to handle BBM stats.  Fixed the PerfLog stuff to not require
    typecasts to denote unused fields.
    Revision 1.6  2007/04/13 17:15:02Z  timothyj
    Added casts for '-1' arguments to function expecting unsigned.
    Revision 1.5  2007/04/06 03:13:58Z  Garyp
    Modified the use of boolean types so we build cleanly in CE.
    Revision 1.4  2007/04/06 01:51:07Z  Garyp
    Added the ability to display the stats in PerfLog form.
    Revision 1.3  2006/11/10 20:19:54Z  Garyp
    Updated to display driver and FML I/O statistics.
    Revision 1.2  2006/08/07 03:19:53Z  Garyp
    Minor cleanup of the messages.
    Revision 1.1  2006/05/08 08:05:50Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxstats.h>

static void DisplayDiskIOStatsCategory(const char *pszName, const DRIVERIOOPSTATS *pOpStats);


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskCompStatsDisplay()

    Display compaction statistics if FFXCONF_STATS_COMPACTION
    is enabled.

    Parameters:
        pStat    - The FFXCOMPSTATS structure to use
        fVerbose - Display verbose statistics

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDriverDiskCompStatsDisplay(
    const FFXCOMPSTATS *pStat,
    D_BOOL              fVerbose)
{
    DclAssert(pStat);

    (void)pStat;
    (void)fVerbose;

    #if FFXCONF_STATS_COMPACTION

        DclPrintf("Driver Compaction Information\n");
        DclPrintf("  Compile Time Settings\n");

      #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
        DclPrintf("    Compaction Model             Background Thread\n");
        DclPrintf("    Sleep Time when Complete            %10u ms\n",   pStat->ulSleepMS);
        DclPrintf("    Thread Priority                     %10u\n",      pStat->uThreadPriority);
        DclPrintf("  Thread Name                         %12s\n",        pStat->szThreadName);
        DclPrintf("  Compactions Initiated                 %10lU\n",     pStat->ulCompactionsInitiated);
        DclPrintf("  Total Time Compacting                 %10lU ms\n",  pStat->ulTotalMS);
        DclPrintf("  Slowest Compaction                    %10lU ms\n\n",pStat->ulSlowestCompactMS);

      #elif FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDIDLE

        DclPrintf("    Compaction Model               Background Idle\n");

      #elif FFX_COMPACTIONMODEL == FFX_COMPACT_SYNCHRONOUS

        DclPrintf("    Compaction Model                   Synchronous\n\n");

      #else

        #error "FFX: Invalid compaction model"

      #endif

    #endif

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxDriverDiskIOStatsDisplay()

    Display Disk I/O statistics information, if available.

    Parameters:
        pStat    - The DRIVERIOSTATS structure to use
        fVerbose - Display verbose statistics

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDriverDiskIOStatsDisplay(
    const DRIVERIOSTATS    *pStat,
    D_BOOL                  fVerbose)
{
    DclAssertReadPtr(pStat, sizeof(*pStat));

    (void)fVerbose;
/*
    DclPrintf("Disk I/O Stats*  [     Total     ]            [ Sequential ] [  Efficiency %  ]\n");
    DclPrintf("                  Calls    Sectors   Avg   Max Calls Sectors  Size   Seq Wghted\n");
*/    
    DclPrintf("Disk I/O Stats*  [          Total        ][  Average  ][  Maximum  ][   Sequential   ]\n");
    DclPrintf("                  Calls    Sectors      MS Sectors   MS Sectors   MS   Calls   Sectors\n");

    DisplayDiskIOStatsCategory("Reads", &pStat->Read);
    DisplayDiskIOStatsCategory("Writes", &pStat->Write);
    DisplayDiskIOStatsCategory("Discards", &pStat->Discard);
    DisplayDiskIOStatsCategory("Flushes",  &pStat->Flush);

/*  Test Data

    pStat->Read.ulSectorIOCalls = 1;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 0;
    pStat->Read.ulSectorIOSeqTotal = 99;
    DisplayDiskIOStatsCategory("Test1", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 2;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 1;
    pStat->Read.ulSectorIOSeqTotal = 99;
    DisplayDiskIOStatsCategory("Test2", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 2;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 0;
    pStat->Read.ulSectorIOSeqTotal = 98;
    DisplayDiskIOStatsCategory("Test3", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 4;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 3;
    pStat->Read.ulSectorIOSeqTotal = 99;
    DisplayDiskIOStatsCategory("Test4", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 4;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 0;
    pStat->Read.ulSectorIOSeqTotal = 96;
    DisplayDiskIOStatsCategory("Test5", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 50;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 0;
    pStat->Read.ulSectorIOSeqTotal = 50;
    DisplayDiskIOStatsCategory("Test6", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 100;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 99;
    pStat->Read.ulSectorIOSeqTotal = 99;
    DisplayDiskIOStatsCategory("Test7", &pStat->Read);

    pStat->Read.ulSectorIOCalls = 100;
    pStat->Read.ulSectorIOTotal = 100;
    pStat->Read.ulSectorIOMax = 999;
    pStat->Read.ulSectorIOSeqCalls = 0;
    pStat->Read.ulSectorIOSeqTotal = 0;
    DisplayDiskIOStatsCategory("Test8", &pStat->Read);
*/    
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxFmlStatsDisplay()

    Display FML statistics information, if available.

    Parameters:
        pStat    - The FFXFMLSTATS structure to use
        fVerbose - Display verbose statistics

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxFmlStatsDisplay(
    const FFXFMLSTATS  *pStat,
    D_BOOL              fVerbose)
{
    DclAssert(pStat);

    DclPrintf("FML I/O Statistics     Calls      Items   Avg  Max\n");
    DclPrintf("  ReadPages        %10lU %10lU %4lU %4lU*\n",
        pStat->ulPageReadRequests, pStat->ulPageReadTotal,
        pStat->ulPageReadRequests ? pStat->ulPageReadTotal / pStat->ulPageReadRequests : 0,
        pStat->ulPageReadMax);

    DclPrintf("  WritePages       %10lU %10lU %4lU %4lU*\n",
        pStat->ulPageWriteRequests, pStat->ulPageWriteTotal,
        pStat->ulPageWriteRequests ? pStat->ulPageWriteTotal / pStat->ulPageWriteRequests : 0,
        pStat->ulPageWriteMax);

    DclPrintf("  EraseBlocks      %10lU %10lU %4lU %4lU*\n",
        pStat->ulBlockEraseRequests, pStat->ulBlockEraseTotal,
        pStat->ulBlockEraseRequests ? pStat->ulBlockEraseTotal / pStat->ulBlockEraseRequests : 0,
        pStat->ulBlockEraseMax);

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    DclPrintf("  ReadControlBytes %10lU %10lU %4lU %4lU*\n",
        pStat->ulControlByteReadRequests, pStat->ulControlByteReadTotal,
        pStat->ulControlByteReadRequests ? pStat->ulControlByteReadTotal / pStat->ulControlByteReadRequests : 0,
        pStat->ulControlByteReadMax);

    DclPrintf("  WriteControlBytes%10lU %10lU %4lU %4lU*\n",
        pStat->ulControlByteWriteRequests, pStat->ulControlByteWriteTotal,
        pStat->ulControlByteWriteRequests ? pStat->ulControlByteWriteTotal / pStat->ulControlByteWriteRequests : 0,
        pStat->ulControlByteWriteMax);
  #endif

  #if FFXCONF_NANDSUPPORT
    DclPrintf("  ReadUncorrectedPages%7lU %10lU %4lU %4lU*\n",
        pStat->ulUncorrectedPageReadRequests, pStat->ulUncorrectedPageReadTotal,
        pStat->ulUncorrectedPageReadRequests ? pStat->ulUncorrectedPageReadTotal / pStat->ulUncorrectedPageReadRequests : 0,
        pStat->ulUncorrectedPageReadMax);

    DclPrintf("  WriteUncorrectedPages%6lU %10lU %4lU %4lU*\n",
        pStat->ulUncorrectedPageWriteRequests, pStat->ulUncorrectedPageWriteTotal,
        pStat->ulUncorrectedPageWriteRequests ? pStat->ulUncorrectedPageWriteTotal / pStat->ulUncorrectedPageWriteRequests : 0,
        pStat->ulUncorrectedPageWriteMax);

    DclPrintf("  ReadTaggedPages  %10lU %10lU %4lU %4lU*\n",
        pStat->ulTaggedPageReadRequests, pStat->ulTaggedPageReadTotal,
        pStat->ulTaggedPageReadRequests ? pStat->ulTaggedPageReadTotal / pStat->ulTaggedPageReadRequests : 0,
        pStat->ulTaggedPageReadMax);

    DclPrintf("  WriteTaggedPages %10lU %10lU %4lU %4lU*\n",
        pStat->ulTaggedPageWriteRequests, pStat->ulTaggedPageWriteTotal,
        pStat->ulTaggedPageWriteRequests ? pStat->ulTaggedPageWriteTotal / pStat->ulTaggedPageWriteRequests : 0,
        pStat->ulTaggedPageWriteMax);

    DclPrintf("  ReadRawPages     %10lU %10lU %4lU %4lU*\n",
        pStat->ulRawPageReadRequests, pStat->ulRawPageReadTotal,
        pStat->ulRawPageReadRequests ? pStat->ulRawPageReadTotal / pStat->ulRawPageReadRequests : 0,
        pStat->ulRawPageReadMax);

    DclPrintf("  WriteRawPages    %10lU %10lU %4lU %4lU*\n",
        pStat->ulRawPageWriteRequests, pStat->ulRawPageWriteTotal,
        pStat->ulRawPageWriteRequests ? pStat->ulRawPageWriteTotal / pStat->ulRawPageWriteRequests : 0,
        pStat->ulRawPageWriteMax);

    DclPrintf("  ReadSpareAreas   %10lU %10lU %4lU %4lU*\n",
        pStat->ulSpareAreaReadRequests, pStat->ulSpareAreaReadTotal,
        pStat->ulSpareAreaReadRequests ? pStat->ulSpareAreaReadTotal / pStat->ulSpareAreaReadRequests : 0,
        pStat->ulSpareAreaReadMax);

    DclPrintf("  WriteSpareAreas  %10lU %10lU %4lU %4lU*\n",
        pStat->ulSpareAreaWriteRequests, pStat->ulSpareAreaWriteTotal,
        pStat->ulSpareAreaWriteRequests ? pStat->ulSpareAreaWriteTotal / pStat->ulSpareAreaWriteRequests : 0,
        pStat->ulSpareAreaWriteMax);

    DclPrintf("  ReadTags         %10lU %10lU %4lU %4lU*\n",
        pStat->ulTagReadRequests, pStat->ulTagReadTotal,
        pStat->ulTagReadRequests ? pStat->ulTagReadTotal / pStat->ulTagReadRequests : 0,
        pStat->ulTagReadMax);

    DclPrintf("  WriteTags        %10lU %10lU %4lU %4lU*\n",
        pStat->ulTagWriteRequests, pStat->ulTagWriteTotal,
        pStat->ulTagWriteRequests ? pStat->ulTagWriteTotal / pStat->ulTagWriteRequests : 0,
        pStat->ulTagWriteMax);
  #endif

    DclPrintf("\n");

    if(pStat->pBlockStats)
    {
        D_UINT32    bb;
        D_UINT32    ulMinErases = D_UINT32_MAX;
        D_UINT32    ulMaxErases = 0;
        D_UINT32    ulMinReads = D_UINT32_MAX;
        D_UINT32    ulMaxReads = 0;
        D_UINT32    ulMinWrites = D_UINT32_MAX;
        D_UINT32    ulMaxWrites = 0;
        D_UINT32    ulPerEraseMinReads = D_UINT32_MAX;
        D_UINT32    ulPerEraseMaxReads = 0;
        D_UINT32    ulPerEraseMinWrites = D_UINT32_MAX;
        D_UINT32    ulPerEraseMaxWrites = 0;
        D_UINT32    ulTotalErases = 0;
        D_UINT32    ulAvgErases;
        D_UINT64    ullTotalReads;
        D_UINT64    ullTotalWrites;
        D_UINT64    ullAvgReads;
        D_UINT64    ullAvgWrites;
        D_UINT64    ullPerEraseAvgReads;
        D_UINT64    ullPerEraseAvgWrites;

        DclUint64AssignUint32(&ullTotalReads, 0);
        DclUint64AssignUint32(&ullTotalWrites, 0);

        DclPrintf("FML Per Block Statistics (%lU tracked blocks)\n", pStat->ulBlockStatsCount);

        if(fVerbose)
        {
            DclPrintf("                                        [   Per Erase    ]\n");
            DclPrintf("  Block#    Erases  TotReads TotWrites  AvgReads AvgWrites\n");
        }

        for(bb=0; bb<pStat->ulBlockStatsCount; bb++)
        {
            D_UINT32    ulAvgReads;
            D_UINT32    ulAvgWrites;

            /*  So long as ulErases is non-zero, calculate the average reads
                and writes per erase.  If ulErases is zero, then simply
                report the unaveraged values as the average (basically
                treating ulTotalErases as '1').  Plainly the flash had to
                have been erased at least once, even if that is not captured
                in our stats.
            */
            ulAvgReads = pStat->pBlockStats[bb].ulReads;
            ulAvgWrites = pStat->pBlockStats[bb].ulWrites;

            if(pStat->pBlockStats[bb].ulErases)
            {
                ulAvgReads /= pStat->pBlockStats[bb].ulErases;
                ulAvgWrites /= pStat->pBlockStats[bb].ulErases;
            }

            ulTotalErases += pStat->pBlockStats[bb].ulErases;
            ulMinErases         = DCLMIN(ulMinErases, pStat->pBlockStats[bb].ulErases);
            ulMaxErases         = DCLMAX(ulMaxErases, pStat->pBlockStats[bb].ulErases);

            DclUint64AddUint32(&ullTotalReads,  pStat->pBlockStats[bb].ulReads);
            ulMinReads          = DCLMIN(ulMinReads,  pStat->pBlockStats[bb].ulReads);
            ulMaxReads          = DCLMAX(ulMaxReads,  pStat->pBlockStats[bb].ulReads);
            ulPerEraseMinReads  = DCLMIN(ulPerEraseMinReads,  ulAvgReads);
            ulPerEraseMaxReads  = DCLMAX(ulPerEraseMaxReads,  ulAvgReads);

            DclUint64AddUint32(&ullTotalWrites, pStat->pBlockStats[bb].ulWrites);
            ulMinWrites         = DCLMIN(ulMinWrites, pStat->pBlockStats[bb].ulWrites);
            ulMaxWrites         = DCLMAX(ulMaxWrites, pStat->pBlockStats[bb].ulWrites);
            ulPerEraseMinWrites = DCLMIN(ulPerEraseMinWrites,  ulAvgWrites);
            ulPerEraseMaxWrites = DCLMAX(ulPerEraseMaxWrites,  ulAvgWrites);

            if(fVerbose)
            {
                DclPrintf("%8lU %9lU %9lU %9lU %9lU %9lU\n", bb,
                    pStat->pBlockStats[bb].ulErases,
                    pStat->pBlockStats[bb].ulReads,
                    pStat->pBlockStats[bb].ulWrites,
                    ulAvgReads, ulAvgWrites);
            }
        }

        ulAvgErases = ulTotalErases / pStat->ulBlockStatsCount;

        ullAvgReads = ullTotalReads;
        DclUint64DivUint32(&ullAvgReads, pStat->ulBlockStatsCount);

        ullAvgWrites = ullTotalWrites;
        DclUint64DivUint32(&ullAvgWrites, pStat->ulBlockStatsCount);

        /*  Similar to what is done above -- for the purposes of calculating
            the averages, zero total erases is the same as 1 erase per block.
        */
        ullPerEraseAvgReads = ullTotalReads;
        ullPerEraseAvgWrites = ullTotalWrites;

        if(ulTotalErases)
        {
            DclUint64DivUint32(&ullPerEraseAvgReads, ulTotalErases);
            DclUint64DivUint32(&ullPerEraseAvgWrites, ulTotalErases);
        }
        else
        {
            DclUint64DivUint32(&ullPerEraseAvgReads, pStat->ulBlockStatsCount);
            DclUint64DivUint32(&ullPerEraseAvgWrites, pStat->ulBlockStatsCount);
        }

        DclPrintf("  Block Stats Summary:      [      Per Block      ] [      Per Erase      ]\n");
        DclPrintf("                      Count Minimum Average Maximum Minimum Average Maximum\n");
        DclPrintf("    Erases:  %14lU %7lU %7lU %7lU     n/a     n/a     n/a\n", ulTotalErases,  ulMinErases, ulAvgErases,  ulMaxErases);
        DclPrintf("    Reads:  %15llU %7lU %7llU %7lU %7lU %7llU %7lU\n",        VA64BUG(ullTotalReads),  ulMinReads,  VA64BUG(ullAvgReads),  ulMaxReads,  ulPerEraseMinReads,  VA64BUG(ullPerEraseAvgReads),  ulPerEraseMaxReads);
        DclPrintf("    Writes: %15llU %7lU %7llU %7lU %7lU %7llU %7lU\n",        VA64BUG(ullTotalWrites), ulMinWrites, VA64BUG(ullAvgWrites), ulMaxWrites, ulPerEraseMinWrites, VA64BUG(ullPerEraseAvgWrites), ulPerEraseMaxWrites);

        DclPrintf("\n");
    }

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxBbmStatsDisplay()

    Display BBM statistics information, if available.

    Parameters:
        pStat    - The FFXBBMSTATS structure to use
        fVerbose - Display verbose statistics

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxBbmStatsDisplay(
    const FFXBBMSTATS  *pStat,
    D_BOOL              fVerbose)
{
    DclAssert(pStat);

    (void)fVerbose;

    DclPrintf("BBM Statistics\n");
    DclPrintf("  Total blocks in device                %10lU\n", pStat->ulTotalBlocks);
    DclPrintf("  Spare blocks reserved for BBM         %10lU\n", pStat->ulSpareBlocks);
    DclPrintf("    Overhead blocks                     %10lU\n", pStat->ulBbmOverhead);
    DclPrintf("    Factory bad blocks                  %10lU\n", pStat->ulFactoryBadBlocks);
    DclPrintf("    Retired blocks                      %10lU\n", pStat->ulRetiredBlocks);
    DclPrintf("    Temporary mappings                  %10lU\n", pStat->ulTemporaryBlocks);
    DclPrintf("    Free (Unused) BBM blocks            %10lU\n", pStat->ulFreeBlocks);
    DclPrintf("  Total mapping requests                %10lU\n", pStat->ulMapCacheRequests);
    DclPrintf("  Total map cache hits                  %10lU\n", pStat->ulMapCacheHits);
    DclPrintf("  BBM Instances                         %10lU\n", pStat->ulBbmInstances);
    return;
}


/*-------------------------------------------------------------------
    Local: DisplayDiskIOStatsCategory()

    Display I/O statistics for the specified category.

    ToDo: 
    - Finish the statistical rating for operation size and sequentiality.

    Parameters:
        pszName  - A pointer to the null-terminated category name.
        pOpStats - A pointer to the DRIVERIOOPSTATS structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayDiskIOStatsCategory(
    const char             *pszName,
    const DRIVERIOOPSTATS  *pOpStats)
{
    #define                 WEIGHTFACTOR    (50)
    char                    szBuff[16];
    char                    szBuff2[16];
    D_UINT32                ulTotalMS;
/*    
    D_UINT32                ulTempSize = 0;
    D_UINT32                ulTempSeq = 0;
    D_UINT32                ulWeight = 0;
*/  
    DclAssertReadPtr(pszName, 0);
    DclAssertReadPtr(pOpStats, sizeof(*pOpStats));
#if 0
    if(pOpStats->ulSectorIOCalls)
    {
        D_UINT64    ullTemp;
        
        ullTemp = DclMulDiv(pOpStats->ulSectorIOTotal, 10, pOpStats->ulSectorIOCalls);
        if(ullTemp > D_UINT32_MAX)
            ulTempSize = 0;
        else
            ulTempSize = (D_UINT32)ullTemp;

        ullTemp = DclMulDiv(pOpStats->ulSectorIOSeqTotal, 10, pOpStats->ulSectorIOCalls - pOpStats->ulSectorIOSeqCalls);
        if(ullTemp > D_UINT32_MAX)
            ulTempSeq = 0;
        else
            ulTempSeq = (D_UINT32)ullTemp;

        ullTemp = ulTempSize;
        DclUint64MulUint32(&ullTemp, WEIGHTFACTOR);
        ullTemp += ulTempSeq;
        DclUint64DivUint32(&ullTemp, WEIGHTFACTOR);
        ullTemp -= 10;

        ulWeight = (D_UINT32)ullTemp;    

        /*  A statistically near perfect score could go up to 101%, just
            put a 100% ceiling on it...
        */            
        if(ulWeight > 1000)
            ulWeight = 1000;
    }
#endif
    
/*  DclPrintf("  %-8s   %10lU %10lU %5s%6lU %6lU %7lU %3lU.%lU %3lU.%lU %3lU.%lU\n",  */

/*  DclPrintf("Disk I/O Stats*  [         Total         ][ Average ][    Max     ][ Sequential ]\n");
    DclPrintf("                  Calls    Sectors      MS Secs   MS  Secs      MS  Calls Sectors\n");
*/

    ulTotalMS = (D_UINT32)((pOpStats->ullTotalUS + 500) / 1000);

    if(DclStrICmp(pszName, "Flushes") != 0)
    {
        DclPrintf("  %-8s   %10lU %10lU %7lU %6s %5s %6lU %5lU %7lU %9lU\n",
        pszName,
            pOpStats->ulCalls,  
            pOpStats->ulTotalSectors,
            ulTotalMS,
            DclRatio(szBuff, sizeof(szBuff), pOpStats->ulTotalSectors, pOpStats->ulCalls, 1),
            DclRatio(szBuff2, sizeof(szBuff2), ulTotalMS, pOpStats->ulTotalSectors, 1),
            pOpStats->ulMaxSectors,
            (pOpStats->ulMaxAverageUS + 500) / 1000,
            pOpStats->ulSeqCalls, 
            pOpStats->ulSeqTotalSectors /*,
        ulTempSize / 10, ulTempSize % 10,
        ulTempSeq / 10, ulTempSeq % 10,
        ulWeight / 10, ulWeight % 10 */ );
    }
    else
    {
        DclPrintf("  %-8s   %10lU %10lU %7lU %6s %5s %6lU %5lU     n/a       n/a\n",
            pszName,
            pOpStats->ulCalls,  
            pOpStats->ulTotalSectors,
            ulTotalMS,
            DclRatio(szBuff, sizeof(szBuff), pOpStats->ulTotalSectors, pOpStats->ulCalls, 1),
            DclRatio(szBuff2, sizeof(szBuff2), ulTotalMS, pOpStats->ulTotalSectors, 1),
            pOpStats->ulMaxSectors,
            (pOpStats->ulMaxAverageUS + 500) / 1000);
    }

    return;
}



