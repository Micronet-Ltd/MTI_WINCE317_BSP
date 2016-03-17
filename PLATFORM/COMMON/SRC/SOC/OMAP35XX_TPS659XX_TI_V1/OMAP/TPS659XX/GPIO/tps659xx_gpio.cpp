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
//  File: omap35xx_gpio.cpp
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <tps659xx.h>
#include <tps659xx_gpio.h>
#include <gpio.h>
#include <initguid.h>
#include <twl.h>


//------------------------------------------------------------------------------
//  Local Definitions

#define GPIO_DEVICE_COOKIE      'gioT'
#define MAX_GPIO_COUNT          (18)
#define GPIO_BITS_PER_BANK      (0x7)   // +1 last register only has 2 GPIO's

#define GPIO_BANK(x)            (x >> 3)
#define GPIO_BIT(x)             (x & GPIO_BITS_PER_BANK)
#define GPIO_SUBGROUP(x)        (GPIO_BIT(x) >> 2)
#define GPIO_SUBINDEX(x)        ((GPIO_BIT(x) & 0x3) * 2)

#define DETECT_RISING           (0x1)
#define DETECT_FALLING          (0x2)
#define DETECT_MASK             (0x3)

#define PULLDOWN_ENABLE         (1 << 0)
#define PULLUP_ENABLE           (1 << 1)


//------------------------------------------------------------------------------
//  Local Structures

static HANDLE                   s_hTritonDriver = NULL;
static CRITICAL_SECTION         s_cs;

//------------------------------------------------------------------------------
//  Device registry parameters

static TPS659XX_GPIO_DATA_REGS s_rgGpioRegs[] = {
    {
    TWL_GPIODATAIN1, TWL_GPIODATADIR1, TWL_GPIODATAOUT1,
    TWL_CLEARGPIODATAOUT1, TWL_SETGPIODATAOUT1, TWL_GPIO_DEBEN1,
        {{
        TWL_GPIOPUPDCTR1, TWL_GPIO_EDR1
        }, {
        TWL_GPIOPUPDCTR2, TWL_GPIO_EDR2
        }}
    }, {
    TWL_GPIODATAIN2, TWL_GPIODATADIR2, TWL_GPIODATAOUT2,
    TWL_CLEARGPIODATAOUT2, TWL_SETGPIODATAOUT2, TWL_GPIO_DEBEN2,
        {{
        TWL_GPIOPUPDCTR3, TWL_GPIO_EDR3
        }, {
        TWL_GPIOPUPDCTR4, TWL_GPIO_EDR4
        }}
    }, {
    TWL_GPIODATAIN3, TWL_GPIODATADIR3, TWL_GPIODATAOUT3,
    TWL_CLEARGPIODATAOUT3, TWL_SETGPIODATAOUT3, TWL_GPIO_DEBEN3,
        {{
        TWL_GPIOPUPDCTR5, TWL_GPIO_EDR5
        }, {
        0, 0
        }}
    }
};

//------------------------------------------------------------------------------
//  Local Functions

// Init function
static BOOL Tps659xxGpioInit(LPCTSTR szContext, HANDLE *phContext, UINT *pGpioCount);
static BOOL Tps659xxGpioDeinit(HANDLE hContext);
static BOOL Tps659xxGpioSetMode(HANDLE hContext, UINT id, UINT mode);
static BOOL Tps659xxGpioGetMode(HANDLE hContext, UINT id, UINT *pMode);
static BOOL Tps659xxGpioInterruptInitialize(HANDLE hcontext,  UINT intrID, HANDLE hEvent);
static BOOL Tps659xxGpioInterruptRelease(HANDLE hcontext,  UINT intrID);
static BOOL Tps659xxGpioInterruptDone(HANDLE hcontext,  UINT intrID);
static BOOL Tps659xxGpioInterruptDisable(HANDLE hcontext,  UINT intrID);
static BOOL Tps659xxGpioInterruptMask(HANDLE hcontext,  UINT intrID, BOOL bEnable);
static BOOL Tps659xxGpioWakeEnable(HANDLE hcontext,  UINT intrID, BOOL bEnable);
static BOOL Tps659xxGpioSetBit(HANDLE hContext, UINT id);
static BOOL Tps659xxGpioClrBit(HANDLE hContext, UINT id);
static BOOL Tps659xxGpioGetBit(HANDLE hContext, UINT id, UINT *pValue);
static void Tps659xxGpioPowerUp(HANDLE hContext);
static void Tps659xxGpioPowerDown(HANDLE hContext);
static BOOL Tps659xxGpioIoControl(HANDLE hContext, UINT code,
                               UCHAR *pinVal, UINT inSize, UCHAR *poutVal,
                               UINT outSize, UINT *pOutSize);
