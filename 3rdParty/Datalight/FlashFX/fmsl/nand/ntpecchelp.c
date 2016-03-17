/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains helper functions which are used by multiple NTMs
    that use the parameterized ECC facilities.

    It was originally derived from revision 1.37 of nthelp.c. It basically
    all the same functions in it, only supplemented with the term "PECC" in
    their names to indicate that they are for use with parameterized ECC.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntpecchelp.c $
    Revision 1.19  2009/12/11 23:56:51Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  
    Revision 1.18  2009/10/06 21:31:59Z  garyp
    Modified to use the revamped single-bit ECC calculation and
    correction functions.
    Revision 1.17  2009/04/18 00:23:39Z  garyp
    Cleaned up the documentation -- no functional changes.
    Revision 1.16  2009/04/17 18:42:59Z  keithg
    Fixed bug 2620 to properly handle MLC spare area sizes.
    Revision 1.15  2009/04/09 03:42:55Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.14  2009/04/02 17:33:47Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.13  2009/03/27 06:07:40Z  glenns
    - Fix Bugzilla #2464: See documentaion in bugzilla report for details.
    Revision 1.12  2009/03/24 18:01:37Z  glenns
    - Removed code that automatically errors out if chip EDC
      requirement is greater than 4. Can do this now because FlashFX
      can now be configured to use weaker EDC mechanisms that
      chip requirements dictate.
    Revision 1.11  2009/03/17 17:45:48Z  glenns
    - Fix Bugzilla #2464: partial fix; repair an inappropriate return code.
    Revision 1.10  2009/03/09 18:37:55Z  glenns
    - Ancillary to fix for Bugzilla #2384: This Bugzilla fix requires NTM's
      that use custom spare formats and parameterized EDC to report
      the physical size of the spare area to the FIM clients and only
      the amount of spare area actually used to the parameterized
      EDC mechanism, and those values are not necessarily the same.
      This checkin accomplishes that adjustment for the parameterized
      EDC mechanism.
    Revision 1.9  2009/02/25 23:22:24Z  glenns
    - Added support for protecting tags with 32-bit CRC rather than
      checkbyte and Hamming code in the MLC case.
    - Re-engineered FfxNtmPECCTagEncode and
      FfxNtmPECCTagDecode to be more flexible in determining
      requirements for tag protection and implementing the correct
      algorithms.
    - Added 32-bit page status field for MLC devices when building
      the spare area.
    - Added ability to FfxNtmPECCDefaultMLCGetPageStatus to
      tolerate higher bit error rates in the page status.
    Revision 1.8  2009/02/18 19:35:56Z  glenns
    - Added code to prevent compiler warnings about unused formal
      parameters.
    Revision 1.7  2009/02/06 22:18:25Z  glenns
    - Added code to support CRC calculation and storage in the spare
      area for writes, and CRC evaluation for reads. Also added
      algorithm to bypass ECC calculation if CRC checks good.
    - Fixed erroneous copy-and-paste error.
    Revision 1.6  2009/01/29 06:42:24Z  glenns
    - Added new implementations for functions used to check and set
      block status for devices that use the parameterized ECC
      interface and/or custom spare area format.
    Revision 1.5  2009/01/27 07:37:02Z  glenns
    - Added revision history block.
    - Updated consistency checking for BuildSpareArea and
      CorrectPage.
    - Added loops to DefaultMLCBuildSpareArea and DefaultMLCCorrectPage
      to cover devices with 4kbyte pages.
    - Clarified commentary.
    - Fixed error in correctable error reporting.
    - Added code to encode "can't tell" status in the FFXIOSTATUS
      structure from the CorrectPage procedures. Part of a change
      which will eventually eliminate FFXSTATUS_FIMCORRECTABLESPARE
      and FFXSTATUS_CORRECTABLEMAIN indications.
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fimdev.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include "nandid.h"
#include "nand.h"
#include <errmanager.h>


/*-------------------------------------------------------------------
    The SmartMedia physical format spec, to which some NAND parts
    conform when it comes to marking blocks as bad from the factory,
    declares that byte 517 (the 6th byte of the spare area of a NAND
    page) of some page in that block will be some value other than
    0xFF if the block came bad from the factory.  In order to
    maximize our chances of discovering such factory bad blocks
    should the original BBM block list be destroyed, we attempt to
    establish this convention for all factory fresh bad blocks by
    calling the SetBlockStatus() function as the very first thing
    we do on virgin NAND flash.
-------------------------------------------------------------------*/
#define SSFDC_BAD_BLOCK_INDEX    5


static D_BOOL PECCCorrectDataSegment(D_UINT16 uCount,
                                     FX_ECC_PARAMS *pEccParams);

static void FfxNtmPECCOffsetZeroBuildSpareArea(FFXDEVHANDLE hDev,
                                               NTMINFO *pNtmInfo,
                                               FX_ECC_PARAMS *pEccParams,
                                               D_BUFFER *pSpare,
                                               const D_BUFFER *pTag,
                                               D_BOOL fUseEcc);

static void FfxNtmPECCSSFDCBuildSpareArea(FFXDEVHANDLE hDev,
                                          NTMINFO *pNtmInfo,
                                          FX_ECC_PARAMS *pEccParams,
                                          D_BUFFER *pSpare,
                                          const D_BUFFER *pTag,
                                          D_BOOL fUseEcc);

static void FfxNtmPECCDefaultMLCBuildSpareArea(FFXDEVHANDLE hDev,
                                               NTMINFO *pNtmInfo,
                                               FX_ECC_PARAMS *pEccParams,
                                               D_BUFFER *pSpare,
                                               const D_BUFFER *pTag,
                                               D_BOOL fUseEcc);

static void FfxNtmPECCOffsetZeroCorrectPage(NTMINFO *pNtmInfo,
                                            FX_ECC_PARAMS *pEccParams,
                                            const D_BUFFER *pSpare,
                                            const D_BUFFER *pReferenceSpare);

static void FfxNtmPECCSSFDCCorrectPage(NTMINFO *pNtmInfo,
                                       FX_ECC_PARAMS *pEccParams,
                                       const D_BUFFER *pSpare,
                                       const D_BUFFER *pReferenceSpare);

static void FfxNtmPECCDefaultMLCCorrectPage(NTMINFO *pNtmInfo,
                                            FX_ECC_PARAMS *pEccParams,
                                            const D_BUFFER *pSpare,
                                            const D_BUFFER *pReferenceSpare);

static FFXIOSTATUS FfxNtmPECCDefaultMLCGetPageStatus(NTMHANDLE hNTM,
                                                     NTMINFO *pNtmInfo,
                                                     PFNREADSPAREAREA pfnReadSpareArea,
                                                     D_UINT32 ulPage);

static D_BOOL IsWithinRange(D_UINT32 ulVal, D_UINT32 ulReference, D_UINT32 ulRange);

/*  REFACTOR: The declarations disabled here are functions that may yet need
    to be added for PECC (particularly when support is added for ONFI), but
    for which the current implementations from standard NTHelp will work okay.
*/
#if 0

static FFXIOSTATUS FfxNtmPECCOffsetZeroIsBadBlock(NTMHANDLE hNTM,
                                                  PFNREADSPAREAREA pfnReadSpareArea,
                                                  const FFXNANDCHIP *pChipInfo,
                                                  D_UINT32 ulBlock);

static FFXIOSTATUS FfxNtmPECCOffsetZeroSetBlockType(NTMHANDLE hNTM,
                                                    PFNREADSPAREAREA pfnReadSpareArea,
                                                    PFNWRITESPAREAREA pfnWriteSpareArea,
                                                    const FFXNANDCHIP *pChipInfo,
                                                    D_UINT32 ulBlock,
                                                    D_UINT32 ulBlockStatus);

static FFXIOSTATUS FfxNtmPECCSSFDCIsBadBlock(NTMHANDLE hNTM,
                                             PFNREADSPAREAREA pfnReadSpareArea,
                                             const FFXNANDCHIP *pChipInfo,
                                             D_UINT32 ulBlock);

FFXIOSTATUS FfxNtmPECCLegacyIsBadBlock(NTMHANDLE hNTM,
                                       PFNREADSPAREAREA pfnReadSpareArea,
                                       const FFXNANDCHIP *pChipInfo,
                                       D_UINT32 ulBlock);

FFXIOSTATUS FfxNtmPECCLegacySetBlockType(NTMHANDLE hNTM,
                                         PFNREADSPAREAREA pfnReadSpareArea,
                                         PFNWRITESPAREAREA pfnWriteSpareArea,
                                         const FFXNANDCHIP *pChipInfo,
                                         D_UINT32 ulBlock,
                                         D_UINT32 ulBlockStatus);
