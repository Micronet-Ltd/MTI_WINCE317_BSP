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
#include <sdk_gpio.h>
#include <ndis.h>
#include "ndis.h"               
#include "WlanDrvIf.h"
#include "osRgstry.h"
/*#include "osRgstry_extra.h"*/
#include "osrgstry_extra.h"
#include "ParamOut.h"
#include "osEntry.h"
#include "public_types.h"
#include "osDebug.h"
#include "osTIType.h"

#include "omap_types.h"
#include <omap3530_config.h>
#include <omap3530_base_regs.h>
#include "SdioAdapter.h"
#include "sdiodrvdbg.h"
#include "host_platform.h"


#define OS_API_MEM_ADRR  	0x0000000
#define OS_API_REG_ADRR  	0x300000
#define SDIO_ATTEMPT_LONGER_DELAY_WM  400

/* Enable GPIO Power line */
typedef struct {
	int GPIO_POWER_LINE;
} GPIO_PWR;
static GPIO_PWR gpio_pwr;

void Get_GPIOPWR_LINE(int POWER_LINE)
{
	gpio_pwr.GPIO_POWER_LINE = POWER_LINE;
}
static int OMAP3530_TNETW_Power(int power_on)
{
    HANDLE hGPIO = GPIOOpen();

   /* Power enable */
 
   if(hGPIO == NULL) 
   {
        printf("OMAP3530_TNETW_Power() GPIOOpen FAILED\n");
        return ERROR_1;
   }
    
    GPIOSetMode(hGPIO, gpio_pwr.GPIO_POWER_LINE, GPIO_DIR_OUTPUT);
    if (power_on)
    {
        GPIOSetBit(hGPIO, gpio_pwr.GPIO_POWER_LINE);
    }
    else
    {
        GPIOClrBit(hGPIO, gpio_pwr.GPIO_POWER_LINE);
    }
    GPIOClose(hGPIO);

    return SUCCESS;
    
} /* OMAP3530_TNETW_Power() */

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
    
    err = OMAP3530_TNETW_Power(0);
    
    Sleep(10);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power off according to a given delay */
int hPlatform_DevicePowerOffSetLongerDelay (void)
{
    int err;
    
    err = OMAP3530_TNETW_Power(0);
    
    Sleep(SDIO_ATTEMPT_LONGER_DELAY_WM);
    
    return err;
}

/*--------------------------------------------------------------------------------------*/

/* Turn device power on */
int hPlatform_DevicePowerOn (void)
{
    int err;

    err = OMAP3530_TNETW_Power(1);

    /* Should not be changed, 50 msec cause failures */
    Sleep(70);

    return err;
}

/*--------------------------------------------------------------------------------------*/

int hPlatform_Wlan_Hardware_Init(void)
{

    PHYSICAL_ADDRESS PhysicalAddress;
    OMAP_SYSC_PADCONFS_REGS    *pConfig;

	PhysicalAddress.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
	pConfig = (OMAP_SYSC_PADCONFS_REGS*) MmMapIoSpace(PhysicalAddress, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

	if (pConfig == NULL)
    {
        printf("TI_STATUS InitializeInterrupt() MmMapIoSpace FAILED to map CONTROL_MMC2_CMD_PAD_PA !!!\n");
        return -1;
    }

    /* set GPIO 150 pin to output direction */
    OUTREG16((unsigned short*)&pConfig->CONTROL_PADCONF_UART1_CTS, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));    /*UART1_RTS is CB_HOST_WL_IRQ*/
	MmUnmapIoSpace ((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));

    return SUCCESS;

} /* hPlatform_Wlan_Hardware_Init() */

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
    PHYSICAL_ADDRESS PhysicalAddress;
    OMAP_SYSC_PADCONFS_REGS    *pConfig = NULL;

	PhysicalAddress.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;
	pConfig = (OMAP_SYSC_PADCONFS_REGS*) MmMapIoSpace(PhysicalAddress, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

	if (pConfig == NULL)
    {
        DEBUGMSG(DBG_INIT_FATAL_ERROR,(L"TI_STATUS InitializeInterrupt() MmMapIoSpace FAILED to map CONTROL_MMC2_CMD_PAD_PA !!!\n"));
        return TI_NOK;
    }

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
    
	 
    hGPIO = GPIOOpen();
    if(hGPIO == NULL) 
    {
        PRINTF(DBG_INIT_FATAL_ERROR,("Error open GPIO handle.\n"));
        return TI_NOK;
    }

     /* set GPIO 149 pin to input direction */
    OUTREG16((unsigned short*)&pConfig->CONTROL_PADCONF_UART1_RTS, (INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_4));    /*UART1_RTS is CB_HOST_WL_IRQ*/
	MmUnmapIoSpace ((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));


    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_OUTPUT);
    GPIOSetBit(hGPIO,  IRQ_LINE);
#ifdef FPGA1273_STAGE_
    GPIOSetMode(hGPIO, WLAN_GPIO_INTERRUPT_LINE, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);
#else
#ifndef USE_IRQ_ACTIVE_HIGH
    GPIOSetMode(hGPIO, IRQ_LINE, GPIO_DIR_INPUT | GPIO_INT_HIGH_LOW);
#else
    GPIOSetMode(hGPIO, WLAN_GPIO_INTERRUPT_LINE, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH);
#endif
#endif

	*InterruptVector = GPIOGetSystemIrq(hGPIO,IRQ_LINE);

	//if(!GPIOIoControl(

 //           hGPIO, 

 //           IOCTL_GPIO_GET_OMAP_HW_INTR,

 //           (UCHAR*)&IRQ_LINE,

 //           sizeof(IRQ_LINE),

 //           (UCHAR*)InterruptVector,

 //           sizeof(*InterruptVector),

 //           0,

 //           0))

 //       {

 //       PRINTF(DBG_INIT_FATAL_ERROR,("Error getting hardware interrupt.\n"));

 //       return TI_NOK;

 //       }
   GPIOClose(hGPIO);

return TRUE;

}