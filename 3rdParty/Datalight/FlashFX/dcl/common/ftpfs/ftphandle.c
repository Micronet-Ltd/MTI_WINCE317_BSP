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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ftphandle.c $
    Revision 1.2  2009/07/15 17:59:11Z  keithg
    Updated to use renamed types, removed obsolete include file; Changed
    the Ftp prefix to FtpFs out of consistency.
    Revision 1.1  2009/07/15 06:53:20Z  keithg
    Initial revision
    Revision 1.3  2007/04/29 09:52:29Z  brandont
    Changed to reconnect after timeout.
    Revision 1.2  2007/04/25 01:25:40Z  brandont
    Added TRACEPRINTF calls.
    Revision 1.1  2007/04/14 07:19:48Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>
#include "ftpfspriv.h"


/*---------------------------------------------------------------------------
    Private: DclFtpHandleFree()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpHandleFree(
    DCLFTPFSFD * phFile)
{
    DCLFTPFSFD hFile;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpHandleFree() hFile=%P\n",
            *phFile));


    hFile = *phFile;
    if(hFile)
    {
        if(hFile->szPath)
        {
            DclMemFree(hFile->szPath);
        }
        DclMemFree(hFile);
    }
    *phFile = (void *)0;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpHandleFree()\n"));
}



/*---------------------------------------------------------------------------
    Private: DclFtpHandleAllocate()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSFD DclFtpHandleAllocate(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    DCLFTFSSTATUS ulStatus;
    DCLFTPFSFD hFile;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpHandleAllocate() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Make sure the FTP connection has not timed out
    */
    ulStatus = DclFtpFsLogin(hInstance);
    if(ulStatus != 0)
    {
        hFile = (void *)0;
        goto Done;
    }


    /*  Allocate a handle
    */
    hFile = DclMemAllocZero(sizeof(*hFile));
    if(!hFile)
    {
        goto Done;
    }
    hFile->szPath = DclMemAllocZero(DclStrLen(szPath)+1);
    if(!hFile->szPath)
    {
        DclFtpHandleFree(&hFile);
    }
    DclStrCpy(hFile->szPath, szPath);
    hFile->hInstance = hInstance;

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpHandleAllocate() hFile=%P\n",
            hFile));
    return hFile;
}


/*---------------------------------------------------------------------------
    Private: DclFtpHandleAcquire()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSFD DclFtpHandleAcquire(
    DCLFTPFSFD hFile)
{
    DCLFTFSSTATUS ulStatus;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpHandleAcquire() hFile=%P\n",
            hFile));


    /*  Make sure the FTP connection has not timed out
    */
    ulStatus = DclFtpFsLogin(hFile->hInstance);
    if(ulStatus != 0)
    {
        hFile = (void *)0;
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpHandleAcquire() Returning=%P\n",
            hFile));
    return hFile;
}


/*---------------------------------------------------------------------------
    Private: DclFtpHandleRelease()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpHandleRelease(
    DCLFTPFSFD hFile)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpHandleRelease() hFile=%P\n",
            hFile));


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpHandleRelease()\n"));
}

