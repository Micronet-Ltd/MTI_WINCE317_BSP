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

/*-----------------------------------------------------------------
    PUBLIC BBM DEFINES
-------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxbbm.h $
    Revision 1.1  2009/03/12 21:42:54Z  keithg
    Initial revision
    Revision 1.17  2009/03/12 21:42:54Z  keithg
    Removed trailing comma from enum lists to placate a number of
    compilers that complain.
    Revision 1.16  2009/02/12 19:46:29Z  keithg
    Added BBM event information from bbm_private.h
    Revision 1.15  2009/01/18 08:43:26Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.14  2009/01/07 17:53:50Z  keithg
    Changed utility function names and added conditional prototypes for
    the functions requiring display output.
    Revision 1.13  2008/12/31 07:31:17Z  keithg
    Removed prototypes for functions that are now static; Removed
    enumeration types that are no longer used; Removed the hooks
    structure which is no more.
    Revision 1.12  2008/12/22 07:48:37Z  keithg
    Commented out prototype for obsolecent unformat function.
    Revision 1.11  2008/12/19 05:24:38Z  keithg
    Added support for the BBM statistics API.
    Revision 1.10  2008/12/18 07:31:38Z  keithg
    Updated the BbmMedia_t interface to use an opaque pointer to
    pass a context between the BBM and the user of the BBM.
    Revision 1.9  2008/12/10 08:19:51Z  keithg
    IO operations now use a page buffer provided with the media structure.
    Revision 1.8  2008/12/08 22:21:03Z  keithg
    Moved definition of the header structure into the private include and
    made the header and handle structures opaque types.
    Revision 1.7  2008/10/23 23:55:51Z  keithg
    Added enums and minimum block define from bbm_internals.
---------------------------------------------------------------------------*/


#ifndef BBM_PUBLIC_H_INCLUDED
#define BBM_PUBLIC_H_INCLUDED

#if FFXCONF_STATS_BBM
#include <fxstats.h>
#endif

/*  BBM requires at minimal number of blocks
*/
#define BBM_MIN_RESERVED_BLOCKS     4
#define BBM_SYSTEM_BLOCKS           2

/*  These are the type values used with searches through the remap list
    via the FfxBbm5EnumerateRemaps() function.
*/
enum {
    BBM_ENUM_ALL,
    BBM_ENUM_FREE,
    BBM_ENUM_SYSTEM,
    BBM_ENUM_RETIRED,
    BBM_ENUM_TEMPORARY,
    BBM_ENUM_FACTORYBAD
};

/*  Searchs implemented by the remapping subsystem
*/
enum {
    BBM_REMAP_FIND_PHYS,
    BBM_REMAP_FIND_DATA,
    BBM_REMAP_FIND_TYPE,
    BBM_REMAP_FIND_ALL
};

/*  The types of remap allocation made within the BBM.  Note these
    must match the 'type' fields used on the media.
*/
typedef enum {

    /*  The following are used to describe physical blocks within
        the flash.  The following values are commited to the media
        as a block type in the block list maintained by the BBM.
        They are held in the premap->phys.type member only.
    */
    BBM_BLOCK_FACTORY_BAD   = 0x00,
    BBM_BLOCK_SYSTEM        = 0x01,
    BBM_BLOCK_RETIRED       = 0x02,
    BBM_BLOCK_TEMPORARY     = 0x03,

    /*  The following are used to describe blocks within the
        flash and are only used internally.  They are *not*
        commited to the media. They are held in the remap.*.type
        field only.
    */
    BBM_BLOCK_LOCKED        = 0x08, /* Run-time allocated, not committed */
    BBM_BLOCK_RESERVED,             /* Describes a block in the reserved area */
    BBM_BLOCK_DATA,                 /* Describes a block in the data area */

    /*  The free map entry type is used both on the media and in
        volatile memory.  It indicates that an entry is free or
        otherwise unused.
    */
    BBM_BLOCK_UNUSED        = 0x0F
} BlockType_e;


/*-----------------------------------------------------------------
    PUBLIC BBM DATA STRUCTURES
-------------------------------------------------------------------*/


/*  Opaque types used by the BBM.
*/
typedef struct tagBbmInstance_s *BBMHANDLE;
typedef struct tagBbmHeader_s BbmHeader_t;
typedef struct tagBbmDisk_s BbmDisk_t;

/*  The IO structure is used to abstract IO to the media.  Access
    is accomplished in page size requests and writes are always
    completed from low address to high address and to an erased
    block.
*/
typedef struct {

    /* Size of a writeable page and eraseable blocks */
    D_UINT32    ulPageSize;
    D_UINT32    ulBlockSize;
    D_UINT32    ulTotalBlocks;

    /* Context to be passed to the IO calls */
    BbmDisk_t *ioContext;

    /*  Page sized buffer for completing read/write operations
        All page reads and writes are completed within this buffer.
    */
    D_UINT8    *rgbBuffer;

    /*  IO routines to read, write, and erase the flash.
    */
    FFXIOSTATUS (*ReadPage)(BbmDisk_t *, D_UINT32, D_UINT8 *);
    FFXIOSTATUS (*WritePage)(BbmDisk_t *, D_UINT32, D_UINT8 *);
    FFXIOSTATUS (*EraseBlock)(BbmDisk_t *, D_UINT32);
} BbmMedia_t;


