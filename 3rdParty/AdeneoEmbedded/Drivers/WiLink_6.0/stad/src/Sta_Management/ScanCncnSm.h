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
/** \file  ScanCncnSm.h 
 *  \brief Scan concentrator state machine declarations
 *
 *  \see   ScanCncnSm.c
 */


#ifndef __SCAN_CNCN_SM_H__
#define __SCAN_CNCN_SM_H__

#include "osTIType.h"
#include "TWDriver.h"
#include "ScanCncn.h"

typedef void (*TScanPrivateSMFunction)(TI_HANDLE hScanCncn);

typedef union
{
    TPeriodicScanParams tPeriodicScanParams;
    TScanParams         tOneShotScanParams;
}UScanParams;

typedef struct
{
    TI_HANDLE               hOS;
    TI_HANDLE               hReport;
    TI_HANDLE               hSCR;
    TI_HANDLE               hTWD;
    TI_HANDLE               hApConn;
    TI_HANDLE               hMlme;
    TI_HANDLE               hGenSM;
    TI_HANDLE               hScanCncn;
    TI_BOOL                 bScanCompletePending; /* TRUE if scan complete event is received
                                                    before all results, for periodic scan */
    TI_BOOL                 bInRequest;
    TI_BOOL                 bSendNullDataOnStop;  /* indicates whether NULL data frame is to be sent wehn
                                                     stopping scan to return to previous PS mode */
    TScanPrivateSMFunction  fScrRequest;
    TScanPrivateSMFunction  fScrRelease;
    TScanPrivateSMFunction  fStartScan;
    TScanPrivateSMFunction  fStopScan;
    TScanPrivateSMFunction  fRecovery;

    /* Scan complete callbacks */ 
    TScanResultCB           tScanResultCB;
    TI_HANDLE               hScanResultCBObj;

    UScanParams             uScanParams;
    EScanCncnResultStatus   eScanResult;
    TI_INT8                 *pScanSmName;
    TI_UINT16               uSPSScanResult;
    TI_UINT32               uResultCounter;
    TI_UINT32               uResultExpectedNumber;
	TI_BOOL					bScanRejectedOn2_4;
} TScanCncnClient;

typedef enum
{
    SCAN_CNCN_SM_STATE_IDLE = 0,
    SCAN_CNCN_SM_STATE_SCR_WAIT,
    SCAN_CNCN_SM_STATE_SCANNING,
    SCAN_CNCN_SM_STATE_STOPPING,
    SCAN_CNCN_SM_NUMBER_OF_STATES
} EScanCncnSmStates;

typedef enum
{
    SCAN_CNCN_SM_EVENT_START = 0,
    SCAN_CNCN_SM_EVENT_RUN,
    SCAN_CNCN_SM_EVENT_SCAN_COMPLETE,
    SCAN_CNCN_SM_EVENT_STOP,
    SCAN_CNCN_SM_EVENT_ABORT,
    SCAN_CNCN_SM_EVENT_RECOVERY,
    SCAN_CNCN_SM_EVENT_REJECT,
    SCAN_CNCN_SM_NUMBER_OF_EVENTS
} EScanCncnSmEvents;

TI_HANDLE   scanCncnSm_Create               (TI_HANDLE hOS);
void        scanCncnSm_Init                 (TI_HANDLE hScanCncnClient, TI_HANDLE hReport, TI_HANDLE hTWD, 
                                             TI_HANDLE hSCR, TI_HANDLE hApConn, TI_HANDLE hMlme, 
                                             TI_HANDLE hScanCncn, TScanPrivateSMFunction fScrRequest, 
                                             TScanPrivateSMFunction fScrRelease, TScanPrivateSMFunction fStartScan, 
                                             TScanPrivateSMFunction fStopScan, TScanPrivateSMFunction fRecovery, 
                                             TI_INT8* pScanSmName);
void        scanCncnSm_Destroy              (TI_HANDLE hScanCncnClient);
void        scanCncnSmApp1Shot_ScrRequest      (TI_HANDLE hScanCncnClient);
void        scanCncnSmApp1Shot_ScrRelease      (TI_HANDLE hScanCncnClient);
void        scanCncnSmApp1Shot_StartScan       (TI_HANDLE hScanCncnClient);
void        scanCncnSmApp1Shot_StopScan        (TI_HANDLE hScanCncnClient);
void        scanCncnSmApp1Shot_Recovery        (TI_HANDLE hScanCncnClient);
void        scanCncnSmAppP_ScrRequest       (TI_HANDLE hScanCncnClient);
void        scanCncnSmAppP_ScrRelease       (TI_HANDLE hScanCncnClient);
void        scanCncnSmAppP_StartScan        (TI_HANDLE hScanCncnClient);
void        scanCncnSmAppP_StopScan         (TI_HANDLE hScanCncnClient);
void        scanCncnSmAppP_Recovery         (TI_HANDLE hScanCncnClient);
void        scanCncnSmDrvP_ScrRequest       (TI_HANDLE hScanCncnClient);
void        scanCncnSmDrvP_ScrRelease       (TI_HANDLE hScanCncnClient);
void        scanCncnSmDrvP_StartScan        (TI_HANDLE hScanCncnClient);
void        scanCncnSmDrvP_StopScan         (TI_HANDLE hScanCncnClient);
void        scanCncnSmDrvP_Recovery         (TI_HANDLE hScanCncnClient);
void        scanCncnSmCont1Shot_ScrRequest     (TI_HANDLE hScanCncnClient);
void        scanCncnSmCont1Shot_ScrRelease     (TI_HANDLE hScanCncnClient);
void        scanCncnSmCont1Shot_StartScan      (TI_HANDLE hScanCncnClient);
void        scanCncnSmCont1Shot_StopScan       (TI_HANDLE hScanCncnClient);
void        scanCncnSmCont1Shot_Recovery       (TI_HANDLE hScanCncnClient);
void        scanCncnSmImmed1Shot_ScrRequest    (TI_HANDLE hScanCncnClient);
void        scanCncnSmImmed1Shot_ScrRelease    (TI_HANDLE hScanCncnClient);
void        scanCncnSmImmed1Shot_StartScan     (TI_HANDLE hScanCncnClient);
void        scanCncnSmImmed1Shot_StopScan      (TI_HANDLE hScanCncnClient);
void        scanCncnSmImmed1Shot_Recovery      (TI_HANDLE hScanCncnClient);

#endif /* __SCAN_CNCN_SM_H__ */

