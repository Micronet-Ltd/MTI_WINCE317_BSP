// Copyright (c) 2007, 2013 2008 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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

**/


#include "IgnitionCpl.h"
#include <creg.hxx>
#include <pm.h>
#include "..\..\Ctlpnl\CPLMAIN\regcpl.h"
#include <bsp.h>

// Returns the number of characters in an expression.
#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))

BOOL CALLBACK IgnitionCplDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE	g_hInstance = NULL;

#define	 STR_SECOND		_T("second")
#define	 STR_SECONDS	_T("seconds")
#define	 STR_MINUTE		_T("minute")
#define	 STR_MINUTES	_T("minutes")

DWORD	stable_times[]	= {0, 1, 2};//, 3, 5, 10 };//, 15, 30, 60, 60 * 2, 60 * 5, 60 * 10, 0};	
DWORD	delay_times[]	= {3, 5, 10, 30, 60, 60 * 2, 60 * 5, 60 * 10 };//, 60 * 30, 60 * 45, 60 * 60, 60 * 60 * 2, 0};	
//////////////////////////////

class CIgniControl
{
public:
	CIgniControl()
	{
		Reset();
		m_ShutDownAbsolute = 0;
	}
	BOOL	operator==(const CIgniControl& igni)
	{
		if( m_fEnable				!= igni.m_fEnable			||
			m_StableTime			!= igni.m_StableTime		||
			m_PerformAction			!= igni.m_PerformAction		||
			m_PerformActionDelay	!= igni.m_PerformActionDelay	)
		{
			return FALSE;
		}
		return TRUE;
	}
	BOOL	operator!=(const CIgniControl& igni)
	{
		if( m_fEnable				!= igni.m_fEnable			||
			m_StableTime			!= igni.m_StableTime		||
			m_PerformAction			!= igni.m_PerformAction		||
			m_PerformActionDelay	!= igni.m_PerformActionDelay	)
		{
			return TRUE;
		}
		return FALSE;
	}

	BOOL GetFromReg()
	{
		//GetShutDownAbsolute();

		CReg reg;
		if( reg.Open( HKEY_LOCAL_MACHINE, RK_FULL_PSC ) )
		{
			m_fEnable				= reg.ValueDW(RK_IGN_ENABLE,		0		);
			m_StableTime			= reg.ValueDW(RK_PSC_IGN_TIME,		3000	);
			
			m_ShutDownAbsolute		= reg.ValueDW(RK_PSC_IGN_SHUTDOWN_ABSOLUTE,	0	);

			if(0 == m_ShutDownAbsolute)
				m_PerformAction			= reg.ValueDW(RK_PSC_IGN_ACT,		1		);
			else
				m_PerformAction			= SHUTDOWN;

			m_PerformActionDelay	= reg.ValueDW(RK_PSC_IGN_ACT_DELAY,	5000	);

			RETAILMSG(1,(_T("IgnitionCpl::GetFromReg %d %d %d %d %d\r\n"), m_fEnable, m_StableTime, m_PerformAction, m_PerformActionDelay, m_ShutDownAbsolute));
			return 1;
		}
		return 0;
	}

	BOOL	SetToReg()
	{
		BOOL res = 0;
		CReg reg;
		if( res = reg.OpenOrCreateRegKey( HKEY_LOCAL_MACHINE, RK_FULL_PSC ) )
		{
			res |= reg.SetDW(RK_IGN_ENABLE,			m_fEnable			);
			res |= reg.SetDW(RK_PSC_IGN_TIME,		m_StableTime		);
			if(0 == m_ShutDownAbsolute)
				res |= reg.SetDW(RK_PSC_IGN_ACT,		m_PerformAction		);
			res |= reg.SetDW(RK_PSC_IGN_ACT_DELAY,	m_PerformActionDelay);
			
			RETAILMSG(1,(_T("IgnitionCpl::SetToReg %d %d %d %d\r\n"), m_fEnable, m_StableTime, m_PerformAction, m_PerformActionDelay));
		}
		return res;
	}

