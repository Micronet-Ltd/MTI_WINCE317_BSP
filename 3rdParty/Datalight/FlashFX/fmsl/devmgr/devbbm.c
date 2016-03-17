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

    This module implements the DeviceManager portion of FlashFX, which
    manages BBM instances.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devbbm.c $
    Revision 1.64  2010/12/08 06:17:27Z  glenns
    Fix bug 3288- use parameters from existing BBM format to adjust
    BBM properties.
    Revision 1.63  2010/09/28 19:24:45Z  garyp
    Commented out some unused code.  Cleaned up some debug messages
    and code formatting.  No functional changes.
    Revision 1.62  2010/09/24 22:53:49Z  glenns
    Fix arithmetic error when remapping a factory bad block in multi-
    chip configurations.
    Revision 1.61  2010/07/07 18:36:41Z  garyp
    Corrected an assert to accommodate an unwise status code.
    Revision 1.60  2010/05/19 19:41:19Z  garyp
    Fixed (again) to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.59  2010/04/30 01:22:13Z  garyp
    Removed some unused code to allow things to build cleanly with the
    TICCS ToolSet abstraction.
    Revision 1.58  2010/04/13 17:29:00Z  glenns
    Fix spelling in previous checkin comment.
    Revision 1.57  2010/04/13 17:27:56Z  glenns
    Fix spelling of FFXSTAT_DEVMGR_UNKNOWNBLOCKSTATUS.
    Revision 1.56  2009/10/13 21:08:09Z  garyp
    Updated DevBbmDoFormatCheck() to break out of the chip mount
    loop early if a mount fails, and to preserve the original status code.
    Updated debugging messages.
    Revision 1.55  2009/08/04 18:49:33Z  garyp
    Merged from the v4.0 branch.  Properly mask the block status before 
    examining the block type bits.  Fixed a misleading status code.
    Revision 1.54  2009/04/22 15:27:32Z  keithg
    corrected cut-and-paste error in the function header for FfxDevBbmRemap()
    Revision 1.53  2009/04/19 23:24:54Z  keithg
    Fixed bug 2640 - removed an erroneous comparason between the number of
    blocks managed by BBM to that managed by the Device Manager.
    Revision 1.52  2009/04/08 21:42:16Z  keithg
    Changed include of bbm_public to fxbbm.h; Added asserts to catch
    calls into the FIM that may violate reserved space.  Corrected calls
    into the error manager to use FIM device relative addresses rather
    than the BBM block and page addresses.
    Revision 1.51  2009/04/03 20:17:36Z  davidh
    Function header error corrected for AutoDoc.
    Revision 1.50  2009/04/01 17:15:28Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.49  2009/03/31 22:43:49Z  keithg
    Corrected problems with FORMAT_NEVER and FORMAT_ALWAYS
    which were introduced in v1.47 changes.
    Revision 1.48  2009/03/31 17:51:07Z  keithg
    Fixed bug 2570, warnings of uninitalized variable.
    Revision 1.47  2009/03/31 17:39:51Z  keithg
    - Added check to allow for unconditional mounts for development
      and testing situations.
    - Added comments to alert user to formats and unconditional mounts.
    Revision 1.46  2009/03/30 22:35:48Z  keithg
    - Eliminated resource leak in error condition path of DevBbmCreate()
    - Corrected handling of reserved space where the resulting total size
      was less than a single flash chip.
    - Changed behavior to fail the mount in the case of an unformat or
      any other kind of format or mount failure.
    Revision 1.45  2009/03/25 01:08:09Z  keithg
    Updated to handle cases where the reserved space encompassed
    exactly one flash chip at the end or start of the array.  Fixed to better
    handle cases wherethe BBM is enabled, but not being used.
    Revision 1.44  2009/03/19 21:59:42Z  keithg
    Fixed bug 2520 - total blocks is now properly calculated in multi-chip
    arrays with reserved space on the first chip.
    Revision 1.43  2009/03/18 05:40:06Z  glenns
    - Fix Bugzilla #2462: Repair erroneous assertion failure.
    - Fix Bugzilla #2370: Remove reference to obsolete block status
      indication BLOCKSTATUS_LEGACYFACTORYBAD.
    Revision 1.42  2009/03/13 19:44:48Z  glenns
    - Fix Bugzilla #2354: Add calls to the error manager when FIM
      IORequest functions return error codes. Recommendations are
      not used in all cases, but error manager should be informed of
      all FIM errors.
    - Fix Bugzilla #2354: Add calls to the Dev Mgr event handler for
      reads, writes and erases.
    - Fix small issue where the result of building the bad block list was
      not being examined and possible problems handled correctly.
    Revision 1.41  2009/03/04 06:35:29Z  keithg
    Updated block copy to use raw read/write in the case where it can
    not be determined if the page was written with EDC. (bug 2409)
    Revision 1.40  2009/02/27 21:43:54Z  billr
    Declare local functions static.  Eliminate use of C99 feature (array
    dimension not constant).  Clarify comments.
    Revision 1.39  2009/02/26 16:41:47Z  billr
    Resolve bug 2334: BBM ignores read errors during block replacement.
    Partially resolve bug 2354: Device Manager (DEVBBM) does not report
    errors.  Resolve bug 2409: Block Replacement does not preserve page
    status and EDC information.
    Revision 1.38  2009/02/19 04:44:30Z  keithg
    - Fixed bug 2446, the spare block percentage is no longer scaled twice.
    - Fixed bug 2445, block replacements now properly handles the remap
      of a failing reserved block.
    Revision 1.37  2009/02/18 03:56:49Z  keithg
    Removed use of BBMSETTINGS from DevEraseGoodBlocks() as it
    is no longer used by the current implementaiton.
    Revision 1.36  2009/02/18 03:21:10Z  keithg
    - Corrected Bug 2420, access to the disk when the BBM is not mounted
      and the access ocurrs in what would otherwise be the reserved blocks,
      was failing.  Mount now properly describes the disk upon failed mount.
    - Corrected comments regarding the commit of a remap for a failed
      block replacement.
    - Corrected behavior of forced format conditions when the BBM was
      pre-existing.  A mount with format always was completing but reporting
      failure.
    Revision 1.35  2009/02/12 22:34:06Z  keithg
    - Changes to fix Bug 2394 to move reserved space handling in the
    device manager.
    - Corrected logic to determine if a block replacemet was successful.
    Revision 1.34  2009/02/12 07:51:54Z  keithg
    Changes to the mount to handle  cases where reserved space results
    in too few blocks within a chip to place a BBM.  The create will now
    adjust any reserved space to accomodate for the unusable areas.
    Revision 1.33  2009/02/09 23:46:31Z  glenns
    - Fix up last checkin not to use hard-coded array subscripts.
    Revision 1.32  2009/02/09 05:17:17Z  glenns
    - Added code to DevBBMRead and DevBBMWrite to cause these
      functions to use EDC.
    Revision 1.31  2009/02/08 20:59:42Z  garyp
    Minor status code updates.
    Revision 1.30  2009/02/07 08:20:26Z  keithg
    Resolved bug 2359 - where the block info was returning unknown;
    and corrected an end of media boundary condition.
    Revision 1.29  2009/02/06 00:09:10  keithg
    Bug: 2368 Updated to correctly accomodate multi-chip configurations
    where some space is reserved from the start or end of the array.
    Revision 1.28  2009/02/03 16:39:37  keithg
    Corrected condition that prevented bad blocks from being properly detected.
    Revision 1.27  2009/01/29 15:12:51  keithg
    Now operates when the BBM has been disabled.
    Revision 1.26  2009/01/29 14:17:15  keithg
    Corrected spelling error.
    Revision 1.25  2009/01/29 14:02:14  keithg
    Corrected assert in block remapping.
    Revision 1.24  2009/01/18 00:43:26  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.23  2009/01/12 19:36:09Z  michaelm
    changed assert in BbmRelocateBlock to accept ulPageCount == uPagesPerBlock
    as valid
    Revision 1.22  2009/01/07 09:09:41Z  keithg
    Removed unnecessary type casts.
    Revision 1.21  2008/12/31 08:54:15Z  keithg
    Corrected prototype changes missed in the last revision.  Removed
    use of the BBM hooks structure.
    Revision 1.20  2008/12/25 06:19:22Z  keithg
    Added debug level 1 output to describe block relocate operations.
    Revision 1.19  2008/12/24 05:14:27Z  keithg
    Corrected asserts in DevBbmRelocateBlock() to validate parameters
    against the instance's media rather than the physical chip.  They were
    incorrectly asserting on media with reserved space.
    Revision 1.18  2008/12/23 07:16:03Z  keithg
    Added support for reusing factory bad blocks; Filled out function header
    comments; Clarified variable structure names; Removed multiple stack
    allocations of the BBM format settings and now pass by reference.
    Revision 1.17  2008/12/23 01:19:03Z  keithg
    Fixed debug macro closed paren.
    Revision 1.16  2008/12/23 01:12:49Z  keithg
    Added debug messages to clarify when a BBM does not fit or is not used.
    Revision 1.15  2008/12/23 00:46:32Z  keithg
    Corrected handling for FIM devices that have reserved spare at the
    start or which result in too few blocks for use as a disk.
    Revision 1.14  2008/12/22 08:40:30Z  keithg
    Corrected spelling errors and incorrect comments; Better handles
    unformatted media now; Removed 'reformat' code;  UNFORMAT
    and ALWAYS format operations now properly work.
    Revision 1.13  2008/12/19 05:12:53Z  keithg
    Added support for querying and compilation of BBM statistics.
    Revision 1.12  2008/12/18 07:28:51Z  keithg
    Updated the BbmMedia_t interface to use an opaque pointer to
    pass a context between the BBM and the user of the BBM.
    Revision 1.11  2008/12/18 05:04:49Z  keithg
    Changed function names for clarity; Fixed QueryRawBlock to properly
    accomodate requests in the BBM system area of the chip.
    Revision 1.10  2008/12/17 00:27:49Z  keithg
    Removed duplicated typedef - only needed structure declaration.
    Revision 1.9  2008/12/15 19:20:23Z  keithg
    Conditioned code on BBM_SUPPORT
    Revision 1.8  2008/12/12 07:34:44Z  keithg
    Now uses the BBM block info API/structure unconditionally;  Now uses
    a single page buffer provided to the BBM instances; Corrected handling
    of reformating and clarified handling of mounts during format.
    Revision 1.7  2008/12/09 08:03:37Z  keithg
    Added unconditional initializer for ulPagesCopied - it was set correctly
    under all conditions.
    Revision 1.6  2008/12/09 07:01:30Z  keithg
    Clarified several variable names; corrected handling of the first and last
    physical chips' reserved blocks and data blocks; now handles uninitialized
    BBM reserved block ratings.
    Revision 1.5  2008/12/04 00:01:19Z  keithg
    Changed DevBbmEraseBlock to poll from the FIM rather than the
    device manager;  Rewrote block relocation to handle raw blocks
    and use BBM to determine current mapping.
    Revision 1.4  2008/12/02 23:04:19Z  keithg
    Added block replacement functionality.
    Revision 1.3  2008/11/19 23:46:47Z  keithg
    Moved use of FFXCONF_BBM_xxx after includes.
    Revision 1.2  2008/10/24 04:39:32Z  keithg
    Conditioned code on USE_V5 and moved BBM list to include file.
    Revision 1.1  2008/10/23 23:38:12Z  keithg
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include <fxiosys.h>
#include <fxbbm.h>
#include "devmgr.h"
#include "devbbm.h"

