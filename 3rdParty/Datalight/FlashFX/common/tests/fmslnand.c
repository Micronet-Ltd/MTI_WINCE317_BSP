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

    FMSL tests specific to NAND flash.  These tests are designed to
    fill some gaps in the FMSL tests, which completely omitted any
    testing of some NAND FIM functions that are used by VBF, and did
    not necessarily cover all code paths.

    As of their introduction, these tests significantly improve test
    coverage but are not necessarily a complete functional test to the
    published specifications.

    All tests first erase the entire area to be written, then do all
    their writing, then read and verify.  This makes it more likely
    that any writes that exceed the specified length will be detected.

    Erase operations are assumed to have been adequately tested
    elsewhere, and thus erased areas are not verified here.

    The data pattern written is designed to be unique in every block
    written.

    The area of the flash to be used for testing is chosen
    pseudorandomly to provide some wear leveling when the test is
    repeated.  This feature needs to be integrated with the rest of
    the FMSL test framework so that a failing test can be recreated
    beginning with the same location.

    Because BBM and the NAND FIM are so intimately related, all tests
    avoid explicitly accessing the BBM area of the flash.  This could
    be considered a testing deficiency, but it's not easy to correct
    given the current architecture.

    There should probably be a concept of an "extensive" version of
    the tests.  The interface provides for this but it is currently
    not implemented.

    One other sort of testing that probably should be implemented is
    one which crosses "significant" boundaries in the chip or flash
    array (like power of 2 fractions).  Some chips have known errata
    involving such boundaries.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmslnand.c $
    Revision 1.86  2011/11/16 20:03:03Z  daniel.lewis
    Eliminated use of -1 for DclSNPrintf() buffer sizes, using the real
    buffer size instead.
    Revision 1.85  2010/12/16 23:55:40Z  glenns
    Fixed to disallow use of BBM-remapped blocks for any tests,
    even those that don't require raw access. Related to bug
    3280.
    Revision 1.84  2010/12/15 04:16:02Z  glenns
    Temporarily disabled certain of the GET_PAGE_INFO tests
    pending determination of their validity.
    Revision 1.83  2010/09/27 22:08:53Z  glenns
    Fix typo in revision comment for last checkin.
    Revision 1.82  2010/09/24 22:17:03Z  glenns
    Fix bug 3276- repair raw page test to appropriately obtain and use
    a raw block number, and update FfxFmslNANDFindTestBlocks to
    be sure there are no remappings or BBM tables in the test block
    range.
    Revision 1.81  2010/09/23 06:04:16Z  garyp
    Fixed BlockInfoTest() to work for Disks which span multiple Devices.
    Minor documentation cleanup.
    Revision 1.80  2010/09/16 18:17:45Z  glenns
    Fix incorrect "fRaw" boolean parameter in call to ValidatePageStatus() from
    RawPageTest.  Error was causing the wrong physical page to be validated in
    multi-chip configurations with BBM turned on if the page was not in chip 0,
    since BBM tables are accounted for in non-raw physical page mappings.
    Revision 1.79  2010/08/05 21:16:46Z  glenns
    Add code to skip NativePage EDC test for NAND devices with transparent EDC.
    Add code to reduce number of tags tested for devices with large erase blocks.
    Revision 1.78  2010/07/07 19:20:44Z  garyp
    Added an option to run BBM tests.  Removed a page buffer which was
    allocated on the stack.  Fixed some Hungarian notation.
    Revision 1.77  2010/07/05 22:03:06Z  garyp
    Test failure messages updated to be more useful.
    Revision 1.76  2010/04/28 23:34:21Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.75  2010/03/02 18:44:27Z  glenns
    Fix Bug 2630: Apply PXA320_HACK more appropriately.
    Revision 1.74  2010/01/22 23:14:59Z  billr
    Fix alignment of tag buffers.
    Revision 1.73  2010/01/10 20:51:51Z  garyp
    Simplified some error handling logic -- no functional changes.
    Revision 1.72  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.71  2009/12/21 17:44:43Z  billr
    Fixed swapped length and value to DclMemSet() (thank you, gcc!).
    Revision 1.70  2009/10/31 19:43:49Z  glenns
    Fix Bugzilla 2881: Modified declaration in NativeOpsTest to ensure alignment
    of "tags" buffer. Also made minor update to an error message.
    Revision 1.69  2009/08/04 01:43:40Z  garyp
    Merged from the v4.0 branch.  Minor datatype changes from D_UINT16 to
    unsigned.  Updated the read and write performance tests to operate on a
    specified count of sectors, to allow iterative testing of varying sizes.
    Changed all uses of FMLREAD_CORRECTEDPAGES() to FMLREAD_PAGES(), and
    all uses of FMLREAD/WRITE_PAGES() to FMLREAD/WRITE_UNCORRECTEDPAGES().
    Removed the repeated re-allocation of the "BigBlockBuffer" and use the
    buffer supplied by the callers.
    Revision 1.68  2009/04/22 04:36:14Z  keithg
    Fixed bug 2648 regarding uninitialized variable and resulting memory leak.
    Revision 1.67  2009/04/17 21:39:18Z  keithg
    Fixed bug 2620 to properly handle MLC spare area sizes.
    Revision 1.66  2009/04/17 18:50:35Z  keithg
    Fixed bug 2632, build failures when BBMSUPPORT is disabled.
    Revision 1.65  2009/04/09 02:58:26Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.64  2009/03/31 19:58:39Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.63  2009/03/26 21:37:44Z  glenns
    Temporarily disable TagTest 4a. See Bugzilla #2550.  Fix logical errors in
    tag readback loop for TagTest 4.
    Revision 1.62  2009/03/18 19:40:18Z  glenns
    Fix minor "variable may be used uninitiallized" warning in BlockInfoTest.
    Revision 1.61  2009/03/18 07:59:10Z  keithg
    Bug 2347 fixed, the block status was not properly masked before tests.  Bug
    2642 fixed, blocks and pages are now properly mapped to raw blocks.  This
    was causing block info and ECC failures on pages within the second chip.
    Revision 1.60  2009/03/09 19:50:58Z  glenns
    Modified TagTest to account for device/controller combinations (such as
    FlexOneNAND) that cannot distinguish between bit errors in the main page
    and spare area when performing corrections.  Modified TagTest to account
    for correctable errors returning flow of control to the test before all
    the tags being tested have been read.
    Revision 1.59  2009/03/04 06:32:59Z  keithg
    Changed retire block test to handle the appropriate return count;  The
    RETIRE_RAWBLOCK function now returns zero or one.
    Revision 1.58  2009/02/19 18:21:42Z  glenns
    Modified RetireBlockTest to supply a dummy tag and use FMLWRITE_TAGGEDPAGES
    so that pages written for the test contain bit error correction information.
    Fixes bug #2389.
    Revision 1.57  2009/02/17 06:13:31Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.56  2009/02/15 21:06:16Z  michaelm
    fixed build errors in the case FFXCONF_BBMSUPPORT is FALSE
    Revision 1.55  2009/02/09 22:09:59Z  glenns
    Added code to bypass the tag-overwrite test if device flags indicate that
    the device is not overwritable.
    Revision 1.54  2009/02/06 22:15:50Z  michaelm
    Now allow FMLREAD_NATIVEPAGES() to return FFXSTAT_FIMUNCORRECTED.
    Revision 1.53  2009/02/06 05:21:00Z  michaelm
    Renamed Read_Pages() to FfxFmslTestReadPages() and moved function
    description to fmsltst.c.  Added basic tests to NativeOpsTest()
    Revision 1.52  2009/02/03 15:45:43Z  michaelm
    Moved fxfmlapi.h include into fmsltst.h
    Revision 1.51  2009/02/02 23:58:35Z  michaelm
    Moved Read_Pages into fmsltst.h.  Now using Read_Pages() so test will
    function normally in the case of a correctable read error.
    Revision 1.50  2009/01/23 00:12:02Z  michaelm
    Removed checks for 'written with ECC' in TagTest.  ValidatePageStatus now
    handles typical (not raw) pages properly.  RawPageTest now erasing raw
    blocks.  RetireBlockTest now bounded by free BBM blocks.  Fixed memory
    leak in RetireBlockTest
    Revision 1.49  2009/01/18 08:52:45Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.48  2009/01/16 21:12:41Z  michaelm
    removed BBMv4-specific code
    Revision 1.47  2009/01/14 23:05:58Z  deanw
    Removed use of global buffers. Functions now use dynamic memory allocation.
    Revision 1.46  2009/01/14 18:55:15Z  michaelm
    Replaced tabs with spaces and added a MemSet to RetireBlockTest to ensure
    that our buffer changes between a write and a subsequent read.
    Revision 1.45  2009/01/12 22:18:06Z  michaelm
    RetireBlockTest now copies up to PagesPerBlock pages.
    Revision 1.44  2009/01/06 22:37:37Z  michaelm
    added command-line switch /R to determine whether to run RetireBlockTest
    Revision 1.42  2009/01/02 21:12:38Z  michaelm
    Added additional test loop for last 32 blocks in BlockInfoTest and added
    placeholders for RetireBlockTest and NativeOpsTest.
    Revision 1.41  2008/12/31 21:09:32Z  michaelm
    Added boundary and error tests and a BBMv5 conditional to block info tests.
    Revision 1.40  2008/12/24 04:43:31Z  michaelm
    Added basic functionality testing for FML_GET_BLOCK_INFO()
    Revision 1.39  2008/10/21 00:01:32Z  keithg
    Conditioned page and block status testing one BBM v5 until
    new tests are implemented.
    Revision 1.38  2008/07/23 05:59:43Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.37  2008/05/09 03:18:44Z  garyp
    Fixed problems with the previous checkin -- use a valid buffer.
    Revision 1.36  2008/05/08 21:06:08Z  garyp
    Udpated TagTest() test #3 to write the tagged pages all as one operation
    to avoid tag buffer alignment issues if the pages are written individually.
    Revision 1.35  2008/05/07 23:03:37Z  garyp
    Function header and general code cleanup -- no functional changes.
    Revision 1.34  2008/05/03 03:25:03Z  garyp
    Corrected to compile cleanly when BBM support is disabled.
    Revision 1.33  2008/05/01 16:58:45Z  Glenns
    Fixed a minor logical inversion at the end of the RawPageTest. Modified the
    TagTest to mor accurately model how VBF uses tags, and the results that VBF
    expects to see.
    Revision 1.32  2008/03/24 20:55:09Z  garyp
    Fixed the modifications in the previous revision so that the blocks are
    erased even when error code paths are taken.  Added tests for the
    GetPageStatus() interface.  Added a test to ensure that tags read from
    erased pages return all 0xFFs.  Added a test to ensure that tags values
    which are all 0xFFs or within 1 bit thereof (in any bit position) return the
    correct tag values.  Modified the test to exercise all possible tag sizes
    if the NTM supports variable length tags.  Enhanced the BlockStatusTest()
    to test more conditions.  Enhanced the error handling and debug messages
    to be more useful.
    Revision 1.31  2008/01/30 21:21:29Z  Glenns
    Added fix for Bugzilla #1759, in which RawPage test was causing subsequent
    initialization of FlashFX to detect additional bad blocks.
    Revision 1.30  2008/01/13 07:26:50Z  keithg
    Function header updates to support autodoc.
    Revision 1.29  2007/11/07 22:38:59Z  pauli
    Added test cases for FMLREAD_TAGGEDPAGES.
    Revision 1.28  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.27  2007/10/22 22:50:02Z  pauli
    Moved ECC tests to fmslecc.c.  Updated output formatting and added verbose
    levels.
    Revision 1.26  2007/10/16 00:39:21Z  pauli
    Expanded the raw page tests to include cases for main data only and main
    + spare data.
    Revision 1.25  2007/10/01 22:17:06Z  pauli
    Updated format specifiers.  Skipped CommonTest if not enough blocks are
    available.  Enhanced TagTest to be smarter about how it calculates the
    number of tags to test.
    Revision 1.24  2007/09/13 22:37:55Z  pauli
    Renamed TESTINFO to FMSLNANDTESTINFO.
    Revision 1.23  2007/09/12 21:37:43Z  Garyp
    General cleanup of the test messages -- no functional changes.
    Revision 1.22  2007/08/21 21:38:37Z  pauli
    Corrected the usage of the ECCTESTINFO structure in the ECC tests so that
    it is passed as a pointer and the data is global to all the ECC tests.
    Changed BlockStatusTest to only test all blocks if extensive mode is
    enabled.
    Revision 1.21  2007/08/21 05:03:04Z  garyp
    Fixed problems in the ECC test which would result in asserts, endless
    loops, and other silliness if the NTM does not support raw access.
    Revision 1.20  2007/08/01 00:00:51Z  timothyj
    Fixed block-spanning loop to span the intended blocks only (previously
    also spanned into the next block, which the test had not erased and
    therefore may contain data).
    Revision 1.19  2007/07/26 23:55:27Z  billr
    Make this compile when D_DEBUG <= 1.  Some compilers don't like static
    functions defined but not referenced.
    Revision 1.18  2007/07/18 23:55:37Z  rickc
    Added new tests for raw page read/write, block status, and ECC correction/
    detection.  Updated for LFA.
    Revision 1.17  2007/06/20 22:39:33Z  timothyj
    Changed tag and page buffer size to depend on the flash characteristics.
    (Ported fix for BZ #1194 from V310_Release branch to trunk.)
    Revision 1.16  2007/03/12 23:35:01Z  rickc
    Fixed to output only one page of output.
    Revision 1.15  2007/03/05 19:08:35Z  Garyp
    Added NAND specific FMSL performance tests which measure page reads and
    write using ECC.
    Revision 1.14  2007/02/20 17:53:38Z  Garyp
    Documentation updates.
    Revision 1.13  2006/12/29 18:37:20Z  Garyp
    Minor type changes -- nothing functional.
    Revision 1.12  2006/10/20 16:33:32Z  Garyp
    Fixed a logic error in PageVerify().
    Revision 1.11  2006/10/03 23:53:11Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.10  2006/05/18 23:21:04Z  Garyp
    Updated to include fmsltst.h.  Miscellaneous minor documentation and debug
    code fixes -- nothing functional.
    Revision 1.9  2006/05/12 18:32:22Z  Garyp
    Moved the hidden tests from fmsltst.c into this module where they belong.
    Changed to using the new FML interface macros, rather than the old
    compatibility mode API.  Modified to verbosely display when corrected
    single bit errors are encountered.
    Revision 1.8  2006/03/16 11:43:24Z  Garyp
    Updated debugging code.
    Revision 1.7  2006/03/05 01:57:23Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.6  2006/02/27 05:27:29Z  Garyp
    Updated to work with the new definition of a NAND tag.
    Revision 1.5  2006/02/15 09:23:01Z  Garyp
    Eliminated FfxFmlOldRead/Write() support for MEM_HIDDEN.
    Revision 1.4  2006/02/09 22:16:12Z  Garyp
    Updated to use the new FML interface.
    Revision 1.3  2006/01/25 03:50:48Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.2  2006/01/11 02:42:38Z  Garyp
    Added debugging code.
    Revision 1.1  2005/11/25 23:02:12Z  Pauli
    Initial revision
    Revision 1.3  2005/11/25 23:02:12Z  Garyp
    Modified to use DclRand().
    Revision 1.2  2005/11/15 14:19:46Z  Garyp
    Fixed to work properly with 2KB pages.  Fixed a bug with regard to the
    minimum number of blocks necessary to run the test.  Cleaned up the
    messages.
    Revision 1.1  2005/11/07 15:14:28Z  Garyp
    Initial revision
    Revision 1.8  2005/10/14 17:41:53Z  billr
    Fix compiler warning in some environments.
    Revision 1.7  2005/10/13 20:23:36Z  brandont
    Corrected signed/unsigned comparison warning.
    Revision 1.6  2005/10/13 00:15:06Z  billr
    Add MEM_NAND_PAGE write test.
    Revision 1.5  2005/10/10 19:06:34Z  Pauli
    fixed compile issue for ads toolset
    Revision 1.4  2005/09/22 19:11:27Z  billr
    Fix compiler warnings in some environments.
    Revision 1.3  2005/09/22 15:55:36Z  billr
    Fix warning about signed/unsigned mismatch.
    Revision 1.2  2005/09/21 17:14:36Z  billr
    Make FMSLNAND_MAX_NAME_LENGTH private to this module.
    Revision 1.1  2005/09/20 23:32:16Z  billr
    Initial revision
    ----------------------------------------
    Bill Roman and Thomas Denholm 2005-09-12
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxtrace.h>
#include "fmsltst.h"
#include <fxstats.h>

#if FFXCONF_NANDSUPPORT
#include "fmslnand.h"

/*  The PXA320 DFC has an apparent hardware defect which will cause this
    test to fail, unless PXA320_HACK is set to TRUE.  This will cause an
    extra read to be performed, which will clear a previous error single-
    bit error state.  This flaw affects FMSLTEST due to the sequence of
    operations, but not the general operation of FlashFX (generally...).
*/
#ifndef PXA320_HACK
  #define PXA320_HACK     FALSE
#endif

#define FMSLNAND_MAX_NAME_LENGTH   (40)
#define FMSLNAND_BUFFER_PAGES       (3)

static D_BOOL   CommonTest(     FMSLNANDTESTINFO *pTI);
static D_BOOL   TagTestMain(    FMSLNANDTESTINFO *pTI);
static D_BOOL   TagTest(        FMSLNANDTESTINFO *pTI, unsigned nTagSize);
static D_BOOL   TaggedPageTest( FMSLNANDTESTINFO *pTI);
static D_BOOL   SpareAreaTest(  FMSLNANDTESTINFO *pTI);
static void     PagePattern(    FMSLNANDTESTINFO *pTI, D_BUFFER *pBuffer, D_UINT32 ulSerial, D_UINT32 ulStartWritePage, const char *pszTestName, D_UINT32 ulPage, D_UINT32 ulPageCount);
static D_BOOL   SingleBitError( D_UINT16 uLength, const D_BUFFER *pBuffer, const D_BUFFER *pRef);
static void     VerifyError(    FMSLNANDTESTINFO *pTI, const D_BUFFER *pBuffer, const D_BUFFER *pRef);
static D_BOOL   BlankVerify(    FMSLNANDTESTINFO *pTI, D_BUFFER *pBuffer, unsigned int uBufferSize, D_UINT32 ulPage, const char *pszTestName, PAGE_VERIFY_MODE mode);
static D_BOOL   EraseAndVerify( FMSLNANDTESTINFO *pTI, D_UINT32 ulBlock, D_UINT32 ulCount);
static D_BOOL   RawPageTest(    FMSLNANDTESTINFO *pTI);
static D_BOOL   BlockStatusTest(FMSLNANDTESTINFO *pTI);
static D_BOOL   NativeOpsTest(  FMSLNANDTESTINFO *pTI);
#if FFXCONF_BBMSUPPORT
static D_BOOL   BlockInfoTest(  FMSLNANDTESTINFO *pTI);
static D_BOOL   RetireBlockTest(FMSLNANDTESTINFO *pTI);
static D_BOOL   ValidatePageStatus(FMSLNANDTESTINFO *pTI, D_UINT32 ulPage, D_UINT32 ulStatus, unsigned nTagSize, D_BOOL fRaw);
#endif

/*-------------------------------------------------------------------
    Private: FfxFmslNANDTest()

    Parameters:
       hFML
       fExtensive

    Return Value:
       TRUE if the tests passed, FALSE if any failed.
-------------------------------------------------------------------*/
D_BOOL FfxFmslNANDTest(
    FFXFMLHANDLE        hFML,
    D_UINT32            ulStartPage,
    D_BUFFER           *pBigBuffer,
    D_UINT32            ulBufferSize,
    D_UINT32           *pulRandomSeed,
    D_BOOL              fExtensive,
    D_BOOL              fRetireBlockTest,
    D_BOOL              fBBMTests,
    unsigned            nVerbosity)
{
    FMSLNANDTESTINFO    ti = {0};
    D_BOOL              fSuccess = FALSE;
    D_UINT32            ulPageBufferMinSize;
    D_UINT32            ulTagBufferMinSize;

    if(FfxFmlDiskInfo(hFML, &ti.FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxFmlDeviceInfo(ti.FmlInfo.nDeviceNum, &ti.FmlDevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    ti.hFML             = hFML;
    ti.ulStartPage      = ulStartPage;
    ti.pBigBuffer       = pBigBuffer;
    ti.ulBigBufferSize  = ulBufferSize;
    ti.fExtensive       = fExtensive;
    ti.pulRandomSeed    = pulRandomSeed;
    ti.nVerbosity       = nVerbosity;
    DclPrintf("NAND Flash Tests\n");

    /*  Compute the number of pages in one erase block
    */
    DclAssert((ti.FmlInfo.ulBlockSize % ti.FmlInfo.uPageSize) == 0);
    ti.ulPagesPerBlock = ti.FmlInfo.ulBlockSize / ti.FmlInfo.uPageSize;

    /*  Validate assumptions about the media and test parameters that
        are vital to the rest of the test code.  Some or all of these
        checks should really be done at the top level of the FMSL tests.
    */
    if (ti.FmlInfo.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("    ERROR: Flash is not NAND\n");
        return FALSE;
    }
    if (ulStartPage >= ti.FmlInfo.ulTotalBlocks * ti.ulPagesPerBlock)
    {
        DclPrintf("    ERROR: Starting page %lU >= total pages %lU\n",
                  ulStartPage, ti.FmlInfo.ulTotalBlocks * ti.ulPagesPerBlock);
        return FALSE;
    }
    if (ti.FmlInfo.ulBlockSize == 0)
    {
        DclPrintf("    ERROR: erase zone size is zero\n");
        return FALSE;
    }
    if (!DCLISPOWEROF2(ti.FmlInfo.ulBlockSize))
    {
        DclPrintf("    ERROR: erase zone size is not a power of 2\n");
        return FALSE;
    }

    /*  Compute the minimum buffer size for the page buffer
    */
    ulPageBufferMinSize = ti.FmlInfo.uPageSize * FMSLNAND_BUFFER_PAGES;

    /*  Compute the minimum buffer size for the tag buffers,
        which must be large enough to hold the number of tags in
        a block, because we erase an entire block for the tag test.
    */
    ulTagBufferMinSize = ti.ulPagesPerBlock * ti.FmlDevInfo.uMetaSize;

    /*  The page buffer (because it is also used as a tag buffer) needs
        to be big enough to hold the tag info for an entire block.  To
        facilitate 512B emulation on devices with 512KB pages (such as
        2KB page ORNAND), (which therefore have a large number of tags/
        block and consequently require a larger tag buffer size) we have
        to adjust this upward.
    */
    ti.ulBufferSize = DCLMAX(ulPageBufferMinSize, ulTagBufferMinSize);

    ti.pBuffer = DclMemAlloc(ti.ulBufferSize);
    if(!ti.pBuffer)
        goto Cleanup;

    ti.pBuffer2 = DclMemAlloc(ti.ulBufferSize);
    if(!ti.pBuffer2)
        goto Cleanup;

    ti.pSpareArea = DclMemAlloc(ti.FmlInfo.uSpareSize * FMSLNAND_BUFFER_PAGES);
    if(!ti.pSpareArea)
        goto Cleanup;

    ti.pSpareArea2 = DclMemAlloc(ti.FmlInfo.uSpareSize * FMSLNAND_BUFFER_PAGES);
    if(!ti.pSpareArea2)
        goto Cleanup;

    fSuccess = CommonTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = TagTestMain(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = TaggedPageTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = SpareAreaTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = RawPageTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = BlockStatusTest(&ti);
    if(!fSuccess)
        goto Cleanup;

  #if FFXCONF_BBMSUPPORT
    fSuccess = BlockInfoTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    /*  Warning!  RetireBlockTest can do serious damage to real flash hardare.
        Run this test only on simulated flash.
    */
    if(fRetireBlockTest)
    {
        fSuccess = RetireBlockTest(&ti);
        if(!fSuccess)
            goto Cleanup;
    }
  #endif

    fSuccess = NativeOpsTest(&ti);
    if(!fSuccess)
        goto Cleanup;

    fSuccess = FfxFmslNANDTestEcc(&ti);
    if(!fSuccess)
        goto Cleanup;

  #if FFXCONF_BBMSUPPORT
    if(fBBMTests)
    {
        fSuccess = FfxFmslNANDTestBBM(&ti);
        if(!fSuccess)
            goto Cleanup;
    }
  #endif

  Cleanup:

    /*  Release resources in reverse order of allocation
    */
    if(ti.pSpareArea2)
        DclMemFree(ti.pSpareArea2);

    if(ti.pSpareArea)
        DclMemFree(ti.pSpareArea);

    if(ti.pBuffer2)
        DclMemFree(ti.pBuffer2);

    if(ti.pBuffer)
        DclMemFree(ti.pBuffer);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Private: FfxFmslNANDTestReadPerform()

    This function determines the general FMSL read performance
    when reading corrected pages (NAND specific).

    Parameters:
        hFML         - The handle denoting the FML to use
        pBuffer      - A pointer to the working buffer to use
        ulBufferSize - The buffer size
        ulMaxPages   - The maximum number of pages to read
        ulSeconds    - The maximum number of seconds to run the test
        nCount       - The number of pages to write in each
                       operation.

    Return Value:
        Returns a KB/second read peformance value.
-------------------------------------------------------------------*/
D_UINT32 FfxFmslNANDTestReadPerform(
    FFXFMLHANDLE    hFML,
    D_BUFFER       *pBuffer,
    D_UINT32        ulBufferSize,
    D_UINT32        ulMaxPages,
    D_UINT32        ulSeconds,
    unsigned        nCount,
    unsigned        nVerbosity)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulPageNum = 0;
    D_UINT32        ulRemaining;
    D_UINT32        ulPagesRead = 0;
    FFXIOSTATUS     ioStat;
    DCLTIMER        tRead;
    FFXFMLINFO      FmlInfo;

    DclAssert(hFML);
    DclAssert(pBuffer);
    DclAssert(ulBufferSize);
    DclAssert(ulMaxPages);
    DclAssert(ulSeconds);
    DclAssert(nCount);

    /*  Get some information about the media
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return 0;

    DclAssert(nCount <= ulBufferSize / FmlInfo.uPageSize);

    /*  Initialize the timer for a certain number of milliseconds.
    */
    DclTimerSet(&tRead, ulSeconds * 1000);

    ulRemaining = ulMaxPages;

    while(ulRemaining && !DclTimerExpired(&tRead))
    {
        D_UINT32    ulCount = DCLMIN(ulRemaining, nCount);

        ioStat = FfxFmslTestReadPages(hFML, ulPageNum, ulCount, pBuffer, READ_PAGES);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("  FfxFmslNANDTestReadPerform() FMLREAD_PAGES() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }

        ulPageNum   += ioStat.ulCount;
        ulRemaining -= ioStat.ulCount;
        ulPagesRead += ioStat.ulCount;
    }

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tRead);

    if(nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Read %s in %lU ms\n", DclScaleBytes(ulPagesRead * FmlInfo.uPageSize, NULL, 0), ulDiffTime);

    /*  check for possible zero denominator
    */
    if(ulDiffTime == 0)
    {
        /*  How could this ever happen? -- In an emulation environment using
            a RAM based FIM, without any emulating of timing, it can...
        */
        ulDiffTime = 1;
    }

    /*  Return the number of kilobytes we read each second
    */
    return (((ulPagesRead * FmlInfo.uPageSize) / 1024) * 1000) / ulDiffTime;
}


/*-------------------------------------------------------------------
    Private: FfxFmslNANDTestWritePerform()

    This function determines the general FMSL write performance
    when writing tagged pages (NAND specific).

    Parameters:
        hFML            - The handle denoting the FML to use
        pBuffer         - A pointer to the working buffer to use
        ulBufferSize    - The buffer size
        pulPagesWritten - A pointer to a variable to receive the
                          number of pages written.
        ulSeconds       - The max number of seconds to run the test
        nCount          - The number of pages to read in each
                          operation.

    Return Value:
        Returns a KB/second write peformance value.
-------------------------------------------------------------------*/
D_UINT32 FfxFmslNANDTestWritePerform(
    FFXFMLHANDLE    hFML,
    D_BUFFER       *pBuffer,
    D_UINT32        ulBufferSize,
    D_UINT32       *pulPagesWritten,
    D_UINT32        ulSeconds,
    unsigned        nCount,
    unsigned        nVerbosity)
{
    #define         MAX_PAGES   (128)
    D_UINT32        ulDiffTime;
    D_UINT32        ulPageNum = 0;
    D_UINT32        ulRemaining;
    D_UINT32        ulPagesWritten = 0;
    FFXFMLINFO      FmlInfo;
    DCLTIMER        tWrite;
    FFXIOSTATUS     ioStat;
    DCLALIGNEDBUFFER (buffer, tags, FFX_NAND_TAGSIZE * MAX_PAGES);

    DclAssert(hFML);
    DclAssert(pBuffer);
    DclAssert(ulBufferSize);
    DclAssert(pulPagesWritten);
    DclAssert(ulSeconds);
    DclAssert(nCount);

    /*  Get some information about the media
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return 0;

    DclAssert(nCount <= ulBufferSize / FmlInfo.uPageSize);

    /*  Preset buffers to a known pattern
    */
    DclMemSet(pBuffer, 0x5A, ulBufferSize);
    DclMemSet(buffer.tags, 0x6B, sizeof buffer.tags);

    /*  Initialize the timer for a certain number of milliseconds.
    */
    DclTimerSet(&tWrite, ulSeconds * 1000);

    ulRemaining = FmlInfo.ulTotalBlocks * (FmlInfo.ulBlockSize / FmlInfo.uPageSize);

    while(ulRemaining && !DclTimerExpired(&tWrite))
    {
        D_UINT32 ulCount = DCLMIN3(ulRemaining, nCount, MAX_PAGES);

        FMLWRITE_TAGGEDPAGES(hFML, ulPageNum, ulCount, pBuffer, buffer.tags, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("  FfxFmslNANDTestWritePerform() FMLWRITE_TAGGEDAGES() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }

        ulPageNum       += ioStat.ulCount;
        ulRemaining     -= ioStat.ulCount;
        ulPagesWritten  += ioStat.ulCount;
    }

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tWrite);

    if(nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Wrote %s in %lU ms\n", DclScaleBytes(ulPagesWritten * FmlInfo.uPageSize, NULL, 0), ulDiffTime);

    /*  check for possible zero denominator
    */
    if(ulDiffTime == 0)
    {
        /*  How could this ever happen? -- In an emulation environment using
            a RAM based FIM, without any emulating of timing, it can...
        */
        ulDiffTime = 1;
    }

    /*  Return the total number of pages written so that the subsequent
        read test can be limited to reading only that number of pages, and
        therefore the ECC will be correct.
    */
    *pulPagesWritten = ulPagesWritten;

    /*  Return the number of kilobytes we wrote each second
    */
    return (((ulPagesWritten * FmlInfo.uPageSize) / 1024) * 1000) / ulDiffTime;
}


/*-------------------------------------------------------------------
    Local: CommonTest()

    Write and read back single pages.  Write and read back multiple
    pages.  In both cases, cross block boundaries.  For multiple
    page operations, include cases that both stop at and cross block
    boundaries.

    Do all the writing then read it all back and verify.  Detect
    and report single-bit error (per page) on read; this is not a
    test failure.  Read back both with same length, same alignment
    reads, and different length, unaligned.  Verify blank beyond
    last write.  Read redundant regions and verify blank (again
    detect and report single-bit errors without failing).

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        TRUE if the tests passed, FALSE if any failed.
-------------------------------------------------------------------*/
static D_BOOL CommonTest(
    FMSLNANDTESTINFO   *pTI)
{
    #define             COMMON_TEST_BLOCKS 4    /* total erase blocks to use */
    D_UINT32            ulPage, ulStartPage;
    D_UINT32            ulStartSerial = pTI->ulSerial;
    D_UINT32            ulSerial;
    D_UINT32            ulStartBlock;
    D_UINT32            i;
    FFXIOSTATUS         ioStat;

    DclPrintf("    Page read/write tests...\n");

    if(pTI->FmlInfo.ulTotalBlocks < COMMON_TEST_BLOCKS)
    {
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("      Test requires %u block(s), but only %lU are available\n",
                      COMMON_TEST_BLOCKS, pTI->FmlInfo.ulTotalBlocks);
        }
        DclPrintf("      SKIPPED\n");
        return TRUE;
    }

    /*  Find test blocks
    */
    if(!FfxFmslNANDFindTestBlocks(pTI, COMMON_TEST_BLOCKS, &ulStartBlock, FALSE))
    {
        DclPrintf("      Failed to find %u erase block(s)\n",
                  COMMON_TEST_BLOCKS);
        return FALSE;
    }

    /*  Erase the area this test will use.
    */
    FMLERASE_BLOCKS(pTI->hFML, ulStartBlock, COMMON_TEST_BLOCKS, ioStat);
    if(!IOSUCCESS(ioStat, COMMON_TEST_BLOCKS))
    {
        DclPrintf("      Failed to erase %u block(s) at %lU, status %s\n",
                  COMMON_TEST_BLOCKS, ulStartBlock, FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    ulStartPage     = ulStartBlock * pTI->ulPagesPerBlock;

    /*  Write one block's worth, a page at a time.
    */
    for (ulPage = ulStartPage;
         ulPage < ulStartPage + pTI->ulPagesPerBlock;
         ulPage++)
    {
        FfxFmslNANDPageFill(pTI, pTI->pBuffer, 1, pTI->ulSerial++, ulPage, "MEM_COMMON");

        FMLWRITE_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Failed to write 1 page at %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
    }

    DclProductionAssert(ulPage == ulStartPage + pTI->ulPagesPerBlock);

    /*  Write another block's worth, two pages at a time.
    */
    for (ulPage = ulStartPage + pTI->ulPagesPerBlock;
         ulPage < ulStartPage + pTI->ulPagesPerBlock * 2;
         ulPage += 2)
    {
        FfxFmslNANDPageFill(pTI, pTI->pBuffer, 2, pTI->ulSerial++, ulPage, "MEM_COMMON");

        FMLWRITE_UNCORRECTEDPAGES(pTI->hFML, ulPage, 2, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 2))
        {
            DclPrintf("      Failed to write 2 pages at %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
    }

    DclProductionAssert(ulPage == ulStartPage + pTI->ulPagesPerBlock * 2);

    /*  Code below here assumes FMSLNAND_BUFFER_PAGES is odd (code
        further down assumes it is exacly three).
    */
    DclProductionAssert(FMSLNAND_BUFFER_PAGES % 2);

    /*  Write three pages at a time, until a block boundary is crossed
        (not just reached).  Since blocks always contain a power of
        two number of pages and a power of two is not divisible by
        three, the last write is guaranteed to span blocks.
    */
    for (ulPage = ulStartPage + pTI->ulPagesPerBlock * 2;
         ulPage < ulStartPage + pTI->ulPagesPerBlock * 3;
         ulPage += 3)
    {
        FfxFmslNANDPageFill(pTI, pTI->pBuffer, FMSLNAND_BUFFER_PAGES,
                            pTI->ulSerial++, ulPage, "MEM_COMMON");

        FMLWRITE_UNCORRECTEDPAGES(pTI->hFML, ulPage, FMSLNAND_BUFFER_PAGES, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, FMSLNAND_BUFFER_PAGES))
        {
            DclPrintf("      Failed to write %u pages at %lU, status %s\n",
                      FMSLNAND_BUFFER_PAGES, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
    }

    /*  Start reading back and verifying what was written.  First read
        back using reads exactly corresponding to the original writes.
    */
    ulSerial = ulStartSerial;

    /*  Read and verify one block's worth, a page at a time.
    */
    for (ulPage = ulStartPage;
         ulPage < ulStartPage + pTI->ulPagesPerBlock;
         ulPage ++)
    {
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Failed to read 1 page at %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++, ulPage,
                                   "MEM_COMMON", 0, 1, NO_ECC))
        {
            DclPrintf("      Failed to verify 1 page at %lU\n", ulPage);
            return FALSE;
        }
    }

    DclProductionAssert(ulPage == ulStartPage + pTI->ulPagesPerBlock);

    /*  Read and verify another block's worth, two pages at a time.
    */
    for (ulPage = ulStartPage + pTI->ulPagesPerBlock;
         ulPage < ulStartPage + pTI->ulPagesPerBlock * 2;
         ulPage += 2)
    {
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 2, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 2))
        {
            DclPrintf("      Failed to read 2 pages at %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 2, ulSerial++, ulPage,
                                   "MEM_COMMON", 0, 2, NO_ECC))
        {
            DclPrintf("      Failed to verify 2 pages at %lU\n", ulPage);
            return FALSE;
        }
    }

    DclProductionAssert(ulPage == ulStartPage + pTI->ulPagesPerBlock * 2);

    /*  Read and verify three pages at a time.
    */
    for (ulPage = ulStartPage + pTI->ulPagesPerBlock * 2;
         ulPage < ulStartPage + pTI->ulPagesPerBlock * 3;
         ulPage += 3)
    {
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, FMSLNAND_BUFFER_PAGES, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, FMSLNAND_BUFFER_PAGES))
        {
            DclPrintf("      Failed to read %u pages at %lU, status %s\n",
                      FMSLNAND_BUFFER_PAGES, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, FMSLNAND_BUFFER_PAGES,
                                   ulSerial++, ulPage, "MEM_COMMON", 0,
                                   FMSLNAND_BUFFER_PAGES, NO_ECC))
        {
            DclPrintf("      Failed to verify %u pages at %lU\n",
                      FMSLNAND_BUFFER_PAGES, ulPage);
            return FALSE;
        }
    }

    /*  Read the next page and verify it's still blank.
    */
    FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      Failed to read 1 page at page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }
    if (!BlankVerify(pTI, pTI->pBuffer, pTI->FmlInfo.uPageSize, ulPage, 0, NO_ECC))
    {
        DclPrintf("      Failed to blank verify 1 page at page %lU\n", ulPage);
        return FALSE;
    }

    /*  Now the math gets messy.  Read back and verify what was
        written again, but this time avoid reading with the same size
        (and alignment, for multi-block writes) as the writes.
    */
    ulSerial = ulStartSerial;

    /*  First read and verify all of the first block that can be read
        three pages at a time without crossing into the next block.
        This was all written one block at a time.
    */
    for (ulPage = ulStartPage;
         ulPage < (ulStartPage + pTI->ulPagesPerBlock - FMSLNAND_BUFFER_PAGES);
         ulPage += FMSLNAND_BUFFER_PAGES)
    {
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, FMSLNAND_BUFFER_PAGES, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, FMSLNAND_BUFFER_PAGES))
        {
            DclPrintf("      Failed to read %u pages at page %lU, status %s\n",
                      FMSLNAND_BUFFER_PAGES, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }


        for (i = 0;
             i < FMSLNAND_BUFFER_PAGES;
             i++)
        {
            if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer + i * pTI->FmlInfo.uPageSize,
                                       1, ulSerial++, ulPage + i, "MEM_COMMON",
                                       0, 1, NO_ECC))
            {
                DclPrintf("      Failed to verify %u pages at page %lU\n",
                          FMSLNAND_BUFFER_PAGES, ulPage);
                return FALSE;
            }
        }

    }

    /*  Code below here assumes FMSLNAND_BUFFER_PAGES is exactly three.
    */
    DclProductionAssert(FMSLNAND_BUFFER_PAGES == 3);

    /*  The next read will cross the block boundary (remember, the
        block size is a power of 2 and thus not divisible by
        FMSLNAND_BUFFER_PAGES).  Read all three pages at once, then
        figure out whether to verify it as a last single page plus
        the first two-page write, or two last single pages plus the
        first page of the first two-page write.

        Note that the serial number is not incremented in the last
        FfxFmslNANDPageVerify() call, leaving it at the value for the
        first write of the second block, because verification will
        continue from the beginning of the second block.
    */
    FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, FMSLNAND_BUFFER_PAGES, pTI->pBuffer, NULL, ioStat);
    if(!IOSUCCESS(ioStat, FMSLNAND_BUFFER_PAGES))
    {
        DclPrintf("      Failed to read %u pages at page %lU, status %s\n",
                  FMSLNAND_BUFFER_PAGES, ulPage, FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    if ((ulStartPage + pTI->ulPagesPerBlock - ulPage) == 1)
    {
        /* The buffer contains the last of the single pages and the
           first of the two-page writes.
        */
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++, ulPage,
                                   "MEM_COMMON", 0, 1, NO_ECC)
            || !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + pTI->FmlInfo.uPageSize,
                                      2, ulSerial, ulPage + 1, "MEM_COMMON",
                                      0, 2, NO_ECC))
        {
            DclPrintf("      Failed to verify %u pages at page %lU\n",
                      FMSLNAND_BUFFER_PAGES, ulPage);
            return FALSE;
        }
    }
    else
    {
        /* The buffer contains the last two of the single pages and the
           first page of the first two-page write.
        */
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++, ulPage,
                                   "MEM_COMMON", 0, 1, NO_ECC)
            || !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + pTI->FmlInfo.uPageSize,
                                      1, ulSerial++, ulPage + 1, "MEM_COMMON",
                                      0, 1, NO_ECC)
            || !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + 2 * pTI->FmlInfo.uPageSize,
                                      1, ulSerial, ulPage + 2, "MEM_COMMON",
                                      0, 2, NO_ECC))
        {
            DclPrintf("      Failed to verify %u pages at page %lU\n",
                      FMSLNAND_BUFFER_PAGES, ulPage);
            return FALSE;
        }
    }

    /*  At least one page of the second block has been verified.  Keep
        things simple by continuing from the beginning of the second
        block -- there's no harm in verifying a page or two one more
        time.

        Continue reading three blocks at a time.  These reads will
        alternately cover one two-block write plus the first half of
        the next one, and the second half of a two-block write plus
        all of the next one.  Do this up through the last read before
        crossing the block boundary.
    */

    for (ulPage = ulStartPage + pTI->ulPagesPerBlock;
         ulPage < (ulStartPage + 2 * pTI->ulPagesPerBlock - FMSLNAND_BUFFER_PAGES);
         ulPage += FMSLNAND_BUFFER_PAGES)
    {
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, FMSLNAND_BUFFER_PAGES, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, FMSLNAND_BUFFER_PAGES))
        {
            DclPrintf("      Failed to read %u pages at page %lU, status %s\n",
                      FMSLNAND_BUFFER_PAGES, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        if (ulPage % 2 == 0)
        {
            /*  The buffer contains a two-block write plus the first
                half of the next one.  Note that the serial number
                is not incremented after verifying the last block,
                it has to remain the same to verify the other half
                of that write.
            */
            if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 2, ulSerial++, ulPage,
                                       "MEM_COMMON", 0, 2, NO_ECC)
                || !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + 2 * pTI->FmlInfo.uPageSize,
                                          1, ulSerial, ulPage + 2, "MEM_COMMON",
                                          0, 2, NO_ECC))
            {
                DclPrintf("      Failed to verify %u pages at page %lU\n",
                          FMSLNAND_BUFFER_PAGES, ulPage);
                return FALSE;
            }
        }
        else
        {
            /*  The buffer contains the second half of a two-block
                write (which was written beginning one page back from
                the current page) plus all of the next one.
            */
            if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++,
                                       ulPage - 1, "MEM_COMMON", 1, 2, NO_ECC)
                || !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + pTI->FmlInfo.uPageSize,
                                          2, ulSerial++, ulPage + 1,
                                          "MEM_COMMON", 0, 2, NO_ECC))
            {
                DclPrintf("      Failed to verify %u pages at page %lU\n",
                          FMSLNAND_BUFFER_PAGES, ulPage);
                return FALSE;
            }
        }
    }

    /*  Verify the remaining one or two pages of the second block,
        using a read that ends at the block boundary rather than
        crossing it.
    */
    DclAssert((ulPage == (ulStartPage + 2 * pTI->ulPagesPerBlock - 1)) ||
              (ulPage == (ulStartPage + 2 * pTI->ulPagesPerBlock - 2)));
    if (ulPage == (ulStartPage + 2 * pTI->ulPagesPerBlock - 1))
    {
        /*  Read and verify the second block of the last two-block
            write.
        */
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Failed to read page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++, ulPage - 1,
                                   "MEM_COMMON", 1, 2, NO_ECC))
        {
            DclPrintf("      Failed to verify page %lU\n", ulPage);
            return FALSE;
        }
    }
    else
    {
        /*  Read and verify the last complete two-block write.
        */
        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 2, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 2))
        {
            DclPrintf("      Failed to read 2 pages at page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 2, ulSerial++, ulPage,
                                   "MEM_COMMON", 0, 2, NO_ECC))
        {
            DclPrintf("      Failed to verify 2 pages at page %lU\n", ulPage);
            return FALSE;
        }
    }

    /*  Read and verify the rest (written three pages at a time) with
        single-block reads.  This runs off the end of the third block
        into the beginning of the fourth.  This is accounted for in
        the rather complicated-looking termination condition (you were
        warned the math would get messy): the last block was written
        three pages at a time, which is guaranteed not to come out
        even, so there is one more 3-page write than the number that
        fit within the block.
    */
    for (ulPage = ulStartPage + 2 * pTI->FmlInfo.ulBlockSize;
         ulPage < (ulStartPage + 3 * pTI->ulPagesPerBlock + 1);
         ulPage++)
    {
        /*  Which page of the write this was.
        */
        unsigned int uPage = (ulPage - ulStartPage + 2 * pTI->FmlInfo.ulBlockSize) % FMSLNAND_BUFFER_PAGES;

        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      Failed to read page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial,
                                   ulPage - uPage, "MEM_COMMON", uPage,
                                   FMSLNAND_BUFFER_PAGES, NO_ECC))
        {
            DclPrintf("      Failed to verify page %lU\n", ulPage);
            return FALSE;
        }

        /*  Go to next serial number after processing pages 0, 1, and
            2 of the write.
        */
        if (uPage == 2)
            ++ulSerial;
    }

    /*  Note: This test could be setup to read back the spare area
        for the entire length and verify that it is unprogrammed.
    */

    DclPrintf("      PASSED\n");
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ValidatePageStatus()

    Parameters:
        pTI      - The test parameters structure
        ulPage   - The page on which to validate status
        ulStatus - The expected status value
        nTagSize - The expected tag width
        fRaw     - This flag tells us whether this is a typical or raw page

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL ValidatePageStatus(
    FMSLNANDTESTINFO   *pTI,
    D_UINT32            ulPage,
    D_UINT32            ulStatus,
    unsigned            nTagSize,
    D_BOOL              fRaw)
{
    FFXIOSTATUS         ioStat;

  #if FFXCONF_BBMSUPPORT
    FFXIOR_FML_GET_BLOCK_INFO info;

    if(!fRaw)
    {
        FML_GET_BLOCK_INFO(pTI->hFML, ulPage / pTI->ulPagesPerBlock, ioStat, info);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n",
                ulPage / pTI->ulPagesPerBlock, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        ulPage = info.ulRawMapping * pTI->ulPagesPerBlock + ulPage % pTI->ulPagesPerBlock;
    }
  #endif

    FML_GET_RAW_PAGE_STATUS(pTI->hFML, ulPage, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("          Raw page status request for page %lU failed, %s\n",
            ulPage, FfxDecodeIOStatus(&ioStat));

        DclPrintf("      FAILED\n");
        return FALSE;
    }

    if(ioStat.ulFlags != IOFLAGS_PAGE)
    {
        DclPrintf("          Unexpected raw page operation type flags for page %lU. Expected %lX but got %lX\n",
            ulPage, IOFLAGS_PAGE, ioStat.ulFlags);

        DclPrintf("      FAILED\n");
        return FALSE;
    }

    if((ioStat.op.ulPageStatus & PAGESTATUS_MASKFLAGS) != ulStatus)
    {
        DclPrintf("          Unexpected raw page status for page %lU. Expected %lX but got %lX\n",
            ulPage, ulStatus, ioStat.op.ulPageStatus & PAGESTATUS_MASKFLAGS);

        DclPrintf("      FAILED\n");
        return FALSE;
    }

    if(PAGESTATUS_GET_TAG_WIDTH(ioStat.op.ulPageStatus) != nTagSize)
    {
        DclPrintf("          Unexpected raw page tag width for page %lU. Expected %u but got %u\n",
            ulPage, nTagSize, (unsigned)PAGESTATUS_GET_TAG_WIDTH(ioStat.op.ulPageStatus));

        DclPrintf("      FAILED\n");
        return FALSE;
    }

    if(ioStat.op.ulPageStatus & PAGESTATUS_MASKRESERVED)
    {
        DclPrintf("          Unexpected raw page status bits for page %lU. Expected %lX but got %lX\n",
            ulPage, 0, ioStat.op.ulPageStatus & PAGESTATUS_MASKRESERVED);

        DclPrintf("      FAILED\n");
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: TaggedPageTest()

    Write more than a block's worth of tagged pages one at a
    time.  Include recognizable tag data.

    Read back multiple ways: as tagged pages; as corrected pages,
    and tags only.  With the latter mode, use both short and long
    reads, both crossing block boundary and not.  Verify blank
    beyond last write.

    Single-bit errors in the hidden data are detected and
    reported, but are not considered failures.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        TRUE if successful, FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL TaggedPageTest(
    FMSLNANDTESTINFO   *pTI)
{
    #define             NAND_PAGE_TEST_BLOCKS           (2)
    #define             NAND_PAGE_TEST_GROUP            (3)
    #define             NAND_PAGE_TEST_READHIDDEN_GROUP (7)
    D_UINT32            ulStartSerial = pTI->ulSerial;
    D_UINT32            ulSerial;
    D_UINT32            ulPageCount;
    D_UINT32            ulStartPage, ulEndPage, ulPage;
    D_UINT32            ulStartBlock;
    D_UINT16            uPageNum;
    D_UINT16            uPagesProcessed;
    FFXIOSTATUS         ioStat;

    DclPrintf("    Tagged Page Test\n");

    ulPageCount = pTI->ulPagesPerBlock +
        ((pTI->FmlInfo.ulBlockSize / 2) / pTI->FmlInfo.uPageSize);

    DclAssert(ulPageCount % NAND_PAGE_TEST_GROUP == 0);
    DclAssert(NAND_PAGE_TEST_GROUP <= FMSLNAND_BUFFER_PAGES);
    DclAssert(NAND_PAGE_TEST_GROUP < NAND_PAGE_TEST_READHIDDEN_GROUP);
    DclAssert(NAND_PAGE_TEST_READHIDDEN_GROUP % NAND_PAGE_TEST_GROUP != 0);

    /* Choose an area of flash to use.
    */
    if(!FfxFmslNANDFindTestBlocks(pTI, NAND_PAGE_TEST_BLOCKS, &ulStartBlock, FALSE))
    {
        DclPrintf("      Failed to find %u erase block(s)\n", NAND_PAGE_TEST_BLOCKS);
        return FALSE;
    }

    /*  Calculate starting and ending page
    */
    ulStartPage     = ulStartBlock * pTI->ulPagesPerBlock;
    ulEndPage       = ulStartPage + pTI->ulPagesPerBlock + (pTI->ulPagesPerBlock / 2);

    /*  Erase the area this test will use.
    */
    FMLERASE_BLOCKS(pTI->hFML, ulStartBlock, NAND_PAGE_TEST_BLOCKS, ioStat);
    if(!IOSUCCESS(ioStat, NAND_PAGE_TEST_BLOCKS))
    {
        DclPrintf("      Failed to erase %u block(s) at %lU, status %s\n",
            NAND_PAGE_TEST_BLOCKS, ulStartBlock, FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    if(!ValidatePageStatus(pTI, ulStartPage, PAGESTATUS_UNWRITTEN, 0, FALSE))
        return FALSE;

    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("      Write %lU tagged pages, StartPage=%lU (%u at a time)\n",
                  ulPageCount, ulStartPage, NAND_PAGE_TEST_GROUP);
    }

    /*  Write one and a half erase blocks' worth of pages.  Note that
        this is always a multiple of three pages.
    */
    uPageNum = 0;
    for (ulPage = ulStartPage;
         ulPage < ulEndPage;
         ulPage += NAND_PAGE_TEST_GROUP)
    {
        FFXIOSTATUS ioStat;
        DCLALIGNEDBUFFER(aligned, tags, NAND_PAGE_TEST_GROUP*FFX_NAND_TAGSIZE);
        unsigned    k;

        for(k=0; k<NAND_PAGE_TEST_GROUP; k++)
        {
            FfxFmslNANDPageFill(pTI, &pTI->pBuffer[k * pTI->FmlInfo.uPageSize],
                                1, pTI->ulSerial++, ulPage + k, "MEM_NAND_PAGE");

            /*  Use the page number as the hidden data.  Byte order
                doesn't matter, it will only be read right here.
            */
            DclAssert(FFX_NAND_TAGSIZE == sizeof (D_UINT16));

            *(D_UINT16*)&aligned.tags[k*FFX_NAND_TAGSIZE] = uPageNum;

            uPageNum++;
        }

        FMLWRITE_TAGGEDPAGES(pTI->hFML, ulPage, NAND_PAGE_TEST_GROUP,
                             pTI->pBuffer, aligned.tags, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, NAND_PAGE_TEST_GROUP))
        {
            DclPrintf("        Failed to write %u pages at page %lU, status %s\n",
                NAND_PAGE_TEST_GROUP, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }
    }

    /*  Read back and verify what was written
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("      Read %lU ECC corrected pages (1 at a time)\n", ulPageCount);
    }

    /*  First read and verify one page at a time.
    */
    ulSerial = ulStartSerial;
    for (ulPage = ulStartPage;
         ulPage < ulEndPage;
         ulPage++)
    {
        ioStat = FfxFmslTestReadPages(pTI->hFML, ulPage, 1, pTI->pBuffer, READ_PAGES);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("        Failed to read page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        if((pTI->nVerbosity >= DCL_VERBOSE_NORMAL) && (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED))
            DclPrintf("        WARNING! A data error was encountered and corrected using ECC at page %lU\n", ulPage);

        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerial++, ulPage,
                                   "MEM_NAND_PAGE", 0, 1, WITH_ECC))
        {
            return FALSE;
        }
    }

    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("      Read %lU ECC corrected pages (%u at a time)\n", ulPageCount, NAND_PAGE_TEST_GROUP);

    /*  Read back and verify three pages at a time.  Remember that
        this comes out even in the end (wrote 3/2 of an erase block),
        but means one read will cross a block boundary (block size is
        a power of two, thus not divisible by three).
    */
    ulSerial = ulStartSerial;
    for (ulPage = ulStartPage;
         ulPage < ulEndPage;
         ulPage += NAND_PAGE_TEST_GROUP)
    {
        ioStat = FfxFmslTestReadPages(pTI->hFML, ulPage, NAND_PAGE_TEST_GROUP, pTI->pBuffer, READ_PAGES);
        if(!IOSUCCESS(ioStat, NAND_PAGE_TEST_GROUP))
        {
            DclPrintf("        Failed to read %u pages at page %lU, status %s\n",
                NAND_PAGE_TEST_GROUP, ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        if((pTI->nVerbosity >= DCL_VERBOSE_NORMAL) && (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED))
            DclPrintf("        WARNING! A data error was encountered and corrected using ECC at page %lU\n", ulPage);

        if (!FfxFmslNANDPageVerify(pTI, pTI->pBuffer + (0 * pTI->FmlInfo.uPageSize), 1, ulSerial++, ulPage + 0, "MEM_NAND_PAGE", 0, 1, WITH_ECC) ||
            !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + (1 * pTI->FmlInfo.uPageSize), 1, ulSerial++, ulPage + 1, "MEM_NAND_PAGE", 0, 1, WITH_ECC) ||
            !FfxFmslNANDPageVerify(pTI, pTI->pBuffer + (2 * pTI->FmlInfo.uPageSize), 1, ulSerial++, ulPage + 2, "MEM_NAND_PAGE", 0, 1, WITH_ECC))
        {
            return FALSE;
        }
    }

    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("      Read %lU tags (%u at a time)\n", ulPageCount, NAND_PAGE_TEST_READHIDDEN_GROUP);

    /*  Read back and verify the hidden data by doing long reads like
        VBF does.  Take advantage of the fact that at sixteen byte
        (four pages' worth) boundaries in the hidden area, the hidden
        address can be expressed as a page address with a zero hidden
        offset part.  This simplifies address calculations.
    */
    uPagesProcessed = 0;
    ulPage = ulStartPage;
    while(uPagesProcessed < ulPageCount)
    {
        DCLALIGNEDBUFFER(aligned, tags, NAND_PAGE_TEST_READHIDDEN_GROUP*FFX_NAND_TAGSIZE);
        unsigned    q;
        FFXIOSTATUS ioStat;

        FMLREAD_TAGS(pTI->hFML, ulPage, NAND_PAGE_TEST_READHIDDEN_GROUP,
                     aligned.tags, FFX_NAND_TAGSIZE, ioStat);
        if(!IOSUCCESS(ioStat, NAND_PAGE_TEST_READHIDDEN_GROUP))
        {
            DclPrintf("        Failed to read %u tags at page %lU, status %s\n",
                      NAND_PAGE_TEST_READHIDDEN_GROUP, ulPage, FfxDecodeIOStatus(&ioStat));

            return FALSE;
        }

        q = 0;
        while(q < NAND_PAGE_TEST_READHIDDEN_GROUP && uPagesProcessed < ulPageCount)
        {
            if (*(D_UINT16*)&aligned.tags[q*FFX_NAND_TAGSIZE] != uPagesProcessed)
            {
                DclPrintf("        Tag data verification error page %lU[%u]: expected %X, found %X, Good Pages=%U\n",
                    ulPage, q, uPagesProcessed,
                    *(D_UINT16*)&aligned.tags[q*FFX_NAND_TAGSIZE], uPagesProcessed);

                return FALSE;
            }

            uPagesProcessed++;
            ulPage++;
            q++;
        }
    }

    DclPrintf("      PASSED\n");
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: SpareAreaTest()

    Write and read back using lengths of 16 bytes or multiples.
    Note that 4K is one block's worth on large-block NAND.  Reads
    and writes both do and don't cross block boundaries.  Read
    both aligned with and not aligned with writes.  Also read
    pages with MEM_COMMON, verify blank.

    Note that data written must avoid the block status.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL SpareAreaTest(
    FMSLNANDTESTINFO *pTI)
{
    DclPrintf("    Spare Area Test\n");

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("      (not yet implemented)\n");

    DclPrintf("      SKIPPED\n");

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: FfxFmslNANDPageFill()

    Fills a buffer with a data pattern that is unique and helps
    identify the test being performed.  The data pattern avoids
    repeating on any power of 2 boundary.  Each page of a
    multiple-page buffer has its own unique data.

    Parameters:
        pTI              - The test parameters structure
        pBuffer          - buffer to fill with pattern
        ulPageCount      - length to fill in pages
        ulSerial         - a unique serial number for the operation
        ulStartWritePage - starting page for operation
        pszTestName      - The test name

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxFmslNANDPageFill(
    FMSLNANDTESTINFO   *pTI,
    D_BUFFER           *pBuffer,
    D_UINT32            ulPageCount,
    D_UINT32            ulSerial,
    D_UINT32            ulStartWritePage,
    const char         *pszTestName)
{
    unsigned            uPage;

    for (uPage = 0; uPage < ulPageCount; ++uPage)
    {
        PagePattern(pTI, pBuffer, ulSerial, ulStartWritePage, pszTestName, uPage, ulPageCount);
        pBuffer += pTI->FmlInfo.uPageSize;
    }
}


/*-------------------------------------------------------------------
    Local: FfxFmslNANDPageVerify()

    Verifies that the pattern in a buffer matches a pattern
    generated with FfxFmslNANDPageFill().  The buffer may
    contain only a portion of the original pattern, starting
    on a page boundary.

    Parameters:
        pTI              - The test parameters structure
        pBuffer          - A pointer to the buffer to verify
        ulPageCount      - The length to verify in pages
        ulSerial         - The serial number used to generate the
                           pattern
        ulStartWritePage - Starting page for original pattern
        pszTestName      - The test name
        ulOrigFirstPage  - The page number relative to the beginning
                           of the original pattern
        ulOrigPageCount  - The number of pages in the original pattern
        mode             - WITH_ECC if ECC correction was applied and
                           pattern should match exactly, NO_ECC if no
                           ECC was used and a single-bit error should
                           be tolerated.

    Return Value:
        TRUE if the pattern was verified, FALSE if it was incorrect.
-------------------------------------------------------------------*/
D_BOOL FfxFmslNANDPageVerify(
    FMSLNANDTESTINFO   *pTI,
    D_BUFFER           *pBuffer,
    D_UINT32            ulPageCount,
    D_UINT32            ulSerial,
    D_UINT32            ulStartWritePage,
    const char         *pszTestName,
    D_UINT32            ulOrigFirstPage,
    D_UINT32            ulOrigPageCount,
    PAGE_VERIFY_MODE    mode)
{
    D_BOOL              fSuccess = TRUE;
    D_BUFFER           *pPattern;
    D_UINT32            ulPage;

    pPattern = DclMemAlloc(pTI->FmlInfo.uPageSize);
    if(!pPattern)
    {
        DclPrintf("Out of memory!\n");
        return FALSE;
    }

    DclAssert(mode == WITH_ECC || mode == NO_ECC);

    for (ulPage = 0; ulPage < ulPageCount; ++ulPage)
    {
        PagePattern(pTI, pPattern, ulSerial, ulStartWritePage, pszTestName,
                    ulOrigFirstPage++, ulOrigPageCount);
        if (DclMemCmp(pBuffer, pPattern, pTI->FmlInfo.uPageSize) != 0)
        {
            if (mode == WITH_ECC || !SingleBitError(pTI->FmlInfo.uPageSize, pBuffer, pPattern))
            {
                /*  The page was either written with ECC, in which case
                    there should have been no errors, -- OR -- ECCs were
                    not used, in which case we should have had exactly a
                    single bit error, no more or less.

                    One of these two conditions failed so report it and
                    fail the test.
                */
                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclPrintf("Verification failed for page %lU of %lU\n",
                              ulPage + 1, ulPageCount);
                }
                if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                {
                    VerifyError(pTI, pBuffer, pPattern);
                }

                fSuccess = FALSE;
                break;
            }
        }

        pBuffer += pTI->FmlInfo.uPageSize;
    }

    DclMemFree(pPattern);

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: PagePattern()

    Generates one page's pattern.

    Parameters:
        pTI              - The test parameters structure
        pBuffer          - pTI->FmlInfo.uPageSize byte buffer
        ulSerial         - the serial number used to generate the
                           pattern
        ulStartWritePage - starting page for original pattern
        pszTestName      - a name to identify the test being performed
        uPage            - the page number relative to the beginning
                           of the original pattern
        uPageCount       - the total number of pages to put in the
                           message

    Return Value:
        None.
-------------------------------------------------------------------*/
static void PagePattern(
    FMSLNANDTESTINFO   *pTI,
    D_BUFFER           *pBuffer,
    D_UINT32            ulSerial,
    D_UINT32            ulStartWritePage,
    const char         *pszTestName,
    D_UINT32            ulPage,
    D_UINT32            ulPageCount)
{
    D_BUFFER           *p;
    unsigned            uLength, uCopyLength;

    /*  DclSNPrintf() doesn't implement a precision on %s.  Use other
        means to limit the name length.  Don't worry about truncation
        on the cast if the string length is unreasonably long.
    */
    uLength = DCLMIN((unsigned int) DclStrLen(pszTestName), FMSLNAND_MAX_NAME_LENGTH);
    DclMemCpy(pBuffer, pszTestName, uLength);
    p = pBuffer + uLength;
    p += DclSNPrintf((char*)p,
                     pTI->FmlInfo.uPageSize - uLength,
                     " %lU at %08lX page %lU of %lU ",
                     ulSerial, ulStartWritePage, ulPage + 1, ulPageCount);

    /*  The string built so far (excluding the terminating '\0') will
        be used as a repeating pattern to fill the page.  Avoid
        pattern lengths that are a power of 2.  The default software
        ECC (SSFDC algorithm) produces an ECC of all ones for a
        pattern that repeats every 32 bytes, others may have similar
        issues.
    */
    if (DCLISPOWEROF2(p - pBuffer))
        *p++ = ' ';

    uLength = p - pBuffer;

    /*  The first copy only of the pattern is terminated with '\0'
        to make it easy to print it for diagnostic purposes.
    */
    *p++ = '\0';

    /*  Repeat the pattern to fill the page.  Truncate the
        last repetition to fit.
    */
    for (uCopyLength = DCLMIN(uLength, (unsigned int) (pBuffer + pTI->FmlInfo.uPageSize - p));
         uCopyLength;
         uCopyLength = DCLMIN(uLength, (unsigned int) (pBuffer + pTI->FmlInfo.uPageSize - p)))
    {
        DclMemCpy(p, pBuffer, uCopyLength);
        p += uCopyLength;
    }

    /*  Helpful for debugging...

    DclPrintf("PagePattern for Tst=%s SN=%lU SWP=%lU Pg=%lU Cnt=%lU generated \"%s\"\n",
        pszTestName, ulSerial, ulStartWritePage, ulPage, ulPageCount, pBuffer);
    */

    return;
}


/*-------------------------------------------------------------------
    Local: SingleBitError()

    Compares uLength bytes of data to a reference copy of expected
    data.  If exactly one bit differs between them, an informative
    message is printed.  No message is printed if the buffers are
    identical (in which case, this function shouldn't even have
    been called) or if there are more bits different (in which case
    VerifyError() should be called to produce the appropriate error
    message).

    Parameters:
        uLength - the number of bytes to check
        pBuffer - buffer with data to verify, pTI->FmlInfo.uPageSize
                  bytes
        pRef    - buffer with reference data, expected to have a
                  reasonable length terminated string

    Return Value:
        Returns TRUE if exactly one bit differed, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL SingleBitError(
    D_UINT16        uLength,
    const D_BUFFER *pBuffer,
    const D_BUFFER *pRef)
{
    unsigned        i, where = 0;
    D_BOOL          fBitError = FALSE;

    for (i = 0; i < uLength; ++i)
    {
        if (pBuffer[i] != pRef[i])
        {
            if (fBitError || !DCLISPOWEROF2(pBuffer[i] ^ pRef[i]))
                return FALSE;
            where = i;
            fBitError = TRUE;
        }
    }
    if (fBitError)
    {
        DclPrintf("Single-bit error detected at offset %u.  Expected %02X, found %02X\n",
                  where, pRef[where], pBuffer[where]);
        DclPrintf("Page information: %s\n", pRef);
    }
    return fBitError;
}


/*-------------------------------------------------------------------
    Local: VerifyError()

    Compares pTI->FmlInfo.uPageSize bytes of data to a reference
    copy of expected data, and prints informative messages about
    the differences.

    Parameters:
        pTI     - The test parameters structure
        pBuffer - buffer with data to verify, pTI->FmlInfo.uPageSize
                  bytes
        pRef    - buffer with reference data, expected to have a
                  reasonable length terminated string

    Return Value:
        None.
-------------------------------------------------------------------*/
static void VerifyError(
    FMSLNANDTESTINFO   *pTI,
    const D_BUFFER     *pBuffer,
    const D_BUFFER     *pRef)
{
    D_UINT16            uCount = pTI->FmlInfo.uPageSize;
    D_UINT16            uOffset = 0;

    while(uCount)
    {
        #define MAXTITLE    (24)
        D_UINT16    k = DCLMIN(32, uCount);
        char        szTitle[MAXTITLE];

        DclSNPrintf(szTitle, MAXTITLE, "%04X Expected: ", uOffset);

        DclHexDump(szTitle,           HEXDUMP_UINT8|HEXDUMP_NOOFFSET, k, k, pRef);
        DclHexDump("        Found: ", HEXDUMP_UINT8|HEXDUMP_NOOFFSET, k, k, pBuffer);

        uOffset += k;
        pRef += k;
        pBuffer += k;
        uCount -= k;
    }
}


/*-------------------------------------------------------------------
    Local: BlankVerify()

    Verifies that the data in a buffer is all ones, with the
    optional exception of a single zero bit.

    Parameters:
        pTI         - The test parameters structure
        pBuffer     - buffer with data to verify
        uBufferSize - length to verify, any size
        ulPage      - starting media page of data
        pszTestName - a name to identify the test being performed
        mode        - WITH_ECC if ECC correction was applied and data
                      should be completely ones, NO_ECC if no ECC was
                      used and a single-bit error (single zero bit)
                      should be tolerated.

    Return Value:
        Returns TRUE if the data was blank, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL BlankVerify(
    FMSLNANDTESTINFO   *pTI,
    D_BUFFER           *pBuffer,
    unsigned            uBufferSize,
    D_UINT32            ulPage,
    const char         *pszTestName,
    PAGE_VERIFY_MODE    mode)
{
    unsigned            i;
    D_BOOL              fBitError = FALSE;

    (void) pszTestName;

    DclAssert(mode == WITH_ECC || mode == NO_ECC);

    for (i = 0; i < uBufferSize; ++i)
    {
        if (i % pTI->FmlInfo.uPageSize == 0)
            fBitError = FALSE;

        if (pBuffer[i] != (D_UCHAR) ~0)
        {
            if (mode == WITH_ECC || fBitError
                || !DCLISPOWEROF2(pBuffer[i] ^ (D_UCHAR) ~0))
            {
                /*  It would be nice to dump the buffer in hex
                    and text.
                */
                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                    DclPrintf("Non-blank at page %lU, found %02X\n", ulPage, pBuffer[i]);
                return FALSE;
            }
            if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("Single-bit error at page %lU, found %02X\n", ulPage, pBuffer[i]);
            fBitError = TRUE;
        }
    }
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: RawPageTest()

    This test ensures that raw page I/O functions properly.

    Parameters:
        pTI - The test parameters structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL RawPageTest(
    FMSLNANDTESTINFO   *pTI)
{
    #define             NAND_RAW_PAGE_TEST_BLOCKS   (2) /* Number of erase blocks to operate on */
    D_UINT32            ulBlock, ulStartPage, ulPage, ulEndPage;
    D_UINT32            ulSerial = pTI->ulSerial;
    D_UINT32            ulNumTestPages;    /* Number of pages to read/write at a time */
    FFXIOSTATUS         ioStat;
    D_UINT32            i, j;
    D_BOOL              afMainOnly[] = {TRUE, FALSE};
    D_BOOL              fSuccess = FALSE;
  #if FFXCONF_BBMSUPPORT
    FFXIOR_FML_GET_BLOCK_INFO   info;
  #endif

    DclAssert(pTI);

    DclPrintf("    Raw page read/write tests...\n");

    /*  Attempt a raw read to check if funtionality is supported.  Don't
        fail test if unsupported, but log as not implemented and return
    */
    FMLREAD_RAWPAGES(pTI->hFML, 0, 1, pTI->pBuffer, NULL, ioStat);
    if(ioStat.ffxStat == FFXSTAT_FIM_UNSUPPORTEDFUNCTION)
    {
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("      (raw access not implemented)\n");
        DclPrintf("      SKIPPED\n");
        return TRUE;
    }

    if(!FfxFmslNANDFindTestBlocks(pTI, NAND_RAW_PAGE_TEST_BLOCKS, &ulBlock, TRUE))
    {
        DclPrintf("      Failed to find %u erase blocks not marked bad\n", NAND_RAW_PAGE_TEST_BLOCKS);
        return FALSE;
    }

  #if FFXCONF_BBMSUPPORT

    /*  Remember that this is a raw block test, so we MUST GET the raw block
        mapping before running any tests using raw accesses. Failure to do
        so could cause corruption to BBM or attempts to run tests on factory
        bad block. Both issues could cause loss of factory bad block data.
        See commentary in FfxFmslNANDFindTestBlocks.
    */
    FML_GET_BLOCK_INFO(pTI->hFML, ulBlock, ioStat, info);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n",
                ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    ulBlock = info.ulRawMapping;

  #endif

    /*  Compute number of pages to operate on and start/end pages
    */
    ulStartPage = ulBlock * pTI->ulPagesPerBlock;
    ulEndPage = ulStartPage + pTI->ulPagesPerBlock * NAND_RAW_PAGE_TEST_BLOCKS;

    /*  Test combinations of main page data only and main + spare data.
    */
    for(i = 0; i < DCLDIMENSIONOF(afMainOnly); i++)
    {
        /*  Test operations of different number of pages.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("      Testing %lU raw pages (main %s) starting at page %lU\n",
                      ulEndPage - ulStartPage, afMainOnly[i] ? "only" : "+ spare", ulStartPage);
        }

        for(ulNumTestPages = 1;
            ulNumTestPages <= FMSLNAND_BUFFER_PAGES;
            ulNumTestPages++)
        {
            if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("        %lU at a time\n", ulNumTestPages);

            /*  Erase the block(s).
            */
            if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            {
                DclPrintf("          Erasing %u blocks starting at block %lU\n",
                          NAND_RAW_PAGE_TEST_BLOCKS,
                          ulStartPage / pTI->ulPagesPerBlock);
            }

            FMLERASE_RAWBLOCKS(pTI->hFML, ulStartPage / pTI->ulPagesPerBlock, NAND_RAW_PAGE_TEST_BLOCKS, ioStat);
            if(!IOSUCCESS(ioStat, NAND_RAW_PAGE_TEST_BLOCKS))
            {
                DclPrintf("            Failed to erase %u blocks at block %lU, status %s\n",
                          NAND_RAW_PAGE_TEST_BLOCKS,
                          ulStartPage / pTI->ulPagesPerBlock,
                          FfxDecodeIOStatus(&ioStat));

                return FALSE;
            }

            /*  Write out pages ulNumTestPages at a time, skipping any
                remaining pages in the erase block(s)
            */
            if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                DclPrintf("          Writing...\n");

            for(ulPage = ulStartPage;
                (ulPage + ulNumTestPages) < ulEndPage;
                ulPage += ulNumTestPages)
            {
                for(j = 0; j < ulNumTestPages; j++)
                {
                    FfxFmslNANDPageFill(pTI, &pTI->pBuffer[j * pTI->FmlInfo.uPageSize],
                                        1, pTI->ulSerial++, ulPage + j, "MEM_RAW_PAGE");
                    DclMemSet(&pTI->pSpareArea[j * pTI->FmlInfo.uSpareSize], j,
                              pTI->FmlInfo.uSpareSize);
                    DclMemSet(&pTI->pSpareArea2[j * pTI->FmlInfo.uSpareSize], 0,
                              pTI->FmlInfo.uSpareSize);
                }

              #if FFXCONF_BBMSUPPORT
                if(!ValidatePageStatus(pTI, ulPage, PAGESTATUS_UNWRITTEN, 0, TRUE))
                    goto Cleanup;
              #endif

                if(afMainOnly[i])
                {
                    FMLWRITE_RAWPAGES(pTI->hFML, ulPage, ulNumTestPages, pTI->pBuffer, NULL, ioStat);

                  #if FFXCONF_BBMSUPPORT
                    /*  Even after writing a raw page, the page status should
                        still indicate "unwritten".
                    */
                    if(!ValidatePageStatus(pTI, ulPage, PAGESTATUS_UNWRITTEN, 0, TRUE))
                        goto Cleanup;
                  #endif

                }
                else
                {
                    FMLWRITE_RAWPAGES(pTI->hFML, ulPage, ulNumTestPages, pTI->pBuffer, pTI->pSpareArea, ioStat);
                }
                if(!IOSUCCESS(ioStat, ulNumTestPages))
                {
                    DclPrintf("            Write failed for page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
                    goto Cleanup;
                }
            }

            /*  Read and verify pages ulNumTestPages at a time, skipping any
                remaining pages in the erase block(s)
            */
            if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                DclPrintf("          Reading...\n");

            for(ulPage = ulStartPage;
                (ulPage + ulNumTestPages) < ulEndPage;
                ulPage += ulNumTestPages)
            {
                if(afMainOnly[i])
                {
                    FMLREAD_RAWPAGES(pTI->hFML, ulPage, ulNumTestPages, pTI->pBuffer, NULL, ioStat);
                }
                else
                {
                    FMLREAD_RAWPAGES(pTI->hFML, ulPage, ulNumTestPages, pTI->pBuffer, pTI->pSpareArea2, ioStat);
                }
                if(!IOSUCCESS(ioStat, ulNumTestPages))
                {
                    DclPrintf("            Read failed for page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
                    goto Cleanup;
                }

                for(j = 0; j < ulNumTestPages; j++)
                {
                    /*  Verify the main page data.
                    */
                    if (!FfxFmslNANDPageVerify(pTI, &pTI->pBuffer[j * pTI->FmlInfo.uPageSize],
                                               1, ulSerial++, ulPage + j,
                                               "MEM_RAW_PAGE", 0, 1, NO_ECC))
                    {
                        DclPrintf("            Verify failed for page %lU\n", ulPage + j);

                        DclHexDump("", HEXDUMP_UINT8, 32, pTI->FmlInfo.uPageSize, &pTI->pBuffer[j * pTI->FmlInfo.uPageSize]);

                        goto Cleanup;
                    }

                    /*  Verify the data from the spare area if we read/wrote it.
                    */
                    if(!afMainOnly[i] &&
                       (DclMemCmp(&pTI->pSpareArea[j * pTI->FmlInfo.uSpareSize],
                                  &pTI->pSpareArea2[j * pTI->FmlInfo.uSpareSize],
                                  pTI->FmlInfo.uSpareSize) != 0) &&
                       !SingleBitError(pTI->FmlInfo.uSpareSize,
                                       &pTI->pSpareArea2[j * pTI->FmlInfo.uSpareSize],
                                       &pTI->pSpareArea[j * pTI->FmlInfo.uSpareSize]))
                    {
                        DclPrintf("            Verify failed for spare area of page %lU\n", ulPage + j);

                        DclHexDump("", HEXDUMP_UINT8, 32, pTI->FmlInfo.uSpareSize, &pTI->pSpareArea2[j * pTI->FmlInfo.uSpareSize]);

                        goto Cleanup;
                    }
                }
            }
        }
    }

    fSuccess = TRUE;

  Cleanup:

    /*  Erase the blocks we used, because these tests can write data into
        spare areas and cause us to inadvertently and undesirably mark a
        block as bad.
    */
    FMLERASE_RAWBLOCKS(pTI->hFML, ulStartPage / pTI->ulPagesPerBlock, NAND_RAW_PAGE_TEST_BLOCKS, ioStat);
    if(!IOSUCCESS(ioStat, NAND_RAW_PAGE_TEST_BLOCKS))
    {
		DclPrintf(" 		   Failed to erase %u blocks at block %lU, status %s\n",
                  NAND_RAW_PAGE_TEST_BLOCKS,
                  ulStartPage / pTI->ulPagesPerBlock,
                  FfxDecodeIOStatus(&ioStat));

        return FALSE;
    }

    if(!fSuccess)
        DclPrintf("      FAILED\n");
    else
        DclPrintf("      PASSED\n");

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: TagTestMain()

    This test ensures that tag I/O functions properly.

    (could/should do)
    Write a variety of lengths, from 4 bytes up to over 2K.
    Exercise both crossing and not crossing 2K boundary, block
    boundary.

    Parameters:
        pTI - The test parameters structure

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL TagTestMain(
    FMSLNANDTESTINFO   *pTI)
{
    unsigned            nTagStart;
    unsigned            nTagEnd;

    DclPrintf("    Tag read/write tests...\n");

    if(pTI->FmlDevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)
    {
        nTagStart = 1;
        nTagEnd = pTI->FmlDevInfo.uMetaSize;
    }
    else
    {
        /*  If variable length tags are not supported, the meta size
            better be exactly what we expect for general FFX usage.
        */
        DclAssert(pTI->FmlDevInfo.uMetaSize == FFX_NAND_TAGSIZE);

        nTagStart = FFX_NAND_TAGSIZE;
        nTagEnd = FFX_NAND_TAGSIZE;
    }

    for(; nTagStart <= nTagEnd; nTagStart++)
    {
        if(!TagTest(pTI, nTagStart))
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: TagTest()

    This test ensures that tag I/O functions properly.

    (could/should do)
    Write a variety of lengths, from 4 bytes up to over 2K.
    Exercise both crossing and not crossing 2K boundary, block
    boundary.

    Parameters:
        pTI      - The test parameters structure
        nTagSize - The tag size to test

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
static D_BOOL TagTest(
    FMSLNANDTESTINFO   *pTI,
    unsigned            nTagSize)
{
    #define             FMSLNAND_TAG_VALUE1 (1) /* Initial tag values */
    D_UINT32            ulIndex;
    D_UINT32            ulStartPage = 0;
    FFXIOSTATUS         ioStat;
    D_UINT32            ulTagCount;
    D_UINT32            ulBlock;
    D_UINT32            ulBlockCount;
    D_UINT32            ulCount;

  #if FFX_NAND_TAGSIZE != 2
    #error "FFX: FMSLTEST:  FFX_NAND_TAGSIZE is not 2!"
  #endif

    DclPrintf("      Tag read/write tests for %u byte tags...\n", nTagSize);

    /*  Determine how many tags to test.  Use the minimum of the number of
        tags that fit in the buffer, or the number of pages on the device.
    */
    ulTagCount = DCLMIN(pTI->ulBufferSize / nTagSize,
                        pTI->FmlInfo.ulTotalBlocks * pTI->ulPagesPerBlock);

    /*  For devices with very large erase blocks, the above calculation of
        ulTagCount could result in a *lot* of tags getting tested, because
        the buffer size is linearly related to the erase block size. This
        could cause the tag test to take a long time. Limit the number of
        tags tested to a reasonable level; say, enough to fill 16 blocks.
    */
    ulTagCount = DCLMIN(ulTagCount, pTI->ulPagesPerBlock * 16);

    /*  Determine the area to test.
    */
    ulBlockCount = ulTagCount / pTI->ulPagesPerBlock;
    ulBlockCount += (ulTagCount % pTI->ulPagesPerBlock) ? 1 : 0;
    if(!FfxFmslNANDFindTestBlocks(pTI, ulBlockCount, &ulBlock, FALSE))
    {
        DclPrintf("        Unable to find %lU block(s) for test\n", ulBlockCount);
        return FALSE;
    }

    /*  Output the test configuration.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("        Testing %lU Tags using %lU block(s) starting at block %lU\n",
                  ulTagCount, ulBlockCount, ulBlock);
    }
    ulStartPage = ulBlock * pTI->ulPagesPerBlock;

    /*  First erase the flash.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        DclPrintf("        Erasing test blocks\n");

    if(!EraseAndVerify(pTI, ulBlock, ulBlockCount))
    {
        DclPrintf("          Erase failed at block %lU\n", ulBlock);
        return FALSE;
    }

    /***********************************************************\
     *  TEST 1: Validate that reading the page status (RAW)    *
     *          for a block which is erased, reports what we   *
     *          would expect for an erased block.              *
    \***********************************************************/

    if(!ValidatePageStatus(pTI, ulStartPage, PAGESTATUS_UNWRITTEN, 0, FALSE))
        return FALSE;

        /***********************************************************\
         *  TEST 2: Verify that tags read from erased pages return *
         *          what we expect -- all 0xFFs.                   *
        \***********************************************************/

    DclPrintf("        Verifying tag reads from pages with no tags written\n");

    FMLREAD_TAGS(pTI->hFML, ulStartPage, ulTagCount, pTI->pBuffer, nTagSize, ioStat);
    if(!IOSUCCESS(ioStat, ulTagCount))
    {
        DclPrintf("          FMLREAD_TAGS() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    for(ulIndex = 0; ulIndex < ulTagCount * nTagSize; ulIndex++)
    {
        if(pTI->pBuffer[ulIndex] != ERASED8)
        {
            D_UINT32    ulFailedPage = ulIndex / nTagSize;

            DclPrintf("          Tag data readback verify failed for page %lU\n", ulFailedPage + ulStartPage);
            return FALSE;
        }
    }

        /***********************************************************\
         *  TEST 3: Verify that any tag which  is all 0xFFs except *
         *          for a single bit (in any bit position) returns *
         *          the correct tag value.                         *
        \***********************************************************/

    DclPrintf("        Verifying tags with 1 bit turned off\n");

    ulCount = DCLMIN3(ulTagCount, (nTagSize * 8), pTI->ulBigBufferSize/pTI->FmlInfo.uPageSize);

    /* Blank page data to use for FMSLWRITE_TAGGEDPAGES:
    */

    DclMemSet(pTI->pBigBuffer, ERASED8, pTI->ulBigBufferSize);

    /* Initialize Tag Test data:
    */

    DclMemSet(pTI->pBuffer, ERASED8, ulCount * nTagSize);
    for(ulIndex = 0; ulIndex < ulCount; ulIndex++)
    {
        pTI->pBuffer[(ulIndex*nTagSize)+((ulIndex)/8)] = ~(1<<((ulIndex)%8));
    }

    /*  Write the bit pattern to the tags for every test page. Use
        FMLWRITE_TAGGEDPAGES to use ECC for platforms/devices that
        support it.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Writing %lU tags\n", ulCount);

    FMLWRITE_TAGGEDPAGES(pTI->hFML, ulStartPage, ulCount, pTI->pBigBuffer,
                         pTI->pBuffer, nTagSize, ioStat);
    if(!IOSUCCESS(ioStat, ulCount))
    {
        DclPrintf("          FMLWRITE_TAGGEDPAGES() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    /*  Read the tags back.
    */
    DclMemSet(pTI->pBuffer2, 0, ulCount * nTagSize);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Reading %lU tags\n", ulCount);

    FMLREAD_TAGS(pTI->hFML, ulStartPage, ulCount, pTI->pBuffer2, nTagSize, ioStat);
    if(!IOSUCCESS(ioStat, ulCount))
    {
        DclPrintf("          FMLREAD_TAGS() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

    /*  Verify that the data we read back matches what we originally wrote.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Verifying %lU tags\n", ulCount);

    for(ulIndex = 0; ulIndex < ulCount * nTagSize; ulIndex++)
    {
        if(pTI->pBuffer[ulIndex] != pTI->pBuffer2[ulIndex])
        {
            D_UINT32    ulFailedPage = ulIndex / nTagSize;
            D_UINT32    ulFailedPageOffset = ulFailedPage * nTagSize;

            DclPrintf("          Tag data readback verify failed for page %lU\n", ulFailedPage + ulStartPage);
            DclHexDump("Original: ", HEXDUMP_UINT8, 8, nTagSize, &pTI->pBuffer[ulFailedPageOffset]);
            DclHexDump("ReadBack: ", HEXDUMP_UINT8, 8, nTagSize, &pTI->pBuffer2[ulFailedPageOffset]);
            return FALSE;
        }
    }

    if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        DclPrintf("        Erasing test blocks\n");

    if(!EraseAndVerify(pTI, ulBlock, ulBlockCount))
    {
        DclPrintf("          Erase failed at block %lU\n", ulBlock);
        return FALSE;
    }

        /***********************************************************\
         *  TEST 4: Write a bunch of tags and make sure they read  *
         *          back correctly.                                *
        \***********************************************************/

    DclPrintf("        Write tags, read them back, and verify them\n");

    /*  Initialize the data pattern to the tags.
    */
    for(ulIndex = 0; ulIndex < pTI->ulBufferSize; ulIndex++)
    {
        pTI->pBuffer[ulIndex] = (D_UCHAR)(ulIndex + FMSLNAND_TAG_VALUE1);
    }

    /*  Write the bit pattern to the tags for every test page.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Writing %lU tags\n", ulTagCount);

    FMLWRITE_TAGS(pTI->hFML, ulStartPage, ulTagCount, pTI->pBuffer, nTagSize, ioStat);
    if(!IOSUCCESS(ioStat, ulTagCount))
    {
        DclPrintf("          FMLWRITE_TAGS() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
        return FALSE;
    }

/*  Disabling the following test pending discussion of Bugzilla #2550.
*/
#if 0
        /***********************************************************\
         *  TEST 4a: Verify that the page status is what we expect *
         *           for a page which has just a tag written, but  *
         *           no data.                                      *
        \***********************************************************/

    if(!ValidatePageStatus(pTI, ulStartPage, PAGESTATUS_UNWRITTEN, nTagSize, FALSE))
        return FALSE;
#endif

    /*  Read the tags back.
    */
    DclMemSet(pTI->pBuffer2, 0, pTI->ulBufferSize);

    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Reading %lU tags\n", ulTagCount);

    /*  Read all the tags back, taking into account that a correctable error
        could return flow of control here before they are all read:
    */
    ulIndex = ulTagCount;
    ulCount = 0;
    while (ulIndex)
    {
        FMLREAD_TAGS(pTI->hFML, ulStartPage+ulCount, ulIndex, &(pTI->pBuffer2[ulCount*nTagSize]), nTagSize, ioStat);
        if(!IOSUCCESS(ioStat, ulIndex))
        {
            /*  Tag reads can return a correctable error status for some driver
                implementations:
            */
            if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
            {
                DclPrintf("          FMLREAD_TAGS():  Bit(s) corrected at tag %d of %d\n",
                            ioStat.ulCount, ulTagCount);
            }
            else
            {
                DclPrintf("          FMLREAD_TAGS() failed\n");
                return FALSE;
            }
        }
        ulIndex -= ioStat.ulCount;
        ulCount += ioStat.ulCount;
    }

    /*  Verify that the data we read back matches what we originally wrote.
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("        Verifying %lU tags\n", ulTagCount);

    for(ulIndex = 0; ulIndex < ulTagCount * nTagSize; ulIndex++)
    {
        if(pTI->pBuffer[ulIndex] != pTI->pBuffer2[ulIndex])
        {
            D_UINT32    ulFailedPage = ulIndex / nTagSize;
            D_UINT32    ulFailedPageOffset = ulFailedPage * nTagSize;

            DclPrintf("          Tag data readback verify failed for page %lU\n", ulFailedPage + ulStartPage);
            DclHexDump("Original: ", HEXDUMP_UINT8, 8, nTagSize, &pTI->pBuffer[ulFailedPageOffset]);
            DclHexDump("ReadBack: ", HEXDUMP_UINT8, 8, nTagSize, &pTI->pBuffer2[ulFailedPageOffset]);
            return FALSE;
        }
    }

        /***********************************************************\
         *  TEST 5: Overwrite the first tag from TEST 4 with the   *
         *          inverse bit pattern, and make sure it reads    *
         *          back as all zeros.                             *
        \***********************************************************/

    DclPrintf("        Overwrite a tag with its inverse\n");

    /*  Fill the first tag in tbe buffer with the tag's inverse to
        invalidate the block.  Write this tag out to the flash and
        read back to verify tag is now zeroed out.

        This is only necessary to do with the first tag because marking
        the first tag as invalid will mark the whole block as invalid.
        We avoid rewriting all the tags (as this test previously did)
        to avoid possible false positives with the test, as writing
        the tags out of could possibly cause write disturb issues in
        the flash.

        Modern NAND parts only allow writes within a block in order
        of the page number, not going backwards.  This rewrite of the
        tag violates this rule.

        If the tag's check byte is wrong or the ECC indicates an
        uncorrectable error (both likely, but not mathematically proven
        to be certain after this overwrite) the NTM is to return all zeros.
    */

    /*  Fill the first tag in the write buffer with inverse patterns.
        The original first tag was (ulIndex + FMSLNAND_TAG_VALUE1) with
        ulIndex incrementing for each byte in the tag.
    */
    /*  Initialize the data pattern to the tags.
    */
    if (pTI->FmlDevInfo.uDeviceFlags & DEV_PGM_ONCE)
    {
        DclPrintf("          SKIPPED- device not overwritable.\n");
    }
    else
    {
        for(ulIndex = 0; ulIndex < nTagSize; ulIndex++)
        {
            pTI->pBuffer[ulIndex] = (D_UCHAR)(~(ulIndex + FMSLNAND_TAG_VALUE1));
        }

        /*  Overwrite the tag WITHOUT ERASING.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Overwriting tag for page %lU\n", ulStartPage);

        FMLWRITE_TAGS(pTI->hFML, ulStartPage, 1, pTI->pBuffer, nTagSize, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("          FMLWRITE_TAGS() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        /*  Read back the tag using FMLREAD_TAGS.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Reading overwritten tag for page %lU (1)\n", ulStartPage);

        FMLREAD_TAGS(pTI->hFML, ulStartPage, 1, pTI->pSpareArea, nTagSize, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("          FMLREAD_TAGS() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        /*  Verify that the second bitmask wrote over the first, this should
            make all the value in the flash in the first tag zero.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Verifying tag for page %lU (1)\n", ulStartPage);

        DclMemSet(pTI->pSpareArea2, 0, nTagSize);
        for(ulIndex = 0; ulIndex < nTagSize; ulIndex++)
        {
            if(pTI->pSpareArea[ulIndex] != pTI->pSpareArea2[ulIndex])
            {
                DclPrintf("          Tag data readback verify failed (1)\n");
                DclHexDump("Expected: ", HEXDUMP_UINT8, 8, nTagSize, pTI->pSpareArea2);
                DclHexDump("ReadBack: ", HEXDUMP_UINT8, 8, nTagSize, pTI->pSpareArea);
                return FALSE;
            }
        }

        /*  Read back the tag using FMLREAD_TAGGEDPAGES.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Reading overwritten tag for page %lU (2)\n", ulStartPage);

        FMLREAD_TAGGEDPAGES(pTI->hFML, ulStartPage, 1, pTI->pBuffer, pTI->pSpareArea, nTagSize, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            /*  Bit errors are an acceptable result here, as overwriting data
                data in devices/NTMs that use hardware EDC will invalidate the
                ECCs previously generated and stored for that page.
            */
            if (!((ioStat.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA) ||
                 (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)))
            {
                DclPrintf("          FMLREAD_TAGGEDPAGES() failed with status %s\n", FfxDecodeIOStatus(&ioStat));
                return FALSE;
            }
        }

        /*  Verify that the second bitmask wrote over the first, this should
            make all the value in the flash in the first tag zero.
            Don't care about the page data for this test.
        */
         if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Verifying tag for page %lU (2)\n", ulStartPage);

        DclMemSet(pTI->pSpareArea2, 0, nTagSize);
        for(ulIndex = 0; ulIndex < nTagSize; ulIndex++)
        {
            if(pTI->pSpareArea[ulIndex] != pTI->pSpareArea2[ulIndex])
            {
                DclPrintf("          Tag data readback verify failed (2)\n");
                DclHexDump("Expected: ", HEXDUMP_UINT8, 8, nTagSize, pTI->pSpareArea2);
                DclHexDump("ReadBack: ", HEXDUMP_UINT8, 8, nTagSize, pTI->pSpareArea);
                return FALSE;
            }
        }
    }

    /*  Erase the area we used for the tag test and verify
        that the area erased!
    */
    if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        DclPrintf("        Erasing test blocks\n");

    if(!EraseAndVerify(pTI, ulBlock, ulBlockCount))
    {
        DclPrintf("          Erase failed at block %lU\n", ulBlock);
        return FALSE;
    }

    DclPrintf("        PASSED\n");

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: EraseAndVerify()

    Erase and verify the specified range of flash.

    NOTE: Trashes pTI->pBuffer!

    Parameters:
        pTI     - The test parameters structure
        ulBlock - The starting block to erase adn verify.
        ulCount - Number of blocks

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL EraseAndVerify(
    FMSLNANDTESTINFO   *pTI,
    D_UINT32            ulBlock,
    D_UINT32            ulCount)
{
    D_INT16             i;
    D_UINT16            uEraseCount;
    D_UINT32            ulPage;
    D_UINT32            ulEndPage;
    FFXIOSTATUS         ioStat;

    DclAssert(pTI);
    DclAssert((ulBlock + ulCount) <= pTI->FmlInfo.ulTotalBlocks);

    uEraseCount = 0;
    ulPage = ulBlock * pTI->ulPagesPerBlock;
    ulEndPage = ulPage + (ulCount * pTI->ulPagesPerBlock);
    while(ulPage < ulEndPage)
    {
        if(ulPage % pTI->ulPagesPerBlock == 0)
        {
            ++uEraseCount;
            FMLERASE_BLOCKS(pTI->hFML, ulPage / pTI->ulPagesPerBlock, 1, ioStat);
            if(!IOSUCCESS(ioStat, 1))
            {
                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    DclPrintf("    FMLERASE_BLOCKS() failed, Block=%lU EraseCount=%U, status %s\n",
                              ulPage / pTI->ulPagesPerBlock, uEraseCount, FfxDecodeIOStatus(&ioStat));
                }
                return FALSE;
            }
        }

        FMLREAD_UNCORRECTEDPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("    FMLREAD_UNCORRECTEDPAGES() failed at Page %lU, status %s\n", ulPage, FfxDecodeIOStatus(&ioStat));
            return FALSE;
        }

        for(i = 0; i < pTI->FmlInfo.uPageSize; ++i)
        {
            if(pTI->pBuffer[i] != 0xFF)
            {
                if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
                    DclPrintf("    Erase verify failed, page: %lU\n", ulPage);
                return FALSE;
            }
        }

        ulPage++;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: BlockStatusTest()

    Tests the basic functionality of the GetBlockStatus API.  Does
    not know where bad blocks should be found, so only logs if
    blocks status other than BLOCKSTATUS_NOTBAD are found.

    If the test is run in extensive mode all the blocks on the
    device will be tested.  Otherwise, only a randomly selected
    group of 32 blocks will be tested.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL BlockStatusTest(
    FMSLNANDTESTINFO   *pTI)
{
    D_UINT32            ulBlock = 0;
    D_UINT32            ulBlockCount = pTI->FmlInfo.ulTotalBlocks;
    FFXIOSTATUS         ioStatRaw;
    FFXIOSTATUS         ioStat;

    DclPrintf("    Block Status Test...\n");

    if(!pTI->fExtensive)
    {
        ulBlockCount = DCLMIN(32, pTI->FmlInfo.ulTotalBlocks);
        if(!FfxFmslNANDFindTestBlocks(pTI, ulBlockCount, &ulBlock, FALSE))
        {
            DclPrintf("      Unable to find %lU consecutive blocks.\n", ulBlockCount);
            DclPrintf("      FAILED\n");
            return FALSE;
        }
    }

    for(ulBlock = 0; ulBlock < ulBlockCount; ulBlock++)
    {
        FML_GET_RAW_BLOCK_STATUS(pTI->hFML, ulBlock, ioStatRaw);
        if(!IOSUCCESS(ioStatRaw, 1))
        {
            DclPrintf("      FML_GET_RAW_BLOCK_STATUS failed at block %lU, status %s\n", ulBlock, FfxDecodeIOStatus(&ioStatRaw));
            DclPrintf("      FAILED\n");
            return FALSE;
        }

        FML_GET_BLOCK_STATUS(pTI->hFML, ulBlock, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_STATUS failed at block %lU, status %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            return FALSE;
        }

        if(pTI->nVerbosity > DCL_VERBOSE_NORMAL)
        {
            DclPrintf("      Block %4lU:  Status=%lX RawStatus=%lX\n",
                ulBlock, ioStat.op.ulBlockStatus, ioStatRaw.op.ulBlockStatus);
        }

        if(ioStatRaw.ulFlags != IOFLAGS_BLOCK)
        {
            DclPrintf("          Unexpected raw block operation type flags for block %lU. Expected %lX but got %lX\n",
                ulBlock, IOFLAGS_BLOCK, ioStatRaw.ulFlags);
            DclPrintf("      FAILED\n");
            return FALSE;
        }

        if(ioStatRaw.op.ulBlockStatus & BLOCKSTATUS_MASKRESERVED)
        {
            DclPrintf("          Unexpected raw block status bits for block %lU. Expected %lX but got %lX\n",
                ulBlock, 0, ioStatRaw.op.ulBlockStatus & BLOCKSTATUS_MASKRESERVED);

            DclPrintf("      FAILED\n");
            return FALSE;
        }

        if(ioStat.ulFlags != IOFLAGS_BLOCK)
        {
            DclPrintf("          Unexpected block operation type flags for block %lU. Expected %lX but got %lX\n",
                ulBlock, IOFLAGS_BLOCK, ioStat.ulFlags);
            DclPrintf("      FAILED\n");
            return FALSE;
        }

        if(ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKRESERVED)
        {
            DclPrintf("          Unexpected block status bits for block %lU. Expected %lX but got %lX\n",
                ulBlock, 0, ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKRESERVED);

            DclPrintf("      FAILED\n");
            return FALSE;
        }

        if(((ioStatRaw.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD) &&
           ((ioStatRaw.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_LEGACYNOTBAD))
        {
            if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("      Bad block marking %lX found in block %lU\n", ioStat.op.ulBlockStatus, ulBlock);
        }
    }

    DclPrintf("      PASSED\n");
    return TRUE;

}


#if FFXCONF_BBMSUPPORT
/*-------------------------------------------------------------------
    Local: VerifyRawMapping()

    Completes basic tests to verify that a given typical block
    address mapps to the given raw block address.

    Parameters:
        pTI        - The test parameters structure.
        ulBlock    - Typical data block address within the array.
        ulRawBlock - Expected raw block mapping

   Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL VerifyRawMapping(
    FMSLNANDTESTINFO   *pTI,
    D_UINT32 ulBlock,
    D_UINT32 ulRawBlock)
{
    FFXIOSTATUS ioStat;
    D_UINT32    ulSerialNumber;
    D_UINT32    ulPage,
                ulRawPage;

    /* Nothing to do if the blocks are the same */
    if(ulBlock == ulRawBlock)
    {
        return TRUE;
    }
    ulPage = ulBlock * pTI->ulPagesPerBlock;
    ulRawPage = ulRawBlock * pTI->ulPagesPerBlock;
    ulSerialNumber = ulPage * ulRawPage;

    /* Erase the target block, verify first page */
    FMLERASE_BLOCKS(pTI->hFML, ulBlock, 1, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLERASE_BLOCKS failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    DclMemSet(pTI->pBuffer, 0x00, pTI->ulBufferSize);
    FMLREAD_NATIVEPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
    if((ioStat.ffxStat != FFXSTAT_FIMUNCORRECTED &&
        ioStat.ffxStat != FFXSTAT_SUCCESS)
        || ioStat.ulCount != 1)
    {
        DclPrintf("      FMLREAD_RAWPAGES failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    if(!BlankVerify(pTI, pTI->pBuffer, pTI->FmlInfo.uPageSize,
            ulBlock * pTI->ulPagesPerBlock, "VerifyRawMapping", NO_ECC))
    {
        /* BlankVerify() will display problems if found */
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    /* Read from the raw mapping and verify that too is erased */
    DclMemSet(pTI->pBuffer, 0x00, pTI->ulBufferSize);
    FMLREAD_RAWPAGES(pTI->hFML, ulRawPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLREAD_RAWPAGES failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    if(!BlankVerify(pTI, pTI->pBuffer, pTI->FmlInfo.uPageSize,
            ulBlock * pTI->ulPagesPerBlock, "VerifyRawMapping", NO_ECC))
    {
        /* BlankVerify() will display problems if found */
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    /* Write verify a data pattern to the typical block */
    FfxFmslNANDPageFill(pTI, pTI->pBuffer, 1, ulSerialNumber, ulPage, "VerifyRawMapping");
    DclMemSet(pTI->pSpareArea, 0xff, pTI->FmlInfo.uSpareSize);
    FMLWRITE_NATIVEPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      Failed to write page at block %lU\n", ulBlock);
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    DclMemSet(pTI->pBuffer, 0x00, pTI->ulBufferSize);
    FMLREAD_NATIVEPAGES(pTI->hFML, ulPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
    if((ioStat.ffxStat != FFXSTAT_FIMUNCORRECTED &&
        ioStat.ffxStat != FFXSTAT_SUCCESS)
        || ioStat.ulCount != 1)
    {
        DclPrintf("      FMLREAD_RAWPAGES failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerialNumber, ulPage,
            "VerifyRawMapping", 0, 1, NO_ECC))
    {
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    /* Read from the raw mapping and verify that matches what we just wrote */
    DclMemSet(pTI->pBuffer, 0x00, pTI->ulBufferSize);
    FMLREAD_RAWPAGES(pTI->hFML, ulRawPage, 1, pTI->pBuffer, pTI->pSpareArea, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLREAD_RAWPAGES failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }
    if(!FfxFmslNANDPageVerify(pTI, pTI->pBuffer, 1, ulSerialNumber, ulPage,
            "VerifyRawMapping", 0, 1, NO_ECC))
    {
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: BlockInfoTest()

    Tests the basic functionality of the FML_GET_BLOCK_INFO and
    FML_GET_RAW_BLOCK_INFO APIs.

    If the test is run in extensive mode all the blocks on the
    device will be tested.  Otherwise, only a randomly selected
    group of 32 blocks will be tested.

    NOTE: In its current form, this test assumes that the
          disk is aligned on an even chip boundary.  If this
          is not the case, this test may fail boundary checks.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL BlockInfoTest(
    FMSLNANDTESTINFO               *pTI)
{
/*    FFXIOR_FML_GET_RAW_BLOCK_INFO   infoRaw; */
    FFXIOR_FML_GET_BLOCK_INFO       info;
    D_UINT32                        ulBlock;
    FFXIOSTATUS                     ioStat;
    FFXFMLINFO                      FmlInfo;
    D_BOOL                          fBlockInfoOk = TRUE;

    DclPrintf("    Block Info Test...\n");

    if(FfxFmlDiskInfo(pTI->hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
            DclPrintf("      Unable to get disk info!\n");
            DclPrintf("      FAILED\n");
            return FALSE;
    }

    /*  Run through basic error conditions.
    */
    FML_GET_BLOCK_INFO(pTI->hFML, FmlInfo.ulTotalBlocks, ioStat, info);
    if(IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_GET_BLOCK_INFO unexpectedly passed with block = %lU, %s\n", FmlInfo.ulTotalBlocks, FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    FML_GET_BLOCK_INFO(NULL, 1, ioStat, info);
    if(IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_GET_BLOCK_INFO unexpectedly passed with NULL FIM handle\n");
        DclPrintf("      FAILED\n");
        return FALSE;
    }

    /*  This test will check a number of the first and last blocks in the
        array.  The test is not and cannot be exhaustive except within a
        highly constrained environment due to some variables such as the
        reserved space, number of physical flash chips actualy used, BBM
        format parameters, and factory bad block marking.

        The following tests will check the first and last group of blocks
        of the disk and verify that they appear correct.

        The test assumptions validated include:

        1) Block zero is the only block that can have a raw block address
           which is equal to the typical block address.  This will not be
           true if that block has been replaced.
        2) Any block that exists within the last 'physical chip' will *not*
           have a raw address that is the same as the typical address.
        3) Mappings and state information will remain consistent throughout
           this test.
    */
    for(ulBlock = 0; ulBlock < FmlInfo.ulTotalBlocks; ++ulBlock)
    {
        /*  If this is not an extensive test, limit to a number of the
            first and last blocks in the array.
        */
        if(!pTI->fExtensive && (FmlInfo.ulTotalBlocks > 50))
        {
            if(ulBlock > 25 && ulBlock < (FmlInfo.ulTotalBlocks - 25))
            {
                ulBlock = FmlInfo.ulTotalBlocks - 25;
            }
        }

        FML_GET_BLOCK_INFO(pTI->hFML, ulBlock, ioStat, info);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            return FALSE;
        }

        switch(info.ulBlockInfo)
        {
            /*  This should be either a good block or a factory bad block
            */
            case BLOCKINFO_GOODBLOCK:
                if(info.ulBlock == info.ulRawMapping)
                {
                    /* Not much more we can do in terms of validation */
                    break;
                }

                /*  The block is good and there is a difference between the
                    typical block address and the raw maped address.
                */
                if(!VerifyRawMapping(pTI, info.ulBlock, info.ulRawMapping))
                {
                    fBlockInfoOk = FALSE;
                }
                break;

            /*  Retired or factory bad blocks should be remapped.
            */
            case BLOCKINFO_FACTORYBAD:
            case BLOCKINFO_RETIRED:
                if(info.ulRawMapping == info.ulBlock)
                {
                    DclPrintf("      FML_GET_BLOCK_INFO indicates a remapped block but is not remapped block %lU\n", ulBlock);
                    DclPrintf("      FAILED\n");
                    fBlockInfoOk = FALSE;
                    break;
                }

                /*  I think the following are bad tests. These checks will fail
                    if the BBM table for a bad block inside a particular FML
                    disk resides outside of that disks raw block space. In a
                    normal configuration that's not a problem, but with disk
                    spanning it is. There's no rule I am aware of that says
                    BBM has to reside within the raw space of an FML disk that
                    may use that perticular BBM.

                    TEMPORARY- disable these tests.
                */

              #if 0
                FML_GET_RAW_BLOCK_INFO(pTI->hFML, info.ulRawMapping, ioStat, infoRaw);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("      FML_GET_RAW_BLOCK_INFO failed at block %lU, raw mapping %lU, %s\n",
                        info.ulBlock, info.ulRawMapping, FfxDecodeIOStatus(&ioStat));
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }

                /*  The blocks had better both describe the same raw block.
                */
                if(info.ulRawMapping != infoRaw.ulRawMapping)
                {
                    DclPrintf("      ulRawMapping mismatch: typical = %lU, raw = %lU\n", info.ulRawMapping, infoRaw.ulRawMapping);
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }
                if(info.ulBlockInfo != infoRaw.ulBlockInfo)
                {
                    DclPrintf("      ulBlockInfo mismatch: typical = %lU, raw = %lU\n", info.ulBlockInfo, infoRaw.ulBlockInfo);
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }

                /* The typical and raw block addresses are different */
                if(!VerifyRawMapping(pTI, info.ulBlock, info.ulRawMapping))
                {
                    fBlockInfoOk = FALSE;
                }
              #endif

                break;

            /*  The info we have right now is based on a typical address used
                and should never be any of the internal types.
            */
            case BLOCKINFO_BBMSYSTEM:
            case BLOCKINFO_RESERVED:
            case BLOCKINFO_TEMPORARY:
                DclPrintf("      FML_GET_BLOCK_INFO returned unexpected status at block %lU, %lU\n", ulBlock, info.ulBlockInfo);
                DclPrintf("      FAILED\n");
                fBlockInfoOk = FALSE;
                break;

            case BLOCKINFO_UNKNOWN:
            default:
                DclPrintf("      FML_GET_BLOCK_INFO returned unknown status at block %lU, %lU\n", ulBlock, info.ulBlockInfo);
                DclPrintf("      FAILED\n");
                fBlockInfoOk = FALSE;
                break;
        }
    }

    if(fBlockInfoOk)
    {
        DclPrintf("      PASSED\n");
    }
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: RetireBlockTest()

    Tests the basic functionality of the FML_RETIRE_RAW_BLOCK API.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL RetireBlockTest(
    FMSLNANDTESTINFO               *pTI)
{
    D_UINT32                        ulBlock = 0;
    D_UINT32                        ulPagesToCopy;
    D_BUFFER                       *pBuffer;
    D_UINT32                        ulBlockCount = pTI->FmlInfo.ulTotalBlocks;
    FFXIOSTATUS                     ioStat;
    D_UINT32                        ulSerial = pTI->ulSerial;
    FFXIOR_FML_GET_RAW_BLOCK_INFO   infoRaw;
    FFXIOR_FML_GET_BLOCK_INFO       info;
    FFXFMLINFO                      FmlInfo;
    FFXFMLDEVINFO                   FmlDevInfo;
    FFXBBMSTATS                     BbmStats;
    D_BOOL                          fSuccess = FALSE;
    D_UCHAR                         aucDummyTag[2] ={0,0};

    pBuffer = NULL;

    DclPrintf("    Retire Block Test...\n");

    if(FfxFmlParameterGet(pTI->hFML, FFXPARAM_STATS_BBM, &BbmStats, sizeof(FFXBBMSTATS) ) != FFXSTAT_SUCCESS)
    {
        DclPrintf("      Unable to get BBM statistics!\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  Loop through four times if we are not doing extensive tests.
        Otherwise, loop through as many times as there are pages
        per block.  Additionally, bound this by the number of spare
        blocks.
    */
    if(!pTI->fExtensive)
    {
        ulBlockCount = DCLMIN(4, pTI->FmlInfo.ulTotalBlocks);
    }
    else
    {
        ulBlockCount = DCLMIN3(pTI->ulPagesPerBlock, pTI->FmlInfo.ulTotalBlocks,
            BbmStats.ulFreeBlocks);
    }

    if(!FfxFmslNANDFindTestBlocks(pTI, ulBlockCount, &ulBlock, FALSE))
    {
        DclPrintf("      Unable to find %lU consecutive blocks.\n", ulBlockCount);
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    if(FfxFmlDiskInfo(pTI->hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
            DclPrintf("      Unable to get disk info!\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
    }

    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
    {
            DclPrintf("      Unable to get device info!\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
    }

    /*  Run through basic error conditions.
    */
    FML_RETIRE_RAW_BLOCK(pTI->hFML, FmlDevInfo.ulRawBlocks, 1, ioStat);
    if(IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_RETIRE_RAW_BLOCK unexpectedly passed with start block past maximum addressable block\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    FML_RETIRE_RAW_BLOCK(NULL, 1, 1, ioStat);
    if(IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_RETIRE_RAW_BLOCK unexpectedly passed with NULL FIM handle\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    FML_RETIRE_RAW_BLOCK(pTI->hFML, 0, pTI->ulPagesPerBlock + 1, ioStat);
    if(IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FML_RETIRE_RAW_BLOCK unexpectedly passed with pages to copy past maximum\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  Allocate enough memory for Page Size * (Pages Per Block + 1)
    */
    pBuffer = DclMemAlloc(FmlInfo.uPageSize * (pTI->ulPagesPerBlock + 1) );
    if(!pBuffer)
    {
        DclPrintf("      MemAlloc failed on test buffer\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    for(ulBlock = 0; ulBlock < ulBlockCount; ulBlock++)
    {
        /*  First, make sure that we are working from a clean slate.
        */
        FMLERASE_BLOCKS(pTI->hFML, ulBlock, 1, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FMLERASE_BLOCKS failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        ulPagesToCopy = ulBlock + 1;

        /*  If we are not performing extensive tests, copy PagesPerBlock - 1 on third iteration
            and PagesPerBlock on the fourth.
        */
        if(!pTI->fExtensive)
        {
            if(ulBlock == 2)
            {
                ulPagesToCopy = pTI->ulPagesPerBlock - 1;
            }

            if(ulBlock == 3)
            {
                ulPagesToCopy = pTI->ulPagesPerBlock;
            }
        }

        FfxFmslNANDPageFill(pTI, pBuffer, ulPagesToCopy, pTI->ulSerial++, ulBlock * pTI->ulPagesPerBlock, "RetireBlockTest");
        FMLWRITE_TAGGEDPAGES(pTI->hFML, ulBlock * pTI->ulPagesPerBlock, ulPagesToCopy, pBuffer, &aucDummyTag[0], 2, ioStat);
        if(!IOSUCCESS(ioStat, ulPagesToCopy))
        {
            DclPrintf("      Failed to write %lU pages at block %lU\n", ulPagesToCopy, ulBlock);
            fSuccess = FALSE;
            goto Cleanup;
        }

        FML_GET_BLOCK_INFO(pTI->hFML, ulBlock, ioStat, info);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        FML_GET_RAW_BLOCK_INFO(pTI->hFML, info.ulRawMapping, ioStat, infoRaw);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_RAW_BLOCK_INFO failed at block %lU, raw mapping %lU, %s\n",
                info.ulBlock, info.ulRawMapping, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        (void)infoRaw;

        FML_RETIRE_RAW_BLOCK(pTI->hFML, info.ulRawMapping, ulPagesToCopy, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_RETIRE_RAW_BLOCK failed at block = %lU, %s\n", info.ulRawMapping, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        /*  Now make sure that a read from the same typical page address still matches what we wrote.
        */
        DclMemSet(pBuffer, 0x55, FmlInfo.uPageSize * (pTI->ulPagesPerBlock + 1));

        ioStat = FfxFmslTestReadPages(pTI->hFML, ulBlock * pTI->ulPagesPerBlock , ulPagesToCopy, pBuffer, READ_PAGES);
        if(!IOSUCCESS(ioStat, ulPagesToCopy))
        {
            DclPrintf("      Failed to read %lU pages at %lU\n", ulPagesToCopy, ulBlock * pTI->ulPagesPerBlock);
            fSuccess = FALSE;
            goto Cleanup;
        }
        if (!FfxFmslNANDPageVerify(pTI, pBuffer, ulPagesToCopy, ulSerial++, ulBlock * pTI->ulPagesPerBlock,
                                   "RetireBlockTest", 0, ulPagesToCopy, NO_ECC))
        {
            DclPrintf("      Page verify failed at %lU\n", ulBlock * pTI->ulPagesPerBlock);
            fSuccess = FALSE;
            goto Cleanup;
        }

        /*  Ensure that ulBlock does not match ulRawMapping
        */
        FML_GET_BLOCK_INFO(pTI->hFML, ulBlock, ioStat, info);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        if( info.ulBlock == info.ulRawMapping )
        {
            DclPrintf("      RETIRE_RAW_BLOCK failed at %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      info.ulBlock unexpectedly matches info.ulRawMapping after block retire.\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }
    }

    /*  If we were bounded by the number of free BBM blocks, make sure a subsequent block retire fails.
    */
    if( ulBlock == BbmStats.ulFreeBlocks && ulBlock != pTI->FmlInfo.ulTotalBlocks)
    {
        FML_GET_BLOCK_INFO(pTI->hFML, ulBlock, ioStat, info);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n", ulBlock, FfxDecodeIOStatus(&ioStat));
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        FML_RETIRE_RAW_BLOCK(pTI->hFML, info.ulRawMapping, 1, ioStat);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FML_RETIRE_RAW_BLOCK unexpectedly passed when free BBM blocks should have been exhausted.\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }
    }

    fSuccess = TRUE;
    DclPrintf("      PASSED\n");

    Cleanup:
    if(pBuffer)
    {
        DclMemFree(pBuffer);
    }

    return fSuccess;
}
#endif


/*-------------------------------------------------------------------
    Local: NativeOpsTest()

    Tests the basic functionality of the FMLREAD_NATIVEPAGES and
    FMLWRITE_NATIVEPAGES APIs.

    Parameters:
        pTI - The test parameters structure.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL NativeOpsTest(
    FMSLNANDTESTINFO   *pTI)
{
    FFXIOSTATUS         ioStat;
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       FmlDevInfo;
    DCLALIGNEDBUFFER    (tags, data, FFX_NAND_TAGSIZE);
    D_BUFFER           *spares;
    D_BUFFER           *pBuffer;
    D_BOOL              fSuccess;
    D_UINT8             ucData;

    pBuffer = NULL;
    spares = NULL;

    DclPrintf("    Native Operations Test...\n");

    /*  Get our disk and device info.
    */
    if(FfxFmlDiskInfo(pTI->hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
            DclPrintf("      Unable to get disk info!\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
    }

    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
    {
            DclPrintf("      Unable to get device info!\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
    }

    /*  Allocate enough space for one page.
    */
    pBuffer = DclMemAlloc(FmlInfo.uPageSize);
    if(!pBuffer)
    {
        DclPrintf("      MemAlloc failed on test buffer\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  Allocate enough space for one page.
    */
    spares = DclMemAlloc(FmlInfo.uSpareSize);
    if(!spares)
    {
        DclPrintf("      MemAlloc failed on spares buffer\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  First, make sure that we are working from a clean slate.
    */
    FMLERASE_BLOCKS(pTI->hFML, 0, 1, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      Failed to erase block zero.\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  Make sure that no tagged data is lost when using native
    reads and writes to create a duplicate of a page.
    */
    FfxFmslNANDPageFill(pTI, pBuffer, 1, pTI->ulSerial, 0, "NativeOpsTest");
    FMLWRITE_TAGGEDPAGES(pTI->hFML, 0, 1, pBuffer, tags.data, FFX_NAND_TAGSIZE, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLWRITE_TAGGEDPAGES failed, %s.\n", FfxDecodeIOStatus(&ioStat));
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    DclMemSet(pBuffer, 0x55, FmlInfo.uPageSize);
    FMLREAD_NATIVEPAGES(pTI->hFML, 0 , 1, pBuffer, spares, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        if(ioStat.ffxStat == FFXSTAT_FIMUNCORRECTED)
        {
            DclPrintf("      Warning: ReadNativePages not implemented!\n" );
        }
        else
        {
            DclPrintf("      FMLREAD_NATIVEPAGES failed, %s.\n", FfxDecodeIOStatus(&ioStat) );
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }
    }

    FMLWRITE_NATIVEPAGES(pTI->hFML, 1, 1, pBuffer, spares, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLWRITE_NATIVEPAGES failed.\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    DclMemSet(pBuffer, 0xAA, FmlInfo.uPageSize);
    FMLREAD_TAGGEDPAGES(pTI->hFML, 1, 1, pBuffer, tags.data, FFX_NAND_TAGSIZE, ioStat);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("      FMLREAD_TAGGEDPAGES failed.\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    if (!FfxFmslNANDPageVerify(pTI, pBuffer, 1, pTI->ulSerial, 0,
                               "NativeOpsTest", 0, 1, NO_ECC))
    {
        DclPrintf("      Page verify failed!\n");
        DclPrintf("      FAILED\n");
        fSuccess = FALSE;
        goto Cleanup;
    }

    /*  Insert a single bit error into the buffer, write it out with
    FMLWRITE_NATIVEPAGES, and make sure that the error is handled properly.
    Don't run this test for devices that don't require ECC.
    */
    if(pTI->FmlDevInfo.uDeviceFlags & DEV_REQUIRES_ECC)
    {
        ucData = pBuffer[5];
        ucData = ucData ^ 4;
        pBuffer[5] = ucData;

        FMLWRITE_NATIVEPAGES(pTI->hFML, 2, 1, pBuffer, spares, ioStat);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FMLWRITE_NATIVEPAGES failed.\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }


        /*  This call should never return success.  Either the read wil return with
        FFXSTAT_FIMCORRECTABLEDATA, or if error correction is not implemented,
        FFXSTAT_FIMUNCORRECTED will be returned.
        */
        FMLREAD_NATIVEPAGES(pTI->hFML, 2, 1, pBuffer, spares, ioStat);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("      FMLREAD_TAGGEDPAGES returned success when a single bit error was expected.\n");
            DclPrintf("      FAILED\n");
            fSuccess = FALSE;
            goto Cleanup;
        }

        if(ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
        {
            if (!FfxFmslNANDPageVerify(pTI, pBuffer, 1, pTI->ulSerial, 0,
                                       "NativeOpsTest", 0, 1, WITH_ECC))
            {
                DclPrintf("      Page verify failed after corrected single-bit error!\n");
                fSuccess = FALSE;
                goto Cleanup;
            }
        }
        else
        {
            if(ioStat.ffxStat != FFXSTAT_FIMUNCORRECTED)
            {
                DclPrintf("      Unexpected read failure after injecting single-bit error!\n");
                DclPrintf("      ioStat.ffxStat set to %lU.\n", ioStat.ffxStat);
                fSuccess = FALSE;
                goto Cleanup;
            }
            else
            {
                DclPrintf("      Warning: Uncorrected single-bit error!\n");
            }
        }
    }

  #if PXA320_HACK
    {
        FFXIOSTATUS     tempIoStat;

        DclPrintf("Performing an extra read of good tags to accomplish PXA320_HACK\n");

        FMLREAD_TAGGEDPAGES(pTI->hFML, 0, 1, pBuffer, spares, FFX_NAND_TAGSIZE, tempIoStat);
        DclAssert(IOSUCCESS(tempIoStat, 1));
    }
  #endif

    fSuccess = TRUE;
    DclPrintf("      PASSED\n");

    Cleanup:
    if(pBuffer)
    {
        DclMemFree(pBuffer);
    }
    if(spares)
    {
        DclMemFree(spares);
    }

    return fSuccess;
}


/*-------------------------------------------------------------------
    Local: FfxFmslNANDFindTestBlocks()

    Finds ulNumBlocks of consective blocks.  If requested with
    fMayBeUsedRaw set to TRUE, the blocks will also be consecutive
    within the physical flash and guaranteed not to overlay any
    factory bad-block markings or BBM reserved areas.

    IMPORTANT NOTE:

    The block numbers provided by this function will be FML logical
    block numbers, NOT raw block numbers, even if fMayBeusedRaw is
    set to TRUE. IT IS THE RESPONSIBILITY OF THE CALLER TO BE SURE
    TO USE GET_BLOCK_INFO TO GET THE RAW MAPPING BEFORE PERFORMING
    ANY RAW ACCESSES, AND PERFORM RAW ACCESSES ONLY ON THE RAW MAPPING.
    FAILURE TO OBSERVE THIS RULE MAY RESULT IN BBM CORRUPTION OR
    ATTEMPTS TO RUN TESTS ON FACTORY BAD BLOCKS, WHICH COULD RESULT
    IN LOSS OF FACTORY BAD BLOCK INFORMATION.

    Parameters:
        pTI           - The test parameters structure
        ulNumBlocks   - Number of consecutive erase blocks to find
        pulBlockNum   - A pointer to the location in which to store
                        the number of the first block of the group
                        found. This location will be modified only
                        if the function returns successfully.
        fMayBeUsedRaw - Specifies that some or all of the blocks may
                        be used by the caller with raw interfaces.
                        This introduces new requirements:
                        1) No remappings for factory bad blocks are
                           permitted in the range.
                        2) Raw mappings must show as good blocks-
                           see commentary below.
                        3) Extra checks must be done to ensure that
                           raw mappings do not straddle BBM reserved
                           areas, which would cause them to not be
                           contiguous on the physical flash.

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
 -------------------------------------------------------------------*/
D_BOOL FfxFmslNANDFindTestBlocks(
    FMSLNANDTESTINFO           *pTI,
    D_UINT32                    ulNumBlocks,
    D_UINT32                   *pulBlockNum,
    D_BOOL                      fMayBeUsedRaw)
{
    #define                     FIND_TEST_BLOCKS_RETRIES 25  /*  Retries to find test blocks */
    D_BOOL                      fFound = TRUE;
    D_UINT32                    ulBlockNum = 0;
    D_UINT32                    ii;
  #if FFXCONF_BBMSUPPORT
    FFXIOR_FML_GET_BLOCK_INFO   info;
    FFXIOSTATUS                 ioStat;
    D_UINT32                    jj;
    D_UINT32                    ulLastRawBlock = 0;
  #endif
    D_BOOL                      fFirstPass;

    DclAssert(pTI);

    /*  Fail if more blocks are requested than are available in the disk.
    */
    if(ulNumBlocks > pTI->FmlInfo.ulTotalBlocks)
        return FALSE;

    for(ii = 0; ii < FIND_TEST_BLOCKS_RETRIES; ii++)
    {
        fFound = TRUE;
        fFirstPass = TRUE;

        /*  Choose an area of flash to use.  Adjust the block number down
            so we don't exceed the total blocks in the disk.
        */
        if(ulNumBlocks < pTI->FmlInfo.ulTotalBlocks)
        {
            ulBlockNum = DclRand(pTI->pulRandomSeed) % (pTI->FmlInfo.ulTotalBlocks - ulNumBlocks);
        }

      #if FFXCONF_BBMSUPPORT
        if(!fMayBeUsedRaw)
        {

            /*  REFACTOR: If we know a block is not going to get used raw,
                one might think it unnecessary to worry about bad block
                remapping, as that is handled automatically by the device
                manager. The reason we do worry is because many tests call
                ValidatePageStatus, and that function traffics in raw pages.
                The conversion to raw pages for a remapped factory bad block
                may result in a raw page number that is problematic for the
                new FML disk-spanning code.

                This is because all normally-mapped FML disks use at least
                one BBM table they don't span, and it may actually be some
                distance away from the disk's unremapped raw blocks. If the
                FML disk contains a remapped bad block whose remapping is
                in that table, a subsequent attempt to use its raw mapping
                through the FML interace will cause the interface to assert.

                Stopping the use of any range of blocks that has a remapped
                bad block in it prevents this from happening. However, if
                a block should go bad and get remapped while running this
                test, the same behavior could be seen.

                See Bugzilla 3280.
            */
            for(jj = 0; jj < ulNumBlocks; jj++)
            {
                FML_GET_BLOCK_INFO(pTI->hFML, ulBlockNum + jj, ioStat, info);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n",
                            ulBlockNum + jj, FfxDecodeIOStatus(&ioStat));
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }

                /*  Be sure no blocks in the selected range are remapped
                    factory bad blocks:
                */
                if (info.ulBlockInfo != BLOCKINFO_GOODBLOCK)
                {
                    fFound = FALSE;
                    break;
                }
            }
        }
        else
        {
            FFXIOR_FML_GET_RAW_BLOCK_INFO   infoRaw;

            /*  Check if erase blocks that aren't marked bad.  Use block
                info to determine if the block has been remapped.
            */
            for(jj = 0; jj < ulNumBlocks; jj++)
            {
                FML_GET_BLOCK_INFO(pTI->hFML, ulBlockNum + jj, ioStat, info);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("      FML_GET_BLOCK_INFO failed at block %lU, %s\n",
                            ulBlockNum + jj, FfxDecodeIOStatus(&ioStat));
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }

                /*  Be sure no blocks in the selected range are remapped
                    factory bad blocks:
                */
                if (info.ulBlockInfo != BLOCKINFO_GOODBLOCK)
                {
                    fFound = FALSE;
                    break;
                }

                /*  In multi-chip configurations, an FML disk may span the
                    individual BBM reserved areas in each chip. The way BBM
                    handles this can cause a block to show as remapped here,
                    even though it isn't within BBM. Check the raw block to
                    be sure it is good:
                */
                FML_GET_RAW_BLOCK_INFO(pTI->hFML, info.ulRawMapping, ioStat, infoRaw);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("      FML_GET_RAW_BLOCK_INFO failed at block %lU, %s\n",
                            info.ulRawMapping, FfxDecodeIOStatus(&ioStat));
                    DclPrintf("      FAILED\n");
                    return FALSE;
                }
                if (infoRaw.ulBlockInfo != BLOCKINFO_GOODBLOCK)
                {
                    fFound = FALSE;
                    break;
                }

                /*  This procedure guarantees the returned block range to be
                    physically contiguous on the flash. That won't be the case
                    if our selected range accidentally straddles a BBM reserved
                    area. Ensure we choose a contiguous range:
                */
                if(fFirstPass)
                {
                    fFirstPass = FALSE;
                    ulLastRawBlock = info.ulRawMapping;
                }
                else
                {
                    if(info.ulRawMapping != ulLastRawBlock + 1)
                    {
                        fFound = FALSE;
                        break;
                    }
                    ulLastRawBlock = info.ulRawMapping;
                }
            }
        }
      #else
        (void)fMayBeUsedRaw;
      #endif

        if(fFound)
        {
            *pulBlockNum = ulBlockNum;
            return TRUE;
        }
    }

    return FALSE;
}

#endif  /* FFXCONF_NANDSUPPORT */



