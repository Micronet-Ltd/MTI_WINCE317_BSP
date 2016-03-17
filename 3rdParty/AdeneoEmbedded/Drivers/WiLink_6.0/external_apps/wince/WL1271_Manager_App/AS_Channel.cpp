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
 
/** \file   AS_Channel.cpp 
 *  \brief  WL1271 Application Scan mode management - Channel configuration
 *
 *  \see    AS_Channel.h
 */
#include <windows.h>
#include <windowsx.h>

#include "802_11Defs.h"

#include "resource.h"
#include "AS_Channel.h"
#include "Wifi.h"

#include "AS_Global.h"

#define MAX_NBCHANS 17

typedef WCHAR* ChannelList_t;
typedef ChannelList_t ChannelListDesc[MAX_NBCHANS];
static const ChannelListDesc ChannelList = 
{L"0",L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9",L"10",L"11",L"12",L"13",L"14",L"15",L"16"};

static AS_Channel_t gAsChannelData[MAX_NUMBER_OF_CHANNELS_PER_SCAN];
static TI_UINT8		gChanIndex;

BOOL ASC_SaveChannelData(HWND hDlg)
{
	TI_UINT8 pMacAddr[6];
	WCHAR tempBssid[17];
	
	GetDlgItemText(hDlg, IDC_EDIT_ASCBSSID,tempBssid,17);
	int iscan = swscanf(tempBssid, L"%2X:%2X:%2X:%2X:%2X:%2X",
							&pMacAddr[5],
							&pMacAddr[4],
							&pMacAddr[3],
							&pMacAddr[2],
							&pMacAddr[1],
							&pMacAddr[0]);
	if (iscan != 6 && 
		((pMacAddr[5] & 0xFF) <= 0xFF) &&
		((pMacAddr[4] & 0xFF) <= 0xFF) &&
		((pMacAddr[3] & 0xFF) <= 0xFF) &&
		((pMacAddr[2] & 0xFF) <= 0xFF) &&
		((pMacAddr[1] & 0xFF) <= 0xFF) &&
		((pMacAddr[0] & 0xFF) <= 0xFF) )
	{
		return FALSE;
	}

	// Update buffer
	gAsChannelData[gChanIndex].BSSID[0] = pMacAddr[5]&0xFF;
	gAsChannelData[gChanIndex].BSSID[1] = pMacAddr[4]&0xFF;
	gAsChannelData[gChanIndex].BSSID[2] = pMacAddr[3]&0xFF;
	gAsChannelData[gChanIndex].BSSID[3] = pMacAddr[2]&0xFF;
	gAsChannelData[gChanIndex].BSSID[4] = pMacAddr[1]&0xFF;
	gAsChannelData[gChanIndex].BSSID[5] = pMacAddr[0]&0xFF;

	gAsChannelData[gChanIndex].ChanIndex = gChanIndex;
	gAsChannelData[gChanIndex].MaxDwellTime = GetDlgItemInt(hDlg, IDC_EDIT_ASCMAXDWELL,NULL ,FALSE);
	gAsChannelData[gChanIndex].MinDwellTime = GetDlgItemInt(hDlg, IDC_EDIT_ASCMINDWELL,NULL ,FALSE);
	gAsChannelData[gChanIndex].ETFrameNumber = GetDlgItemInt(hDlg, IDC_EDIT_ASCETFRAMENB,NULL ,FALSE);
	gAsChannelData[gChanIndex].TxPowerLevel = GetDlgItemInt(hDlg, IDC_EDIT_ASCTXPOWERLVL,NULL ,FALSE);
	gAsChannelData[gChanIndex].ChanNumber = GetDlgItemInt(hDlg, IDC_EDIT_ASCCHANNB,NULL ,FALSE);
	gAsChannelData[gChanIndex].ETCondition = EScanEtCondition(ComboBox_GetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASCETCOND)));
	return TRUE;
}


VOID ASC_LoadChannelData(HWND hDlg)
{
	WCHAR tempBssid[17];
	wsprintf(tempBssid, L"%02X:%02X:%02X:%02X:%02X:%02X",
							gAsChannelData[gChanIndex].BSSID[0],
							gAsChannelData[gChanIndex].BSSID[1],
							gAsChannelData[gChanIndex].BSSID[2],
							gAsChannelData[gChanIndex].BSSID[3],
							gAsChannelData[gChanIndex].BSSID[4],
							gAsChannelData[gChanIndex].BSSID[5]);

	SetDlgItemText(hDlg, IDC_EDIT_ASCBSSID,tempBssid);
	SetDlgItemInt(hDlg, IDC_EDIT_ASCMAXDWELL,gAsChannelData[gChanIndex].MaxDwellTime,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASCMINDWELL,gAsChannelData[gChanIndex].MinDwellTime,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASCETFRAMENB,gAsChannelData[gChanIndex].ETFrameNumber,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASCTXPOWERLVL,gAsChannelData[gChanIndex].TxPowerLevel,FALSE);
	SetDlgItemInt(hDlg, IDC_EDIT_ASCCHANNB,gAsChannelData[gChanIndex].ChanNumber,FALSE);

	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASCCHANINDEX),(UINT)gAsChannelData[gChanIndex].ChanIndex);
	ComboBox_SetCurSel(GetDlgItem(hDlg,ID_CBBOX_ASCETCOND),(UINT)gAsChannelData[gChanIndex].ETCondition);
}


