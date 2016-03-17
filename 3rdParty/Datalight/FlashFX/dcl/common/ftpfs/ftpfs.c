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
    $Log: ftpfs.c $
    Revision 1.3  2009/07/29 18:26:16Z  johnbr
    Changes for the power cycling tests.
    Revision 1.2  2009/07/15 17:59:11Z  keithg
    Updated to use renamed types, removed obsolete include file; Changed
    the Ftp prefix to FtpFs out of consistency.
    Revision 1.1  2009/07/15 06:51:44Z  keithg
    Initial revision
    Revision 1.4  2009/03/18 02:59:17Z  brandont
    Updated to use the current DCL file system abstraction.
    Revision 1.3  2007/04/29 09:52:29Z  brandont
    Changed to reconnect after timeout.
    Revision 1.2  2007/04/25 01:27:43Z  brandont
    Added TRACEPRINTF calls.
    Revision 1.1  2007/04/14 19:20:16Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlnet.h>
#include "ftpfspriv.h"


/*---------------------------------------------------------------------------
    Private: DclFtpFsMkdir()

    Create a directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsMkdir(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACEBIT_FTPFS, 1, TRACEINDENT),
            "DclFtpFsMkdir() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Make sure the FTP connection has not timed out
    */
    ulResult = DclFtpFsLogin(hInstance);
    if(ulResult != 0)
    {
        goto Done;
    }


    /*  Create the directory
    */
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "MKD %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if((ulReply != 250) && (ulReply != 257))
    {
        ulResult = -1;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsMkdir() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*-------------------------------------------------------------------
    Protected: DclFtpFsRmdir()

    Remove a directory on the FTP server

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsRmdir(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "(DclFtpFsRmdir) Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Make sure the FTP connection has not timed out
    */
    ulResult = DclFtpFsLogin(hInstance);
    if(ulResult != 0)
    {
        goto Done;
    }


    /*  Remove the directory
    */
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "RMD %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 250)
    {
        ulResult = -1;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsRmdir() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsStat()

    Retrieve information about a file or directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsStat(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath,
    DCLFSSTAT * pStat)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsStat() Instance=%P Path=%s pStat=%P\n",
            hInstance,
            szPath,
            pStat));

    if(hInstance == NULL)
    {
        ulResult = -1;
        goto Done;
    }

    /*  Make sure the FTP connection has not timed out
    */
    ulResult = DclFtpFsLogin(hInstance);
    if(ulResult != 0)
    {
        goto Done;
    }


    /*  Get the modified date of the file

        NOTE: FTP only supports the modified date so creation and last
            access date will be set to modified date.
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE A\r\n");
    if(ulReply != 200)
    {
        ulResult = -1;
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "MDTM %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 213)
    {
        ulResult = -1;
        goto Done;
    }
    DclFtpFsReplyGetDate(hInstance, &pStat->tCreation);
    DclFtpFsReplyGetDate(hInstance, &pStat->tAccess);
    DclFtpFsReplyGetDate(hInstance, &pStat->tModify);


    /*  Attempt to get the file size
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        ulResult = -1;
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "SIZE %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply == 213)
    {
        /*  The path is for a file
        */
        DclFtpFsReplyGetFileSize(hInstance, &pStat->ulSize);
        pStat->ulAttributes = DCLFSATTR_ARCHIVE;
    }
    else if(ulReply == 550)
    {
        /*  The path is for a directory
        */
        pStat->ulSize = 0;
        pStat->ulAttributes = DCLFSATTR_DIRECTORY;
    }
    else
    {
        ulResult = -1;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsStat() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsCreate()

    Create a file for writing on the FTP server.

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSFD DclFtpFsCreate(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTPFSFD hFile;
    DCLNETSOCKET DataSocket;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsCreate() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Allocate a handle
    */
    hFile = DclFtpHandleAllocate(hInstance, szPath);
    if(!hFile)
    {
        goto Done;
    }


    /*  Create the file

        Note: No data is written at this point but the file will be
            truncated if it exists.  It may also fail here if the path
            is invalid or a directory exists with the same name.
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DataSocket = DclFtpFsDataSocketOpen(hInstance);
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "STOR %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 150)
    {
        DclNetCloseSocket(DataSocket);
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclNetCloseSocket(DataSocket);
    ulReply = DclFtpFsGetReply(hInstance);
    if(ulReply != 226)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }


    /*  Save the modified date of the file
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE A\r\n");
    if(ulReply != 200)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "MDTM %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 213)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclFtpFsReplyGetDate(hFile->hInstance, &hFile->ullModifiedTime);

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsCreate() Fd=%P\n",
            hFile));
    return hFile;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsOpen()

    Open a file for reading and writing on the FTP server.

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSFD DclFtpFsOpen(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTPFSFD hFile;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsOpen() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Allocate a handle
    */
    hFile = DclFtpHandleAllocate(hInstance, szPath);
    if(!hFile)
    {
        goto Done;
    }


    /*  Save the modified date of the file
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE A\r\n");
    if(ulReply != 200)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "MDTM %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 213)
    {
        /*  Does not exist
        */
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclFtpFsReplyGetDate(hFile->hInstance, &hFile->ullModifiedTime);


    /*  Make sure this is a file
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        DclFtpHandleFree(&hFile);
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "SIZE %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 213)
    {
        /*  Not a file
        */
        DclFtpHandleFree(&hFile);
        goto Done;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsOpen() Fd=%P\n",
            hFile));
    return hFile;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsRead()

    Read from a file on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsRead(
    DCLFTPFSFD hFile,
    void * pBuffer,
    D_UINT32 ulLength,
    D_UINT32 * pulTransfered)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLNETSOCKET DataSocket;
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsRead() Fd=%P Buffer=%P Length=%lU\n",
            hFile,
            pBuffer,
            ulLength));


    /*  Acquire the handle
    */
    *pulTransfered = 0;
    hFile = DclFtpHandleAcquire(hFile);
    if(!hFile)
    {
        ulResult = -1;
        goto Done;
    }


    /*  Request the file data to be transfered
    */
    ulReply = DclFtpFsSendText(hFile->hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        ulResult = -1;
        goto OnError;
    }
    DataSocket = DclFtpFsDataSocketOpen(hFile->hInstance);
    if(DataSocket < 0)
    {
        ulResult = -1;
        goto OnError;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "REST %lu\r\n", hFile->ulOffset);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 350)
    {
        DclNetCloseSocket(DataSocket);
        ulResult = -1;
        goto OnError;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "RETR %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 150)
    {
        DclNetCloseSocket(DataSocket);
        ulResult = -1;
        goto OnError;
    }


    /*  Transfer the requested amount
    */
    DclFtpFsReceive(
            DataSocket,
            pBuffer,
            ulLength,
            ulLength,
            1000,
            pulTransfered);
    hFile->ulOffset += *pulTransfered;
    DclNetCloseSocket(DataSocket);
    ulReply = DclFtpFsGetReply(hFile->hInstance);
    if(ulReply != 226)
    {
        ulResult = -1;
        goto OnError;
    }


OnError:

    /*  Release the handle
    */
    DclFtpHandleRelease(hFile);

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsRead() Status=%lU Transfered=%lU\n",
            ulResult,
            *pulTransfered));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsWrite()

    Write to a file on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsWrite(
    DCLFTPFSFD hFile,
    void * pBuffer,
    D_UINT32 ulLength,
    D_UINT32 * pulTransfered)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLNETSOCKET DataSocket;
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsWrite() Fd=%P Buffer=%P Length=%lU\n",
            hFile,
            pBuffer,
            ulLength));


    /*  Acquire the handle
    */
    *pulTransfered = 0;
    hFile = DclFtpHandleAcquire(hFile);
    if(!hFile)
    {
        ulResult = -1;
        goto Done;
    }


    /*  Request the file data to be transfered
    */
    ulReply = DclFtpFsSendText(hFile->hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        ulResult = -1;
        goto OnError;
    }
    DataSocket = DclFtpFsDataSocketOpen(hFile->hInstance);
    if(DataSocket < 0)
    {
        ulResult = -1;
        goto OnError;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "REST %lu\r\n", hFile->ulOffset);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 350)
    {
        DclNetCloseSocket(DataSocket);
        ulResult = -1;
        goto OnError;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "STOR %s\r\n", hFile->szPath);
    ulReply = DclFtpFsSendText(hFile->hInstance, szCommand);
    if(ulReply != 150)
    {
        DclNetCloseSocket(DataSocket);
        ulResult = -1;
        goto OnError;
    }

    /*  Transfer the requested amount
    */
    DclFtpFsSend(
           DataSocket,
           pBuffer,
           ulLength,
           pulTransfered);
    hFile->ulOffset += *pulTransfered;
    DclNetCloseSocket(DataSocket);
    ulReply = DclFtpFsGetReply(hFile->hInstance);
    if(ulReply != 226)
    {
        ulResult = -1;
        goto OnError;
    }

