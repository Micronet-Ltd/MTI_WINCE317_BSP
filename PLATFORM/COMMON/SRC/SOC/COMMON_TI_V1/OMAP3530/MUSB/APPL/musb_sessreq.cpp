// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------

#include <windows.h>
#include <winreg.h>
#include <winbase.h>
#include <string.h>
#include <devload.h>
#include <musbioctl.h>

void PrintHelp();

void PrintHelp()
{    
    NKDbgPrintfW(TEXT("Usage: musb_sessreq -[parmater 1]\r\n"));
    NKDbgPrintfW(TEXT(" [paramter 1]\r\n"));
    NKDbgPrintfW(TEXT(" E,e - Enable Session Request\r\n"));
    NKDbgPrintfW(TEXT(" D,d - Disable Session Request\r\n"));
    NKDbgPrintfW(TEXT(" Q,q - Query Controller Info\r\n"));
    NKDbgPrintfW(TEXT(" U,u - Query ULPI Info\r\n"));
    NKDbgPrintfW(TEXT(" L,l - Set Low Power Mode\r\n"));
    NKDbgPrintfW(TEXT(" H,h - Host Request Mode for HNP (peripheral mode only\r\n"));
    NKDbgPrintfW(TEXT(" S,s - Set Suspend Mode (host mode only) \r\n"));
    NKDbgPrintfW(TEXT(" R,r - Resume (host mode only) \r\n"));
    NKDbgPrintfW(TEXT(" ? - Help\r\n"));
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    DWORD dwErr = 0;
    HANDLE hHandle;
    DWORD dwCode = 0xFF;
    
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

    if( lpCmdLine == NULL || wcslen(lpCmdLine)==0)
    {
        NKDbgPrintfW(TEXT("Enable Session Req\r\n"));
        dwCode = IOCTL_USBOTG_REQUEST_SESSION_ENABLE;
    }
    else
    {      
        switch(lpCmdLine[1])
        {
            case 'e':
            case 'E':    
                NKDbgPrintfW(TEXT("Enable Session Req\r\n"));
                dwCode = IOCTL_USBOTG_REQUEST_SESSION_ENABLE;
                break;

            case 'd':
            case 'D':
                NKDbgPrintfW(TEXT("Disable Session Req\r\n"));
                dwCode = IOCTL_USBOTG_REQUEST_SESSION_DISABLE;
                break;

            case 'q':
            case 'Q':
                NKDbgPrintfW(TEXT("Query Session Req\r\n"));
                dwCode = IOCTL_USBOTG_REQUEST_SESSION_QUERY;
                break;

            case 'u':
            case 'U':
                NKDbgPrintfW(TEXT("Query ULPI Req\r\n"));
                dwCode = IOCTL_USBOTG_DUMP_ULPI_REG;
                break;

            case 'l':
            case 'L':
                NKDbgPrintfW(TEXT("Low POwer Mode\r\n"));
                dwCode = IOCTL_USBOTG_ULPI_SET_LOW_POWER_MODE;
                break;

            case 'H':
            case 'h':
                NKDbgPrintfW(TEXT("Host Mode Request\r\n"));
                dwCode = IOCTL_USBOTG_REQUEST_HOST_MODE;
                break;

            case 's':
            case 'S':
                NKDbgPrintfW(TEXT("Host Suspend\r\n"));
                dwCode = IOCTL_USBOTG_SUSPEND_HOST;
                break;

            case 'r':
            case 'R':
                NKDbgPrintfW(TEXT("Host Resume\r\n"));
                dwCode = IOCTL_USBOTG_RESUME_HOST;
                break;


            case '?':
            default:
                PrintHelp();
        }
    }

    hHandle = CreateFile(TEXT("OTG1:"), 
                                0,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);

    if (INVALID_HANDLE_VALUE == hHandle)
    {
        dwErr = GetLastError();
        NKDbgPrintfW(TEXT("Failed in open device file OTG: (Error = %d)"), dwErr);
        return 0;        
    }

    if( !DeviceIoControl (hHandle, dwCode, NULL, 0,  NULL, 0, NULL, NULL))
        NKDbgPrintfW(TEXT("Write Error\r\n"));
    else
        NKDbgPrintfW(TEXT("Write Success\r\n"));
    CloseHandle(hHandle);
    return 0;
}

