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

    This module contains the default OS Services implementations of the file
    system directory management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    the make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdir.c $
    Revision 1.11  2009/04/10 17:14:08Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.10  2009/02/08 01:57:42Z  garyp
    Merged from the v4.0 branch.  Changed production asserts to regular
    asserts.  Updated debug messages.  General cleanup.
    Revision 1.9  2008/04/17 18:19:26Z  brandont
    Updated to use the new file system services.
    Revision 1.8  2008/04/05 03:56:01Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.7  2008/04/03 23:30:21Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.6  2008/03/19 20:27:15Z  Garyp
    Documentation updates.
    Revision 1.5  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/04/08 01:43:43Z  Garyp
    Fixed a typo and removed a debug message.
    Revision 1.3  2007/04/07 03:18:14Z  Garyp
    Updated debugging code.
    Revision 1.2  2007/03/15 22:26:19Z  Garyp
    Implemented all the functions in this API except the get/set CWD functions
    which have no meaning in CE.
    Revision 1.1  2006/12/14 00:48:26Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>

#include <dcl.h>
#include <dlapiprv.h>
#include "osdate.h"

struct OsDirHandle
{
    HANDLE          hFindFile;
    WIN32_FIND_DATA FindData;
    TCHAR           tzSearch[MAX_PATH];
};


