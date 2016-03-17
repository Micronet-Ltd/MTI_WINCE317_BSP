#include <windows.h>
#include <ndis.h>
#include <Nuiouser.h>
#include <winsock2.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <pm.h>
#include <ndispwr.h>
#include <Service.h>
#include <bt_api.h>
extern "C"
{
	#include <cu_osapi.h>
	#include <convert.h>
	#include <console.h>
	#include <cu_cmd.h>

	void Console_Stop(THandle hConsole){}
	void g_tester_send_event(U8 event_index){}
	void ProcessLoggerMessage(PU8 data, U32 len){}
	void user_main(){}
}

#define TIWLAN_DRV_NAME "tiwlan0"
#define NVS_FILE_PATH	L"\\windows\\nvs_map.bin"

typedef struct CuCmd_t
{
    THandle                 hCuWext;
    THandle                 hCuCommon;
    THandle                 hConsole;
    THandle                 hIpcEvent;
    THandle                 hWpaCore;
    
    U32                     isDeviceRunning;

    scan_Params_t             appScanParams;
    TPeriodicScanParams     tPeriodicAppScanParams;
    scan_Policy_t             scanPolicy;
    
} CuCmd_t;

#define WLAN_DEVICE_NAME 			(const char*) (TEXT("TIWLNAPI1"))
BOOL DoNdisIOControl(DWORD dwCommand, LPVOID pInBuffer, DWORD cbInBuffer, LPVOID pOutBuffer, DWORD *pcbOutBuffer)
{
	HANDLE hNdis;
	BOOL fResult = FALSE;

	hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                        0, NULL);

	if(INVALID_HANDLE_VALUE != hNdis)
	{
		fResult = DeviceIoControl(hNdis, dwCommand, pInBuffer, cbInBuffer,
								  pOutBuffer, (pcbOutBuffer ? *pcbOutBuffer : 0),
								  pcbOutBuffer, NULL);
		CloseHandle(hNdis);
	}
	else
	{
		RETAILMSG(1, (TEXT("DoNdisIOControl: DoNdisIOControl %d\r\n"), GetLastError()));
	}


	return fResult;
}

BOOL WL_GetStatus(void)
{
	WSADATA WsaData;
	ULONG                    ulBufferLength, ulFlags;
	DWORD                    dwError = 1;
	PVOID					 pBuffer = NULL;
	PIP_ADAPTER_ADDRESSES    pAdapterAddresses = NULL;
	DWORD GetAdaptAddRet = 0;
	DWORD timeout;

	if(WSAStartup(MAKEWORD(2,2), &WsaData) == 0)
	{
		ulFlags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
  
		timeout = GetTickCount() + 5000;
		while(dwError && timeout > GetTickCount())
		{	
		    GetAdaptAddRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, NULL, NULL, &ulBufferLength);
    
			pBuffer = malloc(ulBufferLength);
		    pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)pBuffer;
	       
		    if(NULL == pAdapterAddresses)
		    {
			    RETAILMSG(1, (TEXT("WL_GetStatus: Insufficient Memory\r\n")));
				    break;
		    }
    
		    GetAdaptAddRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, NULL, pAdapterAddresses, &ulBufferLength);
  
		    if(GetAdaptAddRet == ERROR_SUCCESS)
		    {
			    while (pAdapterAddresses != NULL)
			    {
				    if(strncmp(pAdapterAddresses->AdapterName, WLAN_DEVICE_NAME, strlen(WLAN_DEVICE_NAME)) == 0)
				    {
					    RETAILMSG(1, (TEXT("WL_GetStatus: TIWLNAPI1 is running\r\n")));
					    dwError = 0;
					    break;
				    }
				    pAdapterAddresses = pAdapterAddresses->Next;
			    }
    
			    RETAILMSG(0, (TEXT("WL_GetStatus: TIWLNAPI1 isn't running\r\n")));
		    }
		    else
		    {
			    RETAILMSG(1, (TEXT("WL_GetStatus: Can't get Adaptaters info\r\n")));
		    }

			free(pBuffer);
		}//~while(dwError && timeout > GetTickCount())

		WSACleanup();
	}//~if(WSAStartup(MAKEWORD(2,2), &WsaData) == 0)

	return (dwError == 0);
}

