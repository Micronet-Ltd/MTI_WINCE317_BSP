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
 
/** \file   Conf_Global.cpp 
 *  \brief  WL1271 Global Scan configuration management
 *
 *  \see    Conf_Global.h
 */
#include <windows.h>

#include "802_11Defs.h"

#include "resource.h"
#include "Conf_Global.h"
#include "Wifi.h"

static Conf_Global_t gGlobalData;

VOID CG_LoadGlobalData(HWND hDlg)
{
	SetDlgItemInt(hDlg, IDC_EDIT_NSINTERVAL,gGlobalData.normalScanInterval,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_DSINTERVAL,gGlobalData.deterioratingScanInterval,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MAXTFAILURES,gGlobalData.maxTrackFailures,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_BSSLISTSIZE,gGlobalData.bssListSize,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_BSSNBSTARTDISC,gGlobalData.bssNumberToStartDisc,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_NBBANDS,gGlobalData.nbOfBands,FALSE);

}

VOID CG_SaveGlobalData(HWND hDlg)
{
	gGlobalData.normalScanInterval = GetDlgItemInt(hDlg, IDC_EDIT_NSINTERVAL,NULL ,FALSE);
	gGlobalData.deterioratingScanInterval = GetDlgItemInt(hDlg, IDC_EDIT_DSINTERVAL,NULL ,FALSE);
	gGlobalData.maxTrackFailures = GetDlgItemInt(hDlg, IDC_EDIT_MAXTFAILURES,NULL ,FALSE);
	gGlobalData.bssListSize = GetDlgItemInt(hDlg, IDC_EDIT_BSSLISTSIZE,NULL ,FALSE);
	gGlobalData.bssNumberToStartDisc = GetDlgItemInt(hDlg, IDC_EDIT_BSSNBSTARTDISC,NULL ,FALSE);
	gGlobalData.nbOfBands = GetDlgItemInt(hDlg, IDC_EDIT_NBBANDS,NULL ,FALSE);
}


// Message handler for Global configuration dialog box.
INT_PTR CALLBACK GLOBALWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
        case WM_INITDIALOG:
			// Recover gGlobalData from driver here
			WL_GetGlobalParameters(&gGlobalData);
			CG_LoadGlobalData(hDlg);
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
				CG_SaveGlobalData(hDlg);
				WL_SetGlobalParameters(&gGlobalData);
	            EndDialog(hDlg, message);
				return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Global Scan Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}


			else if (LOWORD(wParam) == IDC_EDIT_NSINTERVAL)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_NSINTERVAL,NULL,FALSE) > 360000)

					{
						MessageBox(hDlg, L"Normal Scan Interval must be in 0 -> 360000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_NSINTERVAL));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_DSINTERVAL)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_DSINTERVAL,NULL,FALSE) > 360000)

					{
						MessageBox(hDlg, L"Deteriorating Scan interval must be in 0 -> 360000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_DSINTERVAL));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_MAXTFAILURES)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_MAXTFAILURES,NULL,FALSE) > 20)

					{
						MessageBox(hDlg, L"Max Track Failures must be in 0 -> 20", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MAXTFAILURES));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_BSSLISTSIZE)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_BSSLISTSIZE,NULL,FALSE) > 16)

					{
						MessageBox(hDlg, L"BSS List Size must be in 0 -> 16", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_BSSLISTSIZE));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_BSSNBSTARTDISC)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_BSSNBSTARTDISC,NULL,FALSE) > 16)

					{
						MessageBox(hDlg, L"BSS Number to start discovery must be in 0 -> 16", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_BSSNBSTARTDISC));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_NBBANDS)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					if (GetDlgItemInt(hDlg,IDC_EDIT_NBBANDS,NULL,FALSE) > 2)

					{
						MessageBox(hDlg, L"Number of Bands must be in 0 -> 2", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_NBBANDS));
					}
				}
			}

			break;

        case WM_CLOSE:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Global Scan Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
	}
	return (INT_PTR)FALSE;
}
