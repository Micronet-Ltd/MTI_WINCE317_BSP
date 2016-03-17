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
/** \file externalSec.c
 * \brief station externalSec implementation
 *
 * \see externalSec.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	station externalSec		                                    *
 *   PURPOSE:   station uexternalSec implementation						    *
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_134
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"
#include "smeApi.h"
#include "mainSecSm.h"
#include "externalSec.h"
#include "connApi.h"

TI_STATUS externalSecSM_Nop(struct externalSec_t *pExternalSec);
TI_STATUS externalSecSM_start(mainSec_t *pMainSec);
TI_STATUS externalSecSM_stop(mainSec_t *pMainSec);
TI_STATUS externalSecSM_setPort(struct externalSec_t *pExternalSec);
TI_STATUS externalSecSM_Unexpected(struct externalSec_t *pExternalSec);
/**
*
* Function  - externalSec_config.
*
* \b Description: 
*
* Called by mainSecSM (mainSec_config). 
* builds the SM and register the mainSec start and stop events.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
TI_STATUS externalSec_config(mainSec_t *pMainSec)
{
    struct externalSec_t	 *pExtSec = pMainSec-> pExternalSec;
	TI_STATUS                status = TI_NOK;
	
	/** Station externalSec State Machine matrix */
    fsm_actionCell_t externalSec_matrix[EXTERNAL_SEC_NUM_STATES][EXTERNAL_SEC_NUM_EVENTS] =
	{
    	/* next state and actions for IDLE state */
        {	
            {EXTERNAL_SEC_STATE_WAIT,(fsm_Action_t)externalSecSM_Nop},       /*EXTERNAL_SEC_EVENT_START */	  
            {EXTERNAL_SEC_STATE_IDLE,(fsm_Action_t)externalSecSM_Unexpected},       /*EXTERNAL_SEC_EVENT_COMPLETE*/
            {EXTERNAL_SEC_STATE_IDLE,(fsm_Action_t)externalSecSM_Nop}        /*EXTERNAL_SEC_EVENT_STOP */
        },
    	
    	/* next state and actions for Wait state */
    	{	
            {EXTERNAL_SEC_STATE_WAIT,(fsm_Action_t)externalSecSM_Unexpected},/*EXTERNAL_SEC_EVENT_START */
            {EXTERNAL_SEC_STATE_IDLE,(fsm_Action_t)externalSecSM_setPort},    /*EXTERNAL_SEC_EVENT_COMPLETE*/
            {EXTERNAL_SEC_STATE_IDLE,(fsm_Action_t)externalSecSM_Nop}        /*EXTERNAL_SEC_EVENT_STOP */
    	}
	};

    pExtSec->hOs = pMainSec->hOs;
    pExtSec->hReport = pMainSec->hReport;
    pExtSec->pParent = pMainSec;
    pMainSec->start = (mainSecSmStart_t)externalSecSM_start;
    pMainSec->stop = (mainSecSmStart_t)externalSecSM_stop;
    pExtSec->currentState = EXTERNAL_SEC_STATE_IDLE;

    status = fsm_Config(pExtSec->pExternalSecSm, 
						&externalSec_matrix[0][0], 
						EXTERNAL_SEC_NUM_STATES, 
						EXTERNAL_SEC_NUM_EVENTS, 
						NULL, pExtSec->hOs);


	return status;
}



/**
*
* Function  - externalSec_create.
*
* \b Description: 
*
* Called by mainSecSM (mainSec_create). 
* Registers the function 'rsn_UnicastKeyRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
struct externalSec_t* externalSec_create(TI_HANDLE hOs)
{
    struct externalSec_t    *pHandle;
    TI_STATUS		        status;
	
    /* allocate association context memory */
    pHandle = (struct externalSec_t*)os_memoryAlloc(hOs, sizeof(struct externalSec_t));
    if (pHandle == NULL)
    {
        return NULL;
    }
	
    os_memoryZero(hOs, pHandle, sizeof(struct externalSec_t));
	
    /* allocate memory for association state machine */
    status = fsm_Create(hOs,&pHandle->pExternalSecSm, EXTERNAL_SEC_NUM_STATES, EXTERNAL_SEC_NUM_EVENTS);

    if (status != TI_OK)
    {
        os_memoryFree(hOs, pHandle, sizeof(struct externalSec_t));
        return NULL;
    }
	
    return pHandle;
}


