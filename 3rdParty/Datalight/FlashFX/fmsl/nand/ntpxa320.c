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

    This NAND Technology Module (NTM) supports the NAND Flash Controller in
    the PXA320 (Monahans) and related chips.

    Only NAND flash chips with a "standard" bad block marking format are (or
    ever will be) supported.  This means that 8-bit small-block parts use byte
    offset 5 in the spare area, and all 16-bit or large-block parts use byte
    or word offset 0; in all cases the specified location in the first two
    pages of a block must be all ones, or the block is considered bad.

    PXA320 DFC Notes:
    1)  The hardware ECC algorithm used is such that the ECC on an unwritten
        page (all 0xFFs) will be all zeros.  This is different than the
        standard SSFDC algorithm, which in the same scenario results in
        ECC values which are all 0xFFs as well, making it possible to
        correctly read, with ECC, even pages which have never been written.

    2)  Due to the interleaved nature of the hardware ECC algorithm (see the
        spec), two adjacent bits can both be corrected.  In the event that
        this happens, the code SBERR is still returned, even though two bits
        were in error, and both were corrected.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntpxa320.c $
    Revision 1.67  2010/12/14 00:36:45Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.65  2010/12/10 19:54:41Z  glenns
    Add code to allow the use of chips that require multi-bit correction
    but have fully-capable on-die EDC engine.
    Revision 1.64  2010/07/27 21:06:47Z  garyp
    Updated to turn off any M60 EDC when the NTM instance is destroyed.
    Revision 1.63  2010/07/13 16:09:20Z  garyp
    Partial merge from the v4.x branch.  Fixed some error handling code
    paths dealing with uncorrectable errors in tags.
    Revision 1.62  2010/07/09 23:10:39Z  garyp
    Fixed a minor merge issue.
    Revision 1.61  2010/07/09 23:01:16Z  garyp
    Updated TagRead/Write to use the M60 ECC where appropriate.
    Added support for error injection.
    Revision 1.60  2010/07/06 04:12:38Z  garyp
    Fixed a cut-and-paste error.
    Revision 1.59  2010/07/06 02:24:18Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.58  2010/07/06 01:57:36Z  garyp
    Updated debug code.  Cleaned up some error handling code paths for
    readability.  Removed some magic numbers.  No functional changes.
    Revision 1.57  2010/06/25 07:29:41Z  garyp
    Fixed a cut-and-paste error.
    Revision 1.56  2010/06/25 03:04:52Z  garyp
    Fixed the "mixed mode ECC" feature to work properly in duel M60 chip
    configurations.  Fixed a symbol name mixup (though it was not causing
    any actual problem).
    Revision 1.55  2010/06/19 17:01:02Z  garyp
    Updated for a relocated ntmicron.h.
    Revision 1.54  2010/06/19 03:56:54Z  garyp
    Updated to support Micron M60 flash.
    Revision 1.53  2010/04/29 00:04:23Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.52  2009/12/13 01:13:49Z  garyp
    Updated to use some functions which were renamed to avoid naming conflicts.
    Updated to track corrected and uncorrected error counts over the life of
    the NTM instance.  Updated debug code.
    Revision 1.51  2009/12/03 21:51:09Z  garyp
    Refactored so that much of the hardware initialization logic is done in
    the hooks module, to allow project specific customization, such as using
    different flash timing for different chips.  Moved more configuration
    options to be specified at run-time rather than compile-time.  Updated
    to allow cache mode reads to work with Numonyx chips.
    Revision 1.50  2009/10/14 01:26:18Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.49  2009/10/07 17:34:22Z  garyp
    Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
    Revision 1.48  2009/10/03 01:03:10Z  garyp
    Changed a production message to a debug message.
    Revision 1.47  2009/08/01 02:09:48Z  garyp
    Merged from the v4.0 branch.  Updated to support reserved space specified
    as a chip boundary and Device size specified as the next chip boundary.
    Added support for FFXPARAM_FIM_CHIPID.  Updated to use the symbol
    NEW_NTM_FUNCTION_NAMES to indicate whether the new or old style NTM
    function names are being used.  Updated to work properly when two
    differently sized chips are connected to the NAND controller --  so long
    as the fundamental chip characteristics are the same, and they are used
    in two different, mutually exclusive Device definitions.  Updated to
    explicitly check for WriteEnable after issuing an unlock command.  Fixed
    to properly wait for the chip reset to complete, per the Marvell app note.
    Fixed so that all ECC'd pages have the LEGACY_WRITTEN_WITH_ECC flag set,
    even if they are written with no tags.  Implemented Micron cache-mode read
    support.  Modified so the PageWrite() functionality allows a page to be
    written with ECC, but no tags.  As before, if ECC is turned off, then tags
    may not be written.  Fixed to cleanly fail initialization if there are no
    blocks  remaining after the reserved space has been removed.  Updated to
    fully support two flash chips.  Modified to allow support for the second
    chip to be disabled at compile-time.  Now use allocate instance data, in
    the case that the OEM want to separately control the two chips (still
    requires some outside synchronization).  Added an interface to support
    power suspend/resume.  Removed the temporary code where we treated a
    timeout on write or erase as if a block had gone bad.  May have been a
    problem only on the Zylonite hardware.  Added explicit status checks after
    write and erase operations to catch failures due to accessing locked blocks.
    Added an IORequest() interface.  Added support for generalized block lock/
    unlock support.
    Revision 1.46  2009/04/17 03:58:09Z  glenns
    Fixed Bug 2615: Corrected erroneous logic in LFA processing.  
    Fixed Bug 2630: Fixed incorrect logic in setting return status when
    correctable errors occur during read operations.
    Revision 1.45  2009/04/13 16:52:21Z  glenns
    Fixed Bug 2595:  Corrected erroneous calculation of chip-select offset
    causing NTM to believe 2nd half of a single device is part of a second chip.
    Revision 1.44  2009/04/09 03:45:22Z  garyp
    Renamed a helper function to avoid namespace collisions.  Updated
    for AutoDoc.
    Revision 1.43  2009/04/08 20:48:35Z  garyp
    Renamed the public functions so they fit smoothly into the automated
    documentation system.  No other functional changes.
    Revision 1.42  2009/04/02 17:49:28Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.41  2009/03/25 19:55:29Z  glenns
    Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.40  2009/03/22 21:45:18Z  keithg
    Merged changes from 3.4 branch.  v1.22.1.7 Fixed DFCShutdown() to call
    FfxHookNTPxa320Destroy() and avoid a potential memory leak; and 1.22.1.14
    Fixed to properly wait for the chip reset to complete, per the Marvell app
    note.
    Revision 1.39  2009/03/20 22:27:29Z  glenns
    Fix Bugzilla #2514: Modify DFCWritePageECC to appropriately set return error
    codes and break out of the page count loop if an error is detected.
    Revision 1.38  2009/03/18 19:27:24Z  thomd
    Add blockwise support for Large Flash Addressing
    Revision 1.37  2009/03/18 03:02:17Z  glenns
    Fix Bugzilla #2370: Remove assignment of obsolete block status value.
    Revision 1.36  2009/03/04 07:02:49Z  glenns
    Fix Bugzilla #2393: Removed all reserved block/reserved page processing
    from the NTM. This is now handled by the device manager.
    Revision 1.35  2009/03/02 22:12:14Z  billr
    Resolve bug 2451: PXA320 NTM returns incorrect status for timeout.  Remove
    some dead code.
    Revision 1.34  2009/02/26 20:40:46Z  keithg
    Added DFCREG_ALL_ONES with the appropriate sized value to placate compiler
    warnings of signed/unsigned mismatch.
    Revision 1.33  2009/02/23 22:00:56Z  glenns
    Fix compiler warning about variable initialized but never used.
    Revision 1.32  2009/02/17 08:02:09Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.31  2009/02/06 01:55:47Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.30  2009/01/27 12:30:13  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.29  2009/01/27 03:41:26Z  glenns
    Modified to accomodate variable names changed to meet Datalight coding
    standards.  Updated PageRead to accomodate FFXSTAT_FIMCORRECTABLEDATA
    status.  Added call to encode number of corrected bits in the FFXIOSTATUS
    structure for PageRead.
    Revision 1.28  2009/01/23 17:46:57Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.27  2009/01/18 08:52:44Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.26  2009/01/16 23:59:00Z  glenns
    Fixed up literal FFXIOSTATUS initialization in five places.
    Revision 1.25  2008/12/12 07:56:34Z  keithg
    Updated to conditionally use the deprecated xxx_BBMBLOCK type.
    Revision 1.24  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires that BBM functionality
    is compiled in.
    Revision 1.23  2008/06/16 16:55:48Z  thomd
    Renamed ChipClass field to match higher levels.  Propagate chip capability
    fields in Create routine.
    Revision 1.22  2008/05/08 22:26:07Z  garyp
    Implemented cache-mode (for writes only) when using Micron chips which
    support that feature.
    Revision 1.21  2008/05/03 04:02:24Z  garyp
    Added support for 16-bit NAND.  Corrected a bug in the definition of the
    page size mask.  Updated debug code.
    Revision 1.20  2008/04/08 19:52:50Z  billr
    Declare a proper function prototype.
    Revision 1.19  2008/03/27 20:30:52Z  Garyp
    Updated to support variable length tags (for large-block NAND only).
    Implemented the raw page read/write functions.  Fixed hidden read/write
    to use raw reads/writes, rather than using ECC.  Modified so that status
    bits are explicitly cleared after they are read.  Temporarily treat some
    timeout errors as I/O errors (causing block replacement).  This is a work-
    in-progress checkin.
    Revision 1.18  2007/12/15 04:47:57Z  Garyp
    Disabled setting NCSX during DFCInitialize().  Fixed DFCConfigure() to set
    RA_START and PG_PER_BLK_64 if large block NAND is being used.  Fixed a bug
    in GetBlockStatus() where the same page was being examined twice.  Fixed a
    bug in DFCShutdown() where a statically declared structure was being freed.
    Added debug code.
    Revision 1.17  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/09/26 23:48:04Z  pauli
    Resolved Bug 355: Updated to report a required alignment of 1.
    Resolved Bug 1476.
    Revision 1.15  2007/09/21 04:35:48Z  pauli
    Added the ability to report that BBM should or should not be used
    based on the characteristics of the NAND part identified.
    Revision 1.14  2007/08/02 23:03:22Z  timothyj
    Changed units of reserved space and maximum size to be in terms
    of KB instead of bytes.
    Revision 1.13  2007/06/17 00:33:22Z  Garyp
    Resolved Bug 1233 where the factory bad block marks would be trashed
    when writing pages with ECC and tags.  Cleaned up the function header
    documentation.
    Revision 1.12  2007/06/01 19:10:13Z  rickc
    Properly setup NAND controller if using large-page NAND.  Removed NAND
    controller timing; assuming these values are set by the BSP.
    Revision 1.11  2007/05/29 17:48:20Z  jeremys
    Bug 355 - FFXCONF_FORCEALIGNEDIO doesn't align reads.  This NTM has code
    that handles it anyways, but an assert prevented the handler code from
    executing.  Commented out that assert.
    Revision 1.10  2007/04/10 19:42:05Z  timothyj
    Removed assignment in expression and changed D_UCHAR to D_CHAR in debug
    output, to fix compiler warnings for build #441 of the RVDS 3.0 tools.
    Revision 1.9  2007/04/07 03:28:50Z  Garyp
    Documentation updated.
    Revision 1.8  2007/03/30 23:49:32Z  timothyj
    Removed extra computation of array total size.
    Revision 1.7  2007/03/21 22:05:07Z  rickc
    Corrected and commented out assert to work with NAND FIM.
    Revision 1.6  2007/03/20 22:06:48Z  rickc
    Fixed ReadID to poll for read ready, not command ready.
    Revision 1.5  2007/03/14 21:13:35Z  rickc
    Removed create and destroy hooks prototypes.
    Revision 1.4  2007/03/14 18:08:52Z  rickc
    Fixed ntminfo structure initialization.  Fixed compile warning.
    Revision 1.3  2007/03/13 19:08:29Z  rickc
    Removed compile warning.
    Revision 1.2  2007/03/13 00:41:59Z  rickc
    Updated NTMINFO structure initialization.
    Revision 1.1  2007/03/12 22:57:04Z  rickc
    Initial revision
    Revision 1.5  2007/02/13 23:49:55Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.4  2007/02/06 20:43:42Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.3  2007/01/04 00:18:23Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was
    located.  Removed FFXNANDMFG (replaced references with references to the
    constant FFXNANDCHIP returned, above).
    Revision 1.2  2006/11/08 18:23:02Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.1  2006/11/03 00:07:40Z  billr
    Initial revision
    ---------------------
    Bill Roman 2006-10-20
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#define NEW_NTM_FUNCTION_NAMES          TRUE

#include <deverr.h>
#include <fxiosys.h>
#include <fxdevapi.h>
#include <fxnandapi.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include <nandid.h>
#include "nand.h"
#include "ntm.h"
#include "errmanager.h"
#include <fhpxa320.h>
#include <hardware/hwpxa320.h>
 
#if FFXCONF_NANDSUPPORT_MICRON || FFXCONF_NANDSUPPORT_NUMONYX
  #include <ntmicron.h>
  #define SUPPORT_CACHE_MODE_OPS        TRUE    /* Must be TRUE for checkin */
#else
  #define SUPPORT_CACHE_MODE_OPS        FALSE   /* Must be FALSE for checkin */
#endif

#if FFXCONF_NANDSUPPORT_MICRON
  #define SUPPORT_M60_MIXED_MODE        TRUE    /* Must be TRUE for checkin */
#else  
  #define SUPPORT_M60_MIXED_MODE        FALSE   /* Must be FALSE for checkin */
#endif

#define NTMNAME "NTPXA320"


/*-------------------------------------------------------------------
    Spare Area Format

    The spare area format is constrained by the DFC's placement
    of the ECCs.  Fortunately, this is sensible:  it reserves
    the required space at the end of the spare area, leaving the
    beginning available for software use.  This avoids conflict
    with the NAND chip's documented bad block indicator.  The DFC
    uses eight bytes (actually six, rounded up to the size of its
    buffer entries) on small-block NAND, 24 bytes on large-block.

    Different spare area formats for SSFDC and OffsetZero style
    flash.

    SSFDC style has 8 bytes of user area, and only supports the
    traditional 2-byte FlashFX tags.

        +-------------+-------------+-------------+-------------+
     0  |   Resv (1)  |   Resv (1)  |   TagECC    |   TagCheck  |
        +-------------+-------------+-------------+-------------+
     4  | PageStat(3) |   FBB (2)   |    Tag1     |     Tag2    |
        +-------------+-------------+-------------+-------------+

    OffsetZero style has 40 bytes of user area, and supports
    variable length tags, up to 15 bytes in length (longer tags
    could be supported).

        +-------------+-------------+-------------+-------------+
     0  |   FBB (2)   |   FBB (2)   |   TagECC    |   TagCheck  |
        +-------------+-------------+-------------+-------------+
     4  | PageStat(3) |   Resv (1)  |  TagLen (4) |     Tag1    |
        +-------------+-------------+-------------+-------------+
     8  |    Tag2     |    Tag3     |    Tag4     |     Tag5    |
        +-------------+-------------+-------------+-------------+
    12  |    Tag6     |    Tag7     |    Tag8     |     Tag9    |
        +-------------+-------------+-------------+-------------+
    16  |   Tag10     |   Tag11     |   Tag12     |    Tag13    |
        +-------------+-------------+-------------+-------------+
    20  |   Tag14     |   Tag15     |   Unused    |    Unused   |
        +-------------+-------------+-------------+-------------+
    ...
        +-------------+-------------+-------------+-------------+
    36  |   Unused    |   Unused    |   Unused    |    Unused   |
        +-------------+-------------+-------------+-------------+

    1)  (Resv) Reserved bytes which are never written by the NTM.
    2)  (FBB) Factory Bad Block marks.  For OffsetZero style flash,
        the first byte or word of the spare area is used for a bad
        block mark, based on whether the flash is 8 or 16-bit.
    3)  (PageStat) The page status is set to LEGACY_WRITTEN_WITH_ECC
        when a page is written with ECC enabled.
    4)  (TagLen) The TagLen field is used for variable length tags,
        and is encoded with its own ECC.  Only used by OffsetZero
        style flash.
-------------------------------------------------------------------*/

/*  Common offsets shared by SSFDC and OFFSETZERO style spare areas
*/
#define SPARE_INDEX_TAGECC              (2)
#define SPARE_INDEX_TAGCHECK            (3)
#define SPARE_INDEX_PAGESTAT            (4)

#define SSFDC_USER_SIZE                 (8)
#define SSFDC_META_SIZE                 (2) /* Max size of a tag */
#define SSFDC_SPARE_INDEX_FBB           (5)
#define SSFDC_SPARE_INDEX_TAGSTART      (6)

#define OFFSETZERO_USER_SIZE           (40)
#define OFFSETZERO_META_SIZE           (15) /* Max size of a tag */
#define OFFSETZERO_SPARE_INDEX_FBB      (0)
#define OFFSETZERO_SPARE_INDEX_TAGLEN   (6)
#define OFFSETZERO_SPARE_INDEX_TAGSTART (7)

/*  Modified spare area for 4-bit ECC Micron M60A 
*/ 
#define M60A_SPARE_INDEX_TAGLEN         (5)
#define M60A_SPARE_INDEX_TAGSTART       (6)

/*  Defines for enable/disable of 4-bit ECC
*/
#define ENABLE  TRUE
#define DISABLE FALSE

/*  Values used by EncodeTag() and DecodeTag()
*/
#define EXTRABYTES  (2)
#define OFFSETECC   (0)
#define OFFSETCHK   (1)
#define OFFSETTAG   (2)

/*  Timeout in msec. for NAND flash operations.  The longest operation is
    erase, with typical spec of 2 msec.  It's okay if this is generous,
    timeout generally means something is badly wrong, and wasting a few
    msec. doesn't matter at that point.
*/
#define NAND_TIMEOUT (500)


/*  Shorthand notation for readability and to reduce
    diffs from previous revisions.
*/
#define NDCR                    PXA3XX_NDCR(pNFC->Params.pulPXA320Base)
#define NDSR                    PXA3XX_NDSR(pNFC->Params.pulPXA320Base)
#define NDDB                    PXA3XX_NDDB(pNFC->Params.pulPXA320Base)
#define NDCB0                   PXA3XX_NDCB0(pNFC->Params.pulPXA320Base)
#define NDCB1                   PXA3XX_NDCB1(pNFC->Params.pulPXA320Base)
#define NDCB2                   PXA3XX_NDCB2(pNFC->Params.pulPXA320Base)
#define NDTR0CS0                PXA3XX_NDTR0CS0(pNFC->Params.pulPXA320Base)
#define NDTR1CS0                PXA3XX_NDTR1CS0(pNFC->Params.pulPXA320Base)

#define SPARE_EN                PXA3XX_NDCR_SPARE_EN            
#define ECC_EN                  PXA3XX_NDCR_ECC_EN              
#define ND_RUN                  PXA3XX_NDCR_ND_RUN              
#define RD_ID_CNT(c)            PXA3XX_NDCR_RD_ID_CNT(c) 
#define ND_ARB_EN               PXA3XX_NDCR_ND_ARB_EN 
#define INTERRUPT_MASK_ALL      PXA3XX_NDCR_INTERRUPT_MASK_ALL 

#define RDY                     PXA3XX_NDSR_RDY      
#define CS0_CMDD                PXA3XX_NDSR_CS0_CMDD 
#define CS1_CMDD                PXA3XX_NDSR_CS1_CMDD 
#define CS0_BBD                 PXA3XX_NDSR_CS0_BBD  
#define CS1_BBD                 PXA3XX_NDSR_CS1_BBD  
#define DBERR                   PXA3XX_NDSR_DBERR    
#define SBERR                   PXA3XX_NDSR_SBERR    
#define WRDREQ                  PXA3XX_NDSR_WRDREQ   
#define RDDREQ                  PXA3XX_NDSR_RDDREQ   
#define WRCMDREQ                PXA3XX_NDSR_WRCMDREQ 

