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

    This module implements the NAND Simulator Error Mechanism layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nsmech.c $
    Revision 1.39  2010/07/13 00:16:30Z  garyp
    Documentation cleanup -- no functional changes.
    Revision 1.38  2009/12/04 01:14:24Z  garyp
    Updated to default to emulating large-block rather than small-block NAND.
    Revision 1.37  2009/08/05 03:27:56Z  garyp
    Corrected a misnamed variable stemming from the previous checkin.
    Revision 1.36  2009/08/04 21:32:05Z  garyp
    Merged from the v4.0 branch.  Updated to build cleanly when support for
    some NAND manufacturers is disabled.  Modified so that if adequate NAND
    manufacturer support is not enabled, a run-time error will occur, rather
    than a compile-time error.
    Revision 1.35  2009/04/15 16:44:06Z  keithg
    Removed DclError() to allow proper error handling in non-debug builds.
    Revision 1.34  2009/03/09 22:23:17Z  billr
    Fix simulation of partial programming.
    Revision 1.33  2009/03/09 20:08:02Z  glenns
    Fixed Bug 2384: adjusted "SetBuffer" to use byte transfers instead of 
    long-word transfers to fill in a test page/spare area buffer, as some 
    spare area sizes we must support are not evenly divisible by four.
    Revision 1.32  2009/02/06 23:05:26Z  billr
    Resolve bug 2236: NAND Simulator 32-bit integer overflow.
    Revision 1.31  2009/02/06 02:25:14Z  keithg
    Updated to reflect new location of NAND header files and macros,.
    Revision 1.30  2009/01/29 17:18:53  keithg
    Corrected printf which was printing invalid stack data.  Changed the
    initialization of a bad block to something a bit more recognizable.
    Revision 1.29  2008/12/03 11:39:16  keithg
    Removed non-standard, non-constant aggregate initialization.
    Revision 1.28  2008/06/16 13:24:05Z  thomd
    Renamed ChipClass field to match higher levels
    Revision 1.27  2008/02/03 04:22:02Z  keithg
    Comment updates to support autodoc.
    Revision 1.26  2008/01/06 16:55:02Z  garyp
    Fixed a memory leak.  Updated to release resources in reverse order of
    allocation.  Added debug code.  Cleaned up documentation.
    Revision 1.25  2007/12/27 00:26:25Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator code is
    now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.24  2007/12/14 23:30:55Z  Garyp
    Modified so the default NAND manufacturer and ID are a bit more abstract.
    Revision 1.23  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.22  2007/10/05 00:35:45Z  billr
    Add capability to simulate partial programming.
    Revision 1.21  2007/09/20 01:22:18Z  pauli
    Added the ability for the simulation mechanism to report to the NTM
    that the flash is "new."
    Revision 1.20  2007/08/30 15:31:07Z  thomd
    Fix to use 64bit Macros where necessary
    Revision 1.19  2007/08/22 20:57:34Z  pauli
    Changed initialization progress messages from debug to standard so
    it displays better.
    Revision 1.18  2007/08/22 19:49:11Z  pauli
    Added debug output to display the simulator configuration and progress
    when initializing new flash.
    Revision 1.17  2007/08/21 21:30:02Z  pauli
    Resolve Bug 1036: Data written is now merged with existing data.
    Added flash initialization progress indication.
    Revision 1.16  2007/08/09 23:43:08Z  timothyj
    Fixed overflow problem where 32-bit types were not promoted to 64-bit
    automatically.
    Revision 1.15  2007/08/01 18:29:28Z  timothyj
    Converted some 64-bit types back to 32-bit and used 64-bit DCL macros for
    all 64-bit operations, to facilitate use in environments where the emulated
    64-bit types and environment-specific 64-bit libraries are used. (BZ #512
    and BZ #803)
    Revision 1.14  2007/07/27 04:09:49Z  timothyj
    Manually aligned structures for ARM compatibility (see BZ #150).
    Allow graceful failure if policy or storage mechanism fail to initialize.
    Modified to use 64-bit macros for some 64 bit math (see BZ #803).
    Updated to use Erase method during setting of initial storage state
    and for erasing blocks.
    Revision 1.13  2007/06/05 00:51:42Z  rickc
    Removed relative path include, fixed in .mak file.
    Revision 1.12  2007/05/11 18:43:01Z  rickc
    Updated to use portable path separators in relative path.
    Revision 1.11  2007/05/10 01:00:15Z  rickc
    Fixed path issue with Platform Builder toolset
    Revision 1.10  2007/03/01 22:02:10Z  timothyj
    Changed pages per block computations to be D_UINT16, to avoid warnings or
    casts for working with the representations of this value in the NTMINFO and
    FIMINFO structs.  Changed references to the chipclass device size to use
    chip blocks (table format changed, see nandid.c).
    Revision 1.9  2007/01/22 23:57:37Z  timothyj
    IR #777, 778, 681: Modified to use new FfxNandDecodeId() that returns a
    reference to a constant FFXNANDCHIP from the table where the ID was located.
    Revision 1.8  2006/12/11 18:42:01Z  billr
    More complete and accurate BBM statistics.
    --- Added comments ---  billr [2006/12/12 23:09:40Z]
    Implement initializing factory bad blocks.
    Revision 1.7  2006/12/06 23:04:08Z  DeanG
    Add new table-driven error policy module
    Revision 1.6  2006/10/20 00:40:33Z  DeanG
    Forward-port simulator persistence enhancement
    Revision 1.5.1.2  2006/10/20 00:40:33Z  DeanG
    Separate out device dimensions into separate structure.
    Revision 1.5  2006/08/26 00:30:03Z  Pauli
    Added compile time setting to allow enabling/disabling the NAND simulator.
    Revision 1.4  2006/07/31 17:39:02Z  Pauli
    Set the default error policy to the Null Policy (no errors).
    Added missing default cases to switch statements.
    Revision 1.3  2006/07/29 01:29:41Z  DeanG
    Changes to accomodate enhanced random policy module
    Revision 1.2  2006/07/28 22:41:25Z  DeanG
    Changes to add "random" policy module
    Revision 1.1  2006/07/26 19:58:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandsim.h>


#if FFXCONF_NANDSUPPORT

#if FFXCONF_NANDSUPPORT_MICRON
  #define DEFAULT_NAND_MFG_ID     (NAND_MFG_MICRON)
  #define DEFAULT_NAND_CHIP_ID    (0xA1)
#elif FFXCONF_NANDSUPPORT_SAMSUNG
  #define DEFAULT_NAND_MFG_ID     (NAND_MFG_SAMSUNG)
  #define DEFAULT_NAND_CHIP_ID    (0x79)
#else
  /*  If these values get used they will cause a run-time
      failure in the simulator Create() function.
  */
  #define DEFAULT_NAND_MFG_ID     (0)
  #define DEFAULT_NAND_CHIP_ID    (0)
#endif


/*  Note this structure has been manually aligned,
    otherwise querying for the contents using FfxHookOptionGet
    could result in an exception on some platforms.
*/
struct tagNsMechanismData
{
    NsPolicyInterfaces     *pPolicy;
    NsPolicyHandle          hPolicy;
    NsStorageInterfaces    *pStorage;
    NsStorageHandle         hStorage;
    const FFXNANDCHIP      *pChipInfo;
    D_UINT32                ulNumChips;
    NsDevDimensions         devDim;
    /*  For alignment purposes, this field is last.  In future
        revisions, this may not be necessary, depending on the
        structure packing and alignment capabilities and requirements
        of the particular platform.
    */
    D_BUFFER                abFlashID[NAND_ID_SIZE];
};


static void overwrite(NsMechanismHandle hMech, D_UINT64 ullOffset, D_BUFFER const *pBuffer, D_UINT32 ulLength);


/*-------------------------------------------------------------------
    Local: SetBuffer()

    This function initializes a buffer with a 32-bit value.

    Parameters:
        pBuffer - a pointer to the buffer to be initialized.
        ulSize  - the size of the buffer in bytes.
        ulValue - the 32-bit value to be used.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void SetBuffer(
    D_BUFFER * pBuffer,
    D_UINT32    ulSize,
    D_UINT32    ulValue)
{
    D_BUFFER   *pData = pBuffer;
    D_UINT32   ulIndex;
    D_BUFFER   ucLocalValue;

    DclAssert(pBuffer);
    DclAssert(ulSize);

    for(ulIndex=0; ulIndex < ulSize; ulIndex++)
    {
        ucLocalValue = (D_BUFFER)(ulValue >> ((ulIndex % sizeof(ulValue))*8));
        *pData++ = ucLocalValue;
    }

    return;
}


/*-------------------------------------------------------------------
    Local: BlockErrors()

    This function sets bits in a block.

    It gets used two ways.  If pAction->error is NsPolicyError_PageBit,
    ulOffset in the NsPolicyBitError identifies a bit within the page
    specified by pageIndex.  If pAction->error is NsPolicyError_BlockBit,
    pageIndex must be zero and ulOffset identifies a bit within the block
    specified by ulBlock.  In either case, ulOffset indexes the bits of
    just the main page(s) or spare area(s), depending on fSpare.

    Parameters:
        hMech     - a handle to the mechanism layer.
        fSpare    - TRUE if the bit to be set is in the spare area.
        pAction   - a pointer to the actions to be taken.
        pageIndex - a page index to be applied.
        ulBlock   - the block number.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void BlockErrors(
    NsMechanismHandle   hMech,
    D_BOOL              fSpare,
    NsPolicyAction    * pAction,
    D_UINT32            pageIndex,
    D_UINT32            ulBlock )
{
    D_UINT32                 errorIndex;
    D_UINT32                 areaSize = fSpare ? hMech->devDim.pageSpareSize
                                               : hMech->devDim.pageDataSize;

    DclAssert( pAction );
    DclAssert(pAction->pBitErrors);
    DclAssert(((pAction->error == NsPolicyError_BlockBit
                || pAction->error == NsPolicyError_PartialProgram)
               && pageIndex == 0)
              || (pAction->error == NsPolicyError_PageBit
                  && pageIndex < hMech->devDim.pagesPerBlock));

    for ( errorIndex = 0;  errorIndex < pAction->ulNumBitErrors;  errorIndex++ )
    {
        NsPolicyBitError const * const pError = &pAction->pBitErrors[errorIndex];
        D_UINT32 byteIndex = pError->ulOffset / CHAR_BIT;
        D_BUFFER bitMask = 1U << (pError->ulOffset % CHAR_BIT);
        D_UINT64 ullOffset;
        D_BUFFER byte;

        /*  Calculate the byte offset within block number ulBlock.
            Note that the error location is an offset within either
            the main or spare area (as governed by areaSize).
        */
        if (pAction->error == NsPolicyError_BlockBit
            || pAction->error == NsPolicyError_PartialProgram)
        {
            /*  The error location is relative to ulBlock.
            */
            DclAssert(byteIndex < hMech->devDim.pagesPerBlock * areaSize);
            byteIndex = ((byteIndex / areaSize) * hMech->devDim.pageTotalSize
                         + (byteIndex % areaSize)
                         + (fSpare ? hMech->devDim.pageDataSize : 0));
        }
        else /* NsPolicyError_PageBit */
        {
            /*  The error location is relative to pageIndex in
                ulBlock.
            */
            DclAssert(byteIndex < areaSize);
            byteIndex += (pageIndex * hMech->devDim.pageTotalSize
                          + (fSpare ? hMech->devDim.pageDataSize : 0));
        }

        /*  Use the block number and byte offset within the block to
            calculate a 64-bit offset within the storage.
        */
        DclUint64AssignUint32(&ullOffset, ulBlock * hMech->devDim.pagesPerBlock);
        DclUint64MulUint32(&ullOffset, (D_UINT32) hMech->devDim.pageTotalSize);
        DclUint64AddUint32(&ullOffset, byteIndex);

        hMech->pStorage->Read(  hMech->hStorage, ullOffset, & byte,  1 );
        switch ( pError->state )
        {
        case NsPolicyBitState_On:
            byte |= bitMask;
            break;
        case NsPolicyBitState_Off:
            byte &= ~bitMask;
            break;
        case NsPolicyBitState_Toggle:
            byte ^= bitMask;
            break;
        default:
            DclProductionError();
            break;
        }

        hMech->pStorage->Write( hMech->hStorage, ullOffset, & byte,  1 );

        FFXPRINTF(3, ("Block %lU, byte %lU, mask %02x, type %d\n",
                      ulBlock, byteIndex, (unsigned) bitMask, pError->state));
    }
}


/*-------------------------------------------------------------------
    Local: EveryPageErrors()

    This function set bits in each page of a block.

    Parameters:
        hMech    - a handle to the mechanism layer.
        fSpare   - TRUE if the bit to be set is in the spare area.
        pAction  - a pointer to the actions to be taken.
        ulBlock  - the block number.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void EveryPageErrors(
    NsMechanismHandle   hMech,
    D_BOOL              fSpare,
    NsPolicyAction    * pAction,
    D_UINT32            ulBlock )
{
    D_UINT16 pageIndex;

    FFXPRINTF(3, ("Every Page errors\n"));

    for ( pageIndex = 0;  pageIndex < hMech->devDim.pagesPerBlock;  pageIndex++ )
    {
        BlockErrors( hMech, fSpare, pAction, pageIndex, ulBlock );
    }

    return;
}


/*-------------------------------------------------------------------
    Public: NsMechanismCreate()

    This function performs whatever initialization the policy layer needs.

    Parameters:
        hDev       - The related FFXDEVHANDLE.
        pfNewFlash - Flag indicating if the simulated flash is newly
                     created or if we are using "used" flash.

    Return Value:
        Returns a handle to the mechanism layer that is used for
        subsequent mechanism layer calls.
-------------------------------------------------------------------*/
NsMechanismHandle NsMechanismCreate(
    FFXDEVHANDLE    hDev,
    D_BOOL         *pfNewFlash)
{
    NsMechanismHandle hMech;
    D_UINT16        pageIndex;
    D_UINT32        blockIndex;
    D_UINT64        ullTotalSize;
    D_UINT64        ullOffset;
    D_UINT16        uLastPercent = 1000;  /* init to something other than 0-100... */
    D_UINT16        uPercent;

    FFXPRINTF(1, ("NAND Simulator Create\n"));

    DclAssert(pfNewFlash);

    hMech = DclMemAllocZero( sizeof( NsMechanismData ) );
    if(!hMech)
    {
        DclPrintf("Failed to allocate memory.\n");
        DclError();
        return NULL;
    }

    /*  Query the FlashFX Options interface for the characteristics for
        this simulated NAND flash array.  For any option that is not
        explicitly set, the defaults below are used.
    */
    if( !FfxHookOptionGet(FFXOPT_NANDSIM_POLICY, hDev, &hMech->pPolicy,
                          sizeof(hMech->pPolicy)) )
    {
        hMech->pPolicy = &NANDSIM_NullPolicy;
    }
    if(!hMech->pPolicy)
    {
        DclPrintf("Invalid error policy.\n");
        DclError();
        DclMemFree(hMech);
        return NULL;
    }
    if( !FfxHookOptionGet(FFXOPT_NANDSIM_STORAGE, hDev, &hMech->pStorage,
                          sizeof(hMech->pStorage)) )
    {
        hMech->pStorage = &NANDSIM_RamStorage;
    }
    if(!hMech->pStorage)
    {
        DclPrintf("Invalid storage layer.\n");
        DclError();
        DclMemFree(hMech);
        return NULL;
    }
    if( !FfxHookOptionGet(FFXOPT_NANDSIM_ID, hDev, hMech->abFlashID, NAND_ID_SIZE) )
    {
        hMech->abFlashID[0] = DEFAULT_NAND_MFG_ID;
        hMech->abFlashID[1] = DEFAULT_NAND_CHIP_ID;

        if(hMech->abFlashID[0] == 0 || hMech->abFlashID[1] == 0)
        {
            DclPrintf("Invalid manufacturer or chip ID.\n");
            DclError();
            DclMemFree(hMech);
            return NULL;
        }
    }
    if( !FfxHookOptionGet(FFXOPT_NANDSIM_NUM_CHIPS, hDev, &hMech->ulNumChips,
                          sizeof(hMech->ulNumChips)) )
    {
        hMech->ulNumChips = 1;
    }

    /*  Decode the flash ID to determine the characteristics of the flash part
        we are simulating.
    */
    hMech->pChipInfo = FfxNandDecodeID(hMech->abFlashID);
    if( !hMech->pChipInfo )
    {
        DclHexDump("Invalid flash id: ", HEXDUMP_UINT8, 6, 6, hMech->abFlashID);
        DclMemFree(hMech);
        return NULL;
    }

    /*  Calculate the information we use about the flash.
    */
    hMech->devDim.totalBlocks   = hMech->pChipInfo->pChipClass->ulChipBlocks * hMech->ulNumChips;
    hMech->devDim.pageDataSize  = hMech->pChipInfo->pChipClass->uPageSize;
    hMech->devDim.pageSpareSize = hMech->pChipInfo->pChipClass->uSpareSize;
    hMech->devDim.pageTotalSize = hMech->devDim.pageDataSize + hMech->devDim.pageSpareSize;
    DclAssert(hMech->pChipInfo->pChipClass->ulBlockSize /
            hMech->pChipInfo->pChipClass->uPageSize <= D_UINT16_MAX);
    DclAssert(hMech->pChipInfo->pChipClass->ulBlockSize %
            hMech->pChipInfo->pChipClass->uPageSize == 0);
    hMech->devDim.pagesPerBlock = (D_UINT16)(hMech->pChipInfo->pChipClass->ulBlockSize /
                                             hMech->pChipInfo->pChipClass->uPageSize);

    FFXPRINTF(1, ("Simulator Configuration\n"));
    FFXPRINTF(1, ("  Number of Chips: %6lU\n",  hMech->ulNumChips));
    FFXPRINTF(1, ("  Page Size:       %6U B\n", hMech->pChipInfo->pChipClass->uPageSize));
    FFXPRINTF(1, ("  Block Size:  %10lU KB\n",  hMech->pChipInfo->pChipClass->ulBlockSize / 1024));
    FFXPRINTF(1, ("  Total Size:  %10lU KB\n",  hMech->devDim.totalBlocks * (hMech->pChipInfo->pChipClass->ulBlockSize / 1024)));
    FFXPRINTF(1, ("  Error Policy:  %16s\n",    hMech->pPolicy->szName));
    FFXPRINTF(1, ("  Storage:       %16s\n",    hMech->pStorage->szName));

    /*  Create the error policy and storage mechanism.
    */
    hMech->hPolicy   = hMech->pPolicy->Create( hDev, &hMech->devDim );
    if( !hMech->hPolicy )
    {
        DclPrintf("NAND simulator error policy creation failed!\n");
        DclError();
        DclMemFree(hMech);
        return NULL;
    }

    DclUint64AssignUint32(&ullTotalSize, hMech->devDim.totalBlocks);
    DclUint64MulUint32(&ullTotalSize, hMech->devDim.pagesPerBlock);
    DclUint64MulUint32(&ullTotalSize, hMech->devDim.pageTotalSize);

    hMech->hStorage  = hMech->pStorage->Create( hDev, ullTotalSize, pfNewFlash );
    if( !hMech->hStorage )
    {
        DclPrintf("NAND simulator storage layer creation failed!\n");
        DclError();
        hMech->pPolicy->Destroy( hMech->hPolicy );
        DclMemFree(hMech);
        return NULL;
    }

    /*  Initialize the flash if this is a new storage area.
    */
    if ( *pfNewFlash )
    {
        D_BUFFER   *pPageWork;

        pPageWork = DclMemAlloc( hMech->devDim.pageTotalSize );
        DclAssert(pPageWork);

        /*  Use a non-FF (easily identifyable) value for marking the
            blocks as bad.
        */
        SetBuffer(pPageWork, hMech->devDim.pageTotalSize, 0xBAD0D0BA);

        DclPrintf("Initializing Simulated NAND Flash Media\n");

        for ( blockIndex = 0;  blockIndex < hMech->devDim.totalBlocks;  blockIndex++ )
        {
            /*  Decide if the block should be initialized to a factory bad
                state or as a good block.
            */
            NsPolicyBlockParam blockParam;
            NsPolicyAction blockAction;

            blockParam.ulSize = 0;
            blockParam.ulBlockNumber = blockIndex;
            blockAction = hMech->pPolicy->BlockInit(hMech->hPolicy, &blockParam);

            /*  It would be better to mark the block exactly as specified for
                the simulated chip, but this will do for now.
            */
            if(blockAction.error == NsPolicyError_FactoryBadBlock)
            {
                FFXPRINTF(1, ("Block %lX initialized as Factory Bad Block.\n", blockIndex));
                for ( pageIndex = 0;  pageIndex < hMech->devDim.pagesPerBlock;  pageIndex++ )
                {
                    DclUint64AssignUint32(&ullOffset, (blockIndex * hMech->devDim.pagesPerBlock));
                    DclUint64AddUint32(&ullOffset, pageIndex);
                    DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);

                    hMech->pStorage->Write( hMech->hStorage, ullOffset,
                                            pPageWork, hMech->devDim.pageTotalSize );
                }
            }
            else
            {

                DclUint64AssignUint32(&ullOffset, (blockIndex * hMech->devDim.pagesPerBlock));
                DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);

                hMech->pStorage->Erase( hMech->hStorage,
                                        ullOffset,
                                        hMech->devDim.pageTotalSize
                                            * hMech->devDim.pagesPerBlock);
            }

            /*  Display the current % complete.
            */
            uPercent = (D_UINT16) ((blockIndex * 100UL) / hMech->devDim.totalBlocks);
            if(uPercent != uLastPercent)
            {
                DclPrintf("%3U%%\b\b\b\b", uPercent);
                uLastPercent = uPercent;
            }
        }

        DclPrintf("100%%\n");

        DclMemFree(pPageWork);
    }

    return hMech;
}


