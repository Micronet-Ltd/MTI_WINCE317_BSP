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
 
/** \file   WL1271_Manager_App.cpp 
 *  \brief  Application to manage WL1271 board
 *
 *  \see   Bluetooth.cpp Wifi.cpp FM.cpp
 */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <tchar.h>

#include "802_11Defs.h"
#include "resource.h"

#include "WL1271_Manager_App.h"

#include "Bluetooth.h"
#include "Wifi.h"
#include "FM.h"

extern "C" {
#include <cu_osapi.h>
#include <console.h>
#include <cu_cmd.h>


#include <stdio.h>

#include "cu_osapi.h"
#include "TWDriver.h"
#include "convert.h"
#include "console.h"
#include "cu_common.h"
#include "cu_os.h"
#include "ipc_event.h"
#include "wpa_core.h"
#include "cu_cmd.h"
#include "oserr.h"
}


#include "AS_Channel.h"
#include "AS_Global.h"
#include "Conf_Global.h"
#include "DTI_Channel.h"
#include "Roaming.h"
#include "DTI_Modes.h"


#define MAX_LOADSTRING 100

void
UpdateDeviceImage (HWND hWnd, INT iDeviceID);
void
SwitchDevicePower(HWND hWnd, INT iDeviceID);

// Global Variables:
HINSTANCE			v_hInst;					// The current instance
HWND				v_hWnd;
HWND				v_hHelpWnd;
HICON				v_hNotifyIcon;
//DWORD				v_dwBluetootActivated;
//DWORD				v_dwWifiActivated;
//DWORD				v_dwFMActivated;
DWORD				dwSkip, dwTotal;
BOOL				v_fPaletteChanged;


typedef struct Device{
	BOOL (*Init)(void);
	BOOL (*GetStatus)(void);
	BOOL (*SaveStartupState)(int);
	BOOL (*TurnOn)(void);
	BOOL (*TurnOff)(void);
	BOOL (*DeInit)(void);
	BOOL bCurrentStatus;
	BOOL bPrevStatus;
	DWORD iBitmapOn;
	DWORD iBitmapOff;
	INT  iDeviceButton;

} DeviceFunctions;

#define BTH_DEVICE	0
#define WLAN_DEVICE	1
#define FM_DEVICE	2
#define NB_DEVICES	3


DeviceFunctions DevTab[NB_DEVICES] = 
{
	// Bluetooth Device
	{BT_Init, BT_GetStatus, BT_SaveStartupState, BT_TurnOn, BT_TurnOff, BT_DeInit,
	FALSE, 0, IDC_BT_ON, IDC_BT_OFF, ID_ACTIVATE_BT},
	// WLAN Device
	{WL_Init, WL_GetStatus, WL_SaveStartupState, WL_TurnOn, WL_TurnOff, WL_DeInit,
	FALSE, 0, IDC_WL_ON, IDC_WL_OFF, ID_ACTIVATE_WL},
	// FM Device
	{FM_Init, FM_GetStatus, FM_SaveStartupState, FM_TurnOn, FM_TurnOff, FM_DeInit,
	FALSE, 0, IDC_FM_ON, IDC_FM_OFF, ID_ACTIVATE_FM}
};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
DWORD ShowErr (TCHAR *pszFormat, ...);

#define IDM_CHK_STAT		(WM_USER + 201)


