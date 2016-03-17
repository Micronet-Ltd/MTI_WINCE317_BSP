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
    $Log: ftptxrx.c $
    Revision 1.3  2009/08/06 01:11:13Z  johnbr
    Changes for the power cycling tests.
    Revision 1.2  2009/07/15 17:59:12Z  keithg
    Updated to use renamed types, removed obsolete include file; Changed
    the Ftp prefix to FtpFs out of consistency.
    Revision 1.1  2009/07/15 06:58:02Z  keithg
    Initial revision
    Revision 1.2  2007/04/25 01:19:17Z  brandont
    Added TRACEPRINTF calls.
    Revision 1.1  2007/04/18 23:56:44Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>
#include "ftpfspriv.h"


/*---------------------------------------------------------------------------
    Private: DclFtpFsSend()

    Transmit data to the specified socket

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsSend(
    DCLNETSOCKET Socket,
    void * pBuffer,
    D_UINT32 ulLength,
    D_UINT32 * pulTransfered)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsSend() Socket=%d Buffer=%P Length=%lU\n",
            Socket,
            pBuffer,
            ulLength));


    *pulTransfered = DclNetSend(
            Socket,
            pBuffer,
            ulLength,
            DCLNET_MSG_DEFAULT);


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsSend() Transfered=%lU\n",
            *pulTransfered));
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsReceive()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsReceive(
    DCLNETSOCKET Socket,
    void * pBuffer,
    D_UINT32 ulMinLength,
    D_UINT32 ulMaxLength,
    D_UINT32 ulTimeOutMilliseconds,
    D_UINT32 * pulTransfered)
{
    int iResult = 0;
    D_UINT32 ulLength;
    int iTransfered;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsReceive() Socket=%d Buffer=%P MinLength=%lU MaxLength=%lU TimeOut=%lU\n",
            Socket,
            pBuffer,
            ulMinLength,
            ulMaxLength,
            ulTimeOutMilliseconds));


    *pulTransfered = 0;
    if(ulMaxLength)
    {
        do
        {
            iResult = DclNetRecv(
                    Socket,
                    pBuffer,
                    ulMaxLength,
                    DCLNET_MSG_PEEK);
            if(iResult < 0)
            {
                DclPrintf( "DclNetRecv: Error value is %d on os_socket %d, flag is DCLNET_MSG_PEEK\n",
                        iResult,
                        Socket);
                break;
            }
            ulLength = iResult;
            if((D_UINT32)ulLength >= ulMaxLength)
            {
                ulLength = ulMaxLength;
            }
            if(ulLength)
            {
                iTransfered = DclNetRecv(
                        Socket,
                        pBuffer,
                        ulLength,
                        DCLNET_MSG_DEFAULT);
                if ( iTransfered < 0)
                {
                    DclPrintf( "DclNetRecv: Error value is %d on os_socket %d, flag is DCLNET_MSG_DEFAULT\n",
                            iResult,
                            Socket);
                }

                pBuffer = DclPtrAddByte(pBuffer, ulLength);
                ulMaxLength -= iTransfered;
                if(ulLength > ulMinLength)
                {
                    ulMinLength = 0;
                }
                else
                {
                    ulMinLength -= iTransfered;
                }
                *pulTransfered += iTransfered;
            }
            else
            {
                if(ulTimeOutMilliseconds)
                {
                    DclOsSleep(1);
                    ulTimeOutMilliseconds--;
                }
                else
                {
                    break;
                }
            }
        }
        while(ulMinLength);
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsReceive() Transfered=%lU\n",
            *pulTransfered));
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsGetReply()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DclFtpReply DclFtpFsGetReply(
    DCLFTPFSINSTANCE hInstance)
{
    char cReply[4];
    D_UINT32 ulTransfered;
    D_UINT32 ulReply;
    char * szReply;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsGetReply() Instance=%P\n",
            hInstance));


    /*  Discard text following the reply code
    */
    szReply = hInstance->szLastReply;
    DclMemSet(cReply, 0, 4);
    DclFtpFsReceive(
            hInstance->Socket,
            cReply,
            3,
            3,
            1000,
            &ulTransfered);
    if(ulTransfered == 3)
    {
        /*  Set the return code
        */
        ulReply = DclAtoL(cReply);
        hInstance->ulReply = ulReply;


        /*  Discard text following the reply code
        */
        cReply[0] = 0;
#if 0
        DclPrintf("S>%03u", ulReply);
#endif
        while(cReply[0] != '\n')
        {
            DclFtpFsReceive(
                    hInstance->Socket,
                    cReply,
                    1,
                    1,
                    1000,
                    &ulTransfered);
#if 0
            DclPrintf("%c", (unsigned)cReply[0]);
#endif
            *szReply = *cReply;
            szReply++;
        }
        *szReply = 0;
    }
    else
    {
        /*  Return time-out
        */
        *szReply = 0;
        ulReply = 999;
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsGetReply() Reply=%lU\n",
            ulReply));
    return ulReply;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsSendText()

    Transmit an FTP command

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DclFtpReply DclFtpFsSendText(
    DCLFTPFSINSTANCE hInstance,
    char * szCommand)
{
    D_UINT32 ulLength;
    D_UINT32 ulTransfered;
    DclFtpReply ulReply;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsSendText() Instance=%P Command=%s\n",
            hInstance,
            szCommand));


