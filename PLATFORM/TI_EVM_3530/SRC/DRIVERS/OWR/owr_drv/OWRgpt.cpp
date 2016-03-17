/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module OWR GPTimer (#3)
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
static BOOL getGPTModuleInfo(OMAP_GPT_ISR_INFO *pGPTimerInfo);

/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
static OMAP_GPTIMER_REGS   *s_pGPT3 = NULL;

//static HANDLE        s_hGPT3Event = NULL;
//static DWORD         s_sysIntr = -1;

/******************** FUNCTION DEFINITIONS ************************************/

//------------------------------------------------------------------------------
// GPT3 Timer Init
//

BOOL gpTimerInit(OMAP_GPT_ISR_INFO *pGPTimerInfo)
{
    DWORD tclr = 0;
    //DWORD sysClkFreq;
	PHYSICAL_ADDRESS   pA;
	OMAP_GPTIMER_REGS   *s_pGPT3 = NULL;
	DWORD         dwSize;
    IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};
    IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN  s_pParentClk;
    // For test
	HANDLE    hTimer;

    if( pGPTimerInfo == NULL )
     return(FALSE);

	if( !(getGPTModuleInfo(pGPTimerInfo)) )
     return(FALSE);

	pGPTimerInfo->gpTimerDummyIrQNum = IRQ_SW_RESERVED_3;
	
    RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: Start 6.6\r\n"));

	pA.QuadPart = pGPTimerInfo->gpTimerRegPA; //OMAP_GPTIMER4_REGS_PA
	s_pGPT3 = (OMAP_GPTIMER_REGS*)MmMapIoSpace( pA, sizeof(OMAP_GPTIMER_REGS), FALSE);
	if( s_pGPT3 == NULL )
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimerInit: s_pGPT3  MmMapIoSpace  failure\r\n"));
	  return FALSE;
	}
	else
	{
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: s_pGPT3 addr = 0x%X\r\n", s_pGPT3) );
	}


    // use sys clk 
    //PrcmClockSetParent(kGPT3_ALWON_FCLK, kSYS_CLK);
    //IOCTL_PRCM_CLOCK_SET_SOURCECLOCK
	s_pParentClk.clkId = pGPTimerInfo->gpTimer_clkId; //  kGPT3_ALWON_FCLK;
    s_pParentClk.newParentClkId = pGPTimerInfo->gpTimer_newParentClkId;  // kSYS_CLK;
	if( !KernelIoControl(IOCTL_PRCM_CLOCK_SET_SOURCECLOCK, (LPVOID)&s_pParentClk, sizeof(s_pParentClk), NULL, 0, &dwSize) )
	{
     MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	 s_pGPT3 = NULL;
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimerInit: PrcmClockSetParent  failure\r\n"));
     //ISRWriteDebugString((UINT8 *)"ERROR: gpTimer3Init: PrcmClockSetParent failure\r\n");
	 return FALSE;
	}
	else
	{
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: PrcmClockSetParent OK\r\n"));
	}


    //PrcmDeviceEnableClocks(OMAP_DEVICE_GPTIMER3, TRUE);
	s_pcrmDevEnClks.devId = pGPTimerInfo->gpTimer_devId;  //OMAP_DEVICE_GPTIMER3;
	s_pcrmDevEnClks.bEnable = TRUE;
	if( !KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize) )
	{
     MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	 s_pGPT3 = NULL;
     RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimerInit: PrcmDeviceEnableClocks  failure\r\n"));
     //ISRWriteDebugString((UINT8 *)"ERROR: gpTimer3Init: PrcmDeviceEnableClocks  failure\r\n");
	 return FALSE;
	}
	else
	{
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: PrcmDeviceEnableClocks OK\r\n"));
	}


    /* After PCRM clock is enable for GPTimer, its registers are available */


    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, (LPVOID)&pGPTimerInfo->gpTimerIrQNum, sizeof(pGPTimerInfo->gpTimerIrQNum), (LPVOID)&pGPTimerInfo->sysIntr, sizeof(pGPTimerInfo->sysIntr), NULL )) 
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimerInit: IOCTL_HAL_REQUEST_SYSINTR  failure\r\n"));
	  MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	  s_pGPT3 = NULL;
	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);
	  return FALSE;
      //  goto cleanUp;
    }
	else
	{
      RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: sysIntr = %d\r\n", pGPTimerInfo->sysIntr));
	}

	// Request dummy sysintr
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, (LPVOID)&pGPTimerInfo->gpTimerDummyIrQNum, sizeof(pGPTimerInfo->gpTimerDummyIrQNum), (LPVOID)&pGPTimerInfo->dummySysIntr, sizeof(pGPTimerInfo->dummySysIntr), NULL )) 
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimerInit: IOCTL_HAL_REQUEST_SYSINTR  failure\r\n"));
	  MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	  s_pGPT3 = NULL;
	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);
	  return FALSE;
      //  goto cleanUp;
    }
	else
	{
      RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: dummySysIntr = %d\r\n", pGPTimerInfo->dummySysIntr));
	}


    if( (hTimer = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimer4Init: CreateEvent failed\r\n")); 

	  MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	  s_pGPT3 = NULL;
	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);

      // release the SYSINTR value
      KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pGPTimerInfo->sysIntr, sizeof(DWORD), NULL, 0, NULL);

	  return FALSE;
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"gpTimer4Init: Timer Event handle = 0x%X\r\n", hTimer)); 
	 pGPTimerInfo->hGPTEvent = hTimer;
	}

	// dummy Event
	if( (pGPTimerInfo->hGPTDummyEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL )
	{
      RETAILMSG(OWR_DRV_DBG, (L"ERROR: gpTimer4Init: CreateEvent failed\r\n")); 

	  MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	  s_pGPT3 = NULL;
	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);

      // release the SYSINTR value
      KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pGPTimerInfo->sysIntr, sizeof(DWORD), NULL, 0, NULL);

	  return FALSE;
	}
	else
	{
     RETAILMSG(OWR_DRV_DBG, (L"gpTimer4Init: Timer Dummy Event handle = 0x%X\r\n", pGPTimerInfo->hGPTDummyEvent)); 
	}


	RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit: Timer params: num = %d, irq = %d, gptReg = 0x%X, ICLReg = 0x%X,\
		                      sysIntr = %d, hEvent = 0x%X, DummyIrQNum = %d, dummySysIntr = %d, hGPTDummyEvent = 0x%X\r\n",
                       pGPTimerInfo->gpTimerNum,
					   pGPTimerInfo->gpTimerIrQNum,
                       pGPTimerInfo->pGpt,
                       pGPTimerInfo->pICL,
                       pGPTimerInfo->sysIntr,
                       pGPTimerInfo->hGPTEvent,
                       pGPTimerInfo->gpTimerDummyIrQNum,
                       pGPTimerInfo->dummySysIntr,
                       pGPTimerInfo->hGPTDummyEvent
					   ));

    // stop timer
	OUTREG32(&s_pGPT3->TCLR, 0);

    // configure performance timer
    //---------------------------------------------------
    // Soft reset GPTIMER and wait until finished
    SETREG32(&s_pGPT3->TIOCP, SYSCONFIG_SOFTRESET);
    while ((INREG32(&s_pGPT3->TISTAT) & GPTIMER_TISTAT_RESETDONE) == 0);

    // Select non-posted mode
	// !because reset default is '1' - Posted mode
	CLRREG32(&s_pGPT3->TSICR, GPTIMER_TSICR_POSTED);


    // Enable smart idle and autoidle
    // Set clock activity - FCLK can be  switched off, 
    // L4 interface clock is maintained during wkup.
    OUTREG32(&s_pGPT3->TIOCP, 
        /*0x200 | */ SYSCONFIG_CLOCKACTIVITY_IF_ON | SYSCONFIG_NOIDLE
		/* | SYSCONFIG_SMARTIDLE| SYSCONFIG_ENAWAKEUP | SYSCONFIG_AUTOIDLE*/); 

    // clear match register
    OUTREG32(&s_pGPT3->TMAR, 0x0);


    // clear interrupts
    OUTREG32(&s_pGPT3->TISR, 0x00000007);
    
    // enable (match)overflow interrupt
	//OUTREG32(&s_pGPT3->TIER, GPTIMER_TIER_OVERFLOW );
	//OUTREG32(&s_pGPT3->TIER, 0 );
 
    // enable wakeups
	OUTREG32(&s_pGPT3->TWER, 0/*GPTIMER_TWER_MATCH*/ );

    //GPTIMER_TCLR_PRE by 8
    //OUTREG32(&s_pGPT3->TCLR, GPTIMER_TCLR_PRE | (2 << 2) );



    // Set the load register value.
	// FFFF9A70
    //OUTREG32(&s_pGPT3->TLDR, 0xFFFBE76D);
 
    // Trigger a counter reload by writing    
    //OUTREG32(&s_pGPT3->TTGR, 0xFFFFFFFF);

	 RETAILMSG(OWR_DRV_DBG, (L"gpTimerInit finished\r\n")); 

	 return TRUE;
}


