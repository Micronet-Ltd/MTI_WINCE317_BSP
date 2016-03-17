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

    This header defines internal interfaces within the FML.  It is not meant
    to be included or used by any code outside the FML.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fml.h $
    Revision 1.20  2010/12/15 05:38:30Z  glenns
    Expand FFXFMLLAYOUT structure to support FML disk spanning
    of devices with BBMv5. See bug 3280.
    Revision 1.19  2009/07/29 20:33:10Z  garyp
    Merged from the v4.0 branch.  Major update to support the concept of a
    Disk which spans multiple Devices.  Added OTP support.  Added locking
    information to the FFXFMLDISK structure.  Added support for the power
    suspend/resume API.
    Revision 1.18  2009/01/20 04:03:14Z  keithg
    Removed inadvertent NAND conditional around the stats interfaces.
    Revision 1.17  2009/01/19 04:34:41Z  keithg
    Removed code and compile time condition for support of the old
    BBM which is now obsolete.
    Revision 1.16  2008/12/24 06:06:21Z  keithg
    Added support new IOCTL support for raw erase operations.
    Revision 1.15  2008/12/18 17:48:04Z  keithg
    Replace the WriteNativePages functionality which as incorrectly
    removed in the previous check in.
    Revision 1.14  2008/12/17 23:51:00Z  keithg
    Corrected compile time conditions and removed unused IOCTLs.
    Revision 1.13  2008/12/12 07:31:31Z  keithg
    Added support for read/write native pages and setting the block status.
    Revision 1.12  2008/10/23 23:19:11Z  keithg
    Added block info function prototypes.
    Revision 1.11  2008/05/24 05:47:41Z  keithg
    Raw block status functionality is no longer dependant upon the
    inclusion of BBM functionality.
    Revision 1.10  2008/05/07 01:45:14Z  garyp
    Added prototypes.
    Revision 1.9  2008/03/26 02:40:45Z  Garyp
    Prototype updates.
    Revision 1.8  2007/11/03 23:49:36Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/07/12 18:37:17Z  timothyj
    Removed unused offset and length.
    Revision 1.6  2007/06/29 21:09:58Z  rickc
    Added FfxFmlNandGetPhysicalBlockStatus()
    Revision 1.5  2006/11/10 20:20:37Z  Garyp
    Updated to support new statistics gathering capabilities.
    Revision 1.4  2006/05/17 19:21:02Z  Garyp
    Added support for erasing NOR boot blocks.
    Revision 1.3  2006/03/04 21:36:55Z  Garyp
    Added RawPageRead/Write() support.
    Revision 1.2  2006/02/09 02:13:11Z  Garyp
    Refactored such that the FML is literally just the flash mapping layer.
    Other functionality is moved into the Device Manager layer.
    Revision 1.1  2005/11/26 05:59:30Z  Pauli
    Initial revision
    Revision 1.3  2005/11/26 05:59:29Z  Garyp
    Minor prototype tweak.
    Revision 1.2  2005/10/22 08:15:53Z  garyp
    Minor cleanup -- no functional changes.
    Revision 1.1  2005/10/10 17:54:58Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FML_H_INCLUDED
#define FML_H_INCLUDED

#include <fxiosys.h>
#include <fxstats.h>

typedef struct sFFXFMLLAYOUT
{
    FFXFIMDEVHANDLE         hFimDev;
    D_UINT32                ulPageOffset;
    D_UINT32                ulPageCount;
    D_UINT32                ulBlockOffset;
    D_UINT32                ulBlockCount;
  #if FFXCONF_NANDSUPPORT
    D_UINT32                ulRawBlockOffset;
    D_UINT32                ulRawBlockCount;
    D_UINT32                ulRawPageOffset;
    D_UINT32                ulRawPageCount;
  #endif
    struct sFFXFMLLAYOUT   *pNext;
} FFXFMLLAYOUT;

typedef struct sFFXFMLDISK
{
    FFXDISKHANDLE           hDisk;
    unsigned                nDiskNum;
    unsigned                nStartDevNum;
    unsigned                nDevCount;      /* Disk could span multiple Devices */
    unsigned                nBytePageShift;
    D_UINT32                ulTotalPages;   /* Total pages across Devices (possibly) */
    D_UINT32                ulTotalBlocks;  /* Total blocks across Devices (possibly) */
    D_UINT16                uDiskFlags;
    D_UINT16                uLockFlags;
    FFXFMLLAYOUT           *pLayout;        /* Chain of structures, one for each Device */
  #if FFXCONF_NANDSUPPORT
    unsigned                nSpareSize;     /* Total bytes in the spare area */
  #endif
  #if FFXCONF_OTPSUPPORT
    unsigned                nOTPPages;      /* # of OTP pages, or zero if unsupported */
  #endif
  #if FFXCONF_POWERSUSPENDRESUME
    D_ATOMIC32              ulSuspendCount;
  #endif
  #if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    D_UINT32                ulBlockSize;
    D_UINT32                ulBBsPerBlock;  /* Boot blocks per regular block */
  #endif
  #if FFXCONF_STATS_FML
    FFXFMLSTATS             stats;
    D_UINT32                ulPagesPerBlock;
  #endif
    struct sFFXFMLDISK     *pNext;
} FFXFMLDISK;