#define AUTO_RS                 PXA3XX_NDCB_AUTO_RS             
#define CSEL                    PXA3XX_NDCB_CSEL                
#define CMD_TYPE_READ           PXA3XX_NDCB_CMD_TYPE_READ       
#define CMD_TYPE_PROGRAM        PXA3XX_NDCB_CMD_TYPE_PROGRAM    
#define CMD_TYPE_ERASE          PXA3XX_NDCB_CMD_TYPE_ERASE      
#define CMD_TYPE_READ_ID        PXA3XX_NDCB_CMD_TYPE_READ_ID    
#define CMD_TYPE_STATUS_READ    PXA3XX_NDCB_CMD_TYPE_STATUS_READ
#define CMD_TYPE_RESET          PXA3XX_NDCB_CMD_TYPE_RESET      
#define NC                      PXA3XX_NDCB_NC                  
#define DBC                     PXA3XX_NDCB_DBC                 
#define ADDR_CYC(n)             PXA3XX_NDCB_ADDR_CYC(n)         
#define CMD2(cmd)               PXA3XX_NDCB_CMD2(cmd)           
#define CMD1(cmd)               PXA3XX_NDCB_CMD1(cmd)  
 
#define BLOCK_FROM_PAGE(ulPage, pNTM) ((D_UINT16)((ulPage * pNTM->NtmInfo.uPageSize) / pNTM->NtmInfo.ulBlockSize))

/*  State information about the NAND Flash Controller (NFC).
*/
typedef struct
{
    void           *pSpareBuf;      /* One spare buffer, allocated at init time */
    unsigned        nChips;         /* number of chips found */
    unsigned        nChipStart;     /* Starting chip select number */
    unsigned        uAddrCycRW;     /* ADDR_CYC field of ND for read/write */
    unsigned        uAddrCycErase;  /*  or erase, shifted into position */
    unsigned        uPageShift;     /* number of address bits in offset in page */
    unsigned        uPageSize;      /* bytes in one page (main only) */
    unsigned        uSpareSize;     /* bytes in whole spare area */
    unsigned        uUserSize;      /* bytes in user portion of spare area */
    unsigned        nOffsetTagStart;
    unsigned        nMaxChipSelect;
    unsigned        fReadConfirm        : 1;
    unsigned        fResetBeforeProgram : 1;
    unsigned        fM60EDCAllowed      : 1;
  #if SUPPORT_M60_MIXED_MODE
    unsigned        fUseM60ECC          : 1;
  #endif
  #if SUPPORT_CACHE_MODE_OPS
    unsigned        fUseCacheModeReads  : 1;
    unsigned        fUseCacheModeWrites : 1;
  #endif
    D_UINT32        ulCorrectedErrors;
    D_UINT32        ulUncorrectableErrors;
  #if FFXCONF_POWERSUSPENDRESUME
    D_UINT32        ulSavedNDCR;
    D_UINT32        ulSavedNDTR0CS0;
    D_UINT32        ulSavedNDTR1CS0;
    unsigned        nSuspended;
  #endif
    NTMHOOKHANDLE   hHook;
    FFXPXA320PARAMS Params;
} NFC;

/*  Private data structure for this NTM.
*/
struct tagNTMDATA
{
    FFXDEVHANDLE        hDev;
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE        hEI;
  #endif
    NTMINFO             NtmInfo;        /* information visible to upper layers */
    const FFXNANDCHIP  *pChipInfo;      /* chip information                    */
    unsigned            uPageShift;     /* bits to >> offset to get page index */
    unsigned            uChipShift;     /* bits to >> offset to get chip number */
    D_UINT32            ulPageMask;     /* mask for valid bits in page index */
    NFC                *pNFC;
    D_BYTE              abID[NAND_ID_SIZE];
};


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_pxa320 =
{
    FfxNtmCreate,
    FfxNtmDestroy,
    FfxNtmPageRead,
    FfxNtmPageWrite,
    FfxNtmTagRead,
    FfxNtmTagWrite,
    FfxNtmSpareRead,
    FfxNtmSpareWrite,
    FfxNtmBlockErase,
    FfxNtmGetPageStatus,
    FfxNtmGetBlockStatus,
    FfxNtmSetBlockStatus,
    FfxNtmParameterGet,
    FfxNtmParameterSet,
    FfxNtmRawPageRead,
    FfxNtmRawPageWrite,
    FfxNtmIORequest
};

static NFC *        DFCInitialize(FFXDEVHANDLE hDev);
static FFXSTATUS    DFCConfigure(     NFC *pNFC, const FFXNANDCHIP *pChipInfo, D_BYTE *pabID);
static void         DFCShutdown(      NFC *pNFC);
static D_BOOL       DFCResetChip(     NFC *pNFC, unsigned nChip);
static D_UINT32     DFCWaitForStatus( NFC *pNFC, D_UINT32 ulStatus);
static FFXSTATUS    DFCWaitForNDRUN(  NFC *pNFC);
static D_UINT8      DFCReadStatus(    NFC *pNFC, unsigned nChip);
static D_BOOL       DFCReadID(        NFC *pNFC, unsigned nChip, D_UCHAR aucID[NAND_ID_SIZE]);
static FFXIOSTATUS  DFCPageReadECC(   NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount, void *pMain, void *pTags, unsigned nTagSize);
static FFXIOSTATUS  DFCPageReadRaw(   NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount, void *pMain, void *pSpare);
static FFXIOSTATUS  DFCPageWriteECC(  NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount, const void *pMain, const void *pTags, unsigned nTagSize);
static FFXIOSTATUS  DFCPageWriteRaw(  NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount, const void *pMain, const void *pSpare);
static FFXIOSTATUS  DFCBlockErase(    NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount);
static FFXSTATUS    DFCControl4BitECC(NFC *pNFC, unsigned nChip, D_BOOL fEnable);
static void         EncodeTag(        NFC *pNFC, const D_BUFFER *pTag, D_BUFFER *pSpare, unsigned nTagSize);
static D_BOOL       DecodeTag(        NFC *pNFC, D_BUFFER *pTag, const D_BUFFER *pSpare, unsigned nTagSize);
static void         DataRead(         NFC *pNFC, void *pData, unsigned nCount);
static void         DataDiscard(      NFC *pNFC, unsigned nCount);
static void         DataWrite(        NFC *pNFC, const void *pData, unsigned nCount);
static void         DataFill(         NFC *pNFC, unsigned nCount);
static unsigned     IdentifyFlash(NTMDATA *pNTM, FFXFIMBOUNDS *pBounds, D_UINT32 *pulChip0Blocks);
static D_UINT8      EncodeTagLength(unsigned nTagLen);
static unsigned     DecodeTagLength(D_UINT8 bEncodedTagLen);
static D_UINT8      Checksum(const D_BUFFER *pData, unsigned nLen);
static void         MitigateDFCBug(NTMHANDLE hNTM, unsigned nChip, D_UINT32 ulPageOnChip, const FFXIOSTATUS *pIOStat);

#if FFXCONF_NANDSUPPORT_MICRON
static D_UINT8      DFCMicronLockStatus(    NFC *pNFC, unsigned nChip, D_UINT32 ulPage);
static D_BOOL       DFCMicronLockTight(     NFC *pNFC, unsigned nChip);
static D_BOOL       DFCMicronLockBlocks(    NFC *pNFC, unsigned nChip);
static D_BOOL       DFCMicronUnlockBlocks(  NFC *pNFC, unsigned nChip, D_UINT32 ulPageStart, D_UINT32 ulPageEnd, unsigned fInvert);
#endif
#if SUPPORT_CACHE_MODE_OPS
static FFXIOSTATUS  DFCPageReadECCCacheMode(NFC *pNFC, unsigned nChip, D_UINT32 ulPage, unsigned nCount, void *pMain, void *pTags, unsigned nTagSize);
#endif


#if D_DEBUG
/*  Debug helper functions to symbolically dump the state
    of the registers.
*/
static void DFCDump(volatile D_UINT32 *pulBase);
static void DFCDumpNDCR(volatile D_UINT32 *pulBase);
static void DFCDumpNDSR(volatile D_UINT32 *pulBase);
static void DFCDumpNDCB0(volatile D_UINT32 *pulBase);
static void DFCDumpNDTR0CS0(volatile D_UINT32 *pulBase);
static void DFCDumpNDTR1CS0(volatile D_UINT32 *pulBase);
#endif

/*  Because we may have separate NTM instantiations when there is only one
    physical NAND controller, we must take precautions to ensure that a 
    subsequent attempt to initialize an NTM instance does not trash the
    NAND controller for other instances.  Therefore use this global as the
    gating flag.
*/    
static D_ATOMIC32 ulAtomicControllerInitCount = 0;


        /*-----------------------------------------------------*\
         *                                                     *
         *                 External Interface                  *
         *                                                     *
        \*-----------------------------------------------------*/


/*-------------------------------------------------------------------
    Public: FfxNtmCreate()

    Create an NTM instance.  Additionally, it configures the
    corresponding hardware by calling the helper functions and
    project hooks.

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
static NTMHANDLE FfxNtmCreate(
    FFXDEVHANDLE        hDev,
    const NTMINFO     **ppNtmInfo)
{
    NTMDATA            *pNTM = NULL;
    D_UINT16            uPageShiftValue, uPageSizeTemporary;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmCreate() hDev=%P\n", hDev));

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(pNTM)
    {
        pNTM->hDev = hDev;

        pNTM->pNFC = DFCInitialize(hDev);
        if(pNTM->pNFC)
        {
            FFXFIMBOUNDS    bounds;
            D_UINT32        ulChip0Blocks;

            pNTM->pNFC->nMaxChipSelect = (pNTM->pNFC->Params.fSupportCS1 ? 1 : 0);

            /*  Get the array bounds now so we know how far to scan
            */
            FfxDevGetArrayBounds(hDev, &bounds);

            pNTM->pNFC->nChips = IdentifyFlash(pNTM, &bounds, &ulChip0Blocks);
            if(pNTM->pNFC->nChips)
            {
                D_UINT32    ulTotalBlocks;

                ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                /*  Fill in the NTMINFO appropriate to the chip that was found.
                */
                pNTM->NtmInfo.uDeviceFlags  |= DEV_REQUIRES_ECC;
                if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
                if (pNTM->pChipInfo->pChipClass->fProgramOnce)
                    pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

                pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;
                pNTM->NtmInfo.ulBlockSize   = pNTM->pChipInfo->pChipClass->ulBlockSize;
                pNTM->NtmInfo.uPageSize     = pNTM->pChipInfo->pChipClass->uPageSize;
                pNTM->NtmInfo.uSpareSize    = pNTM->pChipInfo->pChipClass->uSpareSize;
                pNTM->NtmInfo.uAlignSize    = 1;  /* arbitrary buffer alignment allowed when reading/writing with this NTM */

                if((pNTM->pChipInfo->pChipClass->uEdcRequirement > 1 ) &&
                   (pNTM->pChipInfo->pChipClass->uEdcRequirement > pNTM->pChipInfo->pChipClass->uEdcCapability))
                {
                    /* This chip requires more correction capabilities
                       than this NTM or the underlying chip can handle-
                       fail the create.
                    */
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                                    NTMNAME"-Create() Insufficient EDC capabilities.\n"));
                    goto Create_Fail;
                }

                /*  The following lines of code set  the NTM EDC capability and
                    segment size based on the PXA controller features. If other
                    EDC is implemented in this NTM or its hooks, this code will
                    need to be updated. These values may be overridden later
                    in this Create procedure if the Micron M60 device is in
                    use.
                */
                pNTM->NtmInfo.uEdcSegmentSize     = 512;
                pNTM->NtmInfo.uEdcCapability      = 1;
                
                pNTM->NtmInfo.uEdcRequirement     = pNTM->pChipInfo->pChipClass->uEdcRequirement;
                pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
                pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

                if(pNTM->NtmInfo.uPageSize == 512)
                {
                    DclAssert((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_SSFDC);
                    pNTM->NtmInfo.uMetaSize     = SSFDC_META_SIZE;
                    pNTM->pNFC->nOffsetTagStart = SSFDC_SPARE_INDEX_TAGSTART;
                }
                else if(pNTM->NtmInfo.uPageSize == 2048)
                {
                    DclAssert((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) == CHIPFBB_OFFSETZERO);
                        
                    if(pNTM->pNFC->fM60EDCAllowed)
                    {
                        pNTM->NtmInfo.uEdcCapability = 4;
                        pNTM->NtmInfo.uMetaSize     = FFX_NAND_TAGSIZE;
                        pNTM->pNFC->nOffsetTagStart = M60A_SPARE_INDEX_TAGSTART;
                    }
                    else
                    {
                        pNTM->NtmInfo.uMetaSize     = OFFSETZERO_META_SIZE;
                        pNTM->NtmInfo.uDeviceFlags |= DEV_VARIABLE_LENGTH_TAGS;
                        pNTM->pNFC->nOffsetTagStart = OFFSETZERO_SPARE_INDEX_TAGSTART;
                    }
                }
                else
                {
                    DclError();
                }

                /*  Calculate this once and store it for use later for
                    both optimization and simplification of the code
                */
                DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
                DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
                pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);
                pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

                /*  Values used internally in the NTM for address calculations.
                    These shifts are preadjusted to match uPageSize
                */
                uPageSizeTemporary = pNTM->NtmInfo.uPageSize;
                uPageShiftValue = 0;
                while(uPageSizeTemporary)
                {
                    uPageSizeTemporary >>= 1;
                    uPageShiftValue++;
                }
                pNTM->uPageShift = pNTM->pChipInfo->pChipClass->uLinearPageAddrMSB;
                pNTM->uChipShift = pNTM->pChipInfo->pChipClass->uLinearChipAddrMSB + 1 - uPageShiftValue;
                pNTM->ulPageMask = (1UL << pNTM->uChipShift) - 1;

              #if FFXCONF_NANDSUPPORT_MICRON
                /*  Just because the given chip supports locking, that
                    does not mean the platform has enabled that feature.
                    Read the lock status to determine if the feature is
                    enabled.
                */
                if((pNTM->abID[0] == NAND_MFG_MICRON) &&
                    (pNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE))
                {
                    D_UINT8 bStatus;

                    bStatus = DFCMicronLockStatus(pNTM->pNFC, pNTM->pNFC->nChipStart, 0);

                    if(bStatus == 0 || bStatus == 0xFF)
                    {
                        /*  The feature is not enabled, or some sort of
                            error occurred -- turn off the "lockable"
                            capabilities flag.
                        */
                        pNTM->NtmInfo.uDeviceFlags &= (D_UINT16)(~DEV_LOCKABLE);

                        FFXPRINTF(1, (NTMNAME"-Micron locking capabilities are disabled\n"));
                    }
                    else
                    {
                        /*  We queried the lock status successfully, so
                            turn on the lock characteristics flags which
                            are standard for Micron chips.
                        */
                        pNTM->NtmInfo.nLockFlags = (FFXLOCKFLAGS_LOCKALLBLOCKS |
                                                    FFXLOCKFLAGS_LOCKFREEZE |
                                                    FFXLOCKFLAGS_UNLOCKINVERTEDRANGE);
                    }
                }
              #endif

              #if FFXCONF_ERRORINJECTIONTESTS
                pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
              #endif

                /*  Successfully created the NTM instance.
                */
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                                NTMNAME"-FfxNtmCreate() returning %P\n", pNTM));

                *ppNtmInfo = &pNTM->NtmInfo;

                return pNTM;
            }

          Create_Fail:

            DFCShutdown(pNTM->pNFC);
            pNTM->pNFC = NULL;
        }
        DclMemFree(pNTM);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmCreate() failed\n"));

    return NULL;
}


/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Read the ID information from the NAND chips, and configure the
    controller hardware.

    If there are two chips which are defined such that the Device
    definition includes part or all of both chips, they must be
    identical.

    The only case where two dissimilar chips are allowed is if the
    Device definition is such that one of the chips is ignored.  For
    example, if DEV0 uses the first chip and has a hard limit which
    ends at or before the end of the first chip, then any subsequent
    chip will be ignored.  Conversely, if a Device definition has
    large enough low reserved space such that the first chip is
    skipped, then it will be ignored, and only the second chip will
    be used.

    Note also that dissimilar chips are only supported so long as
    they have identical characteristics from the standpoint of how
    NDCR is programmed -- there's only one of those, so it must be
    programmed the same, even if two different chips are being used.
    This still allows chips with a different total number of blocks
    to be used, but you can't mix-and-match 8/16bit chips, large/
    small block chips, etc.  

    Likewise the timing characteristics must be compatible, and 
    since there is only one set of timing registers to handle both
    chips, if they have different timing requirements the slower
    ones must be used.  This could be problematic in some scenarios
    since the Hooks function FfxHookNTPxa320Configure() is only
    called when the first chip is initialized.

    To accomplish this, this code must very carefully preserve the
    state of NDCR while probing for chips, otherwise the controller
    state will be messed up for other instances of this NTM.

    This code does not support the scenario where there is a chip
    on CS1, but no chip at all on CS0.

    Parameters:
        pNTM           - A pointer to the NTMDATA structure.
        pBounds        - A pointer to the FFXFIMBOUNDS structure.
        pulChip0Blocks - A variable to receive the number of blocks
                         in Chip0, IFF that chip is entirely set
                         aside as reserved space.

    Return Value:
        The number of chips (0, 1, or 2) detected.
-------------------------------------------------------------------*/
static unsigned IdentifyFlash(
    NTMDATA        *pNTM,
    FFXFIMBOUNDS   *pBounds,
    D_UINT32       *pulChip0Blocks)
{
    unsigned        nChips = 0;
    D_UINT32        ulChip0KB;

    DclAssert(pNTM);
    DclAssert(pBounds);
    DclAssert(pulChip0Blocks);

    /*  Try to read the manufacturer and device IDs from the first
        chip, decode the ID, and configure for the first chip.  Note
        that even if we only intend to use CS1, there must still be
        a chip attached to CS0.
    */
    if(!DFCReadID(pNTM->pNFC, 0, pNTM->abID))
        return 0;

    pNTM->pChipInfo = FfxNandDecodeID(pNTM->abID);
    if(!pNTM->pChipInfo)
        return 0;

    nChips++;

    /*  Save this off now, but may be zero'd out later if Chip0 is <not>
        entirely reserved space.
    */
    *pulChip0Blocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;

    /*  Determine the CS0 chip size in KB
    */
    ulChip0KB = *pulChip0Blocks * (pNTM->pChipInfo->pChipClass->ulBlockSize / 1024);

    if(FFX_ISDEVCHIP(pBounds->ulReservedBottomKB) || pBounds->ulReservedBottomKB >= ulChip0KB)
    {
        /*  We're here if the low reserved space matches or exceeds
            the size of the chip on CS0.  To move forward with the
            initialization, CS1 support must be enabled, and there
            must be a chip on CS1.
        */

        if(!pNTM->pNFC->Params.fSupportCS1)
        {
            FFXPRINTF(1, (NTMNAME":IdentifyFlash() CS0 is reserved and CS1 is disabled\n"));
            return 0;
        }

        if(!DFCReadID(pNTM->pNFC, 1, pNTM->abID))
            return 0;

        pNTM->pChipInfo = FfxNandDecodeID(pNTM->abID);
        if(!pNTM->pChipInfo)
            return 0;

        /*  Adjust the low reserved space to completely eliminate that
            which was removed due to ignoring CS0.
        */
        if(FFX_ISDEVCHIP(pBounds->ulReservedBottomKB))
        {
            /*  Can <only> be Chip0 on this hardware
            */
            DclAssert(FFX_GETDEVCHIP(pBounds->ulReservedBottomKB) == 0);
            pBounds->ulReservedBottomKB = 0;
        }
        else
        {
            pBounds->ulReservedBottomKB -= ulChip0KB;
        }

        /*  In chip select calculations, we're going to start with CS1
        */
        pNTM->pNFC->nChipStart = 1;
    }
    else
    {
        /*  The low reserved space does not fully encompass CS0, so if we
            find a chip on CS1, it <must> be the exact same chip as that
            on CS0.  However, if the max array size is defined such that
            it only uses the chip on CS0, ignore CS1 entirely.
        */

        DclAssert(!FFX_ISDEVCHIP(pBounds->ulReservedBottomKB));

        /*  Field is not used in this case, and <must> be zero'd
        */
        *pulChip0Blocks = 0;

        if(pNTM->pNFC->Params.fSupportCS1)
        {
            D_BYTE  abID[NAND_ID_SIZE];

            if( (pBounds->ulMaxArraySizeKB != FFX_CHIP_REMAINING) &&
                ((pBounds->ulMaxArraySizeKB == FFX_REMAINING) ||
                (pBounds->ulReservedBottomKB + pBounds->ulMaxArraySizeKB > ulChip0KB)) &&
                DFCReadID(pNTM->pNFC, 1, abID) )
            {
                const FFXNANDCHIP  *pCS1ChipInfo;

                /*  Decode the ID for the sole purpose of making sure it
                    exactly matches that of the first chip.
                */
                pCS1ChipInfo = FfxNandDecodeID(abID);
                if(pCS1ChipInfo)
                {
                    if(pCS1ChipInfo != pNTM->pChipInfo)
                    {
                        FFXPRINTF(1, (NTMNAME":IdentifyFlash() CS0 and CS1 use chips with different\n"));
                        FFXPRINTF(1, (NTMNAME":                geometries -- ignoring CS1\n"));
                    }
                    else
                    {
                        /*  Found two matching chips, and we have been
                            configured such that this single FIM instance
                            will create a Device that spans them.
                        */
                        nChips++;
                    }
                }
            }
        }
    }

    /*  At this point we have either one chip, two chips which are
        identical, or two different chips where the reserved space
        is configured such that only one of the chips is actually
        used.
    */
    if(DFCConfigure(pNTM->pNFC, pNTM->pChipInfo, pNTM->abID) == FFXSTAT_SUCCESS)
    {
        if(pNTM->pChipInfo->pChipClass->fBlockLockSupport)
            pNTM->NtmInfo.uDeviceFlags |= DEV_LOCKABLE;

      #if FFXCONF_NANDSUPPORT_MICRON
        if((pNTM->abID[0] == NAND_MFG_MICRON) &&
            pNTM->pNFC->Params.fSupportM60OnDieEDC &&
            ((pNTM->abID[4] & MICRONID4_ONDIE_EDC_MASK) == MICRONID4_ONDIE_EDC_4BIT) )
        {
            pNTM->pNFC->fM60EDCAllowed = TRUE;

            FFXPRINTF(1, (NTMNAME": Using Micron M60 on-die EDC\n"));
        }        
      #endif

      #if SUPPORT_CACHE_MODE_OPS
        /*  The various cache modes can only be used if the M60
            on-die EDC is not being used.
        */    
        if(pNTM->pNFC->Params.fSupportCacheMode && !pNTM->pNFC->fM60EDCAllowed)
        {
          #if FFXCONF_NANDSUPPORT_NUMONYX
            if(pNTM->abID[0] == NAND_MFG_NUMONYX)
                pNTM->pNFC->fUseCacheModeReads = TRUE;
          #endif

          #if FFXCONF_NANDSUPPORT_MICRON
            if(pNTM->abID[0] == NAND_MFG_MICRON)
            {
                pNTM->pNFC->fUseCacheModeReads = TRUE;
          
                if(pNTM->abID[2] & MICRONID2_CACHE_PROGRAM)
                {
                    FFXPRINTF(1, (NTMNAME": Using Micron cache program operations\n"));

                    pNTM->pNFC->fUseCacheModeWrites = TRUE;
                }
            }
          #endif

          #if D_DEBUG
            if(pNTM->pNFC->fUseCacheModeReads)
                FFXPRINTF(1, (NTMNAME": Using cache read operations\n"));
          #endif      
        }        

      #endif /* SUPPORT_CACHE_MODE_OPS */
    }
    else
    {
        FFXPRINTF(1, (NTMNAME": DFCConfigure() failed\n"));
        return 0;
    }

    return nChips;
}


