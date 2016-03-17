// bt_diag.cpp : Defines the entry point for the application.
//

//#define INIT_GUID	1
#include "stdafx.h"
#include <bt_api.h>
#include <bthapi.h>
//#ifdef INIT_GUID
#include <initguid.h>
//#endif
#include <bt_sdp.h>
#include <Sipapi.h>

#include "bt_diag.h"
#include <windows.h>
#include <commctrl.h>

#include "BluetoothHelper.h"

#define MAX_LOADSTRING 100

enum { OP_QUIT = 0, OP_IDLE, OP_INQUIRY, OP_DISCOVERY, OP_PAIR };
// Global Variables:
HINSTANCE			g_hInst;			// current instance
TCHAR				g_ApplTitle[MAX_LOADSTRING] = {0};
int					g_CurrentOperation = -1;
HWND				g_Wnd;
WCHAR				g_szPin[16] = {0};
unsigned char		g_pin[16] = {0};

// Forward declarations of functions included in this code module:
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP

HWND notify = 0;
UINT32 InquiryCallback(MIC_BT_DEVICESLIST *pvList, MIC_BT_DEVICEDETAILS *pDev)
{
	MIC_BT_DEVICESLIST *btdl;
	MIC_BT_DEVICEDETAILS *btd;
	MIC_BT_DEVICESLIST *pvl = pvList;
	do
	{
		if(!pvl)
			break;
		if(!pvl->dev)
		{
			btd = new MIC_BT_DEVICEDETAILS;
			if (btd)
			{
				pvl->dev = btd;
				memcpy(pvl->dev, pDev, sizeof(MIC_BT_DEVICEDETAILS));
				pvl->next = 0;
				TCHAR buf[512];
				wsprintf(buf, L"%s%s(%04x%08x)", pvl->dev->tszName, wcslen(pvl->dev->tszName) == 0 ? L"" : L" ", GET_NAP(pvl->dev->b), GET_SAP(pvl->dev->b));
				int res = SendMessage(notify, LB_ADDSTRING, 0, (LPARAM)buf);
				res = SendMessage(notify, LB_SETITEMDATA, res, (LPARAM)pvl->dev);
				//if(!res)
				//	res = GetLastError();
//				MSG msg ; 
//				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) 
//					DispatchMessage(&msg); 
				break;
			}
		}
		if(pvl->next)
		{
			pvl = pvl->next;
			continue;
		}

		btdl = new MIC_BT_DEVICESLIST;
		btdl->dev = 0;
		btdl->next = 0;
		pvl->next = btdl;
		pvl = pvl->next;
	}while(pvl);

	return 0;
}

UINT32 DiscoveryCallBack(MIC_BTD_SERVICE s, MIC_BT_DEVICEDETAILS *pDev)
{
	TCHAR sz[16] = {0};
	if(s == BTD_MODEM)
		_tcscpy(sz, L"DUN");
	else if(s == BTD_PRINTER)
		_tcscpy(sz, L"PRN");
	else if(s == BTD_ASYNC)
		_tcscpy(sz, L"SER");
	else if(s == BTD_LAP)
		_tcscpy(sz, L"LAP");
	else if(s == BTD_PAN)
		_tcscpy(sz, L"PAN");
	else if(s == BTD_OBEX_FTP)
		_tcscpy(sz, L"OFTP");
	else if(s == BTD_OBEX_OPP)
		_tcscpy(sz, L"OPUSH");
	else if(s == BTD_HEADSET)
		_tcscpy(sz, L"HS");
	else if(s == BTD_HANDSFREE)
		_tcscpy(sz, L"HF");

	TCHAR buf[512];
//	int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
//	SendMessage(notify, LB_GETTEXT, res, (LPARAM)buf);
	if (GetDlgItemText(g_Wnd, IDC_SERVICES, buf, 512) > 0)
	{
		wsprintf(buf, L"%s\r\n%s (%d)", buf, sz, pDev->channels[s]);
		SetDlgItemText(g_Wnd, IDC_SERVICES, buf);
	}

/*	SendMessage(notify, LB_DELETESTRING, res, (LPARAM)buf);
	SendMessage(notify, LB_INSERTSTRING, res, (LPARAM)buf);
	SendMessage(notify, LB_SETCURSEL, res, 0);
	res = SendMessage(notify, LB_SETITEMDATA, res, (LPARAM)pDev);
*/
	return 0;
}


HANDLE g_evOperation = 0;
MIC_BTH_CONTEXT mbc = {0};
MIC_BT_DEVICESLIST mbdl = {0};

void DoOperation(int op)
{
	g_CurrentOperation = op;
	SetEvent(g_evOperation);
}

void SetEnable(int nControlId, bool bEnabled)
{
	HWND hWnd = GetDlgItem(g_Wnd, nControlId);
	if (hWnd)
	{
		EnableWindow(hWnd, bEnabled);
	}
}

