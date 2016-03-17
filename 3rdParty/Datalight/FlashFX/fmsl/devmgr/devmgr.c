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

    This module implements the DeviceManager portion of FlashFX, which
    manages FIM instances.  It serializes access to FIMs, implements
    erase suspend/resume, and break I/O operations into bite-sized chunks
    based on the project's latency settings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devmgr.c $
    Revision 1.98  2011/11/22 01:28:57Z  glenns
    Fix Bug 3548; In FORCEDALIGNEDIO configuirations, 
    GenericRead can fail to copy all data to aligned buffer if the page
    has correctable errors in it.
    Revision 1.97  2011/02/09 23:20:15Z  glenns
    Fix bug 3323- allow device manager to propagate srubbing
    request up through the call stack.
    Revision 1.96  2010/12/14 18:57:47Z  glenns
    Initialize new FFXFIMDEVINFO structure element to pass the
    number of BBM-managed chips up to higher levels. See bug
    3280.
    Revision 1.95  2010/09/29 00:01:05Z  garyp
    Fixed GenericRead() and GenericWrite() to ensure that the request
    packet is restored in the event that an error occurs.  Removed pointless
    static error structure definitions and fixed places where the wrong I/O
    status flags were being returned.  Added debug code.
    Revision 1.94  2010/09/23 07:11:52Z  garyp
    Implemented the previous rev's Bug 2607 fix for writes as well since when
    multiple Disks are on one Device, a write could interrupt a broken up read.
    Fixed to handle the DEV_REQUIRES_BBM and DEV_USING_BBM flags properly.
    Revision 1.93  2010/09/18 02:40:26Z  garyp
    Resolved Bug 2607, where some latency reduction features had been disabled
    to avoid a potential corruption of the erase state information in some 
    geometries.  Modified EraseResume() to only report an error to the Error
    Manager if erase resume functionality is actually supported.
    Revision 1.92  2010/07/21 14:14:29Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.91  2010/07/16 00:48:23Z  garyp
    Fixed so BBM stats are provided only if BBM is actually in use on the Device.
    Revision 1.90  2010/07/08 03:33:19Z  garyp
    Removed some unnecessary error handling code.
    Revision 1.89  2010/07/06 01:21:18Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.88  2010/06/19 03:39:41Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.87  2010/04/28 16:28:01Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.86  2010/03/12 01:44:04Z  glenns
    Fix Bug 3068: prevent derefrence of possibly invalid pointer in 
    cleanup section of FfxDevCreate.
    Revision 1.85  2009/08/27 20:27:48Z  billr
    Fix uninitialized variables.
    Revision 1.84  2009/08/04 17:54:34Z  garyp
    Merged from the v4.0 branch.  Added OTP support.  Added support for 
    "Lock-freeze".  Fixed several problems with regard to the Device create/
    destroy operations and the AccessBegin/End() funtionality.  Modified so
    FfxDevCreate() honors a failure result from AccessBegin(ACCESS_MOUNTS).
    Fixed so that AccessEnd(ACCESS_MOUNTS) is properly called even if 
    the Device creation fails (so long as the "begin" function returned 
    successfully).  Fixed so that the ACCESS_UNMOUNTS is issued for
    Device destruction -- it apparently was never being done.  Added support
    for the "Read/Write-UncorrectedPages" interfaces.  Added support for
    block locking and unlocking.  Added support for power suspend and resume.
    Modified the IO/Erase Prolog/Epilog functions to return an FFXSTATUS value
    rather than a D_BOOL.  Fixed Bug 2819: The DevMgr contains memory/
    resource leaks on shutdown and error cleanup.
    Revision 1.83  2009/04/17 19:50:59Z  keithg
    Corrected read back verify to allow for ECC and raw reads.
    Revision 1.82  2009/04/17 18:29:39Z  keithg
    Fixed bug 2632, typos in READBACKVERIFY code caused build failure.
    Revision 1.81  2009/04/14 16:23:49Z  keithg
    Implemented workaround for bug 2607, disabled latency reduction
    for read operations.
    Revision 1.80  2009/04/02 17:04:55Z  glenns
    Add parens to function name in header comments to satisfy Autodoc.
    Revision 1.79  2009/04/02 16:16:54Z  glenns
    Fixed Bug 2385: Add and amend function header comment blocks as necessary.
    Revision 1.78  2009/04/01 17:25:02Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.77  2009/03/30 15:36:48Z  keithg
    Added comment clarifying initialization of ulRawBlocks.
    Revision 1.76  2009/03/19 04:38:58Z  keithg
    Per Bug 2528, now updates the public ulReservedBlocks from the locally
    maintained reserved blocks since the FIM reserved blocks is no longer used.
    Revision 1.75  2009/03/14 01:40:06Z  glenns
    Fixed Bug 2462: Repaired calculation of the number of reserved blocks when
    the number of KILObytes reserved is less than the block size in BYTES. 
    Order of operation was causing rounding error.
    Revision 1.74  2009/03/13 20:23:03Z  glenns
    Fixed Bug 2354: Remame Dev Mgr's event-reporting functions to meed Datalight
    standards and make them public.
    Revision 1.73  2009/03/10 00:03:14Z  glenns
    Fixed Bug 2395: Add code to FfxDevInfo that assigns values to new elements
    of the FFXFIMDEVINFO structure making device EDC Capability and Requirement,
    BBM Reserved Block Rating and Erase Cycle Rating visible to clients of the
    device manager.
    Revision 1.72  2009/03/04 21:38:57Z  billr
    Resolve bug 2459: Code assumes 'DCLTIMESTAMP' is a integral type.
    Revision 1.71  2009/02/27 21:33:13Z  billr
    Eliminate use of C99 feature (array dimension not constant) in devbbm.c
    Revision 1.70  2009/02/26 21:13:06Z  billr
    Resolve bug 2334: BBM ignores read errors during block replacement.
    Resolve bug 2354: Device Manager (DEVBBM) does not report errors.
    Resolve bug 2429: FXIOSUBFUNC_DEV_WRITE_RAWPAGES may infinite loop on
    write error.  Resolve bug 2457: BBM and NOR flash issues.
    Resolve bug 2430: Possible spurious BBM block replacement.
    Revision 1.69  2009/02/12 16:48:23Z  billr
    Avoid possible overflow in range checks.  Remove superfluous assertions.
    Revision 1.68  2009/02/12 07:36:55Z  keithg
    Fixed bug 2394 - The device manager now maintains a private member
    to describe reserved space and does not use the obsolete FIM/NTM
    reserved space attributes.
    Revision 1.67  2009/02/10 17:36:17Z  keithg
    Relabeled several functions as protected.
    Revision 1.66  2009/02/08 19:53:51Z  garyp
    Minor status code updates.
    Revision 1.65  2009/02/06 07:14:34Z  keithg
    Removed redundant conditional code; removed obsolete comments.
    Revision 1.64  2009/01/29 17:02:35  keithg
    Corrected page accounting in reads from last checkin.
    Revision 1.63  2009/01/29 15:45:22  keithg
    Corrected addressing with reserved space at the start of the device.
    Revision 1.62  2009/01/22 17:35:20  glenns
    Modified to use improved event manager interfaces.
    Revision 1.61  2009/01/18 08:43:26Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.60  2009/01/16 23:24:24Z  glenns
    - Added code to create and destroy error manager and optionally
      event manager instances in a FIM device.
    - Added code to FfxDevIORequest to report readback-verify
      errors to the error manager.
    - Fixed up literal FFXIOSTATUS initialization in one place.
    - Added conditionally-compiled definition of "FfxDevGetEventMgrInstance",
      which allows devmgr clients to get the event manager instance
      associated with the FimDev.
    - Added a series of conditionally-compiled procedures to report
      various sorts of events to the event manager.
    - Modified GenericRead to report errors that occur during read
      operations to the error manager.
    - Modified GenericWrite to report errors that occur during write
      operations to the error manager.
    - Modified EraseBlocks to report errors that occur during erase
      operations to the error manager.
    - Modified EraseSuspend to report errors that occur during erase
      suspend operations to the error manager.
    - Modified EraseResume to report errors that occur during erase
      resume operations to the error manager.
    Revision 1.59  2008/12/31 18:58:09Z  billr
    Conditionally compile code that is only for BBM V5.
    Revision 1.58  2008/12/25 06:14:12Z  keithg
    Added support new IOCTL support for raw erase operations.  Removed asserts
    and debug messages on handled error conditions.
    Revision 1.57  2008/12/22 06:42:00Z  keithg
    Corrected cut-and-paste error - raw writes are no longer remapped.
    Revision 1.56  2008/12/19 02:10:24Z  keithg
    Added support for the BBM statistics API.
    Revision 1.55  2008/12/18 17:43:06Z  keithg
    Replace the WriteNativePages functionality which as incorrectly
    removed in the previous check in.
    Revision 1.54  2008/12/18 04:26:49Z  keithg
    Code is now conditioned properly to compile with and without BBM
    or NAND support appropriately.
    Revision 1.53  2008/12/15 21:20:26Z  keithg
    Conditioned BBM v5 code on FFXCONF_BBMSUPPORT.
    Revision 1.52  2008/12/12 07:23:37Z  keithg
    Added support for reading/writing native pages and setting the block status;
    Now properly handles *not* mapping during raw access;  Added allocation
    of a page buffer for BBM use rather than one for each BBM instance.
    Revision 1.51  2008/12/04 00:27:49Z  keithg
    Fixed BBM v5 mapping to properly handle requests across mapped and unmapped
    blocks and pages.  Clarified variable names with names that reflected the 
    types.
    Revision 1.50  2008/12/02 23:13:33Z  keithg
    Added initial BBM v5 block replacement policy.
    Revision 1.49  2008/11/12 23:50:10Z  billr
    Fix warnings about members of ioStat being used uninitialized.
    Revision 1.48  2008/10/24 04:54:28Z  keithg
    Added interface to BBM for creates and remapping and block info dispatch.
    Improved error handling with status information rather than asserts.
    Revision 1.47  2008/09/02 04:58:10Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.46  2008/05/08 03:13:50Z  garyp
    Reverted part of the changes from the previous rev.  Fixed the alignment
    issue in FMSLTEST instead.
    Revision 1.45  2008/05/05 21:15:57Z  garyp
    Only requires that the "pAltData" pointers be aligned on nAltDataSize
    boundaries.
    Revision 1.44  2008/03/22 22:20:40Z  Garyp
    Updated to support variable width tags.  Expanded the various Get/Set
    Page and Block Status functions.
    Revision 1.43  2008/01/13 07:28:00Z  keithg
    Function header updates to support autodoc.
    Revision 1.42  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.41  2007/10/11 21:40:56Z  pauli
    Corrected a problem introduced in revision 1.39 where memory corruption
    could occur if a misaligned buffer was given for an operation involving two
    or more pages.
    Revision 1.40  2007/10/10 02:28:51Z  Garyp
    Updated debugging messages.
    Revision 1.39  2007/10/01 21:41:17Z  pauli
    Resolved Bug 355: The FORCEALIGNEDIO feature is supports both read and
    write operations.  Reworked the initialization process to only allocate an
    aligned page buffer if necessary.  Added checks to ensure that data buffers
    meet the alignement requirements of the FIM.
    Revision 1.38  2007/08/29 17:14:59Z  pauli
    Added assertions to check that page size and pages per block are set.
    Revision 1.37  2007/08/02 22:11:46Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.  Fixed overflow in debug message print.
    Revision 1.36  2007/07/02 18:57:09Z  rickc
    Fixed issues introduced in Revision 1.35
    Revision 1.35  2007/06/29 21:09:28Z  rickc
    Added GetPhysicalBlockStatus()
    Revision 1.34  2007/04/30 14:07:11Z  garyp
    Updated debug code and comments.
    Revision 1.33  2007/04/08 02:16:58Z  Garyp
    Modified so that the latency parameter get/set functionality is conditional
    on the latency reduction features being enabled.  Updated so that cycling
    of the DEV mutex can be controlled at run-time whereas before it was done
    at compile time only.  Updated so that sleeping while waiting for erases to
    suspend or complete is disabled if all latency reduction features are
    disabled.
    Revision 1.32  2007/04/07 03:36:57Z  Garyp
    Modified the "ParameterGet" function so that the buffer size for a given
    parameter can be queried without actually retrieving the parameter.
    Revision 1.31  2007/03/24 22:33:08Z  Garyp
    Modified to allow the erase poll interval to be queried and set.
    Revision 1.30  2007/03/02 21:00:06Z  timothyj
    Added DclAssert() to ensure that NOR fims simulate a uPagesPerBlock in
    addition to a uPageSize.
    Revision 1.29  2007/03/01 19:58:04Z  timothyj
    Moved ulPagesPerBlock into the FimInfo structure as uPagesPerBlock, where
    it can be used by other modules to which FimInfo is public, rather than
    computing or storing an additional copy of the same value.  Modified call
    to FfxDevApplyArrayBounds() to pass and receive on return a return block
    index in lieu of byte offset, for NOR compatibility with NAND LFA changes.
    Revision 1.28  2007/02/03 02:35:18Z  Garyp
    Tweaked the autotune latency thresholds to provide more accurate timings.
    Revision 1.27  2007/02/01 23:30:12Z  billr
    Fix compiler warning about possible uninitilized variables.
    Revision 1.26  2007/01/31 23:41:37Z  Garyp
    Updated to allow the device timing and latency characteristics to be
    specified at the project level, as well as changed at run-time.  Eliminated
    the ability to specify timing characteristics in the FIM which was not used
    and flawed.  Allow the timing chacteristics to be determined at run-time if
    so configured.  Allow the erase-poll interval to be calculated at run time
    based on the flash performance.  Auto-calculated timings can be plugged
    into ffxconf.h and then the auto-timing feature can be disabled if desired.
    Fixed an issue where the mutex was not released when erasing if the
    device did not support erase suspend.  Allow the entire set of latency
    reducing features (including erase suspend support) to be compiled out
    of the product if so desired.
    Revision 1.25  2006/12/31 21:29:56Z  Garyp
    Added the DEV_OLDSTYLE_NORFIM so we can clearly distinguish old and new
    style NOR FIMs at the DevMgr level.  Fixed so that the artificial 64KB limit
    is only enforced for old-style NOR FIMs.  Updated the read-back-verify
    feature to hexdump the data if the comparison fails.
    Revision 1.24  2006/11/14 05:13:24Z  Garyp
    Modified to allow the read/write/erase latency settings to be queried or set.
    Revision 1.23  2006/11/10 00:20:32Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.22  2006/10/18 17:45:11Z  billr
    Revert changes made in revision 1.21.
    Revision 1.20  2006/10/06 01:03:30Z  Garyp
    Added the ARTIFICIAL_64K_LIMIT_FOR_OLD_NOR_FIMS setting to allow for old-
    style NOR FIMs which still can't process data chunks larger than 64K.
    Revision 1.19  2006/05/30 22:17:40Z  billr
    Fix compiler warning about uninitialized variable (gcc 4.1.0).
    Revision 1.18  2006/05/22 20:50:39Z  billr
    Some compilers won't accept a static function prototype without a
    definition.
    Revision 1.17  2006/05/19 19:03:19Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.16  2006/05/04 20:34:19Z  Garyp
    Cleaned up debug messages.
    Revision 1.15  2006/04/26 20:31:13Z  billr
    Pointers can be wider than 32 bits.
    Revision 1.14  2006/03/19 04:43:20Z  Garyp
    Updated debug code.
    Revision 1.13  2006/03/15 00:37:18Z  Garyp
    Fixed to not allocate the aligned I/O buffer on the stack.
    Revision 1.12  2006/03/13 23:34:37Z  Pauli
    Correctly set the total number of blocks after applying the array bounds.
    Revision 1.11  2006/03/10 07:11:52Z  Garyp
    Updated debug code.
    Revision 1.10  2006/03/09 01:08:02Z  Garyp
    Updated to use some helper macros for readability purposes.
    Revision 1.9  2006/03/06 01:00:37Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.8  2006/02/23 00:32:00Z  Garyp
    Minor tweaks to the readback-verify function.  No functional changes.
    Revision 1.7  2006/02/22 19:05:32Z  Garyp
    Conditioned NAND specific code.  Updated to use the system-wide max page
    size.
    Revision 1.6  2006/02/15 19:23:28Z  Garyp
    Added support for a raw block count.
    Revision 1.5  2006/02/15 05:21:10Z  Garyp
    Added reserved space support for NOR FIMs.
    Revision 1.4  2006/02/12 00:44:08Z  Garyp
    Tweaked to build cleanly.
    Revision 1.3  2006/02/11 03:19:01Z  Garyp
    Removed dead code.
    Revision 1.2  2006/02/11 00:13:00Z  Garyp
    Renamed the NOR_Wrapper FIMDEVICE structure.
    Revision 1.1  2006/02/10 01:01:58Z  Garyp
    Initial revision
    Revision 1.4  2006/01/13 22:23:55Z  Garyp
    Updated to build cleanly with CE.
    Revision 1.3  2006/01/13 18:15:25Z  billr
    Merge Sibley support from v2.01
    Revision 1.2  2005/12/26 02:03:58Z  Garyp
    Reduced reasonable length read and writes (somewhat).
    Revision 1.1  2005/11/27 05:51:08Z  Pauli
    Initial revision
    Revision 1.8  2005/11/27 05:51:07Z  Garyp
    Disabled the MEM_VALIDATE functionality for oemwrite() since it is not used.
    Revision 1.7  2005/11/14 17:51:58Z  Garyp
    Minor doc/formatting changes -- nothing functional.
    Revision 1.6  2005/11/09 18:34:39Z  Garyp
    Eliminated the use of STD_NAND_PAGE_SIZE, but rather use the page size
    reported by the FIM.
    Revision 1.5  2005/10/22 07:09:03Z  garyp
    Renamed the ReadBackVerify option.
    Revision 1.4  2005/10/21 03:45:39Z  garyp
    Added ReadbackVerify functionality for HiddenWrite.
    Revision 1.3  2005/10/11 19:56:13Z  Garyp
    Syntax fix.
    Revision 1.2  2005/10/11 17:30:23Z  Garyp
    Removed singlethreaded FIM support.  Modified the MEM_NAND_PAGE stuff to
    support multi-page requests, resulting in some refactored helper functions.
    Revision 1.1  2005/10/06 06:51:10Z  Garyp
    Initial revision
    Revision 1.3  2005/09/20 19:01:08Z  pauli
    Replaced _syssleep with DclOsSleep.
    Revision 1.2  2005/08/18 01:37:56Z  garyp
    Increased the REASONABLE_LENGTH_READ/WRITE values and reduced the
    ERASE_POLL_INTERVAL (temporary hacks).
    Revision 1.1  2005/08/03 19:29:10Z  pauli
    Initial revision
    Revision 1.4  2005/08/03 19:29:10Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.2  2005/07/31 03:52:43Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.1  2005/07/28 04:55:00Z  pauli
    Initial revision
    Revision 1.203  2005/08/26 21:42:22Z  Cheryl
    Added changes to support ISWF flash.  Some defines related
    to page size are now able to be defined in oemconf.h.  MEM_COMMON
    and MEM_CONTROL are treated differently in ISWF flash cases.
    Utility functions have been added to update ISWF MEM_CONTROL addresses.
    Revision 1.202  2005/06/09 16:55:38Z  billr
    Formatting changes for readability, no functional changes.
    Revision 1.201  2005/06/09 16:45:07Z  billr
    Fix mutual exclusion when oemerase() is reentered (thanks, Jose!).
    Revision 1.200  2005/05/11 17:46:34Z  garyp
    Minor debug code change.
    Revision 1.199  2005/04/25 02:54:24Z  garyp
    Added support for FIMSCANMODE (still experimental, but turned off by
    default, and not yet documented).
    Revision 1.198  2005/02/03 19:26:16Z  GaryP
    Added support for FFXCONF_FORCEALIGNEDIO.  Was previously implemented in
    the Driver Framework.
    Revision 1.197  2004/12/30 23:24:54Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.196  2004/09/29 21:45:52Z  Pauli
    Instead of just calling sysfree(pEMI), it now calls fimmt_destroy();
    Revision 1.195  2004/08/30 00:47:07Z  GaryP
    Added debug code.
    Revision 1.194  2004/07/27 20:45:46Z  jaredw
    In function fimmt_create the pEMI structure is now initialized after memory
    is allocated.
    Revision 1.193  2004/07/21 20:05:30Z  GaryP
    Added debugging code.
    Revision 1.192  2004/07/07 01:25:29Z  GaryP
    Updated to use FfxHookOptionGet().
    Revision 1.191  2004/07/01 23:29:30Z  jaredw
    Added parens to please VxWorks Compiler
    Revision 1.19  2004/06/25 20:21:12Z  BillR
    New interface to multi-threaded FIMs.  New project hooks for initialization
    and configuration.
    Revision 1.18  2004/02/25 19:54:56Z  garys
    Merge from FlashFXMT
    Revision 1.15.1.12  2004/02/25 19:54:56  garyp
    Modified so that oemstatus() is supported for new-style FIMs.
    Revision 1.15.1.11  2004/02/22 21:32:10Z  garyp
    Eliminated oemchanged() and oempowerdown() functionality.  Modified to use
    the new Project Hooks FfxHookDriverCreate/Uninit() functions.
    Revision 1.15.1.10  2004/02/07 19:25:32Z  garyp
    Updated to use the new FFXCONF_... style settings.  Removed support for the
    oeminterface() function.
    Revision 1.15.1.9  2004/02/05 18:31:18Z  garyp
    Improved parameter validation code.  Modified oemmount() to pass the ulID
    field.
    Revision 1.15.1.8  2003/12/17 07:12:46Z  garyp
    Eliminated oemgethandle().  Fixed so the uDriveNum field is initialized
    prior to doing the mount for MT FIMs.
    Revision 1.15.1.7  2003/12/05 10:18:46Z  garyp
    Modified to use the new oemmount() function.
    Revision 1.15.1.6  2003/11/24 01:35:52Z  garyp
    Fixed a bad include directive.
    Revision 1.15.1.5  2003/11/22 00:44:50Z  garyp
    Modified to acquire the FXOEM mutex before calling into the old OEM Layer.
    Revision 1.15.1.3  2003/11/06 23:19:16Z  garyp
    Added the FIM ID number and debugging code.
    Revision 1.15.1.2  2003/11/04 21:14:50Z  garyp
    Re-checked into variant sandbox.
    Revision 1.16  2003/11/04 21:14:50Z  garyp
    Modified the FML entry points to take an hFML parameter.  Modified the
    mount process to separately process new and old style FIMs.
    Revision 1.15  2003/09/22 20:49:44Z  garys
    Removed debug asserts that FFX_RESERVED_SPACE is <= some constant, because
    of a compiler warning.
    Revision 1.14  2003/09/19 00:09:27  billr
    Change test for reserved space to eliminate a warning from some compilers
    when FFX_RESERVED_SPACE was zero.
    Revision 1.13  2003/09/12 18:57:03Z  dennis
    Need to test FFX_RESERVED_SPACE limit for
    MEM_NAND_PAGE as well as for MEM_VALIDATE access.
    Revision 1.12  2003/09/11 20:53:07  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.11  2003/05/08 01:21:08  garyp
    Modified to use ACCESS_MOUNTS in oemmount().
    Revision 1.10  2003/05/06 02:05:22Z  garys
    Comment out the ulTotalSize check in oemmount() so that RAM FIMs will work
    Revision 1.9  2003/05/02 04:43:22  garyp
    Modified so that oemmount() will only happen once.
    Revision 1.8  2003/04/21 18:28:06Z  garyp
    Eliminated LINUX_TARGET stuff and the inclusion of FIM.H.
    Revision 1.7  2003/03/22 20:36:26Z  garyp
    Modified oemmount() to return a D_BOOL success or failure code.
    Revision 1.6  2003/01/08 13:39:12Z  dennis
    Enable ECCs for NVBF page operations
    Revision 1.5  2002/11/30 23:14:14  dennis
    Removed NVBF dependencies from FMSL and tools.  Added defines to OEM.H for
    the data, redundant and page size of NAND (MGM) memories in the max and
    standard cases.  For now, the two cases are identical.
    Revision 1.4  2002/11/28 01:19:44  garys
    allow MEM_NAND_PAGE to be used even if not NVBF
    Revision 1.3  2002/11/12 20:54:58  garyp
    Eliminated the use of __HAL_INTERFACE__.
    08/27/02 de  Made calls to Read/WriteNANDPage options for eomread/write.
                 The read/write type is MEM_NAND_PAGE.  It's all conditional
                 on NVBF getting defined in FLASHFX.H.
    08/01/02 gjs Renamed oemwriteNANDPage to oemwriteNANDpage (read also).
    07/29/02 de  Included NVBF.H and added symbolic constants.
    07/23/02 de  Updated comments.
    07/22/02 de  Several changes to provide better support for NAND.  Added two
                 new oemwriteNANDpage adn oemreadNANDpage routines that allow
                 writing all 528 bytes of a NAND page.
    02/07/02 DE  Updated copyright notice for 2002.
    07/31/00 de  Removed unconditional mount of all drives from oeminit.
    08/01/00 HDS Made new initialization code in oemmount and oemunmount apply
                 only to Linux version.
    07/24/00 HDS Made previous change apply only to Linux version.
    06/23/99 mrm Moved media scan from oeminit() to fd_revalidate().
    03/05/99 gjs Set bStatus to FALSE in default case for switches to avoid
                 CE 2.0 SH3 compiler warns.
    12/16/98 PKG Removed un-used initilization to avoid copmiler warns
    12/15/98 PKG Corrected NULL check, now uses 0L type casted.
    12/13/98 PKG More error checking in calls to access the media
    12/11/98 PKG Added call into the hardware (HAL) project specific code to
                 allow the OEM to have the same extendable interface.
    06/30/98 PKG Added use of the new MEM_ types defined in the include file.
                 These are fully backward compatible and allow the FIM more
                 flexibility by allowing the FIM to know when to use ECC or
                 CRC for additional data validation.
    12/03/97 HDS Changed uCurrentUnit to uCurrentDrive.
    11/25/97 HDS Changed NUM_UNITS to NUM_DRIVES.
    11/25/97 HDS Changed oemsetunit() call to oemsetcurrentdrive().
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    09/12/97 DE  Added checks in the oemread, write, and erase calls for
                 change, write-protect, and not ready errors
    08/26/97 PKG Now uses new Datalight specific types
    06/16/97 PKG Added oemunmount()
    06/11/97 PKG Added Access calls around mounts since some flash memories
                 require Vpp in order to identify them (Sharp LH28F008).
    05/22/97 PKG Added uRomSeg for the BIOS option: STORE_ROM_SEG
    05/02/97 DM  Removed use of NULL, use zero instead
    04/01/97 PKG Added error simulation structure and code for debugging
    12/23/96 PKG Created
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "devmgr.h"
#include "devbbm.h"

