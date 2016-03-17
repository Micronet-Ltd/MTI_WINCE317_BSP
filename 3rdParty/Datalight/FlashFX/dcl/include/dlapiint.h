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

    This header defines the internal API to the Datalight Common Libraries.

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
    $Log: dlapiint.h $
    Revision 1.95  2011/03/23 23:25:18Z  garyp
    Added a prototype for DclSortInsertion32().
    Revision 1.94  2011/03/02 02:04:50Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.93  2010/12/11 20:05:11Z  garyp
    Added synchronization validation functions.
    Revision 1.92  2010/10/18 03:20:49Z  garyp
    Added bitmap range functions.
    Revision 1.91  2010/09/18 02:20:40Z  garyp
    Added DclSleepLock().
    Revision 1.90  2010/08/29 19:49:02Z  garyp
    Added DclTimeFormatUS().
    Revision 1.89  2010/05/04 20:00:37Z  billr
    Refactor headers for Linux build process.
    Revision 1.88  2010/04/18 21:13:34Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.87  2010/04/12 03:17:34Z  garyp
    Added prototypes for directory tree manipulation functions.
    Revision 1.86  2010/01/07 02:34:38Z  garyp
    Added DclScaleItems().
    Revision 1.85  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.84  2009/12/02 21:54:00Z  garyp
    Added prototypes.
    Revision 1.83  2009/11/10 23:29:46Z  garyp
    Made all the memory tracking stuff dependent on DCLCONF_MEMORYTRACKING.
    Revision 1.82  2009/11/05 02:24:42Z  garyp
    Added DclRatio64().  Updated the atomic "read" functions to simply
    dereference the pointer, and to conditionally define the macros so
    that they may be overridden in OS or project code.
    Revision 1.81  2009/10/13 00:24:38Z  keithg
    Added random service initialization and 64 bit prototypes.
    Revision 1.80  2009/10/06 17:44:34Z  garyp
    Added/refactored a number of ECC related functions.  Moved a few items to
    dlapi.h, which must be visible to stuff built outside the DL build process.
    Revision 1.79  2009/10/03 00:36:52Z  garyp
    Added DclEccCorrect().  Renamed DclEccGenerate() to DclEccCalculate()
    for consistency.
    Revision 1.78  2009/09/24 17:29:42Z  garyp
    Added prototypes.
    Revision 1.77  2009/09/14 22:26:35Z  garyp
    Added support for DclMulDiv().
    Revision 1.76  2009/09/08 21:57:44Z  garyp
    Added support for DclSizeToULKB().
    Revision 1.75  2009/07/16 16:05:57Z  keithg
    Migrated network prototypes and structures from DLAPIINT.H into DLNET.H
    Revision 1.74  2009/07/15 17:33:57Z  keithg
    Added FTP services API and types.
    Revision 1.73  2009/07/15 06:35:14Z  keithg
    Corrected socket address type naming.
    Revision 1.72  2009/07/15 05:11:11Z  keithg
    Added prototypes and structures for the DCL network sockets abstraction.
    Revision 1.71  2009/06/29 02:24:49Z  garyp
    Added a missing   DCLFUNC definition for DclScaleBytes().
    Revision 1.70  2009/06/28 22:11:14Z  garyp
    Re-added ulMaxFileSize to the DLFSSTATFS structure, which got lost
    in the merge.
    Revision 1.69  2009/06/25 22:49:22Z  garyp
    Numerous updates pertaining to finalizing the merge from the v4.0 branch.
    Revision 1.68  2009/05/20 07:38:12Z  keithg
    Added DclUtf8StrICmp() prototype.
    Revision 1.67  2009/05/15 02:00:12Z  keithg
    Removed obsolete unicode function, renamed Unicode tolower.
    Revision 1.66  2009/05/14 21:56:59Z  keithg
    Updated to reflect new names of the UTF-8 functions.
    Revision 1.65  2009/05/09 01:30:48Z  brandont
    Added DCL_UTF8_PARTIAL_BUFFER_1 and DCL_UTF8_PARTIAL_BUFFER_2.
    Revision 1.64  2009/05/08 18:52:13Z  garyp
    Added a number of prototypes related to UCS-4 and UTF-8 functionality.
    Revision 1.63  2009/05/07 22:20:16Z  garyp
    Added/updated prototypes.
    Revision 1.62  2009/05/01 21:39:57Z  garyp
    Prototype updates.
    Revision 1.61  2009/04/09 21:41:23Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.60  2009/02/21 01:56:48Z  brandont
    Added DCLFUNC for DclWideCharToMultiByte and DclMultiByteToWideChar.
    Revision 1.59  2009/02/08 01:02:30Z  garyp
    Merged from the v4.0 branch.   Renamed DclOsGetChar() to DclOsInputChar(),
    Updated documentation.
    Revision 1.58  2009/01/21 14:48:13Z  johnb
    Updated DclWideCharToMultiByte and DclMultByteToWideChar to use char rather
    than D_UINT8.
    Revision 1.57  2009/01/19 21:58:54Z  johnb
    Added DclMultiByteToWideChar and DclWideCharToMultiByte prototypes.
    Revision 1.56  2008/10/30 20:24:13Z  keithg
    Corrected assert hook function typedef (was missing return type)
    Revision 1.55  2008/10/29 18:15:26Z  keithg
    Added a prototype and typedef for hooking the default assert.
    Revision 1.54  2008/08/20 00:06:16Z  keithg
    Added CRC 16 function API.
    Revision 1.53  2008/05/19 20:37:14Z  brandont
    Added max filesize field to the DclFsStatFs structure.
    Revision 1.52  2008/05/03 19:50:30Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.51  2008/04/16 22:33:19Z  brandont
    Changed DCLFSDIRHANDLE and DCLFSFILEHANDLE to be opaque structure pointers.
    Removed typedef for DCLFSDIRENTRY.  Changed the buffer pointer for
    DclFsFileWrite() to be const.
    Revision 1.50  2008/04/07 23:38:12Z  brandont
    Removed most of the legacy file system prototypes.
    Revision 1.49  2008/04/04 01:19:20Z  brandont
    Added new file system services abstraction.
    Revision 1.48  2008/04/04 00:17:32Z  brandont
    Changed max path length and max name length members of the DCLFSSTATFS
    structure to be of type unsigned.
    Revision 1.47  2008/04/04 00:11:09Z  brandont
    Changed the uAttributes field of the DCLFSSTAT structure to be 32-bits.
    Revision 1.46  2008/04/03 23:55:34Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.45  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.44  2007/10/30 22:43:18Z  pauli
    Removed DclOsFileTruncate.
    Revision 1.43  2007/10/27 16:43:55Z  Garyp
    Added prototypes for a number of new functions.
    Revision 1.42  2007/10/15 17:28:28Z  Garyp
    Modified DclMemPoolVerify() to take an fQuiet parameter.
    Revision 1.41  2007/09/12 19:21:43Z  Garyp
    Undid the rev 1.40 changes.  Moved the shell related settings to dlshell.h.
    Revision 1.40  2007/09/06 17:43:46Z  johnb
    Added DclOsAbsolutePath() to list of prototypes.
    Revision 1.39  2007/08/18 19:39:31Z  garyp
    Moved Windows specific date/time constants to dlwindows.h.  Added
    prototypes and mapping macros.
    Revision 1.38  2007/07/30 21:44:31Z  Garyp
    Modified the DCLFSSTATFS structure to contain a device name field.
    Revision 1.37  2007/06/23 02:43:41Z  brandont
    Added prototype for DclOsFileTruncate.
    Revision 1.36  2007/06/10 21:34:17Z  keithg
    Changed API to match existing code.
    Revision 1.35  2007/05/29 19:28:44Z  keithg
    Added prototypes for the CRC module.
    Revision 1.34  2007/05/15 03:24:46Z  garyp
    Prototype updates.
    Revision 1.33  2007/04/22 19:51:33Z  Garyp
    Added a prototype for DclRatio().
    Revision 1.32  2007/03/30 22:02:46Z  Garyp
    Added interface definition for DclOsDebugBreak().
    Revision 1.31  2007/03/30 18:59:18Z  Garyp
    Eliminated the obsolete DclMutexRecreate() function.
    Revision 1.30  2007/03/16 02:55:57Z  Garyp
    Eliminated the DclOsFileGet/PutChar() functions.  Split the DclOsRename()
    function into separate functions for files and directories.
    Revision 1.29  2007/02/20 02:01:51Z  Garyp
    Added milliseconds and microseconds to the DCLDATETIME structure.
    Revision 1.28  2007/02/14 20:39:46Z  billr
    Possibly generate better code and enable compile-time error checking.
    Revision 1.27  2007/01/16 22:15:36Z  Garyp
    More prototypes.
    Revision 1.26  2007/01/12 02:40:25Z  Garyp
    Moved the DCLSHELLHANDLE opaque type into dlapi.h so that it can be used
    by external applications not being built by the DL build process.
    Revision 1.25  2007/01/10 01:34:43Z  Garyp
    Prototype update.
    Revision 1.24  2006/12/12 20:57:31Z  Garyp
    Modified the existence of the DclOsGetChar() function to be dependent
    on the DCL_OSFEATURE_CONSOLEINPUT setting.
    Revision 1.23  2006/11/29 23:18:03Z  Garyp
    Minor prototype and shell updates.
    Revision 1.22  2006/10/18 04:27:20Z  Garyp
    Modified so DclOsThreadID() is always included even if the threading
    features are turned off.
    Revision 1.21  2006/10/08 20:51:04Z  Garyp
    Fixed up so we can properly scale the sign-on code out if we want.
    Revision 1.20  2006/10/07 02:52:49Z  Garyp
    Modified so the DCL signon functionality can be stripped out.
    Revision 1.19  2006/10/06 19:49:40Z  peterb
    Added prototypes for DclOsBecomeFileUser and DclOsReleaseFileUser.
    Revision 1.18  2006/10/03 21:13:58Z  Garyp
    Updated to allow output to be entirely disabled (and the code not pulled
    into the image).
    Revision 1.17  2006/09/28 19:42:30Z  brandont
    Added prototypes for DclOsThreadSuspend and DclOsThreadResume.
    Revision 1.16  2006/09/28 18:37:02Z  garyp
    Updated to allow the shell tests or tools to be compiled out.
    Revision 1.15  2006/09/20 00:15:02Z  Garyp
    Eliminated DclProductVersion().
    Revision 1.14  2006/07/14 04:50:34Z  brandont
    Added module prototypes for read/write semaphores.
    Revision 1.13  2006/06/26 17:05:19Z  Garyp
    Added high-res tick function prototypes.
    Revision 1.12  2006/06/15 23:21:49Z  Pauli
    Added DclShellRemoveCmd prototype.
    Revision 1.11  2006/06/01 23:44:16Z  Garyp
    DclULLtoA() no longer requires native 64-bit support.
    Revision 1.10  2006/05/29 01:42:31Z  Garyp
    Updated the shell interface, and moved a number of structures so they
    are now private.
    Revision 1.9  2006/05/06 22:04:52Z  Garyp
    Added/updated prototypes.
    Revision 1.8  2006/05/04 21:24:11Z  Pauli
    Expanded the file system interface abstraction to include directory
    operations.  Added a command shell.
    Revision 1.7  2006/03/16 02:33:29Z  Pauli
    Added absolute path utility function.
    Revision 1.6  2006/03/02 01:52:32Z  joshuab
    Added DclOsAnsiToWcs to header, as it is used in the WinCE port.
    Revision 1.5  2006/02/27 03:29:31Z  Garyp
    Updated DCLDATETIME to be a length which is 32-bit aligned.  Added
    prototypes for hamming codes.
    Revision 1.4  2006/02/22 17:53:34Z  Pauli
    Added function to allocate a buffer and zero initialize it.
    Revision 1.3  2006/02/12 20:21:29Z  Garyp
    Added date/time constants.
    Revision 1.2  2006/02/03 00:59:38Z  Pauli
    Added prototypes for date/time functions.
    Revision 1.1  2006/01/02 13:06:22Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLAPIINT_H_INCLUDED
