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

    This module contains the NAND Simulator NAND Technology Module (NTM)
    which supports simulating NAND flash on virtually any media.  This
    NTM is typically only used for testing and debugging purposes.

    The NAND Simulator consists of four components; NTM interface,
    simulation mechanism, error policy and storage mechanism.  The NTM
    interface and error mechanism are fixed components whereas the error
    policy and storage mechanisms are intended to be interchangeable with
    different implementations to suite the needs of the user.

    The following configuration option settings are used:

    FFXOPT_NANDSIM_ID         - Flash ID string.
    FFXOPT_NANDSIM_NUM_CHIPS  - Number of chips to simulate.
    FFXOPT_NANDSIM_POLICY     - The error policy to use.  If this option is
                                not set, the Null Policy is used and no
                                errors will be generated.
    FFXOPT_NANDSIM_STORAGE    - The storage mechanism to use.  If this
                                option is not set, the RAM storage mechanism
                                will be used.

    For a given project, a customized fhoption.c module may be used to provide
    project specific values for the settings described above.  The specific
    error policy or storage mechanism in use may have additional options.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntsim.c $
    Revision 1.60  2010/12/10 17:47:51Z  glenns
    Fix bug 3213- Add code to be sure appropriate value is being
    assigned to pNtmInfo->uEdcSegmentSize.
    Revision 1.59  2010/07/08 03:42:50Z  garyp
    Use the standard error injection macros.
    Revision 1.58  2010/07/07 19:37:06Z  garyp
    Added support for error injection via the "SetParam" interface.
    Revision 1.57  2009/12/23 17:02:38Z  keithg
    Removed incorrect pointer dereference in DECODEID option call.
    Revision 1.56  2009/12/22 04:33:32Z  keithg
    Changed warning messages regarding out of sequence writes and 'skipped' page
    writes.  Made associated debug messages all level 2.  Added a callout into
    the option module to allow decoding of an otherwise unknown chip ID code.
    Revision 1.55  2009/12/13 03:26:22Z  garyp
    Minor enhancement to ValidatePageWrite().
    Revision 1.54  2009/12/11 23:50:01Z  garyp
    Updated to use some functions which were renamed to avoid naming conflicts.
    Revision 1.53  2009/12/03 00:34:37Z  garyp
    Minor debug code updates -- no functional changes.
    Revision 1.52  2009/10/07 17:34:21Z  garyp
    Eliminated the use of ntspare.h.  Replaced with fxnandapi.h.
    Revision 1.51  2009/10/06 19:13:54Z  garyp
    Updated to use re-abstracted ECC calculation and correction functions.
    Eliminated use of the FFXECC structure.  Modified to use some renamed
    functions to avoid naming conflicts.
    Revision 1.50  2009/08/01 02:09:47Z  garyp
    Merged from the v4.0 branch.  Modified so the PageWrite() functionality
    allows a page to be written with ECC, but no tags.  As before, if ECC is
    turned off, then tags may not be written. Added a default IORequest()
    function.  Documentation updated.
    Revision 1.49  2009/04/09 03:42:54Z  garyp
    Renamed a helper function to avoid namespace collisions.
    Revision 1.48  2009/04/02 18:03:34Z  davidh
    Function headers updated for AuroDoc.
    Revision 1.47  2009/03/31 02:37:53Z  glenns
    - Fix Bugzilla #2565: Repair HiddenWrite to calculate ECCs and
      add them in the spare area if the EDC mechanism is configured
      to cover metadata.
    - Fix Bugzilla #2566: Correct logical errors in section of code to
      evaluate full-up EDC should tag CRC check fail.
    Revision 1.46  2009/03/27 06:08:11Z  glenns
    - Fix Bugzilla #2464: See documentation in bugzilla report for details.
    Revision 1.45  2009/03/17 20:50:14Z  glenns
    - Fix Bugzilla #2523: Modified check on CorrectPage to handle the
      case where page status indicates correctable error in a page, but
      FFXSTATUS indicates uncorrectable error elsewhere.
    Revision 1.44  2009/03/13 04:08:16Z  glenns
    - Correct merging errror from last check-in
    Revision 1.43  2009/03/13 03:40:15Z  glenns
    - Fix Bugzilla #2501: This bug was a side effect of revision 1.41;
      basically the code was originally written not to set any values if the
      required spare area size matched the chip's reported spare area
      size, but with the introduction of the separation between available
      spare area and actual spare area used, the amount of spare area
      used needs to be explicity set under all condiitions.
    Revision 1.42  2009/03/12 22:03:02Z  keithg
    Added static to function declarations that were defined as such.
    Revision 1.41  2009/03/09 18:36:08Z  glenns
    - Fix for Bugzilla #2384: Part of this Bugzilla fix requires NTM's that
      use custom spare formats and parameterized EDC to report the
      physical size of the spare area to the FIM clients and only the
      amount of spare area actually used to the parameterized EDC
      mechanism, and those values are not necessarily the same.
      This checkin accomplishes that adjustment for this NTM.
    Revision 1.40  2009/03/04 20:09:51Z  glenns
    - Fix Bugzilla #2393: Removed all reserved block/reserved page
      processing from the NTM. This is now handled by the device
      manager.
    - Fixed bug in conditional looking for "corrected data" page status.
    Revision 1.39  2009/02/25 19:28:13Z  glenns
    - Modified to use more flexible and rigorous method of tag
      encoding now in ntpecchelp.
    Revision 1.38  2009/02/18 23:57:57Z  glenns
    - Updated to accommodate new function prototypes for EDC
      processor "Init" and "Destroy" functions. Part of fix for Bugzilla
      #2416.
    Revision 1.37  2009/02/09 05:43:34Z  glenns
    - Prepare NTM to handle odd-size spare areas reported by chip
      specifications in nandid..
    Revision 1.36  2009/02/06 02:25:14Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.35  2009/01/28 22:59:48  glenns
    - Minor variable name change.
    - Added calls to functions for checking and setting block status if
      the parameterized ECC interface is in use.
    Revision 1.34  2009/01/27 04:03:43Z  glenns
    - Clarified commentary.
    - Changed default value of NTM_USES_CUSTOM_SPARE_FORMAT
      to FALSE, as automatic detection now exists to set it to TRUE in
      the necessary circumstances.
    - Added some consistency checking and auto-detection for
      circumstances either requiring or forbidding custom spare area
      format indication.
    - Modified to accomodate variable names changed to meet
      Datalight coding standards.
    - Removed unnecessary assignment of "FALSE" to fSuccess flag
      in Create.
    Revision 1.33  2009/01/23 17:49:41Z  billr
    Resolve bug 1838: EUH Invalidation may fail for MLC parts.  Propagate
    fProgramOnce from FFXNANDCHIPCLASS structure to DEV_PGM_ONCE in
    uDeviceFlags of NTMINFO.
    Revision 1.32  2009/01/19 05:25:29Z  keithg
    Added support for the NTSIM_CHECK parameter.
    Revision 1.31  2009/01/19 04:12:24Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.30  2009/01/18 18:19:30Z  glenns
    Fixed erroneous check of failed memory allocation in "Create"
    Revision 1.29  2009/01/17 00:01:14Z  glenns
    - Added macros for enabling or disabling the use of parameterized
      EDC and additional spare area formats.
    - Modified Create and Destroy procedures to detect configuration
      with regard to EDC and spare area, and to appropriately acquire
      and dispose of scratch buffers and an EDC processor as
      necessary.
    - Fixed up literal FFXIOSTATUS initialization in eleven places.
    - Added code to the PageRead procedure to detect if
      parameterized ECC is being used, and if it is, appropriately format
      an ECC_PARAMS structure and feed it into the parameterized
      EDC versions of the nthelp support. Non-parameterized EDC
      continues to be supported unchanged.
    - Added code the PageRead procedure to set the FFXIOSTATUS
      ffxstatus value to FIMCORRECTABLEDATA if a corrected error
      has been noted.
    - Added code to WritePage to detect if parameterized EDC is in
      use, and if it is, to appropriately format an ECC_PARAMS
      structure and feed it into the parameterized-EDC version of
      BuildSpareArea.
    - Modified HiddenRead to detect if parameterized EDC is in use,
      and if it is, use the parameterized-EDC version of TagDecode.
    - Modified HiddenRead to read the entire page with EDC on if the
      first attempt to call TagDecode fails, causing EDC to be applied
      to the tag.
    - Modified HiddenWrite to use the parameterized-EDC version of
      TagEncode if parameterized EDC is in use.
    - Modified GetPageStatus to use the parameterized-EDC version
      of nthelp's GetPageStatus if parameterized EDC is in use.
    Revision 1.28  2008/12/09 08:47:09Z  keithg
    Corrected page access tracking to allow non-contiguous operations.
    Revision 1.27  2008/10/08 20:14:09Z  keithg
    Conditioned reserved space (bounds) on BBM v5 since it is now
    completed by the device manager.
    Revision 1.26  2008/09/02 05:59:46Z  keithg
    The DEV_REQUIRES_BBM device flag no longer requires
    that BBM functionality is compiled in.
    Revision 1.25  2008/06/16 16:56:10Z  thomd
    Renamed ChipClass field to match higher levels;
    propagate chip capability fields in Create routine
    Revision 1.24  2008/03/23 18:59:09Z  Garyp
    Updated the PageRead/Write() and HiddenRead/Write() interfaces to take a
    tag length parameter.  Modified the GetPageStatus() functionality to return
    the tag length, if any.
    Revision 1.23  2008/02/03 04:11:53Z  keithg
    comment updates to support autodoc
    Revision 1.22  2007/12/27 00:22:56Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.21  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.20  2007/09/21 04:04:37Z  pauli
    Resolved Bugs 1455 and 1272: the simulator can now correctly simulate SSFDC
    and OffsetZero style flash parts (small and large page/block).  Added the
    ability to report that BBM should or should not be used based on the
    characteristics of the NAND part identified.  Enhanced how the NTM tracks
    and validates writes to the flash.
    Revision 1.19  2007/09/12 21:25:34Z  Garyp
    Updated to use some renamed functions.
    Revision 1.18  2007/09/06 14:25:06Z  thomd
    Add assert for non-zero divisor
    Revision 1.17  2007/08/25 00:14:22Z  pauli
    Updated trace messages and levels to be more consistent and sensible.
    Revision 1.16  2007/08/02 23:08:23Z  timothyj
    Changed units of reserved space and maximum size to be in terms of KB
    instead of bytes.
    Revision 1.15  2007/07/26 22:44:00Z  billr
    Make this compile when FFXCONF_NAND_SIMULATOR is FALSE.  Some compilers
    don't like functions to be declared static but not defined.
    Revision 1.14  2007/07/24 18:40:39Z  rickc
    Removed assert in RawPageWrite() that incorrectly forced caller to provide
    a spare area.
    Revision 1.13  2007/07/20 21:15:36Z  pauli
    Resolve Bug 954: Moved #if FFXCONF_NAND_SIMULATOR to after include for
    nandsim.h where it is defined if not already defined.
    Revision 1.12  2007/04/07 03:29:19Z  Garyp
    Removed some unnecessary asserts.  Documentation updated.
    Revision 1.11  2007/03/30 00:26:04Z  timothyj
    Changed Validate() to account for reserved areas.
    Revision 1.10  2007/03/02 01:04:32Z  billr
    Fix compiler warning (ulPage could be used before set). Report correct
    starting address in trace output.
    Revision 1.9  2007/03/01 00:35:09Z  timothyj
    Changed references to local uPagesPerBlock to use the value now in the
    FimInfo structure.  Modified call to FfxDevApplyArrayBounds() to pass and
    receive on return a return block index in lieu of byte offset, for LFA
    support.  Changed references to the chipclass device size to use chip
    blocks (table format changed, see nandid.c).
    Revision 1.8  2007/02/14 00:03:31Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.7  2007/02/09 19:58:43Z  timothyj
    Updated interfaces to use blocks and pages instead of linear byte offsets.
    Revision 1.6  2007/01/03 23:52:38Z  Timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Removed FFXNANDMFG (replaced references with references to the constant
    FFXNANDCHIP returned, above).
    Revision 1.5  2006/11/08 03:38:46Z  Garyp
    Modified the ParameterGet/Set() functions to return an FFXSTATUS value
    rather than a D_BOOL.
    Revision 1.4  2006/10/04 00:58:40Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.3  2006/08/26 00:30:04Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.2  2006/08/18 18:17:25Z  billr
    Fix build on Linux: forward slashes in paths are portable, backslashes
    are not.
    Revision 1.1  2006/07/26 20:41:44Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxiosys.h>
