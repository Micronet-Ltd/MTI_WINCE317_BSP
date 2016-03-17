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


#include "MicrophoneCpl.h"
#include <creg.hxx>
#include <TrackBar.h>
#include <wave_audio.h>

// Returns the number of characters in an expression.
#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))

#define NUM_OF_SLIDERS			2

BOOL CALLBACK MicrophoneCplDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInstance = NULL;

class CAdvMic
{
public:
	CAdvMic() //: m_DGain(0), m_AGain(0)
	{
		arr[0].InitBase(	MIN_IN_MAIN_MIC_DGAIN,
							DGAIN_TICK_INTERVAL * MAX_IN_MAIN_MIC_DGAIN,
							DGAIN_TICK_INTERVAL,
							4,	//TICK_FREQ;
							1,	//LINE_SIZE;
							1	//PAGE_SIZE;
						);
		arr[1].InitBase(	MIN_IN_MAIN_MIC_AGAIN,
							AGAIN_TICK_INTERVAL * MAX_IN_MAIN_MIC_AGAIN,
							AGAIN_TICK_INTERVAL,
							1,	//TICK_FREQ;
							1,	//LINE_SIZE;
							1	//PAGE_SIZE;
						);
	}
	void Init( HWND hDlg )
	{
		DWORD DGain(0), AGain(0);

		GetWavFromReg( DGain, AGain );
		
		arr[0].Init( hDlg, IDC_DGAIN, DDb2Val(DGain)	);
		arr[1].Init( hDlg, IDC_AGAIN, ADb2Val(AGain)	);

		//m_DGain = DGain;
		//m_AGain = AGain;
	}
	void UpdateWavSettings()
	{
		DWORD Dval(0), Aval(0);

		Dval = Val2DDb( DGAIN_TICK_INTERVAL * arr[0].GetPos(TRUE)	);
		Aval = Val2ADb( AGAIN_TICK_INTERVAL * arr[1].GetPos(TRUE)	);
		
		//if chenged
		//if(	m_DGain != Dval	||
		//		m_AGain != Aval	)
		{
			if( SetWavToReg(Dval, Aval) )
			{
				NotifyChanged();
				//m_DGain = Dval;
				//m_AGain = Aval;
			}
		}
	}


	void NotifyChanged()
	{
		WAVEFORMATEX wfx = {0};
//		wfx.cbSize = 0;
		wfx.wFormatTag		= WAVE_FORMAT_PCM;
		wfx.wBitsPerSample	= 16;
		wfx.nSamplesPerSec	= 44100;
		wfx.nChannels		= 1;
		wfx.nBlockAlign		= wfx.nChannels * wfx.wBitsPerSample / 8;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
		
		MMRESULT mmres;
		HWAVEOUT hwo = 0;
		UINT	devId = 0;
		for(devId = 0; devId < waveOutGetNumDevs(); devId++)
		{
			mmres = waveOutOpen(&hwo, devId, &wfx, 0, 0, CALLBACK_NULL);
			if(mmres == MMSYSERR_NOERROR)
				break;
		}
		RETAILMSG(0,(_T("waveOutOpen: mmres %d, devId %d hwo %x\r\n"),mmres, devId, hwo));
		if( mmres == MMSYSERR_NOERROR)
		{
			mmres = waveOutMessage(hwo, WPDM_PRIVATE_UPDATE_REG_VAL, 0, 0);
			RETAILMSG(0,(_T("waveOutMessage: mmres %d, hwo %x\r\n"),mmres, hwo));
				
			mmres = waveOutClose(hwo);
		}
	}

private:

	DWORD Val2DDb(DWORD val)
	{
		return val/1;
	}
	DWORD Val2ADb(DWORD val)
	{
		return val/6;
	}
	DWORD DDb2Val(DWORD dbval )
	{
		return dbval * 1;
	}
	DWORD ADb2Val(DWORD dbval )
	{
		return dbval * 6;
	}

	BOOL GetWavFromReg( DWORD& DGain, DWORD& AGain  )
	{
		CReg reg;
		if( reg.Open( HKEY_LOCAL_MACHINE, WAV_DEV_REG_KEY ) )
		{
			DGain = reg.ValueDW(WAV_DEV_REG_MIC_DGAIN,	0xF);
			AGain = reg.ValueDW(WAV_DEV_REG_MIC_AGAIN,	0x4);

			RETAILMSG(0,(_T("MicrophoneCpl::GetWavFromReg %d, %d\r\n"), DGain, AGain)); //temp!!!
			return 1;
		}
		return 0;
	}

