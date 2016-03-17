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

    This module contains the RAM NAND Technology Module (NTM) which supports
    emulated NAND flash in RAM.  It does not interface with the a FlashFX
    Hooks module such as fhbyteio.c or fhpageio.c.  This NTM is typically
    only used for testing and debugging purposes.

    This NTM is designed to allow a specific flash manufacturer ID and flash
    ID to be specified, along with a number of chips.  For now these settings
    must be manually modified by changine the NTRAM_CHIP_MFG, NTRAM_CHIP_ID,
    and NTRAM_NUM_CHIPS defines.

    This NTM behaves exactly as a regular NTM with regard to ECCs.

    The following configuration option settings are used:

    FFXOPT_FIM_NANDRAM_MAIN   - This is an optional setting that defines the
                                base address of the memory block to use for
                                the main data area.  If this value is not set,
                                the NTM will allocate the memory block using
                                DclMemAlloc().

    FFXOPT_FIM_NANDRAM_SPARE  - This is an optional setting that defines the
                                base address of the memory block to use for
                                the spare data area.  If this value is not set,
                                the NTM will allocate the memory block using
                                DclMemAlloc().

    For a given project, a customized fhoption.c module may be used to provide
    project specific values for the settings described above.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntram.c $
    Revision 1.58  2010/12/14 00:37:50Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.57  2010/12/10 20:04:20Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.56  2009/12/22 04:33:53Z  keithg
    Changed warning messages regarding out of sequence writes and
    'skipped' page writes.  Made associated debug messages all level 2.
    Revision 1.55  2009/12/13 01:23:01Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Updated some typecasts which are necessary while migrating
    to the more flexible hooks model.  Enhanced the page validation.
    Revision 1.54  2009/10/06 20:49:50Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.53  2009/07/27 15:39:37Z  garyp
    Merged from the v4.0 branch.  Updated to support FFXPARAM_FIM_CHIPID.
    Added a default IORequest() function.  Modified so the PageWrite()
    functionality allows a page to be written with ECC, but no tags.  As
    before if ECC is turned off, then tags may not be written.  Updated
    to build cleanly when support for some NAND manufacturers is disabled.
    Revision 1.52  2009/04/09 03:42:55Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.51  2009/04/02 17:58:44Z  davidh
    Function headers updated for AuroDoc.
    Revision 1.50  2009/03/12 21:58:35Z  keithg
    Added static to function declarations that were defined as such.
    Revision 1.49  2009/03/04 07:04:01Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    Revision 1.48  2009/01/26 23:00:13Z  glenns
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    Revision 1.47  2009/01/23 17:43:36Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.46  2009/01/20 18:26:06Z  glenns
    - Added check for FFXSTAT_FIMCORRECTABLEDATA when
      reading a page to be sure page count is properly updated.
    Revision 1.45  2009/01/18 08:52:44Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.44  2009/01/16 23:59:18Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in eleven places.
    Revision 1.43  2008/10/08 20:14:08Z  keithg
    Conditioned reserved space (bounds) on BBM v5 since it is now
    completed by the device manager.  Modified the sequential page
    tracking to allow for unprogrammed pages.
    Revision 1.42  2008/09/19 17:44:16Z  thomd
    Fix incorrectly scaled spare offset in BlockErase().
    Revision 1.41  2008/09/12 14:42:25Z  thomd
    Cast page/block to size_t before multiplication;
    remove UINT64; corrected error in Validate() call.
    Revision 1.40  2008/09/05 16:45:45Z  thomd
    updated for large flash addressing
    Revision 1.39  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.38  2008/06/16 16:56:10Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.37  2008/03/23 18:59:09Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.36  2008/02/03 04:16:32Z  keithg
    Comment updates to support autodoc.
    Revision 1.35  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.34  2007/09/26 20:07:24Z  pauli
    Added an explicit D_BOOL cast to resolve a type mismatch warning.
    Revision 1.33  2007/09/25 22:17:38Z  pauli
    Set the default NAND flash to a small block, SSFDC part.
    Revision 1.32  2007/09/21 23:05:35Z  pauli
    Resolved Bugs 1455 and 1274: the RAM NTM can now correctly simulate SSFDC
    and OffsetZero style flash parts (small and large page/block).  Added the
    ability to report that BBM should or should not be used based on the
    characteristics of the NAND part identified.  Enhanced how the NTM tracks
    and validates writes to the flash.  Added a simplified way to specify which
    type of flash part to simulate.
    Revision 1.31  2007/09/12 20:15:03Z  Garyp
    Modified the way the last page written is tracked so that we detect both
    page overwrites as well as out of sequence writes.  Updated to work with
    tag sizes of 8 (useful if FlashFX is acting as an FMD).
    Revision 1.30  2007/09/07 21:05:35Z  thomd
    Add asserts for non-zero divisors; move assignment before use.
    Revision 1.29  2007/08/02 23:07:35Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.28  2007/04/07 03:29:05Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.27  2007/03/01 20:14:38Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.26  2007/02/13 23:51:56Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.
    Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to allow the call
    tree all the way up through the IoRequest to avoid having to range check
    (and/or split) requests.  Removed corresponding casts.
    Revision 1.25  2007/02/06 23:54:31Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.24  2007/01/03 23:50:43Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.23  2006/11/08 03:38:46Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.22  2006/10/04 02:32:02Z  Garyp
    Updated to use DclMemAllocZero().  Updated to use the new printf macros.
    Revision 1.21  2006/05/17 22:02:49Z  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    NtmInfo structure.
    Revision 1.20  2006/04/26 20:31:13Z  billr
    Pointers can be wider than 32 bits.
    Revision 1.19  2006/03/22 01:37:32Z  Garyp
    Updated the function header comments -- no functional changes.
    Revision 1.18  2006/03/12 18:59:22Z  Garyp
    Updated debug code.
    Revision 1.17  2006/03/10 01:18:37Z  Garyp
    Updated to use a modified NTM helper function.
    Revision 1.16  2006/03/07 02:54:12Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.15  2006/03/03 19:32:10Z  Garyp
    Updated to use the modified NTM helper functions.
    Revision 1.14  2006/02/27 06:54:01Z  Garyp
    Updated to handle tag encoding and decoding.  Modified PageRead() to return
    only a tag, rather than the entire spare area.  Added GetPageStatus().
    Revision 1.13  2006/02/24 04:34:29Z  Garyp
    Updated to use refactored headers.
    Revision 1.12  2006/02/23 23:40:00Z  Garyp
    Moved the meat of SpareRead/Write() into helper functions so that they
    may be used by various NTM functions which expect the flash offset to
    already be adjusted by reserved space.
    Revision 1.11  2006/02/18 19:47:05Z  Garyp
    Modified all functions to adjust for reserved space as if it was used, even
    though it will always be zero.  This will reduce errors for those creating
    new NTMs based on this code.
    Revision 1.10  2006/02/17 23:54:09Z  Garyp
    Modified the ECC hook functions to take an hDev parameter.
    Revision 1.9  2006/02/14 22:18:29Z  Garyp
    Notational change -- nothing functional.
    Revision 1.8  2006/02/11 03:09:58Z  Garyp
    Updated the NTM name.
    Revision 1.7  2006/02/09 22:25:46Z  Garyp
    Updated to no longer use the EXTMEDIAINFO structure.
    Revision 1.6  2006/01/25 04:00:42Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.5  2006/01/13 21:46:21Z  Garyp
    Added a chip ID for a 2KB page chip.
    Revision 1.4  2006/01/08 14:59:50Z  Garyp
    Eliminated support for IsWriteProtected() and GetChipInfo().
    Revision 1.3  2005/12/25 12:53:05Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/15 02:17:32Z  garyp
    Fixed some data type issue to avoid CE warnings.
    Revision 1.1  2005/12/02 18:54:34Z  Pauli
    Initial revision
    Revision 1.4  2005/12/02 18:54:34Z  Garyp
    Added the Get/SetBlockStatus() functions.  Modified a number of functions
    to return an FFXIOSTATUS rather than a FIMResult_t.  Added support for
    error injection tests.
    Revision 1.3  2005/11/14 17:51:57Z  Garyp
    Minor doc/formatting changes -- nothing functional.
    Revision 1.2  2005/11/07 18:19:18Z  Garyp
    Modified to work with any power-of-2 page size.
    Revision 1.1  2005/10/14 03:22:22Z  Garyp
    Initial revision
    Revision 1.2  2005/10/11 18:12:39Z  Garyp
    Modified the PageWrite() interface to take a page count.
    Revision 1.1  2005/10/06 06:51:10Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <deverr.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include "nandid.h"
