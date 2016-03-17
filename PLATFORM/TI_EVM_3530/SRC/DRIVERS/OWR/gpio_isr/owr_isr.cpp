/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP module GPIO ISR
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   10-Dec-2009
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

#include "owrdrv.h"

UINT32 g_owrImpulseState = 0;

volatile OWR_TIMER_STATE  g_OwrGptState;
volatile OWR_INPUT_STATE  g_OwrInputState;

//static BOOL  owrIsrWriteBit(UINT32 bit);
//static BOOL  owrIsrReadBit(UINT32 *readData);

// Set 'Reset' Pulse for 480 us
BOOL  owrIsrSetResetPulse(void)
{
	UINT32             lTCRR;
	OMAP_GPT_ISR_INFO *pOWRGptInfo;
	OMAP_GPIO_ISR_INFO *pOWRGpio;

	pOWRGptInfo = getOWRGpt();
	pOWRGpio = getOWRGpio();

    g_OwrGptState.phase = OWR_RESET_PRESENCE;
    g_OwrGptState.state = OWR_RESET_BEGIN;    // Different for Each phase
    g_OwrGptState.cntBegin = 0; // TCRR value (pulse beginning)
    g_OwrGptState.cntEnd = 0;   // TCRR value (pulse end)
	g_OwrGptState.endOp = FALSE;
	g_OwrGptState.signalIST = FALSE;

    g_OwrInputState.lineState = OWR_LINE_IMPULSE_BEGIN;
    g_OwrInputState.numOfPulses = 0;
    g_OwrInputState.cntBegin = 0;
    g_OwrInputState.cntEnd = 0; 
	
    OWRISRMSG(OWR_ISR_DBG, (L"owrIsrSetResetPulse: GP UA = 0x%X\r\n", pOWRGptInfo->pGpt));
	//ISRDbgPintf(L"owrIsrSetResetPulse: GP UA = 0x%X\r\n", pOWRGptInfo->pGpt);

	// Disable GPIO input interrupt.

	// Just read TCRR
	lTCRR = INREG32(&pOWRGptInfo->pGpt->TCRR);

	OWRISRMSG(OWR_ISR_DBG, (L"owrIsrSetResetPulse: TCRR = %d\r\n", lTCRR));

	// Stop Timer
	CLRREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST);

/////////////////////////////
/////////////////////////////////////

	// clear TCRR register
    OUTREG32(&pOWRGptInfo->pGpt->TCRR, 0);

    // Set match register
    OUTREG32(&pOWRGptInfo->pGpt->TMAR, 2600/*250*/); // ~ After 100 (10) us
    
    // clear interrupts
    OUTREG32(&pOWRGptInfo->pGpt->TISR, 0x00000007);
    
    // enable (match)overflow interrupt
	OUTREG32(&pOWRGptInfo->pGpt->TIER, GPTIMER_TIER_MATCH /*GPTIMER_TIER_OVERFLOW */);
	//OUTREG32(&pOWRGptInfo->pGpt->TIER, 0 );
 
    // enable wakeups
	//OUTREG32(&pOWRGptInfo->pGpt->TWER, GPTIMER_TWER_MATCH /*GPTIMER_TWER_OVERFLOW*/);
	
	//  Start the timer.  Do not set for auto reload
    SETREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST | GPTIMER_TCLR_CE /*| GPTIMER_TCLR_AR*/);

    OWRISRMSG(OWR_ISR_DBG, (L"owrIsrSetResetPulse  OK\r\n"));

	// test...
	//while(1);

    return(TRUE);
}


