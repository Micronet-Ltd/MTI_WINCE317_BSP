/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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
    $Log: winmsgdecode.c $
    Revision 1.1  2008/07/23 05:06:48Z  garyp
    Initial revision
    07/16/08 gp  Migrated from the WinLight project
    01/22/01 gp  Updated to use STDRTL.DLL
    07/21/99 gp  PT file update
    05/12/99 gp  Updated to use the WinLight standard Assert.
    03/22/99 gp  Updated for a renamed structure member
    07/17/98 gp  Obsoleted the TRID stuff
    06/25/98 gp  Fixed a bug in the WM_NCCREATE handler
    01/05/98 gp  Changed TrkGetMessageName() to display message greater than
                 WM_USER as "WM_USER+n".
    08/05/97 gp  Added DISPLAY related stuff
    07/09/97 gp  Added many more handlers
    06/23/97 gp  Created.
--------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include "winmsgdecode.h"


/*-------------------------------------------------------------------
    This function takes a message number and returns the message
    name.

    If the message number is greater than or equal to WM_USER, an
    output string in the form "WM_USER+n" will be generated.

    If the message number is less than WM_USER, but not found in
    our list of known messages, an output string in the form
    "WM_?? 0x12345678" will be returned.

    In the event that a WM_USER or WM_?? message is returned, the
    caller <must> use the value immediately, or risk the possibility
    that a subsequent WM_USER or WM_?? message will stomp on the
    static buffer.
-------------------------------------------------------------------*/
const char * DclWinMessageName(
    UINT        nMsg)
{
    unsigned    nIndex = 0;

    if(nMsg >= WM_USER)
    {
        /*  Use a static buffer to return the formatted value.  Since this
            is intended to only be used in debug code, we'll take the risk.
        */
        static char achTempBuff[20];    /* enough space to hold a message name */

        DclSNPrintf(achTempBuff, sizeof achTempBuff, "WM_USER+%u", nMsg-WM_USER);

        return achTempBuff;
    }

    /*  loop as long as the pszMsgName is valid
    */
    while(aMsgInfo[nIndex].pszMsgName)
    {
        if(aMsgInfo[nIndex].nMsgNum == nMsg)
            return aMsgInfo[nIndex].pszMsgName;

        nIndex++;
    }

    /*  msg number wasn't found, so generate one
    */
    {
        /*  Use a separate static buffer from the WM_USER case above, just
            to reduce the possibility of two messages stomping on each other.
        */
        static char achTempBuff[20];    /* enough space to hold a message name */

        DclSNPrintf(achTempBuff, sizeof achTempBuff, "WM_?? %x", nMsg);

        return achTempBuff;
    }
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
char * DclWinMessageFormat(
    char       *pBuffer,
    unsigned    nBufferLen,
    HWND        hWnd,
    UINT        nMsg,
    WPARAM      wParam,
    LPARAM      lParam)
{
    int         iLen;
    char       *pBuffPos = pBuffer;

    DclAssert(pBuffer);
    DclAssert(nBufferLen);

    iLen = DclSNPrintf(pBuffPos, nBufferLen, "hWnd=%lX %s", hWnd, DclWinMessageName(nMsg));

    if((iLen == -1) || ((unsigned)iLen >= nBufferLen-1))
    {
        *(pBuffPos + nBufferLen - 1) = 0;
        return pBuffer;
    }

    pBuffPos += iLen;
    nBufferLen -= iLen;

    switch(nMsg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            DclSNPrintf(pBuffPos, nBufferLen, " vkCode=%lX lParam=%lX", wParam, lParam);
            break;

        case WM_KILLFOCUS:
        case WM_NCACTIVATE:
            DclSNPrintf(pBuffPos, nBufferLen, " wParam=%lX", wParam);
            break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
            DclSNPrintf(pBuffPos, nBufferLen, " Flags=%lX X=%D Y=%D",
                wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOVE:
        case WM_NCHITTEST:
            DclSNPrintf(pBuffPos, nBufferLen, " X=%D Y=%D",
                LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_NCMOUSEMOVE:
            DclSNPrintf(pBuffPos, nBufferLen, " wParam=%lX X=%D Y=%D",
                wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_SIZE:
            DclSNPrintf(pBuffPos, nBufferLen, " Flags=%lX nWidth=%D hHeight=%D",
                wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_TIMER:
            DclSNPrintf(pBuffPos, nBufferLen, " ID=%lX lParam=%lX",
                wParam, lParam);
            break;

        case WM_COMMAND:
            DclSNPrintf(pBuffPos, nBufferLen, " Code=%U ID=%U lParam=%lX",
                HIWORD(wParam), LOWORD(wParam), lParam);
            break;

        /*  generic display for messages we don't handle yet
        */
        default:
            DclSNPrintf(pBuffPos, nBufferLen, " wParam=%lX lParam=%lX",
                wParam, lParam);
            break;
    }

    /*  Always be certain the buffer is null-terminated
    */
    *(pBuffPos + nBufferLen - 1) = 0;

    return pBuffer;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
char * DclWinMsgFormat(
    char       *pBuffer,
    unsigned    nBufferLen,
    const MSG  *pMsg)
{
    unsigned    nLen;
    char       *pBuffPos = pBuffer;

    DclAssert(pBuffer);
    DclAssert(nBufferLen);
    DclAssert(pMsg);

    DclWinMessageFormat(pBuffer, nBufferLen, pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);

    nLen = DclStrLen(pBuffer);

    if(nLen >= nBufferLen-1)
    {
        *(pBuffPos + nBufferLen - 1) = 0;
        return pBuffer;
    }

    pBuffPos += nLen;
    nBufferLen -= nLen;

    DclSNPrintf(pBuffPos, nBufferLen, " Time=%lU X=%lD Y=%lD", pMsg->time, pMsg->pt.x, pMsg->pt.y);

    /*  Always be certain the buffer is null-terminated
    */
    *(pBuffPos + nBufferLen - 1) = 0;

    return pBuffer;
}


