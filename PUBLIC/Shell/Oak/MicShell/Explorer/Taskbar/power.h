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

#ifndef _POWER_MANAGER_UI_H_
#define _POWER_MANAGER_UI_H_

#include <pm.h>

void GetDisplayPowerSettings(BOOL *pShowPowerIcon, BOOL *pShowWarnings);
#define POWER_NOTIFYICON               (WM_USER + 4)

class PowerManagerUI
{
private:
	BOOL m_fShowPower;
	BOOL m_fShowWarning;
	HINSTANCE m_hInstance;

    HWND m_hwndBBL;
    HWND m_hwndBBVL;
    HWND m_hwndMBVL;

	UINT m_uID;

	BOOL m_fShowingIcon;
    BOOL m_fShowingBackupBatteryIcon;

public:
	PowerManagerUI();
	PowerManagerUI(HINSTANCE hInstance);
	virtual ~PowerManagerUI();

	void CreatePowerTrayIcon(HWND hwndTB, UINT uID);
	void UpdatePower(HWND hwndTB, PPOWER_BROADCAST pPowerInfo); 
    BOOL HandleIconMessage(HWND hwnd, WPARAM wp, LPARAM lp);
    BOOL ShowPowerWarning(HWND hwnd, UINT nID, DLGPROC dlgproc);
    LRESULT HandleWarningDialogMessage(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    BOOL IsPowerManagerUIDialogMessage(MSG msg);
};

#endif //_POWER_MANAGER_UI_H_