// For pulse detection, Timer used in non-interrupt mode
// Only for measuring pulse width
BOOL  owrIsrSetDetectPulse(BOOL pulseDetect)
{
	UINT32             lTCRR, mask;
	OMAP_GPT_ISR_INFO *pOWRGptInfo;
	OMAP_GPIO_ISR_INFO *pOWRGpio;
	UINT32             irqBaseNum;


	pOWRGptInfo = getOWRGpt();
	pOWRGpio = getOWRGpio();
    irqBaseNum = ( ((pOWRGpio->irqNum - IRQ_GPIO_0)>>5) << 5 ) + IRQ_GPIO_0;

   	// Disable GPIO input interrupt.
	// it will be temporary enabled at the end of Reset impulse
    //GPIOSetMode(pOWRGpio->hGpio, pOWRGpio->input, GPIO_DIR_INPUT);

	// Stop Timer
	OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);

	// clear TCRR register
    OUTREG32(&pOWRGptInfo->pGpt->TCRR, 0x00000000);

    // Set match register
    //OUTREG32(&pOWRGptInfo->pGpt->TMAR, 250); // ~ After 10 us
  
    // Disable (match)overflow interrupt
	OUTREG32(&pOWRGptInfo->pGpt->TIER, 0 );

    // clear interrupts
    OUTREG32(&pOWRGptInfo->pGpt->TISR, 0x00000007);

    // Disable wakeups
	//OUTREG32(&pOWRGptInfo->pGpt->TWER, GPTIMER_TWER_MATCH /*GPTIMER_TWER_OVERFLOW*/);
	OUTREG32(&pOWRGptInfo->pGpt->TWER, 0 );

	mask = 1<<(pOWRGpio->irqNum - irqBaseNum /*IRQ_GPIO_160*/);

	// Now Clear GPIO status register
    //OUTPORT32(&pOWRGpio->pGpio->IRQSTATUS1, mask);
    //OUTPORT32(&pOWRGpio->pGpio->IRQSTATUS2, mask);

    if( pulseDetect )
	{
	  // Enable input GPIO interrupt (failing edge) for Presence impulse capture
      GPIOSetMode(pOWRGpio->hGpio, pOWRGpio->input, GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW);
	  //CLRREG32(&pOWRGpio->pGpio->RISINGDETECT, mask);
	  //SETREG32(&pOWRGpio->pGpio->FALLINGDETECT, mask);
	  g_OwrInputState.signalIST = TRUE;
	}
	else
	{
	  // Disable input GPIO interrupt (failing edge) for Presence impulse capture
	  GPIOSetMode(pOWRGpio->hGpio, pOWRGpio->input, GPIO_DIR_INPUT);
	  //CLRREG32(&pOWRGpio->pGpio->RISINGDETECT, mask);
	  //CLRREG32(&pOWRGpio->pGpio->FALLINGDETECT, mask);
	  g_OwrInputState.signalIST = FALSE;
	}

    g_OwrGptState.phase = OWR_IDLE;
    g_OwrGptState.state = OWR_RESET_BEGIN;    // Different for Each phase
    g_OwrGptState.cntBegin = 0; // TCRR value (pulse beginning)
    g_OwrGptState.cntEnd = 0;   // TCRR value (pulse end)
	g_OwrGptState.endOp = FALSE;
	g_OwrGptState.signalIST = FALSE;

    g_OwrInputState.lineState = OWR_LINE_IMPULSE_BEGIN;
    g_OwrInputState.numOfPulses = 0;
    g_OwrInputState.cntBegin = 0;
    g_OwrInputState.cntEnd = 0; 

  OWRISRMSG(OWR_ISR_DBG, (L"owrIsrSetDetectPulse  OK\r\n"));
  return(TRUE);
}




BOOL  owrIsrWriteData(UINT8 data[], UINT32 len)
{
	UINT32             temp;
	OMAP_GPT_ISR_INFO *pOWRGptInfo;
	OMAP_GPIO_ISR_INFO *pOWRGpio;

	if( len > OWR_ISR_MAX_DATA_LEN )
     return(FALSE);

	pOWRGptInfo = getOWRGpt();
	pOWRGpio = getOWRGpio();

	g_OwrGptState.endOp = FALSE;

    g_OwrGptState.phase = OWR_WRITE_SLOTS;
    g_OwrGptState.state = OWR_WRITE_SLOT_BEGIN;

    g_OwrGptState.cntBegin = 0; // TCRR value (pulse beginning)
    g_OwrGptState.cntEnd = 0;   // TCRR value (pulse end)

	//g_OwrGptState.writeData[0] = data; // max 32 bytes for data write 
	memcpy( (void *)g_OwrGptState.writeData, data, len );

    g_OwrGptState.writeDataBitLen = 8*len;  // Actual length for write data in bits 
    g_OwrGptState.writeDataBitCnt = 0;  // current number of  written data in bits 
    g_OwrGptState.signalIST = FALSE;

	memset( (void *)g_OwrGptState.log, 0x00, OWR_TIMER_LOG_LEN );
	//OWRISRMSG(OWR_ISR_DBG, (L"owrIsrWriteBit %d\r\n", bit));

	// Stop Timer
	OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);

#if 0
	// clear TCRR register
    OUTREG32(&pOWRGptInfo->pGpt->TCRR, 0x00000000);
