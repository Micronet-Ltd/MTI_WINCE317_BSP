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

    This header contains settings and interface descriptions for file system
    primitives which are shared by multiple tests implemented in this
    directory.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltfsshared.h $
    Revision 1.5  2011/04/26 17:49:19Z  garyp
    Fixed to declare DclTestDirAppendToPath() using DCLFUNC().
    Revision 1.4  2011/03/23 23:47:59Z  garyp
    Added DclTestDirAppendToPath().  Modified arguments for PRIMOPEN().
    Revision 1.3  2010/06/13 21:55:02Z  garyp
    Updated to define function names in terms of DCLFUNC().
    Revision 1.2  2010/06/01 23:59:19Z  garyp
    Changed some stats to use 64-bit counters.  Added support for Get/SetCWD
    primitives.
    Revision 1.1  2010/04/12 03:27:14Z  garyp
    Initial revision
---------------------------------------------------------------------------*/


#ifndef DLTFSSHARED_H_INCLUDED
#define DLTFSSHARED_H_INCLUDED


#include "dltshared.h"

typedef struct
{
    unsigned        nVerbosity;
    D_UINT64        ullTotalTimeUS;
    
    D_UINT32        ulCreateCount;
    D_UINT32        ulCreateErrors;
    D_UINT32        ulCreateMaxTimeUS;
    D_UINT64        ullCreateTimeUS;
    
    D_UINT32        ulOpenCount;
    D_UINT32        ulOpenErrors;
    D_UINT32        ulOpenMaxTimeUS;
    D_UINT64        ullOpenTimeUS;
    
    D_UINT32        ulDeleteCount;
    D_UINT32        ulDeleteErrors;
    D_UINT32        ulDeleteMaxTimeUS;
    D_UINT64        ullDeleteTimeUS;
    
    D_UINT32        ulStatCount;
    D_UINT32        ulStatErrors;
    D_UINT32        ulStatMaxTimeUS;
    D_UINT64        ullStatTimeUS;
    
    D_UINT32        ulRenameCount;
    D_UINT32        ulRenameErrors;
    D_UINT32        ulRenameMaxTimeUS;
    D_UINT64        ullRenameTimeUS;
    
    D_UINT32        ulCloseCount;
    D_UINT32        ulCloseErrors;
    D_UINT32        ulCloseMaxTimeUS;
    D_UINT64        ullCloseTimeUS;
    
    D_UINT32        ulFlushCount;
    D_UINT32        ulFlushErrors;
    D_UINT32        ulFlushMaxTimeUS;
    D_UINT64        ullFlushTimeUS;
    
    D_UINT64        ullSeekCount;
    D_UINT32        ulSeekErrors;
    D_UINT32        ulSeekMaxTimeUS;
    D_UINT64        ullSeekTimeUS;
    
    D_UINT64        ullReadCount;
    D_UINT32        ulReadErrors;
    D_UINT32        ulReadMaxTimeUS;
    D_UINT64        ullReadTimeUS;
    D_UINT64        ullReadBytes;
    
    D_UINT64        ullWriteCount;
    D_UINT32        ulWriteErrors;
    D_UINT32        ulWriteMaxTimeUS;
    D_UINT64        ullWriteTimeUS;
    D_UINT64        ullWriteBytes;
    
    D_UINT32        ulDirGetCWDCount;
    D_UINT32        ulDirGetCWDErrors;
    D_UINT32        ulDirGetCWDMaxTimeUS;
    D_UINT64        ullDirGetCWDTimeUS;
    
    D_UINT32        ulDirSetCWDCount;
    D_UINT32        ulDirSetCWDErrors;
    D_UINT32        ulDirSetCWDMaxTimeUS;
    D_UINT64        ullDirSetCWDTimeUS;
    
    D_UINT32        ulDirCreateCount;
    D_UINT32        ulDirCreateErrors;
    D_UINT32        ulDirCreateMaxTimeUS;
    D_UINT64        ullDirCreateTimeUS;
    
    D_UINT32        ulDirRemoveCount;
    D_UINT32        ulDirRemoveErrors;
    D_UINT32        ulDirRemoveMaxTimeUS;
    D_UINT64        ullDirRemoveTimeUS;
    
    D_UINT32        ulDirOpenCount;
    D_UINT32        ulDirOpenErrors;
    D_UINT32        ulDirOpenMaxTimeUS;
    D_UINT64        ullDirOpenTimeUS;
    
    D_UINT32        ulDirReadCount;
    D_UINT32        ulDirReadErrors;
    D_UINT32        ulDirReadMaxTimeUS;
    D_UINT64        ullDirReadTimeUS;
    
    D_UINT32        ulDirCloseCount;
    D_UINT32        ulDirCloseErrors;
    D_UINT32        ulDirCloseMaxTimeUS;
    D_UINT64        ullDirCloseTimeUS;
    
    D_UINT32        ulDirTreeDeleteCount;
    D_UINT32        ulDirTreeDeleteErrors;
    D_UINT32        ulDirTreeDeleteMaxTimeUS;
    D_UINT64        ullDirTreeDeleteTimeUS;
    
    D_UINT32        ulStatFSCount;
    D_UINT32        ulStatFSErrors;
    D_UINT32        ulStatFSMaxTimeUS;
    D_UINT64        ullStatFSTimeUS;
} FSPRIMCONTROL;


