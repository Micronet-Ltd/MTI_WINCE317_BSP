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
 
/** \file   DTI_Modes.cpp 
 *  \brief  WL1271 Discovery, Tracking and Immediate modes management
 *
 *  \see    DTI_Modes.h
 */
#include <windows.h>
#include <windowsx.h>

#include "802_11Defs.h"

#include "resource.h"
#include "Wifi.h"
#include "DTI_Modes.h"

VOID DTIM_SaveDTIModeData(HWND hDlg, TScanAppData* pScanAppData)
{
	//if(IsDlgButtonChecked(hDlg,ID_WL_BANDINDEX0)== BST_CHECKED)
	//	pScanAppData->bandIndex = ERadioBand(0);
	//else
	//	pScanAppData->bandIndex = ERadioBand(1);

	pScanAppData->bandIndex = gDTIBandIndex;
	pScanAppData->maxChannelDwellTime = GetDlgItemInt(hDlg, IDC_EDIT_MAXDWT,NULL ,FALSE);
	pScanAppData->minChannelDwellTime = GetDlgItemInt(hDlg, IDC_EDIT_MINDWT,NULL ,FALSE);
	pScanAppData->ETMaxNumberOfApFrames = GetDlgItemInt(hDlg, IDC_EDIT_NBFRAMES,NULL ,FALSE);
	pScanAppData->numOfProbeReqs = GetDlgItemInt(hDlg, IDC_EDIT_PRN,NULL ,FALSE);
	pScanAppData->scanDuration = GetDlgItemInt(hDlg, IDC_EDIT_SCANDUR,NULL ,FALSE);
	pScanAppData->triggeringTid = GetDlgItemInt(hDlg, IDC_EDIT_TRIGAC,NULL ,FALSE);
	pScanAppData->txPowerDbm = GetDlgItemInt(hDlg, IDC_EDIT_TXPWLVL,NULL ,FALSE);
	// As scanType, earlyTerminationEvent and bitrate are Enumeration types,
	// and as the combobox is filled by increasing index, getting current index
	// will directly return the good enum index.
	pScanAppData->scanType = EScanType(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_SCANT)));
	pScanAppData->earlyTerminationEvent = EScanEtCondition(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_ETEVT)));
	pScanAppData->bitrate = ERateMask(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_PRR)));
}

VOID DTIM_LoadDTIModeData(HWND hDlg, TScanAppData *pScanAppData, int iID_WL_CONF)
{
	CheckDlgButton(hDlg,ID_WL_CONF_DISC, BST_UNCHECKED);
	CheckDlgButton(hDlg,ID_WL_CONF_IMMSCAN, BST_UNCHECKED);
	CheckDlgButton(hDlg,ID_WL_CONF_TRACK, BST_UNCHECKED);
	CheckDlgButton(hDlg,iID_WL_CONF, BST_CHECKED);

	CheckDlgButton(hDlg,ID_WL_BANDINDEX0, BST_UNCHECKED);
	CheckDlgButton(hDlg,ID_WL_BANDINDEX1, BST_UNCHECKED);

	if(gDTIBandIndex == ERadioBand(1))
	{
		CheckDlgButton(hDlg,ID_WL_BANDINDEX1, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hDlg,ID_WL_BANDINDEX0, BST_CHECKED);
	}

	SetDlgItemInt(hDlg, IDC_EDIT_TRIGAC,pScanAppData->triggeringTid,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MAXDWT,pScanAppData->maxChannelDwellTime,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MINDWT,pScanAppData->minChannelDwellTime,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_NBFRAMES,pScanAppData->ETMaxNumberOfApFrames,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_PRN,pScanAppData->numOfProbeReqs,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_TXPWLVL,pScanAppData->txPowerDbm,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_SCANDUR,pScanAppData->scanDuration,FALSE);

	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_SCANT),(UINT)pScanAppData->scanType);
	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ETEVT),(UINT)pScanAppData->earlyTerminationEvent);
	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_PRR),(UINT)pScanAppData->bitrate);
}


