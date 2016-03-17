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

    This module contains the "Command-Address-Data" NAND Technology Module
    (NTM) which supports NAND controllers with explicit registers for
    Commands and Address cycles, and either registers, FIFOs, buffers,
    or DMA access for Data cycles.  It interfaces with the FlashFX Hooks
    module fhcad.c, which allows easy project level customization of the
    interface without requiring a new NTM to be written.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntcad.c $
    Revision 1.47  2011/04/16 20:58:02Z  garyp
    Documentation and debug instrumentation cleanup -- no functional
    changes.
    Revision 1.46  2010/12/14 00:29:12Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.45  2010/12/09 23:04:09Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.44  2010/07/06 03:00:31Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.43  2010/01/27 04:27:36Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.42  2009/12/11 20:51:27Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Updated some typecasts which are necessary while migrating
    to the more flexible hooks model.
    Revision 1.41  2009/10/06 19:13:55Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.40  2009/08/02 21:42:19Z  garyp
    Fixed a broken variable name stemming from the previous checkin.
    Revision 1.39  2009/07/24 18:33:57Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written. Added a default IORequest()
    function.  Fixed SetBlockStatus() to return the proper operation type.
    Documentation updated.
    Revision 1.38  2009/04/09 03:42:56Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.37  2009/04/03 18:09:29Z  glenns
    - Added explanatory comment related to Bugzilla #2402.
    Revision 1.36  2009/04/01 21:22:18Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.35  2009/03/27 23:36:14Z  glenns
    - Fix minor bug in managing devices with 4KByte pages.
    Revision 1.34  2009/03/04 07:10:38Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    - Renamed a poorly-named variable.
    Revision 1.33  2009/02/26 22:21:35Z  glenns
    - Fixed compiler warning about function defined but not used.
    Revision 1.32  2009/02/26 22:01:58Z  glenns
    - Fixed merging mistake in last check-in
    Revision 1.31  2009/02/26 20:58:46Z  glenns
    - Bugzilla #2405: Overhaul of LFA code including:
      - Eliminating magic numbers and replacing them with macros or
        calculations.
      - Re-engineering the code to examine for multiple chips.
      - Renaming numerous variables to eliminate confusion with
        blocks and clarify purpose.
      - Updating function header comments to correspond with LFA
        code changes.
    Revision 1.30  2009/02/24 02:20:00Z  glenns
    - Implemented fix for Bugzilla #2431 by re-introducing small-block
      spare area access command removed in an earlier revision.
    - Fixed logical problem exposed by LFA changes in SetBlockAddress.
    - Changed hard-coded literal in several calculations with more
      meaningfully-named macro.
    - Fixed up some function header comments.
    - Removed some disabled code.
    Revision 1.29  2009/02/16 21:51:16Z  thomd
    Merge corrections to Large Flash Addressing from branch
    Revision 1.28  2009/02/11 21:59:39Z  glenns
    - In PageRead, moved decoding of tags to before checking for
      correctable errors; otherwise correctable error breaks out of loop
      before tags are checked and invalid tags are returned.
    - Fixed erroneous logic in BlockErase.
    Revision 1.27  2009/02/02 22:21:10Z  thomd
    For Large Flash Addressing, use 512 byte blocks to
    calculate limits and chip select.
    Revision 1.26  2009/01/27 20:33:25Z  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.25  2009/01/26 23:44:14Z  glenns
    - Modified to accommodate variable names changed to meet
      Datalight coding standards.
    - Updated PageRead to accomodate FFXSTAT_FIMCORRECTABLEDATA
      status.
    Revision 1.24  2009/01/23 17:19:45Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.23  2009/01/16 23:56:46Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in thirteen places.
    Revision 1.22  2008/09/02 05:59:47Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.21  2008/06/16 16:55:18Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.20  2008/05/03 21:26:25Z  garyp
    Minor documentation and debug code cleanup -- nothing functional.
    Revision 1.19  2008/03/14 18:49:54Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to
    take a tag length parameter.  Modified the GetPageStatus() functionality to
    return the tag length, if any.
    Revision 1.18  2008/02/05 22:35:33Z  billr
    Resolve bug 1854: ntcad.c fails to compile for 16-bit NAND interface.
    Revision 1.17  2008/01/30 22:25:30Z  Glenns
    Added procedures and calls to those procedures to automatically
    unlock blocks (for devices indicating they have a block locking
    mechanism) when an NTM is created.
    Revision 1.16  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/21 04:35:47Z  pauli
    Resolved Bug 1455.  Added the ability to report that BBM should or should
    not be used based on the characteristics of the NAND part identified.
    Updated trace output to not use hex formatting for pages/blocks.  Resolved
    a compiler warning in WriteSpareArea() for an unitialized variable.
    Revision 1.14  2007/09/13 21:54:46Z  Garyp
    Fixed some broken logic introduced in the previous revision.
    Revision 1.13  2007/09/13 01:35:26Z  Garyp
    Resolved bug 1430 where ECCs for 2KB pages could be calculated incorrectly
    on some hardware.  Resolved bug 1431 where chip select was being done
    incorrectly in some cases and would fail if multiple chips were in use.
    Resolved bug 1240 (introduced during LFA modifications).  Cleaned up the
    code path when using 2KB pages to avoid some unnecessary (but benign)
    operations which are only needed for 512-byte emulation.  Implemented the
    RawPageRead/Write() functionality.  Factored out a bunch of common code
    into helper functions.  Removed a number of unnecessary chip selects, one
    of which was probably causing certain "ResetBeforeProgram" ST parts to
    appear not to work.
    Revision 1.12  2007/08/16 17:19:55Z  garyp
    Abstracted the NTM name to make it easier to compare different NTMs.
    Minor documentation fixes.  No functional changes.
    Revision 1.11  2007/08/02 22:42:32Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.10  2007/04/07 03:26:12Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.9  2007/03/27 20:14:38Z  rickc
    Fix for bug 928.  Removed requirement that structure had to match on-media
    spare area format.
    Revision 1.8  2007/03/01 20:46:00Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.7  2007/02/28 02:28:36Z  pauli
    Resolve bug 920.  This NTM will now properly program the spare area
    when requested to write ONLY the spare area.
    Revision 1.6  2007/02/13 23:14:44Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.
    Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to allow the call
    tree all the way up through the IoRequest to avoid having to range check
    (and/or split) requests.  Removed corresponding casts.
    Revision 1.5  2007/02/07 21:08:31Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.4  2007/01/04 00:09:00Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.3  2006/11/08 03:38:48Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.2  2006/09/07 00:22:10Z  rickc
    Removed unneeded calls to BENDSWAP
    Revision 1.1  2006/07/10 22:38:42Z  Garyp
    Initial revision
    Revision 1.5  2006/07/09 02:39:01Z  Garyp
    Debug code updated.
    Revision 1.4  2006/07/06 00:19:49Z  Garyp
    Updated so the NTCAD hooks use a Create/Destroy metaphor in similar fashion
    to most everything else.  Modified the hooks interface to use standard
    FFXSTATUS return values rather than the old FIMResult_t values, and
    eliminated the need to translate return values back and forth, and thereby
    cleaned up a lot of related logic throughout the NTM.
    Revision 1.3  2006/05/17 22:02:51Z  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    NtmInfo structure.
    Revision 1.2  2006/05/09 01:25:05Z  timothyj
    Added casts for the lengths past to the data in and data out functions.
    Previously was causing a warning on some compilers since the result of
    the computation embedded in the macros was promoted to a wider type.
    Revision 1.1  2006/05/08 19:16:42Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <deverr.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include "nandid.h"
#include "nand.h"
#include "ntm.h"


#define NTMNAME "NTCAD"

/*-------------------------------------------------------------------
    Set this value to TRUE when using 2KB-page flash and you want to
    emulate 512-byte pages.  Imposes a performance hit, so do this
    only when the file-system driver doesn't support 2KB sectors.

    Also note that you WILL need to FORCE a BBM reformat when
    switching between emulated and non-emulated mode.
-------------------------------------------------------------------*/
#define EMULATE_512B_PAGES  FALSE   /* Must be FALSE for checked in code */


struct tagNTMDATA
{
    D_UINT16        uPhysPageSize;
    D_UINT16        uPhysSpareSize;
    D_UINT16        uLinearPageAddrMSB;  /* how much to >> linear address to get chip page index  */
    D_UINT16        uChipPageIndexLSB;   /* how much to << chip page index before sending to chip */
    D_UINT16        uLinearBlockAddrLSB; /* how much to >> linear address to get block index */
    D_UINT16        uLinearChipAddrMSB;  /* how much to >> linear address to get chip select */
/*  FFXSTATUS       (*WriteSkipSpare)(struct tagNTMDATA *pNTM, D_UINT32 ulOffset); */
    FFXSTATUS       (*WriteSeekSpare)(struct tagNTMDATA *pNTM, D_UINT16 uOffset, D_UINT16 uLogicalPage, D_UINT16 uSpareSize);
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE    hEI;
  #endif
    NTMINFO         NtmInfo;            /* information visible to upper layers */
    const FFXNANDCHIP   *pChipInfo;     /* chip information                    */
    FFXDEVHANDLE    hDev;
    PNANDCTL        pNC;
    unsigned        uTagOffset;         /* Offset in spare area for the tag     */
    unsigned        uFlagsOffset;       /* Offset in spare area for the flags   */
    unsigned        uECC1Offset;        /* Offset in spare area for the ECC1    */
    unsigned        uECC2Offset;        /* Offset in spare area for the ECC2    */
};

