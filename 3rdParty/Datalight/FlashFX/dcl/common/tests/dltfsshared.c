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

    This module contains file system related test code which is shared by
    multiple tests.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltfsshared.c $
    Revision 1.8  2011/03/27 14:50:36Z  garyp
    Tweaked the output format -- no functional changes.
    Revision 1.7  2011/03/24 06:03:04Z  garyp
    Fixed to build with various and sundry arrogant compilers which whine
    about anything they cannot understand.
    Revision 1.6  2011/03/23 23:47:27Z  garyp
    Added DclTestDirAppendToPath() and modified other functions to use it.
    Revision 1.5  2011/03/10 00:24:52Z  daniel.lewis
    Removed attempts to dereference opaque pointers.
    Revision 1.4  2010/07/31 19:47:13Z  garyp
    Updated to work around the "VA64BUG" issue.
    Revision 1.3  2010/05/24 14:55:17Z  garyp
    Changed some stats to use 64-bit counters.  Added support for Get/SetCWD
    primitives.
    Revision 1.2  2010/04/18 20:39:56Z  garyp
    Removed some debugging code.
    Revision 1.1  2010/04/12 03:27:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include "dltfsshared.h"


/*-------------------------------------------------------------------
    Private: DclTestDirCreateAndAppend()

    This function creates a new directory under the specified test
    path, using a directory name generated from the supplied prefix
    and either a random number or the specified suffix number.

    If a random number is used, retry attempts will be made using a
    different random number as necessary.

    The pszPathBuff value will point to the new directory so that
    all subsequent tests will use this directory.  Typically this
    call is paired with a call to DclTestDirDestroyAndRemove().

    Parameters:
        pSTI        - The DCLSHAREDTESTINFO structure to use.
        pFSPrim     - The FSPRIMCONTROL structure to use (may be NULL).
        pszPathBuff - The null-terminated path string to process
        nBuffLen    - The length of the buffer pszPathBuff is stored in.
        pszPrefix   - A pointer to the null-terminated directory
                      prefix to use.
        nSuffix     - The suffix number to use, or UINT_MAX to cause
                      a randomly generated suffix number to be used.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestDirCreateAndAppend(
    DCLSHAREDTESTINFO  *pSTI,
    FSPRIMCONTROL      *pFSPrim,
    char               *pszPathBuff,
    size_t              nBuffLen,
    const char         *pszPrefix,
    unsigned            nSuffix)
{
    #define             RETRY_COUNT (100)
    DCLSTATUS           dclStat = DCLSTAT_SUCCESS;  /* Init'ed for picky compilers */
    unsigned            nRetries = RETRY_COUNT;

    DclAssert(nBuffLen);
    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssertWritePtr(pszPathBuff, nBuffLen);
    DclAssertReadPtr(pszPrefix, 0);

    /*  If we are using a specific suffix number, try one time only,
        it had better work...
    */
    if(nSuffix != UINT_MAX)
        nRetries = 1;

    while(nRetries--)
    {
        unsigned    nSuffixToUse = nSuffix;

        while(nSuffixToUse == UINT_MAX)
        {
            /*  Note that we're explicitly casting this so the maximum number
                is D_UINT16_MAX for backward compatibility reasons.  Should
                never overflow an "unsigned" datatype.
            */                
            nSuffixToUse = (D_UINT16)DclRand64(&pSTI->ullRandomSeed);
        }

        /*  The nBuffLen parameter is adjust -1 to ensure there is always
            room in the buffer to append the final path separator.
        */            
        dclStat = DclTestDirAppendToPath(pszPathBuff, nBuffLen-1, pszPrefix, nSuffixToUse);
        if(dclStat != DCLSTAT_SUCCESS)
            break;

        /*  If we can create the directory, append a path separator
            character and return successfully.
        */
        dclStat = DclTestFSPrimDirCreate(pFSPrim, pszPathBuff);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            DclStrCat(pszPathBuff, DCL_PATHSEPSTR);
            return DCLSTAT_SUCCESS;
        }

        /*  Restore the original path and try again
        */
        (void)DclTestDirRemoveFromPath(pszPathBuff);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestDirDestroyAndRemove()

    This function un-does the work that was done by the function
    DclTestDirCreateAndAppend().  The directory does not need to be
    empty for this function to succeed.

    Note that even if the directory cannot be removed, it will
    still be deleted from the pszPathBuff string.

    Parameters:
        pSTI        - The DCLSHAREDTESTINFO structure to use.
        pFSPrim     - The FSPRIMCONTROL structure to use (may be NULL).
        pszPathBuff - The null-terminated path string to process

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestDirDestroyAndRemove(
    DCLSHAREDTESTINFO  *pSTI,
    FSPRIMCONTROL      *pFSPrim,
    char               *pszPathBuff)
{
    size_t              nPathLen;
    DCLSTATUS           dclStat;

    DclAssertWritePtr(pSTI, sizeof(*pSTI));
    DclAssertWritePtr(pszPathBuff, 0);

    nPathLen = DclStrLen(pszPathBuff);

    DclAssert(pszPathBuff[nPathLen-1] == DCL_PATHSEPCHAR);

    /*  Zap the trailing sep char
    */
    pszPathBuff[nPathLen-1] = 0;

    dclStat = DclTestFSPrimDirRemove(pFSPrim, pszPathBuff); 
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestDirDestroyAndRemove() for \"%s\" returned status %lX\n", pszPathBuff, dclStat);

    /*  Regardless whether the actual tree removed worked or not, 
        remove the directory from the path, and do NOT propagate
        the error.
    */
    return DclTestDirRemoveFromPath(pszPathBuff);
}


