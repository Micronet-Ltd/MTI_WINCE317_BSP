/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This header defines the private DCL API that is generally not made
    public to DCL client products.

                            DCL Header Hierarchy

    DCL interfaces are classified into separate headers based on their use
    and visibility.  Every source code module in DCL includes either dcl.h
    or dcl_osname.h as the top level header (after any OS-specific headers).
 
    dcl.h        - This is the top-level header for DCL code which is built
                   using the standard Datalight build process.  This header
                   autmatically includes dlapi.h, dlapiint.h, and dlapimap.h
                   (among others).

    dcl_osname.h - This is the OS-specific top-level header for DCL code
                   which may be built outside the standard Datalight build
                   process.  This header automatically includes dlapi.h.

                            Noteworthy Sub-Headers

    dlapi.h      - This header contains the DCL interface definitions for
                   those functions which may be used by code which could be
                   built outside the Datalight build process.  This header
                   MUST not depend on any other headers which reside outside
                   the DCL include directory.  It is automatically included
                   by both dcl.h and dcl_osname.h.

    dlapiint.h   - This header contains the DCL interface definitions for
                   those functions which are used by one or more Datalight
                   products, built using the standard Datalight product
                   build process.  It is automatically include by dcl.h.

    dlapimap.h   - This header contains the DCL interface definitions for
                   those functions which, similar to dlapiint.h, are used
                   internally by Datalight code, however may get remapped
                   by OS specific code.  Typically these functions are C
                   library equivalents.  It is automatically include by
                   dcl.h.

    dlapiprv.h  - This header contains the DCL interface definitions for
                  those functions which are private to DCL, and are not
                  intended for use outside DCL, whether by Datalight code
                  or not.  Typically these functions include low-level
                  primitives which are wrapped by higher level functions
                  which should be used.

    A few discrete headers exist for specific subsystems which must be
    explicitly included if they are to be used (generally they are used
    only internally by DCL).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: dlapiprv.h $
    Revision 1.11  2010/01/23 19:31:19Z  garyp
    Added DclAtomic32ExchangeAdd().
    Revision 1.10  2009/12/19 01:57:29Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.9  2009/11/10 23:28:27Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.8  2009/06/25 00:26:30Z  garyp
    Added the interface to the Atomic API.
    Revision 1.7  2009/02/08 01:03:45Z  garyp
    Merged from the v4.0 branch.  Reorganized so the interface are grouped
    by category.
    Revision 1.6  2008/04/19 02:16:42Z  brandont
    Implemented the mini-redirector to allow an alternative set of file
    system services to be implemented and have the file system calls
    directed accordingly.
    Revision 1.5  2008/04/16 22:57:16Z  brandont
    Updated to use the new file system services.
    Revision 1.4  2008/04/08 02:42:33Z  brandont
    Added prototypes for the DclOsFs interface.
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/01/02 13:06:19Z  Garyp
    Documentation update.
    Revision 1.1  2005/10/02 02:10:52Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLAPIPRV_H_INCLUDED
#define DLAPIPRV_H_INCLUDED

#include <dlosmem.h>
#include <dlosmutex.h>
#include <dlossem.h>


/*-------------------------------------------------------------------
    DCL Project Create/Destroy
-------------------------------------------------------------------*/
#define     DclProjectCreate        DCLFUNC(DclProjectCreate)
#define     DclProjectDestroy       DCLFUNC(DclProjectDestroy)

DCLSTATUS   DclProjectCreate(DCLINSTANCEHANDLE hDclInst);
DCLSTATUS   DclProjectDestroy(DCLINSTANCEHANDLE hDclInst);


/*-------------------------------------------------------------------
    Atomic API Services

    These functions are in the "private" API because the preferred
    usage is the implementation in osatomic.c.  (This implementation
    exists solely as a fallback mechanism in the event that the OS
    does not provide primitives to support this.)
-------------------------------------------------------------------*/
#define     DclAtomic32CompareExchange      DCLFUNC(DclAtomic32CompareExchange)
#define     DclAtomic32Exchange             DCLFUNC(DclAtomic32Exchange)
#define     DclAtomic32ExchangeAdd          DCLFUNC(DclAtomic32ExchangeAdd)
#define     DclAtomic32Decrement            DCLFUNC(DclAtomic32Decrement)
#define     DclAtomic32Increment            DCLFUNC(DclAtomic32Increment)
#define     DclAtomicPtrCompareExchange     DCLFUNC(DclAtomicPtrCompareExchange)
#define     DclAtomicPtrExchange            DCLFUNC(DclAtomicPtrExchange)

