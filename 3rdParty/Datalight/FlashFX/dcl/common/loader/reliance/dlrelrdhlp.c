/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This module provides the ability to read a file off a Reliance formatted
    disk into memory.  This functionality is typically used in a boot loader
    environment.

    It has the limitation the file is read sequentially.  There is no random
    access or llseek capability.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelrdhlp.c $
    Revision 1.15  2010/01/23 16:46:25Z  garyp
    Fixed to avoid "possible loss of data" warnings (no actual loss of data).
    Removed obsolete comments.
    Revision 1.14  2009/07/01 19:34:37Z  garyp
    Added local initializations to satisfy picky compilers.
    Revision 1.13  2009/05/26 19:24:48Z  garyp
    Renamed some symbols to use "REL" rather than "TFS" notation.
    Revision 1.12  2009/05/02 08:05:45Z  brandont
    Removed obsolete reference to TFS_DEBUG.
    Revision 1.11  2009/02/18 09:31:49Z  brandont
    Corrected unreferenced variable warning.
    Revision 1.10  2009/01/11 01:18:15Z  brandont
    Updated to use DCL types and defines.
    Revision 1.9  2007/12/03 22:10:49Z  brandont
    Corrected warning for unused argument.
    Revision 1.8  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/10/30 01:32:32Z  jeremys
    Fixed an issue where TfsCoordFileOffsetToBlock was not properly checking
    for end of file.  Fixed an issue where the reader would return end of file
    when zero bytes were being read.
    Revision 1.6  2007/10/05 19:27:55Z  brandont
    Removed conditional for DCLCONF_RELIANCEREADERSUPPORT.
    Revision 1.5  2007/10/03 01:52:11Z  brandont
    Removed the unused function RelReaderGetMemoryRequirements.
    Revision 1.4  2007/10/03 01:17:13Z  brandont
    Renamed FILEINFO to sDlLoaderFile.
    Revision 1.3  2007/10/02 23:13:14Z  brandont
    Renamed FILESTATE_SUCCESS to DL_LOADER_FILESTATE_SUCCESS.
    Renamed FILESTATE_EOF to DL_LOADER_FILESTATE_EOF.
    Renamed FILESTATE_IO_ERROR to DL_LOADER_FILESTATE_IO_ERROR.
    Revision 1.2  2007/09/27 20:24:47Z  jeremys
    Renamed DCLCONF_RELIANCESUPPORT to DCLCONF_RELIANCEREADERSUPPORT.
    Revision 1.1  2007/09/27 01:23:40Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlloader.h>

/*  Note that the order in which these items are included is critical
*/
#include "dlrelopts.h"
#include "dlrelbase.h"
#include "dlreltypes.h"
#include "dlrelmacro.h"
#include "dlrelrdhlp.h"
#include "dlrelcore.h"


#define TFSERR_NONE             0
#define TFSERR_IO               1
#define TFSERR_INTERNAL         2
#define TFSERR_GENERAL          3
#define TFSERR_TOO_FEW_ARGS     4
#define TFSERR_EOF              5
#define TFSERR_BADPATH          6
#define TFSERR_NOTFOUND         7
#define TFSERR_DENIED           8
#define TFSERR_BLOCKSIZE        9
#define TFSERR_WRONGVERSION     10
#define TFSERR_UNFORMATTED      11


/*
*/
#define RelEnter(a)
#define RelLeave()
#define RELCONF_DOTDOT_EMULATION            0


/*  Reliance prefix for a unique name
*/
#define TFS_UNIQUE_NAME     ".tfs"


/*  Reliance CDF path and file name limits

    Reliance will not allow the file names or paths that exceed this value to
    be used on the media.

    Note: Port specific limits may be less than CDF limits
*/
#define REL_CDF_NAME_LENGTH     260     /* maximum filename length */
#define REL_CDF_PATH_LENGTH     260     /* maximum full path length */


/*  Returns non-zero if character is considered white space
*/
#define RelIsWhiteSpace(c) ((c == TfsStr(' ')) || (c == TfsStr('\t')))


/*  Return non-zero if character is considered a path separator
*/
#define TFS_PATH_SEPARATOR              TfsStr('\\')
#define TFS_PATH_SEPARATOR2             TfsStr('/')
#define RELCONF_REDUNDANT_PATH_SEPARATORS    1
#define TfsIsPathSeparator(c) \
        ((c) == TFS_PATH_SEPARATOR || (c) == TFS_PATH_SEPARATOR2)


/*  This setting must always be '1' as support for multiple open
    files is not yet implemented.
*/
#define MAX_OPEN_FILES  1
typedef struct RELFILEINFO
{
#if MAX_OPEN_FILES > 1
    PRELFILEINFO    pFINext;    /* pointer to next one of these in the chain */
#endif
    PRELDISKINST    pDisk;
    D_BOOL         fIsOpen;
    tfsOffset       iCurrentPosition;   /* Current position in the file     */
    unsigned short  usFileState;
    D_UINT16       uPadding;   /* IR4219 aka Bug 150 */
    sTfsNewCoord    sCoord;
} RELFILEINFO;
typedef struct RELDISKINST
{
    sTfsVolume *    pVolume;
    PFNDEVICEREADSECTORS pfnBlockDev;
    void           *pBlockDevInfo;
    RELFILEINFO     fi[MAX_OPEN_FILES];
    D_UINT16       uFilesOpen;
#if MAX_OPEN_FILES > 1
    PRELFILEINFO    pFIFreeList;
    PRELFILEINFO    pFIUsedList;
#endif
    D_UINT32       iSecsPerBlock;
} RELDISKINST;


/*  EBUFTYPE

    Buffer type enumeration
*/
typedef enum {
    BUF_TYPE_INDEX,
    BUF_TYPE_INODE,
    BUF_TYPE_DATA,
    BUF_TYPE_CALLER
} EBUFTYPE;


/*  Signature field for the sTfsNewCoord structure
*/
#define TFS_COORD_SIGNATURE     0x12345678


/*  szTfsCDFIllegalCharacters

    Reliance CDF illegal characters
*/
static tfsChar szTfsCDFIllegalCharacters[] = {
    0
};


/*  sTfsEntryInfo
*/
typedef struct {
    sTfsDirEntry sEntry;
    tfsIndex ulDirINodeIndex;
    tfsOffset ulEntryOffset;
    tfsOffset ulPreviousEntryOffset;
    tfsIndex ulParentDirINodeIndex;
} sTfsInfo;


static RELSTATUS TfsIndexGetEntry(
    sTfsVolume * pVolume,
    tfsIndex ulIndex,
    tfsBlock * pulPhyBlockNum );
static void TfsCoordCreate(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    tfsOffset ulFileOffset,
    sTfsNewCoord * pCoord );
static RELSTATUS TfsCoordFileOffsetToBlock(
    sTfsNewCoord * pCoord );
static RELSTATUS TfsCoordArrayLookup(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    D_UINT16 uArrayEntry,
    tfsIndex * pulValue );
static RELSTATUS TfsCoordGetFileSize(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    tfsOffset * pulFileSize );



/*  RelReadBlocks

    Read the specified number of blocks from the block device.
*/
static RELSTATUS RelReadBlocks(
    sTfsVolume * pVolume,
    EBUFTYPE eBufferType,
    tfsBlock ulBlock,
    tfsBlock ulCount,
    D_BUFFER ** ppBuffer )
{
    tfsBlock * pulLastReadBlock;
    RELSTATUS iTfsError = TFSERR_NONE;
    PRELDISKINST pDisk;
    unsigned long ulStartSector;
    unsigned long ulNumSectors;


    /*  Find a buffer to use
    */
    switch( eBufferType )
    {
        /*  Use buffer designated for index file operations
        */
        case BUF_TYPE_INDEX:
            *ppBuffer = pVolume->asMiniBuf[0].pBuffer;
            pulLastReadBlock = &pVolume->asMiniBuf[0].ulLastRead;
            break;

        /*  Use buffer designated for meta data operations
        */
        case BUF_TYPE_INODE:
            *ppBuffer = pVolume->asMiniBuf[1].pBuffer;
            pulLastReadBlock = &pVolume->asMiniBuf[1].ulLastRead;
            break;

        /*  Use buffer designated for intermediate file operations
        */
        case BUF_TYPE_DATA:
            *ppBuffer = pVolume->asMiniBuf[2].pBuffer;
            pulLastReadBlock = &pVolume->asMiniBuf[2].ulLastRead;
            break;

        /*  Use buffer designated by the caller
        */
        case BUF_TYPE_CALLER:
            /*  Buffer (*ppBuffer) already set to callers buffer
            */
            pulLastReadBlock = (void *)0;
            break;

        default:
            return TFSERR_INTERNAL;
    }


    /*  Read this block if not already in the buffer
    */
    if( (!pulLastReadBlock) || (*pulLastReadBlock != ulBlock) )
    {
        /*  Read the data
        */
        pDisk = pVolume->pDisk;
        ulStartSector = ulBlock * pDisk->iSecsPerBlock;
        ulNumSectors = ulCount * pDisk->iSecsPerBlock;
        if((*pDisk->pfnBlockDev) (pDisk->pBlockDevInfo, ulStartSector,
                ulNumSectors, *ppBuffer) == DCLSTAT_SUCCESS)
        {
            /*  Save the block number that was read
            */
            if( pulLastReadBlock )
            {
                *pulLastReadBlock = ulBlock;
            }
        }
        else
        {
            iTfsError = TFSERR_IO;
        }
    }


    return iTfsError;
}


/*  TfsCoordCreate

    Initialize a coord structure
*/
static void TfsCoordCreate(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    tfsOffset ulFileOffset,
    sTfsNewCoord * pCoord )
{
    DclMemSet( pCoord, 0, sizeof(*pCoord) );
    pCoord->pVolume = pVolume;
    pCoord->ulINodeIndex = ulINodeIndex;
    pCoord->ulFileOffset = ulFileOffset;
}