/*-------------------------------------------------------------------
    Public: FfxNtmDestroy()

    Destroy an NTM instance, and release any allocated resources.

    Parameters:
        hNTM - The handle for the NTM instance to destroy.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FfxNtmDestroy(NTMHANDLE hNTM)
{
    DclAssert(hNTM);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-FfxNtmDestroy(%P)\n", hNTM));

    /*  Ensure that for CS0, any M60 4-bit EDC engine on the NAND is
        disabled.  This ensures that in the event that we do a warm
        restart after the driver is unloaded, that the BOOTROM on the
        PXA320 does not end up reading corrupted data, since it always
        expects the first block of NAND to be read/written with the 
        native PXA320 1-bit EDC.

        Essentially the same thing we must deal with for power suspend
        and resume.
    */                
    (void)DFCControl4BitECC(hNTM->pNFC, 0, DISABLE);

    FFXERRORINJECT_DESTROY();

    DFCShutdown(hNTM->pNFC);

    DclMemFree(hNTM);

    return;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPageRead()

    Read one or more pages of main page data, and optionally read
    the corresponding tag data from the spare area.

    The fUseEcc flag controls whether ECC-based error detection
    and correction are performed.  Note that the "raw" page read
    functionality (if implemented) is the preferred interface for
    reading without ECC protection.  If fUseEcc is FALSE, this
    function will not read tag data.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, relative to any
                      reserved space.
        pPages      - A buffer to receive the main page data
        pTags       - A buffer to receive the tag data.  Must be NULL
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
static FFXIOSTATUS FfxNtmPageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmPageRead() Start Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-FfxNtmPageRead", 0, ulCount);

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

    /*  Split the byte offset into a chip number (0 or 1) and page number
        in the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

  #if SUPPORT_M60_MIXED_MODE
    if( hNTM->pNFC->fM60EDCAllowed && 
            ((nChip > hNTM->pNFC->Params.nM60EDC4Chip)
        ||
            ((nChip == hNTM->pNFC->Params.nM60EDC4Chip) &&
            (BLOCK_FROM_PAGE(ulPageOnChip, hNTM) >= hNTM->pNFC->Params.ulM60EDC4Block))) )
    {
        hNTM->pNFC->fUseM60ECC = TRUE;
    }
    else
    {
        hNTM->pNFC->fUseM60ECC = FALSE;
    }
  #endif

    while(ulCount)
    {
        FFXIOSTATUS status;

        if(fUseEcc)
        {
            DclAssert(ulCount <= UINT_MAX);

          #if SUPPORT_CACHE_MODE_OPS
            if(hNTM->pNFC->fUseCacheModeReads && ulCount > 1)
                status = DFCPageReadECCCacheMode(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, pTags, nTagSize);
            else
          #endif
                status = DFCPageReadECC(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, pTags, nTagSize);
        }
        else
        {
            /*  It's only meaningful to read tags if ECC correction is used.
            */
            DclAssert(pTags == 0);
            DclAssert(nTagSize == 0);
            DclAssert(ulCount <= UINT_MAX);
            status = DFCPageReadRaw(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, NULL);
        }

        /*  PAGESTATUS_DATACORRECTED could be set in ulPageStatus.
        */
        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;

        /*  If correctable, encode error in ioStat:
        */
        if (ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            FfxErrMgrEncodeCorrectedBits(1, &ioStat);
        }
        else
        {
            ioStat.ffxStat = status.ffxStat;
        }
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount         -= status.ulCount;
        ulPageOnChip    += status.ulCount;
        pPages          += status.ulCount * hNTM->NtmInfo.uPageSize;
        if(nTagSize)
        {
            DclAssert(fUseEcc);
            DclAssert(pTags);
            DclAssert(nTagSize <= hNTM->NtmInfo.uMetaSize);

            pTags       += status.ulCount * nTagSize;
        }
    }

    if(fUseEcc)
        MitigateDFCBug(hNTM, nChip, ulPageOnChip - 1, &ioStat);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPageWrite()

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
static FFXIOSTATUS FfxNtmPageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmPageWrite() Start Page=%lX Count=%U TagSize=%u fUseEcc=%U\n",
                    ulStartPage, ulCount, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-FfxNtmPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

  #if SUPPORT_M60_MIXED_MODE
    if( hNTM->pNFC->fM60EDCAllowed && 
            ((nChip > hNTM->pNFC->Params.nM60EDC4Chip)
        ||
            ((nChip == hNTM->pNFC->Params.nM60EDC4Chip) &&
            (BLOCK_FROM_PAGE(ulPageOnChip, hNTM) >= hNTM->pNFC->Params.ulM60EDC4Block))) )
    {
        hNTM->pNFC->fUseM60ECC = TRUE;
    }
    else
    {
        hNTM->pNFC->fUseM60ECC = FALSE;
    }
  #endif

    while (ulCount)
    {
        FFXIOSTATUS status;

        if (fUseEcc)
        {
            DclAssert(nTagSize <= hNTM->NtmInfo.uMetaSize);
            DclAssert(ulCount <= UINT_MAX);

            status = DFCPageWriteECC(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, pTags, nTagSize);

            if(pTags)
            {
                DclAssert(nTagSize);
                pTags += status.ulCount * nTagSize;
            }
        }
        else
        {
            /*  It's only meaningful to write tags if ECC correction is used.
            */
            DclAssert(pTags == NULL);
            DclAssert(nTagSize == 0);
            DclAssert(ulCount <= UINT_MAX);
            status = DFCPageWriteRaw(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, NULL);
        }

        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXERRORINJECT_WRITE("PageWrite", ulPageOnChip, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount         -= status.ulCount;
        ulPageOnChip    += status.ulCount;
        pPages          += status.ulCount * hNTM->NtmInfo.uPageSize;
     }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmRawPageRead()

    Read an exact image of the data on the media with no processing
    of ECCs or interpretation of the contents of the spare area.

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
static FFXIOSTATUS FfxNtmRawPageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmRawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-FfxNtmRawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

    while(ulCount)
    {
        FFXIOSTATUS status;

        DclAssert(ulCount <= UINT_MAX);
        status = DFCPageReadRaw(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, pSpares);

        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXERRORINJECT_WRITE("RawPageWrite", ulPageOnChip, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was read successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount         -= status.ulCount;
        ulPageOnChip    += status.ulCount;
        pPages          += status.ulCount * hNTM->NtmInfo.uPageSize;
        if(pSpares)
            pSpares     += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmRawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmRawPageWrite()

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
static FFXIOSTATUS FfxNtmRawPageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmRawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
                    ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-FfxNtmRawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

    while (ulCount)
    {
        FFXIOSTATUS status;

        DclAssert(ulCount <= UINT_MAX);
        status = DFCPageWriteRaw(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, pPages, pSpares);

        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount         -= status.ulCount;
        ulPageOnChip    += status.ulCount;
        pPages          += status.ulCount * hNTM->NtmInfo.uPageSize;
        if (pSpares)
            pSpares     += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmRawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSpareRead()

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
static FFXIOSTATUS FfxNtmSpareRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nCount = 1; /* make this a parameter someday */
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
                    NTMNAME"-FfxNtmSpareRead() Page=%lX\n", ulPage));

    DclProfilerEnter(NTMNAME"-FfxNtmSpareRead", 0, nCount);

    DclAssert((unsigned long) pSpare % 4 == 0);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulPage >> hNTM->uChipShift);
    ulPageOnChip = ulPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

    while (nCount)
    {
        FFXIOSTATUS status;

        /*  Unfortunately, the DFC doesn't seem to have any way to transfer just
            a spare area, it always wants to transfer the main page data.
        */
        status = DFCPageReadRaw(hNTM->pNFC, nChip, ulPageOnChip, nCount, NULL, pSpare);

        ioStat.ulCount  += status.ulCount;
        ioStat.ffxStat   = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= nCount);
        nCount          -= (unsigned)status.ulCount;
        ulPageOnChip    += status.ulCount;
        pSpare          += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmSpareRead() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSpareWrite()

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
static FFXIOSTATUS FfxNtmSpareWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nCount = 1; /* make this a parameter someday */
    unsigned            nChip;
    D_UINT32            ulPageOnChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmSpareWrite() Page=%lX\n", ulPage));
    DclProfilerEnter(NTMNAME"-FfxNtmSpareWrite", 0, nCount);

    DclAssert((unsigned long) pSpare % 4 == 0);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulPage >> hNTM->uChipShift);
    ulPageOnChip = ulPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

    while (nCount)
    {
        FFXIOSTATUS status;

        /*  Unfortunately, the DFC doesn't seem to have any way to transfer just
            a spare area, it always wants to transfer the main page data.
        */
        status = DFCPageWriteRaw(hNTM->pNFC, nChip, ulPageOnChip, nCount, NULL, pSpare);

        ioStat.ulCount  += status.ulCount;
        ioStat.ffxStat   = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXERRORINJECT_WRITE("SpareWrite", ulPageOnChip, &ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The page was written successfully.  Account for it.
        */
        DclAssert(status.ulCount <= nCount);
        nCount          -= (unsigned)status.ulCount;
        ulPageOnChip    += status.ulCount;
        pSpare          += status.ulCount * hNTM->NtmInfo.uSpareSize;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmSpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmTagRead()

    Read the tag data from multiple pages.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The flash offset in pages, relative to any
                      reserved space.
        ulCount     - The number of tags to read.
        pTags       - Buffer receive the data data.
        nTagSize    - The number of bytes in each tag.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxNtmTagRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_UINT32            ulCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;
    D_BOOL              fUseM60ECC = hNTM->pNFC->fM60EDCAllowed;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmTagRead() ulStartPage=%lX ulCount=%X TagSize=%u\n",
                    ulStartPage, ulCount, nTagSize));

    DclProfilerEnter(NTMNAME"-FfxNtmTagRead", 0, ulCount);

    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

  #if SUPPORT_M60_MIXED_MODE
    if(hNTM->pNFC->fM60EDCAllowed && 
            ((nChip > hNTM->pNFC->Params.nM60EDC4Chip)
        ||
            ((nChip == hNTM->pNFC->Params.nM60EDC4Chip) &&
            (BLOCK_FROM_PAGE(ulPageOnChip, hNTM) >= hNTM->pNFC->Params.ulM60EDC4Block))) )
    {
        hNTM->pNFC->fUseM60ECC = TRUE;
    }
    else
    {
        hNTM->pNFC->fUseM60ECC = FALSE;
        fUseM60ECC = FALSE;
    }
  #endif

    while(ulCount) 
    {
        FFXIOSTATUS status;

        DclAssert(ulCount <= UINT_MAX);

        if(fUseM60ECC)
        {
            status = DFCPageReadECC(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, NULL, pTags, nTagSize);

            /*  The function never processes more than one page at a time.
            */
            DclAssert(status.ulCount <= 1);

            /*  Since the act of invalidating a tag may cause the internal
                ECC check to fail, we must allow "uncorrectable" errors to
                be ignored.  The DFCPageReadECC() function will ensure that
                the actual tag bytes are all zeros, which is the way the
                higher level software expects to be notified about bad tags.
            */    
            if(status.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
            {
                DclAssert(status.ulCount == 0);
                DclAssert((pTags[0] == 0 && pTags[1] == 0) || (pTags[0] == ERASED8 && pTags[1] == ERASED8));
                
                status.ulCount = 1;
                status.ffxStat = FFXSTAT_SUCCESS;
            }
        }
        else
        {
            status = DFCPageReadRaw(hNTM->pNFC, nChip, ulPageOnChip, (unsigned)ulCount, NULL, hNTM->pNFC->pSpareBuf);
        }

        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclAssert(status.ulCount == 1);

        if(!fUseM60ECC)
        {
            /*  Decode tag to user buffer.  Note that an uncorrectable
                or obviously invalid tag doesn't cause an error, it is
                represented as all zeroes.
            */
            if(DecodeTag(hNTM->pNFC, pTags, hNTM->pNFC->pSpareBuf, nTagSize))
            {
                ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                hNTM->pNFC->ulCorrectedErrors++;
            }
        }
        
        /*  Successful read, account for it.
        */
        DclAssert(status.ulCount <= ulCount);
        ulCount         -= (unsigned)status.ulCount;
        ulPageOnChip    += status.ulCount;
        pTags           += status.ulCount * nTagSize;
    }

    if(fUseM60ECC)
        MitigateDFCBug(hNTM, nChip, ulPageOnChip - 1, &ioStat);

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
                    NTMNAME"-FfxNtmTagRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmTagWrite()

    Write the tag data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The page offset in bytes, relative to any
                   reserved space.
        pTag     - Buffer containing the data data to write.
        nTagSize - The number of bytes in each tag.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely written.
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxNtmTagWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pTag,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nCount = 1; /* make this a parameter someday */
    unsigned            nChip;
    D_UINT32            ulPageOnChip;
    D_BOOL              fUseM60ECC = hNTM->pNFC->fM60EDCAllowed;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmTagWrite() Page=%lX pTag=%P TagSize=%u\n",
                    ulPage, pTag, nTagSize));

    DclProfilerEnter(NTMNAME"-FfxNtmTagWrite", 0, nCount);

    DclAssert(nCount);
    DclAssert(pTag);
    DclAssert(nTagSize);

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulPage >> hNTM->uChipShift);
    ulPageOnChip = ulPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

  #if SUPPORT_M60_MIXED_MODE
    if( hNTM->pNFC->fM60EDCAllowed && 
            ((nChip > hNTM->pNFC->Params.nM60EDC4Chip)
        ||
            ((nChip == hNTM->pNFC->Params.nM60EDC4Chip) &&
            (BLOCK_FROM_PAGE(ulPageOnChip, hNTM) >= hNTM->pNFC->Params.ulM60EDC4Block))) )
    {
        hNTM->pNFC->fUseM60ECC = TRUE;
    }
    else
    {
        hNTM->pNFC->fUseM60ECC = FALSE;
        fUseM60ECC = FALSE;
    }
  #endif

    while (nCount)
    {
        FFXIOSTATUS status;

        /*  Unfortunately, the DFC doesn't seem to have any way to transfer just
            a spare area, it always wants to transfer the main page data.
        */
        
        if(fUseM60ECC)
        {
            DclAssert(nTagSize == FFX_NAND_TAGSIZE);

            status = DFCPageWriteECC(hNTM->pNFC, nChip, ulPageOnChip, nCount, NULL, pTag, nTagSize);
        }
        else
        {
            DclMemSet(hNTM->pNFC->pSpareBuf, ERASED8, hNTM->pNFC->uSpareSize);
            EncodeTag(hNTM->pNFC, pTag, hNTM->pNFC->pSpareBuf, nTagSize);

            status = DFCPageWriteRaw(hNTM->pNFC, nChip, ulPageOnChip, nCount, NULL, hNTM->pNFC->pSpareBuf);
        }

        ioStat.op.ulPageStatus  |= status.op.ulPageStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXERRORINJECT_WRITE("TagWrite", ulPageOnChip, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Successful write, account for it.
        */
        DclAssert(status.ulCount <= nCount);
        nCount          -= (unsigned)status.ulCount;
        ulPageOnChip    += status.ulCount;
        pTag            += status.ulCount * nTagSize;
    }

    DclProfilerLeave(0UL);
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmTagWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmBlockErase()

    Erase a block (physical erase zone) of a NAND chip.

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
static FFXIOSTATUS FfxNtmBlockErase(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nCount = 1; /* make this a parameter someday */
    unsigned            nChip;
    D_UINT32            ulPageOnChip;
    D_UINT32            ulStartPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-FfxNtmBlockErase() Block=%lX\n", ulBlock));
    DclProfilerEnter(NTMNAME"-FfxNtmBlockErase", 0, nCount);

    DclAssert(hNTM);

    ulStartPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    /*  Split the byte offset into a chip number (0 or 1) and page number in
        the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulStartPage >> hNTM->uChipShift);
    ulPageOnChip = ulStartPage & hNTM->ulPageMask;
    DclAssert(nChip <= hNTM->pNFC->nMaxChipSelect);

    while (nCount)
    {
        FFXIOSTATUS status;

        status = DFCBlockErase(hNTM->pNFC, nChip, ulPageOnChip, nCount);

        ioStat.op.ulBlockStatus |= status.op.ulBlockStatus;
        ioStat.ulCount          += status.ulCount;
        ioStat.ffxStat           = status.ffxStat;
        ulBlock                 += status.ulCount;
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The block was erased successfully.  Account for it.
        */
        DclAssert(status.ulCount <= nCount);
        nCount          -= (unsigned)status.ulCount;
        ulPageOnChip    += status.ulCount;

        /*  In future versions this function may be called to erase multiple
            blocks, so check for crossing a chip boundary.
        */
        if ((ulPageOnChip & hNTM->ulPageMask) == 0)
        {
            DclAssert(nCount == 0 || nChip == 0);
            ulPageOnChip = 0;
            ++nChip;
        }
    }

    DclProfilerLeave(0UL);

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: BlockLockFreeze()

    This function locks a range of flash.

    Parameters:
        hNTM       - The NTM handle

    Return Value:
        Returns a standard FFXSTATUS value.
-------------------------------------------------------------------*/
static FFXSTATUS BlockLockFreeze(
    NTMHANDLE           hNTM)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-BlockLockFreeze() hNTM=%P\n", hNTM));

    DclAssert(hNTM);

  #if FFXCONF_NANDSUPPORT_MICRON
    if( (hNTM->abID[0] == NAND_MFG_MICRON) &&
        (hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE) &&
        (hNTM->NtmInfo.nLockFlags & FFXLOCKFLAGS_LOCKFREEZE) )
    {
        if(DFCMicronLockTight(hNTM->pNFC, 0))
            return FFXSTAT_SUCCESS;
        else
            return FFXSTAT_FIM_LOCKFREEZEFAILED;
    }
    else
  #endif
    {
        return FFXSTAT_UNSUPPORTEDFEATURE;
    }
}


