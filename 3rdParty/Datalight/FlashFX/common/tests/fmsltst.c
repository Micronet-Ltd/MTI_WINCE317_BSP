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

    Exercises and tests all aspects of the FMSL API.

    - Fix to not use static data(!)
    - Need to allow the user to abort during long tests
    - Need to test status bits
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmsltst.c $
    Revision 1.92  2010/12/22 20:52:33Z  garyp
    Fixed to build cleanly when latency reduction features are disabled.
    Revision 1.91  2010/12/15 23:44:27Z  glenns
    Cleaned up FfxFmslTestReadPages to remove compiler warnings.
    Modified ReadAndVerify to allow correctable errors.
    Revision 1.90  2010/07/08 03:56:39Z  garyp
    Updated to disable all random error injection before starting the test (if
    the /BBM option is used).
    Revision 1.89  2010/07/07 19:20:44Z  garyp
    Added BBM tests.
    Revision 1.88  2010/04/29 00:04:21Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.87  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.86  2009/12/03 21:59:30Z  garyp
    Updated to use FfxDisplayDiskInfo().
    Revision 1.85  2009/11/27 18:03:20Z  garyp
    Properly conditioned some code so we build cleanly.
    Revision 1.84  2009/11/25 23:15:45Z  garyp
    Fixed to ensure that the LockTests flag is cleared upon each test
    invocation.
    Revision 1.83  2009/11/18 03:15:49Z  garyp
    Added a first cut at some block locking tests.
    Revision 1.82  2009/08/04 01:48:08Z  garyp
    Merged from the v4.0 branch.  Modified the performance tests to iterate
    through a range of sector counts when reading and writing, so bottlenecks
    can be identified.  Modified the "BIG_BLOCK_SIZE" to be dynamically 
    allocated and be the same size as an erase block.  Allows the performance
    test to do stuff which is more equivalent to what the VBFTEST performance
    test is doing.  Enhanced the alignment test to allow a smaller alignment 
    size to be specified on the command line, to simplify testing and debugging
    FIMs and NTMs.  Chopped out a bunch of logic which is now unnecessary due
    to the cleaned up FML read/write interfaces.  Now regardless whether we 
    are running on NAND or NOR, we can use the FMLREAD/WRITE_PAGES() macros 
    to read and write corrected data  Updated to use the revised FfxSignOn()
    function, which now takes an fQuiet parameter.  Modified so the "huge" 
    tests inablility to allocate a large buffer is not considered a test
    failure -- rather the test is skipped.
    Revision 1.81  2009/04/09 02:58:24Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.80  2009/03/31 20:08:23Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.79  2009/03/18 08:24:56Z  keithg
    Added print out of additional information such as EDC and endurance
    characteristics reported by the device.
    Revision 1.78  2009/03/16 21:07:16Z  glenns
    Fix Bug 2357: Add code to VerifyBlockSize and WriteAndVerify functions to
    allocate a dummy tag buffer and use ECC-enabled FML interfaces to conduct
    tests if the device type is NAND.  Fix Bug 2357: Modify ReadAndVerify to
    use ECC-enabled FML interface if device type is NAND.  Fix Bug 2357: Modify
    Verify function to no longer accommodate single bit errors for NAND devices,
    and the above changes should see to it that such errors are corrected.
    Revision 1.77  2009/03/09 19:31:39Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.76  2009/02/16 17:23:27Z  michaelm
    Fixed test failure for non-NAND environments
    Revision 1.75  2009/02/15 22:02:32Z  michaelm
    fixed various logic errors in FfxFmslTestReadPages
    Revision 1.74  2009/02/09 05:12:08Z  glenns
    Removed test on incorrect ratio between page size and spare area size.  New
    NAND devices can and do have spare area sizes that do not meet this
    requirement.
    Revision 1.73  2009/02/09 00:41:40Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FFXTOOLPARAMS
    structure.
    Revision 1.72  2009/02/06 22:55:54Z  michaelm
    Cleaned up build warnings and logic errors in fmsltst.c
    Revision 1.71  2009/02/06 05:25:17Z  michaelm
    Renamed Read_Pages() to FfxFmslTestReadPages() and moved function
    description to fmsltst.c.  Restored original single-bit error logic in
    Verify()
    Revision 1.70  2009/02/03 15:47:33Z  michaelm
    Moved fxfmlapi.h include into fmsltst.h.
    Revision 1.69  2009/02/03 00:37:25Z  michaelm
    Moved Read_Pages into fmsltst.h.  Now using Read_Pages() so test will
    function normally in the case of a correctable read error.
    Revision 1.68  2009/01/30 20:03:07Z  billr
    Fix typos in help message.
    Revision 1.67  2009/01/28 22:05:26Z  deanw
    Substituted spaces for TABs.
    Revision 1.66  2009/01/26 19:29:42Z  deanw
    Removed FmlTestBlockReadWrite()
    Revision 1.65  2009/01/19 19:07:55Z  deanw
    Added FmlTestBlockReadWrite.
    Revision 1.64  2009/01/14 22:50:30Z  deanw
    Removed use of global buffers. Functions now use dynamic memory allocation.
    Revision 1.63  2009/01/06 22:45:11Z  michaelm
    Added command-line switch /R to determine whether to run RetireBlockTest
    Revision 1.62  2008/12/12 18:46:56Z  thomd
    Document Extensive optional minutes parameter;
    use nand-safe tests in all cases
    Revision 1.61  2008/12/03 21:22:16Z  thomd
    Corrected extensive test - write NAND blocks in order only
    Revision 1.60  2008/05/13 16:01:47Z  thomd
    Added timing parameter to extensive writes test;
    Varied extensive write locations mathematically
    Revision 1.59  2008/05/09 03:09:15Z  garyp
    Updated to pass "BigBlockBuffer" to the NAND test so it is available.
    Revision 1.58  2008/05/06 05:31:14Z  garyp
    Clarified some messaging to make the test more useful -- no functional
    changes.
    Revision 1.57  2008/01/31 01:14:30Z  garyp
    Updated to accommodate variable length tags.
    Revision 1.56  2008/01/13 07:26:53Z  keithg
    Function header updates to support autodoc.
    Revision 1.55  2007/11/08 19:11:50Z  pauli
    Added huge operations tests.
    Revision 1.54  2007/11/03 20:13:21Z  Garyp
    Fixed the alignment tests to function properly on non-NAND devices.
    Removed some bogus typecasts.
    Revision 1.53  2007/10/28 16:54:38Z  Garyp
    Added FMSL level flash stress tests.
    Revision 1.52  2007/10/18 17:07:07Z  pauli
    Added verbose level as a test parameter.
    Revision 1.51  2007/10/16 16:53:47Z  Garyp
    Updated to use the modified DclMemPoolVerify() function.
    Revision 1.50  2007/10/16 00:50:33Z  pauli
    Corrected a copy/paste error in the previous revision.
    Revision 1.49  2007/10/11 22:48:52Z  pauli
    Updated to write pages with ECC protection in the alignment test.
    Revision 1.48  2007/10/11 21:35:04Z  pauli
    Enhanced the alignment test to test with page counts greater than one.
    Revision 1.47  2007/10/11 00:16:06Z  Garyp
    Updated to ensure that the static buffers are properly aligned according
    to DCL_ALIGNSIZE.  Improved error handling code.
    Revision 1.46  2007/10/02 14:17:20Z  pauli
    Only build NAND specific code if NAND support is enabled.
    Revision 1.45  2007/10/01 22:40:14Z  pauli
    Resolve Bug 355: Added alignment tests.
    Revision 1.44  2007/09/13 20:41:48Z  pauli
    Resolved Bug 1437: Added a random seed to the test parameters.
    Revision 1.43  2007/09/12 21:43:44Z  Garyp
    Modified to use FFX_MAX_PAGESIZE rather than a hard-coded 2048 as the
    maximum size test block.  Removed an assert at the end of the test
    if it fails.  Added more configuration validation.
    Revision 1.42  2007/08/27 18:51:42Z  pauli
    Resolved Bug 1259: converted the test to use pages and blocks rather than a
    byte offset. Removed RandChar(), DclRand() is used instead. The NOR control
    read/write tests are now run as part of the NOR specific tests.
    Revision 1.41  2007/06/22 17:26:01Z  Garyp
    Updated to use the new PerfLog "write" macro, as well as the new "Category
    Suffix" field.
    Revision 1.40  2007/06/15 18:18:03Z  peterb
    Updated the Performance test section to return a failure if any times were
    0.  This was fixed for bug 631.
    Revision 1.39  2007/04/07 23:28:47Z  Garyp
    Modified the latency performance tests to be condition on the value of
    FFXCONF_LATENCYREDUCTIONENABLED.
    Revision 1.38  2007/04/02 18:55:15Z  Garyp
    Updated to support PerfLog build number suffixes.
    Revision 1.37  2007/03/30 19:11:24Z  Garyp
    Modified to display the erase poll interval.
    Revision 1.36  2007/03/05 22:18:51Z  Garyp
    Added NAND specific FMSL performance tests which measure page reads
    and write using ECC.
    Revision 1.35  2007/02/28 00:49:35Z  Garyp
    Moved NOR specific tests into fmslnor.c.
    Revision 1.34  2007/02/06 04:37:53Z  Garyp
    Updated the PerfLog output to dump out the Device raw blocks and total
    blocks values.
    Revision 1.33  2007/02/01 21:22:36Z  Garyp
    Output the latency settings as ints.
    Revision 1.32  2007/02/01 00:40:14Z  Garyp
    Added the /Latency switch.  Modified the performance tests to stop using
    the terms cooked and raw, and rather run three performance test config-
    urations, a default configuration, a "max-perf" configuration, and a
    "minimum-latency" configuration.
    Revision 1.31  2007/01/18 22:19:32Z  Garyp
    Modified so the /PERFLOG switch implies the /P switch.
    Revision 1.30  2007/01/02 04:01:53Z  Garyp
    Updated to interface with PerfLog.
    Revision 1.29  2006/12/16 23:32:44Z  Garyp
    Modified the tests for _sysdelay() to measure using the DCL TimeStamp
    services rather than the (possibly) lower resolution DCL Ticker.
    Revision 1.28  2006/11/15 02:50:07Z  Garyp
    Modified the performance tests so they do not wrap, or try to access the
    same area of flash more than once.  Added "cooked" and "raw" modes for the
    tests to enable testing with the DevMgr latency restrictions turned off
    (raw mode).
    Revision 1.27  2006/10/07 01:56:41Z  Garyp
    Corrected to eliminate warnings generated by the RealView tools.
    Revision 1.26  2006/10/03 23:54:50Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.25  2006/09/07 00:19:29Z  Pauli
    Updated to include the timer/delay portion of the general tests when
    Sibley support is enabled.
    Revision 1.24  2006/05/22 20:49:38Z  Pauli
    Reverting to revision 1.21 to deal with a screwed up check-in to the NAND
    LFA branch.
    Revision 1.21  2006/05/22 20:49:38Z  billr
    Fix type in include file name so this will compile.
    Revision 1.20  2006/05/19 00:43:48Z  Garyp
    Updated to include fmsltst.h.  Eliminated the obsolete structure alignment
    tests.  Modified to call the NOR specific tests in fmslnor.c.
    Revision 1.19  2006/05/17 17:08:50Z  billr
    Resolve IR 4624. Also change test pattern written to produce an ECC that is
    not FF FF FF.
    Revision 1.18  2006/05/12 22:35:43Z  Garyp
    Moved the hidden tests into fmslnand.c into this module where they belong.
    Changed some functions to using the new FML interface macros, rather
    than the old compatibility mode API.  Modified to verbosely display when
    corrected.  Removed dead code.
    single bit errors are encountered.
    Revision 1.17  2006/04/26 19:58:34Z  billr
    Eliminate code to align buffer pointer, it should already be aligned.
    Revision 1.16  2006/03/23 01:02:00Z  billr
    Fix WinCE 4.2 compiler warning about integral size mismatch.
    Revision 1.15  2006/03/23 00:06:52Z  billr
    Correct handling of single-bit errors on NAND flash.
    Revision 1.14  2006/03/12 20:35:40Z  Garyp
    Eliminated the "/x" switch and commented out the extensive hidden write
    tests, which are completely bogus.
    Revision 1.13  2006/03/06 20:45:53Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.12  2006/02/27 06:02:18Z  Garyp
    Tweaked some messages.
    Revision 1.11  2006/02/27 05:16:32Z  Garyp
    Updated to work with the new definition of a NAND tag.
    Revision 1.10  2006/02/22 23:58:34Z  joshuab
    Fix type for constant so it builds correctly on CE4/X86.
    Revision 1.9  2006/02/22 18:21:54Z  Garyp
    Updated debug code.
    Revision 1.8  2006/02/16 20:16:05Z  Pauli
    Added nand support conditions to nand specific code.
    Revision 1.7  2006/02/15 20:45:53Z  Garyp
    Added preprocessor logic around NAND specifc tests.
    Revision 1.6  2006/02/15 09:30:07Z  Garyp
    Eliminated FfxFmlOldRead/Write() support for MEM_HIDDEN.
    Revision 1.5  2006/02/12 18:27:24Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a single
    device/disk at a time.
    Revision 1.4  2006/02/10 08:09:11Z  Garyp
    Updated to use the new FML interface.
    Revision 1.3  2006/01/25 03:50:52Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.2  2006/01/11 01:57:50Z  Garyp
    Updated to use DclStrCat().
    Revision 1.1  2005/11/27 05:51:06Z  Pauli
    Initial revision
    Revision 1.6  2005/11/27 05:51:06Z  Garyp
    Modified so we are no longer calling oemwrite() with MEM_VALIDATE.
    Revision 1.5  2005/11/16 00:47:28Z  Garyp
    Cleaned up the hidden and performance tests.
    Revision 1.4  2005/11/07 06:14:56Z  Garyp
    Merged in changes from Cheryl and Bill to support IWSF and fmslnand.c,
    respectively.
    Revision 1.3  2005/10/20 03:23:01Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.2  2005/10/09 23:07:11Z  Garyp
    Removed singlethreaded FIM support.
    Revision 1.1  2005/10/04 21:44:42Z  Garyp
    Initial revision
    Revision 1.5  2005/09/20 19:01:10Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.4  2005/09/18 00:09:08Z  garyp
    Modified so that _sysdelay() and related code is only built if
    FFXCONF_NORSUPPORT is TRUE.
    Revision 1.3  2005/09/08 22:31:03Z  garyp
    Added a helper function to erase the entire media.
    Revision 1.2  2005/08/21 10:42:03Z  garyp
    Eliminated // comments.
    Revision 1.1  2005/08/03 19:31:18Z  pauli
    Initial revision
    Revision 1.3  2005/08/03 19:31:18Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/29 19:29:11Z  Garyp
    No longer use EXTND_DEV_EXISTS.
    Revision 1.1  2005/07/28 19:12:18Z  pauli
    Initial revision
    Revision 1.35  2005/05/03 01:26:06Z  garyp
    Removed references to some obsoleted structure fields.
    Revision 1.34  2005/02/25 19:05:26Z  billr
    Use FfxRand() instead of rand() so stdlib.h isn't needed.
    Revision 1.33  2004/12/30 17:32:43Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.32  2004/12/17 00:08:29Z  garys
    Removed redundant addition of FFX_RESERVED_SPACE to small block testing
    offset in flash.
    Revision 1.31  2004/11/18 19:29:28  garys
    Fixed MAX_BIG_BLOCK_OPS, so perf test doesn't terminate early.
    Revision 1.30  2004/10/28 00:29:11  GaryP
    Added some error handling code for the not so bogus asserts removed in
    the previous revision.
    Revision 1.29  2004/10/27 22:07:02Z  GaryP
    Eliminated the use of D_STATIC.  Removed some bogus asserts.
    Revision 1.28  2004/10/07 18:40:42Z  jaredw
    Now test by default respects FFX_RESERVED_SPACE, this can be changed by
    using new /r switch.
    Revision 1.27  2004/09/25 03:11:56Z  GaryP
    Commented out the error return code for the _sysdelay() tests.
    Revision 1.26  2004/09/23 05:57:50Z  GaryP
    Fixed compiler warnings and printf formatting syntax.
    Revision 1.25  2004/09/23 02:31:40Z  GaryP
    Made the _systimeout() tests conditional on FFXCONF_ENABLEOLDSTYLEFIMS.
    Revision 1.24  2004/09/23 00:29:20Z  jaredw
    Fixed sysdelay test to delay a multiple of clock ticks so we can use
    the tick count as an accurate means to test the delay.
    Revision 1.23  2004/08/10 08:13:38Z  garyp
    Enhanced the _sysdelay() tests.
    Revision 1.22  2004/08/04 22:01:15Z  billr
    Eliminate ulInterleaved in ExtndMediaInfo, no longer used or needed.
    Revision 1.21  2004/07/29 02:52:48Z  jaredw
    Fixed bug where a nor fim was being called with MEM_VALIDATE in
    performance tests.
    Revision 1.20  2004/07/20 00:09:03Z  GaryP
    Minor include file updates.
    Revision 1.19  2004/07/02 18:55:00Z  jaredw
    Revision 1.18  2003/07/02 17:29:50Z  jaredw
    Fixed bug where a nor fim was being called with MEM_VALIDATE in extensive
    writes test.
    Revision 1.17  2004/05/05 20:05:55Z  billr
    Thread-safe timers based on system clock tick.
    Revision 1.16  2004/02/22 19:17:10Z  garys
    Merge from FlashFXMT
    Revision 1.13.1.4  2004/02/22 19:17:10  garyp
    Eliminated the removable and writeprotect tests which relied on the now
    obsolete oemchanged() function.
    Revision 1.13.1.3  2003/11/14 21:38:26Z  garyp
    Minor variable renaming.
    Revision 1.13.1.2  2003/11/04 21:31:18Z  garyp
    Re-checked into variant sandbox.
    Revision 1.14  2003/11/04 21:31:18Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.13  2003/08/15 17:14:00Z  garyp
    Reduced the volume of output in the extensive write tests.
    Revision 1.12  2003/08/07 00:31:08Z  garyp
    Changed two "WARNING!"s to "NOTE:"s
    Revision 1.11  2003/05/29 06:31:18Z  garyp
    Eliminated the STDDEF.H inclusion since it's now done in FLASHFX.H as
    needed.
    Revision 1.10  2003/05/22 19:42:14Z  garyp
    Fixed to initialize the default test settings at run-time rather than
    compile time to avoid problems in VxWorks where the global values from
    previous test executions are persistent.
    Revision 1.9  2003/05/16 00:45:57Z  garys
    Check return codes from calls to WriteAndVerify(), EraseAndVerify(), etc.
    Revision 1.8  2003/05/15 17:42:02  garyp
    Enhanced the timer tests.  Modified to reduce the output level.
    Revision 1.7  2003/05/05 20:33:22Z  garyp
    Updated for new UNITTEST.H location.
    Revision 1.6  2003/04/22 00:27:25Z  garys
    stddef.h cannot be included under CE 3.0
    Revision 1.5  2003/04/16 00:04:06  garyp
    Updated to include STDDEF.H (again).
    Revision 1.4  2003/04/15 21:27:46Z  garyp
    Header changes.
    Revision 1.3  2003/04/15 18:58:20Z  garyp
    Updated to use FFX internal string routines.  Eliminated use of __D_FAR
    where possible.
    Revision 1.2  2003/03/28 21:11:38Z  garyp
    Added _syssleep() tests and changed the _sysdelay test.
    Revision 1.1  2003/03/26 09:22:56Z  garyp
    Initial revision
    Revision 1.21  2003/02/13 19:32:11Z  garys
    Don't include stddef.h for CE 3.0
    Revision 1.20  2002/12/17 00:48:04  garys
    restored AlignAddress() and AlignDword() functions
    Revision 1.18  2002/12/04 20:31:28Z  garyp
    Changed FAML references to FMSL.
    Revision 1.17  2002/12/04 15:47:36Z  jimp
    Added check for redundant areas in ReadEraseAndVerify function.  Initialized
    buffer used in GetWritePerformance test.  Checked for divide by zero in
    Performance tests.  Clarified /W switch description.  Added time-out to
    RemovableTests.
    Revision 1.16  2002/12/02 23:15:18Z  jimp
    Revised HiddenAccess test to do complementary write.  Added /F switch for
    user requested formatting of BBM layer.  Revised ExtensiveWrite test to
    change the test bit pattern used.  General code clean-up (clarify comments,
    replace literals, etc).  Revise ReadEraseAndVerify function to handle
    requests longer than one EraseZone.
    Revision 1.15  2002/12/01 02:51:48Z  dennis
    Removed NVBF dependencies from FMSL and tools.  Added defines to OEM.H for
    the data, redundant and page size of NAND (MGM) memories in the max and
    standard cases.  For now, the two cases are identical.
    Revision 1.14  2002/11/23 03:06:06  garyp
    Updated to include STDDEF.H to satisfy the ARM tools.
    Revision 1.13  2002/11/16 06:36:14Z  garys
    Moved ulAddress variable in ExtensiveWrites() to avoid strange compiler
    error about a null character.
    Revision 1.12  2002/11/15 09:24:48  garyp
    Fixed to move STRING.H ahead of the other includes so that STDDEF.H will
    also be included first (necessary for BC builds).
    Revision 1.11  2002/11/15 05:39:24Z  garyp
    Eliminated STDDEF.H
    Revision 1.10  2002/11/14 20:18:22Z  garys
    Fixed /c option, passing parameters to oeminit in SetSwitches(), as long as
    there are no spaces between them.
    Revision 1.9  2002/11/13 07:18:15  garys
    Changed FfxCheckTimer() param from D_FAR to __D_FAR and added a new file
    header.
    11/09/02 gjs Moved StartTimer() and CheckTimer() to common\util\timer.c
    11/08/02 gjs Moved VMAPI declaration before code in FlashFXTestFAML()
    11/08/02 gjs Changed entry point from FlashFXTestFMSL() to FlashFXTestFAML()
    11/07/02 gjs SetSwitches() now handles OS-generic command line strings
                 Changed usage to display TESTFMSL instead of TESTOEM
    11/05/02 gjs Minimum arg count is 1 (program name is no longer 1st arg) GP
                 removed OS-specific code, changed main() to FlashFXTestFMSL()
                 changed printf() to FfxPrintf(), and removed DisplaySignOn().
    08/29/02 gjs Removed farfree and farmalloc, which fail to compile under CE
    07/29/02 de  Included NVBF.H and added symbolic constants.
    07/22/02 de  Some of these tests violate the write limits on newer NAND
                 flash and those tests have been made illegal when NVBF is
                 defined. Changed extensive write test to format every block.
    07/22/02 de  Added calls to allocate and free a buffer used to verify that
                 we don't exceed 1 write to a main page and 2 writes to a
                 redundant page of NAND flash when run under DOS.  Added
                 Extended Device structure member tests.
    04/04/02 gjs Cast IS_ALIGNED output to D_BOOL for SetTestRecord() input
                 on particular lines for the CE 2.10 SA1100 compiler.
    03/19/02 gjs Fixed usage for /C option (space before the string).
    02/13/02 gjs Added CheckOEMStructs() with DE.
                 Call TestTypeSizes() in TestOemLayer().
    01/08/02 gjs Rewrote type size checks and moved to TestTypeSizes().  Added
                 return code (error level) defines.
    01/02/02 gjs Removed local implementation of _sysassert and rewrote
                 AlignAddress to avoid DOS GNU 2.01 link warnings.
    11/13/01 gjs Assert in GetReadPerform() to make sure pcBigBlockBuffer
                 can be aligned.
    11/09/01 gjs Fixed AlignAddress macro to increment 0 to (max - 1)
                 instead of 1 to max.  Also fixed to compile for QNX.
    11/08/01 gjs DWord aligned pcBigBlockBuffer in GetReadPerform().
                 Removed printfs from loops in Perform routines.
    10/05/01 gjs Added a call to EraseAndVerify() in BlockWrites() to handle
                 small erase zones.
    09/28/01 gjs Added printf's to failure case in the Verify() routine.
    05/06/01 PKG Added type casts to string literals to the type of variable
                 declared to placate picky compilers. (NZ/OSE)
    05/06/01 PKG Changed FlashFX specific includes to use quotes (NZ/OSE)
    01/09/01 GP  Updated to build with the __WINNT_TARGET__
    12/01/00 DE  Removed redundant assignment statement in SetSwitches().
    12/01/00 DE  Corrected wording of "/E" option in help screen text.
    12/01/00 DE  Changed type of second argument to FillBuffer() from D_UINT16
                 to D_UINT32 to allow signing each page with it's logical
                 address.
    12/01/00 DE  Changed ExtensiveWrites() to sign each test buffer using
                 the PATTERN type.  This test now makes a second pass through
                 the entire media, reading each bit of data to verify that the
                 proper verified patterns still exist throughout the disk once
                 the entire disk has been filled.
    12/01/00 DE  Added a ReadAndVerify() function that reads data out of a
                 specified location in the flash and validates it against the
                 contents of a previously filled buffer. This routine is used
                 during the second pass in ExtensiveWrites() to facilitate
                 manually testing the BBM/FIM block replacement.
    12/01/00 DE  Refactored WriteAndVerify() to call ReadAndVerify().  Updated
                 comments.
    09/14/00 HDS Moved #include <flashfx.h> statement to top to insure that
                 __LINUX_TARGET__ is defined before it is used.
    07/29/00 HDS Fixed command line option /c processing.
    06/23/00 mrm Added __TEST_OEM define to ffxconf.h
    01/25/00 HDS Added auto BBM format at start of execution when /x option
                 (Extensive Hidden Tests) selected.
    11/11/99 AJC Changed main() to be callable from the VxWorks shell.
    11/03/99 TWQ Updated with Linux support
    09/02/99 AJC Don't include conio.h under Linux
    04/28/99 HDS Changed the GetErasePerform Function to provide a more
                 accurate erase performance value by erasing blocks throughout
                 the flash instead of only the first block.
    03/05/99 gjs Init bStatus to TRUE in GeneralTests(), to avoid CE 2.0 SH3
                 compiler warns
    12/28/98 PKG Removed debug code accidently left from prior revision
    12/16/98 DM  Added support test code for the VMAPI
    12/16/98 PKG Changed SetTimer to StartTimer to avoid clash with Windows API
                 SetTimer function call.
    12/15/98 PKG Added direct call to stub_printf to avoid compiler warns
    12/13/98 PKG Stub FfxPrintf() calls when in a NO_DISPLAY environment
    09/15/98 PKG Added dllimport declarator to printf to avoid copmiler warns
    08/06/98 PKG Fixed to erase test area only if flash - not RAM.
    08/05/98 PKG Added proto-type for the local FfxPrintf()
    07/20/98 PKG Changed the Performance Test to write before read and to use
                 the ECC.
    07/02/98 HDS Added a memory type parameter to the WriteAndVerify calling
                 sequence to allow memory validation (ECC) for the Extensive
                 Write Test.
    06/17/98 HDS Changed erase zone size too small message in VerifyZoneSize
                 so doesn't imply an error, but only that test can't be run.
    05/08/98 HDS Changed ReadEraseAndVerify printf to indicate erase verify
                 error instead of OEM erase error.
    05/06/98 HDS Incorporated AND device support.
    05/06/98 HDS corrected Verify Zone Size Test to bypass test if erase zone
                 size too small.
    04/28/98 PKG Fixed Extensive hidden tests to work with 16Mbit
                 devices since their erase zone size is smaller.
    03/10/98 PKG Fixed build of unit test in standalone environment
    03/09/98 PKG Removed declaration of assert if the unit test is linked in
                 with the driver, in which case it is not needed
    03/04/98 TWQ Enhanced big block test to catch pointer manipulation issues
                 in the FIM and setup pattern to be modulo 100 decimal
    03/04/98 HDS Modified Hidden Access Test part 2 to write zeros instead of
                 the reverse pattern.
    03/03/98 HDS Added an extensive hidden write test (/X).
    01/09/98 HDS Fixed ReadEraseAndVerify Function so would not hang-up when
                 detecting an erase failure.
    12/09/97 PKG Fixed type cast from main to SetCmdLineArgs
    12/04/97 HDS Added drive number to test display.
    12/04/97 HDS Changed command line parameter from /Ux to /Dx.
    11/25/97 HDS Changed oemsetunit() call to oemsetcurrentdrive().
    11/20/97 PKG Added tests for machine module.
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    09/12/97 DE  Added test for removable media and write protect
    08/26/97 PKG Now uses new Datalight specific types
    08/25/97 PKG Reduced size of the BIG_BLOCK tests to 16KB
    08/12/97 PKG Fixed the butterfly write test to work with extremely small
                 disks and fully tested with a wide rang of small flash arrays
                 from zone sizes of 8K to 128K and disks from 16K to 256K
    07/24/97 PKG Changed /S option to U for consistant naming
    06/25/97 DM  Dual drive support works now. uLastUnit while loop fixed.
    06/12/97 PKG Fixed write tests to verify suitable size before performing
                 WriteBlockTests(), Fixed WritePerformance() to avoid attempting
                 to program bits from 0 to 1
    06/11/97 PKG Removed external API compile options
    05/29/97 PKG Added DEV_MASK to identify flash parts
    05/15/97 PKG Fixed bug in performance tests, now pre-erase
    05/08/97 DM  Added performance option and performance routines
    05/02/97 PKG Removed un-needed include files
    04/25/97 PKG Removed use of randomize(), not ANSI C
    01/06/07 PKG Created
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxinfohelp.h>
#include <dlperflog.h>
#include <dlservice.h>
#include "fmsltst.h"

