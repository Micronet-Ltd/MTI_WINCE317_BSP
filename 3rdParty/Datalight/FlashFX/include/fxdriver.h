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
    for code which is built by the standard FlashFX build process.

    For code which may be built outside the standard FlashFX build process,
    use the header fxdriverfwapi.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxdriver.h $
    Revision 1.26  2010/11/09 14:10:47Z  garyp
    Added FfxDriverDiskCompactAggressive().  Renamed some functions
    for interface consistency.
    Revision 1.25  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.24  2010/01/21 19:31:08Z  garyp
    Updated to support flush.
    Revision 1.23  2010/01/07 23:58:55Z  garyp
    Updated prototypes.
    Revision 1.22  2009/12/04 00:53:34Z  garyp
    Updated the driver I/O stats information.
    Revision 1.21  2009/08/04 02:52:21Z  garyp
    Merged from the v4.0 branch.  Updated the compaction functions to take a
    compaction level.  Added an fIsNAND field to the FFXDISKINFO structure.
    Refactored this header so it contains only internal Driver Framework 
    functionality, for code being built by the standard build process.  
    Other Driver Framework functionality for externally built code is in 
    fxdriverfwapi.h.  Added a field to FFXDEVGEOMETRY to indicate whether
    BBM is loaded or not.  Added support for the power suspend/resume API.
    Revision 1.20  2009/04/08 02:23:00Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.19  2008/03/27 01:33:30Z  Garyp
    Added and modified a number of structures and functions to support more
    flexible DEVICE and DISK creation and management.
    Revision 1.18  2007/11/29 22:16:35Z  Garyp
    Added prototypes.
    Revision 1.17  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.16  2007/08/01 00:10:04Z  timothyj
    Created FfxDriverDiskAdjust() helper to adjust the start and length of a
    disk to align on flash erase block boundaries.  Modified FFXDISKSETTINGS
    to use units of KB instead of bytes.
    Revision 1.15  2007/06/21 23:56:54Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.14  2007/01/10 01:54:11Z  Garyp
    Removed FfxConfigurationInfo() stuff from this header to fxconfig.h.
    Revision 1.13  2006/11/10 18:36:00Z  Garyp
    Prototypes updated.
    Revision 1.12  2006/10/16 20:32:10Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.11  2006/10/13 01:53:16Z  Garyp
    Prototype updates.
    Revision 1.10  2006/07/01 18:57:47Z  Garyp
    Prototype updates.
    Revision 1.9  2006/06/12 11:59:16Z  Garyp
    Added FfxDriverDiskGeometry().
    Revision 1.8  2006/05/08 10:38:10Z  Garyp
    Various prototype updates.
    Revision 1.7  2006/02/20 22:42:16Z  Garyp
    Minor type changes.
    Revision 1.6  2006/02/15 02:59:10Z  Garyp
    Prototype update.
    Revision 1.5  2006/02/12 03:59:27Z  Garyp
    Prototype updates.
    Revision 1.4  2006/02/10 10:16:12Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.3  2006/01/12 04:35:58Z  Garyp
    Minor prototype updates.
    Revision 1.2  2005/12/12 20:50:51Z  garyp
    Added compaction tracking logic.
    Revision 1.1  2005/12/04 21:07:06Z  Pauli
    Initial revision
    Revision 1.3  2005/12/04 21:07:06Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.2  2005/11/06 09:08:45Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/05/03 18:35:26Z  Garyp
    Initial revision
    Revision 1.2  2005/09/18 07:52:41Z  garyp
    Added FfxDriverCompactifIdle().
    Revision 1.1  2005/08/03 04:17:26Z  pauli
    Initial revision
    Revision 1.2  2005/08/03 04:17:26Z  pauli
    Added read ahead caching.
    Revision 1.1  2004/12/30 22:38:04Z  pauli
    Initial revision
    Revision 1.26  2004/12/30 21:38:03Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.25  2004/11/29 18:04:45Z  GaryP
    Added support for FfxDriverGetConfigInfo().
    Revision 1.24  2004/11/20 04:54:21Z  GaryP
    Refactored and moved the internal driver framework structures into
    driverfw.h.
    Revision 1.23  2004/11/09 02:46:07Z  GaryP
    Modified to use the updated background compaction API.
    Revision 1.22  2004/10/29 00:57:49Z  GaryP
    Changed garbage collection terminology to compaction.
    Revision 1.21  2004/10/18 21:26:30Z  garys
    Padded the FFXFORMATSETTINGS structure to 4-byte align members
    Revision 1.20  2004/09/23 03:36:55  GaryP
    Changed to use "compaction" rather than "garbage collection".
    Revision 1.19  2004/09/17 04:16:54Z  GaryP
    New function prototypes.
    Revision 1.18  2004/09/14 02:44:32Z  GaryP
    Removed the fDiskValid field.
    Revision 1.17  2004/09/07 21:37:41Z  GaryP
    Eliminated an obsolete structure field.
    Revision 1.16  2004/08/23 17:04:45Z  GaryP
    Added support for background garbage collection.
    Revision 1.15  2004/08/15 04:16:54Z  GaryP
    Minor function renaming exersize
    Revision 1.14  2004/08/13 19:44:47Z  GaryP
    Updated prototypes for the sector I/O functions.
    Revision 1.13  2004/08/11 19:26:52Z  GaryP
    Added the PFATMONINST opaque type.
    Revision 1.12  2004/08/09 22:06:52Z  GaryP
    Miscellaneous changes to support isoating the file system specific support
    from generic FlashFX functionality.
    Revision 1.11  2004/07/27 15:52:54Z  BillR
    Add an implementation of _sysdelay() that uses a calibrated loop for use
    on platforms that lack a microsecond-resolution time reference,
    Revision 1.10  2004/07/23 23:43:32Z  GaryP
    Added support for specifying a default file system.
    Revision 1.9  2004/07/20 01:36:26Z  GaryP
    Moved the format state definitions to fxmacros.h from fxdriver.h.
    Revision 1.8  2004/07/03 00:50:08Z  GaryP
    Eliminated FfxDriverSetDeviceInfo() and FfxDriverSetDeviceFATInfo() usage.
    Revision 1.7  2004/05/06 00:38:42Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.6  2004/04/27 22:31:37Z  garyp
    Added a prototype for FfxDriverUnitTest().
    Revision 1.5  2003/12/05 07:10:46Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.3  2003/12/05 07:10:46  garyp
    Updated for the revamped driver framework.
    Revision 1.2.1.2  2003/11/05 03:53:44Z  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/11/05 03:53:44Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/07/09 01:35:44Z  garyp
    Added FfxDriverSetDeviceInfo() and FfxDriverSetDeviceFATInfo().
    Revision 1.1  2003/04/15 17:49:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXDRIVER_H_INCLUDED
