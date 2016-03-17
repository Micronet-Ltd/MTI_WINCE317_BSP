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

    This module contains the OneNAND NAND Technology Module (NTM) which
    supports page oriented I/O to OneNAND flash.  It interfaces with the
    FlashFX Hooks module fh1nand.c, which allows easy project level
    customization of the interface without requiring a new NTM to be
    written.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntflex1nand.c $
    Revision 1.5  2010/12/10 20:04:17Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.4  2010/01/27 04:27:37Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.3  2009/12/11 23:56:01Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Updated some typecasts which are necessary while migrating
    to the more flexible hooks model.
    Revision 1.2  2009/10/06 19:13:55Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.1  2009/08/01 02:11:42Z  garyp
    Initial revision
    Revision 1.65  2009/04/09 03:42:56Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.64  2009/04/01 20:50:24Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.63  2009/03/26 19:20:47Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.62  2009/03/12 22:02:35Z  keithg
    Added static to function declarations that were defined as such.
    Revision 1.61  2009/03/06 22:54:33Z  glenns
    - Fix Bugzilla #2471: Added code to detect if user wants to manage
      the SLC or MLC partition of a FlexOneNAND device, set the
      partition and adjust device parameters as necessary.
    - Added commentary to help users understand how to use FlashFX
      to manage FlexOneNAND partitions.
    - Repaired unfinished logic for FlexOneNAND to report bit corrections
      for use by the error manager.
    Revision 1.60  2009/03/04 18:45:20Z  glenns
    - Fix Bugzilla #2393: Removed reserved block processing.
      NOTE: some reserved page processing was preserved, as
      OneNAND has reason to do this internal to itself. See Bugzilla
      #2471.
    - Added code to properly report bit error corrections to the error
      manager.
    - Fixed spelling error in a macro name.
    Revision 1.59  2009/03/03 07:51:17Z  glenns
    - Fix Bugzilla #1334: Modified code to correct remaining LFA issues
      in OneNAND NTM.
    - Repair logic errors in calculation of bitmasks for DDP OneNAND
      devices.
    - Added  cautionary commentary about OneNAND NTM addressing
      mechanisms being wholly dependent on the number of blocks
      per chip and the number of pages per block even powers of 2.
    Revision 1.58  2009/03/01 01:51:06Z  glenns
    - Fix Bugzilla 2470: added code to pass device parameters from
      OneNANDInfo structure to NtmInfo structure.
    - Clarified comments.
    Revision 1.57  2009/01/26 22:40:53Z  glenns
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    Revision 1.56  2009/01/23 17:17:48Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.55  2009/01/16 05:31:06Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in eight places.
    Revision 1.54  2009/01/12 21:14:17Z  billr
    Update copyright date.
    Revision 1.53  2009/01/12 20:59:05Z  billr
    Fix test for uncorrectable error (found by gcc -Wempty-body).
    Revision 1.52  2008/10/09 20:37:26Z  glenns
    - Changed last checkin for new printf to display two hex characters
      instead of just one.
    Revision 1.51  2008/10/08 22:22:45Z  glenns
    - Fixed alignment exception in an FXTRACEPRINTF macro thrown
      from OneNANDNtmTagDecode when the DEBUG level was set
      to 2.
    Revision 1.50  2008/10/02 23:03:48Z  glenns
    - Added support for OneNAND ability to report
      uncorrectable ECC faults in main page and spare
      area separately. This is supported for OneNAND
      only; FlexOneNAND does not have this ability.
    - Modified PageRead and HiddenRead to use the
      support mentioned above.
    - Fixed a bug introduced in the last checkin where
      offsets into the tag buffer are miscalculated if
      an uncorrectable error occurs when reading a
      groupt of tags (HiddenRead).
    - Fixed a bug that has existed in this NTM since
      its inception where if an error occurs on the very
      first page in a standard read or a standard write,
      the ulCount field is set incorrectly.
    Revision 1.49  2008/09/24 22:56:03Z  glenns
    - Added code to PageRead and HiddenRead
      to properly zero variable width tags
      if uncorrectable errors occur, and only
      for those tags.
    - Fixed GetPageStatus to account for the
      unique situation presented by OneNAND
      spare area format.
    - Fixed originating assertion failure from
      Bugzilla #2116.
    - Added OneNANDTagEncode and OneNANDTagDecode
      procedures so we can include a checkbyte
      with tags. Eliminates false corrections as
      described in Bugzilla 2116.
    - Fixed FlexNTMRead to be sure page reads
      with errors in them get counted, so that
      the correct tags get zeroed upon return.
    - Fix BuildSpareArea to allow functionality
      to proceed with null tag data.
    Revision 1.48  2008/09/02 05:59:47Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.47  2008/07/23 20:13:38Z  keithg
    Added type casts to placate older MSVS tools.
    Revision 1.46  2008/06/17 16:10:49Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.45  2008/05/30 16:00:31Z  garyp
    Fixed mismatched trace and profiler directives.
    Revision 1.44  2008/05/01 20:34:57Z  Glenns
    Extensive changes:
    - Added support for FlexOneNAND devices.
    - Fixed commentary style.
    - Rewrote GetPageStatus to accomodate OneNAND Hardware ECC mechanism.
    - Fixed up and added missing function comment blocks to support Autodoc.
    - Cleaned up code
    Revision 1.43  2008/03/23 18:59:10Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.42  2008/02/03 05:26:53Z  keithg
    comment updates to support autodoc
    Revision 1.41  2007/12/12 18:21:34Z  thomd
    Correct problem with failing hardware ECC tests.
    Revision 1.40  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.39  2007/10/22 23:28:40Z  pauli
    Corrected an inverted logic condition when determining if BBM is required.
    Updated to properly support the "Raw" NTM API.
    Revision 1.38  2007/09/25 22:16:33Z  pauli
    Resolved Bug 1455: Updated to use new NTM helper functions.  Resolved Bug
    1441: Correctly use OffsetZero format with SW ECC.  Added the ability to
    report that BBM should or should not be used based on the characteristics
    of the NAND part identified.  Added checks to ensure that the identified
    NAND part is supported.
    Revision 1.37  2007/09/13 20:34:41Z  jimmb
    Broke up the #if FFXCONF_NANDSUPPORT && if FFXCONF_NANDSUPPORT_ONENAND
    into 2 separate conditions. That allows nandid.h to correctly defined the
    required conditions for FFXCONF_NANDSUPPORT_ONENAND.
    Revision 1.36  2007/09/12 21:14:03Z  Garyp
    Updated to use some renamed functions.
    Revision 1.35  2007/08/03 00:29:59Z  timothyj
    Fixed missing parenthesis.
    Revision 1.34  2007/08/02 22:25:57Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.33  2007/04/07 03:25:35Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.32  2007/03/01 20:25:00Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.31  2007/02/13 22:27:30Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.30  2007/02/05 20:18:42Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.29  2007/01/04 00:03:28Z  Timothyj
    Modified to use new global OneNANDFindChip() that returns a reference to a
    constant FFXNANDCHIP from the table where the ID was located (removed
    OneNANDDecodeID).
    Revision 1.28  2006/11/08 03:38:50Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.27  2006/10/04 00:56:10Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.26  2006/05/17 22:02:48Z  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    NtmInfo structure.
    Revision 1.25  2006/03/22 01:39:02Z  Garyp
    Updated the function header comments -- no functional changes.
    Revision 1.24  2006/03/16 21:44:50Z  timothyj
    Removed assert that required a pTagBuff when writing pages (pTagBuff is
    optional).
    Revision 1.23  2006/03/10 03:03:29Z  Garyp
    Updated to use a modified NTM helper function.
    Revision 1.22  2006/03/09 21:19:41Z  timothyj
    Standardized comments and FFXTRACEPRINTF strings.  Removed unused
    unnecessary ONENANDINFO parameter.
    Revision 1.21  2006/03/08 03:01:19Z  Garyp
    Added parens to avoid a compiler warning.
    Revision 1.20  2006/03/08 01:42:35Z  timothyj
    Added RawPage{Read|Write}
    Changed deprecated FFXSTAT_FIMERASEFAILED to FFXSTAT_FIM_ERASEFAILED.
    Revision 1.19  2006/03/07 02:56:00Z  timothyj
    Revised pChipClass to pChip for FfxNtm helper function changes.  Added
    support for HW ECC.  Cleaned up and standardized capitalization,
    indentation, and positioning of 'const' qualifier.  Removed extra call to
    select the OneNAND data buffer.  Added additional trace printing.  Added
    code to map specific errors from the OneNAND controller status register to
    FFX status codes.  Added support for several different densities and
    organizations (128 and 256 Mb, 1KB pages).
    Revision 1.18  2006/03/02 02:38:37Z  timothyj
    HW ECC Support:
      Moved and consolidated inline code into new function OneNANDBuildSpareArea().
      Moved and consolidated inline code into new functions OneNAND{Get|Set}Tag().
      Added code to check for HW ECC successfully corrected pages and indicate
        the outcome in the page status.
      Added preliminary support for HW ECC, including code to dynamically
        enable/disable ECCs for handing fUseEcc parameter.
    DDP Support:
       Added code to determine whether the device is a Dual Die Package,
        based on the ID.
    Cleanup:
      Modified the names of some parameters to match the Page I/O NTM.
      Modified code that was using a union for alignment to use the new
        DCLALIGNEDBUFFER macro.
    Revision 1.17  2006/02/28 23:30:02Z  timothyj
    Changed allocation entry type to D_BUFFER from FFXALLOC, with other
    corresponding changes to remove dependancy on allocation entry structure.
    Added GetPageStatus API.  Changed to use NAND-specific rather than
    deprecated non-specific macros.  For clarity, modifed functions that
    compute offset to take the buffer size parameters in bytes rather than
    ONENAND_IO_TYPE.
    Revision 1.16  2006/02/25 02:40:40Z  timothyj
    Removed extra page size and spare size structure members.  Updated many
    comments. Modified several functions to operate on pages (vs. sectors).
    Isolated reference to the spare area to two functions to facilitate
    changing between HW and SW ECC mechanisms.  Fixed duplicate addition of
    ulReservedSize.  Removed several functions that were performing simple
    calculations (moved calculations inline).
    Revision 1.15  2006/02/24 04:25:12Z  Garyp
    Updated to use refactored headers.
    Revision 1.14  2006/02/23 23:40:00Z  Garyp
    Moved the meat of SpareRead/Write() into helper functions so that they
    may be used by various NTM functions which expect the flash offset to
    already be adjusted by reserved space.
    Revision 1.13  2006/02/23 02:58:13Z  Garyp
    Added some missing static declarations.
    Revision 1.12  2006/02/23 01:55:33Z  Pauli
    Removed extraneous trace printf parameter.
    Revision 1.11  2006/02/23 00:10:48Z  joshuab
    Fixing up casts for ce4/x86 build.  Evidently the X86 compiler is pickier
    than the ARM one.
    Revision 1.10  2006/02/22 23:29:30Z  timothyj
    Removed incorrect extra division of ulOffset by the page size (to yield a
    count, but passed into a called function as an offset).  Fixed Indent/Undent
    mismatch in calls to FFXTRACEPRINTF().  Changed 'NTBYTEIO' strings to
    'NT1NAND'.  Changed 'PageRead' strings to 'OneNANDNTMPageRead'.  Fixed
    format specifiers to be compatible with FFXTRACEPRINTF().  Added code to
    fill the rest of the spare area with 0xFF to prevent incorrectly clearing
    bits outside the hidden area in HiddenWrite.  Added casts to conversion of
    units from bytes to ONENAND_IO_TYPES.
    Revision 1.9  2006/02/22 01:03:56Z  timothyj
    Fixed problem where the second OneNAND SRAM databuffer address was computed
    incorrectly, causing every other buffer to be read or written from
    uninitialized values.  Added additional instrumentation.  Modified trace
    level values for some existing instrumentation.  Fixed mismatched Indent/
    Undent indentation.
    Revision 1.8  2006/02/21 22:03:33Z  Garyp
    Minor tweaks to work with the new interface.  Fixed Get/SetBlockStatus()
    to adjust the flash offset by ulReservedSpace.  Commented out a seemingly
    bogus similar adjustment in SpareWrite().
    Revision 1.7  2006/02/18 05:51:40Z  timothyj
    Removed extra DclAssert() for optional pBuffer parameter to
    OneNANDNTMWrite().  Added additional instrumentation.  Added if (pBuffer)
    checks where pBuffer is optional.  Removed 14 byte offset from allocation
    entry location in the spare area.
    Revision 1.6  2006/02/18 05:08:13Z  timothyj
    Removed bMain parameters from several functions, as the presence of a
    pBuffer implies (indeed requires) this.  Removed extra ulTotalBlocks field
    and corresponding reference that was resulting in a divide-by-zero. Removed
    remaining error injection tests (perhaps temporarily).  Removed incorrect
    DclAssert()s for parameters that are optional.  Added instrumentation.
    Revision 1.5  2006/02/18 02:46:08Z  timothyj
    Updated to latest NTM interfaces, including:
      Changed fmlapi.h to fxfmlapi.h
      Removed oem.h
      Removed all references to EXTMEDIAINFO
    Modified internal/private NTMINFO structure to be an NTMDATA structure (and
    NTMHANDLE pointer), and reference the new public NTMINFO structure.  Added
    fUseEcc parameter in lieu of referencing the flag in the EXTMEDIAINFO
    structure.  Modified array bounds computation to use the updated
    FfxDev{Get|Set}ArrayBounds functions.  Added code to fill in some
    information in the NTMINFO structure used by the upper layers.  Added
    implementations of new ParameterGet() and ParameterSet() functions.  Added
    new flags to FFXIOSTATUS structure initialization.  Added additional
    instrumentation (DclAssert, FFXTRACEPRINTF).  Removed synchronous burst
    mode read flag setting (temporarily).  Removed disabling of hardware ECC
    (ie. enabled hardware ECC).  Added several devices to the table of OneNAND
    devices supported by this NTM (in particular added the one I am testing
    with, stubbed several others).
    Revision 1.4  2006/02/11 03:08:22Z  Garyp
    Updated the NTM name.
    Revision 1.3  2006/02/07 22:49:31Z  timothyj
    Modifed some calls to functions with D_UINT16 parameter types to eliminate
    compiler warnings from MSVC6.
    Revision 1.2  2006/02/04 01:37:54Z  brandont
    Added static to OneNANDNTMRead() and OneNANDNTMWrite() function prototypes
    to match the implementation.  Removed an unused variable in
    OneNANDLoadFinalize.
    Revision 1.1  2006/02/03 01:50:20Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT
#include <fxnandapi.h>
#include "nandid.h"
#include "nand.h"

#include <deverr.h>
#include <ecc.h>
#include <fxdevapi.h>

#if FFXCONF_NANDSUPPORT_ONENAND

#include <onenandconf.h>
#include <onenandctl.h>
#include <onenandregs.h>
#include <errmanager.h>
#include "ntm.h"

#define NTMNAME "NTFLEX1NAND"

#define ONENAND_SECTOR_MAIN_SIZE        (512)
#define ONENAND_SECTOR_SPARE_SIZE        (16)
#define ONENAND_ID_DEVICE_INDEX           (1)
#define ONENAND_ID_SIZE                   (4)
#define ONENAND_SECTOR_TAG_OFFSET         (2)
#define ONENAND_ECCSTATUS_UNCORRECTABLE_BITMASK         (0xAAAA)
#define ONENAND_ECCSTATUS_UNCORRECTABLEMAIN_BITMASK     (0x8888)
#define ONENAND_ECCSTATUS_UNCORRECTABLESPARE_BITMASK    (0x2222)
#define ONENAND_LOWEST_BYTE_MASK        (0xFF)

/* OneNAND Silicon technologies:
*/
#define ONENAND_FLEX                    (0x02)
#define ONENAND_SLC                     (0x00)
#define ONENAND_MLC                     (0x01)

#define ONENAND_ECC_OFFSET              (9)
#define ONENAND_ECC_SIZE                (5)

/* Support macros for FlexOneNAND:
*/
#define FLEX_ECC_STATUS_MASK    0x001F
#define FLEX_ECC_HIBYTE_SHIFT 0x08
#define FLEX_ECC_MAX_CORRECTABLE_ERROR 0x08
#define FLEXONENAND_ECC_OFFSET 7
#define FLEXONENAND_ECC_SIZE 10



typedef struct ONENANDINFO
{
    /*  Size of the OneNAND databuffer, in bytes
        This value is also typically read from the chip at initialization
    */
    D_UINT16 uDataBufferSize;

    /*  Size of the OneNAND sparebuffer, in bytes
        This value is also typically read from the chip at initialization
    */
    D_UINT16 uSpareBufferSize;

    /*  Number of sectors per page.
        This value is typically looked up in a table based on the ID.
    */
    D_UINT16 uSectorsPerPage;

    /*  Number of OneNAND databuffers per page. Equal to "1"
        in first-generation SLC, but later generations and OneNAND
        tecnologies may use other values.
    */
    D_UINT16 uDataBuffersPerPage;

    /*  Number of data buffers in the part. If less than
        uDataBuffersPerPage, part supports writing to
        one buffer while the other programs flash array.
        Read from the chip at initialization.
    */
    D_UINT16 uNumDataBuffers;

    /*  Number of Boot RAM Buffers. Read from the part at
        initialization.
    */
    D_UINT16 uNumBootBuffers;

    /*  The partitioning information for a FlexOneNAND part. Read
        from the Partition Information block of Flex part. Set to
        zero for non-Flex part.
    */
    D_UINT16 uFlexPartition;

    /*  Number of blocks in this OneNAND part
        This value is typically looked up in a table based on the ID.
    */
    D_UINT16 uBlocks;

    /*  Size of the OneNAND block, in bytes
        This value is computed based on the pages per block and the page size
    */
    D_UINT32 ulBlockSize;

    /*  Bitmask to use to select the bit for the core from an address
    */
    D_UINT32 ulCoreAddrBitmask;

    /*  Bitmask to use to select the bits for the block from an address
    */
    D_UINT32 ulBlockAddrBitmask;

    /*  Bitmask to use to select the bits for the page from an address
    */
    D_UINT32 ulPageAddrBitmask;

    /*  Number of bit positions to right-shift the core address bits to get
        the core address
    */
    D_UINT32 ulCoreAddrShift;

    /*  Number of bit positions to right-shift the page index to get
        the block index
    */
    D_UINT32 ulBlockAddrShift;

    /*  Bitmask to use to select the core bit from a block address
    */
    D_UINT32 ulCoreBlockBitmask;

    /*  Bitmask to use to select the block bits from a block address
    */
    D_UINT32 ulBlockBlockBitmask;

    /*  Number of bit positions to right-shift the core bit to get
        the core address from a block address
    */
    D_UINT32 ulCoreBlockShift;

    /*  These fields are for propagating chip capabilities as defined in
        the nandid structures.
    */
    D_UINT16        uEdcRequirement;    /* Required level of error correction */
    D_UINT16        uEdcCapability;     /* Max error correcting capability */
    D_UINT32        ulEraseCycleRating;  /* Factory spec erase cycle rating */
    D_UINT32        ulBBMReservedRating; /* Reserved blocks required for above */

    /*  Indicates the size of the data covered by EDC in bytes
    */
    D_UINT16 uEdcSegmentSize;

    /*  Indicator of whether the system is making use of OneNAND's HW Ecc mechanism
    */
    D_BOOL fHWEcc;

    /*  Indicator of whether the part is a Flex part:
    */
    D_BOOL fIsFlexOneNAND;

    /*  Chip information
    */
    FFXNANDCHIP const *pChip;

    /*  Project-specific information (OneNAND control structure), shared
        between this module and the OneNAND project hooks
    */
    NTMHOOKHANDLE      hHook;

    /*  Contains OneNAND configuration parameters.
        It is filled in by the FfxHookNTOneNANDCreate() function.
    */
    FFXONENANDPARAMS onp;

} ONENANDINFO, *PONENANDINFO;

