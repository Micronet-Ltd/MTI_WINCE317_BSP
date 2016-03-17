//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

linecnfg.c

Abstract:  
  Called by unimodem to when the user wants to configure a
  datamodem device.

Functions:


Notes: 


--*/
#include <windows.h>
#include <types.h>
#include <commctrl.h>
#include <wcommctl.h>
#include <netui.h>
#include <netuip.h>
#include <resource.h>



typedef enum {
   SETTINGS_PAGE_PORT = 0, 
   SETTINGS_PAGE_DIALING,
   SETTINGS_NUM_PAGES
} PAGE_NUM;



typedef struct {
	PLINECONFIGDATA	pLineConfigData;
	RECT			rcDisplay;
	DWORD			fApply;
	BOOL			fPortrait;
} TABDIALOGINFO, *PTABDIALOGINFO;



#define LS_TO_TEMP(x)         (LoadString(v_hInst, x, szParamTemp, sizeof(szParamTemp)/sizeof(szParamTemp[0])) )
#define CB_ADD_ITEM2(w,x)      (SendDlgItemMessage (hDlg, w, \
                 CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR) x))
#define CB_ADD_ITEM(w,x)      (SendDlgItemMessage (hDlg, w, \
                 CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCTSTR)TEXT(x)))
#define CB_SELECT_ITEM(w, x)  (SendDlgItemMessage (hDlg, w, \
                 CB_SETCURSEL, (WPARAM) x, (LPARAM)0))
#define CB_GET_ITEM(w)        (SendDlgItemMessage (hDlg, w, \
                 CB_GETCURSEL, (WPARAM) 0, (LPARAM)0))

#define CHK_CHECK(w)          (SendDlgItemMessage (hDlg, w, \
                 BM_SETCHECK, (WPARAM) BST_CHECKED, (LPARAM)0))
#define CHK_UNCHECK(w)        (SendDlgItemMessage (hDlg, w, \
                 BM_SETCHECK, (WPARAM) BST_UNCHECKED, (LPARAM)0))

