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
/** \file   smeSm.h
 *  \brief SME SM internal header file
 *
 *  \see smeSm.c
 */

#ifndef __SME_SM_H__
#define __SME_SM_H__

#include "GenSM.h"

void sme_SmEvent(TI_HANDLE hGenSm, TI_UINT32 uEvent, void* pData);

typedef enum
{
    SME_SM_STATE_IDLE = 0,
    SME_SM_STATE_WAIT_CONNECT,
    SME_SM_STATE_SCANNING,
    SME_SM_STATE_CONNECTING,
    SME_SM_STATE_CONNECTED,
    SME_SM_STATE_DISCONNECTING,
    SME_SM_NUMBER_OF_STATES
} ESmeSmStates;

typedef enum
{
    SME_SM_EVENT_START = 0,
    SME_SM_EVENT_STOP,
    SME_SM_EVENT_CONNECT,
    SME_SM_EVENT_CONNECT_SUCCESS,
    SME_SM_EVENT_CONNECT_FAILURE,
    SME_SM_EVENT_DISCONNECT,
    SME_SM_NUMBER_OF_EVENTS
} ESmeSmEvents;

extern TGenSM_actionCell tSmMatrix[ SME_SM_NUMBER_OF_STATES ][ SME_SM_NUMBER_OF_EVENTS ];
extern TI_INT8*  uStateDescription[];
extern TI_INT8*  uEventDescription[];

#endif /* __SME_SM_H__ */