/*-------------------------------------------------------------------
    Local: BlockLock()

    This function locks a range of flash.

    Parameters:
        hNTM       - The NTM handle
        ulBlock    - The flash offset in blocks, relative to any
                     reserved space.
        ulCount    - The number of blocks to lock.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks which were completely locked.
        The status indicates whether the operation was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS BlockLock(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-BlockLock() Block=%lX Count=%lU\n", ulBlock, ulCount));

    DclProfilerEnter(NTMNAME"-BlockLock", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;

    /*  Split the byte offset into a chip number (0 or 1) and page number
        in the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulOffset >> hNTM->uChipShift);

  #if FFXCONF_NANDSUPPORT_MICRON
    /*  Note that the Micron lock command does not support locking a range
        of blocks -- only locking the entire array (though you can lock
        a range or two using the unlock command).  For now just lock the
        entire array.
    */
    if(hNTM->abID[0] == NAND_MFG_MICRON)
    {
        if( (hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE) &&
            (hNTM->NtmInfo.nLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS) )
        {
            if(DFCMicronLockBlocks(hNTM->pNFC, nChip))
                ioStat.ulCount = ulCount;
            else
                ioStat.ffxStat = FFXSTAT_FIM_LOCKFAILED;
        }
     }

  #else

    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;

  #endif

    DclProfilerLeave(0UL);

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: BlockUnlock()

    This function unlocks a range of flash (and causes the
    remainder of flash to become locked).

    Parameters:
        hNTM       - The NTM handle
        ulBlock    - The flash offset in blocks, relative to any
                     reserved space.
        ulCount    - The number of blocks to unlock.
        fInvert    - Indicates whether the range should be inverted
                     and everything <except> the specified range is
                     unlocked.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks which were completely unlocked.
        The status indicates whether the operation was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS BlockUnlock(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock,
    D_UINT32            ulCount,
    unsigned            fInvert)
{
    FFXIOSTATUS         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);
    unsigned            nChip;
    D_UINT32            ulPageOnChip;
    D_UINT32            ulEndPage;
    D_UINT32            ulOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-BlockUnlock() Block=%lX Count=%lU Invert=%u\n",
                    ulBlock, ulCount, fInvert));

    DclProfilerEnter(NTMNAME"-BlockUnlock", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;

    /*  Split the byte offset into a chip number (0 or 1) and page number
        in the chip.  Eventually all interfaces will be in pages, so do
        everything else here in terms of the page number.
    */
    nChip = hNTM->pNFC->nChipStart + (unsigned)(ulOffset >> hNTM->uChipShift);
    ulPageOnChip = (ulOffset >> hNTM->uPageShift) & hNTM->ulPageMask;

    ulEndPage = ulPageOnChip + ((ulCount-1) * hNTM->NtmInfo.uPagesPerBlock);

  #if FFXCONF_NANDSUPPORT_MICRON
    {
        if(hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE)
        {
            if(DFCMicronUnlockBlocks(hNTM->pNFC, nChip, ulPageOnChip, ulEndPage, fInvert))
                ioStat.ulCount = ulCount;
            else
                ioStat.ffxStat = FFXSTAT_FIM_UNLOCKFAILED;
        }
    }

  #else

    ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;
  #endif

    DclProfilerLeave(0UL);
    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmGetPageStatus()

    Retrieve the page status information for the given page.

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
static FFXIOSTATUS FfxNtmGetPageStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

    DclAssert(hNTM);

    /*  Don't apply the reserved size offset here, FfxNtmSpareRead() will do it!
    */
    ioStat = FfxNtmSpareRead(hNTM, ulPage, spare.data);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        unsigned    nTagWidth = 0;

        if(hNTM->NtmInfo.uPageSize == 512)
        {
            D_BUFFER    abBuff[SSFDC_META_SIZE + EXTRABYTES];

            /*  Move the tag ECC, check byte, and tag data itself, into a
                temporary buffer, so we can properly examine the whole thing.
            */
            abBuff[OFFSETECC] = spare.data[SPARE_INDEX_TAGECC];
            abBuff[OFFSETCHK] = spare.data[SPARE_INDEX_TAGCHECK];
            DclMemCpy(&abBuff[OFFSETTAG], &spare.data[hNTM->pNFC->nOffsetTagStart], SSFDC_META_SIZE);

            /*  A fully encoded tag, including the tag ECC and check bytes,
                should never appear to be erased, or within 1 bit of being
                erased.
            */
            if(!FfxNtmHelpIsRangeErased1Bit(abBuff, SSFDC_META_SIZE + EXTRABYTES))
                nTagWidth = SSFDC_META_SIZE;
        }
        else if(hNTM->NtmInfo.uPageSize == 2048)
        {
            if(hNTM->pNFC->fM60EDCAllowed)
            {
                nTagWidth = DecodeTagLength(spare.data[M60A_SPARE_INDEX_TAGLEN]);
            }
            else
            {
                nTagWidth = DecodeTagLength(spare.data[OFFSETZERO_SPARE_INDEX_TAGLEN]);
            }
        }

        /*  Stuff in the tag width bytes.  This is done for the express
            use of BBM so it can call the page read/write and tag
            read/write calls with the correct tag length to migrate
            data from a bad block to the replacement block.  The need
            for this goes away when BBM is moved.
        */
        ioStat.op.ulPageStatus = PAGESTATUS_SET_TAG_WIDTH(nTagWidth);

        if(ISWRITTENWITHECC(spare.data[SPARE_INDEX_PAGESTAT]))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
        }
        else if(ISUNWRITTEN(spare.data[SPARE_INDEX_PAGESTAT]))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
        }
        else
        {
            ioStat.op.ulPageStatus = PAGESTATUS_UNKNOWN;   /* indecipherable */
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-FfxNtmGetPageStatus() Page=%lX returning %s\n",
                    ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmGetBlockStatus()

    Retrieve the block status information for the given erase block.

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
static FFXIOSTATUS FfxNtmGetBlockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat;
    unsigned            ii;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
    D_UINT32            ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
                    NTMNAME"-FfxNtmGetBlockStatus() Block=%lX\n", ulBlock));

    DclAssert(hNTM);

    ulPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    /*  Check the bad block indicator in the first two pages of the block
        only.
    */
    for (ii = 0; ii < 2; ii++)
    {
        int fBad;

        ioStat = FfxNtmSpareRead(hNTM, ulPage, spare.data);
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Err on the side of declaring a block factory bad: it's unfortunate
            if a block is declared bad when it's actually good, but it's worse
            if a factory bad block gets erased.  This means ignoring the
            possibility that a single-bit error could cause a good block to
            appear to be bad.
        */
        switch (hNTM->pChipInfo->bFlags & CHIPFBB_MASK)
        {
            case CHIPFBB_SSFDC:
                fBad = (spare.data[SSFDC_SPARE_INDEX_FBB] != ERASED8);
                break;

            case CHIPFBB_OFFSETZERO:
                switch (hNTM->pChipInfo->bFlags & CHIPINT_MASK)
                {
                    case CHIPINT_8BIT:
                        fBad = (spare.data[OFFSETZERO_SPARE_INDEX_FBB] != ERASED8);
                        break;

                    case CHIPINT_16BIT:
                        fBad = (spare.data[OFFSETZERO_SPARE_INDEX_FBB] != ERASED8
                            || spare.data[OFFSETZERO_SPARE_INDEX_FBB + 1] != ERASED8);
                        break;

                    default:
                        DclError();
                        fBad = TRUE;
                        break;
                }
                break;

            default:
                DclError();
                fBad = TRUE;
                break;
        }

        if (fBad)
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            break;
        }

        ulPage++;
    }

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        if((ioStat.op.ulBlockStatus & BLOCKSTATUS_FACTORYBAD) == 0)
            ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

      #if FFXCONF_NANDSUPPORT_MICRON
        if(hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE)
        {
            D_UINT8     bStatus;
            D_UINT32    ulOffset;
            D_UINT32    ulPageOnChip;
            unsigned    nChip;

            ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;

            /*  Split the byte offset into a chip number (0 or 1) and page
                number in the chip.
            */
            nChip = hNTM->pNFC->nChipStart + (unsigned)(ulOffset >> hNTM->uChipShift);
            ulPageOnChip = (ulOffset >> hNTM->uPageShift) & hNTM->ulPageMask;

            bStatus = DFCMicronLockStatus(hNTM->pNFC, nChip, ulPageOnChip);

            FFXPRINTF(2, (NTMNAME"-Micron lock status for block %lX is %02X\n", ulBlock, bStatus));

            if(bStatus != 0xFF)
            {
                bStatus &= MICRON_LOCKSTAT_MASK;

                if(bStatus & MICRON_LOCKSTAT_TIGHT_BIT)
                    ioStat.op.ulBlockStatus |= BLOCKSTATUS_DEV_LOCKFROZEN;

                if(!(bStatus & MICRON_LOCKSTAT_UNLOCKED_BIT))
                    ioStat.op.ulBlockStatus |= BLOCKSTATUS_DEV_LOCKED;
            }
        }
      #endif
    }

  #if D_DEBUG && DCLCONF_OUTPUT_ENABLED
    if (ioStat.ffxStat == FFXSTAT_SUCCESS
        && (ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != BLOCKSTATUS_NOTBAD)
    {
        D_UCHAR *pucSpare = (D_UCHAR *) spare.data;

        FFXPRINTF(1, (NTMNAME"-Bad block status %02lX at %lX contains: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                  ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE, ulBlock,
                  pucSpare[0], pucSpare[1], pucSpare[2], pucSpare[3],
                  pucSpare[4], pucSpare[5], pucSpare[6], pucSpare[7]));
    }
  #endif

    ioStat.ulFlags = IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmGetBlockStatus() Block=%lX returning %s\n",
                    ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmSetBlockStatus()

    Set the block status information for the given erase block.

    Parameters:
        hNTM          - The NTM handle to use
        ulBlock       - The flash offset, in blocks, relative to
                        any reserved space.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxNtmSetBlockStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock,
    D_UINT32        ulBlockStatus)
{
    FFXIOSTATUS     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32        ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
                    NTMNAME"-FfxNtmSetBlockStatus() Block=%lX BlockStat=%lX\n",
                    ulBlock, ulBlockStatus));

    ulPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    DclAssert(hNTM);

    /*  This should only be done for factory bad blocks during formatting.
        There is no need for any other kind of marking.
    */
    switch(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        case BLOCKSTATUS_NOTBAD:
            /*  To mark a block as good, we simply need to undo any existing
                factory bad blocks marks.  The only way to do that is to
                erase it.
            */
            ioStat = FfxNtmBlockErase(hNTM, ulBlock);
            break;

        case BLOCKSTATUS_FACTORYBAD:
        {
            DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
            FFXIOSTATUS         ioStat2;

            DclMemSet(spare.data, ERASED8, sizeof spare.data);

            switch (hNTM->pChipInfo->bFlags & CHIPFBB_MASK)
            {
                case CHIPFBB_SSFDC:
                    spare.data[SSFDC_SPARE_INDEX_FBB] = 0;
                    break;

                case CHIPFBB_OFFSETZERO:
                    spare.data[OFFSETZERO_SPARE_INDEX_FBB] = 0;
                    spare.data[OFFSETZERO_SPARE_INDEX_FBB+1] = 0;
                    break;

                default:
                    DclError();
                    break;
            }

            /*  Try to program the bad block indicator in at least one of
                the first two pages of the block.
            */

            ioStat = FfxNtmSpareWrite(hNTM, ulPage, spare.data);
            ioStat2 = FfxNtmSpareWrite(hNTM, ulPage+1, spare.data);

            /*  If at least one of the operations was successful, then
                we return the success code.
            */
            if(!IOSUCCESS(ioStat, 1))
                ioStat = ioStat2;

            break;
        }

        default:
            /*  Unexpected block status values are an error.
            */
            DclError();
            ioStat.ffxStat = FFXSTAT_BADPARAMETER;
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmSetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
                    ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmParameterGet()

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
static FFXSTATUS FfxNtmParameterGet(
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
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

        case FFXPARAM_FIM_CHIPSN:
            ffxStat = FFXSTAT_UNSUPPORTEDFEATURE;
            break;

        case FFXPARAM_FIM_CORRECTEDERRORS:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hNTM->pNFC->ulCorrectedErrors));
            }
            else
            {
                if(ulBuffLen == sizeof(hNTM->pNFC->ulCorrectedErrors))
                    DclMemCpy(pBuffer, &hNTM->pNFC->ulCorrectedErrors, sizeof(hNTM->pNFC->ulCorrectedErrors));
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        case FFXPARAM_FIM_UNCORRECTABLEERRORS:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hNTM->pNFC->ulUncorrectableErrors));
            }
            else
            {
                if(ulBuffLen == sizeof(hNTM->pNFC->ulUncorrectableErrors))
                    DclMemCpy(pBuffer, &hNTM->pNFC->ulUncorrectableErrors, sizeof(hNTM->pNFC->ulUncorrectableErrors));
                else
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
            }
            break;

        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, (NTMNAME"-FfxNtmParameterGet() unhandled parameter ID=%x\n", id));
            ffxStat = FFXSTAT_BADPARAMETER;
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmParameterSet()

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
static FFXSTATUS FfxNtmParameterSet(
    NTMHANDLE       hNTM,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
                    NTMNAME"-FfxNtmParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
                    hNTM, id, pBuffer, ulBuffLen));

  #if FFXCONF_ERRORINJECTIONTESTS
    if(id >= FFXPARAM_FIM_ERRINJECT_MIN && id <= FFXPARAM_FIM_ERRINJECT_MAX)
    {
        ffxStat = FfxDevErrorInjectParamSet(hNTM->hEI, id, pBuffer, ulBuffLen);
    }
    else
  #endif
    {
        /*  Not a parameter ID we recognize, and no place else to pass it
        */
        FFXPRINTF(1, (NTMNAME"-ParameterSet() unhandled parameter ID=%x\n", id));

        ffxStat = FFXSTAT_BADPARAMETER;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
                    NTMNAME"-FfxNtmParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmIORequest()

    Dispatch an I/O request.

    Parameters:
        hNTM - The NTM handle.
        pIOR - A pointer to the FFXIOREQUEST structure to process.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxNtmIORequest(
    NTMHANDLE           hNTM,
    FFXIOREQUEST       *pIOR)
{
    FFXIOSTATUS         ioStat = {0, FFXSTAT_FIM_UNSUPPORTEDIOREQUEST, 0, {0}};

    DclAssert(pIOR);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-FfxNtmIORequest() hNTM=%P Func=0x%x\n", hNTM, pIOR->ioFunc));

    DclProfilerEnter(NTMNAME"-FfxNtmIORequest", 0, 0);

    DclAssert(hNTM);

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_FIM_LOCK_FREEZE:
        {
            FFXIOR_FIM_LOCK_FREEZE *pReq = (FFXIOR_FIM_LOCK_FREEZE*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            ioStat.ulFlags = IOFLAGS_DEVICE;

            ioStat.ffxStat = BlockLockFreeze(hNTM);

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ulCount = 1;

            break;
        }

        case FXIOFUNC_FIM_LOCK_BLOCKS:
        {
            FFXIOR_FIM_LOCK_BLOCKS *pReq = (FFXIOR_FIM_LOCK_BLOCKS*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat = BlockLock(hNTM, pReq->ulStartBlock, pReq->ulBlockCount);

            break;
        }

        case FXIOFUNC_FIM_UNLOCK_BLOCKS:
        {
            FFXIOR_FIM_UNLOCK_BLOCKS *pReq = (FFXIOR_FIM_UNLOCK_BLOCKS*)pIOR;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat = BlockUnlock(hNTM, pReq->ulStartBlock, pReq->ulBlockCount, pReq->fInvert);

            break;
        }

      #if FFXCONF_POWERSUSPENDRESUME
        case FXIOFUNC_FIM_POWER_SUSPEND:
        {
            NFC                        *pNFC = hNTM->pNFC;
            FFXIOR_FIM_POWER_SUSPEND   *pReq = (FFXIOR_FIM_POWER_SUSPEND*)pIOR;

            (void)pReq;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_SUSPEND() PowerState=%u\n", pReq->nPowerState));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_DEVICE;

            pNFC->ulSavedNDTR0CS0 = NDTR0CS0;
            pNFC->ulSavedNDTR1CS0 = NDTR1CS0;
            pNFC->ulSavedNDCR = NDCR;

            /*  Ensure that for CS0, any M60 4-bit EDC engine on
                the NAND is disabled, so that when we come out of
                suspend, the BOOTROM on the PXA320 does not end up
                reading corrupted data, since it always expects
                the first block of NAND to be read/written with
                the native PXA320 1-bit EDC.
            */                
            (void)DFCControl4BitECC(pNFC, 0, DISABLE);

            pNFC->nSuspended++;

            /*  Better not ever wrap...
            */
            DclAssert(pNFC->nSuspended);

            ioStat.ffxStat = FFXSTAT_SUCCESS;

            break;
        }

        case FXIOFUNC_FIM_POWER_RESUME:
        {
            NFC                        *pNFC = hNTM->pNFC;
            FFXIOR_FIM_POWER_RESUME    *pReq = (FFXIOR_FIM_POWER_RESUME*)pIOR;

            (void)pReq;

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat.ulFlags = IOFLAGS_DEVICE;

            if(pNFC->nSuspended)
            {
                NDTR0CS0 = pNFC->ulSavedNDTR0CS0;
                NDTR1CS0 = pNFC->ulSavedNDTR1CS0;
                NDCR     = pNFC->ulSavedNDCR;

                /*  Blindly reset both NAND chips that might be present.
                */
                (void)DFCResetChip(pNFC, 0);
                (void)DFCResetChip(pNFC, 1);

                pNFC->nSuspended = 0;

                FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_RESUME() PowerState=%u\n", pReq->nPowerState));
            }
            else
            {
                FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_RESUME() PowerState=%u, was not suspended\n", pReq->nPowerState));
            }

            ioStat.ffxStat = FFXSTAT_SUCCESS;

            break;
        }
      #endif

        default:
        {
            FFXPRINTF(3, (NTMNAME"-FfxNtmIORequest() Bad Function %x\n", pIOR->ioFunc));

            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-FfxNtmIORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


        /*-----------------------------------------------------*\
         *                                                     *
         *           Physical Controller Interface             *
         *                                                     *
         *  Some machine dependencies (like number of bits in  *
         *  an int) are allowed, as this is machine-dependent  *
         *  code.                                              *
        \*-----------------------------------------------------*/


#if SUPPORT_CACHE_MODE_OPS

/*-------------------------------------------------------------------
    Local: DFCPageReadECCCacheMode()

    Read physical pages, using ECC on the main page data, and
    decoding the tags from the spare area.

    If an uncorrectable error occurs, the user buffer will
    contain the uncorrected data.

    Parameters:
        pNFC        - private data for this NFC instance
        nChip       - Which chip to select
        ulPage      - The page in the flash chip.
        nCount      - The number of pages to read.  The range of
                      pages must not cross an erase block boundary.
        pMain       - A pointer to the buffer for main page data.  If
                      NULL, main page data is discarded and the ECC
                      result is not reported.
        pTags       - A buffer to receive the tag data.  May be NULL
                      only if nTagSize is 0 (parameter is ignored).
        nTagSize    - The tag size to use.  If this value is 0, then
                      pTags will be ignored.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were read; this may be
        less than nCount.  The status indicates whether the read
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCPageReadECCCacheMode(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount,
    void       *pMain,
    void       *pTags,
    unsigned    nTagSize)
{
    FFXIOSTATUS ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    ulOldTR1 = D_UINT32_MAX;
    D_UINT32    ulSR;
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                    NTMNAME"-DFCPageReadECCCacheMode() Chip=%u Page=%lU Count=%u pMain=%P pTags=%P TagSize=%u\n",
                    nChip, ulPage, nCount, pMain, pTags, nTagSize));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(pMain);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Always enable ECC.
    */
    NDCR |= ECC_EN;

    /*  Always enable spare area.
    */
    NDCR |= SPARE_EN;

    /*  Clear NDSR: bits that are set are cleared by writing one to
        them, so reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for
        the selected chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
        ioStat.ulCount = 0;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(A) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        are read back from their individual addresses.  The apparent
        but overwriting of NDCB0 is not a typo, it's intentional.

        YES! This really is an ERASE command-type used below.  A little
             trickery is required to initiate cache-mode reads.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_ERASE
             | (pNFC->fReadConfirm ? DBC : 0)
             | pNFC->uAddrCycRW
             | NC
             | CMD1(CMD_READ_MAIN) | CMD2(CMD_READ_MAIN_START));
    ulCmd1 = ulPage << pNFC->uPageShift;
    ulCmd2 = ulPage >> (32 - pNFC->uPageShift);

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCPageReadECCCacheMode(B) Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    if(DFCWaitForStatus(pNFC, RDY) == 0)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
        ioStat.ulCount = 0;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(C) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    while(nCount)
    {
        FFXPRINTF(2, ("DFCPageReadECCCacheMode(D) count=%u NDSR=%lX\n", nCount, NDSR));

        /*  The controller will assert its "write command request" for
            the selected chip when it is ready for the command.
        */
        if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            ioStat.ulCount = 0;

            FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(E) error %s\n", FfxDecodeIOStatus(&ioStat)));
            break;
        }

        ulCmd0 = ((nChip ? CSEL : 0) | CMD_TYPE_READ);

        if(nCount > 1)
        {
            if(ulOldTR1 == D_UINT32_MAX)
            {
                D_UINT32    ulTR1 = NDTR1CS0;

                /*  If this is the first time through, reprogram tR to be
                    3us.  This is the only way to get cache-mode reads to
                    actually perform as they are supposed to when using
                    this NAND controller.
                */
                ulOldTR1 = ulTR1;

                ulTR1 &= 0xFFFF;
                ulTR1 |= PXA320_NSEC(3000) << 16;

                NDTR1CS0 = ulTR1;
            }

            ulCmd0 |= CMD1(MICRON_CMD_READ_CACHE_MODE) | NC;
        }
        else
        {
            /*  For the last page, reprogram Timing Register 1 back
                to its orginal values.
            */
            DclAssert(ulOldTR1 != D_UINT32_MAX);

            NDTR1CS0 = ulOldTR1;

            ulOldTR1 = D_UINT32_MAX;

            ulCmd0 |= CMD1(MICRON_CMD_READ_CACHE_MODE_LAST);
        }

        ulCmd1 = 0;
        ulCmd2 = 0;

        NDCB0 = ulCmd0;
        NDCB0 = ulCmd1;
        NDCB0 = ulCmd2;

      #if D_DEBUG
        if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
        {
            FFXPRINTF(1, ("DFCPageReadECCCacheMode(F) Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
                ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
        }
      #endif

        ulSR = DFCWaitForStatus(pNFC, RDDREQ | DBERR);
        if(!ulSR)
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;
            ioStat.ulCount = 0;

            FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(G) error %s\n", FfxDecodeIOStatus(&ioStat)));
            break;
        }

        /*  Always read the data out of the DFC.
        */
        DataRead(pNFC, pMain, pNFC->uPageSize);

        /*  If tags are desired, get the user space portion of the spare
            area out of the DFC buffer and decode its contents.
        */
        if((nTagSize) || (ulSR & DBERR) || (NDSR & SBERR))
        {
            /*  Get the spare area data out of the chip only if we are
                certain we need it.
            */
            DataRead(pNFC, pNFC->pSpareBuf, pNFC->uUserSize);

            if(nTagSize)
            {
                DclAssert(pTags);

                /*  Decode tag to user buffer.  Note that an uncorrectable
                    or obviously invalid tag doesn't cause an error, it is
                    represented as all zeroes.
                */
                if(DecodeTag(pNFC, pTags, pNFC->pSpareBuf, nTagSize))
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                    pNFC->ulCorrectedErrors++;
                }

                pTags = (void*)((char*)pTags + nTagSize);
            }

            if(ISWRITTENWITHECC(((D_UINT8*)pNFC->pSpareBuf)[SPARE_INDEX_PAGESTAT]))
            {
                if(ulSR & DBERR)
                {
                    /*  There was an uncorrectable error, the read fails.
                    */
                    ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;

                    pNFC->ulUncorrectableErrors++;
                  
                  #if D_DEBUG
                    FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(H) uncorrectable data error %s\n", FfxDecodeIOStatus(&ioStat)));
                    DFCDump(pNFC->Params.pulPXA320Base);

                    DclHexDump(NULL, HEXDUMP_UINT8, 32, 2048, pMain);
                  #endif
                }
                else if(NDSR & SBERR)
                {
                    NDSR = SBERR;

                    FFXPRINTF(1, (NTMNAME"-DFCPageReadECCCacheMode(I) error corrected\n"));

                    /*  Single-bit error, succeed but report.
                    */
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;

                    pNFC->ulCorrectedErrors++;
                }
            }
        }
        else
        {
            /*  Didn't need the spare area, so discard it.
            */
            DataDiscard(pNFC, pNFC->uUserSize);
        }

        pMain = (void*)((char*)pMain + pNFC->uPageSize);

        ioStat.ulCount++;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
        nCount--;
        ulPage++;
    }

    /*  If the TR1 register was changed, and we have not already restored
        it (due to an early exit), restore it to its original value.
    */
    if(ulOldTR1 != D_UINT32_MAX)
        NDTR1CS0 = ulOldTR1;

    return ioStat;
}

#endif


/*-------------------------------------------------------------------
    Local: DFCControl4BitECC()

    Enable or Disable the on-die 4-bit ECC.

    Parameters:
        pNFC     - private data for this NFC instance
        nChip    - Which chip to select
        fEnable  - TRUE:  enable
                   FALSE: disable

    Return Value:
        Returns a standard FFXSTATUS value.  
-------------------------------------------------------------------*/
static FFXSTATUS DFCControl4BitECC(
    NFC            *pNFC,
    unsigned        nChip,
    D_BOOL          fEnable)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS; 
    D_UINT32        ulSR;
    D_UINT32        ulCmd0, ulCmd1, ulCmd2;
    unsigned        nCount;
    D_BOOL         *pfState;

    /*  These must be global because this is a physical hardware
        state and other instances of this NTM must be able to 
        accurately update it.
    */
    static D_BOOL   fCurrentState0 = DISABLE;
    static D_BOOL   fCurrentState1 = DISABLE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-DFCControl4BitECC(A) Chip=%u Enable=%u CurrentStateChip0=%u CurrentStateChip1=%u\n",
        nChip, fEnable, fCurrentState0, fCurrentState1));

    DclAssert(pNFC);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    if(nChip == 0)
        pfState = &fCurrentState0;
    else
        pfState = &fCurrentState1;

    if(*pfState == fEnable)
        return ffxStat;

    /* ECC_EN has been disabled on the NFC already. 
       Disable spare area access for the moment as
       SetFeature does not access the spare area.
    */
    NDCR &= ~SPARE_EN;

    /*  Clear the status register and start the operation.
    */
    NDSR = NDSR;
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        FFXPRINTF(1, (NTMNAME"-DFCControl4BitECC(B) error in write command request \n"));
        return FFXSTAT_FIMTIMEOUT;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        are read back from their individual addresses.  The apparent
        but overwriting of NDCB0 is not a typo, it's intentional.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_PROGRAM
             | ADDR_CYC(1)       
             | CMD1(MICRON_CMD_SET_FEATURE) | CMD2(0));
    ulCmd1 = MICRON_FEATURE_ADDRESS_ARRAY_OP;
    ulCmd2 = 0;

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCControl4BitECC(C) Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    /* Wait for the write data request and then enable ECC
       by writing to the feature address
    */
    if(DFCWaitForStatus(pNFC, WRDREQ) != WRDREQ)
    {
        FFXPRINTF(1, (NTMNAME"-DFCControl4BitECC(D) error in writing to feature address \n"));
        return FFXSTAT_FIMTIMEOUT;
    }

    if(fEnable)
        NDDB = MICRON_FEATURE_ENABLE_ECC;
    else
        NDDB = MICRON_FEATURE_DISABLE_ECC;

    /*  Dummy writes of remaining page size according to flash type into NDDB.
    */
    nCount = pNFC->uPageSize / 4;
    while(--nCount) 
        NDDB = 0;
    
    /*  See if the command completed by checking the CSx_CMDD bit in NDSR.
    */
    ulSR = DFCWaitForStatus(pNFC, (nChip ? CS1_CMDD : CS0_CMDD));
    if(!ulSR)
    {
        FFXPRINTF(1, (NTMNAME"-DFCControl4BitECC(E) error in command completion \n"));
        return FFXSTAT_FIMTIMEOUT;
    }

    *pfState = fEnable;

    NDCR &= ~ND_RUN;

    return ffxStat;

}


