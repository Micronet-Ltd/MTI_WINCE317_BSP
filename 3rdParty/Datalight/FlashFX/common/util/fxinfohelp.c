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

    This module implements the "FXINFO" functionality, which reports driver-
    compile time settings, statistics, device, and disk information.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxinfohelp.c $
    Revision 1.12  2011/10/11 18:59:10Z  billr
    Make sure not to overrun acBlockMap[] in the VBFUNITMETRICS structure.
    Revision 1.11  2011/06/27 05:50:16Z  johnb
    No functional changes.
    Revision 1.10  2011/06/22 17:46:58Z  johnb
    Updated after review to use Datalight specific
    extension of %lU.
    Revision 1.9  2011/06/22 13:04:02Z  johnb
    Corrected display issue with 40 bit TI 0DSP compiler.
    Revision 1.8  2011/05/19 00:03:38Z  garyp
    Corrected to properly use DCLDIMENSIONOF() where necessary.
    Revision 1.7  2010/07/05 22:11:53Z  garyp
    Added support for chip serial numbers.
    Revision 1.6  2010/06/20 18:32:27Z  garyp
    Minor message tweak.
    Revision 1.5  2010/06/19 03:35:20Z  garyp
    Updated to show the bit correction capabilities on a per segment basis.
    Revision 1.4  2010/04/19 19:12:58Z  garyp
    Minor update to the info display.
    Revision 1.3  2010/01/07 23:43:11Z  garyp
    Minor formatting fix.
    Revision 1.2  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.1  2009/12/03 22:02:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h> 
#include <fxfmlapi.h>
#include <fxinfohelp.h>

#if FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif

static void     DisplayDevType(D_UINT16 uDevType);
static char *   ScaleKB(D_UINT32 ulKBValue, char *pszBuffer, unsigned nBufferSize);
static char *   ScalePages(D_UINT32 ulPages, D_UINT32 ulPageSize, char *pszBuffer, unsigned nBufferSize);
static char *   ScaleBytes(D_UINT32 ulByteValue, char *pszBuffer, unsigned nBufferSize);