#define     DclTestDirCreateAndAppend       DCLFUNC(DclTestDirCreateAndAppend)
#define     DclTestDirDestroyAndRemove      DCLFUNC(DclTestDirDestroyAndRemove)
#define     DclTestDirAppendToPath          DCLFUNC(DclTestDirAppendToPath)
#define     DclTestDirRemoveFromPath        DCLFUNC(DclTestDirRemoveFromPath)
#define     DclTestFSPrimCreate             DCLFUNC(DclTestFSPrimCreate)
#define     DclTestFSPrimOpen               DCLFUNC(DclTestFSPrimOpen)
#define     DclTestFSPrimDelete             DCLFUNC(DclTestFSPrimDelete)
#define     DclTestFSPrimStat               DCLFUNC(DclTestFSPrimStat)
#define     DclTestFSPrimRename             DCLFUNC(DclTestFSPrimRename)
#define     DclTestFSPrimClose              DCLFUNC(DclTestFSPrimClose)
#define     DclTestFSPrimFlush              DCLFUNC(DclTestFSPrimFlush)
#define     DclTestFSPrimSeek               DCLFUNC(DclTestFSPrimSeek)
#define     DclTestFSPrimRead               DCLFUNC(DclTestFSPrimRead)
#define     DclTestFSPrimWrite              DCLFUNC(DclTestFSPrimWrite)
#define     DclTestFSPrimDirGetCWD          DCLFUNC(DclTestFSPrimDirGetCWD)
#define     DclTestFSPrimDirSetCWD          DCLFUNC(DclTestFSPrimDirSetCWD)
#define     DclTestFSPrimDirCreate          DCLFUNC(DclTestFSPrimDirCreate)
#define     DclTestFSPrimDirRemove          DCLFUNC(DclTestFSPrimDirRemove)
#define     DclTestFSPrimDirOpen            DCLFUNC(DclTestFSPrimDirOpen)
#define     DclTestFSPrimDirRead            DCLFUNC(DclTestFSPrimDirRead)
#define     DclTestFSPrimDirClose           DCLFUNC(DclTestFSPrimDirClose)
#define     DclTestFSPrimDirTreeDelete      DCLFUNC(DclTestFSPrimDirTreeDelete)
#define     DclTestFSPrimStatFS             DCLFUNC(DclTestFSPrimStatFS)
#define     DclTestFSPrimStatsAccumulate    DCLFUNC(DclTestFSPrimStatsAccumulate)
#define     DclTestFSPrimStatsDisplay       DCLFUNC(DclTestFSPrimStatsDisplay)

DCLSTATUS   DclTestDirCreateAndAppend( DCLSHAREDTESTINFO *pSTI, FSPRIMCONTROL *pFSPrim, char *pszPathBuff, size_t nBuffLen, const char *pszPrefix, unsigned nSuffix);
DCLSTATUS   DclTestDirDestroyAndRemove(DCLSHAREDTESTINFO *pSTI, FSPRIMCONTROL *pFSPrim, char *pszPathBuff);
DCLSTATUS   DclTestDirAppendToPath(    char *pszPathBuff, size_t nBuffLen, const char *pszPrefix, unsigned nSuffix);
DCLSTATUS   DclTestDirRemoveFromPath(  char *pszPathBuff);
DCLSTATUS   DclTestFSPrimCreate(         FSPRIMCONTROL *pPrim, const char *pszName, DCLFSFILEHANDLE *phFile);
DCLSTATUS   DclTestFSPrimOpen(           FSPRIMCONTROL *pPrim, const char *pszName, const char *pszMode, DCLFSFILEHANDLE *phFile);
DCLSTATUS   DclTestFSPrimDelete(         FSPRIMCONTROL *pPrim, const char *pszName);
DCLSTATUS   DclTestFSPrimStat(           FSPRIMCONTROL *pPrim, const char *pszName, DCLFSSTAT *pStat);
DCLSTATUS   DclTestFSPrimRename(         FSPRIMCONTROL *pPrim, const char *pszOld, const char *pszNew);
DCLSTATUS   DclTestFSPrimClose(          FSPRIMCONTROL *pPrim, DCLFSFILEHANDLE hFile);
DCLSTATUS   DclTestFSPrimFlush(          FSPRIMCONTROL *pPrim, DCLFSFILEHANDLE hFile);
DCLSTATUS   DclTestFSPrimSeek(           FSPRIMCONTROL *pPrim, DCLFSFILEHANDLE hFile, D_INT32 lOffset, int iOrigin);
DCLSTATUS   DclTestFSPrimRead(           FSPRIMCONTROL *pPrim, DCLFSFILEHANDLE hFile,       void *pBuffer, D_UINT32 ulBytes, D_UINT32 *pulBytesRead);
DCLSTATUS   DclTestFSPrimWrite(          FSPRIMCONTROL *pPrim, DCLFSFILEHANDLE hFile, const void *pBuffer, D_UINT32 ulBytes, D_UINT32 *pulBytesWritten);
DCLSTATUS   DclTestFSPrimDirGetCWD(      FSPRIMCONTROL *pPrim, char *pBuffer, size_t nBuffLen);
DCLSTATUS   DclTestFSPrimDirSetCWD(      FSPRIMCONTROL *pPrim, const char *pszDir);
DCLSTATUS   DclTestFSPrimDirCreate(      FSPRIMCONTROL *pPrim, const char *pszDir);
DCLSTATUS   DclTestFSPrimDirRemove(      FSPRIMCONTROL *pPrim, const char *pszDir);
DCLSTATUS   DclTestFSPrimDirOpen(        FSPRIMCONTROL *pPrim, const char *pszPath, DCLFSDIRHANDLE *phDir);
DCLSTATUS   DclTestFSPrimDirRead(        FSPRIMCONTROL *pPrim, DCLFSDIRHANDLE hDir, char *pszBuffer, size_t nBuffLen, DCLFSSTAT *pStat);
DCLSTATUS   DclTestFSPrimDirClose(       FSPRIMCONTROL *pPrim, DCLFSDIRHANDLE hDir);
DCLSTATUS   DclTestFSPrimDirTreeDelete(  FSPRIMCONTROL *pPrim, const char *pszDir, unsigned nVerbosity);
DCLSTATUS   DclTestFSPrimStatFS(         FSPRIMCONTROL *pPrim, const char *pszDir, DCLFSSTATFS *pStatFS);
void        DclTestFSPrimStatsAccumulate(FSPRIMCONTROL *pDest, FSPRIMCONTROL *pSrc);
void        DclTestFSPrimStatsDisplay(   const FSPRIMCONTROL *pPrim);