#endif




/*-------------------------------------------------------------------
    Public: FfxNtmPECCTagEncode()

    Encode a FlashFX tag in the standard format.  The first two bytes
    are the standard media-independent value, followed by a check byte
    calculated from that value, and a Hamming code ECC for the
    whole thing.

    *IMPORTANT NOTE* -- In some circumstances with some flash
    devices, this code can cause sensitivity to endianness issues.
    In particular, devices or controllers that designate specific
    bytes within the spare area for internal purposes may have
    unwanted interactions with the algorithms in this procedure.
    Consult your datasheet.

    Parameters:
        pNtmInfo - A pointer to the NtmInfo structure. This
                   structure contains information about custom
                   spare area formatting and device characteristics
        pSpare   - A pointer to the spare area which will receive
                   encoded tags.
        pSrc     - Source for raw tag bytes.
        fmt      - Indication of the spare area format to be
                   considered when encoding tags, if it is not
                   a custom format.

    Return Value:
        Returns TRUE if the tag was properly encoded and placed in
        the buffer specified by pDest, otherwise FALSE.
-------------------------------------------------------------------*/
void FfxNtmPECCTagEncode(
    NTMINFO        *pNtmInfo,
    D_BUFFER       *pSpare,
    const D_BUFFER *pSrc,
    NANDSPAREFMT    fmt)
{
    #if LEGACY_ENCODED_TAG_SIZE != 4
    #error "FFX: Bad legacy encoded tag size"
    #endif

    #if LEGACY_TAG_SIZE != 2
    #error "FFX: Bad legacy tag size"
    #endif

    DclAssert(pSpare);
    DclAssert(pSrc);

    if (pNtmInfo->uCustomSpareFormat)
    {
        D_UINT32 ulCrc;

    /*  If custom spare format is being used, the tag will be protected
        with a CRC, which may be stored in a separate location of the spare
        area rather than together with the tag.
    */

        pSpare[NSDEFAULTMLC_TAG_OFFSET] = pSrc[0];
        pSpare[NSDEFAULTMLC_TAG_OFFSET+1] = pSrc[1];
        pSpare[NSDEFAULTMLC_TAG_OFFSET+2] = ERASED8;
        pSpare[NSDEFAULTMLC_TAG_OFFSET+3] = ERASED8;
        ulCrc = DclCrc32Update(0, &(pSpare[NSDEFAULTMLC_TAG_OFFSET]), NSDEFAULTMLC_TAG_LENGTH);
        pSpare[NSDEFAULTMLC_TAGCRC] = (unsigned char)(ulCrc & 0x00FF);
        pSpare[NSDEFAULTMLC_TAGCRC+1] = (unsigned char)((ulCrc >> 8) & 0x00FF);
        pSpare[NSDEFAULTMLC_TAGCRC+2] = (unsigned char)((ulCrc >> 16) & 0x00FF);
        pSpare[NSDEFAULTMLC_TAGCRC+3] = (unsigned char)((ulCrc >> 24) & 0x00FF);
    }
    else
    {
        D_BUFFER *pDest;

        DclAssert(fmt);
        if (fmt == NSF_OFFSETZERO)
            pDest = &(pSpare[NSOFFSETZERO_TAG_OFFSET]);
        else
            pDest = &(pSpare[NSSSFDC_TAG_OFFSET]);

        pDest[0] = pSrc[0];
        pDest[1] = pSrc[1];
        pDest[2] = (D_UINT8)(pDest[0] ^ ~pDest[1]);
        pDest[3] = (D_UINT8)DclHammingCalculate(&pDest[0], 3, 0);

        /*  An encoded tag can never appear to be erased or within 1 bit thereof
        */
        DclAssert(!FfxNtmHelpIsRangeErased1Bit(pDest, LEGACY_ENCODED_TAG_SIZE));
    }
}


/*-------------------------------------------------------------------
    Public: FfxNtmPECCTagDecode()

    Decode a tag which was encoded with FfxNtmPECCTagEncode().

    *IMPORTANT NOTE* -- In some circumstances with some flash
    devices, this code can cause sensitivity to endianness issues.
    In particular, devices or controllers featuring a bus width more
    than one byte and that designate specific bytes within the spare
    area for internal purposes may have unwanted interactions with
    the algorithms in this procedure. Consult your datasheet.

    Parameters:
        pNtmInfo - A pointer to the NtmInfo structure. This
                   structure contains about custom spare area
                   formatting and device characteristics.
        pDest    - The destination buffer to fill with the 2-byte tag
                   data.
        pSpare   - The pointer to the spare area buffer containing
                   encoded tags.
        fmt      - Indication of the spare area format to be
                   considered when decoding tags, if it is not
                   a custom format.

    Return Value:
        Returns TRUE if the tag was properly decoded and placed in
        the buffer specified by pDest, otherwise FALSE.  If FALSE
        is returned, the buffer specified by pDest will be set to
        NULLs.
-------------------------------------------------------------------*/
D_BOOL FfxNtmPECCTagDecode(
    NTMINFO        *pNtmInfo,
    D_BUFFER       *pDest,
    const D_BUFFER *pSpare,
    NANDSPAREFMT    fmt)
{
    DclAssert(pDest);
    DclAssert(pSpare);

    if (pNtmInfo->uCustomSpareFormat)
    {
        D_UINT32 ulCrc, ulStoredCrc;
        const D_BUFFER *pSrc = &(pSpare[NSDEFAULTMLC_TAG_OFFSET]);

        /*  Get the CRC:
        */
        ulStoredCrc = (pSpare[NSDEFAULTMLC_TAGCRC+3] << 24)
                + (pSpare[NSDEFAULTMLC_TAGCRC+2] << 16)
                + (pSpare[NSDEFAULTMLC_TAGCRC+1] << 8)
                + (pSpare[NSDEFAULTMLC_TAGCRC]);

        /*  Calculate the CRC for the tag that is encoded in this
            spare area:
        */
        ulCrc = DclCrc32Update(0, pSrc, NSDEFAULTMLC_TAG_LENGTH);

        /* Compare the CRCs and react accordingly:
        */

        if (ulCrc == ulStoredCrc)
        {
            /*  All good
            */
            pDest[0] = pSrc[0];
            pDest[1] = pSrc[1];;
        }
        else
        {
            /*  CRC fault.
            */
            pDest[0] = 0;
            pDest[1] = 0;
            return FALSE;
       }
    }
    else
    {
        D_UINT32        ecc;
        D_BUFFER        TempBuff[LEGACY_ENCODED_TAG_SIZE];
        const D_BUFFER       *pSrc;

        DclAssert(LEGACY_ENCODED_TAG_SIZE == sizeof(D_UINT32));
        DclAssert(fmt);

        /*  Determine location of the tags based on the traditional
            spare format:
        */
        if (fmt == NSF_OFFSETZERO)
            pSrc = &(pSpare[NSOFFSETZERO_TAG_OFFSET]);
        else
            pSrc = &(pSpare[NSSSFDC_TAG_OFFSET]);


        /*  If the encoded tag is in the fully erased state, return
            successfully, after having set the decoded tag to the
            erased state as well.

            It is possible that a page/tag which is otherwise erased
            has a single bit error in the tag area which makes it
            appear as if the tag has been written.  In this event,
            we let the normal hamming code and check byte stuff
            fail, and return that the tag is bogus.  The allocator
            will then recycle the page, and it will subsequently
            get erased and re-used.
        */
        if(*(D_UINT32*)pSrc == ERASED32)
        {
            pDest[0] = ERASED8;
            pDest[1] = ERASED8;

            return TRUE;
        }

        DclMemCpy(TempBuff, pSrc, LEGACY_ENCODED_TAG_SIZE);

        ecc = DclHammingCalculate(&TempBuff[0], 3, TempBuff[3]);

        if(ecc != 0)
            DclHammingCorrect(&TempBuff[0], 3, ecc);

        /*  Check the validation code
        */
        if(TempBuff[2] != (D_UCHAR) (TempBuff[0] ^ ~TempBuff[1]))
        {
            FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, 0),
                "FfxNtmPECCTagDecode() Unrecoverable tag error, Src=%lX ecc=%lX Corrected=%lX\n",
                *(D_UINT32*)pSrc, ecc, *(D_UINT32*)&TempBuff[0]));

            pDest[0] = 0;
            pDest[1] = 0;

            return FALSE;
        }

        pDest[0] = TempBuff[0];
        pDest[1] = TempBuff[1];
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPECCBuildSpareArea()

    Initializes the spare area using Custom, OffsetZero or SSFDC
    format based on the type of chip being used, as specified
    by pChipInfo, or if a requirement exists to use a customized
    spare area format.

    Note that this function is used to build arrays of spare
    areas when emulating larger page sizes with small-block
    NAND.

    Likewise when emulating larger page sizes, we only record
    the tag value in the first one, and therefore the pTag
    pointer may be NULL if we do not want to record the tag
    for this given spare area.

    *Note* -- This function is only appropriate for use where
              software ECC is being used.

    Parameters:
        hDev       - device handle
        pNtmInfo   - pointer to NTM information
        pEccParams - pointer to structure parameterizing ECC function
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.
        fUseEcc    - A flag indicating whether ECCs should be used.
        pChipInfo  - A pointer to the FFXNANDCHIP structure
                     describing the chip being used.

    Return Value:
        Returns a status value indicating success or failure.