/*  To be obsoleted as soon as possible.  Setting this value to FALSE if
    you know that none of the old NOR FIMs are being used will reduce the
    driver code size.
*/
#define ARTIFICIAL_64K_LIMIT_FOR_OLD_NOR_FIMS   (FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT)

#if FFXCONF_LATENCYAUTOTUNE
/*  This is the minimum number of pages/blocks to be processed before
    the latency settings can be auto-tuned.  There are different erase
    thresholds for NAND and NOR because NAND erases so fast relative to
    most timer resolutions, that we need to have a larger sample in
    order to obtain statistical data in which the average erase time
    comes out to be reasonably accurate.
*/
#define AUTOTUNE_READ_THRESHOLD        (400)
#define AUTOTUNE_WRITE_THRESHOLD       (200)
#define AUTOTUNE_ERASE_NOR_THRESHOLD     (3)
#define AUTOTUNE_ERASE_NAND_THRESHOLD   (50)
#endif

#define DEFAULT_PAGE_SIZE              (512)

#if FFX_DEVMGR_READBACKVERIFY
static D_BOOL ReadBackVerify(FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_WRITE_GENERIC *pDevReq, unsigned nAltDataLen, D_UINT32 ulCount);

/*  NOTE: It appears that according to this logic, if there was only a
          partial write (likely due to an I/O error), then we will not
          validate that the partial write was completed.
*/
#define READBACKVERIFY(handle, req, altlen, iostat)                         \
    if((iostat).ulCount && ((iostat).ffxStat == FFXSTAT_SUCCESS))           \
        if(!ReadBackVerify((handle), (req), (altlen), (iostat).ulCount))    \
            (iostat).ffxStat = FFXSTAT_DEVMGR_READBACKVERIFYFAILED;
#else
#define READBACKVERIFY(handle, req, altlen, iostat)
#endif

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
extern FIMDEVICE FFXFIM_NOR_Wrapper;
#endif

static FFXIOSTATUS  GenericRead( FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_READ_GENERIC *pDevReq, unsigned nAltDataLen);
static FFXIOSTATUS  GenericWrite(FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_WRITE_GENERIC *pDevReq, unsigned nAltDataLen);
static FFXIOSTATUS  EraseBlocks( FFXFIMDEVHANDLE hFimDev, FFXIOR_DEV_ERASE_BLOCKS *pDevReq);
static D_BOOL       EraseResume( FFXFIMDEVHANDLE hFimDev);
static D_BOOL       EraseSuspend(FFXFIMDEVHANDLE hFimDev);
static FFXIOSTATUS  ErasePoll(   FFXFIMDEVHANDLE hFimDev);

static FFXFIMDEVHANDLE aFIMHandles[FFX_MAX_DEVICES];


                    /*----------------------------*\
                     *                            *
                     *    Published Interface     *
                     *                            *
                    \*----------------------------*/


