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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlrelcore.h $
    Revision 1.3  2009/01/11 01:18:15Z  brandont
    Updated to use DCL types and defines.
    Revision 1.2  2007/11/03 23:31:11Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/09/27 01:21:42Z  jeremys
    Initial revision
---------------------------------------------------------------------------*/


#ifndef TFS_CORE_H
#define TFS_CORE_H   1


#define TFS_USING_64BIT_FILESIZES       0


/*  Default volume label
*/
#define TFS_LABEL_DEFAULT       "NOLABEL"


/*  Reliance maximum block size

    Valid values: 512, 1024, 2048, 4096, 8192, 16384, 32768, and 65536
*/
#define TFS_MAX_BLOCK_SIZE      65536


/*  Reliance minimum block size.
*/
#define TFS_MIN_BLOCK_SIZE      128


/*  There should be some number of usable blocks when we finished
    formatting.  This number is arbutrary.
*/
#define REL_FORMAT_MIN_BLOCKS       100


/*  names to be be used for the index and bad block list
*/
#define TFS_SZINDEX_NAME   ".tfsindex"  /* note: no TfsStr() */
#define TFS_SZBADBLOCK_NAME   ".tfsbad" /* note: no TfsStr() */
#define TFS_SZVOLUMELABEL_NAME  ".tfslabel" /* note: no TfsStr() */


/*  Additional debug code
*/
#define TFS_CHECK_IMAP_ENTRY_COUNTS    0        /* IR ? */


/**************************************

    Global information

***************************************/

/*  layout version is the version of data physically laid out. Only changes
    if TFS on-disk layout changes. Independant of TFS_VERSION.

    0x105 = Original implementation
    0x106 = Implemented common data format for meta roots
    0x107 = Implemented common data format for IMAPs
    0x108 = Implemented common data format for index
    0x108 = Implemented common data format for DIR entries
    0x109 = Implemented common data format for INode entries
    0x10A = Exclusive disk format for Nucleus and XP Driver (1.15)
    0x10B = New CDF structures and using Unicode on disk
*/
#define TFS_LAYOUT_VERSION 0x10B


#define TFS_INVALID_BLOCK     D_UINT32_MAX


/*  the initial blocks on a freshly formatted disk
*/
#define TFS_INIT_MASTER       0


/*
    =========================================================================
    Master Block information
    =========================================================================
*/


/*  Number of metaroots used by Reliance
*/
#define TFS_NUM_METAROOTS       2


/*  Number of reserved fields in the master block
*/
#define TFS_MASTER_RESERVED     2


/*  Number of unused bytes before the master block structure when residing on
    the media
*/
#define TFS_MASTER_OFFSET       0x40   /* size of boot code */


/*  Master block checksum and signature
*/
#define TFS_MASTER_SIG          0x5453414DUL      /* 'MAST' */
#define TFS_MASTER_CHECKSUM     TFS_MASTER_SIG


/*  Common data format master block size.
*/
#define TFS_SIZEOF_sTfsMaster   0x30


/*  The Master Block is stored at the very beginning of the disk. It
    describes were to find the MetaRoots, iMaps and contains the
    initial boot code.

    Some fields are reserved to allow for systems to boot from the
    boot sector. See the DOS OS Integration Layer for an example.
*/
typedef struct {
    D_UINT32       ulSignature;    /* 00h 'MAST' */
    D_UINT16       uFmtBldNumber;  /* 04h Build number that formatted the media */
    D_UINT16       uTfsLayout;     /* 06h Layout version (must match) */
    tfsBlockSize    ulBlockSize;    /* 08h Logical block size */
    tfsBlock        ulNumBlocks;    /* 0Ch Total logical blocks */
    tfsBlock        ulMetaBlock[TFS_NUM_METAROOTS]; /* 10h block locations of MetaRoots */
    D_TIME         ullCreateDate;  /* 18h Date/time microseconds since Jan 1, 1970 */
    tfsBlock        ulNumIMapBlocks;/* 20h Number of iMap blocks */
    D_UINT32       ulReserved[TFS_MASTER_RESERVED];  /* 24h Reserved for future use */
    D_UINT32       ulCheckSum;      /* 2Ch Checksum of master block structure */
    D_UINT8       *pucCDFBuffer;   /* Pointer to the CDF buffer, does not reside on disk */
} sTfsMasterBlock;


