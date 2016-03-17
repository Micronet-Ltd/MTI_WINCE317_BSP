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
/*	  MODULE:	txPort_Api.h										       */
/*    PURPOSE:	Tx-Port module API										   */
/*																		   */
/***************************************************************************/
#ifndef _TX_PORT_API_H_
#define _TX_PORT_API_H_

#include "commonTypes.h"
#include "DrvMainModules.h"


/*
 *  Tx-Port module public functions:
 */
TI_HANDLE txPort_create (TI_HANDLE hOs);
TI_STATUS txPort_unLoad (TI_HANDLE hTxPort);
void txPort_init        (TStadHandlesList *pStadHandles);
void txPort_enableData  (TI_HANDLE hTxPort);
void txPort_enableMgmt	(TI_HANDLE hTxPort);
void txPort_suspendTx	(TI_HANDLE hTxPort);
void txPort_resumeTx	(TI_HANDLE hTxPort);


#endif /* _TX_PORT_API_H_ */
