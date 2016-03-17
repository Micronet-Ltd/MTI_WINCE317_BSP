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

    This module tests all aspects of the VBF API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbftst.c $
    Revision 1.70  2010/11/20 00:11:48Z  glenns
    Modify RemountVBF procedure to disable saving the state info
    if the QuickMount feature is turned on. Some VBF tests depend
    on a rescan of the media to perform their tests, and this
    feature can stop that from happening.
    Revision 1.69  2010/07/28 04:37:05Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.68  2010/07/05 22:06:43Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.67  2010/05/28 21:02:42Z  johnbr
    Removing an invalid test which was crashing preflight.
    Revision 1.66  2010/05/27 21:47:19Z  johnbr
    One, final, repair to FfxVbfCompact().
    Revision 1.65  2010/05/26 23:14:20Z  johnbr
    Repairing the damage from the previous checkin, output format
    changes, data type changes and revisions to bring the source 
    back in line with the coding standard.
    Revision 1.64  2010/05/25 18:04:04Z  johnbr
    Added functional and error test cases for the Discard logic
    Revision 1.63  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.62  2009/12/18 03:43:56Z  garyp
    Resolved Bug 2955: Write interruption test failure.
    Revision 1.61  2009/12/03 22:00:26Z  garyp
    Updated to use FfxDisplayVbfDiskInfo().
    Revision 1.60  2009/11/18 02:00:24Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.59  2009/08/02 19:35:13Z  garyp
    Merged from the v4.0 branch.  Updated so that VBF instances are created
    in compaction suspend mode and have to be explicitly resumed.  Updated
    to suspend idle-time compaction at test startup, and restore the original
    suspend level when the test is completed.  Modified the shutdown processes
    to take a mode parameter.  Updated the compaction functions to take a
    compaction level.  Updated to use the revised FfxSignOn() function, which
    now takes an fQuiet parameter.  Updated for new compaction functions 
    which now return an FFXIOSTATUS value rather than a D_BOOL.
    Revision 1.58  2009/04/01 14:37:12Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.57  2009/03/18 19:16:49Z  keithg
    Fixed bug 2508, inconsistent storage class for PreMountRegions()
    Revision 1.56  2009/03/09 19:35:11Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.55  2009/02/09 01:35:40Z  garyp
    Merged from the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.54  2008/12/16 17:08:05Z  thomd
    Document Extensive optional minutes parameter
    Revision 1.53  2008/12/09 20:51:31Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.52  2008/10/24 06:08:48Z  keithg
    Reduced error condition tests for a more reasonable completion time.
    Revision 1.51  2008/05/23 13:32:02Z  thomd
    Added paramter to format - fFormatNoErase = FALSE
    Revision 1.50  2008/05/13 16:02:29Z  thomd
    Added timing parameter to extensive writes test;
    Varied extensive write locations mathematically
    Revision 1.49  2008/02/06 20:21:19Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.48  2008/01/17 04:13:20Z  Garyp
    Increased the "huge" buffer size to better test large I/O operations.  Fixed
    the region size display to display the client size.  Fixed some PerfLog
    stats pertaining to region and disk size.
    Revision 1.47  2008/01/13 07:26:56Z  keithg
    Function header updates to support autodoc.
    Revision 1.46  2007/11/09 14:52:12Z  thomd
    Correct gnu compile problem
    Revision 1.45  2007/11/08 19:11:50Z  pauli
    Added huge operations tests.
    Revision 1.44  2007/11/06 18:15:25Z  Garyp
    Corrected the previous fix to not use an uninitialized variable.
    Revision 1.43  2007/11/06 00:51:52Z  Garyp
    Changed TestDiscardSet() to discard the entire region at once, rather than
    one page at a time, providing a little better test coverage.  Updated all
    asserts to be production level asserts.
    Revision 1.42  2007/11/03 02:59:11Z  Garyp
    Updated the region boundary tests to include testing discards which cross
    multiple region boundaries.
    Revision 1.41  2007/11/02 02:52:31Z  Garyp
    Eliminated some unnecessary includes.
    Revision 1.40  2007/10/29 15:49:43Z  Garyp
    Added wear-leveling tests.
    Revision 1.39  2007/10/24 18:47:14Z  Garyp
    Revision 1.38  2007/10/22 20:11:02Z  pauli
    Added verbose level as a test parameter.
    Revision 1.37  2007/10/12 23:04:26Z  billr
    Add new capability to run special VBF QA tests.
    Revision 1.36  2007/10/10 00:55:36Z  Garyp
    Updated the performance tests to ensure that we don't try to read or discard
    areas which were never written.
    Revision 1.35  2007/10/05 02:43:06Z  Garyp
    Updated the read/write/discard performance tests to all operate in the
    same fashion, where they are operating on areas of the flash where the
    regions are premounted.  Region mount performance is tested
    separately.
    Revision 1.34  2007/09/13 20:41:49Z  pauli
    Changed the random seed syntax to be consistent with other tests and tools.
    Expanded the seed from an int to a long.
    Revision 1.33  2007/09/11 20:25:40Z  pauli
    Renamed the TESTINFO structure to VBFTESTINFO.  Updated the test to use the
    new fields in the VBFDISKMETRICS structure.
    Revision 1.32  2007/08/31 21:22:27Z  pauli
    Resolved Bug 1402: Changed byte counts in VBFUNITMETRICS to page counts.
    Revision 1.31  2007/08/29 22:26:42Z  pauli
    Renamed USE_DEFAULT to FFX_USE_DEFAULT.
    Updated an printf erroneously displaying pages as bytes in hex.
    Revision 1.30  2007/06/27 23:03:39Z  Garyp
    Updated to use the new PerfLog "write" macro, as well as the new "Category
    Suffix" field. Minor tweaks to the performance tests to get a little better
    accuracy.
    Revision 1.29  2007/06/17 00:11:43Z  Garyp
    General cleanup of the TestFillDisk() function.  Eliminated the use of
    redundant variables and updated to display pages which have corrected
    data errors (viable when Bug 1218 is fixed).
    Revision 1.28  2007/04/27 19:14:20Z  rickc
    Moved FfxVbfDiskInfo() call after VBF has been formatted.
    Revision 1.27  2007/04/25 23:54:54Z  rickc
    Removed ulClientSize in VBFTESTINFO for LFA.  Added buffer block counts to
    simplify code.
    Revision 1.26  2007/04/24 21:35:28Z  rickc
    Changes offsets from byte address to page address.  Changed use of
    deprecated vbfdiscard(), vbfread(), and vbfwrite() to FfxVbfDiscardPages(),
    FfxVbfReadPages(), and FfxVbfWritePages().  Removed calls to deprecated
    vbfclientsize().
    Revision 1.25  2007/04/19 21:38:23Z  timothyj
    Changed use of deprecated vbfdiscard() to FfxVbfDiscardPages() in
    TestErrors(), as the deprecated vbfdiscard() returns void (and asserts,
    appropriately) when the discard is out of bounds, now that the underlying
    code has been updated to support the non-deprecated FfxVbfXxxxPages()
    functions.
    Revision 1.24  2007/04/02 18:55:15Z  Garyp
    Updated to support PerfLog build number suffixes.
    Revision 1.23  2007/03/30 18:46:25Z  Garyp
    Updated to use a renamed structure.
    Revision 1.22  2007/02/15 20:53:56Z  Garyp
    Increased the performance test period for compactions so we are more
    accurate on slow NOR flash.
    Revision 1.21  2007/01/18 22:19:33Z  Garyp
    Modified so the /PERFLOG switch implies the /P switch.
    Revision 1.20  2007/01/14 05:51:02Z  Garyp
    Fixed to build properly.
    Revision 1.19  2007/01/14 03:08:21Z  Garyp
    Modified to output MemoryUsage as a VBF class rather than RESOURCES,
    and added VBF memory overhead.
    Revision 1.18  2007/01/08 01:37:42Z  Garyp
    Updated to display resource usage information and tweaked the PerfLog
    output.
    Revision 1.17  2007/01/04 01:22:43Z  Garyp
    Debug code updated.
    Revision 1.16  2007/01/02 22:48:19Z  Garyp
    Updated to interface with PerfLog.  Modified several tests to display a
    hex dump of mismatched data when read-back-verify fails.
    Revision 1.15  2006/12/26 04:12:50Z  Garyp
    Updated the partition mount and compaction performance measurement
    functions to operate in a similar fashion as the other benchmarking
    functions (should not affect the end results).
    Revision 1.14  2006/10/18 17:42:35Z  billr
    Revert changes made in revision 1.13.
    Revision 1.12  2006/08/28 16:34:16Z  billr
    Test both normal and aggressive compaction.
    Revision 1.11  2006/08/04 17:28:58Z  timothyj
    Changed isdigit() call to DclIsDigit().
    Revision 1.10  2006/05/04 01:40:34Z  Garyp
    Updated to use the new write interruption test interfaces.
    Revision 1.9  2006/03/05 01:57:15Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.8  2006/02/21 02:14:13Z  Garyp
    Partially updated to use the new VBF API.
    Revision 1.7  2006/02/12 18:27:23Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a
    single device/disk at a time.
    Revision 1.6  2006/02/11 23:31:23Z  Garyp
    Tweaked to build cleanly.
    Revision 1.5  2006/02/09 22:15:41Z  Garyp
    Eliminated the use of globals and made everything instanced so the test
    can run simultaneously on multiple disks.  Modified to no longer attempt
    to mount VBF while it is already mounted.
    Revision 1.4  2006/01/21 01:16:37Z  Rickc
    Fixed logic to check return value to vbfunmount
    Revision 1.3  2006/01/11 02:02:20Z  Garyp
    Updated to avoid compiler warnings in CE.
    Revision 1.2  2006/01/05 04:06:42Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/11/25 23:02:10Z  Pauli
    Initial revision
    Revision 1.6  2005/11/25 23:02:09Z  Garyp
    Modified to use DclRand().
    Revision 1.5  2005/11/16 00:50:12Z  Garyp
    Fixed the write and discard tests to not process the media more than
    one time (changes the results relative to historical data!).
    Revision 1.4  2005/11/02 13:39:17Z  Garyp
    Tweaked the partitiona and region mount tests to display the results in
    terms of milliseconds per mount.
    Revision 1.3  2005/10/21 03:35:10Z  garyp
    Modified the FillDisk test to not use an internal buffer that is a
    power-of-two size.
    Revision 1.2  2005/10/13 00:16:51Z  Garyp
    Modified to display the random seed before the test starts (again).
    Revision 1.1  2005/10/02 03:24:36Z  Garyp
    Initial revision
    Revision 1.2  2005/09/18 05:55:26Z  garyp
    Renamed vbfcompact() to FfxVbfCompact() and added FfxVbfCompactIfIdle().
    Revision 1.1  2005/08/03 19:34:26Z  pauli
    Initial revision
    Revision 1.50  2005/02/25 19:19:09Z  billr
    Use FfxRand() instead of rand() and srand() so stdlib.h isn't needed.
    Conditionally include ctype.h from the right place for Linux kernel build.
    Revision 1.49  2004/12/30 21:38:33Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.48  2004/10/28 20:14:36Z  GaryP
    Obsoleted the "/F" option.  Changed the meaning of the "/R" option to mean
    "reformat after the test is complete".  Updated loop logic in the perfor-
    mance tests to handle really fast performance cases.  Eliminated the use
    of D_STATIC.  Changed "garbage collection" terminology to "compaction".
    Revision 1.47  2004/09/23 08:43:59Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.46  2004/09/15 23:22:05Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.45  2004/07/20 00:09:03  GaryP
    Minor include file updates.
    Revision 1.44  2003/03/27 00:40:40Z  jaredw
    Fixed order of operators problem to allow printing of progress messages for
    the extensive writes test.
    Revision 1.43  2004/05/05 21:54:18  billr
    Thread-safe timers based on system clock tick.
    Revision 1.42  2004/04/28 02:20:05Z  garyp
    Added a success or failure message.  Moved the random seed display
    to the failure message.
    Revision 1.41  2004/01/07 19:21:38Z  garys
    Merge from FlashFXMT
    Revision 1.38.1.4  2004/01/07 19:21:38  garyp
    Minor debugging code changes.
    Revision 1.38.1.3  2003/12/25 00:06:40Z  garyp
    Changed all uses of VBF_API_BLOCK_SIZE to VBF_BLOCK_SIZE and eliminated any
    conditional code based on different values for those settings.
    Revision 1.38.1.2  2003/11/05 00:32:56Z  garyp
    Re-checked into variant sandbox.
    Revision 1.39  2003/11/05 00:32:56Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.38  2003/09/18 19:36:18Z  billr
    Change test for reserved space to eliminate a warning from some
    compilers when FFX_RESERVED_SPACE was zero.
    Revision 1.37  2003/09/11 21:18:02Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.36  2003/06/24 18:20:10Z  garyp
    Updated the output to eliminate all uses of \r.
    Revision 1.35  2003/06/11 18:16:08Z  billr
    Comment changes only.
    Revision 1.34  2003/06/10 16:18:55Z  billr
    Improve test pattern generation and checking in TestRegionBoundaries().
    Start using new patterns in some (but not all) other tests.
    Revision 1.33  2003/06/05 18:13:32Z  billr
    Make the "huge buffer" size equal to the maximum transfer size (now that
    _sysmalloc() can allocate that much memory). Use the global
    PartitionInformation structure instead of local copies. Tidy up long
    comment lines in the revision log.
    Revision 1.32  2003/06/04 23:44:07Z  billr
    Test enhancements: get partition information and perform a few sanity
    checks; TestSmallBlocks() now writes enough to fill the region plus one
    more block.
    Revision 1.31  2003/05/30 21:23:06Z  brandont
    Added checks for the minimal requirements needed to perform the
    TestDiscardSet() tests and added error checking on the VBF interface calls.
    Revision 1.30  2003/05/30 16:38:59  dennis
    Revision 1.29  2003/05/30 00:39:09  dennis
    TestDiscardSet now only ever discards things one block at a time.
    Revision 1.28  2003/05/29 06:58:54  garyp
    Fixed to work right with CE3.  Fixed more variables to be inited at run-time
    rather than compile time.
    Revision 1.27  2003/05/22 22:44:18Z  brandont
    Corrected a couple compiler warnings under Win CE related to the
    size of D_UINT16
    Revision 1.26  2003/05/22 19:59:24  garyp
    Fixed to initialize the default test settings at run-time rather than
    compile time to avoid problems in VxWorks where the global
    values from previous test executions are persistent.  Changed the
    pattern buffer to be allocated at run time.  Moved all the write
    interruption and region mount performance testing out of this module.
    Fixed several minor casting/signed/unsigned issues.
    Revision 1.25  2003/05/22 02:18:07Z  dennis
    When I changed the unit list to a linked list (from an array) there was one
    place that I missed.  That error caused a logical unit to sometimes be
    converted to the wrong linear unit (if there was a spent unit in the region
    with the same lnu as some other valid lgu) and THAT caused all kinds of
    problems.  That is fixed.  I also fixed a couple other silly mistakes in
    TestDiscardSet.  I also changed the logic in FfxVbfCompact so that the
    behavior of fxreclm is the same as it used to be when run on an empty disk.
    Revision 1.24  2003/05/21 20:46:48Z  dennis
    Had an overflow bug when calculating the address argument value
    to vbfxxx functions in TestDiscardSet
    Revision 1.23  2003/05/20 01:08:16  dennis
    Added new VBF test to check the behavior of the discard set
    (both implicit and explicit discards).
    Revision 1.22  2003/05/15 17:56:40  garyp
    Modified so that the buffers are allocated at run time to eliminate
    alignment problems.  Eliminated the obsolete and misplaced FAT format test.
    Revision 1.21  2003/05/09 04:55:01Z  dennis
    Changes to fix a discard bug when using metadata.   Also added some more
    checks in VBFDEB and did a little bit of cleanup of the code.
    Revision 1.20  2003/05/08 01:28:58Z  brandont
    Corrected math overflow when making sure the media is large enough
    to run the region boundary tests.
    Revision 1.19  2003/05/07 21:19:21  garys
    turned off region boundary test by default (added /B switch)
    Revision 1.18  2003/05/07 05:21:58  garyp
    Updated for new UNITTEST.H location.
    Revision 1.17  2003/05/06 01:29:55Z  garys
    Made including vbfdata.h and calling ClearInVbf() dependent on FX_OESLMONO
    Revision 1.16  2003/05/02 20:46:40  brandont
    Added a test which writes serialized data across region and erase
    zone boundaries and checks that the correct data was written
    without overrunning into other "sectors"
    Revision 1.15  2003/05/02 19:14:56  garyp
    Disabled write interruption testing if the project is not being built to
    support it.  Disabled MountRegion() performance testing if the code
    is not being built in All-In-One fashion.
    Revision 1.14  2003/05/01 22:06:02Z  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h.  core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.  Eliminate vbftst.h, it was not
    included anywhere else and contained information that was strictly internal
    here.  Print partition info after mounting.
    Revision 1.13  2003/04/30 17:59:57Z  billr
    Include ctype.h for isdigit(). Fix compiler warning about isdigit() in a
    more portable way. Return type of rand() and argument of srand() are
    unsigned int.
    Revision 1.12  2003/04/30 01:11:02Z  garys
    Seed rand with _sysgettickcount() instead of localtime() for CE
    Revision 1.11  2003/04/29 23:26:56  garyp
    Minor fixes to mollify the VxWorks/GNU compiler.
    Revision 1.10  2003/04/24 23:00:59  garys
    merged partition info changes with write interruption test changes
    Revision 1.9  2003/04/24 19:42:30  dennis
    Final version of write interruption tests.  Fail all calls when the
    region cannot be mounted in vbfxxx().
    Revision 1.8  2003/04/24 00:58:48  billr
    Fix FXINFO for VBF4. Change the UnitInformation structure and add
    vbfgetpartitioninfo() to support this.
    Revision 1.7  2003/04/23 01:20:52  dennis
    Added pseudo-random interruption order in the write interruption tests w/in
    VBFTST.C.  Removed VBFWINT.C from project and moved guts of that module into
    VBFTST.C; updated \common\tests\tests.mak.  Added stub functions to
    oesl\dos\driver\memclien.c and to \common\tools\wrimage.c so that these
    functions did not link in VBFTST.C during a debug build.  Added a new link
    line to \oesl\dos\tests\tests.mak so that the VBF unit test does not link in
    wrimage.c to satisfy the vbfwriteint function dependency instead of using
    the REAL vbfwriteint in VBFTST.C as it very very clearly should be doing.
    Revision 1.6  2003/04/22 01:05:03  dennis
    Added more write interruption tests and code to deal with them.
    GCs will now fully clean a region if called by a background process
    without disturbing temporal order.  GC "granularity" is decreased.
    Revision 1.5  2003/04/17 15:36:54  billr
    Merge from VBF4 branch.
    Revision 1.4  2003/04/15 20:36:12Z  garyp
    Header changes.
    Revision 1.3  2003/04/15 19:03:46Z  garyp
    Changed output to not use '\r'.  Use _sysmalloc() rather than malloc().
    Revision 1.10.1.12  2003/04/12 00:00:02Z  billr
    Exit with diagnostic message if there's not enough memory to allocate the
    huge buffer.
    Revision 1.10.1.11  2003/04/08 04:08:18Z  dennis
    Now use a combination of address+sequence_number in the
    string we write out in the "fill disk" and "write interruptions" tests.
    This makes sure we are reading the right copy of the data.
    Revision 1.2  2003/04/07 21:01:37Z  garys
    Fixed text string for early termination of GC test (milliseconds instead
    of seconds)
    Revision 1.10.1.10  2003/04/04 23:34:51  billr
    Add the new Media Manager code.
    Revision 1.10.1.9  2003/04/01 22:46:02Z  dennis
    Incorporated changes suggested byf vbfwrite walkthrough
    Revision 1.10.1.8  2003/03/26 22:42:15  dennis
    All unit default unit tests pass with metadata working on NXMS,
    comments, functions and data cleaned up.
    Revision 1.1  2003/03/26 09:23:44Z  garyp
    Initial revision
    Revision 1.10.1.7  2003/03/20 20:28:52Z  dennis
    Factored the wear leveling code into its own policy function
    Revision 1.10.1.6  2003/03/19 04:33:18  dennis
    With the exception of the Structure Tests, the default TESTVBF tests pass.
    Revision 1.10.1.4  2003/03/04 06:07:29  dennis
    Refactored _vbfwrite to write blocks into a region.   WriteNewBlock
    now transparently handles retries of partial writes due to I/O errors.
    WriteConsecutiveBlocks discards data that failed to be fully written
    to the flash.  WriteConsecutiveBlocks attempts to explicitly discard
    any valid data that is being implicitly discarded (up to the MM what
    to do about that.)   All write routines now return a IOReturn_t struct.
    Revision 1.10.1.3  2003/02/24 15:13:02  dennis
    Changed the in-memory Allocation struct definition and changes to the MM
    interface.
    Revision 1.12  2003/02/13 19:32:22Z  garys
    Don't include stddef.h for CE 3.0
    Revision 1.10.1.2  2003/01/21 18:57:55  dennis
    Established temporal order to units within a region.  Expanded the
    representation of unit counts to 32-bits.
    Revision 1.11  2002/11/30 23:20:48  dennis
    No changes.
    Revision 1.10  2002/11/30 23:20:48  dennis
    Changed IsValidEUH() to compare the entire VBF signature in the EUH being
    read off the disk.  Moved the VBF signature definition to VBF.H and use it
    directly throughout the project.  Renamed SIGNATURE_SIZE to
    VBF_SIGNATURE_SIZE to match the name of the new VBF_SIGNATURE string
    constant.
    Revision 1.9  2002/11/23 03:05:40  garyp
    Updated to again include STDDEF.H (needed for the ARM tools).
    Revision 1.8  2002/11/15 20:30:55Z  garys
    Modified GetGarbagePerform() to only return a performance value if
    sufficient garbage collections are needed.  Added BOOLEAN return value to
    indicate insufficient garbage collections needed.
    Revision 1.7  2002/11/15 08:58:18  garyp
    Fixed to move STRING.H ahead of the other includes so that STDDEF.H
    will also be included first (necessary for BC builds).
    Revision 1.6  2002/11/15 05:38:46Z  garyp
    Eliminated STDDEF.H
    Revision 1.5  2002/11/14 19:08:26Z  garys
    fixed /c option, passing parameters to oeminit in SetSwitches(),
    as long as there are no spaces between them.
    Revision 1.4  2002/11/13 23:46:16  garyp
    Cleaned up the included headers.
    Revision 1.3  2002/11/13 08:10:28Z  garys
    Moved StartTimer(), CheckTimer(), EndTimer() to timer.c
    Removed printouts while timing for performance tests
    Changed performance test period from 30 to 7 seconds.
    11/08/02 gjs New directory structure, removed some OS-specific code.
    10/10/02 gjs In TestRegionInfoStruct(), inDownwardResize is D_BOOL.
    09/28/02 gjs New field in TestRegionInfoStruct(), inDownwardResize
    09/27/02 de  Changed counter in write interruption tests to be monotonic.
    09/26/02 de  Write interruption tests now use setjmp/longjmp and remount
                 the disk following an interruption.  This more faithfully
                 simulates a power loss.
    09/03/02 pkg Added structure alignment checks for the
                 UNIT_ALLOC_OFFSET_CACHE entry of the region cache
    08/29/02 gjs Removed farfree and farmalloc, which fail to compile under CE
    07/29/02 de  Included NVBF.H and added symbolic constants.
    07/22/02 de  Added calls to allocate and free a buffer used to
                 verify that we don't exceed 1 write to a main page
                 and 2 writes to a redundant page of NAND flash when
                 run under DOS.  Added structure member tests for EUH and
                 region info sequence numbers.
    04/04/02 gjs Cast IS_ALIGNED output to D_BOOL for SetTestRecord() input
                 on particular lines for the CE 2.10 SA1100 compiler.
    03/19/02 gjs Fixed usage for /C option (space before the string).
    02/14/02 gjs Added CheckVBFStructs() with DE.
                 Call TestTypeSizes() in TestVbfLayer().
    01/08/02 gjs Changed error codes to __LINE__.
    01/07/02 gjs Call TestTypeSizes() in main.  Moved several variable
                 declarations before code block for archaic compilers.
                 Removed local implementation of _sysassert.
    10/01/01 mjm Added VBF patent info.
    01/10/01 gp  Added support for __WINNT_TARGET__.  Fixed the "history"
                 and "todo" entries that were interspersed.
    12/01/00 TWQ Removed un-needed (unused) variable definition from SetSwitches
    09/15/00 HDS Added FORMAT_MEDIA to conditional inclusion test.
    09/14/00 HDS Moved #include <flashfx.h> statement to top to insure that
                 __LINUX_TARGET__ is defined before it is used.
    07/29/00 HDS Fixed command line option /c processing.
    07/11/00 mrm Added ifdef to exclude entire file when not needed.
    06/23/00 mrm Added __TEST_VBF define to OEMCONF.H
    03/14/00 PKG Increased number of iterations in TestSmallBlocks to better
                 test large unit size.
    01/13/00 HDS Modified TestDiscard to better handle the discard same sector
                 twice case.
    01/12/00 HDS Added a couple of typecasts to avoid compiler warnings.
    01/10/00 HDS Modified TestDiscard special case (erase zone size of 256KB)
                 to be compatible with new discard approach.
    12/18/99 HDS Fixed a problem in TestDiscard to insure checking correct data.
    12/03/99 HDS Modified TestDiscard and TestFillDisk functions to be
                 compatible with new discard approach.
    11/16/99 AJC Changed main() to be callable from the VxWorks shell.
    11/03/99 TWQ Added Conditionals for MALLOC and FREE calls.. See DOCTODO..
    10/24/99 HDS Added call to vbfdiscard in TestInterruptions to test this
                 module for potential interruption problems.
    02/04/99 HDS Changed the performance sampling period from 10 to 30 seconds
                 to better account for the effects of garbage collections.
    12/16/98 PKG Removed un-used local initialization to avoid compiler warnings
    12/16/98 PKG Added VMAPI support code (conditional compile).  It requires
                 that a pre-existing file be allocated to run
    12/16/98 PKG Changed SetTimer to StartTimer to avoid clash with Windows API
                 SetTimer function call
    12/15/98 PKG Added direct call to stub_printf to avoid compiler warns
    12/13/98 PKG stub DclPrintf() calls when in a NO_DISPLAY environment
    11/19/98 PKG Setup Write Interruption tests to correctly verify non-
                 interrupted writes.  The data should not become corrupted
    09/15/98 PKG Added dllimport declarator to printf to avoid compiler warns
    08/28/98 HDS Changed TestFillDisk to use VBF_BLOCK_SIZE instead of
                 TEST_BUFFER_SIZE.
    08/28/98 HDS Expanded the TestDiscard Function to test lengths that are not
                 a multiple of the VBF block size.
    08/21/98 HDS Added a conditional compile call to fatformat for additional
                 testing.
    08/21/98 HDS Changed the Reformat media size to be a fixed 2MB.
    08/21/98 HDS Defined TEST_BUFFER_SIZE in terms of VBF_API_BLOCK_SIZE and
                 buffer in terms of VBF_BLOCK_SIZE.
    08/19/98 PKG Removed vbfprogressmessage() since it is already defined in
                 the VBF library
    08/05/98 PKG Added proto-type for the local DclPrintf()
    07/29/98 HDS Recontructed current version - previous changes lost in
                 subsequent releases.
    07/02/98 PKG Change copyright date to 1998.
    03/12/98 HDS Added checks at start of test to insure no oem change, ready
                 and not write protected.
    03/10/98 PKG Fixed build of unit test in stand-alone environment.
    03/09/98 PKG Setup so it can stand-alone when compiled via NO_MAIN and
                 moved malloc from main to TestVbfLayer().
    02/26/98 PKG Added vbfprogress function in case it is needed.
    02/26/98 HDS Fixed 3 printfs in TestFillDisk.
    02/20/98 HDS Fixed TestFillDisk to handle counts > 64K.
    01/29/98 HDS Fixed TestFillDisk to handle counts more than 64KB.
    12/04/97 HDS Added drive number to test display.
    12/04/97 HDS Changed command line parameter /Ux to /Dx.
    12/02/97 HDS Fixed UnitInfo related problems.
    11/25/97 HDS Changed oemsetunit() call to oemsetcurrentdrive() and
                 vbfsetunit() call to vbfsetcurrentdrive().
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    08/07/97 PKG Added test for the optional unit info API call
    08/26/97 PKG Now uses new Datalight specific types
    08/22/97 PKG Fixed Huge test to return success if media is too small
    07/24/97 PKG Changed /S option to U for consistant naming
    06/26/97 PKG Fixed support of multiple unit testing
    06/11/97 PKG Removed use of EXT_API options
    05/08/97 DM  Added benchmark option and benchmark routines
    05/07/97 PKG Fixed free bug (mix of near and far)
    05/02/97 PKG Removed un-used include files
    04/30/97 DM  Made all vAddress locals unsigned to avoid warnings
    04/17/97 PKG Removed include of DOS.H, not needed
    03/25/97 PKG Changed default format options
    01/28/97 PKG Original revision from CardTrick v3.01
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <fxtrace.h>
#include <fxinfohelp.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT

