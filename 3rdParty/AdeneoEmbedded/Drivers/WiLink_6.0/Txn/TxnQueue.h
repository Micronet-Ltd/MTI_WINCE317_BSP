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

/** \file   TxnQueue.h 
 *  \brief  TxnQueue module API definition                                  
 *
 *  \see    TxnQueue.c
 */

#ifndef __TXN_QUEUE_API_H__
#define __TXN_QUEUE_API_H__


#include "TxnDefs.h"
#include "BusDrv.h"



/************************************************************************
 * Defines
 ************************************************************************/


/************************************************************************
 * Macros
 ************************************************************************/


/************************************************************************
 * Types
 ************************************************************************/


/************************************************************************
 * Functions
 ************************************************************************/
/** \brief	Create the TxnQ module
 * 
 * \param  hOs - Handle to Os Abstraction Layer
 * \return Handle of the allocated object, NULL if allocation failed 
 * 
 * \par Description
 * Allocate and clear the module's object
 *
 * \sa	txnQ_Destroy
 */ 
TI_HANDLE   txnQ_Create (TI_HANDLE hOs);

/** \brief	Destroy the module
 * 
 * \param  The module's object
 * \return TI_OK on success or TI_NOK on failure 
 * 
 * \par Description
 * Destroy bus driver and free the module's object
 *
 * \sa	txnQ_Create
 */ 
TI_STATUS   txnQ_Destroy (TI_HANDLE hTxnQ);

/** \brief	Init module
 * 
 * \param  hTxnQ     - The module's object
 * \param  hOs       - Handle to Os Abstraction Layer
 * \param  hReport   - Handle to report module
 * \param  hContext  - Handle to context module
 * \return void        
 * 
 * \par Description
 *
 * \sa
 */ 
void        txnQ_Init (TI_HANDLE hTxnQ, 
					   TI_HANDLE hOs, 
					   TI_HANDLE hReport, 
					   TI_HANDLE hContext);

/** \brief	Configure bus driver
 * 
 * \param  hTxnQ      - The module's object
 * \param  pBusDrvCfg - A union used for per-bus specific configuration. 
 * \param  pRxDmaBufLen - The Rx DMA buffer length in bytes (needed as a limit of the Rx aggregation length)
 * \param  pTxDmaBufLen - The Tx DMA buffer length in bytes (needed as a limit of the Tx aggregation length)
 * \return TI_OK / TI_NOK
 * 
 * \par Description
 * Called by DrvMain (future - by Chip-Manager).
 * Configure the bus driver with its connection configuration (such as baud-rate, bus width etc) 
 * and establish the physical connection. Done once (and not per functional driver startup). 
 * 
 * \sa
 */ 
TI_STATUS   txnQ_ConnectBus (TI_HANDLE  hTxnQ, 
							 TBusDrvCfg *pBusDrvCfg,
							 TTxnDoneCb fConnectCb,
							 TI_HANDLE  hConnectCb, 
                             TI_UINT32  *pRxDmaBufLen,
                             TI_UINT32  *pTxDmaBufLen);

/** \brief	Disconnect bus driver
 * 
 * \param  hTxnQ      - The module's object
 * \return TI_OK / TI_NOK
 * 
 * \par Description
 * Called by DrvMain (future - by Chip-Manager).
 * Disconnect the bus driver.
 * 
 * \sa
 */ 
TI_STATUS   txnQ_DisconnectBus (TI_HANDLE hTxnQ);

/** \brief	Register functional driver to TxnQ
 * 
 * \param  hTxnQ           - The module's object
 * \param  uFuncId         - The calling functional driver
 * \param  uNumPrios       - The number of queues/priorities
 * \param  fTxnQueueDoneCb - The callback to call upon full transaction completion.
 * \param  hCbHandle       - The callback handle                              
 * \return TI_OK / TI_NOK
 * 
 * \par Description
 * Called by each functional driver (WLAN, future-BT) that uses the TxnQ.
 * Save driver's info and create its queues. 
 * Perform in critical section to prevent preemption from TxnDone.
 * 
 * \sa	txnQ_Close
 */ 
