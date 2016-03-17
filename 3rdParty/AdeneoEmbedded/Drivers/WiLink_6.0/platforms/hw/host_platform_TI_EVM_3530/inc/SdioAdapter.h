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

/** \file   SdioAdapter.h 
 *  \brief  SDIO adapter module API definition                                  
 *
 *  \see    SdioAdapter.c
 */

#ifndef __SDIO_ADAPT_API_H__
#define __SDIO_ADAPT_API_H__


#include "TxnDefs.h"


/************************************************************************
 * Defines
 ************************************************************************/

/************************************************************************
 * Types
 ************************************************************************/

/************************************************************************
 * Functions
 ************************************************************************/

int Hw_Check();

/** \brief	sdioAdapt_ConnectBus: Init SDIO driver and HW
 * 
 * \param  fCbFunc       - The bus driver's callback upon async transaction completion
 * \param  hCbArg        - The CB function handle
 * \param  uBlkSizeShift - In block-mode:   BlkSize = (1 << uBlkSizeShift)
 * \param  uSdioThreadPriority - The SDIO interrupt handler thread priority
 * \param  pRxDmaBufAddr - Pointer for providing the Rx DMA buffer address to the upper layers to use it directly
 * \param  pRxDmaBufLen  - The Rx DMA buffer length in bytes
 * \param  pTxDmaBufAddr - Pointer for providing the Tx DMA buffer address to the upper layers to use it directly
 * \param  pTxDmaBufLen  - The Tx DMA buffer length in bytes
 * \return 0 = OK, otherwise = error
 * 
 * \par Description
 * Called by BusDrv to initialize the SDIO driver and HW.
 *
 * \sa
 */ 
int        sdioAdapt_ConnectBus    (void *        fCbFunc,
                                    void *        hCbArg,
                                    unsigned int  uBlkSizeShift,
                                    unsigned int  uSdioThreadPriority,
                                    unsigned char **pRxDmaBufAddr,
                                    unsigned int  *pRxDmaBufLen,
                                    unsigned char **pTxDmaBufAddr,
                                    unsigned int  *pTxDmaBufLen);

/** \brief	sdioAdapt_DisconnectBus: Disconnect SDIO driver
 * 
 * \param  void
 * \return 0 = OK, otherwise = error
 * 
 * \par Description
 * Called by BusDrv. Disconnect the SDIO driver.
 *
 * \sa
 */ 
int        sdioAdapt_DisconnectBus (void);
/** \brief	sdioAdapt_Transact: Process transaction
 * 
 * \param  uFuncId    - SDIO function ID (1- BT, 2 - WLAN)
 * \param  uHwAddr    - HW address where to write the data
 * \param  pHostAddr  - The data buffer to write from or read into
 * \param  uLength    - The data length in bytes
 * \param  bDirection - TRUE = Read,  FALSE = Write
 * \param  bBlkMode   - If TRUE - use block mode
 * \param  bMore      - If TRUE, more transactions are expected so don't turn off any HW
 * \return COMPLETE if Txn completed in this context, PENDING if not, ERROR if failed
 *
 * \par Description
 * Called by the BusDrv module to issue an SDIO transaction.
 * Call write or read SDIO-driver function according to the direction.
 * Use Sync or Async method according to the transaction length
 * 
 * \note   It's assumed that this function is called only when idle (i.e. previous Txn is done).
 * 
 * \sa
 */ 
ETxnStatus sdioAdapt_Transact      (unsigned int  uFuncId,
                                    unsigned int  uHwAddr,
                                    void *        pHostAddr,
                                    unsigned int  uLength,
                                    unsigned int  bDirection,
                                    unsigned int  bBlkMode,
                                    unsigned int  bFixedAddr,
                                    unsigned int  bMore);
/** \brief	sdioAdapt_TransactBytes: Process bytes transaction
 * 
 * \param  uFuncId    - SDIO function ID (1- BT, 2 - WLAN)
 * \param  uHwAddr    - HW address where to write the data
 * \param  pHostAddr  - The data buffer to write from or read into
 * \param  uLength    - The data length in bytes
 * \param  bDirection - TRUE = Read,  FALSE = Write
 * \param  bMore      - If TRUE, more transactions are expected so don't turn off any HW
 * \return COMPLETE if Txn succeeded, ERROR if failed
 *
 * \par Description
 * Called by the BusDrv module to issue a bytes stream SDIO transaction.
 * Call write or read SDIO-driver Sync function according to the direction.
 * 
 * \note   It's assumed that this function is called only when idle (i.e. previous Txn is done).
 * 
 * \sa
 */ 
ETxnStatus sdioAdapt_TransactBytes (unsigned int  uFuncId,
                                    unsigned int  uHwAddr,
                                    void *        pHostAddr,
                                    unsigned int  uLength,
                                    unsigned int  bDirection,
                                    unsigned int  bMore);



#endif /*__SDIO_ADAPT_API_H__*/
