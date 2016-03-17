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

    This module contains the MLC capable version of the "Command-Address-Data"
    NAND Technology Module (NTM) which supports NAND controllers with explicit
    registers for Commands and Address cycles, and either registers, FIFOs,
    buffers, or DMA access for Data cycles.  It interfaces with the FlashFX
    Hooks module fhcad.c, which allows easy project level customization of
    the interface without requiring a new NTM to be written.

    This file was derived from the existing NTCAD NTM, but has been modified
    to use the parameterized EDC mechanism as described in ecc.h. It supports
    *only* the parameterized EDC mechanism. If it is desired to use a
    traditional SLC CAD device, users should use the standare NTCAD NTM.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntcadmlc.c $
    Revision 1.25  2010/12/09 23:19:37Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.24  2010/07/07 02:12:03Z  garyp
    Commented out some broken error injection logic.
    Revision 1.23  2010/07/06 03:00:31Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.22  2010/01/27 04:27:36Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.21  2009/12/11 23:50:01Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.
    Revision 1.20  2009/10/11 20:19:32Z  glenns
    Fix Bugzilla #2863: Add code to account for flash bus width in invocations
    of the NAND_DATA_IN and NAND_DATA_OUT macros.
    Revision 1.19  2009/10/07 17:34:22Z  garyp
    Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
    Revision 1.18  2009/08/02 21:42:19Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written. Added a default IORequest()
    function.  Fixed SetBlockStatus() to return the proper operation type.
    Documentation updated.
    Revision 1.17  2009/04/03 18:10:03Z  glenns
    - Added explanatory comment related to Bugzilla #2402.
    Revision 1.16  2009/04/02 14:24:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.15  2009/03/31 02:12:08Z  glenns
    - Fix Bugzilla #2565: Repair HiddenWrite to calculate ECCs and
      add them in the spare area if the EDC mechanism is configured
      to cover metadata.
    - Fix Bugzilla #2566: Correct logical errors in section of code to
      evaluate full-up EDC should tag CRC check fail.
    Revision 1.14  2009/03/27 06:09:18Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.13  2009/03/24 20:04:19Z  glenns
    - Added code to check flag in ecc.h for whether it is permissible
      to continue operations if the available EDC mechanism is weaker
      than chip requirements.
    - Removed extra MemFree in the case that the Create procedure
      fails.
    Revision 1.12  2009/03/17 22:51:46Z  glenns
    - Fix Bugzilla #2523: Modified check on CorrectPage to handle the
      case where page status indicates correctable error in a page, but
      FFXSTATUS indicates uncorrectable error elsewhere.
    Revision 1.11  2009/03/13 03:40:03Z  glenns
    - Fix Bugzilla #2501: This bug was a side effect of revision 1.10;
      basically the code was originally written not to set any values if the
      required spare area size matched the chip's reported spare area
      size, but with the introduction of the separation between available
      spare area and actual spare area used, the amount of spare area
      used needs to be explicity set under all condiitions.
    Revision 1.10  2009/03/09 19:31:36Z  glenns
    - Ancillary to fix for Bugzilla #2384: This Bugzilla fix requires NTM's
      that use custom spare formats and parameterized EDC to report
      the physical size of the spare area to the FIM clients and only
      the amount of spare area actually used to the parameterized
      EDC mechanism, and those values are not necessarily the same.
      This checkin accomplishes that adjustment for this NTM.
    Revision 1.9  2009/03/04 20:10:08Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    - Renamed a poorly-named variable.
    - Fixed bug in conditional looking for "corrected data" page status.
    Revision 1.8  2009/02/26 22:22:35Z  glenns
    - Fixed compiler warning about function defined but not used.
    Revision 1.7  2009/02/26 22:02:14Z  glenns
    - Fixed merging mistake in last check-in.
    Revision 1.6  2009/02/26 20:58:10Z  glenns
    - Bugzilla #2405: Overhaul of LFA code including:
      - Eliminating magic numbers and replacing them with macros or
        calculations.
      - Re-engineering the code to examine for multiple chips.
      - Renaming numerous variables to eliminate confusion with
        blocks and clarify purpose.
      - Updating function header comments to correspond with LFA
        code changes.
    - Modified to use more flexible and rigorous method of tag
      encoding now in ntpecchelp.
    Revision 1.5  2009/02/24 02:18:29Z  glenns
    - Implemented fix for Bugzilla #2431 by re-introducing small-block
      spare area access command removed in an earlier revision.
    - Fixed logical problem exposed by LFA changes in SetBlockAddress.
    - Changed hard-coded literal in several calculations with more
      meaningfully-named macro.
    - Fixed up some function header comments.
    - Removed some disabled code.
    Revision 1.4  2009/02/18 23:58:29Z  glenns
    - Updated to accommodate new function prototypes for EDC
      processor "Init" and "Destroy" functions. Part of fix for Bugzilla
      #2416.
    Revision 1.3  2009/02/16 21:50:37Z  thomd
    Merge corrections to Large Flash Addressing from branch
    Revision 1.2  2009/02/12 19:55:06Z  glenns
    - Removed unnecessary issuances of the RESET command.
    - Removed unnecessary checks on the fEdcParameterized flag,
      and associated dead code. This NTM always uses parameterized
       EDC.
    Revision 1.1  2009/02/09 03:30:44Z  glenns
    Initial revision
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


