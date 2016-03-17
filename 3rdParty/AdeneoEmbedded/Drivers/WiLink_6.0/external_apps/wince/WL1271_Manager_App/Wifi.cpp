/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/
 
/** \file   Wifi.cpp 
 *  \brief  WL1271 Wifi device management
 *
 *  \see    Wifi.h
 */

#include <stdlib.h>

#include <ndis.h>
#include <Nuiouser.h>
#include <winsock2.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <pm.h>
#include <ndispwr.h>

extern "C" {
#include <cu_osapi.h>
#include <console.h>
#include <cu_cmd.h>


#include <stdio.h>

#include "cu_osapi.h"
#include "TWDriver.h"
#include "convert.h"
#include "console.h"
#include "cu_common.h"
#include "cu_os.h"
#include "ipc_event.h"
#include "wpa_core.h"
#include "cu_cmd.h"
#include "oserr.h"
}

#include <BIN2REG.h>
#include <REG2BIN.h>

#include "Wifi.h"
#include "AS_Global.h"
#include "AS_Channel.h"
#include "Conf_Global.h"
#include "DTI_Channel.h"

BOOL DoNdisIOControl(DWORD , LPVOID , DWORD , LPVOID , DWORD *);

#define WLAN_DEVICE_NAME 			(const char*) (TEXT("TIWLNAPI1"))
#define OID_TI_WILINK_IOCTL         0xFF010200 

// TI WLAN driver definition
#define TIWLAN_DRV_NAME "tiwlan0"
#define SUPPL_IF_FILE "/var/run/tiwlan0"

#define NVS_FILE_PATH	L"\\windows\\nvs_map.bin"
#define NVS_REG_PATH	L"Comm\\tiwlnapi1\\Parms\\NVS"

/* Module control block */
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

// TI CLI object
static THandle g_hCmd;

BOOL WL_Init(void)
{
	DEBUGMSG(1, (TEXT("++WL_Init\r\n")));
	WSADATA WsaData;
	BOOL bRet = TRUE;

	if (WSAStartup(MAKEWORD(2,2), &WsaData) != 0)
	{
		DEBUGMSG(1,(TEXT("WSAStartup failed (error %ld)\n"), GetLastError()));
		bRet = FALSE;
	}
	else
	{
		g_hCmd = CuCmd_Create(TIWLAN_DRV_NAME, NULL, FALSE, NULL);
		
		if (g_hCmd == NULL)
		{
			bRet = FALSE;
		}
	}
	

	DEBUGMSG(1, (TEXT("--WL_Init\r\n")));
	return bRet;
}

BOOL WL_SaveStartupState(int save)
{
	HKEY hKey = 0;
	DWORD Size;
	int  en = 0;

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
	CuCmd_Destroy(hCmd);

	if(TI_OK != data.testCmd_u.P2GCal.oRadioStatus)
		MessageBox(hWnd, L"Radio test is failed\r\n",  L"WL1271", MB_OK);

	return 1;
}

BOOL WL_GetStatus(void)
{
	DEBUGMSG(1, (TEXT("++WL_GetStatus\r\n")));

	ULONG                    ulBufferLength, ulFlags;
	DWORD                    dwError;
	PIP_ADAPTER_ADDRESSES    pAdapterAddresses;
	DWORD GetAdaptAddRet = 0;

	ulFlags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;

	GetAdaptAddRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, NULL, NULL, &ulBufferLength);

	pAdapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(ulBufferLength);
   
	if (NULL == pAdapterAddresses)
	{
		DEBUGMSG(1,(TEXT("Insufficient Memory\n")));
		goto error;
	}

	GetAdaptAddRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, NULL, pAdapterAddresses, &ulBufferLength);

	dwError = GetLastError();

	if (GetAdaptAddRet == ERROR_SUCCESS)
	{
		while (pAdapterAddresses != NULL)
		{
			if(strncmp(pAdapterAddresses->AdapterName, WLAN_DEVICE_NAME, strlen(WLAN_DEVICE_NAME)) == 0)
			{
				DEBUGMSG(1, (TEXT("TIWLNAPI1 is enabled\n")));
				goto isactivated;
			}
			pAdapterAddresses = pAdapterAddresses->Next;
		}

		DEBUGMSG(1, (TEXT("TIWLNAPI1 is disabled\n")));
		goto isnotactivated;

	}
	else
	{
		DEBUGMSG(1, (TEXT("Can't get Adaptaters infos\n")));
		goto error;
	}

isactivated:
	DEBUGMSG(1, (TEXT("--WL_GetStatus\r\n")));
	return TRUE;

isnotactivated:
error:
	DEBUGMSG(1, (TEXT("--WL_GetStatus\r\n")));
	return FALSE;


}