// Message handler for ASC configuration dialog box.
INT_PTR CALLBACK ASCWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND localHandle;
	int i;
	TI_UINT8 ASG_nbChannels = ASG_GetNumberOfChannels();

	switch (message)
    {
        case WM_INITDIALOG:

				// Init the Combobox content for Early Termination Events
				localHandle = GetDlgItem(hDlg,ID_CBBOX_ASCETCOND);
				gChanIndex = 0;
				if (localHandle == NULL)
				{
					MessageBox(hDlg,L"Failed to get ET Condition ComboBox Handler",L"ComboBox Handler Error",MB_OK);
					return (INT_PTR)FALSE;
				}
				// Complete list of Early Termination Events
				for(i=0;i < NB_ETEVENTS;i++)
				{
					if (ComboBox_AddString(localHandle,(char*)ETEventNames[i].EvtName) != i)
					{
						MessageBox(hDlg, L"Failed to set ET Condition List", L"ET Condition List Error", MB_OK);
						return (INT_PTR)FALSE;
					}
				}
				localHandle = GetDlgItem(hDlg,ID_CBBOX_ASCCHANINDEX);
				if (localHandle == NULL)
				{
					MessageBox(hDlg,L"Failed to get Channel index ComboBox Handler",L"ComboBox Handler Error",MB_OK);
					return (INT_PTR)FALSE;
				}
				// Complete list of Channels
				for(i=0;i<ASG_nbChannels;i++)
				{
					if(ComboBox_AddString(localHandle,(char*)ChannelList[i]) != i)
					{
						MessageBox(hDlg, L"Failed to set Channel List", L"Channel List Error", MB_OK);
						return (INT_PTR)FALSE;
					}
				}
				for (i=0;i<ASG_nbChannels;i++)
				{
					WL_GetASChannelParameters(i,&gAsChannelData[i]);
				}
				ASC_LoadChannelData(hDlg);

			return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
				ASC_SaveChannelData(hDlg);
				for (i=0;i<ASG_nbChannels;i++)
				{
					WL_SetASChannelParameters(&gAsChannelData[i]);
				}	
				EndDialog(hDlg, message);
				return TRUE;
            }


			else if (LOWORD(wParam) == IDC_EDIT_ASCBSSID)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					WCHAR tempBssid[20];
					// As GetDlgItemText returns the number of characters
					// read, reading only 17 characters won't allow us to
					// see if parameter contains more characters than required.
					// So, we try to read 20 chars, and we only want 17.
					if (GetDlgItemText(hDlg,IDC_EDIT_ASCBSSID,tempBssid,20) != 17)
					{
						MessageBox(hDlg, L"BSSID must be in format : FF:FF:FF:FF:FF:FF", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCBSSID));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASCMAXDWELL)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASCMAXDWELL,NULL,FALSE) > 100000000)
					{
						MessageBox(hDlg, L"Max Dwell Time must be in 0 -> 100000000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCMAXDWELL));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASCMINDWELL)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASCMINDWELL,NULL,FALSE) > 100000000)
					{
						MessageBox(hDlg, L"Min Dwell Time must be in 0 -> 100000000", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCMINDWELL));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASCETFRAMENB)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASCETFRAMENB,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"ET Frame Number must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCETFRAMENB));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASCTXPOWERLVL)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASCTXPOWERLVL,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Tx Power Level must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCTXPOWERLVL));
					}
				}
			}
			else if (LOWORD(wParam) == IDC_EDIT_ASCCHANNB)
			{
				if (HIWORD(wParam) == EN_KILLFOCUS)
				{
					// Save last information
					if (GetDlgItemInt(hDlg,IDC_EDIT_ASCCHANNB,NULL,FALSE) > 255)
					{
						MessageBox(hDlg, L"Channel Number must be in 0 -> 255", L"Invalid Parameter", MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(hDlg,IDC_EDIT_ASCCHANNB));
					}
				}
			}



			else if (LOWORD(wParam) == ID_CBBOX_ASCCHANINDEX)
			{
				ASC_SaveChannelData(hDlg);
				gChanIndex = ComboBox_GetCurSel(GetDlgItem(hDlg, ID_CBBOX_ASCCHANINDEX));
				ASC_LoadChannelData(hDlg);
			}
            else if (LOWORD(wParam) == IDCANCEL)
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Application Scan - Channel Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
			break;

        case WM_CLOSE:
			{
				if (MessageBox(hDlg, L"Cancel Modifications?", L"Application Scan - Channel Configuration", MB_OKCANCEL | MB_ICONEXCLAMATION) == IDOK)
				{
					// User confirmed the cancel operation
					EndDialog(hDlg, LOWORD(wParam));
				}
                return TRUE;
			}
            return TRUE;
	}
	return (INT_PTR)FALSE;
}