//BOOL WL_TurnOn(int bind)
//{
//	DWORD dwErr = S_OK;
//    BOOL bRet = FALSE;
//	HANDLE hNdisPwr;
//	NDISPWR_SAVEPOWERSTATE SavePowerState;
//	TCHAR szName[MAX_PATH];
//	TCHAR multiSz[257];
//	int nChars;        
//
//
//	TCHAR *WlanDevName = L"TIWLNAPI1";
//
//    DWORD cbBuffer = sizeof(multiSz);
//
//	hNdisPwr = CreateFile(
//				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
//				0x00,											//	Desired access.
//				0x00,											//	Share Mode.
//				NULL,											//	Security Attr
//				OPEN_EXISTING,									//	Creation Disposition.
//				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
//				(HANDLE)INVALID_HANDLE_VALUE);	
//
//	if(hNdisPwr != INVALID_HANDLE_VALUE)
//	{
//		SavePowerState.pwcAdapterName = WlanDevName;
//		SavePowerState.CePowerState   = PwrDeviceUnspecified;
//		
//		bRet = DeviceIoControl(
//					hNdisPwr,
//					IOCTL_NPW_SAVE_POWER_STATE,
//					&SavePowerState,
//					sizeof(NDISPWR_SAVEPOWERSTATE),
//					NULL,
//					0x00,
//					NULL,
//					NULL);
//
//		dwErr = GetLastError();
//
//		CloseHandle(hNdisPwr);
//	}
//
//	if(!bind)
//		return 1;
//
//	nChars = _sntprintf(
//					szName, 
//					MAX_PATH-1, 
//					_T("%s\\%s"), 
//					PMCLASS_NDIS_MINIPORT, 
//					WlanDevName);
//
//	szName[MAX_PATH-1]=0;
//
//	RETAILMSG(1, (TEXT("WL_TurnOn: set unspecified state to %s\r\n"), szName));
//	SetDevicePower(szName, POWER_NAME, PwrDeviceUnspecified);
//
//	
//    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);
//
//	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null
//
//	RETAILMSG(1, (TEXT("WL_TurnOn: bind %s adapter\r\n"), multiSz));
//	bRet = DoNdisIOControl(
//					IOCTL_NDIS_BIND_ADAPTER, 
//					multiSz,
//					(_tcslen(multiSz)+2) * sizeof(TCHAR),
//					NULL, 
//					NULL);   
//
//	return bRet;
//}
//
//BOOL WL_TurnOff(int unbind)
//{
//	DWORD dwErr = S_OK;
//    BOOL bRet = FALSE;
//	HANDLE hNdisPwr;
//	NDISPWR_SAVEPOWERSTATE SavePowerState;
//	TCHAR szName[MAX_PATH];
//	TCHAR multiSz[257];
//	int nChars;        
//
//
//	TCHAR *WlanDevName = L"TIWLNAPI1";
//
//    DWORD       cbBuffer = sizeof(multiSz);
//
//	hNdisPwr = CreateFile(
//				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
//				0x00,											//	Desired access.
//				0x00,											//	Share Mode.
//				NULL,											//	Security Attr
//				OPEN_EXISTING,									//	Creation Disposition.
//				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
//				(HANDLE)INVALID_HANDLE_VALUE);	
//
//	if (hNdisPwr != INVALID_HANDLE_VALUE)
//	{
//		SavePowerState.pwcAdapterName = WlanDevName;
//		SavePowerState.CePowerState   = D4;
//		
//		bRet = DeviceIoControl(
//					hNdisPwr,
//					IOCTL_NPW_SAVE_POWER_STATE,
//					&SavePowerState,
//					sizeof(NDISPWR_SAVEPOWERSTATE),
//					NULL,
//					0x00,
//					NULL,
//					NULL);
//
//		dwErr = GetLastError();
//
//		CloseHandle(hNdisPwr);
//	}
//
//	if(!unbind)
//		return 1;
//
//	nChars = _sntprintf(
//					szName, 
//					MAX_PATH-1, 
//					_T("%s\\%s"), 
//					PMCLASS_NDIS_MINIPORT, 
//					WlanDevName);
//
//	szName[MAX_PATH-1]=0;
//
//	RETAILMSG(1, (TEXT("WL_TurnOff: set D4 state to %s\r\n"), szName));
//	SetDevicePower(szName, POWER_NAME, D4);
//
//	
//    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);
//
//	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null
//
//	RETAILMSG(1, (TEXT("WL_TurnOff: unbind %s adapter\r\n"), multiSz));
//	bRet = DoNdisIOControl(
//					IOCTL_NDIS_UNBIND_ADAPTER, 
//					multiSz,
//					(_tcslen(multiSz)+2) * sizeof(TCHAR),
//					NULL, 
//					NULL);   
//
//	return bRet;
//}
//
BOOL WL_SaveStartupState(UINT32 save)
{
	HKEY	hKey = 0;
	DWORD	Size;
	BOOL	en = 0;

	long l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Comm\\tiwlnapi1\\Parms"), 0, 0, &hKey);
	if(ERROR_SUCCESS == l)
	{
        Size = sizeof(DWORD);

		l = RegSetValueEx(hKey, L"EnableOnStartup", 0, REG_DWORD, (UINT8 *)&save, Size);

		en = (ERROR_SUCCESS == l);

		RegFlushKey(hKey);
		RegCloseKey(hKey);
	}

	return en;
}
UINT32 WL_GetStartupState()
{
	HKEY	hKey = 0;
	DWORD	Size	= sizeof(DWORD);
	UINT32	en = 0;

	long l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Comm\\tiwlnapi1\\Parms"), 0, 0, &hKey);
	if(ERROR_SUCCESS == l)
	{
		l = RegQueryValueEx(hKey, L"EnableOnStartup", 0, 0, (UINT8 *)&en, &Size);

		if(ERROR_SUCCESS != l)
			en = 0;

		RegCloseKey(hKey);
	}

	return en;
}
BOOL BthIoControl(DWORD dwCtl)
{
	int ret = 0;
	WCHAR *argPtr = L"card";

	HANDLE hBTDev = CreateFile (L"BTD0:", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);

	if(hBTDev != INVALID_HANDLE_VALUE)
	{
		ret = DeviceIoControl(hBTDev, dwCtl, argPtr, sizeof(WCHAR) * (wcslen(argPtr) + 1), 0, 0, 0, 0);
		CloseHandle(hBTDev);
	}
	else
	{
		RETAILMSG(1,(TEXT("Unable to open access to the Bth Stack (error %ld)\n"), GetLastError()));
	}

	return ret;
}

