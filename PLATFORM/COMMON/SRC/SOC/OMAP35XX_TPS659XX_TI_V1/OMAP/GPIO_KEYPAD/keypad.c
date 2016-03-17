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
//  File: keypad.c
//
//  This file implements device driver for keypad. The driver isn't implemented
//  as classical keyboard driver. Instead implementation uses stream driver
//  model and it calls keybd_event to pass information to GWE subsystem.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <keypad.h>
#include <pmpolicy.h>

#include <initguid.h>
#include <gpio.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)
#define ZONE_IST            DEBUGZONE(4)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"keypad", {
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

#define KPG_DEVICE_COOKIE           'KPGG'

#define VK_KEYS                     256
#define DWORD_BITS                  32

//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD priority256;
    DWORD enableWake;
    DWORD clockDivider;
    IOCTL_GPIO_SET_DEBOUNCE_TIME_IN debounce;
    DWORD samplePeriod;
    DWORD firstRepeat;
    DWORD nextRepeat;
    HANDLE hIntrEvent;
    HANDLE hIntrThread;
    HANDLE hGpio;
    BOOL intrThreadExit;
    DWORD powerMask;
    CEDEVICE_POWER_STATE powerState;

    DWORD bEnableOffKey;
  
} KeypadDevice_t;

typedef struct {
    BOOL pending;
    BOOL remapped;
    DWORD time;
    DWORD state;
} KeypadRemapState_t;

typedef struct {
    BOOL pending;
    DWORD time;
    BOOL blocked;
} KeypadRepeatState_t;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, priority256),
        fieldsize(KeypadDevice_t, priority256), (VOID*)100
    }, {
        L"EnableWake", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, enableWake),
        fieldsize(KeypadDevice_t, enableWake), (VOID*)1
    }, {
        L"ClockDivider", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, clockDivider),
        fieldsize(KeypadDevice_t, clockDivider), (VOID*)5
    }, {
        L"DebounceCounter", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, debounce.debounceTime),
        fieldsize(KeypadDevice_t, debounce.debounceTime), (VOID*)10
    }, {
        L"SamplePeriod", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, samplePeriod),
        fieldsize(KeypadDevice_t, samplePeriod), (VOID*)40
    }, {
        L"FirstRepeat", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, firstRepeat),
        fieldsize(KeypadDevice_t, firstRepeat), (VOID*)500
    }, {
        L"NextRepeat", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, nextRepeat),
        fieldsize(KeypadDevice_t, nextRepeat), (VOID*)125
    },  {
        L"EnableOffKey", PARAM_DWORD, FALSE, 
        offset(KeypadDevice_t, bEnableOffKey),
        fieldsize(KeypadDevice_t, bEnableOffKey), (VOID*)0
    }, {
        L"PowerMask", PARAM_DWORD, TRUE, 
        offset(KeypadDevice_t, powerMask),
        fieldsize(KeypadDevice_t, powerMask), NULL
    }
};

//------------------------------------------------------------------------------
BOOL
KPG_Deinit(
    DWORD context
    );

DWORD KPG_IntrThread(
    VOID *pContext
    );

static VOID
PhysicalStateToVirtualState(
    UINT8 *gpioInput,
    DWORD vkNewState[],
    BOOL *pKeyDown
    );

static VOID
VirtualKeyRemap(
    DWORD time,
    BOOL *pKeyDown,
    KeypadRemapState_t *pRemapState,
    DWORD vkNewState[]
    );

static VOID
PressedReleasedKeys(
    KeypadDevice_t *pDevice,
    const DWORD vkState[],
    const DWORD vkNewState[]
    );

static VOID
AutoRepeat(
    const KeypadDevice_t *pDevice,
    const DWORD vkNewState[],
    DWORD time,
    KeypadRepeatState_t *pRepeatState
    );