#define CHECKER1_VAL 0x55
#define CHECKER2_VAL 0xAA

#define ARGBUFFLEN            128

/*  Types of data to fill in the data buffer
*/
#define RANDOM       0
#define CHECKER1     1
#define CHECKER2     2
#define PATTERN      3
#define CONSTANT     4
#define ZEROED       5
#define PAGE_PATTERN 6

#define FILL_VALUE      0xD5

/*  Possible error levels to return
*/
#define ERROR_PASSED             0
#define ERROR_USAGE              1
#define ERROR_BAD_OPTION         2
#define ERROR_BAD_PARAMETER      3
#define ERROR_OUTOFMEMORY        4
#define ERROR_GENERAL_TESTS      19
#define ERROR_ZONE_SIZE          20
#define ERROR_BLOCK_WRITE        23
#define ERROR_HUGE_TESTS         24
#define ERROR_BUTTERFLY_WRITE    25
#define ERROR_INTENSIVE_WRITE    30
#define ERROR_EXTENSIVE_WRITE    31
#define ERROR_ALIGNMENT_TESTS    32
#define ERROR_NAND_TESTS         33
#define ERROR_NOR_TESTS          34
#define ERROR_STRESS_TESTS       35
#define ERROR_LOCK_TESTS         36
#define ERROR_PERFORMANCE        42

#define PERF_SECONDS            (10)        /*  Seconds to run each perf test */
#define MSEC_PER_SEC            (1000UL)

/*  The following buffers are used throughout the unit test to verify
    data access into the media.

    TEST_BLOCK_SIZE must be a multiple of the flash page size and must
    be at least as big as the largest supported page size.  Additionally,
    it must be no larger than half the size of an erase zone.

    TODO:  These sizes should be dynamically determined at the beginning
    of the test based on the geometry of the media being tested.  For some
    tests, these values should be as large as possible and for other tests
    they should be as small as possible.
*/
#define TEST_BLOCK_SIZE FFX_MAX_PAGESIZE
#define SINGLE_PAGE_TEST_BLOCK 1


/*  Level of testing verification thoroughness. that is not the number of
    test cases that are run, but the how much of the disk is checked for side effects.
*/

#define EXHAUSTIVE       1
#define BOUNDARIES_ONLY  2


/*-------------------------------------------------------------------
                              Prototypes
-------------------------------------------------------------------*/

