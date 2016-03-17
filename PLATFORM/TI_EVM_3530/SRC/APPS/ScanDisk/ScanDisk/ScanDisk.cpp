// ScanDisk.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ScanDisk.h"
#include <windows.h>
#include <commctrl.h>

#define MAX_LOADSTRING 100

//void WINAPIV NKDbgPrintfW(LPCWSTR lpszFmt, ...);
//#define RETAILMSG(cond,printf_exp)   \
//   ((cond)?(NKDbgPrintfW printf_exp),1:0)

//#define DEBUG_PRINT_ERR(printf_exp) (NKDbgPrintfW printf_exp)
//#define DEBUG_PRINT_ERR(printf_exp) RETAILMSG(1, (TEXT printf_exp))
//#define DEBUG_PRINT_ERR(printf_exp) RETAILMSG(1, printf_exp)
//#define DEBUG_PRINT(printf_exp) RETAILMSG(1, printf_exp)
//#define DEBUG_PRINT(printf_exp) RETAILMSG(1, (TEXT(printf_exp)))
//#define DEBUG_PRINT(printf_exp) RETAILMSG(1, printf_exp)
//#define DEBUG_PRINT(printf_exp) {   char buf[1024]; \
//									WCHAR wbuf[1024]; \
//									wsprintf(wbuf, TEXT(printf_exp)); \
//									RETAILMSG(1, (wbuf));}

// Global Variables:
HINSTANCE			g_hInst;			// current instance
#ifdef SHELL_AYGSHELL
HWND				g_hWndMenuBar;		// menu bar handle
#else // SHELL_AYGSHELL
HWND				g_hWndCommandBar;	// command bar handle
#endif // SHELL_AYGSHELL

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
BOOL			InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#ifndef WIN32_PLATFORM_WFSP
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
#endif // !WIN32_PLATFORM_WFSP


#define RK_SCAN        TEXT("Software\\Scan")
#define RV_RES        TEXT("Res")  //are we in DST?


BOOL	g_RebootRequired = 0;
///
BOOL  SetKey(DWORD Value)
{
	DWORD dwType;
    HKEY hKey = NULL;

	BOOL	ret = 0;

	RETAILMSG(1, (L"Error\r\n"));
	RETAILMSG(1, (L"Error %X \r\n", ret));
//	DEBUG_PRINT((TEXT("Error %X \r\n"), ret));
//	DEBUG_PRINT_ERR((L"Error\r\n"));
//	DEBUG_PRINT_ERR((L"Error %X\r\n", ret));
    /* Read RK_SCAN registry key, If not exist - create it */
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_SCAN, 0, 0, &hKey))
	{
		if( ERROR_SUCCESS != RegCreateKeyEx( HKEY_LOCAL_MACHINE, RK_SCAN, 0, NULL, 0, 0, NULL, &hKey, &dwType ) )
		{
			RegCloseKey(hKey);
			return(FALSE);
		}
	}

	dwType = Value;
    if( ERROR_SUCCESS == RegSetValueEx(hKey, RV_RES, 0, REG_DWORD, (BYTE *)&dwType, sizeof(dwType)) )
	{
		ret = 1;
	}

    RegCloseKey(hKey);
    return(TRUE);

}


/////////////////////////////////////////////////////////////////////////////////////////////
void ScanProgress(DWORD per)
{
	RETAILMSG(1, (_T("Scan: %d%% complete\r\n"), per));
}
///must be used for scan without fixing
int ScanMessage(LPTSTR szMessage, LPTSTR szCaption, BOOL fYesNo)
{
	RETAILMSG(1, (_T("ScanMessage: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo));
	return 0;
}
int ScanFixMessage(LPTSTR szMessage, LPTSTR szCaption, BOOL fYesNo)
{
	RETAILMSG(1, (_T("ScanMessage: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo));

	return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

int	ScanPart()
{
	HANDLE h, hs, hp;
	DWORD ret;
    STOREINFO si = {0};
	PARTINFO  pi = {0};


//	_tprintf(TEXT("Enamerate all stores\r\n"));
	si.cbSize = sizeof(STOREINFO);
    h = FindFirstStore(&si);

	if((HANDLE)-1 != h)
	{
		do
		{
//			_tprintf(TEXT("%d partition(s) is(are) found on '%s'\r\n"),si.dwPartitionCount, si.szDeviceName);
//			if(0 != _tcsnicmp(si.szDeviceName, v_name, vn_len))
//				continue;
//			if(si.dwPartitionCount < 2)
//				break;

			hs = OpenStore(si.szDeviceName);
			if((HANDLE)-1 == hs)
				continue;

//			_tprintf(TEXT("Enamerate all partition on '%s'\r\n"), si.szDeviceName);
			pi.cbSize = sizeof(pi);
			hp = FindFirstPartition(hs, &pi);
			if((HANDLE)-1 != h)
			{
				do
				{
//					_tprintf(TEXT("'%s' partition is found\r\n"), pi.szPartitionName);
//					_tprintf(TEXT("\t'%s' partition of %d sectors\r\n"),
//								(pi.bPartType == PART_BOOTSECTION)?TEXT("Binary"):
//								(pi.bPartType == PART_DOS32)?TEXT("FAT 32"):TEXT("Other"), pi.snNumSectors);
					HANDLE h = OpenPartition(hs, pi.szPartitionName);
					if((HANDLE)-1 != h)
					{
						DISK_INFO di = {0};
						
						ret = DeviceIoControl(h, DISK_IOCTL_GETINFO, &di, sizeof(di), &di, sizeof(di), &ret, 0);

					///get fatutil funcsions/////////////////////////////////
					//	PFN_DEFRAGVOLUME	pfnDefragVolume;
						PFN_SCANVOLUMEEX	pfnScanVolumeEx;
					//	PFN_SCANVOLUME		pfnScanVolume;

						HMODULE hm = LoadLibrary(TEXT("fatutil.dll"));
						if(hm)
						{
							pfnScanVolumeEx = (PFN_SCANVOLUMEEX)GetProcAddress(hm, TEXT("ScanVolumeEx"));
							//	pfnDefragVolume = (PFN_DEFRAGVOLUME)GetProcAddress(hm, TEXT("DefragVolume"));
							//	pfnScanVolume = (PFN_SCANVOLUME)GetProcAddress(hm, TEXT("ScanVolume"));
		
							if(pfnScanVolumeEx)//|| !pfnDefragVolume )
							{
								BOOL	err = 1;

								DEFRAG_OPTIONS dfgo;
								dfgo.dwFatToUse = 1;
								dfgo.dwFlags	= FATUTIL_SCAN_VERIFY_FIX;

								//first scan without error fixing
								SCAN_PARAMS		scp = {0};
								scp.cbSize			= sizeof(SCAN_PARAMS);
								scp.so.dwFatToUse	= 1;
								scp.so.dwFlags		= FATUTIL_DISABLE_MOUNT_CHK | FATUTIL_SCAN_VERIFY_FIX;
								scp.pfnProgress		= ScanProgress;
								scp.pfnMessage		= ScanMessage;
								
								__try
								{
									err = pfnScanVolumeEx(hp, &scp );
								}
								__except(1)
								{
									err = 1;
								}
							}
						}
						CloseHandle(h);
					}
				}while(FindNextPartition(hp, &pi) && (si.dwPartitionCount > 1));
				FindClosePartition(hp);
			}

			CloseHandle(hs);
		}while(FindNextStore(h, &si));

		FindCloseStore(h);
	}

	return 0;
}




int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{

	ScanPart();
	return 0;
}