#define DLAPIINT_H_INCLUDED

#include <dlosatomic.h>
#include <dlosdate.h>
#include <dlosdebug.h>
#include <dloshalt.h>
#include <dloshrtick.h>
#include <dlosmapmem.h>
#include <dlosoutput.h>
#include <dlossleep.h>
#include <dlosthread.h>
#include <dlostick.h>


/*-------------------------------------------------------------------
    Miscellaneous OS Services Layer (public)

    These are never remapped, though they might be completely
    replaced in project level code using modules and functions
    of the same name.

    Note that some OS Services Layer functions are considered to
    be private because they are wrapped by functionality in DCL,
    and we want the wrappers to be used rather than the OS Services
    code directly.  Those functions are not included here, and are
    defined in dlapiprv.h.
-------------------------------------------------------------------*/
#if DCL_OSFEATURE_UNICODE
  #define       DclOsWcsToAnsi              DCLFUNC(DclOsWcsToAnsi)
  #define       DclOsAnsiToWcs              DCLFUNC(DclOsAnsiToWcs)
  unsigned      DclOsWcsToAnsi(char *pszOutBuff, unsigned nOutBuffLen, const D_WCHAR *ptzUnicodeStr, int iLen);
  unsigned      DclOsAnsiToWcs(D_WCHAR *ptzOutBuff, unsigned nOutBuffLen, const char *pszAnsiStr, int iLen);