-------------------------------------------------------------------*/
void FfxNtmPECCBuildSpareArea(
    FFXDEVHANDLE        hDev,
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc,
    const FFXNANDCHIP  *pChipInfo)
{
    FX_ECC_PARAMS       localEccParams = *pEccParams;

    DclAssert(pChipInfo);

    if (pNtmInfo->uCustomSpareFormat)
    {
        FfxNtmPECCDefaultMLCBuildSpareArea(hDev, pNtmInfo, &localEccParams, pSpare, pTag, fUseEcc);
        pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
        return;
    }
    switch(FfxNtmHelpGetSpareAreaFormat(pChipInfo))
    {
        case NSF_OFFSETZERO:
        {
            FfxNtmPECCOffsetZeroBuildSpareArea(hDev, pNtmInfo, &localEccParams, pSpare, pTag, fUseEcc);
            pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
            return;
        }

        case NSF_SSFDC:
        {
            FfxNtmPECCSSFDCBuildSpareArea(hDev, pNtmInfo, &localEccParams, pSpare, pTag, fUseEcc);
            pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
            return;
        }

        case NSF_UNKNOWN:
        default:
        {
            FFXPRINTF(1, ("FfxNtmPECCBuildSpareArea: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            DclError();

            pEccParams->pIoStat->ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;
            return;
        }
    }
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCOffsetZeroBuildSpareArea()

    Initializes the spare area with the fixed values and the
    ECC generated from the data given using the offset zero
    format.

    Note that this function is used to build arrays of spare
    areas when emulating larger page sizes with small-block
    NAND.

    Likewise when emulating larger page sizes, we only record
    the tag value in the first one, and therefore the pTag
    pointer may be NULL if we do not want to record the tag for
    this given spare area.

    *Note* -- This function is only appropriate for use where
              parameterized software ECC is being used.

    Parameters:
        hDev       - device handle
        pNtmInfo   - pointer to NTM information
        pEccParams - pointer to structure parameterizing ECC function
                     calculated.
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.
        fUseEcc    - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FfxNtmPECCOffsetZeroBuildSpareArea(
    FFXDEVHANDLE        hDev,
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    D_UINT16            uNumSegments;

    DclAssert(pEccParams);
    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pNtmInfo->uPageSize);
    DclAssert(pNtmInfo->uPageSize % pEccParams->pEccProcessor->ulNumDataBytesEvaluated == 0);
    DclAssert(pNtmInfo->uSpareSize);

    /*  Parameter not used in this version of BuildSpareArea. However, it is
        used in others and it is desirable to keep the parameter lists the
        same between the varous versions, so we keep it and to this to stop
        the compiler warning:
    */
    (void)hDev;

    uNumSegments = pNtmInfo->uPageSize / NAND512_PAGE;
    DclAssert(pNtmInfo->uSpareSize >= uNumSegments * NAND512_SPARE);

    DclMemSet(pSpare, ERASED8, pNtmInfo->uSpareSize);

    if(pTag)
        FfxNtmPECCTagEncode(pNtmInfo, pSpare, pTag, NSF_OFFSETZERO);

    if(fUseEcc)
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        unsigned    nn = 0;

        /*  Tell the ECC processor how much storage we have for ECCs:
        */

        pEccParams->ulECCBufferByteCount = MAX_ECC_BYTES_PER_PAGE;

        /* Generate ECCs:
        */
        pEccParams->data_buffer.uCount = pNtmInfo->uPageSize;
        pEccParams->data_buffer.uOffset = 0;
        pEccParams->ecc_buffer = abECC;
        if (pEccParams->pEccProcessor->Start)
            (*pEccParams->pEccProcessor->Start)();
        FfxParameterizedEccGenerate(pEccParams);

        while(uNumSegments)
        {
            /*  Build the ECC values
            */
            pSpare[NSOFFSETZERO_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

            pSpare[NSOFFSETZERO_ECC1_OFFSET + 0] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC1_OFFSET + 1] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC1_OFFSET + 2] = abECC[nn++];

            pSpare[NSOFFSETZERO_ECC2_OFFSET + 0] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC2_OFFSET + 1] = abECC[nn++];
            pSpare[NSOFFSETZERO_ECC2_OFFSET + 2] = abECC[nn++];

            uNumSegments--;
            pSpare          += NAND512_SPARE;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCSSFDCBuildSpareArea()

    Initializes the spare area with the fixed values and the
    ECC generated from the data given using the SSFDC format.

    Note that this function is used to build arrays of spare
    areas when emulating larger page sizes with small-block
    NAND.

    Likewise when emulating larger page sizes, we only record
    the tag value in the first one, and therefore the pTag
    pointer may be NULL if we do not want to record the tag for
    this given spare area.

    *Note* -- This function is only appropriate for use where
              software ECC is being used.

    Parameters:
        hDev       - device handle
        pNtmInfo   - pointer to NTM information
        pEccParams - pointer to structure parameterizing ECC function
                     calculated.
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.
        fUseEcc    - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FfxNtmPECCSSFDCBuildSpareArea(
    FFXDEVHANDLE        hDev,
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{
    D_UINT16            uNumSegments;

    DclAssert(pEccParams);
    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pNtmInfo->uPageSize);
    DclAssert(pNtmInfo->uPageSize % pEccParams->pEccProcessor->ulNumDataBytesEvaluated == 0);
    DclAssert(pNtmInfo->uSpareSize);

    /*  "hDev" is not used in this version of BuildSpareArea. However, it is
        used in others and it is desirable to keep the parameter lists the
        same between the varous versions, so we keep it and to this to stop
        the compiler warning:
    */
    (void)hDev;

    uNumSegments = pNtmInfo->uPageSize / NAND512_PAGE;
    DclAssert(pNtmInfo->uSpareSize >= uNumSegments * NAND512_SPARE);

    DclMemSet(pSpare, ERASED8, pNtmInfo->uSpareSize);

    if(pTag)
        FfxNtmPECCTagEncode(pNtmInfo, pSpare, pTag, NSF_SSFDC);

    if(fUseEcc)
    {
        D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];
        unsigned    nn = 0;

        /*  Tell the ECC processor how much storage we have for ECCs:
        */

        pEccParams->ulECCBufferByteCount = MAX_ECC_BYTES_PER_PAGE;

        /* Generate ECCs:
        */
        pEccParams->data_buffer.uCount = pNtmInfo->uPageSize;
        pEccParams->data_buffer.uOffset = 0;
        pEccParams->ecc_buffer = abECC;
        if (pEccParams->pEccProcessor->Start)
            (*pEccParams->pEccProcessor->Start)();
        FfxParameterizedEccGenerate(pEccParams);

        while(uNumSegments)
        {
            /*  Build the ECC values
            */
            pSpare[NSSSFDC_FLAGS_OFFSET] = (D_UINT8)LEGACY_WRITTEN_WITH_ECC;

            pSpare[NSSSFDC_ECC1_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC1_OFFSET + 2] = abECC[nn++];

            pSpare[NSSSFDC_ECC2_OFFSET + 0] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 1] = abECC[nn++];
            pSpare[NSSSFDC_ECC2_OFFSET + 2] = abECC[nn++];

            uNumSegments--;
            pSpare          += NAND512_SPARE;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCDefaultMLCBuildSpareArea()

    Initializes the spare area with the fixed values and the
    ECC generated from the data given using the offset zero
    format.

    Note that this function is used to build arrays of spare
    areas when emulating larger page sizes with small-block
    NAND.

    Likewise when emulating larger page sizes, we only record
    the tag value in the first one, and therefore the pTag
    pointer may be NULL if we do not want to record the tag for
    this given spare area.

    *Note* -- This function is only appropriate for use where
              software ECC is being used.

    Parameters:
        hDev       - device handle
        pNtmInfo   - A pointer to NTM information
        pEccParams - A pointer to structure parameterizing ECC function
                     calculated.
        pSpare     - A pointer to the buffer in which to build the
                     spare data.
        pTag       - A pointer to the tag data to store in the
                     spare area.  This pointer may be NULL if the
                     tag value is to remain unset.
        fUseEcc    - A flag indicating whether ECCs should be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void FfxNtmPECCDefaultMLCBuildSpareArea(
    FFXDEVHANDLE        hDev,
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc)
{

    DclAssert(pEccParams);
    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pNtmInfo->uPageSize);
    DclAssert(pNtmInfo->uPageSize % pEccParams->pEccProcessor->ulNumDataBytesEvaluated == 0);
    DclAssert(pNtmInfo->uSpareSize);

    /*  Parameter not used in this version of BuildSpareArea. However, it is
        used in others and it is desirable to keep the parameter lists the
        same between the varous versions, so we keep it and to this to stop
        the compiler warning:
    */
    (void)hDev;

    DclMemSet(pSpare, ERASED8, pNtmInfo->uSpareSize);

    if(pTag)
        FfxNtmPECCTagEncode(pNtmInfo, pSpare, pTag, NSF_UNKNOWN);

    if(fUseEcc)
    {
        D_UINT32    ulNumSegments;
        D_UINT32    ulDataSegmentLength;
        D_UINT32    ulMetaSegmentLength;
        D_UINT32    ulSegmentsCovered = 0;
        D_UINT32    ulCrc;

        /*  Write the four flag bytes. Since the flag byte must be covered by
            ECC, this must be done before ECC generation.
        */
        pSpare[NSDEFAULTMLC_FLAGS_OFFSET] = 0;
        pSpare[NSDEFAULTMLC_FLAGS_OFFSET+1] = 0;
        pSpare[NSDEFAULTMLC_FLAGS_OFFSET+2] = 0;
        pSpare[NSDEFAULTMLC_FLAGS_OFFSET+3] = 0;

        /*  Tell the ECC processor how much storage we have for ECCs:
        */

        ulNumSegments = (pNtmInfo->uPageSize / pEccParams->pEccProcessor->ulNumDataBytesEvaluated);
        pEccParams->ulECCBufferByteCount = pEccParams->pEccProcessor->ulNumBytesForECC * ulNumSegments;
        ulDataSegmentLength = pNtmInfo->uPageSize / ulNumSegments;
        ulMetaSegmentLength = pNtmInfo->uSpareAreaUsed / ulNumSegments;

        /*  Generate a CRC32 value covering all the main page data
            This code assumes a contiguous data area. Be sure to cover the
            tags and flag bytes.
        */
        ulCrc = DclCrc32Update(0, pEccParams->data_buffer.buffer, pNtmInfo->uPageSize);
        ulCrc = DclCrc32Update(ulCrc, &pSpare[NSDEFAULTMLC_TAG_OFFSET], NSDEFAULTMLC_TAG_LENGTH);
        ulCrc = DclCrc32Update(ulCrc, &pSpare[NSDEFAULTMLC_FLAGS_OFFSET], NSDEFAULTMLC_TAG_LENGTH);
        ulCrc = DclCrc32Update(ulCrc, &pSpare[NSDEFAULTMLC_TAGCRC], NSDEFAULTMLC_TAG_LENGTH);
        pSpare[NSDEFAULTMLC_MAINCRC] = (unsigned char)(ulCrc & 0x00FF);
        pSpare[NSDEFAULTMLC_MAINCRC+1] = (unsigned char)((ulCrc >> 8) & 0x00FF);
        pSpare[NSDEFAULTMLC_MAINCRC+2] = (unsigned char)((ulCrc >> 16) & 0x00FF);
        pSpare[NSDEFAULTMLC_MAINCRC+3] = (unsigned char)((ulCrc >> 24) & 0x00FF);

        /*  Generate four segments of ECCs. Note that it is assumed that
            upon entry to this function, the data_buffer and metadata_buffer
            structures have had their "buffer" elements initialized, but the
            counts and offsets still have to be calculated. This is because
            this module has the spare area format knowledge to do those things.

            This procedure further assumes that the metadata we want to cover
            with ECCs exists at the same offset within every segment of the
            spare area. The DefaultMLC spare area format satisfies this, but
            if a different custom format is developed that does not follow
            this rule, this code will have to be adjusted.
        */
        while (ulSegmentsCovered < ulNumSegments)
        {
            pEccParams->data_buffer.uOffset = ulSegmentsCovered*ulDataSegmentLength;
            pEccParams->data_buffer.uCount = ulDataSegmentLength;
            if (pEccParams->metadata_buffer.buffer)
            {
                pEccParams->metadata_buffer.uCount = NSDEFAULTMLC_TAG_LENGTH;
                pEccParams->metadata_buffer.uOffset =
                    ulSegmentsCovered*ulMetaSegmentLength + NSDEFAULTMLC_TAG_OFFSET;
            }
            pEccParams->ecc_buffer = &pSpare[NSDEFAULTMLC_ECC1_OFFSET];
            if (pEccParams->pEccProcessor->Start)
                (*pEccParams->pEccProcessor->Start)();
            FfxParameterizedEccGenerate(pEccParams);

            /*  Second segment...
            */
            pEccParams->data_buffer.uOffset += ulDataSegmentLength;
            if (pEccParams->metadata_buffer.buffer)
                pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
            pEccParams->ecc_buffer = &pSpare[NSDEFAULTMLC_ECC2_OFFSET];
            if (pEccParams->pEccProcessor->Start)
                (*pEccParams->pEccProcessor->Start)();
            FfxParameterizedEccGenerate(pEccParams);

            /*  Third segment...
            */
            pEccParams->data_buffer.uOffset += ulDataSegmentLength;
            if (pEccParams->metadata_buffer.buffer)
                pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
            pEccParams->ecc_buffer = &pSpare[NSDEFAULTMLC_ECC3_OFFSET];
            if (pEccParams->pEccProcessor->Start)
                (*pEccParams->pEccProcessor->Start)();
            FfxParameterizedEccGenerate(pEccParams);

            /*  Fourth segment...
            */
            pEccParams->data_buffer.uOffset += ulDataSegmentLength;
            if (pEccParams->metadata_buffer.buffer)
                pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
            pEccParams->ecc_buffer = &pSpare[NSDEFAULTMLC_ECC4_OFFSET];
            if (pEccParams->pEccProcessor->Start)
                (*pEccParams->pEccProcessor->Start)();
            FfxParameterizedEccGenerate(pEccParams);

            /*  Remaining "sets" of 4 segments duplicate the first. Necessary
                for parameter settings that result in multiple sets of 4
                segments.
            */
            ulSegmentsCovered += 4;
            pSpare += ulMetaSegmentLength*4;
        }
    }
    return;
}


/*-------------------------------------------------------------------
    Local: PECCCorrectDataSegment()

    This function attempts to correct data errors in the
    specified data segment (DATA_BYTES_PER_ECC long).

    If this function does NOT encounter any uncorrectable data
    errors, the pIOStat->ulCount field will be incremented.

    If this function encounters correctable data errors, the
    PAGESTATUS_DATACORRECTED value will be OR'd into the
    supplied pIOStat->op.ulPageStatus field.

    Parameters:
        uCount     - The current segment count
        pEccParams - A pointer to structure parameterizing ECC function
                     calculated.

    Return Value:
        Returns TRUE if the segment has uncorrectable errors,
        otherwise FALSE.
-------------------------------------------------------------------*/
static D_BOOL PECCCorrectDataSegment(
    D_UINT16        uCount,
    FX_ECC_PARAMS  *pEccParams)
{
    /*  uCount parameter only used if debug level > 1. Disable associated
        compiler warning:
    */
    (void)uCount;

    FfxParameterizedEccCorrectData(pEccParams);
    if(pEccParams->pIoStat->ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
    {
        FFXPRINTF(2, ("ECC failure in the %u bytes starting at page offset %4U\n",
                      DATA_BYTES_PER_ECC, uCount * DATA_BYTES_PER_ECC));

        return TRUE;
    }
    else
    {
        if(FfxErrMgrDecodeCorrectedBits(pEccParams->pIoStat) > 0)
        {
            FFXPRINTF(2, ("ECC corrected an error in the %u bytes starting at page offset %U\n",
                          DATA_BYTES_PER_ECC, uCount * DATA_BYTES_PER_ECC));

            pEccParams->pIoStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
        }
        else
        {
            DclAssert(pEccParams->pIoStat->ffxStat == FFXSTAT_SUCCESS);
        }

        /*  Increment the count so long as we have NOT encountered an
            uncorrectable error.
        */
        pEccParams->pIoStat->ulCount++;
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPECCCorrectPage()

    This function examines an page using the OffsetZero or SSFDC
    format based on the type of chip being used, as specified by
    pChipInfo.

    It determines if the page is OK by checking the page flags
    and potentially the ECC. If a failure is found, the ECC will
    attempt to correct the problem and return a status which will
    indicate the state of the data.

    Parameters:
        pNtmInfo        - NTM Info structure (for uCustomSpareAreaFormat)
        pEccParams      - ECC parameterization structure
        uSpareSize      - The spare area size to use
        pSpare          - A pointer to spare area associated with the
                          given data sector read from the device.
        pReferenceSpare - A pointer to spare area buffer containing the
                          ECC's stored there when data was written.
        pChipInfo       - A pointer to the FFXNANDCHIP structure
                          describing the chip being used.

    Return Value:
        Marks an FFXIOSTATUS structure indicating the state of
        the operation. This structure is contained in the
        pEccParams parameter passed into the function. The function
        itself returns void.

        If ffxStat == FFXSTAT_SUCCESS, the PAGESTATUS_DATACORRECTED
        flag in the op.ulPageStatus field may be set if data errors
        were corrected.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
void FfxNtmPECCCorrectPage(
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    const D_BUFFER     *pSpare,
    const D_BUFFER     *pReferenceSpare,
    const FFXNANDCHIP  *pChipInfo)
{
    FX_ECC_PARAMS       localEccParams = *pEccParams;

    DclAssert(pChipInfo);

    if (pNtmInfo->uCustomSpareFormat)
    {
        FfxNtmPECCDefaultMLCCorrectPage(pNtmInfo, &localEccParams, pSpare, pReferenceSpare);
        return;
    }
    switch(FfxNtmHelpGetSpareAreaFormat(pChipInfo))
    {
        case NSF_OFFSETZERO:
        {
            FfxNtmPECCOffsetZeroCorrectPage(pNtmInfo, &localEccParams, pSpare, pReferenceSpare);
            return;
        }

        case NSF_SSFDC:
        {
            FfxNtmPECCSSFDCCorrectPage(pNtmInfo, &localEccParams, pSpare, pReferenceSpare);
            return;
        }

        case NSF_UNKNOWN:
        default:
        {
            FFXPRINTF(1, ("FfxNtmPECCCorrectPage: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            DclError();

            pEccParams->pIoStat->ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;
            return;
        }
    }
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCSSFDCCorrectPage()

    This function examines an SSFDC style page, checking the page
    flags and potentially the ECC to determine if the page is OK.
    If a failure is found, the ECC will attempt to correct the
    problem and return a status which will indicate the state of
    the data.

    Parameters:
        pEccParams      - ECC parameterization structure
        uNtmInfo        - A pointer to the Ntm Info.
        pSpare          - A pointer to spare area associated with the
                          given data sector read from the device.
        pReferenceSpare - A pointer to spare area buffer containing the
                          ECC's stored there when data was written.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation. This structure is contained in the
        pEccParams parameter passed into the function. The function
        itself returns void.

        If ffxStat == FFXSTAT_SUCCESS, the PAGESTATUS_DATACORRECTED
        flag in the op.ulPageStatus field may be set if data errors
        were corrected.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
static void FfxNtmPECCSSFDCCorrectPage(
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    const D_BUFFER     *pSpare,
    const D_BUFFER     *pReferenceSpare)
{
D_UINT16 uNumSegments;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCSSFDCCorrectPage() SpareSize=%U\n", pNtmInfo->uSpareSize));

    DclProfilerEnter("FfxNtmPECCSSFDCCorrectPage", 0, 0);

    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pReferenceSpare);
    DclAssert(pNtmInfo->uSpareSize);

    /*  Get the number of 512-byte segments in this page.
    */
    uNumSegments = pNtmInfo->uPageSize / NAND512_PAGE;
    DclAssert(pNtmInfo->uSpareSize >= uNumSegments * NAND512_SPARE);

    /*  Check the data block status, if it's clean, then the block is good
    */
    if(ISUNWRITTEN(pSpare[NSSSFDC_FLAGS_OFFSET]))
    {
        /*  TODO: For consistency, if we are using 2K pages, we really
            should be validating the bFlags fields for the full array of
            LEGACYNANDSPARE structures, as we do in the next clause.
        */
        pEccParams->pIoStat->ulCount = 1;
        pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
    }
    else if(ISWRITTENWITHECC(pSpare[NSSSFDC_FLAGS_OFFSET]))
    {
        D_UINT16    uCount = 0;
        D_BOOL      fUncorrectableError = FALSE;

        /*  Note that it is possible for a single-bit error to corrupt
            the status value.  Double-check it by testing whether it
            differs by a single bit.
        */
        while(uNumSegments)
        {
            /*  If dealing with multiple ECCs in a 2K page, all the
                block status values must be identical (except for the
                possible single-bit error).
            */
            if(!ISWRITTENWITHECC(pSpare[NSSSFDC_FLAGS_OFFSET]))
            {
                fUncorrectableError = TRUE;
            }
            pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSSSFDC_ECC1_OFFSET];
            pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSSSFDC_ECC1_OFFSET];

            /*  Calculate ECCs
            */
            if(PECCCorrectDataSegment(uCount, pEccParams))
                fUncorrectableError = TRUE;

            /*  Next segment. Note this procedure for SSFDC spare format only.
            */
            uCount++;
            pEccParams->data_buffer.uOffset += DATA_BYTES_PER_ECC;

            pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSSSFDC_ECC2_OFFSET];
            pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSSSFDC_ECC2_OFFSET];
            if(PECCCorrectDataSegment(uCount, pEccParams))
                fUncorrectableError = TRUE;

            uCount++;
            pEccParams->data_buffer.buffer += DATA_BYTES_PER_ECC;

            /*  Even if we found errors, attempt to correct errors in
                any subsequent areas.
            */
            uNumSegments--;
            pSpare      += NAND512_SPARE;
            pReferenceSpare += NAND512_SPARE;
        }
        if(fUncorrectableError)
            pEccParams->pIoStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
        else if (pEccParams->pIoStat->op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            pEccParams->pIoStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            FfxErrMgrEncodeCorrectionLocale(FFXCORRECTIONLOC_CANTTELL,
                                            pEccParams->pIoStat);
        }
        else
            pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;


      #if D_DEBUG > 1
        if(fUncorrectableError)
        {
            DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, uCount * DATA_BYTES_PER_ECC,
                       pEccParams->data_buffer.buffer - (uCount * DATA_BYTES_PER_ECC));
            DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, uCount * 8,
                       pSpare - ((uCount / 2) * NAND512_SPARE));
        }
      #endif
    }
    else
    {
        /*  We should never be checking for ECC on a page that is written
            without ECC.  BBM writes pages without ECC.  This condition can
            happen for other pages if more than a single bit error occurs in
            the flags field.
        */
        FFXPRINTF(2, ("Attempting to correct a page not written with ECC\n"));
    }

/* IsValidPageCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmPECCSSFDCCorrectPage() returning %s\n", FfxDecodeIOStatus(pEccParams->pIoStat)));
    return;
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCOffsetZeroCorrectPage()

    This function examines an "Offset Zero" style page, checking
    the page flags and potentially the ECC to determine if the
    page is OK.  If a failure is found, the ECC will attempt to
    correct the problem and return a status which will indicate
    the state of the data.

    Parameters:
        pEccParams      - ECC parameterization structure
        uNtmInfo        - A pointer to the Ntm Info.
        pSpare          - A pointer to spare area associated with the
                          given data sector read from the device.
        pReferenceSpare - A pointer to spare area buffer containing the
                          ECC's stored there when data was written.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation. This structure is contained in the
        pEccParams parameter passed into the function. The function
        itself returns void.

        If ffxStat == FFXSTAT_SUCCESS, data was read successfully

        If ffxStat == FFXSTAT_FIMCORRECTABLEDATA, data was read
        successfully, but bit correction took place.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
static void FfxNtmPECCOffsetZeroCorrectPage(
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    const D_BUFFER     *pSpare,
    const D_BUFFER     *pReferenceSpare)
{
D_UINT16    uNumSegments;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCSSFDCCorrectPage() SpareSize=%U\n", pNtmInfo->uSpareSize));

    DclProfilerEnter("FfxNtmPECCSSFDCCorrectPage", 0, 0);

    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pReferenceSpare);
    DclAssert(pNtmInfo->uSpareSize);

    /*  Get the number of 512-byte segments in this page.
    */
    uNumSegments = pNtmInfo->uPageSize / NAND512_PAGE;
    DclAssert(pNtmInfo->uSpareSize >= uNumSegments * NAND512_SPARE);

    /*  Check the data block status, if it's clean, then the block is good
    */
    if(ISUNWRITTEN(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        /*  TODO: For consistency, if we are using 2K pages, we really
            should be validating the bFlags fields for the full array of
            LEGACYNANDSPARE structures, as we do in the next clause.
        */
        pEccParams->pIoStat->ulCount = 1;
        pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
    }
    else if(ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        D_UINT16    uCount = 0;
        D_BOOL      fUncorrectableError = FALSE;

        /*  Note that it is possible for a single-bit error to corrupt
            the status value.  Double-check it by testing whether it
            differs by a single bit.
        */
        while(uNumSegments)
        {
            /*  If dealing with multiple ECCs in a 2K page, all the
                block status values must be identical (except for the
                possible single-bit error).
            */
            if(!ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
            {
                fUncorrectableError = TRUE;
            }

            pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSOFFSETZERO_ECC1_OFFSET];
            pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSOFFSETZERO_ECC1_OFFSET];

            /*  Calculate ECCs
            */
            if(PECCCorrectDataSegment(uCount, pEccParams))
                fUncorrectableError = TRUE;

            uCount++;
            pEccParams->data_buffer.uOffset += DATA_BYTES_PER_ECC;

            pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSOFFSETZERO_ECC2_OFFSET];
            pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSOFFSETZERO_ECC2_OFFSET];
            if(PECCCorrectDataSegment(uCount, pEccParams))
                fUncorrectableError = TRUE;

            uCount++;
            pEccParams->data_buffer.uOffset += DATA_BYTES_PER_ECC;

            /*  Even if we found errors, attempt to correct errors in
                any subsequent areas.
            */
            uNumSegments--;
            pSpare      += NAND512_SPARE;
            pReferenceSpare      += NAND512_SPARE;
        }
        if(fUncorrectableError)
            pEccParams->pIoStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
        else if (pEccParams->pIoStat->op.ulPageStatus & PAGESTATUS_DATACORRECTED)
        {
            pEccParams->pIoStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            FfxErrMgrEncodeCorrectionLocale(FFXCORRECTIONLOC_CANTTELL,
                                            pEccParams->pIoStat);
        }
        else
            pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;

      #if D_DEBUG > 1
        if(fUncorrectableError)
        {
            DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, uCount * DATA_BYTES_PER_ECC,
                       pEccParams->data_buffer.buffer - (uCount * DATA_BYTES_PER_ECC));
            DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, uCount * 8,
                       pSpare - ((uCount / 2) * NAND512_SPARE));
        }
      #endif
    }
    else
    {
        /*  We should never be checking for ECC on a page that is written
            without ECC.  BBM writes pages without ECC.  This condition can
            happen for other pages if more than a single bit error occurs in
            the flags field.
        */
        FFXPRINTF(2, ("Attempting to correct a page not written with ECC\n"));
    }

/* IsValidPageCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmPECCOffsetZeroCorrectPage() returning %s\n", FfxDecodeIOStatus(pEccParams->pIoStat)));
    return;
}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCDefaultMLCCorrectPage()

    This function examines an "Offset Zero" style page, checking
    the page flags and potentially the ECC to determine if the
    page is OK.  If a failure is found, the ECC will attempt to
    correct the problem and return a status which will indicate
    the state of the data.

    Parameters:
        pNtmInfo        - A pointer to the Ntm Info. Contains data needed
                          for custom spare area formatting.
        pEccParams      - A pointer to ECC parameters.
        pSpare          - A pointer to spare area associated with the
                          given data sector.
        pReferenceSpare - A pointer to spare area buffer containing the
                          ECC's stored there when data was written.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation. This structure is contained in the
        pEccParams parameter passed into the function. The function
        itself returns void.

        If ffxStat == FFXSTAT_SUCCESS, data was read successfully

        If ffxStat == FFXSTAT_FIMCORRECTABLEDATA, data was read
        successfully, but bit correction took place.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
static void FfxNtmPECCDefaultMLCCorrectPage(
    NTMINFO            *pNtmInfo,
    FX_ECC_PARAMS      *pEccParams,
    const D_BUFFER     *pSpare,
    const D_BUFFER     *pReferenceSpare)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCDefaultMLCCorrectPage() SpareSize=%U\n", pNtmInfo->uSpareSize));

    DclProfilerEnter("FfxNtmPECCDefaultMLCCorrectPage", 0, 0);

    DclAssert(pEccParams->data_buffer.buffer);
    DclAssert(pSpare);
    DclAssert(pReferenceSpare);
    DclAssert(pNtmInfo->uSpareSize);

    /*  Check the data block status, if it's clean, then the block is good

        REFACTOR: Is this a wise way to do this? MLC devices may have more
        than one bit error in the flag, and the flag should be covered by
        ECC. We may want to evaluate the implications of this for MLC
        devices. We may want to consider a new macro and a new page status
        field width that takes possible multi-bit errors into account.
    */
    if(ISUNWRITTEN(pReferenceSpare[NSDEFAULTMLC_FLAGS_OFFSET]))
    {
        pEccParams->pIoStat->ulCount = 1;
        pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
    }

    /*  Note that we don't use the "ISWRITTENWITHECC" macro for the
        parameterized ECC version of this. This is because with the
        possibillity of multi-bit errors, it is possible for this macro
        to mistakenly think the page has been written without ECC or
        even unwritten when in fact it has been.

        REFACTOR: we need to introduce a mechanism where it is
        deterministic whether a page has been written. This will
        probably involve a larger Page Status field to render it
        insensitive to multi-bit errors. Certainly using macros that
        look for a pattern to only be "within one bit" are useless
        for devices that may experience multi-bit errors.
    */
    else
    {
        D_UINT16    uCount = 0;
        D_BOOL      fUncorrectableError = FALSE;
        D_UINT32    ulNumSegments;
        D_UINT32    ulDataSegmentLength;
        D_UINT32    ulMetaSegmentLength;
        D_UINT32    ulSegmentsCovered = 0;
        D_UINT32    ulCrc, ulStoredCrc;

        /*  Compute main page CRC and compare with stored value.
            If this doesn't match, go through full ECC checking
            This code assumes a contiguous main page area.
        */
        ulCrc = DclCrc32Update(0, pEccParams->data_buffer.buffer, pNtmInfo->uPageSize);
        ulCrc = DclCrc32Update(ulCrc, &pReferenceSpare[NSDEFAULTMLC_TAG_OFFSET], NSDEFAULTMLC_TAG_LENGTH);
        ulCrc = DclCrc32Update(ulCrc, &pReferenceSpare[NSDEFAULTMLC_FLAGS_OFFSET], NSDEFAULTMLC_TAG_LENGTH);
        ulCrc = DclCrc32Update(ulCrc, &pReferenceSpare[NSDEFAULTMLC_TAGCRC], NSDEFAULTMLC_TAG_LENGTH);
        ulStoredCrc = (pReferenceSpare[NSDEFAULTMLC_MAINCRC+3] << 24)
                + (pReferenceSpare[NSDEFAULTMLC_MAINCRC+2] << 16)
                + (pReferenceSpare[NSDEFAULTMLC_MAINCRC+1] << 8)
                + (pReferenceSpare[NSDEFAULTMLC_MAINCRC]);

        if( ulCrc != ulStoredCrc )
        {
            ulNumSegments = (pNtmInfo->uPageSize / pEccParams->pEccProcessor->ulNumDataBytesEvaluated);
            pEccParams->ulECCBufferByteCount = pEccParams->pEccProcessor->ulNumBytesForECC * ulNumSegments;
            ulDataSegmentLength = pNtmInfo->uPageSize / ulNumSegments;
            ulMetaSegmentLength = pNtmInfo->uSpareAreaUsed / ulNumSegments;

            /*  Duplicate sets of four segments.
            */
            while (ulSegmentsCovered < ulNumSegments)
            {
                /*  First segment...
                */
                pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSDEFAULTMLC_ECC1_OFFSET];
                pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSDEFAULTMLC_ECC1_OFFSET];
                pEccParams->data_buffer.uCount = ulDataSegmentLength;
                pEccParams->data_buffer.uOffset = ulSegmentsCovered*ulDataSegmentLength;
                if (pEccParams->metadata_buffer.buffer)
                {
                    pEccParams->metadata_buffer.uCount = NSDEFAULTMLC_TAG_LENGTH;
                    pEccParams->metadata_buffer.uOffset =
                        ulSegmentsCovered*ulMetaSegmentLength + NSDEFAULTMLC_TAG_OFFSET;
                }
                if(PECCCorrectDataSegment(uCount, pEccParams))
                    fUncorrectableError = TRUE;
                uCount++;

                /*  Second segment...
                */
                pEccParams->data_buffer.uOffset += pEccParams->pEccProcessor->ulNumDataBytesEvaluated;
                if (pEccParams->metadata_buffer.buffer)
                {
                    pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
                }
                pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSDEFAULTMLC_ECC2_OFFSET];
                pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSDEFAULTMLC_ECC2_OFFSET];
                if(PECCCorrectDataSegment(uCount, pEccParams))
                    fUncorrectableError = TRUE;
                uCount++;

                /*  Third segment...
                */
                pEccParams->data_buffer.uOffset += pEccParams->pEccProcessor->ulNumDataBytesEvaluated;
                if (pEccParams->metadata_buffer.buffer)
                {
                    pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
                }
                pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSDEFAULTMLC_ECC3_OFFSET];
                pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSDEFAULTMLC_ECC3_OFFSET];
                if(PECCCorrectDataSegment(uCount, pEccParams))
                    fUncorrectableError = TRUE;
                uCount++;

                /*  Fourth segment...
                */
                pEccParams->data_buffer.uOffset += pEccParams->pEccProcessor->ulNumDataBytesEvaluated;
                if (pEccParams->metadata_buffer.buffer)
                {
                    pEccParams->metadata_buffer.uOffset += ulMetaSegmentLength;
                }
                pEccParams->ecc_buffer = (D_BUFFER *)&pSpare[NSDEFAULTMLC_ECC4_OFFSET];
                pEccParams->reference_ecc_buffer = (D_BUFFER *)&pReferenceSpare[NSDEFAULTMLC_ECC4_OFFSET];
                if(PECCCorrectDataSegment(uCount, pEccParams))
                    fUncorrectableError = TRUE;
                uCount++;

                /*  Remaining "sets" of 4 segments duplicate the first. Necessary
                    for parameter settings that result in multiple sets of 4
                    segments.
                */
                ulSegmentsCovered += 4;
                pReferenceSpare += ulMetaSegmentLength*4;
            }

            /*  Check results
            */
            if(fUncorrectableError)
                pEccParams->pIoStat->ffxStat = FFXSTAT_FIMUNCORRECTABLEDATA;
            else if (pEccParams->pIoStat->op.ulPageStatus & PAGESTATUS_DATACORRECTED)
            {
                pEccParams->pIoStat->ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
                FfxErrMgrEncodeCorrectionLocale(FFXCORRECTIONLOC_CANTTELL,
                                                pEccParams->pIoStat);
            }
            else
                pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;

