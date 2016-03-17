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
    Revision 1.5  2011/04/27 16:58:33Z  johnb
    Added Windws Embedded Compact 7 support
    Added conditional code for symbols deprecated in WEC7.
    Revision 1.4  2010/07/30 16:55:26Z  garyp
    Updated to use dlosblockdev.h.  Now support Flush().  Updated to
    get the specific device type.
    Revision 1.3  2010/07/17 20:30:11Z  garyp
    Updated a debug message -- no functional changes.
    Revision 1.2  2009/09/16 19:26:49Z  garyp
    Documentation fixes only -- no functional changes.
    Revision 1.1  2009/09/14 22:35:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <diskio.h>
#include <storemgr.h>

#include <dcl.h>
#include <dlosblockdev.h>
#include <dlapiprv.h>


typedef struct sDCLOSBLOCKDEVCONTROL
{
    HANDLE              hDev;
    DISK_INFO           DiskInfo;
    DCLOSBLOCKDEVACCESS nMode;
} DCLOSBLOCKDEVCONTROL;

static DCLIOSTATUS SectorRead(   DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, D_BUFFER *pBuffer);
static DCLIOSTATUS SectorWrite(  DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, const D_BUFFER *pBuffer);
static DCLIOSTATUS SectorDiscard(DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount);
static DCLIOSTATUS SectorFlush(  DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount);


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
    #define                 MAX_DEV_NAME_LEN    (256)
    DCLOSBLOCKDEVCONTROL   *pDevCtrl;
    DWORD                   dwAccess;
    DWORD                   dwUnused;
    D_WCHAR                 wzDevName[MAX_DEV_NAME_LEN];
    STORAGEDEVICEINFO       sdi = {sizeof(STORAGEDEVICEINFO)};

    DclAssertReadPtr(pszDevName, 0);
    DclAssertWritePtr(phDev, sizeof(*phDev));
    DclAssertWritePtr(pDevInfo, sizeof(*pDevInfo));

    /*  ToDo: Figure out of the DCLOSBLOCKDEVFLAG_CACHED flag has a
              good use in this OS abstraction.
    */

    /*  Determine the proper Win32 access mode...
    */
    switch(nMode)
    {
        case DCLOSBLOCKDEVACCESS_READ:
            dwAccess = GENERIC_READ;
            break;
        case DCLOSBLOCKDEVACCESS_WRITE:
            dwAccess = GENERIC_WRITE;
            break;
        case DCLOSBLOCKDEVACCESS_READWRITE:
            dwAccess = GENERIC_READ|GENERIC_WRITE;
            break;
        default:
            DclError();
            return DCLSTAT_BLOCKDEV_BADACCESS;
    }

    if(!DclOsAnsiToWcs(wzDevName, sizeof(wzDevName), pszDevName, -1))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDevCreate() error converting device name, error %lU\n", dwError));
        return DclOsErrToDclStatus(dwError);
    }

    pDevCtrl = DclMemAllocZero(sizeof(*pDevCtrl));
    if(!pDevCtrl)
         return DCLSTAT_MEMALLOCFAILED;

    pDevCtrl->nMode = nMode;

    /*  FILE_SHARE_WRITE is necessary when opening volumes, but not
        physical disks.  ToDo: examine the device name and only set
        that flag when it is necessary.
    */
    pDevCtrl->hDev = CreateFile(wzDevName, dwAccess, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(pDevCtrl->hDev == INVALID_HANDLE_VALUE)
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDevCreate() CreateFile(\"%W\") failed with error %lU\n", wzDevName, dwError));

        DclMemFree(pDevCtrl);

        return DclOsErrToDclStatus(dwError);
    }

    if(!DeviceIoControl(pDevCtrl->hDev, IOCTL_DISK_GETINFO, NULL, 0,
                        &pDevCtrl->DiskInfo, sizeof(pDevCtrl->DiskInfo), &dwUnused, NULL))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDevCreate() DeviceIoControl(IOCTL_DISK_GETINFO) failed with error %lU\n", dwError));

        DclOsBlockDevDestroy(pDevCtrl);

        return DclOsErrToDclStatus(dwError);
    }

    DclMemSet(pDevInfo, 0, sizeof(*pDevInfo));

    pDevInfo->ulBytesPerSector = pDevCtrl->DiskInfo.di_bytes_per_sect;
    pDevInfo->ullTotalSectors  = pDevCtrl->DiskInfo.di_total_sectors;
    pDevInfo->pfnRead = SectorRead;
    pDevInfo->pfnWrite = SectorWrite;
    pDevInfo->pfnDiscard = SectorDiscard;
    pDevInfo->pfnFlush = SectorFlush;

    if(!DeviceIoControl(pDevCtrl->hDev, IOCTL_DISK_DEVICE_INFO, NULL, 0,
                        &sdi, sizeof(sdi), &dwUnused, NULL))
    {
        DCLPRINTF(1, ("DclOsBlockDevCreate() DeviceIoControl(IOCTL_DISK_DEVICE_INFO) failed with error %lU\n", GetLastError()));

        pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_GENERIC;
    }
    else
    {
        switch(sdi.dwDeviceType)
        {
            case STORAGE_DEVICE_TYPE_ATA:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_ATA;
                break;

            case STORAGE_DEVICE_TYPE_ATAPI:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_ATAPI;
                break;

            case STORAGE_DEVICE_TYPE_CDROM:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_CDROM;
                break;

#if _WIN32_WCE < 700
            case STORAGE_DEVICE_TYPE_CFCARD:
#else
            case STORAGE_DEVICE_TYPE_DEPRECATED1:
#endif
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_COMPACTFLASH;
                break;

            case STORAGE_DEVICE_TYPE_DOC:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_DISKONCHIP;
                break;

            case STORAGE_DEVICE_TYPE_DVD:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_DVD;
                break;

            case STORAGE_DEVICE_TYPE_FLASH:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_FLASH;
                break;

#if _WIN32_WCE < 700
            case STORAGE_DEVICE_TYPE_PCCARD:
#else
            case STORAGE_DEVICE_TYPE_DEPRECATED2:
#endif
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_PCCARD;
                break;

            case STORAGE_DEVICE_TYPE_PCIIDE:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_PCIIDE;
                break;

            case STORAGE_DEVICE_TYPE_USB:
                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_USB;
                break;

            default:
                DCLPRINTF(1, ("DclOsBlockDevCreate() Unhandled device type %lU, using DCLBLOCKDEVTYPE_GENERIC\n"));

                pDevInfo->nDeviceType = DCLBLOCKDEVTYPE_GENERIC;
                break;
        }
    }
    
    *phDev = pDevCtrl;

    return DCLSTAT_SUCCESS;
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
    DclAssertWritePtr(hDev, sizeof(*hDev));
    DclAssert(hDev->hDev);

    if(!CloseHandle(hDev->hDev))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDevDestroy() CloseHandle() failed with error %lU\n", dwError));

        return DclOsErrToDclStatus(dwError);
    }

    return DclMemFree(hDev);
}


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
    DCLIOSTATUS             ioStat = {0};
    SG_REQ                  sg = {0};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
    DclAssertWritePtr(pBuffer, ulCount * hDev->DiskInfo.di_bytes_per_sect);
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_READ ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    sg.sr_start             = (D_UINT32)ullStart;
    sg.sr_num_sec           = ulCount;
    sg.sr_num_sg            = 1;
    sg.sr_sglist[0].sb_buf  = pBuffer;
    sg.sr_sglist[0].sb_len  = ulCount * hDev->DiskInfo.di_bytes_per_sect;

    if(!DeviceIoControl(hDev->hDev, IOCTL_DISK_READ, &sg, sizeof(sg), 0, 0, NULL, 0))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDev:SectorRead() DeviceIoControl() failed with error %lU\n", dwError));

        ioStat.dclStat = DclOsErrToDclStatus(dwError);
    }
    else
    {
        if(sg.sr_status == ERROR_SUCCESS)
            ioStat.ulCount = ulCount;

        ioStat.dclStat = DclOsErrToDclStatus(sg.sr_status);
    }

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
    DCLIOSTATUS             ioStat = {0};
    SG_REQ                  sg = {0};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
    DclAssertReadPtr(pBuffer, ulCount * hDev->DiskInfo.di_bytes_per_sect);
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_WRITE ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    sg.sr_start             = (D_UINT32)ullStart;
    sg.sr_num_sec           = ulCount;
    sg.sr_num_sg            = 1;
    sg.sr_sglist[0].sb_buf  = (PUCHAR)pBuffer;
    sg.sr_sglist[0].sb_len  = ulCount * hDev->DiskInfo.di_bytes_per_sect;

    if(!DeviceIoControl(hDev->hDev, IOCTL_DISK_WRITE, &sg, sizeof(sg), 0, 0, NULL, 0))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDev:SectorWrite() DeviceIoControl() failed with error %lU\n", dwError));

        ioStat.dclStat = DclOsErrToDclStatus(dwError);
    }
    else
    {
        if(sg.sr_status == ERROR_SUCCESS)
            ioStat.ulCount = ulCount;

        ioStat.dclStat = DclOsErrToDclStatus(sg.sr_status);
    }

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
    DCLIOSTATUS             ioStat = {0};
    DELETE_SECTOR_INFO      dsi = {sizeof(dsi)};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_WRITE ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    dsi.startsector = (D_UINT32)ullStart;
    dsi.numsectors = ulCount;

    if(!DeviceIoControl(hDev->hDev, IOCTL_DISK_DELETE_SECTORS, &dsi, sizeof(dsi), 0, 0, NULL, 0))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDev:SectorDiscard() DeviceIoControl() failed with error %lU\n", dwError));

        ioStat.dclStat = DclOsErrToDclStatus(dwError);
    }
    else
    {
        ioStat.ulCount = ulCount;
        ioStat.dclStat = DCLSTAT_SUCCESS;
    }

    return ioStat;
}


