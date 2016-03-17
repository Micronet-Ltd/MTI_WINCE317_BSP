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
/****************************************************************************
*
*    PROGRAM: transdlg.c
*
****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <cpl.h>

#include <commctrl.h>

#include <memory.h>
#include "resource.h"
#include "netuip.h"
#include "loc.h"
#include "dial.h"
#include <intsafe.h>

#ifdef DEBUG
	#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF
	#define SetWaitCursor()   { ASSERT(hcursor_wait_cursor_save == (HCURSOR)0xDEADBEEF); hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	#define ResetWaitCursor() { ASSERT(hcursor_wait_cursor_save != (HCURSOR)0xDEADBEEF); SetCursor(hcursor_wait_cursor_save); hcursor_wait_cursor_save = (HCURSOR)0xDEADBEEF; }
#else
	#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save = NULL
	#define SetWaitCursor()   { hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	#define ResetWaitCursor() { SetCursor(hcursor_wait_cursor_save);  hcursor_wait_cursor_save = NULL; }
#endif

#define CCHSIZEOF(x)	(sizeof(x) / sizeof(TCHAR))
#define LOADSZ(ids, psz)	(LoadString(v_hInst, ids, psz, CCHSIZEOF(psz)))

static const TCHAR gszDialHelpFilePatterns[] =	TEXT("ctpnl.htm#adjust_dialing_patterns");
static const TCHAR gszDialHelpFileLocations[] =	TEXT("ctpnl.htm#adjust_dialing_location_settings");
static LOCATION *lpLocationList;   // array of pointers to Locations
static BOOL		bEmptyName;					

int     cxScreen, cyScreen;
HWND hwndDial;
HWND	*v_pDialogWnd;

extern HINSTANCE	v_hInst;

WNDPROC oldEditCtlProc;        // saved window proc for edit control
int nNumLocations, nCurrLocation;


void UpdateDialDigitsTable(void);   // util.c
void FreeDialDigitsTable(void);     // util.c

// --------------------------------------------
// retriev location struct pointer from index
// --------------------------------------------
BOOL GetLocationPtr(int i, LOCEDIT **lpLocation)
{

	if (! lpLocationList || i < 0 || i >= nNumLocations)  {
		*lpLocation = NULL;
//        DPF2("GetLocationPtr, index %d not found (numLocations = %d)\r\n", i, nNumLocations);
		return FALSE;
		}

	*lpLocation = &lpLocationList[i].LocEdit;                
//    DPF2("GetLocationPtr, index %d = x%X\r\n", i, *lpLocation);
	return TRUE;
}	

// --------------------------------------------
// retriev location struct from index
// --------------------------------------------
#if 0
void GetLocation(int i, LOCEDIT *lpLocation)
{

	if (! lpLocationList || i < 0 || i >= nNumLocations)
		return;

	memcpy((void *)lpLocation, (const void *)&lpLocationList[i].LocEdit, sizeof(LOCEDIT));
}	
#endif
 // --------------------------------------------
// retriev index to first location in ComboBox
// --------------------------------------------
int GetFirstLocation(HWND hwndCB)
{		
int i,j,k;
LOCEDIT *lpLocation;

	j = ComboBox_GetCount(hwndCB);
	for (i = 0; i < j; i++)	 {
		k  = ComboBox_GetItemData(hwndCB, i);  
		GetLocationPtr(k, &lpLocation);	
		if (lpLocation && (lpLocation->dwStatus != CPL_ENTRY_DELETED))
			return k;
		}
	return 0;
}	
// --------------------------------------------
// enable ComboBox redraw 
// --------------------------------------------
void EnableCBRedraw(HWND hwndCB)
{
  SendMessage(hwndCB, WM_SETREDRAW, TRUE, 0 );
  InvalidateRect(hwndCB, NULL, TRUE);
  UpdateWindow(hwndCB);

//  SendMessage(GetWindow(hwndCB, GW_CHILD),WM_SETREDRAW, TRUE, 0 );	// same for the Edit control part of CB
}
// --------------------------------------------
// disable ComboBox redraw 
// --------------------------------------------
void DisableCBRedraw(HWND hwndCB)
{
  SendMessage(hwndCB, WM_SETREDRAW, FALSE, 0 );
//  SendMessage(GetWindow(hwndCB, GW_CHILD),WM_SETREDRAW, FALSE, 0 );	// same for the Edit control part of CB
}


/*--------------------------------------------------------------------------*\

   Function:   LocationFillList
   
   Purpose:    Will fill a Locations list or combo box with all the valid 
               entries and will set the focus to the current location
   
\*--------------------------------------------------------------------------*/
UINT CALLBACK  LocationFillList( HWND  hDlg, UINT  uControl,  UINT  fListBox ) {

UINT  uResult;
LONG  lResult=CB_ERR;
LPTSTR lpszCurrent;
LOCEDIT		*lpLocation;
int i;

//	DPF("In LocationFillList()r\n");
   if ((LPVOID)lpLocationList == NULL || hDlg == NULL)  {
      uResult = CPL_APP_ERROR;
      goto  done;
      }

   // make it so the dialog does not flash, and clear out all old entries 
   //SendDlgItemMessage( hDlg, uControl, WM_SETREDRAW, FALSE, 0 );
   DisableCBRedraw(GetDlgItem(hDlg,uControl)); 
	   
   SendDlgItemMessage( hDlg, uControl, fListBox ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0 );

   //--------------------------------------------------
   // set the values in in the calling dialogs controls
   //--------------------------------------------------
   lpszCurrent = NULL;

	for (i = 0; i < nNumLocations; i++ )  {
		GetLocationPtr(i, &lpLocation);	// retriev location struct from index
		if (lpLocation && lpLocation->dwStatus != CPL_ENTRY_DELETED) {
			lResult = SendDlgItemMessage( hDlg, uControl, fListBox ? LB_ADDSTRING : CB_ADDSTRING, 0, (LPARAM)lpLocation->szName );
			if (lResult < 0)
				break;
			lResult = SendDlgItemMessage( hDlg, uControl, fListBox ? LB_SETITEMDATA : CB_SETITEMDATA, lResult, i);
			if (lResult < 0)
				break;
			}
     	}

	if (lResult == CB_ERRSPACE || lResult == CB_ERR) {
		uResult = CPL_ERR_MEMORY;
		goto  done;
		}  // end if
			  
    // remember the current location
	GetLocationPtr(nCurrLocation, &lpLocation);	// retriev location struct from index
    if (lpLocation == NULL) {
        uResult = CPL_APP_ERROR;
        goto  done;
    }

	lResult = SendDlgItemMessage( hDlg, uControl, fListBox ? LB_FINDSTRING : CB_FINDSTRING, 0, (LPARAM)lpLocation->szName );
   	if ( lResult == CB_ERR )     {
      uResult = CPL_APP_ERROR;
      goto  done;
      }
                        
//   DPF2("Doing CB_SETCURSEL: CurSel=%d lpLocation->szName=%s \r\n", lResult,lpLocation->szName);
   SendDlgItemMessage( hDlg, uControl, fListBox ? LB_SETCURSEL : CB_SETCURSEL, (UINT)lResult, 0 );

   
   // force an update notification message repacking
   PostMessage( hDlg, WM_COMMAND, MAKELONG(uControl, (fListBox ? LBN_SELCHANGE : CBN_SELCHANGE)), (LPARAM)GetDlgItem(hDlg, uControl));
   uResult = CPL_SUCCESS;

done:
    // let the dialog draw again
    //SendDlgItemMessage( hDlg, uControl, WM_SETREDRAW, TRUE, 0 );
	EnableCBRedraw(GetDlgItem(hDlg,uControl)); 
    return uResult;

}  // end LocationFillList

