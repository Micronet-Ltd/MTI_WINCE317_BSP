// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
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


#include "LightSensor.h"
#include <creg.hxx>
#include <light_sense.h>


// Returns the number of characters in an expression.
#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))

#define LS_MIN_SETTING         0
#define LS_MAX_SETTING         100
#define LS_NUM_TICKS           100
#define LS_TICK_INTERVAL       ((LS_MAX_SETTING - LS_MIN_SETTING) / LS_NUM_TICKS)


HINSTANCE g_hInstance = NULL;

BOOL CALLBACK LightSensorDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void	InitSlider(HWND hwndSlider, DWORD dwLevel);
DWORD	GetSliderValue( HWND hwndSlider );
UINT32	Volts2Luma(UINT32 luma);
UINT32	Luma2Volts(UINT32 volts);
void	InitLsSettings(HWND hDlg);
void	RestoreSetting();
void	UpdateLsSettings(HWND hDlg);
void	RecheckPos( HWND hDlg, HWND hwnd  );
BOOL	GetHLFromReg( DWORD* pValH, DWORD* pValL  );
BOOL	SetHLToReg(  DWORD ValH, DWORD ValL   );
BOOL	NotifyChange();
void	StopAndClean();
DWORD LSenseThread(VOID *pParams);

struct LsParams
{
	HANDLE			hExitEvent;
	HWND			NotifWnd;	
};

LsParams	LigthSensorParams	= {0};
HANDLE		hThread				= 0;
DWORD		g_LevelLow = 0x64; 
DWORD		g_LevelHigh = 0x1F4;


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

////////////////////////////////////////////////////////////////////////////////
//
//  void InitSlider(HWND hDlg, DWORD dwLevel)
//
//  Initialises trackbar controls 
//
//////////////////////////////////////////////////////////////////////////////// 
void InitSlider(HWND hwndSlider, DWORD dwLevel)
{
	if( hwndSlider )
	{
		if(dwLevel > LS_MAX_SETTING)
			dwLevel = LS_MAX_SETTING;

		SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(LS_MIN_SETTING/LS_TICK_INTERVAL, LS_MAX_SETTING/LS_TICK_INTERVAL));
		SendMessage(hwndSlider, TBM_SETTICFREQ, 20, 0L);
		SendMessage(hwndSlider, TBM_SETPAGESIZE, 0L, 1);

		SendMessage(hwndSlider, TBM_SETPOS, TRUE, (dwLevel / LS_TICK_INTERVAL));
	}
}
DWORD GetSliderValue( HWND hwndSlider )
{
	DWORD dwLevel = LS_MIN_SETTING;
	if( hwndSlider )
	{
		dwLevel = LS_TICK_INTERVAL * SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
    
		if(dwLevel > LS_MAX_SETTING)
			dwLevel = LS_MAX_SETTING;
	}
//	RETAILMSG( 1, (_T("Level: %d\r\n"), dwLevel) ); 
	return dwLevel;
}
UINT32 Volts2Luma(UINT32 luma)
{
	return luma/15;
}
UINT32 Luma2Volts(UINT32 volts)
{
	return volts * 15;
}
void InitLsSettings(HWND hDlg)
{
	DWORD LevelLow = 0x64; 
	DWORD LevelHigh = 0x1F4;
	
	GetHLFromReg( &LevelHigh, &LevelLow );

	//store for restore
	g_LevelLow	= LevelLow; 
	g_LevelHigh = LevelHigh;


	LevelHigh = Volts2Luma(LevelHigh);
	LevelLow = Volts2Luma(LevelLow);

	InitSlider( GetDlgItem(hDlg, IDC_LIGTH_HIGH), LevelHigh );
	InitSlider( GetDlgItem(hDlg, IDC_LIGTH_LOW), LevelLow );
	
}
////////////////////////////////////////////////////////////////////////////////
//
//  void UpdateLsSettings(HWND hDlg) 
//
//  Updates LS settings to the registry from trackbar control & signal 
//  change event. 
//
//////////////////////////////////////////////////////////////////////////////// 
void UpdateLsSettings(HWND hDlg)
{
	if( hDlg )
	{
		DWORD LevelLow = 0, LevelHigh = 0;
		
		LevelHigh = GetSliderValue( GetDlgItem(hDlg, IDC_LIGTH_HIGH) );
		LevelLow = GetSliderValue( GetDlgItem(hDlg, IDC_LIGTH_LOW) );
	 
		SetHLToReg( Luma2Volts(LevelHigh), Luma2Volts(LevelLow) );
	}
}
void RestoreSetting()
{
	SetHLToReg( g_LevelHigh, g_LevelLow );
}
void RecheckPos( HWND hDlg, HWND hwnd  )
{	
	static HWND	LsHigh	= GetDlgItem(hDlg, IDC_LIGTH_HIGH);
	static HWND	LsLow	= GetDlgItem(hDlg, IDC_LIGTH_LOW);

	DWORD	pos;
	DWORD	Pos;

	if( hwnd == LsHigh )
	{
		Pos = SendMessage(LsHigh, TBM_GETPOS, 0, 0 );
		pos = SendMessage(LsLow, TBM_GETPOS, 0, 0 );
		 
		if( Pos < pos )
			SendMessage( LsHigh, TBM_SETPOS, TRUE, pos );
	}
	else if( hwnd ==  LsLow )
	{
		Pos = SendMessage(LsLow, TBM_GETPOS, 0, 0 );
		pos = SendMessage(LsHigh, TBM_GETPOS, 0, 0 );
		 
		if( Pos > pos )
			SendMessage( LsLow, TBM_SETPOS, TRUE, pos );
	}
}
BOOL GetHLFromReg( DWORD* pValH, DWORD* pValL  )
{
	CReg reg;
	if( reg.Open( HKEY_LOCAL_MACHINE, LIGHT_SENSE_REG_KEY ) )
	{
		*pValH = reg.ValueDW(LIGHT_SENSE_REG_HL, 0x1F4);
		*pValL = reg.ValueDW(LIGHT_SENSE_REG_LL, 0x64);

		return 1;
	}

	return 0;
}