	void	Reset()
	{
		m_fEnable			= 0;     
		m_StableTime		= 0;
		m_PerformAction		= 0;
		m_PerformActionDelay= 0;
		m_ShutDownAbsolute	= 0;
	}
	DWORD	GetfEnable() const				{ return m_fEnable;				}         
	DWORD	GetStableTime() const			{ return m_StableTime;			}
	DWORD	GetPerformAction() const		
	{ 
		if(m_ShutDownAbsolute)
			return SHUTDOWN;

		return m_PerformAction;		
	}    
	DWORD	GetPerformActionDelay() const	{ return m_PerformActionDelay;	}

	void	SetfEnable				(DWORD fEnable			)	{ m_fEnable				= fEnable;			}         
	void	SetStableTime			(DWORD StableTime		)	{ m_StableTime			= StableTime;		}
	void	SetPerformAction		(DWORD PerformAction	)	{ m_PerformAction		= PerformAction;	}    
	void	SetPerformActionDelay	(DWORD PerformActionDelay) 	{ m_PerformActionDelay	= PerformActionDelay;}
	
	BOOL	IsActChangeable		() const					{ return (0 == m_ShutDownAbsolute);			}
private:	
	//void	GetShutDownAbsolute	()
	//{
	//	m_ShutDownAbsolute = 0;	 
	//	UINT32				number			= 0;
	//	manufacture_block	mb				= {0};
	//	char				str[2]			= {0};
	//	char*				endptr;

	//	m_ShutDownAbsolute = 0;	 

	//	HalQueryParamBlock( 0, 0, (void*)&mb, sizeof(mb), &number );
	//	str[0] = mb.device_identification[CONF_BYTE];
	//	number = strtol(str, &endptr, 16);
	//	//bit 3 of byte 18 in catalog number 	
	//	if( (0xFF				!= number) &&	//not exists
	//		((1 << BIT_ENABLE)	&  number) &&
	//		(MASK_EXISTS		&  number) )
	//	{
	//		m_ShutDownAbsolute = 1;
	//	}
	//}

	DWORD	m_fEnable;     
	DWORD	m_StableTime;
	DWORD	m_PerformAction;//  0 - suspend, 1 –Shutdown (default shutdown)
	DWORD	m_PerformActionDelay;
	UINT32	m_ShutDownAbsolute;
};

BOOL GetPowerRegTimeouts(DWORD& BattUserTimeout, DWORD& BattSysTimeout, DWORD& BattSuspTimeout, DWORD& AcSuspTimeout)
{
	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(1, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}
	BattUserTimeout = reg.ValueDW(PM_BATT_USER_IDLE_SZ, 0);
	BattSysTimeout	= reg.ValueDW(PM_BATT_SYSTEM_IDLE_SZ, 0);
	BattSuspTimeout = reg.ValueDW(PM_BATT_SUSPEND_SZ, 0);
	AcSuspTimeout	= reg.ValueDW(PM_AC_SUSPEND_SZ, 0);

	return TRUE;
}

BOOL SetPowerRegTimeouts(DWORD BattUserTimeout, DWORD BattSysTimeout, DWORD BattSuspTimeout, DWORD AcSuspTimeout)
{
	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(1, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}
	reg.SetDW(PM_BATT_USER_IDLE_SZ,		BattUserTimeout);
	reg.SetDW(PM_BATT_SYSTEM_IDLE_SZ,	BattSysTimeout);
	reg.SetDW(PM_BATT_SUSPEND_SZ,		BattSuspTimeout);
	reg.SetDW(PM_AC_SUSPEND_SZ,			AcSuspTimeout);

	return TRUE;
}
BOOL NotifyPowerMgr()
{
	// Tell power manager to reload the registry timeouts
	HANDLE hevReloadActivityTimeouts = OpenEvent(EVENT_ALL_ACCESS, FALSE, PM_EVENT_NAME);
	if(!hevReloadActivityTimeouts) 
	{
		DEBUGMSG(1, (TEXT("Error: Could not open PM reload event\r\n")));
	}
	SetEvent(hevReloadActivityTimeouts);
	CloseHandle(hevReloadActivityTimeouts);
	return TRUE;
}

