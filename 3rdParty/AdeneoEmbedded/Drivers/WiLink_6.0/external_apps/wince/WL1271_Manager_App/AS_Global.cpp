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
 
/** \file   AS_Global.cpp 
 *  \brief  WL1271 Application Scan mode management - Global configuration
 *
 *  \see    AS_Global.h
 */
#include <windows.h>
#include <windowsx.h>

#include "802_11Defs.h"

#include "resource.h"
#include "AS_Global.h"
#include "Wifi.h"


TI_UINT8 ASG_GetNumberOfChannels(void)
{
	return gASGlobalData.nbChannels;
}

VOID ASG_SaveASGlobalData(HWND hDlg)
{
	GetDlgItemText(hDlg, IDC_EDIT_ASGSSID,gASGlobalData.SSID , 33);
	gASGlobalData.numOfProbeReqs = GetDlgItemInt(hDlg, IDC_EDIT_ASGPRN,NULL ,FALSE);
	gASGlobalData.nbChannels = GetDlgItemInt(hDlg, IDC_EDIT_ASGNBCHAN,NULL ,FALSE);
	gASGlobalData.triggeringTid = GetDlgItemInt(hDlg, IDC_EDIT_ASGTID,NULL ,FALSE);

	gASGlobalData.scanType = EScanType(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGSCANTYPE)));
	gASGlobalData.bandIndex = ERadioBand(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGBANDNB)));
	gASGlobalData.bitrate = ERateMask(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGPRR)));
}


VOID ASG_LoadASGlobalData(HWND hDlg)
{
	SetDlgItemText(hDlg, IDC_EDIT_ASGSSID,gASGlobalData.SSID);
	SetDlgItemInt(hDlg, IDC_EDIT_ASGPRN,gASGlobalData.numOfProbeReqs,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASGNBCHAN,gASGlobalData.nbChannels,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASGTID,gASGlobalData.triggeringTid,FALSE);

	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGSCANTYPE),(UINT)gASGlobalData.scanType);
	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGBANDNB),(UINT)gASGlobalData.bandIndex);
	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASGPRR),(UINT)gASGlobalData.bitrate);
}

// Message handler for AS Global configuration dialog box.
INT_PTR CALLBACK ASGWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND localHandle;
	int i;

	switch (message)
    {
        case WM_INITDIALOG:

			// Recover gASGlobalData from driver here

			// Init the Combobox content for Prob Request Rate
			localHandle = GetDlgItem(hDlg,ID_CBBOX_ASGPRR);
			if (localHandle == NULL)
			{
				MessageBox(hDlg,L"Failed to get AS Global PRR Combobox Handler",L"ComboBox Handler Error",MB_OK);
				return (INT_PTR)FALSE;
			}
			// Complete List of Probe Requests
			for(i=0;i < NB_PRRATES;i++)
			{
				if (ComboBox_AddString(localHandle,(char*)PRRateDesc[i].PRRateName) != i)
				{
					MessageBox(hDlg, L"Failed to set AS Global PRR List", L"PR List Error", MB_OK);
					return (INT_PTR)FALSE;
				}
			}

			// Init the Combobox content for Scan Types
			localHandle = GetDlgItem(hDlg,ID_CBBOX_ASGSCANTYPE);
			if (localHandle == NULL)
			{
				MessageBox(hDlg,L"Failed to get AS Global ScanType ComboBox Handler",L"ComboBox Handler Error",MB_OK);
				return (INT_PTR)FALSE;
			}
			// Complete list of possible scan types
			for(i=0;i < NB_SCANTYPES;i++)
			{
				if (ComboBox_AddString(localHandle,(char*)ScanTypeDesc[i]) != i)
				{
					MessageBox(hDlg,L"Failed to set AS Global ScanType List", L"ScanType List Error", MB_OK);
					return (INT_PTR)FALSE;
				}
			}


			// Init the Combobox content for Scan Types
			localHandle = GetDlgItem(hDlg,ID_CBBOX_ASGBANDNB);

			if (localHandle == NULL)
			{
				MessageBox(hDlg,L"Failed to get AS Global Band Combobox Handler",L"ComboBox Handler Error",MB_OK);
				return (INT_PTR)FALSE;
			}
			// Complete List of Bands
			for(i=0;i < NB_BANDINDEX;i++)
			{
				if (ComboBox_AddString(localHandle,(char*)BandIndexList[i]) != i)
				{
					MessageBox(hDlg, L"Failed to set AS Global Band Rate List", L"Band Rate List Error", MB_OK);
					return (INT_PTR)FALSE;
				}
			}
			WL_GetASGlobalParameters(&gASGlobalData);
			ASG_LoadASGlobalData(hDlg);
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
	            ASG_SaveASGlobalData(hDlg);
				WL_SetASGlobalParameters(&gASGlobalData);
				EndDialog(hDlg, message);
				return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Application Scan - Global Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}


			else if (LOWORD(wParam) == IDC_EDIT_ASGPRN)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASGPRN,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Probe Request Number must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASGPRN));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASGNBCHAN)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASGNBCHAN,NULL,FALSE) > 16)
					{
						MessageBox(hDlg, L"Number of Channels must be in 0 -> 16", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASGNBCHAN));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASGTID)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASGTID,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Triggering ID must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASGTID));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASGSSID)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					WCHAR tempSsid[33];
					if (GetDlgItemText(hDlg,IDC_EDIT_ASGSSID,tempSsid,34) > 33)
					{
						MessageBox(hDlg, L"SSID Size must be between 0 and 33 characters", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASGSSID));
					}
				}
			}

			break;

        case WM_CLOSE:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Application Scan - Global Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
	}
	return (INT_PTR)FALSE;
}