BOOL	SetHLToReg(  DWORD ValH, DWORD ValL   )
{
	BOOL res = 0;
	CReg reg;
	if( reg.OpenOrCreateRegKey( HKEY_LOCAL_MACHINE, LIGHT_SENSE_REG_KEY ) )
	{
		res = reg.SetDW(LIGHT_SENSE_REG_HL, ValH);
		if( res )
			res = reg.SetDW(LIGHT_SENSE_REG_LL, ValL );
	}
	return res;
}

BOOL	NotifyChange()
{
	HANDLE hEv;
	if( hEv = CreateEvent( 0, 0, 0, LIGHTSENSE_CONFIG ) )
	{
		SetEvent( hEv );
		CloseHandle( hEv );
		return 1;
	}
	return 0;
}

void StopAndClean()
{
	if( LigthSensorParams.hExitEvent )
	{
		SetEvent( LigthSensorParams.hExitEvent );
	}
	if( hThread )
	{
		if( WAIT_OBJECT_0 != WaitForSingleObject( hThread, 3000 ) )
			RETAILMSG( 1, (_T("LSCPL: Cannot close thread\r\n")));
		CloseHandle(hThread);
		hThread = 0;
	}
	if( LigthSensorParams.hExitEvent )
	{
		CloseHandle( LigthSensorParams.hExitEvent );
		LigthSensorParams.hExitEvent	 = 0;
	}
}
DWORD LSenseThread(VOID *pParams)
{
	LsParams* ls = (LsParams*)pParams;

	if( !ls || !ls->hExitEvent || !ls->NotifWnd )
	{
		RETAILMSG(1,(_T("LSenseThread - wrong parameters\r\n") ) );
		return -1;
	}

	MSGQUEUEOPTIONS msgOptions	= {0};
	TCHAR			QueueName[64] = {0};
	HANDLE			hLsDev, hMSGQ, ev[2];
	
//	memset(&msgOptions, 0, sizeof(msgOptions));
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= 0;
	msgOptions.cbMaxMessage = sizeof(AMBIENT_LIGHT);
	msgOptions.bReadAccess	= 1;

    hLsDev = LightSenseOpen(QueueName, sizeof(QueueName));
    if(!hLsDev)
	{
		RETAILMSG(1,(_T("Cannot open light sensor dev\r\n") ) );
		return -1;
	}
	
	hMSGQ = CreateMsgQueue(QueueName, &msgOptions);
	
	ev[0]	= ls->hExitEvent;
	ev[1]	= hMSGQ;
	
	TCHAR	Txt[32];
	
	while( (WAIT_OBJECT_0 + 1) == WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE) )
	{
		AMBIENT_LIGHT lsdata = AMBIENT_LIGHT_UNSPEC;
		DWORD dwSize, dwFlags;

		RETAILMSG(0,(TEXT("LSCPL: Light sensor notification\r\n")));
        
		if (!ReadMsgQueue(hMSGQ, &lsdata, sizeof(AMBIENT_LIGHT), &dwSize, 0, &dwFlags)) 
        {
			RETAILMSG(1,(TEXT("BKL: ReadMsgQueue lsense failed\r\n")));  
            break;
		}
        else if(dwSize < sizeof(AMBIENT_LIGHT)	||
				AMBIENT_LIGHT_LOW	> lsdata			||
				AMBIENT_LIGHT_HIGH	< lsdata			) 
        {
			RETAILMSG(1,(TEXT("BKL: ReadMsgQueue lsense - wrong data\r\n")));  
		}
		else//all wright
		{
			if( IsWindow( ls->NotifWnd ) )
			{
				wsprintf( Txt, _T("%s"), (	( AMBIENT_LIGHT_LOW		== lsdata ) ?	_T("Low")		:
											( AMBIENT_LIGHT_NORMAL	== lsdata ) ?	_T("Normal")	: _T("High") ));
				SetWindowText( ls->NotifWnd, Txt );
			}
		}
	}
	
	if(hMSGQ)
		CloseMsgQueue(hMSGQ);
	if(hLsDev)
		CloseHandle(hLsDev);
	return 1;
}

