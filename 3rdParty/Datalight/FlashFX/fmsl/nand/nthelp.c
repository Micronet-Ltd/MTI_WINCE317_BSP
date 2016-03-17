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

    This module contains helper functions which are used by multiple NTMs.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nthelp.c $
    Revision 1.55  2010/07/06 01:24:10Z  garyp
    Minor debug code / datatype updates -- no functional changes.
    Revision 1.54  2009/12/11 23:13:51Z  garyp
    Updated to use some functions which were renamed to avoid naming
    conflicts.  Use the more flexible NTMHOOKHANDLE rather than the old
    PNANDCTL.  Refactored FfxNtmHelpGetPageStatus() into two functions
    to allow easier use by hooks code.
    Revision 1.53  2009/10/06 21:21:54Z  garyp
    Modified to use the revamped single-bit ECC calculation and
    correction functions.  Updated the "BuildSpareArea" functions to
    take a NANDCTL parameter rather than a Device handle.  Fixed
    documentation errors.
    Revision 1.52  2009/07/28 02:05:28Z  garyp
    Merged from the v4.0 branch.  Moved the SSFDC and OffsetZero specific
    functions into the modules nthelpssfdc.c and nthelpoffsetzero.c, 
    respectively, and conditioned the code such that support for SSFDC,
    OffsetZero, or "legacy" Toshiba flash can be conditioned out at compile
    time. Implemented FfxNtmSetBlockType() for SSFDC style flash.
    Revision 1.51  2009/04/18 00:23:39Z  garyp
    Cleaned up the documentation -- no functional changes.
    Revision 1.50  2009/04/09 03:42:56Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.49  2009/04/02 14:32:35Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.48  2009/03/27 06:11:21Z  glenns
    Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.47  2009/03/23 19:25:54Z  keithg
    Fixed bug 2509, Factored the tag functionality into module nthelptags.c
    Revision 1.46  2009/03/18 16:53:09Z  glenns
    Fix Bugzilla #2370: Removed assignment of obsolete block status value.
    Revision 1.45  2009/02/25 20:25:35Z  glenns
    Eliminated never-used return value for FfxNtmTagEncode definition.
    Revision 1.44  2009/02/18 18:37:22Z  glenns
    Added code to prevent compiler warnings about unused formal parameters.
    Revision 1.43  2009/02/06 22:14:01Z  glenns
    Fixed erroneous copy-and-paste error.
    Revision 1.42  2009/01/28 23:55:35Z  glenns
    Modified FfxNtmOffsetZeroIsBadBlock function to examine the last page of
    the block if the corresponding flag is set in the ChipInfo structure. 
    Refer to corresponding changes in nandid.
    Revision 1.41  2009/01/20 18:12:22Z  glenns
    Fixed up code to properly return FFXSTAT_FIMCORRECTABLEDATA when detected
    in FFXIOSTATUS when correcting a page
    Revision 1.40  2009/01/18 08:52:44Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.39  2009/01/16 05:36:15Z  glenns
    Fixed up literal FFXIOSTATUS initialization in five places.  Added error
    manager support to report correctable errors.
    Revision 1.38  2008/12/22 04:52:31Z  keithg
    Moved unknown page status debug notice to level 2.
    Revision 1.37  2008/12/18 06:01:55Z  keithg
    Changed GetPageStatus interpretation of a page to return an unknown
    status rather than issuing an assert.
    Revision 1.36  2008/12/12 07:48:12Z  keithg
    Updated to conditionally use the deprecated xxx_BBMBLOCK type.
    Revision 1.35  2008/06/16 13:22:45Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.34  2008/05/03 21:56:06Z  garyp
    Reverted the check in FfxNtmTagDecode() to behave as it did originally,
    in that a tag area which is erased, but has a single bit error, will be
    reported as a bogus tag, and the allocator will eventually cause the page
    to be recycled.
    Revision 1.33  2008/03/23 18:59:09Z  Garyp
    Modified FfxNtmTagDecode() to report a tag area which is within a single
    bit of being erased, as being erased.  Modified it as well to fill the
    tag buffer with NULLs should the tag be invalid (but not erased),
    eliminating the need for the various callers to do that.  Updated the
    various GetPageStatus() functions to return the tag size.
    Revision 1.32  2008/02/03 05:30:21Z  keithg
    comment updates to support autodoc
    Revision 1.31  2008/01/30 21:50:51Z  Glenns
    Adjusted procedure "FfxNtmOffsetZeroIsBadBlock" to account for devices with
    page size larger than 2KB.
    Revision 1.30  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.29  2007/10/18 17:15:50Z  pauli
    Bumped up the debug level for some ECC related messages.
    Removed a couple of bogus assertions.
    Revision 1.28  2007/10/14 21:12:25Z  Garyp
    Modified to use a renamed function.
    Revision 1.27  2007/10/11 00:10:55Z  billr
    Correct debug output.
    Revision 1.26  2007/10/05 19:24:39Z  pauli
    Disabled test code.
    Revision 1.25  2007/09/21 04:35:46Z  pauli
    Resolved Bug 1455.
    Revision 1.24  2007/09/12 18:35:35Z  Garyp
    General cleanup.  Fixed several of the "IsBadBlock" functions to ensure
    that the proper block status value is returned.  Renamed several SSFDC
    specific functions to make that obvious.  Updated to support 4KB pages.
    Revision 1.23  2007/08/01 15:17:33Z  timothyj
    Added minor assertions that flash block size is evenly divisible by
    the page size.
    Revision 1.22  2007/07/23 22:27:36Z  Garyp
    Corrected a misplaced assert.
    Revision 1.21  2007/06/14 21:55:28Z  Garyp
    Minor updates to debugging code.
    Revision 1.20  2007/06/08 01:21:53Z  pauli
    Resolve Bug 1144.
    Revision 1.19  2007/04/02 17:56:05Z  rickc
    Fix for bug 928.  Removed requirement that structure had to  match on-media
    spare area format.
    Revision 1.18  2007/03/06 18:38:46Z  billr
    Dump failing page on uncorrectable ECC error in debug build.
    Revision 1.17  2007/02/05 20:00:26Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.16  2006/10/06 00:19:00Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.15  2006/03/20 20:35:28Z  Garyp
    Documentation updates.
    Revision 1.14  2006/03/18 01:09:30Z  Garyp
    Documentation update.
    Revision 1.13  2006/03/16 07:38:19Z  Garyp
    Fixed FfxNtmTagDecode() so it returns successfully with all 0xFFs in the
    destination buffer if the source tag data is entirely in the erased state.
    Revision 1.12  2006/03/14 10:20:36Z  garyp
    Modified to support a new spare format for "OffsetZero" style flash which
    does not conflict with the factory bad block mark.
    Revision 1.11  2006/03/10 01:18:34Z  Garyp
    Added the ability to mark blocks as used by BBM, and renamed the NTM
    helper functions from "MarkBlockBad" to "SetBlockType".
    Revision 1.10  2006/03/03 20:14:02Z  Garyp
    Updated the spare area handlers to use the FFXNANDCHIP flags field to
    determine the proper behavior in reading and writing the spare area.
    Revision 1.9  2006/02/27 01:11:19Z  Garyp
    Added FfxNtmTagEncode/Decode().  Added FfxNtmGetPageStatus().
    Revision 1.8  2006/02/24 04:30:08Z  Garyp
    Updated to use refactored headers.
    Revision 1.7  2006/02/23 20:41:49Z  Garyp
    Modified to use helper functions in the NTMs for SpareRead/Write() rather
    than the regular NTM entry points, to properly account for the reserved
    space adjustment.
    Revision 1.6  2006/02/17 23:28:34Z  Garyp
    Modified the ECC hook functions to take an hDev parameter.
    Revision 1.5  2006/02/09 22:26:14Z  Garyp
    Updated to no longer use the EXTMEDIAINFO structure.
    Revision 1.4  2006/01/25 04:00:43Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.3  2006/01/11 02:26:25Z  Garyp
    Documentation changes only.
    Revision 1.2  2005/12/14 20:56:39Z  billr
    Assertion could fail spuriously if media format changed. Replace with
    an appropriate run-time test.
    Revision 1.1  2005/12/03 02:58:56Z  Pauli
    Initial revision
    Revision 1.3  2005/12/03 02:58:56Z  Garyp
    Added FfxNtmIsBadBlock() and FfxNtmMarkBlockBad(), and laid the
    foundation for supporting multiple spare area formats (work in progress).
    Revision 1.2  2005/11/13 17:45:12Z  Garyp
    Debug code updated.
    Revision 1.1  2005/10/31 00:11:24Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fimdev.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <errmanager.h>			
