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
*    PROGRAM: UTIL.c
*
****************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "loc.h"
#include "dial.h"
#include "netuip.h"

extern HINSTANCE v_hInst;
extern WNDPROC oldEditCtlProc,oldCBCtlProc;

static TCHAR * g_DialDigitsTable;
static const TCHAR DialDigitsTable[]   = {'A','B','C','D','E','F','G','e','f','g','*','#','T','P',',','!','W','@','$','?','+',(TCHAR)-1}; //'0'-'9'
static const TCHAR CWDigitsTable[]     = {'A','B','C','D','*','#','T','P',',','!','W','@','$','?',(TCHAR)-1};   //and '0'-'9'
static const TCHAR SimpleDigitsTable[] = {(TCHAR)-1};		// and '0'-'9'

static struct tagIdToTable {
	int 	CtrlId;
	LPCTSTR DigitTable;
	}  IdToTable[]= {
					{IDCC_DL_CALL_WAITING, 		CWDigitsTable},
					{IDCE_DL_AREA_CODE,    		SimpleDigitsTable},
					{IDCE_DL_CNTRYRGN_CODE,		SimpleDigitsTable},
					{IDCE_DL_LOCAL_CALLS,  		DialDigitsTable},
					{IDCE_DL_LONG_DIST_CALLS,   DialDigitsTable},
					{IDCE_DL_INTL_CALLS,  		DialDigitsTable},
					};


#define MAX_EDIT_ONLY 5
static HWND G_hwEditOnlyControls[MAX_EDIT_ONLY] = {NULL, NULL, NULL, NULL, NULL};