BOOL WL_TurnOn(void)
{
	RETAILMSG(1, (TEXT("++WL_TurnOn\r\n")));
	DWORD dwErr = S_OK;
    BOOL bRet = FALSE;
	HANDLE hNdisPwr;
	NDISPWR_SAVEPOWERSTATE SavePowerState;
	TCHAR szName[MAX_PATH];
	TCHAR multiSz[257];
	int nChars;        


	TCHAR *WlanDevName = L"TIWLNAPI1";

	RETAILMSG(1, (TEXT("Activating Wifi driver...\r\n")));
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
		SavePowerState.CePowerState   = PwrDeviceUnspecified;
		
		RETAILMSG(1, (TEXT("Save D0 power state\r\n")));
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


	nChars = _sntprintf(
					szName, 
					MAX_PATH-1, 
					_T("%s\\%s"), 
					PMCLASS_NDIS_MINIPORT, 
					WlanDevName);

	Sleep(1250);
	szName[MAX_PATH-1]=0;

	RETAILMSG(1, (TEXT("Set D0 power state\r\n")));
	SetDevicePower(szName, POWER_NAME, PwrDeviceUnspecified);

	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	RETAILMSG(1, (TEXT("Bind adapter\r\n")));
	bRet = DoNdisIOControl(
					IOCTL_NDIS_BIND_ADAPTER, 
					multiSz,
					(_tcslen(multiSz)+2) * sizeof(TCHAR),
					NULL, 
					NULL);   

	RETAILMSG(1, (TEXT("--WL_TurnOn\r\n")));

	return bRet;
}

BOOL WL_TurnOff(void)
{
	RETAILMSG(1, (TEXT("++WL_TurnOff\r\n")));
	DWORD dwErr = S_OK;
    BOOL bRet = FALSE;
	HANDLE hNdisPwr;
	NDISPWR_SAVEPOWERSTATE SavePowerState;
	TCHAR szName[MAX_PATH];
	TCHAR multiSz[257];
	int nChars;        


	TCHAR *WlanDevName = L"TIWLNAPI1";

	DEBUGMSG(1, (TEXT("Desactivating Wifi driver...\r\n")));
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


	nChars = _sntprintf(
					szName, 
					MAX_PATH-1, 
					_T("%s\\%s"), 
					PMCLASS_NDIS_MINIPORT, 
					WlanDevName);

	szName[MAX_PATH-1]=0;

	SetDevicePower(szName, POWER_NAME, D4);

	
    StringCchCopy(multiSz, (cbBuffer / sizeof(TCHAR))-2, WlanDevName);

	multiSz[_tcslen(multiSz)+1] = _T('\0'); // Multi sz needs an extra null

	bRet = DoNdisIOControl(
					IOCTL_NDIS_UNBIND_ADAPTER, 
					multiSz,
					(_tcslen(multiSz)+2) * sizeof(TCHAR),
					NULL, 
					NULL);   

	RETAILMSG(1, (TEXT("--WL_TurnOff\r\n")));

	return bRet;
}


BOOL WL_DeInit(void)
{
	RETAILMSG(1, (TEXT("++WL_DeInit\r\n")));
	

	if (g_hCmd != NULL)
	{
		 CuCmd_Destroy(g_hCmd);
	}

	WSACleanup();
	RETAILMSG(1, (TEXT("--WL_DeInit\r\n")));
	return TRUE;
}

BOOL DoNdisIOControl(DWORD dwCommand, LPVOID pInBuffer,
                                  DWORD cbInBuffer, LPVOID pOutBuffer,
                                  DWORD * pcbOutBuffer)
{
	HANDLE hNdis;
	BOOL fResult = FALSE;
	DWORD dwErr = S_OK;

	hNdis = CreateFile(DD_NDIS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                        0, NULL);

	if (INVALID_HANDLE_VALUE != hNdis)
	{
		fResult = DeviceIoControl(hNdis, dwCommand, pInBuffer, cbInBuffer,
								  pOutBuffer, (pcbOutBuffer ? *pcbOutBuffer : 0),
								  pcbOutBuffer, NULL);
		dwErr = GetLastError();

		CloseHandle(hNdis);
	}


	return fResult;
}

BOOL WL_BIPCalibration (BOOL bSB1_14, 
						BOOL bSB1_4,
						BOOL bSB8_16,
						BOOL bSB34_48,
						BOOL bSB52_64,
						BOOL bSB100_116,
						BOOL bSB120_140,
						BOOL bSB149_165)
{
	RETAILMSG(1, (TEXT("++WL_BIPCalibration\r\n")));
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


	// Post process the file to registry
	CBIN2REG bin2reg;
	bin2reg.BinFileToReg(NVS_FILE_PATH, NVS_REG_PATH);

	// Remove temporary NVS file
	DeleteFile(NVS_FILE_PATH);

	// Restore MAC Address
	WL_SetMACAddress (szMACAddress);

	RETAILMSG(1, (TEXT("--WL_BIPCalibration\r\n")));
	return TRUE;
}

