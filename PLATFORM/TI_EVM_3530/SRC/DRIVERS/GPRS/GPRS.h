// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File:  GPRSDrvr.h
//
//   GPRS Driver header
//  
//
//   Created by Anna Rayer  September 2010
//-----------------------------------------------------------------------------/*

#ifndef __GPRS_H
#define __GPRS_H

#ifdef __cplusplus
extern "C" {
#endif



//==============================================================================================
//            local definitions
//==============================================================================================


#define GPRS_IDENTIFIER                           L"COM0:"

//#define MODEM_ON_OFF_EVENT                        L"ModemOnOffEvent"

#define PCA_GPIO_GPRS_0                        PCA_GPIO_80

#define MODEM_ID_IN_0                            (PCA_GPIO_GPRS_0)
#define MODEM_ID_IN_1                            (PCA_GPIO_GPRS_0+1)
#define MODEM_ID_IN_2                            (PCA_GPIO_GPRS_0+2)
#define MODEM_ID_IN_3                            (PCA_GPIO_GPRS_0+3)

#define MODEM_ID_IN_4                            (PCA_GPIO_GPRS_0+4)
#define MODEM_ID_IN_5                            (PCA_GPIO_GPRS_0+5)
#define MODEM_ID_IN_6                            (PCA_GPIO_GPRS_0+6)
#define UART_RING_2V8_IN                         (PCA_GPIO_GPRS_0+7) 

#define GPRS_POWER_EN_OUT                        (PCA_GPIO_GPRS_0+8) 
#define TEST_OUT                                 (PCA_GPIO_GPRS_0+9)
#define nRESET_FT232RQ_OUT                       (PCA_GPIO_GPRS_0+10)
#define ON_OFF_PULSE_OUT                         (PCA_GPIO_GPRS_0+11) 

#define RESET_GE864_OUT                          (PCA_GPIO_GPRS_0+12)
//#define PCM_ENABLE_OUT                           (PCA_GPIO_GPRS_0+13)
//#define PCM_MASTER_SLAVE_OUT                     (PCA_GPIO_GPRS_0+14) 
#define PWRMON_IN                                (PCA_GPIO_GPRS_0+15)

// NBOARD816V3
#define GPRS_LOW_CHARGE                          GPIO_154
#define GPRS_V4V0V_EN                            GPIO_153

//==============================================================================================
//            IOControls definitions
//==============================================================================================

#define     IOCTL_SET_MODEM_POWER_STATE            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300,METHOD_BUFFERED,FILE_ANY_ACCESS)	
#define     IOCTL_SET_MODEM_CONFIG                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define     IOCTL_GET_MODEM_POWER_STATE            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define     IOCTL_SET_MUX_STATUS				   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define		IOCTL_GET_EXTERNAL_MODEM_STATE         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

 #ifdef __cplusplus
}
#endif

#endif