#endif

#if DCL_OSFEATURE_CONSOLEINPUT
  #define       DclOsInputChar              DCLFUNC(DclOsInputChar)
  DCLSTATUS     DclOsInputChar(void *pContext, D_UCHAR *puChar, unsigned nFlags);
#endif


/*-------------------------------------------------------------------
    System Information
-------------------------------------------------------------------*/

#include <dlsysinfo.h>


/*-------------------------------------------------------------------
    Date Time

    Type: DCLDATETIME

    The DCLDATETIME is a structure used to hold date and time
    information.
-------------------------------------------------------------------*/
#define         DclDateTimeDecode       DCLFUNC(DclDateTimeDecode)
#define         DclDateTimeEncode       DCLFUNC(DclDateTimeEncode)
#define         DclDateTimeString       DCLFUNC(DclDateTimeString)
#define         DclTimeFormat           DCLFUNC(DclTimeFormat)
#define         DclTimeFormatUS         DCLFUNC(DclTimeFormatUS)

typedef struct tagDCLDATETIME
{
    D_UINT16    uYear;          /* 1970+  */
    D_UINT8     uMonth;         /* 0 - 11 */
    D_UINT8     uDay;           /* 1 - 31 */
    D_UINT8     uHour;          /* 0 - 23 */
    D_UINT8     uMinute;        /* 0 - 59 */
    D_UINT8     uSecond;        /* 0 - 59 */
    D_UINT16    uMillisecond;   /* 0 - 999 (may not be supported in all environments) */
    D_UINT16    uMicrosecond;   /* 0 - 999 (may not be supported in all environments) */
} DCLDATETIME;

D_BOOL          DclDateTimeDecode(const D_TIME *pTime, DCLDATETIME *pDateTime);
D_BOOL          DclDateTimeEncode(const DCLDATETIME *pDateTime, D_TIME *pTime);
D_BOOL          DclDateTimeString(const D_TIME *pTime, char *pBuffer, size_t nBuffLen);
char *          DclTimeFormat(D_UINT64 ullSeconds, char *pBuffer, size_t nBuffLen);
char *          DclTimeFormatUS(D_UINT64 ullMicrosecs, char *pBuffer, size_t nBuffLen);


/*-------------------------------------------------------------------
    File System Interface

    Type: DCLFSDIRHANDLE

    DCLFSDIRHANDLE is an opaque pointer to a directory object which
    was opened with DclFsDirOpen().

    Type: DCLFSFILEHANDLE

    DCLFSFILEHANDLE is an opaque pointer to a directory object which
    was opened with DclFsFileOpen().

    Type: DCLFSSTAT

    The DCLFSSTAT structure type contains file system information
    returned by DclFsStat().

    Type: DCLFSSTATFS

    The DCLFSSTATFS structure type contains file system information
    returned by DclFsStatFs().
-------------------------------------------------------------------*/
typedef struct OsDirHandle * DCLFSDIRHANDLE;
typedef struct OsFileHandle * DCLFSFILEHANDLE;

typedef struct
{
    D_TIME      tCreation;
    D_TIME      tAccess;
    D_TIME      tModify;
    D_UINT32    ulSize;
    D_UINT32    ulAttributes;
} DCLFSSTAT;

#define DCL_MAX_DEVICENAMELEN   (32)

typedef struct
{
    D_UINT32    ulBlockSize;
    D_UINT32    ulTotalBlocks;
    D_UINT32    ulFreeBlocks;
    D_UINT32    ulMaxFileSize;  /* 0 if not known */
    unsigned    nMaxPathLen;
    unsigned    nMaxNameLen;
    char        szDeviceName[DCL_MAX_DEVICENAMELEN];
} DCLFSSTATFS;

#define DCLFSFILESEEK_SET        (0)
#define DCLFSFILESEEK_CUR        (1)
#define DCLFSFILESEEK_END        (2)

#define DCLFILE_EOF             (-1)