/*-------------------------------------------------------------------
    Protected: FfxDevCreate()

    Parameters:

    Return Value:
        Returns an FFXFIMDEVHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXFIMDEVHANDLE FfxDevCreate(
    FFXDEVHANDLE        hDev,
    D_UINT16            uDeviceNum)
{
    D_UINT16            i;
    D_BOOL              fAccessGranted = FALSE;
    FIMDEVICE         **ppFIMList;
    FFXFIMDEVICE       *pFIM = NULL;
    FFXSTATUS           ffxStat;
    char                achMutexName[DCL_MUTEXNAMELEN];
    FFXFIMBOUNDS        bounds;
    D_UINT32            ulTotalBlocks;
  #ifdef FIMSCANMODE
    D_UINT16            uMounted = 0;
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEINDENT),
        "FfxDevCreate() hDev=%P DeviceNum=%U\n", hDev, uDeviceNum));

    if(uDeviceNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("DEV%U out of range (%u)\n", uDeviceNum, FFX_MAX_DEVICES));
        goto ErrorCleanup;
    }

    ppFIMList = ppFIMDevList[uDeviceNum];
    if(!ppFIMList)
    {
        FFXPRINTF(1, ("A FIM for DEV%U is not defined\n", uDeviceNum));
        goto ErrorCleanup;
    }

    /*  Don't allow a device to be mounted more than once
    */
    if(aFIMHandles[uDeviceNum])
    {
        FFXPRINTF(1, ("FIM DEV%U already mounted\n", uDeviceNum));
        goto ErrorCleanup;
    }

    /*  allocate a structure for this potential device
    */
    pFIM = DclMemAllocZero(sizeof *pFIM);
    if(!pFIM)
    {
        FFXPRINTF(1, ("FfxDevCreate() Cannot allocate device structure, no memory\n"));
        goto ErrorCleanup;
    }

    pFIM->hDev = hDev;
    pFIM->uDeviceNum = uDeviceNum;

    /*  Reality check asserts to ensure that we can handle
        FIM ID #'s up to at least 0xFF.
    */
    DclAssert(uDeviceNum <= 0xFF);
    DclAssert(DCL_MUTEXNAMELEN >= 8);

    DclSNPrintf(achMutexName, sizeof(achMutexName), "FXDEV%02X", uDeviceNum);

  #if FFX_USE_EVENT_MANAGER
    /*  Create the Event Manager first, in case anyone wants to register
        an event handler procedure.
    */
    pFIM->eventMgrInstance = FfxEventMgrCreate();
    if (!pFIM->eventMgrInstance)
    {
        DclError();
        goto ErrorCleanup;
    }
  #endif

    pFIM->errMgrInstance = FfxErrMgrCreate(pFIM);
    if (!pFIM->errMgrInstance)
    {
        DclError();
        goto ErrorCleanup;
    }

    pFIM->pMutex = DclMutexCreate(achMutexName);
    if(!pFIM->pMutex)
    {
        FFXPRINTF(1, ("FfxDevCreate() cannot acquire mutex\n"));
        goto ErrorCleanup;
    }

    ffxStat = FfxHookAccessBegin(hDev, ACCESS_MOUNTS);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxHookAccessBegin() failed with status %lX\n", ffxStat));
        goto ErrorCleanup;
    }

    fAccessGranted = TRUE;

    /*  Call the create routine for each FIM until we find one that works
    */
    for(i = 0; ppFIMList[i]; i++)
    {
        pFIM->pFimEntry = ppFIMList[i];

      #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
        if(pFIM->pFimEntry->Mount)
        {
            FFXFIM_NOR_Wrapper.pOldFimDevice = pFIM->pFimEntry;
            pFIM->pFimEntry = &FFXFIM_NOR_Wrapper;
        }
      #endif

        /*  See if we can successfully create an instance for this FIM
        */
        DclAssert(pFIM->pFimEntry->Create);
        pFIM->hInst = pFIM->pFimEntry->Create(hDev, &pFIM->pFimInfo);
        if(pFIM->hInst)
        {
            DclAssert(pFIM->pFimInfo);

          #ifdef FIMSCANMODE
            {
                DclAssert(pFIM->pFimEntry->Destroy);
                pFIM->pFimEntry->Destroy(pFIM->hInst);
                pFIM->hInst = NULL;
                pFIM->pFimInfo = NULL;
                uMounted++;
                continue;
            }
          #endif

            break;
        }
    }

  #ifdef FIMSCANMODE
    FFXPRINTF(1, ("FIM Scan Mode found %U FIM(s) that can mount\n", uMounted));

    /*  Mount failed.
    */
    goto ErrorCleanup;
  #endif

    if(!pFIM->hInst)
        goto ErrorCleanup;

    /*  Consider any reserved size, as well as the max array size,
        reducing ulTotalBlocks as necessary.
    */
    FfxDevGetArrayBounds(hDev, &bounds);
    ulTotalBlocks = FfxDevApplyArrayBounds(pFIM->pFimInfo->ulTotalBlocks,
            pFIM->pFimInfo->ulBlockSize, &bounds);
    if(ulTotalBlocks == D_UINT32_MAX)
        goto ErrorCleanup;
    pFIM->ulReservedBlocks =  bounds.ulReservedBottomKB
            / (pFIM->pFimInfo->ulBlockSize / 1024UL);

    /*  Record the total blocks available for FlashFX use.  The BBM may
        further reduce the available ulTotalBlocks to accommodate internal
        structures used to manage bad blocks.
    */
    pFIM->ulTotalBlocks = pFIM->ulRawBlocks = ulTotalBlocks;

  #if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT
    if( (pFIM->pFimInfo->uDeviceType == DEVTYPE_NAND) &&
        (pFIM->pFimInfo->uDeviceFlags & DEV_REQUIRES_BBM) )
    {
        /*  Allocate a buffer big enough for the main page plus spare area.
        */
        pFIM->rgbPageBuffer = DclMemAlloc(pFIM->pFimInfo->uPageSize + pFIM->pFimInfo->uSpareSize);
        if(!pFIM->rgbPageBuffer)
            goto ErrorCleanup;

        /*  Note that the create of the BBM may further modify the
            reserved space to accommodate blocks that are reserved for
            use by the BBM.
        */
        if(FfxDevBbmCreate(pFIM) != FFXSTAT_SUCCESS)
            goto ErrorCleanup;
    }
  #endif

    /*  Old-style NOR FIMs simulate page size and pages-per-block count to
        simpify the higher level algorithms.  If the page size is zero, set
        it to the desired page size to emulate.  The pages-per-block value
        will be initialized as well.
    */
    if(pFIM->pFimInfo->uPageSize == 0)
    {
        D_UINT16    uPageSize;

        DclAssert(pFIM->pFimInfo->uPagesPerBlock == 0);
        DclAssert(pFIM->pFimInfo->uDeviceType == DEVTYPE_NOR);

        if(!FfxHookOptionGet(FFXOPT_DEVICE_PAGESIZE, hDev, &uPageSize, sizeof uPageSize))
            uPageSize = DEFAULT_PAGE_SIZE;

        if(pFIM->pFimEntry->ParameterSet(pFIM->hInst, FFXPARAM_PAGE_SIZE, &uPageSize, sizeof(uPageSize)) != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("FfxDevCreate() FIM page size is not valid\n"));
            goto ErrorCleanup;
        }
        DclAssert(pFIM->pFimInfo->uPageSize);
        DclAssert(pFIM->pFimInfo->uPagesPerBlock);
    }

    /*  One way or another, pages-per-block must be set by now.
    */
    DclAssert(pFIM->pFimInfo->uPagesPerBlock);

    pFIM->ulTotalPages      = pFIM->ulTotalBlocks * pFIM->pFimInfo->uPagesPerBlock;
    pFIM->ulRawTotalPages   = pFIM->ulRawBlocks * pFIM->pFimInfo->uPagesPerBlock;

    FfxDevLatencyInit(pFIM);

  #if ARTIFICIAL_64K_LIMIT_FOR_OLD_NOR_FIMS
    /*  For all old-style NOR/Sibley FIMs, the data lengths are specified as a
        D_UINT16 length.  Using the latency feature to ensure that the I/O is
        broken up into small enough chunks is simpler to handle here than it
        is in the Media Manager.
    */
    if(pFIM->pFimInfo->uDeviceFlags & DEV_OLDSTYLE_NORFIM)
    {
        DclAssert(pFIM->pFimInfo->uDeviceType != DEVTYPE_NAND);

        if(pFIM->ulMaxReadCount > D_UINT16_MAX / pFIM->pFimInfo->uPageSize)
        {
            pFIM->ulMaxReadCount = D_UINT16_MAX / pFIM->pFimInfo->uPageSize;

            FFXPRINTF(1, ("DEV%U read count reduced to %lU pages to accommodate old style NOR FIMs\n",
                uDeviceNum, pFIM->ulMaxReadCount));
        }

        if(pFIM->ulMaxWriteCount > D_UINT16_MAX / pFIM->pFimInfo->uPageSize)
        {
            pFIM->ulMaxWriteCount = D_UINT16_MAX / pFIM->pFimInfo->uPageSize;

            FFXPRINTF(1, ("DEV%U write count reduced to %lU pages to accommodate old style NOR FIMs\n",
                uDeviceNum, pFIM->ulMaxWriteCount));
        }

      #if FFXCONF_LATENCYAUTOTUNE
        /*  If latency auto-tuning is turned on, set this flag to ensure that
            it does not unintentionally tune the value such that it breaks
            the 64K limit stuff we just calculated here.
        */
        pFIM->fReduceOnly = TRUE;
      #endif

    }
  #endif

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    FfxDevBootBlockInit(pFIM);
  #endif

    /*  Sanity check the minimum alignment allowed for this FIM.
    */
    if(!DCLISPOWEROF2(pFIM->pFimInfo->uAlignSize) || !pFIM->pFimInfo->uAlignSize)
    {
        FFXPRINTF(1, ("The FIM reported an invalid alignment size, %U.\n",
                      pFIM->pFimInfo->uAlignSize));
        goto ErrorCleanup;
    }

    /*  Check if this FIM requires special alignment handling.
    */
    if(pFIM->pFimInfo->uAlignSize > 1)
    {
        /*  Make sure we can provide the necessary alignment.
        */
        if(pFIM->pFimInfo->uAlignSize > DCL_ALIGNSIZE)
        {
            FFXPRINTF(1, ("FIM alignment requirements exceed DCL_ALIGNSIZE.\n"));
            goto ErrorCleanup;
        }

      #if FFXCONF_FORCEALIGNEDIO
        /*  Allocate a one page, aligned, buffer so we can ensure proper
            alignment for the FIM.
        */
        pFIM->pAlignedIOBuffer = DclMemAlloc(pFIM->pFimInfo->uPageSize);
        if(!pFIM->pAlignedIOBuffer)
        {
            FFXPRINTF(1, ("Failed to allocated aligned IO buffer.\n"));
            goto ErrorCleanup;
        }
      #else
        /*  Output a big warning message that aligned buffers are required.
        */
        FFXPRINTF(1, ("WARNING: FIM requires aligned I/O but FFXCONF_FORCEALIGNEDIO is FALSE.\n"));
        FFXPRINTF(1, ("         All buffers must be properly aligned on %u-bit boundaries.\n",
                      pFIM->pFimInfo->uAlignSize * 8));
      #endif
    }

    FfxHookAccessEnd(hDev, ACCESS_MOUNTS);

    FFXPRINTF(1, ("FfxDevCreate() DEV%U Type=%X Flags=%X TotalBlocks=%lX BlockSize=%lX PageSize=%U\n",
        uDeviceNum, pFIM->pFimInfo->uDeviceType, pFIM->pFimInfo->uDeviceFlags,
        pFIM->ulTotalBlocks,
        pFIM->pFimInfo->ulBlockSize, pFIM->pFimInfo->uPageSize));

    aFIMHandles[uDeviceNum] = pFIM;

    goto MountCleanup;

  ErrorCleanup:

    if(fAccessGranted)
        FfxHookAccessEnd(hDev, ACCESS_MOUNTS);

    if(pFIM)
    {
        /*  Free resources in reverse order of allocation
        */

      #if FFXCONF_FORCEALIGNEDIO
        if(pFIM->pAlignedIOBuffer)
            DclMemFree(pFIM->pAlignedIOBuffer);
      #endif

      #if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT
        if (pFIM->pFimInfo)
        {
            if(pFIM->pFimInfo->uDeviceType == DEVTYPE_NAND)
            {
                ffxStat = FfxDevBbmDestroy(pFIM);
                DclAssert(ffxStat == FFXSTAT_SUCCESS);
            }
        }

        if(pFIM->rgbPageBuffer)
            DclMemFree(pFIM->rgbPageBuffer);
      #endif

        if(pFIM->hInst)
        {
            FfxHookAccessBegin(hDev, ACCESS_UNMOUNTS);

            pFIM->pFimEntry->Destroy(pFIM->hInst);

            FfxHookAccessEnd(hDev, ACCESS_UNMOUNTS);
        }

        if(pFIM->pMutex)
            DclMutexDestroy(pFIM->pMutex);

        if (pFIM->errMgrInstance)
            FfxErrMgrDestroy(pFIM->errMgrInstance);

      #if FFX_USE_EVENT_MANAGER
        if (pFIM->eventMgrInstance)
            FfxEventMgrDestroy(pFIM->eventMgrInstance);
      #endif

        DclMemFree(pFIM);

        pFIM = NULL;
    }

  MountCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDevCreate() returning hFIM=%P\n", pFIM));

    return pFIM;
}


/*-------------------------------------------------------------------
    Protected: FfxDevDestroy()

    Parameters:

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDevDestroy(
    FFXFIMDEVHANDLE    hFimDev)
{
    /*  Reality checks
    */
    DclAssert(hFimDev);
    DclAssert(hFimDev->uDeviceNum < FFX_MAX_DEVICES);
    DclAssert(aFIMHandles[hFimDev->uDeviceNum] == hFimDev);

    /*  Release resources in reverse order of allocation
    */

  #if FFXCONF_FORCEALIGNEDIO
    if(hFimDev->pAlignedIOBuffer)
        DclMemFree(hFimDev->pAlignedIOBuffer);
  #endif

  #if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT
    if(hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND)
    {
        if(FfxDevBbmDestroy(hFimDev) != FFXSTAT_SUCCESS)
        {
            DclError();
            goto ErrorCleanup;
        }
    }

    if(hFimDev->rgbPageBuffer)
        DclMemFree(hFimDev->rgbPageBuffer);
  #endif

    FfxHookAccessBegin(hFimDev->hDev, ACCESS_UNMOUNTS);

    hFimDev->pFimEntry->Destroy(hFimDev->hInst);

    FfxHookAccessEnd(hFimDev->hDev, ACCESS_UNMOUNTS);

    DclMutexDestroy(hFimDev->pMutex);

    FfxErrMgrDestroy(hFimDev->errMgrInstance);

  #if FFX_USE_EVENT_MANAGER
    if (hFimDev->eventMgrInstance)
        FfxEventMgrDestroy(hFimDev->eventMgrInstance);
  #endif

    /*  Mark the slot as unused
    */
    aFIMHandles[hFimDev->uDeviceNum] = NULL;

    DclMemFree(hFimDev);

  #if FFXCONF_NANDSUPPORT && FFXCONF_BBMSUPPORT
  ErrorCleanup:
  #endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: FfxDevHandle()

    Parameters:

    Return Value:
        Returns an FFXFIMDEVHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXFIMDEVHANDLE FfxDevHandle(
    D_UINT16        uDeviceNum)
{
    if(uDeviceNum >= FFX_MAX_DEVICES)
    {
        FFXPRINTF(1, ("FfxDevHandle() DEV%U out of range (%u)\n", uDeviceNum, FFX_MAX_DEVICES));

        return NULL;
    }

    return aFIMHandles[uDeviceNum];
}


/*-------------------------------------------------------------------
    Protected: FfxDevInfo()

    Parameters:

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDevInfo(
    FFXFIMDEVHANDLE    hFimDev,
    FFXFIMDEVINFO     *pDevInfo)
{
    /*  Reality checks
    */
    DclAssert(hFimDev);
    DclAssert(hFimDev->uDeviceNum < FFX_MAX_DEVICES);
    DclAssert(aFIMHandles[hFimDev->uDeviceNum] == hFimDev);
    DclAssert(pDevInfo);

    DclMemSet(pDevInfo, 0, sizeof *pDevInfo);

    pDevInfo->nDeviceNum            = (unsigned)hFimDev->uDeviceNum;
    pDevInfo->ulRawBlocks           = hFimDev->ulRawBlocks;     /* includes any BBM area */
    pDevInfo->ulTotalBlocks         = hFimDev->ulTotalBlocks;   /* excludes any BBM area */
    pDevInfo->ulReservedBlocks      = hFimDev->ulReservedBlocks;

    pDevInfo->uDeviceType           = hFimDev->pFimInfo->uDeviceType;
    pDevInfo->uDeviceFlags          = hFimDev->pFimInfo->uDeviceFlags;
    pDevInfo->uLockFlags            = hFimDev->pFimInfo->uLockFlags;
    pDevInfo->ulChipBlocks          = hFimDev->pFimInfo->ulChipBlocks;
    pDevInfo->ulBlockSize           = hFimDev->pFimInfo->ulBlockSize;
    pDevInfo->uPageSize             = hFimDev->pFimInfo->uPageSize;
    pDevInfo->uSpareSize            = hFimDev->pFimInfo->uSpareSize;
    pDevInfo->uMetaSize             = hFimDev->pFimInfo->uMetaSize;
    pDevInfo->uAlignSize            = hFimDev->pFimInfo->uAlignSize;

  #if FFXCONF_BBMSUPPORT
    if(hFimDev->pBbm)
        pDevInfo->uDeviceFlags |= DEV_USING_BBM;
    pDevInfo->ulBbmChips          = hFimDev->ulBbmChips;
  #endif

  #if FFXCONF_OTPSUPPORT
    pDevInfo->nOTPPages             = hFimDev->pFimInfo->nOTPPages;
  #endif

    pDevInfo->uEdcSegmentSize       = hFimDev->pFimInfo->uEdcSegmentSize;
    pDevInfo->uEdcCapability        = hFimDev->pFimInfo->uEdcCapability;      /* Max error correcting capability */
    pDevInfo->uEdcRequirement       = hFimDev->pFimInfo->uEdcRequirement;     /* Required level of error correction */
    pDevInfo->ulEraseCycleRating    = hFimDev->pFimInfo->ulEraseCycleRating;  /* Factory spec erase cycle rating */
    pDevInfo->ulBBMReservedRating   = hFimDev->pFimInfo->ulBBMReservedRating; /* Reserved blocks required for above */

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    /*  These structure members exist only if NOR support is enabled.
    */
    pDevInfo->ulBootBlockSize       = hFimDev->pFimInfo->ulBootBlockSize;
    pDevInfo->uLowBootBlockCount    = hFimDev->pFimInfo->uBootBlockCountLow;
    pDevInfo->uHighBootBlockCount   = hFimDev->pFimInfo->uBootBlockCountHigh;
  #endif

    return TRUE;
}


