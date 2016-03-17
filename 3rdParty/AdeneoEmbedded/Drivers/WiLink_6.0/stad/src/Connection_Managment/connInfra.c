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
/** \file connInfra.c
 *  \brief Infra connection implementation
 *
 *  \see connInfra.h
 */

/***************************************************************************/
/*                                                                         */
/*      MODULE: connInfra.c                                                */
/*    PURPOSE:  Infra connection implementation                            */
/*                                                                         */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_27
#include "tidef.h"
#include "report.h"
#include "osApi.h"
#include "conn.h"
#include "connInfra.h"
#include "timer.h"
#include "fsm.h"
#include "siteMgrApi.h"
#include "sme.h"
#include "rsnApi.h"
#include "DataCtrl_Api.h"  
#include "paramOut.h"
#include "siteHash.h"
#include "smeSm.h"
#include "PowerMgr_API.h"
#include "measurementMgrApi.h"
#include "TrafficMonitorAPI.h"
#include "qosMngr_API.h"
#include "EvHandler.h"
#include "SwitchChannelApi.h"
#include "ScanCncn.h"
#include "currBss.h"
#include "healthMonitor.h"
#include "regulatoryDomainApi.h"
#include "txCtrl.h"
#include "TWDriver.h"
#include "SoftGeminiApi.h"
#include "RxQueue_api.h"

#ifdef XCC_MODULE_INCLUDED
#include "XCCMngr.h"
#include "XCCTSMngr.h"
#endif

#define DISCONNECT_TIMEOUT_MSEC      800

/* Local functions prototypes */

static TI_STATUS actionUnexpected(void *pData);

static TI_STATUS actionNop(void *pData);

static TI_STATUS connInfra_ScrWait(void *pData);

static TI_STATUS Idle_to_Idle(void *pData);

static TI_STATUS ScrWait_to_idle(void *pData);

static TI_STATUS ScrWait_to_JoinWait(void *pData);

static TI_STATUS JoinWait_to_mlmeWait(void *pData);

static TI_STATUS JoinWait_to_WaitDisconnect(void *pData);

static TI_STATUS mlmeWait_to_WaitDisconnect(void *pData);

static TI_STATUS mlmeWait_to_rsnWait(void *pData);

static TI_STATUS rsnWait_to_disconnect(void *pData);

static TI_STATUS rsnWait_to_configHW(void *pData);

static TI_STATUS configHW_to_connected(void *pData);

static TI_STATUS configHW_to_disconnect(void *pData);

static TI_STATUS connInfra_ScrWaitDisconn_to_disconnect(void *pData);

static TI_STATUS connect_to_ScrWait(void *pData);

static TI_STATUS prepare_send_disconnect(void *pData);

static TI_STATUS connInfra_WaitDisconnectToIdle (void *pData);


static TI_STATUS stopModules( conn_t *pConn, TI_BOOL bDisconnect );

void InfraConnSM_ScrCB( TI_HANDLE hConn, EScrClientRequestStatus requestStatus,
                        EScrResourceId eResource, EScePendReason pendReason );

int conn_ConfigHwFinishCb(TI_HANDLE pData);

/********************************************/
/*      Functions Implementations           */
/********************************************/


