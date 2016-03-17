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

    This module contains the generalized file I/O abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfsfile.c $
    Revision 1.10  2009/05/02 18:11:19Z  garyp
    Resolved Bug 2670 -- read/write shims return -1 on error.
    Revision 1.9  2009/05/01 23:04:42Z  garyp
    Cleaned up documentation and code formatting.  Added asserts.  Changed
    most production asserts to regular asserts and production messages to
    debug messages.  No functional changes.
    Revision 1.8  2009/02/26 22:31:12Z  keithg
    Added explicit type casts for function return types of size_t.
    Revision 1.7  2008/05/29 01:43:23Z  garyp
    Merged from the WinMobile branch.
    Revision 1.6.1.2  2008/05/29 01:43:23Z  garyp
    Added asserts.
    Revision 1.6  2008/04/19 02:22:36Z  brandont
    Implemented the mini-redirector to allow an alternative set of file
    system services to be implemented and have the file system calls
    directed accordingly.
    Revision 1.5  2008/04/19 00:10:50Z  brandont
    Updated the depricated DclOsFileSeek, DclOsFileTell, DclOsFileRead,
    and DclOsFileWrite to not call OS services implementations directly.
    Revision 1.4  2008/04/19 00:07:34Z  brandont
    Removed DclFsFileDelete and DclFsFileRename.
    Revision 1.3  2008/04/16 23:39:17Z  brandont
    Updated to use the new file system services.
    Revision 1.2  2008/04/08 02:41:14Z  brandont
    Updated to use the DclOsFs interfaces.  Added legacy interfaces
    for DclOsFileEOF, DclOsFileSeek, DclOsFileTell, DclOsFileRead,
    and DclOsFileWrite.
    Revision 1.1  2008/04/04 01:23:44Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


#if DCLCONF_MINI_REDIRECTOR
struct OsFileHandle
{
    DCL_FS_REDIR_INTERFACE *pInterface;
    DCLFSFILEHANDLE         hFile;
};
#endif


