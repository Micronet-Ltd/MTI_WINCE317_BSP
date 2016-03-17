/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/
 
/** \file   Bluetooth.cpp 
 *  \brief  WL1271 bluetooth device management
 *
 *  \see    Bluetooth.h
 */


#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <winsock2.h>
#include <bt_api.h>
#include "Bluetooth.h"

#include <service.h>

#include <tchar.h>

#include "resource.h"

BOOL BthIoControl (DWORD dwCtl);


static HANDLE hBTDev = INVALID_HANDLE_VALUE;

BOOL BT_Init(void)
{
	RETAILMSG(1, (TEXT("++BT_Init\r\n")));

	// Open acces to the stack
	hBTDev = CreateFile (L"BTD0:", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);

	if(hBTDev == INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1,(TEXT("Unable to open access to the Bth Stack (error %ld)\n"), GetLastError()));
		goto error;
	}

	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2,2), &WsaData) != 0)
	{
		RETAILMSG(1,(TEXT("WSAStartup failed (error %ld)\n"), GetLastError()));
		CloseHandle(hBTDev);
		goto error;
	}

	RETAILMSG(1, (TEXT("--BT_Init\r\n")));

	return TRUE;

error:
	RETAILMSG(1, (TEXT("--BT_Init\r\n")));
	return FALSE;
}

BOOL BT_SaveStartupState(int save)
{
	HKEY hKey = 0;
	DWORD Size;
	int  en = 0;

	long l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1"),
						  0, 0, &hKey);
	if(ERROR_SUCCESS == l)
	{
        Size = sizeof(DWORD);

		l = RegSetValueEx(hKey, L"EnableOnStartup", 0, REG_DWORD, (UINT8 *)&save, Size);

		en = (ERROR_SUCCESS == l);

		RegFlushKey(hKey);
		RegCloseKey(hKey);
	}

	return en;
}

BOOL BT_GetStatus(void)
{
	RETAILMSG(1, (TEXT("++BT_GetStatus\r\n")));
	BOOL bBthStatus = FALSE;

	int status = 0;
	int ret = BthGetHardwareStatus(&status);
	if (ret == ERROR_SUCCESS)
	{
		bBthStatus = status == HCI_HARDWARE_RUNNING;
		//RETAILMSG(1,(TEXT("BthGetHardwareStatus OK! (status = %ld)\r\n"), status));
	}
	else
	{
		RETAILMSG(1,(TEXT("BthGetHardwareStatus FAIL! (ret = %ld)\r\n"), ret));
	}

	/*
	WSAQUERYSET		wsaq;
	memset (&wsaq, 0, sizeof(wsaq));
	wsaq.dwSize      = sizeof(wsaq);
	wsaq.dwNameSpace = NS_BTH;
	wsaq.lpcsaBuffer = NULL;

	HANDLE hLookup = NULL;

	// Start look up service
	int iErr = BthNsLookupServiceBegin (&wsaq, LUP_CONTAINERS , &hLookup);

	HANDLE hInquiryHandle = hLookup;

	union {
		CHAR buf[5000];
		double __unused;	// ensure proper alignment
	};

	LPWSAQUERYSET pwsaResults = (LPWSAQUERYSET) buf;
	DWORD dwSize  = sizeof(buf);

	memset(pwsaResults,0,sizeof(WSAQUERYSET));
	pwsaResults->dwSize      = sizeof(WSAQUERYSET);
	pwsaResults->dwNameSpace = NS_BTH;
	pwsaResults->lpBlob      = NULL;

	// Try inquiry to check local bluetooth stack state
	iErr = BthNsLookupServiceNext (hLookup, LUP_RETURN_ADDR | LUP_RETURN_NAME, &dwSize, pwsaResults);

	if (iErr == ERROR_SUCCESS)
	{
		// Stack is currently on
		bBthStatus = TRUE;
	}
	else
	{
		// Stack is currently off
		bBthStatus = FALSE;
	}

	if (hInquiryHandle) {
		BthNsLookupServiceEnd(hLookup);
		hInquiryHandle = NULL;
	}
	*/

	RETAILMSG(1, (TEXT("--BT_GetStatus\r\n")));
	return bBthStatus;
}

BOOL BT_TurnOn(void)
{
	RETAILMSG(1, (TEXT("++BT_TurnOn\r\n")));
	BOOL bRet = FALSE;
	WCHAR *argPtr = L"card";
	int i = 20;

	//bRet = DeviceIoControl (hBTDev, IOCTL_SERVICE_START, argPtr, sizeof(WCHAR) * (wcslen (argPtr) + 1), NULL, NULL, NULL, NULL);
	bRet = BthIoControl (IOCTL_SERVICE_START);
	if (bRet)
	{
		do
		{
			Sleep(500);
		}while(!BT_GetStatus() && i--);

		bRet = i > 0;
	}
	//Sleep(10*1000);
	
	RETAILMSG(1, (TEXT("--BT_TurnOn\r\n")));
	return bRet;
}

BOOL BT_TurnOff(void)
{
	RETAILMSG(1, (TEXT("++BT_TurnOff\r\n")));
	
	BOOL bRet = FALSE;
	WCHAR *argPtr = L"card";
	int i = 20;

	//bRet = DeviceIoControl (hBTDev, IOCTL_SERVICE_STOP, argPtr, sizeof(WCHAR) * (wcslen (argPtr) + 1), NULL, NULL, NULL, NULL);
	bRet = BthIoControl (IOCTL_SERVICE_STOP);
	if (bRet)
	{
		do
		{
			Sleep(500);
		}while (BT_GetStatus() && i--);

		bRet = i > 0;
	}
	//Sleep(10*1000);

	RETAILMSG(1, (TEXT("--BT_TurnOff\r\n")));
	return bRet;
}

BOOL BT_DeInit(void)
{
	RETAILMSG(1, (TEXT("++BT_DeInit\r\n")));
	// Stop socket services
	WSACleanup();

	// Close Bluetooth stack
	if (hBTDev != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hBTDev);
		hBTDev = INVALID_HANDLE_VALUE;
	}

	RETAILMSG(1, (TEXT("--BT_DeInit\r\n")));
	return TRUE;
}

BOOL BthIoControl (DWORD dwCtl)
{
	WCHAR *argPtr = L"card";

	return DeviceIoControl(hBTDev, dwCtl, argPtr, sizeof(WCHAR) * (wcslen (argPtr) + 1), NULL, NULL, NULL, NULL);
}