/*-------------------------------------------------------------------
    Protected: FfxDevParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFimDev   - The FIM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the buffer in which to store the
                    value.  May be NULL which causes this function
                    to return the size of buffer required to store
                    the parameter.
        ulBuffLen - The size of buffer.  May be zero if pBuffer is
                    NULL.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
        If pBuffer is NULL, but the id is valid, the returned
        FFXSTATUS value will be decodeable using the macro
        DCLSTAT_GETUINT20(), which will return the buffer length
        required to store the parameter.
-------------------------------------------------------------------*/
FFXSTATUS FfxDevParameterGet(
    FFXFIMDEVHANDLE hFimDev,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDevParameterGet() hFimDev=%P ID=%x pBuff=%P Len=%lU\n",
        hFimDev, id, pBuffer, ulBuffLen));

    switch(id)
    {
      #if FFXCONF_LATENCYREDUCTIONENABLED
        case FFXPARAM_DEVMGR_MAXREADCOUNT:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFimDev->ulMaxReadCount));
            }
            else
            {
                if(ulBuffLen == sizeof(hFimDev->ulMaxReadCount))
                    *(D_UINT32*)pBuffer = hFimDev->ulMaxReadCount;
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        case FFXPARAM_DEVMGR_MAXWRITECOUNT:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFimDev->ulMaxWriteCount));
            }
            else
            {
                if(ulBuffLen == sizeof(hFimDev->ulMaxWriteCount))
                    *(D_UINT32*)pBuffer = hFimDev->ulMaxWriteCount;
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        case FFXPARAM_DEVMGR_MAXERASECOUNT:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFimDev->ulMaxEraseCount));
            }
            else
            {
                if(ulBuffLen == sizeof(hFimDev->ulMaxEraseCount))
                    *(D_UINT32*)pBuffer = hFimDev->ulMaxEraseCount;
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        case FFXPARAM_DEVMGR_ERASEPOLLINTERVAL:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFimDev->uErasePollInterval));
            }
            else
            {
                if(ulBuffLen == sizeof(hFimDev->uErasePollInterval))
                    *(D_UINT16*)pBuffer = hFimDev->uErasePollInterval;
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        case FFXPARAM_DEVMGR_CYCLEMUTEX:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFimDev->fMutexCycle));
            }
            else
            {
                if(ulBuffLen == sizeof(hFimDev->fMutexCycle))
                    *(unsigned*)pBuffer = hFimDev->fMutexCycle;
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;
      #endif

      #if FFXCONF_BBMSUPPORT
        case FFXPARAM_STATS_BBM:
          #if FFXCONF_STATS_BBM
            if(hFimDev->pBbm)
            {
                if(!pBuffer)
                {
                    /*  A buffer was not supplied -- return an indicator
                        that the parameter is valid and the buffer length
                        required to hold it.
                    */
                    ffxStat = DCLSTAT_SETUINT20(sizeof(FFXBBMSTATS));
                }
                else
                {
                    ffxStat = FfxDevBbmStatsQuery(hFimDev, (FFXBBMSTATS *) pBuffer);
                }
            }
            else
            {
                ffxStat = FFXSTAT_CATEGORYDISABLED;
            }
          #else
            ffxStat = FFXSTAT_CATEGORYDISABLED;
          #endif
            break;
      #endif

        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = hFimDev->pFimEntry->ParameterGet(hFimDev->hInst, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDevParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxDevParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFimDev   - The FIM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxDevParameterSet(
    FFXFIMDEVHANDLE hFimDev,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDevParameterSet() hFimDev=%P ID=%x pBuff=%P Len=%lU\n",
        hFimDev, id, pBuffer, ulBuffLen));

    switch(id)
    {
      #if FFXCONF_LATENCYREDUCTIONENABLED
        case FFXPARAM_DEVMGR_MAXREADCOUNT:
        {
            D_UINT32    ulMax;

            if(ulBuffLen != sizeof(hFimDev->ulMaxReadCount))
            {
                FFXPRINTF(1, ("FfxDevParameterSet(FFXPARAM_DEVMGR_MAXREADCOUNT) invalid ulBuffLen parameter\n"));
                ffxStat = FFXSTAT_BADPARAMETER;
                goto ErrorCleanup;
            }

            ulMax = *(D_UINT32*)pBuffer;

          #if ARTIFICIAL_64K_LIMIT_FOR_OLD_NOR_FIMS
            if(hFimDev->pFimInfo->uDeviceFlags & DEV_OLDSTYLE_NORFIM)
            {
                if(ulMax > D_UINT16_MAX / hFimDev->pFimInfo->uPageSize)
                {
                    ulMax = D_UINT16_MAX / hFimDev->pFimInfo->uPageSize;

                    FFXPRINTF(1, ("Reduced count due to old-style NOR FIM limits\n"));
                }
            }
          #endif

            /*  Better not be setting this value to zero or we're going
                to have real problems.
            */                
            DclAssert(ulMax);

            hFimDev->ulMaxReadCount = ulMax;
            break;
        }

        case FFXPARAM_DEVMGR_MAXWRITECOUNT:
        {
            D_UINT32    ulMax;

            if(ulBuffLen != sizeof(hFimDev->ulMaxWriteCount))
            {
                FFXPRINTF(1, ("FfxDevParameterSet(FFXPARAM_DEVMGR_MAXWRITECOUNT) invalid ulBuffLen parameter\n"));
                ffxStat = FFXSTAT_BADPARAMETER;
                goto ErrorCleanup;
            }

            ulMax = *(D_UINT32*)pBuffer;

          #if ARTIFICIAL_64K_LIMIT_FOR_OLD_NOR_FIMS
            if(hFimDev->pFimInfo->uDeviceFlags & DEV_OLDSTYLE_NORFIM)
            {
                if(ulMax > D_UINT16_MAX / hFimDev->pFimInfo->uPageSize)
                {
                    ulMax = D_UINT16_MAX / hFimDev->pFimInfo->uPageSize;

                    FFXPRINTF(1, ("Reduced count due to old-style NOR FIM limits\n"));
                }
            }
          #endif

            /*  Better not be setting this value to zero or we're going
                to have real problems.
            */                
            DclAssert(ulMax);

            hFimDev->ulMaxWriteCount = ulMax;
            break;
        }

        case FFXPARAM_DEVMGR_MAXERASECOUNT:
            if(ulBuffLen != sizeof(hFimDev->ulMaxEraseCount))
            {
                FFXPRINTF(1, ("FfxDevParameterSet(FFXPARAM_DEVMGR_MAXERASECOUNT) invalid ulBuffLen parameter\n"));
                ffxStat = FFXSTAT_BADPARAMETER;
                goto ErrorCleanup;
            }

            hFimDev->ulMaxEraseCount = *(D_UINT32*)pBuffer;

            /*  Better not be setting this value to zero or we're going
                to have real problems.
            */                
            DclAssert(hFimDev->ulMaxEraseCount);

            break;

        case FFXPARAM_DEVMGR_ERASEPOLLINTERVAL:
            if(ulBuffLen != sizeof(hFimDev->uErasePollInterval))
            {
                FFXPRINTF(1, ("FfxDevParameterSet(FFXPARAM_DEVMGR_ERASEPOLLINTERVAL) invalid ulBuffLen parameter\n"));
                ffxStat = FFXSTAT_BADPARAMETER;
                goto ErrorCleanup;
            }

            hFimDev->uErasePollInterval = *(D_UINT16*)pBuffer;
            break;

        case FFXPARAM_DEVMGR_CYCLEMUTEX:
            if(ulBuffLen != sizeof(hFimDev->fMutexCycle))
            {
                FFXPRINTF(1, ("FfxDevParameterSet(FFXPARAM_DEVMGR_CYCLEMUTEX) invalid ulBuffLen parameter\n"));
                ffxStat = FFXSTAT_BADPARAMETER;
                goto ErrorCleanup;
            }

            hFimDev->fMutexCycle = *(unsigned*)pBuffer;
            break;
      #endif

        default:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.
            */
            ffxStat = hFimDev->pFimEntry->ParameterSet(hFimDev->hInst, id, pBuffer, ulBuffLen);
            break;
    }

  #if FFXCONF_LATENCYREDUCTIONENABLED
  ErrorCleanup:
  #endif
  
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDevParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Protected: FfxDevIORequest()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevIORequest(
    FFXFIMDEVHANDLE     hFimDev,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = INITIAL_UNKNOWNFLAGS_STATUS(FFXSTAT_DEVMGR_UNSUPPORTEDIOREQUEST);

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 2, TRACEINDENT),
        "FfxDevIORequest() hFimDev=%P Func=%x\n", hFimDev, pIOR->ioFunc));

    DclProfilerEnter("FfxDevIORequest", 0, 0);

    DclAssert(hFimDev);

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_DEV_ERASE_RAWBLOCKS:
        case FXIOFUNC_DEV_ERASE_BLOCKS:
        {
            ioStat = EraseBlocks(hFimDev, (FFXIOR_DEV_ERASE_BLOCKS*)pIOR);
            break;
        }

      #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
        case FXIOFUNC_DEV_ERASE_BOOT_BLOCKS:
        {
            ioStat = FfxDevEraseBootBlocks(hFimDev, (FFXIOR_DEV_ERASE_BOOT_BLOCKS*)pIOR);
            break;
        }
      #endif

        case FXIOFUNC_DEV_LOCK_FREEZE:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_LOCK_FREEZE))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_LOCK_FREEZE req = *(FFXIOR_DEV_LOCK_FREEZE*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_LOCK_FREEZE) == sizeof(FFXIOR_DEV_LOCK_FREEZE));

                req.ior.ioFunc = FXIOFUNC_FIM_LOCK_FREEZE;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
                }
            }
            break;
        }

        case FXIOFUNC_DEV_LOCK_BLOCKS:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_LOCK_BLOCKS))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_LOCK_BLOCKS req = *(FFXIOR_DEV_LOCK_BLOCKS*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_LOCK_BLOCKS) == sizeof(FFXIOR_DEV_LOCK_BLOCKS));

                req.ior.ioFunc = FXIOFUNC_FIM_LOCK_BLOCKS;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
                }
            }
            break;
        }

        case FXIOFUNC_DEV_UNLOCK_BLOCKS:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_UNLOCK_BLOCKS))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_UNLOCK_BLOCKS req = *(FFXIOR_DEV_UNLOCK_BLOCKS*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_UNLOCK_BLOCKS) == sizeof(FFXIOR_DEV_UNLOCK_BLOCKS));

                req.ior.ioFunc = FXIOFUNC_FIM_UNLOCK_BLOCKS;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
                }
            }
            break;
        }

      #if FFXCONF_OTPSUPPORT
        case FXIOFUNC_DEV_OTP_READPAGES:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_READ_OTPPAGES))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_READ_OTPPAGES req = *(FFXIOR_FIM_READ_OTPPAGES*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_READ_OTPPAGES) == sizeof(FFXIOR_DEV_READ_OTPPAGES));

                req.ior.ioFunc = FXIOFUNC_FIM_OTP_READPAGES;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_READS);
                }
            }
            break;
        }

        case FXIOFUNC_DEV_OTP_WRITEPAGES:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_WRITE_OTPPAGES))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_WRITE_OTPPAGES req = *(FFXIOR_FIM_WRITE_OTPPAGES*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_WRITE_OTPPAGES) == sizeof(FFXIOR_DEV_WRITE_OTPPAGES));

                req.ior.ioFunc = FXIOFUNC_FIM_OTP_WRITEPAGES;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
                }
            }
            break;
        }

        case FXIOFUNC_DEV_OTP_LOCK:
        {
            ioStat.ulFlags = IOFLAGS_BLOCK;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_LOCK_OTP))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_FIM_LOCK_OTP req = *(FFXIOR_FIM_LOCK_OTP*)pIOR;

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_LOCK_OTP) == sizeof(FFXIOR_DEV_LOCK_OTP));

                req.ior.ioFunc = FXIOFUNC_FIM_OTP_LOCK;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);
                }
            }
            break;
        }
      #endif

      #if FFXCONF_POWERSUSPENDRESUME
        case FXIOFUNC_DEV_POWER_SUSPEND:
        {
            ioStat.ulFlags = IOFLAGS_DEVICE;

			FFXPRINTF(1, ("FXIOFUNC_DEV_POWER_SUSPEND \n"));
            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_POWER_SUSPEND))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_DEV_POWER_SUSPEND req = *(FFXIOR_DEV_POWER_SUSPEND*)pIOR;

                FFXPRINTF(1, ("FXIOFUNC_DEV_POWER_SUSPEND() PowerState=%u\n", req.nPowerState));

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_POWER_SUSPEND) == sizeof(FFXIOR_DEV_POWER_SUSPEND));

                req.ior.ioFunc = FXIOFUNC_FIM_POWER_SUSPEND;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_SUSPEND);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    /*  Should consider whether we should leave the Device
                        mutex acquired until such time as a resume is
                        requested.  That would forcibly ensure that nobody
                        is inappropriately accessing the Device...
                    */    

                    FfxDevIOEpilog(hFimDev, ACCESS_SUSPEND);
                }
            }
            break;
        }

        case FXIOFUNC_DEV_POWER_RESUME:
        {
            ioStat.ulFlags = IOFLAGS_DEVICE;

            if(pIOR->ulReqLen != sizeof(FFXIOR_DEV_POWER_RESUME))
            {
                ioStat.ffxStat = FFXSTAT_BADSTRUCLEN;
            }
            else
            {
                FFXIOR_DEV_POWER_RESUME req = *(FFXIOR_DEV_POWER_RESUME*)pIOR;

                FFXPRINTF(1, ("FXIOFUNC_DEV_POWER_RESUME() PowerState=%u\n", req.nPowerState));

                /*  The DEV and FIM IORequest packets are identical, so assert
                    it so.  Then just replace the DEV function code with the
                    one for the FIM.
                */
                DclAssert(sizeof(FFXIOR_FIM_POWER_RESUME) == sizeof(FFXIOR_DEV_POWER_RESUME));

                req.ior.ioFunc = FXIOFUNC_FIM_POWER_RESUME;

                ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_RESUME);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &req.ior);

                    FfxDevIOEpilog(hFimDev, ACCESS_RESUME);
                }
            }
            break;
        }
      #endif

        case FXIOFUNC_DEV_READ_GENERIC:
        {
            FFXIOR_DEV_READ_GENERIC *pReq = (FFXIOR_DEV_READ_GENERIC*)pIOR;

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_DEV_READ_PAGES:
                {
                  #if FFXCONF_LATENCYAUTOTUNE
                    DCLTIMESTAMP    t = DCLINITTS;

                    /*  Note that the initializer in the statement above is
                        only required for some overly zealous compilers which
                        warn that the variable could used without having been
                        initialized.  We plainly are not doing so...
                    */

                    /*  If the latency setting has not been tuned yet, do so now
                    */
                    if(hFimDev->fReadAutoTune)
                        t = DclTimeStamp();
                  #endif

                    ioStat = GenericRead(hFimDev, pReq, 0);

                  #if FFXCONF_LATENCYAUTOTUNE
                    if(hFimDev->fReadAutoTune)
                    {
                        hFimDev->ulReadTimeMS += DclTimePassed(t);
                        hFimDev->ulReadCount += ioStat.ulCount;
                        if(hFimDev->ulReadTimeMS && hFimDev->ulReadCount >= AUTOTUNE_READ_THRESHOLD)
                        {
                            hFimDev->ulMaxReadCount = FfxDevCalcMaxReadCount(hFimDev,
                                (hFimDev->ulReadTimeMS*1000) / hFimDev->ulReadCount);
                            hFimDev->fReadAutoTune = FALSE;
                        }
                    }
                  #endif

                    break;
                }

              #if FFXCONF_NANDSUPPORT
                case FXIOSUBFUNC_DEV_READ_TAGS:
                case FXIOSUBFUNC_DEV_READ_TAGGEDPAGES:
                {
                    if(pReq->nAltDataSize == 0)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(A) invalid nAltDataSize\n"));
                        break;
                    }

                    if(pReq->nAltDataSize > hFimDev->pFimInfo->uMetaSize)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(B) invalid nAltDataSize\n"));
                        break;
                    }

                    ioStat = GenericRead(hFimDev, pReq, pReq->nAltDataSize);
                    break;
                }

                case FXIOSUBFUNC_DEV_READ_SPARES:
                case FXIOSUBFUNC_DEV_READ_RAWPAGES:
                case FXIOSUBFUNC_DEV_READ_NATIVEPAGES:
                case FXIOSUBFUNC_DEV_READ_UNCORRECTEDPAGES:
                {
                    if(hFimDev->pFimInfo->uSpareSize == 0)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest() invalid internal spare size\n"));
                        ioStat.ffxStat = FFXSTAT_INVALIDSTATE;
                        ioStat.ulFlags = IOFLAGS_PAGE;
                        break;
                    }

                    ioStat = GenericRead(hFimDev, pReq, hFimDev->pFimInfo->uSpareSize);
                    break;
                }
              #endif
                default:
                {
                    FFXPRINTF(1, ("FfxDevIORequest() Bad Read Generic SubFunction %x\n", pReq->ioSubFunc));
                    break;
                }
            }

            break;
        }

        case FXIOFUNC_DEV_WRITE_GENERIC:
        {
            FFXIOR_DEV_WRITE_GENERIC *pReq = (FFXIOR_DEV_WRITE_GENERIC*)pIOR;

            switch(pReq->ioSubFunc)
            {
                case FXIOSUBFUNC_DEV_WRITE_PAGES:
                {
                  #if FFXCONF_LATENCYAUTOTUNE
                    DCLTIMESTAMP    t = DCLINITTS;

                    /*  Note that the initializer in the statement above is
                        only required for some overly zealous compilers which
                        warn that the variable could used without having been
                        initialized.  We plainly are not doing so...
                    */

                    /*  If the latency setting has not been tuned yet, do so now
                    */
                    if(hFimDev->fWriteAutoTune)
                        t = DclTimeStamp();
                  #endif

                    ioStat = GenericWrite(hFimDev, pReq, 0);

                  #if FFXCONF_LATENCYAUTOTUNE
                    if(hFimDev->fWriteAutoTune)
                    {
                        hFimDev->ulWriteTimeMS += DclTimePassed(t);
                        hFimDev->ulWriteCount += ioStat.ulCount;
                        if(hFimDev->ulWriteTimeMS && hFimDev->ulWriteCount >= AUTOTUNE_WRITE_THRESHOLD)
                        {
                            hFimDev->ulMaxWriteCount = FfxDevCalcMaxWriteCount(hFimDev,
                                (hFimDev->ulWriteTimeMS*1000) / hFimDev->ulWriteCount);
                            hFimDev->fWriteAutoTune = FALSE;
                        }
                    }
                  #endif

                    READBACKVERIFY(hFimDev, pReq, 0, ioStat);
                    if (ioStat.ffxStat == FFXSTAT_DEVMGR_READBACKVERIFYFAILED)
                    {
                        D_UINT32    ulPageOffset;
                        D_UINT32    ulBlockNum;

                        ulBlockNum = (pReq->ulStartPage + ioStat.ulCount) / hFimDev->pFimInfo->uPagesPerBlock;
                        ulPageOffset = (pReq->ulStartPage + ioStat.ulCount) % hFimDev->pFimInfo->uPagesPerBlock;
                        FfxErrMgrReportError(hFimDev,
                                             hFimDev->errMgrInstance,
                                             &ioStat,
                                             ulBlockNum,
                                             ulPageOffset);
                    }

                    break;
                }

              #if FFXCONF_NANDSUPPORT
                case FXIOSUBFUNC_DEV_WRITE_TAGGEDPAGES:
                {
                  #if FFXCONF_LATENCYAUTOTUNE
                    DCLTIMESTAMP    t = DCLINITTS;

                    /*  Note that the initializer in the statement above is
                        only required for some overly zealous compilers which
                        warn that the variable could used without having been
                        initialized.  We plainly are not doing so...
                    */

                    /*  If the latency setting has not been tuned yet, do so now
                    */
                    if(hFimDev->fWriteAutoTune)
                        t = DclTimeStamp();
                  #endif

                    if(pReq->nAltDataSize == 0)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(Write A): invalid nAltDataSize\n"));
                    }

                    if(pReq->nAltDataSize > hFimDev->pFimInfo->uMetaSize)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(Write B) invalid nAltDataSize\n"));
                    }

                    ioStat = GenericWrite(hFimDev, pReq, pReq->nAltDataSize);

                  #if FFXCONF_LATENCYAUTOTUNE
                    if(hFimDev->fWriteAutoTune)
                    {
                        hFimDev->ulWriteTimeMS += DclTimePassed(t);
                        hFimDev->ulWriteCount += ioStat.ulCount;
                        if(hFimDev->ulWriteTimeMS && hFimDev->ulWriteCount >= AUTOTUNE_WRITE_THRESHOLD)
                        {
                            hFimDev->ulMaxWriteCount = FfxDevCalcMaxWriteCount(hFimDev,
                                (hFimDev->ulWriteTimeMS*1000) / hFimDev->ulWriteCount);
                            hFimDev->fWriteAutoTune = FALSE;
                        }
                    }
                  #endif

                    READBACKVERIFY(hFimDev, pReq, pReq->nAltDataSize, ioStat);

                    break;
                }

                case FXIOSUBFUNC_DEV_WRITE_SPARES:
                case FXIOSUBFUNC_DEV_WRITE_RAWPAGES:
                case FXIOSUBFUNC_DEV_WRITE_NATIVEPAGES:
                case FXIOSUBFUNC_DEV_WRITE_UNCORRECTEDPAGES:
                {
                    if(hFimDev->pFimInfo->uSpareSize == 0)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(Write C) invalid spare size\n"));
                        ioStat.ffxStat = FFXSTAT_INVALIDSTATE;
                        ioStat.ulFlags = IOFLAGS_PAGE;
                    }

                    ioStat = GenericWrite(hFimDev, pReq, hFimDev->pFimInfo->uSpareSize);

                    READBACKVERIFY(hFimDev, pReq, hFimDev->pFimInfo->uSpareSize, ioStat);
                    if (ioStat.ffxStat == FFXSTAT_DEVMGR_READBACKVERIFYFAILED)
                    {
                        D_UINT32    ulPageOffset;
                        D_UINT32    ulBlockNum;

                        ulBlockNum = (pReq->ulStartPage + ioStat.ulCount) / hFimDev->pFimInfo->uPagesPerBlock;
                        ulPageOffset = (pReq->ulStartPage + ioStat.ulCount) % hFimDev->pFimInfo->uPagesPerBlock;
                        FfxErrMgrReportError(hFimDev,
                                             hFimDev->errMgrInstance,
                                             &ioStat,
                                             ulBlockNum,
                                             ulPageOffset);
                    }

                    break;
                }

                case FXIOSUBFUNC_DEV_WRITE_TAGS:
                {
                    if(pReq->nAltDataSize == 0)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(Write D) invalid nAltDataSize\n"));
                    }

                    if(pReq->nAltDataSize > hFimDev->pFimInfo->uMetaSize)
                    {
                        FFXPRINTF(1, ("FfxDevIORequest(Write D) invalid nAltDataSize\n"));
                    }

                    ioStat = GenericWrite(hFimDev, pReq, pReq->nAltDataSize);

                    READBACKVERIFY(hFimDev, pReq, pReq->nAltDataSize, ioStat);
                    if (ioStat.ffxStat == FFXSTAT_DEVMGR_READBACKVERIFYFAILED)
                    {
                        D_UINT32    ulPageOffset;
                        D_UINT32    ulBlockNum;

                        ulBlockNum = (pReq->ulStartPage + ioStat.ulCount) / hFimDev->pFimInfo->uPagesPerBlock;
                        ulPageOffset = (pReq->ulStartPage + ioStat.ulCount) % hFimDev->pFimInfo->uPagesPerBlock;
                        FfxErrMgrReportError(hFimDev,
                                             hFimDev->errMgrInstance,
                                             &ioStat,
                                             ulBlockNum,
                                             ulPageOffset);
                    }

                    break;
                }
              #endif
                default:
                {
                    FFXPRINTF(1, ("FfxDevIORequest() Bad Write Generic SubFunction %x\n", pReq->ioSubFunc));
                    break;
                }
            }

            break;
        }

      #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
        case FXIOFUNC_DEV_READ_CONTROLDATA:
        {
            ioStat = FfxDevReadControlData(hFimDev, (FFXIOR_DEV_READ_CONTROLDATA*)pIOR);
            break;
        }

        case FXIOFUNC_DEV_WRITE_CONTROLDATA:
        {
            ioStat = FfxDevWriteControlData(hFimDev, (FFXIOR_DEV_WRITE_CONTROLDATA*)pIOR);
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT
      #if FFXCONF_BBMSUPPORT
        case FXIOFUNC_DEV_GET_BLOCK_INFO:
        {
            ioStat = FfxDevNandGetBlockInfo(hFimDev, (FFXIOR_DEV_GET_BLOCK_INFO*)pIOR);
            break;
        }
        case FXIOFUNC_DEV_GET_RAW_BLOCK_INFO:
        {
            ioStat = FfxDevNandGetRawBlockInfo(hFimDev, (FFXIOR_DEV_GET_RAW_BLOCK_INFO*)pIOR);
            break;
        }
        case FXIOFUNC_DEV_RETIRE_RAW_BLOCK:
        {
            ioStat = FfxDevNandRetireRawBlock(hFimDev, (FFXIOR_DEV_RETIRE_RAW_BLOCK *) pIOR);
            break;
        }
      #endif

        case FXIOFUNC_DEV_GET_BLOCK_STATUS:
        {
            ioStat = FfxDevNandGetBlockStatus(hFimDev, (FFXIOR_DEV_GET_BLOCK_STATUS*)pIOR);
            break;
        }

        case FXIOFUNC_DEV_GET_RAW_BLOCK_STATUS:
        {
            ioStat = FfxDevNandGetRawBlockStatus(hFimDev, (FFXIOR_DEV_GET_RAW_BLOCK_STATUS*)pIOR);
            break;
        }

        case FXIOFUNC_DEV_SET_RAW_BLOCK_STATUS:
        {
            ioStat = FfxDevNandSetRawBlockStatus(hFimDev, (FFXIOR_DEV_SET_RAW_BLOCK_STATUS*)pIOR);
            break;
        }

        case FXIOFUNC_DEV_GET_RAW_PAGE_STATUS:
        {
            ioStat = FfxDevNandGetRawPageStatus(hFimDev, (FFXIOR_DEV_GET_RAW_PAGE_STATUS*)pIOR);
            break;
        }
      #endif

        default:
        {
            FFXPRINTF(1, ("FfxDevIORequest() Invalid function %x\n", pIOR->ioFunc));
            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 2, TRACEUNDENT),
        "FfxDevIORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}



                    /*----------------------------*\
                     *                            *
                     *  Public Helper Functions   *
                     *                            *
                    \*----------------------------*/


/*-------------------------------------------------------------------
    Private: FfxDevIOProlog()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDevIOProlog(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT16        uType)
{
    if(!DclMutexAcquire(hFimDev->pMutex))
        return DCLSTAT_MUTEXACQUIREFAILED;

    /*  If an erase is in progress, suspend it now.
    */
    if(hFimDev->nEraseState == ERASESTATE_ERASING)
    {
        DclAssert(hFimDev->nEraseSuspend == 0);
        if(EraseSuspend(hFimDev))
        {
            hFimDev->nEraseState = ERASESTATE_SUSPENDED;
        }
        else
        {
            /*  Something has gone Horribly Wrong if an erase can't be
                suspended.  Resume it (even though this probably won't
                work right either) and let the erasing thread sort things
                out.  This operation must fail.
            */
            FFXPRINTF(1, ("EraseSuspend() failed!\n"));
            EraseResume(hFimDev);
            DclMutexRelease(hFimDev->pMutex);
            return FFXSTAT_DEVMGR_ERASESUSPENDFAILED;
         }
    }

    /*  Increment the "erase suspend" count.  This is the number of
        threads doing something that requires an erase to be suspended
        or held off.
    */
    hFimDev->nEraseSuspend++;

    return FfxHookAccessBegin(hFimDev->hDev, uType);
}