#include <fxfmlapi.h>
#include <vbf.h>
#include <dlperflog.h>
#include <fxconfig.h>
#include "vbftst.h"

#define ARGBUFFLEN            128

#define SERIALIZED_DATA_LENGTH   63

/*  This is the number of milliseconds to test/benchmark
    the access routines.  Used by Benchmark() routine.
*/
#define NUM_MILLISECONDS      (10000UL)
#define MSEC_PER_SEC          (1000UL)

#define WEARLEV_SAMPLE_MINUTES        (1)   /* Minutes between each sampling */
#define WEARLEV_PERCENT_DEFAULT      (25)
#define WEARLEV_MAX_SEQWRITES        (16)   /* Max pages to write at once */

/*-------------------------------------------------------------------
                              Prototypes
-------------------------------------------------------------------*/
static void      ShowUsage(  FFXTOOLPARAMS *pTP);
static D_INT16   SetSwitches(FFXTOOLPARAMS *pTP, VBFTESTINFO *pTI);
static D_BOOL    FormatPartition(         VBFTESTINFO *pTI);
static D_BOOL    RemountVBF(              VBFTESTINFO *pTI);
static D_INT16   TestVbfLayer(            VBFTESTINFO *pTI);
static D_INT16   TestPartitionInfo(       VBFTESTINFO *pTI);
static D_INT16   TestSmallBlocks(         VBFTESTINFO *pTI);
static D_INT16   TestBigBlocks(           VBFTESTINFO *pTI);
static D_INT16   TestHugeBlocks(          VBFTESTINFO *pTI);
static D_INT16   TestHugeOperations(      VBFTESTINFO *pTI);
static D_INT16   TestRegionBoundaries(    VBFTESTINFO *pTI);
static D_INT16   TestExercise(            VBFTESTINFO *pTI);
static D_INT16   TestUnitInfo(            VBFTESTINFO *pTI);
static D_INT16   TestCompaction(          VBFTESTINFO *pTI);
static D_INT16   TestDiscard(             VBFTESTINFO *pTI);
static D_INT16   TestErrors(              VBFTESTINFO *pTI);
static D_INT16   TestWearLeveling(        VBFTESTINFO *pTI);
static D_INT16   TestFillDisk(            VBFTESTINFO *pTI);
static D_INT16   TestDiscardSet(          VBFTESTINFO *pTI);
static void      Benchmark(               VBFTESTINFO *pTI);
static void      ResourceUsage(           VBFTESTINFO *pTI);
static D_UINT32  PremountRegions(         VBFTESTINFO *pTI);
static D_UINT32  GetReadPerform(          VBFTESTINFO *pTI, D_UINT32 ulMaxPages);
static D_UINT32  GetWritePerform(         VBFTESTINFO *pTI, D_UINT32 *pulMaxPages);
static D_UINT32  GetDiscardPerform(       VBFTESTINFO *pTI, D_UINT32 ulMaxPages);
static D_UINT32  GetCompactionPerf(       VBFTESTINFO *pTI);
static D_UINT32  GetDiskMountPerf(        VBFTESTINFO *pTI);
static D_UINT32  SerializeBuffer(         VBFTESTINFO *pTI, D_UCHAR * pBuffer, D_UINT32 ulBufferSize, D_UINT32 ulPage, const char *pszTestName);
static FFXSTATUS GatherEraseCounts(       VBFTESTINFO *pTI, D_UINT32 *pulMinErases, D_UINT32 *pulMaxErases, D_UINT32 *pulAvgErases, unsigned fDisplay);
static D_UINT32  ValidateSerializedBuffer(VBFTESTINFO *pTI, D_UCHAR * pBuffer, D_UINT32 ulBufferSize, D_UINT32 ulPage, D_UINT32 ulWriteSize, D_UINT32 ulBlockNumber, const char *pszTestName, D_UINT32 ulSerialNum);
static void      FillPattern(             char *pattern, D_UINT32 ulPage, D_UINT32 ulWriteSize, D_UINT32 ulBlockNumber, const char *pszTestName, D_UINT32 ulSerialNum);

#endif