#define DEBUG_FLAG	0

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	void  *pRan;
	MSG	  msg;
	TCHAR szParamInfo[256];

	HWND hWnd = FindWindow(L"Dialog", L"WL1271 Manager");

	if(IsWindow(hWnd))
	{
		SetForegroundWindow(hWnd);

		return -1;
	}

	void *vp = CreateEvent(0, 1, 1, L"___TIWLNAPI1_ran");

	if(WAIT_OBJECT_0 != WaitForSingleObject(vp, 5000))
		return -1;

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

	// Init All devices access
	for (int i = 0 ; i < NB_DEVICES ; i++)
	{
		if (!DevTab[i].Init())
		{
			RETAILMSG(TRUE, (TEXT("Failed to init device %d"), i));
		}
	}

	v_hWnd = CreateDialog (hInstance, MAKEINTRESOURCE (IDD_MAINDIALOG), NULL, (DLGPROC)WndProc);


	if (v_hWnd) {
		RECT r, r2;
		if (GetWindowRect(v_hWnd, &r))
		{
			if (GetWindowRect(GetDlgItem(v_hWnd, IDC_FM_ON), &r2))
			{
				SetWindowPos(v_hWnd, NULL, 0, 0, r.right - r.left, r2.top - r.top, SWP_NOMOVE | SWP_NOACTIVATE);
			}
		}

        ShowWindow( v_hWnd, SW_SHOW );
		UpdateWindow(v_hWnd);
	}
	
    return (NULL != v_hWnd);
}


// Message handler for Mac Address dialog box.
INT_PTR CALLBACK MACWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
        case WM_INITDIALOG:
            {
				WCHAR szMACAddress[20];
				DWORD dwSize = 19;
				WL_GetMACAddress (szMACAddress, dwSize);

				SetDlgItemText(hDlg, IDC_EDIT_MAC, szMACAddress);
			}
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
				WCHAR szMACAddress[20];
				DWORD dwSize = 19;
				GetDlgItemText(hDlg, IDC_EDIT_MAC, szMACAddress, dwSize);
				if (WL_SetMACAddress (szMACAddress) == FALSE)
				{
					MessageBox(hDlg, L"Failed to change MAC Address", L"MAC Address Error", MB_OK|MB_ICONEXCLAMATION);
				}
				else
				{
					MessageBox(hDlg, L"MAC Address changed, system reset is required.", L"MAC Address", MB_OK|MB_ICONINFORMATION);
	                EndDialog(hDlg, LOWORD(wParam));
				}
				return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
			{
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
			}
			break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;
	}
	return (INT_PTR)FALSE;
}