/*-------------------------------------------------------------------
    Private: FfxDevIOEpilog()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDevIOEpilog(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT16        uType)
{
    FFXSTATUS       ffxStat;

    /*  ToDo: We're in really bad shape if "AccessEnd" fails (which
        it should not), but it should be handled gracefully...
    */
    ffxStat = FfxHookAccessEnd(hFimDev->hDev, uType);
    DclAssert(ffxStat == FFXSTAT_SUCCESS);

    /*  Resume erase in progress if necessary (because it is currently
        suspended) and appropriate (there's nothing more to read, and
        no other readers).
    */
    hFimDev->nEraseSuspend--;

    if(hFimDev->nEraseSuspend == 0 && hFimDev->nEraseState == ERASESTATE_SUSPENDED)
    {
        EraseResume(hFimDev);
        hFimDev->nEraseState = ERASESTATE_ERASING;
    }

    /*  End of exclusive access.  Permit higher-priority threads to
        preempt.  Assume releasing the mutex will work if acquiring
        it worked.
    */
    DclMutexRelease(hFimDev->pMutex);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevEraseProlog()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDevEraseProlog(
    FFXFIMDEVHANDLE hFimDev)
{
    DclAssert(hFimDev);

    if(!DclMutexAcquire(hFimDev->pMutex))
        return DCLSTAT_MUTEXACQUIREFAILED;

    /*  Wait until no other threads are accessing the flash before
        starting an erase operation.
    */
    while(hFimDev->nEraseSuspend || hFimDev->nEraseState != ERASESTATE_IDLE)
    {
        DclMutexRelease(hFimDev->pMutex);
        DclOsSleep(1);
        DclMutexAcquire(hFimDev->pMutex);
    }

    hFimDev->nEraseState = ERASESTATE_ERASING;

    return FfxHookAccessBegin(hFimDev->hDev, ACCESS_ERASES);
}


/*-------------------------------------------------------------------
    Private: FfxDevEraseEpilog()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXSTATUS FfxDevEraseEpilog(
    FFXFIMDEVHANDLE hFimDev)
{
    FFXSTATUS   ffxStat;

    DclAssert(hFimDev);
    DclAssert(hFimDev->nEraseState == ERASESTATE_ERASING);

    /*  ToDo: We're in really bad shape if "AccessEnd" fails (which
        it should not), but it should be handled gracefully...
    */
    ffxStat = FfxHookAccessEnd(hFimDev->hDev, ACCESS_ERASES);

    hFimDev->nEraseState = ERASESTATE_IDLE;

    DclMutexRelease(hFimDev->pMutex);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Private: FfxDevErasePoll()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS FfxDevErasePoll(
    FFXFIMDEVHANDLE hFimDev)
{
    FFXIOSTATUS     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIM_NOTERASING);

    DclAssert(hFimDev);
    DclAssert(hFimDev->nEraseState == ERASESTATE_ERASING);

    /*  Wait for it to be done.  Poll for completion if it's actually
        erasing, or simply wait a little if another thread has suspended
        the erase.  The FIM is responsible for timing out the operation
        (only the FIM knows how long the timeout should be).
    */
    while(hFimDev->nEraseState != ERASESTATE_IDLE)
    {
        if(hFimDev->nEraseState == ERASESTATE_ERASING)
        {
            ioStat = ErasePoll(hFimDev);
            if(ioStat.ffxStat != FFXSTAT_FIM_ERASEINPROGRESS)
                break;
        }

      #if FFXCONF_LATENCYREDUCTIONENABLED
        /*  Release and acquire the mutex, allowing higher-priority
            accesses to preempt.
        */
        if(hFimDev->fMutexCycle)
            DclMutexRelease(hFimDev->pMutex);

        /*  Sleep for a short while if we are configured to do so
        */
        if(hFimDev->uErasePollInterval)
            DclOsSleep(hFimDev->uErasePollInterval);

        if(hFimDev->fMutexCycle)
            DclMutexAcquire(hFimDev->pMutex);
      #endif
    }

    DclAssert(hFimDev->nEraseState == ERASESTATE_ERASING);

    return ioStat;
}

