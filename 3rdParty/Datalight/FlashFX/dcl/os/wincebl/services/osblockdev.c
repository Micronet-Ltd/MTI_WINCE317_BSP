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
  jurisdictions.  Patents may be pending.

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

    This module contains the OS Services abstraction for interfacing with
    a block device.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the project's dclproj.mak to include any necessary dependencies.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osblockdev.c $
    Revision 1.2  2010/08/04 01:19:29Z  garyp
    Updated to use dlosblockdev.h.
    Revision 1.1  2009/09/16 19:28:16Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlosblockdev.h>
#include <dlapiprv.h>


typedef struct sDCLOSBLOCKDEVCONTROL
{
/*
    HANDLE              hDev;
    DISK_INFO           DiskInfo;
*/
    DCLOSBLOCKDEVACCESS nMode;
} DCLOSBLOCKDEVCONTROL;

/*
static DCLIOSTATUS SectorRead(   DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, D_BUFFER *pBuffer);
static DCLIOSTATUS SectorWrite(  DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, const D_BUFFER *pBuffer);
static DCLIOSTATUS SectorDiscard(DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount);
*/


/*-------------------------------------------------------------------
    Public: DclOsBlockDevCreate()

    Create an instance of a physical block device.

    Parameters:
        pszDevName - A pointer to the block device name to use.
        nMode      - The access mode for which to open the device.
        ulFlags    - The open flags, which may be one or more of the
                     following:
                     DCLOSBLOCKDEVFLAG_CACHED - Open the device in
                                                cached mode.
        phDev      - Populated with a DCLOSBLOCKDEVHANDLE used to
                     access the device.
        pDevInfo   - A pointer to the DCLOSBLOCKDEVINFO structure
                     to fill.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsBlockDevCreate(
    const char             *pszDevName,
    DCLOSBLOCKDEVACCESS     nMode,
    D_UINT32                ulFlags,
    DCLOSBLOCKDEVHANDLE    *phDev,
    DCLOSBLOCKDEVINFO      *pDevInfo)
{
    DclAssertReadPtr(pszDevName, 0);
    DclAssertWritePtr(phDev, sizeof(*phDev));
    DclAssertWritePtr(pDevInfo, sizeof(*pDevInfo));

    DclProductionError();

    return DCLSTAT_UNSUPPORTEDFEATURE;
}


/*-------------------------------------------------------------------
    Public: DclOsBlockDevDestroy()

    Destroy a physical block device instance.

    Parameters:
        hDev   - The handle of the device to close.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsBlockDevDestroy(
    DCLOSBLOCKDEVHANDLE   hDev)
{
    DclAssert(hDev);

    DclProductionError();

    return DCLSTAT_UNSUPPORTEDFEATURE;
}


#if 0

                 ...Templates for the I/O function...


/*-------------------------------------------------------------------
    Local: SectorRead()

    Read sectors from a physical block device.

    Parameters:
        hDev     - The handle of the device to use.
        ullStart - The starting sector number, relative to zero.
        ulCount  - The number of sectors to read.
        pBuffer  - The buffer into which to read the sector data.

    Return Value:
        Returns a DCLIOSTATUS code indicating the result.
-------------------------------------------------------------------*/
static DCLIOSTATUS SectorRead(
    DCLOSBLOCKDEVHANDLE     hDev,
    D_UINT64                ullStart,
    D_UINT32                ulCount,
    D_BUFFER               *pBuffer)
{
    DCLIOSTATUS             ioStat = {0, DCLSTAT_UNSUPPORTEDFEATURE};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
/*  DclAssertWritePtr(pBuffer, ulCount * hDev->DiskInfo.di_bytes_per_sect); */
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_READ ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    DclProductionError();

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: SectorWrite()

    Write sectors to a physical block device.

    Parameters:
        hDev     - The handle of the device to use.
        ullStart - The starting sector number, relative to zero.
        ulCount  - The number of sectors to write.
        pBuffer  - The buffer from which to write the sector data.

    Return Value:
        Returns a DCLIOSTATUS code indicating the result.
-------------------------------------------------------------------*/
static DCLIOSTATUS SectorWrite(
    DCLOSBLOCKDEVHANDLE     hDev,
    D_UINT64                ullStart,
    D_UINT32                ulCount,
    const D_BUFFER         *pBuffer)
{
    DCLIOSTATUS             ioStat = {0, DCLSTAT_UNSUPPORTEDFEATURE};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
/*  DclAssertReadPtr(pBuffer, ulCount * hDev->DiskInfo.di_bytes_per_sect); */
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_WRITE ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    DclProductionError();

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: SectorDiscard()

    Discard sectors on a physical block device.

    Parameters:
        hDev     - The handle of the device to use.
        ullStart - The starting sector number, relative to zero.
        ulCount  - The number of sectors to discard.

    Return Value:
        Returns a DCLIOSTATUS code indicating the result.
-------------------------------------------------------------------*/
static DCLIOSTATUS SectorDiscard(
    DCLOSBLOCKDEVHANDLE     hDev,
    D_UINT64                ullStart,
    D_UINT32                ulCount)
{
    DCLIOSTATUS             ioStat = {0, DCLSTAT_UNSUPPORTEDFEATURE};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_WRITE ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    DclProductionError();

    return ioStat;
}


#endif

