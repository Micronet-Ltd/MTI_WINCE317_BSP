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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This is the master header file for the FlashFX Device Driver Framework
    for code which may be built outside the standard FlashFX build process.

    For code which must be built using the standard FlashFX build process,
    use the header fxdriver.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdriverfwapi.h $
    Revision 1.5  2010/11/17 22:08:01Z  glenns
    Added option flags to the allocator settings.
    Revision 1.4  2010/03/19 22:41:13Z  billr
    Update license, this is a shared header.
    Revision 1.3  2009/07/31 23:35:28Z  garyp
    Re-added the fAlwaysMount flag to the FFXBBMSETTINGS structure.
    Revision 1.2  2009/03/08 16:36:43Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.4  2009/03/08 16:36:43Z  garyp
    Changed the shutdown mode from being an enumerated type to being
    a bitmapped flags value.
    Revision 1.1.1.3  2009/03/05 05:14:41Z  garyp
    Modified the shutdown processes to take a mode parameter.
    Revision 1.1.1.2  2009/02/18 04:27:44Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1  2009/01/09 20:37:46Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXDRIVERFWAPI_H_INCLUDED
#define FXDRIVERFWAPI_H_INCLUDED


/*-------------------------------------------------------------------
    The following values define the various allocators which may
    be used in the system.
-------------------------------------------------------------------*/
#define FFX_ALLOC_NONE                          (0)
#define FFX_ALLOC_VBF                           (1)


/*-------------------------------------------------------------------
    Type: FFXDEVSETTINGS

    Basic configuration settings for a FlashFX Device.

    pBaseFlashAddress may be BAD_ADDRESS (0xFFFFFFFF) if the flash
    is not memory mapped, such as is the case with NAND, or when
    using the various RAM or FILE based FIMs/NTMs.
-------------------------------------------------------------------*/
typedef struct sFFXDEVSETTINGS
{
    void       *pBaseFlashAddress;      /* Uncached base flash address */
    D_UINT32    ulReservedBottomKB;     /* Reserved from the base address */
    D_UINT32    ulReservedTopKB;        /* Reserved at the top of the array */
    D_UINT32    ulMaxArraySizeKB;       /* Max device size in KB, excluding reserved space */
} FFXDEVSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXDEVTIMING

    Timing characteristics for a FlashFX Device.
-------------------------------------------------------------------*/
typedef struct tagFFXDEVTIMING
{
    D_UINT32    ulPageReadUS;           /* US to read one page  */
    D_UINT32    ulPageWriteUS;          /* US to write one page */
    D_UINT32    ulBlockEraseUS;         /* US to erase one block */
} FFXDEVTIMING;


/*-------------------------------------------------------------------
    Type: FFXDEVLATENCY

    Latency configuration characteristics for a FlashFX Device.
-------------------------------------------------------------------*/
typedef struct tagFFXDEVLATENCY
{
    long        lReadLatencyUS;         /* Max us to spend per chunk read    */
    long        lWriteLatencyUS;        /* Max us to spend per chunk written */
    long        lEraseLatencyUS;        /* Max us to spend per chunk erased  */
    long        lErasePollInterval;     /* US to spend for each erase poll   */
} FFXDEVLATENCY;


/*-------------------------------------------------------------------
    Type: FFXBBMSETTINGS

    BBM settings for a FlashFX Device.
-------------------------------------------------------------------*/
typedef struct tagFFXBBMSETTINGS
{
    unsigned    fEnableBBM;
    unsigned    nFormatState;
    unsigned    nFormatType;
    unsigned    nSparePercent;
    unsigned    fAlwaysMount;
} FFXBBMSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXDEVCONFIG

    The master Device configuration structure.

    This structure contains embedded FFXDEVSETTINGS, FFXDEVTIMING,
    FFXDEVLATENCY, and FFXBBMSETTINGS structures.
-------------------------------------------------------------------*/
struct tagFFXDEVCONFIG
{
    unsigned                nDevNum;
    FFXDEVSETTINGS          DevSettings;
    FFXDEVTIMING            DevTiming;
    FFXDEVLATENCY           DevLatency;
    FFXBBMSETTINGS          BbmSettings;
    unsigned                fInitedDevSettings : 1;
    unsigned                fInitedBbmSettings : 1;
    unsigned                fInitedLatencySettings : 1;
};


/*-------------------------------------------------------------------
    Type: FFXDISKSETTINGS

    Basic configuration settings for a FlashFX Disk.

    This structure is used to store the initialization time disk
    configuration settings.  The actual Disk offset and size may
    be adjusted fall within Device constraints (Device size and
    erase block boundaries).
-------------------------------------------------------------------*/
typedef struct
{
    unsigned    nDevNum;
    D_UINT32    ulOffsetKB;
    D_UINT32    ulLengthKB;
    unsigned    nFlags;
} FFXDISKSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXFORMATSETTINGS

    Format settings for a FlashFX Disk.

    This structure is used during device driver initialization
    to specify how the Disk is to be formatted.
-------------------------------------------------------------------*/
typedef struct
{
    D_UINT16        uFormatState;
    D_UINT16        uFileSystem;
    D_BOOL          fUseMBR;
    D_UINT16        uReserved;
} FFXFORMATSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXALLOCSETTINGS

    Allocator configuration settings for FlashFX Disk.

    This structure is used during device driver initialization
    to specify the allocator settings for a Disk.
-------------------------------------------------------------------*/
typedef struct tagFFXALLOCSETTINGS
{
    unsigned    nAllocator;
    D_UINT32    ulOptionFlags;
} FFXALLOCSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXCOMPSETTINGS

    Compaction configuration settings for a FlashFX Disk.

    This structure is used during device driver initialization
    to specify the compaction settings for a Disk.