#define DCLFSATTR_READONLY    (0x01)
#define DCLFSATTR_HIDDEN      (0x02)
#define DCLFSATTR_SYSTEM      (0x04)
#define DCLFSATTR_VOLUME      (0x08)
#define DCLFSATTR_DIRECTORY   (0x10)
#define DCLFSATTR_ARCHIVE     (0x20)

typedef DCLSTATUS DCLFSDIRENUMCALLBACK(void *pContext, const char *pszPath, const DCLFSSTAT *pStat);

#define         DclOsFileEOF            DCLFUNC(DclOsFileEOF)
#define         DclOsFileSeek           DCLFUNC(DclOsFileSeek)
#define         DclOsFileTell           DCLFUNC(DclOsFileTell)
#define         DclOsFileRead           DCLFUNC(DclOsFileRead)
#define         DclOsFileWrite          DCLFUNC(DclOsFileWrite)

#define         DclFsFileOpen           DCLFUNC(DclFsFileOpen)
#define         DclFsFileSeek           DCLFUNC(DclFsFileSeek)
#define         DclFsFileTell           DCLFUNC(DclFsFileTell)
#define         DclFsFileRead           DCLFUNC(DclFsFileRead)
#define         DclFsFileWrite          DCLFUNC(DclFsFileWrite)
#define         DclFsFileFlush          DCLFUNC(DclFsFileFlush)
#define         DclFsFileClose          DCLFUNC(DclFsFileClose)
#define         DclFsFileDelete         DCLFUNC(DclFsFileDelete)
#define         DclFsFileRename         DCLFUNC(DclFsFileRename)

#define         DclFsDirOpen            DCLFUNC(DclFsDirOpen)
#define         DclFsDirRead            DCLFUNC(DclFsDirRead)
#define         DclFsDirRewind          DCLFUNC(DclFsDirRewind)
#define         DclFsDirClose           DCLFUNC(DclFsDirClose)
#define         DclFsDirCreate          DCLFUNC(DclFsDirCreate)
#define         DclFsDirRemove          DCLFUNC(DclFsDirRemove)
#define         DclFsDirRename          DCLFUNC(DclFsDirRename)
#define         DclFsDirSetWorking      DCLFUNC(DclFsDirSetWorking)
#define         DclFsDirGetWorking      DCLFUNC(DclFsDirGetWorking)
#define         DclFsDirTreeDelete      DCLFUNC(DclFsDirTreeDelete)
#define         DclFsDirTreeEnumerate   DCLFUNC(DclFsDirTreeEnumerate)

#define         DclFsBecomeFileUser     DCLFUNC(DclFsBecomeFileUser)
#define         DclFsReleaseFileUser    DCLFUNC(DclFsReleaseFileUser)
#define         DclFsStat               DCLFUNC(DclFsStat)
#define         DclFsStatFs             DCLFUNC(DclFsStatFs)

int             DclOsFileEOF(DCLFSFILEHANDLE hFile);
int             DclOsFileSeek(DCLFSFILEHANDLE hFile, long lOffset, int iOrgin);
long            DclOsFileTell(DCLFSFILEHANDLE hFile);
size_t          DclOsFileRead(void *pBuffer, size_t iSize, size_t iCount, DCLFSFILEHANDLE hFile);
size_t          DclOsFileWrite(const void *pBuffer, size_t iSize, size_t iCount, DCLFSFILEHANDLE hFile);

DCLSTATUS DclFsBecomeFileUser(void);
DCLSTATUS DclFsReleaseFileUser(void);
DCLSTATUS DclFsStat(const char * pszPath, DCLFSSTAT * pStat);
DCLSTATUS DclFsStatFs(const char * pszDevName, DCLFSSTATFS * pStatFs);

DCLSTATUS DclFsFileOpen(const char * pszPath, const char * pszMode, DCLFSFILEHANDLE * phFile);
DCLSTATUS DclFsFileSeek(DCLFSFILEHANDLE hFile, D_INT32 lOffset, int iOrigin);
DCLSTATUS DclFsFileTell(DCLFSFILEHANDLE hFile, D_UINT32 * pulPosition);
DCLSTATUS DclFsFileRead(DCLFSFILEHANDLE hFile, void * pBuffer, D_UINT32 ulSize, D_UINT32 * pulTransfered);
DCLSTATUS DclFsFileWrite(DCLFSFILEHANDLE hFile, const void * pBuffer, D_UINT32 ulSize, D_UINT32 * pulTransfered);
DCLSTATUS DclFsFileFlush(DCLFSFILEHANDLE hFile);
DCLSTATUS DclFsFileClose(DCLFSFILEHANDLE hFile);
DCLSTATUS DclFsFileDelete(const char * pszPath);
DCLSTATUS DclFsFileRename(const char * pszOrigPath, const char * pszNewPath);

DCLSTATUS DclFsDirOpen(const char * pszPath, DCLFSDIRHANDLE * phDir);
DCLSTATUS DclFsDirRead(DCLFSDIRHANDLE hDir, char * pszName, unsigned nMaxNameLength, DCLFSSTAT * pStat);
DCLSTATUS DclFsDirRewind(DCLFSDIRHANDLE hDir);
DCLSTATUS DclFsDirClose(DCLFSDIRHANDLE hDir);
DCLSTATUS DclFsDirCreate(const char * pszPath);
DCLSTATUS DclFsDirRemove(const char * pszPath);
DCLSTATUS DclFsDirRename(const char * pszOrigPath, const char * pszNewPath);
DCLSTATUS DclFsDirSetWorking(const char * pszPath);
DCLSTATUS DclFsDirGetWorking(char * pszPath, unsigned nMaxPathLength);
DCLSTATUS DclFsDirTreeDelete(const char *pszDirSpec, unsigned nVerbosity, D_BOOL fHiddenSystem, D_BOOL fDeleteReadonly, D_BOOL fIgnoreErrors);
DCLSTATUS DclFsDirTreeEnumerate(const char *pszDirSpec, DCLFSDIRENUMCALLBACK *pfnCallback, void *pCallbackContext, D_BOOL fHiddenSystem);