/*  TfsCoordArrayLookup
*/
static RELSTATUS TfsCoordArrayLookup(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    D_UINT16 uArrayEntry,
    tfsIndex * pulValue )
{
    tfsBlock ulINodeBlockNumber;
    tfsIndex * pulArray;
    D_BUFFER * pucCDFBuffer;
    RELSTATUS iTfsError;


    RelEnter("TfsCoordArrayLookup");


    /*  Get the block number for this INode
    */
    iTfsError = TfsIndexGetEntry( pVolume, ulINodeIndex, &ulINodeBlockNumber );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Get the address of this block in the buffers
    */
    iTfsError = RelReadBlocks( pVolume, BUF_TYPE_INODE,
            ulINodeBlockNumber, 1, &pucCDFBuffer );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Get the address of the array
    */
    pulArray = DclPtrAddByte( pucCDFBuffer, TFS_SIZEOF_sTfsINode );


    /*  Retrieve the entry from the array
    */
    DCLLE2NE(pulValue, &pulArray[uArrayEntry], sizeof(*pulValue));

Done:

    RelLeave();
    return iTfsError;
}


/*  TfsIndexGetEntry

    Returns the block # associated with this index #.
    Returns 0 if failed or invalid index #.
*/
static RELSTATUS TfsIndexGetEntry(
    sTfsVolume * pVolume,
    tfsIndex ulIndex,
    tfsBlock * pulPhyBlockNum )
{
    sTfsNewCoord * pCoord;
    D_BUFFER * pucCDFBuffer;
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsIndexGetEntry");


    pCoord = &pVolume->sIndexFileCoord;
    *pulPhyBlockNum = 0;


    /*  special case: index file INode block number is retrieved
        from MetaRoot
    */
    if( ulIndex == TFS_INDEX_INDEX )
    {
        *pulPhyBlockNum = pVolume->ulIndexBlock;
        goto Done;
    }


    /*  Create a coord structure
    */
    pCoord->ulFileOffset = ulIndex * sizeof(tfsBlock);


    /*  Get the block number associated with this offset
    */
    iTfsError = TfsCoordFileOffsetToBlock( pCoord );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Get the block that contains this index entry
    */
    iTfsError = RelReadBlocks( pVolume, BUF_TYPE_INDEX,
            pCoord->ulBlockNumber, 1, &pucCDFBuffer );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Read the index entry
    */
    DCLLE2NE( pulPhyBlockNum,
            DclPtrAddByte(pucCDFBuffer, pCoord->ulOffsetIntoBuffer),
            sizeof(*pulPhyBlockNum) );

Done:

    RelLeave();
    return iTfsError;
}


/*  TfsINodeCDFRead

    Fills in the structure pointed to by pINode with the data from the common
    data format buffer pointed to by pINode->pucCDFBuffer.
*/
static RELSTATUS TfsINodeCDFRead(
    sTfsINode * pINode)
{
    D_UINT8 * pucCDFBuffer;
    D_UINT32 ulIndex;
#if !TFS_USING_64BIT_FILESIZES
    D_UINT64 ullFileSize;
#endif
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsINodeCDFRead");


    DclAssert(pINode->pucCDFBuffer);
    pucCDFBuffer = pINode->pucCDFBuffer;
    COPYFROMCDF(pucCDFBuffer, &pINode->ulSignature);
    if( pINode->ulSignature != TFS_INODE_SIG )
    {
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }
    COPYFROMCDF(pucCDFBuffer, &pINode->ulIndex);
#if TFS_USING_64BIT_FILESIZES
    COPYFROMCDF64(pucCDFBuffer, &pINode->ullFileSize);
#else
    COPYFROMCDF64(pucCDFBuffer, &ullFileSize);
    pINode->ullFileSize = DclUint32CastUint64(&ullFileSize);
#endif
    COPYFROMCDF64(pucCDFBuffer, &pINode->ullCreateDate);
    COPYFROMCDF64(pucCDFBuffer, &pINode->ullModifiedDate);
    COPYFROMCDF64(pucCDFBuffer, &pINode->ullAccessDate);
    COPYFROMCDF(pucCDFBuffer, &pINode->uAttributes);
    COPYFROMCDF(pucCDFBuffer, &pINode->uLinkCount);
    for( ulIndex = 0; ulIndex < TFS_INODE_RESERVED; ulIndex++ )
    {
        COPYFROMCDF(pucCDFBuffer, &pINode->ulReserved[ulIndex]);
    }
    DclAssert((pucCDFBuffer - pINode->pucCDFBuffer) ==
            TFS_SIZEOF_sTfsINode );
    pINode->pulEntries = (tfsBlock *)pucCDFBuffer;

Done:

    RelLeave();
    return iTfsError;
}


/*  TfsINodeGetINode

    Returns a pointer to the iNode loaded into a cache buffer. Returns 0 if
    not a valid index entry.
*/
static RELSTATUS TfsINodeGetINode(
    sTfsVolume * pVolume,
    tfsIndex ulIndex,
    sTfsINode * psINode )
{
    tfsBlock ulBlock;
    RELSTATUS iTfsError;


    RelEnter("TfsINodeGetINode");


    /*  convert index to block number
    */
    iTfsError = TfsIndexGetEntry( pVolume, ulIndex, &ulBlock );
    if( iTfsError )
    {
        goto Done;
    }


    /*  get pointer to the iNode
    */
    iTfsError = RelReadBlocks( pVolume, BUF_TYPE_INODE, ulBlock, 1,
            &psINode->pucCDFBuffer );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Read the INode structure
    */
    iTfsError = TfsINodeCDFRead( psINode );

Done:

    RelLeave();
    return iTfsError;
}


    /*  TfsCoordGetFileSize

    Get the file size for an INode
*/
static RELSTATUS TfsCoordGetFileSize(
    sTfsVolume * pVolume,
    tfsIndex ulINodeIndex,
    tfsOffset * pulFileSize )
{
    sTfsINode sINode;
    RELSTATUS iTfsError;


    RelEnter("TfsCoordGetFileSize");


    /*  Get the INode
    */
    iTfsError = TfsINodeGetINode( pVolume, ulINodeIndex, &sINode );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Save the file size
    */
    *pulFileSize = sINode.ullFileSize;

Done:

    RelLeave();
    return iTfsError;
}


/*  TfsCoordFileOffsetToBlock

    Get the physical block and offset into that block for a file offset in
    the coord structure.
*/
static RELSTATUS TfsCoordFileOffsetToBlock(
    sTfsNewCoord * pCoord )
{
    tfsIndex ulINodeIndex;
    tfsBlock ulINodeBlockNumber;
    tfsOffset ulTempOffset;
    D_UINT16 uDoubleArrayEntry;
    D_UINT16 uIndirectArrayEntry;
    D_UINT16 uDirectArrayEntry;
    D_UINT16 uOffsetIntoBlock = 0;
    D_BOOL fPerformLookup;
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsCoordFileOffsetToBlock");


    /*  validate parameters
    */
    DclAssert(pCoord);
    if( !pCoord )
    {
        iTfsError = TFSERR_TOO_FEW_ARGS;
        goto Done;
    }


    /*  Determine if the coord structure is initialized
    */
    fPerformLookup = (pCoord->ulSignature != TFS_COORD_SIGNATURE);


    /*  Validate the INode (inherent in calls below) and retrieve the INode
        file size.
    */
    if( fPerformLookup )
    {
        /*  Get INode file size
        */
        iTfsError = TfsCoordGetFileSize( pCoord->pVolume, pCoord->ulINodeIndex,
                &pCoord->ulFileSize );
        if( iTfsError )
        {
            goto Done;
        }
    }


    /*  If the offset has exceeded the file size, ...
    */
    if( pCoord->ulFileOffset >= pCoord->ulFileSize )
    {
        iTfsError = TFSERR_EOF;
        goto Done;
    }


    /*  calculate coordinates
    */
    ulTempOffset = pCoord->ulFileOffset;
    uDoubleArrayEntry = (D_UINT16)(ulTempOffset /
            pCoord->pVolume->ulIndirectSize );
    ulTempOffset %= pCoord->pVolume->ulIndirectSize;
    uIndirectArrayEntry = (D_UINT16)(ulTempOffset /
            pCoord->pVolume->ulDirectSize);
    ulTempOffset %= pCoord->pVolume->ulDirectSize;
    uDirectArrayEntry = (D_UINT16)(ulTempOffset /
            pCoord->pVolume->ulBlockSize);
    ulTempOffset %= pCoord->pVolume->ulBlockSize;
    uOffsetIntoBlock = (D_UINT16)ulTempOffset;


    /*  Start with the index for the INode
    */
    ulINodeIndex = pCoord->ulINodeIndex;


    /*  Indicate that the coord structure is not initialized in case the code
        below encounters an error and exits.
    */
    pCoord->ulSignature = 0;


    /*  Resolve double indirection
    */
    if( pCoord->ulFileSize > pCoord->pVolume->ulIndirectSize )
    {
        if( fPerformLookup ||
                (pCoord->sDouble.uArrayEntry != uDoubleArrayEntry) )
        {
            /*  Look up this value in the array within the INode
            */
            iTfsError = TfsCoordArrayLookup( pCoord->pVolume, ulINodeIndex,
                    uDoubleArrayEntry, &pCoord->sDouble.ulValue );
            if( iTfsError )
            {
                goto Done;
            }


            /*  Save the new array entry number
            */
            pCoord->sDouble.uArrayEntry = uDoubleArrayEntry;


            /*  Assume the rest of the information is stale and look it up
                again
            */
            fPerformLookup = TRUE;
        }
        ulINodeIndex = pCoord->sDouble.ulValue;
    }


    /*  Resolve indirection
    */
    if( pCoord->ulFileSize > pCoord->pVolume->ulDirectSize )
    {
        if( fPerformLookup ||
                (pCoord->sIndirect.uArrayEntry != uIndirectArrayEntry) )
        {
            /*  Look up this value in the array within the INode
            */
            iTfsError = TfsCoordArrayLookup( pCoord->pVolume, ulINodeIndex,
                    uIndirectArrayEntry, &pCoord->sIndirect.ulValue );
            if( iTfsError )
            {
                goto Done;
            }


            /*  Save the new array entry number
            */
            pCoord->sIndirect.uArrayEntry = uIndirectArrayEntry;


            /*  Assume the rest of the information is stale and look it up
                again
            */
            fPerformLookup = TRUE;
        }
        ulINodeIndex = pCoord->sIndirect.ulValue;
    }


    /*  Resolve direct
    */
    if( pCoord->ulFileSize > pCoord->pVolume->ulInPlaceSize )
    {
        if( fPerformLookup ||
                (pCoord->sDirect.uArrayEntry != uDirectArrayEntry) )
        {
            /*  Look up this value in the array within the INode
            */
            iTfsError = TfsCoordArrayLookup( pCoord->pVolume, ulINodeIndex,
                    uDirectArrayEntry, &pCoord->sDirect.ulValue );
            if( iTfsError )
            {
                goto Done;
            }


            /*  Save the new array entry number
            */
            pCoord->sDirect.uArrayEntry = uDirectArrayEntry;
        }


        /*  Set the offset and offset within the block
        */
        pCoord->ulBlockNumber = pCoord->sDirect.ulValue;
        pCoord->ulOffsetIntoBuffer = uOffsetIntoBlock;
    }
    else
    {
        /*  Get the block number for this INode
        */
        iTfsError = TfsIndexGetEntry( pCoord->pVolume, pCoord->ulINodeIndex,
                &ulINodeBlockNumber );
        if( iTfsError )
        {
            goto Done;
        }


        /*  Set the offset and offset within the block
        */
        pCoord->ulBlockNumber = ulINodeBlockNumber;
        pCoord->ulOffsetIntoBuffer = pCoord->ulFileOffset +
                TFS_SIZEOF_sTfsINode;
    }


    /*  Indicate that the coord structure is now valid
    */
    pCoord->ulSignature = TFS_COORD_SIGNATURE;

Done:


    /*  Limit checking on the offset into a buffer
    */
    DclAssert( uOffsetIntoBlock < pCoord->pVolume->ulBlockSize );
    if( uOffsetIntoBlock >= pCoord->pVolume->ulBlockSize )
    {
        iTfsError = TFSERR_INTERNAL;
        goto Done;
    }


    RelLeave();
    return iTfsError;
}