BOOL BT_GetStatus(void)
{
	RETAILMSG(0, (TEXT("++BT_GetStatus\r\n")));
	BOOL bBthStatus = FALSE;

	int status = 0;
	int ret = BthGetHardwareStatus(&status);
	if (ret == ERROR_SUCCESS)
	{
		bBthStatus = status == HCI_HARDWARE_RUNNING;
		//RETAILMSG(1,(TEXT("BthGetHardwareStatus OK! (status = %ld)\r\n"), status));
	}
	else
	{
		RETAILMSG(1,(TEXT("BthGetHardwareStatus FAIL! (ret = %ld)\r\n"), ret));
	}

	RETAILMSG(0, (TEXT("--BT_GetStatus\r\n")));
	return bBthStatus;
}

BOOL BT_TurnOn(void)
{
	RETAILMSG(0, (TEXT("++BT_TurnOn\r\n")));
	BOOL bRet = FALSE;
	WCHAR *argPtr = L"card";
	int i = 20;

	bRet = BthIoControl(IOCTL_SERVICE_START);
	if (bRet)
	{
		while(!BT_GetStatus() && i--)
		{
			Sleep(500);
		}

		if(0 > i)
			bRet = 1;
	}
	
	RETAILMSG(0, (TEXT("--BT_TurnOn\r\n")));

	return bRet;
}

