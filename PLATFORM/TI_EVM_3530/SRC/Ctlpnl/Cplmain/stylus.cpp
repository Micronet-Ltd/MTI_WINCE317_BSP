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
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: Code specific to the Stylus & Mouse CPLs
**/

#include "cplpch.h"

/////////////////////////
// Calibrate Page
////////////////////////

extern "C" BOOL APIENTRY CalibrateDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
		case WM_INITDIALOG:
			//Make the title device-neutral. If no regkey, load from RC file
			SetDeviceDependentText(hDlg, IDC_CALIB_TEXT, IDS_CALIBRATE_TEXT);
			return 1;

	    case WM_COMMAND:      
            switch (LOWORD(wParam))
            {
				case IDC_BUTTON_CALIB:
					TouchCalibrate();
					SetFocus(DI(IDC_BUTTON_CALIB));
					return TRUE;

				case IDOK:
					return TRUE;
	    	}
			break;
			
	}
    return (FALSE);
}

/////////////////////////
//
// Double-tap and Double-click pages
//
////////////////////////

#define CX_MIN					10			// Minimum x-distance 
#define CY_MIN					10			// Minimum y-distance 
#define CX_MAX					50			// Maximum x-distance 
#define CY_MAX					50			// Maximum y-distance 
#define DTAP_MIN				250			// Fastest double-tap time
#define DTAP_MAX				2000		// Slowest double-tap time
#define TIMER_ID				1			// Timer id
#define TIMER_TO				(DTAP_MAX+1)	// Timer timeout

class CDblClick
{
private:
	HWND	m_hDlg;
	BOOL	m_fMouse;
	
	// original settings, remembered so we know whether to save or not
	UINT 	 uDblClkTimeOrg;	// Original Dblclk time
	int		 cxDblClkOrg;		// Original x dblclk distance
	int		 cyDblClkOrg;		// Original y dblclk distance
	// current settings
	UINT 	 uCurDelta;  		// Current Doubleclick time
	int	 	 cxDblClkCur;		// Current  x dblclk distance
	int		 cyDblClkCur;		// Current  y dblclk distance

	// The two test bitmaps we swap between
	HBITMAP	 hbmpDblClkTest1;
	HBITMAP	 hbmpDblClkTest2;

	UINT 	 uClkTimeFirst;		// Keep track of Dblclk first lbtndown time
	POINT	 PntFirst;			// First lbtnbown  for DblClk.
	POINT	 PntFirstTest;		// First lbtnbown  for DblClk.
	BOOL 	 fDblClkSetup;  	// Is this the second lbuttondown?
	BOOL 	 fDblClkTest;  		// Is this the second lbuttondown?
	int 	 ClapperToggle;
	int 	 GridToggle;

	VOID  CalcDblClkArea(LPPOINT ppt2, LPPOINT ppt1)
	{
		cxDblClkCur = abs(ppt1->x - ppt2->x);
 		cyDblClkCur = abs(ppt1->y - ppt2->y);
	
		if (cxDblClkCur < CX_MIN)
			cxDblClkCur = CX_MIN;
	
		if (cyDblClkCur < CY_MIN)
			cyDblClkCur = CY_MIN;
	}

	UINT  ValidateDblClkTime( UINT	uDelta)
	{
		if (uDelta > DTAP_MAX)
			return (DTAP_MAX);
		if (uDelta < DTAP_MIN)
			return (DTAP_MIN);
		return uDelta;
	}