/*  number of reads/writes needed for 1) a 512B logical page, 2) a 16B logical
    spare area and 3) a VBF allocation record
*/
#define TAG_ECCCHECKSIZE    (2)     /* two bytes for ECC and check byte */
#define DATA_IO_COUNT       (hNTM->NtmInfo.uPageSize / sizeof(NAND_IO_TYPE))
#define SEGMENT_IO_COUNT    (NAND512_PAGE / sizeof(NAND_IO_TYPE))
#define SPARE_IO_COUNT      (hNTM->NtmInfo.uSpareSize / sizeof(NAND_IO_TYPE))
#define SPARE16_IO_COUNT    (NAND512_SPARE / sizeof(NAND_IO_TYPE))
/* #define TAG_IO_COUNT        ((hNTM->NtmInfo.uTagSize + TAG_ECCCHECKSIZE) / sizeof(NAND_IO_TYPE)) */
#define PAD_VALUE           ((NAND_IO_TYPE) ~((NAND_IO_TYPE) 0))

#if EMULATE_512B_PAGES
  #define PHYSSPARE_IO_COUNT (hNTM->uPhysSpareSize / sizeof(NAND_IO_TYPE))
#endif

/*  The following note only applies to Toshiba and Samsung 64Mbit
    flash devices with a weekly code less than 9828.

    When writing data into the 64Mbit flash devices from Toshiba
    and Samsung, the flash memory will sometimes report success
    via the status and the RDY/#BSY signal before the
    write is actually completed.  The data does not get
    programmed and a read back verify will show that the data
    is not valid.

    Under normal circumstances a write will take 170 micro-
    seconds, when this problem occurs, the write only takes
    about 100 micro-seconds.  If the flash part is heated
    up slightly, the ready signal will assert sooner and
    sooner eventually causing the write to fail.  Rates as
    fast as 60 micro-seconds have been measured.

    One work-around has been to always program at least ten
    bits of data with each re-program.  This appears to avoid
    the problem about 90% of the time but still re-surfaces.
    Heating up the part when multi-bit programs are made, does
    not make as big of a difference in the program speed.

    The part has been run with 3v and 5v logic; the flash
    parts themselves have been run with signals at 5v and
    the chip at 3v.  The problem has been demonstrated on
    three different hardware OSs.

    04/28/98 DS/PKG/AP/DM/LT
*/

/*  The following macros define interfaces to the various hook functions
    implemented in fhcad.c.
*/
#define SET_CHIP_SELECT(x, dir)     (FfxHookNTCADSetChipSelect(hNTM->pNC, (x) >> (hNTM->uLinearChipAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT), (dir)))
#define CHIP_READY_WAIT()           (FfxHookNTCADReadyWait(hNTM->pNC))
#define SET_CHIP_COMMAND(x)         (FfxHookNTCADSetChipCommand(hNTM->pNC, (x)))
#define SET_CHIP_COMMAND_NOWAIT(x)  (FfxHookNTCADSetChipCommandNoWait(hNTM->pNC, (x)))
#define NAND_DATA_IN(x,y)           (FfxHookNTCADDataIn(hNTM->pNC, (x), (y)))
#define NAND_STATUS_IN()            (FfxHookNTCADStatusIn(hNTM->pNC))
#define NAND_DATA_OUT(x,y)          (FfxHookNTCADDataOut(hNTM->pNC, (x), (y)))
#define NAND_ADDR_OUT(addr)         (FfxHookNTCADAddressOut(hNTM->pNC, (addr)))
/* #define WRITE_SKIP_SPARE(x)         (hNTM->WriteSkipSpare(hNTM, (x))) */
#define WRITE_SEEK_SPARE(x,y,z)     (hNTM->WriteSeekSpare(hNTM, (x),(y),(z)))