BOOL BT_TurnOff(void)
{
	RETAILMSG(0, (TEXT("++BT_TurnOff\r\n")));
	
	BOOL bRet = FALSE;
	WCHAR *argPtr = L"card";
	int i = 20;

	bRet = BthIoControl(IOCTL_SERVICE_STOP);
	if (bRet)
	{
		while (BT_GetStatus() && i--)
		{
			Sleep(500);
		}

		if(0 > i)
			bRet = 1;
	}
	RETAILMSG(0, (TEXT("--BT_TurnOff\r\n")));

	return bRet;
}
BOOL BT_SaveStartupState(UINT32 save)
{
	HKEY	hKey = 0;
	DWORD	Size;
	BOOL	en = 0;

	long l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1"),
						  0, 0, &hKey);
	if(ERROR_SUCCESS == l)
	{
        Size = sizeof(DWORD);

		l = RegSetValueEx(hKey, L"EnableOnStartup", 0, REG_DWORD, (UINT8 *)&save, Size);

		en = (ERROR_SUCCESS == l);

		RegFlushKey(hKey);
		RegCloseKey(hKey);
	}

	return en;
}


UINT32 BT_GetStartupState()
{
	HKEY	hKey	= 0;
	DWORD	Size	= sizeof(DWORD);
	UINT32	en		= 0;

	long l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1"),
						  0, 0, &hKey);
	if(ERROR_SUCCESS == l)
	{
		l = RegQueryValueEx(hKey, L"EnableOnStartup", 0, 0, (UINT8 *)&en, &Size);

		if(ERROR_SUCCESS != l)
			en = 0;

		RegCloseKey(hKey);
	}

	return en;
}


BOOL WL_TurnOn(UINT32 save)
{
	DEBUGMSG(1, (TEXT("++WL_TurnOn\r\n")));

	HANDLE					hNdisPwr;
	NDISPWR_SAVEPOWERSTATE	SavePowerState;
	TCHAR					szName[MAX_PATH];
	TCHAR					multiSz[257];
	int						nChars;        
	TCHAR*					WlanDevName = L"TIWLNAPI1";
    DWORD					cbBuffer = sizeof(multiSz);

	CEDEVICE_POWER_STATE	DevicePowerState;

	HANDLE							hMsgQueue = 0, hNdisuio = 0;
    MSGQUEUEOPTIONS					sOptions;			
    NDISUIO_REQUEST_NOTIFICATION	sRequestNotification;
	NDISUIO_DEVICE_NOTIFICATION		sDeviceNotification;
	DWORD							err	 = 0;
	DWORD							dwFlags;
	DWORD							Len;
	DWORD							timeout, wait_time;


	hNdisPwr = CreateFile(
				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
				0x00,											//	Desired access.
				0x00,											//	Share Mode.
				NULL,											//	Security Attr
				OPEN_EXISTING,									//	Creation Disposition.
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
				(HANDLE)INVALID_HANDLE_VALUE);	

	if(hNdisPwr == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	SavePowerState.pwcAdapterName = WlanDevName;
	if(!save)
	{
		if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_QUERY_SAVED_POWER_STATE, 0, 0, &SavePowerState, sizeof(SavePowerState), 0, 0))
		{
			CloseHandle(hNdisPwr);
			return 0;
		}
		
		DevicePowerState			= SavePowerState.CePowerState;
	}

	SavePowerState.CePowerState	= PwrDeviceUnspecified;
		
	if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_SAVE_POWER_STATE, &SavePowerState, sizeof(SavePowerState), 0, 0, 0, 0))
	{
		CloseHandle(hNdisPwr);
		return 0;
	}

	sOptions.dwSize					= sizeof(MSGQUEUEOPTIONS);
	sOptions.dwFlags				= 0;
	sOptions.dwMaxMessages			= 4;
	sOptions.cbMaxMessage			= sizeof(NDISUIO_DEVICE_NOTIFICATION);
	sOptions.bReadAccess			= TRUE;

	hMsgQueue = CreateMsgQueue(NULL,&sOptions);

	if(hMsgQueue == NULL)				
	{
		RETAILMSG(1,(TEXT("WL_TurnOn:: Error CreateMsgQueue()..\r\n")));
		CloseHandle(hNdisPwr);
		return 0;
	}

	hNdisuio = CreateFile((PTCHAR)NDISUIO_DEVICE_NAME, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	(HANDLE)-1);	

    if(hNdisuio == INVALID_HANDLE_VALUE)
	{
		RETAILMSG (1, (TEXT("WL_TurnOn:: Unable to open [%s]\r\n"), NDISUIO_DEVICE_NAME));

   		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		return 0;
	}
	sRequestNotification.hMsgQueue				= hMsgQueue;
	sRequestNotification.dwNotificationTypes	= NDISUIO_NOTIFICATION_BIND;

	if(!DeviceIoControl(hNdisuio, 
						IOCTL_NDISUIO_REQUEST_NOTIFICATION,
						&sRequestNotification,
						sizeof(NDISUIO_REQUEST_NOTIFICATION),
						NULL,
						0x00,
						NULL,
						NULL))
	{
		RETAILMSG(1,(TEXT("WL_TurnOn:: Err IOCTL_NDISUIO_REQUEST_NOTIFICATION\r\n")));
   		
		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		CloseHandle(hNdisuio);
		return 0;
	}

	nChars = _sntprintf(szName, MAX_PATH-1, _T("%s\\%s"), PMCLASS_NDIS_MINIPORT, WlanDevName);

	szName[MAX_PATH-1] = 0;

	RETAILMSG(1, (TEXT("WL_TurnOn: set unspecified state to %s\r\n"), szName));
	SetDevicePower(szName, POWER_NAME, PwrDeviceUnspecified);
	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	if(!DoNdisIOControl(IOCTL_NDIS_BIND_ADAPTER, multiSz, (_tcslen(multiSz)+2) * sizeof(TCHAR), 0, 0))
	{
		DeviceIoControl(hNdisuio, IOCTL_NDISUIO_CANCEL_NOTIFICATION, 0, 0, 0, 0, 0, 0);
		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		CloseHandle(hNdisuio);

		return 0;
	}

	Len = sizeof(szName);
    err = 1;
	timeout = GetTickCount() + 10000;

	while(err && timeout > GetTickCount())
	{	
		wait_time = timeout - GetTickCount();

		if(WaitForSingleObject(hMsgQueue, wait_time) == WAIT_OBJECT_0)
		{
    		while(ReadMsgQueue(	hMsgQueue,
    							&sDeviceNotification,
    							sizeof(NDISUIO_DEVICE_NOTIFICATION),
    							&Len,
    							1,
    							&dwFlags))
			{
				if(	(0 == wcscmp(WlanDevName, sDeviceNotification.ptcDeviceName))	&&
					(NDISUIO_NOTIFICATION_BIND & sDeviceNotification.dwNotificationType)	)
				{
					err = 0;
					break;
				}
			}
		}
	}
	
	if(!save && !err)
	{
		Sleep(2500);//for ndispwr.c hardcoded Sleep(2000);
		SavePowerState.CePowerState = (D0 == DevicePowerState) ? PwrDeviceUnspecified : DevicePowerState;		
		if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_SAVE_POWER_STATE, &SavePowerState, sizeof(SavePowerState), 0, 0, 0, 0))
		{
			err = 1;
		}
	}
	
