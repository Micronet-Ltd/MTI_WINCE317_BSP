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

    This module contains the default OS Services implementations of the file
    system directory management functions.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify the
    the make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: osdir.c $
    Revision 1.7  2009/04/10 20:52:02Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.6  2008/04/10 22:18:04Z  brandont
    Updated to use the new file system services.
    Revision 1.5  2008/04/05 03:56:48Z  brandont
    Updated to use the DclOsFs services prefix.
    Revision 1.4  2008/04/03 23:30:18Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.3  2008/03/19 20:27:11Z  Garyp
    Documentation updates.
    Revision 1.2  2007/11/03 23:31:25Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/03/15 22:26:22Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Public: DclOsFsDirOpen()

    Open a directory for reading.

    Parameters
        pszPath - The path to the directory.
        phDir   - Populated with a DCLFSDIRHANDLE used to access
                  the directory.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirOpen(
    const char * pszPath,
    DCLFSDIRHANDLE * phDir)
{
    DclProductionAssert(pszPath);
    DclProductionAssert(phDir);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRead()

    Read the next entry in a directory.

    Parameters:
        hDir           - The handle returned by an earlier call to
                         DclOsFsDirOpen.
        pszName        - Populated with the next name in the directory.
        uMaxNameLength - Size of pszName including the NULL terminator.
        pStat          - Populated with information about the entry.
                         This can be NULL if the information is not
                         desired.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRead(
    DCLFSDIRHANDLE hDir,
    char * pszName,
    unsigned nMaxNameLength,
    DCLFSSTAT * pStat)
{
    (void)hDir;
    (void)pStat;
    (void)nMaxNameLength;
    DclProductionAssert(pszName);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRewind()

    Reset the position within an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclOsFsDirOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRewind(
    DCLFSDIRHANDLE hDir)
{
    (void)hDir;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirClose()

    Close an open directory.

    Parameters:
        hDir - The handle returned by an earlier call to DclOsFsDirOpen.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirClose(
    DCLFSDIRHANDLE hDir)
{
    (void)hDir;
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirCreate()

    Create a directory.

    Parameters:
        pszPath - The path to the directory to create.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirCreate(
    const char * pszPath)
{
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRemove()

    Remove a directory.

    Parameters:
        pszPath - The path to the directory to remove.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRemove(
    const char * pszPath)
{
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirRename()

    Rename or move a directory.

    Parameters:
        pszOrigPath - The old path.
        pszNewPath  - The new path.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirRename(
    const char * pszOrigPath,
    const char * pszNewPath)
{
    DclProductionAssert(pszOrigPath);
    DclProductionAssert(pszNewPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirSetWorking()

    Set the current working directory.

    *Note* -- Some OS environments do not support the concept of
    a current working directory (CWD).  The DclOsFsDirGetWorking()
    function should be used to determine whether CWD functionality
    can be used or not.  See the comments within that function.

    Parameters:
        pszPath - The path of the directory to set as the CWD.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirSetWorking(
    const char * pszPath)
{
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}


/*-------------------------------------------------------------------
    Public: DclOsFsDirGetWorking()

    Retrieve the current working directory.

    *Note* -- If this functionality is not supported, this function
    must return an error code (without asserting).  Higher level
    software may then use this function to detect whether CWD
    functionality is available or not and act appropriately.

    Parameters:
        pszPath        - A buffer to receive the current working
                         directory.
        uMaxPathLength - Size of buffer including the NULL terminator.

    Return Value:
        Returns a DCLSTATUS value indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclOsFsDirGetWorking(
    char * pszPath,
    unsigned nMaxPathLength)
{
    (void)nMaxPathLength;
    DclProductionAssert(pszPath);
    DclProductionError();
    return DCLSTAT_FS_BADCMD;
}