static BOOL Tps659xxGpioPullup(HANDLE hcontext, UINT id, UINT enable);
static BOOL Tps659xxGpioPulldown(HANDLE hcontext,  UINT id, UINT enable);

//------------------------------------------------------------------------------
//  exported function table
GPIO_TABLE Tps659xx_Gpio = {
    Tps659xxGpioInit,
    Tps659xxGpioDeinit,
    Tps659xxGpioSetMode,
    Tps659xxGpioGetMode,
    Tps659xxGpioInterruptInitialize,
    Tps659xxGpioInterruptRelease,
    Tps659xxGpioInterruptDone,
    Tps659xxGpioInterruptDisable,
    Tps659xxGpioInterruptMask,
    Tps659xxGpioWakeEnable,
    Tps659xxGpioSetBit,
    Tps659xxGpioClrBit,
    Tps659xxGpioGetBit,
    Tps659xxGpioPowerUp,
    Tps659xxGpioPowerDown,
    Tps659xxGpioIoControl,
    Tps659xxGpioPullup,
    Tps659xxGpioPulldown
};

static DWORD    g_dwGPIOIntrMap[] = {TWL_INTR_GPIO_0,  TWL_INTR_GPIO_1,  TWL_INTR_GPIO_2,
                                     TWL_INTR_GPIO_3,  TWL_INTR_GPIO_4,  TWL_INTR_GPIO_5,
                                     TWL_INTR_GPIO_6,  TWL_INTR_GPIO_7,  TWL_INTR_GPIO_8,
                                     TWL_INTR_GPIO_9,  TWL_INTR_GPIO_10, TWL_INTR_GPIO_11,
                                     TWL_INTR_GPIO_12, TWL_INTR_GPIO_13, TWL_INTR_GPIO_14,
                                     TWL_INTR_GPIO_15, TWL_INTR_GPIO_16, TWL_INTR_GPIO_17};

//------------------------------------------------------------------------------
static
BOOL
OpenTwl(
    )
{
    // Try open TWL device driver
    s_hTritonDriver = TWLOpen();

    // If we get handle, we succeded
    return (s_hTritonDriver != NULL);
}

//------------------------------------------------------------------------------
static
VOID
CloseTwl(
    )
{
    if (s_hTritonDriver != NULL)
        {
        TWLClose(s_hTritonDriver);
        s_hTritonDriver = NULL;
        }
}

