/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
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

    This module contains the default implementations of the file I/O OS
    Services functions.

    Should these routines need customization for your project, copy this
    module into the Project Directory, make your changes, and modify
    the master product's make file to build the new module.

    NOTE! These functions are stubbed in this module for the wincebl OS,
    because they are not supported in a standard way in the various
    Windows CE bootloader implementations, and are also not typically
    needed in a bootloader.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfileio.c $
    Revision 1.8  2009/04/10 02:01:32Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.7  2008/04/16 22:54:02Z  brandont
    Corrected prototype mismatch.
    Revision 1.6  2008/04/16 22:17:54Z  brandont
    Updated to use the new file system services.
    Revision 1.5  2008/04/05 03:52:24Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.4  2008/04/03 23:30:46Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.3  2008/03/19 20:24:10Z  Garyp
    Added a stubbed version of DclOsFsFileEof().
    Revision 1.2  2007/11/03 23:31:38Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/03/15 00:35:42Z  Garyp
    Initial revision
    Revision 1.2  2006/11/04 01:24:38Z  Garyp
    Added stubbed directory functions.
    Revision 1.1  2006/03/16 19:53:18Z  timothyj
    Initial revision
    Revision 1.3  2005/05/02 18:05:06Z  Garyp
    Fleshed out the missing file I/O services.
    Revision 1.2  2005/10/07 00:38:04Z  Garyp
    Modified DclOsFsFileDelete() to return a value that is similar to that
    returned by remove().
    Revision 1.1  2005/05/03 06:21:48Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Public: DclOsFsFileOpen()

    Open a file.  This API can also be used to create or truncate
    files.

    Parameters:
        pszPath - The path to the file.
        pszMode - The file open mode.
        phFile  - Populated with a DCLFSFILEHANDLE used to access
                  the file.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileOpen(
    const char * pszFileName,
    const char * pszMode,
    DCLFSFILEHANDLE * phFile)
{
    DclProductionAssert(pszFileName);
    DclProductionAssert(pszMode);
    DclProductionAssert(phFile);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileClose()

    Close an open file.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileClose(
    DCLFSFILEHANDLE hFile)
{
    (void)hFile;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileEOF()

    Determine if the file pointer as at the end-of-file or not.

        Compatibility Note:  Some documentation indicates that this
        function returns zero on the <first attempt to read> past
        the end-of-file.  This behavior should not be depended upon.

        For example if you seek to the end-of-file, and call this
        function it <may> report end-of-file, even though by the
        previous definition it should NOT yet report end-of-file,
        because a read attempt has not yet been made.  Therefore
        this behavior must not be depended upon.

    Parameters:
        hFile       - The DCLFSFILEHANDLE value

    Return Value:
        Returns zero if the file pointer is not at the end-of-file,
        and non-zero if it is.
-------------------------------------------------------------------*/
int DclOsFsFileEOF(
    DCLFSFILEHANDLE   hFile)
{
    (void)hFile;
    DclProductionError();
    return -1;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileFlush()

    Flush any pending file data.

    Parameters:
        hFile - The handle returned by an earlier call to DclOsFsFileOpen.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileFlush(
    DCLFSFILEHANDLE   hFile)
{
    (void)hFile;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileSeek()

    Seek to a position in a file.

    Parameters:
        hFile    - The handle returned by an earlier call to DclOsFsFileOpen.
        lOffset  - Relative file position.
        iOrgin   - Relative to what location in a file.  DCLFSFILESEEK_SET,
                   DCLFSFILESEEK_CUR, or DCLFSFILESEEK_END.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileSeek(
    DCLFSFILEHANDLE hFile,
    D_INT32 lOffset,
    int iOrigin)
{
    (void)hFile;
    (void)lOffset;
    (void)iOrigin;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileTell()

    Obtain the current position in a file.

    Parameters:
        hFile        - The handle returned by an earlier call to
                       DclOsFsFileOpen.
        pulPosition  - Populated with the current file position.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileTell(
    DCLFSFILEHANDLE hFile,
    D_UINT32 * pulPosition)
{
    (void)hFile;
    DclProductionAssert(pulPosition);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileRead()

    Read data from a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclOsFsFileOpen.
        pBuffer        - A pointer to the buffer to fill.
        ulSize         - Number of bytes to read.
        pulTransfered  - Populated with the number of bytes
                         actually read.

    Return Value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRead(
    DCLFSFILEHANDLE hFile,
    void * pBuffer,
    D_UINT32 ulSize,
    D_UINT32 * pulTransfered)
{
    (void)hFile;
    (void)pBuffer;
    (void)ulSize;
    DclProductionAssert(pulTransfered);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileWrite()

    Write data to a file.

    Parameters:
        hFile          - The handle returned by an earlier call to
                         DclOsFsFileOpen.
        pBuffer        - A pointer to the buffer from which to write.
        ulSize         - Number of bytes to write.
        pulTransfered  - Populated with the number of bytes
                         actually written.

    Return value:
        Returns a DCLSTATUS value representing a class of errors
        defined in dlstatus.h.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileWrite(
    DCLFSFILEHANDLE hFile,
    const void * pBuffer,
    D_UINT32 ulSize,
    D_UINT32 * pulTransfered)
{
    (void)hFile;
    (void)pBuffer;
    (void)ulSize;
    DclProductionAssert(pulTransfered);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}