/***********************************************************************
 *                        conn_infraConfig                                  
 ***********************************************************************
DESCRIPTION: Infra Connection configuration function, called by the conection set param function
                in the selection phase. Configures the connection state machine to Infra connection mode
                                                                                                   
INPUT:      hConn   -   Connection handle.

OUTPUT:     

RETURN:     TI_OK on success, TI_NOK otherwise

************************************************************************/
TI_STATUS conn_infraConfig(conn_t *pConn)
{
    static fsm_actionCell_t    smMatrix[CONN_INFRA_NUM_STATES][CONN_INFRA_NUM_EVENTS] =
    {
        /* next state and actions for IDLE state */
        {   {STATE_CONN_INFRA_SCR_WAIT_CONN, connInfra_ScrWait},        /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_IDLE, actionNop       },                  /* "EVENT_SCR_SUCC"*/
            {STATE_CONN_INFRA_IDLE, actionNop       },                  /* "EVENT_JOIN_CMD_CMPLT */
            {STATE_CONN_INFRA_IDLE, Idle_to_Idle    },                  /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_IDLE, actionUnexpected},                  /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_IDLE, actionUnexpected},                  /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_IDLE, actionNop},                         /* "EVENT_CONFIG_HW" */
            {STATE_CONN_INFRA_IDLE, actionUnexpected}                   /* "EVENT_DISCONN_COMPLETE" */
        },

        /* next state and actions for SCR_WAIT_CONN state */
        {   {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionUnexpected},    /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, ScrWait_to_JoinWait},    /* "EVENT_SCR_SUCC"*/
            {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionUnexpected},    /* "EVENT_JOIN_CMD_CMPLT */
            {STATE_CONN_INFRA_IDLE,     ScrWait_to_idle},               /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionUnexpected},    /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionUnexpected},    /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionNop},           /* "EVENT_CONFIG_HW "*/
            {STATE_CONN_INFRA_SCR_WAIT_CONN     , actionNop}            /* "EVENT_DISCONN_COMPLETE" */
        },

        /* next state and actions for WAIT_JOIN_CMPLT */
        {   {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionUnexpected},       /* "EVENT_CONNECT"    */
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionUnexpected},       /* "EVENT_SCR_SUCC"*/
            {STATE_CONN_INFRA_MLME_WAIT,       JoinWait_to_mlmeWait},   /* "EVENT_JOIN_CMD_CMPLT"   */
            {STATE_CONN_INFRA_WAIT_DISCONNECT, JoinWait_to_WaitDisconnect},/* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionUnexpected},       /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionUnexpected},       /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionNop},              /* "EVENT_CONFIG_HW"        */
            {STATE_CONN_INFRA_WAIT_JOIN_CMPLT, actionUnexpected}        /* "EVENT_DISCONN_COMPLETE" */
        
        },

        /* next state and actions for MLME_WAIT state */
        {   {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected},             /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected},             /* "EVENT_SCR_SUCC" */
            {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected},             /* "EVENT_JOIN_CMD_CMPLT"*/
            {STATE_CONN_INFRA_WAIT_DISCONNECT, mlmeWait_to_WaitDisconnect}, /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_RSN_WAIT,  mlmeWait_to_rsnWait},          /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected},             /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected},             /* "EVENT_CONFIG_HW" */
            {STATE_CONN_INFRA_MLME_WAIT, actionUnexpected}              /* "EVENT_DISCONN_COMPLETE" */   
        },
        
        /* next state and actions for RSN_WAIT state */
        {   {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected},          /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected},          /* "EVENT_SCR_SUCC" */
            {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected},          /* "EVENT_JOIN_CMD_CMPLT"*/
            {STATE_CONN_INFRA_WAIT_DISCONNECT,    rsnWait_to_disconnect},   /* "EVENT_DISCONNECT" */
            {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected},          /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_CONFIG_HW,    rsnWait_to_configHW},       /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected},          /* "EVENT_CONFIG_HW"        */
            {STATE_CONN_INFRA_RSN_WAIT,     actionUnexpected}           /* "EVENT_DISCONN_COMPLETE" */
        },
        
        /* next state and actions for CONFIG_HW state */
        {   {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected},             /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected},             /* "EVENT_SCR_SUCC" */
            {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected},             /* "EVENT_JOIN_CMD_CMPLT"*/
            {STATE_CONN_INFRA_WAIT_DISCONNECT, configHW_to_disconnect},     /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected},             /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected},             /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_CONNECTED, configHW_to_connected},        /* "EVENT_CONFIG_HW"        */
            {STATE_CONN_INFRA_CONFIG_HW, actionUnexpected}              /* "EVENT_DISCONN_COMPLETE" */   
        },

        /* next state and actions for CONNECTED state */
        {   {STATE_CONN_INFRA_SCR_WAIT_CONN, connect_to_ScrWait},   /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected},         /* "EVENT_SCR_SUCC"*/
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected},         /* "EVENT_JOIN_CMD_CMPLT" */
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, connInfra_ScrWait}, /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected},         /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected},         /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected},         /* "STATE_CONN_INFRA_CONFIG_HW" */
            {STATE_CONN_INFRA_CONNECTED, actionUnexpected}          /* "EVENT_DISCONN_COMPLETE" */   
        },
        
        /* next state and actions for SCR_WAIT_DISCONN state */
        {   {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionUnexpected},     /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_WAIT_DISCONNECT       , connInfra_ScrWaitDisconn_to_disconnect},  /* "EVENT_SCR_SUCC"*/
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionUnexpected},     /* "EVENT_JOIN_CMD_CMPLT */
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, ScrWait_to_idle},      /* "EVENT_DISCONNECT"       */
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionUnexpected},     /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionUnexpected},     /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionNop},            /* "EVENT_CONFIG_HW "*/
            {STATE_CONN_INFRA_SCR_WAIT_DISCONN, actionNop}             /* "EVENT_DISCONN_COMPLETE" */
        },

        /* next state and actions for STATE_CONN_INFRA_WAIT_DISCONNECT state */
        {   {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "EVENT_CONNECT"  */
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "STATE_CONN_INFRA_SCR_WAIT_CONN"*/
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "EVENT_JOIN_CMD_CMPLT" */
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "EVENT_DISCONNECT" */
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "EVENT_MLME_SUCC"*/
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "EVENT_RSN_SUCC" */
            {STATE_CONN_INFRA_WAIT_DISCONNECT, actionUnexpected},               /* "STATE_CONN_INFRA_CONFIG_HW"  */
            {STATE_CONN_INFRA_IDLE           , connInfra_WaitDisconnectToIdle}  /* "EVENT_DISCONN_COMPLETE" */       
        }
        
    };

    scr_registerClientCB( pConn->hScr, SCR_CID_CONNECT, InfraConnSM_ScrCB, pConn );

    return fsm_Config(pConn->infra_pFsm, (fsm_Matrix_t)smMatrix, CONN_INFRA_NUM_STATES, CONN_INFRA_NUM_EVENTS, conn_infraSMEvent, pConn->hOs);
}


