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

#ifndef __GPIO_ISR_H
#define __GPIO_ISR_H

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

typedef struct _OMAP_GPIO_MODULE_INFO 
{
	UINT      gpioModuleNum;
	UINT      gpioModuleIrq;
	UINT      gpioModulePA;
} OMAP_GPIO_MODULE_INFO;

//HANDLE              hGpio;
typedef struct _OMAP_GPIO_ISR_INFO 
{
    HANDLE    hGpio;
	UINT      input;    // GPIO_163
	UINT      output;   // GPIO_164
	UINT      irqNum;   //IRQ_GPIO6_MPU
	OMAP_GPIO_REGS  *pGpio;
	DWORD     sysIntr;   //
	//BYTE      vIRQ;      // 
	HANDLE    hGPIOEvent;

    OMAP_GPIO_MODULE_INFO   gpioModule;

} OMAP_GPIO_ISR_INFO, *POMAP_GPIO_ISR_INFO;


OMAP_GPIO_ISR_INFO *gpio2Cfg(OMAP_GPIO_ISR_INFO *pOWRGpioInfo);
DWORD gpio2Info(void);
DWORD gpio2ISRHandler( VOID *pIsrInfo );

OMAP_GPIO_ISR_INFO *gpio163Cfg(OMAP_GPIO_ISR_INFO *pOWRGpioInfo);
DWORD gpio163Info(void);
DWORD gpio163ISRHandler( VOID *pIsrInfo );

// Get pointer to OWR GPIO data
OMAP_GPIO_ISR_INFO *getOWRGpio(void);

 #ifdef __cplusplus
}
#endif

#endif //__GPIO_ISR_H

