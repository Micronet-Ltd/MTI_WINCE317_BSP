#include <windows.h>
//#include <stdio.h>
#include <ndis.h>
#include <Nuiouser.h>
#include <winsock2.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <pm.h>
#include <ndispwr.h>

#include <bt_api.h>

#include <storemgr.h>
#include <fatutil.h>
#include <bootpart.h>
#include <ceddkex.h>
#include <service.h>
#include <oal.h>
#include <oalex.h>
#include <args.h>

#if 0
int is_dp_exist(TCHAR *v_name, unsigned long vn_len, TCHAR *dp_name, unsigned long dpn_len)
{
	int found = 0;
	HANDLE h, hs, hp;
    STOREINFO si = {0};
	PARTINFO  pi = {0};

	if(!dp_name)
		return found;

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
					if(pi.bPartType == PART_BOOTSECTION)
					{
						HANDLE h = OpenPartition(hs, pi.szPartitionName);
						if((HANDLE)-1 != h)
						{
							int ret = SetPartitionAttributes(h, PARTITION_ATTRIBUTE_BOOT | PARTITION_ATTRIBUTE_READONLY);
//							int ret = SetPartitionAttributes(h, PARTITION_ATTRIBUTE_READONLY);
							if(!ret)
								ret = GetLastError();
							CloseHandle(h);
						}
						continue;
					}
					if(pi.bPartType != PART_DOS32)
						continue;

					if(0 == _tcsnicmp(pi.szPartitionName, dp_name, dpn_len))
						found = 1;
				}while(FindNextPartition(hp, &pi) && (si.dwPartitionCount > 1) && !found);
				FindClosePartition(hp);
			}

			CloseHandle(hs);
		}while(FindNextStore(h, &si) && !found);

		FindCloseStore(h);
	}

//	_tprintf(TEXT("Data partition is%sfound\r\n"), (found)?TEXT(" "):TEXT(" not "));
	if(!found)
		MessageBox(0, L"Data partition doesn't exist or corrupted", L"Storage fatal error", MB_OK);

	return found;
}

int create_dp(TCHAR *v_name, unsigned long vn_len, TCHAR *dp_name, unsigned long dpn_len, unsigned long force)
{
	unsigned long ret = 0;
	HANDLE hs;

	if(!force)
	{
		if(!v_name)
		{
			v_name = TEXT("Flash Disk");
			vn_len = _tcslen(v_name);
		}

		if(!dp_name)
		{
			dp_name = TEXT("Part01");
			dpn_len = _tcslen(dp_name);
		}

		if(!is_dp_exist(v_name, vn_len, dp_name, dpn_len))
			force = 1;
	}

	if(force)
	{
		if(!dp_name)
			return ret;
		if(!v_name)
			return ret;

//		_tprintf(TEXT("Open %s store\r\n"), v_name);
		hs = OpenStore(v_name);
		if((HANDLE)-1 != hs)
		{
			ret = CreatePartitionEx(hs, dp_name, PART_DOS32, 0);
			CloseHandle(hs);
		}
	}

//	_tprintf(TEXT("FAT 32 partition is%screated\r\n"), (ret)?TEXT(" "):TEXT(" not "));
	if(!ret)
		MessageBox(0, L"Data partition hasn't created", L"Storage fatal error", MB_OK);

	return ret;
}

void FormatProgress(DWORD per)
{
	RETAILMSG(1, (_T("Format: %d%% complete\r\n"), per));

	_tprintf(TEXT("Format: %d%% complete\r\n"), per);
}