#if FFXCONF_BBMSUPPORT

struct tagBbmDisk_s
{
    BBMHANDLE       hbbm;
    BbmMedia_t      media;
    BbmMount_t      mount;
    D_BOOL          fMounted;
    D_UINT32        ulBlockOffset;
    FFXFIMDEVICE    *pfd;
};


static FFXSTATUS DevBbmDoFormatCheck(
    FFXFIMDEVICE *pfd,
    FFXBBMSETTINGS *pBbmSettings);
static FFXSTATUS DevBbmBuildBadBlockList(
    FFXFIMDEVICE *pfd,
    FFXBBMSETTINGS *pBbmSettings,
    D_UINT32 ulChip,
    D_UINT32 *rgulBadBlockList,
    D_UINT32 *pulBadBlockCount);
static FFXSTATUS DevEraseGoodBlocks(
    FFXFIMDEVICE *pfd,
    D_UINT32 ulStartBlock,
    D_UINT32 ulBlockCount);
static FFXIOSTATUS CopyBlock(
    BbmDisk_t *pbbmdisk,
    D_UINT32 ulFrom,
    D_UINT32 ulTo,
    D_UINT32 ulPageCount);
static FFXIOSTATUS CopyPage(
    FFXFIMDEVICE *pfd,
    D_UINT32 ulFromPage,
    D_UINT32 ulToPage);
static FFXIOSTATUS DevBbmRead(
    BbmDisk_t *pcontext,
    D_UINT32 ulPage,
    D_UINT8 *pbuf);
static FFXIOSTATUS DevBbmWrite(
    BbmDisk_t *pcontext,
    D_UINT32 ulPage,
    D_UINT8 *pbuf);
static FFXIOSTATUS DevBbmErase(
    BbmDisk_t *pcontext,
    D_UINT32 ulBlock);