FFXIOSTATUS FfxFmlBlockLockFreeze(          FFXFMLHANDLE hFML, FFXIOR_FML_LOCK_FREEZE *pFmlReq);
FFXIOSTATUS FfxFmlBlockLock(                FFXFMLHANDLE hFML, FFXIOR_FML_LOCK_BLOCKS *pFmlReq);
FFXIOSTATUS FfxFmlBlockUnlock(              FFXFMLHANDLE hFML, FFXIOR_FML_UNLOCK_BLOCKS *pFmlReq);


#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
FFXIOSTATUS FfxFmlNorReadControlData(FFXFMLHANDLE hFML, FFXIOR_FML_READ_CONTROLDATA *pFmlReq);
FFXIOSTATUS FfxFmlNorWriteControlData(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_CONTROLDATA *pFmlReq);
FFXIOSTATUS FfxFmlNorEraseBootBlocks(FFXFMLHANDLE hFML, FFXIOR_FML_ERASE_BOOT_BLOCKS *pFmlReq);
#endif

#if FFXCONF_NANDSUPPORT
FFXIOSTATUS FfxFmlNandReadUncorrectedPages( FFXFMLHANDLE hFML, FFXIOR_FML_READ_UNCORRECTEDPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteUncorrectedPages(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_UNCORRECTEDPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandReadTaggedPages(FFXFMLHANDLE hFML, FFXIOR_FML_READ_TAGGEDPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteTaggedPages(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_TAGGEDPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandReadRawPages(FFXFMLHANDLE hFML, FFXIOR_FML_READ_RAWPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteRawPages(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_RAWPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandReadSpares(FFXFMLHANDLE hFML, FFXIOR_FML_READ_SPARES *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteSpares(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_SPARES *pFmlReq);
FFXIOSTATUS FfxFmlNandReadTags(FFXFMLHANDLE hFML, FFXIOR_FML_READ_TAGS *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteTags(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_TAGS *pFmlReq);
FFXIOSTATUS FfxFmlNandWriteNativePages(FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_NATIVEPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandReadNativePages(FFXFMLHANDLE hFML, FFXIOR_FML_READ_NATIVEPAGES *pFmlReq);
FFXIOSTATUS FfxFmlNandEraseRawBlocks(FFXFMLHANDLE hFML, FFXIOR_FML_ERASE_RAWBLOCKS *pFmlReq);

#if FFXCONF_BBMSUPPORT
FFXIOSTATUS FfxFmlNandGetBlockInfo(FFXFMLHANDLE hFML, FFXIOR_FML_GET_BLOCK_INFO    *pFmlReq);
FFXIOSTATUS FfxFmlNandGetRawBlockInfo(FFXFMLHANDLE hFML, FFXIOR_FML_GET_RAW_BLOCK_INFO    *pFmlReq);
FFXIOSTATUS FfxFmlRetireRawBlock(FFXFMLHANDLE hFML, FFXIOR_FML_RETIRE_RAW_BLOCK *pFmlReq);
#endif
FFXIOSTATUS FfxFmlNandGetBlockStatus(FFXFMLHANDLE hFML, FFXIOR_FML_GET_BLOCK_STATUS *pFmlReq);
FFXIOSTATUS FfxFmlNandGetRawBlockStatus(FFXFMLHANDLE hFML, FFXIOR_FML_GET_RAW_BLOCK_STATUS *pFmlReq);
FFXIOSTATUS FfxFmlNandSetRawBlockStatus(FFXFMLHANDLE hFML, FFXIOR_FML_SET_RAW_BLOCK_STATUS *pFmlReq);
FFXIOSTATUS FfxFmlNandGetRawPageStatus(FFXFMLHANDLE hFML, FFXIOR_FML_GET_RAW_PAGE_STATUS *pFmlReq);
#if FFXCONF_OTPSUPPORT
FFXIOSTATUS FfxFmlOtpReadPages(             FFXFMLHANDLE hFML, FFXIOR_FML_READ_OTPPAGES *pFmlReq);
FFXIOSTATUS FfxFmlOtpWritePages(            FFXFMLHANDLE hFML, FFXIOR_FML_WRITE_OTPPAGES *pFmlReq);
FFXIOSTATUS FfxFmlOtpLock(                  FFXFMLHANDLE hFML, FFXIOR_FML_LOCK_OTP *pFmlReq);
#endif
#endif

#if FFXCONF_STATS_FML
void FfxFmlBlockStatsAddReads(FFXFMLHANDLE hFML, D_UINT32 ulStartPage, D_UINT32 ulPageCount);
void FfxFmlBlockStatsAddWrites(FFXFMLHANDLE hFML, D_UINT32 ulStartPage, D_UINT32 ulPageCount);
#endif


#endif  /* FML_H_INCLUDED */