/*-------------------------------------------------------------------
    Private: DclTestDirAppendToPath()

    Append a name to the specified path string.  The name consists of
    a prefix string and a suffix number will will be formatted as a
    minimum width of 5, padded with leading zeros if necessary.

    If the existing path points to an empty string, the new path will
    be constructed from the root, with a leading path separator.  The
    existing string may or may not be terminated with a standard path
    separator character.  The resulting path will NOT be terminated
    with a path separator character.

    Typically the use of this function call is paired with a call to
    DclTestDirRemoveFromPath().

    The combination of the prefix and the formatted number may not
    be longer than 127 characters.

    Parameters:
        pszPathBuff - The null-terminated path string to process.
        nBuffLen    - The length of the pszPathBuff buffer.
        pszPrefix   - A pointer to the null-terminated directory
                      prefix to use.
        nSuffix     - The suffix number to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestDirAppendToPath(
    char               *pszPathBuff,
    size_t              nBuffLen,
    const char         *pszPrefix,
    unsigned            nSuffix)
{
    size_t              nPathLen;
    size_t              nWorkLen = 0;
    char                szWorkBuff[128];
    int                 iLen;

    DclAssertWritePtr(pszPathBuff, nBuffLen);
    DclAssert(nBuffLen);
    DclAssertReadPtr(pszPrefix, 0);

    nPathLen = DclStrLen(pszPathBuff);
    DclAssert(nPathLen < nBuffLen);

    if(!nPathLen || pszPathBuff[nPathLen-1] != DCL_PATHSEPCHAR)
    {
        DclStrCpy(szWorkBuff, DCL_PATHSEPSTR);

        nWorkLen = DclStrLen(szWorkBuff);
    }

    iLen = DclSNPrintf(&szWorkBuff[nWorkLen], sizeof(szWorkBuff) - nWorkLen, "%s%05u", pszPrefix, nSuffix);
    
    /*  Buffers are too small
    */
    if(iLen <= 0 || (iLen > (int)(nBuffLen - nPathLen)))
        return DCLSTAT_BUFFERTOOSMALL;

    DclStrCpy(&pszPathBuff[nPathLen], szWorkBuff);
    
    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Private: DclTestDirRemoveFromPath()

    This function parses the path (pszPathBuff) and trims off the
    last directory.  The supplied path may or may not have a trailing
    seperator character.  Regardless whether it does or not, the
    resulting path will have one.

    Parameters:
        pszPathBuff - The null-terminated path string to process

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclTestDirRemoveFromPath(
    char               *pszPathBuff)
{
    size_t              nPathLen;

    DclAssertWritePtr(pszPathBuff, 0);

    nPathLen = DclStrLen(pszPathBuff);

    /*  Ignore the trailing sepchar which will be there in <most> cases
    */
    if(pszPathBuff[nPathLen-1] == DCL_PATHSEPCHAR)
        nPathLen--;

    /*  Scan backwards and truncate after the sep char preceding the
        directory we just removed.
    */
    while(nPathLen)
    {
        if(pszPathBuff[nPathLen-1] == DCL_PATHSEPCHAR)
        {
            pszPathBuff[nPathLen] = 0;
            return DCLSTAT_SUCCESS;
        }

        nPathLen--;
    }

    return DCLSTAT_FAILURE;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimCreate()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimCreate(
    FSPRIMCONTROL      *pPrim,
    const char         *pszName,
    DCLFSFILEHANDLE    *phFile)
{
    DCLSTATUS           dclStat;
    
    DclAssertReadPtr(pszName, 0);
    DclAssertWritePtr(phFile, sizeof(*phFile));

    if(!pPrim)
    {
        dclStat = DclFsFileOpen(pszName, "w+b", phFile);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;

        pPrim->ulCreateCount++;

        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimCreate #%05lU: '%s'\n", pPrim->ulCreateCount, pszName);

        ts = DclTimeStamp();

        dclStat = DclFsFileOpen(pszName, "w+b", phFile);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullCreateTimeUS += ulElapsedUS;
        if(pPrim->ulCreateMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulCreateMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulCreateMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimCreate #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulCreateCount, pPrim->ulCreateMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulCreateMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulCreateErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS && pPrim->nVerbosity > DCL_VERBOSE_NORMAL)
        DclPrintf("DclTestFSPrimCreate() Unable to create file '%s', Status=%lX\n", pszName, dclStat);

    return dclStat;    
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimOpen()

    Description
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimOpen(
    FSPRIMCONTROL      *pPrim,
    const char         *pszName,
    const char         *pszMode,
    DCLFSFILEHANDLE    *phFile)
{
    DCLSTATUS           dclStat;
    
    DclAssertReadPtr(pszName, 0);
    DclAssertWritePtr(phFile, sizeof(*phFile));

    if(!pPrim)
    {
        dclStat = DclFsFileOpen(pszName, pszMode, phFile);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;

        pPrim->ulOpenCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimOpen #%05lU: '%s'\n", pPrim->ulOpenCount, pszName);

        ts = DclTimeStamp();

        dclStat = DclFsFileOpen(pszName, pszMode, phFile);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullOpenTimeUS += ulElapsedUS;
        if(pPrim->ulOpenMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulOpenMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulOpenMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimOpen #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulOpenCount, pPrim->ulOpenMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulOpenMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulOpenErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimOpen() Unable to open file '%s', Status=%lX\n", pszName, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDelete()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDelete(
    FSPRIMCONTROL  *pPrim,
    const char     *pszFile)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszFile, 0);
 
    if(!pPrim)
    {
        dclStat = DclFsFileDelete(pszFile);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDeleteCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDelete #%05lU: '%s'\n", pPrim->ulDeleteCount, pszFile);

        ts = DclTimeStamp();

        dclStat = DclFsFileDelete(pszFile);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDeleteTimeUS += ulElapsedUS;
        if(pPrim->ulDeleteMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDeleteMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDeleteMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDelete #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDeleteCount, pPrim->ulDeleteMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDeleteMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDeleteErrors++;
    }
    
    if((dclStat != DCLSTAT_SUCCESS) && (!pPrim || pPrim->nVerbosity > DCL_VERBOSE_NORMAL))
        DclPrintf("DclTestFSPrimDelete() Unable to delete file '%s', Status=%lX\n", pszFile, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimStat()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimStat(
    FSPRIMCONTROL  *pPrim,
    const char     *pszFile,
    DCLFSSTAT      *pStat)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszFile, 0);
    DclAssertWritePtr(pStat, sizeof(*pStat));
 
    if(!pPrim)
    {
        dclStat = DclFsStat(pszFile, pStat);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulStatCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimStat #%05lU: '%s'\n", pPrim->ulStatCount, pszFile);

        ts = DclTimeStamp();

        dclStat = DclFsStat(pszFile, pStat);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullStatTimeUS += ulElapsedUS;
        if(pPrim->ulStatMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulStatMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulStatMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimStat #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulStatCount, pPrim->ulStatMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulStatMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulStatErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimStat() Unable to stat file '%s', Status=%lX\n", pszFile, dclStat);
 
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimRename()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimRename(
    FSPRIMCONTROL  *pPrim,
    const char     *pszOld,
    const char     *pszNew)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszOld, 0);
    DclAssertReadPtr(pszNew, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsFileRename(pszOld, pszNew);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulRenameCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimRename #%05lU: '%s' to '%s'\n", pPrim->ulRenameCount, pszOld, pszNew);

        ts = DclTimeStamp();

        dclStat = DclFsFileRename(pszOld, pszNew);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullRenameTimeUS += ulElapsedUS;
        if(pPrim->ulRenameMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulRenameMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulRenameMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimRename #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulRenameCount, pPrim->ulRenameMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulRenameMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulRenameErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimRename() Unable to rename '%s' to '%s', Status=%lX\n", pszOld, pszNew, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimClose()

    Close the given file.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimClose(
    FSPRIMCONTROL  *pPrim,
    DCLFSFILEHANDLE hFile)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(hFile, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsFileClose(hFile);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulCloseCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimClose #%05lU: %P\n", pPrim->ulCloseCount, hFile);

        ts = DclTimeStamp();

        dclStat = DclFsFileClose(hFile);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullCloseTimeUS += ulElapsedUS;
        if(pPrim->ulCloseMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulCloseMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulCloseMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimClose #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulCloseCount, pPrim->ulCloseMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulCloseMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulCloseErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimClose() Unable to close file, Status=%lX\n", dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimFlush()

    Description
        Flush the given file.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimFlush(
    FSPRIMCONTROL  *pPrim,
    DCLFSFILEHANDLE hFile)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(hFile, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsFileFlush(hFile);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulFlushCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimFlush #%05lU: %P\n", pPrim->ulFlushCount, hFile);

        ts = DclTimeStamp();

        dclStat = DclFsFileFlush(hFile);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullFlushTimeUS += ulElapsedUS;
        if(pPrim->ulFlushMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulFlushMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulFlushMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimFlush #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulFlushCount, pPrim->ulFlushMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulFlushMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulFlushErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimFlush() Unable to flush file, Status=%lX\n", dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimSeek()

    Description
        Seek to the specified offset of the specified file.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimSeek(
    FSPRIMCONTROL  *pPrim,
    DCLFSFILEHANDLE hFile,
    D_INT32         lOffset,
    int             iOrigin)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(hFile, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsFileSeek(hFile, lOffset, iOrigin);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ullSeekCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimSeek #%05llU: %P, %lD bytes from origin %d\n", VA64BUG(pPrim->ullSeekCount), hFile, lOffset, iOrigin);

        ts = DclTimeStamp();

        dclStat = DclFsFileSeek(hFile, lOffset, iOrigin);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullSeekTimeUS += ulElapsedUS;
        if(pPrim->ulSeekMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulSeekMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulSeekMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimSeek #%05llU: Max was %lU us, new max is %lU us\n", 
                    VA64BUG(pPrim->ullSeekCount), pPrim->ulSeekMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulSeekMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulSeekErrors++;
    }
    
    if((dclStat != DCLSTAT_SUCCESS) && (pPrim->nVerbosity > DCL_VERBOSE_NORMAL))
        DclPrintf("DclTestFSPrimSeek() Unable to seek in file %P, Status=%lX\n", hFile, dclStat);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimRead()

    Description
       All the file read logic in this module is funnelled through
       this function so that we can track cumulative performance
       statistics for the entire test, regardless what options may
       be used.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimRead(
    FSPRIMCONTROL  *pPrim,
    DCLFSFILEHANDLE hFile,
    void           *pBuffer,
    D_UINT32        ulBytes,
    D_UINT32       *pulBytesRead)
{
    DCLSTATUS       dclStat;
    D_UINT32        ulRead;
    
    DclAssertWritePtr(hFile, 0);
    DclAssertWritePtr(pBuffer, 0);
    DclAssert(ulBytes);
  
    if(!pulBytesRead)
        pulBytesRead = &ulRead;

    if(!pPrim)
    {
        dclStat = DclFsFileRead(hFile, pBuffer, ulBytes, pulBytesRead);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ullReadCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimRead #%05llU: %lU bytes from file %P\n", VA64BUG(pPrim->ullReadCount), ulBytes, hFile);

        ts = DclTimeStamp();

        dclStat = DclFsFileRead(hFile, pBuffer, ulBytes, pulBytesRead);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullReadTimeUS += ulElapsedUS;
        if(pPrim->ulReadMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulReadMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulReadMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimRead #%05llU: Max was %lU us, new max is %lU us\n", 
                    VA64BUG(pPrim->ullReadCount), pPrim->ulReadMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulReadMaxTimeUS = ulElapsedUS;
        }
        
        pPrim->ullReadBytes += *pulBytesRead;
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulReadErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimRead() Unable to read from file %P, Status=%lX\n", hFile, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimWrite()

    Description
       All the file write logic in this module is funnelled through
       this function so that we can track cumulative performance
       statistics for the entire test, regardless what options may
       be used.
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimWrite(
    FSPRIMCONTROL  *pPrim,
    DCLFSFILEHANDLE hFile,
    const void     *pBuffer,
    D_UINT32        ulBytes,
    D_UINT32       *pulBytesWritten)
{
    DCLSTATUS       dclStat;
    D_UINT32        ulWritten;
    
    DclAssertWritePtr(hFile, 0);
    DclAssertWritePtr(pBuffer, 0);
/*    DclAssert(ulBytes); */
  
    if(!pulBytesWritten)
        pulBytesWritten = &ulWritten;

    if(!pPrim)
    {
        dclStat = DclFsFileWrite(hFile, pBuffer, ulBytes, pulBytesWritten);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ullWriteCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimWrite #%05llU: %lU bytes to file %P\n", VA64BUG(pPrim->ullWriteCount), ulBytes, hFile);

        ts = DclTimeStamp();

        dclStat = DclFsFileWrite(hFile, pBuffer, ulBytes, pulBytesWritten);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullWriteTimeUS += ulElapsedUS;
        if(pPrim->ulWriteMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulWriteMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulWriteMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimWrite #%05llU: Max was %lU us, new max is %lU us\n", 
                    VA64BUG(pPrim->ullWriteCount), pPrim->ulWriteMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulWriteMaxTimeUS = ulElapsedUS;
        }
        
        pPrim->ullWriteBytes += *pulBytesWritten;
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulWriteErrors++;
    }
/*    
    if(dclStat != DCLSTAT_SUCCESS || *pulBytesWritten != ulBytes)
        DclPrintf("DclTestFSPrimWrite() DclFsFileWrite() to file %P failed with status %lX -- disk full?\n", hFile, dclStat);
*/
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirGetCWD()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirGetCWD(
    FSPRIMCONTROL  *pPrim,
    char           *pBuffer,
    size_t          nBuffLen)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(pBuffer, nBuffLen);
  
    if(!pPrim)
    {
        dclStat = DclFsDirGetWorking(pBuffer, nBuffLen);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirGetCWDCount++;
        
        ts = DclTimeStamp();

        dclStat = DclFsDirGetWorking(pBuffer, nBuffLen);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirGetCWD #%05lU: '%s'\n", pPrim->ulDirGetCWDCount, pBuffer);

        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirGetCWDTimeUS += ulElapsedUS;
        if(pPrim->ulDirGetCWDMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirGetCWDMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirGetCWDMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirGetCWD #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirGetCWDCount, pPrim->ulDirGetCWDMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirGetCWDMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirGetCWDErrors++;
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirGetCWD() Failed, Status=%lX\n", dclStat);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirSetCWD()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirSetCWD(
    FSPRIMCONTROL  *pPrim,
    const char     *pszDir)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszDir, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsDirSetWorking(pszDir);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirSetCWDCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirSetCWD #%05lU: '%s'\n", pPrim->ulDirSetCWDCount, pszDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirSetWorking(pszDir);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirSetCWDTimeUS += ulElapsedUS;
        if(pPrim->ulDirSetCWDMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirSetCWDMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirSetCWDMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirSetCWD #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirSetCWDCount, pPrim->ulDirSetCWDMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirSetCWDMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirSetCWDErrors++;
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirSetCWD() Failed, Status=%lX\n", dclStat);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirCreate()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirCreate(
    FSPRIMCONTROL  *pPrim,
    const char     *pszDir)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszDir, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsDirCreate(pszDir);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirCreateCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirCreate #%05lU: '%s'\n", pPrim->ulDirCreateCount, pszDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirCreate(pszDir);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirCreateTimeUS += ulElapsedUS;
        if(pPrim->ulDirCreateMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirCreateMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirCreateMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirCreate #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirCreateCount, pPrim->ulDirCreateMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirCreateMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirCreateErrors++;
    }
    
    if( (dclStat != DCLSTAT_SUCCESS) &&
        ( (pPrim && pPrim->nVerbosity > DCL_VERBOSE_LOUD) || (D_DEBUG > 1) ) )
    {
        /*  The tests will iterative try to create test directories, 
            retrying if one already exists, therefore this API is 
            expected to occassionally fail.  Only display an error
            message if the verbosity/debug level is high enough.
        */            
        DclPrintf("DclTestFSPrimDirCreate() Unable to create directory '%s', Status=%lX\n", pszDir, dclStat);
    }
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirRemove()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirRemove(
    FSPRIMCONTROL  *pPrim,
    const char     *pszDir)
{
    DCLSTATUS       dclStat;
    
    DclAssertWritePtr(pszDir, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsDirRemove(pszDir);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirRemoveCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirRemove #%05lU: '%s'\n", pPrim->ulDirRemoveCount, pszDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirRemove(pszDir);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirRemoveTimeUS += ulElapsedUS;
        if(pPrim->ulDirRemoveMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirRemoveMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirRemoveMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirRemove #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirRemoveCount, pPrim->ulDirRemoveMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirRemoveMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirRemoveErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirRemove() Unable to remove directory '%s', Status=%lX\n", pszDir, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirOpen()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirOpen(
    FSPRIMCONTROL  *pPrim,
    const char     *pszPath,
    DCLFSDIRHANDLE *phDir)
{
    DCLSTATUS       dclStat;
    
    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(phDir, sizeof(*phDir));
  
    if(!pPrim)
    {
        dclStat = DclFsDirOpen(pszPath, phDir);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirOpenCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirOpen #%05lU: '%s'\n", pPrim->ulDirOpenCount, pszPath);

        ts = DclTimeStamp();

        dclStat = DclFsDirOpen(pszPath, phDir);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirOpenTimeUS += ulElapsedUS;
        if(pPrim->ulDirOpenMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirOpenMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirOpenMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirOpen #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirOpenCount, pPrim->ulDirOpenMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirOpenMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirOpenErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirOpen() Unable to open directory '%s', Status=%lX\n", pszPath, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirRead()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirRead(
    FSPRIMCONTROL  *pPrim,
    DCLFSDIRHANDLE  hDir,
    char           *pszBuffer,
    size_t          nBuffLen,
    DCLFSSTAT      *pStat)
 {
    DCLSTATUS       dclStat;

    DclAssertReadPtr(hDir, 0);
    DclAssert(nBuffLen);    
    DclAssertWritePtr(pszBuffer, nBuffLen);
  
    if(!pPrim)
    {
        dclStat = DclFsDirRead(hDir, pszBuffer, nBuffLen, pStat);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirReadCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirRead #%05lU: %P\n", pPrim->ulDirReadCount, hDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirRead(hDir, pszBuffer, nBuffLen, pStat);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirReadTimeUS += ulElapsedUS;
        if(pPrim->ulDirReadMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirReadMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirReadMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirRead #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirReadCount, pPrim->ulDirReadMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirReadMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirReadErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirRead() Unable to read directory %P, Status=%lX\n", hDir, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirClose()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirClose(
    FSPRIMCONTROL  *pPrim,
    DCLFSDIRHANDLE  hDir)
{
    DCLSTATUS       dclStat;

    DclAssertReadPtr(hDir, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsDirClose(hDir);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirCloseCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirClose #%05lU: %P\n", pPrim->ulDirCloseCount, hDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirClose(hDir);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirCloseTimeUS += ulElapsedUS;
        if(pPrim->ulDirCloseMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirCloseMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirCloseMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirClose #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirCloseCount, pPrim->ulDirCloseMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirCloseMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirCloseErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirClose() Unable to close directory %P, Status=%lX\n", hDir, dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimDirTreeDelete()
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimDirTreeDelete(
    FSPRIMCONTROL  *pPrim,
    const char     *pszDir,
    unsigned        nVerbosity)
{
    DCLSTATUS       dclStat;

    DclAssertReadPtr(pszDir, 0);
  
    if(!pPrim)
    {
        dclStat = DclFsDirTreeDelete(pszDir, nVerbosity, FALSE, FALSE, FALSE);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulDirTreeDeleteCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimDirTreeDel #%05lU: '%s'\n", pPrim->ulDirTreeDeleteCount, pszDir);

        ts = DclTimeStamp();

        dclStat = DclFsDirTreeDelete(pszDir, nVerbosity, FALSE, FALSE, FALSE);
            
        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullDirTreeDeleteTimeUS += ulElapsedUS;
        if(pPrim->ulDirTreeDeleteMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulDirTreeDeleteMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulDirTreeDeleteMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimDirTreeDel #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulDirTreeDeleteCount, pPrim->ulDirTreeDeleteMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulDirTreeDeleteMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulDirTreeDeleteErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimDirTreeDelete() Unable to remove directory tree '%s', Status=%lX\n", pszDir, dclStat);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimStatFS()

    Parameters:
        pPrim   - A pointer to the FSPRIMCONTROL structure to use.
        pStatFS - A pointer to the DCLFSSTATFS structure to fill.

    Return Value:
-------------------------------------------------------------------*/
DCLSTATUS DclTestFSPrimStatFS(
    FSPRIMCONTROL  *pPrim,
    const char     *pszPath,
    DCLFSSTATFS    *pStatFS)
{
    DCLSTATUS       dclStat;

    DclAssertReadPtr(pszPath, 0);
    DclAssertWritePtr(pStatFS, sizeof(*pStatFS));
  
    if(!pPrim)
    {
        dclStat = DclFsStatFs(pszPath, pStatFS);
    }
    else
    {
        DCLTIMESTAMP    ts;
        D_UINT32        ulElapsedUS;
         
        pPrim->ulStatFSCount++;
        
        if(pPrim->nVerbosity > DCL_VERBOSE_OBNOXIOUS)
            DclPrintf("      PrimStatFS #%05lU: '%s'\n", pPrim->ulStatFSCount, pszPath);

        ts = DclTimeStamp();

        dclStat = DclFsStatFs(pszPath, pStatFS);

        ulElapsedUS = DclTimePassedUS(ts);
            
        pPrim->ullTotalTimeUS += ulElapsedUS;
        pPrim->ullStatFSTimeUS += ulElapsedUS;
        if(pPrim->ulStatFSMaxTimeUS < ulElapsedUS)
        {
            if( (pPrim->nVerbosity > DCL_VERBOSE_NORMAL) && 
                (pPrim->ulStatFSMaxTimeUS) &&
                (ulElapsedUS >= pPrim->ulStatFSMaxTimeUS << 1) )
            {
                /*  Display this message only if the verbosity it above normal,
                    this is not the first measurement, and this measurement is
                    2x or more the previous measurement.
                */                    
                DclPrintf("      PrimStatFS #%05lU: Max was %lU us, new max is %lU us\n", 
                    pPrim->ulStatFSCount, pPrim->ulStatFSMaxTimeUS, ulElapsedUS);
            }
            
            pPrim->ulStatFSMaxTimeUS = ulElapsedUS;
        }
        
        if(dclStat != DCLSTAT_SUCCESS)
            pPrim->ulStatFSErrors++;
    }
    
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("DclTestFSPrimStatFS() for '%s' failed with status %lX\n", pszPath, dclStat);
    
    return dclStat;
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimStatsAccumulate()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
void DclTestFSPrimStatsAccumulate(
    FSPRIMCONTROL  *pDest,
    FSPRIMCONTROL  *pSrc)
{
    #define ADDSTAT(nam) ((pDest->nam += pSrc->nam), (pSrc->nam = 0))
    #define MAXSTAT(nam) ((pDest->nam = DCLMAX(pDest->nam, pSrc->nam)), (pSrc->nam = 0))
    
    DclAssertWritePtr(pDest, sizeof(*pDest));
    DclAssertWritePtr(pSrc, sizeof(*pSrc));

    ADDSTAT(ullTotalTimeUS);

    ADDSTAT(ulCreateCount);
    ADDSTAT(ulCreateErrors);
    ADDSTAT(ullCreateTimeUS);
    MAXSTAT(ulCreateMaxTimeUS);
    
    ADDSTAT(ulOpenCount);
    ADDSTAT(ulOpenErrors);
    ADDSTAT(ullOpenTimeUS);
    MAXSTAT(ulOpenMaxTimeUS);
    
    ADDSTAT(ulDeleteCount);
    ADDSTAT(ulDeleteErrors);
    ADDSTAT(ullDeleteTimeUS);
    MAXSTAT(ulDeleteMaxTimeUS);
    
    ADDSTAT(ulStatCount);
    ADDSTAT(ulStatErrors);
    ADDSTAT(ullStatTimeUS);
    MAXSTAT(ulStatMaxTimeUS);
    
    ADDSTAT(ulRenameCount);
    ADDSTAT(ulRenameErrors);
    ADDSTAT(ullRenameTimeUS);
    MAXSTAT(ulRenameMaxTimeUS);
    
    ADDSTAT(ulCloseCount);
    ADDSTAT(ulCloseErrors);
    ADDSTAT(ullCloseTimeUS);
    MAXSTAT(ulCloseMaxTimeUS);
    
    ADDSTAT(ulFlushCount);
    ADDSTAT(ulFlushErrors);
    ADDSTAT(ullFlushTimeUS);
    MAXSTAT(ulFlushMaxTimeUS);
    
    ADDSTAT(ullSeekCount);
    ADDSTAT(ulSeekErrors);
    ADDSTAT(ullSeekTimeUS);
    MAXSTAT(ulSeekMaxTimeUS);
    
    ADDSTAT(ullReadCount);
    ADDSTAT(ulReadErrors);
    ADDSTAT(ullReadTimeUS);
    ADDSTAT(ullReadBytes);
    MAXSTAT(ulReadMaxTimeUS);
    
    ADDSTAT(ullWriteCount);
    ADDSTAT(ulWriteErrors);
    ADDSTAT(ullWriteTimeUS);
    ADDSTAT(ullWriteBytes);
    MAXSTAT(ulWriteMaxTimeUS);
    
    ADDSTAT(ulDirGetCWDCount);
    ADDSTAT(ulDirGetCWDErrors);
    ADDSTAT(ullDirGetCWDTimeUS);
    MAXSTAT(ulDirGetCWDMaxTimeUS);
    
    ADDSTAT(ulDirSetCWDCount);
    ADDSTAT(ulDirSetCWDErrors);
    ADDSTAT(ullDirSetCWDTimeUS);
    MAXSTAT(ulDirSetCWDMaxTimeUS);
    
    ADDSTAT(ulDirCreateCount);
    ADDSTAT(ulDirCreateErrors);
    ADDSTAT(ullDirCreateTimeUS);
    MAXSTAT(ulDirCreateMaxTimeUS);
    
    ADDSTAT(ulDirRemoveCount);
    ADDSTAT(ulDirRemoveErrors);
    ADDSTAT(ullDirRemoveTimeUS);
    MAXSTAT(ulDirRemoveMaxTimeUS);
    
    ADDSTAT(ulDirOpenCount);
    ADDSTAT(ulDirOpenErrors);
    ADDSTAT(ullDirOpenTimeUS);
    MAXSTAT(ulDirOpenMaxTimeUS);
    
    ADDSTAT(ulDirReadCount);
    ADDSTAT(ulDirReadErrors);
    ADDSTAT(ullDirReadTimeUS);
    MAXSTAT(ulDirReadMaxTimeUS);
    
    ADDSTAT(ulDirCloseCount);
    ADDSTAT(ulDirCloseErrors);
    ADDSTAT(ullDirCloseTimeUS);
    MAXSTAT(ulDirCloseMaxTimeUS);
    
    ADDSTAT(ulDirTreeDeleteCount);
    ADDSTAT(ulDirTreeDeleteErrors);
    ADDSTAT(ullDirTreeDeleteTimeUS);
    MAXSTAT(ulDirTreeDeleteMaxTimeUS);
    
    ADDSTAT(ulStatFSCount);
    ADDSTAT(ulStatFSErrors);
    ADDSTAT(ullStatFSTimeUS);
    MAXSTAT(ulStatFSMaxTimeUS);

    return;

    #undef ADDSTAT
    #undef MAXSTAT
}


/*-------------------------------------------------------------------
    Private: DclTestFSPrimStatsDisplay()

    Parameters:
        pPrim   - A pointer to the FSPRIMCONTROL structure to use.

    Return Value:
-------------------------------------------------------------------*/
void DclTestFSPrimStatsDisplay(
    const FSPRIMCONTROL    *pPrim)
{
    D_UINT32                ulTempMS;
    D_UINT64                ullTempKB;
    char                    szTempBuff[24];
    
    DclPrintf("  FS Primitives:    Calls   TotalMS      Average     MaximumUS Errors   TotalMB\n");

    DclPrintf("    Create:    %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulCreateCount, 
        (D_UINT32)((pPrim->ullCreateTimeUS + 500) / 1000),
        pPrim->ulCreateCount ? (D_UINT32)(pPrim->ullCreateTimeUS / pPrim->ulCreateCount) : 0,
        pPrim->ulCreateMaxTimeUS,
        pPrim->ulCreateErrors);

    DclPrintf("    Open:      %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulOpenCount, 
        (D_UINT32)((pPrim->ullOpenTimeUS + 500) / 1000),
        pPrim->ulOpenCount ? (D_UINT32)(pPrim->ullOpenTimeUS / pPrim->ulOpenCount) : 0,
        pPrim->ulOpenMaxTimeUS,
        pPrim->ulOpenErrors);

    ullTempKB = (pPrim->ullReadBytes + 512) / 1024;
    ulTempMS = (D_UINT32)((pPrim->ullReadTimeUS + 500) / 1000);
    
    DclPrintf("    Read:      %10llU %9lU %9lU KB/sec %9lU %6lU %9s\n",
        VA64BUG(pPrim->ullReadCount), ulTempMS,
        ulTempMS >= 500 ? (D_UINT32)DclMulDiv64(ullTempKB, 1, (ulTempMS + 500) / 1000) : 0,
        pPrim->ulReadMaxTimeUS,
        pPrim->ulReadErrors,
        DclRatio64(szTempBuff, sizeof(szTempBuff), pPrim->ullReadBytes + ((512*1024)/10), 1024 * 1024, 1));
 
    ullTempKB = (pPrim->ullWriteBytes + 512) / 1024;
    ulTempMS = (D_UINT32)((pPrim->ullWriteTimeUS + 500) / 1000);
    
    DclPrintf("    Write:     %10llU %9lU %9lU KB/sec %9lU %6lU %9s\n",
        VA64BUG(pPrim->ullWriteCount), ulTempMS,
        ulTempMS >= 500 ? (D_UINT32)DclMulDiv64(ullTempKB, 1, (ulTempMS + 500) / 1000) : 0,
        pPrim->ulWriteMaxTimeUS,
        pPrim->ulWriteErrors,
        DclRatio64(szTempBuff, sizeof(szTempBuff), pPrim->ullWriteBytes + ((512*1024)/10), 1024 * 1024, 1));

    DclPrintf("    Seek:      %10llU %9lU %9lU us/op  %9lU %6lU\n",
        VA64BUG(pPrim->ullSeekCount), 
        (D_UINT32)((pPrim->ullSeekTimeUS + 500) / 1000),
        pPrim->ullSeekCount ? (D_UINT32)(pPrim->ullSeekTimeUS / pPrim->ullSeekCount) : 0,
        pPrim->ulSeekMaxTimeUS,
        pPrim->ulSeekErrors);

    DclPrintf("    Flush:     %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulFlushCount, 
        (D_UINT32)((pPrim->ullFlushTimeUS + 500) / 1000),
        pPrim->ulFlushCount ? (D_UINT32)(pPrim->ullFlushTimeUS / pPrim->ulFlushCount) : 0,
        pPrim->ulFlushMaxTimeUS,
        pPrim->ulFlushErrors);

    DclPrintf("    Close:     %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulCloseCount, 
        (D_UINT32)((pPrim->ullCloseTimeUS + 500) / 1000),
        pPrim->ulCloseCount ? (D_UINT32)(pPrim->ullCloseTimeUS / pPrim->ulCloseCount) : 0,
        pPrim->ulCloseMaxTimeUS,
        pPrim->ulCloseErrors);

    DclPrintf("    Stat:      %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulStatCount, 
        (D_UINT32)((pPrim->ullStatTimeUS + 500) / 1000),
        pPrim->ulStatCount ? (D_UINT32)(pPrim->ullStatTimeUS / pPrim->ulStatCount) : 0,
        pPrim->ulStatMaxTimeUS,
        pPrim->ulStatErrors);

    DclPrintf("    Rename:    %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulRenameCount, 
        (D_UINT32)((pPrim->ullRenameTimeUS + 500) / 1000),
        pPrim->ulRenameCount ? (D_UINT32)(pPrim->ullRenameTimeUS / pPrim->ulRenameCount) : 0,
        pPrim->ulRenameMaxTimeUS,
        pPrim->ulRenameErrors);

    DclPrintf("    Delete:    %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDeleteCount, 
        (D_UINT32)((pPrim->ullDeleteTimeUS + 500) / 1000),
        pPrim->ulDeleteCount ? (D_UINT32)(pPrim->ullDeleteTimeUS / pPrim->ulDeleteCount) : 0,
        pPrim->ulDeleteMaxTimeUS,
        pPrim->ulDeleteErrors);

    DclPrintf("    DirGetCWD: %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirGetCWDCount, 
        (D_UINT32)((pPrim->ullDirGetCWDTimeUS + 500) / 1000),
        pPrim->ulDirGetCWDCount ? (D_UINT32)(pPrim->ullDirGetCWDTimeUS / pPrim->ulDirGetCWDCount) : 0,
        pPrim->ulDirGetCWDMaxTimeUS,
        pPrim->ulDirGetCWDErrors);

    DclPrintf("    DirSetCWD: %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirSetCWDCount, 
        (D_UINT32)((pPrim->ullDirSetCWDTimeUS + 500) / 1000),
        pPrim->ulDirSetCWDCount ? (D_UINT32)(pPrim->ullDirSetCWDTimeUS / pPrim->ulDirSetCWDCount) : 0,
        pPrim->ulDirSetCWDMaxTimeUS,
        pPrim->ulDirSetCWDErrors);

    DclPrintf("    DirCreate: %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirCreateCount, 
        (D_UINT32)((pPrim->ullDirCreateTimeUS + 500) / 1000),
        pPrim->ulDirCreateCount ? (D_UINT32)(pPrim->ullDirCreateTimeUS / pPrim->ulDirCreateCount) : 0,
        pPrim->ulDirCreateMaxTimeUS,
        pPrim->ulDirCreateErrors);

    DclPrintf("    DirRemove: %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirRemoveCount, 
        (D_UINT32)((pPrim->ullDirRemoveTimeUS + 500) / 1000),
        pPrim->ulDirRemoveCount ? (D_UINT32)(pPrim->ullDirRemoveTimeUS / pPrim->ulDirRemoveCount) : 0,
        pPrim->ulDirRemoveMaxTimeUS,
        pPrim->ulDirRemoveErrors);

    DclPrintf("    DirOpen:   %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirOpenCount, 
        (D_UINT32)((pPrim->ullDirOpenTimeUS + 500) / 1000),
        pPrim->ulDirOpenCount ? (D_UINT32)(pPrim->ullDirOpenTimeUS / pPrim->ulDirOpenCount) : 0,
        pPrim->ulDirOpenMaxTimeUS,
        pPrim->ulDirOpenErrors);

    DclPrintf("    DirRead:   %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirReadCount, 
        (D_UINT32)((pPrim->ullDirReadTimeUS + 500) / 1000),
        pPrim->ulDirReadCount ? (D_UINT32)(pPrim->ullDirReadTimeUS / pPrim->ulDirReadCount) : 0,
        pPrim->ulDirReadMaxTimeUS,
        pPrim->ulDirReadErrors);

    DclPrintf("    DirClose:  %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirCloseCount, 
        (D_UINT32)((pPrim->ullDirCloseTimeUS + 500) / 1000),
        pPrim->ulDirCloseCount ? (D_UINT32)(pPrim->ullDirCloseTimeUS / pPrim->ulDirCloseCount) : 0,
        pPrim->ulDirCloseMaxTimeUS,
        pPrim->ulDirCloseErrors);

    DclPrintf("    DirTreeDel:%10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulDirTreeDeleteCount, 
        (D_UINT32)((pPrim->ullDirTreeDeleteTimeUS + 500) / 1000),
        pPrim->ulDirTreeDeleteCount ? (D_UINT32)(pPrim->ullDirTreeDeleteTimeUS / pPrim->ulDirTreeDeleteCount) : 0,
        pPrim->ulDirTreeDeleteMaxTimeUS,
        pPrim->ulDirTreeDeleteErrors);

    DclPrintf("    StatFS:    %10lU %9lU %9lU us/op  %9lU %6lU\n",
        pPrim->ulStatFSCount, 
        (D_UINT32)((pPrim->ullStatFSTimeUS + 500) / 1000),
        pPrim->ulStatFSCount ? (D_UINT32)(pPrim->ullStatFSTimeUS / pPrim->ulStatFSCount) : 0,
        pPrim->ulStatFSMaxTimeUS,
        pPrim->ulStatFSErrors);

    return;
}