//clean
	DeviceIoControl(hNdisuio, IOCTL_NDISUIO_CANCEL_NOTIFICATION, 0, 0, 0, 0, 0, 0);
	CloseHandle(hNdisPwr);
	CloseHandle(hMsgQueue);
	CloseHandle(hNdisuio);

	DEBUGMSG(1, (TEXT("--WL_TurnOn\r\n")));

	if(err)
		return 0; 
	return 1;
}
BOOL WL_TurnOff(UINT32 save)
{
	DEBUGMSG(1, (TEXT("++WL_TurnOff\r\n")));
	HANDLE					hNdisPwr;
	NDISPWR_SAVEPOWERSTATE	SavePowerState;
	TCHAR					szName[MAX_PATH];
	TCHAR					multiSz[257];
	int						nChars;        
	CEDEVICE_POWER_STATE    DevicePowerState;
	TCHAR*					WlanDevName = L"TIWLNAPI1";
    DWORD					cbBuffer = sizeof(multiSz);

	HANDLE							hMsgQueue = 0, hNdisuio = 0;
    MSGQUEUEOPTIONS					sOptions;			
    NDISUIO_REQUEST_NOTIFICATION	sRequestNotification;
	NDISUIO_DEVICE_NOTIFICATION		sDeviceNotification;
	DWORD							err	 = 0;
	DWORD							dwFlags;
	DWORD							Len;
	DWORD							timeout, wait_time;

	DEBUGMSG(1, (TEXT("Desactivating Wifi driver...\r\n")));

	hNdisPwr = CreateFile(
				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
				0x00,											//	Desired access.
				0x00,											//	Share Mode.
				NULL,											//	Security Attr
				OPEN_EXISTING,									//	Creation Disposition.
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
				(HANDLE)INVALID_HANDLE_VALUE);	

	if(hNdisPwr == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	SavePowerState.pwcAdapterName = WlanDevName;
	if(!save)
	{
		if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_QUERY_SAVED_POWER_STATE, 0, 0, &SavePowerState, sizeof(SavePowerState), 0, 0))
		{
			CloseHandle(hNdisPwr);
			return 0;
		}
		DevicePowerState = SavePowerState.CePowerState;
	}	

	SavePowerState.CePowerState   = D4;
		
	if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_SAVE_POWER_STATE, &SavePowerState, sizeof(SavePowerState), 0, 0, 0, 0))
	{
		CloseHandle(hNdisPwr);
		return 0;
	}

	sOptions.dwSize					= sizeof(MSGQUEUEOPTIONS);
	sOptions.dwFlags				= 0;
	sOptions.dwMaxMessages			= 4;
	sOptions.cbMaxMessage			= sizeof(NDISUIO_DEVICE_NOTIFICATION);
	sOptions.bReadAccess			= TRUE;

	hMsgQueue = CreateMsgQueue(NULL,&sOptions);

	if(hMsgQueue == NULL)				
	{
		RETAILMSG(1,(TEXT("WL_TurnOff:: Error CreateMsgQueue()..\r\n")));
		CloseHandle(hNdisPwr);
		return 0;
	}

	hNdisuio = CreateFile((PTCHAR)NDISUIO_DEVICE_NAME, 0, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	(HANDLE)-1);	

    if(hNdisuio == INVALID_HANDLE_VALUE)
	{
		RETAILMSG (1, (TEXT("WL_TurnOff:: Unable to open [%s]\r\n"), NDISUIO_DEVICE_NAME));

   		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		return 0;
	}
	sRequestNotification.hMsgQueue				= hMsgQueue;
	sRequestNotification.dwNotificationTypes	= NDISUIO_NOTIFICATION_UNBIND;

	if(!DeviceIoControl(hNdisuio, 
						IOCTL_NDISUIO_REQUEST_NOTIFICATION,
						&sRequestNotification,
						sizeof(NDISUIO_REQUEST_NOTIFICATION),
						NULL,
						0x00,
						NULL,
						NULL))
	{
		RETAILMSG(1,(TEXT("WL_TurnOff:: Err IOCTL_NDISUIO_REQUEST_NOTIFICATION\r\n")));
   		
		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		CloseHandle(hNdisuio);
		return 0;
	}

	nChars = _sntprintf(szName, MAX_PATH-1,	_T("%s\\%s"), PMCLASS_NDIS_MINIPORT, WlanDevName);

	szName[MAX_PATH-1]=0;

	SetDevicePower(szName, POWER_NAME, D4);
	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	if(!DoNdisIOControl(IOCTL_NDIS_UNBIND_ADAPTER, multiSz,	(_tcslen(multiSz)+2) * sizeof(TCHAR), 0, 0))
	{
		DeviceIoControl(hNdisuio, IOCTL_NDISUIO_CANCEL_NOTIFICATION, 0, 0, 0, 0, 0, 0);
		CloseHandle(hNdisPwr);
		CloseHandle(hMsgQueue);
		CloseHandle(hNdisuio);
		return 0;
	}

	Len = sizeof(szName);
    err = 1;
	timeout = GetTickCount() + 10000;

	while(err && timeout > GetTickCount())
	{	
		wait_time = timeout - GetTickCount();

		if(WaitForSingleObject(hMsgQueue, wait_time) == WAIT_OBJECT_0)
		{
    		while(ReadMsgQueue(	hMsgQueue,
    							&sDeviceNotification,
    							sizeof(NDISUIO_DEVICE_NOTIFICATION),
    							&Len,
    							1,
    							&dwFlags))
			{
				if(	(0 == wcscmp(WlanDevName, sDeviceNotification.ptcDeviceName))	&&
					(NDISUIO_NOTIFICATION_UNBIND & sDeviceNotification.dwNotificationType)	)
				{
					err = 0;
					break;
				}
			}
		}
	}

	if(!save && !err)
	{
		SavePowerState.CePowerState = (D0 == DevicePowerState) ? PwrDeviceUnspecified : DevicePowerState;		
		if(!DeviceIoControl(hNdisPwr, IOCTL_NPW_SAVE_POWER_STATE, &SavePowerState, sizeof(SavePowerState), 0, 0, 0, 0))
		{
			err = 1;
		}
	}
	
