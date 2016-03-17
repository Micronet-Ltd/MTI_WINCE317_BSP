// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.
// Copyright (c) 2008 Micronet Ltd. All rights reserved.
//
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
//------------------------------------------------------------------------------
//
//  File: gpio.cpp
//
// GPIO pins define:      0 - 191 MPU Bank1 - Bank6 GPIO pins
//                      192 - 209 TPS659XX GPIO 0-17
//                        210 - 465  PCA9575 GPIO 0-255 (2 banks*8 pins * 16 devices)
//                        16 devices - its the max number of connected PCA9575 devices
//                        PCA9575 pin id = PCA9575 offset(210) + PCA9575 slave address*16 + pin id (gpio offset on the specific device)


#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <bsp.h>

#include <initguid.h>
#include <gpio.h>

#define GPIO_GROUPS     (2)

#ifndef SHIP_BUILD

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"GPIO", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"IST",         L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0003
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define GPIO_DEVICE_COOKIE       'gioC'

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD       cookie;
    UINT        rgRanges[GPIO_GROUPS];
    HANDLE      rgHandles[GPIO_GROUPS];
    GPIO_TABLE  rgGpioTbls[GPIO_GROUPS];
} GpioDevice_t;

//------------------------------------------------------------------------------
//  external global objects

extern GPIO_TABLE Omap35xx_Gpio;
extern GPIO_TABLE Tps659xx_Gpio;

//special case exported function to support IRQ lookup with NdisMRegisterInterrupt
extern _inline DWORD OmapGpioToHwIntr(UINT id);

