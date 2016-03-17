/*
==============================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
==============================================================
*/
#include <windows.h>
#include "sdmapx.h"
#include "sdmaproxy.h"

//
// Debug Zones.
//
#ifndef SHIP_BUILD

    #define DMP_ZONE_INIT       0x0001
    #define DMP_ZONE_OPEN       0x0002
    #define DMP_ZONE_READ       0x0004
    #define DMP_ZONE_WRITE      0x0008
    #define DMP_ZONE_CLOSE      0x0010
    #define DMP_ZONE_IOCTL      0x0020
    #define DMP_ZONE_FUNCTION   0x0040
    #define DMP_ZONE_WARNING    0x0080
    #define DMP_ZONE_ERROR      0x0100
    
DBGPARAM dpCurSettings = {
                TEXT("SDMA Proxy"),
                {
                    TEXT("Init"),
                    TEXT("Open"),
                    TEXT("Read"),
                    TEXT("Write"),
                    TEXT("Close"),
                    TEXT("IO Control"),
                    TEXT("Function"),
                    TEXT("Warning"),
                    TEXT("Error"),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                },
                0x0
};

#else
    
    #define DMP_ZONE_INIT 0
    #define DMP_ZONE_OPEN 0
    #define DMP_ZONE_READ 0
    #define DMP_ZONE_WRITE 0
    #define DMP_ZONE_CLOSE 0
    #define DMP_ZONE_IOCTL 0
    #define DMP_ZONE_FUNCTION 0
    #define DMP_ZONE_WARNING 0
    #define DMP_ZONE_ERROR 0
#endif
    
//------------------------------------------------------------------------------
//  Local Definitions
//
#define DMP_DEVICE_COOKIE       'spxy'

//
// Marshal Helpers
//
__inline
void*
MarshalDataPointerHelper(PVOID pSrcUnmarshalled,
                         DWORD cbSrc,
                         DWORD desc
                         )
{
    HRESULT hr;
    void* pDestMarshalled;

    hr = CeOpenCallerBuffer(&pDestMarshalled, pSrcUnmarshalled, cbSrc, desc, FALSE);
    if (FAILED(hr))
        return NULL;

    return pDestMarshalled;
}
#define MARSHAL_DATA_POINTER(x, y, h)           MarshalDataPointerHelper(x, y, h)
#define UNMARSHAL_DATA_POINTER(a, b, c, h)      CeCloseCallerBuffer(a, b, c, h)

//
// SDMA Client Info Struct
// Internal use
//
typedef struct SdmaPxClient_t {
    DWORD cookie;
    SdmaPx *sdmaPx;
} SdmaPxClient_t;

CRITICAL_SECTION csLock;

//
// DllEntry
//
BOOL WINAPI DllEntry(
                        HINSTANCE hinstDll,
                        DWORD dwReason,
                        LPVOID Reserved)
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        DEBUGMSG (DMP_ZONE_INIT, (TEXT("SdmaProxy process attach\n")));
#ifndef SHIP_BUILD
        RegisterDbgZones(hinstDll, &dpCurSettings);
#endif
	    DisableThreadLibraryCalls((HMODULE) hinstDll);      
    }

    if ( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (DMP_ZONE_INIT, (TEXT("SdmaProxy process detach called\n")));
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Init
////////////////////////////////////////////////////////////////////////////////
DWORD DMP_Init( LPCTSTR szContext, LPCVOID pBusContext)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Init\n")));
    //
    DWORD rc = (DWORD)NULL;
    SdmaPxClient_t *pDevice = NULL;
    
    // Create device structure
    pDevice = (SdmaPxClient_t *)LocalAlloc(LPTR, sizeof(SdmaPxClient_t));
    if (pDevice == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: DMP_Init: "
            L"Failed allocate Sdma Proxy driver structure\r\n"
            ));
    }
    else
    {
        // Set cookie
        pDevice->cookie = DMP_DEVICE_COOKIE;
        pDevice->sdmaPx = new SdmaPx();
    
        if(pDevice->sdmaPx)
        {
            if(pDevice->sdmaPx->Initialize() == FALSE) return 0; // Failed to initialize Sdma Proxy.
        
            rc = (DWORD)pDevice;
        }
        
        // Init mutex
        InitializeCriticalSection(&csLock);       
    }
    //    
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Init\n")));
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Deinit
////////////////////////////////////////////////////////////////////////////////
BOOL DMP_Deinit( DWORD hDeviceContext )
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Deinit\n")));
    //
    BOOL rc = FALSE;
    SdmaPxClient_t *pDevice = (SdmaPxClient_t*)hDeviceContext;
    SdmaPx *psdmaPx = NULL;
    
    if ((pDevice == NULL) || (pDevice->cookie != DMP_DEVICE_COOKIE))
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Deinit: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }
    
    if(psdmaPx = (SdmaPx *)pDevice->sdmaPx)
    {
        if(psdmaPx->CleanUp() == FALSE) 
        {
            DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Deinit: "
                L"Failed to clean up the Sdma Proxy\r\n"
                ));
            goto cleanUp; // Failed to Clean Up Sdma Proxy.  
        }
        //
        delete (psdmaPx);
    }
    
    // Free device structure
    LocalFree(pDevice);
    
    // Free mutex
    DeleteCriticalSection(&csLock);
    
    // Done
    rc = TRUE;
    //