#if FFX_USE_EVENT_MANAGER

/*-------------------------------------------------------------------
    Private: FfxDevGetEventMgrInstance()

    Gets the event manager instance from a FimDev.

    Parameters:
        hFimDev         - Handle to the FIM device

    Return Value:
        Pointer to an event manager instance, which can be used
        for event reporting.
-------------------------------------------------------------------*/
EVENTMGRINSTANCE *FfxDevGetEventMgrInstance( FFXFIMDEVHANDLE hFimDev)
{
    if(hFimDev != NULL)
        return (hFimDev->eventMgrInstance);
    
    return NULL;
}

/*-------------------------------------------------------------------
    Private: FfxDevReportRead()

        If FFX_USE_EVENT_MANAGER is turned on, this function allows
        parts of the Device Manager to report a read operation to
        the event manager. This is a null function if the above-
        mentioned #define is turned off.

    Parameters:
        hFimDev         - Handle to the FIM device
        ulStartPage     - First page in the range that was read
        ulPageCount     - Number of pages read.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevReportRead(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulStartPage,
    D_UINT32 ulPageCount)
{
DECLARE_EVENT_OBJECT(event, FFX_EVENT_REPORT_READ);
READ_METADATA *metadata;
D_UINT32 ulBlockNum, ulPageOffset, index;

    metadata = &(event.event_metadata.read_metadata);
    for (index = 0; index < ulPageCount; index++)
    {
        ulBlockNum = (ulStartPage + index)/
            hFimDev->pFimInfo->uPagesPerBlock;
        ulPageOffset = (ulStartPage + index)%
            hFimDev->pFimInfo->uPagesPerBlock;
        metadata->ulBlockNum = ulBlockNum;
        metadata->ulPageOffset = ulPageOffset;
        FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
    }
}

/*-------------------------------------------------------------------
    Private: FfxDevReportWrite()

        If FFX_USE_EVENT_MANAGER is turned on, this function allows
        parts of the Device Manager to report a write operation to
        the event manager. This is a null function if the above-
        mentioned #define is turned off.

    Parameters:
        hFimDev         - Handle to the FIM device
        ulStartPage     - First page in the range that was written
        ulPageCount     - Number of pages written.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevReportWrite(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulStartPage,
    D_UINT32 ulPageCount)
{
DECLARE_EVENT_OBJECT(event, FFX_EVENT_REPORT_WRITE);
WRITE_METADATA *metadata;
D_UINT32 ulBlockNum, ulPageOffset, index;

    metadata = &(event.event_metadata.write_metadata);
    for (index = 0; index < ulPageCount; index++)
    {
        ulBlockNum = (ulStartPage + index)/
            hFimDev->pFimInfo->uPagesPerBlock;
        ulPageOffset = (ulStartPage + index)%
            hFimDev->pFimInfo->uPagesPerBlock;
        metadata->ulBlockNum = ulBlockNum;
        metadata->ulPageOffset = ulPageOffset;
        FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
    }
}

/*-------------------------------------------------------------------
    Private: FfxDevReportErase()

        If FFX_USE_EVENT_MANAGER is turned on, this function allows
        parts of the Device Manager to report an erase operation to
        the event manager. This is a null function if the above-
        mentioned #define is turned off.

    Parameters:
        hFimDev         - Handle to the FIM device
        ulBlockNum      - Block that was erased

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevReportErase(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulBlockNum)
{
DECLARE_EVENT_OBJECT(event, FFX_EVENT_REPORT_ERASE);
ERASE_METADATA *metadata;

    metadata = &(event.event_metadata.erase_metadata);
    metadata->ulBlockNum = ulBlockNum;
    FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
}

/*-------------------------------------------------------------------
    Private: FfxDevReportSuspend()

        If FFX_USE_EVENT_MANAGER is turned on, this function allows
        parts of the Device Manager to report erase-suspend operation
        to the event manager. This is a null function if the above-
        mentioned #define is turned off.

    Parameters:
        hFimDev         - Handle to the FIM device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevReportSuspend(FFXFIMDEVHANDLE hFimDev)
{
DECLARE_EVENT_OBJECT(event, FFX_EVENT_REPORT_SUSPEND);
SUSPEND_METADATA *metadata;

    metadata = &(event.event_metadata.suspend_metadata);
    metadata->ulBlockNum = 0;
    FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
}

/*-------------------------------------------------------------------
    Private: FfxDevReportResume()

        If FFX_USE_EVENT_MANAGER is turned on, this function allows
        parts of the Device Manager to report erase-resume operation
        to the event manager. This is a null function if the above-
        mentioned #define is turned off.

    Parameters:
        hFimDev         - Handle to the FIM device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxDevReportResume(FFXFIMDEVHANDLE hFimDev)
{
DECLARE_EVENT_OBJECT(event, FFX_EVENT_REPORT_RESUME);
RESUME_METADATA *metadata;

    metadata = &(event.event_metadata.resume_metadata);
    metadata->ulBlockNum = 0;
    FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
}

#else /* #if FFX_USE_EVENT_MANAGER */

/*  If event manager is not being used, make these null functions:
*/
void FfxDevReportRead(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulStartPage,
    D_UINT32 ulPageCount)
{
    (void)hFimDev;
    (void)ulStartPage;
    (void)ulPageCount;
}

void FfxDevReportWrite(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulStartPage,
    D_UINT32 ulPageCount)
{
    (void)hFimDev;
    (void)ulStartPage;
    (void)ulPageCount;
}

void FfxDevReportErase(
    FFXFIMDEVHANDLE hFimDev,
    D_UINT32 ulBlockNum)
{
    (void)hFimDev;
    (void)ulBlockNum;
}

void FfxDevReportSuspend(FFXFIMDEVHANDLE hFimDev)
{
    (void)hFimDev;
}

void FfxDevReportResume(FFXFIMDEVHANDLE hFimDev)
{
    (void)hFimDev;
}

#endif /* #if FFX_USE_EVENT_MANAGER */




                    /*----------------------------*\
                     *                            *
                     *   Local Helper Functions   *
                     *                            *
                    \*----------------------------*/




