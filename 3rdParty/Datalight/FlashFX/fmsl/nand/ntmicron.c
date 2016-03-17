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

    This module contains a NAND Technology Module (NTM) which is designed to
    work with Micron chips which support dual plane operations.  Aside from
    the dual plane issues, in other respects this NTM is very similar to the
    CAD NTM (ntcad.c).  Maintenance done in one of these modules should be
    seriously considered for application to the other module.

    Note: This NTM only supports flash with "Offset-Zero" style factory bad
          block marking.

    This NTM supports MicronM60 Flash with internal EDC enabled.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntmicron.c $
    Revision 1.39  2012/02/09 21:47:14Z  Glenns
    Fix Bug 3384; remove undesireable reserved-area compensation
    from Create procedrue. Added clarifying comment on M59 flash
    family.
    Revision 1.38  2011/04/22 20:54:57Z  garyp
    Updated diagnostics messages -- no functional changes.
    Revision 1.37  2010/12/10 19:00:59Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.36  2010/09/23 01:22:31Z  garyp
    Added a CHIP_READY_WAIT() call in MicronM60ConfigureEcc().
    Revision 1.35  2010/08/13 17:37:57Z  glenns
    Implemented previous fix correctly.
    Revision 1.34  2010/08/13 17:03:19Z  glenns
    Placate noisy compiler- no functional changes.
    Revision 1.33  2010/07/29 16:27:51Z  garyp
    Fixed a logic flaw in the previous checkin.
    Revision 1.32  2010/07/27 22:04:19Z  garyp
    Updated to turn off any M60 EDC when the NTM instance is destroyed.
    Turn of any M60 EDC when going into power suspend mode.  Fixed
    the serial number code to work with 16-bit M50 flash which appears
    to not follow the specification.
    Revision 1.31  2010/07/16 15:24:28Z  garyp
    Updated so the hooks module provides the maximum number of chips
    count to the NTM.
    Revision 1.30  2010/07/08 03:35:17Z  garyp
    Added support for manual injection of errors.
    Revision 1.29  2010/07/06 02:24:57Z  garyp
    Major update to support multiple chips.  Corrected misplaced/missing
    parens from the previous two checkins.  Added support for chip serial
    numbers.  Eliminated duplicated logic for handling M60 error corrections.
    Updated SET_CHIP_SELECT() to operate in terms of pages.  Finalized
    block locking functionality.  Fixed HiddenRead() to not examine M60 ECC
    if it was not enabled, as well as to properly zero-out invalid tags.
    Revision 1.28  2010/06/25 19:17:46Z  glenns
    Add support for Micron M60 boot blocks and resulting split-EDC arrangement.
    Revision 1.27  2010/06/19 17:32:24Z  garyp
    Partial workaround for Bug 3213.  Removed some hard-coded Micron
    feature command magic numbers.
    Revision 1.26  2010/06/09 02:35:32Z  glenns
    Add support for Micron M60 devices.  Add support for block-lock interface.
    Revision 1.25  2009/12/20 00:22:47Z  garyp
    Updated to obtain the alignment requirements from the hooks module.
    Revision 1.24  2009/12/11 21:32:55Z  garyp
    Major update to support more configurable project settings.  The
    entire spare area format is now under control of the hooks interface.
    Work-in-progress to support Numonyx chips.
    Revision 1.23  2009/12/03 21:50:14Z  garyp
    Updated to use a renamed symbol.
    Revision 1.22  2009/10/06 22:20:01Z  garyp
    Updated to use the new page-oriented hooks functions.
    Revision 1.21  2009/09/23 23:08:27Z  garyp
    Added support for querying the chip ID.  Changed the default behavior from
    supporting dual-plane operations to support cache-mode operations.
    Revision 1.20  2009/07/27 17:31:43Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written.  Added a default IORequest()
    function.  Documentation updated.
    Revision 1.19  2009/04/16 19:53:05Z  billr
    Resolve bug 2627: microneval project driver segfaults during initialization.
    Revision 1.18  2009/04/09 03:42:55Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.17  2009/04/03 18:08:37Z  glenns
    - Fix Bugzilla #2402, bu being sure pNTM->uLinearChipAddrMSB
      is set before conducting the scan for multiple chips.
    Revision 1.16  2009/04/02 15:03:46Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.15  2009/03/25 23:32:23Z  billr
    Resolve bug 2545: Assertion failure seen on tx4938 during FSIOTest.
    Revision 1.14  2009/03/04 06:47:42Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    Revision 1.13  2009/01/27 20:59:39Z  thomd
    Fail create if chip's EDC requirement > 1
    Revision 1.12  2009/01/26 22:50:50Z  glenns
    - Modified to accommodate variable names changed to meet
      Datalight coding standards.
    Revision 1.11  2009/01/24 00:24:12Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.10  2009/01/23 17:13:17Z  thomd
    pass blocks instead of address to SET_CHIP_SELECT
    Revision 1.9  2009/01/16 23:58:03Z  glenns
    - Fixed up literal FFXIOSTATUS initialization in eleven places.
    Revision 1.8  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.7  2008/06/16 16:55:18Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.6  2008/05/08 01:03:06Z  garyp
    Added support for cache mode operations -- currently this is mutually
    exclusive with dual-plane operations.
    Revision 1.5  2008/05/03 21:34:43Z  garyp
    Updated to allow dual-plane support to be chosen at compile-time or
    run-time, as desired.  As such, the NTM now supports Micron chips which
    do not support dual-plane operations.  Modified the NTM so that it will not
    work with any parts but Micron parts.
    Revision 1.4  2008/03/23 20:24:18Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.3  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/09/21 04:35:48Z  pauli
    Resolved Bug 1455.  Added the ability to report that BBM should or should
    not be used based on the characteristics of the NAND part identified.
    Revision 1.1  2007/09/12 23:33:16Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <deverr.h>
#include <errmanager.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include "nandid.h"

#if FFXCONF_NANDSUPPORT_MICRON

#include <fxdevapi.h>
#include <fhmicron.h>
#include "nand.h"
#include "ntm.h"
#include <ntmicron.h>

#define NTMNAME "NTMICRON"

/*  The MAX_CHIPS declaration is used only to declare arrays which must be
    done at compile-time, and to validate that the fhmicron.c hooks are 
    not trying to use more chips than the NTM supports.  The actual limit
    on the chip count may be lower, and is provided by the hooks code.
*/  
#define MAX_CHIPS                 (2)   

#define SUPPORT_NUMONYX_CHIPS (FALSE)   /* FALSE for checkin (work-in-progress) */

/*  This NTM contains support for automatically detecting if an M60-style
    device is in use. With this macro turned on, the M60 on-die EDC engine
    will be used to perform EDC. Turn it off to use external EDC with an
    M60 device. Other devices will automatically use external EDC.
*/
#define USE_M60_ONDIE_EDC     (TRUE)    /* TRUE for checkin */

#define SERIALNUM_SIZE              (16)
#define SERIALNUM_COPIES            (16)

struct tagNTMDATA
{
    D_UINT32            ulPhysBlockSize;
    D_UINT16            uPhysPageSize;
    D_UINT16            uPhysSpareSize;
    D_UINT16            uLinearPageAddrMSB;     /* how much to >> linear address to get chip page index  */
/*  D_UINT16            uChipPageIndexLSB;         how much to << chip page index before sending to chip */
    D_UINT16            uLinearBlockAddrLSB;    /* how much to >> linear address to get block index */
    D_UINT16            uLinearChipAddrMSB;     /* how much to >> linear address to get chip select */
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE        hEI;
  #endif
    NTMINFO             NtmInfo;                /* information visible to upper layers */
    const FFXNANDCHIP  *pChipInfo;              /* chip information */
    FFXDEVHANDLE        hDev;
    D_UINT32            ulPagesPerChip;
    D_UINT32            ulFirstBootPage;        /* M60 only */
    D_UINT32            ulLastBootPage;         /* M60 only */
    NTMHOOKHANDLE       hHook;
    FFXMICRONPARAMS     Params;
    D_BYTE              abID[NAND_ID_SIZE];
    D_BYTE              abSN[MAX_CHIPS][SERIALNUM_SIZE];
    unsigned            nChips;                 /* Number of chips in this Device */
    unsigned            nChipStart;             /* Starting chip select number */
    unsigned            fUseCacheReads  : 1;
    unsigned            fUseCacheWrites : 1;
    unsigned            fUseDualPlanes  : 1;
    unsigned            fM60ECCAllowed  : 1;
    D_BOOL              afM60ECCEnabled[MAX_CHIPS]; /* Current state of M60 ECC per chip select */
};

#define TAG_ECCCHECKSIZE    (2)     /* two bytes for ECC and check byte */
#define TAG_IO_COUNT        ((hNTM->NtmInfo.uMetaSize+2) / sizeof(NAND_IO_TYPE))
#define DATA_IO_COUNT       (hNTM->uPhysPageSize / sizeof(NAND_IO_TYPE))
#define SPARE_IO_COUNT      (hNTM->uPhysSpareSize / sizeof(NAND_IO_TYPE))
#define DATA512_IO_COUNT    (NAND512_PAGE / sizeof(NAND_IO_TYPE))
#define SPARE16_IO_COUNT    (NAND512_SPARE / sizeof(NAND_IO_TYPE))
#define PLANE0              (0)
#define PLANE1              (1)

#define MICRONM60_FLAGS_OFFSET      (3)
#define MICRONM60_TAG_OFFSET        (4)
#define MICRONM60_TAG_LENGTH        (3)
#define MICRONM60_ECC_OFFSET        (8)     /* Within each 16-byte segment */
#define MICRONM60_WRITTEN_WITH_ECC  (0)

/*  This macro determines whether a given page in an array belongs to an 
    M60 boot block.
*/
#define IS_BOOT_PAGE(pNTM, pg) ((pg >= pNTM->ulFirstBootPage) && (pg < pNTM->ulLastBootPage))
#define CHIP_OF_PAGE(h, pg)    ((h)->nChipStart + ((pg) >> (h)->uLinearChipAddrMSB))

/*  The following macros define interfaces to the various hook functions
    implemented in fhmicron.c.
*/
#define SET_CHIP_SELECT(pg, mod)    (FfxHookNTMicronSetChipSelect(hNTM->hHook, CHIP_OF_PAGE(hNTM, (pg)), (mod)))
#define CHIP_READY_WAIT()           (FfxHookNTMicronReadyWait(hNTM->hHook))
#define SET_CHIP_COMMAND(x)         (FfxHookNTMicronSetChipCommand(hNTM->hHook, (x)))
#define SET_CHIP_COMMAND_NOWAIT(x)  (FfxHookNTMicronSetChipCommandNoWait(hNTM->hHook, (x)))
#define NAND_DATA_IN(x,y)           (FfxHookNTMicronDataIn(hNTM->hHook, (x), (y)))
#define NAND_STATUS_IN()            (FfxHookNTMicronStatusIn(hNTM->hHook))
#define NAND_DATA_OUT(x,y)          (FfxHookNTMicronDataOut(hNTM->hHook, (x), (y)))
#define NAND_ADDR_OUT(addr)         (FfxHookNTMicronAddressOut(hNTM->hHook, (addr)))

