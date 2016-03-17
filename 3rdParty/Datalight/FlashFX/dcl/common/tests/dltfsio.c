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

    This module contains tests for general file system operations.  It will
    run on any standard disk that can be accessed using standard C I/O
    operations.

    ToDo:  Should investigate using DclTimeStamp/Passed() exclusively, and
           not using the DclTimer* services.  This is because the former
           will use the high-res ticker, if available, while the latter will
           always use the standard ticker, even if the high-res ticker is
           implemented and enabled.

           Refactor this module into multiple modules.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltfsio.c $
    Revision 1.82  2011/04/16 16:03:12Z  garyp
    Fixed more stats to work on super-large volumes.
    Revision 1.81  2011/04/14 20:02:27Z  garyp
    Corrected the file system information display to work properly with very
    large volumes.
    Revision 1.80  2011/04/05 21:46:19Z  qa
    Updated to build cleanly with some picky compilers.
    Revision 1.79  2011/03/29 12:35:19Z  garyp
    Corrected the FILL test so the mid-sized files work properly.
    Revision 1.78  2011/03/29 03:32:15Z  garyp
    Corrected the /FILL test so the data pattern at the tail end of the "big"
    files matches properly.
    Revision 1.77  2011/03/27 02:07:07Z  garyp
    Updated the /FILL test to allow specifying a minimum file size.  Updated
    the test in general to keep a "flush file" which can be used to force
    a flush at any given point.
    Revision 1.76  2011/03/24 06:32:18Z  garyp
    Added debug code.
    Revision 1.75  2011/03/24 06:03:03Z  garyp
    Fixed to build with various and sundry arrogant compilers which whine
    about anything they cannot understand.
    Revision 1.74  2011/03/24 02:32:34Z  garyp
    Added some code lost in the merge.  Fixed the PrimWrite() and PRIMWRITE()
    stuff to preserve the original status code where possible.  Updated the
    /TREE test to not consider "DiskFull" to be a critical error.
    Revision 1.73  2011/03/23 23:44:07Z  garyp
    Internal test version 3.0.1.  Significant update to fix the /FILL test to
    work properly and be more comprehensive.  Added the /SPARSE test.
    Updated to actively track and display remaining free disk space as the
    various tests execute.  Fixed many of the FS primitive calls (PRIM*) to
    preserve the original status codes.
    Revision 1.72  2011/02/09 00:33:58Z  garyp
    Fixed a bug which could occur on small disks with large FS block sizes, such
    that the I/O buffer is the same length as 1 FS block, which would cause a 
    buffer overflow in the unaligned sequential I/O test.
    Revision 1.71  2010/08/04 00:12:18Z  garyp
    Updated to work around the "VA64BUG" issue.  Updated so the random
    and sequential unaligned tests can be disabled.  Added sampling to
    the random unaligned test.
    Revision 1.70  2010/07/18 01:30:43Z  garyp
    Removed some test code which was accidentally checked in.
    Revision 1.69  2010/07/15 22:12:28Z  garyp
    Correct the "short" sleep handling.  Updated the wear-leveling test
    to not rely on precise free space calculations.
    Revision 1.68  2010/06/02 02:50:16Z  garyp
    Updated the freespace calculation in the TREE test.
    Revision 1.67  2010/05/24 13:55:26Z  garyp
    Changed some stats to use 64-bit counters.
    Revision 1.66  2010/05/19 19:30:48Z  garyp
    Removed an unused variable.
    Revision 1.65  2010/05/19 16:40:55Z  garyp
    v3.0.  Major update to perform a wider range of testing and to provide
    results in a more user-friendly fashion.  Allow running in emulation mode
    to allow regression testing (v2.0 emulation performs the same I/O that
    has been used since rev 1.52).  Added a Tree test.  Added tests for
    unaligned I/O.  No longer sleep by default.  Do more regular flushes.
    Revision 1.64  2009/11/02 18:40:37Z  garyp
    Updated to terminate immediately if the DCL instance handle is not
    initialized.  Fixed to free the stats buffer, if any.  Updated to use the
    shared test documentation, where appropriate.
    Revision 1.63  2009/10/19 19:58:02Z  garyp
    Updated to support a 64-bit random seed.
    Revision 1.62  2009/09/17 21:52:08Z  garyp
    Moved the definition of DCLOUTPUT_BUFFERLEN to dltshared.c.
    Revision 1.61  2009/09/11 16:57:00Z  glenns
    Fix compilation issues with gcc.
    Revision 1.60  2009/09/08 22:50:39Z  garyp
    Moved functionality which is shared by multiple tests into the module
    dltshared.c.  Renamed the /T parameter to /TIME.  Otherwise the test
    is functionally unchanged.
    Revision 1.59  2009/06/24 22:51:12Z  garyp
    Updated to accommodate functions which now take DCL Instance handles.
    Revision 1.58  2009/03/16 20:26:25Z  keithg
    Fix for bug 2513, potentially unused variable in ScanTestMeasureCreate()
    Revision 1.57  2009/03/04 21:45:09Z  billr
    Resolve bug 2460: Code assumes 'DCLTIMESTAMP' is a integral type.
    Revision 1.56  2009/02/18 20:24:27Z  brandont
    Updated the PressEnter function to behave properly if the
    DCL_OSFEATURE_CONSOLEINPUT is not set to TRUE.
    Revision 1.55  2009/02/17 06:03:32Z  keithg
    Added explicit void of unused formal parameters.
    Revision 1.54  2009/02/13 22:38:10Z  jimmb
    Unnecessary initialization to satisfy a neurotic compiler
    Revision 1.53  2009/02/11 03:19:44Z  garyp
    Tweaked the FRAG test to try to avoid a situation where there is not enough
    free disk space to complete the test.  Updated the requestor open logic
    to try to open a block device handle through a file system request if there
    is no block device name specfied in the statfs data.
    Revision 1.52  2009/02/08 02:20:27Z  garyp
    Merged from the v4.0 branch.  Changed the RANDOM flush-on-write ratio
    to 128.  Added the ability to start and stop tracing around each test.
    Updated the sequential I/O tests to do microsecond based timing.  Fixed the
    sequential write and rewrite tests to do one last flush before timing is
    stopped.  Reduced the number of iterations that the random test runs so the
    test runs faster.  Tweaked the "small" file size used by the frag test so
    disk space issues are more predictable.  Updated so separate requestor
    handles can be opened to the BlockDev and the FileSys.  Updated the profiler
    commands so that it can separately talk to the BlockDev or  FileSys.  Fixed
    a file naming mechanism which was causing the names to not be deterministic.
    Updated the RAND test to process a fixed amount of data to allow the test to
    be more deterministic.  Added summary information to the FRAG test.  Merged
    in the new SCAN tests from the trunk.  Fixed a parameter parsing error that
    prevented the /DIRS switch from working.  Improved error handling in the
    FRAG test.  Shortened the PERFLOG output.  Cleaned up some messages.
    Updated to use the new DclInputChar() function.
    Revision 1.51  2009/01/26 23:18:50Z  keithg
    Added explicit void of unused formal parameter(s).
    Revision 1.50  2008/10/30 17:25:40  brandont
    Added the scan test category.
    Revision 1.49  2008/07/23 20:46:21Z  keithg
    Added type casts to placate older MSVS compilers.
    Revision 1.48  2008/06/03 20:45:51Z  billr
    --- Added comments ---  billr [2008/06/03 20:50:20Z]
    Fix a couple uninitialized variables. Check for possible infinite loop on
    large buffer.
    Revision 1.47  2008/06/02 22:11:58Z  garyp
    Removed an unused variable, and properly conditioned some code to
    avoid warnings from some compilers.
    Revision 1.46  2008/06/01 21:20:37Z  garyp
    Merged from the WinMobile branch.
    Revision 1.45.2.3  2008/05/30 23:55:23Z  garyp
    Fixed a minor message problem.
    Revision 1.45.2.2  2008/05/29 19:09:16Z  garyp
    Major update to the /FRAG test to spread the files across a set of
    directories when filling the disk for better performance.  Changed the
    "tiny" file size for that test to be the device block size.  Added the
    /stats command-line option and modified so the stats interfaces are only
    called if that is used.  Added the test "UnstructuredRandomWrites()" (not
    documented).  Updated to send profiler commands through the generic IOCTL
    interface.
    Revision 1.45  2008/05/14 17:11:24Z  garyp
    Minor messaging fixes.
    Revision 1.44  2008/05/07 03:23:12Z  garyp
    Documentation update.
    Revision 1.43  2008/05/01 21:26:57Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.42  2008/04/07 22:16:08Z  brandont
    Updated to use the new DCL file system services.
    Revision 1.41  2008/04/03 23:31:09Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.40  2008/03/22 18:33:00Z  Garyp
    Minor data type changes.
    Revision 1.39  2008/03/14 01:27:38Z  Garyp
    Documentation updated.
    Revision 1.38  2008/03/12 00:41:22Z  brandont
    Corrected uninitialized variable warnings.
    Revision 1.37  2008/01/08 23:48:07Z  brandont
    Corrected integral size mismatch warnings.
    Revision 1.36  2008/01/02 19:49:15Z  Garyp
    Updated the test help.  Adjusted the verbosity of the /CREATE test messages.
    Revision 1.35  2007/12/27 02:11:47Z  billr
    Fix compiler warning about possibly uninitialized variable.
    Revision 1.34  2007/12/14 21:03:04Z  Garyp
    Added a wear-leveling test, to be used in conjunction with a low level stats
    interface (typically).  Added the /FSBlockSize and /FSDevName command-line
    parameters.  Modified the stats interface to iterate through sub-types.
    Revision 1.33  2007/11/03 23:31:12Z  Garyp
    Added the standard module header.
    Revision 1.32  2007/10/03 20:51:21Z  Garyp
    Added a typecast to avoid a compiler warning.
    Revision 1.31  2007/08/29 20:22:44Z  peterb
    Fixed code that caused FSIOtest to always think that /Start was enabled.
    Revision 1.30  2007/08/06 20:23:10Z  garyp
    Revamped the /START option to allow more flexibility in the way the range
    is specified.  Updated the PerfLog stuff to work when /START is used.
    Revision 1.29  2007/07/02 18:21:08Z  Garyp
    Modified the PerfLog stuff to use the new DCLPERFLOG_WRITE() functionality
    as well as handle the new "CategorySuffix" parameter.  Use the new generic
    I/O requestor interface to dump out stats for each test run.  Fixed a bug
    which would cause an endless loop for some tests if the I/O block size was
    64KB or larger.
    Revision 1.28  2007/06/16 23:07:48Z  Garyp
    Modified to allow most test categories to be explicitly disabled by placing
    a trailing '-' after the test name.
    Revision 1.27  2007/05/07 18:26:42Z  Garyp
    Modified the /FRAG test to operate in a subdirectory so that any root
    directory file count limits are avoided.
    Revision 1.26  2007/04/25 21:26:30Z  Garyp
    Improved error handling in the /FRAG test.
    Revision 1.25  2007/04/15 01:06:53Z  Garyp
    Eliminated the possibility of getting into an endless loop if a buffer size
    larger than 256KB is used.
    Revision 1.24  2007/04/05 20:07:47Z  Garyp
    Updated to build cleanly with the RVDS tools.
    Revision 1.23  2007/04/03 19:13:54Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.22  2007/02/22 05:50:40Z  Garyp
    Added the "short" and "long" options to the /S parameter.  Updated the
    /FRAG test to run faster and run as part of the /DLPERF test set.
    Revision 1.21  2007/02/11 00:40:30Z  Garyp
    Updated the /DIR test to open, seek, read (1 byte), and close each file.
    Revision 1.20  2007/02/10 21:59:11Z  Garyp
    Updated so that the /FRAG test is fully implemented and usable (not made
    a part of the standard /DLPERF test set yet, but probably will be).
    Generalized the /RANDDIV switch since it is used by multiple tests.  Added
    primitives and stats gathering for directory operations.  Fixed the /CREATE
    test so it should function even with the new random data generation stuff
    (untested yet...).  Fixed so the random seeds progress consistently from
    run to run if initialized to the same value at startup.
    Revision 1.19  2007/01/17 00:47:26Z  Garyp
    Fixed so the DIR test is enabled as a part of DLPERF.
    Revision 1.18  2007/01/16 18:47:10Z  Garyp
    Cleaned up the "DIR" test and updated to provide PerfLog formatted output.
    Added "DIR" to the "DLPERF" list of tests.
    Revision 1.17  2007/01/15 03:47:14Z  Garyp
    Updated to dump memory usage in PerfLog format.
    Revision 1.16  2007/01/02 04:01:53Z  Garyp
    Added the /PERFLOG option, and instrumented the sequential and random I/O
    tests to support this feature.
    Revision 1.15  2006/12/15 19:48:13Z  Garyp
    Major update.  Removed the /ALL switch and added the /DLPERF option.
    Updated the random I/O tests to flush on every Nth write, as well as to
    operation on "SmallBlockSize" boundaries, reading and writing blocks of
    that size as well.  This will affect performance comparisons with prior
    versions.  Cleaned up the output to be more readable.
    Revision 1.14  2006/11/23 21:30:18Z  Garyp
    Updated so that all parameters which take size values will accept the values
    as hex or decimal numbers and 'B', 'KB', and 'MB' suffixes can be used.
    As before, all numbers default to KB if no suffix is specified (this is a
    behavioral change for /START:n which used to default to bytes).
    Revision 1.13  2006/11/17 23:23:51Z  Garyp
    Updated the /SEQ and /RAND tests so the individual read and write
    portions of the test can be selectively run.  Added flushes to ensure
    everything really is on the disk.
    Revision 1.12  2006/11/03 03:57:32Z  Garyp
    Added the /DIR test.  Modified the /V option to take a verbosity level.
    Revision 1.11  2006/10/08 22:10:42Z  Garyp
    Updated to build cleanly with the RealView tools.
    Revision 1.10  2006/07/07 03:00:33Z  Garyp
    Updated FSIOTEST so that the /FBR option Jeremy recently added is the
    default behavior, which may be disabled by using the /VERIFY option
    (renamed /VW).  Fixed some DclSNPrintf() formatting codes.  General
    documentation cleanup.
    Revision 1.9  2006/06/20 21:54:36Z  Garyp
    Fixed to avoid a divide by zero error.
    Revision 1.8  2006/06/08 20:24:27Z  Pauli
    Fixed notation of 32-bit variables and several type conversion warnings.
    Updated to use the DclRand function instead of srand/rand.
    Revision 1.7  2006/06/06 20:57:14Z  jeremys
    Added a random seed and random divisor parameter.  Removed the verbose
    condition from the additional random test output.
    Revision 1.6  2006/06/03 03:12:47Z  Garyp
    Modified the random tests to use file and block sizes that are 1/8th of the
    specified sizes.  Commented out the "random extend" test.  Added an
    extended test desciption to the help.
    Revision 1.5  2006/06/02 22:36:56Z  jeremys
    Added code to track seek time, additional output for the random read and
    write tests, conditional write-verify, and an option to fill buffers with
    random data.
    Revision 1.4  2006/05/06 23:32:54Z  Garyp
    Minor prototype changes.
    Revision 1.3  2005/12/30 15:27:44Z  Garyp
    Modified so the /PROF switch is accepted at run-time, but ignored, if the
    profiler is not enabled.
    Revision 1.2  2005/12/10 16:58:37Z  garyp
    Fixed to properly disable the profiler rather than just displaying stats
    and continuing on.
    Revision 1.1  2005/10/22 03:10:46Z  Pauli
    Initial revision
    Revision 1.5  2005/10/22 04:10:46Z  garyp
    Tweaked some debugging messages.
    Revision 1.4  2005/10/20 03:21:03Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.3  2005/10/11 07:21:59Z  Garyp
    Added support for integrated profiling.  Changed the /FRAG test copy
    count from 20 to 5.
    Revision 1.2  2005/10/07 03:07:15Z  Garyp
    Modified to no longer use standard C file I/O, but rather use the DCL
    abstraction.
    Revision 1.1  2005/05/03 20:54:52Z  Garyp
    Initial revision
    Revision 1.4  2005/09/21 08:44:36Z  garyp
    Updated to build cleanly in DCL.
    Revision 1.3  2005/09/20 19:01:10Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.2  2005/08/03 19:30:40Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/06 05:14:54Z  pauli
    Initial revision
    Revision 1.105  2005/04/13 19:06:02Z  Rickc
    Fixed compile warnings in ce
    Revision 1.104  2005/04/13 06:26:24  garyp
    Fixed compiler warnings.
    Revision 1.103  2005/04/13 03:51:16Z  garyp
    Modified the /FRAG test to copy a big file multiple times rather than simply
    create one multiple times.
    Revision 1.102  2005/04/13 01:20:17Z  garyp
    Added the /FRAG test.
    Revision 1.101  2005/02/10 03:01:57Z  GaryP
    Added the /Start:n functionality to allow the /SEQ test to iterate through
    a range of block sizes.
    Revision 1.100  2005/02/04 01:58:55Z  GaryP
    Finished implementing the /NoDel option.  Added the /Misalign option.
    Cleaned up some error messages.
    Revision 1.99  2005/01/04 23:39:46Z  GaryP
    Added tbe /NoDel and /FOW options.  Renamed /SUSTAINED to /SEQ and
    /RANDOM to /RAND.  Renamed /PERF to /CREATE and removed the read/write
    tests from this class as they are essentially less robust sequential I/O
    tests.  Improved the KB/Sec calculations to reduce overflow and rounding
    errors.  Changed to only call fflush() if the /FOW option is used.
    Revision 1.98  2004/12/30 17:32:44Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.97  2004/11/30 18:38:23Z  GaryP
    Updated the syntax information.
    Revision 1.96  2004/10/25 03:20:24Z  GaryP
    Major update.  Revamped command line options.  Includes cumulative
    read and write performance for the entire test run.  Cleaned up the output
    for readability.  Modified to be able to specify a maximum amount of disk
    space to use for all tests.
    Revision 1.95  2004/09/29 06:49:53Z  GaryP
    Major update to include sleep periods.  Added options to more easily allow
    automated execution.
    Revision 1.94  2004/09/10 22:22:33Z  GaryP
    Changed some data types to mollify the MSVC6 compiler.
    Revision 1.93  2004/08/22 23:10:22Z  GaryP
    Fixed a number of problems so we can build and run properly under DOS.
    Revision 1.92  2004/07/20 01:28:25Z  GaryP
    Minor include file updates.
    Revision 1.91  2004/07/01 23:28:42Z  jaredw
    added initializations to please VxWorks Compiler
    Revision 1.8  2004/05/05 20:51:11Z  billr
    Thread-safe timers based on system clock tick.
    Revision 1.7  2004/02/08 00:38:48Z  garys
    Merge from FlashFXMT
    Revision 1.5.1.5  2004/02/08 00:38:48  garyp
    Updated to work under DOS.
    Revision 1.5.1.4  2003/12/06 07:24:36Z  garyp
    Reduced the performance test disk space requirements.
    Revision 1.5.1.3  2003/11/30 04:32:38Z  garyp
    Major cleanup.  Parameters changed.  Added tests to measure sustained
    read/write/rewrite performance.
    Revision 1.5.1.2  2003/11/18 23:19:12Z  billr
    Merged from trunk build 744.
    Revision 1.6  2003/11/13 18:29:01Z  billr
    Add /Q switch (quiet: no progress messages, just results).
    Revision 1.5  2001/09/10 20:43:12Z  garys
    Changed several variables in SetSwitches() to unsigned int.  This solves
    some compiler warnings under CE.
    Revision 1.4  2003/04/22 00:14:37  garys
    fopen requires a near pointer, so pszFileName is now cast to near
    ptr when calling fopen().
    Revision 1.3  2003/04/21 18:56:58  garyp
    Added the /T option.
    Revision 1.2  2003/04/15 20:07:34Z  garyp
    Updated to use _sysfiledelete.
    Revision 1.1  2003/03/24 06:15:30Z  garyp
    Initial revision
    07/18/01 DE  Changed signon to use common copyright/patent/... text
    08/18/00 HDS Changed 4 references from __LINUX__ to __LINUX_TARGET__.
    07/12/00 mrm Added fsync() after write to flush (linux) buffer cache.
                 Changed most DclPrintf()s to DclPrintf(...)s to allow
                 redirecting results to file.
    08/07/97 PKG Changed BLOCK_SIZE to SMALL_BLOCK_SIZE to eliminate
                 redefinition between other modules
    07/07/97 DM  Fixed WHILE loops in stress routines.  Performance tests will
                 not fail at 240K anymore. Removed passing a buffer size to
                 the stress functions. It won't change and was just being
                 initialized to a DEFINE. Fseeking to current offset in a file
                 was wasting time, fread() increments file pointer on its own.
    07/01/97 DM  All calls to the fopen() function open files as "w+b"
                 random read/write binary, instead of the '_fmode' default.
    06/13/97 DM  We don't require the "/?" cmdline to show usage...
    05/07/97 DM  Created
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapireq.h>
#include <dlstats.h>
#include <dltools.h>
#include <dlperflog.h>
#include <dlservice.h>
#include <dllog.h>
#include "dltshared.h"
#include "dltfsshared.h"

/*  CURRENT_VERSION is always the current test version, irrespective
    of any emulation of older versions.  The version numbers MUST be
    in the Datalight standard version number format where each byte
    in the three highest bytes is a dot-separated segment, and the low
    byte, byte, if it is non-zero, is an alphabetic revision letter.
    The format is the same as that used in dlver.h and formatted by
    the DclVersionFormat() function.
*/
#define CURRENT_VERSION            (0x03000100) /* MUST be in the form described above! */    

/*  The following settings are used to allow older versions of the
    test to be emulated via a command-line override option.  All
    other preprocessor setting are not changed when different
    versions are emulated.  (Note that there may be other run-time
    stuff throughout the code which is version specific, so don't 
    be under the illusion that <this> is the only stuff which might
    change.)
*/    
/*  Settings for the default current version.  This value MUST have
    the same two high bytes as CURRENT_VERSION.  Whenever there 
    are behavioral changes to the test, at least one of the high
    two bytes of the version number should be changed, and the 
    test initialization/configuration code updated to allow the
    older behavior to be emulated.
*/
#define DEFAULT_VERSION       (CURRENT_VERSION) /* MUST be in the form described above! */   
#define DEFAULT_SLEEP_MS                    (0) /* Disabled */
#define DEFAULT_RANDOM_READ_PASSES          (2) /* # of times the file is read, on the average */
#define DEFAULT_RANDOM_WRITE_PASSES         (1) /* # of times the file is written, on the average */

/*  Settings for v2.0 specific stuff
*/
#define V200_VERSION               (0x02000000) /* MUST be in the form described above! */   
#define V200_DEFAULT_SLEEP_MS            (3000)
#define V200_DEFAULT_RANDOM_READ_PASSES    (10) /* # of times the file is read, on the average */
#define V200_DEFAULT_RANDOM_WRITE_PASSES    (5) /* # of times the file is written, on the average */

/*  END OF VERSION SPECIFIC STUFF
*/

#define FILL_VALUE                       (0xD5)

/*  Default test characteristics
*/
#define MAX_BUFFER_SIZE            (512 * 1024) /* Per the performance testing doc */
#define DEFAULT_DISK_SPACE         (256 * 1024)
#define DEFAULT_TEST_SECONDS               (60) /* Default seconds to run some tests */
#define DEFAULT_SAMPLE_RATE                 (2)
#define DEFAULT_SHORT_SLEEP_MS           (3000) 
#define DEFAULT_LONG_SLEEP_MS           (14000)
#define DEFAULT_DIRS                        (5)
#define DEFAULT_FILES                     (200)
#define DEFAULT_SCAN_FILES               (1000)
#define DEFAULT_SCAN_SAMPLES               (20)
#define DEFAULT_SEQ_BASE_BUFFER_SIZE        (0) /* Use the FS block size    */
#define DEFAULT_SEQ_INCREMENT_FACTOR        (2) /* Multiply by 2            */
#define DEFAULT_RANDOMWRITEFLUSHRATIO     (128)
#define DEFAULT_WEARLEVSTATICPERCENT       (50)
#define MAX_WEARLEVSTATICPERCENT           (90)
#define MAX_DEVNAME_LEN                    (16)
#define MAX_SAMPLE_RATE                  (3600) /* Limit to 60 minutes to allow a 32-bit accumulator with microsecond timers (max ~71 minutes) */ 
#define SMALL_FILE_SIZE_FACTOR              (8)
#define FRAG_FILE_COUNT                     (8)
#define FRAG_FILL_FLUSH_SAMPLE             (50) /* # of pairs after which to flush and sample */
#define FRAG_PAIRS_PER_DIR (FRAG_FILL_FLUSH_SAMPLE * 10)
#define RETRY_COUNT                        (10)
#define DEFAULT_MAXDISKSPACEDIVISOR        (16) /* Use 1/16th of available disk space */
#define DEFAULT_BUFFERSIZEDIVISOR         (128) /* Use 1/128th of "MaxDiskSpace" */
#define DEFAULT_SMALLIODIVISOR              (8) /* Small file and block size is 1/8th of the norm */
#define FILESET_MAX                (UINT_MAX-1) /* Max count the "FileSet" API can handle */

#define DLPERF_RANDOM_SEED         (12345678UL)
#define DLPERF_VERBOSITY    (DCL_VERBOSE_QUIET)
#define DLPERF_SAMPLE_RATE                 (10) /* seconds */


typedef struct
{
    D_UINT32        ulTestEmulationVersion; /* 0x02000000 means v2.0, 0x03010000 means v3.1, etc */
    DCLSHAREDTESTINFO sti;
    D_BUFFER       *pBuffer;
    D_UINT32        ulBufferSize;
    D_UINT32        ulBufferMin;            /* /SEQ base buffer size when testing ranges */
    D_UINT32        ulBufferInc;            /* /SEQ increment value when testing ranges */
    D_UINT32        ulDirCount;
    D_UINT32        ulFileCount;
    D_UINT32        ulScanCount;
    D_UINT32        ulFragMaxPairs;
    D_UINT32        ulSleepMS;
    D_UINT32        ulCount;
    D_UINT32        ulOriginalFreeSpaceKB;
    D_UINT32        ulMaxDiskSpace;         /* AKA "LargeFileSize" */
    D_UINT32        ulSmallFileSize;        /* Typically 8 * FSBlockSize */
    D_UINT32        ulFSBlockSize;
    FSPRIMCONTROL   FSPrim;                 /* Per iteration FS primitive stats */
    FSPRIMCONTROL   FSPrimTotal;            /* Whole test FS primitive stats */
    D_UINT32        ulIterationMS;
    D_UINT32        ulIterationSleepMS;
    D_UINT32        ulTotalMS;
    D_UINT32        ulTotalSleepMS;
    unsigned        nSampleRate;
    D_UINT32        ulTestMemUsage;
    D_UINT32        ulFillMin;
    DCLFSFILEHANDLE hFlushFile;
    char            szFlushFile[MAX_FILESPEC_LEN];
    char            szFileSpec[MAX_FILESPEC_LEN];
    char            szPath[MAX_FILESPEC_LEN];
    char            szFSPBuff[8];
    DCLFSSTATFS     statfs;
    int             iFlushOnWriteRatio;
    unsigned        nRandomReadPasses;
    unsigned        nRandomWritePasses;
    unsigned        nWearLevStaticPercent;
    unsigned        fNoDelete               : 1;
    unsigned        fFillVolume             : 1;
    unsigned        fRandomIO               : 1;
    unsigned        fRandRead               : 1;
    unsigned        fRandWrite              : 1;
    unsigned        fRandUnaligned          : 1;
    unsigned        fCreate                 : 1;
    unsigned        fFrag                   : 1;
    unsigned        fSequentialIO           : 1;
    unsigned        fSeqRead                : 1;
    unsigned        fSeqWrite               : 1;
    unsigned        fSeqRewrite             : 1;
    unsigned        fSeqUnaligned           : 1;
    unsigned        fMisalign               : 1;
    unsigned        fNoVerify               : 1;
    unsigned        fWriteVerify            : 1;
    unsigned        fDirTest                : 1;
    unsigned        fScanTest               : 1;
    unsigned        fWearLeveling           : 1;
    unsigned        fTreeTest               : 1;
    unsigned        fSparseTest             : 1;
} FSIOTESTINFO;

static void             ShowHelp(               DCLINSTANCEHANDLE hDclInst, const char *pszCmdName);
static void             DisplayTestDescription( DCLINSTANCEHANDLE hDclInst);
static D_UINT32         BoundaryRoundUp(        D_UINT32 ulValue, D_UINT32 ulMin, D_UINT32 ulMax);
static D_UINT32         GetKBPerSecond(         D_UINT64 ullKB, D_UINT32 ulMS);
static DCLSTATUS        GetSmallFileIOPerf(     FSIOTESTINFO *pTI);
static D_UINT32         TestCreateAndDelete(    FSIOTESTINFO *pTI, unsigned nSetNum, unsigned nCount, D_UINT32 ulBlocks, D_UINT32 ulBlockSize);
static DCLSTATUS        CreateFileSet(          FSIOTESTINFO *pTI, const char *pszPrefix, D_UINT32 *pulTotalMS, D_UINT32 *pulMaxUS, unsigned *pnCreated, unsigned nCount, D_INT32 lBlocks, D_UINT32 ulBlockSize, D_BOOL fVerbose, unsigned nFlushRatio, unsigned nSampleSeconds);
static DCLSTATUS        FileCreate(             FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nFileNum, D_UINT32 ulBlocks, D_UINT32 ulBlockSize, char *pBuffer, D_BOOL fFlush);
static DCLSTATUS        DeleteFileSet(          FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nCount, D_UINT32 *pulTotalTime, D_UINT32 *pulMaxUS, unsigned *pnDeleted, D_BOOL fVerbose);
static DCLSTATUS        FileDelete(             FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nFileNum);
static DCLSTATUS        VerifyFileSet(          FSIOTESTINFO *pTI, const char *pszPrefix, D_UINT32 *pulTotalTime, D_UINT32 *pulMaxUS, unsigned nCount, D_UINT32 ulBlocks, D_UINT32 ulBlockSize, D_BOOL fVerbose);
static DCLSTATUS        VerifyFile(             FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nFileNum, D_UINT32 ulBlocks, D_UINT32 ulBlockSize);
static DCLSTATUS        VerifyFileHelper(       FSIOTESTINFO *pTI, DCLFSFILEHANDLE hFile, D_UINT32 *pulVerified, unsigned nOffset);
static DCLSTATUS        VerifyFileSegment(      FSIOTESTINFO *pTI, DCLFSFILEHANDLE hFile, D_BUFFER *pBuffer, size_t nBufferLen, D_UINT32 ulFileOffset, D_UINT32 *pulBytesRead);
static DCLFSFILEHANDLE  CreateTempFile(         FSIOTESTINFO *pTI, const char *pszFormat, char *pszBuffer);
static DCLSTATUS        PopulateTempFile(       FSIOTESTINFO *pTI, const char *pszFormat, D_UINT32 ulSize, char *pszBuffer);
static DCLSTATUS        CreateUncachedFile(     FSIOTESTINFO *pTI, const char *pszFormat, D_UINT32 ulSize, char *pszBuffer);
static D_BOOL           CloseAndDeleteFile(     FSIOTESTINFO *pTI, DCLFSFILEHANDLE hFile);
static DCLFSFILEHANDLE  SequentialTestInit(     FSIOTESTINFO *pTI, const char *pszType, const char *pszFormatStr, D_UINT32 ulSize, D_UINT32 ulBuffLen);
static DCLSTATUS        SequentialRead(         FSIOTESTINFO *pTI, const char *pszStatsPrefix, const char *pszName, D_UINT32 ulBuffLen, unsigned nBuffOffset, unsigned nRangeIteration);
static DCLSTATUS        SequentialWrite(        FSIOTESTINFO *pTI, const char *pszStatsPrefix, const char *pszName, D_UINT32 ulBuffLen, unsigned nBuffOffset, unsigned nRangeIteration);
static DCLSTATUS        SequentialRewrite(      FSIOTESTINFO *pTI, const char *pszStatsPrefix, const char *pszName, D_UINT32 ulBuffLen, unsigned nBuffOffset, unsigned nRangeIteration);
static D_BOOL           SetSwitches(            FSIOTESTINFO *pTI, const char *pszCmdName, const char *pszCmdLine);
static DCLSTATUS        RunTests(               FSIOTESTINFO *pTI);
static void             AppSleep(               FSIOTESTINFO *pTI, D_UINT16 uMultiplier);
static DCLFSFILEHANDLE  CreateNumberedFile(     FSIOTESTINFO *pTI, char *pBuffer, const char *pszPrefix, unsigned nFileNum);
static DCLSTATUS        OpenNumberedFile(       FSIOTESTINFO *pTI, DCLFSFILEHANDLE *phFile, char *pBuffer, const char *pszPrefix, unsigned nFileNum);
static DCLSTATUS        DeleteNumberedFile(     FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nFileNum);
static DCLSTATUS        WearLeveling(           FSIOTESTINFO *pTI);
static DCLSTATUS        DirTest(                FSIOTESTINFO *pTI);
static DCLSTATUS        ScanTest(               FSIOTESTINFO *pTI);
static DCLSTATUS        DTCreateFiles(          FSIOTESTINFO *pTI, D_UINT32 ulFileStart, D_UINT32 ulFileCount, D_UINT32 ulFileSize, DCLPERFLOGHANDLE hPerfLog);
static DCLSTATUS        DTOpenSeekClose(        FSIOTESTINFO *pTI, D_UINT32 ulFileSize, DCLPERFLOGHANDLE hPerfLog);
static DCLSTATUS        DTDeleteFileSet(        FSIOTESTINFO *pTI, D_UINT32 ulFileNum, D_UINT32 *pulTotalTime, D_UINT32 *pulLongestTime);
static void             ResourceUsage(          FSIOTESTINFO *pTI);
static D_UINT32         FreeDiskBlocks(         FSIOTESTINFO *pTI);
static D_BOOL           InternalStatFS(         FSIOTESTINFO *pTI, DCLFSSTATFS *pStatFS);
static D_UINT32         RangeNext(              FSIOTESTINFO *pTI, D_UINT32 ulLastBuffSize);
static D_UINT64         RandomFlushHelper(      FSIOTESTINFO *pTI, DCLFSFILEHANDLE hTestFile, D_UINT32 *pulFlushNext);
static DCLSTATUS        CreateAndAppendTestDir( FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nSuffix);
static DCLSTATUS        DestroyAndRemoveTestDir(FSIOTESTINFO *pTI);
static DCLSTATUS        TreeTest(               FSIOTESTINFO *pTI);
static DCLSTATUS        TreeCreateRecurse(      FSIOTESTINFO *pTI, unsigned nLevel, unsigned nMaxLevel, unsigned nFiles, D_UINT32 *pulTotal, D_UINT64 *pullTimeUS);
static DCLSTATUS        PrimWrite(              FSIOTESTINFO *pTI, D_BUFFER *pBuffer, size_t nElementLen, size_t nNumElements, DCLFSFILEHANDLE hFile, size_t *pnElementsWritten);
static const char *     GetFreeSpacePercent(    FSIOTESTINFO *pTI);
static DCLSTATUS        WriteHelper(            FSIOTESTINFO *pTI, DCLFSFILEHANDLE hFile, D_UINT32 ulIOSize, D_UINT32 ulCount, unsigned nBuffOffset, D_UINT32 ulFileOffset, D_BOOL fInitData, D_UINT32 *pulTotalWritten, D_UINT32 *pulLongestTimeUS, D_UINT64 *pullTotalTimeUS);
static DCLSTATUS        FillVolumeBigFile(      FSIOTESTINFO *pTI, unsigned nOffset);
static DCLSTATUS        FillVolumeMidSizedFiles(FSIOTESTINFO *pTI);
static DCLSTATUS        FillVolumeSmallFiles(   FSIOTESTINFO *pTI);
static DCLSTATUS        FillVolumeTinyFiles(    FSIOTESTINFO *pTI);
static DCLSTATUS        FillVolumeZeroByteFiles(FSIOTESTINFO *pTI);
static DCLSTATUS        FillVolumeFileSetCreate(FSIOTESTINFO *pTI, const char *pszPrefix, D_UINT32 ulFileSize, D_UINT32 *pulTotalMS, D_UINT32 *pulFilesCreated, unsigned *pnDirsCreated);
static DCLSTATUS        FillVolumeFileSetDelete(FSIOTESTINFO *pTI, const char *pszPrefix, unsigned nDirs, D_UINT32 *pulTotalMS, D_UINT32 *pulFilesDeleted);
static DCLSTATUS        SparseTest(             FSIOTESTINFO *pTI);
static DCLSTATUS        SparseLimits(           FSIOTESTINFO *pTI);
static DCLSTATUS        SparseExtendTest(       FSIOTESTINFO *pTI, D_UINT32 ulFileSize, const char *pszDescription, unsigned *pnCreated);