OnError:

    /*  Release the handle
    */
    DclFtpHandleRelease(hFile);

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsWrite() Status=%lU Transfered=%lU\n",
            ulResult,
            *pulTransfered));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsClose()

    Close a file open for reading and writing on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsClose(
    DCLFTPFSFD hFile)
{
    DCLFTFSSTATUS ulResult = 0;

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsClose() Fd=%P\n",
            hFile));


    hFile = DclFtpHandleAcquire(hFile);
    if(!hFile)
    {
        ulResult = -1;
        goto Done;
    }
    DclFtpHandleFree(&hFile);

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsClose() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsDelete()

    Delete a file on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsDelete(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTFSSTATUS ulResult = 0;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsDelete() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Make sure the FTP connection has not timed out
    */
    ulResult = DclFtpFsLogin(hInstance);
    if(ulResult != 0)
    {
        goto Done;
    }


    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "DELE %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 250)
    {
        ulResult = -1;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsDelete() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsRename()

    Rename a file or directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsRename(
    DCLFTPFSINSTANCE hInstance,
    const char * szOldPath,
    const char * szNewPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLFTFSSTATUS ulResult = 0;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsRename() Instance=%P SrcPath=%s DstPath=%s\n",
            hInstance,
            szOldPath,
            szNewPath));

    /*  Make sure the FTP connection has not timed out
    */
    ulResult = DclFtpFsLogin(hInstance);
    if(ulResult != 0)
    {
        goto Done;
    }


    /*  Rename the file or directory
    */
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "RNFR %s\r\n", szOldPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 350)
    {
        ulResult = -1;
        goto Done;
    }
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "RNTO %s\r\n", szNewPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 250)
    {
        ulResult = -1;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsRename() Status=%lU\n",
            ulResult));
    return ulResult;
}