/*  RelINodeRead

    Read a number of bytes from an INode.
*/
static RELSTATUS TfsINodeRead(
    sTfsVolume * pVolume,
    sTfsNewCoord * pCoord,
    tfsOffset ulOffset,
    tfsOffset * pulLength,
    D_BUFFER * pData )
{
    tfsOffset ulThisLength;
    tfsBlock ulMaxReadBlocks;
    tfsBlock ulReadBlocks;
    tfsBlock ulStartBlock;
    tfsOffset ulLength;
    tfsOffset ulBytesRead = 0;
    D_BUFFER * pInterimBuffer;
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("RelINodeRead");


    /*  Create a local copy of the length argument
    */
    ulLength = *pulLength;


    /*  Set the file offset
    */
    pCoord->ulFileOffset = ulOffset;


    /*  Reduce the length of the read if necessary
    */
    iTfsError = TfsCoordFileOffsetToBlock( pCoord );
    if( iTfsError )
    {
        /*  Should not generate an end of file error on a zero byte read
            regardless of the position of the file pointer
        */
        if((iTfsError == TFSERR_EOF) && (*pulLength == 0))
        {
            iTfsError = TFSERR_NONE;
        }
        goto Done;
    }
    if( (pCoord->ulFileSize - pCoord->ulFileOffset) < ulLength )
    {
        ulLength = pCoord->ulFileSize - pCoord->ulFileOffset;
        if( !ulLength )
        {
            iTfsError = TFSERR_EOF;
            goto Done;
        }
    }


    /*  Read the requested number of bytes
    */
    while( ulLength )
    {
        /*  Update the coord structure
        */
        iTfsError = TfsCoordFileOffsetToBlock( pCoord );
        if( iTfsError )
        {
            /*  Propagate the error
            */
            goto Done;
        }


        /*  Determine if this block needs to be read into the buffers
        */
        if( (pCoord->ulOffsetIntoBuffer) || (ulLength < pVolume->ulBlockSize) )
        {
            /*  Perform a buffered read
            */
            iTfsError = RelReadBlocks( pVolume, BUF_TYPE_DATA,
                    pCoord->ulBlockNumber, 1, &pInterimBuffer );
            if( iTfsError )
            {
                goto Done;
            }


            /*  Determine the length of this read
            */
            ulThisLength = pVolume->ulBlockSize - pCoord->ulOffsetIntoBuffer;
            if( ulThisLength > ulLength )
            {
                ulThisLength = ulLength;
            }


            /*  Copy the data to the callers buffer and adjust the buffer
                for the data read
            */
            DclMemCpy( pData, DclPtrAddByte(pInterimBuffer,
                    pCoord->ulOffsetIntoBuffer), ulThisLength );
            pData = DclPtrAddByte( pData, ulThisLength );


            /*  Adjust the offset in the coord structure
            */
            pCoord->ulFileOffset += ulThisLength;
        }
        else
        {
            /*  Determine the number of contiguous blocks
            */
            ulStartBlock = pCoord->ulBlockNumber;
            pCoord->ulFileOffset += pVolume->ulBlockSize;
            ulMaxReadBlocks = ulLength / pVolume->ulBlockSize;
            for( ulReadBlocks = 1; ulReadBlocks < ulMaxReadBlocks;
                    ulReadBlocks++ )
            {
                /*  Adjust the offset of the coord structure to account for
                    another block of data.
                */
                iTfsError = TfsCoordFileOffsetToBlock( pCoord );
                if( iTfsError )
                {
                    goto Done;
                }


                /*  Update the coord structure and determine if this block is
                    contiguous with the last block.
                */
                if( (ulStartBlock + ulReadBlocks) != pCoord->ulBlockNumber )
                {
                    /*  Not contiguous past this point
                    */
                    break;
                }
                pCoord->ulFileOffset += pVolume->ulBlockSize;
            }


            /*  Read directly into the callers buffer
            */
            iTfsError = RelReadBlocks( pVolume, BUF_TYPE_CALLER,
                    ulStartBlock, ulReadBlocks, &pData );
            if( iTfsError )
            {
                goto Done;
            }


            /*  Adjust the buffer for the blocks read

                Note: Coord offset adjusted above while determining
                    contiguous blocks.
            */
            ulThisLength = ulReadBlocks * pVolume->ulBlockSize;
            pData = DclPtrAddByte( pData, ulThisLength );
        }


        /*  Adjust the length for the data read
        */
        ulLength -= ulThisLength;
        ulBytesRead += ulThisLength;
    }

Done:

    *pulLength = ulBytesRead;
    RelLeave();
    return iTfsError;
}


/*  TfsNameNextName

    Given a path, return the left most name in the path and update the
    pointer to the next part of the path.
*/
static RELSTATUS TfsNameNextName(
    const tfsChar ** pszPath,
    tfsChar * szName,
    D_UINT16 * puLimitedChecksum,
    D_UINT16 * puUnicodeNameLength,
    D_BOOL fFirstCall,
    D_UINT32 ulPortNameLengthLimit,
    tfsChar * szPortIllegalCharacters )
{
    const tfsChar * szPath;
    tfsChar * szOrgName;
    tfsChar * szDotOrSpaceInName = (void *)0;
    D_BOOL fCopiedDotOrSpace = FALSE;
    D_UINT16 uNameLengthWithoutDot = 0;
    D_UINT16 uChecksumWithoutDot = 0;
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsNameNextName");


    /*  Variable initialization
    */
    *puUnicodeNameLength = 0;
    *puLimitedChecksum = 0;


    /*
    */
    szPath = *pszPath;
    szOrgName = szName;


    /*  Special case for root directory
    */
    if( fFirstCall )
    {
        /*  Skip over the path separator
        */
        while( TfsIsPathSeparator(*szPath) )
        {
            szPath++;
#if !RELCONF_REDUNDANT_PATH_SEPARATORS
            break;
#endif
        }
    }


    /*  Skip leading whitespace
    */
    while( RelIsWhiteSpace(*szPath) )
    {
        szPath++;
    }


    /*  Copy the next part of the path up to the path separator
    */
    while( *szPath )
    {
        int iTemp;
        
        /*
        */
        fFirstCall = FALSE;


        /*  Check if the name length is within port limits
        */
        if( *puUnicodeNameLength > ulPortNameLengthLimit )
        {
            iTfsError = TFSERR_BADPATH;
            goto Done;
        }


        /*  If this is a path separator, ...
        */
        if( TfsIsPathSeparator(*szPath) )
        {
            break;
        }


        /*  Check for port specific illegal characters
        */
        if( DclStrChr( szPortIllegalCharacters, *szPath) != (void *)0 )
        {
            iTfsError = TFSERR_BADPATH;
            goto Done;
        }


        /*  Check for Reliance CDF illegal characters
        */
        if( DclStrChr( szTfsCDFIllegalCharacters, *szPath) != (void *)0 )
        {
            iTfsError = TFSERR_BADPATH;
            goto Done;
        }


        /*  Helper for stripping of a DOT at the end of a name.
        */
        if( (*szPath == TfsStr('.')) || RelIsWhiteSpace(*szPath) )
        {
            if( !fCopiedDotOrSpace )
            {
                fCopiedDotOrSpace = TRUE;
                szDotOrSpaceInName = szName;
                uNameLengthWithoutDot = *puUnicodeNameLength;
                uChecksumWithoutDot = *puLimitedChecksum;
            }
        }
        else
        {
            fCopiedDotOrSpace = FALSE;
        }


        /*  Copy this character of the name and
        */
        *szName = *szPath;


        /*  increment the character count
        */
        (*puUnicodeNameLength)++;


        /*  If this value meets the conditions for inclusion in the
            Limited Checksum, do that now.
        */
        if( *puLimitedChecksum & 0x8000 )
        {
            *puLimitedChecksum <<= 1;
            (*puLimitedChecksum)++;
        }
        else
        {
            *puLimitedChecksum <<= 1;
        }
        iTemp = DclToUpper(*szName);
        if( ((iTemp >= TfsStr('A')) && (iTemp <= TfsStr('Z'))) ||
             ((*szName >= TfsStr('0')) && (*szName <= TfsStr('9'))) )
        {
            /*  Include this legal character in the limited checksum
                (Overflow is acceptable).
            */
            *puLimitedChecksum += (D_UINT16)iTemp;
        }


        /*  Increment the pointers for the source and destination
        */
        szName++;
        szPath++;
    }


    /*  Terminate the name
    */
    if( fCopiedDotOrSpace )
    {
        *szDotOrSpaceInName = 0;
        *puUnicodeNameLength = uNameLengthWithoutDot;
        *puLimitedChecksum = uChecksumWithoutDot;
    }
    else
    {
        *szName = 0;
    }


    if( (fFirstCall) && (!(*szOrgName)) && (!(*szPath)) )
    {
        /*  Root directory is ok.  Do nothing.
        */
    }
    else
    {
        /*  If there is no name return an error
        */
        if( (!(*szOrgName)) || RelIsWhiteSpace(*szOrgName) )
        {
            iTfsError = TFSERR_BADPATH;
            goto Done;
        }
    }


    /*  Skip over the path separator
    */
    while( TfsIsPathSeparator(*szPath) )
    {
#if RELCONF_REDUNDANT_PATH_SEPARATORS
        szPath++;
#else
        szPath++;
        break;
#endif
    }


    /*  Convert the name length to Unicode name length
    */
    (*puUnicodeNameLength) <<= 1;


    /*  Save the start of the next name in the path
    */
    *pszPath = szPath;


Done:

    RelLeave();
    return iTfsError;
}