int ComboBox_FindData(HWND hItem, DWORD dwFindData)
{
	for(int i=0; ;i++)
	{
		DWORD dwData = ComboBox_GetItemData(hItem, i);
		if(dwData == CB_ERR)
			return CB_ERR;
		if(dwFindData == dwData)
			return i;
	}
}
int ComboBox_GetSelData(HWND hItem, DWORD& dwFindData)
{
	int iSel;
	
	if( CB_ERR != (iSel = ComboBox_GetCurSel(hItem)) )
	{
		dwFindData = ComboBox_GetItemData(hItem, iSel);
	}
	return iSel;
}
void LoadCBTimeStr(HWND hwndCB, const DWORD rgCBData[], DWORD Len) 
{
	TCHAR szBuf[32] = {0};

	DWORD	dwTime;
	TCHAR*	pstr;
	int		iIndex;
	for(DWORD i = 0; i < Len; i++) 
	{
		dwTime = rgCBData[i];

		if( 60 > dwTime )
		{
			pstr = (dwTime == 1) ? STR_SECOND : STR_SECONDS;
			_sntprintf(szBuf, LENGTHOF(szBuf), TEXT("%u %s"), dwTime, pstr);
		}
		else 
		{ 
			dwTime /= 60;
			pstr = (dwTime == 1) ? STR_MINUTE : STR_MINUTES;
			_sntprintf(szBuf, LENGTHOF(szBuf), TEXT("%u %s"), dwTime, pstr);
		}		
		iIndex = ComboBox_AddString(hwndCB, szBuf);
		if(iIndex!=CB_ERR) 
		{
			ComboBox_SetItemData(hwndCB, iIndex, rgCBData[i]);
		}
	}
}
void GetDlgData( HWND hDlg, CIgniControl& ignition )
{
	HWND hcheck = GetDlgItem(hDlg, IDC_IGNI_ENABLE);
	if(hcheck)
		ignition.SetfEnable( Button_GetCheck(hcheck) );
	
	DWORD data;
	if(hcheck = GetDlgItem(hDlg, IDC_IGNSTABLE))
	{
		if( CB_ERR != ComboBox_GetSelData(hcheck, data) )
		{
			ignition.SetStableTime( data * 1000 );
		}
	}
	if(hcheck = GetDlgItem(hDlg, IDC_ACT))
	{
		if( CB_ERR != ComboBox_GetSelData(hcheck, data) )
		{
			ignition.SetPerformAction(data);
		}
	}
	if(hcheck = GetDlgItem(hDlg, IDC_ACT_DELAY))
	{
		if( CB_ERR != ComboBox_GetSelData(hcheck, data) )
		{
			ignition.SetPerformActionDelay(data);
		}
	}
}
void SetDlgData( HWND hDlg, const CIgniControl& ignition )
{
	DWORD fEnable = ignition.GetfEnable();
	HWND hctr = GetDlgItem(hDlg, IDC_IGNI_ENABLE);
	if(hctr)
		Button_SetCheck(hctr, fEnable);
		
	DWORD ind;
	hctr = GetDlgItem(hDlg, IDC_ACT);
	if(hctr)
	{
		ind = ComboBox_AddString(hctr, _T("Suspend"));
		if(ind!=CB_ERR) 
		{
			ComboBox_SetItemData(hctr, ind, SUSPEND);
		}
		ind = ComboBox_AddString(hctr, _T("Shutdown"));
		if(ind!=CB_ERR) 
		{
			ComboBox_SetItemData(hctr, ind, SHUTDOWN);
		}
		ComboBox_SetCurSel(hctr, ComboBox_FindData(hctr, ignition.GetPerformAction()));
		EnableWindow(hctr, (ignition.IsActChangeable() ? fEnable : 0));
	}

	hctr = GetDlgItem(hDlg, IDC_IGNSTABLE);
	if(hctr)
	{
		LoadCBTimeStr( hctr, stable_times, LENGTHOF(stable_times) );
		ComboBox_SetCurSel(hctr, ComboBox_FindData(hctr, ignition.GetStableTime()/1000));
		EnableWindow(hctr, fEnable);
	}

	hctr = GetDlgItem(hDlg, IDC_ACT_DELAY);
	if(hctr)
	{
		LoadCBTimeStr( hctr, delay_times, LENGTHOF(delay_times) );
		ComboBox_SetCurSel(hctr, ComboBox_FindData(hctr, ignition.GetPerformActionDelay()));
		EnableWindow(hctr, fEnable);
	}
}
extern "C" BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Remember the instance
			g_hInstance = (HINSTANCE)hInstance;
			break;

		case DLL_PROCESS_DETACH:
			g_hInstance = NULL;
			break;
    }
    return TRUE;
}