/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSIOMain(
    DCLTOOLPARAMS  *pTP)
{
    DCLSTATUS       dclStat;
    FSIOTESTINFO    ti = {0};
    char            szCurrentVer[32];
    char            szEmulVer[32];
    char            szScaleBuff[20];
    char            szScaleBuff2[20];
    char            szScaleBuff3[20];
    char            szScaleBuff4[20];
    FSIOTESTINFO   *pTI = &ti;      /* Allow the PRIM* macros to work */

    DclAssertWritePtr(pTP, sizeof(*pTP));

    /*  Ensure the DCL instance is already initialized before bothering
        to do anything else.
    */
    if(!pTP->hDclInst)
        return DCLSTAT_INST_NOTINITIALIZED;

    ti.sti.hDclInst = pTP->hDclInst;
    ti.ulTestEmulationVersion = CURRENT_VERSION;

    /*  If the command line switches are not valid (or if
        it was a help request), return a failure code.
    */
    if(!SetSwitches(&ti, pTP->pszCmdName, pTP->pszCmdLine))
         return DCLSTAT_FAILURE;

    dclStat = DclTestLogOpen(&ti.sti);
    if(dclStat != DCLSTAT_SUCCESS && dclStat != DCLSTAT_FEATUREDISABLED)
        goto MainCleanup;

    /*  If the high two bytes of the emulated version are different than
        the high two bytes of CURRENT_VERSION, display the emulation
        version.  If not, then we are not emulating any older behavior,
        so ignore any differences.
    */        
    if((ti.ulTestEmulationVersion & 0xFFFF0000) != (CURRENT_VERSION & 0xFFFF0000))
    {
        char szTempVer[32];

        DclSNPrintf(szEmulVer, sizeof(szEmulVer), " (emulating v%s)", 
            DclVersionFormat(szTempVer, sizeof(szTempVer), ti.ulTestEmulationVersion, NULL));
    }
    else
    {
        DclStrCpy(szEmulVer, "");
    }

    DclPrintf("DCL File System I/O Performance Test v%s%s\n",
        DclVersionFormat(szCurrentVer, sizeof(szCurrentVer), CURRENT_VERSION, NULL), szEmulVer);
    
    DclSignOn(FALSE);

    /*  Note that we're allocating an I/O buffer twice the size of
        the typical I/O, so the unaligned sequential I/O test runs
        properly -- it uses a length which is ~1.75 times the normal
        I/O size.
    */        
    ti.ulTestMemUsage = (ti.ulBufferSize * 2) + DCL_ALIGNSIZE;

    ti.pBuffer = DclMemAlloc(ti.ulTestMemUsage);
    if(!ti.pBuffer)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
        goto MainCleanup;
    }

    /*  If instructed, force the general data buffer to be misaligned
        by one byte.
    */
    if(ti.fMisalign)
        ti.pBuffer++;

    /*  Seed the pseudo-random number generator
    */
    if(!ti.sti.ullRandomSeed)
        ti.sti.ullRandomSeed = DclOsTickCount();

    DclTestRequestorOpen(&ti.sti, ti.szPath, ti.statfs.szDeviceName);

    ti.hFlushFile = CreateTempFile(&ti, "fsflush%04X.dat", ti.szFlushFile);
    if(!ti.hFlushFile)
    {
        dclStat = DCLSTAT_CURRENTLINE;
        goto MainCleanup;
    }

    dclStat = PRIMFLUSH(ti.hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        goto MainCleanup;

    /*  Re-do the statfs call after having done an initial flush.
    */
    if(!InternalStatFS(&ti, &ti.statfs))
    {
        dclStat = DCLSTAT_CURRENTLINE;
        goto MainCleanup;
    }

    /*  Print out the test header.  These items should list in the 
        actual order the tests will be executed.
    */
    DclPrintf("\nTest Parameters:\n");
    DclPrintf("  Test Cases: %s%s%s%s%s%s%s%s%s%s\n",
        ti.fSequentialIO        ? " /SEQ"    : "",
        ti.fCreate              ? " /CREATE" : "",
        ti.fFrag                ? " /FRAG"   : "",
        ti.fRandomIO            ? " /RAND"   : "",
        ti.fFillVolume          ? " /FILL"   : "",
        ti.fDirTest             ? " /DIR"    : "",
        ti.fScanTest            ? " /SCAN"   : "",
        ti.fSparseTest          ? " /SPARSE" : "",
        ti.fTreeTest            ? " /TREE"   : "",
        ti.fWearLeveling        ? " /WEAR"   : "");

    ti.ulOriginalFreeSpaceKB = (D_UINT32)DclMulDiv(ti.statfs.ulFreeBlocks, ti.statfs.ulBlockSize, 1024);

    DclPrintf("  Device Name: %24s\n",                      ti.statfs.szDeviceName);
    DclPrintf("  File System Block Size:    %10lU Bytes\n", ti.statfs.ulBlockSize);
    DclPrintf("  File System Volume Size:  %14s\n",         DclScaleItems(ti.statfs.ulTotalBlocks, ti.statfs.ulBlockSize, szScaleBuff, sizeof(szScaleBuff)));
    DclPrintf("  File System Free Space:   %14s (%s%%)\n",  DclScaleKB(ti.ulOriginalFreeSpaceKB, szScaleBuff2, sizeof(szScaleBuff2)), GetFreeSpacePercent(&ti));
    DclPrintf("  Max Test File Size (/MAX):%14s\n",         DclScaleBytes(ti.ulMaxDiskSpace, szScaleBuff3, sizeof(szScaleBuff3)));
    DclPrintf("  Max I/O Buffer Size (/B): %14s\n",         DclScaleBytes(ti.ulBufferSize, szScaleBuff4, sizeof(szScaleBuff4)));
    DclPrintf("  Random Seed:     %20llU\n",                VA64BUG(ti.sti.ullRandomSeed));

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(ti.sti.fPerfLog, ti.sti.hLog, "FSIO", "Configuration", NULL, ti.sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "DiskSizeKB",   (D_UINT32)DclMulDiv(ti.statfs.ulTotalBlocks, ti.statfs.ulBlockSize, 1024));
        DCLPERFLOG_NUM(   hPerfLog, "FreeSpaceKB",  ti.ulOriginalFreeSpaceKB);
        DCLPERFLOG_NUM(   hPerfLog, "BlockSize",    ti.statfs.ulBlockSize);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB",    ti.ulMaxDiskSpace / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen",    ti.ulBufferSize);
        DCLPERFLOG_NUM64( hPerfLog, "RandomSeed",   ti.sti.ullRandomSeed);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    /*  Call the function which will call the individual test routines
    */
    dclStat = RunTests(&ti);

    /*  If we adjusted the buffer to force misalignment, restore the
        original value now so we free the right memory.
    */
    if(ti.fMisalign)
        ti.pBuffer--;

  MainCleanup:

    {
        DCLSTATUS   dclStat2;

        /*  Free resources in reverse order of allocation where possible,
            preserving any original failure codes.
        */            
        
        if(ti.hFlushFile)
        {
            dclStat2 = PRIMCLOSE(ti.hFlushFile);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
            
            dclStat2 = PRIMDELETE(ti.szFlushFile);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }

        dclStat2 = DclTestRequestorClose(&ti.sti);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;

        if(ti.pBuffer)
        {
            dclStat2 = DclMemFree(ti.pBuffer);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }

        dclStat2 = DclTestLogClose(&ti.sti);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;

        if(ti.sti.pStatsBuff)
        {
            dclStat2 = DclMemFree(ti.sti.pStatsBuff);
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = dclStat2;
        }
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("\n%s: Error %lX\n\n", pTP->pszCmdName, dclStat);
    else
        DclPrintf("%s completed successfully\n\n", pTP->pszCmdName);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:
       pszCmdName - The name of the command

    Notes:
       Shows the usage of the program.

    Returns:
       nothing
-------------------------------------------------------------------*/
static void ShowHelp(
    DCLINSTANCEHANDLE   hDclInst,
    const char         *pszCmdName)
{
    DclPrintf("Usage:  %s tests [options]\n\n", pszCmdName);
    DclPrintf("Where 'tests' are one or more of the following:\n");
    DclPrintf("  /DLPERF        Run the Datalight standard performance tests (see description)\n");
    DclPrintf("  /CREATE        Run the create/delete tests\n");
    DclPrintf("  /DIR           Run the directory stripe tests\n");
    DclPrintf("  /FILL[:min]    Run the volume fill tests\n");
    DclPrintf("  /SCAN          Run the directory scan tests\n");
    DclPrintf("  /FRAG          Run the fragmentation tests\n");
    DclPrintf("  /RAND[:r|w|u]  Run the random Read/Write/Unaligned I/O tests\n");
    DclPrintf("  /SEQ[:r|w|e|u] Run the sequential Read/Write/rEwrite/Unaligned I/O tests\n");
    DclPrintf("  /SPARSE        Run the sparse file/extend/truncate tests\n");
    DclPrintf("  /TREE          Run the tree tests\n");
    DclPrintf("  /WEAR[:opts]   Run the wear-leveling tests (see extended test description)\n\n");
    DclPrintf("Basic Options:\n");
    DclPrintf("  /?             Display this help information\n");
    DclPrintf("  /??            Display an extended test description for all tests\n");
    DclPrintf("  /TestName /?   Display an extended test description for the specified test\n");
    DclPrintf("  /B:size        Specifies the I/O Buffer size (default is 1/%uth of /MAX:n)\n", DEFAULT_BUFFERSIZEDIVISOR);
    DclPrintf("  /Max:size      The Maximum test file size (default is 1/%uth free disk space)\n", DEFAULT_MAXDISKSPACEDIVISOR);
    DclPrintf("  /P:'path'      Specifies the Path to use (default is the current directory)\n");
    DclPrintf("  /R:n           Specifies the sample Rate in seconds, for those tests which\n");
    DclPrintf("                 support sampling.  Specify 0 to sample every buffer processed\n");
    DclPrintf("                 (default is %u, max is %u).\n", DEFAULT_SAMPLE_RATE, MAX_SAMPLE_RATE);
    DclPrintf("  /S:n           Sleep time in MS.  Injected sleep periods simulate application\n");
    DclPrintf("                 behavior, enabling background operations.  'n' may be \"short\"\n");
    DclPrintf("                 or \"long\" to use the default ranges (default=short 0=disable).\n");
    DLTSHAREDTESTHELP_TIME();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Logging Options:\n");
  #if DCLCONF_OUTPUT_ENABLED
    DLTSHAREDTESTHELP_LOG();
    DLTSHAREDTESTHELP_LB();
    DLTSHAREDTESTHELP_LS();
    DLTSHAREDTESTHELP_PERFLOG();
    DLTSHAREDTESTHELP_STATS();
    DLTSHAREDTESTHELP_TRACE();
  #endif
    DLTSHAREDTESTHELP_REQ();
    DLTSHAREDTESTHELP_VERBOSITY();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Advanced Options:\n");
    DclPrintf("  /Ver:n         Emulate an older version of the test.  This must be a 3 digit\n");
    DclPrintf("                 number signifying the major, minor, and subminor portions of the\n");
    DclPrintf("                 version number.  Only \"200\" is recognized at this time.  It is\n");
    DclPrintf("                 recommended that this option be specified first if it is used.\n");
    DclPrintf("  /I[:n]         Iterations -- run indefinitely if n is unspecified (default=1)\n");
    DLTSHAREDTESTHELP_SEED();
    DclPrintf("  /Dirs:n        The number of dirs to use for some tests (default=%u)\n", DEFAULT_DIRS);
    DclPrintf("  /Files:n       The number of files to use for some tests (default=%u)\n", DEFAULT_FILES);
    DclPrintf("  /SFiles:n      The number of files to use for the /SCAN test (default=%u)\n", DEFAULT_SCAN_FILES);
    DclPrintf("  /FPairs:n      The maximum number of file pairs to create in the /FRAG test.\n");
    DclPrintf("  /RandFOW:n     Flush-On-Write, flush after every Nth random write (default=%u)\n", DEFAULT_RANDOMWRITEFLUSHRATIO);
    DclPrintf("  /RandPass:r:w  Random test passes to perform for reads and writes.  Defaults\n");
    DclPrintf("                 to %u for reads and %u for writes.\n", DEFAULT_RANDOM_READ_PASSES, DEFAULT_RANDOM_WRITE_PASSES);
    DLTSHAREDTESTHELP_PROF();
    DclPrintf("  /Misalign      Perform all I/O with a misaligned data buffer\n");
    DclPrintf("  /NoDel         Don't delete test data (may cause failures and affect results)\n");
    DclPrintf("  /Start[:b[,n]] This option causes the /SEQ test to iterate through a range of\n");
    DclPrintf("                 I/O buffer sizes, starting with 'b' up to the size specified\n");
    DclPrintf("                 by the /B parameter, incrementing by 'n'.  If 'n' <= 10, then\n");
    DclPrintf("                 the previous buffer size is multiplied by 'n' to get the next\n");
    DclPrintf("                 buffer size, otherwise 'n' bytes are added to the previous\n");
    DclPrintf("                 buffer size.  'b' is always specified in bytes.  If they are\n");
    DclPrintf("                 not specified, 'b' defaults to the file system native block\n");
    DclPrintf("                 size, and 'n' defaults to %u.\n", DEFAULT_SEQ_INCREMENT_FACTOR);
    DclPrintf("  /Verify        Read-back and Verify writes (will skew performance results)\n");
    DclPrintf("  /FSBlockSize:n The file system block size to use, in bytes (See Notes).\n");
    DclPrintf("  /FSDevName:nam The device name on which the file system resides (see Notes).\n");
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");
    DclPrintf("Notes: 1) Options which have a 'size' designation may be specified in hex (0x)\n");
    DclPrintf("          or decimal, and may be suffixed by 'B', 'KB', or 'MB'.  If there is\n");
    DclPrintf("          no scale designated, then KB is assumed.  Don't use a 'B' suffix for\n");
    DclPrintf("          hex numbers as it will be interpreted as part of the hex number.\n");
    DclPrintf("       2) For consistent test results (especially on flash media), each test\n");
    DclPrintf("          MUST be individually run on a freshly formatted disk.\n");
    DclPrintf("       3) The /FSBlockSize and /FSDevName options are often used together and\n");
    DclPrintf("          are useful for environments where the automatic determination doesn't\n");
    DclPrintf("          work.  Using them will override any automatically determined values.\n");
    DclPrintf("       4) When the /LOG option is used, the file should be on a different disk\n");
    DclPrintf("          than that being tested, or the /LB option should specify a buffer\n");
    DclPrintf("          large enough to hold the entire test output.  Otherwise performance\n");
    DclPrintf("          test results may be adversely affected.\n");
    DclPrintf("       5) Test names and options are not case sensitive or order sensitive.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpDLPerfTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpDLPerfTest(void)
{
    DclPrintf("  /DLPERF\n");
    DclPrintf("    This option causes a subset set of tests to be run using a predefined\n");
    DclPrintf("    set of options.  Using this option causes the following options to be\n");
    DclPrintf("    used:\n\n");
    DclPrintf("        /SEQ\n");
    DclPrintf("        /FRAG\n");
    DclPrintf("        /RAND\n");
    DclPrintf("        /DIR\n");
    DclPrintf("        /SCAN\n");
    DclPrintf("        /TREE\n");
    DclPrintf("        /SEED:%u\n", DLPERF_RANDOM_SEED);
    DclPrintf("        /R:%u\n", DLPERF_SAMPLE_RATE);
    DclPrintf("        /V:%u\n\n", DLPERF_VERBOSITY);
    DclPrintf("    Additional options may be specified which can override or supplement\n");
    DclPrintf("    these settings.  For example, /B:n may be used to cause a specific\n");
    DclPrintf("    buffer size to be used, rather than calculating the default.\n\n");
    DclPrintf("    Individual test types can be disabled by specifying the switch with a\n");
    DclPrintf("    trailing '-'.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpSequentialTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpSequentialTest(void)
{
    DclPrintf("  /SEQ[:r|w|e|u]\n");
    DclPrintf("    This test exercises sequential I/O performance.  Specify 'r', 'w', or 'e'\n");
    DclPrintf("    to cause only the Read, Write, or rEwrite portions of the test to run.\n");
    DclPrintf("    Specify 'r-', 'w-', 'e-' to prevent the Read, Write, or rEwrite portions\n");
    DclPrintf("    of the test from running.\n\n");
    DclPrintf("    The 'u-' option is independently processed from the others and may be used\n");
    DclPrintf("    to prevent the group of unaligned tests from running.\n\n");
    DclPrintf("    The file size used is that specified by the /MAX command-line parameter,\n");
    DclPrintf("    and the I/O buffer size used is that specified by the /B command-line\n");
    DclPrintf("    parameter.\n\n");
    DclPrintf("    For the read portion of the test, the initial file is created and fully\n");
    DclPrintf("    populated, then it is read sequentially from start to finish using read\n");
    DclPrintf("    operations of the specified buffer size.\n\n");
    DclPrintf("    The write test creates an empty file and writes sequentially to it in the\n");
    DclPrintf("    specified I/O buffer size, until it is fully populated.\n\n");
    DclPrintf("    The rewrite test creates a fully populated file and sequentially overwrites\n");
    DclPrintf("    it using the specified I/O buffer size.\n\n");
    DclPrintf("    The results of these tests are measured in terms of the total time to\n");
    DclPrintf("    complete the test, and are displayed in terms of KB per second.\n\n");
    DclPrintf("    Additionally, the sequential I/O test is designed to sample the I/O rate\n");
    DclPrintf("    every /R:n seconds (default is 2), reporting the average and maximum time\n");
    DclPrintf("    to write the buffers within that particular window.  This allows worst case\n");
    DclPrintf("    streaming performance figures to be calculated, and I/O performance\n");
    DclPrintf("    patterns to be analyzed.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpRandomTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpRandomTest(void)
{
    DclPrintf("  /RAND[:r|w|u]\n");
    DclPrintf("    This test measures random I/O performance.  Specify 'r', 'w' or 'u' to\n");
    DclPrintf("    cause only the Read, Write, or Unaligned portions of the test to run.\n");
    DclPrintf("    Specify 'r-', 'w-', or 'u-' to prevent the Read, Write, or Unaligned\n");
    DclPrintf("    portions of the test from running.\n\n");
    DclPrintf("    The buffer (/B) and file (/MAX) sizes used are scaled according to the\n");
    DclPrintf("    /Divisor:n parameter (defaults to 1/8th).  This ratio is used because\n");
    DclPrintf("    'typical' application behavior in an embedded environment does random\n");
    DclPrintf("    I/O on smaller sized files than those accessed via sequential means.\n\n");
    DclPrintf("    The read portion of the test first creates a fully populated file of the\n");
    DclPrintf("    designated size, then reads at random offsets which are evenly divisible\n");
    DclPrintf("    by the calculated buffer size, buffers which are the calculated buffer\n");
    DclPrintf("    size long.\n\n");
    DclPrintf("    The write portion of the test first creates a fully populated file of the\n");
    DclPrintf("    designated size, then writes at random offsets which are evenly divisible\n");
    DclPrintf("    by the calculated buffer size, buffers which are the calculated buffer size\n");
    DclPrintf("    long.  Additionally, the write test will perform a flush operation after\n");
    DclPrintf("    every Nth write, as specified with the /RandFOW:n option.\n\n");
    DclPrintf("    The Unaligned portion of the test randomly reads and writes unaligned data\n");
    DclPrintf("    to the same file.\n\n");
    DclPrintf("    The tests are run until a prescribed amount of data has been processed,\n");
    DclPrintf("    based on the /RandPass:r:w parameter.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpFragmentationTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpFragmentationTest(void)
{
    DclPrintf("  /FRAG\n");
    DclPrintf("    This test measures performance on fragmented versus non-fragmented\n");
    DclPrintf("    media.  This test will only provide the desired results if run on a\n");
    DclPrintf("    freshly formatted disk, where the /MAX:n parameter specifies a size\n");
    DclPrintf("    that is approximately 1/%uth the total formatted size of the disk.\n", DEFAULT_MAXDISKSPACEDIVISOR);
    DclPrintf("    The test is implemented using the following steps:\n");
    DclPrintf("    1) The first step creates 8 files of /MAX size, verifies the file\n");
    DclPrintf("       contents, and deletes the files.  Performance measurements are\n");
    DclPrintf("       gathered for each file written, verified, and deleted.\n");
    DclPrintf("    2) The second step creates pairs of files, one \"small\" file and one\n");
    DclPrintf("       \"tiny\" file, until the disk is nearly full.  The small files are\n");
    DclPrintf("       up to %u times the file system block size, and the tiny files are the\n", SMALL_FILE_SIZE_FACTOR);
    DclPrintf("       same size as a file system block.  All the small, files are then\n");
    DclPrintf("       deleted, leaving the disk in a fragmented state.\n");
    DclPrintf("    3) The third step then repeats the operations of the first step.  The\n");
    DclPrintf("       effect of fragmentation can be seen by comparing the results of the\n");
    DclPrintf("       first step with the results of the third step.\n");
    DclPrintf("    4) The test cleanup operations then delete the tiny files.\n\n");
    DclPrintf("    NOTE:  This test may take a LONG time to complete!  The files created\n");
    DclPrintf("           in Step 2 are placed in a set of subdirectories to (hopefully)\n");
    DclPrintf("           improve performance.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpDirScanTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpDirScanTest(void)
{
    DclPrintf("  /SCAN\n");
    DclPrintf("    This test measures time it take to create, access, and delete a large\n");
    DclPrintf("    number of files in a single directory.  The file count is configurable\n");
    DclPrintf("    with the /SFILES:n option.  The file size is that of the file system\n");
    DclPrintf("    block size.\n\n");
    DclPrintf("    The test consists of three phases:\n");
    DclPrintf("    - Creating all the files, which consists of performing a file create,\n");
    DclPrintf("      writing one block of random data, and closing the file.\n");
    DclPrintf("    - Opening and closing each file.\n");
    DclPrintf("    - Deleting each file.\n\n");
    DclPrintf("    Each of the phases is sampled 20 times.  Note that the number of files\n");
    DclPrintf("    used may be rounded up so that each of the 20 samples is the same size.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpDirStripeTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpDirStripeTest(void)
{
    DclPrintf("  /DIR\n");
    DclPrintf("    This test exercises file operations in subdirectories, using write\n");
    DclPrintf("    patterns which are likely to stripe directory information across a\n");
    DclPrintf("    disk.  A given number of directories (specified with /DIRS) are\n");
    DclPrintf("    first created.  Then a FileSet, consisting of one file in each\n");
    DclPrintf("    directory, are created iteratively.  The total number of files is\n");
    DclPrintf("    specified with the /FILES option.  The file size is scaled auto-\n");
    DclPrintf("    matically based on the /MAX parameter, with the minimum allowable size\n");
    DclPrintf("    for a file being one byte.  This scaling includes logic to allow some\n");
    DclPrintf("    space for disk metadata.\n\n");
    DclPrintf("    Performance metrics are:\n");
    DclPrintf("    - The total time to create the entire FileSet structure, along with\n");
    DclPrintf("      the average and longest creation times for each FileSet.\n");
    DclPrintf("    - The total time to delete the entire FileSet structure, along with\n");
    DclPrintf("      the average and longest deletion times for each FileSet.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpSparseTest()

    This function displays an extended test description.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void HelpSparseTest(void)
{
    DclPrintf("  /SPARSE\n");
    DclPrintf("    To be completed.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpTreeTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpTreeTest(void)
{
    DclPrintf("  /TREE\n");
    DclPrintf("    To be completed.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpWearLevelingTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpWearLevelingTest(void)
{
    DclPrintf("  /WEAR[:p]\n");
    DclPrintf("    This test is designed to measure the wear-leveling characteristics of the\n");
    DclPrintf("    file system and underlying block device driver.  Typically this test is\n");
    DclPrintf("    run on a file system or block device driver which has the ability to report\n");
    DclPrintf("    statistics on I/O operations.  This test should always be run on a freshly\n");
    DclPrintf("    formatted disk.\n\n");
    DclPrintf("    The following methodology is used:\n");
    DclPrintf("    1) The statistics counters (if any) are reset via the IOCTL interface.\n");
    DclPrintf("    2) The optional 'p' value specifies the percent of the disk which should\n");
    DclPrintf("       contain entirely static data.  This amount of disk space is filled with\n");
    DclPrintf("       files of the size designated by the /MAX parameter.  The exact amount\n");
    DclPrintf("       filled will be approximate due to file system overhead.  The default\n");
    DclPrintf("       value for 'p' is 50%%.  The maximum value for 'p' is 90%%.\n");
    DclPrintf("    3) The statistics counters are dumped to the standard output device.\n");
    DclPrintf("    4) The value specified by the /Divisor parameter is used in conjunction\n");
    DclPrintf("       with the /MAX value to determine a standard file size to use in the\n");
    DclPrintf("       next step.\n");
    DclPrintf("    5) Files of the calculated size are then randomly written, re-written, and\n");
    DclPrintf("       erased, taking care not to entirely fill the disk.  See the source code\n");
    DclPrintf("       for the exact algorithm used.\n");
    DclPrintf("    6) The test runs for the amount of time specified with the /Time:n parameter.\n");
    DclPrintf("    7) The statistics counters are dumped to the standard output device.\n");
    DclPrintf("    8) All test files are deleted.\n");

    return;
}


/*-------------------------------------------------------------------
    HelpCreateTest()

    Description
        This function displays an extended test description.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void HelpCreateTest(void)
{
    DclPrintf("  /CREATE\n");
    DclPrintf("    This test creates, verifies, and deletes sets of small files using I/O\n");
    DclPrintf("    buffer sizes which are \"off-by-one\" from the typical 512, 1024, and 2048\n");
    DclPrintf("    I/O buffer sizes.  Therefore, tested I/O buffer sizes are 511, 512, 513,\n");
    DclPrintf("    1023, 1024, 1025, 2047, 2048, and 2049 bytes.  However no size larger than\n");
    DclPrintf("    that specified by the /B command-line parameter (or default value) will be\n");
    DclPrintf("    used.\n");
    return;
}


/*-------------------------------------------------------------------
    Local: HelpFillTest()

    Display an extended test description.

    Parameters:
        None.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void HelpFillTest(void)
{
    DclPrintf("  /FILL[:min]\n");
    DclPrintf("    The /FILL test exercises entirely filling a volume using various file sizes.\n");
    DclPrintf("    The test iterates through a variety of file sizes, starting with a single\n");
    DclPrintf("    file filling the whole volume, reducing eventually to filling the volume with\n");
    DclPrintf("    zero length files.\n\n");
    DclPrintf("    For some file system and volume size combinations, this test may take a very\n");
    DclPrintf("    LONG time to complete.  The optional \"min\" parameter specifies the minimum\n");
    DclPrintf("    file size to use, in bytes, and can be used to reduce the test time.\n\n");
    DclPrintf("  Analyzing Results:\n");
    DclPrintf("    The fill process performs sampling at regular intervals, showing the number of\n");
    DclPrintf("    files created, the throughput in KB/Sec, and the percentage of free space\n");
    DclPrintf("    remaining on the volume.  Key things to watch for are whether the performance\n");
    DclPrintf("    is consistent throughout the process, or does it deteriorate as the volume gets\n");
    DclPrintf("    more full.  Finally, at the conclusion of each iteration, once the volume is\n");
    DclPrintf("    full, the test displays the \"Per file metadata overhead\".\n");

    return;
}


/*-------------------------------------------------------------------
    DisplayTestDescription()

    Description
        This function displays an extended description for each
        test.

    Parameters
        None

    Return Value
        None
-------------------------------------------------------------------*/
static void DisplayTestDescription(
    DCLINSTANCEHANDLE   hDclInst)
{
    DclPrintf("\n                          Extended Test Description\n\n");
    DclPrintf("The File System I/O test is designed to test file system performance in a\n");
    DclPrintf("fashion similar to that done in a typical application environment.\n\n");
    DclPrintf("The following sections describe the various tests:\n");
    DclPrintf("\n");

    HelpDLPerfTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpCreateTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpDirScanTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpDirStripeTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpFillTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpFragmentationTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpRandomTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpSequentialTest();
    if(!DclTestPressEnter(hDclInst))
        return;
    DclPrintf("\n");

    HelpWearLevelingTest();
    DclPrintf("\n");

    return;
}


/*-------------------------------------------------------------------
    SetSwitches()

    Description
        This function parses the command line items and configures
        the test as specified.

    Parameters
        pTI        - A pointer to the FSIOTESTINFO structure to use
        pszCmdLine - The test command line

    Return Value
        TRUE if successful, else FALSE.  In the event that FALSE is
        returned any necessary error messages or help display will
        already have been displayed.
-------------------------------------------------------------------*/
static D_BOOL SetSwitches(
    FSIOTESTINFO   *pTI,
    const char     *pszCmdName,
    const char     *pszCmdLine)
{
    #define         ARGBUFFLEN  (128)
    D_UINT16        uIndex;
    D_UINT16        uSwitch;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;
    D_UINT32        ulStart = D_UINT32_MAX;
    D_UINT32        ulIncrement = 0;
    char            szFSDevName[DCL_MAX_DEVICENAMELEN] = {0};

    /*  Init default settings
    */
    pTI->nSampleRate            = DEFAULT_SAMPLE_RATE;
    pTI->iFlushOnWriteRatio     = DEFAULT_RANDOMWRITEFLUSHRATIO;
    pTI->nWearLevStaticPercent  = DEFAULT_WEARLEVSTATICPERCENT;
    pTI->sti.nVerbosity         = DCL_VERBOSE_NORMAL;
    pTI->ulSleepMS              = DEFAULT_SLEEP_MS;
    pTI->nRandomReadPasses      = DEFAULT_RANDOM_READ_PASSES;
    pTI->nRandomWritePasses     = DEFAULT_RANDOM_WRITE_PASSES;

    uArgCount = DclArgCount(pszCmdLine);
    if(!uArgCount)
    {
        ShowHelp(pTI->sti.hDclInst, pszCmdName);
        return FALSE;
    }

    /*  Start with argument number 1
    */
    for(uIndex = 1; uIndex <= uArgCount; uIndex++)
    {
        char    achArgHelp[8];

        /*  Initialize this to 1 so that the error handling code at the
            label "BadOption" will work properly for this set of options.
        */
        uSwitch = 1;

        if(!DclArgRetrieve(pszCmdLine, uIndex, sizeof(achArgBuff), achArgBuff))
        {
            DclPrintf("Bad argument!\n\n");
            ShowHelp(pTI->sti.hDclInst, pszCmdName);
            return FALSE;
        }

                /*-----------------------------------------*\
                 *                                         *
                 *    Process main test type args first    *
                 *                                         *
                \*-----------------------------------------*/

        if(DclStrICmp(achArgBuff, "/DLPERF") == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpDLPerfTest();
                return FALSE;
            }

            pTI->fSequentialIO = TRUE;
            pTI->fSeqRead = TRUE;
            pTI->fSeqWrite = TRUE;
            pTI->fSeqRewrite = TRUE;
            pTI->fSeqUnaligned = TRUE;
            pTI->fRandomIO = TRUE;
            pTI->fRandRead = TRUE;
            pTI->fRandWrite = TRUE;
            pTI->fDirTest = TRUE;
            pTI->fScanTest = TRUE;
            pTI->fFrag = TRUE;
            pTI->nSampleRate = DLPERF_SAMPLE_RATE;
            pTI->sti.ullRandomSeed = DLPERF_RANDOM_SEED;
            pTI->sti.nVerbosity = DLPERF_VERBOSITY;

            if(pTI->ulTestEmulationVersion >= DEFAULT_VERSION)
            {
                pTI->fRandUnaligned = TRUE;
                pTI->fTreeTest = TRUE;
            }

            continue;
        }
        else if(DclStrICmp(achArgBuff, "/CREATE") == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpCreateTest();
                return FALSE;
            }

            pTI->fCreate = TRUE;
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FRAG", 5) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpFragmentationTest();
                return FALSE;
            }

            if(achArgBuff[5] == '-' && achArgBuff[6] == 0 && pTI->fFrag)
                pTI->fFrag = FALSE;
            else if(achArgBuff[5] == 0)
                pTI->fFrag = TRUE;
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FILL", 5) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpFillTest();
                return FALSE;
            }

            if(achArgBuff[5] == ':')
            {
                const char *pTmp;
                
                pTmp = DclNtoUL(&achArgBuff[6], &pTI->ulFillMin);
                if(!pTmp || *pTmp != 0)
                    goto BadOption;
            }
            else
            {
                pTI->ulFillMin = 0;
            }            

            pTI->fFillVolume = TRUE;
            continue;
        }
        else if( (DclStrNICmp(achArgBuff, "/RAND", 5) == 0) &&
                 (DclStrNICmp(achArgBuff, "/RANDFOW:", 9) != 0) &&
                 (DclStrNICmp(achArgBuff, "/RANDPASS:", 10) != 0) )
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpRandomTest();
                return FALSE;
            }

            pTI->fRandomIO = TRUE;

            if(achArgBuff[5] == ':')
            {
                if(DclToUpper(achArgBuff[6]) == 'R')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandRead)
                        pTI->fRandRead = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandRead = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[6]) == 'W')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandWrite)
                        pTI->fRandWrite = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandWrite = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[6]) == 'U')
                {
                    if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fRandUnaligned)
                        pTI->fRandUnaligned = FALSE;
                    else if(achArgBuff[7] == 0)
                        pTI->fRandUnaligned = TRUE;
                    else
                        goto BadOption;
                }
                else
                {
                    goto BadOption;
                }

                /*  If both random read and write are, or have become, FALSE,
                    turn off the random I/O test entirely.
                */
                if(!pTI->fRandRead && !pTI->fRandWrite && !pTI->fRandUnaligned)
                    pTI->fRandomIO = FALSE;
            }
            else if(achArgBuff[5] == '-' && achArgBuff[6] == 0 && pTI->fRandomIO)
            {
                pTI->fRandomIO = FALSE;
                pTI->fRandRead = FALSE;
                pTI->fRandWrite = FALSE;
                pTI->fRandUnaligned = FALSE;
            }
            else if(achArgBuff[5] == 0)
            {
                pTI->fRandRead = TRUE;
                pTI->fRandWrite = TRUE;
                if(pTI->ulTestEmulationVersion >= DEFAULT_VERSION)
                    pTI->fRandUnaligned = TRUE;
            }
            else
            {
                goto BadOption;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SEQ", 4) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpSequentialTest();
                return FALSE;
            }

            pTI->fSequentialIO = TRUE;

            if(achArgBuff[4] == ':')
            {
                if(DclToUpper(achArgBuff[5]) == 'R')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqRead)
                        pTI->fSeqRead = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqRead = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[5]) == 'W')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqWrite)
                        pTI->fSeqWrite = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqWrite = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[5]) == 'E')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqRewrite)
                        pTI->fSeqRewrite = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqRewrite = TRUE;
                    else
                        goto BadOption;
                }
                else if(DclToUpper(achArgBuff[5]) == 'U')
                {
                    if(achArgBuff[6] == '-' && achArgBuff[7] == 0 && pTI->fSeqUnaligned)
                        pTI->fSeqUnaligned = FALSE;
                    else if(achArgBuff[6] == 0)
                        pTI->fSeqUnaligned = TRUE;
                    else
                        goto BadOption;
                }
                else
                {
                    goto BadOption;
                }

                /*  If all the sequential I/O categories are, or have become,
                    FALSE, turn off the sequential I/O test entirely.
                */
                if(!pTI->fSeqRead && !pTI->fSeqWrite && !pTI->fSeqRewrite)
                    pTI->fSequentialIO = FALSE;
            }
            else if(achArgBuff[4] == '-' && achArgBuff[5] == 0 && pTI->fSequentialIO)
            {
                pTI->fSequentialIO = FALSE;
                pTI->fSeqRead = FALSE;
                pTI->fSeqWrite = FALSE;
                pTI->fSeqRewrite = FALSE;
                pTI->fSeqUnaligned = FALSE;
            }
            else if(achArgBuff[4] == 0)
            {
                pTI->fSeqRead = TRUE;
                pTI->fSeqWrite = TRUE;
                pTI->fSeqRewrite = TRUE;
                pTI->fSeqUnaligned = TRUE;
            }
            else
            {
                goto BadOption;
            }

            continue;
        }
        else if((DclStrNICmp(achArgBuff, "/DIR", 4) == 0) &&
            (DclStrNICmp(achArgBuff, "/DIRS:", 6) != 0))
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpDirStripeTest();
                return FALSE;
            }

            if(achArgBuff[4] == '-' && achArgBuff[5] == 0 && pTI->fDirTest)
                pTI->fDirTest = FALSE;
            else if(achArgBuff[4] == 0)
                pTI->fDirTest = TRUE;
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SCAN", 5) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpDirScanTest();
                return FALSE;
            }

            if(achArgBuff[5] == '-' && achArgBuff[6] == 0 && pTI->fScanTest)
                pTI->fScanTest = FALSE;
            else if(achArgBuff[5] == 0)
                pTI->fScanTest = TRUE;
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SPARSE", 7) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpSparseTest();
                return FALSE;
            }

            if(achArgBuff[7] == '-' && achArgBuff[8] == 0 && pTI->fSparseTest)
                pTI->fSparseTest = FALSE;
            else if(achArgBuff[7] == 0)
                pTI->fSparseTest = TRUE;
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/TREE", 5) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpTreeTest();
                return FALSE;
            }

            if(achArgBuff[5] == '-' && achArgBuff[6] == 0 && pTI->fTreeTest)
                pTI->fTreeTest = FALSE;
            else if(achArgBuff[5] == 0)
                pTI->fTreeTest = TRUE;
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/WEAR", 5) == 0)
        {
            if((DclArgRetrieve(pszCmdLine, (D_UINT16)(uIndex+1), sizeof(achArgHelp), achArgHelp)
                && (DclStrCmp(achArgHelp, "/?") == 0)))
            {
                HelpWearLevelingTest();
                return FALSE;
            }

            pTI->fWearLeveling = TRUE;

            if(achArgBuff[5] == ':')
            {
                char   *pStr;

                pTI->nWearLevStaticPercent = DclAtoI(&achArgBuff[6]);
                if(pTI->nWearLevStaticPercent > MAX_WEARLEVSTATICPERCENT)
                {
                    DclPrintf("The /WEAR:n static percent value must range from 0 to %u\n\n", MAX_WEARLEVSTATICPERCENT);
                    return FALSE;
                }

                pStr = &achArgBuff[6];
                while(DclIsDigit(*pStr))
                    pStr++;

                if(*pStr != 0)
                    goto BadSyntax;
            }
            else
            {
                if(achArgBuff[5] != 0)
                    goto BadSyntax;
            }

            continue;
        }

                /*-----------------------------------------*\
                 *                                         *
                 *    Test type args complete.             *
                 *    Process optional arguments next.     *
                 *                                         *
                \*-----------------------------------------*/

        else if(DclStrNICmp(achArgBuff, "/RANDFOW:", 9) == 0)
        {
            pTI->iFlushOnWriteRatio = DclAtoL(&achArgBuff[9]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/RANDPASS:", 10) == 0)
        {
            const char *pStr = &achArgBuff[10];
            
            pTI->nRandomReadPasses = DclAtoL(pStr);

            while(pStr && DclIsDigit(*pStr))
                pStr++;

            if(*pStr != ':')
                goto BadSyntax;

            pTI->nRandomWritePasses = DclAtoL(++pStr);
            
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/NODEL") == 0)
        {
            pTI->fNoDelete = TRUE;
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/MISALIGN") == 0)
        {
            pTI->fMisalign = TRUE;
            continue;
        }
        else if(DclStrICmp(achArgBuff, "/??") == 0)
        {
            DisplayTestDescription(pTI->sti.hDclInst);
            return FALSE;
        }
        else if(DclStrNICmp(achArgBuff, "/DIRS:", 6) == 0)
        {
            pTI->ulDirCount = DclAtoL(&achArgBuff[6]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FILES:", 7) == 0)
        {
            pTI->ulFileCount = DclAtoL(&achArgBuff[7]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/SFILES:", 8) == 0)
        {
            pTI->ulScanCount = DclAtoL(&achArgBuff[8]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FPAIRS:", 8) == 0)
        {
            pTI->ulFragMaxPairs = DclAtoL(&achArgBuff[8]);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/MAX:", 5) == 0)
        {
            const char *pTmp;

            pTmp = DclSizeToUL(&achArgBuff[5], &pTI->ulMaxDiskSpace);
            if(!pTmp || *pTmp)
                goto BadSyntaxOverflow;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/START", 6) == 0)
        {
            if(achArgBuff[6] == 0)
            {
                ulStart = DEFAULT_SEQ_BASE_BUFFER_SIZE;
                ulIncrement = DEFAULT_SEQ_INCREMENT_FACTOR;
            }
            else if(achArgBuff[6] == ':')
            {
                const char *pTmp;

                pTmp = DclNtoUL(&achArgBuff[7], &ulStart);
                if(!pTmp || (*pTmp && (*pTmp != ',')))
                    goto BadSyntaxOverflow;

                if(*pTmp == ',')
                {
                    pTmp = DclNtoUL(pTmp+1, &ulIncrement);
                    if(!pTmp || *pTmp)
                        goto BadSyntaxOverflow;
                }
                else
                {
                    ulIncrement = DEFAULT_SEQ_INCREMENT_FACTOR;
                }
            }
            else
            {
                goto BadSyntax;
            }

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/VERIFY", 7) == 0)
        {
            /*  For now the "-" option is not documented...
            */
            if(achArgBuff[7] == '-' && achArgBuff[8] == 0 )
                pTI->fNoVerify = TRUE;      /* Don't do verification in (some) places where we normally do */
            else if(achArgBuff[7] == 0)
                pTI->fWriteVerify = TRUE;   /* Do extra verification where we normally do not */
            else
                goto BadOption;

            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/VER:", 5) == 0)
        {
            unsigned nVer = DclAtoI(&achArgBuff[5]);

            switch(nVer)
            {
                case 200:
                    pTI->ulTestEmulationVersion = V200_VERSION;
                    pTI->ulSleepMS              = V200_DEFAULT_SLEEP_MS;
                    pTI->nRandomReadPasses      = V200_DEFAULT_RANDOM_READ_PASSES;
                    pTI->nRandomWritePasses     = V200_DEFAULT_RANDOM_WRITE_PASSES;

                    break;

                case 300:

                    /*  Already initialized at the start of this function...
                    */
                    break;

                default:                    
                    DclPrintf("Unrecognized version: \"%s\" -- expected 200\n\n", achArgBuff);
                    
                    return FALSE;
            }
            
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FSBLOCKSIZE:", 13) == 0)
        {
            pTI->ulFSBlockSize = DclAtoI(&achArgBuff[13]);
            if(pTI->ulFSBlockSize == 0 || !DCLISPOWEROF2(pTI->ulFSBlockSize))
            {
                DclPrintf("The /FSBlockSize value must be a power-of-two value greater than 0.\n\n");
                return FALSE;
            }

            DclPrintf("Overriding the default FS Block Size with that specified on the command-line (%lU)\n", pTI->ulFSBlockSize);
            continue;
        }
        else if(DclStrNICmp(achArgBuff, "/FSDEVNAME:", 11) == 0)
        {
            DclStrNCpy(szFSDevName, &achArgBuff[11], sizeof(szFSDevName));

            continue;
        }
        else
        {
            /*  Process common test arguments which are shared by
                multiple tests.
            */
            if(DclTestParseParam(&pTI->sti, achArgBuff))
                continue;
        }
        
        uSwitch = 1;

        if(achArgBuff[uSwitch-1] == '/')
        {
            switch(achArgBuff[uSwitch])
            {
                case '?':
                    ShowHelp(pTI->sti.hDclInst, pszCmdName);
                    return FALSE;

                case 'b':
                case 'B':
                {
                    D_UINT32        ulBuffSize;
                    const char     *pTmp;

                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    pTmp = &achArgBuff[uSwitch + 2];

                    pTmp = DclSizeToUL(pTmp, &ulBuffSize);
                    if(!pTmp || *pTmp)
                        goto BadOption;

                    pTI->ulBufferSize = ulBuffSize;

                    break;
                }

                case 'i':
                case 'I':
                    if(achArgBuff[uSwitch + 1] != ':')
                        pTI->ulCount = D_UINT32_MAX;
                    else
                        pTI->ulCount = DclAtoL(&achArgBuff[uSwitch + 2]);
                    break;

                case 'r':
                case 'R':
                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    pTI->nSampleRate = DclAtoI(&achArgBuff[uSwitch + 2]);

                    if(pTI->nSampleRate > MAX_SAMPLE_RATE)
                    {
                        DclPrintf("Using the maximum sample rate of %u seconds\n", MAX_SAMPLE_RATE);
                        pTI->nSampleRate = MAX_SAMPLE_RATE;
                    }
                    
                    break;

                case 's':
                case 'S':
                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    if(DclStrICmp(&achArgBuff[uSwitch + 2], "LONG") == 0)
                        pTI->ulSleepMS = DEFAULT_LONG_SLEEP_MS;
                    else if(DclStrICmp(&achArgBuff[uSwitch + 2], "SHORT") == 0)
                        pTI->ulSleepMS = DEFAULT_SHORT_SLEEP_MS;
                    else
                        pTI->ulSleepMS = DclAtoL(&achArgBuff[uSwitch + 2]);

                    break;

                case 'p':
                case 'P':
                    if(achArgBuff[uSwitch + 1] != ':')
                        goto BadOption;

                    if((achArgBuff[uSwitch + 2] == '"') || (achArgBuff[uSwitch + 2] == '\''))
                    {
                         DclStrNCpy(pTI->szPath, &achArgBuff[uSwitch + 3],
                             DclStrLen(&achArgBuff[uSwitch + 3]) - 1);
                    }
                    else
                    {
                         DclStrNCpy(pTI->szPath, &achArgBuff[uSwitch + 2],
                             DclStrLen(&achArgBuff[uSwitch + 2]));
                    }
                    break;

                default:
                    goto BadOption;
            }
        }
        else
        {
  BadOption:
            DclPrintf("Bad option: \"%s\"\n\n", &achArgBuff[uSwitch-1]);
            return FALSE;
  BadSyntax:
            DclPrintf("Syntax error in '%s'\n\n", &achArgBuff[0]);
            return FALSE;
  BadSyntaxOverflow:
            DclPrintf("Syntax or overflow error in '%s'\n\n", &achArgBuff[0]);
            return FALSE;
        }
    }

    if( !pTI->fRandomIO      &&
        !pTI->fCreate        &&
        !pTI->fFrag          &&
        !pTI->fFillVolume    &&
        !pTI->fSequentialIO  &&
        !pTI->fWearLeveling  &&
        !pTI->fScanTest      &&
        !pTI->fSparseTest    &&
        !pTI->fTreeTest      &&
        !pTI->fDirTest)
    {
        DclPrintf("One or more test types must be specified.\n\n");
        return FALSE;
    }

    if(!InternalStatFS(pTI, &pTI->statfs))
    {
        DclPrintf("Unable to get file system information (try using the \"/P:path\" option).\n\n");
        return FALSE;
    }

    pTI->sti.pszDeviceName = &pTI->statfs.szDeviceName[0];

    /*  If not specified on the command-line, set our local block size value
        to match that returned by statfs.
    */
    if(!pTI->ulFSBlockSize)
    {
        pTI->ulFSBlockSize = pTI->statfs.ulBlockSize;
    }
     
    if(szFSDevName[0])
    {
        DclPrintf("Overriding the default FS Device Name \"%s\" with that specified on the command-line \"%s\"\n",
            pTI->statfs.szDeviceName, szFSDevName);

        DclStrNCpy(pTI->statfs.szDeviceName, szFSDevName, sizeof(pTI->statfs.szDeviceName));
    }

    if(!pTI->ulMaxDiskSpace)
    {
        if(pTI->statfs.ulBlockSize && pTI->statfs.ulFreeBlocks)
        {
            pTI->ulMaxDiskSpace = pTI->statfs.ulBlockSize * pTI->statfs.ulFreeBlocks;
            pTI->ulMaxDiskSpace /= DEFAULT_MAXDISKSPACEDIVISOR;

            /*  Round the size up for the following ranges:
                    <  512KB -- round up to the next 128KB boundary
                    <  1MB   -- round up to the next 256KB boundary
                    <  2MB   -- round up to the next 512KB boundary
                    <  4MB   -- round up to the next 1MB boundary
                    <  8MB   -- round up to the next 2MB boundary
                    >= 8MB   -- round up to the next 4MB boundary
            */
            pTI->ulMaxDiskSpace = BoundaryRoundUp(pTI->ulMaxDiskSpace, 512*1024UL, 8192*1024UL);
        }
        else
        {
            pTI->ulMaxDiskSpace = DEFAULT_DISK_SPACE;
        }
    }

    if(!pTI->ulBufferSize)
    {
         pTI->ulBufferSize = pTI->ulMaxDiskSpace / DEFAULT_BUFFERSIZEDIVISOR;

         if(pTI->ulBufferSize < pTI->statfs.ulBlockSize)
             pTI->ulBufferSize = pTI->statfs.ulBlockSize;

        /*  Round the size up for the following ranges:
                <  512B -- round up to the next 128B boundary
                <  1KB  -- round up to the next 256B boundary
                <  2KB  -- round up to the next 512B boundary
                <  4KB  -- round up to the next 1KB boundary
                <  8KB  -- round up to the next 2KB boundary
                >= 8KB  -- round up to the next 4KB boundary
        */
        pTI->ulBufferSize = BoundaryRoundUp(pTI->ulBufferSize, 512, 8192);

        if(pTI->ulBufferSize > MAX_BUFFER_SIZE)
            pTI->ulBufferSize = MAX_BUFFER_SIZE;
    }

    if(!pTI->ulDirCount)
        pTI->ulDirCount = DEFAULT_DIRS;

    if(!pTI->ulFileCount)
        pTI->ulFileCount = DEFAULT_FILES;

    if(!pTI->ulScanCount)
        pTI->ulScanCount = DEFAULT_SCAN_FILES;

    if(!pTI->sti.ulTestSeconds)
        pTI->sti.ulTestSeconds = DEFAULT_TEST_SECONDS;

    pTI->ulSmallFileSize = pTI->ulFSBlockSize * SMALL_FILE_SIZE_FACTOR;

    if(pTI->fSequentialIO)
    {
        if(ulStart == D_UINT32_MAX)
        {
            /*  If /Start was not specified, just default to the
                buffer size, and a range will not be tested.
            */
            ulStart = pTI->ulBufferSize;
            ulIncrement = 0;
        }

        if(ulStart == DEFAULT_SEQ_BASE_BUFFER_SIZE)
        {
            /*  Use the FS block size as the base for the range.
            */
            ulStart = pTI->statfs.ulBlockSize;

            if(ulStart > pTI->ulBufferSize)
            {
                /*  (Essentially the same error check as below, but with a
                    more descriptive error message.)
                */
                DclPrintf("The /Start option specifies that the FS block size be used as the\n");
                DclPrintf("base value, however this is larger than the /B max block size.\n\n");

                return FALSE;
            }
        }

        if(ulIncrement == 0)
            ulIncrement = DEFAULT_SEQ_INCREMENT_FACTOR;

        if(ulStart > pTI->ulBufferSize)
        {
            DclPrintf("The /Start option cannot specify a value greater than the buffer size %lU\n\n",
                pTI->ulBufferSize);

            return FALSE;
        }

        pTI->ulBufferMin = ulStart;
        pTI->ulBufferInc = ulIncrement;
    }
    else
    {
        if(ulStart != D_UINT32_MAX)
        {
            DclPrintf("The /Start option is only valid with the /SEQ test\n\n");

            return FALSE;
        }
    }

    /*  If a path was not specified, default to the current directory
    */
    if(!pTI->szPath[0])
    {
        DclStrCpy(pTI->szPath, "."DCL_PATHSEPSTR);
    }
    else
    {
        /*  A path was specified, ensure it ends with a path separator
        */
        unsigned nLen = DclStrLen(pTI->szPath);

        if(pTI->szPath[nLen - 1] != DCL_PATHSEPCHAR)
        {
            pTI->szPath[nLen + 0] = DCL_PATHSEPCHAR;
            pTI->szPath[nLen + 1] = 0;
        }
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    BoundaryRoundUp()

    Description
        This function rounds the specified number up so that it falls
        on a boundary defined by the range of ulMin to ulMax.  ulMin
        will be doubled until it exceeds either ulValue or ulMax.
        Then the value would be rounded up to a boundary that 1/4 of
        the "new" ulMin.

        For example, calling BoundaryRoundUp(n, 512, 8192) would
        have the following results:

          n  <  512B -- round up to the next 128B boundary
          n  <  1KB  -- round up to the next 256B boundary
          n  <  2KB  -- round up to the next 512B boundary
          n  <  4KB  -- round up to the next 1KB boundary
          n  <  8KB  -- round up to the next 2KB boundary
          n  >= 8KB  -- round up to the next 4KB boundary

    Parameters
        ulValue - The value to be rounded up.
        ulMin   - The minimum end of the range
        ulMax   - The maximum end of the range.

    Return Value
        Returns the rounded value.
-------------------------------------------------------------------*/
static D_UINT32 BoundaryRoundUp(
    D_UINT32    ulValue,
    D_UINT32    ulMin,
    D_UINT32    ulMax)
{
    DclAssert(ulMin <= ulMax);

    while(ulMin <= ulMax)
    {
        if(ulValue < ulMin)
            break;

        ulMin *= 2;
    }

    ulMin = (ulMin / 4) - 1;

    ulValue = (ulValue + ulMin) & ~ulMin;

    return ulValue;
}


/*-------------------------------------------------------------------
    Local: FillVolume()


    Parameters:
       pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolume(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    D_BOOL          fOldVerify;
    unsigned        nOldSampleRate;

    fOldVerify = pTI->fWriteVerify;
    pTI->fWriteVerify = TRUE;

    nOldSampleRate = pTI->nSampleRate;
    pTI->nSampleRate = DCLMIN(pTI->nSampleRate * 5, MAX_SAMPLE_RATE);
    
    dclStat = FillVolumeBigFile(pTI, 0);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

    dclStat = FillVolumeBigFile(pTI, 7);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

    dclStat = FillVolumeMidSizedFiles(pTI);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

    dclStat = FillVolumeSmallFiles(pTI);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

    dclStat = FillVolumeTinyFiles(pTI);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

    dclStat = FillVolumeZeroByteFiles(pTI);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

    AppSleep(pTI, 6);

  FillCleanup:
    
    /*  Restore the original settings.
    */
    pTI->fWriteVerify = fOldVerify;

    pTI->nSampleRate = nOldSampleRate;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FillVolumeBigFile()

    The following algorithm is used:
    1) Fill the volume using a single large file, using IOBufferSized
       writes.
    2) Once no more can be written, try to append using FS block sized
       writes.  
    3) Close and flush that file.
    4) Write as many zero length files as possible.
    5) Delete any zero length files which were written.
    6) Verify the contents of the large file.
    7) Delete the large file.

    Read and write I/O on the large file may be done with aligned or
    unaligned client buffers and file offsets, based on whether the 
    nOffset parameter is zero or not.  If it is zero, both client 
    memory buffers and file I/O offsets will be natively and FS block
    size aligned, respectively.  If it is non-zero, client memory 
    buffers will be misaligned by 1, and the file I/O offsets will 
    be off by the value of nOffset.

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.
        nOffset - The offset to use to force misaligned I/O.

    Return Value
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeBigFile(
    FSIOTESTINFO   *pTI,
    unsigned        nOffset)
{
    DCLFSFILEHANDLE hTestFile;
    unsigned        nExtra = 0;
    DCLSTATUS       dclStat;
    D_UINT32        ulTotalWritten = 0;
    D_UINT32        ulLongestTimeUS = 0;
    D_UINT64        ullTotalTimeUS = 0;
    D_UINT64        ullTmpUS;
    D_UINT64        ullTmp;
    D_UINT32        ulCount;
    D_UINT32        ulTotalTimeMS;
    char            szScaleBuff[48];
    DCLFSSTAT       stat;
    DCLTIMESTAMP    ts;       
    unsigned        nCreated;
    unsigned        nDeleted;
    D_UINT32        ulTotalMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_UINT32        ulMaxUS;
    D_UINT32        ulVerified;
    D_BOOL          fAligned;
    D_UINT32        ulResult;

    dclStat = PRIMFLUSH(pTI->hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(nOffset)
        fAligned = FALSE;
    else 
        fAligned = TRUE;
    
    /*  Create the file
    */
    DclSNPrintf(szScaleBuff, sizeof(szScaleBuff), "%lU KB %s I/O...", pTI->ulBufferSize / 1024, fAligned ? "aligned" : "unaligned");
    DclPrintf("  Filling volume with a large file using %-32s %3s%%\n", szScaleBuff, GetFreeSpacePercent(pTI));
 
    hTestFile = CreateTempFile(pTI, "fssf%04X.dat", NULL);
    if(!hTestFile)
        return DCLSTAT_CURRENTLINE;

    if(!fAligned)
    {
        unsigned    nn;
        
        for(nn = 0; nn < nOffset; nn++)
            pTI->pBuffer[nn] = (D_BYTE)nn;

        dclStat = PRIMWRITE(hTestFile, pTI->pBuffer, nOffset, &ulResult);
        if(dclStat != DCLSTAT_SUCCESS || ulResult != nOffset)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;
            
            DclPrintf("Write failed with status %lX\n", dclStat);
            return dclStat;
        }
    }

    dclStat = WriteHelper(pTI, hTestFile, pTI->ulBufferSize, D_UINT32_MAX, !fAligned, nOffset, TRUE, &ulTotalWritten, &ulLongestTimeUS, &ullTotalTimeUS);

    /*  Ensure that everything is on the volume before we try to fill any
        remaining space using smaller writes.
    */
    ullTmpUS = pTI->FSPrim.ullFlushTimeUS;
    PRIMFLUSH(hTestFile);
    ullTotalTimeUS += pTI->FSPrim.ullFlushTimeUS - ullTmpUS;

    ulCount = ulTotalWritten / pTI->ulBufferSize;

    ullTmp = ullTotalTimeUS + 500;
    DclUint64DivUint32(&ullTmp, 1000);
    ulTotalTimeMS = (D_UINT32)ullTmp;

    DclPrintf("    Wrote %8s file ------------------------------>%10lU KB/sec  %3s%%\n", 
        DclScaleItems(ulCount, pTI->ulBufferSize, szScaleBuff, sizeof(szScaleBuff)),
        GetKBPerSecond(ulTotalWritten / 1024, ulTotalTimeMS),
        GetFreeSpacePercent(pTI));
         
    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        ullTmp = ullTotalTimeUS;
        DclUint64DivUint32(&ullTmp, ulCount);

        DclPrintf("    Wrote %lU %lU KB blocks in %lU ms, Avg=%llU us, Long=%lU us\n",
            ulCount, pTI->ulBufferSize / 1024,
            ulTotalTimeMS, VA64BUG(ullTmp), ulLongestTimeUS);
    }

    /*  The write may have been too large to accurately fill the volume.
        We will attempt to write some data with a smaller buffer.  The
        write should work and we will fill the volume up more completely.
    */
    while(TRUE)
    {
        dclStat = PRIMWRITE(hTestFile, pTI->pBuffer + !fAligned, pTI->ulFSBlockSize, &ulResult);
        if(dclStat != DCLSTAT_SUCCESS || ulResult != pTI->ulFSBlockSize)
            break;

        nExtra++;
    }

    DclPrintf("    Fill remaining using %4lU byte writes, wrote ----->%10lU bytes   %3s%%\n", 
        pTI->ulFSBlockSize, pTI->ulFSBlockSize * nExtra, GetFreeSpacePercent(pTI));

    /*  Close and delete the file we have just created
    */

    PRIMFLUSH(hTestFile);

    dclStat = PRIMCLOSE(hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("FSIO: Unable to close file, status=%lX\n", dclStat);
        return dclStat;
    }

    dclStat = PRIMSTAT(pTI->szFileSpec, &stat);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("FSIO: Unable to stat file, status=%lX\n", dclStat);
        return dclStat;
    }

    DclPrintf("    Final large file size ---------------------------->%10lU bytes   %3s%%\n", 
        stat.ulSize, GetFreeSpacePercent(pTI));

    dclStat = CreateFileSet(pTI, "FLZ", &ulTotalMS, NULL, &nCreated, FILESET_MAX,
        0, 0, pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL, 0, UINT_MAX);

    if(nCreated && !pTI->fNoDelete)
    {
        dclStat = DeleteFileSet(pTI, "FLZ", nCreated, &ulDeleteMS, &ulMaxUS, &nDeleted, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }
    }
    
    DclPrintf("    Created and deleted %3U zero length files in ----->%10lU ms      %3s%%\n", 
        nCreated, ulTotalMS + ulDeleteMS, GetFreeSpacePercent(pTI));

    ts = DclTimeStamp();

    dclStat = PRIMOPEN(pTI->szFileSpec, "r+b", &hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = VerifyFileHelper(pTI, hTestFile, &ulVerified, nOffset);

    PRIMCLOSE(hTestFile);

    if(dclStat != DCLSTAT_SUCCESS || ulVerified != stat.ulSize)
    {
        DclPrintf("Verification failed! Status=%lX, Verified=%lU, Size=%lU\n",
            dclStat, ulVerified, stat.ulSize);

        return DCLSTAT_CURRENTLINE;
    }

    DclPrintf("    Verified file contents using %s------->%10lU KB/sec  %3s%%\n", 
        nOffset ? "unaligned I/O " : "aligned I/O --",
        GetKBPerSecond((ulVerified + 512) / 1024, DclTimePassed(ts)), GetFreeSpacePercent(pTI));

    if(pTI->fNoDelete)
        return dclStat;

    ts = DclTimeStamp();

    dclStat = PRIMDELETE(pTI->szFileSpec);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("FSIO: Unable to delete large file, status=%lX\n", dclStat);
        return dclStat;
    }

    DclPrintf("    Large file deletion ------------------------------>%10lU ms      %3s%%\n", 
        DclTimePassed(ts), GetFreeSpacePercent(pTI));

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FillVolumeMidSizedFiles()

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeMidSizedFiles(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    char            szScaleBuff[32];
    unsigned        nTotalDirsA;
    unsigned        nTotalDirsB;
    unsigned        nCreatedZ;
    D_UINT32        ulTotalMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_UINT32        ulMaxUS;
    unsigned        nDeleted;
    D_UINT32        ulTotalFiles;
    D_UINT32        ulTotalDeleted;
    D_UINT64        ullKBWritten;
    D_UINT32        ulStartingFreeBlocks;
    const char     *pszFSPercent;

    if(pTI->ulFillMin > pTI->ulBufferSize)
        return DCLSTAT_SUCCESS;
    
    dclStat = PRIMFLUSH(pTI->hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulStartingFreeBlocks = FreeDiskBlocks(pTI);
    
    /*  Create the file
    */
    DclSNPrintf(szScaleBuff, sizeof(szScaleBuff), "%lU KB files...", pTI->ulBufferSize / 1024);
    DclPrintf("  Filling volume with mid-sized %-41s %3s%%\n", szScaleBuff, GetFreeSpacePercent(pTI));

    dclStat = FillVolumeFileSetCreate(pTI, "FLA", pTI->ulBufferSize, &ulTotalMS, &ulTotalFiles, &nTotalDirsA);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    FSIO: Error creating file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    ullKBWritten = DclMulDiv(ulTotalFiles, pTI->ulBufferSize, 1024);

    DclPrintf("    Created %4lU files containing %8s ----------->%10lU KB/sec  %3s%%\n", 
        ulTotalFiles,
        DclScaleItems(ulTotalFiles, pTI->ulBufferSize, szScaleBuff, sizeof(szScaleBuff)),
        GetKBPerSecond(ullKBWritten, ulTotalMS),
        GetFreeSpacePercent(pTI));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        /*  Calculate per-file media overhead, NOT including the actual file
            data itself.
        */            
        D_UINT32 ulBytesOverhead = (D_UINT32)(((DclMulDiv(ulStartingFreeBlocks - FreeDiskBlocks(pTI), pTI->ulFSBlockSize, 1024)
                                    - ullKBWritten) * 1024) / ulTotalFiles);

        DclPrintf("    Per file metadata overhead ----------------------->%10lU bytes\n", ulBytesOverhead);
    }

    dclStat = FillVolumeFileSetCreate(pTI, "FLB", pTI->ulFSBlockSize, &ulTotalMS, &ulTotalFiles, &nTotalDirsB);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    FSIO: Error creating file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    DclPrintf("    Created %2lU %4lU byte files ----------------------->%10lU KB/sec  %3s%%\n", 
        ulTotalFiles,
        pTI->ulFSBlockSize,
        GetKBPerSecond((ulTotalFiles * pTI->ulFSBlockSize) / 1024, ulTotalMS),
        GetFreeSpacePercent(pTI));

    /*  NOTE: These will be created in the root -- hopefully not so many 
              will be created that it will hit the FS root or single
              directory limit.
    */             
    dclStat = CreateFileSet(pTI, "FLZ", &ulTotalMS, NULL, &nCreatedZ, FILESET_MAX,
        0, 0, pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL, 0, UINT_MAX);

    pszFSPercent = GetFreeSpacePercent(pTI);
    
    if(nCreatedZ && !pTI->fNoDelete)
    {
        dclStat = DeleteFileSet(pTI, "FLZ", nCreatedZ, &ulDeleteMS, &ulMaxUS, &nDeleted, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }
    }
    
    DclPrintf("    Created and deleted %3U zero length files -------->%10lU ms      %3s%%\n", 
        nCreatedZ, ulTotalMS + ulDeleteMS, pszFSPercent);

    if(nTotalDirsB && !pTI->fNoDelete)
    {
        dclStat = FillVolumeFileSetDelete(pTI, "FLB", nTotalDirsB, &ulTotalMS, &ulTotalDeleted);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }

        DclPrintf("    Deleted the %2lU small files in -------------------->%10lU ms      %3s%%\n", 
            ulTotalDeleted, ulTotalMS, GetFreeSpacePercent(pTI));
    }
    
    if(nTotalDirsA && !pTI->fNoDelete)
    {
        dclStat = FillVolumeFileSetDelete(pTI, "FLA", nTotalDirsA, &ulDeleteMS, &ulTotalDeleted);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }

        DclPrintf("    Deleted the %4lU mid-sized files in -------------->%10lU ms      %3s%%\n", 
            ulTotalDeleted, ulDeleteMS, GetFreeSpacePercent(pTI));
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FillVolumeSmallFiles()

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeSmallFiles(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    char            szScaleBuff[32];
    unsigned        nTotalDirsA;
    unsigned        nCreatedZ;
    D_UINT32        ulTotalMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_UINT32        ulMaxUS;
    unsigned        nDeleted;
    D_UINT32        ulTotalFiles;
    D_UINT32        ulTotalDeleted;
    D_UINT64        ullKBWritten;
    D_UINT32        ulStartingFreeBlocks;
    const char     *pszFSPercent;

    if(pTI->ulFillMin > pTI->ulFSBlockSize)
        return DCLSTAT_SUCCESS;
    
    dclStat = PRIMFLUSH(pTI->hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulStartingFreeBlocks = FreeDiskBlocks(pTI);

    /*  Create the file
    */
    DclSNPrintf(szScaleBuff, sizeof(szScaleBuff), "%lU byte files...", pTI->ulFSBlockSize);
    DclPrintf("  Filling volume with small %-45s %3s%%\n", szScaleBuff, GetFreeSpacePercent(pTI));
 
    dclStat = FillVolumeFileSetCreate(pTI, "FLA", pTI->ulFSBlockSize, &ulTotalMS, &ulTotalFiles, &nTotalDirsA);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    FSIO: Error creating file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    ullKBWritten = DclMulDiv(ulTotalFiles, pTI->ulFSBlockSize, 1024);

    DclPrintf("    Created %6lU files containing %9s -------->%10lU KB/sec  %3s%%\n", 
        ulTotalFiles,
        DclScaleItems(ulTotalFiles, pTI->ulFSBlockSize, szScaleBuff, sizeof(szScaleBuff)),
        GetKBPerSecond(ullKBWritten, ulTotalMS),
        GetFreeSpacePercent(pTI));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        /*  Calculate per-file media overhead, NOT including the actual file
            data itself.
        */            
        D_UINT32 ulBytesOverhead = (D_UINT32)(((DclMulDiv(ulStartingFreeBlocks - FreeDiskBlocks(pTI), pTI->ulFSBlockSize, 1024)
                                    - ullKBWritten) * 1024) / ulTotalFiles);

        DclPrintf("    Per file metadata overhead ----------------------->%10lU bytes\n", ulBytesOverhead);
    }
    
    dclStat = CreateFileSet(pTI, "FLZ", &ulTotalMS, NULL, &nCreatedZ, FILESET_MAX,
        0, 0, pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL, 0, UINT_MAX);

    pszFSPercent = GetFreeSpacePercent(pTI);
    
    if(nCreatedZ && !pTI->fNoDelete)
    {
        dclStat = DeleteFileSet(pTI, "FLZ", nCreatedZ, &ulDeleteMS, &ulMaxUS, &nDeleted, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }
    }
    
    DclPrintf("    Created and deleted %3U zero length files -------->%10lU ms      %3s%%\n", 
        nCreatedZ, ulTotalMS + ulDeleteMS, pszFSPercent);

    if(nTotalDirsA && !pTI->fNoDelete)
    {
        dclStat = FillVolumeFileSetDelete(pTI, "FLA", nTotalDirsA, &ulDeleteMS, &ulTotalDeleted);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }

        DclPrintf("    Deleted the %6lU small files in ---------------->%10lU ms      %3s%%\n", 
            ulTotalDeleted, ulDeleteMS, GetFreeSpacePercent(pTI));
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FillVolumeTinyFiles()

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeTinyFiles(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    char            szScaleBuff[32];
    unsigned        nTotalDirsA;
    unsigned        nCreatedZ;
    D_UINT32        ulTotalMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_UINT32        ulMaxUS;
    unsigned        nDeleted;
    D_UINT32        ulTotalFiles;
    D_UINT32        ulTotalDeleted;
    D_UINT32        ulStartingFreeBlocks;
    const char     *pszFSPercent;
    
    if(pTI->ulFillMin > 1)
        return DCLSTAT_SUCCESS;
    
    dclStat = PRIMFLUSH(pTI->hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulStartingFreeBlocks = FreeDiskBlocks(pTI);

    /*  Create the file
    */
    DclPrintf("  Filling volume with tiny 1 byte files...                                %3s%%\n",
        GetFreeSpacePercent(pTI));

    dclStat = FillVolumeFileSetCreate(pTI, "FLA", 1, &ulTotalMS, &ulTotalFiles, &nTotalDirsA);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    FSIO: Error creating file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    DclPrintf("    Created %7lU files containing %11s ----->%10lU ms      %3s%%\n", 
        ulTotalFiles,
        DclScaleItems(ulTotalFiles, 1, szScaleBuff, sizeof(szScaleBuff)),
        ulTotalMS,
        GetFreeSpacePercent(pTI));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        D_UINT64 ullKBWritten = DclMulDiv(ulTotalFiles, pTI->ulFSBlockSize, 1024);

        /*  Calculate per-file media overhead, NOT including the actual file
            data itself.
        */            
        D_UINT32 ulBytesOverhead = (D_UINT32)(((DclMulDiv(ulStartingFreeBlocks - FreeDiskBlocks(pTI), pTI->ulFSBlockSize, 1024)
                                    - ullKBWritten) * 1024) / ulTotalFiles);

        DclPrintf("    Per file metadata overhead ----------------------->%10lU bytes\n", ulBytesOverhead);
    }

    dclStat = CreateFileSet(pTI, "FLZ", &ulTotalMS, NULL, &nCreatedZ, FILESET_MAX,
        0, 0, pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL, 0, UINT_MAX);

    pszFSPercent = GetFreeSpacePercent(pTI);
    
    if(nCreatedZ && !pTI->fNoDelete)
    {
        dclStat = DeleteFileSet(pTI, "FLZ", nCreatedZ, &ulDeleteMS, &ulMaxUS, &nDeleted, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }
    }
    
    DclPrintf("    Created and deleted %3U zero length files -------->%10lU ms      %3s%%\n", 
        nCreatedZ, ulTotalMS + ulDeleteMS, pszFSPercent);

    if(nTotalDirsA && !pTI->fNoDelete)
    {
        dclStat = FillVolumeFileSetDelete(pTI, "FLA", nTotalDirsA, &ulDeleteMS, &ulTotalDeleted);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }

        DclPrintf("    Deleted the %6lU tiny files in ----------------->%10lU ms      %3s%%\n", 
            ulTotalDeleted, ulDeleteMS, GetFreeSpacePercent(pTI));
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FillVolumeZeroByteFiles()

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeZeroByteFiles(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    unsigned        nTotalDirsA;
    D_UINT32        ulTotalMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_UINT32        ulTotalFiles;
    D_UINT32        ulTotalDeleted;
    D_UINT32        ulStartingFreeBlocks;
    
    if(pTI->ulFillMin > 0)
        return DCLSTAT_SUCCESS;
    
    dclStat = PRIMFLUSH(pTI->hFlushFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulStartingFreeBlocks = FreeDiskBlocks(pTI);
    
    /*  Create the file
    */
    DclPrintf("  Filling volume with zero length files...                                %3s%%\n",
        GetFreeSpacePercent(pTI));

    dclStat = FillVolumeFileSetCreate(pTI, "FLZ", 0, &ulTotalMS, &ulTotalFiles, &nTotalDirsA);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            DclPrintf("    FSIO: Error creating file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    DclPrintf("    Created %8lU files --------------------------->%10lU ms      %3s%%\n", 
        ulTotalFiles, ulTotalMS, GetFreeSpacePercent(pTI));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        /*  Calculate per-file media overhead, NOT including the actual file
            data itself.
        */            
        D_UINT32 ulBytesOverhead = (D_UINT32)DclMulDiv(ulStartingFreeBlocks - FreeDiskBlocks(pTI), pTI->ulFSBlockSize, ulTotalFiles);

        DclPrintf("    Per file metadata overhead ----------------------->%10lU bytes\n", ulBytesOverhead);
    }

    if(nTotalDirsA && !pTI->fNoDelete)
    {
        dclStat = FillVolumeFileSetDelete(pTI, "FLZ", nTotalDirsA, &ulDeleteMS, &ulTotalDeleted);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);
            return dclStat;
        }

        DclPrintf("    Deleted the %8lU zero length files ----------->%10lU ms      %3s%%\n", 
            ulTotalDeleted, ulDeleteMS, GetFreeSpacePercent(pTI));
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:
        PTI - A pointer to the FSIOTESTINFO structure to use

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeFileSetCreate(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    D_UINT32        ulFileSize,
    D_UINT32       *pulTotalMS,
    D_UINT32       *pulFilesCreated,
    unsigned       *pnDirsCreated)
{
    DCLSTATUS       dclStat;
    unsigned        nDirNum = 1;
    D_INT32         lBlocks = 1;
 
    /*  The CreateFileSet() requires that the block count be zero if the
        block size is also zero (to allow creating zero-length files).
    */        
    if(ulFileSize == 0)
        lBlocks = 0;

    *pulTotalMS = 0;
    *pulFilesCreated = 0;
    *pnDirsCreated = 0;

    do
    {
        D_UINT32    ulTotalMS;
        unsigned    nCreated;
        DCLSTATUS   dclStat2;
        char        szDirBuff[32];

        if(DclSNPrintf(szDirBuff, sizeof(szDirBuff), "%s_DIR", pszPrefix) <= 0)
            return DCLSTAT_BUFFERTOOSMALL;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("    Creating files in directory \"%s%05u\"...\n", szDirBuff, nDirNum);
        
        dclStat = CreateAndAppendTestDir(pTI, szDirBuff, nDirNum);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            if(dclStat != DCLSTAT_FS_FULL)
                DclPrintf("Error creating directory #%u, Status=%lX\n", nDirNum, dclStat);
            
            break;
        }

        dclStat = CreateFileSet(pTI, pszPrefix, &ulTotalMS, NULL, &nCreated, FILESET_MAX, lBlocks,
                                ulFileSize, pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL, 0, pTI->nSampleRate);

        if(nCreated == 0)
        {
            /*  If we could not create any files in this directory, just
                remove it and pretend it never existed.
            */    
            dclStat2 = DestroyAndRemoveTestDir(pTI);
        }
        else
        {
            dclStat2 = DclTestDirRemoveFromPath(pTI->szPath);
            DclAssert(dclStat2 == DCLSTAT_SUCCESS);

            (*pnDirsCreated)++;
        }

        (void)dclStat2;
        
        *pulTotalMS += ulTotalMS;
        *pulFilesCreated += nCreated;

        if(dclStat != DCLSTAT_SUCCESS)
        {
            if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
                DclPrintf("Error creating %u files in directory #%u, Status=%lX\n", nCreated, nDirNum, dclStat);

            /*  If we could not create any files, get out.
            */
            if(!nCreated)
                break;

            /*  If the FS reports it's full, get out.
            */
            if(FreeDiskBlocks(pTI) == 0)
                break;

            /*  Continue and try a new directory...
            */
        }

        nDirNum++;
    }
    while(TRUE);

    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:
        PTI - A pointer to the FSIOTESTINFO structure to use

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS FillVolumeFileSetDelete(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nDirs,
    D_UINT32       *pulTotalMS,
    D_UINT32       *pulFilesDeleted)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nDirNum = 1;

    *pulTotalMS = 0;
    *pulFilesDeleted = 0;

    for(nDirNum = 1; nDirNum <= nDirs; nDirNum++)
    {
        DCLSTATUS   dclStat2;
        D_UINT32    ulDeleteMS = 0;
        unsigned    nDeleted = 0;
        char        szDirBuff[32];

        if(DclSNPrintf(szDirBuff, sizeof(szDirBuff), "%s_DIR", pszPrefix) <= 0)
            return DCLSTAT_BUFFERTOOSMALL;
        
        dclStat = DclTestDirAppendToPath(pTI->szPath, DCLDIMENSIONOF(pTI->szPath)-1, szDirBuff, nDirNum);
        DclAssert(dclStat == DCLSTAT_SUCCESS);

        DclStrCat(pTI->szPath, DCL_PATHSEPSTR);

        dclStat = DeleteFileSet(pTI, pszPrefix, FILESET_MAX, &ulDeleteMS, NULL, &nDeleted, FALSE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to delete file set, status=%lX\n", dclStat);

            /*  Continue...
            */
        }

        *pulTotalMS += ulDeleteMS;
        *pulFilesDeleted += nDeleted;

        dclStat2 = DestroyAndRemoveTestDir(pTI);        
        if(dclStat2 != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Unable to remove test dir #%u, status=%lX\n", nDirNum, dclStat2);
        }
    };

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SparseTest()


    Parameters:
       pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS SparseTest(
    FSIOTESTINFO   *pTI)
{
    DCLSTATUS       dclStat;
    D_BOOL          fOldVerify;
    unsigned        nOldSampleRate;

    fOldVerify = pTI->fWriteVerify;
    pTI->fWriteVerify = TRUE;

    nOldSampleRate = pTI->nSampleRate;
    pTI->nSampleRate = DCLMIN(pTI->nSampleRate * 5, MAX_SAMPLE_RATE);
    
    dclStat = SparseLimits(pTI);
    if(dclStat != DCLSTAT_SUCCESS)
        goto FillCleanup;

  FillCleanup:
    
    /*  Restore the original settings.
    */
    pTI->fWriteVerify = fOldVerify;

    pTI->nSampleRate = nOldSampleRate;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SparseLimits()

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS SparseLimits(
    FSIOTESTINFO   *pTI)
{
    #define         MAX_SIZES   (16)
    DCLSTATUS       dclStat;
    DCLFSSTATFS     statFS;
    char            szScaleBuff[32];
    D_UINT32        aulPositions[MAX_SIZES] = {0};
    char           *pszDescription[MAX_SIZES];
    D_UINT32        ulLastSize = 0;

    dclStat = PRIMSTATFS(pTI->szPath[0] ? pTI->szPath : ".", &statFS);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error: DclFsStatFs(%s) failed with status %lX\n", pTI->szPath[0] ? pTI->szPath : ".", dclStat);
        return dclStat;
    }

    DclPrintf("  File System Characteristics:\n");
    DclPrintf("    Maximum name length ------------------------------>%10u bytes\n", statFS.nMaxNameLen);
    DclPrintf("    Maximum path length ------------------------------>%10u bytes\n", statFS.nMaxPathLen);
    DclPrintf("    Maximum file size -------------------------------->%10lU bytes (%s)\n", 
        statFS.ulMaxFileSize, statFS.ulMaxFileSize ? DclScaleBytes(statFS.ulMaxFileSize, szScaleBuff, sizeof(szScaleBuff)) : "Unknown");
    DclPrintf("    Volume total size -------------------------------->%13s\n", 
        DclScaleItems(statFS.ulTotalBlocks, statFS.ulBlockSize, szScaleBuff, sizeof(szScaleBuff)));
    DclPrintf("    Volume free space -------------------------------->%13s\n", 
        DclScaleItems(statFS.ulFreeBlocks, statFS.ulBlockSize, szScaleBuff, sizeof(szScaleBuff)));

    /*  Build a table of the various <possible> combinations we want to 
        attempt.  Duplicates will be automatically skipped.
    */        
    aulPositions[0] = D_UINT32_MAX;
    pszDescription[0] = "Max Seek + 1";
    aulPositions[1] = D_UINT32_MAX-1;
    pszDescription[1] = "Max Seek";
    aulPositions[2] = D_UINT32_MAX / 2;
    pszDescription[2] = "Max Seek / 2";
    aulPositions[3] = DCLMIN(D_UINT32_MAX, statFS.ulTotalBlocks * statFS.ulBlockSize * 2);
    pszDescription[3] = "Volume Size * 2";
    aulPositions[4] = DCLMIN(D_UINT32_MAX, statFS.ulTotalBlocks * statFS.ulBlockSize);
    pszDescription[4] = "Volume Size";
    aulPositions[5] = DCLMIN(D_UINT32_MAX, statFS.ulFreeBlocks * statFS.ulBlockSize * 2);
    pszDescription[5] = "Free Space * 2";
    aulPositions[6] = DCLMIN(D_UINT32_MAX, statFS.ulFreeBlocks * statFS.ulBlockSize);
    pszDescription[6] = "Free Space";
    aulPositions[7] = DCLMIN(D_UINT32_MAX, (statFS.ulFreeBlocks * statFS.ulBlockSize) / 2);
    pszDescription[7] = "Free Space / 2";
    if(statFS.ulMaxFileSize)
    {
        aulPositions[8] = statFS.ulMaxFileSize;
        pszDescription[8] = "Max File Size";
        aulPositions[9] = statFS.ulMaxFileSize / 2;
        pszDescription[9] = "Half Max File Size";
    }
    
    while(TRUE)
    {
        unsigned    nBest = UINT_MAX;
        unsigned    nPos;
        unsigned    nCreated = 0;

        /*  Find the largest candidate, ignoring any empty slots or 
            duplicated values.
        */    
        for(nPos = 0; nPos < DCLDIMENSIONOF(aulPositions); nPos++)
        {
            /*  Continue if invalid entry
            */
            if(aulPositions[nPos] == 0)
                continue;

            /*  If this value is a duplicate of what we just did, zap it
                and keep looking.
            */    
            if(aulPositions[nPos] == ulLastSize)
            {
                aulPositions[nPos] = 0;
                continue;
            }

            /*  First time through, whatever we found is the best
            */
            if(nBest == UINT_MAX)
            {
                nBest = nPos;
                continue;
            }

            /*  Recore the largest qualified value we find.
            */
            if(aulPositions[nPos] > aulPositions[nBest])
                nBest = nPos;
        }

        /*  If we couldn't find anything else to do, we're done
        */
        if(nBest == UINT_MAX)
        {
            dclStat = DCLSTAT_SUCCESS;
            break;
        }
        
        ulLastSize = aulPositions[nBest];

        dclStat = SparseExtendTest(pTI, ulLastSize, pszDescription[nBest], &nCreated);

        if(pTI->fNoDelete && nCreated)
            return dclStat;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: SparseExtendTest()

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS SparseExtendTest(
    FSIOTESTINFO   *pTI,
    D_UINT32        ulFileSize,
    const char     *pszDescription,
    unsigned       *pnCreated)
{
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;  /* Init'ed for picky compilers */
    DCLSTATUS       dclStat2;
    unsigned        nFileNum = 0;
    unsigned        nSampleNum = 1;
    D_UINT32        ulSampleFiles = 0;
    D_UINT32        ulSampleMS;
    D_UINT64        ullSampleBytes = 0;     /* Not necessary, but some compilers seem to think so... */
    DCLTIMESTAMP    tsSample = DCLINITTS;   /* Not necessary, but some compilers seem to think so... */
    char            szScaleBuff[32];
    
    DclPrintf("  Creating a file and extending it to %lU+1 bytes (%s)\n", ulFileSize, pszDescription);

    while(TRUE)
    {
        DCLFSFILEHANDLE hFile;
        char            szIOFile[MAX_FILESPEC_LEN];
/*      D_BOOL          fSuccess = FALSE; */
        
        ulSampleFiles++;

        /*  If starting a new sample...
        */
        if(ulSampleFiles == 1)
        {
            ullSampleBytes = 0;
            tsSample = DclTimeStamp();
        }
        
        hFile = CreateNumberedFile(pTI, szIOFile, "fssp", nFileNum + 1);
        if(!hFile)
        {
            /*  If we could not create the first file, something is horribly 
                wrong...
            */    
            if(nFileNum == 0)
                dclStat = DCLSTAT_CURRENTLINE;


            /*  So long as we could process at least one file, dclStat
                will be "success" at this point, and the test is 
                considered to have "passed".
            */    
            break;
        }

        /*  Seek to the specified position
        */
        dclStat = PRIMSEEK(hFile, ulFileSize, DCLFSFILESEEK_SET);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            size_t  nWritten;
            
            if(nFileNum == 0)
                DclPrintf("    Successfully set position to the %lU boundary\n", ulFileSize);

            dclStat = PrimWrite(pTI, pTI->pBuffer, 1, 1, hFile, &nWritten);
            if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
            {
                if(dclStat == DCLSTAT_SUCCESS)
                    dclStat = DCLSTAT_FS_WRITEFAILED;
                    
                if(nFileNum == 0)
                    DclPrintf("      Write at that location failed with status %lX\n", dclStat);
            }
            else
            {
                if(nFileNum == 0)
                {
                    DclPrintf("    1-byte write worked, new free disk space is %lUKB\n",
                        (FreeDiskBlocks(pTI) * pTI->statfs.ulBlockSize) / 1024);
                }
                
                dclStat = PRIMFLUSH(hFile);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclPrintf("Flush failed with status %lX\n", dclStat);
                    DclError();
                }
                else
                {
                    /*  Separate ops to get the new total, since ulFileSize
                        might overflow otherwise.
                    */    
                    ullSampleBytes += ulFileSize;
                    ullSampleBytes++;
                    
/*                  fSuccess = TRUE; */
                }
            }            
        }
        else
        {
            DclPrintf("    Seek failed with status %lX\n", dclStat);
        }

        dclStat2 = PRIMCLOSE(hFile);
        if(dclStat2 != DCLSTAT_SUCCESS)
        {
            DclPrintf("Close failed with status %lX\n", dclStat2);
            DclError();
            return dclStat2;
        }

        nFileNum++;
        
        ulSampleMS = DclTimePassed(tsSample);

        /*  If sampling is enabled, and if sampling on every file, OR 
            our sample time has come...
        */
        if( pTI->nSampleRate != UINT_MAX && 
            ((pTI->nSampleRate == 0) || (ulSampleMS >= pTI->nSampleRate * 1000) || dclStat != DCLSTAT_SUCCESS) )
        {
            D_UINT32    ulFiles = ulSampleFiles;

            if(dclStat != DCLSTAT_SUCCESS)
                ulFiles--;
            
            if(ulFiles && pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("      Sample %3u: Created %4lU files in %5lUms ------>%10lU KB/sec  %3s%%\n",
                    nSampleNum, ulFiles, ulSampleMS,
                    GetKBPerSecond((ullSampleBytes + 512) / 1024, ulSampleMS),
                    GetFreeSpacePercent(pTI));
            }
            
            nSampleNum++;

            /*  Resetting ulSampleBlocks to zero will cause all
                the counts to be reset.
            */
            ulSampleFiles = 0;
        }

        if(dclStat != DCLSTAT_SUCCESS)
            break;

        if(nFileNum == 1)
        {
            DclPrintf("    Creating similar files until the volume is full...         FreeSpace: %3s%%\n",
                GetFreeSpacePercent(pTI));
        }
    }

    if(nFileNum > 1)
    {
        DclPrintf("      Created a file set containing %u files and %s\n", 
            nFileNum, DclScaleItems( ((D_UINT64)ulFileSize) + 1, nFileNum, szScaleBuff, sizeof(szScaleBuff)));

        *pnCreated = nFileNum;
    }
    
    if(!pTI->fNoDelete && nFileNum)
    {
        unsigned    nFile = nFileNum;
        
        /*  Initializing this only because some compilers are not
            very smart.   Not technically needed.
        */
        dclStat2 = DCLSTAT_SUCCESS;
        
        DclPrintf("      Deleting file set...\n");

        tsSample = DclTimeStamp();
        
        for(; nFile > 0; nFile--)
        {
            dclStat2 = DeleteNumberedFile(pTI, "fssp", nFile);
            if(dclStat2 != DCLSTAT_SUCCESS)
            {
                DclPrintf("Delete for file %u failed with status %lX\n", nFile, dclStat2);
                DclError();
                break;
            }
        }

        DclPrintf("        Deleted %6u files ------------------------->%10lU ms      %3s%%\n",
            nFileNum, DclTimePassed(tsSample), GetFreeSpacePercent(pTI));

        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;
            
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Parameters:
       PTI - A pointer to the FSIOTESTINFO structure to use

    Notes:
       This function performs random reads.

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS RandomRead(
    FSIOTESTINFO   *pTI)
{
    DCLFSFILEHANDLE hTestFile;
    D_UINT32        ulOffset;
    D_UINT32        ulBytesRead = 0;
    D_UINT32        ulKBRead;
    DCLTIMESTAMP    ts;
    D_UINT64        ullStartReadUS;
    D_UINT32        ulCount = 0;
    D_UINT32        ulElapsedTotalMS = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szWorkFile[MAX_FILESPEC_LEN];
    D_UINT32        ulPassChunks;
    D_UINT32        ulOps;
    char            szScaleBuff[16];
    char            szScaleBuff2[16];
    unsigned        nPassOps = 0;

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  Randomly reading %lU bytes from a %s file, until %s has been read...\n",
            pTI->ulFSBlockSize,
            DclScaleKB(pTI->ulMaxDiskSpace / 1024, szScaleBuff, sizeof(szScaleBuff)),
            DclScaleKB((pTI->ulMaxDiskSpace / 1024) * pTI->nRandomReadPasses, szScaleBuff2, sizeof(szScaleBuff2)));
    }
    
    dclStat = CreateUncachedFile(pTI, "fsrr%04X.dat", pTI->ulMaxDiskSpace, szWorkFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMOPEN(szWorkFile, "r+b", &hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulOffset = 0;

    ulPassChunks = pTI->ulMaxDiskSpace / pTI->ulFSBlockSize;

    ulOps = ulPassChunks * pTI->nRandomReadPasses;

    DclTestInstrumentationStart(&pTI->sti);

    ts = DclTimeStamp();

    /*  Read until we've read everything the prescribed number of times...
    */
    while(ulCount != ulOps)
    {
        nPassOps++;
        
        /*  Calculate the offset into the file where we will read next
        */
        ulOffset = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % ulPassChunks);
        ulOffset *= pTI->ulFSBlockSize;

        dclStat = PRIMSEEK(hTestFile, ulOffset, DCLFSFILESEEK_SET);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Unable to seek, Status=%lX\n", dclStat);
            goto RRCleanup;
        }

        if(pTI->fWriteVerify)
            DclMemSet(pTI->pBuffer, FILL_VALUE + 1, pTI->ulFSBlockSize);

        ullStartReadUS = pTI->FSPrim.ullReadTimeUS;

        dclStat = PRIMREAD(hTestFile, pTI->pBuffer, pTI->ulFSBlockSize, NULL);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Unable to read %U bytes at offset %lU, Status=%lX\n", 
                pTI->ulFSBlockSize, ulOffset, dclStat);

            goto RRCleanup;
        }

        ulBytesRead += pTI->ulFSBlockSize;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        {
            DclPrintf("    Read  %5U bytes at offset %8lU in %5llU us\n",
                pTI->ulFSBlockSize, ulOffset, VA64BUG(pTI->FSPrim.ullReadTimeUS - ullStartReadUS));
        }

        if(pTI->fWriteVerify)
        {
            unsigned    nIndex;

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                DclPrintf("    Verifying data read\n");

            for(nIndex = 0; nIndex < pTI->ulFSBlockSize; nIndex++)
            {
                if(pTI->pBuffer[nIndex] != FILL_VALUE)
                {
                    DclPrintf("    FSIO: Readback verify failed at index %u!\n", nIndex);
                    dclStat = DCLSTAT_CURRENTLINE;
                    goto RRCleanup;
                }
            }
        }

        ulCount++;

        if(ulCount % ulPassChunks == 0)
        {
            D_UINT32 ulPassMS = DclTimePassed(ts);

            if(pTI->ulTestEmulationVersion < DEFAULT_VERSION || pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Pass %2lU: Performed %u ops in %6lU ms for %4lU KB/sec\n",
                    ulCount / ulPassChunks, nPassOps, ulPassMS,
                    GetKBPerSecond((ulPassChunks * pTI->ulFSBlockSize) / 1024, ulPassMS));
            }
            
            ulElapsedTotalMS += ulPassMS;
            nPassOps = 0;

            ts = DclTimeStamp();
        }
    }

  RRCleanup:
    DclTestInstrumentationStop(&pTI->sti, "RandR");

    /*  Close and delete the file we have just used (preserve original dclStat value)
    */
    if(PRIMCLOSE(hTestFile) != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(!pTI->fNoDelete && PRIMDELETE(szWorkFile) != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    ulKBRead = (ulBytesRead + 512) / 1024;

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Read %lU times for a total of %s in %lU ms\n",
            ulCount,
            DclScaleKB(ulKBRead, szScaleBuff, sizeof(szScaleBuff)),
            ulElapsedTotalMS);
    }
/*
    DclPrintf("    Read  %8lUKB in %lU ms ---> %lU KB/sec\n",
              ulKBRead, ulElapsedReadMS, GetKBPerSecond(ulKBRead, ulElapsedReadMS));
    DclPrintf("    Seeked  %8lU times in %lU ms -- %lU ms each\n",
              ulCount, ulElapsedSeekMS, ulElapsedSeekMS / ulCount);
    DclPrintf("    Total Time Elapsed: %lU ms\n", ulElapsedTotalMS);
*/
    DclPrintf("  Small aligned reads -------------------------------->%10lU KB/sec\n",
        GetKBPerSecond(ulKBRead, ulElapsedTotalMS));

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "RandomRead", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB", pTI->ulMaxDiskSpace / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen", pTI->ulFSBlockSize);
        DCLPERFLOG_NUM(   hPerfLog, "OpCount",   ulCount);
        DCLPERFLOG_NUM(   hPerfLog, "TotalRead", ulKBRead);
        DCLPERFLOG_NUM(   hPerfLog, "KB/second", GetKBPerSecond(ulKBRead, ulElapsedTotalMS));
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:
       PTI - A pointer to the FSIOTESTINFO structure to use

    Notes:
       This function tests random write functionality

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS RandomWrite(
    FSIOTESTINFO   *pTI)
{
    DCLFSFILEHANDLE hTestFile;
    D_UINT32        ulOffset;
    D_UINT32        ulBytesWritten = 0;
    D_UINT32        ulKBWritten;
    DCLTIMESTAMP    ts;
    D_UINT64        ullStartWriteUS;
    D_UINT32        ulCount = 0;
    D_UINT32        ulFlushes = 0;
    D_UINT32        ulElapsedTotalMS = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szWorkFile[MAX_FILESPEC_LEN];
    D_UINT32        ulPassChunks;
    D_UINT32        ulOps;
    char            szScaleBuff[16];
    char            szScaleBuff2[16];
    D_UINT32        ulFlushNext = 0;
    unsigned        nPassOps = 0;

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  Randomly writing %lU bytes to a populated %s file, until %s has been written...\n",
            pTI->ulFSBlockSize,
            DclScaleKB(pTI->ulMaxDiskSpace / 1024, szScaleBuff, sizeof(szScaleBuff)),
            DclScaleKB((pTI->ulMaxDiskSpace / 1024) * pTI->nRandomWritePasses, szScaleBuff2, sizeof(szScaleBuff2)));

        if(pTI->iFlushOnWriteRatio > 0)
            DclPrintf("  Flushing after every %u writes\n", pTI->iFlushOnWriteRatio);
        else if(pTI->iFlushOnWriteRatio < 0)
            DclPrintf("  Flushing randomly but always once %u writes have been done\n", -pTI->iFlushOnWriteRatio);
    }
    
    dclStat = CreateUncachedFile(pTI, "fsrw%04X.dat", pTI->ulMaxDiskSpace, szWorkFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMOPEN(szWorkFile, "r+b", &hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulPassChunks = pTI->ulMaxDiskSpace / pTI->ulFSBlockSize;

    DclMemSet(pTI->pBuffer, FILL_VALUE + 1, pTI->ulFSBlockSize);

    ulOps = ulPassChunks * pTI->nRandomWritePasses;

    if(pTI->iFlushOnWriteRatio > 0)
        ulFlushNext = pTI->iFlushOnWriteRatio;
    else if(pTI->iFlushOnWriteRatio < 0)
        ulFlushNext = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % (-pTI->iFlushOnWriteRatio)) + 1;

    DclTestInstrumentationStart(&pTI->sti);

    ts = DclTimeStamp();

    ulOffset = 0;

    /*  Write until we've written everything the prescribed number of times...
    */
    while(ulCount != ulOps)
    {
        size_t  nWritten;
        
        nPassOps++;
        
        /*  Calculate the offset into the file where we will write next
        */
        ulOffset = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % ulPassChunks);
        ulOffset *= pTI->ulFSBlockSize;

        dclStat = PRIMSEEK(hTestFile, ulOffset, DCLFSFILESEEK_SET);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Unable to seek, Status=%lX\n", dclStat);
            goto RWCleanup;
        }

        if(pTI->fWriteVerify)
            DclMemSet(pTI->pBuffer, FILL_VALUE + 1, pTI->ulFSBlockSize);

        ullStartWriteUS = pTI->FSPrim.ullWriteTimeUS;

        dclStat = PrimWrite(pTI, pTI->pBuffer, pTI->ulFSBlockSize, 1, hTestFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;

            DclPrintf("    FSIO: Unable to write %U bytes at offset %lU with status %lX\n", 
                pTI->ulFSBlockSize, ulOffset, dclStat);
            
            goto RWCleanup;
        }

        ulBytesWritten += pTI->ulFSBlockSize;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
        {
            DclPrintf("    Wrote %5U bytes at offset %8lU in %5llU us\n",
                pTI->ulFSBlockSize, ulOffset, VA64BUG(pTI->FSPrim.ullWriteTimeUS - ullStartWriteUS));
        }

        if(ulCount && ulCount == ulFlushNext)
        {
            D_UINT64 ullStartFlushUS = pTI->FSPrim.ullFlushTimeUS;
            
            PRIMFLUSH(hTestFile);
            ulFlushes++;

            if(pTI->iFlushOnWriteRatio > 0)
                ulFlushNext += pTI->iFlushOnWriteRatio;
            else if(pTI->iFlushOnWriteRatio < 0)
                ulFlushNext += (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % (-pTI->iFlushOnWriteRatio)) + 1;

            if(ulFlushNext && pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Flushed after write %lU, requiring %llU us, next flush after %lU more writes\n", 
                    ulCount, VA64BUG(pTI->FSPrim.ullFlushTimeUS - ullStartFlushUS), ulFlushNext - ulCount);
            }
        }

        if(pTI->fWriteVerify)
        {
            unsigned    nIndex;

            if(pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
                DclPrintf("    Verifying data written\n");

            dclStat = PRIMSEEK(hTestFile, -(long)pTI->ulFSBlockSize, DCLFSFILESEEK_CUR);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    FSIO: Unable to seek to specified offset, Status=%lX\n", dclStat);
                goto RWCleanup;
            }

            DclMemSet(pTI->pBuffer, FILL_VALUE + 2, pTI->ulFSBlockSize);

            dclStat = PRIMREAD(hTestFile, pTI->pBuffer, pTI->ulFSBlockSize, NULL);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    FSIO: Unable to read from file, Status=%lX\n", dclStat);
                goto RWCleanup;
            }

            for(nIndex = 0; nIndex < pTI->ulFSBlockSize; nIndex++)
            {
                if(pTI->pBuffer[nIndex] != FILL_VALUE + 1)
                {
                    DclPrintf("    FSIO: Readback verify failed at offset %u!\n", nIndex);
                    dclStat = DCLSTAT_CURRENTLINE;
                    goto RWCleanup;
                }
            }
        }

        ulCount++;

        if(ulCount % ulPassChunks == 0)
        {
            D_UINT32 ulPassMS = DclTimePassed(ts);

            /*  ToDo: Should consider whether we should always flush at this
                      point to ensure more accurate pass statistics.
            */                      

            if(pTI->ulTestEmulationVersion < DEFAULT_VERSION || pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Pass %2lU: Performed %u ops in %6lU ms for %4lU KB/sec\n",
                    ulCount / ulPassChunks, nPassOps, ulPassMS,
                    GetKBPerSecond((ulPassChunks * pTI->ulFSBlockSize) / 1024, ulPassMS));
            }
            
            ulElapsedTotalMS += ulPassMS;
            nPassOps = 0;

            ts = DclTimeStamp();
        }
        
    }

    /*  Make sure everything we wrote is actually on disk before we
        stop timing.
    */
    PRIMFLUSH(hTestFile);
    ulFlushes++;

    /*  Do the close prior to stopping timing so we can be more certain
        that the data really is on the disk.
    */
    dclStat = PRIMCLOSE(hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    hTestFile = NULL;

    /*  Must process any partial sample upon exit from the loop
    */
    ulElapsedTotalMS += DclTimePassed(ts);

  RWCleanup:
    DclTestInstrumentationStop(&pTI->sti, "RandW");

    if(hTestFile)
    {
        if(PRIMCLOSE(hTestFile) != DCLSTAT_SUCCESS)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Close and delete the file we have just used
    */
    if(!pTI->fNoDelete && PRIMDELETE(szWorkFile) != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulKBWritten = (ulBytesWritten + 512) / 1024;

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Wrote %lU times for a total of %s in %lU ms.  Flushed %lU times\n",
            ulCount,
            DclScaleKB(ulKBWritten, szScaleBuff, sizeof(szScaleBuff)),
            ulElapsedTotalMS,
            ulFlushes);
     }
/*
    if(pTI->fWriteVerify)
    {
        DclPrintf("    Read  %8lUKB in %lU ms -- %lUKB per second\n",
            ulKBWritten, ulElapsedReadMS, GetKBPerSecond(ulKBWritten, ulElapsedReadMS));
    }
    DclPrintf("    Seeked  %8lU times in %lU ms -- %lU ms each\n",
        ulCount, ulElapsedSeekMS, ulElapsedSeekMS / ulCount);
    DclPrintf("    Total Time Elapsed: %lU ms\n", ulElapsedTotalMS);
*/
    DclPrintf("  Small aligned writes ------------------------------->%10lU KB/sec\n",
        GetKBPerSecond(ulKBWritten, ulElapsedTotalMS));

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "RandomWrite", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB",  pTI->ulMaxDiskSpace / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen",  pTI->ulFSBlockSize);
        DCLPERFLOG_NUM(   hPerfLog, "OpCount",    ulCount);
        DCLPERFLOG_NUM(   hPerfLog, "TotWritten", ulKBWritten);
        DCLPERFLOG_NUM(   hPerfLog, "KB/second",  GetKBPerSecond(ulKBWritten, ulElapsedTotalMS));
        DCLPERFLOG_NUM(   hPerfLog, "Flushes",    ulFlushes);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:
       PTI - A pointer to the FSIOTESTINFO structure to use

    Notes:
       This function tests random write functionality

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS RandomUnaligned(
    FSIOTESTINFO   *pTI)
{
    DCLFSFILEHANDLE hTestFile;
    D_UINT32        ulKBWritten;
    D_UINT32        ulKBRead;
    DCLTIMESTAMP    ts;
    D_UINT32        ulCount = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szWorkFile[MAX_FILESPEC_LEN];
    D_UINT32        ulPassChunks;
    D_UINT32        ulOps;
    char            szScaleBuff[16];
    char            szScaleBuff2[16];
    D_UINT32        ulMaxIOSize = pTI->ulBufferSize / 2;
    D_UINT32        ulFlushNext = 0;
    D_UINT64        ullStartFlushUS;
    unsigned        nTotalPasses = pTI->nRandomWritePasses + pTI->nRandomReadPasses;

    /*  Variables for test totals
    */    
    D_UINT32        ulTotalReads = 0;
    D_UINT32        ulTotalWrites = 0;
    D_UINT32        ulTotalFlushes = 0;
    D_UINT32        ulTotalBytesRead = 0;
    D_UINT32        ulTotalBytesWritten = 0;
    D_UINT64        ullTotalReadTimeUS = 0;
    D_UINT64        ullTotalWriteTimeUS = 0;
    D_UINT64        ullTotalFlushTimeUS = 0;
    D_UINT32        ulTotalElapsedMS = 0;
    
    /*  Variables for pass totals
    */    
    D_UINT32        ulPassReads = 0;
    D_UINT32        ulPassWrites = 0;
    D_UINT32        ulPassFlushes = 0;
    D_UINT32        ulPassBytesRead = 0;
    D_UINT32        ulPassBytesWritten = 0;
    D_UINT64        ullPassReadTimeUS = 0;
    D_UINT64        ullPassWriteTimeUS = 0;
    D_UINT64        ullPassFlushTimeUS = 0;

    /*  Variables for sample totals
    */
    unsigned        nSampleNum = 0;
    DCLTIMESTAMP    tsSample = DCLINITTS;       /* Not necessary, but some compilers seem to think so... */
    D_UINT32        ulTimeSampleUS;
    D_UINT32        ulSampleReads = 0;
    D_UINT32        ulSampleWrites = 0;
    D_UINT32        ulSampleBytesRead = 0;      /* Not necessary, but some compilers seem to think so... */
    D_UINT32        ulSampleBytesWritten = 0;   /* Not necessary, but some compilers seem to think so... */

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  Randomly reading and writing a populated %s file, until %s has been processed...\n",
            DclScaleKB(pTI->ulMaxDiskSpace / 1024, szScaleBuff, sizeof(szScaleBuff)),
            DclScaleKB((pTI->ulMaxDiskSpace / 1024) * nTotalPasses, szScaleBuff2, sizeof(szScaleBuff2)));

        if(pTI->iFlushOnWriteRatio > 0)
            DclPrintf("  Flushing after every %u writes, or at the end of each pass\n", pTI->iFlushOnWriteRatio);
        else if(pTI->iFlushOnWriteRatio < 0)
            DclPrintf("  Flushing randomly but always once %u writes have been done\n", -pTI->iFlushOnWriteRatio);
    }
    
    dclStat = CreateUncachedFile(pTI, "fsru%04X.dat", pTI->ulMaxDiskSpace, szWorkFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = PRIMOPEN(szWorkFile, "r+b", &hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  ulMaxIOSize is divided by 2 in the following calculation so that
        ulPassChunks is based off the "average" I/O size, which is a 
        random number between 1 and ulMaxIOSize.
    */        
    ulPassChunks = pTI->ulMaxDiskSpace / (ulMaxIOSize / 2);

    ulOps = ulPassChunks * nTotalPasses;

    if(pTI->iFlushOnWriteRatio > 0)
        ulFlushNext = pTI->iFlushOnWriteRatio;
    else if(pTI->iFlushOnWriteRatio < 0)
        ulFlushNext = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % (-pTI->iFlushOnWriteRatio)) + 1;

    DclTestInstrumentationStart(&pTI->sti);

    ts = DclTimeStamp();

    /*  Read/write until we've done the prescribed number of operations...
    */
    while(ulCount != ulOps)
    {
        D_UINT32    ulOffset;
        D_UINT32    ulLen;

        /*  If starting a new sample...
        */
        if(ulSampleReads + ulSampleWrites == 0)
        {
            ulSampleBytesRead = 0;
            ulSampleBytesWritten = 0;
            tsSample = DclTimeStamp();
        }

        /*  Calculate the offset into the file where we will read/write next
        */
        ulOffset = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % pTI->ulMaxDiskSpace);

        dclStat = PRIMSEEK(hTestFile, ulOffset, DCLFSFILESEEK_SET);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Unable to seek, Status=%lX\n", dclStat);
            goto RUCleanup;
        }

        /*  Calculate the length to read/write
        */
        ulLen = (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % ulMaxIOSize) + 1;

        ulLen = DCLMIN(ulLen, pTI->ulMaxDiskSpace - ulOffset);

        if((DclRand64(&pTI->sti.ullRandomSeed) % nTotalPasses) >= pTI->nRandomWritePasses)
        {
            D_UINT64    ullStartReadUS = pTI->FSPrim.ullReadTimeUS;

            dclStat = PRIMREAD(hTestFile, pTI->pBuffer+1, ulLen, NULL);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    FSIO: Unable to read %U bytes at offset %lU, Status=%lX\n", ulLen, ulOffset, dclStat);
                goto RUCleanup;
            }

            ullStartReadUS = pTI->FSPrim.ullReadTimeUS - ullStartReadUS;

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            {
                DclPrintf("    Read  %5U bytes at offset %8lU in %5llU us\n",
                    ulLen, ulOffset, VA64BUG(ullStartReadUS));
            }

            ullPassReadTimeUS += ullStartReadUS;
            ulPassBytesRead += ulLen;
            ulPassReads++;
            ulSampleReads++;
            ulSampleBytesRead += ulLen;
        }
        else
        {
            size_t      nWritten;
            D_UINT64    ullStartWriteUS = pTI->FSPrim.ullWriteTimeUS;

            dclStat = PrimWrite(pTI, pTI->pBuffer+1, ulLen, 1, hTestFile, &nWritten);
            if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
            {
                if(dclStat == DCLSTAT_SUCCESS)
                    dclStat = DCLSTAT_FS_WRITEFAILED;

                DclPrintf("    FSIO: Unable to write %U bytes at offset %lU with status %lX\n", ulLen, ulOffset, dclStat);

                goto RUCleanup;
            }

            ullStartWriteUS = pTI->FSPrim.ullWriteTimeUS - ullStartWriteUS;

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            {
                DclPrintf("    Wrote %5U bytes at offset %8lU in %5llU us\n",
                    ulLen, ulOffset, VA64BUG(ullStartWriteUS));
            }

            if(ulTotalWrites + ulPassWrites == ulFlushNext)
            {
                ullPassFlushTimeUS += RandomFlushHelper(pTI, hTestFile, &ulFlushNext);
                ulPassFlushes++;
            }

            ullPassWriteTimeUS += ullStartWriteUS;
            ulPassBytesWritten += ulLen;
            ulPassWrites++;
            ulSampleWrites++;
            ulSampleBytesWritten += ulLen;
        }

        ulCount++;

        ulTimeSampleUS = DclTimePassedUS(tsSample);

        /*  If sampling on every I/O, OR this is the last sample
            of a pass, OR our sample time has come...
        */
        if( (pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL) &&
            ( (pTI->nSampleRate == 0) ||
              (ulCount % ulPassChunks == 0) ||
              (ulTimeSampleUS >= pTI->nSampleRate * 1000000) ) )
        {
            nSampleNum++;

            DclPrintf("  Sample %2u: Read %5lU KB, wrote %5lU KB, in %5lU ms (%3lU/%3lU reads/writes)\n",
                nSampleNum, 
                (ulSampleBytesRead + 512) / 1024,
                (ulSampleBytesWritten + 512) / 1024,
                (ulTimeSampleUS + 500) / 1000,
                ulSampleReads,
                ulSampleWrites);
            
            /*  Resetting both of these to zero will cause all
                the sample counts to be reset.
            */
            ulSampleReads = ulSampleWrites = 0;
        }

        if(ulCount % ulPassChunks == 0)
        {
            D_UINT32    ulPassMS;
            
            ulFlushNext = ulTotalWrites + ulPassWrites;
            ullPassFlushTimeUS += RandomFlushHelper(pTI, hTestFile, &ulFlushNext);
            ulPassFlushes++;

            ulPassMS = DclTimePassed(ts);

            if(pTI->ulTestEmulationVersion < DEFAULT_VERSION || pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Pass %2lU: Read %5lU KB, wrote %5lU KB, in %5lU ms (includes %lU flushes)\n",
                    ulCount / ulPassChunks, 
                    (ulPassBytesRead + 512) / 1024,
                    (ulPassBytesWritten + 512) / 1024,
                    ulPassMS,
                    ulPassFlushes);
            }
            
            if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
            {
                DclPrintf("               Did %lU reads of %lU KB in %lU ms for %lU KB/sec\n",
                    ulPassReads, 
                    (ulPassBytesRead + 512) / 1024,
                    (ullPassReadTimeUS + 500) / 1000,
                    GetKBPerSecond((ulPassBytesRead + 512) / 1024, (D_UINT32)((ullPassReadTimeUS + 500) / 1000)));

                DclPrintf("               Did %lU writes of %lU KB in %lU ms for %lU KB/sec\n",
                    ulPassWrites, 
                    (ulPassBytesWritten + 512) / 1024,
                    (ullPassWriteTimeUS + 500) / 1000,
                    GetKBPerSecond((ulPassBytesWritten + 512) / 1024, (D_UINT32)((ullPassWriteTimeUS + 500) / 1000)));
                
                DclPrintf("               Did %lU flushes in %lU ms\n",
                    ulPassFlushes, 
                    (ullPassFlushTimeUS + 500) / 1000);
            }
            
            ulTotalReads += ulPassReads;
            ulTotalWrites += ulPassWrites;
            ulTotalFlushes += ulPassFlushes;
            ulTotalBytesRead += ulPassBytesRead;
            ulTotalBytesWritten += ulPassBytesWritten;
            ullTotalReadTimeUS += ullPassReadTimeUS;
            ullTotalWriteTimeUS += ullPassWriteTimeUS;
            ullTotalFlushTimeUS += ullPassFlushTimeUS;
            ulTotalElapsedMS += ulPassMS;

            ulPassBytesRead = 0;
            ulPassBytesWritten = 0;
            ulPassReads = 0;
            ulPassWrites = 0;
            ulPassFlushes = 0;
            ullPassReadTimeUS = 0;
            ullPassWriteTimeUS = 0;
            ullPassFlushTimeUS = 0;

            ts = DclTimeStamp();
        }
        
    }

    /*  Make sure everything we wrote is actually on disk before we
        stop timing.
    */
    ullStartFlushUS = pTI->FSPrim.ullFlushTimeUS;
    
    PRIMFLUSH(hTestFile);
    ulTotalFlushes++;

    ullTotalFlushTimeUS = pTI->FSPrim.ullFlushTimeUS - ullStartFlushUS;

    /*  Do the close prior to stopping timing so we can be more certain
        that the data really is on the disk.
    */
    dclStat = PRIMCLOSE(hTestFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    hTestFile = NULL;

    /*  Must process any partial sample upon exit from the loop
    */
    ulTotalElapsedMS += DclTimePassed(ts);

  RUCleanup:
    DclTestInstrumentationStop(&pTI->sti, "RandU");

    if(hTestFile)
    {
        if(PRIMCLOSE(hTestFile) != DCLSTAT_SUCCESS)
            return DCLSTAT_CURRENTLINE;
    }

    /*  Close and delete the file we have just used
    */
    if(!pTI->fNoDelete && PRIMDELETE(szWorkFile) != DCLSTAT_SUCCESS)
        return DCLSTAT_CURRENTLINE;

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulKBWritten = (ulTotalBytesWritten + 512) / 1024;
    ulKBRead = (ulTotalBytesRead + 512) / 1024;

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Read  %7lU times for a total of %s.\n",
            ulTotalReads,
            DclScaleKB(ulKBRead, szScaleBuff, sizeof(szScaleBuff)));
        
        DclPrintf("    Wrote %7lU times for a total of %s.  Flushed %lU times\n",
            ulTotalWrites,
            DclScaleKB(ulKBWritten, szScaleBuff, sizeof(szScaleBuff)),
            ulTotalFlushes);
     }

     DclPrintf("  Large unaligned reads ------------------------------>%10lU KB/sec\n",
        GetKBPerSecond(ulKBRead, (D_UINT32)((ullTotalReadTimeUS + 500) / 1000)));
     DclPrintf("  Large unaligned writes ----------------------------->%10lU KB/sec\n",
        GetKBPerSecond(ulKBWritten, (D_UINT32)((ullTotalWriteTimeUS + 500) / 1000)));

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    {
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "RandomUnaligned", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB",  pTI->ulMaxDiskSpace / 1024);
        DCLPERFLOG_NUM(   hPerfLog, "MaxIOSize",  ulMaxIOSize);
        DCLPERFLOG_NUM(   hPerfLog, "OpCount",    ulCount);
        DCLPERFLOG_NUM(   hPerfLog, "TotRead",    ulKBRead);
        DCLPERFLOG_NUM(   hPerfLog, "TotWritten", ulKBWritten);
        DCLPERFLOG_NUM(   hPerfLog, "RdKB/sec",   GetKBPerSecond(ulKBRead, (D_UINT32)((ullTotalReadTimeUS + 500) / 1000)));
        DCLPERFLOG_NUM(   hPerfLog, "WrtKB/sec",  GetKBPerSecond(ulKBWritten, (D_UINT32)((ullTotalWriteTimeUS + 500) / 1000)));
        DCLPERFLOG_NUM(   hPerfLog, "Flushes",    ulTotalFlushes);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:
       PTI - A pointer to the FSIOTESTINFO structure to use

    Notes:
       This function tests random write functionality

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static D_UINT64 RandomFlushHelper(
    FSIOTESTINFO   *pTI,
    DCLFSFILEHANDLE hTestFile,
    D_UINT32       *pulFlushNext)
{
    D_UINT32        ulOldWriteNum;
    D_UINT64        ullStartFlushUS;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pulFlushNext, sizeof(*pulFlushNext));

    ulOldWriteNum = *pulFlushNext;
    ullStartFlushUS = pTI->FSPrim.ullFlushTimeUS;
    
    PRIMFLUSH(hTestFile);

    ullStartFlushUS = pTI->FSPrim.ullFlushTimeUS - ullStartFlushUS;

    if(pTI->iFlushOnWriteRatio > 0)
        *pulFlushNext += pTI->iFlushOnWriteRatio;
    else if(pTI->iFlushOnWriteRatio < 0)
        *pulFlushNext += (D_UINT32)(DclRand64(&pTI->sti.ullRandomSeed) % (-pTI->iFlushOnWriteRatio)) + 1;

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Flushed after write %lU, requiring %llU us, next flush after %lU more writes\n", 
            ulOldWriteNum, VA64BUG(ullStartFlushUS), *pulFlushNext - ulOldWriteNum);
    }

    return ullStartFlushUS;
}


