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
/*		MODULE:	admCtrl.c												   */
/*    PURPOSE:	WSM/WME admission Control							   */
/*																	 	   */
/***************************************************************************/

#ifndef _ADM_CTRL_SM_H
#define _ADM_CTRL_SM_H

#include "fsm.h"
#include "qosMngr_API.h"
#include "mlmeApi.h"

#define INITIAL_DIALOG_TOKEN	1

/* state machine states */
typedef enum 
{
	TRAFFIC_ADM_CTRL_SM_STATE_IDLE			= 0,
	TRAFFIC_ADM_CTRL_SM_STATE_WAIT   		= 1
} trafficAdmCtrl_smState_t;

/* State machine inputs */
typedef enum 
{
	TRAFFIC_ADM_CTRL_SM_EVENT_START			= 0,
	TRAFFIC_ADM_CTRL_SM_EVENT_STOP			= 1,
	TRAFFIC_ADM_CTRL_SM_EVENT_ACCEPT    	= 2,
    TRAFFIC_ADM_CTRL_SM_EVENT_REJECT		= 3,
   	TRAFFIC_ADM_CTRL_SM_EVENT_TIMEOUT		= 4
} trafficAdmCtrl_smEvents_t;


typedef struct
{
	TI_UINT8					tsInfo[3];
	trafficAdmCtrl_smState_t	rafficAdmCtrlSmCurrState;
}tidInfo_t;


typedef struct
{
	TI_HANDLE			        hTxMgmtQ;
	TI_HANDLE			        hReport;
	TI_HANDLE			        hOs;
	TI_HANDLE			        hQosMngr;
	TI_HANDLE					hCtrlData;
	TI_HANDLE					hXCCMgr;
	TI_HANDLE					hTimer;
	TI_HANDLE					hTWD;
	TI_HANDLE					hTxCtrl;
	TI_HANDLE					hRoamMng;
	TI_HANDLE					hEvHandler;

	TI_HANDLE			        hAdmCtrlTimer[MAX_NUM_OF_AC];
	TI_UINT8				    currentState[MAX_NUM_OF_AC];
	TI_UINT8				    dialogToken[MAX_NUM_OF_AC];

	TI_UINT8				    dialogTokenCounter;


	fsm_stateMachine_t	        *pTrafficAdmCtrlSm;

	TI_UINT32				     timeout;
    TI_BOOL                      useFixedMsduSize;


} trafficAdmCtrl_t;




TI_HANDLE trafficAdmCtrl_create(TI_HANDLE pOs);

TI_STATUS trafficAdmCtrl_unload(TI_HANDLE hTrafficAdmCtrl);

TI_STATUS trafficAdmCtrl_config (TI_HANDLE hTrafficAdmCtrl,
    						     TI_HANDLE hTxMgmtQ,
    						     TI_HANDLE hReport,
    						     TI_HANDLE hOs,
    						     TI_HANDLE hQosMngr,
    						     TI_HANDLE hCtrlData,
    						     TI_HANDLE hXCCMgr,
    						     TI_HANDLE hTimer,
    						     TI_HANDLE hTWD,
                                 TI_HANDLE hTxCtrl,
                                 TI_HANDLE hRoamMng,
                                 TI_HANDLE hEvHandler,
    						     trafficAdmCtrlInitParams_t	*pTrafficAdmCtrlInitParams);

TI_STATUS trafficAdmCtrl_setParam(TI_HANDLE hTrafficAdmCtrl, paramInfo_t	*pParam);

TI_STATUS trafficAdmCtrl_getParam(TI_HANDLE hTrafficAdmCtrl, paramInfo_t	*pParam);

TI_STATUS trafficAdmCtrl_startAdmRequest(TI_HANDLE	hTrafficAdmCtrl, tspecInfo_t *pTSpecInfo);

TI_STATUS trafficAdmCtrl_stopAdmRequest(TI_HANDLE hTrafficAdmCtrl, TI_UINT8 tid);

TI_STATUS trafficAdmCtrl_stop(TI_HANDLE hTrafficAdmCtrl);

TI_STATUS trafficAdmCtrl_recv(TI_HANDLE hTrafficAdmCtrl, TI_UINT8* pData, TI_UINT8 action);

TI_STATUS trafficAdmCtrl_sendDeltsFrame(TI_HANDLE hTrafficAdmCtrl, tspecInfo_t *pTSpecInfo, TI_UINT8 reasonCode);

void trafficAdmCtrl_print(trafficAdmCtrl_t *pTrafficAdmCtr);

void trafficAdmCtrl_buildTSPec(trafficAdmCtrl_t	*pTrafficAdmCtrl, 
							   tspecInfo_t		*pTSpecInfo, 
							   TI_UINT8				*pDataBuff,
							   TI_UINT32			*len);
void trafficAdmCtrl_parseTspecIE(tspecInfo_t *pTSpecInfo, TI_UINT8 *pData);
#endif