#include <deverr.h>
#include <ecc.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandsim.h>
#include <nandid.h>
#include "nand.h"


#if FFXCONF_NANDSUPPORT
#include "ntm.h"

#define NTMNAME "NTSIM"


/*  Special values for tracking the last page written in each block.
*/
#define NTSIM_LPW_NONE       (D_UINT16_MAX)
#define NTSIM_LPW_UNKNOWN    (D_UINT16_MAX - 1)

/*  Macro for defining whether to use parameterized ECC
    (see ecc.h). Note that "NTM_USES_CUSTOM_SPARE_FORMAT" will be
    overridden if the device in use requires more than one bit
    of EDC.
*/
#define NTM_USES_PARAMETERIZED_ECC TRUE
#define NTM_USES_CUSTOM_SPARE_FORMAT FALSE

struct tagNTMDATA
{
    NTMINFO             NtmInfo;            /* information visible to upper layers */
    const FFXNANDCHIP  *pChipInfo;          /* chip information                    */
    FFXDEVHANDLE        hDev;
    D_UINT16           *pauLastPageWritten; /* array of the last pages written per block */
    D_BUFFER           *pSpare;
    NsMechanismHandle   hSim;
    D_UINT32            ulRawTotalBlocks;
  #if FFXCONF_ERRORINJECTIONTESTS
    ERRINJHANDLE        hEI;
  #endif
    D_UINT16            uTagOffset;
    FX_EDC_PROCESSOR   *pEccProcessor;
};

