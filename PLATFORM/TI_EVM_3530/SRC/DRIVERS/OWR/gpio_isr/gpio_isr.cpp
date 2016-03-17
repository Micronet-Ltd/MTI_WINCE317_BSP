/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module GPIO ISR
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

#include <bsp.h>
//#include <tsn.h>

#include <pkfuncs.h>
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <gpio.h>
#include <bus.h>
#include <oal_prcm.h>
//#include <oal_mem.h>

#include "owrdrv.h"
//#include "gpio_isr.h"
//#include "inst_isr_dbg.h"



#define us (PERCLK1_FREQUENCY/MHz + 1) // Number of Timer2 counts in 1 us (microsecond)
// Some 1-Wire timing constants
#define tRSTLmin 	(480*us) // Min duration of RSTL pulse
#define tRSTHmin 	(480*us) // Min duration of RSTH pulse
#define tPDHmax 	( 60*us) // Max iButton presence response time
#define tPDLmin 	( 60*us) // Min duration of iButton presence pulse
#define tSLOTmax 	(120*us) // Max duration of Read-Data time slot
#define tSLOTmin 	( 60*us) // Min duration of R/W time slot
#define tLOWmax 	( 15*us) // Max duration of R/W activation pulse
#define tLOWmin 	(  1*us) // Min duration of R/W activation pulse
#define tRECmin		(  1*us) // Min iButton recover time
#define tRECext 	(  7*us + tRECmin) // Extended by 7us?` iButton recover time (used after Write-Zero or Read-Zero)
#define tSLOT 		(tSLOTmin + tRECmin)
#define tSLOText 	(tSLOTmin + tRECext)




static UINT32 s_gpio2IntCnt = 0;
static OMAP_GPIO_ISR_INFO  s_OWRGpio2Info;

OMAP_GPIO_ISR_INFO *gpio2Cfg(OMAP_GPIO_ISR_INFO *pOWRGpioInfo)
{
  IOCTL_HAL_OALPATOVA_IN  pPA;
  DWORD                   dwSize;

   OWRISRMSG(OWR_ISR_DBG, (L"GPIO_2_ISR CFG: IOCTL_ISR_GPIO_CFG, inp = %d, out = %d, irqNum = %d, pa = 0x%X\r\n", 
	             pOWRGpioInfo->input, pOWRGpioInfo->output, pOWRGpioInfo->irqNum, 
				 pOWRGpioInfo->gpioModule.gpioModulePA));

   pPA.pa     =  pOWRGpioInfo->gpioModule.gpioModulePA; //OMAP_GPIO1_REGS_PA;
   pPA.cached = FALSE;
   if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), &pOWRGpioInfo->pGpio, sizeof(VOID *), &dwSize) )
   {
     OWRISRMSG(OWR_ISR_DBG, (L"GPIO_2_ISR CFG: IOCTL_ISR_GPIO_CFG, GPIO PA Failure\r\n"));
     return NULL;
   }

   memcpy((char*)&s_OWRGpio2Info, (const char*)pOWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO));
   return(&s_OWRGpio2Info);
}

DWORD gpio2Info(void)
{
  OWRISRMSG(OWR_ISR_DBG, (L"GPIO_2_ISR INFO: intCnt = %d\r\n", s_gpio2IntCnt) );
  return(s_gpio2IntCnt);
}

DWORD gpio2ISRHandler( VOID *pIsrInfo )
{
  OMAP_GPIO_ISR_INFO   *pOWRGpioInfo = (OMAP_GPIO_ISR_INFO *)pIsrInfo;
  UINT32               mask,status;
  DWORD                irqBaseNum;

  
  // Check if it is our IRQ. It can be some other from group #6
  status = INREG32(&pOWRGpioInfo->pGpio->IRQSTATUS1);
  //mask = 1<<(pOWRGpioInfo->irqNum - IRQ_GPIO_0);
  irqBaseNum = ( ((pOWRGpioInfo->irqNum - IRQ_GPIO_0)>>5) << 5 ) + IRQ_GPIO_0;
  mask = 1<<(pOWRGpioInfo->irqNum - irqBaseNum);

  if( (mask & status) == 0 )
  {
   return SYSINTR_CHAIN;
  }
  else
    s_gpio2IntCnt++;
  

   OUTREG32(&pOWRGpioInfo->pGpio->SETIRQENABLE1, 1<<(pOWRGpioInfo->irqNum - irqBaseNum));   // unmask IRQ
   OUTREG32(&pOWRGpioInfo->pGpio->SETWAKEUPENA,  1<<(pOWRGpioInfo->irqNum - irqBaseNum));
   return(SYSINTR_NOP);
}


volatile static UINT32 s_gpio163IntCnt = 0;
volatile static UINT32 s_gpioM6IntCnt = 0;
static OMAP_GPIO_ISR_INFO  s_OWRGpio163Info;