/***********************************************************************
 *                        conn_infraSMEvent                                 
 ***********************************************************************
DESCRIPTION: Infra Connection SM event processing function, called by the connection API
                Perform the following:
                -   Print the state movement as a result from the event
                -   Calls the generic state machine event processing function which preform the following:
                    -   Calls the correspoding callback function
                    -   Move to next state
                
INPUT:      currentState    -   Pointer to the connection current state.
            event   -   Received event
            pConn   -   Connection handle

OUTPUT:     

RETURN:     TI_OK on success, TI_NOK otherwise

************************************************************************/
TI_STATUS conn_infraSMEvent(TI_UINT8 *currentState, TI_UINT8 event, TI_HANDLE hConn)
{
   conn_t *pConn = (conn_t *)hConn;
    TI_STATUS       status;
    TI_UINT8       nextState;

    status = fsm_GetNextState(pConn->infra_pFsm, *currentState, event, &nextState);

    if (status != TI_OK)
    {
        TRACE0(pConn->hReport, REPORT_SEVERITY_SM, "State machine error, failed getting next state\r\n");
        return(TI_NOK);
    }

	TRACE3( pConn->hReport, REPORT_SEVERITY_INFORMATION, "conn_infraSMEvent: <currentState = %d, event = %d> --> nextState = %d\r\n", *currentState, event, nextState);

    status = fsm_Event(pConn->infra_pFsm, currentState, event, (void *)pConn);

    return status;
}

/************************************************************************************************************/
/*      In the following section are listed the callback function used by the Infra connection state machine    */
/************************************************************************************************************/

/* JOIN, SET_DATA_PORT_NOTIFY, START_MLME */
static TI_STATUS ScrWait_to_JoinWait(void *pData)
{
    TI_STATUS status;
    conn_t *pConn = (conn_t *)pData; 

    status = siteMgr_join(((conn_t *)pData)->hSiteMgr );
    /* If the Join command was failed we report the SME that connection failure so it could exit connecting state */
    if (status != TI_OK)
    {
       TRACE0(pConn->hReport, REPORT_SEVERITY_ERROR, "Join command has failed!\r\n");
    }
    return status;
}


static TI_STATUS JoinWait_to_mlmeWait(void *pData)
{
    TI_STATUS status;
    paramInfo_t *pParam;
    conn_t *pConn = (conn_t *)pData;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    /* Set the BA session policies to the FW */
    qosMngr_SetBaPolicies(pConn->hQosMngr);

    pParam->paramType = SITE_MGR_CURRENT_CHANNEL_PARAM;
    siteMgr_getParam(pConn->hSiteMgr, pParam);

    pParam->paramType = REGULATORY_DOMAIN_TX_POWER_AFTER_SELECTION_PARAM;
    pParam->content.channel = pParam->content.siteMgrCurrentChannel;
    regulatoryDomain_setParam(pConn->hRegulatoryDomain, pParam);

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = OPEN_NOTIFY;
    status = rxData_setParam(pConn->hRxData, pParam);
    if (status != TI_OK)
    {
        TRACE1( pConn->hReport, REPORT_SEVERITY_FATAL_ERROR, "JoinWait_to_mlmeWait: rxData_setParam return 0x%x.\r\n", status);
        os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
        return status;
    }

    /* Update TxMgmtQueue SM to open Tx path only for Mgmt packets. */
    txMgmtQ_SetConnState (((conn_t *)pData)->hTxMgmtQ, TX_CONN_STATE_MGMT);

    /* 
     * Set the reassociation flag in the association logic.
     */ 
    pParam->paramType = MLME_RE_ASSOC_PARAM;

    if( pConn->connType == CONN_TYPE_ROAM )
        pParam->content.mlmeReAssoc = TI_TRUE;
    else 
        pParam->content.mlmeReAssoc = TI_FALSE;

    status = mlme_setParam(pConn->hMlmeSm, pParam);

    if (status != TI_OK)
    {
        TRACE1( pConn->hReport, REPORT_SEVERITY_FATAL_ERROR, "JoinWait_to_mlmeWait: mlme_setParam return 0x%x.\r\n", status);
    }
    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    return mlme_start(pConn->hMlmeSm);
}