TI_STATUS   txnQ_Open (TI_HANDLE       hTxnQ, 
                       TI_UINT32       uFuncId, 
                       TI_UINT32       uNumPrios, 
                       TTxnQueueDoneCb fTxnQueueDoneCb,
                       TI_HANDLE       hCbHandle);

/** \brief	Unregister functional driver from TxnQ
 * 
 * \param  hTxnQ      - The module's object
 * \param  uFuncId    - The calling functional driver
 * \return void
 * \sa     txnQ_Open
 * 
 * \par Description
 * Called by registered functional driver that uses the TxnQ to unregister.
 * Clear the function's data and destroy its queues. 
 * Perform in critical section to prevent preemption from TxnDone.
 * 
 * \sa	txnQ_Open
 */ 
void        txnQ_Close (TI_HANDLE  hTxnQ, TI_UINT32 uFuncId);

/** \brief	Restart caller's queues
 * 
 * \param  hTxnQ      - The module's object
 * \param  uFuncId    - The calling functional driver
 * \return COMPLETE if queues were restarted, PENDING if waiting for TxnDone to restart queues
 * 
 * \par Description
 * Called upon functional driver stop command or upon recovery. 
 * If no transaction in progress for the calling function, clear its queues (call the CBs). 
 * If a transaction from this function is in progress, just set state to RESTART and when 
 * called back upon TxnDone clear the queues.
 * Perform in critical section to prevent preemption from TxnDone.
 * \note   
 * The Restart applies only to the calling function's queues.
 * 
 * \sa	txnQ_ClearQueues
 */ 
ETxnStatus  txnQ_Restart (TI_HANDLE hTxnQ, TI_UINT32 uFuncId);

/** \brief	Run caller's queues
 * 
 * \param  hTxnQ   - The module's object
 * \param  uFuncId - The calling functional driver
 * \return void
 * 
 * \par Description
 * Enable TxnQ scheduler to process transactions from the calling function's queues.
 * Run scheduler to issue transactions as possible.
 * Run in critical section to protect from preemption by TxnDone.
 * 
 * \sa	txnQ_ClearQueues
 */ 
void        txnQ_Run (TI_HANDLE hTxnQ, TI_UINT32 uFuncId);

/** \brief	Stop caller's queues
 * 
 * \param  hTxnQ   - The module's object
 * \param  uFuncId - The calling functional driver
 * \return void
 * 
 * \par Description
 * Disable TxnQ scheduler to process transactions from the calling function's queues.
 * 
 * \sa
 */ 
void        txnQ_Stop (TI_HANDLE hTxnQ, TI_UINT32 uFuncId);

/** \brief	Issue a new transaction
 * 
 * \param  hTxnQ - The module's object
 * \param  pTxn  - The transaction object 
 * \return COMPLETE if input pTxn completed in this context, PENDING if not, ERROR if failed
 * 
 * \par Description
 * Called by the functional driver to initiate a new transaction.
 * In critical section save transaction and call scheduler.
 * 
 * \sa
 */ 
ETxnStatus  txnQ_Transact (TI_HANDLE hTxnQ, TTxnStruct *pTxn);

/** 
 * \fn     txnQ_ClearQueues
 * \brief  Clear the function queues
 * 
 * Clear the specified function queues and call its CB for each Txn with status=RECOVERY.
 * 
 * \note   Called in critical section.
 * \param  hTxnQ            - The module's object
 * \param  uFuncId          - The calling functional driver
 * \return void
 * \sa     
 */ 
void        txnQ_ClearQueues (TI_HANDLE hTxnQ, TI_UINT32 uFuncId);


#ifdef TI_DBG
/** \brief	Print Txn Queues
 * 
 * \param  hTxnQ - The module's object
 * \return void
 * 
 * \par Description
 * 
 * \sa
 */ 
void txnQ_PrintQueues (TI_HANDLE hTxnQ);
#endif



#endif /*__TXN_QUEUE_API_H__*/