/*  TfsNameGetTfsNameIndex

    If this is a ".tfs" name, get the index number from the name.
*/
static RELSTATUS TfsNameGetTfsNameIndex(
    const tfsChar * szName,
    tfsIndex * pulTargetIndex )
{
    static tfsChar szUniqueName[] = TfsStr(TFS_UNIQUE_NAME);
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsNameGetTfsNameIndex");


    /*  If this is a ".tfs" name, ...
    */
    if( DclStrNCmp( szName, szUniqueName,
            DCLDIMENSIONOF(szUniqueName) - 1) == 0 )
    {
        /*  Retrieve the index number from the name
        */
        *pulTargetIndex = DclAtoL( &szName[DCLDIMENSIONOF(szUniqueName)] );
        if( *pulTargetIndex == 0 )
        {
            iTfsError = TFSERR_BADPATH;
        }
    }
    else
    {
        *pulTargetIndex = 0;
    }


    RelLeave();
    return iTfsError;
}


/*  TfsNameCompare

    Returns TRUE if the names are the same.
*/
static D_BOOL TfsNameCompare(
    const tfsChar * szName,
    const tfsChar * szEntryName )
{
    D_BOOL fResult = TRUE;


    RelEnter("TfsNameCompare");


    /*  Compare until the end of one of the strings are encountered
    */
    while( (*szName) && (*szEntryName) )
    {
        /*  Compare the characters in the string ignoring case
        */
        if( DclToUpper(*szName) != DclToUpper(*szEntryName) )
        {
            /*  Strings are different
            */
            fResult = FALSE;
            goto Done;
        }


        /*  On to the next character in the string
        */
        szName++;
        szEntryName++;
    }


    /*  If either of the strings are not at the NULL terminator, ...
    */
    if( *szName || *szEntryName )
    {
        /*  Strings are different
        */
        fResult = FALSE;
    }

Done:

    RelLeave();
    return fResult;
}


/*  TfsNameMakeUniqueName

    Create a unique name based off the INode index while preserving the
    extension of the Unicode name if possible.
*/
static void TfsNameMakeUniqueName(
    const D_UINT16 * puUnicodeName,
    D_UINT16 uNameLength,
    tfsIndex ulINodeIndex,
    tfsChar * szName,
    tfsChar * szPortIllegalCharacters )
{
    const D_UINT16 * puExtension = (void *)0;
    D_UINT16 uIndex;
    tfsChar ucTemp;


    RelEnter("TfsNameMakeUniqueName");


    /*  Create a unique name using the INode index
    */
    DclSNPrintf( szName, REL_CDF_NAME_LENGTH, TfsStr("%s%04lu"),
            TfsStr(TFS_UNIQUE_NAME), ulINodeIndex );


    /*  Search for an extension
    */
    for( uIndex = 0; uIndex < uNameLength; uIndex++ )
    {
        if( puUnicodeName[uIndex] == TfsStr('.') )
        {
            puExtension = &puUnicodeName[uIndex];
        }
    }


    /*  If an extension was found, attempt to append it to the name
    */
    if( puExtension )
    {
        /*  Find the end of the unique string
        */
        szName += DclStrLen( szName );


        /*  Adjust the number of Unicode characters remaining after the
            extension.
        */
        uNameLength -= (D_UINT16)(puExtension - puUnicodeName);


        /*  Copy the extension if possible
        */
        for( uIndex = 0; uIndex < uNameLength; uIndex++ )
        {
            /*  Copy the extension while watching for invalid characters
            */
            if( puExtension[uIndex] > 0x7f )
            {
                /*  Terminate the native string without an extension
                */
                szName[0] = 0;
                break;
            }
            ucTemp = (tfsChar)(puExtension[uIndex]);
            if( DclStrChr( szPortIllegalCharacters, ucTemp) != 0 )
            {
                /*  Terminate the native string without an extension
                */
                szName[0] = 0;
                break;
            }
            szName[uIndex] = ucTemp;
        }
        szName[uIndex] = 0;
    }


    RelLeave();
}


/*  TfsNameUnicodeToNative

    Convert the Unicode name to native a native
*/
static void TfsNameUnicodeToNative(
    const D_UINT16 * puUnicodeName,
    D_UINT16 uNameLength,
    tfsIndex ulINodeIndex,
    tfsChar * szName,
    tfsChar * szPortIllegalCharacters )
{
    tfsChar * pucOrgName;
    const D_UINT16 * puOrgUnicodeName;
    D_UINT16 uOrgNameLength;


    /*  Save the start of the name locations
    */
    pucOrgName = szName;
    puOrgUnicodeName = puUnicodeName;
    uNameLength >>= 1;
    uOrgNameLength = uNameLength;


    RelEnter("TfsNameUnicodeToNative");


    /*  Convert the the Unicode characters to ASCII until a charater is
        reached that cannot be converted.
    */
    while( uNameLength )
    {
#if !TFS_UNICODE
        if( (*puUnicodeName > 0x7f) )
        {
            TfsNameMakeUniqueName( puOrgUnicodeName, uOrgNameLength,
                    ulINodeIndex, pucOrgName, szPortIllegalCharacters );
            goto Done;
        }
#endif
        if( DclStrChr(szPortIllegalCharacters,
                (tfsChar)(*puUnicodeName)) != 0 )
        {
            TfsNameMakeUniqueName( puOrgUnicodeName, uOrgNameLength,
                    ulINodeIndex, pucOrgName, szPortIllegalCharacters );
            goto Done;
        }
        *szName = (tfsChar)*puUnicodeName;
        szName++;
        puUnicodeName++;
        uNameLength--;
    }
    *szName = 0;

Done:

    RelLeave();
}


/*  TfsDirReadName
*/
static RELSTATUS TfsDirReadName(
    sTfsNewCoord * pCoord,
    D_UINT16 uNameLength,
    tfsIndex ulEntryINodeIndex,
    tfsChar * szName )
{
    D_UINT16 puUnicodeNameSpace[REL_CDF_NAME_LENGTH];
    D_UINT32 ulReadSize;
    D_BUFFER * pucCDFBuffer = (void *)0;
    RELSTATUS iTfsError = TFSERR_NONE;
    D_UINT16 * szUnicodeName;
    D_UINT16 uLoopNameLength;
    tfsOffset ulOrgFileOffset;
    D_BUFFER * pucCDFBufferNamePart;
    D_UINT16 uIndex;
    D_UINT8 ucPacketNumber;


    RelEnter("TfsDirReadName");


    /*  Save the starting file offset
    */
    ulOrgFileOffset = pCoord->ulFileOffset;


    /*  Create a local pointer to the Unicode name space
    */
    szUnicodeName = puUnicodeNameSpace;


    /*  Determine the number of fixed sized entry packets that were used
        store the name.
    */
    ucPacketNumber = (D_UINT8)((uNameLength +
            (TFS_SIZEOF_DIR_PACKET - 3)) / (TFS_SIZEOF_DIR_PACKET - 2));


    /*  Read or write the entry
    */
    uLoopNameLength = uNameLength;
    while( uLoopNameLength )
    {
        /*  Adjust the entry offset to the name
        */
        pCoord->ulFileOffset += TFS_SIZEOF_DIR_PACKET;


        /*  Recalculate the physical block number for this offset
        */
        iTfsError = TfsCoordFileOffsetToBlock( pCoord );
        if( iTfsError )
        {
            goto Done;
        }


        /*  Determine the number of bytes for the name that can be written in
            the remainder of this buffer
        */
        ulReadSize = TFS_SIZEOF_DIR_PACKET - 2;
        if( ulReadSize > uLoopNameLength  )
        {
            ulReadSize = uLoopNameLength;
        }


        /*  Get the block from the buffers
        */
        iTfsError = RelReadBlocks( pCoord->pVolume, BUF_TYPE_DATA,
                pCoord->ulBlockNumber, 1, &pucCDFBuffer );
        if( iTfsError )
        {
            goto Done;
        }


        /*  Validate the packet number
        */
        pucCDFBufferNamePart = DclPtrAddByte(pucCDFBuffer,
                pCoord->ulOffsetIntoBuffer);
        if( *pucCDFBufferNamePart != ucPacketNumber )
        {
            iTfsError = TFSERR_INTERNAL;
            goto Done;
        }
        pucCDFBufferNamePart += sizeof(ucPacketNumber);
        pucCDFBufferNamePart += sizeof(ucPacketNumber);


        /*  Read the name
        */
        for( uIndex = 0; uIndex < ulReadSize >> 1; uIndex++ )
        {
            DCLLE2NE(&szUnicodeName[uIndex], pucCDFBufferNamePart, sizeof(*szUnicodeName));
            pucCDFBufferNamePart += sizeof(*szUnicodeName);
        }


        /*  Adjust the name space
        */
        szUnicodeName = DclPtrAddByte(szUnicodeName, ulReadSize);


        /*  Adjust the remaining number of bytes
        */
        uLoopNameLength -= (D_UINT16)ulReadSize;


        /*  Adjust the number of packets remaining
        */
        ucPacketNumber--;
    }


    /*  Convert the Unicode name to native name
    */
    TfsNameUnicodeToNative( puUnicodeNameSpace, uNameLength,
            ulEntryINodeIndex, szName,
            pCoord->pVolume->szPortIllegalCharacters );

Done:

    /*  Restore the file offset
    */
    pCoord->ulFileOffset = ulOrgFileOffset;


    RelLeave();
    return iTfsError;
}


