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

/****************************************************************************
 *
 *   MODULE:  txHwQueue_api.h
 *
 *   PURPOSE: HW Tx Queue module API.
 * 
 ****************************************************************************/

#ifndef _TX_HW_QUEUE_API_H
#define _TX_HW_QUEUE_API_H


#include "TWDriver.h"
#include "TWDriverInternal.h"


/* Public Function Definitions */

TI_HANDLE	txHwQueue_Create (TI_HANDLE hOs);
TI_STATUS	txHwQueue_Destroy (TI_HANDLE hTxHwQueue);
TI_STATUS	txHwQueue_Init (TI_HANDLE hTxHwQueue, TI_HANDLE hReport);
TI_STATUS	txHwQueue_Config (TI_HANDLE hTxHwQueue, TTwdInitParams *pInitParams);
TI_STATUS	txHwQueue_SetHwInfo (TI_HANDLE hTxHwQueue, TDmaParams *pDmaParams);
TI_STATUS	txHwQueue_Restart (TI_HANDLE hTxHwQueue);
ETxHwQueStatus txHwQueue_AllocResources (TI_HANDLE hTxHwQueue, TTxCtrlBlk *pTxCtrlBlk);
ETxnStatus         txHwQueue_UpdateFreeResources (TI_HANDLE hTxHwQueue, FwStatus_t *pFwStatus);
void        txHwQueue_RegisterCb (TI_HANDLE hTxHwQueue, TI_UINT32 uCallBackId, void *fCbFunc, TI_HANDLE hCbHndl);
#ifdef TI_DBG
void		txHwQueue_PrintInfo (TI_HANDLE hTxHwQueue);
#endif /* TI_DBG */


#endif /* _TX_HW_QUEUE_API_H */




