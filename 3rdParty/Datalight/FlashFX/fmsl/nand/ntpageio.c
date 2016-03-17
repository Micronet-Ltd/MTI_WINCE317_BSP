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

    This module contains the "PageIO" NAND Technology Module (NTM) which
    supports page oriented I/O to NAND flash.  It interfaces with the FlashFX
    Hooks module fhpageio.c, which allows easy project level customization of
    the interface without requiring a new NTM to be written.

    Virtual Page Emulation

    This module has the ability to emulate larger page sizes than what the
    physical flash supports.  However this requires special measures to
    ensure that the virtual pages are still written and interpreted in an
    atomic fashion.

    Therefore, if we are using virtual pages (pretending to be a larger page
    size than we really are).  We must do  extra validation to ensure that
    virtual pages are written atomically, with their associated metadata
    (tags).

    To do this, physical pages within each virtual page are written
    sequentially, and each physical page's spare area contains a duplicate
    copy of the tag.

    When reading pages, the tag value from last physical page in a virtual
    page is returned.  If it is not valid, then the entire virtual page is
    considered to be invalid.

    When reading tags, the tag is read from the final physical page as well.
    If it is programmed and valid (ECC check passes) all is good.  If it is
    bad, then the virtual page is bad.  If it is in the erased state, when
    then read the tag for the first physical page in the virtual page.  If it
    is also in the erased state, then the whole virtual page is considered to
    be valid (in an erased state).  If the tag is not in the erased state
    (valid or not), the whole virtual page is considered to be invalid, and
    returned as such.  It was probably trashed by a write interruption of some
    sort.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntpageio.c $
    Revision 1.47  2010/12/14 00:28:57Z  glenns
    Ensure that uEdcCapability is appropriately set if default FlashFX
    EDC mechanism is in use.
    Revision 1.46  2010/12/10 20:04:20Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.45  2009/12/11 21:38:12Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Updated some typecasts which are necessary while migrating
    to the more flexible hooks model.
    Revision 1.44  2009/10/06 20:48:28Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.43  2009/07/27 15:22:25Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written. Added a default IORequest()
    function.  Documentation updated.
    Revision 1.42  2009/04/09 03:42:55Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.41  2009/04/02 17:18:57Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.40  2009/03/26 06:52:08Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    - Fix Bugzilla #2467: Modified PageRead and PageWrite to pick
      up status from the hooks and propagate it upward.
    Revision 1.39  2009/03/04 07:12:42Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    Revision 1.38  2009/01/27 20:50:53Z  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.37  2009/01/27 00:13:57Z  glenns
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    - Updated PageRead to accomodate FFXSTAT_FIMCORRECTABLEDATA
      status.
    Revision 1.36  2009/01/23 17:52:57Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.35  2009/01/16 23:58:44Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in six places.
    Revision 1.34  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.33  2008/06/16 16:55:36Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.32  2008/03/23 18:59:10Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.  Implemented the RawPageRead/Write() interfaces.
    Revision 1.31  2008/02/03 05:38:04Z  keithg
    Comment updates to support autodoc.
    Revision 1.30  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.29  2007/09/25 22:19:43Z  pauli
    Corrected a typo in Create() that caused a compile error.
    Revision 1.28  2007/09/25 22:10:03Z  pauli
    Resolved Bug 1455: Updated to use new NTM helper functions.  Resolved Bug
    1341: Removed the use of byte offsets.  Added the ability to report that
    BBM should or should not be used based on the characteristics of the NAND
    part identified.  Added checks to ensure that the identified NAND part is
    supported.
    Revision 1.27  2007/09/13 21:18:20Z  jimmb
    defined initial ioStat = {0, FFXSTAT_SUCCESS, IOFLAGS_PAGE};
    Revision 1.26  2007/09/12 21:22:36Z  Garyp
    Updated to use some renamed functions.
    Revision 1.25  2007/08/02 23:01:07Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.24  2007/04/07 03:27:16Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.23  2007/04/02 18:03:38Z  rickc
    Fix for bug 928.  Removed requirement that structure had to match on-media
    spare area format.
    Revision 1.22  2007/03/01 20:11:40Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.21  2007/02/13 23:27:35Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.20  2007/02/06 20:37:57Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.19  2007/01/03 23:43:49Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.18  2006/11/08 03:38:46Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.17  2006/05/21 17:58:39Z  Garyp
    Modified to set the ulReservedBlocks and ulChipBlocks fields in the
    NtmInfo structure.
    Revision 1.16  2006/03/22 01:37:33Z  Garyp
    Updated the function header comments -- no functional changes.
    Revision 1.15  2006/03/17 02:03:36Z  Garyp
    Added logic to protect against initializing when the max page size is too
    small for the actual detected flash part.
    Revision 1.14  2006/03/16 12:16:59Z  Garyp
    Updated to ensure that writes are interpreted in an atomic fashion when
    virtual page emulation is turned on.
    Revision 1.13  2006/03/13 23:36:51Z  billr
    Correct USE_FILEIO_HOOKS_MODULE.
    Revision 1.12  2006/03/13 03:09:50Z  Garyp
    Updated to use the new tag encode/decode logic.  Added the ability to
    redirect the hooks functionality to the common fhpageio_file.c module,
    rather than the standard hooks module, to allow a file based flash
    emulation.
    Revision 1.11  2006/03/10 01:18:32Z  Garyp
    Updated to use a modified NTM helper function.
    Revision 1.10  2006/03/06 17:33:11Z  Garyp
    Updated to the current NTM model.
    Revision 1.9  2006/03/02 20:30:44Z  Garyp
    Updated to use the modified NTM helper functions.
    Revision 1.8  2006/02/24 04:25:08Z  Garyp
    Updated to use refactored headers.
    Revision 1.7  2006/02/23 23:45:45Z  Garyp
    Moved the meat of SpareRead/Write() into helper functions so that they
    may be used by various NTM functions which expect the flash offset to
    already be adjusted by reserved space.  Fixed to build properly with
    virtual page emulation turned off.
    Revision 1.6  2006/02/18 19:26:02Z  Garyp
    Added missing reserved space adjustments for Get/SetBlockStatus().
    Revision 1.5  2006/02/17 23:28:35Z  Garyp
    Updated to the new NTM model.
    Revision 1.4  2006/02/11 03:08:22Z  Garyp
    Updated debugging code.
    Revision 1.3  2006/01/25 04:00:43Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.2  2006/01/08 02:12:39Z  Garyp
    Eliminated support for IsWriteProtected() and GetChipInfo().
    Revision 1.1  2005/12/02 01:21:24Z  Pauli
    Initial revision
    Revision 1.5  2005/12/02 01:21:24Z  Garyp
    Added the Get/SetBlockStatus() functions.  Modified a number of functions
    to return an FFXIOSTATUS rather than a FIMResult_t.  Added support for
    error injection tests.
    Revision 1.4  2005/11/14 17:51:59Z  Garyp
    Minor cleanup -- no functional changes.
    Revision 1.3  2005/11/07 17:32:44Z  Garyp
    Updated to support any power-of-two page size, as well as larger page
    emulation on a small-page device.  Updated interface to the FlashFX
    Hooks layer.
    Revision 1.2  2005/10/22 23:48:39Z  garyp
    Modified to use the new FlashFX Hooks interface (fhpagio.c) rather
    than calling directly into REX.
    Revision 1.1  2005/10/14 03:22:22Z  Garyp
    Initial revision
    Revision 1.2  2005/10/11 18:26:42Z  Garyp
    Modified the PageWrite() interface to take a page count.  Modified
    HiddenRead() to use a DL4GR function that can read multiple hidden areas.
    Revision 1.1  2005/10/06 06:48:30Z  Garyp
    Initial revision
    Revision 1.6  2005/09/28 03:56:49Z  Garyp
    Eliminated some dead code.
    Revision 1.5  2005/09/15 23:03:39Z  garyp
    Modified to use 4GRrexint.h.
    Revision 1.4  2005/09/14 07:26:43Z  garyp
    Modified to use function prototypes in 4GRconf.h.
    Revision 1.3  2005/08/21 04:52:43Z  garyp
    Eliminated // comments.
    Revision 1.2  2005/08/09 15:52:12Z  pauli
    Added call to ReadID to initialize the flash.
    Revision 1.1  2005/08/05 18:48:02Z  pauli
    Initial revision
    Revision 1.7  2005/08/05 18:48:01Z  Garyp
    Updated to use revamped reserved space options which now allow
    reserved space at the top of the array.
    Revision 1.6  2005/08/03 19:17:24Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.4  2005/07/31 03:52:38Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.3  2005/07/30 21:08:47Z  Garyp
    Fixed a typo.
    Revision 1.2  2005/07/30 21:02:45Z  Garyp
    Updated to support read-ahead.
    Revision 1.1  2005/07/29 17:39:16Z  Garyp
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

