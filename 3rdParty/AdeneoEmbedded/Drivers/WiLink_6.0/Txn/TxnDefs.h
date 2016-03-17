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

/** \file   TxnDefs.h 
 *  \brief  Common TXN definitions                                  
 *
 * These defintions are used also by the SDIO/SPI adapters, so they shouldn't
 *     base on any non-standart types (e.g. use unsigned int and not TI_UINT32)
 * 
 *  \see    
 */

#ifndef __TXN_DEFS_API_H__
#define __TXN_DEFS_API_H__


/************************************************************************
 * Defines
 ************************************************************************/
#define TXN_FUNC_ID_CTRL         0
#define TXN_FUNC_ID_BT           1
#define TXN_FUNC_ID_WLAN         2


/************************************************************************
 * Types
 ************************************************************************/
/* Transactions status (shouldn't override TI_OK and TI_NOK values) */
/** \enum ETxnStatus
 * \brief  Txn Transactions status
 *
 * \par Description
 * Txn Transactions status - shouldn't override TI_OK and TI_NOK values
 *
 * \sa
 */
typedef enum
{
    TXN_STATUS_NONE = 2,	/**< */
    TXN_STATUS_OK,         	/**< */
    TXN_STATUS_COMPLETE,  	/**< */ 
    TXN_STATUS_PENDING,    	/**< */
    TXN_STATUS_ERROR     	/**< */

} ETxnStatus;


#endif /*__TXN_DEFS_API_H__*/
