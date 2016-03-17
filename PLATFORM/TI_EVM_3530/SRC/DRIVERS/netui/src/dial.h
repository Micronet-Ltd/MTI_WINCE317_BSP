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
// prototypes
BOOL    UtilErrorRpt( HWND   hWnd,  UINT   wErrorId );
LPTSTR  UtilGetSelectedStr( HWND   hDlg, UINT  uControl,  BOOL fListBox);
LPTSTR  UtilGetStr( LPTSTR  lpszNewString,  UINT   wStrResId,  UINT   wMaxLeng);
UINT    UtilMsgBox( HWND  hDlg, UINT  wMsgID,  UINT  wTitleID, UINT  wFlags );
BOOL IsLegalDialChar(TCHAR c);
LRESULT CALLBACK EditCtlProc(HWND hwndDialog,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK CBCtlProc(HWND hwndDialog,UINT message,WPARAM wParam,LPARAM lParam);
BOOL    GetLocationPtr(int i, LOCEDIT **lpLocation);
UINT    RegReadLocations(LPHANDLE   lphMemBlock);
HRESULT RegSaveDialSettings(void);
BOOL IsNumericOnlyEdit(const HWND hwnd);
BOOL AddNumericOnlyEdit(const HWND hwnd);
VOID RemoveNumericOnlyEdit(const HWND hwnd);
#ifdef TARGET_NT
int MyHeapChk(void);
#else
#define  MyHeapChk()
#endif


// defines
#define FHasCallWaiting(lpszCancelCW)     (lpszCancelCW[0] != chNull)// if lpszCancelCW == L"" then it means that we DON'T have call waiting 


#define  MAX_REG_STRING	  				512
#define  PULSEDIAL						0x1
#define  MIN_LOCATION_ENTRIES    		1
#define  MAX_LOCATION_ENTRIES    		16

#define chNull  '\0'

//-------
// Errors
//-------
#define  CPL_SUCCESS                 0
#define  CPL_FAILURE                 2
#define  CPL_IGNORE                  3        // error that is just ignored
#define  CPL_APP_ERROR               100
#define  CPL_ERR_MEMORY              101
#define  CPL_ERR_DIALOG_BOX          102
#define  CPL_ERR_LOAD_STRING         103
#define  CPL_ERR_ALREADY_IN_LIST     105
#define  CPL_ERR_INVALID_REGISTRY	 112	  
#define  CPL_WRN_LOCATION_NAME_REQUIRED  207

//----------
// Constants
//----------

#define  CPL_ENTRY_NO_CHANGE        0
#define  CPL_ENTRY_ADDED            1
#define  CPL_ENTRY_MODIFIED         2
#define  CPL_ENTRY_DELETED          3