/*-------------------------------------------------------------------
    Protected: FfxTestVBF()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxTestVBF(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT
    D_INT16         iErrorLevel;
    D_UINT32        ulOriginalSeed;
    VBFTESTINFO    *pTI = NULL;

    DclProductionAssert(pTP);

    DclPrintf("\nFlashFX VBF Unit Test\n");
    FfxSignOn(FALSE);

    pTI = DclMemAllocZero(sizeof *pTI);
    if(!pTI)
        return __LINE__;

    pTI->ulTestMemUsage = sizeof *pTI;
    pTI->ulRandSeed = DclOsTickCount();
    pTI->nVerbosity = 1;
    pTI->nWearLevStaticPercent = WEARLEV_PERCENT_DEFAULT;

    iErrorLevel = SetSwitches(pTP, pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
        goto Cleanup;

    pTI->hFML = FfxFmlHandle(pTP->nDiskNum);
    if(!pTI->hFML)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        iErrorLevel = __LINE__;
        goto Cleanup;
    }

    /*  If VBF is already initialized for this Disk, get the handle
    */
    pTI->hVBF = FfxVbfHandle(pTP->nDiskNum);
    if(pTI->hVBF)
    {
        pTI->fVBFInitialized = TRUE;

      #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
        /*  We don't want idletime/background compaction in effect
            while running VBFTEST.  Increment the suspend flag, and
            save the original suspend level so we can restore it on
            exit.
        */
        pTI->ulSuspendCount = FfxVbfCompactIdleSuspend(pTI->hVBF);

        DclAssert(pTI->ulSuspendCount > 0);

        if(pTI->ulSuspendCount != D_UINT32_MAX)
            pTI->ulSuspendCount--;
      #endif
    }

    /*  Display the random seed both before and after the test so we
        have a better chance of obtaining it if we need to recreate
        a problem.
    */
    ulOriginalSeed = pTI->ulRandSeed;
    DclPrintf("Random Seed = %lU\n", pTI->ulRandSeed);

    /*  Go perform the unit tests
    */
    iErrorLevel = TestVbfLayer(pTI);

    if(iErrorLevel)
    {
        DclPrintf("VBF Unit Test FAILED, Error Code = %U, Random Seed = %lU\n",
                  iErrorLevel, ulOriginalSeed);
    }
    else
    {
        DclPrintf("VBF Unit Test PASSED\n");
    }

  Cleanup:
    DclMemFree(pTI);

    return iErrorLevel;

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ShowUsage()

    Shows help to the user and returns.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nSyntax:  %s disk [options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where:\n");
    DclPrintf("  disk           The disk designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  /?             This help screen\n");
    DclPrintf("  /R             Reformat the media upon test completion\n");
    DclPrintf("  /E[:n]         Extensive write/erase tests to the entire media, where 'n' is the\n");
    DclPrintf("                 number of minutes to run the test. If not specified, this test\n");
    DclPrintf("                 exercises the entire media.\n");
    DclPrintf("  /P             Execute the performance benchmark tests\n");
    DclPrintf("  /I             Test write interruptions\n");
    DclPrintf("  /B             Test region boundaries\n");
    DclPrintf("  /H             Test huge operations\n");
    DclPrintf("  /S:n           Set test random seed\n");
    DclPrintf("  /V:n           The Verbosity level 0-3 (Default=1)\n");
    DclPrintf("  /Wear:min[,n]  Test wear-leveling, where 'min' is the number of minutes to\n");
    DclPrintf("                 run the test, and 'n' is the optional percent of the disk\n");
    DclPrintf("                 which will be filled with static data (default is 25%).\n");
    DclPrintf("  /PerfLog[:sfx] Output performance test results in CSV form, in addition to\n");
    DclPrintf("                 the standard output.  Implies the /P switch.  The optional\n");
    DclPrintf("                 'sfx' value is a text suffix which will be appended to the\n");
    DclPrintf("                 build number in the PerfLog CSV output.\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.  Also shows help via ShowUsage() if needed.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 SetSwitches(
    FFXTOOLPARAMS  *pTP,
    VBFTESTINFO    *pTI)
{
    D_INT16         i, j;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        argc;

    argc = DclArgCount(pTP->dtp.pszCmdLine);

    /*  Parse each string
    */
    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, i, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("\nBad argument!\n");
            return __LINE__;
        }

        if(DclStrNICmp(achArgBuff, "/PERFLOG", 8) == 0)
        {
            pTI->fPerformanceTests = TRUE;
            pTI->fPerfLog = TRUE;

            if(achArgBuff[8] == ':')
            {
                DclStrNCpy(pTI->szPerfLogSuffix, &achArgBuff[9], sizeof(pTI->szPerfLogSuffix));
                pTI->szPerfLogSuffix[sizeof(pTI->szPerfLogSuffix)-1] = 0;
            }

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/WEAR:", 6) == 0)
        {
            pTI->fTestWearLeveling = TRUE;

            pTI->ulWearLevMinutes = DclAtoL(&achArgBuff[6]);
            if(pTI->ulWearLevMinutes < 1)
            {
                DclPrintf("The wear-leveling test must run for at least 1 minute\n");
                return __LINE__;
            }

            /*  Cast quiets a compiler warning on broken
                implementations of isdigit() macro.
            */
            j = 6;
            while(DclIsDigit((unsigned char)achArgBuff[j]))
                j++;

            if(achArgBuff[j] == ',')
            {
                pTI->nWearLevStaticPercent = DclAtoL(&achArgBuff[j+1]);

                if(pTI->nWearLevStaticPercent > 99)
                {
                    DclPrintf("Wear-leveling percent %u is not valid.  Must range from 0 to 99\n",
                        pTI->nWearLevStaticPercent);

                    return __LINE__;
                }
            }
            else if(achArgBuff[j] != 0)
            {
                DclPrintf("\n  Bad option: %s\n", achArgBuff);
                return __LINE__;
            }

            continue;
        }

        /*  Allow for multiple switches in a string
        */
        j = 0;
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
                    case '?':
                        ShowUsage(pTP);
                        return __LINE__;

                        /*  Perform extensive tests (fill up the disk)
                        */
                    case 'e':
                    case 'E':
                        ++j;
                        pTI->fExtensiveTests = TRUE;
                        if(*(achArgBuff + j) == ':')
                        {
                            ++j;
                            pTI->ulExtensiveMinutes = DclAtoL((achArgBuff + j));

                            /*  Cast quiets a compiler warning on broken
                            implementations of isdigit() macro.
                            */
                            while(DclIsDigit((unsigned char)achArgBuff[j]))
                            {
                                ++j;
                            }
                        }
                        break;

                        /*  Perform region boundary tests
                        */
                    case 'b':
                    case 'B':
                        ++j;
                        pTI->fRegionBoundaries = TRUE;
                        break;

                        /*  Write interruptions
                        */
                    case 'i':
                    case 'I':
                        ++j;
                        pTI->fTestWriteInterruptions = TRUE;
                        break;

                        /*  Performance tests
                        */
                    case 'p':
                    case 'P':
                        ++j;
                        pTI->fPerformanceTests = TRUE;
                        break;

                    case 'r':
                    case 'R':
                        ++j;
                        pTI->fReformat = TRUE;
                        break;

                        /*  Huge Operations tests
                        */
                    case 'h':
                    case 'H':
                        ++j;
                        pTI->fHugeTests = TRUE;
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

                        pTI->ulRandSeed = DclAtoL(achArgBuff + j);

                        /*  Cast quiets a compiler warning on broken
                            implementations of isdigit() macro.
                        */
                        while(DclIsDigit((unsigned char)achArgBuff[j]))
                        {
                            ++j;
                        }
                        break;

                    case 'q':
                    case 'Q':
                        ++j;
                        if(achArgBuff[j] != ':')
                        {
                            DclPrintf("Syntax error in \"%s\"\n", achArgBuff);
                            return __LINE__;
                        }
                        ++j;

                        pTI->lQATestNum = DclAtoL(achArgBuff + j);

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

                        pTI->nVerbosity = (unsigned)DclAtoI(achArgBuff + j);
                        if(pTI->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
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

                        /*  Unrecognized switch
                        */
                    default:
                        DclPrintf("\n  Bad option: %s\n", (achArgBuff + j));
                        return __LINE__;
                }
            }
            else
            {
                DclPrintf("\n  Bad option: %s\n", (achArgBuff + j));
                return __LINE__;
            }
        }
    }

    /*  All worked fine!
    */
    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestVBFLayer()

    Tests the VBF Layer.  Several tests are performed on the
    media, and some are controlled via switches set prior to
    the call to TestVbfLayer().

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns zero if successful, otherwise a non-zero line number
-------------------------------------------------------------------*/
static D_INT16 TestVbfLayer(
    VBFTESTINFO    *pTI)
{
    D_INT16         iErrorLevel;

    if(!FormatPartition(pTI))
        return __LINE__;

    pTI->hVBF = FfxVbfCreate(pTI->hFML);
    if(!pTI->hVBF)
    {
        DclPrintf("VBF Create failed!\n");
        return __LINE__;
    }

    /*  Reminder that VBF instances are created with the idle-time
        compaction feature (if any) in suspended state, which is
        exactly where we want it for VBFTEST.
    */

    /*  Retrieve the VfbDiskInfo
    */
    if(FfxVbfDiskInfo(pTI->hVBF, &pTI->DiskInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("FfxVbfDiskInfo() failed\n");
        return __LINE__;
    }

    /*  As a side effect, TestPartitionInfo() initializes
        pTI->DiskInfo and pTI->DiskMets.
    */
    iErrorLevel = TestPartitionInfo(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed at line %U!\n", iErrorLevel);
        goto _DoneVbfTests;
    }

    /*  Numerous places in the code simply assume that the test and
        pattern buffers are each three allocation blocks.
    */
    pTI->ulTestBufferBlocks = 3;
    pTI->ulHugeBufferBlocks = (96 * 1024UL) / pTI->DiskInfo.uPageSize;
    pTI->ulTestBufferSize = pTI->ulTestBufferBlocks * pTI->DiskInfo.uPageSize;
    pTI->ulHugeBufferSize = pTI->ulHugeBufferBlocks * pTI->DiskInfo.uPageSize;

    /*  Allocate the standard buffer
    */
    pTI->ulTestMemUsage += pTI->ulTestBufferSize;
    pTI->pBuffer = DclMemAlloc(pTI->ulTestBufferSize);
    if(pTI->pBuffer == NULL)
    {
        DclPrintf("Error: unable to allocate buffer\n");
        iErrorLevel = __LINE__;
        goto _DoneVbfTests;
    }

    /*  Allocate the pattern buffer
    */
    pTI->ulTestMemUsage += pTI->ulTestBufferSize + 1;
    pTI->pPatBuff = DclMemAlloc(pTI->ulTestBufferSize + 1);
    if(pTI->pPatBuff == NULL)
    {
        DclPrintf("Error: unable to allocate pattern buffer\n");
        iErrorLevel = __LINE__;
        goto _DoneVbfTests;
    }

    /*  Allocate the huge block buffer
    */
    pTI->ulTestMemUsage += pTI->ulHugeBufferSize;
    pTI->pHugeBuff = DclMemAlloc(pTI->ulHugeBufferSize);
    if(pTI->pHugeBuff == NULL)
    {
        DclPrintf("\nConfiguration problem -- can't allocate huge buffer\n");
        iErrorLevel = __LINE__;
        goto _DoneVbfTests;
    }

    /*  Extended QA tests are always run in isolation (no other tests are
        run).
    */
    if (pTI->lQATestNum)
    {
        iErrorLevel = FfxTestVBFQA(pTI);
        goto _DoneVbfTests;
    }

    /*  The benchmark tests cannot fail.  Do the performance tests as the
        first thing after formatting so that the results are not skewed
        by running on "dirty" flash.

        We should also ensure that background compaction is disabled!
    */
    if(pTI->fPerformanceTests)
    {
        Benchmark(pTI);
    }

    iErrorLevel = TestSmallBlocks(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed at line %U!\n", iErrorLevel);
        goto _DoneVbfTests;
    }
    iErrorLevel = TestBigBlocks(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed at line %U!\n", iErrorLevel);
        goto _DoneVbfTests;
    }
    if(pTI->fTestUnitInfo)
    {
        iErrorLevel = TestUnitInfo(pTI);
        if(iErrorLevel != SUCCESS_RETURN_CODE)
        {
            DclPrintf("\n Failed at line %U!\n", iErrorLevel);
            goto _DoneVbfTests;
        }
    }

    if(pTI->fRegionBoundaries)
    {
        iErrorLevel = TestRegionBoundaries(pTI);
        if(iErrorLevel != SUCCESS_RETURN_CODE)
        {
            DclPrintf("\n Failed!\n");
            goto _DoneVbfTests;
        }
    }
    iErrorLevel = TestDiscardSet(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }
    iErrorLevel = TestHugeBlocks(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }
    if(pTI->fHugeTests)
    {
        iErrorLevel = TestHugeOperations(pTI);
        if(iErrorLevel != SUCCESS_RETURN_CODE)
        {
            DclPrintf("\n Failed!\n");
            goto _DoneVbfTests;
        }
    }
    iErrorLevel = TestExercise(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }
    iErrorLevel = TestDiscard(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }
    iErrorLevel = TestErrors(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }
    if(pTI->fExtensiveTests)
    {
        iErrorLevel = TestFillDisk(pTI);
        if(iErrorLevel != SUCCESS_RETURN_CODE)
        {
            DclPrintf("\n Failed!\n");
            goto _DoneVbfTests;
        }
    }
    iErrorLevel = TestCompaction(pTI);
    if(iErrorLevel != SUCCESS_RETURN_CODE)
    {
        DclPrintf("\n Failed!\n");
        goto _DoneVbfTests;
    }

    if(pTI->fTestWriteInterruptions)
    {
      #if VBF_WRITEINT_ENABLED
        {
            DclPrintf("\nTesting write interruptions...\n");

            iErrorLevel = FfxVbfTestWriteInterruptions(pTI->hVBF, pTI->ulRandSeed, pTI->nVerbosity);
            if(iErrorLevel != SUCCESS_RETURN_CODE)
                goto _DoneVbfTests;
        }
      #else
        {
            DclPrintf("Write interruption testing is disabled\n");
            iErrorLevel = SUCCESS_RETURN_CODE;
        }
      #endif
    }

    if(pTI->fTestWearLeveling)
    {
        iErrorLevel = TestWearLeveling(pTI);
        if(iErrorLevel != SUCCESS_RETURN_CODE)
        {
            DclPrintf("\n Failed!\n");
            goto _DoneVbfTests;
        }
    }

    if(pTI->fPerformanceTests)
    {
        ResourceUsage(pTI);
    }

  _DoneVbfTests:

    if(pTI->fReformat)
    {
        DclPrintf("Test complete, reformatting partition...\n");
        FormatPartition(pTI);
    }

    if(pTI->fVBFInitialized)
    {
        /*  Restore VBF to its original state -- if it was initialized
            when we were entered, ensure it is initialized on exit.
        */
        if(!pTI->hVBF)
        {
            pTI->hVBF = FfxVbfCreate(pTI->hFML);
            if(!pTI->hVBF)
                DclPrintf("VBF Create failed!\n");
        }

      #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
        /*  If there is a VBF instance and the original suspend count is
            not an error condition, suspend or resume as is necessary
            to restore the original suspend count.
        */
        if(pTI->hVBF && pTI->ulSuspendCount != D_UINT32_MAX)
        {
            D_UINT32    ulNewCount;

            if(pTI->ulSuspendCount < 1)
            {
                ulNewCount = FfxVbfCompactIdleResume(pTI->hVBF);
                DclAssert(ulNewCount == 0);
            }
            else if(pTI->ulSuspendCount > 1)
            {
                ulNewCount = 1;

                while(ulNewCount < pTI->ulSuspendCount)
                    ulNewCount = FfxVbfCompactIdleSuspend(pTI->hVBF);
            }
            else
            {
                /*  Newly created VBF instances always have a suspend
                    count of 1, so nothing to do.
                */
                DclAssert(pTI->ulSuspendCount == 1);
            }
        }
      #endif
    }
    else
    {
        /*  Restore VBF to its original state -- if it was not initialized
            when we were entered, make sure it is not initialized on exit
        */
        if(pTI->hVBF)
            FfxVbfDestroy(pTI->hVBF, FFX_SHUTDOWNFLAGS_NORMAL);
    }

    /*  Release resources in reverse order of allocation.
    */
    if(pTI->pHugeBuff)
        DclMemFree(pTI->pHugeBuff);
    if(pTI->pPatBuff)
        DclMemFree(pTI->pPatBuff);
    if(pTI->pBuffer)
        DclMemFree(pTI->pBuffer);

    return iErrorLevel;
}


/*-------------------------------------------------------------------
    Local: FormatPartition()

    This function formats the VBF partition using default parameters.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL FormatPartition(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulSpareUnits = FFX_USE_DEFAULT;
    FFXSTATUS       ffxStat;

    /*  Must destroy the VBF instance before formatting.
    */
    if(pTI->hVBF)
    {
        FfxVbfDestroy(pTI->hVBF, FFX_SHUTDOWNFLAGS_TIMECRITICAL);
        pTI->hVBF = NULL;
    }

    DclPrintf("Formatting media...\n");

    ffxStat = FfxVbfFormat(pTI->hFML, FFX_USE_DEFAULT, ulSpareUnits, FALSE);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("VBF Format failed, Status=%lX\n", ffxStat);
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: RemountVBF()

    This function destroys the existing VBF instance and
    creates a new one.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns TRUE if successful, else FALSE.
-------------------------------------------------------------------*/
static D_BOOL RemountVBF(
    VBFTESTINFO    *pTI)
{
    FFXSTATUS       ffxStat;

  #if FFXCONF_QUICKMOUNTSUPPORT
  
    /*  VBF tests may need to do a complete rescan of the media
        when the disk is recreated, so disable QuickMount here:
    */
    FfxVbfDisableStateSave(pTI->hVBF);
  
  #endif
  
    ffxStat = FfxVbfDestroy(pTI->hVBF, FFX_SHUTDOWNFLAGS_NORMAL);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error destroying VBF instance, Status=%lX\n", ffxStat);
        return FALSE;
    }

    pTI->hVBF = FfxVbfCreate(pTI->hFML);
    if(!pTI->hVBF)
    {
        DclPrintf("Error creating VBF instance\n");
        return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: TestPartitionInfo()

    Obtain and print information about the partition.  Check the
    values for sanity.  The partition information is retained for
    future reference.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns 0 if successful, otherwise a line number
-------------------------------------------------------------------*/
static D_INT16 TestPartitionInfo(
    VBFTESTINFO    *pTI)
{
    FFXSTATUS       ffxStat; 
    
    /*  Get the information about this disk
    */
    if(FfxVbfDiskInfo(pTI->hVBF, &pTI->DiskInfo) != FFXSTAT_SUCCESS)
        return __LINE__;

    if(FfxVbfDiskMetrics(pTI->hVBF, &pTI->DiskMets) != FFXSTAT_SUCCESS)
        return __LINE__;

    DclPrintf("\n#########################  VBF Disk Information  #########################\n");

    ffxStat = FfxDisplayVbfDiskInfo(pTI->hVBF, TRUE);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("FfxDisplayVbfDiskInfo() failed with status %lX\n", ffxStat);
        return __LINE__;
    }

    DclPrintf("##########################################################################\n\n");

    /*  The total pages in a region is an integral number of total pages in a unit.
    */
    if(pTI->DiskMets.ulRegionTotalPages % pTI->DiskMets.uUnitTotalPages != 0)
        return __LINE__;

    /*  The client pages in a region is an integral number of client pages in a unit.
    */
    if(pTI->DiskMets.ulRegionClientPages % pTI->DiskMets.uUnitClientPages != 0)
        return __LINE__;

    /*  The number of blocks in a region is expected to fit in a D_UINT16.
        The cast below is safe because of this limitation
    */
    if(pTI->DiskMets.ulRegionTotalPages > D_UINT16_MAX)
        return __LINE__;

    pTI->uUnitsPerRegion = (D_UINT16)pTI->DiskMets.ulRegionClientPages / pTI->DiskMets.uUnitClientPages;

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfvbf.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->fPerfLog, NULL, "VBF", "Configuration", NULL, pTI->szPerfLogSuffix);
        DCLPERFLOG_NUM(  hPerfLog, "RegionSize",    pTI->DiskMets.ulRegionClientPages * pTI->DiskMets.uPageSize);
        DCLPERFLOG_NUM(  hPerfLog, "UnitSize",      pTI->DiskMets.uUnitTotalPages * pTI->DiskMets.uPageSize);
        DCLPERFLOG_NUM(  hPerfLog, "PageSize",      pTI->DiskMets.uPageSize);
        DCLPERFLOG_NUM(  hPerfLog, "DiskUnits",     pTI->DiskMets.ulTotalUnits);
        DCLPERFLOG_NUM(  hPerfLog, "DiskPages",     pTI->DiskMets.ulClientPages);
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestSmallBlocks()

    Write and verify a block of data to the same client address
    enough times to completely fill all the data blocks in a region
    plus one.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
-------------------------------------------------------------------*/
static D_INT16 TestSmallBlocks(
    VBFTESTINFO    *pTI)
{
    D_UINT16        j;
    D_UINT32        ulPage = (pTI->DiskInfo.ulTotalPages / 2);
    D_UINT16        count;
    static char     testName[] = "TestSmallBlocks";
    D_UINT32        ulFailed;
    FFXIOSTATUS     ioStat;

    DclPrintf("Starting Write Blocks...\n");

    /*  The number of blocks to write is one more than the most data
        blocks that could be in all of the units of the region.  Note
        that there might be fewer blocks in the region containing the
        selected address (if it is the last region).
    */
    count = pTI->uUnitsPerRegion * pTI->DiskMets.uUnitDataPages + 1;

    for(j = 0; j < count; ++j)
    {
        D_UINT32 ulSerialNumber1, ulSerialNumber2;

        if((j == 0) || !((j + 1) % 10))
            DclPrintf("    Allocation block %U of %U\n", j + 1, count);

        ulSerialNumber1 = SerializeBuffer(pTI, pTI->pBuffer, pTI->DiskMets.uPageSize, ulPage, testName);
        DclMemSet(pTI->pBuffer + pTI->DiskMets.uPageSize, 0, pTI->DiskMets.uPageSize);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("   FfxVbfWritePages() FAILED at block %U, %s\n", j + 1, FfxDecodeIOStatus(&ioStat));
            return __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0, pTI->DiskMets.uPageSize);
        ulSerialNumber2 = SerializeBuffer(pTI, pTI->pBuffer + pTI->DiskMets.uPageSize,
                                          pTI->DiskMets.uPageSize, ulPage, testName);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("   FfxVbfReadPages() FAILED at block %U, %s\n", j + 1, FfxDecodeIOStatus(&ioStat));
            return __LINE__;
        }

        ulFailed = ValidateSerializedBuffer(pTI, pTI->pBuffer, pTI->DiskMets.uPageSize,
                                           ulPage, pTI->DiskMets.uPageSize, 0,
                                           testName, ulSerialNumber1);
        if(ulFailed)
        {
            DclPrintf("   Validate FAILED at block %U, offset %U\n", j + 1, ulFailed-1);
            DclHexDump(NULL, HEXDUMP_UINT8, 32, pTI->DiskMets.uPageSize, pTI->pBuffer);
            return __LINE__;
        }
        ulFailed = ValidateSerializedBuffer(pTI, pTI->pBuffer + pTI->DiskMets.uPageSize, pTI->DiskMets.uPageSize,
                                           ulPage, pTI->DiskMets.uPageSize, 0,
                                           testName, ulSerialNumber2);
        if(ulFailed)
        {
            DclPrintf("   Validate FAILED at block %U, offset %U\n", j + 1, ulFailed-1);
            DclHexDump(NULL, HEXDUMP_UINT8, 32, pTI->DiskMets.uPageSize, pTI->pBuffer + pTI->DiskMets.uPageSize);
            return __LINE__;
        }
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestBigBlocks()

    Write many blocks accross the media, read back verify
    each of them.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
-------------------------------------------------------------------*/
static D_INT16 TestBigBlocks(
    VBFTESTINFO    *pTI)
{
    D_INT16         i;
    D_UINT32        ulPage;
    D_UINT32        r;
    static char     testName[] = "TestBigBlocks";
    D_UINT32        ulFailed;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nStarting Multi-block Tests...\n");

    for(i = 0; i < 100; ++i)
    {
        D_UINT32       ulSerialNumber;

        DclPrintf("    Block %U of 100\n", i + 1);

        r = DclRand(&pTI->ulRandSeed);
        ulPage = (r % (pTI->DiskInfo.ulTotalPages - 2));

        ulSerialNumber =
            SerializeBuffer(pTI, pTI->pBuffer, pTI->ulTestBufferSize, ulPage, testName);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, pTI->ulTestBufferBlocks, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, pTI->ulTestBufferBlocks))
            return __LINE__;

        DclMemSet(pTI->pBuffer, 0xA5, pTI->ulTestBufferSize);

        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, pTI->ulTestBufferBlocks, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, pTI->ulTestBufferBlocks))
            return __LINE__;

        ulFailed = ValidateSerializedBuffer(pTI, pTI->pBuffer,
                                           pTI->ulTestBufferSize,
                                           ulPage,
                                           pTI->ulTestBufferSize,
                                           0, testName, ulSerialNumber);
        if(ulFailed)
        {
            DclPrintf("   Validate FAILED at block %U, offset %U\n", i + 1, ulFailed-1);
            return __LINE__;
        }
    }

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestUnitInfo()

    Write random data blocks to the media while monitoring
    unit information of the entire flash array.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