#if D_DEBUG > 1
            if(fUncorrectableError)
            {
                DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, uCount * DATA_BYTES_PER_ECC,
                           pEccParams->data_buffer.buffer - (uCount * DATA_BYTES_PER_ECC));
                DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, uCount * 8,
                           pSpare - ((uCount / 2) * NAND512_SPARE));
            }
#endif
        }
        else
            pEccParams->pIoStat->ffxStat = FFXSTAT_SUCCESS;
    }

/* IsValidPageCleanup: */

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmPECCDefaultMLCCorrectPage() returning %s\n", FfxDecodeIOStatus(pEccParams->pIoStat)));
    return;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPECCGetPageStatus()

    Gets the page status for the requested page.

    Parameters:
        hNTM             - The NTM handle
        pNtmInfo         - A pointer to the Ntm Info. Contains data
                           regardinc custom spare area format.
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use.
        ulPage           - The page from which to return status.
        pChipInfo        - A pointer to the FFXNANDCHIP structure
                           describing the chip being used.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        op.ulBlockStatus field will contain the block status
        bits pertaining to bad blocks.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmPECCGetPageStatus(
    NTMHANDLE           hNTM,
    NTMINFO            *pNtmInfo,
    PFNREADSPAREAREA    pfnReadSpareArea,
    D_UINT32            ulPage,
    const FFXNANDCHIP  *pChipInfo)
{
    DclAssert(pChipInfo);

    if (pNtmInfo->uCustomSpareFormat)
    {
        return FfxNtmPECCDefaultMLCGetPageStatus(hNTM, pNtmInfo, pfnReadSpareArea, ulPage);
    }
    else
    {
        return FfxNtmHelpReadPageStatus(hNTM, pfnReadSpareArea, ulPage, pChipInfo);
    }

}


