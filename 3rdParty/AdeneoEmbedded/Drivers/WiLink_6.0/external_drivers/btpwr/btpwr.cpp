// Copyright (c) 2012 Micronet Ltd. All rights reserved.
//
// Created 29-Jan-2012 by Vladimir Zatulovsky
//
//------------------------------------------------------------------------------
//

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <pm.h>
#include <bt_api.h>
#include <service.h>
#if 0
#include <omap35xx.h>
#include <initguid.h>
#include <gpio.h>
#include <twl.h>
#include <wavext.h>
#include <head_set.h>
#endif
#include <oal.h>
#include <oalex.h>
#include <args.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(4)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"btpwr", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions
typedef int (*pfnBthGetHardwareStatus)(int *);

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD priority256;
	int addref;
	CRITICAL_SECTION cs;
	HANDLE BTDev;
	HMODULE hm_btdrt;
	pfnBthGetHardwareStatus bth_hws;
	int bth_hw_stat;
	CEDEVICE_POWER_STATE dx;
} bt_pwr_t;

//------------------------------------------------------------------------------
//  Local Functions

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, 0, offset(bt_pwr_t, priority256), fieldsize(bt_pwr_t, priority256), (VOID*)200
    }
};


//------------------------------------------------------------------------------
//
//  Function:  PWB_Close
//
//  This function closes the device context.
//
BOOL PWB_Close(DWORD context)
{
    bt_pwr_t *pDevice = (bt_pwr_t*)context;

	RETAILMSG(1, (TEXT("PWB_Close: %x, %x, %x\r\n"), pDevice->hm_btdrt, pDevice->bth_hws, pDevice->bth_hw_stat));
	EnterCriticalSection(&pDevice->cs);
	pDevice->addref--;
	if(!pDevice->addref && pDevice->hm_btdrt)
	{
		CloseHandle(pDevice->BTDev);
		pDevice->BTDev = INVALID_HANDLE_VALUE;
		pDevice->bth_hws = 0;
		FreeLibrary(pDevice->hm_btdrt);
		pDevice->hm_btdrt = 0;
	}
	LeaveCriticalSection(&pDevice->cs);
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  PWB_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL PWB_Deinit(DWORD context)
{
    bt_pwr_t *pDevice = (bt_pwr_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+PWB_Deinit(0x%08x)\r\n", context));

	while(pDevice->addref)
		PWB_Close(context);

	DeleteCriticalSection(&pDevice->cs);
    // Free device structure
    LocalFree(pDevice);

    // Done
    DEBUGMSG(ZONE_FUNCTION, (L"-PWB_Deinit\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  PWB_Init
//
//  Called by device manager to initialize device.
//
DWORD PWB_Init(LPCTSTR szContext, LPCVOID pBusContext)
{
    DWORD rc = (DWORD)NULL;
    bt_pwr_t *pDevice = NULL;

	card_ver	m_cv; // card vesion
	UINT32		in = OAL_ARGS_QUERY_WLAN_CFG;

    RETAILMSG(1, (L"+PWB_Init(%s, 0x%08x)\r\n", szContext, pBusContext));

	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &m_cv, sizeof(m_cv), 0))
	{
		m_cv.card_addr	= -1;
		m_cv.ver		= -1;
		m_cv.rev		= -1;
		m_cv.config		= -1;
	}

	if(m_cv.ver == -1 || m_cv.ver < 300)
	{
		RETAILMSG(1, (L"-PWB_Init: Wireless Module is not present\r\n"));
		return 0;
	}   

    // Create device structure
    pDevice = (bt_pwr_t *)LocalAlloc(LPTR, sizeof(bt_pwr_t));
    if(!pDevice)
	{
        RETAILMSG(1, (L"ERROR: PWB_Init: Failed allocate handle\r\n"));
		return 0;
	}

    // initialize memory
    //
    memset(pDevice, 0, sizeof(bt_pwr_t));
	pDevice->BTDev = INVALID_HANDLE_VALUE;

#if 0
	UINT32 in = OAL_ARGS_QUERY_MAIN_CFG;
	memset(&pDevice->mbver, -1, sizeof(pDevice->mbver));

	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &pDevice->mbver, sizeof(pDevice->mbver), 0))
	{
		pDevice->mbver.card_addr	= -1;
		pDevice->mbver.ver			= -1;
		pDevice->mbver.rev			= -1;
		pDevice->mbver.config		= -1;
	}
#endif

	InitializeCriticalSection(&pDevice->cs);
    // Read device parameters
    if(GetDeviceRegistryParams(szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams)!= ERROR_SUCCESS)
	{
		RETAILMSG(1, (TEXT("ERROR: PWB_Init:Failed read driver registry parameters\r\n")));
	}

	pDevice->dx = D0;

    DEBUGMSG(ZONE_FUNCTION, (L"-PWB_Init %X\r\n", pDevice));
	RETAILMSG(1, (TEXT("-PWB_Init %X\r\n"), pDevice));

    return (DWORD)pDevice;
}

//------------------------------------------------------------------------------
//
//  Function:  PWB_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD PWB_Open(DWORD context, DWORD accessCode, DWORD shareMode)
{
    bt_pwr_t *pDevice = (bt_pwr_t*)context;

	EnterCriticalSection(&pDevice->cs);
	pDevice->addref++;

	if(pDevice->BTDev == INVALID_HANDLE_VALUE)
		pDevice->BTDev = CreateFile (L"BTD0:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	if(!pDevice->hm_btdrt)
	{
		pDevice->hm_btdrt = LoadLibrary(L"btdrt.dll");
		if(pDevice->hm_btdrt)
			pDevice->bth_hws = (pfnBthGetHardwareStatus)GetProcAddress(pDevice->hm_btdrt, L"BthGetHardwareStatus");
		if(pDevice->bth_hws)
			if(ERROR_SUCCESS != pDevice->bth_hws(&pDevice->bth_hw_stat))
				pDevice->bth_hw_stat = HCI_HARDWARE_UNKNOWN;
	}
	LeaveCriticalSection(&pDevice->cs);
	RETAILMSG(1, (TEXT("PWB_Open: %x, %x, %x\r\n"), pDevice->hm_btdrt, pDevice->bth_hws, pDevice->bth_hw_stat));
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  PWB_IOControl
//
//  This function sends a command to a device.
//
BOOL PWB_IOControl(DWORD context, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    BOOL rc = 0;
	DWORD Err = ERROR_INVALID_PARAMETER;
    bt_pwr_t *pDevice = (bt_pwr_t*)context;


    DEBUGMSG(ZONE_FUNCTION, (L"+PWB_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
		context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize));

    switch(code)
	{
		case IOCTL_POWER_CAPABILITIES:
		{
			if(pOutBuffer && outSize >= sizeof (POWER_CAPABILITIES) && pOutSize) 
			{
				__try 
				{
					PPOWER_CAPABILITIES PowerCaps;
					
					PowerCaps = (PPOWER_CAPABILITIES)pOutBuffer;
         
					// Only supports D0 (permanently on) D3 and D4(off).         
					memset(PowerCaps, 0, sizeof(POWER_CAPABILITIES));
					PowerCaps->DeviceDx = DX_MASK(D0)|DX_MASK(D3)|DX_MASK(D4);
					*pOutSize = sizeof(POWER_CAPABILITIES);

					Err = ERROR_SUCCESS;

					rc = 1;
				}
				__except(EXCEPTION_EXECUTE_HANDLER) 
				{
					RETAILMSG(ZONE_ERROR, (L"PWB: IOCTL_POWER_CAPABILITIES: exception in ioctl\r\n"));
				}
			}
			break;
		}
		case IOCTL_POWER_GET:
		{
            if(pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
            {
                __try 
                {
                    *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->dx;  

                    Err = ERROR_SUCCESS;
					rc = 1;
                }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                {
					DEBUGMSG(ZONE_ERROR, (TEXT("PWB: IOCTL_POWER_GET: Exception in ioctl\r\n")));
                }
            }

			break;
		}
		case IOCTL_POWER_SET:
		{
            if(pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
            {
                __try 
                {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
					WCHAR *argPtr = L"card";
					DWORD dwCtl = -1;
					int bt_hws = HCI_HARDWARE_UNKNOWN, i;


					RETAILMSG(1, (TEXT("PWB: IOCTL_POWER_SET request D%u -> D%u \r\n"), pDevice->dx, ReqDx));
                    if(!VALID_DX(ReqDx))
                    {
						RETAILMSG(1, (TEXT("PWB: Invalid state request D%u\r\n"), ReqDx));
						break;
                    }

					*(PCEDEVICE_POWER_STATE)pOutBuffer = ReqDx;
					*pOutSize = sizeof(CEDEVICE_POWER_STATE);
					Err = ERROR_SUCCESS;
					rc = 1;

					if(ReqDx == pDevice->dx) 
                    {
						RETAILMSG(1, (TEXT("PWB: already set D%u\r\n"), ReqDx));
						break;
					}

					EnterCriticalSection(&pDevice->cs);
					if(pDevice->bth_hws)
					{
						i = 8;
						if((pDevice->dx == D4 || pDevice->dx == D3) && ReqDx == D0)
						{
							do
							{
								RETAILMSG(1, (L"Wait for BTHW down %d\r\n", pDevice->bth_hw_stat));
								Sleep(50);
								if(ERROR_SUCCESS != pDevice->bth_hws(&bt_hws))
								{
									RETAILMSG(1, (L"Fail to get HWS %d\r\n", GetLastError()));
									bt_hws = HCI_HARDWARE_UNKNOWN;
									break;
								}
								if(bt_hws != HCI_HARDWARE_RUNNING)
									break;
							}while(i--);

							dwCtl = IOCTL_SERVICE_START;
						}
						else if((ReqDx == D4 || ReqDx == D3) && pDevice->dx == D0)
						{
							do
							{
								RETAILMSG(1, (L"Wait for BTHW up %d\r\n", pDevice->bth_hw_stat));
								Sleep(50);
								if(ERROR_SUCCESS != pDevice->bth_hws(&bt_hws))
								{
									RETAILMSG(1, (L"Fail to get HWS %d\r\n", GetLastError()));
									bt_hws = HCI_HARDWARE_UNKNOWN;
									break;
								}
								if(bt_hws == HCI_HARDWARE_INITIALIZING)
								{
									i++;
									continue;
								}
								else
									break;
							}while(i--);
							dwCtl = IOCTL_SERVICE_STOP;
						}
						if(pDevice->BTDev != INVALID_HANDLE_VALUE)
						{
							RETAILMSG(1, (L"PWB: Requst the BT stack (%d) for %x\r\n", bt_hws, dwCtl));
							switch(dwCtl)
							{
								case IOCTL_SERVICE_START:
								{
									if(bt_hws == HCI_HARDWARE_SHUTDOWN)
										DeviceIoControl(pDevice->BTDev, dwCtl, argPtr, sizeof(WCHAR) * (wcslen(argPtr) + 1), 0, 0, 0, 0);
									break;
								}
								case IOCTL_SERVICE_STOP:
								{
									if(bt_hws == HCI_HARDWARE_RUNNING)
										DeviceIoControl(pDevice->BTDev, dwCtl, argPtr, sizeof(WCHAR) * (wcslen(argPtr) + 1), 0, 0, 0, 0);
									break;
								}
								default:
									break;
							}
						}

						i = 8;
						if(bt_hws == HCI_HARDWARE_SHUTDOWN)
						{
							do
							{
								RETAILMSG(1, (L"Wait for BTHW up %d\r\n", bt_hws));
								Sleep(50);
								if(ERROR_SUCCESS != pDevice->bth_hws(&bt_hws))
								{
									bt_hws = HCI_HARDWARE_UNKNOWN;
									break;
								}
								if(bt_hws == HCI_HARDWARE_INITIALIZING)
								{
									i++;
									continue;
								}
								else
									break;
							}while(i--);
						}
						else if(bt_hws == HCI_HARDWARE_RUNNING)
						{
							do
							{
								RETAILMSG(1, (L"Wait for BTHW down %d\r\n", bt_hws));
								Sleep(50);
								if(ERROR_SUCCESS != pDevice->bth_hws(&bt_hws))
								{
									bt_hws = HCI_HARDWARE_UNKNOWN;
									break;
								}
								if(bt_hws != HCI_HARDWARE_RUNNING)
									break;
							}while(i--);
						}
					}
					LeaveCriticalSection(&pDevice->cs);
					pDevice->dx = ReqDx;
					pDevice->bth_hw_stat = bt_hws;

					RETAILMSG(1, (TEXT("PWB: IOCTL_POWER_SET to D%u \r\n"), ReqDx));
                }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                {
					RETAILMSG(1, (TEXT("PWB: IOCTL_POWER_SET: Exception in ioctl\r\n")));
                }
            }
            break;
		}
		default:
		{
			Err = ERROR_INVALID_PARAMETER;
			break;
		}
	}

	if(Err)
		SetLastError(Err);

    DEBUGMSG(ZONE_FUNCTION, (L"-PWB_IOControl(rc = %d)\r\n", rc));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL __stdcall DllMain(HANDLE hDLL, DWORD reason, VOID *pReserved)
{
    switch(reason)
	{
        case DLL_PROCESS_ATTACH:
			RETAILMSG(1, (L"BWB DllMain: DLL_PROCESS_ATTACH\r\n"));
            RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
	}

    return TRUE;
}