BOOL CALLBACK LightSensorDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LIGHT_SENSOR_ICON));
			if (hIcon)
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);

			LigthSensorParams.hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			
			if(LigthSensorParams.hExitEvent == NULL)
			{
				RETAILMSG(1, (L"ERROR: LSCPL: Failed to create exit event\r\n"));
			}
			else
			{
				LigthSensorParams.NotifWnd = GetDlgItem( hwndDlg, IDC_ST_STATE ); 
				hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LSenseThread, &LigthSensorParams, 0, NULL);
				if(!hThread)
				{
					RETAILMSG(1, (TEXT("LSCPL: CreateThread failed\r\n")));
				}
			}
			InitLsSettings(hwndDlg);
			return TRUE;
		}
 		case WM_HSCROLL:            // track bar message
            switch LOWORD(wParam)
            {
				case TB_ENDTRACK:
					RecheckPos( hwndDlg, (HWND)lParam );
					
					UpdateLsSettings(hwndDlg);///temp!!!!!!! maybe
					NotifyChange();///temp!!!!!!! maybe
				break;

				case TB_BOTTOM:
                case TB_THUMBPOSITION:
                case TB_LINEUP:
                case TB_LINEDOWN:
                case TB_PAGEUP:
                case TB_PAGEDOWN:
                case TB_TOP:
                    return TRUE;
            }

            break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					StopAndClean();
					//set params
					UpdateLsSettings(hwndDlg);
					RegFlushKey(HKEY_LOCAL_MACHINE);
					NotifyChange();

					EndDialog(hwndDlg, IDOK);
				}
				break;

				case IDCANCEL:
				{
					StopAndClean();

					RestoreSetting();
					NotifyChange();
					
					EndDialog(hwndDlg, IDCANCEL);
				}
				break;
			}
		}
		break;
		default:
			break;
	}
	return FALSE;
}
// When main control panel is loaded, LightSensor.cpl will be loaded, then
// CPL_INIT, CPL_GETCOUNT and CPL_NEWINQUIRE will be called, then
// LightSensor.cpl will be unloaded. When the LightSensor icon is double clicked,
// LightSensor.cpl will be loaded, then CPL_INIT and CPL_DBLCLK will be called.

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            RETAILMSG(0, (TEXT("LightSensor: CPL_INIT\r\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            RETAILMSG(0, (TEXT("LightSensor: CPL_GETCOUNT\r\n")));
            return 1;

        case CPL_NEWINQUIRE:
		{
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            RETAILMSG(0, (TEXT("LightSensor: CPL_NEWINQUIRE\r\n")));

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
 				
                LoadString(g_hInstance, IDS_LIGHT_SENSOR_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_LIGHT_SENSOR_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->lData = IDI_LIGHT_SENSOR_ICON;
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LIGHT_SENSOR_ICON));

				RETAILMSG(0, (TEXT("LightSensor: szInfo = \"%s\"\r\n"), lpNewCplInfo->szInfo));
                RETAILMSG(0, (TEXT("LightSensor: szName = \"%s\"\r\n"), lpNewCplInfo->szName));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("LightSensor: CPL_NEWINQUIRE failed\r\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
			
        case CPL_DBLCLK:
		{
			// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
			TCHAR pszTitle[MAX_PATH] = {0};

			LoadString(g_hInstance, IDS_LIGHT_SENSOR_DESCRIPTION, pszTitle,MAX_PATH);
			ASSERT(pszTitle);
            
			RETAILMSG(0, (TEXT("LightSensor: CPL_DBLCLK - %s\r\n"), pszTitle));
			HWND hWnd = FindWindow(L"Dialog", pszTitle);
			if (hWnd)
			{
				SetForegroundWindow(hWnd);
			}
			else
			{
				InitCommonControls();
				if( -1 == DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_LSDLG), hwndCPL, LightSensorDlgProc) )
				{
		            RETAILMSG(1, (TEXT("LightSensor: CPL_DBLCLK err %d\r\n"), GetLastError() ));

				}
			}
		}			
		return 0;
			
        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            RETAILMSG(0, (TEXT("LightSensor: CPL_STOP\r\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            RETAILMSG(0, (TEXT("LightSensor: CPL_EXIT\r\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}
