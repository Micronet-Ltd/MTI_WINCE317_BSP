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

    This header contains all the prototypes for internal VBF functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: vbfint.h $
    Revision 1.46  2010/12/12 16:46:31Z  garyp
    Added a missing const keyword.
    Revision 1.45  2010/12/12 07:39:33Z  garyp
    Renamed some variables for clarity.  Removed some dead code.
    Revision 1.44  2010/11/22 16:19:16Z  glenns
    Added fQuickMount flag to VBFDATA structure and additional
    supporting data structures if the QuickMount feature is active.
    Revision 1.43  2009/12/12 22:12:45Z  garyp
    Updated FfxVbfNorDiscardCopySet() to use standard DCL macros to manipulate
    the bitmap.  Eliminated CountInSet() and updated to use DclBitCountArray().
    Conditioned all the remaining "Set" functionality inside FFXCONF_NANDSUPPORT.
    Revision 1.42  2009/08/04 00:56:54Z  garyp
    Merged from the v4.0 branch.  Major update to support Deferred NAND
    Discards, and various related refactoring.  Renamed UNITS_TO_CACHE
    to VBF_MAX_UNITS_PER_REGION for clarity.  Eliminated the use of some
    deprecated type names.  Minor datatype changes from D_UINT16 to 
    unsigned.  Changed the SuspendCount variable to be a D_ATOMIC32 type.
    Updated for new compaction functions which now return an FFXIOSTATUS
    value rather than a D_BOOL.
    Revision 1.41  2009/04/15 22:02:05Z  thomd
    Added fEvaluation flag to VBF structures.
    Revision 1.40  2009/01/21 00:17:01Z  billr
    Resolve bug 1562: implement scrubbing of correctable errors.
    Revision 1.39  2008/09/04 19:39:25Z  thomd
    Increase size of Region_t.
    Revision 1.38  2008/05/13 15:55:25Z  thomd
    Specify size of LinearUnit_t depending on size needed.
    Revision 1.37  2008/05/07 02:32:13Z  keithg
    Removed unused UNIT_SCAN_INCREMENT; added the MOUNT_BLOCK_SCAN used by
    ScanUnits(); minor typo corrections; and clarified comments.
    Revision 1.36  2008/04/14 22:13:24Z  billr
    Resolve bug 1947: Media compatibility error - metadata identifier is wrong
    on 3.3 and trunk.
    Revision 1.35  2008/03/17 17:14:36Z  Garyp
    Region functions renamed to avoid namespace collisions.
    Revision 1.34  2008/01/30 23:51:49Z  Garyp
    Eliminated BLOCK_SIZE_MIN and BLOCK_SIZE_MAX.
    Revision 1.33  2007/12/15 01:14:31Z  Garyp
    Fixed a structure padding problem which was causing the CE 6 emulator
    to crash.
    Revision 1.32  2007/11/07 17:25:24Z  pauli
    Made #error message strings to prevent macro expansion.
    Revision 1.31  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.30  2007/10/31 17:38:45Z  Garyp
    Removed the now obsolete FFXCONF_NAND_INTERRUPTED_PROGRAM setting.
    Eliminated Return_t and IOReturn_T.
    Revision 1.29  2007/10/15 17:06:44Z  billr
    Resolve bug 504: Possible infinite compaction loop on NAND with 16 KB
    erase unit.
    Revision 1.28  2007/10/11 17:05:49Z  billr
    Resolve bug 412.
    Revision 1.27  2007/10/04 23:31:13Z  billr
    Resolve bug 970.
    Revision 1.26  2007/09/14 01:17:30Z  Garyp
    Header inclusion updated.
    Revision 1.25  2007/09/13 00:20:42Z  Garyp
    Increased BLOCK_SIZE_MAX to 4096.
    Revision 1.24  2007/07/31 23:45:29Z  timothyj
    Created VBF_LARGE_ADDRESS_THRESHOLDKB macro (in KB) to replace 64-bit
    VBF_LARGE_ADDRESS_THRESHOLD{HIGH|LOW} macros.
    Revision 1.23  2007/07/31 01:24:58Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.22  2007/04/23 20:47:16Z  timothyj
    Fixes BZ 1058, cushion percentage incorrectly displayed.  Updated 64-bit
    math in these modules to use DCL macros (to support environments where
    64-bit operations are non-native).  Redefined VBF_LARGE_ADDRESS_THRESHOLD
    as two 32-bit (high and low) parts, for compatibility with emulated 64-bit
    operations.
    Revision 1.21  2007/04/19 23:56:28Z  timothyj
    Added large address indicator for the erase unit header.  Added threshold
    over which large addressing is required.  Changed partition structure to
    use references to client allocation blocks in lieu of byte offsets.
    Revision 1.20  2007/01/19 04:25:51Z  Garyp
    Added VBF-level support for enabling and disabling idle-time compaction.
    Revision 1.19  2006/11/17 23:00:19Z  Garyp
    Header tweaks to avoid obfuscation problems.
    Revision 1.18  2006/11/10 03:25:47Z  Garyp
    Standardized on a naming convention for the various "stats" interfaces.
    Modified the stats query to use the generic ParameterGet() ability now
    implemented through most all layers of the code.
    Revision 1.17  2006/10/25 16:54:52Z  billr
    Restore some of the obsolete cruft to maintain media compatibility.
    Revision 1.16  2006/10/24 21:52:45Z  billr
    More changes for packing Allocation structure into bit fields. Clean
    up some obsolete cruft.
    Revision 1.15  2006/10/19 23:32:47Z  Garyp
    Commented out a prototype for an unused function.
    Revision 1.14  2006/10/18 17:38:15Z  billr
    Revert some of the changes made in revision 1.13.
    Revision 1.13  2006/10/13 21:53:23Z  billr
    Pack Allocation structure into four bytes using bit fields, add
    support for direct access.
    Revision 1.12  2006/08/22 00:44:59Z  billr
    Resolve bug 284.
    Revision 1.11  2006/08/17 16:18:42Z  billr
    Resolve bug 179.
    Revision 1.10  2006/06/20 14:12:43Z  Garyp
    Updated the VBF region tracking feature so it operates in the same
    fashion as the other statistics interfaces.
    Revision 1.9  2006/05/08 10:38:10Z  Garyp
    Moved the UnitHeader and compaction tracking structures elsewhere.
    Eliminated the unitStart field from VBFPartition.
    Revision 1.8  2006/03/12 19:49:57Z  Garyp
    Added an "fReady" flash to indicate when VBF is ready for use.  This is
    used only by the write interruption tests to ensure that we don't inject
    errors while in the midst of creating or destroying VBF instances.
    Revision 1.7  2006/02/23 21:44:51Z  billr
    SET_DIMENSION() was allocating a byte for each desired bit.
    Revision 1.6  2006/02/21 02:54:26Z  Garyp
    Changed EUH fields back to using block terminology, rather than pages,
    to be consistent with other internal VBF fields (from a client perspective,
    VBF operates on pages).
    Revision 1.5  2006/02/08 21:49:10Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.4  2006/01/06 21:02:38Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.3  2005/12/24 12:52:50Z  Garyp
    Added compaction buffer capability to improve overall compaction times.
    Revision 1.2  2005/12/12 22:56:56Z  garyp
    Added compaction tracking logic.
    Revision 1.1  2005/12/08 02:44:56Z  Garyp
    Initial revision
    Revision 1.5  2005/12/04 21:07:05Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.4  2005/11/14 13:44:34Z  Garyp
    Updated to use the FFXCONF_IDLETIMECOMPACTION setting.
    Revision 1.3  2005/11/06 09:02:05Z  Garyp
    Minor prototype and structure field updates.
    Revision 1.2  2005/10/21 21:27:49Z  garyp
    Increased the reclaim list size from 10 to 60.
    Revision 1.1  2005/10/12 03:31:50Z  Garyp
    Initial revision
    Revision 1.3  2005/10/12 03:31:50Z  Garyp
    New result codes.
    Revision 1.2  2005/10/09 22:28:34Z  Garyp
    Formatting cleanup.
    Revision 1.1  2005/10/02 02:59:10Z  Garyp
    Initial revision
    Revision 1.4  2005/09/18 05:55:35Z  garyp
    Modified so that last read/write access times are recorded in VBF rather
    than the driver framework, and are not conditional on the background
    compaction feature being enabled.
    Revision 1.3  2005/08/30 17:21:53Z  Garyp
    Fixed a typo.
    Revision 1.2  2005/08/21 04:45:08Z  garyp
    Eliminated // comments.
    Revision 1.1  2005/07/11 00:16:54Z  pauli
    Initial revision
    Revision 1.44  2005/05/11 00:56:16Z  garyp
    Modified ScanUnits() to return a more useful error code.
    Revision 1.43  2005/03/09 20:50:06Z  GaryP
    Moved ProcessMetadataSeries() from vbfsup.c to vbfreg;.c and made it
    static, since that is the only place where it is used.
    Revision 1.42  2005/02/13 20:23:07Z  GaryP
    Renamed the PREGION type to PREGIONINFO.  Renamed the PREGION_CACHE
    type to PREGIONCACHE.  General cleanup for readability.
    Revision 1.41  2004/12/30 23:48:11Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.40  2004/09/23 08:19:07Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.39  2004/09/17 19:21:12Z  garys
    removed fFoundDirt from the vbfpartition structure
    Revision 1.38  2004/09/15 21:51:07  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.37  2004/09/10 20:26:37  jaredw
    Added parens to work around a compiler bug.
    Revision 1.36  2004/08/30 17:48:28Z  garys
    avoid suspicious ptr warns in obfuscated vbf.c
    Revision 1.35  2004/04/23 09:04:25  garyp
    Changed to use CHAR_BIT instead of CHAR_BITS.
    Revision 1.34  2004/01/20 22:22:18Z  garys
    Merge from FlashFXMT
    Revision 1.28.1.11  2004/01/20 22:22:18  garyp
    Fixed several issues pertaining to structure field alignment.
    Revision 1.28.1.10  2003/12/30 17:35:11Z  billr
    No functional changes.  Make WriteConsecutiveBlocks() private in vbfsup.c.
    Revision 1.28.1.9  2003/12/29 17:57:58Z  billr
    VBF_API_BLOCK_SIZE is no longer supported.
    Revision 1.28.1.8  2003/12/18 19:41:33Z  billr
    Fix two separate multi-threading issues in garbage collection.
    Revision 1.28.1.7  2003/12/17 21:12:46Z  garyp
    Fixed several structure padding problems.
    Revision 1.28.1.6  2003/12/17 17:37:44Z  garyp
    Removed some obsolete structures and settings.
    Revision 1.28.1.5  2003/12/04 21:24:24Z  billr
    Changes for thread safety. Compiles (Borland/DOS), not yet tested.
    Revision 1.28.1.4  2003/11/24 20:17:10Z  garys
    moved the return type typecast for the REGION_OF macro
    Revision 1.28.1.3  2003/11/19 23:32:35  billr
    Merged from trunk build 744.
    Revision 1.31  2003/11/13 21:07:17Z  billr
    Implement multiple region caches.
    Revision 1.30  2003/10/21 16:59:18Z  garyp
    Reverted to previous revision to fix checkin problem.
    Revision 1.28.1.2  2003/11/03 06:14:54Z  garyp
    Re-checked into variant sandbox.
    Revision 1.29  2003/11/03 06:14:54Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.28  2003/10/21 16:59:18Z  billr
    Resolve IR 4293: possible zombie discard after interrupted GC (NOR);
    and IR 4294: Assert fails in MountRegion after interrupted GC.
    Revision 1.27  2003/09/19 00:00:09Z  billr
    Change test for reserved space to eliminate a warning from some
    compilers when FFX_RESERVED_SPACE was zero.
    Revision 1.26  2003/09/11 20:52:23Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.25  2003/08/12 22:12:17  billr
    No functional changes. Comment changes only.
    Revision 1.24  2003/08/05 21:45:32Z  billr
    Revision 1.23  2003/06/13 11:50:42Z  garyp
    Eliminated VBF_SCRATCH_BLOCK_SIZE.  Moved several settings to VBF.H so this
    header does not need to be included directly by the DOS FORMAT utility.
    Revision 1.22  2003/06/12 23:49:20Z  billr
    Extend the search for the first valid EUH in a partition to allow for the
    case of all the spares being at the beginning of the partition and erased.
    Revision 1.21  2003/06/11 21:45:52Z  billr
    Comment changes only.
    Revision 1.20  2003/06/10 23:05:49Z  dennis
    Refactored BBM and MGM FIMs for better obfuscation.
    BBM format logic is now in bbminit() function in BBMFMT.C.
    Moved BBM.H and TRACE.H to ffx\include directory.
    Reduced the clear text tokens in obfuscated files (using cobf
    -hi switch and by refactoring header files)  Changed definition
    of vbf writeint to pacify obfuscator.
    Revision 1.19  2003/05/23 00:05:50  billr
    Remove isNAND flag from the EUH. The Media Manager now gets this
    information via oemmediainfo() during vbfmount().
    Revision 1.18  2003/05/22 19:27:22Z  garyp
    Added some padding fields to cause proper alignment.
    Revision 1.17  2003/05/20 01:16:31Z  dennis
    More work on FfxVbfCompact.  Fixed bug where partition was not getting fully
    cleaned and another problem where explicit discards to fill up the (only)
    partially discarded unit with free space were not being counted.
    Revision 1.16  2003/05/19 16:19:04  billr
    Media Manager now modifies its behavior to accommodate MLC flash
    and flash parts that don't allow writing a '1' where a '0' was previously
    programmed. A FIM controls this by setting bits in uDeviceType in
    the ExtndMediaInfo structure.
    Revision 1.15  2003/05/17 05:00:47Z  dennis
    Added ordered list processing in FfxVbfCompact so that less time
    is spent mounting regions and more time is spent actually reclaiming
    the disk.  The list is ordered by "dirtiness" and then by "business".
    The list is small.  When the list is empty the remainder of the regions
    are inspected and cleaned.
    Revision 1.14  2003/05/15 17:13:41  billr
    Modify bit set functions to help the compiler generate better code.
    Revision 1.13  2003/05/09 03:50:30Z  dennis
    Changes to fix a discard bug when using metadata.   Also added some more
    checks in VBFDEB and did a little bit of cleanup of the code.
    Revision 1.12  2003/05/02 17:48:43  billr
    Correct calculation of REGION_CACHE_MAX_ALLOCS.
    Revision 1.11  2003/05/01 19:41:25Z  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h. core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.
    Revision 1.10  2003/04/25 23:15:34Z  billr
    Fix the reserved space issue the right way. The Media Manager takes
    care of the offset, the Allocator always starts with linear unit number 0.
    Revision 1.9  2003/04/22 00:40:52Z  dennis
    Added more write interruption tests and code to deal with them.
    GCs will now fully clean a region if called by a background process
    without disturbing temporal order.  GC "granularity" is decreased.
    Revision 1.8  2003/04/16 17:05:59  billr
    Merge from VBF4 branch.
    Revision 1.6.1.10  2003/04/10 01:38:36Z  dennis
    Removed most of the conditional code left over from the MM merge.
    Revision 1.6.1.9  2003/04/04 23:57:37  billr
    Add the new Media Manager code.
    Revision 1.6.1.8  2003/03/27 02:07:23Z  dennis
    All unit default unit tests pass with metadata working on NXMS,
    comments, functions and data cleaned up.
    Revision 1.6.1.7  2003/03/20 21:01:51  dennis
    Factored the wear leveling code into its own policy function
    Revision 1.6.1.6  2003/03/05 05:14:41  dennis
    Garbage collection and explicit discards mostly work.
    Revision 1.6.1.5  2003/03/04 05:53:08  dennis
    Refactored _vbfwrite to write blocks into a region.   WriteNewBlock
    now transparently handles retries of partial writes due to I/O errors.
    WriteConsecutiveBlocks discards data that failed to be fully written
    to the flash.  WriteConsecutiveBlocks attempts to explicitly discard
    any valid data that is being implicitly discarded (up to the MM what
    to do about that.)   All write routines now return a ReturnType struct.
    Revision 1.6.1.4  2003/02/24 22:46:24  dennis
    These changes maked the TESTVBF small blk tests pass.
    Revision 1.6.1.3  2003/02/24 17:53:19  dennis
    Changed the in-memory Allocation struct definition and
    changes to the MM interface
    Revision 1.6.1.2  2003/01/21 22:31:08  dennis
    Established temporal order to units within a region.  Expanded the
    representation of unit counts to 32-bits.
    Revision 1.7  2002/11/14 07:41:08Z  dennis
    No changes.
    Revision 1.6  2002/11/14 07:41:08  garyp
    Removed the __COBF__ conditional.
    Revision 1.5  2002/11/14 06:53:04Z  garyp
    Updated the module header.
    Revision 1.4  2002/11/13 21:57:54Z  garyp
    Moved vbfgetunitinfo() prototype to VBF.H so it is accessible to
    TESTVBF.C even when VBF is obfuscated.
    Revision 1.3  2002/11/12 22:53:40Z  garyp
    Restructured to include most all the VBF internal prototypes except
    for the low level routines.