/*  TfsDirCDFReadEntry

    Read a common data format directory entry into the in-memory directory
    entry.
*/
static void TfsDirCDFReadEntry(
    D_BUFFER * pucCDFBuffer,
    tfsOffset ulOffset,
    sTfsDirEntry * psDirEntry)
{
  #if D_DEBUG
    D_BUFFER * pucOrgCDFBuffer;
  #endif


    RelEnter("TfsDirCDFReadEntry");


    pucCDFBuffer = DclPtrAddByte( pucCDFBuffer, ulOffset );
  #if D_DEBUG
    pucOrgCDFBuffer = pucCDFBuffer;
  #endif
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->ucPacketFlags);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->uLC);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->ulEntryLength);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->uUnicodeNameLength);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->ucEntryAttributes);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->ucReserved);
    COPYFROMCDF(pucCDFBuffer, &psDirEntry->ulINodeIndex);

    DclAssert((pucCDFBuffer - pucOrgCDFBuffer) == TFS_SIZEOF_sTfsDirEntry);


    RelLeave();
}


/*  TfsDirInfoEntry
*/
static void TfsDirInfoEntry(
    D_BUFFER * pucCDFBuffer,
    sTfsNewCoord * pCoord,
    tfsOffset ulPreviousEntryOffset,
    sTfsInfo * pInfo )
{
    /*  Read the entry header
    */
    TfsDirCDFReadEntry( pucCDFBuffer, pCoord->ulOffsetIntoBuffer,
            &pInfo->sEntry );


    /*  Update the remaining fields in the info structure
    */
    pInfo->ulDirINodeIndex = pCoord->ulINodeIndex;
    pInfo->ulEntryOffset = pCoord->ulFileOffset;
    pInfo->ulPreviousEntryOffset = ulPreviousEntryOffset;
}


/*  TfsDirCDFReadEntryAttrAndLength
*/
static void TfsDirCDFReadEntryAttrAndLength(
    D_BUFFER * pucCDFBuffer,
    tfsOffset ulOffset,
    D_UINT8 * pucEntryAttributes,
    D_UINT32 * pulEntryLength )
{
    RelEnter("TfsDirCDFReadEntryAttrAndLength");

    pucCDFBuffer = DclPtrAddByte( pucCDFBuffer,
            ulOffset +
            DCLSIZEOFMEMBER(sTfsDirEntry, ucPacketFlags) +
            DCLSIZEOFMEMBER(sTfsDirEntry, uLC));

    DclAssert(DCLSIZEOFMEMBER(sTfsDirEntry, ulEntryLength) == sizeof(*pulEntryLength));

    COPYFROMCDF(pucCDFBuffer, pulEntryLength);

    pucCDFBuffer = DclPtrAddByte(pucCDFBuffer, DCLSIZEOFMEMBER(sTfsDirEntry, uUnicodeNameLength));

    DclAssert(DCLSIZEOFMEMBER(sTfsDirEntry, ucEntryAttributes) == sizeof(*pucEntryAttributes));

    COPYFROMCDF(pucCDFBuffer, pucEntryAttributes);

    RelLeave();
}


/*  TfsDirCDFReadEntryINodeIndex
*/
static void TfsDirCDFReadEntryINodeIndex(
    D_BUFFER  *pucCDFBuffer,
    tfsOffset   ulOffset,
    tfsIndex   *pulINodeIndex )
{
    RelEnter("TfsDirCDFReadEntryINodeIndex");

    pucCDFBuffer = DclPtrAddByte( pucCDFBuffer,
            ulOffset +
            DCLSIZEOFMEMBER(sTfsDirEntry, ucPacketFlags) +
            DCLSIZEOFMEMBER(sTfsDirEntry, uLC) +
            DCLSIZEOFMEMBER(sTfsDirEntry, ulEntryLength) +
            DCLSIZEOFMEMBER(sTfsDirEntry, uUnicodeNameLength) +
            DCLSIZEOFMEMBER(sTfsDirEntry, ucEntryAttributes) +
            DCLSIZEOFMEMBER(sTfsDirEntry, ucReserved) );

    DclAssert( DCLSIZEOFMEMBER(sTfsDirEntry, ulINodeIndex) == sizeof(*pulINodeIndex) );

    COPYFROMCDF(pucCDFBuffer, pulINodeIndex);

    RelLeave();
}


/*  TfsDirLoop
*/
static RELSTATUS TfsDirLoop(
    sTfsVolume * pVolume,
    const tfsChar * szName,
    D_UINT16 uNameLength,
    D_UINT16 uNameLC,
    D_UINT8 ucSearchAttributes,
    tfsIndex ulParentIndex,
    sTfsInfo * pInfo )
{
    tfsChar szEntryName[REL_CDF_NAME_LENGTH + 1];
    sTfsNewCoord sCoord;
    tfsOffset ulFileSize;
    tfsIndex ulTargetIndex;
    tfsOffset ulPreviousOffset;
    D_BUFFER * pucCDFBuffer = (void *)0;
    tfsBlock ulCurrentBlock = 0;
    D_UINT8 ucEntryAttributes;
    D_UINT32 ulEntryLength;
    tfsIndex ulINodeIndex;
    RELSTATUS iTfsError = TFSERR_NONE;

    (void)uNameLength;

    RelEnter("TfsDirLoop");


    /*  Initialize the callers INFO structure
    */
    DclMemSet( pInfo, 0, sizeof(*pInfo) );


    /*  Recalculate the physical block and block offset for the
        file offset
    */
    TfsCoordCreate( pVolume, ulParentIndex, 0, &sCoord );
    iTfsError = TfsCoordFileOffsetToBlock( &sCoord );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Get the file size
    */
    ulFileSize = sCoord.ulFileSize;


    /*  If this is a ".tfs" name, get the index number from the name
    */
    iTfsError = TfsNameGetTfsNameIndex( szName, &ulTargetIndex );
    if( iTfsError )
    {
        goto Done;
    }


    /*  Set the previous entry offset into the directory to be zero
    */
    ulPreviousOffset = 0;


    /*  Search the directory
    */
    while( TRUE )
    {
        /*  If this is the end of the file, ...
        */
        if( sCoord.ulFileOffset >= ulFileSize )
        {
            /*  Release any "keep" buffers
            */
            if( pucCDFBuffer )
            {
                pucCDFBuffer = (void *)0;
            }


            /*  Return that a matching entry was not found
            */
            iTfsError = TFSERR_NOTFOUND;
            goto Done;
        }


        /*  Recalculate the physical block and block offset for the
            file offset
        */
        iTfsError = TfsCoordFileOffsetToBlock( &sCoord );
        if( iTfsError )
        {
            goto Done;
        }


        /*  If this is the first pass through the loop or the block number of
            the buffer has changed, ...
        */
        if( (sCoord.ulFileOffset == 0) ||
                (ulCurrentBlock != sCoord.ulBlockNumber) )
        {
            /*  Get the block from the buffers
            */
            iTfsError = RelReadBlocks( pVolume, BUF_TYPE_DATA,
                    sCoord.ulBlockNumber, 1, &pucCDFBuffer );
            if( iTfsError )
            {
                goto Done;
            }
            ulCurrentBlock = sCoord.ulBlockNumber;
        }


        /*  Read the entry attributes
        */
        TfsDirCDFReadEntryAttrAndLength( pucCDFBuffer,
                sCoord.ulOffsetIntoBuffer, &ucEntryAttributes,
                &ulEntryLength );


        /*  If the entry attributes indicate that this entry is used, ...
        */
        if( ucEntryAttributes & TFS_DIR_INUSE )
        {
            if( ulTargetIndex )
            {
                /*  Read the INode index from the entry
                */
                TfsDirCDFReadEntryINodeIndex( pucCDFBuffer,
                        sCoord.ulOffsetIntoBuffer, &ulINodeIndex );


                /*  If the search INode index matches the one we are
                    looking for, ...
                */
                if( ulTargetIndex == ulINodeIndex )
                {
                    /*  Make sure the attributes are what was expected
                    */
                    if( (ucEntryAttributes & ~TFS_DIR_INUSE) !=
                            ucSearchAttributes )
                    {
                        iTfsError = TFSERR_DENIED;
                        goto Done;
                    }


                    /*  Handle the TFS_PROCESSDIR_INFO request
                    */
                    iTfsError = TFSERR_NONE;


                    /*  Read the entry information
                    */
                    TfsDirInfoEntry( pucCDFBuffer, &sCoord,
                            ulPreviousOffset, pInfo );
                    goto Done;
                }
            }
            else
            {
                /*  Since so many of the entry fields will be referenced,
                    read the entire entry
                */
                TfsDirInfoEntry( pucCDFBuffer, &sCoord,
                        ulPreviousOffset, pInfo );


                /*  If the limited checksum warrents further comparison, ...
                */
                if( (pInfo->sEntry.uLC == uNameLC) || (uNameLC == 0) ||
                        (pInfo->sEntry.uLC == 0) )
                {
                    /*  Read the name
                    */
                    iTfsError = TfsDirReadName(  &sCoord,
                            pInfo->sEntry.uUnicodeNameLength,
                            pInfo->sEntry.ulINodeIndex,
                            szEntryName );
                    if( iTfsError )
                    {
                        goto Done;
                    }


                    /*  If the names are the same, ...
                    */
                    if( TfsNameCompare(szName, szEntryName) )
                    {
                        /*  Make sure the attributes are what was
                            expected
                        */
                        if( (pInfo->sEntry.ucEntryAttributes &
                                ~TFS_DIR_INUSE) != ucSearchAttributes )
                        {
                            iTfsError = TFSERR_DENIED;
                            goto Done;
                        }
                        goto Done;
                    }
                }
            }
        }


        /*  Adjust the offset to point to the next directory entry by adding
            the current entry's length.
        */
        sCoord.ulFileOffset += ulEntryLength;
        DclAssert( sCoord.ulFileOffset <= sCoord.ulFileSize );
        if( sCoord.ulFileOffset > sCoord.ulFileSize )
        {
            iTfsError = TFSERR_INTERNAL;
            goto Done;
        }
    }


Done:


    RelLeave();
    return iTfsError;
}


