/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module GPT ISR
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   9-Dec-2009
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
//#include "gpt_isr.h"
//#include "inst_isr_dbg.h"



//static UINT32 s_gpt4IntCnt = 0;
static UINT32 s_gptTCRR = 0;
static OMAP_GPT_ISR_INFO  s_OWRgpt4Info;

OMAP_GPT_ISR_INFO *gpt4Cfg(OMAP_GPT_ISR_INFO *pOWRGptInfo)
{
  IOCTL_HAL_OALPATOVA_IN  pPA;
  DWORD                   dwSize;
  OMAP_GPTIMER_REGS   *pGPT = NULL;
  IOCTL_HAL_PRCMDEVICEENABLECLOCKS_IN  s_pcrmDevEnClks = {0,0};
//  IOCTL_PRCM_CLOCK_SET_SOURCECLOCK_IN  s_pParentClk;



   OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_GPIO_CFG, Timer NUm = %d\r\n", pOWRGptInfo->gpTimerNum));

   // Get GPTimer Registers addr
   pPA.pa     =  pOWRGptInfo->gpTimerRegPA; // OMAP_GPTIMER4_REGS_PA
   pPA.cached = FALSE;
   if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), &pOWRGptInfo->pGpt, sizeof(VOID *), &dwSize) )
   {
     OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_TIMER_CFG, GPT PA Failure\r\n"));
     return NULL;
   }
   else
   {
    OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_TIMER_CFG, GPT PA = 0x%X, UA = 0x%X\r\n", pPA.pa, pOWRGptInfo->pGpt));
   }

   // Get OMAP MPU Interrupt Controller Registers addr
   pPA.pa     = OMAP_INTC_MPU_REGS_PA; //
   pPA.cached = FALSE;
   if( !KernelIoControl(IOCTL_HAL_OALPATOVA, (LPVOID)&pPA, sizeof(IOCTL_HAL_OALPATOVA_IN), &pOWRGptInfo->pICL, sizeof(VOID *), &dwSize) )
   {
     OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_TIMER_CFG, ILC PA Failure\r\n"));
     return NULL;
   }
   {
    OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_TIMER_CFG, ILC PA = 0x%X, UA = 0x%X\r\n", pPA.pa, pOWRGptInfo->pICL) );
   }

   OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR CFG: IOCTL_ISR_TIMER_CFG Finish Init GPTimer registers\r\n"));

   memcpy((char*)&s_OWRgpt4Info, (const char*)pOWRGptInfo, sizeof(OMAP_GPT_ISR_INFO));
   return(&s_OWRgpt4Info);
}

DWORD gpt4Info(void)
{
  OWRISRMSG(OWR_ISR_DBG, (L"GPT_4_ISR INFO: intCnt = %d, TCRR = 0x%X\r\n", g_OwrGptState.gptIntrCnt, s_gptTCRR));
  return(g_OwrGptState.gptIntrCnt);
}


OMAP_GPT_ISR_INFO *getOWRGpt(void)
{
  return(&s_OWRgpt4Info);
}