//------------------------------------------------------------------------------
static
BOOL
WriteTwlReg(
    DWORD           address,
    UINT8          *pdata,
    UINT            dataSize
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    // Call driver
    rc = TWLWriteRegs(s_hTritonDriver, address, pdata, dataSize);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
static
BOOL
ReadTwlReg(
    DWORD   address,
    UINT8  *pdata,
    UINT    dataSize
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    // Call driver
    rc = TWLReadRegs(s_hTritonDriver, address, pdata, dataSize);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
inline
void
Lock()
{
    EnterCriticalSection(&s_cs);
}

//------------------------------------------------------------------------------
inline
void
Unlock()
{
    LeaveCriticalSection(&s_cs);
}

//------------------------------------------------------------------------------
static
void
SetGpioBankPowerState(
    UINT id,
    CEDEVICE_POWER_STATE state
    )
{
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
//  Function:  Tps659xxGpioInit
//
//  Called by device manager to initialize device.
//
BOOL
Tps659xxGpioInit(
    LPCTSTR szContext,
    HANDLE *phContext,
    UINT   *pGpioCount
    )
{
    DEBUGMSG(ZONE_FUNCTION, (
        L"+Tps659xxGpioInit(%s)\r\n", szContext
        ));

    *pGpioCount = MAX_GPIO_COUNT;

    InitializeCriticalSection(&s_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"-Tps659xxGpioInit()\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  Tps659xxGpioDeinit
//
//  Called by device manager to uninitialize device.
//
BOOL
Tps659xxGpioDeinit(
    HANDLE context
    )
{
    DEBUGMSG(ZONE_FUNCTION, (L"+Tps659xxGpioDeinit(0x%08x)\r\n", context));

    CloseTwl();
    DeleteCriticalSection(&s_cs);

    DEBUGMSG(ZONE_FUNCTION, (L"-Tps659xxGpioDeinit()\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioSetMode
//
BOOL
Tps659xxGpioSetMode(
    HANDLE context,
    UINT id,
    UINT mode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT subGroup = GPIO_SUBGROUP(id);
    UINT subIndex = GPIO_SUBINDEX(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;
        UINT8 edgeMode = 0;

        Lock();
        SetGpioBankPowerState(id, D0);

        // set direction
        if ((mode & GPIO_DIR_INPUT) != 0)
            {
            ReadTwlReg(s_rgGpioRegs[bank].GPIODATADIR, &val, sizeof(val));
            val &= ~(1 << bit);
            WriteTwlReg(s_rgGpioRegs[bank].GPIODATADIR, &val, sizeof(val));
            }
        else
            {
            ReadTwlReg(s_rgGpioRegs[bank].GPIODATADIR, &val, sizeof(val));
            val |= (1 << bit);
            WriteTwlReg(s_rgGpioRegs[bank].GPIODATADIR, &val, sizeof(val));
            }

        // enable debouncing
        if ((mode & GPIO_DEBOUNCE_ENABLE) != 0)
            {
            ReadTwlReg(s_rgGpioRegs[bank].GPIO_DEBEN, &val, sizeof(val));
            val |= (1 << bit);
            WriteTwlReg(s_rgGpioRegs[bank].GPIO_DEBEN, &val, sizeof(val));
            }
        else
            {
            ReadTwlReg(s_rgGpioRegs[bank].GPIO_DEBEN, &val, sizeof(val));
            val &= ~(1 << bit);
            WriteTwlReg(s_rgGpioRegs[bank].GPIO_DEBEN, &val, sizeof(val));
            }

        // set edge interrupt type
        if (mode & GPIO_INT_HIGH_LOW) edgeMode |= DETECT_FALLING;
        if (mode & GPIO_INT_LOW_HIGH) edgeMode |= DETECT_RISING;

        ReadTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIO_EDR, &val, sizeof(val));
        val &= ~(DETECT_MASK << subIndex);
        val |= (edgeMode << subIndex);
        WriteTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIO_EDR, &val, sizeof(val));

        SetGpioBankPowerState(id, D4);
        Unlock();
        rc = TRUE;
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioGetMode
//
BOOL
Tps659xxGpioGetMode(
    HANDLE context,
    UINT id,
    UINT *pMode
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT subGroup = GPIO_SUBGROUP(id);
    UINT subIndex = GPIO_SUBINDEX(id);

    *pMode = 0;
    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;
        SetGpioBankPowerState(id, D0);

        // get direction
        ReadTwlReg(s_rgGpioRegs[bank].GPIODATADIR, &val, sizeof(val));
        *pMode = (val & (1 << bit)) ? GPIO_DIR_OUTPUT : GPIO_DIR_INPUT;

        // get debounce state
        ReadTwlReg(s_rgGpioRegs[bank].GPIO_DEBEN, &val, sizeof(val));
        *pMode |= (val & (1 << bit)) ? GPIO_DEBOUNCE_ENABLE : 0;

        // get edge detection mode
        ReadTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIO_EDR, &val, sizeof(val));
        val = (val >> subIndex) & DETECT_MASK;

        if (val & DETECT_FALLING) *pMode |= GPIO_INT_HIGH_LOW;
        if (val & DETECT_RISING) *pMode |= GPIO_INT_LOW_HIGH;

        SetGpioBankPowerState(id, D4);
        rc = TRUE;
        }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioInterruptInitialize - Sets the interrupt event
//
BOOL
Tps659xxGpioInterruptInitialize(
    HANDLE context,
    UINT id,
    HANDLE hEvent
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    if (id >= MAX_GPIO_COUNT) 
        {
        RETAILMSG(1, (TEXT("Tps659xxGpioInterruptInitialize: INVALID GPIO ID (%d)\r\n"), id));
        goto cleanUp;
        }

    // Call driver
    rc = TWLInterruptInitialize(s_hTritonDriver, g_dwGPIOIntrMap[id], hEvent);

    if ((rc == TRUE) && (hEvent != INVALID_HANDLE_VALUE))
        {
        rc = TWLInterruptMask(s_hTritonDriver, g_dwGPIOIntrMap[id], FALSE);
        }

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioInterruptRelease - Release TWL interrupt
//
BOOL
Tps659xxGpioInterruptRelease(
    HANDLE context,
    UINT id)
{
    BOOL rc = FALSE;

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioInterruptDone - Reanble TWL interrupt
//
BOOL
Tps659xxGpioInterruptDone(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    if (id >= MAX_GPIO_COUNT) 
        {
        RETAILMSG(1, (TEXT("Tps659xxGpioInterruptDone: INVALID GPIO ID (%d)\r\n"), id));
        goto cleanUp;
        }

    // Call driver
    rc = TWLInterruptMask(s_hTritonDriver, g_dwGPIOIntrMap[id], FALSE);

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioInterruptDone - Disanble TWL interrupt
//
BOOL
Tps659xxGpioInterruptDisable(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    if (id >= MAX_GPIO_COUNT) 
        {
        RETAILMSG(1, (TEXT("Tps659xxGpioInterruptDisable: INVALID GPIO ID (%d)\r\n"), id));
        goto cleanUp;
        }

    rc = TWLInterruptMask(s_hTritonDriver, g_dwGPIOIntrMap[id], TRUE);

    if (rc == TRUE)
        {
        // Call driver
        rc = TWLInterruptDisable(s_hTritonDriver, g_dwGPIOIntrMap[id]);
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioInterruptDone - Disanble TWL interrupt
//
BOOL
Tps659xxGpioInterruptMask(
    HANDLE context,
    UINT id,
    BOOL bEnable
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    if (id >= MAX_GPIO_COUNT) 
        {
        RETAILMSG(1, (TEXT("Tps659xxGpioInterruptMask: INVALID GPIO ID (%d)\r\n"), id));
        goto cleanUp;
        }

    // Call driver
    rc = TWLInterruptMask(s_hTritonDriver, g_dwGPIOIntrMap[id], bEnable);

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioWakeEnable - Enable GPIO pin as a wake up 
//                interrupt pin
//
BOOL
Tps659xxGpioWakeEnable(
    HANDLE context,
    UINT id,
    BOOL bEnable
    )
{
    BOOL rc = FALSE;

    // If TWL isn't open, try to open it...
    if ((s_hTritonDriver == NULL) && !OpenTwl()) goto cleanUp;

    if (id >= MAX_GPIO_COUNT) 
        {
        RETAILMSG(1, (TEXT("Tps659xxGpioInterruptDisable: INVALID GPIO ID (%d)\r\n"), id));
        goto cleanUp;
        }

    // Call driver
    rc = TWLWakeEnable(s_hTritonDriver, g_dwGPIOIntrMap[id], bEnable);

cleanUp:
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioSetBit - Set the value of the GPIO output pin
//
BOOL
Tps659xxGpioSetBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;

        SetGpioBankPowerState(id, D0);
        val = (1 << bit);
        WriteTwlReg(s_rgGpioRegs[bank].SETGPIODATAOUT, &val, sizeof(val));
        SetGpioBankPowerState(id, D4);
        rc = TRUE;
        }

    return rc;
}




//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioClrBit
//
BOOL
Tps659xxGpioClrBit(
    HANDLE context,
    UINT id
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;

        SetGpioBankPowerState(id, D0);
        val = (1 << bit);
        WriteTwlReg(s_rgGpioRegs[bank].CLEARGPIODATAOUT, &val, sizeof(val));
        SetGpioBankPowerState(id, D4);
        rc = TRUE;
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioGetBit
//
BOOL
Tps659xxGpioGetBit(
    HANDLE context,
    UINT id,
    UINT *pValue
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;
        SetGpioBankPowerState(id, D0);
        ReadTwlReg(s_rgGpioRegs[bank].GPIODATAIN, &val, sizeof(val));
        *pValue = !!(val & (1 << bit));
        SetGpioBankPowerState(id, D4);
        rc = TRUE;
        }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  Tps659xxGpioIoControl
//
//  This function sends a command to a device.
//
BOOL
Tps659xxGpioIoControl(
    HANDLE  context,
    UINT    code,
    UCHAR  *pInBuffer,
    UINT    inSize,
    UCHAR  *pOutBuffer,
    UINT    outSize,
    UINT   *pOutSize
    )
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_FUNCTION, (
        L"+Tps659xxGpioIOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",
        context, code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize
        ));

    DEBUGMSG(ZONE_FUNCTION, (L"-Tps659xxGpioIOControl(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  Tps659xxGpioPowerUp
//
//  This function restores power to a device.
//
VOID
Tps659xxGpioPowerUp(
    HANDLE context
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  Tps659xxGpioPowerDown
//
//  This function suspends power to the device.
//
VOID
Tps659xxGpioPowerDown(
    HANDLE context
    )
{
}

//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioPullup - Pullup enable/disable
//
BOOL
Tps659xxGpioPullup(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT subGroup = GPIO_SUBGROUP(id);
    UINT subIndex = GPIO_SUBINDEX(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;
        UINT pullupState = 0;

        Lock();
        SetGpioBankPowerState(id, D0);

        // set pullup state
        if (enable) 
		    pullupState = PULLUP_ENABLE;

        ReadTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIOPUPDCTR, &val, sizeof(val));
        val &= ~(PULLUP_ENABLE << subIndex);
        val |= (pullupState << subIndex);
        WriteTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIOPUPDCTR, &val, sizeof(val));

        SetGpioBankPowerState(id, D4);
        Unlock();
        rc = TRUE;
        }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: Tps659xxGpioPulldown - Pulldown enable/disable
//
BOOL
Tps659xxGpioPulldown(
    HANDLE context,
    UINT id,
    UINT enable
    )
{
    BOOL rc = FALSE;
    UINT bit = GPIO_BIT(id);
    UINT bank = GPIO_BANK(id);
    UINT subGroup = GPIO_SUBGROUP(id);
    UINT subIndex = GPIO_SUBINDEX(id);

    if (id < MAX_GPIO_COUNT)
        {
        UINT8 val;
        UINT pulldownState = 0;

        Lock();
        SetGpioBankPowerState(id, D0);

        // set pullup state
        if (enable) 
		    pulldownState = PULLDOWN_ENABLE;

        ReadTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIOPUPDCTR, &val, sizeof(val));
        val &= ~(PULLDOWN_ENABLE << subIndex);
        val |= (pulldownState << subIndex);
        WriteTwlReg(s_rgGpioRegs[bank].rgSubGroup[subGroup].GPIOPUPDCTR, &val, sizeof(val));

        SetGpioBankPowerState(id, D4);
        Unlock();
        rc = TRUE;
        }

    return rc;
}

//------------------------------------------------------------------------------