/*  TfsDirProcess

    This function makes sure the path exists for the desired operation then
    forwards the call on to be handled within the specified directory.
*/
static RELSTATUS TfsDirProcess(
    sTfsVolume * pVolume,
    const tfsChar * szPath,
    sTfsInfo * pInfo )
{
    tfsIndex ulParentINodeIndex;
    RELSTATUS iTfsError;
    tfsChar szName[REL_CDF_NAME_LENGTH + 1];
    D_UINT16 uLimitedChecksum;
    D_UINT16 uUnicodeNameLength;
    D_BOOL fFirstCall = TRUE;
    D_UINT32 ulPathLength = 0;
#if RELCONF_DOTDOT_EMULATION
    tfsIndex ulPreviousParentINodeIndex;
#endif


    RelEnter("TfsDirProcess");


    /*  Set the parent INode index to the root directory
    */
    ulParentINodeIndex = TFS_INDEX_ROOTDIR;


    /*  Parse the path
    */
    while( TRUE )
    {
        /*  Get the next name in the path
        */
        iTfsError = TfsNameNextName( &szPath, szName, &uLimitedChecksum,
                &uUnicodeNameLength, fFirstCall,
                pVolume->ulPortNameLengthLimit,
                pVolume->szPortIllegalCharacters );
        if( iTfsError )
        {
            goto Done;
        }
        fFirstCall = FALSE;


        /*  Make sure that the path has not grown to long

            Note: the "add one" below accounts for path separators
        */
        ulPathLength += 1 + (uUnicodeNameLength >> 1);
        if( ulPathLength > pVolume->ulPortPathLengthLimit )
        {
            iTfsError = TFSERR_BADPATH;
            goto Done;
        }


        /*  If this is the last name in the path
        */
        if( !(*szPath) )
        {
            break;
        }


        /*  Get the information for this directory entry
        */
        iTfsError = TfsDirLoop( pVolume, szName, uUnicodeNameLength,
                uLimitedChecksum, TFS_DIR_ISDIR, ulParentINodeIndex, pInfo );
        if( iTfsError )
        {
            if( iTfsError == TFSERR_NOTFOUND )
            {
                iTfsError = TFSERR_BADPATH;
            }
            goto Done;
        }


        /*  Set the new parent directory
        */
#if RELCONF_DOTDOT_EMULATION
        ulPreviousParentINodeIndex = ulParentINodeIndex;
#endif
        ulParentINodeIndex = pInfo->sEntry.ulINodeIndex;
    }


    /*  If this is the root directory
    */
    if( !(*szName) )
    {
        iTfsError = TFSERR_BADPATH;
        goto Done;
    }


    /*  Perform the callers action to this directory entry
    */
    iTfsError = TfsDirLoop( pVolume, szName, uUnicodeNameLength,
            uLimitedChecksum, 0, ulParentINodeIndex, pInfo );
    if( iTfsError )
    {
        goto Done;
    }


Done:


    RelLeave();
    return iTfsError;
}


/*  RelFindFile

    Get the index number associated with a file
*/
static RELSTATUS RelFindFile(
    sTfsVolume * pVolume,
    const char * szPath,
    tfsIndex * pulINodeIndex )
{
    sTfsInfo sInfo;
    RELSTATUS iTfsError;


    /*  Walk the path and open the specified file using the supplied
        attributes.
    */
    iTfsError = TfsDirProcess( pVolume, szPath, &sInfo );
    if( !iTfsError )
    {
        *pulINodeIndex = sInfo.sEntry.ulINodeIndex;
    }


    return iTfsError;
}


/*  TfsCalculateCheckSum

    Calculate a word checksum for this buffer.
*/
static D_UINT32 TfsCalcCheckSum(
    void *pBuffer,
    D_UINT32 iCheckSum,
    D_UINT32 iBuffSize)
{
    void           *pBuffer32;
    D_UINT32       ulValue;
    D_UINT32       iNumWords;

    /*  validate parameters
    */
    DclAssert(pBuffer);

    /*  calculate the checksum
    */
    iNumWords = iBuffSize / sizeof(D_UINT32);
    pBuffer32 = pBuffer;
    while(iNumWords--)
    {
        COPYFROMCDF(pBuffer32, &ulValue);
        iCheckSum += ulValue;
    }


    /*  return it
    */
    return iCheckSum;
}


/*  TfsMasterIsValid

    Is this MasterBlock valid and the right version?
*/
static RELSTATUS TfsMasterIsValid(
    sTfsMasterBlock * pMaster)
{
    D_UINT32       ulCheckSum;
    RELSTATUS iTfsError = TFSERR_NONE;


    RelEnter("TfsMasterIsValid");


    /*  check signature
    */
    if(pMaster->ulSignature != TFS_MASTER_SIG)
    {
        iTfsError = TFSERR_UNFORMATTED;
        goto Done;
    }


    /*  verify the checksum
    */
    ulCheckSum = TfsCalcCheckSum(pMaster->pucCDFBuffer + TFS_MASTER_OFFSET,
            TFS_MASTER_CHECKSUM, TFS_SIZEOF_sTfsMaster );
    ulCheckSum -= pMaster->ulCheckSum;
    if(pMaster->ulCheckSum != ulCheckSum)
    {
        iTfsError = TFSERR_UNFORMATTED;
        goto Done;
    }


    /*  disk may be the wrong version
    */
    if(pMaster->uTfsLayout != TFS_LAYOUT_VERSION)
    {
        iTfsError = TFSERR_WRONGVERSION;
        goto Done;
    }


Done:

    RelLeave();
    return iTfsError;
}


/*  TfsMasterCDFRead

    Fills in the structure pointed to by pMaster with the data from the
    common data format buffer pointed to by pMaster->pucCDFBuffer.
*/
static void TfsMasterCDFRead(
    sTfsMasterBlock * pMaster)
{
    D_UINT8       *pucCDFBuffer;
    int             i;


    RelEnter("TfsMasterCDFRead");


    /*  skip the first 0x40 bytes, which are boot code and not defined
        for CDF
    */
    DclAssert(pMaster->pucCDFBuffer);
    pucCDFBuffer = pMaster->pucCDFBuffer;
    pucCDFBuffer += TFS_MASTER_OFFSET;


    /*  Copy the fields from the buffer to the in memory structure
    */
    COPYFROMCDF(pucCDFBuffer, &pMaster->ulSignature);
    COPYFROMCDF(pucCDFBuffer, &pMaster->uFmtBldNumber);
    COPYFROMCDF(pucCDFBuffer, &pMaster->uTfsLayout);
    COPYFROMCDF(pucCDFBuffer, &pMaster->ulBlockSize);
    COPYFROMCDF(pucCDFBuffer, &pMaster->ulNumBlocks);
    for(i = 0; i < TFS_NUM_METAROOTS; i++)
    {
        COPYFROMCDF(pucCDFBuffer, &pMaster->ulMetaBlock[i]);
    }
    COPYFROMCDF64(pucCDFBuffer, &pMaster->ullCreateDate);
    COPYFROMCDF(pucCDFBuffer, &pMaster->ulNumIMapBlocks);
    for(i = 0; i < TFS_MASTER_RESERVED; i++)
    {
        COPYFROMCDF(pucCDFBuffer, &pMaster->ulReserved[i]);
    }
    COPYFROMCDF(pucCDFBuffer, &pMaster->ulCheckSum);
    DclAssert((pucCDFBuffer - pMaster->pucCDFBuffer) ==
               TFS_SIZEOF_sTfsMaster + TFS_MASTER_OFFSET);


    RelLeave();
}


/*  TfsMetaRootIsValid

    Is this MetaRoot valid?
*/
static RELSTATUS TfsMetaRootIsValid(
    sTfsVolume * pVolume,
    D_UINT8 * pCDFMetaRoot,
    D_UINT32 * pulCounter,
    D_BOOL * pfIsValid )
{
    sTfsMetaRoot   *pMetaRoot = (void *)0;      /*  This is only used for sizeof() fields */
    D_UINT8       *pucBuffer;
    D_UINT32       ulTemp;
    D_UINT32       iCheckSum;


    RelEnter("TfsMetaRootIsValid");


    *pfIsValid = FALSE;


    /*  pMetaRoot is only used for sizeof() structure members.  Some
        compilers give a warning for unused variable.  This should get around
        the compiler warning.
    */
    if(pMetaRoot)
    {
        pMetaRoot = 0;
    }


    /*  Check signature field
    */
    pucBuffer = pCDFMetaRoot;
    COPYFROMCDFTOTEMP(pucBuffer, &ulTemp, sizeof(pMetaRoot->ulSignature));
    if(ulTemp != TFS_META_SIG)
    {
        goto Done;
    }


    /*  Return the value of the counter
    */
    DclAssert(sizeof(ulTemp) == sizeof(pMetaRoot->ulCounter));
    COPYFROMCDFTOTEMP(pucBuffer, &ulTemp, sizeof(pMetaRoot->ulCounter));
    *pulCounter = ulTemp;


    /*  Verify checksum (don't checksum the checksum word)
    */
    DclAssert(sizeof(ulTemp) == sizeof(pMetaRoot->ulCheckSum));
    iCheckSum = TfsCalcCheckSum(pCDFMetaRoot, TFS_META_CHECKSUM,
            pVolume->ulBlockSize );
    pucBuffer = DclPtrAddByte(pCDFMetaRoot, TFS_SIZEOF_sTfsMetaRoot -
            sizeof(D_UINT32));
    COPYFROMCDFTOTEMP(pucBuffer, &ulTemp, sizeof(pMetaRoot->ulCheckSum));
    iCheckSum -= ulTemp;
    if(ulTemp != iCheckSum)
    {
        goto Done;
    }


    /*  As far as we can tell, it's valid
    */
    *pfIsValid = TRUE;

Done:

    RelLeave();
    return TFSERR_NONE;
}