---------------------------------------------------------------------------*/

#ifndef VBFINT_H_INCLUDED
#define VBFINT_H_INCLUDED

#include <vbfconf.h>
#include <vbfunit.h>
#include <fxstats.h>


/**********************************************************
    VBF has a number of limitations and characteristics.
  **********************************************************/

/*  Erase count information
*/
#define MAX_ERASE_COUNT         (0x7FFFFFFFL)

/*  Smallest logical unit size
*/
#define MIN_UNIT_SIZE           (16L * 1024L)

/*  Minimum number of units in the nominal region size.  There
    may be fewer in the last region of a partition.
*/
#define MIN_UNITS_PER_REGION    (2)

/*-------------------------------------------------------------------
    The maximum number of VBF blocks in any single region.  This is
    limited by the field sizes of the on-disk allocation structures
    to a maximum of 4096 blocks (0..4095).  Each of the allocation
    entries within a region is cached in a VBFREGIONINFO structure
    and each entry requires six bytes of RAM.

    This may be set to a value less than 4096 in environments where
    RAM memory is very tight, but it must be done with caution.  If
    you make the number too small then you will not be able to
    account for all the allocation entries you will need within a
    region.

    With all other values set to their default, a value of 4096 will
    allow a partition size of about 2GB.  Using a value of 1024 will
    only allow a partition size of about 512MB but will save 18KB of
    RAM per cached region.
-------------------------------------------------------------------*/
#define REGION_CACHE_MAX_ALLOCS (VBF_MAX_UNIT_BLOCKS * MIN_UNITS_PER_REGION)



