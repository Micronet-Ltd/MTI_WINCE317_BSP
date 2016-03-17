//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

netcard.c

Abstract:  

Functions to get the driver name of a newly inserted PCCard from the user.

Functions:


Notes: 


--*/
#include <windows.h>
#include <types.h>
#include <netui.h>
#include <netuip.h>
#include <resource.h>

#define DEF_STR_LEN 128


// 
// ----------------------------------------------------------------
//
//	Device driver name dialog box.
//
//  Called by DEVICE.EXE to prompt for the driver name for a newly inserted
//  PCCard.  This only occurs when DEVICE.EXE cannot recognize a card.
//
//  Also called by the USB client loader if an unrecognized device is attached. 
//
// ----------------------------------------------------------------
LRESULT CALLBACK
DlgProcDriverName( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	PGETDRIVERNAMEPARMS	pDriverParms;
	DWORD			dwStyle;
    LPWSTR pDriverName;
    DWORD TypeStr;
    DWORD BufLen;
    DWORD StrLen;
    WCHAR TypeName[DEF_STR_LEN];
    WCHAR SocketString[DEF_STR_LEN];
    
    switch( message ) {
    case WM_INITDIALOG:
		DEBUGMSG (ZONE_DRIVERNAME, (TEXT("DlgProcDriverName: INITDIALOG\r\n")));

		pDriverParms = (PGETDRIVERNAMEPARMS)lParam;
        pDriverName = pDriverParms->DriverName;

        //
        // Format initial display of driver name to be like:
        // "<card type string> in socket <n>"
        // Note USB also shares this dialog, in this case driver name string 
        // will be passed in. 
        //

        if (pDriverParms->PCCardType & PCCARDTYPE_USB) {
           // Change window title and text for USB
           StrLen = LoadString(v_hInst, IDS_USB_CAPTION,SocketString,DEF_STR_LEN);
           if (StrLen)
              SetWindowText(hDlg, SocketString);
           StrLen = LoadString(v_hInst, IDS_USB_CMDTEXT,SocketString,DEF_STR_LEN);
           if (StrLen) 
              SetDlgItemText( hDlg, IDC_CMDTEXT,SocketString);
        }
        else {
            if ((pDriverParms->PCCardType >= 0) &&
                (pDriverParms->PCCardType < 9)) {
                TypeStr = IDS_PCCARD_TYPE_MFC + pDriverParms->PCCardType;
            } else {
                TypeStr = IDS_PCCARD_TYPE_UNKNOWN;
            }

            BufLen = DEF_STR_LEN;
            pDriverName[0] = 0;
            StrLen = LoadString(v_hInst, TypeStr, TypeName, BufLen);
            if (StrLen) {
                BufLen = DEF_STR_LEN;
                StrLen = LoadString(v_hInst, IDS_PCCARD_SOCKET, SocketString, BufLen);
                if (StrLen) {
    				DWORD	rgdwIDList[2];
    
    				rgdwIDList[0] = (DWORD)TypeName;
    				rgdwIDList[1] = pDriverParms->Socket + 1;
    				if (FormatMessage (
                           FORMAT_MESSAGE_FROM_STRING |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           (LPCVOID)SocketString,
                           IDS_PCCARD_SOCKET,
                           0,
                           pDriverName,
                           sizeof(pDriverParms->DriverName),
                           (va_list *)rgdwIDList)) {
                    }
                }
            }
        }
		SetWindowLong (hDlg, DWL_USER, (LONG)pDriverParms);

		dwStyle = GetWindowLong (hDlg, GWL_EXSTYLE);
		SetWindowLong (hDlg, GWL_EXSTYLE, dwStyle | WS_EX_CAPTIONOKBTN | WS_EX_TOPMOST);
		SetDlgItemText( hDlg, IDC_DRIVERNAME, pDriverParms->DriverName);
		SetFocus(GetDlgItem(hDlg, IDC_DRIVERNAME));
		SendDlgItemMessage (hDlg, IDC_DRIVERNAME, EM_SETLIMITTEXT, 
							(WPARAM) DRIVER_NAME_LEN, (LPARAM) 0);
        SendMessage(GetDlgItem(hDlg, IDC_DRIVERNAME), EM_SETSEL, 0, -1);
		return FALSE;

    case WM_COMMAND:
		pDriverParms = (PGETDRIVERNAMEPARMS)GetWindowLong (hDlg, DWL_USER);
        
        switch( LOWORD( wParam ) ) {
        case IDCANCEL: 				// Ignore values

            EndDialog( hDlg, FALSE );
            break;

        case IDOK: 					// Get the dialog values
			DEBUGMSG (ZONE_DRIVERNAME, (TEXT("User pressed OK\r\n")));
            GetDlgItemText( hDlg, IDC_DRIVERNAME,
							pDriverParms->DriverName, DRIVER_NAME_LEN );
            EndDialog( hDlg, TRUE );
            break;
        }

    case WM_DESTROY:

        return( FALSE );

    default:
		
        return( FALSE );
        break;
    }
}


BOOL
GetDriverName (HWND hParent, PGETDRIVERNAMEPARMS pDriverParms)
{
	HRSRC hRsrc;
	HGLOBAL hGlob;
    int iRet;
	
	DEBUGMSG (ZONE_DRIVERNAME, (TEXT("GetDriverName: Enter\r\n")));

	hRsrc  = FindResource (v_hInst, MAKEINTRESOURCE(IDD_DRIVERNAME), RT_DIALOG);
    if (hRsrc == NULL)
        return FALSE;
	hGlob = LoadResource (v_hInst, hRsrc);
    if (hGlob == NULL) 
        return FALSE;
	
	iRet = DialogBoxParam (v_hInst, MAKEINTRESOURCE(IDD_DRIVERNAME),
						   hParent, DlgProcDriverName, (LONG)pDriverParms);
    if (iRet == -1) {
        DEBUGMSG(ZONE_DRIVERNAME|ZONE_ERROR,
            (TEXT("GetDriverName: DialogBoxParam returned err %u\r\n"),
                              GetLastError()));
        return FALSE;
    }
    return iRet;
}