/*-------------------------------------------------------------------
    GetFragPerf()

    Description
        This function tests file I/O performance on a fragmented
        disk.

    Parameters

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS GetFragPerf(
    FSIOTESTINFO       *pTI)
{
    D_UINT32            ulLargeFileSizeKB = pTI->ulMaxDiskSpace / 1024;
    D_UINT32            ulSmallFileSize;
    D_UINT32            ulTinyFileSize;
    D_UINT32            ulTotalMS;
    D_UINT32            ulMS;
    D_UINT32            ulTotalPairs;
    D_UINT32            ulFreeBlocks;
    D_UINT16            uSleepScale;
    D_UINT32            ulAvgContCreateMS;
    D_UINT32            ulAvgContVerifyMS;
    D_UINT32            ulAvgContDeleteMS;
    D_UINT32            ulAvgFragCreateMS;
    D_UINT32            ulAvgFragVerifyMS;
    D_UINT32            ulAvgFragDeleteMS;
    D_UINT32            ulContCreateKBS;
    D_UINT32            ulContVerifyKBS;
    D_UINT32            ulContDeleteKBS;
    D_UINT32            ulFragCreateKBS;
    D_UINT32            ulFragVerifyKBS;
    D_UINT32            ulFragDeleteKBS;
    DCLSTATUS           dclStat;
    DCLTIMESTAMP        ts;
    DCLTIMESTAMP        tsSet;
    DCLPERFLOGHANDLE    hPerfLog;
    unsigned            dd;                 /* Dir num */
    unsigned            ff;                 /* File num */
    unsigned            nDirs;              /* Total directories */
    unsigned            nDirFiles = 0;      /* Files per directory (initialization only needed to mollify a neurotic compiler) */
    unsigned            fOldVerify;
    unsigned            nPathLen;
    unsigned            nCreated;           /* Actual contiguous and fragged files created */
    char                szScaleBuff[16];
    D_UINT32            ulMaxUS;
    unsigned            nDeleted;

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  NOTE:  For accurate results with this test, the disk should be freshly\n");
        DclPrintf("         formatted, and the /MAX parameter should be used to specify a\n");
        DclPrintf("         value that is approximately 1/16th of the formatted disk size.\n");
        DclPrintf("         The IO Buffer size (/B) should be at least %u times the device\n", SMALL_FILE_SIZE_FACTOR);
        DclPrintf("         block size.\n\n");
    }
    
    uSleepScale = (D_UINT16)(((pTI->ulMaxDiskSpace * 4) / 1024) / 1024) + 3;

    if(pTI->ulSmallFileSize > pTI->ulBufferSize)
    {
        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("WARNING: Reducing the \"Small\" file size from %lU to %lU so it\n", pTI->ulSmallFileSize, pTI->ulBufferSize);
            DclPrintf("         is not larger than the I/O buffer size.\n");
        }
        
        ulSmallFileSize = pTI->ulBufferSize;
    }
    else
    {
        ulSmallFileSize = pTI->ulSmallFileSize;
    }

    /*  The size for the "tiny" file, in the file pairs, is the same as
        the device block size.  We don't go smaller than this to try to
        avoid skewing the results in favor of those file systems which
        can combine file data and directory entries in a single block.
    */
    ulTinyFileSize = pTI->statfs.ulBlockSize;

    if(ulTinyFileSize >= ulSmallFileSize)
    {
        DclPrintf("The /FRAG test cannot run in this disk geometry.  Try increasing the default\n");
        DclPrintf("buffer sizes with /MAX or /B, or decreasing the device block size.\n");

        return DCLSTAT_FAILURE;
    }

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("  Starting free space: %lU KB\n", DclMulDiv(FreeDiskBlocks(pTI), pTI->ulFSBlockSize, 1024));

    hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "Frag", NULL, pTI->sti.szPerfLogSuffix);
    DCLPERFLOG_NUM(hPerfLog, "TinyB",       ulTinyFileSize);
    DCLPERFLOG_NUM(hPerfLog, "SmallB",      ulSmallFileSize);
    DCLPERFLOG_NUM(hPerfLog, "BigKB",       ulLargeFileSizeKB);
    DCLPERFLOG_NUM(hPerfLog, "FileCnt",     FRAG_FILE_COUNT);
    DCLPERFLOG_NUM(hPerfLog, "BegFS",       FreeDiskBlocks(pTI));

    /*  Save the original verify setting, and set verify to TRUE.  This
        will prevent the PrimWrite() function from filling the buffer
        with random data before writing it out.
    */
    fOldVerify = pTI->fWriteVerify;
    pTI->fWriteVerify = TRUE;

    /*  This test creates a lot of files, which we do not want to do in the
        root.  Therefore create a temporary directory in which these files
        will be created.  MUST be properly cleaned up before exiting this
        function.
    */
    dclStat = CreateAndAppendTestDir(pTI, "FRG", UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

        /*--------------------------------------------------------*\
         *                                                        *
         *  First, on the empty disk, create 8 large files, and   *
         *  record the create, verify and delete times, both      *
         *  average and long times.                               *
         *                                                        *
        \*--------------------------------------------------------*/

    DclTestStatsReset(&pTI->sti);

    DclPrintf("  Generating a baseline using %u large (%s) files\n",
        FRAG_FILE_COUNT, DclScaleKB(ulLargeFileSizeKB, szScaleBuff, sizeof(szScaleBuff)));
    
    dclStat = CreateFileSet(pTI, "FFS", &ulTotalMS, &ulMaxUS, &nCreated, FRAG_FILE_COUNT,
                            pTI->ulMaxDiskSpace / pTI->ulBufferSize, pTI->ulBufferSize, 
                            pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL, 1, UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error creating contiguous file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    ulAvgContCreateMS = ulTotalMS / nCreated;
    ulContCreateKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgContCreateMS);

    DclPrintf("    Created (wrote) %u contiguous files --------------->%10lU KB/sec  %3s%%\n",
        nCreated, ulContCreateKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragCCrt");

    DCLPERFLOG_NUM(hPerfLog, "CCrtTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "CCrtMx",  (ulMaxUS + 500) / 1000);
    DCLPERFLOG_NUM(hPerfLog, "CFS",     FreeDiskBlocks(pTI));

    AppSleep(pTI, 3);

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Verifying %u files...\n", nCreated);

    dclStat = VerifyFileSet(pTI, "FFS", &ulTotalMS, &ulMaxUS, nCreated,
                            pTI->ulMaxDiskSpace / pTI->ulBufferSize, pTI->ulBufferSize,
                            pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error verifying contiguous file set, Status=%lX\n", dclStat);

        goto FragCleanup;
    }

    ulAvgContVerifyMS = ulTotalMS / nCreated;
    ulContVerifyKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgContVerifyMS);

    DclPrintf("    Verified (read) %u contiguous files --------------->%10lU KB/sec  %3s%%\n",
        nCreated, ulContVerifyKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragCVer");

    DCLPERFLOG_NUM(hPerfLog, "CVerTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "CVerMx",  (ulMaxUS + 500) / 1000);

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Deleting %u files...\n", nCreated);

    dclStat = DeleteFileSet(pTI, "FFS", nCreated, &ulTotalMS, &ulMaxUS, &nDeleted, pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error deleting contiguous file set, Status=%lX\n", dclStat);

        goto FragCleanup;
    }

    ulAvgContDeleteMS = ulTotalMS / nCreated;
    ulContDeleteKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgContDeleteMS);

    DclPrintf("    Deleted %u contiguous files ----------------------->%10lU KB/sec  %3s%%\n",
        nCreated, ulContDeleteKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragCDel");

    DCLPERFLOG_NUM(hPerfLog, "CDelTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "CDelMx",  (ulMaxUS + 500) / 1000);

    AppSleep(pTI, (D_UINT16)(uSleepScale/8));


        /*--------------------------------------------------------*\
         *                                                        *
         *  Next, alternately create "small" and "tiny" files     *
         *  until the disk is about pTI->ulMaxDiskSpace short of  *
         *  being full (avoid disk full issues!).  Then delete    *
         *  all the "small" files, leaving the disk fragmented.   *
         *                                                        *
         *  Create these file pairs in a set of directories to    *
         *  avoid the performance degradation when creating lots  *
         *  of files in one directory.                            *
         *                                                        *
        \*--------------------------------------------------------*/


    DclTestStatsReset(&pTI->sti);

    DclPrintf("  Filling the disk with file pairs (%lU and %lU bytes)\n", 
        ulSmallFileSize, ulTinyFileSize);

    ulTotalMS = 0;
    ulTotalPairs = 0;

    for(nDirs = 1; nDirs <= UINT_MAX; nDirs++)
    {
        unsigned        fFillComplete = FALSE;
        DCLTIMESTAMP    tsDir;

        /*  Better not ever wrap...
        */
        DclAssert(nDirs);

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("    Adding files in directory FRD%05u...\n", nDirs);

        tsDir = DclTimeStamp();

        dclStat = CreateAndAppendTestDir(pTI, "FRD", nDirs);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            nDirs--;
            break;
        }

        tsSet = DclTimeStamp();

        for(nDirFiles = 1; nDirFiles <= FRAG_PAIRS_PER_DIR; nDirFiles++)
        {
            dclStat = FileCreate(pTI, "FRA", nDirFiles, 1, ulSmallFileSize, NULL, FALSE);
            if(dclStat == DCLSTAT_SUCCESS)
            {
                D_BOOL  fFlush = FALSE;

                /*  Starting with test v3.0, after each FRAG_PAIRS_PER_DIR
                    count of files created, do a flush.
                */                    
                if(pTI->ulTestEmulationVersion >= DEFAULT_VERSION && !(nDirFiles % FRAG_FILL_FLUSH_SAMPLE))
                    fFlush = TRUE;
                    
                dclStat = FileCreate(pTI, "FRB", nDirFiles, 1, ulTinyFileSize, NULL, fFlush);
                if(dclStat == DCLSTAT_SUCCESS)
                {
                    ulTotalPairs++;
                }
                else
                {
                    /*  Really shouldn't hit this.  If this is hit with any
                        regularity, then we need to re-examine how we are
                        checking for "near-disk-full", because we need to
                        opt-out of this process before the disk gets full.
                    */
                    DclError();

                    /*  Try to recover so that only whole pairs remain
                    */
                    dclStat = FileDelete(pTI, "FRA", nDirFiles);
                    if(dclStat != DCLSTAT_SUCCESS)
                    {
                        /*  Something is really FUBAR if we cannot delete
                            the file we just created.
                        */
                        DclPrintf("    FSIO: Error deleting file FRA%05u, Status=%lX\n", nDirFiles, dclStat);

                        goto FragCleanup;
                    }

                    nDirFiles--;

                    fFillComplete = TRUE;
                }
            }
            else
            {
                /*  Really shouldn't hit this.  If this is hit with any
                    regularity, then we need to re-examine how we are
                    checking for "near-disk-full", because we need to
                    opt-out of this process before the disk gets full.
                */
                DclError();

                nDirFiles--;

                fFillComplete = TRUE;
            }

            if(fFillComplete || !(nDirFiles % FRAG_FILL_FLUSH_SAMPLE))
            {
                if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                {
                    D_UINT64    ullFree = DclMulDiv(FreeDiskBlocks(pTI), pTI->ulFSBlockSize, 1024);
                    
                    DclPrintf("      %6lU ms to create pairs %4u - %4u, FreeSpace=%llU KB\n",
                        DclTimePassed(tsSet), nDirFiles-(FRAG_FILL_FLUSH_SAMPLE-1), 
                        nDirFiles, VA64BUG(ullFree));
                }

                /*  If there is less than /MAX space free on the disk quit now,
                    as we don't want to completely fill the disk.
                */
                if( (fFillComplete) ||
                    (FreeDiskBlocks(pTI) < (pTI->ulMaxDiskSpace / pTI->statfs.ulBlockSize)) ||
                    (pTI->ulFragMaxPairs && ulTotalPairs >= pTI->ulFragMaxPairs) )
                {
                    fFillComplete = TRUE;
                    break;
                }

                /*  Reset the timestamp for the next set
                */
                tsSet = DclTimeStamp();
            }

        }

        if(nDirFiles > FRAG_PAIRS_PER_DIR)
        {
            DclAssert(nDirFiles == FRAG_PAIRS_PER_DIR+1);

            /*  If we broke out of the loop due to maxing out the file count
                for the directory, reduce the file count so it accurately
                reflects the file count for the final directory processed.
            */
            nDirFiles--;
        }

        ulMS = DclTimePassed(tsDir);

        DclPrintf("    Created directory %2u with %3u file pairs --------->%10s seconds %3s%%\n",
            nDirs, nDirFiles, DclRatio(szScaleBuff, sizeof szScaleBuff, ulMS, 1000, 1), GetFreeSpacePercent(pTI));

        ulTotalMS += ulMS;

        /*  Remove the last directory tacked onto the path so that we
            can iterate to the next directory which will be tacked on
            at the same level.
        */
        DclTestDirRemoveFromPath(pTI->szPath);

        if(fFillComplete)
            break;
    }

    DclPrintf("    Created %2u directories with %5lU file pairs ----->%10s seconds\n",
        nDirs, ulTotalPairs, DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1));

    ulFreeBlocks = FreeDiskBlocks(pTI);
/*
    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        D_UINT64    ullFree = DclMulDiv(ulFreeBlocks, pTI->ulFSBlockSize, 1024);
        
        DclPrintf("    %llU KB of free space remains on the disk\n", VA64BUG(ullFree));
    }
*/    
    DclTestStatsDump(&pTI->sti, "FragFill");

    DCLPERFLOG_NUM(hPerfLog, "PCnt",   ulTotalPairs);
    DCLPERFLOG_NUM(hPerfLog, "PCrtMS", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "PFS",    ulFreeBlocks);

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Deleting the %lU byte files (group A)...\n", ulSmallFileSize);

    if(pTI->fNoDelete)
        DclPrintf("  WARNING: Using the /NODEL option defeats the purpose of this test!\n");

    ulTotalMS = 0;
    nPathLen = DclStrLen(pTI->szPath);

    for(dd = 1; dd <= nDirs; dd++)
    {
        unsigned    nDeleted = 0;

        /*  Use a specific number as the suffix
        */
        DclSNPrintf(&pTI->szPath[nPathLen], sizeof(pTI->szPath) - (nPathLen+1),
            "FRD%05u"DCL_PATHSEPSTR, dd);

        tsSet = DclTimeStamp();

        for(ff = 1; ff <= FRAG_PAIRS_PER_DIR; ff++)
        {
            /*  The very last directory may not contain the maximum number
                of files, so opt out early when the time is right.
            */
            if(dd == nDirs && ff > nDirFiles)
                break;

            dclStat = FileDelete(pTI, "FRA", ff);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    FSIO: Error deleting file %u in directory %u, Status=%lX\n", ff, dd, dclStat);

                goto FragCleanup;
            }

            nDeleted++;
        }

        ulMS = DclTimePassed(tsSet);
        ulTotalMS += ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
        {
            DclPrintf("    Deleting %4u 'A' files from directory FRD%05u required %6lU ms\n",
               nDeleted, dd, ulMS);
        }

        /*  Restore the original path
        */
        pTI->szPath[nPathLen] = 0;
    }

    DclTestStatsDump(&pTI->sti, "FragSDel");

    DCLPERFLOG_NUM(hPerfLog, "SDel", ulTotalMS);

    DclPrintf("    Deleted the %5lU byte files --------------------->%10s seconds %3s%%\n",
        ulSmallFileSize,
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1), GetFreeSpacePercent(pTI));

    ulFreeBlocks = FreeDiskBlocks(pTI);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        D_UINT64    ullFree = DclMulDiv(ulFreeBlocks, pTI->ulFSBlockSize, 1024);
        
        DclPrintf("    %llU KB of free space remains on the disk\n", VA64BUG(ullFree));
    }
    
    AppSleep(pTI, (D_UINT16)(uSleepScale/4));


        /*--------------------------------------------------------*\
         *                                                        *
         *  Finally perform the same initial test, creating,      *
         *  verifying, and deleting 8 large files, recording      *
         *  the average and long times.                           *
         *                                                        *
        \*--------------------------------------------------------*/


    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  Creating %u files each %s long...\n",
            FRAG_FILE_COUNT, DclScaleKB(ulLargeFileSizeKB, szScaleBuff, sizeof(szScaleBuff)));
    }

    DclPrintf("  Fragmented performance change relative to baseline\n");
     
    dclStat = CreateFileSet(pTI, "FFS", &ulTotalMS, &ulMaxUS, &nCreated, FRAG_FILE_COUNT,
                            pTI->ulMaxDiskSpace / pTI->ulBufferSize, pTI->ulBufferSize, 
                            pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL, 1, UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error creating fragmented file set, Status=%lX\n", dclStat);

        /*  Don't fail, just keep going...
        */
    }

    ulAvgFragCreateMS = ulTotalMS / nCreated;
    ulFragCreateKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgFragCreateMS);

    szScaleBuff[0] = (ulFragCreateKBS <= ulContCreateKBS) ? '-' : '+';
    DclRatio(szScaleBuff+1, sizeof(szScaleBuff) - 1, DCLABSDIFF(ulContCreateKBS, ulFragCreateKBS) * 100, ulContCreateKBS, 1);

    DclPrintf("    Creation (writing) -----------------> %6s%% ---->%10lU KB/sec  %3s%%\n",
        szScaleBuff, ulFragCreateKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragFCrt");

    DCLPERFLOG_NUM(hPerfLog, "FCrtTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "FCrtMx",  (ulMaxUS + 500) / 1000);

    AppSleep(pTI, 3);

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Verifying %u files...\n", nCreated);

    dclStat = VerifyFileSet(pTI, "FFS", &ulTotalMS, &ulMaxUS, nCreated,
                            pTI->ulMaxDiskSpace / pTI->ulBufferSize, pTI->ulBufferSize, 
                            pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error verifying fragmented file set, Status=%lX\n", dclStat);

        goto FragCleanup;
    }

    ulAvgFragVerifyMS = ulTotalMS / nCreated;
    ulFragVerifyKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgFragVerifyMS);

    szScaleBuff[0] = (ulFragVerifyKBS <= ulContVerifyKBS) ? '-' : '+';
    DclRatio(szScaleBuff+1, sizeof(szScaleBuff) - 1, DCLABSDIFF(ulContVerifyKBS, ulFragVerifyKBS) * 100, ulContVerifyKBS, 1);

    DclPrintf("    Verification (reading) -------------> %6s%% ---->%10lU KB/sec  %3s%%\n",
        szScaleBuff, ulFragVerifyKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragFVer");

    DCLPERFLOG_NUM(hPerfLog, "FVerTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "FVerMx",  (ulMaxUS + 500) / 1000);

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Deleting %u files...\n", nCreated);

    dclStat = DeleteFileSet(pTI, "FFS", nCreated, &ulTotalMS, &ulMaxUS, &nDeleted, pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error deleting fragmented file set, Status=%lX\n", dclStat);

        goto FragCleanup;
    }

    ulAvgFragDeleteMS = ulTotalMS / nCreated;
    ulFragDeleteKBS = GetKBPerSecond((pTI->ulMaxDiskSpace * nCreated) / 1024, ulTotalMS);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Total: %lU ms  Average: %lU ms\n", ulTotalMS, ulAvgFragDeleteMS);

    szScaleBuff[0] = (ulFragDeleteKBS <= ulContDeleteKBS) ? '-' : '+';
    DclRatio(szScaleBuff+1, sizeof(szScaleBuff) - 1, DCLABSDIFF(ulContDeleteKBS, ulFragDeleteKBS) * 100, ulContDeleteKBS, 1);

    DclPrintf("    Deletion ---------------------------> %6s%% ---->%10lU KB/sec  %3s%%\n",
        szScaleBuff, ulFragDeleteKBS, GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragFDel");

    DCLPERFLOG_NUM(hPerfLog, "FDelTot", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "FDelMx",  (ulMaxUS + 500) / 1000);

    AppSleep(pTI, (D_UINT16)(uSleepScale/8));


        /*--------------------------------------------------------*\
         *                                                        *
         *                Delete the "Tiny" files                 *
         *                                                        *
        \*--------------------------------------------------------*/

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Deleting the %lU byte files (group B)...\n", ulTinyFileSize);

    ulTotalMS = 0;
    nPathLen = DclStrLen(pTI->szPath);

    for(dd = 1; dd <= nDirs; dd++)
    {
        unsigned    nDeleted = 0;

        /*  Use a specific number as the suffix
        */
        DclSNPrintf(&pTI->szPath[nPathLen], sizeof(pTI->szPath) - (nPathLen+1),
            "FRD%05u"DCL_PATHSEPSTR, dd);

        tsSet = DclTimeStamp();

        for(ff = 1; ff <= FRAG_PAIRS_PER_DIR; ff++)
        {
            /*  The very last directory may not contain the maximum number
                of files, so opt out early when the time is right.
            */
            if(dd == nDirs && ff > nDirFiles)
                break;

            dclStat = FileDelete(pTI, "FRB", ff);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("    FSIO: Error deleting file %u in directory %u, Status=%lX\n", ff, dd, dclStat);

                goto FragCleanup;
            }

            nDeleted++;
        }

        ulMS = DclTimePassed(tsSet);
        ulTotalMS += ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
        {
            DclPrintf("    Deleting %4u 'B' files from directory FRD%05u required %6lU ms\n",
                nDeleted, dd, ulMS);
        }

        /*  Restore the original path
        */
        pTI->szPath[nPathLen] = 0;
    }

    DclTestStatsDump(&pTI->sti, "FragTDel");

    DCLPERFLOG_NUM(hPerfLog, "TDel", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "DelFS",  FreeDiskBlocks(pTI));

    DclPrintf("  Deleted the %4lU byte files ------------------------>%10s seconds %3s%%\n", 
        ulTinyFileSize,
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1), GetFreeSpacePercent(pTI));


        /*--------------------------------------------------------*\
         *                                                        *
         *                Remove the directories                  *
         *                                                        *
        \*--------------------------------------------------------*/


    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("  Deleting the directories...\n");

    ts = DclTimeStamp();
    nPathLen = DclStrLen(pTI->szPath);

    for(dd = 1; dd <= nDirs; dd++)
    {
        /*  Use a specific number as the suffix
        */
        DclSNPrintf(&pTI->szPath[nPathLen], sizeof(pTI->szPath) - (nPathLen+1),
            "FRD%05u", dd);

        dclStat = PRIMDIRREMOVE(pTI->szPath);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Error deleting the directory FRD%05u, status=%lX\n", dd, dclStat);
            goto FragCleanup;
        }

        /*  Restore the original path
        */
        pTI->szPath[nPathLen] = 0;
    }

    /*  Remove the main FRAG directory
    */
    DestroyAndRemoveTestDir(pTI);

    ulTotalMS = DclTimePassed(ts);

    DclPrintf("  Deleted %2u directories ----------------------------->%10s seconds %3s%%\n", 
        nDirs+1, DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1), GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "FragDDel");

    DCLPERFLOG_NUM(hPerfLog, "DDel", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "EndFS", FreeDiskBlocks(pTI));

    dclStat = DCLSTAT_SUCCESS;

  FragCleanup:

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    /*  Restore the original verify setting.
    */
    pTI->fWriteVerify = fOldVerify;

    return dclStat;
}


