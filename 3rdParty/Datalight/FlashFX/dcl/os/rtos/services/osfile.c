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

    This module contains the default OS Services implementations for the file
    system management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfile.c $
    Revision 1.34  2011/03/24 00:07:55Z  garyp
    Partial fix for Bug 1004 where the target structures for the "stat" and
    "statfs" functions were not being zero'd on entry, therefore leaving some
    fields undefined.  Fixed to remove improper use of DclProductionAssert().
    Revision 1.33  2010/12/08 01:14:24Z  garyp
    Updated to use a renamed symbol.
    Revision 1.32  2009/04/10 20:52:10Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.31  2008/12/06 01:12:00Z  brandont
    Corrected attribute translation in DclOsFsStat.
    Revision 1.30  2008/10/29 23:45:04Z  brandont
    Corrected problems detecting errors.  Updated the statfs call to set
    the device name parameter so that the statistic functions of fsio test
    will work properly.
    Revision 1.29  2008/09/23 22:33:45Z  brandont
    Updated to use the Reliance posix APIs.
    Revision 1.28  2008/06/07 00:59:51Z  brandont
    Removed Unicode support.
    Revision 1.27  2008/05/29 20:15:33Z  garyp
    Merged from the WinMobile branch.
    Revision 1.26.1.2  2008/05/29 20:15:33Z  garyp
    Fixed DclOsFsStatFs() to not overflow the szDeviceName buffer.
    Revision 1.26  2008/04/17 01:07:36Z  brandont
    Updated error code returned from stubbed version of DclOsFsStat.
    Revision 1.25  2008/04/16 23:50:07Z  brandont
    Updated to use the new file system services.
    Revision 1.24  2008/04/05 03:54:47Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.23  2008/04/04 00:17:19Z  brandont
    Changed max path length and max name length members of the DCLFSSTATFS
    structure to be of type unsigned.
    Revision 1.22  2008/04/04 00:11:00Z  brandont
    Changed the uAttributes field of the DCLFSSTAT structure to be 32-bits.
    Revision 1.21  2008/04/03 23:55:46Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.20  2007/12/07 22:50:46Z  brandont
    Updated DclOsFsStatFs to set the device name in the callers structure for
    the DCL os redirector interface.
    Revision 1.19  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.18  2007/08/02 18:53:43Z  timothyj
    Modified initialization of D_TIME to use DCL 64-bit macros.
    Revision 1.17  2007/07/17 20:03:18Z  Garyp
    Updated DclOsFsStatFs() to return a "Device Name" field.  Improved
    parameter validation logic as well as the documentation.
    Revision 1.16  2007/03/15 19:54:08Z  Garyp
    Refactored the modules, placing the directory operations functions in
    osdir.c and the file I/O functions in osfileio.c.  The generic/miscellaneous
    or FS related functions remain in osfile.c.  Eliminated the DclOsFsFileGet/
    PutChar() functions.  Split DclOsRename() into separate functions for
    directories and files (even though they may get redirected to the same
    ultimate function most of the time).
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
    Added implementations for DclOsFsBecomeFileUser and DclOsFsReleaseFileUser.
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

#endif


