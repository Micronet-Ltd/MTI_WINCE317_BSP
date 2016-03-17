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

    This module contains the default OS Services implementations for the file
    system management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfile.c $
    Revision 1.22  2009/06/08 16:27:22Z  garyp
    Updated debug messages -- no functional changes.
    Revision 1.21  2009/04/10 17:46:36Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.20  2009/02/08 01:39:21Z  garyp
    Merged from the v4.0 branch.  Corrected a sizeof() on a wide-character
    array to use DCLDIMENSIONOF().  Updated GetDeviceInfo() to use
    CeGetVolumeInfo() where possible.  Changed a bunch of production
    asserts to regular asserts.
    Revision 1.19  2008/12/11 22:10:44Z  jeremys
    Fixed a bug where a byte count was being treated as a character
    count for a UNICODE string.
    Revision 1.18  2008/04/17 18:23:29Z  brandont
    Updated to use the new file system services.
    Revision 1.17  2008/04/08 02:33:28Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.16  2008/04/04 00:17:20Z  brandont
    Changed max path length and max name length members of the DCLFSSTATFS
    structure to be of type unsigned.
    Revision 1.15  2008/04/04 00:11:02Z  brandont
    Changed the uAttributes field of the DCLFSSTAT structure to be 32-bits.
    Revision 1.14  2008/04/03 23:55:57Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.13  2007/12/04 18:41:14Z  Garyp
    Modified to assume a 512 byte block size if it cannot be determined, rather
    than failing.  Modified to accommodate a NULL volume name.  Both required
    to support WinMobile.
    Revision 1.12  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.11  2007/07/17 20:03:19Z  Garyp
    Updated DclOsFsStatFs() to return a "Device Name" field.  Improved
    parameter validation logic as well as the documentation.  Fixed some
    broken (but not terminal) loop exit logic in GetDeviceInfo().
    Revision 1.10  2007/05/11 02:52:51Z  Garyp
    Modified to use a macro renamed for clarity.
    Revision 1.9  2007/03/15 22:34:59Z  Garyp
    Implemented the DclOsFsStat() function.
    Revision 1.8  2007/02/09 00:10:09Z  joshuab
    Fix for GetDeviceBlockSize on 4.2.  4.2 has different behavior with respect
    to path delimiters on partition names than 5.0.
    Revision 1.7  2007/02/03 00:42:19Z  joshuab
    Added implementation for DclOsRename, added fileuser functions that do
    nothing.
    Revision 1.6  2007/01/21 17:41:52Z  Garyp
    Fixed GetDeviceBlockSize() to skip any leading backslash so that the
    comparison with the partition name will succeed.
    Revision 1.5  2006/12/14 17:44:09Z  Garyp
    Implemented DclOsFsStatFs().  Split out the file I/O and directory
    operations functions into osfileio.c and osdir.c, respectively.
    Revision 1.4  2006/12/13 03:49:21Z  Garyp
    Implemented DclOsFsStatFs().  Updated to use MAX_PATH.
    Revision 1.3  2006/11/04 00:07:20Z  Garyp
    Added stubbed directory functions.  Implemented DclOsDirCreate/Remove().
    Revision 1.2  2006/05/06 21:44:46Z  Garyp
    Added DclOsFsFileEOF(), DclOsFsFileGetChar(), and DclOsFsFilePutChar().
    Revision 1.1  2005/05/02 17:05:06Z  Pauli
    Initial revision
    Revision 1.3  2005/05/02 18:05:06Z  Garyp
    Fleshed out the missing file I/O services.
    Revision 1.2  2005/10/07 00:38:04Z  Garyp
    Modified DclOsFsFileDelete() to return a value that is similar to that
    returned by remove().
    Revision 1.1  2005/05/03 06:21:48Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <storemgr.h>

#include <dcl.h>
#include <dlapiprv.h>
#include "osdate.h"

