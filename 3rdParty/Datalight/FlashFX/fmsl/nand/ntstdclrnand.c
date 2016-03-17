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
    work with Micron's Standard ClearNand chips.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntstdclrnand.c $
    Revision 1.9  2012/04/24 22:06:55Z  glenns
    Fix "used before its value is set" compiler warning.
    Revision 1.8  2012/02/09 21:56:56Z  Glenns
    Remove incorrect and unnecessary use of AdressableUnit
    calculations.
    Revision 1.7  2011/12/12 23:14:12Z  jimmb
    Rmoved call to the deleted API call 
    Revision 1.6  2011/12/12 20:53:30Z  glenns
    Remove compensation for pre-M60 serial number retrieval from
    16-bit devcies, as ClearNAND is not pre-M60.
    Revision 1.5  2011/12/12 20:13:58Z  jimmb
    Panda project and Standard ClearNAND update from GL
    Revision 1.4  2011/12/09 22:54:02Z  glenns
    Add compensation for testing engineering sample devices.
    Revision 1.3  2011/12/09 00:18:27Z  glenns
    Fix bugs regarding settings for erase cycle rating and BBM
    reserved block setting; clean up some comment formatting.
    Revision 1.2  2011/12/01 19:46:02Z  billr
    Fix race condition indicated by compiler warning about a variable that
    may be used before being set.
    Revision 1.1  2011/11/22 17:15:14Z  glenns
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
#include "nand.h"
#include "ntm.h"

#include <fxdevapi.h>
#include <fhstdclrnand.h>
#include <nandparam.h>
#define NTMNAME "NTSTDCLRNAND"

#define NAND_ONFI_ID_SIZE            (4)
#define CMD_READ_MODE                (0x0)


#define STDCLRNAND_CUSTOM_SPARE_SIZE (128)
#define STDCLRNAND_TAG_OFFSET        (0x2)
#define STDCLRNAND_TAG_LENGTH        (0x3)
#define STDCLRNAND_FLAGS_OFFSET      (0x10)
#define STDCLRNAND_FACTORYBAD_OFFSET (0x0)
#define STDCLRNAND_WRITTEN_WITH_ECC  (0)
#define SERIALNUM_SIZE               (16)
#define SERIALNUM_COPIES             (16)

/*  The following define is for testing engineering samples from Micron
    that are known to have an issue of the spare area size only being
    96 bytes rather than the advertised 128. This code should be removed
    once production parts are tested.
*/
#define STDCLRNAND_DEVICE_ESAMPLE    (1)

/* Micron enhanced clear nand DUAL PLANE SUPPORT
*/
#define PLANE0                       (0)
#define PLANE1                       (1)

/* Micron enhanced clear nand OPERATION STATUS OPERATION
*/
#define OP_READ                     (0x1)
#define OP_WRITE                    (0x2)
#define OP_ERASE                    (0x4)


struct tagNTMDATA
{
    D_UINT32            ulPhysBlockSize;
    D_UINT16            uPhysPageSize;
    D_UINT16            uPhysSpareSize;
    D_UINT32            ulPagesPerChip;

    D_UINT16            uLinearPageAddrMSB;     /* how much to >> linear address to 
                                                                                         get chip page index  */
    D_UINT16            uChipPageIndexLSB;
    D_UINT16            uLinearBlockAddrLSB;    /* how much to >> linear address to 
                                                                                         get block index */
    D_UINT16            uLinearChipAddrMSB;     /* how much to >> linear address to
                                                                                         get chip select */

    NTMINFO             NtmInfo;                /* information visible to upper layers */
    NTMHOOKHANDLE       hHook;
    const FFXNANDCHIP   *pChipInfo;             /* chip information                    */
    FFXDEVHANDLE        hDev;
    PNANDCTL            pNC;
    unsigned            nChips;
    unsigned            nChipStart;             /* Starting chip select number */

    unsigned            uTagOffset;             /* Offset in spare area for the tag     */
    unsigned            uFlagsOffset;           /* Offset in spare area for the flags   */

    D_BYTE              abID[NAND_ID_SIZE];    
    D_BYTE              abSN[MAX_CHIPS][SERIALNUM_SIZE];
    D_BYTE              abONFIID[NAND_ONFI_ID_SIZE];
    FFX_PARAMETER_PAGE  ParamPage;
};

#define DATA_IO_COUNT           (hNTM->NtmInfo.uPageSize / sizeof(NAND_IO_TYPE))
#define SPARE_IO_COUNT          (hNTM->NtmInfo.uSpareSize / sizeof(NAND_IO_TYPE))

/*  Currently we are only supporting one chip. There are typical VOLUME related commands
    for implemening multiple chips. This can be used here in future.
*/
#define CHIP_OF_PAGE(h, pg)      ((h)->nChipStart)

/*  The following macros define interfaces to the various hook functions
*/
#define SET_CHIP_SELECT(pg, mod)            (FfxHookNTSTDCLRNandSetChipSelect(hNTM->pNC, CHIP_OF_PAGE(hNTM, pg), (mod)))
#define CHIP_READY_WAIT()                   (FfxHookNTSTDCLRNandReadyWait(hNTM->pNC))
#define SET_CHIP_COMMAND(x)                 (FfxHookNTSTDCLRNandSetChipCommand(hNTM->pNC, (x)))
#define SET_CHIP_COMMAND_NOWAIT(x)          (FfxHookNTSTDCLRNandSetChipCommandNoWait(hNTM->pNC, (x)))
#define NAND_DATA_IN(x, y)                  (FfxHookNTSTDCLRNandDataIn(hNTM->pNC, (x), (y)))
#define NAND_STATUS_IN()                    (FfxHookNTSTDCLRNandStatusIn(hNTM->pNC))
#define NAND_DATA_OUT(x, y)                 (FfxHookNTSTDCLRNandDataOut(hNTM->pNC, (x), (y)))
#define NAND_ADDR_OUT(addr)                 (FfxHookNTSTDCLRNandAddressOut(hNTM->pNC, (addr)))


/*  This NTM supports the optional NativePageRead interface:
*/
static FFXIOSTATUS  NativePageRead(  NTMHANDLE hNTM, D_UINT32 ulStartPage, D_BUFFER *pPages, D_BUFFER *pSpares, D_UINT32 ulCount);