//clean
	DeviceIoControl(hNdisuio, IOCTL_NDISUIO_CANCEL_NOTIFICATION, 0, 0, 0, 0, 0, 0);
	CloseHandle(hNdisPwr);
	CloseHandle(hMsgQueue);
	CloseHandle(hNdisuio);
	
	DEBUGMSG(1, (TEXT("--WL_TurnOff\r\n")));

	if(err)
		return 0;
	return 1;
}




BOOL WL_BIPCalibration (BOOL bSB1_14, 
						BOOL bSB1_4,
						BOOL bSB8_16,
						BOOL bSB34_48,
						BOOL bSB52_64,
						BOOL bSB100_116,
						BOOL bSB120_140,
						BOOL bSB149_165,
						BOOL del)
{
	THandle g_hCmd;

	g_hCmd = CuCmd_Create(TIWLAN_DRV_NAME, NULL, FALSE, NULL);
	// Set power to Active
	ConParm_t powerParam[] = {{(PS8)"ACTIVE", CON_PARM_RANGE, 0, 1, (U32)1 },
						CON_LAST_PARM };

	CuCmd_SetPowerMode(g_hCmd, powerParam, 1);

	// Set Power level to active
	ConParm_t powerLevelParam[] = {{(PS8)"AWAKE", CON_PARM_RANGE, 0, 1, (U32)2 },
						CON_LAST_PARM };
	CuCmd_SetPowerSavePowerLevel(g_hCmd, powerLevelParam, 1);

	// Set Default Power level
	ConParm_t powerDefaultLevelParam[] = {{(PS8)"AWAKE", CON_PARM_RANGE, 0, 1, (U32)2 },
						CON_LAST_PARM };

	CuCmd_SetDefaultPowerLevel(g_hCmd, powerDefaultLevelParam, 1);
	

	// Trac: Ticket N°2
	// Setting Tunes Channel to 7
	ConParm_t channelTuneParam[] = {{(PS8)"Band", CON_PARM_RANGE, 0, 1, (U32)0 },
	{(PS8)"Channel", CON_PARM_RANGE, 0, 1, (U32)7 },
		CON_LAST_PARM };

	CuCmd_RadioDebug_ChannelTune(g_hCmd, channelTuneParam, 2);
	
	// uses the BiP reference point 375, 128 (16dbm*8) for calibration
	ConParm_t refPointParam[] = {
		{(PS8)"iRefPtDetectorValue", CON_PARM_RANGE, 0, 1, (U32)375},
		{(PS8)"iRefPtPower", CON_PARM_RANGE, 0, 1, (U32)128},
		{(PS8)"isubBand", CON_PARM_RANGE, 0, 1, (U32)0},
		CON_LAST_PARM };

	Sleep(1000); // Avoiding Access violation in IPC_STA_Private_Send.
	CuCmd_BIP_BufferCalReferencePoint(g_hCmd,refPointParam, 3);
	// End of: Trac: Ticket N°2


	// Perform BIP for specific channels
	ConParm_t bipParam[] = {{(PS8)"Sub Band B/G:  1 - 14", CON_PARM_RANGE, 0, 1, (U32)bSB1_14 },
                        {(PS8)"Sub Band A:    1 -  4", CON_PARM_RANGE, 0, 1, (U32)bSB1_4 },
                        {(PS8)"Sub Band A:    8 - 16", CON_PARM_RANGE, 0, 1, (U32)bSB8_16 },
                        {(PS8)"Sub Band A:   34 - 48", CON_PARM_RANGE, 0, 1, (U32)bSB34_48 },
                        {(PS8)"Sub Band A:   52 - 64", CON_PARM_RANGE, 0, 1, (U32)bSB52_64 },
                        {(PS8)"Sub Band A:  100 -116", CON_PARM_RANGE, 0, 1, (U32)bSB100_116 },
                        {(PS8)"Sub Band A:  120 -140", CON_PARM_RANGE, 0, 1, (U32)bSB120_140 },
                        {(PS8)"Sub Band A:  149 -165", CON_PARM_RANGE, 0, 1, (U32)bSB149_165 },
						CON_LAST_PARM };

	// Launch BIP cablibration
	// Calibration generates a NVS file on the current storage
	// this file will be then post-processed to get its content
	// stored in registry (use hive registry to keep persistency)
	CuCmd_BIP_StartBIP(g_hCmd, bipParam, 8);

	CuCmd_Destroy(g_hCmd);

	// Remove temporary NVS file
	if(del)
		DeleteFile(NVS_FILE_PATH);

	return TRUE;
}

