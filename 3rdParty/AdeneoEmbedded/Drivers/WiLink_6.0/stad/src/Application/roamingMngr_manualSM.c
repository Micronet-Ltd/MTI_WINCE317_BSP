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
/** \file roamingMngr_manualSM.c
 *  \brief Roaming Manager
 *
 *  \see roamingMngr_manualSM.h
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Roaming Manager                                               *
 *   PURPOSE:                                                               *
 *   Roaming manager is responsible to receive Roaming triggers and try
 *      to select a better AP.
 *      The Roaming triggers are: Low RSSI, PER, consecutive No ACK on TX,
 *      beacon Missed or External request.
 *      In each Internal Roaming request, scan is performed and selection for 
 *      better AP. Better AP is defined as a different AP with better RSSI,
 *      and similar SSID and security settings.
 *      If better AP is found, there is a check for fast-roaming via the
 *      Supplicant. Then connection to the new AP is invoked.
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_136
#include "osApi.h"
#include "paramOut.h"
#include "report.h"
#include "scanMngrApi.h"
#include "roamingMngrApi.h"
#include "roamingMngrTypes.h"
#include "bssTypes.h"
#include "DrvMainModules.h"
#include "TWDriver.h"
#include "siteMgrApi.h"
#include "GenSM.h"
#include "apConnApi.h"
#include "roamingMngr_manualSM.h"
#include "EvHandler.h"
#include "public_types.h"


typedef enum 
{
	REASSOC_RESP_SUCCESS =0,
	REASSOC_RESP_FAILURE,
    REASSOC_RESP_REJECT
} reassociationResp_e;


static void roamingMngr_SendReassocEvent(TI_HANDLE hRoamingMngr, reassociationResp_e ReassResp);
static void roamingMngr_smIdleToStart (TI_HANDLE hRoamingMngr);
static void roamingMngr_smSTOP (TI_HANDLE hRoamingMngr);
static void roamingMngr_smConnectedToScan (TI_HANDLE hRoamingMngr);
static void roamingMngr_smConnectedToHandover(TI_HANDLE hRoamingMngr);
static void roamingMngr_smScanToConnected(TI_HANDLE hRoamingMngr);
static void roamingMngr_smHandoverToHandoverConnectEvent (TI_HANDLE hRoamingMngr);
static void roamingMngr_smHandoverToHandoverFailEvent (TI_HANDLE hRoamingMngr);
static void roamingMngr_smHandoverToConnectedSuccessEvent(TI_HANDLE hRoamingMngr);
static void roamingMngr_smHandoverToConnectedRejectEvent(TI_HANDLE hRoamingMngr);


/*-----------*/
/* Constants */
/*-----------*/

TGenSM_actionCell roamingMngrManual_matrix[ROAMING_MANUAL_NUM_STATES][ROAMING_MANUAL_NUM_EVENTS] =
{
    /* next state and actions for IDLE state */
    {   
        {ROAMING_MANUAL_STATE_CONNECTED, roamingMngr_smIdleToStart},            /* START */
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                   /* SCAN  */ 
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                   /*CONNECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                  /* STOP*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                  /* REJECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                  /* SUCCESS*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                  /* FAIL*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                  /* COMPLETE*/
    },                                                                              
    /* next state and actions for CONNECTED state */
    {   
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},		                /* START */
        {ROAMING_MANUAL_STATE_SCAN, roamingMngr_smConnectedToScan},                 /* SCAN  */ 
        {ROAMING_MANUAL_STATE_HANDOVER, roamingMngr_smConnectedToHandover},        /*CONNECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smSTOP},                 	        /* STOP*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                      /* REJECT*/
        {ROAMING_MANUAL_STATE_CONNECTED, roamingMngr_smNop},                        /* SUCCESS* retain CurrAp called */
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                      /* FAIL*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                      /* COMPLETE*/
    },
    /* next state and actions for SCAN state */
    {   
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},              /* START */
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},            /* SCAN  */ 
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},            /*CONNECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smSTOP},            	    /* STOP*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},              /* REJECT*/
        {ROAMING_MANUAL_STATE_SCAN, roamingMngr_smNop},                     /* SUCCESS* retain CurrAp called */
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},              /* FAIL*/
        {ROAMING_MANUAL_STATE_CONNECTED, roamingMngr_smScanToConnected},  /* COMPLETE*/
    },
    /* next state and actions for HANDOVER state */
    {   
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                      /* START */
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected},                    /* SCAN  */ 
        {ROAMING_MANUAL_STATE_HANDOVER,roamingMngr_smHandoverToHandoverConnectEvent},    /*CONNECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smSTOP},                            /* STOP*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smHandoverToConnectedRejectEvent},  /* REJECT*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smHandoverToConnectedSuccessEvent},  /* SUCCESS*/
        {ROAMING_MANUAL_STATE_HANDOVER, roamingMngr_smHandoverToHandoverFailEvent},    /* FAIL*/
        {ROAMING_MANUAL_STATE_IDLE, roamingMngr_smUnexpected },                    /* COMPLETE*/
    }
};

