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

    This module contains the default implementations of the file I/O OS
    Services functions.

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the dclproj.mak file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfileio.c $
    Revision 1.17  2010/09/13 20:01:42Z  jimmb
    Initialized to satisfy Rvds ARM compiler
    Revision 1.16  2010/07/25 21:21:30Z  garyp
    Updated to build cleanly with Code Sourcery tools.
    Revision 1.15  2010/05/20 18:58:59Z  jeremys
    Fixed a signed/unsigned conversion bug.
    Revision 1.14  2010/05/19 18:15:47Z  garyp
    Changed a use of DCLSTAT_FS_UNDEFINED to DCLSTAT_FS_BADORIGIN.  Removed
    incorrect use of production asserts.
    Revision 1.13  2009/05/22 23:49:44Z  brandont
    Corrected issues with free memory allocated by this abstraction for close,
    flush, and seek.
    Revision 1.12  2009/04/10 02:01:26Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.11  2008/10/29 23:39:31Z  brandont
    Corrected problems detecting errors.
    Revision 1.10  2008/09/18 21:21:46Z  brandont
    Updated to use the Reliance posix APIs.
    Revision 1.9  2008/06/07 00:59:50Z  brandont
    Removed Unicode support.
    Revision 1.8  2008/04/17 00:51:42Z  brandont
    Corrected problem with error code translation in DclOsFsFileSeek.
    Revision 1.7  2008/04/16 23:58:06Z  brandont
    Updated to use the new file system services.
    Revision 1.6  2008/04/05 03:52:50Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.5  2008/04/03 23:55:47Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.4  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/30 22:14:31Z  pauli
    Removed DclOsFsFileTruncate.
    Revision 1.2  2007/06/23 02:43:25Z  brandont
    Added implementation for DclOsFsFileTruncate.
    Revision 1.1  2007/03/15 02:43:16Z  Garyp
    Initial revision
    Revision 1.15  2007/02/28 02:50:35Z  brandont
    Implemented stat functionality for non-Reliance API.
    Revision 1.14  2006/10/16 22:55:22Z  brandont
    Added support for Reliance when in Unicode mode.
    Revision 1.13  2006/10/14 00:34:58Z  joshuab
    Fix for unicode builds.
    Revision 1.12  2006/10/10 18:16:39Z  joshuab
    Fix memory leak - the FILE_HANDLE allocated in the open was never freed in
    the close.
    Revision 1.11  2006/10/07 02:46:07Z  Garyp
    Fixed various errors and warnings noted by the RealView tools.
    Revision 1.10  2006/10/06 22:04:32Z  peterb
    Added implementations for DclOsBecomeFileUser and DclOsReleaseFileUser.
    Revision 1.9  2006/08/25 03:19:26Z  Pauli
    Removed an unused variable.
    Revision 1.8  2006/05/10 17:51:43Z  Pauli
    Expanded the file system interface abstraction to include directory
    operations.
    Revision 1.7  2006/05/06 21:44:46Z  Garyp
    Added DclOsFsFileEOF(), DclOsFsFileGetChar(), and DclOsFsFilePutChar().
    Revision 1.6  2006/04/06 04:44:36Z  brandont
    Added conditional to not build this file if DCL_OSFEATURE_UNICODE is set.
    Revision 1.5  2006/03/01 19:59:47Z  Pauli
    Removed the attempts to include the Reliance RTOS header using relative
    paths.  The correct path is now added to the include directories list by
    the build process.
    Revision 1.4  2006/03/01 04:07:37Z  brandont
    Corrected path problem with including reliance_rtos.h
    Revision 1.3  2006/02/22 23:06:55Z  Pauli
    Renamed Reliance open mode and permission flags.
    Revision 1.2  2006/02/09 23:12:06Z  Pauli
    Implemented file system interface routines.  Added option to map calls
    to standard C routines or the Reliance RTOS kit API.
    Revision 1.1  2005/10/06 23:38:06Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*  Determine which file system interface to use.
*/
#undef OSFILE_USE_RELIANCE
#ifndef OSFILE_USE_RELIANCE
  #if (D_PRODUCTNUM == PRODUCTNUM_4GR) || (D_PRODUCTNUM == PRODUCTNUM_RELIANCE)
    #define OSFILE_USE_RELIANCE TRUE
  #else
    #define OSFILE_USE_RELIANCE FALSE
  #endif
#endif


#if OSFILE_USE_RELIANCE

/*  Using Reliance as the file system interface.
*/
#include <reliance_rtos.h>

struct OsFileHandle
{
    int fd;
};