/*
    =========================================================================
    MetaRoot information
    =========================================================================
*/


/*  Master block checksum and signature
*/
#define TFS_META_SIG            0x4154454DUL    /* 'META' */
#define TFS_META_CHECKSUM       TFS_META_SIG


/*  Common data format metaroot
*/
#define TFS_SIZEOF_sTfsMetaRoot 0x40


/*  This is the number of IMap blocks that the meta root can manage
*/
#define TFS_META_ENTRIES(_ulBlockSize) \
    (((_ulBlockSize) - TFS_SIZEOF_sTfsMetaRoot) * TFS_META_EPB)


/*  Number of reserved fields in the metaroot
*/
#define TFS_META_RESERVED       5


/*  MetaRoot entry fields
*/
#define TFS_META_MASK           0x03    /* mask for one entry */
#define TFS_META_SHIFT          1       /* shift to get to next entry */
#define TFS_META_EPB            4       /* entries per word */
#define TFS_META_IMAP_CURRENT   0x01    /* entry bitfield: current iMap */
#define TFS_META_IMAP_MODIFIED  0x02    /* entry bitfield: iMap has been
modified */


/*  The MetaRoot describes the file system on disk. It is written redundantly
    across multiple blocks so that we can recover from failure to write the
    MetaRoot. It is exactly one block in size.

    *** REGARDING COMMON DATA FORMAT ***

    Any changes to this structure intale reviewing the following
    functions/macros:

    TfsMetaRootCDFRead
    TfsMetaRootWrite
    TfsMetaRootIsValid
    TfsMetaRootMount
    TFS_SIZEOF_sTfsMetaRoot

    Also, using sizeof(sTfsMetaRoot) does not give you the size of the meta
    root on disk (in common data format).  Use TFS_SIZEOF_sTfsMetaRoot

    *** Important ***

    There are pieces of code that make assumptions about the order of the
    structure members.  Do not move rearrange the elements in the structure
    without checking all the uses of that structure.  An example code snippet
    follows where iValue is the new value of an imap entry and iOldValue was
    the previous value of an imap entry.

    pMetaRoot   = gpTfsVolume->pMetaRoot;
    piBlocks = &(pMetaRoot->ulFreeBlocks);
    ++piBlocks[iValue];
    --piBlocks[iOldValue];

    ***************
*/
typedef struct {
    D_UINT32       ulSignature;    /* 00h 'META' */
    D_UINT32       ulCounter;      /* 04h Used to determine most recent MetaRoot */
    tfsBlock        ulIndexBlock;   /* 08h Block number of index iNode */
    tfsBlock        ulMapNextAlloc; /* 0Ch Next block to allocate */
    tfsBlock        ulMapStart[TFS_NUM_METAROOTS];  /* 10h Start of each IMap block */
    tfsBlock        ulFreeBlocks;   /* 18h Number of free blocks */
    tfsBlock        ulUsedBlocks;   /* 1Ch Number of used blocks */
    tfsBlock        ulBadBlocks;    /* 20h Number of bad blocks */
    D_UINT32       ulPortModified; /* 24h Bits representing which ports have modified the media */
    D_UINT32       ulReserved[TFS_META_RESERVED];   /* 28h Reserved for future used */
    D_UINT32       ulCheckSum;     /* 3Ch Checksum of metaroot structure */
    D_UINT8        ucEntries[1];   /* 40h Keeps track of active imap blocks */
} sTfsMetaRoot;


/**************************************

    Volume information

***************************************/

#define TFS_VOLUME_SIG     0x554C4F56UL /* VOLU */

typedef struct {
    D_UINT32 ulLastRead;   /* 00h Block number last read into this buffer */
    void * pBuffer;         /* 04h Address of this buffer */
} MINBUF;

/*  sTfsCoordArray

    Sub-structure for the sTfsNewCoord structure
*/
typedef struct {
    D_UINT16   uArrayEntry;
    tfsBlock    ulValue;
} sTfsCoordArray;