//------------------------------------------------------------------------------
//
//  Function:  GIO_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL GIO_Deinit(DWORD context)
{
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (L"+GIO_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: GIO_Deinit: "
            L"Incorrect context parameter\r\n"
            ));
            goto cleanUp;
        }

    // clear handles
    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        pDevice->rgGpioTbls[i].Deinit(pDevice->rgHandles[i]);
        }

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-GIO_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_Init
//
//  Called by device manager to initialize device.
//
DWORD
GIO_Init(
    LPCTSTR szContext,
    LPCVOID pBusContext
    )
{
    UINT size;
    UINT offset;
    HANDLE hGpio;
    DWORD rc = (DWORD)NULL;
    GpioDevice_t *pDevice = NULL;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+GIO_Init(%s, 0x%08x)\r\n", szContext, pBusContext
        ));

    // Create device structure
    pDevice = (GpioDevice_t *)LocalAlloc(LPTR, sizeof(GpioDevice_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_Init: "
            L"Failed allocate GPIO driver structure\r\n"
            ));
        goto cleanUp;
        }

    // Set cookie
    pDevice->cookie = GPIO_DEVICE_COOKIE;
    pDevice->rgGpioTbls[0] = Omap35xx_Gpio;
    pDevice->rgGpioTbls[1] = Tps659xx_Gpio;

    // setup range and get handles
    offset = 0;
    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (pDevice->rgGpioTbls[i].Init(szContext, &hGpio, &size) == FALSE)
            {
            RETAILMSG(ZONE_ERROR, (L"ERROR: GIO_Init: "
                L"Failed to initialize Gpio table index(%d)\r\n", i
                ));
            goto cleanUp;
            }

        pDevice->rgRanges[i] = size + offset;
        offset += size;

        pDevice->rgHandles[i] = hGpio;
        }

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0) GIO_Deinit((DWORD)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-GIO_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
GIO_Open(
    DWORD context,
    DWORD accessCode,
    DWORD shareMode
    )
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_Close
//
//  This function closes the device context.
//
BOOL
GIO_Close(
    DWORD context
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_PowerUp
//
//  This function restores power to a device.
//
VOID
GIO_PowerUp(
    DWORD context
    )
{
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        pDevice->rgGpioTbls[i].PowerUp(pDevice->rgHandles[i]);
        }
	RETAILMSG(1, (L"GIO_PowerUp\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_PowerDown
//
//  This function suspends power to the device.
//
VOID
GIO_PowerDown(
    DWORD context
    )
{
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

	RETAILMSG(1, (L"GIO_PowerDown\r\n"));

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        pDevice->rgGpioTbls[i].PowerDown(pDevice->rgHandles[i]);
        }
}

//------------------------------------------------------------------------------
//
//  Function: GIO_SetBit - Set the value of the GPIO output pin
//
VOID
GIO_SetBit(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_SetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].SetBit(pDevice->rgHandles[i], id);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to set gpio(%d) bit\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_ClrBit - Clear the value of the GPIO output pin
//
VOID
GIO_ClrBit(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_ClrBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].ClrBit(pDevice->rgHandles[i], id);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to clr gpio(%d) bit\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return;
}


//------------------------------------------------------------------------------
//
//  Function: GIO_GetBit
//
DWORD GIO_GetBit(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    UINT res = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_GetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].GetBit(pDevice->rgHandles[i], id, &res);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to get gpio(%d) bit\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return res;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_SetMode
//
VOID GIO_SetMode(
    DWORD context,
    DWORD id,
    DWORD mode
    )
{
    UINT offset = 0;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_SetMode: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].SetMode(pDevice->rgHandles[i], id, mode);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to set gpio(%d) mode\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_GetMode
//
DWORD GIO_GetMode(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_GetMode: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].GetMode(pDevice->rgHandles[i], id, &mode);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to get gpio(%d) mode\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return mode;
}


//------------------------------------------------------------------------------
//
//  Function: GIO_Pullup
//
DWORD GIO_Pullup(
    DWORD context,
    DWORD id,
    DWORD enable
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_Pullup: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].Pullup(pDevice->rgHandles[i], id, enable);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to get gpio(%d) mode\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: GIO_Pulldown
//
DWORD GIO_Pulldown(
    DWORD context,
    DWORD id,
    DWORD enable
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_Pulldown: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].Pulldown(pDevice->rgHandles[i], id, enable);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to get gpio(%d) mode\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: GIO_InterruptInitialize
//
DWORD GIO_InterruptInitialize(
    DWORD context,
    DWORD id,
    HANDLE hEvent
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    RETAILMSG(0, (L"+GIO_InterruptInitialize, id = %d\r\n", id ));

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_InterruptInitialize: Incorrect context\r\n"));
        goto cleanUp;
        }

    // Check if we get correct context & pin id
    if (hEvent == INVALID_HANDLE_VALUE)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_InterruptInitialize: Invalid event handle\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].InterruptInitialize(pDevice->rgHandles[i], id, hEvent);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to initialize gpio(%d) interrupt\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_InterruptDone