/*-------------------------------------------------------------------
    Local: DFCPageReadECC()

    Read physical pages, using ECC on the main page data, and
    decoding the tags from the spare area.

    If an uncorrectable error occurs, the user buffer will
    contain the uncorrected data.

    Parameters:
        pNFC     - private data for this NFC instance
        nChip    - Which chip to select
        ulPage   - The page in the flash chip.
        nCount   - The number of pages to read.  The range of
                   pages must not cross an erase block boundary.
        pMain    - A pointer to the buffer for main page data.  If
                   NULL, main page data is discarded and the ECC
                   result is not reported.
        pTags    - A buffer to receive the tag data.  May be NULL
                   only if nTagSize is 0 (parameter is ignored).
        nTagSize - The tag size to use.  If this value is 0, then
                   pTags will be ignored.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were read; this may be
        less than nCount.  The status indicates whether the read
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCPageReadECC(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount,
    void       *pMain,
    void       *pTags,
    unsigned    nTagSize)
{
    FFXIOSTATUS ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    ulSR;
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;
    D_UINT8     bChipReadStatus;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-DFCPageReadECC() Chip=%u Page=%lU Count=%u pMain=%P pTags=%P TagSize=%u\n",
        nChip, ulPage, nCount, pMain, pTags, nTagSize));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    if(pNFC->fM60EDCAllowed)
    {
      #if SUPPORT_M60_MIXED_MODE
        if(!pNFC->fUseM60ECC)
        {
            /*  Use the PXA320 controller's ECC for this page.
            */
            NDCR |= ECC_EN;

            ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, DISABLE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                return ioStat;
        }
        else
      #endif
        {
            /*  Disable controller ECC and enable on-die ECC.
            */
            NDCR &= ~ECC_EN;

            ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, ENABLE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                return ioStat;
        }
    }
    else
    {
        /*  Enable ECC in NFC.
        */
        NDCR |= ECC_EN;
    }

    /*  Always enable spare area.
    */
    NDCR |= SPARE_EN;

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(B) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        are read back from their individual addresses.  The apparent
        but overwriting of NDCB0 is not a typo, it's intentional.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_READ
             | (pNFC->fReadConfirm ? DBC : 0)
             | pNFC->uAddrCycRW
             | CMD1(CMD_READ_MAIN) | CMD2(CMD_READ_MAIN_START));
    ulCmd1 = ulPage << pNFC->uPageShift;
    ulCmd2 = ulPage >> (32 - pNFC->uPageShift);

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCPageReadECC(B) Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    /*  The NAND chip goes busy, then ready when it has finished the read and
        data is available; this sets the RDY bit in NDSR.  The DFC then has to
        clock the data out of the chip.  When it has finished doing this it
        sets the RDDREQ bit in NDSR.
    */
    ulSR = DFCWaitForStatus(pNFC, RDDREQ | DBERR);
    if(!ulSR)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(C) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    /*  Always read the data out of the DFC.
    */
    if(pMain)
    {
        DataRead(pNFC, pMain, pNFC->uPageSize);
    }
    else
    {
        /*  Dummy reads of page size according to flash type out of NDDB.
        */
        DataDiscard(pNFC, pNFC->uPageSize);
    }

    if(pNFC->fM60EDCAllowed && !(NDCR & ECC_EN)) 
    {
        /*  If we are using the M60 ECC, we must read the whole spare
            area, not just the user size.  ECC_EN will only be off
            if M60 is being used.
        */    
        DataRead(pNFC, pNFC->pSpareBuf, pNFC->uSpareSize);
    }
    else
    {
        DataRead(pNFC, pNFC->pSpareBuf, pNFC->uUserSize);
    }

    /*  If tags are desired, get the user space portion of the spare area
        out of the DFC buffer and decode its contents.
    */
    if(nTagSize)
    {
        DclAssert(pTags);

        /*  Decode tag to user buffer.  Note that an uncorrectable or
            obviously invalid tag doesn't cause an error, it is represented
            as all zeroes.
        */
        if(DecodeTag(pNFC, pTags, pNFC->pSpareBuf, nTagSize))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
            pNFC->ulCorrectedErrors++;
        }
    }

    if(pNFC->fM60EDCAllowed && !(NDCR & ECC_EN))  /* ECC_EN check for SUPPORT_M60_MIXED_MODE case */
    {
        bChipReadStatus = DFCReadStatus(pNFC, nChip);

        switch(bChipReadStatus & (NANDSTAT_ERROR | NANDSTAT_DATA_CORRECTED))
        {
            case 0:
                /*  Passed.
                */
                break;
                
            case NANDSTAT_ERROR:
            {
                D_BUFFER   *pTagData = (D_BUFFER*)pTags; 
                                    
                /*  Uncorrectable error.
                */
                ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;

                if(pTagData && (pTagData[0] != ERASED8 || pTagData[1] != ERASED8))
                {
                    pTagData[0] = 0;
                    pTagData[1] = 0;
                }

                pNFC->ulUncorrectableErrors++;

              #if D_DEBUG
                FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(D) uncorrectable data error %s\n", FfxDecodeIOStatus(&ioStat)));
                DFCDump(pNFC->Params.pulPXA320Base);

/*              DclHexDump(NULL, HEXDUMP_UINT8, 32, 2048, pMain); */
              #endif
              
                break;
            }
                
            case NANDSTAT_DATA_CORRECTED:
                /*  Pass but with ECC correction.
                */
                FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(E) error corrected\n"));

                /*  4-bit or less error, succeed but report.
                */
                ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;

                pNFC->ulCorrectedErrors++;
                break;

            default:
                /*  Both bits should never be set...
                */
                DclProductionError();
        }
    }
    else if(pMain && ISWRITTENWITHECC(((D_UINT8*)pNFC->pSpareBuf)[SPARE_INDEX_PAGESTAT]))
    {
        if(ulSR & DBERR)
        {
            /*  There was an uncorrectable error, the read fails.
            */
            ioStat.ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;

            pNFC->ulUncorrectableErrors++;
          
          #if D_DEBUG
            FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(F) uncorrectable data error %s\n", FfxDecodeIOStatus(&ioStat)));
            DFCDump(pNFC->Params.pulPXA320Base);

            DclHexDump(NULL, HEXDUMP_UINT8, 32, 2048, pMain);
          #endif
        }
        else if((NDSR & SBERR) != 0)
        {
            NDSR = SBERR;

            FFXPRINTF(1, (NTMNAME"-DFCPageReadECC(G) error corrected\n"));

            /*  Single-bit error, succeed but report.
            */
            ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;

            pNFC->ulCorrectedErrors++;

/*          DclHexDump(NULL, HEXDUMP_UINT8, 32, 2048, pMain); */
        }
    }

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        ioStat.ulCount = 1;

    DFCWaitForNDRUN(pNFC);

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: DFCPageReadRaw()

    Read physical pages, returning exactly the bits read from the
    main page and entire spare area.  There is no ECC processing
    of the main page or interpretation of the spare area.

    Parameters:
        pNFC   - private data for this NFC instance
        nChip  - Which chip to select
        ulPage - The page in the flash chip.
        nCount - The number of pages to read.  The range of
                 pages must not cross an erase block boundary.
        pMain  - A pointer to the buffer for main page data.  If
                 NULL, main page data is discarded.
        pSpare - A pointer to the buffer for spare area data.
                 If NULL, the spare area is not read.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were read; this may be
        less than nCount.  The status indicates whether the read
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCPageReadRaw(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount,
    void       *pMain,
    void       *pSpare)
{
    FFXIOSTATUS ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-DFCPageReadRaw() Chip=%u Page=%lU Count=%u pMain=%P pSpare=%P\n",
        nChip, ulPage, nCount, pMain, pSpare));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Disable ECC for raw read.
    */
    NDCR &= ~ECC_EN;

    if(pNFC->fM60EDCAllowed)
    {
        /* Disable on-die ECC.
        */
        ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, DISABLE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            return ioStat;
    }

    /*  Tell the DFC to return the spare area only if the tag is desired.
    */
    if (pSpare)
        NDCR |= SPARE_EN;
    else
        NDCR &= ~SPARE_EN;

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadRaw(B) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_READ
             | (pNFC->fReadConfirm ? DBC : 0)
             | pNFC->uAddrCycRW
             | CMD1(CMD_READ_MAIN) | CMD2(CMD_READ_MAIN_START));
    ulCmd1 = ulPage << pNFC->uPageShift;
    ulCmd2 = ulPage >> (32 - pNFC->uPageShift);

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCPageReadRaw(B) Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    /*  The NAND chip goes busy, then ready when it has finished the read and
        data is available; this sets the RDY bit in NDSR.  The DFC then has to
        clock the data out of the chip.  When it has finished doing this it
        sets the RDDREQ bit in NDSR.

        [Is this exactly correct?  Is the buffer big enough for an entire page?
        Or does RDDREQ cycle as portions of the page are transferred?  That doesn't
        make sense, because there's no way to find out when what's in the buffer
        so far has been drained]
    */
    if(DFCWaitForStatus(pNFC, RDDREQ) != RDDREQ)
    {
        /*  Return a ioStat indicating timeout.
        */
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageReadRaw(C) error %s\n", FfxDecodeIOStatus(&ioStat)));
        return ioStat;
    }

    /*  Always read the data out of the DFC.
    */
    if (pMain)
    {
        /*  Main page data was requested.  Copy page size according to
            flash type out of NDDB to user buffer.
         */
        DataRead(pNFC, pMain, pNFC->uPageSize);
    }
    else
    {
        /*  Dummy reads of page size according to flash type out of NDDB.
            ECC status is ignored.
         */
        DataDiscard(pNFC, pNFC->uPageSize);
    }

    /*  If the spare area is desired, get it out of the DFC buffer;
        otherwise, the controller was told not to bother loading this
        data into its buffer at all.
    */
    if (pSpare)
    {
        DataRead(pNFC, pSpare, pNFC->uSpareSize);
    }

    DFCWaitForNDRUN(pNFC);

    /*  We only do one page at a time...
    */
    ioStat.ulCount = 1;

    return ioStat;
}


