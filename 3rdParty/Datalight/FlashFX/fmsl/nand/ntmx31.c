/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
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

    This NAND Technology Module (NTM) supports the NAND Flash Controller in
    the Freescale i.MX31 and related chips.

    Currently native page sizes of 512B or 2KB are supported.  2KB Pages
    are also supported using 512B emulation with software ECCs.

    Ultimately, the intention is to support building for one or more of
    five modes (several of which are mutually exclusive):

    - native 512B page size for small-block NAND
    - native 2KB page size for large-block NAND
    - 512 byte page size, with software ECC, emulated on large-block ORNAND
      with command 31h for partial page reads
    - 512 byte page size, with software ECC, emulated on large-block NAND
      (including ORNAND that does not support command 31h for partial page
      reads)
    - 512 byte page size, with hardware ECC, emulated on large-block NAND
    - 2 KB page size emulated on small-block flash.

    Byte order with these controllers is a bit confusing.  In 16-bit mode, the
    NF_BIG bit in NAND_FLASH_CONFIG1 controls the word order in the RAM buffer
    when 32-bit access are performed.  Bytes within a word are not swapped:
    the low-order byte of the chip I/O pins always ends up in the low-order
    byte of the word in the RAM buffer.  This means that byte offsets in the
    RAM buffer depend on the processor byte order (little- or big-endian).

    This controller imposes its own spare area format, placing ECCs for the
    main page and part of the spare area.  ECC is calculated only for the
    first three bytes (8-bit devices) or two words (16-bit devices) of the
    spare area, requiring the hidden data to be stored there.  This conflicts
    with the factory bad block mark for small-block 16-bit chips and all
    large-block chips.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntmx31.c $
    Revision 1.64  2012/03/05 19:46:44Z  garyp
    Minor typecast fixes to operate properly with 64-bit pointers.
    Revision 1.63  2012/02/18 01:13:24Z  garyp
    Declared some variables as static to avoid global namespace contention.
    However the better long-term fix awaits.
    Revision 1.62  2010/12/14 00:26:30Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.61  2010/12/10 05:41:24Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.60  2010/07/06 18:20:17Z  garyp
    Removed some unused code.
    Revision 1.59  2010/07/06 02:24:17Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.58  2010/04/29 00:04:23Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.57  2009/12/11 21:35:00Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.
    Revision 1.56  2009/11/04 20:15:07Z  garyp
    Corrected a debug message.
    Revision 1.55  2009/10/14 22:45:09Z  keithg
    Removed now obsolete fxio.h include file; Changed MEM_PUT...
    macros to the new ones DCLMEM... macros.
    Revision 1.54  2009/10/06 20:46:50Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.53  2009/07/25 00:26:29Z  garyp
    Merged from the v4.0 branch.   Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written. Added a default IORequest()
    function.  Documentation updated.
    Revision 1.52  2009/04/02 15:40:31Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.51  2009/03/31 15:56:41Z  billr
    Remove debug code.
    Revision 1.50  2009/03/30 21:39:48Z  billr
    Fix unbalanced trace indentation.
    Revision 1.49  2009/03/25 17:13:50Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.48  2009/03/18 03:00:59Z  glenns
    - Fix Bugzilla #2370: Remove assignment of obsolete block status
      value.
    Revision 1.47  2009/03/04 20:25:30Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    - Added code to properly report bit error corrections to the error
      manager.
    Revision 1.46  2009/02/06 01:51:14Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.45  2009/01/27 12:42:05  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.44  2009/01/27 00:10:51Z  glenns
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    - Updated PageRead to accomodate FFXSTAT_FIMCORRECTABLEDATA
      status.
    Revision 1.43  2009/01/23 17:45:57Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    --- Added comments ---  billr [2009/01/26 18:24:15Z]
    Added raw page read/write functions.
    Revision 1.42  2009/01/18 08:52:44Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.41  2009/01/16 23:58:21Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in forty-two places.
    Revision 1.40  2008/12/12 07:56:03Z  keithg
    Updated to conditionally use the deprecated xxx_BBMBLOCK type.
    Revision 1.39  2008/11/24 22:16:59Z  billr
    Resolve bug 2010: does not return correct operation flags for
    FML_GET_RAW_BLOCK_STATUS().  Resolve bug 2242: fails assertion in
    FMSL test (tag read/write)
    Revision 1.38  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.37  2008/07/25 16:54:06Z  jimmb
    Added an initialzation to quite the gcc compiler.  It (gcc) does not
    understand the full data path and cannot determine that ucStatus
    will not be used.  Hence the unitialized warning - (error)
    Revision 1.36  2008/06/16 16:55:18Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.35  2008/03/23 20:24:19Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.34  2007/11/07 17:25:25Z  pauli
    Made #error message strings to prevent macro expansion.
    Revision 1.33  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.32  2007/10/14 21:12:35Z  Garyp
    Modified to use a renamed function.
    Revision 1.31  2007/10/01 19:25:20Z  pauli
    Resolved Bug 355: Updated to handle misaligned data buffers and report a
    required alignment of 1.
    --- Added comments ---  pauli [2007/10/01 23:30:01Z]
    Resolved Bug 1475.
    Revision 1.30  2007/09/21 04:35:46Z  pauli
    Added the ability to report that BBM should or should not be used based on
    the characteristics of the NAND part identified.
    Revision 1.29  2007/08/02 23:09:00Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.28  2007/06/26 16:53:19Z  timothyj
    Minor change to debug output (BZ #1237)
    Revision 1.27  2007/06/22 05:35:30Z  timothyj
    Ported 512B emulation and other enhancements from V310_Release branch.
    Revision 1.26  2007/06/12 18:14:09Z  timothyj
    Ported 2KB native and 512B emulated page support from the V310_Release
    branch.
    Revision 1.13.1.11  2007/06/09 18:43:07Z  timothyj
    Changed default build options to 2KB Native and 512B Native.
    Revision 1.13.1.10  2007/06/09 01:22:20Z  timothyj
    Added 512B page emulation with software ECC (including for Spansion ORNAND
    devices).
    Revision 1.13.1.9  2007/06/05 04:17:01Z  timothyj
    Fixed where GetBlockStatus() and SetBlockStatus() were using the page index
    where the offset was required.
    Revision 1.13.1.8  2007/06/02 02:13:00Z  timothyj
    Added 2KB native page support.
    Revision 1.13.1.7  2007/05/22 23:08:39Z  timothyj
    Changed strings from MX21 to MX31 (IR/BZ #998).  Fill spare area buffer to
    ensure programming with fUseEcc FALSE cannot inadvertently clear bits
    (IR/BZ #1165).
    Revision 1.13.1.6  2007/05/14 22:26:03Z  timothyj
    Added calls to new MX31 project hooks for setting the width and page size
    of the detected NAND.  Added bus width to the static information, so that
    the status byte's location within the halfword can be determined at run-
    time.
    Revision 1.23  2007/04/07 03:33:32Z  Garyp
    Removed some unnecessary asserts.  Debug messages updated.
    Revision 1.22  2007/03/28 19:14:27Z  timothyj
    Added writing of tag indicator to allow BBM re-format to distinguish
    between OFFSETZERO-style factory bad blocks and those written with a tag
    (IR/BZ #866).  Modified GetBlockStatus() to handle BBM re-formats of
    existing formatted SSFDC-style flash, even without the tag indicators
    added in this version of the NTM.
    Revision 1.21  2007/03/26 18:58:56Z  timothyj
    Fixed GetBlockStatus() to look at the first two pages (was only looking at
    first page twice) of a block for bad block indicators (BZ/IR #972)
    Revision 1.20  2007/03/23 21:50:18Z  timothyj
    Added byte width configuration using nandconf.h for 8-bit NAND
    support (see IR/BZ#921, 979).
    Converted offset arguments to page numbers for LFA support (see IR/BZ#889).
    Modified NFCWait() and removed calls to ReadyWait(), for polling of INT,
    also added self-tuning and timing statistics for system-specific tuning.
    Added INT_MASK to initial settings to turn off interrupt generation.  Added
    code to unlock buffers (required if reconfigured for buffer 0 or 1).
    Revision 1.19  2007/03/01 20:09:14Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.18  2007/02/13 23:22:47Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.17  2007/02/06 20:31:20Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.16  2007/01/03 23:40:00Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was
    located.  Removed FFXNANDMFG (replaced references with references to the
    constant FFXNANDCHIP returned, above).
    Revision 1.15  2006/11/08 03:38:47Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.14  2006/10/06 00:19:34Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.13  2006/05/25 22:21:19Z  billr
    Workaround for FFXCONF_FORCEALIGNEDIO not aligning reads.
    Revision 1.12  2006/05/22 20:34:22Z  billr
    The NAND FIM shouldn't be writing tags on a page without ECC, but it does.
    Revision 1.11  2006/05/17 22:02:50Z  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    NtmInfo structure.
    Revision 1.10  2006/04/18 19:58:36Z  billr
    Correct placement of bit fields in ECC_STATUS_RESULT register per
    ArgonLV spec rev 2.5.
    Revision 1.9  2006/04/17 20:49:30Z  billr
    Use the correct page size in assertions about alignment.
    Revision 1.8  2006/04/17 20:25:43Z  billr
    The NFC corrects data itself. More fixes from testing.
    Revision 1.7  2006/04/14 21:01:33Z  billr
    Fixes from testing.
    Revision 1.6  2006/04/14 16:52:41Z  billr
    Fix and comment byte order issues. Fix PageIn() to copy to/from the
    right places.
    Revision 1.5  2006/04/13 22:02:48Z  billr
    Fix byte order issue with status. Correct return type in definition of
    NFCReset.
    Revision 1.4  2006/04/13 21:10:45Z  billr
    Always read status in main page. NFC apparently doesn't set INT
    after being reset.
    Revision 1.3  2006/04/13 19:52:58Z  billr
    Fixes from testing, minor cleanup.
    Revision 1.2  2006/04/13 17:49:32Z  billr
    Fix some issues found in testing, plus other cleanup.
    Revision 1.1  2006/04/10 20:22:34Z  billr
    Initial revision
    ---------------------
    Bill Roman 2006-03-23
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <deverr.h>
#include <fxiosys.h>
#include <nandcmd.h>
#include <ecc.h>
#include <errmanager.h>
#include "nandconf.h"
#include "nandctl.h"
#include "nandid.h"
#include "nand.h"
#include "ntm.h"

#define NTMNAME "NTMX31"


/*  Configuration options for this NTM.
*/
/*  Define this as TRUE for variants of the controller that have an NFC_RST
    bit in the NAND_FLASH_CONFIG1 register.
*/
#ifndef NFC_HAS_RESET
#define NFC_HAS_RESET TRUE
#endif

/*  Define this as TRUE for variants of the controller that have an NF_BIG
    bit in the NAND_FLASH_CONFIG1 register.
*/
#ifndef NFC_HAS_ENDIAN
#define NFC_HAS_ENDIAN TRUE
#endif

/*  Define this as TRUE if you're reformatting BBM AND YOU'RE SURE NONE OF
    the ACTUAL factory bad blocks contain FBBSTATUS_FBB_INVALID at SPARE_INDEX_FBBSTATUS
*/
#ifndef FFXCONF_CHECK_DLFBBSTATUS
#define FFXCONF_CHECK_DLFBBSTATUS FALSE
#endif


/*  The following options' defaults can be overridden by defining them
    in the project-specific ffxconf.h file.  Defining them in ffxconf.h
    (as opposed to editing the values defined in this file) is the preferred
    method for configuring this NTM to build for specific support.
*/


/*  Define this as TRUE to include support for flash parts using native 512B page sizes.
    This option can be included in addition to either 2KB Native or 512B emulated, or
    neither, but not both.
*/
#ifndef FFXCONF_NAND_512BNATIVE
#define FFXCONF_NAND_512BNATIVE TRUE
#endif


/*  The following options are mutually excusive - you can either
    build for 2KB Native pages or 512B emulated pages, but not both.
*/

/*  Define this as TRUE to include support for flash parts using native 2K page sizes.
*/
#ifndef FFXCONF_NAND_2KBNATIVE
#define FFXCONF_NAND_2KBNATIVE TRUE
#endif

/*  Define this as TRUE to include support for 2KB ORNAND parts
    with CMD_READ_SEGMENT_START, using emulated 512B page sizes.
*/
#ifndef FFXCONF_NAND_512BORNANDEMULATED
#define FFXCONF_NAND_512BORNANDEMULATED FALSE
#endif

/*  Define this as TRUE to include support for 2KB parts using emulated 512B page sizes.
*/
#ifndef FFXCONF_NAND_512BEMULATED
#define FFXCONF_NAND_512BEMULATED FALSE
#endif


#if FFXCONF_NAND_2KBNATIVE & (FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
#error "Cannot configure for both 2KB native and 512B emulated support"
#endif




/*  Spare area location

    If the flash is small-block NAND (512B pages), the spare area as viewed
    by the MX31 NFC is identical to the spare area of the flash.

    If the flash is large-block NAND (2KB pages), the spare area as viewed
    by the MX31 NFC is generally NOT (except for the last 16 bytes) the same
    as the spare area of the flash, because the NFC reads/writes in 528B
    (main + spare) elements, regardless of the setting of the NFC_FMS setting.

    Generally, this is not a problem, with two notable exceptions:

    1.  The factory bad block marks as programmed by the flash manufacturers
        will NOT be in the spare buffer in the 'obvious' location.

    2.  512B page emulation on chips that require 512B aligned offsets for
        partial page program operations cannot use hardware ECCs, since the
        partial page program operations with hardware ECCs will be aligned on
        528B boundaries.


    Conceptual (and NAND-chip) 2KB page layout (not to scale):

    +------------------------------------------------------+----------------+
    |                   2048B Main Data                    | 64B Spare Data |
    +------------------------------------------------------+----------------+

    Actual (MX31-controller enforced) layout (not to scale):

    +-------------+---+-------------+---+-------------+---+-------------+---+
    |  512B Main  |16B|  512B Main  |16B|  512B Main  |16B|  512B Main  |16B|
    +-------------+---+-------------+---+-------------+---+-------------+---+


    The Offsetzero factory bad block mark therefore appears here,
    in the an MX31 main data area buffer:


                                Factory bad block mark ******
                                                            *
    +------------------------------------------------------+*----------------+
    |                   2048B Main Data                    |* 64B Spare Data |
    +------------------------------------------------------+*----------------+
    +-------------+---+-------------+---+-------------+---+-*------------+---+
    |  512B Main  |16B|  512B Main  |16B|  512B Main  |16B| * 512B Main  |16B|
    +-------------+---+-------------+---+-------------+---+-*------------+---+
                                                            *
                                                            *


    And FlashFX Pro therefore stores an SSFDC-style FBB marker and
    FlashFX value indicating the offset-zero style information is not
    valid, in the 'last' MX31 spare area buffer, which is not overwritten
    by either the MX31 NFC's ECCs or the 2KB main data.  The tag data for
    the 2KB native pages are also stored only in the last 16B spare area:


                                                   FlashFX FBB Info ******
                                                                         *
    +------------------------------------------------------+-------------*---+
    |                   2048B Main Data                    | 64B Spare   *   |
    +------------------------------------------------------+-------------*---+
    +-------------+---+-------------+---+-------------+---+-------------+*---+
    |  512B Main  |16B|  512B Main  |16B|  512B Main  |16B|  512B Main  |*16B|
    +-------------+---+-------------+---+-------------+---+-------------+*---+
                                                                         *
                                                                         *

    HW ECC spare area format

    If the processor is in little-endian mode, the low order byte of each
    16-bit word is at an even address, and the high-order byte is at the
    succeeding odd address.  In big-endian mode, these are reversed.

                        +-----------------------------------+
                     0  |                tag                |
                        +-----------------+-----------------+
                     2  |  fbb status (1) |    tag check    |
                        +-----------------+-----------------+
                     4  |     BI (2)      | ECC status (3)  |
                        +-----------------+-----------------+
                     6  | main ECC byte 2 | main ECC byte 1 |
                        +-----------------+-----------------+
                     8  | tag ECC byte 1  | main ECC byte 3 |
                        +-----------------+-----------------+
                    10  |     BI (4)      | tag ECC byte 2  |
                        +-----------------+-----------------+
                    12  |              unused               |
                        +-----------------------------------+
                    14  |              unused               |
                        +-----------------------------------+

        (1)  Programmed as 0xF0 when the offset-zero FBB location is written.
        (2)  Primary bad block indicator.  Unprogrammed in a good block,
             programmed to 0x00 in first two physical pages of a bad block.
        (3)  Unprogrammmed in a page that was written without ECC, programmed
             to 0xF0 if it was written with ECC.
        (4)  Secondary bad block indicator.  Never programmed, but checked
             if the primary bad block block indicator says the block is good.

    SW ECC spare area format

    If the processor is in little-endian mode, the low order byte of each
    16-bit word is at an even address, and the high-order byte is at the
    succeeding odd address.  In big-endian mode, these are reversed.

                        +-----------------------------------+
                     0  |       BI        |       BI        |
                        +-----------------+-----------------+
                     2  |  fbb status (1) |     unused      |
                        +-----------------+-----------------+
                     4  |     unused      | ECC status (2)  |
                        +-----------------+-----------------+
                     6  | main ECC byte   | main ECC byte   |
                        +-----------------+-----------------+
                     8  | main ECC byte   | main ECC byte   |
                        +-----------------+-----------------+
                    10  | main ECC byte   | main ECC byte   |
                        +-----------------+-----------------+
                    12  |                tag                |
                        +-----------------+-----------------+
                    14  |    tag check    |    tag ECC      |
                        +-----------------+-----------------+

        (1)  Left unprogrammed when using SW ECC (which does not overwrite the
             FBB), but checked in GetBlockStatus in case the flash was
             previously formatted with HW ECC.
        (2)  Unprogrammmed in a page that was written without ECC, programmed
             to 0xF0 if it was written with ECC.

*/

/*  The RAM buffer contains 16-bit words, and may not be accessed as bytes.
    It gets copied to RAM for manipulation.  The word order can be correctly
    configured with NF_BIG in NAND_FLASH_CONFIG1 so that the word order is
    correct when 32-bit accesses are performed.  The offsets of bytes need
    to be conditional on the processor byte order (specified at compile time).
*/
/* word location, address must be even */
#define SPARE_INDEX_HWECC_TAG   0
/* word location, address must be even */
#define SPARE_INDEX_SWECC_TAG   12
#define TAG_SIZE 2

/*  These are byte locations, offsets depend on byte order.
*/
#define SPARE_INDEX_HWECC_TAGCHECK  (2 ^ DCL_BIG_ENDIAN)
#define SPARE_INDEX_HWECC_TAGECC    (9 ^ DCL_BIG_ENDIAN)

#define SPARE_INDEX_SWECC_ECC1      (6)
#define SPARE_INDEX_SWECC_ECC2      (9)
#define SPARE_INDEX_SWECC_TAGCHECK  (14 ^ DCL_BIG_ENDIAN)
#define SPARE_INDEX_SWECC_TAGECC    (15 ^ DCL_BIG_ENDIAN)

#define SPARE_INDEX_FBBSTATUS       (3 ^ DCL_BIG_ENDIAN)
#define SPARE_INDEX_ECCSTATUS       (4 ^ DCL_BIG_ENDIAN)
#define SPARE_INDEX_BLOCKSTATUS1    (5 ^ DCL_BIG_ENDIAN)
#define SPARE_INDEX_BLOCKSTATUS2    (11 ^ DCL_BIG_ENDIAN)

#define ECCSTATUS_NONE ERASED8
#define ECCSTATUS_ECC 0xF0

/*  This indicator is used to indicate that the location potentially
    containing a factory bad block marker is invalid (has been erased or
    overwritten).  Note that this only applies for those pages in blocks that
    were NOT marked bad by the manufacturer (in other words, we don't write
    this value in blocks that actually contain a FBB marker indicating the
    block is bad - only in blocks that DON'T).
*/
#define FBBSTATUS_FBB_INVALID 0xF0

#define CORRECTABLE(a, b) DCLISPOWEROF2((a) ^ (b))

#define MAIN_BUFFER_SIZE (0x200)
#define SPARE_BUFFER_SIZE (16)


/*  Timeout in msec. for NAND flash operations.  The longest operation is
    erase, with typical spec of 2 msec, but can be as high as 174ms on some
    devices.  It's okay if this is generous, timeout generally means
    something is badly wrong, and wasting a few  msec. doesn't matter at
    that point.
*/
#define NAND_TIMEOUT (5000)

/*  Private table for the NTM's NAND device-specific internal operations.
*/
typedef struct
{
    FFXIOSTATUS (*PageReadEcc)    (D_UINT32 ulPage,       void *pMain,       void *pTag, unsigned nTagSize);
    FFXIOSTATUS (*PageReadNoEcc)  (D_UINT32 ulPage,       void *pMain, void *pSpare);
    FFXIOSTATUS (*PageWriteEcc)   (D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize);
    FFXIOSTATUS (*PageWriteNoEcc) (D_UINT32 ulPage, const void *pMain, const void *pSpare);
    FFXIOSTATUS (*SpareRead)      (D_UINT32 ulPage,       void *pSpare);
    FFXIOSTATUS (*SpareWrite)     (D_UINT32 ulPage, const void *pSpare);
    FFXIOSTATUS (*TagRead)        (D_UINT32 ulPage,       void *pTags, unsigned nTagSize);
    FFXIOSTATUS (*TagWrite)       (D_UINT32 ulPage, const void *pTags, unsigned nTagSize);
    FFXIOSTATUS (*GetPageStatus)  (D_UINT32 ulPage);
    FFXIOSTATUS (*GetBlockStatus) (D_UINT32 ulPhysicalPage, D_UINT8 bFbbType);
    FFXIOSTATUS (*SetBlockStatus) (D_UINT32 ulPhysicalPage, D_UINT32 ulBlockStatus);
}   NTMX31OPERATIONS;


/*  Private data structure for this NTM.
*/
struct tagNTMDATA
{
    FFXDEVHANDLE        hDev;
    NTMINFO             NtmInfo;    /* information visible to upper layers */
    NTMX31OPERATIONS   *pNANDOps;   /* NAND device specific operations functions  */
    const FFXNANDCHIP  *pChipInfo;
    PNANDCTL            pNC;        /* Private project hooks information   */
};


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_mx31 =
{
    Create,
    Destroy,
    PageRead,
    PageWrite,
    HiddenRead,
    HiddenWrite,
    SpareRead,
    SpareWrite,
    BlockErase,
    GetPageStatus,
    GetBlockStatus,
    SetBlockStatus,
    ParameterGet,
    ParameterSet,
    RawPageRead,
    RawPageWrite,
    IORequest
};

/*  Generic operations (conceptually perhaps should be in the
    vector, but no differences among implementations)
*/
static FFXSTATUS ProcessChipStatus(unsigned int uChipStatus);
static FFXIOSTATUS EraseBlock(D_UINT32 ulPhysicalPage);

/*  Shared operations (common among several implementations)
*/
#if (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
static FFXIOSTATUS GetBlockStatus2K(D_UINT32 ulPhysicalPage, D_UINT8 bFbbType);
static FFXIOSTATUS SetBlockStatus2K(D_UINT32 ulPhysicalPage, D_UINT32 ulBlockStatus);
#endif /* FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED */


#if FFXCONF_NAND_512BNATIVE
/*  Operations for flash with 512B pages
*/
static FFXIOSTATUS PageRead512Ecc(D_UINT32 ulPage, void *pMain, void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageRead512NoEcc(D_UINT32 ulPage, void *pMain, void *pSpare);
static FFXIOSTATUS PageWrite512Ecc(D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageWrite512NoEcc(D_UINT32 ulPage, const void *pMain, const void *pSpare);
static FFXIOSTATUS SpareRead16(D_UINT32 ulPage, void *pSpare);
static FFXIOSTATUS SpareWrite16(D_UINT32 ulPage, const void *pSpare);
static FFXIOSTATUS TagRead16(D_UINT32 ulPage, void *pTags, unsigned nTagSize);
static FFXIOSTATUS TagWrite16(D_UINT32 ulPage, const void *pTags, unsigned nTagSize);
static FFXIOSTATUS GetPageStatus512(D_UINT32 ulPage);
static FFXIOSTATUS GetBlockStatus512(D_UINT32 ulPage, D_UINT8 bFbbType);
static FFXIOSTATUS SetBlockStatus512(D_UINT32 ulPage, D_UINT32 ulBlockStatus);

static NTMX31OPERATIONS Native512Ops =
{
    PageRead512Ecc,
    PageRead512NoEcc,
    PageWrite512Ecc,
    PageWrite512NoEcc,
    SpareRead16,
    SpareWrite16,
    TagRead16,
    TagWrite16,
    GetPageStatus512,
    GetBlockStatus512,
    SetBlockStatus512
};
#endif /* FFXCONF_NAND_512BNATIVE */

#if FFXCONF_NAND_2KBNATIVE
/*  Operations for flash with 2KB pages
*/
static FFXIOSTATUS PageRead2KEcc(D_UINT32 ulPage, void *pMain, void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageRead2KNoEcc(D_UINT32 ulPage, void *pMain, void *pSpare);
static FFXIOSTATUS PageWrite2KEcc(D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageWrite2KNoEcc(D_UINT32 ulPage, const void *pMain, const void *pSpare);
static FFXIOSTATUS SpareRead64(D_UINT32 ulPage, void *pSpare);
static FFXIOSTATUS SpareWrite64(D_UINT32 ulPage, const void *pSpare);
static FFXIOSTATUS TagRead64(D_UINT32 ulPage, void *pTags, unsigned nTagSize);
static FFXIOSTATUS TagWrite64(D_UINT32 ulPage, const void *pTags, unsigned nTagSize);
static FFXIOSTATUS GetPageStatus2K(D_UINT32 ulPage);

static NTMX31OPERATIONS Native2KOps =
{
    PageRead2KEcc,
    PageRead2KNoEcc,
    PageWrite2KEcc,
    PageWrite2KNoEcc,
    SpareRead64,
    SpareWrite64,
    TagRead64,
    TagWrite64,
    GetPageStatus2K,
    GetBlockStatus2K,
    SetBlockStatus2K
};
#endif /* FFXCONF_NAND_2KBNATIVE */


#if (FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
/*  Operations for NAND with 2KB pages, emulating 512B pages, and using software ECC
*/
static FFXIOSTATUS PageReadEmul512SwEcc(D_UINT32 ulPage, void *pMain, void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageReadEmul512NoEcc(D_UINT32 ulPage, void *pMain);
static FFXIOSTATUS PageWriteEmul512SwEcc(D_UINT32 ulPage, const void *pMain, const void *pTag, unsigned nTagSize);
static FFXIOSTATUS PageWriteEmul512NoEcc(D_UINT32 ulPage, const void *pMain);
static FFXIOSTATUS SpareReadEmul16SwEcc(D_UINT32 ulPage, void *pSpare);
static FFXIOSTATUS SpareWriteEmul16SwEcc(D_UINT32 ulPage, const void *pSpare);
static FFXIOSTATUS TagReadEmul16SwEcc(D_UINT32 ulPage, void *pTags, unsigned nTagSize);
static FFXIOSTATUS TagWriteEmul16SwEcc(D_UINT32 ulPage, const void *pTags, unsigned nTagSize);
static FFXIOSTATUS GetPageStatusEmul512SwEcc(D_UINT32 ulPage);

static NTMX31OPERATIONS Emulated512Ops =
{
    PageReadEmul512SwEcc,
    PageReadEmul512NoEcc,
    PageWriteEmul512SwEcc,
    PageWriteEmul512NoEcc,
    SpareReadEmul16SwEcc,
    SpareWriteEmul16SwEcc,
    TagReadEmul16SwEcc,
    TagWriteEmul16SwEcc,
    GetPageStatusEmul512SwEcc,
    GetBlockStatus2K,
    SetBlockStatus2K
};
#endif /* FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED */



#if FFXCONF_NAND_512BORNANDEMULATED
/*  Operations for NAND with 2KB pages, emulating 512B pages, and using software ECC
*/
static FFXIOSTATUS ORNANDPgRdEm512SwEcc(D_UINT32 ulPage, void *pMain, void *pTag, unsigned nTagSize);
static FFXIOSTATUS ORNANDPgRdEm512NoEcc(D_UINT32 ulPage, void *pMain);
static FFXIOSTATUS ORNANDSpRdEm16SwEcc(D_UINT32 ulPage, void *pSpare);


static NTMX31OPERATIONS ORNANDEmulated512Ops =
{
    ORNANDPgRdEm512SwEcc,
    ORNANDPgRdEm512NoEcc,
    PageWriteEmul512SwEcc,
    PageWriteEmul512NoEcc,
    ORNANDSpRdEm16SwEcc,
    SpareWriteEmul16SwEcc,
    TagReadEmul16SwEcc,
    TagWriteEmul16SwEcc,
    GetPageStatusEmul512SwEcc,
    GetBlockStatus2K,
    SetBlockStatus2K
};
#endif /* FFXCONF_NAND_512BORNANDEMULATED */

/*  This initialized static buffer is to be used in
    when the caller does not supply a spare buffer to
    NFCPageOut().

    This is a bad practice.  Static variables should almost never be
    used unless they are also declared as "const".  Also, why do we
    bother initializing this to ERASED8 when the code itself also
    initializes the buffer?
*/
static union
{
    D_BUFFER            data[SPARE_BUFFER_SIZE];
    DCL_ALIGNTYPE       DummyAlign;
} UnusedSpare = { { ERASED8, ERASED8, ERASED8, ERASED8,
                    ERASED8, ERASED8, ERASED8, ERASED8,
                    ERASED8, ERASED8, ERASED8, ERASED8,
                    ERASED8, ERASED8, ERASED8, ERASED8 } };

static NTMX31OPERATIONS * IdentifyFlash(NTMHANDLE hNTM);
static NTMX31OPERATIONS * ConfigController(PNANDCTL pNC, const FFXNANDCHIP *pChip);

static D_BOOL ReadStatus(unsigned int *puStatus);
static D_BOOL InitController(NTMHANDLE hNTM);
static void UninitController(void);
static D_BOOL IDIn(D_UCHAR aucID[NAND_ID_SIZE]);
static D_BOOL StatusIn(D_UCHAR *pucStatus);

#if (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BNATIVE)     /* not used for SW ECC */
static D_BOOL PageInHwEcc(void *pMain, void *pTag, unsigned nTagSize, FFXIOSTATUS *pStatus);
static D_BOOL PageOutHwEcc(const void *pMain, const void *pTag, unsigned nTagSize);
static void EncodeTagHwEcc(const D_BUFFER *pTag, unsigned nTagSize, D_BUFFER pSpare[SPARE_BUFFER_SIZE]);
static D_BOOL DecodeTagHwEcc(D_BUFFER *pTag, unsigned nTagSize, const D_BUFFER pSpare[SPARE_BUFFER_SIZE]);
#endif  /*(FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BNATIVE) */

#if (FFXCONF_NAND_512BEMULATED || FFXCONF_NAND_512BORNANDEMULATED)     /* not used for HW ECC */
static void EncodeTagSwEcc(const D_BUFFER *pTag, unsigned nTagSize, D_BUFFER pSpare[SPARE_BUFFER_SIZE]);
static D_BOOL DecodeTagSwEcc(D_BUFFER *pTag, unsigned nTagSize, const D_BUFFER pSpare[SPARE_BUFFER_SIZE]);
#endif  /*(FFXCONF_NAND_512BEMULATED || FFXCONF_NAND_512BORNANDEMULATED) */

static D_BOOL NFCPageIn(void *pMain, void *pSpare);
static D_BOOL NFCPageOut(const void *pMain, const void *pSpare);
static D_BOOL NFCSpareIn(void *pSpare);
static D_BOOL NFCSpareOut(const void *pSpare);
static D_BOOL NFCCommandOut(D_UCHAR uCommand);
static D_BOOL NFCAddressOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCRowOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCColumnOut(D_UINT32 ulAddress, unsigned uCount);
static D_BOOL NFCReadyWait(void);
static D_BOOL NFCWait(void);
static void SetEccMode(D_BOOL fUseEcc);

#if NFC_HAS_RESET
static void NFCReset(void);
#else
#define NFCReset()
#endif




                    /*-----------------------------*\
                     *                             *
                     *     External Interface      *
                     *                             *
                    \*-----------------------------*/


/*-------------------------------------------------------------------
    Public: Create()

    This function creates an NTM instance.  Additionally, it
    configures the corresponding hardware by calling the helper
    functions and project hooks.

    Parameters:
        hDev      - An opaque pointer to a device on which this
                    module is to begin operating.  This module uses
                    this handle to acquire per-device configuration
                    and to specify this particular device to some
                    helper functions provided from other modules.
        ppNtmInfo - Output, on return points to the address of the
                    structure containing information shared with the
                    NAND module.

    Return Value:
        Returns an NTMHANDLE, the contents of which are private to
        this module, and unique to the device specified by hDev.
-------------------------------------------------------------------*/
static NTMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const NTMINFO     **ppNtmInfo)
{
    NTMDATA            *pNTM = NULL;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-Create()\n"));

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if (pNTM)
    {
        pNTM->hDev = hDev;

        pNTM->pNC = FfxHookNTMX31Create(hDev);
        if(pNTM->pNC)
        {
            if (InitController(pNTM))
            {
                pNTM->pNANDOps = IdentifyFlash(pNTM);
                if (pNTM->pNANDOps)
                {
                    D_UINT32 ulTotalBlocks;

                    FFXFIMBOUNDS bounds;

                    /*  Get the array bounds now so we know how far to scan
                    */
                    FfxDevGetArrayBounds(hDev, &bounds);

                    /*  This controller only supports a single chip (has only
                        one chip select), so the total size is the size of the
                        chip.
                    */
                    ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                    /*  Fill in the NTMINFO appropriate to the chip that
                        was found.
                    */
                    pNTM->NtmInfo.uDeviceFlags  |= DEV_REQUIRES_ECC;

                    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
                        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
                    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
                        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

                    pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;
                    pNTM->NtmInfo.ulBlockSize   = pNTM->pChipInfo->pChipClass->ulBlockSize;
                  #if (FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
                    /*  Whether the physical page size is 2K or 512, we
                        want this to be small page
                    */
                    pNTM->NtmInfo.uPageSize = NAND512_PAGE;
                    pNTM->NtmInfo.uSpareSize = NAND512_SPARE;
                  #else
                    pNTM->NtmInfo.uPageSize     = pNTM->pChipInfo->pChipClass->uPageSize;
                    pNTM->NtmInfo.uSpareSize    = pNTM->pChipInfo->pChipClass->uSpareSize;
                  #endif
                    pNTM->NtmInfo.uMetaSize     = TAG_SIZE; /* covered by spare area ECC */
                    pNTM->NtmInfo.uAlignSize    = 1;        /* arbitrary buffer alignment allowed when reading/writing with this NTM */

                    if( pNTM->pChipInfo->pChipClass->uEdcRequirement > 1 )
                    {
                        /* This chip requires more correction capabilities
                           than this NTM can handle - fail the create.
                        */
                        FFXPRINTF(1, ("NTMX31-Create() Insufficient EDC capabilities.\n"));
                        goto CreateCleanup;
                    }

                    /*  MX31 controller is capable of correcting 1 bit per
                        segment. Assign and verify it so:
                    */
                    
                    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
                    pNTM->NtmInfo.uEdcCapability     = 1;
                    pNTM->NtmInfo.uEdcSegmentSize    = NAND512_PAGE; /* EDC on 512-byte segments */
                    DclAssert(pNTM->NtmInfo.uEdcRequirement <= pNTM->NtmInfo.uEdcCapability);
                    
                    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
                    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

                    /*  Calculate this once and store it for use later for
                        both optimization and simplification of the code
                    */
                    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
                    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
                    pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

                    DclAssert((pNTM->NtmInfo.ulBlockSize % 1024UL) == 0);

                    pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                    /*  Initialize the spare area buffer used when
                        the caller does not supply their own.
                    */
                    DclMemSet(UnusedSpare.data, ERASED8, sizeof UnusedSpare.data);

                    /* Successfully created the NTM instance.
                    */
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                                    "NTMX31-Create() returning %P\n", pNTM));
                    *ppNtmInfo = &pNTM->NtmInfo;
                    return pNTM;
                }

  CreateCleanup:

                UninitController();
            }
            FfxHookNTMX31Destroy(pNTM->pNC);
        }
        DclMemFree(pNTM);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    "NTMX31-Create() failed\n"));

    return NULL;
}

/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Read the ID information from the NAND chip, and make sure it
    is a chip suitable for use.

    Parameters:
        hNTM - The handle for the NTM instance with which to
               identify the flash.

    Return Value:
        A vector containing functions to operate the flash using
        this controller, if the chip is one that this NTM knows how
        to operate, NULL if not.
-------------------------------------------------------------------*/
static NTMX31OPERATIONS * IdentifyFlash(NTMHANDLE hNTM)
{
    D_UCHAR abID[NAND_ID_SIZE];   /*  Manufacturer and device IDs, plus chip-
                                      specific data
                                  */

    /*  Read the manufacturer and device IDs from the flash.
    */
    if (NFCCommandOut(CMD_READ_ID)
        && NFCRowOut(0, 1)
        && IDIn(abID))
    {
        /*  Look up the chip
        */
        hNTM->pChipInfo = FfxNandDecodeID(abID);

        if (hNTM->pChipInfo)
        {
            /*  Ensure that the flash that was found is compatible with the
                spare area format supported by the MX31 and NTM with regard
                to the factory bad block marks
            */
            if ((hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC
                || (hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO)
            {
                /* Configure the controller for the flash.
                */
                return (ConfigController(hNTM->pNC, hNTM->pChipInfo));
            }

            FFXPRINTF(1, ("NTMX31:IdentifyFlash() Factory bad block marker method incompatible with this NTM.\n"));
        }

    }
    return NULL;
}


/*-------------------------------------------------------------------
    Public: Destroy()

    This function destroys an NTM instance, and releases any
    allocated resources.

    Parameters:
        hNTM - The handle for the NTM instance to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void Destroy(
    NTMHANDLE           hNTM)
{
    DclAssert(hNTM);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NTMX31-Destroy(%P)\n", hNTM));

    UninitController();
    FfxHookNTMX31Destroy(hNTM->pNC);
    DclMemFree(hNTM);
}


/*-------------------------------------------------------------------
    Public: PageRead()

    Read one or more pages of main page data, and optionally read
    the corresponding tag data from the spare area.

    The fUseEcc flag controls whether ECC-based error detection
    and correction are performed.  Note that the "raw" page read
    functionality (if implemented) is the preferred interface for
    reading without ECC protection.  If fUseEcc is FALSE, this
    function will not read tag data.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The flash offset in pages, relative to any
                   reserved space.
        pPages   - A buffer to receive the main page data
        pTags    - A buffer to receive the tag data.  Must be NULL
                   fUseEcc is FALSE.
        ulCount  - Number of pages to read.  The range of pages
                   must not cross an erase block boundary.
        nTagSize - The tag size to use.  Must be 0 if fUseEcc is
                   FALSE.
        fUseEcc  - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pPages array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-PageRead() Start Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTMX31-PageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTags && nTagSize == TAG_SIZE) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
        above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    while (ulCount)
    {
        FFXIOSTATUS status;

        if (fUseEcc)
        {
            status = hNTM->pNANDOps->PageReadEcc(ulStartPage, pPages, pTags, nTagSize);
        }
        else
        {
            DclAssert(pTags == NULL);
            DclAssert(nTagSize == 0);
            status = hNTM->pNANDOps->PageReadNoEcc(ulStartPage, pPages, NULL);
        }

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  PAGESTATUS_DATACORRECTED could be set in ulPageStatus. Break out
            if correctable error detected.
        */
        ioStat.op.ulPageStatus |= status.op.ulPageStatus;
        if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            ioStat.ulCount++;
            break;
        }

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulStartPage += status.ulCount;
        if (pTags)
            pTags += status.ulCount * nTagSize;
    }

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: PageWrite()

    Write one or more logical pages of main array data, along with
    optional tag data.

    If fUseEcc is TRUE, an ECC for the main page data is generated
    and written in the spare area, along with the optional tag.  If
    fUseEcc is FALSE, no ECC and no tag is written.  The spare area
    includes a record of the state of fUseEcc.  Note that the "raw"
    page write functionality (if implemented) is the preferred
    interface for writing data without ECC.

    Parameters:
        hNTM        - The NTM handle to use.
        ulStartPage - The starting page to write, relative to any
                      reserved space.
        pPages      - A pointer to the main page data to write.
        pTags       - A buffer containing the optional tag data,
                      so long as nTagSize is valid, and fUseEcc is
                      TRUE.  Must be NULL otherwise.
        ulCount     - Number of pages to write.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if pTags is
                      NULL or fUseEcc is FALSE.
        fUseEcc     - If TRUE, write the data with ECC.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the operation
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-PageWrite() StartPage=%lX Count=%lU TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, nTagSize, fUseEcc));

    DclProfilerEnter("NTMX31-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert((D_UINTPTR)pPages % 4 == 0);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    while (ulCount)
    {
        FFXIOSTATUS status;

        if (fUseEcc)
        {
            status = hNTM->pNANDOps->PageWriteEcc(ulStartPage, pPages, pTags, nTagSize);
        }
        else
        {
            DclAssert(!pTags);
            DclAssert(!nTagSize);
            status = hNTM->pNANDOps->PageWriteNoEcc(ulStartPage, pPages, NULL);
        }

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulStartPage += status.ulCount;
        if(nTagSize)
            pTags += status.ulCount * nTagSize;
    }

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: RawPageRead()

    Read an exact image of the data on the media with no
    processing of ECCs or interpretation of the contents of
    the spare area.

    Reading the spare area is optional.

    Not all NTMs support this function; not all NAND controllers
    can support it.

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to
                  any reserved space.
        pPages  - A buffer to receive the main page data.
        pSpares - A buffer to receive the spare area data.
                  May be NULL.
        ulCount - The number of pages to read.  The range of pages
                  must not cross an erase block boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pPages array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS RawPageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-RawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTMX31-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->PageReadNoEcc(ulPage, pPages, pSpares);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulPage += status.ulCount;
        if (pSpares)
            pSpares += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: RawPageWrite()

    Write data to the entire page (main and spare area) with no
    ECC processing or other interpretation or formatting.

    Not all NTMs support this function; not all NAND controllers
    can support it.

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to
                  any reserved space.
        pPages  - A pointer to the main page data to write.
        pSpares - A pointer to data to write in the spare area.
        ulCount - The number of pages to write.  The range of
                  pages must not cross an erase block boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS RawPageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTMX31-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->PageWriteNoEcc(ulPage, pPages, pSpares);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pPages += status.ulCount * hNTM->NtmInfo.uPageSize;
        ulPage += status.ulCount;
        if (pSpares)
            pSpares += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareRead()

    Read the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to the buffer to fill.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of spares which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
                    "NTMX31-SpareRead() Page=%lX\n", ulPage));

    DclAssert((D_UINTPTR) pSpare % 4 == 0);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->SpareRead(ulPage, pSpare);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pSpare += status.ulCount * hNTM->NtmInfo.uSpareSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    "NTMX31-SpareRead() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareWrite()

    Write the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to the spare area data to write.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of spares which were completely written.
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-SpareWrite() Page=%lX\n", ulPage));

    DclAssert((D_UINTPTR) pSpare % 4 == 0);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->SpareWrite(ulPage, pSpare);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pSpare += status.ulCount * hNTM->NtmInfo.uSpareSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenRead()

    Read the hidden (tag) data from multiple pages.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, relative to any
                      reserved space.
        ulCount     - The number of tags to read.
        pTags       - Buffer for the hidden data read.
        nTagSize    - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-HiddenRead() Start Page=%lX Count=%X TagSize=%u\n", ulStartPage, ulCount, nTagSize));

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    while (ulCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->TagRead(ulStartPage, pTags, nTagSize);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  PAGESTATUS_DATACORRECTED could be set in ulPageStatus. Break out
            if correctable error detected.
        */
        ioStat.op.ulPageStatus |= status.op.ulPageStatus;
        if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            ioStat.ulCount++;
            break;
        }

        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount -= status.ulCount;
        ioStat.ulCount += status.ulCount;
        pTags += status.ulCount * nTagSize;
        ulStartPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    "NTMX31-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenWrite()

    Write the hidden (tag) data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The page offset in bytes, relative to any
                   reserved space.
        pTag     - Buffer for the hidden data to write.
        nTagSize - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely written.
        The status indicates whether the write was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    "NTMX31-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n",
                    ulPage, *(D_UINT16*)pTag, nTagSize));

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    while (uCount)
    {
        FFXIOSTATUS status;

        status = hNTM->pNANDOps->TagWrite(ulPage, pTag, nTagSize);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful write, account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ioStat.ulCount += status.ulCount;
        pTag += status.ulCount * nTagSize;
        ulPage += status.ulCount;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    "NTMX31-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: BlockErase()

    This function erases a block (physical erase zone) of a
    NAND chip.

    Parameters:
        hNTM    - The NTM handle
        ulBlock - The flash offset in blocks, relative to any
                  reserved space.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks which were completely erased.
        The status indicates whether the erase was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS BlockErase(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;
    unsigned int        uCount = 1; /* make this a parameter someday */
    D_UINT32            ulPhysicalPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NTMX31-BlockErase() Block=%lX\n", ulBlock));

    while (uCount)
    {
        FFXIOSTATUS status;

        ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

        status = EraseBlock(ulPhysicalPage);

        ioStat.ffxStat = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /* The block was erased successfully.  Account for it.
        */
        DclAssert(status.ulCount <= uCount);
        uCount -= (D_UINT16) status.ulCount;
        ulBlock += status.ulCount;
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage)
{
    FFXIOSTATUS         ioStat;

    DclAssert(hNTM);

    ioStat = hNTM->pNANDOps->GetPageStatus(ulPage);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NTMX31-GetPageStatus() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetBlockStatus()

    This function retrieves the block status information for the
    given erase block.

    Parameters:
        hNTM    - The NTM handle to use
        ulBlock - The flash offset, in blocks, relative to any
                  reserved space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetBlockStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock)
{
    FFXIOSTATUS     ioStat;
    int i;
    D_UINT32        ulPhysicalPage;

    DclAssert(hNTM);

    /*  Ensure that the flash in use is compatible with this function
    */
    DclAssert((hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC
              || (hNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO);

    /*  Convert to PHYSICAL page size

        Note!  This is NOT the emulated page size.
    */
    ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    /*  Check the bad block indicator in the first two pages of the block
        only.
    */
    for (i = 0; i < 2; ++i)
    {
        ioStat = hNTM->pNANDOps->GetBlockStatus(ulPhysicalPage, (D_UINT8)(hNTM->pChipInfo->bFlags & CHIPFBB_MASK));
        if (ioStat.op.ulBlockStatus != BLOCKSTATUS_NOTBAD)
        {
            break;
        }
        ulPhysicalPage++;
    }
    ioStat.ulFlags = (ioStat.ulFlags & ~IOFLAGS_TYPEMASK) | IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NTMX31-GetBlockStatus() Block=%lX returning %s\n",
                    ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SetBlockStatus()

    This function sets the block status information for the
    given erase block.

    Parameters:
        hNTM          - The NTM handle to use
        ulBlock       - The flash offset, in blocks, relative to any
                        reserved space.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;
    D_UINT32        ulPhysicalPage;

    DclAssert(hNTM);

    /*  Convert to PHYSICAL page size

        Note!  This is NOT the emulated page size.
    */
    ulPhysicalPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    ioStat = hNTM->pNANDOps->SetBlockStatus(ulPhysicalPage, ulBlockStatus);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NTMX31-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
                    ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    This function implements a generic IORequest handler at the NTM
    level.

    Parameters:
        hNTM      - The NTM handle.
        pIOR      - A pointer to the FFXIOREQUEST structure to use.

    Returns:
        Returns a standard FFXIOSTATUS structure.  The status code
        FFXSTAT_FIM_UNSUPPORTEDIOREQUEST is specifically returned
        for those requests which are not supported.
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    NTMHANDLE           hNTM,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = NULL_IO_STATUS;

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-IORequest() hNTM=%P Func=%x\n", hNTM, pIOR->ioFunc));

    DclProfilerEnter(NTMNAME"-IORequest", 0, 0);

    DclAssert(hNTM);

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_FIM_LOCK_FREEZE:
        {
            FFXIOR_FIM_LOCK_FREEZE *pReq = (FFXIOR_FIM_LOCK_FREEZE*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_FREEZE() (STUBBED!)\n"));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "lock-freeze" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

        case FXIOFUNC_FIM_LOCK_BLOCKS:
        {
            FFXIOR_FIM_LOCK_BLOCKS *pReq = (FFXIOR_FIM_LOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_BLOCKS() StartBlock=%lU Count-%lU (STUBBED!)\n",
                pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "lock-block" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

        case FXIOFUNC_FIM_UNLOCK_BLOCKS:
        {
            FFXIOR_FIM_UNLOCK_BLOCKS *pReq = (FFXIOR_FIM_UNLOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_UNLOCK_BLOCKS() StartBlock=%lU Count-%lU (STUBBED!)\n",
                pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "unlock-block" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_BLOCK;

            break;
        }

      #if FFXCONF_POWERSUSPENDRESUME
        case FXIOFUNC_FIM_POWER_SUSPEND:
        {
            FFXIOR_FIM_POWER_SUSPEND   *pReq = (FFXIOR_FIM_POWER_SUSPEND*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_SUSPEND() PowerState=%u (STUBBED!)\n", pReq->nPowerState));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "power-suspend" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }

        case FXIOFUNC_FIM_POWER_RESUME:
        {
            FFXIOR_FIM_POWER_RESUME    *pReq = (FFXIOR_FIM_POWER_RESUME*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_RESUME() PowerState=%u (STUBBED!)\n", pReq->nPowerState));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            /*  Add code here to implement "power-resume" support...
            */

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }
      #endif

        default:
        {
            FFXPRINTF(3, (NTMNAME"-IORequest() Unsupported function %x\n", pIOR->ioFunc));

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;

            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/******************************************************************************
*******************************************************************************
    Physical NAND Operations functions.

    This implementation reflects the fact that the Freescale chips
    incorporating these related controllers all have exactly one controller.

    Some machine dependencies (like number of bits in an int) are
    allowed, as this is machine-dependent code.

*******************************************************************************
******************************************************************************/


/*  State information about the NAND Flash Controller (NFC.  Since the i.MX31
    has exactly one of these, the single structure may be allocated statically
    and referenced directly (no need to pass a pointer around).
*/
static struct
{
    void *pBase;
    unsigned int uConfig1;
    D_UCHAR ucWidthBytes;
    NTMDATA *pNTM;
    D_BOOL fEccEnabled;
    unsigned uAddrsBytes;   /* page # and up (not offset in page) */
    unsigned fResetBeforeProgram : 1;

} NFC;

typedef D_UINT32 MainBuffer[MAIN_BUFFER_SIZE / sizeof (D_UINT32)];
typedef D_UINT32 SpareBuffer[SPARE_BUFFER_SIZE / sizeof (D_UINT32)];

#define NFC_MAIN_BUFFER ((MainBuffer *) ((char *) NFC.pBase))
#define NFC_SPARE_BUFFER ((SpareBuffer *) ((char *) NFC.pBase + 0x800))

/*  Only one of the four possible 512+16-byte buffer areas is used.
*/
#define XFER_BUFFER_NUMBER 3

/* Controller registers and bit fields
*/
#define NFCREG(offset)   ((D_UINT16 *) ((char *) NFC.pBase + 0xE00 + offset))


#define NFC_BUFSIZE            NFCREG(0x00)
#define BLOCK_ADD_LOCK         NFCREG(0x02)
#define RAM_BUFFER_ADDRESS     NFCREG(0x04)
#define NAND_FLASH_ADD         NFCREG(0x06)
#define NAND_FLASH_CMD         NFCREG(0x08)
#define NFC_CONFIGURATION      NFCREG(0x0A)
#define ECC_STATUS_RESULT      NFCREG(0x0C)
#define ECC_SPARE_CORR           (1U << 0)
#define ECC_SPARE_UNCORR         (1U << 1)
#define ECC_MAIN_CORR            (1U << 2)
#define ECC_MAIN_UNCORR          (1U << 3)
#define ECC_STATUS_MAIN          (ECC_MAIN_CORR | ECC_MAIN_UNCORR)
#define ECC_STATUS_SPARE         (ECC_SPARE_CORR | ECC_SPARE_UNCORR)
#define ECC_RSLT_MAIN_AREA     NFCREG(0x0E)
#define ECC_RSLT_SPARE_AREA    NFCREG(0x10)
#define NF_WR_PROT             NFCREG(0x12)
#define WRPROT_UNLOCK            (4)
#define WRPROT_LOCK              (2)
#define WRPROT_LOCKTIGHT         (1)
#define UNLOCK_START_BLK_ADD   NFCREG(0x14)
#define UNLOCK_END_BLK_ADD     NFCREG(0x16)
#define NAND_FLASH_WR_PR_ST    NFCREG(0x18)
#define NAND_FLASH_CONFIG1     NFCREG(0x1A)
#define NFC_RST                  (1U << 6)
#define NF_BIG                   (1U << 5)
#define INT_MASK                 (1U << 4)
#define ECC_EN                   (1U << 3)
#define SP_EN                    (1U << 2)
#define NAND_FLASH_CONFIG2     NFCREG(0x1C)
#define INT                      (1U << 15)
#define FDO_PAGE                 (1U <<  3)
#define FDO_ID                   (2U <<  3)
#define FDO_STATUS               (4U <<  3)
#define FDI                      (1U <<  2)
#define FADD                     (1U <<  1)
#define FCMD                     (1U <<  0)



/*-------------------------------------------------------------------
    Local: ProcessChipStatus()

    Given a status value read from a NAND chip, interpret
    its value into a FlashFX status.

    Parameters:
        uChipStatus - status read from the NAND chip

    Return Value:
        FFXIOSTATUS indicating the status of the NAND operation.
-------------------------------------------------------------------*/
static FFXSTATUS ProcessChipStatus(unsigned int uChipStatus)
{
    FFXSTATUS ffxStat;

    if ((uChipStatus & (NANDSTAT_WRITE_ENABLE | NANDSTAT_READY | NANDSTAT_ERROR))
        == (NANDSTAT_WRITE_ENABLE | NANDSTAT_READY))
    {
        ffxStat = FFXSTAT_SUCCESS;
    }
    else
    {
        if (!(uChipStatus & NANDSTAT_READY))
        {
            DclError();
            ffxStat = FFXSTAT_FIMTIMEOUT;
        }
        else if (!(uChipStatus & NANDSTAT_WRITE_ENABLE))
        {
            ffxStat = FFXSTAT_FIM_WRITEPROTECTEDBLOCK;
        }
        else
        {
            ffxStat = FFXSTAT_FIMIOERROR;
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: EraseBlock()

    Erase one physical erase block.

    Parameters:
        ulPhysicalPage - physical (never emulated) page number of
                         the first page in the block to erase

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS EraseBlock(
    D_UINT32 ulPhysicalPage)
{
    FFXIOSTATUS status = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    if (NFCCommandOut(CMD_ERASE_BLOCK)
        && NFCRowOut(ulPhysicalPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_ERASE_BLOCK_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("Erase: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


#if (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
/*-------------------------------------------------------------------
    Local: SpareRead64Native()

    Perform an entire spare area Read command for large-block NAND,
    returning the NATIVE 2K page spare area (that which contains
    the factory bad block mark) which is distinctly different from
    the LOGICAL 2K page spare area as enforced by the MX31 control-
    ler implementation, EXCEPT for the last 16 bytes, which is the
    only region that we use for FlashFX Info with the exception of
    a status byte indicating ECC is in use in each of the other
    16 byte regions.

    Parameters:
        ulPhysicalPage - physical page number of the page containing
                         the spare area data to be read
        pSpare         - 64-byte buffer to receive spare area data, aligned
                         on a 4-byte boundary.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead64Native(
    D_UINT32 ulPhysicalPage,
    D_BUFFER *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.

        NOTE! When using this function for emulation,
        the ECC may not have been on to begin with!
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(NAND2K_PAGE, 2)
        && NFCRowOut(ulPhysicalPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(pSpare)
        && NFCSpareIn((D_UCHAR *)pSpare + SPARE_BUFFER_SIZE)
        && NFCSpareIn((D_UCHAR *)pSpare + (SPARE_BUFFER_SIZE * 2))
        && NFCSpareIn((D_UCHAR *)pSpare + (SPARE_BUFFER_SIZE * 3)))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}

/*-------------------------------------------------------------------
    Local: SpareWrite64Native()

    Perform an entire spare area write operation for large-block NAND,
    programming the NATIVE 2K page spare area (that which contains
    the factory bad block mark) which is distinctly different from
    the LOGICAL 2K page spare area as enforced by the MX31 control-
    ler implementation, EXCEPT for the last 16 bytes, which is the
    only region that we use for FlashFX Info with the exception of
    a status byte indicating ECC is in use in each of the other
    16 byte regions.

    Parameters:
        ulPhysicalPage - physical page number of the page containing
                         the spare area data to be read
        pSpare         - 64-byte buffer containing spare area data, aligned
                         on a 4-byte boundary.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite64Native(
    D_UINT32 ulPhysicalPage,
    D_BUFFER *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.

        NOTE! When using this function for emulation,
        the ECC may not have been on to begin with!
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(NAND2K_PAGE, 2)     /* column address of 64 bytes of native spare area */
        && NFCRowOut(ulPhysicalPage, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCSpareOut((D_BUFFER *)pSpare + SPARE_BUFFER_SIZE)
        && NFCSpareOut((D_BUFFER *)pSpare + 2 * SPARE_BUFFER_SIZE)
        && NFCSpareOut((D_BUFFER *)pSpare + 3 * SPARE_BUFFER_SIZE)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("SpareWrite64Native: error status %02x\n", uChipStatus));
        }
    }

    return status;
}
#endif  /* (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED) */


#if FFXCONF_NAND_512BNATIVE
/*-------------------------------------------------------------------
    Local: PageRead512Ecc()

    Perform an entire Read command for small-block NAND.

    Parameters:
        ulPage - page to read
        pMain  - 512-byte buffer to receive main page data.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead512Ecc(
    D_UINT32    ulPage,
    void       *pMain,
    void       *pTag,
    unsigned    nTagSize)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    /*  Turn the controller's ECC processing on
    */
    SetEccMode(TRUE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && PageInHwEcc(pMain, pTag, nTagSize, &status))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageRead512NoEcc()

    Perform an entire Read command for small-block NAND.

    Parameters:
        ulPage - page to read
        pMain  - 512-byte buffer to receive main page data.
        pSpare - 16-byte buffer to receive spare area data, may
                 be NULL.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead512NoEcc(
    D_UINT32 ulPage,
    void *pMain,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCPageIn(pMain, pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWrite512Ecc()

    Perform an entire Page Write command for small-block NAND.

    Parameters:
        ulPage - page to write
        pMain  - 512-byte buffer containing main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite512Ecc(
    D_UINT32    ulPage,
    const void *pMain,
    const void *pTag,
    unsigned    nTagSize)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned    uChipStatus;

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && PageOutHwEcc(pMain, pTag, nTagSize)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWrite512Ecc: error status %02x\n", uChipStatus));
        }
    }
    return status;
}


/*-------------------------------------------------------------------
    Local: PageWrite512NoEcc()

    Perform an entire Page Write command for small-block NAND.

    Parameters:
        ulPage - Page to write.
        pMain  - 512-byte buffer containing main page data, aligned
                 on a 4-byte boundary.
        pSpare - 16-byte buffer containing spare area data, may
                 be NULL.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite512NoEcc(
    D_UINT32 ulPage,
    const void *pMain,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCPageOut(pMain, pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWrite512NoEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}

/*-------------------------------------------------------------------
    Local: SpareRead16()

    Perform an entire Read command for small-block NAND.

    Parameters:
        ulPage - the page containing the spare area
        pSpare - buffer to receive the spare area data

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead16(
    D_UINT32 ulPage,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_SPARE)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareIn(pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: SpareWrite16()

    Parameters:
        ulPage - the page containing the spare area
        pSpare - 16-byte buffer with data to be written to the spare
                 area, aligned on a 4-byte boundary.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite16(
    D_UINT32 ulPage,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_READ_SPARE)
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("SpareWrite16: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagRead16()

    An uncorrectable ECC error or bad check byte causes an
    all-zero (invalid) tag to be returned, but still returns a
    value indicating success.

    Parameters:
        ulPage - the page containing the tag
        pTags  - buffer to receive the tags

    Return Value:
        Should tag reads ever fail?  Yes, if timeout occurs then something
        is Horribly Wrong.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagRead16(
    D_UINT32            ulPage,
    void               *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclAssert(pTags);
    DclAssert(nTagSize == TAG_SIZE);

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    if (NFCCommandOut(CMD_READ_SPARE)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareIn(spare.data))
    {
        /*  Check for an ECC error in the spare area.

            BOGUS!  Cribbed (with minor modifications) from PageInHwEcc(), this
            probably should be wrapped up as a function.  It's inappropriate
            here to be accessing the NFC directly.
         */
        unsigned int uEccStatus = DCLMEMGET16(ECC_STATUS_RESULT);

        switch (uEccStatus & ECC_STATUS_SPARE)
        {
            case ECC_SPARE_CORR:
                /*  A correctable error occurred in the spare area.  The
                    controller has already corrected the data.
                */
                FFXPRINTF(1, ("Correctable error in tag: page %lu, location %04X\n",
                              ulPage, DCLMEMGET16(ECC_RSLT_SPARE_AREA)));
                    FfxErrMgrEncodeCorrectedBits(1, &status);
                    status.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                /* fall through */

            case 0:
                /*  DecodeTagHwEcc() takes care of an invalid tag (bad check byte).
                */
                DecodeTagHwEcc(pTags, nTagSize, spare.data);
                break;

            default:
                DclError();
                /* fall through */

            case ECC_SPARE_UNCORR:
                /*  An uncorrectable error in the tag causes an invalid (all
                    zero) tag to be returned.

                    BOGUS!  Is there (or should there be) a way to show the
                    tag was uncorrectable and thus invalidated while still
                    returning success status for good main page data?
                */
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                                "Uncorrectable error in tag: page %lu\n", ulPage));
                DclMemSet(pTags, 0, nTagSize);
                break;
        }
        status.ffxStat = FFXSTAT_SUCCESS;
        status.ulCount = 1;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagWrite16()

    Write tags to small-block NAND

    Parameters:
        ulPage - the page to contain the tag
        pTag   - buffer contain the tag

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS TagWrite16(
    D_UINT32            ulPage,
    const void         *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    unsigned            uChipStatus;

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
        EncodeTagHwEcc(pTag, nTagSize, spare.data);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_READ_SPARE)
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 1)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(spare.data)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("TagWrite16: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: GetPageStatus512()

    This function retrieves the page status information for the
    given page.

    Parameters:
        ulPage - The flash page from which to retrieve the status.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatus512(
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    ioStat = SpareRead16(ulPage, spare.data);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_NONE))
            ioStat.op.ulPageStatus = PAGESTATUS_UNWRITTEN;   /* blank/no ECC */
        else if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
            ioStat.op.ulPageStatus = PAGESTATUS_WRITTENWITHECC;
        else
            ioStat.op.ulPageStatus = PAGESTATUS_UNKNOWN;   /* indecipherable */

        /*  If the tag area, including the check byte, is within 1 bit of
            being erased, then we know that a tag was not written.  If it
            is anything else, we know a tag was written.
        */

        /*  Move the tag check byte so it immediately follows the
            tag data (depending on byte order it might already
            be there).
        */
        spare.data[SPARE_INDEX_HWECC_TAG+TAG_SIZE] = spare.data[SPARE_INDEX_HWECC_TAGCHECK];

        if(!FfxNtmHelpIsRangeErased1Bit(&spare.data[SPARE_INDEX_HWECC_TAG], TAG_SIZE+1))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(TAG_SIZE);
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: GetBlockStatus512()

    This function retrieves the block status information for the
    given erase block.

    Parameters:
        ulPage   - The page whose block contains the status to return
        bFbbType - the factory bad block type for the flash

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetBlockStatus512(
    D_UINT32        ulPage,
    D_UINT8         bFbbType)
{
    FFXIOSTATUS     ioStat;
    DCLALIGNEDBUFFER (spare, data, SPARE_BUFFER_SIZE);

    do
    {

        ioStat = SpareRead16(ulPage, spare.data);
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
              break;

        /*  Err on the side of declaring a block factory bad: it's unfortunate
            if a block is declared bad when it's actually good, but it's worse
            if a factory bad block gets erased.  This means (among other
            things) sometimes ignoring the possibility that a single-bit error
            could cause a good block to appear to be bad.

            There are two types of factory bad block marks that are understood
            by this NTM: CHIPFBB_SSFDC and CHIPFBB_OFFSETZERO.  All 8-bit
            small-block NAND parts supported by this NTM use the SSFDC
            format: any zero bit in byte offset 5 in the spare area of either
            of the first two pages of a block indicates the block is bad.
            All supported 16-bit and large-block NAND parts use byte or word
            offset zero.

            Reserving offset zero as a bad block mark is a problem for this
            NTM, as only the first three bytes or two words are covered by the
            spare area ECC, so the tag must be placed there.  Once a tag has
            been written, it becomes much harder to identify factory bad
            blocks.

            Two techniques are used to improve the reliability of bad block
            detection on these parts.  First, some (not all) 16-bit NAND chips
            specify that word offset 5 (byte offsets 10 and 11) is a also a
            bad block marker.  This NTM avoids ever writing to byte offset 11
            (offset 10 is used by the controller) so it is safe to check it.
            Second, when a tag is written, another location is used to record
            that; if that special mark is present, any zero bits in the byte
            or word at offset zero are assumed to be a tag, not a bad block
            mark.

            To recognize factory bad blocks, check the primary bad block
            indicator at byte offset 5 first.  This is the factory bad block
            location in the SSFDC format and thus is already marked on these
            chips and doesn't need to be written by the NTM.  On offset zero
            chips, this location has no other meaning, and is programmed by
            the NTM to mark a bad block.

            If the primary bad block indicator contains no zero bits, check
            the secondary bad block indicator at byte offset 11.  This is never
            programmed by the NTM, but is part of the bad block marker at word offset
            5 in 16-bit flash.

            If neither of these locations indicates the block is bad, the
            offset zero factory bad block mark is checked.  When reformatting
            used flash this location is likely to have been programmed with a
            tag, so an additional check is made of the tag status at byte
            offset 3.  If this indicates a valid tag was programmed, the block
            is good; if not, it is bad.  A single-bit error in the tag status
            is accounted for.
        */

        /*  Allow a single-bit error in either of the block status bytes, but
            not in both.

            Note we check this for ALL FBB marker types (both SSFDC and OFFSETZERO),
            because this is how we mark blocks bad in SetBlockStatus() (including for
            OFFSETZERO type flash).
        */
        if (!CORRECTABLE(spare.data[SPARE_INDEX_BLOCKSTATUS1], ERASED8)
            || !CORRECTABLE(spare.data[SPARE_INDEX_BLOCKSTATUS2], ERASED8)
            || (spare.data[SPARE_INDEX_BLOCKSTATUS1] != ERASED8
                && spare.data[SPARE_INDEX_BLOCKSTATUS2] != ERASED8))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            break;
        }

        /*  Only check the OFFSETZERO if indeed this is a chip that uses that
            factory bad block marker type.  It's OK to check them all (because
            of the FBBSTATUS_FBB_INVALID marker), but explicitly checking here allows
            this code to be compatible with earlier versions of this NTM which
            did NOT write FBBSTATUS_FBB_INVALID at SPARE_INDEX_FBBSTATUS.
            See BZ/IR #866
        */
        if(bFbbType == CHIPFBB_OFFSETZERO)
        {
            /*  If there are any zero bits in the first two bytes (word offset 0),
                and the tag status byte does not indicate that data (or a tag) was
                written, the block is bad.
            */
            if ((spare.data[SPARE_INDEX_HWECC_TAG] != ERASED8
                 || spare.data[SPARE_INDEX_HWECC_TAG + 1] != ERASED8)
                && !CORRECTABLE(spare.data[SPARE_INDEX_FBBSTATUS], FBBSTATUS_FBB_INVALID))
            {
                ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                break;
            }
        }

        /*  All checks are okay, the block is good.
        */
        ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

    } while(FALSE);

#if D_DEBUG
    if (ioStat.ffxStat == FFXSTAT_SUCCESS
        && (ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD)
    {
        D_UINT16 *puSpare = (D_UINT16 *) spare.data;
        D_UINT32 *pulSpare = (D_UINT32 *) spare.data;

        DclPrintf("Bad block status %02lX at page %08lX contains:\n",
                  ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE, ulPage);
        DclPrintf("    %04X %04X %04X %04X %04X %04X %04X %04X\n",
                  puSpare[0], puSpare[1], puSpare[2], puSpare[3],
                  puSpare[4], puSpare[5], puSpare[6], puSpare[7]);
        DclPrintf("    %08lX %08lX %08lX %08lX\n",
                  pulSpare[0], pulSpare[1], pulSpare[2], pulSpare[3]);
    }
#endif

    return ioStat;
}


/*-------------------------------------------------------------------
    Local SetBlockStatus512()

    This function sets the block status information for the
    given erase block.

    Parameters:
        ulPage        - The first page in the erase block to be marked.
                        The offset is relative to any reserved space.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus512(
    D_UINT32        ulPage,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    /*  This should only be done for factory bad blocks during formatting.
        There is no need for any other kind of marking.
    */
    switch (ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        case BLOCKSTATUS_FACTORYBAD:
        {
            DCLALIGNEDBUFFER (spare, data, SPARE_BUFFER_SIZE);
            int i;

            DclMemSet(spare.data, ERASED8, sizeof spare.data);

            /*  We're going to update three locations:

                1.  SSFDC-style factory bad block mark.  If we successfully program this,
                    GetBlockStatus will always treat this is a factory bad block, which
                    is exactly what we want.  This location is not overwritten by
                    the MX31 NAND controller.

                2.  Our FBB status indicator.  If we successfully program this (or it
                    was erased prior to this function being called), GetBlockStatus
                    will be able to determine that the Offsetzero factory bad block
                    mark is indeed intact.

                3.  The offset-zero style factory bad block mark location.  It doesn't do
                    any harm to program this on SSFDC-style flash, and IF we manage to
                    program the FBB status indicator, this will allow GetBlockStatus
                    to determine this is a bad block mark.
            */

            spare.data[SPARE_INDEX_BLOCKSTATUS1] = 0;
            spare.data[SPARE_INDEX_HWECC_TAG] = 0;
            spare.data[SPARE_INDEX_FBBSTATUS] = 0;

            /*  Try to program the bad block indicator in at least one of the first
                two pages of the block.
            */
            for (i = 0; i < 2; ++i)
            {
                ioStat = SpareWrite16(ulPage, spare.data);
                if (ioStat.ffxStat == FFXSTAT_SUCCESS)
                    break;

                ulPage++;
            }
        }

        default:
            /*  Unexpected block status values are an error.
            */
            DclError();
            ioStat.ffxStat = FFXSTAT_BADPARAMETER;
            break;

    }

    return ioStat;
}

#endif /* FFXCONF_NAND_512BNATIVE */


#if FFXCONF_NAND_2KBNATIVE

/*  These are the column addresses of each 16 byte portion of the 64 byte spare
    area for a 2KB page.
*/
#define MX31_SPARE_COLUMN0 (1*NAND512_PAGE + 0*NAND512_SPARE)
#define MX31_SPARE_COLUMN1 (2*NAND512_PAGE + 1*NAND512_SPARE)
#define MX31_SPARE_COLUMN2 (3*NAND512_PAGE + 2*NAND512_SPARE)
#define MX31_SPARE_COLUMN3 (4*NAND512_PAGE + 3*NAND512_SPARE)

/*  This is the column address of the tag for a 2KB page.
*/
#define TAG_SPARE_COLUMN      MX31_SPARE_COLUMN3


/*-------------------------------------------------------------------
    Local: PageRead2KNoEcc()

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - page to read
        pMain  - 2048-byte buffer to receive main page data.
        pSpare - 64-byte buffer to receive spare area data, may
                 be NULL.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead2KNoEcc(
    D_UINT32 ulPage,
    void *pMain,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(0, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCPageIn(pMain, pSpare)
        && NFCPageIn((D_UCHAR *) pMain + MAIN_BUFFER_SIZE,
                  pSpare ? (D_UCHAR *) pSpare + SPARE_BUFFER_SIZE : NULL)
        && NFCPageIn((D_UCHAR *) pMain + 2 * MAIN_BUFFER_SIZE,
                  pSpare ? (D_UCHAR *) pSpare + 2 * SPARE_BUFFER_SIZE : NULL)
        && NFCPageIn((D_UCHAR *) pMain + 3 * MAIN_BUFFER_SIZE,
                  pSpare ? (D_UCHAR *) pSpare + 3 * SPARE_BUFFER_SIZE : NULL))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageRead2KEcc()

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - page to read.
        pMain  - 2048-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageRead2KEcc(
    D_UINT32    ulPage,
    void       *pMain,
    void       *pTag,
    unsigned    nTagSize)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    /*  Note that we retrieve the tag from the last spare buffer
        (and consequently we've got to write it in the same
         place).
    */
    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(0, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && PageInHwEcc(pMain, NULL, 0, &status)
        && PageInHwEcc((D_UCHAR *)pMain + MAIN_BUFFER_SIZE,  NULL, 0, &status)
        && PageInHwEcc((D_UCHAR *)pMain + (MAIN_BUFFER_SIZE * 2), NULL, 0, &status)
        && PageInHwEcc((D_UCHAR *)pMain + (MAIN_BUFFER_SIZE * 3), pTag, nTagSize, &status))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWrite2KNoEcc()

    Perform an entire program command sequence for large-block NAND.

    Parameters:
        ulPage - page to write
        pMain  - 2048-byte buffer containing main page data, aligned
                 on a 4-byte boundary.
        pSpare - 64-byte buffer containing spare area data, may
                 be NULL.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite2KNoEcc(
    D_UINT32 ulPage,
    const void *pMain,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;
    DCLALIGNEDBUFFER    (FbbStatusSpare, data, SPARE_BUFFER_SIZE);

    /*  Initialize the spare area buffer
    */
    DclMemSet(FbbStatusSpare.data, ERASED8, sizeof FbbStatusSpare.data);

    /*  Since write operations using this buffer also overwrite the location
        that would have otherwise contained a factory bad block mark,
        we need to store a value indicating the FBB mark is no
        longer valid.

        Note this conceptually violates the intention of the callers of
        PageWrite(fUseEcc == FALSE), which presumably expect the
        spare area to be unmodified, but that is precisely the function
        that can overwrite the factory bad block location, so we have to
        do something, or any blocks written with PageWrite(fUseEcc == FALSE)
        will appear bad during the next BBM format.
    */
    FbbStatusSpare.data[SPARE_INDEX_FBBSTATUS] = FBBSTATUS_FBB_INVALID;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Should chip information be kept closer at hand?
        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCPageOut(pMain, pSpare)
        && NFCPageOut((D_UCHAR *) pMain + MAIN_BUFFER_SIZE,
                   pSpare ? (D_UCHAR *) pSpare + SPARE_BUFFER_SIZE : NULL)
        && NFCPageOut((D_UCHAR *) pMain + 2 * MAIN_BUFFER_SIZE,
                   pSpare ? (D_UCHAR *) pSpare +  2 * SPARE_BUFFER_SIZE : NULL)
        && NFCPageOut((D_UCHAR *) pMain + 3 * MAIN_BUFFER_SIZE,
                   pSpare ? (D_UCHAR *) pSpare +  3 * SPARE_BUFFER_SIZE : FbbStatusSpare.data)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWrite2KNoEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWrite2KEcc()

    Write a 2K-byte page from large-block NAND.

    Perform an entire program command sequence for large-block NAND.

    Parameters:
        ulPage - page to write
        pMain  - 2048-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - tag to write

    Return Value:
        (Needs to be able to indicate no error; correctable error;
        uncorrectable error; timeout)
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWrite2KEcc(
    D_UINT32    ulPage,
    const void *pMain,
    const void *pTag,
    unsigned    nTagSize)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned    uChipStatus;

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Should chip information be kept closer at hand?
        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(0, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && PageOutHwEcc(pMain, NULL, 0)
        && PageOutHwEcc((D_UCHAR *)pMain + MAIN_BUFFER_SIZE, NULL, 0)
        && PageOutHwEcc((D_UCHAR *)pMain + (MAIN_BUFFER_SIZE * 2), NULL, 0)
        && PageOutHwEcc((D_UCHAR *)pMain + (MAIN_BUFFER_SIZE * 3), pTag, nTagSize)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
   {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWrite2KEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: SpareRead64()

    Read a 64-byte spare area from large-block NAND.

    Perform an entire Read command for large-block NAND.

    Parameters:
        ulPage - the page containing the spare area
        pSpare - buffer to receive the spare area data

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareRead64(
    D_UINT32 ulPage,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulColumn;
    unsigned uSpare = 0;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  First column is after the first page of main data
    */
    ulColumn = NAND512_PAGE;

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)     /* column address of one spare area */
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(pSpare)
        )
    {
        status.ffxStat = FFXSTAT_SUCCESS;
        pSpare = (D_UCHAR *)pSpare + NAND512_SPARE;
        uSpare++;
    }

    /*  Retrieve the remainder of the spare area using column address change
    */
    while ((status.ffxStat == FFXSTAT_SUCCESS) && (uSpare < 4))
    {
        ulColumn = NAND512_PAGE * (uSpare + 1) + NAND512_SPARE * uSpare;

        status.ffxStat = FFXSTAT_FIMTIMEOUT;

        if (NFCCommandOut(CMD_READ_PAGE_RANDOM)
            && NFCColumnOut(ulColumn, 2)     /* column address of one spare area */
            && NFCCommandOut(CMD_READ_PAGE_RANDOM_START)
            && NFCSpareIn(pSpare)
            )
        {
            status.ffxStat = FFXSTAT_SUCCESS;
            pSpare = (D_UCHAR *)pSpare + NAND512_SPARE;
            uSpare++;
        }
    }

    if (status.ffxStat == FFXSTAT_SUCCESS)
    {
        status.ulCount = 1;
    }

    return status;

}


/*-------------------------------------------------------------------
    Local: SpareWrite64()

    Write a 64-byte spare area to large-block NAND.

    Parameters:
        ulPage - the page containing the spare area
        pSpare - 64-byte buffer with data to be written to the spare
                 area, aligned on a 4-byte boundary.

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWrite64(
    D_UINT32 ulPage,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned int uChipStatus;

    /*  Turn the controller's ECC processing off.
    */
    SetEccMode(FALSE);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(MX31_SPARE_COLUMN0, 2)     /* column address of 1st 16 bytes of spare area */
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCCommandOut(CMD_WRITE_PAGE_RANDOM)
        && NFCColumnOut(MX31_SPARE_COLUMN1, 2)     /* column address of 2nd 16 bytes of spare area */
        && NFCSpareOut((D_BUFFER *)pSpare + SPARE_BUFFER_SIZE)
        && NFCCommandOut(CMD_WRITE_PAGE_RANDOM)
        && NFCColumnOut(MX31_SPARE_COLUMN2, 2)     /* column address of 3rd 16 bytes of spare area */
        && NFCSpareOut((D_BUFFER *)pSpare + 2 * SPARE_BUFFER_SIZE)
        && NFCCommandOut(CMD_WRITE_PAGE_RANDOM)
        && NFCColumnOut(MX31_SPARE_COLUMN3, 2)     /* column address of 4th 16 bytes of spare area */
        && NFCSpareOut((D_BUFFER *)pSpare + 3 * SPARE_BUFFER_SIZE)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("SpareWrite64: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagRead64()

    Read tags from from large-block NAND.

    An uncorrectable ECC error or bad check byte causes an
    all-zero (invalid) tag to be returned, but still returns a
    value indicating success.

    Parameters:
        ulPage - the page containing the tag
        pTag   - buffer to receive the tags

    Return Value:
        Should tag reads ever fail?  Yes, if timeout occurs then something
        is Horribly Wrong.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagRead64(
    D_UINT32            ulPage,
    void               *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    /*  Turn the controller's ECC on.
    */
    SetEccMode(TRUE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(TAG_SPARE_COLUMN, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(spare.data))
    {
        /*  Check for an ECC error in the spare area.

            BOGUS!  Cribbed (with minor modifications) from PageInHwEcc(), this
            probably should be wrapped up as a function.  It's inappropriate
            here to be accessing the NFC directly.
         */
        unsigned int uEccStatus = DCLMEMGET16(ECC_STATUS_RESULT);

        switch (uEccStatus & ECC_STATUS_SPARE)
        {
            case ECC_SPARE_CORR:
                /*  A correctable error occurred in the spare area.  The
                    controller has already corrected the data.
                */
                FFXPRINTF(1, ("Correctable error in tag: page %lu, location %04X\n",
                              ulPage, DCLMEMGET16(ECC_RSLT_SPARE_AREA)));
                FfxErrMgrEncodeCorrectedBits(1, &status);
                status.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                /* fall through */

            case 0:
                /*  DecodeTagHwEcc() takes care of an invalid tag (bad check byte).
                */
                DecodeTagHwEcc(pTag, nTagSize, spare.data);
                break;

            default:
                DclError();
                /* fall through */

            case ECC_SPARE_UNCORR:
                /*  An uncorrectable error in the tag causes an invalid (all
                    zero) tag to be returned.

                    BOGUS!  Is there (or should there be) a way to show the
                    tag was uncorrectable and thus invalidated while still
                    returning success status for good main page data?
                */
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                                "Uncorrectable error in tag: page %lu\n", ulPage));
                DclMemSet(pTag, 0, nTagSize);
                break;
        }
        status.ffxStat = FFXSTAT_SUCCESS;
        status.ulCount = 1;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagWrite64()

    Write tags to from large-block NAND.

    Parameters:
        ulPage - the page containing the tag
        pTag   - buffer to receive the tags

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS TagWrite64(
    D_UINT32            ulPage,
    const void         *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    unsigned            uChipStatus;
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    /*  Turn the controller's ECC processing on.
    */
    SetEccMode(TRUE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
        EncodeTagHwEcc(pTag, nTagSize, spare.data);

    /*  Some NAND chips need to be reset before programming.

        BOGUS!  Is this necessary for EVERY program command, or
        just the first?
    */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(TAG_SPARE_COLUMN, 2)
        && NFCRowOut(ulPage, NFC.uAddrsBytes)
        && NFCSpareOut(spare.data)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("TagWrite64: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: GetPageStatus2K()

    This function retrieves the page status information for the
    given page.

    Parameters:
        ulPage - The flash page from which to retrieve the status.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatus2K(
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
    D_BUFFER           *pLast512Spare;

    ioStat = SpareRead64(ulPage, spare.data);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Because only the last 16 bytes of the NAND controller's
            view of the spare area overlap with the NAND device's
            view of the spare area for Native 2K pages, we only store
            the FlashFX bad block status information in that last
            16 byte region (See comments at the top of this module
            and in GetBlockStatus2K for additional details).

            Basically, with the exception of ECC info, we treat the
            last 16 bytes of the spare area as if it were really the
            entire/only spare area available to FlashFX.
        */
        pLast512Spare = spare.data + (3 * NAND512_SPARE);

        if (CORRECTABLE(pLast512Spare[SPARE_INDEX_ECCSTATUS], ECCSTATUS_NONE))
            ioStat.op.ulPageStatus = PAGESTATUS_UNWRITTEN;   /* blank/no ECC */
        else if (CORRECTABLE(pLast512Spare[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
            ioStat.op.ulPageStatus = PAGESTATUS_WRITTENWITHECC;
        else
            ioStat.op.ulPageStatus = PAGESTATUS_UNKNOWN;   /* indecipherable */

        /*  If the tag area, including the check byte, is within 1 bit of
            being erased, then we know that a tag was not written.  If it
            is anything else, we know a tag was written.
        */

        /*  Move the tag check byte so it immediately follows the
            tag data (depending on byte order it might already
            be there).
        */
        pLast512Spare[SPARE_INDEX_HWECC_TAG+TAG_SIZE] = pLast512Spare[SPARE_INDEX_HWECC_TAGCHECK];

        if(!FfxNtmHelpIsRangeErased1Bit(&pLast512Spare[SPARE_INDEX_HWECC_TAG], TAG_SIZE+1))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(TAG_SIZE);
    }

    return ioStat;
}
#endif /* FFXCONF_NAND_2KBNATIVE */


#if (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
/*-------------------------------------------------------------------
    Local: GetBlockStatus2K()

    This function retrieves the block status information for the
    given erase block.

    Parameters:
        ulPhysicalPage - The physical page containing the status
                         of the block to return.
        bFbbType       - flags indicating the factory bad block type for the
                         flash

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetBlockStatus2K(
    D_UINT32        ulPhysicalPage,
    D_UINT8         bFbbType)
{
    FFXIOSTATUS     ioStat;
    DCLALIGNEDBUFFER (spare, data, FFX_NAND_MAXSPARESIZE);
    D_BUFFER        *pNative2KSpare;
  #if FFXCONF_CHECK_DLFBBSTATUS
    D_BUFFER        *pLast512Spare;
  #endif

    /*  All 2K page flash supported by this NTM uses the OFFSETZERO
        factory bad block mark type.
    */
    DclAssert(bFbbType == CHIPFBB_OFFSETZERO);
    (void)bFbbType;

    do
    {
        /*  Read the spare area, since we'll need to check that for
            our tag written marker and bad block marks.
        */
        ioStat = SpareRead64Native(ulPhysicalPage, spare.data);
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("NTMX31:GetBlockStatus2K() SpareRead64Native() failed.\n"));
             break;
        }
        /* Because we're looking at info offset from the BEGINNING of the
           64 byte spare area when we're looking for the actual offset-zero
           factory bad block marks, but our flag indicating the FBB info is
           invalid is in the spare area is at an offset indexed from the
           LAST 16 byte region,  set up two different views of the data that
           make sense for those scenarios.
        */
        pNative2KSpare = spare.data;
        
      #if FFXCONF_CHECK_DLFBBSTATUS
        pLast512Spare = pNative2KSpare + (3 * NAND512_SPARE);

        /*  Only check the original offset-zero FBB in the chip-relative
            spare area if the portion of the spare area that's common
            between the MX31's "spare" area and flash manufacturer's spare
            area does NOT indicate that location has been written (since we
            should never ever write a tag into a block that was marked bad
            by the NAND manufacturer).
        */
        if(!CORRECTABLE(pLast512Spare[SPARE_INDEX_FBBSTATUS], FBBSTATUS_FBB_INVALID))
      #endif
        {

            /*  If there are any zero bits in the first two bytes (word offset 0),
                of the 'real' (chip) spare area (and for 2KB page native, this is
                distinctly NOT the spare area data as read into the spare buffer
                by the MX31 NAND flash controller), the block is bad.

                Note also the logic below is both BE and LE compatible, since
                for 8-bit flash, pNative2KSpare[0] holds the FBB mark regardless
                of the processor endianness, and for 16-bit flash, we have to check
                BOTH bytes of the word (and the order is irrelevant).  Some day I
                hope to have a clearer way to write this that is also correct for
                both.
            */
            if ((pNative2KSpare[NSOFFSETZERO_FACTORYBAD_OFFSET] != ERASED8)
                && ((NFC.ucWidthBytes == 1) || (pNative2KSpare[NSOFFSETZERO_FACTORYBAD_OFFSET + 1] != ERASED8)))
            {
                ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                break;
            }
        }

        /*  All checks are okay, the block is good.
        */
        ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

    } while(FALSE);

#if D_DEBUG
    if (ioStat.ffxStat == FFXSTAT_SUCCESS
        && (ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD)
    {
        D_UINT16 *puSpare = (D_UINT16 *) spare.data;
        D_UINT32 *pulSpare = (D_UINT32 *) spare.data;

        DclPrintf("Bad block status %02lX at physical page %08lX contains:\n",
                  ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE, ulPhysicalPage);
        DclPrintf("    %04X %04X %04X %04X %04X %04X %04X %04X\n",
                  puSpare[0], puSpare[1], puSpare[2], puSpare[3],
                  puSpare[4], puSpare[5], puSpare[6], puSpare[7]);
        DclPrintf("    %08lX %08lX %08lX %08lX\n",
                  pulSpare[0], pulSpare[1], pulSpare[2], pulSpare[3]);
    }
#endif

    return ioStat;

}

/*-------------------------------------------------------------------
    Local: SetBlockStatus2K()

    This function sets the block status information for the
    given erase block.

    Parameters:
        ulPhysicalPage - The first physical page in the erase block
                         to be marked.
        ulBlockStatus  - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus2K(
    D_UINT32        ulPhysicalPage,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_BUFFER        *pLast512Spare;

    /*  This should only be done for factory bad blocks during formatting.
        There is no need for any other kind of marking.
    */
    switch (ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        case BLOCKSTATUS_FACTORYBAD:
        {
            DCLALIGNEDBUFFER (spare, data, FFX_NAND_MAXSPARESIZE);
            int i;

            DclMemSet(spare.data, ERASED8, sizeof spare.data);

            /*  All 2KB page flash is offsetzero style, mark the FBB location
            */
            spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET] = 0;

            /*  It's important to zero the FBBSTATUS_FBB_INVALID indicator
                at SPARE_INDEX_FBBSTATUS, since we can't assume it was successfully
                erased at any point.  Hopefully we can program enough bits in
                our FBB invalid indicator to invalidate it (invalidated invalid
                indicator means FBB is valid).

                If we couldn't erase it, and we can't program it, BBM will still
                add it to the FBB table during formatting, and FlashFX Pro won't
                use it anymore -- it will just have contents that appear to
                indicate it's a good block, and will need to be re-detected as 'bad'
                by a similar failure to program or erase during any subsequent BBM
                formats.
            */
            pLast512Spare = spare.data + (3 * NAND512_SPARE);
            pLast512Spare[SPARE_INDEX_FBBSTATUS] = 0;

            /*  Try to program the bad block indicator in at least one of the first
                two pages of the block.
            */
            for (i = 0; i < 2; ++i)
            {
                ioStat = SpareWrite64Native(ulPhysicalPage, spare.data);
                if (ioStat.ffxStat == FFXSTAT_SUCCESS)
                    break;

                ulPhysicalPage++;
            }
        }

        default:
            /*  Unexpected block status values are an error.
            */
            DclError();
            ioStat.ffxStat = FFXSTAT_BADPARAMETER;
            break;

    }

    return ioStat;
}
#endif /* FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED */


#if (FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED)
/*-------------------------------------------------------------------
    Local: SwEccCalc()

    Performs necessary initialization for ECC calculations before
    a data transfer.

    Parameters:
        pData   - A pointer to the data for which the ECCs are
                  to be calculated.
        uLength - The data length
        pabECC  - A pointer to an array of ECC bytes, typically 3
                  bytes for each 256 bytes of data.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void SwEccCalc(
    const D_BUFFER *pData,
    D_UINT16        uLength,
    D_BUFFER       *pabECC)
{
    DclAssert(pData);
    DclAssert(pabECC);
    DclAssert(uLength);
    DclAssert(uLength % DATA_BYTES_PER_ECC == 0);

    while(uLength)
    {
        FfxEccCalculate(pData, pabECC);

        uLength -= DATA_BYTES_PER_ECC;
        pData   += DATA_BYTES_PER_ECC;
        pabECC  += BYTES_PER_ECC;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: SpareOutSwEcc()

    Build spare area and transfer a 512+16 byte page to the NAND chip.

    Transfer a 512-byte main page and (optionally) a tag from memory to
    the NAND chip through the controller's RAM buffer.

    Parameters:
        pMain - 512-byte buffer with main page data, aligned
                on a 4-byte boundary.
        pTag  - buffer containing tag data

    Return Value:
        TRUE if the controller successfully transferred the data to the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL SpareOutSwEcc(
    const void         *pMain,
    const void         *pTag,
    unsigned            nTagSize)
{
    D_BUFFER            abECC[MAX_ECC_BYTES_PER_PAGE];
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
    {
        DclAssert(pTag);
        EncodeTagSwEcc(pTag, nTagSize, spare.data);
    }

    spare.data[SPARE_INDEX_ECCSTATUS] = ECCSTATUS_ECC;

    /*  Calculate software ECC for this page
    */
    SwEccCalc(pMain, NAND512_PAGE, abECC);

    /*  Build the ECC values!
    */
    spare.data[SPARE_INDEX_SWECC_ECC1 + 0] = abECC[0];
    spare.data[SPARE_INDEX_SWECC_ECC1 + 1] = abECC[1];
    spare.data[SPARE_INDEX_SWECC_ECC1 + 2] = abECC[2];

    spare.data[SPARE_INDEX_SWECC_ECC2 + 0] = abECC[3];
    spare.data[SPARE_INDEX_SWECC_ECC2 + 1] = abECC[4];
    spare.data[SPARE_INDEX_SWECC_ECC2 + 2] = abECC[5];

    return NFCSpareOut(spare.data);
}


/*-------------------------------------------------------------------
    Local: CorrectDataSegment()

    This function attempts to correct data errors in the
    specified data segment (DATA_BYTES_PER_ECC long).

    If this function does NOT encounter any uncorrectable data
    errors, the pIOStat->ulCount field will be incremented.

    If this function encounters correctable data errors, the
    PAGESTATUS_DATACORRECTED value will be OR'd into the
    supplied pIOStat->op.ulPageStatus field.

    Parameters:
        uCount       - The current segment count
        pData        - A pointer to the data segment
        pabEccOriginal - The original ECC value
        pabEccCalced   - The calculated ECC value
        pIOStat        - A pointer to the FFXIOSTATUS structure to use

    Return Value:
        Returns TRUE if the segment has uncorrectable errors,
        otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL CorrectDataSegment(
    D_UINT16        uCount,
    D_BUFFER       *pData,
    D_BUFFER       *pabEccOriginal,
    const D_BUFFER *pabEccCalced,
    FFXIOSTATUS    *pIOStat)
{
    DCLECCRESULT    iResult;

    iResult = FfxEccCorrect(pData, pabEccOriginal, pabEccCalced);
    if(iResult == DCLECC_UNCORRECTABLE)
    {
        FFXPRINTF(1, ("ECC failure in the %u bytes starting at page offset %4U\n",
            DATA_BYTES_PER_ECC, uCount*DATA_BYTES_PER_ECC));

        return TRUE;
    }
    else
    {
        if(iResult == DCLECC_DATACORRECTED)
        {
            FFXPRINTF(1, ("ECC corrected a data error in the %u bytes starting at page offset %U\n",
                DATA_BYTES_PER_ECC, uCount*DATA_BYTES_PER_ECC));

            pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
        }
        else if(iResult == DCLECC_ECCCORRECTED)
        {
            FFXPRINTF(1, ("ECC contained an error for the %u bytes starting at page offset %U\n",
                DATA_BYTES_PER_ECC, uCount*DATA_BYTES_PER_ECC));

            pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
        }
        else
        {
            DclAssert(iResult == DCLECC_VALID);
        }

        /*  Increment the count so long as we have NOT encountered an
            uncorrectable error.
        */
        pIOStat->ulCount++;
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: MX31Emul512SwEccCorrectPage()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
FFXIOSTATUS MX31Emul512SwEccCorrectPage(
    D_UINT16            uSpareSize,
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);
    D_UINT16    uCount = 0;
    D_BOOL      fUncorrectableError = FALSE;

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(uSpareSize);
    DclAssert(uSpareSize % NAND512_SPARE == 0);

    /*  Note that it is possible for a single-bit error to corrupt
        the status value.  Double-check it by testing whether it
        differs by a single bit.
    */
    while(uSpareSize)
    {
        if(CorrectDataSegment(uCount, pData, &pSpare[SPARE_INDEX_SWECC_ECC1], pabECC, &ioStat))
            fUncorrectableError = TRUE;

        uCount++;
        pData += DATA_BYTES_PER_ECC;
        pabECC  += BYTES_PER_ECC;

        if(CorrectDataSegment(uCount, pData, &pSpare[SPARE_INDEX_SWECC_ECC2], pabECC, &ioStat))
            fUncorrectableError = TRUE;

        uCount++;
        pData += DATA_BYTES_PER_ECC;
        pabECC  += BYTES_PER_ECC;

        /*  Even if we found errors, attempt to correct errors in
            any subsequent areas.
        */
        uSpareSize  -= NAND512_SPARE;
        pSpare      += NAND512_SPARE;
    }

    /*  Return FFXSTAT_SUCCESS if none of the segments had uncorrectable
        errors.
    */
    if(!fUncorrectableError)
        ioStat.ffxStat = FFXSTAT_SUCCESS;

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: CorrectPage512SwEcc()

    Correct (if necessary) a 512-byte page segment.

    Perform a partial page Read for 2KB-page ORNAND.  Because of
    the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        pMain   - 512-byte buffer containing uncorrected main
                  page data, aligned on a 4-byte boundary.
        pSpare  - 16-byte buffer containing spare data
        pTag    - buffer to receive the tag; may be NULL if tag is
                  not desired.
        pStatus - TBD

    Return Value:
        Returns boolean indicating success or failure,
        and pStatus is updated to indicate more detailed
        status.
-------------------------------------------------------------------*/
static D_BOOL CorrectPage512SwEcc(
    D_BUFFER       *pMain,
    D_BUFFER       *pSpare,
    void           *pTag,
    unsigned        nTagSize,
    FFXIOSTATUS    *pStatus)
{
    D_BUFFER        abECC[MAX_ECC_BYTES_PER_PAGE];

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    if (!CORRECTABLE(pSpare[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
    {
        FFXPRINTF(1, ("attempting ECC on a page written without ECC\n"));
        pStatus->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
    }
    else
    {
        /*  Calculate software ECC for this page
        */
        SwEccCalc(pMain, NAND512_PAGE, abECC);

        /*  Correct with SW ECC in a MX31 compatible spare area layout
        */
        *pStatus = MX31Emul512SwEccCorrectPage(NAND512_SPARE, pMain, pSpare, abECC);

        if (pStatus->ffxStat == FFXSTAT_SUCCESS)
        {
            pStatus->ulCount = 1;

            if (pTag)
            {
                if (DecodeTagSwEcc(pTag, nTagSize, pSpare) == FALSE)
                {
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                                    "Uncorrectable error in tag.\n"));
                }
            }
        }
        else if (pStatus->ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
        {
            FFXPRINTF(1, ("Uncorrectable error in page data\n"));
        }
    }

    return (pStatus->ffxStat == FFXSTAT_SUCCESS);
}


/*-------------------------------------------------------------------
    Local: PageReadEmul512SwEcc()

    Read a 512-byte page segment from ORNAND.

    Perform a partial page Read for 2KB-page ORNAND.  Because of
    the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page to read
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageReadEmul512SwEcc(
    D_UINT32            ulPage,
    void               *pMain,
    void               *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    D_UINT32            ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32            ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;
    /*  Get the main segment number by dividing the main area
        column address by 512.  Generate a spare area column by using
        the segment number, muliptlying by spare segment size (16),
        and adding the starting offset of the spare area (2048).
    */
    D_UINT32            ulSpareColumn = ((ulColumn / NAND512_PAGE) * NAND512_SPARE) + NAND2K_PAGE;

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCPageIn(pMain, NULL)
        && NFCCommandOut(CMD_READ_PAGE_RANDOM)
        && NFCColumnOut(ulSpareColumn, 2)
        && NFCCommandOut(CMD_READ_PAGE_RANDOM_START)
        && NFCSpareIn(spare.data)
        && CorrectPage512SwEcc(pMain, spare.data, pTag, nTagSize, &status))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageReadEmul512NoEcc()

    Read a 512-byte page segment from ORNAND.

    Perform a partial page Read for 2KB-page ORNAND.

    Parameters:
        ulPage - emulated page to read
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pSpare - 16-byte buffer to receive spare area data, may
                 be NULL.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageReadEmul512NoEcc(
    D_UINT32 ulPage,
    void *pMain,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32 ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCPageIn(pMain, pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWriteEmul512SwEcc()

    Write a 512-byte page segment to ORNAND.

    Perform a partial Page Write for 2KB-page ORNAND.  Because of
    the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page to write
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWriteEmul512SwEcc(
    D_UINT32        ulPage,
    const void     *pMain,
    const void     *pTag,
    unsigned        nTagSize)
{
    FFXIOSTATUS     status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32        ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32        ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;
    unsigned        uChipStatus;
    /*  Get the main segment number by dividing the main area
        column address by 512.  Generate a spare area column by using
        the segment number, muliptlying by spare segment size (16),
        and adding the starting offset of the spare area (2048).
    */
    D_UINT32        ulSpareColumn = ((ulColumn / NAND512_PAGE) * NAND512_SPARE) + NAND2K_PAGE;

    /*  Some NAND chips need to be reset before programming. */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCPageOut(pMain, NULL)
        && NFCCommandOut(CMD_WRITE_PAGE_RANDOM)
        && NFCColumnOut(ulSpareColumn, 2)
        && SpareOutSwEcc(pMain, pTag, nTagSize)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWriteEmul512SwEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: PageWriteEmul512NoEcc()

    Write a 512-byte page segment to ORNAND.

    Perform a partial Page Write for 2KB-page ORNAND.

    Parameters:
        ulPage - emulated page to write
        pMain  - 512-byte buffer containing main page data, aligned
                 on a 4-byte boundary.
        pSpare - 16-byte buffer containing spare area data, may
                 be NULL.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageWriteEmul512NoEcc(
    D_UINT32 ulPage,
    const void *pMain,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32 ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;
    unsigned int uChipStatus;

    /*  Some NAND chips need to be reset before programming. */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCPageOut(pMain, pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("PageWriteEmul512NoEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: SpareReadEmul16SwEcc()

    Read a 16-byte spare segment from ORNAND.

    Perform a spare area segment Read for 2KB-page ORNAND.  Because
    of the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page number related to the spare area segment
        pSpare - buffer to receive the spare area data

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareReadEmul16SwEcc(
    D_UINT32 ulPage,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;

    /*  Calculate the segment number within this physical page by:
        absolute emulated page number % 4
    */
    D_UINT32 ulMainSegmentNumber = ulPage % (NAND2K_PAGE / NAND512_PAGE);

    /*  Generate a spare area column by using the segment number,
        muliptlying by spare segment size (16), and adding the
        starting offset of the spare area (2048).
    */
    D_UINT32 ulColumn = (ulMainSegmentNumber * NAND512_SPARE) + NAND2K_PAGE;

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: SpareWriteEmul16SwEcc()

    Write a 16-byte spare segment to ORNAND.

    Perform a spare area segment Write for 2KB-page ORNAND.  Because
    of the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page number related to the spare area segment
        pSpare - 16-byte buffer with data to be written to the spare
                 area, aligned on a 4-byte boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS SpareWriteEmul16SwEcc(
    D_UINT32 ulPage,
    const void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    unsigned int uChipStatus;

    /*  Calculate the segment number within this physical page by:
        absolute emulated page number % 4
    */
    D_UINT32 ulMainSegmentNumber = ulPage % (NAND2K_PAGE / NAND512_PAGE);

    /*  Generate a spare area column by using the segment number,
        multiplying by spare segment size (16), and adding the
        starting offset of the spare area (2048).
    */
    D_UINT32 ulColumn = (ulMainSegmentNumber * NAND512_SPARE) + NAND2K_PAGE;

    /*  Some NAND chips need to be reset before programming. */
    if ((!NFC.fResetBeforeProgram || NFCCommandOut(CMD_RESET))
        && NFCCommandOut(CMD_PROGRAM)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCSpareOut(pSpare)
        && NFCCommandOut(CMD_PROGRAM_START)
        && ReadStatus(&uChipStatus))
    {
        status.ffxStat = ProcessChipStatus(uChipStatus);
        if (status.ffxStat == FFXSTAT_SUCCESS)
        {
            status.ulCount++;
        }
        else
        {
            FFXPRINTF(1, ("SpareWriteEmul16SwEcc: error status %02x\n", uChipStatus));
        }
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagReadEmul16SwEcc()

    Read tags from from 2KB-page ORNAND.

    An uncorrectable ECC error or bad check byte causes an
    all-zero (invalid) tag to be returned, but still returns a
    value indicating success.

    Parameters:
        ulPage - emulated page number related to the spare area segment
        pTag   - buffer to receive the tags

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagReadEmul16SwEcc(
    D_UINT32            ulPage,
    void               *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    D_UINT32            ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    /*  Calculate the segment number within this physical page by:
        absolute emulated page number % 4
    */
    D_UINT32            ulMainSegmentNumber = ulPage % (NAND2K_PAGE / NAND512_PAGE);
    /*  Generate a spare area column by using the segment number,
        muliptlying by spare segment size (16), and adding the
        starting offset of the spare area (2048).
    */
    D_UINT32            ulColumn = (ulMainSegmentNumber * NAND512_SPARE) + NAND2K_PAGE;

    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_MAIN_START)
        && NFCSpareIn(spare.data))
    {
        if (!DecodeTagSwEcc(pTag, nTagSize, spare.data))
        {
            /*  An uncorrectable error in the tag causes an invalid (all
                zero) tag to be returned, which is not treated as an
                error in this layer.
            */
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                            "Uncorrectable error in tag: page %lu\n", ulPage));
            DclMemSet(pTag, 0, nTagSize);
        }

        status.ffxStat = FFXSTAT_SUCCESS;
        status.ulCount = 1;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: TagWriteEmul16SwEcc()

    Write tags to 2KB-page ORNAND.

    Parameters:
        ulPage - emulated page number related to the spare area segment
        pTag   - buffer to receive the tags

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS TagWriteEmul16SwEcc(
    D_UINT32            ulPage,
    const void         *pTag,
    unsigned            nTagSize)
{
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);
    EncodeTagSwEcc(pTag, nTagSize, spare.data);
    return SpareWriteEmul16SwEcc(ulPage, spare.data);
}


/*-------------------------------------------------------------------
    Local: GetPageStatusEmul512SwEcc()

    This function retrieves the page status information for the
    given page.

    Parameters:
        ulPage - The flash page from which to retrieve the status.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatusEmul512SwEcc(
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    ioStat = SpareReadEmul16SwEcc(ulPage, spare.data);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_NONE))
            ioStat.op.ulPageStatus = PAGESTATUS_UNWRITTEN;   /* blank/no ECC */
        else if (CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
            ioStat.op.ulPageStatus = PAGESTATUS_WRITTENWITHECC;
        else
            ioStat.op.ulPageStatus = PAGESTATUS_UNKNOWN;   /* writen by BBM, indecipherable */

        /*  If the tag area, including the ECC and check bytes, is within
            1 bit of being erased, then we know that a tag was not written.
            If it is anything else, we know a tag was written.
        */

        /*  This code depends on the two tag bytes, the ECC, and the
            check bytes being in sequence in the spare area (though
            the order does not matter).
        */
        DclAssert((SPARE_INDEX_SWECC_TAGECC   = SPARE_INDEX_SWECC_TAG+2)   ||
                  (SPARE_INDEX_SWECC_TAGCHECK = SPARE_INDEX_SWECC_TAG+2));
        DclAssert((SPARE_INDEX_SWECC_TAGECC   = SPARE_INDEX_SWECC_TAG+3)   ||
                  (SPARE_INDEX_SWECC_TAGCHECK = SPARE_INDEX_SWECC_TAG+3));

        if(!FfxNtmHelpIsRangeErased1Bit(&spare.data[SPARE_INDEX_SWECC_TAG], TAG_SIZE+2))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(TAG_SIZE);
    }

    return ioStat;
}
#endif /* (FFXCONF_NAND_512BORNANDEMULATED || FFXCONF_NAND_512BEMULATED) */


#if FFXCONF_NAND_512BORNANDEMULATED
/*-------------------------------------------------------------------
    Local: ORNANDPgRdEm512SwEcc()

    Read a 512-byte page segment from ORNAND.

    Perform a partial page Read for 2KB-page ORNAND.  Because of
    the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page to read
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pTag   - buffer to receive the tag; may be NULL if tag is
                 not desired.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS ORNANDPgRdEm512SwEcc(
    D_UINT32            ulPage,
    void               *pMain,
    void               *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    D_UINT32            ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32            ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_SEGMENT_START)   /* reads 528B like small page */
        && NFCPageIn(pMain, spare.data)
        && CorrectPage512SwEcc(pMain, spare.data, pTag, nTagSize, &status))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: ORNANDPgRdEm512NoEcc()

    Read a 512-byte page segment from ORNAND.

    Perform a partial page Read for 2KB-page ORNAND.

    Parameters:
        ulPage - emulated page to read
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pSpare - 16-byte buffer to receive spare area data, may
                 be NULL.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS ORNANDPgRdEm512NoEcc(
    D_UINT32 ulPage,
    void *pMain,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;
    D_UINT32 ulColumn = (ulPage * NAND512_PAGE) % NAND2K_PAGE;

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_SEGMENT_START)   /* reads 528B like small page */
        && NFCPageIn(pMain, pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}

/*-------------------------------------------------------------------
    Local: ORNANDSpRdEm16SwEcc()

    Read a 16-byte spare segment from ORNAND.

    Perform a spare area segment Read for 2KB-page ORNAND.  Because
    of the segment offsets required by ORNAND, software ECC is used.

    Parameters:
        ulPage - emulated page number related to the spare area segment
        pSpare - buffer to receive the spare area data

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pBuffer array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS ORNANDSpRdEm16SwEcc(
    D_UINT32 ulPage,
    void *pSpare)
{
    FFXIOSTATUS status = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMTIMEOUT);
    D_UINT32 ulRow = (ulPage * NAND512_PAGE) / NAND2K_PAGE;

    /*  Calculate the segment number within this physical page by:
        absolute emulated page number % 4
    */
    D_UINT32 ulMainSegmentNumber = ulPage % (NAND2K_PAGE / NAND512_PAGE);

    /*  Generate a spare area column by using the segment number,
        muliptlying by spare segment size (16), and adding the
        starting offset of the spare area (2048).
    */
    D_UINT32 ulColumn = (ulMainSegmentNumber * NAND512_SPARE) + NAND2K_PAGE;

    if (NFCCommandOut(CMD_READ_MAIN)
        && NFCColumnOut(ulColumn, 2)
        && NFCRowOut(ulRow, NFC.uAddrsBytes)
        && NFCCommandOut(CMD_READ_SEGMENT_START)
        && NFCSpareIn(pSpare))
    {
        status.ulCount = 1;
        status.ffxStat = FFXSTAT_SUCCESS;
    }

    return status;
}
#endif /* FFXCONF_NAND_512BORNANDEMULATED */



/*-------------------------------------------------------------------
    Public: ParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hNTM      - The NTM handle.
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
static FFXSTATUS ParameterGet(
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    "NTMX31:ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
                    hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTMX31:ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    "NTMX31:ParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: ParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hNTM      - The NTM handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ParameterSet(
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    "NTMX31:ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
                    hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTMX31:ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    "NTMX31:ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ReadStatus()

    Read status from the NAND chip.

    Issue a status command and read the status.

    Parameters:
        puStatus - the status returned by the chip

    Return Value:
        TRUE if the status was read, FALSE if the command timed out.
-------------------------------------------------------------------*/
static D_BOOL ReadStatus(unsigned int *puStatus)
{
    D_UCHAR ucStatus = 0;

    if (NFCCommandOut(CMD_STATUS)
        && StatusIn(&ucStatus))
    {
        *puStatus = ucStatus;
        return TRUE;
    }
    return FALSE;
}

/*-------------------------------------------------------------------
    Actual operations on the controller.
-------------------------------------------------------------------*/

#if D_DEBUG
/*-------------------------------------------------------------------
    Local: ByteOrderTest()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ByteOrderTest(void)
{
    D_UINT32 ulTest = 0x04030201;
    D_UCHAR *puc;
    D_UINT16 *pu;
    D_UINT32 *pul;

    puc = (D_UCHAR *) &ulTest;
    DclPrintf("Memory byte order: %02x %02x %02x %02x\n",
              puc[0], puc[1], puc[2], puc[3]);

    pu = (D_UINT16 *) &ulTest;
    DclPrintf("Memory word order: %04x %04x\n", pu[0], pu[1]);

    pul = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
    *pul = ulTest;

    pu = (D_UINT16 *) NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
    DclPrintf("Buffer word order: %04x %04x\n", pu[0], pu[1]);
}
#endif

/*-------------------------------------------------------------------
    Local: InitController()

    Initialize the NAND controller.

    Find out the address of the NAND controller, and set it
    up at least enough to read the device ID.

    Parameters:
        hNTM - handle for the NTM

    Return Value:
        TRUE if the controller was initialized successfully, FALSE if not.
-------------------------------------------------------------------*/
static D_BOOL InitController(NTMHANDLE hNTM)
{
    /*  Find out the memory-mapped I/O address of the controller

        There is no default, if the option is not available,
        initialization fails.
    */
    if (FfxHookOptionGet(FFXOPT_FLASH_START, hNTM->hDev,
                         &NFC.pBase, sizeof NFC.pBase))
    {

        /*  Reset the controller state machine
        */
        NFCReset();

        /*  Treat the flash as x8 until we identify the part,
            at which point the width will be known.
        */
        if (FfxHookNTMX31SetParameters(hNTM->pNC, 1, NAND512_PAGE))
        {

            DclAssert(DCLMEMGET16(NFC_BUFSIZE) == 0x0001);

            NFC.uConfig1 = INT_MASK;

    #if NFC_HAS_ENDIAN && DCL_BIG_ENDIAN
            /*  Configure for big-endian byte order.
            */
            NFC.uConfig1 |= NF_BIG;
    #endif

            DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);

    #if D_DEBUG
            ByteOrderTest();
    #endif

            /*  Set the buffer area to use.
            */
            DCLMEMPUT16(RAM_BUFFER_ADDRESS, XFER_BUFFER_NUMBER);

            /*  Reset the NAND chip.
            */
            if (NFCCommandOut(CMD_RESET))
            {
                /*  Unlock all blocks.
                */
                DCLMEMPUT16(UNLOCK_START_BLK_ADD, 0);
                DCLMEMPUT16(UNLOCK_END_BLK_ADD, ~0);
                DCLMEMPUT16(NF_WR_PROT, WRPROT_UNLOCK);

                return TRUE;
            }
        }
    }
    return FALSE;
}


/*-------------------------------------------------------------------
    Local: ConfigController()

    Configure the NAND controller for the flash.

    Configure the NFC and this NTM to operate with the kind of flash
    that was detected.

    Parameters:
        pNC   - private project hooks info
        pChip - the chip information

    Return Value:
        A vector containing functions to operate the flash using
        this controller, if the chip is one that this NTM knows how
        to operate, NULL if not.
-------------------------------------------------------------------*/
static NTMX31OPERATIONS * ConfigController(
    PNANDCTL            pNC,
    const FFXNANDCHIP  *pChip)
{
    NTMX31OPERATIONS   *pNANDOps = NULL;

    NFC.fResetBeforeProgram = pChip->pChipClass->ResetBeforeProgram;
    NFC.uAddrsBytes =
        (pChip->pChipClass->uLinearChipAddrMSB - pChip->pChipClass->uLinearPageAddrMSB + 7)
        / 8;


    /*  For page size, select the appropriate set of operations functions.
    */
    if (pChip->pChipClass->uPageSize == NAND512_PAGE)
    {
#if FFXCONF_NAND_512BNATIVE
        DclAssert(!pChip->pChipClass->ReadConfirm);
        pNANDOps = &Native512Ops;
#else
        FFXPRINTF(1, ("NTMX31:ConfigController() 512B page NAND device detected in NTM built without native 512B support.\n"));
        return NULL;
#endif /* FFXCONF_NAND_512BNATIVE */
    }
    else if (pChip->pChipClass->uPageSize == NAND2K_PAGE)
    {

        DclAssert(pChip->pChipClass->ReadConfirm);

        /*  This NTM only supports those 2KB parts that implement command 0x85.
        */
        /*  BOGUS! This flag from the table is supposed to indicate whether
            CMD_WRITE_PAGE_RANDOM (0x85) can be used or not, but is not
            correct for some parts at this time.  This won't hurt anything as
            long as the NAND parts actually do support this command, which
            most newer parts do.  Generally it would be surprising to see this
            NTM used with older parts.  See BZ #1203.

            DclAssert(pChip->pChipClass->Samsung2kOps);
        */
#if D_DEBUG
        if (!pChip->pChipClass->Samsung2kOps)
        {
            FFXPRINTF(1, ("NTMX31:ConfigController() 2KB page NAND device with FALSE in the nandid table for CMD_WRITE_PAGE_RANDOM (0x85) support.\n"));
        }
#endif

        /*  The following options are mutually excusive - you can either
            build for 2KB Native pages or 512B emulated pages, but not both.
        */
#if FFXCONF_NAND_2KBNATIVE

        pNANDOps = &Native2KOps;

#elif FFXCONF_NAND_512BORNANDEMULATED

        if (pChip->pChipClass->ORNANDPartialPageReads)
        {
            /*  Can use ORNAND partial page read operations,
                but can't use HW ECC in this controller.
            */
            pNANDOps = &ORNANDEmulated512Ops;

        }
        else
        {
            /*  Can use HW ECC in this controller, but can't
                use ORNAND partial page read operations.

                TBD: Currently using SW ECC for ALL 512B emulation,
                whether ORNAND or not.
            */
            pNANDOps = &Emulated512Ops;
        }
#elif FFXCONF_NAND_512BEMULATED

        pNANDOps = &Emulated512Ops;

#else
        FFXPRINTF(1, ("NTMX31:ConfigController() 2KB page NAND device detected in NTM built without 2KB support.\n"));
        return NULL;
#endif /* FFXCONF_NAND_XXXX */

    }
    else
    {
        FFXPRINTF(1, ("NTMX31:ConfigController() unknown page size=%x\n", pChip->pChipClass->uPageSize));
        return NULL;
    }

    /*  For 16-bit wide NAND, set the 16-bit flag in the RCSR
    */
    if((pChip->bFlags & CHIPINT_MASK) == CHIPINT_8BIT)
    {
        NFC.ucWidthBytes = 1;
    }
    else if((pChip->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
    {
        NFC.ucWidthBytes = 2;
    }
    else
    {
        FFXPRINTF(1, ("NTMX31:ConfigController() unknown bus width.\n"));
        return NULL;
    }

    /*  Program the system with the characteristics of the chip.
    */
    if (!FfxHookNTMX31SetParameters(pNC, NFC.ucWidthBytes, pChip->pChipClass->uPageSize))
    {
        FFXPRINTF(1, ("NTMX31:ConfigController() FfxHookNTMX31SetParameters returned FALSE.\n"));
        return NULL;
    }

    return pNANDOps;
}



/*-------------------------------------------------------------------
    Local: UninitController()

    Undo initialization of NAND controller.

    Placeholder, nothing to do in current implementation.

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void UninitController(void)
{
    /* nothing to do.
    */
}


/*-------------------------------------------------------------------
    Local: IDIn()

    Get ID data from the NAND chip.

    Transfers chip ID data from the NAND chip to memory.

    Parameters:
        pCtrlr - NAND_CONTROLLER structure returned by
                 InitController()
        aucID  - NAND_ID_SIZE buffer to receive the ID data

    Return Value:
        TRUE if transfer succeeded, FALSE if controller timed out.
-------------------------------------------------------------------*/
static D_BOOL IDIn(D_UCHAR aucID[NAND_ID_SIZE])
{
    /*  Use the last buffer of the four.
     */
    DCLMEMPUT16(RAM_BUFFER_ADDRESS, XFER_BUFFER_NUMBER);

    /*  Make sure SP_EN isn't set.
     */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);

    /*  Get the ID data out of the chip.  The controller reads six
        bytes; depending on the chip, varying amounts may be
        meaningful.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDO_ID);

    if (NFCWait())
    {
        D_UINT32 ulID;

        DclAssert(NAND_ID_SIZE > 5);

        /*  Note that because the NAND bus width is always x8 at this point,
            the ID bytes are byte-packed even if the NAND chip has an x16
            data bus.
        */
#if DCL_BIG_ENDIAN
        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0]);
        aucID[0] = (D_UCHAR) (ulID >> 24);
        aucID[1] = (D_UCHAR) (ulID >> 16);
        aucID[2] = (D_UCHAR) (ulID >> 8);
        aucID[3] = (D_UCHAR) ulID;

        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][1]);
        aucID[4] = (D_UCHAR) (ulID >> 24);
        aucID[5] = (D_UCHAR) (ulID >> 16);
#else
        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0]);
        aucID[0] = (D_UCHAR) ulID;
        aucID[1] = (D_UCHAR) (ulID >> 8);
        aucID[2] = (D_UCHAR) (ulID >> 16);
        aucID[3] = (D_UCHAR) (ulID >> 24);

        ulID = DCLMEMGET32(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][1]);
        aucID[4] = (D_UCHAR) ulID;
        aucID[5] = (D_UCHAR) (ulID >> 8);
#endif


#if D_DEBUG
        {
            D_UINT32 *pulID = &NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0];
            D_UINT16 *puID = (D_UINT16 *) &NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0];

            DclPrintf("ID RAM buffer contents:\n");
            DclPrintf("%08lX %08lX %08lX %04x %04X %04X %04X %04X %04X\n",
                      DCLMEMGET32(pulID), DCLMEMGET32(pulID + 1), DCLMEMGET32(pulID + 2),
                      DCLMEMGET16(puID), DCLMEMGET16(puID + 1), DCLMEMGET16(puID + 2),
                      DCLMEMGET16(puID + 3), DCLMEMGET16(puID + 4), DCLMEMGET16(puID + 5));
            DclPrintf("Returning ID bytes %02x %02x %02x %02x %02x %02x\n",
                      aucID[0], aucID[1], aucID[2], aucID[3], aucID[4], aucID[5]);
        }
#endif

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-------------------------------------------------------------------
    Local: StatusIn()

    Get status data from the NAND chip.

    Transfers a status byte from the NAND chip

    Parameters:
        pucStatus - where to store the status byte

    Return Value:
        TRUE if transfer succeeded, FALSE if controller timed out.
-------------------------------------------------------------------*/
static D_BOOL StatusIn(D_UCHAR *pucStatus)
{
    D_UINT16 uStatus;

    /*  Make sure SP_EN isn't set.
     */
    DclAssert((NFC.uConfig1 & SP_EN) == 0);
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);

    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDO_STATUS);

    if (NFCWait())
    {

        uStatus = DCLMEMGET16(&NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER][0]);

#if DCL_BIG_ENDIAN
        switch(NFC.ucWidthBytes)
        {
        case 1:
            /*  The first (only) byte provided by the NAND
                device is in the upper byte (adjust for BE)
            */
            *pucStatus = (D_UCHAR) (uStatus >> 8);
            break;
        case 2:
            /*  The LSB (the only one we need for status) of the
                halfword provided by the NAND device is placed into
                the lower byte of the halfword we read from the RAM buffer
                by the NFC (based on NF_BIG flag), so no
                swapping necessary in x16 configuration
            */
            *pucStatus = (D_UCHAR) (uStatus);
            break;
        default:
            DclError();
            break;
        }
#else
        /*  No swapping necessary, the LSB is in the same
            location in the RAM buffer in LE mode,
            regardless of the width
        */
        *pucStatus = (D_UCHAR) (uStatus);
#endif

        return TRUE;
    }

    /*  If this times out, this is probably indicitive of a system failure
        (hardware or software) that needs to be debugged, as opposed to one
        of the expected failure mechanisms for NAND flash.
    */
    DclAssert(FALSE);
    return FALSE;
}


#if (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BNATIVE)     /* not used for SW ECC */
/*-------------------------------------------------------------------
    Local: PageInHwEcc()

    Transfer and correct a 512+16 byte page from the NAND chip.

    Transfer a 512-byte main page and (optionally) 16-byte spare area from
    the NAND chip through the controller's RAM buffer to memory and
    perform error detection and correction.

    Note that if the page was written without ECC, the result of reading
    it with ECC is undefined.

    Parameters:
        pMain    - 512-byte buffer to receive main page data, aligned
                   on a 4-byte boundary.
        pTag     - 16-byte buffer to receive spare area data, aligned
                   on a 4-byte boundary; may be NULL if spare data is
                   not desired.
        nTagSize - The tag size to use.
        pStatus  - FFXIOSTATUS indicating more detailed information
                   than the simple success or failure return code.
                   The caller must initialize the contents of this
                   structure.

                   On return, pStatus contents will be updated to include
                   an ffxStat value, as well as the flags updated with
                   PAGESTATUS_DATACORRECTED if necessary.  The ulCount
                   will NOT be updated with the number of pages read,
                   since this function cannot know whether it is
                   working with partial pages or full pages.

    Return Value:
        Indicates whether the function failed or succeeded.
-------------------------------------------------------------------*/
static D_BOOL PageInHwEcc(
    void               *pMain,
    void               *pTag,
    unsigned            nTagSize,
    FFXIOSTATUS        *pStatus)
{
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);
    D_UINT32            ulBitsCorrected = 0;

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize == TAG_SIZE) || (!pTag && !nTagSize));

    /*  Transfer the raw page in from the NAND chip.
    */
    if (NFCPageIn(pMain, spare.data))
    {
        unsigned int uEccStatus = DCLMEMGET16(ECC_STATUS_RESULT);

        if (!CORRECTABLE(spare.data[SPARE_INDEX_ECCSTATUS], ECCSTATUS_ECC))
        {
            FFXPRINTF(1, ("attempting ECC read on a page written without ECC\n"));
        }

        /*  Check for an ECC error in the main page.
        */
        switch (uEccStatus & ECC_STATUS_MAIN)
        {
            case 0:
                pStatus->ffxStat = FFXSTAT_SUCCESS;
                break;   /* no error, no correction required */

            case ECC_MAIN_CORR:
                /*  A correctable error occurred in the main page.  The
                    controller has already corrected the data.
                */
                FFXPRINTF(1, ("Correctable error in page data: location %04X\n",
                              DCLMEMGET16(ECC_RSLT_MAIN_AREA)));
                pStatus->ffxStat = FFXSTAT_SUCCESS;
                FfxErrMgrEncodeCorrectedBits(1, pStatus);
                pStatus->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                break;

            default:
                DclError();
                /* fall through */

            case ECC_MAIN_UNCORR:
                FFXPRINTF(1, ("Uncorrectable error in page data\n"));
                pStatus->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                break;
        }

        /*  The tag (in the spare area) is only error corrected and extracted
            if it's desired (pTag is not NULL).
        */
        if (pStatus->ffxStat == FFXSTAT_SUCCESS && pTag)
        {
            DclAssert(nTagSize == TAG_SIZE);

            /*  Check for an ECC error in the spare area.
             */
            switch (uEccStatus & ECC_STATUS_SPARE)
            {
            case ECC_SPARE_CORR:
                /*  A correctable error occurred in the spare area.  The
                    controller has already corrected the data.
                */
                FFXPRINTF(1, ("Correctable error in tag: location %04X\n",
                              DCLMEMGET16(ECC_RSLT_SPARE_AREA)));
                ulBitsCorrected = FfxErrMgrDecodeCorrectedBits(pStatus);
                ulBitsCorrected++;
                FfxErrMgrEncodeCorrectedBits(ulBitsCorrected, pStatus);

                pStatus->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                /* fall through */

            case 0:
                /*  DecodeTagHwEcc() takes care of an invalid tag (bad check byte).
                */
                DecodeTagHwEcc(pTag, nTagSize, spare.data);
                break;

            default:
                DclError();
                /* fall through */

            case ECC_SPARE_UNCORR:
                /*  An uncorrectable error in the tag causes an invalid (all
                    zero) tag to be returned.

                    BOGUS!  Is there (or should there be) a way to show the
                    tag was uncorrectable and thus invalidated while still
                    returning success status for good main page data?
                */
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                                "Uncorrectable error in tag.\n"));
                DclMemSet(pTag, 0, nTagSize);
                break;
            }
        }
    }
    else
    {
        pStatus->ffxStat = FFXSTAT_FIMTIMEOUT;
    }

    return (pStatus->ffxStat == FFXSTAT_SUCCESS);
}


/*-------------------------------------------------------------------
    Local: PageOutHwEcc()

    Build spare area and transfer a 512+16 byte page to the NAND chip.

    Transfer a 512-byte main page and (optionally) a tag from memory to
    the NAND chip through the controller's RAM buffer.

    Parameters:
        pMain - 512-byte buffer to receive main page data, aligned
                on a 4-byte boundary.
        pTag  - buffer containing tag data; may be NULL if tag is
                not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL PageOutHwEcc(
    const void         *pMain,
    const void         *pTag,
    unsigned            nTagSize)
{
    DCLALIGNEDBUFFER    (spare, data, SPARE_BUFFER_SIZE);

    DclMemSet(spare.data, ERASED8, sizeof spare.data);

    if(nTagSize)
    {
        DclAssert(pTag);
        EncodeTagHwEcc(pTag, nTagSize, spare.data);
    }

    spare.data[SPARE_INDEX_ECCSTATUS] = ECCSTATUS_ECC;

    return NFCPageOut(pMain, spare.data);
}


/*-------------------------------------------------------------------
    Local: EncodeTagHwEcc()

    Accepts a NULL tag pointer and does nothing.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void EncodeTagHwEcc(
    const D_BUFFER *pTag,
    unsigned        nTagSize,
    D_BUFFER        pSpare[SPARE_BUFFER_SIZE])
{
    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    DclMemCpy(&pSpare[SPARE_INDEX_HWECC_TAG], pTag, nTagSize);

    pSpare[SPARE_INDEX_HWECC_TAGCHECK] = pTag[0] ^ ~pTag[1];

    /*  Write FBBSTATUS_FBB_INVALID to indicate ECC has been
        written (which invalidates the factory bad block marking)
        This value is used by GetBlockStatus to distinguish
        between a factory bad block mark and 512B page tags (or
        2KB page main data), since the factory bad block mark
        indication is overwritten with tag data due to the
        spare area layout.
    */
    pSpare[SPARE_INDEX_FBBSTATUS] = FBBSTATUS_FBB_INVALID;

    return;
}


/*-------------------------------------------------------------------
    Local: DecodeTagHwEcc()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DecodeTagHwEcc(
    D_BUFFER       *pTag,
    unsigned        nTagSize,
    const D_BUFFER  pSpare[SPARE_BUFFER_SIZE])
{
    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);

    if ((pSpare[SPARE_INDEX_HWECC_TAG] ^ (D_BUFFER) ~pSpare[SPARE_INDEX_HWECC_TAG + 1])
        == pSpare[SPARE_INDEX_HWECC_TAGCHECK])
    {
        DclMemCpy(pTag, &pSpare[SPARE_INDEX_HWECC_TAG], nTagSize);
        return TRUE;
    }
    else
    {
        static D_BUFFER abZero[TAG_SIZE] = {0};

        /*  What the heck is this?  Is there something wrong
            with DclMemSet()... ?
        */
        DclMemCpy(pTag, abZero, nTagSize);
        return FALSE;
    }
}
#endif  /* (FFXCONF_NAND_2KBNATIVE || FFXCONF_NAND_512BNATIVE) */


#if (FFXCONF_NAND_512BEMULATED || FFXCONF_NAND_512BORNANDEMULATED)     /* not used for HW ECC */
/*-------------------------------------------------------------------
    Local: EncodeTagSwEcc()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void EncodeTagSwEcc(
    const D_BUFFER *pTag,
    unsigned        nTagSize,
    D_BUFFER        pSpare[SPARE_BUFFER_SIZE])
{
    if (pTag)
    {
        DclAssert(nTagSize == TAG_SIZE);

        DclMemCpy(&pSpare[SPARE_INDEX_SWECC_TAG], pTag, nTagSize);
        pSpare[SPARE_INDEX_SWECC_TAGCHECK] = pTag[0] ^ ~pTag[1];
        pSpare[SPARE_INDEX_SWECC_TAGECC] = (D_UINT8)DclHammingCalculate(&pSpare[SPARE_INDEX_SWECC_TAG], 3, 0);
    }
}


/*-------------------------------------------------------------------
    Local: DecodeTagSwEcc()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DecodeTagSwEcc(
    D_BUFFER       *pTag,
    unsigned        nTagSize,
    const D_BUFFER  pSpare[SPARE_BUFFER_SIZE])
{
    D_UINT32        ecc;
    D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];

    DclAssert(pTag);
    DclAssert(nTagSize == TAG_SIZE);
    DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));

    /*  If the encoded tag is in the fully erased state, return
        successfully, after having set the decoded tag to the
        erased state as well.

        ToDo: This logic does not allow for single bit errors
              in the erased area (this is BZ#1921).
    */
    if ((pSpare[SPARE_INDEX_SWECC_TAG] == ERASED8)
        && (pSpare[SPARE_INDEX_SWECC_TAG + 1] == ERASED8)
        && (pSpare[SPARE_INDEX_SWECC_TAGCHECK] == ERASED8)
        && (pSpare[SPARE_INDEX_SWECC_TAGECC] == ERASED8))
    {
        pTag[0] = ERASED8;
        pTag[1] = ERASED8;

        return TRUE;
    }

    /*  Copy in the tag, tag check, and ECC
    */
    DclMemCpy(TempBuff, &pSpare[SPARE_INDEX_SWECC_TAG], nTagSize);
    TempBuff[nTagSize] = pSpare[SPARE_INDEX_SWECC_TAGCHECK];
    TempBuff[nTagSize + 1] = pSpare[SPARE_INDEX_SWECC_TAGECC];

    ecc = DclHammingCalculate(&TempBuff[0], nTagSize + 1, TempBuff[nTagSize + 1]);

    if(ecc != 0)
        DclHammingCorrect(&TempBuff[0], nTagSize + 1, ecc);

    /*  Check the validation code
    */
    if(TempBuff[nTagSize] != (D_UCHAR) (TempBuff[0] ^ ~TempBuff[1]))
    {
        /*  Why is this not just using DclMemSet()?
        */
        static D_BUFFER abZero[TAG_SIZE] = {0};
        DclMemCpy(pTag, abZero, nTagSize);

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
            "DecodeTagSwEcc() Unrecoverable tag error, Src=%lX ecc=%lX Corrected=%lX\n",
            *(D_UINT32*)&pSpare[SPARE_INDEX_SWECC_TAG], ecc, *(D_UINT32*)&TempBuff[0]));

        return FALSE;
    }

    DclMemCpy(pTag, TempBuff, nTagSize);

    return TRUE;

}
#endif  /*(FFXCONF_NAND_512BEMULATED || FFXCONF_NAND_512BORNANDEMULATED) */


