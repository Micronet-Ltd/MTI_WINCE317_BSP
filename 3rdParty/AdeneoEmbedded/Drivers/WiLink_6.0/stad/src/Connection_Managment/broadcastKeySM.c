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
/** \file broadcastKeySM.c
 * \brief broadcast key SM implementation
 *
 * \see broadcastKeySM.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	broadcast key SM                               *
 *   PURPOSE:   broadcast key SM implementation				*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_24
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"

#include "broadcastKeySM.h"
#include "broadcastKey802_1x.h"
#include "broadcastKeyNone.h"

/** number of states in the state machine */
#define	BCAST_KEY_MAX_NUM_STATES		3

/** number of events in the state machine */
#define	BCAST_KEY_MAX_NUM_EVENTS		4


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

broadcastKey_t* broadcastKey_create(TI_HANDLE hOs)
{
	TI_STATUS				status;
	broadcastKey_t 		*pBroadcastKey;

	/* allocate key parser context memory */
	pBroadcastKey = (broadcastKey_t*)os_memoryAlloc(hOs, sizeof(broadcastKey_t));
	if (pBroadcastKey == NULL)
	{
		return NULL;
	}

	os_memoryZero(hOs, pBroadcastKey, sizeof(broadcastKey_t));

	/* allocate memory for association state machine */
	status = fsm_Create(hOs, &pBroadcastKey->pBcastKeySm, BCAST_KEY_MAX_NUM_STATES, BCAST_KEY_MAX_NUM_EVENTS);
	if (status != TI_OK)
	{
		os_memoryFree(hOs, pBroadcastKey, sizeof(broadcastKey_t));
		return NULL;
	}

	pBroadcastKey->pKeyDerive = keyDerive_create(hOs);
	if (pBroadcastKey->pKeyDerive == NULL)
	{
		fsm_Unload(hOs, pBroadcastKey->pBcastKeySm);
		os_memoryFree(hOs, pBroadcastKey, sizeof(broadcastKey_t));
		return NULL;
	}

	pBroadcastKey->hOs = hOs;

	return pBroadcastKey;
}

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

TI_STATUS broadcastKey_unload(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS		status;

	status = keyDerive_unload(pBroadcastKey->pKeyDerive);
                    
	if (status != TI_OK)
	{
        TRACE0(pBroadcastKey->hReport, REPORT_SEVERITY_CONSOLE, "BCAST_KEY_SM: Error in unloading key derivation module\n");
		WLAN_OS_REPORT(("BCAST_KEY_SM: Error in unloading key derivation module\n"));
	}

	status = fsm_Unload(pBroadcastKey->hOs, pBroadcastKey->pBcastKeySm);
	if (status != TI_OK)
	{
        TRACE0(pBroadcastKey->hReport, REPORT_SEVERITY_CONSOLE, "BCAST_KEY_SM: Error in unloading state machine\n");
		WLAN_OS_REPORT(("BCAST_KEY_SM: Error in unloading state machine\n"));
	}

	/* free key parser context memory */
	os_memoryFree(pBroadcastKey->hOs, pBroadcastKey, sizeof(broadcastKey_t));

	return TI_OK;
}

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

TI_STATUS broadcastKey_config(struct _broadcastKey_t *pBroadcastKey,
                              TRsnPaeConfig *pPaeConfig,
                              struct _mainKeys_t *pParent,
                              TI_HANDLE hReport,
                              TI_HANDLE hOs)
{
	TI_STATUS		status = TI_NOK;

	pBroadcastKey->hReport = hReport;
	pBroadcastKey->hOs = hOs;
	pBroadcastKey->pParent = pParent;

	/* configure according to the keyMng suite and cipher suite */
	switch (pPaeConfig->keyExchangeProtocol)
	{
	case RSN_KEY_MNG_NONE:
		status = broadcastKeyNone_config(pBroadcastKey);
		break;
	case RSN_KEY_MNG_802_1X:
		if (pPaeConfig->broadcastSuite == TWD_CIPHER_NONE)
		{
			status = broadcastKeyNone_config(pBroadcastKey);
		} else {
			status = broadcastKey802_1x_config(pBroadcastKey);
		}
		break;
    default:
		status = broadcastKeyNone_config(pBroadcastKey);
		break;
	}

	status = keyDerive_config(pBroadcastKey->pKeyDerive, pPaeConfig->broadcastSuite, pParent, hReport, hOs);

	return status;
}


TI_STATUS broadcastKeySmUnexpected(struct _broadcastKey_t *pBroadcastKey)
{
TRACE0(pBroadcastKey->hReport, REPORT_SEVERITY_ERROR, "BROADCAST_KEY_SM: ERROR: UnExpected Event\n");

	return(TI_NOK);
}

TI_STATUS broadcastKeySmNop(struct _broadcastKey_t *pBroadcastKey)
{
	return(TI_OK);
}