/*  Little-endian!  But reasonably easy to fix.

    It might be slightly more efficient for unaligned transfers to
    align them with some odd bytes, then repack the bytes from NDDB
    and perform aligned 32-bit stores, and finish up with some odd
    bytes at the end.  But the repacking might cost more than the
    memory stores (which, after all, go to cache).
*/
/*-------------------------------------------------------------------
    Local: DataRead()

    Copy data from the DFC buffer to memory.

    Parameters:
        pNFC - private data for this NFC instance

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DataRead(
    NFC        *pNFC,
    void       *pData,
    unsigned    nCount)
{
    FFXPRINTF(2, ("DataRead into %P len=%u NDSR=%lX\n", pData, nCount, NDSR));

    (void)pNFC;

    DclAssert(pNFC);
    DclAssert(pData);
    DclAssert(nCount);
    DclAssert(nCount % PXA3XX_DFC_MINIMUM_IO_SIZE == 0);

    nCount >>= 3;

    if(DCLISALIGNED((D_UINTPTR)pData, sizeof(D_UINT32)) && (!(nCount & 0x1)))
    {
        register D_UINT32 *p = pData;

        nCount >>= 1;

        while (nCount--)
        {
            *p++ = NDDB;
            *p++ = NDDB;
            *p++ = NDDB;
            *p++ = NDDB;
/*            p += 4; */
        }

        return;
    }

    if(DCLISALIGNED((D_UINTPTR)pData, sizeof(D_UINT16)))
    {
        D_UINT32 ulData;
        D_UINT16 *p = pData;

        while (nCount--)
        {
            ulData = NDDB;
            p[0] = (D_UINT16) ulData;
            p[1] = (D_UINT16) (ulData >> 16);
            ulData = NDDB;
            p[2] = (D_UINT16) ulData;
            p[3] = (D_UINT16) (ulData >> 16);
            p += 4;
        }
    }
    else
    {
        D_UINT32 ulData;
        D_UCHAR *p = pData;

        while (nCount--)
        {
            ulData = NDDB;
            p[0] = (D_UCHAR) ulData;
            p[1] = (D_UCHAR) (ulData >>= 8);
            p[2] = (D_UCHAR) (ulData >>= 8);
            p[3] = (D_UCHAR) (ulData >> 8);
            ulData = NDDB;
            p[4] = (D_UCHAR) ulData;
            p[5] = (D_UCHAR) (ulData >>= 8);
            p[6] = (D_UCHAR) (ulData >>= 8);
            p[7] = (D_UCHAR) (ulData >> 8);
            p += 8;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DataDiscard()

    Remove data from the DFC buffer without saving it.

    Parameters:
        pNFC - private data for this NFC instance

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DataDiscard(
    NFC        *pNFC,
    unsigned    nCount)
{
    (void)pNFC;

    FFXPRINTF(2, ("DataDiscard len=%u NDSR=%lX\n", nCount, NDSR));

    DclAssert(nCount % PXA3XX_DFC_MINIMUM_IO_SIZE == 0);

    nCount >>= 3;

    while (nCount--)
    {
        (void) NDDB;
        (void) NDDB;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DFCPageWriteECC()

    Write physical pages, using ECC on the main page data, and
    encoding the tags in the spare area.

    Parameters:
        pNFC     - private data for this NFC instance
        nChip    - Which chip to select
        ulPage   - The page in the flash chip.
        nCount   - The number of pages to write.  The range of
                   pages must not cross an erase block boundary.
        pMain    - A pointer to the buffer containing main page
                   data.  If NULL, no main page data is written.
        pTags    - A buffer containing the tag data.  May be NULL
                   only if nTagSize is 0 (parameter is ignored).
        nTagSize - The tag size to use.  If this value is 0, then
                   pTags will be ignored.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were written; this may be
        less than nCount.  The ioStat indicates whether the write
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCPageWriteECC(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount,
    const void *pMain,
    const void *pTags,
    unsigned    nTagSize)
{
    FFXIOSTATUS ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);
    D_UINT32    uBbd = (nChip == 0 ? CS0_BBD : CS1_BBD);
    D_UINT32    ulSR;
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;
    D_UINT8     bStatus;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                    NTMNAME"-DFCPageWriteECC() Chip=%u Page=%lU Count=%u pMain=%P pTags=%P TagSize=%u\n",
                    nChip, ulPage, nCount, pMain, pTags, nTagSize));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    if(!nTagSize)
    {
        /*  If a tag was not specified, we still must minimally write
            the LEGACY_WRITTEN_WITH_ECC flag, so construct the spare
            buffer one time, for all pages.
        */
        DclAssert(!pTags);

        DclMemSet(pNFC->pSpareBuf, ERASED8, pNFC->uSpareSize);
        ((D_UINT8*)pNFC->pSpareBuf)[SPARE_INDEX_PAGESTAT] = LEGACY_WRITTEN_WITH_ECC;

        pTags = pNFC->pSpareBuf;
    }

    if(pNFC->fM60EDCAllowed)
    {
      #if SUPPORT_M60_MIXED_MODE
        if(!pNFC->fUseM60ECC)
        {
            /*  Use the PXA320 controller's ECC for this page.
            */
            NDCR |= ECC_EN;

            ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, DISABLE);
        }
        else
      #endif
        {
            /*  Disable controller ECC and enable on-die ECC.
            */
            NDCR &= ~ECC_EN;

            ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, ENABLE);
        }

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            return ioStat;
    }
    else
    {
        /*  Enable ECC on the NFC.
        */
        NDCR |= ECC_EN;
    }

    /*  Always enable spare area.
    */
    NDCR  |= SPARE_EN;

    while(nCount)
    {
        /*  If tag data was supplied, the tag needs to be encoded properly.
        */
        if(nTagSize)
        {
            DclAssert(pTags);

            DclMemSet(pNFC->pSpareBuf, ERASED8, pNFC->uSpareSize);
            EncodeTag(pNFC, pTags, pNFC->pSpareBuf, nTagSize);

            pTags = (void*)((char*)pTags + nTagSize);

            ((D_UINT8*)pNFC->pSpareBuf)[SPARE_INDEX_PAGESTAT] = LEGACY_WRITTEN_WITH_ECC;
        }

        /*  Clear NDSR: bits that are set are cleared by writing one to them, so
            reading it and writing it back clears it completely.
        */
        NDSR = NDSR;

        /*  Start the operation.
        */
        NDCR |= ND_RUN;

        /*  The controller will assert its "write command request" for the selected
            chip when it is ready for the command.
        */
        if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

            FFXPRINTF(1, (NTMNAME"-DFCPageWriteECC(B) error %s\n", FfxDecodeIOStatus(&ioStat)));

            return ioStat;
        }

        /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
            but are read back from their individual addresses.  The apparent
            overwriting of NDCB0 is not a typo, it's intentional.
        */
        ulCmd0 = ((nChip ? CSEL : 0)
                 | AUTO_RS
                 | CMD_TYPE_PROGRAM
                 | DBC
                 | pNFC->uAddrCycRW
                 | CMD1(CMD_PROGRAM));

      #if SUPPORT_CACHE_MODE_OPS
        if(pNFC->fUseCacheModeWrites && (nCount > 1))
            ulCmd0 |= CMD2(MICRON_CMD_PROGRAM_CACHE_MODE);
        else
      #endif
            ulCmd0 |= CMD2(CMD_PROGRAM_START);

        ulCmd1 = ulPage << pNFC->uPageShift;
        ulCmd2 = ulPage >> (32 - pNFC->uPageShift);

        NDCB0 = ulCmd0;
        NDCB0 = ulCmd1;
        NDCB0 = ulCmd2;

      #if D_DEBUG
        if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
        {
            FFXPRINTF(1, ("DFCPageWriteECC() Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
                ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
        }
      #endif

        /*  The DFC should (almost) immediately request the data to be written.
        */
        if(DFCWaitForStatus(pNFC, WRDREQ) != WRDREQ)
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

            FFXPRINTF(1, (NTMNAME"-DFCPageWriteECC(C) error %s\n", FfxDecodeIOStatus(&ioStat)));

            return ioStat;
        }

        /*  If it indeed is ready for data, copy data into the DFC buffer.
        */
        if(pMain)
        {
            DataWrite(pNFC, pMain, pNFC->uPageSize);
            pMain = (void*)((char*)pMain + pNFC->uPageSize);
        }
        else
        {
            DataFill(pNFC, pNFC->uPageSize);
        }

        if(pNFC->fM60EDCAllowed && !(NDCR & ECC_EN))  /* ECC_EN check for SUPPORT_M60_MIXED_MODE case */
        {
            DataWrite(pNFC, pNFC->pSpareBuf, pNFC->uSpareSize);
        }
        else
        {
            DataWrite(pNFC, pNFC->pSpareBuf, pNFC->uUserSize);
        }

        /*  The controller will assert its "CmdDone" for the selected
            chip if it completes successfully, or "BadBlock" if the
            operation failed.
        */
        ulSR = DFCWaitForStatus(pNFC, uCmdd | uBbd);
        if(!ulSR)
        {
            ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

            FFXPRINTF(1, (NTMNAME"-DFCPageWriteECC(D) timeout error %s\n", FfxDecodeIOStatus(&ioStat)));

            return ioStat;
        }

        /*  Check for program failure.  If the "BadBlock" bit was not
            set then the "CmdDone" bit must have been set, and the
            operation was successful.
        */
        if(ulSR & uBbd)
        {
            /*  The status from the chip indicated program failure.
                Report a bad block.  "CmdDone" is documented not to
                be set in this case.
            */
            DclAssert((NDSR & uBbd) == 0);

            ioStat.ffxStat = FFXSTAT_FIMIOERROR;

            FFXPRINTF(1, (NTMNAME"-DFCPageWriteECC(E) bad block detected %s\n", FfxDecodeIOStatus(&ioStat)));

            return ioStat;
        }

        /*  If it was not the BadBlock bit, it must have been the
            CmdDone bit.
        */
        DclAssert(ulSR & uCmdd);

        /*  The PXA320 NAND controller blindly reports "all-is-well" after
            trying to write to a locked block.  Explicitly query the status
            register to find out if it really was successful.
        */
        bStatus = DFCReadStatus(pNFC, nChip);
        if(!(bStatus & NANDSTAT_WRITE_ENABLE))
        {
            ioStat.ffxStat = FFXSTAT_FIM_WRITEPROTECTEDPAGE;

            FFXPRINTF(1, (NTMNAME"-DFCPageWriteECC(F) attempted to write to a write protected area, Page=%lu, Status=0x%02x\n",
                ulPage, bStatus));

            return ioStat;
        }

        DFCWaitForNDRUN(pNFC); 

        ioStat.ulCount++;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
        nCount--;
        ulPage++;
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: EncodeTag()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void EncodeTag(
    NFC            *pNFC,
    const D_BUFFER *pTag,
    D_BUFFER       *pSpare,
    unsigned        nTagSize)
{
    D_BUFFER        abBuff[OFFSETZERO_META_SIZE + EXTRABYTES];

    /*  Calculate a basic checksum for the tag data
    */
    abBuff[OFFSETCHK] = Checksum(pTag, nTagSize);

    /*  Place the tag in the temp buffer, immediately following the checksum
    */
    DclMemCpy(&abBuff[OFFSETTAG], pTag, nTagSize);

    /*  Hamming code on the check byte and the whole tag
    */
    abBuff[OFFSETECC] = (D_UINT8)DclHammingCalculate(&abBuff[OFFSETCHK], nTagSize+1, 0);

    /*  A fully encoded tag, including the tag ECC and check bytes,
        should never appear to be erased, or within 1 bit of being
        erased.
    */
    DclAssert(!FfxNtmHelpIsRangeErased1Bit(abBuff, nTagSize + EXTRABYTES));

    /*  Place results into the proper locations in the spare area
    */
    pSpare[SPARE_INDEX_TAGECC] = abBuff[OFFSETECC];
    pSpare[SPARE_INDEX_TAGCHECK] = abBuff[OFFSETCHK];

    DclMemCpy(&pSpare[pNFC->nOffsetTagStart], &abBuff[OFFSETTAG], nTagSize);

    /*  OFFSETZERO style flash, which supports variable length tags,
        has a specially encoded tag length, with its own ECC.
    */
    if(pNFC->uPageSize == 2048)
    {
        DclAssert(nTagSize <= OFFSETZERO_META_SIZE);

        if(pNFC->fM60EDCAllowed)
        {
            pSpare[M60A_SPARE_INDEX_TAGLEN] = EncodeTagLength(nTagSize);
            DclAssert(!FfxNtmHelpIsRangeErased1Bit(&pSpare[M60A_SPARE_INDEX_TAGLEN], 1));
        }
        else
        {
            pSpare[OFFSETZERO_SPARE_INDEX_TAGLEN] = EncodeTagLength(nTagSize);

            /*  Similarly, the encoded tag length should never be
                within 1 bit of being erased.
            */
            DclAssert(!FfxNtmHelpIsRangeErased1Bit(&pSpare[OFFSETZERO_SPARE_INDEX_TAGLEN], 1));
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DecodeTag()

    Decode a tag from its on-media format.

    If no tag was written, meaning the tag fields are in their erased
    state (or within one bit thereof), the return buffer will be all
    0xFFs.  If an invalid tag is detected, the return buffer will be
    all zeros.  In both these cases, the return value will be FALSE.
    If the specified nTagSize value is different than that found on
    the media, this is treated as an invalid tag.

    If a valid tag is detected and returned, a return value of TRUE
    indicates that a correctable error was corrected.  Otherwise a
    return value of FALSE will be specified.

    Parameters:
        pNFC       - The NTM instance
        pTag       - Where to store the decoded tag
        pSpare     - Spare area in NTM format containing tag
        nTagSize   - The tag size to use

    Return Value:
        Returns TRUE if the tag data is valid, but contained a
        corrected error.  Returns FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL DecodeTag(
    NFC            *pNFC,
    D_BUFFER       *pTag,
    const D_BUFFER *pSpare,
    unsigned        nTagSize)
{
    unsigned        fErasedLength = TRUE;
    unsigned long   lError;
    D_BUFFER        abBuff[OFFSETZERO_META_SIZE + EXTRABYTES];

    DclAssert(nTagSize <= OFFSETZERO_META_SIZE);

    /*  OFFSETZERO style flash, which supports variable length tags,
        has a specially encoded tag length, with its own ECC.
    */
    if(pNFC->uPageSize == 2048)
    {
        /*  If the value is within 1 bit of being erased, then there is
            no length, so return FALSE.
        */
        if(pNFC->fM60EDCAllowed)
        {
            if(!ISWITHIN1BIT(pSpare[M60A_SPARE_INDEX_TAGLEN], ERASED8))
            {
                if(DecodeTagLength(pSpare[M60A_SPARE_INDEX_TAGLEN]) != nTagSize)
                {
                    /*  If the encoded tag size does not match the tag size which
                        is being requested, then something is drastically wrong.
                        Return a null buffer to indicate a bogus tag.
                    */
                    DclMemSet(pTag, 0, nTagSize);
                    
                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                        "DecodeTag(A) Unrecoverable tag error\n"));
                    
                    return FALSE;
                }
                /*  Set to FALSE to indicate that the tag length value (if any)
                    is NOT in the erased state.
                */
                fErasedLength = FALSE;
            }
        }
        else
        {
            if(!ISWITHIN1BIT(pSpare[OFFSETZERO_SPARE_INDEX_TAGLEN], ERASED8))
            {
                if(DecodeTagLength(pSpare[OFFSETZERO_SPARE_INDEX_TAGLEN]) != nTagSize)
                {
                    /*  If the encoded tag size does not match the tag size which
                        is being requested, then something is drastically wrong.
                        Return a null buffer to indicate a bogus tag.
                    */
                    DclMemSet(pTag, 0, nTagSize);

                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                        "DecodeTag(B) Unrecoverable tag error\n"));
                    
                    return FALSE;
                }

                /*  Set to FALSE to indicate that the tag length value (if any)
                    is NOT in the erased state.
                */
                fErasedLength = FALSE;
            }
        }
    }

    /*  Move the tag ECC, check byte, and tag data itself, into a temporary
        buffer, so we can properly examine the whole thing.
    */
    abBuff[OFFSETECC] = pSpare[SPARE_INDEX_TAGECC];
    abBuff[OFFSETCHK] = pSpare[SPARE_INDEX_TAGCHECK];

    DclMemCpy(&abBuff[OFFSETTAG], &pSpare[pNFC->nOffsetTagStart], nTagSize);

    /*  A fully encoded tag, including the tag ECC and check bytes,
        should never appear to be erased, or within 1 bit of being
        erased.  If it is, then simply return a tag value which is
        a fully erased buffer.
    */
    if(FfxNtmHelpIsRangeErased1Bit(abBuff, nTagSize + EXTRABYTES))
    {
        if(fErasedLength)
        {
            /*  The tag ECC, check byte, data, and TagLen value (if any)
                are all with 1 bit of being erased, so there must have
                never been a tag written to this page.  In this event we
                MUST return a tag with the ERASED8 content, so the higher
                level software will know that this page was not written
                with a tag.
            */
            DclMemSet(pTag, ERASED8, nTagSize);
        }
        else
        {
            /*  The tag length value is NOT erased, but the rest of the
                stuff (ECC, check byte, and data) is.  This is not valid
                so return a null tag.
            */
            DclMemSet(pTag, 0, nTagSize);
        }

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
            "DecodeTag(C) Unrecoverable tag error\n"));
                    
        return FALSE;
    }

    /*  Perform any needed correction on the check byte and the whole tag
    */
    lError = DclHammingCalculate(&abBuff[OFFSETCHK], nTagSize+1, abBuff[OFFSETECC]);
    if(lError)
        DclHammingCorrect(&abBuff[OFFSETCHK], nTagSize+1, lError);

    /*  Calculate a basic checksum for the tag data
    */
    if(abBuff[OFFSETCHK] != Checksum(&abBuff[OFFSETTAG], nTagSize))
    {
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
            "DecodeTag(D) Unrecoverable tag error\n"));

        DclMemSet(pTag, 0, nTagSize);
        return FALSE;
    }

    /*  All good, copy the tag data into the output buffer.
    */
    DclMemCpy(pTag, &abBuff[OFFSETTAG], nTagSize);

    return lError ? TRUE : FALSE;
}


/*-------------------------------------------------------------------
    Local: EncodeTagLength()

    This function encodes a tag length, which must be from 1 to 15,
    in a form with a hamming code, which allows single-bit errors
    to be corrected.

    The returned value has the encoded length in the high nibble,
    and the hamming code in the low nibble.  The value is guaranteed
    never to return 0xFF.

    Parameters:
        nTagLen - The tag length value to encode, from 1 to 15.

    Return Value:
        Returns the encoded tag length value.
-------------------------------------------------------------------*/
static D_UINT8 EncodeTagLength(
    unsigned        nTagLen)
{
    unsigned long   lHamming;
    D_UINT8         bEncodedTagLen;
    D_UINT8         bTagLen;

    DclAssert(nTagLen);
    DclAssert(nTagLen <= OFFSETZERO_META_SIZE);

    /*  Make the tag length a number relative to zero so it requires
        one fewer bit to store (up to 4 bits for 0 to 15).
    */
    nTagLen--;
    bTagLen = (D_UINT8)nTagLen;

    lHamming = DclHammingCalculate(&bTagLen, sizeof(bTagLen), 0);

    bEncodedTagLen = (D_UINT8)((bTagLen << 4) | (lHamming & 0x7));

    /*  The encoded tag length must never be within 1 bit of being erased
        (which is why the max meta length is 15 rather than 16.
    */
    DclAssert(!ISWITHIN1BIT(bEncodedTagLen, ERASED8));

    return bEncodedTagLen;
}


/*-------------------------------------------------------------------
    Local: DecodeTagLength()

    This function decodes a tag length value, which was encoded with
    EncodeTagLength().

    Parameters:
        bEncodedTagLen - The encoded tag length value.

    Return Value:
        Returns the unencoded tag length, a value from 1 to 15.
        Returns zero if the tag length is not valid.
-------------------------------------------------------------------*/
static unsigned DecodeTagLength(
    D_UINT8         bEncodedTagLen)
{
    unsigned long   lError;
    unsigned long   lHamming = bEncodedTagLen & 0x7;
    D_UINT8         bTagLen = bEncodedTagLen >> 4;

    /*  If the value is within 1 bit of being erased, then there is
        no length, so return 0.
    */
    if(ISWITHIN1BIT(bEncodedTagLen, ERASED8))
        return 0;

    lError = DclHammingCalculate(&bTagLen, 1, lHamming);
    if(lError)
        DclHammingCorrect(&bTagLen, 1, lError);

    /*  The tag length at this point must be less than OFFSETZERO_META_SIZE.
        If not, then this page probably was not written with a tag at all
        and we must return 0, indicating that the tag length is bogus.
    */
    if(bTagLen >= OFFSETZERO_META_SIZE)
        return 0;

    return (unsigned)(bTagLen+1);
}