/*  Internal helper routines used primarily by the new FIM interface routines
*/
static FFXSTATUS    ReadID(                   NTMHANDLE hNTM, D_UCHAR * pucID, D_UINT32 ulOffsetAddressableUnit);
static FFXSTATUS    ResetChipAndWait(         NTMHANDLE hNTM);
static void         AdjustPhysicalPageOffset( NTMHANDLE hNTM, D_UINT16 uOffset);
static void         SetLogicalPageAddress(    NTMHANDLE hNTM, D_UINT32 ulAddressableUnitOffset);
static void         SetBlockAddress(          NTMHANDLE hNTM, D_UINT32 ulAddressableUnitOffset);
static FFXSTATUS    SetReadChipStatus(        NTMHANDLE hNTM, D_UINT32 ulAbsoluteOffsetAddressableUnit);
static FFXSTATUS    GetError(                 NTMHANDLE hNTM, D_UINT32 ulAbsoluteOffsetAddressableUnit);
static FFXSTATUS    ExecuteChipCommand(       NTMHANDLE hNTM, D_UCHAR ucCommand, D_UINT32 ulAbsoluteOffsetAddressableUnit);
/* static FFXSTATUS WriteSkipSamsungSpare(    NTMHANDLE hNTM, D_UINT32 ulOffset); */
/* static FFXSTATUS WriteSkipToshibaSpare(    NTMHANDLE hNTM, D_UINT32 ulOffset); */
static FFXSTATUS    WriteSeekSamsungSpare(    NTMHANDLE hNTM, D_UINT16 uOffset, D_UINT16 uLogicalPage, D_UINT16 uSpareSize);
static FFXSTATUS    WriteSeekToshibaSpare(    NTMHANDLE hNTM, D_UINT16 uOffset, D_UINT16 uLogicalPage, D_UINT16 uSpareSize);
static FFXIOSTATUS  ReadSpareArea(            NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(           NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXSTATUS    RandomReadSetup(          NTMHANDLE hNTM, D_UINT32 ulAddressableUnitOffset, unsigned nSpareOffset);
static D_UINT32     UpperBoundProximity(      D_UINT32  ulTotalAddressableUnits, D_UINT32 ulUpperBoundKB);

/*  The following procedure is used only in debug configurations:
*/
#if D_DEBUG
static D_UINT32     AddressableUnitToKB(      D_UINT32  ulAddressableUnit);
#endif

#if EMULATE_512B_PAGES
  static FFXIOSTATUS ReadPhysSpareArea(       NTMHANDLE hNTM, D_UINT32 ulLogicalPage, D_BUFFER *pSpare);
  static FFXIOSTATUS WritePhysSpareArea(      NTMHANDLE hNTM, D_UINT32 ulLogicalPage, const D_BUFFER *pSpare);
#endif
static FFXSTATUS    UnlockBlocks(NTMHANDLE hNTM, D_UINT32 ulStartBlock, D_UINT32 ulEndBlock);

/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_cad =
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
static NTMHANDLE Create(
    FFXDEVHANDLE        hDev,
    const NTMINFO     **ppNtmInfo)
{
    NTMDATA            *pNTM = NULL;
    D_UCHAR             aucID[NAND_ID_SIZE];
    D_UINT32            ulTotalAddressableUnits;
    D_UINT32            ulDeviceSizeInAddressableUnits;
    D_UINT32            ulTotalBlocks;
    D_BOOL              fSuccess = FALSE;
    FFXFIMBOUNDS        bounds;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    pNTM->pNC = FfxHookNTCADCreate(hDev);
    if(!pNTM->pNC)
        goto CreateCleanup;

    /*  Get the array bounds now so we know how far to scan
    */
    FfxDevGetArrayBounds(hDev, &bounds);

    /*  Identify the first chip in the array. Note that the ReadID
        function uses the SET_CHIP_SELECT macro, which uses the
        pNTM->uLinearChipAddrMSB value, which is at this point in
        the code set to zero. This has the potential to generate
        some pretty serious miscalculations if you try to call this
        macro with an address other than zero, so be sure to use
        zero here.
    */
    if(ReadID(pNTM, aucID, 0) != FFXSTAT_SUCCESS)
        goto CreateCleanup;

    pNTM->pChipInfo = FfxNandDecodeID(aucID);
    if (!pNTM->pChipInfo)
        goto CreateCleanup;

  #if !EMULATE_512B_PAGES
    if(pNTM->pChipInfo->pChipClass->uPageSize > FFX_NAND_MAXPAGESIZE)
    {
        DclPrintf("FFX: FFX_NAND_MAXPAGESIZE is %u, but the flash detected has a page size of %U\n",
            FFX_NAND_MAXPAGESIZE, pNTM->pChipInfo->pChipClass->uPageSize);

        goto CreateCleanup;
    }
  #endif

    /*  Addressable unit size must not be larger than a page:
    */
    DclAssert(BYTES_PER_ADDRESSABLE_UNIT <= pNTM->pChipInfo->pChipClass->uPageSize);

    if( pNTM->pChipInfo->pChipClass->uEdcRequirement > 1 )
    {
        /* This chip requires more correction capabilities
           than this NTM can handle - fail the create.
        */
        DclPrintf("FFX: Insufficient EDC capabilities.\n");
        goto CreateCleanup;
    }


    /*  Move these into the main NTM structure for easier access
    */
    pNTM->uLinearPageAddrMSB  = pNTM->pChipInfo->pChipClass->uLinearPageAddrMSB;
    pNTM->uChipPageIndexLSB   = pNTM->pChipInfo->pChipClass->uChipPageIndexLSB;
    pNTM->uLinearBlockAddrLSB = pNTM->pChipInfo->pChipClass->uLinearBlockAddrLSB;
    pNTM->uLinearChipAddrMSB  = pNTM->pChipInfo->pChipClass->uLinearChipAddrMSB;

    ulDeviceSizeInAddressableUnits = pNTM->pChipInfo->pChipClass->ulChipBlocks *
                                        (pNTM->pChipInfo->pChipClass->ulBlockSize /
                                        BYTES_PER_ADDRESSABLE_UNIT);
    ulTotalAddressableUnits = ulDeviceSizeInAddressableUnits;

    DclAssert(ulTotalAddressableUnits);

    /*  Loop through all possible additional chips (this may be none).
        They must all match the first chip.  Leave ulTotalAddressableUnits set to
        the address after the last chip detected (the total combined
        size of all the chips).
    */
    for (;;)
    {
        /*  Try to read the ID of the next possible chip.  If this
            fails, or if the chip info doesn't match, quit with
            what was found so far.
        */
        if (ReadID(pNTM, aucID, ulTotalAddressableUnits) != FFXSTAT_SUCCESS)
            break;

        if (pNTM->pChipInfo != FfxNandDecodeID(aucID))
            break;

        /*  Accumulate the total size.  Stop if adding another chip
            would cause overflow.
        */
        if(ulDeviceSizeInAddressableUnits > UpperBoundProximity(ulTotalAddressableUnits, bounds.ulMaxScanTotalKB))
            break;

        ulTotalAddressableUnits += ulDeviceSizeInAddressableUnits;
    }

    if(pNTM->pChipInfo->pChipClass->Samsung2kOps)
    {
/*        pNTM->WriteSkipSpare = WriteSkipSamsungSpare; */
        pNTM->WriteSeekSpare = WriteSeekSamsungSpare;
    }
    else
    {
/*        pNTM->WriteSkipSpare = WriteSkipToshibaSpare; */
        pNTM->WriteSeekSpare = WriteSeekToshibaSpare;
    }

    /*  Determine which spare area format to use.
    */
    switch(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo))
    {
        case NSF_OFFSETZERO:
        {
            DclAssert(NSOFFSETZERO_TAG_LENGTH == LEGACY_ENCODED_TAG_SIZE);

            pNTM->uTagOffset    = NSOFFSETZERO_TAG_OFFSET;
            pNTM->uFlagsOffset  = NSOFFSETZERO_FLAGS_OFFSET;
            pNTM->uECC1Offset   = NSOFFSETZERO_ECC1_OFFSET;
            pNTM->uECC2Offset   = NSOFFSETZERO_ECC2_OFFSET;
            break;
        }

        case NSF_SSFDC:
        {
            pNTM->uTagOffset    = NSSSFDC_TAG_OFFSET;
            pNTM->uFlagsOffset  = NSSSFDC_FLAGS_OFFSET;
            pNTM->uECC1Offset   = NSSSFDC_ECC1_OFFSET;
            pNTM->uECC2Offset   = NSSSFDC_ECC2_OFFSET;
            break;
        }

        default:
        {
            DclPrintf("FFX: The detected part requires an unsupported spare area format.\n");
            goto CreateCleanup;
        }
    }

    /*  Save the native page/spare size
    */
    pNTM->uPhysPageSize = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->uPhysSpareSize = pNTM->pChipInfo->pChipClass->uSpareSize;

  #if EMULATE_512B_PAGES
    if(pNTM->uPhysPageSize == NAND2K_PAGE)
    {
        pNTM->NtmInfo.uPageSize = NAND512_PAGE;
        pNTM->NtmInfo.uSpareSize = NAND512_SPARE;
    }
    else
  #endif
    {
        pNTM->NtmInfo.uPageSize = pNTM->uPhysPageSize;
        pNTM->NtmInfo.uSpareSize = pNTM->uPhysSpareSize;
    }

    pNTM->NtmInfo.ulBlockSize = pNTM->pChipInfo->pChipClass->ulBlockSize;

    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;
    
    /*  The following lines of code set  the NTM EDC capability and segment
        size based on the standard 1-bit EDC available through the nthelp
        facility. If other EDC is implemented in this NTM or its hooks,
        this code will need to be updated.
    */
    pNTM->NtmInfo.uEdcCapability     = 1;
    pNTM->NtmInfo.uEdcSegmentSize    = DATA_BYTES_PER_ECC;

    ulTotalBlocks = BYTES_PER_ADDRESSABLE_UNIT * (ulTotalAddressableUnits / pNTM->NtmInfo.ulBlockSize);

    pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;

    /*  Calculate this once and store it for use later for
        both optimization and simplification of the code
    */
    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

    /*  REFACTOR: The line of code below will unlock all the blocks in a chip or
        array of chips. Unlocking blocks that may end up in a reserved area goes
        against the principle of having a reserved area, but in the absence of a
        FIM-level interface for managing block-locking mechanisms, we will
        simply have to trust that the device manager does not attempt to write
        or erase blocks that it has placed in reserve.
    */
    if (UnlockBlocks(pNTM, 0, pNTM->NtmInfo.ulTotalBlocks-1) != FFXSTAT_SUCCESS)
        goto CreateCleanup;

    pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;
    pNTM->NtmInfo.uMetaSize = FFX_NAND_TAGSIZE;
    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;
    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

    FFXPRINTF(1, (NTMNAME": TotalSize=%lXKB ChipBlocks=%lX BlockSize=%lX PageSize=%X SpareSize=%X\n",
        AddressableUnitToKB(ulTotalAddressableUnits),
        pNTM->NtmInfo.ulChipBlocks,
        pNTM->NtmInfo.ulBlockSize,
        pNTM->uPhysPageSize,
        pNTM->uPhysSpareSize));

  #if EMULATE_512B_PAGES
    if(pNTM->uPhysPageSize != pNTM->NtmInfo.uPageSize)
    {
        FFXPRINTF(1, (NTMNAME": Emulated PageSize=%X Emulated SpareSize=%X\n",
            pNTM->NtmInfo.uPageSize,
            pNTM->NtmInfo.uSpareSize));
    }
  #endif

    /*  Make sure the parent has a pointer to our NTMINFO structure
    */
    *ppNtmInfo = &pNTM->NtmInfo;

    fSuccess = TRUE;

  CreateCleanup:

    if(pNTM && !fSuccess)
    {
        if(pNTM->pNC)
            FfxHookNTCADDestroy(pNTM->pNC);

        DclMemFree(pNTM);
        pNTM = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-Create() returning %P\n", pNTM));

    return pNTM;
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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-Destroy()\n"));

    DclAssert(hNTM);

    FFXERRORINJECT_DESTROY();

    FfxHookNTCADDestroy(hNTM->pNC);

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
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    D_BUFFER            abECC[MAX_ECC_BYTES_PER_PAGE];
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddressableUnitAddr;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageRead() StartPage=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageRead", 0, ulCount);

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

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulAddressableUnitAddr, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    while(ulCount)
    {
        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);
        if(hNTM->pChipInfo->pChipClass->ReadConfirm)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        /*  Read a page of data, assume success
        */
        ioStat.ffxStat = CHIP_READY_WAIT();
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if(fUseEcc)
        {
            int         seg;
            D_BUFFER   *pTmpBuffer = pPages;

            for(seg=0; seg<hNTM->NtmInfo.uPageSize/NAND512_PAGE; seg++)
            {
                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                    NTMNAME"-PageRead() reading page segment %u (%U %u-byte elements)\n",
                    seg, SEGMENT_IO_COUNT, sizeof(NAND_IO_TYPE)));

                /*  An NTMHOOKHANDLE is the new paradigm, but essentially
                    operates in the same fashion as the older PNANDCTL 
                    parameter.  Once this NTM is updated to the more 
                    flexible configuration model, the cast below can be
                    removed.
                */                
                FfxHookEccCalcStart((NTMHOOKHANDLE)hNTM->pNC, pTmpBuffer, NAND512_PAGE, &abECC[seg*2*BYTES_PER_ECC], ECC_MODE_READ);

                NAND_DATA_IN((NAND_IO_TYPE*)pTmpBuffer, (D_UINT16)SEGMENT_IO_COUNT);

                FfxHookEccCalcRead((NTMHOOKHANDLE)hNTM->pNC, pTmpBuffer, NAND512_PAGE, &abECC[seg*2*BYTES_PER_ECC], ECC_MODE_READ);

                pTmpBuffer += NAND512_PAGE;
            }
        }
        else
        {
            DclAssert(pTags == NULL);
            DclAssert(nTagSize == 0);

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-PageRead() reading page data (%U %u-byte elements)\n",
                DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

            NAND_DATA_IN((NAND_IO_TYPE*)pPages, (D_UINT16)DATA_IO_COUNT);
        }

#if EMULATE_512B_PAGES
        if(hNTM->uChipPageIndexLSB > 8) /* 2KB page chip? */
        {
            D_UINT16    uSpareOffset;

            /*  Where in this physical page is the appropriate spare area?
            */
            DclAssert(hNTM->uPhysPageSize == NAND2K_PAGE);
            uSpareOffset = (D_UINT16)((ulAddressableUnitAddr &
                ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));
            uSpareOffset *= hNTM->NtmInfo.uSpareSize;
            uSpareOffset += hNTM->uPhysPageSize;
            uSpareOffset >>= NAND_DATA_WIDTH_POW2;

            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                AdjustPhysicalPageOffset(hNTM, uSpareOffset);
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;
            }
        }