struct tagNTMDATA
{
    FFXDEVHANDLE    hDev;
    FFXNANDMFG      mfg;
    NTMINFO         NtmInfo;        /* Information visible to upper layers */
    ONENANDINFO    *pONI;           /* OneNAND-specific layer info (used within this module only) */
    D_UINT32        ulReservedPages;
    D_BYTE          abID[ONENAND_ID_SIZE];
};

/*  Internal helper functions used by this module and also provided to the
    NTM helper functions.
*/
static FFXIOSTATUS  ReadSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);

/*  Internal helper functions used to interface to the private OneNAND-specific
    layer implemented and used within this module
*/
static PONENANDINFO OneNANDCreate(   FFXDEVHANDLE hDev, NTMHANDLE hNTM, FFXFIMBOUNDS *pBounds);
static FFXSTATUS    OneNANDDestroy(  PONENANDINFO pONI);
static FFXIOSTATUS  OneNANDNTMRead(  NTMHANDLE    hNTM, D_UINT32 ulStartPage, D_BUFFER *pPageBuff, D_BUFFER *pSpareBuff, D_BUFFER *pTagBuff, D_UINT32  ulCount, unsigned nTagSize, D_BOOL fUseEcc);
static FFXIOSTATUS  OneNANDNTMWrite( NTMHANDLE    hNTM, D_UINT32 ulStartPage, const D_BUFFER *pPageBuff, const D_BUFFER *pSpareBuff, const D_BUFFER *pTagBuff, D_UINT32 ulCount, unsigned nTagSize, D_BOOL fUseEcc);
static FFXIOSTATUS  OneNANDEraseBlocks(PONENANDINFO pONI, D_UINT32 ulStartBlock, D_UINT32 uBlocks);


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_flexonenand =
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
------------------------------------------------------------------*/
static NTMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const NTMINFO     **ppNtmInfo)
{
    NTMHANDLE           hNTM = NULL;
    D_UINT32            ulTotalBlocks;
    D_BOOL              fSuccess = FALSE;
    FFXFIMBOUNDS        bounds;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT), NTMNAME"-Create()\n"));

    hNTM = DclMemAllocZero(sizeof *hNTM);
    if(!hNTM)
    {
         FFXPRINTF(1, (NTMNAME"-Create() Unable to create NTM - DclMemAlloc() failed.\n"));
    }
    else
    {
        hNTM->hDev = hDev;

        /*  Get the array bounds now so we know how far to scan
        */
        FfxDevGetArrayBounds(hDev, &bounds);

        hNTM->pONI = OneNANDCreate(hDev, hNTM, &bounds);
        if(!hNTM->pONI)
        {
             FFXPRINTF(1, (NTMNAME"-Create() Unable to create OneNAND Info - OneNANDCreate() failed.\n"));
        }
        else
        {
            /*  Get the reserved size, as well as the max array size,
                reducing TotalPhysicalSize as necessary.
            */
            ulTotalBlocks = FfxDevApplyArrayBounds(hNTM->pONI->uBlocks, hNTM->pONI->ulBlockSize, &bounds);
            if (ulTotalBlocks == D_UINT32_MAX)
            {
                 FFXPRINTF(1, (NTMNAME"-Create() Unable to apply bounds - FfxDevApplyArrayBounds() failed.\n"));
            }
            else
            {
                /*  Pass up some information in this layer to the layers above
                */
                hNTM->NtmInfo.uPageSize     = hNTM->pONI->uDataBufferSize * hNTM->pONI->uDataBuffersPerPage;
                hNTM->NtmInfo.uSpareSize    = hNTM->pONI->uSpareBufferSize;
                hNTM->NtmInfo.ulBlockSize   = hNTM->pONI->ulBlockSize;
                hNTM->NtmInfo.ulTotalBlocks  = ulTotalBlocks;
                
                /*  Range check on alignment boundary. Alignment is set to
                    CLIENT_BUFFER_ALIGNMENT in the hooks. It's hard to see
                    why anyone would do this, but it isn't illegal to set
                    the alignment larger than D_UINT16_MAX.
                */
                DclAssert(hNTM->pONI->onp.nAlignmentBoundary <= D_UINT16_MAX);
                hNTM->NtmInfo.uAlignSize    = (D_UINT16)(hNTM->pONI->onp.nAlignmentBoundary);
                hNTM->NtmInfo.uEdcRequirement = hNTM->pONI->uEdcCapability;
                hNTM->NtmInfo.uEdcCapability = hNTM->pONI->uEdcRequirement;
                hNTM->NtmInfo.uEdcSegmentSize = hNTM->pONI->uEdcSegmentSize;
                hNTM->NtmInfo.fEdcParameterized = FALSE;
                hNTM->NtmInfo.fEdcProtectsMetadata = TRUE;
                hNTM->NtmInfo.uCustomSpareFormat= FALSE;
                hNTM->NtmInfo.ulEraseCycleRating = hNTM->pONI->ulEraseCycleRating;
                hNTM->NtmInfo.ulBBMReservedRating = hNTM->pONI->ulBBMReservedRating;


                /*  Calculate this once and store it for use later for
                    both optimization and simplification of the code
                */
                DclAssert((hNTM->NtmInfo.ulBlockSize / hNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
                DclAssert((hNTM->NtmInfo.ulBlockSize % hNTM->NtmInfo.uPageSize) == 0);
                hNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(hNTM->NtmInfo.ulBlockSize / hNTM->NtmInfo.uPageSize);

                /*  Record the reserved space to use for this FIM.  All flash
                    offsets passed into this FIM will be relative to zero,
                    starting after this reserved space.
                */
                DclAssert((hNTM->NtmInfo.ulBlockSize % 1024UL) == 0);

                hNTM->NtmInfo.ulChipBlocks = hNTM->pONI->uBlocks;
                hNTM->NtmInfo.uMetaSize = FFX_NAND_TAGSIZE;
                hNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;
                if((hNTM->pONI->pChip->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
                    hNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
                if (hNTM->pONI->pChip->pChipClass->fProgramOnce)
                    hNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

                FFXPRINTF(1, (NTMNAME"- TotalBlocks=%lX BlockSize=%lX PhysPageSize=%X\n",
                    ulTotalBlocks,
                    hNTM->pONI->ulBlockSize,
                    hNTM->NtmInfo.uPageSize));

                FFXPRINTF(1, (NTMNAME": ReservedPages=%lX VirtualArrayBlocks=%lX\n",
                    hNTM->ulReservedPages,
                    ulTotalBlocks));

                /*  Pass back the information the upper layers use
                */
                *ppNtmInfo = &hNTM->NtmInfo;

                fSuccess = TRUE;
            }
        }
    }

/*  CreateCleanup:
*/

    if(hNTM && !fSuccess)
    {
        if(hNTM->pONI)
        {
            OneNANDDestroy(hNTM->pONI);
        }

        DclMemFree(hNTM);
        hNTM = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-Create() returning %P\n", hNTM));

    return hNTM;
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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0), NTMNAME"-Destroy()\n"));

    DclAssert(hNTM);

    OneNANDDestroy(hNTM->pONI);

    DclMemFree(hNTM);

    return;
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
    FFXIOSTATUS ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageRead() Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageRead", 0, 1);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
        above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    /*  Adjust the address in the event that there is reserved space
    */
    ulStartPage += hNTM->ulReservedPages;

    ioStat = OneNANDNTMRead(hNTM,
                            ulStartPage,
                            pPages,
                            NULL,
                            pTags,
                            ulCount,
                            nTagSize,
                            fUseEcc);

    /*  check for success:
    */
    if (!IOSUCCESS(ioStat,ulCount))
    {
        /*  Decode uncorrectable error results:
        */
        switch (ioStat.ffxStat)
        {
            case FFXSTAT_FIMUNCORRECTABLESPARE:

            /*  Spare-area-only uncorrectable errors are to be
                interpreted as invalidated tags.
            */
                if (pTags)
                {
                D_UINT32 ulIndex;

                    for (ulIndex = 0; ulIndex < nTagSize; ulIndex++)
                        pTags[(ioStat.ulCount*nTagSize) + ulIndex] = 0;
                }
                ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                break;
            case FFXSTAT_FIMUNCORRECTABLEMAIN:

                /*  Main page uncorrectable errors are in an area where
                    tags are not stored, so don't consider tags invalid.
                    However, must still propagate main page error upward.
                */
                ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                break;
            default:

                /*  Other errors simply get propagated upward.
                */
                break;
        }

    }

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageWrite() Page=%lX Count=%lX pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageWrite", 0, 1);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    /*  Adjust the address in the event that there is reserved space
    */
    ulStartPage += hNTM->ulReservedPages;

    ioStat = OneNANDNTMWrite(hNTM, ulStartPage, pPages, NULL, pTags, ulCount, nTagSize, fUseEcc);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-SpareRead() hNTM=%lX  ulPage=%lX (%u)  pSpare=%lX\n",
        hNTM,
        ulPage, ulPage,
        pSpare
        ));

    DclProfilerEnter(NTMNAME"-SpareRead", 0, 1);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    ioStat = ReadSpareArea(hNTM, ulPage, pSpare);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-SpareRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-SpareWrite() hNTM=%lX ulPage=%lX (%u)  pSpare=%lX\n",
        hNTM, ulPage, ulPage, pSpare ));

    DclProfilerEnter(NTMNAME"-SpareWrite", 0, 1);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    ioStat = WriteSpareArea(hNTM, ulPage, pSpare);

    /*  SpareWriteCleanup:
    */
    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS ioStat;
    D_UINT32 ulPagesDone = 0;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenRead() hNTM=%lX StartPage=%lX Count=%lX pTags=%P TagSize=%u\n",
        hNTM, ulStartPage, ulCount, pTags, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenRead", 0, ulCount);

    /*  Adjust the address in the event that there is reserved space
    */
    ulStartPage += hNTM->ulReservedPages;

    /*  Start reading the tags. Be sure to account for uncorrectable
        read errors, as these have implications for how to set the
        tag. Any other type of error will break out of this loop.
    */
    do
    {

        ioStat = OneNANDNTMRead(hNTM,
                                ulStartPage+ulPagesDone,
                                NULL,
                                NULL,
                                &(pTags[ulPagesDone*nTagSize]),
                                ulCount-ulPagesDone,
                                nTagSize,
                                TRUE);

        /*  Check for uncorrectable errors. If they exist when trying to get
            the tag, we must set the tag value to zero and propagate a status
            of SUCCESS up the chain. Remember to account for variable tag
    		width.
        */
        if (!IOSUCCESS(ioStat,ulCount))
        {

            switch (ioStat.ffxStat)
            {
                case FFXSTAT_FIMUNCORRECTABLEDATA:
                case FFXSTAT_FIMUNCORRECTABLESPARE:
                    {
                    /*  General or spare-area uncorrectable errors are to be
                        interpreted as invalidated tags.
                    */
                    D_UINT32 ulIndex;

                        for (ulIndex = 0; ulIndex < nTagSize; ulIndex++)
                            pTags[((ulPagesDone + ioStat.ulCount)*nTagSize) + ulIndex] = 0;

                        ioStat.ffxStat = FFXSTAT_SUCCESS;
						ioStat.ulCount++;
                        break;
                    }
                case FFXSTAT_FIMUNCORRECTABLEMAIN:
                    /*  Main page uncorrectable errors are in an area where
                        tags are not stored, so don't consider tags invalid.
                        However, must still propagate main page error upward.
                    */
                    ioStat.ffxStat = FFXSTAT_SUCCESS;
                    ioStat.ulCount++;
                    break;
                default:
                    /*  Other errors simply get propagated upward.
                    */
                    break;
            }

        }
        ulPagesDone += ioStat.ulCount;
    }while ((ulPagesDone < ulCount) && (ioStat.ffxStat == FFXSTAT_SUCCESS));

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenWrite() hNTM=%lX Page=%lX pTag=%P TagSize=%u\n",
        hNTM, ulPage, pTag, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenWrite", 0, 1);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    ioStat = OneNANDNTMWrite(hNTM, ulPage, NULL, NULL, pTag, 1, nTagSize, TRUE);

    /*  HiddenWriteCleanup:
    */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat;
    D_UINT32            ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-BlockErase() hNTM=%lX ulBlock=%lX\n", hNTM, ulBlock));

    DclProfilerEnter(NTMNAME"-BlockErase", 0, 1);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPagesPerBlock = (hNTM->NtmInfo.ulBlockSize / hNTM->NtmInfo.uPageSize);
    ulBlock += hNTM->ulReservedPages / ulPagesPerBlock;

    ioStat = OneNANDEraseBlocks(hNTM->pONI, ulBlock, 1);

    /*  BlockEraseCleanup:
    */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-BlockErase() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS     ioStat;

    DclAssert(hNTM);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    if (!(hNTM->pONI->fHWEcc))
    {
         /* If hardware ECC is not in use, we can use the standard method
            for examining the spare area to get the page status:
         */

        ioStat = FfxNtmHelpReadPageStatus(hNTM, &ReadSpareArea, ulPage, hNTM->pONI->pChip);
    }
    else
    {
    DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);

        DclAssert(buffer.ns);

        /*  If hardware ECC is in use, OneNAND and FlexOneNAND cannot
            use the standard GetPageStatus because the hardware ECC
            forces thespare area format not to comply with what
            FfxNtmHelpReadPageStatus() expects to see (OneNAND is
            considered OffsetZero).  Need a custom implementation:
        */

        ioStat = ReadSpareArea(hNTM, ulPage, (void *)buffer.ns);
        if(IOSUCCESS(ioStat, 1))
        {
            /*  If the tag area, including ECC (which covers page data as
                well) is within 1 bit of being erased, then we know that a tag
                was not written. If it is anything else, we know a tag was
                written.  OneNAND with hardware ECC always uses legacy tag size.
            */
            if (!(hNTM->pONI->fIsFlexOneNAND))
            {
                if((!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[ONENAND_SECTOR_TAG_OFFSET], LEGACY_TAG_SIZE)) ||
                   (!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[ONENAND_ECC_OFFSET], ONENAND_ECC_SIZE)))
                    ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);
            }
            else
            {
                if((!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[ONENAND_SECTOR_TAG_OFFSET], LEGACY_TAG_SIZE)) ||
                   (!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[FLEXONENAND_ECC_OFFSET], FLEXONENAND_ECC_SIZE)))
                    ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);
            }

            /* If non-blank ECC's exist in the spare area, we know the page
               and/or spare area have been written. Note that the size of the
               spare area ECC storage field is different between OneNAND and
               FlexOneNAND.
            */
            if (!(hNTM->pONI->fIsFlexOneNAND))
            {
                if(!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[ONENAND_ECC_OFFSET], ONENAND_ECC_SIZE))
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
                }
                else
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
                }
            }
            else
            {
                if(!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[FLEXONENAND_ECC_OFFSET], FLEXONENAND_ECC_SIZE))
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
                }
                else
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
                }
            }
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-GetPageStatus() Page=%lX returning %s\n",
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
    D_UINT32        ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-GetBlockStatus() hNTM=%lX  ulBlock=%lX (%u)\n",
        hNTM, ulBlock, ulBlock));

    DclProfilerEnter(NTMNAME"-GetBlockStatus", 0, 1);

    DclAssert(hNTM);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPagesPerBlock = (hNTM->NtmInfo.ulBlockSize / hNTM->NtmInfo.uPageSize);
    ulBlock += hNTM->ulReservedPages / ulPagesPerBlock;

    ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pONI->pChip, ulBlock);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-GetBlockStatus() Block=%lX returning %s\n",
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
    FFXIOSTATUS     ioStat = DEFAULT_BLOCKIO_STATUS;
    D_UINT32        ulPagesPerBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-SetBlockStatus() hNTM=%lX  ulBlock=%lX (%u)  ulBlockStatus=%lX (%u)\n",
        hNTM, ulBlock, ulBlock, ulBlockStatus, ulBlockStatus));

    DclProfilerEnter(NTMNAME"-SetBlockStatus", 0, 1);

    DclAssert(hNTM);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPagesPerBlock = (hNTM->NtmInfo.ulBlockSize / hNTM->NtmInfo.uPageSize);
    ulBlock += hNTM->ulReservedPages / ulPagesPerBlock;

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pONI->pChip, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
        ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

    DclProfilerLeave(0UL);

    return ioStat;
}


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
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    NTMNAME"-ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        case FFXPARAM_FIM_CHIPID:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hNTM->abID));
            }
            else
            {
                if(ulBuffLen)
                {
                    /*  Fill only as much of the buffer as is supplied by
                        the caller.  If the buffer is larger than needed,
                        the remaining bytes will be untouched.
                    */
                    ulBuffLen = DCLMIN(ulBuffLen, sizeof(hNTM->abID));
                    DclMemCpy(pBuffer, hNTM->abID, ulBuffLen);
                }
                else
                {
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
                }
            }
            break;

        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, (NTMNAME"-ParameterGet() unhandled parameter ID=%x\n", id));
            ffxStat = FFXSTAT_BADPARAMETER;
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    NTMNAME"-ParameterGet() returning status=%lX\n", ffxStat));

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
        NTMNAME"-ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, (NTMNAME":ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        NTMNAME"-ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
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
        ulPage  - The flash offset in pages, relative to any
                  reserved space.
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
    FFXIOSTATUS         ioStat = DEFAULT_UNSUPPORTEDFUNCTION_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(ulCount <= D_UINT16_MAX);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    ioStat = OneNANDNTMRead(hNTM,
                            ulPage,
                            pPages,
                            pSpares,
                            NULL,
                            (D_UINT16)ulCount,
                            0,
                            FALSE);


    /*  PageReadCleanup:
    */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
        ulPage  - The flash offset in pages, relative to any
                  reserved space.
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
    FFXIOSTATUS         ioStat = DEFAULT_UNSUPPORTEDFUNCTION_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(ulCount <= D_UINT16_MAX);

    /*  Adjust the address in the event that there is reserved space
    */
    ulPage += hNTM->ulReservedPages;

    ioStat = OneNANDNTMWrite(hNTM,
                             ulPage,
                             pPages,
                             pSpares,
                             NULL,
                             (D_UINT16)ulCount,
                             0,
                             FALSE);

    /*  RawPageWriteCleanup:
    */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: IORequest

    Parameters:
        hNTM    - The NTM handle to use
        pIOR    - Pointer to the specific IO request

    Return:
        Returns a standard FFXIOSTATUS value.