static D_INT16  SetSwitches(            FFXTOOLPARAMS *pTP, FFXFMLHANDLE hFML);
static void     ShowUsage(              FFXTOOLPARAMS *pTP);
static D_INT16  TestFMSL(               FFXFMLHANDLE hFML);
static D_BOOL   AlignmentTests(         FFXFMLHANDLE hFML);
static D_BOOL   GeneralTests(           FFXFMLHANDLE hFML);
static D_BOOL   HugeOperationTests(     FFXFMLHANDLE hFML);
static D_BOOL   VerifyBlockSize(        FFXFMLHANDLE hFML);
static D_BOOL   BasicReadWrite(         FFXFMLHANDLE hFML);
static D_BOOL   ButterflyWrites(        FFXFMLHANDLE hFML);
static D_BOOL   IntensiveWrites(        FFXFMLHANDLE hFML);
static D_BOOL   ExtensiveWrites(        FFXFMLHANDLE hFML, D_UINT32 ulCount);
static D_BOOL   EraseMedia(             FFXFMLHANDLE hFML, FFXFMLINFO *pMI, unsigned fQuiet);
static D_BOOL   EraseIfNecessary(       FFXFMLHANDLE hFML, D_UINT32 ulBlock, D_UINT32 ulCount);
static D_BOOL   EraseAndVerify(         FFXFMLHANDLE hFML, D_UINT32 ulBlock, D_UINT32 ulCount);
static D_BOOL   Performance(            FFXFMLHANDLE hFML);
static D_UINT32 GetReadPerform(         FFXFMLHANDLE hFML, unsigned nCount);
static D_UINT32 GetWritePerform(        FFXFMLHANDLE hFML, unsigned nCount);
static D_UINT32 GetErasePerform(        FFXFMLHANDLE hFML);
static D_BOOL   WriteAndVerify(         FFXFMLHANDLE hFML, D_UINT32 ulPage, const D_BUFFER *pcOriginalDataBuffer, D_UINT32 ulBufferSize);
static D_BOOL   ReadAndVerify(          FFXFMLHANDLE hFML, D_UINT32 ulPage, const D_BUFFER *pcOriginalDataBuffer, D_UINT32 ulBufferSize);
static D_BOOL   Verify(                 const D_BUFFER *pcOriginalDataBuffer, const D_BUFFER *pcDataReadBack, D_UINT32 ulBufferSize);
static void     FillBuffer(             D_UINT16 uType, D_UINT32 ulData,D_BUFFER *pcOriginalDataBuffer, D_UINT32 ulBufferSize );
static D_BOOL   RunPerfIterations(      FFXFMLHANDLE hFML, const char *pszPerfLogName, unsigned nMinPages, unsigned nMaxPages);

static D_BOOL   fHugeTests;
static D_BOOL   fAlignmentTests;
static D_BOOL   fExtensiveTests;
static D_BOOL   fPerformanceTests;
static D_BOOL   fPerfLatency;
static D_BOOL   fPerfLog;
static D_BOOL   fStress;
#if FFXCONF_NANDSUPPORT
static D_BOOL   fBBMTests;
static D_BOOL   fRetireBlockTest;
static D_BOOL   fLockTests;
#endif
static char     szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
static unsigned uVerbosity;
static D_UINT32 ulStressMinutes;
static D_UINT32 ulAlignmentMin;
static D_UINT32 ulExtensiveMinutes;

/*  Device types recognized
*/
static const char *szDeviceArray[] =
{
    (char *) "ROM",
    (char *) "RAM",
    (char *) "ATA",
    (char *) "NOR Flash",
    (char *) "NAND Flash",
    (char *) "AND Flash",
#if FFXCONF_ISWFSUPPORT
    (char *) "ISW Flash",
#endif
};

static D_UINT32 ulRandomSeed;
static D_UINT32 ulOriginalSeed;
static D_UINT32 ulBigBlockSize;
static D_BUFFER *pcBigBlockBuffer;


/*-------------------------------------------------------------------
    Protected: FfxTestFMSL()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxTestFMSL(
    FFXTOOLPARAMS  *pTP)
{
    D_INT16         iErrorLevel;
    FFXFMLHANDLE    hFML;

    DclPrintf("FlashFX FMSL Unit Test\n");
    FfxSignOn(FALSE);

    /*  Initialize default test values.  We MUST do this on entry
        into the test because in some environments, such as VxWorks,
        the variables remain static between test executions.
    */
    fHugeTests = FALSE;
    fAlignmentTests = FALSE;
    fExtensiveTests = FALSE;
    fStress = FALSE;
    fPerformanceTests = FALSE;
    fPerfLatency = FALSE;
    fPerfLog = FALSE;
  #if FFXCONF_NANDSUPPORT
    fRetireBlockTest = FALSE;
    fLockTests = FALSE;
    fBBMTests = FALSE;
  #endif
    szPerfLogSuffix[0] = 0;
    ulRandomSeed = DclOsTickCount();
    ulStressMinutes = 0;
    ulExtensiveMinutes = 0;
    uVerbosity = DCL_VERBOSE_NORMAL;
    pcBigBlockBuffer = NULL;
    ulBigBlockSize = 0;

    hFML = FfxFmlHandle(pTP->nDiskNum);
    if(!hFML)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    iErrorLevel = SetSwitches(pTP, hFML);

    if(iErrorLevel != ERROR_PASSED)
        return iErrorLevel;

    /*  Display the random seed both before and after the test so we
        have a better chance of obtaining it if we need to recreate
        a problem.
    */
    ulOriginalSeed = ulRandomSeed;
    DclPrintf("Random Seed = %lU\n", ulRandomSeed);

    /*  Go perform the unit tests
    */
    iErrorLevel = TestFMSL(hFML);

    if(iErrorLevel)
    {
        DclPrintf("FMSL Unit Test FAILED, Error Code = %D, Random Seed = %lU\n",
                  iErrorLevel, ulOriginalSeed);
    }
    else
    {
        DclPrintf("FMSL Unit Test PASSED\n");
    }

    return iErrorLevel;
}


/*-------------------------------------------------------------------
    Local: TestFMSL()

    All the tests are called from this function rather than the
    main.  In some development environments linking the unit
    test into another application may be required.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        0        - Tests passed
        non-zero - test number that failed
-------------------------------------------------------------------*/
static D_INT16 TestFMSL(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
    D_INT16         iReturn = ERROR_PASSED;

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        iReturn = ERROR_BAD_PARAMETER;
        goto Cleanup;
    }

    /*  ulBigBlockSize must be a multiple of the page size and at least 2x
        the size of TEST_BLOCK_SIZE.  Additionally, it must not be larger
        than an erase zone.

        Therefore, make it exactly the size of an erase block.
    */
    ulBigBlockSize = FmlInfo.ulBlockSize;
    pcBigBlockBuffer = DclMemAlloc(ulBigBlockSize);
    if(!pcBigBlockBuffer)
    {
        iReturn = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if(!GeneralTests(hFML))
    {
        iReturn = ERROR_GENERAL_TESTS;
        goto Cleanup;
    }

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    if(FmlInfo.uDeviceType != DEVTYPE_NAND)
    {
        if(!FfxFmslNORTest(hFML, &ulRandomSeed, fExtensiveTests))
        {
            iReturn = ERROR_NOR_TESTS;
            goto Cleanup;
        }
    }
  #endif

    if(!VerifyBlockSize(hFML))
    {
        iReturn = ERROR_ZONE_SIZE;
        goto Cleanup;
    }

    if(!BasicReadWrite(hFML))
    {
        iReturn = ERROR_BLOCK_WRITE;
        goto Cleanup;
    }

    if(fHugeTests)
    {
        if(!HugeOperationTests(hFML))
        {
            iReturn = ERROR_HUGE_TESTS;
            goto Cleanup;
        }
    }

    if(fExtensiveTests)
    {
        if(!ExtensiveWrites(hFML, ulExtensiveMinutes))
        {
            iReturn = ERROR_EXTENSIVE_WRITE;
            goto Cleanup;
        }
    }

    if(fAlignmentTests)
    {
        if(!AlignmentTests(hFML))
        {
            iReturn = ERROR_ALIGNMENT_TESTS;
            goto Cleanup;
        }
    }

    if(FmlInfo.uDeviceType != DEVTYPE_NAND)
    {
        /*  These tests violate the write-cycle limits on many NAND
            devices.
        */
        if(!ButterflyWrites(hFML))
        {
            iReturn = ERROR_BUTTERFLY_WRITE;
            goto Cleanup;
        }

        if(!IntensiveWrites(hFML))
        {
            iReturn = ERROR_INTENSIVE_WRITE;
            goto Cleanup;
        }
    }

  #if FFXCONF_NANDSUPPORT
    /*  Tests for NAND flash only.
    */
    if(FmlInfo.uDeviceType == DEVTYPE_NAND)
    {
        if(!FfxFmslNANDTest(hFML, 0, pcBigBlockBuffer, ulBigBlockSize, &ulRandomSeed, fExtensiveTests, fRetireBlockTest, fBBMTests, uVerbosity))
        {
            iReturn = ERROR_NAND_TESTS;
            goto Cleanup;
        }
 
        /*  At some point the block locking tests should be generalized
            for NOR as well, but only on NAND for now...
        */            
        if(fLockTests)
        {
            FFXSTATUS   ffxStat;
        
            ffxStat = FfxFmslLockTest(hFML, &ulRandomSeed, uVerbosity);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("Block locking tests failed with status %lX\n", ffxStat);
                iReturn = ERROR_LOCK_TESTS;
                goto Cleanup;
            }
        }
    }
  #endif

    if(fStress)
    {
        if(!FfxFmslStress(hFML, ulStressMinutes, &ulRandomSeed, fExtensiveTests, uVerbosity))
        {
            iReturn = ERROR_STRESS_TESTS;
            goto Cleanup;
        }
    }

    if(fPerformanceTests)
    {
        if(!Performance(hFML))
        {
            iReturn = ERROR_PERFORMANCE;
            goto Cleanup;
        }
    }

  Cleanup:

    if(pcBigBlockBuffer)
    {
        DclMemFree(pcBigBlockBuffer);
        pcBigBlockBuffer = NULL;
    }

    return iReturn;
}


/*--------------------------------------------------------------------------
                     INDIVIDUAL TESTS
--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Local: GeneralTests()

    Run timer tests.
    Obtains some general information about the media from
    the FMSL and reports it to the user.
    Obtain status info and report it.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.  An error message is
                print displayed before return
-------------------------------------------------------------------*/
static D_BOOL GeneralTests(
    FFXFMLHANDLE        hFML)
{
    D_BOOL              bStatus = TRUE;
    FFXFMLINFO          FmlInfo;
    FFXFMLDEVINFO       FmlDevInfo;

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    /*  The _sysdelay() function only builds if we are using NOR or
        Sibley, so only build this test if that support is enabled.

        TODO: This should be part of the NOR specific tests.
    */
    if(!FfxFmslNORSysDelay())
        return FALSE;
  #endif

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    DclPrintf("\n#####################  Device and Disk Information  ######################\n");
    
    if(FfxDisplayDiskInfo(hFML, TRUE) != FFXSTAT_SUCCESS)
        return FALSE;

    DclPrintf("##########################################################################\n\n");

    if(FmlInfo.ulTotalBlocks < 2)
    {
        DclPrintf("    Only one erase zone found, unable to continue!\n");
        bStatus = FALSE;
    }

    /*  Flash Must have a zone size
    */
    if(FmlInfo.ulBlockSize == 0)
    {
        DclPrintf("    ERROR: Erase block size is zero\n");
        bStatus = FALSE;
    }

    /*  Flash must have a page size
    */
    if(FmlInfo.uPageSize == 0)
    {
        DclPrintf("    ERROR: Page size is zero\n");
        bStatus = FALSE;
    }

    /*  Page size must be <= TEST_BLOCK_SIZE
    */
    if(FmlInfo.uPageSize > TEST_BLOCK_SIZE)
    {
        DclPrintf("    ERROR: Page size is too large\n");
        bStatus = FALSE;
    }

    /*  Erase block size must be >= ulBigBlockSize
    */
    if(FmlInfo.ulBlockSize < ulBigBlockSize)
    {
        DclPrintf("    ERROR: Block size is too small\n");
        bStatus = FALSE;
    }

    DclProductionAssert(TEST_BLOCK_SIZE <= ulBigBlockSize/2);

  #if FFXCONF_NANDSUPPORT
    if(FmlInfo.uDeviceType == DEVTYPE_NAND)
    {
        /*  NAND flash must have a spare size
        */
        if(FmlInfo.uSpareSize == 0)
        {
            DclPrintf("    ERROR: NAND spare size is zero\n");
            bStatus = FALSE;
        }
    }
  #endif

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffmsl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(fPerfLog, NULL, "FMSL", "Configuration", NULL, szPerfLogSuffix);
        DCLPERFLOG_STRING(hPerfLog, "DevType",       szDeviceArray[FmlDevInfo.uDeviceType]);
        DCLPERFLOG_HEX16( hPerfLog, "DevFlags",      FmlDevInfo.uDeviceFlags);
        DCLPERFLOG_NUM  ( hPerfLog, "DevRawBlocks",  FmlDevInfo.ulRawBlocks);
        DCLPERFLOG_NUM  ( hPerfLog, "DevTotalBlocks",FmlDevInfo.ulTotalBlocks);
        DCLPERFLOG_HEX16( hPerfLog, "DiskFlags",     FmlInfo.uDiskFlags);
        DCLPERFLOG_NUM(   hPerfLog, "BlockCount",    FmlInfo.ulTotalBlocks);
        DCLPERFLOG_NUM(   hPerfLog, "BlockSizeKB",   FmlInfo.ulBlockSize / 1024UL);
        DCLPERFLOG_NUM(   hPerfLog, "PageSize",      FmlInfo.uPageSize);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    /*  A failure may have occurred
    */
    return bStatus;
}