DWORD gpt4ISRHandler( VOID *pIsrInfo )
{
  volatile OMAP_GPT_ISR_INFO  *pOWRGptInfo = (OMAP_GPT_ISR_INFO *)pIsrInfo;
  OMAP_GPIO_ISR_INFO *pOWRGpio;
  UINT32             mask;  
  register UINT32     bitNum;
  volatile register UINT32    temp;
  UINT32            irqBaseNum, gpioBaseNum;

  pOWRGpio = getOWRGpio();

  irqBaseNum  = ( ((pOWRGpio->irqNum - IRQ_GPIO_0)>>5) << 5 ) + IRQ_GPIO_0;
  gpioBaseNum = ( ((pOWRGpio->output - GPIO_0)>>5) << 5 ) + GPIO_0;

  // Clear interrupt
      OUTREG32(&pOWRGptInfo->pGpt->TISR, GPTIMER_TIER_MATCH);

   // Stop Timer
   //  OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);

      //s_gpt4IntCnt++;
	  g_OwrGptState.gptIntrCnt++;

	//  s_gptTCRR = INREG32(&pOWRGptInfo->pGpt->TCRR);

      switch(g_OwrGptState.phase)
	  {
	   case OWR_RESET_PRESENCE:
	    {
          switch(g_OwrGptState.state)
		  {
		   case OWR_RESET_BEGIN:
			{
             // Start Reset Impulse, set OWR line low
		     OUTREG32( &pOWRGpio->pGpio->SETDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
			 g_OwrGptState.cntBegin = INREG32(&pOWRGptInfo->pGpt->TCRR);
			 // Re-calculate MAT register for enter impulse End
             temp = g_OwrGptState.cntBegin + 12860; // 500 us
			 g_OwrGptState.cntMatch = temp;
			 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

			 g_OwrGptState.state = OWR_RESET_END;

			} break;

		   case OWR_RESET_END:
			{
              g_OwrGptState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR);

              // End Reset Impulse, set OWR line high
              OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );

			  //g_OwrGptState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR);

			  // Enable input GPIO interrupt (failing edge) for Presence impulse capture
			  mask = 1<<(pOWRGpio->irqNum - irqBaseNum /*IRQ_GPIO_160*/);
         	  CLRREG32(&pOWRGpio->pGpio->RISINGDETECT, mask);
	          SETREG32(&pOWRGpio->pGpio->FALLINGDETECT, mask);


              // Re-calculate MAT register for enter wait presence timeout
              temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
			  temp += (2*12860); // 2*500 us
			  OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

			  g_OwrGptState.state = OWR_PRESENCE_END;
			} break;

		   case OWR_PRESENCE_END:
			{
              // End of wait/presence procedure
              // Stop Timer here and signal IST(return(pOWRGptInfo->sysIntr) )
              // Stop Timer
              OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);

			  // Disable GPIO interrupt
			  mask = 1<<(pOWRGpio->irqNum - irqBaseNum /*IRQ_GPIO_160*/);
         	  CLRREG32(&pOWRGpio->pGpio->RISINGDETECT, mask);
	          CLRREG32(&pOWRGpio->pGpio->FALLINGDETECT, mask);

			  //g_OwrGptState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR);

              g_OwrGptState.state = OWR_RESET_BEGIN;
			  g_OwrGptState.endOp = TRUE;
			  g_OwrGptState.signalIST = TRUE;
			} break;

		   default: // Error state
			{
              // Stop Timer
              OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
			  g_OwrGptState.signalIST = TRUE;
			} break;

		  }

		} break;



	   case  OWR_WRITE_SLOTS:
		{

          switch(g_OwrGptState.state)
		  {
           case OWR_WRITE_SLOT_BEGIN:
			{
			  //register UINT32  cntBegin;

			  bitNum = g_OwrGptState.writeDataBitCnt;
			  // Error condition 
			  if( bitNum >= g_OwrGptState.writeDataBitLen )
			  {
				  // Stop Timer
				  OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
				  // End SLOT_0 Impulse, set OWR line high
				  OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
				  g_OwrGptState.signalIST = TRUE;
				  break;
			  }

			  //g_OwrGptState.log[bitNum] = '@';

				if( g_OwrGptState.writeData[bitNum>>3] & ( 1<<(bitNum&0x07) )  )
				{
                 // Write '1'
				 // Start Reset Impulse, set OWR line low for ~2 us
				 OUTREG32( &pOWRGpio->pGpio->SETDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
				 temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
				 while( (INREG32(&pOWRGptInfo->pGpt->TCRR) - temp) < 50);

				  // set OWR line high
				  OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );

				 // Re-calculate MAT register for enter impulse End
                 temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
			     temp += 1672; // 500 us
				 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

                 g_OwrGptState.state = OWR_WRITE_SLOT_END;

				 g_OwrGptState.log[bitNum] = '>';
				}
				else
				{
                 // Write '0' 
				 // Start Reset Impulse, set OWR line low
            	 OUTREG32( &pOWRGpio->pGpio->SETDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
				 temp = INREG32((volatile unsigned long *)&pOWRGptInfo->pGpt->TCRR);
				 // Re-calculate MAT register for enter impulse End
				 temp += 1672; // 65 us
				 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

				 g_OwrGptState.state = OWR_WRITE_SLOT_END;

				 //g_OwrGptState.log[bitNum] = '<';
				}

			} break;

           case OWR_WRITE_SLOT_END:
			{
              bitNum = g_OwrGptState.writeDataBitCnt;

              g_OwrGptState.writeDataBitCnt++;
			  if( (bitNum+1) >= g_OwrGptState.writeDataBitLen )
			  {
               // Stop Timer
               OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
			   g_OwrGptState.endOp = TRUE;
			   g_OwrGptState.signalIST = TRUE;
			  }
			  else
			  {
                //Reprogram Timer for next time slot
				 temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
				 temp += 1672/*250*/; // after ~10 (65) us
                 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);
			  }

              // If '0' End SLOT_0 Impulse, set OWR line high
			  if( g_OwrGptState.writeData[bitNum>>3] & ( 1<<(bitNum&0x07) )  )
				  ;
				 //g_OwrGptState.log[g_OwrGptState.writeDataBitCnt-1] = '0';
			  else
			  {
               OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
			   	 //g_OwrGptState.log[g_OwrGptState.writeDataBitCnt-1] = '1';
			  }
              
			  //g_OwrGptState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR);
              g_OwrGptState.state = OWR_WRITE_SLOT_BEGIN;
			  

			} break;

		   default: // error state
			{
              // Stop Timer
              OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
			  g_OwrGptState.signalIST = TRUE;
			} break;

		  }

		} break;



