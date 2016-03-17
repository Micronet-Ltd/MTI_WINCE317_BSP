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

    This module contains the default OS Services implementations of the file
    system directory management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    the make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdir.c $
    Revision 1.10  2010/12/08 01:14:24Z  garyp
    Updated to use a renamed symbol.
    Revision 1.9  2009/04/10 20:52:09Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.8  2008/09/23 22:30:53Z  brandont
    Updated to use the Reliance posix APIs.
    Revision 1.7  2008/06/07 00:58:56Z  brandont
    Removed Unicode support.
    Revision 1.6  2008/04/16 23:53:57Z  brandont
    Updated to use the new file system services.
    Revision 1.5  2008/04/05 03:56:36Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.4  2008/04/03 23:30:19Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.3  2008/03/19 20:27:12Z  Garyp
    Documentation updates.
    Revision 1.2  2007/11/03 23:31:30Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/03/15 22:26:22Z  Garyp
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

struct OsDirHandle 
{
    DLP_DIR    *RelDir;
    char        szPath[REL_RTOS_MAXPATHLEN];
    unsigned    nPathLength;
};

#endif


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
    const char * pszPath,
    DCLFSDIRHANDLE * phDir)
{
#if OSFILE_USE_RELIANCE

    DCLFSDIRHANDLE hDir;
    DCLSTATUS DclStatus;


    DclProductionAssert(pszPath);
    DclProductionAssert(phDir);


    /*  Allocate a dir structure.
    */
    hDir = DclMemAlloc(sizeof(*hDir));
    if(!hDir)
    {
        DclStatus = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        hDir->nPathLength = DclStrLen(pszPath);
        if((hDir->nPathLength + 1) >= DCLDIMENSIONOF(hDir->szPath))
        {
            DclStatus = DCLSTAT_FS_NAMELENGTH;
        }
        else
        {
            DclStrCpy(hDir->szPath, pszPath);
            if((!hDir->nPathLength) || (hDir->szPath[hDir->nPathLength] != DCL_PATHSEPCHAR))
            {
                DclStrCat(hDir->szPath, DCL_PATHSEPSTR);
                hDir->nPathLength++;
            }


            /*  Open the directory.
            */
            hDir->RelDir = dlp_opendir(pszPath);

            if(hDir->RelDir)
            {
                DclStatus = DCLSTAT_SUCCESS;
            }
            else
            {
                DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
            }
        }
    }


    if(DclStatus)
    {
        if(hDir)
        {
            DclMemFree(hDir);
            hDir = (void *)0;
        }
    }
    *phDir = hDir;
    return DclStatus;

#else
    DclProductionAssert(pszPath);
    DclProductionAssert(phDir);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    DCLFSDIRHANDLE hDir,
    char * pszName,
    unsigned nMaxNameLength,
    DCLFSSTAT * pStat)
{
#if OSFILE_USE_RELIANCE

    DLP_DIRENT  *pDirEnt;
    DCLSTATUS   DclStatus;
    unsigned    nLength;


    DclProductionAssert(hDir);
    DclProductionAssert(pszName);


    /*  Read the directory.
    */
    pDirEnt = dlp_readdir(hDir->RelDir);
    if (!pDirEnt)
    {
        DclStatus = DclOsErrToDclStatus(DLP_ERRNO_GET());
    } else {
        DclStatus = DCLSTAT_SUCCESS;
    }

    if(!DclStatus)
    {
        nLength = DclStrLen(pDirEnt->d_name) + 1;
        if(nLength > nMaxNameLength)
        {
            DclStatus = DCLSTAT_FS_NAMELENGTH;
        }
        else
        {
            DclStrCpy(pszName, pDirEnt->d_name);
        }


        if((!DclStatus) && (pStat))
        {
            if((nLength + hDir->nPathLength) >= DCLDIMENSIONOF(hDir->szPath))
            {
                DclStatus = DCLSTAT_FS_NAMELENGTH;
            }
            else
            {
                DclStrCpy(&hDir->szPath[hDir->nPathLength], pszName);
                DclStatus = DclOsFsStat(hDir->szPath, pStat);
                hDir->szPath[hDir->nPathLength] = 0;
            }
        }
    }

    return DclStatus;
#else
    (void)hDir;
    (void)pStat;
    (void)nMaxNameLength;
    DclProductionAssert(pszName);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
#if OSFILE_USE_RELIANCE

    DclProductionAssert(hDir);


    /*  Close the directory
    */
    dlp_rewinddir(hDir->RelDir);
    return (DCLSTAT_SUCCESS);

#else
    (void)hDir;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
#if OSFILE_USE_RELIANCE

    DclProductionAssert(hDir);

    /*  Close the directory
    */
    if( 0 == dlp_closedir(hDir->RelDir) )
    {
        DclMemFree(hDir);
        return (DCLSTAT_SUCCESS);
    }
    return DclOsErrToDclStatus(DLP_ERRNO_GET());

#else
    (void)hDir;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    const char * pszPath)
{
#if OSFILE_USE_RELIANCE

    DclProductionAssert(pszPath);

    /*  Create the directory.
    */
        if (0 == dlp_mkdir(pszPath))
            return (DCLSTAT_SUCCESS);
        else
            return (DclOsErrToDclStatus(DLP_ERRNO_GET()));

#else
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    const char * pszPath)
{
#if OSFILE_USE_RELIANCE

    DclProductionAssert(pszPath);

    /*  Remove the directory.
    */
    if (0 == dlp_rmdir(pszPath))
        return (DCLSTAT_SUCCESS);
    else
        return (DclOsErrToDclStatus(DLP_ERRNO_GET()));

#else
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    const char * pszOrigPath,
    const char * pszNewPath)
{
#if OSFILE_USE_RELIANCE
    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);

    /*  Directories are renamed the same way as files, so just pass-through
        to the file handler.
    */
    return (DclOsFsFileRename(pszOrigPath, pszNewPath));

#else
    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    const char * pszPath)
{
#if OSFILE_USE_RELIANCE
    int             iStatus;
    unsigned short  uDrive;


    DclProductionAssert(pszPath);

    /*  if the path includes a drive specifier, set the default drive.
    */
    if(pszPath[1] == ':')
    {
        uDrive = (unsigned short)DclToUpper(pszPath[0]) - 'A';
        iStatus = relFs_Set_Default_Drive(uDrive);
        if(iStatus != REL_SUCCESS)
        {
            return DclOsErrToDclStatus(iStatus);
        }
    }

    /*  Set the current directory.
    */
    iStatus = relFs_Set_Current_Dir(pszPath);
    return DclOsErrToDclStatus(iStatus);

#else
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
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
    char * pszPath,
    unsigned nMaxPathLength)
{
#if OSFILE_USE_RELIANCE
    int iStatus;


    (void)nMaxPathLength;
    DclProductionAssert(pszPath);

    /*  Get the current directory.
    */
    pszPath[0] = 'A' + relFs_Get_Default_Drive();
    pszPath[1] = ':';
    pszPath[2] = 0;
    iStatus = relFs_Current_Dir(pszPath, &pszPath[2]);
    return DclOsErrToDclStatus(iStatus);

#else
    (void)nMaxPathLength;
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
#endif
}
