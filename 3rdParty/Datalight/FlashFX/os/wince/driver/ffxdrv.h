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

    This header contains information that is private to the FlashFX driver
    for Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxdrv.h $
    Revision 1.12  2010/09/22 02:51:19Z  garyp
    Enhanced the startup and shutdown logic to provide more useful
    error information.
    Revision 1.11  2010/04/19 19:18:31Z  garyp
    Updated so the QUEUE_PENDING_DISCARDS feature is configurable at
    compile time.  Added profiler instrumentation.
    Revision 1.10  2010/01/23 21:40:46Z  garyp
    Updated to support flush.
    Revision 1.9  2009/07/22 00:52:33Z  garyp
    Merged from the v4.0 branch.  Prototypes updated.
    Revision 1.8  2008/03/26 02:40:48Z  Garyp
    Prototype updates.
    Revision 1.7  2007/11/03 23:50:10Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/04/02 16:18:21Z  keithg
    FSINTTEST has been removed from FFX, it is now part of DCL.
    Revision 1.5  2007/03/04 02:54:49Z  Garyp
    Removed CE 3.0 support.  Added CE 6.0 support.
    Revision 1.4  2007/01/23 23:30:31Z  Garyp
    Added FSIntTest support.
    Revision 1.3  2006/02/13 10:37:15Z  Garyp
    Updated to new external API requestor interface.
    Revision 1.2  2006/02/07 04:40:17Z  Garyp
    Updated to use the new DeviceNum/DiskNum concepts.
    Revision 1.1  2005/04/03 02:40:52Z  Pauli
    Initial revision
    Revision 1.15  2005/04/02 23:40:52Z  GaryP
    Minor documentation and code formatting cleanup.
    Revision 1.14  2004/12/30 17:31:19Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.13  2004/09/29 02:28:31Z  GaryP
    Removed some obsolete stuff.
    Revision 1.12  2004/08/06 22:28:11Z  GaryP
    Moved the root dir entries count to ffxwce.h.
    Revision 1.11  2004/07/20 00:42:11Z  GaryP
    Include fxtools.h rather than fxtests.h.
    Revision 1.10  2004/06/03 00:04:04Z  jaredw
    Added FMLHANDLE parameter to FfxPerformUnitTests to fix problem with needed
    struct not defined.  This allows running of unit tests from the driver.
    Revision 1.9  2003/12/05 06:53:48  garys
    Merge from FlashFXMT
    Revision 1.8.1.3  2003/12/05 06:53:48  garyp
    Updated for newly modularized driver.
    Revision 1.8.1.2  2003/11/22 04:29:50Z  garyp
    Updated for new external API interface.
    Revision 1.8  2003/06/05 17:47:22Z  garyp
    Renamed FFX_UNIT_TEST to FFXCONF_DRIVERAUTOTEST.
    Revision 1.7  2003/05/29 17:48:06Z  garyp
    Updated to use VBF_MAX_TRANSFER_SIZE.
    Revision 1.6  2003/05/29 02:01:40Z  garyp
    Fixed to build right under CE3
    Revision 1.5  2003/04/21 11:05:42Z  garyp
    Updated to cache pending discards.
    Revision 1.4  2003/04/15 17:49:14Z  garyp
    Updated to use the new Driver Framework.
    Revision 1.3  2003/02/17 10:55:06Z  garyp
    Prototype tweaks.
    Revision 1.2  2003/02/10 23:05:24Z  garyp
    Moved general stuff from FFXDRV.C here.
---------------------------------------------------------------------------*/


#define QUEUE_PENDING_DISCARDS      TRUE    /*  Must be TRUE for checkin */

 
/*-------------------------------------------------------------------
    For compatibility with WinCE v2.0 define GETNAME if it's not
    already.  CE v2.10 FATFS makes this call to allow the driver
    to override the Folder value in the registry.
-------------------------------------------------------------------*/
#ifndef DISK_IOCTL_GETNAME
#define DISK_IOCTL_GETNAME 9
#endif


/*-------------------------------------------------------------------
    If these are not defined, we are likely running on CE 2.0.
    Define them here so to keep the remainder of the code clean.
-------------------------------------------------------------------*/
#ifndef IOCTL_DISK_READ
#define IOCTL_DISK_READ         DISK_IOCTL_READ
#define IOCTL_DISK_WRITE        DISK_IOCTL_WRITE
#define IOCTL_DISK_GETNAME      DISK_IOCTL_GETNAME
#define IOCTL_DISK_FORMAT_MEDIA DISK_IOCTL_FORMAT_MEDIA
#endif


/*-------------------------------------------------------------------
                              Prototypes
-------------------------------------------------------------------*/
D_BOOL          FfxCeCreateDevices( FFXDRIVERINFO *pFDI);
FFXSTATUS       FfxCeDestroyDevices(FFXDRIVERINFO *pFDI);
FFXSTATUS       FfxCeDriverInit(    PDRIVERDATA pDD);
FFXSTATUS       FfxCeDriverDeinit(  PDRIVERDATA pDD);
FFXDISKHANDLE   FfxCeDiskCreate(    PDRIVERDATA pDD, LPCTSTR lptzActiveKey);
FFXSTATUS       FfxCeDiskDestroy(          FFXDISKHANDLE hDisk);
D_BOOL          FfxCeDispatchIoctl(        FFXDISKHANDLE hDisk, FFXIOREQUEST *pReqHdr, DWORD dwLen);
DWORD           FfxCeIoctlReadWrite(       FFXDISKHANDLE hDisk, DWORD dwCode, PSG_REQ pSgr);
FFXSTATUS       FfxCeIoctlFlush(           FFXDISKHANDLE hDisk);
FFXDISKHANDLE   FfxCeAdjustIODiskAndOffset(FFXDISKHANDLE hDisk, D_UINT32 *pulSector);
void *          FfxCeMapPtr(void *ptr);
void            FfxCeUnMapPtr(void *ptr);


#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT && QUEUE_PENDING_DISCARDS
void            FfxCeFlushPendingDiscards(FFXDISKHANDLE hDisk);
#endif






/*---------------------------------------------------------
    File pruned by DL-Prune v1.04

    Pruned/grafted 1/0 instances containing 3 lines.
    Modified 0 lines.
---------------------------------------------------------*/
