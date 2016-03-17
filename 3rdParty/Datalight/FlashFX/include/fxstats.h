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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header file contains defines the interface to the "statistics"
    feature in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxstats.h $
    Revision 1.22  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.21  2010/01/23 21:44:10Z  garyp
    Enhanced the driver statistics.  Now support flush.
    Revision 1.20  2009/12/03 00:49:53Z  garyp
    Updated the driver I/O stats information.
    Revision 1.19  2009/08/06 21:20:40Z  garyp
    Added missing native I/O stats.
    Revision 1.18  2009/07/29 19:15:52Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes.  Removed some
    obsolete stats.  Added support for new statistics.  Added support for
    uncorrected page stats.  Added new compaction stats fields.
    Revision 1.17  2009/01/19 04:35:03Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.16  2008/12/24 06:02:54Z  keithg
    Added support new IOCTL support for raw erase operations.
    Revision 1.15  2008/12/19 05:29:50Z  keithg
    Added support for the BBM statistics API.
    Revision 1.14  2008/08/06 04:33:30Z  keithg
    Clarified comment on the BBM stats structure element.
    Revision 1.13  2008/05/08 01:10:36Z  garyp
    Updated to track read and write stats on a per-block basis.
    Revision 1.12  2007/12/13 20:46:47Z  Garyp
    Updated the FML and region stats structures to accommodate querying
    the erase block and region mount stats, respectively, in iterations.
    Revision 1.11  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/06/21 18:54:20Z  Garyp
    Refactored the PerfLog output stuff so that it is separate from the stats
    display stuff.
    Revision 1.9  2007/04/15 17:51:57Z  Garyp
    Added BBM stats support.
    Revision 1.8  2007/04/06 03:13:55Z  Garyp
    Modified the use of boolean types so we build cleanly in CE.
    Revision 1.7  2007/04/06 00:26:51Z  Garyp
    Added the ability to display the stats in PerfLog form.
    Revision 1.6  2006/11/21 18:43:35Z  billr
    More complete and accurate BBM statistics.
    Revision 1.5  2006/11/14 18:03:12Z  billr
    Add support for new parameter to get BBM information.
    Revision 1.4  2006/11/10 18:11:48Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.3  2006/08/21 22:36:04Z  Garyp
    Added wear-leveling statistics to the VBFCOMPSTATS structure.
    Revision 1.2  2006/06/19 23:57:10Z  Garyp
    Updated to handle region tracking.
    Revision 1.1  2006/05/08 10:38:12Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXSTATS_H_INCLUDED
#define FXSTATS_H_INCLUDED


/*-------------------------------------------------------------------
    DRIVERIOOPSTATS and DRIVERIOSTATS

    This structure holds Driver Framework level I/O statistics.
-------------------------------------------------------------------*/

typedef struct  /* Driver I/O Operation Stats */
{
    D_UINT64        ullTotalUS;         /* Total US doing I/O */
    D_UINT32        ulMaxAverageUS;     /* Slowest average US per sector */
    D_UINT32        ulCalls;            /* Total number of calls */
    D_UINT32        ulTotalSectors;     /* Absolute total sectors */
    D_UINT32        ulMaxSectors;       /* The most sectors in one call */
    D_UINT32        ulSeqCalls;         /* Total calls sequential with the previous call */
    D_UINT32        ulSeqTotalSectors;  /* Total sectors sequential with the previous call */
} DRIVERIOOPSTATS;    

typedef struct
{
    DRIVERIOOPSTATS Read;
    DRIVERIOOPSTATS Write;
    DRIVERIOOPSTATS Discard;
    DRIVERIOOPSTATS Flush;
    D_UINT32        ulFlushActualMax;   /* Max actual sectors flushed in any one call */
    D_UINT32        ulFlushActualTotal; /* Total actual sectors flushed */
} DRIVERIOSTATS;


/*-------------------------------------------------------------------
    FFXCOMPSTATS

    This structure holds asynchronous compaction tracking information
    and is enabled if FFXCONF_STATS_COMPACTION is TRUE.
-------------------------------------------------------------------*/
typedef struct
{
    char        szThreadName[DCL_THREADNAMELEN];
    D_UINT32    ulCompactionsInitiated;
    D_UINT32    ulTotalMS;
    D_UINT32    ulSlowestCompactMS;
    D_UINT32    ulSleepMS;
    D_UINT16    uThreadPriority;
 } FFXCOMPSTATS;


