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
 
/** \file   Roaming.cpp 
 *  \brief  WL1271 Roaming configuration
 *
 *  \see    Roaming.h
 */
#include <windows.h>
#include <windowsx.h>

#include "802_11Defs.h"

#include "resource.h"
#include "Wifi.h"

#include "Roaming.h"
#include "roamingMngrTypes.h"


VOID SaveRoamingData(HWND hDlg)
{
	gRoamingConfig.roamingMngrConfig.lowPassFilterRoamingAttempt = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLPF ,NULL ,FALSE);
	gRoamingConfig.roamingMngrConfig.apQualityThreshold = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGQT ,NULL ,TRUE);

	gRoamingConfig.roamingMngrThresholdsConfig.dataRetryThreshold = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGDRT ,NULL ,FALSE);
	gRoamingConfig.roamingMngrThresholdsConfig.numExpectedTbttForBSSLoss = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGBSSLOSS ,NULL ,FALSE);
	gRoamingConfig.roamingMngrThresholdsConfig.txRateThreshold = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGTXRATE ,NULL ,FALSE);
	gRoamingConfig.roamingMngrThresholdsConfig.lowRssiThreshold = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLOWRSSI ,NULL ,TRUE);
	gRoamingConfig.roamingMngrThresholdsConfig.lowSnrThreshold = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLOWSNR ,NULL ,FALSE);
	gRoamingConfig.roamingMngrThresholdsConfig.lowQualityForBackgroungScanCondition = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLQSCAN ,NULL ,TRUE);
	gRoamingConfig.roamingMngrThresholdsConfig.normalQualityForBackgroungScanCondition = GetDlgItemInt(hDlg,IDC_EDIT_ROAMINGNQSCAN ,NULL ,TRUE);

}

VOID LoadRoamingData(HWND hDlg)
{
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLPF ,gRoamingConfig.roamingMngrConfig.lowPassFilterRoamingAttempt,FALSE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGQT ,gRoamingConfig.roamingMngrConfig.apQualityThreshold,TRUE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGDRT ,gRoamingConfig.roamingMngrThresholdsConfig.dataRetryThreshold ,FALSE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGBSSLOSS ,gRoamingConfig.roamingMngrThresholdsConfig.numExpectedTbttForBSSLoss ,FALSE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGTXRATE ,gRoamingConfig.roamingMngrThresholdsConfig.txRateThreshold ,FALSE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLOWRSSI ,gRoamingConfig.roamingMngrThresholdsConfig.lowRssiThreshold ,TRUE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLOWSNR ,gRoamingConfig.roamingMngrThresholdsConfig.lowSnrThreshold ,FALSE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGLQSCAN ,gRoamingConfig.roamingMngrThresholdsConfig.lowQualityForBackgroungScanCondition ,TRUE);
	SetDlgItemInt(hDlg,IDC_EDIT_ROAMINGNQSCAN ,gRoamingConfig.roamingMngrThresholdsConfig.normalQualityForBackgroungScanCondition ,TRUE);

}

INT_PTR CALLBACK RoamingWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
    {
        case WM_INITDIALOG:

			// Recover gASGlobalData from driver here
			WL_GetRoamingParameters(gRoamingConfig);
			LoadRoamingData(hDlg);
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
	            SaveRoamingData(hDlg);
				WL_SetRoamingParameters(gRoamingConfig);
				EndDialog(hDlg, message);
				return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Roaming Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
			break;

        case WM_CLOSE:
			if (MessageBox(hDlg, L"Cancel Modifications?", L"Roaming Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
			{
				// User confirmed the cancel operation
				EndDialog(hDlg, LOWORD(wParam));
			}
			return TRUE;
	}
	return (INT_PTR)FALSE;
}