/*--------------------------------------------------------------------------*\

   Function:   LocationFindInList
   
   Purpose:    Finds the location in the list of locations
               searches the entries from the cahced ini file
               then searches through the new entries
   
\*--------------------------------------------------------------------------*/
int LocationFindInList(LPTSTR lpszLocation) {
LOCEDIT *lpLocation;
int i;
      
 //---------------------------------------
 // find the name in the list of locations
 //---------------------------------------
	for (i = 0; i < nNumLocations; i++ )  {
		GetLocationPtr(i, &lpLocation);	// retriev location struct from index
		if (lpLocation && lpLocation->dwStatus != CPL_ENTRY_DELETED && wcsicmp( lpszLocation, lpLocation->szName) == 0 )
				return i;   // found it!!!
		}   
	 return -1;
}  // end LocationFindInList

/*--------------------------------------------------------------------------*\

   Function:   SetCancelCWCtls
   
   Purpose:    Read in the list from code space, and fill up the list box
   
\*--------------------------------------------------------------------------*/
UINT CALLBACK  SetCancelCWCtls(HWND hDlg,LPTSTR lpszUserDef ) 
{
static LPTSTR rgszCancelCWStrings[4] = {NULL,L"",L"", L""};
static TCHAR szTemp[CPL_MAX_STRING] = L"\0";
LONG  lListIndex;
LONG  lListIndexCur;
UINT  i, j;
LPTSTR   lpsz4;
HWND     hwndCB = GetDlgItem(hDlg, IDCC_DL_CALL_WAITING);
HWND	 hwndText = GetDlgItem(hDlg, IDC_STATIC_DIAL);

	if(rgszCancelCWStrings[0] == NULL) {
		rgszCancelCWStrings[0] = szTemp;
		for(i=0; i<3; rgszCancelCWStrings[i+1] = lstrlen(rgszCancelCWStrings[i]) + 1 + rgszCancelCWStrings[i], i++ ) {
			LoadString(v_hInst, IDS_CALL_WAITING_STR1+i, rgszCancelCWStrings[i], CPL_MAX_STRING - ( rgszCancelCWStrings[i] - szTemp ));
		}
	}
	
    DisableCBRedraw(hwndCB); 
    ComboBox_ResetContent(hwndCB);

    if (!IsDlgButtonChecked(hDlg, IDCK_DL_CALL_WAITING) ) {
        // We don't have call waiting. Disable dropdown list 
        *lpszUserDef = chNull;
        EnableWindow(hwndCB, FALSE);
		if (hwndText)
			EnableWindow(hwndText, FALSE);
		EnableCBRedraw(hwndCB); 
        return CPL_SUCCESS;
        }

	if (hwndText)
		EnableWindow(hwndText, TRUE);
    EnableWindow(hwndCB, TRUE);
    for (i=0; i < 3; i++)  {
        lListIndex = ComboBox_AddString(hwndCB, (LPTSTR)rgszCancelCWStrings[i]);
        ComboBox_SetItemData(hwndCB, lListIndex, i);
        }
    i = 3;

    lListIndexCur = SendDlgItemMessage(hDlg, IDCC_DL_CALL_WAITING, CB_FINDSTRING, 0, (LPARAM)(LPTSTR)lpszUserDef);
    if (lListIndexCur != CB_ERR)     		// if it already one of the 3 defaults call waiting string, add the null string
        								// Add szCancelCW3 (the null string), but set current sel. to lListIndex 
        lpsz4 = (LPTSTR) rgszCancelCWStrings[3];
    else   
        // Don't add the CallWaiting sentinel string (" ") to the listbox 
        lpsz4 = (!*lpszUserDef) ? (LPTSTR)rgszCancelCWStrings[3] : lpszUserDef;
   
    j = ComboBox_AddString(hwndCB, lpsz4);
    ComboBox_SetItemData(hwndCB, j, i);
    if (lListIndexCur == CB_ERR) {    		// if it already one of the 3 defaults call waiting string, add the null string
		// The user string is not one of the three defaults.  If it's "", then set focus to the first one (0).
		// Otherwise, set focus to the user string.
		if(*lpszUserDef) {
			lListIndexCur = j;
		} else {
			lListIndexCur = 0;
		}
	}

    ComboBox_SetCurSel(hwndCB, lListIndexCur);

	EnableCBRedraw(hwndCB); 
    InvalidateRect( GetDlgItem(hDlg, IDCC_DL_CALL_WAITING), NULL, TRUE );
    return CPL_SUCCESS;
}  // end SetCancelCWCtls