#endif

	// Re-calculate MAT register
    temp = INREG32(&pOWRGptInfo->pGpt->TCRR); // 
    temp += 250;
	OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

    // Set match register
    //OUTREG32(&pOWRGptInfo->pGpt->TMAR, 250); // ~ After 10 us
    
    // clear interrupts
    OUTREG32(&pOWRGptInfo->pGpt->TISR, 0x00000007);
    
    // enable (match)overflow interrupt
	//OUTREG32(&pOWRGptInfo->pGpt->TIER, GPTIMER_TIER_MATCH /*GPTIMER_TIER_OVERFLOW */);
	//OUTREG32(&pOWRGptInfo->pGpt->TIER, 0 );
 
    // enable wakeups
	//OUTREG32(&pOWRGptInfo->pGpt->TWER, GPTIMER_TWER_MATCH /*GPTIMER_TWER_OVERFLOW*/);
	//OUTREG32(&s_pGPT3->TWER, GPTIMER_TWER_MATCH );

	//  Start the timer.  Do not set for auto reload
    SETREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST | GPTIMER_TCLR_CE /*| GPTIMER_TCLR_AR*/);

#if 0
	//  Start the timer.  Also set for auto reload
    SETREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST /*| GPTIMER_TCLR_AR*/);
     while ((INREG32(pOWRGptInfo->pGpt->TWPS) & GPTIMER_TWPS_TCLR) != 0);
#endif


 // OWRISRMSG(OWR_ISR_DBG, (L"owrIsrWriteBit  OK\r\n"));
  return(TRUE);

}


BOOL  owrIsrReadData(UINT32 len)
{
	UINT32             temp;
	OMAP_GPT_ISR_INFO *pOWRGptInfo;
	OMAP_GPIO_ISR_INFO *pOWRGpio;
	//UINT32             tPrev, tNext;

	if( len > OWR_ISR_MAX_DATA_LEN )
     return(FALSE);

	pOWRGptInfo = getOWRGpt();
	pOWRGpio = getOWRGpio();

	g_OwrGptState.endOp = FALSE;

    g_OwrGptState.phase = OWR_READ_SLOTS;
    g_OwrGptState.state = OWR_READ_SLOT_BEGIN;    // Different for Each phase

    g_OwrGptState.cntBegin = 0; // TCRR value (pulse beginning)
    g_OwrGptState.cntEnd = 0;   // TCRR value (pulse end)

	memset( (void *)g_OwrGptState.readData, 0x00, len );
    g_OwrGptState.readDataBitLen = 8*len;  // Actual length for write data in bits 
    g_OwrGptState.readDataBitCnt = 0;  // current number of  written data in bits 
    g_OwrGptState.signalIST = FALSE;

	memset( (void *)g_OwrGptState.log, 0x00, OWR_TIMER_LOG_LEN );

	// Stop Timer
	OUTREG32(&pOWRGptInfo->pGpt->TCLR, 0);

#if 0
	// clear TCRR register
    OUTREG32(&pOWRGptInfo->pGpt->TCRR, 0x00000000);
#endif

	// Re-calculate MAT register
    temp = INREG32(&pOWRGptInfo->pGpt->TCRR); // 
    temp += 250;
	OUTREG32(&pOWRGptInfo->pGpt->TMAR, temp);

    // Set match register
    //OUTREG32(&pOWRGptInfo->pGpt->TMAR, 250); // ~ After 10 us
    
    // clear interrupts
    OUTREG32(&pOWRGptInfo->pGpt->TISR, 0x00000007);
    
    // enable (match)overflow interrupt
	//OUTREG32(&pOWRGptInfo->pGpt->TIER, GPTIMER_TIER_MATCH /*GPTIMER_TIER_OVERFLOW */);
	//OUTREG32(&pOWRGptInfo->pGpt->TIER, 0 );
 
    // enable wakeups
	//OUTREG32(&pOWRGptInfo->pGpt->TWER, GPTIMER_TWER_MATCH /*GPTIMER_TWER_OVERFLOW*/);
	//OUTREG32(&s_pGPT3->TWER, GPTIMER_TWER_MATCH );

	//  Start the timer.  Do not set for auto reload
    SETREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST | GPTIMER_TCLR_CE /*| GPTIMER_TCLR_AR*/);

#if 0
	//  Start the timer.  Also set for auto reload
    SETREG32(&pOWRGptInfo->pGpt->TCLR, GPTIMER_TCLR_ST /*| GPTIMER_TCLR_AR*/);
     while ((INREG32(pOWRGptInfo->pGpt->TWPS) & GPTIMER_TWPS_TCLR) != 0);
#endif

  //OWRISRMSG(OWR_ISR_DBG, (L"owrIsrReadBit  OK\r\n"));
  return(TRUE);

}