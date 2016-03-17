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

    This module contains functions for manipulating directory trees.

    ToDo: Support for allowing DclFsDirTreeDelete() to delete readonly files
          and directories exists in the documentation and the code framework,
          however it is not fully implemented due to the lack of a DCL FS
          Services function for setting attributes.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlfsdirtree.c $
    Revision 1.3  2010/07/31 19:47:14Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.2  2010/04/12 05:55:15Z  garyp
    Corrected a prototype.
    Revision 1.1  2010/04/11 18:40:36Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>

typedef struct
{
    D_UINT64    ullFiles;
    D_UINT64    ullDirs;
    D_UINT64    ullBytes;
    D_UINT64    ullFileErrors;
    D_UINT64    ullDirErrors;
    unsigned    nVerbosity;
    D_BOOL      fDeleteReadonly;
    D_BOOL      fIgnoreErrors;
} DELTREEINFO;    

typedef struct
{
    char                   *pszPath;
    size_t                  nPathBuffLen;
    char                   *pszName;
    size_t                  nNameBuffLen;
    DCLFSDIRENUMCALLBACK   *pfnCallback;
    void                   *pCallbackContext;
    D_BOOL                  fHiddenSystem;
} ENUMRECURSEINFO;    


static DCLSTATUS EnumRecurse(ENUMRECURSEINFO *pERI);
static DCLSTATUS DelTreeCallback(void *pContext, const char *pszPath, const DCLFSSTAT *pStat);