D_UINT32    DclAtomic32CompareExchange(     D_ATOMIC32 *pulAtomic, D_UINT32 ulCompare, D_UINT32 ulExchange);
D_UINT32    DclAtomic32Exchange(            D_ATOMIC32 *pulAtomic, D_UINT32 ulExchange);
D_UINT32    DclAtomic32ExchangeAdd(         D_ATOMIC32 *pulAtomic, D_UINT32 ulAdd);
D_UINT32    DclAtomic32Decrement(           D_ATOMIC32 *pulAtomic);
D_UINT32    DclAtomic32Increment(           D_ATOMIC32 *pulAtomic);
void *      DclAtomicPtrCompareExchange(    D_ATOMICPTR *pDestination, void *pCompare, void *pExchange);
void *      DclAtomicPtrExchange(           D_ATOMICPTR *pDestination, void *pExchange);

/*  These functions are implemented as macros since they are
    basically just a shorthand way of using the standard APIs.
*/
#define     DclAtomic32Retrieve(pVal)       DclAtomic32CompareExchange(pVal, 0, 0)
#define     DclAtomicPtrRetrieve(pPtr)      DclAtomicPtrCompareExchange(pPtr, NULL, NULL)



/*-------------------------------------------------------------------
    File System OS Services
-------------------------------------------------------------------*/
#define     DclOsFsBecomeFileUser   DCLFUNC(DclOsFsBecomeFileUser)
#define     DclOsFsReleaseFileUser  DCLFUNC(DclOsFsReleaseFileUser)
#define     DclOsFsStat             DCLFUNC(DclOsFsStat)
#define     DclOsFsStatFs           DCLFUNC(DclOsFsStatFs)

#define     DclOsFsFileOpen         DCLFUNC(DclOsFsFileOpen)
#define     DclOsFsFileClose        DCLFUNC(DclOsFsFileClose)
#define     DclOsFsFileEOF          DCLFUNC(DclOsFsFileEOF)
#define     DclOsFsFileFlush        DCLFUNC(DclOsFsFileFlush)
#define     DclOsFsFileSeek         DCLFUNC(DclOsFsFileSeek)
#define     DclOsFsFileTell         DCLFUNC(DclOsFsFileTell)
#define     DclOsFsFileRead         DCLFUNC(DclOsFsFileRead)
#define     DclOsFsFileWrite        DCLFUNC(DclOsFsFileWrite)
#define     DclOsFsFileDelete       DCLFUNC(DclOsFsFileDelete)
#define     DclOsFsFileRename       DCLFUNC(DclOsFsFileRename)

#define     DclOsFsDirOpen          DCLFUNC(DclOsFsDirOpen)
#define     DclOsFsDirRead          DCLFUNC(DclOsFsDirRead)
#define     DclOsFsDirRewind        DCLFUNC(DclOsFsDirRewind)
#define     DclOsFsDirClose         DCLFUNC(DclOsFsDirClose)
#define     DclOsFsDirCreate        DCLFUNC(DclOsFsDirCreate)
#define     DclOsFsDirRemove        DCLFUNC(DclOsFsDirRemove)
#define     DclOsFsDirRename        DCLFUNC(DclOsFsDirRename)
#define     DclOsFsDirSetWorking    DCLFUNC(DclOsFsDirSetWorking)
#define     DclOsFsDirGetWorking    DCLFUNC(DclOsFsDirGetWorking)

#define     DclOsErrToDclStatus     DCLFUNC(DclOsErrToDclStatus)

DCLSTATUS   DclOsFsBecomeFileUser(  void);
DCLSTATUS   DclOsFsReleaseFileUser( void);
DCLSTATUS   DclOsFsStat(            const char *pszPath, DCLFSSTAT *pStat);
DCLSTATUS   DclOsFsStatFs(          const char *pszDevName, DCLFSSTATFS *pStatFs);

DCLSTATUS   DclOsFsFileOpen(        const char *pszPath, const char *pszMode, DCLFSFILEHANDLE *phFile);
DCLSTATUS   DclOsFsFileSeek(        DCLFSFILEHANDLE hFile, D_INT32 lOffset, int iOrigin);
DCLSTATUS   DclOsFsFileTell(        DCLFSFILEHANDLE hFile, D_UINT32 *pulPosition);
DCLSTATUS   DclOsFsFileRead(        DCLFSFILEHANDLE hFile, void *pBuffer, D_UINT32 ulSize, D_UINT32 *pulTransfered);
DCLSTATUS   DclOsFsFileWrite(       DCLFSFILEHANDLE hFile, const void *pBuffer, D_UINT32 ulSize, D_UINT32 *pulTransfered);
DCLSTATUS   DclOsFsFileFlush(       DCLFSFILEHANDLE hFile);
DCLSTATUS   DclOsFsFileClose(       DCLFSFILEHANDLE hFile);
int         DclOsFsFileEOF(         DCLFSFILEHANDLE hFile);
DCLSTATUS   DclOsFsFileDelete(      const char *pszPath);
DCLSTATUS   DclOsFsFileRename(      const char *pszOrigPath, const char *pszNewPath);