/*  Internal helper routines used primarily by the new FIM interface routines
*/
static FFXSTATUS    ReadID(                   NTMHANDLE hNTM, D_BYTE *pabID, unsigned nChip);
static FFXSTATUS    ReadSerialNumber(         NTMHANDLE hNTM, D_UCHAR *pucID, unsigned nChip);
static unsigned     IdentifyFlash(            NTMHANDLE hNTM, FFXFIMBOUNDS *pBounds);
static void         SetLogicalPageAddress(    NTMHANDLE hNTM, D_UINT32 ulPageNum, D_UINT32 ulPageOffset, unsigned nPlane);
static void         SetBlockAddress(          NTMHANDLE hNTM, D_UINT32 ulOffset);
static FFXSTATUS    SetReadChipStatus(        NTMHANDLE hNTM, D_UINT32 ulAbsolutePage);
static FFXSTATUS    GetStatus(                NTMHANDLE hNTM, D_UINT32 ulAbsolutePage, NAND_IO_TYPE *pStatus);
static FFXSTATUS    GetError(                 NTMHANDLE hNTM, D_UINT32 ulAbsolutePage);
static FFXSTATUS    ExecuteChipCommand(       NTMHANDLE hNTM, D_UCHAR ucCommand, D_UINT32 ulAbsolutePage);
static FFXIOSTATUS  ReadSpareArea(            NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(           NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXSTATUS    RandomReadSetup(          NTMHANDLE hNTM, D_UINT32 ulChipPage, unsigned nOffset, unsigned nPlane);
static FFXIOSTATUS  PageReadTransfer(         NTMHANDLE hNTM, D_BUFFER *pPageBuff, D_BUFFER *pTag, D_BOOL fUseEcc, D_UINT32 ulPage);
static FFXSTATUS    PageWriteTransfer(        NTMHANDLE hNTM, const D_BUFFER *pBuffer, const D_BUFFER *pTag, D_BOOL fUseEcc, D_UINT32 ulPage);
static FFXIOSTATUS  IsBadBlock(               NTMHANDLE hNTM, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
static FFXSTATUS    BlockLockFreeze(          NTMHANDLE hNTM);
static FFXIOSTATUS  BlockLock(                NTMHANDLE hNTM, D_UINT32 ulBlock, D_UINT32 ulCount);
static FFXIOSTATUS  BlockUnlock(              NTMHANDLE hNTM, D_UINT32 ulBlock, D_UINT32 ulCount, D_BOOL fInvert);
static D_UINT8      BlockLockStatus(          NTMHANDLE hNTM, D_UINT32 ulBlock);

/*  Internal helper routines used for Micron M60 Flash
*/
static FFXSTATUS    MicronM60ConfigureEcc(    NTMHANDLE hNTM, D_UINT32 ulPage, D_BOOL fEnable);
static D_BOOL       MicronM60GetECCStatus(    NTMHANDLE hNTM, FFXIOSTATUS *pIOStat);
static D_BOOL       MicronM60NtmTagDecode(    D_BUFFER *pDest, const D_BUFFER *pSrc);
static D_BOOL       MicronM60NtmTagEncode(    D_BUFFER *pDest, const D_BUFFER *pSrc);



/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_micron =
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
    D_UINT32            ulTotalBlocks;
    D_UINT32            ulBlockSizeKB;
    D_BOOL              fSuccess = FALSE;
    FFXFIMBOUNDS        bounds;
    unsigned            nChip;
 
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    pNTM->Params.nStructLen = sizeof(pNTM->Params);
    pNTM->hHook = FfxHookNTMicronCreate(hDev, &pNTM->Params);
    if(!pNTM->hHook)
        goto CreateCleanup;

  #if D_DEBUG
    DclPrintf("NTMICRON Project Configuration Parameters:\n");
    DclPrintf("  NAND Bus Width                 %8u Bits\n", NAND_DATA_WIDTH_BYTES * 8);
    DclPrintf("  Max Chips                      %8u\n", pNTM->Params.nMaxChips);
    DclPrintf("  Tag Offset                     %8u\n", pNTM->Params.nTagOffset);
    DclPrintf("  Alignment                      %8u Byte\n", pNTM->Params.nAlignment);
    DclPrintf("  M60 EDC Start Chip             %8u\n", pNTM->Params.nM60EDC4Chip);
    DclPrintf("  M60 EDC Start Block            %8u\n", pNTM->Params.ulM60EDC4Block);
    DclPrintf("  Allow Cache Mode Reads         %8s\n", pNTM->Params.fSupportCacheModeReads ? "Yes" : "No");
    DclPrintf("  Allow Cache Mode Writes        %8s\n", pNTM->Params.fSupportCacheModeWrites ? "Yes" : "No");
    DclPrintf("  Allow Dual Plane Operations    %8s\n", pNTM->Params.fSupportDualPlanes ? "Yes" : "No");
  #endif

    /*  Get the array bounds now so we know how far to scan
    */
    FfxDevGetArrayBounds(hDev, &bounds);

    pNTM->nChips = IdentifyFlash(pNTM, &bounds);
    if(!pNTM->nChips)
        goto CreateCleanup;

    /*  Compute ulTotalBlocks. Remember that the device manager now
        accounts for reserved area, so we want to limit ulTotalBlocks
        only by the maximum array size.

        This call to FfxDevApplyArrayBounds is only to error-check
        the bounds structure; it's result is not used to adjust the
        number of blocks reported by the NTM.
    */
    if (D_UINT32_MAX == FfxDevApplyArrayBounds(pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->nChips,
                                               pNTM->pChipInfo->pChipClass->ulBlockSize, &bounds))
        goto CreateCleanup;

    ulBlockSizeKB = pNTM->pChipInfo->pChipClass->ulBlockSize / 1024;
    ulTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->nChips;
    if(bounds.ulMaxArraySizeKB &&
        bounds.ulMaxArraySizeKB != FFX_REMAINING &&
        bounds.ulMaxArraySizeKB != FFX_CHIP_REMAINING &&
        (ulTotalBlocks > bounds.ulMaxArraySizeKB / ulBlockSizeKB))
    {
        ulTotalBlocks = (D_UINT32)(bounds.ulMaxArraySizeKB / ulBlockSizeKB);
    }

    for(nChip = pNTM->nChipStart; 
        nChip < pNTM->nChipStart + pNTM->nChips; 
        nChip ++)
    {
        FFXSTATUS   ffxStat;
        
        ffxStat = FfxHookNTMicronConfigure(pNTM->hHook, nChip, pNTM->pChipInfo, &pNTM->abID[0]);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, (NTMNAME"-FfxHookNTMicronConfigure() failed with status %lX\n", ffxStat));
            goto CreateCleanup;
        }            
    }            

    /*  This NTM cannot use both cache mode operations (read or write) at
        the same time as using dual-plane operations.  Since cache-mode
        reads seem to be supported for most all chips, and since that is
        generally more beneficial, if either cache mode reads or writes
        are set to TRUE, don't allow dual plane support.

        This NTM also cannot use cache mode operations if the device is
        M60-family and we are using the on-die EDC.
    */                
    if(USE_M60_ONDIE_EDC && ((pNTM->abID[4] & MICRONID4_ONDIE_EDC_MASK) == MICRONID4_ONDIE_EDC_4BIT))
    {
        pNTM->fM60ECCAllowed = TRUE;
        
        /* defaults to FALSE
        pNTM->fUseCacheReads = FALSE;
        pNTM->fUseCacheWrites = FALSE;
        */
    }
    else
    {
        /* defaults to FALSE
        pNTM->fM60ECCAllowed = FALSE;
        */
        
        pNTM->fUseCacheReads = pNTM->Params.fSupportCacheModeReads;

        if(pNTM->Params.fSupportCacheModeWrites && pNTM->abID[2] & MICRONID2_CACHE_PROGRAM)
            pNTM->fUseCacheWrites = TRUE;
    }
    
    if(!pNTM->fM60ECCAllowed &&
       !pNTM->fUseCacheReads && 
       !pNTM->fUseCacheWrites && 
        pNTM->Params.fSupportDualPlanes &&
        pNTM->pChipInfo->pChipClass->fMicronDualPlaneOps &&
       (pNTM->pChipInfo->pChipClass->uPageSize * 2 <= FFX_NAND_MAXPAGESIZE))
    {
        /*  Use dual-plane operations only if:
            A) We are not using M60 on-die EDC
            B) neither cache mode reads nor writes are enabled,
            C) it is requested by the hook params
            D) it is supported by the flash part
            E) the max NAND page size is large enough
        */
        pNTM->fUseDualPlanes = TRUE;
    }

    DclAssert(pNTM->pChipInfo->pChipClass->Samsung2kOps);
    DclAssert(NSOFFSETZERO_TAG_LENGTH == LEGACY_ENCODED_TAG_SIZE);

    pNTM->uLinearPageAddrMSB    = pNTM->pChipInfo->pChipClass->uLinearPageAddrMSB;
/*  pNTM->uChipPageIndexLSB     = pNTM->pChipInfo->pChipClass->uChipPageIndexLSB; */
    pNTM->uLinearBlockAddrLSB   = pNTM->pChipInfo->pChipClass->uLinearBlockAddrLSB;
    pNTM->uLinearChipAddrMSB    = pNTM->pChipInfo->pChipClass->uLinearChipAddrMSB;

    FFXPRINTF(1, ("NTMICRON Actual Configuration Parameters:\n"));
    FFXPRINTF(1, ("  M60 on-die EDC             %8s\n", pNTM->fM60ECCAllowed  ? "Enabled" : "Disabled"));
    FFXPRINTF(1, ("  Cache read operations      %8s\n", pNTM->fUseCacheReads  ? "Enabled" : "Disabled"));
    FFXPRINTF(1, ("  Cache write operations     %8s\n", pNTM->fUseCacheWrites ? "Enabled" : "Disabled"));
    FFXPRINTF(1, ("  Dual-plane operations      %8s\n", pNTM->fUseDualPlanes  ? "Enabled" : "Disabled"));
    FFXPRINTF(2, ("  LinearPageAddrMSB          %8U\n", pNTM->uLinearPageAddrMSB));
    FFXPRINTF(2, ("  LinearBlockAddrLSB         %8U\n", pNTM->uLinearBlockAddrLSB));
    FFXPRINTF(2, ("  LinearChipAddrMSB          %8U\n", pNTM->uLinearChipAddrMSB));

    /*  Move these into the main NTM structure for easier access
    */
    pNTM->ulPhysBlockSize             = pNTM->pChipInfo->pChipClass->ulBlockSize;
    pNTM->uPhysPageSize               = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->uPhysSpareSize              = pNTM->pChipInfo->pChipClass->uSpareSize;

    /*  Set the block/page/spare sizes we will present to higher layers
    */
    pNTM->NtmInfo.ulBlockSize         = pNTM->ulPhysBlockSize;
    pNTM->NtmInfo.uPageSize           = pNTM->uPhysPageSize;
    pNTM->NtmInfo.uSpareSize          = pNTM->uPhysSpareSize;
    pNTM->NtmInfo.ulChipBlocks        = pNTM->pChipInfo->pChipClass->ulChipBlocks;
    pNTM->NtmInfo.uAlignSize          = (D_UINT16)pNTM->Params.nAlignment;

    pNTM->NtmInfo.uEdcRequirement     = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    pNTM->NtmInfo.uEdcCapability      = pNTM->pChipInfo->pChipClass->uEdcCapability;
    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

    /*  If the chip has no built-in capability, use the default 
        software correction algorithms.
    */        
    if(!pNTM->NtmInfo.uEdcCapability)
    {
        pNTM->NtmInfo.uEdcCapability = 1;
        pNTM->NtmInfo.uEdcSegmentSize = DATA_BYTES_PER_ECC;
    }
    else if(pNTM->fM60ECCAllowed)
    {
        /*  M60 EDC covers 512-byte segments
        */
        pNTM->NtmInfo.uEdcSegmentSize = 512;
    }
    else
    {
        /*  We have an EDC capability reported, but it's not the M60 EDC.
            the default segment size. Note: Currently there are no known
            known Micron chips that do this, but this might occur if there
            is a separate NAND controller involved.
        */
        if(!pNTM->NtmInfo.uEdcSegmentSize) 
            pNTM->NtmInfo.uEdcSegmentSize = DATA_BYTES_PER_ECC;
    }
    
    if(pNTM->NtmInfo.uEdcRequirement > pNTM->NtmInfo.uEdcCapability)
    {
        /*  This chip requires more correction capabilities than this 
            NTM can handle and that is provided by the device -- fail 
            the create.
        */
        DclPrintf("FFX: NTMICRON: Insufficient EDC capabilities, Corrects %U per %U bytes, Requires %U\n",
            pNTM->NtmInfo.uEdcCapability, pNTM->NtmInfo.uEdcSegmentSize, pNTM->NtmInfo.uEdcRequirement);
        
        goto CreateCleanup;
    }

    if(pNTM->fUseDualPlanes)
    {
        pNTM->uLinearPageAddrMSB++;
        pNTM->NtmInfo.ulBlockSize  *= 2;
        pNTM->NtmInfo.uPageSize    *= 2;
        pNTM->NtmInfo.uSpareSize   *= 2;
        pNTM->NtmInfo.ulChipBlocks /= 2;
    }

    if(pNTM->NtmInfo.uPageSize > FFX_NAND_MAXPAGESIZE)
    {
        DclPrintf("FFX: NTMICRON: FFX_NAND_MAXPAGESIZE is %u, but the flash has a page size of %U\n",
            FFX_NAND_MAXPAGESIZE, pNTM->NtmInfo.uPageSize);

        goto CreateCleanup;
    }

    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    DclAssert((pNTM->NtmInfo.ulBlockSize % 1024UL) == 0);

    pNTM->NtmInfo.ulTotalBlocks     = ulTotalBlocks;
    pNTM->NtmInfo.uPagesPerBlock    = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);
    pNTM->NtmInfo.uMetaSize         = FFX_NAND_TAGSIZE;
    pNTM->NtmInfo.uDeviceFlags     |= DEV_REQUIRES_ECC;

    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

    pNTM->ulPagesPerChip            = pNTM->NtmInfo.ulChipBlocks * pNTM->NtmInfo.uPagesPerBlock;

    if(pNTM->fM60ECCAllowed && (pNTM->nChipStart <= pNTM->Params.nM60EDC4Chip))
    {
        pNTM->ulFirstBootPage       = pNTM->ulPagesPerChip * pNTM->Params.nM60EDC4Chip;
        pNTM->ulLastBootPage        = pNTM->ulFirstBootPage + pNTM->NtmInfo.uPagesPerBlock * pNTM->Params.ulM60EDC4Block;
    }

    for(nChip = pNTM->nChipStart; 
        nChip < pNTM->nChipStart + pNTM->nChips; 
        nChip ++)
    {
        if(pNTM->fM60ECCAllowed)
        {
            FFXSTATUS   ffxStat;

            /*  Initialize the state of the M60 ECC engine.  This will also 
                get the afM60ECCEnabled[] flags into thier initial state.
            */
            ffxStat = MicronM60ConfigureEcc(pNTM, nChip * pNTM->ulPagesPerChip, TRUE);
            if(ffxStat != FFXSTAT_SUCCESS)
            {
                FFXPRINTF(1, (NTMNAME"-MicronM60ConfigureEcc() Chip=%u failed with status %lX\n", nChip, ffxStat));
                goto CreateCleanup;
            }            
        }

        /*  Get the Chip serial number
        */
        ReadSerialNumber(pNTM, &pNTM->abSN[nChip - pNTM->nChipStart][0], nChip);
    }            
    
  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

    if(pNTM->pChipInfo->pChipClass->fBlockLockSupport)
    {
        D_UINT8 bStatus;

        /*  Just because the given chip supports locking, that
            does not mean the platform has enabled that feature.
            Read the lock status to determine if the feature is
            enabled.
        */
        pNTM->NtmInfo.uDeviceFlags |= DEV_LOCKABLE;
        
        bStatus = BlockLockStatus(pNTM, 0);
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

    FFXPRINTF(1, (NTMNAME": TotalBlocks=%lX ChipBlocks=%lX BlockSize=%lX PageSize=%X SpareSize=%X\n",
        ulTotalBlocks,
        pNTM->NtmInfo.ulChipBlocks,
        pNTM->NtmInfo.ulBlockSize,
        pNTM->NtmInfo.uPageSize,
        pNTM->NtmInfo.uSpareSize));

    /*  Make sure the parent has a pointer to our NTMINFO structure
    */
    *ppNtmInfo = &pNTM->NtmInfo;

    fSuccess = TRUE;

  CreateCleanup:

    if(pNTM && !fSuccess)
    {
        if(pNTM->hHook)
            FfxHookNTMicronDestroy(pNTM->hHook);

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

    /*  Ensure that for CS0, any M60 4-bit EDC engine on the NAND
        is disabled, so that if after the driver is unloaded, we 
        do a warm restart, any BOOTROM on the host which expects
        the 4-bit EDC to be disabled will operate correctly.

        Essentially the same thing we must deal with for power
        suspend/resume.
    */                
    if(hNTM->fM60ECCAllowed)
        MicronM60ConfigureEcc(hNTM, 0, FALSE);

    FFXERRORINJECT_DESTROY();

    FfxHookNTMicronDestroy(hNTM->hHook);

    DclMemFree(hNTM);

    return;
}


/*-------------------------------------------------------------------
    Public:PageRead()

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
    FFXIOSTATUS         ioTotal = DEFAULT_GOOD_PAGEIO_STATUS;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulChipPage;
    D_BOOL              fInCacheMode = FALSE;

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

    /*  Turn on or off hardware ECC corresponding to the value of fUseEcc
        and whether the requested page is in a boot block, which don't use
        the hardware EDC. Note that this has to be done after reserved
        pages calculation because we are interested in the physical page
        counted from the beginning of the device. Also note that read requests
        straddling erase blocks are illegal, so we don't need to worry about
        requests straddling the EDC boundary if boot blocks are in play.
    */
    if(hNTM->fM60ECCAllowed)
    {
        if(IS_BOOT_PAGE(hNTM, ulPage))
            MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
        else
            MicronM60ConfigureEcc(hNTM, ulPage, fUseEcc);
    }

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.  Unlike writes,
        we can do this one time and be done with it.
    */
    ioTotal.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    /*  Calculate ulChipPage so that it is relative to the chip we
        just selected.
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    ioTotal.ffxStat = CHIP_READY_WAIT();
    if(ioTotal.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    while(ulCount)
    {
        DCLALIGNEDBUFFER    (tag1, data, FFX_NAND_MAXTAGSIZE);
        DCLALIGNEDBUFFER    (tag2, data, FFX_NAND_MAXTAGSIZE);

        if(!fInCacheMode)
        {
            /*  PLANE 0
                Send the programming command and address for the data page
            */
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

            if(hNTM->fUseDualPlanes)
            {
                /*  PLANE 1
                    Send the programming command and address for the data page
                */
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN);
                if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);
            }

            if(hNTM->pChipInfo->pChipClass->ReadConfirm)
            {
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
                if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;
            }

            /*  Read a page of data, assume success
            */
            ioStat.ffxStat = CHIP_READY_WAIT();
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        if(hNTM->fUseCacheReads && ulCount > 1)
        {
            fInCacheMode = TRUE;

            /*  Send command indicating that the next sequential page
                is what we will read next.
            */
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_READ_CACHE_MODE);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = CHIP_READY_WAIT();
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }
        else if((ulCount == 1) && fInCacheMode)
        {
            /*  Send command indicating that the next sequential page
                is will be the last one read.
            */
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_READ_CACHE_MODE_LAST);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = CHIP_READY_WAIT();
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        /*  PLANE 0
        */
        ioStat = PageReadTransfer(hNTM, pPages, pTags ? tag1.data : 0, fUseEcc, ulPage);
        ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

        pPages += hNTM->uPhysPageSize;


        if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
            ioTotal.ffxStat = ioStat.ffxStat;
        else if (ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if(hNTM->fUseDualPlanes)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_READ_PAGE_DUALPLANE_RANDOM);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);

            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            /*  PLANE 1
            */
            ioStat = PageReadTransfer(hNTM, pPages, pTags ? tag2.data : 0, fUseEcc, ulPage);
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            if (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
                ioTotal.ffxStat = ioStat.ffxStat;
            else if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            pPages += hNTM->uPhysPageSize;
        }

        if(nTagSize)
        {
            DclAssert(fUseEcc);
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            if(hNTM->fUseDualPlanes)
            {
                /*  For dual plane operations, the tag values must exactly match
                    from both halves of the virtual page.  If they do not match,
                    perhaps due to a write interruption, the data in the entire
                    page must be considered invalid.
                */
                if(DclMemCmp(&tag1.data, &tag2.data, nTagSize) != 0)
                {
                    /*  If there was an error in the tag which could not be
                        handled, return a null tag value.
                    */
                    DclMemSet(pTags, 0, nTagSize);
                }
                else
                {
                    DclMemCpy(pTags, &tag1.data, nTagSize);
                }
            }
            else
            {
                DclMemCpy(pTags, &tag1.data, nTagSize);
            }

            pTags += nTagSize;
        }

        ioTotal.ulCount++;
        ulCount--;
        ulChipPage++;
        ulPage++;

        /*  The read either succeeded with no error or a correctable
            error was detected.  In the latter case, the page was just
            counted as successful, but the read ends.
        */
        if (ioStat.ffxStat != FFXSTAT_SUCCESS
            || ioTotal.ffxStat != FFXSTAT_SUCCESS)
        {
            break;
        }
    }

    if (ioStat.ffxStat != FFXSTAT_SUCCESS)
        ioTotal.ffxStat = ioStat.ffxStat;

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-PageRead() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
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
    D_UINT32            ulChipPage;

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

    /*  Turn on or off hardware ECC corresponding to the value of fUseEcc
        and whether the requested page is in a boot block, which don't use
        the hardware EDC. Note that this has to be done after reserved
        pages calculation because we are interested in the physical page
        counted from the beginning of the device. Also note that read requests
        straddling erase blocks are illegal, so we don't need to worry about
        requests straddling the EDC boundary if boot blocks are in play.
    */
    if(hNTM->fM60ECCAllowed)
    {
        if(IS_BOOT_PAGE(hNTM, ulPage))
            MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
        else
            MicronM60ConfigureEcc(hNTM, ulPage, fUseEcc);
    }

    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    while(ulCount)
    {
        if(hNTM->pChipInfo->pChipClass->ResetBeforeProgram)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = CHIP_READY_WAIT();

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

        /*  PLANE 0
        */
        ioStat.ffxStat = PageWriteTransfer(hNTM, pPages, pTags, fUseEcc, ulPage);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        pPages += hNTM->uPhysPageSize;

        if(hNTM->fUseDualPlanes)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_PROGRAM_NEXTPLANE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);

            /*  PLANE 1
            */
            ioStat.ffxStat = PageWriteTransfer(hNTM, pPages, pTags, fUseEcc, ulPage);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            pPages += hNTM->uPhysPageSize;
        }

        if(hNTM->fUseCacheWrites && (ulCount > 1))
        {
            /*  Now actually program the data...
            */
            ioStat.ffxStat = ExecuteChipCommand(hNTM, MICRON_CMD_PROGRAM_CACHE_MODE, ulPage);
        }
        else
        {
            /*  Now actually program the data...
            */
            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulPage);
        }

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("PageWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if(nTagSize)
        {
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            pTags += nTagSize;
        }

        ulPage++;
        ulChipPage++;
        ulCount--;
    }

