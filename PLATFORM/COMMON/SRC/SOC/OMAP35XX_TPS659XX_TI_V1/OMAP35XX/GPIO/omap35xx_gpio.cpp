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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//
//  File: omap35xx_gpio.cpp
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <initguid.h>
#include <bus.h>
#include <omap35xx_guid.h>
#include <nkintr.h>

//------------------------------------------------------------------------------
//  Local Definitions

#define GPIO_DEVICE_COOKIE      'gioO'
#define GPIO_BITS_PER_BANK      (0x1F)
#define MAX_GPIO_COUNT          (32 * OMAP_GPIO_BANK_COUNT)

#define GPIO_BANK(x)            (x >> 5)
#define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    HANDLE hParentBus;
    DWORD powerEnabled[OMAP_GPIO_BANK_COUNT];
    DWORD memBase[OMAP_GPIO_BANK_COUNT];
    DWORD memLen[OMAP_GPIO_BANK_COUNT];
    CRITICAL_SECTION pCs[OMAP_GPIO_BANK_COUNT];
    OMAP_GPIO_REGS *ppGpioRegs[OMAP_GPIO_BANK_COUNT]; //We have 6 GPIO banks
    DWORD rgGpioDebounceState[OMAP_GPIO_BANK_COUNT];
} GpioDevice_t;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
    L"OmapGpio", PARAM_MULTIDWORD, TRUE, offset(GpioDevice_t, memBase),
    fieldsize(GpioDevice_t, memBase), NULL
    }, {
    L"OmapGpioLen", PARAM_MULTIDWORD, TRUE, offset(GpioDevice_t, memLen),
    fieldsize(GpioDevice_t, memLen), NULL
    }
};

static OMAP_DEVICE s_rgGpioClocks[] = {
    OMAP_DEVICE_GPIO1,
    OMAP_DEVICE_GPIO2,
    OMAP_DEVICE_GPIO3,
    OMAP_DEVICE_GPIO4,
    OMAP_DEVICE_GPIO5,
    OMAP_DEVICE_GPIO6,
};

OMAP_DEVCLKMGMT_FNTABLE    s_GpioDevClkMgmtTable;

//------------------------------------------------------------------------------
//  Local Functions

// Init function
static BOOL OmapGpioInit(LPCTSTR szContext, HANDLE *phContext, UINT *pGpioCount);
static BOOL OmapGpioDeinit(HANDLE hContext);
static BOOL OmapGpioSetMode(HANDLE hContext, UINT id, UINT mode);
static BOOL OmapGpioGetMode(HANDLE hContext, UINT id, UINT *pMode);
static BOOL OmapGpioInterruptInitialize(HANDLE hcontext,  UINT intrId, HANDLE hEvent);
static BOOL OmapGpioInterruptRelease(HANDLE hcontext,    UINT intrId);
static BOOL OmapGpioInterruptDone(HANDLE hcontext,    UINT intrId);
static BOOL OmapGpioInterruptDisable(HANDLE hcontext,    UINT intrId);
static BOOL OmapGpioInterruptMask(HANDLE hcontext,    UINT intrId, BOOL bEnable);
static BOOL OmapGpioWakeEnable(HANDLE hcontext,    UINT intrId, BOOL bEnable);
static BOOL OmapGpioSetBit(HANDLE hContext, UINT id);
static BOOL OmapGpioClrBit(HANDLE hContext, UINT id);
static BOOL OmapGpioGetBit(HANDLE hContext, UINT id, UINT *pValue);
static void OmapGpioPowerUp(HANDLE hContext);
static void OmapGpioPowerDown(HANDLE hContext);
static BOOL OmapGpioIoControl(HANDLE hContext, UINT code,
                               UCHAR *pinVal, UINT inSize, UCHAR *poutVal,
                               UINT outSize, UINT *pOutSize);
static BOOL OmapGpioPullup(HANDLE hcontext, UINT id, UINT enable);
static BOOL OmapGpioPulldown(HANDLE hcontext, UINT id, UINT enable);

