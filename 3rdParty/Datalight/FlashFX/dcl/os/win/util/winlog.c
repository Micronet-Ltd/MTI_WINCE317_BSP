/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This module contains functions to facilitate logging in a Windows
    environment.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History   
    $Log: winlog.c $
    Revision 1.3  2010/11/01 04:18:00Z  garyp
    Fixed DclWinLogOpen() to properly support appending to a file.  Modified
    DclWinLogClose() to attempt to flush any data before closing the file.
    Revision 1.2  2009/02/08 05:09:31Z  garyp
    Define DCLOUTPUT_BUFFERLEN if necessary.
---------------------------------------------------------------------------*/

#include <windows.h>
#ifndef _WIN32_WCE
#include <stdio.h>      /* printf() */
#endif

#include <dcl.h>
#include <dlprintf.h>
#include <dlwinutil.h>

/*  This is not a "standard" output mechanism, so even if output is
    disabled using normal mechanisms, still allow this to work.
*/
#ifndef DCLOUTPUT_BUFFERLEN
#define DCLOUTPUT_BUFFERLEN     (256)
#endif


/*-------------------------------------------------------------------
    Public: DclWinLogOpen()

    Open the specified device/file for use logging output.  The 
    handle must be closed with DclWinLogClose().

    This function may be used to open a serial port for output if
    the name specified has the form: "COMn:[baud]".  If the baud
    rate is not specified, the default setting will be used.

    Parameters:
        ptzName     - A pointer to the name of the file or device to
                      open.
        fAppend     - TRUE to append to the file, or FALSE to replace
                      any pre-existing file.  This flag is ignored if
                      it is a COMM port which is opened.

    Return Value:
        If successful, returns a file handle suitable for use with
        DclWinLogPrintf(), or INVALID_HANDLE_VALUE otherwise.
-------------------------------------------------------------------*/
HANDLE DclWinLogOpen(
    const D_WCHAR  *ptzName,
    D_BOOL          fAppend)
{
    HANDLE          hFile = INVALID_HANDLE_VALUE;
    D_BOOL          fIsComm = FALSE;

    DclAssert(ptzName);

    /*  If the name starts out in the form "COMn:", where 'n' is a number
        from 0 to 9, it must be a COMM port which is being opened.
    */
    if((_wcsnicmp(ptzName, TEXT("COM"), 3) == 0) &&
        (ptzName[3] >= L'0') &&
        (ptzName[3] <= L'9') &&
        (ptzName[4] == L':'))
    {
        D_WCHAR     tzCommName[16];
        D_UINT32    ulBaud = D_UINT32_MAX;

        fIsComm = TRUE;

        /*  Make a local copy of the port name so we can ensure it
            is null-terminated.
        */
        wcsncpy(tzCommName, ptzName, DCLDIMENSIONOF(tzCommName));
        tzCommName[5] = 0;

        /*  See if a baud rate was specified, and if so, grab it
        */
        if(ptzName[5] != 0)
        {
            D_WCHAR    *ptzEnd;

            ulBaud = wcstoul(&ptzName[5], &ptzEnd, 10);
            if(ulBaud == 0 || ulBaud == D_UINT32_MAX || *ptzEnd != 0)
            {
                DCLPRINTF(1, ("DclWinLogOpen() Can't determine baud rate from \"%W\"\n", &ptzName[5]));
                return hFile;
            }
        }

        hFile = DclWinCommOpen(tzCommName);
        if(hFile == INVALID_HANDLE_VALUE)
        {
            DclPrintf("Unable to open COMM port \"%W\"\n", tzCommName);
            return hFile;
        }

        /*  If a baud rate was specified, modify the port parameters to use it.
        */
        if(ulBaud != D_UINT32_MAX)
        {
            DCLSTATUS   dclStat;

            dclStat = DclWinCommParamInit(hFile, ulBaud, D_UINT8_MAX, D_UINT8_MAX, D_UINT8_MAX);
            if(dclStat != DCLSTAT_SUCCESS)
            {
                DclPrintf("Error %lX setting COMM port parameters\n", dclStat);

                DclWinCommClose(hFile);
                return INVALID_HANDLE_VALUE;
            }
        }
    }
    else
    {
        /*  It was not a COMM port which was specified, so just open the
            specified file name.
        */
        hFile = CreateFile( ptzName,
                            GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if(hFile != INVALID_HANDLE_VALUE)
        {
            if(fAppend && !fIsComm)
                 SetFilePointer(hFile, 0, NULL, FILE_END);
        }
        else
        {
            DCLPRINTF(1, ("DclWinLogOpen() Error %lU opening file \"%W\"\n", GetLastError(), ptzName));
        }
    }

    return hFile;
}


/*-------------------------------------------------------------------
    Public: DclWinLogClose()

    Close a handle which was opened with DclWinLogOpen().

    Parameters:
        hLogFile - The handle to close

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinLogClose(
    HANDLE  hLogFile)
{
    DclAssert(hLogFile != INVALID_HANDLE_VALUE);

    /*  ToDo: This function cannot distinguish between handles which
              were opened with CreateFile() and DclWinCommOpen().
              Fortunately they both resolved to CloseHandle(), so
              this solution is "ok" for now.  In the long term we
              probably want to keep some state information...
    */

    /*  Allow the COMM port to finish -- need to be able to distinguish
        between COMM ports and files, and know whether they are done
        with their I/O or not...
    */
    FlushFileBuffers(hLogFile);
    DclOsSleep(500); 
    
    if(!CloseHandle(hLogFile))
    {
        DCLPRINTF(1, ("DclWinLogClose() Error %lU closing handle %lU\n", GetLastError(), hLogFile));
        return DCLSTAT_WIN_CLOSEHANDLEFAILED;
    }
    else
    {
        return DCLSTAT_SUCCESS;
    }
}


