//===================================================================
//
//  Module Name:    NLED.DLL
//
//  File Name:      nleddrv.c
//
//  Description:    Control of the notification LED(s)
//
//===================================================================
// Copyright (c) 2007- 2009 BSQUARE Corporation. All rights reserved.
//===================================================================

//-----------------------------------------------------------------------------
//
// header files
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap35xx.h>
#include <nled.h>
#include <led_drvr.h>
//#include <pmpolicy.h>

#include <initguid.h>
#include <gpio.h>

//-----------------------------------------------------------------------------
//
// Local Macros
//
//-----------------------------------------------------------------------------

// EVM has a very dim LED connected to GPIO SYS_BOOT6/GPIO_8 pad, active low

#define ENABLE_DEBUG_MESSAGES                   TRUE

// number of NLEDs supported by this driver, must be 1..3
// Could change to get this data from registry...
#define NLEDS_NUMBER_LEDS                       1

// number of NLEDs reported by driver, allows hidden NLEDs
// Could change to get this data from registry...
#define NLEDS_NUMBER_LEDS_REPORTED              1

// data to initialize array of NLED GPIO IDs
// Could change to get this data from registry...
#define NLEDS_GPIO_ID_ARRAY_INIT                {8}

// data to initialize array of NLED active states, 1 => LED on when GPIO high, 0 => LED on when GPIO low
// Could change to get this data from registry...
#define NLEDS_GPIO_ACTIVE_STATE_ARRAY_INIT      {0}

// if TRUE then the last NLED will be marked as a vibrator
// Could change to get this data from registry...
#define NLED_DRIVER_SUPPORTS_VIBRATE            FALSE

#if NLEDS_NUMBER_LEDS <= 0
    #error "NLED Driver configured for no notification LEDs (NLEDS_NUMBER_LEDS)"
#endif

//-----------------------------------------------------------------------------
//
// private data
//
//-----------------------------------------------------------------------------

static HANDLE g_hGPIO = NULL;

// Array to hold current Blink Params
static volatile struct NLED_SETTINGS_INFO BlinkParams[NLEDS_NUMBER_LEDS];

// Array to hold current LED settings
static volatile int NLedCurrentState[NLEDS_NUMBER_LEDS];

// Array to hold event handles for waking each LED Thread
static HANDLE hLedHandle[NLEDS_NUMBER_LEDS];
static HANDLE hNewThread[NLEDS_NUMBER_LEDS];
static DWORD g_GPIOId[NLEDS_NUMBER_LEDS] = NLEDS_GPIO_ID_ARRAY_INIT;
static DWORD g_GPIOActiveState[NLEDS_NUMBER_LEDS] = NLEDS_GPIO_ACTIVE_STATE_ARRAY_INIT;
// flag to indicate thread should exit
// Shared by all threads
static BOOL g_bExitThread = FALSE;

// Helper function used to set state of NLED control bits
static void NLedDriverSetLedState(UINT LedNum, int LedState)
{
    if (LedNum >= NLEDS_NUMBER_LEDS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: NLedDriverSetLedState: invalid NLED number: %d\r\n"), LedNum));
        return;
    }

    switch (LedState)
    {
        case 0:
            // turn LED off 
            if (g_GPIOActiveState[LedNum])
                GPIOClrBit(g_hGPIO, g_GPIOId[LedNum]);
            else
                GPIOSetBit(g_hGPIO, g_GPIOId[LedNum]);
            break;

        case 1:
            // turn LED on
            if (g_GPIOActiveState[LedNum])
                GPIOSetBit(g_hGPIO, g_GPIOId[LedNum]);
            else
                GPIOClrBit(g_hGPIO, g_GPIOId[LedNum]);
            break;
    }
}