/*-------------------------------------------------------------------
    Local: Checksum()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_UINT8 Checksum(
    const D_BUFFER *pData,
    unsigned        nLen)
{
    D_UINT8         bChecksum = 0;

    while(nLen)
    {
        bChecksum += *pData++;
        nLen--;
    }

    bChecksum = (~bChecksum) + 1;

    return bChecksum;
}


/*-------------------------------------------------------------------
    Local: DFCPageWriteRaw()

    Write physical pages, using no ECC on the main page data, and
    writing the entire spare area with supplied data.

    Parameters:
        pNFC   - private data for this NFC instance
        nChip  - Which chip to select
        ulPage - The page in the flash chip.
        nCount - The number of pages to write.  The range of
                 pages must not cross an erase block boundary.
        pMain  - A pointer to the buffer for main page data.  If
                 NULL, no main page data is written,
        pSpare - A pointer to the buffer for spare area data.
                 If NULL, nothing is written in the spare area.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were written; this may
        be less than nCount.  The status indicates whether the write
        was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCPageWriteRaw(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount,
    const void *pMain,
    const void *pSpare)
{
    FFXIOSTATUS ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);
    D_UINT32    uBbd = (nChip == 0 ? CS0_BBD : CS1_BBD);
    D_UINT32    ulSR;
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;
    D_UINT8     bStatus;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                    NTMNAME"-DFCPageWriteRaw() Chip=%u Page=%lU Count=%u pMain=%P pSpare=%P\n",
                    nChip, ulPage, nCount, pMain, pSpare));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Always disable ECC.
    */
    NDCR &= ~ECC_EN;

    if(pNFC->fM60EDCAllowed)
    {
        /* Disable on-die ECC.
        */
        ioStat.ffxStat = DFCControl4BitECC(pNFC, nChip, DISABLE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            return ioStat;

        /*  With the on-die ECC disabled, the M60 does not automatically
            clear error conditions. 
        */
        DFCResetChip(pNFC, nChip);
    }

    /*  If spare area data was supplied, the DFC must be told to accept data
        for the spare area.
    */
    if (pSpare)
        NDCR |= SPARE_EN;
    else
        NDCR &= ~SPARE_EN;

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageWriteRaw(B) error %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | AUTO_RS
             | CMD_TYPE_PROGRAM
             | DBC
             | pNFC->uAddrCycRW
             | CMD1(CMD_PROGRAM) | CMD2(CMD_PROGRAM_START));
    ulCmd1 = ulPage << pNFC->uPageShift;
    ulCmd2 = ulPage >> (32 - pNFC->uPageShift);

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCPageWriteRaw() Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    /*  The DFC should (almost) immediately request the data to be written.
    */
    if(DFCWaitForStatus(pNFC, WRDREQ) != WRDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageWriteRaw(C) error %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  If it indeed is ready for data, copy data into the DFC buffer.
    */
    if (pMain)
        DataWrite(pNFC, pMain, pNFC->uPageSize);
    else
        DataFill(pNFC, pNFC->uPageSize);

    if (pSpare)
        DataWrite(pNFC, pSpare, pNFC->uSpareSize);

    /*  The controller will assert its "CmdDone" for the selected
        chip if it completes successfully, or "BadBlock" if the
        operation failed.
    */
    ulSR = DFCWaitForStatus(pNFC, uCmdd | uBbd);
    if(!ulSR)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCPageWriteRaw(D) timeout error %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  Check for program failure.  If the "BadBlock" bit was not
        set then the "CmdDone" bit must have been set, and the
        operation was successful.
    */
    if(ulSR & uBbd)
    {
        /*  The status from the chip indicated program failure.
            Report a bad block.  "CmdDone" is documented not to
            be set in this case.
        */
        DclAssert((NDSR & uBbd) == 0);

        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

        FFXPRINTF(1, (NTMNAME"-DFCPageWriteRaw(E) bad block detected %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  If it was not the BadBlock bit, it must have been the
        CmdDone bit.
    */
    DclAssert(ulSR & uCmdd);

    /*  The PXA320 NAND controller blindly reports "all-is-well" after
        trying to write to a locked block.  Explicitly query the status
        register to find out if it really was successful.
    */
    bStatus = DFCReadStatus(pNFC, nChip);
    if(!(bStatus & NANDSTAT_WRITE_ENABLE))
    {
        ioStat.ffxStat = FFXSTAT_FIM_WRITEPROTECTEDPAGE;

        FFXPRINTF(1, (NTMNAME"-DFCPageWriteRaw(F) attempted to write to a write protected area, Page=%lu, Status=0x%02x\n",
            ulPage, bStatus));

        return ioStat;
    }

    ioStat.ulCount = 1;

    DFCWaitForNDRUN(pNFC); 

    return ioStat;
}


/*  Little-endian!  But reasonably easy to fix.

    It might be slightly more efficient for unaligned transfers to
    align them with some odd bytes, then repack the bytes from NDDB
    and perform aligned 32-bit stores, and finish up with some odd
    bytes at the end.  But the repacking might cost more than the
    memory stores (which, after all, go to cache).
*/
/*-------------------------------------------------------------------
    Local: DataWrite()

    Parameters:
        pNFC   - private data for this NFC instance
        pData  -
        nCount     - number of bytes of data, must be a multiple of
                     PXA3XX_DFC_MINIMUM_IO_SIZE

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DataWrite(
    NFC        *pNFC,
    const void *pData,
    unsigned    nCount)
{
    DclAssert(nCount % PXA3XX_DFC_MINIMUM_IO_SIZE == 0);

    nCount >>= 3;

    if(DCLISALIGNED((D_UINTPTR)pData, sizeof(D_UINT32)))
    {
        const D_UINT32 *p = pData;

        while (nCount--)
        {
            NDDB = p[0];
            NDDB = p[1];
            p += 2;
        }

        return;
    }

    if(DCLISALIGNED((D_UINTPTR)pData, sizeof(D_UINT16)))
    {
        const D_UINT16 *p = pData;

        while (nCount--)
        {
            NDDB = p[0] | ((D_UINT32) p[1] << 16);
            NDDB = p[2] | ((D_UINT32) p[3] << 16);
            p += 4;
        }
    }
    else
    {
        const D_UCHAR *p = pData;

        while (nCount--)
        {
            NDDB = p[0] | ((D_UINT32) p[1] << 8) | ((D_UINT32) p[2] << 16) | ((D_UINT32) p[3] << 24);
            NDDB = p[4] | ((D_UINT32) p[5] << 8) | ((D_UINT32) p[6] << 16) | ((D_UINT32) p[7] << 24);
            p += 8;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DataFill()

    Parameters:
        pNFC   - private data for this NFC instance
        nCount     - number of bytes to fill, must be a multiple of
                     PXA3XX_DFC_MINIMUM_IO_SIZE

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DataFill(
    NFC        *pNFC,
    unsigned    nCount)
{
    DclAssert(nCount % PXA3XX_DFC_MINIMUM_IO_SIZE == 0);

    nCount >>= 3;

    while (nCount--)
    {
        NDDB = ERASED32;
        NDDB = ERASED32;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: MitigateDFCBug()

    This function of code exists solely to mitigate an apparent bug
    in the DFC, where the internal state of the controller is not
    properly reset after having corrected a bit error.  Reading any
    other page which has no error will clear the situation.
      
    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void MitigateDFCBug(
    NTMHANDLE           hNTM,
    unsigned            nChip,
    D_UINT32            ulPageOnChip,
    const FFXIOSTATUS  *pIOStat)
{
  #if SUPPORT_M60_MIXED_MODE
    if(!hNTM->pNFC->fUseM60ECC)
    {
        static D_UINT32     ulLastGoodPage1 = D_UINT32_MAX;
  #else
    if(!hNTM->pNFC->fM60EDCAllowed)
    {
  #endif

        static D_UINT32     ulLastGoodPage2 = D_UINT32_MAX;

        if(pIOStat->op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            FFXIOSTATUS         tmpStat;
            DCLALIGNEDBUFFER    (tag, data, FFX_NAND_TAGSIZE);
            
          #if SUPPORT_M60_MIXED_MODE
            if(nChip == hNTM->pNFC->Params.nM60EDC4Chip)
            {
                tmpStat = DFCPageReadECC(hNTM->pNFC, nChip, ulLastGoodPage1, 1, NULL, tag.data, FFX_NAND_TAGSIZE);

                FFXPRINTF(1, ("DFC bug mitigation: Reading a bogus tag read from chippage %lX, status=%s\n", 
                    ulLastGoodPage1, FfxDecodeIOStatus(&tmpStat)));
            }
            else
          #endif
            {
                tmpStat = DFCPageReadECC(hNTM->pNFC, nChip, ulLastGoodPage2, 1, NULL, tag.data, FFX_NAND_TAGSIZE);

                FFXPRINTF(1, ("DFC bug mitigation: Reading a bogus tag read from chippage %lX, status=%s\n", 
                    ulLastGoodPage2, FfxDecodeIOStatus(&tmpStat)));

                (void)tmpStat;
            }
        }
        else
        {
          #if SUPPORT_M60_MIXED_MODE
            if(nChip == hNTM->pNFC->Params.nM60EDC4Chip)   /* M60A */
                ulLastGoodPage1 = ulPageOnChip;
            else
          #endif
                ulLastGoodPage2 = ulPageOnChip;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: DFCBlockErase()

    Erase physical erase blocks.

    Parameters:
        pNFC   - private data for this NFC instance
        nChip  - Which chip to select
        ulPage - The page in the flash chip where the first block
                 to be erased begins.  This must be on an erase
                 block boundary.
        nCount - The number of blocks to erase.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks that were erased successfully;
        this may be less than nCount.  The status indicates whether
        the erase was entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS DFCBlockErase(
    NFC        *pNFC,
    unsigned    nChip,
    D_UINT32    ulPage,
    unsigned    nCount)
{
    FFXIOSTATUS ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);
    D_UINT32    uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);
    D_UINT32    uBbd = (nChip == 0 ? CS0_BBD : CS1_BBD);
    D_UINT32    ulSR;
    D_UINT32    ulCmd0, ulCmd1, ulCmd2;
    D_UINT8     bStatus;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                    NTMNAME"-DFCBlockErase() Chip=%u Page=%lU Count=%u\n",
                    nChip, ulPage, nCount));

    DclAssert(pNFC);
    DclAssert(nCount);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCBlockErase(A) error %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    ulCmd0 = ((nChip ? CSEL : 0)
             | AUTO_RS
             | CMD_TYPE_ERASE
             | DBC
             | pNFC->uAddrCycErase
             | CMD1(CMD_ERASE_BLOCK) | CMD2(CMD_ERASE_BLOCK_START));
    ulCmd1 = ulPage;  /* Address fits entirely in NDCB1 */
    ulCmd2 = 0;

    NDCB0 = ulCmd0;
    NDCB0 = ulCmd1;
    NDCB0 = ulCmd2;

  #if D_DEBUG
    if(NDCB0 != ulCmd0 || NDCB1 != ulCmd1 || NDCB2 != ulCmd2)
    {
        FFXPRINTF(1, ("DFCBlockErase() Error: NDCB0 %lX!=%lX || NDCB1 %lX!=%lX || NDCB2 %lX!=%lX\n",
            ulCmd0, NDCB0, ulCmd1, NDCB1, ulCmd2, NDCB2));
    }
  #endif

    /*  The controller will assert its "CmdDone" for the selected
        chip if it completes successfully, or "BadBlock" if the
        operation failed.
    */
    ulSR = DFCWaitForStatus(pNFC, uCmdd | uBbd);
    if(!ulSR)
    {
        ioStat.ffxStat = FFXSTAT_FIMTIMEOUT;

        FFXPRINTF(1, (NTMNAME"-DFCBlockErase(C) timeout error %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  Check for erase failure.  If the "BadBlock" bit was not
        set then the "CmdDone" bit must have been set, and the
        operation was successful.
    */
    if(ulSR & uBbd)
    {
        /*  The status from the chip indicated erase failure.
            Report a bad block.  "CmdDone" is documented not to
            be set in this case.
        */
        DclAssert((NDSR & uBbd) == 0);

        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

        FFXPRINTF(1, (NTMNAME"-DFCBlockErase() bad block detected %s\n", FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }

    /*  If it was not the BadBlock bit, it must have been the
        CmdDone bit.
    */
    DclAssert(ulSR & uCmdd);

    /*  The PXA320 NAND controller blindly reports "all-is-well" after
        trying to write to a locked block.  Explicitly query the status
        register to find out if it really was successful.
    */
    bStatus = DFCReadStatus(pNFC, nChip);
    if(!(bStatus & NANDSTAT_WRITE_ENABLE))
    {
        ioStat.ffxStat = FFXSTAT_FIM_WRITEPROTECTEDBLOCK;

        FFXPRINTF(1, (NTMNAME"-DFCBlockErase() attempted to erase a write protected area, Page=%lu, Status=0x%02x\n",
            ulPage, bStatus));

        return ioStat;
    }

    ioStat.ulCount = 1;

/*    DFCWaitForNDRUN(pNFC); */

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: DFCReadStatus()

    Parameters:
        pNFC       - private data for this NFC instance
        nChip      - Which chip to select

    Return Value:
        Returns the status byte, or 0xFF if an error occurred.
-------------------------------------------------------------------*/
static D_UINT8 DFCReadStatus(
    NFC                *pNFC,
    unsigned            nChip)
{
    DCLALIGNEDBUFFER    (id, data, PXA3XX_DFC_MINIMUM_IO_SIZE);

    DclAssert(pNFC);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return 0xFF;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_STATUS_READ
             | CMD1(CMD_STATUS));
    NDCB0 = 0;
    NDCB0 = 0;

    /*  The controller will assert its "ready to read" for the selected
        chip when it completes.
    */
    if(DFCWaitForStatus(pNFC, RDDREQ) != RDDREQ)
        return 0xFF;

    DataRead(pNFC, id.data, sizeof id.data);

    FFXPRINTF(2, (NTMNAME"-DFCReadStatus() returning 0x%02x\n", id.data[0]));

    /*  We're making an assumption that statis 0xFF is never returned
        under any normal circumstance -- assert it so.
    */
    DclAssert(id.data[0] != 0xFF);

    return id.data[0];
}


/*-------------------------------------------------------------------
    Local: DFCReadID()

    Obtain the ID information from the NAND chip.  Note that
    regardless whether this function succeeds or fails, the
    original value of NDCR is preserved.

    Parameters:
        pNFC   - private data for this NFC instance
        nChip  - Which chip to select
        ulPage - The page in the flash chip where the first block
                 to be erased begins.  This must be on an erase
                 block boundary.
        aucID  - Buffer to receive the ID information.

    Return Value:
        If the ID was read successfully, aucID[] contains the ID
        bytes and the function returns TRUE.  If there was a problem
        reading the ID, FALSE is returned, and the contents of
        aucID[] are undefined.
-------------------------------------------------------------------*/
static D_BOOL DFCReadID(
    NFC                *pNFC,
    unsigned            nChip,
    D_UCHAR             aucID[NAND_ID_SIZE])
{
    DCLALIGNEDBUFFER    (id, data, ((NAND_ID_SIZE + PXA3XX_DFC_MINIMUM_IO_SIZE-1) / PXA3XX_DFC_MINIMUM_IO_SIZE) * PXA3XX_DFC_MINIMUM_IO_SIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-DFCReadID() Chip=%u\n", nChip));

    DclAssert(pNFC);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_READ_ID
             | ADDR_CYC(1)
             | CMD1(CMD_READ_ID));
    NDCB0 = 0;
    NDCB0 = 0;

    /*  The controller will assert its "ready to read" for the selected
        chip when it completes.
    */
    if(DFCWaitForStatus(pNFC, RDDREQ) != RDDREQ)
    {
        DclMemSet(aucID, ERASED8, NAND_ID_SIZE);
        return FALSE;
    }

    DataRead(pNFC, id.data, sizeof id.data);
    DclMemCpy(aucID, id.data, NAND_ID_SIZE);

    DCLPRINTF(1, ("NAND ID Bytes: %02x %02x %02x %02x %02x %02x\n",
        aucID[0], aucID[1], aucID[2], aucID[3], aucID[4], aucID[5]));

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: DFCInitialize()

    Initialize the NAND controller.

    Find out the address of the NAND controller, and set it
    up at least enough to read the device ID.

    Parameters:
        hDev - handle for the device

    Return Value:
        A pointer to the struct NFC for the controller if successful,
        NULL if not.
-------------------------------------------------------------------*/
static NFC *DFCInitialize(
    FFXDEVHANDLE    hDev)
{
    NFC            *pNFC;

    pNFC = DclMemAllocZero(sizeof *pNFC);
    if(!pNFC)
        return NULL;

    pNFC->Params.nStructLen = sizeof(pNFC->Params);
    pNFC->hHook = FfxHookNTPxa320Create(hDev, &pNFC->Params);
    if(!pNFC->hHook)
    {
        DclMemFree(pNFC);
        return NULL;
    }

  #if D_DEBUG
    DclPrintf("Project Configuration Parameters:\n");
    DclPrintf("  PXA320 Register Base     %P\n",     pNFC->Params.pulPXA320Base);
    DclPrintf("  Cache Modes                %8s\n",  pNFC->Params.fSupportCacheMode ? "Enabled" : "Disabled");
    DclPrintf("  Support CS1                %8s\n",  pNFC->Params.fSupportCS1 ? "Yes" : "No");
    DclPrintf("  Support M60 on-die EDC     %8s\n",  pNFC->Params.fSupportM60OnDieEDC ? "Yes" : "No");
    DclPrintf("  M60 on-die EDC chip        %8u\n",  pNFC->Params.nM60EDC4Chip);
    DclPrintf("  M60 on-die first EDC block %8lU\n", pNFC->Params.ulM60EDC4Block);

    DclPrintf("Original PXA320 NAND controller registers (instance=%lU):\n", DclOsAtomic32Retrieve(&ulAtomicControllerInitCount));
    DclPrintf("        NDCR = %lX\n", PXA3XX_NDCR(pNFC->Params.pulPXA320Base));
    DclPrintf("        NDSR = %lX\n", PXA3XX_NDSR(pNFC->Params.pulPXA320Base));
    DclPrintf("       NDCB0 = %lX\n", PXA3XX_NDCB0(pNFC->Params.pulPXA320Base));
    DclPrintf("    NDTR0CS0 = %lX\n", PXA3XX_NDTR0CS0(pNFC->Params.pulPXA320Base));
    DclPrintf("    NDTR1CS0 = %lX\n", PXA3XX_NDTR1CS0(pNFC->Params.pulPXA320Base));
  #endif

    /*  We must only initialize the hardware once, and we must accommodate
        the notion that a given NTM instance initialization may fail, but
        any previous instances will be unaffected.
    */
    if(DclOsAtomic32Increment(&ulAtomicControllerInitCount) == 1)
    {
        NDCR = INTERRUPT_MASK_ALL;
        
        /*  Initialize the default timing characteristics.  These 
            settings must be generic enough to allow the NAND ID to
            be read, regardless of the flash type.  Once the ID has
            been read, FfxHookNTPxa320Configure() will be called to
            allow optimal timings to be specified.
        */    
        PXA3XX_NDTR0CS0(pNFC->Params.pulPXA320Base) = pNFC->Params.ulDefaultTR0;
        PXA3XX_NDTR1CS0(pNFC->Params.pulPXA320Base) = pNFC->Params.ulDefaultTR1;
    
        /*  Note that ND_ARB_EN in NDCR is carefully preserved, as it
            must be set up during system initialization "before the
            static memory controller is configured".

            (The comment above seems wrong, but it's been there since
            rev 1.1 dated 12Mar2007, with ND_ARB_EN handled the same way.)
        */
        NDCR = (/* NCSX | */
                (RD_ID_CNT(NAND_ID_SIZE))
                | (ND_ARB_EN)
                | (INTERRUPT_MASK_ALL));

        /*  Blindly reset both NAND chips that might be present.
        */
        (void)DFCResetChip(pNFC, 0);
        (void)DFCResetChip(pNFC, 1);

      #if D_DEBUG
        DclPrintf("PXA320 NAND controller registers used for auto-detect:\n");
        DclPrintf("        NDCR = %lX\n", PXA3XX_NDCR(pNFC->Params.pulPXA320Base));
        DclPrintf("    NDTR0CS0 = %lX\n", PXA3XX_NDTR0CS0(pNFC->Params.pulPXA320Base));
        DclPrintf("    NDTR1CS0 = %lX\n", PXA3XX_NDTR1CS0(pNFC->Params.pulPXA320Base));
      #endif
    }

    return pNFC;
}


/*-------------------------------------------------------------------
    Local: DFCConfigure()

    Configure the NFC and this NTM to operate with the kind of
    flash that was detected.

    Parameters:
        pNFC      - private data for this NFC instance
        pChipInfo - A pointer to the FFXNANDCHIP structure to use
        pabID     - A pointer to an array of NAND ID bytes, which will
                    contain at least NAND_ID_SIZE entries.

    Return Value:
        Returns an FFXSTATUS code indicating success or the nature
        of the failure.
-------------------------------------------------------------------*/
static FFXSTATUS DFCConfigure(
    NFC                *pNFC,
    const FFXNANDCHIP  *pChipInfo,
    D_BYTE             *pabID)
{
    FFXSTATUS           ffxStat;
    
    DclAssertWritePtr(pNFC, sizeof(*pNFC));
    DclAssertReadPtr(pChipInfo, sizeof(*pChipInfo));
    DclAssertWritePtr(pabID, NAND_ID_SIZE);

    /*  Allow the project hooks to do their thing...
    */
    if(DclOsAtomic32Retrieve(&ulAtomicControllerInitCount) == 1)
    {
        ffxStat = FfxHookNTPxa320Configure(pNFC->hHook, pChipInfo, pabID);
        if(ffxStat != FFXSTAT_SUCCESS)
            return ffxStat;

      #if D_DEBUG
        DFCDumpNDCR(pNFC->Params.pulPXA320Base);
        DFCDumpNDTR0CS0(pNFC->Params.pulPXA320Base);
        DFCDumpNDTR1CS0(pNFC->Params.pulPXA320Base);
      #endif
    }

    pNFC->fResetBeforeProgram   = pChipInfo->pChipClass->ResetBeforeProgram;
    pNFC->fReadConfirm          = pChipInfo->pChipClass->ReadConfirm;
    pNFC->uPageSize             = pChipInfo->pChipClass->uPageSize;
    pNFC->uSpareSize            = pChipInfo->pChipClass->uSpareSize;

    /*  Not supported by this NTM
    */
    DclAssert(!pNFC->fResetBeforeProgram);

    /*  An Erase command takes just a page number as an address.
    */
    pNFC->uAddrCycErase =
        ADDR_CYC((pChipInfo->pChipClass->uLinearChipAddrMSB
                  - pChipInfo->pChipClass->uLinearPageAddrMSB + 7)
                 / 8);

    /*  Read and Program (write) commands additionally take a byte address
        within a page.  This is always zero-filled (by shifting).  It is
        one byte on small-block, two bytes on large-block.
    */
    DclAssert(pChipInfo->pChipClass->uChipPageIndexLSB % 8 == 0);
    pNFC->uPageShift = pChipInfo->pChipClass->uChipPageIndexLSB;
    pNFC->uAddrCycRW = pNFC->uAddrCycErase + ADDR_CYC(pNFC->uPageShift / 8);

    DclAssert(pNFC->uPageSize == 2048 || pNFC->uPageSize == 512);

    /*  Update the controller for page size.  Available user
        portion of spare area is 8 bytes for small-block, 40 
        bytes (64 - 4 * 6) for large-block.
    */
    if(pNFC->uPageSize == 2048)
        pNFC->uUserSize = OFFSETZERO_USER_SIZE;
    else
        pNFC->uUserSize = SSFDC_USER_SIZE;

    /*  Allocate a buffer for the spare area.  This saves consuming stack
        space.
    */
    pNFC->pSpareBuf = DclMemAlloc(pNFC->uSpareSize);
    if(!pNFC->pSpareBuf)
    {
        FFXPRINTF(1, ("Failed to allocate buffer for spare area\n"));
        return DCLSTAT_MEMALLOCFAILED;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: DFCShutdown()

    Undo initialization of NAND controller.

    Parameters:
        pNFC - private data for this NFC instance

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DFCShutdown(
    NFC    *pNFC)
{
    DclAssert(pNFC);
    DclAssert(DclOsAtomic32Retrieve(&ulAtomicControllerInitCount) >= 1);

    /*  Free the resources for <this> instance, in reverse order of
        allocation.
    */
    if(pNFC->pSpareBuf)
        DclMemFree(pNFC->pSpareBuf);

    FfxHookNTPxa320Destroy(pNFC->hHook);

    DclMemFree(pNFC);

    DclOsAtomic32Decrement(&ulAtomicControllerInitCount);

    /*  Should probably at least clear the RUN bit, maybe mask off interrupts?
    */

    return;
}


/*-------------------------------------------------------------------
    Local: DFCResetChip()

    Issue a "reset" command to a NAND chip.

    Parameters:
        pNFC  - private data for this NFC instance
        nChip - Which chip to select

    Return Value:
        TRUE if the command was issued successfully, FALSE if not
        (typically because the NFC timed out).
-------------------------------------------------------------------*/
static D_BOOL DFCResetChip(
    NFC        *pNFC,
    unsigned    nChip)
{
    DclAssert(pNFC);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_RESET
             | CMD1(CMD_RESET));
    NDCB0 = 0;
    NDCB0 = 0;

    /*  Wait for RDY, rather than "Command Done".
        See the MV-S301195-00 app note from Marvell.
    */
    if(DFCWaitForStatus(pNFC, RDY) != RDY)
    {
        FFXPRINTF(1, ("DFCResetChip() failed for chip %u\n", nChip));
        return FALSE;
    }

/*    DFCWaitForNDRUN(pNFC); */
    return TRUE;
}


#if FFXCONF_NANDSUPPORT_MICRON

/*-------------------------------------------------------------------
    Local: DFCMicronLockStatus()

    Parameters:
        pNFC       - private data for this NFC instance
        nChip      - Which chip to select

    Return Value:
        Returns the lock status byte, or 0xFF if an error occurred.
-------------------------------------------------------------------*/
static D_UINT8 DFCMicronLockStatus(
    NFC                *pNFC,
    unsigned            nChip,
    D_UINT32            ulPage)
{
    DCLALIGNEDBUFFER    (id, data, PXA3XX_DFC_MINIMUM_IO_SIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-DFCMicronLockStatus() Chip=%u Page=%lU\n",
                    nChip, ulPage));

    DclAssert(pNFC);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return 0xFF;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_READ_ID
             | ADDR_CYC(3)
             | CMD1(MICRON_CMD_READ_LOCK_STATUS));
    NDCB0 = ulPage;
    NDCB0 = 0;

    /*  The controller will assert its "ready to read" for the selected
        chip when it completes.
    */
    if(DFCWaitForStatus(pNFC, RDDREQ) != RDDREQ)
        return 0xFF;

    DataRead(pNFC, id.data, sizeof id.data);

    return id.data[0];
}


/*-------------------------------------------------------------------
    Local: DFCMicronLockTight()

    Parameters:
        pNFC        - private data for this NFC instance
        nChip       - Which chip to select

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DFCMicronLockTight(
    NFC                *pNFC,
    unsigned            nChip)
 {
    D_UINT32            uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);

    FFXPRINTF(1, (NTMNAME"-DFCMicronLockTight() Chip=%u\n", nChip));

    DclAssert(pNFC);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_RESET
             | CMD1(MICRON_CMD_LOCKTIGHT));
    NDCB0 = 0;
    NDCB0 = 0;

    if(DFCWaitForStatus(pNFC, uCmdd) != uCmdd)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: DFCMicronLockBlocks()

    Parameters:
        pNFC        - private data for this NFC instance
        nChip       - Which chip to select

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DFCMicronLockBlocks(
    NFC                *pNFC,
    unsigned            nChip)
{
    D_UINT32            uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);

    FFXPRINTF(1, (NTMNAME"-DFCMicronLockBlocks() Chip=%u\n", nChip));

    DclAssert(pNFC);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_RESET
             | CMD1(MICRON_CMD_LOCK));
    NDCB0 = 0;
    NDCB0 = 0;

    if(DFCWaitForStatus(pNFC, uCmdd) != uCmdd)
        return FALSE;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: DFCMicronUnlockBlocks()

    Parameters:
        pNFC        - private data for this NFC instance
        nChip       - Which chip to select
        ulPageStart - The first page in the first block to unlock
        ulPageEnd   - The first page in the last block to unlock
        fInvert     - A flag to indicate whether the range should be
                      inverted.

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL DFCMicronUnlockBlocks(
    NFC                *pNFC,
    unsigned            nChip,
    D_UINT32            ulPageStart,
    D_UINT32            ulPageEnd,
    unsigned            fInvert)
{
    D_UINT32            uCmdd = (nChip == 0 ? CS0_CMDD : CS1_CMDD);
    D_UINT8             bStatus;

    FFXPRINTF(1, (NTMNAME"-DFCMicronUnlockBlocks() Chip=%u PageStart=%lX PageEnd=%lX Invert=%u\n",
        nChip, ulPageStart, ulPageEnd, fInvert));

    DclAssert(pNFC);
    DclAssert(ulPageStart <= ulPageEnd);
    DclAssert(nChip <= pNFC->nMaxChipSelect);

    /*  Clear NDSR: bits that are set are cleared by writing one to them, so
        reading it and writing it back clears it completely.
    */
    NDSR = NDSR;

    /*  Start the operation.
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for the selected
        chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
        return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_ERASE
             | ADDR_CYC(3)
             | CMD1(MICRON_CMD_UNLOCK1));
    NDCB0 = ulPageStart;
    NDCB0 = 0;

    if(DFCWaitForStatus(pNFC, uCmdd) != uCmdd)
        return FALSE;

    /*  Trickery to get around the NAND controller!  Tell the NAND
        controller to quit the previous operation, we'll then issue
        the second half of the unlock command.
    */
    NDCR &= ~ND_RUN;

    NDSR = NDSR;

    /*  Start the operation for the second part
    */
    NDCR |= ND_RUN;

    /*  The controller will assert its "write command request" for
        the selected chip when it is ready for the command.
    */
    if(DFCWaitForStatus(pNFC, WRCMDREQ) != WRCMDREQ)
         return FALSE;

    /*  NDCB0, NDCB1, and NDCB2 are all written (in that order) at NDCB0,
        but are read back from their individual addresses.  The apparent
        overwriting of NDCB0 is not a typo, it's intentional.
    */
    NDCB0 = ((nChip ? CSEL : 0)
             | CMD_TYPE_ERASE
             | ADDR_CYC(3)
             | CMD1(MICRON_CMD_UNLOCK2));
    NDCB0 = ulPageEnd | fInvert;
    NDCB0 = 0;

    /*  The controller will assert its "command done" for the selected
        chip when it completes.
    */
    if(DFCWaitForStatus(pNFC, uCmdd) != uCmdd)
        return FALSE;

    /*  We must clear this before exiting!
    */
    NDCR &= ~ND_RUN;

    /*  The PXA320 NAND controller blindly reports "all-is-well" after
        trying to write to a locked block.  Explicitly query the status
        register to find out if it really was successful.
    */
    bStatus = DFCReadStatus(pNFC, nChip);
    if(!(bStatus & NANDSTAT_WRITE_ENABLE))
    {
        FFXPRINTF(1, (NTMNAME"-DFCMicronUnlockBlocks() attempt to unlock failed, Page=%lu, Status=0x%02x\n",
            ulPageStart, bStatus));

        return FALSE;
    }

    return TRUE;
}

#endif


/*-------------------------------------------------------------------
    Local: DFCWaitForStatus()

    Wait for one of the specified status bits to be set.  When a
    given bit is detected, it is immediately cleared, and the bit
    is returned.

    Parameters:
        ulStatus    - One or more bits to detect

    Return Value:
        Returns the bit that was detected (and cleared), or zero if
        the operation timed out.
-------------------------------------------------------------------*/
static D_UINT32 DFCWaitForStatus(
    NFC        *pNFC,
    D_UINT32    ulStatus)
{
    D_UINT32    ulSR;

    FFXPRINTF(2, (NTMNAME"-DFCWaitForStatus(%lX) Current NDSR=%lX\n", ulStatus, NDSR));

    DclAssert(pNFC);
    DclAssert(ulStatus);

    if((NDSR & ulStatus) == 0)
    {
        DCLTIMER    t;

        DclTimerSet(&t, NAND_TIMEOUT);
        while ((NDSR & ulStatus) == 0)
        {
            if (DclTimerExpired(&t))
            {
                /*  Check again after expiration - this task isn't the
                    only one running on the CPU (in general)
                */
                if ((NDSR & ulStatus) == 0)
                {
                  #if D_DEBUG
                    FFXPRINTF(1, (NTMNAME"-DFCWaitForStatus() timeout on status %lX! Current NDSR=%lX\n", ulStatus, NDSR));

                    /*  Do a verbose register dump only if it was not RDY
                        we were waiting for.  Reduces needless output when
                        doing a reset on a non-existent chip.
                    */    
                    if(ulStatus != RDY)
                        DFCDump(pNFC->Params.pulPXA320Base);
                  #endif

                    return 0;
                }
            }
        }
    }

    ulSR = NDSR & ulStatus;

    /*  Turn off the status bit that we found
    */
    NDSR = ulSR;

    FFXPRINTF(2, (NTMNAME"-DFCWaitForStatus(%lX) found and cleared status %lX\n",
        ulStatus, ulSR));

    return ulSR;
}


/*-------------------------------------------------------------------
    Local: DFCWaitForNDRUN()

    Parameters:
        ulStatus -

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS DFCWaitForNDRUN(
    NFC        *pNFC)
{
    DCLTIMER    t;

    if((NDCR & ND_RUN) == 0)
        return FFXSTAT_SUCCESS;

    DclTimerSet(&t, NAND_TIMEOUT);
    while ((NDCR & ND_RUN) != 0)
    {
        if (DclTimerExpired(&t))
        {
            /*  Check again after expiration - this task isn't the
                only one running on the CPU (in general)
            */
            if ((NDCR & ND_RUN) != 0)
            {
              #if D_DEBUG
                FFXPRINTF(1, (NTMNAME"-DFCWaitForNDRUN() timeout on waiting for ND_RUN!\n"));
                DFCDump(pNFC->Params.pulPXA320Base);
              #endif
                DclError();
                return FFXSTAT_FAILURE;
            }
        }
    }

    return FFXSTAT_SUCCESS;
}



        /*-----------------------------------------------------*\
         *                                                     *
         *                Debugging Functions                  *
         *                                                     *
        \*-----------------------------------------------------*/