/*-------------------------------------------------------------------
    Public: NsMechanismGetParameters()

    This function provides flash Parameters:.

    Parameters:
        hMech       - a handle to the mechanism layer.
        pID         - pointer to the area into which the Flash ID is
                      returned.
        pulNumChips - pointer to the area into which the number of
                      flash chips is returned.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismGetParameters(
    NsMechanismHandle   hMech,
    D_BUFFER          * pID,
    D_UINT32          * pulNumChips )
{
    DclAssert( hMech );
    DclAssert( pID );
    DclAssert( pulNumChips );

    DclMemCpy(pID, hMech->abFlashID, NAND_ID_SIZE);
    *pulNumChips = hMech->ulNumChips;

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: NsMechanismDestroy()

    This function performs whatever termination cleanup the
    mechanism layer needs.

    Parameters:
        hMech - a handle to the mechanism layer.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismDestroy(
    NsMechanismHandle hMech )
{
    DclAssert( hMech );

    hMech->pStorage->Destroy( hMech->hStorage );
    hMech->pPolicy->Destroy( hMech->hPolicy );

    DclMemFree( hMech );

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: NsMechanismReadPage()

    This function is called to read a page and process a policy action.

    Parameters:
        hMech   - a handle to the mechanism layer.
        ulPage  - the page number to be read.
        pBuffer - a pointer to the buffer to be filled by the read.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismReadPage(
    NsMechanismHandle   hMech,
    D_UINT32            ulPage,
    D_BUFFER          * pBuffer )
{
    NsPolicyPageParam pageParam;
    NsPolicyAction    action;
    FFXSTATUS         status = FFXSTAT_SUCCESS;
    D_UINT64          ullOffset;

    DclAssert( hMech );

    pageParam.ulPageNumber    = ulPage;
    pageParam.pData           = pBuffer;
    pageParam.ulSize          = hMech->devDim.pageDataSize;
    pageParam.uPagesPerBlock  = hMech->devDim.pagesPerBlock;

    DclUint64AssignUint32(&ullOffset, ulPage);
    DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);

    hMech->pStorage->Read( hMech->hStorage,
                           ullOffset,
                           pBuffer,  hMech->devDim.pageDataSize );
    action = hMech->pPolicy->ReadPage( hMech->hPolicy, & pageParam );
    switch ( action.error )
    {
    case NsPolicyError_Timeout:
    case NsPolicyError_IO:
        SetBuffer( pBuffer, hMech->devDim.pageDataSize, 0xDEADDEAD );
        break;
    default:
        break;
    }

    switch ( action.error )
    {
    case NsPolicyError_None:
        break;
    case NsPolicyError_Timeout:
        FFXPRINTF(3, ("Read Page timeout error page %lU\n", ulPage));
        status = FFXSTAT_FIMTIMEOUT;
        break;
    case NsPolicyError_IO:
        FFXPRINTF(3, ("Read Page I/O error page %lU\n", ulPage));
        status = FFXSTAT_FIMIOERROR;
        break;
    case NsPolicyError_PageBit:
        FFXPRINTF(3, ("Read Page disturb error page %lU\n", ulPage));
        EveryPageErrors( hMech, FALSE, & action, ulPage / hMech->devDim.pagesPerBlock );
        break;
    case NsPolicyError_BlockBit:
        FFXPRINTF(3, ("Read Page disturb error page %lU\n", ulPage));
        BlockErrors( hMech, FALSE, & action, 0, ulPage / hMech->devDim.pagesPerBlock );
        break;
    default:
        DclProductionError();
        break;
    }

    return status;
}


/*-------------------------------------------------------------------
    Public: NsMechanismWritePage()

    This function is called to write a page and process a policy action.

    Parameters:
        hMech   - a handle to the mechanism layer.
        ulPage  - the page number to be written.
        pBuffer - a pointer to the buffer to be written.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismWritePage(
    NsMechanismHandle   hMech,
    D_UINT32            ulPage,
    D_BUFFER const    * pBuffer )
{
    NsPolicyPageParam pageParam;
    NsPolicyAction    action;
    FFXSTATUS         status = FFXSTAT_SUCCESS;
    D_UINT64          ullOffset;

    DclAssert( hMech );

    pageParam.ulPageNumber    = ulPage;
    pageParam.pData           = pBuffer;
    pageParam.ulSize          = hMech->devDim.pageDataSize;
    pageParam.uPagesPerBlock  = hMech->devDim.pagesPerBlock;

    DclUint64AssignUint32(&ullOffset, ulPage);
    DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);

    action = hMech->pPolicy->WritePage( hMech->hPolicy, & pageParam );
    switch ( action.error )
    {
    case NsPolicyError_PartialProgram:
        if (action.pBitErrors[0].state == NsPolicyBitState_Off)
            break; /* clear a few bits in an unprogrammed page */
        else
            /* fall through: program page, then set a few bits */
    case NsPolicyError_None:
    case NsPolicyError_PageBit:
    case NsPolicyError_BlockBit:
        overwrite(hMech, ullOffset, pBuffer,  hMech->devDim.pageDataSize);
        break;
    default:
        break;
    }
    switch ( action.error )
    {
    case NsPolicyError_None:
        break;
    case NsPolicyError_Timeout:
        FFXPRINTF(3, ("Write Page timeout error page %lU\n", ulPage));
        status = FFXSTAT_FIMTIMEOUT;
        break;
    case NsPolicyError_IO:
        FFXPRINTF(3, ("Write Page I/O error page %lU\n", ulPage));
        status = FFXSTAT_FIMIOERROR;
        break;
    case NsPolicyError_PageBit:
        FFXPRINTF(3, ("Write Page disturb error page %lU\n", ulPage));
        EveryPageErrors( hMech, FALSE, & action, ulPage / hMech->devDim.pagesPerBlock );
        break;
    case NsPolicyError_BlockBit:
    case NsPolicyError_PartialProgram:
        FFXPRINTF(3, ("Write Page disturb error page %lU\n", ulPage));
        BlockErrors( hMech, FALSE, & action, 0, ulPage / hMech->devDim.pagesPerBlock );
        break;
    default:
        DclProductionError();
        break;
    }

    return status;
}


