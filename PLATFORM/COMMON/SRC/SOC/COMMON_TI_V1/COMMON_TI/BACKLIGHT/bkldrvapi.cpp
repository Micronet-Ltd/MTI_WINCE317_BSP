// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// Portions Copyright (c) Texas Instruments.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File: bkldrvapi.c
//
//  Backlight driver source code
//
#include <windows.h>
#include <pm.h>
#include <devload.h>
#include "backlight_class.h"
#include "bkli.h"
#include "ceddkex.h"


// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

CBacklightRoot* g_pBacklight;

static VOID FreeContext(BKL_MDD_INFO *pBKLinfo)
{
    if(pBKLinfo)
    {
        if(pBKLinfo->hBklThread)
        {
            // Thread has begun - wait for it to finish
            pBKLinfo->fExit = TRUE;
            SetEvent(pBKLinfo->hExitEvent);
            WaitForSingleObject(pBKLinfo->hBklThread, INFINITE);
            CloseHandle(pBKLinfo->hBklThread);
        }
        
        if(pBKLinfo->hExitEvent)
        {
            CloseHandle(pBKLinfo->hExitEvent);
        }
        
         g_pBacklight->Uninitialize();

        if (pBKLinfo->hCoreDll) 
        {
            FreeLibrary(pBKLinfo->hCoreDll);
        }
            

        LocalFree(pBKLinfo);
    }
}


/* 
    pContext [in]: Pointer to a string containing the registry path to the active key 
    for the stream interface driver

    lpvBusContext [in]: Potentially process-mapped pointer passed as the fourth parameter to ActivateDeviceEx. 
    If this driver was loaded through legacy mechanisms, then lpvBusContext is zero.    
*/
extern "C" DWORD BKL_Init(
  LPCTSTR pContext, 
  LPCVOID lpvBusContext
)
{   
    DWORD dwStatus, dwSize, dwType;
    HKEY hkDevice = NULL;
    BKL_MDD_INFO *pBKLinfo = NULL;

    UNREFERENCED_PARAMETER(lpvBusContext);
   
    if (IsDVIMode())
        return 0;
    
    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("+BKL_Init() context %s.\r\n"), pContext));


    g_pBacklight = GetBacklightObject();
    if (g_pBacklight == NULL)
    {
        goto error;
    }

    // Allocate enough storage for this instance of our backlight
    pBKLinfo = (BKL_MDD_INFO *)LocalAlloc(LPTR, sizeof(BKL_MDD_INFO));
    if (pBKLinfo == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Init: "
            L"Failed allocate BKL_MDD_INFO device structure\r\n" ));
        goto error;
    }

    // get device name from registry
    dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pContext, 0, 0, &hkDevice);
    if(dwStatus != ERROR_SUCCESS) 
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BLK_Init: OpenDeviceKey failed with %u\r\n"), dwStatus));
        goto error;
    }
    dwSize = sizeof(pBKLinfo->szName);
    dwStatus = RegQueryValueEx(hkDevice, DEVLOAD_DEVNAME_VALNAME, NULL, &dwType, (LPBYTE)pBKLinfo->szName, &dwSize);
    if(dwStatus != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BLK_Init: RegQueryValueEx failed with %u\r\n"), dwStatus));
        goto error;

    }
    RegCloseKey(hkDevice);
    hkDevice = NULL;

    // create exit event to be set by deinit:
    pBKLinfo->hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(NULL == pBKLinfo->hExitEvent)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BLK_Init: OpenDeviceKey failed with %u\r\n"), dwStatus));
        goto error;
    }

    pBKLinfo->hCoreDll = LoadLibrary(TEXT("coredll.dll"));
    if (NULL != pBKLinfo->hCoreDll) 
    {
        pBKLinfo->pfnGetSystemPowerStatusEx2 = (PFN_GetSystemPowerStatusEx2)
            GetProcAddress(pBKLinfo->hCoreDll, TEXT("GetSystemPowerStatusEx2"));
        if (!pBKLinfo->pfnGetSystemPowerStatusEx2) 
        {
            DEBUGMSG(ZONE_WARN,  (TEXT("GetProcAddress failed for GetSystemPowerStatusEx2. Assuming always on AC power.\r\n")));
        }
    }

//MYS     pBKLinfo->dwPddContext = BacklightInit(pContext, lpvBusContext, &(pBKLinfo->dwCurState));
    if (g_pBacklight->Initialize(pContext) == FALSE)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Init: "
            L"Failed to initialize backlight object\r\n"
            ));
        goto error;
        }
    
    // if there are no user settings for this, we act as if they are selected:
    pBKLinfo->fBatteryTapOn = TRUE;
    pBKLinfo->fExternalTapOn = TRUE;

    // in case there is no setting for this:
    pBKLinfo->dwBattTimeout = 0;
    pBKLinfo->dwACTimeout = 0;

    pBKLinfo->dwCurState = D0;
    g_pBacklight->SetPowerState(pBKLinfo->dwCurState);
 
    //create thread to wait for reg / power source changes:
    pBKLinfo->hBklThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)fnBackLightThread, pBKLinfo, 0, NULL);
    if(NULL == pBKLinfo->hBklThread)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("BLK_Init: OpenDeviceKey failed with %u\r\n"), dwStatus));
        goto error;
    }

    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("-BKL_Init().\r\n")));

    return (DWORD) pBKLinfo;

