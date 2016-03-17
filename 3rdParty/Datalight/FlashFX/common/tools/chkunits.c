/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
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

    Checks a flash disk spare units.

    ToDo:
      - This utility should be fleshed out into something that really
        examines and verifies the VBF data structures in detail.
      - Modify this utility to return standard status values.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: chkunits.c $
    Revision 1.16  2009/07/21 21:31:21Z  garyp
    Merged from the v4.0 branch.  Updated to use the revised FfxSignOn()
    function, which now takes an fQuiet parameter.  Documentation updated.
    Revision 1.15  2009/04/01 14:38:39Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.14  2009/03/09 19:36:29Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.13  2009/02/09 01:35:40Z  garyp
    Merged from the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.12  2008/02/06 20:24:19Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.11  2008/01/13 07:26:57Z  keithg
    Function header updates to support autodoc.
    Revision 1.10  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.9  2007/08/31 22:56:11Z  pauli
    Resolved Bug 1267: Replaced byte addressing with pages and blocks.
    Revision 1.8  2007/08/16 04:55:51Z  garyp
    Modified to never return negative values.
    Revision 1.7  2006/10/06 00:59:01Z  Garyp
    Modified to no longer use FfxFmlOldErase().
    Revision 1.6  2006/03/12 20:52:13Z  Garyp
    Updated to work with any standard page size.
    Revision 1.5  2006/03/05 01:57:28Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.4  2006/02/21 02:02:39Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.3  2006/02/12 18:48:22Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a
    single device/disk at a time.
    Revision 1.2  2006/01/31 06:54:34Z  Garyp
    Updated to use the new FML interface.
    Revision 1.1  2005/10/25 03:15:56Z  Pauli
    Initial revision
    Revision 1.3  2005/10/25 04:15:56Z  Garyp
    General cleanup.  Removed some dead code.  No functional changes.
    Revision 1.2  2005/10/20 03:27:23Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/02 03:24:24Z  Garyp
    Initial revision
    Revision 1.16  2004/12/30 23:00:15Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.15  2004/01/03 18:09:20Z  garys
    Merge from FlashFXMT
    Revision 1.12.1.3  2004/01/03 18:09:20  garyp
    Changed to use FfxStrICmp().
    Revision 1.12.1.2  2003/11/03 04:48:36Z  garyp
    Re-checked into variant sandbox.
    Revision 1.13  2003/11/03 04:48:36Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.12  2003/07/01 17:40:50Z  garyp
    Fixed the return values for several functions to work properly with the
    Green Hills tools.
    Revision 1.11  2003/05/22 21:42:58Z  brandont
    Updated the FXCHK common code to work with the new code base.
    Revision 1.10  2003/04/16 16:11:35  billr
    Merge from VBF4 branch.
    Revision 1.9  2003/04/08 21:52:02Z  garyp
    Updated to use FFX internal string routines.  Eliminated use of __D_FAR
    where possible.
    Revision 1.8  2003/03/27 23:30:18Z  garyp
    Fixed some command-line parsing problems.
    Revision 1.7  2003/03/24 06:04:18Z  garyp
    Eliminated all OESL specific code, along with any code related to
    the external API or oem/vbf initialization.  Revamped to use the new
    TOOLS and TESTS framework to deal with these issues.
    Revision 1.6.1.2  2003/02/21 20:44:09Z  dennis
    Changed the in-memory Allocation struct definition and
    changes to the MM interface
    Revision 1.6  2002/12/01 00:11:34Z  dennis
    Removed NVBF dependencies from FMSL and tools.  Added defines to OEM.H for
    the data, redundant and page size of NAND (MGM) memories in the max and
    standard cases.  For now, the two cases are identical.
    Revision 1.5  2002/11/28 00:40:36  garys
    modified to support NVBF
    Revision 1.4  2002/11/26 15:18:44  jimp
    Changed uArgCount check to look for only one argument, added comments.
    Added $ Log tag to file.
    11/29/99 PKG /TWQ Corrected RCS version information - RCS had
                 incorrectly branched from a previous version.
    11/03/99 TWQ Update with Linux specific includes;
    02/09/99 HDS Flagged all printfs to make them easier to find
                 for modification.
    01/28/99 HDS Modified test patterns to prevent attempts to
                 program 0's to 1's.
    10/29/98 HDS Created this flash disk check tool.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>