/*-------------------------------------------------------------------
    Local: GenericRead()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS GenericRead(
    FFXFIMDEVHANDLE             hFimDev,
    FFXIOR_DEV_READ_GENERIC    *pDevReq,
    unsigned                    nAltDataLen)
{
    FFXIOR_FIM_READ_GENERIC     FimReq;
    FFXIOSTATUS                 ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);
    D_UINT32                    ulRemaining;
    D_UINT32                    ulMaxPageCount;
    D_UINT32                    ulReservedPages;
  #if FFXCONF_FORCEALIGNEDIO
    D_BOOL                      fNeedsAligning = FALSE;
    D_UINT32                    ulCount = 0;
    D_BUFFER                   *pOrigPageData = NULL;
  #endif

    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDev:GenericRead() hFimDev=%P IOSubFunc=%x StartPage=%lX Count=%lX pPage=%P pAlt=%P AltLen=%u\n",
        hFimDev, pDevReq->ioSubFunc, pDevReq->ulStartPage, pDevReq->ulCount, pDevReq->pPageData, pDevReq->pAltData, nAltDataLen));

    DclProfilerEnter("FfxDev:GenericRead", 0, pDevReq->ulCount);

    DclAssert(hFimDev);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        DclError();
        goto ReadCleanup;
    }

    DclAssert(pDevReq->ior.ulReqLen == sizeof(*pDevReq));
    DclAssert(pDevReq->pPageData || pDevReq->pAltData);
    DclAssert(hFimDev->pFimInfo->uPageSize);

    ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_READS);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto ReadCleanup;

  #if FFXCONF_NANDSUPPORT
    if(pDevReq->ioSubFunc == FXIOSUBFUNC_DEV_READ_RAWPAGES)
    {
        if(pDevReq->ulStartPage >= hFimDev->ulRawTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericRead() Page range error, request starts at %lU\n",
                  pDevReq->ulStartPage ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto ReadEpilog;
        }

        if(pDevReq->ulStartPage + pDevReq->ulCount > hFimDev->ulRawTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericRead() Page range error, request count is %lU\n",
                      pDevReq->ulCount ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto ReadEpilog;
        }
    }
    else
  #endif
    {
        if(pDevReq->ulStartPage >= hFimDev->ulTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericRead() Page range error, request starts at %lU\n",
                  pDevReq->ulStartPage ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto ReadEpilog;

        }

        if (pDevReq->ulCount > hFimDev->ulTotalPages - pDevReq->ulStartPage)
        {
            FFXPRINTF(1, ("FfxDev:GenericRead() Page range error, request count is %lU\n",
                  pDevReq->ulCount ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto ReadEpilog;
        }
    }

    /*  Get the default max pages we are allowed to read at once.  However,
        if we've just suspended an erase to do this read, forget about latency
        reduction for reads, and do this as fast as we can using large read
        operations.  In addition to being more optimal, this avoid potential
        corruption of the erase state information (Bug 2607).
    */
    if(hFimDev->nEraseState == ERASESTATE_SUSPENDED)
        ulMaxPageCount = pDevReq->ulCount;
    else
        ulMaxPageCount = hFimDev->ulMaxReadCount;

    if(!pDevReq->pPageData)
    {
        /*  If we are reading something other than full pages, such as
            spares or tags only, increase the read size proportionately.
        */
        if((nAltDataLen == 0) || (nAltDataLen > hFimDev->pFimInfo->uPageSize))
        {
            FFXPRINTF(1, ("FfxDev:GenericRead() invalid nAltDataLen (%u)\n",nAltDataLen));
            ioStat.ffxStat = FFXSTAT_INVALIDSTATE;
            goto ReadEpilog;
        }

        if(ulMaxPageCount != D_UINT32_MAX)
            ulMaxPageCount *= (hFimDev->pFimInfo->uPageSize / nAltDataLen);
    }

  #if FFXCONF_FORCEALIGNEDIO
    /*  Note that we only force alignment if we are doing page oriented I/O,
        in which case pPageData will be non-NULL.  For other things such as
        reading spare areas and tags, these buffer pointers MUST already
        be aligned properly, and they should be since it is primarily only
        FlashFX internal code which uses those features.
    */
    DclAssert(hFimDev->pFimInfo->uAlignSize);

    if(hFimDev->pAlignedIOBuffer && pDevReq->pPageData &&
       !DCLISALIGNED((D_UINTPTR)pDevReq->pPageData, hFimDev->pFimInfo->uAlignSize))
    {
        /*  If we have to force things to be aligned, use our already
            in place logic which enforces "reasonable" length operations
            to enforce reading only one page at a time.
        */
        fNeedsAligning = TRUE;
        ulMaxPageCount = 1;

        /*  Save the original page data buffer and stuff the pointer to our
            aligned buffer into the request structure.  We do this so we do not
            need to do any special handling for the alignment checks below.
        */
        pOrigPageData = pDevReq->pPageData;
        pDevReq->pPageData = hFimDev->pAlignedIOBuffer;

        FFXPRINTF(1, ("FfxDev:GenericRead() Double-buffering a %U page request due to a misaligned pointer (%P)\n",
            pDevReq->ulCount, pOrigPageData));
    }
  #endif

    /*  Make sure the data pointers are aligned appropriately.
    */
    if(pDevReq->pPageData && !DCLISALIGNED((D_UINTPTR)pDevReq->pPageData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDev:GenericRead() pPageData is misaligned (%lX) or missing\n", pDevReq->pPageData));
        ioStat.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto ReadEpilog;
    }

    if(pDevReq->pAltData && !DCLISALIGNED((D_UINTPTR)pDevReq->pAltData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDev:GenericRead() pAltData is misaligned (%lX)\n", pDevReq->pAltData));
        ioStat.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto ReadEpilog;
    }

    ulRemaining = pDevReq->ulCount;

    /*  For the moment these structures are identical
    */
    FimReq                  = *pDevReq;
    FimReq.ior.ioFunc       = FXIOFUNC_FIM_READ_GENERIC;
    FimReq.ioSubFunc        = FFX_TRANSFORM_DEV2FIM(pDevReq->ioSubFunc);

    /*  Account for reserved space in the flash
    */
    ulReservedPages = hFimDev->ulReservedBlocks * hFimDev->pFimInfo->uPagesPerBlock;
    FimReq.ulStartPage += ulReservedPages;

    while(ulRemaining)
    {
        FFXIOSTATUS ioTemp;
        D_UINT32    ulPageOffset;
        D_UINT32    ulBlockNum;

        /*  Break transfers into chunks, based on the project latency settings,
            allowing other FIM accesses to preempt this one between them.
        */
        FimReq.ulCount = DCLMIN(ulRemaining, ulMaxPageCount);

      #if FFXCONF_BBMSUPPORT
        /*  Check the BBM for any mapping.  This may change the size and
            location of the request if the BBM has moved the block where this
            page is located at.
        */
        if (hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND
            && pDevReq->ioSubFunc != FXIOSUBFUNC_DEV_READ_RAWPAGES)
        {
            FimReq.ulStartPage = pDevReq->ulStartPage
                + ulReservedPages
                + pDevReq->ulCount
                - ulRemaining;

            FfxDevBbmRemapPage(hFimDev, &FimReq.ulStartPage, &FimReq.ulCount);
        }
      #endif

        ioTemp = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

        ioStat.ulCount         += ioTemp.ulCount;
        ioStat.ffxStat          = ioTemp.ffxStat;
        ioStat.op.ulPageStatus |= ioTemp.op.ulPageStatus;

        /*  Report reads for this chunk
        */
        FfxDevReportRead(hFimDev, FimReq.ulStartPage, ioTemp.ulCount);

      #if FFXCONF_FORCEALIGNEDIO
        if (fNeedsAligning && ioTemp.ulCount)
        {
            DclAssert(ulMaxPageCount == 1);
            DclAssert(ioTemp.ulCount == 1);
            DclAssert(pOrigPageData);
 
            /*  Copy the data from our aligned buffer into the original page
                data buffer.
            */
            DclMemCpy(pOrigPageData + (ulCount * hFimDev->pFimInfo->uPageSize),
                      hFimDev->pAlignedIOBuffer, hFimDev->pFimInfo->uPageSize);
 
            /*  Keep our own private count so we know how far to adjust
                the destination pointer on the next iteration.
            */
            ulCount++;
        }
      #endif
 
        /*  Check to see if there were any errors, and report them
            if there were.
        */
        if (ioTemp.ffxStat != FFXSTAT_SUCCESS)
        {
            ulBlockNum = (FimReq.ulStartPage + ioTemp.ulCount) /
                hFimDev->pFimInfo->uPagesPerBlock;
            ulPageOffset = (FimReq.ulStartPage + ioTemp.ulCount) %
                hFimDev->pFimInfo->uPagesPerBlock;
            FfxErrMgrReportError(hFimDev,
                                 hFimDev->errMgrInstance,
                                 &ioStat,
                                 ulBlockNum,
                                 ulPageOffset);
            break;
        }

        /*  Reduce the count, and advance the start page and buffer
            pointers as appropriate.
        */
        ulRemaining             -= ioTemp.ulCount;
        if(!ulRemaining)
            break;

        FimReq.ulStartPage      += ioTemp.ulCount;
      #if FFXCONF_FORCEALIGNEDIO
        if(FimReq.pPageData && !fNeedsAligning)
            FimReq.pPageData    += ioTemp.ulCount * hFimDev->pFimInfo->uPageSize;
      #else
        if(FimReq.pPageData)
            FimReq.pPageData    += ioTemp.ulCount * hFimDev->pFimInfo->uPageSize;
      #endif
        if(FimReq.pAltData)
            FimReq.pAltData     += ioTemp.ulCount * nAltDataLen;

      /*    Don't cycle the mutex if we have suspended an erase to accommodate
            this read.  This prevents possible loss of internal state data for
            an interrupted multi-block erase operation (see Bug 2607).
      */
      #if FFXCONF_LATENCYREDUCTIONENABLED
        if(hFimDev->fMutexCycle && hFimDev->nEraseState != ERASESTATE_SUSPENDED)
        {
            /*  Release and acquire the mutex, allowing higher-priority
                accesses to preempt.
            */
            DclMutexRelease(hFimDev->pMutex);
            DclMutexAcquire(hFimDev->pMutex);
        }
      #endif
    }

  ReadEpilog:

  #if FFXCONF_FORCEALIGNEDIO
    if(fNeedsAligning)
    {
        /*  Put the original page data buffer back into the request structure.
        */
        pDevReq->pPageData = pOrigPageData;
    }
  #endif

    FfxDevIOEpilog(hFimDev, ACCESS_READS);

  ReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDev:GenericRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: GenericWrite()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS GenericWrite(
    FFXFIMDEVHANDLE             hFimDev,
    FFXIOR_DEV_WRITE_GENERIC   *pDevReq,
    unsigned                    nAltDataLen)
{
    FFXIOR_FIM_WRITE_GENERIC    FimReq;
    FFXIOSTATUS                 ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);
    D_UINT32                    ulRemaining;
    D_UINT32                    ulMaxPageCount;
    D_UINT32                    ulReservedPages;
  #if FFXCONF_FORCEALIGNEDIO
    D_BOOL                      fNeedsAligning = FALSE;
    D_UINT32                    ulCount = 0;
    const D_BUFFER             *pOrigPageData = NULL;
  #endif

    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDev:GenericWrite() hFimDev=%P IOSubFunc=%x StartPage=%lX Count=%lX pPage=%P pAlt=%P AltLen=%u\n",
        hFimDev, pDevReq->ioSubFunc, pDevReq->ulStartPage, pDevReq->ulCount, pDevReq->pPageData, pDevReq->pAltData, nAltDataLen));

    DclProfilerEnter("FfxDev:GenericWrite", 0, pDevReq->ulCount);

    DclAssert(hFimDev);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        FFXPRINTF(1, ("FfxDev:GenericWrite() hFimReq length error\n"));
        goto WriteCleanup;
    }

    if((pDevReq->pPageData == NULL) && (pDevReq->pAltData == NULL) )
    {
        FFXPRINTF(1, ("FfxDev:GenericWrite() both request data pointers are invalid\n"));
        ioStat.ffxStat = FFXSTAT_BADPARAMETER;
        goto WriteCleanup;
    }

    ioStat.ffxStat = FfxDevIOProlog(hFimDev, ACCESS_WRITES);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto WriteCleanup;

    DclAssert(hFimDev->pFimInfo->uPageSize);
  #if FFXCONF_NANDSUPPORT
    if(pDevReq->ioSubFunc == FXIOSUBFUNC_DEV_WRITE_RAWPAGES)
    {
        if(pDevReq->ulStartPage >= hFimDev->ulRawTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericWrite()Page range error, request starts at %lu\n",
                  pDevReq->ulStartPage ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto WriteEpilog;
        }

        if (pDevReq->ulStartPage + pDevReq->ulCount > hFimDev->ulRawTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericWrite() Page range error, request count is %lu\n",
                  pDevReq->ulCount ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto WriteEpilog;

        }
    }
    else
  #endif
    {
        if(pDevReq->ulStartPage >= hFimDev->ulTotalPages)
        {
            FFXPRINTF(1, ("FfxDev:GenericWrite() Page range error, request starts at %lu\n",
                  pDevReq->ulStartPage ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto WriteEpilog;
        }
        if (pDevReq->ulCount > hFimDev->ulTotalPages - pDevReq->ulStartPage)
        {
            FFXPRINTF(1, ("FfxDev:GenericWrite() Page range error, request count is %lu\n",
                  pDevReq->ulCount ));
            ioStat.ffxStat = FFXSTAT_OUTOFRANGE;
            goto WriteEpilog;
        }
    }

    /*  Get the default max pages we are allowed to write at once.  However,
        if we've just suspended an erase to do this write, forget about latency
        reduction for writes, and do this as fast as we can using large write
        operations.  In addition to being more optimal, this avoid potential
        corruption of the erase state information (Bug 2607).
    */
    if(hFimDev->nEraseState == ERASESTATE_SUSPENDED)
        ulMaxPageCount = pDevReq->ulCount;
    else
        ulMaxPageCount = hFimDev->ulMaxWriteCount;

    if(!pDevReq->pPageData)
    {
        /*  If we are writing something other than full pages, such as
            spares or tags only, increase the write size proportionately.
        */
        if((nAltDataLen == 0) || (nAltDataLen > hFimDev->pFimInfo->uPageSize))
        {
            FFXPRINTF(1, ("FfxDev:GenericWrite() invalid nAltDataLen (%u)\n",nAltDataLen));
            ioStat.ffxStat = FFXSTAT_INVALIDSTATE;
            goto WriteEpilog;
        }

        if(ulMaxPageCount != D_UINT32_MAX)
            ulMaxPageCount *= (hFimDev->pFimInfo->uPageSize / nAltDataLen);
    }

  #if FFXCONF_FORCEALIGNEDIO
    /*  Note that we only force alignment if we are doing page oriented I/O,
        in which case pPageData will be non-NULL.  For other things such as
        writing spare areas and tags, these buffer pointers MUST already
        be aligned properly, and they should be since it is primarily only
        FlashFX internal code which uses those features.
    */
    DclAssert(hFimDev->pFimInfo->uAlignSize);

    if(hFimDev->pAlignedIOBuffer && pDevReq->pPageData &&
       !DCLISALIGNED((D_UINTPTR)pDevReq->pPageData, hFimDev->pFimInfo->uAlignSize))
    {
        /*  If we have to force things to be aligned, use our already
            in place logic which enforces "reasonable" length operations
            to enforce writing only one page at a time.
        */
        fNeedsAligning = TRUE;
        ulMaxPageCount = 1;

        /*  Save the original page data buffer and stuff the pointer to our
            aligned buffer into the request structure.  We do this so we do not
            need to do any special handling for the alignment checks below.
        */
        pOrigPageData = pDevReq->pPageData;
        pDevReq->pPageData = hFimDev->pAlignedIOBuffer;

        FFXPRINTF(1, ("FfxDev:GenericWrite() Double-buffering a %U page request due to a misaligned pointer (%P)\n",
            pDevReq->ulCount, pOrigPageData));
    }
  #endif

    /*  Make sure everything is aligned appropriately.
    */
    if(pDevReq->pPageData && !DCLISALIGNED((D_UINTPTR)pDevReq->pPageData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDev:GenericWrite() pPageData is misaligned (%lX) or missing\n", pDevReq->pPageData));
        ioStat.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto WriteEpilog;
    }

    if(pDevReq->pAltData && !DCLISALIGNED((D_UINTPTR)pDevReq->pAltData, hFimDev->pFimInfo->uAlignSize))
    {
        FFXPRINTF(1, ("FfxDev:GenericWrite() pAltData is misaligned (%lX)\n", pDevReq->pAltData));
        ioStat.ffxStat = FFXSTAT_DEVMGR_MISALIGNED;
        goto WriteEpilog;
    }

    ulRemaining = pDevReq->ulCount;

    /*  For the moment these structures are identical
    */
    FimReq                  = *pDevReq;
    FimReq.ior.ioFunc       = FXIOFUNC_FIM_WRITE_GENERIC;
    FimReq.ioSubFunc        = FFX_TRANSFORM_DEV2FIM(pDevReq->ioSubFunc);

    /*  Account for reserved space in the flash
    */
    ulReservedPages = hFimDev->ulReservedBlocks * hFimDev->pFimInfo->uPagesPerBlock;
    FimReq.ulStartPage += ulReservedPages;

    while(ulRemaining)
    {
        FFXIOSTATUS     ioTemp;

      #if FFXCONF_FORCEALIGNEDIO
        if(fNeedsAligning)
        {
            DclAssert(ulMaxPageCount == 1);
            DclAssert(pOrigPageData);

            /*  Copy each page into an aligned buffer allocated for this device.
            */
            DclMemCpy(hFimDev->pAlignedIOBuffer,
                      pOrigPageData + (ulCount * hFimDev->pFimInfo->uPageSize),
                      hFimDev->pFimInfo->uPageSize);

            /*  Keep our own private count so we know how far to adjust
                the source pointer on the next iteration.
            */
            ulCount++;
        }
      #endif

        /*  Break transfers into chunks, based on the project latency settings,
            allowing other FIM accesses to preempt this one between them.
        */
        FimReq.ulCount = DCLMIN(ulRemaining, ulMaxPageCount);

      #if FFXCONF_BBMSUPPORT
        /*  Check the BBM for any mapping.  This may change the size and
            location of the request if the BBM has moved the block where this
            page is located at.
        */
        if (hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND
            && pDevReq->ioSubFunc != FXIOSUBFUNC_DEV_WRITE_RAWPAGES)
        {
            FimReq.ulStartPage = pDevReq->ulStartPage
                    + ulReservedPages
                    + pDevReq->ulCount
                    - ulRemaining;

            FfxDevBbmRemapPage(hFimDev, &FimReq.ulStartPage, &FimReq.ulCount);
        }
      #endif

        ioTemp = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &FimReq.ior);

        ioStat.ulCount          += ioTemp.ulCount;
        ioStat.ffxStat           = ioTemp.ffxStat;
        ioStat.op.ulPageStatus  |= ioTemp.op.ulPageStatus;

        /* Report writes for this chunk
        */
        FfxDevReportWrite(hFimDev, FimReq.ulStartPage, ioTemp.ulCount);

        if (ioTemp.ffxStat != FFXSTAT_SUCCESS)
        {
            const D_UINT32 ulBlockNum = ((FimReq.ulStartPage + ioTemp.ulCount)
                                         / hFimDev->pFimInfo->uPagesPerBlock);
            const D_UINT32 ulPageOffset = ((FimReq.ulStartPage + ioTemp.ulCount)
                                           % hFimDev->pFimInfo->uPagesPerBlock);
            FfxErrMgrReportError(hFimDev,
                                 hFimDev->errMgrInstance,
                                 &ioTemp,
                                 ulBlockNum,
                                 ulPageOffset);

          #if FFXCONF_BBMSUPPORT
            /*  Raw page writes don't get block replacement.
            */
            if (pDevReq->ioSubFunc == FXIOSUBFUNC_DEV_WRITE_RAWPAGES)
                break;

            if (hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND
                && FfxErrMgrDecodeRecommendation(&ioTemp) == FFXERRPOLICY_RETIREBLOCK)
            {
                D_UINT32     ulFailedPage = FimReq.ulStartPage + ioTemp.ulCount;
                D_UINT32     ulBlock = ulFailedPage / hFimDev->pFimInfo->uPagesPerBlock;
                D_UINT32     ulPages = ulFailedPage % hFimDev->pFimInfo->uPagesPerBlock;
                FFXIOSTATUS  status;

                status = FfxDevBbmRelocateBlock(hFimDev, ulBlock, ulPages);

                /*  If the replacement was unsuccessful, terminate the
                    write.  Note that the status returned is of the
                    failed write, not the failed replacement.
                */
                if (status.ffxStat != FFXSTAT_SUCCESS)
                    break;

                /*  Block replacement was successful, so clear the
                    error indications.  The write that failed will be
                    retried.
                */
                FfxErrMgrKillError(&ioTemp);
                ioStat.ffxStat = FFXSTAT_SUCCESS;
            }
            else
            {
                /*  Write failed, and either not NAND, or Error
                    Manager says don't attempt to retire the block.
                */
                break;
            }

          #else

            break;

          #endif /* FFXCONF_BBMSUPPORT */
        }

        /*  Reduce the count, and advance the start page and buffer
            pointers as appropriate.
        */
        ulRemaining             -= ioTemp.ulCount;
        if(!ulRemaining)
            break;

        FimReq.ulStartPage      += ioTemp.ulCount;

      #if FFXCONF_FORCEALIGNEDIO
        if(FimReq.pPageData && !fNeedsAligning)
            FimReq.pPageData    += ioTemp.ulCount * hFimDev->pFimInfo->uPageSize;
      #else
        if(FimReq.pPageData)
            FimReq.pPageData    += ioTemp.ulCount * hFimDev->pFimInfo->uPageSize;
      #endif
        if(FimReq.pAltData)
            FimReq.pAltData     += ioTemp.ulCount * nAltDataLen;

      /*    Don't cycle the mutex if we have suspended an erase to accommodate
            this write.  This prevents possible loss of internal state data for
            an interrupted multi-block erase operation (see Bug 2607).
      */
      #if FFXCONF_LATENCYREDUCTIONENABLED
        if(hFimDev->fMutexCycle && hFimDev->nEraseState != ERASESTATE_SUSPENDED)
        {
            /*  Release and acquire the mutex, allowing higher-priority
                accesses to preempt.

                If we are operating in our "force-aligned-I/O" special case,
                we could end up doing this more often than is necessary,
                according to the configuration parameters.  But who cares,
                we're already taking a big hit if we have to force the
                buffers to be aligned.
            */
            DclMutexRelease(hFimDev->pMutex);
            DclMutexAcquire(hFimDev->pMutex);
        }
      #endif
    }

  WriteEpilog:
    
  #if FFXCONF_FORCEALIGNEDIO
    if(fNeedsAligning)
    {
        /*  Put the original page data buffer back into the request structure.
        */
        pDevReq->pPageData = pOrigPageData;
    }
  #endif

    FfxDevIOEpilog(hFimDev, ACCESS_WRITES);

  WriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDev:GenericWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: EraseBlocks()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS EraseBlocks(
    FFXFIMDEVHANDLE             hFimDev,
    FFXIOR_DEV_ERASE_BLOCKS    *pDevReq)
{
    FFXIOSTATUS                 ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
    FFXIOR_FIM_ERASE_START      ReqStart;
    D_UINT32                    ulRemaining;

    DclAssert(hFimDev);
    DclAssert(pDevReq);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEINDENT),
        "FfxDev:EraseBlocks() hFimDev=%P StartBlock=%lX Count=%lX\n",
        hFimDev, pDevReq->ulStartBlock, pDevReq->ulBlockCount));

    DclProfilerEnter("FfxDev:EraseBlocks", 0, pDevReq->ulBlockCount);

    if(pDevReq->ior.ulReqLen != sizeof *pDevReq)
    {
        FFXPRINTF(1, ("FfxDev:EraseBlocks() hFimReq length error\n"));
        goto EraseCleanup;
    }

    ioStat.ffxStat = FfxDevEraseProlog(hFimDev);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto EraseCleanup;

    ulRemaining = pDevReq->ulBlockCount;

    ReqStart.ior.ioFunc     = FXIOFUNC_FIM_ERASE_START;
    ReqStart.ior.ulReqLen   = sizeof ReqStart;
    ReqStart.ulStartBlock   = pDevReq->ulStartBlock;

    /*  Account for reserved space in the flash
    */
    ReqStart.ulStartBlock += hFimDev->ulReservedBlocks;

    while(ulRemaining)
    {
        FFXIOSTATUS ioTemp;
        D_UINT32    index;

      #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
        if(FfxDevIsInBootBlocks(hFimDev, ReqStart.ulStartBlock))
        {
            FFXIOR_FIM_ERASE_BOOT_BLOCK_START   ReqStartBB;

            ReqStartBB.ior.ioFunc   = FXIOFUNC_FIM_ERASE_BOOT_BLOCK_START;
            ReqStartBB.ior.ulReqLen = sizeof ReqStart;
            ReqStartBB.ulStartBB    = ReqStart.ulStartBlock * hFimDev->ulBBsPerBlock;
            ReqStartBB.ulBBCount    = hFimDev->ulBBsPerBlock;

            ioTemp = FfxDevEraseBootBlockSequence(hFimDev, &ReqStartBB);
            if(IOSUCCESS(ioTemp, hFimDev->ulBBsPerBlock))
            {
                /*  We erased exactly one full block's worth of boot blocks,
                    so change the count to one, and continue on.
                */
                ioTemp.ulCount = 1;
            }
            else
            {
                /*  If for any reason all of the boot blocks could not be
                    erased, from a big-block perspective, it is just as
                    if none of them have been erased.
                */
                ioTemp.ulCount = 0;
            }
        }
        else
      #endif
        {
          #if FFXCONF_LATENCYAUTOTUNE
            DCLTIMESTAMP    t = DCLINITTS;

            /*  Note that the initializer in the statement above is
                only required for some overly zealous compilers which
                warn that the variable could used without having been
                initialized.  We plainly are not doing so...
            */

            /*  If the latency setting has not been tuned yet, do so now.
                Note that we're doing this explicitly at this point to
                avoid doing this tuning on boot blocks.
            */
            if(hFimDev->fEraseAutoTune)
                t = DclTimeStamp();
          #endif

            /*  Break transfers into "reasonable length" chunks, and allow
                other FIM accesses to preempt this one between them.
            */
            ReqStart.ulCount = DCLMIN(ulRemaining, hFimDev->ulMaxEraseCount);

          #if FFXCONF_BBMSUPPORT
            /*  Check the BBM for any mapping.  This may change the size and
                location of the request if the BBM has moved the block where
                this page is located at.
            */
            if(pDevReq->ior.ioFunc != FXIOFUNC_DEV_ERASE_RAWBLOCKS)
            {
                ReqStart.ulStartBlock = pDevReq->ulStartBlock
                    + hFimDev->ulReservedBlocks
                    + pDevReq->ulBlockCount
                    - ulRemaining;

                FfxDevBbmRemapBlock(hFimDev, &ReqStart.ulStartBlock,
                    &ReqStart.ulCount);
            }
          #endif

            /*  Attempt to start an erase operation.
            */
            ioTemp = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &ReqStart.ior);
            if(ioTemp.ffxStat == FFXSTAT_SUCCESS)
            {
                ioTemp = FfxDevErasePoll(hFimDev);
            }

          #if FFXCONF_LATENCYAUTOTUNE
            if(hFimDev->fEraseAutoTune)
            {
                hFimDev->ulEraseTimeMS += DclTimePassed(t);
                hFimDev->ulEraseCount += ioStat.ulCount;
                if(hFimDev->ulEraseTimeMS &&
                    ((hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND && hFimDev->ulEraseCount >= AUTOTUNE_ERASE_NAND_THRESHOLD) ||
                    (hFimDev->pFimInfo->uDeviceType != DEVTYPE_NAND && hFimDev->ulEraseCount >= AUTOTUNE_ERASE_NOR_THRESHOLD)))
                {
                    hFimDev->ulMaxEraseCount = FfxDevCalcMaxEraseCount(hFimDev,
                        (hFimDev->ulEraseTimeMS*1000) / hFimDev->ulEraseCount);
                    hFimDev->fEraseAutoTune = FALSE;
                }
            }
          #endif
        }

        /*  Accumulate our counts for what was successfully erased.
        */
        ioStat.ulCount             += ioTemp.ulCount;
        ioStat.op.ulBlockStatus    |= ioTemp.op.ulBlockStatus;
        ioStat.ffxStat              = ioTemp.ffxStat;

        /*  Report Erases:
        */
        for (index = 0; index < ioTemp.ulCount; index++)
        {
            FfxDevReportErase(hFimDev, ReqStart.ulStartBlock + index);
        }

        if (ioTemp.ffxStat != FFXSTAT_SUCCESS)
        {
            FfxErrMgrReportError(hFimDev,
                                 hFimDev->errMgrInstance,
                                 &ioTemp,
                                 ReqStart.ulStartBlock + ioTemp.ulCount,
                                 0);

          #if FFXCONF_BBMSUPPORT
            /*  Raw blocks don't get block replacement.
            */
            if (pDevReq->ior.ioFunc == FXIOFUNC_DEV_ERASE_RAWBLOCKS)
                break;

            if (hFimDev->pFimInfo->uDeviceType == DEVTYPE_NAND
                && FfxErrMgrDecodeRecommendation(&ioTemp) == FFXERRPOLICY_RETIREBLOCK)
            {
                FFXIOSTATUS status;

                status = FfxDevBbmRelocateBlock(hFimDev,
                                                ReqStart.ulStartBlock + ioTemp.ulCount,
                                                0);

                /*  If the replacement was unsuccessful, terminate the
                    erase.  Note that the status returned is of the
                    failed erase, not the failed replacement.
                */
                if (status.ffxStat != FFXSTAT_SUCCESS)
                    break;

                /*  The block was erased in the relocation process, so
                    update the status to show this.
                */
                FfxErrMgrKillError(&ioTemp);
                ++ioTemp.ulCount;
                ioStat.ffxStat = FFXSTAT_SUCCESS;
                ++ioStat.ulCount;
            }
            else
            {
                /*  Erase failed, and either not NAND, or Error
                    Manager says don't attempt to retire the block.
                */
                break;
            }

          #else

            break;

          #endif /* FFXCONF_BBMSUPPORT */
        }

        /*  Reduce the count, and advance the start block as appropriate.
        */
        ulRemaining                 -= ioTemp.ulCount;
        if(!ulRemaining)
            break;

        /*  The operation succeeded.  The FIM reported how many blocks
            were erased.  Adjust the starting block accordingly, and go
            around again to erase any remaining.
        */
        ReqStart.ulStartBlock       += ioTemp.ulCount;

      #if FFXCONF_LATENCYREDUCTIONENABLED
        if(hFimDev->fMutexCycle)
        {
            /*  Release and acquire the mutex, allowing higher-priority
                accesses to preempt.
            */
            DclMutexRelease(hFimDev->pMutex);
            DclMutexAcquire(hFimDev->pMutex);
        }
      #endif
    }

    FfxDevEraseEpilog(hFimDev);

  EraseCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDev:EraseBlocks() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: EraseSuspend()

    This function suspends or completes an erase operation.  If
    the flash does not support erase-suspend, then this function
    will poll until the current erase operation is complete.

    Parameters:
        hFimDev - The FIM handle

    Return Value:
        Returns TRUE if the erase was successfully suspended or
        completed, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseSuspend(
    FFXFIMDEVHANDLE             hFimDev)
{
    FFXIOSTATUS                 ioStat;
  #if FFXCONF_ERASESUSPENDSUPPORT
    FFXIOR_FIM_ERASE_SUSPEND    ReqSusp;

    ReqSusp.ior.ioFunc = FXIOFUNC_FIM_ERASE_SUSPEND;
    ReqSusp.ior.ulReqLen = sizeof ReqSusp;

    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &ReqSusp.ior);
    FfxDevReportSuspend(hFimDev);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return TRUE;

    if(ioStat.ffxStat != FFXSTAT_FIM_UNSUPPORTEDIOREQUEST)
    {
        /*  If the suspend failed, the only legitimate reason is that
            suspending is not supported.  If the reason is anything
            else, assert...
        */
        FfxErrMgrReportError(hFimDev,
                             hFimDev->errMgrInstance,
                             &ioStat,
                             0,
                             0);
        DclError();
        return FALSE;
    }
  #endif

    /*  Since suspending an erase is not supported, simply poll
        until the erase is complete.
    */
    while(TRUE)
    {
        ioStat = ErasePoll(hFimDev);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS && ioStat.ulCount != 0)
            return TRUE;

        if(ioStat.ffxStat == FFXSTAT_FIM_ERASEFAILED)
        {
            /*  Report this erase failure to the error manager:
            */
            FfxErrMgrReportError(hFimDev,
                                 hFimDev->errMgrInstance,
                                 &ioStat,
                                 0,
                                 0);
            return FALSE;
        }

        DclAssert(ioStat.ffxStat == FFXSTAT_FIM_ERASEINPROGRESS);

      #if FFXCONF_LATENCYREDUCTIONENABLED
        /*  There's no sense in releasing the mutex, no other thread can
            do anything to the flash until the erase is done anyway.

            Sleep for a short while if we are so configured...
        */
        if(hFimDev->uErasePollInterval)
            DclOsSleep(hFimDev->uErasePollInterval);
      #endif
    }
}