/*-------------------------------------------------------------------
    Critical Sections
-------------------------------------------------------------------*/
#define         DclCriticalSectionEnter DCLFUNC(DclCriticalSectionEnter)
#define         DclCriticalSectionLeave DCLFUNC(DclCriticalSectionLeave)

D_BOOL          DclCriticalSectionEnter(D_ATOMIC32 *pulAtomicSectionGate, D_UINT32 ulTimeoutMS, D_BOOL fSleepOK);
void            DclCriticalSectionLeave(D_ATOMIC32 *pulAtomicSectionGate);


/*-------------------------------------------------------------------
    Memory Management
-------------------------------------------------------------------*/
#define         DclMemAlloc             DCLFUNC(DclMemAlloc)
#define         DclMemAllocZero         DCLFUNC(DclMemAllocZero)
#define         DclMemFree              DCLFUNC(DclMemFree)

void *          DclMemAlloc(D_UINT32 ulSize);
void *          DclMemAllocZero(D_UINT32 ulSize);
DCLSTATUS       DclMemFree(void *pMem);

#define DCLMEM_ERROR_FAIL       (0)
#define DCLMEM_ERROR_ASSERT     (1)
#define DCLMEM_ERROR_INVALID    (UINT_MAX)

#if DCLCONF_MEMORYTRACKING

  #define       DclMemTrackServiceInit  DCLFUNC(DclMemTrackServiceInit)
  #define       DclMemTrackErrorMode    DCLFUNC(DclMemTrackErrorMode)
  #define       DclMemTrackPoolVerify   DCLFUNC(DclMemTrackPoolVerify)

  DCLSTATUS     DclMemTrackServiceInit( DCLINSTANCEHANDLE hDclInst);
  unsigned      DclMemTrackErrorMode(   DCLINSTANCEHANDLE hDclInst, unsigned nNewMode);
  DCLSTATUS     DclMemTrackPoolVerify(  DCLINSTANCEHANDLE hDclInst, D_BOOL fQuiet);

#else

  #define       DclMemTrackErrorMode(inst, mode)    DCLMEM_ERROR_INVALID
  #define       DclMemTrackPoolVerify(inst, quiet)  DCLSTAT_FEATUREDISABLED

#endif


#include <dlmutex.h>


/*-------------------------------------------------------------------
    Counting Semaphores

    Type: PDCLSEMAPHORE

    The PDCLSEMAPHORE type is an opaque pointer to a counting
    semaphore object which was created with DclSemaphoreCreate().
-------------------------------------------------------------------*/
typedef struct  DCLSEMAPHORE        *PDCLSEMAPHORE;

#define         DclSemaphoreAcquire     DCLFUNC(DclSemaphoreAcquire)
#define         DclSemaphoreCreate      DCLFUNC(DclSemaphoreCreate)
#define         DclSemaphoreDestroy     DCLFUNC(DclSemaphoreDestroy)
#define         DclSemaphoreRelease     DCLFUNC(DclSemaphoreRelease)

PDCLSEMAPHORE   DclSemaphoreCreate (const char *pszName, D_UINT32 ulCount);
D_BOOL          DclSemaphoreAcquire(PDCLSEMAPHORE pSem);
D_BOOL          DclSemaphoreDestroy(PDCLSEMAPHORE pSem);
D_BOOL          DclSemaphoreRelease(PDCLSEMAPHORE pSem);


/*-------------------------------------------------------------------
    Read/Write Semaphores

    Type: PDCLSEMAPHORERDWR

    The PDCLSEMAPHORERDWR type is an opaque pointer to a read/write
    semaphore object which was created with DclSemaphoreRdWrCreate().
-------------------------------------------------------------------*/
typedef struct DCLSEMAPHORERDWR * PDCLSEMAPHORERDWR;

#define DclSemaphoreRdWrAcquireRead     DCLFUNC(DclSemaphoreRdWrAcquireRead)
#define DclSemaphoreRdWrAcquireWrite    DCLFUNC(DclSemaphoreRdWrAcquireWrite)
#define DclSemaphoreRdWrCreate          DCLFUNC(DclSemaphoreRdWrCreate)
#define DclSemaphoreRdWrDestroy         DCLFUNC(DclSemaphoreRdWrDestroy)
#define DclSemaphoreRdWrRelease         DCLFUNC(DclSemaphoreRdWrRelease)

PDCLSEMAPHORERDWR DclSemaphoreRdWrCreate (const char * szName, D_UINT32 ulCount);
D_BOOL DclSemaphoreRdWrAcquireRead(PDCLSEMAPHORERDWR pSem);
D_BOOL DclSemaphoreRdWrAcquireWrite(PDCLSEMAPHORERDWR pSem);
D_BOOL DclSemaphoreRdWrDestroy(PDCLSEMAPHORERDWR pSem);
D_BOOL DclSemaphoreRdWrRelease(PDCLSEMAPHORERDWR pSem);


/*-------------------------------------------------------------------
    ECCs, CRCs, and Hamming Code Utility Functions
-------------------------------------------------------------------*/
typedef enum
{
    DCLECC_LOWLIMIT = 0,
    DCLECC_VALID,               /* Data and ECC were valid */
    DCLECC_DATACORRECTED,       /* ECC was valid, and the data was corrected */
    DCLECC_ECCCORRECTED,        /* Data was valid, and the ECC was corrected */
    DCLECC_UNCORRECTABLE,       /* The data or the ECC, or both, have errors */
    DCLECC_HIGHLIMIT
} DCLECCRESULT;

#define         DclCrc16Update              DCLFUNC(DclCrc16Update)
#define         DclCrc32Update              DCLFUNC(DclCrc32Update)
#define         DclEccCalculate             DCLFUNC(DclEccCalculate)
#define         DclEccCorrect               DCLFUNC(DclEccCorrect)
#define         DclEccOmap35xToCanonical    DCLFUNC(DclEccOmap35xToCanonical)
#define         DclEccCanonicalToOmap35x    DCLFUNC(DclEccCanonicalToOmap35x)
#define         DclHammingCalculate         DCLFUNC(DclHammingCalculate)
#define         DclHammingCorrect           DCLFUNC(DclHammingCorrect)