#define NTMNAME "NTCADMLC"
#define NTCADMLC_MAXEDC  4

/*  Macro for defining whether to use parameterized ECC
    (see ecc.h). Note that "NTM_USES_CUSTOM_SPARE_FORMAT" will be
    overridden if the device in use requires more than one bit
    of EDC.
*/
#define NTM_USES_PARAMETERIZED_ECC FALSE
#define NTM_USES_CUSTOM_SPARE_FORMAT FALSE

struct tagNTMDATA
{
    D_UINT16        uPhysPageSize;
    D_UINT16        uPhysSpareSize;
    D_UINT16        uLinearPageAddrMSB;  /* how much to >> linear address to get chip page index  */
    D_UINT16        uChipPageIndexLSB;   /* how much to << chip page index before sending to chip */
    D_UINT16        uLinearBlockAddrLSB; /* how much to >> linear address to get block index */
    D_UINT16        uLinearChipAddrMSB;  /* how much to >> linear address to get chip select */
    D_BUFFER       *pSpare;              /* MLC support: For handling variable spare area formats */
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
    FX_EDC_PROCESSOR *pEccProcessor;
};

/*  number of reads/writes needed for 1) a 512B logical page, 2) a 16B logical
    spare area and 3) a VBF allocation record
*/
#define DATA_IO_COUNT       (hNTM->NtmInfo.uPageSize / sizeof(NAND_IO_TYPE))
#define SPARE_IO_COUNT      (hNTM->NtmInfo.uSpareSize / sizeof(NAND_IO_TYPE))
#define SPARE16_IO_COUNT    (NAND512_SPARE / sizeof(NAND_IO_TYPE))
#define PAD_VALUE           ((NAND_IO_TYPE) ~((NAND_IO_TYPE) 0))

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
static FFXSTATUS    WriteSeekSamsungSpare(    NTMHANDLE hNTM, D_UINT16 uOffset, D_UINT16 uLogicalPage, D_UINT16 uSpareSize);
static FFXSTATUS    WriteSeekToshibaSpare(    NTMHANDLE hNTM, D_UINT16 uOffset, D_UINT16 uLogicalPage, D_UINT16 uSpareSize);
static FFXIOSTATUS  ReadSpareArea(            NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(           NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXSTATUS    RandomReadSetup(          NTMHANDLE hNTM, D_UINT32 ulBlockOffset, unsigned nSpareOffset);
static D_UINT32     UpperBoundProximity(      D_UINT32  ulAddressableUnit, D_UINT32 ulUpperBoundKB);

/*  The following procedure is used only in debug configurations:
*/
#if D_DEBUG
static D_UINT32     AddressableUnitToKB(      D_UINT32  ulAddressableUnit);
#endif

static FFXSTATUS    UnlockBlocks(             NTMHANDLE hNTM, D_UINT32 ulStartBlock, D_UINT32 ulEndBlock);

/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_cadmlc =
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
    D_UINT32            ulEccSize;
    D_UINT16            uRequiredSpareSize;

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

    /*  Addressable unit size must not be larger than a page:
    */
    DclAssert(BYTES_PER_ADDRESSABLE_UNIT <= pNTM->pChipInfo->pChipClass->uPageSize);

    if( !ALLOW_INFERIOR_EDC && pNTM->pChipInfo->pChipClass->uEdcRequirement > NTCADMLC_MAXEDC )
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

    /*  We allocate a full-size spare area here even though we may not use
        all of it. This is to accomodate one-off conditions where a
        particular device or modified NTM may actually encode information in
        areas of the spare area that this NTM does not currently use.
    */
    pNTM->pSpare = DclMemAlloc(pNTM->pChipInfo->pChipClass->uSpareSize);
    if(!pNTM->pSpare)
        goto CreateCleanup;

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
        pNTM->WriteSeekSpare = WriteSeekSamsungSpare;
    }
    else
    {
        pNTM->WriteSeekSpare = WriteSeekToshibaSpare;
    }

    /*  Save the native page/spare size
    */
    pNTM->uPhysPageSize = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->uPhysSpareSize = pNTM->pChipInfo->pChipClass->uSpareSize;

    pNTM->NtmInfo.uPageSize = pNTM->uPhysPageSize;
    pNTM->NtmInfo.uSpareSize = pNTM->uPhysSpareSize;

    pNTM->NtmInfo.ulBlockSize = pNTM->pChipInfo->pChipClass->ulBlockSize;

    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    pNTM->NtmInfo.uEdcCapability     = pNTM->pChipInfo->pChipClass->uEdcCapability;
    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

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

    /*  Determine which spare area format to use. This NTM *only* supports
        parameterized ECC as given in ecc.h; the traditional FlashFX Pro
        call chain into the EDC mechanisms is not supported. Use the
        NTCAD NTM if such support is desired.
    */
    pNTM->NtmInfo.fEdcParameterized = TRUE;

    /*  Standard spare formats only support 1-bit correction mechanisms:
    */
    if (pNTM->NtmInfo.uEdcRequirement > 1)
        pNTM->NtmInfo.uCustomSpareFormat = TRUE;
    else
        pNTM->NtmInfo.uCustomSpareFormat = NTM_USES_CUSTOM_SPARE_FORMAT;
    if (pNTM->NtmInfo.uCustomSpareFormat)
    {
        /* Cannot support custom spare format without parameterized ECC:
        */
        DclAssert(pNTM->NtmInfo.fEdcParameterized);

        /*  For the first release of Tera, if a custom spare area format
            is specified, we assume the DEFAULT MLC format as given in
            fxnandapi.h.  If a different format is desired, support for
            it will have to be added and the NTM will have to be modified
            in places like this.

            In the future, a new flexible method for specifying spare
            area formats may be included which will automate the process
            of selecting a spare area format and obviate the need for
            this sort of thing inside the NTMs.
        */
        pNTM->NtmInfo.fEdcProtectsMetadata = TRUE;
        pNTM->uTagOffset = NSDEFAULTMLC_TAG_OFFSET;
    }
    else switch(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo))
    {
        /*  The following are the traditional FlashFX Pro spare formats, but
            are also supported through the parameterized EDC mechanism.
        */
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

    /*  See what to do about ECC for this NTM/chip combination.
        Start by acquiring an ECC processor, and if it has an initialization
        procedure, call it.
    */
    pNTM->pEccProcessor = FfxAcquireEdcProcessor(pNTM->NtmInfo.uEdcRequirement);
    if (pNTM->pEccProcessor == NULL)
    {
        DclPrintf("FFX: Device ECC requirements not supportable.\n");
        goto CreateCleanup;
    }
    pNTM->NtmInfo.uEdcCapability = pNTM->pEccProcessor->ulCorrectionCapability;
    
    /*  The following assert is to assure that the cast on the next line
        of code is safe. The only time it wouldn't be is if the number of
        bytes evaluated in a given call to the EDC processor is greater
        that 65535, which isn't so for any EDC mechanisms currently in use.
    */
    DclAssert(pNTM->pEccProcessor->ulNumDataBytesEvaluated < D_UINT16_MAX);
    pNTM->NtmInfo.uEdcSegmentSize = (D_UINT16)(pNTM->pEccProcessor->ulNumDataBytesEvaluated);
    
    if (pNTM->pEccProcessor->Init)
    {
        if (!(*(pNTM->pEccProcessor->Init))(pNTM->pEccProcessor))
        {
            DclPrintf("FFX: EDC processor cannot be initialized.\n");
            goto CreateCleanup;
        }
    }

    /*  Allocate a scratch spare area buffer. We need to allocate a scratch
        buffer for this functionality because it supports spare areas that
        may not be derivable at compile time, so we need to use malloc rather
        arrays on the stack to get space for it. In keeping with the principle
        that we only do mallocs at initialization, we do it now.

        It is also important to note that some devices we use may have odd
        sized spare areas. Since our ECC algorithms depend on spare areas
        of a particular size and configuration, lets do some processing
        now to determine just how much spare area we need, and whether all
        of the spare area that exists on the chip will be utilized.

        REFACTOR: In the future a better method for managing spare area
        formats will be implemented, which will largely eliminate the need
        for this sort of processing. For now, we use macros. Differing
        spare area formats will require adjustments to this code.
    */
    if (pNTM->NtmInfo.uCustomSpareFormat)
    {
        uRequiredSpareSize = NSDEFAULTMLC_FACTORYBAD_LENGTH +
                              NSDEFAULTMLC_TAG_LENGTH +
                              NSDEFAULTMLC_ECC_LENGTH;
        uRequiredSpareSize *= (pNTM->NtmInfo.uPageSize / pNTM->pEccProcessor->ulNumDataBytesEvaluated);
        if (pNTM->NtmInfo.uSpareSize < uRequiredSpareSize)
        {
            DclPrintf("FFX: Detected part spare size too small for required spare format.\n");
            goto CreateCleanup;
        }
        else
        {
            /*  Physical spare size is larger than needed, so just use what
                we need and ignore the remaining bytes.
            */
            pNTM->NtmInfo.uSpareAreaUsed = uRequiredSpareSize;
            pNTM->NtmInfo.uSpareSize = pNTM->pChipInfo->pChipClass->uSpareSize;
       }
    }

    /*  We still need to allocate the full size spare area even though
        we may not use it all, because the nthelp mechanisms may use this
        buffer in the ReadSpareArea callback which will fill in the
        full spare area size.
    */
    pNTM->NtmInfo.pScratchSpareBuffer = DclMemAlloc(pNTM->NtmInfo.uSpareSize);
    if (!pNTM->NtmInfo.pScratchSpareBuffer)
    {
        DclPrintf("FFX: Insufficient memory to support parameterized ECC.\n");
        goto CreateCleanup;
    }

    /*  Figure out the number of bytes needed to store all ECCs for an entire
        page and allocate a scratch buffer of that size:
    */
    ulEccSize = (pNTM->NtmInfo.uPageSize / pNTM->pEccProcessor->ulNumDataBytesEvaluated);
    ulEccSize *= pNTM->pEccProcessor->ulNumBytesForECC;
    pNTM->NtmInfo.pScratchEccBuffer = DclMemAlloc(ulEccSize);
    if (!pNTM->NtmInfo.pScratchEccBuffer)
    {
        DclPrintf("FFX: Insufficient memory to support parameterized ECC.\n");
        goto CreateCleanup;
    }
    /*  Be sure the number of bytes consumed by this processor's ECCs will
        fit in the spare area:
    */
    if (NSDEFAULTMLC_ECC_LENGTH <= pNTM->pEccProcessor->ulNumBytesForECC)
    {
        DclPrintf("FFX: Insufficient space for ECCs in spare area.\n");
        goto CreateCleanup;
    }

    /*  Get a scratch buffer for page data. New TagDecode algorithms
        may need to use this to evaluate ECCs if tags are covered by
        the main-page ECC mechanism, and bit errors occur in the tag.
    */
    pNTM->NtmInfo.pScratchPageBuffer = DclMemAlloc(pNTM->NtmInfo.uPageSize);
    if (!pNTM->NtmInfo.pScratchPageBuffer)
    {
        DclPrintf("FFX: Insufficient memory to support parameterized ECC.\n");
        goto CreateCleanup;
    }

    FFXPRINTF(1, (NTMNAME": TotalSize=%lXKB ChipBlocks=%lX BlockSize=%lX PageSize=%X SpareSize=%X\n",
        AddressableUnitToKB(ulTotalAddressableUnits),
        pNTM->NtmInfo.ulChipBlocks,
        pNTM->NtmInfo.ulBlockSize,
        pNTM->uPhysPageSize,
        pNTM->uPhysSpareSize));

    /*  Make sure the parent has a pointer to our NTMINFO structure
    */
    *ppNtmInfo = &pNTM->NtmInfo;

    fSuccess = TRUE;

  CreateCleanup:

    if(pNTM && !fSuccess)
    {
        if (pNTM->NtmInfo.pScratchPageBuffer)
        {
            DclMemFree(pNTM->NtmInfo.pScratchPageBuffer);
        }
        if (pNTM->NtmInfo.pScratchEccBuffer)
        {
            DclMemFree(pNTM->NtmInfo.pScratchEccBuffer);
        }
        if (pNTM->NtmInfo.pScratchSpareBuffer)
        {
            DclMemFree(pNTM->NtmInfo.pScratchSpareBuffer);
        }
        if (pNTM->pEccProcessor)
        {
            if (pNTM->pEccProcessor->Destroy)
                (*(pNTM->pEccProcessor->Destroy))(pNTM->pEccProcessor);

            /*  Note- this code assumes that ECC Processors are
                either pointers to static objects, or if they are
                dynamically allocated, the "Destroy" procedure will
                free the associated memory. Therefore we do not free
                that memory here.
            */
            pNTM->pEccProcessor = NULL;
        }
        if (pNTM->pSpare)
        {
            DclMemFree(pNTM->pSpare);
        }
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


    if (hNTM->NtmInfo.pScratchPageBuffer)
    {
        DclMemFree(hNTM->NtmInfo.pScratchPageBuffer);
    }
    if (hNTM->NtmInfo.pScratchEccBuffer)
    {
        DclMemFree(hNTM->NtmInfo.pScratchEccBuffer);
    }
    if (hNTM->NtmInfo.pScratchSpareBuffer)
    {
        DclMemFree(hNTM->NtmInfo.pScratchSpareBuffer);
    }
    if (hNTM->pEccProcessor)
    {
        if (hNTM->pEccProcessor->Destroy)
            (*(hNTM->pEccProcessor->Destroy))(hNTM->pEccProcessor);

        /*  Note- this code assumes that ECC Processors are
            either pointers to static objects, or if they are
            dynamically allocated, the "Destroy" procedure will
            free the associated memory. Therefore we do not free
            that memory here.
        */
        hNTM->pEccProcessor = NULL;
    }
    if (hNTM->pSpare)
    {
        DclMemFree(hNTM->pSpare);
    }
    if(hNTM->pNC)
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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulAddressableUnitAddr;
    D_BUFFER           *pSpare = hNTM->pSpare;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageRead() StartPage=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageRead", 0, ulCount);

    DclAssert(pSpare);
    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);
    DclAssert(hNTM->NtmInfo.fEdcParameterized);

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

        NAND_DATA_IN((NAND_IO_TYPE*)pPages, hNTM->NtmInfo.uPageSize/sizeof(NAND_IO_TYPE));
        NAND_DATA_IN((NAND_IO_TYPE*)pSpare, hNTM->NtmInfo.uSpareSize/sizeof(NAND_IO_TYPE));
        if (fUseEcc)
        {
            FFXIOSTATUS ioValid= INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);
            FX_ECC_PARAMS eccParams;

            /*  At this point, hNTM->pSpare now contains the spare area
                that was read from the device. This is the "reference"
                spare area. Using this and the page data we just read,
                we *may* need to generate new ECC's, and then call the
                correction procedure. We'll use the FfxNtmPECCBuildSpareArea
                procedure to generate the new ECCs after duplicating the
                conditions in which the original reference spare area was
                built.

                To duplicate those conditions, we formulate the ECCParams
                structure. NTHelp stuff will derive the counts and offsets.

                REFACTOR: It would probably be better to change the
                declarations of the PECC NTHelp to take only the relevant
                pointers and then formulate the ECCParams structure within
                NTPECCHelp.
            */
            eccParams.data_buffer.buffer = pPages;
            eccParams.data_buffer.uCount = 0;
            eccParams.data_buffer.uOffset = 0;
            if (hNTM->NtmInfo.fEdcProtectsMetadata)
            {
                eccParams.metadata_buffer.buffer = pSpare;
                eccParams.metadata_buffer.uCount = 0;
                eccParams.metadata_buffer.uOffset = 0;
            }
            else
            {
                eccParams.metadata_buffer.buffer = NULL;
                eccParams.metadata_buffer.uCount = 0;
                eccParams.metadata_buffer.uOffset = 0;
            }
            eccParams.ecc_buffer = NULL;
            eccParams.reference_ecc_buffer = NULL;
            eccParams.pIoStat = &ioValid;
            eccParams.ulECCBufferByteCount = 0;
            eccParams.pEccProcessor = hNTM->pEccProcessor;


            /*  Create the spare area with newly calculated ECCs so
                we can perform the corrections. Note the peculiar
                assignment to the "pTags" parameter. This is because
                metadata may be used in the calculation of ECCs, and
                we want to be sure that the reference tags are included
                in the construction of the new spare area.

                Also note that not all ECC processors will require
                the client to provide a new set of ECCs calculated
                on the data read from the device; some do this on
                their own. If the processor doesn't require it, don't
                bother calculating them.
            */
            if (eccParams.pEccProcessor->fRequiresRecalculatedECC)
            {
                FfxNtmPECCBuildSpareArea(hNTM->hDev,
                                         &hNTM->NtmInfo,
                                         &eccParams,
                                         hNTM->NtmInfo.pScratchSpareBuffer,
                                         pSpare,
                                         fUseEcc,
                                         hNTM->pChipInfo);
                if (ioValid.ffxStat == FFXSTAT_UNSUPPORTEDFUNCTION)
                    break;
            }
            /*  Perform corrections.
            */
            FfxNtmPECCCorrectPage(&hNTM->NtmInfo,
                                  &eccParams,
                                  hNTM->NtmInfo.pScratchSpareBuffer,
                                  pSpare,
                                  hNTM->pChipInfo);

            /*  Finish with tag data. The caller may not care about it,
                so only do it if a tag buffer is supplied. In the case of
                parameterized ECC, we evaluate tags after performing
                correction because the ECC may cover tags.
            */
            if(pTags)
            {
                DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                (void)FfxNtmPECCTagDecode(&hNTM->NtmInfo,
                                          pTags,
                                          pSpare,
                                          FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo));

                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                               "Encoded Tag=%lX Decoded Tag=%X\n",
                               *(D_UINT32 *)pSpare,
                               *(D_UINT16 *)pTags));

                pTags += nTagSize;
            }

            /*  set adjusted page status
            */
            if (ioValid.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
            {
                ioStat.ulCount++;
            }
            ioStat.ffxStat = ioValid.ffxStat;
            ioStat.op.ulPageStatus |= ioValid.op.ulPageStatus;
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
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
    DclAssert(hNTM->NtmInfo.fEdcParameterized);

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
        D_BUFFER    *pSpare = hNTM->pSpare;

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

            DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

            /*  Send main page data. Do this before calculating ECCs to
                allow any DMA activity to occur in parallel if the platform
                suppports such things.
            */
            NAND_DATA_OUT((NAND_IO_TYPE*)pPages, hNTM->NtmInfo.uPageSize/sizeof(NAND_IO_TYPE));
            if (fUseEcc)
            {
                FX_ECC_PARAMS       eccParams;

                /*  Only set the buffer pointers for main page and metadata
                    here. The NTHelp stuff will use the spare area format info
                    and the NTMInfo available to it to calculate counts and
                    offsets.

                    REFACTOR: It would probably be better to change the
                    declarations of the PECC NTHelp to take only the relevant
                    pointers and then formulate the ECCParams structure within
                    NTHelp.
                */
                eccParams.data_buffer.buffer = (D_BUFFER *)pPages;
                eccParams.data_buffer.uCount = 0;
                eccParams.data_buffer.uOffset = 0;
                eccParams.cmd = ECC_CMD_ENCODE;
                if (hNTM->NtmInfo.fEdcProtectsMetadata)
                {
                    eccParams.metadata_buffer.buffer = pSpare; /* hNTM->pSpare; */
                    eccParams.metadata_buffer.uCount = 0;
                    eccParams.metadata_buffer.uOffset = 0;
                }
                else
                {
                    eccParams.metadata_buffer.buffer = NULL;
                    eccParams.metadata_buffer.uCount = 0;
                    eccParams.metadata_buffer.uOffset = 0;
                }
                /*  ECC buffers aren't needed for simply building the spare area.
                    ECCs will be stored directly into the spare area buffer.
                */
                eccParams.ecc_buffer = NULL;
                eccParams.reference_ecc_buffer = NULL;
                eccParams.pIoStat = &ioStat;
                eccParams.ulECCBufferByteCount = 0;
                eccParams.pEccProcessor = hNTM->pEccProcessor;

                FfxNtmPECCBuildSpareArea(hNTM->hDev, &hNTM->NtmInfo, &eccParams, pSpare, pTags, fUseEcc, hNTM->pChipInfo);

            }
            else
            {
                DclAssert(pTags == NULL);
                DclAssert(nTagSize == 0);
            }


            /*  Now send the spare area. If ECCs were not calculated, this will
                contain blank data.
            */
            NAND_DATA_OUT((NAND_IO_TYPE*)pSpare, hNTM->NtmInfo.uSpareSize/sizeof(NAND_IO_TYPE));

            /*  Now actually program the data..
                ExecuteChipCommand expects address in Addressable Units, so divide by BYTES_PER_ADDRESSABLE_UNIT
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
        pPages          += hNTM->NtmInfo.uPageSize;
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
        hNTM        - The NTM handle to use
        ulPage      - The flash offset in pages, relative to any
                      reserved space.
        pPages      - A buffer to receive the main page data.
        pSpares     - A buffer to receive the spare area data.
                      May be NULL.
        ulCount     - The number of pages to read.  The range of
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
            D_BUFFER   *pSpare = hNTM->pSpare;

            /*  We were not supplied any data to write into the spare
                area, so initialize the transfer buffer with all 0xFFs
            */
            DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

            NAND_DATA_OUT((NAND_IO_TYPE *) pSpare, (D_UINT16) SPARE_IO_COUNT);
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
    D_UINT32        ulLocalPage = ulPage;
    D_UINT32        ulLength;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenRead() StartPage=%lX Count=%lX TagSize=%u\n", ulPage, ulCount, nTagSize));

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
    DclAssert(hNTM->NtmInfo.pScratchSpareBuffer);
    DclAssert(hNTM->NtmInfo.pScratchPageBuffer);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);
    DclAssert(ulCount * nTagSize <= D_UINT16_MAX);
    ulLength = ulCount * nTagSize;

    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto HiddenReadCleanup;

    ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

    while(ulLength)
    {
        D_BUFFER        *pSpare = hNTM->NtmInfo.pScratchSpareBuffer;
        NAND_IO_TYPE    *pIO = (NAND_IO_TYPE *)pSpare;

        DclAssert(pSpare);
        ioStat.ffxStat = RandomReadSetup(hNTM, ulAddressableUnitAddr, 0);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Get spare area data. Don't assume we know where the tag is.
        */
        NAND_DATA_IN(pIO, SPARE_IO_COUNT);

        /*  TODO: Could be injecting the error into a portion of the spare
            area besides where the tag is, useless, but benign.
        FFXERRORINJECT_READ("HiddenRead",
                            ulAddressableUnitAddr / (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT),
                            spare.data,
                            sizeof spare.data,
                            &ioStat);
        */

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Decode the tags.
        */
        if (!FfxNtmPECCTagDecode(&(hNTM->NtmInfo), pTags, pSpare, FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo)))
        {

            /*  If tags come back invalid, and we know the tags are covered
                by the main page EDC, read the whole page to force the EDC
                mechanism into play. Then break out of the loop to give the
                error manager a chance to see any errors that occured,
                correctable or otherwise.
            */
            if (hNTM->NtmInfo.fEdcProtectsMetadata)
            {
            D_UINT32 ulTempCount = ioStat.ulCount;

                ioStat = PageRead(hNTM,
                                  ulLocalPage,
                                  hNTM->NtmInfo.pScratchPageBuffer,
                                  pTags,
                                  1,
                                  nTagSize,
                                  TRUE);

                /*  Look for FFXSTAT_SUCCESS here as well, because even if
                    a bit error occured in the original read of the spare
                    area, it's still possible that we have a flaky bit and
                    it didn't appear in this read operation.
                */
                if ((ioStat.ffxStat == FFXSTAT_SUCCESS) || (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA))
                    ioStat.ulCount = ulTempCount+1;
                else
                    ioStat.ulCount = ulTempCount;
                break;
            }
        }

        ioStat.ulCount++;

        ulAddressableUnitAddr  += (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);
        ulLength         -= nTagSize;
        pTags           += nTagSize;
        ulLocalPage++;
    }

  HiddenReadCleanup:

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
    D_BUFFER           *pSpare = hNTM->pSpare;
    NAND_IO_TYPE       *pIO;
    D_UINT32            ulOffsetAddressableUnit;

    pIO = (NAND_IO_TYPE *)pSpare;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
    DclAssert(pSpare);

    ulOffsetAddressableUnit = ulPage * (hNTM->NtmInfo.uPageSize / BYTES_PER_ADDRESSABLE_UNIT);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulOffsetAddressableUnit, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ulAddressableUnitAddr = ulPage * (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT);

        DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

        if(hNTM->uChipPageIndexLSB <= 8)     /* 512b page chip? */
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_SPARE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                goto HiddenWriteCleanup;
        }

        /*  If we are depending on the primary EDC mechanism to protect the
            tags, we need to generate ECCs here. We do that through
            BuildSpareArea. Otherwise, we just need to encode the tags.
        */
        if (hNTM->NtmInfo.fEdcProtectsMetadata)
        {
        FX_ECC_PARAMS       eccParams;

            /*  Only set the buffer pointers for main page and metadata
                here. The NTHelp stuff will use the spare area format info
                and the NTMInfo available to it to calculate counts and
                offsets.
            */
            DclMemSet(hNTM->NtmInfo.pScratchPageBuffer, ERASED8, hNTM->NtmInfo.uPageSize);
            eccParams.data_buffer.buffer = (D_BUFFER *)hNTM->NtmInfo.pScratchPageBuffer;
            eccParams.data_buffer.uCount = 0;
            eccParams.data_buffer.uOffset = 0;
            eccParams.cmd = ECC_CMD_ENCODE;
            if (hNTM->NtmInfo.fEdcProtectsMetadata)
            {
                eccParams.metadata_buffer.buffer = pSpare; /* hNTM->pSpare; */
                eccParams.metadata_buffer.uCount = 0;
                eccParams.metadata_buffer.uOffset = 0;
            }
            else
            {
                eccParams.metadata_buffer.buffer = NULL;
                eccParams.metadata_buffer.uCount = 0;
                eccParams.metadata_buffer.uOffset = 0;
            }
            /*  ECC buffers aren't needed for simply building the spare area.
                ECCs will be stored directly into the spare area buffer.
            */
            eccParams.ecc_buffer = NULL;
            eccParams.reference_ecc_buffer = NULL;
            eccParams.pIoStat = &ioStat;
            eccParams.ulECCBufferByteCount = 0;
            eccParams.pEccProcessor = hNTM->pEccProcessor;

            FfxNtmPECCBuildSpareArea(hNTM->hDev, &hNTM->NtmInfo, &eccParams, pSpare, pTag, TRUE, hNTM->pChipInfo);

        }
        else
        {
            FfxNtmPECCTagEncode(&(hNTM->NtmInfo), pSpare, pTag, FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo));
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

            NAND_DATA_OUT(pIO, SPARE_IO_COUNT);

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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-BlockErase() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


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
    DclAssert(hNTM->NtmInfo.fEdcParameterized);

    ioStat = FfxNtmPECCGetPageStatus(hNTM,
                                     &hNTM->NtmInfo,
                                     &ReadSpareArea,
                                     ulPage,
                                     hNTM->pChipInfo);

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
    DclAssert(hNTM->NtmInfo.fEdcParameterized);

    ioStat = FfxNtmPECCIsBadBlock(hNTM, &hNTM->NtmInfo, &ReadSpareArea, hNTM->pChipInfo, ulBlock);

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
    DclAssert(hNTM->NtmInfo.fEdcParameterized);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        ioStat = FfxNtmPECCSetBlockType(hNTM, &hNTM->NtmInfo, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-SetBlockStatus() Block=%lU BlockStat=%lX returning %s\n",
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


/*-------------------------------------------------------------------
    Local: RandomReadSetup()

    Prepare to read from a random offset within the spare area.

    Parameters:
        hNTM                    - The NTM handle to use
        ulAddressableUnitOffset - The offset of the page within the chip,
                                  in addressable units.
        nSpareOffset            - The offset within the spare area

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
                    unsigned int i;

                    NAND_ADDR_OUT(0);

                    /*  Read the device ID.  The first two bytes are always
                        the manufacturer ID and device ID, respectively.
                        The meaning (if any) of subsequent bytes depends on
                        the device.
                    */
                    for (i = 0; i < NAND_ID_SIZE; ++i)
                    {
                        NAND_IO_TYPE Temp;

                        NAND_DATA_IN(&Temp, 1);

                        pucID[i] = (D_UCHAR) BENDSWAP(Temp);
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
    D_UINT32    uPagesShiftCount;
    D_UINT32    ulLocalBlockAddr;

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
        Standard FFXSTATUS value
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