/**********************************************************
    Definitions that affect statically allocated structure sizes...
  **********************************************************/



/*-------------------------------------------------------------------
    This controls the maximum number of spare units that are allowed
    within a partition.  By default, a single spare unit is allocated
    to each partition but the VBF format options allow more to be
    specified.  Having extra spare units in a partition may allow for
    some additional bad block protection on NOR flash.
-------------------------------------------------------------------*/
#define SPARE_UNITS_MAX    (5)

/*-------------------------------------------------------------------
    Maximum number of valid units within a region.  This value is
    established to provide a good deal of flexibility in the sizing
    of units during the VBF format operation.
-------------------------------------------------------------------*/
#define VBF_MAX_UNITS_PER_REGION        (16)

/*-------------------------------------------------------------------
    Disks larger than this value require a different method
    of working with the addresses stored in the VBF erase unit
    header on the media.  This value is in units of KBytes, and
    represents 2GB.
-------------------------------------------------------------------*/
#define VBF_LARGE_ADDRESS_THRESHOLDKB  (0x00200000UL)

/*-------------------------------------------------------------------
    VBF uses this bit in the regionNumber field of the on-media
    erase unit header to indicate that the remaining bits in the
    field contain a region number.  If this bit is clear, the
    remaining bits contain a linear byte offset (client address).
-------------------------------------------------------------------*/
#define VBF_REGION_NUMBER_INDICATOR (0x80000000UL)

