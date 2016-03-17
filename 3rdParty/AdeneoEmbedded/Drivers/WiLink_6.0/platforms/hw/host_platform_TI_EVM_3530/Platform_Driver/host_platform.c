/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <gpio.h>
#include <ndis.h>
#include "ndis.h"               
#include "WlanDrvIf.h"
#include "osRgstry.h"
#include "osrgstry_extra.h"
#include "ParamOut.h"
#include "osEntry.h"
#include "public_types.h"
#include "osDebug.h"
#include "osTIType.h"

#include <bsp.h>
#include <omap35xx_config.h>
#include <omap35xx_base_regs.h>
#include "SdioAdapter.h"
#include "sdiodrvdbg.h"
#include "host_platform.h"

#define SDIO_ATTEMPT_LONGER_DELAY_WM  400

/* Enable GPIO Power line */
typedef struct {
	int GPIO_POWER_LINE;
	int GPIO_WL_EN;
} GPIO_PWR;
static GPIO_PWR gpio_pwr;

void Get_GPIOPWR_LINE(int POWER_LINE)
{
	gpio_pwr.GPIO_POWER_LINE = POWER_LINE;
}

static int OMAP3530_TNETW_Power(int power_on, int pwr)
{
    HANDLE hGPIO = GPIOOpen();

   // Power enable
 
   if(hGPIO == NULL) 
   {
        RETAILMSG(1, (L"OMAP3530_TNETW_Power() GPIOOpen FAILED\r\n"));
        return ERROR_1;
   }
    
    if(power_on)
    {
		if(pwr)
		{
			RETAILMSG(1, (L"OMAP3530_TNETW_Power: restart power\r\n"));

			GPIOClrBit(hGPIO, GPIO_137);
			GPIOClrBit(hGPIO, gpio_pwr.GPIO_WL_EN);

			GPIOClrBit(hGPIO, GPIO_95);
			GPIOClrBit(hGPIO, GPIO_136);
			StallExecution(100*1000);

			GPIOSetBit(hGPIO, GPIO_136);
			GPIOSetBit(hGPIO, GPIO_95);

			StallExecution(100*1000);
		}
		GPIOSetBit(hGPIO, gpio_pwr.GPIO_WL_EN);
		StallExecution(100*1000);
    }
    else
    {
        GPIOClrBit(hGPIO, gpio_pwr.GPIO_WL_EN);
    }
    GPIOClose(hGPIO);

	RETAILMSG(1, (L"OMAP3530_TNETW_Power: %d, %d\r\n", power_on, pwr));

    return SUCCESS;
    
}

/*-----------------------------------------------------------------------------

Routine Name:

        hPlatform_hardResetTnetw

Routine Description:

        set the GPIO to low after awaking the TNET from ELP.

Arguments:

        OsContext - our adapter context.


Return Value:

        None

-----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------*/

/* Turn device power off */
int hPlatform_DevicePowerOff (void)
{
    int err;
    
    err = OMAP3530_TNETW_Power(0, 0);
    
    StallExecution(10*1000);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power off according to a given delay */
int hPlatform_DevicePowerOffSetLongerDelay (void)
{
    int err;
    
    err = OMAP3530_TNETW_Power(0, 0);
    
    StallExecution(SDIO_ATTEMPT_LONGER_DELAY_WM*1000);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power on */
int hPlatform_DevicePowerOn (int pwr)
{
    int err;

    err = OMAP3530_TNETW_Power(1, pwr);

    /* Should not be changed, 50 msec cause failures */
    StallExecution(200*1000);

    return err;
}

/*--------------------------------------------------------------------------------------*/

int hPlatform_Wlan_Hardware_Init(void)
{
	// Vladimir
	// Power enable pin already initialized by XLDR
    return SUCCESS;
}

void hPlatform_Wlan_Hardware_DeInit(void)
{

}

BOOL hPlatform_Wlan_Intr_Enable(TWlanDrvIfObjPtr pAdapter, PVOID *InterruptVector)
{
	HANDLE  hGPIO;	
	NDIS_STRING IrqLine                     = NDIS_STRING_CONST( "IrqLine" );
	NDIS_STRING Wlan_PwrLine                = NDIS_STRING_CONST( "Wlan_PwrLine" );

	int IRQ_LINE;
	int GPIOPWR_LINE;

	// Configure Interrupt line
	regReadIntegerParameter(pAdapter, &IrqLine,
                            0, 0,
                            200,
                            sizeof (int), 
                            (TI_UINT8 *)&IRQ_LINE);

	regReadIntegerParameter(pAdapter, &Wlan_PwrLine,
                            0, 0,
                            200,
                            sizeof (int), 
                            (TI_UINT8 *)&GPIOPWR_LINE);

	gpio_pwr.GPIO_POWER_LINE = GPIOPWR_LINE;
	gpio_pwr.GPIO_WL_EN		 = GPIO_138;
    
	 
    hGPIO = GPIOOpen();
    if(hGPIO == NULL) 
    {
        RETAILMSG(1, (L"Error open GPIO handle.\r\n"));
        return TI_NOK;
    }

    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_OUTPUT);
    GPIOSetBit(hGPIO,  IRQ_LINE);
#ifndef USE_IRQ_ACTIVE_HIGH
    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_INPUT | GPIO_INT_HIGH_LOW);
#else
    GPIOSetMode(hGPIO, WLAN_GPIO_INTERRUPT_LINE, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);
#endif

	*InterruptVector = GPIOGetSystemIrq(hGPIO,IRQ_LINE);

	GPIOClose(hGPIO);

	return 1;
}