#define NTMNAME "NTPAGEIO"


/*  Set USE_FILEIO_HOOKS_MODULE to TRUE to cause this code to use the
    hooks found in fhpageio_file.c, rather than the standard fhpageic.c
    hooks located in the project directory.

    This special hooks module will use standard file I/O to redirect
    all operations to a file.  See the comments within the hooks
    module for more information.
*/
#define USE_FILEIO_HOOKS_MODULE     FALSE    /* must be FALSE for checkin */

#if USE_FILEIO_HOOKS_MODULE
  #include "fhpageio_file.h"

  #define HookCreate            FfxHookNTPageFileCreate
  #define HookDestroy           FfxHookNTPageFileDestroy
  #define HookPageRead          FfxHookNTPageFileRead
  #define HookPageReadStart     FfxHookNTPageFileReadStart
  #define HookPageReadComplete  FfxHookNTPageFileReadComplete
  #define HookPageReadHidden    FfxHookNTPageFileReadHidden
  #define HookPageWrite         FfxHookNTPageFileWrite
  #define HookPageWriteStart    FfxHookNTPageFileWriteStart
  #define HookPageWriteComplete FfxHookNTPageFileWriteComplete
  #define HookPageEraseBlock    FfxHookNTPageFileEraseBlock
#else
  #define HookCreate            FfxHookNTPageCreate
  #define HookDestroy           FfxHookNTPageDestroy
  #define HookPageRead          FfxHookNTPageRead
  #define HookPageReadStart     FfxHookNTPageReadStart
  #define HookPageReadComplete  FfxHookNTPageReadComplete
  #define HookPageReadHidden    FfxHookNTPageReadHidden
  #define HookPageWrite         FfxHookNTPageWrite
  #define HookPageWriteStart    FfxHookNTPageWriteStart
  #define HookPageWriteComplete FfxHookNTPageWriteComplete
  #define HookPageEraseBlock    FfxHookNTPageEraseBlock
