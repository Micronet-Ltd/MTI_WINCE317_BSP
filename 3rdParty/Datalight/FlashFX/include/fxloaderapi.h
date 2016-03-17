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

    This is the master header file for the FlashFX Loader API, which may be
    built outside the standard FlashFX build process.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxloaderapi.h $
    Revision 1.6  2010/07/17 23:50:23Z  garyp
    Updated so the "AltUnlock" functionality is optional, and must be
    explicitly turned on.
    Revision 1.5  2010/06/30 19:28:36Z  garyp
    Major update so that Loader Device and Disk handles are now separate
    objects rather than pseudo Driver Device and Disk handles.  Updated
    FfxLoaderUnlockBlocks(), so that when using flash which only supports
    unlocking a single range, the lowest unlocked block will be tracked.
    and subsequent requests to unlock higher blocks will be ignored.
    Revision 1.4  2010/06/17 19:16:47Z  garyp
    Added FfxLoaderReadSectors() and FfxLoaderWriteSectors() which
    provide allocator level read/write sector functionality, assuming the
    allocator is enabled. 
    Revision 1.3  2009/12/09 19:51:30Z  garyp
    Added FFXLOADER_DEV_UNLOCK.
    Revision 1.2  2009/03/18 22:49:54Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.5  2009/03/18 22:49:54Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.4  2009/03/02 19:47:32Z  garyp
    Updated FfxLoaderDiskCreate() to allow the Device number to be specified.
    Revision 1.1.1.3  2009/02/27 20:20:30Z  garyp
    Updated the lock-freeze function to take a start block and count.
    Revision 1.1.1.2  2009/02/05 23:17:48Z  garyp
    Cleaned up the interface so it is better suited for long-term general use.
    Revision 1.1  2009/02/03 23:20:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXLOADERAPI_H_INCLUDED
#define FXLOADERAPI_H_INCLUDED


/*-------------------------------------------------------------------
    Type: FFXLOADERHANDLE

    A handle to a FlashFX Loader object, created with FfxLoaderCreate().

    Type: FFXLOADERDEVHANDLE

    A handle to an FlashFX Loader Device object, created with
    FfxLoaderDeviceCreate().

    Type: FFXLOADERDISKHANDLE

    A handle to an FlashFX Loader Disk object, created with
    FfxLoaderDiskCreate().
-------------------------------------------------------------------*/
typedef struct sFFXLDRINFO       *FFXLOADERHANDLE;
typedef struct sFFXLDRDEVICEINFO *FFXLOADERDEVHANDLE;
typedef struct sFFXLDRDISKINFO   *FFXLOADERDISKHANDLE;


/*-------------------------------------------------------------------
    Type: FFXLOADERDEVINFO

    A structure containing information about a FlashFX Loader Device.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulBlockSize;        /* Number of bytes in a block           */
    D_UINT32    ulReservedBlocks;   /* Low reserved blocks (inaccessible)   */
    D_UINT32    ulRawBlocks;        /* Raw blocks in the device             */
    D_UINT32    ulTotalBlocks;      /* Total usable blocks in the device    */
    D_UINT32    ulChipBlocks;       /* Blocks per physical chip             */
    unsigned    nPageSize;          /* Bytes per page                       */
} FFXLOADERDEVINFO;


/*-------------------------------------------------------------------
    Type: FFXLOADERDISKINFO

    A structure containing information about a FlashFX Loader Disk.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT32    ulTotalBlocks;
    D_UINT32    ulBytesPerBlock;
    D_UINT32    ulAllocatorSectors; /* Only valid if an allocator is being used */
    unsigned    nSectorsPerBlock;
    unsigned    nBytesPerSector;
    D_BOOL      fIsNAND : 1;
} FFXLOADERDISKINFO;


/*  Device initialization flags for FfxLoaderDeviceCreate()
*/
#define FFXLOADER_DEV_USEBBM            (0x00000001)
#define FFXLOADER_DEV_UNLOCK            (0x00000002)
#define FFXLOADER_DEV_RESERVEDMASK      (0xFFFFFFFC)

/*  Disk initialization flags for FfxLoaderDiskCreate()
*/
#define FFXLOADER_DISK_USEALLOCATOR     (0x00000001)
#define FFXLOADER_DISK_SPANDEVICES      (0x00000002)
#define FFXLOADER_DISK_ALTUNLOCK        (0x00000004)
#define FFXLOADER_DISK_RESERVEDMASK     (0xFFFFFFF8)


/*-------------------------------------------------------------------
    Prototypes for Public Functions
-------------------------------------------------------------------*/

FFXLOADERHANDLE     FfxLoaderCreate(        DCLINSTANCEHANDLE hDclInst);
FFXSTATUS           FfxLoaderDestroy(       FFXLOADERHANDLE hLdr);
FFXLOADERDEVHANDLE  FfxLoaderDeviceCreate(  FFXLOADERHANDLE hLdr, unsigned nDevNum, D_UINT32 ulExtraReservedLowKB, D_UINT32 ulExtraReservedHighKB, D_UINT32 ulMaxArraySizeKB, D_UINT32 ulFlags);
FFXSTATUS           FfxLoaderDeviceDestroy( FFXLOADERDEVHANDLE hDev);
FFXSTATUS           FfxLoaderGetDeviceInfo( FFXLOADERDEVHANDLE hDev, FFXLOADERDEVINFO *pDevInfo);
FFXLOADERDISKHANDLE FfxLoaderDiskCreate(    FFXLOADERHANDLE hLdr, unsigned nDiskNum, unsigned nDevNum, D_UINT32 ulFlags);
FFXSTATUS           FfxLoaderDiskDestroy(   FFXLOADERDISKHANDLE hDisk);
FFXSTATUS           FfxLoaderGetDiskInfo(   FFXLOADERDISKHANDLE hDisk, FFXLOADERDISKINFO *pInfo);
unsigned            FfxLoaderGetBlockStatus(FFXLOADERDISKHANDLE hDisk, D_UINT32 ulBlockOffset);
D_UINT32            FfxLoaderEraseBlocks(   FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartBlock, D_UINT32 ulCount);
FFXSTATUS           FfxLoaderLockFreeze(    FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartBlock, D_UINT32 ulCount);
FFXSTATUS           FfxLoaderLockBlocks(    FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartBlock, D_UINT32 ulCount);
FFXSTATUS           FfxLoaderUnlockBlocks(  FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartBlock, D_UINT32 ulCount, D_BOOL fInvert);
D_UINT32            FfxLoaderReadFmlPages(  FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pBuffer, D_BUFFER *pTag, unsigned nTagSize);
D_UINT32            FfxLoaderWriteFmlPages( FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartPage, D_UINT32 ulCount, const D_BUFFER *pBuffer, const D_BUFFER *pTag, unsigned nTagSize);
D_UINT32            FfxLoaderReadTags(      FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartPage, D_UINT32 ulCount, D_BUFFER *pTag, unsigned nTagSize);
D_UINT32            FfxLoaderReadSectors(   FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, D_BUFFER *pBuffer);
D_UINT32            FfxLoaderWriteSectors(  FFXLOADERDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, const D_BUFFER *pBuffer);

#endif  /* FXLOADERAPI_H_INCLUDED */