NLedControlThread(
    UINT    NLedID
    )
{
    int i;

    // turn NLED off
    NLedDriverSetLedState(NLedID, 0);

    while (1)
    {
        StateChangeDueToApiCall:

        if (g_bExitThread == TRUE)
            break;

        // check unchanged NLED state
        if ( NLedCurrentState[NLedID] == BlinkParams[NLedID].OffOnBlink )
        {
            // handle blinking state
            if ( NLedCurrentState[NLedID] == 2 )
            {
                // Do meta cycle on blinks (just do regular blink cycle if Meta is 0)
                //DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED%d MetaCycleOn %d periods\r\n"), NLedID, BlinkParams[NLedID].MetaCycleOn));
                for (i = 0; i < (BlinkParams[NLedID].MetaCycleOn == 0 ? 1 : BlinkParams[NLedID].MetaCycleOn); i++)
                {
                    // turn NLED on
                    NLedDriverSetLedState(NLedID, 1);

                    // wait for on time (or change from API call)
                    if ( WaitForSingleObject(hLedHandle[NLedID], (BlinkParams[NLedID].OnTime / 1000)) == WAIT_OBJECT_0 )
                        goto StateChangeDueToApiCall;

                    // turn NLED off
                    NLedDriverSetLedState(NLedID, 0);

                    // wait for off time (or change from API call)
                    if ( WaitForSingleObject(hLedHandle[NLedID], ((BlinkParams[NLedID].TotalCycleTime - BlinkParams[NLedID].OnTime) / 1000)) == WAIT_OBJECT_0 )
                        goto StateChangeDueToApiCall;
                }

                // check for meta off specified, wait for that time period (or change from API call)
                //DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED%d MetaCycleOff %d periods\r\n"), NLedID, BlinkParams[NLedID].MetaCycleOff));
                if ( BlinkParams[NLedID].MetaCycleOff > 0 )
                {
                    if ( WaitForSingleObject(hLedHandle[NLedID], (((BlinkParams[NLedID].OffTime + BlinkParams[NLedID].OnTime) / 1000) * BlinkParams[NLedID].MetaCycleOff)) == WAIT_OBJECT_0 )
                        goto StateChangeDueToApiCall;
                }
            }
            else
            {
                // LED state unchanged, do nothing for on or off state except wait for API change
                WaitForSingleObject(hLedHandle[NLedID], INFINITE);
                #if ENABLE_DEBUG_MESSAGES
                    DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d driver thread awakened:\r\n"), NLedID));
                #endif
            }
        }
        else
        {
            // LED state changed, update NLED for new state
            #if ENABLE_DEBUG_MESSAGES
                DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d driver thread: Mode change to %d\r\n"), NLedID, BlinkParams[NLedID].OffOnBlink));
            #endif
            if ( BlinkParams[NLedID].OffOnBlink == 0 )
            {
                NLedDriverSetLedState(NLedID, 0);
                NLedCurrentState[NLedID] = 0;
            }
            if ( BlinkParams[NLedID].OffOnBlink == 1 )
            {
                NLedDriverSetLedState(NLedID, 1);
                NLedCurrentState[NLedID] = 1;
            }
            if ( BlinkParams[NLedID].OffOnBlink == 2 )
                NLedCurrentState[NLedID] = 2;
        }
    }

    DEBUGMSG(ZONE_INIT, (TEXT("NLedControlThread exiting!!\r\n"), NLedID));

    return 0;
}


