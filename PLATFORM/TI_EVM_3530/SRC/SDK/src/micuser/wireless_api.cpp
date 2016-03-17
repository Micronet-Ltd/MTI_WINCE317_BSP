//-----------------------------------------------------------------------------
// Copyright 2011 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  wireless_api.cpp
//
//  This file contains wireless module API code.
//
//  Created by Vitaly Ilyasov    2011
//-----------------------------------------------------------------------------



#include <wireless_api.h>
#include <pm.h>
#include <bsp.h>

#define WIRELESS_API_MUTEX		TEXT("Mutex_MIC_WirelessModulePower")
#define BT_API_MUTEX			TEXT("Mutex_MIC_BTPower")

typedef BOOL 	(*PFN_WL_GetStatus		)(void);
typedef BOOL	(*PFN_WL_TurnOn			)(UINT32 save);	
typedef BOOL	(*PFN_WL_TurnOff		)(UINT32 save);
typedef BOOL	(*PFN_WL_SaveStartupState)(UINT32 val);	
typedef BOOL	(*PFN_BT_GetStatus		)(void);
typedef BOOL	(*PFN_BT_TurnOn			)(void);
typedef BOOL	(*PFN_BT_TurnOff		)(void);	
typedef BOOL	(*PFN_BT_SaveStartupState)(UINT32 val);	
typedef struct
{
	PFN_WL_GetStatus		pfn_WL_GetStatus;
	PFN_BT_GetStatus		pfn_BT_GetStatus;
	PFN_WL_TurnOn			pfn_WL_TurnOn;
	PFN_WL_TurnOff			pfn_WL_TurnOff;
	PFN_WL_SaveStartupState	pfn_WL_SaveStartupState;
	PFN_BT_SaveStartupState	pfn_BT_SaveStartupState;
	PFN_BT_TurnOn			pfn_BT_TurnOn;
	PFN_BT_TurnOff			pfn_BT_TurnOff;
} WL_FUNCS;

HINSTANCE	LoadLibFuncs( WL_FUNCS* pFuncs )
{
	HINSTANCE	hLib;
	if(hLib = LoadLibrary( L"wl1271helper.dll" ))
	{
		pFuncs->pfn_WL_GetStatus		= (PFN_WL_GetStatus)	GetProcAddress(hLib, L"WL_GetStatus");
		pFuncs->pfn_BT_GetStatus		= (PFN_BT_GetStatus)	GetProcAddress(hLib, L"BT_GetStatus");
		pFuncs->pfn_WL_TurnOn			= (PFN_WL_TurnOn)		GetProcAddress(hLib, L"WL_TurnOn");
		pFuncs->pfn_WL_TurnOff			= (PFN_WL_TurnOff)		GetProcAddress(hLib, L"WL_TurnOff");
		pFuncs->pfn_WL_SaveStartupState	= (PFN_WL_SaveStartupState)GetProcAddress(hLib, L"WL_SaveStartupState");
		pFuncs->pfn_BT_SaveStartupState	= (PFN_BT_SaveStartupState)GetProcAddress(hLib, L"BT_SaveStartupState");
		pFuncs->pfn_BT_TurnOn			= (PFN_BT_TurnOn)		GetProcAddress(hLib, L"BT_TurnOn");
		pFuncs->pfn_BT_TurnOff			= (PFN_BT_TurnOff)		GetProcAddress(hLib, L"BT_TurnOff");

	}
	if(	!pFuncs->pfn_WL_GetStatus		 || !pFuncs->pfn_BT_GetStatus		 || !pFuncs->pfn_WL_TurnOn || !pFuncs->pfn_WL_TurnOff ||
		!pFuncs->pfn_WL_SaveStartupState ||	!pFuncs->pfn_BT_SaveStartupState || !pFuncs->pfn_BT_TurnOn || !pFuncs->pfn_BT_TurnOff)
	{
		if(hLib)
		{
			FreeLibrary(hLib);
			hLib = 0;
		}
		RETAILMSG(1,(L"error loading wl1271helper\r\n"));
	}
	return hLib;
}

