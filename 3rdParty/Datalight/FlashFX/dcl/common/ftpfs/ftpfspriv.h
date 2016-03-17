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
    $Log: ftpfspriv.h $
    Revision 1.3  2009/07/17 23:10:36Z  johnbr
    Changes for the power cycling tests.
    Revision 1.2  2009/07/15 17:42:37Z  keithg
    Updated types to standard naming conventions.
    Revision 1.1  2009/07/15 06:50:48Z  keithg
    Initial revision
    Revision 1.1  2007/04/14 07:21:12Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FTPFSPRIV_H_INCLUDE
#define FTPFSPRIV_H_INCLUDE


/*
*/
#define FTP_NAME_LENGTH             260
#define FTP_COMMAND_LENGTH          (FTP_NAME_LENGTH + 7)


/*
*/
typedef D_UINT32 DclFtpReply;


/*  DclFtpFsInstanceData

    Data associated with a single instance of the FTP file system
*/
#define REPLY_BUFFER            512
struct DclFtpFsInstanceData {
    DCLNETSOCKET Socket;
    D_BOOL fLogout;
    char * szAddress;
    char * szUser;
    char * szPassword;
    D_UINT32 ulReply;
    char szLastReply[REPLY_BUFFER];
};


/*  DCLFTPFSFD

    Handle used for reading and writing to files on the FTP server
*/
struct DclFtpFsHandleData {
    DCLFTPFSINSTANCE hInstance;
    char * szPath;
    D_UINT32 ulOffset;
    D_TIME ullModifiedTime;
};

/* From ftplogin.c
*/
DCLNETSOCKET DclFtpConnect(
    const char * szAddress,
    D_UINT16 uPort);

DCLFTFSSTATUS DclFtpFsLogin(
    DCLFTPFSINSTANCE hInstance);

void DclFtpFsLogout(
    DCLFTPFSINSTANCE hInstance);

/* From ftphandle.c
*/
DCLFTPFSFD DclFtpHandleAllocate(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath);

void DclFtpHandleFree(
    DCLFTPFSFD * phFile);

DCLFTPFSFD DclFtpHandleAcquire(
    DCLFTPFSFD hFile);

void DclFtpHandleRelease(
    DCLFTPFSFD hFile);

/* From ftptxrx.c
*/
void DclFtpFsSend(
    DCLNETSOCKET Socket,
    void * pBuffer,
    D_UINT32 ulLength,
    D_UINT32 * pulTransfered);

void DclFtpFsReceive(
    DCLNETSOCKET Socket,
    void * pBuffer,
    D_UINT32 ulMinLength,
    D_UINT32 ulMaxLength,
    D_UINT32 ulTimeOutMilliseconds,
    D_UINT32 * pulTransfered);

DclFtpReply DclFtpFsGetReply(
    DCLFTPFSINSTANCE hInstance);

DclFtpReply DclFtpFsSendText(
    DCLFTPFSINSTANCE hInstance,
    char * szCommand);

DCLNETSOCKET DclFtpFsDataSocketOpen(
    DCLFTPFSINSTANCE hInstance);

void DclFtpFsReplyGetFileSize(
    DCLFTPFSINSTANCE hInstance,
    D_UINT32 * pulFileSize);

void DclFtpFsReplyGetDate(
    DCLFTPFSINSTANCE hInstance,
    D_TIME * pullModifiedTime);


#endif /* #ifndef FTPFSPRIV_H_INCLUDE */