/*  PageWriteCleanup:  */
 
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageRead() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

	/*  Turn off M60 hardware ECC
    */
    if(hNTM->fM60ECCAllowed)
        MicronM60ConfigureEcc(hNTM, ulPage, FALSE);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        D_UINT32            ulChipPage;

        /*  Calculate ulChipPage so that it is relative to the chip we
            just selected.
        */
        ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

        while(ulCount)
        {
            /*  PLANE 0
                Send the programming command and address for the data page
            */
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

            if(hNTM->fUseDualPlanes)
            {
                /*  PLANE 1
                    Send the programming command and address for the data page
                */
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);
            }

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

            /*  Read Plane 0 main page data
            */
            NAND_DATA_IN((NAND_IO_TYPE *)pPages, (D_UINT16)DATA_IO_COUNT);

            if(pSpares)
            {
                /*  Read Plane 0 spare area
                */
                NAND_DATA_IN((NAND_IO_TYPE *)pSpares, (D_UINT16)SPARE_IO_COUNT);

                pSpares += hNTM->uPhysSpareSize;
            }

            pPages += hNTM->uPhysPageSize;

            if(hNTM->fUseDualPlanes)
            {
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_READ_PAGE_DUALPLANE_RANDOM);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);

                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_PAGE_RANDOM_START);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                /*  Read Plane 1 main page data
                */
                NAND_DATA_IN((NAND_IO_TYPE *)pPages, (D_UINT16)DATA_IO_COUNT);

                if(pSpares)
                {
                    /*  Read Plane 1 spare area
                    */
                    NAND_DATA_IN((NAND_IO_TYPE *)pSpares, (D_UINT16)SPARE_IO_COUNT);

                    pSpares += hNTM->uPhysSpareSize;
                }

                pPages += hNTM->uPhysPageSize;
            }

            ioStat.ulCount++;
            ulCount--;
            ulChipPage++;
        }
    }

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
    D_UINT32            ulChipPage;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageWrite() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

	/*  Turn off M60 hardware ECC
    */
    if(hNTM->fM60ECCAllowed)
        MicronM60ConfigureEcc(hNTM, ulPage, FALSE);

    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    if(!pSpares)
        DclMemSet(spare.data, ERASED8, hNTM->uPhysSpareSize);

    while(ulCount)
    {
        /*  If we are using M60 on-board ECC and doing a raw write, reset
            the chip before the raw page program operation. This is because
            the M60 does not automatically clear error conditions if the 
            on-board ECC is turned off.
        */
        if(hNTM->pChipInfo->pChipClass->ResetBeforeProgram || hNTM->fM60ECCAllowed)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ffxStat = CHIP_READY_WAIT();

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

        if(pSpares)
        {
            DclMemCpy(spare.data, pSpares, hNTM->uPhysSpareSize);
            pSpares += hNTM->uPhysSpareSize;
        }

        /*  Transfer the data for PLANE 0
        */
        NAND_DATA_OUT((NAND_IO_TYPE *)pPages, (D_UINT16)DATA_IO_COUNT);

        /*  Write the actual metadata to the flash part's internal buffer
        */
        NAND_DATA_OUT((NAND_IO_TYPE *)&spare.data, (D_UINT16)SPARE_IO_COUNT);

        pPages += hNTM->uPhysPageSize;

        if(hNTM->fUseDualPlanes)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_PROGRAM_NEXTPLANE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE1);

            if(pSpares)
            {
                DclMemCpy(spare.data, pSpares, hNTM->uPhysSpareSize);
                pSpares += hNTM->uPhysSpareSize;
            }

            /*  Transfer the data for PLANE 1
            */
            NAND_DATA_OUT((NAND_IO_TYPE *)pPages, (D_UINT16)DATA_IO_COUNT);

            /*  Write the actual metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT((NAND_IO_TYPE *)&spare.data, (D_UINT16)SPARE_IO_COUNT);

            pPages += hNTM->uPhysPageSize;
        }

        /*  Now actually program the data...
        */
        ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulPage);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("RawPageWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulPage++;
        ulChipPage++;
        ulCount--;
    }