//------------------------------------------------------------------------------
//
//  Function:  SendKeyPadEvent
//
//  Send keypad event
//
static VOID 
SendKeyPadEvent(
    BYTE bVk,
    BYTE bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo)
{
    USHORT index;
    UCHAR vk_extra, order;
    
    order = KEYPAD_EXTRASEQ_ORDER_NONE; // no extra key needed
    
    // Check extra virtual key sequence table
    for (index = 0; index < g_keypadExtraSeq.count; index ++)
        {
        if (g_keypadExtraSeq.pItem[index].vk_orig == bVk)
            {
            vk_extra = g_keypadExtraSeq.pItem[index].vk_extra;
            order = g_keypadExtraSeq.pItem[index].order;
            }
        }
      
    // Check to send extra vk first  
    if (order == KEYPAD_EXTRASEQ_ORDER_EXTRAFIRST || 
        (order == KEYPAD_EXTRASEQ_ORDER_EXTRAORIG && (dwFlags & KEYEVENTF_KEYUP) == 0) )
        {
        keybd_event(
            vk_extra,
            0,
            dwFlags | KEYEVENTF_SILENT,
            dwExtraInfo);
        }
          
    // Send original vk
    keybd_event(
        bVk,
        bScan,
        dwFlags,
        dwExtraInfo);
      
   // Check to send extra key
   if (order == KEYPAD_EXTRASEQ_ORDER_ORIGFIRST || 
       (order == KEYPAD_EXTRASEQ_ORDER_EXTRAORIG && (dwFlags & KEYEVENTF_KEYUP)))
       {
       keybd_event(
           vk_extra,
           0,
           dwFlags | KEYEVENTF_SILENT,
           dwExtraInfo);
       }
}

//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
//  Sets the device power state
//
BOOL
SetPowerState(
    KeypadDevice_t         *pDevice, 
    CEDEVICE_POWER_STATE    power
    )
{
    BOOL rc = FALSE;
    
    DEBUGMSG(ZONE_FUNCTION, (
        L"+SetPowerState(0x%08X, 0x%08x)\r\n", pDevice, power
        ));

    switch (power)
        {
        case D0:
        case D1:
        case D2:            
        case D3:
        case D4:
            break;

        default:
            RETAILMSG(ZONE_WARN, (L"WARN: KPG::SetPowerState: "
                L"Invalid power state (%d)\r\n", power
                ));            
            goto cleanUp;
        }

    pDevice->powerState = power;

    rc = TRUE;
    
cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (
        L"-SetPowerState(0x%08X, 0x%08x)\r\n", pDevice, power
        ));
        
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_Init
//
//  Called by device manager to initialize device.
//
DWORD
KPG_Init(
    LPCTSTR szContext,
    LPCVOID pBusContext
    )
{
    DWORD rc = (DWORD)NULL;
    KeypadDevice_t *pDevice = NULL;
    UINT16 ix;
    UINT16 gpio=0;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+KPG_Init(%s, 0x%08x)\r\n", szContext, pBusContext
        ));

    // Create device structure
    pDevice = (KeypadDevice_t *)LocalAlloc(LPTR, sizeof(KeypadDevice_t));
    if (pDevice == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPG_Init: "
            L"Failed allocate KDP driver structure\r\n"
            ));
        goto cleanUp;
        }

    memset(pDevice, 0, sizeof(KeypadDevice_t));

    // Set cookie & initialize critical section
    pDevice->cookie = KPG_DEVICE_COOKIE;
    
    // Read device parameters
    if (GetDeviceRegistryParams(
            szContext, pDevice, dimof(s_deviceRegParams), s_deviceRegParams)
            != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPG_Init: "
            L"Failed read KPG driver registry parameters\r\n"
            ));
        goto cleanUp;
        }

    // Open gpio driver
    pDevice->hGpio = GPIOOpen();
    if (pDevice->hGpio == NULL)
        {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPG_Init: "
            L"Failed to open Gpio driver \r\n"
            ));
        goto cleanUp;
        }
    
    // Create interrupt event
    pDevice->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (pDevice->hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: KPG_Init: "
            L"Failed create interrupt event\r\n"
            ));
        goto cleanUp;
    }

    for(ix = 0 ; ix < g_GpioKeypadVK.count ; ix++)
        {
        const KEYPAD_GPIO_VK_ITEM *pItem = &g_GpioKeypadVK.pItem[ix];
        UINT16 gpio = pItem->gpio;

        // Initialize GPIO pins
        GPIOSetMode(pDevice->hGpio, gpio,
            GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW|GPIO_INT_LOW_HIGH|GPIO_DEBOUNCE_ENABLE
            );

        // Enable wakeup from keyboard if required
        if (pDevice->enableWake != 0) 
            {
            if (!GPIOWakeEnable(pDevice->hGpio, gpio) ) 
                {
                DEBUGMSG(ZONE_ERROR, (L"WARN: KPG_Init: "
                    L"Failed enable keyboard as wakeup source\r\n"
                    ));
                }
            }

        // Initialize interrupt
        if (!GPIOInterruptInitialize(pDevice->hGpio, gpio, pDevice->hIntrEvent))
            {
            DEBUGMSG (ZONE_ERROR, (L"ERROR: KPG_Init: "
                L"InterruptInitialize failed\r\n"
                ));
            goto cleanUp;
            }

        // Set GPIO Debounce time
        pDevice->debounce.gpioId = gpio;
        GPIOIoControl(pDevice->hGpio, 
            IOCTL_GPIO_SET_DEBOUNCE_TIME, 
            (UCHAR*)&(pDevice->debounce), sizeof(pDevice->debounce), NULL, 0, NULL, NULL
            );      
        }
    
    // Start interrupt service thread
    pDevice->intrThreadExit = FALSE;
    pDevice->hIntrThread = CreateThread(
        NULL, 0, KPG_IntrThread, pDevice, 0,NULL
        );
    if (!pDevice->hIntrThread)
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPG_Init: "
            L"Failed create interrupt thread\r\n"
            ));
        goto cleanUp;
        }
    
    // Set thread priority
    CeSetThreadPriority(pDevice->hIntrThread, pDevice->priority256);

    // Return non-null value
    rc = (DWORD)pDevice;