/*-------------------------------------------------------------------
    VBFCOMPSTATS

    This structure holds VBF compaction tracking information
    and is enabled if FFXCONF_STATS_COMPACTION is TRUE.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulIdleCompactionChecks;             /* FfxVbfCompactIfIdle() calls */
    D_UINT32    ulIdleCompactionsRequested;
    D_UINT32    ulIdleCompactionsCompleted;
    D_UINT32    ulIdleCompactionTime;               /* ms spent in idletime compactions */
    D_UINT32    ulIdleCompactedPages;               /* pages recovered */
    D_UINT32    ulExplicitCompactionsRequested;     /* FfxVbfCompact() calls */
    D_UINT32    ulExplicitCompactionsCompleted;
    D_UINT32    ulExplicitCompactionTime;           /* ms spent in explicit compactions */
    D_UINT32    ulExplicitCompactedPages;           /* pages recovered */
    D_UINT32    ulWearLevelingAttempts;             /* Number of attempts to wear-level */
    D_UINT32    ulWearLevelingTotalTime;            /* Total ms spent in wear-leveling code */
    D_UINT32    ulWearLevelingCompactions;          /* Collector() calls from wear-leveling code */
    D_UINT32    ulWearLevelingCompactionTime;       /* ms spent in Collector via wear-leveling */
    D_UINT32    ulWearLevelingCompactedPages;       /* pages recovered */
    D_UINT32    ulAggressiveCount;
    D_UINT32    ulSyncWriteCompactionsRequested;
    D_UINT32    ulSyncWriteCompactionsCompleted;
    D_UINT32    ulSyncWriteCompactionTime;          /* ms spent in sync write compactions */
    D_UINT32    ulSyncWriteCompactedPages;          /* pages recovered */
    D_UINT32    ulSyncDiscardCompactionsRequested;
    D_UINT32    ulSyncDiscardCompactionsCompleted;
    D_UINT32    ulSyncDiscardCompactionTime;        /* ms spent in sync discard compactions */
    D_UINT32    ulSyncDiscardCompactedPages;        /* pages recovered */
    D_UINT32    ulSyncDiscardCompactionRetries;
    D_UINT32    ulTotalCompactions;                 /* Absolute total compactions (all causes) */
    D_UINT32    ulTotalCompactedPages;              /* Absolute total pages recovered */
    D_UINT32    ulTotalUnitsReformatted;            /* Absolute total units reformatted */
} VBFCOMPSTATS;


/*-------------------------------------------------------------------
    VBFREGIONMOUNTDATA

    This structure contains mount data for an individual region.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulTotalMounts;
    D_UINT32    ulCacheHits;
    D_UINT32    ulConcurrentMounts;
} VBFREGIONMOUNTDATA;


/*-------------------------------------------------------------------
    VBFREGIONSTATS

    This structure contains region mount statistics for a given
    VBF disk.
-------------------------------------------------------------------*/
typedef struct
{
    unsigned            nRegionCacheSize;
    D_UINT32            ulMountRequests;
    D_UINT32            ulMountMS;          /* MS spent mounting non-cached regions */
    D_UINT32            ulCacheHits;
    D_UINT32            ulCompactionMounts;
    unsigned            nRegionCount;       /* Total VBF regions -- may be larger than uRegionsTracked */
    unsigned            nMountDataStart;    /* Starting entry to report */
    unsigned            nMountDataCount;    /* Count on entry indicating the bufer size, modified on exit to be the number of entries reported */
    VBFREGIONMOUNTDATA *pMountData;
} VBFREGIONSTATS;


/*-------------------------------------------------------------------
    FFXFMLSTATS

    This structure holds FML statistics.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32            ulReads;            /* # of page reads per block */
    D_UINT32            ulWrites;           /* # of page writes per block */
    D_UINT32            ulErases;           /* # of erases per block */
} FFXFMLBLOCKSTATS;