/*--------------------------------------------------------------------------*\
	Get the Call Waiting code from control to memory   
\*--------------------------------------------------------------------------*/
void GetCW(HWND hDlg, LPLOCEDIT lpEditLoc)
{
    HWND hwndCB;

    hwndCB = GetDlgItem(hDlg, IDCC_DL_CALL_WAITING);

    if (IsDlgButtonChecked(hDlg, IDCK_DL_CALL_WAITING)) {
        EnableWindow(hwndCB, TRUE);

        if (!GetWindowText(hwndCB, (LPTSTR)lpEditLoc->szCancelCW, MAX_CALL_WAIT_NUM+1)) {
            SetCancelCWCtls(hDlg, lpEditLoc->szCancelCW);
            GetWindowText(hwndCB, (LPTSTR)lpEditLoc->szCancelCW, MAX_CALL_WAIT_NUM+1);
        }
    } else {
        lpEditLoc->szCancelCW[0] = chNull;
    }
}

//------------------------------------------------------------------------	  
// Find if location Dialing Prefix valuse have been changed
//------------------------------------------------------------------------
BOOL CALLBACK    FPrefixChanged (LPLOCEDIT lpEditLoc, LPLOCEDIT lpOldEditLoc)
{

    if  (   lstrcmp(lpEditLoc->szLocPrefix,  lpOldEditLoc->szLocPrefix)
         || lstrcmp(lpEditLoc->szLDPrefix,   lpOldEditLoc->szLDPrefix)
         || lstrcmp(lpEditLoc->szIntlPrefix, lpOldEditLoc->szIntlPrefix)
		 )
        return TRUE;
    return FALSE;
}
//------------------------------------------------------------------------	  
// Find If location valuse have been changed
//------------------------------------------------------------------------
BOOL CALLBACK    FLocationChanged (LPLOCEDIT lpEditLoc, LPLOCEDIT lpOldEditLoc)
{

    if (   FPrefixChanged(lpEditLoc, lpOldEditLoc) 
		|| lstrcmp(lpEditLoc->szName, lpOldEditLoc->szName)
		|| lstrcmp(lpEditLoc->szCancelCW, lpOldEditLoc->szCancelCW)
		|| lstrcmp(lpEditLoc->szCntryRgnCode, lpOldEditLoc->szCntryRgnCode)
		|| lstrcmp(lpEditLoc->szAreaCode, lpOldEditLoc->szAreaCode)
		|| lpEditLoc->dwOptions != lpOldEditLoc->dwOptions
		 )
        return TRUE;
    return FALSE;
}

//------------------------------------------------------
// Get data from controls and update Location with it
//------------------------------------------------------
UINT UUpdateLocList(HWND hDlg, LPLOCEDIT lpEditLoc,  LPDWORD lpdw)
{
HWND hwndCB =  GetDlgItem(hDlg,IDCC_DL_CURR_LOCATION);
LOCEDIT  Location;

    *lpdw = CPL_ENTRY_NO_CHANGE;

	// save location before changes made
	memcpy((void *)&Location, (const void *)lpEditLoc, sizeof(LOCEDIT));
	GetCW(hDlg, lpEditLoc);

    GetDlgItemText(hDlg, IDCE_DL_CNTRYRGN_CODE, (LPTSTR)lpEditLoc->szCntryRgnCode, MAX_CNTRYRGN_CODE_NUM+1);
    GetDlgItemText(hDlg, IDCE_DL_AREA_CODE,    (LPTSTR)lpEditLoc->szAreaCode, MAX_AREA_CODE_NUM+1);
	if (IsDlgButtonChecked(hDlg, IDCR_PULSE))  
		lpEditLoc->dwOptions |= PULSEDIAL;
	else
		lpEditLoc->dwOptions &= ~PULSEDIAL;

    if (FLocationChanged(lpEditLoc, &Location) && lpEditLoc->dwStatus != CPL_ENTRY_ADDED)
        *lpdw  = lpEditLoc->dwStatus = CPL_ENTRY_MODIFIED;
    return CPL_SUCCESS;
}
/*--------------------------------------------------------------------------*\

   Function:   LocationReadList
   
   Purpose:    Read in the list from the ini file.  CardReadList must
			   be read before LocationReadList
   
\*--------------------------------------------------------------------------*/
UINT  CALLBACK LocationReadList(BOOL NotUsed)
{
UINT  uResult;
HANDLE   hIniList;
            
   nCurrLocation = 0;
   //
   // read in all the locations
   //
   hIniList = NULL;     // Ini function will allocate the block for us!
   uResult = RegReadLocations( &hIniList);
   if ( uResult != ERROR_SUCCESS )
      goto  LDone;

   /* lock down the pointer and store it */
   lpLocationList = (LPVOID)LocalLock( hIniList );
   MyHeapChk();

   if ((LPVOID)lpLocationList == NULL)   {
      uResult = CPL_ERR_MEMORY;
      goto  LDone;
      }
   uResult = CPL_SUCCESS;
//   DPF("Location info looks good...\r\n");
LDone: 
	if (uResult != CPL_SUCCESS)
		LocationClearList();      // clean up anything that might be left
    return (uResult);
}  /* end LocationReadList */


/*--------------------------------------------------------------------------*\

   Function:   AddOneLocation
   
   Purpose:    
               
   
\*--------------------------------------------------------------------------*/
UINT  CALLBACK AddOneLocation(void)
{
	UINT	uResult = CPL_ERR_MEMORY;
	HANDLE	lpMem	= NULL;

	/* lock down the pointer and store it */
	UINT uT;
	INT nT;
            
	LocalUnlock( lpLocationList );
	MyHeapChk();

	if (nNumLocations <= nNumLocations + 1 &&
		SUCCEEDED(UIntMult(nNumLocations + 1, sizeof(LOCATION), &uT)) &&
		SUCCEEDED(UIntToInt(uT, &nT)))
		lpMem = LocalReAlloc((HANDLE)lpLocationList, (nNumLocations + 1) * sizeof(LOCATION),  LMEM_MOVEABLE | LMEM_ZEROINIT);
	if (! lpMem) {
		//	  DPF("Error in AddOneLocation !!\r\n");
		uResult = CPL_ERR_MEMORY;
		goto  LDone;
	}
	MyHeapChk();

	lpLocationList = (LPVOID)LocalLock( lpMem );
	MyHeapChk();  

	if ((LPVOID)lpLocationList == NULL)   {
		//	  DPF("Error in AddOneLocation !!\r\n");
		uResult = CPL_ERR_MEMORY;
		goto  LDone;
	}
	nNumLocations++;

	uResult = CPL_SUCCESS;
LDone: 
	return (uResult);
}  /* end LocationReadList */