/* STOP_MLME, SET_DATA_PORT_CLOSE, DIS_JOIN */
static TI_STATUS mlmeWait_to_WaitDisconnect(void *pData)
{
    TI_STATUS   status;
    paramInfo_t *pParam;
    conn_t      *pConn = (conn_t *)pData;

    status = mlme_stop( pConn->hMlmeSm, DISCONNECT_IMMEDIATE, pConn->disConnReasonToAP );
    if (status != TI_OK)
        return status;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = CLOSE;
    rxData_setParam(pConn->hRxData, pParam);


    /* Update TxMgmtQueue SM to close Tx path. */
    txMgmtQ_SetConnState (pConn->hTxMgmtQ, TX_CONN_STATE_CLOSE);

    /* Start the disconnect complete time out timer. 
       Disconect Complete event, which stops the timer. */
    tmr_StartTimer (pConn->hConnTimer, conn_timeout, (TI_HANDLE)pConn, DISCONNECT_TIMEOUT_MSEC, TI_FALSE);

    /* FW will send the disconn frame according to disConnType */ 
    TWD_CmdFwDisconnect (pConn->hTWD, pConn->disConnType, pConn->disConnReasonToAP); 

#ifdef XCC_MODULE_INCLUDED
    XCCMngr_updateIappInformation(pConn->hXCCMngr, XCC_DISASSOC);
#endif
    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    return TI_OK;
}

/* This function is called from the WAIT_FOR_JOIN_CB_CMPLT state (before mlme_start)
  - all we need to do is call siteMgr_disJoin */
static TI_STATUS JoinWait_to_WaitDisconnect(void *pData)
{
    conn_t *pConn = (conn_t *)pData; 

    /* Start the disconnect complete time out timer. 
       Disconect Complete event, which stops the timer. */
    tmr_StartTimer (pConn->hConnTimer, conn_timeout, (TI_HANDLE)pConn, DISCONNECT_TIMEOUT_MSEC, TI_FALSE);

    /* FW will send the disconn frame according to disConnType */ 
    TWD_CmdFwDisconnect (pConn->hTWD, pConn->disConnType, pConn->disConnReasonToAP); 
    
   return TI_OK;
}

/* SET_DATA_PORT_EAPOL, START_RSN */
static TI_STATUS mlmeWait_to_rsnWait(void *pData)
{
    TI_STATUS status;
    paramInfo_t *pParam;
    conn_t *pConn = (conn_t *)pData;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = OPEN_EAPOL;
    status = rxData_setParam(pConn->hRxData, pParam);    
    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    if (status != TI_OK)
        return status;

    /* Update TxMgmtQueue SM to enable EAPOL packets. */
    txMgmtQ_SetConnState (((conn_t *)pData)->hTxMgmtQ, TX_CONN_STATE_EAPOL);

    /*
     *  Notify that the driver is associated to the supplicant\IP stack. 
     */
    EvHandlerSendEvent(pConn->hEvHandler, IPC_EVENT_ASSOCIATED, NULL,0);

    status = rsn_start(pConn->hRsn);
    return status;
}



/* STOP_RSN, SET_DATA_PORT_CLOSE, STOP_MLME, DIS_JOIN */
static TI_STATUS rsnWait_to_disconnect(void *pData)
{
    TI_STATUS status;
    paramInfo_t *pParam;
    conn_t *pConn = (conn_t *)pData;

    status = rsn_stop(pConn->hRsn, pConn->disConEraseKeys);
    if (status != TI_OK)
        return status;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = CLOSE;
    status = rxData_setParam(pConn->hRxData, pParam);
    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    if (status != TI_OK)
        return status;

    /* Update TxMgmtQueue SM to close Tx path for all except Mgmt packets. */
    txMgmtQ_SetConnState (pConn->hTxMgmtQ, TX_CONN_STATE_MGMT);

    status = mlme_stop( pConn->hMlmeSm, DISCONNECT_IMMEDIATE, pConn->disConnReasonToAP );

    if (status != TI_OK)
        return status;

    /* send disconnect command to firmware */
    prepare_send_disconnect(pData);

    return TI_OK;
}


/* STOP_RSN, SET_DATA_PORT_CLOSE, STOP_MLME, DIS_JOIN */
static TI_STATUS configHW_to_disconnect(void *pData)
{
    TI_STATUS status;
    paramInfo_t *pParam;
    conn_t *pConn = (conn_t *)pData;

    status = rsn_stop(pConn->hRsn, pConn->disConEraseKeys );
    if (status != TI_OK)
        return status;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = CLOSE;
    status = rxData_setParam(pConn->hRxData, pParam);
    if (status == TI_OK) 
    {
        /* Update TxMgmtQueue SM to close Tx path for all except Mgmt packets. */
        txMgmtQ_SetConnState (pConn->hTxMgmtQ, TX_CONN_STATE_MGMT);

        status = mlme_stop( pConn->hMlmeSm, DISCONNECT_IMMEDIATE, pConn->disConnReasonToAP );
        if (status == TI_OK) 
        {
            pParam->paramType = REGULATORY_DOMAIN_DISCONNECT_PARAM;
            regulatoryDomain_setParam(pConn->hRegulatoryDomain, pParam);

            /* Must be called AFTER mlme_stop. since De-Auth packet should be sent with the
            supported rates, and stopModules clears all rates. */
            stopModules(pConn, TI_TRUE);

            /* send disconnect command to firmware */
            prepare_send_disconnect(pData);
        }
    }

    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    return status;
}