void gpTimerDeInit(OMAP_GPT_ISR_INFO *pGPTimerInfo)
{
  DWORD  dwSize;
  IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};

    if( pGPTimerInfo == NULL )
	{
      RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit # %d failed, pGPTimerInfo = NULL\r\n", pGPTimerInfo->gpTimerNum));
	  return;
	}

	if( !(getGPTModuleInfo(pGPTimerInfo)) )
	{
     RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit # %d failed\r\n"));
     return;
	}


    if( s_pGPT3 != NULL )
	{
      OUTREG32(&s_pGPT3->TCLR, 0);  // stop timer
	  MmUnmapIoSpace( (PVOID)s_pGPT3, sizeof(OMAP_GPTIMER_REGS));
	  s_pGPT3 = NULL;
	  RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d stopped\r\n", pGPTimerInfo->gpTimerNum ));
	}


	if( pGPTimerInfo->hGPTEvent != NULL )
	{
     if( CloseHandle(pGPTimerInfo->hGPTEvent) )
	  RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d IST event closed\r\n", pGPTimerInfo->gpTimerNum ));
	 else
      RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d IST event close failure\r\n", pGPTimerInfo->gpTimerNum ));
	}

	// Switch Off PCRM clock on Failure
	 s_pcrmDevEnClks.bEnable = FALSE;
	 s_pcrmDevEnClks.devId = pGPTimerInfo->gpTimer_devId;  //OMAP_DEVICE_GPTIMER3;
	 if( KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize) )
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d pcrmDevEnClks disabled\r\n", pGPTimerInfo->gpTimerNum ));
	 else
        RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d pcrmDevEnClks disable failure\r\n", pGPTimerInfo->gpTimerNum ));

    // release the SYSINTR value
	if( pGPTimerInfo->sysIntr != -1 )
	{
      if( KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pGPTimerInfo->sysIntr, sizeof(DWORD), NULL, 0, NULL) )
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d sysIntr released\r\n", pGPTimerInfo->gpTimerNum ));
	  else
		RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d sysIntr releas failure\r\n", pGPTimerInfo->gpTimerNum ));
	}
	else
      RETAILMSG(OWR_DRV_DBG, (L"gpTimerDeInit: timer # %d Invalid s_sysIntr\r\n", pGPTimerInfo->gpTimerNum ));

	return;
}