/*-------------------------------------------------------------------
    Public: NsMechanismReadSpare()

    This function is called to read a spare area and process a policy action.

    Parameters:
        hMech   - a handle to the mechanism layer.
        ulPage  - the page number to be read.
        pBuffer - a pointer to the buffer to be filled by the read.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismReadSpare(
    NsMechanismHandle   hMech,
    D_UINT32            ulPage,
    D_BUFFER          * pBuffer )
{
    NsPolicyPageParam pageParam;
    NsPolicyAction    action;
    FFXSTATUS         status = FFXSTAT_SUCCESS;
    D_UINT64          ullOffset;

    DclAssert( hMech );

    pageParam.ulPageNumber    = ulPage;
    pageParam.pData           = pBuffer;
    pageParam.ulSize          = hMech->devDim.pageSpareSize;
    pageParam.uPagesPerBlock  = hMech->devDim.pagesPerBlock;

    DclUint64AssignUint32(&ullOffset, ulPage);
    DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);
    DclUint64AddUint32(&ullOffset, hMech->devDim.pageDataSize);

    hMech->pStorage->Read( hMech->hStorage,
                           ullOffset,
                           pBuffer,  hMech->devDim.pageSpareSize );
    action = hMech->pPolicy->ReadSpare( hMech->hPolicy, & pageParam );
    switch ( action.error )
    {
    case NsPolicyError_Timeout:
    case NsPolicyError_IO:
        SetBuffer( pBuffer, hMech->devDim.pageSpareSize, 0xDEADDEAD );
        break;
    default:
        break;
    }

    switch ( action.error )
    {
    case NsPolicyError_None:
        break;
    case NsPolicyError_Timeout:
        FFXPRINTF(3, ("Read Spare timeout error page %lU\n", ulPage));
        status = FFXSTAT_FIMTIMEOUT;
        break;
    case NsPolicyError_IO:
        FFXPRINTF(3, ("Read Spare I/O error page %lU\n", ulPage));
        status = FFXSTAT_FIMIOERROR;
        break;
    case NsPolicyError_PageBit:
        FFXPRINTF(3, ("Read Spare disturb error page %lU\n", ulPage));
        EveryPageErrors( hMech, TRUE, & action, ulPage / hMech->devDim.pagesPerBlock );
        break;
    case NsPolicyError_BlockBit:
        FFXPRINTF(3, ("Read Spare disturb error page %lU\n", ulPage));
        BlockErrors( hMech, TRUE, & action, 0, ulPage / hMech->devDim.pagesPerBlock );
        break;
    default:
        DclError();
        break;
    }

    return status;
}


/*-------------------------------------------------------------------
    Public: NsMechanismWriteSpare()

    This function is called to write a spare area and process a policy action.

    Parameters:
        hMech   - a handle to the mechanism layer.
        ulPage  - the page number to be written.
        pBuffer - a pointer to the buffer to be written.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismWriteSpare(
    NsMechanismHandle   hMech,
    D_UINT32            ulPage,
    D_BUFFER const    * pBuffer )
{
    NsPolicyPageParam pageParam;
    NsPolicyAction    action;
    FFXSTATUS         status = FFXSTAT_SUCCESS;
    D_UINT64          ullOffset;

    DclAssert( hMech );

    pageParam.ulPageNumber    = ulPage;
    pageParam.pData           = pBuffer;
    pageParam.ulSize          = hMech->devDim.pageSpareSize;
    pageParam.uPagesPerBlock  = hMech->devDim.pagesPerBlock;

    DclUint64AssignUint32(&ullOffset, ulPage);
    DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);
    DclUint64AddUint32(&ullOffset, hMech->devDim.pageDataSize);

    action = hMech->pPolicy->WriteSpare( hMech->hPolicy, & pageParam );
    switch ( action.error )
    {
    case NsPolicyError_PartialProgram:
        if (action.pBitErrors[0].state == NsPolicyBitState_Off)
            break; /* clear a few bits in an unprogrammed page */
        else
            /* fall through: program page, then set a few bits */
    case NsPolicyError_None:
    case NsPolicyError_PageBit:
    case NsPolicyError_BlockBit:
        overwrite(hMech, ullOffset, pBuffer,  hMech->devDim.pageSpareSize);
        break;
    default:
        break;
    }

    switch ( action.error )
    {
    case NsPolicyError_None:
        break;
    case NsPolicyError_Timeout:
        FFXPRINTF(3, ("Write Spare timeout error page %lU\n", ulPage));
        status = FFXSTAT_FIMTIMEOUT;
        break;
    case NsPolicyError_IO:
        FFXPRINTF(3, ("Write Spare I/O error page %lU\n", ulPage));
        status = FFXSTAT_FIMIOERROR;
        break;
    case NsPolicyError_PageBit:
        FFXPRINTF(3, ("Write Spare disturb error page %lU\n", ulPage));
        EveryPageErrors( hMech, TRUE, & action, ulPage / hMech->devDim.pagesPerBlock );
        break;
    case NsPolicyError_BlockBit:
        FFXPRINTF(3, ("Write Spare disturb error page %lU\n", ulPage));
        BlockErrors( hMech, TRUE, & action, 0, ulPage / hMech->devDim.pagesPerBlock );
        break;
    case NsPolicyError_PartialProgram:
        FFXPRINTF(3, ("Write Spare disturb error page %lU\n", ulPage));
        BlockErrors( hMech, TRUE, & action, 0, ulPage / hMech->devDim.pagesPerBlock );
        DclOsHalt(NULL, 0);
        DclProductionError();
        break;
    default:
        DclError();
        break;
    }

    return status;
}