cleanUp:
    if (rc == 0)
        {
        KPG_Deinit((DWORD)pDevice);
        }
    DEBUGMSG(ZONE_FUNCTION, (L"-KPG_Init(rc = %d\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_Deinit
//
//  Called by device manager to uninitialize device.
//
BOOL
KPG_Deinit(
    DWORD context
    )
{
    BOOL rc = FALSE;
    KeypadDevice_t *pDevice = (KeypadDevice_t*)context;
    UINT16 ix;

    DEBUGMSG(ZONE_FUNCTION, (L"+KPG_Deinit(0x%08x)\r\n", context));

    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != KPG_DEVICE_COOKIE))
        {
        DEBUGMSG (ZONE_ERROR, (L"ERROR: KPG_Deinit: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // Signal stop to threads
    pDevice->intrThreadExit = TRUE;
        
    // Close interrupt thread
    if (pDevice->hIntrThread != NULL)
        {
        // Set event to wake it
        SetEvent(pDevice->hIntrEvent);
        // Wait until thread exits
        WaitForSingleObject(pDevice->hIntrThread, INFINITE);
        // Close handle
        CloseHandle(pDevice->hIntrThread);
        }

    // Disable interrupt
     for(ix = 0 ; ix < g_GpioKeypadVK.count ; ix++)
        {
            GPIOInterruptDisable(pDevice->hGpio, g_GpioKeypadVK.pItem[ix].gpio);
            GPIOInterruptRelease(pDevice->hGpio, g_GpioKeypadVK.pItem[ix].gpio);
        }
    // Close interrupt handler
    if (pDevice->hIntrEvent != NULL) CloseHandle(pDevice->hIntrEvent);
    
    // Close Gpio 
    if (pDevice->hGpio != NULL) GPIOClose(pDevice->hGpio);
    
    // Free device structure
    LocalFree(pDevice);

    // Done
    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-KPG_Deinit(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_Open
//
//  Called by device manager to open a device for reading and/or writing.
//
DWORD
KPG_Open(
    DWORD context, 
    DWORD accessCode, 
    DWORD shareMode
    )
{
    return context;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_Open
//
//  This function closes the device context.
//
BOOL
KPG_Close(
    DWORD context
    )
{
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_PowerUp
//
//  Called on resume of device.  Current implementation of keypad driver
//  will disable the keypad interrupts before suspend.  Make sure the
//  keypad interrupts are re-enabled on resume.
//
void
KPG_PowerUp(
    DWORD context
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_IOControl
//
//  This function sends a command to a device.
//
BOOL
KPG_IOControl(
    DWORD context, 
    DWORD code, 
    UCHAR *pInBuffer, 
    DWORD inSize, 
    UCHAR *pOutBuffer,
    DWORD outSize, 
    DWORD *pOutSize
    )
{
    BOOL rc = FALSE;
    KeypadDevice_t *pDevice = (KeypadDevice_t*)context;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+KPG_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));
        
    // Check if we get correct context
    if ((pDevice == NULL) || (pDevice->cookie != KPG_DEVICE_COOKIE))
        {
        RETAILMSG(ZONE_ERROR, (L"ERROR: KPG_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }
    
    switch (code)
        {
        case IOCTL_POWER_CAPABILITIES: 
            DEBUGMSG(ZONE_INFO, (L"KPG: Received IOCTL_POWER_CAPABILITIES\r\n"));
            if (pOutBuffer && outSize >= sizeof (POWER_CAPABILITIES) && 
                pOutSize) 
                {
                    __try 
                        {
                        PPOWER_CAPABILITIES PowerCaps;
                        PowerCaps = (PPOWER_CAPABILITIES)pOutBuffer;
         
                        // Only supports D0 (permanently on) and D4(off.         
                        memset(PowerCaps, 0, sizeof(*PowerCaps));
                        PowerCaps->DeviceDx = (UCHAR)pDevice->powerMask;
                        *pOutSize = sizeof(*PowerCaps);
                        
                        rc = TRUE;
                        }
                    __except(EXCEPTION_EXECUTE_HANDLER) 
                        {
                        RETAILMSG(ZONE_ERROR, (L"exception in ioctl\r\n"));
                        }
                }
            break;

        // determines whether changing power state is feasible
        case IOCTL_POWER_QUERY: 
            DEBUGMSG(ZONE_INFO,(L"KPG: Received IOCTL_POWER_QUERY\r\n"));
            if (pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                // Return a good status on any valid query, since we are 
                // always ready to change power states (if asked for state 
                // we don't support, we move to next highest, eg D3->D4).
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
 
                    if (VALID_DX(ReqDx)) 
                        {
                        // This is a valid Dx state so return a good status.
                        rc = TRUE;
                        }
 
                    DEBUGMSG(ZONE_INFO, (L"KPG: IOCTL_POWER_QUERY %d\r\n"));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
                }
            break;

        // requests a change from one device power state to another
        case IOCTL_POWER_SET: 
            DEBUGMSG(ZONE_INFO,(L"KPG: Received IOCTL_POWER_SET\r\n"));
            if (pOutBuffer && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuffer;
 
                    if (SetPowerState(pDevice, ReqDx))
                        {   
                        *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->powerState;
                        *pOutSize = sizeof(CEDEVICE_POWER_STATE);
 
                        rc = TRUE;
                        DEBUGMSG(ZONE_INFO, (L"KPG: "
                            L"IOCTL_POWER_SET to D%u \r\n",
                            pDevice->powerState
                            ));
                        }
                    else 
                        {
                        RETAILMSG(ZONE_ERROR, (L"KPG: "
                            L"Invalid state request D%u \r\n", ReqDx
                            ));
                        }
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
            }
            break;

        // gets the current device power state
        case IOCTL_POWER_GET: 
            RETAILMSG(ZONE_INFO, (L"KPG: Received IOCTL_POWER_GET\r\n"));
            if (pOutBuffer != NULL && outSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    *(PCEDEVICE_POWER_STATE)pOutBuffer = pDevice->powerState;
 
                    rc = TRUE;

                    DEBUGMSG(ZONE_INFO, (L"KPG: "
                            L"IOCTL_POWER_GET to D%u \r\n",
                            pDevice->powerState
                            ));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
                    RETAILMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
                    }
                }     
            break;
        }

cleanUp:
    DEBUGMSG(ZONE_FUNCTION, (L"-KPG_IOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  KPG_InterruptThread
//
//  This function acts as the IST for the keypad.
//
DWORD KPG_IntrThread(VOID *pContext)
{
    KeypadDevice_t *pDevice = (KeypadDevice_t*)pContext;
    KeypadRemapState_t *pRemapState = NULL;
    KeypadRepeatState_t *pRepeatState = NULL;
    UINT8 *pGpioInput=NULL;
    DWORD vkState[VK_KEYS/DWORD_BITS];
    DWORD vkNewState[VK_KEYS/DWORD_BITS];
    DWORD timeout;
    UINT16 ix;
    UINT16 pinInput, mask;

    // Init data
    memset(vkState, 0, sizeof(vkState));
    memset(vkNewState, 0, sizeof(vkNewState));

    // Initialize GPIO pin Input array
    if (g_GpioKeypadVK.count > 0)
        {
        // Allocate state structure for remap, zero initialized
        pGpioInput = LocalAlloc(
            LPTR, g_GpioKeypadVK.count * sizeof(UINT8)
            );
        if (pGpioInput == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPG_IntrThread: "
                L"Failed allocate memory for GPIO Input array\r\n"
                ));
            goto cleanUp;
            }
        }


    // Initialize remap informations
    if (g_keypadRemap.count > 0)
        {
        // Allocate state structure for remap, zero initialized
        pRemapState = LocalAlloc(
            LPTR, g_keypadRemap.count * sizeof(KeypadRemapState_t)
            );
        if (pRemapState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPG_IntrThread: "
                L"Failed allocate memory for virtual key remap\r\n"
                ));
            goto cleanUp;
            }
        }

    // Initialize repeat informations
    if (g_keypadRepeat.count > 0)
        {
        // Allocate state structure for repeat, zero initialized
        pRepeatState = LocalAlloc(
            LPTR, g_keypadRepeat.count * sizeof(KeypadRepeatState_t)
            );
        if (pRepeatState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPG_IntrThread: "
                L"Failed allocate memory for virtual key auto repeat\r\n"
                ));
            goto cleanUp;
            }
        }

    // Set delay to sample period
    timeout = pDevice->samplePeriod;

    // Loop until we are not stopped...
    while (!pDevice->intrThreadExit)
        {
        DWORD time;
        BOOL keyDown = FALSE;
        pinInput=0;
        mask=0;
        
        // Wait for event
        WaitForSingleObject(pDevice->hIntrEvent, timeout);
        if (pDevice->intrThreadExit) break;
        
        // Read Gpio pins
        for(ix = 0 ; ix < g_GpioKeypadVK.count ; ix++)
            {
            const KEYPAD_GPIO_VK_ITEM *pItem = &g_GpioKeypadVK.pItem[ix];
            UINT16 gpio = pItem->gpio;

            pGpioInput[ix] = (UINT8)GPIOGetBit(pDevice->hGpio, gpio);
            }

        // Convert physical state to virtual keys state
        PhysicalStateToVirtualState(pGpioInput, vkNewState, &keyDown);

        time = GetTickCount();

        // Remap multi virtual keys to final virtual key
        VirtualKeyRemap(time, &keyDown, pRemapState, vkNewState);
        PressedReleasedKeys(pDevice, vkState, vkNewState);
        AutoRepeat(pDevice, vkNewState, time, pRepeatState);

        //--------------------------------------------------------------
        // Prepare for next run
        //--------------------------------------------------------------

        // New state become old
        memcpy(vkState, vkNewState, sizeof(vkState));
        // Get new state for virtual key table
        memset(vkNewState, 0, sizeof(vkNewState));

        // Set timeout period depending on data state
        timeout = keyDown ? pDevice->samplePeriod : INFINITE;

        // Interrupt is done
        for(ix = 0 ; ix < g_GpioKeypadVK.count ; ix++)
            {
            GPIOInterruptDone(pDevice->hGpio, g_GpioKeypadVK.pItem[ix].gpio);
            }
        }

cleanUp:
    if ( pRemapState != NULL )
        {
        LocalFree(pRemapState);
        }

    if ( pRepeatState != NULL )
        {
        LocalFree(pRepeatState);
        }

    if ( pGpioInput != NULL )
        {
        LocalFree(pGpioInput);
        }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  PhysicalStateToVirtualState
//
//  Convert physical state to virtual keys state
//
static VOID
PhysicalStateToVirtualState(
    UINT8 *gpioInput,
    DWORD vkNewState[],
    BOOL *pKeyDown
    )
{
    BOOL keyDown = FALSE;
    ULONG ix;
    UINT8 vk;
   
    for(ix = 0 ; ix < g_GpioKeypadVK.count ; ix++)
        {
            const KEYPAD_GPIO_VK_ITEM *pItem = &g_GpioKeypadVK.pItem[ix];
            UINT8 vkey = pItem->vkey;

            if(gpioInput[ix] != 1)
                {
                // g_gpio_keypadVK is defined by the platform
                vk = vkey;
                vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                keyDown = TRUE;
                }
        }

    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  VirtualKeyRemap
//
//  This function remaps multiple virtual key to final virtual key.
//
static VOID
VirtualKeyRemap(
    DWORD time,
    BOOL *pKeyDown,
    KeypadRemapState_t *pRemapState,
    DWORD vkNewState[]
    )
{
    BOOL keyDown = FALSE;
    int ix;
    
    for (ix = 0; ix < g_keypadRemap.count; ix++)
        {
        const KEYPAD_REMAP_ITEM *pItem = &g_keypadRemap.pItem[ix];
        KeypadRemapState_t *pState = &pRemapState[ix];
        DWORD state = 0;
        USHORT down = 0;
        UINT8 vk = 0;

        // Count number of keys down & save down/up state
        int ik;
        for (ik = 0; ik < pItem->keys; ik++)
            {
            vk = pItem->pVKeys[ik];
            if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
                {
                state |= 1 << ik;
                down++;
                }
            }

        // Depending on number of keys down
        if (down >= pItem->keys && pItem->keys > 1)
            {
            // Clear all mapping keys
            for (ik = 0; ik < pItem->keys; ik++)
                {
                vk = pItem->pVKeys[ik];
                vkNewState[vk >> 5] &= ~(1 << (vk & 0x1F));
                }
            // All keys are down set final key
            vk = pItem->vkey;
            vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
            DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                L"Mapped vkey: 0x%x\r\n", vk
                ));
            // Clear remap pending flag
            pState->pending = FALSE;
            // Set remap processing flag
            pState->remapped = TRUE;
            }
        else if ( down > 0 )
            {
            // If already remapping or remapping is not pending
            // or pending time expired
            if  (  pState->remapped ||
                  !pState->pending  ||
                  (INT32)( time - pState->time ) < 0 )
                {
                // If we are not pending and not already remapping, start
                if (!pState->pending && !pState->remapped)
                    {
                    pState->pending = TRUE;
                    pState->time = time + pItem->delay;
                    }
                // Clear all mapping keys
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    vk = pItem->pVKeys[ik];
                    vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
                    }
                }
            else if (  pItem->keys == 1 &&
                      (INT32)( time - pState->time ) >= 0 )
                {
                // This is press and hold key
               DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                    L"Mapped press and hold vkey: 0x%x\r\n", vk
                    ));
                vk = pItem->vkey;
                vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                keyDown = TRUE;
                pState->pending = FALSE;
                }
            }
        else
            {
            // All keys are up, if remapping was pending set keys
            if ( pState->pending )
                {
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    if ( ( pState->state & ( 1 << ik ) ) != 0 )
                        {
                        vk = pItem->pVKeys[ik];
                        vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                        keyDown = TRUE;
                        }
                    }
                pState->pending = FALSE;
                }
            pState->remapped = FALSE;
            }
        // Save key state
        pState->state = state;
        }

    // Set output variable
    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  PressedReleasedKeys
//
//  Find keys pressed, by comparing old and new state.
//  Send key events for all changed keys.
//
static VOID
PressedReleasedKeys(
    KeypadDevice_t *pDevice,
    const DWORD vkState[],
    const DWORD vkNewState[]
    )
{
    UINT8 vk;
    int ic;

    for (ic = 0, vk = 0; ic < VK_KEYS/DWORD_BITS; ic++)
        {
        DWORD change = vkState[ic] ^ vkNewState[ic];
        if (change == 0)
            {
            vk += DWORD_BITS;
            }
        else
            {
            DWORD mask;
            for (mask = 1; mask != 0; mask <<= 1, vk++)
                {
                // Check for change
                if ((change & mask) != 0)
                    {
                    if ((vkNewState[ic] & mask) != 0)
                        {
                        DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                            L"Key Down: 0x%x\r\n", vk
                            )); 
                        // Send key down event
                        if (vk != VK_OFF)
                            {
                            SendKeyPadEvent(vk, 0, 0, 0);
                            }
                        }
                    else
                        {
                        DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                            L"Key Up: 0x%x\r\n", vk
                            ));

                        // Need to send the keydown as well as keyup for
                        // device to suspend under cebase.                          
                        if (pDevice->bEnableOffKey == TRUE && vk == VK_OFF)
                            {
                            SendKeyPadEvent(vk, 0, 0, 0);
                            }

                        // Send key down event
                        if (pDevice->bEnableOffKey != FALSE || vk != VK_OFF)
                            {
                            SendKeyPadEvent(vk, 0, KEYEVENTF_KEYUP, 0);
                            }

                        // send PowerPolicyNotify notification
                        switch (vk)
                            {
                            case VK_TPOWER:
                            case VK_OFF:
                                // only disable interrupts if we are about to enter
                                // a suspend state
                                RETAILMSG(ZONE_IST, 
                                    (L" PressedReleasedKeys: 0x%02X\r\n", 
                                    vk
                                    )); 
                                PowerPolicyNotify(PPN_SUSPENDKEYPRESSED, 0);
                                break;

                            case VK_APPS:
                            case VK_APP1:
                            case VK_APP2:
                            case VK_APP3:
                            case VK_APP4:
                            case VK_APP5:
                            case VK_APP6:                     
                                PowerPolicyNotify(PPN_APPBUTTONPRESSED, 0);
                                break;
                            }
                        
                        }
                    }
                }
            }
        }
}