/*--------------------------------------------------------------------------*\

   Function:   LocationClearList
   
   Purpose:    Clears the list, frees all memory...
   
\*--------------------------------------------------------------------------*/
UINT  LocationClearList( VOID )
{
UINT  uResult;
HLOCAL  hMem;

   if ((LPVOID)lpLocationList == NULL)     {
      uResult = CPL_APP_ERROR;
      goto  done;
      }

   hMem = LocalHandle(lpLocationList);

   LocalUnlock( hMem );
   LocalFree( hMem );
   lpLocationList = NULL;
   uResult = CPL_SUCCESS;

done:
    return( uResult );
}  // end LocationClearList


/**********************************************************************************/
UINT CALLBACK  CleanupAll()
{
    /* Clean up lists */
   LocationClearList();
   return 0;
}
BOOL APIENTRY EditDialPatternDlg(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
static LOCEDIT *lpLocation;
LOCEDIT Location;
TCHAR	szTemp[ CPL_MAX_STRING ];

	// Verify that our temp string is long enough.
	ASSERT (CPL_MAX_STRING > MAX_LOCAL_NUM);
	ASSERT (CPL_MAX_STRING > MAX_LONG_DIST_NUM);
	ASSERT (CPL_MAX_STRING > MAX_INTL_NUM);

    switch (message)     {
		case WM_HELP:
			CreateProcess(TEXT("peghelp"), (LPTSTR)gszDialHelpFilePatterns, NULL,NULL,FALSE,0,NULL,NULL,NULL,NULL);
			break;
			
	    case WM_INITDIALOG:
//	TODO		CenterInWorkArea(hDlg);
			lpLocation = (LPLOCEDIT) lParam;

			SendDlgItemMessage( hDlg, IDCE_DL_LOCAL_CALLS, 		EM_LIMITTEXT, MAX_LOCAL_NUM,	0 );
			SendDlgItemMessage( hDlg, IDCE_DL_LONG_DIST_CALLS,	EM_LIMITTEXT, MAX_LONG_DIST_NUM,0 );
			SendDlgItemMessage( hDlg, IDCE_DL_INTL_CALLS,		EM_LIMITTEXT, MAX_INTL_NUM,		0 );

			SetDlgItemText(hDlg, IDCE_DL_LOCAL_CALLS, 		lpLocation->szLocPrefix );
			SetDlgItemText(hDlg, IDCE_DL_LONG_DIST_CALLS,	lpLocation->szLDPrefix );
			SetDlgItemText(hDlg, IDCE_DL_INTL_CALLS,		lpLocation->szIntlPrefix );

    	  	SetWindowLong(GetDlgItem(hDlg, IDCE_DL_LOCAL_CALLS), 	GWL_WNDPROC, (LONG) EditCtlProc);
    	  	SetWindowLong(GetDlgItem(hDlg, IDCE_DL_LONG_DIST_CALLS),GWL_WNDPROC, (LONG) EditCtlProc);
    	  	SetWindowLong(GetDlgItem(hDlg, IDCE_DL_INTL_CALLS),		GWL_WNDPROC, (LONG) EditCtlProc);

            if(GetFileAttributes(PEGHELP_EXE) != -1) {
                DWORD dwStyle = GetWindowLong (hDlg, GWL_EXSTYLE);
                SetWindowLong (hDlg, GWL_EXSTYLE, dwStyle | WS_EX_CONTEXTHELP);
            }

			// RAID #12968 fix. 
            // Make this window IME disabled
            ImmAssociateContext(GetDlgItem(hDlg, IDCE_DL_LOCAL_CALLS), (HIMC)NULL);
            ImmAssociateContext(GetDlgItem(hDlg, IDCE_DL_LONG_DIST_CALLS), (HIMC)NULL);
            ImmAssociateContext(GetDlgItem(hDlg, IDCE_DL_INTL_CALLS), (HIMC)NULL);

			SetFocus(GetDlgItem(hDlg, IDCE_DL_LOCAL_CALLS));
			SendDlgItemMessage(hDlg, IDCE_DL_LOCAL_CALLS, EM_SETSEL, 0L, -1);
			
			break;
	    case WM_COMMAND:      
			switch (LOWORD(wParam)) {
				case IDOK:
					memcpy((void *)&Location, (const void *)lpLocation,
						   sizeof(LOCEDIT));

					GetDlgItemText(hDlg, IDCE_DL_LOCAL_CALLS ,szTemp,
								   MAX_LOCAL_NUM+1);
					LCMapString (LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH, 
								 szTemp, _tcslen(szTemp)+1, 
								 lpLocation->szLocPrefix,
								 MAX_LOCAL_NUM+1);
					
					GetDlgItemText(hDlg, IDCE_DL_LONG_DIST_CALLS ,szTemp,
								   MAX_LONG_DIST_NUM+1);
					LCMapString (LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH,
								 szTemp, _tcslen(szTemp)+1, 
								 lpLocation->szLDPrefix, MAX_LONG_DIST_NUM+1);

					GetDlgItemText(hDlg, IDCE_DL_INTL_CALLS, szTemp,
								   MAX_INTL_NUM+1);
					LCMapString (LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH,
								 szTemp, _tcslen(szTemp)+1, 
								 lpLocation->szIntlPrefix, MAX_INTL_NUM+1);

				    if (FPrefixChanged(lpLocation, &Location) && lpLocation->dwStatus != CPL_ENTRY_ADDED)
				        lpLocation->dwStatus = CPL_ENTRY_MODIFIED;
				    EndDialog(hDlg, TRUE); 
					break;
				case IDCANCEL:
				    EndDialog(hDlg, FALSE); 
					break;
				default:
				    break;
				}
		}	
    return FALSE;
}