#endif

/*  Define VIRTUALPAGESIZE and VIRTUALSPARESIZE to allow larger page sizes
    to be emulated.  This provides dramatic performance benefits in some
    environments, particularly where the "ReadTags" functionality is slow,
    resulting in poor region mount performance.
*/
#if 0
  #define VIRTUALPAGESIZE       (2048)
  #define VIRTUALSPARESIZE      (64)
#endif

#ifdef VIRTUALPAGESIZE
  #if VIRTUALSPARESIZE == 0
    #error "FFX: ntpageio.c: VIRTUALSPARESIZE not initialized"
  #endif
  #if VIRTUALPAGESIZE > FFX_NAND_MAXPAGESIZE
    #error "FFX: ntpageio.c: VIRTUALPAGESIZE cannot be bigger than FFX_NAND_MAXPAGESIZE"
  #endif
  #define PAGESIZE          (hNTM->uVPageSize)
  #define SPARESIZE         (hNTM->uVSpareSize)
  #define V2PCOUNT(c)       ((D_UINT16)(hNTM->uPhysPagesPerVPage * (c)))
  #define V2PPAGE(c)        ((D_UINT32)(hNTM->uPhysPagesPerVPage * (c)))
  #define P2VCOUNT(c)       ((c) / hNTM->uPhysPagesPerVPage)
#else
  #define PAGESIZE          (hNTM->uPhysPageSize)
  #define SPARESIZE         (hNTM->uPhysSpareSize)
  #define V2PCOUNT(c)       (c)
  #define V2PPAGE(c)        (c)
  #define P2VCOUNT(c)       (c)
#endif

struct tagNTMDATA
{
    D_UINT16        uPhysPageSize;
    D_UINT16        uPhysSpareSize;
  #ifdef VIRTUALPAGESIZE
    D_UINT16        uVPageSize;         /* Virtual page size */
    D_UINT16        uVSpareSize;        /* Virtual spare size */
    D_UINT16        uPhysPagesPerVPage; /* number of phys pages in a virtual page */
    D_UINT16        uPadding;
  #endif
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE    hEI;
  #endif
    NTMINFO         NtmInfo;            /* information visible to upper layers */
    const FFXNANDCHIP   *pChipInfo;     /* chip information                    */
    FFXDEVHANDLE    hDev;
    PNANDCTL        pNC;
};