error:
    if(hkDevice)
    {
        RegCloseKey(hkDevice);
    }

    FreeContext(pBKLinfo);    

    return 0;

}

//------------------------------------------------------------------------------

extern "C" BOOL BKL_Deinit(DWORD dwContext)
{
    BKL_MDD_INFO *pBKLinfo = NULL;
    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("BKL_Deinit().\r\n")));

    // Verify our context
    if(! dwContext)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: BKL_Deinit: "
            L"Incorrect context paramer\r\n" ));

        return FALSE;
    }

    pBKLinfo = (BKL_MDD_INFO *) dwContext;

    FreeContext(pBKLinfo);

    if (g_pBacklight)
    {
        delete g_pBacklight;
    }
    
    return TRUE;
}

//------------------------------------------------------------------------------

extern "C" DWORD BKL_Open(DWORD dwClientContext, DWORD dwAccess, DWORD dwShareMode)
{
    UNREFERENCED_PARAMETER(dwAccess);
    UNREFERENCED_PARAMETER(dwShareMode);
    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("BKL_Open().\r\n")));
    return dwClientContext;
}

//------------------------------------------------------------------------------

extern "C" BOOL BKL_Close(DWORD dwOpenContext)
{
    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("BKL_Close().\r\n")));
    return (dwOpenContext != 0);
}

//------------------------------------------------------------------------------