#endif

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-PageRead() reading spare data (%U %u-byte elements)\n",
            SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

        /*  Read the spare area
        */
        NAND_DATA_IN((NAND_IO_TYPE*)spare.data, (D_UINT16)SPARE_IO_COUNT);

        if(fUseEcc)
        {
            FFXIOSTATUS ioValid;
/*
            FFXERRORINJECT_READ("PageRead", ulPage, pPages, hNTM->NtmInfo.uPageSize, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
*/
            /*  An NTMHOOKHANDLE is the new paradigm, but essentially operates
                in the same fashion as the older PNANDCTL parameter.  Once this
                NTM is updated to the more flexible configuration model, the
                cast below can be removed.
            */                
            ioValid = FfxHookEccCorrectPage((NTMHOOKHANDLE)hNTM->pNC, pPages, spare.data, abECC, hNTM->pChipInfo);

            ioStat.ffxStat = ioValid.ffxStat;
            ioStat.op.ulPageStatus |= ioValid.op.ulPageStatus;

            if(nTagSize)
            {
                DclAssert(pTags);
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                FfxNtmHelpTagDecode(pTags, &spare.data[hNTM->uTagOffset]);

                pTags += nTagSize;
            }

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
                    ioStat.ulCount++;
                break;
            }

        }

        ioStat.ulCount++;
        ulCount--;
        pPages          += hNTM->NtmInfo.uPageSize;
        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
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
    D_UINT32            ulPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddressableUnitAddr;
    D_UINT32            ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageWrite() PageNum=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    /*  We will be operating within one chip (within one block as a
        matter of fact).
    */
    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    ulAddressableUnitAddr =ulOffsetAddressableUnit;
    while(ulCount)
    {
        DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
        D_BUFFER           *pNS = spare.data;

        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Send the programming command and address for the data page
        */
        if(hNTM->uPhysPageSize == NAND512_PAGE)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        if(hNTM->pChipInfo->pChipClass->ResetBeforeProgram)
        {
            ioStat.ffxStat = ResetChipAndWait(hNTM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

            DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);

            if(fUseEcc)
            {
                int     seg;

                if(pTags)
                {
                    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                    FfxNtmHelpTagEncode(&pNS[hNTM->uTagOffset], pTags);
                }

                for(seg=0; seg<hNTM->NtmInfo.uPageSize/NAND512_PAGE; seg++)
                {
                    D_BUFFER    abECC[2*BYTES_PER_ECC];

                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                        NTMNAME"-PageWrite() writing page segment %u (%U %u-byte elements)\n",
                        seg, SEGMENT_IO_COUNT, sizeof(NAND_IO_TYPE)));

                    /*  An NTMHOOKHANDLE is the new paradigm, but essentially
                        operates in the same fashion as the older PNANDCTL 
                        parameter.  Once this NTM is updated to the more 
                        flexible configuration model, the cast below can be
                        removed.
                    */                
                    FfxHookEccCalcStart((NTMHOOKHANDLE)hNTM->pNC, pPages, NAND512_PAGE, abECC, ECC_MODE_WRITE);

                    NAND_DATA_OUT((NAND_IO_TYPE*)pPages, (D_UINT16)SEGMENT_IO_COUNT);

                    FfxHookEccCalcRead((NTMHOOKHANDLE)hNTM->pNC, pPages, NAND512_PAGE, abECC, ECC_MODE_WRITE);

                    /*  Store the calculated ECC values in the appropriate
                        place in the spare area.
                    */
                    pNS[hNTM->uFlagsOffset] = LEGACY_WRITTEN_WITH_ECC;

                    pNS[hNTM->uECC1Offset + 0] = abECC[0];
                    pNS[hNTM->uECC1Offset + 1] = abECC[1];
                    pNS[hNTM->uECC1Offset + 2] = abECC[2];

                    pNS[hNTM->uECC2Offset + 0] = abECC[3];
                    pNS[hNTM->uECC2Offset + 1] = abECC[4];
                    pNS[hNTM->uECC2Offset + 2] = abECC[5];

                    pNS += NAND512_SPARE;
                    pPages += NAND512_PAGE;
                 }
            }
            else
            {
                DclAssert(pTags == NULL);
                DclAssert(nTagSize == 0);

                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                    NTMNAME"-PageWrite() writing page (%U %u-byte elements)\n",
                    DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

                NAND_DATA_OUT((NAND_IO_TYPE*)pPages, (D_UINT16)DATA_IO_COUNT);
            }

          #if EMULATE_512B_PAGES
            if(hNTM->uChipPageIndexLSB > 8)     /* 2KB page chip? */
            {
                D_UINT16    uLogicalPage;

                /*  Where in this physical page is the appropriate logical page?
                */
                uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));

                /*  Seek from the end of this logical main page.  This typecast
                    is safe because the page number multipied by the standard
                    NAND data size is a maximum of 2048.
                */
                ioStat.ffxStat = WRITE_SEEK_SPARE(
                    (D_UINT16)((uLogicalPage + 1) * hNTM->NtmInfo.uPageSize),
                    uLogicalPage, hNTM->NtmInfo.uSpareSize);

                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;
            }
          #endif

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-PageWrite() writing spare data (%U %u-byte elements)\n",
                SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

            /*  Write the actual metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT((NAND_IO_TYPE*)spare.data, (D_UINT16)SPARE_IO_COUNT);

            /*  Now actually program the data..
            */
            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulAddressableUnitAddr);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("PageWrite",
                             ulAddressableUnitAddr / (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT),
                             &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if(nTagSize)
            pTags       += nTagSize;
        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        ulCount--;
    }

/*  PageWriteCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    ToDo:
      This could probably be refactored to share a lot of code
      with PageRead().

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to any
                  reserved space.
        pPages  - A buffer to receive the main page data.
        pSpares - A buffer to receive the spare area data.
                  May be NULL.
        ulCount - The number of pages to read.  The range of
                  pages must not cross an erase block boundary.

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddressableUnitAddr;
    D_UINT32            ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageRead() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    while(ulCount)
    {
        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);
        if(hNTM->pChipInfo->pChipClass->ReadConfirm)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        /*  Read a page of data, assume success
        */
        ioStat.ffxStat = CHIP_READY_WAIT();
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-RawPageRead() reading page data (%U %u-byte elements)\n",
            DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

        NAND_DATA_IN((NAND_IO_TYPE *) pPages, (D_UINT16) DATA_IO_COUNT);

        if(pSpares)
        {
          #if EMULATE_512B_PAGES
            if(hNTM->uChipPageIndexLSB > 8) /* 2KB page chip? */
            {
                D_UINT16    uSpareOffset;

                /*  Where in this physical page is the appropriate spare area?
                */
                DclAssert(hNTM->uPhysPageSize == NAND2K_PAGE);
                uSpareOffset = (D_UINT16)((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));
                uSpareOffset *= hNTM->NtmInfo.uSpareSize;
                uSpareOffset += NAND2K_PAGE;
                uSpareOffset >>= NAND_DATA_WIDTH_POW2;

                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    AdjustPhysicalPageOffset(hNTM, uSpareOffset);
                    ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
                    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                        break;
                }
            }
          #endif

            /*  Read the spare area
            */
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-RawPageRead() reading spare data (%U %u-byte elements)\n",
                SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

            NAND_DATA_IN((NAND_IO_TYPE *) pSpares, (D_UINT16) SPARE_IO_COUNT);

            pSpares += hNTM->NtmInfo.uSpareSize;
        }

        ioStat.ulCount++;
        ulCount--;
        pPages          += hNTM->NtmInfo.uPageSize;
        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
    }

  PageReadCleanup:

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

    ToDo:
      This could probably be refactored to share a lot of code
      with PageWrite().

    Parameters:
        hNTM    - The NTM handle to use
        ulPage  - The flash offset in pages, relative to any
                  reserved space.
        pPages  - A pointer to the main page data to write.
        pSpares - A pointer to data to write in the spare area.
                  This value may be NULL if nothing is to be
                  written into the spare area.
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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddressableUnitAddr;
    D_UINT32            ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageWrite() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  We will be operating within one chip (within one block as a
        matter of fact.
    */
    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    while(ulCount)
    {
        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Send the programming command and address for the data page
        */
        if(hNTM->uPhysPageSize == NAND512_PAGE)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        if(hNTM->pChipInfo->pChipClass->ResetBeforeProgram)
        {
            ioStat.ffxStat = ResetChipAndWait(hNTM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-RawPageWrite() writing page data (%U %u-byte elements)\n",
            DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

        NAND_DATA_OUT((NAND_IO_TYPE *) pPages, (D_UINT16) DATA_IO_COUNT);

        if(pSpares)
        {
          #if EMULATE_512B_PAGES
            if(hNTM->uChipPageIndexLSB > 8)     /* 2KB page chip? */
            {
                D_UINT16    uLogicalPage;

                /*  Where in this physical page is the appropriate logical page?
                */
                uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));

                /*  Seek from the end of this logical main page.  This typecast
                    is safe because the page number multipied by the standard
                    NAND data size is a maximum of 2048.
                */
                ioStat.ffxStat = WRITE_SEEK_SPARE(
                    (D_UINT16)((uLogicalPage + 1) * hNTM->NtmInfo.uPageSize),
                    uLogicalPage, hNTM->NtmInfo.uSpareSize);

                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;
            }
          #endif

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-RawPageWrite() writing spare data (%U %u-byte elements)\n",
                SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

            /*  Write the actual metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT((NAND_IO_TYPE *) pSpares, (D_UINT16) SPARE_IO_COUNT);

            pSpares += hNTM->NtmInfo.uSpareSize;
        }
        else
        {
            DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

            /*  We were not supplied any data to write into the spare
                area, so initialize the transfer buffer with all 0xFFs
            */
            DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);

            NAND_DATA_OUT((NAND_IO_TYPE *) spare.data, (D_UINT16) SPARE_IO_COUNT);
        }

        /*  Now actually program the data..
        */
        ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulOffsetAddressableUnit);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("RawPageWrite",
                             ulAddressableUnitAddr / (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT),
                             &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        pPages          += hNTM->NtmInfo.uPageSize;
        ulCount--;
    }