static BOOL getGPTModuleInfo(OMAP_GPT_ISR_INFO *pGPTimerInfo)
{
   if( pGPTimerInfo == NULL )
   {
     RETAILMSG(OWR_DRV_DBG, (L"getGPTModuleInfo: Inv ptr\r\n"));
	 return(FALSE);
   }

   RETAILMSG(OWR_DRV_DBG, (L"getGPTModuleInfo: Start\r\n"));

   // Allowed GPTimers from 4 to 7
   if( ( pGPTimerInfo->gpTimerNum < 3 ) || ( pGPTimerInfo->gpTimerNum > 7 ) )
   {
     RETAILMSG(OWR_DRV_DBG, (L"getGPTModuleInfo: Inv Timer number\r\n"));
     return(FALSE);
   }

   switch(pGPTimerInfo->gpTimerNum)
   {
    case 3:
		{
         pGPTimerInfo->gpTimerIrQNum = IRQ_GPTIMER3;
         pGPTimerInfo->gpTimer_clkId = kGPT3_ALWON_FCLK;
		 pGPTimerInfo->gpTimer_newParentClkId = kSYS_CLK;
		 pGPTimerInfo->gpTimer_devId = OMAP_DEVICE_GPTIMER3;
		 pGPTimerInfo->gpTimerRegPA = OMAP_GPTIMER3_REGS_PA;
		} break;

    case 4:
		{
         pGPTimerInfo->gpTimerIrQNum = IRQ_GPTIMER4;
         pGPTimerInfo->gpTimer_clkId = kGPT4_ALWON_FCLK;
		 pGPTimerInfo->gpTimer_newParentClkId = kSYS_CLK;
		 pGPTimerInfo->gpTimer_devId = OMAP_DEVICE_GPTIMER4;
		 pGPTimerInfo->gpTimerRegPA = OMAP_GPTIMER4_REGS_PA;
		} break;

    case 5:
		{
         pGPTimerInfo->gpTimerIrQNum = IRQ_GPTIMER5;
         pGPTimerInfo->gpTimer_clkId = kGPT5_ALWON_FCLK;
		 pGPTimerInfo->gpTimer_newParentClkId = kSYS_CLK;
		 pGPTimerInfo->gpTimer_devId = OMAP_DEVICE_GPTIMER5;
         pGPTimerInfo->gpTimerRegPA = OMAP_GPTIMER5_REGS_PA;
		} break;

    case 6:
		{
         pGPTimerInfo->gpTimerIrQNum = IRQ_GPTIMER6;
         pGPTimerInfo->gpTimer_clkId = kGPT6_ALWON_FCLK;
		 pGPTimerInfo->gpTimer_newParentClkId = kSYS_CLK;
		 pGPTimerInfo->gpTimer_devId = OMAP_DEVICE_GPTIMER6;
         pGPTimerInfo->gpTimerRegPA = OMAP_GPTIMER6_REGS_PA;
		} break;

    case 7:
		{
         pGPTimerInfo->gpTimerIrQNum = IRQ_GPTIMER7;
         pGPTimerInfo->gpTimer_clkId = kGPT7_ALWON_FCLK;
		 pGPTimerInfo->gpTimer_newParentClkId = kSYS_CLK;
		 pGPTimerInfo->gpTimer_devId = OMAP_DEVICE_GPTIMER7;
         pGPTimerInfo->gpTimerRegPA = OMAP_GPTIMER7_REGS_PA;
		} break;

	default: break;
   }

   return(TRUE);
}