int FormatMessage(LPTSTR szMessage, LPTSTR szCaption, BOOL fYesNo)
{
	RETAILMSG(1, (_T("Finally: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo));

	_tprintf(TEXT("Finally: %s %s %d complete\r\n"), szMessage, szCaption, fYesNo);

	return 1;
}

int format_dp(TCHAR *v_name, TCHAR *p_name)
{
	HANDLE hs;
	HANDLE hp;
	PARTINFO  pi = {0};
	DISK_INFO di = {0};

	if(!v_name)
		return 0;
	if(!p_name)
		return 0;

	_tprintf(TEXT("Open %s store\r\n"), v_name);

	hs = OpenStore(v_name);

	if((HANDLE)-1 != hs)
	{
		_tprintf(TEXT("Open '%s' partition\r\n"), p_name);
		hp = OpenPartition(hs, p_name);
		if((HANDLE)-1 != hs)
		{
			pi.cbSize = sizeof(pi);
			GetPartitionInfo(hp, &pi);

			if(pi.dwAttributes & PARTITION_ATTRIBUTE_MOUNTED)
			{
				_tprintf(TEXT("Dismount partition\r\n"));
				DismountPartition(hp);
			}

			PFN_FORMATVOLUME pfnFormatVolume = 0;

			HMODULE hm = LoadLibrary(TEXT("fatutil.dll"));
			if(hm)
			{
				FORMAT_OPTIONS fo;

				pfnFormatVolume = (PFN_FORMATVOLUME)GetProcAddress(hm, TEXT("FormatVolume"));
				if(pfnFormatVolume)
				{
					ULONG ret;
					if(DeviceIoControl(hp, DISK_IOCTL_GETINFO, &di, sizeof(di), &di, sizeof(di), &ret, 0))
					{
						fo.dwClusSize = di.di_bytes_per_sect;//*512;
						fo.dwRootEntries = 512;
						fo.dwFatVersion = 64;
						fo.dwNumFats = 2;
						fo.dwFlags = FATUTIL_FORMAT_TFAT | FATUTIL_FORMAT_EXFAT | FATUTIL_FULL_FORMAT;

						_tprintf(TEXT("Preparing to Transactional FAT format:\r\n\tFAT version: exFAT\r\n\tClaster size: %d\r\n\tRoot entries %d\r\n\tFat copies %d\r\n"),
							fo.dwClusSize, fo.dwRootEntries, fo.dwNumFats);
						pfnFormatVolume(hp, &di, &fo, FormatProgress, FormatMessage);
					}
				}
				FreeLibrary(hm);
			}

			MountPartition(hp);
			CloseHandle(hp);
		}
		CloseHandle(hs);
	}

	return 0;
}

#endif
BOOL IsWirelessModuleEnabled()
{
	BOOL bIsEnabled = FALSE;
	HKEY hKey = NULL;
	LONG lStatus;

	lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("\\Drivers\\BuiltIn\\SDHC2"), 0, 0, &hKey);
	if (ERROR_SUCCESS == lStatus)
	{
		DWORD dwValue = 0;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwValType = REG_DWORD;
		
		lStatus = RegQueryValueEx( hKey, TEXT("EnableWirelessModule"), NULL, &dwValType, (LPBYTE)&dwValue, &dwSize);
		if (ERROR_SUCCESS == lStatus)
		{
			if (dwValue == 1)
				bIsEnabled = TRUE;
		}

		RegCloseKey(hKey);
	}

	return bIsEnabled;
}

#define POWER_RESTART	0x00

#ifdef POWER_RESTART

#include "omap35xx_base_regs.h"
#include "omap35xx_config.h"
#include "oal_io.h"
#include <initguid.h>
#include "gpio.h"
typedef void* (*fnShell_MapIOSpace)(PHYSICAL_ADDRESS, UINT32, BOOL); 
#endif

BOOL TIWIR2PowerOnOff ()
{
#ifdef POWER_RESTART
	HMODULE					hLib					= 0;
	fnShell_MapIOSpace		pfn_ShellMapSpace		= 0;
	void*					pCurrAdr				= 0;
	HANDLE					hGPIO					= 0;
	PHYSICAL_ADDRESS		pa;


	if ( hLib = LoadLibrary(L"omap_shell.dll") )
	{
		pfn_ShellMapSpace = (fnShell_MapIOSpace)GetProcAddress(hLib, L"MmMapIoSpace_Proxy");
		if ( NULL == pfn_ShellMapSpace )
			RETAILMSG(1, (L"afdp: pfn_ShellMapSpace FAILED\r\n"));
	}

	hGPIO = GPIOOpen();
	if(NULL == hGPIO) 
	{
		RETAILMSG(1, (L"afdp: GPIOOpen FAILED\r\n"));
	}

	if (pfn_ShellMapSpace && hGPIO)
	{
		//Shutdown WL1271 module
		pa.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
		OMAP_SYSC_PADCONFS_REGS   *pConfig = (OMAP_SYSC_PADCONFS_REGS*)pfn_ShellMapSpace(pa, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_CLKX, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));  // UART2_TX
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_FSX,  (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));  // UART2_RX
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));  // UART2_RTS
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));  // UART2_CTS

		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CLK, (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_7));	// MMC2_CLK
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CMD, (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_7));	// MMC2_CMD
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT0, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));	// MMC2_DAT0
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT1, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));	// MMC2_DAT1
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT2, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));	// MMC2_DAT2
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT3, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_7));	// MMC2_DAT3

		//Disable
		GPIOClrBit(hGPIO, GPIO_137);
		GPIOClrBit(hGPIO, GPIO_138);

		// Remove VIO 1.8 V
		GPIOSetBit(hGPIO, GPIO_58);
		// Remove VBAT 3.6 V
		GPIOClrBit(hGPIO, GPIO_136);

		Sleep (500);

		// Supply VBAT 3.6 V to WiFi
		GPIOSetBit(hGPIO, GPIO_136);
		//OALStall(10);
		GPIOClrBit(hGPIO, GPIO_58);

		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CLK,  (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_CLK
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_CMD,  (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_CMD
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT0, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT0
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT1, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT1
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT2, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT2
		OUTREG16(&pConfig->CONTROL_PADCONF_MMC2_DAT3, (INPUT_ENABLE  | PULL_UP | MUX_MODE_0));	// MMC2_DAT3

		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_CLKX, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_TX
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_FSX,  (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // UART2_RX
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DR,   (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_1));  // UART2_RTS
		OUTREG16(&pConfig->CONTROL_PADCONF_MCBSP3_DX,   (INPUT_ENABLE  | PULL_INACTIVE | MUX_MODE_1));  // UART2_CTS

		//Enable
		//GPIOSetBit (hGPIO, GPIO_137);
		//GPIOSetBit(hGPIO, GPIO_138);		
	}

	if (hLib)
		FreeLibrary (hLib);

	if ( hGPIO )
		GPIOClose(hGPIO);

	return TRUE;
#endif //POWER_RESTART

	return FALSE;
}