/*  RawPageWriteCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    D_BUFFER       *pSpare)
{
    FFXIOSTATUS     ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-SpareRead() Page=%lX\n", ulPage));

    ioStat = ReadSpareArea(hNTM, ulPage, pSpare);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        NTMNAME"-SpareRead() Page=%lX returning %s\n",
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
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-SpareWrite() Page=%lX\n", ulPage));

    ioStat = WriteSpareArea(hNTM, ulPage, pSpare);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenRead()

    Read the hidden (tag) data from multiple pages.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The flash offset in pages, relative to any
                   reserved space.
        ulCount  - The number of tags to read.
        pTags    - Buffer for the hidden data read.
        nTagSize - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenRead(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    D_UINT32        ulCount,
    D_BUFFER       *pTags,
    unsigned        nTagSize)
{
    D_UINT32        ulAddressableUnitAddr;
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32        ulOffsetAddressableUnit;
    D_UINT32        ulLength;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenRead() StartPage=%lX Count=%lX TagSize=%u\n", ulPage, ulCount, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);
    DclAssert(ulCount * nTagSize <= D_UINT16_MAX);
    ulLength = ulCount * nTagSize;

    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto HiddenReadCleanup;

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  NOTE: When emulating 512B pages, we probably only need to read
        a 2KB page once to get up to 4 tags.
    */

    while(ulLength)
    {
        DCLALIGNEDBUFFER    (spare, data, NAND512_SPARE);
        NAND_IO_TYPE       *pIO = (NAND_IO_TYPE *)spare.data;

        ioStat.ffxStat = RandomReadSetup(hNTM, ulAddressableUnitAddr, 0);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Read the first 16 bytes of the spare area since we know the
            tag will be somewhere in there.
        */
        NAND_DATA_IN(pIO, SPARE16_IO_COUNT);
        pIO += SPARE16_IO_COUNT;

        /*  TODO: Could be injecting the error into a portion of the spare
            area besides where the tag is, useless, but benign.
        */
        FFXERRORINJECT_READ("HiddenRead",
                            ulAddressableUnitAddr / (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT),
                            spare.data,
                            sizeof spare.data,
                            &ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FfxNtmHelpTagDecode(pTags, &spare.data[hNTM->uTagOffset]);

        ioStat.ulCount++;

        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        ulLength         -= nTagSize;
        pTags           += nTagSize;
    }

  HiddenReadCleanup:

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
    D_UINT32            ulAddressableUnitAddr;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (spare, data, NAND512_SPARE);
    NAND_IO_TYPE       *pIO;
    D_UINT32            ulOffsetAddressableUnit;

    pIO = (NAND_IO_TYPE *)spare.data;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenWrite", 0, 0);
    
    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

        DclMemSet(spare.data, ERASED8, sizeof spare.data);

        FfxNtmHelpTagEncode(&spare.data[hNTM->uTagOffset], pTag);

        if(hNTM->uChipPageIndexLSB <= 8)     /* 512b page chip? */
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_SPARE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                goto HiddenWriteCleanup;
        }

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

            if(hNTM->uChipPageIndexLSB > 8)     /* 2KB page chip? */
            {
                D_UINT16    uLogicalPage;

                /*  Where in this physical page is the appropriate logical page?
                */
                uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));

                /*  Seek from the start of this logical main page.  This
                    typecast is safe because the page number multipied by the
                    standard NAND data size is a maximum of 1536.
                */
                ioStat.ffxStat = WRITE_SEEK_SPARE((D_UINT16)(uLogicalPage * hNTM->NtmInfo.uPageSize),
                                                  uLogicalPage, hNTM->NtmInfo.uSpareSize);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                {
                    goto HiddenWriteCleanup;
                }
            }

            /*  Write first 16 bytes of the spare area since the tag will
                always be there regardless of page size.
            */
            NAND_DATA_OUT(pIO, SPARE16_IO_COUNT);

            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulOffsetAddressableUnit);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                ioStat.ulCount = 1;

                FFXERRORINJECT_WRITE("HiddenWrite",
                                     BYTES_PER_ADDRESSABLE_UNIT * ulOffsetAddressableUnit / hNTM->NtmInfo.uPageSize,
                                     &ioStat);
            }
        }
    }

  HiddenWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: BlockErase()

    This function erases a block (physical erase zone) of a
    NAND chip.

    Parameters:
        hNTM    - The NTM handle
        ulBlock - The flash offset in blocks

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of blocks which were completely erased.
        The status indicates whether the erase was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS BlockErase(
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock)
{
    D_UINT32        ulAddressableUnitAddr;
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;
    D_UINT32        ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-BlockErase() Block=%lX\n", ulBlock));

    DclProfilerEnter(NTMNAME"-BlockErase", 0, 0);

    ulOffsetAddressableUnit = ulBlock * (hNTM->NtmInfo.ulBlockSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ulAddressableUnitAddr = ulBlock * (hNTM->NtmInfo.ulBlockSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

        /*  Tell the flash to erase those blocks now
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_ERASE_BLOCK);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            SetBlockAddress(hNTM, ulAddressableUnitAddr);

            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_ERASE_BLOCK_START, ulOffsetAddressableUnit);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                ioStat.ulCount = 1;

                FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);
            }
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-BlockErase() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#if 0
/*-------------------------------------------------------------------
    Local: IsWriteProtected()

    Universal routine to determine if a chip is write protected
    by checking the write protect bit in any NAND chip status
    register.

    Simply send the status read command to the flash and return
    a boolean TRUE if the flash is write protected. False if
    otherwise.

    Parameters:

    Return Value:
        FIM_OK          - if the flash is NOT write protected
        FIM_EWRITE_PROT - if the flash IS write protected
        FIM_ETIMEOUT    - if the chip is too dead to tell
-------------------------------------------------------------------*/
static FIMResult_t IsWriteProtected(
    NTMHANDLE       hNTM,
    D_UINT32        ulOffset)
{
    NAND_IO_TYPE    Status;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-IsWriteProtected() Offset=%lX\n", ulOffset));

    /*  Adjust the address in the event that there is reserved space
    */
    ulOffset += hNTM->ulReservedSize;

    if(CHIP_READY_WAIT() == FFXSTAT_SUCCESS)
    {
        if(SetReadChipStatus(hNTM, ulOffset) == FFXSTAT_SUCCESS)
        {
            Status = NAND_STATUS_IN();
            Status = BENDSWAP(Status);  /* unsafely swaps volatile memory ptrs */

            /*  Ensure the write protect status is up to date
            */
            if(Status & NANDSTAT_WRITE_ENABLE)
                return FIM_OK;
            else
                return FIM_EWRITE_PROT;
        }
    }

    return FIM_ETIMEOUT;
}
#endif


/*-------------------------------------------------------------------
    Public: GetPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The flash offset in pages.

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

    ioStat = FfxNtmHelpReadPageStatus(hNTM, &ReadSpareArea, ulPage, hNTM->pChipInfo);

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
        ulBlock - The flash offset in blocks.

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

    DclAssert(hNTM);

  #if EMULATE_512B_PAGES
    if(hNTM->NtmInfo.uPageSize != hNTM->uPhysPageSize)
        ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadPhysSpareArea, hNTM->pChipInfo, ulBlock);
    else
  #endif
        ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pChipInfo, ulBlock);


    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
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
        ulBlock       - The flash offset in blocks.
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

    DclAssert(hNTM);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
      #if EMULATE_512B_PAGES
        if(hNTM->NtmInfo.uPageSize != hNTM->uPhysPageSize)
        {
            ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadPhysSpareArea, &WritePhysSpareArea,
                hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
        }
        else
      #endif
        {
            ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
                hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
        ulBlock, ulBlockStatus, FfxDecodeIOStatus(&ioStat)));

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
    FFXSTATUS       ffxStat = FFXSTAT_BADPARAMETER;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        NTMNAME"-ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, (NTMNAME"-ParameterGet() unhandled parameter ID=%x\n", id));
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
            FFXPRINTF(1, (NTMNAME"-ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        NTMNAME"-ParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: IORequest()

    This function implements a generic IORequest handler at the NTM
    level.

    Parameters:
        hNTM      - The NTM handle.
        pIOR      - A pointer to the FFXIOREQUEST structure to use.

    Return Value:
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

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_SUSPEND() hNTM=%P PowerState=%u (STUBBED!)\n", 
                hNTM, pReq->nPowerState));

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

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_POWER_RESUME() hNTM=%P PowerState=%u (STUBBED!)\n", 
                hNTM, pReq->nPowerState));

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
            FFXPRINTF(3, (NTMNAME"-IORequest() hNTM=%P Unsupported function %x\n", 
                hNTM, pIOR->ioFunc));

            ioStat.ffxStat = FFXSTAT_FIM_UNSUPPORTEDIOREQUEST;

            break;
        }
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-IORequest() hNTM=%P returning %s\n", hNTM, FfxDecodeIOStatus(&ioStat)));

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
        ulPage - The page to use
        pSpare - The buffer to fill

    Return Value:
        Returns a standard FFXIOSTATUS value.
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareArea(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    D_BUFFER       *pSpare)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32        ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-ReadSpareArea() Page=%lX\n", ulPage));

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        D_UINT32 ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

        ioStat.ffxStat = RandomReadSetup(hNTM, ulAddressableUnitAddr, 0);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            NAND_DATA_IN((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);

            ioStat.ulCount = 1;
        }
    }

/*  SpareReadCleanup: */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-ReadSpareArea() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: WriteSpareArea()

    This function writes the spare area for the given flash
    offset from the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The page to use
        pSpare - The data to write

    Return Value:
        Returns a standard FFXIOSTATUS value.
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    const NAND_IO_TYPE *pIO = (NAND_IO_TYPE *)pSpare;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulOffsetAddressableUnit;
    D_UINT32            ulAddressableUnitAddr;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-WriteSpareArea() Page=%lX\n", ulPage));

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

        if(hNTM->uChipPageIndexLSB <= 8)     /* 512b page chip? */
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_SPARE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                goto SpareWriteCleanup;
        }

        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

            if(hNTM->uChipPageIndexLSB > 8)     /* 2KB page chip? */
            {
                D_UINT16    uLogicalPage;

                /*  Where in this physical page is the appropriate logical page?
                */
                uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));

                /*  Seek from the start of this logical main page.  This
                    typecast is safe because the page number multipied by the
                    standard NAND data size is a maximum of 1536.
                */
                ioStat.ffxStat = WRITE_SEEK_SPARE(
                    (D_UINT16)(uLogicalPage * hNTM->NtmInfo.uPageSize),
                    uLogicalPage, hNTM->NtmInfo.uSpareSize);

                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    goto SpareWriteCleanup;
            }

            /*  Now write the metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT(pIO, (D_UINT16)SPARE_IO_COUNT);

            /*  Program it
            */
            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulOffsetAddressableUnit);
        }
    }

  SpareWriteCleanup:

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ioStat.ulCount = 1;

        FFXERRORINJECT_WRITE("WriteSpareArea", ulPage, &ioStat);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-WriteSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#if EMULATE_512B_PAGES