// Message handler for WLAN configuration dialog box.
INT_PTR CALLBACK WLANWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
				// Update WLAN Roaming info
				BOOL bRoamingEnable = FALSE;
				if (WL_GetRoamingStatus(bRoamingEnable))
				{
					CheckDlgButton(hDlg, ID_WL_CONF_ROAMING, (bRoamingEnable)?BST_CHECKED:BST_UNCHECKED);
				}

				// Recover Scan modes data from driver
				gDTIScanMode = ID_WL_CONF_DISC;

				gDTIBandIndex = ERadioBand(0);
				WL_GetScanAppDiscParameters(0, &gDTIScanAppData[0].discData);
				WL_GetScanAppTrackParameters(0, &gDTIScanAppData[0].trackData);
				WL_GetScanAppImmScanParameters(0, &gDTIScanAppData[0].immScanData);

				WL_GetScanAppDiscParameters(1, &gDTIScanAppData[1].discData);
				WL_GetScanAppTrackParameters(1, &gDTIScanAppData[1].trackData);
				WL_GetScanAppImmScanParameters(1, &gDTIScanAppData[1].immScanData);
			#if !defined(BSP_EVM_DM_AM_37X)
				EnableWindow(GetDlgItem(hDlg, ID_WL_CONF_MAC), 0);
				EnableWindow(GetDlgItem(hDlg, ID_WL_CONF_BIP), 0);
			#endif

			}
			return (INT_PTR)TRUE;

        case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
				break;

			case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
				break;

			case ID_WL_CONF_SCAN:
				{
					// Allow user to change the Scan Mode & Scan Parameters
					DialogBox(v_hInst, (LPCTSTR)IDD_SCANDIALOG, hDlg, ScanWndProc);
					return TRUE;
				}
			case ID_WL_ROAMING:
				{
					// Allow user to change the Roaming Parameters
					DialogBox(v_hInst, (LPCTSTR)IDD_ROAMINGDIALOG, hDlg, RoamingWndProc);
					return TRUE;
				}

			case ID_WL_START_SCAN:
				{
					// Send START Command
					WL_StartScan();
					return TRUE;
				}

			case ID_WL_STOP_SCAN:
				{
					// Send STOP Command
					WL_StopScan();
					return TRUE;
				}
			case ID_WL_GLOBAL:
				{
					DialogBox(v_hInst, (LPCTSTR)IDD_GLOBALCONFDIALOG, hDlg, GLOBALWndProc);
					return TRUE;
				}
			case ID_WL_MISC:
				{
					DialogBox(v_hInst, (LPCTSTR)IDD_MISCANDCHANDIALOG, hDlg, MISCWndProc);
					return TRUE;
				}
			case ID_WL_ASG:
				{
					DialogBox(v_hInst, (LPCTSTR)IDD_ASGLOBALDIALOG, hDlg, ASGWndProc);
					DialogBox(v_hInst, (LPCTSTR)IDD_ASCHANDIALOG, hDlg, ASCWndProc);
					return TRUE;
				}

			case ID_WL_CONF_MAC:
				{
					// Allow user to change the MAC address;
					DialogBox(v_hInst, (LPCTSTR)IDD_MACDIALOG, hDlg, MACWndProc);
					return TRUE;
				}

			case ID_WL_CONF_BIP:
				{
					// Launch BIP calibration process for all bands
					if (WL_BIPCalibration (TRUE, 
						TRUE,
						TRUE,
						TRUE,
						TRUE,
						TRUE,
						TRUE,
						TRUE) == TRUE)
					{
						MessageBox(hDlg, L"BIP Calibration performed, new configuration is saved in registry.", L"BIP Calibration", MB_OK | MB_ICONINFORMATION);
					}
					else
					{
						MessageBox(hDlg, L"BIP Calibration failed", L"BIP Calibration", MB_OK | MB_ICONEXCLAMATION);
					}
					return TRUE;
				}
				break;

			case ID_WL_CONF_ROAMING:
				{	
					// Enable/disable roaming option
					if (BST_CHECKED == IsDlgButtonChecked(hDlg, ID_WL_CONF_ROAMING))
					{
						WL_RoamingEnable();
					}
					else
					{
						WL_RoamingDisable();
					}
				}
				break;

			default:
				//
			break;

			}
			break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return TRUE;
	}
	return (INT_PTR)FALSE;
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
BOOL WL_RadioTest(HWND);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	static	DWORD dwInHelp;

	switch (message) {
		case WM_INITDIALOG :
			// 
			for (int i = 0 ; i < NB_DEVICES ; i++)
			{
				DevTab[i].bPrevStatus = DevTab[i].bCurrentStatus = DevTab[i].GetStatus();

				//Update images
				UpdateDeviceImage (hWnd, i);

				//Update Text in buttons
				SetWindowText (GetDlgItem(hWnd, DevTab[i].iDeviceButton), 
					DevTab[i].bCurrentStatus ? TEXT("Deactivate") : TEXT("Activate"));


			}
			return FALSE;
			break;

		case WM_PALETTECHANGED :
			v_fPaletteChanged = TRUE;
			break;

		case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId) {	
				case ID_ACTIVATE_BT:
					SwitchDevicePower(hWnd, BTH_DEVICE);

					DevTab[WLAN_DEVICE].bCurrentStatus = DevTab[WLAN_DEVICE].GetStatus();

					// Update images
					UpdateDeviceImage (hWnd, WLAN_DEVICE);

					// Update Text in buttons
					SetWindowText(GetDlgItem(hWnd, DevTab[WLAN_DEVICE].iDeviceButton), 
								DevTab[WLAN_DEVICE].bCurrentStatus ? TEXT("Deactivate") : TEXT("Activate"));
					break;

				case ID_ACTIVATE_WL:
				{
					SwitchDevicePower(hWnd, WLAN_DEVICE);
					DevTab[BTH_DEVICE].bCurrentStatus = DevTab[BTH_DEVICE].GetStatus();

					// Update images
					UpdateDeviceImage (hWnd, BTH_DEVICE);

					// Update Text in buttons
					SetWindowText(GetDlgItem(hWnd, DevTab[BTH_DEVICE].iDeviceButton), 
								DevTab[BTH_DEVICE].bCurrentStatus ? TEXT("Deactivate") : TEXT("Activate"));

/*
					if(DevTab[WLAN_DEVICE].bCurrentStatus)
					{
						Sleep(100);
						WL_RadioTest(hWnd);
					}
*/
					break;
				}
				case ID_CONF_WL:
					{
						// Allow user to change the MAC address;
						DialogBox(v_hInst, (LPCTSTR)IDD_WIFIDIALOG, hWnd, WLANWndProc);
					}
					break;

				case ID_ACTIVATE_FM:
					SwitchDevicePower(hWnd, FM_DEVICE);
					break;

				case IDOK:
				{
					for(int i = 0 ; i < NB_DEVICES ; i++)
					{
						if(DevTab[i].bCurrentStatus != DevTab[i].bPrevStatus)
						{
							DevTab[i].bPrevStatus = DevTab[i].bCurrentStatus;
							DevTab[i].SaveStartupState(DevTab[i].bCurrentStatus);
						}
					}
					DestroyWindow (hWnd);
					break;
				}
				case IDCANCEL:
				{
					for(int i = 0 ; i < NB_DEVICES ; i++)
					{
						if(DevTab[i].bCurrentStatus != DevTab[i].bPrevStatus)
						{
							if(DevTab[i].bPrevStatus)
								DevTab[i].TurnOn();
							else
								DevTab[i].TurnOff();
						}
					}
					DestroyWindow (hWnd);
					break;
				}
				default:
					return FALSE;
			}
			return TRUE;
		}
		case WM_DESTROY:
			// Deinit All devices access
			for (int i = 0 ; i < NB_DEVICES ; i++)
			{
				if (!DevTab[i].DeInit())
				{
					RETAILMSG(TRUE, (TEXT("Failed to deinit device %d"), i));
				}
			}
			DestroyWindow (hWnd);
			PostQuitMessage(0);
			break;

		default:
			break;
	}
	return FALSE;
}