#define FXDRIVER_H_INCLUDED

#include <fxdriverfwapi.h>
#include <fxdevapi.h>
#include <diskapi.h>
#include <fxstats.h>


#if FFXCONF_FATMONITORSUPPORT
/*-------------------------------------------------------------------
    PFATMONINST contains instance information for the FAT Monitor.
-------------------------------------------------------------------*/
typedef struct tagFATMONINST *PFATMONINST;
#endif

#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
typedef struct tagFFXCOMPACTINFO FFXCOMPACTINFO;
#endif

#if D_DEBUG
  #define FFX_SIGNATURELEN      (4)
  #define FFX_DRIVER_SIGNATURE  "DRVR"
  #define FFX_DEVICE_SIGNATURE  "FDEV"
  #define FFX_DISK_SIGNATURE    "DISK"
#endif


/*-------------------------------------------------------------------
    FFXDEVINFO

    This structure defines the template to be used for a FlashFX
    Device.  This structure embeds all the components needed to
    identify each FlashFX Device.
-------------------------------------------------------------------*/
struct tagFFXDEVINFO
{
  #if D_DEBUG
    char                    acSignature[FFX_SIGNATURELEN];
  #endif
    FFXDRIVERINFO          *pDriverInfo;
    FFXDEVHOOK             *pDevHook;
    FFXFIMDEVHANDLE         hFimDev;
    FFXDEVCONFIG            Conf;               /* Init time configuration settings */
    unsigned                nUsageCount;
};