/*  Internal helper routines used primarily by the new FIM interface routines
*/
static FFXIOSTATUS  ReadSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);
static FFXSTATUS    RandomReadSetup(NTMHANDLE hNTM, D_UINT32 ulChipPage, unsigned nOffset, unsigned nPlane);
static FFXSTATUS    RandomWriteSetup(NTMHANDLE hNTM, unsigned nOffset, unsigned nPlane);
static D_BOOL       MicronSTDCLRNANDECCStatus(NTMHANDLE hNTM, FFXIOSTATUS *pIOStat);
static D_BOOL       SCLRNANDNtmTagDecode(D_BUFFER *pDest, const D_BUFFER *pSrc);
static D_BOOL       SCLRNANDNtmTagEncode(D_BUFFER *pDest, const D_BUFFER *pSrc);
static FFXIOSTATUS  IsBadBlock(NTMHANDLE hNTM, const FFXNANDCHIP *pChipInfo, D_UINT32 ulBlock);
static FFXSTATUS    ReadID(NTMHANDLE hNTM, D_BYTE *pabID, unsigned nChip);
static FFXSTATUS    ReadSerialNumber(NTMHANDLE hNTM, D_UCHAR *pucID, unsigned nChip);
static FFXSTATUS    ReadONFIID(NTMHANDLE hNTM, D_BYTE *pabID, unsigned nChip);
static unsigned     IdentifyFlash(NTMHANDLE hNTM, FFXFIMBOUNDS *pBounds);
static FFXSTATUS    ReadParameterPage(NTMHANDLE hNTM, D_BYTE *pabID, unsigned nChip);
static FFXSTATUS    ExecuteChipCommand(NTMHANDLE hNTM, D_UCHAR ucCommand, D_UINT32 ulAbsolutePage);
static void         SetBlockAddress(NTMHANDLE hNTM, D_UINT64 ulOffset);
static void         SetLogicalPageAddress(NTMHANDLE hNTM, D_UINT32 ulPageNum, D_UINT32 ulPageOffset, unsigned nPlane);

/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_stdclrnand =
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
    IORequest,
    NativePageRead
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
    FFXFIMBOUNDS        bounds;
    D_UINT32            ulTotalBlocks;
    D_BOOL              fSuccess = FALSE;
    unsigned            nChip;
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    pNTM->pNC = FfxHookNTSTDCLRNandCreate(hDev);
    if(!pNTM->pNC)
        goto CreateCleanup;

    /*  Get the array bounds now so we know how far to scan
    */
    FfxDevGetArrayBounds(hDev, &bounds);

    /*  Here we read both Flash ID as well as Parameter Page of the Enhanced ClearNAND Flash
             We get the Flash Geometry here
    */
    pNTM->nChips = IdentifyFlash(pNTM, &bounds);
    if(!pNTM->nChips)
        goto CreateCleanup;

    /*  This NTM supports inbuilt ECC inside the Flash Part itself
    */
    DclAssert(pNTM->pChipInfo->pChipClass->uEdcRequirement == 0);
    DclAssert(pNTM->pChipInfo->pChipClass->uEdcCapability == 0);

     /*  Consider any reserved size, as well as the max array size,
              computing ulTotalBlocks.
    */
    ulTotalBlocks = FfxDevApplyArrayBounds(pNTM->pChipInfo->pChipClass->ulChipBlocks * pNTM->nChips,
                                           pNTM->pChipInfo->pChipClass->ulBlockSize, &bounds);

    pNTM->uLinearPageAddrMSB        = pNTM->pChipInfo->pChipClass->uLinearPageAddrMSB;
    pNTM->uChipPageIndexLSB         = pNTM->pChipInfo->pChipClass->uChipPageIndexLSB;
    pNTM->uLinearBlockAddrLSB       = pNTM->pChipInfo->pChipClass->uLinearBlockAddrLSB;
    pNTM->uLinearChipAddrMSB        = pNTM->pChipInfo->pChipClass->uLinearChipAddrMSB;


    /*  Move these into the main NTM structure for easier access
    */
    pNTM->ulPhysBlockSize            = pNTM->pChipInfo->pChipClass->ulBlockSize;
    pNTM->uPhysPageSize              = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->uPhysSpareSize             = pNTM->pChipInfo->pChipClass->uSpareSize;

    /*  Set the block/page/spare sizes we will present to higher layers
    */
    pNTM->NtmInfo.ulBlockSize        = pNTM->ulPhysBlockSize;
    pNTM->NtmInfo.uPageSize          = pNTM->uPhysPageSize;

/*  TODO: The following code is necessary because the engineering sample
    parts used to develop the NTM had a truncated spare area size. This
    code should be removed once production parts are tested.
*/
#if STDCLRNAND_DEVICE_ESAMPLE
    pNTM->NtmInfo.uSpareSize         = 96;
#else
    pNTM->NtmInfo.uSpareSize         = pNTM->uPhysSpareSize;
#endif

    pNTM->NtmInfo.ulChipBlocks       = pNTM->pChipInfo->pChipClass->ulChipBlocks;

    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    pNTM->NtmInfo.uEdcCapability     = pNTM->pChipInfo->pChipClass->uEdcCapability;
    pNTM->NtmInfo.ulEraseCycleRating = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;

    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    DclAssert((pNTM->NtmInfo.ulBlockSize % 1024UL) == 0);


    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

    pNTM->NtmInfo.ulTotalBlocks     = ulTotalBlocks;
    pNTM->NtmInfo.uPagesPerBlock    = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);
    pNTM->NtmInfo.uMetaSize         = FFX_NAND_TAGSIZE;

    pNTM->ulPagesPerChip             = pNTM->NtmInfo.ulChipBlocks * pNTM->NtmInfo.uPagesPerBlock;
#if D_DEBUG
    FFXPRINTF(1, (NTMNAME"-Create() BlockSize %d\n",pNTM->NtmInfo.ulBlockSize));
    FFXPRINTF(1, (NTMNAME"-Create() PageSize %d\n",pNTM->NtmInfo.uPageSize));
    FFXPRINTF(1, (NTMNAME"-Create() SpareSize %d\n",pNTM->NtmInfo.uSpareSize));
    FFXPRINTF(1, (NTMNAME"-Create() TotalBlocks %d\n",pNTM->NtmInfo.ulTotalBlocks));
    FFXPRINTF(1, (NTMNAME"-Create() PagesPerBlock %d\n",pNTM->NtmInfo.uPagesPerBlock));
