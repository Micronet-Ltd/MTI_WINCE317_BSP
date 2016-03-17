// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
// FILENAME: Touch.cpp
//
// DESCRIPTION:
//   Implementation of the WinCE touch screen PDD.
//  NOTE - This driver is written for the Texas Instruments TSC2046 touch screen controller
//
// Functions:
//
//  TouchDriverCalibrationPointGet
//  DdsiTouchPanelGetDeviceCaps
//  DdsiTouchPanelSetMode
//  DdsiTouchPanelEnable
//  DdsiTouchPanelDisable
//  DdsiTouchPanelAttach
//  DdsiTouchPanelDetach
//  DdsiTouchPanelGetPoint
//  DdsiTouchPanelPowerHandler
//
////////////////////////////////////////////////////////////////////////////////


//------------------------------------------------------------------------------
// Public
//
#include <windows.h>
#include <types.h>
#include <nkintr.h>
#include <creg.hxx>
#include <tchddsi.h>

//------------------------------------------------------------------------------
// Platform
//
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>

#include <initguid.h>
#include <gpio.h>
#include <spi.h>

//------------------------------------------------------------------------------
// Local
//
#include "touchscreen.h"


//------------------------------------------------------------------------------
// Defines
//
#define DEFAULT_DEVICE_ADDRESS          0 //CS0 of SPI1

// touch controller is used in differential reference mode, 12 bit conversion mode
#define COMMAND_EN_TOUCH                0x800000
#define COMMAND_XPOS                    0xD00000
#define COMMAND_YPOS                    0x900000
#define POWER_MASK                      0x010000
    
#define MAX_PTS         3
#define DELTA           30
    
#define CALC_DELTA(a, b) ((a) < (b) ? (b) - (a) : (a) - (b))

//------------------------------------------------------------------------------
// local data structures
//

typedef struct  
{
    BOOL    bInitialized;
    DWORD   nSampleRate;    
    DWORD   nInitialSamplesDropped;    
    LONG    nPenGPIO;    
    DWORD   nPenUpDebounceMS;
    LONG    nSPIAddr;
    LONG    nSPIBaudrate;

    LONG    nProcessAttached;
    HANDLE  hGPIO;
    HANDLE  hSPI;
    LONG    nPenIRQ;
}
TOUCH_DEVICE;

//------------------------------------------------------------------------------
//  Device registry parameters

static WCHAR const* s_szRegistryPath = L"\\HARDWARE\\DEVICEMAP\\TOUCH";

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = {
    {
        L"SampleRate", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nSampleRate),
        fieldsize(TOUCH_DEVICE, nSampleRate), (VOID*)DEFAULT_SAMPLE_RATE_HZ
    }, {
        L"PenGPIO", PARAM_DWORD, TRUE, offset(TOUCH_DEVICE, nPenGPIO),
        fieldsize(TOUCH_DEVICE, nPenGPIO), 0
    }, {
        L"PenUpDebounceMS", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nPenUpDebounceMS),
        fieldsize(TOUCH_DEVICE, nPenUpDebounceMS), (VOID*)40
    }, {
        L"SPIAddr", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nSPIAddr),
        fieldsize(TOUCH_DEVICE, nSPIAddr), 0
	}, {
        L"InitialSamplesDropped", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nInitialSamplesDropped),
        fieldsize(TOUCH_DEVICE, nInitialSamplesDropped), (VOID*)DEFAULT_INITIAL_SAMPLES_DROPPED
    }, {
        L"SPIBaudrate", PARAM_DWORD, FALSE, offset(TOUCH_DEVICE, nSPIBaudrate),
        fieldsize(TOUCH_DEVICE, nSPIBaudrate), (void*)0xA
    }
};
//48 MHz/1024(10) = ~46.8 kbps 46.8/16 = 2.928 KHz max sample rate
//48 MHz/256(8) = ~187 kbps 187/16 = 11.688 KHz max sample rate

//------------------------------------------------------------------------------
// global variables
//

static TOUCH_DEVICE s_TouchDevice =
    {
    FALSE,
    DEFAULT_SAMPLE_RATE_HZ,
    0,
    0,
    0
    };