#define PRIMCREATE(nam, phandl)         DclTestFSPrimCreate(       (pTI ? &pTI->FSPrim : NULL), (nam), (phandl))
#define PRIMOPEN(nam, mod, phandl)      DclTestFSPrimOpen(         (pTI ? &pTI->FSPrim : NULL), (nam), (mod), (phandl))
#define PRIMDELETE(nam)                 DclTestFSPrimDelete(       (pTI ? &pTI->FSPrim : NULL), (nam))
#define PRIMSTAT(nam, pstat)            DclTestFSPrimStat(         (pTI ? &pTI->FSPrim : NULL), (nam), (pstat))
#define PRIMRENAME(pold, pnew)          DclTestFSPrimRename(       (pTI ? &pTI->FSPrim : NULL), (pold), (pnew))
#define PRIMCLOSE(handl)                DclTestFSPrimClose(        (pTI ? &pTI->FSPrim : NULL), (handl))
#define PRIMFLUSH(handl)                DclTestFSPrimFlush(        (pTI ? &pTI->FSPrim : NULL), (handl))
#define PRIMSEEK(handl, offs, strt)     DclTestFSPrimSeek(         (pTI ? &pTI->FSPrim : NULL), (handl), (offs), (strt))
#define PRIMREAD(handl, ptr, cnt, res)  DclTestFSPrimRead(         (pTI ? &pTI->FSPrim : NULL), (handl), (ptr), (cnt), (res))
#define PRIMWRITE(handl, ptr, cnt, res) DclTestFSPrimWrite(        (pTI ? &pTI->FSPrim : NULL), (handl), (ptr), (cnt), (res))
#define PRIMDIRGETCWD(pbuff, len)       DclTestFSPrimDirGetCWD(    (pTI ? &pTI->FSPrim : NULL), (pbuff), (len))
#define PRIMDIRSETCWD(nam)              DclTestFSPrimDirSetCWD(    (pTI ? &pTI->FSPrim : NULL), (nam))
#define PRIMDIRCREATE(nam)              DclTestFSPrimDirCreate(    (pTI ? &pTI->FSPrim : NULL), (nam))
#define PRIMDIRREMOVE(nam)              DclTestFSPrimDirRemove(    (pTI ? &pTI->FSPrim : NULL), (nam))
#define PRIMDIROPEN(nam, phandl)        DclTestFSPrimDirOpen(      (pTI ? &pTI->FSPrim : NULL), (nam), (phandl))
#define PRIMDIRREAD(handl, p, l, pstat) DclTestFSPrimDirRead(      (pTI ? &pTI->FSPrim : NULL), (handl), (p), (l), (pstat))
#define PRIMDIRCLOSE(handl)             DclTestFSPrimDirClose(     (pTI ? &pTI->FSPrim : NULL), (handl))
#define PRIMDIRTREEDEL(nam, verb)       DclTestFSPrimDirTreeDelete((pTI ? &pTI->FSPrim : NULL), (nam), (verb))
#define PRIMSTATFS(nam, ptr)            DclTestFSPrimStatFS(       (pTI ? &pTI->FSPrim : NULL), (nam), (ptr))



#endif  /* DLTFSSHARED_H_INCLUDED */

