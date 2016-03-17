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

    This module implements the main FML (Flash Mapping Layer) logic which
    maps Disks onto Devices.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fml.c $
    Revision 1.55  2010/12/22 17:39:57Z  garyp
    Corrected a broken debug message.
    Revision 1.54  2010/12/15 08:19:23Z  glenns
    Fix build failure when FFXCONF_NANDSUPPORT is FALSE.
    Revision 1.53  2010/12/14 23:15:58Z  glenns
    Major revision to support FML disk spanning of devices with
    BBMv5. See bug 3280.
    Revision 1.52  2010/07/08 03:33:58Z  garyp
    Removed some unnecessary error handling code.
    Revision 1.51  2010/07/05 22:26:57Z  garyp
    Added support for chip serial numbers.
    Revision 1.50  2010/06/19 03:41:30Z  garyp
    Updated to track the bit correction capabilities on a per segment basis.
    Revision 1.49  2010/04/29 00:04:22Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.48  2010/01/27 04:27:34Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.47  2009/12/11 20:33:43Z  garyp
    Fixed an uninitialized variable.
    Revision 1.46  2009/12/02 23:50:38Z  garyp
    Updated FfxFmlParameterGet() to accommodate gathering chip IDs
    for Disks which span multiple Devices.
    Revision 1.45  2009/11/17 20:26:51Z  garyp
    Fixed a bug that caused the number of Disks mapped to a Device
    to always be reported 0 in the FFXFMLDEVINFO structure.
    Revision 1.44  2009/08/04 19:34:37Z  garyp
    Merged from the v4.0 branch.  Major update to support the concept of a
    Disk which spans multiple Devices.  Added OTP support.  Updated so
    the Read/WriteUncorrectedPages functionality is visible even if NAND
    support is disabled, as this simplifies some higher level code.  A run-time
    determination is made to determine which low level interfaces to use.
    Moved the locking related functions into a separate module.  Added 
    support for the "Read/WriteUncorrectedPages" interfaces.  Added 
    support for block locking and unlocking.
    Revision 1.43  2009/04/19 20:28:37Z  keithg
    Fixed bug 2646, FXIOFUNC_FML_GET_BLOCK_STATUS now calls the correct Device
    Manager call to be mapped by the BBM.
    Revision 1.42  2009/04/09 02:58:33Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.41  2009/04/07 20:35:29Z  davidh
    Function headers updated for AutoDoc:
    Revision 1.40  2009/04/02 20:19:13Z  davidh
    Function headers fixed for AutoDoc.
    Revision 1.39  2009/04/01 20:18:00Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.38  2009/03/10 00:13:21Z  glenns
    Fix Bugzilla #2395: Added code to FfxFmlDeviceInfo to assign values to new
    elements of the FFXFMLDEVICEINFO structure, making device EDC Capability
    and Requirement, BBM Reserved Blocks Rating and Erase Cycle Rating visible
    to clients of the ML.
    Revision 1.37  2009/02/08 21:04:57Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables
    -- no functional changes.
    Revision 1.36  2009/01/19 04:34:41Z  keithg
    Removed code and compile time condition for support of the old BBM which is
    now obsolete.
    Revision 1.35  2008/12/31 00:04:45Z  keithg
    Updated the stats tracking to allow the capture of raw accesses with
    exception of any that may be out of bounds relative to the FML disk mapping.
    Revision 1.34  2008/12/24 18:10:28Z  keithg
    Added support new IOCTL support for raw erase operations.  Clarified debug
    messages.
    Revision 1.33  2008/12/18 17:17:05Z  keithg
    Replace the WriteNativePages functionality which as incorrectly removed in
    the previous check in.
    Revision 1.32  2008/12/17 23:51:43Z  keithg
    Corrected compile time conditions and removed unused IOCTLs.
    Revision 1.31  2008/12/11 23:47:16Z  keithg
    Added support for read/write native pages and setting the block status.
    Revision 1.30  2008/10/23 23:18:19Z  keithg
    Improved error handling with status information rather than asserts.
    Revision 1.29  2008/09/02 05:12:45Z  keithg
    Raw block status functionality is no longer dependant upon the inclusion of
    BBM functionality.  Removed asserts from handled error conditions.
    Revision 1.28  2008/08/10 06:02:49Z  keithg
    Changed invalid parameter warnings to debug level 2.
    Revision 1.27  2008/05/07 01:45:14Z  garyp
    Updated to track read and write stats on a per-block basis.
    Revision 1.26  2008/05/03 21:26:25Z  garyp
    Added debug code.
    Revision 1.25  2008/03/14 17:49:32Z  Garyp
    Updated to support variable width tags.  Expanded the various Get/Set
    Page and Block Status functions.
    Revision 1.24  2008/01/13 07:36:39Z  keithg
    Function header updates to support autodoc.
    Revision 1.23  2007/12/13 20:46:46Z  Garyp
    Updated to support tracking and querying erase counts.
    Revision 1.22  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.21  2007/09/28 00:42:08Z  pauli
    Resolved Bug 355: Added an alignment field to the FFXFMLDEVICEINFO
    structure.
    Revision 1.20  2007/08/01 00:06:55Z  timothyj
    Removed access to obsolete byte offset and length.
    Revision 1.19  2007/07/16 20:22:47Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.18  2007/06/29 21:12:22Z  rickc
    Added FfxFmlNandGetPhysicalBlockStatus()
    Revision 1.17  2007/04/07 03:37:35Z  Garyp
    Modified the "ParameterGet" function so that the buffer size for a given
    parameter can be queried without actually retrieving the parameter.
    Revision 1.16  2006/11/08 18:32:29Z  Garyp
    Updated to support FML statistics gathering.
    Revision 1.15  2006/10/18 17:46:37Z  billr
    Revert changes made in revision 1.14.
    Revision 1.13  2006/09/07 20:10:31Z  Pauli
    Updated to allow FfxFmlDeviceInfo to be called even if no disks
    have been mapped.
    Revision 1.12  2006/05/23 18:36:37Z  Garyp
    Updated to build properly if NOR/Sibley is disabled.
    Revision 1.11  2006/05/19 00:52:55Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.10  2006/03/29 23:37:16Z  Garyp
    Documentation updates.
    Revision 1.9  2006/03/07 03:16:29Z  Garyp
    Added RawPageRead/Write() support.  Added FfxFmlDeviceInfo().
    Revision 1.8  2006/03/03 07:37:47Z  Garyp
    Debug code updated.
    Revision 1.7  2006/02/15 23:02:57Z  Garyp
    Re-added a commented out structure member.
    Revision 1.6  2006/02/15 19:23:25Z  Garyp
    Cleaned up som messages.
    Revision 1.5  2006/02/14 22:46:00Z  Garyp
    Corrected some function storage classes.
    Revision 1.4  2006/02/13 02:43:22Z  Garyp
    Added debugging code.
    Revision 1.3  2006/02/10 23:33:42Z  Garyp
    Updated debugging code.
    Revision 1.2  2006/02/10 09:32:56Z  Garyp
    Refactored such that the FML is literally just the flash mapping layer.
    Other functionality is moved into the Device Manager layer.
    Revision 1.1  2005/11/28 20:26:48Z  Pauli
    Initial revision
    Revision 1.6  2005/11/28 20:26:47Z  Garyp
    Added an undocumented feature which allows the read-back verify
    functionality to be enabled, but in a less verbose fashion.
    Revision 1.5  2005/11/14 19:44:09Z  Garyp
    Debug code updated.
    Revision 1.4  2005/10/30 08:15:43Z  Garyp
    Eliminated the use of STD_NAND_PAGE_SIZE, but rather use the page
    size reported by the FIM.
    Revision 1.3  2005/10/22 08:19:50Z  garyp
    Renamed the ReadBackVerify option.
    Revision 1.2  2005/10/12 04:43:35Z  Garyp
    Added ReadBackVerify.
    Revision 1.1  2005/10/11 17:30:24Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fxfmlapi.h>
#include "fml.h"

static FFXFMLHANDLE aFMLHandles[FFX_MAX_DISKS];
static PDCLMUTEX    pFMLMutex = NULL;
static unsigned     nNumCurrentMappings = 0;
static FFXFMLDISK  *pChainHead = NULL;

static FFXIOSTATUS ReadPages(  FFXFMLHANDLE hFML, FFXIOR_FML_READ_PAGES *pFmlReq);
static FFXIOSTATUS WritePages( FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_PAGES *pFmlReq);
static FFXIOSTATUS EraseBlocks(FFXFMLHANDLE hFML, FFXIOR_FML_ERASE_BLOCKS *pFmlReq);
static void FreeLayoutStructs(FFXFMLDISK *pFML);

/* Static error code declarations for common error returns
 */
static const FFXIOSTATUS    gsioBlockStructLen  = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);
static const FFXIOSTATUS    gsioBlockBadParam   = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADPARAMETER);
static const FFXIOSTATUS    gsioPageStructLen  = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);
static const FFXIOSTATUS    gsioUnsupported= INITIAL_PAGEIO_STATUS(FFXSTAT_FML_UNSUPPORTEDIOREQUEST);