#endif
     /*  CLEAR NAND does not require any external EDC engine:
    */
    pNTM->NtmInfo.uDeviceFlags &= ~(DEV_REQUIRES_ECC);

    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

    
    for(nChip = pNTM->nChipStart; 
        nChip < pNTM->nChipStart + pNTM->nChips; 
        nChip ++)
    {
        /*  Get the Chip serial number
        */
        ReadSerialNumber(pNTM, &pNTM->abSN[nChip - pNTM->nChipStart][0], nChip);
    }            
  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

     /*  Assign the spare area format, and set EDC parameters to indicate
        that no EDC is used.
    */
    pNTM->NtmInfo.fEdcParameterized = FALSE;
    pNTM->NtmInfo.uCustomSpareFormat = FALSE;
    pNTM->NtmInfo.fEdcProtectsMetadata = TRUE;
    pNTM->uTagOffset    = STDCLRNAND_TAG_OFFSET;
    pNTM->uFlagsOffset  = STDCLRNAND_FLAGS_OFFSET;

    /*  Make sure the parent has a pointer to our NTMINFO structure
    */
    *ppNtmInfo = &pNTM->NtmInfo;

    fSuccess = TRUE;
CreateCleanup:

    if(pNTM && !fSuccess)
    {
        if(pNTM->hHook)
            FfxHookNTSTDCLRNandDestroy(pNTM->pNC);

        if(pNTM->pChipInfo)
        {
            DclMemFree((void *)pNTM->pChipInfo->pChipClass);
            DclMemFree((void *)pNTM->pChipInfo);
        }

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

    if(hNTM->pNC)
        FfxHookNTSTDCLRNandDestroy(hNTM->pNC);

    if(hNTM->pChipInfo)
    {
        DclMemFree((void *)hNTM->pChipInfo->pChipClass);
        DclMemFree((void *)hNTM->pChipInfo);
    }

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
static FFXIOSTATUS PageRead (
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pTags,
    D_UINT32            ulCount,
    unsigned            nTagSize,
    D_BOOL              fUseEcc)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);
    D_UINT32            ulChipPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-PageRead() StartPage=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter(NTMNAME"-PageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    DclAssert((pTags && nTagSize) || (!pTags && !nTagSize));

    /*  If fUseEcc is FALSE, then the tag size (and the tag pointer, asserted
            above) must be clear.
    */
    DclAssert(fUseEcc || !nTagSize);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);
    ioStat.ffxStat = CHIP_READY_WAIT();
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    while(ulCount)
    {
    
        /*  Send the read command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

        if(hNTM->pChipInfo->pChipClass->ReadConfirm)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if (ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ffxStat = CHIP_READY_WAIT();
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        {
            FFXIOSTATUS ioValid = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);
            if(!MicronSTDCLRNANDECCStatus(hNTM, &ioValid))
            {
                ioStat.ffxStat = ioValid.ffxStat;
                break;
            }
            else
            {
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
        }

        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MODE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);

        NAND_DATA_IN((NAND_IO_TYPE*)pPages, (hNTM->NtmInfo.uPageSize ) / sizeof(NAND_IO_TYPE));
        NAND_DATA_IN((NAND_IO_TYPE*)spare.data, hNTM->NtmInfo.uSpareSize / sizeof(NAND_IO_TYPE));        
        if(pTags)
        {
            DclAssert(fUseEcc);
            DclAssert(pTags);
            DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

            /* If Tag is invalid, just zero them out and continue reading
                     */
            SCLRNANDNtmTagDecode(pTags, &spare.data[STDCLRNAND_TAG_OFFSET]);

            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                               "Encoded Tag=%lX Decoded Tag=%X\n",
                               *(D_UINT32 *)spare.data,
                               *(D_UINT16 *)pTags));
            pTags += nTagSize;
        }

        ulChipPage++;
        ulPage++;
        ioStat.ulCount++;
        ulCount--;
        pPages          += hNTM->NtmInfo.uPageSize;
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
        ulPage - The starting page to write, relative to any
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
    DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);

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

    /*  Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage % hNTM->ulPagesPerChip;
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

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);
        DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);

        /*  Send main page data.
        */
        NAND_DATA_OUT((NAND_IO_TYPE*)pPages, hNTM->NtmInfo.uPageSize/sizeof(NAND_IO_TYPE));

        /*  ECC is always on in Clear NAND, but functionally for FlashFX, we
            only write tags and flag bytes when ECC is requested to be on.
            Add this code here to adhere to traditional FlashFX behavior.
        */
        if (fUseEcc)
        {
            if (pTags)
                SCLRNANDNtmTagEncode(&(spare.data[STDCLRNAND_TAG_OFFSET]), pTags);
            spare.data[STDCLRNAND_FLAGS_OFFSET] = STDCLRNAND_WRITTEN_WITH_ECC;

        }
        else
        {
            DclAssert(pTags == NULL);
            DclAssert(nTagSize == 0);
        }

        /*  Now send the spare area.
        */
        NAND_DATA_OUT((NAND_IO_TYPE*)&spare.data, hNTM->NtmInfo.uSpareSize/sizeof(NAND_IO_TYPE));

        /*  Now actually program the data...
        */
        ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulChipPage);
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
        pPages += hNTM->uPhysPageSize;
    }

