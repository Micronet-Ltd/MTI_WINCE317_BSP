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

    This module contains functions to manage serial ports under Windows.

    This code must be fully compilable and usable under both normal Win32
    user mode, as well as Windows CE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: wincomm.c $
    Revision 1.2  2010/11/01 04:16:05Z  garyp
    Documentation and debug messages updated -- no functional changes.
    Revision 1.1  2008/11/05 01:20:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>

#include <dcl.h>
#include <dlwinutil.h>


/*-------------------------------------------------------------------
    Public: DclWinCommOpen()

    Open the specified serial port.

    Parameters:
        ptzCommName - A pointer to the port name, in the form "COM1:"

    Return Value:
        Returns the handle value if successful, or
        INVALID_HANDLE_VALUE otherwise.
-------------------------------------------------------------------*/
HANDLE DclWinCommOpen(
    const WCHAR    *ptzCommName)
{
    HANDLE          hComm;

    DclAssert(ptzCommName);

    hComm = CreateFile( ptzCommName, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hComm == INVALID_HANDLE_VALUE)
        DCLPRINTF(1, ("DclWinCommOpen() Error %lU opening COMM port \"%W\"\n", GetLastError(), ptzCommName));

    return hComm;
}


/*-------------------------------------------------------------------
    Public: DclWinCommClose()

    Close a serial port.

    Parameters:
        hComm     - The COMM port handle which must have been opened
                    with CreateFile().

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinCommClose(
    HANDLE          hComm)
{
    DclAssert(hComm != INVALID_HANDLE_VALUE);

    if(!CloseHandle(hComm))
    {
        DCLPRINTF(1, ("DclWinCommClose() Error %lU closing COMM port handle %lU\n", GetLastError(), hComm));
        return DCLSTAT_WIN_COMMCLOSEFAILED;
    }
    else
    {
        return DCLSTAT_SUCCESS;
    }
}


/*-------------------------------------------------------------------
    Public: DclWinCommParamInit()

    Initialize an already open COMM port with the specified values.
    These values must match legitimate values for the DCB structure,
    as documented by Microsoft.  See the Microsoft documentation or
    header files for more information.

    Parameters:
        hComm     - The COMM port handle which must have been opened
                    with CreateFile().
        ulBaud    - The baud rate to use or D_UINT32_MAX to leave the
                    setting unchanged.
        bParity   - The parity value to use or D_UINT8_MAX to leave
                    the setting unchanged.
        bDataBits - The data bits value to use or D_UINT8_MAX to
                    leave the setting unchanged.
        bStopBits - The stop bits value to use or D_UINT8_MAX to
                    leave setting unchanged.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclWinCommParamInit(
    HANDLE      hComm,
    D_UINT32    ulBaud,
    D_BYTE      bParity,
    D_BYTE      bDataBits,
    D_BYTE      bStopBits)
{
    DCB         dcb;

    if(!GetCommState(hComm, &dcb))
    {
        DCLPRINTF(1, ("DclWinCommParamInit() Error %lU getting COMM state\n", GetLastError()));
        return DCLSTAT_WIN_COMMGETPARAMSFAILED;
    }

  #if D_DEBUG
    DclPrintf("Old COMM parameters:\n");
    DclWinCommParamDump(&dcb, TRUE);
  #endif

    if(ulBaud != D_UINT32_MAX)
        dcb.BaudRate = ulBaud;

    if(bParity != D_UINT8_MAX)
        dcb.Parity = bParity;

    if(bDataBits != D_UINT8_MAX)
        dcb.ByteSize = bDataBits;

    if(bStopBits != D_UINT8_MAX)
        dcb.StopBits = bStopBits;

    if(!SetCommState(hComm, &dcb))
    {
        DCLPRINTF(1, ("DclWinCommParamInit() Error %lU setting COMM state\n", GetLastError()));
        return DCLSTAT_WIN_COMMGETPARAMSFAILED;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: DclWinCommParamDump()

    Display COMM port parameters as specified in the supplied DCB
    structure.

    Parameters:
        pDCB       - A pointer to the DCB structure to dump.
        fVerbose   - TRUE to display a full dump of the entire
                     structure.

    Return Value:
        None.
-------------------------------------------------------------------*/
void DclWinCommParamDump(
    const DCB  *pDCB,
    unsigned    fVerbose)
{
    char        szParity[16];
    char        szStopBits[16];

    DclAssert(pDCB);

    switch(pDCB->Parity)
    {
        case NOPARITY:
            DclStrNCpy(szParity, "None", sizeof(szParity));
            break;
        case EVENPARITY:
            DclStrNCpy(szParity, "Even", sizeof(szParity));
            break;
        case ODDPARITY:
            DclStrNCpy(szParity, "Odd", sizeof(szParity));
            break;
        case MARKPARITY:
            DclStrNCpy(szParity, "Mark", sizeof(szParity));
            break;
        case SPACEPARITY:
            DclStrNCpy(szParity, "Space", sizeof(szParity));
            break;
        default:
            DclSNPrintf(szParity, sizeof(szParity), "??(%u)", pDCB->Parity);
            break;
    }

    switch(pDCB->StopBits)
    {
        case ONESTOPBIT:
            DclStrNCpy(szStopBits, "1", sizeof(szStopBits));
            break;
        case ONE5STOPBITS:
            DclStrNCpy(szStopBits, "1.5", sizeof(szStopBits));
            break;
        case TWOSTOPBITS:
            DclStrNCpy(szStopBits, "2", sizeof(szStopBits));
            break;
        default:
            DclSNPrintf(szStopBits, sizeof(szStopBits), "??(%u)", pDCB->StopBits);
            break;
    }

    DclPrintf("Baud=%lU Parity=%s DataBits=%u StopBits=%s\n",
        pDCB->BaudRate, szParity, pDCB->ByteSize, szStopBits);

    if(fVerbose)
    {
        DclPrintf("Parity=%u Binary=%u CTSFlow=%u DSRFlow=%u DTRCtrl=%u DSRSens=%u TXContX=%u OutX=%u InX=%u\n",
            pDCB->fParity, pDCB->fBinary, pDCB->fOutxCtsFlow, pDCB->fOutxDsrFlow, pDCB->fDtrControl, pDCB->fDsrSensitivity, pDCB->fTXContinueOnXoff, pDCB->fOutX, pDCB->fInX);
        DclPrintf("ErrChr=%u Null=%u RTRCtrl=%u AbortErr=%u Dummy=%lX XonLim=%X XoffLim=%X\n",
            pDCB->fErrorChar, pDCB->fNull, pDCB->fRtsControl, pDCB->fAbortOnError, pDCB->fDummy2, pDCB->XonLim, pDCB->XoffLim);
        DclPrintf("XonChr=%u XoffChr=%u ErrorChr=%u EofChr=%u EvtChr=%u Reserved=%X Reserved1=%X\n",
            pDCB->XonChar, pDCB->XoffChar, pDCB->ErrorChar, pDCB->EofChar, pDCB->EvtChar, pDCB->wReserved, pDCB->wReserved1);
    }
}