#define CHK_GET_CHECK(w)      (SendDlgItemMessage (hDlg, w, \
                 BM_GETCHECK, (WPARAM) 0, (LPARAM)0))



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
LRESULT CALLBACK
PortSettingsDlgProc (
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
	PROPSHEETPAGE *psp;
	TCHAR	szParamTemp[32];
	PTABDIALOGINFO	pTabInfo;
	NMHDR   *header;
	TCHAR   szSeparator[2];
	TCHAR   szOnePointFive[4];
	DWORD   dwOldBaudRate;

	DEBUGMSG (ZONE_LINECONFIG, (TEXT("+PortSettingsDlgProc(0x%X, 0x%X, 0x%X, 0x%X)\r\n"),
				   hDlg, message, wParam, lParam));

    switch (message) {
	case WM_NOTIFY :
		{
			header = (NMHDR*) lParam;
			if (!header)
				break;
			
			switch (header->code)
			{
			case PSN_HELP:
				{
					TCHAR cmdLine[256];
					RETAILMSG(1, (TEXT("PegHelp file:rnetw.htm#Main_Contents\r\n")));
					_tcscpy(cmdLine,TEXT("file:rnetw.htm#Main_Contents"));
					CreateProcess(TEXT("peghelp"),cmdLine,
						NULL,NULL,FALSE,0,NULL,NULL,NULL,NULL);
				}break;
				

			case PSN_APPLY:
				{

					DEBUGMSG (ZONE_LINECONFIG, (TEXT("PSN_APPLY in PortSettingsDlgProc\r\n")));
					pTabInfo = (PTABDIALOGINFO) GetWindowLong (hDlg, GWL_USERDATA);
					DEBUGMSG (ZONE_LINECONFIG, (TEXT("pTabInfo=0x%X\r\n"), pTabInfo));
					
					ASSERT (pTabInfo);
					
					pTabInfo->fApply = TRUE;
					
					dwOldBaudRate = pTabInfo->pLineConfigData->dwBaudRate;
					switch (CB_GET_ITEM(IDC_BAUD_RATE))  {
					case  0 : pTabInfo->pLineConfigData->dwBaudRate = CBR_110;     break;
					case  1 : pTabInfo->pLineConfigData->dwBaudRate = CBR_300;     break;
					case  2 : pTabInfo->pLineConfigData->dwBaudRate = CBR_600;     break;
					case  3 : pTabInfo->pLineConfigData->dwBaudRate = CBR_1200;    break;
					case  4 : pTabInfo->pLineConfigData->dwBaudRate = CBR_2400;    break;
					case  5 : pTabInfo->pLineConfigData->dwBaudRate = CBR_4800;    break;
					case  6 : pTabInfo->pLineConfigData->dwBaudRate = CBR_9600;    break;
					case  7 : pTabInfo->pLineConfigData->dwBaudRate = CBR_14400;   break;
					case  8 : pTabInfo->pLineConfigData->dwBaudRate = CBR_19200;   break;
					case  9 : pTabInfo->pLineConfigData->dwBaudRate = CBR_38400;   break;
					case 10 : pTabInfo->pLineConfigData->dwBaudRate = CBR_57600;   break;
					case 11 : pTabInfo->pLineConfigData->dwBaudRate = CBR_115200;  break;
					default : pTabInfo->pLineConfigData->dwBaudRate = CBR_9600;    break;
					}
					
					pTabInfo->pLineConfigData->bParity   = (BYTE) CB_GET_ITEM (IDC_PARITY);
					pTabInfo->pLineConfigData->bByteSize = (BYTE) CB_GET_ITEM (IDC_DATA_BITS) + 4;
					pTabInfo->pLineConfigData->bStopBits = (BYTE) CB_GET_ITEM (IDC_STOP_BITS);
					
					pTabInfo->pLineConfigData->dwModemOptions = 0;
					
					switch (CB_GET_ITEM(IDC_FLOW_CONTROL))  {
					case 0 : 
						pTabInfo->pLineConfigData->dwModemOptions |=
							NETUI_LCD_MDMOPT_SOFT_FLOW;
						break;
						
					case 1 : 
						pTabInfo->pLineConfigData->dwModemOptions |=
							NETUI_LCD_MDMOPT_HARD_FLOW;
						break;
						
					case 2 : 
						// set no bits...
						break;
					}
					
					if (BST_CHECKED == CHK_GET_CHECK(IDC_CHK_MANUAL))  {
						pTabInfo->pLineConfigData->dwTermOptions |= NETUI_LCD_TRMOPT_MANUAL_DIAL;
					} else {
						pTabInfo->pLineConfigData->dwTermOptions &= ~NETUI_LCD_TRMOPT_MANUAL_DIAL;
					}
					
					if (BST_CHECKED == CHK_GET_CHECK(IDC_CHK_PREDIAL))  {
						pTabInfo->pLineConfigData->dwTermOptions |= NETUI_LCD_TRMOPT_PRE_DIAL;
					} else {
						pTabInfo->pLineConfigData->dwTermOptions &= ~NETUI_LCD_TRMOPT_PRE_DIAL;
					}
					
					if (BST_CHECKED == CHK_GET_CHECK(IDC_CHK_POSTDIAL))  {
						pTabInfo->pLineConfigData->dwTermOptions |= NETUI_LCD_TRMOPT_POST_DIAL;
					} else {
						pTabInfo->pLineConfigData->dwTermOptions &= ~NETUI_LCD_TRMOPT_POST_DIAL;
					}

				}break;
			
			default:
				{
					// don't care, just fall through
				}
			}
		}break;



	case WM_INITDIALOG :

		DEBUGMSG (ZONE_LINECONFIG, (TEXT("WM_INITDIALOG in PortSettingsDlgProc\r\n")));

		// Don't allow anyone to drag us around.
		SetWindowLong (GetParent(hDlg), GWL_EXSTYLE,
					   GetWindowLong(GetParent(hDlg), GWL_EXSTYLE) |
					   WS_EX_NODRAG);
		
		psp = (PROPSHEETPAGE *)lParam;
		pTabInfo = (PTABDIALOGINFO)psp->lParam;
		SetWindowLong (hDlg, GWL_USERDATA, (LONG)pTabInfo);
		DEBUGMSG (ZONE_LINECONFIG, (TEXT("pTabInfo=0x%X\r\n"), pTabInfo));
		
		ASSERT (pTabInfo);
/***		
		// Try to center our parent
		{
			RECT rc;
			int cxScreen, cyScreen;
			GetWindowRect (GetParent(hDlg), &rc);
			cxScreen = GetSystemMetrics (SM_CXSCREEN);
			cyScreen = GetSystemMetrics (SM_CYSCREEN);

			DEBUGMSG (ZONE_LINECONFIG,
					  (TEXT("ParentRect (%d,%d,%d,%d) Screen(%d,%d)\r\n"),
					   rc.left, rc.top, rc.right, rc.bottom,
					   cxScreen, cyScreen));

			if (cxScreen > (rc.right - rc.left)) {
				cxScreen -= (rc.right - rc.left);
				cxScreen /= 2;
				rc.left = cxScreen;
			} else {
				rc.left = 0;
			}
			if ((cyScreen - 26) > (rc.bottom - rc.top)) {
				cyScreen -= 26;
				cyScreen -= (rc.bottom - rc.top);
				cyScreen /= 2;
				rc.top = cyScreen;
			} else {
				rc.top = 0;
			}
			DEBUGMSG (ZONE_LINECONFIG, (TEXT("Changing rc.left=%d rc.top=%d\r\n"),
						   rc.left, rc.top));
			SetWindowPos (GetParent(hDlg), HWND_TOP, rc.left, rc.top, 0, 0,
						  SWP_NOSIZE);
		}
//		SetWindowPos (hDlg, HWND_TOP, pTabInfo->rcDisplay.left,
//					  pTabInfo->rcDisplay.top, 0, 0, SWP_NOSIZE);
****/

		CB_ADD_ITEM (IDC_BAUD_RATE, "110");
		CB_ADD_ITEM (IDC_BAUD_RATE, "300");
		CB_ADD_ITEM (IDC_BAUD_RATE, "600");
		CB_ADD_ITEM (IDC_BAUD_RATE, "1200");
		CB_ADD_ITEM (IDC_BAUD_RATE, "2400");
		CB_ADD_ITEM (IDC_BAUD_RATE, "4800");
		CB_ADD_ITEM (IDC_BAUD_RATE, "9600");
		CB_ADD_ITEM (IDC_BAUD_RATE, "14400");
		CB_ADD_ITEM (IDC_BAUD_RATE, "19200");
		CB_ADD_ITEM (IDC_BAUD_RATE, "38400");
		CB_ADD_ITEM (IDC_BAUD_RATE, "57600");
		CB_ADD_ITEM (IDC_BAUD_RATE, "115200");

		DEBUGMSG (ZONE_LINECONFIG, (TEXT("added baud rates\r\n")));

		switch (pTabInfo->pLineConfigData->dwBaudRate)  {
		case CBR_110 :    CB_SELECT_ITEM(IDC_BAUD_RATE, 0); break;
		case CBR_300 :    CB_SELECT_ITEM(IDC_BAUD_RATE, 1); break;
		case CBR_600 :    CB_SELECT_ITEM(IDC_BAUD_RATE, 2); break;
		case CBR_1200 :   CB_SELECT_ITEM(IDC_BAUD_RATE, 3); break;
		case CBR_2400 :   CB_SELECT_ITEM(IDC_BAUD_RATE, 4); break;
		case CBR_4800 :   CB_SELECT_ITEM(IDC_BAUD_RATE, 5); break;
		case CBR_9600 :   CB_SELECT_ITEM(IDC_BAUD_RATE, 6); break;
		case CBR_14400 :  CB_SELECT_ITEM(IDC_BAUD_RATE, 7); break;
		case CBR_19200 :  CB_SELECT_ITEM(IDC_BAUD_RATE, 8); break;
		case CBR_38400 :  CB_SELECT_ITEM(IDC_BAUD_RATE, 9); break;
		case CBR_57600 :  CB_SELECT_ITEM(IDC_BAUD_RATE, 10); break;
		case CBR_115200 : CB_SELECT_ITEM(IDC_BAUD_RATE, 11); break;
		default :         CB_SELECT_ITEM(IDC_BAUD_RATE, 6); break;
		}

		LS_TO_TEMP(IDS_PARAMETER_NONE);        //PARITY None
		CB_ADD_ITEM2 (IDC_PARITY, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_ODD);         //PARITY Odd
		CB_ADD_ITEM2 (IDC_PARITY, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_EVEN);        //PARITY Even
		CB_ADD_ITEM2 (IDC_PARITY, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_MARK);        //PARITY Mark
		CB_ADD_ITEM2 (IDC_PARITY, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_SPACE);       //PARITY Space
		CB_ADD_ITEM2 (IDC_PARITY, szParamTemp);
		CB_SELECT_ITEM (IDC_PARITY, pTabInfo->pLineConfigData->bParity);

		LS_TO_TEMP(IDS_PARAMETER_SOFTWARE);    //FLOW software
		CB_ADD_ITEM2 (IDC_FLOW_CONTROL, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_HARDWARE);    //FLOW hardware
		CB_ADD_ITEM2 (IDC_FLOW_CONTROL, szParamTemp);
		LS_TO_TEMP(IDS_PARAMETER_NONE);        //FLOW None
		CB_ADD_ITEM2 (IDC_FLOW_CONTROL, szParamTemp);

		if (pTabInfo->pLineConfigData->dwModemOptions &
			NETUI_LCD_MDMOPT_SOFT_FLOW)  {
			CB_SELECT_ITEM (IDC_FLOW_CONTROL, 0);
		} else if (pTabInfo->pLineConfigData->dwModemOptions &
				   NETUI_LCD_MDMOPT_HARD_FLOW)  {
			CB_SELECT_ITEM (IDC_FLOW_CONTROL, 1);
		} else  {
			CB_SELECT_ITEM (IDC_FLOW_CONTROL, 2);   // no flow control
		}

		CB_ADD_ITEM (IDC_DATA_BITS, "4");
		CB_ADD_ITEM (IDC_DATA_BITS, "5");
		CB_ADD_ITEM (IDC_DATA_BITS, "6");
		CB_ADD_ITEM (IDC_DATA_BITS, "7");
		CB_ADD_ITEM (IDC_DATA_BITS, "8");
		CB_SELECT_ITEM (IDC_DATA_BITS, pTabInfo->pLineConfigData->bByteSize - 4);            

        //get locale specific separator
        if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szSeparator, 2)) {
            //function failed, use period
            lstrcpy(szSeparator, _T("."));
        }    

        //build the stop bit 1x4 string
        lstrcpy(szOnePointFive, _T("1"));
        lstrcat(szOnePointFive, szSeparator);
        lstrcat(szOnePointFive, _T("5"));
        
		CB_ADD_ITEM (IDC_STOP_BITS, "1");
		SendDlgItemMessage (hDlg, IDC_STOP_BITS, CB_ADDSTRING, 0, (LPARAM)szOnePointFive);
		CB_ADD_ITEM (IDC_STOP_BITS, "2");
		CB_SELECT_ITEM (IDC_STOP_BITS, pTabInfo->pLineConfigData->bStopBits);

        if (pTabInfo->pLineConfigData->dwTermOptions & NETUI_LCD_TRMOPT_MANUAL_DIAL) {
            CHK_CHECK (IDC_CHK_MANUAL);
        } else  {
            CHK_UNCHECK(IDC_CHK_MANUAL);
        }

        if (pTabInfo->pLineConfigData->dwTermOptions & NETUI_LCD_TRMOPT_PRE_DIAL) {
            CHK_CHECK (IDC_CHK_PREDIAL);
        } else  {
            CHK_UNCHECK (IDC_CHK_PREDIAL);
        }

        if (pTabInfo->pLineConfigData->dwTermOptions & NETUI_LCD_TRMOPT_POST_DIAL) {
            CHK_CHECK (IDC_CHK_POSTDIAL);
        } else  {
            CHK_UNCHECK (IDC_CHK_POSTDIAL);
        }

		DEBUGMSG (ZONE_LINECONFIG,
				  (TEXT("-PortSettingsDlgProc: WM_INITDIALOG Return FALSE\r\n")));
		return FALSE;

	default :
		DEBUGMSG (ZONE_LINECONFIG,
				  (TEXT("-PortSettingsDlgProc: (default) Return FALSE\r\n")));
		return (FALSE);
	}
	DEBUGMSG (ZONE_LINECONFIG,
			  (TEXT("-PortSettingsDlgProc: Return TRUE\r\n")));
	return TRUE;
}




// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
LRESULT CALLBACK
CallOptionsDlgProc (
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
	PROPSHEETPAGE *psp;
	TCHAR szTemp[21]={0};
	UINT  nValue;
	TCHAR szTemp2[(NETUI_LCD_DIAL_MOD_LEN+1)*2]={0};
	TCHAR szTemp3[32]={0};
	TCHAR szTemp4[32]={0};
	PTABDIALOGINFO	pTabInfo;
	NMHDR   *header;


   switch (message)
   {

	case WM_COMMAND :
		{
			if (BN_CLICKED == HIWORD(wParam) && IDC_CHK_TIMEOUT == LOWORD(wParam))
			{
				BOOL bChecked = (BST_CHECKED == CHK_GET_CHECK(IDC_CHK_TIMEOUT));
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TIMEOUT), bChecked);
			}
		}break;

	case WM_NOTIFY :
		{
			header = (NMHDR*) lParam;
			if (!header)
				break;
			
			switch (header->code)
			{
			case PSN_HELP:
				{
					TCHAR cmdLine[256];
					RETAILMSG(1, (TEXT("PegHelp file:rnetw.htm#Main_Contents\r\n")));
					_tcscpy(cmdLine,TEXT("file:rnetw.htm#Main_Contents"));
					CreateProcess(TEXT("peghelp"),cmdLine,
						NULL,NULL,FALSE,0,NULL,NULL,NULL,NULL);
				}break;
				
				
			case PSN_APPLY:
				{
					DEBUGMSG (ZONE_LINECONFIG, (TEXT("PSN_APPLY in CallOptionsDlgProc\r\n")));
					pTabInfo = (PTABDIALOGINFO) GetWindowLong (hDlg, GWL_USERDATA);
					DEBUGMSG (ZONE_LINECONFIG, (TEXT("pTabInfo=0x%X\r\n"), pTabInfo));
					ASSERT (pTabInfo);
					
					pTabInfo->fApply = TRUE;
					
					// LAM - Note that this is backwards.  The box actually gets checked
					// if we do NOT want to do a blind dial.
					if (1 == CHK_GET_CHECK(IDC_CHK_BLIND_DIAL))  {
						pTabInfo->pLineConfigData->dwModemOptions &=
							~(NETUI_LCD_MDMOPT_BLIND_DIAL);
					} else  {
						pTabInfo->pLineConfigData->dwModemOptions |=
							NETUI_LCD_MDMOPT_BLIND_DIAL;
					}
					
					// 
					// Map Fullwidth to halfwidth numbers
					//
					GetDlgItemText (hDlg, IDC_EXTRA, szTemp2,
						sizeof(szTemp2)/sizeof(szTemp2[0]));
					LCMapString (LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH, 
						szTemp2, _tcslen(szTemp2)+1, 
						pTabInfo->pLineConfigData->szDialModifier,
						pTabInfo->pLineConfigData->dwModMaxLen+1);
					
					
					// Check for maximum bong....
					// 
					// Map Fullwidth to halfwidth numbers
					//
					GetDlgItemText (hDlg, IDC_EDIT_WAITBONG, szTemp3, 16);
					LCMapString(LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH, szTemp3,
						_tcslen(szTemp3)+1, szTemp4, 16);
					nValue = (UINT)_ttol(szTemp4);
					
					pTabInfo->pLineConfigData->wWaitBong = (nValue > 0xFFFF) ? 0xFFFF : nValue;
					
					// 
					// Map Fullwidth to halfwidth numbers
					//
					if (BST_CHECKED == CHK_GET_CHECK(IDC_CHK_TIMEOUT)) {
						GetDlgItemText (hDlg, IDC_EDIT_TIMEOUT, szTemp3, 16);
						LCMapString(LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH, szTemp3,_tcslen(szTemp3)+1, szTemp4, 16);
						pTabInfo->pLineConfigData->dwCallSetupFailTimer = (DWORD)_ttol(szTemp4);
					} else {
						pTabInfo->pLineConfigData->dwCallSetupFailTimer = 0;
					}
					
					DEBUGMSG (ZONE_LINECONFIG, (
						TEXT("*** CallOptionsDlgProc ***\r\n")
						TEXT("\tExtra Settings  = %s\r\n")
						TEXT("\tWait bong       = %d seconds\r\n")
						TEXT("\tTimeout         = %d seconds\r\n"),
						pTabInfo->pLineConfigData->szDialModifier,
						pTabInfo->pLineConfigData->wWaitBong,
						pTabInfo->pLineConfigData->dwCallSetupFailTimer
						));
				}break;
			
			default:
				{
					// don't care, just fall through
				}
			}
		}break;

	case WM_INITDIALOG :
		DEBUGMSG (ZONE_LINECONFIG, (TEXT("WM_INITDIALOG in CallOptionsDlgProc\r\n")));
		psp = (PROPSHEETPAGE *)lParam;
		pTabInfo = (PTABDIALOGINFO)psp->lParam;
		SetWindowLong (hDlg, GWL_USERDATA, (LONG)pTabInfo);
		DEBUGMSG (ZONE_LINECONFIG, (TEXT("pTabInfo=0x%X\r\n"), pTabInfo));

		// Don't move around if portrait
		if (!(pTabInfo->fPortrait)) {
			//
			// Move the controls around dynamically so that different languages
			// still look okay.
			//
			DWORD dwCancelWidth, dwCancelHeight;
			RECT rcCancel, rcDlg, rcEdit, rcSec;
			TCHAR szTempText[132];
			SIZE mysize;
			HDC hdc;
			HWND hControl;

#define BUFFER_WIDTH 4

			hControl = GetDlgItem(hDlg, IDC_CHK_TIMEOUT);

			hdc = GetWindowDC (hControl);

			GetWindowRect (hDlg, &rcDlg);
			GetWindowRect (GetDlgItem(hDlg, IDC_EDIT_TIMEOUT), &rcEdit);
			GetWindowRect (GetDlgItem(hDlg, IDC_CHK_TIMEOUT),  &rcCancel);
			GetWindowRect (GetDlgItem(hDlg, IDC_TEXT_SECONDS), &rcSec);

			GetDlgItemText(hDlg, IDC_CHK_TIMEOUT, szTempText, sizeof(szTempText)/sizeof(TCHAR) - sizeof(TCHAR));
			GetTextExtentPoint32 (hdc, szTempText, _tcslen(szTempText), &mysize);

			dwCancelWidth  = mysize.cx + 22; // include some constant width for checkbox.
			dwCancelHeight = rcCancel.bottom - rcCancel.top;
			//
			// Resize the "cancel the call..." string
			//
			SetWindowPos (hControl, 0, 0, 0, dwCancelWidth, dwCancelHeight, 
						  SWP_NOMOVE | SWP_NOZORDER);
			//
			// Move the seconds EDIT box in flush.
			//
			SetWindowPos (GetDlgItem(hDlg, IDC_EDIT_TIMEOUT), 0,
						  rcCancel.left - rcDlg.left + dwCancelWidth + BUFFER_WIDTH, 
						  rcCancel.top - rcDlg.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			//
			// Move the "seconds" string to the left.
			// 
			SetWindowPos (GetDlgItem(hDlg, IDC_TEXT_SECONDS), 0,
						  rcCancel.left - rcDlg.left + dwCancelWidth + BUFFER_WIDTH*2 + rcEdit.right - rcEdit.left, 
						  rcSec.top - rcDlg.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			ReleaseDC(hControl, hdc);
		}


		if (pTabInfo->pLineConfigData->dwCallSetupFailTimer)  {
			CHK_CHECK (IDC_CHK_TIMEOUT);
		} else  {
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TIMEOUT), FALSE);
			CHK_UNCHECK (IDC_CHK_TIMEOUT);
		}

		// LAM - Note that this is backwards.  The box actually gets checked
		// if we do NOT want to do a blind dial.
		if (pTabInfo->pLineConfigData->dwModemOptions &
			NETUI_LCD_MDMOPT_BLIND_DIAL)  {
			CHK_UNCHECK (IDC_CHK_BLIND_DIAL);
		} else  {
			CHK_CHECK (IDC_CHK_BLIND_DIAL);
		}

		//
		// Don't let the edit fields be huge...
		//

		// RAID #10485,10486 fix. 
		// Make this window IME disabled
		ImmAssociateContext(GetDlgItem(hDlg, IDC_EXTRA), (HIMC)NULL);

		SendDlgItemMessage (hDlg, IDC_EXTRA, EM_SETLIMITTEXT, 
							(WPARAM) pTabInfo->pLineConfigData->dwModMaxLen,
							(LPARAM) 0);

		SendDlgItemMessage (hDlg, IDC_EDIT_TIMEOUT, EM_SETLIMITTEXT, 
							(WPARAM) 4, (LPARAM) 0);

		SendDlgItemMessage (hDlg, IDC_EDIT_WAITBONG, EM_SETLIMITTEXT, 
							(WPARAM) 4, (LPARAM) 0);

		SetDlgItemText(hDlg, IDC_EXTRA,
					   pTabInfo->pLineConfigData->szDialModifier);

		wsprintf (szTemp, TEXT("%d"), pTabInfo->pLineConfigData->wWaitBong);
		SetDlgItemText(hDlg, IDC_EDIT_WAITBONG, szTemp);

		wsprintf (szTemp, TEXT("%u"), pTabInfo->pLineConfigData->dwCallSetupFailTimer);
		SetDlgItemText(hDlg, IDC_EDIT_TIMEOUT, szTemp);

		break;

	default :
		return (FALSE);

	}
	return FALSE;
}