/*-------------------------------------------------------------------
    Public: DclOsFsDirOpen()

    Open a directory for reading.

    Parameters
        pszPath - The path to the directory.
        phDir   - Populated with a DCLFSDIRHANDLE used to access
                  the directory.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirOpen(
    const char         *pszPath,
    DCLFSDIRHANDLE     *phDir)
{
    DCLFSDIRHANDLE      hDir;
    int                 iLen;
    DWORD               dwAttrib;
    DCLSTATUS           dclStat;

    DclAssert(pszPath);
    DclAssert(phDir);

    hDir = DclMemAlloc(sizeof(*hDir));
    if(!hDir)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        hDir->hFindFile = INVALID_HANDLE_VALUE;

        iLen = MultiByteToWideChar(CP_ACP, 0, pszPath, -1, hDir->tzSearch, DCLDIMENSIONOF(hDir->tzSearch));
        if(!iLen || iLen == MAX_PATH)
        {
            dclStat = DCLSTAT_FS_NAMELENGTH;
        }
        else
        {
            dwAttrib = GetFileAttributes(hDir->tzSearch);
            if(dwAttrib == 0xFFFFFFFF)
            {
                dclStat = DclOsErrToDclStatus(GetLastError());
            }
            else if(!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
            {
                dclStat = DCLSTAT_FS_BADPATH;
            }
            else
            {
                if(hDir->tzSearch[iLen-2] != L'\\')
                {
                    wcscat(hDir->tzSearch, TEXT("\\"));
                    iLen++;
                }

                if(iLen > MAX_PATH - 3)
                {
                    dclStat = DCLSTAT_FS_NAMELENGTH;
                }
                else
                {
                    wcscat(hDir->tzSearch, TEXT("*.*"));
                    dclStat = DCLSTAT_SUCCESS;
                }
            }
        }
    }

    if(dclStat)
    {
        if(hDir)
        {
            DclMemFree(hDir);
            hDir = (void *)0;
        }
    }

    *phDir = hDir;
    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRead()

    Read the next entry in a directory.

    Parameters:
        hDir           - The handle returned by an earlier call to
                         DclOsFsDirOpen.
        pszName        - Populated with the next name in the directory.
        uMaxNameLength - Size of pszName including the NULL terminator.
        pStat          - Populated with information about the entry.
                         This can be NULL if the information is not
                         desired.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRead(
    DCLFSDIRHANDLE  hDir,
    char           *pszName,
    unsigned        nMaxNameLength,
    DCLFSSTAT      *pStat)
{
    unsigned        nLength;

    DclAssert(hDir);
    DclAssert(pszName);

    while(TRUE)
    {
        if(hDir->hFindFile == INVALID_HANDLE_VALUE)
        {
            hDir->hFindFile = FindFirstFile(hDir->tzSearch, &hDir->FindData);
            if(hDir->hFindFile == INVALID_HANDLE_VALUE)
            {
                return DclOsErrToDclStatus(GetLastError());
            }
        }
        else
        {
            if(!FindNextFile(hDir->hFindFile, &hDir->FindData))
            {
                return DclOsErrToDclStatus(GetLastError());
            }
        }

        /*  Do not return a ".." entry since the stat function which is
            implemented using FindFirst/FindNext can't seem to handle it.
        */
        if(wcscmp(hDir->FindData.cFileName, TEXT("..")) != 0)
            break;
    }

    nLength = (unsigned)WideCharToMultiByte(CP_ACP, 0, hDir->FindData.cFileName, -1, pszName, nMaxNameLength, NULL, NULL);
    if(!nLength || nLength == nMaxNameLength)
    {
        return DCLSTAT_FS_NAMELENGTH;
    }

    if(pStat)
    {
        DclMemSet(pStat, 0, sizeof(*pStat));
        if(hDir->FindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)
            pStat->ulAttributes |= DCLFSATTR_ARCHIVE;
        if(hDir->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            pStat->ulAttributes |= DCLFSATTR_DIRECTORY;
        if(hDir->FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
            pStat->ulAttributes |= DCLFSATTR_HIDDEN;
        if(hDir->FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            pStat->ulAttributes |= DCLFSATTR_READONLY;
        if(hDir->FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
            pStat->ulAttributes |= DCLFSATTR_SYSTEM;

        DclAssert(!hDir->FindData.nFileSizeHigh);
        pStat->ulSize = hDir->FindData.nFileSizeLow;

        DclOsFileTimeToDclTime(&pStat->tCreation, &hDir->FindData.ftCreationTime);
        DclOsFileTimeToDclTime(&pStat->tAccess, &hDir->FindData.ftLastAccessTime);
        DclOsFileTimeToDclTime(&pStat->tModify, &hDir->FindData.ftLastWriteTime);
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRewind()

    Reset the position within an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclOsFsDirOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRewind(
    DCLFSDIRHANDLE hDir)
{
    DclAssert(hDir);

    if(INVALID_HANDLE_VALUE != hDir->hFindFile)
    {
        FindClose(hDir->hFindFile);
    }
    hDir->hFindFile = INVALID_HANDLE_VALUE;

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirClose()

    Close an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclOsFsDirOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirClose(
    DCLFSDIRHANDLE hDir)
{
    DclAssert(hDir);

    if(INVALID_HANDLE_VALUE != hDir->hFindFile)
    {
        FindClose(hDir->hFindFile);
    }

    DclMemFree(hDir);

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirCreate()

    Create a directory.

    Parameters:
        pszPath - The path to the directory to create.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirCreate(
    const char *pszPath)
{
    TCHAR       awcBuffer[MAX_PATH];

    DclAssert(pszPath);
    MultiByteToWideChar(CP_ACP, 0, pszPath, -1, awcBuffer, DCLDIMENSIONOF(awcBuffer));

    if(CreateDirectory(awcBuffer, NULL))
        return DCLSTAT_SUCCESS;
    else
        return DclOsErrToDclStatus(GetLastError());
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRemove()

    Remove a directory.

    Parameters:
        pszPath - The path to the directory to remove.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRemove(
    const char *pszPath)
{
    TCHAR       awcBuffer[MAX_PATH];

    DclAssert(pszPath);
    MultiByteToWideChar(CP_ACP, 0, pszPath, -1, awcBuffer, DCLDIMENSIONOF(awcBuffer));

    if(RemoveDirectory(awcBuffer))
        return DCLSTAT_SUCCESS;
    else
        return DclOsErrToDclStatus(GetLastError());
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRename()

    Rename or move a directory.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRename(
    const char *pszOrigPath,
    const char *pszNewPath)
{
    DclAssert(pszOrigPath);
    DclAssert(pszNewPath);

    /*  Directories are renamed the same way as files, so just pass-through
        to the file handler.
    */
    return DclOsFsFileRename(pszOrigPath, pszNewPath);
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirSetWorking()

    Set the current working directory.

    *Note* -- Some OS environments do not support the concept of
    a current working directory (CWD).  The DclOsFsDirGetWorking()
    function should be used to determine whether CWD functionality
    can be used or not.  See the comments within that function.

    Parameters:
        pszPath - The path of the directory to set as the CWD.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirSetWorking(
    const char     *pszPath)
{
    DclAssert(pszPath);

    DCLPRINTF(1, ("DclOsFsDirSetWorking() functionality is not supported\n"));

    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirGetWorking()

    Retrieve the current working directory.

    *Note* -- If this functionality is not supported, this function
    must return an error code (without asserting).  Higher level
    software may then use this function to detect whether CWD
    functionality is available or not and act appropriately.

    Parameters:
        pszPath        - A buffer to receive the current working
                         directory.
        uMaxPathLength - Size of buffer including the NULL terminator.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirGetWorking(
    char       *pszPath,
    unsigned    nMaxPathLength)
{
    DclAssert(pszPath);
    DclAssert(nMaxPathLength);

    return DCLSTAT_FS_BADCMD;
}


