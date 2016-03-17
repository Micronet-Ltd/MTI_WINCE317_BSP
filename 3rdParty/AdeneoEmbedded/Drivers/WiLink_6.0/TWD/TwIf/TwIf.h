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


/** \file   TwIf.h 
 *  \brief  TwIf module API definition                                  
 *
 *  \see    TwIf.c
 */

#ifndef __TWIF_API_H__
#define __TWIF_API_H__


#include "Device.h"
#include "TxnDefs.h"
#include "BusDrv.h"


/************************************************************************
 * Defines
 ************************************************************************/


/************************************************************************
 * Macros
 ************************************************************************/
/* 
 * Defines a TNETWIF read/write field with padding.
 * A low level driver may use this padding internally
 */
#define PADDING(field) \
    TI_UINT8 padding [TNETWIF_READ_OFFSET_BYTES];  \
    field;


/************************************************************************
 * Types
 ************************************************************************/

typedef void (*TTwIfCallback)(TI_HANDLE hCb);
typedef void (*TRecoveryCb)(TI_HANDLE hCb);



/************************************************************************
 * Functions
 ************************************************************************/
TI_HANDLE   twIf_Create (TI_HANDLE hOs);
TI_STATUS   twIf_Destroy (TI_HANDLE hTwIf);
void        twIf_Init (TI_HANDLE hTwIf, 
                       TI_HANDLE hReport, 
                       TI_HANDLE hContext, 
                       TI_HANDLE hTimer, 
                       TI_HANDLE hTxnQ, 
                       TRecoveryCb fRecoveryCb, 
                       TI_HANDLE hRecoveryCb);
void        twIf_RegisterErrCb (TI_HANDLE hTwIf, void *fErrCb, TI_HANDLE hErrCb);
ETxnStatus  twIf_Restart (TI_HANDLE hTwIf);
void        twIf_SetPartition (TI_HANDLE hTwIf,
                               TPartition *pPartition);
void        twIf_Awake (TI_HANDLE hTwIf);
void        twIf_Sleep (TI_HANDLE hTwIf);
void        twIf_HwAvailable (TI_HANDLE hTwIf);
ETxnStatus  twIf_Transact (TI_HANDLE hTwIf, TTxnStruct *pTxn);
ETxnStatus  twIf_TransactReadFWStatus (TI_HANDLE hTwIf, TTxnStruct *pTxn);

TI_BOOL		twIf_isValidMemoryAddr(TI_HANDLE hTwIf, TI_UINT32 Address, TI_UINT32 Length);
TI_BOOL		twIf_isValidRegAddr(TI_HANDLE hTwIf, TI_UINT32 Address, TI_UINT32 Length);

#ifdef TI_DBG
    void        twIf_PrintModuleInfo (TI_HANDLE hTwIf);
    void        twIf_PrintQueues (TI_HANDLE hTwIf);
    void        twIf_PrintWakeLockCounter(TI_HANDLE hTwIf);
#endif /* TI_DBG */




#endif /*__TWIF_API_H__*/
