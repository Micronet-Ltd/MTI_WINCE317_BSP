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
#ifndef DRVMAIN_MODULES_H
#define DRVMAIN_MODULES_H

#include "tidef.h"

/* STAD Modules Handles List */
typedef struct
{
    TI_HANDLE           hDrvMain;
    TI_HANDLE           hOs;
    TI_HANDLE           hReport;
    TI_HANDLE           hContext;
    TI_HANDLE           hTimer;
    TI_HANDLE           hTWD;
    TI_HANDLE           hCmdHndlr;
    TI_HANDLE           hSme;
    TI_HANDLE           hSiteMgr;
    TI_HANDLE           hConn;
    TI_HANDLE           hMlmeSm;
    TI_HANDLE           hAuth;
    TI_HANDLE           hAssoc;
    TI_HANDLE           hRxData;
    TI_HANDLE           hTxCtrl;
    TI_HANDLE           hTxPort;
    TI_HANDLE           hTxDataQ;
    TI_HANDLE           hTxMgmtQ;
    TI_HANDLE           hCtrlData;
    TI_HANDLE           hTrafficMon;
    TI_HANDLE           hRsn;
    TI_HANDLE           hRegulatoryDomain;
    TI_HANDLE           hMeasurementMgr;
    TI_HANDLE           hSoftGemini;
    TI_HANDLE           hXCCMngr;
    TI_HANDLE           hRoamingMngr;
    TI_HANDLE           hQosMngr;
    TI_HANDLE           hPowerMgr;
    TI_HANDLE           hPowerSrv;
    TI_HANDLE           hEvHandler;
    TI_HANDLE           hAPConnection;
    TI_HANDLE           hCurrBss;
    TI_HANDLE           hSwitchChannel;
    TI_HANDLE           hSCR;               
    TI_HANDLE           hScanCncn;          
    TI_HANDLE           hScanMngr;          
    TI_HANDLE           hHealthMonitor; 
    TI_HANDLE           hCmdDispatch; 
    TI_HANDLE           hStaCap;
    TI_HANDLE           hTxnQ;

} TStadHandlesList;


#endif  /* DRVMAIN_MODULES_H */