-------------------------------------------------------------------*/
static FFXIOSTATUS IORequest(
    NTMHANDLE           hNTM,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = {0, FFXSTAT_FIM_UNSUPPORTEDIOREQUEST, 0, {0}};

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
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-IORequest() Unhandled function %x\n", pIOR->ioFunc));

            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-IORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


                    /*-----------------------------*\
                     *                             *
                     *      Helper Functions       *
                     *                             *
                    \*-----------------------------*/



/*-------------------------------------------------------------------
    Local: ReadSpareArea()

    This function reads the spare area for the given flash offset
    into the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to a location in which to put spare
                 area data.

    Return Value:
        Returns a standard FFXIOSTATUS value.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-ReadSpareArea() hNTM=%lX  ulPage=%lX (%u)  pSpare=%lX\n",
        hNTM, ulPage, ulPage, pSpare));

    DclProfilerEnter(NTMNAME"-ReadSpareArea", 0, 1);

    ioStat = OneNANDNTMRead(hNTM,
                            ulPage,
                            NULL,
                            pSpare,
                            NULL,
                            1,
                            0,
                            FALSE);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-ReadSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: WriteSpareArea()

    This function writes the spare area for the given flash
    page from the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages, relative to any
                 reserved space.
        pSpare - A pointer to a location where spare area data
                 is located.

    Return Value:
        Returns a standard FFXIOSTATUS value.

-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-WriteSpareArea() hNTM=%lX  ulPage=%lX (%u)  pSpare=%lX\n",
        hNTM, ulPage, ulPage, pSpare));

    DclProfilerEnter(NTMNAME"-WriteSpareArea", 0, 1);

    ioStat = OneNANDNTMWrite(hNTM, ulPage, NULL, pSpare, NULL, 1, 0, FALSE);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-WriteSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}






        /*------------------------------------------------------*\
        *                                                        *
        *            OneNAND Interface layer begins here         *
        *                                                        *
        \*------------------------------------------------------*/

/*  number of bit positions to shift a core index before writing to the OneNAND
    SA1 register
*/
#define ONENAND_DFS_SHIFT   (15)

/*  number of bit positions to shift a page index before writing to the OneNAND
    SA8 register
*/
#define ONENAND_FPA_SHIFT   (2)

/*  number of bit positions to shift a BufferRAM identifier before writing to
    the OneNAND SB register
*/
#define ONENAND_BSA_SHIFT   (8)

/*  number of bit positions to shift a core index before writing to the OneNAND
    SA2 register
*/
#define ONENAND_DBS_SHIFT   (15)

/*  Bitmask for the DDP bit in the Device Id register
*/
#define ONENAND_DDP_MASK        0x0008


static void OneNANDReadChipId(
    const PONENANDINFO pONI,
    D_BYTE             *pabID);

static FFXSTATUS OneNANDResetChip(      const PONENANDINFO pONI);
static D_BOOL OneNANDConfigureEcc(      const PONENANDINFO pONI, D_BOOL fHWEcc);
static D_BOOL OneNANDConfigureMode(     const PONENANDINFO pONI, D_BOOL fSync);
static FFXSTATUS OneNANDUnlockBlock(    const PONENANDINFO pONI, D_UINT16 uBlock);
static D_UINT16 OneNANDGetFlexPartition(ONENANDINFO *pONI);

static FFXSTATUS OneNANDSetFlexPartition(
    ONENANDINFO *pONI,
    D_UINT16 uPartition);

static void OneNANDLoadStart(
    PONENANDINFO    pONI,
    D_UINT32        ulPage,
    D_UINT16        uBufferIndex,
    D_BOOL          fMain);

static void OneNANDLoadFinalize(
    PONENANDINFO    pONI,
    D_BUFFER        *pPageBuff,
    D_BUFFER        *pSpareBuff,
    D_BUFFER        *pTagBuff,
    D_UINT16        uBufferIndex,
    unsigned        nTagSize);

static void OneNANDProgramInit(
    PONENANDINFO    pONI,
    D_UINT32        ulPage,
    const D_BUFFER  *pPageBuff,
    const D_BUFFER  *pSpareBuff,
    D_UINT16        uBufferIndex);

static void OneNANDProgramStart(
    PONENANDINFO    pONI,
    D_UINT32        ulPage,
    D_UINT16        uBufferIndex,
    D_BOOL          fMain);

static void OneNANDCheckCorrect(
    NTMHANDLE          hNTM,
    D_BUFFER           *pPage,
    D_BUFFER           *pSpare,
    FFXIOSTATUS        *pIOStat,
    D_BOOL             fUseEcc);

static D_BOOL OneNANDNtmTagEncode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc);

static D_BOOL OneNANDNtmTagDecode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc);

static void OneNANDBuildSpareArea(
    NTMHANDLE        hNTM,
    const D_BUFFER   *pData,
    D_BUFFER         *pSpare,
    const D_BUFFER   *pTag,
    unsigned         nTagSize,
    D_BOOL           fUseEcc);

static void  OneNANDGetTag(
    const PONENANDINFO   pONI,
    D_BUFFER             *pTag,
    const D_BUFFER       *pSpare,
    unsigned             nTagSize);

static void  OneNANDSetTag(
    const PONENANDINFO  pONI,
    D_BUFFER            *pSpare,
    const D_BUFFER      *pTag,
    unsigned            nTagSize);

static void OneNANDSetPageAddress(
    const PONENANDINFO  pONI,
    D_UINT16            uCore,
    D_UINT16            uBlock,
    D_UINT16            uPage);

static void OneNANDSetBufferParameters( const PONENANDINFO pONI, D_UINT16 uBuffer);
static void OneNANDSelectDataRAM(       const PONENANDINFO pONI, D_UINT16 uCore);
static void OneNANDStartLoad(           const PONENANDINFO pONI);
static void OneNANDStartProgram(        const PONENANDINFO pONI);
static void OneNANDStartSpareProgram(   const PONENANDINFO pONI);
static void OneNANDStartSpareLoad(      const PONENANDINFO pONI);
static void OneNANDStartErase(          const PONENANDINFO pONI);
static void OneNANDStartUnlock(         const PONENANDINFO pONI);

static FFXIOSTATUS OneNANDFlexNTMRead(
    NTMHANDLE   hNTM,
    D_UINT32    ulStartPage,
    D_BUFFER    *pPageBuff,
    D_BUFFER    *pSpareBuff,
    D_BUFFER    *pTagBuff,
    D_UINT32    ulCount,
    unsigned    nTagSize,
    D_BOOL      fUseEcc);

static FFXIOSTATUS OneNANDFlexNTMWrite(
    NTMHANDLE       hNTM,
    D_UINT32        ulStartPage,
    const D_BUFFER  *pPageBuff,
    const D_BUFFER  *pSpareBuff,
    const D_BUFFER  *pTagBuff,
    D_UINT32        ulCount,
    unsigned        nTagSize,
    D_BOOL          fUseEcc);

static ONENAND_IO_TYPE OneNANDBSCFromCount(
    PONENANDINFO pONI,
    D_UINT16 uCount);

static ONENAND_IO_TYPE OneNANDBSAFromBufferSector(
    D_UINT16  uBuffer,
    D_UINT16  uSector);

static FFXSTATUS OneNANDError(                  const PONENANDINFO pONI);
static FFXSTATUS OneNANDFlexOneNANDError(       const PONENANDINFO pONI);
static FFXSTATUS OneNANDGetChipCommandResult(   const PONENANDINFO pONI);

/*  Function to compute a main buffer offset into the OneNAND main buffer array
*/
static D_UINT16 OneNANDMainBufferOffset(D_UINT16 uBuffer, D_UINT16 uDataBufferSize);

/*  Function to compute a spare region offset into the OneNAND buffer array
*/
static D_UINT16 OneNANDSpareBufferOffset(D_UINT16 uBuffer, D_UINT16 uSpareBufferSize);

/*  Utility function that checks to see if a number is an even power of two
    and returns the exponent:
*/
static D_UINT32 OneNANDPow2(D_UINT32 ulNum);

/*  Utility macros for generating page and block indices:

    REFACTOR: These macros and the quantities they use are dependent on three
    assumptions:

        1. The number of pages in a block is a power of 2.
        2. The number of blocks per die is a power of 2.
        3. The number of dies per device is never greater than 2.

    As of today (3/2/2009), all known OneNAND devices conform to these
    assumptions, but should the day come when we get one that doesn't, we
    will have to re-evaluate how this is done.
*/

#define CORE_FROM_PAGEINDEX(pONI, page)  ((D_UINT16)((page & pONI->ulCoreAddrBitmask) >> pONI->ulCoreAddrShift))
#define BLOCK_FROM_PAGEINDEX(pONI, page) ((D_UINT16)((page & pONI->ulBlockAddrBitmask) >> pONI->ulBlockAddrShift))
#define PAGE_FROM_PAGEINDEX(pONI, page)  ((D_UINT16)(page & pONI->ulPageAddrBitmask))
#define CORE_FROM_BLOCKINDEX(pONI, block)     ((D_UINT16)((block & pONI->ulCoreBlockBitmask) >> pONI->ulCoreBlockShift))
#define BLOCK_FROM_BLOCKINDEX(pONI, block)    ((D_UINT16)(block & pONI->ulBlockBlockBitmask))


/*-------------------------------------------------------------------
    Local: OneNANDCreate()

    This function creates an OneNANDInfo instance.  It identifies
    the part, initializes it, establishes the FlexOneNAND partition
    (if necessary) and configures the device geometry.

    Parameters:
        hDev    - An opaque pointer to a device on which this
                  module is to begin operating.  This module uses
                  this handle to acquire per-device configuration
                  and to specify this particular device to some
                  helper functions provided from other modules.

        hNTM    - Output, on return has it's uPagesPrBlock element
                  initialized.

        pBounds - pointer to boundary address information

    Return Value:
        Returns an freshly-created instance of a ONENANDINFO structure,
        or NULL if it could not create or initialize it.
------------------------------------------------------------------*/
static PONENANDINFO OneNANDCreate(
    FFXDEVHANDLE        hDev,
    NTMHANDLE           hNTM,
    FFXFIMBOUNDS       *pBounds)
{
    ONENANDINFO        *ponenandinfo;
    FFXSTATUS          ffxStat;
    D_UINT16           uCurrentBlock;
    D_UINT32           ulBottomBlocksReserved;
    D_UINT32           ulLastFFXBlock;
    D_UINT16           uPagesPerBlock;
    D_BUFFER            abID[ONENAND_ID_SIZE] = {0}; /* Necessary to placate a picky compiler */

    ponenandinfo = DclMemAllocZero(sizeof *ponenandinfo);
    if(!ponenandinfo)
    {
         FFXPRINTF(1, (NTMNAME"-OneNANDCreate() Unable to create OneNANDInfo structure - DclMemAlloc() failed.\n"));
         return NULL;
    }

    ponenandinfo->onp.nStructLen = sizeof(ponenandinfo->onp);

    ponenandinfo->hHook = FfxHookNTOneNANDCreate(hDev, &ponenandinfo->onp);
    if(!ponenandinfo->hHook)
    {
         FFXPRINTF(1, (NTMNAME"-OneNANDCreate() Unable to create OneNAND control information FfxHookNTOneNANDCreate() failed.\n"));
         return NULL;
    }

    /*  For now there are no backwards compatibility concerns, so the
        structure length we get back <must> be the same as what we
        passed in.
    */
    DclAssert(ponenandinfo->onp.nStructLen == sizeof(ponenandinfo->onp));

    FFXPRINTF(1, ("FlexOneNAND Configuration:\n"));
    FFXPRINTF(1, ("  Timeout             %4u ms\n", ponenandinfo->onp.nTimeoutMS));
    FFXPRINTF(1, ("  DataWidth           %4u\n",    ponenandinfo->onp.nIOWidth));
    FFXPRINTF(1, ("  AlignmentBoundary   %4u\n",    ponenandinfo->onp.nAlignmentBoundary));
    FFXPRINTF(1, ("  UseHardwareECC      %4u\n",    ponenandinfo->onp.fUseHardwareECC));
    FFXPRINTF(1, ("  UseSynchBurstMode   %4u\n",    ponenandinfo->onp.fUseSynchBurstMode));

    /*  if the configuration specifies hardware ECC support,
        use hardware ECC's
    */
    ponenandinfo->fHWEcc = ponenandinfo->onp.fUseHardwareECC;

    /*  Reset and find the chip
    */
    ffxStat = OneNANDResetChip(ponenandinfo);
    if (ffxStat != FFXSTAT_SUCCESS)
    {
         FFXPRINTF(1, (NTMNAME"-OneNANDCreate() Unable to reset chip - OneNANDResetchip() failed.  ffxStat: %lX (%u)\n",
             ffxStat, ffxStat));
         return NULL;
    }

    OneNANDReadChipId(ponenandinfo, hNTM->abID);

    ponenandinfo->pChip = OneNANDFindChip(hNTM->abID);
    if (ponenandinfo->pChip == NULL)
    {
         FFXPRINTF(1, (NTMNAME"-OneNANDCreate() Unable to decode chip ID - OneNANDFindChip() failed.\n"));
         return NULL;
    }

    if(!ponenandinfo->fHWEcc &&
       (FfxNtmHelpGetSpareAreaFormat(ponenandinfo->pChip) != NSF_OFFSETZERO))
    {
        DclPrintf("FFX: The OneNAND NTM only supports OffsetZero style parts when SW ECC mode is enabled.\n");
        return NULL;
    }

    /*  Note whether part is FlexOneNAND:
    */
    if (hNTM->abID[2] == ONENAND_FLEX)
    {
        ponenandinfo->fIsFlexOneNAND = TRUE;
    }
    else
    {
        ponenandinfo->fIsFlexOneNAND = FALSE;
        ponenandinfo->uFlexPartition = 0;
    }


    /*  Read the data buffer size from the chip
    */
    ponenandinfo->uDataBufferSize = FfxHookNTOneNANDReadRegister(ponenandinfo->hHook, ONENAND_REG_DATA_BUF_SIZE);

    /*  Read the number of data and boot buffers from the chip
    */
    {
    ONENAND_IO_TYPE numBufReg;

        numBufReg = FfxHookNTOneNANDReadRegister(ponenandinfo->hHook, ONENAND_REG_AMOUNT_OF_BUF);
        ponenandinfo->uNumBootBuffers = (D_UINT16)(numBufReg & ONENAND_LOWEST_BYTE_MASK);
        ponenandinfo->uNumDataBuffers = (D_UINT16)((numBufReg >> 8) & ONENAND_LOWEST_BYTE_MASK);
    }

    /*  First-generation OneNAND parts had a Data Buffer size the same as the
        page size, but later generation parts, this is not necessarily the case.
        We can at least make sure that the page size is an even multiple
        of the data buffer size, but they can't be assumed to be equal:
    */
    DclAssert(!(ponenandinfo->pChip->pChipClass->uPageSize % ponenandinfo->uDataBufferSize));
    ponenandinfo->uDataBuffersPerPage = ponenandinfo->pChip->pChipClass->uPageSize/ponenandinfo->uDataBufferSize;

    /*  For OneNAND, there are 16 bytes of spare area for each 512 bytes of data
        buffer - use that fact to compute the size of the spare buffer given
        the size of the data buffer. Take into account later-generation OneNAND
        parts whose page size might be a multiple of the data buffer size rather
        than equal to it.
    */
    ponenandinfo->uSpareBufferSize = ponenandinfo->uDataBufferSize / (ONENAND_SECTOR_MAIN_SIZE / ONENAND_SECTOR_SPARE_SIZE);
    ponenandinfo->uSpareBufferSize *= ponenandinfo->uDataBuffersPerPage;
    DclAssert(ponenandinfo->uSpareBufferSize == ponenandinfo->pChip->pChipClass->uSpareSize);

    ponenandinfo->uEdcRequirement     = ponenandinfo->pChip->pChipClass->uEdcRequirement;
    ponenandinfo->uEdcCapability      = ponenandinfo->pChip->pChipClass->uEdcCapability;
    ponenandinfo->uEdcSegmentSize     = ONENAND_SECTOR_MAIN_SIZE;
    ponenandinfo->ulEraseCycleRating  = ponenandinfo->pChip->pChipClass->ulEraseCycleRating;
    ponenandinfo->ulBBMReservedRating = ponenandinfo->pChip->pChipClass->ulBBMReservedRating;

    /*  Get the block size from the info from the table
    */
    ponenandinfo->ulBlockSize = ponenandinfo->pChip->pChipClass->ulBlockSize;

    /*  Remember that Flex devices are parameterized in the NAND ID tables for
        a pure SLC configuration. Block size doubles if managing MLC partition:
    */
    if (ponenandinfo->fIsFlexOneNAND && FLEX1NAND_MANAGE_MLC)
        ponenandinfo->ulBlockSize *= 2;

    /*  Compute the number of OneNAND sectors per page, taking into account that
        there may be more than one data buffer per page
    */
    ponenandinfo->uSectorsPerPage = ponenandinfo->uDataBufferSize / ONENAND_SECTOR_MAIN_SIZE;
    ponenandinfo->uSectorsPerPage *= ponenandinfo->uDataBuffersPerPage;

    /*  Compute the number of blocks
    */
    ponenandinfo->uBlocks = (D_UINT16)(ponenandinfo->pChip->pChipClass->ulChipBlocks);

    /*  Precompute the number of bit positions to shift a page index to get a
        a block index
    */
    DclAssert(ponenandinfo->ulBlockSize /
        (ponenandinfo->uDataBufferSize * ponenandinfo->uDataBuffersPerPage ) <= D_UINT16_MAX);
    DclAssert(ponenandinfo->ulBlockSize % ponenandinfo->uDataBufferSize == 0);
    hNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(ponenandinfo->ulBlockSize /
        (ponenandinfo->uDataBufferSize *
         ponenandinfo->uDataBuffersPerPage));
    FFXPRINTF(1, ("NT1NAND:    ulPagesPerBlock=%lX\n", hNTM->NtmInfo.uPagesPerBlock));

    /*  From right to left, find the first (should be only) bit in the pages
        per block that is not zero

        REFACTOR: This code assumes that the number of pages in a block is
        a power of 2. For current (3/2/2009) OneNAND devices this is true,
        but we should be wary because ONFI does not promise this.
    */
    for(ponenandinfo->ulBlockAddrShift = 0, uPagesPerBlock = hNTM->NtmInfo.uPagesPerBlock;
         (uPagesPerBlock & 1) == 0;
        ponenandinfo->ulBlockAddrShift++)
    {
        uPagesPerBlock >>= 1;
    }
    /*  At this point, we can verify that uPagesPerBlock is still a power of
        two (and therefore shifting is valid).
    */
    DclAssert(uPagesPerBlock == 1);

    /*  Set up the core shifts and bitmasks for Dual Die Package parts, if DDP.

        REFACTOR: The same commentary as above applies to these calculations.
        They depend on both the number of pages per block and the number of
        blocks per die being a power of 2.
    */
    if (abID[ONENAND_ID_DEVICE_INDEX] & ONENAND_DDP_MASK)
    {
        /*  Dual-die package, compute the appropriate shifts and bitmasks.

            The core address bit is the first bit of the chip address for DDP
            packages. When setting up core address shifts and bitmasks,
            remember that linear chip addresses from the NAND ID tables
            are byte addresses that include offset into a page. For OneNAND,
            offsets into a page are handled through a memory-mapped interface
            and are not programmed into the access registers, and should not
            be taken into account when calculating the core address bit shifts
            and masks.

            Note also that it is assumed that the NAND ID tables give the full
            size of a given device, and therefore if it is a DDP package, it
            must be taken into consideration that only half the addressable
            space is in die 0 while the other half is in die 1. That's what
            accounts for the funny subtractions of one to the shifts calculated
            below.
        */
        DclAssert(DCLISPOWEROF2(hNTM->NtmInfo.uPagesPerBlock));
        DclAssert(DCLISPOWEROF2(ponenandinfo->uBlocks));

        ponenandinfo->ulCoreAddrShift = OneNANDPow2(hNTM->NtmInfo.uPagesPerBlock) +
                                         OneNANDPow2(ponenandinfo->uBlocks) - 1;
        ponenandinfo->ulCoreBlockShift = OneNANDPow2(ponenandinfo->uBlocks) - 1;
        ponenandinfo->ulCoreAddrBitmask = 1 << ponenandinfo->ulCoreAddrShift;
        ponenandinfo->ulCoreBlockBitmask = 1 << ponenandinfo->ulCoreBlockShift;

    }
    else
    {
        /*  Single-die package, no shifts or bitmasks
        */
        ponenandinfo->ulCoreAddrShift = 0;
        ponenandinfo->ulCoreBlockShift = 0;
        ponenandinfo->ulCoreAddrBitmask = 0;
        ponenandinfo->ulCoreBlockBitmask = 0;
    }

    /*  Precompute the block address bitmask
    */
    ponenandinfo->ulBlockAddrBitmask = (ponenandinfo->ulBlockSize - 1) << ponenandinfo->ulBlockAddrShift;

    /*  Precompute the page address bitmask - note there's no page address
        shift, as the page address is by definition the lowest bits. Take into
        account that for later-generation OneNAND devices, the page size may be
        a multiple of the data buffer size.
    */
    ponenandinfo->ulPageAddrBitmask = (ponenandinfo->ulBlockSize /
        (ponenandinfo->uDataBufferSize* ponenandinfo->uDataBuffersPerPage)) - 1;

    /*  Precompute the bitmask to get a block address from a block index
    */
    ponenandinfo->ulBlockBlockBitmask = ponenandinfo->ulBlockSize - 1;

    /*  If the part is a Flex part, get the partitioning information, and make
        some decisions about device geometry, including reservation of the
        unused partition. Note that we waited until here to do this instead of
        above where we detect whether a part is a flex part because the
        functions that manipulate the partition depend on the "onenandinfo"
        structure to be filled in.

                                IMPORTANT NOTE:

        The following code sets up a reserved area for a FlexOneNand part
        based on the partition information given in onenandconf.h. It is
        important to note that individual instances of the FlashFX FIMs do
        not handle variable device geometry. Because of that, to manage a
        partitioned FlexOneNand device two instances of the Nand FIM
        configured to use the OneNAND NTM will be required. It is incumbent
        on any customer who wished to do such a thing to modify this NTM to
        account for device information in order to configure the FIM
        appropriately for each device. Perhaps most importantly, if you
        split a FlexOneNAND chip up across two FlashFX devices, access to
        the chip available through the hooks will have to be protected by
        a mutex scheme that is not currently implemented.

        The default behavior for this NTM if a Flex device is used is to
        configure all blocks as MLC except for the first, which is fixed
        by the device to be SLC only.

        The macro FLEX1NAND_MANAGE_MLC in Onenandconf.h tells this NTM which
        partition to manage. The default value is to manage MLC.

        To make this configurable at runtime, the following code will
        need to be altered to take some other value available at runtime
        into acocunt. It is up to the user to decide how to do that.
    */

    if (ponenandinfo->fIsFlexOneNAND)
    {
        D_UINT16 uPartition;

        /*  Configure device geometry based on which partition we are managing,
            according to FLEX1NAND_MANAGE_MLC:
        */
        if (FLEX1NAND_MANAGE_MLC)
        {
            /*  Set the SLC partition as reserved pages. Remember that the first
                block in the device is always SLC, and cannot be placed in an
                MLC partition:
            */
            uPartition = 0;
            hNTM->ulReservedPages = (1 + uPartition) *
                hNTM->NtmInfo.uPagesPerBlock;

            /*  Adjust the number of available blocks so that the loss of those
                in the SLC partition is accounted for:
            */
            ponenandinfo->uBlocks -= 1 + uPartition;

        }
        else
        {
            /*  Set the reserved pages to zero. SLC partition is always at the
                bottom, so no reserved area.
            */
            uPartition = ponenandinfo->uBlocks - 1;
            hNTM->ulReservedPages = 0;

            /*  Adjust the number of blocks in the device to set aside any that
                are in the MLC partition:
            */
            ponenandinfo->uBlocks = uPartition + 1;
        }

        /*  Physically set the partition accordingly:
        */
        ponenandinfo->uFlexPartition = OneNANDGetFlexPartition(ponenandinfo);
        if (ponenandinfo->uFlexPartition != uPartition)
        {
            ffxStat = OneNANDSetFlexPartition(ponenandinfo, uPartition);

            /*  Be sure the partition setting took:
            */
            ponenandinfo->uFlexPartition = OneNANDGetFlexPartition(ponenandinfo);
            if (ponenandinfo->uFlexPartition != uPartition)
            {
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    "NT1NAND-OneNANDCreate() Unable to set flex partition: 0x%04x (%u).\n",
                    uPartition, uPartition
                    ));
                return NULL;
            }
        }
    }

    /*  Determine the start block and end block for unlocking this region
        Note the computed values are 32 bits for bounds checking purposes.
    */
    DclAssert((ponenandinfo->ulBlockSize % 1024UL) == 0);
    ulBottomBlocksReserved = pBounds->ulReservedBottomKB / (ponenandinfo->ulBlockSize / 1024UL);
    ulLastFFXBlock = ponenandinfo->uBlocks - (D_UINT16)(pBounds->ulReservedTopKB / (ponenandinfo->ulBlockSize / 1024UL));

    DclAssert(ulBottomBlocksReserved <= D_UINT16_MAX);
    DclAssert(ulLastFFXBlock <= D_UINT16_MAX);

    for (uCurrentBlock = (D_UINT16)ulBottomBlocksReserved;
         uCurrentBlock <= (D_UINT16)ulLastFFXBlock;
         uCurrentBlock++)
    {
        ffxStat = OneNANDUnlockBlock(ponenandinfo, uCurrentBlock);
        if (ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, (NTMNAME"-OneNANDCreate() Unable to unlock block 0x%04x (%u).\n",
                uCurrentBlock, uCurrentBlock));
            return NULL;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-OneNANDCreate() ONENANDINFO:\n"));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "uDataBufferSize            %X\n",  ponenandinfo->uDataBufferSize));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "uSpareBufferSize           %X\n",  ponenandinfo->uSpareBufferSize));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "uSectorsPerPage            %X\n",  ponenandinfo->uSectorsPerPage));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "uBlocks                    %X\n",  ponenandinfo->uBlocks));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulBlockSize            %lX\n",     ponenandinfo->ulBlockSize));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulCoreAddrBitmask      %lX\n",     ponenandinfo->ulCoreAddrBitmask));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulBlockAddrBitmask     %lX\n",     ponenandinfo->ulBlockAddrBitmask));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulPageAddrBitmask      %lX\n",     ponenandinfo->ulPageAddrBitmask));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulCoreAddrShift        %lX\n",     ponenandinfo->ulCoreAddrShift));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulBlockAddrShift       %lX\n",     ponenandinfo->ulBlockAddrShift));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulCoreBlockBitmask     %lX\n",     ponenandinfo->ulCoreBlockBitmask));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulBlockBlockBitmask    %lX\n",     ponenandinfo->ulBlockBlockBitmask));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "ulCoreBlockShift       %lX\n",     ponenandinfo->ulCoreBlockShift));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "fHWEcc                     %s\n",  ponenandinfo->fHWEcc ? "TRUE" : "FALSE"));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "pChip                  %P\n",      ponenandinfo->pChip));
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "hHook                  %P\n",      ponenandinfo->hHook));

    #if D_DEBUG
        FfxNtmOneNANDDumpRegisters(ponenandinfo->hHook);
    #endif

    return ponenandinfo;
}