D_UINT16        DclCrc16Update(             D_UINT16 uCRC,  const D_BUFFER *pBuffer, size_t nLength);
D_UINT32        DclCrc32Update(             D_UINT32 ulCRC, const D_BUFFER *pBuffer, size_t nLength);
D_UINT32        DclEccCalculate(            const void *pData, size_t nLength);
DCLECCRESULT    DclEccCorrect(              D_BUFFER *pData, size_t nLength, D_UINT32 *pulOldECC, D_UINT32 ulNewECC);
D_UINT32        DclEccOmap35xToCanonical(   const D_BUFFER *pabECC);
void            DclEccCanonicalToOmap35x(   D_UINT32 ulECC, D_BUFFER *pabECC);
unsigned long   DclHammingCalculate(        const void *pBuffer, size_t nLength, unsigned long ulParity);
void            DclHammingCorrect(                void *pBuffer, size_t nLength, unsigned long ulCheck);


/*-------------------------------------------------------------------
    Synchronization Validation Functions
-------------------------------------------------------------------*/

#define DCL_SYNCVALIDATE_MAX_LOCALES    (8) /* The number of locales to record */

typedef struct
{
    D_ATOMIC32  ulAtomicState;
    D_ATOMIC32  ulAtomicCRC;        /* The data CRC if pData/nDataLen describe a memory range */
    D_ATOMICPTR apAtomicLocale[DCL_SYNCVALIDATE_MAX_LOCALES];
    D_ATOMIC32  ulAtomicLocaleIndex;/* The index into apAtomicLocale of the last item written */
    void       *pData;              /* The data pointer; may be NULL if no CRC is to be done */
    size_t      nDataLen;           /* The data length, must be zero of pData is NULL */
} DCLSYNCSTATE;

#define         DclSyncValidateInitialize   DCLFUNC(DclSyncValidateInitialize)
#define         DclSyncValidateReadBegin    DCLFUNC(DclSyncValidateReadBegin)
#define         DclSyncValidateReadEnd      DCLFUNC(DclSyncValidateReadEnd)
#define         DclSyncValidateWriteBegin   DCLFUNC(DclSyncValidateWriteBegin)
#define         DclSyncValidateWriteEnd     DCLFUNC(DclSyncValidateWriteEnd)
#define         DclSyncValidateReadToWrite  DCLFUNC(DclSyncValidateReadToWrite)
#define         DclSyncValidateWriteToRead  DCLFUNC(DclSyncValidateWriteToRead)

void            DclSyncValidateInitialize( DCLSYNCSTATE *pSyncState, void *pData, size_t nDataLen, D_BOOL fClearHistory);
D_UINT32        DclSyncValidateReadBegin(  DCLSYNCSTATE *pSyncState, const char *pszLocale);
DCLSTATUS       DclSyncValidateReadEnd(    DCLSYNCSTATE *pSyncState, D_UINT32 ulToken);
D_UINT32        DclSyncValidateWriteBegin( DCLSYNCSTATE *pSyncState, const char *pszLocale);
DCLSTATUS       DclSyncValidateWriteEnd(   DCLSYNCSTATE *pSyncState, D_UINT32 ulToken);
D_UINT32        DclSyncValidateReadToWrite(DCLSYNCSTATE *pSyncState, D_UINT32 ulToken, const char *pszLocale);
D_UINT32        DclSyncValidateWriteToRead(DCLSYNCSTATE *pSyncState, D_UINT32 ulToken, const char *pszLocale);


/*-------------------------------------------------------------------
    Miscellaneous Utility Functions
-------------------------------------------------------------------*/
#define         DclAbsolutePath         DCLFUNC(DclAbsolutePath)
#define         DclBitCount             DCLFUNC(DclBitCount)
#define         DclBitCountArray        DCLFUNC(DclBitCountArray)
#define         DclBitHighest           DCLFUNC(DclBitHighest)
#define         DclBitmapRangeGet       DCLFUNC(DclBitmapRangeGet)
#define         DclBitmapRangeSet       DCLFUNC(DclBitmapRangeSet)
#define         DclBitmapRangeClear     DCLFUNC(DclBitmapRangeClear)
#define         DclFormatBitFlags       DCLFUNC(DclFormatBitFlags)
#define         DclMulDiv               DCLFUNC(DclMulDiv)
#define         DclMulDiv64             DCLFUNC(DclMulDiv64)
#define         DclRatio                DCLFUNC(DclRatio)
#define         DclRatio64              DCLFUNC(DclRatio64)
#define         DclScaleBytes           DCLFUNC(DclScaleBytes)
#define         DclScaleItems           DCLFUNC(DclScaleItems)
#define         DclScaleKB              DCLFUNC(DclScaleKB)
#define         DclSignOn               DCLFUNC(DclSignOn)
#define         DclSleepLock            DCLFUNC(DclSleepLock)
#define         DclSortInsertion32      DCLFUNC(DclSortInsertion32)
#define         DclVersionFormat        DCLFUNC(DclVersionFormat)