// Message handler for Scan Settings dialog box.
INT_PTR CALLBACK ScanWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HWND localHandle = NULL;

	switch (message)
    {
        case WM_INITDIALOG:
            {
				// Init the Combobox content for Prob Request Rate
				localHandle = GetDlgItem(hDlg,ID_CBBOX_PRR);
				if (localHandle == NULL)
				{
					MessageBox(hDlg,L"Failed to get PRR Combobox Handler",L"ComboBox Handler Error",MB_OK);
					return (INT_PTR)FALSE;
				}
				// Complete List of Probe Requests
				for(i=0;i < NB_PRRATES;i++)
				{
					if (ComboBox_AddString(localHandle,(char*)PRRateDesc[i].PRRateName) != i)
					{
						MessageBox(hDlg, L"Failed to set PR List", L"PR List Error", MB_OK);
						return (INT_PTR)FALSE;
					}
				}

				// Init the Combobox content for Early Termination Events
				localHandle = GetDlgItem(hDlg,ID_CBBOX_ETEVT);
				if (localHandle == NULL)
				{
					MessageBox(hDlg,L"Failed to get Event ComboBox Handler",L"ComboBox Handler Error",MB_OK);
					return (INT_PTR)FALSE;
				}
				// Complete list of Early Termination Events
				for(i=0;i < NB_ETEVENTS;i++)
				{
					if (ComboBox_AddString(localHandle,(char*)ETEventNames[i].EvtName) != i)
					{
						MessageBox(hDlg, L"Failed to set Event List", L"Event List Error", MB_OK);
						return (INT_PTR)FALSE;
					}
				}

				// Init the Combobox content for Scan Types
				localHandle = GetDlgItem(hDlg,ID_CBBOX_SCANT);
				if (localHandle == NULL)
				{
					MessageBox(hDlg,L"Failed to get ScanType ComboBox Handler",L"ComboBox Handler Error",MB_OK);
					return (INT_PTR)FALSE;
				}
				// Complete list of possible scan types
				for(i=0;i < NB_SCANTYPES;i++)
				{
					if (ComboBox_AddString(localHandle,(char*)ScanTypeDesc[i]) != i)
					{
						MessageBox(hDlg,L"Failed to set ScanType List", L"ScanType List Error", MB_OK);
						return (INT_PTR)FALSE;
					}
				}
				// Update information from driver            
				WL_GetScanAppDiscParameters(0, &gDTIScanAppData[0].discData);
				WL_GetScanAppTrackParameters(0, &gDTIScanAppData[0].trackData);
				WL_GetScanAppImmScanParameters(0, &gDTIScanAppData[0].immScanData);

				WL_GetScanAppDiscParameters(1, &gDTIScanAppData[1].discData);
				WL_GetScanAppTrackParameters(1, &gDTIScanAppData[1].trackData);
				WL_GetScanAppImmScanParameters(1, &gDTIScanAppData[1].immScanData);

				gDTIScanAppData[0].discData.bandIndex = ERadioBand(0);
				gDTIScanAppData[0].immScanData.bandIndex = ERadioBand(0);
				gDTIScanAppData[0].trackData.bandIndex = ERadioBand(0);

				gDTIScanAppData[1].discData.bandIndex = ERadioBand(1);
				gDTIScanAppData[1].immScanData.bandIndex = ERadioBand(1);
				gDTIScanAppData[1].trackData.bandIndex = ERadioBand(1);

				// Load parameters in Window
				switch (gDTIScanMode)
				{
				case ID_WL_CONF_IMMSCAN:
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData, ID_WL_CONF_IMMSCAN);
					break;
				case ID_WL_CONF_DISC:
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData, ID_WL_CONF_DISC);
					break;
				case ID_WL_CONF_TRACK:
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData, ID_WL_CONF_TRACK);
					break;
				}

			}
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
			case IDOK:
				{
				// Recover data from current Window
					switch (gDTIScanMode)
					{
					case ID_WL_CONF_IMMSCAN:
						DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData);
					break;
					case ID_WL_CONF_DISC:
						DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData);
					break;
					case  ID_WL_CONF_TRACK:
						DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData);
					break;
					}
				// Send data to driver
				WL_SetScanAppImmScanParameters(&gDTIScanAppData[0].immScanData);
				WL_SetScanAppTrackParameters(&gDTIScanAppData[0].trackData);
				WL_SetScanAppDiscParameters(&gDTIScanAppData[0].discData);
				
				WL_SetScanAppImmScanParameters(&gDTIScanAppData[1].immScanData);
				WL_SetScanAppTrackParameters(&gDTIScanAppData[1].trackData);
				WL_SetScanAppDiscParameters(&gDTIScanAppData[1].discData);

				EndDialog(hDlg,LOWORD(wParam));
				return TRUE;
				}

			case IDC_EDIT_TRIGAC:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_TRIGAC,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Triggering AC value must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_TRIGAC));
					}
				}
			break;
			}
			case IDC_EDIT_SCANDUR:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_SCANDUR,NULL,FALSE) > 100000000)
					{
						MessageBox(hDlg, L"Scan duration must be in 0 -> 100000000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_SCANDUR));
					}
				}
			break;
			}
			case IDC_EDIT_TXPWLVL:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_TXPWLVL,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Tx Power Level must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_TXPWLVL));
					}
				}
			break;
			}
			case IDC_EDIT_MAXDWT:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MAXDWT,NULL,FALSE) > 100000000)
					{
						MessageBox(hDlg, L"Max Dwell Time must be in 0 -> 100000000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MAXDWT));
					}
				}
			break;
			}
			case IDC_EDIT_MINDWT:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MINDWT,NULL,FALSE) > 100000000)
					{
						MessageBox(hDlg, L"Min Dwell Time must be in 0 -> 100000000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MINDWT));
					}
				}
			break;
			}
			case IDC_EDIT_PRN:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_PRN,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Probe Request Number must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_PRN));
					}
				}
			break;
			}
			case IDC_EDIT_NBFRAMES:
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_NBFRAMES,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"ET - Number of frames must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_NBFRAMES));
					}
				}
			break;
			}

			case IDCANCEL:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Scan Mode Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return (INT_PTR)TRUE;
			}

			// A new Scan Mode was selected :
			// Set new Scan Mode in global vars			
			case ID_WL_CONF_IMMSCAN:
				// Save last modified config
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData);
				else
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData);

				// Load Immediate scan mode data
				gDTIBandIndex = ERadioBand(0);
				gDTIScanMode = ID_WL_CONF_IMMSCAN;
				DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData, gDTIScanMode);
				break;

			case ID_WL_CONF_DISC:
				// Save last modified config
				if(gDTIScanMode == ID_WL_CONF_IMMSCAN)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData);
				else
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData);

				// Load Discovery mode data
				gDTIBandIndex = ERadioBand(0);
				gDTIScanMode = ID_WL_CONF_DISC;
				DTIM_LoadDTIModeData(hDlg,&gDTIScanAppData[gDTIBandIndex].discData,gDTIScanMode);
				break;
			case ID_WL_CONF_TRACK:
				// Save last modified config
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData);
				else
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData);

				// Load Tracking mode data
				gDTIBandIndex = ERadioBand(0);
				gDTIScanMode = ID_WL_CONF_TRACK;
				DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData, gDTIScanMode);
				break;

			// A new Band Index was selected :
			case ID_WL_BANDINDEX0:
				{
				// Modify buttons state
				CheckDlgButton(hDlg,ID_WL_BANDINDEX0, BST_CHECKED);
				CheckDlgButton(hDlg,ID_WL_BANDINDEX1, BST_UNCHECKED);
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData);
				else if (gDTIScanMode == ID_WL_CONF_IMMSCAN)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData);
				else
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData);

				// Save band index in data (depending of current scan mode)
				gDTIBandIndex = ERadioBand(0);

				// Load data
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData, gDTIScanMode);

				else if (gDTIScanMode == ID_WL_CONF_IMMSCAN)
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData, gDTIScanMode);

				else
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData, gDTIScanMode);


				//switch (gDTIScanMode)
				//{
				//case ID_WL_CONF_DISC:
				//	gDTIScanAppData.discData.bandIndex = gDTIBandIndex;
				//	break;
				//case ID_WL_CONF_TRACK:
				//	gDTIScanAppData.trackData.bandIndex = gDTIBandIndex;
				//	break;
				//case ID_WL_CONF_IMMSCAN:
				//	gDTIScanAppData.immScanData.bandIndex = gDTIBandIndex;
				//	break;
				//default :
				//	break;
				//}
				}
				break;
			case ID_WL_BANDINDEX1:
				{
				// Modify buttons state
				CheckDlgButton(hDlg,ID_WL_BANDINDEX1, BST_CHECKED);
				CheckDlgButton(hDlg,ID_WL_BANDINDEX0, BST_UNCHECKED);
				// Save band index in data (depending of current scan mode)
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData);
				else if (gDTIScanMode == ID_WL_CONF_IMMSCAN)
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData);
				else
					DTIM_SaveDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData);

				gDTIBandIndex = ERadioBand(1);

				// Load data
				if(gDTIScanMode == ID_WL_CONF_DISC)
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].discData, gDTIScanMode);

				else if (gDTIScanMode == ID_WL_CONF_IMMSCAN)
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].immScanData, gDTIScanMode);

				else
					DTIM_LoadDTIModeData(hDlg, &gDTIScanAppData[gDTIBandIndex].trackData, gDTIScanMode);				//switch (gDTIScanMode)
				//{
				//case ID_WL_CONF_DISC:
				//	gDTIScanAppData.discData.bandIndex = gDTIBandIndex;
				//	break;
				//case ID_WL_CONF_TRACK:
				//	gDTIScanAppData.trackData.bandIndex = gDTIBandIndex;
				//	break;
				//case ID_WL_CONF_IMMSCAN:
				//	gDTIScanAppData.immScanData.bandIndex = gDTIBandIndex;
				//	break;
				//default :
				//	break;
				//}
				}
				break;

			default :
				break;
			}
			return (INT_PTR)TRUE;

        case WM_CLOSE:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Scan Mode Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
				return (INT_PTR)TRUE;
			}


	}
	return (INT_PTR)FALSE;
}