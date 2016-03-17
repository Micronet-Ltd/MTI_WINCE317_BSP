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

    This module contains the default OS Services implementations for the file
    system management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    make file to build the new module.

    NOTE! These functions are stubbed in this module for the wincebl OS,
    because they are not supported in a standard way in the various
    Windows CE bootloader implementations, and are also not typically
    needed in a bootloader.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osfile.c $
    Revision 1.10  2009/04/10 20:52:22Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.9  2008/04/17 01:19:50Z  brandont
    Changed the stubbed versions of DclOsFsBecomeFileUser and
    DclOsFsReleaseFileUser to error by default.
    Revision 1.8  2008/04/17 01:07:36Z  brandont
    Updated to use the new file system services.
    Revision 1.7  2008/04/05 03:54:03Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.6  2008/04/03 23:56:00Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.5  2007/11/03 23:31:38Z  Garyp
    Added the standard module header.
    Revision 1.4  2007/07/17 20:03:16Z  Garyp
    Minor documentation and debug code cleanup -- nothing functional.
    Revision 1.3  2007/03/15 22:26:22Z  Garyp
    Refactored the modules, placing the directory operations functions in osdir.c
    and the file I/O functions in osfileio.c.  The generic/miscellaneous or FS
    related functions remain in osfile.c.  Eliminated the DclOsFsFileGet/PutChar()
    functions.  Split DclOsRename() into separate functions for directories and
    files (even though they may get redirected to the same ultimate function
    most of the time).
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
    Public: DclOsFsFileDelete()

    Delete a file.

    Parameters:
        pszPath - The path of the file to delete.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileDelete(
    const char * pszPath)
{
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsStat()

    Retrieve information about a file or directory.

    Parameters:
        pszName - The name of the object to get info about.
        pStat   - A buffer to receive the Stat information.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStat(
    const char * pszName,
    DCLFSSTAT * pStat)
{
    DclProductionAssert(pszName);
    DclProductionAssert(pStat);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsStatFs()

    Retrieve information about a file system.

    Parameters:
        pszPath - A path on the target file system.
        pStatFs - A buffer to receive the StatFs information.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsStatFs(
    const char * pszPath,
    DCLFSSTATFS * pStatFs)
{
    DclProductionAssert(pszPath);
    DclProductionAssert(pStatFs);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsFileRename()

    Rename or move a file.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsFileRename(
    const char * pszOrigName,
    const char * pszNewName)
{
    DclProductionAssert(pszOrigName);
    DclProductionAssert(pszNewName);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsBecomeFileUser()

    Allow this thread to access the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsBecomeFileUser(void)
{
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsReleaseFileUser()

    Indicate that this thread is finished accessing the file system.

    Parameters:
        None.

    Return Value:
        Returns a DCLSTATUS code indicating the result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsReleaseFileUser(void)
{
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}
