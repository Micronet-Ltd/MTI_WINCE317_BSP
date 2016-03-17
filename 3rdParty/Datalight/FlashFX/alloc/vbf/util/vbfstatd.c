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

    This module contains VBF utility functions for displaying statistics.

    It resides in a different library (FXVBFUTL) so that it may be linked
    into code without pulling in all of VBF.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfstatd.c $
    Revision 1.16  2010/07/13 00:19:00Z  garyp
    Minor stats updates -- no functional changes (to non-stats stuff).
    Revision 1.15  2009/07/21 20:43:52Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Removed some
    obsolete stats.  Updated to accommodate new statistics.  Updated to 
    handle additional compaction stats.
    Revision 1.14  2009/03/31 16:44:04Z  davidh
    Function headers updated for AutoDoc
    Revision 1.13  2008/05/16 01:53:45Z  garyp
    Added a decimal point to the hit percentage display.
    Revision 1.12  2008/05/05 23:55:25Z  keithg
    Function header comment updates and additions.
    Revision 1.11  2008/01/13 07:20:43Z  keithg
    Function header updates to support autodoc.
    Revision 1.10  2007/12/13 03:55:02Z  Garyp
    Minor data type changes to the region stats information.
    Revision 1.9  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/06/21 19:11:01Z  Garyp
    Refactored the PerfLog output stuff so that it is separate from the stats
    display stuff.
    Revision 1.7  2007/04/06 03:13:56Z  Garyp
    Modified the use of boolean types so we build cleanly in CE.
    Revision 1.6  2007/04/06 02:06:51Z  Garyp
    Added the ability to display the stats in PerfLog form.
    Revision 1.5  2006/11/13 18:28:08Z  Garyp
    Minor message tweaks.
    Revision 1.4  2006/11/10 19:16:50Z  Garyp
    Standardized on a naming convention for the various "stats" interfaces.
    Modified the stats query to use the generic ParameterGet() ability now
    implemented through most all layers of the code.
    Revision 1.3  2006/08/20 00:10:01Z  Garyp
    Added wear-leveling compactions to the compactions stats.
    Revision 1.2  2006/06/19 23:57:09Z  Garyp
    Updated the VBF region tracking feature so it operates in the same
    fashion as the other statistics interfaces.
    Revision 1.1  2006/05/08 10:44:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxstats.h>


/*-------------------------------------------------------------------
    Protected: FfxVbfCompactionStatsDisplay()

    Display VBF level compaction statistics.  No error checking is
    performed on the given statistics structure.

    Parameters:
        pCS              - A pointer to the VBFCOMPSTATS structure
        fVerbose         - TRUE to display verbose statistics

    Return Value:
        None.
 -------------------------------------------------------------------*/