/*-------------------------------------------------------------------
    The default compaction thresholds are different for NAND and NOR,
    since NOR typically has a much longer life and can be compacted
    more aggressively.  Additionally the benefit of background
    compaction is higher with NOR than NAND because NOR erases so
    much more slowly.  These defaults are used if the OEM has left
    the vbfconf.h setting VBFCONF_COMPACTIONTHRESHOLD set at the
    default value of -1, which means "let FlashFX figure it out".
-------------------------------------------------------------------*/
#define VBF_DEFAULT_NAND_COMPACTION_THRESHOLD   (65)
#define VBF_DEFAULT_NOR_COMPACTION_THRESHOLD    (50)

/*-------------------------------------------------------------------
    The type of the index into the unitInfo array of UnitData
    structures in a RegionCache struct.  Abbreviated lgu.
    LOGICAL_UNIT_MAX determines the dimension of the unitInfo array.

    Because of the Hide n' Seek method we employ in the discovery
    of spare units, and because of interrupted compactions, we may,
    for a transient interval during the region mount process, end
    up with more units identified by their EUH as belonging to a
    particular region than really belong there.  The most units we
    can transiently end up identifying as a belonging to a region is
    identified by LOGICAL_UNIT_MAX.  The greatest number of valid units
    that can be in a region is identified by VBF_MAX_UNITS_PER_REGION.
    It is the job of the region mount code to eject what are actually
    spare units from the region cache and make sure the number of
    remaining units is <= VBF_MAX_UNITS_PER_REGION (or the actual
    number of units in the particular region if that number is less
    than VBF_MAX_UNITS_PER_REGION -- as may happen when formatting
    less than maximum sized partitions).

    LGU_EOL is the sentinal value that marks the end of a linked list
    of lgus.
-------------------------------------------------------------------*/
typedef D_UCHAR LogicalUnit_t;
#define  LOGICAL_UNIT_MAX  (VBF_MAX_UNITS_PER_REGION + SPARE_UNITS_MAX)
#define  LGU_EOL              (255)

/*-------------------------------------------------------------------
    The type of a region number within a VBF Disk.  The size of 
    Region_t controls the number of distinct regions that a Disk
    can account for.  The two highest possible values are reserved
    for internal purposes.
-------------------------------------------------------------------*/
typedef D_UINT32  Region_t;
#define REGION_MAX D_UINT32_MAX

/*-------------------------------------------------------------------
    The type of an index into the array of linear erase units that
    make up a VBF partition.  Abbreviated lnu.  A linear unit is
    only operated on in the Media Manager.

    LINEAR_UNIT_MAX defines the maximum number of erase units that
    may be contained in a single partition.
-------------------------------------------------------------------*/
#if VBF_PARTITION_MAX_UNITS > D_UINT16_MAX
typedef D_UINT32  LinearUnit_t;
#else
typedef D_UINT16  LinearUnit_t;
#endif