#endif


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
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileOpen(
    const char         *pszFileName,
    const char         *pszMode,
    DCLFSFILEHANDLE    *phFile)
{
  #if OSFILE_USE_RELIANCE
    unsigned short      uFlags = 0;
    unsigned short      uMode = 0;
    DCLFSFILEHANDLE     hFile;
    DCLSTATUS           DclStatus = DCLSTAT_SUCCESS;

    DclAssert(pszFileName);
    DclAssert(pszMode);
    DclAssert(phFile);

    hFile = DclMemAlloc(sizeof(*hFile));
    if(!hFile)
    {
        DclStatus = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        /*  Parse the mode switches
        */
        if(DclStrChr(pszMode, 'w'))
        {
            /*  Creation mode
            */
            uFlags = REL_O_CREAT | REL_O_TRUNC;
            uMode = REL_IWRITE;
            if(DclStrChr(pszMode, '+'))
            {
                uMode |= REL_IREAD;
                uFlags |= REL_O_RDWR;
            }
            else
            {
                uFlags |= REL_O_WRONLY;
            }
        }
        else if(DclStrChr(pszMode, 'a'))
        {
            /*  Append mode
            */
            uFlags = REL_O_APPEND;
            uMode = REL_IWRITE;
            if(DclStrChr(pszMode, '+'))
            {
                uMode |= REL_IREAD;
                uFlags |= REL_O_RDWR;
            }
            else
            {
                uFlags |= REL_O_WRONLY;
            }
        }
        else if(DclStrChr(pszMode, 'r'))
        {
            /*  Read mode
            */
            uMode = REL_IREAD;
            if(DclStrChr(pszMode, '+'))
            {
                uMode |= REL_IWRITE;
                uFlags |= REL_O_RDWR;
            }
            else
            {
                uFlags |= REL_O_RDONLY;
            }
        }
        else
        {
            /*  Default case, allow read/write
            */
            uMode = REL_IWRITE | REL_IREAD;
            uFlags = REL_O_RDWR;
        }

        /*  ToDo: Why is uFlags declared as an "unsigned short", but
                  being passed to a function that takes an "int"?
        */

        /*  open/create the file
        */
        hFile->fd = dlp_open(pszFileName, uFlags, uMode);
        if( hFile->fd < 0 )
        {
            DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
            DclMemFree(hFile);
            hFile = NULL;
        }
    }

    *phFile = hFile;
    return DclStatus;
    
  #else
  
    DclProductionAssert(pszFileName);
    DclProductionAssert(pszMode);
    DclProductionAssert(phFile);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileClose()

    Close an open file.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen().

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileClose(
    DCLFSFILEHANDLE hFile)
{
  #if OSFILE_USE_RELIANCE
    DCLSTATUS       DclStatus;
  
    DclAssert(hFile);

    if ( dlp_close(hFile->fd)  <  0 )
    {
        DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
    }
    else
    {
        DclStatus = DclMemFree(hFile);
    }

    return DclStatus;

  #else
  
    (void)hFile;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileEOF()

    Determine if the file pointer as at the end-of-file or not.

    Compatibility Note:  Some documentation indicates that this
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
        Returns zero if the file pointer is not at the end-of-file,
        and non-zero if it is.
-------------------------------------------------------------------*/
int DclOsFsFileEOF(
    DCLFSFILEHANDLE hFile)
{
  #if OSFILE_USE_RELIANCE
    D_UINT32        ulCurrentPosition = 0; /* initialized to satisfy Rvds ARM compiler */
    D_UINT32        ulFileSize = 0;	 /* initialized to satisfy Rvds ARM compiler */
    int             nReturn = 1;    /* default to reporting EOF */
    DCLSTATUS       DclStatus;

    DclAssert(hFile);

    /*  Save the current file position
    */
    DclStatus = DclOsFsFileTell(hFile, &ulCurrentPosition);
    if(DclStatus)
    {
        /*  ToDo: This function should be refactored to provide a
                  mechanism for reporting errors.
        */
        DclProductionError();
    }

    if(DclOsFsFileSeek(hFile, 0, DCLFSFILESEEK_END) != DCLSTAT_SUCCESS)
    {
        DclProductionError();
    }

    DclStatus = DclOsFsFileTell(hFile, &ulFileSize);
    if(DclStatus)
    {
        DclProductionError();
    }

    /*  If we are NOT at EOF, change our return value accordingly
    */
    if(ulCurrentPosition < ulFileSize)
        nReturn = 0;

    /*  Restore the original position
    */
    if(ulCurrentPosition > D_INT32_MAX)
    {
        DclStatus = DclOsFsFileSeek(hFile, D_INT32_MAX, DCLFSFILESEEK_SET);
        if(DclStatus == DCLSTAT_SUCCESS)
        {
            DclStatus = DclOsFsFileSeek(hFile, (D_INT32)(ulCurrentPosition - D_INT32_MAX), DCLFSFILESEEK_CUR);
        }
    }
    else
    {
        DclStatus = DclOsFsFileSeek(hFile, (D_INT32)ulCurrentPosition, DCLFSFILESEEK_SET);
    }

    if(DclStatus)
    {
        DclProductionError();
    }

    return nReturn;
    
  #else
  
    (void)hFile;
    DclProductionError();
    return -1;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileFlush()

    Flush any pending file data.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileFlush(
    DCLFSFILEHANDLE hFile)
{
  #if OSFILE_USE_RELIANCE
    DCLSTATUS       DclStatus;

    DclAssert(hFile);

    if ( dlp_fsync(hFile->fd)  <  0 )
        DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
    else
        DclStatus = DCLSTAT_SUCCESS;

    return DclStatus;

  #else
  
    (void)hFile;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
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
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileSeek(
    DCLFSFILEHANDLE hFile,
    D_INT32         lOffset,
    int             iOrigin)
{
  #if OSFILE_USE_RELIANCE
    unsigned short  uRelOrigin;
    DCLSTATUS       DclStatus;

    DclAssert(hFile);

    /*  Translate the origin
    */
    if(iOrigin == DCLFSFILESEEK_SET)
    {
        uRelOrigin = REL_SEEK_SET;
    }
    else if(iOrigin == DCLFSFILESEEK_CUR)
    {
        uRelOrigin = REL_SEEK_CUR;
    }
    else if(iOrigin == DCLFSFILESEEK_END)
    {
        uRelOrigin = REL_SEEK_END;
    }
    else
    {
        /*  invalid origin
        */
        return DCLSTAT_FS_BADORIGIN;
    }

    /*  ToDo: Why is uRelOrigin declared as an "unsigned short", but
              being passed to a function that takes an "int"?
    */

    if ( dlp_lseek(hFile->fd, lOffset, uRelOrigin)  <  0 )
        DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
    else
        DclStatus = DCLSTAT_SUCCESS;

    return DclStatus;

  #else
  
    (void)hFile;
    (void)lOffset;
    (void)iOrigin;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileTell()

    Obtain the current position in a file.

    Parameters:
        hFile        - The handle returned by an earlier call to
                       DclOsFsFileOpen.
        pulPosition  - Populated with the current file position.

    Return Value:
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileTell(
    DCLFSFILEHANDLE     hFile,
    D_UINT32           *pulPosition)
{
  #if OSFILE_USE_RELIANCE
    DLP_off_t           lPos;

    DclAssert(hFile);
    DclAssert(pulPosition);

    lPos = dlp_lseek(hFile->fd, 0, REL_SEEK_CUR);
    if( lPos < 0 )
    {
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }
    else
    {
        DclAssert(lPos <= D_INT32_MAX);
        DclAssert(lPos >= D_INT32_MIN);
        *pulPosition = (D_UINT32)lPos;
        return DCLSTAT_SUCCESS;
    }

  #else
  
    (void)hFile;
    DclProductionAssert(pulPosition);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
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
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRead(
    DCLFSFILEHANDLE     hFile,
    void               *pBuffer,
    D_UINT32            ulSize,
    D_UINT32           *pulTransfered)
{
  #if OSFILE_USE_RELIANCE
    DLP_size_t          lResult;

    DclAssert(hFile);
    DclAssert(pulTransfered);

    lResult = dlp_read(hFile->fd, pBuffer, (long)ulSize);
    if( lResult < 0 )
    {
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }
    else
    {
        DclAssert(lResult <= D_INT32_MAX);
        DclAssert(lResult >= D_INT32_MIN);
        *pulTransfered = (D_UINT32)lResult;
        return DCLSTAT_SUCCESS;
    }

  #else

    (void)hFile;
    (void)pBuffer;
    (void)ulSize;
    DclProductionAssert(pulTransfered);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
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
        Returns a DCLSTATUS value indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileWrite(
    DCLFSFILEHANDLE     hFile,
    const void         *pBuffer,
    D_UINT32            ulSize,
    D_UINT32           *pulTransfered)
{
  #if OSFILE_USE_RELIANCE  
    DLP_size_t          lResult;

    DclAssert(hFile);
    DclAssert(pulTransfered);

    lResult = dlp_write(hFile->fd, pBuffer, (long)ulSize);
    if( lResult < 0 )
    {
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }
    else
    {
        DclAssert(lResult <= D_INT32_MAX);
        DclAssert(lResult >= D_INT32_MIN);
        *pulTransfered = (D_UINT32)lResult;
        return DCLSTAT_SUCCESS;
    }

  #else
  
    (void)hFile;
    (void)pBuffer;
    (void)ulSize;
    DclProductionAssert(pulTransfered);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}