UINT32 MIC_WirelessModulePower(UINT32 power, UINT32 save)
{
	UINT32		ret = 0;
	BOOL		WlRet = 0, BtRet = 0;
	BOOL		WlStatus, BTStatus;
	HANDLE		hMutexWl = 0, hMutexBt = 0;
	HINSTANCE	hLib = 0;
	WL_FUNCS	Funcs	= {0};

	if (power > 1 || save > 1)
		return WIRELESS_ERROR_INVALID_PARAMETER;

	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;

	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	WlStatus = Funcs.pfn_WL_GetStatus();
	BTStatus = Funcs.pfn_BT_GetStatus();

	if(power && WlStatus && BTStatus)
		return WIRELESS_ERROR_ALREADY_ON;

	if(!power && !WlStatus && !BTStatus)
		return WIRELESS_ERROR_ALREADY_OFF;

	hMutexWl = CreateMutex(NULL, FALSE, WIRELESS_API_MUTEX);
	if (!hMutexWl || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		FreeLibrary(hLib);
		if(hMutexWl)
			CloseHandle(hMutexWl);
		return WIRELESS_ERROR_ACCESS_DENIED;
	}

	hMutexBt = CreateMutex(NULL, FALSE, BT_API_MUTEX);
	if (!hMutexBt || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		FreeLibrary(hLib);
		CloseHandle(hMutexWl);
		if(hMutexBt)
			CloseHandle(hMutexBt);
		return WIRELESS_ERROR_ACCESS_DENIED;
	}
	ret		= WIRELESS_ERROR_SUCCESS;
	WlRet	= 1;
	BtRet	= 1;

	if(power)
	{
		if(!WlStatus)
			WlRet = Funcs.pfn_WL_TurnOn(save);

		if(!BTStatus && WlRet)
			BtRet = Funcs.pfn_BT_TurnOn();
	}	
	else
	{
		if(WlStatus)
			WlRet = Funcs.pfn_WL_TurnOff(save);

		if(BTStatus && WlRet)
			BtRet = Funcs.pfn_BT_TurnOff();
	}
	if(!WlRet || !WlRet)
	{
		ret = WIRELESS_ERROR_DRIVER_ERROR;
	}
	else
	{
		if(save)
		{
			if(!Funcs.pfn_WL_SaveStartupState(power)	|| !Funcs.pfn_BT_SaveStartupState(power))
				ret = WIRELESS_ERROR_REGISTRY_UPDATING_ERROR;
		}
	}

	FreeLibrary(hLib);
	CloseHandle(hMutexWl);
	CloseHandle(hMutexBt);
	 
	return ret;
}
UINT32 MIC_WirelessModuleGetPowerStatus(UINT32* power)
{
	HINSTANCE	hLib;
	WL_FUNCS	Funcs	= {0};
	
	if (!power)
		return WIRELESS_ERROR_INVALID_PARAMETER;
	
	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;
	
	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	if(Funcs.pfn_WL_GetStatus() && Funcs.pfn_BT_GetStatus())
		*power = 1;
	else
		*power = 0;

	FreeLibrary(hLib);
	
	return WIRELESS_ERROR_SUCCESS;
}
extern "C" __declspec(dllexport) UINT32 MIC_WiFiEnable(UINT32 power, UINT32 save)
{
	UINT32 ret = WIRELESS_ERROR_SUCCESS;
	BOOL fOn;
	HINSTANCE	hLib;
	WL_FUNCS	Funcs	= {0};

	if (power > 1 || save > 1)
		return WIRELESS_ERROR_INVALID_PARAMETER;

	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;

	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	fOn = Funcs.pfn_WL_GetStatus();
	
	if(power && fOn)
		return WIRELESS_ERROR_ALREADY_ON;

	if(!power && !fOn)
		return WIRELESS_ERROR_ALREADY_OFF;

	HANDLE hMutexWl = CreateMutex(NULL, FALSE, WIRELESS_API_MUTEX);
	if (!hMutexWl || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if(hMutexWl)
			CloseHandle(hMutexWl);
		return WIRELESS_ERROR_ACCESS_DENIED;
	}

	if(power)
	{
		if(!Funcs.pfn_WL_TurnOn(save))
			ret = WIRELESS_ERROR_DRIVER_ERROR;
	}
	else
	{
		if(!Funcs.pfn_WL_TurnOff(save))
			ret = WIRELESS_ERROR_DRIVER_ERROR;
	}
	
	if(save && (WIRELESS_ERROR_SUCCESS == ret))
	{
		if(!Funcs.pfn_WL_SaveStartupState(power))
			ret = WIRELESS_ERROR_REGISTRY_UPDATING_ERROR;
	}
	
	FreeLibrary(hLib);
	CloseHandle(hMutexWl);
	return ret;
}
extern "C" __declspec(dllexport) UINT32 MIC_BluetoothEnable(UINT32 power, UINT32 save)
{
	UINT32		ret = WIRELESS_ERROR_SUCCESS;
	BOOL		fOn;
	HINSTANCE	hLib;
	WL_FUNCS	Funcs	= {0};

	if (power > 1 || save > 1)
		return WIRELESS_ERROR_INVALID_PARAMETER;

	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;
	
	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	fOn = Funcs.pfn_BT_GetStatus();
	if(power && fOn)
		return WIRELESS_ERROR_ALREADY_ON;

	if(!power && !fOn)
		return WIRELESS_ERROR_ALREADY_OFF;

	HANDLE hMutexBt = CreateMutex(NULL, FALSE, WIRELESS_API_MUTEX);
	if (!hMutexBt || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if(hMutexBt)
			CloseHandle(hMutexBt);
		return WIRELESS_ERROR_ACCESS_DENIED;
	}

	if(power)
	{
		if(!Funcs.pfn_BT_TurnOn())
			ret = WIRELESS_ERROR_DRIVER_ERROR;
	}
	else
	{
		if(!Funcs.pfn_BT_TurnOff())
			ret = WIRELESS_ERROR_DRIVER_ERROR;
	}
	if(save && (WIRELESS_ERROR_SUCCESS == ret))
	{
		if(!Funcs.pfn_BT_SaveStartupState(power))
			ret = WIRELESS_ERROR_REGISTRY_UPDATING_ERROR;
	}

	FreeLibrary(hLib);
	CloseHandle(hMutexBt);
	return ret;
}
extern "C" __declspec(dllexport) UINT32 MIC_WiFiGetHWStatus(UINT32* power)
{
	HINSTANCE	hLib;
	WL_FUNCS	Funcs	= {0};

	if (!power)
		return WIRELESS_ERROR_INVALID_PARAMETER;
	
	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;
	
	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	if(Funcs.pfn_WL_GetStatus())
		*power = 1;
	else
		*power = 0;

	FreeLibrary(hLib);
	return WIRELESS_ERROR_SUCCESS;
}
extern "C" __declspec(dllexport) UINT32 MIC_BluetoothGetHWStatus(UINT32* power)
{
	HINSTANCE	hLib;
	WL_FUNCS	Funcs	= {0};

	if (!power)
		return WIRELESS_ERROR_INVALID_PARAMETER;
	
	if (!IsWirelessModulePresent())
		return WIRELESS_ERROR_MODULE_NOT_DETECTED;
	
	if(!(hLib = LoadLibFuncs(&Funcs)))
		return WIRELESS_ERROR_UNKNOWN_ERROR;

	if(Funcs.pfn_BT_GetStatus())
		*power = 1;
	else
		*power = 0;

	FreeLibrary(hLib);
	return WIRELESS_ERROR_SUCCESS;
}