/*-------------------------------------------------------------------
    This function determines a benchmark for reading data from a disk.

    (Note, this test has pretty much been made obsolete by various
    other, more sophisticated tests.)

    Parameters:

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS GetSmallFileIOPerf(
    FSIOTESTINFO   *pTI)
{
    unsigned        nCount = 50;
    D_UINT32        ulTotal = 0;
    D_UINT32        ulSingle;

    DclPrintf("  Create and Delete Small Files\n");

    /*  Default to 50 files, but reduce that if it will exceed the max
        amount of disk space we are allowed to use.
    */
    if(nCount * 20UL * 512UL > pTI->ulMaxDiskSpace)
        nCount = (pTI->ulMaxDiskSpace / (20 * 512));

    ulSingle = TestCreateAndDelete(pTI, 1, nCount, 20, 511);
    if(!ulSingle)
        return DCLSTAT_CURRENTLINE;
    
    ulTotal += ulSingle;

    ulSingle = TestCreateAndDelete(pTI, 2, nCount, 20, 512);
    if(!ulSingle)
        return DCLSTAT_CURRENTLINE;
    ulTotal += ulSingle;

    if(pTI->ulBufferSize > 512)
    {
        ulSingle = TestCreateAndDelete(pTI, 3, nCount, 20, 513);
        if(!ulSingle)
            return DCLSTAT_CURRENTLINE;
        ulTotal += ulSingle;

        nCount = 50;
        if(nCount * 20UL * 1024UL > pTI->ulMaxDiskSpace)
            nCount = (pTI->ulMaxDiskSpace / (20 * 1024));

        ulSingle = TestCreateAndDelete(pTI, 4, nCount, 20, 1023);
        if(!ulSingle)
            return DCLSTAT_CURRENTLINE;
        ulTotal += ulSingle;

        ulSingle = TestCreateAndDelete(pTI, 5, nCount, 20, 1024);
        if(!ulSingle)
            return DCLSTAT_CURRENTLINE;
        ulTotal += ulSingle;

        if(pTI->ulBufferSize > 1024)
        {
            ulSingle = TestCreateAndDelete(pTI, 6, nCount, 20, 1025);
            if(!ulSingle)
                return DCLSTAT_CURRENTLINE;
            ulTotal += ulSingle;

            nCount = 50;
            if(nCount * 20UL * 2048UL > pTI->ulMaxDiskSpace)
                nCount = (pTI->ulMaxDiskSpace / (20 * 2048));

            ulSingle = TestCreateAndDelete(pTI, 7, nCount, 20, 2047);
            if(!ulSingle)
                return DCLSTAT_CURRENTLINE;
            ulTotal += ulSingle;

            ulSingle = TestCreateAndDelete(pTI, 8, nCount, 20, 2048);
            if(!ulSingle)
                return DCLSTAT_CURRENTLINE;
            ulTotal += ulSingle;

            if(pTI->ulBufferSize > 2048)
            {
                ulSingle = TestCreateAndDelete(pTI, 9, nCount, 20, 2049);
                if(!ulSingle)
                    return DCLSTAT_CURRENTLINE;
                ulTotal += ulSingle;
            }
        }
    }

    DclPrintf("    Test required a total of %lU ms (not including sleep time)\n", ulTotal);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_UINT32 TestCreateAndDelete(
    FSIOTESTINFO   *pTI,
    unsigned        nSetNum,
    unsigned        nCount,
    D_UINT32        ulBlocks,
    D_UINT32        ulBlockSize)
{
    D_UINT32        ulCreateMS = 0;
    D_UINT32        ulVerifyMS = 0;
    D_UINT32        ulDeleteMS = 0;
    D_BOOL          fOldVerify;
    D_UINT32        ulTotal;
    D_UINT32        ulMaxUS;
    unsigned        nCreated;
    DCLSTATUS       dclStat;

    /*  Save the original verify setting, and set verify to TRUE.  This
        will prevent the PrimWrite() function from filling the buffer
        with random data before writing it out.
    */
    fOldVerify = pTI->fWriteVerify;
    pTI->fWriteVerify = TRUE;

    DclTestInstrumentationStart(&pTI->sti);

    dclStat = CreateFileSet(pTI, "FFS", &ulCreateMS, &ulMaxUS, &nCreated, nCount, ulBlocks, ulBlockSize, FALSE, 0, UINT_MAX);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
            DclPrintf("    File Set Creation required %4lU ms (long=%64lU us)\n", ulCreateMS, ulMaxUS);

        dclStat = VerifyFileSet(pTI, "FFS", &ulVerifyMS, &ulMaxUS, nCreated, ulBlocks, ulBlockSize, FALSE);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            unsigned        nDeleted;
            
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("    File Set Verify required   %4lU ms (long=%6lU us)\n", ulVerifyMS, ulMaxUS);

            dclStat = DeleteFileSet(pTI, "FFS", nCreated, &ulDeleteMS, &ulMaxUS, &nDeleted, FALSE);
            if(dclStat == DCLSTAT_SUCCESS)
            {
                if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                    DclPrintf("    File Set Delete required   %4lU ms (long=%6lU us)\n", ulDeleteMS, ulMaxUS);
            }
            else
            {
                DclPrintf("File set deletion failed with status %lX\n", dclStat);
            }
        }
        else
        {
            DclPrintf("File set verification failed with status %lX\n", dclStat);
        }
    }
    else
    {
        DclPrintf("File set creation failed with status %lX\n", dclStat);
    }

    DclTestInstrumentationStop(&pTI->sti, NULL);

    /*  Restore the original verify setting.
    */
    pTI->fWriteVerify = fOldVerify;

    if(dclStat != DCLSTAT_SUCCESS)
        return 0UL;

    ulTotal = ulCreateMS + ulVerifyMS + ulDeleteMS;

    DclPrintf("    Set %u took %8lU ms (%u files with %lU blocks of %lU bytes)\n",
        nSetNum, ulTotal, nCreated, ulBlocks, ulBlockSize);

    AppSleep(pTI, 2);

    /*  Never return 0 if we were otherwise successful, as this
        will cause the test to terminate early.
    */
    if(!ulTotal)
        ulTotal++;

    return ulTotal;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CreateFileSet(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    D_UINT32       *pulTotalMS,
    D_UINT32       *pulMaxUS,
    unsigned       *pnCreated,
    unsigned        nCount,
    D_INT32         lBlocks,
    D_UINT32        ulBlockSize,
    D_BOOL          fVerbose,
    unsigned        nFlushRatio,
    unsigned        nSampleSeconds)
{
    unsigned        kk;
    D_UINT64        ullTotalUS = 0;
    D_UINT32        ulMaxUS = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS; /* Init'ed for picky compilers */
    unsigned        nSampleNum = 1;
    D_UINT32        ulSampleFiles = 0;
    D_UINT32        ulSampleMS;
    D_UINT64        ullSampleBytes = 0;     /* Not necessary, but some compilers seem to think so... */  
    DCLTIMESTAMP    tsSample = DCLINITTS;   /* Not necessary, but some compilers seem to think so... */

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssertWritePtr(pulTotalMS, sizeof(*pulTotalMS));
    DclAssertWritePtr(pnCreated, sizeof(*pnCreated));
    DclAssert(nCount);
    DclAssert((lBlocks && ulBlockSize) || (!lBlocks && !ulBlockSize));

    *pnCreated = 0;

    for(kk = 0; kk < nCount; kk++)
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;
        D_BOOL          fFlush = FALSE;
        D_UINT32        ulBlocks = (D_UINT32)lBlocks;
        D_UINT32        ulThisBlockSize = ulBlockSize;

        /*  Special case if lBlocks is negative.  Set the block size for
            to a random value from 1 to ulBlockSize.
        */            
        if(lBlocks < 0)
        {
            DclAssert(ulBlockSize <= pTI->ulBufferSize);
            ulThisBlockSize = (D_UINT32)((DclRand64(&pTI->sti.ullRandomSeed) % ulBlockSize) + 1);
            ulBlocks = -lBlocks;
        }

        /*  Starting with test v3.0, allow flushing every Nth file.
        */                    
        if( (pTI->ulTestEmulationVersion >= DEFAULT_VERSION) && 
            (nFlushRatio) && !((kk+1) % nFlushRatio) )
        {
            fFlush = TRUE;
        }                    

        ulSampleFiles++;

        /*  If starting a new sample...
        */
        if(ulSampleFiles == 1)
        {
            ullSampleBytes = 0;
            tsSample = DclTimeStamp();
        }
        
        ts = DclTimeStamp();

        dclStat = FileCreate(pTI, pszPrefix, kk, ulBlocks, ulThisBlockSize, NULL, fFlush);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            if(fVerbose || pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
            {
                DclPrintf("    FSIO: Error creating file set. SetSize=%u BlockCount=%lU BlockSize=%lU FileNum=%u Error=%lX\n",
                     nCount, ulBlocks, ulThisBlockSize, kk, dclStat);
            }

            /* Don't count the failed file in the sample.
            */
            ulSampleFiles--;
        }
        else
        {
            ullSampleBytes += ulBlocks * ulThisBlockSize;

            ulUS = DclTimePassedUS(ts);
            ullTotalUS += ulUS;
            if(ulMaxUS < ulUS)
                ulMaxUS = ulUS;
        }

        ulSampleMS = DclTimePassed(tsSample);

        /*  If sampling is enabled, and if sampling on every file, OR 
            our sample time has come...
        */
        if( nSampleSeconds != UINT_MAX && 
            ((nSampleSeconds == 0) || (ulSampleMS >= nSampleSeconds * 1000) || dclStat != DCLSTAT_SUCCESS) )
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Sample %3u: Created %5lU files in %5lUms ------->%10lU KB/sec  %3s%%\n",
                    nSampleNum, ulSampleFiles, ulSampleMS,
                    GetKBPerSecond((ullSampleBytes + 512) / 1024, ulSampleMS),
                    GetFreeSpacePercent(pTI));
            }
            
            nSampleNum++;

            /*  Resetting ulSampleBlocks to zero will cause all
                the counts to be reset.
            */
            ulSampleFiles = 0;
        }

        if(dclStat != DCLSTAT_SUCCESS)
            break;