#include "nand.h"
#include "ntm.h"

#define NTMNAME "NTRAM"

#define NTRAM_TAG_SIZE              (LEGACY_TAG_SIZE) /* only LEGACY_TAG_SIZE or 8 are supported */

#define NTRAM_CHIP_512_SSFDC        (1)
#define NTRAM_CHIP_512_ANYBITZERO   (2)
#define NTRAM_CHIP_512_OFFSETZERO   (3)
#define NTRAM_CHIP_512_NONE         (4)
#define NTRAM_CHIP_2KB_OFFSETZERO   (5)
#define NTRAM_CHIP_2KB_NONE         (6)

#define NTRAM_CHIP_TYPE             NTRAM_CHIP_512_SSFDC
#define NTRAM_NUM_CHIPS             (1)


/*  Special values for tracking the last page written in each block.
*/
#define NTRAM_LPW_NONE              (D_UINT16_MAX)
#define NTRAM_LPW_UNKNOWN           (D_UINT16_MAX - 1)

struct tagNTMDATA
{
    D_UINT32            ulTotalSize;
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE        hEI;
  #endif
    NTMINFO             NtmInfo;            /* information visible to upper layers */
    const FFXNANDCHIP  *pChipInfo;          /* chip information                    */
    FFXDEVHANDLE        hDev;
    PNANDCTL            pNC;
    D_BUFFER           *pMain;              /* space allocated for main array */
    D_BUFFER           *pSpare;             /* space allocated for spare array */
    D_BOOL              fWeAllocedMainBlock;
    D_BOOL              fWeAllocedSpareBlock;
    D_UINT16           *pauLastPageWritten; /* array of the last pages written per block */
    D_UINT16            uTagOffset;
    NANDFLASHINFO       nfi;
};

static PNANDCTL     FAKE_FfxHookNTRamCreate(FFXDEVHANDLE hDev, NANDFLASHINFO *pNFI, FFXFIMBOUNDS *pBounds);