static FFXIOSTATUS ReadSpareArea(     NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS WriteSpareArea(    NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
#ifdef VIRTUALPAGESIZE
static FFXIOSTATUS ReadPhysSpareArea( NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS WritePhysSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
#endif

/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_pageio =
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
    D_UINT32            ulTotalSize;
    D_UINT32            ulTotalBlocks;
    D_UINT32            ulDeviceSize;
    D_BOOL              fSuccess = FALSE;
    FFXFIMBOUNDS        bounds;
    NANDFLASHINFO       nfi;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    /*  Get the array bounds now so we know how far to scan
    */
    FfxDevGetArrayBounds(hDev, &bounds);

    pNTM->pNC = HookCreate(hDev, &nfi, &bounds);
    if(!pNTM->pNC)
        goto CreateCleanup;

    pNTM->pChipInfo = FfxNandDecodeID(nfi.abID);
    if(!pNTM->pChipInfo)
        goto CreateCleanup;

    /*  Make sure it's a small-block, SSFDC part, that's all that's
        supported right now.
    */
    if(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo) != NSF_SSFDC)
    {
        DclPrintf("FFX: The detected NAND part is not supported by the ntpageio NTM.\n");
        goto CreateCleanup;
    }

    if(pNTM->pChipInfo->pChipClass->uPageSize > FFX_NAND_MAXPAGESIZE)
    {
        DclPrintf("FFX: FFX_NAND_MAXPAGESIZE is %u, but the flash detected has a page size of %U\n",
            FFX_NAND_MAXPAGESIZE, pNTM->pChipInfo->pChipClass->uPageSize);

        goto CreateCleanup;
    }

    ulDeviceSize = pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->pChipInfo->pChipClass->ulBlockSize;
    ulTotalSize = ulDeviceSize * nfi.uDevices;

    pNTM->uPhysPageSize         = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->uPhysSpareSize        = pNTM->pChipInfo->pChipClass->uSpareSize;

  #ifdef VIRTUALPAGESIZE
    if(pNTM->uPhysPageSize < VIRTUALPAGESIZE)
    {
        DclAssert(VIRTUALPAGESIZE % pNTM->uPhysPageSize == 0);

        pNTM->uVPageSize        = VIRTUALPAGESIZE;
        pNTM->uVSpareSize       = VIRTUALSPARESIZE;
        pNTM->uPhysPagesPerVPage = VIRTUALPAGESIZE / pNTM->uPhysPageSize;
    }
    else
    {
        pNTM->uVPageSize        = pNTM->uPhysPageSize;
        pNTM->uVSpareSize       = pNTM->uPhysSpareSize;
        pNTM->uPhysPagesPerVPage = 1;
    }

    pNTM->NtmInfo.uPageSize     = pNTM->uVPageSize;
    pNTM->NtmInfo.uSpareSize    = pNTM->uVSpareSize;

  #else

    pNTM->NtmInfo.uPageSize     = pNTM->uPhysPageSize;
    pNTM->NtmInfo.uSpareSize    = pNTM->uPhysSpareSize;

  #endif

    pNTM->NtmInfo.ulBlockSize   = pNTM->pChipInfo->pChipClass->ulBlockSize;
    ulTotalBlocks = ulTotalSize / pNTM->NtmInfo.ulBlockSize;
    pNTM->NtmInfo.ulTotalBlocks = ulTotalBlocks;

    /*  Calculate this once and store it for use later for
        both optimization and simplification of the code
    */
    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);


    pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;
    pNTM->NtmInfo.uMetaSize = FFX_NAND_TAGSIZE;
    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;

    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

    if( pNTM->pChipInfo->pChipClass->uEdcRequirement > 1 )
    {
        /* This chip requires more correction capabilities
           than this NTM can handle - fail the create.
        */
        FFXPRINTF(1, ("NTPAGEIO: Insufficient EDC capabilities.\n"));
        goto CreateCleanup;
    }

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

  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

    FFXPRINTF(1, ("NTPAGEIO: TotalBlocks=%lX ChipBlocks=%lX BlockSize=%lX PhysPageSize=%X\n",
        ulTotalBlocks,
        pNTM->pChipInfo->pChipClass->ulChipBlocks,
        pNTM->pChipInfo->pChipClass->ulBlockSize,
        pNTM->pChipInfo->pChipClass->uPageSize));

  #ifdef VIRTUALPAGESIZE
    FFXPRINTF(1, ("NTPAGEIO: VirtualPageSize=%U PhysPagesPerVirtualPage=%U\n",
        pNTM->uVPageSize, pNTM->uPhysPagesPerVPage));
  #endif

    /*  Make sure the parent has a pointer to our NTMINFO structure
    */
    *ppNtmInfo = &pNTM->NtmInfo;

    fSuccess = TRUE;

  CreateCleanup:

    if(pNTM && !fSuccess)
    {
        if(pNTM->pNC)
            HookDestroy(pNTM->pNC);

        DclMemFree(pNTM);
        pNTM = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTPAGEIO-Create() returning %P\n", pNTM));

    return pNTM;
}


/*-------------------------------------------------------------------
    Public: Destroy()

    This function destroys an NTM instance, and releases any
    allocated resources.

    Parameters:
        hNTM - The handle for the NTM instance to destroy.

    Return Value:
        None
-------------------------------------------------------------------*/
static void Destroy(
    NTMHANDLE           hNTM)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTPAGEIO-Destroy()\n"));

    DclAssert(hNTM);

    FFXERRORINJECT_DESTROY();

    HookDestroy(hNTM->pNC);

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
        ulStartPage - The starting page to read, relative to any
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
    D_BOOL              fFirstPage = TRUE;
    D_BOOL              fReading = FALSE;
    D_UINT32            ulPageNum;
    D_UINT16            uCompleted = 0;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (buffer, spare, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-PageRead() Page=%lU Count=%U pPages=%P pTags=%P TagSize=%u fUseECC=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTPAGEIO-PageRead", 0, ulCount);

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

    /*  Bug 1440
        This only supports SSFDC style flash.
    */
    DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

    /*  Convert the virtual page and count to a physical page and count.
    */
    ulCount = V2PCOUNT(ulCount);
    ulPageNum = V2PPAGE(ulStartPage);

    while(ulCount)
    {
        if(fFirstPage)
        {
            /*  For the first iteration through, we need to completely
                read a page.
            */
            ioStat = HookPageRead(hNTM->pNC, ulPageNum, 1, pPages, buffer.spare);

            fFirstPage = FALSE;
        }
        else
        {
            /*  For subsequent iterations, simply finish the previously
                started operation.
            */
            ioStat = HookPageReadComplete(hNTM->pNC, pPages, buffer.spare);

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                fReading = FALSE;
        }

        if(!IOSUCCESS(ioStat, 1))
        {
            DclError();
            break;
        }

        if(ulCount > 1)
        {
            /*  If there is another page to read <after> this one, get
                a jump start and start reading it now.
            */
            ioStat.ffxStat = HookPageReadStart(hNTM->pNC, ulPageNum+1);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                DclError();
                break;
            }

            fReading = TRUE;
        }

        if(fUseEcc)
        {
            D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
            FFXIOSTATUS ioValid;

            FFXERRORINJECT_READ("PageRead", ulPageNum, pPages, hNTM->uPhysPageSize, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            /*  An NTMHOOKHANDLE is the new paradigm, but essentially operates
                in the same fashion as the older PNANDCTL parameter.  Once this
                NTM is updated to the more flexible configuration model, the
                casts below can be removed.
            */                

            FfxHookEccCalcStart((NTMHOOKHANDLE)hNTM->pNC, pPages, hNTM->uPhysPageSize, abECC, ECC_MODE_READ);
            FfxHookEccCalcRead((NTMHOOKHANDLE)hNTM->pNC, pPages, hNTM->uPhysPageSize, abECC, ECC_MODE_READ);

            /*  We used to pass the spare size into the "CorrectPage"
                function, but for what purpose? -- appears we already
                have that data in the chip class information...
            */                
            DclAssert(hNTM->uPhysSpareSize == hNTM->pChipInfo->pChipClass->uSpareSize);

            ioValid = FfxHookEccCorrectPage((NTMHOOKHANDLE)hNTM->pNC, pPages, buffer.spare, abECC, hNTM->pChipInfo);

            ioStat.ffxStat = ioValid.ffxStat;
            ioStat.op.ulPageStatus |= ioValid.op.ulPageStatus;

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
                    ioStat.ulCount++;
                break;
            }
        }

        ulCount--;

        /*  The caller may not care about the tag data, so copy it only
            if a pointer is supplied.

            Also ensure that this is the last physical page in any
            virtual page.
        */
        if(fUseEcc && nTagSize && (ulCount % V2PCOUNT(1) == 0))
        {
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            FFXERRORINJECT_READ("PageRead-Tag", ulPageNum, buffer.spare, LEGACY_ENCODED_TAG_SIZE, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            FfxNtmHelpTagDecode(pTags, buffer.spare);

            pTags += nTagSize;
        }

        uCompleted++;
        ulPageNum++;
        pPages += hNTM->uPhysPageSize;
    }

    /*  Terminate any read operation in progress.
    */
    if(fReading)
    {
        /*  We only get into this code if there was a prior error,
            so ignore any error codes at this level.
        */
        HookPageReadComplete(hNTM->pNC, NULL, NULL);
    }

/*  PageReadCleanup: */

    /*  Return the count of virtual pages read
    */
    ioStat.ulCount = P2VCOUNT(uCompleted);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-PageRead() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

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
    D_BOOL              fWriting = FALSE;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_INTERNAL_ERROR);
    D_UINT32            ulCompleted = 0;    /* physical pages written */
    NTMINFO             ni;
    D_UINT32            ulPageNum;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-PageWrite() Start Page=%lU Count=%U TagSize=%u fUseECC=%U\n",
        ulStartPage, ulCount, nTagSize, fUseEcc));

    DclProfilerEnter("NTPAGEIO-PageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  The tag pointer and tag size are either always both set or both
        clear.  fUseEcc is FALSE, then the tag is never used, however if
        fUseEcc is TRUE, tags may or may not be used.  Assert it so.
    */
    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));
    DclAssert((fUseEcc) || (!pTags && !nTagSize));


    /*  Make a local copy of the NtmInfo structure which we can modify in
        the event that we are using virtual pages -- we must pass the
        physical page information to FfxNtmHelpBuildSpareArea().
    */
    ni = hNTM->NtmInfo;
    ni.uPageSize /= V2PCOUNT(1);
    ni.uSpareSize /= V2PCOUNT(1);

    /*  Convert the virtual page and count to a physical page and count.
    */
    ulCount = V2PCOUNT(ulCount);
    ulPageNum = V2PPAGE(ulStartPage);

    while(ulCount)
    {
        DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

        /*  Initialize spare area, insert ECC codes and possibly a tag.
        */

        /*  An NTMHOOKHANDLE is the new paradigm, but essentially operates
            in the same fashion as the older PNANDCTL parameter.  Once this
            NTM is updated to the more flexible configuration model, the
            cast below can be removed.
        */                
        FfxNtmHelpBuildSpareArea((NTMHOOKHANDLE)hNTM->pNC, &ni, pPages, spare.data, pTags, fUseEcc, hNTM->pChipInfo);

        if(fWriting)
        {
            ioStat = HookPageWriteComplete(hNTM->pNC);

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                goto PageWriteCleanup;

            fWriting = FALSE;
            ulCompleted++;
        }

      #if FFXCONF_ERRORINJECTIONTESTS
        if(ulCompleted % V2PCOUNT(1) == 0)
        {
            ioStat.ulCount = P2VCOUNT(ulCompleted);

            FFXERRORINJECT_WRITE("PageWrite", ulPageNum, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                ulCompleted--;
                break;
            }
        }
      #endif

        /*  Write the actual data to the flash part's internal buffer
        */
        ioStat.ffxStat = HookPageWriteStart(hNTM->pNC, ulPageNum, pPages, spare.data);
        if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            goto PageWriteCleanup;
        }

        fWriting = TRUE;

        ulCount--;

        /*  If we have moved onto the next virtual page, adjust the
            tag pointer.
        */
        if(nTagSize && ulCount % V2PCOUNT(1) == 0)
        {
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            pTags += nTagSize;
        }

        ulPageNum++;
        pPages += hNTM->uPhysPageSize;
    }

    if(fWriting)
    {
        ioStat = HookPageWriteComplete(hNTM->pNC);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            goto PageWriteCleanup;

        ulCompleted++;
    }

    if(ulCount == 0)
        ioStat.ffxStat = FFXSTAT_SUCCESS;

  PageWriteCleanup:

    /*  Return the count of virtual pages written
    */
    ioStat.ulCount = P2VCOUNT(ulCompleted);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-PageWrite() returning IOStatus: Count=%lU Status=%lX\n",
        ioStat.ulCount, ioStat.ffxStat));

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
        hNTM        - The NTM handle to use
        ulStartPage - The starting page to read, relative to
                      any reserved space.
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
    D_UINT32            ulStartPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pSpares,
    D_UINT32            ulCount)
 {
    D_BOOL              fFirstPage = TRUE;
    D_BOOL              fReading = FALSE;
    D_UINT32            ulPageNum;
    D_UINT16            uCompleted = 0;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-RawPageRead() Page=%lU Count=%lU pPages=%P pSpares=%P\n",
        ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTPAGEIO-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Convert the virtual page and count to a physical page and count.
    */
    ulCount = V2PCOUNT(ulCount);
    ulPageNum = V2PPAGE(ulStartPage);

    while(ulCount)
    {
        if(fFirstPage)
        {
            /*  For the first iteration through, we need to completely
                read a page.
            */
            ioStat = HookPageRead(hNTM->pNC, ulPageNum, 1, pPages, pSpares);

            fFirstPage = FALSE;
        }
        else
        {
            /*  For subsequent iterations, simply finish the previously
                started operation.
            */
            ioStat = HookPageReadComplete(hNTM->pNC, pPages, pSpares);

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                fReading = FALSE;
        }

        if(!IOSUCCESS(ioStat, 1))
        {
            DclError();
            break;
        }

        if(ulCount > 1)
        {
            /*  If there is another page to read <after> this one, get
                a jump start and start reading it now.
            */
            if(HookPageReadStart(hNTM->pNC, ulPageNum+1) != FFXSTAT_SUCCESS)
            {
                DclError();
                ioStat.ffxStat = FFXSTAT_FIMREADSTARTFAILED;
                break;
            }

            fReading = TRUE;
        }

        ulCount--;

        uCompleted++;
        ulPageNum++;
        pPages += hNTM->uPhysPageSize;
        if(pSpares)
            pSpares += hNTM->uPhysSpareSize;
    }

    /*  Terminate any read operation in progress.
    */
    if(fReading)
    {
        /*  We only get into this code if there was a prior error,
            so ignore any error codes at this level.
        */
        HookPageReadComplete(hNTM->pNC, NULL, NULL);
    }

/*  PageReadCleanup: */

    /*  Return the count of virtual pages read
    */
    ioStat.ulCount = P2VCOUNT(uCompleted);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: RawPageWrite()

    Write data to the entire page (main and spare area) with no
    ECC processing or other interpretation or formatting.

    Not all NTMs support this function; not all NAND controllers
    can support it.

    Parameters:
        hNTM        - The NTM handle to use
        ulStartPage - The starting page to write, relative to
                      any reserved space.
        pPages      - A pointer to the main page data to write.
        pSpares     - A pointer to data to write in the spare area.
                      This value may be NULL if nothing is to be
                      written into the spare area.
        ulCount     - The number of pages to write.  The range of
                      pages must not cross an erase block boundary.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS RawPageWrite(
    NTMHANDLE           hNTM,
    D_UINT32            ulStartPage,
    const D_BUFFER     *pPages,
    const D_BUFFER     *pSpares,
    D_UINT32            ulCount)
{
    D_BOOL              fWriting = FALSE;
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_INTERNAL_ERROR);
    D_UINT32            ulCompleted = 0;    /* physical pages written */
    D_UINT32            ulPageNum;
    DCLALIGNEDBUFFER    (buffer, spare, FFX_NAND_MAXSPARESIZE);
    const D_BUFFER     *pWorkSpare = pSpares;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-RawPageWrite() Page=%lU Count=%lU pPages=%P pSpares=%P\n",
        ulStartPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTPAGEIO-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    if(!pWorkSpare)
    {
        pWorkSpare = buffer.spare;
        DclMemSet(buffer.spare, ERASED8, sizeof buffer.spare);
    }

    /*  Convert the virtual page and count to a physical page and count.
    */
    ulCount = V2PCOUNT(ulCount);
    ulPageNum = V2PPAGE(ulStartPage);

    while(ulCount)
    {
        if(fWriting)
        {
            ioStat = HookPageWriteComplete(hNTM->pNC);

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                goto PageWriteCleanup;

            fWriting = FALSE;
            ulCompleted++;
        }

      #if FFXCONF_ERRORINJECTIONTESTS
        if(ulCompleted % V2PCOUNT(1) == 0)
        {
            ioStat.ulCount = P2VCOUNT(ulCompleted);

            FFXERRORINJECT_WRITE("PageWrite", ulPageNum, &ioStat);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                ulCompleted--;
                break;
            }
        }
      #endif

        /*  Write the actual data to the flash part's internal buffer
        */
        if(HookPageWriteStart(hNTM->pNC, ulPageNum, pPages, pWorkSpare) != FFXSTAT_SUCCESS)
        {
            ioStat.ffxStat = FFXSTAT_FIMWRITESTARTFAILED;
            goto PageWriteCleanup;
        }

        fWriting = TRUE;

        ulCount--;
        ulPageNum++;
        pPages += hNTM->uPhysPageSize;
        if(pSpares)
            pWorkSpare += hNTM->uPhysSpareSize;
    }

    if(fWriting)
    {
        ioStat = HookPageWriteComplete(hNTM->pNC);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            goto PageWriteCleanup;

        ulCompleted++;
    }

    if(ulCount == 0)
        ioStat.ffxStat = FFXSTAT_SUCCESS;

  PageWriteCleanup:

    /*  Return the count of virtual pages written
    */
    ioStat.ulCount = P2VCOUNT(ulCompleted);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareRead()

    Read the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The page to read, relative to any reserved space.
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
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-SpareRead() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-SpareRead", 0, 1);

    ioStat = ReadSpareArea(hNTM, ulPage, pSpare);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-SpareRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SpareWrite()

    Write the spare area of one page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The page to write, relative to any reserved space.
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
        "NTPAGEIO-SpareWrite() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-SpareWrite", 0, 1);

    ioStat = WriteSpareArea(hNTM, ulPage, pSpare);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenRead()

    Read the hidden (tag) data from multiple pages.

    Parameters:
        hNTM       - The NTM handle to use
        ulPage     - The starting page to read, relative to any
                     reserved space.
        ulTagCount - The number of tags to read.
        pTags      - Buffer for the hidden data read.
        nTagSize   - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely read
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_UINT32            ulTagCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_INTERNAL_ERROR);
    D_UINT32            ulPageNum;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-HiddenRead() Page=%lU Count=%lU TagSize=%u\n", ulPage, ulTagCount, nTagSize));

    DclProfilerEnter("NTPAGEIO-HiddenRead", 0, ulTagCount);

    DclAssert(hNTM);
    DclAssert(ulTagCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Convert the virtual page to a physical page.
    */
    ulPageNum = V2PPAGE(ulPage);

    while(ulTagCount)
    {
        #define             MAX_TAGS    (32)
        DCLALIGNEDBUFFER    (tag, data, (MAX_TAGS * LEGACY_ENCODED_TAG_SIZE));
        D_BUFFER           *pTemp;
        FFXIOSTATUS         ioTemp;
        D_UINT32            ulCount = DCLMIN(ulTagCount, MAX_TAGS);

        pTemp = &tag.data[0];

        /*  Read a bunch of encoded tags into our temporary buffer.  If
            we are using virtual pages, we are reading the tag data from
            the last physical page in each virtual page.
        */
        ioTemp = HookPageReadHidden(hNTM->pNC, ulPageNum+V2PCOUNT(1)-1,
                                    ulCount, V2PCOUNT(1), pTemp);
        if(!IOSUCCESS(ioTemp, ulCount))
        {
            ioStat.ffxStat = ioTemp.ffxStat;
            goto HiddenReadCleanup;
        }

        while(ulCount)
        {
          #ifdef VIRTUALPAGESIZE
            if(FfxNtmHelpTagDecode(pTags, pTemp))
            {
                /*  If we are using virtual pages (pretending to be a larger
                    page size than we really are).  We must do extra validation
                    to ensure that virtual pages are written atomically, with
                    their associated metadata (tags).
                */
                DCLALIGNEDBUFFER    (tag, temp, LEGACY_ENCODED_TAG_SIZE);

                DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));

                /*  Does the tag on the final physical page claim to be erased?
                */
                if(*(D_UINT32*)pTemp == ERASED32)
                {
                    /*  If so, the tag decode routine should have output
                        an erased value for the decoded tag.
                    */
                    DclAssert(*(D_UINT16*)pTags == ERASED16);

                    /*  Read the encoded tag value for the first physical
                        page in this virtual page.
                    */
                    ioTemp = HookPageReadHidden(hNTM->pNC, ulPageNum, 1, 0, tag.temp);
                    if(!IOSUCCESS(ioTemp, 1))
                    {
                        ioStat.ffxStat = ioTemp.ffxStat;
                        goto HiddenReadCleanup;
                    }

                    /*  Is it erased as well?
                    */
                    if(*(D_UINT32*)&tag.temp[0] != ERASED32)
                    {
                        /*  The tag in the last physical page is in the erased
                            state, however the tag in the first physical page
                            is not, therefore the page is trashed.  Zero the
                            tag field so higher level software knows it is
                            bogus.
                        */
                        DclMemSet(pTags, 0, nTagSize);
                    }
                }
            }
          #else
            FfxNtmHelpTagDecode(pTags, pTemp);
          #endif

            ulCount--;
            ulTagCount--;
            ioStat.ulCount++;
            ulPageNum       += V2PCOUNT(1);
            pTags           += nTagSize;
            pTemp           += LEGACY_ENCODED_TAG_SIZE;
        }
    }

    ioStat.ffxStat = FFXSTAT_SUCCESS;