/*-------------------------------------------------------------------
    Protected: FfxDisplayDiskInfo()

    Display information about a Disk.

    Parameters:
        hFML     - The FML handle to use
        fVerbose - TRUE to display verbose Disk information, such 
                   as information about the Devices upon which the
                   Disk resides.

    Return Value:
        Returns an FFXSTATUS value indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDisplayDiskInfo(
    FFXFMLHANDLE    hFML,
    D_BOOL          fVerbose)
{
    FFXSTATUS       ffxStat;
    FFXFMLDEVINFO   FmlDevInfo;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulFmlBlocksKB;
    char            szScaleBuffer[32];
    unsigned        nn;
    D_UINT32        ulDiskOffsetKB;
    D_UINT32        ulDiskSizeKB;
    D_UINT32        ulRemainingBlocks;
    D_UINT32        ulFirstDevBlocks;

    DclAssert(hFML);

    /*  Get the Disk information.
    */
    ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    if(fVerbose)
    {
        unsigned    nDevNum;
        
        for(nDevNum = FmlInfo.nDeviceNum; nDevNum < FmlInfo.nDeviceNum + FmlInfo.nDeviceCount; nDevNum++)
        {
            ffxStat = FfxDisplayDeviceInfo(hFML, nDevNum);
            if(ffxStat != FFXSTAT_SUCCESS)
                return ffxStat;

            DclPrintf("\n");
        }
    }
    
    ffxStat = FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    ulFmlBlocksKB       = FmlInfo.ulBlockSize / 1024;
    ulDiskOffsetKB      = FmlInfo.ulStartBlock * (FmlDevInfo.ulBlockSize / 1024);
    ulDiskSizeKB        = FmlInfo.ulTotalBlocks * (FmlDevInfo.ulBlockSize / 1024);
    ulFirstDevBlocks    = (FmlInfo.nDeviceCount == 1) ? FmlInfo.ulTotalBlocks : FmlDevInfo.ulTotalBlocks - FmlInfo.ulStartBlock;
    
    /*  Display the Disk information from an FML perspective
    */
    DclPrintf("DISK%u FML Information\n", FmlInfo.nDiskNum);
    {
        char                szTempBuff[128];
        static const char  *apszName[] = {"RawAccess",      /* DISK_RAWACCESSONLY   */
                                          "HasBootBlocks",  /* DISK_HASBOOTBLOCKS   */
                                          "IsNAND"};        /* DISK_NAND            */
        
        DclStrCpy(szTempBuff, "(");

        DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), apszName, 
                          DCLDIMENSIONOF(apszName), (D_UINT32)FmlInfo.uDiskFlags);

        if(DclStrLen(szTempBuff) == 2)
            DclStrCpy(szTempBuff, "(None)");
        else
            szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

        DclPrintf("  Disk Flags                                %X %s\n", FmlInfo.uDiskFlags, szTempBuff);
    }

    DisplayDevType(FmlInfo.uDeviceType);

    DclPrintf("  Total Devices used                    %10u\n",        FmlInfo.nDeviceCount);
    DclPrintf("  Starting Block Offset in DEV%u         %10lU\n",      FmlDevInfo.nDeviceNum, FmlInfo.ulStartBlock);
    DclPrintf("  Total Blocks used in DEV%U             %10lU\n",      FmlDevInfo.nDeviceNum, ulFirstDevBlocks);

    ulRemainingBlocks = FmlInfo.ulTotalBlocks - ulFirstDevBlocks;
    for(nn=1; nn<FmlInfo.nDeviceCount; nn++)
    {
        D_UINT32            ulThisDevBlocks;
        FFXFMLDEVINFO       NextDev;

        DclAssert(ulRemainingBlocks);

        ffxStat = FfxFmlDeviceInfo(FmlInfo.nDeviceNum+nn, &NextDev);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        ulThisDevBlocks = DCLMIN(ulRemainingBlocks, FmlDevInfo.ulTotalBlocks);

        DclPrintf("  Total Blocks used in DEV%U             %10lU\n",  FmlDevInfo.nDeviceNum+nn, ulThisDevBlocks);

        ulRemainingBlocks -= ulThisDevBlocks;
    }

    DclPrintf("  Start Offset within DEV%u              %10lU KB%s\n", FmlDevInfo.nDeviceNum, ulDiskOffsetKB, ScaleKB(ulDiskOffsetKB, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("  Disk Size                             %10lU KB%s\n",  ulDiskSizeKB, ScaleKB(ulDiskSizeKB, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("  Block Size                            %10lU KB\n",    ulFmlBlocksKB);
    DclPrintf("  Page Size                             %10U bytes\n",  FmlInfo.uPageSize);
    
    if(FmlDevInfo.uDeviceType == DEVTYPE_NAND)
        DclPrintf("  Spare Size                            %10U bytes\n", FmlInfo.uSpareSize);
    
    DclPrintf("  FFXDISKHANDLE                         %P\n", FmlInfo.hDisk);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxDisplayDeviceInfo()

    Display information about a Device.

    Parameters:
        hFML    - The FML handle
        nDevNum - The Device number.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDisplayDeviceInfo(
    FFXFMLHANDLE    hFML,
    unsigned        nDevNum)
{
    FFXSTATUS       ffxStat;
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;
    D_UINT32        ulFmlDevBlocksKB;
    D_UINT32        ulRawSizeKB;
    D_UINT32        ulTotalSizeKB;
    D_UINT32        ulChipSizeKB;
    D_UINT32        ulLowReservedKB;
    D_UINT32        ulHighReservedKB;
    unsigned        nChipCount;
    char            szScaleBuffer[32];
    D_BYTE          abID[FFX_MAX_DEVICES * FFXPARAM_CHIPID_LENGTH] = {0};
    D_BYTE          abSN[FFX_MAX_DEVICES * FFXPARAM_CHIPSN_LENGTH] = {0};

    DclAssert(hFML);
    
    ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclAssert(nDevNum >= FmlInfo.nDeviceNum);
    DclAssert(nDevNum < FmlInfo.nDeviceNum + FmlInfo.nDeviceCount);

    ffxStat = FfxFmlDeviceInfo(nDevNum, &FmlDevInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Display the Device information.
    */
    DclPrintf("DEV%u Information\n", nDevNum);
    {
        char                szTempBuff[128];
        static const char  *apszName[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                                         "Lockable",    /* DEV_LOCKABLE             */
                                         "VarLenTags",  /* DEV_VARIABLE_LENGTH_TAGS */
                                         "OldNorFim",   /* DEV_OLDSTYLE_NORFIM      */
                                         "NeedsECC",    /* DEV_REQUIRES_ECC         */
                                         "PgmOnce",     /* DEV_PGM_ONCE             */
                                         "MergeWrites", /* DEV_MERGE_WRITES         */
                                         "NorSLC",      /* DEV_NOT_MLC              */
                                         "UsingBBM",    /* DEV_USING_BBM            */
                                         "NeedsBBM"};   /* DEV_REQUIRES_BBM         */
        
        DclStrCpy(szTempBuff, "(");

        DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), apszName, 
                          DCLDIMENSIONOF(apszName), (D_UINT32)FmlDevInfo.uDeviceFlags);

        szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

        DclPrintf("  Device Flags              %X %s\n", FmlDevInfo.uDeviceFlags, szTempBuff);
    }
    
    if(FmlDevInfo.uDeviceFlags & DEV_LOCKABLE)
    {
        char                szTempBuff[128];

        DclStrCpy(szTempBuff, "(");

        if(FmlDevInfo.nLockFlags)
        {
            static const char *apszName[] = {"LockAll",         /* FFXLOCKFLAGS_LOCKALLBLOCKS         */
                                             "LockFreeze",      /* FFXLOCKFLAGS_LOCKFREEZE            */
                                             "UnlockRange",     /* FFXLOCKFLAGS_UNLOCKSINGLERANGE     */
                                             "UnlockInvRange"}; /* FFXLOCKFLAGS_UNLOCKINVERTEDRANGE   */
            
            DclFormatBitFlags(szTempBuff, DCLDIMENSIONOF(szTempBuff), 
                              apszName, DCLDIMENSIONOF(apszName), (D_UINT32)FmlDevInfo.nLockFlags);
        }
        else
        {
            DclStrNCat(szTempBuff, "Discrete ", sizeof(szTempBuff) - DclStrLen(szTempBuff));
        }

        szTempBuff[DclStrLen(szTempBuff) - 1] = ')';

        DclPrintf("  Locking Capabilities  0x%08x %s\n", FmlDevInfo.nLockFlags, szTempBuff);
    }
    else
    {
        /*  The lock flags should always be 0 if DEV_LOCKABLE is not set
        */
        DclAssert(!FmlDevInfo.nLockFlags);
    }

    DisplayDevType(FmlDevInfo.uDeviceType);

    DclPrintf("  Minimum Alignment Required               %3U-bit\n", FmlDevInfo.uAlignSize * 8);
    DclPrintf("  Page Alignment Buffer                   %s\n",     ((FmlDevInfo.uAlignSize > 1) && FFXCONF_FORCEALIGNEDIO) ? " Enabled" : "Disabled");
    DclPrintf("  OTP Pages                             %10u\n",       FmlDevInfo.nOTPPages);

    ulFmlDevBlocksKB    = FmlDevInfo.ulBlockSize / 1024;
    ulRawSizeKB         = FmlDevInfo.ulRawBlocks * ulFmlDevBlocksKB;
    ulTotalSizeKB       = FmlDevInfo.ulTotalBlocks * ulFmlDevBlocksKB;
    ulChipSizeKB        = FmlDevInfo.ulChipBlocks * ulFmlDevBlocksKB;
    ulLowReservedKB     = FmlDevInfo.ulReservedBlocks * ulFmlDevBlocksKB;
    nChipCount = (ulLowReservedKB + ulRawSizeKB + ulChipSizeKB - 1) / ulChipSizeKB;
    ulHighReservedKB    = (ulChipSizeKB * nChipCount) - (ulLowReservedKB + ulRawSizeKB);

    DclPrintf("  Current DISKn Mappings                %10u\n", FmlDevInfo.nDisksMapped);
    if(FmlDevInfo.uDeviceType == DEVTYPE_NAND)
    {
        char        szPercent[16];
        D_UINT32    ulBBMSizeKB = ulRawSizeKB - ulTotalSizeKB;

        DclPrintf("  DevMgr Raw Size (includes BBM)        %10lU KB%s\n", ulRawSizeKB, ScaleKB(ulRawSizeKB, szScaleBuffer, sizeof(szScaleBuffer)));
        DclPrintf("  DevMgr Device Size (excludes BBM)     %10lU KB%s\n", ulTotalSizeKB, ScaleKB(ulTotalSizeKB, szScaleBuffer, sizeof(szScaleBuffer)));
        DclPrintf("  BBM Media Overhead                    %10lU KB (~%s%%)\n", ulBBMSizeKB, DclRatio(szPercent, sizeof(szPercent), (FmlDevInfo.ulRawBlocks - FmlDevInfo.ulTotalBlocks)*100, FmlDevInfo.ulRawBlocks, 1));
    }
    else
    {
        DclAssert(FmlDevInfo.ulRawBlocks == FmlDevInfo.ulTotalBlocks);

        DclPrintf("  DevMgr Device Size                    %10lU KB%s\n", ulTotalSizeKB, ScaleKB(ulTotalSizeKB, szScaleBuffer, sizeof(szScaleBuffer)));
    }

    DclPrintf("  Block Size                            %10lU KB\n",         ulFmlDevBlocksKB);
    DclPrintf("  Page Size                             %10U bytes\n",       FmlDevInfo.uPageSize);
    if(FmlDevInfo.uDeviceType == DEVTYPE_NAND)
    {
        char szTempBuff[64];
        
        DclPrintf("  Spare Size                            %10U bytes\n",   FmlDevInfo.uSpareSize);
        DclPrintf("  Meta Size                             %10U bytes\n",   FmlDevInfo.uMetaSize);

        DclSNPrintf(szTempBuff, sizeof(szTempBuff), "%U bit%s per %U bytes", 
            FmlDevInfo.uEdcCapability, 
            FmlDevInfo.uEdcCapability > 1 ? "s" : "", 
            FmlDevInfo.uEdcSegmentSize);

        DclPrintf("  Recommended EDC                       %10U\n",         FmlDevInfo.uEdcRequirement);
        DclPrintf("  EDC Capabilities      %32s\n",                         szTempBuff);
    }

    DclPrintf("  Recommended Spare Blocks              %10lU\n",            FmlDevInfo.ulBBMReservedRating);
    DclPrintf("  Erase Cycle Rating                    %10lU\n",            FmlDevInfo.ulEraseCycleRating);
    DclPrintf("  FFXFIMDEVHANDLE                       %P\n",               FmlDevInfo.hFimDev);
    DclPrintf("  Physical Flash Array Info\n");
    DclPrintf("    Chip Size                           %10lU KB%s\n", ulChipSizeKB, ScaleKB(ulChipSizeKB, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("    Chip Count                          %10u\n", nChipCount);

    if(FfxFmlParameterGet(hFML, FFXPARAM_FIM_CHIPID, abID, sizeof(abID)) == FFXSTAT_SUCCESS)
    {
        D_BYTE *pabID = &abID[(nDevNum - FmlInfo.nDeviceNum) * FFXPARAM_CHIPID_LENGTH];

        /*  We queried the Chip ID values for all the Devices upon which this
            Disk resides, however we only want to display the ID values for
            the one Device we are actually processing.
        */            
        DclPrintf("    Chip ID                %02x %02x %02x %02x %02x %02x %02x %02x\n",
            *(pabID+0), *(pabID+1), *(pabID+2), *(pabID+3), *(pabID+4), *(pabID+5), *(pabID+6), *(pabID+7));
    }
    else
    {
        DclPrintf("    Chip ID                          Not Available\n");
    }

    if(FfxFmlParameterGet(hFML, FFXPARAM_FIM_CHIPSN, abSN, sizeof(abSN)) == FFXSTAT_SUCCESS)
    {
        D_BYTE *pabSN = &abSN[(nDevNum - FmlInfo.nDeviceNum) * FFXPARAM_CHIPSN_LENGTH];

        /*  We queried the Chip SN values for all the Devices upon which this
            Disk resides, however we only want to display the SN values for
            the one Device we are actually processing.
        */            
        DclPrintf("    Chip SN    %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
            *(pabSN+0), *(pabSN+1), *(pabSN+2), *(pabSN+3), *(pabSN+4), *(pabSN+5), *(pabSN+6), *(pabSN+7),
            *(pabSN+8), *(pabSN+9), *(pabSN+10), *(pabSN+11), *(pabSN+12), *(pabSN+13), *(pabSN+14), *(pabSN+15));
    }
    else
    {
        DclPrintf("    Chip Serial Number               Not Available\n");
    }

    DclPrintf("    Low Reserved Space (not accessible) %10lU KB%s\n", ulLowReservedKB,  ScaleKB(ulLowReservedKB, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("    High Reserved Space (not accessible)%10lU KB%s\n", ulHighReservedKB, ScaleKB(ulHighReservedKB, szScaleBuffer, sizeof(szScaleBuffer)));

    if(FmlDevInfo.uDeviceType == DEVTYPE_NOR)
    {
        DclPrintf("    Boot Block Size                     %10lU KB\n", FmlDevInfo.ulBootBlockSize / 1024);
        DclPrintf("      Low Boot Block Count              %10U\n",     FmlDevInfo.uLowBootBlockCount);
        DclPrintf("      High Boot Block Count             %10U\n",     FmlDevInfo.uHighBootBlockCount);
    }

  #if FFXCONF_LATENCYREDUCTIONENABLED
    {
        D_UINT32    ulReadCount;
        D_UINT32    ulWriteCount;
        D_UINT32    ulEraseCount;
        D_UINT16    uErasePollInterval;
        unsigned    fCycleMutex;

        FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXREADCOUNT,      &ulReadCount,        sizeof(ulReadCount));
        FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXWRITECOUNT,     &ulWriteCount,       sizeof(ulWriteCount));
        FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXERASECOUNT,     &ulEraseCount,       sizeof(ulEraseCount));
        FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_ERASEPOLLINTERVAL, &uErasePollInterval, sizeof(uErasePollInterval));
        FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_CYCLEMUTEX,        &fCycleMutex,        sizeof(fCycleMutex));

        /*  Note that we are intentionally displaying the unsigned values
            below with signs, simply to make the output more concise since
            a D_UINT32_MAX value is legit and common.
        */
        DclPrintf("  Latency Settings\n");
        DclPrintf("    Max Amount to Read per Request           %5lD pages\n", ulReadCount);
        DclPrintf("    Max Amount to Write per Request          %5lD pages\n", ulWriteCount);
        DclPrintf("    Max Amount to Erase per Request          %5lD blocks\n", ulEraseCount);
        DclPrintf("    Erase Poll Interval                      %5U ms\n", uErasePollInterval);
        DclPrintf("    Cycle the DEV mutex during I/O           %5s\n", fCycleMutex ? "Yes" : "No");
    }
  #endif

    return FFXSTAT_SUCCESS;
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Protected: FfxDisplayVbfDiskInfo()

    Displays information about a VBF Disk.  This includes global
    information (such as location and capacity) and a summary of
    per-unit information (like free space and erase count).

    Parameters:
        hVBF        - The VBF handle
        fVerbose    - TRUE to display more detailed information.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDisplayVbfDiskInfo(
    VBFHANDLE       hVBF,
    D_BOOL          fVerbose)
{
    #define         BUFFLEN     (64)
    FFXSTATUS       ffxStat;
    VBFDISKINFO     DiskInfo;
    VBFDISKMETRICS  DiskMets;
    D_UINT32        ulTotalPages;
    D_UINT32        ulMetaPages;
    D_UINT32        ulCushionPages;
    D_UINT32        ulOverheadPages;
    D_UINT32        ulFormattedPages;
    char            szPercent[16];
    char            szScaleBuffer[32];

    DclAssert(hVBF);

    /*  Get the information about this disk
    */
    ffxStat = FfxVbfDiskInfo(hVBF, &DiskInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    ffxStat = FfxVbfDiskMetrics(hVBF, &DiskMets);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    /*  Formatted pages = available client pages times number of non-spare
        units.
    */
    ulFormattedPages = (DiskMets.ulTotalUnits - DiskMets.uSpareUnits) * DiskMets.uUnitClientPages;
    DclAssert(ulFormattedPages == DiskMets.ulClientPages);

    /*  Total pages = pages per unit times total units
    */
    ulTotalPages = DiskMets.uUnitTotalPages * DiskMets.ulTotalUnits;

    /*  Display information about VBF in a format useful to people.
    */
    DclPrintf("DISK%u VBF Allocator Information\n",                        DiskInfo.nDiskNum);
    
    if(fVerbose)
    {
        DclPrintf("  Compile-Time VBF Settings\n");
        DclPrintf("    Minimum VBF Allocation Size         %10u\n",        VBF_MIN_BLOCK_SIZE);
        DclPrintf("    Maximum VBF Allocation Size         %10u\n",        VBF_MAX_BLOCK_SIZE);
        DclPrintf("    Maximum Erase Units per Disk        %10lu\n",       VBF_PARTITION_MAX_UNITS);
        DclPrintf("    Maximum Erase Unit Size             %10lu KB\n",    VBF_MAX_UNIT_SIZE / 1024UL);
      #if VBFCONF_REGIONCACHESIZE > 0
        DclPrintf("    Region Cache Entries (fixed maximum) %9lu\n", VBFCONF_REGIONCACHESIZE);
      #else
        DclPrintf("    Region Cache Entries                 %9lu%%\n",     VBFCONF_REGIONCACHESIZE * -1);
      #endif
      #ifdef VBFCONF_STATICREGION
        DclPrintf("    Statically Mounted Region #           %8u\n",       VBFCONF_STATICREGION);
      #else
        DclPrintf("    Statically Mounted Region #               None\n");
      #endif
        DclPrintf("    Default Cushion                     %8u.%u%%\n",    VBFCONF_DEFAULTCUSHION / 10, VBFCONF_DEFAULTCUSHION % 10);
        DclPrintf("    Deferred NAND Discards              %10s\n", 
      #if VBFCONF_DEFERREDDISCARD
            "Enabled");
      #else
            "Disabled");
      #endif
        DclPrintf("    Compaction Threshold                %10ld\n",       VBFCONF_COMPACTIONTHRESHOLD);
        DclPrintf("    Compaction Buffer Size              %10lu bytes\n", VBF_COMPACTION_BUFFER_SIZE);
    }
    
    DclPrintf("  Serial Number                         %lX\n",             DiskInfo.ulSerialNumber);
    if(DiskInfo.fReadOnly || DiskInfo.fEvaluation)
        DclPrintf("  Flags                                 %10s%s\n",      DiskInfo.fEvaluation ? "EVALUATION" : "", DiskInfo.fReadOnly ? " READ-ONLY" : "");
    DclPrintf("  Allocation Size                         %8U bytes\n",     DiskMets.uPageSize);
    DclPrintf("  Unformatted Size                      %10lU allocs%s\n",  ulTotalPages, ScalePages(ulTotalPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("  Formatted Size                        %10lU allocs%s\n",  ulFormattedPages, ScalePages(ulFormattedPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("  Region Information\n");
    DclPrintf("    Total/Cached Regions               %5lU /%4u\n",        DiskMets.ulTotalRegions, DiskMets.nRegionsCached);
    DclPrintf("    Erase Units Per Region              %10lU\n",           DiskMets.ulRegionTotalPages / DiskMets.uUnitTotalPages);
    DclPrintf("    Physical Region Size                %10lU allocs%s\n",  DiskMets.ulRegionTotalPages, ScalePages(DiskMets.ulRegionTotalPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("    Client Region Size                  %10lU allocs%s\n",  DiskMets.ulRegionClientPages, ScalePages(DiskMets.ulRegionClientPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("  Erase Unit Information\n");
    DclPrintf("    Total Units                           %8U\n",           DiskMets.ulTotalUnits);
    DclPrintf("    Spare Unit(s)                         %8U\n",           DiskMets.uSpareUnits);
    DclPrintf("    Unit Size                           %10lU allocs%s\n",  DiskMets.uUnitTotalPages, ScalePages(DiskMets.uUnitTotalPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    DclPrintf("      Per Unit Metadata                   %8U allocs\n",    DiskMets.uUnitTotalPages - DiskMets.uUnitDataPages);
    DclPrintf("      Per Unit Cushion                    %8U allocs\n",    DiskMets.uUnitDataPages - DiskMets.uUnitClientPages);
    DclPrintf("      Per Unit Client Data                %8U allocs\n",    DiskMets.uUnitClientPages);

    /*  Display overhead information
    */

    /*  Metadata includes the EUHs and allocation maps.  NAND
        discard metadata is not included because it does not
        reduce the available client space on the flash.
    */
    ulMetaPages = (DiskMets.ulTotalUnits - DiskMets.uSpareUnits) * (DiskMets.uUnitTotalPages - DiskMets.uUnitDataPages);

    /*  Cushion pages.  Each unit has equal cushion: it is the
        difference between the number of data pages and the
        number of client pages.  Spares don't count as cushion.
    */
    ulCushionPages = (DiskMets.uUnitDataPages - DiskMets.uUnitClientPages) * (DiskMets.ulTotalUnits - DiskMets.uSpareUnits);

    /*  Total overhead pages is the difference between the total pages
        in the FML Disk, and the number of usable client pages after
        formatting with the allocator.
    */
    ulOverheadPages = ulTotalPages - ulFormattedPages;

    DclPrintf("  Media Overhead\n");
    DclPrintf("    Metadata                            %10lU allocs (~%s%%)\n", ulMetaPages, DclRatio(szPercent, sizeof(szPercent), ulMetaPages * 100, ulTotalPages, 1));
    DclPrintf("    Cushion                             %10lU allocs (~%s%%)\n", ulCushionPages, DclRatio(szPercent, sizeof(szPercent), ulCushionPages * 100, ulTotalPages, 1));
    DclPrintf("    Spare Unit(s)                       %10lU allocs (~%s%%)\n", DiskMets.uUnitTotalPages, DclRatio(szPercent, sizeof(szPercent), DiskMets.uSpareUnits * 100, DiskMets.ulTotalUnits, 1));
    DclPrintf("    Total                               %10lU allocs (~%s%%)\n", ulOverheadPages, DclRatio(szPercent, sizeof(szPercent), ulOverheadPages * 100,  DiskMets.ulTotalPages, 1));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxDisplayVbfMediaUsageInfo()

    Display VBF media usage information.  Note that this function
    will skew <any and all> statistics information which might be
    in the process of being gathered.

    Parameters:
        hVBF        - The VBF handle
        fVerbose    - TRUE to display more detailed information.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDisplayVbfMediaUsageInfo(
    VBFHANDLE       hVBF,
    D_BOOL          fVerbose)
{
    VBFDISKMETRICS  DiskMets;
    D_UINT32        ulRgn;
    D_UINT32        ulUnit;
    D_UINT32        ulValidUnits = 0;
    D_UINT32        ulMinEraseCount = 0;
    D_UINT32        ulMaxEraseCount = 0;
    D_UINT32        ulTotalErases = 0;
    D_UINT32        ulDataPages = 0;
    D_UINT32        ulFreePages = 0;
    D_UINT32        ulDiscardedPagesActive = 0;
    D_UINT32        ulDiscardedPagesInactive = 0;
    char            szTempBuff[BUFFLEN];
    FFXSTATUS       ffxStat;
    unsigned        nUnitsSuperDirty = 0;
    unsigned        nUnitsVeryDirty = 0;
    unsigned        nUnitsQualifiedDirty = 0;
    unsigned        nUnitsNotClean = 0;

    DclAssert(hVBF);

    ffxStat = FfxVbfDiskMetrics(hVBF, &DiskMets);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    DclPrintf("VBF Media Usage Information\n");
    DclPrintf("  Gathering -- one moment please...\n");

    if(fVerbose)
    {
        DclPrintf("  Detailed Region Information\n");
        DclPrintf("    Region Usage Unit  Allocations [----Unit Dirtiness-----]\n");
        DclPrintf("    Number Count Count Spent  Left Super  Very Dirty Unclean\n");
    }

    for(ulRgn=0; ulRgn<DiskMets.ulTotalRegions; ulRgn++)
    {
        VBFREGIONMETRICS    RgnMets;

        ffxStat = FfxVbfRegionMetrics(hVBF, ulRgn, &RgnMets);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Error %lX getting region information for region %lU\n", ffxStat, ulRgn);
            return ffxStat;
        }

        if(!RgnMets.fUnitsDirtyValid)
            DclPrintf("Region %lU units dirty data is not valid\n", ulRgn);

        if(fVerbose)
        {
            DclPrintf("    %5lU %4lU  %5u  %5u %5u %5u %5u %5u   %5u\n", ulRgn,
                RgnMets.ulUsageCount,     RgnMets.nUnits,
                RgnMets.nAllocsSpent,     RgnMets.nAllocsLeft,
                RgnMets.nUnitsSuperDirty, RgnMets.nUnitsVeryDirty,
                RgnMets.nUnitsDirty,      RgnMets.nUnitsNotClean);
        }

        nUnitsSuperDirty        += RgnMets.nUnitsSuperDirty;
        nUnitsVeryDirty         += RgnMets.nUnitsVeryDirty;
        nUnitsQualifiedDirty    += RgnMets.nUnitsDirty;
        nUnitsNotClean          += RgnMets.nUnitsNotClean;
    }

    DclPrintf("  Region Composite Unit Dirtiness\n");
    DclPrintf("    SuperDirty - %5u+ discarded allocs  %8u units\n", DiskMets.nQualSuperDirty, nUnitsSuperDirty);
    DclPrintf("    VeryDirty  - %5u+ discarded allocs  %8u units\n", DiskMets.nQualVeryDirty,  nUnitsVeryDirty);
    DclPrintf("    Qualified  - %5u+ discarded allocs  %8u units\n", DiskMets.nQualDirty,      nUnitsQualifiedDirty);
    DclPrintf("    NotClean   - %5u+ discarded allocs  %8u units\n", DiskMets.nQualNotClean,   nUnitsNotClean);

    /*  If using the external API, it's possible this program and the FlashFX
        driver were compiled with different configurations.  Make sure that
        uUnitTotalPages does not exceed the dimension of acBlockMap[] in the
        VBFUNITMETRICS structure.

        Unfortunately, there doesn't seem to be a way to determine at compile
        time whether the code is being built for the external API, so this
        check must always be done, even if it couldn't actually fail.
    */
    if (DiskMets.uUnitTotalPages > DCLDIMENSIONOF(((VBFUNITMETRICS *) 0)->acBlockMap))
    {
        DclPrintf("\n  Detailed usage information cannot be obtained or displayed.\n");
        DclPrintf("\n  This program appears to have been built with a configuration that");
        DclPrintf("\n  doesn't match the FlashFX driver.  The value of VBF_MAX_UNIT_BLOCKS");
        DclPrintf("\n  (VBF_MAX_UNIT_SIZE / VBF_MIN_BLOCK_SIZE) is smaller than the actual");
        DclPrintf("\n  number of allocation blocks in an erase unit.  Make sure the same");
        DclPrintf("\n  values were used in vbfconf.h when compiling the driver and the tools.\n\n");
        return ffxStat;
    }

    if(fVerbose)
    {
        DclPrintf("\n  Detailed Unit Information\n");
        DclPrintf("     Unit   Total  [-------Allocations-------] Region   Is \n");
        DclPrintf("    Number  Erases Valid   Free Discard System (*act) Spare\n");
    }

    /*  Iterate through the units and calculate summary information.
    */
    for(ulUnit = 0; ulUnit < DiskMets.ulTotalUnits; ulUnit++)
    {
        VBFUNITMETRICS  uInfo;

        uInfo.lnu = ulUnit;
        ffxStat = FfxVbfUnitMetrics(hVBF, &uInfo);

        if((ffxStat == FFXSTAT_SUCCESS) && uInfo.bValidVbfUnit)
        {
            D_BOOL fIsActive = FALSE;

            /*  Special case on the first valid unit to initialize
                erase count statistics.
            */
            if(ulValidUnits++ == 0)
            {
                ulMinEraseCount = ulMaxEraseCount = ulTotalErases = uInfo.ulEraseCount;
            }
            else
            {
                ulMaxEraseCount = DCLMAX(ulMaxEraseCount, uInfo.ulEraseCount);
                ulMinEraseCount = DCLMIN(ulMinEraseCount, uInfo.ulEraseCount);

                /*  There is a potential for this to overflow in the
                    reasonably near future.
                */
                ulTotalErases += uInfo.ulEraseCount;
            }

            /*  Don't tally space in spare units.
            */
            if(!uInfo.bSpareUnit)
            {
                ulDataPages += uInfo.ulValidPages;
                ulFreePages += uInfo.ulFreePages;

                if(uInfo.ulFreePages && (uInfo.ulValidPages || uInfo.ulDiscardedPages))
                {
                    fIsActive = TRUE;
                    ulDiscardedPagesActive += uInfo.ulDiscardedPages;
                }
                else
                {
                    ulDiscardedPagesInactive += uInfo.ulDiscardedPages;
                }
            }

            if(fVerbose)
            {
                DclPrintf("    %6lU %7lU %6lU %6lU %6lU %6lU %6lU%s %s\n",
                    ulUnit, uInfo.ulEraseCount, uInfo.ulValidPages,
                    uInfo.ulFreePages, uInfo.ulDiscardedPages,
                    uInfo.ulSystemPages, uInfo.region,
                    fIsActive ? "*" : " ",
                    uInfo.bSpareUnit ? "Yes" : " No");
            }

            if( uInfo.ulValidPages + uInfo.ulFreePages +
                uInfo.ulDiscardedPages + uInfo.ulSystemPages != DiskMets.uUnitTotalPages)
            {
                DclPrintf("WARNING! Unit %lU page counts do not add up!\n", ulUnit);
            }
        }
    }
 
    /*  Display the disk usage information
    */
    DclAssert(ulValidUnits);

    if(ulValidUnits)
    {
        char    szScaleBuffer[32];
        
        DclPrintf("  Total Unit Erases                     %10lU\n", ulTotalErases);

        DclSNPrintf(szTempBuff, BUFFLEN, "(%lU, %lU, %lU)", ulMaxEraseCount, ulTotalErases / ulValidUnits, ulMinEraseCount);

        DclPrintf("  Erases Per Unit (Max, Avg, Min) %16s\n",  szTempBuff);
        DclPrintf("  Data Used                             %10lU allocs%s\n", ulDataPages, ScalePages(ulDataPages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
        DclPrintf("  Free Space                            %10lU allocs%s\n", ulFreePages, ScalePages(ulFreePages, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
        DclPrintf("  Recoverable Space in Inactive Units   %10lU allocs%s\n", ulDiscardedPagesInactive, ScalePages(ulDiscardedPagesInactive, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
        DclPrintf("  Recoverable Space in Active Units     %10lU allocs%s\n", ulDiscardedPagesActive, ScalePages(ulDiscardedPagesActive, DiskMets.uPageSize, szScaleBuffer, sizeof(szScaleBuffer)));
    }

    return ffxStat;
}
    
#endif  /* FFXCONF_VBFSUPPORT */
 

/*-------------------------------------------------------------------
    Protected: FfxDisplayEmulatedDiskInfo()

    Display information about the emulated Disk.

    Parameters:
        hFML - The FML handle to use.

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDisplayEmulatedDiskInfo(
    FFXFMLHANDLE        hFML)
{
    FFXSTATUS           ffxStat;
    FFXFMLINFO          FmlInfo;
    FFXDISKGEOMETRY     geo;
    D_UINT32            ulLostBytes;
    D_UINT64            ullTemp1;
    D_UINT64            ullTemp2;
    char                szScaleBuffer[32];
  #if FFXCONF_VBFSUPPORT
    VBFDISKMETRICS      DiskMets;
    VBFHANDLE           hVBF;
  #endif

    DclAssert(hFML);

    ffxStat = FfxFmlDiskInfo(hFML, &FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

    ffxStat = FfxDriverDiskGeometry(FfxDriverDiskHandle(NULL, FmlInfo.nDiskNum), &geo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return ffxStat;

  #if FFXCONF_VBFSUPPORT
    hVBF = FfxVbfHandle(FmlInfo.nDiskNum);
    if(hVBF)
    {
        ffxStat = FfxVbfDiskMetrics(hVBF, &DiskMets);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        DclUint64AssignUint32(&ullTemp1, DiskMets.ulClientPages);
        DclUint64MulUint32(&ullTemp1, DiskMets.uPageSize);
    }
    else
  #endif
    {
        DclUint64AssignUint32(&ullTemp1, FmlInfo.ulTotalBlocks);
        DclUint64MulUint32(&ullTemp1, FmlInfo.ulBlockSize);
    }

    DclUint64AssignUint32(&ullTemp2, geo.ulLogicalSectorCount);
    DclUint64MulUint32(&ullTemp2, geo.nBytesPerSector);

    DclUint64SubUint64(&ullTemp1, &ullTemp2);

    ulLostBytes = DclUint32CastUint64(&ullTemp1);

    DclPrintf("DISK%u Emulated Geometry\n",                                geo.nDiskNum);
    DclPrintf("  Sector Length                         %10u bytes\n",      geo.nBytesPerSector);
    DclPrintf("  Disk Size                             %10lU sectors%s\n", geo.ulLogicalSectorCount, ScalePages(geo.ulLogicalSectorCount, geo.nBytesPerSector, szScaleBuffer, sizeof(szScaleBuffer)));

    if(geo.fCHSValid)
    {
        DclPrintf("  CHS Info \n");
        DclPrintf("    Cylinders                           %10u\n",        geo.nCylinders);
        DclPrintf("    Heads                               %10u\n",        geo.nHeads);
        DclPrintf("    SectorsPerTrack                     %10u\n",        geo.nSectorsPerTrack);
    }
    else
    {
        DclPrintf("  CHS Info                               Not Valid\n");
    }

    DclPrintf("  Waste Due to Emulation Rounding       %10lU bytes%s\n", ulLostBytes, ScaleBytes(ulLostBytes, szScaleBuffer, sizeof(szScaleBuffer)));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: DisplayDevType()

    Display the device type.

    Parameters:
        uDevType - the DEVTYPE_* type

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayDevType(
    D_UINT16    uDevType)
{
    switch(uDevType)
    {
        case DEVTYPE_ROM:
            DclPrintf("  Media Type                                   ROM\n");
            break;
        case DEVTYPE_RAM:
            DclPrintf("  Media Type                                   RAM\n");
            break;
        case DEVTYPE_ATA:
            DclPrintf("  Media Type                                   ATA\n");
            break;
        case DEVTYPE_NOR:
            DclPrintf("  Media Type                                   NOR\n");
            break;
        case DEVTYPE_NAND:
            DclPrintf("  Media Type                                  NAND\n");
            break;
        case DEVTYPE_AND:
            DclPrintf("  Media Type                                   AND\n");
            break;
        case DEVTYPE_ISWF:
            DclPrintf("  Media Type                                Sibley\n");
            break;
        default:
            DclError();
            DclPrintf("  Media Type                               Unknown\n");
            break;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ScaleKB()

    This function scales a number from KB up to MB or GB as
    appropriate.  If the size is scaled, the value is formatted
    as a string in the buffer given using the form " (nnn MB)".
    If the size cannot be scaled, the buffer is set to an empty
    string.

    Parameters:
        ulSizeInKB  - the size to scale, in KB
        pszBuffer   - the buffer to format the scaled size
        nBufferSize - the size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
static char * ScaleKB(
    D_UINT32    ulKBValue,
    char       *pszBuffer,
    unsigned    nBufferSize)
{
    char       *pBuff;
    char        szBuffer[16];

    DclAssert(pszBuffer);
    DclAssert(nBufferSize);

    if(ulKBValue)
    {
        pBuff = DclScaleKB(ulKBValue, szBuffer, sizeof(szBuffer));
        if(*pBuff != 0)
        {
            /*  If the second to last digit is <not> a 'K', then the number
                was scaled, and we want to finish formatting it.  If the
                number was not scaled, then we just want to return an empty
                string.
            */
            if(szBuffer[DclStrLen(szBuffer)-2] != 'K')
            {
                /*  Format the scaled value into the client buffer
                */
                if(DclSNPrintf(pszBuffer, nBufferSize, " (%s)", &szBuffer[0]) != -1)
                    return pszBuffer;
            }
        }
    }

    /*  Either the number was not evenly scalable to MB or GB, or we had
        buffer overflow.  In either case, just return an empty string.
    */
    *pszBuffer = 0;

    return pszBuffer;
}


/*-------------------------------------------------------------------
    Local: ScaleBytes()

    This function scales a number from bytes up to KB, MB or GB as
    appropriate.  If the size is scaled, the value is formatted
    as a string in the buffer given using the form " (nnn MB)".
    If the size cannot be scaled, the buffer is set to an empty
    string.

    Parameters:
        ulSizeInKB  - the size to scale, in bytes
        pszBuffer   - the buffer to format the scaled size
        nBufferSize - the size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
static char * ScaleBytes(
    D_UINT32    ulByteValue,
    char       *pszBuffer,
    unsigned    nBufferSize)
{
    char       *pBuff;
    char        szBuffer[16];

    DclAssert(pszBuffer);
    DclAssert(nBufferSize);

    if(ulByteValue && !(ulByteValue % 1024))
    {
        pBuff = DclScaleKB(ulByteValue/1024, szBuffer, sizeof(szBuffer));
        if(*pBuff != 0)
        {
            /*  If the second to last digit is <not> a 'K', then the number
                was scaled, and we want to finish formatting it.  If the
                number was not scaled, then we just want to return an empty
                string.
            */
            if(szBuffer[DclStrLen(szBuffer)-2] != 'K')
            {
                /*  Format the scaled value into the client buffer
                */
                if(DclSNPrintf(pszBuffer, nBufferSize, " (%s)", &szBuffer[0]) != -1)
                    return pszBuffer;
            }
        }
    }

    /*  Either the number was not evenly scalable to MB or GB, or we had
        buffer overflow.  In either case, just return an empty string.
    */
    *pszBuffer = 0;

    return pszBuffer;
}


/*-------------------------------------------------------------------
    Local: ScalePages()

    This function scales a number of pages into KB, MB, or GB as
    appropriate.  The scaled value is formatted into the given
    string buffer.  If the size cannot be scaled the buffer is
    set to an empty string.

    Parameters:
        ulPages     - the number of pages to scale
        ulPageSize  - the size of each page, in bytes
        pszBuffer   - the buffer to format the scaled size
        nBufferSize - the size of the output buffer

    Return Value:
        Returns pszBuffer.
-------------------------------------------------------------------*/
static char * ScalePages(
    D_UINT32        ulPages,
    D_UINT32        ulPageSize,
    char           *pszBuffer,
    unsigned        nBufferSize)
{
    D_UINT32        ulSizeInKB;
    D_UINT32        ulPagesPerKB;

    /*  In the event of an early exit...
    */
    DclStrCpy(pszBuffer, "");

    if(!DCLISPOWEROF2(ulPageSize))
    {
        DclError();
        return pszBuffer;
    }

    /*  Calculate the size in KB.
    */
    ulPagesPerKB = ulPageSize / 1024;
    if(ulPagesPerKB)
        ulSizeInKB = ulPages * ulPagesPerKB;
    else
        ulSizeInKB = ulPages / (1024 / ulPageSize);

    /*  Attempt to scale the KB size into MB or GB.  If that
        fails, just display the size as KB.
    */
    ScaleKB(ulSizeInKB, pszBuffer, nBufferSize);
    if(!(*pszBuffer))
    {
        /*  Format the string with the size scaled to KB.
        */
        if(DclSNPrintf(pszBuffer, nBufferSize, " (%lU KB)", ulSizeInKB) == -1)
        {
            /*  Buffer size was too small, just output an empty string.
            */
            DclStrCpy(pszBuffer, "");
            return pszBuffer;
        }
    }

    return pszBuffer;
}