static D_BOOL       Validate(NTMHANDLE hNTM, D_UINT32 ulStartPage, D_UINT32 ulLength, const char *pszFunc);
static D_BOOL       ValidatePageWrite(NTMHANDLE hNTM, D_UINT32 ulPage, const char *pszFunc);
static D_BOOL       InitializePageTracking(NTMHANDLE hNTM, D_BOOL fNewFlash);
static FFXIOSTATUS  ReadSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_ram =
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
    D_BOOL              fSuccess = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT), "NTRAM-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    /*  We don't use a NAND control module for the RAM NTM, so fake
        a pNC for good looks, along with emulating its functionality.
    */
    pNTM->pNC = FAKE_FfxHookNTRamCreate(hDev, &pNTM->nfi, NULL);

    if(!pNTM->pNC)
        goto CreateCleanup;

    pNTM->pChipInfo = FfxNandDecodeID(pNTM->nfi.abID);
    if(!pNTM->pChipInfo)
        goto CreateCleanup;

    pNTM->ulTotalSize = pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->pChipInfo->pChipClass->ulBlockSize * pNTM->nfi.uDevices;
    pNTM->NtmInfo.ulBlockSize = pNTM->pChipInfo->pChipClass->ulBlockSize;
    pNTM->NtmInfo.ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->nfi.uDevices;
    pNTM->ulTotalSize = pNTM->NtmInfo.ulTotalBlocks * pNTM->NtmInfo.ulBlockSize;

    pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;
    pNTM->NtmInfo.uPageSize = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->NtmInfo.uSpareSize = pNTM->pChipInfo->pChipClass->uSpareSize;
    pNTM->NtmInfo.uMetaSize = NTRAM_TAG_SIZE;
    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;
    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    
    /*  The following lines of code set  the NTM EDC capability and segment
        size based on the standard 1-bit EDC available through the nthelp
        facility. If other EDC is implemented in this NTM or its hooks,
        this code will need to be updated.
    */
    pNTM->NtmInfo.uEdcCapability     = 1;
    pNTM->NtmInfo.uEdcSegmentSize    = DATA_BYTES_PER_ECC;
    
    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

    /*  Calculate this once and store it for use later for
        both optimization and simplification of the code
    */
    DclAssert(pNTM->NtmInfo.uPageSize != 0);
    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

    if(!FfxHookOptionGet(FFXOPT_FIM_NANDRAM_MAIN, hDev, &pNTM->pMain, sizeof pNTM->pMain))
    {
        /*  If the RAM disk memory for the main array has not been
            supplied for us, allocate it now.
        */
        pNTM->pMain = DclMemAlloc(pNTM->ulTotalSize);
        if(!pNTM->pMain)
            goto CreateCleanup;

        /*  Erase the main area so that it appears as new flash.  If the client
            is supplying the spare area, it it up to him to put it in whatever
            initial format he prefers.
        */
        DclMemSet(pNTM->pMain, ERASED8, pNTM->ulTotalSize);

        pNTM->fWeAllocedMainBlock = TRUE;
    }

    if(!FfxHookOptionGet(FFXOPT_FIM_NANDRAM_SPARE, hDev, &pNTM->pSpare, sizeof pNTM->pSpare))
    {
        D_UINT32    ulLen;

        ulLen = (pNTM->ulTotalSize / pNTM->NtmInfo.uPageSize) * pNTM->NtmInfo.uSpareSize;

        /*  If the RAM disk memory for the spare array has not been
            supplied for us, allocate it now.

            (If it is already allocated, it darn well better be
            the "right" size...)
        */
        pNTM->pSpare = DclMemAlloc(ulLen);
        if(!pNTM->pSpare)
            goto CreateCleanup;

        /*  Erase the spare area so that it appears as new flash.  If the client
            is supplying the spare area, it it up to him to put it in whatever
            initial format he prefers.
        */
        DclMemSet(pNTM->pSpare, ERASED8, ulLen);

        pNTM->fWeAllocedSpareBlock = TRUE;
    }

    DclAssert(pNTM->NtmInfo.ulBlockSize != 0);

    /*  Determine which spare area format to use.
    */
    switch(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo))
    {
        case NSF_OFFSETZERO:
        {
            DclAssert(NSOFFSETZERO_TAG_LENGTH == LEGACY_ENCODED_TAG_SIZE);

            pNTM->uTagOffset = NSOFFSETZERO_TAG_OFFSET;
            break;
        }

        case NSF_SSFDC:
        {
            pNTM->uTagOffset = NSSSFDC_TAG_OFFSET;
            break;
        }

        default:
        {
            DclPrintf("FFX: The detected part requires an unsupported spare area format.\n");
            goto CreateCleanup;
        }
    }

    /*  Initialize the page write tracking mechanism.
        The D_BOOL cast is required because it may be a different type than
        that used by the compiler for a logical and operation.
    */
    if(!InitializePageTracking(pNTM, (D_BOOL)(pNTM->fWeAllocedMainBlock &&
                                              pNTM->fWeAllocedSpareBlock)))
        goto CreateCleanup;

  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

    DclPrintf("FFX: NTRAM device MainAddr=%P SpareAddr=%P Size=%lUKB\n",
        pNTM->pMain, pNTM->pSpare, pNTM->ulTotalSize / 1024UL);

    FFXPRINTF(1, ("NTRAM: TotalSize=%lX ChipBlocks=%lX BlockSize=%lX PageSize=%X\n",
        pNTM->ulTotalSize,
        pNTM->pChipInfo->pChipClass->ulChipBlocks,
        pNTM->pChipInfo->pChipClass->ulBlockSize,
        pNTM->pChipInfo->pChipClass->uPageSize));

    *ppNtmInfo = &pNTM->NtmInfo;
    fSuccess = TRUE;

  CreateCleanup:

    if(pNTM && !fSuccess)
    {
        /*  Free resources in reverse order
        */
        if(pNTM->pauLastPageWritten)
        {
            DclMemFree(pNTM->pauLastPageWritten);

            pNTM->pauLastPageWritten = NULL;
        }

        if(pNTM->pSpare && pNTM->fWeAllocedSpareBlock)
        {
            DclMemFree(pNTM->pSpare);

            pNTM->pSpare = NULL;
        }

        if(pNTM->pMain && pNTM->fWeAllocedMainBlock)
        {
            DclMemFree(pNTM->pMain);

            pNTM->pMain = NULL;
        }

        DclMemFree(pNTM);
        pNTM = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTRAM-Create() returning %P\n", pNTM));

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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0), "NTRAM-Destroy()\n"));

    DclAssert(hNTM);

    FFXERRORINJECT_DESTROY();

    /*  Free resources in reverse order
    */
    if(hNTM->pauLastPageWritten)
    {
        DclMemFree(hNTM->pauLastPageWritten);

        hNTM->pauLastPageWritten = NULL;
    }

    if(hNTM->pSpare && hNTM->fWeAllocedSpareBlock)
    {
        DclMemFree(hNTM->pSpare);

        hNTM->pSpare = NULL;
    }

    if(hNTM->pMain && hNTM->fWeAllocedMainBlock)
    {
        DclMemFree(hNTM->pMain);

        hNTM->pMain = NULL;
    }

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
static FFXIOSTATUS PageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            ulPageOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-PageRead() Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTRAM-PageRead", 0, ulCount);

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

    /*  Compute the offset
    */
    ulPageOffset = ulStartPage;

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulStartPage, ulCount, "PageRead"))
        goto PageReadCleanup;

    ioStat.ffxStat = FFXSTAT_SUCCESS;

    while(ulCount)
    {
/*        const */ D_BUFFER *pFlashSpare;
        size_t nOffset;

        /*  Make a pointer to the spare data in the flash array
        */
        pFlashSpare = hNTM->pSpare + ulPageOffset * hNTM->NtmInfo.uSpareSize;

        /*  Technically we really don't care about alignment issues here,
            however it's handy to do this in the same fashion as the real
            NTM in case we're analyzing our behavior with a profiler.
        */
        nOffset = (size_t)ulPageOffset * (size_t)hNTM->NtmInfo.uPageSize;
        if (((D_UINTPTR) pPages & (DCL_ALIGNSIZE-1)) != 0)
            DclMemCpy(pPages, hNTM->pMain+nOffset, hNTM->NtmInfo.uPageSize);
        else
            DclMemCpyAligned(pPages, hNTM->pMain+nOffset, hNTM->NtmInfo.uPageSize);

        /*  The caller may not care about the tag data, so copy it only
            if a pointer is supplied.
        */
        if(nTagSize)
        {
            DclAssert(pTags);
            DclAssert(nTagSize == NTRAM_TAG_SIZE);
            DclAssert(fUseEcc);

            /*  This error injection code is modifying the original data in
                the in-memory disk.  If a second bit error is ever injected
                at this same spot, before VBF re-uses the unit, we'll be in
                bad shape.
            */
            FFXERRORINJECT_READ("PageRead-Tag", ulPageOffset, pFlashSpare, LEGACY_ENCODED_TAG_SIZE, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

          #if NTRAM_TAG_SIZE == LEGACY_TAG_SIZE
            /*  If we are using a standard format, just use the standard
                helper function.
            */
            FfxNtmHelpTagDecode(pTags, &pFlashSpare[hNTM->uTagOffset]);

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                           "Encoded Tag=%lX Decoded Tag=%X\n",
                           *(D_UINT32 *)&pFlashSpare[hNTM->uTagOffset],
                           *(D_UINT16 *)pTags));
          #else
            #if NTRAM_TAG_SIZE != 8
            #error "NTRAM: unsupported tag size"
            #endif

            DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

            *(pTags+0) = pFlashSpare[NSSSFDC_TAG_OFFSET+0];
            *(pTags+1) = pFlashSpare[NSSSFDC_TAG_OFFSET+1];
            *(pTags+2) = pFlashSpare[NSSSFDC_TAG_OFFSET+2];
            *(pTags+3) = pFlashSpare[NSSSFDC_TAG_OFFSET+3];
            *(pTags+4) = pFlashSpare[NSSSFDC_RESERVED_1_OFFSET];
            *(pTags+5) = pFlashSpare[NSSSFDC_RESERVED_2_OFFSET];
            *(pTags+6) = pFlashSpare[NSSSFDC_RESERVED_3_OFFSET];
            *(pTags+7) = pFlashSpare[NSSSFDC_MARKEDBAD_OFFSET];
          #endif

            pTags += NTRAM_TAG_SIZE;
        }

        if(fUseEcc)
        {
            D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
            FFXIOSTATUS ioValid;

            FFXERRORINJECT_READ("PageRead", ulPageOffset, pPages, hNTM->NtmInfo.uPageSize, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            /*  An NTMHOOKHANDLE is the new paradigm, but essentially operates
                in the same fashion as the older PNANDCTL parameter.  Once this
                NTM is updated to the more flexible configuration model, the
                casts below can be removed.
            */

            FfxHookEccCalcStart((NTMHOOKHANDLE)hNTM->pNC, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);
            FfxHookEccCalcRead((NTMHOOKHANDLE)hNTM->pNC, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);

            ioValid = FfxHookEccCorrectPage((NTMHOOKHANDLE)hNTM->pNC, pPages, pFlashSpare, abECC, hNTM->pChipInfo);

            ioStat.ffxStat = ioValid.ffxStat;
            ioStat.op.ulPageStatus |= ioValid.op.ulPageStatus;

            if (ioValid.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
                ioStat.ulCount++;
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ulCount++;
        ulCount--;
        ulPageOffset++;
        pPages     += hNTM->NtmInfo.uPageSize;
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            ulPageOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-PageWrite() Page=%lX Count=%lX pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTRAM-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulStartPage, ulCount, "PageWrite"))
        goto PageWriteCleanup;

    ulPageOffset = ulStartPage;

    while(ulCount)
    {
        DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
        D_UCHAR            *pucTo;
        const D_BUFFER     *pcucFrom;
        int                 i;

        /*  Check for out of order writes and rewrites.
        */
        if(!ValidatePageWrite(hNTM, ulStartPage, "PageWrite"))
            goto PageWriteCleanup;

        /*  Initialize spare area and insert ECC codes
        */

        /*  An NTMHOOKHANDLE is the new paradigm, but essentially operates
            in the same fashion as the older PNANDCTL parameter.  Once this
            NTM is updated to the more flexible configuration model, the
            casts below can be removed.
        */

      #if NTRAM_TAG_SIZE == LEGACY_TAG_SIZE
        /*  If we are using a standard format, just use the standard
            helper function.
        */
        FfxNtmHelpBuildSpareArea((NTMHOOKHANDLE)hNTM->pNC, &hNTM->NtmInfo, pPages,
                             spare.data, pTags, fUseEcc, hNTM->pChipInfo);
      #else
        #if NTRAM_TAG_SIZE != 8
        #error "NTRAM: unsupported tag size"
        #endif

        DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

        /*  Not a standard format, so call the same function, but with a
            NULL tag pointer.  We will manually stuff in the tag value.
        */
        FfxNtmHelpBuildSpareArea((NTMHOOKHANDLE)hNTM->pNC, &hNTM->NtmInfo, pPages,
                             spare.data, NULL, fUseEcc, hNTM->pChipInfo);

        if(nTagSize)
        {
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
            DclAssert(pTags);

            spare.data[NSSSFDC_TAG_OFFSET+0]        = *(pTags+0);
            spare.data[NSSSFDC_TAG_OFFSET+1]        = *(pTags+1);
            spare.data[NSSSFDC_TAG_OFFSET+2]        = *(pTags+2);
            spare.data[NSSSFDC_TAG_OFFSET+3]        = *(pTags+3);
            spare.data[NSSSFDC_RESERVED_1_OFFSET]   = *(pTags+4);
            spare.data[NSSSFDC_RESERVED_2_OFFSET]   = *(pTags+5);
            spare.data[NSSSFDC_RESERVED_3_OFFSET]   = *(pTags+6);
            spare.data[NSSSFDC_MARKEDBAD_OFFSET]    = *(pTags+7);
        }
      #endif

        pucTo = hNTM->pMain + (size_t)ulPageOffset * (size_t)hNTM->NtmInfo.uPageSize;
        pcucFrom = pPages;
        for (i = 0; i < hNTM->NtmInfo.uPageSize; ++i)
            pucTo[i] &= pcucFrom[i];

        /*  The spare array has hNTM->NtmInfo.uSpareSize bytes for every
            hNTM->NtmInfo.uPageSize bytes in the main array.
        */
        pucTo = hNTM->pSpare + ulPageOffset * hNTM->NtmInfo.uSpareSize;
        pcucFrom = &spare.data[0];
        for (i = 0; i < hNTM->NtmInfo.uSpareSize; ++i)
            pucTo[i] &= pcucFrom[i];

        ioStat.ffxStat = FFXSTAT_SUCCESS;
        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("PageWrite", ulPageOffset, &ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulCount--;
        ulStartPage ++;
        ulPageOffset++;
        pPages     += hNTM->NtmInfo.uPageSize;
        if(nTagSize)
        {
            DclAssert(pTags);
            pTags   += NTRAM_TAG_SIZE;
        }
    }

  PageWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_BUFFER           *pSpare,
    D_UINT32            ulCount)
 {
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            ulPageOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-RawPageRead() Page=%lX Count=%lU pPages=%P pSpare=%P\n",
        ulPage, ulCount, pPages, pSpare));

    DclProfilerEnter("NTRAM-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    ulPageOffset = ulPage;

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "RawPageRead"))
        goto PageReadCleanup;

    ioStat.ffxStat = FFXSTAT_SUCCESS;

    while(ulCount)
    {
        size_t nOffset;

        /*  Technically we really don't care about alignment issues here,
            however it's handy to do this in the same fashion as the real
            NTM in case we're analyzing our behavior with a profiler.
        */
		nOffset = (size_t)ulPageOffset * (size_t)hNTM->NtmInfo.uPageSize;
        if (((D_UINTPTR) pPages & (DCL_ALIGNSIZE-1)) != 0)
            DclMemCpy(pPages, hNTM->pMain+nOffset, hNTM->NtmInfo.uPageSize);
        else
            DclMemCpyAligned(pPages, hNTM->pMain+nOffset, hNTM->NtmInfo.uPageSize);

        /*  The caller may not care about the spare data, so copy it only
            if a pointer is supplied.
        */
        if(pSpare)
        {
            const D_BUFFER *pFlashSpare;

            /*  Make a pointer to the spare data in the flash array
            */
            pFlashSpare = hNTM->pSpare + ulPageOffset * hNTM->NtmInfo.uSpareSize;

            DclMemCpy(pSpare, pFlashSpare, hNTM->NtmInfo.uSpareSize);

            pSpare  += hNTM->NtmInfo.uSpareSize;
        }

        ioStat.ulCount++;
        ulCount--;
        ulPageOffset++;
        pPages     += hNTM->NtmInfo.uPageSize;
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            ulPageOffset;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTRAM-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    ulPageOffset = ulPage;

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "RawPageWrite"))
        goto RawPageWriteCleanup;

    while(ulCount)
    {
        D_UCHAR    *pucTo;
        int         i;

        /*  Check for out of order writes and rewrites.
        */
        if(!ValidatePageWrite(hNTM, ulPage, "RawPageWrite"))
            goto RawPageWriteCleanup;

        pucTo = hNTM->pMain + ulPageOffset * hNTM->NtmInfo.uPageSize;
        for(i=0; i < hNTM->NtmInfo.uPageSize; i++)
            pucTo[i] &= pPages[i];

        /*  The spare array has hNTM->NtmInfo.uSpareSize bytes for every
            hNTM->NtmInfo.uPageSize bytes in the main array.
        */
        if(pSpares)
        {
            pucTo = hNTM->pSpare + (size_t)ulPageOffset * (size_t)hNTM->NtmInfo.uSpareSize;
            for(i=0; i < hNTM->NtmInfo.uSpareSize; i++)
            {
                pucTo[i] &= pSpares[i];
            }

            pSpares  += hNTM->NtmInfo.uSpareSize;
        }

        ioStat.ffxStat = FFXSTAT_SUCCESS;
        ioStat.ulCount++;

        ulPage++;
        ulCount--;
        ulPageOffset++;
        pPages       += hNTM->NtmInfo.uPageSize;
    }

  RawPageWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_BUFFER           *pBuffer)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "NTRAM-SpareRead() Page=%lX\n", ulPage));

    DclProfilerEnter("NTRAM-SpareRead", 0, 1);

    ioStat = ReadSpareArea(hNTM, ulPage, pBuffer);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTRAM-SpareRead() Page=%lX returning %s\n",
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
    const D_BUFFER     *pBuffer)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-SpareWrite() Page=%lX\n", ulPage));

    DclProfilerEnter("NTRAM-SpareWrite", 0, 1);

    ioStat = WriteSpareArea(hNTM, ulPage, pBuffer);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
        pBuffer     - Buffer for the hidden data read.
        nTagSize    - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_UINT32            ulCount,
    D_BUFFER           *pBuffer,
    unsigned            nTagSize)
{
    const D_BUFFER     *pFrom;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            uLength;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-HiddenRead() Page=%lX Count=%lX TagSize=%u\n", ulPage, ulCount, nTagSize));

    DclProfilerEnter("NTRAM-HiddenRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pBuffer);
    DclAssert(nTagSize == NTRAM_TAG_SIZE);

    uLength = ulCount * NTRAM_TAG_SIZE;

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "HiddenRead"))
        goto HiddenReadCleanup;

    ioStat.ffxStat = FFXSTAT_SUCCESS;

    pFrom = hNTM->pSpare + (size_t)ulPage * hNTM->NtmInfo.uSpareSize;

    while(uLength)
    {
        const D_BUFFER *pTemp = pFrom;

      #if FFXCONF_ERRORINJECTIONTESTS
        D_BUFFER    temp[LEGACY_ENCODED_TAG_SIZE];

        DclMemCpy(temp, pFrom, LEGACY_ENCODED_TAG_SIZE);

        /*  This error injection code is modifying the original data in
            the in-memory disk.  If a second bit error is ever injected
            at this same spot, before VBF re-uses the unit, we'll be in
            bad shape.
        */
/*        FFXERRORINJECT_READ("HiddenRead", ulPage, temp, LEGACY_ENCODED_TAG_SIZE, &ioStat); */
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        pTemp = temp;
      #endif

      #if NTRAM_TAG_SIZE == LEGACY_TAG_SIZE
        /*  If we are using a standard format, just use the standard
            helper function.
        */
        FfxNtmHelpTagDecode(pBuffer, &pTemp[hNTM->uTagOffset]);

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                       "Encoded Tag=%lX Decoded Tag=%X\n",
                       *(D_UINT32 *)&pTemp[hNTM->uTagOffset],
                       *(D_UINT16 *)pBuffer));
      #else

        DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

        *(pBuffer+0) = pTemp[NSSSFDC_TAG_OFFSET+0];
        *(pBuffer+1) = pTemp[NSSSFDC_TAG_OFFSET+1];
        *(pBuffer+2) = pTemp[NSSSFDC_TAG_OFFSET+2];
        *(pBuffer+3) = pTemp[NSSSFDC_TAG_OFFSET+3];
        *(pBuffer+4) = pTemp[NSSSFDC_RESERVED_1_OFFSET];
        *(pBuffer+5) = pTemp[NSSSFDC_RESERVED_2_OFFSET];
        *(pBuffer+6) = pTemp[NSSSFDC_RESERVED_3_OFFSET];
        *(pBuffer+7) = pTemp[NSSSFDC_MARKEDBAD_OFFSET];
      #endif

        ioStat.ulCount++;
        pFrom   += hNTM->NtmInfo.uSpareSize;
        pBuffer += NTRAM_TAG_SIZE;
        uLength -= NTRAM_TAG_SIZE;
    }

  HiddenReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTRAM-HiddenRead() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

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
    D_UCHAR            *pTo;
    unsigned            i;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclProfilerEnter("NTRAM-HiddenWrite", 0, 1);

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == NTRAM_TAG_SIZE);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "HiddenWrite"))
        goto HiddenWriteCleanup;

    /*  Check for out of order writes and rewrites.
    */
    if(!ValidatePageWrite(hNTM, ulPage, "HiddenWrite"))
        goto HiddenWriteCleanup;

    pTo = hNTM->pSpare + (ulPage * hNTM->NtmInfo.uSpareSize);

    /*  Initialize spare area and insert ECC codes
    */
  #if NTRAM_TAG_SIZE == LEGACY_TAG_SIZE
    {
        D_BUFFER    TagBuff[LEGACY_ENCODED_TAG_SIZE];

        FfxNtmHelpTagEncode(TagBuff, pTag);

        /*  Update the spare area with the new tag data.
        */
        for(i = 0; i < LEGACY_ENCODED_TAG_SIZE; ++i)
            pTo[i + hNTM->uTagOffset] &= TagBuff[i];
    }
  #else
    DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

    pTo[NSSSFDC_TAG_OFFSET+0]        = *(pTag+0);
    pTo[NSSSFDC_TAG_OFFSET+1]        = *(pTag+1);
    pTo[NSSSFDC_TAG_OFFSET+2]        = *(pTag+2);
    pTo[NSSSFDC_TAG_OFFSET+3]        = *(pTag+3);
    pTo[NSSSFDC_RESERVED_1_OFFSET]   = *(pTag+4);
    pTo[NSSSFDC_RESERVED_2_OFFSET]   = *(pTag+5);
    pTo[NSSSFDC_RESERVED_3_OFFSET]   = *(pTag+6);
    pTo[NSSSFDC_MARKEDBAD_OFFSET]    = *(pTag+7);
  #endif

    ioStat.ffxStat = FFXSTAT_SUCCESS;
    ioStat.ulCount = 1;

    FFXERRORINJECT_WRITE("HiddenWrite", ulPage, &ioStat);

  HiddenWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    NTMHANDLE       hNTM,
    D_UINT32        ulBlock)
{
    FFXIOSTATUS     ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "NTRAM-BlockErase() Block=%lX\n", ulBlock));


    /*  Check that this request is within the bounds of the flash.
    */
    if (!Validate(hNTM, ulBlock, hNTM->NtmInfo.uPagesPerBlock, "BlockErase"))
        goto BlockEraseCleanup;

    DclMemSet(hNTM->pMain + (size_t)ulBlock * hNTM->NtmInfo.ulBlockSize, ERASED8, hNTM->NtmInfo.ulBlockSize);

    /*  The spare array has hNTM->NtmInfo.uSpareSize bytes for every
        hNTM->NtmInfo.uPageSize bytes in the main array.
    */
    DclMemSet(hNTM->pSpare + (size_t)ulBlock * hNTM->NtmInfo.uSpareSize *
        (hNTM->NtmInfo.ulBlockSize/hNTM->NtmInfo.uPageSize),
        ERASED8, hNTM->NtmInfo.uPagesPerBlock * hNTM->NtmInfo.uSpareSize);

    ioStat.ffxStat = FFXSTAT_SUCCESS;
    ioStat.ulCount = 1;

    FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);

    /*  Reset the last page written for the block just erased.
    */
    hNTM->pauLastPageWritten[ulBlock] = NTRAM_LPW_NONE;

  BlockEraseCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTRAM-BlockErase() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

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

  #if NTRAM_TAG_SIZE == LEGACY_TAG_SIZE
    ioStat = FfxNtmHelpReadPageStatus(hNTM, &ReadSpareArea, ulPage, hNTM->pChipInfo);
  #else
    #if NTRAM_TAG_SIZE != 8
    #error "NTRAM: unsupported tag size"
    #endif
    {
        DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);

        /*  Read and scan the next common region of the flash.
        */
        ioStat = ReadSpareArea(hNTM, ulPage, (void *)buffer.ns);
        if(IOSUCCESS(ioStat, 1))
        {
            D_BUFFER        abTagTemp[NTRAM_TAG_SIZE];

            DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

            abTagTemp[0] = buffer.ns[NSSSFDC_TAG_OFFSET+0];
            abTagTemp[1] = buffer.ns[NSSSFDC_TAG_OFFSET+1];
            abTagTemp[2] = buffer.ns[NSSSFDC_TAG_OFFSET+2];
            abTagTemp[3] = buffer.ns[NSSSFDC_TAG_OFFSET+3];
            abTagTemp[4] = buffer.ns[NSSSFDC_RESERVED_1_OFFSET];
            abTagTemp[5] = buffer.ns[NSSSFDC_RESERVED_2_OFFSET];
            abTagTemp[6] = buffer.ns[NSSSFDC_RESERVED_3_OFFSET];
            abTagTemp[7] = buffer.ns[NSSSFDC_MARKEDBAD_OFFSET];

            /*  If the tag area, including the ECC and check bytes, is within
                1 bit of being erased, then we know that a tag was not written.
                If it is anything else, we know a tag was written.  This function
                always uses standard size tags.
            */
            if(!FfxNtmHelpIsRangeErased1Bit(abTagTemp, NTRAM_TAG_SIZE))
                ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(NTRAM_TAG_SIZE);

            if(ISWRITTENWITHECC(pNS[NSSSFDC_FLAGS_OFFSET]))
            {
                ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
            }
            else if(ISUNWRITTEN(pNS[NSSSFDC_FLAGS_OFFSET]))
            {
                ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
            }
            else
            {
                DclError();
            }
        }
    }
  #endif

     FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTRAM-GetPageStatus() Page=%lX returning %s\n",
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

    DclAssert(hNTM);

    ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pChipInfo, ulBlock);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTRAM-GetBlockStatus() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SetBlockStatus()

    This function sets the block status information for the
    given erase block.

    Parameters:
        hNTM          - The NTM handle to use
        ulBlock       - The flash offset, in blocks, relative to
                        any reserved space.
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
        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTRAM-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
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
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "NTRAM:ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
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
                ffxStat = DCLSTAT_SETUINT20(sizeof(hNTM->nfi.abID));
            }
            else
            {
                if(ulBuffLen)
                {
                    /*  Fill only as much of the buffer as is supplied by
                        the caller.  If the buffer is larger than needed,
                        the remaining bytes will be untouched.
                    */
                    ulBuffLen = DCLMIN(ulBuffLen, sizeof(hNTM->nfi.abID));
                    DclMemCpy(pBuffer, hNTM->nfi.abID, ulBuffLen);
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
            FFXPRINTF(1, ("NTRAM:ParameterGet() unhandled parameter ID=%x\n", id));
            ffxStat = FFXSTAT_BADPARAMETER;
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTRAM:ParameterGet() returning status=%lX\n", ffxStat));

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
        "NTRAM:ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTRAM:ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTRAM:ParameterSet() returning status=%lX\n", ffxStat));

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
    Local: FAKE_FfxHookNTRamCreate()

    This function creates a NAND Control Module instance which
    is associated with the ntram NTM.

    Parameters:
        hDev - The FFXDEVHANDLE
        pNFI - A pointer to the NANDFLASHINFO structure to fill

    Return Value:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