#include <fxnandapi.h>
#include "nandid.h"
#include "nand.h"


/*-------------------------------------------------------------------
    Public: FfxNtmHelpBuildSpareArea()

    Initializes the spare area using the OffsetZero or SSFDC
    format based on the type of chip being used, as specified
    by pChipInfo.

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
        hNtmHook    - The NTM Hook handle to use.  This value may be
                      NULL for some NTMs.
        pNtmInfo    - A pointer to the NTMINFO structure to use.
        pData       - A pointer to the data for which ECCs are to be
                      calculated.
        pSpare      - A pointer to the buffer in which to build the
                      spare data.
        pTag        - A pointer to the tag data to store in the
                      spare area.  This pointer may be NULL if the
                      tag value is to remain unset.
        fUseEcc     - A flag indicating whether ECCs should be used.
        pChipInfo   - A pointer to the FFXNANDCHIP structure
                      describing the chip being used.

    Return Value:
        Returns a status value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxNtmHelpBuildSpareArea(
    NTMHOOKHANDLE       hNtmHook,
    NTMINFO            *pNtmInfo,
    const D_BUFFER     *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pTag,
    D_BOOL              fUseEcc,
    const FFXNANDCHIP  *pChipInfo)
{
    DclAssert(pChipInfo);

    switch(FfxNtmHelpGetSpareAreaFormat(pChipInfo))
    {
      #if FFXCONF_NANDSUPPORT_OFFSETZERO
        case NSF_OFFSETZERO:
        {
            FfxNtmOffsetZeroBuildSpareArea(hNtmHook, pNtmInfo, pData, pSpare, pTag, fUseEcc);
            return FFXSTAT_SUCCESS;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_SSFDC
        case NSF_SSFDC:
        {
            FfxNtmSSFDCBuildSpareArea(hNtmHook, pNtmInfo, pData, pSpare, pTag, fUseEcc);
            return FFXSTAT_SUCCESS;
        }
      #endif

        case NSF_UNKNOWN:
        default:
        {
            FFXPRINTF(1, ("FfxNtmHelpBuildSpareArea: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            DclError();

            return FFXSTAT_UNSUPPORTEDFUNCTION;
        }
    }
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpCorrectPage()

    This function examines an page using the OffsetZero or SSFDC
    format based on the type of chip being used, as specified by
    pChipInfo.

    It determines if the page is OK by checking the page flags
    and potentially the ECC. If a failure is found, the ECC will
    attempt to correct the problem and return a status which will
    indicate the state of the data.

    Parameters:
        pData      - A pointer to page of data read from the flash.
        pSpare     - A pointer to spare area associated with the
                     given data sector.
        pabECC     - A pointer to an array of ECC bytes -- typically
                     3 bytes for every DATA_BYTES_PER_ECC worth of
                     data.
        pChipInfo  - A pointer to the FFXNANDCHIP structure
                     describing the chip being used.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.

        ffxStat will be FFXSTAT_FIMCORRECTABLEDATA if one or more
        bit errors were corrected with ECC.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxNtmHelpCorrectPage(
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC,
    const FFXNANDCHIP  *pChipInfo)
{
    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(pChipInfo);

    /*  The "size_t" casts below are safe as unsigned
        will always be at least as wide as D_UINT16.
    */
    
    switch(FfxNtmHelpGetSpareAreaFormat(pChipInfo))
    {
      #if FFXCONF_NANDSUPPORT_OFFSETZERO
        case NSF_OFFSETZERO:
        {
            return FfxNtmOffsetZeroCorrectPage((size_t)pChipInfo->pChipClass->uSpareSize, pData, pSpare, pabECC);
        }
      #endif

      #if FFXCONF_NANDSUPPORT_SSFDC
        case NSF_SSFDC:
        {
            return FfxNtmSSFDCCorrectPage((size_t)pChipInfo->pChipClass->uSpareSize, pData, pSpare, pabECC);
        }
      #endif

        case NSF_UNKNOWN:
        default:
        {
            FFXIOSTATUS ioStat;

            FFXPRINTF(1, ("FfxNtmHelpCorrectPage: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            DclError();

            ioStat.ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;
            return ioStat;
        }
    }
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpIsBadBlock()

    This function determines if the given block is bad, according
    to the bad-block marking style for the given chip class.

    Parameters:
        hNTM             - The NTM handle
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
FFXIOSTATUS FfxNtmHelpIsBadBlock(
    NTMHANDLE                   hNTM,
    PFNREADSPAREAREA            pfnReadSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock)
{
    FFXIOSTATUS                 ioStat = DEFAULT_BLOCKIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmHelpIsBadBlock() Block=%lX\n", ulBlock));

    DclAssert(hNTM);
    DclAssert(pfnReadSpareArea);
    DclAssert(pChipInfo);

    switch(pChipInfo->bFlags & CHIPFBB_MASK)
    {
      #if FFXCONF_NANDSUPPORT_SSFDC
        case CHIPFBB_SSFDC:
        {
            ioStat = FfxNtmSSFDCIsBadBlock(hNTM, pfnReadSpareArea, pChipInfo, ulBlock);
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_OFFSETZERO
        case CHIPFBB_OFFSETZERO:
        {
            ioStat = FfxNtmOffsetZeroIsBadBlock(hNTM, pfnReadSpareArea, pChipInfo, ulBlock);
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_LEGACY
        case CHIPFBB_ANYBITZERO:
        {
            ioStat = FfxNtmLegacyIsBadBlock(hNTM, pfnReadSpareArea, pChipInfo, ulBlock);
            break;
        }
      #endif

        case CHIPFBB_NONE:
        {
            /*  It's OK for IsBadBlock to be called for NAND that has no bad
                blocks.  We just report that the block isn't bad.
            */
            ioStat.ffxStat = FFXSTAT_SUCCESS;
            ioStat.op.ulBlockStatus = BLOCKSTATUS_NOTBAD;
            break;
        }

        default:
        {
            FFXPRINTF(1, ("FfxNtmHelpIsBadBlock: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));

            DclError();
            break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmHelpIsBadBlock() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpSetBlockType()

    Attempts to mark the given block bad in a fashion that is
    compatible and distinct from the factory bad block method,
    according to the style of chip specified by pChipInfo.

    Parameters:
        hNTM              - The NTM handle
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
FFXIOSTATUS FfxNtmHelpSetBlockType(
    NTMHANDLE                   hNTM,
    PFNREADSPAREAREA            pfnReadSpareArea,
    PFNWRITESPAREAREA           pfnWriteSpareArea,
    const FFXNANDCHIP          *pChipInfo,
    D_UINT32                    ulBlock,
    D_UINT32                    ulBlockStatus)
{
    FFXIOSTATUS                 ioStat = DEFAULT_BLOCKIO_STATUS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmHelpSetBlockType() Block=%lX Status=%lX\n", ulBlock, ulBlockStatus));

    DclAssert(hNTM);
    DclAssert(pfnWriteSpareArea);
    DclAssert(pChipInfo);

    switch(pChipInfo->bFlags & CHIPFBB_MASK)
    {
      #if FFXCONF_NANDSUPPORT_SSFDC
        case CHIPFBB_SSFDC:
            ioStat = FfxNtmSSFDCSetBlockType(hNTM, pfnReadSpareArea,
                pfnWriteSpareArea, pChipInfo, ulBlock, ulBlockStatus);

            break;
      #endif

      #if FFXCONF_NANDSUPPORT_OFFSETZERO
        case CHIPFBB_OFFSETZERO:
            ioStat = FfxNtmOffsetZeroSetBlockType(hNTM, pfnReadSpareArea,
                pfnWriteSpareArea, pChipInfo, ulBlock, ulBlockStatus);

            break;
      #endif

      #if FFXCONF_NANDSUPPORT_LEGACY
        case CHIPFBB_ANYBITZERO:
            /*  For flash which simply identifies factory bad blocks as any
                zero bit in the original flash, we must have some way to
                mark bad blocks prior to ever using a block, otherwise we'll
                never be able to re-discover the bad blocks -- because they
                will ALL appear to be bad.
            */
            ioStat = FfxNtmLegacySetBlockType(hNTM, pfnReadSpareArea,
                pfnWriteSpareArea, pChipInfo, ulBlock, ulBlockStatus);

            break;
      #endif

        case CHIPFBB_NONE:
            /*  Really should not be calling this if the NAND does not require
                BBM.  Fall through....
            */
        default:
        {
            FFXPRINTF(1, ("FfxNtmHelpSetBlockType: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            ioStat.ffxStat = FFXSTAT_UNSUPPORTEDFUNCTION;

            DclError();
            break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmHelpSetBlockType() Block=%lX returning %s\n",
        ulBlock, FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpCorrectDataSegment()

    Attempt to correct data errors in the specified data segment
    (DATA_BYTES_PER_ECC long).

    If this function does NOT encounter any uncorrectable data
    errors, the pIOStat->ulCount field will be incremented.

    If this function encounters correctable data errors, the
    PAGESTATUS_DATACORRECTED value will be OR'd into the
    supplied pIOStat->op.ulPageStatus field.

    Parameters:
        nCount         - The current segment count
        pData          - A pointer to the data segment
        pabEccOriginal - The original ECC value
        pabEccCalced   - The calculated ECC value
        pIOStat        - A pointer to the FFXIOSTATUS structure to use.

    Return Value:
        Returns TRUE if the segment has uncorrectable errors,
        otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxNtmHelpCorrectDataSegment(
    unsigned        nCount,
    D_BUFFER       *pData,
    D_BUFFER       *pabEccOriginal,
    const D_BUFFER *pabEccCalced,
    FFXIOSTATUS    *pIOStat)
{
    DCLECCRESULT    iResult;

    /*  nCount is used only if the debug level is 2 or higher. Prevent
        compiler warnings for debug less than two.
    */
    (void)nCount;

    iResult = FfxEccCorrect(pData, pabEccOriginal, pabEccCalced);
    if(iResult == DCLECC_UNCORRECTABLE)
    {
        FFXPRINTF(2, ("ECC failure in the %u bytes starting at page offset %4u\n",
                      DATA_BYTES_PER_ECC, nCount * DATA_BYTES_PER_ECC));

        return TRUE;
    }
    else
    {
        if(iResult == DCLECC_DATACORRECTED)
        {
            FFXPRINTF(2, ("ECC corrected a data error in the %u bytes starting at page offset %u\n",
                          DATA_BYTES_PER_ECC, nCount * DATA_BYTES_PER_ECC));

            pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
            FfxErrMgrEncodeCorrectedBits(1, pIOStat);
        }
        else if(iResult == DCLECC_ECCCORRECTED)
        {
            FFXPRINTF(2, ("ECC contained an error for the %u bytes starting at page offset %u\n",
                          DATA_BYTES_PER_ECC, nCount * DATA_BYTES_PER_ECC));

            pIOStat->op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
            FfxErrMgrEncodeCorrectedBits(1, pIOStat);
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
    Public: FfxNtmHelpReadPageStatus()

    Get the status for a page.

    Parameters:
        hNTM             - The NTM handle
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
FFXIOSTATUS FfxNtmHelpReadPageStatus(
    NTMHANDLE           hNTM,
    PFNREADSPAREAREA    pfnReadSpareArea,
    D_UINT32            ulPage,
    const FFXNANDCHIP  *pChipInfo)
{
    FFXIOSTATUS         ioStat;
    DCLALIGNEDBUFFER    (buffer, ns, FFX_NAND_MAXSPARESIZE);
    
    DclAssert(pChipInfo);

    ioStat = (*pfnReadSpareArea)(hNTM, ulPage, (void *)buffer.ns);
    if(!IOSUCCESS(ioStat, 1))
        return ioStat;

    switch(FfxNtmHelpGetSpareAreaFormat(pChipInfo))
    {
      #if FFXCONF_NANDSUPPORT_OFFSETZERO
        case NSF_OFFSETZERO:
        {
            ioStat.op.ulPageStatus = FfxNtmHelpGetPageStatus(buffer.ns, 
                                    NSOFFSETZERO_TAG_OFFSET, NSOFFSETZERO_FLAGS_OFFSET);
            break;
        }
      #endif

      #if FFXCONF_NANDSUPPORT_SSFDC
        case NSF_SSFDC:
        {
            ioStat.op.ulPageStatus = FfxNtmHelpGetPageStatus(buffer.ns, 
                                    NSSSFDC_TAG_OFFSET, NSSSFDC_FLAGS_OFFSET);
            break;
        }
      #endif

        case NSF_UNKNOWN:
        default:
        {
            ioStat.ffxStat = FFXSTAT_UNSUPPORTEDREQUEST;

            FFXPRINTF(1, ("FfxNtmHelpReadPageStatus: Operation not supported for this part, Chip Flags=%X\n", (D_UINT16)pChipInfo->bFlags));
            DclError();
        }
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpGetPageStatus()

    Return the page status for a page using the prescribed Tag and
    Flags offset.  This function only works with standard length 
    tags.

    Parameters:
        pSpare       - A pointer to a buffer containing the spare 
                       area data.
        nTagOffset   - The offset into the spare area where the tag
                       is stored.
        nFlagsOffset - The offset into the spare area where the flags
                       bytes are stored.

    Return Value:
        Returns a D_UINT32 containing the page status bits.
-------------------------------------------------------------------*/
D_UINT32 FfxNtmHelpGetPageStatus(
    const D_BUFFER     *pSpare,
    size_t              nTagOffset,
    size_t              nFlagsOffset)
{
    D_UINT32            ulPageStatus;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxNtmHelpGetPageStatus() TagOffset=%u FlagsOffset=%u\n", nTagOffset, nFlagsOffset));

    DclAssertReadPtr(pSpare, 0);

    /*  If the tag area, including the ECC and check bytes, is within
        1 bit of being erased, then we know that a tag was not written.
        If it is anything else, we know a tag was written.  This function
        always uses standard size tags.
    */
    if(!FfxNtmHelpIsRangeErased1Bit(pSpare+nTagOffset, LEGACY_ENCODED_TAG_SIZE))
        ulPageStatus = PAGESTATUS_SET_TAG_WIDTH(LEGACY_TAG_SIZE);
    else
        ulPageStatus = 0;

    if(ISWRITTENWITHECC(pSpare[nFlagsOffset]))
    {
        ulPageStatus |= PAGESTATUS_WRITTENWITHECC;
    }
    else if(ISUNWRITTEN(pSpare[nFlagsOffset]))
    {
        ulPageStatus |= PAGESTATUS_UNWRITTEN;
    }
    else
    {
        FFXPRINTF(1, ("FfxNtmHelpGetPageStatus() unrecognized spare area format\n"));
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxNtmHelpGetPageStatus() returning page status %lX\n", ulPageStatus));

    return ulPageStatus;
}


/*-------------------------------------------------------------------
    Public: FfxNtmHelpGetSpareAreaFormat()

    Determine which "standard" spare area format (supported by
    the helper functions) to use for the specified chip. This
    is intended to be used in conjunction with the NTM spare area
    helper functions.  NTMs which utilize their own spare area
    format should not need to use this function.

    NSF_OFFSETZERO - Returned by CHIPFBB_OFFSETZERO or CHIPFBB_NONE
    NSF_SSFDC - Returned by CHIPFBB_SSFDC or CHIPFBB_ANYBITZERO

    Parameters:
        pChipInfo   - A pointer to the FFXNANDCHIP structure
                      describing the chip being used.

    Return Value:
        Returns a NANDSPAREFMT type indicating which spare area
        format to use.
-------------------------------------------------------------------*/
NANDSPAREFMT FfxNtmHelpGetSpareAreaFormat(
    const FFXNANDCHIP *pChip)
{
    DclAssert(pChip);

    switch(pChip->bFlags & CHIPFBB_MASK)
    {
        case CHIPFBB_ANYBITZERO:
        case CHIPFBB_SSFDC:
            return NSF_SSFDC;

        case CHIPFBB_NONE:
        case CHIPFBB_OFFSETZERO:
            return NSF_OFFSETZERO;

        default:
            return NSF_UNKNOWN;
    }
}


#if 0
/*  Test for FfxNtmHelpGetSpareAreaFormat().
*/
void Test_NS_FORMAT(void)
{
    FFXNANDCHIP sChip;

    DclMemSet(&sChip, 0, sizeof(sChip));

    sChip.bFlags = CHIPFBB_ANYBITZERO;
    if(FfxNtmHelpGetSpareAreaFormat(&sChip) != NSF_SSFDC)
        DclProductionError();

    sChip.bFlags = CHIPFBB_SSFDC;
    if(FfxNtmHelpGetSpareAreaFormat(&sChip) != NSF_SSFDC)
        DclProductionError();

    sChip.bFlags = CHIPFBB_NONE;
    if(FfxNtmHelpGetSpareAreaFormat(&sChip) != NSF_OFFSETZERO)
        DclProductionError();

    sChip.bFlags = CHIPFBB_OFFSETZERO;
    if(FfxNtmHelpGetSpareAreaFormat(&sChip) != NSF_OFFSETZERO)
        DclProductionError();

    sChip.bFlags = ~(CHIPFBB_NONE | CHIPFBB_ANYBITZERO | CHIPFBB_SSFDC | CHIPFBB_OFFSETZERO);
    if(FfxNtmHelpGetSpareAreaFormat(&sChip) != NSF_UNKNOWN)
        DclProductionError();
}
#endif



#endif  /* FFXCONF_NANDSUPPORT */