cleanUp: 
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Deinit\n")));
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Open
////////////////////////////////////////////////////////////////////////////////
DWORD DMP_Open(
                DWORD hDeviceContext,
                DWORD AccessCode,
                DWORD ShareMode)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Open (device context=0x%x)\n"), hDeviceContext));
    //
    SdmaPxClient_t *pDevice = (SdmaPxClient_t *)hDeviceContext;
    SdmaPx *pSdmaPx = NULL;
    DWORD rc = (DWORD)NULL;
    
    if ((pDevice == NULL) || (pDevice->cookie != DMP_DEVICE_COOKIE))
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Open: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }
    //
    if((pSdmaPx = pDevice->sdmaPx) != NULL)
    {
        if(pSdmaPx->AddSdmaPxClient(GetCallerVMProcessId()) == FALSE)
        {
            DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Open: "
                L"Failed to Add Sdma Proxy Client\r\n"
                ));
            goto cleanUp; // Failed to add Sdma Proxy Client
        }
#ifdef DEBUG
        DWORD callerId = GetCallerVMProcessId();
        DEBUGMSG (DMP_ZONE_FUNCTION, (L"DMP_Open: "
                L"Add Sdma Proxy Client ID = 0x%x, Index = %d\r\n",
                callerId,
                pSdmaPx->GetSdmaPxClientIdx(callerId)
                ));
#endif        
    }
    
    // Done
    rc = hDeviceContext;
    //
cleanUp:    
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Open (device context=0x%x)\n"), hDeviceContext));
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Close
////////////////////////////////////////////////////////////////////////////////
BOOL DMP_Close(DWORD hOpenContext) 
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Close (device context=0x%x)\n"), hOpenContext));
    //
    SdmaPxClient_t *pDevice = (SdmaPxClient_t *)hOpenContext;
    SdmaPx *pSdmaPx = NULL;
    BOOL rc = FALSE;
    
    if ((pDevice == NULL) || (pDevice->cookie != DMP_DEVICE_COOKIE))
    {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Close: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
    }
    //
    if((pSdmaPx = pDevice->sdmaPx) != NULL)
    {
#ifdef DEBUG
        DWORD callerId = GetCallerVMProcessId();
        DEBUGMSG (DMP_ZONE_FUNCTION, (L"DMP_Close: "
                L"Close Sdma Proxy Client ID = 0x%x, Index = %d\r\n",
                callerId,
                pSdmaPx->GetSdmaPxClientIdx(callerId)
                ));
#endif      
        if (pSdmaPx->RemoveSdmaPxClient(GetCallerVMProcessId()) == FALSE)      
        {
            DEBUGMSG (ZONE_ERROR, (L"ERROR: DMP_Close: "
                L"Failed to Remove Sdma Proxy Client\r\n"
                ));
            goto cleanUp; // Failed to add Sdma Proxy Client
        }        
    }
    // Done
    rc = TRUE;
    //
cleanUp:    
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Close (device context=0x%x)\n"), hOpenContext));
    return rc;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Read
////////////////////////////////////////////////////////////////////////////////
DWORD DMP_Read(
                DWORD hOpenContext,
                LPVOID pBuffer,
                DWORD Count)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Read\n")));
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Read\n")));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Write
////////////////////////////////////////////////////////////////////////////////
DWORD DMP_Write(
                DWORD hOpenContext,
                LPCVOID pSourceBytes,
                DWORD NumberOfBytes)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Write\n")));
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Write\n")));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_Seek
////////////////////////////////////////////////////////////////////////////////
DWORD DMP_Seek(
                DWORD hOpenContext,
                long Amount,
                DWORD Type)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_Seek\n")));
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_Seek\n")));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_IOControl
////////////////////////////////////////////////////////////////////////////////
BOOL DMP_IOControl(
                        DWORD hOpenContext,
                        DWORD dwCode,
                        PBYTE pBufIn,
                        DWORD dwLenIn,
                        PBYTE pBufOut,
                        DWORD dwLenOut,
                        PDWORD pdwActualOut)
{
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("+DMP_IOControl - %s - (device context=0x%x / thread id=0x%x / pBufIn=0x%x, pBufOut=0x%x)\n"), (dwCode == 0) ? L"CONFIGURE" : L"COPY", hOpenContext, GetCurrentThreadId(), pBufIn, pBufOut));

	// Get mutex
    EnterCriticalSection(&csLock);

    BOOL RetVal = TRUE;
    INT32 clientIdx;
    SdmaPxClient_t *pDevice = NULL;
    SdmaPx *pSdmaPx = NULL;
     
    if((pDevice = (SdmaPxClient_t *)hOpenContext) == NULL)
    {
        RetVal = FALSE;
        DEBUGMSG(DMP_ZONE_ERROR, (TEXT("IoControl Code = 0x%x, Invalid context!\n"), dwCode));
        goto cleanUp;
    }
    
    if((pSdmaPx = pDevice->sdmaPx) == NULL)
    {
        RetVal = FALSE;
        DEBUGMSG(DMP_ZONE_ERROR, (TEXT("IoControl Code = 0x%x, Invalid Proxy!\n"), dwCode));
        goto cleanUp;
    }

