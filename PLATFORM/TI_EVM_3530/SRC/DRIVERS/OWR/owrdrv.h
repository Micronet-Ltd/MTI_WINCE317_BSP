/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module OWR
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

#ifndef __OWRDRV_H
#define __OWRDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "owr.h"
#include "inst_isr.h"
#include "gpt_isr.h"
#include "gpio_isr.h"
#include "owr_isr.h"
#include "inst_isr_dbg.h"

//#include "iButton.h"

#define   OWR_ISR_DBG        0
#define   OWR_DRV_DBG        0


#define     USER_IOCTL(X)                         (IOCTL_KLIB_USER + (X))
#define     IOCTL_ISR_GPIO_INFO                   USER_IOCTL(12)
#define     IOCTL_ISR_GPIO_UNLOAD                 USER_IOCTL(13)

#define     IOCTL_ISR_TIMER_INFO                  USER_IOCTL(14)
#define     IOCTL_ISR_TIMER_UNLOAD                USER_IOCTL(15)

#define     IOCTL_ISR_GPIO_CFG                    USER_IOCTL(16)
#define     IOCTL_ISR_TIMER_CFG                   USER_IOCTL(17)

#define     IOCTL_ISR_TEST_1                      USER_IOCTL(18)

#define     IOCTL_ISR_GPIO_GET_DATA               USER_IOCTL(19)
#define     IOCTL_ISR_TIMER_GET_DATA              USER_IOCTL(20)

#define     IOCTL_ISR_OWR_RESET_PRESENCE          USER_IOCTL(21) 
#define     IOCTL_ISR_OWR_ROM_CMD                 USER_IOCTL(22)
#define     IOCTL_ISR_OWR_READ                    USER_IOCTL(23) 
#define     IOCTL_ISR_OWR_DETECT_PULSE            USER_IOCTL(24)

//==============================================================================================
//            local definitions
//==============================================================================================

//------------------------------------------------------------------------------

typedef enum{
	i_BUTTON_SIMPLE_MODE = 0,
	i_BUTTON_EXT_MODE,

} OWR_i_BUTTON_MODE;


typedef struct _ONE_WIRE_iBUTTON
{
 OWR_i_BUTTON_MODE    iButtonMode;
 TCHAR                iButtonPrefix[4];
 TCHAR                iButtonSuffix[4];
 
}ONE_WIRE_iBUTTON;

typedef struct _ONE_WIRE_TEMPR_SENSOR
{
 TCHAR                temprSensorName[32];
 
}ONE_WIRE_TEMPR_SENSOR;


typedef struct _OMAP_GPIO_ONE_WIRE
{
 OMAP_GPIO_ISR_INFO     owrOmapGpio;
 OMAP_GPT_ISR_INFO      owrOmapTimer;
 ONE_WIRE_iBUTTON       owriButton;
 ONE_WIRE_TEMPR_SENSOR  owrTemprSensor;

 HANDLE                 owrIsrHandle_gpio;
 HANDLE                 owrIsrHandle_gpt;

 HANDLE                 owrResetPresenceEvent;
 HANDLE                 owrWriteEvent;
 HANDLE                 owrReadEvent;

}OMAP_GPIO_ONE_WIRE;


BOOL gpTimerInit(OMAP_GPT_ISR_INFO *pGPTimerInfo);
void gpTimerDeInit(OMAP_GPT_ISR_INFO *pGPTimerInfo);
BOOL GPTInterruptInitialize(OMAP_GPT_ISR_INFO *pGPTimerInfo);
void GPTInterruptDone(OMAP_GPT_ISR_INFO *pGPTimerInfo);

BOOL owrGPIOInit(OMAP_GPIO_ISR_INFO *pOWRGPIOInfo);
void owrGPIODeInit(OMAP_GPIO_ISR_INFO *pOWRGpioInfo);

BOOL owrResetPresence(OMAP_GPIO_ONE_WIRE *pOWR_OMAP);
BOOL owrWrite(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 data[], UINT32 dataLen);
BOOL owrRead(OMAP_GPIO_ONE_WIRE *pOWR_OMAP, UINT8 data[], UINT32 dataLen);

 #ifdef __cplusplus
}
#endif

#endif