/*
        if(fVerbose || pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("    File %u create required %6lU us\n", kk, ulUS);
*/
        (*pnCreated)++;
    }

    *pulTotalMS = (D_UINT32)((ullTotalUS + 500) / 1000);

    if(pulMaxUS)
        *pulMaxUS = ulMaxUS;

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS FileCreate(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nFileNum,
    D_UINT32        ulBlocks,
    D_UINT32        ulBlockSize,
    char           *pBuffer,
    D_BOOL          fFlush)
{
    char            szIOFile[MAX_FILESPEC_LEN];
    unsigned        kk;
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat;
    D_UINT32        ulOffset = 0;

    /*  Ensure that the I/O size we are doing fits in our buffer, and
        that we can't end up in an endless loop when initializing it.
    */
    while(ulBlockSize > pTI->ulBufferSize || ulBlockSize > UINT_MAX)
    {
        ulBlockSize >>= 1;
        ulBlocks <<= 1;
    };

    hFile = CreateNumberedFile(pTI, szIOFile, pszPrefix, nFileNum);
    if(!hFile)
        return DCLSTAT_FS_CREATEFAILED;

    while(ulBlocks)
    {
        size_t      nWritten;

        /*  Generate a data pattern.  This code must exactly match
            the data verification algorithm used in VerifyFile().
        */
        for(kk = 0; kk < ulBlockSize; kk++)
        {
            pTI->pBuffer[kk] = (kk + ulOffset) % 256;
        }

        dclStat = PrimWrite(pTI, pTI->pBuffer, (size_t)ulBlockSize, 1, hFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
        {
            DCLSTATUS   dclStat2;

            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;

            dclStat2 = PRIMCLOSE(hFile);
            if(dclStat2 != DCLSTAT_SUCCESS)
            {
                DclPrintf("    Unable to close file after write error, Status=%lX\n", dclStat2);
                return dclStat;
            }

            dclStat2 = PRIMDELETE(szIOFile);
            if(dclStat2 != DCLSTAT_SUCCESS)
            {
                DclPrintf("    Unable to delete file after write error, Status=%lX\n", dclStat2);
            }

            return dclStat;
        }

        ulBlocks--;

        ulOffset += ulBlockSize;
    }

    if(fFlush)
    {
        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("Created file %s with flush\n", szIOFile);
        
        dclStat = PRIMFLUSH(hFile);
        if(dclStat != DCLSTAT_SUCCESS)
            DclPrintf("    Unable to flush file, Status=%lX\n", dclStat);

        /*  Ignore the error code and try to close the file anyway...
        */
    }
    
    dclStat = PRIMCLOSE(hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("    Unable to close file, Status=%lX\n", dclStat);
    }
    else
    {
        if(pBuffer)
            DclStrCpy(pBuffer, szIOFile);
    }
    
    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS VerifyFileSet(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    D_UINT32       *pulTotalMS,
    D_UINT32       *pulMaxUS,
    unsigned        nCount,
    D_UINT32        ulBlocks,
    D_UINT32        ulBlockSize,
    D_BOOL          fVerbose)
{
    unsigned        kk;
    DCLTIMESTAMP    ts;
    D_UINT32        ulUS;
    D_UINT64        ullTotalUS = 0;
    D_UINT32        ulMaxUS = 0;
    DCLSTATUS       dclStat;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pulTotalMS, sizeof(*pulTotalMS));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nCount <= FILESET_MAX);
    DclAssert(nCount);
 
    DCLPRINTF(2, ("    Starting File Set Verification of %u files\n", nCount));

    for(kk = 0; kk < nCount; kk++)
    {
        ts = DclTimeStamp();

        dclStat = VerifyFile(pTI, pszPrefix, kk, ulBlocks, ulBlockSize);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("    FSIO: Error verifying file set. SetSize=%u BlockCount=%lU BlockSize=%lU FileNum=%u, Status=%lX\n",
                 nCount, ulBlocks, ulBlockSize, kk, dclStat);

            return dclStat;
        }

        ulUS = DclTimePassedUS(ts);
        ullTotalUS += ulUS;
        if(ulMaxUS < ulUS)
            ulMaxUS = ulUS;

        if(fVerbose || pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("    File %u verify required %6lU us\n", kk, ulUS);
    }

    *pulTotalMS = (D_UINT32)((ullTotalUS + 500) / 1000);

    if(pulMaxUS)
        *pulMaxUS = ulMaxUS;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS VerifyFile(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nFileNum,
    D_UINT32        ulBlocks,
    D_UINT32        ulBlockSize)
{
    char            szIOFile[MAX_FILESPEC_LEN];
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat;
    DCLSTATUS       dclStat2;
    D_UINT32        ulVerified;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nFileNum <= FILESET_MAX);
    
    /*  Ensure that the I/O size we are doing fits in our buffer, and
        that we can't end up in an endless loop when initializing it.
    */
    while(ulBlockSize > pTI->ulBufferSize || ulBlockSize > UINT_MAX)
    {
        ulBlockSize >>= 1;
        ulBlocks <<= 1;
    };

    dclStat = OpenNumberedFile(pTI, &hFile, szIOFile, pszPrefix, nFileNum);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = VerifyFileHelper(pTI, hFile, &ulVerified, 0);

    dclStat2 = PRIMCLOSE(hFile);
    if(dclStat2 != DCLSTAT_SUCCESS)
        DCLPRINTF(1, ("    Unable to close file, Status=%lX\n", dclStat));

    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = dclStat2;

    return dclStat;
}