/*-------------------------------------------------------------------
    Protected: FfxDevBbmDestroy()

    Destroys the BBM instances associated with a given FIM device.
    Structure elements and allocated memory is freed by this call.

    The destroy method is *not* thread safe.
    This function is used exclusively by the device manager.

    Parameters:
        pfd - pointer to the FFXFIMDEVICE holding the BBM information.

    Return Value:
        FFXSTAT_SUCCESS once the BBM instance and state
        information are cleared.

    See Also:
        FfxDevBbmCreate
-------------------------------------------------------------------*/
FFXSTATUS FfxDevBbmDestroy(
    FFXFIMDEVICE *pfd)
{
    BbmDisk_t *rgBbmList;
    D_UINT32 ulChip;

    /* Free each allocated BBM instance */
    if(pfd->pBbm)
    {
        rgBbmList = pfd->pBbm;
        for(ulChip = 0; ulChip < pfd->ulBbmChips; ++ulChip)
        {
            if(rgBbmList[ulChip].hbbm)
            {
                FfxBbm5Destroy(&rgBbmList[ulChip].hbbm);
            }
            rgBbmList[ulChip].hbbm = NULL;
        }

        DclMemFree(pfd->pBbm);
    }

    /* Invalidate our FIMDEVICE parameters */
    pfd->pBbm = NULL;
    pfd->ulBbmChips = 0;
    pfd->ulBbmDataBlocks = 0;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Protected: FfxDevBbmCreate()

    Creates the BBM instance information and initializes BBM related
    state information for the given FIM device.  BBM specific
    parameters are obtained by querying the FFXOPT_DEVICE_BBMSETTINGS
    options.  BBM instances will be formatted on the flash chips if
    specified by the options.

    The create method is *not* thread safe.
    This function is used exclusively by the device manager.

    Parameters:
        pfd - pointer to the FFXFIMDEVICE holding the BBM information.

    Return Value:
        Returns an FFXSTAT_SUCCESS once the BBM instance and state
        information are successfully initialized.

    See Also:
        FfxDevBbmDestroy
-------------------------------------------------------------------*/
FFXSTATUS FfxDevBbmCreate(
    FFXFIMDEVICE   *pfd)
{
    FFXSTATUS       status;
    D_UINT32        ulChip;
    const FIMINFO  *pfi;
    BbmDisk_t      *rgBbmList;
    D_UINT32        ulBlocks;
    D_UINT32        ulFirstChip;
    D_UINT32        ulLastChip;
    FFXBBMSETTINGS  BbmSettings;

    DclAssert(pfd);
    DclAssert(pfd->pFimInfo);
    DclAssert(pfd->pFimInfo->ulChipBlocks);

    pfd->pBbm = NULL;
    pfi = pfd->pFimInfo;
    status = FFXSTAT_INTERNAL_ERROR;

    /*  Make sure the BBM is supposed to be enabled.
    */
    if(!FfxHookOptionGet(FFXOPT_DEVICE_BBMSETTINGS, pfd->hDev, &BbmSettings, sizeof(BbmSettings)))
    {
        FFXPRINTF(1, ("Warning: Could not query BBM settings, enabling BBM by default.\n"));
        FFXPRINTF(2, ("         The BBM format state is undefined, defaulting to 'ONCE'\n"));
        FFXPRINTF(2, ("         The BBM format type is undefined, defaulting to 'PRESERVE_BADBLOCKS'\n"));
        FFXPRINTF(2, ("         The BBM spare block percentage is undefined, defaulting to 100%% of 'FACTORY_RECOMMENDED'\n"));

        BbmSettings.fEnableBBM = TRUE;
        BbmSettings.nSparePercent = 100;
        BbmSettings.nFormatState = FFX_FORMAT_ONCE;
        BbmSettings.nFormatType = FFX_BBMFORMAT_PRESERVE_BADBLOCKS;
    }
    
    if(!BbmSettings.fEnableBBM)
    {
        status = FFXSTAT_SUCCESS;
        goto _CreateReturn;
    }

    /*  Determine and allocate the maximum number of chips the BBM has
        to handle.  This count includes any reserved space from the start
        of the array to simplify indexing and later calculations. The raw
        block count should have already been initialized by the caller.
    */
    DclAssert(pfd->ulRawBlocks == pfd->ulTotalBlocks);
    pfd->ulBbmChips  = (pfd->ulReservedBlocks + pfd->ulRawBlocks
                        + pfi->ulChipBlocks - 1) / pfi->ulChipBlocks;
    rgBbmList = DclMemAllocZero(pfd->ulBbmChips * sizeof(*rgBbmList));
    if(!rgBbmList)
    {
        status = FFXSTAT_OUTOFMEMORY;
        goto _CreateReturn;
    }
    
    pfd->pBbm = rgBbmList;

    /*  Setup our typical disk characteristics, this may change later
    */
    if(pfi->ulBBMReservedRating >= pfi->ulChipBlocks)
    {
        pfd->ulBbmReservedBlocks = (pfi->ulChipBlocks * 24) / 1024;

        FFXPRINTF(1, ("Warning: flash reserved rating is invalid (%lX), defaulting to %lU of %lU.\n", pfi->ulBBMReservedRating,
            pfd->ulBbmReservedBlocks, pfi->ulChipBlocks));
    }
    else
    {
        pfd->ulBbmReservedBlocks = pfi->ulBBMReservedRating;
    }

    if(BbmSettings.nSparePercent != FFX_USE_DEFAULT)
    {
        pfd->ulBbmReservedBlocks *= BbmSettings.nSparePercent;
        pfd->ulBbmReservedBlocks  /= 100;
    }

    pfd->ulBbmReservedBlocks += BBM_SYSTEM_BLOCKS;
    pfd->ulBbmDataBlocks = pfi->ulChipBlocks - pfd->ulBbmReservedBlocks;

    DclAssert(pfd->ulBbmReservedBlocks < pfi->ulChipBlocks);
    DclAssert(pfd->ulBbmDataBlocks < pfi->ulChipBlocks);

    /*  All flash chips require a BBM that fulfills the entire flash chip
        reserved block requirements as modified by the OEM above.  If the
        device configuration  reserved blocks from the start of the array
        results in a BBM disk that has  too few blocks for a  BBM, adjust
        the reserved blocks up or total blocks down.
    */
    ulBlocks = pfi->ulChipBlocks - (pfd->ulReservedBlocks % pfi->ulChipBlocks);
    if(ulBlocks <= pfd->ulBbmReservedBlocks)
    {
        pfd->ulReservedBlocks += ulBlocks;
        pfd->ulTotalBlocks -= ulBlocks;
        pfd->ulRawBlocks = pfd->ulTotalBlocks;
    }

    ulBlocks = (pfd->ulTotalBlocks + pfd->ulReservedBlocks) % pfi->ulChipBlocks;
    if(ulBlocks && (ulBlocks <= pfd->ulBbmReservedBlocks))
    {
        pfd->ulTotalBlocks -= ulBlocks;
        pfd->ulRawBlocks -= ulBlocks;
    }

    /*  Determine what the first and last chip numbers where the BBM
        is expected to reside.  Note the chip numbers are zero based
        and the total blocks and reserved blocks are counts.
    */
    ulFirstChip = pfd->ulReservedBlocks / pfi->ulChipBlocks;
    ulLastChip = ((pfd->ulTotalBlocks + pfd->ulReservedBlocks - 1) / pfi->ulChipBlocks);

    /*  Initialize each of our BBM control structures
    */
    for(ulChip=0; ulChip < pfd->ulBbmChips; ++ulChip)
    {
        /*  Init the general fields for each flash chip.  The page buffer
            is maintained by the device manager as part of the BBM
            elements.  The use of the same buffer for each BBM instance
            works because a single FIMDEVICE is *cannot* be reentered.
        */
        rgBbmList[ulChip].pfd = pfd;
        rgBbmList[ulChip].fMounted = FALSE;

        rgBbmList[ulChip].media.ulPageSize = pfi->uPageSize;
        rgBbmList[ulChip].media.ulBlockSize = pfi->ulBlockSize;
        rgBbmList[ulChip].media.ulTotalBlocks = pfi->ulChipBlocks;
        rgBbmList[ulChip].media.rgbBuffer = pfd->rgbPageBuffer;
        rgBbmList[ulChip].media.ReadPage = DevBbmRead;
        rgBbmList[ulChip].media.WritePage = DevBbmWrite;
        rgBbmList[ulChip].media.EraseBlock = DevBbmErase;
        rgBbmList[ulChip].media.ioContext = (void *) &rgBbmList[ulChip];
        rgBbmList[ulChip].ulBlockOffset = ulChip * pfi->ulChipBlocks;

        if(ulChip < ulFirstChip)
        {
            /*  Disable the instance if there is no usable space.  Note this may
                seem wasteful, however it is possible that multiple blocks are
                bad including the reserved area.
            */
            rgBbmList[ulChip].media.ioContext = NULL;
            rgBbmList[ulChip].ulBlockOffset = D_UINT32_MAX;
            rgBbmList[ulChip].media.ulTotalBlocks = D_UINT32_MAX;
            continue;
        }
        else if(ulChip == ulLastChip)
        {
            /*  The first chip may also be the last chip.  This check needs
                to be completed prior to checking for the first chip condition.
            */
            rgBbmList[ulChip].media.ulTotalBlocks =
                (pfd->ulTotalBlocks + pfd->ulReservedBlocks)
                    - (rgBbmList[ulChip].ulBlockOffset);
        }
        else if(ulChip == ulFirstChip)
        {
            rgBbmList[ulChip].media.ulTotalBlocks = DCLMIN(
                pfd->ulTotalBlocks + pfd->ulReservedBlocks, pfi->ulChipBlocks);
        }
        else if(ulChip < ulLastChip)
        {
            rgBbmList[ulChip].media.ulTotalBlocks = pfi->ulChipBlocks;
        }

        /* Create this BBM instance */
        status = FfxBbm5Create(&rgBbmList[ulChip].media, &rgBbmList[ulChip].hbbm);
        if(status != FFXSTAT_SUCCESS)
        {
            goto _CreateReturn;
        }

        rgBbmList[ulChip].fMounted = FALSE;
    }

    /*  Success, record the BBM information and format if needed
    */
    status = DevBbmDoFormatCheck(pfd, &BbmSettings);

    if(status == FFXSTAT_SUCCESS)
    {
        /*  Correct the total blocks to account for the BBM reserved blocks
            Note it is reasonable that the size of the raw blocks and total
            blocks will be the same if the BBM is not formatted or should
            not be formatted.
        */
        DclAssert(pfd->ulTotalBlocks == pfd->ulRawBlocks);
        DclAssert(pfd->ulTotalPages == pfd->ulRawTotalPages);
        DclAssert(pfd->ulRawTotalPages >= pfd->ulTotalPages);
        for(ulChip=0; ulChip < pfd->ulBbmChips; ++ulChip)
        {
            if(rgBbmList[ulChip].fMounted)
            {
                pfd->ulTotalBlocks -= rgBbmList[ulChip].mount.ulReservedBlocks;
            }
        }
        pfd->ulTotalPages = pfd->ulTotalBlocks * pfi->uPagesPerBlock;
    }
    else
    {
        /*  Allow the system to continue without a BBM.
        */
        FFXPRINTF(1, ("Warning: BBM is not placed into the media, Status=%lX\n", status));
        pfd->ulBbmReservedBlocks = 0;
        pfd->ulBbmDataBlocks = pfi->ulChipBlocks;
    }

  _CreateReturn:
    if(status != FFXSTAT_SUCCESS)
    {
        FfxDevBbmDestroy(pfd);
    }

    /*  Allow the developer to force successful mounts of the media.
    */
    if((status != FFXSTAT_SUCCESS) && BbmSettings.fAlwaysMount)
    {
        FFXPRINTF(1, ("Warning: Reporting successful create on BBM status of %lX\n", status));

        status = FFXSTAT_SUCCESS;
    }

    return status;
}


#if 0
NOT USED ANYWHERE

/*-------------------------------------------------------------------
    Protected: FfxDevBbmMapBlockToRaw()

    Calculates the raw block address of a given FIM device relative
    block address.  Note the given block address must have any disk
    mappings or reserved space accounted prior to this mapping.

    This function is used by the device manager and FML.

    Parameters:
        pfd      - pointer to the FFXFIMDEVICE holding the BBM information.
        pulBlock - FIM device relative block offset.

    Return Value:
        Returns an FFXSTAT_SUCCESS after updating the block address
        to a raw block address.
-------------------------------------------------------------------*/
void FfxDevBbmMapBlockToRaw(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulBlock)
{
    BbmDisk_t *pBbmList;
    D_UINT32 ulChip;
    D_UINT32 ulBlock;

    DclAssert(pfd);
    DclAssert(pulBlock);
    DclAssert(*pulBlock < pfd->ulTotalBlocks);

    /* Determine which chip the requested block resides in */
    ulBlock = *pulBlock;
    pBbmList = pfd->pBbm;
    if(pBbmList)
    {
        ulChip = *pulBlock / pfd->ulBbmDataBlocks;
        DclAssert(ulChip < pfd->ulBbmChips);

        if(pBbmList[ulChip].fMounted)
        {
            /* Offset within the physical chip */
            ulBlock = ulChip * pfd->pFimInfo->ulChipBlocks;
            ulBlock += *pulBlock % pfd->ulBbmDataBlocks;
            DclAssert(ulBlock < pBbmList[ulChip].ulBlockOffset
                    + pBbmList[ulChip].media.ulTotalBlocks);
        }

    }

    *pulBlock = ulBlock;

    return;
}
#endif


/*-------------------------------------------------------------------
    Protected: FfxDevBbmRemapPage()

    Determines if and where a block has been remapped by the BBM.
    Given a FIM device relative page number queries the BBM for any
    remapping information and returns the correct location of that
    page.

    This function is used exclusively by the device manager.

    Parameters:
        pfd      - pointer to the FFXFIMDEVICE holding the BBM information.
        pulPage  - FIM device relative page offset.
        pulCount - length of consecutive pages the caller intends to
                   access. Updated to fit within the remapped block.

    Return Value:
        Updates the given page number if the page has been remapped
        by the BBM.  The pulCount may be reduced to describe the
        number of pages that can be consecutively accessed before
        another remap.

        Returns an FFXSTAT_SUCCESS after updating the block address
        to a raw block address.
-------------------------------------------------------------------*/
void FfxDevBbmRemapPage(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulPage,
    D_UINT32 *pulCount)
{
    FFXSTATUS status;
    BbmDisk_t *pBbmList;
    D_UINT32 ulChip;
    D_UINT32 ulPage;
    D_UINT32 ulMaxCount;
    D_UINT32 ulDataPagesPerChip;

    DclAssert(pfd);
    DclAssert(pulPage);
    DclAssert(pfd->pFimInfo->uPagesPerBlock);

    /* Determine which instance is handling this page */
    pBbmList = pfd->pBbm;
    if(pBbmList)
    {
        DclAssert(pfd->ulBbmDataBlocks);

        ulDataPagesPerChip = pfd->ulBbmDataBlocks
                * pfd->pFimInfo->uPagesPerBlock;
        ulChip = *pulPage / ulDataPagesPerChip;

        /*  It is possible to have the BBM enabled, but no BBM format
            and in which case, the BBM list will not describe a fully
            valid BBM.  The additional checks here are necessary to
            ensure the 'chip' has a BBM instance associated with it.
        */
        if( (ulChip < pfd->ulBbmChips)
            && pBbmList[ulChip].hbbm
            && pBbmList[ulChip].fMounted)
        {
            ulPage = *pulPage % ulDataPagesPerChip;

            /*  Limit the count to fit within the block */
            ulMaxCount = pfd->pFimInfo->uPagesPerBlock;
            ulMaxCount -= ulPage % pfd->pFimInfo->uPagesPerBlock;
            if(ulMaxCount < *pulCount)
            {
                *pulCount = ulMaxCount;
            }

            status = FfxBbm5MapPage(pBbmList[ulChip].hbbm, &ulPage);

            *pulPage = ulPage + (pBbmList[ulChip].ulBlockOffset
                        * pfd->pFimInfo->uPagesPerBlock);

            if(status != FFXSTAT_SUCCESS)
            {
                FFXPRINTF(1, ("Warning: FfxDevBbmRemapPage() failure "
                    "(Array Page: %lU, Chip: %lU, Chip Page: %lU\n",
                        *pulPage, ulChip, ulPage));
            }
        }
    }
    return;
}


/*-------------------------------------------------------------------
    Protected: FfxDevBbmRemap()

    Determines if and where a block has been remapped by the BBM.
    Given a FIM device relative block number, queries the BBM for any
    remapping information and returns the correct location of that
    block.

    This function is used exclusively by the device manager.

    Parameters:
        pfd      - pointer to the FFXFIMDEVICE holding the BBM information.
        pulBlock - FIM device relative block offset.
        pulCount - length of consecutive blocks the caller intends to
                   access. Updated to fit within the remapped block.

    Return Value:
        Updates the given block number if the block has been remapped
        by the BBM.  The pulCount is fixed (or reduced) to one due to
        blocks being the granularity of replacements.

        Returns an FFXSTAT_SUCCESS after updating the block address
        to a raw block address.
-------------------------------------------------------------------*/
void FfxDevBbmRemapBlock(
    FFXFIMDEVICE *pfd,
    D_UINT32 *pulBlock,
    D_UINT32 *pulCount)
{
    FFXSTATUS status;
    BbmDisk_t *pBbmList;
    D_UINT32 ulChip;
    D_UINT32 ulPage;
    D_UINT32 ulBlock;

    DclAssert(pfd);
    DclAssert(pulBlock);

    /*  Determine which instance is handling this block 
    */
    pBbmList = pfd->pBbm;
    if(pBbmList)
    {
        DclAssert(pfd->ulBbmDataBlocks);
        ulChip = *pulBlock / pfd->ulBbmDataBlocks;

        /*  It is possible to have the BBM enabled, but no BBM format
            and in which case, the BBM list will not describe a fully
            valid BBM.  The additional checks here are necessary to
            ensure the 'chip' has a BBM instance associated with it.
        */
        if( (ulChip < pfd->ulBbmChips)
            && pBbmList[ulChip].hbbm
            && pBbmList[ulChip].fMounted)
        {
            ulBlock = *pulBlock % pfd->ulBbmDataBlocks;
            ulPage = ulBlock * pfd->pFimInfo->uPagesPerBlock;

            status = FfxBbm5MapPage(pBbmList[ulChip].hbbm, &ulPage);
            DclAssert(ulPage % pfd->pFimInfo->uPagesPerBlock == 0);

            *pulBlock = (ulPage / pfd->pFimInfo->uPagesPerBlock) + pBbmList[ulChip].ulBlockOffset;
            *pulCount = 1;

            if(status != FFXSTAT_SUCCESS)
            {
                FFXPRINTF(1, ("Warning: FfxDevBbmMapBlockToRaw() failure (Array Page: %lU, Chip: %lU, Chip Page: %lU\n",
                    *pulBlock, ulChip, ulBlock));
                
                *pulCount = 0;
            }
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxDevBbmQueryBlock()

    Determines the type of block relative to the BBM and it's
    physical (raw) location within the FIM device.

    This function is used exclusively by the device manager.

    Parameters:
        pfd   - pointer to the FFXFIMDEVICE holding the BBM information.
        pinfo - pointer to the block information structure to fill in.

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxDevBbmQueryRawBlock(
    FFXFIMDEVICE *pfd,
    FFXIOR_DEV_GET_RAW_BLOCK_INFO *pinfo)
{
    BbmDisk_t *pBbmList;
    BbmRemap_t remap;
    D_UINT32 ulChip;
    D_UINT32 ulBlock;

    DclAssert(pfd);
    DclAssert(pinfo);
    DclAssert(pinfo->ulBlock < pfd->ulRawBlocks + pfd->ulReservedBlocks);

    /* Determine which instance is handling this page */
    pBbmList = pfd->pBbm;
    pinfo->ulRawMapping = pinfo->ulBlock;
    pinfo->ulBlockInfo = BLOCKINFO_UNKNOWN;

    if(pBbmList)
    {
        DclAssert(pfd->ulBbmDataBlocks);
        ulBlock = pinfo->ulBlock;
        ulChip = ulBlock / pfd->pFimInfo->ulChipBlocks;
        DclAssert(ulChip < pfd->ulBbmChips);
        DclAssert(ulBlock >= pBbmList[ulChip].ulBlockOffset);

        if(pBbmList[ulChip].fMounted)
        {
            FFXSTATUS status;
            
            /*  Get the BBM block information
            */
            ulBlock = ulBlock - pBbmList[ulChip].ulBlockOffset;
            status = FfxBbm5QueryBlock(pBbmList[ulChip].hbbm, ulBlock, &remap);

            /*  Should act on this status....
            */
            DclAssert(status == FFXSTAT_SUCCESS || status == FFXSTAT_BBM_RESERVED_BLOCK);
            (void)status;

            /*  The block type that we will pass back needs to be that of the raw
                data block if the address is in the region reserved for use as
                spares by the BBM.  Otherwise the request is for a data block and
                the type needs to be that of the actual block which has been
                remapped into the BBM reserved blocks.
            */
            switch(remap.data.type)
            {
                case BBM_BLOCK_DATA:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_GOODBLOCK;
                    break;
                }

                case BBM_BLOCK_FACTORY_BAD:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_FACTORYBAD;
                    break;
                }

                case BBM_BLOCK_RESERVED:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_RESERVED;
                    break;
                }

                case BBM_BLOCK_SYSTEM:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_BBMSYSTEM;
                    break;
                }

                case BBM_BLOCK_RETIRED:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_RETIRED;
                    break;
                }

                case BBM_BLOCK_TEMPORARY:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_TEMPORARY;
                    break;
                }

                default:
                {
                    pinfo->ulBlockInfo = BLOCKINFO_UNKNOWN;
                    break;
                }
            }
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Private: FfxDevBbmRelocateBlock()

    Allocates a reserved block from the BBM and moves contents from
    the given block to the allocated replacement block.  The new
    replacement block will be committed only if the erase and
    subsequent writes to the new block are completed without error.

    Any error encountered during the erase or transfer (writes) to
    the new block will abort the transfer operation and commit the
    allocated block as a retired block.  The iostatus causing the
    abort will be returned to the caller and the original block
    relocation will *not* be completed.

    Location of the newly allocated block can be determined with a
    subsequent request to map the original block or page.

     Parameters:
        pfd         - FIM device to complete the operation within
        ulFromBlock - Original physical block being replaced
        ulPageCount - Number of pages to copy to the new block.

    Return Value:
        Resulting FFXIOSTATUS of the replacement operation.
        FFXSTAT_DEVMGR_NOMORERESERVEDBLOCKS - if there are no
        remaining blocks for replacements.
 -------------------------------------------------------------------*/
FFXIOSTATUS FfxDevBbmRelocateBlock(
    FFXFIMDEVICE *pfd,
    D_UINT32 ulFromBlock,
    D_UINT32 ulPageCount)
{
    FFXIOSTATUS iostatus = DEFAULT_BLOCKIO_STATUS;
    BbmDisk_t *pBbmList;
    BbmRemap_t  remapTo;
    BbmRemap_t  remapFrom;

    D_UINT32 ulChip;

    DclAssert(pfd);
    DclAssert(ulPageCount <= pfd->pFimInfo->uPagesPerBlock);

    pBbmList = pfd->pBbm;
    if(!pBbmList)
    {
        iostatus.ffxStat = FFXSTAT_DEVREQUESTFAILED;
        return iostatus;
    }

    DclAssert(pfd->ulBbmDataBlocks);
    DclAssert(pfd->pFimInfo->ulChipBlocks);

    /*  Determine where the failure really is in terms of the physical
        chip and block.
    */
    ulChip = ulFromBlock / pfd->pFimInfo->ulChipBlocks;
    DclAssert(ulChip < pfd->ulBbmChips);
    DclAssert(ulFromBlock >= pBbmList[ulChip].ulBlockOffset);

    if(!pBbmList[ulChip].fMounted)
    {
        iostatus.ffxStat = FFXSTAT_DEVREQUESTFAILED;
        return iostatus;
    }

    /*  Determine what data block, if any, we are replacing...
    */
    ulFromBlock -= pBbmList[ulChip].ulBlockOffset;
    iostatus.ffxStat = FfxBbm5QueryBlock(pBbmList[ulChip].hbbm, ulFromBlock, &remapFrom);
    if (iostatus.ffxStat != FFXSTAT_SUCCESS && iostatus.ffxStat != FFXSTAT_BBM_RESERVED_BLOCK)
    {
        DclError();
        return iostatus;
    }

    /*  Loop, repeating the replacement until it succeeds, there are
        no replacement blocks left, or a fatal I/O error occurs.
    */
    for (;;)
    {
        /* Allocate a block from the BBM to replace the failing block
         */
        iostatus.ffxStat = FfxBbm5AcquireRemap(pBbmList[ulChip].hbbm, &remapTo);
        if(iostatus.ffxStat != FFXSTAT_SUCCESS)
        {
            FFXPRINTF(1, ("Warning: Failed to acquire a replacement block from chip: %lU\n",
                          ulChip));
            break;
        }
        DclAssert(remapTo.phys.key >= pBbmList[ulChip].mount.ulDataBlocks);
        DclAssert(remapTo.phys.key < pBbmList[ulChip].media.ulTotalBlocks);

        iostatus = CopyBlock(&pBbmList[ulChip], remapFrom.phys.key, remapTo.phys.key, ulPageCount);

        if (iostatus.ffxStat == FFXSTAT_SUCCESS)
        {
            /*  The copy succeeded, commit the remap.  If the commit
                fails, the entire replacement fails.
            */
            iostatus.ffxStat = FfxBbm5CommitRemap(pBbmList[ulChip].hbbm, &remapTo,
                                                  BBM_BLOCK_RETIRED, remapFrom.data.key);
            break;
        }
        else if (FfxErrMgrDecodeRecommendation(&iostatus) == FFXERRPOLICY_RETIREBLOCK)
        {
            /*  Presumably there was a program or erase failure in the
                selected replacement block.  Try to retire it and try
                again with another.  Ignore the status of the retire,
                what more can we do?
            */
            (void) FfxBbm5CommitRemap(pBbmList[ulChip].hbbm, &remapTo,
                                      BBM_BLOCK_RETIRED, remapTo.phys.key);
        }
        else
        {
            /*  The copy failed because of an unrecoverable error, so
                just release the remap that was acquired, and fail the
                replacement operation.  Ignore the status of the
                release, what more can we do?
            */
            (void) FfxBbm5ReleaseRemap(pBbmList[ulChip].hbbm, &remapTo);
            break;
        }
    }

#if D_DEBUG
    if(iostatus.ffxStat == FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("Successful relocate of block %lX on chip %lX to block %lX.\n",
            ulFromBlock, ulChip, remapTo.phys.key));
    }
    else
    {
        FFXPRINTF(1, ("Failed relocation of block %lX on chip %lX to block %lX.\n"
                      "Status: %s\n",
                    ulFromBlock, ulChip, remapTo.phys.key,
                    FfxDecodeIOStatus(&iostatus)));
    }
#endif

    return iostatus;

}


