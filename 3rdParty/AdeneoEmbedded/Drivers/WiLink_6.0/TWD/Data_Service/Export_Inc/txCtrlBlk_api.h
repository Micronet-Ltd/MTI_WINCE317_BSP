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
/*	  MODULE:	txCtrlBlk_api.h											       */
/*    PURPOSE:	Tx control block module API.							   */
/*																		   */
/***************************************************************************/
#ifndef _TX_CTRL_BLK_API_H_
#define _TX_CTRL_BLK_API_H_


#include "TWDriver.h"

/* Public Function Definitions */
TI_HANDLE	txCtrlBlk_Create    (TI_HANDLE hOs);
TI_STATUS	txCtrlBlk_Destroy   (TI_HANDLE hTxCtrlBlk);
TI_STATUS   txCtrlBlk_Init      (TI_HANDLE hTxCtrlBlk, TI_HANDLE hReport, TI_HANDLE hContext);
TTxCtrlBlk *txCtrlBlk_Alloc     (TI_HANDLE hTxCtrlBlk);
void		txCtrlBlk_Free      (TI_HANDLE hTxCtrlBlk, TTxCtrlBlk *pCurrentEntry);
TTxCtrlBlk *txCtrlBlk_GetPointer(TI_HANDLE hTxCtrlBlk, TI_UINT8 descId);
#ifdef TI_DBG
void		txCtrlBlk_PrintTable(TI_HANDLE hTxCtrlBlk);
#endif /* TI_DBG */


#endif  /* _TX_CTRL_BLK_API_H_ */
		

