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
  jurisdictions.  Patents may be pending.

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

    This module contains functions for display DCL system information.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstats.c $
    Revision 1.18  2010/07/31 19:47:13Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.17  2010/04/18 21:12:17Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.16  2009/11/04 19:13:00Z  garyp
    Updated to track statistics in microseconds rather than milliseconds.
    Revision 1.15  2009/06/28 00:49:11Z  garyp
    Minor format changes to some stats.
    Revision 1.14  2009/04/10 02:00:59Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.13  2007/12/18 03:52:00Z  brandont
    Updated function headers.
    Revision 1.12  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.11  2007/08/19 03:50:45Z  garyp
    Updated to use a segmented, three-part version number.
    Revision 1.10  2007/03/31 19:08:48Z  Garyp
    Added support for mutex tracking.
    Revision 1.9  2007/03/30 18:27:30Z  Garyp
    Added functions to display mutex and read/write semaphore statistics.
    Revision 1.8  2006/12/16 21:34:32Z  Garyp
    Added more compile time project information.
    Revision 1.7  2006/12/11 22:04:17Z  Garyp
    Updated to support DCL_OSFEATURE_CONSOLEINPUT.
    Revision 1.6  2006/12/08 04:00:40Z  Garyp
    Updated to work with the D_CPUTYPE value which is a string rather than
    a number.
    Revision 1.5  2006/10/01 18:13:28Z  Garyp
    Updated to track the maximum number of concurrent blocks allocated.
    Revision 1.4  2006/09/16 21:11:20Z  Garyp
    Modified to accommodate build numbers which are now strings rather than
    numbers.
    Revision 1.3  2006/08/18 19:37:59Z  Garyp
    Added support for the processor alignment boundary.
    Revision 1.2  2006/01/02 12:25:20Z  Garyp
    Cleaned up the interface and added functions for displaying memory and
    semaphore stats.
    Revision 1.1  2005/11/15 15:22:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlstats.h>

#define BUFFLEN (DCL_MAX_VERSION_LENGTH + 16)

/*-------------------------------------------------------------------
    Protected: DclSystemInfoDisplay()

    Display the system statistics contained in the supplied
    DCLSYSTEMINFO structure.

    Parameters:
        pDSS    - A pointer to the DCLSYSTEMINFO structure to use

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSystemInfoDisplay(
    const DCLSYSTEMINFO    *pDSS)
{
    char                    szBuff[BUFFLEN];
    DCLSTATUS               dclStat;

    DclAssertReadPtr(pDSS, sizeof(*pDSS));

    if(pDSS->nStrucLen != sizeof(*pDSS))
        return DCLSTAT_BADSTRUCLEN;

    DclPrintf("System Compile-Time Configuration Options:\n");

    dclStat = DclSystemVersionDisplay(pDSS);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    DclStrNCpy(szBuff, pDSS->szOSName, sizeof(szBuff));
    if(DclStrICmp(pDSS->szOSVer, "n/a") != 0)
    {
        size_t nLen = DclStrLen(szBuff);
        DclSNPrintf(&szBuff[nLen], sizeof(szBuff)-nLen, " %s", pDSS->szOSVer);
    }

    DclPrintf("  ToolSet Number                     %13u\n",   pDSS->nToolSetNum);
    DclPrintf("  Target Hardware                 %16s\n",      pDSS->szCPUType);
    DclPrintf("    Byte-Order                       %13s\n",   pDSS->fBigEndian             ? "Big-Endian" : "Little-Endian");
    DclPrintf("    Native 64-bit Support            %13s\n",   pDSS->fNative64BitSupport    ? "Yes" : "No");
    DclPrintf("    Processor Alignment Boundary     %13u\n",   pDSS->nProcessorAlignment);
    DclPrintf("  Target OS       %32s\n",                      szBuff);
    DclPrintf("    OS/ToolSet Alignment Boundary    %13u\n",   pDSS->nAlignment);
    DclPrintf("    Supports Unicode                 %13s\n",   pDSS->fOSFeatureUnicode      ? "Yes" : "No");
    DclPrintf("    Supports Threads                 %13s\n",   pDSS->fOSFeatureThreads      ? "Yes" : "No");
    DclPrintf("    Supports Console Input           %13s\n",   pDSS->fOSFeatureConsoleInput ? "Yes" : "No");
    DclPrintf("  Project Settings:\n");
    DclPrintf("    HighRes Timestamp                %13s\n",   pDSS->fHighResTimeStamp      ? "Enabled" : "Disabled");
    DclPrintf("    Output Enabled                   %13s\n",   pDSS->fOutputEnabled         ? "Yes" : "No");
    DclPrintf("    Debug Settings:\n");
    DclPrintf("      Debug Level                    %13u\n",   pDSS->nDebugLevel);
    DclPrintf("      DCL Trace Mask                    %lX\n", pDSS->ulDebugTraceMask);
    DclPrintf("      Trace Auto-Enable              %13s\n",   pDSS->fTraceAutoEnable       ? "Yes" : "No");
    DclPrintf("      Profiler Included              %13s\n",   pDSS->fProfiler              ? "Yes" : "No");
    DclPrintf("      Profiler Auto-Enable           %13s\n",   pDSS->fProfilerAutoEnable    ? "Yes" : "No");
    DclPrintf("      Memory Tracking                %13s\n",   pDSS->fMemoryTracking        ? "Enabled" : "Disabled");
    DclPrintf("      Mutex Tracking                 %13s\n",   pDSS->fMutexTracking         ? "Enabled" : "Disabled");
    DclPrintf("      Semaphore Tracking             %13s\n\n", pDSS->fSemaphoreTracking     ? "Enabled" : "Disabled");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclSystemVersionDisplay()

    Display the system version information contained in the supplied
    DCLSYSTEMINFO structure.

    Parameters:
        pDSS    - A pointer to the DCLSYSTEMINFO structure to use

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSystemVersionDisplay(
    const DCLSYSTEMINFO    *pDSS)
{
    char                    szBuff[BUFFLEN];
    char                    achVersionBuff[DCL_MAX_VERSION_LENGTH+1];

    DclAssertReadPtr(pDSS, sizeof(*pDSS));

    if(pDSS->nStrucLen != sizeof(*pDSS))
        return DCLSTAT_BADSTRUCLEN;

    DclVersionFormat(achVersionBuff, sizeof(achVersionBuff), pDSS->ulProductVer, pDSS->szProductBuild);

    DclSNPrintf(szBuff, BUFFLEN, "%s v%s", pDSS->szProductName, achVersionBuff);

    DclPrintf("  Product     %36s\n", szBuff);

    if(pDSS->nProductNum != pDSS->nSubProductNum)
    {
        DclAssert(pDSS->nSubProductNum == PRODUCTNUM_DCL);

        DclVersionFormat(achVersionBuff, sizeof(achVersionBuff), pDSS->ulSubProductVer, pDSS->szSubProductBuild);

        DclSNPrintf(szBuff, BUFFLEN, "DCL v%s", achVersionBuff);

        DclPrintf("  SubProduct            %26s\n", szBuff);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclMemStatsDisplay()

    Display the memory usage statistics contained in the supplied
    DCLMEMSTATS structure.

    Parameters:
        pDMS    - A pointer to the DCLMEMSTATS structure to use

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
 -------------------------------------------------------------------*/