#include "eapol.h"
#include "wzcsapi.h"

void EnDisZeroConfig ( BOOL bState )
{
	//find WIFI card
	INTFS_KEY_TABLE IntfsTable;
	IntfsTable.dwNumIntfs = 0;
	IntfsTable.pIntfs = NULL;
	DWORD dwInFlags = 0;
	INTF_ENTRY_EX Intf;
	DWORD dwStatus;

	dwStatus = WZCEnumInterfaces(NULL, &IntfsTable);

	if(dwStatus != ERROR_SUCCESS)
	{
		RETAILMSG(1, (TEXT("afdp: WZCEnumInterfaces() error 0x%08X \r\n"), dwStatus) );
		return;        
	}

	if(!IntfsTable.dwNumIntfs)
	{
		RETAILMSG(1, (L"afdp: system has no wireless card.\r\n"));
		return;
	}

	//wcsncpy(g_WirelessCard1, IntfsTable.pIntfs[0].wszGuid, MAX_PATH-1);
	RETAILMSG(1, (TEXT("afdp: wireless card found: %s \r\n"), IntfsTable.pIntfs[0].wszGuid) );
		//

	memset(&Intf, 0x00, sizeof(INTF_ENTRY_EX));
	Intf.wszGuid = IntfsTable.pIntfs[0].wszGuid;
	if ( bState)
	{
		RETAILMSG(1, (L"afdp: Try WZS En\r\n"));
		Intf.dwCtlFlags |= INTFCTL_ENABLED;
	}
	else
	{
		RETAILMSG(1, (L"afdp: Try WZS Dis\r\n"));
		Intf.dwCtlFlags &= ~INTFCTL_ENABLED;
	}

	dwStatus = WZCSetInterfaceEx(NULL, INTF_ENABLED, &Intf, &dwInFlags);
	if(dwStatus)
		RETAILMSG(1, (TEXT("afdp: WZCSetInterfaceEx() error dwStatus=0x%0X, dwOutFlags=0x%0X\r\n"), dwStatus, dwInFlags) );
	else
		RETAILMSG(1, (TEXT("afdp: %s\r\n"), bState ? L"enabled" : L"disabled") );

	// need to free memory allocated by WZC for us.
	LocalFree(IntfsTable.pIntfs);
}

#if defined(HOST_PLAT_EVM_OMAP3530)
//#define WLAN_DEVICE_NAME 			(const char*) (TEXT("TIWLNAPI1"))
#define NVS_FILE_PATH	L"\\windows\\nvs_map.bin"
#if 0
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
		RETAILMSG(1, (TEXT("WL_TurnOn: DoNdisIOControl %d\r\n"), GetLastError()));
	}


	return fResult;
}