/*
    FFXERRORINJECT_READ("HiddenRead", ulPage, pTags, uLength, &ioStat);
*/
  HiddenReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenWrite()

    Write the hidden (tag) data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The page to write, relative to any reserved space.
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
    DCLALIGNEDBUFFER    (buffer, spare, FFX_NAND_MAXSPARESIZE);
    FFXIOSTATUS         ioStat;
    D_BUFFER           *pNS = buffer.spare;
    D_UINT16            uCount;
    D_UINT32            ulPageNum;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-HiddenWrite() Page=%lU Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclProfilerEnter("NTPAGEIO-HiddenWrite", 0, 1);

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Bug 1440
        This won't work correctly for OffsetZero style flash.
    */
    DclAssert(FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo) == NSF_SSFDC);

    DclMemSet(buffer.spare, ERASED8, hNTM->NtmInfo.uSpareSize);

    /*  Convert the virtual page to a physical page.
    */
    ulPageNum = V2PPAGE(ulPage);

    /*  If we are using virtual pages, we need to write the tag to
        each of the physical pages.
    */
    uCount = V2PCOUNT(1);

    FfxNtmHelpTagEncode(&pNS[NSSSFDC_TAG_OFFSET], pTag);

    while(uCount)
    {
        ioStat = HookPageWrite(hNTM->pNC, ulPageNum, 1, NULL, buffer.spare);
        if(!IOSUCCESS(ioStat, 1))
            break;

        ulPageNum++;
        uCount--;
    }

    if(uCount == 0)
        ioStat.ffxStat = FFXSTAT_SUCCESS;

    /*  Return the count of virtual pages written
    */
    ioStat.ulCount = P2VCOUNT(ioStat.ulCount);

    FFXERRORINJECT_WRITE("HiddenWrite", ulPageNum, &ioStat);