DCLSTATUS DclMemStatsDisplay(
    DCLMEMSTATS    *pDMS)
{
    DclAssert(pDMS);

    if(pDMS->uStrucLen != sizeof(*pDMS))
        return DCLSTAT_BADSTRUCLEN;

    DclPrintf("System Memory Statistics\n");
    DclPrintf("  Current usage:\n");
    DclPrintf("    Total bytes currently allocated     %10lU\n",  pDMS->ulBytesCurrentlyAllocated);
    DclPrintf("    Total blocks currently allocated    %10lU\n",  pDMS->ulBlocksCurrentlyAllocated);
    DclPrintf("  Historical usage:\n");
    DclPrintf("    Maximum concurrent bytes allocated  %10lU*\n", pDMS->ulMaxConcurrentBytes);
    DclPrintf("    Maximum concurrent blocks allocated %10lU*\n", pDMS->ulMaxConcurrentBlocks);
    DclPrintf("    Total alloc calls                   %10lU*\n", pDMS->ulTotalAllocCalls);
    DclPrintf("    Largest allocation requested        %10lU*\n", pDMS->ulLargestAllocation);
    DclPrintf("  Memory tracker overhead per block     %10U\n",   pDMS->uOverhead);
    DclPrintf("  *Since last statistics reset\n\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclMutexStatsDisplay()

    Display mutex usage statistics.

    Parameters:
        pDMS     - A pointer to the DCLMUTEXSTATS structure.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclMutexStatsDisplay(
    DCLMUTEXSTATS  *pDMS)
{
    unsigned        kk;

    DclAssert(pDMS);

    if(pDMS->nStrucLen != sizeof(*pDMS))
        return DCLSTAT_BADSTRUCLEN;

    DclPrintf("Mutex statistics for %U objects:\n", pDMS->nCount);

    DclPrintf("                Current    Total    [----Time Waiting*----]\n");
    DclPrintf("   Mutex Name    Count   Acquires*  TotalMS   AvgUS   MaxUS\n");

    for(kk=0; kk<pDMS->nReturned; kk++)
    {
        char    achWorkBuff[32];
        D_UINT64    ullTemp = (pDMS->pDMI[kk].ullTotalWaitUS + 500) / 1000;

        DclPrintf("%16s %6u %10lU %8llU %7s %7lU\n",
            pDMS->pDMI[kk].szName,
            pDMS->pDMI[kk].nCurrentCount,
            pDMS->pDMI[kk].ulAcquireCount,
            VA64BUG(ullTemp),
            DclRatio64(achWorkBuff, sizeof(achWorkBuff), pDMS->pDMI[kk].ullTotalWaitUS, pDMS->pDMI[kk].ulAcquireCount, 1),
            pDMS->pDMI[kk].ulMaxWaitUS);
    }

    if(pDMS->nReturned)
        DclPrintf("  *Since last statistics reset\n");

    DclPrintf("\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclSemaphoreStatsDisplay()

    Display semaphore usage statistics.

    Parameters:
        pDSS     - A pointer to the DCLSEMAPHORESTATS structure.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSemaphoreStatsDisplay(
    DCLSEMAPHORESTATS  *pDSS)
{
    unsigned            kk;

    DclAssert(pDSS);

    if(pDSS->nStrucLen != sizeof(*pDSS))
        return DCLSTAT_BADSTRUCLEN;

    DclPrintf("Semaphore statistics for %U objects\n", pDSS->nCount);

    DclPrintf("                Initial Current  High     Total    [---Time Waiting*---]\n");
    DclPrintf(" Semaphore Name  Count   Count  Water*  Acquires*  TotalMS  AvgUS  MaxUS\n");

    for(kk=0; kk<pDSS->nReturned; kk++)
    {
        char    achWorkBuff[32];
        D_UINT64    ullTemp = (pDSS->pDSI[kk].ullTotalWaitUS + 500) / 1000;

        DclPrintf("%16s %5lU   %5lU  %5lU %10lU %9llU %6s %6lU\n",
            pDSS->pDSI[kk].szName,
            pDSS->pDSI[kk].ulMaxCount,
            pDSS->pDSI[kk].ulCurrentCount,
            pDSS->pDSI[kk].ulHighWater,
            pDSS->pDSI[kk].ulAcquireCount,
            VA64BUG(ullTemp),
            DclRatio64(achWorkBuff, sizeof(achWorkBuff), pDSS->pDSI[kk].ullTotalWaitUS, pDSS->pDSI[kk].ulAcquireCount, 1),
            pDSS->pDSI[kk].ulMaxWaitUS);
    }

    if(pDSS->nReturned)
        DclPrintf("  *Since last statistics reset\n");

    DclPrintf("\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: DclSemaphoreRdWrStatsDisplay()

    Display read/write semaphore usage statistics.

    Parameters:
        pRWS     - A pointer to the DCLRDWRSEMAPHORESTATS structure.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSemaphoreRdWrStatsDisplay(
    DCLRDWRSEMAPHORESTATS  *pRWS)
{
    unsigned                kk;

    DclAssert(pRWS);

    if(pRWS->nStrucLen != sizeof(*pRWS))
        return DCLSTAT_BADSTRUCLEN;

    DclPrintf("Read/Write semaphore statistics for %U objects\n", pRWS->nCount);

    DclPrintf("                                      [-------------Reading*---------------][-----------Writing*------------]\n");
    DclPrintf("   ReadWrite     Initial Current Wri- High     Total   [---Time Waiting----]    Total   [---Time Waiting----]\n");
    DclPrintf(" Semaphore Name   Count   Count  ting Water   Acquires TotalMS  AvgUS  MaxUS   Acquires TotalMS  AvgUS  MaxUS\n");

    for(kk=0; kk<pRWS->nReturned; kk++)
    {
        char    achWorkBuff[32];
        char    achWorkBuff2[32];
        D_UINT64    ullRead = (pRWS->pRWI[kk].ullReadTotalWaitUS + 500) / 1000;
        D_UINT64    ullWrite = (pRWS->pRWI[kk].ullWriteTotalWaitUS + 500) / 1000;

        DclPrintf("%16s %6lU %7lU   %1u %7lU %10lU %7llU %6s %6lU %10lU %7llU %6s %6lU\n",
            pRWS->pRWI[kk].szName,
            pRWS->pRWI[kk].ulMaxCount,
            pRWS->pRWI[kk].ulCurrentCount,
            pRWS->pRWI[kk].fWriting,
            pRWS->pRWI[kk].ulReadHighWater,
            pRWS->pRWI[kk].ulReadAcquireCount,
            VA64BUG(ullRead),
            DclRatio64(achWorkBuff, sizeof(achWorkBuff), pRWS->pRWI[kk].ullReadTotalWaitUS, pRWS->pRWI[kk].ulReadAcquireCount, 1),
            pRWS->pRWI[kk].ulReadMaxWaitUS,
            pRWS->pRWI[kk].ulWriteAcquireCount,
            VA64BUG(ullWrite),
            DclRatio64(achWorkBuff2, sizeof(achWorkBuff2), pRWS->pRWI[kk].ullWriteTotalWaitUS, pRWS->pRWI[kk].ulWriteAcquireCount, 1),
            pRWS->pRWI[kk].ulWriteMaxWaitUS);
    }

    if(pRWS->nReturned)
        DclPrintf("  *Since last statistics reset\n");

    DclPrintf("\n");

    return DCLSTAT_SUCCESS;
}
