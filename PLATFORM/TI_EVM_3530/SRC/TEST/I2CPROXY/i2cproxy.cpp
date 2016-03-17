/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: proxydriver.cpp
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <i2c.h>
#include <i2cproxy.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

//#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)
#define ZONE_DVFS           DEBUGZONE(4)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"ProxyDriver", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined" ,  L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x000B
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define I2C_DEVICE_COOKIE       'i2cD'
#define I2C_INSTANCE_COOKIE     'i2cI'

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD               cookie;
    CRITICAL_SECTION    cs;
    OMAP_DEVICE         devId;
    HANDLE              hParent;
    DWORD               i2cBus[1];
} Device_t;

typedef struct {
    DWORD               cookie;
    Device_t           *pDevice;
    HANDLE              hI2C;
    DWORD               subAddress;
} Instance_t;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"i2cBus", PARAM_MULTIDWORD, TRUE, offset(Device_t, i2cBus),
        fieldsize(Device_t, i2cBus), NULL
    }
};


//------------------------------------------------------------------------------
//  Local Functions
BOOL I2C_Deinit(DWORD context);

//------------------------------------------------------------------------------
//
//  Function:  I2C_Init
//
//  Called by device manager to initialize device.
//
DWORD
I2C_Init(
    LPCTSTR szContext, 
    LPCVOID pBusContext
    )
{
    DWORD rc = (DWORD)NULL;
    Device_t *pDevice;
    // Create device structure
    pDevice = (Device_t *)LocalAlloc(LPTR, sizeof(Device_t));
    if (pDevice == NULL)
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed allocate Proxy driver structure\r\n"
            ));

        goto cleanUp;
        }

    memset(pDevice, 0, sizeof(Device_t));
    
    // Set cookie
    pDevice->cookie = I2C_DEVICE_COOKIE;

    // Initialize critical sections
    InitializeCriticalSection(&pDevice->cs);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed read I2C Proxy driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    // get handle to rootbus for notification to dvfs policy
    // Open clock device
    pDevice->hParent = CreateFile(L"BUS1:", 0, 0, NULL, 0, 0, NULL);
    if (pDevice->hParent == INVALID_HANDLE_VALUE)
    {
        RETAILMSG(ZONE_ERROR, (L"ERROR: I2C_Init: "
            L"Failed open parent bus device '%s'\r\n", L"BUS1:"
            ));
        pDevice->hParent = NULL;
        goto cleanUp;
    }

    pDevice->devId = (OMAP_DEVICE)I2CGetDeviceIdFromMembase(pDevice->i2cBus[0]);

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0) I2C_Deinit((DWORD)pDevice);

    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL
I2C_Deinit(
    DWORD context
    )
{
    BOOL rc = FALSE;
    Device_t *pDevice = (Device_t*)context;

    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != I2C_DEVICE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Deinit) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    //Close handle to bus driver
    if (pDevice->hParent != NULL)
        {
        CloseHandle(pDevice->hParent);
        }

    // Delete critical sections
    DeleteCriticalSection(&pDevice->cs);

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
I2C_Open(
    DWORD context, 
    DWORD accessCode, 
    DWORD shareMode
    )
{
    DWORD dw = 0;
    Instance_t *pInstance = NULL;
    Device_t *pDevice = (Device_t*)context;
    
    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Open(0x%08x, 0x%08x, 0x%08x)\r\n", 
        context, accessCode, shareMode
        ));
    
    if ((pDevice == NULL) || (pDevice->cookie != I2C_DEVICE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Open) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // Create device structure
    pInstance = (Instance_t*)LocalAlloc(LPTR, sizeof(Instance_t));
    if (pInstance == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: I2C_Open: "
            L"Failed allocate I2C instance structure\r\n"
            ));
        goto cleanUp;
        }

    // initialize instance
    memset(pInstance, 0, sizeof(Instance_t));
    pInstance->cookie = I2C_INSTANCE_COOKIE;
    pInstance->pDevice = pDevice;
    pInstance->hI2C = I2COpen(pDevice->devId);

    dw = (DWORD)pInstance;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Open(0x%08x, 0x%08x, 0x%08x) == %d\r\n", 
        context, accessCode, shareMode, dw
        ));

    return dw;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Close
