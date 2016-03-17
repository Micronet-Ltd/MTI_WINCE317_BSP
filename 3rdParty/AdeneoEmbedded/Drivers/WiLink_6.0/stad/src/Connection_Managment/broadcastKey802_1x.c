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
/** \file broadcastKey802_1x.c
 * \brief broadcast key 802.1x implementation
 *
 * \see broadcastKey802_1x.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	802.1x station broadcast key SM                             *
 *   PURPOSE:   802.1x station broadcast key SM implementation				*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_22
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"

#include "broadcastKey802_1x.h"
#include "mainKeysSm.h"


TI_STATUS broadcastKey802_1x_start(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey802_1x_stop(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey802_1x_recvSuccess(struct _broadcastKey_t *pBroadcastKey, 
									encodedKeyMaterial_t *pEncodedKeyMaterial);

TI_STATUS broadcastKey802_1x_recvFailure(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey802_1x_distribute(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey802_1x_redistribute(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey802_1x_event(struct _broadcastKey_t *pBroadcastKey,
							  TI_UINT8 event, 
							  void *pData);


/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_BroadcastKeyRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS broadcastKey802_1x_config(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS		status = TI_NOK;
	
	/** Station broadcast key State Machine matrix */
	fsm_actionCell_t    broadcastKey802_1x_matrix[BCAST_KEY_802_1X_NUM_STATES][BCAST_KEY_802_1X_NUM_EVENTS] =
	{
		/* next state and actions for IDLE state */
		{	{BCAST_KEY_802_1X_STATE_START, (fsm_Action_t)broadcastKeySmNop},
			{BCAST_KEY_802_1X_STATE_IDLE, (fsm_Action_t)broadcastKeySmNop},
			{BCAST_KEY_802_1X_STATE_IDLE, (fsm_Action_t)broadcastKeySmNop},
			{BCAST_KEY_802_1X_STATE_IDLE, (fsm_Action_t)broadcastKeySmUnexpected}
		},
	
		/* next state and actions for START state */
		{	{BCAST_KEY_802_1X_STATE_START, (fsm_Action_t)broadcastKeySmUnexpected},
			{BCAST_KEY_802_1X_STATE_IDLE, (fsm_Action_t)broadcastKeySmNop},
			{BCAST_KEY_802_1X_STATE_COMPLETE, (fsm_Action_t)broadcastKey802_1x_distribute},
			{BCAST_KEY_802_1X_STATE_START, (fsm_Action_t)broadcastKeySmNop}
		},
	
		/* next state and actions for COMPLETE state */
		{	{BCAST_KEY_802_1X_STATE_COMPLETE, (fsm_Action_t)broadcastKeySmUnexpected},
			{BCAST_KEY_802_1X_STATE_IDLE, (fsm_Action_t)broadcastKeySmNop},
			{BCAST_KEY_802_1X_STATE_COMPLETE, (fsm_Action_t)broadcastKey802_1x_distribute},
			{BCAST_KEY_802_1X_STATE_COMPLETE, (fsm_Action_t)broadcastKeySmUnexpected}
		}
	};


	pBroadcastKey->start = broadcastKey802_1x_start;
	pBroadcastKey->stop = broadcastKey802_1x_stop;
	pBroadcastKey->recvFailure = broadcastKey802_1x_recvFailure;
	pBroadcastKey->recvSuccess = broadcastKey802_1x_recvSuccess;

	pBroadcastKey->currentState = BCAST_KEY_802_1X_STATE_IDLE;

	status = fsm_Config(pBroadcastKey->pBcastKeySm, 
						&broadcastKey802_1x_matrix[0][0], 
						BCAST_KEY_802_1X_NUM_STATES, 
						BCAST_KEY_802_1X_NUM_EVENTS, 
						NULL, pBroadcastKey->hOs);



	return status;
}