/*  PageWriteCleanup:  */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));
    return ioStat;
}
/*-------------------------------------------------------------------
    Public: NativePageRead()

    Read an exact image of the data on the media with EDC results
    considered.

    Reading the spare area is optional.

    Not all NTMs support this function; not all NAND controllers
    can support it.

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
static FFXIOSTATUS NativePageRead(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pPages,
    D_BUFFER           *pSpares,
    D_UINT32            ulCount)
{
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32            ulChipPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-NativePageRead() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-NativePageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);


    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);
    
    while(ulCount)
    {
        FFXIOSTATUS ioValid;
        
        /*  Send the programming command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

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

        /*  Check for data corrections:
        */        
        if (!MicronSTDCLRNANDECCStatus(hNTM,&ioValid))
            break;


        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MODE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-NativePageRead() reading page data (%U %u-byte elements)\n",
            DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

        NAND_DATA_IN((NAND_IO_TYPE *) pPages, (D_UINT16) DATA_IO_COUNT);
        if(pSpares)
        {
            /*  Read the spare area
            */
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-NativePageRead() reading spare data (%U %u-byte elements)\n",
                SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));

            NAND_DATA_IN((NAND_IO_TYPE *) pSpares, (D_UINT16) SPARE_IO_COUNT);

            pSpares += hNTM->NtmInfo.uSpareSize;
        }

        
        ioStat.ulCount++;
        ulCount--;
        pPages                 += hNTM->NtmInfo.uPageSize;        
        ulChipPage++;
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-NativePageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_UINT32            ulChipPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageRead() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter(NTMNAME"-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;

    ulChipPage = ulPage & (hNTM->ulPagesPerChip-1);
    ioStat.ffxStat = CHIP_READY_WAIT();
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto PageReadCleanup;
    
    while(ulCount)
    {

        /*  Send the read command and address for the data page
        */
        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);
        
        if(hNTM->pChipInfo->pChipClass->ReadConfirm)
        {
            ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }
        
        ioStat.ffxStat = CHIP_READY_WAIT();
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        
        ioStat.ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MODE);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-RawPageRead() reading page data (%U %u-byte elements)\n",
            DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));
        
        NAND_DATA_IN((NAND_IO_TYPE*)pPages, hNTM->NtmInfo.uPageSize / sizeof(NAND_IO_TYPE));
        if(pSpares)
        {
            /*  Read the spare area
                    */
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                NTMNAME"-RawPageRead() reading spare data (%U %u-byte elements)\n",
                SPARE_IO_COUNT, sizeof(NAND_IO_TYPE)));
            NAND_DATA_IN((NAND_IO_TYPE*)pSpares, hNTM->NtmInfo.uSpareSize / sizeof(NAND_IO_TYPE));
            pSpares += hNTM->NtmInfo.uSpareSize;
        }

        ulPage++;
        ulChipPage++;
        ioStat.ulCount++;
        ulCount--;
        pPages += hNTM->NtmInfo.uPageSize;
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
    D_UINT32            ulChipPage;


     FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-RawPageWrite() StartPage=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));


    DclProfilerEnter(NTMNAME"-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);
    
    /*    Calculate ulChipPage so that it is relative to the chip
    */
    ulChipPage = ulPage % hNTM->ulPagesPerChip;
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

        SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

        /*  Send main page data.
        */
        
        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
            NTMNAME"-RawPageWrite() writing page data (%U %u-byte elements)\n",
            DATA_IO_COUNT, sizeof(NAND_IO_TYPE)));

        NAND_DATA_OUT((NAND_IO_TYPE*)pPages, hNTM->NtmInfo.uPageSize/sizeof(NAND_IO_TYPE));

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
            DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);
            D_BUFFER   *pSpare = spare.data;

            /*  We were not supplied any data to write into the spare
                area, so initialize the transfer buffer with all 0xFFs
            */
            DclMemSet(pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);

            NAND_DATA_OUT((NAND_IO_TYPE *) pSpare, (D_UINT16) SPARE_IO_COUNT);
        }

       /*  Now actually program the data...
       */
       ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulChipPage);
       if(ioStat.ffxStat != FFXSTAT_SUCCESS)
           break;

       ioStat.ulCount++;

       FFXERRORINJECT_WRITE("RawPageWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
           break;

        pPages += hNTM->uPhysPageSize;

        ulPage++;
        ulChipPage++;
        ulCount--;
    }