/*-------------------------------------------------------------------
    Local: OneNANDDestroy()

    This function destroys a OneNANDInfo instance.

    Parameters:
        pOni - A pointer to a previously-created ONENANDINFO
               structure.

    Return Value:
        Returns a standard FFXSTATUS indictation.
------------------------------------------------------------------*/
static FFXSTATUS OneNANDDestroy(PONENANDINFO pONI)
{
    FFXSTATUS   ffxStat;

    ffxStat = FfxHookNTOneNANDDestroy(pONI->hHook);

    DclAssert(ffxStat == FFXSTAT_SUCCESS);

    DclMemFree(pONI);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: OneNANDNTMRead()

    Read one or more pages of main page data, and optionally read
    the corresponding tag data from the spare area. Uses OneNAND-
    specific NTM operations.

    The fUseEcc flag controls whether ECC-based error detection
    and correction are performed.  Note that the "raw" page read
    functionality (if implemented) is the preferred interface for
    reading without ECC protection.  If fUseEcc is FALSE, this
    function will not read tag data.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, of the first page to
                      read.
        pPageBuff   - A buffer to receive the main page data
        pSpareBuff  - A buffer to receive any spare area data, if desired
        pTagBuff    - A buffer to receive the tag data.  Must be NULL
                      fUseEcc is FALSE.
        ulCount     - Number of pages to read.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if fUseEcc is
                      FALSE.
        fUseEcc     - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pPages array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS OneNANDNTMRead(
    NTMHANDLE   hNTM,
    D_UINT32    ulStartPage,
    D_BUFFER    *pPageBuff,
    D_BUFFER    *pSpareBuff,
    D_BUFFER    *pTagBuff,
    D_UINT32    ulCount,
    unsigned    nTagSize,
    D_BOOL      fUseEcc)
{
    FFXIOSTATUS ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT16    uPhysPagesToLoad;
    D_UINT32    ulPageNumToLoad;
    D_UINT32    ulPageNumToCopy;
    D_UINT16    uBufferIndex;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-OneNANDNTMRead() StartPage=%lX Count=%U pPageBuff=%P pSpareBuff=%P pTagBuff=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPageBuff, pSpareBuff, pTagBuff, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-OneNANDNTMRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    /*  If we're running with hardware ECC support, and the caller requested
        ECC, and it's currently disabled, enable it
    */
    if (hNTM->pONI->fHWEcc)
    {
        OneNANDConfigureEcc(hNTM->pONI, fUseEcc);
    }

    /*  If this is a FlexOneNAND part, call a different routine because
        FlexOneNAND does not support using the two DataRAM buffers to interleave
        loading and reading operations.
    */

    if (hNTM->pONI->fIsFlexOneNAND)
        return  OneNANDFlexNTMRead(hNTM,
                                   ulStartPage,
                                   pPageBuff,
                                   pSpareBuff,
                                   pTagBuff,
                                   ulCount,
                                   nTagSize,
                                   fUseEcc);


    /*  Initially, the number of pages to load is all of the pages this function
        was asked to load
    */
    DclAssert(ulCount <= D_UINT16_MAX);
    uPhysPagesToLoad = (D_UINT16)ulCount;

    ulPageNumToLoad = ulStartPage;

    /*  Take into account the fact that we're often not copying the same page
        we're reading, but we certainly start out that way.
    */
    ulPageNumToCopy = ulPageNumToLoad;

    /*  Issue first load command, to get it loading (takes 25us typical in
        KFG5616x1A-DEB5 OneNAND256 parts)
    */
    uBufferIndex = (D_UINT16)ulPageNumToLoad & 1;
    OneNANDLoadStart(hNTM->pONI, ulPageNumToLoad, uBufferIndex, (D_BOOL)(pPageBuff ? TRUE : FALSE));
    ioStat.ffxStat = FFXSTAT_SUCCESS;

    /*  While there are still pages to load and nothing has failed
    */
    while(uPhysPagesToLoad && (ioStat.ffxStat == FFXSTAT_SUCCESS))
    {

        /*  At this point, there is always a previously started load in progress.
            Wait for it to complete, one way or another.
        */
         ioStat.ffxStat = OneNANDGetChipCommandResult(hNTM->pONI);
         if(ioStat.ffxStat != FFXSTAT_SUCCESS)
         {
             FFXPRINTF(2, (NTMNAME"-OneNANDNTMRead() OneNANDGetChipCommandResult failed: %lX)\n", ioStat.ffxStat));

         }
         else
         {
            /*  At this point, there is always a previously started load
                successfully completed, and a new page to load. Start
                loading the next buffer, if there is one.
            */

            ulPageNumToLoad++;
            uPhysPagesToLoad--;

            /*  Issue first load command, to get it loading, while we read out
                that previously completed load.
            */
            if (uPhysPagesToLoad)
            {
                uBufferIndex = (D_UINT16)ulPageNumToLoad & 1;
                OneNANDLoadStart(hNTM->pONI,
                                 ulPageNumToLoad,
                                 uBufferIndex,
                                 (D_BOOL)(pPageBuff ? TRUE : FALSE));
            }

            /*  At this point, there's always a previously completed load
                waiting in the buffer (and maybe another one in progress).
                Copy out that previously loaded data.

                This loop code implies exactly two buffers, use the last bit of
                the page as the buffer index.
            */
            uBufferIndex = (D_UINT16)ulPageNumToCopy & 1;
            OneNANDLoadFinalize(hNTM->pONI,
                                pPageBuff,
                                pSpareBuff,
                                pTagBuff,
                                uBufferIndex,
                                nTagSize);
            OneNANDCheckCorrect(hNTM, pPageBuff, pSpareBuff, &ioStat, fUseEcc);
            if ((ioStat.ffxStat != FFXSTAT_SUCCESS) && (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA))
            {
                 FFXPRINTF(1, (NTMNAME"-OneNANDNTMRead() OneNANDCheckCorrect failed: %lX (%u)\n",
                    ioStat.ffxStat, ioStat.ffxStat));
                /*  This page was *not* loaded or had uncorrectable errors:
                */
                uPhysPagesToLoad++;
                break;
            }

            /*  At this point, we have either:
                 - an error
                 - or a good buffer of data copied out, and a read in progress
                   (just like we did when we entered the loop the first time).
                   Move on to the next buffer for the next copy, and let the
                   loop do the next one.  By the time we get back to the top,
                   a portion (and perhaps all) of the 25us will have elapsed
                   and we'll spend little time (if any) busy waiting.
            */
            if (pPageBuff)
            {
                pPageBuff += hNTM->NtmInfo.uPageSize;
            }
            if (pSpareBuff)
            {
                pSpareBuff += hNTM->NtmInfo.uSpareSize;
            }
            if (pTagBuff)
            {
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
                pTagBuff += nTagSize;
            }
            ulPageNumToCopy++;

         }
    }

    /*  PageReadCleanup:
    */

    /*  Return the count of pages read
    */
    ioStat.ulCount = ulCount - uPhysPagesToLoad;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-OneNANDNTMRead() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: OneNANDNTMWrite()

    NTM-specific procedure to Write one or more logical pages of main array
    data, along with the tag data if it is provided.

    If fUseEcc is TRUE, an ECC for the main page data is generated
    and written in the spare area, along with the specified tag.  If
    fUseEcc is FALSE, no ECC and no tag is written.  The spare area
    includes a record of the state of fUseEcc.  Note that the "raw"
    page write functionality (if implemented) is the preferred
    interface for writing data without ECC.

    Parameters:
        hNTM        - The NTM handle to use.
        ulStartPage - The starting page to write, relative to any
                      reserved space.
        pPageBuff   - A pointer to the main page data to write.
        PSpareBuff  - A pointer to spare aread data to write. May be
                      NULL.
        pTagBuff    - A buffer containing the tag data.  Must be NULL
                      if nTagSize is 0 and fUseEcc is FALSE.
        ulCount     - Number of pages to write.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if pTags is
                      NULL and fUseEcc is FALSE.
        fUseEcc     - If TRUE, write the data with ECC.  This flag
                      implies that a tag is being written as well.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the operation
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS OneNANDNTMWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPageBuff,
    const D_BUFFER     *pSpareBuff,
    const D_BUFFER     *pTagBuff,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS       ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT16          uPhysPagesToProgram;
    D_UINT32          ulPageNumToProgram;
    D_UINT32          ulNumSuccessfulPrograms=0;
    D_UINT32          ulPageNumToCopy;
    D_UINT16          uBufferIndex;
    D_BUFFER         *pSpareToWrite;
    D_BUFFER         *pTagToWrite;
    DCLALIGNEDBUFFER(spare, data, FFX_NAND_MAXSPARESIZE);
    DCLALIGNEDBUFFER(tag, data, FFX_NAND_TAGSIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-OneNANDNTMWrite() hNTM=%lX StartPage=%lX pPageBuff=%P pSpareBuff=%P pTagBuff=%P Count=%lU TagSize=%u fUseEcc=%U\n",
        hNTM, ulStartPage, pPageBuff, pSpareBuff, pTagBuff, ulCount, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-OneNANDNTMWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTagBuff && nTagSize) || (!pTagBuff && !nTagSize));
    DclAssert((fUseEcc) || (!pTagBuff && !nTagSize));

    /*  If we're running with hardware ECC support, and the caller requested
        ECC, and it's currently disabled, enable it
    */
    if (hNTM->pONI->fHWEcc)
    {
        OneNANDConfigureEcc(hNTM->pONI, fUseEcc);
    }

    /*  If this is a FlexOneNAND part, call a different routine because
        FlexOneNAND does not support using the two DataRAM buffers to interleave
        loading and reading operations.
    */
    if (hNTM->pONI->fIsFlexOneNAND)
        return  OneNANDFlexNTMWrite(hNTM,
                                    ulStartPage,
                                    pPageBuff,
                                    pSpareBuff,
                                    pTagBuff,
                                    ulCount,
                                    nTagSize,
                                    fUseEcc);

    /*  If the caller supplies a spare area, we use it.  If the caller did not
        supply a spare area, we must generate one.
    */
    if (!pSpareBuff)
    {
        pSpareToWrite = spare.data;
    }
    else
    {
        pSpareToWrite = (D_BUFFER *)pSpareBuff;
    }

    /*  If the caller supplies a tag buffer, we use it.  If the caller did not
        supply a tag buffer, we must generate one.
    */
    if (!pTagBuff)
    {
        DclAssert(nTagSize == 0);
        DclAssert(FFX_NAND_TAGSIZE == hNTM->NtmInfo.uMetaSize);

        pTagToWrite = tag.data;
        DclMemSet(pTagToWrite, ERASED8, FFX_NAND_TAGSIZE);
    }
    else
    {
        DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

        pTagToWrite = (D_BUFFER *)pTagBuff;
    }


    DclAssert(ulCount <= D_UINT16_MAX);
    uPhysPagesToProgram = (D_UINT16)ulCount;
    ulPageNumToProgram = ulStartPage;

    /*  Take into account the fact that we're often not copying the same page
        we're writing, but we certainly start out that way.
    */
    ulPageNumToCopy = ulPageNumToProgram;

    /*  Initialize spare area, if the caller didn't supply it.
    */
    if (!pSpareBuff)
    {
        if (nTagSize)
        {
            OneNANDBuildSpareArea(hNTM,
                                  pPageBuff,
                                  pSpareToWrite,
                                  pTagToWrite,
                                  nTagSize,
                                  fUseEcc);
        }
        else
        {
            OneNANDBuildSpareArea(hNTM,
                                  pPageBuff,
                                  pSpareToWrite,
                                  NULL,
                                  0,
                                  fUseEcc);
        }

    }

    /*  Copy the first data to the flash part's internal buffer,
        using the last bit of the page number as the buffer index.
    */
    uBufferIndex = (D_UINT16)ulPageNumToCopy & 1;
    OneNANDProgramInit(hNTM->pONI,
                       ulPageNumToCopy,
                       pPageBuff,
                       pSpareToWrite,
                       uBufferIndex);

    /*  Account for the data that we just copied
    */
    ulPageNumToCopy++;
    if (pSpareBuff)
    {
        pSpareToWrite += hNTM->NtmInfo.uSpareSize;
    }
    if (pTagBuff)
    {
        pTagToWrite += nTagSize;
    }
    if (pPageBuff)
    {
        pPageBuff += hNTM->NtmInfo.uPageSize;
    }

    /*  The data's there, start writing it
    */
    uBufferIndex = (D_UINT16)ulPageNumToProgram & 1;
    OneNANDProgramStart(hNTM->pONI,
                        ulPageNumToProgram,
                        uBufferIndex,
                        (D_BOOL)(pPageBuff ? TRUE : FALSE));

    /*  Account for the program operation we just started
    */
    ulPageNumToProgram++;
    uPhysPagesToProgram--;

    while(uPhysPagesToProgram && (ioStat.ffxStat == FFXSTAT_SUCCESS))
    {


        /*  At this point, there is always a previously started program
            operation in progress. While it's programming, copy the next page
            into the OneNAND chip's buffer.
        */

        /*  Initialize spare area, if the caller didn't supply it.
        */
        if (!pSpareBuff)
        {
            if (nTagSize)
            {
                OneNANDBuildSpareArea(hNTM,
                                      pPageBuff,
                                      pSpareToWrite,
                                      pTagToWrite,
                                      nTagSize,
                                      fUseEcc);
            }
            else
            {
                OneNANDBuildSpareArea(hNTM,
                                      pPageBuff,
                                      pSpareToWrite,
                                      NULL,
                                      0,
                                      fUseEcc);
            }
        }

        /*  Write the actual data to the flash part's internal buffer
        */
        uBufferIndex = (D_UINT16)ulPageNumToCopy & 1;
        OneNANDProgramInit(hNTM->pONI,
                           ulPageNumToCopy,
                           pPageBuff,
                           pSpareToWrite,
                           uBufferIndex);


        ulPageNumToCopy++;
        if (pSpareBuff)
        {
            pSpareToWrite += hNTM->NtmInfo.uSpareSize;
        }
        if (pPageBuff)
        {
            pPageBuff += hNTM->NtmInfo.uPageSize;
        }
        if (pTagBuff)
        {
            pTagToWrite += nTagSize;
        }

        /*  At this point, there is always a previously started program in
            progress. (and data waiting to be written) Wait for it to complete,
            one way or another.
        */
         ioStat.ffxStat = OneNANDGetChipCommandResult(hNTM->pONI);
         if(ioStat.ffxStat != FFXSTAT_SUCCESS)
         {
             FFXPRINTF(1, (NTMNAME"-OneNANDNTMWrite() OneNANDGetChipCommandResult failed: %lX (%u)\n",
                 ioStat.ffxStat, ioStat.ffxStat));
         }
         else
         {

            /*  At this point, there is always a previously started program
                successfully completed, and a new page to waiting to program.
                Program it, then return to top of the loop to start the next copy
            */
            ulNumSuccessfulPrograms++;
            uBufferIndex = (D_UINT16)ulPageNumToProgram & 1;
            OneNANDProgramStart(hNTM->pONI,
                                ulPageNumToProgram,
                                uBufferIndex,
                                (D_BOOL)(pPageBuff ? TRUE : FALSE));

            ulPageNumToProgram++;
            uPhysPagesToProgram--;
         }
    }

    /*  If we get to this point, we've either copied all of the data and started
        programming it, or there was an error.
    */
    if (ioStat.ffxStat == FFXSTAT_SUCCESS)
    {

        /*  if this is success, there should be no remaining pages to write
        */
        DclAssert(uPhysPagesToProgram == 0);

        ioStat.ffxStat = OneNANDGetChipCommandResult(hNTM->pONI);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, (NTMNAME"-OneNANDNTMWrite() OneNANDGetChipCommandResult failed: %lX (%u)\n",
                ioStat.ffxStat, ioStat.ffxStat));
        }
        else
            ulNumSuccessfulPrograms++;
    }

    /*  Return the count of pages written
    */
    ioStat.ulCount = ulNumSuccessfulPrograms;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-OneNANDNTMWrite() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: OneNANDFlexNTMRead()

    An implementation of OneNANDNTMRead that is specific to the
    FlexOneNAND Device. Takes the same parameters and returns
    the same results.

    The fUseEcc flag controls whether ECC-based error detection
    and correction are performed.  Note that the "raw" page read
    functionality (if implemented) is the preferred interface for
    reading without ECC protection.  If fUseEcc is FALSE, this
    function will not read tag data.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, of the first page to
                      read.
        pPageBuff   - A buffer to receive the main page data
        pSpareBuff  - A buffer to receive any spare area data, if desired
        pTagBuff    - A buffer to receive the tag data.  Must be NULL
                      fUseEcc is FALSE.
        ulCount     - Number of pages to read.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if fUseEcc is
                      FALSE.
        fUseEcc     - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely read
        successfully.  Portions of the pPages array beyond those
        corresponding to the ulCount may have been modified.  The
        status indicates whether the read was entirely successful.