#define MAC_ADDR_LENGTH		17
#define MAC_IN_NVS_1		0x0B
#define MAC_IN_NVS_2		0x0A
#define MAC_IN_NVS_3		0x06
#define MAC_IN_NVS_4		0x05
#define MAC_IN_NVS_5		0x04
#define MAC_IN_NVS_6		0x03

// Buffer should be large enought to store a mac address
// "xx:xx:xx:xx:xx:xx" -> 17 WCHAR
BOOL WL_GetMACAddress (LPTSTR szMACAddress, DWORD dwSize)
{
	
	// Read the NVS from registry
	CREG2BIN reg2bin;
	LPBYTE pNVSBuffer;
	DWORD dwNVSBufferSize;
	
	if (szMACAddress == NULL || dwSize<(MAC_ADDR_LENGTH))
	{
		return FALSE;
	}

	if (reg2bin.RegToBuffer(NVS_REG_PATH, NVSVALUENAME, pNVSBuffer, &dwNVSBufferSize) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// 
	wsprintf(szMACAddress, L"%02X:%02X:%02X:%02X:%02X:%02X",
							pNVSBuffer[MAC_IN_NVS_1],
							pNVSBuffer[MAC_IN_NVS_2],
							pNVSBuffer[MAC_IN_NVS_3],
							pNVSBuffer[MAC_IN_NVS_4],
							pNVSBuffer[MAC_IN_NVS_5],
							pNVSBuffer[MAC_IN_NVS_6]);

	// Free memory
	LocalFree(pNVSBuffer);

	return TRUE;
}

BOOL WL_SetMACAddress (LPCTSTR szMACAddress)
{
	
	// Read the NVS from registry
	CREG2BIN reg2bin;
	LPBYTE pNVSBuffer;
	DWORD dwNVSBufferSize;
	
	if (szMACAddress == NULL || wcslen(szMACAddress) <(MAC_ADDR_LENGTH))
	{
		return FALSE;
	}

	if (reg2bin.RegToBuffer(NVS_REG_PATH, NVSVALUENAME, pNVSBuffer, &dwNVSBufferSize) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// Extract info from the string
	int piMACBuffer[6]={0};
	int iscan = swscanf(szMACAddress, L"%2X:%2X:%2X:%2X:%2X:%2X",
							&piMACBuffer[5],
							&piMACBuffer[4],
							&piMACBuffer[3],
							&piMACBuffer[2],
							&piMACBuffer[1],
							&piMACBuffer[0]);
	if (iscan != 6 && 
		((piMACBuffer[5] & 0xFF) <= 0xFF) &&
		((piMACBuffer[4] & 0xFF) <= 0xFF) &&
		((piMACBuffer[3] & 0xFF) <= 0xFF) &&
		((piMACBuffer[2] & 0xFF) <= 0xFF) &&
		((piMACBuffer[1] & 0xFF) <= 0xFF) &&
		((piMACBuffer[0] & 0xFF) <= 0xFF) )
	{
		LocalFree(pNVSBuffer);
		return FALSE;
	}

	// Update buffer
	pNVSBuffer[MAC_IN_NVS_1] = piMACBuffer[5]&0xFF;
	pNVSBuffer[MAC_IN_NVS_2] = piMACBuffer[4]&0xFF;
	pNVSBuffer[MAC_IN_NVS_3] = piMACBuffer[3]&0xFF;
	pNVSBuffer[MAC_IN_NVS_4] = piMACBuffer[2]&0xFF;
	pNVSBuffer[MAC_IN_NVS_5] = piMACBuffer[1]&0xFF;
	pNVSBuffer[MAC_IN_NVS_6] = piMACBuffer[0]&0xFF;

	// Put back data to registry
	CBIN2REG bin2reg;
	HRESULT hr = bin2reg.BufferToReg(pNVSBuffer, dwNVSBufferSize, NVS_REG_PATH, NVSVALUENAME, NVSCHUNCKSIZE);
	
	// Free Memory
	LocalFree(pNVSBuffer);

	return (ERROR_SUCCESS == hr);
}

BOOL WL_RoamingEnable ()
{
	ConParm_t roamingParam[] = { CON_LAST_PARM };
	CuCmd_RoamingEnable(g_hCmd, roamingParam, 0);
	return TRUE;
}

BOOL WL_RoamingDisable ()
{
	ConParm_t roamingParam[] = { CON_LAST_PARM };
	CuCmd_RoamingDisable(g_hCmd, roamingParam, 0);
	return TRUE;
}

BOOL WL_RoamingSetLowPassFilter(TI_UINT32 lowPassFilter)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,lowPassFilter };
	CuCmd_RoamingLowPassFilter(g_hCmd, roamingParam, 1);
	return TRUE;
}