/*  Values of AllocationState are arbitrary.  Note that the array
    of Allocations is initialized to all ones, which doesn't match
    any of these values.
*/
typedef enum
{
    ALLOC_FREE = 0,
    ALLOC_VALID,
    ALLOC_DISCARDED,
    ALLOC_BAD,
    ALLOC_RESERVED = 7  /* Can't use allocation value 111b */
} AllocationState;


typedef struct
{
    unsigned nState : 3;            /* Any one of the AllocationStates described above */
    unsigned lgu : 5;               /* The logical unit containing the associated block */
    unsigned uBlkInRegion : 13;     /* The block offset into the containing region. */
    unsigned uBlkInUnit : 11;       /* The block offset into the containing linear unit. */
} Allocation;


/*  Each unit in the region cache has this info
*/
typedef struct
{
    D_UINT32        ulSequenceNumber;   /* within the partition */
    LinearUnit_t    lnu;                /* within the regionList[] */
    LogicalUnit_t   lguNext;            /* oldest younger lgu in region (or LGU_EOL) */
    LogicalUnit_t   lguPrev;            /* youngest older lgu in region (or LGU_EOL) */
    D_UINT16        uFirstFreeBlock;    /* linear address of first free data */
    D_UINT16        uCleanBlocks;       /* number of clean allocs (available) */
    D_UINT16        uInvalidBlocks;     /* number of invalid (dirty) allocations (including metadata record) */

    /*  Flag indicating this logical unit needs a correctable error
        scrubbed.  This is a D_UINT32 in anticipation of eventually
        using DCL atomic operations to manipulate it.  Until that
        happy day, hold the region's pMutexAlloc to modify this.
    */
    D_UINT32        fScrub;
} UnitData;


#if FFXCONF_NANDSUPPORT
    typedef unsigned int Set;

    #define SET_DIMENSION(n) (((n) + sizeof (Set) * CHAR_BIT - 1) / (sizeof (Set) * CHAR_BIT))

    /*  The discard set always occupies a full VBF block even if it doesn't
        need that many bits.  It must fit within one block so that it can be
        written atomically.
    */

    #if (REGION_CACHE_MAX_ALLOCS > VBF_MAX_BLOCK_SIZE * CHAR_BIT)
        #error "FFX: A complete discard set must fit in one allocation block."
    #endif


    /*  A DiscardSet_t holds the NAND discard metadata, which must fit in
        a VBF allocation block, which is (generally) the same size as a
        NAND page.  This is REGION_CACHE_MAX_ALLOCS (for all the allocations
        that could really be in the region) plus SPARE_UNITS_MAX times 
        VBF_MAX_UNIT_BLOCKS (because all the spares could potentially appear
        to be in the same region).
    */

    /*  Note assumption that sizeof (Set) evenly divides VBF_MAX_BLOCK_SIZE
    */

    typedef Set DiscardSet_t[DCLMAX(VBF_MAX_BLOCK_SIZE / sizeof(Set),
        SET_DIMENSION(REGION_CACHE_MAX_ALLOCS + SPARE_UNITS_MAX * VBF_MAX_UNIT_BLOCKS))];
   
    /*  The value of METADATA_MAGIC_ADDR preserves historical behavior.
    */
    #define  METADATA_MAGIC_ADDR        (0x1FF0)
    #define  METADATA_SERIES_BLOCKS     1
#endif


/*  The region is cached in this structure
*/
typedef struct
{
    D_BOOL          fValid;         /* Is its data valid? */
    D_ATOMIC32      ulUsageCount;
    Region_t        r;              /* And what region number */
    D_UINT16        numUnits;       /* How many units are here */
    PDCLMUTEX       pMutexAlloc;    /* mutex protecting allocation info */
    DCLTIMESTAMP    tsReleased;     /* Time the region was last released */

    /*  This holds information for each logical unit.  The UnitData structures
        are temporally ordered by regional sequence number during the mount,
        at which time an ordered list is established.  lguMin and lguMax point
        to the oldest and youngest units in the region, respectively.  The
        next lgu in the ordered list is the oldest younger lgu in the region.
        The temporal order of the list is maintained across compactions.
    */
    UnitData        unitInfo[LOGICAL_UNIT_MAX];
    LogicalUnit_t   lguMin;
    LogicalUnit_t   lguMax;

    /*  lguMetadata is the logical unit which contains the most recent
        discard metadata record on NAND.  Will always be LGU_EOL for NOR.
        If there has <never> been an explicit discard in a region, or if
        any explicit discards have since been overwritten, then the value
        will be LGU_EOL.
    */
    LogicalUnit_t   lguMetadata;

    /*  This field appears to be obsolete since region cache information is
        now completely private to the VBF instance.  There is no need to
        validate that the cached entry belongs to the right partition.
        However, we need it for now since some helper function only pass
        around the pRegion pointer and not the VBF instance data.
    */
    struct sVBFDATA *pPartition;    /* partition containing this region */

  #if FFXCONF_NANDSUPPORT
    /*  If the MM does not support explicit marking of discarded blocks then
        a metadata record containing the set of all discarded allocations must
        be written out when an explicit discard is performed.  If the current
        discard set is in the from unit of a compaction then a new record must
        be written out to replace it.

        The fMetadataDirty flag means that the DiscardSet does not match the
        MetaData record on the media and that it cannot be reconstructed by
        reading the media during a region mount.
    */
    DiscardSet_t    DiscardSet;
    D_UINT16        uDiscardCount;
    D_UINT16        uMetadataBlock;
    D_BOOL          fMetadataDirty;
  #endif

    /*  compaction stuff
    */
    D_UINT16        uBlocksSpent;
    D_UINT16        uBlocksLeft;

    /*  Count of logical units needing correctable errors scrubbed.
        This is a D_UINT32 in anticipation of eventually using DCL
        atomic operations to manipulate it.  Until that happy day,
        hold pMutexAlloc to modify this.
    */
    D_UINT32        ulScrubCount;

    /*  This is the allocation list for the region.  The allocations are
        maintained in pigeon-hole order according to their regional address
        and so may be directly selected by that address.
    */
    Allocation      regionAlloc[REGION_CACHE_MAX_ALLOCS];
} VBFREGIONINFO;