/*  DclFtpFsDirEntryData
*/
struct DclFtpFsDirEntryData{
    char * szName;
};


/*  DclFtpFsDirData
*/
struct DclFtpFsDirData{
    char * szDir;
    D_UINT32 ulSize;
    D_UINT32 ulOffset;
    struct DclFtpFsDirEntryData Entry;
};


#if 0
/*  DclFtpFsDirEntryData
*/
typedef char * DclFtpFsDirEntryData;
#endif


#define DIR_ALLOC_CHUNK             512
/*---------------------------------------------------------------------------
    Private: DclFtpFsOpenDir()

    Open a directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSDIR DclFtpFsOpenDir(
    DCLFTPFSINSTANCE hInstance,
    const char * szPath)
{
    char szCommand[FTP_COMMAND_LENGTH];
    DclFtpReply ulReply;
    DCLNETSOCKET DataSocket;
    D_UINT32 ulTransfered;
    void * pNewBuffer;
    DCLFTPFSDIR hDir;
    DCLFTFSSTATUS ulStatus;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsOpenDir() Instance=%P Path=%s\n",
            hInstance,
            szPath));


    /*  Make sure the FTP connection has not timed out
    */
    ulStatus = DclFtpFsLogin(hInstance);
    if(ulStatus != 0)
    {
        hDir = (void *)0;
        goto Done;
    }


    /*  Allocate an structure to manage this opendir
    */
    hDir = DclMemAllocZero(sizeof(*hDir));
    if(!hDir)
    {
        goto Done;
    }


    /*  Allocate a buffer to store all the names in the directory

        NOTE: This will grow as needed
    */
    hDir->szDir = DclMemAllocZero(DIR_ALLOC_CHUNK);
    if(!hDir->szDir)
    {
        DclMemFree(hDir);
        hDir = (void *)0;
        goto Done;
    }


    /*
    */
    ulReply = DclFtpFsSendText(hInstance, "TYPE I\r\n");
    if(ulReply != 200)
    {
        goto Done;
    }
    DataSocket = DclFtpFsDataSocketOpen(hInstance);
    DclSNPrintf(szCommand, DCLDIMENSIONOF(szCommand), "NLST %s\r\n", szPath);
    ulReply = DclFtpFsSendText(hInstance, szCommand);
    if(ulReply != 150)
    {
        DclNetCloseSocket(DataSocket);
        goto Done;
    }


    /*  Read the directory contents
    */
    while(TRUE)
    {
        /*  Read a portion of the directory
        */
        DclFtpFsReceive(
                DataSocket,
                hDir->szDir + hDir->ulSize,
                0,
                DIR_ALLOC_CHUNK,
                1000,
                &ulTransfered);
        hDir->ulSize += ulTransfered;
        if(ulTransfered == DIR_ALLOC_CHUNK)
        {
            /*  Grow the buffer for the directory
            */
            pNewBuffer = DclMemAllocZero(hDir->ulSize + DIR_ALLOC_CHUNK);
            if(!pNewBuffer)
            {
                DclMemFree(hDir->szDir);
                DclMemFree(hDir);
                hDir = (void *)0;
                break;
            }
            DclMemCpy(pNewBuffer, hDir->szDir, hDir->ulSize);
            DclMemFree(hDir->szDir);
            hDir->szDir = pNewBuffer;
        }
        else
        {
            break;
        }
    }


    DclNetCloseSocket(DataSocket);
    ulReply = DclFtpFsGetReply(hInstance);
    if(ulReply != 226)
    {
        DclMemFree(hDir->szDir);
        DclMemFree(hDir);
        hDir = (void *)0;
        goto Done;
    }