BOOL WL_RoamingSetQualityThreshold(TI_UINT32 qualityThreshold)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,qualityThreshold };
	CuCmd_RoamingQualityIndicator(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetDataRetryThreshold(TI_UINT32 dataRetry)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,dataRetry };
	CuCmd_RoamingDataRetryThreshold(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetExpectedTbttForBssLoss(TI_UINT32 expectedTbtt)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,expectedTbtt };
	CuCmd_RoamingNumExpectedTbttForBSSLoss(g_hCmd,roamingParam, 1);
	return TRUE;
}

BOOL WL_RoamingSetTxRateThreshold(TI_UINT32 txRateThreshold)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,txRateThreshold };
	CuCmd_RoamingTxRateThreshold(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetLowRssiThreshold(TI_UINT32 lowRssiThreshold)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,lowRssiThreshold };
	CuCmd_RoamingLowRssiThreshold(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetLowSnrThreshold(TI_UINT32 lowSnrThreshold)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,lowSnrThreshold };
	CuCmd_RoamingLowSnrThreshold(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetLowQualityForBackgroundScan(TI_UINT32 quality)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,quality };
	CuCmd_RoamingLowQualityForBackgroungScanCondition(g_hCmd,roamingParam , 1);
	return TRUE;
}

BOOL WL_RoamingSetNormalQualityForBackgroundScan(TI_UINT32 quality)
{
	ConParm_t roamingParam[] = { NULL,0,0,0,quality };
	CuCmd_RoamingNormalQualityForBackgroungScanCondition(g_hCmd,roamingParam , 1);
	return TRUE;
}


VOID WL_GetASGlobalParameters(AS_Global_t* pASGlobalData)
{
	const CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	const scan_Params_t* pScanParams = &(pCuCmd->appScanParams);
	
	pASGlobalData->scanType = pScanParams->scanType;
	pASGlobalData->bandIndex = pScanParams->band;
	pASGlobalData->numOfProbeReqs = pScanParams->probeReqNumber;
	pASGlobalData->bitrate = pScanParams->probeRequestRate;
	pASGlobalData->triggeringTid = pScanParams->Tid;
	pASGlobalData->nbChannels = pScanParams->numOfChannels;

	mbstowcs(pASGlobalData->SSID,pScanParams->desiredSsid.str,pScanParams->desiredSsid.len);
	
}

BOOL WL_SetASGlobalParameters(const AS_Global_t* pAsGlobalData)
{
	CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	scan_Params_t* pScanParams = &(pCuCmd->appScanParams);

	pScanParams->scanType = pAsGlobalData->scanType;
	pScanParams->band = pAsGlobalData->bandIndex;
	pScanParams->probeReqNumber = pAsGlobalData->numOfProbeReqs;
	pScanParams->probeRequestRate = pAsGlobalData->bitrate;
	pScanParams->Tid = pAsGlobalData->triggeringTid;
	pScanParams->numOfChannels = pAsGlobalData->nbChannels;

	pScanParams->desiredSsid.len = strlen((char*)pAsGlobalData->SSID);
	if(pScanParams->desiredSsid.len > MAX_SSID_LEN)
	{
		pScanParams->desiredSsid.len = 0;	
		return FALSE;
	}
	else
	{
		wcstombs(pScanParams->desiredSsid.str,pAsGlobalData->SSID,pScanParams->desiredSsid.len);
	}

	return TRUE;
}

BOOL WL_GetASChannelParameters(TI_UINT8 ChannelIndex, AS_Channel_t* pAsChannelData)
{
	const CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	const scan_Params_t* pScanParams = &(pCuCmd->appScanParams);
	int i;

	if (ChannelIndex > MAX_NUMBER_OF_CHANNELS_PER_SCAN)
		return FALSE;
	pAsChannelData->ChanIndex = ChannelIndex;
	pAsChannelData->ChanNumber = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.channel;
	pAsChannelData->ETCondition = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.earlyTerminationEvent;
	pAsChannelData->ETFrameNumber = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.ETMaxNumOfAPframes;
	pAsChannelData->MaxDwellTime = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.maxChannelDwellTime;
	pAsChannelData->MinDwellTime = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.minChannelDwellTime;
	pAsChannelData->TxPowerLevel = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.txPowerDbm;

	for (i = 0;i < MAC_ADDR_LEN;i++)
	{
		pAsChannelData->BSSID[i] = pScanParams->channelEntry[ChannelIndex].normalChannelEntry.bssId[i];
	}
	return TRUE;
}