/*  TfsMetaRootCDFRead

    Read the meta root from a buffer in common data format into an in-memory
    structure.
*/
static void TfsMetaRootCDFRead(
    sTfsMetaRoot * pInMemoryMetaRoot,
    D_UINT8 * pCDFMetaRoot,
    tfsBlockSize ulBlockSize )
{
    D_UINT8 * pucBuffer;
    D_UINT16 uIndex;


    RelEnter("TfsMetaRootCDFRead");

    (void)ulBlockSize;

    /*  Copy the CDF buffer to the in-memory metaroot
    */
    pucBuffer = pCDFMetaRoot;
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulSignature);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulCounter);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulIndexBlock);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulMapNextAlloc);
    for(uIndex = 0; uIndex < TFS_NUM_METAROOTS; uIndex++)
    {
        COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulMapStart[uIndex]);
    }
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulFreeBlocks);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulUsedBlocks);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulBadBlocks);
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulPortModified);
    for(uIndex = 0; uIndex < TFS_META_RESERVED; uIndex++)
    {
        COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulReserved[uIndex]);
    }
    COPYFROMCDF(pucBuffer, &pInMemoryMetaRoot->ulCheckSum);
#if 0  /* Not needed for the Reliance Reader */
    DclMemCpy(pInMemoryMetaRoot->ucEntries, pucBuffer,
            ulBlockSize - TFS_SIZEOF_sTfsMetaRoot );
#endif


    /*  Validate the buffer position
    */
    DclAssert( (pucBuffer - pCDFMetaRoot) == TFS_SIZEOF_sTfsMetaRoot );
    DclAssert( (TFS_META_ENTRIES(ulBlockSize)/TFS_META_EPB) ==
            (ulBlockSize - TFS_SIZEOF_sTfsMetaRoot) );


    RelLeave();
}


/*  TfsInitializeVolumeStructure
*/
static void TfsInitializeVolumeStructure(
    sTfsVolume * pVolume,
    tfsBlock ulIndexBlock )
{
    D_UINT16 uNodeEntries;
    D_UINT16 uIndirectEntries;
    static tfsChar szPortIllegalCharacters[] = { 0 };


    /*  Compute the sizes of the various file modes.
    */
    pVolume->ulInPlaceSize = (tfsOffset) pVolume->ulBlockSize -
                (TFS_INODE_FIELDS * sizeof(tfsBlock));
    uNodeEntries = (D_UINT16)((pVolume->ulBlockSize -
                TFS_SIZEOF_sTfsINode) / sizeof(tfsBlock));
    pVolume->ulDirectSize = (tfsOffset)uNodeEntries *
                pVolume->ulBlockSize;
    uIndirectEntries = (D_UINT16)((pVolume->ulBlockSize -
                TFS_SIZEOF_sTfsIndirect) / sizeof(tfsBlock));
    pVolume->ulIndirectSize = (tfsOffset)uNodeEntries *
                uIndirectEntries * pVolume->ulBlockSize;
    pVolume->ulMaxFileSize = (tfsOffset)uNodeEntries *
                uIndirectEntries * uIndirectEntries *
                pVolume->ulBlockSize;


    /*  max size may have been set to something smaller than 4G, so check.
    */
    if( (pVolume->ulBlockSize >= 1024) ||
            (pVolume->ulMaxFileSize > TFS_INODE_MAXSIZE) )
    {
        pVolume->ulMaxFileSize = TFS_INODE_MAXSIZE;
    }
    if( pVolume->ulDirectSize > pVolume->ulMaxFileSize )
    {
        pVolume->ulDirectSize = pVolume->ulMaxFileSize;
    }
    if( (pVolume->ulBlockSize >= 8192) ||
        (pVolume->ulIndirectSize > pVolume->ulMaxFileSize) )
    {
        pVolume->ulIndirectSize = pVolume->ulMaxFileSize;
    }


    /*  Set the block number and create a coord structure for the index file.
    */
    pVolume->ulIndexBlock = ulIndexBlock;
    TfsCoordCreate( pVolume, TFS_INDEX_INDEX, 0,
            &pVolume->sIndexFileCoord );


    /*  Set port limits and port specific illegal characters.
    */
    pVolume->ulPortPathLengthLimit = REL_CDF_PATH_LENGTH;
    pVolume->ulPortNameLengthLimit = REL_CDF_NAME_LENGTH;
    pVolume->szPortIllegalCharacters = szPortIllegalCharacters;


    /*  Mark this structure as valid.
    */
    pVolume->ulSignature = TFS_VOLUME_SIG;
}


/*-------------------------------------------------------------------
---------------------------------------------------------------------
----------- Interface to Reliance Reader API Functions --------------
---------------------------------------------------------------------
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    AllocateFileStructure() - Allocate a sDlLoaderFile structure

    Description
       This routine simply manipulates a pair of linked lists
       that control the classification (free or used) of elements
       of a static array of file structures.

    Parameters
       pDisk    - A pointer to the RELDISKINST structure to use

    Returns:
       A pointer to the next available RELFILEINFO structure or NULL
       if there are none free.
-------------------------------------------------------------------*/
static PRELFILEINFO AllocateFileStructure(
    PRELDISKINST pDisk)
{
    PRELFILEINFO    pFileNew;

    pDisk->uFilesOpen++;

    if(MAX_OPEN_FILES < pDisk->uFilesOpen)
    {
        pDisk->uFilesOpen--;
        return (void *)0;
    }

#if MAX_OPEN_FILES == 1

    /*  If only one open file is supported at a time, just
        return a pointer to the first and only entry in the
        RELFILEINFO array.
    */
    pFileNew = &pDisk->fi[0];
#else

    /*  Otherwise, get the next file structure from the free list.
    */
    pFileNew = pDisk->pFIFreeList;

    /*  remove this file structure from the free list
    */
    pDisk->pFIFreeList = pFileNew->pFINext;

    /*  add file structure to the used list and return it
    */
    pFileNew->pFINext = pDisk->pFIUsedList;
    pDisk->pFIUsedList = pFileNew;
#endif

    DclMemSet(pFileNew, 0, sizeof *pFileNew);

    pFileNew->pDisk = pDisk;

    /*  return new (possibly NULL) file
    */
    return pFileNew;
}


/*-------------------------------------------------------------------
    FreeFileStructure() - Free an allocated sDlLoaderFile structure

    Description
       This routine simply manipulates a pair of linked lists
       that control the classification (free or used) of elements
       of a static array of file structures.

    Parameters
       pFile    - A pointer to the RELFILEINFO structure to free

    Returns
       Nothing
-------------------------------------------------------------------*/
static void FreeFileStructure(
    PRELFILEINFO pFile)
{
#if MAX_OPEN_FILES > 1

    /*  allow a special case at the start of the list
    */
    if(pFile->pDisk->pFIUsedList == pFileToFree)
    {
        /*  unlink this file struc from the used list
        */
        pFile->pDisk->pFIUsedList = pFileToFree->pFINext;
    }
    else
    {
        PRELFILEINFO    pFileCurrent;

        pFileCurrent = pFile->pDisk->pFIUsedList;
        while(NULL != pFileCurrent->pFINext)
        {
            if(pFileCurrent->pFINext == pFileToFree)
            {
                /*  unlink file struc from used list
                */
                pFileCurrent->pFINext = pFile->pFINext;
                break;
            }
            else
            {
                pFileCurrent = pFileCurrent->pFINext;
            }
        }

        /*  Error condition, should never get here!
        */
        return;
    }

    /*  add file struc to head of free list
    */
    pFile->pFINext = pFile->pDisk->pFIFreeList;
    pFile->pDisk->pFIFreeList = pFile;
#endif

    pFile->pDisk->uFilesOpen--;

    DclMemSet(pFile, 0, sizeof *pFile);
    return;
}


#define ALIGNED_MASK_BITS       (DCL_ALIGNSIZE - 1)