static TI_STATUS connInfra_ScrWaitDisconn_to_disconnect(void *pData)
{
    TI_STATUS status;
    paramInfo_t *pParam;
    conn_t *pConn = (conn_t *)pData;
    
    status = rsn_stop(pConn->hRsn, pConn->disConEraseKeys);
    if (status != TI_OK)
        return status;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = CLOSE;
    status = rxData_setParam(pConn->hRxData, pParam);
    if (status == TI_OK) 
    {
        /* Update TxMgmtQueue SM to close Tx path for all except Mgmt packets. */
        txMgmtQ_SetConnState (pConn->hTxMgmtQ, TX_CONN_STATE_MGMT);

        pParam->paramType = REGULATORY_DOMAIN_DISCONNECT_PARAM;
        regulatoryDomain_setParam(pConn->hRegulatoryDomain, pParam);

        status = mlme_stop( pConn->hMlmeSm, DISCONNECT_IMMEDIATE, pConn->disConnReasonToAP );
        if (status == TI_OK) 
        {
            /* Must be called AFTER mlme_stop. since De-Auth packet should be sent with the
                supported rates, and stopModules clears all rates. */
            stopModules(pConn, TI_TRUE);
    
            /* send disconnect command to firmware */
            prepare_send_disconnect(pData);
        }
    }

    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    return status;
}


static TI_STATUS rsnWait_to_configHW(void *pData)
{
    conn_t *pConn=(conn_t *)pData;
    TI_STATUS status;
    paramInfo_t *pParam;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    /* Open the RX to DATA */
    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = OPEN;
    status = rxData_setParam(pConn->hRxData, pParam);
    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    if (status != TI_OK)
        return status;

    status = qosMngr_connect(pConn->hQosMngr);
    if (status != TI_OK)
    {
         TRACE2(pConn->hReport, REPORT_SEVERITY_ERROR, "Infra Conn status=%d, have to return (%d)\r\n",status,__LINE__);
         return status;
    }

    status = measurementMgr_connected(pConn->hMeasurementMgr);
    if (status != TI_OK)
    {
        TRACE2(pConn->hReport, REPORT_SEVERITY_ERROR, "Infra Conn status=%d, have to return (%d)\r\n",status,__LINE__);
        return status;
    }

    status = TrafficMonitor_Start(pConn->hTrafficMonitor);
    if (status != TI_OK)
    {
        TRACE2(pConn->hReport, REPORT_SEVERITY_ERROR, "Infra Conn status=%d, have to return (%d)\r\n",status,__LINE__);
        return status;
    }

    healthMonitor_setState(pConn->hHealthMonitor, HEALTH_MONITOR_STATE_CONNECTED);

    switchChannel_start(pConn->hSwitchChannel);

    scanCncn_SwitchToConnected (pConn->hScanCncn);

    PowerMgr_startPS(pConn->hPwrMngr);
    
    TRACE1(pConn->hReport, REPORT_SEVERITY_INFORMATION, "rsnWait_to_configHW: setStaStatus %d\r\n",STA_STATE_CONNECTED);
    TWD_CmdSetStaState(pConn->hTWD, STA_STATE_CONNECTED, (void *)conn_ConfigHwFinishCb, pData);

    return TI_OK;
}

/* last command of rsnWait_to_configHW callback */
int conn_ConfigHwFinishCb(TI_HANDLE pData)
{
    conn_t *pConn = (conn_t *)pData;

    TRACE0(pConn->hReport, REPORT_SEVERITY_INFORMATION, "conn_MboxFlushFinishCb: called \r\n");
    return conn_infraSMEvent(&pConn->state, CONN_INFRA_HW_CONFIGURED, pConn);
}