/*  PageWriteCleanup:  */

    DclProfilerLeave(0UL);

   FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));
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
     FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
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
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_UINT32            ulCount,
    D_BUFFER           *pTags,
    unsigned            nTagSize)
{
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    unsigned        nLength;
    D_UINT32        ulChipPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenRead() StartPage=%lX Count=%lX TagSize=%u\n", ulPage, ulCount, nTagSize));

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);
    DclAssert(ulCount * nTagSize <= D_UINT16_MAX);

    nLength = (ulCount * nTagSize);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_READ);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        goto HiddenReadCleanup;

     ulChipPage = ulPage & (hNTM->ulPagesPerChip - 1);

    while(nLength)
    {
        DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);
        NAND_IO_TYPE        *pIO = (NAND_IO_TYPE *)spare.data;

        DclAssert(pIO);

        ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE0);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Get spare area data. Don't assume we know where the tag is.
        */
        NAND_DATA_IN(pIO, SPARE_IO_COUNT);

        /*  TODO: Could be injecting the error into a portion of the spare
            area besides where the tag is, useless, but benign.
        */
        FFXERRORINJECT_READ("HiddenRead",
                            ulChipPage / (hNTM->NtmInfo.uPageSize >> ADDR_LINES_PER_ADDRESSABLE_UNIT),
                            spare.data,
                            hNTM->NtmInfo.uSpareSize,
                            &ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        if(!MicronSTDCLRNANDECCStatus(hNTM,&ioStat))
        {
            pTags[0] = 0;
            pTags[1] = 0;

            FFXPRINTF(1, (NTMNAME"-HiddenRead() invalid tag for page %lX, status %s\n", 
                ulPage, FfxDecodeIOStatus(&ioStat)));

            /*  Failure is indicated by the tag bytes being NULL
                           not by a status code.
                     */                            
            ioStat.ffxStat = FFXSTAT_SUCCESS;
        }
        else
        {
            /*  Decode the tags. Ignore the boolean result for now, as
                           this procedure takes care of zeroing the tags for us.
                     */
            (void)SCLRNANDNtmTagDecode(pTags, &spare.data[STDCLRNAND_TAG_OFFSET]);
        }

        ulChipPage++;
        ulPage++;
        ioStat.ulCount++;
        nLength         -= nTagSize;
        pTags           += nTagSize;
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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);
    NAND_IO_TYPE       *pIO;
    D_UINT32            ulChipPage = ulPage % hNTM->ulPagesPerChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        NTMNAME"-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclAssert(hNTM);
    pIO = (NAND_IO_TYPE *)spare.data;
    DclAssert(pIO);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Encode tags...
        */
        DclMemSet(spare.data, ERASED8, hNTM->NtmInfo.uSpareSize);
        SCLRNANDNtmTagEncode(&(spare.data[STDCLRNAND_TAG_OFFSET]), pTag);

        if(ioStat.ffxStat  == FFXSTAT_SUCCESS)
        {
            /*  Set the chip selects and mask off the chip select bits.
                This must be done inside the loop because the chip select is
                reset to read-mode inside the ExecuteChipCommand() function
                (proven to be necessary for the TX4938).
            */
            ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
            if(ioStat.ffxStat  == FFXSTAT_SUCCESS)
            {
                ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    /*  Calculate ulChipPage so that it is relative to the chip
                    */
                    SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

                    RandomWriteSetup(hNTM, hNTM->uPhysPageSize, PLANE0);

                    NAND_DATA_OUT(pIO, SPARE_IO_COUNT);

                    ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulChipPage);
                    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    {
                        ioStat.ulCount = 1;

                        FFXERRORINJECT_WRITE("HiddenWrite", ulPage, &ioStat);
                    }
                }
            }
        }
    }

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
        D_UINT64    ulChipOffset = ulBlock * (D_UINT64)hNTM->NtmInfo.ulBlockSize;
        D_UINT32    ulRowIndex   = (D_UINT32)(ulChipOffset >> hNTM->uLinearBlockAddrLSB);

        if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        {

            /*  Tell the flash to erase those blocks now
            */
            ioStat.ffxStat = SET_CHIP_COMMAND(CMD_ERASE_BLOCK);
            if(ioStat.ffxStat == FFXSTAT_SUCCESS)
            {
                SetBlockAddress(hNTM, ulChipOffset);
                ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_ERASE_BLOCK_START, ulRowIndex);

                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    ioStat.ulCount = 1;

                    FFXERRORINJECT_ERASE("BlockErase", ulBlock, &ioStat);
                }
            }
        }

    }

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_BUFFER            *pIO;
    DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);

    DclAssert(hNTM);
    pIO = spare.data;
    DclAssert(pIO);

    ioStat = ReadSpareArea(hNTM, ulPage, pIO);
    if((ioStat.ulCount == 1) &&
        ((ioStat.ffxStat == FFXSTAT_SUCCESS) ||
         (ioStat.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)))

    {
        D_UINT32 ulFlags = 0;

        /*    If the tag area is within 1 bit of being erased, then we know
            that a tag was not written. If it is anything else, we know a
            tag was written. Toshiba PBA always uses legacy tag size.

            REFACTOR: Since EDC is always on in PBA NAND, there may be no
            reason to use the "Erased1Bit macros. It's no help to consider
            using the read status bit to determine if an uncorrectable
            error occured, because the depth of correction inside the
            device in not visible to us.
        */
        if(!FfxNtmHelpIsRangeErased1Bit(&pIO[STDCLRNAND_TAG_OFFSET], STDCLRNAND_TAG_LENGTH))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);

        ulFlags += pIO[STDCLRNAND_FLAGS_OFFSET];
        if(DclBitCount(ulFlags) <= 4)
            ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
        else
            ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
     }
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
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;

    DclAssert(hNTM);

    ioStat = IsBadBlock(hNTM, hNTM->pChipInfo, ulBlock);

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
    FFXIOSTATUS     ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    D_UINT32           ulPagesRemaining;
    D_UINT32           ulPage;
    D_UINT32           ulPagesPerBlock;
    D_BOOL             fMarked = FALSE;
    D_BOOL             fMarkLastPage;
    const FFXNANDCHIP *pChipInfo;


    /*  Parameter checking
    */
    DclAssert(hNTM);
    pChipInfo = hNTM->pChipInfo;
    DclAssert(pChipInfo);
    DclAssert(ulBlockStatus == BLOCKSTATUS_FACTORYBAD);

    /*  We only need to write the mark in one of the first two pages, or
        the last page for devices with block markers in that location.
        (as of FlashFX 3.0, BBM does not require every page to be marked).
    */
    ulPagesRemaining = 2;

    /*  Compute the number of pages per block once, before the loop
    */
    DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
    ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

    ulPage = ulBlock * ulPagesPerBlock;
    fMarkLastPage = pChipInfo->pChipClass->fLastPageFBB;

    while(ulPagesRemaining)
    {
        DCLALIGNEDBUFFER    (spare, data, STDCLRNAND_CUSTOM_SPARE_SIZE);
        D_BUFFER *pSpare = spare.data;

        DclAssert(pSpare);

        /*  Mark block, and do so only if no bad block mark already
            exists.
        */
        ioStat = ReadSpareArea(hNTM, ulPage, (void *)pSpare);
        if(IOSUCCESS(ioStat, 1))
        {
            if(ulBlockStatus == BLOCKSTATUS_FACTORYBAD)
            {
                if(pSpare[NSDEFAULTMLC_FACTORYBAD_OFFSET] != ERASED8)
                {
                    fMarked = TRUE;
                    goto ECLRNANDSetBlockTypeCleanup;
                }
                else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
                {
                    /*  For 16-bit interfaces, one of the first two bytes
                        will be marked.
                    */
                    if(pSpare[NSDEFAULTMLC_FACTORYBAD_OFFSET + 1] != ERASED8)
                    {
                        fMarked = TRUE;
                        goto ECLRNANDSetBlockTypeCleanup;
                    }
                }

                pSpare[NSDEFAULTMLC_FACTORYBAD_OFFSET + 0] = 0;
                pSpare[NSDEFAULTMLC_FACTORYBAD_OFFSET + 1] = 0;

                ioStat = WriteSpareArea(hNTM, ulPage, (void *)pSpare);
                if(IOSUCCESS(ioStat, 1))
                {
                    fMarked = TRUE;
                }
            }
        }

        ulPage++;
        ulPagesRemaining--;

        /*  If device specifies that bad block marks may be in the last
            page of a block, try to mark that as well.
        */
        if (!ulPagesRemaining && fMarkLastPage)
        {
            ulPagesRemaining = 1;
            fMarkLastPage = FALSE;
            /*  Last page in a block = start page + (number of pages -1):
            */
            ulPage = ulBlock * ulPagesPerBlock + ulPagesPerBlock - 1;
        }
    }

 ECLRNANDSetBlockTypeCleanup:

    /*  If at least one page was marked, consider it good.
    */
    if(fMarked)
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    else
        ioStat.ffxStat = FFXSTAT_FIMIOERROR;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME":SetBlockStatus() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));
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
        NTMNAME":ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, (NTMNAME":ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        NTMNAME":ParameterGet() returning status=%lX\n", ffxStat));
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
        NTMNAME":ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
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
        NTMNAME":ParameterSet() returning status=%lX\n", ffxStat));
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
                     *                                                   *
                     *      Helper Functions                      *
                     *                                                   *
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
        D_UINT32 ulChipPage = ulPage & (hNTM->ulPagesPerChip - 1);

        while(ulCount)
        {
            ioStat.ffxStat = RandomReadSetup(hNTM, ulChipPage, hNTM->uPhysPageSize, PLANE0);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            NAND_DATA_IN((NAND_IO_TYPE *)pSpare, (D_UINT16)SPARE_IO_COUNT);
            pSpare += hNTM->uPhysSpareSize;
            
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
    Public: WriteSpareArea()

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
    FFXIOSTATUS         ioStat = DEFAULT_GOOD_PAGEIO_STATUS;
    const NAND_IO_TYPE *pIO = (NAND_IO_TYPE *)pSpare;
    D_UINT32            ulChipPage = ulPage % hNTM->ulPagesPerChip;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEINDENT),
        NTMNAME"-WriteSpareArea() Page=%lX\n", ulPage));

    DclAssert(hNTM);
    DclAssert(pSpare);

    /*  Set the chip selects and mask off the chip select bits: only want
        addresses relative to the chip for the rest of this.
    */
    ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        if(ioStat.ffxStat  == FFXSTAT_SUCCESS)
        {
            /*  Set the chip selects and mask off the chip select bits.
                This must be done inside the loop because the chip select is
                reset to read-mode inside the ExecuteChipCommand() function
                (proven to be necessary for the TX4938).
            */
            ioStat.ffxStat = SET_CHIP_SELECT(ulPage, MODE_WRITE);
            if(ioStat.ffxStat  == FFXSTAT_SUCCESS)
            {
                ioStat.ffxStat = SET_CHIP_COMMAND(CMD_PROGRAM);
                if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                {
                    /*  Calculate ulChipPage so that it is relative to the chip
                    */
                    SetLogicalPageAddress(hNTM, ulChipPage, 0, PLANE0);

                    RandomWriteSetup(hNTM, hNTM->uPhysPageSize, PLANE0);

                    NAND_DATA_OUT(pIO, SPARE_IO_COUNT);

                    ioStat.ffxStat = ExecuteChipCommand(hNTM, CMD_PROGRAM_START, ulChipPage);
                    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
                    {
                        ioStat.ulCount = 1;

                        FFXERRORINJECT_WRITE("WriteSpareArea", ulPage, &ioStat);
                    }
                }
            }
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        NTMNAME"-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));
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
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-RandomReadSetup() ChipPage=%lX Offset=%x Plane=%u\n", ulChipPage, nOffset, nPlane));

    DclAssert(hNTM);
    DclAssert(nPlane == PLANE0);

    /*  Send the read command and address for the data page
    */
    ffxStat = SET_CHIP_COMMAND(CMD_READ_MAIN);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        SetLogicalPageAddress(hNTM, ulChipPage, nOffset, nPlane);

        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MAIN_START);

        if(ffxStat == FFXSTAT_SUCCESS)
        {
            ffxStat = CHIP_READY_WAIT();
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MODE);
            }
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-RandomReadSetup() returning status %lX\n", ffxStat));


    return ffxStat;
}