-------------------------------------------------------------------*/
#define UNIT_INFO_COUNT 1000
static D_INT16 TestUnitInfo(
    VBFTESTINFO    *pTI)
{
    D_UINT16        i;
    D_UINT32        uli;
    D_INT16         count;
    D_UINT32        ulNumBlocks;
    VBFUNITMETRICS  unitInfo;
    D_UINT32        ulSystemPages;
    D_UINT32        ulValidPages;
    D_UINT32        ulFreePages;
    D_UINT32        ulDiscardedPages;
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nStarting Unit Information Tracking...\n");

    ulNumBlocks = pTI->DiskInfo.ulTotalPages;

    /*  write and verify a whole bunch
    */
    for(count = 0; count < UNIT_INFO_COUNT; ++count)
    {
        /*  Get a random address to write to in the flash array
        */
        ulPage = (DclRand(&pTI->ulRandSeed) % (ulNumBlocks - 1));

        /*  Init to random, but reproducable pattern
        */
        for(i = 0; i < pTI->DiskMets.uPageSize; ++i)
            pTI->pBuffer[i] = (D_UCHAR) (ulPage | i);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
            return __LINE__;

        DclMemSet(pTI->pBuffer, 0xAA, pTI->DiskMets.uPageSize);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
            return __LINE__;

        for(i = 0; i < pTI->DiskMets.uPageSize; ++i)
            if(pTI->pBuffer[i] != (D_UCHAR) (ulPage | i))
                return __LINE__;

        /*  Start with zero totals
        */
        ulSystemPages = 0;
        ulValidPages = 0;
        ulFreePages = 0;
        ulDiscardedPages = 0;

        /*  Get info for each unit
        */
        for(uli = 0; uli < pTI->DiskMets.ulTotalUnits; ++uli)
        {
            unitInfo.lnu = uli;
            unitInfo.bValidVbfUnit = FALSE;
            if(FfxVbfUnitMetrics(pTI->hVBF, &unitInfo) != FFXSTAT_SUCCESS)
                return __LINE__;
            if(!unitInfo.bValidVbfUnit)
                return __LINE__;

            /*  Keep track of totals for the entire partition
            */
            ulSystemPages += unitInfo.ulSystemPages;
            ulValidPages += unitInfo.ulValidPages;
            ulFreePages += unitInfo.ulFreePages;
            ulDiscardedPages += unitInfo.ulDiscardedPages;
        }

        DclPrintf("Valid: %lU Discarded: %lU Free: %lU System: %lU    \n",
                  ulValidPages, ulDiscardedPages, ulFreePages, ulSystemPages);
    }

    return SUCCESS_RETURN_CODE;

}