BOOL WL_GetStatus(void)
{
	WSADATA WsaData;
	ULONG                    ulBufferLength, ulFlags;
	DWORD                    dwError = 1;
	PIP_ADAPTER_ADDRESSES    pAdapterAddresses;
	DWORD GetAdaptAddRet = 0;

	if(WSAStartup(MAKEWORD(2,2), &WsaData) == 0)
	{
		ulFlags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;

		GetAdaptAddRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, NULL, NULL, &ulBufferLength);

		pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(ulBufferLength);
	   
		if(NULL == pAdapterAddresses)
		{
			RETAILMSG(1, (TEXT("WL_GetStatus: Insufficient Memory\r\n")));
			return 0;
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
				}
				pAdapterAddresses = pAdapterAddresses->Next;
			}

			RETAILMSG(dwError, (TEXT("WL_GetStatus: TIWLNAPI1 isn't running\r\n")));
		}
		else
		{
			RETAILMSG(1, (TEXT("WL_GetStatus: Can't get Adaptaters info\r\n")));
		}

		WSACleanup();
	}

	return (dwError == 0);
}

BOOL WL_TurnOn(int bind)
{
	DWORD dwErr = S_OK;
    BOOL bRet = FALSE;
	HANDLE hNdisPwr;
	NDISPWR_SAVEPOWERSTATE SavePowerState;
	TCHAR szName[MAX_PATH];
	TCHAR multiSz[257];
	int nChars;        


	TCHAR *WlanDevName = L"TIWLNAPI1";

    DWORD cbBuffer = sizeof(multiSz);

	hNdisPwr = CreateFile(
				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
				0x00,											//	Desired access.
				0x00,											//	Share Mode.
				NULL,											//	Security Attr
				OPEN_EXISTING,									//	Creation Disposition.
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
				(HANDLE)INVALID_HANDLE_VALUE);	

	if(hNdisPwr != INVALID_HANDLE_VALUE)
	{
		SavePowerState.pwcAdapterName = WlanDevName;
		SavePowerState.CePowerState   = PwrDeviceUnspecified;
		
		bRet = DeviceIoControl(
					hNdisPwr,
					IOCTL_NPW_SAVE_POWER_STATE,
					&SavePowerState,
					sizeof(NDISPWR_SAVEPOWERSTATE),
					NULL,
					0x00,
					NULL,
					NULL);

		dwErr = GetLastError();

		CloseHandle(hNdisPwr);
	}

	if(!bind)
		return 1;

	nChars = _sntprintf(
					szName, 
					MAX_PATH-1, 
					_T("%s\\%s"), 
					PMCLASS_NDIS_MINIPORT, 
					WlanDevName);

	szName[MAX_PATH-1]=0;

	RETAILMSG(1, (TEXT("WL_TurnOn: set unspecified state to %s\r\n"), szName));
	SetDevicePower(szName, POWER_NAME, PwrDeviceUnspecified);

	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	RETAILMSG(1, (TEXT("WL_TurnOn: bind %s adapter\r\n"), multiSz));
	bRet = DoNdisIOControl(
					IOCTL_NDIS_BIND_ADAPTER, 
					multiSz,
					(_tcslen(multiSz)+2) * sizeof(TCHAR),
					NULL, 
					NULL);   

	return bRet;
}

BOOL WL_TurnOff(int unbind)
{
	DWORD dwErr = S_OK;
    BOOL bRet = FALSE;
	HANDLE hNdisPwr;
	NDISPWR_SAVEPOWERSTATE SavePowerState;
	TCHAR szName[MAX_PATH];
	TCHAR multiSz[257];
	int nChars;        


	TCHAR *WlanDevName = L"TIWLNAPI1";

    DWORD       cbBuffer = sizeof(multiSz);

	hNdisPwr = CreateFile(
				(PTCHAR)NDISPWR_DEVICE_NAME,					//	Object name.
				0x00,											//	Desired access.
				0x00,											//	Share Mode.
				NULL,											//	Security Attr
				OPEN_EXISTING,									//	Creation Disposition.
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,	//	Flag and Attributes..
				(HANDLE)INVALID_HANDLE_VALUE);	

	if (hNdisPwr != INVALID_HANDLE_VALUE)
	{
		SavePowerState.pwcAdapterName = WlanDevName;
		SavePowerState.CePowerState   = D4;
		
		bRet = DeviceIoControl(
					hNdisPwr,
					IOCTL_NPW_SAVE_POWER_STATE,
					&SavePowerState,
					sizeof(NDISPWR_SAVEPOWERSTATE),
					NULL,
					0x00,
					NULL,
					NULL);

		dwErr = GetLastError();

		CloseHandle(hNdisPwr);
	}

	if(!unbind)
		return 1;

	nChars = _sntprintf(
					szName, 
					MAX_PATH-1, 
					_T("%s\\%s"), 
					PMCLASS_NDIS_MINIPORT, 
					WlanDevName);

	szName[MAX_PATH-1]=0;

	RETAILMSG(1, (TEXT("WL_TurnOff: set D4 state to %s\r\n"), szName));
	SetDevicePower(szName, POWER_NAME, D4);

	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	RETAILMSG(1, (TEXT("WL_TurnOff: unbind %s adapter\r\n"), multiSz));
	bRet = DoNdisIOControl(
					IOCTL_NDIS_UNBIND_ADAPTER, 
					multiSz,
					(_tcslen(multiSz)+2) * sizeof(TCHAR),
					NULL, 
					NULL);   

	return bRet;
}
#endif
typedef BOOL (*fnWL_BIPCalibration)(BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL, BOOL);
typedef BOOL (*fnWL_RadioTest)(HWND);
typedef BOOL 	(*PFN_WL_GetStatus		)(void);
typedef BOOL	(*PFN_WL_TurnOn			)(UINT32 save);	
typedef BOOL	(*PFN_WL_TurnOff		)(UINT32 save);
typedef UINT32	(*PFN_WL_GetStartupState)(void);	
typedef BOOL	(*PFN_BT_GetStatus		)(void);
typedef BOOL	(*PFN_BT_TurnOn			)(void);
typedef BOOL	(*PFN_BT_TurnOff		)(void);	
typedef UINT32	(*PFN_BT_GetStartupState)(void);	