// Referenced in MDD. TS controller only asserts SYSINTR_TOUCH.
//
DWORD gIntrTouchChanged = SYSINTR_NOP;   // Not used here.
DWORD gIntrTouch        = SYSINTR_NOP;

void StartCalibrationThread();

//------------------------------------------------------------------------------
// exports
//
extern "C" DWORD gdwTouchIstTimeout;    // MDD thread wait timeout.

// The MDD requires a minimum of MIN_CAL_COUNT consecutive samples before
// it will return a calibration coordinate to GWE.
extern "C" const int MIN_CAL_COUNT = 20;

//------------------------------------------------------------------------------
//
//  TouchDriverCalibrationPointGet
//
extern "C" BOOL 
TouchDriverCalibrationPointGet(
    TPDC_CALIBRATION_POINT *pTCP 
    )
{
    BOOL rc = FALSE;

    INT32 cDisplayWidth  = pTCP->cDisplayWidth;
    INT32 cDisplayHeight = pTCP->cDisplayHeight;

    int CalibrationRadiusX = cDisplayWidth / 20;
    int CalibrationRadiusY = cDisplayHeight / 20;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TouchDriverCalibrationPointGet+\r\n")));    
#if 1
    // Check which of the 5 calibration point is requested.
    switch ( pTCP->PointNumber )
        {
        case 0:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;
            rc = TRUE;
            break;

        case 1:
            pTCP->CalibrationX = CalibrationRadiusX * 2;
            pTCP->CalibrationY = CalibrationRadiusY * 2;
            rc = TRUE;
            break;

        case 2:
            pTCP->CalibrationX = CalibrationRadiusX * 2;
            pTCP->CalibrationY = cDisplayHeight - ( CalibrationRadiusY * 2 );
            rc = TRUE;
            break;

        case 3:
            pTCP->CalibrationX = cDisplayWidth - ( CalibrationRadiusX * 2 );
            pTCP->CalibrationY = cDisplayHeight - ( CalibrationRadiusY * 2 );
            rc = TRUE;
            break;

        case 4:
            pTCP->CalibrationX = cDisplayWidth - ( CalibrationRadiusX * 2 );
            pTCP->CalibrationY = CalibrationRadiusY * 2;
            rc = TRUE;
            break;

        default:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;

            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
#else
    switch ( pTCP->PointNumber )
        {
        case 0:
            pTCP->CalibrationX = cDisplayWidth / 4;
            pTCP->CalibrationY = cDisplayHeight / 4;
            rc = TRUE;
            break;

        case 1:
            pTCP->CalibrationX = cDisplayWidth / 4;
            pTCP->CalibrationY = cDisplayHeight / 2;
            rc = TRUE;
            break;

        case 2:
            pTCP->CalibrationX = cDisplayWidth / 4;
            pTCP->CalibrationY = 3 * cDisplayHeight / 4;
            rc = TRUE;
            break;

        case 3:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = 3*cDisplayHeight / 4;
            rc = TRUE;
            break;

        case 4:
            pTCP->CalibrationX = 3*cDisplayWidth / 4;
            pTCP->CalibrationY = 3*cDisplayHeight / 4;
            rc = TRUE;
            break;
		case 5:
            pTCP->CalibrationX = 3*cDisplayWidth / 4;
            pTCP->CalibrationY = cDisplayHeight / 2;
            rc = TRUE;
			break;
		case 6:
            pTCP->CalibrationX = 3*cDisplayWidth / 4;
            pTCP->CalibrationY = cDisplayHeight / 4;
            rc = TRUE;
			break;
		case 7:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 4;
            rc = TRUE;
			break;
		case 8:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;
            rc = TRUE;
			break;
        default:
            pTCP->CalibrationX = cDisplayWidth / 2;
            pTCP->CalibrationY = cDisplayHeight / 2;

            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }
#endif

    DEBUGMSG(ZONE_FUNCTION, (TEXT("cDisplayWidth        : %4X\r\n"), cDisplayWidth     ));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("cDisplayHeight       : %4X\r\n"), cDisplayHeight    ));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationRadiusX   : %4d\r\n"), CalibrationRadiusX));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationRadiusY   : %4d\r\n"), CalibrationRadiusY));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("pTCP -> PointNumber  : %4d\r\n"), pTCP->PointNumber));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("pTCP -> CalibrationX : %4d\r\n"), pTCP->CalibrationX));
    DEBUGMSG(ZONE_FUNCTION, (TEXT("pTCP -> CalibrationY : %4d\r\n"), pTCP->CalibrationY));

    DEBUGMSG(ZONE_FUNCTION, (TEXT("TouchDriverCalibrationPointGet-\r\n")));    
    return ( rc );
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelGetDeviceCaps
//
//