/*-------------------------------------------------------------------
    RelMountVolume
-------------------------------------------------------------------*/
unsigned short RelReaderVolumeMount(
    PRELDISKINST           *ppDisk,
    PFNDEVICEREADSECTORS    pfnDevRead,
    unsigned long           ulBytesPerSector,
    void                   *pDevInfo)
{
    sTfsMasterBlock         sMaster = {0};   /* Init'ed for a picky compiler only */
    sTfsMetaRoot            sMetaRoot = {0}; /* Init'ed for a picky compiler only */
    sTfsVolume             *pVolume;
    D_UINT32                ulBuffer;
    D_BUFFER               *pMetaRootBuffer;
    D_BOOL                  fCounterIsHigher;
    D_UINT32                iCounter;
    D_BOOL                  fIsValid;
    D_BOOL                  fMetaRootValid;
    D_UINT16                uMetaRoot;
    PRELDISKINST            psDisk;
    RELSTATUS               iTfsError;

    /*  Allocate and initialize 'PRELDISKINST'
    */
    psDisk = DclMemAllocZero(sizeof(*psDisk));
    if( !psDisk )
    {
        /*  No "out of memory" error is defined for the Reader, so return the
            generic failure code.
        */
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }
    psDisk->pBlockDevInfo = pDevInfo;
    psDisk->pfnBlockDev = pfnDevRead;


    /*  Allocate and initialize the volume structure
    */
    pVolume = DclMemAllocZero(sizeof(*pVolume));
    if( !psDisk )
    {
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }
    psDisk->pVolume = pVolume;
    pVolume->pDisk = psDisk;


    /*  Using a temporary buffer, read the first sector of the master block and
        parse it to determine the block size of the Reliance format.
    */
    iTfsError = TFSERR_GENERAL;  /*  Set error in case the alloc fails  */
    psDisk->iSecsPerBlock = 1;  /*  Initialize for RelReadBlocks  */
    sMaster.pucCDFBuffer = DclMemAllocZero(ulBytesPerSector);
    if( sMaster.pucCDFBuffer )
    {
        /*  Read the master block
        */
        iTfsError = RelReadBlocks( pVolume, BUF_TYPE_CALLER,
                TFS_INIT_MASTER, 1, &sMaster.pucCDFBuffer );
        if( !iTfsError )
        {
            /*  Validate the master block
            */
            TfsMasterCDFRead(&sMaster);
            iTfsError = TfsMasterIsValid(&sMaster);
            if( !iTfsError )
            {
                /*  Verify the block size is compatable with the sector size of
                    the disk and Reliance requirements (basic sanity check).
                */
                if( (sMaster.ulBlockSize > TFS_MAX_BLOCK_SIZE) ||
                   (sMaster.ulBlockSize < TFS_MIN_BLOCK_SIZE) ||
                   (!DCLISPOWEROF2(sMaster.ulBlockSize)) ||
                    sMaster.ulBlockSize < ulBytesPerSector ||
                    sMaster.ulBlockSize % ulBytesPerSector)
                {
                    iTfsError = TFSERR_BLOCKSIZE;
                }
                else
                {
                    /*  Indicate the operation was successful and store the
                        block size.
                    */
                    iTfsError = TFSERR_NONE;
                    psDisk->iSecsPerBlock = sMaster.ulBlockSize / ulBytesPerSector;
                }
            }
        }

        DclMemFree(sMaster.pucCDFBuffer);
    }
    if( iTfsError )
    {
        goto Done;
    }


    /*  Allocate the buffers.
    */
    pVolume->asMiniBuf[0].pBuffer = DclMemAllocZero(sMaster.ulBlockSize);
    if( !pVolume->asMiniBuf[0].pBuffer )
    {
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }
    pVolume->asMiniBuf[1].pBuffer = DclMemAllocZero(sMaster.ulBlockSize);
    if( !pVolume->asMiniBuf[0].pBuffer )
    {
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }
    pVolume->asMiniBuf[2].pBuffer = DclMemAllocZero(sMaster.ulBlockSize);
    if( !pVolume->asMiniBuf[0].pBuffer )
    {
        iTfsError = TFSERR_GENERAL;
        goto Done;
    }

    /*  Initialize the buffers
    */
    for( ulBuffer = 0; ulBuffer < 3; ulBuffer++ )
    {
        DclAssert( DclPtrMask(pVolume->asMiniBuf[ulBuffer].pBuffer,
                ALIGNED_MASK_BITS) == 0 );
        pVolume->asMiniBuf[ulBuffer].ulLastRead = ~((D_UINT32)0);
    }


    /*  read each MetaRoot off the disk. Find the most recent valid one.
    */
    pVolume->ulBlockSize = sMaster.ulBlockSize;
    pVolume->ulNumBlocks = sMaster.ulNumBlocks;
    fIsValid = FALSE;
    iCounter = 0;
    for(uMetaRoot = 0; uMetaRoot < TFS_NUM_METAROOTS; ++uMetaRoot)
    {
        /*  is the MetaRoot valid? if not, ignore
        */
        iTfsError = RelReadBlocks( pVolume, BUF_TYPE_INODE,
                sMaster.ulMetaBlock[uMetaRoot], 1, &pMetaRootBuffer );
        if( iTfsError )
        {
            continue;
        }
        iTfsError = TfsMetaRootIsValid( pVolume, pMetaRootBuffer,
                &iCounter, &fMetaRootValid );
        if( (iTfsError) || (!fMetaRootValid) )
        {
            continue;
        }


        /*  Determine if the previous meta root counter was valid
        */
        if(fIsValid)
        {
            if(iCounter > sMetaRoot.ulCounter)
            {
                fCounterIsHigher = TRUE;
            }
            else
            {
                fCounterIsHigher = FALSE;
            }
            if(iCounter == D_UINT32_MAX)
            {
                if( sMetaRoot.ulCounter == 0)
                {
                    fCounterIsHigher = FALSE;
                }
            }
            else if(iCounter == 0)
            {
                if(sMetaRoot.ulCounter == D_UINT32_MAX)
                {
                    fCounterIsHigher = TRUE;
                }
            }
        }
        else
        {
            fCounterIsHigher = TRUE;
        }


        /*  is this the only valid or most recent?
        */
        if(fCounterIsHigher)
        {
            /*  found a valid one!
            */
            fIsValid = TRUE;

            /*  set up this one as most recent MetaRoot
            */
            TfsMetaRootCDFRead( &sMetaRoot, pMetaRootBuffer,
                    sMaster.ulBlockSize );
        }
    }

    /*  no valid MetaRoots, indicate that
    */
    if(!fIsValid)
    {
        iTfsError = TFSERR_INTERNAL;
        goto Done;
    }


    /*  Initialize the 'volume' structure
    */
    TfsInitializeVolumeStructure( pVolume, sMetaRoot.ulIndexBlock );


Done:

    if( iTfsError )
    {
        RelReaderVolumeDismount(psDisk);
        psDisk = NULL;
    }
    else
    {
        *ppDisk = psDisk;
    }

    return (unsigned short)iTfsError;
}


/*-------------------------------------------------------------------
    RelReaderVolumeDismount()
-------------------------------------------------------------------*/
void RelReaderVolumeDismount(
    PRELDISKINST psDisk)
{
    DclAssert( psDisk );

    if( psDisk->pVolume )
    {
        if( psDisk->pVolume->asMiniBuf[0].pBuffer )
        {
            DclMemFree( psDisk->pVolume->asMiniBuf[0].pBuffer );
            psDisk->pVolume->asMiniBuf[0].pBuffer = NULL;
        }

        if( psDisk->pVolume->asMiniBuf[1].pBuffer )
        {
            DclMemFree( psDisk->pVolume->asMiniBuf[1].pBuffer );
            psDisk->pVolume->asMiniBuf[1].pBuffer = NULL;
        }

        if( psDisk->pVolume->asMiniBuf[2].pBuffer )
        {
            DclMemFree( psDisk->pVolume->asMiniBuf[2].pBuffer );
            psDisk->pVolume->asMiniBuf[2].pBuffer = NULL;
        }

        DclMemFree( psDisk->pVolume );
        psDisk->pVolume = NULL;
    }

    DclMemFree( psDisk );
    return;
}


/*-------------------------------------------------------------------
    RelReaderFileOpen

    Open a file specified by the filename. Volume to be used has already been
    chosen, and is accessed through RelBootReadSectors.

        Try and find the file system directory entry for this file, if it's
        not there then free the struc and return NULL.  Otherwise the file
        access info we need will be copied from the file directory entry
        into our file control struct and we'll be ready to go!

    Returns error code if failed.
-------------------------------------------------------------------*/
PRELFILEINFO RelReaderFileOpen(
    PRELDISKINST pDisk,
    const char * szFileName)
{
    tfsIndex ulINodeIndex = 0;
    RELSTATUS iTfsError;
    PRELFILEINFO pFile;


    /*  allocate a new file structure, return NULL if none left
    */
    pFile = AllocateFileStructure(pDisk);
    if((void *)0 == pFile)
    {
        return pFile;
    }


    /*  verify the file exists
    */
    iTfsError = RelFindFile( pDisk->pVolume, szFileName, &ulINodeIndex );
    if( !iTfsError )
    {
        /*  set up the Open Control Block
        */
        pFile->fIsOpen = TRUE;
        TfsCoordCreate( pDisk->pVolume, ulINodeIndex, 0, &pFile->sCoord );
    }
    if( iTfsError )
    {
        FreeFileStructure(pFile);
        pFile = (void *)0;
    }


    return pFile;
}


/*-------------------------------------------------------------------
    RelReaderFileClose

    Close a file opened by RelReaderFileOpen.
-------------------------------------------------------------------*/
void RelReaderFileClose(
    PRELFILEINFO pFile)
{
    FreeFileStructure(pFile);
    return;
}


/*-------------------------------------------------------------------
    RelReaderFileRead() - Read sequential data from an open file

    Read the file that was opened with RelReaderFileOpen.  Reads
    into the specified buffer.
-------------------------------------------------------------------*/
unsigned long RelReaderFileRead(
    PRELFILEINFO pFile,
    unsigned long ulLength,
    void * pBuffer)
{
    tfsOffset iLength = ulLength;
    RELSTATUS iTfsError;

    /*  Assert argument conversions
    */
    DclAssert( sizeof(unsigned long) == sizeof(iLength) ); /* return arg */
    DclAssert( sizeof(ulLength) == sizeof(iLength) ); /* ulLength arg */

    /*  don't read anything if not open
    */
    if( !pFile )
    {
        return 0;
    }


    /*  read from the current position in the file
    */
    iTfsError = TfsINodeRead( pFile->pDisk->pVolume, &pFile->sCoord,
            pFile->iCurrentPosition, &iLength, (D_BUFFER *)pBuffer );
    pFile->usFileState = DL_LOADER_FILESTATE_SUCCESS;
    if( iTfsError == TFSERR_EOF  )
    {
        /*  Advance the current position counter
        */
        pFile->usFileState = DL_LOADER_FILESTATE_EOF;
    }
    else if( iTfsError )
    {
        pFile->usFileState = DL_LOADER_FILESTATE_IO_ERROR;
        iLength = 0;
    }

    pFile->iCurrentPosition += iLength;
    return iLength;
}


/*-------------------------------------------------------------------
    RelReaderFileState() - Get the file state information

    Description
       This function returns the file state information for
       the specified file, and clears the state.

       One of the following values will be returned:

          DL_LOADER_FILESTATE_SUCCESS    - No error
          DL_LOADER_FILESTATE_EOF        - End-of-file has been reached
          DL_LOADER_FILESTATE_IO_ERROR   - Error reading from the disk

    Parameters
       hFile       - The file handle

    Returns
       Returns the FILESTATE information.
-------------------------------------------------------------------*/
unsigned short RelReaderFileState(
    PRELFILEINFO pFile)
{
    unsigned short  usFileState;

    usFileState = pFile->usFileState;

    /*  Clear the state
    */
    pFile->usFileState = DL_LOADER_FILESTATE_SUCCESS;

    return usFileState;
}
