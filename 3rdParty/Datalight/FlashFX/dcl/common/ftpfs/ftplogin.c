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
    $Log: ftplogin.c $
    Revision 1.2  2009/07/15 17:59:11Z  keithg
    Updated to use renamed types, removed obsolete include file; Changed
    the Ftp prefix to FtpFs out of consistency.
    Revision 1.1  2009/07/15 06:54:54Z  keithg
    Initial revision
    Revision 1.4  2009/03/20 01:47:47Z  brandont
    Corrected an error detection case in DafFtpFsConnect.
    Revision 1.3  2007/04/29 09:55:52Z  brandont
    Updated to detect when login is necessary.
    Revision 1.2  2007/04/25 01:26:25Z  brandont
    Added TRACEPRINTF calls.
    Revision 1.1  2007/04/18 23:00:54Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>
#include "ftpfspriv.h"


/*---------------------------------------------------------------------------
    Private: DclFtpConnect()

    Establish a connection with the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLNETSOCKET DclFtpConnect(
    const char * szAddress,
    D_UINT16 uPort)
{
    DCLNETSOCKET DclSocket;
    int iErr;
    DCLNETSOCKADDRIN adr_host;
    DCLNETHOSTENT *H;
    D_UINT32 ulNetworkAddress;
    int optval = 1;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpConnect() Address=%s Port=%U\n",
            szAddress,
            uPort));


    DclSocket = DclNetSocket(
            DCLNET_AF_INET,
            DCLNET_SOCK_STREAM,
            DCLNET_IPPROTO_TCP);
    if(DclSocket < 0)
    {
        goto Done;
    }


    /*  Allow this socket to be reused
    */
    iErr = DclNetSetSockOpt(
            DclSocket,
            DCLNET_SOL_SOCKET,
            DCLNET_SO_REUSEADDR,
            (char *)&optval,
            sizeof(optval));


    /*  Establish a connection with the FTP server
    */
    ulNetworkAddress = DclNetInetAddr(szAddress);
    H = DclNetGetHostByAddr(
            (char *)&ulNetworkAddress,
            4,
            DCLNET_AF_INET);
    adr_host.sin_family = DCLNET_AF_INET;
    adr_host.sin_port = DclNetHtons(uPort);
    if((!H) || (!(*((D_UINT32 *)H->h_addr))))
    {
        DclNetCloseSocket(DclSocket);
        DclSocket = -1;
        goto Done;
    }
    adr_host.sin_addr.s_addr = *((D_UINT32 *)H->h_addr);
    DclMemSet(adr_host.sin_zero, 0, 8);
    iErr = DclNetConnect(
            DclSocket,
            (DCLNETSOCKADDR *)&adr_host,
            sizeof(adr_host));
    if(iErr)
    {
        DclNetCloseSocket(DclSocket);
        DclSocket = -1;
        goto Done;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpConnect() Socket=%d\n",
            DclSocket));
    return DclSocket;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsLogin()

    Login to the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsLogin(
    DCLFTPFSINSTANCE hInstance)
{
    char szCommand[FTP_COMMAND_LENGTH];
    D_UINT32 ulReply;
    DCLFTFSSTATUS ulResult = -1;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpFsLogin() Instance=%P\n",
            hInstance));


    /*  See if the FTP server is listening
    */
    ulReply = DclFtpFsSendText(hInstance, "NOOP\r\n");
    if(ulReply == 200)
    {
        /*  Already connected
        */
        ulResult = 0;
    }
    else
    {
        /*  Send user name
        */
        DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "USER %s\r\n", hInstance->szUser);
        ulReply = DclFtpFsSendText(hInstance, szCommand);
        if(ulReply == 331)
        {
            /*  Send password
            */
            DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "PASS %s\r\n", hInstance->szPassword);
            ulReply = DclFtpFsSendText(hInstance, szCommand);
            if(ulReply == 230)
            {
                /*  Successful
                */
                ulResult = 0;
            }
        }
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpFsLogin() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsLogout()

    Logout of the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsLogout(
    DCLFTPFSINSTANCE hInstance)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEINDENT),
            "DclFtpFsLogout() Instance=%P\n",
            hInstance));


    DclFtpFsSendText(hInstance, "QUIT\r\n");


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 2, TRACEUNDENT),
            "DclFtpFsLogout()\n"));
}
