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
 *   MODULE:  rxXfer_api.h
 *
 *   PURPOSE: Rx Xfer module API.
 * 
 ****************************************************************************/

#ifndef _RX_XFER_API_H
#define _RX_XFER_API_H


#include "TWDriverInternal.h"

/* Public Function Definitions */


TI_HANDLE			rxXfer_Create(TI_HANDLE hOs);

void				rxXfer_Destroy(TI_HANDLE hRxXfer);

void				rxXfer_Init( TI_HANDLE hRxXfer,
								 TI_HANDLE hFwEvent, 
								 TI_HANDLE hReport,
                                 TI_HANDLE hTwIf,
                                 TI_HANDLE hRxQueue);

void                rxXfer_SetDefaults (TI_HANDLE hRxXfer, TTwdInitParams *pInitParams);

void                rxXfer_SetBusParams (TI_HANDLE hRxXfer, TI_UINT32 uDmaBufLen);

ETxnStatus           rxXfer_RxEvent (TI_HANDLE hRxXfer, FwStatus_t* pFwStatus);

void				rxXfer_Register_CB(TI_HANDLE hRxXfer,TI_UINT32 CallBackID,void *CBFunc,TI_HANDLE CBObj);

void                rxXfer_SetRxDirectAccessParams (TI_HANDLE hRxXfer, TDmaParams *pDmaParams);
                    
void                rxXfer_RegisterErrCb (TI_HANDLE hRxXfer, void *fErrCb, TI_HANDLE hErrCb);

void                rxXfer_Restart (TI_HANDLE hRxXfer);


#ifdef TI_DBG

void rxXfer_ClearStats( TI_HANDLE hRxXfer );

void rxXfer_PrintStats( TI_HANDLE hRxXfer );

#endif /* TI_DBG */


#endif /* _RX_XFER_API_H */