Done:

    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsOpenDir() hDir=%P\n",
            hDir));
    return hDir;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsReadDir()

    Read from a directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTPFSDIRENTRY DclFtpFsReadDir(
    DCLFTPFSDIR hDir)
{
    DCLFTPFSDIRENTRY hEntry = (void *)0;
    D_UINT32 ulIndex;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsReadDir() hDir=%P\n",
            hDir));


    for(ulIndex = hDir->ulOffset; ulIndex < hDir->ulSize; ulIndex++)
    {
        if(hDir->szDir[ulIndex] == '\r')
        {
            hDir->szDir[ulIndex] = 0;
            hDir->Entry.szName = &hDir->szDir[hDir->ulOffset];
            hDir->ulOffset = ulIndex + 2;
            hEntry = &hDir->Entry;
            break;
        }
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsReadDir() hDirEntry=%P\n",
            hEntry));
    return hEntry;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsGetEntryName()

    Retrieve the entry name from the last read directory on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsGetEntryName(
    DCLFTPFSDIRENTRY hDirEntry,
    char * szEntryName)
{
    DCLFTFSSTATUS ulResult;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsGetEntryName() hDirEntry=%P\n",
            hDirEntry));


    if(hDirEntry)
    {
        DclStrCpy(szEntryName, hDirEntry->szName);
        ulResult = 0;
    }
    else
    {
        *szEntryName = 0;
        ulResult = -1;
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsGetEntryName() Status=%lU Path=%s\n",
            ulResult,
            szEntryName));
    return ulResult;
}


/*---------------------------------------------------------------------------
    Private: DclFtpFsCloseDir()

    Close a directory opened for reading on the FTP server

    Parameters:

    Return Value:
---------------------------------------------------------------------------*/
DCLFTFSSTATUS DclFtpFsCloseDir(
    DCLFTPFSDIR hDir)
{
    DCLFTFSSTATUS ulResult;


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEINDENT),
            "DclFtpFsCloseDir() hDir=%P\n",
            hDir));


    if(hDir)
    {
        DclMemFree(hDir->szDir);
        DclMemFree(hDir);
        ulResult = 0;
    }
    else
    {
        ulResult = -1;
    }


    DCLTRACEPRINTF((MAKETRACEFLAGS(DCLTRACE_FTPFS, 1, TRACEUNDENT),
            "DclFtpFsCloseDir() Status=%lU\n",
            ulResult));
    return ulResult;
}