static D_BOOL       Validate(NTMHANDLE hNTM, D_UINT32 ulStartPage, D_UINT32 ulCount, const char *pszFunc);
static D_BOOL       ValidatePageWrite(NTMHANDLE hNTM, D_UINT32 ulPage, const char *pszFunc);
static D_BOOL       InitializePageTracking(NTMHANDLE hNTM, D_BOOL fNewFlash);
static FFXIOSTATUS  ReadSpareArea( NTMHANDLE hNTM, D_UINT32 ulPage, D_BUFFER *pSpare);
static FFXIOSTATUS  WriteSpareArea(NTMHANDLE hNTM, D_UINT32 ulPage, const D_BUFFER *pSpare);


/*-------------------------------------------------------------------
    NTM Declaration

    This structure declaration is used to define the entry points
    into the NTM.
-------------------------------------------------------------------*/
NANDTECHNOLOGYMODULE FFXNTM_sim =
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
    D_BOOL              fNewFlash = FALSE;
    D_BUFFER            abFlashID[NAND_ID_SIZE] = {0};
    D_UINT32            ulNumChips;
    FFXSTATUS           ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
            "NTSIM-Create()\n"));

    DclAssert(hDev);
    DclAssert(ppNtmInfo);

    pNTM = DclMemAllocZero(sizeof *pNTM);
    if(!pNTM)
        goto CreateCleanup;

    pNTM->hDev = hDev;

    /*  Create the Simulator Mechanism for this device.
        This will subsequently initialize the error policy and storage
        mechanism.
    */
    pNTM->hSim  = NsMechanismCreate(hDev, &fNewFlash);
    if(!pNTM->hSim)
        goto CreateCleanup;

    /*  Get the device parameters from the simulation mechanism.
    */
    ffxStat = NsMechanismGetParameters(pNTM->hSim, abFlashID, &ulNumChips);
    if(ffxStat != FFXSTAT_SUCCESS)
        goto CreateCleanup;

    pNTM->pChipInfo = FfxNandDecodeID(abFlashID);
    if(!pNTM->pChipInfo)
    {
        /*  If this is an unknown chip, see if the project is handling it.
        */
        if(!FfxHookOptionGet(FFXOPT_NANDSIM_DECODEID, hDev,
                             (void *)&pNTM->pChipInfo, sizeof(pNTM->pChipInfo)))
        {
            goto CreateCleanup;
        }
    }


    /*  We allocate a full-size spare area here even though we may not use
        all of it. This is to accomodate one-off conditions where a
        particular device or modified NTM may actually encode information in
        areas of the spare area that this NTM does not currently use.
    */
    pNTM->pSpare = DclMemAlloc(pNTM->pChipInfo->pChipClass->uSpareSize);
    if(!pNTM->pSpare)
        goto CreateCleanup;

    pNTM->ulRawTotalBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks * ulNumChips;
    pNTM->NtmInfo.ulBlockSize = pNTM->pChipInfo->pChipClass->ulBlockSize;
    pNTM->NtmInfo.ulTotalBlocks = pNTM->ulRawTotalBlocks;

    pNTM->NtmInfo.uPageSize = pNTM->pChipInfo->pChipClass->uPageSize;

    /*  Calculate this once and store it for use later for
        both optimization and simplification of the code
    */
    DclAssert(pNTM->NtmInfo.uPageSize != 0);
    DclAssert((pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize) <= D_UINT16_MAX);
    DclAssert((pNTM->NtmInfo.ulBlockSize % pNTM->NtmInfo.uPageSize) == 0);
    pNTM->NtmInfo.uPagesPerBlock = (D_UINT16)(pNTM->NtmInfo.ulBlockSize / pNTM->NtmInfo.uPageSize);

    pNTM->NtmInfo.ulChipBlocks = pNTM->pChipInfo->pChipClass->ulChipBlocks;
    pNTM->NtmInfo.uPageSize = pNTM->pChipInfo->pChipClass->uPageSize;
    pNTM->NtmInfo.uSpareSize = pNTM->pChipInfo->pChipClass->uSpareSize;
    pNTM->NtmInfo.uMetaSize = FFX_NAND_TAGSIZE;
    pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_ECC;
    if((pNTM->pChipInfo->bFlags & CHIPFBB_MASK) != CHIPFBB_NONE)
        pNTM->NtmInfo.uDeviceFlags |= DEV_REQUIRES_BBM;
    if (pNTM->pChipInfo->pChipClass->fProgramOnce)
        pNTM->NtmInfo.uDeviceFlags |= DEV_PGM_ONCE;

    pNTM->NtmInfo.uEdcRequirement    = pNTM->pChipInfo->pChipClass->uEdcRequirement;
    pNTM->NtmInfo.ulEraseCycleRating  = pNTM->pChipInfo->pChipClass->ulEraseCycleRating;
    pNTM->NtmInfo.ulBBMReservedRating = pNTM->pChipInfo->pChipClass->ulBBMReservedRating;

    /*  Determine which spare area format to use, and whether to use
        parameterized ECC as given in ecc.h:
    */
    pNTM->NtmInfo.fEdcParameterized = NTM_USES_PARAMETERIZED_ECC;

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
        if (pNTM->NtmInfo.uSpareSize < NSDEFAULTMLC_SPARESIZE)
        {
            DclPrintf("FFX: Detected part spare size too small for required spare format.\n");
            goto CreateCleanup;
        }
        pNTM->uTagOffset = NSDEFAULTMLC_TAG_OFFSET;
    }
    else switch(FfxNtmHelpGetSpareAreaFormat(pNTM->pChipInfo))
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

    /*  See what to do about ECC for this NTM/chip combination
    */
    if (pNTM->NtmInfo.fEdcParameterized)
    {
    D_UINT32 ulEccSize;
    D_UINT16 uRequiredSpareSize;

        /*  Acquire an ECC processor, and if it has an initialization procedure,
            call it.
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

    }
    else
    {
        pNTM->pEccProcessor = NULL;
        pNTM->NtmInfo.pScratchSpareBuffer = NULL;
        pNTM->NtmInfo.pScratchEccBuffer = NULL;
        pNTM->NtmInfo.pScratchPageBuffer = NULL;
        if (pNTM->pChipInfo->pChipClass->uEdcCapability)
            pNTM->NtmInfo.uEdcCapability = pNTM->pChipInfo->pChipClass->uEdcCapability;
        else
            pNTM->NtmInfo.uEdcCapability = 1;
        
        /*  REFACTOR: The following assignment really should be based on
            information read from the NAND ID table, but the tables don't
            currently support such a field. So, for now, lets just assume 
            a value.
        */
        pNTM->NtmInfo.uEdcSegmentSize = DATA_BYTES_PER_ECC;
    }

    /*  Initialize the mechanism for tracking the last page written to ensure
        that pages are written sequentially within each block.
    */
    if(!InitializePageTracking(pNTM, fNewFlash))
        goto CreateCleanup;

  #if FFXCONF_ERRORINJECTIONTESTS
    pNTM->hEI = FFXERRORINJECT_CREATE(hDev, DclOsTickCount());
  #endif

    FFXPRINTF(1, ("NTSIM: RawTotalBlocks=%lX ChipBlocks=%lX BlockSize=%lX PageSize=%X\n",
        pNTM->ulRawTotalBlocks,
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

        if(pNTM->pSpare)
        {
            DclMemFree(pNTM->pSpare);
            pNTM->pSpare = NULL;
        }

        if(pNTM->hSim)
        {
            NsMechanismDestroy(pNTM->hSim);
            pNTM->hSim = NULL;
        }

        DclMemFree(pNTM);
        pNTM = NULL;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTSIM-Create() returning %P\n", pNTM));

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
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-Destroy()\n"));

    DclAssert(hNTM);

    FFXERRORINJECT_DESTROY();

    /*  Free resources in reverse order
    */
    if(hNTM->pauLastPageWritten)
    {
        DclMemFree(hNTM->pauLastPageWritten);
        hNTM->pauLastPageWritten = NULL;
    }

    /*  Free scratch buffers if they were allocated:
    */
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

    /*  Destroy the Ecc processor if one was acquired:
    */
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

    /*  Free the spare area
    */
    if(hNTM->pSpare)
    {
        DclMemFree(hNTM->pSpare);
        hNTM->pSpare = NULL;
    }

    /*  Free the simulation mechanism
    */
    if(hNTM->hSim)
    {
        NsMechanismDestroy(hNTM->hSim);
        hNTM->hSim = NULL;
    }

    DclMemFree(hNTM);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTSIM-Destroy() returning\n"));

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
    D_UINT32            ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-PageRead() Start Page=%lX Count=%U pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTSIM-PageRead", 0, ulCount);

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

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulStartPage, ulCount, "PageRead"))
        goto PageReadCleanup;

    /*  Read each of the requested pages one at a time.
    */
    ulPage = ulStartPage;
    while(ulCount)
    {
        /*  Read a page from the Simulator mechanism.
        */
        ioStat.ffxStat = NsMechanismReadPage(hNTM->hSim, ulPage, pPages);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Read the spare area from the Simulator mechanism.
        */
        ioStat.ffxStat = NsMechanismReadSpare(hNTM->hSim, ulPage, hNTM->pSpare);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
             break;

        if(fUseEcc)
        {
        FFXIOSTATUS ioValid= INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);

            if (hNTM->NtmInfo.fEdcParameterized)
            {
            FX_ECC_PARAMS eccParams;


                /*  At this point, hNTM->pSpare now contains the spare area
                    that was read from the Nandsim mechanism. This is the
                    "reference" spare area. Using this and the page data
                    we just read, we need to generate new ECC's, and then
                    call the correction procedure. We'll use the
                    FfxNtmPECCBuildSpareArea procedure to generate the
                    new ECCs after duplicating the conditions in which
                    the original reference spare area was built:

                    To duplicate those conditoins, we formulate the ECCParams
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
                    eccParams.metadata_buffer.buffer = hNTM->pSpare;
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
                                             &(hNTM->pSpare[hNTM->uTagOffset]),
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
                                      hNTM->pSpare,
                                      hNTM->pChipInfo);

                /*  Finish with tag data. The caller may not care about it,
                    so only do it if a tag buffer is supplied. In the case of
                    parameterized ECC, we evaluate tags after performing
                    correction because the ECC may cover tags.
                */
                if(pTags)
                {
                    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                    (void)FfxNtmPECCTagDecode(&hNTM->NtmInfo, pTags, hNTM->pSpare, FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo));

                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                                   "Encoded Tag=%lX Decoded Tag=%X\n",
                                   *(D_UINT32 *)&hNTM->pSpare[hNTM->uTagOffset],
                                   *(D_UINT16 *)pTags));

                    pTags += nTagSize;
                }

            }
            else
            {
                D_BUFFER    abECC[MAX_ECC_BYTES_PER_PAGE];

                /*  The caller may not care about the tag data, so copy it only
                    if a pointer is supplied.
                */
                if(pTags)
                {
                    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

                    FfxNtmHelpTagDecode(pTags, &hNTM->pSpare[hNTM->uTagOffset]);

                    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                                   "Encoded Tag=%lX Decoded Tag=%X\n",
                                   *(D_UINT32 *)&hNTM->pSpare[hNTM->uTagOffset],
                                   *(D_UINT16 *)pTags));

                    pTags += nTagSize;
                }

                FfxHookEccCalcStart(NULL, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);
                FfxHookEccCalcRead(NULL, pPages, hNTM->NtmInfo.uPageSize, abECC, ECC_MODE_READ);

                ioValid = FfxHookEccCorrectPage(NULL, pPages, hNTM->pSpare, abECC, hNTM->pChipInfo);
            }

            if (ioValid.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
            {
				ioStat.ulCount++;
            }
            ioStat.ffxStat = ioValid.ffxStat;
            ioStat.op.ulPageStatus |= ioValid.op.ulPageStatus;

            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        ioStat.ulCount++;
        ulCount--;
        ulPage++;
        pPages += hNTM->NtmInfo.uPageSize;
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-PageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    D_UINT32            ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-PageWrite() Start Page=%lX Count=%lX pPages=%P pTags=%P TagSize=%u fUseEcc=%U\n",
        ulStartPage, ulCount, pPages, pTags, nTagSize, fUseEcc));

    DclProfilerEnter("NTSIM-PageWrite", 0, ulCount);

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

    /*  Write each of the requested pages one at a time.
    */
    ulPage = ulStartPage;
    while(ulCount)
    {
        /*  Check for out of order writes and rewrites.
        */
        if(!ValidatePageWrite(hNTM, ulPage, "PageWrite"))
            goto PageWriteCleanup;

        /*  Initialize spare area and insert ECC codes
        */
        if (hNTM->NtmInfo.fEdcParameterized)
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
                eccParams.metadata_buffer.buffer = hNTM->pSpare;
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

            FfxNtmPECCBuildSpareArea(hNTM->hDev, &hNTM->NtmInfo, &eccParams, hNTM->pSpare, pTags, fUseEcc, hNTM->pChipInfo);
        }
        else
        {
            FfxNtmHelpBuildSpareArea(NULL, &hNTM->NtmInfo, pPages, hNTM->pSpare, pTags, fUseEcc, hNTM->pChipInfo);
        }

        /*  Write the page to the simulation mechanism.
        */
        ioStat.ffxStat = NsMechanismWritePage(hNTM->hSim, ulPage, pPages);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  Write the spare to the simulation mechanism.
        */
        ioStat.ffxStat = NsMechanismWriteSpare(hNTM->hSim, ulPage, hNTM->pSpare);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;
        
        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("PageWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        pPages      += hNTM->NtmInfo.uPageSize;
        if(nTagSize)
            pTags   += nTagSize;

        ulPage++;
        ulCount--;
    }

  PageWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-PageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-RawPageRead() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTSIM-RawPageRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "RawPageRead"))
        goto PageReadCleanup;

    /*  Read each of the requested pages one at a time.
    */
    while(ulCount)
    {
        /*  Read the page from the simulation mechanism.
        */
        ioStat.ffxStat = NsMechanismReadPage(hNTM->hSim, ulPage, pPages);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The caller may not care about the spare data, so write it only
            if a pointer is supplied.
        */
        if(pSpares)
        {
            /*  Read the spare from the simulation mechanism.
            */
            ioStat.ffxStat = NsMechanismReadSpare(hNTM->hSim, ulPage, pSpares);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            pSpares  += hNTM->NtmInfo.uSpareSize;
        }

        ioStat.ulCount++;
        ulCount--;
        ulPage++;
        pPages      += hNTM->NtmInfo.uPageSize;
    }

  PageReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-RawPageRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-RawPageWrite() Page=%lX Count=%lU pPages=%P pSpares=%P\n",
        ulPage, ulCount, pPages, pSpares));

    DclProfilerEnter("NTSIM-RawPageWrite", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(pPages);
    DclAssert(ulCount);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "RawPageWrite"))
        goto RawPageWriteCleanup;

    /*  Write each of the requested pages one at a time.
    */
    while(ulCount)
    {
        /*  Check for out of order writes and rewrites.
        */
        if(!ValidatePageWrite(hNTM, ulPage, "RawPageWrite"))
            goto RawPageWriteCleanup;

        /*  Write the page to the simulation mechanism.
        */
        ioStat.ffxStat = NsMechanismWritePage(hNTM->hSim, ulPage, pPages);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        /*  The caller may not care about the spare data, so write it only
            if a pointer is supplied.
        */
        if(pSpares)
        {
            /*  Write the spare to the simulation mechanism.
            */
            ioStat.ffxStat = NsMechanismWriteSpare(hNTM->hSim, ulPage, pSpares);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;

            pSpares  += hNTM->NtmInfo.uSpareSize;
        }

        ioStat.ulCount++;

        FFXERRORINJECT_WRITE("RawPageWrite", ulPage, &ioStat);

        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        ulCount--;
        ulPage++;
        pPages       += hNTM->NtmInfo.uPageSize;
    }

  RawPageWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-RawPageWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-SpareRead() Page=%lX\n", ulPage));

    DclProfilerEnter("NTSIM-SpareRead", 0, 1);

    ioStat = ReadSpareArea(hNTM, ulPage, pSpare);

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-SpareRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = {0} /* DEFAULT_PAGEIO_STATUS(FFXSTAT_SUCCESS) */;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-SpareWrite() Page=%lX\n", ulPage));

    DclProfilerEnter("NTSIM-SpareWrite", 0, 1);

    ioStat = WriteSpareArea(hNTM, ulPage, pSpare);

    FFXERRORINJECT_WRITE("SpareWrite", ulPage, &ioStat);

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-SpareWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32            ulPage = ulStartPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-HiddenRead() Page=%lX Count=%lX TagSize=%u\n", ulStartPage, ulCount, nTagSize));

    DclProfilerEnter("NTSIM-HiddenRead", 0, ulCount);

    DclAssert(hNTM);
    DclAssert(ulCount);
    DclAssert(pTags);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, ulCount, "HiddenRead"))
        goto HiddenReadCleanup;

    /*  Read all of the requested tags one at a time.
    */
    while(ulCount)
    {
        /*  Read the spare area from the simulation mechanism.
        */
        ioStat.ffxStat = NsMechanismReadSpare(hNTM->hSim, ulPage, hNTM->pSpare);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        if (hNTM->NtmInfo.fEdcParameterized)
        {
            /*  Decode the tags.
            */
            if (!FfxNtmPECCTagDecode(&(hNTM->NtmInfo), pTags, hNTM->pSpare, FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo)))
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
                                      ulPage,
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
        }
        else
        {
            (void)FfxNtmHelpTagDecode(pTags, &hNTM->pSpare[hNTM->uTagOffset]);
        }

        FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, 0),
                       "Encoded Tag=%lX Decoded Tag=%X\n",
                       *(D_UINT32 *)&hNTM->pSpare[hNTM->uTagOffset],
                       *(D_UINT16 *)pTags));

        ioStat.ulCount++;
        ulPage++;
        pTags += nTagSize;
        ulCount--;
    }

  HiddenReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-HiddenRead() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: HiddenWrite()

    Write the hidden (tag) data for one page.

    Parameters:
        hNTM     - The NTM handle to use
        ulPage   - The flash offset in bytes, on a page boundary.
                   The offset is relative to any reserved space.
        pTag     - Buffer for the hidden data to write.
        nTagSize - The tag size to use.

    Return Value:
        Returns a standard FFXIOSTATUS value.  The ulCount field
        contains the number of tags which were completely written.
        The status indicates whether the read was entirely
        successful.