BOOL WL_SetASChannelParameters(const AS_Channel_t* pAsChannelData)
{
	CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	scan_Params_t* pScanParams = &(pCuCmd->appScanParams);
	int i;

	if(pAsChannelData->ChanIndex > MAX_NUMBER_OF_CHANNELS_PER_SCAN)
		return FALSE;

	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.maxChannelDwellTime = pAsChannelData->MaxDwellTime ;
	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.minChannelDwellTime = pAsChannelData->MinDwellTime ;
	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.earlyTerminationEvent = pAsChannelData->ETCondition ;
	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.ETMaxNumOfAPframes = pAsChannelData->ETFrameNumber ;
	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.txPowerDbm = pAsChannelData->TxPowerLevel ;
	pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.channel = pAsChannelData->ChanNumber ;
	// TMacAddr type was not found -> it won't work.
	for (i=0;i < MAC_ADDR_LEN;i++)
	{
		pScanParams->channelEntry[pAsChannelData->ChanIndex].normalChannelEntry.bssId[i] = pAsChannelData->BSSID[i];
	}
	return TRUE;
}

VOID WL_GetGlobalParameters(Conf_Global_t* pGlobalData)
{
    const CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    const scan_Policy_t* pScanPolicy = &(pCuCmd->scanPolicy);

	pGlobalData->normalScanInterval = pScanPolicy->normalScanInterval ;
	pGlobalData->deterioratingScanInterval = pScanPolicy->deterioratingScanInterval ;
	pGlobalData->maxTrackFailures = pScanPolicy->maxTrackFailures ;
	pGlobalData->bssListSize = pScanPolicy->BSSListSize ;
	pGlobalData->bssNumberToStartDisc = pScanPolicy->BSSNumberToStartDiscovery ;
	pGlobalData->nbOfBands = pScanPolicy->numOfBands ;
}
VOID WL_SetGlobalParameters(const Conf_Global_t* pGlobalData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    scan_Policy_t* pScanPolicy = &(pCuCmd->scanPolicy);
	
	pScanPolicy->normalScanInterval = pGlobalData->normalScanInterval ;
	pScanPolicy->deterioratingScanInterval = pGlobalData->deterioratingScanInterval ;
	pScanPolicy->maxTrackFailures = pGlobalData->maxTrackFailures ;
	pScanPolicy->BSSListSize = pGlobalData->bssListSize ;
	pScanPolicy->BSSNumberToStartDiscovery = pGlobalData->bssNumberToStartDisc ;
	pScanPolicy->numOfBands = pGlobalData->nbOfBands ;

}

VOID WL_GetDTIChannelParameters(TI_UINT8 bandNumber,DTI_Channel_t* pDtiChannelData)
{
	int i;
	const CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	const scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[bandNumber]);

	pDtiChannelData->bandNumber = bandNumber;

	pDtiChannelData->bandIndex = pBandPolicy->band;
	pDtiChannelData->bssThreshold = pBandPolicy->rxRSSIThreshold;
	pDtiChannelData->channelNumberForDiscCycle = pBandPolicy->numOfChannlesForDiscovery;
	pDtiChannelData->nbChannels = pBandPolicy->numOfChannles;

	for (i=0;i < pBandPolicy->numOfChannles;i++)
	{
		pDtiChannelData->channelArray[i] = pBandPolicy->channelList[i];
	}
}

BOOL WL_SetDTIChannelParameters(const DTI_Channel_t* pDtiChannelData)
{
	int i;
	CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	if(pDtiChannelData->bandNumber >= RADIO_BAND_NUM_OF_BANDS)
		return FALSE;

	if(pDtiChannelData->nbChannels >= MAX_BAND_POLICY_CHANNLES)
		return FALSE;

	scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[pDtiChannelData->bandNumber]);

	pBandPolicy->band = pDtiChannelData->bandIndex ;
	pBandPolicy->rxRSSIThreshold = pDtiChannelData->bssThreshold ;
	pBandPolicy->numOfChannlesForDiscovery = pDtiChannelData->channelNumberForDiscCycle ;
	pBandPolicy->numOfChannles = pDtiChannelData->nbChannels ;

	for (i=0;i < pBandPolicy->numOfChannles;i++)
	{
		pBandPolicy->channelList[i] = pDtiChannelData->channelArray[i] ;
	}
	return TRUE;
}

BOOL WL_SetScanAppImmScanParameters(TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[pScanAppData->bandIndex]);

    pBandPolicy->immediateScanMethod.scanType = pScanAppData->scanType;
	pBandPolicy->band = pScanAppData->bandIndex;

    switch (pBandPolicy->immediateScanMethod.scanType)
    {
        case SCAN_TYPE_NORMAL_ACTIVE:
        case SCAN_TYPE_NORMAL_PASSIVE:
            pBandPolicy->immediateScanMethod.method.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->immediateScanMethod.method.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->immediateScanMethod.method.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->immediateScanMethod.method.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
			pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_TRIGGERED_ACTIVE:
        case SCAN_TYPE_TRIGGERED_PASSIVE:
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.triggeringTid = pScanAppData->triggeringTid;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
            pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_SPS:
			pBandPolicy->immediateScanMethod.method.spsMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->immediateScanMethod.method.spsMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->immediateScanMethod.method.spsMethodParams.scanDuration = pScanAppData->scanDuration;
            break;

        default:
            pBandPolicy->immediateScanMethod.scanType = SCAN_TYPE_NO_SCAN;
            break;
    }
	return TRUE;
}