static PNANDCTL FAKE_FfxHookNTRamCreate(
    FFXDEVHANDLE        hDev,
    NANDFLASHINFO      *pNFI,
    FFXFIMBOUNDS       *pBounds)
{
    (void)hDev;
    (void)pBounds;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0), "FAKE_FfxHookNTRamCreate()\n"));

    DclMemSet(pNFI->abID, 0, NAND_ID_SIZE);

    switch(NTRAM_CHIP_TYPE)
    {
      #if FFXCONF_NANDSUPPORT_SAMSUNG
        case NTRAM_CHIP_512_SSFDC:
        {
            /*  Total   Page    FBB
                16MB    512B    SSFDC
            */
            pNFI->abID[0] = NAND_MFG_SAMSUNG;
            pNFI->abID[1] = 0x73;
            break;
        }

        case NTRAM_CHIP_512_OFFSETZERO:
        {
            /*  Total   Page    FBB
                16MB    512B    OFFSETZERO
            */
            pNFI->abID[0] = NAND_MFG_SAMSUNG;
            pNFI->abID[1] = 0x43;
            break;
        }

        case NTRAM_CHIP_2KB_OFFSETZERO:
        {
            /*  Total   Page    FBB
                128MB   2KB     OFFSETZERO
            */
            pNFI->abID[0] = NAND_MFG_SAMSUNG;
            pNFI->abID[1] = 0xA1;
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_TOSHIBA
        case NTRAM_CHIP_512_ANYBITZERO:
        {
            /*  Total   Page    FBB
                16MB    512B    ANYBITZERO
            */
            pNFI->abID[0] = NAND_MFG_TOSHIBA;
            pNFI->abID[1] = 0x73;
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_SPANSION
        case NTRAM_CHIP_512_NONE:
        {
            /*  Total   Page    FBB
                16MB    512B    NONE
            */
            pNFI->abID[0] = NAND_MFG_SPANSION;
            pNFI->abID[1] = 0x73;
            pNFI->abID[2] = 0x01;
            break;
        }

        case NTRAM_CHIP_2KB_NONE:
        {
            /*  Total   Page    FBB
                64MB    2KB     NONE
            */
            pNFI->abID[0] = NAND_MFG_SPANSION;
            pNFI->abID[1] = 0x81;
            pNFI->abID[2] = 0x01;
            break;
        }
      #endif

        default:
        {
            DclPrintf("FFX: NTRAM: Unknown Chip Type %u, or support is disabled\n", NTRAM_CHIP_TYPE);
            DclError();
            return NULL;
        }
    }

    pNFI->uDevices = NTRAM_NUM_CHIPS;

    return (PNANDCTL)1/*pNC*/;
}


/*-------------------------------------------------------------------
    Local: ReadSpareArea()

    This function reads the spare area for the given flash offset
    into the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pBuffer)
{
    D_BUFFER           *pFrom;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "NTRAM-ReadSpareArea() Page=%lX\n", ulPage));

    DclProfilerEnter("NTRAM-ReadSpareArea", 0, 1);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "ReadSpareArea"))
        goto SpareReadCleanup;

    /*  The spare array has hNTM->NtmInfo.uSpareSize
        bytes for every hNTM->NtmInfo.uPageSize bytes in the main array.
    */
    pFrom = hNTM->pSpare + (size_t)ulPage * hNTM->NtmInfo.uSpareSize;
    DclMemCpy(pBuffer, pFrom, hNTM->NtmInfo.uSpareSize);

    ioStat.ffxStat = FFXSTAT_SUCCESS;

/*  FFXERRORINJECT_READ("ReadSpareArea", ulPage, pBuffer, hNTM->NtmInfo.uSpareSize, &ioStat);
*/
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        ioStat.ulCount++;

  SpareReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTRAM-ReadSpareArea() Page=%lX returning %s\n",
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

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pBuffer)
{
    int                 i;
    D_UCHAR            *pTo;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTRAM-WriteSpareArea() Page=%lX\n", ulPage));

    DclProfilerEnter("NTRAM-WriteSpareArea", 0, 1);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "WriteSpareArea"))
        goto SpareWriteCleanup;

    /*  Check for out of order writes and rewrites.
    */
    if(!ValidatePageWrite(hNTM, ulPage, "WriteSpareArea"))
        goto SpareWriteCleanup;

    /*  The spare array has hNTM->NtmInfo.uSpareSize
        bytes for every hNTM->NtmInfo.uPageSize bytes in the main array.
    */
    pTo = hNTM->pSpare + (ulPage * hNTM->NtmInfo.uSpareSize);

    for (i = 0; i < hNTM->NtmInfo.uSpareSize; ++i)
        pTo[i] &= pBuffer[i];

    ioStat.ffxStat = FFXSTAT_SUCCESS;
    ioStat.ulCount = 1;

    FFXERRORINJECT_WRITE("WriteSpareArea", ulPage, &ioStat);

  SpareWriteCleanup:

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTRAM-WriteSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: Validate()

    Validate that the specified flash range, denoted by a start
    page and count, is valid.

    Parameters:

    Return Value:
        Returns TRUE if valid, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL Validate(
    NTMHANDLE       hNTM,
    D_UINT32        ulStartPage,
    D_UINT32        ulCount,
    const char     *pszFunc)
{
    D_BOOL          fResult = TRUE;
    D_UINT32        ulTotalPages;

    if(!hNTM)
    {
        DclError();
        return FALSE;
    }

    ulTotalPages = hNTM->ulTotalSize / hNTM->NtmInfo.uPageSize;

    if(ulStartPage >= ulTotalPages)
    {
        FFXPRINTF(1, ("NTRAM %s: StartPage %lX exceeds total pages %lX\n",
                      pszFunc, ulStartPage, ulTotalPages));

        fResult = FALSE;
    }

    if(ulCount == 0)
    {
        FFXPRINTF(1, ("NTRAM %s: count is zero\n", pszFunc));

        fResult = FALSE;
    }

    if(ulCount > (ulTotalPages - ulStartPage))
    {
        FFXPRINTF(1, ("NTRAM %s: StartPage %lX with count %lX exceeds total pages %lX\n",
                      pszFunc, ulStartPage, ulCount, ulTotalPages));

        fResult = FALSE;
    }

    DclAssert(fResult);

    return fResult;
}