/*  sTfsNewCoord
*/
typedef struct {
    D_UINT32       ulSignature;
    struct TfsTagVolume * pVolume;
    tfsIndex        ulINodeIndex;
    tfsOffset       ulFileOffset;
    tfsOffset       ulFileSize;
    sTfsCoordArray  sDouble;
    sTfsCoordArray  sIndirect;
    sTfsCoordArray  sDirect;
    tfsBlock        ulBlockNumber;
    tfsOffset       ulOffsetIntoBuffer;
} sTfsNewCoord;



typedef struct TfsTagVolume
{
    D_UINT32       ulSignature;        /* 00h volume signature */
    D_UINT32       ulBlockSize;        /* 14h Block size for this mounted volume */
    tfsBlock        ulNumBlocks;        /* 18h Total number of blocks on this volume */
    tfsOffset       ulInPlaceSize;      /* 2ch in-place size for a given iNode tree */
    tfsOffset       ulDirectSize;       /* 30h direct size for a given iNode tree */
    tfsOffset       ulIndirectSize;     /* 34h indirect size for a given iNode tree */
    tfsOffset       ulMaxFileSize;      /* 38h max file size for a given iNode tree */
    D_UINT32       ulPortPathLengthLimit;   /* 8ch Port specific path length limit */
    D_UINT32       ulPortNameLengthLimit;   /* 90h Port specific name length limit */
    tfsChar *       szPortIllegalCharacters; /* 94h Port specific illegal characters */

    MINBUF          asMiniBuf[3];       /* 00h Minimal buffer scheme */
    tfsBlock        ulIndexBlock;       /* 00h Block number for the index INode */
    sTfsNewCoord sIndexFileCoord;       /* 00h Coord structure for the index file */
    void * pDisk;                       /* 00h FFX stuff */
} sTfsVolume;


#define TfsVolumeIsValid(_pVolume) ((_pVolume) && (_pVolume)->ulSignature == TFS_VOLUME_SIG)


/*  # of blocks to remain free for use by delete operations
*/
#define TFS_RESERVED_BLOCKS         20


/*  IO layer capabilities
*/
#define TFS_IO_CAP_REMOVABLE  0x0001    /* device supports removable media */
#define TFS_IO_CAP_DISCARD    0x0002    /* device supports block discard */


#define TfsDiscardCapable(_pVolume)     ((_pVolume)->ulCapabilities & TFS_IO_CAP_DISCARD)
#define TfsRemoveCapable(_pVolume)      ((_pVolume)->ulCapabilities & TFS_IO_CAP_REMOVABLE)


/*
    =========================================================================
    IMap information
    =========================================================================
*/


/*  imap info
*/
#define TFS_IMAP_SIG        0x50414D49UL    /* 'IMAP' */
#define TFS_IMAP_FIELDS     2   /* Number of non-entry fields in the IMap */
#define TFS_IMAP_RESERVED   1   /* Number of reserved entries in the IMap */


/*  Each iMap entry can be one of 4 types. Each entry is 2 bits in size.
*/
#define TFS_IMAP_FREE       0x00 /* block is free for use */
#define TFS_IMAP_BRANCHED   0x01 /* block contains "branched" data, not yet freed */
#define TFS_IMAP_INUSE      0x02 /* block contains existing data. cannot be written to */
#define TFS_IMAP_NEW        0x03 /* block newly allocated. can be written to */


/*  masks & things for dealing with individual iMap entries
*/
#define TFS_IMAP_BRANCH_MASK    0xAA    /* mask off "branched" and "new" bits */
#define TFS_IMAP_EPB            4       /* entries per byte */
#define TFS_IMAP_EPW            (TFS_IMAP_EPB * 4)  /* entries per 32-bit word */
#define TFS_IMAP_SHIFT          1       /* bits per entry as a << shift value */
#define TFS_IMAP_MASK           0x03    /* mask for these bits */


/*  TfsTagIMap

    The iMap is a set of blocks that contain allocation bitmaps for the
    entire disk.

    *** Important ***

    Do not reference these structure members directly.  Use the CDF macros to
    retrieve the information.  Use add the sizeof each structure element to
    compute the offset of ucEntries.

    **************
*/
typedef struct {
    D_UINT32       iSignature; /* signature to verify this is an iMapblock */
    D_UINT32       iReserved[TFS_IMAP_RESERVED];       /* reserved for future use */
    D_UINT8        ucEntries[1];       /* blocks bits in this iMap block */
} sTfsIMap;