/*  RawPageWriteCleanup:  */

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

	/*  Turn off M60 hardware ECC
    */ 
    if(hNTM->fM60ECCAllowed)
        MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
    
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

	/*  Turn off M60 hardware ECC
    */
    if(hNTM->fM60ECCAllowed)
        MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
    
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
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_UINT32            ulCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    D_UINT32            ulChipPage;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_BOOL              fM60ECCEnabled = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenRead() StartPage=%lX Count=%lX TagSize=%u\n", ulPage, ulCount, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Turn on or off hardware ECC corresponding to the value of fUseEcc
        and whether the requested page is in a boot block, which don't use
        the hardware EDC.  Note that this has to be done after reserved
        pages calculation because we are interested in the physical page
        counted from the beginning of the device.  Also note that read 
        requests straddling erase blocks are illegal, so we don't need to
        worry about requests straddling the EDC boundary if boot blocks
        are in play.
    */
    if(hNTM->fM60ECCAllowed)
    {
        if(IS_BOOT_PAGE(hNTM, ulPage))
        {
            MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
        }
        else
        {
            MicronM60ConfigureEcc(hNTM, ulPage, TRUE);
            fM60ECCEnabled = TRUE;
        }
    }
    
    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        while(ulCount)
        {
            DCLALIGNEDBUFFER    (rawtag1, data, FFX_NAND_MAXTAGSIZE+2);
            unsigned            nOffset;

            if(fM60ECCEnabled)
                nOffset = MICRONM60_TAG_OFFSET;
            else
                nOffset = hNTM->Params.nTagOffset;
                
            ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize + nOffset, PLANE0);
			if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            NAND_DATA_IN((NAND_IO_TYPE *)rawtag1.data, (D_UINT16)TAG_IO_COUNT);

            if(hNTM->fUseDualPlanes)
            {
                DCLALIGNEDBUFFER    (rawtag2, data, FFX_NAND_MAXTAGSIZE+2);
                DCLALIGNEDBUFFER    (tag1, data, FFX_NAND_MAXTAGSIZE);
                DCLALIGNEDBUFFER    (tag2, data, FFX_NAND_MAXTAGSIZE);

                DclAssert(!hNTM->fM60ECCAllowed);

                ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize + hNTM->Params.nTagOffset, PLANE1);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                NAND_DATA_IN((NAND_IO_TYPE *)rawtag2.data, (D_UINT16)TAG_IO_COUNT);

                /*  Validating that we atomically wrote both parts of a
                    page during dual plane operations is ensuring that the
                    tag exactly matches in both halves.  If it does not, then
                    return a NULL tag value.

                    Note that when using dual planes, we will never be using
                    the M60 on-die ECC, so there is no need to consult the
                    M60 controller for the existence of bit errors.
                */
                if(FfxNtmHelpTagDecode(tag1.data, rawtag1.data) &&
                   FfxNtmHelpTagDecode(tag2.data, rawtag2.data) &&
                   DclMemCmp(tag1.data, tag2.data, nTagSize) == 0)
                {
                    DclMemCpy(pTags, tag1.data, nTagSize);
                }
                else
                {
                    /*  If there was an error in the tag which could not be handled,
                        return a null tag value.
                    */
                    DclMemSet(pTags, 0, nTagSize);
                }
            }
            else
            {
                if(fM60ECCEnabled)
                {
                    if(!MicronM60GetECCStatus(hNTM, &ioStat))
                    {
                        pTags[0] = 0;
                        pTags[1] = 0;

                        FFXPRINTF(1, (NTMNAME"-HiddenRead() invalid tag for page %lX, status %s\n", 
                            ulPage, FfxDecodeIOStatus(&ioStat)));

                        /*  Failure is indicated by the tag bytes being NUL
                            not by a status code.
                        */                            
                        ioStat.ffxStat = FFXSTAT_SUCCESS;
                    }
                    else
                    {
                        MicronM60NtmTagDecode(pTags, rawtag1.data);
                    }
                }
                else
                {
                    FfxNtmHelpTagDecode(pTags, rawtag1.data);
                }
            }

            ulCount--;
            ioStat.ulCount++;
            ulChipPage++;
            ulPage++;
            pTags += nTagSize;
        }
    }

    /*  HiddenReadCleanup: 
    */

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
    D_UINT32            ulChipPage;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (spare, data, NAND4K_SPARE/2);
    D_UINT32            ulCount = 1;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclProfilerEnter(NTMNAME"-HiddenWrite", 0, 0);
    
    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Turn on or off hardware ECC corresponding to the value of fUseEcc
        and whether the requested page is in a boot block, which don't use
        the hardware EDC. Note that this has to be done after reserved
        pages calculation because we are interested in the physical page
        counted from the beginning of the device. Also note that read requests
        straddling erase blocks are illegal, so we don't need to worry about
        requests straddling the EDC boundary if boot blocks are in play.
    */
    if(hNTM->fM60ECCAllowed)
    {
        if(IS_BOOT_PAGE(hNTM, ulPage))
            MicronM60ConfigureEcc(hNTM, ulPage, FALSE);
        else
            MicronM60ConfigureEcc(hNTM, ulPage, TRUE);
    }

    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    DclMemSet(spare.data, ERASED8, hNTM->uPhysSpareSize);

    while(ulCount)
    {
        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if(hNTM->fM60ECCAllowed && !IS_BOOT_PAGE(hNTM, ulPage))
            MicronM60NtmTagEncode(&spare.data[MICRONM60_TAG_OFFSET], pTag);
		else
            FfxNtmHelpTagEncode(&spare.data[hNTM->Params.nTagOffset], pTag);

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE0);

        /*  Write the actual metadata to the flash part's internal buffer
        */
        NAND_DATA_OUT((NAND_IO_TYPE *)&spare.data, (D_UINT16)SPARE_IO_COUNT);

        if(hNTM->fUseDualPlanes)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_PROGRAM_NEXTPLANE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE1);

            /*  Write the actual metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT((NAND_IO_TYPE *)&spare.data, (D_UINT16)SPARE_IO_COUNT);
        }

        /*  Now actually program the data...
        */
        ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulPage);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("HiddenWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulPage++;
        ulChipPage++;
        ulCount--;
        pTag += nTagSize;
    }