/*-------------------------------------------------------------------
    Local: CopyBlock()

    Copy one block during block relocation.  The destination block is
    first erased, then the specified number of pages (which may be
    zero) are copied from the original block to the destination block.

    Parameters:
        pbbmdisk    - the BBM disk within which the copy takes place
        ulFrom      - Physical key of the original block to copy from
        ulTo        - Physical key of the destination block to copy to
        ulPageCount - number of pages to copy, may be zero

    Return Value:
        The returned FFXIOSTATUS is the status of the last operation
        performed.  If any operation in the copy fails, this is the
        status of that particular operation, and has been processed by
        the Error Manager.  Note that correctable errors on read are
        replaced by FFXSTAT_SUCCESS.
 -------------------------------------------------------------------*/
static FFXIOSTATUS CopyBlock(
    BbmDisk_t *pbbmdisk,
    D_UINT32 ulFrom,
    D_UINT32 ulTo,
    D_UINT32 ulPageCount)
{
    FFXIOSTATUS iostatus;

    iostatus = DevBbmErase(pbbmdisk, ulTo);

    if (iostatus.ffxStat == FFXSTAT_SUCCESS)
    {
        FFXFIMDEVICE *pfd = pbbmdisk->pfd;
        D_UINT32 ulFromPage = (ulFrom + pbbmdisk->ulBlockOffset) * pfd->pFimInfo->uPagesPerBlock;
        D_UINT32 ulToPage = (ulTo + pbbmdisk->ulBlockOffset) * pfd->pFimInfo->uPagesPerBlock;
        unsigned i;

        for (i = 0; i < ulPageCount; ++i)
        {
            iostatus = CopyPage(pfd, ulFromPage, ulToPage);

            if (iostatus.ffxStat != FFXSTAT_SUCCESS)
                break;

            ++ulFromPage;
            ++ulToPage;
        }
    }

    return iostatus;
}