BOOL CALLBACK IgnitionCplDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
 	static CIgniControl Igni;

	switch (message)
    {
	    case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_IGNI_ICON));
			if (hIcon)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
			Igni.GetFromReg();
			SetDlgData(hDlg, Igni);
		}
	    return 1;

//   	case WM_DRAWITEM:
//   		break;

		case WM_COMMAND:      
		{
			switch (LOWORD(wParam))
			{
				// enable/disable CBs based on checkbox
				case IDC_IGNI_ENABLE:
				{	
					HWND hcheck = GetDlgItem(hDlg, IDC_IGNI_ENABLE);
					if(hcheck)
					{
						DWORD flag = Button_GetCheck(hcheck);
						EnableWindow(GetDlgItem(hDlg, IDC_IGNSTABLE), flag);
						EnableWindow(GetDlgItem(hDlg, IDC_ACT), (Igni.IsActChangeable() ? flag : 0));
						EnableWindow(GetDlgItem(hDlg, IDC_ACT_DELAY), flag);
					}
				}
				return TRUE;
				case IDC_IGNSTABLE:
				{
					if(CBN_DROPDOWN == (int)HIWORD(wParam))
					{
						EnableWindow(GetDlgItem(hDlg, IDC_ACT_DELAY), 0);
					}
					else if(CBN_CLOSEUP == (int)HIWORD(wParam))
					{
						EnableWindow(GetDlgItem(hDlg, IDC_ACT_DELAY), 1);
					}
				}
				break;
				case IDOK:
				{
					BOOL	ResetFlag = 0;
					CIgniControl TmpIgni;
					GetDlgData(hDlg, TmpIgni);

					TCHAR str[MAX_PATH] = {0};
					LoadString(g_hInstance, IDS_IGNI_DESCRIPTION, str, LENGTHOF(str)); 

					if(TmpIgni.GetfEnable())
					{
						DWORD	BattUserTimeout = 0, BattSysTimeout = 0, 
								BattSuspTimeout = 0, AcSuspTimeout = 0;
						GetPowerRegTimeouts(BattUserTimeout, BattSysTimeout, BattSuspTimeout, AcSuspTimeout);
						
						//if batt == never or ac != never
						if(	PM_TRANSITION_NEVER_DW == BattUserTimeout	|| 
							PM_TRANSITION_NEVER_DW == BattSysTimeout	|| 
							PM_TRANSITION_NEVER_DW == BattSuspTimeout	|| 
							PM_TRANSITION_NEVER_DW != AcSuspTimeout		)
						{
							if( IDYES != MessageBox(hDlg, PWR_TIMEOUTS_WARN, str, MB_ICONWARNING | MB_YESNO) )
								break;
							//update incompatible velues
							if(PM_TRANSITION_NEVER_DW == BattUserTimeout)
								BattUserTimeout = PM_MAX_BATT_TIMEOUT;
							if(PM_TRANSITION_NEVER_DW == BattSysTimeout) 
								BattSysTimeout = PM_MAX_BATT_TIMEOUT;
							if(PM_TRANSITION_NEVER_DW ==  BattSuspTimeout)
								BattSuspTimeout  = PM_MAX_BATT_TIMEOUT;
							if(PM_TRANSITION_NEVER_DW != AcSuspTimeout)
								AcSuspTimeout = PM_TRANSITION_NEVER_DW;

							SetPowerRegTimeouts(BattUserTimeout, BattSysTimeout, BattSuspTimeout,AcSuspTimeout);
							NotifyPowerMgr();
						}
					}
					if( TmpIgni != Igni )
					{
						TmpIgni.SetToReg();
						if(IDYES == MessageBox(hDlg, RESTART_STR, str, MB_ICONWARNING | MB_YESNO))
						{
							ResetFlag = 1;
							SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
						}
					}
					if(ResetFlag)
						SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
					else
						RegFlushKey(HKEY_LOCAL_MACHINE);
					EndDialog(hDlg, IDOK);
				}	
				break;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
				break;
			}
		}	
		break;
   		case WM_DESTROY:
			RETAILMSG(0,(_T("IgnitionCpl: destroid\r\n")));
   			break;
    }
    return (FALSE);
}