/*-------------------------------------------------------------------
    Public: DclOsFsFileDelete()

    Delete a file.

    Parameters:
        pszPath - The path of the file to delete.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileDelete(
    const char *pszPath)
{
  #if OSFILE_USE_RELIANCE

    DclAssertReadPtr(pszPath, 0);

    /*  Delete the file
    */
    if (0 == dlp_remove(pszPath))
        return (DCLSTAT_SUCCESS);
    else
        return DclOsErrToDclStatus(DLP_ERRNO_GET());

  #else
  
    DclProductionAssert(pszPath);
    DclProductionError();
    
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsStat()

    Retrieve information about a file or directory.

    Parameters:
        pszName - The name of the object to get info about.
        pStat   - A buffer to receive the Stat information.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStat(
    const char     *pszName,
    DCLFSSTAT      *pStat)
{
  #if OSFILE_USE_RELIANCE

    int             fd;
    DLP_STAT        sRelStat;

    DclAssertReadPtr(pszName, 0);
    DclAssertWritePtr(pStat, sizeof(*pStat));

    DclMemSet(pStat, 0, sizeof(*pStat));
        
    /*  Get the stat info
    */
    fd = dlp_open(pszName, REL_O_RDONLY, REL_IREAD);
    if(fd < 0 )
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    
    if( 0 == dlp_fstat(fd, &sRelStat))
    {
        /*  Convert Posix Stat to DCLFSSTAT
        */
        pStat->ulSize = sRelStat.st_size;
        pStat->ulAttributes = 0;
        if(sRelStat.st_attr & REL_ATTR_ARCHIVE)
        {
            pStat->ulAttributes |= DCLFSATTR_ARCHIVE;
        }
        if(sRelStat.st_attr & REL_ATTR_DIRECTORY)
        {
            pStat->ulAttributes |= DCLFSATTR_DIRECTORY;
        }
        if(sRelStat.st_attr & REL_ATTR_HIDDEN)
        {
            pStat->ulAttributes |= DCLFSATTR_HIDDEN;
        }
        if(sRelStat.st_attr & REL_ATTR_READ_ONLY)
        {
            pStat->ulAttributes |= DCLFSATTR_READONLY;
        }
        if(sRelStat.st_attr & REL_ATTR_SYSTEM)
        {
            pStat->ulAttributes |= DCLFSATTR_SYSTEM;
        }
        if(sRelStat.st_attr & REL_ATTR_VOL_LABEL)
        {
            pStat->ulAttributes |= DCLFSATTR_VOLUME;
        }
        DclUint64AssignUint32(&pStat->tAccess, sRelStat.st_atime);
        DclUint64MulUint32(&pStat->tAccess, 1000000);
        DclUint64AssignUint32(&pStat->tCreation, sRelStat.st_ctime);
        DclUint64MulUint32(&pStat->tCreation, 1000000);
        DclUint64AssignUint32(&pStat->tModify, sRelStat.st_mtime);
        DclUint64MulUint32(&pStat->tModify, 1000000);
        
        dlp_close(fd);
        
        return DCLSTAT_SUCCESS;
    }
    else
    {
        dlp_close(fd);
        
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }
    
  #else
  
    DclProductionAssert(pszName);
    DclProductionAssert(pStat);
    DclProductionError();
    
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsStatFs()

    Retrieve information about a file system.

    Parameters:
        pszPath - A path on the target file system.
        pStatFs - A buffer to receive the StatFs information.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStatFs(
    const char     *pszPath,
    DCLFSSTATFS    *pStatFs)
{
  #if OSFILE_USE_RELIANCE

    int             fd;
    DLP_STATVFS     sRelStatVfs;

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStatFs, sizeof(*pStatFs));

    DclMemSet(pStatFs, 0, sizeof(*pStatFs));

    /*  Get the stat info
    */
    fd = dlp_open(pszPath, REL_O_RDONLY, REL_IREAD);
    if(fd < 0 )
    {
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }

    if( 0 == dlp_fstatvfs(fd, &sRelStatVfs) )
    {
        /*  Convert Posix StatVfs to DCLFSSTATFS
        */
        pStatFs->ulBlockSize = sRelStatVfs.f_bsize;
        pStatFs->ulFreeBlocks = sRelStatVfs.f_bfree;
        pStatFs->ulTotalBlocks = sRelStatVfs.f_blocks;
        pStatFs->nMaxPathLen = REL_RTOS_MAXPATHLEN;
        pStatFs->nMaxNameLen = REL_RTOS_MAXPATHLEN - 3;

        DclStrNCpy(pStatFs->szDeviceName, pszPath, DCLDIMENSIONOF(pStatFs->szDeviceName));
        
        dlp_close(fd);
        
        return DCLSTAT_SUCCESS;
    }
    else
    {
        dlp_close(fd);
        return DclOsErrToDclStatus(DLP_ERRNO_GET());
    }

  #else

    DclProductionAssert(pszPath);
    DclProductionAssert(pStatFs);
    DclProductionError();
    
    return DCLSTAT_FS_BADCMD;

  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileRename()

    Rename or move a file.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRename(
    const char *pszOrigPath,
    const char *pszNewPath)
{
  #if OSFILE_USE_RELIANCE

    DclAssertReadPtr(pszOrigPath, 0);
    DclAssertReadPtr(pszNewPath, 0);

    /*  perform the rename operation
    */
    if ( 0 == dlp_rename(pszOrigPath, pszNewPath))
        return (DCLSTAT_SUCCESS);
    else
        return DclOsErrToDclStatus(DLP_ERRNO_GET());

  #else

    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;

  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsBecomeFileUser()

    Allow this thread to access the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsBecomeFileUser(void)
{
  #if OSFILE_USE_RELIANCE
    int iStatus;

    iStatus = relFs_Become_File_User();
    return DclOsErrToDclStatus(iStatus);

  #else
  
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}


/*-------------------------------------------------------------------
    Public: DclOsFsReleaseFileUser()

    Indicate that this thread is finished accessing the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsReleaseFileUser(void)
{
  #if OSFILE_USE_RELIANCE
    relFs_Release_File_User();
    return DCLSTAT_SUCCESS;
    
  #else
  
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
  #endif
}