static TI_STATUS configHW_to_connected(void *pData)
{
    conn_t          *pConn=(conn_t *)pData;
    EScrResourceId  uResourceIndex;

    /* Update TxMgmtQueue SM to open Tx path to all packets. */
    txMgmtQ_SetConnState (((conn_t *)pData)->hTxMgmtQ, TX_CONN_STATE_OPEN);

#ifdef XCC_MODULE_INCLUDED
    XCCMngr_updateIappInformation(pConn->hXCCMngr, XCC_ASSOC_OK);
#endif

    /* Start keep alive process */
    siteMgr_start(pConn->hSiteMgr);

    /* free both SCR resources */
    for (uResourceIndex = SCR_RESOURCE_SERVING_CHANNEL;
         uResourceIndex < SCR_RESOURCE_NUM_OF_RESOURCES;
         uResourceIndex++)
    {
        scr_clientComplete(pConn->hScr, SCR_CID_CONNECT, uResourceIndex );
        pConn->scrRequested[ uResourceIndex ] = TI_FALSE;
    }

    /* Update current BSS connection type and mode */
    currBSS_updateConnectedState(pConn->hCurrBss, TI_TRUE, BSS_INFRASTRUCTURE);

    pConn->pConnStatusCB( pConn->connStatCbObj, STATUS_SUCCESSFUL, 0);
    
    SoftGemini_SetPSmode(pConn->hSoftGemini);
#ifdef REPORT_LOG  
    TRACE0(pConn->hReport, REPORT_SEVERITY_CONSOLE, "************ NEW CONNECTION ************\r\n"); 
    WLAN_OS_REPORT(("************ NEW CONNECTION ************\r\n"));
    siteMgr_printPrimarySiteDesc(pConn->hSiteMgr);
     TRACE0(pConn->hReport, REPORT_SEVERITY_CONSOLE, "****************************************\r\n"); 
    WLAN_OS_REPORT(("****************************************\r\n"));
#endif

    return TI_OK;
}


static TI_STATUS actionUnexpected(void *pData) 
{
#ifdef TI_DBG
    conn_t *pConn = (conn_t *)pData; 
    
    TRACE0(pConn->hReport, REPORT_SEVERITY_SM, "State machine error, unexpected Event\r\n\r\n");
#endif /*TI_DBG*/
    
    return TI_OK;
}

static TI_STATUS actionNop(void *pData) 
{
    return TI_OK;
}