	UINT  ValidateDblClkDist(UINT	uDist,	BOOL	fWidth)
	{
		UINT	uMin = fWidth ? CX_MIN : CY_MIN;
		UINT	uMax = fWidth ? CX_MAX : CY_MAX;

		if (uDist > uMax)
			return (uMax);
		if (uDist < uMin)
			return (uMin);
		return uDist;
	}

public:
	CDblClick(BOOL fMouse, HWND hDlg)
	{
		ZEROMEM(this);

		m_hDlg = hDlg;
		m_fMouse = fMouse;
		
		// Get current DblTap settings
		uDblClkTimeOrg = GetDoubleClickTime();
		cxDblClkOrg = GetSystemMetrics(SM_CXDOUBLECLK);
		cyDblClkOrg = GetSystemMetrics(SM_CYDOUBLECLK);
	
		DEBUGMSG(ZONE_STYLUS, (L"Get from system: time=%d distx=%d  disty=%d \r\n", uDblClkTimeOrg, cxDblClkOrg, cyDblClkOrg));

		// Validate & reset to within limits if neccesary
		uCurDelta = uDblClkTimeOrg = ValidateDblClkTime(uDblClkTimeOrg);
		cxDblClkCur = cxDblClkOrg = ValidateDblClkDist(cxDblClkOrg, TRUE);
		cyDblClkCur = cyDblClkOrg = ValidateDblClkDist(cyDblClkOrg, FALSE);

		// Load bitmaps
	 	hbmpDblClkTest1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLAPGRID1));
 		hbmpDblClkTest2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLAPGRID2));
		ASSERT(hbmpDblClkTest1 && hbmpDblClkTest2);
		//SendDlgItemMessage(m_hDlg, IDC_GETDBLCLK1, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmpDblClkTest1);
		//SendDlgItemMessage(m_hDlg, IDC_GETDBLCLK2, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmpDblClkTest2);
	}

	~CDblClick() {
		MyDeleteObject(hbmpDblClkTest1);
		MyDeleteObject(hbmpDblClkTest2);
	}

	// we are no longer between-clicks -- Kill timer & reset flags
	void ResetPenEvent(void)
	{
		KillTimer(m_hDlg, TIMER_ID);
		fDblClkSetup = FALSE;
		fDblClkTest  = FALSE;
	}

	void SaveSettings(void)
	{
		uCurDelta 	= ValidateDblClkTime(uCurDelta);
		cxDblClkCur = ValidateDblClkDist(cxDblClkCur, TRUE);
		cyDblClkCur = ValidateDblClkDist(cyDblClkCur, FALSE);

		if 	(uCurDelta != uDblClkTimeOrg || cxDblClkCur != cxDblClkOrg ||  cyDblClkCur != cyDblClkOrg)
		{
			CReg reg;
			reg.Create(HKEY_CURRENT_USER, RK_CONTROLPANEL_PEN);
			reg.SetDW(RV_DBLTAPTIME, uCurDelta);
			reg.SetDW(RV_DBLTAPDIST, max(cxDblClkCur, cyDblClkCur)/2); // take max of both and div by 2
			NotifyWinUserSystem(NWUS_DOUBLE_TAP_CHANGED);
		}
	}

	// called on button-down in setup area
	void OnSetupClick(POINT pt, UINT time)
	{
		if(fDblClkTest)
			ResetPenEvent(); // if we had started a Test, but 2nd click is in Set, reset Test flags/timer

		if(!fDblClkSetup)
		{
			// if first click
		 	uClkTimeFirst = time;
		 	PntFirst = pt;
			SetTimer(m_hDlg, TIMER_ID, TIMER_TO, NULL);
		}
		else
		{
			// if second click
			KillTimer(m_hDlg, TIMER_ID);
			// compute, validate & save dbl click distances
			CalcDblClkArea(&pt, &PntFirst);
			// compute, validate & save dbl click time
			uCurDelta = ValidateDblClkTime(time - uClkTimeFirst);
			// toggle the grid flag
			GridToggle = ~GridToggle;
			//fDblClkWasSet = TRUE;
			DEBUGMSG(ZONE_STYLUS, (L"Set DblClick. time=%d cx=%d cy=%d \r\n", uCurDelta, cxDblClkCur, cyDblClkCur));

			ShowWindow(GetDlgItem(m_hDlg, IDC_GETDBLCLK1), GridToggle ? SW_HIDE : SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg, IDC_GETDBLCLK2), GridToggle ? SW_SHOW : SW_HIDE);
		}
	 	fDblClkSetup = !fDblClkSetup;
	 	fDblClkTest = FALSE;
	 }

	// called on button-down in test area
	void OnTestClick(POINT pt, UINT time)
	{
		if(fDblClkSetup) 
			ResetPenEvent(); // if we had started a Set, but 2nd click is in Test, reset Set flags/timer

		fDblClkSetup = FALSE;
		if (fDblClkTest)
		{
			KillTimer(m_hDlg, TIMER_ID);
			
			DEBUGMSG(ZONE_STYLUS, (L"In Test. Curr: time=%d cx=%d cy=%d \r\n", uCurDelta, cxDblClkCur, cyDblClkCur));
			DEBUGMSG(ZONE_STYLUS, (L"  time=%d cx=%d cy=%d \r\n", time - uClkTimeFirst, abs(PntFirstTest.x - pt.x), abs(PntFirstTest.y - pt.y) ));

			if ((uCurDelta   >= (UINT)time - uClkTimeFirst) &&
				 (cyDblClkCur >= abs(PntFirstTest.y - pt.y)) &&
				 (cxDblClkCur >= abs(PntFirstTest.x - pt.x)))
			{
				// success in range !!
				ClapperToggle = ~ClapperToggle;
				ShowWindow(GetDlgItem(m_hDlg, IDC_CLAPPER1), ClapperToggle ? SW_HIDE : SW_SHOW);
				ShowWindow(GetDlgItem(m_hDlg, IDC_CLAPPER2), ClapperToggle ? SW_SHOW : SW_HIDE);
				DEBUGMSG(ZONE_STYLUS, (L"Dbl click in range.\r\n"));
				fDblClkTest = FALSE;
				return;
			}
		}
		uClkTimeFirst = time;
		PntFirstTest = pt;
		SetTimer(m_hDlg, TIMER_ID, TIMER_TO, NULL);
		fDblClkTest = TRUE;
	}
};