/*  HiddenWriteCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: BlockErase()

    This function erases a block (physical erase zone) of a
    NAND chip.

    Parameters:
        hNTM    - The NTM handle
        ulBlock - The block to erase, relative to any
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTPAGEIO-BlockErase() Block=%lU\n", ulBlock));

    ioStat = HookPageEraseBlock(hNTM->pNC, ulBlock);

    FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);

/*  BlockEraseCleanup: */

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetPageStatus()

    This function retrieves the page status information for the
    given page.

    Parameters:
        hNTM   - The NTM handle to use
        ulPage - The page to query, relative to any reserved space.

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
        "NTPAGEIO-GetPageStatus() Page=%lU returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: GetBlockStatus()

    This function retrieves the block status information for the
    given erase block.

    Parameters:
        hNTM    - The NTM handle to use
        ulBlock - The block to query, relative to any reserved space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulBlockStatus variable will contain the block status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetBlockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    FFXIOSTATUS         ioStat;

    DclAssert(hNTM);

  #ifdef VIRTUALPAGESIZE
    ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadPhysSpareArea, hNTM->pChipInfo, ulBlock);
  #else
    ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pChipInfo, ulBlock);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTPAGEIO-GetBlockStatus() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: SetBlockStatus()

    This function sets the block status information for the
    given erase block.

    Parameters:
        hNTM          - The NTM handle to use
        ulBlock       - The block to set, relative to any reserved space.
        ulBlockStatus - The block status information to set.

    Return Value:
        Returns an FFXIOSTATUS structure with standard I/O status
        information.
