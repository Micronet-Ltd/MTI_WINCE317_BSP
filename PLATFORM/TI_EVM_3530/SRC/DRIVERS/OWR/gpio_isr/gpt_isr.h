/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module gpt_isr
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

#ifndef __GPT_ISR_H
#define __GPT_ISR_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _OMAP_GPT_ISR_INFO 
{
    UINT32               gpTimerNum;
	UINT                 gpTimerIrQNum;
    UINT                 gpTimer_clkId;  // kGPT3_ALWON_FCLK
    UINT                 gpTimer_newParentClkId;  //kSYS_CLK
    UINT                 gpTimer_devId;  //OMAP_DEVICE_GPTIMER3
	UINT32               gpTimerRegPA; 
	OMAP_GPTIMER_REGS *pGpt;          //OMAP_GPTIMERx_REGS_UA
	OMAP_INTC_MPU_REGS *pICL;          //
	DWORD                 sysIntr;       //

	HANDLE               hGPTEvent;

	// Dummy Event
    UINT                 gpTimerDummyIrQNum;
	DWORD                dummySysIntr;
	HANDLE               hGPTDummyEvent;

} OMAP_GPT_ISR_INFO, *POMAP_GPT_ISR_INFO;

BOOL owrGPTInit(OMAP_GPT_ISR_INFO *pOWRGPTInfo);
OMAP_GPT_ISR_INFO *gpt4Cfg(OMAP_GPT_ISR_INFO *pOWRGptInfo);
DWORD gpt4Info(void);
DWORD gpt4ISRHandler( VOID *pIsrInfo );

OMAP_GPT_ISR_INFO *getOWRGpt(void);

 #ifdef __cplusplus
}
#endif

#endif //__GPT_ISR_H