BOOL GPTInterruptInitialize(OMAP_GPT_ISR_INFO *pGPTimerInfo)
{
  DWORD dwSize; 
  IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};
  //IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN  s_pParentClk;

	if( !InterruptInitialize(pGPTimerInfo->sysIntr, pGPTimerInfo->hGPTEvent, NULL, 0) )
	{
	  RETAILMSG(OWR_DRV_DBG, (L"ERROR: GPTInterruptInitialize: InterruptInitialize failure\r\n")); 

	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);

      // release the SYSINTR value
      KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pGPTimerInfo->sysIntr, sizeof(DWORD), NULL, 0, NULL);

	  CloseHandle(pGPTimerInfo->hGPTEvent);
	  return FALSE;
      //  goto cleanUp;
	}

	if( !InterruptInitialize(pGPTimerInfo->dummySysIntr, pGPTimerInfo->hGPTDummyEvent, NULL, 0) )
	{
	  RETAILMSG(OWR_DRV_DBG, (L"ERROR: GPTInterruptInitialize: Dummy InterruptInitialize failure\r\n")); 

	  // Switch Off PCRM clock on Failure
	  s_pcrmDevEnClks.bEnable = FALSE;
	  KernelIoControl(IOCTL_HAL_PRCMDEVICEENABLECLOCKS, (LPVOID)&s_pcrmDevEnClks, sizeof(s_pcrmDevEnClks), NULL, 0, &dwSize);

      // release the SYSINTR value
      KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pGPTimerInfo->dummySysIntr, sizeof(DWORD), NULL, 0, NULL);

	  CloseHandle(pGPTimerInfo->hGPTDummyEvent);
	  return FALSE;
      //  goto cleanUp;
	}

  return(TRUE);
}

void GPTInterruptDone(OMAP_GPT_ISR_INFO *pGPTimerInfo)
{
	InterruptDone(pGPTimerInfo->sysIntr);
}