void FfxVbfCompactionStatsDisplay(
    const VBFCOMPSTATS *pCS,
    D_BOOL              fVerbose)
{
    D_UINT32            ulAsyncPercent = 0;
    D_UINT32            ulTotalSync;
    char                szBuffer[16];

    DclAssert(pCS);

    (void)fVerbose;

    ulTotalSync = pCS->ulSyncWriteCompactionsCompleted +
                  pCS->ulSyncDiscardCompactionsCompleted;

    if(pCS->ulExplicitCompactionsCompleted + ulTotalSync)
    {
        ulAsyncPercent = (pCS->ulExplicitCompactionsCompleted * 100) /
            (pCS->ulExplicitCompactionsCompleted + ulTotalSync);
    }

    DclPrintf("VBF Compaction Statistics\n");
    DclPrintf("  Asynchronous (%lU%%)\n", ulAsyncPercent);
    DclPrintf("    Idletime compaction checks          %10lU\n",    pCS->ulIdleCompactionChecks);
    DclPrintf("      Idletime compactions requested    %10lU\n",    pCS->ulIdleCompactionsRequested);
    DclPrintf("      Idletime compactions completed    %10lU\n",    pCS->ulIdleCompactionsCompleted);
    DclPrintf("      Idletime total/average time  %10lU/%4s ms\n",  pCS->ulIdleCompactionTime, DclRatio(szBuffer, sizeof(szBuffer), pCS->ulIdleCompactionTime, pCS->ulIdleCompactionsCompleted, 1));
    DclPrintf("      Idletime total pages recovered    %10lU\n",    pCS->ulIdleCompactedPages);
    DclPrintf("      Idletime average pages per compaction %6s\n",  DclRatio(szBuffer, sizeof(szBuffer), pCS->ulIdleCompactedPages, pCS->ulIdleCompactionsCompleted, 1));
    DclPrintf("    Explicit compactions requested      %10lU\n",    pCS->ulExplicitCompactionsRequested);
    DclPrintf("    Explicit compactions completed      %10lU\n",    pCS->ulExplicitCompactionsCompleted);
    DclPrintf("    Explicit total/average time    %10lU/%4s ms\n",  pCS->ulExplicitCompactionTime, DclRatio(szBuffer, sizeof(szBuffer),pCS->ulExplicitCompactionTime, pCS->ulExplicitCompactionsCompleted, 1));
    DclPrintf("    Explicit total pages recovered      %10lU\n",    pCS->ulExplicitCompactedPages);
    DclPrintf("    Explicit average pages per compaction   %6s\n",  DclRatio(szBuffer, sizeof(szBuffer), pCS->ulExplicitCompactedPages, pCS->ulExplicitCompactionsCompleted, 1));
    DclPrintf("      Aggressive                        %10lU\n",    pCS->ulAggressiveCount);
    DclPrintf("  Synchronous (%lU%%)\n", 100-ulAsyncPercent);
    DclPrintf("    Write compactions requested         %10lU\n",    pCS->ulSyncWriteCompactionsRequested);
    DclPrintf("    Write compactions completed         %10lU\n",    pCS->ulSyncWriteCompactionsCompleted);
    DclPrintf("    Write total/average time       %10lU/%4s ms\n",  pCS->ulSyncWriteCompactionTime, DclRatio(szBuffer, sizeof(szBuffer), pCS->ulSyncWriteCompactionTime, pCS->ulSyncWriteCompactionsCompleted, 1));
    DclPrintf("    Write total pages recovered         %10lU\n",    pCS->ulSyncWriteCompactedPages);
    DclPrintf("    Write average pages per compaction       %5s\n", DclRatio(szBuffer, sizeof(szBuffer), pCS->ulSyncWriteCompactedPages, pCS->ulSyncWriteCompactionsCompleted, 1));
    DclPrintf("    Discard compactions requested       %10lU\n",    pCS->ulSyncDiscardCompactionsRequested);
    DclPrintf("    Discard compactions completed       %10lU\n",    pCS->ulSyncDiscardCompactionsCompleted);
    DclPrintf("    Discard total/average time     %10lU/%4s ms\n",  pCS->ulSyncDiscardCompactionTime, DclRatio(szBuffer, sizeof(szBuffer), pCS->ulSyncDiscardCompactionTime, pCS->ulSyncDiscardCompactionsCompleted, 1));
    DclPrintf("    Discard total pages recovered       %10lU\n",    pCS->ulSyncDiscardCompactedPages);
    DclPrintf("    Discard average pages per compaction     %5s\n", DclRatio(szBuffer, sizeof(szBuffer), pCS->ulSyncDiscardCompactedPages, pCS->ulSyncDiscardCompactionsCompleted, 1));
    DclPrintf("    Discard compaction retries          %10lU\n",    pCS->ulSyncDiscardCompactionRetries);
    DclPrintf("  Wear-Leveling Statistics\n");
    DclPrintf("    Requests                            %10lU\n",    pCS->ulWearLevelingAttempts);
    DclPrintf("    Total/average time per request %10lU/%4s ms\n",  pCS->ulWearLevelingTotalTime, DclRatio(szBuffer, sizeof(szBuffer), pCS->ulWearLevelingTotalTime, pCS->ulWearLevelingAttempts, 1));
    DclPrintf("    Compactions initiated               %10lU\n",    pCS->ulWearLevelingCompactions);
    DclPrintf("    Total/average time per compaction%8lU/%4s ms\n", pCS->ulWearLevelingCompactionTime, DclRatio(szBuffer, sizeof(szBuffer), pCS->ulWearLevelingCompactionTime, pCS->ulWearLevelingCompactions, 1));
    DclPrintf("    Average pages per compaction             %5s\n", DclRatio(szBuffer, sizeof(szBuffer), pCS->ulWearLevelingCompactedPages, pCS->ulWearLevelingCompactions, 1));
    DclPrintf("  Absolute Totals\n");
    DclPrintf("    Total compactions                   %10lU\n",    pCS->ulTotalCompactions);
    DclPrintf("    Total pages compacted               %10lU\n",    pCS->ulTotalCompactedPages);
    DclPrintf("    Average pages compacted             %10s\n",     DclRatio(szBuffer, sizeof(szBuffer), pCS->ulTotalCompactedPages, pCS->ulTotalCompactions, 1));
    DclPrintf("    Total units reformatted             %10lU\n\n",  pCS->ulTotalUnitsReformatted);

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxVbfRegionStatsDisplay()

    Display VBF region statistics.  No error checking is performed
    on the given statistics structure.

    Parameters:
        pRS              - A pointer to the VBFREGIONSTATS structure
        fVerbose         - Display verbose statistics

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxVbfRegionStatsDisplay(
    const VBFREGIONSTATS   *pRS,
    D_BOOL                  fVerbose)
{
    D_UINT32                ulAverageMS = 0;
    D_UINT32                ulMaxMounts = 0;
    D_UINT32                ulMaxConcur = 0;
    char                    szHitPercent[8];

    DclAssert(pRS);

    DclRatio(szHitPercent, sizeof(szHitPercent), pRS->ulCacheHits * 100,
        pRS->ulMountRequests, 1);

    if(pRS->ulMountRequests - pRS->ulCacheHits)
        ulAverageMS = pRS->ulMountMS / (pRS->ulMountRequests - pRS->ulCacheHits);

    DclPrintf("VBF Region Statistics\n");

    DclPrintf("  Total regions in DISKn                %10u\n",     pRS->nRegionCount);
    DclPrintf("  Region cache size                     %10u\n",     pRS->nRegionCacheSize);
    DclPrintf("  Total region mount requests           %10lU*\n",   pRS->ulMountRequests);
    DclPrintf("  Cache hits                            %10lU*\n",   pRS->ulCacheHits);
    DclPrintf("  Hit percentage                        %10s%%*\n",  szHitPercent);
    DclPrintf("  Compaction driven new mounts          %10lU*\n",   pRS->ulCompactionMounts);
    DclPrintf("  Total mount time (non-cached)         %10lU ms*\n",pRS->ulMountMS);
    DclPrintf("  Average mount time (non-cached)       %10lU ms*\n",ulAverageMS);

    if(fVerbose && pRS->nMountDataCount && pRS->pMountData)
    {
        D_UINT16    r;

        DclPrintf("  Region Mount Data:\n");
        DclPrintf("    Reg#: Mounts Concur   Hits --- Reg#: Mounts Concur   Hits*\n");

        for(r=0; r<pRS->nMountDataCount/2; r++)
        {
            D_UINT16 k = r + (pRS->nMountDataCount/2);

            DclPrintf("    %4U: %6lU %6lU %6lU --- %4U: %6lU %6lU %6lU\n",
                r, pRS->pMountData[r].ulTotalMounts, pRS->pMountData[r].ulConcurrentMounts, pRS->pMountData[r].ulCacheHits,
                k, pRS->pMountData[k].ulTotalMounts, pRS->pMountData[k].ulConcurrentMounts, pRS->pMountData[k].ulCacheHits);

            ulMaxMounts = DCLMAX(ulMaxMounts, pRS->pMountData[r].ulTotalMounts);
            ulMaxMounts = DCLMAX(ulMaxMounts, pRS->pMountData[k].ulTotalMounts);
            ulMaxConcur = DCLMAX(ulMaxConcur, pRS->pMountData[r].ulConcurrentMounts);
            ulMaxConcur = DCLMAX(ulMaxConcur, pRS->pMountData[k].ulConcurrentMounts);
        }

        /*  Display the last one if there are an odd number of regions
        */
        if(pRS->nMountDataCount & 1)
        {
            r = pRS->nMountDataCount - 1;

            DclPrintf("                               --- %4U: %6lU %6lU %6lU\n",
                r, pRS->pMountData[r].ulTotalMounts, pRS->pMountData[r].ulConcurrentMounts, pRS->pMountData[r].ulCacheHits);

            ulMaxMounts = DCLMAX(ulMaxMounts, pRS->pMountData[r].ulTotalMounts);
            ulMaxConcur = DCLMAX(ulMaxConcur, pRS->pMountData[r].ulConcurrentMounts);
        }

        DclPrintf("  Max mounts for any one region         %10lU*\n", ulMaxMounts);
        DclPrintf("  Max concurrent mounts for any region  %10lU*\n", ulMaxConcur);
    }
    else
    {
        if(!fVerbose)
            DclPrintf("  Region Mount Data                        Skipped\n");
    }

    DclPrintf("\n");

    return;
}