DWORD WINAPI BtWorkThread(LPVOID lpvParam)
{
	TCHAR str[MAX_LOADSTRING] = {0};

	int quit = 0, res;
	while (!quit)
	{
		g_CurrentOperation = OP_IDLE;
		DWORD dwStat = WaitForSingleObject(g_evOperation, INFINITE);
		if (dwStat == WAIT_OBJECT_0)
		{
			switch (g_CurrentOperation)
			{
				case (OP_QUIT):
				{
					quit = 1;
					break;
				}

				case (OP_INQUIRY):
				{
					SendMessage(notify, LB_RESETCONTENT, 0, 0);
					wsprintf(str, L"%s (Inquiring)", g_ApplTitle);
					SetWindowText(g_Wnd, str);
					mbc.pBLOB = &mbdl;
					res = MIC_BthInquiry(&mbc, (PFN_BTH_INQUIRY_CALLBACK)InquiryCallback);
					if (!res)
					{
						wsprintf(str, L"%s (Inquiry failed)", g_ApplTitle);
						SetWindowText(g_Wnd, str);
					}
					else
					{
						if (mbc.status == ERROR_OPERATION_ABORTED)
						{
							wsprintf(str, L"%s (Inquiry aborted)", g_ApplTitle);
							SetWindowText(g_Wnd, str);
						}
						else
							SetWindowText(g_Wnd, g_ApplTitle);
					}

					SetDlgItemText(g_Wnd, IDR_INQ, L"Enquiry");
					SetEnable(IDR_INQ, true);
					break;
				}

				case (OP_DISCOVERY):
				{
					res = SendMessage(notify, LB_GETCURSEL, 0, 0);
					if (res == -1)
						break;

					wsprintf(str, L"%s (Discovering)", g_ApplTitle);
					SetWindowText(g_Wnd, str);

					MIC_BT_DEVICEDETAILS *dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
					wsprintf(str, L"%s (%04x%08x) services:", dev->tszName, GET_NAP(dev->b), GET_SAP(dev->b));
					SetDlgItemText(g_Wnd, IDC_SERVICES, str);

					res = MIC_BthDiscoveryServices(dev, (PFN_BTH_DISCOVERY_CALLBACK)DiscoveryCallBack, &mbc);
					if (!res)
					{
						wsprintf(str, L"%s (Discovery failed)", g_ApplTitle);
						SetWindowText(g_Wnd, str);
					}
					else
					{
						if (mbc.status == ERROR_OPERATION_ABORTED)
						{
							wsprintf(str, L"%s (Discovery aborted)", g_ApplTitle);
							SetWindowText(g_Wnd, str);
						}
						else
							SetWindowText(g_Wnd, g_ApplTitle);
					}

					SetDlgItemText(g_Wnd, IDR_DISC, L"Discovery services");
					SetEnable(IDR_DISC, true);
					SetEnable(IDR_INQ, true);
					SetEnable(IDC_PAIR, true);
					SetEnable(IDC_DEV, true);
					break;
				}

				case OP_PAIR:
				{
					res = SendMessage(notify, LB_GETCURSEL, 0, 0);
					if (res == -1)
						break;

					MIC_BT_DEVICEDETAILS *dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
					if (g_pin[0])
					{
						SetEnable(IDR_INQ, false);
						SetEnable(IDC_PAIR, false);
						SetEnable(IDR_DISC, false);
						SetEnable(IDC_DEV, false);

						wsprintf(str, L"%s (Pairing with PIN: %s)", g_ApplTitle, g_szPin);
						SetWindowText(g_Wnd, str);

                        if (!MIC_BthAuthenticate(dev, &mbc, g_pin))
						{
							wsprintf(str, L"%s (Pair failed)", g_ApplTitle);
							SetWindowText(g_Wnd, str);
						}
						else
							SetWindowText(g_Wnd, g_ApplTitle);

						PostMessage(g_Wnd, WM_COMMAND, 0x00010000 + IDC_DEV, 0);
						SetEnable(IDR_INQ, true);
						SetEnable(IDC_PAIR, true);
						SetEnable(IDR_DISC, true);
						SetEnable(IDC_DEV, true);
					}
					break;
				}
			}
		}
	}
	return 1;
}
BOOL CALLBACK Pin(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static MIC_BT_DEVICEDETAILS *dev = 0;
	WCHAR szString[256];
    switch(uMsg)
	{
		case WM_INITDIALOG:
        {
            SetForegroundWindow(hWnd);
            SetWindowText(GetDlgItem (hWnd, IDC_PIN), L"");
            SetFocus(GetDlgItem (hWnd, IDC_PIN));
            
			int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
			dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
            wsprintf(szString, L"Pair with %04x%08x device", GET_NAP(dev->b), GET_SAP(dev->b));
            SetWindowText(hWnd, szString);
			return 0;
        }
		case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
				case IDC_GENERATE:
				{
					int r = rand() % 8999;
					wsprintf(szString, L"%d", (r + 1000) % 9999);
					SetDlgItemText(hWnd, IDC_PIN, szString);
					break;
				}
				case IDC_KEYBD:
				{
					SIPINFO SipInfo = {0};
					memset(&SipInfo, 0, sizeof(SIPINFO));
					SipInfo.cbSize = sizeof(SIPINFO);
					if (SipGetInfo(&SipInfo))
					{
						if (SipInfo.fdwFlags & SIPF_ON)
							SipShowIM(SIPF_OFF); 
						else
							SipShowIM(SIPF_ON);
					}
					break;
				}
				case IDOK:
                {
                    WCHAR *pszPIN = g_szPin;

                    GetWindowText(GetDlgItem (hWnd, IDC_PIN), g_szPin, 16);

                    int cPin = 0;

                    while((*pszPIN) && (cPin < 15))
                        g_pin[cPin++] = (unsigned char)*(pszPIN++);
					
					g_pin[cPin] = 0;
                    
                    EndDialog(hWnd, 0);
					DoOperation(OP_PAIR);
					return 1;
                }
				case IDCANCEL:
				{
                    EndDialog (hWnd, 0);
					return 1;
                }
            }
			break;
        }
    }

    return 0;
}