/*  HiddenWriteCleanup:  */

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
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_BLOCKIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-BlockErase() Block=%lX\n", ulBlock));

    DclProfilerEnter(NTMNAME"-BlockErase", 0, 0);
    
    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulBlock * hNTM->NtmInfo.uPagesPerBlock, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        D_UINT32    ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;
        D_UINT32    ulChipOffset = ulOffset & ((1 << hNTM->uLinearChipAddrMSB) - 1);

        /*  Tell the flash to erase those blocks now
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_ERASE_BLOCK);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            SetBlockAddress(hNTM, ulChipOffset);

            if(hNTM->fUseDualPlanes)
            {
                ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_ERASE_BLOCK);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    SetBlockAddress(hNTM, ulChipOffset + hNTM->ulPhysBlockSize);
                }
                else
                {
                    goto BlockEraseCleanup;
                }
            }

            ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_ERASE_BLOCK_START, ulBlock * hNTM->NtmInfo.uPagesPerBlock);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                ioStat.ulCount = 1;

                FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);
            }
        }
    }

  BlockEraseCleanup:

    DclProfilerLeave(0UL);

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
        ulPage - The flash offset in pages, relative to any
                 reserved space.

    Return Value:
        Returns an FFXIOSTATUS structure with standard status
        information.  If the status is FFXSTAT_SUCCESS, the
        op.ulPageStatus variable will contain the page status
        information, as defined in fxiosys.h.
-------------------------------------------------------------------*/
static FFXIOSTATUS GetPageStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);

    DclAssert(hNTM);

    if(!hNTM->fM60ECCAllowed || IS_BOOT_PAGE(hNTM, ulPage))
    {
        ioStat = ReadSpareArea(hNTM, ulPage, (void *)buffer.ns);  

        /*  If hardware ECC is not in use, we can use the standard method
            for examining the spare area to get the page status:
        */
        ioStat.op.ulPageStatus = FfxHookNTMicronGetPageStatus(hNTM->hHook, buffer.ns);
    }
    else
    {
        MicronM60ConfigureEcc(hNTM, ulPage, TRUE); 

        ioStat = ReadSpareArea(hNTM, ulPage, (void *)buffer.ns);
        if((ioStat.ulCount == 1) &&
            ((ioStat.ffxStat == FFXSTAT_SUCCESS) || 
             (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)))
           
        {
            /*	If the tag area, including ECC (which covers page data as
                well) is within 1 bit of being erased, then we know that a
                tag was not written.  If it is anything else, we know a tag
                was written.  MicronM60 with hardware ECC always uses legacy
                tag size.

                REFACTOR: Use of "within one bit" macros is probably not 
                advisable for 4 bit EDC devices, as they can have multiple
                bit errors within the spare area.  This should be addressed.
            */
            if(!FfxNtmHelpIsRangeErased1Bit(&buffer.ns[MICRONM60_TAG_OFFSET], MICRONM60_TAG_LENGTH)) 
                ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);

            if(DclBitCountArray(&buffer.ns[MICRONM60_FLAGS_OFFSET], 8) <= 4)
                ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
            else
                ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
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

    DclAssert(hNTM);

    ioStat = IsBadBlock(hNTM, hNTM->pChipInfo, ulBlock);

    if(hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE)
    {
        D_UINT8     bStatus;

        bStatus = BlockLockStatus(hNTM, ulBlock);

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

    DclAssert(hNTM);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        /*  If M60 ECC is available, use it for this operation.

            Should be applying the "BOOTPAGES" here right?  This only
            seems to be used by test code anyway...
        
        */
        if(hNTM->fM60ECCAllowed)
            MicronM60ConfigureEcc(hNTM, ulBlock * hNTM->NtmInfo.uPagesPerBlock, TRUE);

        ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
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

        case FFXPARAM_FIM_CHIPSN:
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */

                ffxStat = DCLSTAT_SETUINT20(sizeof(hNTM->abSN));
            }
            else
            {
                if(ulBuffLen)
                {
                    /*  Fill only as much of the buffer as is supplied by
                        the caller.  If the buffer is larger than needed,
                        the remaining bytes will be untouched.
                    */
                    ulBuffLen = DCLMIN(ulBuffLen, sizeof(hNTM->abSN));
                    DclMemCpy(pBuffer, &hNTM->abSN[0], ulBuffLen);
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
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        NTMNAME"-ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
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
        FFXPRINTF(1, ("NTSIM:ParameterSet() unhandled parameter ID=%x\n", id));

        ffxStat = FFXSTAT_BADPARAMETER;
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

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_FREEZE() hNTM=%P\n", hNTM));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            (void)pReq;

            ioStat.ffxStat = BlockLockFreeze(hNTM);

            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ulCount = 1;

            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }

        case FXIOFUNC_FIM_LOCK_BLOCKS:
        {
            FFXIOR_FIM_LOCK_BLOCKS *pReq = (FFXIOR_FIM_LOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_LOCK_BLOCKS() hNTM=%P StartBlock=%lU Count=%lU\n",
                hNTM, pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat = BlockLock(hNTM, pReq->ulStartBlock, pReq->ulBlockCount);

            ioStat.ulFlags = IOFLAGS_DEVICE;

            break;
        }

        case FXIOFUNC_FIM_UNLOCK_BLOCKS:
        {
            FFXIOR_FIM_UNLOCK_BLOCKS *pReq = (FFXIOR_FIM_UNLOCK_BLOCKS*)pIOR;

            FFXPRINTF(1, (NTMNAME"-FXIOFUNC_FIM_UNLOCK_BLOCKS() hNTM=%P StartBlock=%lU Count=%lU\n",
                hNTM, pReq->ulStartBlock, pReq->ulBlockCount));

            DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

            ioStat = BlockUnlock(hNTM, pReq->ulStartBlock, pReq->ulBlockCount, pReq->fInvert);

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

            /*  Ensure that for CS0, any M60 4-bit EDC engine on the NAND
                is disabled, so that when we come out of suspend, in the 
                that any BOOTROM on the host expects the 4-bit EDC to be
                disabled.
            */                
            if(hNTM->fM60ECCAllowed)
                MicronM60ConfigureEcc(hNTM, 0, FALSE);

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
    Local: PageReadTransfer()

    This is a helper function that performs the transfer of the
    data from the NAND controller's internal buffer to the client
    buffer.

    For a dual plane NTM, this function transfers one physical
    page, or one half of the virtual page.

    Parameters:
        hNTM      - The NTM handle to use
        pPageBuff - A buffer to receive the main page data
        pTag      - A buffer to receive the tag data.  May be NULL.
        fUseEcc   - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.
-------------------------------------------------------------------*/
static FFXIOSTATUS PageReadTransfer(
    NTMHANDLE           hNTM,
    D_BUFFER           *pPageBuff,
    D_BUFFER           *pTag,
    D_BOOL              fUseEcc,
    D_UINT32            ulPage)
{
    D_BUFFER            abECCs[MAX_ECC_BYTES_PER_PAGE];
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);
    D_BOOL              fUseSWEcc;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-PageReadTransfer() reading page data (%U %u-byte elements)\n",
        DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

    DclProfilerEnter(NTMNAME"-PageReadTransfer", 0, 0);

    DclAssert(hNTM);
    DclAssert(pPageBuff);

    fUseSWEcc = ((!hNTM->fM60ECCAllowed || IS_BOOT_PAGE(hNTM, ulPage)) && fUseEcc);
    FfxHookNTMicronPageRead(hNTM->hHook, (NAND_IO_TYPE*)pPageBuff, DATA_IO_COUNT, fUseSWEcc ? abECCs : NULL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-PageReadTransfer() reading spare data (%U %u-byte elements)\n",
        SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

    /*  Read the spare area
    */
    NAND_DATA_IN((NAND_IO_TYPE *)spare.data, (D_UINT16)SPARE_IO_COUNT);

    if(fUseEcc)
    {
        if (hNTM->fM60ECCAllowed && !IS_BOOT_PAGE(hNTM, ulPage))
            MicronM60GetECCStatus(hNTM, &ioStat);
        else
            ioStat = FfxHookEccCorrectPage((NTMHOOKHANDLE)hNTM->hHook, pPageBuff, spare.data, abECCs, hNTM->pChipInfo);
	}

    if((ioStat.ffxStat == FFXSTAT_SUCCESS) || (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA))
    {
        if(pTag)
		{
            if(hNTM->fM60ECCAllowed && !IS_BOOT_PAGE(hNTM, ulPage))
                MicronM60NtmTagDecode(pTag, &spare.data[MICRONM60_TAG_OFFSET]);
            else
                FfxNtmHelpTagDecode(pTag, &spare.data[hNTM->Params.nTagOffset]);
		}

        ioStat.ulCount = 1;
    }

    DclProfilerLeave(0UL);

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: PageWriteTransfer()

    This is a helper function that performs the transfer of the
    data from the client buffer to the NAND controller's internal
    buffer, processing ECCs and tags as directed.

    For a dual plane NTM, this function transfers one physical
    page, or one half of the virtual page.

    If fUseEcc is TRUE, an ECC for the main page data is generated
    and written in the spare area.  If fUseEcc is FALSE, no ECC is
    written.  The spare area includes a record of the state of 
    fUseEcc.

    Parameters:
        hNTM       - The NTM handle to use
        pPageBuff  - A pointer to the main page data to write.
        pTags      - A pointer to tag data to write.  May be NULL
                     to skip writing the tag.  May be set only if
                     fUseEcc is TRUE.
        fUseEcc    - If TRUE, perform error detection and correction.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of pages which were completely written
        successfully.  The status indicates whether the read was
        entirely successful.
-------------------------------------------------------------------*/
static FFXSTATUS PageWriteTransfer(
    NTMHANDLE           hNTM,
    const D_BUFFER     *pPageBuff,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc,
    D_UINT32            ulPage)
{
    DCLALIGNEDBUFFER    (spare, data, FFX_NAND_MAXSPARESIZE);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-PageWriteTransfer() writing page (%U %u-byte elements)\n",
        DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

    DclAssert(hNTM);
    DclAssert(pPageBuff);
    DclAssert(sizeof(spare.data) >= hNTM->uPhysSpareSize);

    DclProfilerEnter(NTMNAME"-PageWriteTransfer", 0, 0);

    DclMemSet(spare.data, ERASED8, hNTM->uPhysSpareSize);

    if(!fUseEcc)
    {
        DclAssert(!pTag);
        FfxHookNTMicronPageWrite(hNTM->hHook, (const NAND_IO_TYPE*)pPageBuff, DATA_IO_COUNT, NULL);
    }
    else
    {
        if(!hNTM->fM60ECCAllowed || IS_BOOT_PAGE(hNTM, ulPage))
        {
            if(pTag)
                FfxNtmHelpTagEncode(&spare.data[hNTM->Params.nTagOffset], pTag);

            /*  Why are we not getting ffxStat result from this?
            */
            FfxHookNTMicronPageWrite(hNTM->hHook, (const NAND_IO_TYPE*)pPageBuff, DATA_IO_COUNT, spare.data);
        }
        else
        {
            if(pTag)
                MicronM60NtmTagEncode(&spare.data[MICRONM60_TAG_OFFSET], pTag);

            spare.data[MICRONM60_FLAGS_OFFSET] = MICRONM60_WRITTEN_WITH_ECC;
                
            /*  Why are we not getting ffxStat result from this?
            */
            FfxHookNTMicronPageWrite(hNTM->hHook, (const NAND_IO_TYPE*)pPageBuff, DATA_IO_COUNT, NULL);
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
        NTMNAME"-PageWriteTransfer() writing spare data (%U %u-byte elements)\n",
        SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

    /*  Write the actual metadata to the flash part's internal buffer
    */
    NAND_DATA_OUT((NAND_IO_TYPE *)&spare.data, (D_UINT16)SPARE_IO_COUNT);

    DclProfilerLeave(0UL);

    /*  Can't fail -- for the time being...
    */
    return FFXSTAT_SUCCESS;
}


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
    D_UINT32        ulCount = 1;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-ReadSpareArea() Page=%lX\n", ulPage));

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Calculate ulChipPage so that it is relative to the chip
        */
        D_UINT32 ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

        while(ulCount)
        {
            ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE0);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            NAND_DATA_IN((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);

            pSpare += hNTM->uPhysSpareSize;

            if(hNTM->fUseDualPlanes)
            {
                ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE1);
                if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                    break;

                NAND_DATA_IN((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);

                pSpare += hNTM->uPhysSpareSize;
            }

            ioStat.ulCount++;
            ulChipPage++;
            ulCount--;
        }
    }

    /*  SpareReadCleanup: 
    */

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
    D_UINT32            ulChipPage;
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulCount = 1;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-WriteSpareArea() Page=%lX\n", ulPage));

    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);

    while(ulCount)
    {
        /*  Set the chip selects and mask off the chip select bits.

            This must be done inside the loop because the chip select is
            reset to read-mode inside the ExecuteChipCommand() function
            (proven to be necessary for the TX4938).
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE0);

        /*  Write the actual metadata to the flash part's internal buffer
        */
        NAND_DATA_OUT((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);

        pSpare += hNTM->uPhysSpareSize;

        if(hNTM->fUseDualPlanes)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(MICRON_CMD_PROGRAM_NEXTPLANE);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            SetLogicalPageAddress(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE1);

            /*  Write the actual metadata to the flash part's internal buffer
            */
            NAND_DATA_OUT((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);

            pSpare += hNTM->uPhysSpareSize;
        }

        /*  Now actually program the data...
        */
        ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulPage);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("SpareWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulPage++;
        ulChipPage++;
        ulCount--;
    }

    /*  SpareWriteCleanup:
    */

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
        hNTM       - The NTM handle to use
        ulChipPage - The page within the currently selected chip
        nOffset    - The offset to use
        nPlane     - The plane to use -- PLANE0 or PLANE1

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS RandomReadSetup(
    NTMHANDLE       hNTM,
    D_UINT32        ulChipPage,
    unsigned        nOffset,
    unsigned        nPlane)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-RandomReadSetup() ChipPage=%lX Offset=%x Plane=%u\n", ulChipPage, nOffset, nPlane));

    DclAssert(hNTM);
    DclAssert(nPlane == PLANE0 || nPlane == PLANE1);
    DclAssert(nPlane == PLANE0 || hNTM->fUseDualPlanes);

    /*  Send the programming command and address for the data page
    */
    ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        SetLogicalPageAddress(hNTM, ulChipPage, nOffset, nPlane);

        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            ffxStat = CHIP_READY_WAIT();
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-RandomReadSetup() returning status %lX\n", ffxStat));

    return ffxStat;
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
    FFXSTATUS           ffxStat;
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-BlockLockFreeze() hNTM=%P\n", hNTM));

    DclAssert(hNTM);

    if( (hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE) &&
        (hNTM->NtmInfo.nLockFlags & FFXLOCKFLAGS_LOCKFREEZE) )
    {
        ffxStat = SET_CHIP_COMMAND(MICRON_CMD_LOCKTIGHT);
    }
    else
    {
        ffxStat = FFXSTAT_FIM_LOCKFAILED;
    }

    return ffxStat;
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_ALWAYS, 1, 0),
        NTMNAME"-BlockLock() hNTM=%P Block=%lX Count=%lU\n", hNTM, ulBlock, ulCount));

    DclProfilerEnter(NTMNAME"-BlockLock", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    if( (hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE) &&
        (hNTM->NtmInfo.nLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS) )
    {
        /*  Set the chip selects and mask off the chip select bits: only want
            addresses relative to the chip for the rest of this.
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulBlock * hNTM->NtmInfo.uPagesPerBlock, MODE_WRITE);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(MICRON_CMD_LOCK);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                ioStat.ulCount = ulCount;
        }
    }
    else
    {
        ioStat.ffxStat = FFXSTAT_FIM_LOCKFAILED;
    }
    
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
    D_BOOL              fInvert)
{
    FFXIOSTATUS         ioStat = INITIAL_BLOCKIO_STATUS(FFXSTAT_SUCCESS);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_ALWAYS, 1, 0),
        NTMNAME"-BlockUnlock() hNTM=%P Block=%lX Count=%lU Invert=%u\n",
        hNTM, ulBlock, ulCount, fInvert));

    DclProfilerEnter(NTMNAME"-BlockUnlock", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);

    if(hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE)
    {
        D_UINT32    ulChipOffset, ulEndChipOffset;
        D_UINT32    ulOffset;
        
        ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;

        /*  Split the byte offset into a chip number (0 or 1) and page number
            in the chip.  Eventually all interfaces will be in pages, so do
            everything else here in terms of the page number.
        */
        ulChipOffset = ulOffset & ((1 << hNTM->uLinearChipAddrMSB) - 1);
        ulEndChipOffset = (ulOffset + ((ulCount-1) * hNTM->NtmInfo.ulBlockSize)) & ((1 << hNTM->uLinearChipAddrMSB) - 1);

        /*  Set the chip selects and mask off the chip select bits: only want
            addresses relative to the chip for the rest of this.
        */
        ioStat.ffxStat = SET_CHIP_SELECT(ulBlock * hNTM->NtmInfo.uPagesPerBlock, MODE_WRITE);
        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND(MICRON_CMD_UNLOCK1);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                SetBlockAddress(hNTM, ulChipOffset);

                ioStat.ffxStat = SET_CHIP_COMMAND(MICRON_CMD_UNLOCK2);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    SetBlockAddress(hNTM, ulEndChipOffset | (fInvert << hNTM->uLinearBlockAddrLSB));
                    ioStat.ulCount = ulCount;
                }
            }
        }
    }
    else
    {
        ioStat.ffxStat = FFXSTAT_FIM_UNLOCKFAILED;
    }
    
    DclProfilerLeave(0UL);
    return ioStat;
}