D_BOOL          DclAbsolutePath(const char *pszRelPath, const char *pszCWD, const char *pszSeparators, char *pszAbsPath);
unsigned        DclBitCount(D_UINT32 ulValue);
unsigned        DclBitCountArray(const D_BUFFER *pBuffer, unsigned nLen);
unsigned        DclBitHighest(D_UINT32 ulValue);
D_UINT32        DclBitmapRangeGet(const D_BUFFER *pBuffer, D_UINT32 ulOffset, D_UINT32 ulCount, unsigned *pnValue); 
void            DclBitmapRangeSet(      D_BUFFER *pBuffer, D_UINT32 ulOffset, D_UINT32 ulCount);
void            DclBitmapRangeClear(    D_BUFFER *pBuffer, D_UINT32 ulOffset, D_UINT32 ulCount);
D_BOOL          DclFormatBitFlags(char *pszBuffer, size_t nBuffLen, const char **ppszNameList, unsigned nNames, D_UINT32 ulFlags);
D_UINT64        DclMulDiv(D_UINT32 ulBase, D_UINT32 ulMultiplier, D_UINT32 ulDivisor);
D_UINT64        DclMulDiv64(D_UINT64 ullBase, D_UINT32 ulMultiplier, D_UINT64 ullDivisor);
char *          DclRatio(char *pBuffer, unsigned nBufferLen, D_UINT32 ulDividend, D_UINT32 ulDivisor, unsigned nDecPlaces);
char *          DclRatio64(char *pBuffer, unsigned nBufferLen, D_UINT64 ullDividend, D_UINT64 ullDivisor, unsigned nDecPlaces);
char *          DclScaleBytes(D_UINT32 ulByteValue, char *pszBuffer, unsigned nBufferSize);
char *          DclScaleItems(D_UINT64 ullItems, D_UINT32 ulItemSize, char *pszBuffer, size_t nBufferSize);
char *          DclScaleKB(D_UINT32 ulKBValue, char *pszBuffer, unsigned nBufferSize);
const char *    DclSignOn(D_BOOL fQuiet);
DCLSTATUS       DclSleepLock(D_ATOMIC32 *pulAtomicFlag, D_UINT32 ulCompare, D_UINT32 ulExchange, D_UINT32 ulMaxMS, D_UINT32 ulIntervalMS);
size_t          DclSortInsertion32(D_UINT32 *pulList, size_t nElements);
char *          DclVersionFormat(char *pBuffer, unsigned nBuffLen, D_UINT32 ulVersion, const char *pszBuildNum);


/*-------------------------------------------------------------------
    Miscellaneous C Library Functions

    This lists those functions which cannot be remapped by OS or
    project code.  See dlapimap.h for the definitions for those
    functions which can be remapped.
-------------------------------------------------------------------*/
#define         DclHtoUL                DCLFUNC(DclHtoUL)
#define         DclNtoUL                DCLFUNC(DclNtoUL)
#define         DclRandomServiceInit    DCLFUNC(DclRandomServiceInit)
#define         DclRand                 DCLFUNC(DclRand)
#define         DclRand64               DCLFUNC(DclRand64)
#define         DclSNPrintf             DCLFUNC(DclSNPrintf)
#define         DclLtoA                 DCLFUNC(DclLtoA)
#define         DclULtoA                DCLFUNC(DclULtoA)
#define         DclULLtoA               DCLFUNC(DclULLtoA)
#define         DclSizeToUL             DCLFUNC(DclSizeToUL)
#define         DclSizeToULKB           DCLFUNC(DclSizeToULKB)
#define         DclWideCharToMultiByte  DCLFUNC(DclWideCharToMultiByte)
#define         DclMultiByteToWideChar  DCLFUNC(DclMultiByteToWideChar)

const char *    DclHtoUL(const char *pStr, D_UINT32 *pulResult);
const char *    DclNtoUL(const char *pStr, D_UINT32 *pulResult);
DCLSTATUS       DclRandomServiceInit(DCLINSTANCEHANDLE hDclInst);
D_UINT32        DclRand(D_UINT32 *pulSeed);
D_UINT64        DclRand64(D_UINT64 *pulSeed);
int             DclSNPrintf(char *pBuffer, int nBufferLen, const char *pszFmt, ...);
size_t          DclLtoA(  char *pBuffer, size_t *pnBufferLen, D_INT32 lNum, size_t nFillLen, char cFill);
size_t          DclULtoA( char *pBuffer, size_t *pnBufferLen, D_UINT32 ulNum, unsigned nRadix, size_t nFillLen, char cFill);
size_t          DclULLtoA(char *pBuffer, size_t *pnBufferLen, D_UINT64 *pullNum, unsigned nRadix, size_t nFillLen, char cFill);
const char *    DclSizeToUL(  const char *pachNum, D_UINT32 *pulResult);
const char *    DclSizeToULKB(const char *pachNum, D_UINT32 *pulResult);
DCLSTATUS       DclWideCharToMultiByte(const D_UINT16 *pWideCharStr, int iWideCharStrLen, char *pMultiByteBuffer, size_t nMultiByteBufLimit, size_t *pnNumConverted);
DCLSTATUS       DclMultiByteToWideChar(const char *pMultiByteStr, int iMultiByteStrLen, D_UINT16 *pWideCharBuffer, size_t nWideCharBufLimit, size_t *pnNumConverted);


/*-------------------------------------------------------------------
    Miscellaneous Service Initialization Functions
-------------------------------------------------------------------*/
#define         DclLogServiceInit       DCLFUNC(DclLogServiceInit)

DCLSTATUS       DclLogServiceInit(DCLINSTANCEHANDLE hDclInst);


/*-------------------------------------------------------------------
    UTF Functions
-------------------------------------------------------------------*/
#define         DclUtfScalarToLower     DCLFUNC(DclUtfScalarToLower)
#define         DclUtf8IsValidSequence  DCLFUNC(DclUtf8IsValidSequence)
#define         DclUtf8FromScalar       DCLFUNC(DclUtf8FromScalar)
#define         DclUtf8ToScalar         DCLFUNC(DclUtf8ToScalar)
#define         DclUtf8StrNICmp         DCLFUNC(DclUtf8StrNICmp)
#define         DclUtf8StrICmp          DCLFUNC(DclUtf8StrICmp)
#define         DclUtf8StringToLower    DCLFUNC(DclUtf8StringToLower)