#if 0
extern "C"
{
#include "..\\..\\..\\..\\..\\..\\3rdParty\\AdeneoEmbedded\\Drivers\\WiLink_6.0\\CUDK\\os\\common\\inc\\cu_osapi.h"
#include "..\\..\\..\\..\\..\\..\\3rdParty\\AdeneoEmbedded\\Drivers\\WiLink_6.0\\CUDK\\configurationutility\\inc\\console.h"
#include "..\\..\\..\\..\\..\\..\\3rdParty\\AdeneoEmbedded\\Drivers\\WiLink_6.0\\CUDK\\configurationutility\\inc\\cu_cmd.h"
}
extern "C"
{
void Console_Stop(THandle hConsole)
{
}
void g_tester_send_event(U8 event_index)
{
}

void ProcessLoggerMessage(PU8 data, U32 len)
{
}
void user_main(){}
}

#define TIWLAN_DRV_NAME "tiwlan0"

BOOL WL_BIPCalibration (BOOL bSB1_14, 
						BOOL bSB1_4,
						BOOL bSB8_16,
						BOOL bSB34_48,
						BOOL bSB52_64,
						BOOL bSB100_116,
						BOOL bSB120_140,
						BOOL bSB149_165)
{
	THandle g_hCmd;
/*
	// Pre process by pushing the reg into an NVS file
	CREG2BIN reg2bin;
	if (ERROR_SUCCESS != reg2bin.RegToFile(NVS_FILE_PATH, NVS_REG_PATH))
	{
		return FALSE;
	}
	
	// Backup MAC Address
	WCHAR szMACAddress[20];
	DWORD dwSize = 19;
	WL_GetMACAddress (szMACAddress, dwSize);
*/
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

/*
	// Post process the file to registry
	CBIN2REG bin2reg;
	bin2reg.BinFileToReg(NVS_FILE_PATH, NVS_REG_PATH);

	// Remove temporary NVS file
	DeleteFile(NVS_FILE_PATH);

	// Restore MAC Address
	WL_SetMACAddress (szMACAddress);
*/
	CuCmd_Destroy(g_hCmd);

	return TRUE;
}
#endif
#if 0
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
	RETAILMSG(1, (TEXT("++BT_GetStatus\r\n")));
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

	RETAILMSG(1, (TEXT("--BT_GetStatus\r\n")));
	return bBthStatus;
}

BOOL BT_TurnOn(void)
{
	RETAILMSG(1, (TEXT("++BT_TurnOn\r\n")));
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

		bRet = i > 0;
	}
	
	RETAILMSG(1, (TEXT("--BT_TurnOn\r\n")));

	return bRet;
}

BOOL BT_TurnOff(void)
{
	RETAILMSG(1, (TEXT("++BT_TurnOff\r\n")));
	
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

		bRet = i > 0;
	}

	RETAILMSG(1, (TEXT("--BT_TurnOff\r\n")));

	return bRet;
}

#endif
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

#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
#if 0
//	UINT32 QueryFormat = 0, ret;
//	CEDEVICE_POWER_STATE ps = PwrDeviceUnspecified;
//	TCHAR v_name[] = TEXT("NFD");
//	TCHAR p_name[] = TEXT("Part01");