/*-------------------------------------------------------------------
    Public: NsMechanismEraseBlock()

    This function is called to specify an action when a block is erased.

    Parameters:
        hMech   - a handle to the mechanism layer.
        pParams - a pointer to the parameters to be used.

    Return Value:
        FFXSTATUS value.
-------------------------------------------------------------------*/
FFXSTATUS NsMechanismEraseBlock(
    NsMechanismHandle hMech,
    D_UINT32          ulBlock )
{
    NsPolicyBlockParam blockParam;
    NsPolicyAction    action;
    FFXSTATUS         status = FFXSTAT_SUCCESS;
    D_UINT16          pageIndex;
    D_UINT64          ullOffset;

    DclAssert( hMech );

    blockParam.ulBlockNumber  = ulBlock;
    blockParam.ulSize         = hMech->devDim.pageDataSize
                              * hMech->devDim.pagesPerBlock;

    action = hMech->pPolicy->EraseBlock( hMech->hPolicy, & blockParam );
    switch ( action.error )
    {
    case NsPolicyError_None:
    case NsPolicyError_PageBit:
    case NsPolicyError_BlockBit:
        for ( pageIndex = 0;  pageIndex < hMech->devDim.pagesPerBlock;  pageIndex ++ )
        {
            DclUint64AssignUint32(&ullOffset, ulBlock);
            DclUint64MulUint32(&ullOffset, hMech->devDim.pagesPerBlock);
            DclUint64AddUint32(&ullOffset, pageIndex);
            DclUint64MulUint32(&ullOffset, hMech->devDim.pageTotalSize);

            hMech->pStorage->Erase( hMech->hStorage, ullOffset, hMech->devDim.pageTotalSize);
        }
        break;
    default:
        break;
    }

    switch ( action.error )
    {
    case NsPolicyError_None:
        break;
    case NsPolicyError_Timeout:
        status = FFXSTAT_FIMTIMEOUT;
        break;
    case NsPolicyError_IO:
        status = FFXSTAT_FIMIOERROR;
        break;
    case NsPolicyError_PageBit:
        EveryPageErrors( hMech, FALSE, & action, ulBlock );
        break;
    case NsPolicyError_BlockBit:
        BlockErrors( hMech, FALSE, & action, 0, ulBlock );
        break;
    default:
        DclError();
        break;
    }

    return status;
}