BOOL WL_SetScanAppTrackParameters(TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[pScanAppData->bandIndex]);

    pBandPolicy->trackingMethod.scanType = pScanAppData->scanType;
	pBandPolicy->band = pScanAppData->bandIndex;

    switch (pBandPolicy->trackingMethod.scanType)
    {
        case SCAN_TYPE_NORMAL_ACTIVE:
        case SCAN_TYPE_NORMAL_PASSIVE:
            pBandPolicy->trackingMethod.method.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->trackingMethod.method.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->trackingMethod.method.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->trackingMethod.method.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
			pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_TRIGGERED_ACTIVE:
        case SCAN_TYPE_TRIGGERED_PASSIVE:
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.triggeringTid = pScanAppData->triggeringTid;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
            pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_SPS:
			pBandPolicy->trackingMethod.method.spsMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->trackingMethod.method.spsMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->trackingMethod.method.spsMethodParams.scanDuration = pScanAppData->scanDuration;
            break;

        default:
            pBandPolicy->trackingMethod.scanType = SCAN_TYPE_NO_SCAN;
            break;
    }
	return TRUE;

}

BOOL WL_SetScanAppDiscParameters(TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[pScanAppData->bandIndex]);

    //if(OK != CuCommon_SetBuffer(pCuCmd->hCuCommon, SCAN_MNGR_SET_CONFIGURATION,
    //    &pCuCmd->scanPolicy, sizeof(scan_Policy_t))) return FALSE;

    pBandPolicy->discoveryMethod.scanType = pScanAppData->scanType;
	pBandPolicy->band = pScanAppData->bandIndex;

    switch (pBandPolicy->discoveryMethod.scanType)
    {
        case SCAN_TYPE_NORMAL_ACTIVE:
        case SCAN_TYPE_NORMAL_PASSIVE:
            pBandPolicy->discoveryMethod.method.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->discoveryMethod.method.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->discoveryMethod.method.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->discoveryMethod.method.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
			pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_TRIGGERED_ACTIVE:
        case SCAN_TYPE_TRIGGERED_PASSIVE:
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.triggeringTid = pScanAppData->triggeringTid;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime = pScanAppData->maxChannelDwellTime;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime = pScanAppData->minChannelDwellTime;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate = pScanAppData->bitrate;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs = pScanAppData->numOfProbeReqs;
            pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm = pScanAppData->txPowerDbm;
            break;

        case SCAN_TYPE_SPS:
			pBandPolicy->discoveryMethod.method.spsMethodParams.earlyTerminationEvent = pScanAppData->earlyTerminationEvent;
            pBandPolicy->discoveryMethod.method.spsMethodParams.ETMaxNumberOfApFrames = pScanAppData->ETMaxNumberOfApFrames;
            pBandPolicy->discoveryMethod.method.spsMethodParams.scanDuration = pScanAppData->scanDuration;
            break;

        default:
            pBandPolicy->discoveryMethod.scanType = SCAN_TYPE_NO_SCAN;
            break;
    }
	return TRUE;
}

BOOL WL_GetScanAppImmScanParameters(UINT bandIndex, TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	
	scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[bandIndex]);

	pScanAppData->bandIndex = pBandPolicy->band;
	pScanAppData->scanType = pBandPolicy->immediateScanMethod.scanType;
	switch (pScanAppData->scanType)
	{
	case SCAN_TYPE_NORMAL_ACTIVE:
	case SCAN_TYPE_NORMAL_PASSIVE:
	pScanAppData->maxChannelDwellTime = pBandPolicy->immediateScanMethod.method.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->immediateScanMethod.method.basicMethodParams.minChannelDwellTime;
    pScanAppData->earlyTerminationEvent = pBandPolicy->immediateScanMethod.method.basicMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->immediateScanMethod.method.basicMethodParams.ETMaxNumberOfApFrames;
	pScanAppData->bitrate = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.triggeringTid;
	break;

	case SCAN_TYPE_TRIGGERED_ACTIVE:
    case SCAN_TYPE_TRIGGERED_PASSIVE:
	pScanAppData->maxChannelDwellTime = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime;
    pScanAppData->earlyTerminationEvent = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames;
	pScanAppData->bitrate = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.triggeringTid;
	break;

	case SCAN_TYPE_SPS:
	pScanAppData->scanDuration = pBandPolicy->immediateScanMethod.method.spsMethodParams.scanDuration;
    pScanAppData->earlyTerminationEvent = pBandPolicy->immediateScanMethod.method.spsMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->immediateScanMethod.method.spsMethodParams.ETMaxNumberOfApFrames;

	pScanAppData->maxChannelDwellTime = pBandPolicy->immediateScanMethod.method.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->immediateScanMethod.method.basicMethodParams.minChannelDwellTime;
	pScanAppData->bitrate = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->immediateScanMethod.method.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->immediateScanMethod.method.TidTriggerdMethodParams.triggeringTid;

		break;

	default:
		pBandPolicy->immediateScanMethod.scanType = SCAN_TYPE_NO_SCAN;
		pScanAppData->scanType = pBandPolicy->immediateScanMethod.scanType;
        break;

	}
	return FALSE;
}