/*-------------------------------------------------------------------
    Local: TestHugeBlocks()

    Write many blocks, and overwrite blocks, and read back verify
    each block.  This will exercise the typical fragmentation
    over long term use.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
-------------------------------------------------------------------*/
static D_INT16 TestHugeBlocks(
    VBFTESTINFO    *pTI)
{
    D_UINT32        i, j;
    D_UINT32        ulPage;
    D_UINT32        r;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nStarting Huge-block Tests...\n");

    if(pTI->DiskInfo.ulTotalPages <= (pTI->ulHugeBufferBlocks * 2L))
    {
        DclPrintf("    Media not large enough, skipping huge block tests.\n");
        return SUCCESS_RETURN_CODE;
    }

    for(i = 0; i < 30; ++i)
    {
        DclPrintf("    Block %U of 30\n", i + 1);

        r = DclRand(&pTI->ulRandSeed);
        ulPage = (r % (pTI->DiskInfo.ulTotalPages - (pTI->ulHugeBufferBlocks + 1)));

        for(j = 0; j < (pTI->ulHugeBufferBlocks * pTI->DiskInfo.uPageSize); ++j)
            pTI->pHugeBuff[j] = (D_UCHAR) (j + ulPage);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
            return __LINE__;

        DclMemSet(pTI->pHugeBuff, 0xA5, pTI->ulHugeBufferSize);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage + pTI->ulHugeBufferBlocks, 2, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, 2))
            return __LINE__;

        ioStat = FfxVbfReadPages(pTI->hVBF, (ulPage + 1),
                                 pTI->ulHugeBufferBlocks - 2,
                                 pTI->pHugeBuff + pTI->DiskMets.uPageSize);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks - 2))
            return __LINE__;

        for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
        {
            if(pTI->pHugeBuff[j] != 0xA5)
            {
                DclPrintf("    Mismatch at offset %U\n", j);
                return __LINE__;
            }
        }

        for(; j < ((pTI->ulHugeBufferBlocks - 1) * pTI->DiskMets.uPageSize); ++j)
        {
            if(pTI->pHugeBuff[j] != ((D_UCHAR) (j + ulPage)))
            {
                DclPrintf("    Mismatch at offset %U\n", j);
                return __LINE__;
            }
        }

        for(; j < pTI->ulHugeBufferSize; ++j)
        {
            if(pTI->pHugeBuff[j] != 0xA5)
            {
                DclPrintf("    Mismatch at offset %U\n", j);
                return __LINE__;
            }
        }

        ioStat = FfxVbfReadPages(pTI->hVBF, (ulPage + 2), pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
            return __LINE__;

        for(j = (pTI->DiskMets.uPageSize * 2); j < pTI->ulHugeBufferSize; ++j)
        {
            if(pTI->pHugeBuff[j - (pTI->DiskMets.uPageSize * 2)] != ((D_UCHAR)(j + ulPage)))
            {
                DclPrintf("    Mismatch at offset %U\n", j);
                return __LINE__;
            }
        }

        for(j = pTI->ulHugeBufferSize - (pTI->DiskInfo.uPageSize * 2); j < pTI->ulHugeBufferSize; ++j)
        {
            if(pTI->pHugeBuff[j] != 0xA5)
            {
                DclPrintf("    Mismatch at offset %U\n", j);
                return __LINE__;
            }
        }
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestHugeOperations()

    Performs huge operations.  Attempts to read/write/discard
    up to HUGE_UNIT_COUNT or HUGE_MIN_BYTES of data in a single
    request, which ever is greater.

    Depending upon the flash configuration and the available
    memory it may not be possible to allocate the memory
    required to run this test.  In this case, the test will
    report failure.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level, non-zero if any problems are encountered.
-------------------------------------------------------------------*/
#define HUGE_MIN_BYTES    (256 * 1024UL)
#define HUGE_UNIT_COUNT   (2)
static D_INT16 TestHugeOperations(
    VBFTESTINFO    *pTI)
{
    D_BUFFER       *pBuffer = NULL;
    D_UINT32        ulPage = 0;
    D_UINT32        ulPageCount;
    D_UINT32        ulUnitCount;
    D_UINT32        ulMaxPages;
    D_UINT32        ulTestBytes;
    D_UINT32        i, j;
    FFXIOSTATUS     ioStat;
    D_BUFFER        uTestData = 0xA0;
    D_INT16         iRet;

    DclPrintf("\nStarting Huge Operation Tests...\n");

    /*  Calculate the test conditions.

        First determine the number of pages (in even multiples of the
        unit client size) needed to reach the desired target size.
    */
    ulTestBytes = DCLMAX((unsigned long)HUGE_MIN_BYTES,
                         (unsigned long)(HUGE_UNIT_COUNT *
                                        pTI->DiskMets.uUnitClientPages *
                                        pTI->DiskInfo.uPageSize ) );
    ulUnitCount = 0;
    ulMaxPages = 0;
    while((ulMaxPages * pTI->DiskInfo.uPageSize) < ulTestBytes)
    {
        ulUnitCount++;
        ulMaxPages = ulUnitCount * pTI->DiskMets.uUnitClientPages;
    }

    /*  Allocate a buffer for the test.
    */
    pBuffer = DclMemAlloc(pTI->DiskInfo.uPageSize * ulMaxPages);
    if(!pBuffer)
    {
        DclPrintf("    Error: Unable to allocate page buffer of %lU bytes\n",
                  pTI->DiskInfo.uPageSize * ulMaxPages);
        iRet = __LINE__;
        goto HugeTestCleanup;
    }
    pTI->ulTestMemUsage += pTI->DiskInfo.uPageSize * ulMaxPages;

    if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Testing up to %lU pages (%lU units, %lU KB) starting at page %lU\n",
                  ulMaxPages, ulUnitCount,
                  (ulMaxPages * pTI->DiskInfo.uPageSize) / 1024, ulPage);
    }

    /*  Perform each test case.
    */
    for(i = 1; i <= ulUnitCount; i++)
    {
        ulPageCount = pTI->DiskMets.uUnitClientPages * i;
        ulTestBytes = ulPageCount * pTI->DiskInfo.uPageSize;
        if(pTI->nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("    Testing %lU pages\n", ulPageCount);

        /*  Set the buffer to a known state.
        */
        uTestData++;
        if(pTI->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("        Test Data = 0x%2X\n", uTestData);
        DclMemSet(pBuffer, uTestData, ulTestBytes);

        /*  Write pages.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Writing...\n");
        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, ulPageCount, pBuffer);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("        Writing at page %lU failed!\n", ulPage);
            iRet = __LINE__;
            goto HugeTestCleanup;
        }

        /*  Read pages.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Reading...\n");
        DclMemSet(pBuffer, ~uTestData, ulTestBytes);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, ulPageCount, pBuffer);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("        Reading from page %lU failed!\n", ulPage);
            iRet = __LINE__;
            goto HugeTestCleanup;
        }

        /*  Verify the data.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Verifying...\n");
        for(j = 0; j < ulTestBytes; j++)
        {
            if(pBuffer[j] != uTestData)
            {
                DclPrintf("        Data mismatch at offset %U, expected 0x%2X\n", j, uTestData);
                DclHexDump("        Buffer:\n", HEXDUMP_UINT8, 16, ulTestBytes, pBuffer);
                iRet = __LINE__;
                goto HugeTestCleanup;
            }
        }

        /*  Discard pages.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Discarding...\n");
        ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, ulPageCount);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("        Discarding at page %lU failed!\n", ulPage);
            iRet = __LINE__;
            goto HugeTestCleanup;
        }

        /*  Read pages.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Reading...\n");
        DclMemSet(pBuffer, ~VBF_FILL_VALUE, ulTestBytes);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, ulPageCount, pBuffer);
        if(!IOSUCCESS(ioStat, ulPageCount))
        {
            DclPrintf("        Reading from page %lU failed!\n", ulPage);
            iRet = __LINE__;
            goto HugeTestCleanup;
        }

        /*  Verify the data.
        */
        if(pTI->nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("        Verifying...\n");
        for(j = 0; j < ulTestBytes; j++)
        {
            if(pBuffer[j] != VBF_FILL_VALUE)
            {
                DclPrintf("        Data mismatch at offset %U, expected 0x%2X\n", j, VBF_FILL_VALUE);
                DclHexDump("        Buffer:\n", HEXDUMP_UINT8, 16, ulTestBytes, pBuffer);
                iRet = __LINE__;
                goto HugeTestCleanup;
            }
        }
    }

    DclPrintf("    PASSED\n");
    iRet = SUCCESS_RETURN_CODE;

  HugeTestCleanup:

    if(pBuffer)
        DclMemFree(pBuffer);

    return iRet;
}


/*-------------------------------------------------------------------
    Local: TestRegionBoundaries()

    Test writing and reading huge blocks which will include
    read/writes/discards that cross region boundaries.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_INT16 TestRegionBoundaries(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulPage;
    D_UINT32        ulEraseZoneBlocks;
    D_UINT16        uBlockSize;
    D_UINT32        ulBlocksForRegionTest;
    D_UINT32        ulBlockNumber;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulSerialNumber, ulOldSerialNumber = 0;
    D_UINT32        ulFailed;
    D_UINT32        kk;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nStarting Region Boundary Tests...\n");

    /*  Determine the region block size
    */
    uBlockSize = pTI->DiskMets.uPageSize;

    /*  Determine the number of blocks to use in the region test

        NOTE: Test at least three region boundaries
    */
    ulBlocksForRegionTest = (pTI->DiskMets.ulRegionClientPages * 3) + 1;

    /*  Determine the erase zone size and make sure that the
        region boundary test will also cross an erase zone
        boundary
    */
    FfxFmlDiskInfo(pTI->hFML, &FmlInfo);
    ulEraseZoneBlocks = FmlInfo.ulBlockSize / uBlockSize;
    if(ulEraseZoneBlocks > ulBlocksForRegionTest)
    {
        ulBlocksForRegionTest = ulEraseZoneBlocks + 1;
    }

    /*  Make sure that the media is large enough
    */
    if(pTI->DiskInfo.ulTotalPages < (ulBlocksForRegionTest + pTI->ulHugeBufferBlocks))
    {
        DclPrintf("    Media not large enough, skipping region boundary tests.\n");
        return SUCCESS_RETURN_CODE;
    }

    for(ulBlockNumber = 0; ulBlockNumber < ulBlocksForRegionTest; ulBlockNumber++)
    {
        if((ulBlockNumber == 0) || !((ulBlockNumber + 1) % 10))
            DclPrintf("    Block %lU of %lU\n", ulBlockNumber + 1, ulBlocksForRegionTest);

        /*  Determine the page to write to the flash
        */
        ulPage = ulBlocksForRegionTest - ulBlockNumber;

        /*  Set the information in the buffer for the write
        */
        ulSerialNumber = SerializeBuffer(pTI, pTI->pHugeBuff, pTI->ulHugeBufferSize, ulPage, "TestRegionBoundaries");

        /*  Perform the write
        */
        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
        {
            DclPrintf("    FfxVbfWritePages() FAILED at block %lU, %s\n",
                ulBlockNumber + 1, FfxDecodeIOStatus(&ioStat));
            return __LINE__;
        }

        /*  Read back the data
        */
        DclMemSet(pTI->pHugeBuff, 0, pTI->ulHugeBufferSize);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
        {
            DclPrintf("    FfxVbfReadPages() FAILED at block %lU, %s\n",
                ulBlockNumber + 1, FfxDecodeIOStatus(&ioStat));
            return __LINE__;
        }

        /*  Validate that the data read back is correct
        */
        ulFailed = ValidateSerializedBuffer(pTI, pTI->pHugeBuff,
                                           pTI->ulHugeBufferSize,
                                           ulPage,
                                           pTI->ulHugeBufferSize,
                                           0, "TestRegionBoundaries", ulSerialNumber);
        if(ulFailed)
        {
            DclPrintf("    Validate FAILED at block %lU, offset %lU\n", ulBlockNumber+1, ulFailed-1);
            return __LINE__;
        }

        /*  Determine if too much data was written to the media
        */
        if(ulBlockNumber != 0)
        {
            /*  Read the last block of the previous write
            */
            ioStat = FfxVbfReadPages(pTI->hVBF, ulPage + pTI->ulHugeBufferBlocks, 1, pTI->pHugeBuff);
            if(!IOSUCCESS(ioStat, 1))
            {
                DclPrintf("    Overwrite check FfxVbfReadPages() FAILED at block %lU, %s\n",
                     ulBlockNumber + 1, FfxDecodeIOStatus(&ioStat));
                return __LINE__;
            }

            /*  Validate that the data beyond our write is untouched.
            */
            ulFailed = ValidateSerializedBuffer(pTI, pTI->pHugeBuff, uBlockSize, ulPage+1,
                                               pTI->ulHugeBufferSize,
                                               pTI->ulHugeBufferBlocks - 1,
                                               "TestRegionBoundaries", ulOldSerialNumber);
            if(ulFailed)
            {
                DclPrintf("    Overwrite check FAILED at block %lU, offset %lU\n",
                          ulBlockNumber+1, ulFailed-1);
                return __LINE__;
            }
        }

        /*  Test discards for every other iteration through this loop
        */
        if(!(ulPage & 1))
        {
            /*  Discard everything else we wrote, except for the last block
            */
            ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks-1);
            if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks-1))
            {
                DclPrintf("    FfxVbfDiscardPages() FAILED at block %lU, %s\n",
                    ulBlockNumber + 1, FfxDecodeIOStatus(&ioStat));
                return __LINE__;
            }

            /*  Read and revalidate the entire chunk, ensuring that the area
                we discarded, really is marked as such.
            */
            ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
            if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
            {
                DclPrintf("    FfxVbfReadPages() after discard FAILED at block %lU, %s\n",
                    ulBlockNumber + 1, FfxDecodeIOStatus(&ioStat));
                return __LINE__;
            }

            /*  Validate that the data we discarded really is marked as such,
                and that the last block still is intact.
            */
            for(kk=0; kk<pTI->ulHugeBufferSize - uBlockSize; kk++)
            {
                if(pTI->pHugeBuff[kk] != VBF_FILL_VALUE)
                {
                    DclPrintf("    Validate after discard FAILED at block %lU, offset %lU\n", ulBlockNumber+1, kk);
                    return __LINE__;
                }
            }

            ulFailed = ValidateSerializedBuffer(pTI, pTI->pHugeBuff+kk, uBlockSize,
                                               ulPage, pTI->ulHugeBufferSize,
                                               pTI->ulHugeBufferBlocks - 1,
                                               "TestRegionBoundaries", ulSerialNumber);
            if(ulFailed)
            {
                DclPrintf("    Validate after discard FAILED at block %lU, offset %lU\n", ulBlockNumber+1, ulFailed+kk-1);
                return __LINE__;
            }
        }

        ulOldSerialNumber = ulSerialNumber;
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestExercise()

    Write random data blocks to the media.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
-------------------------------------------------------------------*/
#define EXERCISE_COUNT 1000
static D_INT16 TestExercise(
    VBFTESTINFO    *pTI)
{
    D_INT16         i;
    D_INT16         count;
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nStarting Random Exercise...\n");
    DclPrintf("    Current Seed = %lU\n", pTI->ulRandSeed);

    /*  write and verify a whole bunch
    */
    for(count = 0; count < EXERCISE_COUNT; ++count)
    {
        if((count == 0) || !((count + 1) % 10))
            DclPrintf("    Block %U of %U\n", count + 1, EXERCISE_COUNT);

        /*  Set up a cycle of three writes:  first, a random data sector,
            then two fat sectors (#1 and #2).  Repeat the cycle.  This
            will roughly simulate a true DOS partition.
        */
        if((count % 3) == 0)
            ulPage = (DclRand(&pTI->ulRandSeed) % (pTI->DiskInfo.ulTotalPages - 1));
        else
            ulPage = (count % 3);

        /*  Init to random, but reproducible pattern
        */
        for(i = 0; i < pTI->DiskMets.uPageSize; ++i)
            pTI->pBuffer[i] = (D_UCHAR) (ulPage | i);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfWritePages() FAILED at block %U\n", count + 1);
            return __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0xAA, pTI->DiskMets.uPageSize);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfReadPages() FAILED at block %U\n", count + 1);
            return __LINE__;
        }

        for(i = 0; i < pTI->DiskMets.uPageSize; ++i)
        {
            if(pTI->pBuffer[i] != (D_UCHAR) (ulPage | i))
            {
                DclPrintf("    verify FAILED at block %U, offset %U\n", count + 1, i);
                return __LINE__;
            }
        }
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestDiscard()

    Tests the functionality of the FfxVbfDiscardPages() API call.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 TestDiscard(
    VBFTESTINFO    *pTI)
{
    D_INT16         i, j;
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;
    D_INT16         uResult = SUCCESS_RETURN_CODE;

    DclPrintf("\nStarting Discard Test...\n");

    /*  First, discard 100 single pages from the volume.  
    */
    for(i = 0; i < 100; ++i)
    {
        DclPrintf("    Block %U of 100\n", i + 1);

        ulPage = (DclRand(&pTI->ulRandSeed) % (pTI->DiskInfo.ulTotalPages - 1));

        for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
            pTI->pBuffer[j] = (D_UCHAR) (ulPage % (j + 1));

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscard: FfxVbfWritePages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0xEC, pTI->DiskMets.uPageSize);
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscard: FfxVbfReadPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
        {
            if(pTI->pBuffer[j] != (D_UCHAR) (ulPage % (j + 1)))
            {
                DclPrintf("TestDiscard: buffer mismatch:%u: failed, at byte %s\n",
                    __LINE__, j );
                uResult = __LINE__;
                break;
            }
        }

        ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, 1);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscard: FfxVbfDiscardPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0xD9, pTI->DiskMets.uPageSize);

        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscard: FfxVbfReadPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
        {
            if(pTI->pBuffer[j] != (D_UCHAR) VBF_FILL_VALUE)
            {
                DclPrintf("TestDiscard: buffer mismatch:%u: failed, at byte %d\n",
                    __LINE__, j );
                uResult = __LINE__;
                break;
            }
        }

        if(!RemountVBF(pTI))
        {
            DclPrintf("TestDiscard: RemountVBF:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
    }

    /*  Now attempt to discard the same sector twice.
    */
    ulPage = (DclRand(&pTI->ulRandSeed) % (pTI->DiskInfo.ulTotalPages - 1));

    for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
        pTI->pBuffer[j] = (D_UCHAR) (ulPage % (j + 1));

    ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfWritePages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    DclMemSet(pTI->pBuffer, 0xEC, pTI->DiskMets.uPageSize);
    ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfReadPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
    {
        if(pTI->pBuffer[j] != (D_UCHAR) (ulPage % (j + 1)))
        {
            DclPrintf("TestDiscard: buffer mismatch:%u: failed, at byte %d\n",
                __LINE__, j );
            uResult = __LINE__;
            break;
        }
    }

    ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, 1);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfDiscardPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    DclMemSet(pTI->pBuffer, 0xD9, pTI->DiskMets.uPageSize);
    ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfDiscardPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
    {
        if(pTI->pBuffer[j] != (D_UCHAR) VBF_FILL_VALUE)
        {
            DclPrintf("TestDiscard: buffer mismatch:%u: failed, at byte %d\n",
                __LINE__, j );
            uResult = __LINE__;
            break;
        }
    }

    ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, 1);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfDiscardPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    DclMemSet(pTI->pBuffer, 0xEC, pTI->DiskMets.uPageSize);
    ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscard: FfxVbfReadPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    for(j = 0; j < pTI->DiskMets.uPageSize; ++j)
    {
        if(pTI->pBuffer[j] != (D_UCHAR) VBF_FILL_VALUE)
        {
            DclPrintf("TestDiscard: buffer mismatch:%u: failed, at byte %d\n",
                __LINE__, j );
            uResult = __LINE__;
            break;
        }
    }

    if ( uResult == SUCCESS_RETURN_CODE )
    {
        DclPrintf("    PASSED\n");
    }
    else
    {
        DclPrintf("    FAILED\n");
    }
    return uResult;
}