/*-------------------------------------------------------------------
    Local: ReadPhysSpareArea()

    This function is used when emulating 512 byte pages to
    allow for some operations, such as "GetBlockStatus" to
    access physical pages on the flash.  It reads the spare
    area for the given physical page into the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a page number which has
    already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadPhysSpareArea(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    D_BUFFER       *pSpare)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32        ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-ReadPhysSpareArea() Page=%lU\n", ulPage));

    /*  This function only exists when emulating small pages on 2KB-page
        flash, therefore this better always be TRUE...
    */
    DclAssert(hNTM->uChipPageIndexLSB > 8);
    DclAssert(hNTM->uPhysPageSize == NAND2K_PAGE);

    ulOffsetAddressableUnit = ulPage * (hNTM->uPhysPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            D_UINT32 ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

            SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                D_UINT16    uSpareOffset, uLogicalPage;

                /*  Note: Performing this calculation while waiting for the
                          chip to become ready provides a small, but measurable
                          performance boost.
                */

                /*  Where in this physical page is the appropriate logical page?
                */
                uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                    (hNTM->uPhysPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));
                uSpareOffset = uLogicalPage * hNTM->uPhysSpareSize;
                uSpareOffset += hNTM->uPhysPageSize;
                uSpareOffset >>= NAND_DATA_WIDTH_POW2;

                ioStat.ffxStat = CHIP_READY_WAIT();
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM);
                    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    {
                        AdjustPhysicalPageOffset(hNTM, uSpareOffset);

                        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
                        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                        {
                            NAND_DATA_IN((NAND_IO_TYPE *)pSpare, (D_UINT16)PHYSSPARE_IO_COUNT);

                            ioStat.ulCount = 1;
                        }
                    }
                }
            }
        }
    }

/*  PhysSpareReadCleanup: */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-ReadPhysSpareArea() Page=%lU returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: WritePhysSpareArea()

   This function is used when emulating 512 byte pages to
   allow for some operations, such as "GetBlockStatus" to
   access physical pages on the flash.  It writes the spare
   area for the given flash offset from the supplied buffer.

   This function is used internally by the NTM and various
   NTM helper functions, and must use a page number which has
   already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WritePhysSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulOffsetAddressableUnit;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-WritePhysSpareArea() Page=%lU\n", ulPage));

    /*  This function only exists when emulating small pages on 2KB-page
        flash, therefore this better always be TRUE...
    */
    DclAssert(hNTM->uChipPageIndexLSB > 8);
    DclAssert(hNTM->uPhysPageSize == NAND2K_PAGE);

    ulOffsetAddressableUnit = ulPage * (hNTM->uPhysPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            D_UINT16 uLogicalPage;
            D_UINT32 ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

            /*  Where in this physical page is the appropriate logical page?
            */
            uLogicalPage = (D_UINT16) ((ulAddressableUnitAddr &
                ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) / (hNTM->uPhysPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));

            SetLogicalPageAddress(hNTM, ulAddressableUnitAddr);

            /*  Seek from the start of this logical main page.  This
                typecast is safe because the page number multipied by the
                standard NAND data size is a maximum of 1536.
            */
            ioStat.ffxStat = WRITE_SEEK_SPARE((D_UINT16)(uLogicalPage * hNTM->uPhysPageSize),
                                                uLogicalPage, hNTM->uPhysSpareSize);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                /*  Now write the metadata to the flash part's internal buffer
                */
                NAND_DATA_OUT((NAND_IO_TYPE *)pSpare, (D_UINT16)PHYSSPARE_IO_COUNT);

                /*  Program it
                */
                ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulOffsetAddressableUnit);
            }
        }
    }

/*  WritePhysSpareCleanup: */

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ioStat.ulCount = 1;

        FFXERRORINJECT_WRITE("WritePhysSpareArea", BYTES_PER_ADDRESSABLE_UNIT * ulOffsetAddressableUnit / hNTM->NtmInfo.uPageSize, &ioStat);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-WritePhysSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

#endif


