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
 *   MODULE:  txResult_api.h
 *
 *   PURPOSE: Tx result module API.
 * 
 ****************************************************************************/

#ifndef _TX_RESULT_API_H
#define _TX_RESULT_API_H


#include "TWDriverInternal.h"


/* Public Function Definitions */

TI_HANDLE	txResult_Create (TI_HANDLE hOs);
TI_STATUS	txResult_Destroy (TI_HANDLE hTxResult);
TI_STATUS   txResult_Init (TI_HANDLE hTxResult, TI_HANDLE hReport, TI_HANDLE hTwIf);
void		txResult_setHwInfo (TI_HANDLE hTxResult, TDmaParams *pDmaParams);
void		txResult_RegisterCb (TI_HANDLE hTxResult, TI_UINT32 CallBackID, void *CBFunc, TI_HANDLE CBObj);
ETxnStatus         txResult_TxCmpltIntrCb (TI_HANDLE hTxResult, FwStatus_t *pFwStatus);

#ifdef TI_DBG
void		txResult_PrintInfo (TI_HANDLE hTxResult);
void		txResult_ClearInfo (TI_HANDLE hTxResult);
#endif


#endif /* _TX_RESULT_API_H */