/*-------------------------------------------------------------------
    Public: DclFsFileOpen()

    Open a file.  This API can also be used to create or truncate
    files.

    Parameters:
        pszPath - The path to the file.
        pszMode - The file open mode.
        phFile  - Populated with a DCLFSFILEHANDLE used to access
                  the file.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileOpen(
    const char         *pszPath,
    const char         *pszMode,
    DCLFSFILEHANDLE    *phFile)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLFSFILEHANDLE     hFile;
    const char         *pszFsPath;
    DCLSTATUS           dclStat;

    DclAssert(pszPath);
    DclAssert(pszMode);
    DclAssert(phFile);

    hFile = DclMemAllocZero(sizeof(*hFile));
    if(!hFile)
    {
        dclStat = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        /*  Determine the type of file system to use and direct this
            call to the appropriate service implementation.
        */
        hFile->pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
        if(hFile->pInterface && hFile->pInterface->FileOpen)
        {
            dclStat = hFile->pInterface->FileOpen(pszFsPath, pszMode, &hFile->hFile);
        }
        else
        {
            DCLPRINTF(1, ("DclFsFileOpen(): Invalid file system command\n"));
            dclStat = DCLSTAT_FS_BADCMD;
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

  #else

    DclAssert(pszPath);
    DclAssert(pszMode);
    DclAssert(phFile);

    return DclOsFsFileOpen(pszPath, pszMode, phFile);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileSeek()

    Seek to a position in a file.

    Parameters:
        hFile    - The handle returned by an earlier call to DclFsFileOpen().
        lOffset  - Relative file position.
        iOrgin   - Relative to what location in a file.  DCLFSFILESEEK_SET,
                   DCLFSFILESEEK_CUR, or DCLFSFILESEEK_END.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileSeek(
    DCLFSFILEHANDLE hFile,
    D_INT32         lOffset,
    int             iOrigin)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);

    if(hFile->pInterface && hFile->pInterface->FileSeek)
    {
        dclStat = hFile->pInterface->FileSeek(hFile->hFile, lOffset, iOrigin);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileSeek(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    return dclStat;

  #else

    DclAssert(hFile);

    return DclOsFsFileSeek(hFile, lOffset, iOrigin);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileTell()

    Obtain the current position in a file.

    Parameters:
        hFile        - The handle returned by an earlier call to
                       DclFsFileOpen.
        pulPosition  - Populated with the current file position.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileTell(
    DCLFSFILEHANDLE hFile,
    D_UINT32       *pulPosition)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(pulPosition);

    if(hFile->pInterface && hFile->pInterface->FileTell)
    {
        dclStat = hFile->pInterface->FileTell(hFile->hFile, pulPosition);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileTell(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    return dclStat;

  #else

    DclAssert(hFile);
    DclAssert(pulPosition);

    return DclOsFsFileTell(hFile, pulPosition);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileRead()

    Read data from a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclFsFileOpen.
        pBuffer        - A pointer to the buffer to fill.
        ulSize         - Number of bytes to read.
        pulTransfered  - Populated with the number of bytes
                         actually read.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileRead(
    DCLFSFILEHANDLE hFile,
    void           *pBuffer,
    D_UINT32        ulSize,
    D_UINT32       *pulTransfered)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(pulTransfered);

    if(hFile->pInterface && hFile->pInterface->FileRead)
    {
        dclStat = hFile->pInterface->FileRead(hFile->hFile, pBuffer, ulSize, pulTransfered);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileRead(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    return dclStat;

  #else

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(pulTransfered);

    return DclOsFsFileRead(hFile, pBuffer, ulSize, pulTransfered);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileWrite()

    Write data to a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclFsFileOpen.
        pBuffer        - A pointer to the buffer from which to write.
        ulSize         - Number of bytes to write.
        pulTransfered  - Populated with the number of bytes
                         actually written.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileWrite(
    DCLFSFILEHANDLE hFile,
    const void     *pBuffer,
    D_UINT32        ulSize,
    D_UINT32       *pulTransfered)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(pulTransfered);

    if(hFile->pInterface && hFile->pInterface->FileWrite)
    {
        dclStat = hFile->pInterface->FileWrite(hFile->hFile, pBuffer, ulSize, pulTransfered);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileWrite(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    return dclStat;

  #else

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(pulTransfered);

    return DclOsFsFileWrite(hFile, pBuffer, ulSize, pulTransfered);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileFlush()

    Flushes any pending file data.

    Parameters:
        hFile - The handle returned by an earlier call to DclFsFileOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileFlush(
    DCLFSFILEHANDLE hFile)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);

    if(hFile->pInterface && hFile->pInterface->FileFlush)
    {
        dclStat = hFile->pInterface->FileFlush(hFile->hFile);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileFlush(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    return dclStat;

  #else

    DclAssert(hFile);

    return DclOsFsFileFlush(hFile);

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileClose()

    Close an open file.

    Parameters:
        hFile - The handle returned by an earlier call to DclFsFileOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileClose(
    DCLFSFILEHANDLE hFile)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS       dclStat;

    DclAssert(hFile);

    if(hFile->pInterface && hFile->pInterface->FileClose)
    {
        dclStat = hFile->pInterface->FileClose(hFile->hFile);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileClose(): Invalid file system command\n"));
        dclStat = DCLSTAT_FS_BADCMD;
    }

    if(!dclStat)
    {
        DclMemFree(hFile);
    }
    return dclStat;

  #else

    DclAssert(hFile);

    return DclOsFsFileClose(hFile);

  #endif
}


/*-------------------------------------------------------------------
    Protected: DclOsFileEOF()

    This interface exists solely as a shim for code which has not
    yet been migrated from the old OS Services level file I/O
    interface to the new generalized file I/O interface.
-------------------------------------------------------------------*/
int DclOsFileEOF(
    DCLFSFILEHANDLE hFile)
{
  #if DCLCONF_MINI_REDIRECTOR
    int             iResult;

    DclAssert(hFile);

    if(hFile->pInterface && hFile->pInterface->FileEOF)
    {
        iResult = hFile->pInterface->FileEOF(hFile->hFile);
    }
    else
    {
        DCLPRINTF(1, ("DclOsFileEOF(): Invalid file system command\n"));
        DclProductionError();
        iResult = 1;
    }

    return iResult;

  #else

    DclAssert(hFile);

    return DclOsFsFileEOF(hFile);

  #endif
}


/*-------------------------------------------------------------------
    Protected: DclOsFileSeek()

    This interface exists solely as a shim for code which has not
    yet been migrated from the old OS Services level file I/O
    interface to the new generalized file I/O interface.
-------------------------------------------------------------------*/
int DclOsFileSeek(
    DCLFSFILEHANDLE hFile,
    long            lOffset,
    int             iOrgin)
{
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(lOffset <= D_INT32_MAX);

    dclStat = DclFsFileSeek(hFile, (D_INT32)lOffset, iOrgin);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


/*-------------------------------------------------------------------
    Protected: DclOsFileTell()

    This interface exists solely as a shim for code which has not
    yet been migrated from the old OS Services level file I/O
    interface to the new generalized file I/O interface.
-------------------------------------------------------------------*/
long DclOsFileTell(
    DCLFSFILEHANDLE hFile)
{
    D_UINT32        ulPosition;
    DCLSTATUS       dclStat;

    DclAssert(hFile);

    dclStat = DclFsFileTell(hFile, &ulPosition);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        return -1;
    }
    else
    {
        DclAssert(ulPosition <= D_INT32_MAX);
        return (long)ulPosition;
    }
}


/*-------------------------------------------------------------------
    Protected: DclOsFileRead()

    This interface exists solely as a shim for code which has not
    yet been migrated from the old OS Services level file I/O
    interface to the new generalized file I/O interface.
-------------------------------------------------------------------*/
size_t DclOsFileRead(
    void           *pBuffer,
    size_t          nSize,
    size_t          nCount,
    DCLFSFILEHANDLE hFile)
{
    D_UINT32        ulTransfered;
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(nSize);
    DclAssert((nSize * nCount) <= D_UINT32_MAX);

    dclStat = DclFsFileRead(hFile, pBuffer, (D_UINT32)(nSize * nCount), &ulTransfered);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        return 0;
    }
    else
    {
        return (size_t)(ulTransfered / nSize);
    }
}


/*-------------------------------------------------------------------
    Protected: DclOsFileWrite()

    This interface exists solely as a shim for code which has not
    yet been migrated from the old OS Services level file I/O
    interface to the new generalized file I/O interface.
-------------------------------------------------------------------*/
size_t DclOsFileWrite(
    const void     *pBuffer,
    size_t          nSize,
    size_t          nCount,
    DCLFSFILEHANDLE hFile)
{
    D_UINT32        ulTransfered;
    DCLSTATUS       dclStat;

    DclAssert(hFile);
    DclAssert(pBuffer);
    DclAssert(nSize);
    DclAssert((nSize * nCount) <= D_UINT32_MAX);

    dclStat = DclFsFileWrite(hFile, pBuffer, (D_UINT32)(nSize * nCount), &ulTransfered);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        return 0;
    }
    else
    {
        return (size_t)(ulTransfered / nSize);
    }
}