-------------------------------------------------------------------*/
static FFXIOSTATUS HiddenWrite(
    NTMHANDLE       hNTM,
    D_UINT32        ulPage,
    const D_BUFFER *pTag,
    unsigned        nTagSize)
{
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-HiddenWrite() Page=%lX Tag=%X TagSize=%u\n", ulPage, *(D_UINT16*)pTag, nTagSize));

    DclProfilerEnter("NTSIM-HiddenWrite", 0, 1);

    DclAssert(hNTM);
    DclAssert(pTag);
    DclAssert(nTagSize == hNTM->NtmInfo.uMetaSize);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "HiddenWrite"))
        goto HiddenWriteCleanup;

    /*  Check for out of order writes and rewrites.
    */
    if(!ValidatePageWrite(hNTM, ulPage, "HiddenWrite"))
        goto HiddenWriteCleanup;

    /*  Encode the tags into the spare area buffer. The simulation mechanism
        will handle merging the new data with the existing data.
    */
    DclMemSet(hNTM->pSpare, ERASED8, hNTM->NtmInfo.uSpareSize);
    if (hNTM->NtmInfo.fEdcParameterized)
    {
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
                eccParams.metadata_buffer.buffer = hNTM->pSpare;
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

            FfxNtmPECCBuildSpareArea(hNTM->hDev, &hNTM->NtmInfo, &eccParams, hNTM->pSpare, pTag, TRUE, hNTM->pChipInfo);
        }
        else
        {
            FfxNtmPECCTagEncode(&(hNTM->NtmInfo), hNTM->pSpare, pTag, FfxNtmHelpGetSpareAreaFormat(hNTM->pChipInfo));
        }
    }
    else
	{
        FfxNtmHelpTagEncode(&hNTM->pSpare[hNTM->uTagOffset], pTag);
	}

    /*  Write the spare to the simulation mechanism.
    */
    ioStat.ffxStat = NsMechanismWriteSpare(hNTM->hSim, ulPage, hNTM->pSpare);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        ioStat.ulCount++;

    FFXERRORINJECT_WRITE("HiddenWrite", ulPage, &ioStat);

  HiddenWriteCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-HiddenWrite() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
    FFXIOSTATUS     ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);
    D_UINT32        ulPage;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-BlockErase() Block=%lX\n", ulBlock));

    ulPage = ulBlock * hNTM->NtmInfo.uPagesPerBlock;

    if (!Validate(hNTM, ulPage, 1, "Erase"))
        goto BlockEraseCleanup;

    /*  Call into the Simulator Mechanism to erase a block.
    */
    ioStat.ffxStat = NsMechanismEraseBlock(hNTM->hSim, ulBlock);

    /*  Reset the last page written for the block just erased.
    */
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        hNTM->pauLastPageWritten[ulBlock] = NTSIM_LPW_NONE;
        ioStat.ulCount++;
    }

    FFXERRORINJECT_ERASE("BlockErase", ulPage, &ioStat);

  BlockEraseCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-BlockErase() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-GetPageStatus() Page=%lX\n", ulPage));

    DclAssert(hNTM);

    if (hNTM->NtmInfo.fEdcParameterized)
	{
        ioStat = FfxNtmPECCGetPageStatus(hNTM,
                                         &hNTM->NtmInfo,
                                         &ReadSpareArea,
                                         ulPage,
                                         hNTM->pChipInfo);
    }
    else
    {
        ioStat = FfxNtmHelpReadPageStatus(hNTM, &ReadSpareArea, ulPage, hNTM->pChipInfo);
	}

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTSIM-GetPageStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-GetBlockStatus() Block=%lX\n", ulBlock));

    DclAssert(hNTM);

    if (hNTM->NtmInfo.fEdcParameterized)
        ioStat = FfxNtmPECCIsBadBlock(hNTM, &hNTM->NtmInfo, &ReadSpareArea, hNTM->pChipInfo, ulBlock);
    else
        ioStat = FfxNtmHelpIsBadBlock(hNTM, &ReadSpareArea, hNTM->pChipInfo, ulBlock);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTSIM-GetBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-SetBlockStatus() Block=%lX BlockStat=%lX\n", ulBlock,
        ulBlockStatus));

    DclAssert(hNTM);

    /*  For now, the only thing this function knows how to do is mark
        a block bad -- fail if anything else is requested.
    */
    if(ulBlockStatus & BLOCKSTATUS_MASKTYPE)
    {
        if (hNTM->NtmInfo.fEdcParameterized)
        {
            ioStat = FfxNtmPECCSetBlockType(hNTM, &hNTM->NtmInfo, &ReadSpareArea, &WriteSpareArea,
                hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
        }
        else
        {
            ioStat = FfxNtmHelpSetBlockType(hNTM, &ReadSpareArea, &WriteSpareArea,
            hNTM->pChipInfo, ulBlock, ulBlockStatus & BLOCKSTATUS_MASKTYPE);
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEUNDENT),
        "NTSIM-SetBlockStatus() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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
        "NTSIM-ParameterGet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
        hNTM, id, pBuffer, ulBuffLen));

    switch(id)
    {
        case FFXPARAM_NTM_SIMCHECK:
            ffxStat = FFXSTAT_SUCCESS;
            break;

        default:
            /*  Not a parameter ID we recognize, and no place else to pass it
            */
            FFXPRINTF(1, ("NTSIM:ParameterGet() unhandled parameter ID=%x\n", id));
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "NTSIM-ParameterGet() returning status=%lX\n", ffxStat));

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
        "NTSIM-ParameterSet() hNTM=%P ID=%x pBuff=%P Len=%lU\n",
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
        "NTSIM-ParameterSet() returning status=%lX\n", ffxStat));

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

            FFXPRINTF(2, (NTMNAME"-FXIOFUNC_FIM_LOCK_BLOCKS() StartBlock=%lU Count-%lU (STUBBED!)\n",
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

            FFXPRINTF(2, (NTMNAME"-FXIOFUNC_FIM_UNLOCK_BLOCKS() StartBlock=%lU Count-%lU Invert=%u (STUBBED!)\n",
                pReq->ulStartBlock, pReq->ulBlockCount, pReq->fInvert));

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

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    D_BUFFER           *pSpare)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-ReadSpareArea() Page=%lX\n", ulPage));

    DclProfilerEnter("NTSIM-ReadSpareArea", 0, 1);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "ReadSpareArea"))
        goto SpareReadCleanup;

    /*  Read the spare area from the simulation mechanism.
    */
    ioStat.ffxStat = NsMechanismReadSpare(hNTM->hSim, ulPage, pSpare);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        ioStat.ulCount++;

  SpareReadCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-ReadSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WriteSpareArea(
    NTMHANDLE           hNTM,
    D_UINT32            ulPage,
    const D_BUFFER     *pSpare)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMRANGEINVALID);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 1, TRACEINDENT),
        "NTSIM-WriteSpareArea() Page=%lX\n", ulPage));

    DclProfilerEnter("NTSIM-WriteSpareArea", 0, 1);

    /*  Check that this request is within the bounds of the flash.
    */
    if(!Validate(hNTM, ulPage, 1, "WriteSpareArea"))
        goto SpareWriteCleanup;

    /*  Check for out of order writes and rewrites.
    */
    if(!ValidatePageWrite(hNTM, ulPage, "WriteSpareArea"))
        goto SpareWriteCleanup;

    /*  Write the spare area to the simulation mechanism.
    */
    ioStat.ffxStat = NsMechanismWriteSpare(hNTM->hSim, ulPage, pSpare);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        ioStat.ulCount = 1;
    }

  SpareWriteCleanup:

    DclProfilerLeave(0);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "NTSIM-WriteSpareArea() returning %s\n", FfxDecodeIOStatus(&ioStat)));

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

    ulTotalPages = hNTM->ulRawTotalBlocks * hNTM->NtmInfo.uPagesPerBlock;

    if (ulStartPage > ulTotalPages)
    {
        FFXPRINTF(1, ("NTSIM %s: start page %lX exceeds total number of pages %lX\n",
                      pszFunc, ulStartPage, hNTM->ulRawTotalBlocks * hNTM->NtmInfo.uPagesPerBlock));

        fResult = FALSE;
    }

    if (ulCount == 0)
    {
        FFXPRINTF(1, ("NTSIM %s: count is zero\n", pszFunc));

        fResult = FALSE;
    }

    if (ulCount > ulTotalPages - ulStartPage)
    {
        FFXPRINTF(1, ("NTSIM %s: start page %lX plus count %lX exceeds total pages %lX\n",
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
    ulMemSize = hNTM->ulRawTotalBlocks * sizeof(*hNTM->pauLastPageWritten);
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
        uLastPageWritten = NTSIM_LPW_NONE;
    else
        uLastPageWritten = NTSIM_LPW_UNKNOWN;
    for(i = 0; i < hNTM->ulRawTotalBlocks; i++)
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
    if(hNTM->pauLastPageWritten[ulBlock] == NTSIM_LPW_UNKNOWN)
    {
        /*  The last page written for this block is unknown.
            Set the now known last page written and get out.
        */
        hNTM->pauLastPageWritten[ulBlock] = uPageInBlock;
        return TRUE;
    }
    else if(hNTM->pauLastPageWritten[ulBlock] == NTSIM_LPW_NONE)
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
        FFXPRINTF(2, ("NTSIM: WARNING! %s() out of sequence write to Block %lU at Page=%U\n",
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
        FFXPRINTF(2, ("NTSIM: WARNING! %s() page write skipped Block %lU at Page %U, expected Page %U\n",
                  pszFunc, ulBlock, uPageInBlock, uExpectedPage));

        /*  We skipped some pages for some reason -- nevertheless, record
            the page...
        */
        hNTM->pauLastPageWritten[ulBlock] = uPageInBlock;
    }

    return TRUE;
}



#endif  /* FFXCONF_NANDSUPPORT */