/*--------------------------------------------------------------------------*\

   Function:   IsNumericOnlyEdit
   
   Purpose: Check to see if this is in our list of numeric only edits
   
\*--------------------------------------------------------------------------*/
BOOL IsNumericOnlyEdit(const HWND hwnd)
{
	int i = 0;
	for (i=0; i < MAX_EDIT_ONLY; i++)
	{
		if (hwnd == G_hwEditOnlyControls[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*--------------------------------------------------------------------------*\

   Function:   AddNumericOnlyEdit
   
   Purpose: Adds a HWND to our list of numeric edits

	NOTE: This is a fixed list for sake of simplicity.  It will be rare
			for anyone to have more than 5 numeric edits on a single dialog
			but if they did it would be simple enough to increase the array
			size.
   
\*--------------------------------------------------------------------------*/
BOOL AddNumericOnlyEdit(const HWND hwnd)
{
	int i = 0;
	// find an empty slot for this entry and store it
	for (i=0; i < MAX_EDIT_ONLY; i++)
	{
		if (NULL == G_hwEditOnlyControls[i])
		{
			G_hwEditOnlyControls[i] = hwnd;
			return TRUE;
		}
	}

	ASSERT(!L"AddNumericOnlyEdit too many items added");

	return FALSE;
}

/*--------------------------------------------------------------------------*\

   Function:   RemoveNumericOnlyEdit
   
   Purpose: Remove a HWND from our list
   
\*--------------------------------------------------------------------------*/
VOID RemoveNumericOnlyEdit(const HWND hwnd)
{
	int i = 0;

	for (i=0; i < MAX_EDIT_ONLY; i++)
	{
		if (hwnd == G_hwEditOnlyControls[i])
		{
			G_hwEditOnlyControls[i] = NULL;
			return;
		}
	}
}





/*--------------------------------------------------------------------------*\

   Function:   UtilGetStr
   
   Purpose: Load a resource string
   
\*--------------------------------------------------------------------------*/
LPTSTR UtilGetStr( LPTSTR  lpszNewString,  UINT   wStrResId,  UINT   wMaxLeng ) {
static TCHAR szUtilString[CPL_MAX_STRING];   // function global
   
   if ( !LoadString( v_hInst, wStrResId, 
                     lpszNewString ? lpszNewString : szUtilString,
                     wMaxLeng ? wMaxLeng : CPL_MAX_STRING - 1 ))
      {
      UtilErrorRpt( NULL, CPL_ERR_LOAD_STRING );

      if ( lpszNewString )
         lpszNewString[0] = 0;
      return( NULL );
      }  // end if                 
   
   return( lpszNewString ? lpszNewString : szUtilString );
}  // end UtilGetString

/*--------------------------------------------------------------------------*\

   Function:   UtilMsgBox
   
   Purpose:    Generic private message box routine
   
\*--------------------------------------------------------------------------*/
UINT UtilMsgBox( HWND  hDlg, UINT  wMsgID,  UINT  wTitleID, UINT  wFlags ) 
{
UINT   uResult = 0;
TCHAR  szTitle[CPL_MAX_STRING];

   if ( hDlg == NULL )
      hDlg = GetActiveWindow();
      
   if (!((wFlags & MB_ICONQUESTION) || (wFlags & MB_ICONSTOP) || (wFlags & MB_ICONASTERISK)))
      wFlags |= MB_ICONEXCLAMATION;
      
   //MessageBeep( MB_ICONASTERISK );  // done by  MessageBox...
   uResult = (UINT)MessageBox( hDlg, UtilGetStr( NULL, wMsgID, 0 ), UtilGetStr( (LPTSTR)szTitle, wTitleID, sizeof(szTitle)/sizeof(TCHAR) ), wFlags );
   
    return( uResult  );
}  // end UtilMsgBox

/*--------------------------------------------------------------------------*\

   Function:   UtilErrorRpt
   
   Purpose:    Complain about an error found in the app.  Return TRUE
               if it's a bad error and exit out of dlgs, if FALSE can stay
               in the dialog
   
\*--------------------------------------------------------------------------*/
BOOL UtilErrorRpt( HWND   hDlg,  UINT   wErrorId )  {
UINT  wStrID;
UINT  wTitleID = IDS_WRN_TITLE_WARNING; // default to "Warning"
BOOL  fClose;
   
   fClose = FALSE;      // default is to not close calling dialog
   
   switch ( wErrorId )     {
      case  CPL_ERR_MEMORY:    
         fClose = TRUE;
         wStrID = IDS_ERR_MEMORY;
         break;
      case  CPL_ERR_ALREADY_IN_LIST:
         wStrID = IDS_ERR_ALREADY_IN_LIST;
         wTitleID = IDS_WRN_TITLE_NAMEINUSE;
         break;
      case CPL_ERR_INVALID_REGISTRY:
         fClose = TRUE;
         wStrID = IDS_ERR_INVALID_INI;
         
         UtilMsgBox( hDlg, wStrID, wTitleID, MB_YESNO | MB_DEFBUTTON2 );
         goto  LDone;         // goto exit

      case  CPL_IGNORE:         
         fClose = TRUE;
         goto  LDone;         // goto exit
         
      case  CPL_WRN_LOCATION_NAME_REQUIRED:         
         wStrID = IDS_WRN_LOCATION_NAME_REQUIRED;
         wTitleID = IDS_WRN_TITLE_REQUIRED;
         break;
      default:
         fClose = TRUE;
         wStrID = IDS_ERR_APPLICATION;
         break;
      }  // end case

   UtilMsgBox( hDlg, wStrID, wTitleID, MB_OK );

LDone:
    return( fClose );
}  // end UtilErrorRpt


/*--------------------------------------------------------------------------*\

   Function:   UtilGetSelectedStr
   
   Purpose:    Returns a valid pointer to a selected string in a list box
 			   If no string is selcted and it is a COmboBox return the Edit text	  
\*--------------------------------------------------------------------------*/
LPTSTR UtilGetSelectedStr( HWND   hDlg, UINT  uControl,  BOOL fListBox)   
{  // optional
UINT  uSize;
LONG  lIndex;
UINT  uResult;
static 	TCHAR gszUtilString[CPL_MAX_STRING];     // fucntion global
      
   // find the string
   //----------------                    
   lIndex = SendDlgItemMessage( hDlg, uControl, fListBox ? LB_GETCURSEL : CB_GETCURSEL, 0, 0 );
   if ( lIndex == LB_ERR )     {
   	  if (! fListBox) {
	  	 SendDlgItemMessage(hDlg, uControl, WM_GETTEXT, CPL_MAX_STRING,  (LPARAM)gszUtilString);
		 return( gszUtilString );
		 }
      uResult = CPL_APP_ERROR;
      goto  error;
      }
      
   // get some memory for this guy
   //-----------------------------
   uSize = (UINT)SendDlgItemMessage( hDlg, uControl, fListBox ? LB_GETTEXTLEN : CB_GETLBTEXTLEN, (UINT)lIndex, 0 ) + 1;
   if ((uSize + 1) >= sizeof(gszUtilString))  {
      uResult = CPL_APP_ERROR;
      goto  error;
      }

   SendDlgItemMessage( hDlg, uControl, fListBox ? LB_GETTEXT : CB_GETLBTEXT, (UINT)lIndex, (LPARAM)(LPTSTR)gszUtilString );
   
   return( gszUtilString );
  
error:
    UtilErrorRpt( hDlg, uResult );
    return( NULL );
}  // end UtilGetSelectedStr


LPCTSTR GetTable(HWND hwnd)
{
int i,CtlId;

	CtlId = GetWindowLong(hwnd, GWL_ID);
	for (i = 0; i < ARRAYSIZE(IdToTable); i++)
		if (IdToTable[i].CtrlId == CtlId)
			return IdToTable[i].DigitTable;

	return NULL;
}


/*************************************************************************************************/
/* This is the "WinProc" for our subclassed Edit controls.                                       */
/*************************************************************************************************/
LRESULT CALLBACK EditCtlProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	LPCTSTR lpTable, lpTable1; 
	TCHAR   c; 
#ifdef FAREAST
	TCHAR ch, chHW;
#endif // FAREAST
	HKL     hkl = GetKeyboardLayout(0);
	
	ASSERT(oldEditCtlProc);

	switch (message) {
	case WM_PASTE :
		{
			BOOL bPassThrough = TRUE;

			if (IsNumericOnlyEdit(hwnd))
			{
				// only going to accept pastes that have pure numeric data in them

				HANDLE hMem;

				if(OpenClipboard(hwnd))
				{
					hMem = GetClipboardData(CF_UNICODETEXT);
					if(hMem)
					{
						TCHAR *pText = (TCHAR *)LocalLock(hMem); // can't fail on CE
						int iLen = _tcslen(pText);
						int iPos = 0;

						for (iPos = 0; (iPos < iLen) && bPassThrough; iPos++)
						{
							if (!_istdigit(pText[iPos]))
							{
								// bad char
								bPassThrough = FALSE;

							}
						}

						LocalUnlock(hMem);
					}
					CloseClipboard();
				}
			}

			if (bPassThrough)
			{
				return CallWindowProc(oldEditCtlProc,hwnd, message, wParam, lParam);
			}else{
				MessageBeep(MB_OK);
				return FALSE;
			}
		}break;
		
		
	case WM_IME_COMPOSITION:
		if( ImmIsIME(hkl ) && 
			LOWORD(hkl ) == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT))
		{
			HIMC himc = ImmGetContext(hwnd);
			if (himc)
			{
				TCHAR szTempStr[4];
				if (0<ImmGetCompositionString(himc,GCS_COMPSTR, szTempStr, 4))
				{
					DWORD fdwConversion;
					DWORD fdwSentence;
					
					ImmNotifyIME(himc,NI_COMPOSITIONSTR,CPS_CANCEL,0);
					ImmGetConversionStatus(himc, &fdwConversion, &fdwSentence);
					fdwConversion&=(~IME_CMODE_NATIVE);
					ImmSetConversionStatus(himc,fdwConversion, fdwSentence);
				}
				ImmReleaseContext(hwnd,himc);
				return CallWindowProc(oldEditCtlProc,hwnd, message, wParam, lParam);
			} 
		}
		break;
		
	case WM_CHAR:
#ifdef FAREAST // Convert full-width numbers to half width
		ch = (TCHAR)wParam;
		LCMapString(LOCALE_USER_DEFAULT, LCMAP_HALFWIDTH,
			&ch, 1, &chHW, 1);
		wParam = (WPARAM)chHW;
#endif // FAREAST
		// This character is not ASCII. If your country/region needs specific characters. You
		// have to change this. Otherwise, we abandon this character.
		if (wParam >= 0x80 )  {  // service on ilegal chars
			MessageBeep(MB_OK);
			return TRUE;
		}
		if (wParam < VK_SPACE || wParam > 0x7e )  {  // service on legal chars
			//DPF("None ascii char. ignore \r\n ");
			break;
		}
		
		if ((lpTable = GetTable(hwnd)) == NULL) {
			//				DPF("Can not find table, trying parent \r\n"); 
			if ((lpTable = GetTable(GetParent(hwnd))) == NULL) {
				//DPF("Can not find table,  \r\n"); 
				break;
			}
		}			   		
		lpTable1 = lpTable;
		if (wParam >= '0' && wParam <= '9')
			goto found;
		while ((c = *lpTable++) > 0) {
			if ((TCHAR)wParam == c)
				goto found;
		}
		// now try w/ upper case
		if (iswlower((TCHAR)wParam)) {
			wParam = (LPARAM)towupper((TCHAR)wParam);			
			while ((c = *lpTable1++) > 0) {
				if ((TCHAR)wParam == c)
					goto found;
			}
		}
		//DPF1("Char '%c' not in table\r\n", wParam);
		MessageBeep(MB_OK);
		return TRUE;
found:
		//DPF1("Found char '%c' in table\r\n", wParam);
		break;
	default:
		break;
	}                                                                                   // of switch
	return CallWindowProc(oldEditCtlProc, hwnd, message, wParam, lParam);
}                                                                          


DWORD RegReadDialDigitsTable(TCHAR * lpDialDigitsTable);    // reg.c

//
// Read updated DialDigitsTable from registry and discard previous
//
void UpdateDialDigitsTable(void)
{
    TCHAR * lpOldDialDigitsTable;
    TCHAR * lpNewDialDigitsTable;
    DWORD i;

    i = RegReadDialDigitsTable(NULL);
    if (i == 0) {
        return;
    }

    lpNewDialDigitsTable = LocalAlloc(LPTR, i + sizeof(TCHAR));
    if (lpNewDialDigitsTable == NULL) {
        return;
    }

    i = RegReadDialDigitsTable(lpNewDialDigitsTable)/sizeof(TCHAR);
    lpNewDialDigitsTable[i] = (TCHAR)-1;    // properly terminate the new digit table


    if (g_DialDigitsTable == NULL) {
        lpOldDialDigitsTable = (TCHAR *)DialDigitsTable;
    } else {
        lpOldDialDigitsTable = g_DialDigitsTable;
        LocalFree(lpOldDialDigitsTable);
    }

    g_DialDigitsTable = lpNewDialDigitsTable;

    //
    // Update the digit tables that apply
    //
	for (i = 0; i < ARRAYSIZE(IdToTable); i++) {
        if (IdToTable[i].DigitTable == lpOldDialDigitsTable) {
            IdToTable[i].DigitTable = lpNewDialDigitsTable;
        }
    }
}


//
// Discard memory used by dial digits table read from registry
//
void FreeDialDigitsTable(void)
{
    if (g_DialDigitsTable) {
        LocalFree(g_DialDigitsTable);
        g_DialDigitsTable = NULL;
    }
}