-------------------------------------------------------------------*/

static FFXIOSTATUS OneNANDFlexNTMRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_BUFFER           *pPageBuff,
    D_BUFFER           *pSpareBuff,
    D_BUFFER           *pTagBuff,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT16    uPhysPagesToLoad;
    D_UINT32    ulPageNumToLoad;

    DclProfilerEnter("OneNANDFlexNTMRead", 0, ulCount);

    /*  Initially, the number of pages to load is all of the pages this function
        was asked to load
    */
    DclAssert(ulCount <= D_UINT16_MAX);
    uPhysPagesToLoad = (D_UINT16)ulCount;
    ulPageNumToLoad = ulStartPage;

    /*  Main loop, executed while there are still pages to load and nothing has
        failed. Note that we load and then read pages one by one. FlexOneNAND
        does not support interleaving these operations between two DataRAMS like
        the original OneNAND parts do. FlexOneNand does have features (super
        loads and cached program operations) that perfrom a similar function,
        but they are currently only available in the 1.8V parts and not the 3V
        parts. Therefore, these features are not currently being exercised. If
        it is desired to use these features, this procedure is the one that
        should be modified to do it.
    */
    while(uPhysPagesToLoad && (ioStat.ffxStat == FFXSTAT_SUCCESS))
    {

        /*  Issue  load command, and wait for it to complete. Remember that with
            FlexOneNAND we can't interleave the DataRAM buffers.
        */
        OneNANDLoadStart(hNTM->pONI, ulPageNumToLoad, 0, (D_BOOL)(pPageBuff ? TRUE : FALSE));
        ioStat.ffxStat = OneNANDGetChipCommandResult(hNTM->pONI);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                "NT1NAND-OneNANDNTMRead() OneNANDGetChipCommandResult failed: %lX (%u)\n",
                ioStat.ffxStat, ioStat.ffxStat));

            /*  The existence of FIMIOERROR here is an indication that in
                FlexOneNAND, the error bit was set but no evidence was
                found that an uncorrectable error occured. The only way
                that can happen is if the OneNAND controller was reset during
                a load operation, which will only happen if the hardware
                platform does something unexpected (such as go into sleep
                mode while a load is ongoing).
            */
            if (ioStat.ffxStat == FFXSTAT_FIMIOERROR)
                ioStat.ffxStat = FFXSTAT_FIM_UNEXPECTEDHWSTATE;
        }
        else
        {
            /*  At this point, there is always a previously started load
                successfully completed. Copy out that previously loaded data.
            */
            OneNANDLoadFinalize(hNTM->pONI,
                                pPageBuff,
                                pSpareBuff,
                                pTagBuff,
                                0,
                                nTagSize);
            OneNANDCheckCorrect(hNTM, pPageBuff, pSpareBuff, &ioStat, fUseEcc);
            if ((ioStat.ffxStat != FFXSTAT_SUCCESS) && (ioStat.ffxStat != FFXSTAT_FIMCORRECTABLEDATA))
            {
                 FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                     "NT1NAND-OneNANDNTMRead() OneNANDCheckCorrect failed: %lX (%u)\n",
                     ioStat.ffxStat, ioStat.ffxStat));
                 break;
            }

            /*  At this point, we have either:
                 - an error
                 - or a good buffer of data copied out. Move on to the next
                   buffer for the next copy, and let the loop do the next one.
            */
            if (pPageBuff)
            {
                pPageBuff += hNTM->NtmInfo.uPageSize;
            }
            if (pSpareBuff)
            {
                pSpareBuff += hNTM->NtmInfo.uSpareSize;
            }
            if (pTagBuff)
            {
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
                pTagBuff += nTagSize;
            }
            ulPageNumToLoad++;
            uPhysPagesToLoad--;
        }
    }

    /*  PageReadCleanup:
    */

    /*  Return the count of pages read
    */
    ioStat.ulCount = ulCount - uPhysPagesToLoad;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NT1NAND-OneNANDNTMRead() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

    return ioStat;
}


/*-------------------------------------------------------------------
    Private: OneNANDFlexNTMWrite()

    A version of OneNANDNTMWrite that is specific for FlexOneNAND
    devices. Takes the same parameters and returns the same result.

    If fUseEcc is TRUE, an ECC for the main page data is generated
    and written in the spare area, along with the specified tag.  If
    fUseEcc is FALSE, no ECC and no tag is written.  The spare area
    includes a record of the state of fUseEcc.  Note that the "raw"
    page write functionality (if implemented) is the preferred
    interface for writing data without ECC.

    Parameters:
        hNTM        - The NTM handle to use.
        ulStartPage - The starting page to write, relative to any
                      reserved space.
        pPageBuff   - A pointer to the main page data to write.
        PSpareBuff  - A pointer to spare aread data to write. May be
                      NULL.
        pTagBuff    - A buffer containing the tag data.  Must be NULL
                      if nTagSize is 0 and fUseEcc is FALSE.
        ulCount     - Number of pages to write.  The range of pages
                      must not cross an erase block boundary.
        nTagSize    - The tag size to use.  Must be 0 if pTags is
                      NULL and fUseEcc is FALSE.
        fUseEcc     - If TRUE, write the data with ECC.  This flag
                      implies that a tag is being written as well.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the operation
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS OneNANDFlexNTMWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPageBuff,
    const D_BUFFER     *pSpareBuff,
    const D_BUFFER     *pTagBuff,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS       ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT16          uPhysPagesToProgram;
    D_UINT32          ulPageNumToProgram;
    D_BUFFER         *pSpareToWrite;
    D_BUFFER         *pTagToWrite;
    DCLALIGNEDBUFFER(spare, data, FFX_NAND_MAXSPARESIZE);
    DCLALIGNEDBUFFER(tag, data, FFX_NAND_TAGSIZE);

    DclProfilerEnter("OneNANDFlexNTMWrite", 0, ulCount);

    if (!pSpareBuff)
    {
        pSpareToWrite = spare.data;
    }
    else
    {
        pSpareToWrite = (D_BUFFER *)pSpareBuff;
    }

    /*  If the caller supplies a tag buffer, we use it.  If the caller did not
        supply a tag buffer, we must generate one.
    */
    if (!pTagBuff)
    {
        DclAssert(nTagSize == 0);
        DclAssert(FFX_NAND_TAGSIZE == hNTM->NtmInfo.uMetaSize);

        pTagToWrite = tag.data;
        DclMemSet(pTagToWrite, ERASED8, FFX_NAND_TAGSIZE);
    }
    else
    {
        DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

        pTagToWrite = (D_BUFFER *)pTagBuff;
    }


    DclAssert(ulCount <= D_UINT16_MAX);
    uPhysPagesToProgram = (D_UINT16)ulCount;
    ulPageNumToProgram = ulStartPage;

    /*  Primary loop. executed while there are still pages to program and
        nothing has failed. Note that we write and then program  pages one by
        one. FlexOneNAND does not support interleaving these operations between
        two DataRAMS like the original OneNAND parts do. FlexOneNand does have
        features (super loads and cached program operations) that perfrom a
        similar function, but they are currently only available in the 1.8V
        parts and not the 3V parts. Therefore, these features are not currently
        being exercised. If it is desired to use these features, this
        procedure is the one that should be modified to do it.
    */

    while(uPhysPagesToProgram && (ioStat.ffxStat == FFXSTAT_SUCCESS))
    {

        /*  Initialize spare area, if the caller didn't supply it.
        */
        if (!pSpareBuff)
        {
            if (nTagSize)
            {
                OneNANDBuildSpareArea(hNTM,
                                      pPageBuff,
                                      pSpareToWrite,
                                      pTagToWrite,
                                      nTagSize,
                                      fUseEcc);
            }
            else
            {
                OneNANDBuildSpareArea(hNTM,
                                      pPageBuff,
                                      pSpareToWrite,
                                      NULL,
                                      0,
                                      fUseEcc);
            }
        }

        /*  Write the actual data to the flash part's internal buffer
        */
            OneNANDProgramInit(hNTM->pONI,
                               ulPageNumToProgram,
                               pPageBuff,
                               pSpareToWrite,
                               0);


        if (pSpareBuff)
        {
            pSpareToWrite += hNTM->NtmInfo.uSpareSize;
        }
        if (pPageBuff)
        {
            pPageBuff += hNTM->NtmInfo.uPageSize;
        }
        if (pTagBuff)
        {
            pTagToWrite += nTagSize;
        }

        /*  At this point, The data has been written to the device. Program it,
            wait for it to complete, and then return to top of the loop to start
            the next copy
         */
        OneNANDProgramStart(hNTM->pONI,
                            ulPageNumToProgram,
                            0,
                            (D_BOOL)(pPageBuff ? TRUE : FALSE));
        ioStat.ffxStat = OneNANDGetChipCommandResult(hNTM->pONI);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
         FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
             "NT1NAND-OneNANDNTMWrite() OneNANDGetChipCommandResult failed: %lX (%u)\n",
             ioStat.ffxStat, ioStat.ffxStat));

        }
        else
        {
            ulPageNumToProgram++;
            uPhysPagesToProgram--;
        }
    }

    /*  Return the count of pages written
    */
    ioStat.ulCount = ulCount - uPhysPagesToProgram;

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NT1NAND-OneNANDNTMWrite() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

    return ioStat;
}

/*-------------------------------------------------------------------
    Private: OneNANDLoadStart()

    Does the preliminary work of starting an operation to load flash
    data into one of the RAM buffers contained on a OneNAND Chip, up
    to and including:

     - Establishing the page and block indexes to access flash data
     - Selecting which of the RAM buffers to use (FlexOneNAND uses
       both)
     - Issuing the command to commence the load operation.

    Note that neither the Start procedure or the Finalize procedure
    check on the progress of a previously started load. External logic
    must be used to perform that function.

    Parameters:
        pONI         - Pointer to the OneNANDInfo structure (contains
                       info about device geometry).
        ulPage       - Offset to the logical page we want to load.
        uBufferIndex - index to which internal RAM buffer to use.
        fMain        - Flag indicating whether we wish to load both
                       main page data as well as spare area.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDLoadStart(
    PONENANDINFO pONI,
    D_UINT32 ulPage,
    D_UINT16 uBufferIndex,
    D_BOOL fMain)
{

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDLoadStart() pONI: %P  ulPage: %lX (%u) uBufferIndex: 0x%04x (%u)\n",
        pONI, ulPage, ulPage, uBufferIndex, uBufferIndex));

    DclProfilerEnter(NTMNAME"-OneNANDLoadStart", 0, 0);

    /*  Set up the address for the page
    */
    OneNANDSetPageAddress(pONI,
                          CORE_FROM_PAGEINDEX(pONI, ulPage),
                          BLOCK_FROM_PAGEINDEX(pONI, ulPage),
                          PAGE_FROM_PAGEINDEX(pONI, ulPage));

    /* Set up the buffer to use
    */
    OneNANDSetBufferParameters(pONI,
                               uBufferIndex);

    /*  Select the DataRAM to use
    */
    OneNANDSelectDataRAM(pONI, CORE_FROM_PAGEINDEX(pONI, ulPage));

    /*  Start the load operation. Note that FlexOneNAND does not support the
        "spare-only" commands.
    */

    if (fMain || pONI->fIsFlexOneNAND)
    {
        /* loads both main and spare data
        */
        OneNANDStartLoad(pONI);
    }
    else
    {
        /* loads spare data only
        */
        OneNANDStartSpareLoad(pONI);
    }

    DclProfilerLeave(0);

}