/*-------------------------------------------------------------------
    Local: RandomWriteSetup()

    Prepare to read from a random offset within the spare area.

    Parameters:
        hNTM       - The NTM handle to use
        nOffset    - The offset to use
        nPlane     - The plane to use -- PLANE0 or PLANE1

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS RandomWriteSetup(
    NTMHANDLE       hNTM,
    unsigned        nOffset,
    unsigned        nPlane)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;
    D_UCHAR         ucAddress = 0;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-RandomWriteSetup() ChipPage=%lX Offset=%x Plane=%u\n", nOffset, nPlane));

    DclAssert(hNTM);
    DclAssert(nPlane == PLANE0);

    ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_WRITE_PAGE_RANDOM);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ucAddress = (D_UCHAR) nOffset;
        NAND_ADDR_OUT(ucAddress);

        nOffset >>= 8;

        ucAddress = (D_UCHAR) nOffset;
        NAND_ADDR_OUT(ucAddress);
    }


    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-RandomWriteSetup() returning status %lX\n", ffxStat));

    return ffxStat;
}

/*-------------------------------------------------------------------
    Local: SCLRNANDNtmTagDecode()

    Decode the allocation entry. The first two bytes are the standard
    media-independent value, and in keeping with the peculiarities of
    the Enhanced ClearNand spare area format.  Note that for properly
    corrected errors, the checkbyte *must* verify.

    Parameters:
        pDest - The destination buffer to fill with the 2-byte tag data.
        pSrc  - The 4-byte source buffer to process.

    Return Value:
        Returns TRUE if the tag was properly decoded and placed in
        the buffer specified by pDest, otherwise FALSE.  If FALSE
        is returned, the buffer specified by pDest will be set to
        NULLs.
-------------------------------------------------------------------*/
static D_BOOL SCLRNANDNtmTagDecode(
    D_BUFFER       *pDest,
    const D_BUFFER *pSrc)
{
    D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];
    D_UCHAR         ucZeroBits = ERASED8;
    D_UCHAR         ucIndex;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-SCLRNANDNtmTagDecode() pDest=%P pDest=%P\n",
        pDest, pSrc));

    DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));

    DclAssert(pDest);
    DclAssert(pSrc);

    /*  If the encoded tag is in the fully erased state, return
        successfully, after having set the decoded tag to the
        erased state as well. Note that for alignment purposes,
        the tag has to be examined byte-by-byte.

        Note that the ToshibaPBA NTM eschews the Hamming code,
        because the tag is covered by the internal ECC mechanism.
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
        FFXPRINTF(2, ("SCLRNANDNtmTagDecode() Unrecoverable tag error, Src=%02X %02X %02X %02X Corrected=%02X %02X %02X %02X\n",
            pSrc[3], pSrc[2], pSrc[1], pSrc[0],TempBuff[3], TempBuff[2], TempBuff[1], TempBuff[0]));

        pDest[0] = 0;
        pDest[1] = 0;

        return FALSE;
    }

    pDest[0] = TempBuff[0];
    pDest[1] = TempBuff[1];

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
    NTMNAME"-SCLRNANDNtmTagDecode() returning\n"));

    return TRUE;
}

/*-------------------------------------------------------------------
    Local: ECLRNANDNtmTagEncode()

    Build the allocation entry.  The first two bytes are the standard
    media-independent value, and the third byte then holds the check
    byte.  Note that for properly corrected errors, the checkbyte
    *must* verify.

    If the device reports a correctable error but the checkbyte does
    not verify, it is a sure indication that there are multiple bit
    errors in the tag that the Enhanced ClearNAND ECC mechanism has
    falsely corrected. This is a positive indicator that the tag has been
    invalidated.

    Parameters:
        pDest - The 4-byte destination buffer to fill with the
                2-byte tag data and its checkbyte.
        pSrc  - The 2-byte allocation entry.

    Return Value:
        Returns TRUE if the tag was properly encoded and placed in
        the buffer specified by pDest, otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL SCLRNANDNtmTagEncode(
    D_BUFFER          *pDest,
    const D_BUFFER    *pSrc)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        NTMNAME"-ECLRNANDNtmTagEncode() pDest=%P pSrc=%P\n",
        pDest, pSrc));

    DclAssert(pDest);
    DclAssert(pSrc);

    pDest[0] = pSrc[0];
    pDest[1] = pSrc[1];
    pDest[2] = (D_UINT8)(pDest[0] ^ ~pDest[1]);
    pDest[3] = ERASED8;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        NTMNAME"-ECLRNANDNtmTagEncode() returning\n"));

    return TRUE;
}

/*-------------------------------------------------------------------
    Local: MicronSTDCLRNANDECCStatus()

    Get the ECC status after a read.  The pIOStat->ffxStat and 
    pIOStat->op.ulPageStatus fields will be updated accordingly.

    Parameters:
        hNTM    - The NTM handle.
        pIOStat - The FFXIOSTATUS structure to update.

    Return Value:
        Returns TRUE if the the read was successful (regardless
        whether data was corrected or not.  Returns FALSE if
        there was uncorrectable data or some other error occurred.
-------------------------------------------------------------------*/
static D_BOOL MicronSTDCLRNANDECCStatus(
    NTMHANDLE       hNTM, 
    FFXIOSTATUS    *pIOStat)
{
    DclAssert(hNTM);
    DclAssert(pIOStat);
    
    /*  Using HW ECC, determine if this page was corrected
    */
    pIOStat->ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_STATUS);
    if(pIOStat->ffxStat == FFXSTAT_SUCCESS)
    {
        NAND_IO_TYPE    Status;
    
        Status = NAND_STATUS_IN();
        Status = BENDSWAP(Status);  /* unsafely swaps volatile memory ptrs */
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

    FFXPRINTF(1, (NTMNAME"-MicronSTDCLRNANDECCStatus() read failed, status %s\n", FfxDecodeIOStatus(pIOStat)));

    return FALSE;
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

        if( spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET] != ERASED8 )
        {
            ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
            goto IsBadBlockCleanup;
        }
        else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
        {
            /*  For 16-bit interfaces, one of the first two bytes
                will be marked.
            */
            if( spare.data[NSOFFSETZERO_FACTORYBAD_OFFSET+1] != ERASED8 )
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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEUNDENT),
        "IsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: VerifyDeviceCompliance()

    Verify if the Flash chip complies with the requirements of this
    NTM. The flash device must be a ONFI 2.3 compliant or it must
    support EZNand feature atleast. If not, fail and return immediately.

    Parameters:
        ParamPageArr    - Parameter Page ARRAY

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS VerifyDeviceCompliance(
    D_BYTE              *ParamPageArr)
{
    D_UINT16 uFeaturesSupported = 0;

    /* Check if the parameter page's signature matches to "ONFI"
    */
    if(DclMemCmp(&ParamPageArr[SIGNATURE], FFX_ONFI_SIG, ONFI_SIG_SZ) != 0)
        return FFXSTAT_NOT_ONFI_COMPLIANT;
    
/*  TO DO: We're not going to do the check on device compliance
    right now because I think this code is buggy. Right now we'll
    just assume that if this NTM is in use, that whoever is using
    it is smart enough not to do so on a device that is not
    standard ClearNAND.
*/

    (void)uFeaturesSupported;

#if 0
    /* If we do not support Clear Nand products, the device has to be either ONFI 2.3 compliant or 
    support EZNAND feature. If not, return No support.
    */
    DCLLE2NE(&uFeaturesSupported, &ParamPageArr[FEATURES_SUPPORTED], sizeof(uFeaturesSupported));
    if((ParamPageArr[REVISION_NUMBER] & REV_NUM_ONFI_2_3) == 0)
    {
        if((uFeaturesSupported & FEATURE_EZNAND_SUPPORT) == 0)
            return FFXSTAT_NO_EZ_NAND_SUPPORT;
    }
#endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: IdentifyFlash()

    Read the Device ID and ONFI parameter page from the flash volume. Parse the
    parameter page and check if the volume complies with EZNand specification.
    If not, fail and return immediately.

    Parameters:
        hNTM           - The NTM handle.
        pBounds        - A pointer to the FFXFIMBOUNDS structure.

    Return Value:
        The number of volumes (0, 1, or 2) detected.
-------------------------------------------------------------------*/
static unsigned IdentifyFlash(
    NTMHANDLE       hNTM,
    FFXFIMBOUNDS   *pBounds)
{
    FFXSTATUS       ffxStat     = FFXSTAT_SUCCESS;
    D_BYTE          ParamPageArr[NAND_PARAM_PAGE_SZ];

    DclAssert(hNTM);
    DclAssert(pBounds);
    
    /*  Chip select calculations will be set CS0
    */
    hNTM->nChipStart = 0;
    /*  Read the Nand ID from the first volume in the array.
    */
    if(ReadID(hNTM, hNTM->abID, 0) != FFXSTAT_SUCCESS)
        return 0;

    /*  Read the ONFI ID from the device before proceeding to
        read parameter page
    */
    if(ReadONFIID(hNTM, hNTM->abONFIID, 0) != FFXSTAT_SUCCESS)
        return 0;

    DclMemSet(ParamPageArr, ERASED8, NAND_PARAM_PAGE_SZ);

    /*  Read the parameter page from the first volume in the array.
    */
    if(ReadParameterPage(hNTM, ParamPageArr, 0) != FFXSTAT_SUCCESS)
        return 0;
    
    /*  Verify if the device is supported by this NTM
    */
    if(VerifyDeviceCompliance(ParamPageArr) != FFXSTAT_SUCCESS)
        return 0;

    /*  Fill the NTM's parameter page array member with the data
        read from flash
    */
    ffxStat = FfxNandDecodeParameterPage(ParamPageArr, (FFX_PARAMETER_PAGE *)&hNTM->ParamPage);
    
    if (FFXSTAT_SUCCESS == ffxStat)
    {
        /*  We have the parameter page. Fill in FFXNANDCHIPCLASS
            and FFXNANDCHIP structures
        */
        hNTM->pChipInfo = FfxNandFillChipInfo(hNTM->abID, (FFX_PARAMETER_PAGE *)&hNTM->ParamPage);
        
        if (!hNTM->pChipInfo)
             return 0;
    }
    else
    {
        /*  This NTM is for supporting Micron's enhanced clear nand
            devices only and we have to fail if this support is not
            found
        */
        return 0;
        
    }

    /*  Standard ClearNAND does not support volumes like Enhanced
        ClearNAND, so here we return 1 to indicate as a single volume.
    */
    return 1;
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

    ffxStat = FfxHookNTSTDCLRNandSetChipSelect(hNTM->pNC, nChip, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
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

    return ffxStat;
}

/*-------------------------------------------------------------------
    Local: ReadONFIID()

    Read the ONFI ID from a NAND chip.

    Parameters:
        hNTM     - The NTM handle
        pabID    - A pointer to a NAND_ID_SIZE-byte buffer to receive
                   the ID
        nChip    - The chip select to use, 0 or 1.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ReadONFIID(
    NTMHANDLE       hNTM,
    D_BYTE         *pabID,
    unsigned        nChip)
{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-ReadONFIID() nChip=%u\n", nChip));

    ffxStat = FfxHookNTSTDCLRNandSetChipSelect(hNTM->pNC, nChip, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_RESET);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                ffxStat = SET_CHIP_COMMAND(CMD_READ_ID);
                if(ffxStat == FFXSTAT_SUCCESS)
                {
                    unsigned ii;

                    NAND_ADDR_OUT(0x20);

                    for(ii = 0; ii < NAND_ONFI_ID_SIZE; ii++)
                    {
                        NAND_IO_TYPE Temp;

                        NAND_DATA_IN(&Temp, 1);

                        pabID[ii] = (D_UCHAR) BENDSWAP(Temp);
                    }

                    FFXPRINTF(1, (NTMNAME"-ReadONFIID() NAND CS%u ID %02x %02x %02x %02x\n",
                                  nChip, pabID[0], pabID[1], pabID[2], pabID[3]));
                }
            }
        }
    }

    return ffxStat;
}

/*-------------------------------------------------------------------
    Local: ReadParameterPage()

    Read the ID from a NAND chip.

    Parameters:
        hNTM     - The NTM handle
        pabID    - A pointer to a NAND_ID_SIZE-byte buffer to receive
                   the ID
        nChip    - The chip select to use, 0 or 1.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
static FFXSTATUS ReadParameterPage(
    NTMHANDLE       hNTM,
    D_BYTE         *pabID,
    unsigned        nChip)

{
    FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, 0),
        NTMNAME"-ReadParameterPage() nChip=%u\n", nChip));

    ffxStat = FfxHookNTSTDCLRNandSetChipSelect(hNTM->pNC, nChip, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND(CMD_READ_PARAM_PAGE);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            NAND_ADDR_OUT(0x0);
            {
                DCLTIMER        t;
                volatile NAND_IO_TYPE Status;
                {
                    DclTimerSet(&t, NAND_TIME_OUT);
                    while(!DclTimerExpired(&t))
                    {
                        ffxStat = SET_CHIP_COMMAND(CMD_STATUS);
                        if(ffxStat == FFXSTAT_SUCCESS)
                        {
                            Status = NAND_STATUS_IN();
                            Status = BENDSWAP(Status);    /* unsafely swaps volatile memory ptrs */
                            if((D_UCHAR)Status & NANDSTAT_READY)
                                break;
                        }
                    }

                    Status = NAND_STATUS_IN();
                    Status = BENDSWAP(Status);            /* unsafely swaps volatile memory ptrs */
                    if(!(Status & NANDSTAT_READY))
                        return FFXSTAT_FAILURE;

                    ffxStat = SET_CHIP_COMMAND_NOWAIT(CMD_READ_MODE);
                    if(ffxStat == FFXSTAT_SUCCESS)
                    {
                        unsigned ii;

                        for(ii = 0; ii < NAND_PARAM_PAGE_SZ; ii++)
                        {
                            NAND_IO_TYPE Temp;
                            NAND_DATA_IN(&Temp, 1);
                            pabID[ii] = (D_UCHAR) BENDSWAP(Temp);
                        }
                    }
                }
            }
        }
    }