-------------------------------------------------------------------*/
typedef struct tagFFXCOMPSETTINGS
{
    D_UINT32    ulReadIdleMS;
    D_UINT32    ulWriteIdleMS;
    D_UINT32    ulSleepMS;
    D_UINT16    uThreadPriority;
    D_UINT16    uPadding;
} FFXCOMPSETTINGS;


/*-------------------------------------------------------------------
    Type: FFXDISKCONFIG

    The master Device configuration structure.

    This structure contains embedded FFXDISKSETTINGS, FFXALLOCSETTINGS,
    FFXFORMATSETTINGS, and FFXCOMPSETTINGS structures.
-------------------------------------------------------------------*/
struct tagFFXDISKCONFIG
{
    unsigned                nDiskNum;
    FFXDISKSETTINGS         DiskSettings;
    FFXALLOCSETTINGS        AllocSettings;
    FFXFORMATSETTINGS       FormatSettings;
    FFXCOMPSETTINGS         CompSettings;
    unsigned                fInitedDiskSettings : 1;
    unsigned                fInitedFormatSettings : 1;
    unsigned                fInitedAllocatorSettings : 1;
    unsigned                fInitedCompactionSettings : 1;
};


/*-------------------------------------------------------------------
    Type: FFXDEVGEOMETRY

    FlashFX Device geometry settings as returned by
    FfxDriverDeviceGeometry().
-------------------------------------------------------------------*/
typedef struct
{
    unsigned    nDevNum;            /* DEVn                                         */
    unsigned    nDevType;           /* Device type code prefixed with DEVTYPE_*     */
    D_UINT32    ulReservedBlocks;   /* Blocks reserved (inaccessible)               */
    D_UINT32    ulTotalBlocks;      /* Total usable blocks in the array             */
    D_UINT32    ulBlockSize;        /* Physical erase block size                    */
    unsigned    nPageSize;          /* Device page size                             */
    unsigned    fBBMLoaded;         /* Flag indicating whether BBM is being used    */
} FFXDEVGEOMETRY;


/*-------------------------------------------------------------------
    Type: FFXDISKGEOMETRY

    FlashFX Disk geometry settings as returned by
    FfxDriverDiskGeometry().
-------------------------------------------------------------------*/
typedef struct
{
    unsigned    nDiskNum;
    unsigned    nDevNum;
    D_UINT32    ulBlockOffset;          /* Block offset within DEVn */
    D_UINT32    ulBlockCount;           /* Number of blocks within DEVn */
    D_UINT32    ulBlockSize;            /* Number of bytes in a block */
    D_UINT32    ulLogicalSectorCount;
    unsigned    nBytesPerSector;
    unsigned    nSectorsPerTrack;
    unsigned    nHeads;
    unsigned    nCylinders;
    unsigned    fAllocatorLoaded : 1;
    unsigned    fCHSValid : 1;
} FFXDISKGEOMETRY;


/*-------------------------------------------------------------------
    Prototypes for the Driver Framework functions.
-------------------------------------------------------------------*/

FFXDEVHANDLE    FfxDriverDeviceEnumerate(   FFXDRIVERINFO *pFDI, FFXDEVHANDLE hDev);
FFXDEVHANDLE    FfxDriverDeviceHandle(      FFXDRIVERINFO *pFDI, unsigned nDevNum);
FFXDEVHANDLE    FfxDriverDeviceCreateParam( FFXDRIVERINFO *pFDI, FFXDEVCONFIG *pConf, FFXDEVHOOK *pHook);
FFXSTATUS       FfxDriverDeviceDestroy(     FFXDEVHANDLE hDev);
FFXSTATUS       FfxDriverDeviceGeometry(    FFXDEVHANDLE hDev, FFXDEVGEOMETRY *pGeometry);
FFXSTATUS       FfxDriverDeviceSettings(    FFXDEVCONFIG *pConf);
FFXSTATUS       FfxDriverDeviceBbmSettings( FFXDEVCONFIG *pConf);

FFXDISKHANDLE   FfxDriverDiskEnumerate(     FFXDRIVERINFO *pFDI, FFXDISKHANDLE hDisk);
FFXDISKHANDLE   FfxDriverDiskHandle(        FFXDRIVERINFO *pFDI, unsigned nDiskNum);
FFXDISKHANDLE   FfxDriverDiskCreateParam(   FFXDRIVERINFO *pFDI, FFXDISKCONFIG *pConf, FFXDISKHOOK *pHook);
FFXSTATUS       FfxDriverDiskDestroy(       FFXDISKHANDLE hDisk);
FFXSTATUS       FfxDriverDiskGeometry(      FFXDISKHANDLE hDisk, FFXDISKGEOMETRY *pGeometry);
FFXSTATUS       FfxDriverDiskParameterGet(  FFXDISKHANDLE hDisk, FFXPARAM id, void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS       FfxDriverDiskParameterSet(  FFXDISKHANDLE hDisk, FFXPARAM id, const void *pBuffer, D_UINT32 ulBuffLen);
FFXSTATUS       FfxDriverDiskSettings(      FFXDISKCONFIG *pConf);

FFXSTATUS       FfxDriverAllocatorCreate(   FFXDISKHANDLE hDisk);
FFXSTATUS       FfxDriverAllocatorDestroy(  FFXDISKHANDLE hDisk, D_UINT32 ulFlags);


#endif /* FXDRIVERFWAPI_H_INCLUDED */