/*-------------------------------------------------------------------
    Verify a file's contents match the pattern used by FileCreate().
    This function assumes that the file has just been opened and that
    the file pointer is at offset zero.  

    The nOffset parameter may be used to force this function to do 
    the I/O using both misaligned client buffers and file offsets.
-------------------------------------------------------------------*/
static DCLSTATUS VerifyFileHelper(
    FSIOTESTINFO   *pTI,
    DCLFSFILEHANDLE hFile,
    D_UINT32       *pulVerified,
    unsigned        nOffset)
{
    DCLSTATUS       dclStat;
    D_UINT32        ulOffset;
    D_UINT32        ulBytesRead;
    D_BUFFER       *pBuff;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pulVerified, sizeof(*pulVerified));
    DclAssert(hFile);

    *pulVerified = 0;

    /*  Do we want to do misaligned I/O?
    */
    if(nOffset)
    {
        /*  Use a misaligned client buffer.
        */
        pBuff = pTI->pBuffer + 1;

        /*  Verify the first nOffset bytes separately so the remainder of
            the file is verified using misaligned file offsets.
        */            
        dclStat = VerifyFileSegment(pTI, hFile, pBuff, nOffset, 0, &ulBytesRead);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("VerifyFileHelper(A) file verification error, Status=%lX\n", dclStat);
            
            return dclStat;
        }

        ulOffset = ulBytesRead;

        *pulVerified = ulBytesRead;
    }
    else
    {
        /*  Use aligned I/O and client buffer
        */
        pBuff = pTI->pBuffer;

        ulOffset = 0;
    }
    
    do
    {
        dclStat = VerifyFileSegment(pTI, hFile, pBuff, pTI->ulBufferSize, ulOffset, &ulBytesRead);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("VerifyFileHelper(B) file verification error, Status=%lX\n", dclStat);
            
            return dclStat;
        }

        ulOffset += ulBytesRead;
        *pulVerified += ulBytesRead;
    }
    while(ulBytesRead != 0);

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("    Verified %lU bytes\n", ulOffset);
 
    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS VerifyFileSegment(
    FSIOTESTINFO   *pTI,
    DCLFSFILEHANDLE hFile,
    D_BUFFER       *pBuffer,
    size_t          nBufferLen,
    D_UINT32        ulFileOffset,
    D_UINT32       *pulBytesRead)
{
    D_UINT32        kk;
    DCLSTATUS       dclStat;
    
    dclStat = PRIMREAD(hFile, pBuffer, nBufferLen, pulBytesRead);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("    Unable to read from file, Status=%lX\n", dclStat));
        
        return dclStat;
    }

    /*  Verify that we read what we wrote...  This code must exactly match
        the data creation algorithm used in FileCreate().
    */
    for(kk = 0; kk < *pulBytesRead; kk++)
    {
        if(pBuffer[kk] != (D_BYTE)(ulFileOffset + kk))
        {
            DCLPRINTF(1, ("    Verification error at offset %lU, %02U != %02U\n",
                ulFileOffset + kk, pBuffer[kk], (D_BYTE)(ulFileOffset + kk)));

            if(!pTI->fNoVerify)
                return DCLSTAT_CURRENTLINE;
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS DeleteFileSet(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nCount,
    D_UINT32       *pulTotalMS,
    D_UINT32       *pulMaxUS,       /* may be NULL if not used */
    unsigned       *pnDeleted,
    D_BOOL          fVerbose)
{
    unsigned        kk;
    D_UINT64        ullTotalUS = 0;
    D_UINT32        ulMaxUS = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pulTotalMS, sizeof(*pulTotalMS));
    DclAssertWritePtr(pnDeleted, sizeof(*pnDeleted));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nCount <= FILESET_MAX);
    DclAssert(nCount);

    DCLPRINTF(2, ("    Starting File Set Deletion of up to %u files\n", nCount));

    for(kk = 0; kk < nCount; kk++)
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulUS;
        
        ts = DclTimeStamp();

        dclStat = FileDelete(pTI, pszPrefix, kk);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            /*  If we were instructed to delete "everything" (sequentially,
                mind you), then change the status code to "success".
            */    
            if((dclStat == DCLSTAT_FS_NOTFOUND) && (nCount == FILESET_MAX))
                dclStat = DCLSTAT_SUCCESS;

            if(dclStat != DCLSTAT_SUCCESS)                
                DclPrintf("    FSIO: Error deleting file set at file %u, Status=%lX\n", kk, dclStat);
            
            break;
        }

        ulUS = DclTimePassedUS(ts);
        ullTotalUS += ulUS;
        if(ulMaxUS < ulUS)
            ulMaxUS = ulUS;

        if(fVerbose || pTI->sti.nVerbosity > DCL_VERBOSE_LOUD)
            DclPrintf("    File %u delete required %6lU us\n", kk, ulUS);

         (*pnDeleted)++;
    }

    *pulTotalMS = (D_UINT32)((ullTotalUS + 500) / 1000);

    if(pulMaxUS)
        *pulMaxUS = ulMaxUS;

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS FileDelete(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nFileNum)
{
    char            szIOFile[MAX_FILESPEC_LEN];

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nFileNum <= FILESET_MAX);

    DclSNPrintf(szIOFile, -1, "%s%s%05u.DAT", pTI->szPath, pszPrefix, nFileNum);

    if(!pTI->fNoDelete)
        return PRIMDELETE(szIOFile);
    else
        return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Parameters:
       pTI - A pointer to the FSIOTESTINFO structure to use

    Notes:

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS RunTests(
    FSIOTESTINFO   *pTI)
{
    D_UINT32        ulIteration;

    /*  If the "/c" parameter was not used, we want to only do
        one iteration.
    */
    if(pTI->ulCount == 0)
        pTI->ulCount = 1;

    pTI->FSPrim.nVerbosity = pTI->sti.nVerbosity;

    for(ulIteration = 1; ulIteration <= pTI->ulCount; ulIteration++)
    {
        DCLSTATUS       dclStat;
        DCLTIMESTAMP    ts;
        
        if(pTI->ulCount > 1)
            DclPrintf("\n>>>>>>>>>>>>>>>>>>>>>>>>>> Starting Iteration %lU <<<<<<<<<<<<<<<<<<<<<<<<<<\n", ulIteration);

        /*  Reset the iteration counts...
        */
        pTI->ulIterationMS = 0;
        pTI->ulIterationSleepMS = 0;

        ts = DclTimeStamp();

        /*  Run the sequential I/O tests first because they are the most
            basic and important form of performance measurement, and doing
            them first helps ensure that they are more likely to be running
            on a freshly formatted disk.
        */
        if(pTI->fSequentialIO)
        {
            D_UINT32    ulBuffLen;
            unsigned    nRangeIteration = 1;
            char        szScaleBuff[16];

            DclPrintf("\nStarting Sequential I/O tests (/SEQ) using %6s files...      FreeSpace:%3s%%\n",
                DclScaleBytes(pTI->ulMaxDiskSpace, szScaleBuff, sizeof(szScaleBuff)),
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            for(ulBuffLen = pTI->ulBufferMin;
                ulBuffLen <= pTI->ulBufferSize;
                ulBuffLen = RangeNext(pTI, ulBuffLen))
            {
                if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
                {
                    DclPrintf("  Aligned I/O using %lU byte blocks and %u second samples\n",
                        ulBuffLen, pTI->nSampleRate);
                }
              
                if(pTI->fSeqRead)
                {
                    dclStat = SequentialRead(pTI, "SeqR", "SeqReadAligned", ulBuffLen, 0, nRangeIteration);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }

                if(pTI->fSeqWrite)
                {
                    dclStat = SequentialWrite(pTI, "SeqW", "SeqWriteAligned", ulBuffLen, 0, nRangeIteration);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }

                if(pTI->fSeqRewrite)
                {
                    dclStat = SequentialRewrite(pTI, "SeqRW", "SeqRewriteAligned", ulBuffLen, 0, nRangeIteration);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }

                nRangeIteration++;
            }

            /*  The sequential tests above are oriented towards best-case
                sequential I/O performance.  

                The following tests exercise pretty much the worst case
                performance, where the I/O buffer length and offset almost
                never fall on FS block sized boundaries, and the actual 
                memory buffer is misaligned as well.
            */                
            if((pTI->ulTestEmulationVersion >= DEFAULT_VERSION) && pTI->fSeqUnaligned)
            {
                D_UINT32    ulMisalignedBuff;
                unsigned    nAlignOffset;

                /*  Use an I/O size which is 1.75 times the size of the
                    FS block size, plus one byte.  This will pretty much
                    ensure that neither the I/O start nor the end will
                    hardly ever fall on a block sized boundary.

                    WARNING: pBuffer is allocated at twice the FS block
                             size to accommodate this test.  The assert
                             below ensures no overflow.
                */                    
                ulMisalignedBuff = pTI->ulFSBlockSize + (pTI->ulFSBlockSize - (pTI->ulFSBlockSize >> 2)) + 1;

                DclAssert(ulMisalignedBuff <= pTI->ulTestMemUsage);

                /*  Adjust the alignment ONLY if the fMisalign option
                    has NOT already been applied.
                */                    
                nAlignOffset = pTI->fMisalign ? 0 : 1;
                
                if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
                {
                    DclPrintf("  Unaligned I/O using %lU byte blocks and %u second samples\n",
                        ulMisalignedBuff, pTI->nSampleRate);
                }

                if(pTI->fSeqRead)
                {
                    dclStat = SequentialRead(pTI, "SeqRU", "SeqReadUnaligned", ulMisalignedBuff, nAlignOffset, 1);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }
                
                if(pTI->fSeqWrite)
                {
                    dclStat = SequentialWrite(pTI, "SeqWU", "SeqWriteUnaligned", ulMisalignedBuff, nAlignOffset, 1);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }
                
                if(pTI->fSeqRewrite)
                {
                    dclStat = SequentialRewrite(pTI, "SeqRWU", "SeqRewriteUnaligned", ulMisalignedBuff, nAlignOffset, 1);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;

                    AppSleep(pTI, 3);
                }
            }
        }

        if(pTI->fCreate)
        {
            DclPrintf("\nStarting the Create tests (/CREATE)...\n");

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = GetSmallFileIOPerf(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 2);
        }

        if(pTI->fFrag)
        {
            DclPrintf("\nStarting the Fragmentation tests (/FRAG)...                     FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = GetFragPerf(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 6);
        }

        if(pTI->fRandomIO)
        {
            DclPrintf("\nStarting the Random I/O tests (/RAND)...                        FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            if(pTI->fRandRead)
            {
                dclStat = RandomRead(pTI);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                AppSleep(pTI, 3);
            }

            if(pTI->fRandWrite)
            {
                dclStat = RandomWrite(pTI);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                AppSleep(pTI, 3);
            }

            if(pTI->ulTestEmulationVersion >= DEFAULT_VERSION)
            {
                if(pTI->fRandUnaligned)
                {
                    dclStat = RandomUnaligned(pTI);
                    if(dclStat != DCLSTAT_SUCCESS)
                        return dclStat;
    
                    AppSleep(pTI, 3);
                }
            }
        }

        if(pTI->fFillVolume)
        {
            DclPrintf("\nStarting the Fill Volume tests (/FILL)...                       FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = FillVolume(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 3);
        }

        if(pTI->fDirTest)
        {
            DclPrintf("\nStarting the Directory Stripe tests (/DIR)...                   FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = DirTest(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 3);
        }

        if(pTI->fScanTest)
        {
            DclPrintf("\nStarting the Directory Scan tests (/SCAN)...                    FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = ScanTest(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 3);
        }

        if(pTI->fSparseTest)
        {
            DclPrintf("\nStarting the Sparse tests (/SPARSE)...                          FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = SparseTest(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 3);
        }
        
        if(pTI->ulTestEmulationVersion >= DEFAULT_VERSION)
        {
            if(pTI->fTreeTest)
            {
                DclPrintf("\nStarting the Tree tests (/TREE)...                              FreeSpace:%3s%%\n",
                    GetFreeSpacePercent(pTI));

                if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                    DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

                dclStat = TreeTest(pTI);
                if(dclStat != DCLSTAT_SUCCESS)
                    return dclStat;

                AppSleep(pTI, 3);
            }
        }
        
        if(pTI->fWearLeveling)
        {
            DclPrintf("\nStarting the Static Wear-leveling tests (/WEAR)...              FreeSpace:%3s%%\n",
                GetFreeSpacePercent(pTI));

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
                DclPrintf("  Current seed:  %llU\n", VA64BUG(pTI->sti.ullRandomSeed));

            dclStat = WearLeveling(pTI);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            AppSleep(pTI, 6);
        }

        dclStat = PRIMFLUSH(pTI->hFlushFile);
        if(dclStat != DCLSTAT_SUCCESS)
            return dclStat;

        pTI->ulIterationMS = DclTimePassed(ts);

        DclAssert(pTI->ulIterationMS >= pTI->ulIterationSleepMS);

        DclPrintf("\n");

        /*-------------------------------------------------
            Display Iteration and/or Cumulative Totals
        -------------------------------------------------*/
        {
            char            szTempBuff[16];
            char            szTimeBuff[24];
            char           *pszTime = szTimeBuff;
            D_UINT64        ullIOTime;
            D_UINT64        ullOverheadSecs;
            D_UINT64        ullTempKB;
            D_UINT64        ullBytes;

            /*  If we are only doing 1 iteration, skip this and just
                display the cumulative totals...
            */
            if(pTI->ulCount > 1)
            {
                D_UINT32 ulMS;
                
                pszTime = DclTimeFormat((pTI->ulIterationMS+500)/1000, szTimeBuff, sizeof szTimeBuff);
                if(*pszTime == '0')
                    pszTime += 2;
                
                DclPrintf("Iteration complete: Time %s (includes %lU seconds sleeping)\n",
                    pszTime, (pTI->ulIterationSleepMS + 500) / 1000);

                ullTempKB = (pTI->FSPrim.ullReadBytes + 512) / 1024;
                ulMS = (D_UINT32)((pTI->FSPrim.ullReadTimeUS + 500) / 1000);
                
                DclPrintf("  File Data Read:    %6s MB in %10lU ms -- %8lU KB/sec\n",
                    DclRatio64(szTempBuff, sizeof(szTempBuff), pTI->FSPrim.ullReadBytes, 1024*1024, 1),
                    ulMS,
                    GetKBPerSecond(ullTempKB, ulMS));

                ullTempKB = (pTI->FSPrim.ullWriteBytes + 512) / 1024;
                ulMS = (D_UINT32)((pTI->FSPrim.ullWriteTimeUS + 500) / 1000);
                
                DclPrintf("  File Data Written: %6s MB in %10lU ms -- %8lU KB/sec\n\n",
                    DclRatio64(szTempBuff, sizeof(szTempBuff), pTI->FSPrim.ullWriteBytes, 1024*1024, 1),
                    ulMS,
                    GetKBPerSecond(ullTempKB, ulMS));
            }

            DclPrintf("Cumulative Totals\n");

            pTI->ulTotalMS      += pTI->ulIterationMS;
            pTI->ulTotalSleepMS += pTI->ulIterationSleepMS;

            DclTestFSPrimStatsAccumulate(&pTI->FSPrimTotal, &pTI->FSPrim);

            /*  Get total I/O time in seconds
            */
            ullIOTime = (pTI->FSPrimTotal.ullTotalTimeUS + 500000) / 1000000;

            pszTime = DclTimeFormat(ullIOTime, szTimeBuff, sizeof szTimeBuff);
            if(*pszTime == '0')
                pszTime += 2;

            DclPrintf("  I/O Time ------------------------------------------->  %8s hh:mm:ss\n", pszTime);

            if(pTI->ulTotalSleepMS)
            {
                pszTime = DclTimeFormat((pTI->ulTotalSleepMS + 500) / 1000, szTimeBuff, sizeof szTimeBuff);
                if(*pszTime == '0')
                    pszTime += 2;
        
                DclPrintf("  Sleep Time ----------------------------------------->  %8s hh:mm:ss\n", pszTime);
            }

            ullOverheadSecs = ((pTI->ulTotalMS - pTI->ulTotalSleepMS) + 500) / 1000;
            if(ullOverheadSecs > ullIOTime)
                ullOverheadSecs -= ullIOTime;
            else
                ullOverheadSecs = 0;
                
            pszTime = DclTimeFormat(ullOverheadSecs, szTimeBuff, sizeof szTimeBuff);
            if(*pszTime == '0')
                pszTime += 2;

            DclPrintf("  Test Overhead Time --------------------------------->  %8s hh:mm:ss\n", pszTime);

            pszTime = DclTimeFormat((pTI->ulTotalMS + 500) / 1000, szTimeBuff, sizeof szTimeBuff);
            if(*pszTime == '0')
                pszTime += 2;

            DclPrintf("  Total Test Execution Time -------------------------->  %8s hh:mm:ss\n", pszTime);

            ullBytes = (pTI->FSPrimTotal.ullReadBytes / 10) / pTI->ulOriginalFreeSpaceKB;
             
            DclPrintf("  Total File Data Read ------------------------------->%10s MB (%llU%%)\n",
                DclRatio64(szTempBuff, sizeof(szTempBuff), pTI->FSPrimTotal.ullReadBytes, 1024*1024, 1),
                VA64BUG(ullBytes));

            ullBytes = (pTI->FSPrimTotal.ullWriteBytes / 10) / pTI->ulOriginalFreeSpaceKB;
            
            DclPrintf("  Total File Data Written ---------------------------->%10s MB (%llU%%)\n",
                DclRatio64(szTempBuff, sizeof(szTempBuff), pTI->FSPrimTotal.ullWriteBytes, 1024*1024, 1),
                VA64BUG(ullBytes));

            DclPrintf("  Final File System Free Space ----------------------->%13s (%s%%)\n\n",
                DclScaleItems(FreeDiskBlocks(pTI), pTI->statfs.ulBlockSize, szTempBuff, sizeof(szTempBuff)), GetFreeSpacePercent(pTI));
            
            if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
                 DclTestFSPrimStatsDisplay(&pTI->FSPrimTotal);
        }
    }

    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("\n");
        DclPrintf("Note that the Cumulative and Iteration totals (if any) include data from ALL\n");
        DclPrintf("types of I/O, including the sequential I/O used to create the test files.  It\n");
        DclPrintf("is displayed in this form for academic cross-checking purposes <only> and should\n");
        DclPrintf("not be used as the \"general\" test results.  The results to use are found at the\n");
        DclPrintf("conclusion of each individual test.\n\n");
    }
    
    {
        /*---------------------------------------------------------
            Write data to the performance log, if enabled.  Note
            that any changes to the test name or category must be
            accompanied by changes to perffsio.bat.  Any changes to
            the actual data fields recorded here requires changes
            to the various spreadsheets which track this data.
        ---------------------------------------------------------*/
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "RawOps", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(  hPerfLog, "CrtCnt",             pTI->FSPrimTotal.ulCreateCount);
        DCLPERFLOG_NUM(  hPerfLog, "CrtMS",  (D_UINT32)((pTI->FSPrimTotal.ullCreateTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "OpenCnt",            pTI->FSPrimTotal.ulOpenCount);
        DCLPERFLOG_NUM(  hPerfLog, "OpenMS", (D_UINT32)((pTI->FSPrimTotal.ullOpenTimeUS + 500) / 1000));
        DCLPERFLOG_NUM64(hPerfLog, "RdCnt",              pTI->FSPrimTotal.ullReadCount);
        DCLPERFLOG_NUM(  hPerfLog, "RdKB",   (D_UINT32)((pTI->FSPrimTotal.ullReadBytes + 512) / 1024));
        DCLPERFLOG_NUM(  hPerfLog, "RdMS",   (D_UINT32)((pTI->FSPrimTotal.ullReadTimeUS + 500) / 1000));
        DCLPERFLOG_NUM64(hPerfLog, "WrtCnt",             pTI->FSPrimTotal.ullWriteCount);
        DCLPERFLOG_NUM(  hPerfLog, "WrtKB",  (D_UINT32)((pTI->FSPrimTotal.ullWriteBytes + 512) / 1024));
        DCLPERFLOG_NUM(  hPerfLog, "WrtMS",  (D_UINT32)((pTI->FSPrimTotal.ullWriteTimeUS + 500) / 1000));
        DCLPERFLOG_NUM64(hPerfLog, "SeekCnt",            pTI->FSPrimTotal.ullSeekCount);
        DCLPERFLOG_NUM(  hPerfLog, "SeekMS", (D_UINT32)((pTI->FSPrimTotal.ullSeekTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "FlCnt",              pTI->FSPrimTotal.ulFlushCount);
        DCLPERFLOG_NUM(  hPerfLog, "FlMS",   (D_UINT32)((pTI->FSPrimTotal.ullFlushTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "CloseCnt",           pTI->FSPrimTotal.ulCloseCount);
        DCLPERFLOG_NUM(  hPerfLog, "CloseMS",(D_UINT32)((pTI->FSPrimTotal.ullCloseTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "DelCnt",             pTI->FSPrimTotal.ulDeleteCount);
        DCLPERFLOG_NUM(  hPerfLog, "DelMS",  (D_UINT32)((pTI->FSPrimTotal.ullDeleteTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "DCrtCnt",            pTI->FSPrimTotal.ulDirCreateCount);
        DCLPERFLOG_NUM(  hPerfLog, "DCrtMS", (D_UINT32)((pTI->FSPrimTotal.ullDirCreateTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "DDelCnt",            pTI->FSPrimTotal.ulDirRemoveCount);
        DCLPERFLOG_NUM(  hPerfLog, "DDelMS", (D_UINT32)((pTI->FSPrimTotal.ullDirRemoveTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "DTDelCnt",           pTI->FSPrimTotal.ulDirTreeDeleteCount);
        DCLPERFLOG_NUM(  hPerfLog, "DTDelMS",(D_UINT32)((pTI->FSPrimTotal.ullDirTreeDeleteTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "StatCnt",            pTI->FSPrimTotal.ulStatFSCount);
        DCLPERFLOG_NUM(  hPerfLog, "StatMS", (D_UINT32)((pTI->FSPrimTotal.ullStatFSTimeUS + 500) / 1000));
        DCLPERFLOG_NUM(  hPerfLog, "TstSecs",           (pTI->ulTotalMS+500)/1000);
        DCLPERFLOG_NUM(  hPerfLog, "SlpSecs",           (pTI->ulTotalSleepMS+500)/1000);
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    if(pTI->sti.fPerfLog)
        ResourceUsage(pTI);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    RangeNext()

    Description
        This function calculates the next buffer size to use when
        the /SEQ tests are run using a range of buffer sizes.

    Parameters
        pTI            - A pointer to the FSIOTESTINFO structure
        ulLastBuffSize - The buffer size from the previous iteration

    Return Value
        Returns the buffer size for the next iteration.
-------------------------------------------------------------------*/
static D_UINT32 RangeNext(
    FSIOTESTINFO   *pTI,
    D_UINT32        ulLastBuffSize)
{
    D_UINT32        ulNextSize;

    DclAssert(pTI);
    DclAssert(pTI->ulBufferInc);
    DclAssert(ulLastBuffSize);

    if(pTI->ulBufferInc <= 10)
        ulNextSize = ulLastBuffSize * pTI->ulBufferInc;
    else
        ulNextSize = ulLastBuffSize + pTI->ulBufferInc;

    /*  Handle cases where we overflow.
    */
    if(ulNextSize < ulLastBuffSize)
        return D_UINT32_MAX;
    else
        return ulNextSize;
}


/*-------------------------------------------------------------------
    ResourceUsage()

    Description
        This function displays resource usage.  It is called at the
        end of the test cycle to ensure that a wide variety of
        operations have been performed.

    Parameters
        pTI - A pointer to the FSIOTESTINFO structure

    Return Value
        None
-------------------------------------------------------------------*/
static void ResourceUsage(
    FSIOTESTINFO           *pTI)
{
    DCLDECLAREREQUESTPACKET (MEMTRACK, STATS, stats);   /* DCLREQ_MEMTRACK_STATS */
    DCLSTATUS               dclStat;

    stats.dms.uStrucLen = sizeof(stats.dms);
    stats.fVerbose = FALSE;
    stats.fReset = FALSE;

    dclStat = DclServiceIoctl(pTI->sti.hDclInst, DCLSERVICE_MEMTRACK, &stats.ior);
    if((dclStat == DCLSTAT_SUCCESS) && (stats.dms.uStrucLen == sizeof(stats.dms)))
    {
        /*---------------------------------------------------------
            Write data to the performance log, if enabled.  Note
            that any changes to the test name or category must be
            accompanied by changes to perffsio.bat.  Any changes to
            the actual data fields recorded here requires changes
            to the various spreadsheets which track this data.
        ---------------------------------------------------------*/
        DCLPERFLOGHANDLE    hPerfLog;
        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "MemoryUsage", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(  hPerfLog, "Overhead",      stats.dms.uOverhead);
        DCLPERFLOG_NUM(  hPerfLog, "CurrentBytes",  stats.dms.ulBytesCurrentlyAllocated);
        DCLPERFLOG_NUM(  hPerfLog, "CurrentBlocks", stats.dms.ulBlocksCurrentlyAllocated);
        DCLPERFLOG_NUM(  hPerfLog, "MaxConcBytes",  stats.dms.ulMaxConcurrentBytes);
        DCLPERFLOG_NUM(  hPerfLog, "MaxConcBlocks", stats.dms.ulMaxConcurrentBlocks);
        DCLPERFLOG_NUM(  hPerfLog, "TotalAllocs",   stats.dms.ulTotalAllocCalls);
        DCLPERFLOG_NUM(  hPerfLog, "LargestAlloc",  stats.dms.ulLargestAllocation);
        DCLPERFLOG_NUM(  hPerfLog, "TestOverhead",  pTI->ulTestMemUsage);
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }
    else
    {
        DclPrintf("Memory usage statistics are disabled or unavailable, Status=%lX\n", dclStat);
    }

    return;
}


/*-------------------------------------------------------------------
    CreateTempFile()

    Description
        This function creates a new file using the specified name
        formatting template.

    Parameters
        pTI       - The FSIOTESTINFO structure to use
        pszFormat - The null-terminated format string which must
                    contain a %U or %X format specifier.
        pszBuffer - A buffer in which the full name is to be stored.
                    May be NULL if the name is not to be saved.

    Return Value
        Returns a file handle, or NULL upon failure.
-------------------------------------------------------------------*/
static DCLFSFILEHANDLE CreateTempFile(
    FSIOTESTINFO   *pTI,
    const char     *pszFormat,
    char           *pszBuffer)
{
    DCLFSFILEHANDLE hFile;
    unsigned        nLen;

    DclAssert(pTI);
    DclAssert(pszFormat);

    DclStrCpy(pTI->szFileSpec, pTI->szPath);

    nLen = DclStrLen(pTI->szFileSpec);

    DclSNPrintf(pTI->szFileSpec + nLen, -1, pszFormat, (D_UINT16)DclRand64(&pTI->sti.ullRandomSeed));

    if(PRIMCREATE(pTI->szFileSpec, &hFile) != DCLSTAT_SUCCESS)
        return NULL;

    if(pszBuffer)
        DclStrCpy(pszBuffer, pTI->szFileSpec);

    return hFile;
}


/*-------------------------------------------------------------------
    PopulateTempFile()

    Description
        This function creates a new file using the specified name
        formatting template, and populates it to the specified size.

    Parameters
        pTI       - The FSIOTESTINFO structure to use
        pszFormat - The null-terminated format string which must
                    contain a %U or %X format specifier.
        ulSize    - The file size.
        pszBuffer - A buffer in which the full name is to be stored.

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS PopulateTempFile(
    FSIOTESTINFO   *pTI,
    const char     *pszFormat,
    D_UINT32        ulSize,
    char           *pszBuffer)
{
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat;

    DclAssert(pTI);
    DclAssert(pszFormat);
    DclAssert(pszBuffer);
    DclAssert(ulSize);

    /*  Fill the write buffer with a known value.
    */
    DclMemSet(pTI->pBuffer, FILL_VALUE, pTI->ulBufferSize);

    /*  Create the file
    */
    hFile = CreateTempFile(pTI, pszFormat, pszBuffer);
    if(!hFile)
        return DCLSTAT_CURRENTLINE;

    /*  Create the file to use
    */
    while(ulSize)
    {
        size_t  nWritten;
        size_t  nThis = DCLMIN(pTI->ulBufferSize, ulSize);

        dclStat = PrimWrite(pTI, pTI->pBuffer, nThis, 1, hFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;

            DclPrintf("FSIO: Error writing to file, Status=%lX\n", dclStat);
            PRIMCLOSE(hFile);
            return dclStat;
        }

        ulSize -= nThis;
    }

    /*  Ensure that the test file is fully written to disk
    */
    dclStat = PRIMFLUSH(hFile);

    dclStat = PRIMCLOSE(hFile);

    return dclStat;
}


/*-------------------------------------------------------------------
    CreateUncachedFile()

    Description
        This function creates a new file using the specified name
        formatting template, and populates it to the specified size.
        This is done in such a fashion that the file should not be
        in the system disk cache.

    Parameters
        pTI       - The FSIOTESTINFO structure to use
        pszFormat - The null-terminated format string which must
                    contain a %U or %X format specifier.
        ulSize    - The file size.
        pszBuffer - A buffer in which the full name is to be stored.

    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS CreateUncachedFile(
    FSIOTESTINFO   *pTI,
    const char     *pszFormat,
    D_UINT32        ulSize,
    char           *pszBuffer)
{
    DCLFSFILEHANDLE hFile;
    D_UINT32        ulRemaining;
    DCLSTATUS       dclStat;
    char            szBuffer[MAX_FILESPEC_LEN];

    DclAssert(pTI);
    DclAssert(pszFormat);
    DclAssert(pszBuffer);
    DclAssert(ulSize);

    /*  Create a temporary file as big as we allow for the express purpose
        of reading it in piecemeal to cause the system disk cache to be
        flushed.  Any piece of our real target file should hopefully not
        be in the cache.
    */
    dclStat = PopulateTempFile(pTI, "fstt%04X.dat", pTI->ulMaxDiskSpace, szBuffer);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Now create our real target file.
    */
    dclStat = PopulateTempFile(pTI, pszFormat, ulSize, pszBuffer);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Open the first file and read it in piecemeal, hopefully pushing out
        whatever pieces of the second file that might be in the cache.
    */
    dclStat = PRIMOPEN(szBuffer, "r+b", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;    

    ulRemaining = pTI->ulMaxDiskSpace;
    while(ulRemaining)
    {
        D_UINT32 ulThis = DCLMIN(pTI->ulBufferSize, ulRemaining);

        dclStat = PRIMREAD(hFile, pTI->pBuffer, ulThis, NULL);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("FSIO: Error reading from file, Status=%lX\n", dclStat);
            PRIMCLOSE(hFile);
            return dclStat;
        }

        ulRemaining -= ulThis;
    }

    /*  Close and delete the temporary file.
    */
    PRIMCLOSE(hFile);

    if(!pTI->fNoDelete)
        PRIMDELETE(szBuffer);

    /*  Give any background processes time to clean up.
    */
    AppSleep(pTI, 3);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static D_BOOL CloseAndDeleteFile(
    FSIOTESTINFO   *pTI,
    DCLFSFILEHANDLE hFile)
{
    if(PRIMCLOSE(hFile) != DCLSTAT_SUCCESS)
    {
        DclPrintf("FSIO: Unable to close file!\n");
        return FALSE;
    }

    if(pTI->fNoDelete)
        return TRUE;

    if(PRIMDELETE(pTI->szFileSpec) == DCLSTAT_SUCCESS)
        return TRUE;
    else
        return FALSE;
}


/*-------------------------------------------------------------------
    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS SequentialRead(
    FSIOTESTINFO   *pTI,
    const char     *pszStatsPrefix,
    const char     *pszName,
    D_UINT32        ulBuffLen,
    unsigned        nBuffOffset,
    unsigned        nRangeIteration)
{
    DCLFSFILEHANDLE hFile;
    D_UINT32        ulCount;
    D_UINT32        i;
    D_UINT64        ullTmp;
    D_UINT64        ullTotalTimeUS = D_UINT64_MIN;
    D_UINT32        ulTotalTimeMS;
    D_UINT32        ulTotalRead;
    D_UINT32        ulLongestTimeUS;
    D_UINT32        ulSampleBlocks;

    /*  Unnecessary initialization to satisfy a neurotic compiler
    */
    D_UINT32        ulSampleTimeUS = 0;
    D_UINT32        ulSampleLongUS = 0;
    D_UINT32        ulLastTimeUS = 0;
    DCLTIMESTAMP    ts = DCLINITTS;

    unsigned        nSampleNum = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szScaleBuff[16];

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszStatsPrefix, 0);
    DclAssertReadPtr(pszName, 0);
    DclAssert(ulBuffLen);

    hFile = SequentialTestInit(pTI, "READ", "fssr%04X.dat", pTI->ulMaxDiskSpace, ulBuffLen);
    if(!hFile)
        return DCLSTAT_CURRENTLINE;

    DclTestInstrumentationStart(&pTI->sti);

    ulCount = pTI->ulMaxDiskSpace / ulBuffLen;

    ulTotalRead = 0;
    ulLongestTimeUS = 0;
    ulSampleBlocks = 0;

    for(i = 0; i < ulCount; i++)
    {
        D_UINT32    ulThisTimeUS;
        D_UINT32    ulElapsedTimeUS;
        D_UINT32    ulSampleMS;

        ulSampleBlocks++;

        /*  If starting a new sample...
        */
        if(ulSampleBlocks == 1)
        {
            ulSampleTimeUS = 0;
            ulSampleLongUS = 0;
            ulLastTimeUS = 0;
            ts = DclTimeStamp();
        }

        dclStat = PRIMREAD(hFile, pTI->pBuffer + nBuffOffset, ulBuffLen, NULL);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("  FSIO: Error reading from file, Status=%lX\n", dclStat);

            break;
        }

        ulThisTimeUS = DclTimePassedUS(ts);
        ulElapsedTimeUS = ulThisTimeUS - ulLastTimeUS;
        ulLastTimeUS = ulThisTimeUS;

        ulSampleTimeUS += ulElapsedTimeUS;
        DclUint64AddUint32(&ullTotalTimeUS, ulElapsedTimeUS);

        if(ulLongestTimeUS < ulElapsedTimeUS)
            ulLongestTimeUS = ulElapsedTimeUS;

        if(ulSampleLongUS < ulElapsedTimeUS)
            ulSampleLongUS = ulElapsedTimeUS;

        ulTotalRead += ulBuffLen;

        ulSampleMS = (ulSampleTimeUS + 500) / 1000;

        /*  If sampling on every block, OR this is the last sample OR
            our sample time has come...
        */
        if( (pTI->nSampleRate == 0) ||
            (i == ulCount-1) ||
            (ulSampleMS >= pTI->nSampleRate * 1000) )
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Sample %3u: %5lU ops in %5lUms, Long=%4lUms ---->%10lU KB/sec  %3s%%\n",
                    nSampleNum, ulSampleBlocks, ulSampleMS,
                    (ulSampleLongUS + 500) / 1000,
                    GetKBPerSecond((ulSampleBlocks * ulBuffLen) / 1024, ulSampleMS),
                    GetFreeSpacePercent(pTI));
            }
            
            nSampleNum++;

            /*  Resetting ulSampleBlocks to zero will cause all
                the counts to be reset.
            */
            ulSampleBlocks = 0;
        }
    }

    DclTestInstrumentationStop(&pTI->sti, pszStatsPrefix);

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ullTmp = ullTotalTimeUS;
    DclUint64AddUint32(&ullTmp, 500);
    DclUint64DivUint32(&ullTmp, 1000);
    ulTotalTimeMS = DclUint32CastUint64(&ullTmp);

    ullTmp = ullTotalTimeUS;
    DclUint64DivUint32(&ullTmp, ulCount);

    if(ulBuffLen % pTI->ulFSBlockSize)
    {
        DclPrintf("  %s Unaligned Reads ------------------------------>%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalRead / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
    else
    {
        DclPrintf("  %s Aligned Reads -------------------------------->%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalRead / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
        
    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Read %lU %lU byte blocks (%s) in %lU ms, Avg=%llU us, Long=%lU us\n",
            ulCount, ulBuffLen,
            DclScaleKB((ulCount * ulBuffLen) / 1024, szScaleBuff, sizeof(szScaleBuff)),
            ulTotalTimeMS, VA64BUG(ullTmp), ulLongestTimeUS);
    }

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    if(pTI->sti.fPerfLog)
    {
        DCLPERFLOGHANDLE    hPerfLog;
        char                szCategory[32];

        if(pTI->ulBufferMin != pTI->ulBufferSize)
        {
            /*  If we are iterating through a series of block sizes, use
                a sequentially increasing category number.

                Note that the default perffsio.bat batch file does not
                parse this.
            */
            DclSNPrintf(szCategory, sizeof(szCategory), "%s-%lU", pszStatsPrefix, nRangeIteration);
        }
        else
        {
            /*  Otherwise just use the standard category name.
            */
            DclStrCpy(szCategory, pszName);
        }

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", szCategory, NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB", pTI->ulMaxDiskSpace);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen", ulBuffLen);
        DCLPERFLOG_NUM(   hPerfLog, "KB/second", GetKBPerSecond(ulTotalRead / 1024, ulTotalTimeMS));
        DCLPERFLOG_NUM(   hPerfLog, "AverageUS", DclUint32CastUint64(&ullTmp));
        DCLPERFLOG_NUM(   hPerfLog, "MaximumUS", ulLongestTimeUS);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    if(!CloseAndDeleteFile(pTI, hFile))
        return DCLSTAT_CURRENTLINE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS SequentialWrite(
    FSIOTESTINFO   *pTI,
    const char     *pszStatsPrefix,
    const char     *pszName,
    D_UINT32        ulBuffLen,
    unsigned        nBuffOffset,
    unsigned        nRangeIteration)
{
    DCLFSFILEHANDLE hFile;
    D_UINT32        ulCount;
    D_UINT64        ullTmp;
    D_UINT64        ullTotalTimeUS = D_UINT64_MIN;
    D_UINT32        ulTotalTimeMS;
    D_UINT32        ulTotalWritten = 0;
    D_UINT32        ulLongestTimeUS = 0;
    D_UINT64        ullTmpUS;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szScaleBuff[16];

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszStatsPrefix, 0);
    DclAssertReadPtr(pszName, 0);
    DclAssert(ulBuffLen);

    hFile = SequentialTestInit(pTI, "WRITE", "fssw%04X.dat", 0L, ulBuffLen);
    if(!hFile)
        return DCLSTAT_CURRENTLINE;

    DclTestInstrumentationStart(&pTI->sti);

    ulCount = pTI->ulMaxDiskSpace / ulBuffLen;

    dclStat = WriteHelper(pTI, hFile, ulBuffLen, ulCount, nBuffOffset, 0, FALSE, &ulTotalWritten, &ulLongestTimeUS, &ullTotalTimeUS);

    /*  Ensure that everything is on the disk before we stop the profiler
        and dump the stats.  Note however that the time taken to do this
        flush is included <only> in the total test time, and not as part
        of the last sample.  This ensures that the sample times are not
        unfairly skewed, since the flush could be writing data from many
        previous samples.
    */
    ullTmpUS = pTI->FSPrim.ullFlushTimeUS;
    PRIMFLUSH(hFile);
    ullTotalTimeUS += pTI->FSPrim.ullFlushTimeUS - ullTmpUS;

    DclTestInstrumentationStop(&pTI->sti, pszStatsPrefix);

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ullTmp = ullTotalTimeUS;
    DclUint64AddUint32(&ullTmp, 500);
    DclUint64DivUint32(&ullTmp, 1000);
    ulTotalTimeMS = DclUint32CastUint64(&ullTmp);

    ullTmp = ullTotalTimeUS;
    DclUint64DivUint32(&ullTmp, ulCount);

    if(ulBuffLen % pTI->ulFSBlockSize)
    {
        DclPrintf("  %s Unaligned Writes ----------------------------->%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalWritten / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
    else
    {
        DclPrintf("  %s Aligned Writes ------------------------------->%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalWritten / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
        
    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Wrote %lU %lU byte blocks (%s) in %lU ms, Avg=%llU us, Long=%lU us\n",
            ulCount, ulBuffLen,
            DclScaleKB((ulCount * ulBuffLen) / 1024, szScaleBuff, sizeof(szScaleBuff)),
            ulTotalTimeMS, VA64BUG(ullTmp), ulLongestTimeUS);
    }

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    if(pTI->sti.fPerfLog)
    {
        DCLPERFLOGHANDLE    hPerfLog;
        char                szCategory[32];

        if(pTI->ulBufferMin != pTI->ulBufferSize)
        {
            /*  If we are iterating through a series of block sizes, use
                a sequentially increasing category number.

                Note that the default perffsio.bat batch file does not
                parse this.
            */
            DclSNPrintf(szCategory, sizeof(szCategory), "%s-%lU", pszStatsPrefix, nRangeIteration);
        }
        else
        {
            /*  Otherwise just use the standard category name.
            */
            DclStrCpy(szCategory, pszName);
        }

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", szCategory, NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB", pTI->ulMaxDiskSpace);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen", ulBuffLen);
        DCLPERFLOG_NUM(   hPerfLog, "KB/second", GetKBPerSecond(ulTotalWritten / 1024, ulTotalTimeMS));
        DCLPERFLOG_NUM(   hPerfLog, "AverageUS", DclUint32CastUint64(&ullTmp));
        DCLPERFLOG_NUM(   hPerfLog, "MaximumUS", ulLongestTimeUS);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    if(!CloseAndDeleteFile(pTI, hFile))
        return DCLSTAT_CURRENTLINE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Return Value
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS SequentialRewrite(
    FSIOTESTINFO   *pTI,
    const char     *pszStatsPrefix,
    const char     *pszName,
    D_UINT32        ulBuffLen,
    unsigned        nBuffOffset,
    unsigned        nRangeIteration)
{
    DCLFSFILEHANDLE hFile;
    D_UINT32        ulCount;
    D_UINT32        i;
    D_UINT64        ullTmp;
    D_UINT64        ullTotalTimeUS = D_UINT64_MIN;
    D_UINT32        ulTotalTimeMS;
    D_UINT32        ulTotalRewritten;
    D_UINT32        ulLongestTimeUS;

    /*  Unnecessary initialization to satisfy a neurotic compiler
    */
    D_UINT32        ulSampleBlocks = 0;
    D_UINT32        ulSampleTimeUS = 0;
    D_UINT32        ulSampleLongUS = 0;
    D_UINT32        ulLastTimeUS = 0;
    DCLTIMESTAMP    ts = DCLINITTS;

    D_UINT64        ullTmpUS;
    unsigned        nSampleNum = 1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    char            szScaleBuff[16];

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszStatsPrefix, 0);
    DclAssertReadPtr(pszName, 0);
    DclAssert(ulBuffLen);

    hFile = SequentialTestInit(pTI, "REWRITE", "fsse%04X.dat", pTI->ulMaxDiskSpace, ulBuffLen);
    if(!hFile)
        return DCLSTAT_CURRENTLINE;

    DclTestInstrumentationStart(&pTI->sti);

    ulCount = pTI->ulMaxDiskSpace / ulBuffLen;

    ulTotalRewritten = 0;
    ulLongestTimeUS = 0;
    ulSampleBlocks = 0;

    for(i = 0; i < ulCount; i++)
    {
        D_UINT32    ulThisTimeUS;
        D_UINT32    ulElapsedTimeUS;
        D_UINT32    ulSampleMS;
        size_t      nWritten;

        ulSampleBlocks++;

        /*  If starting a new sample...
        */
        if(ulSampleBlocks == 1)
        {
            ulSampleTimeUS = 0;
            ulSampleLongUS = 0;
            ulLastTimeUS = 0;
            ts = DclTimeStamp();
        }

        dclStat = PrimWrite(pTI, pTI->pBuffer + nBuffOffset, ulBuffLen, 1, hFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;

            break;
        }

        /*  The time is measured only to the resolution of a clock tick,
            which could be many milliseconds.  The elapsed time will average
            out over the long term, and the longest times will likely be
            reasonably accurate as they are probably multiple ticks.
        */
        ulThisTimeUS = DclTimePassedUS(ts);
        ulElapsedTimeUS = ulThisTimeUS - ulLastTimeUS;
        ulLastTimeUS = ulThisTimeUS;

        ulSampleTimeUS += ulElapsedTimeUS;
        DclUint64AddUint32(&ullTotalTimeUS, ulElapsedTimeUS);

        if(ulLongestTimeUS < ulElapsedTimeUS)
            ulLongestTimeUS = ulElapsedTimeUS;

        if(ulSampleLongUS < ulElapsedTimeUS)
            ulSampleLongUS = ulElapsedTimeUS;

        ulTotalRewritten += ulBuffLen;

        ulSampleMS = (ulSampleTimeUS + 500) / 1000;

        /*  If sampling on every block, OR this is the last sample OR
            our sample time has come...
        */
        if( (pTI->nSampleRate == 0) ||
            (i == ulCount-1) ||
            (ulSampleMS >= pTI->nSampleRate * 1000) )
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Sample %3u: %5lU ops in %5lUms, Long=%4lUms ---->%10lU KB/sec  %3s%%\n",
                    nSampleNum, ulSampleBlocks, ulSampleMS, 
                    (ulSampleLongUS + 500) / 1000, 
                    GetKBPerSecond((ulSampleBlocks * ulBuffLen) / 1024, ulSampleMS),
                    GetFreeSpacePercent(pTI));
            }
            
            nSampleNum++;

            /*  Resetting ulSampleBlocks to zero will cause all
                the counts to be reset.
            */
            ulSampleBlocks = 0;
        }
    }

    /*  Ensure that everything is on the disk before we stop the profiler
        and dump the stats.  Note however that the time taken to do this
        flush is included <only> in the total test time, and not as part
        of the last sample.  This ensures that the sample times are not
        unfairly skewed, since the flush could be writing data from many
        previous samples.
    */
    ullTmpUS = pTI->FSPrim.ullFlushTimeUS;
    PRIMFLUSH(hFile);
    ullTotalTimeUS += pTI->FSPrim.ullFlushTimeUS - ullTmpUS;

    DclTestInstrumentationStop(&pTI->sti, pszStatsPrefix);

    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ullTmp = ullTotalTimeUS;
    DclUint64AddUint32(&ullTmp, 500);
    DclUint64DivUint32(&ullTmp, 1000);
    ulTotalTimeMS = DclUint32CastUint64(&ullTmp);

    ullTmp = ullTotalTimeUS;
    DclUint64DivUint32(&ullTmp, ulCount);

    if(ulBuffLen % pTI->ulFSBlockSize)
    {
        DclPrintf("  %s Unaligned Rewrites --------------------------->%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalRewritten / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
    else
    {
        DclPrintf("  %s Aligned Rewrites ----------------------------->%10lU KB/sec  %3s%%\n",
            (ulBuffLen >= pTI->ulBufferSize / 2) ? "Large" : "Small",
            GetKBPerSecond(ulTotalRewritten / 1024, ulTotalTimeMS),
            GetFreeSpacePercent(pTI));
    }
        
    if(pTI->sti.nVerbosity > DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Rewrote %lU %lU byte blocks (%s) in %lU ms, Avg=%llU us, Long=%lU us\n",
            ulCount, ulBuffLen,
            DclScaleKB((ulCount * ulBuffLen) / 1024, szScaleBuff, sizeof(szScaleBuff)),
            ulTotalTimeMS, VA64BUG(ullTmp), ulLongestTimeUS);
    }

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    if(pTI->sti.fPerfLog)
    {
        DCLPERFLOGHANDLE    hPerfLog;
        char                szCategory[32];

        if(pTI->ulBufferMin != pTI->ulBufferSize)
        {
            /*  If we are iterating through a series of block sizes, use
                a sequentially increasing category number.

                Note that the default perffsio.bat batch file does not
                parse this.
            */
            DclSNPrintf(szCategory, sizeof(szCategory), "%s-%lU", pszStatsPrefix, nRangeIteration);
        }
        else
        {
            /*  Otherwise just use the standard category name.
            */
            DclStrCpy(szCategory, pszName);
        }

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", szCategory, NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(   hPerfLog, "FileLenKB", pTI->ulMaxDiskSpace);
        DCLPERFLOG_NUM(   hPerfLog, "IOBuffLen", ulBuffLen);
        DCLPERFLOG_NUM(   hPerfLog, "KB/second", GetKBPerSecond(ulTotalRewritten / 1024, ulTotalTimeMS));
        DCLPERFLOG_NUM(   hPerfLog, "AverageUS", DclUint32CastUint64(&ullTmp));
        DCLPERFLOG_NUM(   hPerfLog, "MaximumUS", ulLongestTimeUS);
        DCLPERFLOG_WRITE( hPerfLog);
        DCLPERFLOG_CLOSE( hPerfLog);
    }

    if(!CloseAndDeleteFile(pTI, hFile))
        return DCLSTAT_CURRENTLINE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLFSFILEHANDLE SequentialTestInit(
    FSIOTESTINFO   *pTI,
    const char     *pszType,
    const char     *pszFormatStr,
    D_UINT32        ulSize,
    D_UINT32        ulBuffLen)
{
    DCLFSFILEHANDLE hFile;
    D_UINT32        ulCount;
    D_UINT32        i;

    ulCount = ulSize / pTI->ulBufferSize;
/*
    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("  %s -- BlockSize=%lU bytes, FileSize=%lUKB, Sampling every %u seconds\n",
            pszType, ulBuffLen, (pTI->ulBufferSize * ulCount) / 1024, pTI->nSampleRate);
    }
*/    
    if(pTI->ulMaxDiskSpace < pTI->ulBufferSize)
    {
        DclPrintf("    FSIO: Sequential test init parameter error, file size is less than buffer size\n");
        return NULL;
    }

    hFile = CreateTempFile(pTI, pszFormatStr, NULL);
    if(!hFile)
        return NULL;

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("    Creating %lUKB test file '%s'\n", ulSize / 1024L, (const char *) pTI->szFileSpec);

    if(PRIMSEEK(hFile, 0, DCLFSFILESEEK_SET) != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Error seeking to beginning of file!\n");
        return NULL;
    }

    for(i = 0; i < ulCount; i++)
    {
        size_t      nWritten;
        DCLSTATUS   dclStat;
            
        dclStat = PrimWrite(pTI, pTI->pBuffer, pTI->ulBufferSize, 1, hFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
            return NULL;
    }

    if(PRIMFLUSH(hFile) != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Unable to flush file!\n");
        return NULL;
    }

    if(PRIMCLOSE(hFile) != DCLSTAT_SUCCESS)
    {
        DclPrintf("    FSIO: Unable to close file!\n");
        return NULL;
    }

    if(PRIMOPEN(pTI->szFileSpec, "r+b", &hFile) != DCLSTAT_SUCCESS)
        return NULL;

    return hFile;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS WriteHelper(
    FSIOTESTINFO   *pTI,
    DCLFSFILEHANDLE hFile,
    D_UINT32        ulIOSize,
    D_UINT32        ulCount,
    unsigned        nBuffOffset,
    D_UINT32        ulFileOffset,
    D_BOOL          fInitData,
    D_UINT32       *pulTotalWritten,
    D_UINT32       *pulLongestTimeUS,
    D_UINT64       *pullTotalTimeUS)
{
    D_UINT32        ii;
    D_UINT32        ulSampleBlocks = 0;
    D_UINT32        ulSampleTimeUS = 0; /* Unnecessary init to satisfy a picky compiler */
    D_UINT32        ulSampleLongUS = 0; /* "" */
    D_UINT32        ulLastTimeUS = 0;   /* "" */
    DCLTIMESTAMP    ts = DCLINITTS;     /* "" */
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    unsigned        nSampleNum = 1;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssert(hFile);
    DclAssert(ulIOSize);
    DclAssert(ulCount);
    DclAssertWritePtr(pulTotalWritten, sizeof(*pulTotalWritten));      
    DclAssertWritePtr(pulLongestTimeUS, sizeof(*pulLongestTimeUS));    

    /*  If so instructed, initialize the data using a simple algorithm where
        each byte contains the low 8-bits of its offset in the file.
    */  
    if(fInitData)
    {
        D_UINT32    nn;
        
        for(nn = 0; nn < ulIOSize; nn++)
            pTI->pBuffer[nBuffOffset + nn] = (D_BYTE)(nn + ulFileOffset);
    }
    
    for(ii = 0; ii < ulCount; ii++)
    {
        D_UINT32    ulThisTimeUS;
        D_UINT32    ulElapsedTimeUS;
        D_UINT32    ulSampleMS;
        size_t      nWritten;

        ulSampleBlocks++;

        /*  If starting a new sample...
        */
        if(ulSampleBlocks == 1)
        {
            ulSampleTimeUS = 0;
            ulSampleLongUS = 0;
            ulLastTimeUS = 0;
            ts = DclTimeStamp();
        }

        dclStat = PrimWrite(pTI, pTI->pBuffer + nBuffOffset, ulIOSize, 1, hFile, &nWritten);
        if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
        {
            if(dclStat == DCLSTAT_SUCCESS)
                dclStat = DCLSTAT_FS_WRITEFAILED;
        }
        else
        {
            *pulTotalWritten += ulIOSize;
        }

        ulThisTimeUS = DclTimePassedUS(ts);
        ulElapsedTimeUS = ulThisTimeUS - ulLastTimeUS;
        ulLastTimeUS = ulThisTimeUS;

        ulSampleTimeUS += ulElapsedTimeUS;
        DclUint64AddUint32(pullTotalTimeUS, ulElapsedTimeUS);

        if(*pulLongestTimeUS < ulElapsedTimeUS)
            *pulLongestTimeUS = ulElapsedTimeUS;

        if(ulSampleLongUS < ulElapsedTimeUS)
            ulSampleLongUS = ulElapsedTimeUS;

        ulSampleMS = (ulSampleTimeUS + 500) / 1000;

        /*  If sampling on every block, OR this is the last sample OR
            our sample time has come OR the operation failed...
        */
        if( (pTI->nSampleRate == 0) ||
            (ii == ulCount-1) ||
            (ulSampleMS >= pTI->nSampleRate * 1000) ||
            (dclStat != DCLSTAT_SUCCESS) )
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("    Sample %3u: %5lU ops in %5lUms, Long=%4lUms ---->%10lU KB/sec  %3s%%\n",
                    nSampleNum, ulSampleBlocks, ulSampleMS,
                    (ulSampleLongUS + 500) /  1000,
                    GetKBPerSecond((ulSampleBlocks * ulIOSize) / 1024, ulSampleMS),
                    GetFreeSpacePercent(pTI));
            }
            
            nSampleNum++;

            /*  Resetting ulSampleBlocks to zero will cause all
                the counts to be reset.
            */
            ulSampleBlocks = 0;
        }

        if(dclStat != DCLSTAT_SUCCESS) 
            break;
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS WearLeveling(
    FSIOTESTINFO   *pTI)
{
    #define         WEARLEV_SELECT      (0)
    #define         WEARLEV_CREATE      (1)
    #define         WEARLEV_DELETE      (2)
    #define         WEARLEV_OVERWRITE   (3)
    D_UINT32        ulInitialFreeBlocks;
    D_UINT32        ulWorkingFreeBlocks;
    D_UINT32        ulCurrentFreeBlocks;
    D_UINT32        ulThresholdBlocks;
    D_UINT32        ulWorkFileSize;
    D_UINT32        ulFilesCreated = 0;
    D_UINT32        ulFilesDeleted = 0;
    D_UINT32        ulFilesOverwritten = 0;
    D_UINT32        ulIterations = 0;
    unsigned        nMaxWorkFiles = 0;
    unsigned        nBlocksPerFile;
    unsigned        nStaticFileCount = 0;
    unsigned        nFileNum;
    D_BUFFER       *pBitmap = NULL;
    DCLTIMESTAMP    ts;
    DCLTIMESTAMP    tsF;
    DCLSTATUS       dclStat = DCLSTAT_CURRENTLINE;

    ulInitialFreeBlocks = FreeDiskBlocks(pTI);

    /*  Assume that the work file size will be small, unless we determine
        otherwise later.
    */
    ulWorkFileSize = pTI->ulMaxDiskSpace / 8;

    DclTestInstrumentationStart(&pTI->sti);

    if(pTI->nWearLevStaticPercent && (ulInitialFreeBlocks > 500))
    {
        D_INT32     lStaticBlocks;
        D_INT32     lLastFileBlocksUsed = 0;

        /*  Divide first, then multiply to avoid overflow.  Don't worry about
            losing accuracy as the end-result of creating the static data is
            only going to be approximate anyway...
        */
        lStaticBlocks = (ulInitialFreeBlocks / 100) * pTI->nWearLevStaticPercent;

        DclPrintf("Filling ~%u%% of the disk with static data, using %lU KB files...\n",
            pTI->nWearLevStaticPercent, pTI->ulMaxDiskSpace/1024);

        while(lStaticBlocks > lLastFileBlocksUsed)
        {
            D_UINT32    ulFreeBlocks = FreeDiskBlocks(pTI);

            dclStat = FileCreate(pTI, "WLS", nStaticFileCount, 1, pTI->ulMaxDiskSpace, NULL, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error creating the static data files (FileNum=%u) Error=%lX\n", nStaticFileCount, dclStat);
                goto WearLevCleanup;
            }

            lLastFileBlocksUsed = (D_INT32)(ulFreeBlocks - FreeDiskBlocks(pTI));
            lStaticBlocks -= lLastFileBlocksUsed;

            nStaticFileCount++;
        }

        DclPrintf("%u static file(s) fill %u%% of the available disk space (includes FS metadata)\n",
            nStaticFileCount, 100-((FreeDiskBlocks(pTI)*100)/ulInitialFreeBlocks));
    }
    else
    {
        DclPrintf("Using no static data due to the test configuration (or the disk is too small)\n");
    }

    DclTestInstrumentationStop(&pTI->sti, "WrLvS");
    DclTestInstrumentationStart(&pTI->sti);

    /*  Determine the number of WorkingFreeBlocks after having created the
        static data.
    */
    ulWorkingFreeBlocks = FreeDiskBlocks(pTI);

    /*  Threshold blocks is 25% of whatever WorkingFreeBlocks is
    */
    ulThresholdBlocks = ulWorkingFreeBlocks >> 2;

    /*  Increase the size of the files we will use if the total working
        file count is over a certain threshold.
    */
    if((ulWorkingFreeBlocks * pTI->statfs.ulBlockSize) / ulWorkFileSize > 1000)
        ulWorkFileSize = pTI->ulMaxDiskSpace;

    /*  Data blocks per file, plus a guesstimated 1 metadata block.
    */
    nBlocksPerFile = (ulWorkFileSize / pTI->statfs.ulBlockSize) + 1;

    /*  Given the number of WorkingFreeBlocks, calculate the maximum number
        of files we will use.
    */
    nMaxWorkFiles = (unsigned)((ulWorkingFreeBlocks * pTI->statfs.ulBlockSize) / ulWorkFileSize);

    /*  Allocate a bitmap with 1 bit for each potential file we will create.
    */
    pBitmap = DclMemAllocZero(DCLBITMAPDIMENSION(nMaxWorkFiles));
    if(!pBitmap)
    {
        DclPrintf("Error allocating bitmap memory\n");
        dclStat = DCLSTAT_OUTOFMEMORY;
        goto WearLevCleanup;
    }

    DclPrintf("Randomly creating, overwriting, and deleting %lU KB files, using up to 75%% of\n", ulWorkFileSize / 1024);
    DclPrintf("the <remaining> disk space, for %lU seconds...\n", pTI->sti.ulTestSeconds);

    ulCurrentFreeBlocks = ulWorkingFreeBlocks;

    ts = DclTimeStamp();
    while(DclTimePassed(ts) < pTI->sti.ulTestSeconds * 1000)
    {
        unsigned    fAffinity;
        unsigned    fAction;

        /*  Based on the number of current free blocks on the disk,
            determine if we should have an affinity for creating,
            deleting, or randomly choosing an action.
        */
        if(ulCurrentFreeBlocks > ulWorkingFreeBlocks - ulThresholdBlocks) 
        {
            /*  If the current number of free blocks is greater than
                75% of the original number of working free blocks,
                always create a file, and set the affinity to "create".
            */
            fAffinity = WEARLEV_CREATE;
            fAction = WEARLEV_CREATE;
        }
        else if(ulCurrentFreeBlocks < ulThresholdBlocks) 
        {
            /*  If the current number of free blocks is less than
                25% of the original number of working free blocks,
                always delete a file, and set the affinity to "delete".
            */
            fAffinity = WEARLEV_DELETE;
            fAction = WEARLEV_DELETE;
        }
        else
        {
            /*  The number of free blocks is somewhere between 25% and 75%
                of the original number of working free blocks.  Setting
                the action to "select" will cause a semi-random action to
                be done.  Could be a create, could be a delete, could be a
                file overwrite.
            */
            fAffinity = WEARLEV_CREATE;
            fAction = WEARLEV_SELECT;
        }

        while(TRUE)
        {
            nFileNum = (unsigned)DclRand64(&pTI->sti.ullRandomSeed) % nMaxWorkFiles;

            if(fAction == WEARLEV_CREATE)
            {
                /*  We're in create mode -- examine the bitmap -- if the
                    file already exists, continue and use a different
                    random file number.
                */
                if(DCLBITGET(pBitmap, nFileNum) == 1)
                    continue;
            }
            else if(fAction == WEARLEV_DELETE)
            {
                /*  We're in delete mode -- examine the bitmap -- if the
                    file does not exist, continue and use a different
                    random file number.
                */
                if(DCLBITGET(pBitmap, nFileNum) == 0)
                    continue;
            }
            else
            {
                DclAssert(fAction == WEARLEV_SELECT);
                DclAssert(fAffinity == WEARLEV_CREATE || fAffinity == WEARLEV_DELETE);

                /*  In "select" mode, if the file does not yet exist, it
                    will be created.  If the file already exists, it will
                    be deleted if the current affinity is "delete", or
                    overwritten if the current affinity is "create".
                */
                if(DCLBITGET(pBitmap, nFileNum) == 0)
                    fAction = WEARLEV_CREATE;
                else
                    fAction = (fAffinity == WEARLEV_DELETE ? WEARLEV_DELETE : WEARLEV_OVERWRITE);
           }

           break;
        }

        if(fAction == WEARLEV_DELETE)
        {
            DclAssert(DCLBITGET(pBitmap, nFileNum));

            tsF = DclTimeStamp();

            dclStat = DeleteNumberedFile(pTI, "WLW", nFileNum);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error deleting numbered file %u, status=%lX\n", nFileNum, dclStat);
                goto WearLevCleanup;
            }

            ulFilesDeleted++;

            ulCurrentFreeBlocks += nBlocksPerFile;

            /*  Clear the entry in the bitmap
            */
            DCLBITCLEAR(pBitmap, nFileNum);

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("File %4u deleted     : Free blocks remaining: %6lU  Time:%4lU ms\n",
                    nFileNum, FreeDiskBlocks(pTI), DclTimePassed(tsF));
            }
        }
        else if(fAction == WEARLEV_CREATE)
        {
            DclAssert(!DCLBITGET(pBitmap, nFileNum));

            tsF = DclTimeStamp();

            dclStat = FileCreate(pTI, "WLW", nFileNum, 1, ulWorkFileSize, NULL, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error creating numbered file %u, Error=%lX\n", nFileNum, dclStat);
                goto WearLevCleanup;
            }

            ulFilesCreated++;

            ulCurrentFreeBlocks -= nBlocksPerFile;

            /*  Set the entry in the bitmap
            */
            DCLBITSET(pBitmap, nFileNum);

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("File %4u created     : Free blocks remaining: %6lU  Time:%4lU ms\n",
                    nFileNum, FreeDiskBlocks(pTI), DclTimePassed(tsF));
            }
        }
        else
        {
            DclAssert(DCLBITGET(pBitmap, nFileNum));
            DclAssert(fAction == WEARLEV_OVERWRITE);

            tsF = DclTimeStamp();

            dclStat = FileCreate(pTI, "WLW", nFileNum, 1, ulWorkFileSize, NULL, FALSE);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error overwriting numbered file %u, Error=%lX\n", nFileNum, dclStat);
                goto WearLevCleanup;
            }

            ulFilesOverwritten++;

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            {
                DclPrintf("File %4u overwritten : Free blocks remaining: %6lU  Time:%4lU ms\n",
                    nFileNum, FreeDiskBlocks(pTI), DclTimePassed(tsF));
            }
        }

        ulIterations++;

        /*  Every 500th operation, sleep for a short while
        */
        if(!(ulIterations % 500))
            AppSleep(pTI, 2);
    }

    DclPrintf("Created   %5lU files containing approximately %6lU MB of data\n",
        ulFilesCreated, (ulFilesCreated * (ulWorkFileSize/1024)) / 1024);
    DclPrintf("Deleted   %5lU files containing approximately %6lU MB of data\n",
        ulFilesDeleted, (ulFilesDeleted * (ulWorkFileSize/1024)) / 1024);
    DclPrintf("Overwrote %5lU files containing approximately %6lU MB of data\n",
        ulFilesOverwritten, (ulFilesOverwritten * (ulWorkFileSize/1024)) / 1024);

  WearLevCleanup:

    DclTestInstrumentationStop(&pTI->sti, "WrLvW");

    DclPrintf("Removing test files...\n");

    if(pBitmap)
    {
        for(nFileNum=0; nFileNum<nMaxWorkFiles; nFileNum++)
        {
            if(DCLBITGET(pBitmap, nFileNum))
            {
                dclStat = DeleteNumberedFile(pTI, "WLW", nFileNum);
                if(dclStat != DCLSTAT_SUCCESS)
                {
                    DclPrintf("Error deleting numbered file %u, status=%lX\n", nFileNum, dclStat);
                }
            }
        }

        DclMemFree(pBitmap);
    }

    for(nFileNum=0; nFileNum<nStaticFileCount; nFileNum++)
    {
        dclStat = DeleteNumberedFile(pTI, "WLS", nFileNum);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Error deleting numbered file %u, status=%lX\n", nFileNum, dclStat);
        }
    }

    DclTestStatsDump(&pTI->sti, "WrLvF");

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: PrimWrite()

    All the file write logic in this module is funnelled through this
    function so that we can track cumulative performance statistics 
    for the entire test, regardless what options may be used.
-------------------------------------------------------------------*/
static DCLSTATUS PrimWrite(
    FSIOTESTINFO       *pTI,
    D_BUFFER           *pBuffer,
    size_t              nElementLen,
    size_t              nNumElements,
    DCLFSFILEHANDLE     hFile,
    size_t             *pnElementsWritten)
{
    D_UINT32            ulWritten = 0;
    DCLSTATUS           dclStat;

    DclAssert(pTI);
    DclAssert(pBuffer);
    DclAssert(hFile);

    if(!pTI->fWriteVerify)
    {
        unsigned    nInitLen = nElementLen * nNumElements;
        unsigned    ii;
        D_UINT64    ullOldRandSeed;
        D_UINT32   *pulBuff = (D_UINT32*)pBuffer;

        /*  Deal with the case where the buffer is not aligned.  In this
            event, the first few bytes of the buffer, as well as possibly
            the last few bytes of the buffer will remain in their original
            state.
        */            
        if(!DCLISALIGNED((D_UINTPTR)pulBuff, sizeof(*pulBuff)))
        {
            unsigned nAdjust = (sizeof(*pulBuff) - ((D_UINTPTR)pulBuff & (sizeof(*pulBuff) - 1)));

            DclAssert(nAdjust);
            DclAssert(nAdjust < sizeof(*pulBuff));
            
            pulBuff  = (D_UINT32*)(((char*)pulBuff) + nAdjust);

            DclAssert(DCLISALIGNED((D_UINTPTR)pulBuff, sizeof(*pulBuff)));

            if(nAdjust < nInitLen)
                nInitLen -= nAdjust;
            else
                nInitLen = 0;
        }
        
        if(!DCLISALIGNED(nInitLen, sizeof(*pulBuff)))
            nInitLen &= ~(sizeof(*pulBuff) - 1);

        DclAssert(DCLISALIGNED((D_UINTPTR)pulBuff, sizeof(*pulBuff)));
        DclAssert(DCLISALIGNED(nInitLen, sizeof(*pulBuff)));
        
        /*  Save the original random seed, which we will restore at the end
            of this function.  We do this because this test may call the rand
            function a variable number of times from run-to-run, resulting
            in a non-deterministic random seed.
        */
        ullOldRandSeed = pTI->sti.ullRandomSeed;

        for(ii = 0; ii < nInitLen / sizeof(D_UINT32); ii++)
        {
            pulBuff[ii] = (D_UINT32)DclRand64(&pTI->sti.ullRandomSeed);
        }

        pTI->sti.ullRandomSeed = ullOldRandSeed;
    }

    dclStat = PRIMWRITE(hFile, pBuffer, nElementLen * nNumElements, &ulWritten);

    *pnElementsWritten = ulWritten / nElementLen;

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: CreateAndAppendTestDir()

    This function creates a new directory under the specified test
    path, using a directory name generated from the supplied prefix
    and either a random number or the specified suffix number.

    If a random number is used, retry attempts will be made using a
    different random number as necessary.

    The pTI->szPath value will point to the new directory so that
    all subsequent tests will use this directory.  Typically this
    call is paired with a call to DestroyAndRemoveTestDir().

    Parameters:
        pTI       - A pointer to the FSIOTESTINFO structure to use.
        pszPrefix - A pointer to the null-terminated directory
                    prefix to use.
        nSuffix   - The suffix number to use, or UINT_MAX to cause
                    a randomly generated suffix number to be used.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS CreateAndAppendTestDir(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nSuffix)
{
    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszPrefix, 0);

    return DclTestDirCreateAndAppend(&pTI->sti, &pTI->FSPrim, pTI->szPath, sizeof(pTI->szPath), pszPrefix, nSuffix);
}


/*-------------------------------------------------------------------
    Local: DestroyAndRemoveTestDir()

    This function un-does the work that was done by the function
    CreateAndAppendTestDir().  The directory must be empty for
    this function to succeed.

    Note that even if the directory cannot be removed, it will
    still be deleted from the pTI->szPath string.

    Parameters:
        pTI       - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS DestroyAndRemoveTestDir(
    FSIOTESTINFO   *pTI)
{
    DclAssertWritePtr(pTI, sizeof(*pTI));

    return DclTestDirDestroyAndRemove(&pTI->sti, &pTI->FSPrim, pTI->szPath);
}


/*-------------------------------------------------------------------
    AppSleep()

    Description
        Inject periodic sleep operations, as specified with the /S:n
        command-line option, to allow background compaction to take
        effect.

        The actual time spent sleeping is removed from the total
        test time.

    Parameters
        pTI         - A pointer to the FSIOTESTINFO structure to use
        uMulitplier - A multiplier by which the user specified sleep
                      MS value is multiplied.

    Return Value
        None.
-------------------------------------------------------------------*/
static void AppSleep(
    FSIOTESTINFO   *pTI,
    D_UINT16        uMultiplier)
{
    DCLTIMESTAMP    ts;

    DclAssert(pTI);

    /*  Do nothing if the option is disabled
    */
    if(!pTI->ulSleepMS)
        return;

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("Sleeping for %lU seconds...\n", (pTI->ulSleepMS * uMultiplier) / 1000);

    ts = DclTimeStamp();

    DclOsSleep(pTI->ulSleepMS * uMultiplier);

    pTI->ulIterationSleepMS += DclTimePassed(ts);

    return;
}


/*-------------------------------------------------------------------
    GetKBPerSecond() - Calculate KB/sec, scaling as necessary

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static D_UINT32 GetKBPerSecond(
    D_UINT64        ullKB,
    D_UINT32        ulMS)
{
    D_UINT32        ulScale = 100000UL;

    /*  Avoid divide-by-zero...
    */
    if(!ulMS)
    {
        /*  If nothing was written in no time, just return zero
        */
        if(!ullKB)
            return 0;

        /*  Something was written, so assume it took at least a millisecond...
        */
        ulMS++;
    }

    while((ullKB > D_UINT32_MAX / ulScale) || (ulMS > D_UINT32_MAX / ulScale))
    {
        ulScale /= 2;
        DclAssert(ulScale);
    }

    ullKB *= ulScale;
    ulMS *= ulScale;

    if(ulMS < 1000)
        return 0;
    else
        return (D_UINT32)(ullKB / (ulMS / 1000));
}


/*-------------------------------------------------------------------
    CreateNumberedFile()

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static DCLFSFILEHANDLE CreateNumberedFile(
    FSIOTESTINFO   *pTI,
    char           *pBuffer,
    const char     *pszPrefix,
    unsigned        nFileNum)
{
    DCLFSFILEHANDLE hFile;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pBuffer, 0);
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nFileNum <= FILESET_MAX);

    DclSNPrintf(pBuffer, -1, "%s%s%05u.DAT", pTI->szPath, pszPrefix, nFileNum);

    if(PRIMCREATE(pBuffer, &hFile) != DCLSTAT_SUCCESS)
        return NULL;

    return hFile;
}


/*-------------------------------------------------------------------
    DeleteNumberedFile()

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static DCLSTATUS DeleteNumberedFile(
    FSIOTESTINFO   *pTI,
    const char     *pszPrefix,
    unsigned        nFileNum)
{
    char            szFileName[MAX_FILESPEC_LEN];

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nFileNum <= FILESET_MAX);

    DclSNPrintf(szFileName, -1, "%s%s%05u.DAT", pTI->szPath, pszPrefix, nFileNum);

    if(pTI->fNoDelete)
        return DCLSTAT_SUCCESS;
    else
        return PRIMDELETE(szFileName);
}


/*-------------------------------------------------------------------
    OpenNumberedFile()

    Parameters:

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static DCLSTATUS OpenNumberedFile(
    FSIOTESTINFO       *pTI,
    DCLFSFILEHANDLE    *phFile,
    char               *pBuffer,
    const char         *pszPrefix,
    unsigned            nFileNum)
{
    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(phFile, sizeof(*phFile));
    DclAssertWritePtr(pBuffer, 0);
    DclAssertReadPtr(pszPrefix, 0);
    DclAssert(nFileNum <= FILESET_MAX);

    DclSNPrintf(pBuffer, -1, "%s%s%05u.DAT", pTI->szPath, pszPrefix, nFileNum);

    return PRIMOPEN(pBuffer, "r+b", phFile);
}


/*-------------------------------------------------------------------
    ScanTestPerfLogOutput()

    Parameters:

    Returns:
        Nothing
-------------------------------------------------------------------*/
static void ScanTestPerfLogOutput(
    FSIOTESTINFO       *pTI,
    const char         *pszTitle,
    D_UINT32            ulTotalMS,
    D_UINT32            ulMaxMS,
    D_UINT32           *pulMS,
    unsigned            nSamples)
{
    DCLPERFLOGHANDLE    hPerfLog;
    char                szSample[10];
    unsigned            nScale;
    unsigned            nn;

    DclAssert(pTI);
    DclAssert(pszTitle);
    DclAssert(pulMS);
    DclAssert(nSamples);

    /*  Scale down the timing numbers if necessary so the data fits
        within a 256 byte output line.
    */
    if(ulMaxMS > 9999999)
        nScale = 1000;
    else if(ulMaxMS > 999999)
        nScale = 100;
    else if(ulMaxMS > 99999)
        nScale = 10;
    else
        nScale = 1;

    hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", pszTitle, NULL, pTI->sti.szPerfLogSuffix);

    DCLPERFLOG_NUM(hPerfLog, "TotMS", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "MaxMS", ulMaxMS);
    DCLPERFLOG_NUM(hPerfLog, "Scale", nScale);
    DCLPERFLOG_NUM(hPerfLog, "FS",    FreeDiskBlocks(pTI));

    for(nn=1; nn<=nSamples; nn++)
    {
        DclSNPrintf(szSample, DCLDIMENSIONOF(szSample), "Sam%u", nn);
        DCLPERFLOG_NUM(hPerfLog, szSample, pulMS[nn-1] / nScale);
    }

    DCLPERFLOG_WRITE(hPerfLog);
    DCLPERFLOG_CLOSE(hPerfLog);

    return;
}


/*-------------------------------------------------------------------
    ScanTestMeasureCreate()

    This function is a supports the ScanTest.  It will create
    the specified number of files polling the time it takes
    at the specified interval.

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Returns:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS ScanTestMeasureCreate(
    FSIOTESTINFO       *pTI,
    D_UINT32            ulFileCount,
    D_UINT32            ulFileSize,
    D_UINT32            ulSampleSize)
{
    D_UINT32            ulMS;
    D_UINT32            ulSampleLongMS = 0;
    D_UINT32            ulTotalCumulMS = 0;
    D_UINT32            ff;
    DCLTIMESTAMP        ts;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    D_UINT32            aulMS[DEFAULT_SCAN_SAMPLES];
    unsigned            nSample = 0;
    char                szScaleBuff[16];

    DclAssert(pTI);
    DclAssert(ulFileCount);
    DclAssert(ulSampleSize);

    DclMemSet(&aulMS[0], 0, sizeof(aulMS));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("Creating %lU files of %lU bytes each, sampling every %lU files...\n",
            ulFileCount, ulFileSize, ulSampleSize);
    }
    
    DclTestInstrumentationStart(&pTI->sti);

    for(ff = 1; ff <= ulFileCount; ff++)
    {
        D_BOOL  fFlush = FALSE;

        /*  Starting with test v3.0, for the very last file in the
            complete set, do a flush.
        */                    
        if( (pTI->ulTestEmulationVersion >= DEFAULT_VERSION) && (ff == pTI->ulFileCount) )
            fFlush = TRUE;
        
        ts = DclTimeStamp();

        dclStat = FileCreate(pTI, "scn", ff, 1, ulFileSize, NULL, fFlush);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;

        ulMS = DclTimePassed(ts);
        ulTotalCumulMS += ulMS;
        aulMS[nSample] += ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("    Created file number %u in %lU ms\n", ff, ulMS);

        if((ff % ulSampleSize) == 0)
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("  Sample %2u: Created %lU files in %5lU ms\n", nSample+1, ulSampleSize, aulMS[nSample]);

            if(ulSampleLongMS < aulMS[nSample])
                ulSampleLongMS = aulMS[nSample];

            if(ff < ulFileCount)
                nSample++;

            /*  Let any background operations do their thing...
            */
            AppSleep(pTI, 2);
        }
    }

    ScanTestPerfLogOutput(pTI, "ScanCreate", ulTotalCumulMS, ulSampleLongMS, aulMS, nSample+1);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Average sample create %lU ms -- Long sample create %lU ms\n",
            ulTotalCumulMS / nSample, ulSampleLongMS);
    }

    DclPrintf("  Created %4lU files --------------------------------->%10s seconds %3s%%\n",
        ulFileCount, 
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalCumulMS, 1000, 1), GetFreeSpacePercent(pTI));
    
  Cleanup:

    DclTestInstrumentationStop(&pTI->sti, "ScanCrt");

    return dclStat;
}


/*-------------------------------------------------------------------
    ScanTestMeasureOpen()

    This function is a supports the ScanTest.  It will open
    the specified number of files polling the time it takes
    at the specified interval.

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Returns:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS ScanTestMeasureOpen(
    FSIOTESTINFO       *pTI,
    D_UINT32            ulFileCount,
    D_UINT32            ulSampleSize)
{
    D_UINT32            ulMS;
    D_UINT32            ulSampleLongMS = 0;
    D_UINT32            ulTotalCumulMS = 0;
    D_UINT32            ff;
    DCLTIMESTAMP        ts;
    DCLFSFILEHANDLE     hFile;
    char                szFileName[MAX_FILESPEC_LEN];
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    D_UINT32            aulMS[DEFAULT_SCAN_SAMPLES];
    unsigned            nSample = 0;
    char                szScaleBuff[16];

    DclAssert(pTI);
    DclAssert(ulFileCount);
    DclAssert(ulSampleSize);

    DclMemSet(&aulMS[0], 0, sizeof(aulMS));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Opening %lU files...\n", ulFileCount);

    DclTestInstrumentationStart(&pTI->sti);

    for(ff = 1; ff <= ulFileCount; ff++)
    {
        ts = DclTimeStamp();

        dclStat = OpenNumberedFile(pTI, &hFile, szFileName, "scn", ff);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("  FSIO: Unable to open file number %lU, Status=%lX\n", ff, dclStat);
            goto Cleanup;
        }

        dclStat = PRIMCLOSE(hFile);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("  FSIO: Unable to close file, Status=%lX\n", dclStat);
            goto Cleanup;
        }

        ulMS = DclTimePassed(ts);
        ulTotalCumulMS += ulMS;
        aulMS[nSample] += ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("    Opened file '%s' in %lU ms\n", szFileName, ulMS);

        if((ff % ulSampleSize) == 0)
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("  Sample %2u: Opened %lU files in %5lU ms\n", nSample+1, ulSampleSize, aulMS[nSample]);

            if(ulSampleLongMS < aulMS[nSample])
                ulSampleLongMS = aulMS[nSample];

            if(ff < ulFileCount)
                nSample++;
        }
    }

    ScanTestPerfLogOutput(pTI, "ScanOpen", ulTotalCumulMS, ulSampleLongMS, aulMS, nSample+1);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Average sample open %lU ms -- Long sample open %lU ms\n",
            ulTotalCumulMS / nSample, ulSampleLongMS);
    }

    DclPrintf("  Opened --------------------------------------------->%10s seconds %3s%%\n",
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalCumulMS, 1000, 1), GetFreeSpacePercent(pTI));
     
  Cleanup:

    DclTestInstrumentationStop(&pTI->sti, "ScanOpen");

    return dclStat;
}


/*-------------------------------------------------------------------
    ScanTestMeasureDelete()

    This function is a supports the ScanTest.  It will delete
    the specified number of files polling the time it takes
    at the specified interval.

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Returns:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS ScanTestMeasureDelete(
    FSIOTESTINFO       *pTI,
    D_UINT32            ulFileCount,
    D_UINT32            ulSampleSize)
{
    D_UINT32            ulMS;
    D_UINT32            ulSampleLongMS = 0;
    D_UINT32            ulTotalCumulMS = 0;
    D_UINT32            ff;
    DCLTIMESTAMP        ts;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    D_UINT32            aulMS[DEFAULT_SCAN_SAMPLES];
    unsigned            nSample = 0;
    char                szScaleBuff[16];

    DclAssert(pTI);
    DclAssert(ulFileCount);
    DclAssert(ulSampleSize);

    DclMemSet(&aulMS[0], 0, sizeof(aulMS));

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Deleting %lU files...\n", ulFileCount, 0);

    DclTestInstrumentationStart(&pTI->sti);

    for(ff = 1; ff <= ulFileCount; ff++)
    {
        ts = DclTimeStamp();

        dclStat = DeleteNumberedFile(pTI, "scn", ff);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;

        ulMS = DclTimePassed(ts);
        ulTotalCumulMS += ulMS;
        aulMS[nSample] += ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("    Deleted file number %lU in %lU ms\n", ff, ulMS);

        if((ff % ulSampleSize) == 0)
        {
            if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("  Sample %2u: Deleted %lU files in %5lU ms\n", nSample+1, ulSampleSize, aulMS[nSample]);

            if(ulSampleLongMS < aulMS[nSample])
                ulSampleLongMS = aulMS[nSample];

            if(ff < ulFileCount)
                nSample++;

            /*  Let any background operations do their thing...
            */
            AppSleep(pTI, 2);
        }
    }

    ScanTestPerfLogOutput(pTI, "ScanDelete", ulTotalCumulMS, ulSampleLongMS, aulMS, nSample+1);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Average sample delete %lU ms -- Long sample delete %lU ms\n",
            ulTotalCumulMS / nSample, ulSampleLongMS);
    }
        
    DclPrintf("  Deleted -------------------------------------------->%10s seconds %3s%%\n",
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalCumulMS, 1000, 1), GetFreeSpacePercent(pTI));
 
  Cleanup:

    DclTestInstrumentationStop(&pTI->sti, "ScanDel");

    return dclStat;
}


/*-------------------------------------------------------------------
    ScanTest()

    This test exercises the time it takes to create, open, and
    delete a number of files (specified by /SFILES:n).  The
    files are created, opened, and deleted in forward order.  The
    size used for each file is zero.

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Returns:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS ScanTest(
    FSIOTESTINFO       *pTI)
{
    D_UINT32            ulFileCount;
    D_UINT32            ulFileSize;
    D_UINT32            ulSampleSize;
    DCLSTATUS           dclStat;

    /*  This test is primarily about scanning, but at the same time
        we want to be realistic.  Use a file size which is small,
        but not non-existant.
    */
    ulFileSize = pTI->ulFSBlockSize;

    /*  Determine the number of files within each sample
    */
    ulFileCount = DCLMIN(pTI->ulScanCount + (DEFAULT_SCAN_SAMPLES - 1), D_UINT16_MAX);
    ulSampleSize = ulFileCount / DEFAULT_SCAN_SAMPLES;
    ulFileCount = ulSampleSize * DEFAULT_SCAN_SAMPLES;

    {
        DCLPERFLOGHANDLE    hPerfLog;

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "ScanTest", NULL, pTI->sti.szPerfLogSuffix);
        DCLPERFLOG_NUM(     hPerfLog, "FileCount",  ulFileCount);
        DCLPERFLOG_NUM(     hPerfLog, "FileSize",   ulFileSize);
        DCLPERFLOG_NUM(     hPerfLog, "SampleSize", ulSampleSize);
        DCLPERFLOG_NUM(     hPerfLog, "FSBefore",   FreeDiskBlocks(pTI));
        DCLPERFLOG_WRITE(   hPerfLog);
        DCLPERFLOG_CLOSE(   hPerfLog);
    }

    /*  Create a directory to test in
    */
    dclStat = CreateAndAppendTestDir(pTI, "SCN", UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    /*  Measure create times
    */
    dclStat = ScanTestMeasureCreate(pTI, ulFileCount, ulFileSize, ulSampleSize);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    /*  Measure open times
    */
    dclStat = ScanTestMeasureOpen(pTI, ulFileCount, ulSampleSize);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    /*  Measure delete times
    */
    dclStat = ScanTestMeasureDelete(pTI, ulFileCount, ulSampleSize);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    /*  Remove the main SCN directory
    */
    DestroyAndRemoveTestDir(pTI);

  Cleanup:

    return dclStat;
}


/*-------------------------------------------------------------------
    DirTest()

    Description
        This test exercises directories by creating a number of
        directories (specified by /DIRS:n), and in each directory
        creating a number of files (specified by /FILES:n).

        The files are created horizontally across the directory
        tree so that directory information will (likely) be striped
        across the disk.

        The size used for each file is fixed, and is automatically
        calculated based on the amount of disk space the test is
        designated to use (/MAX:n).

        These files are then deleted in a reverse staggered fashion.

    Parameters
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS DirTest(
    FSIOTESTINFO       *pTI)
{
    #define DIRNAMELEN   (8)    /* ToDo: Fix the DIR and NAME lengths to be variable */
    #define FILENAMELEN (32)
    D_UINT32            ulFileSize;
    unsigned            dd;
    char                szFileName[MAX_FILESPEC_LEN];
    DCLSTATUS           dclStat;
    unsigned long       ff, iter;
    DCLPERFLOGHANDLE    hPerfLog = NULL;
    DCLTIMESTAMP        t;
    D_UINT32            ulMS = 0;
    D_UINT32            ulTotalMS;
    D_UINT32            ulLongestMS;
    char                szScaleBuff[16];

    ulFileSize = pTI->ulMaxDiskSpace;

    if(ulFileSize <= pTI->ulDirCount)
        goto TooSmall;

    ulFileSize /= pTI->ulDirCount;

    if(ulFileSize <= pTI->ulFileCount)
        goto TooSmall;

    ulFileSize /= pTI->ulFileCount;

    /*  Allow for 1/8th of the space for metadata
    */
    ulFileSize -= ulFileSize >> 3;

    ulFileSize = BoundaryRoundUp(ulFileSize, 32, 16384);

    if(!ulFileSize)
       goto TooSmall;

    /*---------------------------------------------------------
        Write data to the performance log, if enabled.  Note
        that any changes to the test name or category must be
        accompanied by changes to perffsio.bat.  Any changes to
        the actual data fields recorded here requires changes
        to the various spreadsheets which track this data.
    ---------------------------------------------------------*/
    hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "DirTest", NULL, pTI->sti.szPerfLogSuffix);
    DCLPERFLOG_NUM(hPerfLog, "DirNameLen",      DIRNAMELEN);
    DCLPERFLOG_NUM(hPerfLog, "FileNameLen",     FILENAMELEN);
    DCLPERFLOG_NUM(hPerfLog, "DirCount",        pTI->ulDirCount);
    DCLPERFLOG_NUM(hPerfLog, "FileCount",       pTI->ulFileCount);
    DCLPERFLOG_NUM(hPerfLog, "FileSize",        ulFileSize);
    DCLPERFLOG_NUM(hPerfLog, "FSBefore",        FreeDiskBlocks(pTI));

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Creating %lU directories...\n", pTI->ulDirCount);

    dclStat = CreateAndAppendTestDir(pTI, "DIRSTRIPE", UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        goto QuickCleanup;
 
    t = DclTimeStamp();

    for(dd = 0; dd < pTI->ulDirCount; dd++)
    {
        DclSNPrintf(szFileName, MAX_FILESPEC_LEN, "%sDIR%05u", pTI->szPath, dd+1);

        dclStat = PRIMDIRCREATE(szFileName);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;
    }

    ulMS = DclTimePassed(t);

    DclPrintf("  Directories created (%lU directories) ---------------->%10s seconds %3s%%\n", 
        pTI->ulDirCount, DclRatio(szScaleBuff, sizeof szScaleBuff, ulMS, 1000, 1), GetFreeSpacePercent(pTI));

    DclTestStatsDump(&pTI->sti, "DirDCrt");

    DCLPERFLOG_NUM(hPerfLog, "DirCreate", ulMS);

    DclTestStatsReset(&pTI->sti);

    dclStat = DTCreateFiles(pTI, 0, pTI->ulFileCount, ulFileSize, hPerfLog);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    DclTestStatsDump(&pTI->sti, "DirFCrt");

    /*  Let any background operations do their thing...
    */
    AppSleep(pTI, 5);

    DCLPERFLOG_NUM(hPerfLog, "FSDuring", FreeDiskBlocks(pTI));

    DclTestStatsReset(&pTI->sti);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Open, seek to end, read (1 byte), and close each file\n");

    dclStat = DTOpenSeekClose(pTI, ulFileSize, hPerfLog);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    DclTestStatsDump(&pTI->sti, "DirRead");

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Delete each of %lU FileSets in a staggered reverse pattern\n", pTI->ulFileCount);

    DclTestStatsReset(&pTI->sti);

    ff = pTI->ulFileCount-1;
    iter = ff;

    ulTotalMS = 0;
    ulLongestMS = 0;

    while(TRUE)
    {
        D_UINT32    ulCurrentMS = ulTotalMS;

        dclStat = DTDeleteFileSet(pTI, ff, &ulTotalMS, &ulLongestMS);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("  FileSet %3lU: Deleted in %5lU ms\n", ff+1, ulTotalMS - ulCurrentMS);

        if(ff >= 10)
        {
            ff -= 10;
        }
        else
        {
            iter--;
            if(iter < pTI->ulFileCount-10)
                break;

            ff = iter;

            /*  Let any background operations do their thing...
            */
            AppSleep(pTI, 3);
        }
    }

    DclTestStatsDump(&pTI->sti, "DirFDel");

    DCLPERFLOG_NUM(hPerfLog, "FileDelTotal", ulTotalMS);
    DCLPERFLOG_NUM(hPerfLog, "FileDelLongest", ulLongestMS);

    DclPrintf("  FileSets deleted ----------------------------------->%10s seconds %3s%%\n",
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1), GetFreeSpacePercent(pTI));
 
    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        DclPrintf("Deleting %lU directories...\n", pTI->ulDirCount);

    DclTestStatsReset(&pTI->sti);

    t = DclTimeStamp();

    for(dd = 0; dd < pTI->ulDirCount; dd++)
    {
        DclSNPrintf(szFileName, MAX_FILESPEC_LEN, "%sDIR%05u", pTI->szPath, dd+1);

        dclStat = PRIMDIRREMOVE(szFileName);
        if(dclStat != DCLSTAT_SUCCESS)
            break;
    }

    ulMS = DclTimePassed(t);

    DclPrintf("  Directories deleted -------------------------------->%10s seconds %3s%%\n",
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulMS, 1000, 1), GetFreeSpacePercent(pTI));

  Cleanup:
    DestroyAndRemoveTestDir(pTI);

    DclTestStatsDump(&pTI->sti, "DirDDel");

    DCLPERFLOG_NUM(hPerfLog, "DirDel",  ulMS);
    DCLPERFLOG_NUM(hPerfLog, "FSAfter", FreeDiskBlocks(pTI));

  QuickCleanup:
    if(hPerfLog)
    {
        DCLPERFLOG_WRITE(hPerfLog);
        DCLPERFLOG_CLOSE(hPerfLog);
    }

    return dclStat;

  TooSmall:
    DclPrintf("The combination of /DIRS:%lU  and /FILES:%lU is too large relative\n", pTI->ulDirCount, pTI->ulFileCount);
    DclPrintf("to the /MAX=%lU KB value for the /DIR test to function properly.\n", pTI->ulMaxDiskSpace / 1024UL);

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    DTCreateFiles()

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static DCLSTATUS DTCreateFiles(
    FSIOTESTINFO       *pTI,
    D_UINT32            ulFileStart,
    D_UINT32            ulFileCount,
    D_UINT32            ulFileSize,
    DCLPERFLOGHANDLE    hPerfLog)
{
    unsigned            dd;
    D_UINT32            ff;
    D_UINT32            ulTotalCumulMS = 0;
    D_UINT32            ulLongMS = 0;
    DCLTIMESTAMP        ts;
    unsigned            iter = 0;
    char                szScaleBuff[16];

    DclAssert(pTI);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("For each of %lU directories, creating %lU files of %lU bytes each...\n",
            pTI->ulDirCount, ulFileCount, ulFileSize);
    }
    
    for(ff = ulFileStart; ff < ulFileStart + ulFileCount; ff++)
    {
        D_UINT32    ulFileCumulMS = 0;

        for(dd = 0; dd < pTI->ulDirCount; dd++)
        {
            D_UINT32        ulRemaining = ulFileSize;
            D_UINT32        ulMS;
            DCLFSFILEHANDLE hFile;
            char            szFileName[MAX_FILESPEC_LEN];
            DCLSTATUS       dclStat;

            DclSNPrintf(szFileName, MAX_FILESPEC_LEN, "%sDIR%05u%sAAAAAAAAAAAAAAAAAAAAAAAAAAA%05lU.dat",
                pTI->szPath, dd+1, DCL_PATHSEPSTR, ff+1);

            szFileName[MAX_FILESPEC_LEN-1] = 0;

            ts = DclTimeStamp();

            dclStat = PRIMCREATE(szFileName, &hFile);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            while(ulRemaining)
            {
                size_t      nWritten;
                unsigned    nThis = DCLMIN(ulRemaining, pTI->ulBufferSize);

                dclStat = PrimWrite(pTI, pTI->pBuffer, nThis, 1, hFile, &nWritten);
                if(dclStat != DCLSTAT_SUCCESS || nWritten != 1)
                {
                    if(dclStat == DCLSTAT_SUCCESS)
                        dclStat = DCLSTAT_FS_WRITEFAILED;

                    PRIMCLOSE(hFile);
                }

                ulRemaining -= nThis;
            }

            /*  Starting with test v3.0, for the very last file in the
                complete set, do a flush.
            */                    
            if( (pTI->ulTestEmulationVersion >= DEFAULT_VERSION) && 
                (dd == pTI->ulDirCount-1) && 
                (ff == ulFileStart + ulFileCount - 1) )
            {
                if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                    DclPrintf("Flushing last file\n");
                
                PRIMFLUSH(hFile);
            }

            dclStat = PRIMCLOSE(hFile);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("  FSIO: Unable to close file, Status=%lX\n", dclStat);
                return dclStat;
            }

            ulMS = DclTimePassed(ts);
            ulFileCumulMS += ulMS;
            ulTotalCumulMS += ulMS;
            if(ulLongMS < ulMS)
                ulLongMS = ulMS;

            if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
                DclPrintf("    Created file '%s' in %lU ms\n", szFileName, ulMS);
        }

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
        {
            DclPrintf("  FileSet %3u: Created %lU files (1 per directory) in %5lU ms\n",
                iter+1, pTI->ulDirCount, ulFileCumulMS);
        }

        iter++;
    }

    DCLPERFLOG_NUM(hPerfLog, "TotalTime",   ulTotalCumulMS);
    DCLPERFLOG_NUM(hPerfLog, "LongestTime", ulLongMS);

    DclPrintf("  FileSets created (%4lU files) ---------------------->%10s seconds %3s%%\n",
        pTI->ulDirCount * ulFileCount, 
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalCumulMS, 1000, 1), GetFreeSpacePercent(pTI));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    DTOpenSeekClose()

    Description

    Parameters

    Return Value