extern "C" BOOL 
DdsiTouchPanelGetDeviceCaps( 
    INT iIndex, 
    LPVOID lpOutput 
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelGetDeviceCaps+\r\n")));
 
    BOOL rc = FALSE;
    if ( lpOutput == NULL )
        {
        DEBUGMSG(ZONE_ERROR, (TEXT("TouchPanelGetDeviceCaps: Invalid parameter.\r\n")));
        SetLastError( ERROR_INVALID_PARAMETER );
        }
    else
        {
        TPDC_SAMPLE_RATE *pTSR = (TPDC_SAMPLE_RATE*)lpOutput;
        TPDC_CALIBRATION_POINT_COUNT *pTCPC = (TPDC_CALIBRATION_POINT_COUNT*)lpOutput;

        // Check which of the device capabilities are requested.
        switch ( iIndex )
            {
            // Return the sample rate.
            case TPDC_SAMPLE_RATE_ID:

                pTSR->SamplesPerSecondLow      = TOUCHPANEL_SAMPLE_RATE_LOW;
                pTSR->SamplesPerSecondHigh     = TOUCHPANEL_SAMPLE_RATE_HIGH;
                pTSR->CurrentSampleRateSetting = s_TouchDevice.nSampleRate;

                rc = TRUE;
                break;

            // Return the number of calibration points used to calibrate the touch screen.
            case TPDC_CALIBRATION_POINT_COUNT_ID:

                pTCPC->flags              = 0;
//                pTCPC->cCalibrationPoints = 9;
                pTCPC->cCalibrationPoints = 5;

                rc = TRUE;
                break;

            // Return the x and y coordinates of the requested calibration point.
            // The index of the calibration point is set in lpOutput->PointNumber.
            case TPDC_CALIBRATION_POINT_ID:
                rc = TouchDriverCalibrationPointGet( (TPDC_CALIBRATION_POINT*)lpOutput );
                break;

            default:
                DEBUGMSG( ZONE_ERROR, 
                           (TEXT("TouchPanelGetDeviceCaps: Invalid parameter.\r\n")));
                SetLastError(ERROR_INVALID_PARAMETER);
                break;
        }
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelGetDeviceCaps-\r\n")));

    return ( rc );
}


//------------------------------------------------------------------------------
//
//  DdsiTouchPanelSetMode
//
//  Comments:
//    Only one sample rate supported.
//

BOOL 
DdsiTouchPanelSetMode( 
    INT iIndex, 
    LPVOID lpInput 
    )
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelSetMode+\r\n")));
    
    switch ( iIndex )
        {
        case TPSM_SAMPLERATE_LOW_ID:
        case TPSM_SAMPLERATE_HIGH_ID:
            SetLastError( ERROR_SUCCESS );
            rc = TRUE;
            break;

        default:
            DEBUGMSG( ZONE_ERROR, 
                       (TEXT("DdsiTouchPanelSetMode: Invalid parameter.\r\n")));
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelSetMode-\r\n")));

    return rc;
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelEnable
//

BOOL 
DdsiTouchPanelEnable()
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelEnable+\r\n")));
    
    // Initialize once only
    if (s_TouchDevice.bInitialized)
        {
        rc = TRUE;
        goto cleanup;
        }

    // Initialize HW
    if (!PddInitializeHardware())
        {
        DEBUGMSG(
            ZONE_ERROR, 
            (TEXT("ERROR: TOUCH: Failed to initialize touch PDD.\r\n"))
            );
        goto cleanup;
        }
    
    // Get sysintr values from the OAL for PENIRQ interrupt
    if (!KernelIoControl( 
            IOCTL_HAL_REQUEST_SYSINTR, 
            &(s_TouchDevice.nPenIRQ), 
            sizeof(s_TouchDevice.nPenIRQ),
            &gIntrTouch, 
            sizeof(gIntrTouch), 
            NULL
            ) ) 
        {
        DEBUGMSG(
            ZONE_ERROR, 
            (TEXT("ERROR: TOUCH: Failed to request the touch sysintr.\r\n"))
            );

        gIntrTouch = SYSINTR_UNDEFINED;
        goto cleanup;
        }

    // Create a calibration thread. This thread will check to see if calibration is needed.
    StartCalibrationThread();

    //Done
    s_TouchDevice.bInitialized = TRUE;
    rc = TRUE;
    
cleanup:
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelEnable-\r\n")));
    return rc;
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelDisable
//
//

VOID 
DdsiTouchPanelDisable()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelDisable+\r\n")));

    // Close pen event and kill thread.
    PddDeinitializeHardware();
        
    // Release interrupt
    if (gIntrTouch != 0) 
        {
        KernelIoControl(
            IOCTL_HAL_RELEASE_SYSINTR, 
            &gIntrTouch, 
            sizeof(gIntrTouch), 
            NULL, 
            0, 
            NULL
            );
        }

    s_TouchDevice.bInitialized = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelDisable-\r\n")));
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelAttach
//
//
LONG 
DdsiTouchPanelAttach()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelAttach+\r\n")));

    // Increment the number of process attach calls.
    s_TouchDevice.nProcessAttached++;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelAttach-\r\n")));
        
    // Return the number.
    return ( s_TouchDevice.nProcessAttached );
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelDetach
//
//
LONG 
DdsiTouchPanelDetach()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelDetach+\r\n")));
    
    // Decrement the number of process attach calls.
    s_TouchDevice.nProcessAttached--;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelDetach\r\n")));
    
    // Return the number.
    return ( s_TouchDevice.nProcessAttached );
}

//------------------------------------------------------------------------------
//
//  DdsiTouchPanelGetPoint
//
//
void 
DdsiTouchPanelGetPoint( 
    TOUCH_PANEL_SAMPLE_FLAGS *pTipStateFlags,
    INT *pUncalX, 
    INT *pUncalY 
    )
{
    // MDD needs us to hold on to last valid sample and previous pen state.
    static USHORT usLastFilteredX       = 0;    // This holds the previous X sample
    static USHORT usLastFilteredY       = 0;    // This holds the previous Y sample
    static USHORT usSavedFilteredX      = 0;    // This holds the last reported X sample
    static USHORT usSavedFilteredY      = 0;    // This holds the last reported Y sample
    static BOOL bPrevReportedPenDown    = FALSE;
    static DWORD DelayedSampleCount     = 0;
    
    BOOL bReportedPenDown               = FALSE; // This indicates if we are reporting pen down to the mdd  
    BOOL bActualPenDown                 = FALSE; // This indicates if the pen is actually down, whether we report or not
    UINT32 xpos = 0;
    UINT32 ypos = 0;

    DEBUGMSG(ZONE_FUNCTION&&ZONE_SAMPLES, (TEXT("DdsiTouchPanelGetPoint+\r\n")));

    // By default, any sample returned will be ignored.
    *pTipStateFlags = TouchSampleIgnore;

    // Check if pen data are available If so, get the data.
    // Note that we always return data from the previous sample to avoid returning data nearest
    // the point where the pen is going up.  Data during light touches is not accurate and should
    // normally be rejected.  Without the ability to do pressure measurement, we are limited to 
    // rejecting points near the beginning and end of the pen down period.
    bActualPenDown = PddGetTouchIntPinState();
    if (bActualPenDown == TRUE)
    {
        PddGetTouchData(&xpos, &ypos);
        
        // Check if pen is still down to validate the data.
        bActualPenDown = PddGetTouchIntPinState();
        if (bActualPenDown == TRUE)
        {
            if (DelayedSampleCount > s_TouchDevice.nInitialSamplesDropped)
            {
                // Indicate pen down so we can return valid data.
                bReportedPenDown = TRUE;
            }
            else
            {
                DelayedSampleCount++;
            }
        }
    }

    // Check if we have valid data to report to the MDD.
    if (bReportedPenDown)
    {
        // Return the valid pen data.  Note that this data was actually obtained on the
        // previous sample
        *pUncalX = usLastFilteredX;
        *pUncalY = usLastFilteredY;
        
        // Save the data that we returned.  
        // This will also be returned on penup to help avoid returning data obtained during
        // a light press
        usSavedFilteredX = usLastFilteredX;
        usSavedFilteredY = usLastFilteredY;
        
        DEBUGMSG(ZONE_SAMPLES, ( TEXT( "X(0x%X) Y(0x%X)\r\n" ), *pUncalX, *pUncalY ) );
        
        // Store reported pen state.
        *pTipStateFlags = TouchSampleDownFlag | TouchSampleValidFlag;
    }
    else    // Otherwise, assume pen is up.
    {
        // Check if previously down and valid.
        if ( bPrevReportedPenDown )
        {
            DEBUGMSG(ZONE_TIPSTATE, ( TEXT( "Pen Up!\r\n" ) ) );

            // Use the last valid sample. MDD needs an up with valid data.
            *pUncalX = usSavedFilteredX;
            *pUncalY = usSavedFilteredY;
            *pTipStateFlags = TouchSampleValidFlag;
        }
        DEBUGMSG(ZONE_SAMPLES, ( TEXT( "Point: (%d,%d)\r\n" ), *pUncalX, *pUncalY ) );
    }
        
    // Save current reported pen state.
    bPrevReportedPenDown = bReportedPenDown;

    // Set up interrupt/timer for next sample
    if (bActualPenDown)
    {
        // Pen down so set MDD timeout for polling mode.
        gdwTouchIstTimeout = 1000 / s_TouchDevice.nSampleRate;
        // Save point measured during this sample so it can be reported on the next sample
        usLastFilteredX = xpos;
        usLastFilteredY = ypos;
    }
    else
    {
        // Reset the delayed sample counter
        DelayedSampleCount = 0;
        usLastFilteredX = 0;
        usLastFilteredY = 0;
        
        // Pen up so set MDD timeout for interrupt mode.
        gdwTouchIstTimeout = INFINITE;

        // Set the proper state for the next interrupt.
        InterruptDone( gIntrTouch );
    }

    DEBUGMSG(ZONE_FUNCTION&&ZONE_SAMPLES, (TEXT("DdsiTouchPanelGetPoint+\r\n")));
    
    RETAILMSG(0/**pTipStateFlags == TouchSampleValidFlag*/, (L"Touch: Point (%d, %d) %x\r\n", *pUncalX, *pUncalY, *pTipStateFlags));

    return;
}


//------------------------------------------------------------------------------
//
//  DdsiTouchPanelPowerHandler
//
//  Comments:
//    The controller powers down between samples.
//
//
void 
DdsiTouchPanelPowerHandler( 
    BOOL bOff 
    )
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelPowerHandler+\r\n")));

    if (gIntrTouch != SYSINTR_NOP)
        InterruptMask(gIntrTouch, bOff);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("DdsiTouchPanelPowerHandler+\r\n")));
}


        
//------------------------------------------------------------------------------
//
//  PddInitializeHardware()
//