/*-------------------------------------------------------------------
    Local: InitializePageTracking()

    Initializes the mechanism used to track page writes to each
    block for the purpose of ensuring that pages are written
    sequentially in each block.

    Parameters:

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL InitializePageTracking(
    NTMHANDLE       hNTM,
    D_BOOL          fNewFlash)
{
    D_UINT32        ulMemSize;
    D_UINT16        uLastPageWritten;
    D_UINT32        i;

    DclAssert(hNTM);
    DclAssert(hNTM->NtmInfo.uPagesPerBlock < (D_UINT16_MAX - 2));

    /*  Allocate memory for tracking sequential page writes within a block.
    */
    ulMemSize = (hNTM->ulTotalSize / hNTM->NtmInfo.ulBlockSize) *
                sizeof(*hNTM->pauLastPageWritten);
    hNTM->pauLastPageWritten = DclMemAlloc(ulMemSize);
    if(!hNTM->pauLastPageWritten)
        return FALSE;

    /*  Set the initial last page written value for each block based on what
        we know.  We don't have a readily available mechanism for determining
        what the last page written is for each block on "used" flash.  For
        freshly "created" flash we know that no pages have been written for
        any blocks.
    */
    if(fNewFlash)
        uLastPageWritten = NTRAM_LPW_NONE;
    else
        uLastPageWritten = NTRAM_LPW_UNKNOWN;
    for(i = 0; i < (hNTM->ulTotalSize / hNTM->NtmInfo.ulBlockSize); i++)
        hNTM->pauLastPageWritten[i] = uLastPageWritten;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ValidatePageWrite()

    Validate that requested write to a page is valid.  This
    includes checking that pages within a block are written
    in order and that pages are not overwritten.

    Parameters:

    Return Value:
        Always returns TRUE unless the NTM handle is not valid.