/*  This is the offset of the "iEntries" field in the IMAP.  It is now
    constant with the CDF changes.  This is only used by the checker to
double
    check the calculations using a different method.
*/
#define TFS_IMAP_OFFSET_ENTRIES     8


/*  For those places where code needed to know the sizeof sTfsIMap.  Note
that
    the sTfsImap structure is treated as if it contains 3 D_UINT32 fields
for
    calculation purposes.
*/
#define TFS_SIZEOF_sTfsIMap         0x0c


/*
    =========================================================================
    INode information
    =========================================================================
*/


/*  iNode info
*/
#define TFS_INODE_SIG           0x444F4E49UL      /* 'INOD' */


/*  Number of reserved fields in the INode
*/
#define TFS_INODE_RESERVED       5


/*  maximum possible file/dir size (in bytes)
*/
#define TFS_INODE_MAXSIZE       (D_UINT32_MAX - (((TFS_MAX_BLOCK_SIZE) * 2) - 1))


/*  Common data format INode structure.
*/
#define TFS_SIZEOF_sTfsINode    0x40


/*  # of 32-bit "fields" in an iNode
*/
#define TFS_INODE_FIELDS        (TFS_SIZEOF_sTfsINode / sizeof(tfsBlock))


/*  blocks are kept track of in one of 4 methods:
*/
#define TfsINodeGetMode(_pINode)    \
    ((_pINode)->uAttributes & TFS_INODE_FB_MODE)
#define TfsINodeSetMode(_pINode, _iMode)    \
    TfsINodeSetAttributes(&((_pINode)->uAttributes), (_iMode),TFS_INODE_FB_MODE)
#define TfsINodeInPlace(_pINode)    \
    (TfsINodeGetMode(_pINode) == TFS_INODE_FB_INPLACE)
#define TfsINodeDirect(_pINode)     \
    (TfsINodeGetMode(_pINode) == TFS_INODE_FB_DIRECT)
#define TfsINodeIndirect(_pINode)   \
    (TfsINodeGetMode(_pINode) == TFS_INODE_FB_INDIRECT)
#define TfsINodeInDouble(_pINode)   \
    (TfsINodeGetMode(_pINode) == TFS_INODE_FB_INDOUBLE)


/*  iNode iFlags masks
    (note: some functions use mode like an enum, must be lowest bits)
*/
#define TFS_INODE_FB_MODE       0x0003  /* mask for file mode bits */
#define TFS_INODE_FB_INPLACE    0x0000  /* file data is in the iNode */
#define TFS_INODE_FB_DIRECT     0x0001  /* file data is in blocks */
#define TFS_INODE_FB_INDIRECT   0x0002  /* file data is in indirect blocks */
#define TFS_INODE_FB_INDOUBLE   0x0003  /* file data is in double indirect
blocks */
#define TFS_INODE_FB_PERMANENT  0x0040  /* Internal attribute for index and
root */


/*  An iNode describes every file and directory on a TFS disk. It is really
    the root of a tree structure that defines a file so that data can be
    accessed rapidly even when using random I/O.

    The iNode contains everything about a file except its name. Names are
    stored in directories, which entries point to iNodes.

    *** REGARDING COMMON DATA FORMAT ***

    Any changes to this structure intale reviewing the following
    functions/macros:

    TfsINodeCDFRead
    TfsINodeCDFWrite
    TfsINodeCDFIsValid

    Also, using sizeof(sTfsINode) does not give you the size of the INode
    on disk (in common data format).  Use TFS_SIZEOF_sTfsINode.
*/
typedef struct {
    D_UINT32   ulSignature;        /* 00h 'INOD' */
    tfsIndex    ulIndex;            /* 04h points back to index */
#if TFS_USING_64BIT_FILESIZES
    D_UINT64   ullFileSize;        /* 08h File size in bytes */
#else
    D_UINT32   ullFileSize;        /* 08h File size in bytes */
    D_UINT32   ulPad;              /* 0ch Unused */
#endif
    D_TIME     ullCreateDate;      /* 10h when created (in milliseconds) */
    D_TIME     ullModifiedDate;    /* 18h when last modified (in milliseconds) */
    D_TIME     ullAccessDate;      /* 20h when last accessed (in milliseconds) */
    D_UINT16   uAttributes;        /* 28h CDF attributes */
    D_UINT16   uLinkCount;         /* 2Ah Number of entries linked to this INode */
    D_UINT32   ulReserved[TFS_INODE_RESERVED]; /* 2Ch Reserved for future use */
    tfsBlock *  pulEntries;         /* 34h Entries (or file data in direct mode) */
    D_UINT8 *  pucCDFBuffer;       /* 38h Pointer to the CDF buffer */
}sTfsINode;


