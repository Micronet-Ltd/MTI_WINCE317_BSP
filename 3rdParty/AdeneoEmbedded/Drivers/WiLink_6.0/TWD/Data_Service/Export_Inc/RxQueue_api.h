/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/


/** \file   RxQueue_api.h 
 *  \brief  RxQueue module header file.                                  
 *
 *  \see    RxQueue_api.c
 */

#ifndef _RX_QUEUE_H_
#define _RX_QUEUE_H_

/* 
 * External Functions Prototypes 
 * ============================= 
 */
TI_HANDLE RxQueue_Create        (TI_HANDLE hOs);
TI_STATUS RxQueue_Destroy       (TI_HANDLE hRxQueue);
TI_STATUS RxQueue_Init          (TI_HANDLE hRxQueue, TI_HANDLE hReport, TI_HANDLE hTimerModule);
void      RxQueue_CloseBaSession(TI_HANDLE hRxQueue, TI_UINT8 uFrameTid);
void      RxQueue_ReceivePacket (TI_HANDLE hRxQueue, const void *aFrame);
void      RxQueue_Register_CB   (TI_HANDLE hRxQueue, TI_UINT32 CallBackID, void *CBFunc, TI_HANDLE CBObj);


#endif  /* _STA_CAP_H_ */