/*-------------------------------------------------------------------
    Local: BlockLockStatus()

    NOTE! Unlike the other block locking functions, the caller of
          this function has already added in any reserved space.

    Parameters:
        hNTM       - The NTM handle
        ulBlock    - The flash offset in blocks, relative to any
                     reserved space.

    Return Value:
-------------------------------------------------------------------*/
static D_UINT8 BlockLockStatus(
    NTMHANDLE           hNTM,
    D_UINT32            ulBlock)
{
    D_UINT8             bResult = 0;

    DclProfilerEnter(NTMNAME"-BlockLockStatus", 0, 0);

    DclAssert(hNTM);

    if(hNTM->NtmInfo.uDeviceFlags & DEV_LOCKABLE)
    {
        if(SET_CHIP_SELECT(ulBlock * hNTM->NtmInfo.uPagesPerBlock, MODE_READ) == FFXSTAT_SUCCESS)
        {
            if(SET_CHIP_COMMAND(MICRON_CMD_READ_LOCK_STATUS) == FFXSTAT_SUCCESS)
            {
                NAND_IO_TYPE    id;
                D_UINT32        ulChipOffset;
                D_UINT32        ulOffset;
        
                ulOffset = ulBlock * hNTM->NtmInfo.ulBlockSize;

                ulChipOffset = ulOffset & ((1 << hNTM->uLinearChipAddrMSB) - 1);

                SetBlockAddress(hNTM, ulChipOffset); 

                NAND_DATA_IN(&id, 1);

                bResult = (D_UINT8)BENDSWAP(id);
            }
        }
    }
    
    DclProfilerLeave(0UL);
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-BlockLockStatus() Block=%lX Status=0x%02x\n", ulBlock, bResult));

    return bResult;
}


/*-------------------------------------------------------------------
    Local: IsBadBlock()

    Parameters:
        hNTM      - The NTM handle
        pChipInfo - A pointer to the FFXNANDCHIP structure
                    describing the chip being used.
        ulBlock   - The block to mark.  Must have been
                    adjusted for reserved space (if any).

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        op.ulBlockStatus field will contain the block status
        bits pertaining to bad blocks.
-------------------------------------------------------------------*/
static FFXIOSTATUS IsBadBlock(
    NTMHANDLE                   hNTM,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock)
{
    D_UINT32                    ulPagesRemaining;
    FFXIOSTATUS                 ioStat;
    D_UINT32                    ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "IsBadBlock() Block=%lX\n", ulBlock));

    DclAssert(hNTM);
    DclAssert(pChipInfo);
    DclAssert(pChipInfo->pChipClass->uSpareSize <= NAND4K_SPARE);

    /*  If there is a factory bad block mark, it will be in one of the
        first two pages.
    */
    ulPagesRemaining = 2;

    ulPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    while(ulPagesRemaining)
    {
        /*  Note that this buffer <must> be large enough to hold a
            4KB-page spare area (128-bytes) regardless what our max
            NAND page size might be.
        */
        DCLALIGNEDBUFFER    (spare, data, NAND4K_SPARE);

        ioStat = ReadSpareArea(hNTM, ulPage, (void *)spare.data);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            /*  If we failed to read the spare area, zero out the
                ulBlockStatus field, as it could have PageStatus
                information in there (since we just did a page
                oriented operation).
            */
            ioStat.op.ulBlockStatus = 0;
            goto IsBadBlockCleanup;
        }

        if(spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET] != ERASED8
            || (hNTM->fUseDualPlanes && (spare.data[(hNTM->uPhysSpareSize)+NSOFFSETZERO_FACTORYBAD_OFFSET] != ERASED8)))
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsBadBlockCleanup;
        }
        else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
        {
            /*  For 16-bit interfaces, one of the first two bytes
                will be marked.
            */
            if(spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+1] != ERASED8
                || (hNTM->fUseDualPlanes && (spare.data[(hNTM->uPhysSpareSize)+NSOFFSETZERO_FACTORYBAD_OFFSET+1] != ERASED8)))
            {
                ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                goto IsBadBlockCleanup;
            }
        }

        ulPage++;
        ulPagesRemaining--;
    }

    ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

  IsBadBlockCleanup:

    /*  We were doing page operations, so change the type back to block...
    */
    ioStat.ulFlags = IOFLAGS_BLOCK;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "IsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Read the ID information from the NAND chips, and configure them
    properly considering the OEM's Device configuration with regard
    to reserved space.

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

    This code does not support the scenario where there is a chip
    on CS1, but no chip at all on CS0.

    Parameters:
        hNTM           - The NTM handle.
        pBounds        - A pointer to the FFXFIMBOUNDS structure.

    Return Value:
        The number of chips (0, 1, or 2) detected.