BOOL APIENTRY NewLocationDlg(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
static int index;
LOCEDIT *lpLocation, *lpLocation1;
TCHAR szText[MAX_LOCATION_NAME+1];
int	i, j;

    switch (message)     {
	    case WM_INITDIALOG:
//	TODO		CenterInWorkArea(hDlg);
			index = lParam;
			SendDlgItemMessage( hDlg, IDCE_NL_NEW_LOCATION,	EM_LIMITTEXT, MAX_LOCATION_NAME,0 );
			SetFocus(GetDlgItem(hDlg, IDCE_NL_NEW_LOCATION));
			break;

	    case WM_COMMAND:      
			switch (LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hDlg, IDCE_NL_NEW_LOCATION ,szText, MAX_LOCATION_NAME+1);

					// Trim the string.
					for (i=0; szText[i] && szText[i] == TEXT(' '); i++) {
						;
					}
					if (i) {
						for (j=0; szText[i]; i++,j++) {
							szText[j] = szText[i];
						}
						szText[j] = TEXT('\0');
					}
					for (i=_tcslen(szText)-1; (i > 0) && (szText[i] == TEXT(' ')); i--) {
						;
					}
					szText[i+1] = TEXT('\0');
					
					if (szText[0]) {

						if (LocationFindInList(szText) >= 0)  {
						    TCHAR szErrMsg[MAX_PATH];
						    LOADSZ(IDS_ERR_ALREADY_IN_LIST,szErrMsg);
						    MessageBox(hDlg, szErrMsg, NULL, MB_OK | MB_ICONEXCLAMATION);
							SetFocus(GetDlgItem(hDlg, IDCE_NL_NEW_LOCATION));
							SendDlgItemMessage(hDlg, IDCE_NL_NEW_LOCATION, EM_SETSEL, 0L, -1);
							break;
							}
						AddOneLocation();         
						GetLocationPtr(index, &lpLocation);						// get old location
						GetLocationPtr(nNumLocations - 1, &lpLocation1);		// get newly created location

						if (lpLocation && lpLocation1) {
							memcpy((void *)lpLocation1, (const void *)lpLocation, sizeof(LOCEDIT));
							lstrcpy(lpLocation1->szName,szText); 
							lpLocation1->dwStatus = CPL_ENTRY_ADDED;
							lpLocation1->dwPermLocationID = 0xffffffff;
						}

					    EndDialog(hDlg, TRUE); 
						break;
					}
					// fall through if empty string
				case IDCANCEL:
				    EndDialog(hDlg, FALSE); 
					break;
				default:
				    break;
				}
		}	
    return FALSE;
}


/*********************************************************************************************************/
/*********************************************************************************************************/
// if we just lost focus from the Location COmboBox and location name is empty return,
// set the error dialog and return FALSE
BOOL LegalLocationName(HWND hDlg) 
{
HWND hwndCB_Loc;
	
	hwndCB_Loc = GetDlgItem(hDlg, IDCC_DL_CURR_LOCATION);
	bEmptyName = FALSE;

	if ( GetFocus() == GetWindow(hwndCB_Loc, GW_CHILD))	{	// did we just lost focus in the CB edit control ?
//		DPF3("LegalLocationName: Lost Edit focus. focus=%x hwndCB=%x hwndCB/Edit=%x\r\n", GetFocus(), hwndCB_Loc, GetWindow(hwndCB_Loc, GW_CHILD));
		SendMessage(hDlg, WM_COMMAND,  MAKELONG(IDCC_DL_CURR_LOCATION,CBN_KILLFOCUS), 0); // it will dispaly the right dialog
		if (bEmptyName)
			return FALSE;
		}
	return TRUE;
}