/*-------------------------------------------------------------------
    Local: RandomReadSetup()

    Prepare to read from a random offset within the spare area.

    Parameters:
        hNTM         - The NTM handle to use
        ulChipOffset - The offset of the page within the chip
        nSpareOffset - The offset within the spare area

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS RandomReadSetup(
    NTMHANDLE       hNTM,
    D_UINT32        ulAddressableUnitOffset,
    unsigned        nSpareOffset)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-RandomReadSetup() ChipOffset=%lXkb SpareOffset=%x\n",
        AddressableUnitToKB(ulAddressableUnitOffset),
        nSpareOffset));

    if(hNTM->uChipPageIndexLSB > 8)     /* 2KB page chip? */
        ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
    else
        ffxStat = SET_CHIP_COMMAND(CMD_READ_SPARE);

    if(ffxStat == FFXSTAT_SUCCESS)
    {
        SetLogicalPageAddress(hNTM, ulAddressableUnitOffset);

        if(hNTM->uChipPageIndexLSB > 8)             /* 2KB page chip? */
        {
            ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                D_UINT16    uSpareIndex;

                /*  Note: Performing this calculation while waiting for the
                          chip to become ready provides a small, but measurable
                          performance boost.
                */

                /*  Where in this physical page is the appropriate logical page?
                */
                uSpareIndex = (D_UINT16)((ulAddressableUnitOffset &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1)) /
                     (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT));
                uSpareIndex *= hNTM->NtmInfo.uSpareSize;
                uSpareIndex += (hNTM->uPhysPageSize + nSpareOffset);
                uSpareIndex >>= NAND_DATA_WIDTH_POW2;

                ffxStat = CHIP_READY_WAIT();
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM);
                    if(ffxStat == FFXSTAT_SUCCESS)
                    {
                        AdjustPhysicalPageOffset(hNTM, uSpareIndex);
                        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
                    }
                }
            }
        }
        else
        {
            ffxStat = CHIP_READY_WAIT();
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-RandomReadSetup() returning status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ReadID()

    Read the ID from a NAND chip.

    Parameters:
        hNTM                   - The NTM handle
        pucID                  - A pointer to a NAND_ID_SIZE-byte buffer
                                 to receive the ID
        ulOffsetAddresableUnit - The offset within the total array
                                 representing the beginning of the chip,
                                 in addressable units.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ReadID(
    NTMHANDLE       hNTM,
    D_UCHAR        *pucID,
    D_UINT32        ulOffsetAddressableUnit)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-ReadID() Offset=%lXKB\n", AddressableUnitToKB(ulOffsetAddressableUnit)));

    /*  select chip based on address MSB
    */
    ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            ffxStat = CHIP_READY_WAIT();
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                ffxStat = SET_CHIP_COMMAND(CMD_READ_ID);
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    unsigned ii;

                    NAND_ADDR_OUT(0);

                    /*  Read the device ID.  The first two bytes are always
                        the manufacturer ID and device ID, respectively.
                        The meaning of subsequent bytes (if any) depends on
                        the device.
                    */
                    for(ii = 0; ii < NAND_ID_SIZE; ii++)
                    {
                        NAND_IO_TYPE Temp;

                        NAND_DATA_IN(&Temp, 1);

                        pucID[ii] = (D_UCHAR) BENDSWAP(Temp);
                    }

                  #if NAND_ID_SIZE != 6
                    #error "NAND_ID_SIZE has changed, fix this code"
                  #endif

                    FFXPRINTF(1, ("NAND chip ID %02x %02x %02x %02x %02x %02x\n",
                                  pucID[0], pucID[1], pucID[2], pucID[3], pucID[4], pucID[5]));
                }
            }
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: SetReadChipStatus()

    Universal "read chip status" command for all NAND chips.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS SetReadChipStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulAbsoluteOffsetAddressableUnit)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-SetReadChipStatus() AbsOffset=%lXKB\n", AddressableUnitToKB(ulAbsoluteOffsetAddressableUnit)));

    /*  select chip based on address MSB
    */
    ffxStat = SET_CHIP_SELECT(ulAbsoluteOffsetAddressableUnit, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_STATUS);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: GetError()

    Universal routine to detect an error condition in the status
    register of all NAND chips.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS GetError(
    NTMHANDLE       hNTM,
    D_UINT32        ulAbsoluteOffsetAddressableUnit)
{
    NAND_IO_TYPE    Status;
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-GetError() AbsOffset=%lXKB\n", AddressableUnitToKB(ulAbsoluteOffsetAddressableUnit)));

    ffxStat = SetReadChipStatus(hNTM, ulAbsoluteOffsetAddressableUnit);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        Status = NAND_STATUS_IN();
        Status = BENDSWAP(Status);      /* unsafely swaps volatile memory ptrs */

        /*  This function should only be called after the chip has
            reported it has finished an operation.  If this isn't
            so, the rest of the status isn't yet valid; just call it
            a timeout.
        */
        if((Status & NANDSTAT_READY) == 0)
            ffxStat = FFXSTAT_FIMTIMEOUT;
        else if((Status & NANDSTAT_ERROR) != 0)
            ffxStat = FFXSTAT_FIMIOERROR;
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ExecuteChipCommand()

    This routine is similar to the NANDSetChipCommand() except that
    it is used specifically for write/erase confirmation commands.
    It waits for those commands to complete (or to time-out) and
    returns the status.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS ExecuteChipCommand(
    NTMHANDLE       hNTM,
    D_UCHAR         ucCommand,
    D_UINT32        ulAbsoluteOffsetAddressableUnit)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-ExecuteChipCommand() Cmd=%u AbsOffset=%lXKB\n", ucCommand, AddressableUnitToKB(ulAbsoluteOffsetAddressableUnit)));

    ffxStat = SET_CHIP_COMMAND_NOWAIT(ucCommand);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = CHIP_READY_WAIT();
        if(ffxStat == FFXSTAT_SUCCESS)
            ffxStat = GetError(hNTM, ulAbsoluteOffsetAddressableUnit);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-ExecuteChipCommand() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ResetChipAndWait()

    Universal "reset chip" command for all NAND chips.  This waits
    for the device to be ready after sending the reset command.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS ResetChipAndWait(
    NTMHANDLE       hNTM)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-ResetChipAndWait()\n"));

    ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = CHIP_READY_WAIT();

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: SetBlockAddress()

    Universal routine to output a block address for a NAND chip.

    ulAddressableUnitOffset is an addressable unit that only contains the
    offset into the chip.  The chip selects must have been set and masked
    off before this routine is called.

    NOTE: Consult your NAND manufacturer's data sheets for the
          definition of the LSB used in this routine. It may be that
          the chip expects a page address rather than an actual block
          address.  Or not.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void SetBlockAddress(
    NTMHANDLE       hNTM,
    D_UINT32        ulAddressableUnitOffset)
{
    int             iBits;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-SetBlockAddress() Block=%lX\n",
        (ulAddressableUnitOffset/(hNTM->NtmInfo.ulBlockSize/BYTES_PER_ADDRESSABLE_UNIT))));

    ulAddressableUnitOffset >>= (hNTM->uLinearBlockAddrLSB - ADDR_LINES_PER_ADDRESSABLE_UNIT);
    iBits = hNTM->uLinearChipAddrMSB - hNTM->uLinearBlockAddrLSB;
    do
    {
        NAND_ADDR_OUT((D_UCHAR)ulAddressableUnitOffset);

        ulAddressableUnitOffset >>= 8;
        iBits -= 8;

    }
    while(iBits > 0);

}


/*-------------------------------------------------------------------
    Local: SetLogicalPageAddress()

    Universal routine to output a page address for a NAND chip.

    ulAddressableUnitOffset is a modified version of the linear address
    that only contains the offset into the chip.  The chip selects
    must have been set and masked off before this routine is called.
    The offset must aligned with the size of a logical NAND page.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void SetLogicalPageAddress(
    NTMHANDLE       hNTM,
    D_UINT32        ulAddressableUnitOffset)
{
    D_UCHAR         ucAddress;
    D_UINT32        ulPageOffset, ulPageIndex;
    int             iBits;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-SetLogicalPageAddress() Offset=%lXkb\n", AddressableUnitToKB(ulAddressableUnitOffset)));

    /*  Get offset into the physical data page.  This offset is in terms of the
        width of the data port (byte, word, ...)
    */
    ulPageOffset = ulAddressableUnitOffset &
                    ((1 << (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT)) - 1);

    /*  Now that the we are using a masked ulPageOffset, it needs to
        be restored to a linear byte address.
    */
    ulPageOffset <<= ADDR_LINES_PER_ADDRESSABLE_UNIT;

    ulPageOffset &= ((1 << hNTM->uChipPageIndexLSB) - 1);   /* for 512B pages */
    ulPageOffset >>= NAND_DATA_WIDTH_POW2;

    /*  Convert the upper bits of the address into a page index and align it
        as the chip expects.  They may easily be more than 16 address bits in
        the page index so don't try and merge that with the page offset.
    */
    ulPageIndex = ulAddressableUnitOffset >> (hNTM->uLinearPageAddrMSB - ADDR_LINES_PER_ADDRESSABLE_UNIT);

    /*  Output the page offset one byte at a time, LSB first.
    */
    ucAddress = (D_UCHAR) ulPageOffset;
    NAND_ADDR_OUT(ucAddress);
    if(hNTM->uChipPageIndexLSB > 8) /* 2KB page chip? */
    {
        ulPageOffset >>= 8;     /* align offset MSB */
        ucAddress = (D_UCHAR) ulPageOffset;     /* put out offset MSB */
        NAND_ADDR_OUT(ucAddress);
    }

    /*  Figure out how many bits we have yet to send for this chip
        and output the page index one byte at a time, LSB first.
    */
    iBits = hNTM->uLinearChipAddrMSB - hNTM->uLinearPageAddrMSB;
    do
    {
        ucAddress = (D_UCHAR) ulPageIndex;
        NAND_ADDR_OUT(ucAddress);

        ulPageIndex >>= 8;
        iBits -= 8;
    }
    while(iBits > 0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-SetLogicalPageAddress() complete\n"));
}