//------------------------------------------------------------------------------
//  exported function table
GPIO_TABLE Omap35xx_Gpio = {
    OmapGpioInit,
    OmapGpioDeinit,
    OmapGpioSetMode,
    OmapGpioGetMode,
    OmapGpioInterruptInitialize,
    OmapGpioInterruptRelease,
    OmapGpioInterruptDone,
    OmapGpioInterruptDisable,
    OmapGpioInterruptMask,
    OmapGpioWakeEnable,
    OmapGpioSetBit,
    OmapGpioClrBit,
    OmapGpioGetBit,
    OmapGpioPowerUp,
    OmapGpioPowerDown,
    OmapGpioIoControl,
    OmapGpioPullup,
    OmapGpioPulldown
};

//special case exported function to support IRQ lookup with NdisMRegisterInterrupt
_inline 
DWORD 
OmapGpioToHwIntr(
    UINT id)
{
    return IRQ_GPIO_0 + id;
}

static DWORD    g_dwSysIntrMap[MAX_GPIO_COUNT];


//------------------------------------------------------------------------------
//
//  Function:  InternalEnableGpioDebounceClock
//
//  This routine is to workaround the requirement of a functional clock
//  when gpio debouncing for a gpio bank is required.  If debouncing is
//  required for a particular gpio pin the functional clock cannot be
//  disabled.  As a result we need to have a custom routine to increment
//  or decrement the kernel refcount of the functional clock for a gpio
//  bank
//
void
InternalEnableGpioDebounceClock(
    GpioDevice_t *pDevice,
    DWORD id,
    BOOL bEnable
    )
{
    // determine GPIO bank
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    DWORD prevState = pDevice->rgGpioDebounceState[bank];

    if (bEnable == TRUE)
        {
        pDevice->rgGpioDebounceState[bank] |= bit;
        }
    else
        {
        pDevice->rgGpioDebounceState[bank] &= ~bit;
        }

    if (prevState != pDevice->rgGpioDebounceState[bank])
        {
        s_GpioDevClkMgmtTable.pfnEnableDeviceFClock(s_rgGpioClocks[bank], bEnable);
        }
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioInit
//
//  Called by client to initialize device.
//
BOOL
OmapGpioInit(
    LPCTSTR szContext,
    HANDLE *phContext,
    UINT   *pGpioCount
    )
{
    DWORD bit;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = NULL;
    PHYSICAL_ADDRESS pa;
    DWORD size;
    UINT8 i = 0;
    DWORD mask;


    DEBUGMSG(ZONE_FUNCTION, (
        L"+OmapGpioInit(%s)\r\n", szContext
        ));

    // Create device structure
    pDevice = (GpioDevice_t *)LocalAlloc(LPTR, sizeof(GpioDevice_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed allocate GPIO driver structure\r\n"
            ));
        goto cleanUp;
        }

    memset(pDevice, 0, sizeof(GpioDevice_t));

    // Set cookie
    pDevice->cookie = GPIO_DEVICE_COOKIE;

    // Retrieve the kernel gpio fclk routines.  Necessary because
    // gpio debounce hw block requires the fclk for the GPIO bank
    // to always be on.
    if (!KernelIoControl(IOCTL_PRCM_DEVICE_GET_DEVICEMANAGEMENTTABLE, (void*)&KERNEL_DEVCLKMGMT_GUID,
            sizeof(KERNEL_DEVCLKMGMT_GUID), &s_GpioDevClkMgmtTable,
            sizeof(OMAP_DEVCLKMGMT_FNTABLE),
            NULL))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed get clock management table from kernel\r\n"
            ));
        goto cleanUp;
        }

    // Initialize critical sections
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        InitializeCriticalSection(&pDevice->pCs[i]);

    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams
            ) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed read GPIO driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        {
        // Map GPIO registers
        pa.QuadPart = pDevice->memBase[i];
        size = pDevice->memLen[i];
        pDevice->ppGpioRegs[i] = (OMAP_GPIO_REGS*)MmMapIoSpace(pa, size, FALSE);
        if (pDevice->ppGpioRegs[i] == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
                 L"Failed map GIO%d controller registers\r\n",i
                ));
            goto cleanUp;
            }

        // Enable the Interface clock
        s_GpioDevClkMgmtTable.pfnEnableDeviceIClock(s_rgGpioClocks[i], TRUE);

        // update the fclk refcount for a gpio bank based on
        // number of debounce pins enabled.
        bit = 0;
        pDevice->rgGpioDebounceState[i] = 0;
        mask = INREG32(&pDevice->ppGpioRegs[i]->DEBOUNCENABLE);
        while (mask != 0)
            {
            if (mask & 1)
                {
                InternalEnableGpioDebounceClock(pDevice, (32 * i) + bit, TRUE);
                }

            // check next bit
            mask >>= 1;
            bit++;
            }

        // Disable the Interface clock
        s_GpioDevClkMgmtTable.pfnEnableDeviceIClock(s_rgGpioClocks[i], FALSE);

        }

    for (i = 0; i < MAX_GPIO_COUNT; i++)
        {
        g_dwSysIntrMap[i] = SYSINTR_NOP;
        }

    // Open parent bus
    pDevice->hParentBus = CreateBusAccessHandle(szContext);
    if (pDevice->hParentBus == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInit: "
            L"Failed open parent bus driver\r\n"
            ));
        goto cleanUp;
        }

    // indicate gpio registers need to be saved for OFF mode
    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

    // Return non-null value
    rc = TRUE;
    *phContext = (HANDLE)pDevice;
    *pGpioCount = MAX_GPIO_COUNT;