extern "C" BOOL BKL_IOControl(DWORD dwOpenContext, DWORD dwIoControlCode, LPBYTE lpInBuf, 
                   DWORD nInBufSize, LPBYTE lpOutBuf, DWORD nOutBufSize, 
                   LPDWORD lpBytesReturned)
{
    DWORD dwErr = ERROR_INVALID_PARAMETER;
    BKL_MDD_INFO *pBKLinfo = NULL;

    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpInBuf);

    DEBUGMSG(ZONE_BACKLIGHT,(TEXT("BKL_IOControl IOCTL code = %d\r\n"), dwIoControlCode));

    // Verify context
    if( !dwOpenContext )
    {
        DEBUGMSG(ZONE_ERROR, (
            L"ERROR: BKL_Deinit: "
            L"Incorrect context paramer\r\n" 
        ));

        return FALSE;
    }

    pBKLinfo = (BKL_MDD_INFO *) dwOpenContext;

    switch (dwIoControlCode) 
    {
        case IOCTL_POWER_CAPABILITIES:  
            DEBUGMSG(ZONE_BACKLIGHT, (TEXT("BKL: IOCTL_POWER_CAPABILITIES\r\n")));
            if (lpOutBuf && nOutBufSize >= sizeof (POWER_CAPABILITIES) && lpBytesReturned) 
            {
                __try 
                {
                    PPOWER_CAPABILITIES PowerCaps = (PPOWER_CAPABILITIES)lpOutBuf;

                    memset(PowerCaps, 0, sizeof(*PowerCaps));
                    PowerCaps->DeviceDx = g_pBacklight->BacklightGetSupportedStates();

                     *lpBytesReturned = sizeof(*PowerCaps);

                    pBKLinfo->ucSupportedStatesMask = PowerCaps->DeviceDx;

                    ASSERT(pBKLinfo->ucSupportedStatesMask < 0x20);
                    
                    dwErr = ERROR_SUCCESS;
                }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                {
                    DEBUGMSG(ZONE_BACKLIGHT, (TEXT("exception in ioctl\r\n")));
                }
            }

            break;

        case IOCTL_POWER_QUERY: // determines whether changing power state is feasible
                DEBUGMSG(ZONE_BACKLIGHT,(TEXT("BKL: Received IOCTL_POWER_QUERY\r\n")));
                if (lpOutBuf && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                    // Return a good status on any valid query, since we are always ready to
                    // change power states (if asked for state we don't support, we move to next highest, eg D3->D4).
                    __try 
                    {
                        CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)lpOutBuf;

                        if (VALID_DX(ReqDx)) 
                        {
                            // This is a valid Dx state so return a good status.
                            dwErr = ERROR_SUCCESS;
                        }

                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("BKL: IOCTL_POWER_QUERY %s\r\n"), dwErr == ERROR_SUCCESS ? (TEXT("succeeded")) : (TEXT("failed")) ));
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("Exception in ioctl\r\n")));
                    }
                }
                break;

            break;

        case IOCTL_POWER_SET: // requests a change from one device power state to another
                DEBUGMSG(ZONE_BACKLIGHT,(TEXT("BKL: Received IOCTL_POWER_SET\r\n")));
                if (lpOutBuf && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                    __try 
                    {
                        CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)lpOutBuf;

                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("IOCTL_POWER_SET request to D%u \r\n"), ReqDx));
                        if (VALID_DX(ReqDx)) 
                        {
                            CEDEVICE_POWER_STATE SupportedDx = ReqDx;
                            
                            // figure out which state to ask driver to go to:
                            if(GetBestSupportedState(pBKLinfo, ReqDx, &SupportedDx))
                            {       
                                 pBKLinfo->dwCurState = SupportedDx;
                                 g_pBacklight->SetPowerState(pBKLinfo->dwCurState);

                                // keep display driver on while backlight is on:
                                if(D0 == SupportedDx)
                                {   
                                    PTCHAR pszDevice;       // to hold device name (GUID + interface name)                                    
                                    UINT uMaxDeviceBytes;   // memory allocated to hold device name

                                    // allow space for null terminator + separating '\':
                                    uMaxDeviceBytes = (sizeof(PMCLASS_DISPLAY) + sizeof(pBKLinfo->szDisplayInterface) + 2*sizeof(TCHAR));
                                    pszDevice = (PTCHAR)LocalAlloc(0, uMaxDeviceBytes);
                                    if(pszDevice != NULL)
                                    {
                                        // copy null-terminated GUID string
                                        ASSERT(SUCCEEDED(StringCbCopy(pszDevice, uMaxDeviceBytes, PMCLASS_DISPLAY)));
                                        
                                        // concatenate '\' separator + display interface name:
                                        if (FAILED(StringCbCat(pszDevice, uMaxDeviceBytes, TEXT("\\")))) 
                                        {
                                            dwErr = ERROR_INSUFFICIENT_BUFFER;
                                        }                                        
                                        else if (FAILED(StringCbCat(pszDevice, uMaxDeviceBytes, pBKLinfo->szDisplayInterface))) 
                                        {
                                            dwErr = ERROR_INSUFFICIENT_BUFFER;
                                        }      
                                        else
                                        {
                                            pBKLinfo->hDDIPowerReq = SetPowerRequirement(pszDevice, D0, POWER_NAME, NULL, 0);
                                        }
                                        LocalFree(pszDevice);
                                    }
                                }
                                else
                                {
                                    ReleasePowerRequirement(pBKLinfo->hDDIPowerReq);
                                }
//MYS                                pBKLinfo->dwCurState=SupportedDx;
                                
                                // tell pm which state we went to:
                                *(PCEDEVICE_POWER_STATE) lpOutBuf = SupportedDx;
                                *lpBytesReturned = sizeof(CEDEVICE_POWER_STATE);

                                dwErr = ERROR_SUCCESS;
                                DEBUGMSG(ZONE_BACKLIGHT, (TEXT("IOCTL_POWER_SET to D%u \r\n"), ReqDx));
                                
                            }
                            else
                            {
                                DEBUGMSG(ZONE_BACKLIGHT, (TEXT("Unsupported state request D%u\r\n"), ReqDx));
                            }
                        }
                        else 
                        {
                            DEBUGMSG(ZONE_BACKLIGHT, (TEXT("Invalid state request D%u\r\n"), ReqDx));
                        }
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("Exception in ioctl\r\n")));
                    }
                }
                break;

        case IOCTL_POWER_GET: // gets the current device power state
               DEBUGMSG(ZONE_BACKLIGHT,(TEXT("BKL: Received IOCTL_POWER_GET\r\n")));
                if (lpOutBuf != NULL && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                    __try 
                    {
                        CEDEVICE_POWER_STATE   CurrentDx;   

                        CurrentDx = pBKLinfo->dwCurState; 
                        *(PCEDEVICE_POWER_STATE)lpOutBuf = CurrentDx;  

                        dwErr = ERROR_SUCCESS;

                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("IOCTL_POWER_GET: passing back %u\r\n"), CurrentDx));  
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("Exception in ioctl\r\n")));
                    }
                }

            break;
        
        default:     
            break;
    }

    SetLastError(dwErr);
    if(dwErr != ERROR_SUCCESS) 
    {
        return FALSE;
    } 
    else 
    {
       return TRUE;
    }

}


//------------------------------------------------------------------------------

BOOL WINAPI DllMain(HANDLE hinstDll, ULONG Reason, LPVOID Reserved)
{
    UNREFERENCED_PARAMETER(hinstDll);
    UNREFERENCED_PARAMETER(Reserved);
    
    switch(Reason) {
    case DLL_PROCESS_ATTACH:
        DEBUGREGISTER((HMODULE)hinstDll);
        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("DllMain(): dll attach.\r\n")));
        break;
        
    case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_BACKLIGHT, (TEXT("DllMain(): dll detach.\r\n")));
        break;
    }
    return TRUE;
}