-------------------------------------------------------------------*/
static unsigned IdentifyFlash(
    NTMHANDLE       hNTM,
    FFXFIMBOUNDS   *pBounds)
{
    unsigned        nChips = 0;
    D_UINT32        ulChip0KB;

    DclAssert(hNTM);
    DclAssert(pBounds);
    DclAssert(hNTM->Params.nMaxChips > 0);
    DclAssert(hNTM->Params.nMaxChips <= MAX_CHIPS);

    /*  Identify the first chip in the array.
    */
    if(ReadID(hNTM, hNTM->abID, 0) != FFXSTAT_SUCCESS)
        return 0;

    hNTM->pChipInfo = FfxNandDecodeID(hNTM->abID);
    if(!hNTM->pChipInfo)
        return 0;

    nChips++;

    /*  Determine the CS0 chip size in KB
    */
    ulChip0KB = hNTM->pChipInfo->pChipClass->ulChipBlocks * (hNTM->pChipInfo->pChipClass->ulBlockSize / 1024);

    if( (hNTM->Params.nMaxChips > 1) &&  
        (FFX_ISDEVCHIP(pBounds->ulReservedBottomKB) || pBounds->ulReservedBottomKB >= ulChip0KB) )
    {
        /*  We're here if the low reserved space matches or exceeds
            the size of the chip on CS0.  To move forward with the
            initialization, there must be a chip on CS1.
        */

        /*  Identify the next chip in the array.
        */
        if(ReadID(hNTM, hNTM->abID, 1) != FFXSTAT_SUCCESS)
            return 0;

        hNTM->pChipInfo = FfxNandDecodeID(hNTM->abID);
        if(!hNTM->pChipInfo)
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

        /*  Chip select calculations will be relative to CS1 rather than CS0
        */
        hNTM->nChipStart = 1;
    }
    else
    {
        D_BYTE  abID[NAND_ID_SIZE];
        
        /*  The low reserved space does not fully encompass CS0, so if we
            find a chip on CS1, it <must> be the exact same chip as that
            on CS0.  However, if the max array size is defined such that
            it only uses the chip on CS0, ignore CS1 entirely.
        */

        DclAssert(!FFX_ISDEVCHIP(pBounds->ulReservedBottomKB));

        if( (hNTM->Params.nMaxChips > 1) &&
            (pBounds->ulMaxArraySizeKB != FFX_CHIP_REMAINING) &&
            ((pBounds->ulMaxArraySizeKB == FFX_REMAINING) ||
            (pBounds->ulReservedBottomKB + pBounds->ulMaxArraySizeKB > ulChip0KB)) &&
            (ReadID(hNTM, abID, 1) == FFXSTAT_SUCCESS) )
        {
            const FFXNANDCHIP  *pCS1ChipInfo;

            /*  Decode the ID for the sole purpose of making sure it
                exactly matches that of the first chip.
            */
            pCS1ChipInfo = FfxNandDecodeID(abID);
            if(pCS1ChipInfo)
            {
                if(pCS1ChipInfo != hNTM->pChipInfo)
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

    /*  After all that, it had better be the right kind of flash...
    */

  #if SUPPORT_NUMONYX_CHIPS
    if((hNTM->abID[0] != NAND_MFG_MICRON) && (hNTM->abID[0] != NAND_MFG_NUMONYX))
    {
        DclPrintf("FFX: NTMICRON: The NTM only supports Micron and Numonyx chips\n");

        return 0;
    }
  #else
    if(hNTM->abID[0] != NAND_MFG_MICRON)
    {
        DclPrintf("FFX: NTMICRON: The NTM only supports Micron chips\n");

        return 0;
    }
  #endif

    if((hNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_OFFSETZERO)
    {
        FFXPRINTF(1, ("NTMICRON: This chip is not an OffsetZero style part.\n"));
        return 0;
    }
    
    return nChips;
}


/*-------------------------------------------------------------------
    Local: ReadID()

    Read the ID from a NAND chip.

    Parameters:
        hNTM     - The NTM handle
        pabID    - A pointer to a NAND_ID_SIZE-byte buffer to receive
                   the ID
        nChip    - The chip select to use, 0 or 1.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ReadID(
    NTMHANDLE       hNTM,
    D_BYTE         *pabID,
    unsigned        nChip)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-ReadID() nChip=%u\n", nChip));

    ffxStat = FfxHookNTMicronSetChipSelect(hNTM->hHook, nChip, MODE_READ);
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

                        pabID[ii] = (D_UCHAR) BENDSWAP(Temp);
                    }

                  #if NAND_ID_SIZE != 6
                    #error "NAND_ID_SIZE has changed, fix this code"
                  #endif

                    FFXPRINTF(1, (NTMNAME"-ReadID() NAND CS%u ID %02x %02x %02x %02x %02x %02x\n",
                                  nChip, pabID[0], pabID[1], pabID[2], pabID[3], pabID[4], pabID[5]));
                }
            }
        }
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: ReadSerialNumber()

    Read the serial number from a NAND chip.  The buffer pabSN will
    only be modified if this function is successful.

    Parameters:
        hNTM     - The NTM handle.
        pabSN    - A pointer to a SERIALNUM_SIZE size byte buffer to
                   receive the serial number.
        nChip    - The chip to query.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
static FFXSTATUS ReadSerialNumber(
    NTMHANDLE       hNTM,
    D_BUFFER       *pabSN,
    unsigned        nChip)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-ReadSerialNumber() hNTM=%P Chip=%u\n", hNTM, nChip));

    ffxStat = FfxHookNTMicronSetChipSelect(hNTM->hHook, nChip, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            ffxStat = CHIP_READY_WAIT();
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                ffxStat = SET_CHIP_COMMAND(CMD_READ_SN);
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    NAND_ADDR_OUT(0);

                    ffxStat = CHIP_READY_WAIT();
                    if(ffxStat == FFXSTAT_SUCCESS)
                    {
                        unsigned    cc;    

                        for(cc = 0; cc < SERIALNUM_COPIES; cc++)
                        {
                            unsigned        bb;
                            D_BUFFER        bBase[SERIALNUM_SIZE];
                            D_BOOL          fSuccess = TRUE;
                            NAND_IO_TYPE    Temp;
                            
                          #if NAND_DATA_WIDTH_BYTES == 2
                            if(!hNTM->fM60ECCAllowed)
                            {
                                /*  There appears to be a flaw in the serial
                                    number implementation for 16-bit flash
                                    prior to the M60 stuff.  

                                    Contrary to the M50 documentation, the 
                                    serial number is read 16-bits at a time.

                                    IMPORTANT: there are some revisions of
                                    M59 flash that have been shown *NOT* to
                                    display this abberant behavior. If such
                                    a chip is present, this entire "if" block
                                    should be compiled out.
                                    
                                    Read the base copy of the chip serial number
                                */
                                for(bb = 0; bb < SERIALNUM_SIZE; bb += 2)
                                {
                                    NAND_DATA_IN(&Temp, 1);

                                    Temp = BENDSWAP(Temp);
                                    
                                    bBase[bb] = (D_BYTE)Temp;
                                    bBase[bb+1] = (D_BYTE)(Temp >> 8);

                                    FFXPRINTF(1, (NTMNAME"-ReadSerialNumber() index %2u BENDSWAPPED=0x%04x Bytes 0x%02x 0x%02x\n", 
                                        bb, Temp, bBase[bb], bBase[bb+1])); 
                                }

                                /*  Read the complement and validate
                                */
                                for(bb = 0; bb < SERIALNUM_SIZE; bb += 2)
                                {
                                    D_BYTE  bComp1;
                                    D_BYTE  bComp2;

                                    NAND_DATA_IN(&Temp, 1);

                                    Temp = BENDSWAP(Temp);
                                    
                                    bComp1 = (D_BYTE)Temp;
                                    bComp2 = (D_BYTE)(Temp >> 8);

                                    FFXPRINTF(1, (NTMNAME"-ReadSerialNumber() index %2u, Complement=0x%04x Bytes 0x%02x 0x%02x\n", 
                                        bb, Temp, bComp1, bComp2)); 

                                    if( ((bBase[bb] ^ bComp1) != 0xFF) ||
                                        ((bBase[bb+1] ^ bComp2) != 0xFF) )
                                    {
                                        fSuccess = FALSE;

                                        /*  Keep reading this group so we clock all
                                            the data in, and we can then try the 
                                            next group.
                                        */    
                                    }
                                }
                            }
                            else
                          #endif    /* NAND_DATA_WIDTH_BYTES == 2 */
                            {
                                /*  This code appears to work properly for 8-bit
                                    M50 flash, as well as 16-bit M60 flash.

                                    Read the base copy of the chip serial number
                            */
                            for(bb = 0; bb < SERIALNUM_SIZE; bb++)
                            {
                                NAND_DATA_IN(&Temp, 1);

                                bBase[bb] = (D_BYTE)BENDSWAP(Temp);

                                    FFXPRINTF(1, (NTMNAME"-ReadSerialNumber() index %2u Original=0x%04x BENDSWAPPED=0x%02x\n", 
                                        bb, Temp, bBase[bb])); 
                            }

                            /*  Read the complement and validate
                            */
                            for(bb = 0; bb < SERIALNUM_SIZE; bb++)
                            {
                                D_BYTE  bComp;

                                NAND_DATA_IN(&Temp, 1);
                                bComp = (D_BYTE)BENDSWAP(Temp);

                                    FFXPRINTF(1, (NTMNAME"-ReadSerialNumber() index %2u, Complement=0x%04x BENDSWAPPED=0x%02x\n", 
                                        bb, Temp, bComp)); 

                                if((bBase[bb] ^ bComp) != 0xFF)
                                {
                                    fSuccess = FALSE;

                                    /*  Keep reading this group so we clock all
                                        the data in, and we can then try the 
                                        next group.
                                    */    
                                    }
                                }
                            }

                            if(fSuccess)
                            {
                                DclMemCpy(pabSN, bBase, SERIALNUM_SIZE);
                                
                                FFXPRINTF(1, (NTMNAME"-NAND CS%u Serial Number: %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                     nChip, pabSN[0], pabSN[1], pabSN[2], pabSN[3], pabSN[4], pabSN[5], pabSN[6], pabSN[7]));
                                FFXPRINTF(1, ("                                 %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                     pabSN[8], pabSN[9], pabSN[10], pabSN[11], pabSN[12], pabSN[13], pabSN[14], pabSN[15]));

                                return FFXSTAT_SUCCESS;
                            }
                            else
                            {
                                FFXPRINTF(1, (NTMNAME" Reading CS%u Serial Number attempt %u failed\n", nChip, cc+1));
                            }
                        }

                        ffxStat = FFXSTAT_FAILURE;
                    }
                }
            }
        }
    }

    FFXPRINTF(1, (NTMNAME"-ReadSerialNumber() CS%u Serial number could not be successfully read, status=%lX\n", 
        nChip, ffxStat));
                    
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
    D_UINT32        ulAbsolutePage)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-SetReadChipStatus() AbsPage=%lX\n", ulAbsolutePage));

    /*  select chip based on address MSB
    */
    ffxStat = SET_CHIP_SELECT(ulAbsolutePage, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_STATUS);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: GetStatus()

    Universal routine to detect an error condition in the status
    register of all NAND chips.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXSTATUS GetStatus(
    NTMHANDLE       hNTM,
    D_UINT32        ulAbsolutePage,
    NAND_IO_TYPE   *pStatus)
{
    FFXSTATUS       ffxStat;

    ffxStat = SetReadChipStatus(hNTM, ulAbsolutePage);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        NAND_IO_TYPE   Status;
        
        Status = NAND_STATUS_IN();
        *pStatus = BENDSWAP(Status);      /* unsafely swaps volatile memory ptrs */
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-GetStatus() AbsPage=%lX returning %lX, NandStatus=%u\n", ulAbsolutePage, ffxStat, *pStatus));

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
    D_UINT32        ulAbsolutePage)
{
    NAND_IO_TYPE    Status = 0;
    FFXSTATUS       ffxStat;

    ffxStat = GetStatus(hNTM, ulAbsolutePage, &Status);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  This function should only be called after the chip has
            reported it has finished an operation.  If this isn't
            so, the rest of the status isn't yet valid; just call it
            a timeout.
        */
        if(!(Status & NANDSTAT_READY))
        {
            ffxStat = FFXSTAT_FIMTIMEOUT;
        }
        else if(Status & NANDSTAT_ERROR)
        {
            ffxStat = FFXSTAT_FIMIOERROR;
        }
        else if(!(Status & NANDSTAT_WRITE_ENABLE))
        {
            ffxStat = FFXSTAT_FIM_WRITEPROTECTEDBLOCK;
            FFXPRINTF(3, ("WRITEPROTECTED! Page=%lX\n", ulAbsolutePage));
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-GetError() AbsPage=%lX returning %lX\n", ulAbsolutePage, ffxStat));

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
    D_UINT32        ulAbsolutePage)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-ExecuteChipCommand() Cmd=0x%02x AbsPage=%lX\n", ucCommand, ulAbsolutePage));

    ffxStat = SET_CHIP_COMMAND_NOWAIT(ucCommand);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = CHIP_READY_WAIT();
        if(ffxStat == FFXSTAT_SUCCESS)
            ffxStat = GetError(hNTM, ulAbsolutePage);
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-ExecuteChipCommand() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Local: SetBlockAddress()

    Universal routine to output a block address for a NAND chip.

    ulOffset is a linear byte address that only contains the offset
    into the chip.  The chip selects must have been set and masked
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
    D_UINT32        ulOffset)
{
    int             iBits;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
        NTMNAME"-SetBlockAddress() Offset=%lX\n", ulOffset));

    ulOffset >>= hNTM->uLinearBlockAddrLSB;
    iBits = hNTM->uLinearChipAddrMSB - hNTM->uLinearBlockAddrLSB;
    do
    {
        NAND_ADDR_OUT((D_UCHAR)ulOffset);

        ulOffset >>= 8;
        iBits -= 8;

    }
    while(iBits > 0);

    return;
}