#if 0
    DclPrintf("C>%s", szCommand);
#endif
    ulLength = DclStrLen(szCommand);
    DclFtpFsSend(
            hInstance->Socket,
            szCommand,
            ulLength,
            &ulTransfered);
    ulReply = DclFtpFsGetReply(hInstance);


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsSendText() Reply=%lU\n",
            ulReply));
    return ulReply;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsDataSocketOpen()

    Open the FTP data socket

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLNETSOCKET DclFtpFsDataSocketOpen(
    DCLFTPFSINSTANCE hInstance)
{
    DCLNETSOCKET DclDataSocket;
    char * szReply;
    int v1;
    int v2;
    int v3;
    int v4;
    int v5;
    int v6;
    char szIpAddress[17];
    int iPort;
    DclFtpReply ulReply;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsDataSocketOpen() Instance=%P\n",
            hInstance));


    /*  Enter passive mode
    */
    ulReply = DclFtpFsSendText(hInstance, "PASV\r\n");
    if(ulReply != 227)
    {
        DclDataSocket = -1;
        goto Done;
    }


    /*  Retrieve the IP address and port number from the reply
    */
    szReply = hInstance->szLastReply;
    while(*szReply != '(')
        szReply++;
    szReply++;
    v1 = DclAtoI(szReply);
    while(*szReply != ',')
        szReply++;
    szReply++;
    v2 = DclAtoI(szReply);
    while(*szReply != ',')
        szReply++;
    szReply++;
    v3 = DclAtoI(szReply);
    while(*szReply != ',')
        szReply++;
    szReply++;
    v4 = DclAtoI(szReply);
    while(*szReply != ',')
        szReply++;
    szReply++;
    v5 = DclAtoI(szReply);
    while(*szReply != ',')
        szReply++;
    szReply++;
    v6 = DclAtoI(szReply);
    DclSNPrintf(szIpAddress, DCLDIMENSIONOF(szIpAddress), "%d.%d.%d.%d", v1, v2, v3, v4);
    iPort = (v5 * 256) + v6;


    /*  Establish a data connection with the FTP server
    */
    DclDataSocket = DclFtpConnect(szIpAddress, (D_UINT16)iPort);

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsDataSocketOpen() Socket=%d\n",
            DclDataSocket));
    return DclDataSocket;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsReplyGetFileSize()

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsReplyGetFileSize(
    DCLFTPFSINSTANCE hInstance,
    D_UINT32 * pulFileSize)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsReplyGetFileSize() Instance=%P\n",
            hInstance));


    /*  Get the file size from the reply
    */
    *pulFileSize = DclAtoL(&hInstance->szLastReply[1]);


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsReplyGetFileSize() FileSize=%lU\n",
            *pulFileSize));
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsReplyGetDate()

    TODO: Call DclDateTimeEncode

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
void DclFtpFsReplyGetDate(
    DCLFTPFSINSTANCE hInstance,
    D_TIME * pullModifiedTime)
{
    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEINDENT),
            "DclFtpFsReplyGetDate() Instance=%P\n",
            hInstance));


    /*  TODO:  Get the date/time from the reply and convert it to
            DCL date/time
    */
    *pullModifiedTime = 0;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 3, TRACEUNDENT),
            "DclFtpFsReplyGetDate() ModifiedTime=%llX\n",
            *pullModifiedTime));
}

