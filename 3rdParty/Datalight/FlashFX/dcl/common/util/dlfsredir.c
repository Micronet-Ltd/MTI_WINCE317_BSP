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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfsredir.c $
    Revision 1.2  2011/03/07 16:19:04Z  garyp
    Documentation updated.  Added debug code.  No functional changes.
    Revision 1.1  2008/04/21 20:53:46Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


#if DCLCONF_MINI_REDIRECTOR

static DCL_FS_REDIR_INTERFACE DclDefaultServices = 
{
    "",
    DclOsFsFileOpen,
    DclOsFsFileClose,
    DclOsFsFileEOF,
    DclOsFsFileFlush,
    DclOsFsFileSeek,
    DclOsFsFileTell,
    DclOsFsFileRead,
    DclOsFsFileWrite,
    DclOsFsDirOpen,
    DclOsFsDirRead,
    DclOsFsDirRewind,
    DclOsFsDirClose,
    DclOsFsDirCreate,
    DclOsFsDirRemove,
    DclOsFsDirSetWorking,
    DclOsFsDirGetWorking,
    DclOsFsDirRename,
    DclOsFsFileDelete,
    DclOsFsFileRename,
    DclOsFsStat,
    DclOsFsStatFs,
    DclOsFsBecomeFileUser,
    DclOsFsReleaseFileUser,
    NULL
};

DCL_FS_REDIR_INTERFACE * gfsTypeCWD = &DclDefaultServices;

extern DCL_FS_REDIR_INTERFACE DclFsRedir1;


/*-------------------------------------------------------------------
    Protected: DclFsGetFileSystemInterface()

    Determine the type of file system to use based on the path.  If
    the path includes the host token string, the call will be passed
    to the "host" file system (win32).  If the path is relative and
    does not contain a drive specifier, the CWD file system type is
    used.

    If the host token string is found in the path, it is removed in
    the caller's buffer.

    Parameters:
        pszPath    - The path to parse.
        ppszFsPath - The location in which to store the path pointer.
                    
    Return Value:
        Returns the type of file system specified by the path.
-------------------------------------------------------------------*/
DCL_FS_REDIR_INTERFACE * DclFsGetFileSystemInterface(
    const char     *pszPath,
    const char    **ppszFsPath)
{
    size_t          nPrefixLength;

    DclAssertReadPtr(pszPath, 0);
    DclAssertReadPtr(ppszFsPath, sizeof(*ppszFsPath));

    nPrefixLength = DclStrLen(DclFsRedir1.szPrefix);
    if(DclStrNICmp(pszPath, DclFsRedir1.szPrefix, nPrefixLength) == 0)
    {
        *ppszFsPath = &pszPath[nPrefixLength];
        return &DclFsRedir1;
    }
    else if((pszPath[0] == DCL_PATHSEPCHAR) || ((pszPath[0]) && (pszPath[1] == ':')))
    {
        *ppszFsPath = pszPath;
        return &DclDefaultServices;
    }
    else
    {
        *ppszFsPath = pszPath;
        return gfsTypeCWD;
    }
}



#endif  /* #if DCLCONF_MINI_REDIRECTOR */