/*-------------------------------------------------------------------
    Local: TestDiscardSet()

    Test discard set processing.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_INT16 TestDiscardSet(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulRegionPages, ulStart;
    D_UINT16        uBlockSize, uUnitBlocks;
    D_UINT16        i;
    D_UINT32        ulUseableUnits, ulRegionUnits;
    int             j;
    FFXIOSTATUS     ioStat;
    D_INT16         uResult = SUCCESS_RETURN_CODE;
    
    DclPrintf("\nTesting Discard Set\n");

    /*  Determine the region size and block size
    */
    ulRegionPages = pTI->DiskMets.ulRegionClientPages;
    uBlockSize = pTI->DiskMets.uPageSize;
    uUnitBlocks = pTI->DiskMets.uUnitClientPages;

    /*  Must have two regions to perform this test.
    */
    ulUseableUnits = pTI->DiskMets.ulTotalUnits - pTI->DiskMets.uSpareUnits;
    ulRegionUnits = ulRegionPages / uUnitBlocks;
    if(ulUseableUnits <= ulRegionUnits)
    {
        DclPrintf("    Not enough regions. TestDiscardSet skipped.\n");
        return SUCCESS_RETURN_CODE;
    }
    if(ulRegionUnits < 3UL)
    {
        DclPrintf("    Not enough units. TestDiscardSet skipped.\n");
        return SUCCESS_RETURN_CODE;
    }

    /*  Discard the first region
    */
    DclPrintf("    Discarding...\n");
    ioStat = FfxVbfDiscardPages(pTI->hVBF, 0, ulRegionPages);
    if(!IOSUCCESS(ioStat, ulRegionPages))
    {
        DclPrintf("TestDiscardSet: FfxVbfDiscardPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    /*  Compact entire disk
    */
    DclPrintf("    Compacting...\n");
    do
    {
        ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
    } while(ioStat.ffxStat == FFXSTAT_SUCCESS);

    /*  Mount an arbitrary region and write enough to fill up two units in the
        region. Allow for space (that will be) taken up by metadata.

    */
    DclPrintf("    Writing...\n");
    for(i = 0; i < 2 * uUnitBlocks - 3; i++)
    {
        DclMemSet(pTI->pBuffer, (D_UCHAR) i, uBlockSize);
        ioStat = FfxVbfWritePages(pTI->hVBF, i, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfWritePages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
    }

    DclPrintf("    Verifying writes...\n");
    for(i = 0; i < 2 * uUnitBlocks - 3; i++)
    {
        ioStat = FfxVbfReadPages(pTI->hVBF, (D_UINT32)i, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfReadPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
        for(j = 0; j < uBlockSize; j++)
        {
            if(pTI->pBuffer[j] != (D_UCHAR) i)
            {
                DclPrintf("TestDiscardSet: buffer mismatch:%u: failed, at byte %d\n",
                    __LINE__, j );
                uResult = __LINE__;
                break;
            }
        }
    }

    /*  discard the second group of allocations in the arbitrary region
    */
    DclPrintf("    Discarding...\n");
    for(i = 0, ulStart = uUnitBlocks;
        i < uUnitBlocks - 3; i++, ulStart++)
    {
        ioStat = FfxVbfDiscardPages(pTI->hVBF, ulStart, 1);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfDiscardPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
    }

    /*  mount a different region
    */
    ioStat = FfxVbfReadPages(pTI->hVBF, ulRegionUnits + uUnitBlocks, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
        return __LINE__;

    /*  Remount the arbitrary region and make sure the blocks are still
        discarded after the mount.
    */
    DclPrintf("    Verifying explicit discards...\n");
    for(i = uUnitBlocks; i < 2 * uUnitBlocks - 3; i++)
    {

        ioStat = FfxVbfReadPages(pTI->hVBF, i, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfReadPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
        for(j = 0; j < uBlockSize; j++)
        {
            if(pTI->pBuffer[j] != (D_UCHAR) VBF_FILL_VALUE)
            {
                DclPrintf("TestDiscardSet: buffer mismatch:%u: failed, at byte %s\n",
                    __LINE__, j );
                uResult = __LINE__;
                break;
            }
        }
    }

    /*  Rewrite the second group of blocks in this region. These rewrites
        should  cause an implicit discard of the explicit discards recorded
        in the discard set.
    */
    DclPrintf("    Rewriting...\n");
    for(i = uUnitBlocks; i < 2 * uUnitBlocks - 3; i++)
    {
        DclMemSet(pTI->pBuffer, (D_UCHAR) i, uBlockSize);
        ioStat = FfxVbfWritePages(pTI->hVBF, i, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfWritePages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
    }

    /*  1st group = all good, 2nd group = all discarded,  3rd group = all
        good (replaces data that was in 2nd unit). Force a GC to write out
        a new metadata record with a higher temporal key. Because of the
        implicit replacement of the elements of the discard set, this
        new metadata should not contain any bit for the 3rd group of
        allocations.
    */
    ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("TestDiscardSet: FfxVbfCompact:%u: failed, result is %s\n",
		  __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    /*  mount a different region
    */
    ioStat = FfxVbfReadPages(pTI->hVBF, ulRegionUnits + uUnitBlocks, 1, pTI->pBuffer);
    if(!IOSUCCESS(ioStat, 1))
    {
        DclPrintf("TestDiscardSet: FfxVbfReadPages:%u: failed, result is %s\n",
            __LINE__, FfxDecodeIOStatus(&ioStat) );
        uResult = __LINE__;
    }

    /*  Remount the arbitrary region and make sure all the data is in
        the region that should be there.
    */
    DclPrintf("    Verifying implicit discards...\n");
    for(i = 0; i < 2 * uUnitBlocks - 3; i++)
    {
        ioStat = FfxVbfReadPages(pTI->hVBF, i, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestDiscardSet: FfxVbfReadPages:%u: failed, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }
        for(j = 0; j < uBlockSize; j++)
        {
            if(pTI->pBuffer[j] != (D_UCHAR) i)
            {
                DclPrintf("TestDiscardSet: buffer mismatch:%u: failed, at byte %d\n",
                    __LINE__, j );
                uResult = __LINE__;
                break;
            }
        }
    }

    return uResult;
}

/*-------------------------------------------------------------------
    Local: TestCompaction()

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 TestCompaction(
    VBFTESTINFO    *pTI)
{
    unsigned int    count;
    FFXIOSTATUS     ioStat;

    DclPrintf("\nPerforming Normal Compaction...\n");

    /*  Start by doing unaggressive compaction until it reports nothing left to
        do.  If this reaches an "unreasonable" count (guaranteed to fit in a
        Standard C unsigned int), give it up but don't report a failure yet --
        give it another chance with aggressive compaction.
    */
    for (count = 1; count < 65535; ++count)
    {
        if (count % 20 == 0)
            DclPrintf("    Iteration %u\n", count);

        ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_NORMAL);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
    }
    DclPrintf("    Total compactions: %u\n", count);

    if (!RemountVBF(pTI))
    {
        DclPrintf("VBF remount() FAILED\n");
        return __LINE__;
    }

    DclPrintf("    PASSED\n");

    /*  Now do aggressive compaction until it is all done, with the same
        limit on the number of iterations.  This is expected to complete.
        Yes, I know it's arbitrary, but reasonable nonetheless.
    */
    DclPrintf("\nPerforming Aggressive Compaction...\n");
    for (count = 1; count < 65535; ++count)
    {
        if (count % 20 == 0)
            DclPrintf("    Iteration %u\n", count);

        ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
    }
    if (count == 65535)
    {
        DclPrintf("Aggressive compaction FAILED\n");
        return __LINE__;
    }

    DclPrintf("    Total compactions: %u\n", count);

    if (!RemountVBF(pTI))
    {
        DclPrintf("VBF remount() FAILED\n");
        return __LINE__;
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: TestErrors()

       Ensure we can't do anything stupid like read past the end of the
       media. or write to non-existant data sectors.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Error level non-zero if any problems are encountered.
        Zero if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 TestErrors(
    VBFTESTINFO    *pTI)
{
    D_INT16         i;
    D_UINT32        ulPage;
    FFXIOSTATUS     ioStat;
    D_INT32         ulPages[3];
    D_INT16         uResult = SUCCESS_RETURN_CODE;
    D_BOOL          fResult;
    
    DclPrintf("\nStarting Error Tests...\n");

     /*  First test will attempt right past the end, the second pass to be
         ulTotalPages + 1 and the following pass a small distance past the
         boundary condition.
    */
    ulPage = (pTI->DiskInfo.ulTotalPages);
    ulPages[0] = ulPage;
    ulPages[1] = ulPage + 1;
    ulPages[2] = ulPage + 23;

    for(i = 0; i < 3; ++i)
    {
        ulPage = ulPages[i];
        DclPrintf("    Pass %U of 3, page count is %lu\n", i + 1, ulPage);

        DclMemSet(pTI->pBuffer, 0xA9, pTI->DiskMets.uPageSize);

        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestErrors: FfxVbfWritePages:%u: succeeded, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        fResult = RemountVBF(pTI);
        if( ! fResult )
        {
            DclPrintf("TestErrors: RemountVBF:%u: failed, result is %d\n",
                __LINE__, fResult );
            uResult = __LINE__;
        }

        ioStat = FfxVbfWritePages(pTI->hVBF, 1, ulPage, pTI->pBuffer);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestErrors: FfxVbfWritePages:%u: succeeded, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        fResult = RemountVBF(pTI);
        if( ! fResult )
        {
            DclPrintf("TestErrors: RemountVBF:%u: failed, result is %d\n",
                __LINE__, fResult );
            uResult = __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0xA8, pTI->DiskMets.uPageSize);

        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestErrors: FfxVbfReadPages:%u: succeeded, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        if(!RemountVBF(pTI))
        {
            DclPrintf("TestErrors: RemountVBF:%u: failed\n",
                __LINE__ );
            uResult = __LINE__;
        }

        ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, 1);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestErrors: FfxVbfDiscardPages:%u: succeeded, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        if(!RemountVBF(pTI))
        {
            DclPrintf("TestErrors: RemountVBF:%u: failed\n",
                __LINE__ );
            uResult = __LINE__;
        }

        ioStat = FfxVbfDiscardPages(pTI->hVBF, 1, ulPage);
        if(IOSUCCESS(ioStat, 1))
        {
            DclPrintf("TestErrors: FfxVbfDiscardPages:%u: succeeded, result is %s\n",
                __LINE__, FfxDecodeIOStatus(&ioStat) );
            uResult = __LINE__;
        }

        if(!RemountVBF(pTI))
        {
            DclPrintf("TestErrors: RemountVBF:%u: failed\n",
                __LINE__ );
            uResult = __LINE__;
        }
    }

    if ( uResult == SUCCESS_RETURN_CODE )
    {
        DclPrintf("    PASSED\n");
    }
    else
    {
        DclPrintf("    FAILED\n");
    }
    return uResult;
}

/*-------------------------------------------------------------------
    Local: TestWearLeveling()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static D_INT16 TestWearLeveling(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulStaticPages = 0;  /* # of pages written one time only */
    D_UINT32        ulDynamicPages;     /* # of pages which are continually rewritten */
    D_UINT32        ulMaxAtATime;       /* Max pages to write at one time */
    D_UINT32        ulPage;
    D_UINT16        uPageSize;          /* Page size in a local for convenience */
    FFXIOSTATUS     ioStat;
    DCLTIMESTAMP    ts;
    D_UINT64        ullTotalSeconds;
    D_UINT64        ullWriteCount;      /* Total writes (not counting initial static data creation) */
    D_UINT64        ullWritePageCount;  /* Total pages written */
    unsigned        kk;

    DclPrintf("\nTesting Wear-Leveling\n");

    uPageSize = pTI->DiskInfo.uPageSize;

    /*  The disk needs to start in a freshly formatted state...
    */
    if(!FormatPartition(pTI))
        return __LINE__;

    pTI->hVBF = FfxVbfCreate(pTI->hFML);
    if(!pTI->hVBF)
    {
        DclPrintf("VBF Create failed!\n");
        return __LINE__;
    }

    /* Retrieve the VfbDiskInfo
    */
    if(FfxVbfDiskInfo(pTI->hVBF, &pTI->DiskInfo) != FFXSTAT_SUCCESS)
    {
        DclPrintf("FfxVbfDiskInfo failed\n");
        return __LINE__;
    }

    if(FfxVbfDiskMetrics(pTI->hVBF, &pTI->DiskMets) != FFXSTAT_SUCCESS)
        return __LINE__;

    if(pTI->nWearLevStaticPercent)
    {
        D_UINT64    ullTemp;

        DclUint64AssignUint32(&ullTemp, pTI->DiskInfo.ulTotalPages);
        DclUint64MulUint32(&ullTemp, pTI->nWearLevStaticPercent);
        DclUint64DivUint32(&ullTemp, 100);
        ulStaticPages = DclUint32CastUint64(&ullTemp);

        DclProductionAssert(ulStaticPages);
        DclProductionAssert(ulStaticPages < pTI->DiskInfo.ulTotalPages);

        DclPrintf("  Filling %u%% of the disk (%lU pages) with static data...\n",
            pTI->nWearLevStaticPercent, ulStaticPages);

        /*  Fill a page buffer with random data
        */
        for(kk=0; kk<uPageSize; kk++)
        {
            pTI->pBuffer[kk] = (D_UINT8)DclRand(&pTI->ulRandSeed);
        }

        for(ulPage=0; ulPage<ulStaticPages; ulPage++)
        {
            /*  Write each page of random static data out.  They are all
                the same for now.
            */
            ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, 1, pTI->pBuffer);
            if(!IOSUCCESS(ioStat, 1))
            {
                DclPrintf("Error writing static data page %lU, %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat));

                return __LINE__;
            }
        }
    }

    ulDynamicPages = pTI->DiskInfo.ulTotalPages - ulStaticPages;
    ulMaxAtATime = DCLMIN3(ulDynamicPages, pTI->ulHugeBufferBlocks, WEARLEV_MAX_SEQWRITES);

    DclPrintf("  Writing random sized blocks of data, up to %lU pages at a time, to random\n", ulMaxAtATime);
    DclPrintf("  offsets within the dynamic portion of the disk.\n");

    if(pTI->ulWearLevMinutes)
        DclPrintf("  Test Duration:  %lU minutes\n", pTI->ulWearLevMinutes);
    else
        DclPrintf("  Test Duration:  Infinite (until a flash error occurs)\n");
    DclPrintf("  Sampling every %u minute(s)\n", WEARLEV_SAMPLE_MINUTES);

    DclUint64AssignUint32(&ullTotalSeconds, 0);
    DclUint64AssignUint32(&ullWriteCount, 0);
    DclUint64AssignUint32(&ullWritePageCount, 0);
    ts = DclTimeStamp();
    while(TRUE)
    {
        D_UINT32    ulCount;
        D_UINT32    ulElapsedMS;
        D_UINT32    ulSample = 0;

        /*  Pick a random number of pages to write, up to the max allowable
        */
        ulCount = (DclRand(&pTI->ulRandSeed) % ulMaxAtATime) + 1;

        /*  Pick a random starting page
        */
        ulPage = (DclRand(&pTI->ulRandSeed) % (ulDynamicPages - ulCount)) + ulStaticPages;

        /*  Better not go past the end of the disk
        */
        DclProductionAssert(ulPage + ulCount <= pTI->DiskInfo.ulTotalPages);

        /*  Fill a page buffer with random data
        */
        for(kk=0; kk<uPageSize; kk++)
        {
            pTI->pBuffer[kk] = (D_UINT8)DclRand(&pTI->ulRandSeed);
        }

        /*  Copy it into each of the pages we are going to write
        */
        for(kk=0; kk<ulCount; kk++)
        {
            DclMemCpy(pTI->pHugeBuff+(kk*uPageSize), pTI->pBuffer, uPageSize);
        }

        /*  Write out all the pages for this iteration
        */
        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, ulCount, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("Error writing dynamic page %lU for %lU pages, %s\n",
                ulPage, ulCount, FfxDecodeIOStatus(&ioStat));

            return __LINE__;
        }

        /*  Increment the write counters
        */
        DclUint64AddUint32(&ullWriteCount, 1);
        DclUint64AddUint32(&ullWritePageCount, ulCount);

        ulElapsedMS = DclTimePassed(ts);
        if(ulElapsedMS >= WEARLEV_SAMPLE_MINUTES * 60 * 1000)
        {
            char        szTime[16];
            FFXSTATUS   ffxStat;
            D_UINT32    ulMinErases;
            D_UINT32    ulMaxErases;
            D_UINT32    ulAvgErases;

            ulSample++;

            DclUint64AddUint32(&ullTotalSeconds, (ulElapsedMS+500) / 1000);

            ffxStat = GatherEraseCounts(pTI, &ulMinErases, &ulMaxErases, &ulAvgErases, FALSE);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                DclPrintf("Error %lX gathering erase counts!\n", ffxStat);
                return __LINE__;
            }

            DclPrintf("      Sample %lU, Elapsed time %s, performed %llU writes of %llU pages.  Min/Max/Avg erases = %lU/%lU/%lU\n",
                ulSample, DclTimeFormat(ullTotalSeconds, szTime, sizeof(szTime)),
                VA64BUG(ullWriteCount), VA64BUG(ullWritePageCount), ulMinErases, ulMaxErases, ulAvgErases);

            if(pTI->ulWearLevMinutes)
            {
                D_UINT64    ullMinutes = ullTotalSeconds;

                DclUint64DivUint32(&ullMinutes, 60);

                if(DclUint64GreaterUint32(&ullMinutes, pTI->ulWearLevMinutes-1))
                {
                    DclPrintf("      Test time period expired (%lU minutes)\n", pTI->ulWearLevMinutes);
                    GatherEraseCounts(pTI, NULL, NULL, NULL, TRUE);
                    break;
                }
            }

            /*  Reset the timestamp base to start counting up until the
                next sample.
            */
            ts = DclTimeStamp();
        }
    }

    return 0;
}


/*-------------------------------------------------------------------
    Local: Benchmark()

    This function performs benchmark tests.  The philosophy for
    these tests is that they should show "best case" performance
    numbers, so that the results will more readily be comparable
    to FMSL and FSIO performance test results.  To ensure this,
    the following measures are taken:

    1) The tests are run at the beginning of VBFTEST, after the
       flash has been freshly formatted.

    2) Measures are taken to "pre-mount" regions, so that region
       mount time does not factor into the read/write/discard
       test results.  Region mount time is separately tested.

    3) The amount of data written is bounded by the number of
       client pages contained in the pre-mounted regions.

    4) The amount of data read is bounded by the number of data
       pages written in the write portion of the test.  The
       reads will wrap and re-read pages if necessary to ensure
       that the test runs long enough relative to the timer
       resolution to provide a reasonable result.

    5) The amount of data discarded is bounded by the number of
       data pages written in the write portion of the test.  This
       cannot treat this like reads, and wrap, since once the
       data has been discarded, discarding it again would not
       provide accurate results.  (This may need future work.)

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        None.
-------------------------------------------------------------------*/
static void Benchmark(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulWriteKBPerSecond;
    D_UINT32        ulReadKBPerSecond;
    D_UINT32        ulDiscardKBPerSecond;
    D_UINT32        ulCompactionMS;
    D_UINT32        ulDiskMountMS;
    D_UINT32        ulRegionMountMS;
    D_UINT32        ulMaxPages;

    DclPrintf("Testing VBF performance\n");

    ulMaxPages = PremountRegions(pTI);
    if(!ulMaxPages)
    {
        DclPrintf("    Error prep'ing the perf test\n");
        return;
    }

    if(ulMaxPages < pTI->ulHugeBufferBlocks)
    {
        DclPrintf("    Media too small to do performance testing\n");
        return;
    }

    ulWriteKBPerSecond = GetWritePerform(pTI, &ulMaxPages);
    DclPrintf("    Write:       %10lU KB per second\n", ulWriteKBPerSecond);

    /*  The write test may have reduced the ulMaxPages value, however don't
        allow it to be reduced so small that the read/discard tests will
        break (nevertheless, warn about it).
    */
    if(ulMaxPages < pTI->ulHugeBufferBlocks)
    {
        DclPrintf("NOTE: The read and discard tests may be inaccurate\n");
        ulMaxPages = pTI->ulHugeBufferBlocks;
    }

    ulReadKBPerSecond = GetReadPerform(pTI, ulMaxPages);
    DclPrintf("    Read:        %10lU KB per second\n", ulReadKBPerSecond);

    ulDiscardKBPerSecond = GetDiscardPerform(pTI, ulMaxPages);
    DclPrintf("    Discard:     %10lU KB per second\n", ulDiscardKBPerSecond);

    ulCompactionMS = GetCompactionPerf(pTI);
    DclPrintf("    Compaction:  %10lU ms per compaction\n", ulCompactionMS);

    ulDiskMountMS = GetDiskMountPerf(pTI);
    DclPrintf("    Disk Mount:  %10lU ms per mount\n", ulDiskMountMS);

    /*  Note that this function is different than the rest of the perf
        functions because it must call test functionality which resides
        in the VBF core, whereas the other tests are using standard
        interfaces.
    */
    ulRegionMountMS = FfxVbfTestRegionMountPerf(pTI->hVBF);
    DclPrintf("    Region Mount:%10lU ms per mount\n", ulRegionMountMS);

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perfvbf.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->fPerfLog, NULL, "VBF", "Performance", NULL, pTI->szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "Write KB/s",     ulWriteKBPerSecond);
        DCLPERFLOG_NUM(   hPerfLog, "Read KB/s",      ulReadKBPerSecond);
        DCLPERFLOG_NUM(   hPerfLog, "Discard KB/s",   ulDiscardKBPerSecond);
        DCLPERFLOG_NUM(   hPerfLog, "MS/Compaction",  ulCompactionMS);
        DCLPERFLOG_NUM(   hPerfLog, "MS/DiskMount",   ulDiskMountMS);
        DCLPERFLOG_NUM(   hPerfLog, "MS/RegionMount", ulRegionMountMS);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ResourceUsage()

    This function displays resource usage.  It is called at the
    end of the test cycle to ensure that a wide variety of
    operations have been performed.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ResourceUsage(
    VBFTESTINFO    *pTI)
{
    FFXCONFIGINFO   fci;

    DclMemSet(&fci, 0, sizeof(fci));

    fci.uStrucLen = sizeof fci;
    fci.MemStats.uStrucLen = sizeof(fci.MemStats);

    DclPrintf("\n");

    /*  We are using this FlashFX interface to retrieve the memory
        stats rather than calling the DCL MemStats function directly
        because this function is handled by the external API while
        the DCL function is not (currently).
    */
    if((FfxConfigurationInfo(&fci, FALSE, FALSE) == FFXSTAT_SUCCESS) &&
        (DclMemStatsDisplay(&fci.MemStats) == DCLSTAT_SUCCESS))
    {
        /*---------------------------------------------------------
            Write data to the performance log, if enabled.  Note
            that any changes to the test name or category must be
            accompanied by changes to perfvbf.bat.  Any changes to
            the actual data fields recorded here requires changes
            to the various spreadsheets which track this data.
        ---------------------------------------------------------*/
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->fPerfLog, NULL, "VBF", "MemoryUsage", NULL, pTI->szPerfLogSuffix);
        DCLPERFLOG_NUM(  hPerfLog, "Overhead",      fci.MemStats.uOverhead);
        DCLPERFLOG_NUM(  hPerfLog, "CurrentBytes",  fci.MemStats.ulBytesCurrentlyAllocated);
        DCLPERFLOG_NUM(  hPerfLog, "CurrentBlocks", fci.MemStats.ulBlocksCurrentlyAllocated);
        DCLPERFLOG_NUM(  hPerfLog, "MaxConcBytes",  fci.MemStats.ulMaxConcurrentBytes);
        DCLPERFLOG_NUM(  hPerfLog, "MaxConcBlocks", fci.MemStats.ulMaxConcurrentBlocks);
        DCLPERFLOG_NUM(  hPerfLog, "TotalAllocs",   fci.MemStats.ulTotalAllocCalls);
        DCLPERFLOG_NUM(  hPerfLog, "LargestAlloc",  fci.MemStats.ulLargestAllocation);
        DCLPERFLOG_NUM(  hPerfLog, "TestOverhead",  pTI->ulTestMemUsage);
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }
    else
    {
        DclPrintf("Memory usage statistics are disabled\n");
    }

    return;
}


/*-------------------------------------------------------------------
    Local: PremountRegions()

    This test prepares VBF for performance testing by ensuring
    that we have as many regions mounted at the beginning of
    the flash as possible, based on the region cache size.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns the maximum number of pages, starting with page
        zero, which can be accessed in mounted regions.  Returns
        zero on failure.
-------------------------------------------------------------------*/
static D_UINT32 PremountRegions(
    VBFTESTINFO    *pTI)
{
    unsigned        nReg;
    unsigned        nRegCount;

    /*  Calculate the maximum number of regions we should try to mount
    */
    nRegCount = pTI->DiskMets.nRegionsCached;

  #ifdef VBFCONF_STATICREGION
    /*  It is possible that the static region (if any) is NOT in the
        range of regions we are going to try to mount.  If this is
        the case, reduce the total number of regions by one since the
        static region will not be included in the total page count we
        are returning.
    */
    if(nRegCount <= VBFCONF_STATICREGION)
        nRegCount--;
  #endif

    DclProductionAssert(nRegCount);

    for(nReg=0; nReg<nRegCount; nReg++)
    {
        FFXIOSTATUS     ioStat;

        /*  Read the first page of the given region.  This will cause
            the region to be mounted.  Makes no difference whether
            the data in that page was ever written.
        */
        ioStat = FfxVbfReadPages(pTI->hVBF, nReg * pTI->DiskMets.ulRegionClientPages, 1, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("  PremountRegions:FfxVbfReadPages() failed with %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }
    }

    return DCLMIN(pTI->DiskMets.ulClientPages, nRegCount * pTI->DiskMets.ulRegionClientPages);
}


/*-------------------------------------------------------------------
    Local: GetWritePerform()

    This function determines the speed at which we write data.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        This function will return the kilobytes per second that were
        written by the FfxVbfWritePages() routine.
-------------------------------------------------------------------*/
static D_UINT32 GetWritePerform(
    VBFTESTINFO    *pTI,
    D_UINT32       *pulMaxPages)
{
    D_UCHAR         cValue = 0xD5;
    D_UINT32        ulDiffTime;
    D_UINT32        ulNumWrites = 0;
    D_UINT32        ulPage = 0;
    DCLTIMER        tWrite;

    DclProductionAssert(pulMaxPages);
    DclProductionAssert(*pulMaxPages >= pTI->ulHugeBufferBlocks);
    DclProductionAssert(NUM_MILLISECONDS >= MSEC_PER_SEC);

    DclTimerSet(&tWrite, NUM_MILLISECONDS);
    do
    {
        FFXIOSTATUS     ioStat;

        DclMemSetAligned(pTI->pHugeBuff, cValue, pTI->ulHugeBufferSize);

        /*  Write a group of pages
        */
        ioStat = FfxVbfWritePages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
        {
            DclPrintf("  FfxVbfWritePages() failed with %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }

        /*  Increment the count of the number of writes we have done
            successfully.
        */
        ulNumWrites++;

        /*  Change the fill value of the buffer
        */
        cValue = (D_UCHAR)ulNumWrites | 0x10;

        /*  Increase the page we will write in the media
        */
        ulPage += pTI->ulHugeBufferBlocks;

        /*  We don't want to write past the end of the client data area
            and we don't want to wrap around since that will cause
            compactions -- quit early if necessary.
        */
        if(ulPage > (*pulMaxPages - pTI->ulHugeBufferBlocks))
            break;

        /*  Check the timer. Are we out of time?
        */
    }
    while(!DclTimerExpired(&tWrite));

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tWrite);

    FFXPRINTF(1, ("Write Performance: %lU writes of %lU pages, %lU ms\n",
                  ulNumWrites, pTI->ulHugeBufferBlocks, ulDiffTime));

    /*  Return the actual number of pages written, so that the subsequent
        read and discard tests will operate on only pages where data has
        actually been written.
    */
    *pulMaxPages = ulNumWrites * pTI->ulHugeBufferBlocks;

    /*  In case the flash is really, really fast, return obvious nonsense
        instead of dividing by zero.
    */
    if (ulDiffTime == 0)
        return 0;

    /*  Return the number of kilobytes we wrote each second.  Note that the
        scaling (dividing by 8) is to avoid truncation while also avoiding
        overflow.
    */
    return (((pTI->ulHugeBufferSize / (1024 / 8)) * ulNumWrites * (MSEC_PER_SEC / 8)) / ulDiffTime);
}


/*-------------------------------------------------------------------
    Local: GetReadPerform()

    This function determines the speed at which we read data.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        This function will return the kilobytes per second that were
        read by the FfxVbfReadPages routine.
-------------------------------------------------------------------*/
static D_UINT32 GetReadPerform(
    VBFTESTINFO    *pTI,
    D_UINT32        ulMaxPages)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulNumReads = 0;
    DCLTIMER        tRead;
    D_UINT32        ulPage = 0;

    DclProductionAssert(ulMaxPages >= pTI->ulHugeBufferBlocks);
    DclProductionAssert(NUM_MILLISECONDS >= MSEC_PER_SEC);

    DclTimerSet(&tRead, NUM_MILLISECONDS);
    do
    {
        FFXIOSTATUS     ioStat;

        /*  Read a group of pages
        */
        ioStat = FfxVbfReadPages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks, pTI->pHugeBuff);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
        {
            DclPrintf("  FfxVbfReadPages() failed with %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }

        /*  Increment the count of the number of reads we have done
            successfully.
        */
        ulNumReads++;

        /*  Index to the next group of pages
        */
        ulPage += pTI->ulHugeBufferBlocks;

        /*  We don't want to read past the end of the data which was
            written during the write portion of the performance test
            -- wrap and re-read data if necessary.
        */
        if(ulPage > (ulMaxPages - pTI->ulHugeBufferBlocks))
            ulPage = 0;

        /*  Check the timer.  Are we out of time?
        */
    }
    while(!DclTimerExpired(&tRead));

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tRead);

    FFXPRINTF(1, ("Read Performance: %lU reads of %lU pages, %lU ms\n",
                  ulNumReads, pTI->ulHugeBufferBlocks, ulDiffTime));

    /*  In case the flash is really, really fast, return obvious nonsense
        instead of dividing by zero.
    */
    if (ulDiffTime == 0)
        return 0;

    /*  Return the number of kilobytes we read each second.  Note that the
        scaling (dividing by 8) is to avoid truncation while also avoiding
        overflow.
    */
    return (((pTI->ulHugeBufferSize / (1024 / 8)) * ulNumReads * (MSEC_PER_SEC / 8)) / ulDiffTime);
}


/*-------------------------------------------------------------------
    Local: GetDiscardPerform()

    This function will determine VBF's speed at discarding data.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        This function will return the kilobytes per second that were
        discarded by the discard routine.
-------------------------------------------------------------------*/
static D_UINT32 GetDiscardPerform(
    VBFTESTINFO    *pTI,
    D_UINT32        ulMaxPages)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulNumDiscards = 0;
    D_UINT32        ulPage = 0;
    DCLTIMER        tDiscard;

    DclProductionAssert(ulMaxPages >= pTI->ulHugeBufferBlocks);
    DclProductionAssert(NUM_MILLISECONDS >= MSEC_PER_SEC);

    DclTimerSet(&tDiscard, NUM_MILLISECONDS);
    do
    {
        FFXIOSTATUS     ioStat;

        /*  Discard a group of pages at one time (as many as we were
            writing and reading at a time).
        */
        ioStat = FfxVbfDiscardPages(pTI->hVBF, ulPage, pTI->ulHugeBufferBlocks);
        if(!IOSUCCESS(ioStat, pTI->ulHugeBufferBlocks))
        {
            DclPrintf("  FfxVbfDiscardPages() failed with %s\n", FfxDecodeIOStatus(&ioStat));
            return 0L;
        }

        /*  Increment the count of the number of discards we have done
            successfully.
        */
        ulNumDiscards++;

        /*  Increment the page we are discarding.
        */
        ulPage += pTI->ulHugeBufferBlocks;

        /*  We don't want to discard past the end of the data which was
            written during the write performance test -- quit early if
            necessary.
        */
        if(ulPage > (ulMaxPages - pTI->ulHugeBufferBlocks))
            break;

        /*  Check the timer. Are we out of time?
        */
    }
    while(!DclTimerExpired(&tDiscard));

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tDiscard);

    FFXPRINTF(1, ("Discard Performance: %lU discards of %lU pages, %lU ms\n",
                  ulNumDiscards, pTI->ulHugeBufferBlocks, ulDiffTime));

    /*  In case the flash is really, really fast, return obvious nonsense
        instead of dividing by zero.
    */
    if (ulDiffTime == 0)
        return 0;

    /*  Return the number of kilobytes we discarded each second.  Note
        that the scaling (dividing by 8) is to avoid truncation while
        also avoiding overflow.  Note also that this calculation will
        fail if the block size is the theoretical minimum of 64 bytes
        (unlikely).
    */
    return (((pTI->ulHugeBufferSize / (1024 / 8)) * ulNumDiscards * (MSEC_PER_SEC / 8)) / ulDiffTime);
}


/*-------------------------------------------------------------------
    Local: GetCompactionPerf()

    This function determines the speed at which VBF compacts.

    Parameters:
        pTI            - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns the average number of milliseconds to perform a
        compaction.  Returns 0 to indicate failure.
-------------------------------------------------------------------*/
static D_UINT32 GetCompactionPerf(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulComps = 0;
    DCLTIMER        t;
    FFXIOSTATUS     ioStat;

    /*  Initialize the timer for a certain number of milliseconds.  Double
        the standard amount of time for compactions because they may be
        very slow (at least on NOR).
    */
    DclTimerSet(&t, NUM_MILLISECONDS*2);
    do
    {
        ioStat = FfxVbfCompact(pTI->hVBF, FFX_COMPACTFLAGS_EVERYTHING);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Increment the count of the number of compactions we have done
            successfully.
        */
        ulComps++;

        /*  Have we completed the requested time interval?
        */
    }
    while(!DclTimerExpired(&t));

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&t);

    FFXPRINTF(2, ("Compaction performance: %lU compactions in %lU ms\n", ulComps, ulDiffTime));

    if(!ulComps)
        return 0;
    else
        return ulDiffTime / ulComps;
}


/*-------------------------------------------------------------------
    Local: GetDiskMountPerf()

    This function determines the speed at which VBF mounts Disks.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns the number of milliseconds to mount the partition.
        Returns 0 to indicate failure.
-------------------------------------------------------------------*/
static D_UINT32 GetDiskMountPerf(
    VBFTESTINFO    *pTI)
{
    D_UINT32        ulDiffTime;
    D_UINT32        ulNumDiskMounts = 0;
    DCLTIMER        tMount;

    /*  Initialize the timer for a certain number of milliseconds.
    */
    DclTimerSet(&tMount, NUM_MILLISECONDS);
    do
    {
        if(!RemountVBF(pTI))
        {
            /*  Darned well better not ever fail...
            */
            DclProductionError();
            break;
        }

        /*  Increment the count of the number of partition mounts
            we have done successfully.
        */
        ulNumDiskMounts++;

        /*  Have we completed the requested time interval?
        */
    }
    while(!DclTimerExpired(&tMount));

    /*  Get actual elapsed time in milliseconds.
    */
    ulDiffTime = DclTimerElapsed(&tMount);

    FFXPRINTF(2, ("Disk Mount Performance: %lU mounts in %lU ms\n", ulNumDiskMounts, ulDiffTime));

    if(!ulNumDiskMounts)
        return 0;
    else
        return ulDiffTime / ulNumDiskMounts;
}


/*-------------------------------------------------------------------
    Local: TestFillDisk()

    Fill the media with data, read the entire media back,
    perform several random data updates and discards.  This
    test is run when the "/E" command-line option is used.

    Parameters:
        pTI - A pointer to the VBFTESTINFO structure

    Return Value:
        Returns zero if successful, or a line number otherwise.
-------------------------------------------------------------------*/
static D_INT16 TestFillDisk(
    VBFTESTINFO    *pTI)
{
    #define         BLOCK_PATTERN_LENGTH (31) /* prime, non power-of-two */
    const char      szFormatString[] = "%08lX %08lX %05U VBFTST ";
    /*                                 "12345678 12345678 12345 VBFTST "
    */
    D_UINT16        u;
    D_UINT32        ul;
    D_UINT32        ulNumBlocks;
    D_UINT16        uMod = 0;
    FFXIOSTATUS     ioStat;
    unsigned        ii;
    DCLTIMER        tRead;
    D_UINT32        ulTestMax, ulCounter, ulVerifyMax;

    DclPrintf("\nStarting Disk Fill...\n");

    ulNumBlocks = pTI->DiskInfo.ulTotalPages;

    if(ulNumBlocks > 100)
        uMod = 10;
    if(ulNumBlocks > 1000)
        uMod = 100;
    if(ulNumBlocks > 10000)
        uMod = 1000;

    if( pTI->ulExtensiveMinutes )
    {
        /*  Set countdown timer in milliseconds
        */
        DclTimerSet(&tRead, pTI->ulExtensiveMinutes*60*1000);
    }

    ulTestMax = ulNumBlocks;
    if( ( ulTestMax % 2 ) == 1 )
    {
        /*  For an odd number of blocks, ulTestMax--; the
            last (even) block tested will be the exact midpoint
        */
        ulTestMax--;
    }

    DclPrintf("  Pass #1, Write Entire Media\n");

    /*  Fill disk up once
    */
    for(ulCounter = 0; ulCounter < ulNumBlocks; ulCounter++)
    {
        if( pTI->ulExtensiveMinutes )
        {
            if( DclTimerExpired( &tRead ))
            {
                DclPrintf("  (Terminated early - timed out.\n");
                break;
            }
        }

        if((ulCounter == 0) || (uMod && !((ulCounter + 1) % uMod)))
            DclPrintf("    Block %lU of %lU\n", ulCounter + 1, ulNumBlocks);

        /*  Now determine exactly which block we are to test
        */
        if( ( ulCounter % 2 ) == 1 )
        {
            ul = ulTestMax - ulCounter;
        }
        else
        {
            ul = ulCounter;
        }


        for(u = 0; u < pTI->DiskMets.uPageSize; u += BLOCK_PATTERN_LENGTH)
        {
            DclSNPrintf((char*)&pTI->pPatBuff[u], pTI->DiskMets.uPageSize - u,
                szFormatString, ul, ul, u);
        }

        ioStat = FfxVbfWritePages(pTI->hVBF, ul, 1, pTI->pPatBuff);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfWritePages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        ioStat = FfxVbfReadPages(pTI->hVBF, ul, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfReadPages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        /*  NOTE: This won't function right until Bug 1218 is fixed
            and flash errors are properly passed through VBF.
        */
        if(ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
            DclPrintf("    AllocBlock %lU contains a corrected data error\n", ul + 1);

        for(u = 0; u < pTI->DiskMets.uPageSize; u++)
        {
            if(pTI->pBuffer[u] != pTI->pPatBuff[u])
            {
                DclPrintf("    verify FAILED at block %lU, offset %U\n", ul + 1, u);
                return __LINE__;
            }
        }
    }

    /*  Only read and verify pages written above. If we exited early,
        we can use ulCounter to bound this loop
    */
    DclPrintf("\n  Pass #2, Read, Verify Entire Media\n");

    /*  Read it back and verify it
    */
    ulVerifyMax = ulCounter;
    for(ulCounter = 0; ulCounter < ulVerifyMax; ulCounter++)
    {
        /*  Now determine exactly which block we are to test
        */
        if( ( ulCounter % 2 ) == 1 )
        {
            ul = ulTestMax - ulCounter;
        }
        else
        {
            ul = ulCounter;
        }

        if((ul == 0) || (uMod && !((ul + 1) % uMod)))
            DclPrintf("    Block %lU of %lU\n", ul + 1, ulNumBlocks);

        for(u = 0; u < pTI->DiskMets.uPageSize; u += BLOCK_PATTERN_LENGTH)
        {
            DclSNPrintf((char*)&pTI->pPatBuff[u], pTI->DiskMets.uPageSize - u,
                szFormatString, ul, ul, u);
        }

        ioStat = FfxVbfReadPages(pTI->hVBF, ul, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfReadPages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        /*  NOTE: This won't function right until Bug 1218 is fixed
            and flash errors are properly passed through VBF.
        */
        if(ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
            DclPrintf("    AllocBlock %lU contains a corrected data error\n", ul + 1);

        for(u = 0; u < pTI->DiskMets.uPageSize; u++)
        {
            if(pTI->pBuffer[u] != pTI->pPatBuff[u])
            {
                DclPrintf("    Verify FAILED at block %lU, offset %U\n", ul + 1, u);
                DclHexDump(NULL, HEXDUMP_UINT8, 32, pTI->DiskMets.uPageSize, pTI->pBuffer);

                DclPrintf("Expected:\n");
                DclHexDump(NULL, HEXDUMP_UINT8, 32, pTI->DiskMets.uPageSize, pTI->pPatBuff);

                return __LINE__;
            }
        }
    }

    DclPrintf("\n  Pass #3, Random Discards\n");

    /*  It is FULL, now update some sectors
    */
    for(ul = 0; ul < 100; ul++)
    {
        DclPrintf("    Discard %lU of 100\n", ul + 1);

        /*  Init to reproduceable pattern
        */
        DclMemSet(pTI->pBuffer, 0x0, pTI->DiskMets.uPageSize * 2);

        ioStat = FfxVbfWritePages(pTI->hVBF, ul, 2, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 2))
            return __LINE__;

        ioStat = FfxVbfDiscardPages(pTI->hVBF, ul, 2);
        if(!IOSUCCESS(ioStat, 2))
        {
            DclPrintf("    FfxVbfDiscardPages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0x5A, pTI->DiskMets.uPageSize * 2);

        ioStat = FfxVbfReadPages(pTI->hVBF, ul, 2, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 2))
            return __LINE__;

        for(u = 0; u < pTI->DiskMets.uPageSize * 2; ++u)
        {
            if(pTI->pBuffer[u] != (D_UCHAR) VBF_FILL_VALUE)
                return __LINE__;
        }
    }

    DclPrintf("\n  Pass #4, Random Updates\n");

    /*  It is FULL, now update some sectors
    */
    for(ii = 0; ii < 100; ii++)
    {
        DclPrintf("    Block %lU of 100\n", ii + 1);

        ul = (DclRand(&pTI->ulRandSeed) % (ulNumBlocks - 1));

        for(u = 0; u < pTI->DiskMets.uPageSize * 2; u += BLOCK_PATTERN_LENGTH)
        {
            DclSNPrintf((char*)&pTI->pPatBuff[u], (pTI->DiskMets.uPageSize * 2) - u,
                szFormatString, ul, ii, u);
        }

        ioStat = FfxVbfWritePages(pTI->hVBF, ul, 2, pTI->pPatBuff);
        if(!IOSUCCESS(ioStat, 2))
            return __LINE__;

        DclMemSet(pTI->pBuffer, 0x5A, pTI->DiskMets.uPageSize * 2);

        ioStat = FfxVbfReadPages(pTI->hVBF, ul, 2, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 2))
            return __LINE__;

        for(u = 0; u < pTI->DiskMets.uPageSize * 2; u++)
        {
            if(pTI->pBuffer[u] != pTI->pPatBuff[u])
                return __LINE__;
        }
    }

    /*  Discard entire disk
    */
    DclPrintf("\n  Pass #5, Discard Entire Media\n");
    for(ul = 0; ul < ulNumBlocks; ul++)
    {
        if((ul == 0) || (uMod && !((ul + 1) % uMod)))
            DclPrintf("    Discard %lU of %lU\n", ul + 1, ulNumBlocks);

        /*  Init to reproduceable pattern
        */
        DclMemSet(pTI->pBuffer, 0x0, pTI->DiskMets.uPageSize);

        ioStat = FfxVbfWritePages(pTI->hVBF, ul, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfWritePages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        ioStat = FfxVbfDiscardPages(pTI->hVBF, ul, 1);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfDiscardPages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        DclMemSet(pTI->pBuffer, 0xD9, pTI->DiskMets.uPageSize);
        ioStat = FfxVbfReadPages(pTI->hVBF, ul, 1, pTI->pBuffer);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("    FfxVbfReadPages() FAILED at block %lU\n", ul + 1);
            return __LINE__;
        }

        for(u = 0; u < pTI->DiskMets.uPageSize; ++u)
        {
            if(pTI->pBuffer[u] != (D_UCHAR) VBF_FILL_VALUE)
            {
                DclPrintf("    verify FAILED at block %lU, offset %U\n", ul + 1, u);
                return __LINE__;
            }
        }
    }

    DclPrintf("    PASSED\n");

    return SUCCESS_RETURN_CODE;
}


/*-------------------------------------------------------------------
    Local: SerializeBuffer()

    Fills a buffer with a test pattern to be written to the
    media.  The pattern changes in every block (as determined
    from the partition's reported block size), and on every
    call to SerializeBuffer() (even if all of the parameters
    are the same).  It does not repeat on any multiple of a
    power of two less than the maximum possible VBF block size.

    Parameters:
        pTI          - A pointer to the VBFTESTINFO structure
        pBuffer      - The buffer to be filled with a pattern.
        ulBufferSize - Length in bytes of pattern to generate.
        ulPage       - The page offset of where the buffer will be
                       written to the media.
        pszTestName  - Name of the test that wrote the data.

    Return Value:
        A serial number to be passed to ValidateSerializedBuffer()
        as its ulSerialNum parameter to recreate the pattern.
-------------------------------------------------------------------*/
static D_UINT32 SerializeBuffer(
    VBFTESTINFO    *pTI,
    D_UCHAR        *pBuffer,
    D_UINT32        ulBufferSize,
    D_UINT32        ulPage,
    const char     *pszTestName)
{
    D_UINT32        ulSizeLeft = ulBufferSize;
    D_UINT32        ulBlockNumber;
    static D_UINT32 ulSerialNum = 0;

    /*  Generate a new serial number.  This value is written in the fill
        pattern, and returned for use when verifying data later.
    */
    ++ulSerialNum;

    /*  Loop for each block (or portion thereof) to fill.
    */
    ulBlockNumber = 0;
    while(ulSizeLeft != 0)
    {
        D_UINT32    ulBlockSize = DCLMIN(ulSizeLeft, pTI->DiskMets.uPageSize);
        D_UINT32    ulFillLength;
        char        pattern[SERIALIZED_DATA_LENGTH];

        /*  Get the fill pattern for this block.
        */
        FillPattern(pattern, ulPage, ulBufferSize, ulBlockNumber, pszTestName, ulSerialNum);

        /*  Copy the pattern into the buffer repeatedly, truncating the last
            repetition appropriately.
        */
        ulFillLength = ulBlockSize;
        while(ulFillLength != 0)
        {
            D_UINT32 ulCopyLength = DCLMIN(ulFillLength, SERIALIZED_DATA_LENGTH);

            DclMemCpy(pBuffer, pattern, ulCopyLength);
            pBuffer += ulCopyLength;
            ulFillLength -= ulCopyLength;
        }

        ulSizeLeft -= ulBlockSize;
        ++ulBlockNumber;
    }

    return ulSerialNum;
}


/*-------------------------------------------------------------------
    Local: ValidateSerializedBuffer()

    Regenerates the pattern from the Parameters: and compares it
    to the buffer.

    Parameters:
        pTI           - A pointer to the VBFTESTINFO structure
        pBuffer       - The buffer containing data to validate
        ulBufferSize  - Length in bytes of data to validate
        ulPage        - The page offset of the write that wrote the
                        data to the media.
        ulWriteSize   - The length in bytes of the write.
        ulBlockNumber - Offset (in units of allocation blocks) of
                        the data in the original write.
        pszTestName   - Name of the test that wrote the data.
        ulSerialNum   - Value returned by SerializeBuffer when the
                        data pattern was generated.

    Return Value:
        Returns zero if successful, else a non-zero byte number where
        the mismatch occurred.
-------------------------------------------------------------------*/
static D_UINT32 ValidateSerializedBuffer(
    VBFTESTINFO    *pTI,
    D_UCHAR        *pBuffer,
    D_UINT32        ulBufferSize,
    D_UINT32        ulPage,
    D_UINT32        ulWriteSize,
    D_UINT32        ulBlockNumber,
    const char     *pszTestName,
    D_UINT32        ulSerialNum)
{
    D_UINT32        ulSizeLeft = ulBufferSize;
    D_UINT32        ulByteCount = 1;

    /*  Loop for each block (or portion thereof) to validate.
    */
    while(ulSizeLeft)
    {
        unsigned    nBlockSize = (unsigned)DCLMIN(ulSizeLeft, pTI->DiskMets.uPageSize);
        unsigned    nCompareLength;
        char        pattern[SERIALIZED_DATA_LENGTH];

        /*  Get the fill pattern for this block.
        */
        FillPattern(pattern, ulPage, ulWriteSize, ulBlockNumber, pszTestName, ulSerialNum);

        /*  Compare the pattern to the buffer repeatedly, as many complete
            copies as fit.
        */
        nCompareLength = nBlockSize;
        while(nCompareLength)
        {
            unsigned  nThisLength = DCLMIN(nCompareLength, SERIALIZED_DATA_LENGTH);
            unsigned  nLen = nThisLength;
            unsigned  nCount = 0;

            while(nLen)
            {
                /*  Compare the byte and return an non-zero byte
                    count if it does not match.
                */
                if(*pBuffer != pattern[nCount])
                {
                    DclPrintf("Mismatch at offset %u in pattern (pattern length is %u)\n", nCount, nThisLength);
                    DclHexDump("Expected:\n", HEXDUMP_NOOFFSET|HEXDUMP_UINT8, 32, nThisLength, pattern);
                    DclHexDump("Got:     \n", HEXDUMP_NOOFFSET|HEXDUMP_UINT8, 32, nThisLength, pBuffer-nCount);

                    return ulByteCount;
                }

                pBuffer++;
                nLen--;
                nCount++;
                ulByteCount++;
            }

            nCompareLength -= nThisLength;
        }

        ulSizeLeft -= nBlockSize;
        ++ulBlockNumber;
    }

    /*  Indicate success by returning zero
    */
    return 0;
}


/*-------------------------------------------------------------------
    Local: FillPattern()

    Generate a fill pattern that will be repeated to fill one
    block of a buffer to be written and later checked.  The
    pattern encodes a serial number (changed for each write), the
    staring address and total length of the write, the number of
    the block within the write to which this pattern will be
    written, and the name of the test that will write it.  The
    pattern's length is smaller than the largest allowable VBF
    block size, and is not a power of two, so it will never
    repeat on a block boundary.

    Parameters:
        pattern       - Where to store SERIALIZED_DATA_LENGTH bytes
                        of fill pattern.
        ulPage        - The starting page of the write operation
                        to be checked.
        ulWriteSize   - Length in bytes of data that will be written.
        ulBlockNumber - Which block of the write this pattern will
                        be written in.
        pszTestName   - Name of the test that will write the data.
        ulSerialNum   - Serial number for this call to FillPattern().

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FillPattern(
    char           *pattern,
    D_UINT32        ulPage,
    D_UINT32        ulWriteSize,
    D_UINT32        ulBlockNumber,
    const char     *pszTestName,
    D_UINT32        ulSerialNum)
{
    unsigned int    length;

    /*  Generate the fill pattern.  Note that the serial number is 32
        bits, so will not exceed ten digits, and the block number is 16
        bits and will not exceed five digits.  Plenty of room is left
        over for digit separators appropriate to the locale.
    */
    DclSNPrintf(pattern, SERIALIZED_DATA_LENGTH, "%24s %10lU %08lX %04X %U",
        pszTestName, ulSerialNum, ulPage, ulWriteSize, ulBlockNumber);

    length = DclStrLen(pattern);

    DclProductionAssert(length < SERIALIZED_DATA_LENGTH);

    DclMemSet(pattern + length, '.', SERIALIZED_DATA_LENGTH - length);

    return;
}


/*-------------------------------------------------------------------
    Local: GatherEraseCounts()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static FFXSTATUS GatherEraseCounts(
    VBFTESTINFO    *pTI,
    D_UINT32       *pulMinErases,
    D_UINT32       *pulMaxErases,
    D_UINT32       *pulAvgErases,
    unsigned        fDisplay)
{
    D_UINT32        ulUnit;
    D_UINT32        ulValidUnits = 0;
    D_UINT32        ulMinEraseCount = D_UINT32_MAX;
    D_UINT32        ulMaxEraseCount = 0;
    D_UINT64        ullTotalErases;
    FFXSTATUS       ffxStat;

    DclProductionAssert(pTI);

    DclUint64AssignUint32(&ullTotalErases, 0);

    if(fDisplay)
        DclPrintf("      Erase Summary:  Unit  EraseCount\n");

    /*  Iterate through the units and calculate summary information.
    */
    for(ulUnit = 0; ulUnit < pTI->DiskMets.ulTotalUnits; ulUnit++)
    {
        VBFUNITMETRICS  uInfo;

        uInfo.lnu = ulUnit;
        ffxStat = FfxVbfUnitMetrics(pTI->hVBF, &uInfo);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

        if(uInfo.bValidVbfUnit)
        {
            ulValidUnits++;

            ulMaxEraseCount = DCLMAX(ulMaxEraseCount, uInfo.ulEraseCount);
            ulMinEraseCount = DCLMIN(ulMinEraseCount, uInfo.ulEraseCount);

            DclUint64AddUint32(&ullTotalErases, uInfo.ulEraseCount);

            if(fDisplay)
                DclPrintf("                    %6lU  %10lU\n", ulUnit, uInfo.ulEraseCount);
        }
    }

    DclProductionAssert(ulValidUnits);

    if(pulAvgErases)
    {
        DclUint64DivUint32(&ullTotalErases, ulValidUnits);
        *pulAvgErases = DclUint32CastUint64(&ullTotalErases);
    }

    if(pulMinErases)
        *pulMinErases = ulMinEraseCount;

    if(pulMaxErases)
        *pulMaxErases = ulMaxEraseCount;

    return FFXSTAT_SUCCESS;
}


#endif  /* FFXCONF_VBFSUPPORT */

