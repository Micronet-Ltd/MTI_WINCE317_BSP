/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains the default implementations of the file I/O OS
    Services functions.

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfileio.c $
    Revision 1.15  2011/03/27 06:54:30Z  garyp
    Updated DclOsFsFileFlush() to send a file sytem flush IOCTL down
    the stack.
    Revision 1.14  2011/03/25 02:30:38Z  garyp
    Updated documentation and debug code -- no functional changes.
    Revision 1.13  2009/06/04 06:01:14Z  jeremys
    Fixed DclOsFsFileClose, which was not freeing the handle structure.
    Revision 1.12  2009/05/02 18:44:24Z  garyp
    Updated DclOsFsFileOpen() to display more verbose error message information
    if CreateFile() fails.  Corrected the use of production asserts to be regular
    asserts.
    Revision 1.11  2009/04/10 02:01:31Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.10  2008/05/07 03:23:12Z  garyp
    Fixed the read and write functions to use the proper handle.
    Revision 1.9  2008/04/17 23:45:09Z  jeremys
    Fixed a typo.
    Revision 1.8  2008/04/17 19:20:37Z  brandont
    Updated to use the new file system services.
    Revision 1.7  2008/04/05 03:52:18Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.6  2008/04/03 23:55:58Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.5  2007/11/03 23:31:36Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/03/15 19:54:09Z  Garyp
    Eliminated DclOsFsFileGet/PutChar().  Removed unneeded unicode
    conditional code.
    Revision 1.3  2007/02/07 01:00:29Z  joshuab
    Modification in behavior with respect to the sharing flags and
    interpretation of "w", "a" and "r".  "w" modes now disallow all
    sharing, "a" permit read sharing and "r" permit all sharing.
    Revision 1.2  2007/02/05 16:20:41Z  joshuab
    Changed to implementation using Win32 APIs.
    Revision 1.1  2006/12/14 00:48:26Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#if _WIN32_WCE >= 500
#include <fsioctl.h>
#endif

#include <dcl.h>
#include <dlapiprv.h>

struct OsFileHandle
{
    HANDLE  hHandle;
};