BOOL WL_GetScanAppTrackParameters(UINT bandIndex, TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[bandIndex]);

	pScanAppData->bandIndex = pBandPolicy->band;
	pScanAppData->scanType = pBandPolicy->trackingMethod.scanType;
	switch (pScanAppData->scanType)
	{
	case SCAN_TYPE_NORMAL_ACTIVE:
	case SCAN_TYPE_NORMAL_PASSIVE:
	pScanAppData->maxChannelDwellTime = pBandPolicy->trackingMethod.method.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->trackingMethod.method.basicMethodParams.minChannelDwellTime;
    pScanAppData->earlyTerminationEvent = pBandPolicy->trackingMethod.method.basicMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->trackingMethod.method.basicMethodParams.ETMaxNumberOfApFrames;
	pScanAppData->bitrate = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.triggeringTid;
	break;
	case SCAN_TYPE_TRIGGERED_ACTIVE:
    case SCAN_TYPE_TRIGGERED_PASSIVE:
	pScanAppData->maxChannelDwellTime = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime;
    pScanAppData->earlyTerminationEvent = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames;
	pScanAppData->bitrate = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.triggeringTid;

		break;

	case SCAN_TYPE_SPS:
	pScanAppData->scanDuration = pBandPolicy->trackingMethod.method.spsMethodParams.scanDuration;
    pScanAppData->earlyTerminationEvent = pBandPolicy->trackingMethod.method.spsMethodParams.earlyTerminationEvent;
    pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->trackingMethod.method.spsMethodParams.ETMaxNumberOfApFrames;

	pScanAppData->maxChannelDwellTime = pBandPolicy->trackingMethod.method.basicMethodParams.maxChannelDwellTime;
    pScanAppData->minChannelDwellTime = pBandPolicy->trackingMethod.method.basicMethodParams.minChannelDwellTime;
	pScanAppData->bitrate = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.bitrate;
	pScanAppData->numOfProbeReqs = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
	pScanAppData->txPowerDbm = pBandPolicy->trackingMethod.method.basicMethodParams.probReqParams.txPowerDbm;
	pScanAppData->triggeringTid = pBandPolicy->trackingMethod.method.TidTriggerdMethodParams.triggeringTid;

		break;

	default:
		pBandPolicy->trackingMethod.scanType = SCAN_TYPE_NO_SCAN;
		pScanAppData->scanType = pBandPolicy->trackingMethod.scanType;
        break;

	}
	return FALSE;
}

VOID WL_StartScan()
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	CuCmd_StartScan(pCuCmd,NULL,0);
}

VOID WL_StopScan()
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	CuCmd_StopScan(pCuCmd,NULL,0);
}