int WINAPI mainDlgProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
	INT32 res;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			g_Wnd = w;
			LoadString(g_hInst, IDS_APP_TITLE, g_ApplTitle, MAX_LOADSTRING);
			SetWindowText(w, g_ApplTitle);
			SetEnable(IDR_DISC, false);
			SetEnable(IDC_PAIR, false);

			g_evOperation = CreateEvent(0, 0, 0, 0);
			notify = GetDlgItem(w, IDC_DEV);
			mbc.size = sizeof(mbc);
			res = MIC_BthEnableScan(0, &mbc);
			res = MIC_BthEnableScan(1, &mbc);
			CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)BtWorkThread, 0, 0, 0));

			break;
		}

		case WM_CLOSE:
			DoOperation(OP_QUIT);
			MIC_BthCancelInquiry(&mbc);
			MIC_BthDesroyDevicesList(&mbdl, &mbc);
			CloseHandle(g_evOperation);
			EndDialog(w, 1);
			return 1;

		case WM_USER:
			break;

		case WM_COMMAND:
		{
			switch(LOWORD(wp))
			{
				case IDR_INQ:
				{
					if (g_CurrentOperation == OP_INQUIRY)
					{
						MIC_BthCancelInquiry(&mbc);
						SetEnable(IDR_INQ, false);
						SetDlgItemText(w, IDR_INQ, L"Stopping");
					}
					else if (g_CurrentOperation == OP_IDLE)
					{
						SetEnable(IDR_DISC, false);
						SetEnable(IDC_PAIR, false);
						MIC_BthDesroyDevicesList(&mbdl, &mbc);
						DoOperation(OP_INQUIRY);
						SetDlgItemText(w, IDR_INQ, L"Stop");
					}
					break;
				}
				case IDR_DISC:
				{
					if (g_CurrentOperation == OP_DISCOVERY)
					{
						MIC_BthCancelDiscovery(&mbc);
						SetEnable(IDR_DISC, false);
						SetDlgItemText(w, IDR_DISC, L"Stopping");
					}
					else if (g_CurrentOperation == OP_IDLE)
					{
						SetEnable(IDR_INQ, false);
						SetEnable(IDC_PAIR, false);
						SetEnable(IDC_DEV, false);
						DoOperation(OP_DISCOVERY);
						SetDlgItemText(w, IDR_DISC, L"Stop");
					}
					break;
				}
				case IDC_PAIR:
				{
					MIC_BT_DEVICEDETAILS *dev = 0;
					int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
					dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
					if (Mic_BthIsAuthenticated(dev, &mbc))
					{
						MIC_BthRevokePin(dev, &mbc);
						PostMessage(w, WM_COMMAND, 0x00010000 + IDC_DEV, 0);
					}
					else
						DialogBox(g_hInst, MAKEINTRESOURCE(IDD_PIN), g_Wnd, Pin);
					
					break;
				}
				case IDC_DEV:
				{
					MIC_BT_DEVICEDETAILS *dev = 0;
					int res = SendMessage(notify, LB_GETCURSEL, 0, 0);
					if (res != -1 && (HIWORD(wp) == 1 || HIWORD(wp) == 2))
					{
						dev = (MIC_BT_DEVICEDETAILS *)SendMessage(notify, LB_GETITEMDATA, res, 0);
						if (Mic_BthIsAuthenticated(dev, &mbc))
							SetDlgItemText(w, IDC_PAIR, L"Unpair");
						else
							SetDlgItemText(w, IDC_PAIR, L"Pair");

						SetEnable(IDR_DISC, true);
						SetEnable(IDC_PAIR, true);
					}
					break;
				}

				default:
					break;
			}
		}
		default:
			break;
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	return (int) DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), 0, mainDlgProc);
}

#ifndef WIN32_PLATFORM_WFSP
#endif // !WIN32_PLATFORM_WFSP