static D_UINT32 GetDeviceInfo(unsigned nBufferLen, TCHAR *ptzBuffer, const TCHAR *ptzPath);


/*-------------------------------------------------------------------
    Public: DclOsFsFileDelete()

    Delete a file.

    Parameters:
        pszPath - The path of the file to delete.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileDelete(
    const char *pszPath)
{
    TCHAR       awcBuffer[MAX_PATH];

    DclAssert(pszPath);

    MultiByteToWideChar(CP_ACP, 0, pszPath, -1, awcBuffer, DCLDIMENSIONOF(awcBuffer));

    if(DeleteFile(awcBuffer))
        return DCLSTAT_SUCCESS;
    else
        return DclOsErrToDclStatus(GetLastError());
}


/*-------------------------------------------------------------------
    Public: DclOsFsStat()

    Retrieve information about a file or directory.

    Parameters:
        pszName - The name of the object to get info about.
        pStat   - A buffer to receive the Stat information.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStat(
    const char         *pszName,
    DCLFSSTAT          *pStat)
{
    TCHAR               awcBuffer[MAX_PATH];
    HANDLE              hFindFile;
    WIN32_FIND_DATA     FindData;

    DclAssert(pszName);
    DclAssert(pStat);

    MultiByteToWideChar(CP_ACP, 0, pszName, -1, awcBuffer, DCLDIMENSIONOF(awcBuffer));

    hFindFile = FindFirstFile(awcBuffer, &FindData);
    if(hFindFile == INVALID_HANDLE_VALUE)
        return DclOsErrToDclStatus(GetLastError());

    FindClose(hFindFile);

    DclMemSet(pStat, 0, sizeof(*pStat));

    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
        pStat->ulAttributes |= DCLFSATTR_ARCHIVE;
    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        pStat->ulAttributes |= DCLFSATTR_DIRECTORY;
    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
        pStat->ulAttributes |= DCLFSATTR_HIDDEN;
    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        pStat->ulAttributes |= DCLFSATTR_READONLY;
    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
        pStat->ulAttributes |= DCLFSATTR_SYSTEM;

    DclAssert(!FindData.nFileSizeHigh);
    pStat->ulSize = FindData.nFileSizeLow;

    DclOsFileTimeToDclTime(&pStat->tCreation, &FindData.ftCreationTime);
    DclOsFileTimeToDclTime(&pStat->tAccess, &FindData.ftLastAccessTime);
    DclOsFileTimeToDclTime(&pStat->tModify, &FindData.ftLastWriteTime);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsStatFs()

    Retrieve information about a file system.

    Parameters:
        pszPath - A path on the target file system.
        pStatFs - A buffer to receive the StatFs information.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStatFs(
    const char     *pszPath,
    DCLFSSTATFS    *pStatFs)
{
    ULARGE_INTEGER  ullTotalBytes;
    ULARGE_INTEGER  ullFreeBytes;
    ULARGE_INTEGER  ullFreeBytesToCaller;
    D_UINT64        ullTemp;
    TCHAR           awcPath[MAX_PATH];
    TCHAR           awcDev[DCL_MAX_DEVICENAMELEN];

    DclAssert(pszPath);
    DclAssert(pStatFs);

    if(!DclOsAnsiToWcs(awcPath, DCLDIMENSIONOF(awcPath), pszPath, -1))
        return DclOsErrToDclStatus(GetLastError());

    if(!GetDiskFreeSpaceEx(awcPath, &ullFreeBytesToCaller, &ullTotalBytes, &ullFreeBytes))
        return DclOsErrToDclStatus(GetLastError());

    /*  We really want the file system's cluster size, however we can't quite
        figure out how to get this just yet.  The device block size is the
        next best thing, since the FS blocks size cannot be any smaller than
        what the device provides.
    */
    pStatFs->ulBlockSize = GetDeviceInfo(sizeof(awcDev), awcDev, awcPath);
    if(!pStatFs->ulBlockSize)
    {
        /*  If for whatever reason we could not retrieve the device's block
            size.  Sucks to be CE...
        */
        DCLPRINTF(1, ("Unable to determine the device block size for \"%W\", assuming 512 bytes!\n", awcPath));

        pStatFs->ulBlockSize = 512;
    }

    if(awcDev[0])
    {
        if(!DclOsWcsToAnsi(pStatFs->szDeviceName, sizeof(pStatFs->szDeviceName), awcDev, -1))
            return DclOsErrToDclStatus(GetLastError());
    }
    else
    {
        pStatFs->szDeviceName[0] = 0;
    }

    pStatFs->nMaxPathLen = MAX_PATH;
    pStatFs->nMaxNameLen = MAX_PATH;

    DclUint64AssignHiLo(&ullTemp, ullTotalBytes.HighPart, ullTotalBytes.LowPart);
    DclUint64DivUint32(&ullTemp, pStatFs->ulBlockSize);
    pStatFs->ulTotalBlocks = DclUint32CastUint64(&ullTemp);
    DclAssert(DclUint64IsEqualUint32(&ullTemp, pStatFs->ulTotalBlocks));

    DclUint64AssignHiLo(&ullTemp, ullFreeBytes.HighPart, ullFreeBytes.LowPart);
    DclUint64DivUint32(&ullTemp, pStatFs->ulBlockSize);
    pStatFs->ulFreeBlocks = DclUint32CastUint64(&ullTemp);
    DclAssert(DclUint64IsEqualUint32(&ullTemp, pStatFs->ulFreeBlocks));

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileRename()

    Rename or move a file.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRename(
    const char *pszOrigPath,
    const char *pszNewPath)
{
    D_WCHAR     tzOrigName[MAX_PATH];
    D_WCHAR     tzNewName[MAX_PATH];

    DclAssert(pszOrigPath);
    DclAssert(pszNewPath);

    MultiByteToWideChar(CP_ACP, 0, pszOrigPath, -1, tzOrigName, DCLDIMENSIONOF(tzOrigName));
    MultiByteToWideChar(CP_ACP, 0, pszNewPath, -1, tzNewName, DCLDIMENSIONOF(tzNewName));

    if(!MoveFile(tzOrigName, tzNewName))
        return DclOsErrToDclStatus(GetLastError());
    else
        return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsBecomeFileUser()

    Allow this thread to access the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsBecomeFileUser(void)
{
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsReleaseFileUser()

    Indicate that this thread is finished accessing the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsReleaseFileUser(void)
{
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: GetDeviceInfo()

    This function iterates through all the CE Stores (block devices)
    and partitions looking for the one that supplies the volume upon
    which the specified file/path resides, and returns the physical
    block size for that device.

    Parameters:
        ptzPath - A pointer to the full path/file specification
                  which is used to identify the volume.

    Return Value:
        Returns the device's physical block size, or zero if it
        could not be determined.
-------------------------------------------------------------------*/
static D_UINT32 GetDeviceInfo(
    unsigned        nBufferLen,
    TCHAR          *ptzBuffer,
    const TCHAR    *ptzPath)
{
    HANDLE          hStoreSrch;
    STOREINFO       si;
    D_UINT32        ulBlockSize = 0;

    DclAssert(nBufferLen);
    DclAssert(ptzBuffer);
    DclAssert(ptzPath);

    /*  If the CeGetVolumeInfo() is supported (5.0 and later?) then use that,
        as it really will give us the file system's block/cluster size.
    */
    {
        CE_VOLUME_INFO  cvi;

        cvi.cbSize = sizeof(cvi);

        if(CeGetVolumeInfo(ptzPath, CeVolumeInfoLevelStandard, &cvi))
        {
            DCLPRINTF(2, ("VolumeInfo: Store=%W Part=\"%W\" Attribs=%lX Flags=%lX BlockSize=%lU\n",
                cvi.szStoreName, cvi.szPartitionName, cvi.dwAttributes, cvi.dwFlags, cvi.dwBlockSize));

            DclAssert(wcslen(cvi.szStoreName) < (nBufferLen / sizeof(WCHAR)));
            wcsncpy(ptzBuffer, &cvi.szStoreName[0], (nBufferLen / sizeof(WCHAR)));

            return cvi.dwBlockSize;
        }
    }

    /*  Null terminate now in the event of an error
    */
    *ptzBuffer = 0;

    /*  Skip past any leading backslash since the partition names below
        may not include one.
    */
    if(*ptzPath == '\\')
        ptzPath++;

    /*  Iterate through all the stores...
    */
    si.cbSize = sizeof(si);
    hStoreSrch = FindFirstStore(&si);

    while(hStoreSrch != INVALID_HANDLE_VALUE)
    {
        HANDLE      hStore;
        HANDLE      hPart;
        PARTINFO    pi;

        DCLPRINTF(1, ("Found Store Dev=%W Name=\"%W\"\n",                           si.szDeviceName, si.szStoreName));
        DCLPRINTF(1, ("  Class=%lX Type=%lX Flags=%lX Attribs=%lX\n",               si.dwDeviceClass, si.dwDeviceType, si.dwDeviceFlags, si.dwAttributes));
        DCLPRINTF(1, ("  Total Sectors=%llU BytesPerSector=%lU FreeSectors=%llU\n", si.snNumSectors, si.dwBytesPerSector, si.snFreeSectors));
        DCLPRINTF(1, ("  NumPartitions=%lU MountedPartitions=%lU\n",                si.dwPartitionCount, si.dwMountCount));

        hStore = OpenStore(&si.szDeviceName[0]);
        if(hStore == INVALID_HANDLE_VALUE)
        {
            DCLPRINTF(1, ("Error opening store!\n"));
        }
        else
        {
            /*  Iterate through all the partitions on this store...
            */
            pi.cbSize = sizeof(pi);
            hPart = FindFirstPartition(hStore, &pi);

            while(hPart != INVALID_HANDLE_VALUE)
            {
                D_UINT32    ulNameOffset;
                unsigned    nLen;

                DCLPRINTF(1, ("  PartName=\"%W\" FileSys=\"%W\" VolName=\"%W\"\n", pi.szPartitionName, pi.szFileSys, pi.szVolumeName));
                DCLPRINTF(1, ("    NumSectors=%llU Attribs=%lX Type=%u\n",         pi.snNumSectors, pi.dwAttributes, pi.bPartType));

                /*  Strip off any path delimiters at the start of the volume
                    name - CE 4.2 doesn't provide them, CE 5.0 does.
                */
                for(ulNameOffset = 0; pi.szVolumeName[ulNameOffset] == '\\'; ulNameOffset++);

                nLen = wcslen(&pi.szVolumeName[ulNameOffset]);

                /*  Found a partition whose volume name matches the start of
                    the supplied path?
                */
                if(nLen && (_wcsnicmp(&pi.szVolumeName[ulNameOffset], ptzPath, nLen) == 0))
                {
                    /*  YES, save the block size for this store, clean up,
                        and get out.
                    */
                    ulBlockSize = si.dwBytesPerSector;

                    DclAssert(wcslen(si.szDeviceName) < (nBufferLen / sizeof(WCHAR)));
                    wcsncpy(ptzBuffer, &si.szDeviceName[0], (nBufferLen / sizeof(WCHAR)));

                    FindClosePartition(hPart);
                    CloseHandle(hStore);
                    FindCloseStore(hStoreSrch);

                    return ulBlockSize;
                }

                if(!FindNextPartition(hPart, &pi))
                {
                    FindClosePartition(hPart);
                    break;
                }
            }

            CloseHandle(hStore);
        }

        if(!FindNextStore(hStoreSrch, &si))
        {
            FindCloseStore(hStoreSrch);
            break;
        }
    }

    return ulBlockSize;
}