#if D_DEBUG

#define BUF_LEN (32*3 + 2)
static void DFCDumpBits(D_UINT32 ulVal)
{
    unsigned iBit;
    D_CHAR   buf[BUF_LEN];
    D_CHAR  *pBuf;

    DclMemSet(buf, 0, BUF_LEN);

    for (iBit = 31, pBuf = buf;
        (iBit > 0) && (pBuf - buf < BUF_LEN);
         iBit--, pBuf += 3)
    {
        DclSNPrintf(pBuf, 3, " %u ", (ulVal & (1 << iBit)) >> iBit);
    }
    DclPrintf("%s %u\n", buf, (ulVal & (1 << iBit)) >> iBit);
}

static void DFCDumpNDCR(
    volatile D_UINT32  *pulBase)
{
    D_UINT32            NDCRVal = PXA3XX_NDCR(pulBase);

    DclPrintf("NDCR %lX contains:\n", NDCRVal);
    DclPrintf( "                                  C                 P           C  C                           \n" );
    DclPrintf( "                                  L                 G     N     S  S  C  C                    W\n" );
    DclPrintf( " S           D  D              r  R        R     R  |  r  D     0  1  S  S  C  C              R\n" );
    DclPrintf( " P           W  W   P       N  e  |  C     D     A  P  e  |     |  |  0  1  S  S        W  R  C\n" );
    DclPrintf( " A  E  D  N  I  I   A       D  s  P  L     |     |  E  s  A     P  P  |  |  0  1  D  S  R  D  M\n" );
    DclPrintf( " R  C  M  D  D  D   G       |  e  G  R     I     S  R  e  R     A  A  C  C  |  |  B  B  D  D  D\n" );
    DclPrintf( " E  C  A  |  T  T   E    N  S  r  |  |     D     T  |  r  B  R  G  G  M  M  B  B  E  E  R  R  R\n" );
    DclPrintf( " |  |  |  R  H  H   |    C  T  v  C  E     |     A  B  v  |  D  E  E  D  D  B  B  R  R  E  E  E\n" );
    DclPrintf( " E  E  E  U  |  |   S    S  O  e  N  C     C     R  L  e  E  Y  D  D  D  D  D  D  R  R  Q  Q  Q\n" );
    DclPrintf( " N  N  N  N  C  M   Z    X  P  d  T  C     N     T  K  d  N  M  M  M  M  M  M  M  M  M  M  M  M\n" );
    DclPrintf( " -  -  -  -  -  -  |--|  -  -  -  -  -  |--T--|  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n" );
    DclPrintf( " 3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0  0\n" );
    DclPrintf( " 1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0\n" );
    DclPrintf( " -  -  -  -  -  -  ----  -  -  -  -  -  -------  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n" );

    DFCDumpBits(NDCRVal);
}

static void DFCDumpNDSR(
    volatile D_UINT32  *pulBase)
{
    D_UINT32            NDSRVal = PXA3XX_NDSR(pulBase);

    DclPrintf("NDSR %lX contains:\n", NDSRVal);
    DclPrintf( "                                                                C  C                           \n" );
    DclPrintf( "                                                                S  S  C  C                    W\n" );
    DclPrintf( "                                                                0  1  S  S  C  C              R\n" );
    DclPrintf( "                                                                |  |  0  1  S  S        W  R  C\n" );
    DclPrintf( "                                                                P  P  |  |  0  1  D  S  R  D  M\n" );
    DclPrintf( "                                                                A  A  C  C  |  |  B  B  D  D  D\n" );
    DclPrintf( "                                                             R  G  G  M  M  B  B  E  E  R  R  R\n" );
    DclPrintf( "                                                             D  E  E  D  D  B  B  R  R  E  E  E\n" );
    DclPrintf( "                                                             Y  D  D  D  D  D  D  R  R  Q  Q  Q\n" );
    DclPrintf( " |-----------------------reserved-------------------------|  -  -  -  -  -  -  -  -  -  -  -  -\n" );
    DclPrintf( " 3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0  0\n" );
    DclPrintf( " 1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0\n" );
    DclPrintf( " ----------------------------------------------------------  -  -  -  -  -  -  -  -  -  -  -  -\n" );

    DFCDumpBits(NDSRVal);
}

static void DFCDumpNDTR0CS0(
    volatile D_UINT32  *pulBase)
{
    D_UINT32            NDTR0CS0Val = PXA3XX_NDTR0CS0(pulBase);

    DclPrintf("NDTR0CS0 %lX contains:\n", NDTR0CS0Val);
    DclPrintf( "                                                  r                       r                    \n" );
    DclPrintf( "                                                  e                       e                    \n" );
    DclPrintf( "                                  t        t      s       t        t      s       t        t   \n" );
    DclPrintf( "                                  C        C      v       W        W      v       R        R   \n" );
    DclPrintf( " |---------reserved---------|  |--H--|  |--S--|  |--|  |--H--|  |--P--|  |--|  |--H--|  |--P--|\n" );
    DclPrintf( " 3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0  0\n" );
    DclPrintf( " 1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0\n" );
    DclPrintf( " ----------------------------  -------  -------  ----  -------  -------  ----  -------  -------\n" );

    DFCDumpBits(NDTR0CS0Val);
}

static void DFCDumpNDTR1CS0(
    volatile D_UINT32  *pulBase)
{
    D_UINT32            NDTR1CS0Val = PXA3XX_NDTR1CS0(pulBase);

    DclPrintf("NDTR1CS0 %lX contains:\n", NDTR1CS0Val);
    DclPrintf( " |---------------------tR---------------------|  |------reserved------|  |--tWHR--|  |--tAR---|\n" );
    DclPrintf( " 3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0  0\n" );
    DclPrintf( " 1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0\n" );
    DclPrintf( " ----------------------------------------------  ----------------------  ----------  ----------\n" );

    DFCDumpBits(NDTR1CS0Val);
}

static void DFCDumpNDCB0(
    volatile D_UINT32  *pulBase)
{
    D_UINT32            NDCB0Val = PXA3XX_NDCB0(pulBase);

    DclPrintf("NDCB0 %lX contains:\n", NDCB0Val);
    DclPrintf( "                   A        C              A                                                   \n" );
    DclPrintf( "                   U        M              D                                                   \n" );
    DclPrintf( "                   T        D              D                                                   \n" );
    DclPrintf( "                   O  C     |              R                                                   \n" );
    DclPrintf( "                   |  S     T        D     |               C                       C           \n" );
    DclPrintf( "                   R  E     Y     N  B     C               M                       M           \n" );
    DclPrintf( "                   S  L     P     C  C     Y               D                       D           \n" );
    DclPrintf( " |---reserved---|  -  -  |--E--|  -  -  |--C--|  |---------2----------|  |---------1----------|\n" );
    DclPrintf( " 3  3  2  2  2  2  2  2  2  2  2  2  1  1  1  1  1  1  1  1  1  1  0  0  0  0  0  0  0  0  0  0\n" );
    DclPrintf( " 1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0  9  8  7  6  5  4  3  2  1  0\n" );
    DclPrintf( " ----------------  -  -  -------  -  -  -  -  -  ----------------------  ----------------------\n" );

    DFCDumpBits(NDCB0Val);
}

static void DFCDump(
    volatile D_UINT32  *pulBase)
{
    DclPrintf("Dumping NAND controller registers:\n");
    DclPrintf("-----------------------------------\n");
    DFCDumpNDCR(pulBase);
    DFCDumpNDSR(pulBase);
    DFCDumpNDCB0(pulBase);
    DFCDumpNDTR0CS0(pulBase);
    DFCDumpNDTR1CS0(pulBase);
    DclPrintf("-----------------------------------\n\n");
}

#endif /* D_DEBUG */
#endif /* FFXCONF_NANDSUPPORT */