/*-------------------------------------------------------------------
    Local: CopyPage()

    Copy one page during block relocation.  Uses the FIM page status
    IOR to determine if the page appears written with EDC information
    and if it can be read successfully with ECC, it is simply written
    back with ECC.  If it has an uncorrectable error, it is read and
    written with the "native pages" functions.

    Other read errors and any write errors are processed by the Error
    Manager, and the resulting status (with recommendation) is
    returned.

    Parameters:
        pfd        - FIM device to complete the operation within
        ulFromPage - Physical page to copy from
        ulToPage   - Physical page to copy to

    Return Value:
        The returned FFXIOSTATUS is the status of the last operation
        performed.  If any operation in the copy fails, this is the
        status of that particular operation, and has been processed by
        the Error Manager.  Note that correctable errors on read are
        replaced by FFXSTAT_SUCCESS.
-------------------------------------------------------------------*/
static FFXIOSTATUS CopyPage(
    FFXFIMDEVICE *pfd,
    D_UINT32 ulFromPage,
    D_UINT32 ulToPage)
{
    FFXIOSTATUS iostatus;
    FFXIOR_FIM_READ_GENERIC fimRead = {{ FXIOFUNC_FIM_READ_GENERIC, sizeof fimRead }};
    FFXIOR_FIM_WRITE_GENERIC fimWrite = {{ FXIOFUNC_FIM_WRITE_GENERIC, sizeof fimWrite }};
    FFXIOR_FIM_GET_PAGE_STATUS fimPageStatus = {{ FXIOFUNC_FIM_GET_PAGE_STATUS, sizeof fimPageStatus }};
    D_UINT32 ulPage;        /* for possible Error Manager report */
    D_BOOL fReadWithEdc;

    /*  Determine how to read the from page: if the page was *not* written
        with EDC, complete a raw read.  Note this means the data may be
        read with errors.
    */
    fReadWithEdc = FALSE;
    fimPageStatus.ulPage = ulFromPage;
    iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &fimPageStatus.ior);
    if(iostatus.ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Go ahead and report this to the error manager for the error
            manager's purposes, but ignore the recommendation as there isn't
            much we can do about any error we get from this operation here
            anyway.
        */
        FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus,
                             ulFromPage / pfd->pFimInfo->uPagesPerBlock,
                             ulFromPage % pfd->pFimInfo->uPagesPerBlock);
        FfxErrMgrKillError(&iostatus);
    }
    else if ((iostatus.op.ulPageStatus & PAGESTATUS_MASKFLAGS)
            == PAGESTATUS_WRITTENWITHECC)
    {
        fReadWithEdc = TRUE;
    }

    fimWrite.ulStartPage = ulToPage;
    fimWrite.ulCount = 1;
    fimWrite.pPageData = pfd->rgbPageBuffer;
    fimWrite.pAltData = pfd->rgbPageBuffer + pfd->pFimInfo->uPageSize;

    fimRead.ulStartPage = ulFromPage;
    fimRead.ulCount = 1;
    fimRead.pPageData = pfd->rgbPageBuffer;
    fimRead.pAltData = pfd->rgbPageBuffer + pfd->pFimInfo->uPageSize;

    if(fReadWithEdc)
    {
        /* Note the following code assumes the tag size (bug 2466) */
        fimRead.nAltDataSize = FFX_NAND_TAGSIZE;
        fimRead.ioSubFunc = FXIOSUBFUNC_FIM_READ_TAGGEDPAGES;

        fimWrite.nAltDataSize = FFX_NAND_TAGSIZE;
        fimWrite.ioSubFunc = FXIOSUBFUNC_FIM_WRITE_TAGGEDPAGES;
    }
    else
    {
        fimRead.nAltDataSize = pfd->pFimInfo->uSpareSize;
        fimRead.ioSubFunc = FXIOSUBFUNC_FIM_READ_RAWPAGES;

        fimWrite.nAltDataSize = pfd->pFimInfo->uSpareSize;
        fimWrite.ioSubFunc = FXIOSUBFUNC_FIM_WRITE_RAWPAGES;
    }

    iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &fimRead.ior);
    ulPage = ulFromPage;

    /*  Ignore correctable errors, they are scrubbed in the copy.
    */
    if (iostatus.ffxStat == FFXSTAT_FIMCORRECTABLEDATA)
        FfxErrMgrKillError(&iostatus);

    if (iostatus.ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Write tagged page.
        */
        iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &fimWrite.ior);
        ulPage = ulToPage;
    }
    else if (iostatus.ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA)
    {
        /*  Read native page.
        */
        fimRead.ioSubFunc = FXIOSUBFUNC_FIM_READ_NATIVEPAGES;

        iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &fimRead.ior);

        if (iostatus.ffxStat == FFXSTAT_SUCCESS
            || iostatus.ffxStat == FFXSTAT_FIMUNCORRECTED)
        {
            /*  Write native page.
            */
            fimWrite.ioSubFunc = FXIOSUBFUNC_FIM_WRITE_NATIVEPAGES;

            iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &fimWrite.ior);
            ulPage = ulToPage;
        }
    }

    if (iostatus.ffxStat != FFXSTAT_SUCCESS)
    {
        FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus,
                             ulPage / pfd->pFimInfo->uPagesPerBlock,
                             ulPage % pfd->pFimInfo->uPagesPerBlock);
    }

    return iostatus;
}