/*-------------------------------------------------------------------
    Local: NFCPageIn()

    Transfer a 512+16 byte page from the NAND chip to memory.

    Transfer a 512-byte main page and (optionally) 16-byte spare area from
    the NAND chip through the controller's RAM buffer to memory.

    There is no checking of ECCs or interpretation of the data.

    Parameters:
        pMain  - 512-byte buffer to receive main page data.
        pSpare - 16-byte buffer to receive spare area data; may be NULL
                 if spare data is not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCPageIn(void *pMain, void *pSpare)
{
    D_BOOL result = TRUE;
    static D_UINT32 aulBounce[4];

    /*  Trigger the transfer from the NAND chip to the controller's RAM
        buffer.  Writing NAND_FLASH_CONFIG1 with the saved value sets or
        clears ECC_EN as appropriate for the operation, and always clears
        SP_EN.  Writing NAND_FLASH_CONFIG2 starts the transfer.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDO_PAGE);

    /*  The controller waits for the NAND chip's Ready/Busy signal to indicate
        that it has data ready to transfer, then transfers it from the chip to
        its internal RAM buffer.  When the entire main page and spare area
        transfer is done, it sets INT in NAND_FLASH_CONFIG2.
    */
    if (NFCWait())
    {
        D_UINT32 *pCtrlrRam = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];
        unsigned int i;

        /*  The buffer for main page data may be unaligned.
        */
        if (DCLISALIGNED((D_UINTPTR)pMain, sizeof(D_UINT32)))
        {
            /*  Aligned properly, copy directly into it.
            */
            D_UINT32 *pBuffer = pMain;
            for (i = 0; i < MAIN_BUFFER_SIZE / sizeof (D_UINT32); i += 4)
            {
                pBuffer[i+0] = DCLMEMGET32(pCtrlrRam + (i+0));
                pBuffer[i+1] = DCLMEMGET32(pCtrlrRam + (i+1));
                pBuffer[i+2] = DCLMEMGET32(pCtrlrRam + (i+2));
                pBuffer[i+3] = DCLMEMGET32(pCtrlrRam + (i+3));
            }
        }
        else
        {
            /*  Unaligned, use a local bounce buffer to align it.
            */
            D_UCHAR *pBuffer = pMain;
            for (i = 0; i < MAIN_BUFFER_SIZE / sizeof (D_UINT32); i += 4)
            {
                aulBounce[0] = DCLMEMGET32(pCtrlrRam + (i+0));
                aulBounce[1] = DCLMEMGET32(pCtrlrRam + (i+1));
                aulBounce[2] = DCLMEMGET32(pCtrlrRam + (i+2));
                aulBounce[3] = DCLMEMGET32(pCtrlrRam + (i+3));
                DclMemCpy(pBuffer + i * sizeof (D_UINT32), aulBounce, sizeof aulBounce);
            }
        }

        if (pSpare)
        {
            pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];

            /*  The buffer for spare data may be unaligned.
            */
            if (DCLISALIGNED((D_UINTPTR)pSpare, sizeof(D_UINT32)))
            {
                /*  Aligned properly, copy directly into it.
                */
                D_UINT32 *pBuffer = pSpare;
                pBuffer[0] = DCLMEMGET32(pCtrlrRam + 0);
                pBuffer[1] = DCLMEMGET32(pCtrlrRam + 1);
                pBuffer[2] = DCLMEMGET32(pCtrlrRam + 2);
                pBuffer[3] = DCLMEMGET32(pCtrlrRam + 3);
            }
            else
            {
                /*  Unaligned, use a local bounce buffer to align it.
                */
                aulBounce[0] = DCLMEMGET32(pCtrlrRam + 0);
                aulBounce[1] = DCLMEMGET32(pCtrlrRam + 1);
                aulBounce[2] = DCLMEMGET32(pCtrlrRam + 2);
                aulBounce[3] = DCLMEMGET32(pCtrlrRam + 3);
                DclMemCpy(pSpare, aulBounce, sizeof aulBounce);
            }
        }

        result = TRUE;
    }
    else
    {
        FFXPRINTF(1, ("NFCPageIn: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  If the NAND chip itself timed out on a read, it may need to be
            reset to return to a usable state.  If this fails, there's not
            much to be done for it here, so ignore the status of this
            operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCPageOut()

    Transfer a 512+16 byte page from memory to the NAND chip.

    Transfer a 512-byte main page and (optionally) 16-byte spare area from
    memory to the NAND chip through the controller's RAM buffer.

    There is no processing of the data.  ECC generation may or may not
    occur, depending on how the transfer was set up outside this function.

    Parameters:
        pMain  - 512-byte buffer containing the main page data.
        pSpare - 16-byte buffer containing the spare area data; may be
                 NULL if spare data is not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCPageOut(const void *pMain, const void *pSpare)
{
    D_BOOL result = TRUE;
    D_UINT32 *pCtrlrRam;
    static D_UINT32 aulBounce[4];
    unsigned int i;

    pCtrlrRam = NFC_MAIN_BUFFER[XFER_BUFFER_NUMBER];

    /*  The buffer for main page data may be unaligned.
    */
    if (DCLISALIGNED((D_UINTPTR)pMain, sizeof(D_UINT32)))
    {
        /*  Aligned properly, copy directly from it.
        */
        const D_UINT32 *pBuffer = pMain;
        for (i = 0; i < MAIN_BUFFER_SIZE / sizeof (D_UINT32); i += 4)
        {
            DCLMEMPUT32(pCtrlrRam + (i+0), pBuffer[i+0]);
            DCLMEMPUT32(pCtrlrRam + (i+1), pBuffer[i+1]);
            DCLMEMPUT32(pCtrlrRam + (i+2), pBuffer[i+2]);
            DCLMEMPUT32(pCtrlrRam + (i+3), pBuffer[i+3]);
        }
    }
    else
    {
        /*  Unaligned, use a local bounce buffer to align it.
        */
        const D_UCHAR *pBuffer = pMain;
        for (i = 0; i < MAIN_BUFFER_SIZE / sizeof (D_UINT32); i += 4)
        {
            DclMemCpy(aulBounce, pBuffer + i * sizeof (D_UINT32), sizeof aulBounce);
            DCLMEMPUT32(pCtrlrRam + (i+0), aulBounce[0]);
            DCLMEMPUT32(pCtrlrRam + (i+1), aulBounce[1]);
            DCLMEMPUT32(pCtrlrRam + (i+2), aulBounce[2]);
            DCLMEMPUT32(pCtrlrRam + (i+3), aulBounce[3]);
        }
    }

    if (!pSpare)
    {
        /*  The caller did not supply spare data, use our own spare buffer.
        */
        pSpare = &UnusedSpare.data;
    }

    pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];

    /*  The buffer for spare data may be unaligned.
    */
    if (DCLISALIGNED((D_UINTPTR)pSpare, sizeof(D_UINT32)))
    {
        /*  Aligned properly, copy directly from it.
        */
        const D_UINT32 *pBuffer = pSpare;
        DCLMEMPUT32(pCtrlrRam + 0, pBuffer[0]);
        DCLMEMPUT32(pCtrlrRam + 1, pBuffer[1]);
        DCLMEMPUT32(pCtrlrRam + 2, pBuffer[2]);
        DCLMEMPUT32(pCtrlrRam + 3, pBuffer[3]);
    }
    else
    {
        /*  Unaligned, use a local bounce buffer to align it.
        */
        DclMemCpy(aulBounce, pSpare, sizeof aulBounce);
        DCLMEMPUT32(pCtrlrRam + 0, aulBounce[0]);
        DCLMEMPUT32(pCtrlrRam + 1, aulBounce[1]);
        DCLMEMPUT32(pCtrlrRam + 2, aulBounce[2]);
        DCLMEMPUT32(pCtrlrRam + 3, aulBounce[3]);
    }

    /*  Trigger the transfer from the controller's RAM buffer to the NAND
        chip.  Writing NAND_FLASH_CONFIG1 with the saved value sets or clears
        ECC_EN as appropriate for the operation, and always clears SP_EN.
        Writing NAND_FLASH_CONFIG2 starts the transfer.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDI);

    if (!NFCWait())
    {
        FFXPRINTF(1, ("NFCPageOut: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  The NAND chip is in the middle of some sort of write command
            sequence, and needs to be reset to be reset to return to a usable
            state.  If this fails, there's not much to be done for it here, so
            ignore the status of this operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCSpareIn()

    Transfer a 16-byte spare area from the NAND chip through the
    controller's RAM buffer to memory.

    There is no checking of ECCs or interpretation of the data.

    Parameters:
        pMain  - 512-byte buffer to receive main page data, aligned
                 on a 4-byte boundary.
        pSpare - 16-byte buffer to receive spare area data, aligned
                 on a 4-byte boundary; may be NULL if spare data is
                 not desired.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCSpareIn(void *pSpare)
{
    D_BOOL result = TRUE;

    /*  Trigger the transfer from the NAND chip to the controller's RAM
        buffer.  Writing NAND_FLASH_CONFIG1 with the saved value sets or
        clears ECC_EN as appropriate for the operation; set SP_EN here.
        Writing NAND_FLASH_CONFIG2 starts the transfer.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1 | SP_EN);
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDO_PAGE);

    /*  The controller waits for the NAND chip's Ready/Busy signal to indicate
        that it has data ready to transfer, then transfers it from the chip to
        its internal RAM buffer.  When the entire main page and spare area
        transfer is done, it sets INT in NAND_FLASH_CONFIG2.
    */

    if (NFCWait())
    {
        D_UINT32 *pBuffer = pSpare;
        D_UINT32 *pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];

        pBuffer[0] = DCLMEMGET32(pCtrlrRam);
        pBuffer[1] = DCLMEMGET32(pCtrlrRam + 1);
        pBuffer[2] = DCLMEMGET32(pCtrlrRam + 2);
        pBuffer[3] = DCLMEMGET32(pCtrlrRam + 3);

        result = TRUE;
    }
    else
    {
        FFXPRINTF(1, ("NFCSpareIn: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  If the NAND chip itself timed out on a read, it may need to be
            reset to return to a usable state.  If this fails, there's not
            much to be done for it here, so ignore the status of this
            operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCSpareOut()

    Transfer a 16-byte spare area from memory to the NAND chip through the
    controller's RAM buffer.

    There is no processing of the data.  ECC generation may or may not
    occur, depending on how the transfer was set up outside this function.

    Parameters:
        pSpare - 16-byte buffer to receive spare area data, aligned
                 on a 4-byte boundary.

    Return Value:
        TRUE if the controller successfully transferred the data from the
        NAND chip; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCSpareOut(const void *pSpare)
{
    D_BOOL result = TRUE;
    const D_UINT32 *pBuffer;
    D_UINT32 *pCtrlrRam;

    pBuffer = pSpare;
    pCtrlrRam = NFC_SPARE_BUFFER[XFER_BUFFER_NUMBER];
    DCLMEMPUT32(pCtrlrRam + 0, pBuffer[0]);
    DCLMEMPUT32(pCtrlrRam + 1, pBuffer[1]);
    DCLMEMPUT32(pCtrlrRam + 2, pBuffer[2]);
    DCLMEMPUT32(pCtrlrRam + 3, pBuffer[3]);

    /*  Trigger the transfer from the controller's RAM buffer to the NAND
        chip.  Writing NAND_FLASH_CONFIG1 with the saved value sets or clears
        ECC_EN as appropriate for the operation; set SP_EN here.  Writing
        NAND_FLASH_CONFIG2 starts the transfer.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1 | SP_EN);
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FDI);

    if (!NFCWait())
    {
        FFXPRINTF(1, ("NFCSpareOut: NFC timeout\n"));

        /*  Who knows what state the controller is in after a timeout?  Try
            to restore it to sanity.
        */
        NFCReset();

        /*  The NAND chip is in the middle of some sort of write command
            sequence, and needs to be reset to return to a usable
            state.  If this fails, there's not much to be done for it here, so
            ignore the status of this operation.
        */
        (void) NFCCommandOut(CMD_RESET);
        result = FALSE;
    }

    return result;
}


/*-------------------------------------------------------------------
    Local: NFCCommandOut()

    Send a command byte to the NAND chip through the controller,
    and wait for it to be accepted.

    Parameters:
        uCommand - the command byte to send

    Return Value:
        TRUE if the controller accepted the command; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCCommandOut(D_UCHAR uCommand)
{
    DCLMEMPUT16(NAND_FLASH_CMD, uCommand);
    DCLMEMPUT16(NAND_FLASH_CONFIG2, FCMD);
    return NFCWait();
}


/*-------------------------------------------------------------------
    Local: NFCAddressOut()

    Split an address into individual bytes and send them to the NAND chip.

    Parameters:
        ulAddress - the address to send
        uCount    - the number of address cycles to generate, must not
                    be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCAddressOut(D_UINT32 ulAddress, unsigned uCount)
{
    DclAssert(uCount != 0);
    DclAssert(uCount <= sizeof(ulAddress));

    do
    {
        DCLMEMPUT16(NAND_FLASH_ADD, (D_UCHAR) ulAddress);
        DCLMEMPUT16(NAND_FLASH_CONFIG2, FADD);
        ulAddress >>= 8;
        if (!NFCWait())
            break;
    }
    while (--uCount);
    return (uCount == 0);
}

/*-------------------------------------------------------------------
    Local: NFCRowOut()

    Send a row address to the NAND chip.

    Parameters:
        ulRow  - the ulRow address to send
        uCount - the number of row address cycles to generate,
                 must not be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCRowOut(D_UINT32 ulRow, unsigned uCount)
{
    DclAssert(uCount != 0);

    return NFCAddressOut(ulRow, uCount);
}

/*-------------------------------------------------------------------
    Local: NFCColumnOut()

    Send a column address to the NAND chip.

    If necessary, shift the column address to account for the
    NAND chip width, then send the address out.

    Parameters:
        ulColumn - the column address to send
        uCount   - the number of column address cycles to generate,
                   must not be zero

    Return Value:
        TRUE if the controller accepted the address; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_BOOL NFCColumnOut(D_UINT32 ulColumn, unsigned uCount)
{
    D_UINT32 ulAddress;
    DclAssert(uCount != 0);

    /*  Convert to width-specific column address
        (column addresses are either byte index or word index,
        depending on the bus width)
    */
    ulAddress = ulColumn >> (NFC.ucWidthBytes - 1);

    return NFCAddressOut(ulAddress, uCount);
}

/*-------------------------------------------------------------------
    Local: NFCReadyWait()

    Wait for the controller to indicate completion of an operation that
    requires waiting for the NAND chip to indicate is is ready.  These
    operations typically take long enough (tens to thousands of
    microseconds) for it to be worth setting a timer.

    Parameters:
        None

    Return Value:
        TRUE if the chip operation finished; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_UINT32 sulReadyWaitLoopsMax = 0;
static D_UINT32 sulReadyWaitEntries = 0;
static D_BOOL NFCReadyWait(void)
{
    DCLTIMER t;
    D_UINT32 ulReadyWaitLoops = 0;
    D_UINT16 uFlashConfig2Val;

    sulReadyWaitEntries++;

    DclTimerSet(&t, NAND_TIMEOUT);
    do
    {
        uFlashConfig2Val = DCLMEMGET16(NAND_FLASH_CONFIG2);
        if ((uFlashConfig2Val & INT) != 0)
        {
            /*  Per the specification, the operation bits should be cleared
                when INT is set.  Sanity check that here.
            */
            DclAssert(uFlashConfig2Val == INT);
            return TRUE;
        }
        ulReadyWaitLoops++;
        if (ulReadyWaitLoops > sulReadyWaitLoopsMax)
        {
            sulReadyWaitLoopsMax = ulReadyWaitLoops;
        }


    }
    while (!DclTimerExpired(&t));

    uFlashConfig2Val = DCLMEMGET16(NAND_FLASH_CONFIG2);
    if ((uFlashConfig2Val & INT) != 0)
    {
        /*  Per the specification, the operation bits should be cleared
            when INT is set.  Sanity check that here.
        */
        DclAssert(uFlashConfig2Val == INT);
        return TRUE;
    }

    FFXPRINTF(1, ("NFCReadyWait: timeout\n"));
    /*  Generally speaking, if NAND flash operations are timing out,
        something is wrong with the code or the system (rather than one
        of the expected failure cases for NAND) and needs to be debugged.
    */
    DclAssert(FALSE);
    return FALSE;
}