BOOL WL_RadioTest(HWND hWnd)
{
	THandle hCmd;
//	Sleep(5000);
	hCmd = CuCmd_Create(TIWLAN_DRV_NAME, NULL, FALSE, NULL);

	// Set power to Active
	ConParm_t powerParam[] = {{(PS8)"ACTIVE", CON_PARM_RANGE, 0, 1, (U32)1 }, CON_LAST_PARM};
	CuCmd_SetPowerMode(hCmd, powerParam, 1);

	// Set Power level to active
	ConParm_t powerLevelParam[] = {{(PS8)"AWAKE", CON_PARM_RANGE, 0, 1, (U32)2 }, CON_LAST_PARM};
	CuCmd_SetPowerSavePowerLevel(hCmd, powerLevelParam, 1);

	// Set Default Power level
	ConParm_t powerDefaultLevelParam[] = {{(PS8)"AWAKE", CON_PARM_RANGE, 0, 1, (U32)2 }, CON_LAST_PARM };
	CuCmd_SetDefaultPowerLevel(hCmd, powerDefaultLevelParam, 1);

	// Trac: Ticket N°2
	// Setting Tunes Channel to 7
	ConParm_t channelTuneParam[] =
	{{(PS8)"Band", CON_PARM_RANGE, 0, 1, (U32)0 }, {(PS8)"Channel", CON_PARM_RANGE, 0, 1, (U32)7 }, CON_LAST_PARM };
	CuCmd_RadioDebug_ChannelTune(hCmd, channelTuneParam, 2);
		
	// uses the BiP reference point 375, 128 (16dbm*8) for calibration
	ConParm_t refPointParam[] = { {(PS8)"iRefPtDetectorValue", CON_PARM_RANGE, 0, 1, (U32)375},
								  {(PS8)"iRefPtPower", CON_PARM_RANGE, 0, 1, (U32)128},
								  {(PS8)"isubBand", CON_PARM_RANGE, 0, 1, (U32)0}, CON_LAST_PARM};

	Sleep(1000); // Avoiding Access violation in IPC_STA_Private_Send.
	CuCmd_BIP_BufferCalReferencePoint(hCmd,refPointParam, 3);
	// End of: Trac: Ticket N°2

/*
	CuCmd_t* pCuCmd = (CuCmd_t*)hCmd;
	TTestCmd data;
	int i;

	memset(&data, 0, sizeof(data));

	data.testCmdId = TEST_CMD_P2G_CAL;
	data.testCmd_u.P2GCal.iSubBandMask = 0;
	for(i = 0; i < 8; i++) 
	{
		data.testCmd_u.P2GCal.iSubBandMask |= 1 << i;
	}

	CuCommon_Radio_Test(pCuCmd->hCuCommon, &data);
*/
	CuCmd_Destroy(hCmd);

//	if(TI_OK != data.testCmd_u.P2GCal.oRadioStatus)
//		MessageBox(hWnd, L"Radio test is failed\r\n",  L"WL1271", MB_OK);

	return 1;
}

int WINAPI DllMain(HANDLE hInstance, DWORD reason, LPVOID pvReserved)
{
    if(reason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls((HMODULE)hInstance);

    return 1;
}