BOOL
WINAPI
NLedDriverGetDeviceInfo(
    INT     nInfoId,
    PVOID   pOutput
    )
{
    if ( nInfoId == NLED_COUNT_INFO_ID )
    {
        struct NLED_COUNT_INFO * p = (struct NLED_COUNT_INFO*)pOutput;

        if (p == NULL)
            goto ReturnError;

        // Fill in number of leds
        p->cLeds = NLEDS_NUMBER_LEDS_REPORTED;
        DEBUGMSG(ZONE_INIT, (TEXT("NLEDDRV: NLedDriverGetDeviceInfo(NLED_COUNT_INFO_ID...) returning %d NLEDs\n"), NLEDS_NUMBER_LEDS_REPORTED));
        return TRUE;
    }

    if ( nInfoId == NLED_SUPPORTS_INFO_ID )
    {
        struct NLED_SUPPORTS_INFO * p = (struct NLED_SUPPORTS_INFO *)pOutput;

        if (p == NULL)
            goto ReturnError;

        if ( p->LedNum >= NLEDS_NUMBER_LEDS )
            goto ReturnError;

        // Fill in LED capabilities
        p->lCycleAdjust = 1000;         // Granularity of cycle time adjustments (microseconds)
        p->fAdjustTotalCycleTime = TRUE;    // LED has an adjustable total cycle time
        p->fAdjustOnTime = TRUE;            // @FIELD   LED has separate on time
        p->fAdjustOffTime = TRUE;           // @FIELD   LED has separate off time
        p->fMetaCycleOn = TRUE;             // @FIELD   LED can do blink n, pause, blink n, ...
        p->fMetaCycleOff = TRUE;            // @FIELD   LED can do blink n, pause n, blink n, ...

        // override individual LED capabilities
        #if NLED_DRIVER_SUPPORTS_VIBRATE
            if (p->LedNum == (NLEDS_NUMBER_LEDS - 1))
            {
                // vibrate must be last NLED, reports special lCycleAdjust value
                p->lCycleAdjust = -1;               // Well that was obvious!
                p->fAdjustTotalCycleTime = FALSE;   // LED has an adjustable total cycle time
                p->fAdjustOnTime = FALSE;           // @FIELD   LED has separate on time
                p->fAdjustOffTime = FALSE;          // @FIELD   LED has separate off time
                p->fMetaCycleOn = FALSE;            // @FIELD   LED can do blink n, pause, blink n, ...
                p->fMetaCycleOff = FALSE;           // @FIELD   LED can do blink n, pause n, blink n, ...
            }
        #endif
        return TRUE;
    }
    else if ( nInfoId == NLED_SETTINGS_INFO_ID )
    {
        struct NLED_SETTINGS_INFO * p = (struct NLED_SETTINGS_INFO *)pOutput;

        if (p == NULL)
            goto ReturnError;

        if ( p->LedNum >= NLEDS_NUMBER_LEDS )
            goto ReturnError;

        // Fill in current LED settings

        // Get any individual current settings
        p->OffOnBlink = BlinkParams[p->LedNum].OffOnBlink;
        p->TotalCycleTime = BlinkParams[p->LedNum].TotalCycleTime;
        p->OnTime = BlinkParams[p->LedNum].OnTime;
        p->OffTime = BlinkParams[p->LedNum].OffTime;
        p->MetaCycleOn = BlinkParams[p->LedNum].MetaCycleOn;
        p->MetaCycleOff = BlinkParams[p->LedNum].MetaCycleOff;

        return TRUE;
    }

ReturnError:

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}


BOOL
WINAPI
NLedDriverSetDevice(
    INT     nInfoId,
    PVOID   pInput
    )
{
    struct NLED_SETTINGS_INFO * p = (struct NLED_SETTINGS_INFO *)pInput;

    if ( nInfoId == NLED_SETTINGS_INFO_ID )
    {
        if (pInput == NULL)
            goto ReturnError;

        if ( p->LedNum >= NLEDS_NUMBER_LEDS )
            goto ReturnError;

        // check for invalid parameters
        if ( p->OffOnBlink < 0 || p->OffOnBlink > 2 )
            goto ReturnError;

        // for blink state, check for valid times (why were integers used?)
        if ( p->OffOnBlink == 2 )
            if (p->TotalCycleTime < p->OnTime ||
                p->TotalCycleTime < p->OffTime ||
                p->MetaCycleOn < 0 ||
                p->MetaCycleOff < 0 ||
                p->TotalCycleTime < 0 ||
                p->OnTime < 0 ||
                p->OffTime < 0 ||
                p->TotalCycleTime < p->OnTime + p->OffTime
            )
                goto ReturnError;

        // check for any changed NLED settings
        if ( BlinkParams[p->LedNum].OffOnBlink != p->OffOnBlink ||
             BlinkParams[p->LedNum].TotalCycleTime != p->TotalCycleTime ||
             BlinkParams[p->LedNum].OnTime != p->OnTime ||
             BlinkParams[p->LedNum].OffTime != p->OffTime ||
             BlinkParams[p->LedNum].MetaCycleOn != p->MetaCycleOn ||
             BlinkParams[p->LedNum].MetaCycleOff != p->MetaCycleOff
        )
        {
            // Update NLED settings
            BlinkParams[p->LedNum].OffOnBlink = p->OffOnBlink;
            BlinkParams[p->LedNum].TotalCycleTime = p->TotalCycleTime;
            BlinkParams[p->LedNum].OnTime = p->OnTime;
            BlinkParams[p->LedNum].OffTime = p->OffTime;
            BlinkParams[p->LedNum].MetaCycleOn = p->MetaCycleOn;
            BlinkParams[p->LedNum].MetaCycleOff = p->MetaCycleOff;

            // wake up appropriate NLED thread
            #if ENABLE_DEBUG_MESSAGES
                DEBUGMSG(ZONE_FUNCTION, (TEXT("NLED # %d change signaled\r\n"), p->LedNum));
            #endif

            SetEvent(hLedHandle[p->LedNum]);
        }

        return TRUE;
    }

ReturnError:

    #if ENABLE_DEBUG_MESSAGES
        DEBUGMSG(ZONE_ERROR, (TEXT("NLED: NLedDriverSetDevice: NLED %x, Invalid parameter\r\n"), p->LedNum));
    #endif

    SetLastError(ERROR_INVALID_PARAMETER);

    return FALSE;
}