/*-------------------------------------------------------------------
    Local: FfxNtmPECCDefaultMLCGetPageStatus()

    Gets the page status for the requested page on a device
    that supports the DefaultMLC spare area format.

    Parameters:
        hNTM             - The NTM handle
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use.
        pChipClass       - A pointer to the FFXNANDCHIPCLASS
                           describing the chip being used.
        ulPage           - The page from which to return status.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        op.ulBlockStatus field will contain the block status
        bits pertaining to bad blocks.
-------------------------------------------------------------------*/
static FFXIOSTATUS FfxNtmPECCDefaultMLCGetPageStatus(
    NTMHANDLE           hNTM,
    NTMINFO            *pNtmInfo,
    PFNREADSPAREAREA    pfnReadSpareArea,
    D_UINT32            ulPage)
{
    FFXIOSTATUS         ioStat;
    D_BUFFER           *buffer;
    D_UINT32            ulFlags;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCDefaultMLCGetPageStatus() Page=%lX\n", ulPage));

    DclAssert(hNTM);
    DclAssert(pNtmInfo);
    DclAssert(pNtmInfo->pScratchSpareBuffer);
    DclAssert(pfnReadSpareArea);

    /*  Read the spare area
    */
    buffer = pNtmInfo->pScratchSpareBuffer;
    ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer);
    if(IOSUCCESS(ioStat, 1))
    {
        /*  If the tag area, including the ECC and check bytes, is within
            1 bit of being erased, then we know that a tag was not written.
            If it is anything else, we know a tag was written.  This function
            always uses standard size tags.

            REFACTOR: Use of "within one bit" macros is probably not
            advisable for MLC devices, as they can have multiple bit
            errors within the spare area. This should be addressed.
        */
        if(!FfxNtmHelpIsRangeErased1Bit(&(buffer[NSDEFAULTMLC_TAG_OFFSET]), NSDEFAULTMLC_TAG_LENGTH))
            ioStat.op.ulPageStatus |= PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);

        ulFlags = (buffer[NSDEFAULTMLC_FLAGS_OFFSET+3] << 24)
                + (buffer[NSDEFAULTMLC_FLAGS_OFFSET+2] << 16)
                + (buffer[NSDEFAULTMLC_FLAGS_OFFSET+1] << 8)
                + (buffer[NSDEFAULTMLC_FLAGS_OFFSET]);

        if(IsWithinRange(ulFlags, MLC_WRITTEN_WITH_ECC, pNtmInfo->uEdcRequirement))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
        }
        else if(IsWithinRange(ulFlags, MLC_UNWRITTEN, pNtmInfo->uEdcRequirement))
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_UNWRITTEN;
        }
        else
        {
            ioStat.op.ulPageStatus |= PAGESTATUS_UNKNOWN;
            FFXPRINTF(1, ("Warning: Unknown page status, page = %lX\n", ulPage));
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmPECCDefaultMLCGetPageStatus() Page=%lX returning %s\n",
        ulPage, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}

