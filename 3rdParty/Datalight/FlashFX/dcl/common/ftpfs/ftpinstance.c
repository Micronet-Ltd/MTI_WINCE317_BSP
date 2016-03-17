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
    $Log: ftpinstance.c $
    Revision 1.3  2009/08/06 21:48:30Z  johnbr
    Changes for the power cycling tests.
    Revision 1.2  2009/07/15 17:59:11Z  keithg
    Updated to use renamed types, removed obsolete include file; Changed
    the Ftp prefix to FtpFs out of consistency.
    Revision 1.1  2009/07/15 06:53:54Z  keithg
    Initial revision
    Revision 1.3  2007/04/29 09:50:46Z  brandont
    Moved the code block used to clear FTP signon chatter from the
    login to the instance create.
    Revision 1.2  2007/04/25 00:53:12Z  brandont
    Added TRACEPRINTF calls.
    Revision 1.1  2007/04/14 07:19:36Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>
#include "ftpfspriv.h"

/*---------------------------------------------------------------------------
    Private: DclFtpFsInstanceDestroy()

    Destroy an instance of the FTP file system

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsInstanceDestroy(
    DCLFTPFSINSTANCE * phInstance)
{
    DCLFTPFSINSTANCE hInstance;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsInstanceDestroy() Instance=%P\n",
            *phInstance));

    DclAssert(phInstance);

    hInstance = *phInstance;
    if(hInstance)
    {
        if(hInstance->fLogout)
        {
            /*  Terminate the connection with FTP server
            */
            DclFtpFsLogout(hInstance);
        }

        if(hInstance->Socket >= 0)
        {
            /*  Close the network socket
            */
            DclNetCloseSocket(hInstance->Socket);
        }

        /*  Free the saved instance create arguments
        */
        if(hInstance->szPassword)
        {
            DclMemFree(hInstance->szPassword);
        }
        if(hInstance->szUser)
        {
            DclMemFree(hInstance->szUser);
        }
        if(hInstance->szAddress)
        {
            DclMemFree(hInstance->szAddress);
        }

        /*  Free the space allocate to manage this instance of
            the FTP file system
        */
        DclMemFree(hInstance);
    }
    *phInstance = (void *)0;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsInstanceDestroy()\n"));
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsInstanceCreate()

    Create an instance of the FTP file system

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSINSTANCE DclFtpFsInstanceCreate(
    const char * szAddress,
    const char * szUser,
    const char * szPassword)
{
    char szCommand[512];
    D_UINT32 ulTransfered;
    DCLFTPFSINSTANCE hInstance;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsInstanceCreate() Address=%s User=%s Password=%s\n",
            szAddress,
            szUser,
            szPassword));

    /*  Allocate space to manage this instance of the FTP file system
    */
    hInstance = DclMemAllocZero(sizeof(*hInstance));
    if(!hInstance)
        goto Done;

    /*  Save the calling arguments in the instance in case the connection
        times out and needs to reconnect.
    */
    hInstance->szAddress = DclMemAllocZero(DclStrLen(szAddress)+1);
    if(!hInstance->szAddress)
    {
        DclFtpFsInstanceDestroy(&hInstance);
        goto Done;
    }
    DclStrCpy(hInstance->szAddress, szAddress);

    hInstance->szUser = DclMemAllocZero(DclStrLen(szUser)+1);
    if(!hInstance->szUser)
    {
        DclFtpFsInstanceDestroy(&hInstance);
        goto Done;
    }
    DclStrCpy(hInstance->szUser, szUser);

    hInstance->szPassword = DclMemAllocZero(DclStrLen(szPassword)+1);
    if(!hInstance->szPassword)
    {
        DclFtpFsInstanceDestroy(&hInstance);
        goto Done;
    }
    DclStrCpy(hInstance->szPassword, szPassword);

    /*  Establish a connection with the FTP server
    */
    hInstance->Socket = DclFtpConnect(
            hInstance->szAddress,
            DCLNET_IPPORT_FTP);
    if(hInstance->Socket < 0)
    {
        DclFtpFsInstanceDestroy(&hInstance);
        goto Done;
    }

    /*  Clear out the FTP chatter
    */
    DclFtpFsReceive(
            hInstance->Socket,
            szCommand,
            1,
            DCLDIMENSIONOF(szCommand),
            500,
            &ulTransfered);

    /*  Login to the FTP server
    */
    if(DclFtpFsLogin(hInstance))
    {
        DclFtpFsInstanceDestroy(&hInstance);
        goto Done;
    }
    hInstance->fLogout = TRUE;

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsInstanceCreate() Instance=%P\n",
            hInstance));
    return hInstance;
}