/*-------------------------------------------------------------------
    Local: SetLogicalPageAddress()

    Universal routine to output a page address for a NAND chip.

    ulOffset is a linear byte address that only contains the offset
    into the chip.  The chip selects must have been set and masked
    off before this routine is called.  The offset must aligned with
    the size of a logical NAND page.

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void SetLogicalPageAddress(
    NTMHANDLE       hNTM,
    D_UINT32        ulPageNum,
    D_UINT32        ulPageOffset,
    unsigned        nPlane)
{
    D_UCHAR         ucAddress;
    D_UINT32        ulPageIndex;
    int             iBits;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-SetLogicalPageAddress() PageNum=%lX Offset=%lX Plane=%u\n",
        ulPageNum, ulPageOffset, nPlane));

    DclAssert(hNTM);
    DclAssert(nPlane == PLANE0 || nPlane == PLANE1);
    DclAssert(nPlane == PLANE0 || hNTM->fUseDualPlanes);

    /*  Get offset into the physical data page.  This offset is in terms
        of the width of the data port (byte, word, ...)
    */
    ulPageOffset >>= NAND_DATA_WIDTH_POW2;

    /*  Output the page offset one byte at a time, LSB first.
    */
    ucAddress = (D_UCHAR) ulPageOffset;
    NAND_ADDR_OUT(ucAddress);
    ulPageOffset >>= 8;                     /* align offset MSB */
    ucAddress = (D_UCHAR) ulPageOffset;     /* put out offset MSB */
    NAND_ADDR_OUT(ucAddress);

    if(hNTM->fUseDualPlanes)
    {
        /*  Convert the upper bits of the address into a page index and align
            it as the chip expects.  They may easily be more than 16 address
            bits in the page index so don't try and merge that with the page
            offset.

            ulPageNum nPlane : ulPageIndex
            --------- ------   -----------
                0        0          0
                0        1         64
                1        0          1
                1        1         65
                ...
                64       0        128
                64       1        192
                65       0        129
                65       1        193
        */
        ulPageIndex = ((ulPageNum >> 6) * (hNTM->NtmInfo.uPagesPerBlock * 2))
                        | (ulPageNum & (hNTM->NtmInfo.uPagesPerBlock - 1));

        if(nPlane == PLANE1)
            ulPageIndex += hNTM->NtmInfo.uPagesPerBlock;
    }
    else
    {
        ulPageIndex = ulPageNum;
    }

    /*  Figure out how many bits we have yet to send for this chip and output
        the page index one byte at a time, LSB first.
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

    return;
}

 
/*-------------------------------------------------------------------
    Local: MicronM60AConfigureEcc()

    Configure the Micron M60 chip to either perform hardware ECC
    generation/validation or not, depending on whether the NTM has
    been configured to use hardware EDC and whether the caller has
    asked for EDC to be performed.

    Parameters:
        hNTM     - Handle for NTM to use
        ulPage   - The absolute page number which is used to 
                   determine the chip select.
        fEnable  - TRUE to enable or FALSE to disable the ECC engine 

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
 -------------------------------------------------------------------*/
static FFXSTATUS MicronM60ConfigureEcc(
    NTMHANDLE       hNTM, 
    D_UINT32        ulPage,
    D_BOOL          fEnable)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;
    unsigned        nChip;
    D_BOOL         *pfEnabled;

    DclAssert(hNTM);
    DclAssert(hNTM->fM60ECCAllowed);

    nChip = CHIP_OF_PAGE(hNTM, ulPage);

    pfEnabled = &hNTM->afM60ECCEnabled[nChip];
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-MicronM60ConfigureEcc() Chip=%u fCurrent=%u fEnable=%u\n", 
        nChip, *pfEnabled, fEnable));

    if(*pfEnabled != fEnable)
    {
        ffxStat = FfxHookNTMicronSetChipSelect(hNTM->hHook, nChip, MODE_READ);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            /*  ECC is in the wrong state -- reverse it
            */
            ffxStat = SET_CHIP_COMMAND(MICRON_CMD_SET_FEATURE);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                D_BYTE          bFeatureAddr = MICRON_FEATURE_ADDRESS_ARRAY_OP;
                NAND_IO_TYPE    cmd;
                
                NAND_ADDR_OUT(bFeatureAddr);

                if(fEnable)
                    cmd = MICRON_FEATURE_ENABLE_ECC;
                else
                    cmd = MICRON_FEATURE_DISABLE_ECC;

                NAND_DATA_OUT(&cmd, 1);      /* P1 = 0 or 0x08  */
                cmd = 0x00;
                NAND_DATA_OUT(&cmd, 1);      /* P2 = 0     */
                NAND_DATA_OUT(&cmd, 1);      /* P3 = 0     */
                NAND_DATA_OUT(&cmd, 1);      /* P4 = 0     */

                CHIP_READY_WAIT();

                FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
                    NTMNAME"-MicronM60ConfigureEcc() Chip=%u switched to M60 ECC enabled=%u\n", nChip, fEnable));

                *pfEnabled = fEnable;
            }
        }
    }
    
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-MicronM60ConfigureEcc() returning status %lX\n", ffxStat));

    return ffxStat;
}
 
  
/*-------------------------------------------------------------------
    Local: MicronM60NtmTagDecode()

    Decode the allocation entry. The first two bytes are the standard
    media-independent value, and in keeping with the peculiarities of
    the MicronM60A spare area format.  Note that for properly corrected
    errors, the checkbyte *must* verify. 

    Parameters:
        pDest - The destination buffer to fill with the 2-byte tag data.
        pSrc  - The 4-byte source buffer to process.

    Return Value:
        Returns TRUE if the tag was properly decoded and placed in
        the buffer specified by pDest, otherwise FALSE.  If FALSE
        is returned, the buffer specified by pDest will be set to
        NULLs.
-------------------------------------------------------------------*/
static D_BOOL MicronM60NtmTagDecode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];
    D_UCHAR         ucZeroBits = ERASED8;
    D_UCHAR         ucIndex;

	FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-MicronM60NtmTagDecode() pDest=%P pDest=%P\n",
        pDest, pSrc));

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
        and it will subsequently get erased and re-used. Note that
        the MicronM60 NTM eschews the Hamming code, because the tag
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
        FFXPRINTF(1, ("MicronM60NtmTagDecode() Unrecoverable tag error, Src=%02X %02X %02X %02X Corrected=%02X %02X %02X %02X\n",
            pSrc[3], pSrc[2], pSrc[1], pSrc[0],TempBuff[3], TempBuff[2], TempBuff[1], TempBuff[0]));

        pDest[0] = 0;
        pDest[1] = 0;

        return FALSE;
    }

    pDest[0] = TempBuff[0];
    pDest[1] = TempBuff[1];

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
    NTMNAME"-MicronM60NtmTagDecode() returning\n"));

    return TRUE;
}
  
	
/*-------------------------------------------------------------------
    Local: MicronM60NtmTagEncode()

    Build the allocation entry.  The first two bytes are the standard
    media-independent value, and the third byte then holds the check
    byte.  Note that for properly corrected errors, the checkbyte 
    *must* verify. 

    If the device reports a correctable error but the checkbyte does
    not verify, it is a sure indication that there are multiple bit
    errors in the tag that the Micron M60A ECC mechanism has falsely
    corrected.  This is a positive indicator that the tag has been
    invalidated.

    Parameters:
        pDest - The 4-byte destination buffer to fill with the
                2-byte tag data and its checkbyte.
        pSrc  - The 2-byte allocation entry.

    Return Value:
        Returns TRUE if the tag was properly encoded and placed in
        the buffer specified by pDest, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL MicronM60NtmTagEncode(
    D_BUFFER          *pDest,
    const D_BUFFER    *pSrc)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-MicronM60NtmTagEncode() pDest=%P pSrc=%P\n",
        pDest, pSrc));

    DclAssertWritePtr(pDest, 0);
    DclAssertReadPtr(pSrc, 0);
 
    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = (D_UINT8)(pDest[0] ^ ~pDest[1]);
    pDest[3] = ERASED8;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-MicronM60NtmTagEncode() returning\n"));

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: MicronM60GetECCStatus()

    Get the ECC status after a read using the M60 on-die ECC.  The
    pIOStat->ffxStat and pIOStat->op.ulPageStatus fields will be
    updated accordingly.

    Parameters:
        hNTM    - The NTM handle.
        pIOStat - The FFXIOSTATUS structure to update.

    Return Value:
        Returns TRUE if the the read was successful (regardless
        whether data was corrected or not.  Returns FALSE if
        there was uncorrectable data or some other error occurred.
-------------------------------------------------------------------*/
static D_BOOL MicronM60GetECCStatus(
    NTMHANDLE       hNTM, 
    FFXIOSTATUS    *pIOStat)
{
    DclAssertWritePtr(pIOStat, sizeof(*pIOStat));
    
    /*  Using HW ECC, determine if this page was corrected
    */
    pIOStat->ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_STATUS);
    if(pIOStat->ffxStat == FFXSTAT_SUCCESS)
    {
        NAND_IO_TYPE    Status;
    
        Status = NAND_STATUS_IN();
        Status = BENDSWAP(Status);	  /* unsafely swaps volatile memory ptrs */

        if(!(Status & NANDSTAT_ERROR))
        { 
            if(Status & NANDSTAT_DATA_CORRECTED)
            {
                pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                pIOStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                FfxErrMgrEncodeCorrectedBits(HW_REQUESTS_SCRUB, pIOStat);
            }

            return TRUE;
        }			

        pIOStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
    }

    FFXPRINTF(1, (NTMNAME"-MicronM60GetECCStatus() read failed, status %s\n", FfxDecodeIOStatus(pIOStat)));

    return FALSE;
}


#endif  /* FFXCONF_NANDSUPPORT_MICRON */
#endif  /* FFXCONF_NANDSUPPORT */