/*-------------------------------------------------------------------
    Protected: DevBbm5StatsQuery()

    Compiles BBM specific statistics for the given FIM device.
    Note that any one FIM device may be composed of a number of
    physical chips each having a different BBM instance.

    Parameters:
        pfd       - FIM device to complete the operation within.
        pBbmStats - Pointer to the BBM statistics to compile.

    Return Value:
-------------------------------------------------------------------*/
#if FFXCONF_STATS_BBM
FFXSTATUS FfxDevBbmStatsQuery(
    FFXFIMDEVICE *pfd,
    FFXBBMSTATS *pBbmStats)
{
    BbmDisk_t *pBbmList;
    D_UINT32 ulChip;
    FFXBBMSTATS stats;
    FFXSTATUS status;

    DclAssert(pfd);
    DclAssert(pBbmStats);

    status = FFXSTAT_DEVREQUESTFAILED;
    pBbmList = pfd->pBbm;
    if(pBbmList)
    {
        /* Add up all the statistics for each BBM instance */
        DclMemSet(pBbmStats, 0x00, sizeof(*pBbmStats));
        for(ulChip=0; ulChip<pfd->ulBbmChips; ++ulChip)
        {
            if(pBbmList[ulChip].fMounted)
            {
                status = FfxBbm5StatsQuery(pBbmList[ulChip].hbbm, &stats);
                if(status == FFXSTAT_SUCCESS)
                {
                    pBbmStats->ulBbmInstances++;
                    pBbmStats->ulTotalBlocks      += stats.ulTotalBlocks;
                    pBbmStats->ulSpareBlocks      += stats.ulSpareBlocks;
                    pBbmStats->ulBbmOverhead      += stats.ulBbmOverhead;
                    pBbmStats->ulFactoryBadBlocks += stats.ulFactoryBadBlocks;
                    pBbmStats->ulRetiredBlocks    += stats.ulRetiredBlocks;
                    pBbmStats->ulTemporaryBlocks  += stats.ulTemporaryBlocks;
                    pBbmStats->ulFreeBlocks       += stats.ulFreeBlocks;

                    pBbmStats->ulMapCacheRequests += stats.ulMapCacheRequests;
                    pBbmStats->ulMapCacheHits     += stats.ulMapCacheHits;
                }
            }
        }

        /*  Complete a bit of a reality check.  These should not be out of bounds
        */
      #if D_DEBUG
        if(pBbmStats->ulBbmInstances != pfd->ulBbmChips)
        {
            FFXPRINTF(1, ("Warning: The number of BBM instances does not match the number of chips.\n"));
            status = FFXSTAT_DEVMGR_INFOFAILED;
        }
        if((pBbmStats->ulFactoryBadBlocks + pBbmStats->ulRetiredBlocks
                + pBbmStats->ulBbmOverhead + pBbmStats->ulTemporaryBlocks
                + pBbmStats->ulFreeBlocks) > pBbmStats->ulSpareBlocks)
        {
            FFXPRINTF(1, ("Warning: BBM statistics do not match the FIMDEVICE.\n"));
            status = FFXSTAT_DEVMGR_INFOFAILED;
        }
      #endif
    }

    return status;
}
#endif


/*-------------------------------------------------------------------
    Local: DevBbmDoFormatCheck()

    Queries format information for the BBM and formats the media
    if required.  After completion, all BBM state information is
    current and the media formatted if necessary.  The media will
    be mounted on completion of this call if a valid BBM exists or
    is successfully placed into the disk.

    Parameters:
        pfd - pointer to the FFXFIMDEVICE to check

    Return Value:
        Returns an FFXSTAT_SUCCESS after format check is completed
        or once the format operation is completed.
-------------------------------------------------------------------*/
static FFXSTATUS DevBbmDoFormatCheck(
    FFXFIMDEVICE *pfd,
    FFXBBMSETTINGS * pBbmSettings)
{
    FFXSTATUS status;
    BbmDisk_t *rgBbmList;
    D_UINT32 ulChip;
    D_UINT32 ulChipCount;
    D_BOOL   fFailedMount;

    D_UINT32 ulReservedCount;
    D_UINT32 ulBadBlockCount;
    D_UINT32 *rgulBadBlockList;

    rgBbmList = NULL;
    rgulBadBlockList = NULL;
    status = FFXSTAT_INTERNAL_ERROR; /* To placate picky compilers */

    DclAssert((pBbmSettings->nFormatState == FFX_FORMAT_NEVER)    ||
              (pBbmSettings->nFormatState == FFX_FORMAT_ONCE)     ||
              (pBbmSettings->nFormatState == FFX_FORMAT_ALWAYS)   ||
              (pBbmSettings->nFormatState == FFX_FORMAT_UNFORMAT));

    DclAssert((pBbmSettings->nFormatType == FFX_BBMFORMAT_PRESERVE_BADBLOCKS) ||
              (pBbmSettings->nFormatType == FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS));

    /* Get a pointer to our list of BBM entries */
    DclAssert(pfd->pBbm);
    rgBbmList = pfd->pBbm;
    ulChipCount = pfd->ulBbmChips;

    /*  Handle the simple cases first...
    */
    if(pBbmSettings->nFormatState == FFX_FORMAT_UNFORMAT)
    {
        /* Unformat = just erase everything except factory bad blocks */
        FFXPRINTF(1, ("Warning: the media will be unmountable after this operation.\n"));
        FFXPRINTF(1, ("Completing an unformat of the entire array, please wait...\n"));

        status = DevEraseGoodBlocks(pfd, pfd->ulReservedBlocks, pfd->ulRawBlocks);
        status = FFXSTAT_BBM_NOT_FORMATTED;  /* Force failure */
        goto _FormatCheckComplete;
    }

    /*  Initialize all our instance mount structures.  The actual mount is
        conditional at this point since we may have to format regardless of
        the mount results.
    */
    fFailedMount = FALSE;
    for(ulChip = 0; ulChip < ulChipCount; ++ulChip)
    {
        /* Only mount chips we are managing */
        if(rgBbmList[ulChip].hbbm == NULL)
        {
            continue;
        }

        rgBbmList[ulChip].mount.ulDataBlocks
            = rgBbmList[ulChip].media.ulTotalBlocks - pfd->ulBbmReservedBlocks;
        rgBbmList[ulChip].mount.ulReservedBlocks = pfd->ulBbmReservedBlocks;
        rgBbmList[ulChip].fMounted = FALSE;

        if(pBbmSettings->nFormatState == FFX_FORMAT_ONCE)
        {
            /*  Allow an existing format to be used, so long as it fits properly
                in the chip:
            */
            status = FfxBbm5Mount(rgBbmList[ulChip].hbbm, &rgBbmList[ulChip].mount, FALSE);
            rgBbmList[ulChip].fMounted = (status == FFXSTAT_SUCCESS);
            if(!rgBbmList[ulChip].fMounted)
            {
                fFailedMount = TRUE;
            }
            else
            {
                /*  Doublecheck the BBM table geometry- may have found an
                    existing BBM format that doesn't meet requested parameters,
                    but is otherwise perfectly acceptable and usable. At least
                    verify that all chips use the same geometry.
                */
                if (ulChip == 0)
                {
                    pfd->ulBbmReservedBlocks = rgBbmList[ulChip].mount.ulReservedBlocks;
                    pfd->ulBbmDataBlocks = rgBbmList[ulChip].mount.ulDataBlocks;
                }
                else
                {
                    if ((pfd->ulBbmReservedBlocks != rgBbmList[ulChip].mount.ulReservedBlocks) ||
                        (pfd->ulBbmDataBlocks != rgBbmList[ulChip].mount.ulDataBlocks))
                        fFailedMount = TRUE;

                }
            }
        }
    }

    /*  The media is now mounted if it was already formatted, nothing
        more to do with the BBM mount.
    */
    if(pBbmSettings->nFormatState == FFX_FORMAT_NEVER)
    {
        status = FFXSTAT_SUCCESS;
        goto _FormatCheckComplete;
    }

    /*  If all the chips were successfully mounted, and the user indicated
        that format should be completed only once, we are done.
    */
    if((pBbmSettings->nFormatState == FFX_FORMAT_ONCE) && !fFailedMount)
    {
        /* All chips are mounted, success */
        status = FFXSTAT_SUCCESS;
        goto _FormatCheckComplete;
    }

    /*  At this point the format is either format ALWAYS or format ONCE *and*
        the media is not mounted. Dismount the media if we successfuly mounted
        already, this is necessary to format the media.
    */
    for(ulChip = 0; ulChip < ulChipCount; ++ulChip)
    {
        /* Only mount chips we are managing */
        if(rgBbmList[ulChip].hbbm != NULL && rgBbmList[ulChip].fMounted)
        {
            FfxBbm5Unmount(rgBbmList[ulChip].hbbm);
            rgBbmList[ulChip].fMounted = FALSE;
        }
    }

    /* Determine the number of blocks to reserved for replacements */
    ulReservedCount = pfd->pFimInfo->ulChipBlocks - pfd->ulBbmDataBlocks;
    if(ulReservedCount < BBM_MIN_RESERVED_BLOCKS)
    {
        ulReservedCount = BBM_MIN_RESERVED_BLOCKS;
        FFXPRINTF(1, ("Warning: reserved percentage results in too few blocks, defaulting to %U\n",
                ulReservedCount));
    }
    if(ulReservedCount >= pfd->pFimInfo->ulChipBlocks - BBM_MIN_RESERVED_BLOCKS)
    {
        FFXPRINTF(1, ("Error: reserved percentage results in too many (%lU) reserved blocks.\n",
                ulReservedCount));
        status = FFXSTAT_BADPARAMETER;
        goto _FormatCheckComplete;
    }

    /* Allocate a buffer to hold our list for each chip */
    rgulBadBlockList = DclMemAlloc(ulReservedCount * sizeof(*rgulBadBlockList));
    if(!rgulBadBlockList)
    {
        status = FFXSTAT_OUTOFMEMORY;
        goto _FormatCheckComplete;
    }

    for(ulChip = 0; ulChip < ulChipCount; ++ulChip)
    {
        /* Only format chips we are managing */
        if(rgBbmList[ulChip].hbbm == NULL)
        {
            continue;
        }

        /*  This is either an initial format (FORMAT_ONCE), or a
            reformat with new parameters or a FORMAT_ALWAYS.
        */
        ulBadBlockCount = ulReservedCount;
        status = DevBbmBuildBadBlockList(pfd, pBbmSettings, ulChip,
            rgulBadBlockList, &ulBadBlockCount);
        if (status == FFXSTAT_SUCCESS)
        {
            status = FfxBbm5Format(rgBbmList[ulChip].hbbm, ulReservedCount,
                ulBadBlockCount, rgulBadBlockList);
        }

        /* Fail if format returned something other than success */
        if(status != FFXSTAT_SUCCESS)
        {
            break;
        }
    }

  _FormatCheckComplete:

    /*  if the format appears successful mount the media
    */
    if(status == FFXSTAT_SUCCESS)
    {
        for(ulChip = 0; ulChip < pfd->ulBbmChips; ++ulChip)
        {
            if(rgBbmList[ulChip].hbbm && !rgBbmList[ulChip].fMounted)
            {
                status = FfxBbm5Mount(rgBbmList[ulChip].hbbm, &rgBbmList[ulChip].mount, FALSE);
                rgBbmList[ulChip].fMounted = (status == FFXSTAT_SUCCESS);
                if(!rgBbmList[ulChip].fMounted)
                {
                    break;
                }
            }
        }
    }

    if(rgulBadBlockList)
    {
        DclMemFree(rgulBadBlockList);
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: DevEraseGoodBlocks()

    Erases all blocks reported as good by the FIM.  This operation
    will destroy all data within the FIM device including BBM
    retired blocks and format information.

    This may be used under certain format operations during
    development where erroneous mappings have been incurred.

    Parameters:
        pfd - pointer to the FFXFIMDEVICE to erase.

    Return Value:
        Returns an FFXSTAT_SUCCESS after format check is completed
        or once the format operation is completed.
-------------------------------------------------------------------*/
static FFXSTATUS DevEraseGoodBlocks(
    FFXFIMDEVICE   *pfd,
    D_UINT32        ulStartBlock,
    D_UINT32        ulBlockCount)
{
    D_UINT32        ulBlock;
    FFXIOSTATUS     iostatus = DEFAULT_BLOCKIO_STATUS;

    DclAssert(pfd);
    DclAssert(ulBlockCount);

    for(ulBlock = ulStartBlock; ulBlock < ulStartBlock + ulBlockCount; ++ulBlock)
    {
        D_BOOL                      fDoErase;
        FFXIOR_FIM_GET_BLOCK_STATUS blockstatus;

        blockstatus.ior.ioFunc = FXIOFUNC_FIM_GET_BLOCK_STATUS;
        blockstatus.ior.ulReqLen = sizeof(blockstatus);
        blockstatus.ulBlock = ulBlock;

        iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &blockstatus.ior);

        /*  If there is an error, report it to the error manager for the
            error manager's purposes, but don't try to do anything about
            it because we are doing a a BBM format and there is currently
            no BBM to remap a discovered bad block.  We'll just have to
            wait until someone tries to use it later to rediscover it
            and remap it then.
        */
        if (iostatus.ffxStat != FFXSTAT_SUCCESS)
        {
            FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus, ulBlock, 0);
            FfxErrMgrKillError(&iostatus);
        }
        switch(iostatus.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE)
        {
            case BLOCKSTATUS_FACTORYBAD:
            {
                fDoErase = FALSE;
                break;
            }

            case BLOCKSTATUS_UNKNOWN:
            case BLOCKSTATUS_NOTBAD:
            {
                fDoErase = TRUE;
                break;
            }

            default:
            {
                FFXPRINTF(1, ("Warning: Unknown block status! (%lX)\n", iostatus.op.ulBlockStatus));
                fDoErase = FALSE;
                iostatus.ffxStat = FFXSTAT_DEVMGR_UNKNOWNBLOCKSTATUS;
                break;
            }
        }

        if(fDoErase)
        {
            FFXIOR_FIM_ERASE_START eraseStartReq;

            /*  Note: We cannot use our private interface since this
                function may be called prior to the BBM being
                actually initialized.
            */
            eraseStartReq.ulCount = 1;
            eraseStartReq.ulStartBlock = ulBlock;
            eraseStartReq.ior.ioFunc     = FXIOFUNC_FIM_ERASE_START;
            eraseStartReq.ior.ulReqLen   = sizeof(eraseStartReq);

            DclAssert(eraseStartReq.ulStartBlock >= pfd->ulReservedBlocks);
            iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &eraseStartReq.ior);

            if(iostatus.ffxStat == FFXSTAT_SUCCESS)
            {
                FFXIOR_FIM_ERASE_POLL   erasePoll;

                erasePoll.ior.ioFunc = FXIOFUNC_FIM_ERASE_POLL;
                erasePoll.ior.ulReqLen = sizeof(erasePoll);
                iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &erasePoll.ior);
                if(iostatus.ffxStat != FFXSTAT_SUCCESS)
                {
                    /*  If there is an error, report it to the error manager for the
                        error manager's purposes, but don't try to do anything about
                        it because we are doing a BBM format and there is currently
                        no BBM to remap a discovered bad block.  We'll just have to
                        wait until someone tries to use it later to rediscover it
                        and remap it then.
                    */
                    FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus, ulBlock, 0);
                    FfxErrMgrKillError(&iostatus);
                }
            }
            else
            {
                /*  If there is an error, report it to the error manager for the
                    error manager's purposes, but don't try to do anything about
                    it because we are doing a BBM format and there is currently
                    no BBM to remap a discovered bad block. We'll just have to
                    wait until someone tries to use it later to rediscover it
                    and remap it then.
                */
                FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus, ulBlock, 0);
                FfxErrMgrKillError(&iostatus);
            }
        }
    }

    return iostatus.ffxStat;
}