/*-------------------------------------------------------------------
    FFXDISKINFO

    This structure defines the template to be used for a FlashFX
    Disk.  This structure embeds all the components needed to
    identify each FlashFX Disk.
-------------------------------------------------------------------*/
struct tagFFXDISKINFO
{
  #if D_DEBUG
    char                    acSignature[FFX_SIGNATURELEN];
  #endif
    FFXDRIVERINFO          *pDriverInfo;
    FFXDISKHOOK            *pDiskHook;
    FFXFMLHANDLE            hFML;
  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    VBFHANDLE               hVBF;
  #endif
    FFXDEVINFO             *pDev;
    FFXDISKCONFIG           Conf;               /* Init time configuration settings */
    D_UINT32                ulBlockOffset;      /* Erase block offset in the first Device */
    D_UINT32                ulBlockCount;       /* Total erase blocks in the Disk */
    D_UINT32                ulBlockSize;        /* Bytes per erase block */
    D_UINT32                ulSectorLength;     /* Accommodate <big> sectors */
    D_UINT16                uPageSize;          /* Physical/allocator page size */
    D_UINT16                uSectorToPageShift; /* Shift count to convert sectors to pages */
    FFXPHYSICALDISKINFO     fpdi;
  #if FFXCONF_NANDSUPPORT
    D_BOOL                  fIsNAND;
  #endif
  #if FFXCONF_FATMONITORSUPPORT
    PDCLMUTEX               pFatMonMutex;
    PFATMONINST             pFMI;
  #endif
  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    FFXCOMPACTINFO         *pFCI;
  #endif
  #if FFXCONF_STATS_DRIVERIO
    D_ATOMIC32              ulAtomicReadStatsGate;
    D_ATOMIC32              ulAtomicWriteStatsGate;
    D_ATOMIC32              ulAtomicFlushStatsGate;
    D_ATOMIC32              ulAtomicDiscardStatsGate;
    D_UINT32                ulNextSeqReadSectorNum;
    D_UINT32                ulNextSeqWriteSectorNum;
    D_UINT32                ulNextSeqDiscardSectorNum;
    DRIVERIOSTATS           stats;
  #endif
};


/*-------------------------------------------------------------------
    FFXPFNOPTIONGET

    This function pointer type is used by FfxDriverOptionHook(),
    which allows functions to be hook into the "OptionGet" interface
    at run-time.
-------------------------------------------------------------------*/
typedef D_BOOL(*FFXPFNOPTIONGET)(FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);


/*-------------------------------------------------------------------
    This is the master FlashFX structure used for a device driver
    instance, which embeds information about all DEVICEs and DISKs
    which the driver controls.
-------------------------------------------------------------------*/
struct tagFFXDRIVERINFO
{
  #if D_DEBUG
    char                acSignature[FFX_SIGNATURELEN];
  #endif
    DCLINSTANCEHANDLE   hDclInst;
    PDCLMUTEX           pMutex;     /* mutex protecting the entire driver */
    FFXDEVINFO         *apDev[FFX_MAX_DEVICES];
    FFXDISKINFO        *apDisk[FFX_MAX_DISKS];
    FFXPFNOPTIONGET     pfnOptionGet;
    unsigned            fInFFX : 1;
    unsigned            fLocalDcl : 1;
};


/*-------------------------------------------------------------------
    Prototypes for the Driver Framework functions.
-------------------------------------------------------------------*/
FFXDRIVERINFO  *FfxDriverHandle(unsigned nDriverNum);
FFXDRIVERINFO  *FfxDriverCreate(unsigned nDriverNum, DCLINSTANCEHANDLE hDclInst);
FFXSTATUS       FfxDriverDestroy(               FFXDRIVERINFO *pFDI);
D_BOOL          FfxDriverLock(                  FFXDRIVERINFO *pFDI);
void            FfxDriverUnlock(                FFXDRIVERINFO *pFDI);

FFXDEVHANDLE    FfxDriverDeviceCreate(          FFXDRIVERINFO *pFDI, FFXDEVINITDATA *pDevCreateData);
FFXDEVINFO *    FfxDriverDevicePtr(             FFXDEVHANDLE hDev);
D_BOOL          FfxDriverDeviceHandleValidate(  FFXDEVHANDLE hDev);

#if FFXCONF_LATENCYREDUCTIONENABLED
  FFXSTATUS     FfxDriverDeviceLatencySettings( FFXDEVCONFIG *pConf);
#endif

FFXDISKHANDLE   FfxDriverDiskCreate(            FFXDRIVERINFO *pFDI, FFXDISKINITDATA *pDiskCreateData);
FFXDISKINFO *   FfxDriverDiskPtr(               FFXDISKHANDLE hDisk);
D_BOOL          FfxDriverDiskHandleValidate(    FFXDISKHANDLE hDisk);
D_BOOL          FfxDriverDiskCalcParams(        FFXDISKHANDLE hDisk);