-------------------------------------------------------------------*/
static D_BOOL ValidatePageWrite(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    const char     *pszFunc)
{
    D_UINT32        ulBlock;
    D_UINT16        uPageInBlock;
    D_UINT16        uExpectedPage;

    if(!hNTM)
    {
        DclError();
        return FALSE;
    }

    /*  Calculate the block number and the page offset with the block
        for the requested write.
    */
    ulBlock = ulPage / hNTM->NtmInfo.uPagesPerBlock;
    uPageInBlock = (D_UINT16)(ulPage % hNTM->NtmInfo.uPagesPerBlock);

    /*  Determine the page we expect to be written for this block.
    */
    if(hNTM->pauLastPageWritten[ulBlock] == NTRAM_LPW_UNKNOWN)
    {
        /*  The last page written for this block is unknown.
            Set the now known last page written and get out.
        */
        hNTM->pauLastPageWritten[ulBlock] = uPageInBlock;
        return TRUE;
    }
    else if(hNTM->pauLastPageWritten[ulBlock] == NTRAM_LPW_NONE)
    {
        /*  This block is freshly erased so the next expected page is page 0.
        */
        uExpectedPage = 0;
    }
    else
    {
        /*  We know what that last page written is so we expect the next
            sequential page to be written.
        */
        uExpectedPage = hNTM->pauLastPageWritten[ulBlock] + 1;
    }

    /*  Check if the expected page is being written.
        Note that the warning messages below are debug level 2 - there are
        a few instances where these are deemed acceptable within normal
        FlashFX operations.  They are set to level 2 to minimize noise during
        typical debug sessions.
    */
    if(uPageInBlock == uExpectedPage)
    {
        /*  Record this page as the last page written for this block.
        */
        hNTM->pauLastPageWritten[ulBlock] = uPageInBlock;
    }
    else if(uPageInBlock < uExpectedPage)
    {
        FFXPRINTF(2, ("NTRAM: WARNING! %s() out of sequence write to Block %lU at Page=%U\n",
                  pszFunc, ulBlock, uPageInBlock));

        /*  DON'T record the page in this case.
        */
    }
    else
    {
        /*  Page writes in NAND flash must only be completed in an ascending order;
            the skip of a page is OK.  This warning is left in place only because
            this information may be interesting in certain cases.
        */
        FFXPRINTF(2, ("NTRAM: WARNING! %s() page write skipped Block %lU at Page %U, expected Page %U\n",
                  pszFunc, ulBlock, uPageInBlock, uExpectedPage));

        /*  We skipped some pages for some reason -- nevertheless, record
            the page...
        */
        hNTM->pauLastPageWritten[ulBlock] = uPageInBlock;
    }

    return TRUE;
}


#endif  /* FFXCONF_NANDSUPPORT */