/*-------------------------------------------------------------------
    Local: DevBbmBuildBadBlockList()

    Builds a list of bad blocks as reported as factory bad by the
    FIM.  The list must be pre-allocated and is bound to the physical
    chip and the maximum number of bad blocks given.

    Parameters:
        pfd              - pointer to the FFXFIMDEVICE to erase.
        ulChip           - the physical chip number to query.
        rgulBadBlockList - list of blocks found to be factory bad.
        pulBadBlockCount - Maximum size of the bad block list on entry.

    Return Value:
        Returns an FFXSTAT_SUCCESS after blocks are queried and the
        list has been populated.  The pulBadBlockCount is updated to
        the number of bad blocks found at completion.
-------------------------------------------------------------------*/
static FFXSTATUS DevBbmBuildBadBlockList(
    FFXFIMDEVICE       *pfd,
    FFXBBMSETTINGS     *pBbmSettings,
    D_UINT32            ulChip,
    D_UINT32           *rgulBadBlockList,
    D_UINT32           *pulBadBlockCount)
{
    BbmDisk_t          *pbbmdisk;
    FFXSTATUS           status;
    D_UINT32            ulMaxBlocks;
    D_UINT32            ulBlock;

    DclAssert(pfd);
    DclAssert(rgulBadBlockList);
    DclAssert(pulBadBlockCount);
    DclAssert(*pulBadBlockCount < pfd->pFimInfo->ulPhysicalBlocks);
    DclAssert(*pulBadBlockCount != 0);

    pbbmdisk = pfd->pBbm;
    DclAssert(pbbmdisk);

    /*  The maximum number of bad blocks that can be placed into
        the array is passed in, going out it represents how many
        were placed into the array, reset it to zero.
    */
    ulMaxBlocks = *pulBadBlockCount;
    *pulBadBlockCount = 0;
    status = FFXSTAT_SUCCESS;

    /*  Loop through all the blocks of this chip, note that
        the total blocks and starting offset of each chip
        may be different.  Specifically with the first and last
        chips.
    */
    for(ulBlock = pbbmdisk[ulChip].ulBlockOffset;
        ulBlock < pbbmdisk[ulChip].ulBlockOffset + pbbmdisk[ulChip].media.ulTotalBlocks;
        ++ulBlock)
    {
        FFXIOSTATUS                 iostatus;
        FFXIOR_FIM_GET_BLOCK_STATUS blockinfo;

        DclAssert(ulBlock < pfd->pFimInfo->ulPhysicalBlocks);

        blockinfo.ior.ioFunc = FXIOFUNC_FIM_GET_BLOCK_STATUS;

        blockinfo.ulBlock = ulBlock;
        blockinfo.ior.ulReqLen = sizeof(blockinfo);
        iostatus = pfd->pFimEntry->IORequest(pfd->hInst, &blockinfo.ior);

        /*  If there was an error, inform the error manager.
        */
        if (iostatus.ffxStat != FFXSTAT_SUCCESS)
        {
            FfxErrMgrReportError(pfd, pfd->errMgrInstance, &iostatus, ulBlock, 0);
            status = iostatus.ffxStat;
        }

        /*  Call out to the BBM hook to allow the block status to be
            monitored or updated.  Note this is used for both OEM and
            testing purposes.
        */
        iostatus.op.ulBlockStatus = FfxHookBbmFormatBlock(
            pfd->hDev, blockinfo.ulBlock, iostatus.op.ulBlockStatus);
        switch(iostatus.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE)
        {
            case BLOCKSTATUS_FACTORYBAD:
             {
                if(pBbmSettings->nFormatType == FFX_BBMFORMAT_REUSE_FACTORYBADBLOCKS)
                {
                    break;
                }

                /*  Do not add more blocks than which we can handle.  Allow
                    the system to format and mount, however.
                */
                if(*pulBadBlockCount == ulMaxBlocks)
                {
                    FFXPRINTF(2, ("Warning: bad block list full for FIMDEV: %U Chip: %lX "
                                  "Further bad blocks will be recorded for this chip.\n",
                        pfd->uDeviceNum, ulChip));
                    status = FFXSTAT_BBM_NO_SPACE;
                    break;
                }

                /*  Record this bad block; being sure to account for the fact
                    that each chip has its own self-contained BBM instance
                    and the block number must be corrected by the chip's
                    offset into the array.
                */
                rgulBadBlockList[*pulBadBlockCount] = blockinfo.ulBlock - 
                    pbbmdisk[ulChip].ulBlockOffset;
                ++*pulBadBlockCount;
                break;
            }

            case BLOCKSTATUS_NOTBAD:
            case BLOCKSTATUS_UNKNOWN:
            {
                /*  These are good blocks (or at least not clearly bad blocks);
                    there is nothing to do with them.
                */
                break;
            }

            default:
            {
                /*  These types are no longer used with the new BBM
                */
                status = FFXSTAT_DEVMGR_UNKNOWNBLOCKSTATUS;
                DclError();
                break;
            }
        }


        /*  Stop parsing the blocks if we got an error at this point.
        */
        if(status != FFXSTAT_SUCCESS)
        {
            break;
        }
    }

    /*  The bad block list is updated and the number of bad
        blocks detected is set.  Return the last status reported.
    */
    return status;
}