#if FFXCONF_POWERSUSPENDRESUME
  FFXSTATUS     FfxDriverDiskPowerSuspend(      FFXDISKHANDLE hDisk, unsigned nPowerState);
  FFXSTATUS     FfxDriverDiskPowerResume(       FFXDISKHANDLE hDisk, unsigned nPowerState);
#endif

FFXSTATUS       FfxDriverUnitTest(              DCLINSTANCEHANDLE hDclInst, FFXDISKHANDLE hDisk);
DCLSTATUS       FfxDriverUnitTestDCL(           DCLINSTANCEHANDLE hDclInst);
FFXPFNOPTIONGET FfxDriverOptionHook(            FFXPFNOPTIONGET pfnOptionGet);
D_BOOL          FfxDriverOptionGet(             FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);
D_BOOL          FfxDriverFWOptionGet(           FFXOPTION opt, void *handle, void *pBuffer, D_UINT32 ulBuffLen);

#if FFXCONF_FORMATSUPPORT
  FFXSTATUS     FfxDriverDiskFormatSettings(    FFXDISKCONFIG *pConf);
#endif

D_UINT32        FfxDriverSectorRead(            FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, D_UCHAR * pBuff);
D_UINT32        FfxDriverSectorWrite(           FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, const D_BUFFER *pBuff);
D_UINT32        FfxDriverSectorFlush(           FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount);
FFXSTATUS       FfxDriverSectorDiscard(         FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount);

FFXIOSTATUS     FfxDriverFmlBlockErase(         FFXDISKHANDLE hDisk, D_UINT32 ulStartBlock, D_UINT32 ulCount);
FFXIOSTATUS     FfxDriverFmlSectorRead(         FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, D_BUFFER *pBuffer);
FFXIOSTATUS     FfxDriverFmlSectorWrite(        FFXDISKHANDLE hDisk, D_UINT32 ulStartSector, D_UINT32 ulCount, const D_BUFFER *pBuffer, D_BUFFER *pBlockBuff);

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
  FFXSTATUS     FfxDriverDiskAllocatorSettings( FFXDISKCONFIG *pConf);
  FFXIOSTATUS   FfxDriverDiskCompactAggressive( FFXDISKHANDLE hDisk);
  FFXSTATUS     FfxDriverDiskWipe(              FFXDISKHANDLE hDisk);
#if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
  D_UINT32      FfxDriverDiskCompactIfIdle(     FFXDISKHANDLE hDisk, D_UINT32 ulFlags);
#endif
#if FFXCONF_FORMATSUPPORT
  FFXSTATUS     FfxDriverAllocatorFormat(       FFXDISKHANDLE hDisk, unsigned nAllocator);
  FFXSTATUS     FfxDriverFormatFileSystem(      FFXDISKHANDLE hDisk, FFXFORMATSETTINGS *pFS);
#endif
#endif


#if FFXCONF_MBRFORMAT
  D_BOOL        FfxDriverWriteMBR(              FFXDISKHANDLE hDisk, D_UINT32 ulMediaSize, D_UINT32 *pulHiddenSecs, D_UINT16 *puSides, D_UINT16 *puSPT, D_UINT16 *puCylinders);
#endif

#if FFXCONF_STATS_COMPACTION
  D_BOOL        FfxDriverDiskCompactStatsQuery( FFXDISKINFO *pDisk, FFXCOMPSTATS *pStats, D_BOOL fReset);
  D_BOOL        FfxDriverDiskCompactStatsReset( FFXDISKINFO *pDisk);
#endif

#if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS
  FFXSTATUS     FfxDriverDiskCompSettings(      FFXDISKCONFIG *pConf);
  FFXSTATUS     FfxDriverDiskCompactSuspend(    FFXDISKINFO *pDisk);
  FFXSTATUS     FfxDriverDiskCompactResume(     FFXDISKINFO *pDisk);
#endif

#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
  FFXSTATUS     FfxDriverDiskCompactCreate(     FFXDISKHANDLE hDisk);
  FFXSTATUS     FfxDriverDiskCompactDestroy(    FFXDISKINFO *pDisk, D_UINT32 ulTimeout);
#endif


#endif /* FXDRIVER_H_INCLUDED */