#if NFC_HAS_RESET
/*-------------------------------------------------------------------
    Local: NFCReset()

    Reset the NFC itself (as opposed to the NAND chip).

    Parameters:
        None

    Return Value:
        None.
-------------------------------------------------------------------*/
static void NFCReset(void)
{
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC_RST);

    /*  The specs are silent on whether controller reset takes any time.
        Based on testing, it does not appear to set INT when it is complete.
        Assume for now that the reset is essentially instantaneous.
    */

    /*  Set NAND_FLASH_CONFIG1 again, it's cleared by the reset command above.
    */
    DCLMEMPUT16(NAND_FLASH_CONFIG1, NFC.uConfig1);
}
#endif


/*-------------------------------------------------------------------
    Local: NFCWait()

    Wait for the controller to indicate completion of an operation.

    Note that depending on circumstances and OS overhead, this function
    may exit immediately or could possibly wait up to several milliseconds.

    We use a three-stage approach to reduce latency and overhead,
    but the three stages are based on assumptions that must be
    tuned or validated in any particular system in order to be of
    the most benefit.  Note this function will behave correctly
    regardless of any tuning, and the difference between the
    worst case and best case is comparitively small, but
    relevant (10% for writes on the system I tested it on.)

    Parameters:
        None.

    Return Value:
        TRUE if the controller operation finished; FALSE if this timed out.
-------------------------------------------------------------------*/
static D_UINT32 sulNFCWaitLoopsMax = 0;
static D_UINT32 sulNFCWaitEntries = 0;
static D_UINT32 sulNFCWaitShorts = 0;
static D_BOOL NFCWait(void)
{
    D_UINT16 uFlashConfig2Val;

    sulNFCWaitEntries++;

    {
        /*  Stage one:  Just check it.  If it's already set, don't bother
            with the loops.
        */
        uFlashConfig2Val = DCLMEMGET16(NAND_FLASH_CONFIG2);
        if ((uFlashConfig2Val & INT) != 0)
        {
            /*  Per the specification, the operation bits should be cleared
                when INT is set.  Sanity check that here.
            */
            sulNFCWaitShorts++;
            DclAssert(uFlashConfig2Val == INT);
            return TRUE;
        }
    }

    {
        /*  Stage two: Check it in a counted loop.  If it gets set quickly,
            don't bother with the loop that calls into the OS for timer info
        */

        /*  Note this implementation auto-tunes the wait loop for best
            performance, entering the timed loop only until the count is
            calibrated, and only thereafter on a worst-case or catestrophic
            failure.
        */
        long timeout = sulNFCWaitLoopsMax;
        D_UINT32 ulNFCWaitLoops = 0;

        do
        {
            uFlashConfig2Val = DCLMEMGET16(NAND_FLASH_CONFIG2);
            if ((uFlashConfig2Val & INT) != 0)
            {
                /*  Per the specification, the operation bits should be cleared
                    when INT is set.  Sanity check that here.
                */
                DclAssert(uFlashConfig2Val == INT);
                return TRUE;
            }
            ulNFCWaitLoops++;
            if (ulNFCWaitLoops > sulNFCWaitLoopsMax)
            {
                sulNFCWaitLoopsMax = ulNFCWaitLoops;
            }
        }
        while (--timeout);
    }

    {
        /*  Stage three: it hasn't gotten set for 'a while' now,
            assume it's not going to be set for 'a longer while',
            and use a timed loop to get a valid, CPU clock-independant
            timed wait.
        */
        if (NFCReadyWait())
        {
            return TRUE;
        }
    }



    FFXPRINTF(1, ("NFCWait: timeout\n"));

    /*  Generally speaking, if operations time out, there is something
        wrong with the system (as opposed to the specific expected failures
        of operations on NAND flash) that needs to be debugged.
    */
    DclAssert(FALSE);
    return FALSE;
}


static void SetEccMode(D_BOOL fUseEcc)
{
    if (fUseEcc)
        NFC.uConfig1 |= ECC_EN;
    else
        NFC.uConfig1 &= ~ECC_EN;
}


#endif /* FFXCONF_NANDSUPPORT */