OMAP_GPIO_ISR_INFO *gpio163Cfg(OMAP_GPIO_ISR_INFO *pOWRGpioInfo)
{
  IOCTL_HAL_OALPATOVA_IN  pPA;
  DWORD                   dwSize;

   OWRISRMSG(OWR_ISR_DBG, (L"GPIO_163_ISR CFG: IOCTL_ISR_GPIO_CFG, inp = %d, out = %d, irqNum = %d, pa = 0x%X\r\n", 
	             pOWRGpioInfo->input, pOWRGpioInfo->output, pOWRGpioInfo->irqNum, 
				 pOWRGpioInfo->gpioModule.gpioModulePA));

   pPA.pa     =  pOWRGpioInfo->gpioModule.gpioModulePA; //OMAP_GPIO1_REGS_PA;
   pPA.cached = FALSE;
   if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), &pOWRGpioInfo->pGpio, sizeof(VOID *), &dwSize) )
   {
     OWRISRMSG(OWR_ISR_DBG, (L"GPIO_163_ISR CFG: IOCTL_ISR_GPIO_CFG, GPIO PA Failure\r\n"));
     return NULL;
   }

   memcpy((char*)&s_OWRGpio163Info, (const char*)pOWRGpioInfo, sizeof(OMAP_GPIO_ISR_INFO));
   return(&s_OWRGpio163Info);
}

DWORD gpio163Info(void)
{
  OWRISRMSG(OWR_ISR_DBG, (L"GPIO_163_ISR INFO: intCnt = %d\r\n", s_gpio163IntCnt));
  OWRISRMSG(OWR_ISR_DBG, (L"GPIO_FSM INFO: cnt = %d, begin = 0x%X, end = 0x%x\r\n", g_OwrInputState.numOfPulses, g_OwrInputState.cntBegin, g_OwrInputState.cntEnd ));
  return(s_gpio163IntCnt);
}


// Get pointer to OWR GPIO data
OMAP_GPIO_ISR_INFO *getOWRGpio(void)
{
  return(&s_OWRGpio163Info);
}


DWORD gpio163ISRHandler( VOID *pIsrInfo )
{
  OMAP_GPIO_ISR_INFO *pOWRGpioInfo = (OMAP_GPIO_ISR_INFO *)pIsrInfo;
  UINT32 mask,status;
  OMAP_GPT_ISR_INFO  *pOWRGptInfo;
  DWORD               irqBaseNum;
  
  pOWRGptInfo = getOWRGpt();

  // Check if it is our IRQ. It can be some other from group #6
  status = INREG32(&pOWRGpioInfo->pGpio->IRQSTATUS1);
  //mask = 1<<(pOWRGpioInfo->irqNum - IRQ_GPIO_160);
  //irqBaseNum = ( ((pOWRGpioInfo->irqNum - IRQ_GPIO_0)/32)*32) + IRQ_GPIO_0;
  irqBaseNum = ( ((pOWRGpioInfo->irqNum - IRQ_GPIO_0)>>5) << 5 ) + IRQ_GPIO_0;
  mask = 1<<(pOWRGpioInfo->irqNum - irqBaseNum);

  if( (mask & status) == 0 )
  {
   s_gpioM6IntCnt++;
   return SYSINTR_CHAIN;
  }
  else
    s_gpio163IntCnt++;
  
  if( g_OwrInputState.signalIST == FALSE )
  {
   switch(g_OwrInputState.lineState)
   {
    case OWR_LINE_IMPULSE_BEGIN:
	 {
       g_OwrInputState.cntBegin = INREG32(&pOWRGptInfo->pGpt->TCRR); 

	   // Reprogram GPIO for Rising edge interrupt
	   CLRREG32(&pOWRGpioInfo->pGpio->FALLINGDETECT, mask);
	   SETREG32(&pOWRGpioInfo->pGpio->RISINGDETECT, mask);

	   g_OwrInputState.lineState = OWR_LINE_IMPULSE_END;

	 } break;

	case OWR_LINE_IMPULSE_END:
	 {
       g_OwrInputState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR); 

	   // Reprogram GPIO for Falling edge interrupt
	   // disable GPIO interrupt
	   CLRREG32(&pOWRGpioInfo->pGpio->RISINGDETECT, mask);
	   CLRREG32(&pOWRGpioInfo->pGpio->FALLINGDETECT, mask);

	   g_OwrInputState.numOfPulses++;
	   g_OwrInputState.lineState = OWR_LINE_IMPULSE_BEGIN;

	 } break;

	default: break; 
   }

    //OUTREG32(&pOWRGpioInfo->pGpio->SETIRQENABLE1, 1<<(pOWRGpioInfo->irqNum - IRQ_GPIO_160));   // unmask IRQ
      OUTREG32(&pOWRGpioInfo->pGpio->SETIRQENABLE1, 1<<(pOWRGpioInfo->irqNum - irqBaseNum));   // unmask IRQ
    //OUTREG32(&pOWRGpioInfo->pGpio->SETWAKEUPENA,  1<<(pOWRGpioInfo->irqNum - IRQ_GPIO_160));
	  OUTREG32(&pOWRGpioInfo->pGpio->SETWAKEUPENA,  1<<(pOWRGpioInfo->irqNum - irqBaseNum));
    return(SYSINTR_NOP);
  }
  else
  {
   return(pOWRGpioInfo->sysIntr);
  }
}