static TI_STATUS connInfra_ScrWait(void *pData)
{
    conn_t *pConn = (conn_t *)pData;
    EScrClientRequestStatus scrReplyStatus[ SCR_RESOURCE_NUM_OF_RESOURCES ];
    EScePendReason          scrPendReason[ SCR_RESOURCE_NUM_OF_RESOURCES ];
    EScrResourceId          uResourceIndex;

    TRACE0( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Connnect SM: Requesting SCR.\r\n");

    /* request the SCR for both resources, and act according to return status */
    for (uResourceIndex = SCR_RESOURCE_SERVING_CHANNEL;
         uResourceIndex < SCR_RESOURCE_NUM_OF_RESOURCES;
         uResourceIndex++)
    {
        scrReplyStatus[ uResourceIndex ] = scr_clientRequest( pConn->hScr, SCR_CID_CONNECT, 
                                                              uResourceIndex, 
                                                              &(scrPendReason[ uResourceIndex ]));
        pConn->scrRequested[ uResourceIndex ] = TI_TRUE;

        /* sanity check */
        if (scrReplyStatus[ uResourceIndex ] > SCR_CRS_PEND)
        {
            TRACE2(pConn->hReport, REPORT_SEVERITY_ERROR , "Idle_to_ScrWait: SCR for resource %d returned status %d\r\n", uResourceIndex, scrReplyStatus[ uResourceIndex ]);
            return TI_NOK;
        }
    }

    /* analyze SCR results: */
    /* both returned run - continue to next stage */
    if ((SCR_CRS_RUN == scrReplyStatus[ SCR_RESOURCE_SERVING_CHANNEL ]) &&
        (SCR_CRS_RUN == scrReplyStatus[ SCR_RESOURCE_PERIODIC_SCAN ]))
    {
        /* send an SCR SUCCESS event to the SM */
        TRACE0( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Conn: SCR acquired.\r\n");
        conn_infraSMEvent(&pConn->state, CONN_INFRA_SCR_SUCC, (TI_HANDLE) pConn);
    }
    else
    {
        /* mark which resource is pending (or both) */
        for (uResourceIndex = SCR_RESOURCE_SERVING_CHANNEL;
             uResourceIndex < SCR_RESOURCE_NUM_OF_RESOURCES;
             uResourceIndex++)
        {
            if (SCR_CRS_PEND == scrReplyStatus[ uResourceIndex ])
            {
                TRACE2( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Conn: SCR pending for resource %d with pend reason: %d, stay in wait SCR state.\r\n", uResourceIndex, scrPendReason);
                pConn->bScrAcquired[ uResourceIndex ] = TI_FALSE;
            }
            else
            {
                pConn->bScrAcquired[ uResourceIndex ] = TI_TRUE;
            }
        }
    }
    return TI_OK;
}



void InfraConnSM_ScrCB( TI_HANDLE hConn, EScrClientRequestStatus requestStatus,
                        EScrResourceId eResource, EScePendReason pendReason )
{
    conn_t *pConn = (conn_t *)hConn;

    TRACE2( pConn->hReport, REPORT_SEVERITY_INFORMATION, "InfraConnSM_ScrCB called by SCR for resource %d. Status is: %d.\r\n", eResource, requestStatus);
    
    /* act according to the request staus */
    switch ( requestStatus )
    {
    case SCR_CRS_RUN:
        TRACE0( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Conn: SCR acquired.\r\n");
        /* mark that the SCR was acquired for this resource */
        pConn->bScrAcquired[ eResource ] = TI_TRUE;

        /* if both resources had now been acquired */
        if ((TI_TRUE == pConn->bScrAcquired[ SCR_RESOURCE_SERVING_CHANNEL ]) &&
            (TI_TRUE == pConn->bScrAcquired[ SCR_RESOURCE_PERIODIC_SCAN ]))
        {
            /* send an SCR SUCCESS event to the SM */
            conn_infraSMEvent(&pConn->state, CONN_INFRA_SCR_SUCC, (TI_HANDLE) pConn);
        }
        break;

	case SCR_CRS_FW_RESET:
        if (pConn->state == STATE_CONN_INFRA_WAIT_JOIN_CMPLT)
		{
			connInfra_JoinCmpltNotification((TI_HANDLE) pConn);
		}

        TRACE0( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Conn: Recovery occured.\r\n");
        break;

    default:
        TRACE3( pConn->hReport, REPORT_SEVERITY_ERROR, "Illegal SCR request status:%d, pend reason:%d, resource: %d.\r\n", requestStatus, pendReason, eResource);
        break;
    }
}



static TI_STATUS ScrWait_to_idle(void *pData)
{
    conn_t          *pConn = (conn_t *)pData;
    EScrResourceId  uResourceIndex;

    TRACE0( pConn->hReport, REPORT_SEVERITY_INFORMATION, "Infra Connnect SM: Stop event while in SCR wait, moving to IDLE.\r\n");

    /* free both SCR resources */
    for (uResourceIndex = SCR_RESOURCE_SERVING_CHANNEL;
         uResourceIndex < SCR_RESOURCE_NUM_OF_RESOURCES;
         uResourceIndex++)
    {
        scr_clientComplete(pConn->hScr, SCR_CID_CONNECT, uResourceIndex );
        pConn->scrRequested[ uResourceIndex ] = TI_FALSE;
    }

    /*
     * Call the connection lost callback set by the SME or AP_CONN.
     */
    pConn->pConnStatusCB( pConn->connStatCbObj, pConn->smContext.disAssocEventReason, pConn->smContext.disAssocEventStatusCode);

    return TI_OK;
}


static TI_STATUS stopModules( conn_t *pConn, TI_BOOL bDisconnect )
{
   
    measurementMgr_disconnected(pConn->hMeasurementMgr);

    rxData_stop(pConn->hRxData);

    ctrlData_stop(pConn->hCtrlData);
    
    TrafficMonitor_Stop(pConn->hTrafficMonitor);

    switchChannel_stop(pConn->hSwitchChannel);

    healthMonitor_setState(pConn->hHealthMonitor, HEALTH_MONITOR_STATE_DISCONNECTED);

    siteMgr_stop(pConn->hSiteMgr);

    /* stopping power save */
    PowerMgr_stopPS(pConn->hPwrMngr, bDisconnect);

    scanCncn_SwitchToNotConnected (pConn->hScanCncn);

    /* Set Current BSS Module to stop triggerring roaming events */
    currBSS_updateConnectedState(pConn->hCurrBss, TI_FALSE, BSS_INFRASTRUCTURE);

    SoftGemini_unSetPSmode(pConn->hSoftGemini);

    return TI_OK;
}


static TI_STATUS prepare_send_disconnect(void *pData)
{
    conn_t          *pConn = (conn_t *)pData;

    txCtrlParams_setEapolEncryptionStatus(pConn->hTxCtrl, DEF_EAPOL_ENCRYPTION_STATUS);
    qosMngr_disconnect (pConn->hQosMngr, TI_TRUE);

#ifdef XCC_MODULE_INCLUDED
    measurementMgr_disableTsMetrics(pConn->hMeasurementMgr, MAX_NUM_OF_AC);
#endif

    /* Start the disconnect complete time out timer. 
       Disconect Complete event, which stops the timer. */
    tmr_StartTimer (pConn->hConnTimer, conn_timeout, (TI_HANDLE)pConn, DISCONNECT_TIMEOUT_MSEC, TI_FALSE);

    /* FW will send the disconn frame according to disConnType */ 
    TWD_CmdFwDisconnect (pConn->hTWD, pConn->disConnType, pConn->disConnReasonToAP); 

#ifdef XCC_MODULE_INCLUDED
    XCCMngr_updateIappInformation(pConn->hXCCMngr, XCC_DISASSOC);
#endif

    return TI_OK;
}

static TI_STATUS connInfra_WaitDisconnectToIdle(void *pData)
{
    conn_t          *pConn = (conn_t *)pData;
    EScrResourceId  uResourceIndex;

    /* close all BA sessions */
    TWD_CloseAllBaSessions(pConn->hTWD);

    /* Stop the disconnect timeout timer. */
    tmr_StopTimer (pConn->hConnTimer);

    /*
     * In case of connection failuer we might get here without freeing the SCR.
     */
    for (uResourceIndex = SCR_RESOURCE_SERVING_CHANNEL;
         uResourceIndex < SCR_RESOURCE_NUM_OF_RESOURCES;
         uResourceIndex++)
    {
        if (pConn->scrRequested[ uResourceIndex ] == TI_TRUE)
        {
            scr_clientComplete(pConn->hScr, SCR_CID_CONNECT, uResourceIndex );
            pConn->scrRequested[ uResourceIndex ] = TI_FALSE;
        }
    }

    /*
     * Call the connection lost callback set by the SME or AP_CONN.
     */
    pConn->pConnStatusCB( pConn->connStatCbObj, pConn->smContext.disAssocEventReason, pConn->smContext.disAssocEventStatusCode);

    return TI_OK;
}

static TI_STATUS connect_to_ScrWait(void *pData)
{
    TI_STATUS   status;
    paramInfo_t *pParam;
    conn_t      *pConn = (conn_t *)pData;

    /*
     * This function performs roaming by two steps:
     * First - close the current connection without notify the SME.
     * Second - start new connection in reassociation mode.
     */ 

    /* close all BA sessions */
    TWD_CloseAllBaSessions(pConn->hTWD);

    status = rsn_stop(pConn->hRsn, pConn->disConEraseKeys);
    if (status != TI_OK)
        return status;

    pParam = (paramInfo_t *)os_memoryAlloc(pConn->hOs, sizeof(paramInfo_t));
    if (!pParam)
    {
        return TI_NOK;
    }

    pParam->paramType = RX_DATA_PORT_STATUS_PARAM;
    pParam->content.rxDataPortStatus = CLOSE;
    status = rxData_setParam(pConn->hRxData, pParam);
    if (status == TI_OK)
    {
        /* Update TxMgmtQueue SM to close Tx path. */
        txMgmtQ_SetConnState (((conn_t *)pData)->hTxMgmtQ, TX_CONN_STATE_CLOSE);

        status = mlme_stop(pConn->hMlmeSm, DISCONNECT_IMMEDIATE, pConn->disConnReasonToAP);
        if (status == TI_OK)
        {
            pParam->paramType = REGULATORY_DOMAIN_DISCONNECT_PARAM;
            regulatoryDomain_setParam(pConn->hRegulatoryDomain, pParam);

#ifdef XCC_MODULE_INCLUDED
            XCCMngr_updateIappInformation(pConn->hXCCMngr, XCC_DISASSOC);
#endif
        /* Must be called AFTER mlme_stop. since De-Auth packet should be sent with the
            supported rates, and stopModules clears all rates. */
            stopModules(pConn, TI_FALSE);

            txCtrlParams_setEapolEncryptionStatus(pConn->hTxCtrl, DEF_EAPOL_ENCRYPTION_STATUS);
            qosMngr_disconnect (pConn->hQosMngr, TI_FALSE);

        /* 
         * Start new connection.
         */ 
            connInfra_ScrWait(pConn);
        }
    }

    os_memoryFree(pConn->hOs, pParam, sizeof(paramInfo_t));
    return status;
}

static TI_STATUS Idle_to_Idle(void *pData)
{
    conn_t *pConn = (conn_t *)pData;

    /* 
     * In case we are in IDLE and getting DISCONNECT event, we need to inform
     * the SME\AP_connection that we are disconnected. 
     * Call the connection lost callback set by the SME or AP_CONN.
     */
    pConn->pConnStatusCB( pConn->connStatCbObj, pConn->smContext.disAssocEventReason, pConn->smContext.disAssocEventStatusCode);

    return TI_OK;
}

/***********************************************************************
                connInfra_JoinCmpltNotification
 ***********************************************************************
DESCRIPTION: Call back upon receving Join Event Complete.

INPUT:      hSiteMgr    -   site mgr handle.

OUTPUT:

RETURN:     
************************************************************************/
TI_STATUS connInfra_JoinCmpltNotification(TI_HANDLE hconn)
{
    conn_t *pConn = (conn_t *)hconn;
    
    TRACE0(pConn->hReport, REPORT_SEVERITY_INFORMATION, "connInfra_JoinCmpltNotification: has been called\r\n");

   if (pConn->currentConnType == CONNECTION_INFRA ) {
       conn_infraSMEvent(&pConn->state, CONN_INFRA_JOIN_CMD_CMPLT, pConn);
   }

   return TI_OK;
}

void connInfra_DisconnectComplete (conn_t *pConn, TI_UINT8  *data, TI_UINT8   dataLength)
{
    /* send an DISCONNECT COMPLETE event to the SM */
    conn_infraSMEvent(&pConn->state, CONN_INFRA_DISCONN_COMPLETE, (TI_HANDLE) pConn);
}