/*-------------------------------------------------------------------
    Local: OneNANDLoadFinalize()

    Reads the main page and spare data from a device that has had
    the LoadStart function run on it.

    Note that this function does not check to be sure that the
    LoadStart function that was used to start loading flash data
    into one of the internal RAM buffers is complete. Since the logic
    of determining that is tied up with which RAM buffer is in use,
    logic to examine the state of a load start must be implemented
    external to either this function or the load start function


    Parameters:
        pONI         - Pointer to the OneNANDInfo structure (contains
                       info about device geometry).
        pPageBuff    - Pointer to buffer we can put main page data into
        pSpareBuff   - Pointer to buffer we can put spare area data into
        pTagBuff     - Pointer to buffer we can write tags into
        uBufferIndex - index to which internal RAM buffer to use.
        nTagSize     - size of the tag data.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDLoadFinalize(
    PONENANDINFO        pONI,
    D_BUFFER           *pPageBuff,
    D_BUFFER           *pSpareBuff,
    D_BUFFER           *pTagBuff,
    D_UINT16            uBufferIndex,
    unsigned            nTagSize)
{
    D_UINT16 uSpareOffset;
    D_UINT16 uMainOffset;
    D_BUFFER *pSpareToRead;
    DCLALIGNEDBUFFER(spare, data, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDLoadFinalize() pONI: %P  pPageBuff: %lX  pSpareBuff: %lX  pTagBuff: %lX  uBufferIndex: %U\n",
        pONI,
        pPageBuff,
        pSpareBuff,
        pTagBuff,
        uBufferIndex));

    DclProfilerEnter(NTMNAME"-OneNANDLoadFinalize", 0, 0);

    if (pSpareBuff)
    {
        pSpareToRead = pSpareBuff;
    }
    else
    {
        pSpareToRead = spare.data;
    }

    if (pPageBuff)
    {

        /* Compute the offset into the buffer
        */
        uMainOffset = OneNANDMainBufferOffset(uBufferIndex,
                                              pONI->uDataBufferSize);


        FfxHookNTOneNANDReadBuffer(pONI->hHook,
                                   pPageBuff,
                                   uMainOffset,
                                   (D_UINT16) (pONI->uDataBufferSize * pONI->uDataBuffersPerPage) );
    }



    /*  where in this page is the appropriate
        spare area?
    */
    uSpareOffset = OneNANDSpareBufferOffset(uBufferIndex,
                                            pONI->uSpareBufferSize);


    /*  get the spare region data
    */
    FfxHookNTOneNANDReadBuffer(pONI->hHook, pSpareToRead, uSpareOffset, pONI->uSpareBufferSize);

    if (pTagBuff)
    {
        DclAssert(nTagSize);
        OneNANDGetTag(pONI, pTagBuff, pSpareToRead, nTagSize);
    }

    DclProfilerLeave(0);

}


/*-------------------------------------------------------------------
    Local: OneNANDProgramInit()

    Performs the preliminaries of a Program operation to the flash
    memory array. Those operations are as follows:

     - Choose which DataRAM buffer to write data into
     - Compute the buffer offset
     - Copy main page and spare data into the buffer

    Parameters:
        pONI         - Pointer to the OneNANDInfo structure (contains
                       info about device geometry).
        ulPage       - The logical page to which data will be written
        pPageBuff    - Pointer to main page data buffer
        pSpareBuff   - Pointer to spare area data buffer
        uBufferIndex - index to which internal RAM buffer to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDProgramInit(
    PONENANDINFO    pONI,
    D_UINT32        ulPage,
    const D_BUFFER *pPageBuff,
    const D_BUFFER *pSpareBuff,
    D_UINT16        uBufferIndex)
{

    D_UINT16 uMainOffset;
    D_UINT16 uSpareOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-OneNANDProgramInit() ulPage=%lX pPageBuff=%P pSpareBuff=%P uBufferIndex=%U\n",
        ulPage, pPageBuff, pSpareBuff, uBufferIndex));

    DclProfilerEnter(NTMNAME"-OneNANDProgramInit", 0, 0);

    /*  Select which DataRAM buffer in the package to use
    */
    OneNANDSelectDataRAM(pONI, CORE_FROM_PAGEINDEX(pONI, ulPage));

    /*  Compute the offset into the array of buffers
    */
    uMainOffset = OneNANDMainBufferOffset(uBufferIndex,
                                          pONI->uDataBufferSize);
    /*  pPageBuff is optional, but pSpareBuff is always required
    */
    if (pPageBuff)
    {
        /*  Copy a buffer of the caller's data into DataRAM Main area
        */
        FfxHookNTOneNANDWriteBuffer(pONI->hHook,
                                    pPageBuff,
                                    uMainOffset,
                                    (D_UINT16) (pONI->uDataBufferSize * pONI->uDataBuffersPerPage) );
    }
    else if (pONI->fIsFlexOneNAND)
    {
        /*  Tell NTM hook to write blank data- only necessary for flex parts
        */
        FfxHookNTOneNANDWriteBuffer(pONI->hHook,
                                    NULL,
                                    uMainOffset,
                                    (D_UINT16) (pONI->uDataBufferSize * pONI->uDataBuffersPerPage) );
    }

    /*  Copy ECC Data to the DataRAM Spare area
    */
    uSpareOffset = OneNANDSpareBufferOffset(uBufferIndex,
                                            pONI->uSpareBufferSize);

    FfxHookNTOneNANDWriteBuffer(pONI->hHook, pSpareBuff, uSpareOffset, pONI->uSpareBufferSize);

    DclProfilerLeave(0);

}


/*-------------------------------------------------------------------
    Local: OneNANDProgramStart()

    Performs the preliminaries of a Program operation to the flash
    memory array. Those operations are as follows:

     - Set up the page and block addressing registers
     - Choose which DataRAM buffer from which to program the flash
       array
     - Issue the start program command.

    Parameters:
        pONI         - Pointer to the OneNANDInfo structure (contains
                       info about device geometry).
        ulPage       - The logical page to which data will be written
        uBufferIndex - index to which internal RAM buffer to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDProgramStart(
    PONENANDINFO pONI,
    D_UINT32 ulPage,
    D_UINT16 uBufferIndex,
    D_BOOL fMain)
{

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-OneNANDProgramStart()\n"));

    DclProfilerEnter(NTMNAME"-OneNANDProgramStart", 0, 0);

    /*  Set up the address for the page
    */
    OneNANDSetPageAddress(pONI,
                          CORE_FROM_PAGEINDEX(pONI, ulPage),
                          BLOCK_FROM_PAGEINDEX(pONI, ulPage),
                          PAGE_FROM_PAGEINDEX(pONI, ulPage));

    /* Set up the buffer to use
    */
    OneNANDSetBufferParameters(pONI,
                               uBufferIndex);

    /*  Start the program operation. Note that FlexOneNAND does not support
        "spare-only" operations.
    */
    if (fMain || pONI->fIsFlexOneNAND)
    {
        OneNANDStartProgram(pONI);
    }
    else
    {
        OneNANDStartSpareProgram(pONI);
    }

    DclProfilerLeave(0);

}


/*-------------------------------------------------------------------
    Local: OneNANDEraseBlocks()

    Erases a range of blocks:

     - Tell the device which block to erase
     - Issue the erase command
     - Wait for the operation to finish and check the status.
     - Loop back if more blocks to erase

    Parameters:
        pONI         - Pointer to the OneNANDInfo structure (contains
                       info about device geometry).
        usStartBlock - The first of the blocks to erase
        uBlocks      - The number of blocks to erase.

    Return Value:
        Standard FFXIOSTATUS indication
-------------------------------------------------------------------*/
static FFXIOSTATUS OneNANDEraseBlocks(
    PONENANDINFO pONI,
    D_UINT32 ulStartBlock,
    D_UINT32 uBlocks)
{
    FFXIOSTATUS        ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32           uBlock;
    ONENAND_IO_TYPE    StartAddress1Val;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-OneNANDEraseBlocks() ulStartBlock=%lX  uBlocks=%lX\n",
        ulStartBlock, uBlocks));

    DclProfilerEnter(NTMNAME"-OneNANDEraseBlocks", 0, 0);

    uBlock = ulStartBlock;
    while (uBlocks)
    {
        /* set up the block number to erase
        */
        StartAddress1Val = (CORE_FROM_BLOCKINDEX(pONI, uBlock) << ONENAND_DFS_SHIFT)
                        |  BLOCK_FROM_BLOCKINDEX(pONI, uBlock);

        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_1, StartAddress1Val);

        /*  Tell the flash to erase those blocks now
        */
        OneNANDStartErase(pONI);

        /*  Wait for completion of the program operation and return the result
        */
        ioStat.ffxStat = OneNANDGetChipCommandResult(pONI);

        uBlocks--;
        uBlock++;
    }

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-OneNANDEraseBlocks() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: OneNANDSetPageAddress()

    Universal routine to output a page address for a OneNAND chip.
    The OneNAND and FlexOneNAND have a set of registers to contain
    the page address of data to be programmed or loaded. This
    procedure sets those registers.

    Parameters:
        pONI   - Pointer to the OneNANDInfo structure (contains
                 info about device geometry).
        uCore  - Which of the internal chips to use if the device
                 is a DDP (dual core) device
        uBlock - Which block we are interested in accessing
        uPage  - Which page in the block of interest we want to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDSetPageAddress(
    const PONENANDINFO  pONI,
    D_UINT16            uCore,
    D_UINT16            uBlock,
    D_UINT16            uPage)
{

    ONENAND_IO_TYPE StartAddress1Val;
    ONENAND_IO_TYPE StartAddress8Val;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-OneNANDSetPageAddress() pONI=%lX  uCore=%lX (%u)  uBlock=%lX (%u)  uPage=%lX (%u)\n",
        pONI,
        uCore, uCore,
        uBlock, uBlock,
        uPage, uPage));

    StartAddress1Val = (uCore << ONENAND_DFS_SHIFT)
                        |  uBlock;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDSetPageAddress() StartAddress1Val=%lX (%u)\n",
        StartAddress1Val, StartAddress1Val));

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_1, StartAddress1Val);

    StartAddress8Val = (uPage << ONENAND_FPA_SHIFT);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDSetPageAddress() StartAddress8Val=%lX (%u)\n",
        StartAddress8Val, StartAddress8Val));

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_8, StartAddress8Val);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-OneNANDSetPageAddress() returning.\n"));

}


/*-------------------------------------------------------------------
    Local: OneNANDSetBufferParameters()

    Universal routine to output a buffer sector start and count
    for a OneNAND chip. OneNAND devices divide DataRAMs into sectors,
    and some of the error reporting mechanisms for FlexOneNAND as
    well as page addressing mechanisms for OneNAND are established
    in terms of sectors.

    Parameters:
        pONI     - Pointer to the OneNANDInfo structure (contains
                   info about device geometry).
        ulBuffer - The DataRAM buffer offset

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDSetBufferParameters(
    const PONENANDINFO pONI,
    D_UINT16           uBuffer)
{
    ONENAND_IO_TYPE StartBufferVal;
    ONENAND_IO_TYPE BufferRAMSectorAddress;
    ONENAND_IO_TYPE BufferRAMSectorCount;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-OneNANDSetBufferParameters() pONI=%lX  uBuffer=%lX (%u)\n",
        pONI,
        uBuffer, uBuffer));

    BufferRAMSectorAddress = OneNANDBSAFromBufferSector(uBuffer, 0);
    BufferRAMSectorCount = OneNANDBSCFromCount(pONI, pONI->uSectorsPerPage);

    /*  FlexOneNand parts won't do loads or programs from any buffer starting
        location other than the first sector of the first buffer:
    */

    if (pONI->fIsFlexOneNAND)
        DclAssert(BufferRAMSectorAddress == ONENAND_VAL_BSA_DATARAM0_0);

    StartBufferVal = (BufferRAMSectorAddress << ONENAND_BSA_SHIFT)
                   | BufferRAMSectorCount;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDSetBufferParameters() StartBufferVal=%lX\n",
        StartBufferVal));

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_BUFFER, StartBufferVal);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-OneNANDSetBufferParameters() returning.\n"));
}


/*-------------------------------------------------------------------
    Local: OneNANDSelectDataRam()

    Programs the OneNAND register that chooses which core DataRAM
    to use for DDP devices

    Parameters:
        pONI  - Pointer to the OneNANDInfo structure (contains
                info about device geometry).
        uCore - Indicator of which core to use in DDP device

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDSelectDataRAM(const PONENANDINFO pONI,
                                 D_UINT16           uCore)
{
    ONENAND_IO_TYPE StartAddress2Val;

    /*  Set up the buffer, relevant on the DDP devices
    */
    StartAddress2Val = uCore << ONENAND_DBS_SHIFT;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-OneNANDSelectDataRAM() StartAddress2Val=%lX\n",
        StartAddress2Val));

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_2, StartAddress2Val);
}


/*-------------------------------------------------------------------
    Local: OneNANDStartLoad()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Load operation.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
               info about device geometry).

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartLoad(const PONENANDINFO pONI)
{
    /*  Clear interrupts.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write load command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_LOAD_DATA);
}


/*-------------------------------------------------------------------
    Local: OneNANDStartProgram()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Program operation.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
               info about device geometry).

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartProgram(const PONENANDINFO pONI)
{
    /*  Clear interrupts.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write program command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_PROGRAM_DATA);
}


/*-------------------------------------------------------------------
    Local: OneNANDStartSpareProgram()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Program Spare Area operation. Note that the
    FlexOneNAND device does not support this operation; logic to gate
    calling this procedure must be in place to avoid this operation if
    a FlexOneNAND device is present.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
               info about device geometry).

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartSpareProgram(const PONENANDINFO pONI)
{
    /*  Clear interrupts.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write program command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_PROGRAM_SPARE);
}


/*-------------------------------------------------------------------
    Local: OneNANDStartUnlock()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Block Unlock operation.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
               info about device geometry).
    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartUnlock(const PONENANDINFO pONI)
{

    /*  Clear interrupts.
    */

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write unlock command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_UNLOCK_BLOCK);
}


/*-------------------------------------------------------------------
    Private: OneNANDGetChipCommandResult()

    Waits for any in-progress commands to complete
    (or to time-out) and  returns the status.

    Parameters:
        pONI - Pointer to the OneNAND-specific info.

    Return Value:
        FFXSTAT_SUCCESS if successful
        - or -
        The FIMResult_t error code returned by
        FfxHookNTOneNANDReadyWait, if not FFXSTAT_SUCCESS
        - or -
        The FIMResult_t error code returned by
        OneNANDError if the chip indicated an error
-------------------------------------------------------------------*/
static FFXSTATUS OneNANDGetChipCommandResult(const PONENANDINFO pONI)
{
    FFXSTATUS  ffxStat;

    /* Wait for the operation to complete
    */
    ffxStat = FfxHookNTOneNANDReadyWait(pONI->hHook);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
         FFXPRINTF(1, (NTMNAME"-OneNANDGetChipCommandResult() FfxHookNTOneNANDReadyWait() indicated a failure, Status=%lX\n", ffxStat));
    }
    else
    {
        ffxStat = OneNANDError(pONI);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
             FFXPRINTF(2, (NTMNAME"-OneNANDGetChipCommandResult() OneNANDError() indicated a failure, Status=%lX\n", ffxStat));
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: OneNANDMainBufferOffset()

    From a given buffer, sector, and buffer size, compute the offest
    from the OneNAND base offset to the desired address in the main buffer
    array.

    Parameters:
        uBuffer         - Which DataRAM buffer we wnat to use
        uDataBufferSize - Size of the buffer (different between OneNAND and
                          FlexOneNAND)

    Return Value:
        Memory-mapped Buffer Offset
-------------------------------------------------------------------*/
static D_UINT16 OneNANDMainBufferOffset(D_UINT16 uBuffer, D_UINT16 uDataBufferSize)
{
    D_UINT16 uMainOffset;

    uMainOffset = ONENAND_REG_DATARAM_MAIN
               + (uBuffer * uDataBufferSize / sizeof(ONENAND_IO_TYPE));

    return uMainOffset;

}


/*-------------------------------------------------------------------
    Local: OneNANDSpareBufferOffset()

    From a given buffer, sector, and buffer size, compute the offest
    from the OneNAND base offset to the desired address in the apare buffer
    array.

    Parameters:
        uBuffer         - Which buffer we want to use
        uDataBufferSize - Size of the apare buffer

    Return Value:
        Memory-mapped Buffer Offset
-------------------------------------------------------------------*/
static D_UINT16 OneNANDSpareBufferOffset(D_UINT16 uBuffer, D_UINT16 uSpareBufferSize)
{
    D_UINT16 uSpareOffset;

    uSpareOffset = ONENAND_REG_DATARAM_SPARE
                 + (uBuffer * uSpareBufferSize / sizeof(ONENAND_IO_TYPE));

    return uSpareOffset;
}


/*-------------------------------------------------------------------
    Local: OneNANDGetTag()

    Retrieve the tags from the spare area buffer specified by pSpare
    into the tag (allocation entry) buffer specified by pTag.

    NOTE: If the function is unable to decode the tag from the buffer,
          a null tag value is returned.

    Parameters:

        pONI     - Pointer to the OneNAND-specific info
        pTag     - pointer to location in which to place retrieved Tag data
        pSpare   - pointer to buffer containing spare area data from
                   which to retrieve the tag
        nTagSize - Size of the tag data

    Return Value:
        Tag in the pTag buffer. Tag will be zero if tag data cannot
        be decoded.
-------------------------------------------------------------------*/
static void  OneNANDGetTag(
    const PONENANDINFO pONI,
    D_BUFFER           *pTag,
    const D_BUFFER     *pSpare,
    unsigned            nTagSize)
{
    if (pONI->fHWEcc)
    {
        /*  Get the tag from the ECC-managed portion of the spare area buffer
            for the first sector in the page
        */
        DclAssert(nTagSize == FFX_NAND_TAGSIZE);
        DclAssert(nTagSize == 2);
        OneNANDNtmTagDecode(pTag, &pSpare[ONENAND_SECTOR_TAG_OFFSET]);
    }
    else
    {
        /*  All supported OneNAND parts use the OffsetZero FBB marking.
        */
        DclAssert(FfxNtmHelpGetSpareAreaFormat(pONI->pChip) == NSF_OFFSETZERO);
        FfxNtmHelpTagDecode(pTag, &pSpare[NSOFFSETZERO_TAG_OFFSET]);
    }

    return;
}


/*-------------------------------------------------------------------
    Local: OneNANDSetTag()

    Place the allocation entry specified by pTag into the spare area
    buffer specified by pSpare.

    Parameters:
        pONI     - Pointer to the OneNAND-specific info
        pTag     - pointer to location from which to get Tag data
        pSpare   - pointer to buffer containing spare area data to
                   which to set the tag
        nTagSize - Size of the tag data

    Return Value:
        Tag in the pTag buffer. Tag will be zero if tag data cannot
        be decoded.
-------------------------------------------------------------------*/
static void  OneNANDSetTag(
    const PONENANDINFO pONI,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    unsigned           nTagSize)
{
    DclAssert(pTag);

    /*  Currently only used for the assertion for HW ecc tag size.
    */
    (void)nTagSize;

    if (pONI->fHWEcc)
    {
        /*  Get the tag from the ECC-managed portion of the spare area buffer
            for the first sector in this page
        */
        DclAssert(nTagSize == FFX_NAND_TAGSIZE);
        DclAssert(nTagSize == 2);
        OneNANDNtmTagEncode(&pSpare[ONENAND_SECTOR_TAG_OFFSET], pTag);
    }
    else
    {
        /*  All supported OneNAND parts use the OffsetZero FBB marking.
        */
        DclAssert(FfxNtmHelpGetSpareAreaFormat(pONI->pChip) == NSF_OFFSETZERO);
        FfxNtmHelpTagEncode(&pSpare[NSOFFSETZERO_TAG_OFFSET], pTag);
    }
}


/*-------------------------------------------------------------------
    Local: OneNANDBSAFromBufferSector()

    Convert a buffer and sector into a OneNAND BSA register value.

    Parameters:
        uBuffer         - Which buffer we want to use
        uDataBufferSize - Which sector in the buffer we want to use

    Return Value:
        BSA register value that may be used to program device BSA
-------------------------------------------------------------------*/
static ONENAND_IO_TYPE OneNANDBSAFromBufferSector(
    D_UINT16  uBuffer,
    D_UINT16  uSector)
{
    ONENAND_IO_TYPE BufferRAMSectorAddress;

    static ONENAND_IO_TYPE BufferRAMSectorAddressVals[2][4] =
    {
        {
            ONENAND_VAL_BSA_DATARAM0_0,
            ONENAND_VAL_BSA_DATARAM0_1,
            ONENAND_VAL_BSA_DATARAM0_2,
            ONENAND_VAL_BSA_DATARAM0_3,
        },
        {
            ONENAND_VAL_BSA_DATARAM1_0,
            ONENAND_VAL_BSA_DATARAM1_1,
            ONENAND_VAL_BSA_DATARAM1_2,
            ONENAND_VAL_BSA_DATARAM1_3,
        },
    };

    BufferRAMSectorAddress = BufferRAMSectorAddressVals[uBuffer][uSector];

    return BufferRAMSectorAddress;

}


/*-------------------------------------------------------------------
    Local: OneNANDBSCFromCount()

    Convert a count into a OneNAND BSC register value.

    Parameters:
        pONI   - Pointer to OneNAND private info
        uCount - Sector count

    Return Value:
        Sector count register value that may be used to program device BSC
-------------------------------------------------------------------*/
static ONENAND_IO_TYPE OneNANDBSCFromCount(
    PONENANDINFO pONI,
    D_UINT16 uCount)
{
    ONENAND_IO_TYPE BufferRAMSectorCount;

/*  Don't set more bits than allowed by the number of available sectors:
*/

    DclAssert(uCount <= pONI->uSectorsPerPage);
    if (uCount == pONI->uSectorsPerPage)
    {
        BufferRAMSectorCount = 0;
    }
    else
    {
        BufferRAMSectorCount = uCount;
    }

    return BufferRAMSectorCount;
}


/*-------------------------------------------------------------------
    Local: OneNANDStartErase()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Block Erase operation.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
              info about device geometry).

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartErase(
    const PONENANDINFO  pONI)
{

    /*  Clear interrupts.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write erase command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_BLOCK_ERASE);
}


/*-------------------------------------------------------------------
    Private: OneNANDFlexOneNANDError()

    FlexOneNAND-specific procedure for reading error status registers
    from the FlexOneNAND device and converting that information into
    a standard FFXSTATUS indication. Need a separate procedure for
    FlexOneNAND because it's EDC error indication registers are
    different from original OneNAND devices.

    Parameters:
        pONI - Pointer to OneNAND private info

    Return Value:
        Standard FFXSTATUS indication
-------------------------------------------------------------------*/

static FFXSTATUS OneNANDFlexOneNANDError(const PONENANDINFO pONI)
{
    ONENAND_IO_TYPE Status;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    /*  Read the status register and decode its contents. If an error is
        indicated, read more registers to figure out what it is.
    */
    Status = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_CONTROLLER_STATUS);
    if ((Status & ONENAND_REG_STATUS_ERROR) == ONENAND_REG_STATUS_ERROR)
    {
    D_UINT16 i;
    ONENAND_IO_TYPE ECCStatus;

        /*  At this point we've determined that an error occured. Read
            the EDC status registers to see if its an uncorrectible
            ECC error.
        */

         FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
             "NT1NAND-FlexOneNANDError() chip controller status 0x%04x, indicating an error.\n",
             Status));

        for (i = 0; i < 4; i++)
        {
            ECCStatus = FfxHookNTOneNANDReadRegister(pONI->hHook, (D_UINT16) (i+ONENAND_REG_ECC_STATUS));
            if((ECCStatus & FLEX_ECC_STATUS_MASK) > FLEX_ECC_MAX_CORRECTABLE_ERROR)
                ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
            ECCStatus >>= FLEX_ECC_HIBYTE_SHIFT;
            if ((ECCStatus & FLEX_ECC_STATUS_MASK) > FLEX_ECC_MAX_CORRECTABLE_ERROR)
                ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
        }
        if (!(ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA))
        {

            /*  If not an uncorrectible ECC error, return an general IO
                error status. This will happen if the error bit is set
                in the status register but there is no evidence of
                uncorrectable bit errors. Since the status register for
                FlexOneNAND (unlike OneNAND) does not encode information
                about read or write errors, we simply return FIM IOERROR
                status here and let higher-level code which is more aware
                of the context sort out why.
            */
            ffxStat = FFXSTAT_FIMIOERROR;
        }
    }
    return ffxStat;
}