int CALLBACK 
PropSheetProc(
   HWND hwndDlg,  // handle to the property sheet dialog box
   UINT uMsg,     // message identifier
   LPARAM lParam  // message parameter
   )
{
	DEBUGMSG (ZONE_LINECONFIG, (TEXT("PropSheetProc(0x%X, 0x%X, 0x%X)\r\n"),
				   hwndDlg, uMsg, lParam));
//   if (uMsg == PSCB_INITIALIZED)  {
//      g_PropSheetWnd = hwndDlg;
//   }
   return 0;
}   




// 
// ----------------------------------------------------------------
//
//	LineConfigEdit
//
//  Called by unimodem to when the user wants to configure a
//	datamodem device.
//
// ----------------------------------------------------------------
BOOL
LineConfigEdit (HWND hParent, PLINECONFIGDATA pLineConfigData)
{

	PROPSHEETPAGE psp[SETTINGS_NUM_PAGES];
	PROPSHEETHEADER psh;
	TCHAR szCaption1[30];
	TCHAR szCaption2[30];
	TCHAR szTitle[30];
	PTABDIALOGINFO	pTabInfo;
	BOOL			fRetVal = FALSE;

	DEBUGMSG (ZONE_FUNCTION|ZONE_LINECONFIG,
			  (TEXT("+LineConfigEdit(0x%X, 0x%X)\r\n"),
			   hParent, pLineConfigData));

	// We need the common control functions
	if (!ValidCommCtrlFunc(InitCommonControls)) {
		DEBUGMSG (ZONE_ERROR|ZONE_LINECONFIG,
				  (TEXT("!LineConfigEdit: Common Controls not loaded\r\n")));
		return FALSE;
	}

	// Initialize the common controls.
	CallCommCtrlFunc(InitCommonControls)();

	// I need PropertySheetW as well.
	if (!ValidCommCtrlFunc(PropertySheetW)) {
		DEBUGMSG (ZONE_ERROR|ZONE_LINECONFIG,
				  (TEXT("!LineConfigEdit: PropertySheetW not available\r\n")));
		return FALSE;
	}
	
	pTabInfo = LocalAlloc (LPTR, sizeof(TABDIALOGINFO));
	DEBUGMSG (ZONE_LINECONFIG, (TEXT("pTabInfo = 0x%X\r\n"), pTabInfo));

	if (NULL == pTabInfo) {
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	pTabInfo->pLineConfigData = pLineConfigData;

	if (GetSystemMetrics(SM_CXSCREEN) < 480) {
		// Use the portrait mode dialogs
		pTabInfo->fPortrait = TRUE;
	} else {
		pTabInfo->fPortrait = FALSE;
	}

#define SP_PORT SETTINGS_PAGE_PORT
	psp[SP_PORT].dwSize = sizeof(PROPSHEETPAGE);
	psp[SP_PORT].dwFlags = PSP_USETITLE;
	psp[SP_PORT].hInstance = v_hInst;
	psp[SP_PORT].pszTemplate = MAKEINTRESOURCE(pTabInfo->fPortrait ?
											   IDD_PORT_SETTINGS2 :
											   IDD_PORT_SETTINGS);
	psp[SP_PORT].pszIcon = NULL;
	psp[SP_PORT].pfnDlgProc = PortSettingsDlgProc;
	LoadString(v_hInst, IDS_PORT_SETTINGS, szCaption1,
			   sizeof(szCaption1)/sizeof(szCaption1[0]));
	psp[SP_PORT].pszTitle = szCaption1;
	psp[SP_PORT].lParam = (LPARAM)pTabInfo;
	psp[SP_PORT].pfnCallback = NULL;

#define SP_EMUL SETTINGS_PAGE_DIALING
	psp[SP_EMUL].dwSize = sizeof(PROPSHEETPAGE);
	psp[SP_EMUL].dwFlags = PSP_USETITLE;
	psp[SP_EMUL].hInstance = v_hInst;
	psp[SP_EMUL].pszTemplate = MAKEINTRESOURCE(pTabInfo->fPortrait ?
											   IDD_DIALING_SETTINGS2 :
											   IDD_DIALING_SETTINGS);
	psp[SP_EMUL].pszIcon = NULL;
	psp[SP_EMUL].pfnDlgProc = CallOptionsDlgProc;
	LoadString(v_hInst, IDS_CALL_OPTIONS, szCaption2,
			   sizeof(szCaption2)/sizeof(szCaption2[0]));
	psp[SP_EMUL].pszTitle = szCaption2;
	psp[SP_EMUL].lParam = (LPARAM)pTabInfo;
	psp[SP_EMUL].pfnCallback = NULL;


	psh.dwSize     = sizeof(PROPSHEETHEADER);
	psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	if ((GetFileAttributes(PEGHELP_EXE) != -1) && (GetFileAttributes(HELP_FILE) != -1) )
		psh.dwFlags |= PSH_HASHELP;
	psh.hwndParent = hParent;
	psh.hInstance  = v_hInst;
	psh.pszIcon    = NULL;
	LoadString(v_hInst, IDS_DEVICE_PROPERTIES, szTitle,
			   sizeof(szTitle)/sizeof(szTitle[0]));
	psh.pszCaption = (LPTSTR) szTitle;
	psh.nPages     = SETTINGS_NUM_PAGES;
	psh.nStartPage = 0;       // we will always jump to the first page
	psh.ppsp       = (LPCPROPSHEETPAGE) psp;
	psh.pfnCallback = PropSheetProc;

	
	CallCommCtrlFunc(PropertySheetW)(&psh);

	if (pTabInfo->fApply) {
		DEBUGMSG (ZONE_LINECONFIG, (TEXT("Apply the changes\r\n")));
		fRetVal = TRUE;
	}
	LocalFree (pTabInfo);
	DEBUGMSG (ZONE_FUNCTION|ZONE_LINECONFIG,
			  (TEXT("-LineConfigEdit: %d\r\n"), fRetVal));
	return fRetVal;
	
}