BOOL 
PddInitializeHardware(VOID)
{
    BOOL   rc = FALSE; 
    DWORD  config;
    UINT32 spiBuffer;
    UINT32 xPos, yPos; 

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddInitializeHardware+\r\n")));
	RETAILMSG(0, (L"Touch:+%S\r\n",__FUNCTION__));

    // Read parameters from registry
    if (GetDeviceRegistryParams(
            s_szRegistryPath, 
            &s_TouchDevice, 
            dimof(s_deviceRegParams), 
            s_deviceRegParams) != ERROR_SUCCESS)
        {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PddInitializeHardware: Error reading from Registry.\r\n")));
        goto cleanup;
        }

    // Open GPIO driver
    s_TouchDevice.hGPIO = GPIOOpen();
    if (s_TouchDevice.hGPIO == NULL) 
        {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PddInitializeHardware: Failed open GPIO device driver\r\n")));
		RETAILMSG(1, (L"Touch:%S, PddInitializeHardware: Failed open GPIO device driver \r\n",__FUNCTION__));
        goto cleanup;
        }

    // Setup nPENIRQ for input mode, falling edge detect, debounce enable
    GPIOSetMode(s_TouchDevice.hGPIO, s_TouchDevice.nPenGPIO, GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW|GPIO_DEBOUNCE_ENABLE);
    // Get the IRQ for the GPIO
    s_TouchDevice.nPenIRQ = GPIOGetSystemIrq(s_TouchDevice.hGPIO, s_TouchDevice.nPenGPIO);

	RETAILMSG(0, (L"Touch: Pen IRQ GPIO (%d)\r\n", s_TouchDevice.nPenGPIO));
	RETAILMSG(0, (L"Touch:%S Pen IRQ GPIO (%d)\r\n",__FUNCTION__, s_TouchDevice.nPenGPIO));

    // Set debounce time on GPIO
    IOCTL_GPIO_SET_DEBOUNCE_TIME_IN debounce;
    
    debounce.gpioId = s_TouchDevice.nPenGPIO;
    debounce.debounceTime = 10;
    
    GPIOIoControl(s_TouchDevice.hGPIO, IOCTL_GPIO_SET_DEBOUNCE_TIME, (UCHAR*)&debounce, sizeof(debounce), NULL, 0, NULL, NULL);

    // Open SPI driver
    s_TouchDevice.hSPI = SPIOpen(SPI1_DEVICE_NAME);
    if (s_TouchDevice.hSPI == NULL) 
        {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PddInitializeHardware: Failed open SPI device driver\r\n")));
		RETAILMSG(1, (L"Touch:%S : Failed open SPI device driver\r\n",__FUNCTION__));
        goto cleanup;
        }
 
    //  Settings for Touchscreen device
    //
    //      Tx/Rx
    //      Tx enable on data line 0
    //
    
    // Raising clock rate can make the touch screen sample unstable, although the rate is in the nominal range.

    config =    MCSPI_PHA_ODD_EDGES |
                MCSPI_POL_ACTIVEHIGH |
                MCSPI_CHCONF_CLKD(s_TouchDevice.nSPIBaudrate) |
                MCSPI_CSPOLARITY_ACTIVELOW |
                MCSPI_CHCONF_WL(24) |
                MCSPI_CHCONF_TRM_TXRX |
                MCSPI_CHCONF_DPE0;

    // Configure SPI channel
	RETAILMSG(0, (L"Touch: SPI Addr (%d)\r\n", s_TouchDevice.nSPIAddr));
	RETAILMSG(0, (L"Touch:%S : SPI Addr (%d)\r\n",__FUNCTION__, s_TouchDevice.nSPIAddr));
    if (!SPIConfigure(s_TouchDevice.hSPI, s_TouchDevice.nSPIAddr, config))
        {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PddInitializeHardware: Failed configure SPI device driver\r\n")));
		RETAILMSG(1, (L"Touch:%S : Failed configure SPI device driver\r\n",__FUNCTION__));
        goto cleanup;
        }

	RETAILMSG(0, (L"Touch:%S : SPI device driver Ready\r\n",__FUNCTION__));
    // Enable the touchscreen
    spiBuffer = COMMAND_EN_TOUCH;
    if (!SPIWriteRead(s_TouchDevice.hSPI, sizeof(UINT32), &spiBuffer, &spiBuffer)) {
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: PAD_IOControl: failed to enable touchpad on spi bus\r\n")));
		RETAILMSG(1, (L"Touch:%S : PAD_IOControl: failed to enable touchpad on spi bus\r\n",__FUNCTION__));
        goto cleanup;
        }

	RETAILMSG(0, (L"Touch:%S : Touch enabled\r\n",__FUNCTION__));
    // Grab a point from touchscreen
    PddGetTouchData( &xPos, &yPos );
    
    // Test: read touch data when cable disconnected
#if 0
	 RETAILMSG(1, (L"Touch:%S read touch data for 1 sec\r\n",__FUNCTION__));

	 DWORD  dwTickCount = GetTickCount() + 1000;

	 while(GetTickCount() < dwTickCount)
	 {
      PddGetTouchData( &xPos, &yPos );
	  RETAILMSG(1, (L"[%d , %d] \r\n", xPos, yPos));
	 }

	 RETAILMSG(0, (L"Touch:-%S End reading touch data\r\n",__FUNCTION__));
#endif
	//-----------------------------------------------

    // Done    
    rc = TRUE;

cleanup:

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddInitializeHardware-\r\n")));    
    if( rc == FALSE )
        {
        PddDeinitializeHardware();
        }

	RETAILMSG(1, (L"Touch:-%S, rc = %d\r\n",__FUNCTION__, rc));
    return rc;
}