#ifdef DEBUG
        DWORD callerId = GetCallerVMProcessId();
        DEBUGMSG (DMP_ZONE_FUNCTION, (L"DMP_IOControl: "
                L"IoControl Sdma Proxy Client ID = 0x%x, Index = %d\r\n",
                callerId,
                pSdmaPx->GetSdmaPxClientIdx(callerId)
                ));
#endif 
    
    if((clientIdx = pSdmaPx->GetSdmaPxClientIdx(GetCallerVMProcessId())) == -1)
    {
        RetVal = FALSE;
        DEBUGMSG(DMP_ZONE_ERROR, (TEXT("IoControl Code = 0x%x, could not retrieve client!\n"), dwCode));
        goto cleanUp;
    }
    
    switch(dwCode)
    {    
    case IOCTL_SDMAPROXY_COPY: // keep this case first for optimization
        {
            if( (NULL == pBufOut) || (NULL == pBufIn) )
            {
                DEBUGMSG(DMP_ZONE_IOCTL, (TEXT("IoControl Code = 0x%x, Invalid Parameter!\n"), dwCode));
                RetVal = FALSE;
                goto cleanUp;
            }
        
            // Check if length is < 2^24 as the count elements can only be a 24bits value. See TRM Table9-43 for OMAP3.
            // Increase the allowed size by 4 times as the DMA copy uses 32bits elements instead of the default byte value.
            if(dwLenIn < (0x01000000 * 4))
            {
                // Do the system DMA copy
                dwLenOut = pSdmaPx->Copy(pBufIn, pBufOut, (DWORD)clientIdx);
            }
            else
            {
                // Failure
                DEBUGMSG(DMP_ZONE_IOCTL, (TEXT("IoControl, Invalid Length!\n")));
                dwLenOut = 0;
                RetVal = FALSE;
            }

            *pdwActualOut = dwLenOut; // return the size
            break;
        }
    
    case IOCTL_SDMAPROXY_CONFIG:
        {
            SdmaConfig_t *pdmaDataConfig;
            
            if( (dwLenIn < sizeof(SdmaConfig_t)) || (NULL == pBufIn) )
            {
                DEBUGMSG(DMP_ZONE_IOCTL, (TEXT("IoControl Code = 0x%x, Invalid Parameter!\n"), dwCode));
                RetVal = FALSE;
                goto cleanUp;
            }
            
            pdmaDataConfig = (SdmaConfig_t *)pBufIn;
            RetVal = pSdmaPx->Configure(pdmaDataConfig, (DWORD)clientIdx);
            break;
        }
    
    default:
        RetVal = FALSE;
        break;
    }

cleanUp:
    LeaveCriticalSection(&csLock); // release mutex
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_IOControl - %s - (device context=0x%x / thread id=0x%x)\n"), (dwCode == 0) ? L"CONFIGURE" : L"COPY", hOpenContext, GetCurrentThreadId()));
    return RetVal;
}

////////////////////////////////////////////////////////////////////////////////
// DMP_PowerUp
////////////////////////////////////////////////////////////////////////////////
VOID DMP_PowerUp(DWORD hDeviceContext )
{
    RETAILMSG(1, (TEXT("+DMP_PowerUp\r\n")));
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_PowerUp\n")));
}

////////////////////////////////////////////////////////////////////////////////
// DMP_PowerDown
////////////////////////////////////////////////////////////////////////////////
VOID DMP_PowerDown(DWORD hDeviceContext)
{
    RETAILMSG(1, (TEXT("+DMP_PowerDown\r\n")));
    DEBUGMSG (DMP_ZONE_FUNCTION, (TEXT("-DMP_PowerDown\n")));
}