/*-------------------------------------------------------------------
    Public: DclOsFsFileOpen()

    Open a file.  This API can also be used to create or truncate
    files.

    Parameters:
        pszPath - The path to the file.
        pszMode - The file open mode.
        phFile  - Populated with a DCLFSFILEHANDLE used to access
                  the file.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileOpen(
    const char         *pszFileName,
    const char         *pszMode,
    DCLFSFILEHANDLE    *phFile)
{
    D_WCHAR             szUnicodeName[_MAX_PATH];
    DWORD               dwDesiredAccess = 0;
    DWORD               dwShareMode = 0;
    DWORD               dwCreationDisposition = 0;
    DCLFSFILEHANDLE     hFile;
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;

    DclAssertReadPtr(pszFileName, 0);
    DclAssertReadPtr(pszMode, 0);
    DclAssertWritePtr(phFile, sizeof(*phFile));

    hFile = DclMemAlloc(sizeof(*hFile));
    if(!hFile)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        /*  Parse the mode switches
        */
        if(DclStrChr(pszMode, 'w'))
        {
            /*  Creation mode
            */
            dwDesiredAccess = GENERIC_WRITE;
            dwShareMode = 0;
            dwCreationDisposition = CREATE_ALWAYS;
            if(DclStrChr(pszMode, '+'))
            {
                dwDesiredAccess |= GENERIC_READ;
            }
        }
        else if(DclStrChr(pszMode, 'a'))
        {
            /*  Append mode
            */
            dwDesiredAccess = GENERIC_WRITE;
            dwShareMode = FILE_SHARE_READ;
            dwCreationDisposition = OPEN_ALWAYS;
            if(DclStrChr(pszMode, '+'))
            {
                dwDesiredAccess |= GENERIC_READ;
            }
        }
        else if(DclStrChr(pszMode, 'r'))
        {
            /*  Read mode
            */
            dwDesiredAccess = GENERIC_READ;
            dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
            dwCreationDisposition = OPEN_EXISTING;
            if(DclStrChr(pszMode, '+'))
            {
                dwDesiredAccess |= GENERIC_WRITE;
            }
        }
        else
        {
            /*  Default case, allow read/write
            */
            dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
            dwShareMode = 0;
            dwCreationDisposition = OPEN_EXISTING;
        }

        /*  open/create the file
        */
        DclOsAnsiToWcs(szUnicodeName, DCLDIMENSIONOF(szUnicodeName), pszFileName, -1);
        hFile->hHandle = CreateFile(
                szUnicodeName,
                dwDesiredAccess,
                dwShareMode,
                NULL,
                dwCreationDisposition,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                NULL);

        if(INVALID_HANDLE_VALUE == hFile->hHandle)
        {
            DWORD   dwError = GetLastError();

            DCLPRINTF(dwError == ERROR_DISK_FULL ? 2 : 1, 
                ("DclOsFsFileOpen() CreateFile() File='%W' Access=%lX Share=%lX Disp=%lX returned error %lX\n",
                szUnicodeName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwError));

            dclStat = DclOsErrToDclStatus(dwError);
        }
    }

    if(dclStat != DCLSTAT_SUCCESS)
    {
        if(hFile)
        {
            DclMemFree(hFile);
            hFile = (void *)0;
        }
    }

    *phFile = hFile;

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileClose()

    Close an open file.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileClose(
    DCLFSFILEHANDLE     hFile)
{
	HANDLE              hHandle;
    
	DclAssert(hFile);

	hHandle = hFile->hHandle;
	DclMemFree(hFile);

    if(CloseHandle(hHandle))
    {
        return DCLSTAT_SUCCESS;
    }
    else
    {
        return DclOsErrToDclStatus(GetLastError());
    }
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileEOF()

    Determine if the file pointer as at the end-of-file or not.

    *Compatibility Note* -- Some documentation indicates that this
    function returns zero on the <first attempt to read> past
    the end-of-file.  This behavior should not be depended upon.

    For example if you seek to the end-of-file, and call this
    function it <may> report end-of-file, even though by the
    previous definition it should NOT yet report end-of-file,
    because a read attempt has not yet been made.  Therefore
    this behavior must not be depended upon.

    Parameters:
        hFile       - The DCLFSFILEHANDLE value

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
int DclOsFsFileEOF(
    DCLFSFILEHANDLE hFile)
{
    D_BOOL          fEndOfFile = FALSE;
    DWORD           dwCurrentPosition;
    DWORD           dwEndPosition;

    DclAssert(hFile);

    /*  Get the current file pointer position and the end positions
    */
    dwCurrentPosition = SetFilePointer(hFile->hHandle, 0, NULL, FILE_CURRENT);
    dwEndPosition = SetFilePointer(hFile->hHandle, 0, NULL, FILE_END);

    /*  Check if we were at the end
    */
    if(dwCurrentPosition == dwEndPosition)
    {
        fEndOfFile = TRUE;
    }

    /*  Set the position back
    */
    SetFilePointer(hFile->hHandle, dwCurrentPosition, NULL, FILE_BEGIN);

    return fEndOfFile;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileFlush()

    Flush any pending file data.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileFlush(
    DCLFSFILEHANDLE hFile)
{
    DclAssert(hFile);

    if(FlushFileBuffers(hFile->hHandle))
    {
      #if _WIN32_WCE >= 500
        /*  The FlushFileBuffers() API only flushes intermediate buffers
            used by the file system.  Ensure that we really are flushing
            the data down to the block device (and beyond).
        */  
        if(!DeviceIoControl(hFile->hHandle, FSSCTL_FLUSH_BUFFERS, NULL, 0, NULL, 0, NULL, NULL))
            DCLPRINTF(1, ("DclOsFsFileFlush() DeviceIoControl(FSSCTL_FLUSH_BUFFERS) failed with error %lU\n", GetLastError()));
      #endif

        return DCLSTAT_SUCCESS;
    }
    else
    {
        return DclOsErrToDclStatus(GetLastError());
    }
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileSeek()

    Seek to a position in a file.

    Parameters:
        hFile    - The handle returned by an earlier call to DclOsFsFileOpen.
        lOffset  - Relative file position.
        iOrgin   - Relative to what location in a file.  DCLFSFILESEEK_SET,
                   DCLFSFILESEEK_CUR, or DCLFSFILESEEK_END.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileSeek(
    DCLFSFILEHANDLE hFile,
    D_INT32         lOffset,
    int             iOrigin)
{
    DWORD           dwMoveMethod;
    DWORD           dwResult;

    DclAssert(hFile);

    if(iOrigin == DCLFSFILESEEK_CUR)
        dwMoveMethod = FILE_CURRENT;
    else if(iOrigin == DCLFSFILESEEK_SET)
        dwMoveMethod = FILE_BEGIN;
    else if(iOrigin == DCLFSFILESEEK_END)
        dwMoveMethod = FILE_END;
    else
        return DCLSTAT_FS_UNDEFINED;

    dwResult = SetFilePointer(hFile->hHandle, lOffset, NULL, dwMoveMethod);
    if(INVALID_SET_FILE_POINTER == dwResult)
    {
        return DclOsErrToDclStatus(GetLastError());
    }
    else
    {
        return DCLSTAT_SUCCESS;
    }
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileTell()

    Obtain the current position in a file.

    Parameters:
        hFile        - The handle returned by an earlier call to
                       DclOsFsFileOpen.
        pulPosition  - Populated with the current file position.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileTell(
    DCLFSFILEHANDLE hFile,
    D_UINT32       *pulPosition)
{
    DWORD           dwCurrentPosition;

    DclAssert(hFile);
    DclAssertWritePtr(pulPosition, sizeof(*pulPosition));

    dwCurrentPosition = SetFilePointer(hFile->hHandle, 0, NULL, FILE_CURRENT);
    if (INVALID_SET_FILE_POINTER == dwCurrentPosition)
    {
        return DclOsErrToDclStatus(GetLastError());
    }
    else
    {
        DclAssert(dwCurrentPosition <= D_UINT32_MAX);

        *pulPosition = dwCurrentPosition;
        return DCLSTAT_SUCCESS;
    }
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileRead()

    Read data from a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclOsFsFileOpen.
        pBuffer        - A pointer to the buffer to fill.
        ulSize         - Number of bytes to read.
        pulTransfered  - Populated with the number of bytes
                         actually read.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRead(
    DCLFSFILEHANDLE hFile,
    void           *pBuffer,
    D_UINT32        ulSize,
    D_UINT32       *pulTransfered)
{
    DWORD           dwBytesTransfered;
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssertWritePtr(pBuffer, ulSize);
    DclAssertWritePtr(pulTransfered, sizeof(*pulTransfered));

    if(ReadFile(hFile->hHandle, pBuffer, ulSize, &dwBytesTransfered, NULL))
    {
        dclStat = DCLSTAT_SUCCESS;
    }
    else
    {
        dclStat = DclOsErrToDclStatus(GetLastError());
    }

    *pulTransfered = dwBytesTransfered;

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileWrite()

    Write data to a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclOsFsFileOpen.
        pBuffer        - A pointer to the buffer from which to write.
        ulSize         - Number of bytes to write.
        pulTransfered  - Populated with the number of bytes
                         actually written.

    Return value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileWrite(
    DCLFSFILEHANDLE hFile,
    const void     *pBuffer,
    D_UINT32        ulSize,
    D_UINT32       *pulTransfered)
{
    DWORD           dwBytesTransfered;
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssertReadPtr(pBuffer, ulSize);
    DclAssertWritePtr(pulTransfered, sizeof(*pulTransfered));

    if(WriteFile(hFile->hHandle, pBuffer, ulSize, &dwBytesTransfered, NULL))
    {
        dclStat = DCLSTAT_SUCCESS;
    }
    else
    {
        dclStat = DclOsErrToDclStatus(GetLastError());
    }

    *pulTransfered = dwBytesTransfered;

    return dclStat;
}
