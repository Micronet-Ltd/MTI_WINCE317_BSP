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
 
/** \file   DTI_Channel.cpp 
 *  \brief  WL1271 Discovery, Tracking and Immediate misc & channel management
 *
 *  \see    DTI_Channel.h
 */
#include <windows.h>
#include <windowsx.h>
#include <Winuser.h>

#include "802_11Defs.h"

#include "resource.h"
#include "DTI_Channel.h"
#include "Wifi.h"

static DTI_Channel_t gChannelData[RADIO_BAND_NUM_OF_BANDS]; // Dual Selection Not Allowed
static TI_UINT8 gChannelIndex;
static TI_UINT8 gBandIndex;

VOID DTIC_SaveDTIChannelData(HWND hDlg)
{
	gChannelData[gBandIndex].bandIndex = ERadioBand(gBandIndex);
	gChannelData[gBandIndex].bandNumber = GetDlgItemInt(hDlg, IDC_EDIT_MCBAND,NULL ,FALSE);
	gChannelData[gBandIndex].bssThreshold = GetDlgItemInt(hDlg, IDC_EDIT_MCBSSTHRESHOLD,NULL ,TRUE);
	gChannelData[gBandIndex].channelNumberForDiscCycle = GetDlgItemInt(hDlg, IDC_EDIT_MCCHANNBDISCCYCLE,NULL ,FALSE);
	gChannelData[gBandIndex].nbChannels = GetDlgItemInt(hDlg, IDC_EDIT_MCNBCHANS,NULL ,FALSE);

}


VOID DTIC_LoadDTIChannelData(HWND hDlg)
{
	HWND localHandle = GetDlgItem(hDlg,ID_CBBOX_MCCHANINDEX);
	int CbBoxIndex;
	int i;

	gChannelIndex = 0;

	if (localHandle == NULL)
	{
		MessageBox(hDlg,L"Failed to get Channel Index Combobox Handler",L"ComboBox Handler Error",MB_OK);
	}
	// Clear ComboBox
	CbBoxIndex = ComboBox_GetCount(localHandle);
	for (i=0;i < CbBoxIndex; i++)
	{
		ComboBox_DeleteString(localHandle,ComboBox_GetCount(localHandle) - 1);
	}
	// Complete List of Channels
	for(i=0;i < gChannelData[gBandIndex].nbChannels;i++)
	{
		WCHAR tempBuff[3];
		_itow(i, tempBuff, 10);
		if (ComboBox_AddString(localHandle,tempBuff) != i)
		{
			MessageBox(hDlg, L"Failed to set Channel Index List", L"Channel Index List Error", MB_OK);
		}
	}

	ComboBox_SetCurSel(GetDlgItem(hDlg, ID_CBBOX_MCCHANINDEX), gChannelIndex);
	SetDlgItemInt(hDlg, IDC_EDIT_MCBAND,gChannelData[gBandIndex].bandNumber,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MCBSSTHRESHOLD,gChannelData[gBandIndex].bssThreshold,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MCCHANNBDISCCYCLE,gChannelData[gBandIndex].channelNumberForDiscCycle,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_MCNBCHANS,gChannelData[gBandIndex].nbChannels,FALSE);

	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_MCBANDINDEX),(UINT)gChannelData[gBandIndex].bandIndex);
}