/*  Mark a logical unit of a region as having a correctable
    error that needs to be scrubbed.  The region's mutex
    (pMutexAlloc) must be held, unless this is done during
    region mount.
*/
#define VBF_MARK_FOR_SCRUBBING(pr, lgu)         \
    do                                          \
    {                                           \
        (pr)->unitInfo[(lgu)].fScrub = TRUE;    \
        ++(pr)->ulScrubCount;                   \
    }                                           \
    while (0)

#define VBF_NEEDS_SCRUBBING(pr) ((pr)->ulScrubCount != 0)

/*  Opaque type for the Region Cache structure
*/
typedef struct sVBFREGIONCACHE VBFREGIONCACHE;

/*  Opaque type for the Media Manager
*/
typedef struct MEDIAMGRINSTANCE * HMEDIAMGR;

typedef struct tagVBFWINTTESTDATA  *VBFWINTTESTHANDLE;

/*  This structure contains data for EVERY region, even though which
    are not currently mounted.  Therefore is should be kept as small
    as reasonably possible.
*/    
typedef struct
{
    D_ATOMIC32      ulCompactionKey;
} VBFREGIONDATA;

typedef struct
{
    unsigned        nNotClean;
    unsigned        nDirty;
    unsigned        nVeryDirty;
    unsigned        nSuperDirty;
} VBFCOMPKEYDATA;

typedef FFXSTATUS (*PFNDISCARDVALIDALLOCS)(const struct sVBFDATA *pVBF, VBFREGIONINFO *pRegion, D_UINT32 ulFirstAlloc, D_UINT32 ulAllocCount);

typedef struct sVBFDATA
{
    D_UINT16        uTotalBlocksPerUnit;    /* MM: size in blocks of an erase unit  */
    D_UINT16        uAllocationBlockSize;   /* from EUH                             */
    D_UINT32        formattedBlocks;        /* client available data allocation blocks */
    D_UINT32        serialNumber;           /* MM: partition serial number          */
    HMEDIAMGR       hMM;                    /* Handle for Media Manager instance    */
    D_UINT16        numSpareUnits;          /* MM: number of spare units available  */
    D_UINT16        uWearLevelDelayCount;   /* scales wear leveling to total compaction count */
    D_UINT32        clientBlocksPerRegion;  /* size of each region, in allocation blocks */
    D_INT32         lLowestEraseCount;      /* lowest erase count of some unit      */

    LinearUnit_t    lnuTotal;               /* number of units in the media         */
    LinearUnit_t    lnuNext;                /* location of last spare unit found +1 */
    LinearUnit_t    lnuStatic;              /* next static unit to reclaim */
    LinearUnit_t    lnuPerRegion;           /* Number of logical units per region   */
    D_UINT16        uBlocksPerUnit;         /* Number of allocations per unit       */
    D_UINT16        uBadWriteCount;         /* Number of consecutive bad writes     */

    /*  list of our linear units
    */
    Region_t        numRegions;             /* number of regions                */
    Region_t        regionList[DCL_PAD(VBF_PARTITION_MAX_UNITS, Region_t)];

    unsigned        nDiskNum;               /*                                  */
    unsigned        nTotalRegionsCached;

    VBFREGIONCACHE *pRegionCache;           /* Partition's region cache         */
    FFXFMLHANDLE    hFML;
    PDCLMUTEX       pMutexWrite;            /* enforce single writer            */

    D_UINT32        ulCompactionBufferLen;
    D_BUFFER       *pCompactionBuffer;
    D_BUFFER       *pTempBuffer;            /* NOR only */

    PFNDISCARDVALIDALLOCS   pfnDiscardValidAllocs;

    VBFREGIONDATA          *paRegionData;       /* ptr to the 1st of an array of structures */
    unsigned                nCompKeySuperDirty;
    unsigned                nCompKeyVeryDirty;
    unsigned                nCompKeyDirty;

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    D_ATOMIC32              ulCompactionSuspendCount;
    volatile DCLTIMESTAMP   tLastReadTime;
    volatile DCLTIMESTAMP   tLastWriteTime;
    D_UINT32                ulReadInactivityMS;     /* ms of read inactivity    */
    D_UINT32                ulWriteInactivityMS;    /* ms of write inactivity   */
  #endif

  #if FFXCONF_STATS_COMPACTION
    VBFCOMPSTATS            CompStats;
  #endif

  #if VBF_WRITEINT_ENABLED
    VBFWINTTESTHANDLE       hWTD;                   /* Write interruption test data     */
  #endif

  #if FFXCONF_NANDSUPPORT
    D_BUFFER               *pbmpRegionsMounted;
  #endif

    unsigned                valid:1;                /* Is this Disk valid?     */
    unsigned                fReadonly:1;            /* Is this Disk read only? */
    unsigned                fReady:1;
    unsigned                fLargeAddressing:1;     /* Support large addressing? */
    unsigned                fIsNand : 1;
    unsigned                fDeferredDiscard : 1;   /* TRUE only for NAND */
    unsigned                fAllRegionsCached : 1;  /* All good things come to those who wait... */
    unsigned                fEvaluation:1;          /* evaluation limit enabled     */

  #if FFXCONF_QUICKMOUNTSUPPORT
    unsigned                fQuickMount:1;          /* TRUE if QuickMount enabled for this partition */
  #endif
  
} VBFPartition, VBFDATA, *PVBFPARTITION;  /* IR4348   */