/*
    =========================================================================
    Indirect INode information
    =========================================================================
*/


/*  Indirect blocks keep track of file blocks indirectly. Each indirect block
    is allocated from the index just like iNodes.
*/
#define TFS_INDIRECT_SIG            0x49444E49UL   /* 'INDI' */
#define TFS_INDOUBLE_SIG            0x494C4244UL   /* 'DBLI' */


/*  Common data format INode indirect structure.
*/
#if 0
#define TFS_INDIRECT_RESERVED       1
#define TFS_SIZEOF_sTfsIndirect     0x18
#else
#define TFS_INDIRECT_RESERVED       \
    ((TFS_SIZEOF_sTfsINode - 0x14)/sizeof(D_UINT32))
#define TFS_SIZEOF_sTfsIndirect     \
    (0x14 + (TFS_INDIRECT_RESERVED * sizeof(D_UINT32)))
#endif


/*
    =========================================================================
    Index information
    =========================================================================
*/


/*  signature for index data
*/
#define TFS_INDEX_SIG           0x58444E49UL /* INDX */


/*  for unused index entries (when iNodes are deleted)
*/
#define TFS_INDEX_UNUSED        0


/*  hard-coded original index #s at format time (0 is reserved)
*/
#define TFS_INDEX_HEADER        0   /* Hard-coded index set to TFS_INDEX_SIG */
#define TFS_INDEX_INDEX         1   /* Hard-coded index for index file */
#define TFS_INDEX_ROOTDIR       2   /* Hard-coded index for root dir */
#define TFS_INDEX_BADFILE       3   /* Hard-coded index for bad file */
#define TFS_INDEX_LABEL         4   /* Hard-coded index for volume label */
#define TFS_INDEX_FIRST_FREE    5   /* First usable index after format */

/*
    =========================================================================
    Directory information
    =========================================================================
*/


/*  ucEntryAttributes bits
*/
#define TFS_DIR_INUSE               0x01
#define TFS_DIR_ISDIR               0x02
#define TFS_DIR_ISLABEL             0x04


/*
*/
#define TFS_DIR_PACKET_ENTRY        0x80
#define TFS_SIZEOF_DIR_PACKET       0x10


/*  Common data format directory entry.
*/
#define TFS_SIZEOF_sTfsDirEntry     0x0f


/*  a single entry on disk, refers to both directories and files

    *** REGARDING COMMON DATA FORMAT ***

    Any changes to this structure intale reviewing the following
    functions/macros:

    TfsDirCDFReadEntry
    TfsDirCDFWriteEntry
    RelFmtAddRootDirEntry

    Also, using sizeof(sTfsDirEntry) does not give you the size of the dir
    entry on disk (in common data format).  Use TFS_SIZEOF_sTfsDirEntry.
*/
typedef struct {
    D_UINT8    ucPacketFlags;
    D_UINT16   uLC;                /* 00h Limitted checksum of known portable characters */
    D_UINT32   ulEntryLength;      /* 02h Length of entry (may be longer than name) */
    D_UINT16   uUnicodeNameLength; /* 04h Length of name in bytes */
    D_UINT8    ucEntryAttributes;  /* 06h Attributes for this entry */
    D_UINT8    ucReserved;         /* 07h Reserved for future use */
    tfsIndex    ulINodeIndex;       /* 08h Index of the INode for this entry */
} sTfsDirEntry;


#endif /* TFS_CORE_H */