//
//  This function closes the device context.
//
BOOL
I2C_Close(
    DWORD context
    )
{
    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Close(0x%08x)\r\n", 
        context
        ));

    BOOL rc = FALSE;
    Instance_t *pInstance = (Instance_t*)context;
    if ((pInstance == NULL) || (pInstance->cookie != I2C_INSTANCE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Close) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    I2CClose(pInstance->hI2C);
    LocalFree(pInstance);
    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Close(0x%08x)=%d\r\n", 
        context, rc
        ));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Read
//
//  This function closes the device context.
//
DWORD
I2C_Read(
    DWORD   context,
    LPVOID  pBuffer,
    DWORD   count
    )
{
    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Read(0x%08x, 0x%08X, %d)\r\n", 
        context, pBuffer, count
        ));

    DWORD rc = -1;
    Instance_t *pInstance = (Instance_t*)context;
    if ((pInstance == NULL) || (pInstance->cookie != I2C_INSTANCE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Close) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    rc = I2CRead(pInstance->hI2C, pInstance->subAddress, pBuffer, count);
    
cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Read(0x%08x)=%d\r\n", 
        context, rc
        ));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Write
//
//  This function closes the device context.
//
DWORD
I2C_Write(
    DWORD   context,
    LPCVOID pBuffer,
    DWORD   count
    )
{
    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Write(0x%08x, 0x%08X, %d)\r\n", 
        context, pBuffer, count
        ));

    DWORD rc = -1;
    Instance_t *pInstance = (Instance_t*)context;
    if ((pInstance == NULL) || (pInstance->cookie != I2C_INSTANCE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Close) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    rc = I2CWrite(pInstance->hI2C, pInstance->subAddress, pBuffer, count);

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Write(0x%08x)=%d\r\n", 
        context, rc
        ));

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_Seek
//
//  This function closes the device context.
//
BOOL
I2C_Seek(
    DWORD context,
    long  amount,
    WORD  type
    )
{
    RETAILMSG(ZONE_FUNCTION, (L"+I2C_Seek(0x%08x, %d, %d)\r\n", 
        context, amount, type
        ));

    Instance_t *pInstance = (Instance_t*)context;
    if ((pInstance == NULL) || (pInstance->cookie != I2C_INSTANCE_COOKIE))
        {
        RETAILMSG (ZONE_ERROR, (L"TLD: !ERROR(I2C_Close) - "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    pInstance->subAddress = (DWORD)amount;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-I2C_Seek(0x%08x)\r\n", 
        context
        ));

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  I2C_IOControl
//
//  This function sends a command to a device.
//
BOOL
I2C_IOControl(
    DWORD context, 
    DWORD code, 
    UCHAR *pInBuffer, 
    DWORD inSize, 
    UCHAR *pOutBuffer, 
    DWORD outSize, 
    DWORD *pOutSize
    )
{
    RETAILMSG(ZONE_FUNCTION, (
        L"+I2C_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    BOOL rc = FALSE;
    Instance_t *pInstance = (Instance_t*)context;

    // Check if we get correct context
    if ((pInstance == NULL) || (pInstance->cookie != I2C_INSTANCE_COOKIE))
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: I2C_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case IOCTL_I2C_SET_SLAVE_ADDRESS:
            I2CSetSlaveAddress(pInstance->hI2C, *(DWORD*)pInBuffer);
            rc = TRUE;
            break;

        case IOCTL_I2C_SET_SUBADDRESS_MODE:
            I2CSetSubAddressMode(pInstance->hI2C, *(DWORD*)pInBuffer);
            rc = TRUE;
            break;
        case IOCTL_I2C_SET_BAUD_INDEX:
			I2CSetBaudIndex(pInstance->hI2C, *(DWORD*)pInBuffer);
            rc = TRUE;
            break;        
	}

cleanUp:

    RETAILMSG(ZONE_FUNCTION, (L"-I2C_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  DLL entry point.
//
BOOL WINAPI DllMain(HANDLE hDLL, ULONG Reason, LPVOID Reserved)
{
    switch(Reason) 
        {
        case DLL_PROCESS_ATTACH:
            RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
        }
    return TRUE;
}