// Message handler for Misc & channel configuration dialog box.
INT_PTR CALLBACK MISCWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND localHandle;
	int i;

	switch (message)
    {
        case WM_INITDIALOG:
			// Recover gChannelData from driver here
			gBandIndex = 0;
			for(i=0;i < RADIO_BAND_NUM_OF_BANDS;i++)
			{
				WL_GetDTIChannelParameters(i,&gChannelData[i]);
			}

			localHandle = GetDlgItem(hDlg,ID_CBBOX_MCBANDINDEX);
			if (localHandle == NULL)
			{
				MessageBox(hDlg,L"Failed to get Band Combobox Handler",L"ComboBox Handler Error",MB_OK);
				return (INT_PTR)FALSE;
			}
			// Complete List of Bands
			for(i=0;i < NB_BANDINDEX;i++)
			{
				if (ComboBox_AddString(localHandle,(char*)BandIndexList[i]) != i)
				{
					MessageBox(hDlg, L"Failed to set Band Rate List", L"Band Rate List Error", MB_OK);
					return (INT_PTR)FALSE;
				}
			}
			// Load Data
			DTIC_LoadDTIChannelData(hDlg);
			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
				gChannelData[gBandIndex].channelArray[gChannelIndex] = GetDlgItemInt(hDlg,IDC_EDIT_MCCHANNB, NULL, FALSE);
				DTIC_SaveDTIChannelData(hDlg);
				for(i=0;i < RADIO_BAND_NUM_OF_BANDS;i++)
				{
					WL_SetDTIChannelParameters(&gChannelData[i]);
				}
	            EndDialog(hDlg, message);
				return TRUE;
            }
            else if (LOWORD(wParam) == IDCANCEL)
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Channel Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}


			else if (LOWORD(wParam) == IDC_EDIT_MCBSSTHRESHOLD)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if ((GetDlgItemInt(hDlg,IDC_EDIT_MCBSSTHRESHOLD,NULL,TRUE) > 0) || GetDlgItemInt(hDlg,IDC_EDIT_MCBSSTHRESHOLD,NULL,TRUE) < -100)
					{
						MessageBox(hDlg, L"BSS Threshold must be in -100 -> 0", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MCBSSTHRESHOLD));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_MCCHANNBDISCCYCLE)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MCCHANNBDISCCYCLE,NULL,FALSE) > 30)
					{
						MessageBox(hDlg, L"Chan Number for discovery cycle must be in 0 -> 30", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MCCHANNBDISCCYCLE));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_MCBAND)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MCBAND,NULL,FALSE) > 1)
					{
						MessageBox(hDlg, L"Band number must be in 0 -> 1", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MCBAND));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_MCCHANNB)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MCCHANNB,NULL,FALSE) > 160)
					{
						MessageBox(hDlg, L"Channel Number must be in 0 -> 160", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MCCHANNB));
					}
				}
			}


			else if (LOWORD(wParam) == IDC_EDIT_MCNBCHANS)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_MCNBCHANS,NULL,FALSE) > 30)
					{
						MessageBox(hDlg, L"Number of channels must be in 0 -> 30", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_MCNBCHANS));
					}
					else
					{
						gChannelData[gBandIndex].nbChannels = GetDlgItemInt(hDlg,IDC_EDIT_MCNBCHANS,NULL,FALSE);
						DTIC_SaveDTIChannelData(hDlg);
						if (gChannelData[gBandIndex].nbChannels <= gChannelIndex)
							gChannelIndex = gChannelData[gBandIndex].nbChannels - 1;
						DTIC_LoadDTIChannelData(hDlg);
					}
				}
			}
			else if (LOWORD(wParam) == ID_CBBOX_MCBANDINDEX)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
				// Save last information
					DTIC_SaveDTIChannelData(hDlg);

					// Update Band Number Information.
					gBandIndex = ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_MCBANDINDEX));
					DTIC_LoadDTIChannelData(hDlg);
				}
				return TRUE;
			}

			else if (LOWORD(wParam) == ID_CBBOX_MCCHANINDEX)
			{
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					// Save last information
					gChannelData[gBandIndex].channelArray[gChannelIndex] = GetDlgItemInt(hDlg,IDC_EDIT_MCCHANNB, NULL, FALSE);
					// Update Channel Number information.
					gChannelIndex = ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_MCCHANINDEX));
					SetDlgItemInt(hDlg,IDC_EDIT_MCCHANNB,gChannelData[gBandIndex].channelArray[gChannelIndex], FALSE);

				}
				return TRUE;
			}
			break;

        case WM_CLOSE:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Channel Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
	}
	return (INT_PTR)FALSE;
}