BOOL WL_GetScanAppDiscParameters(UINT bandIndex, TScanAppData* pScanAppData)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;

	scan_bandPolicy_t* pBandPolicy = &(pCuCmd->scanPolicy.bandScanPolicy[bandIndex]);

	pScanAppData->bandIndex = pBandPolicy->band;
	pScanAppData->scanType = pBandPolicy->discoveryMethod.scanType;
	switch (pScanAppData->scanType)
	{
	case SCAN_TYPE_NORMAL_ACTIVE:
	case SCAN_TYPE_NORMAL_PASSIVE:
		pScanAppData->maxChannelDwellTime = pBandPolicy->discoveryMethod.method.basicMethodParams.maxChannelDwellTime;
		pScanAppData->minChannelDwellTime = pBandPolicy->discoveryMethod.method.basicMethodParams.minChannelDwellTime;
		pScanAppData->earlyTerminationEvent = pBandPolicy->discoveryMethod.method.basicMethodParams.earlyTerminationEvent;
		pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->discoveryMethod.method.basicMethodParams.ETMaxNumberOfApFrames;
		pScanAppData->bitrate = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.bitrate;
		pScanAppData->numOfProbeReqs = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
		pScanAppData->txPowerDbm = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.txPowerDbm;
		pScanAppData->triggeringTid = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.triggeringTid;
		break;

	case SCAN_TYPE_TRIGGERED_ACTIVE:
	case SCAN_TYPE_TRIGGERED_PASSIVE:
		pScanAppData->maxChannelDwellTime = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.maxChannelDwellTime;
		pScanAppData->minChannelDwellTime = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.minChannelDwellTime;
		pScanAppData->earlyTerminationEvent = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.earlyTerminationEvent;
		pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.ETMaxNumberOfApFrames;
		pScanAppData->bitrate = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.bitrate;
		pScanAppData->numOfProbeReqs = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.numOfProbeReqs;
		pScanAppData->txPowerDbm = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.basicMethodParams.probReqParams.txPowerDbm;
		pScanAppData->triggeringTid = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.triggeringTid;
		break;

	case SCAN_TYPE_SPS:
		pScanAppData->scanDuration = pBandPolicy->discoveryMethod.method.spsMethodParams.scanDuration;
		pScanAppData->earlyTerminationEvent = pBandPolicy->discoveryMethod.method.spsMethodParams.earlyTerminationEvent;
		pScanAppData->ETMaxNumberOfApFrames = pBandPolicy->discoveryMethod.method.spsMethodParams.ETMaxNumberOfApFrames;

		pScanAppData->maxChannelDwellTime = pBandPolicy->discoveryMethod.method.basicMethodParams.maxChannelDwellTime;
		pScanAppData->minChannelDwellTime = pBandPolicy->discoveryMethod.method.basicMethodParams.minChannelDwellTime;
		pScanAppData->bitrate = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.bitrate;
		pScanAppData->numOfProbeReqs = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.numOfProbeReqs;
		pScanAppData->txPowerDbm = pBandPolicy->discoveryMethod.method.basicMethodParams.probReqParams.txPowerDbm;
		pScanAppData->triggeringTid = pBandPolicy->discoveryMethod.method.TidTriggerdMethodParams.triggeringTid;
		break;

	default:
		pBandPolicy->discoveryMethod.scanType = SCAN_TYPE_NO_SCAN;
		pScanAppData->scanType = pBandPolicy->discoveryMethod.scanType;
		break;
	}

	return FALSE;
}

BOOL WL_GetRoamingStatus(BOOL &bRoamingEnable)
{
    CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
    roamingMngrConfigParams_t   roamingMngrConfigParams;
    
    if(OK != CuCommon_GetBuffer(pCuCmd->hCuCommon, ROAMING_MNGR_APPLICATION_CONFIGURATION,
        &roamingMngrConfigParams, sizeof(roamingMngrConfigParams_t))) return FALSE;
    bRoamingEnable = roamingMngrConfigParams.roamingMngrConfig.enableDisable == ROAMING_ENABLED ;  
	return TRUE;
}

BOOL WL_GetRoamingParameters(roamingMngrConfigParams_t &roamingMngrConfigParams)
{
	CuCmd_t* pCuCmd = (CuCmd_t*)g_hCmd;
	roamingMngrConfigParams_t tempRoamingParams;
    if(OK != CuCommon_GetBuffer(pCuCmd->hCuCommon, ROAMING_MNGR_APPLICATION_CONFIGURATION,
        &tempRoamingParams, sizeof(roamingMngrConfigParams_t))) return FALSE;
	roamingMngrConfigParams = tempRoamingParams;
	return TRUE;
}

VOID WL_SetRoamingParameters(const roamingMngrConfigParams_t roamingMngrConfigParams)
{
	WL_RoamingSetLowPassFilter(roamingMngrConfigParams.roamingMngrConfig.lowPassFilterRoamingAttempt);
	WL_RoamingSetQualityThreshold(roamingMngrConfigParams.roamingMngrConfig.apQualityThreshold);
	WL_RoamingSetDataRetryThreshold(roamingMngrConfigParams.roamingMngrThresholdsConfig.dataRetryThreshold);
	WL_RoamingSetExpectedTbttForBssLoss(roamingMngrConfigParams.roamingMngrThresholdsConfig.numExpectedTbttForBSSLoss);
	WL_RoamingSetTxRateThreshold(roamingMngrConfigParams.roamingMngrThresholdsConfig.txRateThreshold);
	WL_RoamingSetLowRssiThreshold(roamingMngrConfigParams.roamingMngrThresholdsConfig.lowRssiThreshold);
	WL_RoamingSetLowSnrThreshold(roamingMngrConfigParams.roamingMngrThresholdsConfig.lowSnrThreshold);
	WL_RoamingSetLowQualityForBackgroundScan(roamingMngrConfigParams.roamingMngrThresholdsConfig.lowQualityForBackgroungScanCondition);
	WL_RoamingSetNormalQualityForBackgroundScan(roamingMngrConfigParams.roamingMngrThresholdsConfig.normalQualityForBackgroungScanCondition);
}

// Stubs for all OS
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