//------------------------------------------------------------------------------
//
//  PddDeinitializeHardware()
//
VOID 
PddDeinitializeHardware()
{
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddDeinitializeHardware+\r\n")));    

    // Close SPI device
    if (s_TouchDevice.hSPI != NULL)
        {
            SPIClose(s_TouchDevice.hSPI);
            s_TouchDevice.hSPI = NULL;
        }

    // Close GPIO device
    if (s_TouchDevice.hGPIO != NULL)
        {
            GPIOClose(s_TouchDevice.hGPIO);
            s_TouchDevice.hGPIO = NULL;
        }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("PddDeinitializeHardware-\r\n")));    
}


//------------------------------------------------------------------------------
//
//  PddGetTouchIntPinState()
//
inline BOOL 
PddGetTouchIntPinState()
{
    //  Return state of pen down
   return( GPIOGetBit(s_TouchDevice.hGPIO, s_TouchDevice.nPenGPIO) == 0 );
}

//------------------------------------------------------------------------------
//
//  PddGetControllerData 
//
void PddGetControllerData(UINT32 control, UINT32 * read_data)
{
    DWORD  config;

	RETAILMSG(0, (L"Touch:+%S\r\n",__FUNCTION__));

	config =    MCSPI_PHA_ODD_EDGES |
                MCSPI_POL_ACTIVEHIGH |
                MCSPI_CHCONF_CLKD(s_TouchDevice.nSPIBaudrate) |
                MCSPI_CSPOLARITY_ACTIVELOW |
                MCSPI_CHCONF_WL(24) |
                MCSPI_CHCONF_TRM_TXRX |
                MCSPI_CHCONF_DMAW_DISABLE |
                MCSPI_CHCONF_DMAR_DISABLE |
                MCSPI_CHCONF_DPE0;

    // Configure SPI channel
    if(SPIConfigure(s_TouchDevice.hSPI, s_TouchDevice.nSPIAddr, config))
	{
     RETAILMSG(0, (L"Touch:%S, SPI Configured Ok\r\n",__FUNCTION__));
    SPIWriteRead(s_TouchDevice.hSPI, 3, &control, read_data);
	}

  RETAILMSG(0, (L"Touch:-%S\r\n",__FUNCTION__));
}