static D_BOOL   VerifyUnit(FFXFMLHANDLE hFML, D_UINT32 ulAddress, D_UINT32 ulLength, D_UCHAR ucValue);
static D_BOOL   StressUnit(FFXFMLHANDLE hFML, D_UINT32 ulPhysicalAddress, D_UINT32 ulUnitSize);
static D_BOOL   IdentifyandTestSpareUnit(VBFHANDLE hVBF);
static D_INT16  CheckSpareUnits(VBFHANDLE hVBF);
static void     ShowUsage(FFXTOOLPARAMS *pTP);

#define ARGBUFFLEN   128

#endif

/*-------------------------------------------------------------------
    Protected: FfxCheckUnits()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxCheckUnits(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT
    D_INT16         iErrorLevel;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;
    VBFHANDLE       hVBF;

    DclPrintf("\nFlashFX Check Units\n");
    FfxSignOn(FALSE);

    uArgCount = DclArgCount(pTP->dtp.pszCmdLine);

    if(uArgCount)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, 1, ARGBUFFLEN, achArgBuff))
        {
            DclError();
            return 3;
        }

        if(DclStrICmp(achArgBuff, "/?") == 0)
        {
            ShowUsage(pTP);

            return 1;
        }

        DclPrintf("Invalid argument '%s'\n", (const char *) achArgBuff);

        return 2;
    }

    hVBF = FfxVbfHandle(pTP->nDiskNum);
    if(!hVBF)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    /*  If resetting the disk fails, then return a non-zero error code to
        the caller and print a message out to the user!
    */
    iErrorLevel = CheckSpareUnits(hVBF);
    if(iErrorLevel != 0)
    {
        DclPrintf("\n\n  Flash Disk Check Failed!\n");

        return iErrorLevel;
    }

    DclPrintf("\n\n  Flash Disk Check Completed\n");

    /*  Return error code 0 to the caller - this means no errors occured
    */
    return 0;

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ShowUsage()

    Parameters:

    Return Value:
	    None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nThis utility checks for bad spare units\n\n");
    DclPrintf("Usage: %s drive\n", pTP->dtp.pszCmdName);
    DclPrintf("  drive  The drive designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  /?     This help information\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    DclPrintf("Returns an errorlevel - number of bad units.\n");

    return;
}