/*-------------------------------------------------------------------
    Local: DevBbmRead()

    Transfers a single page of data within a given BBM disk instance.
    This function interfaces directly with the FIM to read a page
    from the flash media.  The given page is relative to a physical
    flash chip as described by the FIM and is read with any native
    ECC and/or EDC that is required by the flash hardware.

    The BBM uses this function exlcusively to read it's header
    information and subsequent bad block maps.

    The size of the buffer is defined by the context in which the
    read takes place.  pbbmchip provides a pointer to the context
    which includes a description of the media.

    Parameters:
        pbbmchip - Pointer to the private BbmDisk_t structure that
                   describes the physical chip.
        ulPage   - Page relative to the physical chip to be read
        pbuf     - Pointer to a page sized buffer to recieve the data.

    Return Value:
        FFXIOSTATUS - of the resulting read.  The BBM will determine
                      what action to take based on the results of the
                      read.

    See Also:
        DevBbmWrite()
        DevBbmErase()
-------------------------------------------------------------------*/
static FFXIOSTATUS DevBbmRead(
    BbmDisk_t *pbbmchip,
    D_UINT32 ulPage,
    D_BUFFER *pbuf)
{
    D_UINT32  ulArrayPage;
    FFXIOSTATUS ioStat;
    FFXIOR_FIM_READ_GENERIC FimReq;
    D_UINT8 aucBBMTag[FFX_NAND_TAGSIZE];

    DclAssert(pbbmchip);

    ulArrayPage = ulPage + (pbbmchip->ulBlockOffset * pbbmchip->pfd->pFimInfo->uPagesPerBlock);

    FimReq.ulStartPage = ulArrayPage;
    FimReq.ulCount = 1;
    FimReq.pPageData = pbuf;

    FimReq.ior.ioFunc = FXIOFUNC_FIM_READ_GENERIC;
    FimReq.ior.ulReqLen = sizeof(FFXIOR_FIM_READ_GENERIC);
    FimReq.ioSubFunc = FXIOSUBFUNC_FIM_READ_TAGGEDPAGES;
    FimReq.pAltData = &aucBBMTag[0];
    FimReq.nAltDataSize = DCLDIMENSIONOF(aucBBMTag);

    DclAssert(FimReq.ulStartPage
        >= pbbmchip->pfd->ulReservedBlocks
                * pbbmchip->pfd->pFimInfo->uPagesPerBlock);

    ioStat = pbbmchip->pfd->pFimEntry->IORequest(pbbmchip->pfd->hInst, &FimReq.ior);

    FfxDevReportRead(pbbmchip->pfd,ulArrayPage,1);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Report any error to error manager;
        */
        FfxErrMgrReportError(pbbmchip->pfd, pbbmchip->pfd->errMgrInstance, &ioStat,
                             ulArrayPage / pbbmchip->pfd->pFimInfo->uPagesPerBlock,
                             ulArrayPage % pbbmchip->pfd->pFimInfo->uPagesPerBlock);
    }
    return ioStat;
}


/*-------------------------------------------------------------------
    Local: DevBbmWrite()

    Transfers a single page of data within a given BBM disk instance.
    This function interfaces directly with the FIM to write a page
    into the flash media.  The given page is relative to a physical
    flash chip as described by the FIM and is read with any native
    ECC and/or EDC that is required by the flash hardware.

    The BBM uses this function exlcusively to write it's header
    information and subsequent bad block maps.

    The size of the buffer is defined by the context in which the
    read takes place.  pbbmchip provides a pointer to the context
    which includes a description of the media.

    Parameters:
        pbbmchip - Pointer to the private BbmDisk_t structure that
                   describes the physical chip.
        ulPage   - Page relative to the physical chip to be written.
        pbuf     - Pointer to a page sized buffer to recieve the data.

    Return Value:
        FFXIOSTATUS - of the resulting write.  The BBM will determine
                      what action to take based on the results of the
                      write.

    See Also:
        DevBbmRead()
        DevBbmErase()
-------------------------------------------------------------------*/
static FFXIOSTATUS DevBbmWrite(
    BbmDisk_t *pbbmchip,
    D_UINT32 ulPage,
    D_BUFFER *pbuf)
{
    D_UINT32  ulArrayPage;
    FFXIOSTATUS ioStat;
    FFXIOR_FIM_READ_GENERIC FimReq;
    D_UINT8 aucBBMTag[FFX_NAND_TAGSIZE] = {0x42, 0x64};

    DclAssert(pbbmchip);

    ulArrayPage = ulPage + (pbbmchip->ulBlockOffset * pbbmchip->pfd->pFimInfo->uPagesPerBlock);

    FimReq.ulStartPage = ulArrayPage;
    FimReq.ulCount = 1;
    FimReq.pPageData = pbuf;

    FimReq.ior.ioFunc = FXIOFUNC_FIM_WRITE_GENERIC;
    FimReq.ior.ulReqLen = sizeof(FFXIOR_FIM_WRITE_GENERIC);
    FimReq.ioSubFunc = FXIOSUBFUNC_FIM_WRITE_TAGGEDPAGES;
    FimReq.pAltData = &aucBBMTag[0];
    FimReq.nAltDataSize = DCLDIMENSIONOF(aucBBMTag);

    DclAssert(FimReq.ulStartPage
        >= pbbmchip->pfd->ulReservedBlocks
                * pbbmchip->pfd->pFimInfo->uPagesPerBlock);

    ioStat = pbbmchip->pfd->pFimEntry->IORequest(pbbmchip->pfd->hInst, &FimReq.ior);

    FfxDevReportWrite(pbbmchip->pfd,ulArrayPage,1);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Report any error to error manager;
        */
        FfxErrMgrReportError(pbbmchip->pfd, pbbmchip->pfd->errMgrInstance, &ioStat,
                             ulArrayPage / pbbmchip->pfd->pFimInfo->uPagesPerBlock,
                             ulArrayPage % pbbmchip->pfd->pFimInfo->uPagesPerBlock);
    }
    return ioStat;
}


/*-------------------------------------------------------------------
    Local: DevBbmErase()

    Erases a single flash erase block within the given BBM instance.
    This function interfaces directly with the FIM to erase a block
    of the flash media.  The given block is relative to a physical
    flash chip as described by the FIM.

    The BBM uses this function exlcusively to prepare a block for
    writing header information and subsequent bad block maps.

    pbbmchip provides a pointer to the context which includes a
    description of the media.

    Parameters:
        pbbmchip - Pointer to the private BbmDisk_t structure that
                   describes the physical chip.
        ulBlock  - Page relative to the physical chip to be written.

    Return Value:
        FFXIOSTATUS - of the resulting erase.  If it is not a successful
                      status, it has been processed by the Error Manager.
                      The BBM will determine what action to take based
                      on the results of the erase.

    See Also:
        DevBbmRead()
        DevBbmWrite()
-------------------------------------------------------------------*/
static FFXIOSTATUS DevBbmErase(
    BbmDisk_t *pbbmdisk,
    D_UINT32 ulBlock)
{
    FFXIOSTATUS ioStat;
    FFXIOR_FIM_ERASE_START eraseStartReq;

    DclAssert(pbbmdisk);

    ulBlock += pbbmdisk->ulBlockOffset;

    eraseStartReq.ulCount = 1;
    eraseStartReq.ulStartBlock = ulBlock;
    eraseStartReq.ior.ioFunc     = FXIOFUNC_FIM_ERASE_START;
    eraseStartReq.ior.ulReqLen   = sizeof(eraseStartReq);

    DclAssert(eraseStartReq.ulStartBlock >= pbbmdisk->pfd->ulReservedBlocks);
    ioStat = pbbmdisk->pfd->pFimEntry->IORequest(
                pbbmdisk->pfd->hInst, &eraseStartReq.ior);
    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
    {
        FFXIOR_FIM_ERASE_POLL   erasePoll;

        erasePoll.ior.ioFunc = FXIOFUNC_FIM_ERASE_POLL;
        erasePoll.ior.ulReqLen = sizeof(erasePoll);

        ioStat = pbbmdisk->pfd->pFimEntry->IORequest(
                    pbbmdisk->pfd->hInst, &erasePoll.ior);
        FfxDevReportErase(pbbmdisk->pfd, eraseStartReq.ulStartBlock);
    }

    if (ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        FfxErrMgrReportError(pbbmdisk->pfd, pbbmdisk->pfd->errMgrInstance,
                &ioStat, eraseStartReq.ulStartBlock, 0);
    }

    return ioStat;
}

#endif