#if FFXCONF_STATS_FML
static void ResetFMLStats(FFXFMLHANDLE hFML);
#endif


/*-------------------------------------------------------------------
    Public: FfxFmlCreate()

    Create an FML instance, which is a window mapped onto a Device,
    thereby creating a Disk.

    The ulStartBlock and ulBlockCount arguments specify the
    location and size of the window in terms of physical erase
    blocks.

    *Note* - When the FFX_DISK_SPANDEVICES flag is used, the
             ulStartBlock value must not specify an offset which
             is beyond the length of the first Device.

    Parameters:
        hDisk        - The FFXDISKHANDLE to use
        hFimDev      - The handle to the FIM device, created by the
                       DevMgr layer.
        nDiskNum     - The number for this Disk (DISKn).
        ulStartBlock - The starting block number, relative to zero.
        ulBlockCount - The number of blocks in the Disk.  May be
                       FFX_REMAINING to use all the remaining blocks
                       in the Device.
        nFlags       - The flags to use, which may be zero or more
                       of the following:
                     + FFX_DISK_RAW - Create a raw Disk mapping, which
                       is allowed to include the BBM area (if NAND).
                       Only selected APIs may be used on the Disk if
                       is is mapped in raw mode.
                     + FFX_DISK_SPANDEVICES - If ulBlockCount is
                       longer than the number of blocks remaining on
                       the Device, and there is a subsequent Device
                       with the same characteristics, allow the Disk
                       to span the Devices.


    Return Value:
        Returns an FFXFMLHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXFMLHANDLE FfxFmlCreate(
    FFXDISKHANDLE   hDisk,
    FFXFIMDEVHANDLE hFimDev,
    unsigned        nDiskNum,
    D_UINT32        ulStartBlock,
    D_UINT32        ulBlockCount,
    unsigned        nFlags)
{
    FFXFMLDISK     *pFML = NULL;
    FFXFMLDISK     *pThis;
    FFXFIMDEVINFO   fi;
    D_UINT32        ulPagesPerBlock;
    D_BOOL          fMutexAcquired = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(TRACEALWAYS, 1, TRACEINDENT),
        "FfxFmlCreate() hDisk=%P hFimDev=%P DiskNum=%u StartBlock=%lX Count=%lX Flags=%x\n",
        hDisk, hFimDev, nDiskNum, ulStartBlock, ulBlockCount, nFlags));

    if(nDiskNum >= FFX_MAX_DISKS)
    {
        FFXPRINTF(1, ("FML DISK%u out of range (%u)\n", nDiskNum, FFX_MAX_DISKS));
        goto ErrorCleanup;
    }

    /*  Don't allow a Disk to be mounted more than once
    */
    if(aFMLHandles[nDiskNum])
    {
        FFXPRINTF(1, ("FML DISK%u already created\n", nDiskNum));
        goto ErrorCleanup;
    }

    if(!FfxDevInfo(hFimDev, &fi))
    {
        FFXPRINTF(1, ("FIM is not valid\n"));
        goto ErrorCleanup;
    }

    if(!fi.uPageSize || !fi.ulBlockSize)
    {
        FFXPRINTF(1, ("FfxFmlCreate() Page size (%u) or block size (%lU) is invalid\n", fi.uPageSize, fi.ulBlockSize));
        goto ErrorCleanup;
    }

    if(nFlags & FFX_DISK_RAW)
    {
        if(fi.uDeviceType != DEVTYPE_NAND)
        {
            DclPrintf("The FFX_DISK_RAW is only valid on NAND\n");
            goto ErrorCleanup;
        }
        
        if(nFlags & FFX_DISK_SPANDEVICES)
        {
            DclPrintf("The FFX_DISK_SPANDEVICES and FFX_DISK_RAW options may not be used together\n");
            goto ErrorCleanup;
        }
    }

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    if((nFlags & FFX_DISK_SPANDEVICES) && fi.ulBootBlockSize)
    {
        DclPrintf("The FFX_DISK_SPANDEVICES option is not available on flash with boot blocks\n");
        goto ErrorCleanup;
    }
  #endif

    if(!pFMLMutex)
    {
        DclAssert(DCL_MUTEXNAMELEN >= 8);

        pFMLMutex = DclMutexCreate("FXFML");
        if(!pFMLMutex)
        {
            DclError();
            goto ErrorCleanup;
        }
    }

    /*  Allocate a structure for this potential Disk
    */
    pFML = DclMemAllocZero(sizeof(*pFML));
    if(!pFML)
    {
        DclError();
        goto ErrorCleanup;
    }

    /*  Allocate the first layout structure -- we know we will
        need at least one.
    */
    pFML->pLayout = DclMemAllocZero(sizeof(*pFML->pLayout));
    if(!pFML->pLayout)
    {
        DclError();
        goto ErrorCleanup;
    }

    DclAssert((fi.ulBlockSize % fi.uPageSize) == 0);
    ulPagesPerBlock = fi.ulBlockSize / fi.uPageSize;

    while(1 << pFML->nBytePageShift != fi.uPageSize)
        pFML->nBytePageShift++;

    pFML->hDisk             = hDisk;
    pFML->nDiskNum          = nDiskNum;
    pFML->uLockFlags        = fi.uLockFlags;
    pFML->nStartDevNum      = fi.nDeviceNum;

    if(fi.uDeviceType == DEVTYPE_NAND)
        pFML->uDiskFlags   |= DISK_NAND;

  #if FFXCONF_NANDSUPPORT
    pFML->nSpareSize        = (unsigned)fi.uSpareSize;
  #endif

  #if FFXCONF_OTPSUPPORT
    pFML->nOTPPages         = fi.nOTPPages;
  #endif

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    pFML->ulBlockSize       = fi.ulBlockSize;

    if(fi.ulBootBlockSize)
    {
        /*  Denote that this particular Device has boot blocks, and
            pre-calculate the number of boot blocks per block so we
            don't have to repeatedly do it later.
        */
        pFML->uDiskFlags   |= DISK_HASBOOTBLOCKS;
        pFML->ulBBsPerBlock = fi.ulBlockSize / fi.ulBootBlockSize;
    }
  #endif

    if(nFlags & FFX_DISK_RAW)
    {
        if(ulBlockCount == FFX_REMAINING)
            pFML->pLayout->ulBlockCount = fi.ulRawBlocks - ulStartBlock;
        else
            pFML->pLayout->ulBlockCount = ulBlockCount;

        if(ulStartBlock + pFML->pLayout->ulBlockCount > fi.ulRawBlocks)
        {
            DclPrintf("DISK%u raw block count out of range\n", nDiskNum);
            goto ErrorCleanup;
        }
            
        pFML->pLayout->hFimDev          = hFimDev;
        pFML->pLayout->ulBlockOffset    = ulStartBlock;
        pFML->pLayout->ulPageOffset     = ulStartBlock * ulPagesPerBlock;
        pFML->pLayout->ulPageCount      = pFML->pLayout->ulBlockCount * ulPagesPerBlock;
      #if FFXCONF_BBMSUPPORT
        pFML->pLayout->ulRawBlockCount  = pFML->pLayout->ulBlockCount;
        pFML->pLayout->ulRawBlockOffset = pFML->pLayout->ulBlockOffset;
        pFML->pLayout->ulRawPageOffset  = pFML->pLayout->ulPageOffset;
        pFML->pLayout->ulRawPageCount   = pFML->pLayout->ulPageCount;
      #endif
        
        pFML->uDiskFlags       |= DISK_RAWACCESSONLY;
        pFML->nDevCount         = 1;
        pFML->ulTotalBlocks     = pFML->pLayout->ulBlockCount;
        pFML->ulTotalPages      = pFML->pLayout->ulPageCount;
    }
    else
    {
        D_UINT32        ulDevBlock;
        FFXFMLLAYOUT   *pLayout;
        
        /*  Starting with the offset in the specified Device, add blocks
            to the Disk.  If the FFX_DISK_SPANDEVICES flag is set, then
            if the ulBlockCount is large enough (or FFX_REMAINING), try
            to appropriate blocks from subsequent Devices.  Any Devices
            handled in this manner <must> be numerically sequential, and
            the Device characteristics must be virtually identical (though
            the total number of blocks may be different).
        */

        /*  The offset into the first Device is as specified by the calling
            parameters (any subsequent Devices will start at offset zero).
        */
        ulDevBlock = ulStartBlock;
        pLayout = pFML->pLayout;

        while(ulBlockCount)
        {
            D_UINT32    ulDevBlocks;
          #if FFXCONF_BBMSUPPORT
            D_UINT32    ulDevRawBlocks;

            ulDevRawBlocks = fi.ulRawBlocks;
          #endif
          
            ulDevBlocks = fi.ulTotalBlocks;

            DclAssert(ulDevBlocks > ulDevBlock);
            
            /*  REFACTOR: The following lines of code assume a one-to-one
                correspondence between raw device blocks and normally-
                mapped device blocks; i.e. a starting offset into a device
                takes away the same number of normally-mapped and raw blocks.
                THIS IS ONLY TRUE IF THE DEVICE ONLY COVERS ONE CHIP. If 
                there is more than one chip in the device, there could be BBM
                tables at a point lower in the raw address space then the
                offset, and that would cause the number of raw blocks and
                the number of normally-mapped blocks taken away from the
                FML mapping to be different.

                The primary effect of this is that the raw block count
                in the first layout would be incorrect. This has two
                consequences. First, range checking on any of the layouts
                for raw accesses will not be correct for raw accesses. This
                isn't really a big deal, as in the current incarnation
                of fmlnand.c, the range checking has been removed from the
                raw accesses anyway.

                The second consequence is more severe. Correct raw block
                aceess for additional layouts in a spanning configuration
                are completely dependent on an accurate raw block count in
                the previous layout. If the first layout isn't accurate, 
                none of the others will be either, and for these additional
                layouts the wrong block will get accessed for the raw
                interfaces.

                This second consequence is a showstopper. It literally means
                that spanning configurations cannot span devices with 
                multiple chips. For now, disallow such configurations.

                See Bugzilla 3280.
            */
            ulDevBlocks -= ulDevBlock;
            
          #if FFXCONF_BBMSUPPORT
            if ((nFlags & FFX_DISK_SPANDEVICES) && (fi.ulBbmChips > 1))
            {
                DclPrintf("DISK%u cannot span device with multiple BBM-managed chips\n", nDiskNum);
                goto ErrorCleanup;
            }
            ulDevRawBlocks -= ulDevBlock;
          #endif

            if(ulBlockCount == FFX_REMAINING)
            {
                /*  We were told to use all the remaining blocks.  If the
                    "SpanDevices" feature is not being used, we're done.
                */
                if(!(nFlags & FFX_DISK_SPANDEVICES))
                    ulBlockCount = 0;
            }
            else
            {
                /*  If an explicit block count was specified, and the count
                    exceeds the number of available blocks in this Device,
                    we had better be using the "SpanDevices" feature, or we
                    will error out.
                */
                if(ulBlockCount > ulDevBlocks)
                {
                    if(!(nFlags & FFX_DISK_SPANDEVICES))
                    {
                        DclPrintf("DISK%u block count out of range\n", nDiskNum);
                        goto ErrorCleanup;
                    }
                }
                else
                {
                    /*  A size was specified which is smaller than the
                        available space in the Device.  No problem.

                        REFACTOR- related to the note above, this code suffers
                        similar shortcoming. If the mapping spans a BBM table
                        in a multichip configuration, this assignment to 
                        uDevRawBlocks is incorrect. See bugzilla 3280.
                    */
                    ulDevBlocks = ulBlockCount;
                  #if FFXCONF_BBMSUPPORT
                    ulDevRawBlocks = ulBlockCount;
                  #endif
                }

                /*  Adjust ulBlockCount to track the remaining blocks
                    to map, if any.
                */
                ulBlockCount -= ulDevBlocks;
            }

            /*  Record all the information for the portion of the Disk which
                resides in this Device.
            */
            pLayout->hFimDev          = hFimDev;
            pLayout->ulBlockOffset    = ulDevBlock;
            pLayout->ulBlockCount     = ulDevBlocks;
            pLayout->ulPageOffset     = ulDevBlock * ulPagesPerBlock;
            pLayout->ulPageCount      = ulDevBlocks * ulPagesPerBlock;
          #if FFXCONF_BBMSUPPORT
            pLayout->ulRawBlockOffset = ulDevBlock;
            pLayout->ulRawBlockCount  = ulDevRawBlocks;
            pLayout->ulRawPageOffset  = ulDevBlock * ulPagesPerBlock;
            pLayout->ulRawPageCount   = ulDevRawBlocks * ulPagesPerBlock;
          #endif

            pFML->ulTotalBlocks += ulDevBlocks;
            pFML->nDevCount++;

            /*  ulBlockCount will only be non-zero if there are more blocks
                to map, <AND> the FFX_DISK_SPANDEVICES flag is set.  If so,
                then get information for the next Device and see if it is
                a close enough match to allow a Disk to span both Devices.
            */
            if(ulBlockCount)
            {
                FFXFIMDEVHANDLE hNewFimDev;

                hNewFimDev = FfxDevHandle(fi.nDeviceNum + 1);
                if(hNewFimDev)
                {
                    FFXFIMDEVINFO   fiNew;

                    if(FfxDevInfo(hNewFimDev, &fiNew))
                    {
                        if( (fiNew.uDeviceType     == fi.uDeviceType) &&
                            (fiNew.uDeviceFlags    == fi.uDeviceFlags) &&
                            (fiNew.uLockFlags      == fi.uLockFlags) &&
                            (fiNew.ulBlockSize     == fi.ulBlockSize) &&
                            (fiNew.uPageSize       == fi.uPageSize) &&
                            (fiNew.uSpareSize      == fi.uSpareSize) &&
                            (fiNew.uMetaSize       == fi.uMetaSize) )
                        {
                            FFXPRINTF(1, ("Spanning DISK%u onto the next Device (DEV%u)\n", nDiskNum, fiNew.nDeviceNum));

                            hFimDev = hNewFimDev;
                            fi = fiNew;

                            /*  The next Device is a close enough match. 
                                Allocate a layout structure for it.
                            */
                            pLayout->pNext = DclMemAllocZero(sizeof(*pLayout->pNext));
                            if(!pLayout->pNext)
                            {
                                DclError();
                                goto ErrorCleanup;
                            }

                            pLayout = pLayout->pNext;

                            /*  All subsequent Devices will always start at
                                block 0
                            */
                            ulDevBlock = 0;

                            continue;
                        }
                        else
                        {
                            FFXPRINTF(1, ("Device characteristics do not match -- spanning done/disabled for DISK%u\n", nDiskNum));
                        }
                    }
                }

                /*  If we were directed to use the remaining space, and we
                    have run out of Devices, we're all good.  Break out and
                    continue initializing.
                */
                if(ulBlockCount == FFX_REMAINING)
                    break;

                /*  An explicit range was specified and we can't find a
                    subsequent Device.  Error out.
                */
                DclPrintf("DISK%u block count out of range\n", nDiskNum);
                goto ErrorCleanup;
            }
        }
    }
    
    pFML->ulTotalPages = pFML->ulTotalBlocks * ulPagesPerBlock;

  #if FFXCONF_STATS_FML
    /*  Allocate memory to store the erase counts for each block in the Disk
    */
    pFML->stats.pBlockStats = DclMemAllocZero(pFML->ulTotalBlocks * sizeof(*pFML->stats.pBlockStats));
    if(pFML->stats.pBlockStats)
    {
        pFML->stats.ulBlockStatsCount = pFML->ulTotalBlocks;
    }
    else
    {
        DclPrintf("Warning: Unable to allocate memory for FML block statistics, continuing...\n");
    }

    /*  Save a copy of this value where it is easily accessible so we don't
        have to go look it up every time we record block statistics.
    */
    pFML->ulPagesPerBlock = ulPagesPerBlock;

  #endif

    if(DclMutexAcquire(pFMLMutex))
    {
        unsigned    nFirst;
        unsigned    nLast;

        fMutexAcquired = TRUE;

        /*  Local variables for readability
        */
        nFirst = pFML->nStartDevNum;
        nLast = pFML->nStartDevNum + pFML->nDevCount - 1;

        for(pThis = pChainHead;
            pThis;
            pThis = pThis->pNext)
        {
            FFXFMLDISK     *pLow;
            FFXFMLDISK     *pHigh;
            unsigned        nThisFirst = pThis->nStartDevNum;
            unsigned        nThisLast = pThis->nStartDevNum + pThis->nDevCount - 1;
            FFXFMLLAYOUT   *pLastLayout;

            /*  Eliminate the simple cases first -- where the mappings
                clearly can't overlap, because they are using completely
                different Devices.
            */
            if( (nThisLast < nFirst) || (nThisFirst > nLast) )
                continue;

            /*  To simplify this logic, we're going to determine which of the
                two Disk mappings has the lower Device number and/or starting
                block offset, in the event that they start in the same Device.
            */
            if(nFirst < nThisFirst)
            {
                /*  New new mapping is lower than an existing mapping
                */
                pLow = pFML;
                pHigh = pThis;
            }
            else if(nFirst > nThisFirst)
            {
                /*  New new mapping is higher than an existing mapping
                */
                pLow = pThis;
                pHigh = pFML;
            }
            else
            {
                /*  New new mapping starts in the exact same Device as the
                    existing mapping, so check the block offset.
                */
                if(pFML->pLayout->ulBlockOffset < pThis->pLayout->ulBlockOffset)
                {
                    pLow = pFML;
                    pHigh = pThis;
                }
                else if(pFML->pLayout->ulBlockOffset > pThis->pLayout->ulBlockOffset)
                {
                    pLow = pThis;
                    pHigh = pFML;
                }
                else
                {
                    /*  The two mappings start in the exact same block, in the
                        exact same Device, so it must be an error.
                    */
                    goto OverlapError;
                }
            }

            if(pLow->nStartDevNum + pLow->nDevCount - 1 > pHigh->nStartDevNum)
            {
                /*  The lower mapping extends beyond the Device which contains
                    the start of the higher mapping, so error out.
                */
                goto OverlapError;
            }

            /*  By the time we get to this point, we've ruled out three
                situations:
                1) The higher mapping starts in a Device which is higher than
                   the highest Device in the lower mapping (NO ERROR).
                2) The higher mapping starts in a Device which is lower than
                   the highest Device in the lower mapping (ERROR).
                3) The lower and higher mapping start in the exact same block
                   in the exact same Device (ERROR).

                The only scenario which remains is that where the two mappings
                start in the same Device, but different blocks.
            */

            /*  Find the last layout structure for the lower mapping.
            */
            pLastLayout = pLow->pLayout;
            while(pLastLayout->pNext)
                pLastLayout = pLastLayout->pNext;

            /*  Verify that the last block used by the lower mapping is less
                than the first block used by the higher mapping.  If not,
                error out.
            */
            if(pLastLayout->ulBlockOffset + pLastLayout->ulBlockCount > pHigh->pLayout->ulBlockOffset)
                goto OverlapError;
        }

        if(nFlags & FFX_DISK_RAW)
            DclPrintf("FFX: Mapping for DISK%u allows RAW access only!\n", nDiskNum);

        pFML->pNext = pChainHead;
        pChainHead = pFML;
        nNumCurrentMappings++;
        aFMLHandles[nDiskNum] = pFML;

        DclMutexRelease(pFMLMutex);

        goto CreateCleanup;
    }
    else
    {
        DclError();
        goto ErrorCleanup;
    }

  OverlapError:

    FFXPRINTF(1, ("FML DISK%u block offset/count overlaps the mapping for DISK%U\n",
        nDiskNum, pThis->nDiskNum));

  ErrorCleanup:

    /*  Free resources in reverse order of allocation
    */
    if(fMutexAcquired)
        DclMutexRelease(pFMLMutex);

    if(pFML)
    {
        FreeLayoutStructs(pFML);

        DclMemFree(pFML);

        pFML = NULL;
    }

    if(pFMLMutex && !nNumCurrentMappings)
    {
        DclMutexDestroy(pFMLMutex);
        pFMLMutex = NULL;
    }

  CreateCleanup:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlCreate() returning hFML=%P\n", pFML));

    return pFML;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDestroy()

    Destroy an FML instance which was created with FfxFmlCreate().

    Parameters:
        hFML - The handle for the FML instance to destroy.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDestroy(
    FFXFMLHANDLE    hFML)
{
    FFXFMLDISK     *pThis;
    FFXFMLDISK     *pPrev;
	FFXSTATUS       ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFmlDestroy() hFML=%P\n", hFML));

    /*  Reality checks
    */
    DclAssert(hFML);
    DclAssert(hFML->nDiskNum < FFX_MAX_DISKS);
    DclAssert(aFMLHandles[hFML->nDiskNum] == hFML);
    DclAssert(pFMLMutex);
    DclAssert(pChainHead);
    DclAssert(nNumCurrentMappings);

    if(!DclMutexAcquire(pFMLMutex))
    {
        FFXPRINTF(1, ("FfxFmlDestroy(): Mutex acquire failed\n"));
        ffxStat = DCLSTAT_MUTEXACQUIREFAILED;
        goto ErrorExit;
    }

  #if FFXCONF_STATS_FML
    if(hFML->stats.pBlockStats)
        DclMemFree(hFML->stats.pBlockStats);
  #endif

    pThis = pChainHead;
    pPrev = NULL;
    while(pThis)
    {
        /*  Find our entry in the linked list and unlink ourselves
        */
        if(pThis == hFML)
        {
            if(pPrev)
                pPrev->pNext = pThis->pNext;
            else
                pChainHead = pThis->pNext;

            nNumCurrentMappings--;
            break;
        }

        pPrev = pThis;
        pThis = pThis->pNext;
    }
    DclAssert(pThis);

    ffxStat = FFXSTAT_SUCCESS;
    aFMLHandles[hFML->nDiskNum] = NULL;

    /*  Free resources in reverse order of allocation
    */
    DclMutexRelease(pFMLMutex);

    FreeLayoutStructs(hFML);

    DclMemFree(hFML);

    if(!nNumCurrentMappings)
    {
        DclMutexDestroy(pFMLMutex);
        pFMLMutex = NULL;
    }

  ErrorExit:

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFmlDestroy() returning Status %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlHandle()

    Obtain the FFXFMLHANDLE for a given Disk number.

    Parameters:
        nDiskNum - The Disk number

    Return Value:
        Returns an FFXFMLHANDLE if successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXFMLHANDLE FfxFmlHandle(
    unsigned        nDiskNum)
{
    if(nDiskNum >= FFX_MAX_DISKS)
    {
        FFXPRINTF(1, ("FfxFmlHandle() FML DISK%U out of range (%u)\n", nDiskNum, FFX_MAX_DISKS));
        return NULL;
    }

    return aFMLHandles[nDiskNum];
}


/*-------------------------------------------------------------------
    Public: FfxFmlDeviceInfo()

    Returns information about a physical Device (DEVn)

    The Device specified by nDeviceNum must already be created before
    calling this function.

    Parameters:
        nDeviceNum - The Device number
        pFDI       - A pointer to the FFXFMLDEVINFO structure to
                     fill

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDeviceInfo(
    unsigned            nDeviceNum,
    FFXFMLDEVINFO      *pFDI)
{
    FFXFIMDEVINFO       di;
    FFXFMLDISK         *pThis;

    if(!pFDI)
    {
        FFXPRINTF(1, ("FfxFmlDeviceInfo() invalid pFDI parameter\n"));
        return FFXSTAT_FML_DEVICENOTVALID;
    }

    DclMemSet(pFDI, 0, sizeof *pFDI);

    /*  Typecast is OK because nDeviceNum is limited elsewhere to be less than
        FFX_MAX_DEVICES, which if greater than a certain threshold will cause
        the compiler preprocessor to quit in the driver framework.
    */
    pFDI->hFimDev = FfxDevHandle((D_UINT16)nDeviceNum);
    if(!pFDI->hFimDev)
    {
        FFXPRINTF(1, ("FfxFmlDeviceInfo: Device %u is invalid", nDeviceNum ));
        return FFXSTAT_FML_DEVICENOTVALID;
    }

    if(!FfxDevInfo(pFDI->hFimDev, &di))
        return FFXSTAT_FML_INFOFAILED;

    /*  If the mutex has not been created there are no disks mapped.
    */
    if(pFMLMutex)
    {
        if(!DclMutexAcquire(pFMLMutex))
        {
            FFXPRINTF(1, ("FfxFmlDeviceInfo() Could not acquire mutex\n"));
            return DCLSTAT_MUTEXACQUIREFAILED;
        }

        pThis = pChainHead;
        while(pThis)
        {
            if( (nDeviceNum >= pThis->nStartDevNum) &&
                (nDeviceNum < pThis->nStartDevNum + pThis->nDevCount) )
            {
                pFDI->nDisksMapped++;
            }

            pThis = pThis->pNext;
        }
        DclMutexRelease(pFMLMutex);
    }

/*    DclAssert(di.uLockFlags <= UINT_MAX);*/

    pFDI->uDeviceType           = di.uDeviceType;
    pFDI->uDeviceFlags          = di.uDeviceFlags;
    pFDI->nDeviceNum            = di.nDeviceNum;
    pFDI->nLockFlags            = (unsigned)di.uLockFlags;
  #if FFXCONF_OTPSUPPORT
    pFDI->nOTPPages             = di.nOTPPages;
  #endif
    pFDI->ulReservedBlocks      = di.ulReservedBlocks;
    pFDI->ulRawBlocks           = di.ulRawBlocks;      /* Includes BBM blocks */
    pFDI->ulTotalBlocks         = di.ulTotalBlocks;    /* Excludes BBM blocks */
    pFDI->ulChipBlocks          = di.ulChipBlocks;
    pFDI->ulBlockSize           = di.ulBlockSize;
    pFDI->uPageSize             = di.uPageSize;
    pFDI->uSpareSize            = di.uSpareSize;
    pFDI->uMetaSize             = di.uMetaSize;
    pFDI->uAlignSize            = di.uAlignSize;
    pFDI->uEdcSegmentSize       = di.uEdcSegmentSize;
    pFDI->uEdcCapability        = di.uEdcCapability;     /* Max error correcting capability */
    pFDI->uEdcRequirement       = di.uEdcRequirement;    /* Required level of error correction */
    pFDI->ulEraseCycleRating    = di.ulEraseCycleRating;  /* Factory spec erase cycle rating */
    pFDI->ulBBMReservedRating   = di.ulBBMReservedRating; /* Reserved blocks required for above */

  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    /*  These fields always exist in the FFXFMLDEVINFO structure
        even if NOR support is not enabled, since this is a customer
        visible structure, and we want it to be consistent.  They
        will simply be zero for non-NOR devices.

        Note that the FFXFIMDEVINFO is an internal structure, and
        the fields will exist only if NOR support is enabled.
    */
    pFDI->ulBootBlockSize       = di.ulBootBlockSize;
    pFDI->uLowBootBlockCount    = di.uLowBootBlockCount;
    pFDI->uHighBootBlockCount   = di.uHighBootBlockCount;
  #endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxFmlDiskInfo()

    Return information about a Disk.

    Parameters:
        hFML    - The FML handle
        pFI     - A pointer to the FFXFMLINFO structure to fill

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlDiskInfo(
    FFXFMLHANDLE    hFML,
    FFXFMLINFO     *pFI)
{
    FFXFIMDEVINFO   di;

    if(!hFML)
    {
        FFXPRINTF(1, ("FfxFmlDiskInfo() Invalid FML handle\n"));
        return FFXSTAT_BADPARAMETER;
    }

    if(!pFI)
    {
        FFXPRINTF(1, ("FfxFmlDiskInfo() Invalid pFI parameter\n"));
        return FFXSTAT_FML_DEVICENOTVALID;
    }

    if(!FfxDevInfo(hFML->pLayout->hFimDev, &di))
        return FFXSTAT_FML_INFOFAILED;

    DclMemSet(pFI, 0, sizeof *pFI);

    pFI->uDeviceType    = di.uDeviceType;
    pFI->ulBlockSize    = di.ulBlockSize;
    pFI->uPageSize      = di.uPageSize;
    pFI->uSpareSize     = di.uSpareSize;
/*
    DclAssert(hFML->ulBlockCount <=
        ((hFML->uDiskFlags & DISK_RAWACCESSONLY) ? di.ulRawBlocks : di.ulTotalBlocks));
*/
    pFI->ulTotalBlocks  = hFML->ulTotalBlocks;
    pFI->uDiskFlags     = hFML->uDiskFlags;
    pFI->nDiskNum       = hFML->nDiskNum;
    pFI->hDisk          = hFML->hDisk;
    pFI->nDeviceNum     = hFML->nStartDevNum;
    pFI->nDeviceCount   = hFML->nDevCount;
    pFI->ulStartBlock   = hFML->pLayout->ulBlockOffset;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxFmlParameterGet()

    Get a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFML      - The FML handle.
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
FFXSTATUS FfxFmlParameterGet(
    FFXFMLHANDLE    hFML,
    FFXPARAM        id,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxFmlParameterGet() hFML=%P ID=%x pBuff=%P Len=%lU\n",
        hFML, id, pBuffer, ulBuffLen));

    if(!hFML)
    {
        FFXPRINTF(1, ("FfxFmlParameterGet() ID=%u, Invalid FML handle\n", id));
        return FFXSTAT_BADPARAMETER;
    }

    if(id <= FFXPARAM_STARTOFLIST || id >= FFXPARAM_ENDOFLIST)
    {
        FFXPRINTF(1, ("FfxFmlParameterGet() ID parameter (%u) is invalid\n", id));
        return FFXSTAT_BADPARAMETER;
    }

    switch(id)
    {
        case FFXPARAM_FIM_CHIPID:
        {
            FFXFMLLAYOUT   *pLayout;
            D_UINT32        ulBuffRemaining = ulBuffLen;
            unsigned        nDevsRemaining;
            D_BUFFER       *pTempBuff = (D_BUFFER*)pBuffer;

            if(!pBuffer)
            {
                DclAssert(!ulBuffLen);

                /*  Return a length long enough to query the chip ID values
                    for all the Devices which this Disk uses.
                */                    
                ffxStat = DCLSTAT_SETUINT20(hFML->nDevCount * FFXPARAM_CHIPID_LENGTH);
                break;
            }

            /*  If this Disk does not span multiple Devices, or if there
                are not enough ID bytes for more than one standard length
                ID, just use the default handler to pass this through to
                the lower level code.
            */                
            if(hFML->nDevCount <= 1 || ulBuffLen <= FFXPARAM_CHIPID_LENGTH)
                goto DefaultCase;

            DclAssert(pBuffer);

            /*  Zero the full buffer in case we run out of Devices early,
                or if there is an early exit due to a failure status code.
            */
            DclMemSet(pBuffer, 0, ulBuffLen);

            /*  Iterate through the layout structures until we find the
                one which contains the first page we want to access.
            */
            nDevsRemaining = hFML->nDevCount;
            pLayout = hFML->pLayout;
            while(ulBuffRemaining && nDevsRemaining)
            {
                D_UINT32 ulThisLen = DCLMIN(ulBuffRemaining, FFXPARAM_CHIPID_LENGTH);

                DclAssert(pLayout);

                ffxStat = FfxDevParameterGet(pLayout->hFimDev, id, pTempBuff, ulThisLen);
                if(ffxStat != FFXSTAT_SUCCESS)
                    break;
                
                ulBuffRemaining -= ulThisLen;
                pTempBuff += ulThisLen;
                nDevsRemaining--;

                pLayout = pLayout->pNext;
            }

            break;
        }
            
        case FFXPARAM_FIM_CHIPSN:
        {
            FFXFMLLAYOUT   *pLayout;
            D_UINT32        ulBuffRemaining = ulBuffLen;
            unsigned        nDevsRemaining;
            D_BUFFER       *pTempBuff = (D_BUFFER*)pBuffer;

            if(!pBuffer)
            {
                DclAssert(!ulBuffLen);

                /*  Return a length long enough to query the chip SN values
                    for all the Devices which this Disk uses.
                */                    
                ffxStat = DCLSTAT_SETUINT20(hFML->nDevCount * FFXPARAM_CHIPSN_LENGTH);
                break;
            }

            /*  If this Disk does not span multiple Devices, or if there
                are not enough SN bytes for more than one standard length
                SN, just use the default handler to pass this through to
                the lower level code.
            */                
            if(hFML->nDevCount <= 1 || ulBuffLen <= FFXPARAM_CHIPSN_LENGTH)
                goto DefaultCase;

            DclAssert(pBuffer);

            /*  Zero the full buffer in case we run out of Devices early,
                or if there is an early exit due to a failure status code.
            */
            DclMemSet(pBuffer, 0, ulBuffLen);

            /*  Iterate through the layout structures until we find the
                one which contains the first page we want to access.
            */
            nDevsRemaining = hFML->nDevCount;
            pLayout = hFML->pLayout;
            while(ulBuffRemaining && nDevsRemaining)
            {
                D_UINT32 ulThisLen = DCLMIN(ulBuffRemaining, FFXPARAM_CHIPSN_LENGTH);

                DclAssert(pLayout);

                ffxStat = FfxDevParameterGet(pLayout->hFimDev, id, pTempBuff, ulThisLen);
                if(ffxStat != FFXSTAT_SUCCESS)
                    break;
                
                ulBuffRemaining -= ulThisLen;
                pTempBuff += ulThisLen;
                nDevsRemaining--;

                pLayout = pLayout->pNext;
            }

            break;
        }
            
        case FFXPARAM_STATS_FML:
        case FFXPARAM_STATS_FMLRESET:
          #if FFXCONF_STATS_FML
            if(!pBuffer)
            {
                /*  A buffer was not supplied -- return an indicator
                    that the parameter is valid and the buffer length
                    required to hold it.
                */
                ffxStat = DCLSTAT_SETUINT20(sizeof(hFML->stats));
            }
            else
            {
                if(ulBuffLen == sizeof(hFML->stats))
                {
                    FFXFMLBLOCKSTATS   *pTmpStats;
                    D_UINT32            ulTmpCount;
                    D_UINT32            ulTmpStart;
                    FFXFMLSTATS        *pClientBuff = (FFXFMLSTATS*)pBuffer;

                    if(pClientBuff->ulBlockStatsCount)
                    {
                        DclAssert(pClientBuff->pBlockStats);

                        DclMemSet(pClientBuff->pBlockStats, 0,
                            sizeof(*pClientBuff->pBlockStats) * pClientBuff->ulBlockStatsCount);

                        /*  The caller might not want to start with the first
                            block, since he could be making iterative requests
                            to get all the block erase count data...
                        */
                        if(hFML->stats.ulBlockStatsCount > pClientBuff->ulBlockStatsStart)
                        {
                            pClientBuff->ulBlockStatsCount =
                                DCLMIN(pClientBuff->ulBlockStatsCount,
                                    hFML->stats.ulBlockStatsCount - pClientBuff->ulBlockStatsStart);

                            DclMemCpy(pClientBuff->pBlockStats,
                                &hFML->stats.pBlockStats[pClientBuff->ulBlockStatsStart],
                                sizeof(*pClientBuff->pBlockStats) * pClientBuff->ulBlockStatsCount);
                        }
                        else
                        {
                            /*  Request is out of range, return 0
                            */
                            pClientBuff->ulBlockStatsCount = 0;
                        }
                    }

                    /*  We need the entire contents of the stats structure
                        except the pointer and count fields for block erases.
                        put them in temporary variables, do a structure copy,
                        then patch them back into the client buffer.
                    */
                    pTmpStats = pClientBuff->pBlockStats;
                    ulTmpCount = pClientBuff->ulBlockStatsCount;
                    ulTmpStart = pClientBuff->ulBlockStatsStart;

                    *pClientBuff = hFML->stats;

                    pClientBuff->pBlockStats = pTmpStats;
                    pClientBuff->ulBlockStatsCount = ulTmpCount;
                    pClientBuff->ulBlockStatsStart = ulTmpStart;

                    if(id == FFXPARAM_STATS_FMLRESET)
                        ResetFMLStats(hFML);

                    ffxStat = FFXSTAT_SUCCESS;
                }
                else
                {
                    ffxStat = FFXSTAT_BADPARAMETERLEN;
                }
            }
          #else
            ffxStat = FFXSTAT_CATEGORYDISABLED;
          #endif
            break;

        case FFXPARAM_STATS_RESETALL:
          #if FFXCONF_STATS_FML
              ResetFMLStats(hFML);
          #endif

            /*  Fall through
                    .
                    .
                    .
            */

        default:
          DefaultCase:
            /*  Not a parameter ID we recognize, so pass the request to
                the layer below us.  Note that this will always pass the
                request to the first Device, should the Disk be spanning
                multiple Devices.
            */
            ffxStat = FfxDevParameterGet(hFML->pLayout->hFimDev, id, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxFmlParameterGet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlParameterSet()

    Set a FlashFX system parameter.  If the parameter ID is not
    recognized for this layer, the request will be passed to the
    next lower software layer.

    Parameters:
        hFML      - The FML handle.
        id        - The parameter identifier (FFXPARAM_*).
        pBuffer   - A pointer to the parameter data.
        ulBuffLen - The parameter data length.

    Return Value:
        Returns an FFXSTATUS code indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlParameterSet(
    FFXFMLHANDLE    hFML,
    FFXPARAM        id,
    const void     *pBuffer,
    D_UINT32        ulBuffLen)
{
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS; /* Init'd for an overly picky compiler */

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxFmlParameterSet() hFML=%P ID=%x pBuff=%P Len=%lU\n",
        hFML, id, pBuffer, ulBuffLen));

    if(!hFML)
    {
        FFXPRINTF(1, ("FfxFmlParameterSet() Invalid FML handle\n"));
        return FFXSTAT_BADHANDLE;
    }

    if(id <= FFXPARAM_STARTOFLIST || id >= FFXPARAM_ENDOFLIST)
    {
        FFXPRINTF(1, ("FfxFmlParameterSet() id parameter (%u) is invalid\n", id));
        return FFXSTAT_OUTOFRANGE;
    }

    switch(id)
    {
        /*  Not a parameter ID we recognize, so pass the request to
            the layer below us.
        */
        default:
        {
            FFXFMLLAYOUT   *pLayout;
            unsigned        nDevsRemaining;

            /*  Iterate through the layout structures sending the command
                to each one in turn.
            */
            nDevsRemaining = hFML->nDevCount;
            pLayout = hFML->pLayout;
            while(nDevsRemaining)
            {
                DclAssertReadPtr(pLayout, sizeof(*pLayout));

                ffxStat = FfxDevParameterSet(pLayout->hFimDev, id, pBuffer, ulBuffLen);
                if(ffxStat != FFXSTAT_SUCCESS)
                    break;
                
                nDevsRemaining--;

                pLayout = pLayout->pNext;
            }

            break;
        }
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxFmlParameterSet() returning status=%lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxFmlIORequest()

    Dispatch an FML level I/O request.  See fxiosys.h for a
    description of the various I/O request functions.
    Alternatively use the FML macro interface in fxfmlapi.h
    to dispatch requests.

    Parameters:
        hFML    - The FML handle
        pIOR    - A pointer to the FFXIOREQUEST structure to use

    Return Value:
        Returns an FFXIOSTATUS structure indicating the results of
        the request.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxFmlIORequest(
    FFXFMLHANDLE    hFML,
    FFXIOREQUEST   *pIOR)
{
    FFXIOSTATUS     ioStat;

    if(!hFML)
    {
        FFXPRINTF(1, ("FfxFmlIORequest() Invalid FML handle\n"));
        return gsioBlockBadParam;
    }

    if(!pIOR)
    {
        FFXPRINTF(1, ("FfxFmlIORequest() Invalid pIOR parameter\n"));
        return gsioBlockBadParam;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEINDENT),
        "FfxFmlIORequest() hFML=%P Func=%x\n", hFML, pIOR->ioFunc));

    DclProfilerEnter("FfxFmlIORequest", 0, 0);

  #if FFXCONF_NANDSUPPORT
    if(hFML->uDiskFlags & DISK_RAWACCESSONLY)
    {
        /*  Raw access mode is forced on when someone maps an FML window to
            an area of the flash which includes the BBM area.  This means
            that the window size is bounded by FFXFIMDEVINFO->ulRawBlocks,
            rather than FFXFIMDEVINFO->ulTotalBlocks.  These will only be
            be different when using NAND, and BBM has reduced the available
            blocks on the device.

            Since the developer has chosen to map a window which includes
            part or all of the BBM area, we're assuming he really knows
            what he is doing, and doesn't care about BBM or VBF, and
            probably just wants raw access to the flash.  Therefore only
            allow RawPageRead, RawPageWrite, and Erase functionality.
        */
        if((pIOR->ioFunc != FXIOFUNC_FML_ERASE_RAWBLOCKS) &&
            (pIOR->ioFunc != FXIOFUNC_FML_READ_RAWPAGES) &&
            (pIOR->ioFunc != FXIOFUNC_FML_WRITE_RAWPAGES))
        {
            DclPrintf("FFX: The mapping for DISK%u allows RAW Read/Write/Erase access only.\n", hFML->nDiskNum);
            DclPrintf("     FfxFmlIORequest() function %x is not supported in this mode.\n", pIOR->ioFunc);

            /*  Don't assert here because we have tools which we want to
                run without problems.
            */
            /*
                DclError();
            */

            ioStat.ffxStat = FFXSTAT_FML_RAWACCESSONLY;

            goto IORequestCleanup;
        }
    }
  #endif

    switch(pIOR->ioFunc)
    {
        case FXIOFUNC_FML_ERASE_BLOCKS:
        {
            ioStat = EraseBlocks(hFML, (FFXIOR_FML_ERASE_BLOCKS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_LOCK_FREEZE:
        {
            ioStat = FfxFmlBlockLockFreeze(hFML, (FFXIOR_FML_LOCK_FREEZE*)pIOR);
            break;
        }
        case FXIOFUNC_FML_LOCK_BLOCKS:
        {
            ioStat = FfxFmlBlockLock(hFML, (FFXIOR_FML_LOCK_BLOCKS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_UNLOCK_BLOCKS:
        {
            ioStat = FfxFmlBlockUnlock(hFML, (FFXIOR_FML_UNLOCK_BLOCKS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_PAGES:
        {
            ioStat = ReadPages(hFML, (FFXIOR_FML_READ_PAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_PAGES:
        {
            ioStat = WritePages(hFML, (FFXIOR_FML_WRITE_PAGES*)pIOR);
            break;
        }

        /*  In order to simplify higher level code, the read/write
            uncorrected page interface is supported regardless whether
            NAND or NOR is being used.  A run-time determination is
            used (if necessary) to direct the code down the right path.
        */
        case FXIOFUNC_FML_READ_UNCORRECTEDPAGES:
        {
            FFXIOR_FML_READ_UNCORRECTEDPAGES *pReq = (FFXIOR_FML_READ_UNCORRECTEDPAGES*)pIOR;

          #if FFXCONF_NANDSUPPORT
            if(hFML->uDiskFlags & DISK_NAND)
            {
                ioStat = FfxFmlNandReadUncorrectedPages(hFML, pReq);
                break;
            }
            else
          #endif
            {
                FFXIOR_FML_READ_PAGES   tmpreq;

                /*  We must only be in this section of code if we are
                    not using NAND, and the pSpare parameter is NULL.

                    All NOR requests are "uncorrected".
                */
                DclAssert(!(hFML->uDiskFlags & DISK_NAND));
                DclAssert(!pReq->pSpare);
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                DclMemSet(&tmpreq, 0, sizeof(tmpreq));

                /*  Change the request into a regular ReadPages request,
                    ignoring the pSpare parameter.
                */
                tmpreq.ior.ulReqLen = sizeof tmpreq;
                tmpreq.ior.ioFunc   = FXIOFUNC_FML_READ_PAGES;
                tmpreq.ulStartPage  = pReq->ulStartPage;
                tmpreq.ulPageCount  = pReq->ulPageCount;
                tmpreq.pPageData    = pReq->pPageData;

                ioStat = ReadPages(hFML, &tmpreq);
                break;
            }
        }
        case FXIOFUNC_FML_WRITE_UNCORRECTEDPAGES:
        {
            FFXIOR_FML_WRITE_UNCORRECTEDPAGES *pReq = (FFXIOR_FML_WRITE_UNCORRECTEDPAGES*)pIOR;

          #if FFXCONF_NANDSUPPORT
            if(hFML->uDiskFlags & DISK_NAND)
            {
                ioStat = FfxFmlNandWriteUncorrectedPages(hFML, pReq);
                break;
            }
            else
          #endif
            {
                FFXIOR_FML_WRITE_PAGES   tmpreq;

                /*  We must only be in this section of code if we are
                    not using NAND, and the pSpare parameter is NULL.

                    All NOR requests are "uncorrected".
                */
                DclAssert(!(hFML->uDiskFlags & DISK_NAND));
                DclAssert(!pReq->pSpare);
                DclAssert(pReq->ior.ulReqLen == sizeof *pReq);

                DclMemSet(&tmpreq, 0, sizeof(tmpreq));

                /*  Change the request into a regular WritePages request,
                    ignoring the pSpare parameter.
                */
                tmpreq.ior.ulReqLen = sizeof tmpreq;
                tmpreq.ior.ioFunc   = FXIOFUNC_FML_WRITE_PAGES;
                tmpreq.ulStartPage  = pReq->ulStartPage;
                tmpreq.ulPageCount  = pReq->ulPageCount;
                tmpreq.pPageData    = pReq->pPageData;

                ioStat = WritePages(hFML, &tmpreq);
                break;
            }
        }

      #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
        case FXIOFUNC_FML_ERASE_BOOT_BLOCKS:
        {
            if(!(hFML->uDiskFlags & DISK_HASBOOTBLOCKS))
            {
                ioStat.ffxStat = FFXSTAT_FML_NOBOOTBLOCKS;
                goto IORequestCleanup;
            }

            ioStat = FfxFmlNorEraseBootBlocks(hFML, (FFXIOR_FML_ERASE_BOOT_BLOCKS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_CONTROLDATA:
        {
            ioStat = FfxFmlNorReadControlData(hFML, (FFXIOR_FML_READ_CONTROLDATA*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_CONTROLDATA:
        {
            ioStat = FfxFmlNorWriteControlData(hFML, (FFXIOR_FML_WRITE_CONTROLDATA*)pIOR);
            break;
        }
      #endif
      
      #if FFXCONF_NANDSUPPORT
        case FXIOFUNC_FML_READ_TAGGEDPAGES:
        {
            ioStat = FfxFmlNandReadTaggedPages(hFML, (FFXIOR_FML_READ_TAGGEDPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_TAGGEDPAGES:
        {
            ioStat = FfxFmlNandWriteTaggedPages(hFML, (FFXIOR_FML_WRITE_TAGGEDPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_ERASE_RAWBLOCKS:
        {
            ioStat = FfxFmlNandEraseRawBlocks(hFML, (FFXIOR_FML_ERASE_RAWBLOCKS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_RAWPAGES:
        {
            ioStat = FfxFmlNandReadRawPages(hFML, (FFXIOR_FML_READ_RAWPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_NATIVEPAGES:
        {
            ioStat = FfxFmlNandReadNativePages(hFML, (FFXIOR_FML_READ_NATIVEPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_RAWPAGES:
        {
            ioStat = FfxFmlNandWriteRawPages(hFML, (FFXIOR_FML_WRITE_RAWPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_NATIVEPAGES:
        {
            ioStat = FfxFmlNandWriteNativePages(hFML, (FFXIOR_FML_WRITE_NATIVEPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_SPARES:
        {
            ioStat = FfxFmlNandReadSpares(hFML, (FFXIOR_FML_READ_SPARES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_SPARES:
        {
            ioStat = FfxFmlNandWriteSpares(hFML, (FFXIOR_FML_WRITE_SPARES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_READ_TAGS:
        {
            ioStat = FfxFmlNandReadTags(hFML, (FFXIOR_FML_READ_TAGS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_WRITE_TAGS:
        {
            ioStat = FfxFmlNandWriteTags(hFML, (FFXIOR_FML_WRITE_TAGS*)pIOR);
            break;
        }

      #if FFXCONF_BBMSUPPORT
        case FXIOFUNC_FML_GET_BLOCK_INFO:
        {
            ioStat = FfxFmlNandGetBlockInfo(hFML, (FFXIOR_FML_GET_RAW_BLOCK_INFO*)pIOR);
            break;
        }
        case FXIOFUNC_FML_GET_RAW_BLOCK_INFO:
        {
            ioStat = FfxFmlNandGetRawBlockInfo(hFML, (FFXIOR_FML_GET_RAW_BLOCK_INFO*)pIOR);
            break;
        }
        case FXIOFUNC_FML_RETIRE_RAW_BLOCK:
        {
            ioStat = FfxFmlRetireRawBlock(hFML, (FFXIOR_FML_RETIRE_RAW_BLOCK*)pIOR);
            break;
        }
      #endif

        case FXIOFUNC_FML_GET_BLOCK_STATUS:
        {
            ioStat = FfxFmlNandGetBlockStatus(hFML, (FFXIOR_FML_GET_RAW_BLOCK_STATUS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_GET_RAW_BLOCK_STATUS:
        {
            ioStat = FfxFmlNandGetRawBlockStatus(hFML, (FFXIOR_FML_GET_RAW_BLOCK_STATUS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_SET_RAW_BLOCK_STATUS:
        {
            ioStat = FfxFmlNandSetRawBlockStatus(hFML, (FFXIOR_FML_SET_RAW_BLOCK_STATUS*)pIOR);
            break;
        }
        case FXIOFUNC_FML_GET_RAW_PAGE_STATUS:
        {
            ioStat = FfxFmlNandGetRawPageStatus(hFML, (FFXIOR_FML_GET_RAW_PAGE_STATUS*)pIOR);
            break;
        }
        
      #if FFXCONF_OTPSUPPORT
        case FXIOFUNC_FML_OTP_READPAGES:
        {
            ioStat = FfxFmlOtpReadPages(hFML, (FFXIOR_FML_READ_OTPPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_OTP_WRITEPAGES:
        {
            ioStat = FfxFmlOtpWritePages(hFML, (FFXIOR_FML_WRITE_OTPPAGES*)pIOR);
            break;
        }
        case FXIOFUNC_FML_OTP_LOCK:
        {
            ioStat = FfxFmlOtpLock(hFML, (FFXIOR_FML_LOCK_OTP*)pIOR);
            break;
        }
      #endif

      #endif /* FFXCONF_NANDSUPPORT */

        default:
        {
            FFXPRINTF(1, ("FfxFmlIORequest() Unknown function 0x%x\n", pIOR->ioFunc));
            ioStat = gsioUnsupported;
            break;
        }
    }

  IORequestCleanup:

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "FfxFmlIORequest() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: ReadPages()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS ReadPages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_READ_PAGES          *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
        FFXPRINTF(1, ("FfxFml:ReadPages() Parameter block length invalid\n" ));
        return gsioPageStructLen;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFml:ReadPages() hFML=%P StartPage=%lX Count=%lX pData=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData));

    DclProfilerEnter("FfxFml:ReadPages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_READ_GENERIC     DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_READ_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_READ_PAGES;
        DevReq.pPageData    = pFmlReq->pPageData;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulPageReadRequests++;
    hFML->stats.ulPageReadTotal += ioTotal.ulCount;
    if(hFML->stats.ulPageReadMax < ioTotal.ulCount)
        hFML->stats.ulPageReadMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddReads(hFML, pFmlReq->ulStartPage, ioTotal.ulCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "FfxFml:ReadPages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: WritePages()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS WritePages(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_WRITE_PAGES         *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_PAGEIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
       FFXPRINTF(1, ("FfxFml:WritePages() Parameter block length invalid\n" ));
       return gsioPageStructLen;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFml:WritePages() hFML=%P StartPage=%lX Count=%lX pData=%P\n",
        hFML, pFmlReq->ulStartPage, pFmlReq->ulPageCount, pFmlReq->pPageData));

    DclProfilerEnter("FfxFml:WritePages", 0, pFmlReq->ulPageCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartPage >= hFML->ulTotalPages) ||
        (pFmlReq->ulPageCount > hFML->ulTotalPages - pFmlReq->ulStartPage))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_WRITE_GENERIC    DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulPage = pFmlReq->ulStartPage;
        D_UINT32                    ulPagesRemaining = pFmlReq->ulPageCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first page we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulPage >= pLayout->ulPageCount)
        {
            ulPage -= pLayout->ulPageCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_WRITE_GENERIC;
        DevReq.ioSubFunc    = FXIOSUBFUNC_DEV_WRITE_PAGES;
        DevReq.pPageData    = pFmlReq->pPageData;

        while(ulPagesRemaining)
        {
            DevReq.ulStartPage = ulPage + pLayout->ulPageOffset;
            DevReq.ulCount = DCLMIN(pLayout->ulPageCount - ulPage, ulPagesRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount         += ioStat.ulCount;
            ioTotal.ffxStat          = ioStat.ffxStat;
            ioTotal.ulFlags          = ioStat.ulFlags;
            ioTotal.op.ulPageStatus |= ioStat.op.ulPageStatus;

            ulPagesRemaining        -= ioStat.ulCount;

            if(!ulPagesRemaining || !IOSUCCESS(ioStat, DevReq.ulCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            DevReq.pPageData        += ioStat.ulCount << hFML->nBytePageShift;

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulPage = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulPageWriteRequests++;
    hFML->stats.ulPageWriteTotal += ioTotal.ulCount;
    if(hFML->stats.ulPageWriteMax < ioTotal.ulCount)
        hFML->stats.ulPageWriteMax = ioTotal.ulCount;

    FfxFmlBlockStatsAddWrites(hFML, pFmlReq->ulStartPage, ioTotal.ulCount);
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 2, TRACEUNDENT),
        "FfxFml:WritePages() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: EraseBlocks()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static FFXIOSTATUS EraseBlocks(
    FFXFMLHANDLE                    hFML,
    FFXIOR_FML_ERASE_BLOCKS        *pFmlReq)
{
    FFXIOSTATUS                     ioTotal = INITIAL_BLOCKIO_STATUS(FFXSTAT_BADSTRUCLEN);

    DclAssert(hFML);
    DclAssert(pFmlReq);

    if(pFmlReq->ior.ulReqLen != sizeof *pFmlReq)
    {
       FFXPRINTF(1, ("FfxFml:EraseBlocks() Parameter block length invalid\n" ));
       return gsioBlockStructLen;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEINDENT),
        "FfxFml:EraseBlocks() hFML=%P StartBlock=%lX Count=%lX\n",
        hFML, pFmlReq->ulStartBlock, pFmlReq->ulBlockCount));

    DclProfilerEnter("FfxFml:EraseBlocks", 0, pFmlReq->ulBlockCount);

    DclAssert(pFmlReq->ulBlockCount);

    /*  Validate the range
    */
    if((pFmlReq->ulStartBlock >= hFML->ulTotalBlocks) ||
        (pFmlReq->ulBlockCount > hFML->ulTotalBlocks - pFmlReq->ulStartBlock))
    {
        ioTotal.ffxStat = FFXSTAT_FML_OUTOFRANGE;
    }
    else
    {
        FFXIOSTATUS                 ioStat;
        FFXIOR_DEV_ERASE_BLOCKS     DevReq;
        FFXFMLLAYOUT               *pLayout;
        D_UINT32                    ulBlock = pFmlReq->ulStartBlock;
        D_UINT32                    ulBlocksRemaining = pFmlReq->ulBlockCount;

        /*  Iterate through the layout structures until we find the
            one which contains the first block we want to access.
        */
        pLayout = hFML->pLayout;
        while(ulBlock >= pLayout->ulBlockCount)
        {
            ulBlock -= pLayout->ulBlockCount;
            pLayout = pLayout->pNext;
            DclAssert(pLayout);
        }

        DclMemSet(&DevReq, 0, sizeof DevReq);

        DevReq.ior.ulReqLen = sizeof DevReq;
        DevReq.ior.ioFunc   = FXIOFUNC_DEV_ERASE_BLOCKS;

        while(ulBlocksRemaining)
        {
            DevReq.ulStartBlock = ulBlock + pLayout->ulBlockOffset;
            DevReq.ulBlockCount = DCLMIN(pLayout->ulBlockCount - ulBlock, ulBlocksRemaining);

            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);

            ioTotal.ulCount          += ioStat.ulCount;
            ioTotal.ffxStat           = ioStat.ffxStat;
            ioTotal.ulFlags           = ioStat.ulFlags;
            ioTotal.op.ulBlockStatus |= ioStat.op.ulBlockStatus;

            ulBlocksRemaining        -= ioStat.ulCount;

            if(!ulBlocksRemaining || !IOSUCCESS(ioStat, DevReq.ulBlockCount))
                break;

            pLayout = pLayout->pNext;
            DclAssert(pLayout);

            /*  Accesses in subsequent Devices will always be at offset 0
            */
            ulBlock = 0;
        }
    }

    DclProfilerLeave(0UL);

  #if FFXCONF_STATS_FML
    hFML->stats.ulBlockEraseRequests++;
    hFML->stats.ulBlockEraseTotal += ioTotal.ulCount;
    if(hFML->stats.ulBlockEraseMax < ioTotal.ulCount)
        hFML->stats.ulBlockEraseMax = ioTotal.ulCount;

    if(hFML->stats.pBlockStats)
    {
        D_UINT32    jj;

        /*  Normally these will always be the same.  If that ever
            changes, this code will need to be adjusted.
        */
        DclAssert(hFML->stats.ulBlockStatsCount == hFML->ulTotalBlocks);

        /*  Increment the erase count for each block we are about to
            erase, so long as we won't cause the counter to wrap.
        */
        for(jj = pFmlReq->ulStartBlock;
            jj < pFmlReq->ulStartBlock + ioTotal.ulCount;
            jj ++)
        {
            if(hFML->stats.pBlockStats[jj].ulErases != D_UINT32_MAX)
                hFML->stats.pBlockStats[jj].ulErases++;
        }
    }
  #endif

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_FML, 1, TRACEUNDENT),
        "FfxFml:EraseBlocks() returning %s\n", FfxDecodeIOStatus(&ioTotal)));

    return ioTotal;
}


/*-------------------------------------------------------------------
    Local: FreeLayoutStructs()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void FreeLayoutStructs(
    FFXFMLDISK         *pFML)
{
    DclAssert(pFML);

    /*  We need to free all the structures in a singly linked list,
        however we want to free them in reverse order of allocation,
        therefore we need to have two loops.
    */
    while(pFML->pLayout)
    {
        FFXFMLLAYOUT   *pPrev = NULL;
        FFXFMLLAYOUT   *pThis = pFML->pLayout;

        while(pThis->pNext)
        {
            /*  Loop until we find the last entry in the chain
            */
            pPrev = pThis;
            pThis = pThis->pNext;
        }

        /*  Free the last entry
        */
        DclMemFree(pThis);

        if(pPrev)
        {
            DclAssert(pPrev->pNext == pThis);
            pPrev->pNext = NULL;
        }
        else
        {
            DclAssert(pFML->pLayout == pThis);
            pFML->pLayout = NULL;
        }
    }

    return;
}


#if FFXCONF_STATS_FML

/*-------------------------------------------------------------------
    Private: FfxFmlBlockStatsAddReads()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxFmlBlockStatsAddReads(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount)
{
    if(hFML->stats.pBlockStats)
    {
        D_UINT32    ulBlock = ulStartPage / hFML->ulPagesPerBlock;
        D_UINT32    ulBlockPage = ulStartPage % hFML->ulPagesPerBlock;

        while(ulPageCount)
        {
            D_UINT32    ulPagesThisBlock = DCLMIN(hFML->ulPagesPerBlock - ulBlockPage, ulPageCount);

            /*  Do not add stats out of range.  It is possible to receive
                requests that are out of bounds if the caller is performing
                raw access on an FML disk that is not raw.
            */
            if(ulBlock >= hFML->stats.ulBlockStatsCount)
                break;

            hFML->stats.pBlockStats[ulBlock].ulReads += ulPagesThisBlock;

            ulBlock++;
            ulBlockPage = 0;
            ulPageCount -= ulPagesThisBlock;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Private: FfxFmlBlockStatsAddWrites()

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxFmlBlockStatsAddWrites(
    FFXFMLHANDLE    hFML,
    D_UINT32        ulStartPage,
    D_UINT32        ulPageCount)
{
    if(hFML->stats.pBlockStats)
    {
        D_UINT32    ulBlock = ulStartPage / hFML->ulPagesPerBlock;
        D_UINT32    ulBlockPage = ulStartPage % hFML->ulPagesPerBlock;

        while(ulPageCount)
        {
            D_UINT32    ulPagesThisBlock = DCLMIN(hFML->ulPagesPerBlock - ulBlockPage, ulPageCount);

            /*  Do not add stats out of range.  It is possible to receive
                requests that are out of bounds if the caller is performing
                raw access on an FML disk that is not raw.
            */
            if(ulBlock >= hFML->stats.ulBlockStatsCount)
                break;

            hFML->stats.pBlockStats[ulBlock].ulWrites += ulPagesThisBlock;

            ulBlock++;
            ulBlockPage = 0;
            ulPageCount -= ulPagesThisBlock;
        }
    }

    return;
}


/*-------------------------------------------------------------------
    Local: ResetFMLStats()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void ResetFMLStats(
    FFXFMLHANDLE        hFML)
{
    FFXFMLBLOCKSTATS   *pTmpStats = hFML->stats.pBlockStats;
    D_UINT32            ulTmpSize = hFML->stats.ulBlockStatsCount;

    if(pTmpStats)
    {
        DclAssert(ulTmpSize);

        DclMemSet(pTmpStats, 0, ulTmpSize * sizeof(*hFML->stats.pBlockStats));
    }

    DclMemSet(&hFML->stats, 0, sizeof(hFML->stats));

    hFML->stats.pBlockStats = pTmpStats;
    hFML->stats.ulBlockStatsCount = ulTmpSize;

    return;
}



#endif