/*********************************************************************************************************/
/*********************************************************************************************************/
BOOL APIENTRY DialDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
static LOCEDIT *lpLocation;
static HWND hwndCB_Loc;
TCHAR szText[CPL_MAX_STRING];
WPARAM wCmdId;
WPARAM wCmdCbn;
UINT  uResult=0;
DWORD dwRetID;
int i, j;
HWND hwnd;
static BOOL	bInKillFocus = FALSE;


    switch (message)     {
		case WM_HELP:
			CreateProcess(TEXT("peghelp"), (LPTSTR)gszDialHelpFileLocations, NULL,NULL,FALSE,0,NULL,NULL,NULL,NULL);
			break;

		case WM_GETICON:
		{
			LRESULT lRet = (LRESULT)LoadImage(v_hInst, MAKEINTRESOURCE(IDI_DIALINGICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)lRet);
        	return lRet;
		}
	    case WM_INITDIALOG:
		    hwndDial = hDlg;
			if (v_pDialogWnd) {
				*v_pDialogWnd = hDlg;
			}
			hwndCB_Loc = GetDlgItem(hDlg, IDCC_DL_CURR_LOCATION);

			// ComboBox_SetExtendedUI(hwndCB_Loc, TRUE);
			// SendDlgItemMessage(hDlg, IDCC_DL_CALL_WAITING, CB_SETEXTENDEDUI, TRUE, 0);

			//SendDlgItemMessage(hDlg, IDCC_DL_CURR_LOCATION, CB_SETDROPPEDWIDTH, 210,0);

			ComboBox_LimitText(hwndCB_Loc, MAX_LOCATION_NAME);
			ComboBox_LimitText(GetDlgItem(hDlg, IDCC_DL_CALL_WAITING), MAX_CALL_WAIT_NUM);
			SendDlgItemMessage( hDlg, IDCE_DL_AREA_CODE, 	EM_LIMITTEXT, MAX_AREA_CODE_NUM,	0);		  // enough digits ?
			SendDlgItemMessage( hDlg, IDCE_DL_CNTRYRGN_CODE, EM_LIMITTEXT, MAX_CNTRYRGN_CODE_NUM,	0);				   

            if(GetFileAttributes(PEGHELP_EXE) != -1) {
                DWORD dwStyle = GetWindowLong (hDlg, GWL_EXSTYLE);
                SetWindowLong (hDlg, GWL_EXSTYLE, dwStyle | WS_EX_CONTEXTHELP);
            }

		    if (LocationReadList(TRUE))	{
//	        	DPF("Error in LocationReadList \r\n");
	            CleanupAll();
	            return FALSE;
	    		}		
	         if (LocationFillList( hDlg, IDCC_DL_CURR_LOCATION, FALSE) != CPL_SUCCESS )	 {
//	            DPF("The location info in telephon.ini info is hosed. \r\n");
	            goto  LError;
	         	}
	 		 MyHeapChk();
			// contains the default calling loc name, set focus to it
			if ((i = LocationFindInList(UtilGetSelectedStr(hDlg, IDCC_DL_CURR_LOCATION, FALSE))) < 0  )  {
//				DPF(" The location info in registry is broken. \r\n");
	            goto  LError;
				}
			GetLocationPtr(i, &lpLocation);
            if (lpLocation == NULL) 
                goto  LError;

			//Make this window IME disabled
			hwnd = 	GetDlgItem(hDlg, IDCC_DL_CURR_LOCATION);
			ImmAssociateContext(hwnd, (HIMC)NULL);

			hwnd = 	GetDlgItem(hDlg, IDCC_DL_CALL_WAITING);
			// get the edit control of the combo box
			hwnd = GetWindow(hwnd, GW_CHILD);		

            // Make this (edit control of the combo box) window IME disabled
            ImmAssociateContext(hwnd, (HIMC)NULL);
    	  	oldEditCtlProc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditCtlProc);


			// setup the two numeric only edit controls to only accept pastes of pure numeric.
			// we will trap the WM_PASTE messsage in the subclass proc.
			// in these cases a failure to get the hwnd into the list will not be fatal so don't care about return
			hwnd = GetDlgItem(hDlg, IDCE_DL_AREA_CODE);
			if (hwnd)
			{
				(void) AddNumericOnlyEdit(hwnd);
	    	  	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditCtlProc);
			}

			hwnd = 	GetDlgItem(hDlg, IDCE_DL_CNTRYRGN_CODE);
			if (hwnd)
			{
				(void) AddNumericOnlyEdit(hwnd); 
	    	  	SetWindowLong(hwnd, GWL_WNDPROC, (LONG) EditCtlProc);
			}
		
LUpdateEntries:
	        /* Update entries in all the controls */
			uResult = ComboBox_FindString(hwndCB_Loc, -1, lpLocation->szName); 
	        if ( uResult == CB_ERR ) {
	            uResult = CPL_APP_ERROR;
	            goto  LError;
	            }
//		    DPF2("Doing ComboBox_SetCurSel: CurSel=%d lpLocation->szName=%s \r\n", uResult,lpLocation->szName);
			if (uResult != ComboBox_GetCurSel(hwndCB_Loc))	// prevent flashing
				ComboBox_SetCurSel(hwndCB_Loc, uResult); 

			SetDlgItemText(hDlg, IDCE_DL_AREA_CODE,		lpLocation->szAreaCode );
			SetDlgItemText(hDlg, IDCE_DL_CNTRYRGN_CODE,	lpLocation->szCntryRgnCode);
			SendDlgItemMessage(hDlg,
							   ((UINT)lpLocation->dwOptions & PULSEDIAL) ?
	                           	               IDCR_PULSE :
	                                           IDCR_TONE , 
	                            BM_SETCHECK, TRUE,0);
            SendDlgItemMessage(hDlg,                       // clear the other radio
                               !((UINT)lpLocation->dwOptions & PULSEDIAL) ?
                                               IDCR_PULSE :
                                               IDCR_TONE , 
                                BM_SETCHECK, FALSE, 0);

            CheckDlgButton(hDlg, IDCK_DL_CALL_WAITING,
                           FHasCallWaiting(lpLocation->szCancelCW));
            
            SetCancelCWCtls(hDlg, lpLocation->szCancelCW);

			i = ComboBox_GetCount(hwndCB_Loc);
	        EnableWindow(GetDlgItem(hDlg, IDCB_DL_REMOVE_LOCATION), i > MIN_LOCATION_ENTRIES);
	        EnableWindow(GetDlgItem(hDlg, IDCB_DL_NEW_LOCATION), i < MAX_LOCATION_ENTRIES);