cleanUp:
    if (rc == FALSE) OmapGpioDeinit((HANDLE)pDevice);
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioInit()\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioDeinit
//
//  Called by device manager to uninitialize device.
//
BOOL
OmapGpioDeinit(
    HANDLE context
    )
{
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;
    UINT8 i = 0;

    DEBUGMSG(ZONE_FUNCTION, (L"+OmapGpioDeinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: OmapGpioDeinit: "
            L"Incorrect context parameter\r\n"
            ));
            goto cleanUp;
        }

    // Delete critical sections
    for (i = 0; i < OMAP_GPIO_BANK_COUNT; i++)
        DeleteCriticalSection(&pDevice->pCs[i]);

    // Unmap module registers
    for (i = 0 ; i < OMAP_GPIO_BANK_COUNT; i++)
    {
       if (pDevice->ppGpioRegs[i] != NULL)
        {
            MmUnmapIoSpace((VOID*)pDevice->ppGpioRegs[i], pDevice->memLen[i]);
        }
    }

    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioDeinit()\r\n"));
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioSetMode
//
BOOL
OmapGpioSetMode(
    HANDLE context,
    UINT id,
    UINT mode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetMode: "
            L"Incorrect context\r\n"
            ));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        UINT32 mask = 1 << (bit);
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
        OMAP_GPIO_REGS *pGpio = pDevice->ppGpioRegs[bank];

        EnterCriticalSection(pCs);

        // set gpio direction
        if ((mode & GPIO_DIR_INPUT) != 0)
            {
            SETREG32(&pGpio->OE, mask);
            }
        else
            {
            CLRREG32(&pGpio->OE, mask);
            }

        // set debounce mode
        if ((mode & GPIO_DEBOUNCE_ENABLE) != 0)
            {
            InternalEnableGpioDebounceClock(pDevice, id, TRUE);
            SETREG32(&pGpio->DEBOUNCENABLE, mask);
            }
        else
            {
            CLRREG32(&pGpio->DEBOUNCENABLE, mask);
            InternalEnableGpioDebounceClock(pDevice, id, FALSE);
            }

        // set edge/level detect mode
        if ((mode & GPIO_INT_LOW) != 0)
            {
            SETREG32(&pGpio->LEVELDETECT0, mask);
            }
        else
            {
            CLRREG32(&pGpio->LEVELDETECT0, mask);
            }

        if ((mode & GPIO_INT_HIGH) != 0)
            {
            SETREG32(&pGpio->LEVELDETECT1, mask);
            }
        else
            {
            CLRREG32(&pGpio->LEVELDETECT1, mask);
            }

        if ((mode & GPIO_INT_LOW_HIGH) != 0)
            {
            SETREG32(&pGpio->RISINGDETECT, mask);
            }
        else
            {
            CLRREG32(&pGpio->RISINGDETECT, mask);
            }

        if ((mode & GPIO_INT_HIGH_LOW) != 0)
            {
            SETREG32(&pGpio->FALLINGDETECT, mask);
            }
        else
            {
            CLRREG32(&pGpio->FALLINGDETECT, mask);
            }

        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioGetMode
//
BOOL
OmapGpioGetMode(
    HANDLE context,
    UINT id,
    UINT *pMode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioGetMode: "
            L"Incorrect context\r\n"
            ));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        *pMode = 0;
        OMAP_GPIO_REGS *pGpio = pDevice->ppGpioRegs[bank];
        UINT32 mask = 1 << (bit);
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);

        // get edge mode
        if ((INREG32(&pGpio->OE) & mask) != 0)
            {
            *pMode |= GPIO_DIR_INPUT;
            }
        else
            {
            *pMode |= GPIO_DIR_OUTPUT;
            }

        // get debounce mode
        if ((INREG32(&pGpio->DEBOUNCENABLE) & mask) != 0)
            {
            *pMode |= GPIO_DEBOUNCE_ENABLE;
            }

        // get edge/level detect mode
        if ((INREG32(&pGpio->LEVELDETECT0) & mask) != 0)
            {
            *pMode |= GPIO_INT_LOW;
            }

        if ((INREG32(&pGpio->LEVELDETECT1) & mask) != 0)
            {
            *pMode |= GPIO_INT_HIGH;
            }

        if ((INREG32(&pGpio->RISINGDETECT) & mask) != 0)
            {
            *pMode |= GPIO_INT_LOW_HIGH;
            }

        if ((INREG32(&pGpio->FALLINGDETECT) & mask) != 0)
            {
            *pMode |= GPIO_INT_HIGH_LOW;
            }

        LeaveCriticalSection(pCs);
        rc = TRUE;
        }

cleanUp:
    return rc;
}



//------------------------------------------------------------------------------
//
//  Function: OmapGpioInterruptInitialize
//
BOOL
OmapGpioInterruptInitialize(
    HANDLE context,
    UINT id,
    HANDLE hEvent
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInterruptInitialize: Incorrect context\r\n"));
        }
    else
        {
        DWORD   dwSysIntr = g_dwSysIntrMap[id];

		if(dwSysIntr == SYSINTR_NOP)
		{
            DWORD   dwHWIntr = OmapGpioToHwIntr(id);

            // Get sysintr values from the OAL for PENIRQ interrupt
			if(KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwHWIntr, sizeof(dwHWIntr), &dwSysIntr, sizeof(dwSysIntr), 0))
			{
                g_dwSysIntrMap[id] = dwSysIntr;
				RETAILMSG(1, (L"%S: GPIO_%d sys intr %d(%d)\r\n", __FUNCTION__, id, dwSysIntr, dwHWIntr));
			}

            if (g_dwSysIntrMap[id] != SYSINTR_NOP)
                {
                // Make sure hEvent is not NULL. We can't disassociate system interrupt
                // from an event once the association is set as the T2 interrupts do. because
                // there is not API call to do so in CE.
                rc = InterruptInitialize(g_dwSysIntrMap[id], hEvent, NULL, 0);
                }
            }
        }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrRelease
//
BOOL
OmapGpioInterruptRelease(
    HANDLE context,
    UINT id
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInterruptRelease: Incorrect context\r\n"));
        }
    else
        {
        DWORD   dwSysIntr = g_dwSysIntrMap[id];

        if (dwSysIntr != SYSINTR_NOP)
            {
            // Get sysintr values from the OAL for PENIRQ interrupt
            rc = KernelIoControl( 
                    IOCTL_HAL_RELEASE_SYSINTR, 
                    &(dwSysIntr), 
                    sizeof(dwSysIntr),
                    NULL, 
                    0, 
                    NULL
                    ); 

            if (rc == TRUE)
                {
                g_dwSysIntrMap[id] = SYSINTR_NOP;
                }
            }
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrEnable
//
BOOL OmapGpioInterruptDone(HANDLE context, UINT id)
{
    BOOL    rc = 0;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInterruptDone: Incorrect context\r\n"));
    }
    else
	{
        if(g_dwSysIntrMap[id] != SYSINTR_NOP)
        {
            // Simply call the kernel to enable the interrupt via InterruptDone.
            InterruptDone(g_dwSysIntrMap[id]);
            rc = TRUE;
        }
    }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrDisable
//
BOOL
OmapGpioInterruptDisable(
    HANDLE context,
    UINT id
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInterruptDisable: Incorrect context\r\n"));
        }
    else
        {
        if (g_dwSysIntrMap[id] != SYSINTR_NOP)
            {
            // Simply call the kernel to disable the interrupt via InterruptDisable.
            InterruptDisable(g_dwSysIntrMap[id]);
            rc = TRUE;
            }
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrDisable
//
BOOL
OmapGpioInterruptMask(
    HANDLE context,
    UINT id,
    BOOL bEnable
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioInterruptMask: Incorrect context\r\n"));
        }
    else
        {
        if (g_dwSysIntrMap[id] != SYSINTR_NOP)
            {
            // Simply call the kernel to disable the interrupt via InterruptDisable.
            InterruptMask(g_dwSysIntrMap[id], bEnable);
            rc = TRUE;
            }
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioIntrDisable
//
BOOL
OmapGpioWakeEnable(
    HANDLE context,
    UINT id,
    BOOL bEnable
    )
{
    BOOL    rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE) || (id >= MAX_GPIO_COUNT))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioWakeEnable: Incorrect context\r\n"));
        }
    else
        {
        if (g_dwSysIntrMap[id] != SYSINTR_NOP)
            {
			RETAILMSG(0, (L"OmapGpioWakeEnable: %d(%d)\r\n", id, g_dwSysIntrMap[id]));
            rc = KernelIoControl(
                    bEnable ? IOCTL_HAL_ENABLE_WAKE : IOCTL_HAL_DISABLE_WAKE, 
                    &(g_dwSysIntrMap[id]), sizeof(g_dwSysIntrMap[id]), 
                    NULL, 0, NULL);

            if (rc == FALSE)
                {
                DEBUGMSG(ZONE_ERROR, (L"WARN: KPG_Init: "
                    L"Failed enable keyboard as wakeup source\r\n"
                    ));
                }
            }
        }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioSetBit - Set the value of the GPIO output pin
//
BOOL
OmapGpioSetBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);
        SETREG32(p, 1 << (bit));
        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioClrBit
//
BOOL
OmapGpioClrBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioClrBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];

        EnterCriticalSection(pCs);
        CLRREG32(p, 1 << (bit));
        LeaveCriticalSection(pCs);

        // indicate gpio registers need to be saved for OFF mode
        HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioGetBit
//
BOOL
OmapGpioGetBit(
    HANDLE context,
    UINT id,
    UINT *pValue
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioGetBit: Incorrect context\r\n"));
        goto cleanUp;
        }

    if (id < MAX_GPIO_COUNT)
        {
        volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAIN;
        CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
        EnterCriticalSection(pCs);
        *pValue = (INREG32(p) >> (bit)) & 0x01;
        LeaveCriticalSection(pCs);
        rc = TRUE;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioIoControl
//
//  This function sends a command to a device.
//
BOOL
OmapGpioIoControl(
    HANDLE  context,
    UINT    code,
    UCHAR  *pInBuffer,
    UINT    inSize,
    UCHAR  *pOutBuffer,
    UINT    outSize,
    UINT   *pOutSize
    )
{
    UINT bank;
    BOOL rc = FALSE;
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+OmapGpioIOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioIOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case IOCTL_GPIO_SET_DEBOUNCE_TIME:
            {
                if ((pInBuffer == NULL) ||
                    (inSize < sizeof(IOCTL_GPIO_SET_DEBOUNCE_TIME_IN)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }

                IOCTL_GPIO_SET_DEBOUNCE_TIME_IN *pDebounce;

                pDebounce = (IOCTL_GPIO_SET_DEBOUNCE_TIME_IN*)pInBuffer;

                if (pDebounce->gpioId < MAX_GPIO_COUNT)
                    {
                    CRITICAL_SECTION *pCs;
                    bank = GPIO_BANK(pDebounce->gpioId);
                    pCs = &pDevice->pCs[bank];
                    EnterCriticalSection(pCs);
                    OUTREG32(&pDevice->ppGpioRegs[bank]->DEBOUNCINGTIME,
                        pDebounce->debounceTime);
                    LeaveCriticalSection(pCs);

                    // indicate gpio registers need to be saved for OFF mode
                    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

                    rc = TRUE;
                    }
            }
            break;

        case IOCTL_GPIO_GET_DEBOUNCE_TIME:
            {
                if ((pInBuffer == NULL) || (pOutBuffer == NULL) ||
                    (inSize < sizeof(UINT)) ||
                    (outSize < sizeof(UINT)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }

                UINT *pId;
                UINT *pOut;

                pId = (UINT*)pInBuffer;
                pOut = (UINT*)pOutBuffer;

                if (*pId < MAX_GPIO_COUNT)
                    {

                    bank = GPIO_BANK(*pId);

                    *pOut = INREG32(&pDevice->ppGpioRegs[bank]->DEBOUNCINGTIME);

                    // indicate gpio registers need to be saved for OFF mode
                    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

                    rc = TRUE;
                    }
            }
            break;

        case IOCTL_GPIO_GET_OUTPUT_BIT:
            {
                if ((pInBuffer == NULL) || (pOutBuffer == NULL) ||
                    (inSize < sizeof(UINT)) ||
                    (outSize < sizeof(UINT)))
                    {
                    SetLastError(ERROR_INVALID_PARAMETER);
                    break;
                    }

                UINT *pId;
                UINT *pOut;

                pId = (UINT*)pInBuffer;
                pOut = (UINT*)pOutBuffer;

				UINT bit = GPIO_BIT(*pId);
				UINT temp;

				RETAILMSG(0, (L"OMAP35xx_GPIO:%S, IOCTL_GPIO_GET_OUTPUT_BIT, iD = %d, bit = %d\r\n",__FUNCTION__, *pId, bit));

                if (*pId < MAX_GPIO_COUNT)
                    {

                    bank = GPIO_BANK(*pId);

                    temp = INREG32(&pDevice->ppGpioRegs[bank]->DATAOUT);
					*pOut = ( temp >> (bit) ) & 0x01; 

					RETAILMSG(0, (L"OMAP35xx_GPIO:%S, IOCTL_GPIO_GET_OUTPUT_BIT, out = %d\r\n",__FUNCTION__, *pOut));

                    // indicate gpio registers need to be saved for OFF mode
                    HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

                    rc = TRUE;
                    }
            }
            break;

        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-OmapGpioIOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OmapGpioPullup
//
BOOL
OmapGpioPullup(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    /*BOOL    rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    GpioDevice_t *pDevice = (GpioDevice_t*)context;

    // Check if we get correct context & pin id
    if ((pDevice == NULL) || (pDevice->cookie != GPIO_DEVICE_COOKIE))
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: OmapGpioSetBit: Incorrect context\r\n"));
        }
    else
        {
        if (id < MAX_GPIO_COUNT)
            {
            volatile UINT *p = &pDevice->ppGpioRegs[bank]->DATAOUT;
            CRITICAL_SECTION *pCs = &pDevice->pCs[bank];
            UINT val;

            EnterCriticalSection(pCs);
            InternalSetGpioBankPowerState(pDevice, id, D0);
            if (enable)
                {
                val = 0 << (bit);
                }
            else
                {
                val = 1 << (bit);
                }
            SETREG32(p, val);
            InternalSetGpioBankPowerState(pDevice, id, D4);
            LeaveCriticalSection(pCs);

            // indicate gpio registers need to be saved for OFF mode
            HalContextUpdateDirtyRegister(HAL_CONTEXTSAVE_GPIO);

            rc = TRUE;
            }
        }
*/
    return 1;//rc;
}


//------------------------------------------------------------------------------
//
//  Function: OmapGpioPulldown
//
BOOL
OmapGpioPulldown(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioPowerUp
//
//  This function restores power to a device.
//
VOID
OmapGpioPowerUp(
    HANDLE hContext
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  OmapGpioPowerDown
//
//  This function suspends power to the device.
//
VOID
OmapGpioPowerDown(
    HANDLE hContext
    )
{
}

//------------------------------------------------------------------------------