/*-------------------------------------------------------------------
    Public: DclWinLogPrintf()

    Write logging data to the file/device denoted by the supplied
    handle.

    Note that even if the hLogFile parameter is INVALID_HANDLE_VALUE,
    this function <may> still redirect the output via the C-library
    printf() function, if this is real Win32 (not CE).

    Note that the return value pertains to the length written to
    the supplied file handle, if any.  Any output written via the
    printf() method described above is not considered, even if that
    is the only output written.

    Parameters:
        hLogFile    - The handle returned by DclWinLogOpen().
        pszFmtStr   - The standard Datalight style printf() format
                      string.
        ...         - The variable argument list

    Return Value:
        Returns the length output, or -1 if the buffer overflowed.
-------------------------------------------------------------------*/
int DclWinLogPrintf(
    HANDLE          hLogFile,
    const char     *pszFmtStr,
    ...)
{
    int             iLen = 0;
    int             iWritten = 0;
    va_list         arglist;
    char            achBuffer[DCLOUTPUT_BUFFERLEN];

    DclAssert(pszFmtStr);

    va_start(arglist, pszFmtStr);

    iLen = DclVSNPrintf(achBuffer, sizeof(achBuffer), pszFmtStr, arglist);

    DclAssert(iLen < sizeof(achBuffer));

    if(iLen < 0)
    {
        /*  DclSNPrintf() will return -1 if we ran out of buffer space.
            If this happens truncate the data.
        */

        /*  Return the full buffer length, minus the null-terminator
        */
        iLen = sizeof(achBuffer) - 1;

        /*  Ensure we are null terminated.
        */
        achBuffer[iLen] = 0;

        /*  If the original string was \n terminated and the new one is
            not, due to truncation, stuff a \n into the new one.
        */
        if(pszFmtStr[DclStrLen(pszFmtStr)-1] == '\n')
            achBuffer[iLen-1] = '\n';
    }

  #ifndef _WIN32_WCE
    /*  This printf() causes the data to go to the stdout, which for a
        Windows app is the bit bucket -- unless redirection is used, in
        which case everything can be captured to a file.  Very handy
        for debugging, however it cannot be used under CE because it will
        cause a command prompt window to be spawned.
    */
    printf("%s", achBuffer);
  #endif

    if(hLogFile != INVALID_HANDLE_VALUE)
    {
        char *pBuffer = achBuffer;

        while(iLen)
        {
            DWORD       dwBytesWritten;
            unsigned    nLen = 0;

            while(pBuffer[nLen] != 0 && pBuffer[nLen] != '\n')
                nLen++;

            if(nLen)
            {
                if(!WriteFile(hLogFile, pBuffer, nLen, &dwBytesWritten, NULL))
                {
                    /* DclPrintf("Error %lU writing!\n", GetLastError());
                    DclError();*/
                }
                iWritten    += dwBytesWritten;
                pBuffer     += nLen;
                iLen        -= nLen;
            }

            /*  Replace each LF with a CR/LF combination
            */
            if(pBuffer[0] == '\n')
            {
                if(!WriteFile(hLogFile, "\r\n", 2, &dwBytesWritten, NULL))
                {
                    /* DclPrintf("Error %lU writing!\n", GetLastError());
                    DclError();*/
                }
                iWritten    += dwBytesWritten;
                pBuffer     += 1;
                iLen        -= 1;
            }
        }
    }

    va_end(arglist);

    return iWritten;
}