/*-------------------------------------------------------------------
    Local: EraseResume()

    Resume an erase operation.  Note that this function may be
    called even if the flash does not support erase-suspend.

    Parameters:
        hFimDev - The FIM handle

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL EraseResume(
    FFXFIMDEVHANDLE         hFimDev)
{
  #if FFXCONF_ERASESUSPENDSUPPORT
    FFXIOSTATUS             ioStat;
    FFXIOR_FIM_ERASE_RESUME ReqResume;

    ReqResume.ior.ioFunc = FXIOFUNC_FIM_ERASE_RESUME;
    ReqResume.ior.ulReqLen = sizeof ReqResume;

    ioStat = hFimDev->pFimEntry->IORequest(hFimDev->hInst, &ReqResume.ior);
    FfxDevReportResume(hFimDev);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        /*  Only report an error if the feature is supported.
        */
        if(ioStat.ffxStat != FFXSTAT_FIM_UNSUPPORTEDIOREQUEST)
        {
            /*  Report this erase failure to the error manager:
            */
            FfxErrMgrReportError(hFimDev,
                                 hFimDev->errMgrInstance,
                                 &ioStat,
                                 0,
                                 0);
        }
        
        return FALSE;
    }
  #else
    (void)hFimDev;

    /*  If the erase-suspend feature is compiled out, then we never really
        suspended in the first place...
    */
    return FALSE;
  #endif
}


/*-------------------------------------------------------------------
    Local: ErasePoll()

    Poll for completion of an erase operation.

    Parameters:
        hFimDev - The FIM handle

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.
-------------------------------------------------------------------*/
static FFXIOSTATUS ErasePoll(
    FFXFIMDEVHANDLE         hFimDev)
{
    FFXIOR_FIM_ERASE_POLL   ReqPoll;

    ReqPoll.ior.ioFunc = FXIOFUNC_FIM_ERASE_POLL;
    ReqPoll.ior.ulReqLen = sizeof ReqPoll;

    return hFimDev->pFimEntry->IORequest(hFimDev->hInst, &ReqPoll.ior);
}



#if FFX_DEVMGR_READBACKVERIFY

/*-------------------------------------------------------------------
    Local: ReadBackVerify()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL ReadBackVerify(
    FFXFIMDEVHANDLE             hFimDev,
    FFXIOR_DEV_WRITE_GENERIC   *pDevReq,
    unsigned                    nAltDataLen,
    D_UINT32                    ulCount)
{
    DCLALIGNEDBUFFER            (buffer, data, FFX_MAX_PAGESIZE+FFX_MAX_SPARESIZE);
    D_BOOL                      fSuccess = FALSE;
    FFXIOR_DEV_READ_GENERIC     FimReq;
    FFXIOSTATUS                 ioStat;
    const D_BUFFER             *pPageBuff;
    const D_BUFFER             *pAltBuff;

    /*  Undocumented behavior!  If the readback-verify setting is TRUE*2, it
        is enabled, but quietly.
    */
  #if FFX_DEVMGR_READBACKVERIFY != (TRUE * 2)
    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEINDENT),
        "FfxDev:ReadBackVerify() StartPage=%lX Count=%lX\n", pDevReq->ulStartPage, ulCount));
  #endif

    DclAssert(hFimDev);
    DclAssert(ulCount);
    DclAssert(pDevReq);
    DclAssert(pDevReq->ior.ioFunc == FXIOFUNC_DEV_WRITE_GENERIC);
    DclAssert(pDevReq->ior.ulReqLen == sizeof(FFXIOR_DEV_WRITE_GENERIC));

    pPageBuff = pDevReq->pPageData;
    pAltBuff = pDevReq->pAltData;

    DclMemSet(&FimReq, 0, sizeof FimReq);

    FimReq.ior.ioFunc      = FXIOFUNC_DEV_READ_GENERIC;
    FimReq.ior.ulReqLen    = sizeof(FimReq);
    FimReq.ioSubFunc       = FFX_TRANSFORM_WRITE2READ(pDevReq->ioSubFunc);
    FimReq.ulStartPage     = pDevReq->ulStartPage;
    FimReq.ulCount         = 1;
    if(pPageBuff)
        FimReq.pPageData = buffer.data;
    if(pAltBuff)
    {
        FimReq.pAltData    = &buffer.data[hFimDev->pFimInfo->uPageSize];
        FimReq.nAltDataSize = pDevReq->nAltDataSize;
    }

    while(ulCount)
    {
        ioStat = GenericRead(hFimDev, &FimReq, nAltDataLen);

        /*  Only complete the readback verify if the result is apparently
            successful.
        */
        if( (ioStat.ffxStat != FFXSTAT_SUCCESS)
            && (ioStat.ffxStat != FFXSTAT_FIMUNCORRECTED)
            && (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA))
        {
            /*  Release level code -- if readback verify is turned on, we
                want it to work even when not building in debug mode.
            */
            DclPrintf("FFX: FfxDev:ReadBackVerify GenericRead() failed, Status=%lX\n", ioStat.ffxStat);
            DclProductionError();
            break;
        }

        if(pPageBuff && DclMemCmp(FimReq.pPageData, pPageBuff, hFimDev->pFimInfo->uPageSize) != 0)
        {
            unsigned    nOffset = 0;

            for(nOffset=0; nOffset<hFimDev->pFimInfo->uPageSize; nOffset++)
            {
                if(FimReq.pPageData[nOffset] != pPageBuff[nOffset])
                    break;
            }

            DclPrintf("FFX: FfxDev:ReadBackVerify data mismatch, StartPage=%lU Count=%lU FailedPage=%lU ByteOffset=%u\n",
                pDevReq->ulStartPage, pDevReq->ulCount, FimReq.ulStartPage, nOffset);
            DclHexDump("Original\n", HEXDUMP_UINT8, 32, hFimDev->pFimInfo->uPageSize, pPageBuff);
            DclHexDump("Readback\n", HEXDUMP_UINT8, 32, hFimDev->pFimInfo->uPageSize, FimReq.pPageData);
            DclProductionError();
            break;
        }

      #if FFXCONF_NANDSUPPORT
        if(FimReq.ioSubFunc == FXIOSUBFUNC_DEV_READ_TAGS)
        {
            D_UINT16    k;

            for(k=0; k<nAltDataLen; k++)
            {
                if((pAltBuff[k] & FimReq.pAltData[k]) != FimReq.pAltData[k])
                    break;
            }

            if(k<nAltDataLen)
            {
                DclPrintf("FFX: FfxDev:ReadBackVerify tag mismatch, StartPage=%lU Count=%lU FailedPage=%lU\n",
                    pDevReq->ulStartPage, pDevReq->ulCount, FimReq.ulStartPage);

                DclHexDump("Original:", HEXDUMP_UINT8, nAltDataLen, nAltDataLen, pAltBuff);
                DclHexDump("Readback:", HEXDUMP_UINT8, nAltDataLen, nAltDataLen, FimReq.pAltData);
                DclProductionError();
                break;
            }
        }
        else
      #endif
        {
            if(pAltBuff && DclMemCmp(FimReq.pAltData, pAltBuff, nAltDataLen) != 0)
            {
                DclPrintf("FFX: FfxDev:ReadBackVerify alt mismatch, StartPage=%lU Count=%lU FailedPage=%lU\n",
                    pDevReq->ulStartPage, pDevReq->ulCount, FimReq.ulStartPage);
                DclHexDump("Original:", HEXDUMP_UINT8, nAltDataLen, nAltDataLen, pAltBuff);
                DclHexDump("Readback:", HEXDUMP_UINT8, nAltDataLen, nAltDataLen, FimReq.pAltData);
                DclProductionError();
                break;
            }
        }

        ulCount--;
        FimReq.ulStartPage++;

        if(pPageBuff)
            pPageBuff += hFimDev->pFimInfo->uPageSize;

        if(pAltBuff)
            pAltBuff += nAltDataLen;
    }

    if(!ulCount)
        fSuccess = TRUE;

  #if FFX_DEVMGR_READBACKVERIFY != (TRUE * 2)
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_DEVMGR, 1, TRACEUNDENT),
        "FfxDev:ReadBackVerify() returning %U\n", fSuccess));
  #endif

    return fSuccess;
}


#endif