/*-------------------------------------------------------------------
    Local: overwrite()

    Implement standard flash behavior when existing data is
    overwritten, by performing a bitwise AND between the new
    and existing data.

    Parameters:
        hMech        - a handle to the mechanism layer.
        ullOffset    - offset in the storage at which to write
        pBuffer      - new data to write
        ulLength     - number of bytes to write

    Return Value:
        None.
-------------------------------------------------------------------*/
static void overwrite(
    NsMechanismHandle hMech,
    D_UINT64 ullOffset,
    D_BUFFER const *pBuffer,
    D_UINT32 ulLength)
{
    D_BUFFER buf[512];   /* dimension asssumed not to exceed UINT_MAX */

    while (ulLength)
    {
        unsigned uThisLength = (unsigned) DCLMIN(ulLength, sizeof buf);
        unsigned i;

        hMech->pStorage->Read(hMech->hStorage, ullOffset, buf, uThisLength);

        for (i = 0; i < uThisLength; ++i)
            buf[i] &= *pBuffer++;

        hMech->pStorage->Write(hMech->hStorage, ullOffset, buf, uThisLength);

        ulLength -= uThisLength;

        DclUint64AddUint16(&ullOffset, uThisLength);
    }

    return;
}



#endif  /* FFXCONF_NANDSUPPORT */