#if FFXCONF_QUICKMOUNTSUPPORT

typedef D_UINT32 *VBFSTATEINFO;

#define VBF_SSI_VERSIONNUM      0x01
#define VBF_SSI_ELEMENT_SIZE    sizeof(D_UINT32)

enum
{
    VBF_SSI_CHECKSUM = 0,
    VBF_SSI_SIZE,
    VBF_SSI_DISKNUM,
    VBF_SSI_NUMENTRIES,
    VBF_SSI_VERSION,
    VBF_SSI_SERIALNUM,
    VBF_SSI_STATIC_UNIT,
    VBF_SSI_LOWEST_COUNT,
    VBF_SSI_FIXED_COUNT
};

#endif

/**********************************************************
    Defines for the above structures...
  **********************************************************/

/*  The erased state!
*/
#define ERASED_8              ((D_UCHAR)(0xFF))
#define ERASED_16             ((D_UINT16)(0xFFFF))
#define ERASED_32             ((D_UINT32)(0xFFFFFFFFL))

#define REGION_OF_BLOCK(x,p)     ((Region_t)((x) / RegionBlocks(p)))

/*  VBF previously used a char (8 bits) for a region number,but
    now uses an unsigned integer (16 bits).
*/

/*  Unit list identifiers.  These are special values used to
    indicate the purpose for units mapped within the disk.

    These values must fit in a Region_t.
 */
#define LIST_UNIT_SPARE     (REGION_MAX - 0)
#define LIST_UNIT_BAD       (REGION_MAX - 1)

/*  This is the maximum number of erase blocks scanned when
    looking for an erase unit header.  Used by MountEUH().
*/
#define MOUNT_BLOCK_SCAN    (16)


#define VBF_COMPKEY_CLEAN           (0)             /* Has 0 discarded allocs */
#define VBF_COMPKEY_NOTCLEAN        (0x00000001)    /* Has >= 1 and < "Qualified" discarded allocs */
#define VBF_COMPKEY_QUALIFIED       (0x00000100)    /* Meets OEM configured "Dirty" threshold */
#define VBF_COMPKEY_VERYQUALIFIED   (0x00010000)    /* Midway between "Qualified" and "SuperQualified" */
#define VBF_COMPKEY_SUPERQUALIFIED  (0x01000000)    /* Is almost all discarded allocs */
#define VBF_COMPKEY_BUSY            (D_UINT32_MAX-1)/* Being updated */
#define VBF_COMPKEY_UNINITIALIZED   (D_UINT32_MAX)  /* Not yet examined */


/*  vbfdeb.c
*/
D_BOOL          IsValidUnit(   const VBFPartition * pThisPartition, LinearUnit_t lgu);
D_BOOL          IsValidAlloc(  const VBFPartition * pThisPartition, const VBFREGIONINFO *pThisRegion, const Allocation * alloc);
void            FfxVbfValidateRegion(const VBFPartition * pThisPartition, const VBFREGIONINFO *pThisRegion, const char *pszLocale);

/*  vbfreg.c
*/
FFXSTATUS       FfxVbfRegionCacheCreate(  VBFPartition *pVBFInst);
void            FfxVbfRegionCacheDestroy( VBFPartition *pVBFInst, D_UINT32 ulFlags);
VBFREGIONINFO * FfxVbfRegionMount(        VBFPartition *pVBFInst, Region_t r, VBFACCESSTYPE nAccessType);
FFXSTATUS       FfxVbfRegionDataInitialize(VBFDATA *pVBF);
D_BOOL          FfxVbfRegionRemount(      VBFREGIONINFO *pRegion);
void            FfxVbfRegionRelease(      VBFREGIONINFO *pRegion);
void            FfxVbfRegionLock(   const VBFREGIONINFO *pRegion);
void            FfxVbfRegionUnlock( const VBFREGIONINFO *pRegion);
Region_t        GetRegionOfLnu(     const VBFPartition *pVBFInst, LinearUnit_t lnu);
void            SetLnuRegion(             VBFPartition *pVBFInst, LinearUnit_t lnu, Region_t r);
D_BOOL          LinearToLogicalUnit(const VBFREGIONINFO *pRegion, LinearUnit_t lnu, LogicalUnit_t * plgu);
D_UINT32        GetNextRegionalSequenceNumber(const VBFREGIONINFO *pRegion);
LogicalUnit_t   NextLgu(            const VBFREGIONINFO *pRegion, LogicalUnit_t lgu);
/* LogicalUnit_t PrevLgu(           const VBFREGIONINFO *pRegion, LogicalUnit_t lgu); */
LogicalUnit_t   DeleteLgu(                VBFREGIONINFO *pRegion, LogicalUnit_t lgu);
LogicalUnit_t   AppendLgu(                VBFREGIONINFO *pRegion, LogicalUnit_t lgu);