// Note: This function is called by the power handler, it must not make
// any system calls other than the few that are specifically allowed
// (such as SetInterruptEvent()).
VOID
NLedDriverPowerDown(
   BOOL power_down
   )
{
    UINT NledNum;

    if ( power_down )
    {
        // shut off all NLEDs
        for (NledNum = 0; NledNum < NLEDS_NUMBER_LEDS; NledNum++)
        {
            NLedDriverSetLedState(NledNum, 0);
            NLedCurrentState[NledNum] = 0;
        }
    }
    else
    {
        for (NledNum = 0; NledNum < NLEDS_NUMBER_LEDS; NledNum++)
        {
            // On Power Up (Resume) turn on any LEDs that should be "ON".
            //  the individual LED control threads will put the Blinking
            //  LEDs back in their proper pre suspend state while "OFF" LEDs
            //  will stay off.
            if ( BlinkParams[NledNum].OffOnBlink == 1 )
            {
                NLedDriverSetLedState(NledNum, 1);
                NLedCurrentState[NledNum] = 1;
            }
        }
    }
}


BOOL
NLedDriverInitialize(
    VOID
    )
{
    BOOL bResult = TRUE;

    int i;

    // Open gpio driver
    g_hGPIO = GPIOOpen();
    if (g_hGPIO == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: NLedDriverInitialize: Failed to open Gpio driver!\r\n"));
        return FALSE;
    }

    // initialize the NLED state array and blink parameter structure
    for (i = 0; i < NLEDS_NUMBER_LEDS; i++)
    {
        BlinkParams[i].LedNum = i;
        BlinkParams[i].OffOnBlink = 0;
        BlinkParams[i].TotalCycleTime = 0;
        BlinkParams[i].OnTime = 0;
        BlinkParams[i].OffTime = 0;
        BlinkParams[i].MetaCycleOn = 0;
        BlinkParams[i].MetaCycleOff = 0;
        NLedCurrentState[i] = 0;
    }

    for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
    {
        hLedHandle[i] = NULL;
        hNewThread[i] = NULL;
    }

    // make NLED GPIO pins outputs
    for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
    {
        GPIOSetMode(g_hGPIO, g_GPIOId[i], GPIO_DIR_OUTPUT);
    }
        
    for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
    {
        hLedHandle[i] = CreateEvent(0, FALSE, FALSE, NULL);
        hNewThread[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)NLedControlThread, (LPVOID)i, 0, NULL);

        if ( hNewThread[i] == NULL || hLedHandle[i] == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: NLedDriverInitialize: Could not create event and/or start thread.\r\n")));
            bResult = FALSE;
        }
        else
        {
            DEBUGMSG(ZONE_INIT, (TEXT("NLedDriverInitialize: NLED # %d driver thread: (0x%X) started.\r\n"), i, hNewThread[i]));
        }
    }

    return bResult;
}

BOOL
NLedDriverDeInitialize(
    VOID
    )
{
    int i;

    DEBUGMSG(ZONE_INIT, (TEXT("NLEDDRV:  NLedDriverDeInitialize() Unloading driver...\r\n")));

    // Stop all threads, close handles
    g_bExitThread = TRUE;
    for ( i = 0; i < NLEDS_NUMBER_LEDS; i++ )
    {
        if (hLedHandle[i])
            SetEvent(hLedHandle[i]);

        if (hNewThread[i])
        {
            WaitForSingleObject(hNewThread[i], INFINITE);
            CloseHandle(hNewThread[i]);
        }

        if (hLedHandle[i])
            CloseHandle(hLedHandle[i]);

        hNewThread[i] = NULL;
        hLedHandle[i] = NULL;
    }

    GPIOClose(g_hGPIO);
    
    return TRUE;
}