/*-------------------------------------------------------------------
    Local: VerifyUnit()

    Reads data from the flash array and compares it with
    the given value.  Used only by StressUnit().

    Parameters:
        hFML            - A handle denoting the FML instance to use
        ulBlock         - Erase block number of the unit to stress to
                          determine if bad.
        ulBlocksPerUnit - Number of blocks in a unit.
        ucValue         - Value to verify the data against

    Return Value:
        TRUE  - If all bytes are the same as the given value
        FALSE - If any bytes are different from the given value
-------------------------------------------------------------------*/
static D_BOOL VerifyUnit(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulBlock,
    D_UINT32        ulBlocksPerUnit,
    D_UCHAR         ucValue)
{
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPagesPerBlock;
    D_UINT32        ulTotalPages;
    D_UINT32        ulPage;

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    /*  Read and verify all pages in the unit.
    */
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPage = ulBlock * ulPagesPerBlock;
    ulTotalPages = ulBlocksPerUnit * ulPagesPerBlock;
    while(ulTotalPages)
    {
        DCLALIGNEDBUFFER    (page, data, FFX_MAX_PAGESIZE);
        FFXIOSTATUS         ioStat;
        D_UINT16            uIndex;

        FMLREAD_PAGES(hFML, ulPage, 1, page.data, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;

        /*  Compare each byte
        */
        for(uIndex = 0; uIndex < FmlInfo.uPageSize; ++uIndex)
        {
            if(page.data[uIndex] != ucValue)
                return FALSE;
        }

        ulTotalPages--;
        ulPage++;
    }

    /*  All pages are equal
    */
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: StressUnit()

    Provides a method to stress a unit to help determine
    if it is bad.

    Parameters:
        hFML             - A handle denoting the FML to use.
        ulBlock          - Erase block number of the unit to stress to
                           determine if bad.
        ulBlocksPerUnit  - Number of blocks in a unit.

    Return Value:
        TRUE  - If the unit is good (unit is now erased)
        FALSE - If any errors are found during the access
-------------------------------------------------------------------*/
#define TEST_BYTE 0x55
static D_BOOL StressUnit(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulBlock,
    D_UINT32        ulBlocksPerUnit)
{
    FFXFMLINFO      FmlInfo;
    DCLALIGNEDBUFFER (page, data, FFX_MAX_PAGESIZE);
    FFXIOSTATUS     ioStat;
    D_UINT32        ulPage;
    D_UINT32        ulPagesPerBlock;
    D_UINT32        u;

    DclAssert(hFML);

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    DclAssert(ulBlock < FmlInfo.ulTotalBlocks);
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;

    /*  Erase unit and verify erase
    */
    FMLERASE_BLOCKS(hFML, ulBlock, ulBlocksPerUnit, ioStat);
    if(!IOSUCCESS(ioStat, ulBlocksPerUnit))
        return FALSE;

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, ERASED8))
        return FALSE;

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Write test pattern (0x55) and verify
    */
    DclMemSet(page.data, TEST_BYTE, FmlInfo.uPageSize);
    ulPage = ulBlock * ulPagesPerBlock;
    for(u = 0; u < (ulBlocksPerUnit * ulPagesPerBlock); u++)
    {
        FMLWRITE_PAGES(hFML, ulPage + u, 1, page.data, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;
    }

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, TEST_BYTE))
        return FALSE;

    /*  If this is NAND flash, bail out now.  The rest of the stress
        routine will attempt to overwrite previously written pages,
        which we do not allow for NAND.
    */
    if(FmlInfo.uDeviceType == DEVTYPE_NAND)
    {
        return TRUE;
    }

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Write zero to program the remaining bits (0xAA) to zero
        and verify.
    */
    DclMemSet(page.data, 0x00, FmlInfo.uPageSize);
    ulPage = ulBlock * ulPagesPerBlock;
    for(u = 0; u < (ulBlocksPerUnit * ulPagesPerBlock); u++)
    {
        FMLWRITE_PAGES(hFML, ulPage + u, 1, page.data, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;
    }

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, (D_UCHAR)~ERASED8))
        return FALSE;

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Erase unit and verify erase
    */
    FMLERASE_BLOCKS(hFML, ulBlock, ulBlocksPerUnit, ioStat);
    if(!IOSUCCESS(ioStat, ulBlocksPerUnit))
        return FALSE;

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, ERASED8))
        return FALSE;

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Write NOT test pattern (0xAA) and verify
    */
    DclMemSet(page.data, ~TEST_BYTE, FmlInfo.uPageSize);
    ulPage = ulBlock * ulPagesPerBlock;
    for(u = 0; u < (ulBlocksPerUnit * ulPagesPerBlock); u++)
    {
        FMLWRITE_PAGES(hFML, ulPage + u, 1, page.data, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;
    }

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, (D_UCHAR)~TEST_BYTE))
        return FALSE;

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Write zero to program the remaining bits (0x55) to zero
        and verify.
    */
    DclMemSet(page.data, 0x00, FmlInfo.uPageSize);
    ulPage = ulBlock * ulPagesPerBlock;
    for(u = 0; u < (ulBlocksPerUnit * ulPagesPerBlock); u++)
    {
        FMLWRITE_PAGES(hFML, ulPage + u, 1, page.data, ioStat);
        if(!IOSUCCESS(ioStat, 1))
            return FALSE;
    }

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, (D_UCHAR)~ERASED8))
        return FALSE;

    /*  Show testing active
    */
    DclPrintf(" -");

    /*  Erase unit and verify erase
    */
    FMLERASE_BLOCKS(hFML, ulBlock, ulBlocksPerUnit, ioStat);
    if(!IOSUCCESS(ioStat, ulBlocksPerUnit))
        return FALSE;

    if(!VerifyUnit(hFML, ulBlock, ulBlocksPerUnit, ERASED8))
        return FALSE;

    /*  The unit is good
    */
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: IdentifyandTestSpareUnit()

    This function identifies all spare units and performs stress
    testing on each one to determine if it is good or bad.

    Parameters:
        hFML - A handle denoting the FML handle to use
        hVBF - A handle denoting the VBF instance to use

    Return Value:
        TRUE  - operation successful, and all units are good
        FALSE - operation failed, or bad units found
