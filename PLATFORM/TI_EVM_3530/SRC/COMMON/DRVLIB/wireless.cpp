#include <windows.h>
#include <bsp.h>
#include <service.h>
#include "eapol.h"
#include "wzcsapi.h"
#include <bt_api.h>

BOOL IsWirelessModulePresent()
{
	BOOL bPresent = FALSE;
	card_ver cv = {0};
	UINT32 in = OAL_ARGS_QUERY_WLAN_CFG;

	HANDLE hProxy = CreateFile(L"PXY1:", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hProxy != INVALID_HANDLE_VALUE)
	{

		if (DeviceIoControl(hProxy, IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &cv, sizeof(cv), 0, 0))
		{
			if (cv.ver != -1)
				bPresent = TRUE;
		}

		CloseHandle(hProxy);
	}
	
	return bPresent;
}

BOOL IsWirelessModuleActive()
{
	BOOL bActive = FALSE;
	HANDLE hCardPresent = CreateEvent(0, 1, 0, L"MMC Card Present 1");
	if (hCardPresent)
	{
		DWORD res = WaitForSingleObject(hCardPresent, 0);
		if (res == WAIT_OBJECT_0)
			bActive = TRUE;

		CloseHandle(hCardPresent);
	}

	return bActive;
}

BOOL IsWirelessCardAvailable()
{
    INTFS_KEY_TABLE IntfsTable;
    IntfsTable.dwNumIntfs = 0;
    IntfsTable.pIntfs = NULL;

    DWORD dwStatus = WZCEnumInterfaces(NULL, &IntfsTable);

    if (dwStatus != ERROR_SUCCESS)
        return FALSE;        

    if (!IntfsTable.dwNumIntfs)
        return FALSE;

	return TRUE;
}

void RunBluetooth()
{
	HKEY hKeyRoot = 0;
	DWORD dwDisp;
	INT32 iErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1", 0, 0, 0, KEY_ALL_ACCESS, 0, &hKeyRoot, &dwDisp);
	if (iErr == ERROR_SUCCESS)
	{
		RegSetValueEx(hKeyRoot, L"driver", 0, REG_SZ, (UINT8 *)L"azbthuart_pm.dll", sizeof(L"azbthuart_pm.dll"));
		RegCloseKey(hKeyRoot);
		TCHAR argPtr[] = L"card";
		HANDLE hDev = CreateFile(L"BTD0:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if (hDev != INVALID_HANDLE_VALUE)
		{
			int iErr = DeviceIoControl(hDev, IOCTL_SERVICE_START, argPtr, sizeof(argPtr[0]) * (_tcslen(argPtr) + 1), 0, 0, 0, 0);
			CloseHandle(hDev);
		}
	}
}

BOOL IsBluetoothStackActive()
{
	int status = 0;
	int ret = BthGetHardwareStatus(&status);
	if (ret == ERROR_SUCCESS)
	{
		return (status == HCI_HARDWARE_RUNNING);
	}

	return FALSE;
}
