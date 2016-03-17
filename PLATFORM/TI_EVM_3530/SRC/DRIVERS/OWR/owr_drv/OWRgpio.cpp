/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module OWR GPIO
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   1-Dec-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/
#include <bsp.h>
//#include <tsn.h>

#include <pkfuncs.h>
#include <windows.h>
#include <strsafe.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <bus.h>
#include <oal_prcm.h>

//#include "..\gpio_isr\gpio_isr.h"
//#include "..\gpio_isr\gpt_isr.h"
#include "owrdrv.h"

/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
static BOOL getGPIOModuleInfo(OMAP_GPIO_ISR_INFO *pGPIOModule);

/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
/******************** FUNCTION DEFINITIONS ************************************/

//------------------------------------------------------------------------------
// GPIO Init
//

BOOL owrGPIOInit(OMAP_GPIO_ISR_INFO *pOWRGpioInfo)
{
 //DWORD   sysIntr;

    if( pOWRGpioInfo == NULL )
      return FALSE;


    pOWRGpioInfo->hGpio = GPIOOpen();
    if (pOWRGpioInfo->hGpio == NULL)
    {
		RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIOInit: Failed to open Gpio driver\r\n"));
		//OWR_Deinit((DWORD)pCxt);
        return FALSE;
    }

   // SetInitConfig(pCxt);
	//GPIOSetMode(pOWRGpioInfo->hGpio, GPIO_2, GPIO_DIR_OUTPUT);
	//OUTREG32(&pGpio->CLEARDATAOUT, (1<<GPIO_2) );
	
    //GPIOSetMode(pOWRGpioInfo->hGpio, pOWRGpioInfo->input, GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW);
	GPIOSetMode(pOWRGpioInfo->hGpio, pOWRGpioInfo->input, GPIO_DIR_INPUT);
	//GPIOSetMode(pOWRGpioInfo->hGpio, pOWRGpioInfo->input, GPIO_DIR_INPUT);
	GPIOSetMode(pOWRGpioInfo->hGpio, pOWRGpioInfo->output, GPIO_DIR_OUTPUT);


	// Map interrupt
	pOWRGpioInfo->irqNum = GPIOGetSystemIrq(pOWRGpioInfo->hGpio, pOWRGpioInfo->input);

	if( (getGPIOModuleInfo(pOWRGpioInfo)) == FALSE )
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIOInit: getGPIOModuleInfo  failure\r\n"));
	}
	else
	{
		RETAILMSG(OWR_DRV_DBG, (L"owrGPIOInit: getGPIOModuleInfo: Num = %d, IRQ = %d, PA = 0x%X\r\n", 
			           pOWRGpioInfo->gpioModule.gpioModuleNum, 
					   pOWRGpioInfo->gpioModule.gpioModuleIrq,
					   pOWRGpioInfo->gpioModule.gpioModulePA
					   ));
	}

    if( (pOWRGpioInfo->hGPIOEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIOInit: CreateEvent failed\r\n"));
	  return FALSE;
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"owrGPIOInit: GPIO Event handle = 0x%X\r\n", pOWRGpioInfo->hGPIOEvent));
	}


	if(!GPIOInterruptInitialize(pOWRGpioInfo->hGpio, pOWRGpioInfo->input, pOWRGpioInfo->hGPIOEvent))
	 {
       RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIOInit: GPIO InterruptInitialize failure\r\n"));
       return FALSE;
	 }
	else
	{
	  RETAILMSG(OWR_DRV_DBG, (L"owrGPIOInit: GPIO InterruptInitialize Ok\r\n"));
	}

	// mask interrupt
	GPIOInterruptMask(pOWRGpioInfo->hGpio, pOWRGpioInfo->input, TRUE);

	if(!KernelIoControl(IOCTL_HAL_IRQ2SYSINTR, &pOWRGpioInfo->irqNum, sizeof(pOWRGpioInfo->irqNum),  &pOWRGpioInfo->sysIntr, sizeof(pOWRGpioInfo->sysIntr), 0))
    {
		RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIOInit: cannot get sysIntr\r\n"));
		return FALSE;
    }
	else
	{
      RETAILMSG(OWR_DRV_DBG, (L"owrGPIOInit: Inp num = %d, Out num = %d\r\n", pOWRGpioInfo->input, pOWRGpioInfo->output));
      RETAILMSG(OWR_DRV_DBG, (L"owrGPIOInit: Irq num = %d, sysIntr = %d\r\n", pOWRGpioInfo->irqNum, pOWRGpioInfo->sysIntr));
	}


	return(TRUE);
}


static BOOL getGPIOModuleInfo(OMAP_GPIO_ISR_INFO *pOWRGpioInfo)
{
	if( pOWRGpioInfo == NULL )
		return FALSE;
   
	DWORD  irq;

	irq = pOWRGpioInfo->irqNum;

	    if ( irq < IRQ_GPIO_0 )
          return FALSE; 
        else if ( irq <= IRQ_GPIO_31 )
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 1;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO1_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO1_REGS_PA;
            }
        else if (irq <= IRQ_GPIO_63)
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 2;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO2_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO2_REGS_PA;
            }
        else if (irq <= IRQ_GPIO_95)
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 3;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO3_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO3_REGS_PA;
            }
        else if (irq <= IRQ_GPIO_127)
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 4;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO4_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO4_REGS_PA;
            }
        else if (irq <= IRQ_GPIO_159)
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 5;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO5_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO5_REGS_PA;
            }
        else if (irq <= IRQ_GPIO_191)
            {
             pOWRGpioInfo->gpioModule.gpioModuleNum = 6;
             pOWRGpioInfo->gpioModule.gpioModuleIrq = IRQ_GPIO6_MPU;
             pOWRGpioInfo->gpioModule.gpioModulePA = OMAP_GPIO6_REGS_PA;
            }

   return TRUE;
}



void owrGPIODeInit(OMAP_GPIO_ISR_INFO *pOWRGpioInfo)
{
  if( pOWRGpioInfo == NULL )
  {
	RETAILMSG(OWR_DRV_DBG, (L"ERROR: owrGPIODeInit: pOWRGpioInfo invalid pointer\r\n"));
	return;
  }

	if( !(getGPIOModuleInfo(pOWRGpioInfo)) )
	{
	 RETAILMSG(OWR_DRV_DBG, (L"getGPIOModuleInfo failed\r\n"));
	 return;
	}

	if( pOWRGpioInfo->hGPIOEvent != NULL )
	{
	 if( CloseHandle(pOWRGpioInfo->hGPIOEvent) )
	  RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: GPIO IST event # %d  closed\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	 else
	  RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: GPIO IST event # %d close failure\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	}

#if 0
    // release the SYSINTR value
	if( pOWRGpioInfo->sysIntr != -1 )
	{
      if( KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pOWRGpioInfo->sysIntr, sizeof(DWORD), NULL, 0, NULL) )
		RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: Gpio Module # %d sysIntr released\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	  else
		RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: Gpio Module # %d sysIntr releas failure\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	}
	else
      RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: Gpio Module # %d Invalid s_sysIntr\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
#endif

	if( GPIOInterruptDisable(pOWRGpioInfo->hGpio, pOWRGpioInfo->input) )
	{
		RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: Gpio Module # %d Disabled Ok\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	}
	else
	{
		RETAILMSG(OWR_DRV_DBG, (L"owrGPIODeInit: Gpio Module # %d Disabling Failure\r\n", pOWRGpioInfo->gpioModule.gpioModuleNum ));
	}


	if (pOWRGpioInfo->hGpio != NULL)
	  GPIOClose(pOWRGpioInfo->hGpio);
}