-------------------------------------------------------------------*/
static FFXIOSTATUS SetBlockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock,
    D_UINT32            ulBlockStatus)
{
    FFXIOSTATUS         ioStat = DEFAULT_BLOCKIO_STATUS;

    DclAssert(hNTM);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
      #ifdef VIRTUALPAGESIZE
        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadPhysSpareArea, &WritePhysSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
      #else
        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
      #endif
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        "NTPAGEIO-SetBlockStatus() Block=%lX BlockStat=%lX returning %s\n",
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
        "NTPAGEIO:ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTPAGEIO:ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTPAGEIO:ParameterGet() returning status=%lX\n", ffxStat));

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
        "NTPAGEIO:ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTPAGEIO:ParameterSet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTPAGEIO:ParameterSet() returning status=%lX\n", ffxStat));

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

    This function reads the spare area for the given page
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
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-ReadSpareArea() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-ReadSpareArea", 0, 1);

    ioStat = HookPageRead(hNTM->pNC, V2PPAGE(ulPage), V2PCOUNT(1), NULL, pSpare);

    /*  Convert returned count from phys back into virtual
    */
    ioStat.ulCount = P2VCOUNT(ioStat.ulCount);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-ReadSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-WriteSpareArea() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-WriteSpareArea", 0, 1);

    ioStat = HookPageWrite(hNTM->pNC, V2PPAGE(ulPage), V2PCOUNT(1), NULL, pSpare);

    /*  Convert returned count from phys back into virtual
    */
    ioStat.ulCount = P2VCOUNT(ioStat.ulCount);

    FFXERRORINJECT_WRITE("WriteSpareArea", ulPage, &ioStat);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-WriteSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#ifdef VIRTUALPAGESIZE

/*-------------------------------------------------------------------
    Local: ReadPhysSpareArea()

    This function reads the spare area for the given flash offset
    into the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadPhysSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-ReadPhysSpareArea() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-ReadPhysSpareArea", 0, 1);

    ioStat = HookPageRead(hNTM->pNC, ulPage, 1, NULL, pSpare);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-ReadPhysSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: WritePhysSpareArea()

    This function writes the spare area for the given flash
    offset from the supplied buffer.

    This function is used internally by the NTM and various NTM
    helper functions, and must use a flash offset which has
    already been adjusted for reserved space.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WritePhysSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTPAGEIO-WritePhysSpareArea() Page=%lU\n", ulPage));

    DclProfilerEnter("NTPAGEIO-WritePhysSpareArea", 0, 1);

    ioStat = HookPageWrite(hNTM->pNC, ulPage, 1, NULL, pSpare);

    FFXERRORINJECT_WRITE("WritePhysSpareArea", ulPage, &ioStat);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTPAGEIO-WritePhysSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

#endif



#endif  /* FFXCONF_NANDSUPPORT */