//------------------------------------------------------------------------------
//
//  PddGetTouchData
//
BOOL 
PddGetTouchData(
    UINT32 * xpos,
    UINT32 * ypos
    )
{
    UINT32 tempdata = 0;
    UINT16 px, py;
    UINT32 pxAvg = 0, pyAvg = 0;
    int i;
    int ixAvg;
    int iyAvg;
	UINT32 samplesCount;

    DEBUGMSG(ZONE_FUNCTION, ( TEXT("PddGetTouchData+\r\n" )) );
	RETAILMSG(0, (L"Touch:+%S\r\n",__FUNCTION__));

	samplesCount = 0;

    do 
    {
        PddGetControllerData(COMMAND_XPOS, &tempdata);
        for( i = 0; i < MAX_PTS; i++ )
        {
            PddGetControllerData(COMMAND_XPOS | POWER_MASK, &tempdata);

            px = (tempdata & 0x7FF8) >> 3;

            if (i == 0)
            {
                pxAvg = px;
                ixAvg = 1;
            }
            else if (((pxAvg >= px)  &&  ((UINT16)(px+DELTA) >= pxAvg)) || ((px > pxAvg)  &&  ((UINT16)(pxAvg+DELTA) > px)))
            {
                pxAvg = (pxAvg +px)/2;
                ixAvg++;
            }
            else
                break;
        }

		samplesCount++;
		if( samplesCount >= DEFAULT_SAMPLE_RATE_HZ )
		{
         pxAvg = 0;
         pyAvg = 0;
         RETAILMSG(1, (L"Touch:%S, cannot calculate X\r\n",__FUNCTION__)); 
         break;
		}

        if (ixAvg < MAX_PTS)
             continue;

        PddGetControllerData(COMMAND_YPOS, &tempdata);
        for( i = 0; i < MAX_PTS; i++ )
        {

            PddGetControllerData(COMMAND_YPOS, &tempdata);
            
            py = (tempdata & 0x7FF8) >> 3;

            if (i == 0)
            {
                pyAvg = py;
                iyAvg = 1;
            }
            else if (((pyAvg >=py)  &&  ((UINT16)(py+DELTA) >=pyAvg)) || ((py > pyAvg)  &&  ((UINT16)(pyAvg+DELTA) > py)))
            {
                pyAvg = (pyAvg +py)/2;
                iyAvg++;
            }
            else
                break;
        }

		if( samplesCount >= DEFAULT_SAMPLE_RATE_HZ )
		{
         pyAvg = 0;
         RETAILMSG(1, (L"Touch:%S, cannot calculate Y\r\n",__FUNCTION__)); 
         break;
		}


    } while(ixAvg < MAX_PTS || iyAvg < MAX_PTS);

    *xpos = pxAvg;
    *ypos = pyAvg;
    
    // Possibly Pen Up, give pen status detection a chance to debounce
    if(pyAvg == 0 && s_TouchDevice.nPenUpDebounceMS) 
        Sleep(s_TouchDevice.nPenUpDebounceMS);

    DEBUGMSG(ZONE_FUNCTION, ( TEXT("PddGetTouchData-\r\n" )) );
	RETAILMSG(0, (L"Touch:-%S\r\n",__FUNCTION__));

    return TRUE;
}