//	void *h = CreateBusAccessHandle(TEXT("Drivers\\Active\\12"));
//	ret = GetDevicePowerState(h, &ps, 0);
//	if(h)
//		CloseBusAccessHandle(h);
//	_tprintf(TEXT("Auto-check Data Partition\r\n"));
/*
	ret = HalQueryFormatPartition(&QueryFormat, sizeof(QueryFormat));
	if(!ret)
	{
		QueryFormat = 0;
		ret = GetLastError();
	}

	if(!is_dp_exist(v_name, _tcslen(v_name), p_name, _tcslen(p_name)))
	{
		if(create_dp(v_name, _tcslen(v_name), p_name, _tcslen(p_name), 1))
			QueryFormat = 1;
		else
		{
			_tcscpy(v_name, TEXT("DSK0:"));
			if(create_dp(v_name, _tcslen(v_name), p_name, _tcslen(p_name), 1))
				QueryFormat = 1;
		}
	}
	if(QueryFormat)
	{
		format_dp(v_name, p_name);

		_tprintf(TEXT("Restarting platform ..."));
		SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
	}
*/
//	int c = getc(stdin);

//	STORE_INFORMATION sti = {0};

//	GetStoreInformation(&sti);

//	if(is_dp_formatted())
//		SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
/*
	HANDLE hCardDetectEvent0 = CreateEvent(0, 0, 0, L"MMC Card Detect 0");
	if(hCardDetectEvent0)
	{
		RETAILMSG(1, (_T("Signal MMC CD %d\r\n"), GetTickCount()));
		SetEvent(hCardDetectEvent0);
		Sleep(1000);
		CloseHandle(hCardDetectEvent0);

		HANDLE hWaitForDir = FindFirstChangeNotification(L"\\", 1, FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_CEGETINFO);

		DWORD err, aval;

		DWORD buf[256];
		FILE_NOTIFY_INFORMATION *fni = (FILE_NOTIFY_INFORMATION *)buf;
		err = WaitForSingleObject(hWaitForDir, 10000);

		if(WAIT_OBJECT_0 == err)
		{
			err = CeGetFileNotificationInfo(hWaitForDir, 0, fni, sizeof(buf), (DWORD *)&ret, &aval);
			fni->FileName[fni->FileNameLength] = 0;
			RETAILMSG(1, (_T("New Volume '%s' detected %d\r\n"), (err)?fni->FileName:L"isn't", GetTickCount()));
		}
		FindCloseChangeNotification(hWaitForDir);
	}
	Sleep(2000);
	HANDLE hCardDetectEvent = CreateEvent(0, 0, 0, L"MMC Card Detect 1");
	HANDLE hCardPresentEvent = CreateEvent(0, 1, 0, L"MMC Card Present 1");
	if(hCardDetectEvent && hCardPresentEvent)
	{
		RETAILMSG(1, (_T("Signal WL/BT CD %d\r\n"), GetTickCount()));
		SetEvent(hCardPresentEvent);
		SetEvent(hCardDetectEvent);
		Sleep(1000);
		CloseHandle(hCardDetectEvent);
		CloseHandle(hCardPresentEvent);
	}
*/
#endif
#if !defined(HOST_PLAT_EVM_OMAP3530)
	if (!IsWirelessModuleEnabled())
	{
		HANDLE hCardPresent = CreateEvent(0, 1, 0, L"MMC Card Present 1");
		if (hCardPresent)
		{
			HANDLE hCardDetectEvent = CreateEvent(0, 0, 0, L"MMC Card Detect 1");
			if (hCardDetectEvent)
			{
				ResetEvent(hCardPresent);
				SetEvent(hCardDetectEvent);
				CloseHandle(hCardDetectEvent);
			}

			CloseHandle(hCardPresent);
		}
	}
	else
	{

// startup of BT HCI transport
#if BSP_BTH_CSR_ONLY != 1
//	card_ver m_cv;							// card vesion
//	UINT32   in = OAL_ARGS_QUERY_WLAN_CFG;

//	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &m_cv, sizeof(m_cv), 0))
//	{
//		m_cv.card_addr	= -1;
//		m_cv.ver		= -1;
//		m_cv.rev		= -1;
//		m_cv.config		= -1;
//        RETAILMSG(1, (_T("Failuere to get WL/BT card version!\r\n")));
//	}

		// allocate the interrupt event for card detection
		HANDLE m_hCardPresent = CreateEvent(0, 1, 0, L"MMC Card Present 1");

		if(WAIT_OBJECT_0 == WaitForSingleObject(m_hCardPresent, 200))
		{
			HKEY hKeyRoot = 0;
			DWORD dwDisp;
		//	INT32 iErr = RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1", &hKeyRoot);
			INT32 iErr = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				L"Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1",
				0, 0, 0, KEY_ALL_ACCESS, 0, &hKeyRoot, &dwDisp);
			if(iErr == ERROR_SUCCESS)
		//	if(m_cv.ver != -1)
			{
				RegSetValueEx(hKeyRoot, L"driver", 0, REG_SZ, (UINT8 *)L"azbthuart_pm.dll", sizeof(L"azbthuart_pm.dll"));
				RegCloseKey(hKeyRoot);
				TCHAR argPtr[] = L"card";
				HANDLE hDev = CreateFile(L"BTD0:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
				if(hDev != INVALID_HANDLE_VALUE)
				{
					int iErr = DeviceIoControl(hDev, IOCTL_SERVICE_START, argPtr, sizeof(argPtr[0])*(_tcslen(argPtr) + 1), 0, 0, 0, 0);
					CloseHandle(hDev);
				}
			}
			CloseHandle(m_hCardPresent);
		}
#endif
	}
