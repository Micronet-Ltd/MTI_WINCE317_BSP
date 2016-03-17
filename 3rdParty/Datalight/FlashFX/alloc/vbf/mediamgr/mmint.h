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

    Internal Media Manager interface.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: mmint.h $
    Revision 1.20  2010/12/12 06:49:07Z  garyp
    Removed a duplicated symbol -- no functional changes.
    Revision 1.19  2009/03/12 20:56:04Z  billr
    Resolve bug 2124: Page buffers are allocated on the stack.
    Revision 1.18  2009/01/29 19:42:29Z  billr
    Resolved Bug 1601: VBF write protects the disk after an uncorrectable ECC 
    error.  Implement new Media Manager function FfxMediaMgrCopyBadBlock().  
    Allow per-instance data for media-specific functions.
    Revision 1.17  2008/03/31 02:11:59Z  garyp
    Minor type fixes.
    Revision 1.16  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/11/01 00:43:38Z  Garyp
    Major update to eliminate the inconsistent use of D_UINT16 types.  All block
    indexes and counts which were D_UINT16 are now consistently unsigned.
    Replaced the use of Return_t and IOReturn_t with FFXSTATUS and FFXIOSTATUS.
    Improved error handling.
    Revision 1.14  2007/10/16 23:41:04Z  Garyp
    Minor type change to keep MSVC6 happy.
    Revision 1.13  2007/10/10 22:30:33Z  billr
    Resolve Bug 412.
    Revision 1.12  2007/10/04 23:39:18Z  billr
    Resolve Bug 970.
    Revision 1.11  2007/04/19 23:57:48Z  timothyj
    Added fLargeAddressing parameter, to indicate whether the on-media format
    should use large flash array compatible addressing.
    Revision 1.10  2007/04/11 23:15:55Z  timothyj
    Removed interface to functions that became NOR or ISWF specific after
    changes to support large NAND arrays out of the mediamgr.c module
    (UnitLinearAddress and DataBlockAddress).
    Revision 1.9  2006/10/06 01:00:19Z  Garyp
    Added FfxMMDataBlockNum().
    Revision 1.8  2006/03/20 20:05:29Z  Garyp
    Fixed so that NAND_TAG_BUFFER_DIM is no larger than necessary.
    Revision 1.7  2006/03/05 03:01:56Z  Garyp
    Minor structure modification.
    Revision 1.6  2006/02/26 01:59:52Z  Garyp
    Split out the NOR and NAND tag handling logic.
    Revision 1.5  2006/02/01 00:07:02Z  Garyp
    Modified to use the updated FML interface.
    Revision 1.4  2006/01/25 03:53:16Z  Garyp
    Updated to conditionally build only if NAND support is enabled.
    Revision 1.3  2006/01/11 00:25:46Z  billr
    Comment change only.
    Revision 1.2  2006/01/05 19:12:18Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/10/25 21:50:54Z  Garyp
    Initial revision
    Revision 1.2  2005/10/25 23:50:54Z  Garyp
    Eliminated the use of STD_NAND_DATA_SIZE.  Changed some terminology
    from "blocks" to "pages" for clarity.  Began changing from a compile-time
    block size to a run-time page size paradigm.
    Revision 1.1  2005/10/02 02:58:40Z  Garyp
    Initial revision
    Revision 1.3  2005/09/09 21:31:00Z  johnb
    Modified this to not explicitly set the size that the ADS 1.2
    compiler will not complain.
    Revision 1.2  2005/08/31 22:39:38Z  Cheryl
    Add VBF_SIGNATURE.  Remove MIN/MAX (in vbfconf.h)
    Revision 1.1  2005/08/27 20:09:50Z  Cheryl
    Initial revision
---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Format of an allocation entry:

    bit   meaning
    ---   -------
     15   zero => remaining bits represent client data
     14   one => valid, zero => discarded
    13:0  client address (XOR with hash in second half on NOR)
-------------------------------------------------------------------*/
#define MEDIA_ALLOC_TYPE_SHIFT (14)
#define MEDIA_ALLOC_TYPE(x) ((x) >> MEDIA_ALLOC_TYPE_SHIFT)
#define MEDIA_ALLOC_MASK ((D_UINT16) ((1 << MEDIA_ALLOC_TYPE_SHIFT) - 1))


/*-------------------------------------------------------------------
    The high two bits of an allocation entry encode its type.  These
    are the type values after the entry has been shifted down by
    MEDIA_ALLOC_TYPE_SHIFT.
-------------------------------------------------------------------*/
#define MEDIA_ALLOC_TYPE_DISCARDED (0)
#define MEDIA_ALLOC_TYPE_VALID     (1)
#define MEDIA_ALLOC_TYPE_INVALID   (3)  /* possible from write interruption */


/*-------------------------------------------------------------------
    Make the sixteen-bit basic allocation entry (first half).  This
    is the same on NAND and NOR.
-------------------------------------------------------------------*/
#define MAKE_ALLOC(type, value)                         \
   ((D_UINT16) (((type) << MEDIA_ALLOC_TYPE_SHIFT)      \
              | ((value) & MEDIA_ALLOC_MASK)))