// When main control panel is loaded, IgnitionCpl.cpl will be loaded, then
// CPL_INIT, CPL_GETCOUNT and CPL_NEWINQUIRE will be called, then
// IgnitionCpl.cpl will be unloaded. When the IgnitionCpl icon is double clicked,
// IgnitionCpl.cpl will be loaded, then CPL_INIT and CPL_DBLCLK will be called.

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            RETAILMSG(0, (TEXT("IgnitionCpl: CPL_INIT\r\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            RETAILMSG(0, (TEXT("IgnitionCpl: CPL_GETCOUNT\r\n")));
            return 1;

        case CPL_NEWINQUIRE:
		{
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            RETAILMSG(0, (TEXT("IgnitionCpl: CPL_NEWINQUIRE\r\n")));

            // this applet supports only 1 action, lParam1 must be 0
            ASSERT(0 == lParam1);
            ASSERT(lParam2);

            NEWCPLINFO* lpNewCplInfo = (NEWCPLINFO *) lParam2;
 
			if (lpNewCplInfo)
            {
				_tcscpy(lpNewCplInfo->szName, _T(""));
				_tcscpy(lpNewCplInfo->szInfo, _T(""));

				lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
                lpNewCplInfo->dwFlags = 0;
                lpNewCplInfo->dwHelpContext = 0;
 				
                LoadString(g_hInstance, IDS_IGNI_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_IGNI_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->lData = IDI_IGNI_ICON;
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_IGNI_ICON));
				
				RETAILMSG(0, (TEXT("IgnitionCpl: szInfo = \"%s\"\r\n"), lpNewCplInfo->szInfo));
                RETAILMSG(0, (TEXT("IgnitionCpl: szName = \"%s\"\r\n"), lpNewCplInfo->szName));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("IgnitionCpl: CPL_NEWINQUIRE failed\r\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
			
        case CPL_DBLCLK:
		{
			// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
			TCHAR pszTitle[MAX_PATH] = {0};

			LoadString(g_hInstance, IDS_IGNI_DESCRIPTION, pszTitle,MAX_PATH);
			ASSERT(pszTitle);
            
			RETAILMSG(0, (TEXT("IgnitionCpl: CPL_DBLCLK - %s\r\n"), pszTitle));
			HWND hWnd = FindWindow(L"Dialog", pszTitle);
			if (hWnd)
			{
				SetForegroundWindow(hWnd);
			}
			else
			{
				//InitCommonControls();
				if( -1 == DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_IGNIDLG), hwndCPL, IgnitionCplDlgProc) )
				{
		            RETAILMSG(1, (TEXT("IgnitionCpl: CPL_DBLCLK err %d\r\n"), GetLastError() ));

				}
			}
		}			
		return 0;
			
        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            RETAILMSG(0, (TEXT("IgnitionCpl: CPL_STOP\r\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            RETAILMSG(0, (TEXT("IgnitionCpl: CPL_EXIT\r\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}
