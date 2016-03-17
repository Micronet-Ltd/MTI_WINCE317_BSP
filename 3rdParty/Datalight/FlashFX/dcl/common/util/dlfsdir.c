/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfsdir.c $
    Revision 1.5  2008/04/22 22:01:27Z  brandont
    Conditioned the OsDirHandle structure to only be used if the mini
    redirector is enabled.
    Revision 1.4  2008/04/21 20:01:14Z  brandont
    Implemented the mini-redirector to allow an alternative set of file
    system services to be implemented and have the file system calls
    directed accordingly.
    Revision 1.3  2008/04/16 23:34:36Z  brandont
    Updated to use the new file system services.
    Revision 1.2  2008/04/08 02:39:50Z  brandont
    Updated to use the DclOsFs interfaces.
    Revision 1.1  2008/04/04 01:22:54Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


#if DCLCONF_MINI_REDIRECTOR
struct OsDirHandle {
    DCL_FS_REDIR_INTERFACE * pInterface;
    DCLFSDIRHANDLE hDir;
};
#endif


/*-------------------------------------------------------------------
    Public: DclFsDirOpen

    Open a directory for reading.

    Parameters
        pszPath - The path to the directory.
        phDir   - Populated with a DCLFSDIRHANDLE used to access
                  the directory.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.

    See:
        DclFsDirRead
        DclFsDirRewind
        DclFsDirClose
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirOpen(
    const char * pszPath,
    DCLFSDIRHANDLE * phDir)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLFSDIRHANDLE hDir;
    const char * szFsPath;
    DCLSTATUS DclStatus;


    DclProductionAssert(pszPath);
    DclProductionAssert(phDir);


    hDir = DclMemAllocZero(sizeof(*hDir));
    if(!hDir)
    {
        DclStatus = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        /*  Determine the type of file system to use and direct this
            call to the appropriate service implementation.
        */
        hDir->pInterface = DclFsGetFileSystemInterface(pszPath, &szFsPath);
        if(hDir->pInterface && hDir->pInterface->DirOpen)
        {
            DclStatus = hDir->pInterface->DirOpen(szFsPath, &hDir->hDir);
        }
        else
        {
            DclPrintf("DclFsDirOpen(): Invalid file system command\n");
            DclStatus = DCLSTAT_FS_BADCMD;
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
    return DclOsFsDirOpen(pszPath, phDir);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirRead

    Read the next entry in a directory.

    Parameters:
        hDir           - The handle returned by an earlier call to
                         DclFsDirOpen.
        pszName        - Populated with the next name in the directory.
        uMaxNameLength - Size of pszName including the NULL terminator.
        pStat          - Populated with information about the entry.
                         This can be NULL if the information is not
                         desired.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirRead(
    DCLFSDIRHANDLE hDir,
    char * pszName,
    unsigned nMaxNameLength,
    DCLFSSTAT * pStat)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;


    DclProductionAssert(hDir);
    DclProductionAssert(pszName);
    if(hDir->pInterface && hDir->pInterface->DirRead)
    {
        DclStatus = hDir->pInterface->DirRead(hDir->hDir, pszName, nMaxNameLength, pStat);
    }
    else
    {
        DclPrintf("DclFsDirRead(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszName);
    return DclOsFsDirRead(hDir, pszName, nMaxNameLength, pStat);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirRewind

    Reset the position within an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclFsDirOpen.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirRewind(
    DCLFSDIRHANDLE hDir)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;


    DclProductionAssert(hDir);
    if(hDir->pInterface && hDir->pInterface->DirRewind)
    {
        DclStatus = hDir->pInterface->DirRewind(hDir->hDir);
    }
    else
    {
        DclPrintf("DclFsDirRewind(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }


    return DclStatus;
#else
    return DclOsFsDirRewind(hDir);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirClose

    Close an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclOsFsDirOpen.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirClose(
    DCLFSDIRHANDLE hDir)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;


    DclProductionAssert(hDir);
    if(hDir->pInterface && hDir->pInterface->DirClose)
    {
        DclStatus = hDir->pInterface->DirClose(hDir->hDir);
    }
    else
    {
        DclPrintf("DclFsDirClose(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    if(!DclStatus)
    {
        DclMemFree(hDir);
    }
    return DclStatus;
#else
    return DclOsFsDirClose(hDir);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirCreate

    Create a directory.

    Parameters:
        pszPath - The path to the directory to create.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.

    See:
        DclFsDirRemove
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirCreate(
    const char * pszPath)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;
    const char * pszFsPath;
    DCL_FS_REDIR_INTERFACE * pInterface;


    DclProductionAssert(pszPath);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->DirCreate)
    {
        DclStatus = pInterface->DirCreate(pszFsPath);
    }
    else
    {
        DclPrintf("DclFsDirCreate(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszPath);
    return DclOsFsDirCreate(pszPath);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirRemove

    Remove a directory.

    Parameters:
        pszPath - The path to the directory to remove.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.

    See:
        DclFsDirCreate
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirRemove(
    const char * pszPath)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;
    const char * pszFsPath;
    DCL_FS_REDIR_INTERFACE * pInterface;


    DclProductionAssert(pszPath);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->DirRemove)
    {
        DclStatus = pInterface->DirRemove(pszFsPath);
    }
    else
    {
        DclPrintf("DclFsDirRemove(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszPath);
    return DclOsFsDirRemove(pszPath);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirRename

    Rename or move a directory.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirRename(
    const char * pszOrigPath,
    const char * pszNewPath)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;
    const char * pszSrcFsPath;
    const char * pszDstFsPath;
    DCL_FS_REDIR_INTERFACE * pSrcInterface;
    DCL_FS_REDIR_INTERFACE * pDstInterface;


    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pSrcInterface = DclFsGetFileSystemInterface(pszOrigPath, &pszSrcFsPath);
    pDstInterface = DclFsGetFileSystemInterface(pszNewPath, &pszDstFsPath);
    if(pSrcInterface && pSrcInterface->DirRename && (pSrcInterface == pDstInterface))
    {
        DclStatus = pSrcInterface->DirRename(pszSrcFsPath, pszDstFsPath);
    }
    else
    {
        DclPrintf("DclFsDirRename(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);
    return DclOsFsDirRename(pszOrigPath, pszNewPath);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirSetWorking

    Set the current working directory.

    Parameters:
        pszPath - The path of the directory to set as the CWD.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.

    See:
        DclFsDirGetWorking
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirSetWorking(
    const char * pszPath)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;
    const char * pszFsPath;
    DCL_FS_REDIR_INTERFACE * pInterface;


    DclProductionAssert(pszPath);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->DirSetWorking)
    {
        DclStatus = pInterface->DirSetWorking(pszFsPath);
        if(!DclStatus)
        {
            /*  Remember the type of file system used to set the CWD so we can
                handle it correctly when requested to retrieve the CWD.
            */
            gfsTypeCWD = pInterface;
        }
    }
    else
    {
        DclPrintf("DclFsDirSetWorking(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszPath);
    return DclOsFsDirSetWorking(pszPath);
#endif
}


/*-------------------------------------------------------------------
    Public: DclFsDirGetWorking

    Retrieve the current working directory.

    Parameters:
        pszPath        - A buffer to receive the current working
                         directory.
        uMaxPathLength - Size of buffer including the NULL terminator.

    Return:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.

    NOTE: If this functionality is not supported, this function may
          return an error code (without asserting), and things like
          the shell will take note of that and avoid using either of
          the "Get/SetCWD" functions.

    See:
        DclFsDirSetWorking
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirGetWorking(
    char * pszPath,
    unsigned nMaxPathLength)
{
#if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS DclStatus;
    DCL_FS_REDIR_INTERFACE * pInterface;
    size_t nLength;


    DclProductionAssert(pszPath);
    pInterface = gfsTypeCWD;
    if(pInterface && pInterface->DirGetWorking)
    {
        DclProductionAssert(DclStrLen(pInterface->szPrefix) < nMaxPathLength);
        DclStrCpy(pszPath, pInterface->szPrefix);
        nLength = DclStrLen(pszPath);
        DclStatus = pInterface->DirGetWorking(&pszPath[nLength], nMaxPathLength - nLength);
    }
    else
    {
        DclPrintf("DclFsDirGetWorking(): Invalid file system command\n");
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
#else
    DclProductionAssert(pszPath);
    return DclOsFsDirGetWorking(pszPath, nMaxPathLength);
#endif
}