-------------------------------------------------------------------*/
static DCLSTATUS DTOpenSeekClose(
    FSIOTESTINFO       *pTI,
    D_UINT32            ulFileSize,
    DCLPERFLOGHANDLE    hPerfLog)
{
    unsigned            dd;
    D_UINT32            ulTotalMS = 0;
    char                szScaleBuff[16];

    DclAssert(pTI);

    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("For each of %lU directories, open, seek to the end, and close %lU files...\n",
            pTI->ulDirCount, pTI->ulFileCount);
    }
    
    for(dd = 0; dd < pTI->ulDirCount; dd++)
    {
        D_UINT32        ulMS;
        D_UINT32        ff;
        DCLTIMESTAMP    ts;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("  Processing directory #%u...\n", dd+1);

        ts = DclTimeStamp();

        for(ff = 0; ff < pTI->ulFileCount; ff++)
        {
            DCLFSFILEHANDLE   hFile;
            char            szFileName[MAX_FILESPEC_LEN];
            DCLSTATUS       dclStat;
            D_BUFFER        buff[5];

            DclSNPrintf(szFileName, MAX_FILESPEC_LEN, "%sDIR%05u%sAAAAAAAAAAAAAAAAAAAAAAAAAAA%05lU.dat",
                pTI->szPath, dd+1, DCL_PATHSEPSTR, ff+1);

            szFileName[MAX_FILESPEC_LEN-1] = 0;

            dclStat = PRIMOPEN(szFileName, "r+b", &hFile);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            /*  Seek to the last byte in the file
            */
            dclStat = PRIMSEEK(hFile, ulFileSize-1, DCLFSFILESEEK_SET);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            /*  Read the last byte in the file
            */
            dclStat = PRIMREAD(hFile, &buff[0], 1, NULL);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;

            dclStat = PRIMCLOSE(hFile);
            if(dclStat != DCLSTAT_SUCCESS)
                return dclStat;
        }

        ulMS = DclTimePassed(ts);

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
            DclPrintf("  Finished -- directory required %lU ms\n", ulMS);

        ulTotalMS += ulMS;
    }

    DCLPERFLOG_NUM(hPerfLog, "OpenSeekCloseMS", ulTotalMS);

    DclPrintf("  FileSets processed (open/seek/read/close) ---------->%10s seconds %3s%%\n",
        DclRatio(szScaleBuff, sizeof szScaleBuff, ulTotalMS, 1000, 1), GetFreeSpacePercent(pTI));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    DTDeleteFileSet()

    Description

    Parameters

    Return Value
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS DTDeleteFileSet(
    FSIOTESTINFO   *pTI,
    D_UINT32        ulFileNum,
    D_UINT32       *pulTotalTime,
    D_UINT32       *pulLongestTime)
{
    char            szFileName[MAX_FILESPEC_LEN];
    unsigned        dd;

    DclAssert(pTI);
    DclAssert(pulTotalTime);
    DclAssert(pulLongestTime);

    for(dd = 0; dd < pTI->ulDirCount; dd++)
    {
        DCLTIMESTAMP    t;
        D_UINT32        ulMS;

        DclSNPrintf(szFileName, MAX_FILESPEC_LEN, "%sDIR%05u%sAAAAAAAAAAAAAAAAAAAAAAAAAAA%05lU.dat",
            pTI->szPath, dd+1, DCL_PATHSEPSTR, ulFileNum+1);

        szFileName[MAX_FILESPEC_LEN-1] = 0;

        t = DclTimeStamp();

        if(!pTI->fNoDelete && PRIMDELETE(szFileName) != DCLSTAT_SUCCESS)
            return DCLSTAT_CURRENTLINE;

        ulMS = DclTimePassed(t);

        *pulTotalTime += ulMS;

        if(*pulLongestTime < ulMS)
            *pulLongestTime = ulMS;

        if(pTI->sti.nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("    Deleted File #%u in %lU ms\n", dd+1, ulMS);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    TreeTest()

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Returns:
        Returns a DCLSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static DCLSTATUS TreeTest(
    FSIOTESTINFO   *pTI)
{
    #define         TREE_LEVELS            (6) /* change to a run-time setting */
    #define         TREE_DIRS            (873) /* calculate this based on TREE_LEVELS */
    #define         TREE_FILES_PER_DIR    (64) /* default starting value */
    #define         TREE_FILE_SIZE_DIVISOR (4)
    DCLSTATUS       dclStat;
    D_UINT32        ulTotalFiles = 0;
    DCLTIMESTAMP    ts = DCLINITTS; /* Not necessary, but some compilers seem to think so... */
    char            szScaleBuff[16];
    unsigned        nTotalDirs = TREE_DIRS;
    unsigned        nFiles = TREE_FILES_PER_DIR;
    D_UINT64        ullUS;
    D_UINT32        ulFreeSpace;
    D_UINT32        ulAvgSize;

    {
        DCLPERFLOGHANDLE    hPerfLog;

        hPerfLog = DCLPERFLOG_OPEN(pTI->sti.fPerfLog, pTI->sti.hLog, "FSIO", "TreeTest", NULL, pTI->sti.szPerfLogSuffix);
/*        DCLPERFLOG_NUM(     hPerfLog, "FileCount",  ulFileCount);
        DCLPERFLOG_NUM(     hPerfLog, "FileSize",   ulFileSize);
        DCLPERFLOG_NUM(     hPerfLog, "SampleSize", ulSampleSize); */
        DCLPERFLOG_NUM(     hPerfLog, "FSBefore",   FreeDiskBlocks(pTI));
        DCLPERFLOG_WRITE(   hPerfLog);
        DCLPERFLOG_CLOSE(   hPerfLog);
    }

    /*  Scale the file count so as not to use more than 25% of the
        available disk space (approximated).
    */
    ulFreeSpace = FreeDiskBlocks(pTI) * pTI->ulFSBlockSize;
    ulAvgSize = DCLMAX(pTI->ulFSBlockSize, pTI->ulBufferSize / TREE_FILE_SIZE_DIVISOR / 2);
    
    while(((nFiles * ulAvgSize) * nTotalDirs) > ulFreeSpace >> 2)
        nFiles >>= 1;

    nFiles = DCLMAX(nFiles, 2);

    /*  Create a directory to test in
    */
    dclStat = CreateAndAppendTestDir(pTI, "TREE", UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    dclStat = TreeCreateRecurse(pTI, 2, TREE_LEVELS, nFiles, &ulTotalFiles, &ullUS);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Create one last file with an explicit flush 
        */        
        ts = DclTimeStamp();
        dclStat = FileCreate(pTI, "TreeC", 1, 1, 1, NULL, TRUE);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclError();
            return dclStat;
        }
        else
        {
            ullUS += DclTimePassedUS(ts);
            ulTotalFiles++;
        }
    }
    else
    {
        if(dclStat == DCLSTAT_FS_FULL)
            dclStat = DCLSTAT_SUCCESS;
        else
            DclPrintf("Tree test failed with status %lX\n", dclStat);
    }

    DclPrintf("  Created %5lU files in a %u level tree -------------->%10s seconds %3s%%\n",
        ulTotalFiles, TREE_LEVELS,
        DclRatio64(szScaleBuff, sizeof(szScaleBuff), ullUS, 1000000, 1), 
        GetFreeSpacePercent(pTI));

    if((dclStat == DCLSTAT_SUCCESS) && !pTI->fNoDelete)    
    {
        unsigned    nVerbosity = DCL_VERBOSE_QUIET;

        if(pTI->sti.nVerbosity > DCL_VERBOSE_QUIET)
            nVerbosity = pTI->sti.nVerbosity - 1;
            
        ts = DclTimeStamp();    
        
        dclStat = PRIMDIRTREEDEL(pTI->szPath, nVerbosity);
        if(dclStat != DCLSTAT_SUCCESS)
            goto Cleanup;
    }
    
    DclTestDirRemoveFromPath(pTI->szPath);    

    if((dclStat == DCLSTAT_SUCCESS) && !pTI->fNoDelete)    
    {
        /*  Note that the call to DclTestDirRemoveFromPath() above, MUST be 
            done before attempting to get the free space percentage.
        */            
        DclPrintf("  Deleted the directory tree ------------------------->%10s seconds %3s%%\n",
            DclRatio64(szScaleBuff, sizeof szScaleBuff, DclTimePassedUS(ts), 1000000, 1), 
            GetFreeSpacePercent(pTI));
    }
    
  Cleanup:

    return dclStat;
}


/*-------------------------------------------------------------------
    Description

    Parameters
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value
-------------------------------------------------------------------*/
static DCLSTATUS TreeCreateRecurse(
    FSIOTESTINFO       *pTI,
    unsigned            nLevel,
    unsigned            nMaxLevel,
    unsigned            nFiles,
    D_UINT32           *pulCreated,
    D_UINT64           *pullTimeUS)
{
    unsigned            nn;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;
    D_UINT32            ulCreateMS;
    D_UINT32            ulTotalFiles = 0;
    D_UINT64            ullTotalUS = 0;
    unsigned            nCreated;

    DclAssertWritePtr(pTI, sizeof(*pTI));
    DclAssertWritePtr(pulCreated, sizeof(*pulCreated));
    DclAssertWritePtr(pullTimeUS, sizeof(*pullTimeUS));
    DclAssert(nLevel <= nMaxLevel + 1);
    DclAssert(nFiles);
    DclAssert(!(nFiles & 1));
    
    dclStat = CreateFileSet(pTI, "TreeA", &ulCreateMS, NULL, &nCreated, nFiles, -1, pTI->ulBufferSize / TREE_FILE_SIZE_DIVISOR, FALSE, 0, UINT_MAX);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    ulTotalFiles = nCreated;
    ullTotalUS = ulCreateMS * 1000;
    
    if(pTI->sti.nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("    Required %4lU ms to create %u files in directory \"%s\"\n", 
            ulCreateMS, nCreated, pTI->szPath);
    }
 
    if(nLevel <= nMaxLevel)
    {
        for(nn = 1; nn <= nLevel; nn++)
        {
            D_UINT32    ulCreated;
            D_UINT64    ullUS;
            
            dclStat = CreateAndAppendTestDir(pTI, "LEV", UINT_MAX);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            dclStat = TreeCreateRecurse(pTI, nLevel+1, nMaxLevel, nFiles, &ulCreated, &ullUS);
            if(dclStat != DCLSTAT_SUCCESS)
                break;

            ullTotalUS += ullUS;
            ulTotalFiles += ulCreated;

            dclStat = DclTestDirRemoveFromPath(pTI->szPath);    
            if(dclStat != DCLSTAT_SUCCESS)
                break;
        }
    }

    *pullTimeUS = ullTotalUS;
    *pulCreated = ulTotalFiles;
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: FreeDiskBlocks()

    This function returns the number of free blocks on the disk.

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns the disk blocks free, or D_UINT32_MAX if error.
-------------------------------------------------------------------*/
static D_UINT32 FreeDiskBlocks(
    FSIOTESTINFO   *pTI)
{
    DCLFSSTATFS     statfs;

    DclAssert(pTI);

    if(!InternalStatFS(pTI, &statfs))
    {
        DclPrintf("InternalStatFS() failed!\n");
        return D_UINT32_MAX;
    }

    DclAssert(statfs.ulBlockSize == pTI->statfs.ulBlockSize);
    DclAssert(statfs.ulBlockSize == pTI->ulFSBlockSize);

    return statfs.ulFreeBlocks;
}


/*-------------------------------------------------------------------
    Local: InternalStatFS()

    This function implements "statfs" functionality but scales
    the results based on the FS block size specified on the
    command-line (if any).

    Parameters:
        pTI     - A pointer to the FSIOTESTINFO structure to use.
        pStatFS - A pointer to the DCLFSSTATFS structure to fill.

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL InternalStatFS(
    FSIOTESTINFO   *pTI,
    DCLFSSTATFS    *pStatFS)
{
    DCLFSSTATFS     statfs;
    DCLSTATUS       dclStat;

    DclAssert(pTI);
    DclAssert(pStatFS);

    dclStat = PRIMSTATFS(pTI->szPath[0] ? pTI->szPath : ".", &statfs);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error: DclFsStatFs(%s) failed with status %lX\n", pTI->szPath[0] ? pTI->szPath : ".", dclStat);
        return FALSE;
    }

    /*  Since we may have overridden the block size to use on the command-
        line (because some environments don't let this be accurately queried),
        scale the results reported so they are accurate based on the desired
        block size.
    */
    if(pTI->ulFSBlockSize && statfs.ulBlockSize != pTI->ulFSBlockSize)
    {
        D_UINT64    ullTemp;

        DclUint64AssignUint32(&ullTemp, statfs.ulTotalBlocks);
        DclUint64MulUint32(&ullTemp, statfs.ulBlockSize);
        DclUint64DivUint32(&ullTemp, pTI->ulFSBlockSize);
        DclAssert(DclUint64LessUint32(&ullTemp, D_UINT32_MAX));
        statfs.ulTotalBlocks = DclUint32CastUint64(&ullTemp);

        DclUint64AssignUint32(&ullTemp, statfs.ulFreeBlocks);
        DclUint64MulUint32(&ullTemp, statfs.ulBlockSize);
        DclUint64DivUint32(&ullTemp, pTI->ulFSBlockSize);
        DclAssert(DclUint64LessUint32(&ullTemp, D_UINT32_MAX));
        statfs.ulFreeBlocks = DclUint32CastUint64(&ullTemp);

        statfs.ulBlockSize = pTI->ulFSBlockSize;
    }

    *pStatFS = statfs;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: GetFreeSpacePercent()

    Parameters:
        pTI - A pointer to the FSIOTESTINFO structure to use.

    Return Value:
        Returns a pointer to a buffer containing the formatted
        percent value.  This value must be copied and used prior
        to subsequent calls to this function.
-------------------------------------------------------------------*/
static const char * GetFreeSpacePercent(
    FSIOTESTINFO   *pTI)
{
    D_UINT32        ulFreeBlocks;
    
    DclAssert(pTI);

    ulFreeBlocks = FreeDiskBlocks(pTI);
    DclAssert(ulFreeBlocks != D_UINT32_MAX);

    DclRatio64(pTI->szFSPBuff, sizeof(pTI->szFSPBuff), ((D_UINT64)ulFreeBlocks) * 100, pTI->statfs.ulTotalBlocks, 0);

    /*  DclRatio() will truncate results -- if ulFreeBlocks is non-zero, but
        less than 1%, "0" will be returned.  Modify the result so that "0" is
        never returned unless the actual number of free blocks really is zero.
    */
    if(ulFreeBlocks != 0 && (DclStrCmp(pTI->szFSPBuff, "0") == 0))
        DclStrCpy(pTI->szFSPBuff, "<1");

    return pTI->szFSPBuff;
}