/*-------------------------------------------------------------------
    Public: DclFsDirTreeDelete()

    Remove the specified directory tree.

    *Note* -- The fDeleteReadonly flag is not yet supported.  It may
              be used, but it is not functional.

    *Note* -- There is no special handling of the current working
              directory.  Attempting to remove it will result in
              undefined behavior.

    Parameters:
        pszDirSpec      - A pointer to the null-terminated directory
                          name to remove.
        nVerbosity      - The standard DCL verbosity level to use.
        fHiddenSystem   - TRUE to process files and directories
                          which have either of the hidden or system
                          attributes set.
        fDeleteReadonly - TRUE to forcibly delete readonly files and
                          directories.
        fIgnoreErrors   - TRUE if errors should be ignored rather than
                          causing the operation to abort.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirTreeDelete(
    const char     *pszDirSpec,
    unsigned        nVerbosity,
    D_BOOL          fHiddenSystem,
    D_BOOL          fDeleteReadonly,
    D_BOOL          fIgnoreErrors)
{
    DELTREEINFO     info = {0};
    DCLSTATUS       dclStat;

    DclAssertReadPtr(pszDirSpec, 0);
    DclAssert(nVerbosity <= DCL_VERBOSE_OBNOXIOUS);

    info.nVerbosity = nVerbosity;
    info.fDeleteReadonly = fDeleteReadonly;
    info.fIgnoreErrors = fIgnoreErrors;

    if(nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        DclPrintf("Deleting directory tree: \"%s\"\n", pszDirSpec);

        if(nVerbosity == DCL_VERBOSE_NORMAL)
            DclPrintf("\nOne moment please...\n");
    }

    /*  Process everything in the directory tree, calling DelTreeCallback()
        for each item.
    */        
    dclStat = DclFsDirTreeEnumerate(pszDirSpec, DelTreeCallback, &info, fHiddenSystem);

    if(dclStat != DCLSTAT_SUCCESS)
    {
        DCLPRINTF(1, ("DclFsDirTreeDelete() \"%s\" failed with status %lX\n", pszDirSpec, dclStat));
        return dclStat;
    }

    dclStat = DclFsDirRemove(pszDirSpec);

    if(dclStat != DCLSTAT_SUCCESS && nVerbosity != DCL_VERBOSE_QUIET)
        DclPrintf("  Dir: %s failed, Status=%lX\n", pszDirSpec, dclStat);
    else if(nVerbosity >= DCL_VERBOSE_LOUD)
        DclPrintf("  Dir: %s\n", pszDirSpec);

    if(dclStat == DCLSTAT_SUCCESS)
        info.ullDirs++;
    else
        info.ullDirErrors++;

    if(nVerbosity >= DCL_VERBOSE_NORMAL)
    {
        char szSizeBuff[16];
        
        DclPrintf("\nDeleted Directory Tree: \"%s\"\n", pszDirSpec);
        DclPrintf("         Consisting of: %7llU directories and\n", VA64BUG(info.ullDirs));
        DclPrintf("                 %14llU files containing\n",      VA64BUG(info.ullFiles));
        DclPrintf("                 %14llU bytes, or %s MB\n",       VA64BUG(info.ullBytes), 
            DclRatio64(szSizeBuff, sizeof(szSizeBuff), info.ullBytes + ((512 * 1024) / 10), 1024 * 1024, 1));

        if(info.ullFileErrors || info.ullDirErrors)
        {
            DclPrintf("%llU files and %llU directories were not deleted due to errors\n", 
            VA64BUG(info.ullFileErrors), VA64BUG(info.ullDirErrors));
        }
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclFsDirTreeEnumerate()

    Enumerate everything in the specified directory tree.

    Parameters:
        pszDirSpec       - A pointer to the null-terminated directory
                           name to enumerate.
        pfnCallback      - The callback function to use.
        pCallbackContext - A context pointer to pass to the callback
                           function.
        fHiddenSystem    - TRUE to process files and directories
                           which have either of the hidden or system
                           attributes set.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclFsDirTreeEnumerate(
    const char             *pszDirSpec,
    DCLFSDIRENUMCALLBACK   *pfnCallback,
    void                   *pCallbackContext,
    D_BOOL                  fHiddenSystem)
{
    DCLSTATUS               dclStat;
    DCLFSSTATFS             statFS;
    ENUMRECURSEINFO         eri = {0};

    DclAssertReadPtr(pszDirSpec, 0);
    DclAssert(pfnCallback);

    dclStat = DclFsStatFs(pszDirSpec, &statFS);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    if(DclStrLen(pszDirSpec) >= statFS.nMaxPathLen)
        return DCLSTAT_FS_PATHTOOLONG;

    /*  The half of the buffer is the "path" buffer, and the 
        second half is the "name" buffer.
    */        
    eri.pszPath = DclMemAlloc((statFS.nMaxPathLen + 1) * 2);
    if(!eri.pszPath)
        return DCLSTAT_OUTOFMEMORY;

    eri.pszName          = eri.pszPath + statFS.nMaxPathLen + 1;
    eri.nPathBuffLen     = statFS.nMaxPathLen + 1;
    eri.nNameBuffLen     = statFS.nMaxPathLen + 1;
    eri.pfnCallback      = pfnCallback;
    eri.pCallbackContext = pCallbackContext;
    eri.fHiddenSystem    = fHiddenSystem;

    DclStrCpy(eri.pszPath, pszDirSpec);

    dclStat = EnumRecurse(&eri);

    DclMemFree(eri.pszPath);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: EnumRecurse()

    Process the specified file or directory.
              
    Parameters:
        pERI - A pointer to the ENUMRECURSEINFO structure to use.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS EnumRecurse(
    ENUMRECURSEINFO    *pERI)
{
    DCLFSDIRHANDLE      hDir;
    size_t              nPathLen;
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pERI->pszPath, pERI->nPathBuffLen);
    DclAssertWritePtr(pERI->pszName, pERI->nNameBuffLen);
    DclAssert(pERI->pfnCallback);

    nPathLen = DclStrLen(pERI->pszPath);

    DclAssert(nPathLen);
    DclAssert(nPathLen < pERI->nPathBuffLen);
    
    dclStat = DclFsDirOpen(pERI->pszPath, &hDir);
    if(dclStat != DCLSTAT_SUCCESS)
	{
        DCLPRINTF(1, ("DclFsDirTree:EnumRecurse() Unable to open directory \"%s\", Status=%lX\n", pERI->pszPath, dclStat));
        return dclStat;
	}
    else
    {
        DCLSTATUS   dclStat2;
        
        do
        {
            DCLFSSTAT   stat;
            size_t      nNameLen;
            
            dclStat = DclFsDirRead(hDir, pERI->pszName, pERI->nNameBuffLen, &stat);
            if(dclStat != DCLSTAT_SUCCESS)
                 break;

            /*  Process HIDDEN and SYSTEM files only if allowed to
            */
            if( ( (stat.ulAttributes & DCLFSATTR_SYSTEM) || 
                  (stat.ulAttributes & DCLFSATTR_HIDDEN) ) && !pERI->fHiddenSystem)
            {
                continue;
            }

            /*  Skip the DOT and DOT DOT entries
            */
            if((stat.ulAttributes & DCLFSATTR_DIRECTORY) && (pERI->pszName[0] == '.'))
            {
                if(pERI->pszName[1] == 0)
                    continue;

                if((pERI->pszName[1] == '.') && (pERI->pszName[2] == 0))
                    continue;
            }

            /*  Build path for this entry and recurse into subdirectories
            */
            pERI->pszPath[nPathLen] = 0;
            nNameLen = DclStrLen(pERI->pszName);
            if(pERI->pszPath[nPathLen - 1] == DCL_PATHSEPCHAR)
            {
                if(pERI->nPathBuffLen < (nPathLen + nNameLen + 1))
                {
                    dclStat = DCLSTAT_FS_PATHTOOLONG;
                    break;
                }
                
                DclSNPrintf(&pERI->pszPath[nPathLen], pERI->nPathBuffLen - nPathLen, "%s", pERI->pszName);
            }
            else
            {
                if(pERI->nPathBuffLen < (nPathLen + nNameLen + 2))
                {
                    dclStat = DCLSTAT_FS_PATHTOOLONG;
                    break;
                }
                
                DclSNPrintf(&pERI->pszPath[nPathLen], pERI->nPathBuffLen - nPathLen, "%c%s", DCL_PATHSEPCHAR, pERI->pszName);
            }
                
            if(stat.ulAttributes & DCLFSATTR_DIRECTORY)
            {
                /*  If we've got a directory, we must recursively process
                    everything under it before calling the callback function.
                */                    
                dclStat = EnumRecurse(pERI);

                if(dclStat != DCLSTAT_SUCCESS)
                    break;
            }

            /*  Call the user's callback function for this file or directory.
            */                
            dclStat = pERI->pfnCallback(pERI->pCallbackContext, pERI->pszPath, &stat);
        } while(dclStat == DCLSTAT_SUCCESS);

        /*  If we broke out of the loop for expected, normal reasons,
            then change the status code to "success".
        */            
        if((dclStat == DCLSTAT_FS_LASTENTRY) || (dclStat == DCLSTAT_FS_NOTFOUND))
            dclStat = DCLSTAT_SUCCESS;

        /*  Close the directory handle and preserve any orginal status
            failure code.
        */            
        dclStat2 = DclFsDirClose(hDir);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = dclStat2;

        /*  Regardless whether we actually return the dclStat2 value or not,
            we've got a serious problem if we could not close the handle.
        */            
        DclAssert(dclStat2 == DCLSTAT_SUCCESS);
    }

    /*  Ensure that the original path value is restored before returning
        out of the recursion.
    */        
    pERI->pszPath[nPathLen] = 0;
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DelTreeCallback()

    Process the specified file or directory.
              
    Parameters:
        pContext - A pointer to the DELTREEINFO structure to use.
        pszPath  - A pointer to the null-terminated file or directory.
        pStat    - A pointer to the DCLFSSTAT structure to use.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
static DCLSTATUS DelTreeCallback(
    void               *pContext,
    const char         *pszPath,
    const DCLFSSTAT    *pStat)
{
    DELTREEINFO        *pInfo = pContext;
    DCLSTATUS           dclStat;
    
    DclAssertWritePtr(pInfo, sizeof(*pInfo));
    DclAssertReadPtr(pszPath, sizeof(*pszPath));
    DclAssertReadPtr(pStat, sizeof(*pStat));

    /*  A subtle difference between the treatment of files and directories,
        aside from the obvious use of different APIs and stats, is that
        directories display a message if nVerbosity is "LOUD" or higher,
        while files display a message only if nVerbosity is "OBNOXIOUS" or
        higher.

        Errors always display messages, so long as nVerbosity is not "QUIET".
    */    

    if(pStat->ulAttributes & DCLFSATTR_DIRECTORY)
    {
        if(pStat->ulAttributes & DCLFSATTR_READONLY)
        {
            if(pInfo->fDeleteReadonly)
            {
                /*  ### REMOVE READONLY ATTRIBUTE HERE ###

                    ToDo: This functionality needs to be implemented -- there
                    is no DCL abstraction for changing attributes.
                */                    
                DclPrintf("Can't delete READONLY directory \"%s\"\n", pszPath);
                pInfo->ullDirErrors++;
                dclStat = DCLSTAT_UNSUPPORTEDFEATURE;
            }
            else
            {
                dclStat = DCLSTAT_FS_DIRREADONLY;
            }

            if(pInfo->nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("  Dir: %s -- READONLY, Status=%lX\n", pszPath, dclStat);
            
            if(pInfo->fIgnoreErrors)
                return DCLSTAT_SUCCESS;
            else
                return dclStat;
        }

        dclStat = DclFsDirRemove(pszPath);

        if(dclStat != DCLSTAT_SUCCESS && pInfo->nVerbosity != DCL_VERBOSE_QUIET)
        {
            DclPrintf("  Dir: %s failed, Status=%lX\n", pszPath, dclStat);
        }
        else if(pInfo->nVerbosity >= DCL_VERBOSE_LOUD)
        {
            DclPrintf("  Dir: %s\n", pszPath);
        }

        if(dclStat == DCLSTAT_SUCCESS)
            pInfo->ullDirs++;
        else
            pInfo->ullDirErrors++;
    }
    else
    {
        if(pStat->ulAttributes & DCLFSATTR_READONLY)
        {
            if(pInfo->fDeleteReadonly)
            {
                /*  ### REMOVE READONLY ATTRIBUTE HERE ###
                
                    ToDo: This functionality needs to be implemented -- there
                    is no DCL abstraction for changing attributes.
                */                    
                DCLPRINTF(1, ("Can't delete READONLY file \"%s\"\n", pszPath));
                pInfo->ullFileErrors++;
                dclStat = DCLSTAT_UNSUPPORTEDFEATURE;
            }
            else
            {
                dclStat = DCLSTAT_FS_READONLY;
            }

            if(pInfo->nVerbosity >= DCL_VERBOSE_NORMAL)
                DclPrintf("          %s is READONLY, Status=%lX\n", pszPath, dclStat);

            if(pInfo->fIgnoreErrors)
                return DCLSTAT_SUCCESS;
            else
                return dclStat;
        }

        dclStat = DclFsFileDelete(pszPath);

        if(dclStat != DCLSTAT_SUCCESS && pInfo->nVerbosity != DCL_VERBOSE_QUIET)
            DclPrintf("       %s failed, Status=%lX\n", pszPath, dclStat);
        else if(pInfo->nVerbosity >= DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("       %s\n", pszPath);

        if(dclStat == DCLSTAT_SUCCESS)
        {
            pInfo->ullFiles++;
            pInfo->ullBytes += pStat->ulSize;
        }
        else
        {
            pInfo->ullFileErrors++;
        }
    }
 
    if(pInfo->fIgnoreErrors)
        return DCLSTAT_SUCCESS;
    else
        return dclStat;
}