/*  vbfsup.c
*/
D_BOOL          IsValidEUH(const EUH *pEUH);
D_BOOL          IsValidPartition(     const VBFPartition *pThisPartition); /* only used in VBF.C */
D_BOOL          IsReadOnly(           const VBFPartition *pThisPartition);
D_UINT16        AllocationBlockSize(  const VBFPartition *pThisPartition);
D_UINT32        RegionBlocks(         const VBFPartition *pThisPartition);
LinearUnit_t    TotalUnits(           const VBFPartition *pThisPartition);
D_UINT32        FormattedBlocks(      const VBFPartition *pThisPartition);
D_UINT32        UnitSize(             const VBFPartition *pThisPartition);
FFXSTATUS       FfxVbfFindAndLoadEUH(VBFDATA *pVBF, EUH *pEUH);
FFXSTATUS       FfxVbfScanUnits(     VBFDATA *pVBF, EUH *pEUH);
FFXIOSTATUS     FfxVbfWriteNewBlock(        VBFPartition *pThisPartition, VBFREGIONINFO *pRegion, D_UINT32 ulBlockInRegion, D_UINT32 ulBlockCount, const void * pBuffer);
FFXIOSTATUS     FfxVbfReadConsecutiveBlocks(const VBFPartition* pThisPartition, VBFREGIONINFO *pThisRegion, D_UINT32 ulFirstBlock, D_UINT32 ulBlockCount, D_BUFFER *pData);
D_BOOL          FfxVbfDiscardAllocBlocks(   VBFPartition *pThisPartition, VBFREGIONINFO *pRegion, D_UINT16 u1stBlock, D_UINT16 uBlockCount);
FFXIOSTATUS     FfxVbfCompactor(            VBFPartition *pThisPartition, VBFREGIONINFO *pRegion);
D_BOOL          FfxVbfWearLevel(            VBFPartition *pThisPartition);
FFXSTATUS       FfxVbfErrorScrub(           VBFREGIONINFO *pThisRegion);
void            FfxVbfAcquireWriteMutex(const VBFDATA *pVBF);
void            FfxVbfReleaseWriteMutex(const VBFDATA *pVBF);
void            FfxVbfDumpRegionDetails(const VBFREGIONINFO *pRgn);
void            FfxVbfDumpRegions(      const VBFREGIONINFO *apRgn[], unsigned nCount, D_BOOL fDetails);
#if FFXCONF_NANDSUPPORT
    /*  If these functions stay around for the long haul, they should be
        moved into vbfnand.c.  However they are likely to go away...
    */    
    D_BOOL      IsInSet(const Set* s, unsigned int x);
    void        AddToSet(     Set* s, unsigned int x);
    void        RemoveFromSet(Set* s, unsigned int x);
#endif

/*  vbfcomp.c
*/
void            FfxVbfCompactionKeyInit(  VBFDATA *pVBF, int iThresholdPercent);
void            FfxVbfCompactionKeyUpdate(    VBFDATA *pVBF, const VBFREGIONINFO *pRegion);
FFXSTATUS       FfxVbfCompactionKeyData(const VBFDATA *pVBF, Region_t rgn, VBFCOMPKEYDATA *pKeyData);

#if FFXCONF_STATS_VBFREGION
D_BOOL          FfxVbfRegionStatsQuery(     VBFHANDLE hVBF, VBFREGIONSTATS *pStat, D_BOOL fReset);
D_BOOL          FfxVbfRegionStatsReset(     VBFHANDLE hVBF);
#endif

#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
/*  vbfnor.c.
*/
FFXSTATUS       FfxVbfNorDiscardValidAllocs(     const VBFDATA *pVBF, VBFREGIONINFO *pRegion, D_UINT32 ulFirstBlock, D_UINT32 ulBlockCount);
FFXSTATUS       FfxVbfNorDiscardCopySet(         const VBFDATA *pVBF, VBFREGIONINFO *pRegion, LogicalUnit_t lguFrom, const D_BUFFER *pbmpCopySet);
FFXSTATUS       FfxVbfNorFinishCompactionDiscard(const VBFDATA *pVBF, LinearUnit_t lnuFrom, Allocation * const paAlloc, unsigned nAllocs);
D_BOOL          FfxVbfNorMarkImplicitDiscards(VBFREGIONINFO *pRegion, const D_BUFFER *pDiscardBitmap);
#endif

#if FFXCONF_NANDSUPPORT
/*  vbfnand.c
*/
FFXSTATUS       FfxVbfNandDiscardValidAllocs(const VBFDATA *pVBF, VBFREGIONINFO *pRegion, D_UINT32 ulFirstBlock, D_UINT32 ulBlockCount);
D_BOOL          FfxVbfNandUpdateMetadataState(     VBFDATA *pVBF, VBFREGIONINFO *pRegion);
FFXSTATUS       FfxVbfNandForceWriteMetadataSeries(VBFDATA *pVBF, VBFREGIONINFO *pRegion, D_BOOL fTimeCritical);
FFXSTATUS       FfxVbfNandWriteMetadataSeries(     VBFDATA *pVBF, VBFREGIONINFO *pRegion);
D_BOOL          FfxVbfNandReadMetadataSeries(const VBFDATA *pVBF, VBFREGIONINFO *pRegion);
#endif

#if FFXCONF_QUICKMOUNTSUPPORT
/*  vbfquickmount.c
*/
FFXSTATUS FfxVbfSaveDiskStateInfo(const PVBFPARTITION pVBFInst);
FFXSTATUS FfxVbfRestoreDiskStateInfo(PVBFPARTITION pVBFInst);
FFXSTATUS FfxVbfDiskUsesQuickMount(const PVBFPARTITION pVBFInst);
#endif


/*  Some sanity checks...
*/
#if (VBF_PARTITION_MAX_UNITS < VBF_MAX_UNITS_PER_REGION)
#error "Too few logical units in VBF_PARTITION_MAX_UNITS!"
#endif

#ifdef VBF_BLOCK_SIZE
#error "VBF_BLOCK_SIZE is obsolete.  Do not define it."
#endif

#if (VBF_MAX_BLOCK_SIZE < VBF_MIN_BLOCK_SIZE)
#error "Maximum VBF block size is less than minimum!"
#endif

#if (FFX_MAX_DISKS < 1)
#error "The number of units cannot be zero."
#endif

/*  Limit is due to number of bits in a 512-byte NAND data page.  There
    must be a bit for each page in a region for the discard set.
*/

#if (REGION_CACHE_MAX_ALLOCS > 4096)
#error  "REGION_CACHE_MAX_ALLOCS is too big."
#endif


#endif /* VBFINT_H_INCLUDED */