LUpdateDialNum:
	        SetDlgItemText(hDlg, IDC_STATIC_LOCAL, lpLocation->szLocPrefix);  
	        SetDlgItemText(hDlg, IDC_STATIC_LONG, lpLocation->szLDPrefix);  
	        SetDlgItemText(hDlg, IDC_STATIC_INTL, lpLocation->szIntlPrefix);  
			return TRUE;

	    case WM_COMMAND:      
            wCmdId =  LOWORD(wParam);
            wCmdCbn = HIWORD(wParam);
			if (! hwndDial)
				break;
            switch (wCmdId)            {
				case IDCB_DL_CHANGE_PATERN:
					if (!LegalLocationName(hDlg))
						return FALSE;
                    DialogBoxParam(v_hInst, 
                                   MAKEINTRESOURCE ((cxScreen < 480) ? 
                                                    IDD_EDIT_PATTERN_N : IDD_EDIT_PATTERN_W),
                                   hDlg, (DLGPROC)EditDialPatternDlg, (LPARAM)lpLocation);
                    SetFocus(GetDlgItem(hDlg,IDCB_DL_CHANGE_PATERN)); 
                    goto LUpdateDialNum;

                case IDCC_DL_CURR_LOCATION:   
					if (wCmdCbn == CBN_KILLFOCUS)  {
CB_LostFocus:
						if (bInKillFocus)		// do not allow nested processing
							return TRUE;
						bInKillFocus = TRUE;
#ifdef DEBUG_MSGS
						// take out 
                      	GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
						j  = ComboBox_GetCurSel(hwndCB_Loc);   
						i  = ComboBox_GetItemData(hwndCB_Loc, j);   
//						DPF3("CBN_KILLFOCUS: CurSel=%d ItemData=%d szText=%s \r\n", j,i,szText);
						// end out
#endif
                      	GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
 
						if (lstrcmp(lpLocation->szName, szText)) {		 // if name was edited
						    if (! szText[0])     		// can not take empty names
						        i = CPL_WRN_LOCATION_NAME_REQUIRED;
							else	{
	                   			i = LocationFindInList(szText);   // make sure name is not already somewher else in list
	                   			if ( i >= 0 && i != nCurrLocation)   // make sure name is not already somewher else in list
									i = CPL_ERR_ALREADY_IN_LIST;
								else  {
									if(lpLocation->dwStatus != CPL_ENTRY_ADDED) {
						       			lpLocation->dwStatus = CPL_ENTRY_MODIFIED;
									}
			                      	StringCchCopyN(lpLocation->szName, ARRAYSIZE(lpLocation->szName), szText, ARRAYSIZE(lpLocation->szName)-1);		// OK to modify entry
									bInKillFocus = FALSE;
//									DPF("CBN_KILLFOCUS: Entry Mofified \r\n");
						            goto LUpdateListEntries;
									}
								}

	                     	//SetWindowText( hwndCB_Loc, lpLocation->szName);	// restore original CB name
							j = ComboBox_FindString(hwndCB_Loc, -1, lpLocation->szName); 	// restore original location
							if ( j != CB_ERR ) 
								ComboBox_SetCurSel(hwndCB_Loc, j); 
                         	SetFocus(hwndCB_Loc);
							bInKillFocus = FALSE;
							bEmptyName = TRUE;
							return FALSE;
							}
						bInKillFocus = FALSE;
						return TRUE;
						}
					else  
                    if (wCmdCbn == CBN_SELCHANGE)     {

#ifdef DEBUG_MSGS		// take out 
                      	GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
						j  = ComboBox_GetCurSel(hwndCB_Loc);   
						i  = ComboBox_GetItemData(hwndCB_Loc, j);  
						if (wCmdCbn == CBN_SELCHANGE) {
//							DPF3("CBN_SELCHANGE: CurSel=%d ItemData=%d ComboBox Text=%s \r\n", j,i,szText);
							}
						else {
//							DPF3("CBN_SELENDOK: CurSel=%d ItemData=%d ComboBox Text=%s \r\n", j,i,szText);
						}
#endif					// end out 

						// we do that becuase the CBN_SELCHANGE is send BEFORE the edit box is changed
						// and w eneed it after. This way we will get the  CBN_SELENDOK after the text changed
						PostMessage(hDlg, WM_COMMAND,  MAKELONG(IDCC_DL_CURR_LOCATION,CBN_SELENDOK), 0); // it will dispaly the right dialog
						}
					else
                    if (wCmdCbn == CBN_SELENDOK)     {
#ifdef DEBUG_MSGS		// take out 
                      	GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
						j  = ComboBox_GetCurSel(hwndCB_Loc);   
						i  = ComboBox_GetItemData(hwndCB_Loc, j);  
//						DPF3("CBN_SELENDOK: CurSel=%d ItemData=%d ComboBox Text=%s \r\n", j,i,szText);
#endif					// end out 

						j  = ComboBox_GetCurSel(hwndCB_Loc); 
						if (j == CB_ERR)  {
	                      	GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
 							if (!lstrcmp(lpLocation->szName, szText)) {		 // if name was not edited, do nothing
								j = ComboBox_FindString(hwndCB_Loc, -1, szText);   // make sure thats the one selected also
								if ( j != CB_ERR ) 
									ComboBox_SetCurSel(hwndCB_Loc, j); 
								break;
								}
	                     	goto CB_LostFocus;	 // a new name was edited in place while CB is dropped
							}
						i = ComboBox_GetItemData(hwndCB_Loc, j);   
						if (i == nCurrLocation)	   {
                      		GetWindowText( hwndCB_Loc, szText,  CPL_MAX_STRING);
							j = ComboBox_FindString(hwndCB_Loc, -1, szText);   // make sure thats the one selected also
//							DPF3("  CBN_SELENDOK: No Change ItemData=%d nCurrLocation=%d Find=%d\r\n", i,nCurrLocation, j);
							if ( j != CB_ERR ) 
								ComboBox_SetCurSel(hwndCB_Loc, j); 
							break;		// no change
						}
						nCurrLocation  = i;   
                     	//goto LUpdateListEntries;
						goto LUpdateListEntries1;
                        }
					break;
            	case IDCC_DL_CALL_WAITING: 
                    if (wCmdCbn == CBN_EDITCHANGE) {
              			goto LUpdateDialNum;
						}
					else						
					if (wCmdCbn  == CBN_SELENDOK || wCmdCbn == CBN_SELCHANGE || wCmdCbn == CBN_KILLFOCUS)  {
					  	// Check if user typed in a new string. If so, update	 the list box entries.
						GetDlgItemText( hDlg, IDCC_DL_CALL_WAITING, szText, CPL_MAX_STRING );

//						DPF2("In IDCC_DL_CALL_WAITING: CBN_SELENDOK=%d  szText=%s \r\n", wCmdCbn == CBN_SELENDOK,szText);

						if (szText[0] != chNull)   {
                    		uResult = SendDlgItemMessage(hDlg, IDCC_DL_CALL_WAITING, CB_FINDSTRING, 0, (LPARAM)(LPVOID)szText);
                    		if (uResult == CB_ERR)          	// if not already in list
                    			SetCancelCWCtls(hDlg, szText);
                  			}
						if (wCmdCbn == CBN_KILLFOCUS)
							GetCW(hDlg, lpLocation);            			
              			goto LUpdateDialNum;
               			}
               		break;
	            case IDCK_DL_CALL_WAITING:
					UUpdateLocList(hDlg, lpLocation, &dwRetID);
                    SetCancelCWCtls(hDlg,  lpLocation->szCancelCW);
                    goto LUpdateDialNum;

			    case IDCE_DL_AREA_CODE:          
                case IDCE_DL_CNTRYRGN_CODE:    
                    if (wCmdCbn != EN_KILLFOCUS)     
						break;
				case IDCR_PULSE:
				case IDCR_TONE:
					UUpdateLocList(hDlg, lpLocation, &dwRetID);
					break;

				case IDCB_DL_REMOVE_LOCATION:    
					
					if (ComboBox_GetCount(hwndCB_Loc) <= 1)
						break;							// just to be safe...

					if (!LegalLocationName(hDlg))
						return FALSE;
					SetFocus(GetDlgItem(hDlg, IDCB_DL_REMOVE_LOCATION));	// we must remove focus from IDCC_DL_CURR_LOCATION, since we process CBN_KILLFOCUS on it

					if (UtilMsgBox(hDlg, IDS_LOCATION_REMOVE_OR_STUPID, IDS_REMOVE_TITLE, MB_YESNO | MB_DEFBUTTON2) == IDYES )	                        {
						lpLocation->dwStatus = CPL_ENTRY_DELETED;   
						SetFocus(GetDlgItem(hDlg, IDCC_DL_CURR_LOCATION));
						SendMessage(GetDlgItem(hDlg, IDCB_DL_REMOVE_LOCATION), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE);	// remove the dark DEfault border)
						nCurrLocation = GetFirstLocation(hwndCB_Loc);		// set focus to first location
						//nNumLocations--;  do not decreament !, it must count TOTAL allocated locations 
	                    goto LUpdateListEntries;
	                    }
	                break;

	            case IDCB_DL_NEW_LOCATION:  
					if (!LegalLocationName(hDlg))
						return FALSE;
					SetFocus(GetDlgItem(hDlg, IDCB_DL_NEW_LOCATION));	// we must remove focus from IDCC_DL_CURR_LOCATION, since we process CBN_KILLFOCUS on it
					j  = ComboBox_GetCurSel(hwndCB_Loc);   
					i  = ComboBox_GetItemData(hwndCB_Loc, j);   
                    i = DialogBoxParam(v_hInst, 
                                       MAKEINTRESOURCE ((cxScreen < 480) ? IDD_NEW_LOCATION_N : 
                                            IDD_NEW_LOCATION_W),
                                       hDlg, (DLGPROC)NewLocationDlg, (LPARAM)i);								

					if (i) {
						SetFocus(GetDlgItem(hDlg, IDCE_DL_AREA_CODE));
						PostMessage(GetDlgItem(hDlg, IDCE_DL_AREA_CODE), EM_SETSEL, 0L, -1);			
						SendMessage(GetDlgItem(hDlg, IDCB_DL_NEW_LOCATION), BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE);	// remove the dark DEfault border)
						nCurrLocation = nNumLocations - 1;		// set focus to the new created location
						}
					else
						goto LUpdateListEntries1;

LUpdateListEntries:
                   	if ((uResult = LocationFillList(hDlg, IDCC_DL_CURR_LOCATION, FALSE )) != CPL_SUCCESS )
                       	goto  LError;
LUpdateListEntries1:
					GetLocationPtr(nCurrLocation, &lpLocation);  // need to do that since memory might have been moved/re-allocated
					if (lpLocation == NULL) 
						goto LError;
           			goto LUpdateEntries;

				case IDOK :
					RETAILMSG (1, (TEXT("Got IDOK\r\n")));
					if (!LegalLocationName(hDlg))
						return FALSE;

					UUpdateLocList(hDlg, lpLocation, &dwRetID);
					{
						DECLAREWAITCURSOR;
						SetWaitCursor();
                        RegSaveDialSettings();
						ResetWaitCursor();
					}
					EndDialog (hDlg, TRUE);
					break;
					
				case IDCANCEL :
					RETAILMSG (1, (TEXT("Got IDCANCEL\r\n")));
					EndDialog (hDlg, FALSE);
					break;
					
				case IDOK_EXIT:
//					DPF("DialDlgProc IDOK\r\n");

					if (!LegalLocationName(hDlg))
						return FALSE;

					UUpdateLocList(hDlg, lpLocation, &dwRetID);
					{
						DECLAREWAITCURSOR;
						SetWaitCursor();
                        RegSaveDialSettings();
                        //nCurrLocation = 0;				// set to 1st location so will not have to save again
                        //LocationFillList(hDlg, IDCC_DL_CURR_LOCATION, FALSE);
						ResetWaitCursor();
					}
					return TRUE;

				case IDC_TAB: 		// tab changed
				case IDM_EXIT:
exit:
//					DPF("Destroying Dialing Window\r\n");
	            	CleanupAll();
					
                    hwndDial = NULL;
					DestroyWindow(hDlg);
					return TRUE;
		    	}
			break;
        
        case WM_DESTROY:
			hwnd = GetDlgItem(hDlg, IDCE_DL_AREA_CODE);
			if (hwnd)
			{
				RemoveNumericOnlyEdit(hwnd);
			}

			hwnd = 	GetDlgItem(hDlg, IDCE_DL_CNTRYRGN_CODE);
			if (hwnd)
			{
				RemoveNumericOnlyEdit(hwnd);
			}

			CleanupAll();
			break;
	    
        case WM_CLOSE:      
			goto exit;
		}
    return (FALSE);

LError:
//    UtilErrorRpt( hDlg, uResult );
    return (uResult);
}

void
LineTranslateDialog( HWND hWndOwner, HWND *pDialogWnd)
{
    UpdateDialDigitsTable();

    cxScreen = GetSystemMetrics(SM_CXSCREEN);
	// NOTE: as soon as there is a standard way to get screen height
	// minus taskbar this should change
    cyScreen = GetSystemMetrics(SM_CYSCREEN) - 26;

	v_pDialogWnd = pDialogWnd;
    
    // OK, bring up the translate dialog.
    DialogBox (v_hInst, 
               MAKEINTRESOURCE ((cxScreen < 480) ? DIAL_DLG_POPUP_N : DIAL_DLG_POPUP_W), 
               hWndOwner, DialDlgProc);
    hwndDial = NULL;
    
    FreeDialDigitsTable();
}