DCLSTATUS   DclOsFsDirOpen(         const char *pszPath, DCLFSDIRHANDLE *phDir);
DCLSTATUS   DclOsFsDirRead(         DCLFSDIRHANDLE hDir, char *pszName, unsigned nMaxNameLength, DCLFSSTAT *pStat);
DCLSTATUS   DclOsFsDirRewind(       DCLFSDIRHANDLE hDir);
DCLSTATUS   DclOsFsDirClose(        DCLFSDIRHANDLE hDir);
DCLSTATUS   DclOsFsDirCreate(       const char *pszPath);
DCLSTATUS   DclOsFsDirRemove(       const char *pszPath);
DCLSTATUS   DclOsFsDirRename(       const char *pszOrigPath, const char *pszNewPath);
DCLSTATUS   DclOsFsDirSetWorking(   const char *pszPath);
DCLSTATUS   DclOsFsDirGetWorking(   char *pszPath, unsigned nMaxPathLength);

DCLSTATUS   DclOsErrToDclStatus(    int iSysErr);


/*-------------------------------------------------------------------
    File System Redirector
-------------------------------------------------------------------*/
#if DCLCONF_MINI_REDIRECTOR
    #define DCL_FS_REDIR_MAX_PREFIX  20
    typedef struct
    {
        char        szPrefix[DCL_FS_REDIR_MAX_PREFIX];
        DCLSTATUS   (*FileOpen)         (const char *pszPath, const char *pszMode, DCLFSFILEHANDLE *phDir);
        DCLSTATUS   (*FileClose)        (DCLFSFILEHANDLE hFile);
        int         (*FileEOF)          (DCLFSFILEHANDLE hFile);
        DCLSTATUS   (*FileFlush)        (DCLFSFILEHANDLE hFile);
        DCLSTATUS   (*FileSeek)         (DCLFSFILEHANDLE hFile, D_INT32 lOffset, int iOrgin);
        DCLSTATUS   (*FileTell)         (DCLFSFILEHANDLE hFile, D_UINT32 *pulPosition);
        DCLSTATUS   (*FileRead)         (DCLFSFILEHANDLE hFile, void *pBuffer, D_UINT32 ulSize, D_UINT32 *pulTransfered);
        DCLSTATUS   (*FileWrite)        (DCLFSFILEHANDLE hFile, const void *pBuffer, D_UINT32 ulSize, D_UINT32 *pulTransfered);
        DCLSTATUS   (*DirOpen)          (const char *pszPath, DCLFSDIRHANDLE *phDir);
        DCLSTATUS   (*DirRead)          (DCLFSDIRHANDLE hDir, char *pszName, unsigned nMaxNameLength, DCLFSSTAT *pStat);
        DCLSTATUS   (*DirRewind)        (DCLFSDIRHANDLE hDir);
        DCLSTATUS   (*DirClose)         (DCLFSDIRHANDLE hDir);
        DCLSTATUS   (*DirCreate)        (const char *pszPath);
        DCLSTATUS   (*DirRemove)        (const char *pszPath);
        DCLSTATUS   (*DirSetWorking)    (const char *pszPath);
        DCLSTATUS   (*DirGetWorking)    (char *pszPath, unsigned nMaxPathLength);
        DCLSTATUS   (*DirRename)        (const char *pszOrigPath, const char *pszNewPath);
        DCLSTATUS   (*FileDelete)       (const char *pszPath);
        DCLSTATUS   (*FileRename)       (const char *pszOrigPath, const char *pszNewPath);
        DCLSTATUS   (*Stat)             (const char *pszPath, DCLFSSTAT *pStat);
        DCLSTATUS   (*StatFs)           (const char *pszPath, DCLFSSTATFS *pStatFs);
        DCLSTATUS   (*BecomeFileUser)   (void);
        DCLSTATUS   (*ReleaseFileUser)  (void);
        char*       (*AbsolutePath)     (const char *pszPartialPath, char *pszFullPath);
    } DCL_FS_REDIR_INTERFACE;

    extern DCL_FS_REDIR_INTERFACE * gfsTypeCWD;

    DCL_FS_REDIR_INTERFACE * DclFsGetFileSystemInterface(const char *pszPath, const char **ppszFsPath);
#endif


#endif  /* DLAPIPRV_H_INCLUDED */