void
UpdateDeviceImage (HWND hWnd, INT iDeviceID)
{
	//Update images
	if(DevTab[iDeviceID].bCurrentStatus)
	{
		ShowWindow(GetDlgItem(hWnd, DevTab[iDeviceID].iBitmapOff),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, DevTab[iDeviceID].iBitmapOn),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hWnd, DevTab[iDeviceID].iBitmapOn),SW_HIDE);
		ShowWindow(GetDlgItem(hWnd, DevTab[iDeviceID].iBitmapOff),SW_SHOW);
	}
}

void
SwitchDevicePower(HWND hWnd, INT iDeviceID)
{
	//EnableWindow(hWnd, FALSE);
	EnableWindow(GetDlgItem(hWnd, ID_ACTIVATE_WL), 0);
	EnableWindow(GetDlgItem(hWnd, ID_ACTIVATE_BT), 0);

	HCURSOR cur = SetCursor(LoadCursor(NULL, IDC_WAIT));

	if (DevTab[iDeviceID].bCurrentStatus)
	{
		DevTab[iDeviceID].TurnOff();
	}
	else
	{
		DevTab[iDeviceID].TurnOn();
	}
	Sleep(200);
	DevTab[iDeviceID].bCurrentStatus = DevTab[iDeviceID].GetStatus();

	SetCursor(cur);
	//EnableWindow(hWnd, TRUE);
	//
	UpdateDeviceImage (hWnd, iDeviceID);

	//Update Text in buttons
	SetWindowText (GetDlgItem(hWnd, DevTab[iDeviceID].iDeviceButton), 
		DevTab[iDeviceID].bCurrentStatus ? TEXT("Deactivate") : TEXT("Activate"));

	EnableWindow(GetDlgItem(hWnd, ID_ACTIVATE_WL), 1);
	EnableWindow(GetDlgItem(hWnd, ID_ACTIVATE_BT), 1);
}

