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
/*	  MODULE:	txMgmtQueue_Api.h									       */
/*    PURPOSE:	Tx Mgmt Queue module API Header file					   */
/*																		   */
/***************************************************************************/
#ifndef _TX_MGMT_QUEUE_API_H_
#define _TX_MGMT_QUEUE_API_H_

#include "DrvMainModules.h"


#define MGMT_QUEUES_DEPTH	4	/* Up to 4 packets per queue. */


/* Tx connection states updated by the connection SM. */
typedef enum
{
	TX_CONN_STATE_CLOSE,	/* Tx closed for all packets. */
	TX_CONN_STATE_MGMT,		/* Tx open only for mgmt packets. */
	TX_CONN_STATE_EAPOL,	/* Tx open only for mgmt or EAPOL packets. */
	TX_CONN_STATE_OPEN		/* Tx open for all packets. */
} ETxConnState;



/* 
 *  The module public functions:
 */
TI_HANDLE	txMgmtQ_Create (TI_HANDLE hOs);
void        txMgmtQ_Init (TStadHandlesList *pStadHandles);
TI_STATUS	txMgmtQ_Destroy (TI_HANDLE hTxMgmtQ);
void        txMgmtQ_ClearQueues (TI_HANDLE hTxMgmtQ);
TI_STATUS   txMgmtQ_Xmit (TI_HANDLE hTxMgmtQ, TTxCtrlBlk *pPktCtrlBlk, TI_BOOL bExternalContext);
void        txMgmtQ_QueuesNotEmpty (TI_HANDLE hTxMgmtQ);
void		txMgmtQ_StopQueue (TI_HANDLE hTxMgmtQ, TI_UINT32 tidBitMap);
void		txMgmtQ_UpdateBusyMap (TI_HANDLE hTxMgmtQ, TI_UINT32 tidBitMap);
void		txMgmtQ_StopAll (TI_HANDLE hTxMgmtQ);
void		txMgmtQ_WakeAll (TI_HANDLE hTxMgmtQ);
void		txMgmtQ_SetConnState (TI_HANDLE hTxMgmtQ, ETxConnState txConnState);

ETxConnState  txMgmtQ_GetConnState (TI_HANDLE hTxMgmtQ);

#ifdef TI_DBG
void        txMgmtQ_PrintModuleParams (TI_HANDLE hTxMgmtQ);
void        txMgmtQ_PrintQueueStatistics (TI_HANDLE	hTxMgmtQ);
void        txMgmtQ_ResetQueueStatistics (TI_HANDLE	hTxMgmtQ);
#endif


#endif /* _TX_MGMT_QUEUE_API_H_ */