/*-------------------------------------------------------------------
    Private: OneNANDError()

    Procedure for reading error status registers from a OneNAND
    device and converting that information intoa standard FFXSTATUS
    indication.

    Parameters:
        pONI - Pointer to OneNAND private info

    Return Value:
        Standard FFXSTATUS indication
-------------------------------------------------------------------*/
static FFXSTATUS OneNANDError(
    const PONENANDINFO  pONI)
{
ONENAND_IO_TYPE Status;
FFXSTATUS ffxStat;

    /* FlexOneNAND error status indications for error correction and command results
       are notably different from the original OneNAND parts:
    */
    if (pONI->fIsFlexOneNAND)
        return OneNANDFlexOneNANDError(pONI);

    Status = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_CONTROLLER_STATUS);

    if ((Status & ONENAND_REG_STATUS_ERROR) == ONENAND_REG_STATUS_ERROR)
    {
        FFXPRINTF(2, (NTMNAME"-OneNANDError() chip controller status %X, indicating an error.\n", Status));

        if (Status & ONENAND_REG_STATUS_LOAD)
        {
            ONENAND_IO_TYPE ECCStatus;

            /*  A load error occured. The only situation in which a load error
                can occur without the existence of uncorrectable errors is if
                the OneNAND device gets unexpectedly reset while performing a
                load. FlashFX never does that deliberately, but there are
                unusual situations in hardware that could cause such a thing
                to happen. Therefore, initialize the return status to indicate
                that an unknown hardware status exists, and then try to
                discover if it is an uncorrectable bit error.
            */
            ffxStat = FFXSTAT_FIM_UNEXPECTEDHWSTATE;

            /*  Look for uncorrectable errors.
            */
            ECCStatus = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_ECC_STATUS);
            if (ECCStatus & ONENAND_ECCSTATUS_UNCORRECTABLEMAIN_BITMASK)
                ffxStat = FFXSTAT_FIMUNCORRECTABLEMAIN;

            if (ECCStatus & ONENAND_ECCSTATUS_UNCORRECTABLESPARE_BITMASK)
            {
                if (ffxStat == FFXSTAT_FIMUNCORRECTABLEMAIN)
                    ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
                else
                    ffxStat = FFXSTAT_FIMUNCORRECTABLESPARE;
            }
        }
        else if (Status & ONENAND_REG_STATUS_PROG)
        {
            ffxStat = FFXSTAT_FIMIOERROR;
        }
        else if (Status & ONENAND_REG_STATUS_ERASE)
        {
            ffxStat = FFXSTAT_FIM_ERASEFAILED;
        }
        else
        {
            /*  Although OneNAND supports erase suspend, FlashFX does
                suspend an erase operation for OneNAND devices. Therefore
                the only way we can get here is the issuance of an
                invalid command, which would be a software fault:
            */
            DclError();
            ffxStat = FFXSTAT_INTERNAL_ERROR;
        }
    }
    else
    {
        ffxStat = FFXSTAT_SUCCESS;
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: OneNANDStartSpareLoad()

    Writes the binary commands to the necessary registers in the OneNAND
    device to initiate the Spare Area Load operation.

    Parameters:
        pONI - Pointer to the OneNANDInfo structure (contains
               info about device geometry).

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDStartSpareLoad(
    const PONENANDINFO  pONI)
{
    /*  Clear interrupts.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_INTERRUPT, 0);

    /*  write load command
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_LOAD_SPARE);

}

/*  The following two procedures are used in lieu of equivalent
    procedures in nthelp.c, specifically for OneNAND/FlexOneNAND
    devices that have hardware ECC mechanisms enabled. The primary
    reason is because with hardware ECC, we do not need to maintain
    the Hamming code because tags are covered by the OneNAND ECC
    mechanism. However, we do want to maintain the checkbyte because
    of the propensity of error correction algorithms to return
    false corrections if there are more bit errors than the algorithm
    is designed to manage.
*/

/*-------------------------------------------------------------------
    Local: OneNANDNtmTagEncode()

    Build the allocation entry.  The first two bytes are the
    standard media-independent value, and in keeping with the
    peculiarities of the OneNAND/FlexOneNAND spare area format,
    the third byte is left alone. The fourth byte then holds
    the check byte. Note that for properly corrected errors,
    the checkbyte *must* verify. If the device reports a
    correctable error but the checkbyte does not verify, it
    is a sure indication that there are multiple bit errors
    in the tag that the OneNAND ECC mechanism has falsely
    corrected. This is a positive indicator that the tag has
    been invalidated.

    Parameters:
        pDest - The 4-byte destination buffer to fill with the
                2-byte tag data and its checkbyte.
        pSrc  - The 2-byte allocation entry.

    Return Value:
        Returns TRUE if the tag was properly encoded and placed in
        the buffer specified by pDest, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL OneNANDNtmTagEncode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    DclAssert(pDest);
    DclAssert(pSrc);
    if(!pDest || !pSrc)
        return FALSE;

    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = (D_UINT8)(pDest[0] ^ ~pDest[1]);
    pDest[3] = ERASED8;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: OneNANDNtmTagDecode()

    Decode the allocation entry. The first two bytes are the
    standard media-independent value, and in keeping with the
    peculiarities of the OneNAND/FlexOneNAND spare area format,
    the third byte is left alone. The fourth byte then holds
    the check byte. Note that for properly corrected errors,
    the checkbyte *must* verify. If the device reports a
    correctable error but the checkbyte does not verify, it
    is a sure indication that there are multiple bit errors
    in the tag that the OneNAND ECC mechanism has falsely
    corrected. This is a positive indicator that the tag has
    been invalidated.

    Parameters:
        pDest - The destination buffer to fill with the 2-byte tag
                data.
        pSrc  - The 4-byte source buffer to process.

    Return Value:
        Returns TRUE if the tag was properly decoded and placed in
        the buffer specified by pDest, otherwise FALSE.  If FALSE
        is returned, the buffer specified by pDest will be set to
        NULLs.
-------------------------------------------------------------------*/
static D_BOOL OneNANDNtmTagDecode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];
    D_UCHAR         ucZeroBits = ERASED8;
    D_UCHAR         ucIndex;

    DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));

    DclAssert(pDest);
    DclAssert(pSrc);

    /*  If the encoded tag is in the fully erased state, return
        successfully, after having set the decoded tag to the
        erased state as well. Note that for alignment purposes,
        the tag has to be examined byte-by-byte.

        It is possible that a page/tag which is otherwise erased
        has a single bit error in the tag area which makes it
        appear as if the tag has been written.  In this event,
        we let the normal check byte stuff fail, and return that
        the tag is bogus.  The allocator will then recycle the page,
        and it will subsequently get erased and re-used. Note that\
        the OneNAND NTM eschews the Hamming code, because the tag
        is covered by the ECC mechanism.
    */
    for (ucIndex = 0; ucIndex < LEGACY_ENCODED_TAG_SIZE; ucIndex++)
        ucZeroBits &= pSrc[ucIndex];
    if(ucZeroBits == ERASED8)
    {
        pDest[0] = ERASED8;
        pDest[1] = ERASED8;

        return TRUE;
    }

    DclMemCpy(TempBuff, pSrc, LEGACY_ENCODED_TAG_SIZE);

    /*  Check the validation code
    */
    if(TempBuff[2] != (D_UCHAR) (TempBuff[0] ^ ~TempBuff[1]))
    {
        FFXPRINTF(1, ("OneNANDNtmTagDecode() Unrecoverable tag error, Src=%02X %02X %02X %02X Corrected=%02X %02X %02X %02X\n",
            pSrc[3], pSrc[2], pSrc[1], pSrc[0],
            TempBuff[3], TempBuff[2], TempBuff[1], TempBuff[0]));

        pDest[0] = 0;
        pDest[1] = 0;

        return FALSE;
    }

    pDest[0] = TempBuff[0];
    pDest[1] = TempBuff[1];

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: OneNANDBuildSpareArea()

    Initializes the spare area with the fixed values. ECC's are
    generated by the OneNAND NTM only if the OneNANDInfo structure
    shows that the hardware ECC generation mechanism is off.

    NOTE:  This function is only appropriate for use where
           sofware ECC is being used.

    Parameters:
        pData   - A pointer to the main page data
        pSpare  - A pointer to the buffer in which to build the
                  spare data
        pTag    - A pointer to the tag data to store in the
                  spare area.  This pointer may be NULL if the
                  tag value is to remain unset.
        fUseEcc - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDBuildSpareArea(
    NTMHANDLE      hNTM,
    const D_BUFFER *pData,
    D_BUFFER       *pSpare,
    const D_BUFFER *pTag,
    unsigned       nTagSize,
    D_BOOL         fUseEcc)
{
    /*  The tag pointer and tag size are either always both set or both clear
    */
    DclAssert((pTag && nTagSize) || (!pTag && !nTagSize));

    /*  If fUseEcc is FALSE, then tags may not be used.
    */
    DclAssert(fUseEcc || !nTagSize);

    /* If using SW ECC, and the caller supplied a buffer, build the spare area
    */
    if (pData && !hNTM->pONI->fHWEcc)
    {
        FfxNtmHelpBuildSpareArea(hNTM->pONI->hHook,
                             &hNTM->NtmInfo,
                             pData,
                             pSpare,
                             pTag,
                             fUseEcc,
                             hNTM->pONI->pChip);
    }
    else
    {
        /*  If the caller did NOT supply a main buffer, or using HW ECC,
            the spare area is built from the tag only, everything else
            is either left intact or provided by the OneNAND internals
            when the program command is given.
        */
        DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

        /*  Place the allocation entry into the spare buffer
        */
        if (nTagSize)
            OneNANDSetTag(hNTM->pONI, pSpare, pTag, nTagSize);
    }
}


/*-------------------------------------------------------------------
    Local: OneNANDCheckCorrect()

    Checks the status of error correction after a data load.
    Distinguishes between hardware and software error correction
    techniques

    NOTE:  This function is only appropriate for use where
           sofware ECC is being used.

    Parameters:
        hNTM    - A pointer to the NTM handle, used to access OneNAND
                  specific data
        pPage   - A pointer to the main page data. Used to calculate
                  ECC's for verification if software ECC was used.
        pSpare  - A pointer to the spare area containing ECC's. Used
                  in software ECC verification
        pIOStat - A pointer to an IOStat variable in which error
                  correction results can be stored.
        fUseEcc - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDCheckCorrect(
    NTMHANDLE           hNTM,
    D_BUFFER           *pPage,
    D_BUFFER           *pSpare,
    FFXIOSTATUS        *pIOStat,
    D_BOOL              fUseEcc)
{
    /*  This function is a no-op unless all of the required
        parameters are supplied and fUseEcc is TRUE.
    */
    if (fUseEcc)
    {
        if (hNTM->pONI->fHWEcc)
        {
        D_UINT32 i;
        D_UINT32 ulCorrections = 0;
        ONENAND_IO_TYPE ECCStatus;

            /* Using HW Ecc, determine if this page was corrected */


            /*  FlexOneNAND correctible error conditions are different from the original
                OneNAND parts, because it supports more rigorous error correction
            */
            if (hNTM->pONI->fIsFlexOneNAND)
            {

                for (i=0; i<4; i++)
                {
                    ECCStatus = FfxHookNTOneNANDReadRegister(hNTM->pONI->hHook, (D_UINT16)(i+ONENAND_REG_ECC_STATUS));
                    DclAssert((ECCStatus & FLEX_ECC_STATUS_MASK) <= FLEX_ECC_MAX_CORRECTABLE_ERROR);
                    if (ECCStatus & FLEX_ECC_STATUS_MASK)
                    {
                        /*  Refer to FlexOneNAND documentation on how the number
                            of bit errors is encoded in the ECC status registers
                            for why this peculiar code is here:
                        */
                        ulCorrections += OneNANDPow2(ECCStatus) + 1;
                    }
                    ECCStatus >>= FLEX_ECC_HIBYTE_SHIFT;
                    DclAssert((ECCStatus & FLEX_ECC_STATUS_MASK) <= FLEX_ECC_MAX_CORRECTABLE_ERROR);
                    if (ECCStatus & FLEX_ECC_STATUS_MASK)
                    {
                        /*  Refer to FlexOneNAND documentation on how the number
                            of bit errors is encoded in the ECC status registers
                            for why this peculiar code is here:
                        */
                        ulCorrections += OneNANDPow2(ECCStatus) + 1;
                    }
                }
                if (ulCorrections)
                {
                    FfxErrMgrEncodeCorrectedBits(ulCorrections, pIOStat);
                        pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                    pIOStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                }
                else
                {
                    pIOStat->ffxStat = FFXSTAT_SUCCESS;
                }
            }

            /*  Not a Flex part:
            */
            else
            {
                ECCStatus = FfxHookNTOneNANDReadRegister(hNTM->pONI->hHook, ONENAND_REG_ECC_STATUS);
                if (ECCStatus != 0)
                {
                    /*  Shouldn't get here for multi-bit errors or reserved values
                    */
                    DclAssert((ECCStatus & ONENAND_ECCSTATUS_UNCORRECTABLE_BITMASK) == 0);

                    /*  Count the corrections:
                    */
                    for (i=0; i<8; i++)
                    {
                        if (ECCStatus & 0x03)
                            ulCorrections++;
                    }
                    FfxErrMgrEncodeCorrectedBits(ulCorrections, pIOStat);
                    pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                    pIOStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                }
            }
        }
        else if (pSpare && pPage)
        {
            D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
            FFXIOSTATUS ioValid;
            
            FfxHookEccCalcStart(hNTM->pONI->hHook, pPage, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);
            FfxHookEccCalcRead(hNTM->pONI->hHook, pPage, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);

            ioValid = FfxHookEccCorrectPage(hNTM->pONI->hHook, pPage, pSpare, abECC, hNTM->pONI->pChip);

            pIOStat->ffxStat = ioValid.ffxStat;
            pIOStat->op.ulPageStatus |= ioValid.op.ulPageStatus;
        }
    }

}