/*-------------------------------------------------------------------
    Local: AdjustPhysicalPageOffset()

    Universal routine to adjust the current page address of NAND
    chips with 2KB pages.

    uOffset is a linear byte offset that only contains the offset
    into the page. uOffset must be aligned according to the page
    being read; 512B for a data page and 16B for a spare area.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void AdjustPhysicalPageOffset(
    NTMHANDLE       hNTM,
    D_UINT16        uOffset)
{
    D_UCHAR         ucAddress;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-AdjustPhysicalPageOffset() Offset=%X\n", uOffset));

    DclAssert(hNTM->uChipPageIndexLSB > 8);

    ucAddress = (D_UCHAR) uOffset;              /* output offset LSB */
    NAND_ADDR_OUT(ucAddress);
    uOffset >>= 8;                              /* align offset MSB */

    ucAddress = (D_UCHAR) uOffset;              /* put out offset MSB */
    NAND_ADDR_OUT(ucAddress);

    return;
}


/*-------------------------------------------------------------------
    Local: WriteSeekToshibaSpare()

    Moves to the spare region for the specified logical page.

    uOffset tells us if we are starting at the beginning or
    the end of this logical page.  Usually, the main page data
    has already been written, but this isn't always the case.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS WriteSeekToshibaSpare(
    NTMHANDLE       hNTM,
    D_UINT16        uOffset,
    D_UINT16        uLogicalPage,
    D_UINT16        uSpareSize)
{
    D_UINT16        uSpareOffset;
    NAND_IO_TYPE    pad = PAD_VALUE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-WriteSeekToshibaSpare() Offset=%X Page=%X SpareSize=%U\n",
        uOffset, uLogicalPage, uSpareSize));

    DclAssert(uLogicalPage <= 3);
    DclAssert(hNTM->uPhysPageSize >= NAND2K_PAGE);

    uSpareOffset = uLogicalPage * uSpareSize;
    uSpareOffset += hNTM->uPhysPageSize;

    /*  Skip from the end (or start) of this main page to the start of
        the associated spare area.  The only way to do this is to
        write the same pad data that already exists in the buffer.

        It takes about 4*512*50ns or 102us to skip by filling the write
        buffer in the worst case. That is significantly less than the
        200-300us it would take to do an additional programming
        operation.
    */

    /*  TODO: This can be optimized by creating a static buffer full of
        pad values at initialization, and calling NAND_DATA_OUT with that
        buffer and the appropriate length.
    */
    while(uOffset < uSpareOffset)
    {
        NAND_DATA_OUT(&pad, 1);
        uOffset += NAND_DATA_WIDTH_BYTES;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: WriteSeekSamsungSpare()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS WriteSeekSamsungSpare(
    NTMHANDLE       hNTM,
    D_UINT16        uOffset,
    D_UINT16        uLogicalPage,
    D_UINT16        uSpareSize)
{
    D_UINT16        uSpareOffset;
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-WriteSeekSamsungSpare() Offset=%X Page=%X SpareSize=%U\n",
        uOffset, uLogicalPage, uSpareSize));

    /*  On Samsung chips, we don't care where we started because there
        is a command to take us to this particular offset, but not all
        NAND chips have this wonderful feature.
    */
    (void)uOffset;

    DclAssert(uLogicalPage <= 3);
    DclAssert(hNTM->uPhysPageSize >= NAND2K_PAGE);

    uSpareOffset = uLogicalPage * uSpareSize;
    uSpareOffset += hNTM->uPhysPageSize;
    uSpareOffset >>= NAND_DATA_WIDTH_POW2;

    /*  Adjust the write pointer to the appropriate spare area
    */
    ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_WRITE_PAGE_RANDOM);
    if(ffxStat == FFXSTAT_SUCCESS)
        AdjustPhysicalPageOffset(hNTM, uSpareOffset);

    return ffxStat;
}

/*------------------------------------------------------------------
    Local: SendBlockAddr()

    Sends a block address to the chip. Designed to be used in
    conjunction with block lock and unlock commands.

    Parameters:
        hNTM        - Handle to NTM
        ulBlockAddr - Which block address to send

    Return Value:
        None.
-------------------------------------------------------------------*/
static void SendBlockAddr(
    NTMHANDLE   hNTM,
    D_UINT32    ulBlockNum)
{
    D_UINT32 uPagesShiftCount;
    D_UINT32 ulLocalBlockAddr;

    /*  Figure out how many bits are needed to represent the number of pages
        in a block:
    */
    for (uPagesShiftCount = 0;
         (0x01 << uPagesShiftCount) < hNTM->NtmInfo.uPagesPerBlock;
         uPagesShiftCount++);
    ulLocalBlockAddr = ulBlockNum << uPagesShiftCount;

    NAND_ADDR_OUT((D_UCHAR)ulLocalBlockAddr);
    ulLocalBlockAddr >>= 8;
    NAND_ADDR_OUT((D_UCHAR)ulLocalBlockAddr);
    ulLocalBlockAddr >>= 8;
    NAND_ADDR_OUT((D_UCHAR)ulLocalBlockAddr);
}


/*-------------------------------------------------------------------
    Local: UnlockBlocks()

    Procedure to unlock all blocks in a NAND chip that uses NTCad.
    This procedure is intended only to be called at creation of the
    NTM, and should NEVER be called during general chip operations.
    This is because this procedure resets the chip, and any operation
    that was being performed when this procedure is called will be
    abandoned.

    Parameters:
        hNTM          - Handle for NTM to use
        ulStartBlocks - First block in range to unlock
        ulEndBlocks   - Last block in range to unlock

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS UnlockBlocks(
    NTMHANDLE   hNTM,
    D_UINT32    ulStartBlocks,
    D_UINT32    ulEndBlocks)
{
    FFXIOSTATUS ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    if (!(hNTM->pChipInfo->pChipClass->fBlockLockSupport))
        return FFXSTAT_SUCCESS;

/*  Don't bother checking if blocks are already unlocked; just unlock everything
*/

    if ((ioStat.ffxStat = SET_CHIP_COMMAND(CMD_BLOCK_UNLOCK_START)) == FFXSTAT_SUCCESS)
    {
        SendBlockAddr(hNTM,ulStartBlocks);
        if ((ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_BLOCK_UNLOCK_END)) == FFXSTAT_SUCCESS)
        {
            SendBlockAddr(hNTM, ulEndBlocks);
            return FFXSTAT_SUCCESS;
        }
    }
    return ioStat.ffxStat;
}
   

/*-------------------------------------------------------------------
    Local: UpperBoundProximity()

    This procedure checks a chip address given in addressable units
    against the NTM address boundary. A separate procedure is given
    for doing this because depending on whether an addressable unit
    is bigger, smaller or the same as the units that the bounds are
    given in, care must be taken in how the comparison is done to
    avoid gratuitously losing half the addressable space that should
    be available to the NTM.

    Parameters:
        ulAddressableUnit - Addressable unit
        ulUpperBoundKB    - Upper limit of addressable range for this
                            NTM.

    Return Value:
        Number of addressable units between the input addressable
        unit and the upper bound; zero if the bound is already reached
        or exceeded.
-------------------------------------------------------------------*/
static D_UINT32 UpperBoundProximity(D_UINT32 ulAddressableUnit, D_UINT32 ulUpperBoundKB)
{
D_UINT32 ulReturnVal;

    if (BYTES_PER_ADDRESSABLE_UNIT <= BYTES_PER_KILOBYTE)
    {
    D_UINT32 ulAddressableUnitsPerKilobyte = BYTES_PER_KILOBYTE/BYTES_PER_ADDRESSABLE_UNIT;

        if (ulAddressableUnit/ulAddressableUnitsPerKilobyte < ulUpperBoundKB)
            ulReturnVal = (ulUpperBoundKB - ulAddressableUnit/ulAddressableUnitsPerKilobyte) *
                              ulAddressableUnitsPerKilobyte;
        else
            ulReturnVal = 0;
    }
    else
    {
    D_UINT32 ulKilobytesPerAddressableUnit = BYTES_PER_ADDRESSABLE_UNIT/BYTES_PER_KILOBYTE;

        if (ulUpperBoundKB/ulKilobytesPerAddressableUnit > ulAddressableUnit)
            ulReturnVal = ulUpperBoundKB/ulKilobytesPerAddressableUnit - ulAddressableUnit;
        else
            ulReturnVal = 0;
    }
    return ulReturnVal;
}

/*  The following procedure is used only in debug configurations:
*/
#if D_DEBUG
/*-------------------------------------------------------------------

    Local: AddressableUnitToKB()

    This procedure converts addressable units to kilobytes. Note
    that if the addressable unit is less than a kilobyte, the result
    of this function could be zero even if the input parameter is not.
    There's nothing we can do about that without using floating point
    math.

    Parameters:
        ulAddressableUnit - Addressable unit

    Return Value:
        Kilobytes.
-------------------------------------------------------------------*/
static D_UINT32     AddressableUnitToKB(D_UINT32 ulAddressableUnit)
{
    if (BYTES_PER_ADDRESSABLE_UNIT <= BYTES_PER_KILOBYTE)
        return  (ulAddressableUnit/(BYTES_PER_KILOBYTE/BYTES_PER_ADDRESSABLE_UNIT));
    else
        return (ulAddressableUnit*(BYTES_PER_ADDRESSABLE_UNIT/BYTES_PER_KILOBYTE));
}



#endif  /* D_DEBUG */
#endif  /* FFXCONF_NANDSUPPORT */