#else
	HKEY hKey = 0;
	DWORD Size;
	int  en = 0, en2 = 0, run = 0, run2 = 0;
	BOOL bPresent = 0;	
	BOOL bReady = 0;

	fnWL_BIPCalibration fnCal;
	fnWL_RadioTest fnRadioTest;

	PFN_WL_GetStatus		pfn_WL_GetStatus		= 0;
	PFN_BT_GetStatus		pfn_BT_GetStatus		= 0;
	PFN_WL_TurnOn			pfn_WL_TurnOn			= 0;
	PFN_WL_TurnOff			pfn_WL_TurnOff			= 0;
	PFN_WL_GetStartupState	pfn_WL_GetStartupState	= 0;
	PFN_BT_GetStartupState	pfn_BT_GetStartupState	= 0;
	PFN_BT_TurnOn			pfn_BT_TurnOn			= 0;
	PFN_BT_TurnOff			pfn_BT_TurnOff			= 0;
	HMODULE					hLib					= 0;

	MSGQUEUEOPTIONS			sOptions;
	NDISUIO_DEVICE_NOTIFICATION	sDeviceNotification;
	HANDLE						hEthManQueue = NULL;
	sOptions.dwSize           = sizeof(MSGQUEUEOPTIONS);
    sOptions.dwFlags          = MSGQUEUE_ALLOW_BROKEN;
    sOptions.dwMaxMessages    = 0;
    sOptions.cbMaxMessage     = sizeof(NDISUIO_DEVICE_NOTIFICATION);
    sOptions.bReadAccess      = FALSE;
	sDeviceNotification.dwNotificationType = NDISUIO_NOTIFICATION_DEVICE_POWER_UP;
	_tcscpy(sDeviceNotification.ptcDeviceName, L"TIWI-R2");

    hEthManQueue = CreateMsgQueue(_T("NDISPWR_QUEUE"), &sOptions);
    RETAILMSG(hEthManQueue == NULL, (TEXT("afdp: Could not create NDISPWR_QUEUE message queue for notifying Ethman\r\n")));

	RETAILMSG(1, (TEXT("afdp: get WLAN startup status\r\n")));

	if(hLib = LoadLibrary(L"wl1271helper.dll"))
	{
		fnRadioTest				= (fnWL_RadioTest)GetProcAddress(hLib, L"WL_RadioTest");
		fnCal					= (fnWL_BIPCalibration)GetProcAddress(hLib, L"WL_BIPCalibration");

		pfn_WL_GetStatus		= (PFN_WL_GetStatus)	GetProcAddress(hLib, L"WL_GetStatus");
		pfn_BT_GetStatus		= (PFN_BT_GetStatus)	GetProcAddress(hLib, L"BT_GetStatus");
		pfn_WL_TurnOn			= (PFN_WL_TurnOn)		GetProcAddress(hLib, L"WL_TurnOn");
		pfn_WL_TurnOff			= (PFN_WL_TurnOff)		GetProcAddress(hLib, L"WL_TurnOff");
		pfn_WL_GetStartupState	= (PFN_WL_GetStartupState)GetProcAddress(hLib, L"WL_GetStartupState");
		pfn_BT_GetStartupState	= (PFN_BT_GetStartupState)GetProcAddress(hLib, L"BT_GetStartupState");
		pfn_BT_TurnOn			= (PFN_BT_TurnOn)		GetProcAddress(hLib, L"BT_TurnOn");
		pfn_BT_TurnOff			= (PFN_BT_TurnOff)		GetProcAddress(hLib, L"BT_TurnOff");

	}
	if(	!pfn_WL_GetStatus		|| !pfn_BT_GetStatus		|| !pfn_WL_TurnOn || !pfn_WL_TurnOff ||
		!pfn_WL_GetStartupState || !pfn_BT_GetStartupState	|| !pfn_BT_TurnOn || !pfn_BT_TurnOff)
	{
		if(hLib)
			FreeLibrary(hLib);

		if (hEthManQueue)
	        CloseHandle(hEthManQueue);
		RETAILMSG(1, (TEXT("afdp: error loading wl1271helper\r\n")));
		return -1;
	}
	bPresent = IsWirelessModulePresent();
	if(bPresent)
	{
		en = pfn_WL_GetStartupState();
		en2 = pfn_BT_GetStartupState();
	}
	else
	{
		RETAILMSG(1, (TEXT("afdp: WirelessModule isn't present\r\n")));
		en = 0;
		en2 = 0;
	}

	RETAILMSG(1, (TEXT("afdp: WLAN%senabled\r\n"), (en)?L" ":L" doesn't "));

	run = pfn_WL_GetStatus();

	int i = 4;
	do{
		Sleep(400);
		if(run2 = pfn_BT_GetStatus())
			break;
	}while(i--);
	// Check more time if NDISPWR already has changed it state
	// TODO: synchronize with NDISPWR