/**
*
* broadcastKey802_1x_event
*
* \b Description: 
*
* broadcast key state machine transition function
*
* \b ARGS:
*
*  I/O - currentState - current state in the state machine\n
*  I   - event - specific event for the state machine\n
*  I   - pData - Data for state machine action function\n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS broadcastKey802_1x_event(struct _broadcastKey_t *pBroadcastKey, TI_UINT8 event, void *pData)
{
	TI_STATUS 		status;
	TI_UINT8			nextState;

	status = fsm_GetNextState(pBroadcastKey->pBcastKeySm, pBroadcastKey->currentState, event, &nextState);
	if (status != TI_OK)
	{
TRACE0(pBroadcastKey->hReport, REPORT_SEVERITY_ERROR, "BROADCAST_KEY_SM: ERROR: failed getting next state\n");
		return TI_NOK;
	}

TRACE3(pBroadcastKey->hReport, REPORT_SEVERITY_INFORMATION, "STATION_BROADCAST_KEY_SM: <currentState = %d, event = %d> --> nextState = %d\n", pBroadcastKey->currentState, event, nextState);

	status = fsm_Event(pBroadcastKey->pBcastKeySm, &pBroadcastKey->currentState, event, pData);

	return status;
}


/**
*
* broadcastKey802_1x_start
*
* \b Description: 
*
* START event handler
*
* \b ARGS:
*
*  I   - pCtrlB - station control block  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa broadcastKey802_1x_stop()
*/
TI_STATUS broadcastKey802_1x_start(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS  status;
	
	status = broadcastKey802_1x_event(pBroadcastKey, BCAST_KEY_802_1X_EVENT_START, pBroadcastKey);

	return status;
}


/**
*
* broadcastKey802_1x_stop
*
* \b Description: 
*
* START event handler
*
* \b ARGS:
*
*  I   - pCtrlB - station control block  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa broadcastKey802_1x_start()
*/
TI_STATUS broadcastKey802_1x_stop(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS  status;

	status = broadcastKey802_1x_event(pBroadcastKey, BCAST_KEY_802_1X_EVENT_STOP, pBroadcastKey);

	return status;
}


/**
*
* broadcastKey802_1x_recvSuccess
*
* \b Description: 
*
* SUCCESS event handler
*
* \b ARGS:
*
*  I   - pCtrlB - station control block  \n
*  I   - pEncodedKeyMaterial - Encoded key material \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
*/
TI_STATUS broadcastKey802_1x_recvSuccess(struct _broadcastKey_t *pBroadcastKey, encodedKeyMaterial_t *pEncodedKeyMaterial)
{
	TI_STATUS  status;

	pBroadcastKey->data.pEncodedKeyMaterial = pEncodedKeyMaterial;

	status = broadcastKey802_1x_event(pBroadcastKey, BCAST_KEY_802_1X_EVENT_SUCCESS, pBroadcastKey);

	return status;
}


/**
*
* broadcastKey802_1x_recvFailure
*
* \b Description: 
*
* FAILURE event handler
*
* \b ARGS:
*
*  I   - pCtrlB - station control block  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
*/
TI_STATUS broadcastKey802_1x_recvFailure(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS  status;
	
	status = broadcastKey802_1x_event(pBroadcastKey, BCAST_KEY_802_1X_EVENT_FAILURE, pBroadcastKey);

	return status;
}


/**
*
* broadcastKey802_1x_distribute
*
* \b Description: 
*
* Distribute broadcast key material to the driver and report the main key SM on broadcast complete.
*
* \b ARGS:
*
*  I   - pData - Encoded key material  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/
TI_STATUS broadcastKey802_1x_distribute(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS  status=TI_NOK;
	
	if (pBroadcastKey->pKeyDerive->derive!=NULL)
    {
	status = pBroadcastKey->pKeyDerive->derive(pBroadcastKey->pKeyDerive, 
											   pBroadcastKey->data.pEncodedKeyMaterial);
    }
	if (status != TI_OK)
	{
		return TI_NOK;
	}

	if (pBroadcastKey->pParent->reportBcastStatus!=NULL)
    {
	status = pBroadcastKey->pParent->reportBcastStatus(pBroadcastKey->pParent, TI_OK);
    }

	mainKeys_reAuth(pBroadcastKey->pParent);

	return status;
}