// ////////////////// Read

	   case  OWR_READ_SLOTS:
		{
          switch(g_OwrGptState.state)
		  {
		   case OWR_READ_SLOT_BEGIN:
			{
			  bitNum = g_OwrGptState.readDataBitCnt;
			  // Error condition 
			  if( bitNum >= g_OwrGptState.readDataBitLen )
			  {
				  // Stop Timer
				  OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
				  // End SLOT_0 Impulse, set OWR line high
				  OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );
				  g_OwrGptState.signalIST = TRUE;
				  break;
			  }

             //mask = 1<<(pOWRGpio->irqNum - irqBaseNum /*IRQ_GPIO_160*/);

             // Start Read Impulse, set OWR line low for ~2 us
		     OUTREG32( &pOWRGpio->pGpio->SETDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );

 		     temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
			 while( (INREG32(&pOWRGptInfo->pGpt->TCRR) - temp) < 50);

             // Return OWR line high
             OUTREG32( &pOWRGpio->pGpio->CLEARDATAOUT, (1<< (pOWRGpio->output - gpioBaseNum /*GPIO_160*/)) );

			 // Wait here additional 11 us and read value
			 while( (INREG32(&pOWRGptInfo->pGpt->TCRR) - temp) < (50 + 283) );
             
			 // Read Value Here
			 //DATAIN
			 
			 mask = INREG32( &pOWRGpio->pGpio->DATAIN );
			 if( mask & (1<< (pOWRGpio->input - gpioBaseNum /*GPIO_160*/)) )
			 {
              g_OwrGptState.readData[bitNum>>3] |= ( 1<<(bitNum&0x07) );
			 }
			 else
			 {
              g_OwrGptState.readData[bitNum>>3] &= (~(1<<(bitNum&0x07)));
			 }


			 // Re-calculate MAT register for enter impulse End
			 temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
			 temp += 1672; // 65 us
			 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

			 g_OwrGptState.state = OWR_READ_SLOT_END;

			} break;

           case OWR_READ_SLOT_END:
			{
              bitNum = g_OwrGptState.readDataBitCnt;

              g_OwrGptState.readDataBitCnt++;
			  if( (bitNum+1) >= g_OwrGptState.readDataBitLen )
			  {
               // Stop Timer
               OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
			   g_OwrGptState.endOp = TRUE;
			   g_OwrGptState.signalIST = TRUE;
			  }
			  else
			  {
                //Reprogram Timer for next time slot
				 temp = INREG32(&pOWRGptInfo->pGpt->TCRR);
				 temp += 1672 /*250*/; // after ~10 (65) us
                 OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);
			  }

              // Stop Timer
              //OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
              
			  //g_OwrGptState.cntEnd = INREG32(&pOWRGptInfo->pGpt->TCRR);
              g_OwrGptState.state = OWR_READ_SLOT_BEGIN;
			  //g_OwrGptState.endOp = TRUE;

			} break;

		   default: //error state
			{
              // Stop Timer
              OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
			  g_OwrGptState.signalIST = TRUE;
			} break;

		  }

		} break;

// ///////////////// End of Read
	   case OWR_IDLE:
		{ // Not supposed to enter here.
          // Just stop timer
          OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
		} break;

	   default: // Error state
		{
          // Stop Timer
          OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);
		  g_OwrGptState.signalIST = TRUE;
		} break;

	  }

/*
	  if( ( g_OwrGptState.state == OWR_RESET_BEGIN )&& (g_OwrGptState.phase == OWR_RESET_PRESENCE) )
	   return(pOWRGptInfo->sysIntr);
	  else  
	   return(SYSINTR_NOP);
*/

      if( g_OwrGptState.signalIST == TRUE )
	  {
       return(pOWRGptInfo->sysIntr); 
	  }
	  else
	  {
       // re-enable interrupts
	   OUTREG32(&pOWRGptInfo->pICL->INTC_MIR_CLEAR1, 1 << (pOWRGptInfo->gpTimerIrQNum - 32));
	   // For test puposes. for elimination entering IDLE state
       return(pOWRGptInfo->dummySysIntr); 
	   //return(SYSINTR_NOP);
	  }

}


