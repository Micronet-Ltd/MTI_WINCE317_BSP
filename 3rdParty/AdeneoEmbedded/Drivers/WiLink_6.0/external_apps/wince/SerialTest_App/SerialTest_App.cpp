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
 
/** \file   SerialTest_App.cpp 
 *  \brief  Application to send/receive data throught a serial port
 *
 *  \see 
 */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <tchar.h>

#include "resource.h"

#include "SerialTest_App.h"
#include "CommHandler.h"

#define MAX_LOADSTRING 100
#define STR_OPEN	L"Open"
#define STR_CLOSE	L"Close"

// Global Variables:
HINSTANCE			v_hInst;					// The current instance
HWND				v_hWnd;
BOOL				v_fPaletteChanged;
CCommHandler	   *v_pserialCommHandler;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
DWORD ShowErr (TCHAR *pszFormat, ...);
BOOL ConnectSerialPort(HWND hWnd, DWORD dwIDConnectButton, DWORD dwIDCommNumber);
BOOL DisconnectSerialPort(HWND hWnd, DWORD dwIDConnectButton);
BOOL SendData(HWND hWnd, DWORD dwIDConnectButton);

#define IDM_CHK_STAT		(WM_USER + 201)


#define DEBUG_FLAG	0

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	TCHAR		szParamInfo[256];
	    
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	SystemParametersInfo (SPI_GETPLATFORMTYPE, sizeof(szParamInfo)/sizeof(TCHAR), szParamInfo, 0);
	SystemParametersInfo (SPI_GETOEMINFO, sizeof(szParamInfo)/sizeof(TCHAR), szParamInfo, 0);
	
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))  {
		if (!IsDialogMessage (v_hWnd, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}


BOOL
InitInstance (HINSTANCE hInstance, int nCmdShow)
{
	v_hInst = hInstance;
	//
	v_hWnd = CreateDialog (hInstance, MAKEINTRESOURCE (IDD_MAINDIALOG), NULL, (DLGPROC)WndProc);

	if (v_hWnd) {
        ShowWindow( v_hWnd, SW_SHOW );
		UpdateWindow(v_hWnd);
	}
	
    return (NULL != v_hWnd);
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	static	DWORD dwInHelp;

	switch (message) {
		case WM_INITDIALOG :
			//
			v_pserialCommHandler = new CCommHandler(hWnd, IDC_RECEIVED_DATA);
			SetWindowText(GetDlgItem(hWnd, IDC_COM_DATA), L"1");
			return FALSE;
			break;

		case WM_PALETTECHANGED :
			v_fPaletteChanged = TRUE;
			break;

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId) {	
				case ID_CONNECT:
					if (v_pserialCommHandler->IsStarted() == FALSE)
					{
						ConnectSerialPort(hWnd, ID_CONNECT, IDC_COM_DATA);
					}
					else
					{
						DisconnectSerialPort(hWnd, ID_CONNECT);
					}
					break;

				case ID_SEND_DATA:
					SendData(hWnd, IDC_SENT_DATA);
					break;

				case IDOK:
				case IDCANCEL:
					if (v_pserialCommHandler->IsStarted() != FALSE)
					{
						DisconnectSerialPort(hWnd, ID_CONNECT);
					}
					DestroyWindow (hWnd);
					break;

				default:
					return FALSE;
					break;

			}
			return TRUE;

			break;

		case WM_DESTROY:
			delete v_pserialCommHandler;
			v_pserialCommHandler = NULL;
			DestroyWindow (hWnd);
			PostQuitMessage(0);
			break;

		default:
			break;
	}
	return FALSE;
}

BOOL ConnectSerialPort(HWND hWnd, DWORD dwIDConnectButton, DWORD dwIDCommNumber)
{
	WCHAR szCommPort[10];
	DWORD dwCommPortLength;
	DWORD dwCommPortNumber;

	dwCommPortLength = GetWindowText(GetDlgItem(hWnd, dwIDCommNumber), szCommPort, sizeof(szCommPort)/sizeof(szCommPort[0]));
	if (dwCommPortLength != 1)
	{
		MessageBox(hWnd, (dwCommPortLength == 0)?L"Please, specify a com Port !":L"Specify a COM port between 0 and 9.", L"Comm Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Convert
	dwCommPortNumber = _wtol(szCommPort);

	if (dwCommPortNumber > 9)
	{
		MessageBox(hWnd, (dwCommPortLength == 0)?L"Please, specify a com Port !":L"Specify a COM port between 0 and 9.", L"Comm Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if (v_pserialCommHandler->Start(dwCommPortNumber))
	{
		SetWindowText(GetDlgItem(hWnd, dwIDConnectButton), STR_CLOSE);
	}
	else
	{
		MessageBox(hWnd, L"Failed to open COM port", L"Comm Error", MB_OK | MB_ICONERROR);
		SetWindowText(GetDlgItem(hWnd, dwIDConnectButton), STR_OPEN);
	}

	return TRUE;
}

BOOL DisconnectSerialPort(HWND hWnd, DWORD dwIDConnectButton)
{
	if (v_pserialCommHandler->Stop())
	{
		SetWindowText(GetDlgItem(hWnd, dwIDConnectButton), STR_OPEN);
	}
	else
	{
		SetWindowText(GetDlgItem(hWnd, dwIDConnectButton), STR_CLOSE);
	}
	return TRUE;
}

BOOL SendData(HWND hWnd, DWORD dwIDConnectButton)
{
	WCHAR szStringToSend[MAX_LOADSTRING];
	DWORD dwStringLength;
	BOOL bRet = FALSE;
	
	if (v_pserialCommHandler->IsStarted())
	{
		dwStringLength = GetWindowText(GetDlgItem(hWnd, dwIDConnectButton), szStringToSend, sizeof(szStringToSend)/sizeof(szStringToSend[0]));
	
		bRet = v_pserialCommHandler->WriteData(szStringToSend);
		if (bRet == FALSE)
		{
			MessageBox(hWnd, L"Failed to send data", L"Comm Error", MB_OK | MB_ICONERROR);
		}
		else
		{
			SetWindowText(GetDlgItem(hWnd, dwIDConnectButton), L"");
		}
	}
	
	return TRUE;
}