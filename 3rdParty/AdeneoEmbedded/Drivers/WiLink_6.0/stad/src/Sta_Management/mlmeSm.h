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
 /** \file mlmeSm.h
 *  \brief 802.11 MLME SM
 *
 *  \see mlmeSm.c
 */


/***************************************************************************/
/*                                                                         */
/*      MODULE: mlmeSm.h                                                   */
/*    PURPOSE:  802.11 MLME SM                                     */
/*                                                                         */
/***************************************************************************/

#ifndef _MLME_SM_H
#define _MLME_SM_H

#include "fsm.h"
#include "mlmeApi.h"

/* Constants */

/* changed to fit double buffer size - S.G */
/*#define MAX_MANAGEMENT_FRAME_BODY_LEN 2312*/
#define MAX_MANAGEMENT_FRAME_BODY_LEN   1476

/* Enumerations */

/* state machine states */
typedef enum 
{
    MLME_SM_STATE_IDLE          = 0,
    MLME_SM_STATE_AUTH_WAIT     = 1,
    MLME_SM_STATE_ASSOC_WAIT    = 2,
    MLME_SM_STATE_ASSOC         = 3,
    MLME_SM_NUM_STATES
} mlme_smStates_t;

/* State machine inputs */
typedef enum 
{
    MLME_SM_EVENT_START             = 0,
    MLME_SM_EVENT_STOP              = 1,
    MLME_SM_EVENT_AUTH_SUCCESS      = 2,
    MLME_SM_EVENT_AUTH_FAIL         = 3,
    MLME_SM_EVENT_ASSOC_SUCCESS     = 4,
    MLME_SM_EVENT_ASSOC_FAIL        = 5,
    MLME_SM_NUM_EVENTS          
} mlme_smEvents_t;



/* Typedefs */

typedef struct
{
    mgmtStatus_e mgmtStatus;
    TI_UINT16        uStatusCode;
} mlmeData_t;


typedef struct
{
    fsm_stateMachine_t  *pMlmeSm;
    TI_UINT8            currentState;
    mlmeData_t          mlmeData;
    legacyAuthType_e    legacyAuthType;
    TI_BOOL             reAssoc;
    DisconnectType_e    disConnType;
    mgmtStatus_e        disConnReason;
    TI_BOOL             bParseBeaconWSC;

    /* temporary frame info */
    mlmeIEParsingParams_t tempFrameInfo;
    
    /* debug info - start */
    TI_UINT32           debug_lastProbeRspTSFTime;
    TI_UINT32           debug_lastDtimBcnTSFTime;
    TI_UINT32           debug_lastBeaconTSFTime;
    TI_BOOL             debug_isFunctionFirstTime;
    TI_UINT32           totalMissingBeaconsCounter;
    TI_UINT32           totalRcvdBeaconsCounter;
    TI_UINT32           maxMissingBeaconSequence;
    TI_UINT32           BeaconsCounterPS;
    /* debug info - end */

    TI_HANDLE           hAuth;
    TI_HANDLE           hAssoc;
    TI_HANDLE           hSiteMgr;
    TI_HANDLE           hCtrlData;
    TI_HANDLE           hConn;
    TI_HANDLE           hTxMgmtQ;
    TI_HANDLE           hMeasurementMgr;
    TI_HANDLE           hSwitchChannel;
    TI_HANDLE           hRegulatoryDomain;
    TI_HANDLE           hReport;
    TI_HANDLE           hOs;
    TI_HANDLE           hCurrBss;
    TI_HANDLE           hApConn;
    TI_HANDLE           hScanCncn;
    TI_HANDLE           hQosMngr;
    TI_HANDLE           hTWD;
    TI_HANDLE           hTxCtrl;
} mlme_t;

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

void mlme_smTimeout(TI_HANDLE hMlme);

/* local functions */

TI_STATUS mlme_smEvent(TI_UINT8 *currentState, TI_UINT8 event, TI_HANDLE hMlme);

/* state machine functions */
TI_STATUS mlme_smStartIdle(mlme_t *pMlme);
TI_STATUS mlme_smAuthSuccessAuthWait(mlme_t *pMlme);
TI_STATUS mlme_smAuthFailAuthWait(mlme_t *pMlme);
TI_STATUS mlme_smStopAssocWait(mlme_t *pMlme);
TI_STATUS mlme_smAssocSuccessAssocWait(mlme_t *pMlme);
TI_STATUS mlme_smAssocFailAssocWait(mlme_t *pMlme);
TI_STATUS mlme_smStopAssoc(mlme_t *pMlme);
TI_STATUS mlme_smActionUnexpected(mlme_t *pMlme);
TI_STATUS mlme_smNOP(mlme_t *pMlme);

TI_STATUS mlme_smReportStatus(mlme_t *pMlme);

#endif