#if 0

	RETAILMSG(1, (TEXT("afdp: Start Power Down/UP \r\n")));
	
	if (run2)
		pfn_BT_TurnOff();
	if (run)
		pfn_WL_TurnOff(1);

	do
	{
		if( !pfn_BT_GetStatus() && !pfn_WL_GetStatus() )
			break;
		Sleep(400);
		RETAILMSG(1, (TEXT("afdp: Try shutdown the stacks \r\n")));
	}while(1);
	
	TIWIR2PowerOnOff ();

	run2 = run = 0;

	//return 0;
#endif

#if 0
	EnDisZeroConfig ( TRUE );
	return 0;
#endif

	__try
	{
		if(en)
		{
			if(!run)
			{
				pfn_WL_TurnOn(1);
				Sleep(200);
				run = pfn_WL_GetStatus();
			}

			if(run)
			{
				// Remove temporary NVS file
				//DeleteFile(NVS_FILE_PATH);

				//EnDisZeroConfig ( FALSE );

				//Sleep (1000);

				HANDLE nvs_map = CreateFile(NVS_FILE_PATH, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

				if(nvs_map != (HANDLE)-1)
				{
					CloseHandle(nvs_map);
					//if(fnRadioTest)
					//	fnRadioTest(0);
				}
				else
				{
					if(fnCal)
						fnCal(1, 1,1,1,1,1,1,1, 0);
				}

				//Sleep (1000);

				//EnDisZeroConfig ( TRUE );

			}
		}
		else
		{
			if(run)
				pfn_WL_TurnOff(1);
		}

		Sleep(100);

		if(en2)
		{
			if(!run2)
			{
				Sleep(100);
				pfn_BT_TurnOn();
				
			}
		}
		else
		{
			//Sleep(400);
			pfn_BT_TurnOff();
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		RETAILMSG(1, (TEXT("afdp: error of radio config\r\n")));
	}

	if(hLib)
		FreeLibrary(hLib);

	//if(!bReady)
	{
		bReady = WriteMsgQueue(
				hEthManQueue, 
				&sDeviceNotification,
				sizeof(NDISUIO_DEVICE_NOTIFICATION),
				0,
				0);
	}

	if (hEthManQueue)
        CloseHandle(hEthManQueue);

#endif
#define ROOT_HUB_RESTART_EVENT_NAME TEXT("RootHubEventName")
	while (1)
	{
		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, ROOT_HUB_RESTART_EVENT_NAME);

		if (hEvent)
		{
			DWORD dwResult = WaitForSingleObject( hEvent, INFINITE );

			RETAILMSG(1, (L"AFDP: Event Received: RootHubEventName \r\n"));

			SetDevicePower(L"EHC1:", POWER_NAME, D4);
			Sleep(1000);
			SetDevicePower(L"EHC1:", POWER_NAME, PwrDeviceUnspecified);

			CloseHandle(hEvent);
		}
		
		Sleep(1000);
	}
	return 0;
}