/*  REFACTOR: For now we assume that the standard Bad Block handling
    stuff is good enough. Assuming a custom spare area format to follow
    OffsetZero conventions will not always be valid, but in the absence of
    generalized custom spare area format processing, we don't have much
    choice. The convention is good enough for DefaultMLC, but this will
    eventually need attention, especially when it comes time to implement
    FBB detection for ONFI-compliant devices.
*/


/*-------------------------------------------------------------------
    Public: FfxNtmPECCIsBadBlock()

    This function determines if the given block is bad, according
    to the bad-block marking style for the given chip class.

    Parameters:
        hNTM             - The NTM handle
        pNtmInfo         - Pointer to the NTMINFO structure
        pfnReadSpareArea - A pointer to the ReadSpareArea function
                           to use
        pChipInfo        - A pointer to the FFXNANDCHIP structure
                           describing the chip being used.
        ulBlock          - The block to mark.  Must have been
                           adjusted for reserved space (if any).

    Return Value:
        Returns an FFXIOSTATUS structure describing the state of
        the operation.  If ffxStat is FFXSTAT_SUCCESS, the
        misc.ulBlockStatus field will contain the block status value.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmPECCIsBadBlock(
    NTMHANDLE                   hNTM,
    NTMINFO                    *pNtmInfo,
    PFNREADSPAREAREA            pfnReadSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock)
{
    FFXIOSTATUS                 ioStat = DEFAULT_BLOCKIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCIsBadBlock() Block=%lX\n", ulBlock));

    if (!pNtmInfo->uCustomSpareFormat)
    {
        return FfxNtmHelpIsBadBlock(hNTM, pfnReadSpareArea, pChipInfo, ulBlock);
    }
    else
    {
        D_UINT32                    ulPagesRemaining;
        D_UINT32                    ulPage;
        D_UINT32                    ulPagesPerBlock;
        D_BOOL                      fCheckLastPage;

        DclAssert(hNTM);
        DclAssert(pNtmInfo);
        DclAssert(pfnReadSpareArea);
        DclAssert(pChipInfo);

        /* Compute the number of pages per block once, before the loop
        */
        DclAssert((pChipInfo->pChipClass->ulBlockSize % pChipInfo->pChipClass->uPageSize) == 0);
        ulPagesPerBlock = pChipInfo->pChipClass->ulBlockSize / pChipInfo->pChipClass->uPageSize;

        /*  If there is a factory bad block mark, it will be in one of the
            first two pages, or possibly the last page if the device is so
            specified.
        */
        ulPagesRemaining = 2;
        ulPage = ulBlock * ulPagesPerBlock;
        fCheckLastPage = pChipInfo->pChipClass->fLastPageFBB;
        while(ulPagesRemaining)
        {
        D_BUFFER *spare = pNtmInfo->pScratchSpareBuffer;

            DclAssert(spare);

            /*  Read and scan the next common region of the flash.
            */
            ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)spare);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            {
                /*  If we failed to read the spare area, zero out the
                    ulBlockStatus field, as it could have PageStatus
                    information in there (since we just did a page
                    oriented operation).
                */
                ioStat.op.ulBlockStatus = 0;
                goto IsOffsetZeroBadBlockCleanup;
            }
            if(spare[NSDEFAULTMLC_FACTORYBAD_OFFSET] != ERASED8)
            {
                ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                goto IsOffsetZeroBadBlockCleanup;
            }
            else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
            {
                /*  For 16-bit interfaces, one of the first two bytes
                    will be marked.
                */
                if(spare[NSDEFAULTMLC_FACTORYBAD_OFFSET+1] != ERASED8)
                {
                    ioStat.op.ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                    goto IsOffsetZeroBadBlockCleanup;
                }
            }

            ulPage++;
            ulPagesRemaining--;
            if (!ulPagesRemaining && fCheckLastPage)
            {
                ulPagesRemaining = 1;
                fCheckLastPage = FALSE;

                /*  Last page in a block = start page + (number of pages -1):
                */
                ulPage = ulBlock * ulPagesPerBlock + ulPagesPerBlock - 1;
            }
        }

        ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;

      IsOffsetZeroBadBlockCleanup:

        /*  We were doing page operations, so change the type back to block...
        */
        ioStat.ulFlags = IOFLAGS_BLOCK;

    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmIsPECCBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmPECCSetBlockType()

    Attempts to mark the given block bad in a fashion that is
    compatible and distinct from the factory bad block method,
    according to the style of chip specified by pChipInfo.

    Parameters:
        hNTM              - The NTM handle
        pNtmInfo          - Pointer to the NTMINFO structure
        pfnReadSpareArea  - A pointer to the ReadSpareArea function
                            to use
        pfnWriteSpareArea - A pointer to the WriteSpareArea function
                            to use
        pChipInfo         - A pointer to the FFXNANDCHIP structure
                            describing the chip being used.
        ulBlock           - The block to mark.  Must have been
                            adjusted for reserved space (if any).
        ulBlockStatus     - The block status value.

    Return Value:
        Returns an FFXIOSTATUS structure describing the state of
        the operation.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmPECCSetBlockType(
    NTMHANDLE                   hNTM,
    NTMINFO                    *pNtmInfo,
    PFNREADSPAREAREA            pfnReadSpareArea,
    PFNWRITESPAREAREA           pfnWriteSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock,
    D_UINT32                    ulBlockStatus)
{
    FFXIOSTATUS                 ioStat = DEFAULT_BLOCKIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmPECCSetBlockType() Block=%lX Status=%lX\n", ulBlock, ulBlockStatus));

    DclAssert(hNTM);
    DclAssert(pNtmInfo);
    DclAssert(pfnWriteSpareArea);
    DclAssert(pChipInfo);

    /*  Call traditional SetBlockType for traditional spare area formats.
    */
    if (!pNtmInfo->uCustomSpareFormat)
    {
        return FfxNtmHelpSetBlockType(hNTM,
                                 pfnReadSpareArea,
                                 pfnWriteSpareArea,
                                 pChipInfo,
                                 ulBlock,
                                 ulBlockStatus);
    }
    else
    {
        D_UINT32 ulPagesRemaining;
        D_UINT32 ulPage;
        D_UINT32 ulPagesPerBlock;
        D_BOOL   fMarked = FALSE;
        D_BOOL   fMarkLastPage;

        DclAssert(hNTM);
        DclAssert(pfnReadSpareArea);
        DclAssert(pfnWriteSpareArea);
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
        D_BUFFER *spare = pNtmInfo->pScratchSpareBuffer;

            DclAssert(spare);

            /*  Mark block, and do so only if no bad block mark already
                exists.
            */
            ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)spare);
            if(IOSUCCESS(ioStat, 1))
            {
                if(ulBlockStatus == BLOCKSTATUS_FACTORYBAD)
                {
                    if(spare[NSDEFAULTMLC_FACTORYBAD_OFFSET] != ERASED8)
                    {
                        fMarked = TRUE;
                        goto CustomSpareFormatSetBlockTypeCleanup;
                    }
                    else if((pChipInfo->bFlags & CHIPINT_MASK) == CHIPINT_16BIT)
                    {
                        /*  For 16-bit interfaces, one of the first two bytes
                            will be marked.
                        */
                        if(spare[NSDEFAULTMLC_FACTORYBAD_OFFSET+1] != ERASED8)
                        {
                            fMarked = TRUE;
                            goto CustomSpareFormatSetBlockTypeCleanup;
                        }
                    }

                    spare[NSDEFAULTMLC_FACTORYBAD_OFFSET+0] = 0;
                    spare[NSDEFAULTMLC_FACTORYBAD_OFFSET+1] = 0;

                    ioStat = (*pfnWriteSpareArea)(hNTM, ulPage, (void *)spare);
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

      CustomSpareFormatSetBlockTypeCleanup:

        /*  If at least one page was marked, consider it good.
        */
        if(fMarked)
            ioStat.ffxStat = FFXSTAT_SUCCESS;
        else
            ioStat.ffxStat = FFXSTAT_FIMIOERROR;

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
            "FfxNtmPECCSetBlockType() Block=%lX returning %s\n",
            ulBlock, FfxDecodeIOStatus(&ioStat)));

        return ioStat;
    }
}


/*-------------------------------------------------------------------
    Local: IsWithinRange()

    A utility function to see if the Hamming distance between
    two D_UINT32s is within a certain level.

    Parameters:
        ulVal      - The value to be tested.
        pReference - The reference value against which "ulVal
                     is to be tested.
        ulLevel    - The Hamming distance above which this
                     function returns FALSE.

    Return Value:
        Returns FALSE if the Hamming distance betwee ulVal and
        ulReference is above ulLevel; otherwise returns TRUE.
-------------------------------------------------------------------*/
static D_BOOL IsWithinRange(
    D_UINT32 ulVal,
    D_UINT32 ulReference,
    D_UINT32 ulLevel)
{
    D_UINT32 ulBitDifference;
    D_UINT32 ulIndex;
    D_UINT32 ulCount = 0;

    ulBitDifference = ulVal ^ ulReference;
    for (ulIndex=0; ulIndex < sizeof(D_UINT32)*8; ulIndex++)
    {
        if (ulBitDifference & 0x1)
            ulCount++;
        ulBitDifference >>= 0x1;
    }
    if (ulCount <= ulLevel)
        return TRUE;
    return FALSE;
}

#endif  /* FFXCONF_NANDSUPPORT */


