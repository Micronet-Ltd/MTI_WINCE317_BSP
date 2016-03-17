// PeripheralsCPL.cpp : Defines the entry point for the DLL application.
//

#include "GPSCpl.h"
#include "..\\..\\SDK\\inc\\gps_api.h"

// Returns the number of characters in an expression.
#define lengthof(exp) ((sizeof((exp)))/sizeof((*(exp))))

HINSTANCE g_hInstance = NULL;
int g_GPSStatus = 0;
TCHAR g_szStatus[MAX_PATH] = {0};


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


BOOL CALLBACK GPSCplDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	static HCURSOR hcw;
	static int def_pwr = 0;

	switch (message)
    {
	    case WM_INITDIALOG:
		{
			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_GPS));
		
			if (hIcon)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);

			hcw = LoadCursor(g_hInstance, IDC_WAIT);

			// Check gps power status
            int res = MIC_GPSGetPowerStatus(&g_GPSStatus);

            if (res == 0)
            {
				RETAILMSG(1, (TEXT("MIC_GSMGetPowerStatus error\r\n")));
				
				EnableWindow(GetDlgItem(hDlg, IDC_GPS_ON), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SAVE), FALSE);
				ShowWindow(GetDlgItem(hDlg, IDC_ABSENT), SW_SHOW);
				SetDlgItemText(hDlg, IDC_STATUS, L"GPS device is absent");
                return 1;
            }

			SendDlgItemMessage(hDlg, IDC_GPS_ON, BM_SETCHECK, g_GPSStatus ? BST_CHECKED : BST_UNCHECKED, 0);

			def_pwr = g_GPSStatus;
			if(g_GPSStatus)
				ShowWindow(GetDlgItem(hDlg, IDC_ON), SW_SHOW); 
			else
			{
				ShowWindow(GetDlgItem(hDlg, IDC_OFF), SW_SHOW);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SAVE), FALSE);
			}
		}
	    return 1;

 		
		case WM_COMMAND:      
		{
			switch (LOWORD(wParam))
			{
				case IDC_GPS_ON:
				{
					if(HIWORD(wParam) == BN_CLICKED)
					{
						int res = 0;
						HCURSOR hCurrentCursor;
						hCurrentCursor = SetCursor(hcw);

						// Check gps power status
						res = MIC_GPSGetPowerStatus(&g_GPSStatus);

						if (res == 0)
						{
							RETAILMSG(1, (TEXT("MIC_GSMGetPowerStatus error\r\n")));
							SetDlgItemText(hDlg, IDC_STATUS, L"MIC_GPSGetPowerStatus error");
							
							ShowWindow(GetDlgItem(hDlg, IDC_ON), SW_HIDE); 
							ShowWindow(GetDlgItem(hDlg, IDC_OFF), SW_HIDE); 
							ShowWindow(GetDlgItem(hDlg, IDC_ABSENT), SW_SHOW);
							
							EnableWindow(GetDlgItem(hDlg, IDC_GPS_ON), FALSE);
							return 1;
						}

						if (Button_GetCheck(GetDlgItem(hDlg, IDC_GPS_ON)))
						{
							if(!g_GPSStatus)
								res = MIC_GPSPower(1);

							if(res != 0)
							{
								// Check gps power status
								res = MIC_GPSGetPowerStatus(&g_GPSStatus);

								if (res == 0)
								{
									RETAILMSG(1, (TEXT("MIC_GSMGetPowerStatus error\r\n")));
									SetDlgItemText(hDlg, IDC_STATUS, L"MIC_GPSGetPowerStatus error");

									ShowWindow(GetDlgItem(hDlg, IDC_ON), SW_HIDE); 
									ShowWindow(GetDlgItem(hDlg, IDC_OFF), SW_HIDE); 
									ShowWindow(GetDlgItem(hDlg, IDC_ABSENT), SW_SHOW);
									
									EnableWindow(GetDlgItem(hDlg, IDC_GPS_ON), FALSE);
									return 1;
								}

								EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SAVE), g_GPSStatus);
								if(g_GPSStatus)
								{
									ShowWindow(GetDlgItem(hDlg, IDC_ON), SW_SHOW); 
									ShowWindow(GetDlgItem(hDlg, IDC_OFF), SW_HIDE); 
								}
								else
								{
									SendDlgItemMessage(hDlg, IDC_GPS_ON, BM_SETCHECK, BST_UNCHECKED, 0);
								}
							}
						}
						else
						{
							if(g_GPSStatus)
								res = MIC_GPSPower(0);
							
							if(res != 0)
							{
								// Check gps power status
								res = MIC_GPSGetPowerStatus(&g_GPSStatus);

								if (res == 0)
								{
									RETAILMSG(1, (TEXT("MIC_GSMGetPowerStatus error\r\n")));
									SetDlgItemText(hDlg, IDC_STATUS, L"MIC_GPSGetPowerStatus error");
									return 1;
								}

								EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SAVE), g_GPSStatus);
								if(!g_GPSStatus)
								{
									ShowWindow(GetDlgItem(hDlg, IDC_ON), SW_HIDE); 
									ShowWindow(GetDlgItem(hDlg, IDC_OFF), SW_SHOW); 
								}
								else
								{
									SendDlgItemMessage(hDlg, IDC_GPS_ON, BM_SETCHECK, BST_CHECKED, 0);
								}
							}
						}

						if (res == 0)
						{
							RETAILMSG(1, (TEXT("MIC_GPSPower error\r\n")));
						}	

						SetCursor(hCurrentCursor);
					} // BM_CLICKED
				}	
				break;

				case IDOK:
					{
						DWORD dwStatus;

						dwStatus=SendDlgItemMessage(hDlg, IDC_CHECK_SAVE, BM_GETCHECK, 0, 0);
						if(dwStatus==BST_CHECKED)
						 dwStatus=1;
						else dwStatus=0;
						
						HKEY hKey;
						if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Drivers\\BuiltIn\\GPS"), 0, 0, &hKey)==ERROR_SUCCESS)
						{
							RegSetValueEx(hKey,TEXT("StartupOn"), 0, REG_DWORD, (BYTE *)&dwStatus, sizeof(dwStatus));
							RegFlushKey(hKey);
							RegCloseKey(hKey);
						}
						EndDialog(hDlg, IDOK);
					}
					break;
				case IDCANCEL:
				{
					if(def_pwr != g_GPSStatus)
						MIC_GPSPower(def_pwr);
					EndDialog(hDlg, IDCANCEL);
				}
				break;
			}
		}	
		break;

   		case WM_DESTROY:
			RETAILMSG(0,(_T("GPSCpl: destroied\r\n")));
   			break;
    }
    return (FALSE);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// The entry point to the Control Panel application.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL,
                  UINT message, LPARAM lParam1, LPARAM lParam2)
{
  switch (message)
  {
    case CPL_INIT:
      // Perform global initializations, especially memory
      // allocations, here.
      // Return 1 for success or 0 for failure.
      // Control Panel does not load if failure is returned.
      return 1;

    case CPL_GETCOUNT:
      // The number of actions supported by this Control
      // Panel application.
      return 1;

    case CPL_NEWINQUIRE:
      // This message is sent once for each dialog box, as
      // determined by the value returned from CPL_GETCOUNT.
      // lParam1 is the 0-based index of the dialog box.
      // lParam2 is a pointer to the NEWCPLINFO structure.
    {
      ASSERT(0 == lParam1);
      ASSERT(lParam2);

      NEWCPLINFO* lpNewCplInfo = (NEWCPLINFO *) lParam2;
      if (lpNewCplInfo)
      {
         lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
         lpNewCplInfo->dwFlags = 0;
         lpNewCplInfo->dwHelpContext = 0;
         lpNewCplInfo->lData = IDI_GPS;

         // The large icon for this application. Do not free this 
         // HICON; it is freed by the Control Panel infrastructure.
         lpNewCplInfo->hIcon = LoadIcon(g_hInstance,
                                  MAKEINTRESOURCE(IDI_GPS));

          LoadString(g_hInstance, IDS_APP_TITLE, lpNewCplInfo->szName,
                     lengthof(lpNewCplInfo->szName));
          LoadString(g_hInstance, IDS_INFO, lpNewCplInfo->szInfo,
                     lengthof(lpNewCplInfo->szInfo));
 //       LoadString(g_hInstance, IDS_INFO, lpNewCplInfo->szInfo,
 //                    lengthof(lpNewCplInfo->szInfo));
          _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
          return 0;
      }
      return 1;  // Nonzero value means CPlApplet failed.
    }

    case CPL_DBLCLK:
    {
      // The user has double-clicked the icon for the
      // dialog box in lParam1 (zero-based).
		TCHAR pszTitle[MAX_PATH] = {0};

		LoadString(g_hInstance, IDS_INFO, pszTitle,MAX_PATH);
		ASSERT(pszTitle);
            
		RETAILMSG(1, (TEXT("GPSCpl: CPL_DBLCLK - %s\r\n"), pszTitle));
		HWND hWnd = FindWindow(L"Dialog", pszTitle);
			
		if (hWnd)
		{
			SetForegroundWindow(hWnd);
		}
		else
		{
			if( -1 == DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_GPSDLG), 0, GPSCplDlgProc) )
			{
				RETAILMSG(0, (TEXT("GPSCpl: CPL_DBLCLK err %d\r\n"), GetLastError() ));
				return 1;     // CPlApplet failed.
			}
		}
		return 0;
    }

    case CPL_STOP:
      // Called once for each dialog box. Used for cleanup.
    case CPL_EXIT:
      // Called only once for the application. Used for cleanup.
    default:
      return 0;
  }

  return 1;  // CPlApplet failed.
  }  // CPlApplet

