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

/***************************************************************************/
/*																		   */
/*	  MODULE:	measurementMgrSM.h										   */
/*    PURPOSE:	measurement Manager module State Machine header file       */
/*																		   */
/***************************************************************************/

#ifndef __MEASUREMENTMGRSM_H__
#define __MEASUREMENTMGRSM_H__

#include "fsm.h"



/** State machine states */
typedef enum 
{
	MEASUREMENTMGR_STATE_IDLE					= 0,
    MEASUREMENTMGR_STATE_PROCESSING_REQUEST		= 1,
    MEASUREMENTMGR_STATE_WAITING_FOR_SCR		= 2,
    MEASUREMENTMGR_STATE_MEASURING				= 3,
	MEASUREMENTMGR_STATE_LAST					= 4
} measurementMgrSM_States;


/** State machine events */
typedef enum 
{
	MEASUREMENTMGR_EVENT_CONNECTED				= 0,
	MEASUREMENTMGR_EVENT_DISCONNECTED			= 1,
	MEASUREMENTMGR_EVENT_ENABLE					= 2,
	MEASUREMENTMGR_EVENT_DISABLE				= 3,
    MEASUREMENTMGR_EVENT_FRAME_RECV				= 4,
    MEASUREMENTMGR_EVENT_SEND_REPORT			= 5,
	MEASUREMENTMGR_EVENT_REQUEST_SCR			= 6,
    MEASUREMENTMGR_EVENT_SCR_WAIT				= 7,
    MEASUREMENTMGR_EVENT_SCR_RUN				= 8,
    MEASUREMENTMGR_EVENT_ABORT					= 9,
    MEASUREMENTMGR_EVENT_COMPLETE				= 10,
    MEASUREMENTMGR_EVENT_FW_RESET				= 11,
	MEASUREMENTMGR_EVENT_LAST					= 12
} measurementMgrSM_Events;


#define MEASUREMENTMGR_NUM_STATES		MEASUREMENTMGR_STATE_LAST	 
#define MEASUREMENTMGR_NUM_EVENTS		MEASUREMENTMGR_EVENT_LAST	




TI_STATUS measurementMgrSM_config(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgrSM_event(TI_UINT8 * currentState, TI_UINT8 event, TI_HANDLE hMeasurementMgr);


#endif /* __MEASUREMENTMGRSM_H__*/
