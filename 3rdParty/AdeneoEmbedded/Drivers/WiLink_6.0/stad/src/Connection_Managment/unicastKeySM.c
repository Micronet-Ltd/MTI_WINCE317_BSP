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
/** \file unicastKeySM.c
 * \brief station unicast key SM implementation
 *
 * \see unicastKeySM.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	station unicast key SM		                                *
 *   PURPOSE:   station unicast key SM implementation						*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_46
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "unicastKeySM.h"
#include "unicastKey802_1x.h"
#include "unicastKeyNone.h"

/** number of states in the state machine */
#define	UCAST_KEY_MAX_NUM_STATES		3

/** number of events in the state machine */
#define	UCAST_KEY_MAX_NUM_EVENTS		4


/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
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

unicastKey_t* unicastKey_create(TI_HANDLE hOs)
{
	TI_STATUS				status;
	unicastKey_t 		*pUnicastKey;

	/* allocate key parser context memory */
	pUnicastKey = (unicastKey_t*)os_memoryAlloc(hOs, sizeof(unicastKey_t));
	if (pUnicastKey == NULL)
	{
		return NULL;
	}

	os_memoryZero(hOs, pUnicastKey, sizeof(unicastKey_t));

	/* allocate memory for association state machine */
	status = fsm_Create(hOs, &pUnicastKey->pUcastKeySm, UCAST_KEY_MAX_NUM_STATES, UCAST_KEY_MAX_NUM_EVENTS);
	if (status != TI_OK)
	{
		os_memoryFree(hOs, pUnicastKey, sizeof(unicastKey_t));
		return NULL;
	}

	pUnicastKey->pKeyDerive = keyDerive_create(hOs);
	if (pUnicastKey->pKeyDerive == NULL)
	{
		fsm_Unload(hOs, pUnicastKey->pUcastKeySm);
		os_memoryFree(hOs, pUnicastKey, sizeof(unicastKey_t));
		return NULL;
	}

	pUnicastKey->hOs = hOs;

	return pUnicastKey;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
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

TI_STATUS unicastKey_unload(struct _unicastKey_t *pUnicastKey)
{
	TI_STATUS		status;

	status = keyDerive_unload(pUnicastKey->pKeyDerive);
	if (status != TI_OK)
	{                  
        TRACE0(pUnicastKey->hReport, REPORT_SEVERITY_CONSOLE,"BCAST_KEY_SM: Error in unloading key derivation module\n");
		WLAN_OS_REPORT(("BCAST_KEY_SM: Error in unloading key derivation module\n"));
	}

	status = fsm_Unload(pUnicastKey->hOs, pUnicastKey->pUcastKeySm);
	if (status != TI_OK)
	{
        TRACE0(pUnicastKey->hReport, REPORT_SEVERITY_CONSOLE,"BCAST_KEY_SM: Error in unloading state machine\n");
		WLAN_OS_REPORT(("BCAST_KEY_SM: Error in unloading state machine\n"));
	}

	/* free key parser context memory */
	os_memoryFree(pUnicastKey->hOs, pUnicastKey, sizeof(unicastKey_t));

	return TI_OK;
}

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
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

TI_STATUS unicastKey_config(struct _unicastKey_t *pUnicastKey,
						 TRsnPaeConfig *pPaeConfig,
						 struct _mainKeys_t *pParent,
						 TI_HANDLE hReport,
						 TI_HANDLE hOs)
{
	TI_STATUS		status = TI_NOK;

	pUnicastKey->hReport = hReport;
	pUnicastKey->hOs = hOs;
	pUnicastKey->pParent = pParent;

	/* configure according to the keyMng suite and cipher suite */
    switch (pPaeConfig->keyExchangeProtocol)
    {
    case RSN_KEY_MNG_NONE:
       status = unicastKeyNone_config(pUnicastKey);
       break;
    case RSN_KEY_MNG_802_1X:
       if (pPaeConfig->unicastSuite == TWD_CIPHER_NONE)
   	    {
   	    	status = unicastKeyNone_config(pUnicastKey);
   	    } else {
   	    	status = unicastKey802_1x_config(pUnicastKey);
   	    }
   	break;
    default:
   	    status = unicastKeyNone_config(pUnicastKey);
   	    break;
    }

	status = keyDerive_config(pUnicastKey->pKeyDerive, pPaeConfig->unicastSuite, pParent, hReport, hOs);
	
	return status;
}


TI_STATUS unicastKeySmUnexpected(struct _unicastKey_t *pUnicastKey)
{
TRACE0(pUnicastKey->hReport, REPORT_SEVERITY_ERROR, "UNICAST_KEY_SM: ERROR: UnExpected Event\n");

	return(TI_NOK);
}

TI_STATUS unicastKeySmNop(struct _unicastKey_t *pUnicastKey)
{
	return(TI_OK);
}