/*-------------------------------------------------------------------
    Local: HugeOperationTests()

    Performs huge operations.  Attempts to read/write up to
    HUGE_BLOCK_COUNT or HUGE_MIN_BYTES of data in a single
    request, which ever is greater.

    Depending upon the flash configuration and the available
    memory it may not be possible to allocate the memory
    required to run this test.  In this case, the test will
    report failure.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.  An error message is
                displayed before return
-------------------------------------------------------------------*/
static D_BOOL HugeOperationTests(
    FFXFMLHANDLE    hFML)
{
    #define             HUGE_MIN_BYTES      (256 * 1024UL)
    #define             HUGE_BLOCK_COUNT    (2)
    D_BUFFER       *pBuffer = NULL;
    D_UINT32        ulBlock = 0;
    D_UINT32        ulBlockCount;
    D_UINT32        ulPage;
    D_UINT32        ulPageCount;
    D_UINT32        ulPagesPerBlock;
    D_UINT32        ulMaxPages;
    D_UINT32        ulTestBytes;
    D_UINT32        i, j;
    D_BUFFER        uTestData = 0xA0;
    FFXIOSTATUS     ioStat;
    D_BOOL          fPass = FALSE;
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;

    DclPrintf("Testing Huge Operations...\n");

    /*  Get the disk and device information.
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;
    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    /*  Calculate the test conditions.

        First determine the number of pages (in even multiples of the block
        size) needed to reach the desired target size.
    */
    ulTestBytes = DCLMAX(HUGE_MIN_BYTES, HUGE_BLOCK_COUNT * FmlInfo.ulBlockSize);
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulBlockCount = 0;
    ulMaxPages = 0;
    while((ulMaxPages * FmlInfo.uPageSize) < ulTestBytes)
    {
        ulBlockCount++;
        ulMaxPages = ulBlockCount * ulPagesPerBlock;
    }

    /*  Allocate a buffer for the test.
    */
    pBuffer = DclMemAlloc(FmlInfo.uPageSize * ulMaxPages);
    if(!pBuffer)
    {
        DclPrintf("    Error: Unable to allocate page buffer of %lU bytes\n", FmlInfo.uPageSize * ulMaxPages);
        DclPrintf("    SKIPPED\n");

        /*  Say it passed even though it was really skipped.
        */
        fPass = TRUE;
        goto HugeTestCleanup;
    }

    ulPage = ulBlock * ulPagesPerBlock;
    if(uVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  Testing up to %lU pages (%lU blocks, %lU KB) starting at page %lU\n",
                  ulMaxPages, ulBlockCount,
                  (ulMaxPages * FmlInfo.uPageSize) / 1024, ulPage);
    }

    /*  Perform each test case.
    */
    for(i = 1; i <= ulBlockCount; i++)
    {
        ulPageCount = ulPagesPerBlock * i;
        ulTestBytes = ulPageCount * FmlInfo.uPageSize;
        if(uVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("  Testing %lU pages\n", ulPageCount);

        /*  Set the buffer to a known state.
        */
        uTestData++;
        if(uVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("    Test Data = 0x%2X\n", uTestData);
        DclMemSet(pBuffer, uTestData, ulTestBytes);

        /*  Erase the blocks we are about to write to.
        */
        if(uVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("    Erasing...\n");
        if(!EraseAndVerify(hFML, ulBlock, i))
        {
            goto HugeTestCleanup;
        }

        /*  Write pages.
        */
        if(uVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("    Writing...\n");

        FMLWRITE_PAGES(hFML, ulPage, ulPageCount, pBuffer, ioStat);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("    Writing to page %lU failed!\n", ulPage);
            goto HugeTestCleanup;
        }

        /*  Read pages.
        */
        if(uVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("    Reading...\n");
        DclMemSet(pBuffer, 0, ulTestBytes);
        ioStat = FfxFmslTestReadPages(hFML, ulPage, ulPageCount, pBuffer, READ_PAGES);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("    Reading from page %lU failed!\n", ulPage);
            goto HugeTestCleanup;
        }

        /*  Verify the data.
        */
        if(uVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("    Verifying...\n");
        for(j = 0; j < ulTestBytes; j++)
        {
            if(pBuffer[j] != uTestData)
            {
                DclPrintf("    Data mismatch at offset %U, expected 0x%2X\n", j, uTestData);
                DclHexDump("    Buffer:\n", HEXDUMP_UINT8, 16, ulTestBytes, pBuffer);
                goto HugeTestCleanup;
            }
        }
    }

    DclPrintf("  PASSED\n");
    fPass = TRUE;

  HugeTestCleanup:

    if(pBuffer)
        DclMemFree(pBuffer);

    return fPass;
}


/*-------------------------------------------------------------------
    Local: AlignmentTests()

    Performs page read/write operations with misaligned buffers.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.  An error message is
                displayed before return
-------------------------------------------------------------------*/
static D_BOOL AlignmentTests(
    FFXFMLHANDLE    hFML)
{
    D_BUFFER       *pWriteBuffer = NULL;
    D_BUFFER       *pReadBuffer = NULL;
    D_BUFFER       *pWrite = NULL;
    D_BUFFER       *pRead = NULL;
    D_UINT32        ulBlock;
    D_UINT32        ulBlockCount;
    D_UINT32        ulPage;
    D_UINT32        ulPagesPerBlock;
    FFXIOSTATUS     ioStat;
    D_BOOL          fPass = FALSE;
    D_UINT32        ulBufferSize;
    D_UINT16        i, j, k;
    D_UINT16        uMaxAlignment = DCL_ALIGNSIZE;
    D_UINT16        uMinAlignment;
    D_UINT16        uAlignCases;
    D_UINT16        uMinPages = 1; /* minimum page count to test */
    D_UINT16        uMaxPages = 2; /* maximum page count to test */
    D_UINT16        uTotalPages;
    D_BOOL          fForceAlignedIO;
    DCLSTATUS       dclStatus;
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;

    DclPrintf("Testing Alignment...\n");

    /*  Get the disk and device information.
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    uMinAlignment = FmlDevInfo.uAlignSize;
    DclAssert(DCLISPOWEROF2(uMinAlignment));
    DclAssert(uMinAlignment <= uMaxAlignment);

    /*  Determine the alignment boundaries to test.
    */
    fForceAlignedIO = (D_BOOL)(FFXCONF_FORCEALIGNEDIO && (uMinAlignment > 1));
    if(fForceAlignedIO)
        uMinAlignment = 1;

    DclPrintf("  The Device reports %U-bit as the minimum allowable alignment.\n",
              FmlDevInfo.uAlignSize * 8);

    DclPrintf("  The FFXCONF_FORCEALIGNEDIO feature is %s for this Device.\n",
              fForceAlignedIO ? "enabled" : "disabled");

    if(ulAlignmentMin && ulAlignmentMin != uMinAlignment)
    {
        if(ulAlignmentMin <= FmlDevInfo.uAlignSize)
        {
            DclPrintf("  Overriding default test minimum alignment (%U) with %lU byte(s)\n",
                uMinAlignment, ulAlignmentMin);

            uMinAlignment = (D_UINT16)ulAlignmentMin;
        }
        else
        {
            DclPrintf("  The specified alignment override (%lU) is greater than what the\n", ulAlignmentMin);
            DclPrintf("  Device requires.  Using the default value (%U) instead.\n", uMinAlignment);
        }
    }

    DclPrintf("  %U-bit through %U-bit aligned buffers will be tested.\n",
              uMinAlignment * 8, uMaxAlignment * 8);

    /*  Allocate buffers for the test.
        We give ourselves a little extra padding so we can verify the memory
        both before and after the pages we are operating on.
    */
    ulBufferSize = (FmlInfo.uPageSize * uMaxPages) + (uMaxAlignment * 2);
    pWriteBuffer = DclMemAlloc(ulBufferSize);
    pReadBuffer = DclMemAlloc(ulBufferSize);
    if(!pWriteBuffer || !pReadBuffer)
    {
        DclPrintf("    Failed to allocate test buffers\n");
        goto AlignmentTestCleanup;
    }

    /*  Determine the number of block needed for the test.  This is
        the sum of multiplying the number of alignment test cases by
        the number of pages in each operation size test case.
    */
    uAlignCases = 0;
    uTotalPages = 0;
    for(i = uMinAlignment; i <= uMaxAlignment; i *= 2)
        uAlignCases++;
    for(i = uMinPages; i <= uMaxPages; i++)
        uTotalPages += i * uAlignCases;
    ulBlockCount = uTotalPages / ulPagesPerBlock;
    if(uTotalPages % ulPagesPerBlock)
        ulBlockCount++;

    /*  Find a set of block(s) to test with and erase them.

        TBD: Need to make FindTestBlocks() from fmslnand.c a general purpose
        test utility.  That would allow for this test to choose a random
        set of blocks to test rather than always testing from block 0.
    */
    ulBlock = 0;
    if(!EraseAndVerify(hFML, ulBlock, ulBlockCount))
    {
        goto AlignmentTestCleanup;
    }

    /*  Perform each alignment test case.
    */
    ulPage = ulBlock * ulPagesPerBlock;
    for(i = uMinAlignment; i <= uMaxAlignment; i *= 2)
    {
        /*  Get a pointer at the current alignment.
        */
        pWrite = &pWriteBuffer[i];
        pRead = &pReadBuffer[i];

        DclPrintf("    Testing %2U-bit aligned buffers: W=%P, R=%P\n",
                  (i * 8), pWrite, pRead);

        /*  Perform write/read using the range of page counts.

            This will test a different alignment with operations involving
            different number of pages.
        */
        for(j = uMinPages; j <= uMaxPages; j++)
        {
            DclPrintf("      Testing %2U page operations\n", j);

            /*  Set the buffers to a known state.
            */
            DclMemSet(pWriteBuffer, 0xA5, ulBufferSize);
            DclMemSet(pReadBuffer, 0xA5, ulBufferSize);
            DclMemSet(pWrite, i, FmlInfo.uPageSize * j);
            DclMemSet(pRead, 0, FmlInfo.uPageSize * j);

            /*  Write pages.
            */
            FMLWRITE_PAGES(hFML, ulPage, j, pWrite, ioStat);
            if(!IOSUCCESS(ioStat, j))
            {
                DclPrintf("      Writing to page %lU failed!\n", ulPage);
                goto AlignmentTestCleanup;
            }

            /*  Read pages.
            */
            ioStat = FfxFmslTestReadPages(hFML, ulPage, j, pRead, READ_PAGES);
            if(!IOSUCCESS(ioStat, j))
            {
                DclPrintf("      Reading from page %lU failed!\n", ulPage);
                goto AlignmentTestCleanup;
            }

            /*  Verify the data.
            */
            for(k = 0; k < ulBufferSize; k++)
            {
                if(pWriteBuffer[k] != pReadBuffer[k])
                {
                    DclPrintf("      Data mismatch at offset %U\n", k);
                    DclHexDump("      Write Buffer:\n", HEXDUMP_UINT8, 16, ulBufferSize, pWriteBuffer);
                    DclHexDump("      Read Buffer:\n", HEXDUMP_UINT8, 16, ulBufferSize, pReadBuffer);
                    goto AlignmentTestCleanup;
                }
            }

            /*  This check is here because there was a bug where memory
                corruption could occur if the FORCEALIGNEDIO feature was
                utilized for a misaligned operation of 2 or more pages.
            */
            dclStatus = DclMemTrackPoolVerify(0, FALSE);
            if( dclStatus != DCLSTAT_SUCCESS &&
                dclStatus != DCLSTAT_FEATUREDISABLED &&
                dclStatus != DCLSTAT_SERVICE_NOTREGISTERED)
            {
                DclPrintf("      DclMemPoolVerify() failed: %lX\n", dclStatus);
                goto AlignmentTestCleanup;
            }

            ulPage += j;
        }
    }

    if(!EraseAndVerify(hFML, ulBlock, ulBlockCount))
    {
        goto AlignmentTestCleanup;
    }

    DclPrintf("  PASSED\n");
    fPass = TRUE;

  AlignmentTestCleanup:

    if(pReadBuffer)
        DclMemFree(pReadBuffer);
    if(pWriteBuffer)
        DclMemFree(pWriteBuffer);

    return fPass;
}


/*-------------------------------------------------------------------
    Local: VerifyBlockSize()

    Verifies that the eraseable zone size reported by the FML
    is correct.  This is done by writing across the boundary
    of an erase zone and verifing the data and erases of the
    two zones.

    Parameters:
       hFML - The handle denoting the FML to use.

    Return Value:
       TRUE  - If the test passed.
       FALSE - If a problem is found.  An error message is
               print displayed before return.
-------------------------------------------------------------------*/
static D_BOOL VerifyBlockSize(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
    FFXFMLDEVINFO   FmlDevInfo;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_UINT32        ulPagesPerBigTestBlock; /* number of pages in the big test buffer */
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;
    D_BOOL          fReturnValue;
    D_BUFFER       *pcBlockBuffer1;
    D_BUFFER       *pcBlockBuffer2;

    fReturnValue = FALSE;
    pcBlockBuffer1 = NULL;
    pcBlockBuffer2 = NULL;

    pcBlockBuffer1 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer1 == NULL)
    {
        DclPrintf("VerifyBlockSize Error: line %D in %s \n", __LINE__, __FILE__);
        goto VerifyBlockSizeCleanup;
    }

    pcBlockBuffer2 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer2 == NULL)
    {
        DclPrintf("VerifyBlockSize Error: line %D in %s \n", __LINE__, __FILE__);
        goto VerifyBlockSizeCleanup;
    }

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
		DclPrintf("VerifyBlockSize Error: line %D in %s \n", __LINE__, __FILE__);
        goto VerifyBlockSizeCleanup;
    }
    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
    {
		DclPrintf("VerifyBlockSize Error: line %D in %s \n", __LINE__, __FILE__);
        goto VerifyBlockSizeCleanup;
    }

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;
    ulPagesPerBigTestBlock = ulBigBlockSize / FmlInfo.uPageSize;

    DclPrintf("Testing erase block operations\n");

    /*  Bypass test if erase zone size too small.
    */
    if(FmlInfo.ulBlockSize < ulBigBlockSize)
    {
        DclPrintf("    Test skipped - erase zone size too small!\n");
        fReturnValue = TRUE;
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Erase and verify the first two erase blocks...\n");

    /*  Erase the first and second zone
    */
    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erasing first and second zones failed! [1]\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Write data across an erase block boundary...\n");

    /*  Set the values in the buffer to a known state.
    */
    DclMemSet(pcBigBlockBuffer, FILL_VALUE, ulBigBlockSize);

    /*  Write a data block that will write across an erase zone boundary
    */
    ulPage = ulPagesPerBlock - ulPagesPerTestBlock;
    FMLWRITE_PAGES(hFML, ulPage, ulPagesPerBigTestBlock, pcBigBlockBuffer, ioStat);
    if(!IOSUCCESS(ioStat, ulPagesPerBigTestBlock))
    {
        DclPrintf("    Write across zone boundaries failed!\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Erase and verify the first two erase blocks...\n");

    /*  Erase the first and second zone
    */
    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erasing first and second zones failed! [2]\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Write data across an erase block boundary...\n");

    /*  Write and Verify across the zone boundaries
    */
    FillBuffer(RANDOM, 0L, pcBlockBuffer1, TEST_BLOCK_SIZE);
    if(!WriteAndVerify(hFML, ulPagesPerBlock - ulPagesPerTestBlock, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("    Write at end of first zone failed!\n");
        goto VerifyBlockSizeCleanup;
    }

    FillBuffer(RANDOM, 0L, pcBlockBuffer1, TEST_BLOCK_SIZE);
    if(!WriteAndVerify(hFML, ulPagesPerBlock, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("    Write at start of second zone failed!\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Erase and verify the first erase block...\n");

    /*  Erase the first zone
    */
    if(!EraseAndVerify(hFML, 0, 1))
    {
        DclPrintf("    Erase zone size reported is too big.\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    Verify data in the second erase block...\n");

    /*  Verify data in second zone
    */
    ioStat = FfxFmslTestReadPages(hFML, ulPagesPerBlock, ulPagesPerTestBlock, pcBlockBuffer2, READ_PAGES);
    if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
    {
        DclPrintf("  VerifyBlockSize() FfxFmslTestReadPages() failed!\n");
        goto VerifyBlockSizeCleanup;
    }
    if(!Verify(pcBlockBuffer1, pcBlockBuffer2, TEST_BLOCK_SIZE))
    {
        DclPrintf("    Erase zone size reported is too small!\n");
        goto VerifyBlockSizeCleanup;
    }

    /*  Erase the second zone
    */
    if(!EraseAndVerify(hFML, 1, 1))
    {
        DclPrintf("    Erase of second zone failed!\n");
        goto VerifyBlockSizeCleanup;
    }

    DclPrintf("    PASSED\n");
    fReturnValue = TRUE;

VerifyBlockSizeCleanup:

    if (pcBlockBuffer1)
    {
        DclMemFree(pcBlockBuffer1);
    }

    if (pcBlockBuffer2)
    {
        DclMemFree(pcBlockBuffer2);
    }

    return(fReturnValue);

}


/*-------------------------------------------------------------------
    Local: BasicReadWrite()

    Verifies that writes to the media work by performing
    several small writes and read back verifies.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.  An error message is
                print displayed before return
-------------------------------------------------------------------*/
static D_BOOL BasicReadWrite(
    FFXFMLHANDLE    hFML)
{
    D_UINT32        ulIndex;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_UINT32        ulPagesPerBigTestBlock; /* number of pages in the big test buffer */
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;
    D_BOOL          fReturnValue;
    D_BUFFER       *pcBlockBuffer1;
    D_BUFFER       *pcBlockBuffer2;

    fReturnValue = FALSE;
    pcBlockBuffer1 = NULL;
    pcBlockBuffer2 = NULL;

    pcBlockBuffer1 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer1 == NULL)
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }

    pcBlockBuffer2 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer2 == NULL)
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }


    DclPrintf("Testing data reads and writes\n");

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;
    ulPagesPerBigTestBlock = ulBigBlockSize / FmlInfo.uPageSize;

    /*  Make sure the media is large enough
    */
    if(FmlInfo.ulTotalBlocks < 2)
    {
        DclPrintf("    Media too small!\n");
        fReturnValue =  TRUE;
        goto BasicReadWriteCleanup;
    }

    /*  Verify several small writes

        This test performs two small writes within the same erase zone.

        TODO: We should verify the entire set of data written AFTER
        completing the writes. What if the second write corrupted the
        data written in the first write?
    */
    DclPrintf("    Single page write, read, and verify...\n");

    if(!EraseAndVerify(hFML, 0, 1))
    {
        DclPrintf("    Erase failed\n");
        goto BasicReadWriteCleanup;
    }
    FillBuffer(RANDOM, 1L, pcBlockBuffer1, TEST_BLOCK_SIZE);
    ulPage = 0;
    if(!WriteAndVerify(hFML, ulPage, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }
    ulPage += ulPagesPerTestBlock;
    if(!WriteAndVerify(hFML, ulPage, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }

    /*  Cross an erase zone boundary

        This test performs two small writes, the first will write up to
        the end of an erase zone and the second will write at the beginning
        of the next erase zone.

        TODO: We should verify the entire set of data written AFTER
        completing the writes. What if the second write corrupted the
        data written in the first write?
    */
    DclPrintf("    Single page write, read, and verify, on both sides of a block boundary...\n");

    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erase failed!\n");
        goto BasicReadWriteCleanup;
    }
    FillBuffer(RANDOM, 1L, pcBlockBuffer1, TEST_BLOCK_SIZE);
    ulPage = ulPagesPerBlock - ulPagesPerTestBlock;
    if(!WriteAndVerify(hFML, ulPage, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }
    FillBuffer(RANDOM, 1L, pcBlockBuffer1, TEST_BLOCK_SIZE);
    ulPage += ulPagesPerTestBlock;
    if(!WriteAndVerify(hFML, ulPage, pcBlockBuffer1, TEST_BLOCK_SIZE))
    {
        DclPrintf("BasicReadWrite Error: line %D in %s \n", __LINE__, __FILE__);
        goto BasicReadWriteCleanup;
    }

    /*  Large write

        This test writes a single large block of data that will cross an
        erase zone boundary.
    */
    DclPrintf("    Multi-page write, read, and verify...\n");

    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erase failed!\n");
        goto BasicReadWriteCleanup;
    }

    /*  Fill the buffer with a modulo pattern
    */
    for(ulIndex = 0; ulIndex < ulBigBlockSize; ++ulIndex)
    {
        pcBigBlockBuffer[ulIndex] = (D_UCHAR) (ulIndex % 100);
    }

    /*  Calculate the starting page for this test.  It should be such that
        half of the data is written to the first erase zone and the other half
        is written to the second erase zone.
    */
    ulPage = ulPagesPerBlock - (ulPagesPerBigTestBlock / 2);

    /*  write buffer onto flash
    */
    FMLWRITE_PAGES(hFML, ulPage, ulPagesPerBigTestBlock, pcBigBlockBuffer, ioStat);
    if(!IOSUCCESS(ioStat, ulPagesPerBigTestBlock))
    {
        DclPrintf("    FMLWRITE_PAGES(), Large write failed, Page: %lU, Count: %lU\n",
                  ulPage, ulPagesPerBigTestBlock);
        goto BasicReadWriteCleanup;
    }

    /*  preset buffer to a known pattern
    */
    DclMemSet(pcBigBlockBuffer, 0x5A, ulBigBlockSize);

    /*  read flash into buffer
    */
    ioStat = FfxFmslTestReadPages(hFML, ulPage, ulPagesPerBigTestBlock, pcBigBlockBuffer, READ_PAGES);
    if(!IOSUCCESS(ioStat, ulPagesPerBigTestBlock))
    {
        DclPrintf("    FfxFmslTestReadPages(), Large read failed, Page: %lU, Count: %lU\n",
                  ulPage, ulPagesPerBigTestBlock);
        goto BasicReadWriteCleanup;
    }

    /*  Verify each byte matches the original pattern.
    */
    for(ulIndex = 0; ulIndex < ulBigBlockSize; ++ulIndex)
    {
        if(pcBigBlockBuffer[ulIndex] != (D_UCHAR) (ulIndex % 100))
        {
            DclPrintf("    Big write verify failed!\n");
            goto BasicReadWriteCleanup;
        }
    }

    DclPrintf("    PASSED\n");
    fReturnValue = TRUE;

BasicReadWriteCleanup:

    if (pcBlockBuffer1)
    {
        DclMemFree(pcBlockBuffer1);
    }

    if (pcBlockBuffer2)
    {
        DclMemFree(pcBlockBuffer2);
    }

    return(fReturnValue);

}


/*-------------------------------------------------------------------
    Local: ButterflyWrites()

    This test will ensure that the paging scheme, if any,
    works properly by writing to numerous addresses in
    the media. Alternate even and odd addresses from top
    to bottom and bottom to top are written and verified.

    Individual erase blocks are still written from low
    address to high address, since some flash devices
    require it.

    TODO: This test is only run on NOR flash. It should be
    moved to the NOR specific test module.

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.
-------------------------------------------------------------------*/
static D_BOOL ButterflyWrites(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPass;
    D_UINT32        ulNumPasses;
    D_UINT32        ulPage;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_BOOL          fReturnValue;
    D_BUFFER       *pcBlockBuffer1;
    D_BUFFER       *pcBlockBuffer2;

    DclPrintf("Butterfly write tests...\n");

    fReturnValue = FALSE;
    pcBlockBuffer1 = NULL;
    pcBlockBuffer2 = NULL;
    pcBlockBuffer1 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer1 == NULL)
    {
        DclPrintf("ButterflyWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ButterflyWritesCleanup;
    }

    pcBlockBuffer2 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer2 == NULL)
    {
        DclPrintf("ButterflyWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ButterflyWritesCleanup;
    }

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("ButterflyWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ButterflyWritesCleanup;
    }

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;

    /*  Limit the writes to the media size or 200 block writes
        This calculation is safe because ulNumPasses will be no
        larger than the total number of pages.
    */
    ulNumPasses = FmlInfo.ulTotalBlocks * (FmlInfo.ulBlockSize / TEST_BLOCK_SIZE);
    if(ulNumPasses > 200)
    {
        ulNumPasses = 200;
    }

    if(!EraseMedia(hFML, &FmlInfo, FALSE))
    {
        DclPrintf("ButterflyWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ButterflyWritesCleanup;
    }

    for(ulPass = 0; ulPass < ulNumPasses; ++ulPass)
    {
        if(ulPass & 1)
        {
            /*  Odd passes will write from the end of the media
            */
            ulPage = (FmlInfo.ulTotalBlocks * ulPagesPerBlock) - (((ulPass / 2) + 1) * ulPagesPerTestBlock);
            FillBuffer(CHECKER2, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
        }
        else
        {
            /*  Even passes will write from the start of the media
            */
            ulPage = (ulPass / 2) * ulPagesPerTestBlock;
            FillBuffer(CHECKER1, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
        }

        if(!WriteAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Butterfly write tests failed, Page: %lU, Pass: %lU\n", ulPage, ulPass);
            goto ButterflyWritesCleanup;
        }
    }

    DclPrintf("    PASSED\n");
    fReturnValue = TRUE;

    /*  Worked
    */

ButterflyWritesCleanup:

    if (pcBlockBuffer1)
    {
        DclMemFree(pcBlockBuffer1);
    }

    if (pcBlockBuffer2)
    {
        DclMemFree(pcBlockBuffer2);
    }

    return (fReturnValue);

}


/*-------------------------------------------------------------------
    Local: IntensiveWrites()

    Writes and reads bit patterns to and from the media.
    The media is then written with all zeros and verified.
    Two erase blocks are tested.

    TODO: This test is only run on NOR flash. It should be
    moved to the NOR specific test module.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        TRUE  - If the test passed.
        FALSE - If a problem is found.
-------------------------------------------------------------------*/
static D_BOOL IntensiveWrites(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_UINT32        ulPage;
    D_BOOL          fReturnValue;
    D_BUFFER      *pcBlockBuffer1;
    D_BUFFER      *pcBlockBuffer2;

    DclPrintf("Intensive write tests...\n");

    fReturnValue = FALSE;
    pcBlockBuffer1 = NULL;
    pcBlockBuffer2 = NULL;
    pcBlockBuffer1 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer1 == NULL)
    {
        DclPrintf("IntensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto IntensiveWritesCleanup;
    }

    pcBlockBuffer2 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer2 == NULL)
    {
        DclPrintf("IntensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto IntensiveWritesCleanup;
    }

    /*  Get the Media Info
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("IntensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto IntensiveWritesCleanup;
    }

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;

    /*  Test writing a different bit pattern to the first and second
        erase zones.
    */
    DclPrintf("    Write non-zero bit pattern...\n");
    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erase failed\n");
        goto IntensiveWritesCleanup;
    }
    ulPage = 0;
    FillBuffer(CHECKER1, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
    while(ulPage < ulPagesPerBlock)
    {
        if(!WriteAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Write failed, Page: %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }
    FillBuffer(CHECKER2, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
    while(ulPage < (ulPagesPerBlock * 2))
    {
        if(!WriteAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Write failed, Page %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }

    /*  Verify the non-zero bit pattern is still valid.
    */
    DclPrintf("    Verify non-zero bit pattern...\n");
    ulPage = 0;
    FillBuffer(CHECKER1, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
    while(ulPage < ulPagesPerBlock)
    {
        if(!ReadAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Verify failed, Page %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }
    FillBuffer(CHECKER2, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
    while(ulPage < (ulPagesPerBlock * 2))
    {
        if(!ReadAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Verify failed, Page %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }

    /*  Test writing a 0 pattern to the first two erase zones.
    */
    DclPrintf("    Write zero bit pattern...\n");
    if(!EraseAndVerify(hFML, 0, 2))
    {
        DclPrintf("    Erasing failed\n");
        goto IntensiveWritesCleanup;
    }
    FillBuffer(ZEROED, 0L,pcBlockBuffer1,TEST_BLOCK_SIZE);
    ulPage = 0;
    while(ulPage < (ulPagesPerBlock * 2))
    {
        if(!WriteAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Write failed, Page: %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }

    /*  Verify the zero bit pattern is still valid.
    */
    DclPrintf("    Verify zero bit pattern...\n");
    ulPage = 0;
    while(ulPage < (ulPagesPerBlock * 2))
    {
        if(!ReadAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    Verify failed, Page %lU\n", ulPage);
            goto IntensiveWritesCleanup;
        }
        ulPage += ulPagesPerTestBlock;
    }

    DclPrintf("    PASSED\n");
    fReturnValue = TRUE;

IntensiveWritesCleanup:

    if (pcBlockBuffer1)
    {
        DclMemFree(pcBlockBuffer1);
    }

    if (pcBlockBuffer2)
    {
        DclMemFree(pcBlockBuffer2);
    }


    return(fReturnValue);

}


/*-------------------------------------------------------------------
    Local: ExtensiveWrites()

    Erases, Writes, and Verifies every byte in the media.

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        TRUE  - If the test passed
        FALSE - If a problem is found.  An error message is
                displayed before return
-------------------------------------------------------------------*/
static D_BOOL ExtensiveWrites(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulTestMinutes)
{
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_UINT32        ulPage;
    D_UINT32        ulTestBlock;
    D_UINT32        ulTotalTestBlocks;
    FFXFMLINFO      FmlInfo;
    char            szFormatStr[] = "ExtensiveWrites page %lU sequence %lU"; /* 33 - 51 characters */
#define EXTENSIVEWRITE_STRLEN 52
    DCLTIMER        tRead;
    D_UINT32        ulTestMax, ulCounter, ulVerifyMax, ulBlockLoopCounter;
    D_BOOL          fReturnValue;
    D_BUFFER       *pcBlockBuffer1;

    DclPrintf("Extensive Write Tests...\n");

    fReturnValue = FALSE;
    pcBlockBuffer1 = NULL;
    pcBlockBuffer1 = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcBlockBuffer1 == NULL)
    {
        DclPrintf("ExtensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ExtensiveWritesCleanup;
    }

    /*  get media info
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("ExtensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ExtensiveWritesCleanup;
    }

    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;

    if(!EraseMedia(hFML, &FmlInfo, FALSE))
    {
        DclPrintf("ExtensiveWrites Error: line %D in %s \n", __LINE__, __FILE__);
        goto ExtensiveWritesCleanup;
    }

    /*  Calculate the number of test blocks that are needed to use all of
        the available space.
    */
    ulTotalTestBlocks = FmlInfo.ulTotalBlocks;

    /*  Write to the entire media.  The pattern we write contains the
        starting page being read and written. Once we have actually
        filled the entire array, we can make a second verification pass
        to make sure that all of the data is still intact.  This is useful
        when manually simulating read/write errors in MGM flash to make sure
        the FIM/BBM replaces failed blocks correctly.
        The pattern is:

        ExtensiveWrites page XXXX...X sequence YYYY...X
        where:
        XXXX...X   is the page number of the start of the block, in decimal
        YYYY...Y   is the sequence number of that data block, in decimal

        This pattern repeats to fill the buffer, with the last
        repetition truncated to fit.  There is no NUL character.  It
        is deliberately not a power of 2 length: the default software
        ECC (SSFDC algorithm) produces an ECC of all ones for a
        pattern that repeats every 32 bytes, others may have similar
        issues.
    */
    ulTestMax = ulTotalTestBlocks;
    if( ( ulTestMax % 2 ) == 1 )
    {
        /*  For an odd number of blocks, ulTestMax--; the
            last (even) block tested will be the exact midpoint
        */
        ulTestMax--;
    }

    if( ulTestMinutes )
    {
        /*  Set countdown timer in milliseconds
        */
        DclTimerSet(&tRead, ulTestMinutes*60*1000);
    }

    for(ulCounter = 0; ulCounter < ulTotalTestBlocks; ulCounter++)
    {
        int iLen;
        D_UCHAR *pcBuf;

        if( ulTestMinutes )
        {
            if( DclTimerExpired( &tRead ))
            {
                DclPrintf("    (Terminated early - timed out.\n");
                break;
            }
        }

        /*  Now determine exactly which block we are to test
        */
        if( ( ulCounter % 2 ) == 1 )
        {
            ulTestBlock = ulTestMax - ulCounter;
        }
        else
        {
            ulTestBlock = ulCounter;
        }


        /*  Calculate the page number for this test block.
        */
        ulPage = ulTestBlock * ulPagesPerBlock;

        /*  Progress message every 64KB
            TODO: This should be scaled with the size of the media, or
            changed to a time based reporting.
        */
        if((ulPage % ((64 * 1024) / FmlInfo.uPageSize)) == 0)
        {
            DclPrintf("    Writing to page %lU of %lU\n", ulPage,
                      ulPagesPerBlock * FmlInfo.ulTotalBlocks);
        }

        /*  fill pcBlockBuffer1 with the specified pattern
        */
        iLen = DclSNPrintf((char *) pcBlockBuffer1, EXTENSIVEWRITE_STRLEN,
                szFormatStr, ulPage, ulTestBlock);
        pcBuf = pcBlockBuffer1;

        while (iLen > 0)
        {
            pcBuf += iLen;
            if (pcBuf < pcBlockBuffer1 + TEST_BLOCK_SIZE)
            {
                iLen = DCLMIN(pcBuf - pcBlockBuffer1, pcBlockBuffer1 + TEST_BLOCK_SIZE - pcBuf);
                DclMemCpy(pcBuf, pcBlockBuffer1, iLen);
            }
            else
            {
                iLen = 0;
            }
        }

        ulBlockLoopCounter = ulPagesPerBlock / ulPagesPerTestBlock;

        while(ulBlockLoopCounter--)
        {
        /*  write and verify the block
        */
        if(!WriteAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    ERROR, Page: %lU\n", ulPage);
            goto ExtensiveWritesCleanup;
        }
            ulPage += ulPagesPerTestBlock;
        }
    }

    /*  Now make sure all the previously verified pages are still intact.
    */
    ulVerifyMax = ulCounter;
    for( ulCounter = 0; ulCounter < ulVerifyMax; ulCounter++ )
    {
        int iLen;
        D_UCHAR *pcBuf;

        if( ( ulCounter % 2 ) == 1 )
        {
            ulTestBlock = ulTestMax - ulCounter;
        }
        else
        {
            ulTestBlock = ulCounter;
        }

        /*  Calculate the page number for this test block.
        */
        ulPage = ulTestBlock * ulPagesPerBlock;

        /*  Progress message every 64KB
        */
        if((ulPage % ((64 * 1024) / FmlInfo.uPageSize)) == 0)
            DclPrintf("    Verifying page %lU\n", ulPage);

        /*  fill pcBlockBuffer1 with the specified pattern
        */
        iLen = DclSNPrintf((char *) pcBlockBuffer1, EXTENSIVEWRITE_STRLEN,
                szFormatStr, ulPage, ulTestBlock);
        pcBuf = pcBlockBuffer1;

        while (iLen > 0)
        {
            pcBuf += iLen;
            if (pcBuf < pcBlockBuffer1 + TEST_BLOCK_SIZE)
            {
                iLen = DCLMIN(pcBuf - pcBlockBuffer1, pcBlockBuffer1 + TEST_BLOCK_SIZE - pcBuf);
                DclMemCpy(pcBuf, pcBlockBuffer1, iLen);
            }
            else
            {
                iLen = 0;
            }
        }

        ulBlockLoopCounter = ulPagesPerBlock / ulPagesPerTestBlock;

        while(ulBlockLoopCounter--)
        {
        /*  read and verify the block
        */
        if(!ReadAndVerify(hFML, ulPage,pcBlockBuffer1,TEST_BLOCK_SIZE))
        {
            DclPrintf("    ERROR, Page: %lU\n", ulPage);
            goto ExtensiveWritesCleanup;
        }
            ulPage += ulPagesPerTestBlock;
        }
    }

    DclPrintf("    PASSED\n");
    fReturnValue = TRUE;

    /*  All worked
    */

ExtensiveWritesCleanup:

    if (pcBlockBuffer1)
    {
        DclMemFree(pcBlockBuffer1);
    }

    return(fReturnValue);

}


/*-------------------------------------------------------------------
    Local: Performance()

    This function determines the FlashfX FMSL performance level.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL Performance(
    FFXFMLHANDLE    hFML)
{
    FFXFMLINFO      FmlInfo;
  #if FFXCONF_LATENCYREDUCTIONENABLED
    D_UINT32        ulOldReadCount;
    D_UINT32        ulOldWriteCount;
    D_UINT32        ulOldEraseCount;
    D_UINT16        uOldErasePollInterval;
    unsigned        fOldCycleMutex;
  #endif

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("Performance Error line %D in %s \n", __LINE__, __FILE__);
        return FALSE;
    }

    DclPrintf("Testing FMSL performance\n");

    if((FmlInfo.ulTotalBlocks * FmlInfo.ulBlockSize) < 0x00200000L)
    {
        DclPrintf("    NOTE: Performance measurements on small media may not be accurate.\n");
    }

  #if FFXCONF_LATENCYREDUCTIONENABLED
    /*  Preserve original latency settings
    */
    FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXREADCOUNT,      &ulOldReadCount,    sizeof(ulOldReadCount));
    FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXWRITECOUNT,     &ulOldWriteCount,   sizeof(ulOldWriteCount));
    FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_MAXERASECOUNT,     &ulOldEraseCount,   sizeof(ulOldEraseCount));
    FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_ERASEPOLLINTERVAL, &uOldErasePollInterval, sizeof(uOldErasePollInterval));
    FfxFmlParameterGet(hFML, FFXPARAM_DEVMGR_CYCLEMUTEX,        &fOldCycleMutex,    sizeof(fOldCycleMutex));

    /*  Note that we are intentionally displaying the unsigned values
        below with signs, simply to make the output more concise since
        a D_UINT32_MAX value is legit and common.
    */
    DclPrintf("  Device Latency Settings\n");
    DclPrintf("    MaxPagesToReadPerFIMRequest   %5lD\n", ulOldReadCount);
    DclPrintf("    MaxPagesToWritePerFIMRequest  %5lD\n", ulOldWriteCount);
    DclPrintf("    MaxBlocksToErasePerFIMRequest %5lD\n", ulOldEraseCount);
    DclPrintf("    ErasePollInterval             %5U\n",  uOldErasePollInterval);
    DclPrintf("    CycleMutex                    %5s\n", fOldCycleMutex ? "Yes" : "No");
  #endif

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffmsl.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(fPerfLog, NULL, "FMSL", "PerfConfig", NULL, szPerfLogSuffix);
      #if FFXCONF_LATENCYREDUCTIONENABLED
        DCLPERFLOG_INT(hPerfLog, "MaxWriteCount",    ulOldWriteCount);
        DCLPERFLOG_INT(hPerfLog, "MaxReadCount",     ulOldReadCount);
        DCLPERFLOG_INT(hPerfLog, "MaxEraseCount",    ulOldEraseCount);
      #else
        DCLPERFLOG_INT(hPerfLog, "MaxWriteCount",    -1);
        DCLPERFLOG_INT(hPerfLog, "MaxReadCount",     -1);
        DCLPERFLOG_INT(hPerfLog, "MaxEraseCount",    -1);
      #endif
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    DclPrintf("  FMSL performance using default configuration settings\n");

    if(!RunPerfIterations(hFML, "PerfDefault", 1, FmlInfo.ulBlockSize / FmlInfo.uPageSize))
        return FALSE;

  #if FFXCONF_LATENCYREDUCTIONENABLED
    if(fPerfLatency)
    {
        D_UINT32    ulNewReadCount = D_UINT32_MAX;
        D_UINT32    ulNewWriteCount = D_UINT32_MAX;
        D_UINT32    ulNewEraseCount = D_UINT32_MAX;
        D_UINT16    uNewErasePollInterval = 0;
        unsigned    fNewCycleMutex = FALSE;

        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXREADCOUNT,      &ulNewReadCount,    sizeof(ulNewReadCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXWRITECOUNT,     &ulNewWriteCount,   sizeof(ulNewWriteCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXERASECOUNT,     &ulNewEraseCount,   sizeof(ulNewEraseCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_ERASEPOLLINTERVAL, &uNewErasePollInterval, sizeof(uNewErasePollInterval));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_CYCLEMUTEX,        &fNewCycleMutex,    sizeof(fNewCycleMutex));

        DclPrintf("  FMSL performance using \"max-performance\" (higher latency) settings\n");

        if(!RunPerfIterations(hFML, "MaxPerf", 1, FmlInfo.ulBlockSize / FmlInfo.uPageSize))
            return FALSE;

        ulNewReadCount = 1;
        ulNewWriteCount = 1;
        ulNewEraseCount = 1;
        uNewErasePollInterval = 1;
        fNewCycleMutex = TRUE;
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXREADCOUNT,      &ulNewReadCount,    sizeof(ulNewReadCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXWRITECOUNT,     &ulNewWriteCount,   sizeof(ulNewWriteCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXERASECOUNT,     &ulNewEraseCount,   sizeof(ulNewEraseCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_ERASEPOLLINTERVAL, &uNewErasePollInterval, sizeof(uNewErasePollInterval));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_CYCLEMUTEX,        &fNewCycleMutex,    sizeof(fNewCycleMutex));

        DclPrintf("  FMSL performance using \"minimum-latency\" settings\n");

        if(!RunPerfIterations(hFML, "MinLatency", 1, FmlInfo.ulBlockSize / FmlInfo.uPageSize))
            return FALSE;

        /*  Restore the original latency settings
        */
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXREADCOUNT,      &ulOldReadCount,    sizeof(ulOldReadCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXWRITECOUNT,     &ulOldWriteCount,   sizeof(ulOldWriteCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_MAXERASECOUNT,     &ulOldEraseCount,   sizeof(ulOldEraseCount));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_ERASEPOLLINTERVAL, &uOldErasePollInterval, sizeof(uOldErasePollInterval));
        FfxFmlParameterSet(hFML, FFXPARAM_DEVMGR_CYCLEMUTEX,        &fOldCycleMutex,    sizeof(fNewCycleMutex));
    }
  #endif

    return TRUE;
}


/*--------------------------------------------------------------------------
         HELPER FUNCTIONS
--------------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Local: EraseMedia()

    Erases all erase blocks.

    Parameters:
        hFML - The handle denoting the FML to use

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL  EraseMedia(
    FFXFMLHANDLE    hFML,
    FFXFMLINFO     *pMI,
    unsigned        fQuiet)
{
    D_UINT16        uBlockMod;
    D_UINT32        ulBlockNum;

    if(!fQuiet)
        DclPrintf("    Erasing the flash array...\n");

    /*  Set the progress interval.
            every     10 if      1 -   1,000 blocks
            every    100 if  1,001 -  10,000 blocks
            every  1,000 if 10,001 - 100,000 blocks
            every 10,000 if more than 100,000 blocks
    */
    uBlockMod = 10;
    if(pMI->ulTotalBlocks > 1000)
        uBlockMod = 100;
    if(pMI->ulTotalBlocks > 10000)
        uBlockMod = 1000;
    if(pMI->ulTotalBlocks > 100000)
        uBlockMod = 10000;

    /*  Erase all the blocks
    */
    for(ulBlockNum = 0;
        ulBlockNum < pMI->ulTotalBlocks;
        ulBlockNum++)
    {
        /*  Progress message
        */
        if(!fQuiet && ((ulBlockNum == 0) || !((ulBlockNum + 1) % uBlockMod)))
        {
            DclPrintf("      Erasing %lU of %lU\n", ulBlockNum + 1, pMI->ulTotalBlocks);
        }

        /*  Erase this block if it is not already erased.
        */
        if(!EraseIfNecessary(hFML, ulBlockNum, 1))
        {
            DclPrintf("      Erase verify failed, Block: %lU\n", ulBlockNum + 1);

            return FALSE;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: EraseIfNecessary()

    Erase the current zone (area) IF it is not already erased
    and verify it erases with block buffer 2. Preserves
    block buffer 1

    Parameters:
        hFML     - The handle denoting the FML to use
        ulBlock  - Starting block to erase and verify
        ulLength - Number of blocks

    Return Value:
        TRUE  - If the flash memory is erased.
        FALSE - If the area is not erased.
-------------------------------------------------------------------*/
static D_BOOL EraseIfNecessary(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulBlock,
    D_UINT32        ulCount)
{
    D_UINT16        u;
    D_BOOL          bErased;
    D_BOOL          bEraseFailure;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_UINT32        ulPage;
    D_UINT32        ulEndPage;
    FFXFMLINFO      FmlInfo;
    FFXIOSTATUS     ioStat;
    D_BOOL          fReturnValue;
    D_BUFFER       *pcDataReadBackBuffer;
    D_BUFFER       *pcSpare;

    fReturnValue = FALSE;
    pcDataReadBackBuffer = NULL;
    pcSpare = NULL;

    pcDataReadBackBuffer = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcDataReadBackBuffer == NULL)
    {
        DclPrintf("ReadEraseAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto ReadEraseAndVerifyCleanup;
    }

    pcSpare = DclMemAlloc(TEST_BLOCK_SIZE);
    if (pcSpare == NULL)
    {
        DclPrintf("ReadEraseAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto ReadEraseAndVerifyCleanup;
    }

    /*  Get some information about the media
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("ReadEraseAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto ReadEraseAndVerifyCleanup;
    }

    DclAssert(FmlInfo.ulBlockSize);
    DclAssert(FmlInfo.uPageSize);
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    ulPagesPerTestBlock = TEST_BLOCK_SIZE / FmlInfo.uPageSize;

    /*  set initial values
    */
    bEraseFailure = FALSE;
    ulPage = ulBlock * ulPagesPerBlock;
    ulEndPage = ulPage + (ulPagesPerBlock * ulCount);

    /*  initialize the spare buffer to an 'erased' condition
    */
    DclMemSet(pcSpare, 0xFF, ulPagesPerTestBlock * FmlInfo.uPageSize);


    /*  check the media
    */
    while(ulPage < ulEndPage)
    {
        /*  assume it dosnt need to be erased
        */
        bErased = FALSE;

        /*  check for an EraseZone boundary
        */
        if((ulPage % ulPagesPerBlock) == 0)
        {
            /*  reset for the new EraseZone
            */
            bEraseFailure = FALSE;
        }

        /*  do the read
        */
        FMLREAD_UNCORRECTEDPAGES(hFML, ulPage, ulPagesPerTestBlock, pcDataReadBackBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
        {
            DclPrintf("  EraseIfNecessary  FMLREAD_UNCORRECTEDPAGES() failed, Page: %lU\n", ulPage);
            goto ReadEraseAndVerifyCleanup;
         }

      #if FFXCONF_NANDSUPPORT
        /*  If media is NAND, read the spare area as well
        */
        if(FmlInfo.uDeviceType == DEVTYPE_NAND)
        {
            ioStat = FfxFmslTestReadPages(hFML, ulPage, ulPagesPerTestBlock, pcSpare, READ_SPARES);
            if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
            {
                DclPrintf("  EraseIfNecessary  FfxFmslTestReadPages() failed, Page: %lU\n", ulPage);
                goto ReadEraseAndVerifyCleanup;
            }
        }
      #endif

        /*  Determine if current EraseZone needs to be erased.
               */
            for(u = 0; u < (ulPagesPerTestBlock * FmlInfo.uPageSize); ++u)
            {
            if((pcDataReadBackBuffer[u] != 0xFF) || (pcSpare[u] != 0xFF))
            {
                /*  This EraseZone is dirty ...
                             */

                /*  Determine if this is second erase attempt - if so, FAIL
                            */
                if(bEraseFailure)
                {
                    DclPrintf("    Erase verify failed, Page: %lU\n", ulPage);
                    goto ReadEraseAndVerifyCleanup;
                }

                /*  set flag for next pass
                             */
                bEraseFailure = TRUE;

                /*  backup to start of current erase zone
                             */
                ulPage -= (ulPage % ulPagesPerBlock);

                /*  erase the current EraseZone
                             */
                ulBlock = ulPage / ulPagesPerBlock;
                FMLERASE_BLOCKS(hFML, ulBlock, 1, ioStat);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("    FMLERASE_BLOCKS() failed, Block: %lU\n", ulBlock);
                    goto ReadEraseAndVerifyCleanup;
                }

                /*  We just erased a zone
                             */
                bErased = TRUE;
                break;
            }
        }
        if(!bErased)
        {
            /*  continue to check media, a block at a time
                      */
            ulPage += ulPagesPerTestBlock;
        }
    }
    fReturnValue = TRUE;

ReadEraseAndVerifyCleanup:

    if (pcDataReadBackBuffer)
    {
        DclMemFree(pcDataReadBackBuffer);
    }


    if (pcSpare)
    {
        DclMemFree(pcSpare);
    }

    return fReturnValue;

}


/*-------------------------------------------------------------------
    Local: EraseAndVerify()

    Erase the given zones and verify it erases with
    block buffer 2. Preserves block buffer 1!

    Parameters:
        hFML    - The handle denoting the FML to use
        ulBlock - Starting block to erase
        ulCount - Number of blocks to erase

    Return Value:
        TRUE  - If the flash memory is erased
        FALSE - If the area is not erased
-------------------------------------------------------------------*/
static D_BOOL EraseAndVerify(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulBlock,
    D_UINT32        ulCount)
{
    D_UINT16        uOffset;
    D_UINT16        uEraseCount;
    D_UINT32        ulPage;
    D_UINT32        ulEndPage;
    D_UINT32        ulPagesPerBlock;        /* number of pages in an erase block */
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    FFXFMLINFO      FmlInfo;
    FFXIOSTATUS     ioStat;
    D_BOOL          fReturnValue;
    D_BUFFER       *pcDataReadBackBuffer;

    fReturnValue = FALSE;
    pcDataReadBackBuffer = NULL;

    /*  Get some information about the media
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("EraseAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto ExitEraseAndVerify;
    }

    DclAssert(FmlInfo.ulBlockSize);
    DclAssert(FmlInfo.uPageSize);
    ulPagesPerBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    DclAssert(ulPagesPerBlock);
    DclAssert(ulCount <= FmlInfo.ulTotalBlocks);

    ulPagesPerTestBlock = FmlInfo.ulBlockSize / FmlInfo.uPageSize;
    pcDataReadBackBuffer = DclMemAlloc(FmlInfo.ulBlockSize);

    if (pcDataReadBackBuffer == NULL)
    {
        DclPrintf("EraseAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto ExitEraseAndVerify;
    }

    uEraseCount = 0;
    ulPage = ulBlock * ulPagesPerBlock;
    ulEndPage   = ulPage + (ulPagesPerBlock * ulCount);
    while(ulPage < ulEndPage)
    {
        if((ulPage % ulPagesPerBlock) == 0)
        {
            ulBlock = ulPage / ulPagesPerBlock;
            ++uEraseCount;
            FMLERASE_BLOCKS(hFML, ulBlock, 1, ioStat);
            if(!IOSUCCESS(ioStat, 1))
            {
                DclPrintf("    FMLERASE_BLOCKS() failed, Block: %lU, EraseCount: %U\n",
                     ulBlock, uEraseCount);
                goto ExitEraseAndVerify;
            }
        }

        FMLREAD_UNCORRECTEDPAGES(hFML, ulPage, ulPagesPerTestBlock, pcDataReadBackBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
        {
            DclPrintf("    FMLREAD_UNCORRECTEDPAGES() failed, Page: %lU\n", ulPage);
            goto ExitEraseAndVerify;
        }

        for(uOffset = 0; uOffset < TEST_BLOCK_SIZE; ++uOffset)
        {
            if(pcDataReadBackBuffer[uOffset] != 0xFF)
            {
                DclPrintf("    Erase verify failed, Page: %lU Offset: %D\n", ulPage, uOffset);
                DclHexDump(NULL, HEXDUMP_UINT8, 32, TEST_BLOCK_SIZE, pcDataReadBackBuffer);
                goto ExitEraseAndVerify;
            }
        }

        ulPage += ulPagesPerTestBlock;
    }

    fReturnValue = TRUE;

ExitEraseAndVerify:

    if (pcDataReadBackBuffer)
    {
        DclMemFree(pcDataReadBackBuffer);
    }

    return (fReturnValue);

}


/*-------------------------------------------------------------------
    Local: GetReadPerform()

    This function determines the general FMSL read performance.

    See the notes in the declaration of pcBigBlockBuffer for more
    information about alignment issues.

    Parameters:
        hFML - The handle denoting the FML to use.
        nCount  - The number of pages to read in each operation.

    Return Value:
        Returns a KB/second read performance value.
-------------------------------------------------------------------*/
static D_UINT32 GetReadPerform(
    FFXFMLHANDLE    hFML,
    unsigned        nCount)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulPageNum = 0;
    D_UINT32        ulRemaining;
    D_UINT32        ulPagesRead = 0;
    FFXIOSTATUS     ioStat;
    DCLTIMER        tRead;
    FFXFMLINFO      FmlInfo;

    /*  Get some information about the media
    */
    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("GetReadPerform Error line %D in %s \n", __LINE__, __FILE__);
        return 0;
    }

    /*  Initialize the timer for a certain number of milliseconds.
    */
    DclTimerSet(&tRead, PERF_SECONDS*1000);

    ulRemaining = FmlInfo.ulTotalBlocks * (FmlInfo.ulBlockSize / FmlInfo.uPageSize);

    while(ulRemaining && !DclTimerExpired(&tRead))
    {
        D_UINT32 ulCount = DCLMIN(ulRemaining, nCount);

        FMLREAD_UNCORRECTEDPAGES(hFML, ulPageNum, ulCount, pcBigBlockBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("  GetReadPerform  FMLREAD_UNCORRECTEDPAGES() Failed!\n");
            return 0L;
        }

        ulPageNum   += ioStat.ulCount;
        ulRemaining -= ioStat.ulCount;
        ulPagesRead += ioStat.ulCount;
    }

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tRead);

    if(uVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Read %s in %lU ms\n", DclScaleBytes(ulPagesRead * FmlInfo.uPageSize, NULL, 0), ulDiffTime);

    /*  Check for possible zero denominator
    */
    if(ulDiffTime == 0)
    {
        /*  How could this ever happen? -- In an emulation environment using
            a RAM based FIM, without any emulating of timing, it can...
        */
        ulDiffTime = 1;
    }

    return (((ulPagesRead * FmlInfo.uPageSize) / 1024) * MSEC_PER_SEC) / ulDiffTime;
}


/*-------------------------------------------------------------------
    Local: GetWritePerform()

    This function determines the general FMSL write performance.

    Parameters:
        hFML - The handle denoting the FML to use.
        nCount  - The number of pages to read in each operation.

    Return Value:
        Returns a KB/second write performance value.
-------------------------------------------------------------------*/
static D_UINT32 GetWritePerform(
    FFXFMLHANDLE    hFML,
    unsigned        nCount)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulPageNum = 0;
    D_UINT32        ulRemaining;
    D_UINT32        ulPagesWritten = 0;
    FFXFMLINFO      FmlInfo;
    DCLTIMER        tWrite;
    FFXIOSTATUS     ioStat;

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("GetWritePerform Error line %D in %s \n", __LINE__, __FILE__);
        return 0;
    }

    /*  preset buffer to a known pattern
    */
    DclMemSet(pcBigBlockBuffer, 0x5A, ulBigBlockSize);

    /*  Initialize the timer for a certain number of milliseconds.
    */
    DclTimerSet(&tWrite, PERF_SECONDS*1000);

    ulRemaining = FmlInfo.ulTotalBlocks * (FmlInfo.ulBlockSize / FmlInfo.uPageSize);

    while(ulRemaining && !DclTimerExpired(&tWrite))
    {
        D_UINT32 ulCount = DCLMIN(ulRemaining, nCount);

        FMLWRITE_UNCORRECTEDPAGES(hFML, ulPageNum, ulCount, pcBigBlockBuffer, NULL, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("  GetWritePerform  FMLWRITE_UNCORRECTEDPAGES() Failed!\n");
            return 0L;
        }

        ulPageNum       += ioStat.ulCount;
        ulRemaining     -= ioStat.ulCount;
        ulPagesWritten  += ioStat.ulCount;
    }

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tWrite);

    if(uVerbosity > DCL_VERBOSE_NORMAL)
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

    return (((ulPagesWritten * FmlInfo.uPageSize) / 1024) * MSEC_PER_SEC) / ulDiffTime;
}


/*-------------------------------------------------------------------
    Local: GetErasePerform()

    This function determines the general FMSL erase performance.

    Parameters:
        hFML - The handle denoting the FML to use.

    Return Value:
        Returns a KB/second erase performance value.
-------------------------------------------------------------------*/
static D_UINT32 GetErasePerform(
    FFXFMLHANDLE    hFML)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulBlockNum = 0;
    D_UINT32        ulRemaining;
    D_UINT32        ulBlocksErased = 0;
    FFXFMLINFO      FmlInfo;
    DCLTIMER        tErase;
    FFXIOSTATUS     ioStat;

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return 0;

    /*  Initialize the timer for a certian number of milliseconds.
    */
    DclTimerSet(&tErase, PERF_SECONDS*1000);

    ulRemaining = FmlInfo.ulTotalBlocks;

    while(ulRemaining && !DclTimerExpired(&tErase))
    {
        D_UINT32    ulCount = DCLMIN(ulRemaining, 16);   /* 16 at a time if we can */

        /*  We try to erase more than one block at a time so that there
            is potentially some difference between the "max-perf" and
            "min-latency" test configurations.

            However, we don't try to erase <everything> with one call
            because that may dramatically exceed the amount of time we
            have allotted for this test.
        */

        /*  Erase some data at the beginning of the flash
        */
        FMLERASE_BLOCKS(hFML, ulBlockNum, ulCount, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("    FMLERASE_BLOCKS() failed!\n");
            return 0L;
        }

        ulBlockNum      += ioStat.ulCount;
        ulRemaining     -= ioStat.ulCount;
        ulBlocksErased  += ioStat.ulCount;
    }

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tErase);

    /*  check for possible zero denominator
    */
    if(ulDiffTime == 0)
    {
        /*  How could this ever happen? -- In an emulation environment using
            a RAM based FIM, without any emulating of timing, it can...
        */
        ulDiffTime = 1;
    }

    /*  Return the number of kilobytes we erased each second
    */
    DclAssert(FmlInfo.ulBlockSize % 1024 == 0);
    DclAssert(FmlInfo.ulBlockSize <= D_UINT32_MAX / MSEC_PER_SEC);

    if(uVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Erased %s in %lU ms\n", DclScaleBytes(ulBlocksErased * FmlInfo.ulBlockSize, NULL, 0), ulDiffTime);

    return (((ulBlocksErased * FmlInfo.ulBlockSize) / 1024) * MSEC_PER_SEC) / ulDiffTime;
}


/*-------------------------------------------------------------------
    Local: RunPerfIterations()

    Parameters:
        hFML    - The handle denoting the FML to use

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL RunPerfIterations(
    FFXFMLHANDLE    hFML,
    const char     *pszPerfLogName,
    unsigned        nMinPages,
    unsigned        nMaxPages)
{
    D_UINT32        ulEraseKBPerSecond = 0;
    D_UINT32        ulWriteKBPerSecond = 0;
    D_UINT32        ulReadKBPerSecond = 0;
    FFXFMLINFO      FmlInfo;
    unsigned        nCount;

    DclAssert(hFML);
    DclAssert(pszPerfLogName);
    DclAssert(nMaxPages > nMinPages);

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    DclAssert(nMaxPages <= FmlInfo.ulBlockSize / FmlInfo.uPageSize);
    DclAssert(nMaxPages <= ulBigBlockSize / FmlInfo.uPageSize);

    for(nCount = nMinPages; nCount <= nMaxPages; )
    {
        DclPrintf("  Processing %u pages at a time...\n", nCount);

        if(nCount == nMinPages)
        {
            ulEraseKBPerSecond = GetErasePerform(hFML);
            DclPrintf("      Erase: %10lU KB per second\n", ulEraseKBPerSecond);
        }

        if(FmlInfo.uDeviceType != DEVTYPE_NAND || fExtensiveTests)
        {
          #if FFXCONF_NANDSUPPORT
            if(FmlInfo.uDeviceType == DEVTYPE_NAND)
                DclPrintf("    Without ECC\n");
          #endif

            /*  Be sure the flash starts out clean
            */
            if(!EraseMedia(hFML, &FmlInfo, TRUE))
                return FALSE;

            ulWriteKBPerSecond = GetWritePerform(hFML, nCount);
            DclPrintf("      Write: %10lU KB per second\n", ulWriteKBPerSecond);

            ulReadKBPerSecond = GetReadPerform(hFML, nCount);
            DclPrintf("      Read:  %10lU KB per second\n", ulReadKBPerSecond);
        }

      #if FFXCONF_NANDSUPPORT
        if(FmlInfo.uDeviceType == DEVTYPE_NAND)
        {
            D_UINT32    ulPagesWritten;

            if(!EraseMedia(hFML, &FmlInfo, TRUE))
                return FALSE;

            if(fExtensiveTests)
                DclPrintf("    With ECC\n");

            ulWriteKBPerSecond = FfxFmslNANDTestWritePerform(hFML, pcBigBlockBuffer, ulBigBlockSize, &ulPagesWritten, PERF_SECONDS, nCount, uVerbosity);
            DclPrintf("      Write: %10lU KB per second\n", ulWriteKBPerSecond);

            ulReadKBPerSecond = FfxFmslNANDTestReadPerform(hFML, pcBigBlockBuffer, ulBigBlockSize, ulPagesWritten, PERF_SECONDS, nCount, uVerbosity);
            DclPrintf("      Read:  %10lU KB per second\n", ulReadKBPerSecond);
        }
      #endif

        /*---------------------------------------------------------
            Write data to the performance log, if enabled.  Note
            that any changes to the test name or category must be
            accompanied by changes to perffmsl.bat.  Any changes to
            the actual data fields recorded here requires changes
            to the various spreadsheets which track this data.
        ---------------------------------------------------------*/
        if(nCount == nMinPages || nCount == nMaxPages)
        {
            DCLPERFLOGHANDLE    hPerfLog;
            char                szNameBuff[32];

            DclSNPrintf(szNameBuff, sizeof(szNameBuff), "%s%s",
                pszPerfLogName, nCount == nMinPages ? "SmallIO" : "BigIO");

            hPerfLog = DCLPERFLOG_OPEN(fPerfLog, NULL, "FMSL", szNameBuff, NULL, szPerfLogSuffix);
            DCLPERFLOG_NUM(hPerfLog, "PagesPerOp",  nCount);
            DCLPERFLOG_NUM(hPerfLog, "Erase KB/s",  ulEraseKBPerSecond);
            DCLPERFLOG_NUM(hPerfLog, "Write KB/s",  ulWriteKBPerSecond);
            DCLPERFLOG_NUM(hPerfLog, "Read KB/s",   ulReadKBPerSecond);
            DCLPERFLOG_WRITE(hPerfLog);
            DCLPERFLOG_CLOSE(hPerfLog);
        }

    	/*  Check to see if any Timing functions returned with 0.
    	*/
    	if( (ulReadKBPerSecond  == 0)		||
    		(ulWriteKBPerSecond == 0)		||
    		(ulEraseKBPerSecond == 0) )
    	{
    		return FALSE;
    	}

        /*  If in "ExtensiveTests" mode, test every page count possbility.
            If not then just double the page count each time.
        */
        if(fExtensiveTests)
            nCount++;
        else
            nCount *= 2;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: WriteAndVerify()

    Writes the data in pcOriginalDataBuffer to the flash disk starting
    at ulPage. Calls ReadAndVerify check that the write occurred.

    Parameters:
        hFML                 - The handle denoting the FML to use
        ulPage               - Starting page to write and read back from read
                               the media
        pcOriginalDataBuffer - The data to be written
        ulBufferSize         - Size of the buffer in bytes, must be a multiple of FmlInfo.uPageSize

    Return Value:
       TRUE  - If the data read from the disk matches what was just written.
       FALSE - If the data does not match or the read/write call
               failed.
-------------------------------------------------------------------*/
static D_BOOL WriteAndVerify(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulPage,
    const D_BUFFER *pcOriginalDataBuffer,
    D_UINT32        ulBufferSize)
{
    FFXIOSTATUS     ioStat;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    FFXFMLDEVINFO   FmlDevInfo;

    DclAssert(pcOriginalDataBuffer);

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
        return FALSE;
    if(FfxFmlDeviceInfo(FmlInfo.nDeviceNum, &FmlDevInfo) != FFXSTAT_SUCCESS)
        return FALSE;

    ulPagesPerTestBlock = ulBufferSize / FmlInfo.uPageSize;

    /*  Write data block one
    */
    FMLWRITE_PAGES(hFML, ulPage, ulPagesPerTestBlock, pcOriginalDataBuffer, ioStat);
    if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
    {
        DclPrintf("    FMLWRITE_PAGES() failed!\n");
        return FALSE;
    }

    /*  Read back and verify the data that we just wrote
    */
    if(!ReadAndVerify(hFML, ulPage, pcOriginalDataBuffer, ulBufferSize))
    {
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ReadAndVerify()

    Reads data from the flash disk into local pcDataReadBackBuffer and
    verifies that that every byte is the same as in pcOriginalDataBuffer.

    Parameters:
        hFML                 - The handle denoting the FML to use.
        ulPage               - Starting page from which to read from read the media.
        pcOriginalDataBuffer - The data that was written
        ulBufferSize         - Size of the buffer in bytes, must be a multiple of FmlInfo.uPageSize

    Return Value:
       TRUE  - If the data read from the disk matches that in pcBlockBuffer1.
       FALSE - If the data does not match or the read call failed
-------------------------------------------------------------------*/
static D_BOOL ReadAndVerify(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulPage,
    const D_BUFFER  *pcOriginalDataBuffer,
    D_UINT32        ulBufferSize)
{
    FFXIOSTATUS     ioStat;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulPagesPerTestBlock;    /* number of pages in the test buffer */
    D_BUFFER       *pcDataReadBackBuffer;
    D_BOOL          fReturnValue;


    fReturnValue = FALSE;
    pcDataReadBackBuffer = NULL;
    pcDataReadBackBuffer = DclMemAlloc (ulBufferSize);
    if (pcDataReadBackBuffer==NULL)
    {
        DclPrintf("ReadAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto Cleanup;
    }

    if (pcOriginalDataBuffer==NULL)
    {
        DclPrintf("ReadAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto Cleanup;
    }

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("ReadAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto Cleanup;
    }

    if(FmlInfo.uPageSize > ulBufferSize)
    {
        DclPrintf("ReadAndVerify Error line %D in %s \n", __LINE__, __FILE__);
        goto Cleanup;
    }

    ulPagesPerTestBlock = ulBufferSize / FmlInfo.uPageSize;

    /*  Read the flash disk data into buffer #2
    */
    FMLREAD_PAGES(hFML, ulPage, ulPagesPerTestBlock, pcDataReadBackBuffer, ioStat);
    if(!IOSUCCESS(ioStat, ulPagesPerTestBlock))
    {
        if (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA)
        {
            DclPrintf(" ReadAndVerify   FMLREAD_PAGES() failed!\n");
            return FALSE;
        }
    }

    /*  Verify the data we just read matches whatever is in buffer #1
    */
    if(!Verify(pcOriginalDataBuffer,pcDataReadBackBuffer,ulBufferSize))
    {
        goto Cleanup;
    }

     fReturnValue = TRUE;

Cleanup:
    if (pcDataReadBackBuffer)
    {
        DclMemFree(pcDataReadBackBuffer);
    }

    return (fReturnValue);
}


/*-------------------------------------------------------------------
    Local: Verify()

    Verifies that two buffers, pcDataBuffer1 and
    pcDataBuffer2 are identical.

    Parameters:
       fAllowSingleBitErrors - indicates if single bit errors are allowed
       *pcDataBuffer1        - First buffer
       *pcDataBuffer2        - Second buffer
       ulBufferSize          - size of the buffers that are compared.

    Return Value:
       TRUE  - If the buffers are identical.
       FALSE - If the buffers differ.
-------------------------------------------------------------------*/
static D_BOOL Verify(
    const D_BUFFER  *pcDataBuffer1,
    const D_BUFFER  *pcDataBuffer2,
    D_UINT32        ulBufferSize)
{
    D_UINT16        uIndex;

    DclAssert(pcDataBuffer1);
    DclAssert(pcDataBuffer2);

    for(uIndex = 0; uIndex < ulBufferSize; ++uIndex)
    {

        if(pcDataBuffer1[uIndex] != pcDataBuffer2[uIndex])
        {

                /*  Failed, return failure
                 */
                DclPrintf("    Data verify failed!\n");
                DclPrintf("    Offset in %u Byte block = %X\n", ulBufferSize, uIndex);
                DclPrintf("    Byte expected from read = 0x%02X\n", pcDataBuffer1[uIndex]);
                DclPrintf("    Byte read from flash    = 0x%02X\n", pcDataBuffer2[uIndex]);

                return FALSE;
        }
    }
    return TRUE;
}


/*-------------------------------------------------------------------
    Local: FillBuffer()

    Fills the pcDataBuffer with data given a type and some
    sample data. Valid types are ZERORED, RANDOM, CHECKER1,
    CHECKER2, PATTERN, PAGE_PATTERN and CONSTANT.

    Parameters:
        uType          - The method used to fill the buffer.
        ulData         - Input data, used for some fill methods.
        * pcDataBuffer - pointer to the buffer to be filled.
        ulBufferSize   - Size of the buffer to be filled.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FillBuffer(
    D_UINT16        uType,
    D_UINT32        ulData,
    D_BUFFER       *pcDataBuffer,
    D_UINT32        ulBufferSize)
{
    D_UINT16        u;
    D_UINT32       *pulBlockBuffer;
    static D_UINT32 ulSeed = 1;

    DclAssert(pcDataBuffer);

    pulBlockBuffer = (D_UINT32 *) pcDataBuffer;

    switch (uType)
    {
        case ZEROED:
            for(u = 0; u < ulBufferSize; ++u)
            {
                pcDataBuffer[u] = 0;
            }
            break;

        case RANDOM:
            for(u = 0; u < ulBufferSize; ++u)
            {
                pcDataBuffer[u] = (D_UCHAR) (DclRand(&ulSeed) + ulData);
            }
            break;

        case CHECKER1:
            for(u = 0; u < ulBufferSize; ++u)
            {
                pcDataBuffer[u] = (D_UCHAR) CHECKER1_VAL;
            }
            break;

        case CHECKER2:
            for(u = 0; u < ulBufferSize; ++u)
            {
                pcDataBuffer[u] = (D_UCHAR) CHECKER2_VAL;
            }
            break;

        case PAGE_PATTERN:
            for(u = 0; u < ulBufferSize; ++u)
            {
                pcDataBuffer[u] = (D_UCHAR) (DclRand(&ulData));
            }
            break;

        case PATTERN:
            for(u = 0; u < ulBufferSize / sizeof(D_UINT32); ++u)
            {
                pulBlockBuffer[u] = (ulData + u);
            }
            break;

        case CONSTANT:
            for(u = 0; u < ulBufferSize / sizeof(D_UINT32); ++u)
            {
                pulBlockBuffer[u] = ulData;
            }
            break;

        default:
            DclProductionError();
            break;
    }
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.  Also shows help via ShowUsage() if they need
    it.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure.

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 SetSwitches(
    FFXTOOLPARAMS  *pTP,
    FFXFMLHANDLE    hFML)
{
    D_INT16         i, j;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        argc;

    argc = DclArgCount(pTP->dtp.pszCmdLine);

    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, i, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("Bad argument!\n");
            return ERROR_BAD_OPTION;
        }

        if(DclStrNICmp(achArgBuff, "/STRESS", 7) == 0)
        {
            fStress = TRUE;

            if(achArgBuff[7] == ':')
            {
                ulStressMinutes = DclAtoL(&achArgBuff[8]);
            }

            continue;
        }

      #if FFXCONF_NANDSUPPORT
        if(DclStrNICmp(achArgBuff, "/BBM", 4) == 0)
        {
            FFXSTATUS   ffxStat;

            /*  This logic serves two purposes!  First it confirms that
                error injection features are enabled, otherwise the BBM
                test cannot run.  Secondly it disables any random injection
                of errors which might enabled in ffxconf.h.  They are <NOT>
                re-enabled after the test is completed.
            */                
            ffxStat = FfxFmlParameterSet(hFML, FFXPARAM_FIM_ERRINJECT_DISABLE, NULL, 0);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("The error injection functionality is not enabled -- ignoring /BBM (Status=%lX)\n", ffxStat);
            }        
            else
            {
                fBBMTests = TRUE;
            }
 
            continue;
        }

        if(DclStrNICmp(achArgBuff, "/LOCK", 5) == 0)
        {
            fLockTests = TRUE;
 
            continue;
        }
      #endif

        if(DclStrNICmp(achArgBuff, "/PERFLOG", 8) == 0)
        {
            fPerformanceTests = TRUE;
            fPerfLog = TRUE;

          #if FFXCONF_LATENCYREDUCTIONENABLED
            fPerfLatency = TRUE;
          #else
            DclPrintf("WARNING: The latency reduction features are disabled.\n");
          #endif

            if(achArgBuff[8] == ':')
            {
                DclStrNCpy(szPerfLogSuffix, &achArgBuff[9], sizeof(szPerfLogSuffix));
                szPerfLogSuffix[sizeof(szPerfLogSuffix)-1] = 0;
            }

            continue;
        }

        if(DclStrICmp(achArgBuff, "/LATENCY") == 0)
        {
          #if FFXCONF_LATENCYREDUCTIONENABLED
            fPerfLatency = TRUE;
            continue;
            
          #else
          
            DclPrintf("The /Latency option is only valid when used when latency reduction\n");
            DclPrintf("features are enabled with FFXCONF_LATENCYREDUCTIONENABLED.\n");
            return ERROR_BAD_OPTION;
          #endif
        }

        j = 0;

        /*  Allow for multiple switches in a string
        */
        while(*(achArgBuff + j))
        {
            if(*(achArgBuff + j) == '/')
            {
                /*  Go to the next character to test
                */
                ++j;

                /*  Information specfic for each switch
                */
                switch (*(achArgBuff + j))
                {
                    /*  Help
                    */
                    case '?':
                        ShowUsage(pTP);
                        return ERROR_USAGE;

                    /*  Performance Tests
                    */
                    case 'p':
                    case 'P':
                        ++j;
                        fPerformanceTests = TRUE;
                        break;

                  #if FFXCONF_NANDSUPPORT
                    /*  Run RetireBlockTest.  This tests can damage real flash
                        hardware and should only be run under simulation.
                    */
                    case 'r':
                    case 'R':
                        ++j;
                        fRetireBlockTest = TRUE;
                        break;
                  #endif

                    /*  Extensive Tests
                    */
                    case 'e':
                    case 'E':
                        ++j;
                        fExtensiveTests = TRUE;
                        if(*(achArgBuff + j) == ':')
                        {
                            ++j;
                            ulExtensiveMinutes = DclAtoL((achArgBuff + j));

                            /*  Cast quiets a compiler warning on broken
                            implementations of isdigit() macro.
                            */
                            while(DclIsDigit((unsigned char)achArgBuff[j]))
                            {
                                ++j;
                            }
                        }
                        break;

                    /*  Alignment Tests
                    */
                    case 'a':
                    case 'A':
                        ++j;
                        fAlignmentTests = TRUE;

                        if(*(achArgBuff + j) == ':')
                        {
                            ++j;

                            ulAlignmentMin = DclAtoL(achArgBuff + j);

                            /*  Cast quiets a compiler warning on broken
                                implementations of isdigit() macro.
                            */
                            while(DclIsDigit((unsigned char)achArgBuff[j]))
                                ++j;
                        }

                        break;

                    /*  Huge Tests
                    */
                    case 'h':
                    case 'H':
                        ++j;
                        fHugeTests = TRUE;
                        break;

                    /*  Random seed
                    */
                    case 's':
                    case 'S':
                        ++j;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        ++j;

                        ulRandomSeed = DclAtoL(achArgBuff + j);

                        /*  Cast quiets a compiler warning on broken
                            implementations of isdigit() macro.
                        */
                        while(DclIsDigit((unsigned char)achArgBuff[j]))
                        {
                            ++j;
                        }
                        break;

                    /*  Verbosity
                    */
                    case 'v':
                    case 'V':
                        ++j;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        ++j;

                        uVerbosity = (unsigned)DclAtoI(achArgBuff + j);
                        if(uVerbosity > DCL_VERBOSE_OBNOXIOUS)
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }

                        /*  Cast quiets a compiler warning on broken
                            implementations of isdigit() macro.
                        */
                        while(DclIsDigit((unsigned char)achArgBuff[j]))
                        {
                            ++j;
                        }
                        break;

                    /*  Un-Recognized switch
                    */
                    default:
                        DclPrintf("Bad option: %s\n", (achArgBuff + j));
                        return ERROR_BAD_OPTION;
                }
            }
            else
            {
                DclPrintf("Bad option: %s\n", (achArgBuff + j));
                return ERROR_BAD_OPTION;
            }
        }
    }

    if(fPerfLatency && !fPerformanceTests)
    {
        DclPrintf("The /Latency switch is only valid when used with the /P switch\n");
        return ERROR_BAD_OPTION;
    }

    /*  All worked fine!
    */
    return ERROR_PASSED;
}


/*---------------------------------------------------------------------
    Local: FfxFmslTestReadPages()

    This function is essentially a wrapper for FMLREAD_PAGES(),
    FMLREAD_CORRECTEDPAGES(), and FMLREAD_SPARES().  These functions now
    may return with a status other than FFXSTAT_SUCCCESS if a correctable error
    was found.  The read will only have succeeded up to the error, and so the remainder
    will need to be read with a subsequent request.  This function allows for assumptions
    based on the old functionality, where all the data would be read into the buffer and
    the request would come back invariably FFXSTAT_SUCCESS if all of the data is
    available.

    Parameters:
        hFML                  - FML handle
        ulStartPage           - Page to start reading from
        ulNumberOfPagesToRead - Number of pages to read
        pcDataReadBackBuffer  - Buffer to read into
        ulOperation           - Operation to perform

    Return Value:
        FFXIOSTATUS - Indicates of the reads were successful.
----------------------------------------------------------------------*/
FFXIOSTATUS FfxFmslTestReadPages(
    FFXFMLHANDLE hFML,
    D_UINT32     ulStartPage,
    D_UINT32     ulNumberOfPagesToRead,
    D_BUFFER   * pcDataReadBackBuffer,
    D_UINT32     ulOperation)
{
    FFXIOSTATUS  ioStat;
    FFXFMLINFO   FmlInfo;
    D_UINT32     ulBufferIndex;
    FFXIOSTATUS  ioReturnIoStat;
    D_UINT32     ulCurrentPage;
    D_UINT32     ulCount;
    D_UINT32     ulElementSize;

    ioReturnIoStat.ffxStat = FFXSTAT_SUCCESS;
    ulCurrentPage = ulStartPage;
    ulCount = ulNumberOfPagesToRead;
    ulBufferIndex =0;
    ioReturnIoStat.ulCount = 0;
    ioReturnIoStat.ulFlags = 0;
    ioReturnIoStat.op.ulDeviceStatus = 0;
    ioReturnIoStat.op.ulBlockStatus = 0;
    ioReturnIoStat.op.ulPageStatus = 0;
    ioReturnIoStat.op.ulByteStatus = 0;

    if(FfxFmlDiskInfo(hFML, &FmlInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("FfxFmslTestReadPages Error line %D in %s \n", __LINE__, __FILE__);
        ioReturnIoStat.ffxStat = FFXSTAT_FAILURE;
    }
    else
    {
        switch(ulOperation)
        {
            case READ_PAGES:
                ulElementSize = FmlInfo.uPageSize;
                break;
#if FFXCONF_NANDSUPPORT
            case READ_CORRECTEDPAGES:
                ulElementSize = FmlInfo.uPageSize;
                break;
            case READ_SPARES:
                ulElementSize = FmlInfo.uSpareSize;
                break;
#endif
            default:
                DclError();
                ioReturnIoStat.ffxStat = FFXSTAT_FAILURE;
                return ioReturnIoStat;
        }

        while( ulCount > 0 )
        {
            switch(ulOperation)
            {
                case READ_PAGES:
                    FMLREAD_PAGES(hFML, ulCurrentPage, ulCount, &pcDataReadBackBuffer[ulBufferIndex], ioStat);
                    break;
#if FFXCONF_NANDSUPPORT
                case READ_CORRECTEDPAGES:
                    FMLREAD_CORRECTEDPAGES(hFML, ulCurrentPage, ulCount, &pcDataReadBackBuffer[ulBufferIndex], ioStat);
                    break;
                case READ_SPARES:
                    FMLREAD_SPARES(hFML, ulCurrentPage, ulCount, &pcDataReadBackBuffer[ulBufferIndex], ioStat);
                    break;
#endif
                default:
                    DclError();
                    ioReturnIoStat.ffxStat = FFXSTAT_FAILURE;
                    return ioReturnIoStat;
            }
            ioReturnIoStat.ulFlags = ioStat.ulFlags;
            ulCurrentPage += ioStat.ulCount;
            ulCount -= ioStat.ulCount;
            ioReturnIoStat.ulCount += ioStat.ulCount;
            ulBufferIndex = (ulCurrentPage - ulStartPage) * ulElementSize;
            ioReturnIoStat.op = ioStat.op;

            if (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA && ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
               ioReturnIoStat.ffxStat = FFXSTAT_FAILURE;
               break;
            }

        }

    }

    return (ioReturnIoStat);

}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    Shows help to the user and returns.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nSyntax:  %s disk [options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where:\n");
    DclPrintf("  disk           The flash Disk designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  /?             This help screen\n");
    DclPrintf("  /A[:n]         Run the buffer Alignment tests.  The optional value overrides the\n");
    DclPrintf("                 default the minimum alignment to use.  Use the override feature\n");
    DclPrintf("                 with caution, as it could cause a fault.\n");
    DclPrintf("  /E[:n]         Extensive write/erase tests to the entire media, where 'n' is the\n");
    DclPrintf("                 number of minutes to run the test. If not specified, this test\n");
    DclPrintf("                 writes and erases every block.\n");
    DclPrintf("  /H             Huge operation tests\n");
    DclPrintf("  /P             Run the performance benchmarks\n");
    DclPrintf("  /S:n           Set test random seed\n");
    DclPrintf("  /V:n           The Verbosity level 0-3 (Default=1)\n");
  #if FFXCONF_NANDSUPPORT
    DclPrintf("  /BBM           Run the BBM tests - Do this only under simulation!\n");
    DclPrintf("  /R             Run the RetireBlockTest - Do this only under simulation!\n");
    DclPrintf("  /Lock          Run the block locking tests (NAND only)\n");
  #endif
    DclPrintf("  /Stress[:n]    Run the flash stress tests, where 'n' is the optional number of\n");
    DclPrintf("                 minutes to run the test.  The default is to run forever, or until\n");
    DclPrintf("                 the test failure thresholds are reached.\n");
    DclPrintf("  /Latency       Run the performance benchmarks with multiple latency settings\n");
    DclPrintf("  /PerfLog[:sfx] Output performance test results in CSV form, in addition\n");
    DclPrintf("                 to the standard output.  Implies the /P and /Latency switches.\n");
    DclPrintf("                 The optional 'sfx' value is a text suffix which will be\n");
    DclPrintf("                 appended to the build number in the PerfLog CSV output.\n\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