/**
*
* Function  - externalSec_Destroy.
*
* \b Description:
*
* Called by mainSecSM (mainSec_unload).
*
* \b ARGS:
*
*
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure.
*
*/
TI_STATUS externalSec_Destroy (struct externalSec_t *pExternalSec)
{
    TI_STATUS	status;

    if (pExternalSec == NULL)
    {
        return TI_NOK;
    }

    status = fsm_Unload (pExternalSec->hOs, pExternalSec->pExternalSecSm);
    if (status != TI_OK)
    {
        /* report failure but don't stop... */
        TRACE0(pExternalSec->hReport, REPORT_SEVERITY_ERROR, "EXTERNAL SECURITY: Error releasing FSM memory \n");
    }

    os_memoryFree (pExternalSec->hOs, pExternalSec, sizeof(struct externalSec_t));

    return TI_OK;
}


/**
*
* Function  - externalSecSM_start.
*
* \b Description: 
*
* Called upon the EXTERNAL_SEC_EVENT_START event  
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
TI_STATUS externalSecSM_start(mainSec_t *pMainSec)
{
	/* called by the rsn_start() */
	return externalSec_event(pMainSec->pExternalSec, EXTERNAL_SEC_EVENT_START, pMainSec->pExternalSec);
}

/**
*
* Function  - externalSecSM_stop.
*
* \b Description: 
*
* Called upon the EXTERNAL_SEC_EVENT_STOP event  
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
TI_STATUS externalSecSM_stop(mainSec_t *pMainSec)
{
	/* called by the rsn_stop() */
    return externalSec_event(pMainSec->pExternalSec, EXTERNAL_SEC_EVENT_STOP, pMainSec->pExternalSec);
}

/**
*
* Function  - externalSec_event.
*
* \b Description: 
*
* Called by the  rsn_PortStatus_Set() API upon external set port status cmd. 
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
TI_STATUS externalSec_event(struct externalSec_t *pExternalSec, TI_UINT8 event, void *pData)
{
    TI_STATUS   status;
    TI_UINT8    nextState;

    status = fsm_GetNextState(pExternalSec->pExternalSecSm,
                              pExternalSec->currentState,
                              event,
                              &nextState);
    if (status != TI_OK)
    {
        TRACE0(pExternalSec->hReport, REPORT_SEVERITY_ERROR, "EXTERNAL_SEC_SM: ERROR: failed getting next state\n");
        return TI_NOK;
	}

    TRACE3(pExternalSec->hReport, REPORT_SEVERITY_INFORMATION, "STATION_EXT_SEC_SM: <currentState = %d, event = %d> --> nextState = %d\n", pExternalSec->currentState, event, nextState);
    status = fsm_Event(pExternalSec->pExternalSecSm,
                       &pExternalSec->currentState,
                       event,
                       pData);

    return status;
}


/**
*
* Function  - externalSecSM_setPort.
*
* \b Description: 
*
* Call the connection report status API. 
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/
TI_STATUS externalSecSM_setPort(struct externalSec_t *pExternalSec)
{
    TI_STATUS       status = TI_OK;	
    struct _rsn_t   *pRsn;

    pRsn = pExternalSec->pParent->pParent;
    if (TI_TRUE == pExternalSec->bPortStatus)
    {
        status  = conn_reportRsnStatus(pRsn->hConn, (mgmtStatus_e)STATUS_SUCCESSFUL);
    }
    else
    {
        status = conn_reportRsnStatus(pRsn->hConn, (mgmtStatus_e)STATUS_SECURITY_FAILURE);
    }
	
    return status;
}


TI_STATUS externalSec_rsnComplete(struct externalSec_t *pExternalSec)
{
	return externalSec_event(pExternalSec, EXTERNAL_SEC_EVENT_COMPLETE, pExternalSec);
}

/**
*
* Function  - externalSecSM_Nop.
*
* \b Description: 
*
* Do nothing
*
* \b ARGS:
*
* \b RETURNS: TI_OK
*
*/
TI_STATUS externalSecSM_Nop(struct externalSec_t *pExternalSec)
{
    return(TI_OK);
}

/**
*
* Function  - externalSecSM_Unexpected.
*
* \b Description: 
*
* Do nothing
*
* \b ARGS:
*
* \b RETURNS: TI_STATUS
*
*/
TI_STATUS externalSecSM_Unexpected(struct externalSec_t *pExternalSec)
{
    TRACE0(pExternalSec->hReport, REPORT_SEVERITY_ERROR, "EXTERNAL_SEC_SM: ERROR UnExpected Event\n");
    return(TI_OK);
}