/*-------------------------------------------------------------------
    Tag information for NAND.
-------------------------------------------------------------------*/
#if FFXCONF_NANDSUPPORT
    typedef D_BUFFER FFXNANDTAG[FFX_NAND_TAGSIZE];

    /*  Number of tags to buffer at once.  This number is chosen
        somewhat arbitrarily so that the buffer will be about the
        same size as other buffers.  It is expected to fit in an
        int (16-bit signed value)
    */
    #define NAND_TAG_BUFFER_DIM (64)
#endif


/*-------------------------------------------------------------------
    Information for NOR/ISWF.
-------------------------------------------------------------------*/
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    typedef D_BUFFER FFXNORTAG[FFX_NOR_TAGSIZE];

    /*  Number of tags to buffer at once.  This number is chosen
        somewhat arbitrarily so that the buffer will be about the
        same size as other buffers.  It is expected to fit in an
        int (16-bit signed value)

        NOTE: It appears that there is no value in this being any
              larger than ALLOC_BUFFER_DIMENSION in vbfreg.c --
              just wastes stack space. (TODO)
    */
    #define NOR_TAG_BUFFER_DIM  (512 / FFX_NOR_TAGSIZE)

    /*  A byte mask for the bit to clear to discard an allocation.  It is
        bit 14 of the allocation entry, but the mask is only for the high
        byte.  This is valid only on NOR flash, NAND flash must not be
        rewritten.
    */
    #define NOR_ALLOC_DISCARD_MASK ((D_UCHAR) ~(1 << (MEDIA_ALLOC_TYPE_SHIFT - CHAR_BIT)))

    /*  On NOR flash, this value is XORed with the first half of the
        allocation entry to produce the value for the second half.  It
        is chosen so that it does not affect the entry's status bits
        (high-order two bits).
    */
    #define NOR_HASH (0x3865)
#endif

/*  Actual signature defined in mediamgr.c - this was done
    to get around some obfuscator string inconsistencies
*/
extern const D_UCHAR achSig[];


typedef struct MEDIAMGRINSTANCE MEDIAMGRINSTANCE;

/*-------------------------------------------------------------------
    MMDEVICE
-------------------------------------------------------------------*/
typedef struct
{
    FFXIOSTATUS (*ReadBlocks)        (const VBFPartition *, LinearUnit_t, unsigned, unsigned, D_BUFFER *);
    FFXIOSTATUS (*WriteBlocks)       (const VBFPartition *, LinearUnit_t, unsigned, unsigned, unsigned, const D_BUFFER *);
    FFXIOSTATUS (*ReadAllocations)   (const VBFPartition *, LinearUnit_t, unsigned, unsigned, Allocation[]);
    FFXIOSTATUS (*DiscardAllocations)(const VBFPartition *, LinearUnit_t, unsigned, unsigned);
    FFXIOSTATUS (*ExpendAllocations) (const VBFPartition *, LinearUnit_t, unsigned, unsigned);

    /*  All the rest of these functions are optional, and these
        pointers may be NULL if not needed.  They are placed last so
        they can be implicitly initialized to NULL by not initializing
        them at all.
    */
    void        (*Destroy)           (MEDIAMGRINSTANCE *);
    FFXIOSTATUS (*ReadHeader)        (MEDIAMGRINSTANCE *, D_UINT32, EUH *);
    FFXSTATUS   (*FormatUnit)        (MEDIAMGRINSTANCE *, D_UINT32, const EUH *, D_BOOL, D_BOOL);
#if FFXCONF_NANDSUPPORT
    FFXSTATUS   (*CopyBadBlock)      (const VBFPartition *, LinearUnit_t, unsigned, LinearUnit_t, unsigned);
    FFXSTATUS   (*BlankCheck)        (const VBFPartition *, LinearUnit_t, unsigned);
    FFXSTATUS   (*Verify)            (const VBFPartition *, LinearUnit_t, unsigned);
#endif
} MMDEVICE;


/*-------------------------------------------------------------------
    MMDEVINFO

    This is an opaque structure available for use by an instance of
    media-specific Media Manager operations.  Not all media types need
    this.  The media-specific DeviceInit function allocates this and
    fills it in as necessary.
-------------------------------------------------------------------*/
typedef struct MMDEVINFO MMDEVINFO;


/*-------------------------------------------------------------------
    MEDIAMGRINSTANCE
-------------------------------------------------------------------*/
struct MEDIAMGRINSTANCE
{
    const MMDEVICE *pMmOps;
    MMDEVINFO      *pMmInfo;
    FFXFMLHANDLE    hFML;
    D_UINT16        uPageSize;
    D_UINT16        uSpareSize;
    D_UINT16        uDevType;
    unsigned        multiLevelCell:1;   /* MLC flash                                    */
    unsigned        mergeWrites:1;      /* merge old data with new to write (NOR)       */
    unsigned        programOnce:1;      /* don't program bit (NOR) or page (NAND) twice */
};


/*-------------------------------------------------------------------
    Prototypes for public helper functions found in mediamgr.c
-------------------------------------------------------------------*/
D_BOOL      FfxMMIsErasedAlloc(const D_BUFFER *pBuffer, unsigned nBufferLen);
void        FfxMMBuildEUH(D_BUFFER *, const EUH *, D_BOOL);
D_BOOL      FfxMMParseEUH(const D_BUFFER *, EUH *);
D_UINT32    FfxMMDataBlockNum(const VBFPartition *pVBF, LinearUnit_t lnu, unsigned nBlock);