extern "C" BOOL APIENTRY DblTapDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	// can't have statics in this wndproc, because it's shared by 2 cpls -- stylus & mouse
	CDblClick* pDblClick = (CDblClick*)GetWindowLong(hDlg, DWL_USER);
	
    switch (message)
    {
    case WM_INITDIALOG:
      {
    	// The mouse & stylus versions of this page share this same WndProc
  		// lParam is the PROPSHEETPAGE struct. From it figure out if we
  		// are in the Mouse or Stylus case
  		BOOL fMouse = (lstrcmpi(CELOADSZ(IDS_MOUSE_DBLCLICKTAB), ((PROPSHEETPAGE*)lParam)->pszTitle)==0);
  		
  		// create our context object
    	pDblClick = new CDblClick(fMouse, hDlg);
		SetWindowLong(hDlg, DWL_USER, (DWORD)pDblClick);

#ifdef DEBUG
		RECT rc1, rc2;
		GetWindowRect(DI(IDC_GETDBLCLK1), &rc1);
		GetWindowRect(DI(IDC_TESTDBLCLK), &rc2);
		DEBUGMSG(ZONE_STYLUS, (L"Calib: box1=(%d,%d,%d,%d) box2=(%d,%d,%d,%d)\r\n", rc1.left, rc1.top, rc1.right, rc1.bottom, rc2.left, rc2.top, rc2.right, rc2.bottom));
#endif

    	// set up statics based on mouse or stylus
		SetDlgItemText(hDlg, IDC_DBLTAP_SET, (LPCWSTR)LoadString(g_hInst, (fMouse ? IDS_DBLCLICK_SET : IDS_DBLTAP_SET), 0, 0));
		SetDlgItemText(hDlg, IDC_DBLTAP_TEST, (LPCWSTR)LoadString(g_hInst, (fMouse ? IDS_DBLCLICK_TEST : IDS_DBLTAP_TEST), 0, 0));
		return 1;
	  }				
    case WM_LBUTTONDOWN:   
    case WM_LBUTTONDBLCLK:   
      {
		// Since GetMessageTime API is NYI use GetTickCount (note though that the 
		// time field of the MSG struct *is* being set correctly -- but it's not accessible 
		// since the GetMessage loop is in the the PropSheet code. 
		UINT 	time = GetTickCount(); 
		POINT 	pt;
 		MakePoint(&pt, lParam);
		int 	idCtl = GetDlgCtrlID(ChildWindowFromPoint(hDlg, pt));

		DEBUGMSG(ZONE_STYLUS, (L"In BtnDown. idCtl=%d pt.x=%d pt.y=%d \r\n", idCtl, pt.x, pt.y));
		switch(idCtl)
		{	
		default:
			// clicked outside our to areas
			pDblClick->ResetPenEvent();
			return FALSE;
		
		case IDC_GETDBLCLK1:
		case IDC_GETDBLCLK2:
			pDblClick->OnSetupClick(pt, time);
			return TRUE;

		case IDC_TESTDBLCLK:
		case IDC_CLAPPER1:
		case IDC_CLAPPER2:
			pDblClick->OnTestClick(pt, time);
			return TRUE;
		}
    	break;
      }
				
	case WM_TIMER:
		DEBUGMSG(ZONE_STYLUS, (L"Timer expired, reseting...\r\n"));
		pDblClick->ResetPenEvent();
		break;

    case WM_COMMAND:      
		switch (LOWORD(wParam))
		{
		case IDOK:
			pDblClick->SaveSettings();
			return TRUE;
		}
		break;
			
	case WM_DESTROY:
		pDblClick->ResetPenEvent();
		delete pDblClick;
		pDblClick = NULL;
		SetWindowLong(hDlg, DWL_USER, 0);
		break;
	}
    return (FALSE);
}

// Need seperate dlg procs for mouse & stylus componentization
extern "C" BOOL APIENTRY DblClickDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	return DblTapDlgProc(hDlg, message, wParam, lParam);
}