TI_INT8*  ManualRoamStateDescription[] = 
{
    "IDLE",
    "CONNECTED",
    "SCAN",
    "HANDOVER"
};

TI_INT8*  ManualRoamEventDescription[] = 
{
    "START",
    "SCAN",
    "CONNECT",
    "STOP",
    "REJECT",
    "SUCCESS",
    "FAIL",
    "COMPLETE"
};

static void roamingMngr_smIdleToStart (TI_HANDLE hRoamingMngr)
{
    roamingMngr_t* pRoamingMngr;

    pRoamingMngr = (roamingMngr_t*)hRoamingMngr;
    scanMngr_startManual((TI_HANDLE)pRoamingMngr->hScanMngr);
}

static void roamingMngr_smSTOP (TI_HANDLE hRoamingMngr)
{
    roamingMngr_t* pRoamingMngr = (roamingMngr_t*)hRoamingMngr;

    //if (SCAN_ISS_IDLE != pScanMngr->immedScanState || SCAN_CSS_IDLE!= != pScanMngr->contScanState) 
    {
        roamingMngr_smStopWhileScanning(hRoamingMngr);
    }
    
    scanMngr_stopManual(pRoamingMngr->hScanMngr);
}

	
static void roamingMngr_smConnectedToScan (TI_HANDLE hRoamingMngr)
{
    roamingMngr_t  *pRoamingMngr = (roamingMngr_t*) hRoamingMngr;
    TI_STATUS status = TI_OK;

    status= apConn_prepareToRoaming(pRoamingMngr->hAPConnection, ROAMING_TRIGGER_NONE);

    if (status  == TI_OK)
    {
        status = scanMngr_startImmediateScan (pRoamingMngr->hScanMngr,TI_FALSE);
    }
    else
    {
        roamingMngr_smEvent(ROAMING_MANUAL_EVENT_COMPLETE, hRoamingMngr);
    }
}

static void roamingMngr_smConnectedToHandover(TI_HANDLE hRoamingMngr)
{
    roamingMngr_t  *pRoamingMngr = (roamingMngr_t*)hRoamingMngr;
    TI_STATUS status = TI_OK;

    pRoamingMngr->handoverWasPerformed = TI_TRUE;
    status= apConn_prepareToRoaming(pRoamingMngr->hAPConnection, pRoamingMngr->roamingTrigger); 

    if (status  == TI_OK)
    {
        apConn_connectToAP(pRoamingMngr->hAPConnection, &(pRoamingMngr->targetAP.newAP), &(pRoamingMngr->targetAP.connRequest), TI_TRUE);
    }
    else
    {
        roamingMngr_smEvent(ROAMING_MANUAL_EVENT_REJECT, hRoamingMngr);
    }	
}

static void roamingMngr_smScanToConnected(TI_HANDLE hRoamingMngr)
{
    roamingMngr_t  *pRoamingMngr = (roamingMngr_t*) hRoamingMngr;
    apConn_connRequest_t request;

    request.dataBufLength = 0;
    request.requestType = AP_CONNECT_RETAIN_CURR_AP;
    apConn_connectToAP(pRoamingMngr->hAPConnection, NULL , &request , TI_FALSE);
}

static void roamingMngr_smHandoverToHandoverConnectEvent (TI_HANDLE hRoamingMngr)
{
	roamingMngr_t  *pRoamingMngr = (roamingMngr_t*) hRoamingMngr;

    apConn_connectToAP(pRoamingMngr->hAPConnection, &(pRoamingMngr->targetAP.newAP), &(pRoamingMngr->targetAP.connRequest), TI_TRUE);
}

static void roamingMngr_smHandoverToHandoverFailEvent (TI_HANDLE hRoamingMngr)
{
    roamingMngr_SendReassocEvent(hRoamingMngr, REASSOC_RESP_FAILURE);
}

static void roamingMngr_smHandoverToConnectedSuccessEvent(TI_HANDLE hRoamingMngr)
{
    roamingMngr_SendReassocEvent(hRoamingMngr, REASSOC_RESP_SUCCESS);
}

static void roamingMngr_smHandoverToConnectedRejectEvent(TI_HANDLE hRoamingMngr)
{
    roamingMngr_SendReassocEvent(hRoamingMngr, REASSOC_RESP_REJECT);
}

static void roamingMngr_SendReassocEvent(TI_HANDLE hRoamingMngr, reassociationResp_e ReassResp)
{
	roamingMngr_t  *pRoamingMngr = (roamingMngr_t*) hRoamingMngr;

    TRACE1(pRoamingMngr->hReport, REPORT_SEVERITY_INFORMATION, "roamingMngr_SendReassocEvent(): %d \n", ReassResp);
    EvHandlerSendEvent(pRoamingMngr->hEvHandler,
                       IPC_EVENT_REASSOCIATION_RESP,
                       (TI_UINT8*)(&ReassResp),
                       sizeof(reassociationResp_e));
}



