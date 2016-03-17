/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module gpio_isr
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   7-Dec-2009
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef __INST_ISR_H
#define __INST_ISR_H

#ifdef __cplusplus
extern "C" {
#endif



//==============================================================================================
//            local definitions
//==============================================================================================


#define OWR_IDENTIFIER                          L"OWR1:"

#define 	OWR_QUEUE_NAME_LEN   64




/*
typedef enum {
	INVALID_NOTIFY_STATE = 0,
    OPENED,
    REGISTER,
	WAITING_FOR_EVENT
} INPUT_NOTIFY_STATE;
*/

typedef enum {
	NO_INST_ISR = 0,
	GPIO_ISR,
    GPTIMER_ISR,
} INSTALLABLE_ISR_TYPE;

typedef DWORD (*FPTR_INST_ISR)(VOID *pIsrInfo);

typedef struct _INSTALLABLE_ISR_INFO 
{
	INSTALLABLE_ISR_TYPE    isrType;      // GPIO, TIMER, etc
	VOID                    *pIsrInfo;     // ptr to ISR Info object
	FPTR_INST_ISR            fPrtIsrHandler;
} INSTALLABLE_ISR_INFO;



 #ifdef __cplusplus
}
#endif

#endif //__INST_ISR_H