typedef struct
{
    D_UINT32            ulPageReadRequests;
    D_UINT32            ulPageReadTotal;
    D_UINT32            ulPageReadMax;
    D_UINT32            ulPageWriteRequests;
    D_UINT32            ulPageWriteTotal;
    D_UINT32            ulPageWriteMax;
    D_UINT32            ulBlockEraseRequests;
    D_UINT32            ulBlockEraseTotal;
    D_UINT32            ulBlockEraseMax;
    D_UINT32            ulBlockStatsStart;  /* Starting element in the pBlockStats array */
    D_UINT32            ulBlockStatsCount;  /* Entries in the pBlockStats array */
    FFXFMLBLOCKSTATS   *pBlockStats;        /* Per-block statistics */
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    D_UINT32            ulControlByteReadRequests;
    D_UINT32            ulControlByteReadTotal;
    D_UINT32            ulControlByteReadMax;
    D_UINT32            ulControlByteWriteRequests;
    D_UINT32            ulControlByteWriteTotal;
    D_UINT32            ulControlByteWriteMax;
  #endif
  #if FFXCONF_NANDSUPPORT
    D_UINT32            ulCorrectedPageReadRequests;
    D_UINT32            ulCorrectedPageReadTotal;
    D_UINT32            ulCorrectedPageReadMax;
    D_UINT32            ulUncorrectedPageReadRequests;
    D_UINT32            ulUncorrectedPageReadTotal;
    D_UINT32            ulUncorrectedPageReadMax;
    D_UINT32            ulUncorrectedPageWriteRequests;
    D_UINT32            ulUncorrectedPageWriteTotal;
    D_UINT32            ulUncorrectedPageWriteMax;
    D_UINT32            ulNativePageReadRequests;
    D_UINT32            ulNativePageReadTotal;
    D_UINT32            ulNativePageReadMax;
    D_UINT32            ulNativePageWriteRequests;
    D_UINT32            ulNativePageWriteTotal;
    D_UINT32            ulNativePageWriteMax;
    D_UINT32            ulTaggedPageReadRequests;
    D_UINT32            ulTaggedPageReadTotal;
    D_UINT32            ulTaggedPageReadMax;
    D_UINT32            ulTaggedPageWriteRequests;
    D_UINT32            ulTaggedPageWriteTotal;
    D_UINT32            ulTaggedPageWriteMax;
    D_UINT32            ulRawEraseRequests;
    D_UINT32            ulRawEraseTotal;
    D_UINT32            ulRawEraseMax;
    D_UINT32            ulRawPageReadRequests;
    D_UINT32            ulRawPageReadTotal;
    D_UINT32            ulRawPageReadMax;
    D_UINT32            ulRawPageWriteRequests;
    D_UINT32            ulRawPageWriteTotal;
    D_UINT32            ulRawPageWriteMax;
    D_UINT32            ulSpareAreaReadRequests;
    D_UINT32            ulSpareAreaReadTotal;
    D_UINT32            ulSpareAreaReadMax;
    D_UINT32            ulSpareAreaWriteRequests;
    D_UINT32            ulSpareAreaWriteTotal;
    D_UINT32            ulSpareAreaWriteMax;
    D_UINT32            ulTagReadRequests;
    D_UINT32            ulTagReadTotal;
    D_UINT32            ulTagReadMax;
    D_UINT32            ulTagWriteRequests;
    D_UINT32            ulTagWriteTotal;
    D_UINT32            ulTagWriteMax;
  #endif
} FFXFMLSTATS;


/*-------------------------------------------------------------------
    FFXBBMSTATS

    This structure holds BBM statistics returned by FfxBbmStatsQuery().
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32 ulBbmInstances;     /* # of instances used to manage the disk */
    D_UINT32 ulTotalBlocks;      /* Total # of erase blocks managed by BBM */
    D_UINT32 ulSpareBlocks;      /* # of blocks allocated for BBM */
    D_UINT32 ulBbmOverhead;      /* # of those blocks used internally by BBM */
    D_UINT32 ulFactoryBadBlocks; /* # of those blocks found to be bad */
    D_UINT32 ulRetiredBlocks;    /* # of those blocks retired */
    D_UINT32 ulTemporaryBlocks;  /* # of blocks mapped on a temporary basis */

    D_UINT32 ulMapCacheRequests; /* Total remap requests */
    D_UINT32 ulMapCacheHits;     /* Number of hits to the cache */
    D_UINT32 ulFreeBlocks;       /* # of those blocks still unused */

} FFXBBMSTATS;


/*-------------------------------------------------------------------
    Prototypes for the statistics interface.  Note that these
    functions are structured such that the "query" functions
    reside in libraries which are used when linking together a
    device driver, while these "display" functions are located
    so that they may be discretely linked in by anyone who needs
    to use them.
-------------------------------------------------------------------*/
void    FfxDriverDiskCompStatsDisplay(      const FFXCOMPSTATS          *pStat, D_BOOL fVerbose);
void    FfxDriverDiskIOStatsDisplay(        const DRIVERIOSTATS         *pStat, D_BOOL fVerbose);
void    FfxVbfCompactionStatsDisplay(       const VBFCOMPSTATS          *pCS,   D_BOOL fVerbose);
void    FfxVbfRegionStatsDisplay(           const VBFREGIONSTATS        *pRS,   D_BOOL fVerbose);
void    FfxFmlStatsDisplay(                 const FFXFMLSTATS           *pStat, D_BOOL fVerbose);
void    FfxBbmStatsDisplay(                 const FFXBBMSTATS           *pStat, D_BOOL fVerbose);




#endif