//------------------------------------------------------------------------------
//
//  Function:  AutoRepeat
//
//  This function sends KeyEvents for auto-repeat keys.
//
static VOID
AutoRepeat(
    const KeypadDevice_t *pDevice,
    const DWORD vkNewState[],
    DWORD time,
    KeypadRepeatState_t *pRepeatState
    )
{
    ULONG ix;

    for (ix = 0; ix < g_keypadRepeat.count; ix++)
        {
        const KEYPAD_REPEAT_ITEM *pItem = &g_keypadRepeat.pItem[ix];
        KeypadRepeatState_t *pState = &pRepeatState[ix];
        DWORD delay;
        BOOL blockRepeat = FALSE;
        UINT8 vkBlock;
        UINT8 vk = pItem->vkey;

        if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
            {
            if (!pState->pending)
                {
                // Key was just pressed
                delay = pItem->firstDelay;
                if (delay == 0) delay = pDevice->firstRepeat;
                pState->time = time + delay;
                pState->pending = TRUE;
                pState->blocked = FALSE;
                }
            else if (((INT32)(time - pState->time)) >= 0)
                {
                // Check if any blocking keys are pressed
                const KEYPAD_REPEAT_BLOCK *pBlock = pItem->pBlock;
                if (pBlock != 0)
                    {
                    int ik;
                    for ( ik = 0; ik < pBlock->count; ik++ )
                        {
                        vkBlock = pBlock->pVKey[ik];
                        if ((vkNewState[vkBlock >> 5] &
                               (1 << (vkBlock & 0x1F))) != 0)
                            {
                            pState->blocked = TRUE;
                            DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                                L"Block repeat: 0x%x bcause of 0x%x\r\n",
                                vk, vkBlock
                                ));
                            break;
                        }
                    }
                }
                // Repeat if not blocked
                if (!pState->blocked)
                    {
                    DEBUGMSG(ZONE_IST, (L" KPG_IntrThread: "
                        L"Key Repeat: 0x%x\r\n", vk
                        ));
                    SendKeyPadEvent(vk, 0, pItem->silent ? KEYEVENTF_SILENT : 0, 0);
                    }
                // Set time for next repeat
                delay = pItem->nextDelay;
                if (delay == 0) delay = pDevice->nextRepeat;
                pState->time = time + delay;
                }
            }
        else
            {
            pState->pending = FALSE;
            pState->blocked = FALSE;
            }
        }
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