typedef struct {

    /*  The number of data blocks that can be mapped or replaced
        and the number of blocks allocated for use for
        replacement blocks.  Together these indicate the total
        number of blocks within the disk to manage.
    */
    D_UINT32    ulDataBlocks;
    D_UINT32    ulReservedBlocks;

} BbmMount_t;


/*  Each entry within a BBM map contains an address of the original
    physical block or other unique address and an identifier that
    indicates what type of entry is recorded.
*/
typedef struct {
    unsigned    type    : 4;
    unsigned    key     : 28;
} BbmMapEntry_t;


/*  Remap entries are used to associate a data block with a spare
    block or vice-a-versa.  This structure is used both internally
    and externally to track remaped blocks and to fulfill query
    requests on blocks.

    The actual BBM entry that is placed into the media is built from
    the data element of this structure.
*/
typedef struct {
    BbmMapEntry_t  data;
    BbmMapEntry_t  phys;
} BbmRemap_t;

/*  The BBM event structures, types, and enum below are used for
    instrumentation purposes only.  These are for internal use only.
*/
typedef struct {
    unsigned nEvent;
    D_UINT32 ulBlock;
    BbmDisk_t *pioContext;
} BbmEvent_t;

typedef void (*PFNBBMEVENT)(BbmEvent_t *);

enum {
    BBM_EVENT_INVALID,
    BBM_EVENT_CREATE,
    BBM_EVENT_MOUNT,
    BBM_EVENT_DESTROY,
    BBM_EVENT_WRITE_HEADER,
    BBM_EVENT_SCRUB_HEADER
};



/*-----------------------------------------------------------------
    PUBLIC BBM INTERFACES
-------------------------------------------------------------------*/


FFXSTATUS FfxBbm5Create(
    BbmMedia_t   *pBbmMedia,
    BBMHANDLE    *phBbm);
FFXSTATUS FfxBbm5Destroy(
    BBMHANDLE    *phBbm);
FFXSTATUS FfxBbm5IsValidHandle(
    BBMHANDLE    handle);
FFXSTATUS FfxBbm5Mount(
    BBMHANDLE    hBbm,
    BbmMount_t   *pBbmMount,
    D_BOOL       fStrict);
FFXSTATUS FfxBbm5Unmount(
    BBMHANDLE   hBbm);
FFXSTATUS FfxBbm5MapPage(
    BBMHANDLE    hBbm,
    D_UINT32     *pulPage);
FFXSTATUS FfxBbm5QueryBlock(
    BBMHANDLE    hBbm,
    D_UINT32     ulBlock,
    BbmRemap_t   *pRemap);
FFXSTATUS FfxBbm5AcquireRemap(
    BBMHANDLE    hBbm,
    BbmRemap_t   *pRemap);
FFXSTATUS FfxBbm5CommitRemap(
    BBMHANDLE    hBbm,
    BbmRemap_t   *pRemap,
    unsigned     type,
    D_UINT32     ulKey);
FFXSTATUS FfxBbm5ReleaseRemap(
    BBMHANDLE    hBbm,
    BbmRemap_t   *pRemap);
FFXSTATUS FfxBbm5RemoveMapping(
    BBMHANDLE    hBbm,
    D_UINT32     ulBlock);
FFXSTATUS FfxBbm5EnumerateRemaps(
    BBMHANDLE    hBbm,
    unsigned     type,
    BbmRemap_t   *rgRemap,
    D_UINT32     *pulCount,
    D_UINT32     *pulContinueKey);
FFXSTATUS FfxBbm5Format(
    BBMHANDLE   hBbm,
    D_UINT32    ulReservedCount,
    D_UINT32    ulBadBlockCount,
    D_UINT32    *rgulBadBlocks);
FFXSTATUS FfxBbm5GetHeaderFromBuffer(
    D_UINT8 * rgbBuffer,
    D_UINT32 ulLength,
    BbmHeader_t *pheader);
FFXSTATUS FfxBbm5PutHeaderInBuffer(
    D_UINT8 * rgbBuffer,
    D_UINT32 ulLength,
    BbmHeader_t *pheader);
FFXSTATUS FfxBbm5GetMapEntryFromBuffer(
    D_UINT8 * rgbBuffer,
    D_UINT32 ulLength,
    BbmMapEntry_t *pentry);
FFXSTATUS FfxBbm5PutMapEntryInBuffer(
    D_UINT8 * rgbBuffer,
    D_UINT32 ulLength,
    BbmMapEntry_t *pentry);
void FfxBbm5EventFunction(
    PFNBBMEVENT *pfnOld,
    PFNBBMEVENT *pfnNew);
#if FFXCONF_STATS_BBM
FFXSTATUS FfxBbm5StatsQuery(
    BBMHANDLE       hBbm,
    FFXBBMSTATS    *pBbmStats);
#endif
#if DCLCONF_OUTPUT_ENABLED
void FfxBbm5DisplayHeader(
    BbmHeader_t *pheader);
void FfxBbm5DisplayMapEntry(
    BbmMapEntry_t *pentry);
void FfxBbm5DisplayRemap(
    BbmRemap_t *premap);
void FfxBbm5DisplayMap(
    D_UINT8 * rgbBuffer,
    D_UINT32 ulLength);
#endif


#endif

