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

    This module contains the Windows CE OS Services default implementations
    for:

        DclOsErrToDclStatus()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the master product's make file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oserr.c $
    Revision 1.9  2011/06/14 21:48:29Z  garyp
    Updated to map ERROR_DEVICE_REMOVED.
    Revision 1.8  2009/10/29 23:52:57Z  garyp
    Updated to handle ERROR_WRITE_PROTECT.
    Revision 1.7  2009/09/24 21:52:15Z  garyp
    Updated to map ERROR_NOT_SUPPORTED.
    Revision 1.6  2009/04/10 17:30:08Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.5  2009/02/08 01:59:28Z  garyp
    Merged from the v4.0 branch.  Display a message rather than asserting, if
    an unhandled error is encountered.  Updated to map ERROR_SHARING_VIOLATION.
    Revision 1.4  2008/05/02 17:11:45Z  garyp
    Removed a duplicated case.
    Revision 1.3  2008/04/21 21:01:49Z  brandont
    Added error code mapping for DCLSTAT_FS_LASTENTRY.
    Revision 1.2  2008/04/17 23:42:31Z  jeremys
    Replaced a Reliance-specific include with windows.h.
    Revision 1.1  2008/04/09 00:08:44Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlapiprv.h>


/*-------------------------------------------------------------------
    Protected: DclOsErrToDclStatus()

    Maps an error code from the OS (system) error code to a DCLSTATUS
    error code.

    Parameters:
        iSysErr - OS specific error code.

    Return Value:
        Returns a DCLSTATUS code indicating the translated result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsErrToDclStatus(
    int         iSysErr)
{
    DCLSTATUS   dclStat;

    switch(iSysErr)
    {
        case 0:
            dclStat = DCLSTAT_SUCCESS;
            break;

        case ERROR_DIR_NOT_EMPTY:
            dclStat = DCLSTAT_FS_DIRNOTEMPTY;
            break;

        case ERROR_SHARING_VIOLATION:
        case ERROR_ACCESS_DENIED:
        case ERROR_ALREADY_EXISTS:
            dclStat = DCLSTAT_FS_DENIED;
            break;

        case ERROR_FILE_NOT_FOUND:
            dclStat = DCLSTAT_FS_NOTFOUND;
            break;

        case ERROR_INVALID_HANDLE:
            dclStat = DCLSTAT_FS_BADHANDLE;
            break;

        case ERROR_TOO_MANY_OPEN_FILES:
        case ERROR_OUTOFMEMORY:
        case ERROR_NOT_ENOUGH_MEMORY:
            dclStat = DCLSTAT_MEMALLOCFAILED;
            break;

        case ERROR_HANDLE_EOF:
            dclStat = DCLSTAT_FS_EOF;
            break;

        case ERROR_HANDLE_DISK_FULL:
        case ERROR_DISK_FULL:
            dclStat = DCLSTAT_FS_FULL;
            break;

        case ERROR_BAD_PATHNAME:
        case ERROR_PATH_NOT_FOUND:
            dclStat = DCLSTAT_FS_BADPATH;
            break;

        case ERROR_FILENAME_EXCED_RANGE:
            dclStat = DCLSTAT_FS_PATHTOOLONG;
            break;

        case ERROR_NO_MORE_FILES:
            dclStat = DCLSTAT_FS_LASTENTRY;
            break;

        case ERROR_CANNOT_MAKE:
            dclStat = DCLSTAT_FS_TOOMANY;
            break;

        case ERROR_NOT_SUPPORTED:
            dclStat = DCLSTAT_UNSUPPORTED;
            break;

        case ERROR_INVALID_PARAMETER:
            dclStat = DCLSTAT_BADPARAMETER;
            break;

        case ERROR_DEVICE_REMOVED:
            dclStat = DCLSTAT_FS_UNMOUNTED;
            break;

        case ERROR_WRITE_PROTECT:
            dclStat = DCLSTAT_WRITEPROTECT;
            break;

        default:
            DCLPRINTF(1, ("DclOsErrToDclStatus() Unmapped OS error code %d - Mapping to DCLSTAT_FS_UNDEFINED\n", iSysErr));

            dclStat = DCLSTAT_FS_UNDEFINED;
            break;
    }

    return dclStat;
}