#define DCL_UTF8_PARTIAL_BUFFER_1       0x01U
#define DCL_UTF8_PARTIAL_BUFFER_2       0x02U


D_UINT32        DclUtfScalarToLower(D_UINT32 ulCodePoint);
D_BOOL          DclUtf8IsValidSequence(const char *pacBuffer, size_t nBufferLen, size_t *pnSequenceLength);
size_t          DclUtf8FromScalar(D_UINT32 ulCodePoint, char *pacBuffer, size_t nBuffSize);
D_UINT32        DclUtf8ToScalar(const char *pacBuffer, size_t nBuffSize, size_t *pnBytesProcessed);
int             DclUtf8StrNICmp(const char *pacBuffer1, const char *pacBuffer2, size_t nBuffer1Size, size_t nBuffer2Size, size_t *pnProcessed1, size_t *pnProcessed2, unsigned *pnCmpFlags);
int             DclUtf8StrICmp(const char *pacBuffer1, const char *pacBuffer2);
void            DclUtf8StringToLower(const char *pacInputBuffer, size_t nInputBuffSize, char *pacOutputBuffer, size_t nOutputBuffSize, size_t *pnProcessedInput, size_t *pnProcessedOutput);


/*-------------------------------------------------------------------
    Requestor structures for Memory Tracking and Validation
-------------------------------------------------------------------*/
typedef struct
{
    DCLIOREQUEST    ior;
    const D_BUFFER *pBuffer;
    D_BOOL          fQuiet;
    DCLSTATUS       dclStat;
} DCLREQ_MEMTRACK_BLOCKVERIFY;

typedef struct
{
    DCLIOREQUEST    ior;
    D_BOOL          fQuiet;
} DCLREQ_MEMTRACK_POOLVERIFY;

typedef struct
{
    DCLIOREQUEST    ior;
    unsigned        nLength;
    DCLSTATUS       dclStat;
} DCLREQ_MEMTRACK_POOLDUMP;

typedef struct
{
    DCLIOREQUEST    ior;
    unsigned        nNewMode;
    unsigned        nOldMode;
} DCLREQ_MEMTRACK_ERRORMODE;


/*-------------------------------------------------------------------
    FTP Abstraction Interface
-------------------------------------------------------------------*/
#define DclFtpFsInstanceCreate      DCLFUNC(DclFtpFsInstanceCreate)
#define DclFtpFsInstanceDestroy     DCLFUNC(DclFtpFsInstanceDestroy)
#define DclFtpFsCreate              DCLFUNC(DclFtpFsCreate)
#define DclFtpFsOpen                DCLFUNC(DclFtpFsOpen)
#define DclFtpFsRead                DCLFUNC(DclFtpFsRead)
#define DclFtpFsWrite               DCLFUNC(DclFtpFsWrite)
#define DclFtpFsClose               DCLFUNC(DclFtpFsClose)
#define DclFtpFsDelete              DCLFUNC(DclFtpFsDelete)
#define DclFtpFsStat                DCLFUNC(DclFtpFsStat)
#define DclFtpFsRename              DCLFUNC(DclFtpFsRename)
#define DclFtpFsMkdir               DCLFUNC(DclFtpFsMkdir)
#define DclFtpFsRmdir               DCLFUNC(DclFtpFsRmdir)
#define DclFtpFsOpenDir             DCLFUNC(DclFtpFsOpenDir)
#define DclFtpFsReadDir             DCLFUNC(DclFtpFsReadDir)
#define DclFtpFsGetEntryName        DCLFUNC(DclFtpFsGetEntryName)
#define DclFtpFsCloseDir            DCLFUNC(DclFtpFsCloseDir)


typedef struct DclFtpFsInstanceData * DCLFTPFSINSTANCE;
typedef struct DclFtpFsHandleData * DCLFTPFSFD;
typedef struct DclFtpFsDirData * DCLFTPFSDIR;
typedef struct DclFtpFsDirEntryData * DCLFTPFSDIRENTRY;
typedef int DCLFTFSSTATUS;


DCLFTPFSINSTANCE    DclFtpFsInstanceCreate(const char * szAddress, const char * szUser, const char * szPassword);
void                DclFtpFsInstanceDestroy(DCLFTPFSINSTANCE * phInstance);
DCLFTPFSFD          DclFtpFsCreate( DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTPFSFD          DclFtpFsOpen(   DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTFSSTATUS       DclFtpFsDelete( DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTFSSTATUS       DclFtpFsStat(   DCLFTPFSINSTANCE hInstance, const char * szPath, DCLFSSTAT * pStat);
DCLFTFSSTATUS       DclFtpFsRename( DCLFTPFSINSTANCE hInstance, const char * szOldPath, const char * szNewPath);
DCLFTFSSTATUS       DclFtpFsMkdir(  DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTFSSTATUS       DclFtpFsRmdir(  DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTPFSDIR         DclFtpFsOpenDir(DCLFTPFSINSTANCE hInstance, const char * szPath);
DCLFTFSSTATUS       DclFtpFsRead(   DCLFTPFSFD hFile, void * pBuffer, D_UINT32 ulLength, D_UINT32 * pulTransfered);
DCLFTFSSTATUS       DclFtpFsWrite(  DCLFTPFSFD hFile, void * pBuffer, D_UINT32 ulLength, D_UINT32 * pulTransfered);
DCLFTFSSTATUS       DclFtpFsClose(  DCLFTPFSFD hFile);
DCLFTPFSDIRENTRY    DclFtpFsReadDir(DCLFTPFSDIR hDir);
DCLFTFSSTATUS       DclFtpFsCloseDir(DCLFTPFSDIR hDir);
DCLFTFSSTATUS       DclFtpFsGetEntryName(DCLFTPFSDIRENTRY hDirEntry, char * szEntryName);


/*  These should probably be relocated -- needs refactoring...
*/
#include <dlinstance.h>
#include <dlservice.h>


#endif  /* DLAPIINT_H_INCLUDED */