#if VERBOSE_CHIP_INFO
    {
        unsigned ii;

        FFXPRINTF(1, ("Parameter Page ->\n"));
        for(ii = 0; ii < NAND_PARAM_PAGE_SZ; ii++)
        {
            FFXPRINTF(1, ("Byte %03u - %02x \n", ii, pabID[ii]));
        }
        FFXPRINTF(1, ("Parameter Page <-\n"));
    }
#endif
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

    ffxStat = FfxHookNTSTDCLRNandSetChipSelect(hNTM->pNC, nChip, MODE_READ);
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        ffxStat = SET_CHIP_COMMAND(CMD_READ_SN);
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            NAND_ADDR_OUT(0);
            if(ffxStat == FFXSTAT_SUCCESS)
            {
                unsigned    cc;    

                for(cc = 0; cc < SERIALNUM_COPIES; cc++)
                {
                    unsigned        bb;
                    D_BUFFER        bBase[SERIALNUM_SIZE];
                    D_BOOL          fSuccess = TRUE;
                    NAND_IO_TYPE    Temp;
                            
                    /*  Read the base copy of the chip serial number
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
    D_UINT64        ulOffset)
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
    DclAssert(nPlane == PLANE0);

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

    ulPageIndex = ulPageNum;

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

#endif /* END FFXCONF_NANDSUPPORT */

