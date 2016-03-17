/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains routines to manipulate FlashFX PerfLog data.

    Note that this module explicitly resides in a location where it is built
    into a separate library such that it may be linked into an application
    completely separate from the device driver code.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxperflog.c $
    Revision 1.12  2010/01/23 21:44:10Z  garyp
    Enhanced the driver statistics.  Now support flush.
    Revision 1.11  2009/12/02 21:47:29Z  garyp
    Added more driver I/O stats for tracking sequentialness.
    Revision 1.10  2009/07/30 02:19:14Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.9  2009/07/22 00:14:23Z  garyp
    Merged from the v4.0 branch.  Added a variety of new stats.
    Revision 1.8  2009/04/01 15:42:37Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.7  2009/01/19 04:34:40Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.6  2008/12/19 05:42:18Z  keithg
    Conditioned statistics per BBM support.
    Revision 1.5  2008/04/14 20:43:43Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.4  2008/01/13 07:27:14Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/12/13 03:53:49Z  Garyp
    Minor data type changes.
    Revision 1.2  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/06/22 17:51:28Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if DCLCONF_OUTPUT_ENABLED

#include <fxstats.h>
#include <fxperflog.h>
#include <dlperflog.h>


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteDiskIOStats()

    This function write the FlashFX Driver Framework level
    disk I/O statistics in the PerfLog format.

    Parameters:
        pStat        - The DRIVERIOSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteDiskIOStats(
    const DRIVERIOSTATS    *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatDiskIOStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        return FFXSTAT_SUCCESS;
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteDiskCompactionStats()

    This function write the FlashFX Driver Framework level
    compaction statistics in the PerfLog format.

    Parameters:
        pStat        - The FFXCOMPSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteDiskCompactionStats(
    const FFXCOMPSTATS     *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatDiskCompactionStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        return FFXSTAT_SUCCESS;
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteVbfCompactionStats()

    This function write the FlashFX VBF level compaction
    statistics in the PerfLog format.

    Parameters:
        pStat        - The VBFCOMPSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteVbfCompactionStats(
    const VBFCOMPSTATS     *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatVbfCompactionStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        return FFXSTAT_SUCCESS;
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteVbfRegionStats()

    This function write the FlashFX VBF level region statistics
    in the PerfLog format.

    Parameters:
        pStat        - The VBFREGIONSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteVbfRegionStats(
    const VBFREGIONSTATS   *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatVbfRegionStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        return FFXSTAT_SUCCESS;
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteFmlIOStats()

    This function write the FlashFX FML level I/O statistics
    in the PerfLog format.

    Parameters:
        pStat        - The FFXFMLSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteFmlIOStats(
    const FFXFMLSTATS      *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatFmlIOStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        hPerfLog = FfxPerfLogFormatFmlIONandStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
        if(hPerfLog)
        {
            DclPerfLogWrite(hPerfLog);
            DclPerfLogClose(hPerfLog);

            return FFXSTAT_SUCCESS;
        }
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogWriteBbmStats()

    This function write the FlashFX BBM statistics in the
    PerfLog format.

    Parameters:
        pStat        - The FFXBBMSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxPerfLogWriteBbmStats(
    const FFXBBMSTATS      *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    hPerfLog = FfxPerfLogFormatBbmStats(pStat, hLog, pszCatSuffix, pszBldSuffix);
    if(hPerfLog)
    {
        DclPerfLogWrite(hPerfLog);
        DclPerfLogClose(hPerfLog);

        return FFXSTAT_SUCCESS;
    }

    return FFXSTAT_PERFLOG_OPENFAILED;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatDiskIOStats()

    This function creates a PerfLog instance and records the
    FlashFX Driver Framework level disk I/O statistics.  If
    this function returns a valid handle, the log must be
    explicitly closed with the DCLPERFLOG_CLOSE() macro.

    Parameters:
        pStat        - The DRIVERIOSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatDiskIOStats(
    const DRIVERIOSTATS    *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    DclAssert(pStat);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "DrvIO", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(hPerfLog, "RCls",            pStat->Read.ulCalls);
    DCLPERFLOG_NUM(hPerfLog, "RSec",            pStat->Read.ulTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "RMax",            pStat->Read.ulMaxSectors);
    DCLPERFLOG_NUM(hPerfLog, "RMS", (D_UINT32)((pStat->Read.ullTotalUS+500) / 1000));
    DCLPERFLOG_NUM(hPerfLog, "RMaxUS",          pStat->Read.ulMaxAverageUS);
    DCLPERFLOG_NUM(hPerfLog, "RSqCls",          pStat->Read.ulSeqCalls);
    DCLPERFLOG_NUM(hPerfLog, "RSqSec",          pStat->Read.ulSeqTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "WCls",            pStat->Write.ulCalls);
    DCLPERFLOG_NUM(hPerfLog, "WSec",            pStat->Write.ulTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "WMax",            pStat->Write.ulMaxSectors);
    DCLPERFLOG_NUM(hPerfLog, "WMS", (D_UINT32)((pStat->Write.ullTotalUS+500) / 1000));
    DCLPERFLOG_NUM(hPerfLog, "WMaxUS",          pStat->Write.ulMaxAverageUS);
    DCLPERFLOG_NUM(hPerfLog, "WSqCls",          pStat->Write.ulSeqCalls);
    DCLPERFLOG_NUM(hPerfLog, "WSqSec",          pStat->Write.ulSeqTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "DCls",            pStat->Discard.ulCalls);
    DCLPERFLOG_NUM(hPerfLog, "DSec",            pStat->Discard.ulTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "DMax",            pStat->Discard.ulMaxSectors);
    DCLPERFLOG_NUM(hPerfLog, "DMS", (D_UINT32)((pStat->Discard.ullTotalUS+500) / 1000));
    DCLPERFLOG_NUM(hPerfLog, "DMaxUS",          pStat->Discard.ulMaxAverageUS);
    DCLPERFLOG_NUM(hPerfLog, "DSqCls",          pStat->Discard.ulSeqCalls);
    DCLPERFLOG_NUM(hPerfLog, "DSqSec",          pStat->Discard.ulSeqTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "FCls",            pStat->Flush.ulCalls);
    DCLPERFLOG_NUM(hPerfLog, "FRSec",           pStat->Flush.ulTotalSectors);
    DCLPERFLOG_NUM(hPerfLog, "FRMax",           pStat->Flush.ulMaxSectors);
    DCLPERFLOG_NUM(hPerfLog, "FMS", (D_UINT32)((pStat->Flush.ullTotalUS+500) / 1000));
    DCLPERFLOG_NUM(hPerfLog, "FMaxUS",          pStat->Flush.ulMaxAverageUS);
    DCLPERFLOG_NUM(hPerfLog, "FAMax",           pStat->ulFlushActualMax);
    DCLPERFLOG_NUM(hPerfLog, "FATot",           pStat->ulFlushActualTotal);

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatDiskCompactionStats()

    This function creates a PerfLog instance and records the
    FlashFX Driver Framework level compaction statistics.  If
    this function returns a valid handle, the log must be
    explicitly closed with the DCLPERFLOG_CLOSE() macro.

    Parameters:
        pStat        - The FFXCOMPSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatDiskCompactionStats(
    const FFXCOMPSTATS     *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog = NULL;

    DclAssert(pStat);

    (void)pStat;

#if FFXCONF_STATS_COMPACTION

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "DrvComp", pszCatSuffix, pszBldSuffix);
  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    DCLPERFLOG_STRING(hPerfLog, "CompactionModel", "BackgroundThread");
    DCLPERFLOG_NUM(   hPerfLog, "IdleSleepTime",   pStat->ulSleepMS);
    DCLPERFLOG_NUM(   hPerfLog, "ThreadPriority",  pStat->uThreadPriority);
    DCLPERFLOG_NUM(   hPerfLog, "Compactions",     pStat->ulCompactionsInitiated);
    DCLPERFLOG_NUM(   hPerfLog, "TotalTime",       pStat->ulTotalMS);
    DCLPERFLOG_NUM(   hPerfLog, "Slowest",         pStat->ulSlowestCompactMS);
  #else
  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDIDLE
    DCLPERFLOG_STRING(hPerfLog, "CompactionModel", "BackgroundIdle");
  #else
    DCLPERFLOG_STRING(hPerfLog, "CompactionModel", "Synchronous");
  #endif
    DCLPERFLOG_NUM(   hPerfLog, "IdleSleepTime",   D_UINT32_MAX);
    DCLPERFLOG_NUM(   hPerfLog, "ThreadPriority",  D_UINT32_MAX);
    DCLPERFLOG_NUM(   hPerfLog, "Compactions",     D_UINT32_MAX);
    DCLPERFLOG_NUM(   hPerfLog, "TotalTime",       D_UINT32_MAX);
    DCLPERFLOG_NUM(   hPerfLog, "Slowest",         D_UINT32_MAX);
  #endif

#else

    (void)hLog;
    (void)pszCatSuffix;
    (void)pszBldSuffix;

#endif

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatVbfCompactionStats()

    This function creates a PerfLog instance and records the
    FlashFX VBF level compaction statistics.  If this function
    returns a valid handle, the log must be explicitly closed
    with the DCLPERFLOG_CLOSE() macro.

    Parameters:
        pCS          - A pointer to the VBFCOMPSTATS structure
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatVbfCompactionStats(
    const VBFCOMPSTATS     *pCS,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    DclAssert(pCS);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "VbfComp", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(  hPerfLog, "AIdChk",   pCS->ulIdleCompactionChecks);
    DCLPERFLOG_NUM(  hPerfLog, "AIdRq",    pCS->ulIdleCompactionsRequested);
    DCLPERFLOG_NUM(  hPerfLog, "AIdCp",    pCS->ulIdleCompactionsCompleted);
    DCLPERFLOG_NUM(  hPerfLog, "AIdMS",    pCS->ulIdleCompactionTime);
    DCLPERFLOG_NUM(  hPerfLog, "AIdPg",    pCS->ulIdleCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "AExRq",    pCS->ulExplicitCompactionsRequested);
    DCLPERFLOG_NUM(  hPerfLog, "AExCp",    pCS->ulExplicitCompactionsCompleted);
    DCLPERFLOG_NUM(  hPerfLog, "AExMS",    pCS->ulExplicitCompactionTime);
    DCLPERFLOG_NUM(  hPerfLog, "AExPg",    pCS->ulExplicitCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "AExAg",    pCS->ulAggressiveCount);
    DCLPERFLOG_NUM(  hPerfLog, "SWrRq",    pCS->ulSyncWriteCompactionsRequested);
    DCLPERFLOG_NUM(  hPerfLog, "SWrCp",    pCS->ulSyncWriteCompactionsCompleted);
    DCLPERFLOG_NUM(  hPerfLog, "SWrMS",    pCS->ulSyncWriteCompactionTime);
    DCLPERFLOG_NUM(  hPerfLog, "SWrPg",    pCS->ulSyncWriteCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "SDsRq",    pCS->ulSyncDiscardCompactionsRequested);
    DCLPERFLOG_NUM(  hPerfLog, "SDsCp",    pCS->ulSyncDiscardCompactionsCompleted);
    DCLPERFLOG_NUM(  hPerfLog, "SDsMS",    pCS->ulSyncDiscardCompactionTime);
    DCLPERFLOG_NUM(  hPerfLog, "SDsPg",    pCS->ulSyncDiscardCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "SDsRe",    pCS->ulSyncDiscardCompactionRetries);
    DCLPERFLOG_NUM(  hPerfLog, "WLvRq",    pCS->ulWearLevelingAttempts);
    DCLPERFLOG_NUM(  hPerfLog, "WLvMS",    pCS->ulWearLevelingTotalTime);
    DCLPERFLOG_NUM(  hPerfLog, "WLvCp",    pCS->ulWearLevelingCompactions);
    DCLPERFLOG_NUM(  hPerfLog, "WLvPg",    pCS->ulWearLevelingCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "WLvCMS",   pCS->ulWearLevelingCompactionTime);
    DCLPERFLOG_NUM(  hPerfLog, "TComp",    pCS->ulTotalCompactions);
    DCLPERFLOG_NUM(  hPerfLog, "TPage",    pCS->ulTotalCompactedPages);
    DCLPERFLOG_NUM(  hPerfLog, "TUnRe",    pCS->ulTotalUnitsReformatted);

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatVbfRegionStats()

    This function creates a PerfLog instance and records the
    FlashFX VBF region statistics.  If this function returns a
    valid handle, the log must be explicitly closed with the
    DCLPERFLOG_CLOSE() macro.

    Parameters:
        pRS          - A pointer to the VBFREGIONSTATS structure
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatVbfRegionStats(
    const VBFREGIONSTATS   *pRS,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    DclAssert(pRS);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "Region", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(  hPerfLog, "TotalRegions",      pRS->nRegionCount);
    DCLPERFLOG_NUM(  hPerfLog, "RegionCacheSize",   pRS->nRegionCacheSize);
    DCLPERFLOG_NUM(  hPerfLog, "MountRequests",     pRS->ulMountRequests);
    DCLPERFLOG_NUM(  hPerfLog, "CacheHits",         pRS->ulCacheHits);
    DCLPERFLOG_NUM(  hPerfLog, "TotalMountTime",    pRS->ulMountMS);

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatFmlIOStats()

    This function creates a PerfLog instance and records the
    FlashFX FML level I/O statistics.  If this function returns
    a valid handle, the log must be explicitly closed with the
    DCLPERFLOG_CLOSE() macro.

    Parameters:
        pStat        - The FFXFMLSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatFmlIOStats(
    const FFXFMLSTATS      *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    DclAssert(pStat);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "FMLIO", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(hPerfLog, "RdPgRq",      pStat->ulPageReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RdPgCnt",     pStat->ulPageReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RdPgMax",     pStat->ulPageReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WrPgRq",      pStat->ulPageWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WrPgCnt",     pStat->ulPageWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WrPgMax",     pStat->ulPageWriteMax);
    DCLPERFLOG_NUM(hPerfLog, "EraseBlkRq",  pStat->ulBlockEraseRequests);
    DCLPERFLOG_NUM(hPerfLog, "EraseBlkCnt", pStat->ulBlockEraseTotal);
    DCLPERFLOG_NUM(hPerfLog, "EraseBlkMax", pStat->ulBlockEraseMax);
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlRq",    pStat->ulControlByteReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlCnt",   pStat->ulControlByteReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlMax",   pStat->ulControlByteReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlRq",    pStat->ulControlByteWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlCnt",   pStat->ulControlByteWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlMax",   pStat->ulControlByteWriteMax);
  #else
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlRq",    D_UINT32_MAX);
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlCnt",   D_UINT32_MAX);
    DCLPERFLOG_NUM(hPerfLog, "RdCtrlMax",   D_UINT32_MAX);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlRq",    D_UINT32_MAX);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlCnt",   D_UINT32_MAX);
    DCLPERFLOG_NUM(hPerfLog, "WrCtrlMax",   D_UINT32_MAX);
  #endif

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatFmlIONandStats()

    This function creates a PerfLog instance and records the
    FlashFX FML level disk NAND I/O statistics.  If this
    function returns a valid handle, the log must be
    explicitly closed with the DCLPERFLOG_CLOSE() macro.

    Parameters:
        pStat        - The FFXFMLSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatFmlIONandStats(
    const FFXFMLSTATS      *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog = NULL;

    DclAssert(pStat);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/

  #if FFXCONF_NANDSUPPORT
    /*  We're recording the NAND FML stats in a separate category simply
        to keep the line length in the text files manageable.  The data
        will (likely) be merged in the spreadsheet.
    */
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "FMLNANDIO", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(hPerfLog, "RUnPRq",     pStat->ulUncorrectedPageReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RUnPN",      pStat->ulUncorrectedPageReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RUnPMx",     pStat->ulUncorrectedPageReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WUnPRq",     pStat->ulUncorrectedPageWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WUnPN",      pStat->ulUncorrectedPageWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WUnPMx",     pStat->ulUncorrectedPageWriteMax);
    DCLPERFLOG_NUM(hPerfLog, "RTgPRq",     pStat->ulTaggedPageReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RTgPN",      pStat->ulTaggedPageReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RTgPMx",     pStat->ulTaggedPageReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WTgPRq",     pStat->ulTaggedPageWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WTgPN",      pStat->ulTaggedPageWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WTgPMx",     pStat->ulTaggedPageWriteMax);
    DCLPERFLOG_NUM(hPerfLog, "RRwPRq",     pStat->ulRawPageReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RRwPN",      pStat->ulRawPageReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RRwPMx",     pStat->ulRawPageReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WRwPRq",     pStat->ulRawPageWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WRwPN",      pStat->ulRawPageWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WRwPMx",     pStat->ulRawPageWriteMax);
    DCLPERFLOG_NUM(hPerfLog, "RSpRq",      pStat->ulSpareAreaReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RSpN",       pStat->ulSpareAreaReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RSpMx",      pStat->ulSpareAreaReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WSpRq",      pStat->ulSpareAreaWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WSpN",       pStat->ulSpareAreaWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WSpMx",      pStat->ulSpareAreaWriteMax);
    DCLPERFLOG_NUM(hPerfLog, "RTgRq",      pStat->ulTagReadRequests);
    DCLPERFLOG_NUM(hPerfLog, "RTgN",       pStat->ulTagReadTotal);
    DCLPERFLOG_NUM(hPerfLog, "RTgMx",      pStat->ulTagReadMax);
    DCLPERFLOG_NUM(hPerfLog, "WTgRq",      pStat->ulTagWriteRequests);
    DCLPERFLOG_NUM(hPerfLog, "WTgN",       pStat->ulTagWriteTotal);
    DCLPERFLOG_NUM(hPerfLog, "WTgMx",      pStat->ulTagWriteMax);

  #else

    (void)pStat;
    (void)hLog;
    (void)pszCatSuffix;
    (void)pszBldSuffix;

  #endif

    return hPerfLog;
}


/*-------------------------------------------------------------------
    Protected: FfxPerfLogFormatBbmStats()

    This function creates a PerfLog instance and records the
    FlashFX BBM statistics.  If this function returns a valid
    handle, the log must be explicitly closed with the
    DCLPERFLOG_CLOSE() macro.

    Parameters:
        pStat        - The FFXBBMSTATS structure to use
        hLog         - The optional log handle for PerfLog output
                       (NULL to use the default device).
        pszCatSuffix - The optional PerfLog category suffix
        pszBldSuffix - The optional PerfLog build number suffix

    Return Value:
        Returns the PERFLOG handle if successful, or NULL otherwise.
-------------------------------------------------------------------*/
DCLPERFLOGHANDLE FfxPerfLogFormatBbmStats(
    const FFXBBMSTATS      *pStat,
    DCLLOGHANDLE            hLog,
    const char             *pszCatSuffix,
    const char             *pszBldSuffix)
{
    DCLPERFLOGHANDLE        hPerfLog;

    DclAssert(pStat);

    /*---------------------------------------------------------
        Write PerfLog records.  Note that any changes to the
        test name or category must be accompanied by changes
        to perffxstats.bat.  Any changes to the actual data
        fields recorded here requires changes to the various
        spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(TRUE, hLog, "FXSTATS", "BBM", pszCatSuffix, pszBldSuffix);
    DCLPERFLOG_NUM(   hPerfLog, "TotalDevBlocks",       pStat->ulTotalBlocks);
    DCLPERFLOG_NUM(   hPerfLog, "BBMReservedBlocks",    pStat->ulSpareBlocks);
    DCLPERFLOG_NUM(   hPerfLog, "OverheadBlocks",       pStat->ulBbmOverhead);
    DCLPERFLOG_NUM(   hPerfLog, "FactoryBadBlocks",     pStat->ulFactoryBadBlocks);
    DCLPERFLOG_NUM(   hPerfLog, "FreeBlocks",           pStat->ulFreeBlocks);
    DCLPERFLOG_NUM(   hPerfLog, "RetiredBlocks",        pStat->ulRetiredBlocks);
    DCLPERFLOG_NUM(   hPerfLog, "TemporaryBlocks",      pStat->ulTemporaryBlocks);

    return hPerfLog;
}


#endif  /* DCLCONF_OUTPUT_ENABLED */

