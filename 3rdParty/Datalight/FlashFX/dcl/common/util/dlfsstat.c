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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfsstat.c $
    Revision 1.8  2010/07/29 01:14:01Z  garyp
    Updated to build cleanly with Code Sourcery tools.  Removed the 
    incorrect use of production asserts.  Fixed to properly use debug
    messages.  Code formatting and documentation updated.
    Revision 1.7  2009/05/22 18:04:23Z  jeremys
    Fixed a typo.  DclFsReleaseFileUser was calling DclOsFsBecomeFileUser when
    it should have been calling DclOsFsReleaseFileUser.
    Revision 1.6  2009/05/13 03:20:33Z  brandont
    Changed DclFsBecomeFileUser and DclFsReleaseFileUser to call the
    redirector interface and the DCL service implementation.
    Revision 1.5  2008/04/21 20:01:14Z  brandont
    Implemented the mini-redirector to allow an alternative set of file
    system services to be implemented and have the file system calls
    directed accordingly.
    Revision 1.4  2008/04/19 00:07:34Z  brandont
    Added DclFsFileDelete and DclFsFileRename.
    Revision 1.3  2008/04/09 17:55:32Z  brandont
    Updated to use the new file system services.
    Revision 1.2  2008/04/08 02:42:02Z  brandont
    Updated to use the DclOsFs interfaces.  Added DclFsBecomeFileUser
    and DclFsReleaseFileUser.
    Revision 1.1  2008/04/03 01:16:16Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Public: DclFsStat()

    Retrieve information about a file or directory.

    Parameters:
        pszPath - The path of the object to get info about.
        pStat   - A buffer to receive the Stat information.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsStat(
    const char             *pszPath,
    DCLFSSTAT              *pStat)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS               DclStatus;
    const char             *pszFsPath;
    DCL_FS_REDIR_INTERFACE *pInterface;

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStat, 0);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->Stat)
    {
        DclStatus = pInterface->Stat(pszFsPath, pStat);
    }
    else
    {
        DCLPRINTF(1, ("DclFsStat() Invalid file system command\n"));
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
    
  #else

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStat, 0);
    
    return DclOsFsStat(pszPath, pStat);
    
  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsStatFs()

    Retrieve information about a file system.

    Parameters:
        pszPath - A path on the target file system.
        pStatFs - A buffer to receive the StatFs information.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsStatFs(
    const char             *pszPath,
    DCLFSSTATFS            *pStatFs)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS               DclStatus;
    const char             *pszFsPath;
    DCL_FS_REDIR_INTERFACE *pInterface;

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStatFs, 0);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->StatFs)
    {
        DclStatus = pInterface->StatFs(pszFsPath, pStatFs);
    }
    else
    {
        DCLPRINTF(1, ("DclFsStatFs() Invalid file system command\n"));
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
    
  #else

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStatFs, 0);
    
    return DclOsFsStatFs(pszPath, pStatFs);
    
  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsBecomeFileUser()

    Allow this thread to access the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsBecomeFileUser(void)
{
  #if DCLCONF_MINI_REDIRECTOR
    extern DCL_FS_REDIR_INTERFACE   DclFsRedir1;
    DCL_FS_REDIR_INTERFACE         *pInterface;
    const char                     *pszUnsused;
    DCLSTATUS                       DclStatus = DCLSTAT_SUCCESS;

    pInterface = DclFsGetFileSystemInterface(DclFsRedir1.szPrefix, &pszUnsused);
    if(pInterface && pInterface->BecomeFileUser)
        DclStatus = pInterface->BecomeFileUser();

    if(DclStatus == DCLSTAT_SUCCESS)
        DclStatus = DclOsFsBecomeFileUser();

    return DclStatus;
    
  #else

    return DclOsFsBecomeFileUser();

  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsReleaseFileUser()

    Indicate that this thread is finished accessing the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsReleaseFileUser(void)
{
  #if DCLCONF_MINI_REDIRECTOR
    extern DCL_FS_REDIR_INTERFACE   DclFsRedir1;
    DCL_FS_REDIR_INTERFACE         *pInterface;
    const char                     *pszUnsused;
    DCLSTATUS                       DclStatus = DCLSTAT_SUCCESS;

    pInterface = DclFsGetFileSystemInterface(DclFsRedir1.szPrefix, &pszUnsused);
    if(pInterface && pInterface->ReleaseFileUser)
        DclStatus = pInterface->ReleaseFileUser();
    
    if(DclStatus == DCLSTAT_SUCCESS)
        DclStatus = DclOsFsReleaseFileUser();

    return DclStatus;
    
  #else
  
    return DclOsFsReleaseFileUser();
  
  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileDelete()

    Delete a file.

    Parameters:
        pszPath - The path of the file to delete.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileDelete(
    const char             *pszPath)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS               DclStatus;
    const char             *pszFsPath;
    DCL_FS_REDIR_INTERFACE *pInterface;

    DclAssertReadPtr(pszPath, 0);

    /*  Determine the type of file system to use and direct this
        call to the appropriate service implementation.
    */
    pInterface = DclFsGetFileSystemInterface(pszPath, &pszFsPath);
    if(pInterface && pInterface->FileDelete)
    {
        DclStatus = pInterface->FileDelete(pszFsPath);
    }
    else
    {
        DCLPRINTF(1, ("DclFsFileDelete() Invalid file system command\n"));
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
    
  #else
  
    DclAssertReadPtr(pszPath, 0);
  
    return DclOsFsFileDelete(pszPath);
    
  #endif
}


/*-------------------------------------------------------------------
    Public: DclFsFileRename()

    Rename or move a file.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclFsFileRename(
    const char             *pszOrigPath,
    const char             *pszNewPath)
{
  #if DCLCONF_MINI_REDIRECTOR
    DCLSTATUS               DclStatus;
    const char             *pszSrcFsPath;
    const char             *pszDstFsPath;
    DCL_FS_REDIR_INTERFACE *pSrcInterface;
    DCL_FS_REDIR_INTERFACE *pDstInterface;

    DclAssertReadPtr(pszOrigPath, 0);
    DclAssertReadPtr(pszNewPath, 0);

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
        DCLPRINTF(1, ("DclFsFileRename() Invalid file system command\n"));
        DclStatus = DCLSTAT_FS_BADCMD;
    }

    return DclStatus;
    
  #else
  
    DclAssertReadPtr(pszOrigPath, 0);
    DclAssertReadPtr(pszNewPath, 0);
    
    return DclOsFsFileRename(pszOrigPath, pszNewPath);
    
  #endif
}