	BOOL	SetWavToReg(DWORD DGain, DWORD AGain )
	{
		BOOL res = 0;
		CReg reg;
		if( reg.OpenOrCreateRegKey( HKEY_LOCAL_MACHINE, WAV_DEV_REG_KEY ) )
		{
			if( res = reg.SetDW(WAV_DEV_REG_MIC_DGAIN, DGain) )
				res = reg.SetDW(WAV_DEV_REG_MIC_AGAIN, AGain );
			
			RETAILMSG(0,(_T("MicrophoneCpl::SetWavToReg %d, %d res %d\r\n"), DGain, AGain, res )); //temp!!!
		}
		return res;
	}

	CTrackbar	arr[NUM_OF_SLIDERS];
	//DWORD		m_DGain;
	//DWORD		m_AGain;
};


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

BOOL CALLBACK MicrophoneCplDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
 	static CAdvMic* pAdvMic = 0;

	switch (message)
    {
	    case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MIC_ICON));
			if (hIcon)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
		    if( pAdvMic = new CAdvMic() )
				pAdvMic->Init(hDlg); 
		}
	    return 1;

//   	case WM_DRAWITEM:
//   		DrawCPLButton((LPDRAWITEMSTRUCT)lParam, (LOWORD(wParam)==IDC_TEST) ? IDB_RIGHT : IDB_STOP);
   		break;

 		case WM_HSCROLL:            // track bar message
            switch LOWORD(wParam)
            {
				//case TB_ENDTRACK:
				//	s_TrackArr.PosChanged((HWND)lParam); //RecheckPos( hwndPage, (HWND)lParam );
				//break;

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
			switch (LOWORD(wParam))
			{
				case IDOK:
				{
					pAdvMic->UpdateWavSettings();

					if( Button_GetCheck( GetDlgItem( hDlg, IDC_MIC_SAVE )  ) )
					{
						RegFlushKey(HKEY_LOCAL_MACHINE);
					}
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
   			delete pAdvMic;
			RETAILMSG(0,(_T("MicrophoneCpl: destroid\r\n"))); //temp!!!
   			break;
    }
    return (FALSE);
}


// When main control panel is loaded, MicrophoneCpl.cpl will be loaded, then
// CPL_INIT, CPL_GETCOUNT and CPL_NEWINQUIRE will be called, then
// MicrophoneCpl.cpl will be unloaded. When the MicrophoneCpl icon is double clicked,
// MicrophoneCpl.cpl will be loaded, then CPL_INIT and CPL_DBLCLK will be called.

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_INIT\r\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_GETCOUNT\r\n")));
            return 1;

        case CPL_NEWINQUIRE:
		{
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_NEWINQUIRE\r\n")));

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
 				
                LoadString(g_hInstance, IDS_MIC_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_MIC_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->lData = IDI_MIC_ICON;
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MIC_ICON));

				RETAILMSG(0, (TEXT("MicrophoneCpl: szInfo = \"%s\"\r\n"), lpNewCplInfo->szInfo));
                RETAILMSG(0, (TEXT("MicrophoneCpl: szName = \"%s\"\r\n"), lpNewCplInfo->szName));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("MicrophoneCpl: CPL_NEWINQUIRE failed\r\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
			
        case CPL_DBLCLK:
		{
			// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
			TCHAR pszTitle[MAX_PATH] = {0};

			LoadString(g_hInstance, IDS_MIC_DESCRIPTION, pszTitle,MAX_PATH);
			ASSERT(pszTitle);
            
			RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_DBLCLK - %s\r\n"), pszTitle));
			HWND hWnd = FindWindow(L"Dialog", pszTitle);
			if (hWnd)
			{
				SetForegroundWindow(hWnd);
			}
			else
			{
				InitCommonControls();
				if( -1 == DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_MICDLG), hwndCPL, MicrophoneCplDlgProc) )
				{
		            RETAILMSG(1, (TEXT("MicrophoneCpl: CPL_DBLCLK err %d\r\n"), GetLastError() ));

				}
			}
		}			
		return 0;
			
        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_STOP\r\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            RETAILMSG(0, (TEXT("MicrophoneCpl: CPL_EXIT\r\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}