/*-------------------------------------------------------------------
    Local: OneNANDReadChipId()

    Read the ID from a OneNAND chip.

    Parameters:
        pONI  - OneNAND project hooks private data
        pabID - A ONENAND_ID_SIZE-byte buffer to receive the ID

    Return Value:
        None.
-------------------------------------------------------------------*/
static void OneNANDReadChipId(
    const PONENANDINFO pONI,
    D_BYTE             *pabID)
{
    ONENAND_IO_TYPE ManufacturerId;
    ONENAND_IO_TYPE DeviceId;

    DclAssert(pONI);
    DclAssert(pabID);

    /*  Ensure that any bytes we don't explicitly touch are initialized
    */
    DclMemSet(pabID, 0, ONENAND_ID_SIZE);

    ManufacturerId = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_MANUFACTURER_ID);
    DeviceId = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_DEVICE_ID);

    FFXPRINTF(1, (NTMNAME"-OneNANDReadChipId() read MfgID=0x%04x DevID=0x%04x\n", ManufacturerId, DeviceId));

    pabID[0] = ManufacturerId & 0xFF;
    pabID[1] = DeviceId & 0xFF;

    /*  Some of the newer FlexOneNAND chips have information in the upper byte
        of the DeviceID read from the chip, and this information is necessary
        to distinguish these devices from some of the earlier ones. Capture
        this data, and gate it with the size of the ID read (if the ID was only
        one byte in size, there is no "upper byte").

        (That last sentence makes no sense -- ONENAND_ID_SIZE is a constant...)
    */
    if ((sizeof(ONENAND_IO_TYPE) > 1) && (ONENAND_ID_SIZE >= 4))
    {
        pabID[2] = (DeviceId >> 8) & 0xFF;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: OneNANDResetChip()

    Reset the specified OneNAND chip

    Parameters:
        pONI  - OneNAND private data
        uChip - the chip to reset

    Return Value:
        Returns FIM_OK if successful, or a FIM error code if not.
-------------------------------------------------------------------*/
static FFXSTATUS OneNANDResetChip(const PONENANDINFO pONI)
{
    FFXSTATUS ffxStat;

    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);


    /*  Wait for completion of the program operation and return the result
     */
    ffxStat = OneNANDGetChipCommandResult(pONI);


    if (ffxStat == FFXSTAT_SUCCESS)
    {

        /*  Set or clear the ECC bit based on the configuration option.  Leave
            the other bits as their defaults for now
        */
        OneNANDConfigureEcc(pONI, pONI->fHWEcc);

        /*  Set or clear the sync mode settings based on the configuration option.
            Leave the other bits as their defaults for now.
        */
        OneNANDConfigureMode(pONI, pONI->onp.fUseSynchBurstMode);

    }
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-OneNANDResetChip() returning. ffxStat: %lX (%u)\n",
        ffxStat, ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: OneNANDConfigureECC()

    Configure the OneNAND chip to either perform hardware ECC generation/
    validation or not, depending on whether the NTM has been configured to
    use hardware EDC and whether the caller has asked for EDC to be
    performed.

    Parameters:
        pONI   - OneNAND private data
        fHWEcc - Indicates whether ECC generation/validation has
                 been requested

    Return Value:
        Returns FIM_OK if successful, or a FIM error code if not.
-------------------------------------------------------------------*/
static D_BOOL OneNANDConfigureEcc(const PONENANDINFO pONI, D_BOOL fHWEcc)
{
    ONENAND_IO_TYPE Config1Val;
    D_BOOL          fOldEcc;

    Config1Val = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1);

    /*  Determine the current state, for return to the caller
    */
    if (Config1Val & ONENAND_REG_SCR1_ECC)
    {
        /*  bypass bit is set, ECC is disabled
        */
        fOldEcc = FALSE;
        if (fHWEcc)
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),NTMNAME"-OneNANDConfigureEcc() turning hardware ECC ON.\n"));
            Config1Val &= ~ONENAND_REG_SCR1_ECC;
            FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1, Config1Val);
        }
        /*  else don't do anything, it's already disabled and they're asking
            to disable it
        */
    }
    else
    {
        /*  bypass bit is clear, ECC is enabled
        */
        fOldEcc = TRUE;
        if (!fHWEcc)
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),NTMNAME"-OneNANDConfigureEcc() turning hardware ECC OFF.\n"));
            Config1Val |= ONENAND_REG_SCR1_ECC;
            FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1, Config1Val);
        }
        /*  else don't do anything, it's already enabled and they're asking
            to enable it
        */
    }
    return fOldEcc;
}


/*-------------------------------------------------------------------
    Local: OneNANDConfigureMode()

    Configure the OneNAND chip to either use sychronous burst I/O
    or not. Note that if the option is turned on, the surrounding
    platform must support the operation, otherwise it may not be
    possible to communicate with the device.

    Parameters:
        pONI   - OneNAND private data
        fSynch - Indicates whether ECC generation/validation has
                 been requested

    Return Value:
        Returns FIM_OK if successful, or a FIM error code if not.
-------------------------------------------------------------------*/
static D_BOOL OneNANDConfigureMode(const PONENANDINFO pONI, D_BOOL fSynch)
{
    ONENAND_IO_TYPE Config1Val;
    D_BOOL          fOldSync;

    Config1Val = FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1);


    /*  Enable sync burst mode - Note that it is insufficient to simply set
        this bit. The hardware interface capabilities must match this setting.
        Typically a BSP or project hooks with which this code is used must
        configure the interface for synchronous access.
    */


    /*  Determine the current state, for return to the caller
    */
    if (Config1Val & ONENAND_REG_SCR1_RM)
    {
        /* synchronous mode is set */
        fOldSync = TRUE;
        if (!fSynch)
        {
            FFXPRINTF(1, (NTMNAME"-OneNANDConfigureMode() turning synchronous burst mode OFF.\n"));
            Config1Val &= ~(ONENAND_REG_SCR1_RM
                          | ONENAND_REG_SCR1_IOBE);
            FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1, Config1Val);
        }
        /*  else don't do anything, it's already synchronous and the caller is
            requesting synchronous
        */
    }
    else
    {
        /*  synchronous mode is clear (asynchronous)
        */
        fOldSync = FALSE;
        if (fSynch)
        {
            FFXPRINTF(1, (NTMNAME"-OneNANDConfigureMode() turning synchronous burst mode ON.\n"));
            Config1Val |= ONENAND_REG_SCR1_RM
                        | ONENAND_REG_SCR1_IOBE;

            Config1Val |= 0x0600;  /* Burst Length = 16 words. */

            FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_SYSTEM_CONFIG_1, Config1Val);
        }
        /*  else don't do anything, it's already asynchronous and they're asking for asynchronous
        */
    }

    return fOldSync;
}


/*-------------------------------------------------------------------
    Local: OneNANDUnlockBlock()

    Unlock the specified block.

    Parameters:
        pONI   - OneNAND private info
        uBlock - Block number to unlock

    Return Value:
        Returns FIM_OK if successful, or a FIM error code if not.
-------------------------------------------------------------------*/
static FFXSTATUS OneNANDUnlockBlock(
    const PONENANDINFO pONI,
    D_UINT16 uBlock)
{
    FFXSTATUS ffxStat;

    /* set up the block number to unlock
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_BLOCK_ADDRESS, uBlock);

    /*  Tell the flash to unlock that blocks now
    */
    OneNANDStartUnlock(pONI);

    /*  Wait for completion of the unlock operation.  Correctly formatted
        unlock operations should never fail at run-time.  Therefore, assert on
        the return status and ignore it during the normal course of operation.
    */
    ffxStat = OneNANDGetChipCommandResult(pONI);

    if(ffxStat == FFXSTAT_SUCCESS)
    {
        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_1, uBlock);
        if((FfxHookNTOneNANDReadRegister(pONI->hHook, ONENAND_REG_WRITE_PROTECT_STATUS) & ONENAND_REG_FWPS_UNLOCKED)
            != ONENAND_REG_FWPS_UNLOCKED)
        {
            ffxStat = FFXSTAT_FIM_UNLOCKFAILED;
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Private: OneNANDGetFlexPartition()

    Get SLC/MLC partition information from a FlexOneNAND part.

    Beware that this function should only be called at initialization,
    as it will overwrite the BufferRAMs in order to get at the partition
    interface. It will also reset the part at the end of its operations.

    Parameters:
        pONI - OneNAND private info

    Return Value:
        FlexOneNAND Partition location
 -------------------------------------------------------------------*/

#define ONENAND_FLEX_PARTITION_MASK (0x3FF)
static D_UINT16 OneNANDGetFlexPartition(ONENANDINFO *pONI)
{
    ONENAND_IO_TYPE partitionInfo;
    FFXSTATUS status;

    /*  Determine if the part is a flex part. Do nothing if it isn't.
    */
    if (!(pONI->fIsFlexOneNAND))
        return 0;

    /*  Access to the partition data is available in the "PI" block of a FlexOneNand device,
        in the first word of the first page of that block. Run the following procedure
        to get it:

            1. Enter the PI block access mode.
            2. Set up to FPA to read the first page.
            3. Load the page into DataRam
            4. Read the first word of the page
            5. Mask off the partition information
            6. Reset the part, and return the partitino information.
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_PI_ACCESS);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_8, 0);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_LOAD_DATA);
    status =  FfxHookNTOneNANDReadyWait(pONI->hHook);
    FfxHookNTOneNANDReadBuffer(pONI->hHook, &partitionInfo, ONENAND_REG_DATARAM_MAIN, 1);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
    status =  FfxHookNTOneNANDReadyWait(pONI->hHook);
    partitionInfo &= ONENAND_FLEX_PARTITION_MASK;
    if (status != FFXSTAT_SUCCESS) return 0;
    return (D_UINT16)partitionInfo;

}

/*-------------------------------------------------------------------
    Private: OneNANDSetFlexPartition()

    Set SLC/MLC partition on a FlexOneNAND part.

    Beware that this function should only be called at initialization, as it will
    overwrite the BufferRAMs in order to get at the partition interface. It will
    lso reset the part at the end of its operations.

    Parameters:
        ONI    - OneNAND private data
        uBlock - Partition setting

    Return Value:
        Returns FIM_OK if successful, or a FIM error code if not.
 -------------------------------------------------------------------*/

#define FLEXONENAND_PI_UNLOCKED 0xFC00
static FFXSTATUS OneNANDSetFlexPartition(ONENANDINFO *pONI, D_UINT16 uPartition)
{
    D_UINT16 *pPageBuff;
    D_UINT16 *pSpareBuff;
    D_UINT32 ulPageSize;
    D_UINT32 ulSpareSize;
    D_UINT32 i;
    FFXSTATUS status;
    FFXIOSTATUS ioStat;

    /*  Determine if the part is a flex part. Do nothing if it isn't.
    */
    if (!(pONI->fIsFlexOneNAND))
        return FFXSTAT_SUCCESS;

    /*  If the requested partition is beyond the end of the device, set
        the partition at the end of the device:
    */
    if (uPartition > ONENAND_FLEX_PARTITION_MASK)
        uPartition = ONENAND_FLEX_PARTITION_MASK;

    /*  Access to the partition data is available in the "PI" block of a FlexOneNand
        device, in the first word of the first page of that block. Run the following
        procedure to set it:

            1. Enter the PI block access mode.
            2. Erase the first block.
            3. Send the PI program sequence with the necessary block number.
            4. Issue the PI update command
            5. Hot reset the part
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_PI_ACCESS);
    ioStat = OneNANDEraseBlocks(pONI, 0, 1);
    if (ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
        return ioStat.ffxStat;
    }

    /*  Build page buffer and spare area for programming the partition page. The
        page data will be all FFFF except for the first word, which will contain
        the partition information. The spare area will be all FFFF.
    */
    ulPageSize = pONI->uDataBufferSize*pONI->uDataBuffersPerPage;
    ulSpareSize = pONI->uSpareBufferSize*pONI->uDataBuffersPerPage;
    pPageBuff = DclMemAlloc(ulPageSize);
    if (!pPageBuff)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
            "NT1NAND-OneNANDNTMSetFlexPartition() DclMemAlloc failed: %lX (%u)\n",
            ulPageSize, ulPageSize));
        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
        return FFXSTAT_OUTOFMEMORY;
    }
    pSpareBuff = DclMemAlloc(ulSpareSize);
    if (!pSpareBuff)
    {
        DclMemFree(pPageBuff);
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
            "NT1NAND-OneNANDNTMSetFlexPartition() DclMemAlloc failed: %lX (%u)\n",
            ulSpareSize, ulSpareSize));
        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
        return FFXSTAT_OUTOFMEMORY;
    }

    *pPageBuff = FLEXONENAND_PI_UNLOCKED | uPartition;
    for (i = 1; i < ulPageSize/ONENAND_DATA_WIDTH_BYTES; i++)
        *(pPageBuff + i) = D_UINT16_MAX;
    for (i = 0; i < ulSpareSize/ONENAND_DATA_WIDTH_BYTES; i++)
        *(pSpareBuff + i) = D_UINT16_MAX;

    /*  Perform the program:
    */
    OneNANDProgramInit(pONI,
                       0,
                       (D_BUFFER *)pPageBuff,
                       (D_BUFFER *)pSpareBuff,
                       0);
    OneNANDProgramStart(pONI,
                        0,
                        0,
                        TRUE);
    status = OneNANDGetChipCommandResult(pONI);
    DclMemFree(pPageBuff);
    DclMemFree(pSpareBuff);
    if(status != FFXSTAT_SUCCESS)
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
            "NT1NAND-OneNANDNTMSetFlexPartition() OneNANDGetChipCommandResult failed: %lX (%u)\n",
            status, status));
        FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
        return status;

    }

    /*  Send the command to update the partition info:
    */
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_1, 0x0000);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_BUFFER, 0x0800);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_START_ADDRESS_8, 0x0000);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_PI_UPDATE);
    status =  FfxHookNTOneNANDReadyWait(pONI->hHook);
    FfxHookNTOneNANDWriteRegister(pONI->hHook, ONENAND_REG_COMMAND, ONENAND_REG_CMD_RESET_ONENAND);
    status =  FfxHookNTOneNANDReadyWait(pONI->hHook);
    return status;
}

/*-------------------------------------------------------------------
    Private: OneNANDPow2()

    Utility function to find the exponent to whcih one must raise 2
    in order to get a number. The number is checked for being an
    even power of two. For example, "OneNANDPow2(0x100)" will
    return "8".

    Parameters:
        ulNum - Input value to be checked

    Return Value:
        Exponent value
-------------------------------------------------------------------*/
static D_UINT32 OneNANDPow2(D_UINT32 ulNum)
{
D_UINT16 uExponent;

    DclAssert(DCLISPOWEROF2(ulNum));
    for (uExponent = 0; ulNum != 1; uExponent++)
    {
        ulNum >>= 1;
    }
    return uExponent;
}


#if D_DEBUG

/*-------------------------------------------------------------------
    FfxNtmOneNANDDumpRegisters() - debug-only helper function to
    dump the state of the OneNAND chip's registers.
-------------------------------------------------------------------*/
void FfxNtmFlexOneNANDDumpRegisters(
    NTMHOOKHANDLE   hHook)
{
 	ONENAND_IO_TYPE ECCResult[8];
	unsigned        nn;

 	ECCResult[0] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_MAIN_RESULT_1);
	ECCResult[1] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SPARE_RESULT_1);
	ECCResult[2] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_MAIN_RESULT_2);
	ECCResult[3] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SPARE_RESULT_2);
	ECCResult[4] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_MAIN_RESULT_3);
	ECCResult[5] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SPARE_RESULT_3);
	ECCResult[6] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_MAIN_RESULT_4);
	ECCResult[7] = FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SPARE_RESULT_4);

    DclPrintf("FFX: ManufacturerId:     0x%04X : 0x%04x\n", ONENAND_REG_MANUFACTURER_ID,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_MANUFACTURER_ID));
    DclPrintf("FFX: DeviceId:           0x%04X : 0x%04x\n", ONENAND_REG_DEVICE_ID,            FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_DEVICE_ID));
    DclPrintf("FFX: DataBufferSize:     0x%04X : 0x%04x\n", ONENAND_REG_DATA_BUF_SIZE,        FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_DATA_BUF_SIZE));
    DclPrintf("FFX: BootBufferSize:     0x%04X : 0x%04x\n", ONENAND_REG_BOOT_BUF_SIZE,        FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_BOOT_BUF_SIZE));
    DclPrintf("FFX: AmountOfBuffers:    0x%04X : 0x%04x\n", ONENAND_REG_AMOUNT_OF_BUF,        FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_AMOUNT_OF_BUF));
    DclPrintf("FFX: Technology:         0x%04X : 0x%04x\n", ONENAND_REG_TECHNOLOGY,           FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_TECHNOLOGY));
    DclPrintf("FFX: StartAddress1:      0x%04X : 0x%04x\n", ONENAND_REG_START_ADDRESS_1,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_ADDRESS_1));
    DclPrintf("FFX: StartAddress2:      0x%04X : 0x%04x\n", ONENAND_REG_START_ADDRESS_2,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_ADDRESS_2));
    DclPrintf("FFX: StartAddress3:      0x%04X : 0x%04x\n", ONENAND_REG_START_ADDRESS_3,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_ADDRESS_3));
    DclPrintf("FFX: StartAddress4:      0x%04X : 0x%04x\n", ONENAND_REG_START_ADDRESS_4,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_ADDRESS_4));
    DclPrintf("FFX: StartAddress8:      0x%04X : 0x%04x\n", ONENAND_REG_START_ADDRESS_8,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_ADDRESS_8));
    DclPrintf("FFX: StartBuffer:        0x%04X : 0x%04x\n", ONENAND_REG_START_BUFFER,         FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_BUFFER));
    DclPrintf("FFX: Command:            0x%04X : 0x%04x\n", ONENAND_REG_COMMAND,              FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_COMMAND));
    DclPrintf("FFX: Config1:            0x%04X : 0x%04x\n", ONENAND_REG_SYSTEM_CONFIG_1,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SYSTEM_CONFIG_1));
    DclPrintf("FFX: Config2:            0x%04X : 0x%04x\n", ONENAND_REG_SYSTEM_CONFIG_2,      FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_SYSTEM_CONFIG_2));
    DclPrintf("FFX: Status:             0x%04X : 0x%04x\n", ONENAND_REG_CONTROLLER_STATUS,    FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_CONTROLLER_STATUS));
    DclPrintf("FFX: Interrupt:          0x%04X : 0x%04x\n", ONENAND_REG_INTERRUPT,            FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_INTERRUPT));
    DclPrintf("FFX: StartBlockAddress:  0x%04X : 0x%04x\n", ONENAND_REG_START_BLOCK_ADDRESS,  FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_START_BLOCK_ADDRESS));
    DclPrintf("FFX: WriteProtectStatus: 0x%04X : 0x%04x\n", ONENAND_REG_WRITE_PROTECT_STATUS, FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_WRITE_PROTECT_STATUS));
    DclPrintf("FFX: ECCStatus:          0x%04X : 0x%04x\n", ONENAND_REG_ECC_STATUS,           FfxHookNTOneNANDReadRegister(hHook, ONENAND_REG_ECC_STATUS));
    DclPrintf("FFX: ECC results register values:\n");
	for (nn = 0; nn < 8; nn++)
	{
        DclPrintf("FFX:     %u : 0x%04x\n", nn, ECCResult[nn]);
	}

    return;
}

#endif  /* D_DEBUG */

#endif  /* FFXCONF_NANDSUPPORT_ONENAND */
#endif  /* FFXCONF_NANDSUPPORT */



