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


#ifndef DRVMAIN_H
#define DRVMAIN_H


#include "paramOut.h"
#include "WlanDrvCommon.h"


/* Driver-Main module external functions */

TI_STATUS drvMain_Create 			(TI_HANDLE  hOs, 
									 TI_HANDLE *pDrvMainHndl, 
									 TI_HANDLE *pCmdHndlr, 
									 TI_HANDLE *pContext, 
									 TI_HANDLE *pTxDataQ, 
									 TI_HANDLE *pTxMgmtQ,
									 TI_HANDLE *pTxCtrl,
									 TI_HANDLE *pTwd,
									 TI_HANDLE *pEvHandler,
                                     TI_HANDLE *pCmdDispatch,
                                     TI_HANDLE *pReport);
TI_STATUS drvMain_Destroy           (TI_HANDLE  hDrvMain);
TI_STATUS drvMain_InsertAction      (TI_HANDLE  hDrvMain, EActionType eAction);
TI_STATUS drvMain_Recovery          (TI_HANDLE  hDrvMain);
void      drvMain_SmeStop           (TI_HANDLE hDrvMain);
ESuspendAction drvMain_GetSuspendAction(TI_HANDLE hDrvMain);

#endif