//------------------------------------------------------------------------------

#define RK_HARDWARE_DEVICEMAP_TOUCH     (TEXT("HARDWARE\\DEVICEMAP\\TOUCH"))
#define RV_CALIBRATION_DATA             (TEXT("CalibrationData"))

static DWORD CalibrationThread()
{
    HKEY hKey;
    DWORD dwType;
    LONG lResult;
    HANDLE hAPIs;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread+\r\n")));    

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        DEBUGMSG(ZONE_CALIBRATE, (TEXT("touchp: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return 0;
    }

    // check for calibration data (query the type of data only)
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    RegCloseKey(hKey);
    if (lResult == ERROR_SUCCESS)
    {
        // registry contains calibration data, return
        return 1;
    }

    hAPIs = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("SYSTEM/GweApiSetReady"));
    if (hAPIs)
    {
        WaitForSingleObject(hAPIs, INFINITE);
        CloseHandle(hAPIs);
    }

    // Perform calibration
    TouchCalibrate();

    // try to open [HKLM\hardware\devicemap\touch] key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_HARDWARE_DEVICEMAP_TOUCH, 0, KEY_ALL_ACCESS, &hKey))
    {
        RETAILMSG(1, (TEXT("touchp: calibration: Can't find [HKLM/%s]\r\n"), RK_HARDWARE_DEVICEMAP_TOUCH));
        return 0;
    }

    // display new calibration data
    lResult = RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, NULL, NULL);
    if (lResult == ERROR_SUCCESS)
    {
        TCHAR szCalibrationData[100];
        DWORD Size = sizeof(szCalibrationData);

        RegQueryValueEx(hKey, RV_CALIBRATION_DATA, 0, &dwType, (BYTE *) szCalibrationData, (DWORD *) &Size);
        RETAILMSG(1, (TEXT("touchp: calibration: new calibration data is \"%s\"\r\n"), szCalibrationData));
    }
    RegCloseKey(hKey);
    
    DEBUGMSG(ZONE_FUNCTION, (TEXT("CalibrationThread-\r\n")));    

    return 1;
}

void StartCalibrationThread()
{
    HANDLE hThread;
    
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CalibrationThread, NULL, 0, NULL);
    // We don't need the handle, close it here
    CloseHandle(hThread);
}