-------------------------------------------------------------------*/
static D_BOOL IdentifyandTestSpareUnit(
    VBFHANDLE       hVBF)
{
    VBFDISKINFO     DiskInfo;
    VBFDISKMETRICS  DiskMets;
    VBFUNITMETRICS  UnitInfo;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulNumberGoodUnits;
    D_UINT32        ulNumberBadUnits;
    D_UINT32        ulNumberSpareUnits;
    D_UINT32        ulPagesPerBlock;
    D_UINT32        ulBlocksPerUnit;
    D_UINT32        ulBlock;
    D_UINT32        ulUnit;
    D_BOOL          fTestPassed;

    /*  Set the intial values for the counter variables
    */
    ulNumberGoodUnits = 0;
    ulNumberBadUnits = 0;
    ulNumberSpareUnits = 0;

    /*  Get the information about this disk
    */
    if(FfxVbfDiskInfo(hVBF, &DiskInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxVbfDiskMetrics(hVBF, &DiskMets) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxFmlDiskInfo(DiskInfo.hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulBlocksPerUnit = DiskMets.uUnitTotalPages / ulPagesPerBlock;

    DclPrintf("\nTESTING SPARE UNITS\n");

    /*  Iterate through all the units on this partition
    */
    for(ulUnit = 0; ulUnit < DiskMets.ulTotalUnits; ulUnit++)
    {
        /*  Set the unit number that we want more information about
            in the UnitInfo structure.  UnitInfo.lnu is the unit number
            relative to the starting unit of the partition.
        */
        UnitInfo.lnu = ulUnit;

        /*  Get the information about this unit.
        */
        if(FfxVbfUnitMetrics(hVBF, &UnitInfo) != FFXSTAT_SUCCESS)
        {
            /*  The information about this unit is invalid
            */
            continue;
        }

        /*  Establish the erase block number for the unit to examine.
        */
        ulBlock = ulUnit * ulBlocksPerUnit;

        /*  Determine if this is a spare unit
        */
        if(UnitInfo.bSpareUnit)
        {
            /*  Inform user of spare unit being tested
            */
            ulNumberSpareUnits++;
            DclPrintf("  Spare Unit %2lU @ Block %lU", ulNumberSpareUnits, ulBlock);

            /*  Stress test this unit
            */
            fTestPassed = StressUnit(DiskInfo.hFML, ulBlock, ulBlocksPerUnit);

            /*  Display the results of the stress test for this unit
            */
            if(fTestPassed)
            {
                /*  Inform user that spare unit is good
                */
                ulNumberGoodUnits++;
                DclPrintf(" GOOD\n");
            }
            else
            {
                /*  Inform user that spare unit is bad
                */
                ulNumberBadUnits++;
                DclPrintf(" BAD\n");
            }
        }
    }

    /*  Display results
    */
    DclPrintf("\nUNIT STATUS SUMMARY\n");
    DclPrintf("  Total Number of Units- - - - - - - - - %lU\n", DiskMets.ulTotalUnits);
    DclPrintf("  Number of Spare Units- - - - - - - - - %lU\n", ulNumberSpareUnits);
    DclPrintf("  Number of GOOD Units - - - - - - - - - %lU\n", ulNumberGoodUnits);
    DclPrintf("  Number of BAD Units- - - - - - - - - - %lU\n", ulNumberBadUnits);

    /*  Return FALSE if bad units found
    */
    if(ulNumberBadUnits)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: CheckSpareUnits()

    This function checks for the spare units on a flash disk and
    determines whether they are good or bad.

    Parameters:
        hVBF - A handle denoting the VBF instance to use

    Return Value:
        This function will return an error level of 0, if successful.
        It will return an error level of non-zero, if an error occurs.
-------------------------------------------------------------------*/
static D_INT16 CheckSpareUnits(
    VBFHANDLE       hVBF)
{
    if(!IdentifyandTestSpareUnit(hVBF))
        return 4;              /* Failed */
    else
        return 0;
}



#endif  /* FFXCONF_VBFSUPPORT */