/*-------------------------------------------------------------------
    Local: SectorFlush()

    Flush a range of sectors on a physical block device.

    Parameters:
        hDev     - The handle of the device to use.
        ullStart - The starting sector number, relative to zero.
        ulCount  - The number of sectors to flush.

    Return Value:
        Returns a DCLIOSTATUS code indicating the result.
-------------------------------------------------------------------*/
static DCLIOSTATUS SectorFlush(
    DCLOSBLOCKDEVHANDLE     hDev,
    D_UINT64                ullStart,
    D_UINT32                ulCount)
{
    DCLIOSTATUS             ioStat = {0};

    DclAssert(hDev);
    DclAssert(ulCount);
    DclAssert(ullStart <= D_UINT32_MAX);
    DclAssert(hDev->nMode == DCLOSBLOCKDEVACCESS_WRITE ||
              hDev->nMode == DCLOSBLOCKDEVACCESS_READWRITE);

    if(!DeviceIoControl(hDev->hDev, IOCTL_DISK_FLUSH_CACHE, NULL, 0, 0, 0, NULL, 0))
    {
        DWORD   dwError = GetLastError();

        DCLPRINTF(1, ("DclOsBlockDev:SectorFlush() DeviceIoControl() failed with error %lU\n", dwError));

        ioStat.dclStat = DclOsErrToDclStatus(dwError);
    }
    else
    {
        /*  Since this OS interface does not support flushing a range of
            sectors, return a count of D_UINT32_MAX, which the higher 
            level software recognizes to mean "everything".
        */            
        ioStat.ulCount = D_UINT32_MAX;
        ioStat.dclStat = DCLSTAT_SUCCESS;
    }

    return ioStat;
}



