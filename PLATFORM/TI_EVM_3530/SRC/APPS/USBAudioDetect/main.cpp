
#include <windows.h>
#include <pnp.h>
#include "resource.h"
#include <initguid.h>

DEFINE_GUID(WAVE_DEVCLASS_USBAUDIO, 
    0x20375f48, 0x7756, 0x40dc, 0x95, 0x20, 0x28, 0xe9, 0x6d, 0xe6, 0xb2, 0xe3);

#define PNP_QUEUE_SIZE	(sizeof(DEVDETAIL) + (MAX_DEVCLASS_NAMELEN * sizeof(TCHAR)))

HWND hListbox = NULL;

void AddToList(TCHAR *text)
{
	if (!hListbox)
		return;

	LRESULT count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)text);
	SendMessage(hListbox, LB_SETCURSEL, count, 0);
	UpdateWindow(hListbox);
}

BOOL ConvertStringToGuid(LPCTSTR GuidString, GUID *Guid )
{
    UINT Data4[8];
    int  Count;
    BOOL Ok = FALSE;
    LPWSTR GuidFormat = L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";

	if (!GuidString || !Guid)
		return Ok;

    __try 
    {

      if (_stscanf_s(GuidString, GuidFormat, &Guid->Data1, 
        &Guid->Data2, &Guid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
        &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == 11) 
      {

          for (Count = 0; Count < (sizeof(Data4) / sizeof(Data4[0])); Count++) 
          {
              Guid->Data4[Count] = (UCHAR) Data4[Count];
          }
      }

      Ok = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) 
    {
        AddToList(TEXT("Exception in convertstringtoguid\r\n"));
    }

    return Ok;
}

BOOL GetRegistryValues(TCHAR *pArray)
{
	BOOL fRetVal = FALSE;
	HKEY hKey;
    DWORD dwType;
    DWORD cbData = 0;

	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Drivers\\USB\\ClientDrivers\\Audio_Class"), 0, 0, &hKey);
    if (ERROR_SUCCESS != lStatus)
		return fRetVal;

    if (RegQueryValueEx(hKey, TEXT("IClass"), NULL, &dwType, NULL, &cbData ) == ERROR_SUCCESS && dwType == REG_MULTI_SZ && cbData > 0)
    {
        LPCTSTR lpValue = new TCHAR[cbData];
        if (lpValue)
        {
            DWORD dwStrLen = cbData;
            if (RegQueryValueEx( hKey, TEXT("IClass"), NULL, &dwType, (LPBYTE)lpValue, &dwStrLen ) == ERROR_SUCCESS && dwType == REG_MULTI_SZ && dwStrLen == cbData )
            {
                fRetVal = TRUE;
				memcpy((LPVOID)pArray, lpValue, dwStrLen);
            }

			delete [] lpValue;
        }

		RegCloseKey(hKey);
    }

    return fRetVal;
}


DWORD WINAPI DetectThread(HANDLE hExitEvent)
{
	static TCHAR text[200] = {0};
	DWORD dwResult;
	DWORD   dwSize;    
    DWORD   dwFlags;

	WAVEINCAPS wic;
	WAVEOUTCAPS woc;

	BYTE DevDetail[PNP_QUEUE_SIZE] = {0};
    DEVDETAIL *pDevDetail = (DEVDETAIL*)DevDetail;
	
	GUID guid = {0};
	HANDLE WaitEvents[2];

	MSGQUEUEOPTIONS msgopts	= {0};
	memset(&msgopts, 0, sizeof(msgopts));
    msgopts.dwSize = sizeof(msgopts);
    msgopts.dwFlags = 0;
    msgopts.dwMaxMessages = 0;                  
    msgopts.cbMaxMessage = sizeof(DevDetail);   
    msgopts.bReadAccess = TRUE;

	WaitEvents[0] = hExitEvent;
	WaitEvents[1] = CreateMsgQueue(NULL, &msgopts);
    if (!WaitEvents[1]) 
    {
		AddToList(TEXT("Can't create message queue"));
        goto exit;
    }

	/*
	if (!GetRegistryValues(text))
	{
        AddToList(TEXT("Can't get USB Audio Class registry values"));
        goto exit;
	}

	if (!ConvertStringToGuid(text, &guid)) 
    {
        AddToList(TEXT("Can't convert string to GUID"));
        goto exit;
    }
	*/
    
    HANDLE hDevNotification = RequestDeviceNotifications(&WAVE_DEVCLASS_USBAUDIO, WaitEvents[1], TRUE);
    if (!hDevNotification) 
    {
        AddToList(TEXT("Can't request device notification"));
        goto exit;
    }

	while (1)
	{
		dwResult = WaitForMultipleObjects(2, &WaitEvents[0], FALSE, INFINITE);
		if (dwResult == WAIT_OBJECT_0 + 1)
		{
			if (ReadMsgQueue(WaitEvents[1], &DevDetail, sizeof(DevDetail), &dwSize, 0, &dwFlags)) 
            {
				wsprintf(text, L"%s is %stached", pDevDetail->szName, pDevDetail->fAttached ? L"at" : L"de");
                AddToList(text);

				if (pDevDetail->fAttached)
				{
					for (UINT i = 0; i < waveInGetNumDevs(); i++)
					{
						if (MMSYSERR_NOERROR == waveInGetDevCaps(i, &wic, sizeof(wic)))
						{
							if (wcsstr(wic.szPname, TEXT("USB Audio")))
							{
								// Audio recording device
								wsprintf(text, L"%s (Device ID: %d) added", wic.szPname, i);
								AddToList(text);
							}
						}
					}

					for (UINT i = 0; i < waveOutGetNumDevs(); i++)
					{
						if (MMSYSERR_NOERROR == waveOutGetDevCaps(i, &woc, sizeof(woc)))
						{
							if (wcsstr(woc.szPname, TEXT("USB Audio")))
							{
								// Audio playback device
								wsprintf(text, L"%s (Device ID: %d) added", woc.szPname, i);
								AddToList(text);
							}
						}
					}
				}
				else
				{
					AddToList(TEXT("USB Audio Class Input/Output removed"));
				}
            }
		}
		else
			break;
	}

exit:;
	if (hDevNotification)
		StopDeviceNotifications(hDevNotification);

	if (WaitEvents[1])
		CloseMsgQueue(WaitEvents[1]);

	return 0;
}

INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hExitEvent = NULL;
	static HANDLE hDetectThread = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
			{
				hListbox = GetDlgItem(hDlg, IDC_LIST);
				hExitEvent = CreateEvent(0, 1, 0, 0);
				hDetectThread = CreateThread(NULL, 0, DetectThread, (void *)hExitEvent, 0, NULL);
			}
			break;

        case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
			}
			break;

        case WM_CLOSE:
            EndDialog(hDlg, message);
            return (INT_PTR)TRUE;

		case WM_DESTROY:
			SetEvent(hExitEvent);
			WaitForSingleObject(hDetectThread, INFINITE);
			CloseHandle(hDetectThread);
			CloseHandle(hExitEvent);
			break;
    }

    return (INT_PTR)FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)WndProc);
}