//
DWORD GIO_InterruptDone(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_InterruptDone: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].InterruptDone(pDevice->rgHandles[i], id);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to re-enable gpio(%d) interrupt\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_InterruptDisable
//
DWORD GIO_InterruptDisable(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_InterruptDisable: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].InterruptDisable(pDevice->rgHandles[i], id);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to disable gpio(%d) interrupt.\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_InterruptMask
//
DWORD GIO_InterruptMask(
    DWORD context,
    DWORD id,
    BOOL  bEnable
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_InterruptMask: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].InterruptMask(pDevice->rgHandles[i], id, bEnable);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to mask gpio(%d) interrupt.\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_EnableWake
//
DWORD GIO_EnableWake(
    DWORD context,
    DWORD id,
    BOOL  bEnable
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_SetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].EnableWake(pDevice->rgHandles[i], id, bEnable);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to enable/disable gpio(%d) wake.\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_InterruptRelease
//
DWORD GIO_InterruptRelease(
    DWORD context,
    DWORD id
    )
{
    UINT offset = 0;
    UINT mode = -1;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GPIO_InterruptRelease: Incorrect context\r\n"));
        goto cleanUp;
        }

    for (int i = 0; i < GPIO_GROUPS; ++i)
        {
        if (id < pDevice->rgRanges[i])
            {
            id -= offset;
            rc = pDevice->rgGpioTbls[i].InterruptRelease(pDevice->rgHandles[i], id);
            RETAILMSG(rc == FALSE,
                (L"GIO: WARN! Unable to release gpio(%d) interrupt.\r\n", id
                ));
            break;
            }
        offset = pDevice->rgRanges[i];
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: GIO_GetSystemIrq
//
DWORD
GIO_GetSystemIrq(
    DWORD context,
    DWORD id
    )
{
    GpioDevice_t *pDevice = (GpioDevice_t*)context;
    DWORD rc = -1;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_GetIrq: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id <= pDevice->rgRanges[0])
        {
        // Mapping is simple
        rc = IRQ_GPIO_0 + id;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  GIO_IOControl
//
//  This function sends a command to a device.
//
BOOL
GIO_IOControl(
    DWORD context,
    DWORD code,
    UCHAR *pInBuffer,
    DWORD inSize,
    UCHAR *pOutBuffer,
    DWORD outSize,
    DWORD *pOutSize
    )
{
    int i;
    UINT offset = 0;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;
    DEVICE_IFC_GPIO ifc;
    DWORD id, value, mode, HwIntr;
    IOCTL_GPIO_POWER_STATE_IN PowerIn;
    IOCTL_GPIO_SET_DETAIL_DEBOUNCE_TIME_IN DebounceIn;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+GIO_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case IOCTL_DDK_GET_DRIVER_IFC:
            // We can give interface only to our peer in device process
            if (GetCurrentProcessId() != (DWORD)GetCallerProcess())
                {
                DEBUGMSG(ZONE_ERROR, (L"ERROR: GIO_IOControl: "
                    L"IOCTL_DDK_GET_DRIVER_IFC can be called only from "
                    L"device process (caller process id 0x%08x)\r\n",
                    GetCurrentProcessId()
                    ));
                SetLastError(ERROR_ACCESS_DENIED);
                goto cleanUp;
                }
            if ((pInBuffer == NULL) || (inSize < sizeof(GUID)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            if (IsEqualGUID(*(GUID*)pInBuffer, DEVICE_IFC_GPIO_GUID))
                {
                if (pOutSize != NULL) *pOutSize = sizeof(DEVICE_IFC_GPIO);
                if ((pOutBuffer == NULL) || (outSize < sizeof(DEVICE_IFC_GPIO)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }
                ifc.context                 = context;
                ifc.pfnSetBit               = GIO_SetBit;
                ifc.pfnClrBit               = GIO_ClrBit;
                ifc.pfnGetBit               = GIO_GetBit;
                ifc.pfnSetMode              = GIO_SetMode;
                ifc.pfnGetMode              = GIO_GetMode;
                ifc.pfnPullup               = GIO_Pullup;
                ifc.pfnPulldown             = GIO_Pulldown;
                ifc.pfnInterruptInitialize  = GIO_InterruptInitialize;
                ifc.pfnInterruptRelease     = GIO_InterruptRelease;
                ifc.pfnInterruptDone        = GIO_InterruptDone;
                ifc.pfnInterruptDisable     = GIO_InterruptDisable;
                ifc.pfnInterruptMask        = GIO_InterruptMask;
                ifc.pfnEnableWake           = GIO_EnableWake;
                ifc.pfnGetSystemIrq         = GIO_GetSystemIrq;
                if (!CeSafeCopyMemory(pOutBuffer, &ifc, sizeof(ifc))) break;
                rc = TRUE;
                break;
            }
            SetLastError(ERROR_INVALID_PARAMETER);
            break;

        case IOCTL_GPIO_SETBIT:
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            GIO_SetBit(context, id);
            rc = TRUE;
            break;

        case IOCTL_GPIO_CLRBIT:
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            GIO_ClrBit(context, id);
            rc = TRUE;
            break;

        case IOCTL_GPIO_GETBIT:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)) ||
               (pOutBuffer == NULL) || (outSize < sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            value = GIO_GetBit(context, id);
            if (!CeSafeCopyMemory(pOutBuffer, &value, sizeof(value)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            rc = TRUE;
            break;

        case IOCTL_GPIO_SETMODE:
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < 2 * sizeof(DWORD)) ||
                !CeSafeCopyMemory(
                    &id, &((DWORD*)pInBuffer)[0], sizeof(DWORD)
                    ) ||
                !CeSafeCopyMemory(
                    &mode, &((DWORD*)pInBuffer)[1], sizeof(DWORD)
                    ))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            GIO_SetMode(context, id, mode);
            rc = TRUE;
            break;

        case IOCTL_GPIO_GETMODE:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)) ||
                (pOutBuffer == NULL) || (outSize < sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            mode = GIO_GetMode(context, id);
            if (!CeSafeCopyMemory(pOutBuffer, &mode, sizeof(mode)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            rc = TRUE;
            break;

        case IOCTL_GPIO_SET_DEBOUNCE_TIME:
            if (pOutSize != 0)
				*pOutSize = sizeof(DWORD);

            if((pInBuffer == NULL) || (inSize < sizeof(IOCTL_GPIO_SET_DEBOUNCE_TIME_IN)))
			{
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
			}

			if(!CeSafeCopyMemory(&DebounceIn, pInBuffer, inSize))
			{
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
			}
            for (i = 0; i < GPIO_GROUPS; ++i)
                {
                if (DebounceIn.gpioId < pDevice->rgRanges[i])
                    {
                    DebounceIn.gpioId -= offset;
                    rc = pDevice->rgGpioTbls[i].IoControl(pDevice->rgHandles[i],
                            code, (UCHAR*)&DebounceIn, inSize, pOutBuffer, outSize,
                            (UINT*)pOutSize
                            );
                    RETAILMSG(rc == FALSE,
                        (L"GIO: WARN! Unable to set gpio(%d) debounce time\r\n",
                        DebounceIn.gpioId
                        ));
                    break;
                    }
                offset = pDevice->rgRanges[i];
                }
            break;

        case IOCTL_GPIO_GET_DEBOUNCE_TIME:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) ||
                (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            for (i = 0; i < GPIO_GROUPS; ++i)
                {
                if (id < pDevice->rgRanges[i])
                    {
                    id -= offset;
                    rc = pDevice->rgGpioTbls[i].IoControl(pDevice->rgHandles[i],
                            code, (UCHAR*)&id, sizeof(id), pOutBuffer, outSize,
                            (UINT*)pOutSize
                            );
                    RETAILMSG(rc == FALSE,
                        (L"GIO: WARN! Unable to set gpio(%d) debounce time\r\n",
                        id
                        ));
                    break;
                    }
                offset = pDevice->rgRanges[i];
                }
            break;

        case IOCTL_GPIO_GET_OUTPUT_BIT:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) ||
                (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            for (i = 0; i < GPIO_GROUPS; ++i)
                {
                if (id < pDevice->rgRanges[i])
                    {
                    id -= offset;
                    rc = pDevice->rgGpioTbls[i].IoControl(pDevice->rgHandles[i],
                            code, (UCHAR*)&id, sizeof(id), pOutBuffer, outSize,
                            (UINT*)pOutSize
                            );
                    RETAILMSG(rc == FALSE,
                        (L"GIO: WARN! Unable to get gpio(%d) output bit\r\n",
                        id
                        ));
                    break;
                    }
                offset = pDevice->rgRanges[i];
                }
            break;


        case IOCTL_GPIO_INIT_INTERRUPT:
            if ((pInBuffer == NULL) || (inSize != sizeof(IOCTL_GPIO_INIT_INTERRUPT_INFO)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            else
            {
                HANDLE      hCallerHandle       = GetCallerProcess();
                HANDLE      hCurrentProcHandle  = GetCurrentProcess();
                PIOCTL_GPIO_INIT_INTERRUPT_INFO pInitIntrInfo = (PIOCTL_GPIO_INIT_INTERRUPT_INFO)pInBuffer;
                HANDLE      hLocalEvent         = pInitIntrInfo->hEvent;

                if ((hCurrentProcHandle != hCallerHandle) && (hLocalEvent != INVALID_HANDLE_VALUE))
                {
                    if (pInitIntrInfo->hEvent != INVALID_HANDLE_VALUE)
                    {
                        BOOL    bStatus;

                        bStatus = DuplicateHandle(hCallerHandle, pInitIntrInfo->hEvent,
                                              hCurrentProcHandle, &hLocalEvent,
                                              DUPLICATE_SAME_ACCESS,
                                              FALSE,
                                              DUPLICATE_SAME_ACCESS);

                        if ((bStatus == FALSE) || (hLocalEvent == INVALID_HANDLE_VALUE))
                    	{
                            RETAILMSG(1, (TEXT("GIO_IOControl: IOCTL_GPIO_INIT_INTERRUPT unable to duplicate event handle.\r\n")));
                            break;
                    	}
                	}
               	}


                rc = GIO_InterruptInitialize(context, pInitIntrInfo->uGpioID, hLocalEvent);
                CloseHandle(hLocalEvent);
            }
            break;

        case IOCTL_GPIO_ACK_INTERRUPT:
            if ((pInBuffer == NULL) || (inSize != sizeof(DWORD)))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            else
            {
                DWORD   dwGpioID = *(DWORD*)pInBuffer;

                rc = GIO_InterruptDone(context, dwGpioID);
            }
            break;

        case IOCTL_GPIO_DISABLE_INTERRUPT:
			RETAILMSG(1, (L"+IOCTL_GPIO_DISABLE_INTERRUPT\r\n"));
            if ((pInBuffer == NULL) || (inSize != sizeof(DWORD)))
            {
				RETAILMSG(1, (L"ERROR: IOCTL_GPIO_DISABLE_INTERRUPT - wrong parameters\r\n"));
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            else
            {
                DWORD   dwGpioID = *(DWORD*)pInBuffer;

                rc = GIO_InterruptDisable(context, dwGpioID);
            }
            break;

        case IOCTL_GPIO_RELEASE_INTERRUPT:
            if ((pInBuffer == NULL) || (inSize != sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            else
                {
                DWORD   dwGpioID = *(DWORD*)pInBuffer;

                rc = GIO_InterruptRelease(context, dwGpioID);
                }
            break;

        case IOCTL_GPIO_MASK_INTERRUPT:
            if ((pInBuffer == NULL) || (inSize != sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            else
                {
                IOCTL_GPIO_INTERRUPT_MASK *psMaskParam = (IOCTL_GPIO_INTERRUPT_MASK *)pInBuffer;

                rc = GIO_InterruptMask(context, psMaskParam->gpioId, psMaskParam->bEnable);
                }
            break;

        case IOCTL_GPIO_ENABLE_WAKE:
            if ((pInBuffer == NULL) || (inSize != sizeof(IOCTL_GPIO_ENABLE_WAKE_IN)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            else
                {
                IOCTL_GPIO_ENABLE_WAKE_IN   *pWakeEnableParam = (IOCTL_GPIO_ENABLE_WAKE_IN*)pInBuffer;

                rc = GIO_EnableWake(context, pWakeEnableParam->gpioId, pWakeEnableParam->bEnable);
                }
            break;

        // special case for the getting the Omap Harware Interrupt Number 
        case IOCTL_GPIO_GET_OMAP_HW_INTR:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)) ||
                (pOutBuffer == NULL) || (outSize < sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
                
            if (id <= pDevice->rgRanges[0])
                {
                
                HwIntr = OmapGpioToHwIntr(id);
                       
                if (!CeSafeCopyMemory(pOutBuffer, &HwIntr, sizeof(HwIntr)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }
                
                rc = TRUE;
                }
            break;

        case IOCTL_GPIO_PULLUP_ENABLE:
			RETAILMSG(1, (L"+IOCTL_GPIO_PULLUP\r\n"));
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
            {
				RETAILMSG(1, (L"ERROR: IOCTL_GPIO_PULLUP - wrong parameters\r\n"));
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            GIO_Pullup(context, id, TRUE);
            rc = TRUE;
            break;

        case IOCTL_GPIO_PULLUP_DISABLE:
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            GIO_Pullup(context, id, FALSE);
            rc = TRUE;
            break;

        case IOCTL_GPIO_PULLDOWN_ENABLE:
			RETAILMSG(1, (L"+IOCTL_GPIO_PULLDOWN\r\n"));
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
            {
				RETAILMSG(1, (L"ERROR: IOCTL_GPIO_PULLDOWN - wrong parameters\r\n"));
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
            }
            GIO_Pulldown(context, id, TRUE);
            rc = TRUE;
            break;

        case IOCTL_GPIO_PULLDOWN_DISABLE:
            if (pOutSize != 0) *pOutSize = 0;
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            GIO_Pulldown(context, id, FALSE);
            rc = TRUE;
            break;

        case IOCTL_GPIO_GETIRQ:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) || (inSize < sizeof(DWORD)) ||
                !CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)) ||
                (pOutBuffer == NULL) || (outSize < sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            value = GIO_GetSystemIrq(context, id);
            if (!CeSafeCopyMemory(pOutBuffer, &value, sizeof(value)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            rc = TRUE;
            break;

        case IOCTL_GPIO_SET_POWER_STATE:
        case IOCTL_GPIO_GET_POWER_STATE:
            if (pOutSize != 0) *pOutSize = sizeof(DWORD);
            if ((pInBuffer == NULL) ||
                (inSize < sizeof(IOCTL_GPIO_POWER_STATE_IN)) ||
                !CeSafeCopyMemory(&PowerIn, pInBuffer, sizeof(PowerIn)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }
            for (i = 0; i < GPIO_GROUPS; ++i)
                {
                if (PowerIn.gpioId < pDevice->rgRanges[i])
                    {
                    PowerIn.gpioId -= offset;
                    rc = pDevice->rgGpioTbls[i].IoControl(pDevice->rgHandles[i],
                            code, (UCHAR*)&PowerIn, sizeof(PowerIn), pOutBuffer, outSize,
                            (UINT*)pOutSize
                            );

                    RETAILMSG(rc == FALSE,
                        (L"GIO: WARN! Unable to set/get gpio(%d) power state\r\n",
                        PowerIn.gpioId
                        ));
                    break;
                    }
                offset = pDevice->rgRanges[i];
                }
            break;

		default:
			DWORD* gpioId = (DWORD*)pInBuffer;
            if ((pInBuffer == NULL) ||
                (inSize < sizeof(DWORD)) ||
				!CeSafeCopyMemory(&id, pInBuffer, sizeof(DWORD)))
                {
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
                }

            for (i = 0; i < GPIO_GROUPS; ++i)
                {
                if (id < pDevice->rgRanges[i])
                    {
                    id -= offset;
					
                    rc = pDevice->rgGpioTbls[i].IoControl(pDevice->rgHandles[i],
                            code, pInBuffer, inSize, pOutBuffer, outSize,(UINT*)pOutSize
                            );
                    RETAILMSG(rc == FALSE,
                        (L"GIO: WARN! Unable to handle IOControl %x \r\n",code));
                    break;
                    }
                offset = pDevice->rgRanges[i];
                }
            break;
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-GIO_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  DllMain
//
//  Standard Windows DLL entry point.
//
BOOL
__stdcall
DllMain(
    HANDLE hDLL,
    DWORD reason,
    VOID *pReserved
    )
{
    switch (reason)
        {
        case DLL_PROCESS_ATTACH:
            RETAILREGISTERZONES((HMODULE)hDLL);
            DisableThreadLibraryCalls((HMODULE)hDLL);
            break;
        }
    return TRUE;
}

//------------------------------------------------------------------------------