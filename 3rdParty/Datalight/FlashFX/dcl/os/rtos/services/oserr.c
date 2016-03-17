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

    This module contains the RTOS OS Services default implementations for
    error code translation.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    the dclproj.mak file to build the new module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oserr.c $
    Revision 1.8  2011/03/26 20:53:55Z  garyp
    Updated a diagnostic message -- no functional changes.
    Revision 1.7  2010/05/19 17:48:26Z  garyp
    Corrected so that REL_ERR_PEMFILE and DLP_EMFILE do not return
    DCLSTAT_FS_UNDEFINED.  Fixed to display a useful error message for
    unmapped error codes.
    Revision 1.6  2009/04/25 05:03:11Z  brandont
    Added handling for the default error case and removed the production assert.
    Revision 1.5  2009/04/10 17:30:49Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.4  2009/02/07 23:56:33Z  garyp
    Merge from the v4.0 branch.  Updated a status code.
    Revision 1.3  2008/09/18 20:23:12Z  brandont
    Updated to use the Reliance posix APIs.
    Revision 1.2  2008/06/07 02:27:44Z  brandont
    Added error code translation for DCLSTAT_FS_DIRNOTEMPTY.
    Revision 1.1  2008/04/16 23:47:12Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlapiprv.h>


/*  Determine which file system interface to use.
*/
#undef OSFILE_USE_RELIANCE
#ifndef OSFILE_USE_RELIANCE
  #if (D_PRODUCTNUM == PRODUCTNUM_4GR) || (D_PRODUCTNUM == PRODUCTNUM_RELIANCE)
    #define OSFILE_USE_RELIANCE TRUE
  #else
    #define OSFILE_USE_RELIANCE FALSE
  #endif
#endif


#if OSFILE_USE_RELIANCE

/*  Using Reliance as the file system interface.
*/
#include <reliance_rtos.h>

#endif


/*-------------------------------------------------------------------
    Protected: DclOsErrToDclStatus()

    Map an error code from the OS (system) error code to a DCLSTATUS
    error code.

    Parameters:
        iSysErr - OS specific error code.

    Return Value:
        Returns a DCLSTATUS code indicating the translated result.
-------------------------------------------------------------------*/
DCLSTATUS DclOsErrToDclStatus(
    int         iSysErr)
{
  #if OSFILE_USE_RELIANCE
    DCLSTATUS   dclStat;

    switch(iSysErr)
    {
        case 0:
            dclStat = DCLSTAT_SUCCESS;
            break;

        case REL_ERR_ACCES:
        case REL_ERR_BADPARM:
        case REL_ERR_NOT_OPENED:
        case REL_ERR_BADUSER:
        case REL_ERR_BADDRIVE:
        case REL_ERR_INVPARCMB:
        case DLP_EACCES:
            dclStat = DCLSTAT_FS_DENIED;
            break;

        case REL_ERR_LONGPATH:
        case DLP_ENAMETOOLONG:
            dclStat = DCLSTAT_FS_PATHTOOLONG;
            break;

        case REL_ERR_INVNAME:
        case DLP_EINVAL:
            dclStat = DCLSTAT_FS_BADPATH;
            break;

        case REL_ERR_NOSPC:
        case DLP_ENOSPC:
            dclStat = DCLSTAT_FS_FULL;
            break;

        case REL_ERR_NOFILE:
        case DLP_ENOENT:
            dclStat = DCLSTAT_FS_NOTFOUND;
            break;

        case REL_ERR_IO_ERROR:
        case DLP_EIO:
            dclStat = DCLSTAT_FS_IOERROR;
            break;

        case REL_ERR_BADFILE:
        case DLP_EBADF:
            dclStat = DCLSTAT_FS_BADHANDLE;
            break;

        case REL_ERR_EXIST:
        case DLP_EEXIST:
            dclStat = DCLSTAT_FS_EXISTS;
            break;

        case REL_ERR_SHARE:
        case DLP_EBUSY:
            dclStat = DCLSTAT_FS_HANDLESHARE;
            break;

        case REL_ERR_NOEMPTY:
        case DLP_ENOTEMPTY:
            dclStat = DCLSTAT_FS_DIRNOTEMPTY;
            break;

        case REL_ERR_READ_ONLY:
        case DLP_EROFS:
            dclStat = DCLSTAT_FS_FULL;
            break;

        case REL_ERR_BAD_CMD:
        case DLP_ESPIPE:
            dclStat = DCLSTAT_FS_BADCMD;
            break;

        case REL_ERR_MAXFILE_SIZE:
        case DLP_EFBIG:
            dclStat = DCLSTAT_FS_WRITETOOLONG;
            break;

        case REL_ERR_PEMFILE:
        case DLP_EMFILE:
            dclStat = DCLSTAT_FS_NOHANDLES;
            break;

        case REL_ERR_BLOCKSIZE:
            dclStat = DCLSTAT_FS_BLOCKSIZE;
            break;
            
        case REL_ERR_FORMAT:
            dclStat = DCLSTAT_FS_UNFORMATTED;
            break;
            
        case REL_ERR_LAST_ENTRY:
            dclStat = DCLSTAT_FS_LASTENTRY;
            break;
            
        case REL_ERR_NO_MEMORY:
            dclStat = DCLSTAT_MEMALLOCFAILED;
            break;

        case REL_ERR_ISDIR:
        case REL_ERR_ISNOTDIR:
        case DLP_EPERM:
            /* ToDo:  Why are these mapped this way?
            */
            
        default:
            DclPrintf("DclOsErrToDclStatus() unhandled code %d translated to DCLSTAT_FS_UNDEFINED\n", iSysErr);
            dclStat = DCLSTAT_FS_UNDEFINED;
            break;
    }

    return dclStat;
    
  #else
  
    (void)iSysErr;
    DclProductionError();
    return DCLSTAT_FS_UNDEFINED;
    
  #endif
}
