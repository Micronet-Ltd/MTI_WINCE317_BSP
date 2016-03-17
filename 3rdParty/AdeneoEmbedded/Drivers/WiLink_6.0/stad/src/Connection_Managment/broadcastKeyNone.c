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
/** \file broadcastKeyNone.c
 * \brief broadcast key None implementation
 *
 * \see broadcastKeyNone.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	None station broadcast key SM                             *
 *   PURPOSE:   None station broadcast key SM implementation				*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_23
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"

#include "broadcastKeyNone.h"
#include "mainKeysSm.h"



TI_STATUS broadcastKeyNone_distribute(struct _broadcastKey_t *pBroadcastKey, encodedKeyMaterial_t *pEncodedKeyMaterial);
TI_STATUS broadcastKeyNone_start(struct _broadcastKey_t *pBroadcastKey);



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

TI_STATUS broadcastKeyNone_config(struct _broadcastKey_t *pBroadcastKey)
{

	pBroadcastKey->start = broadcastKeyNone_start;
	pBroadcastKey->stop = broadcastKeySmNop;
	pBroadcastKey->recvFailure = broadcastKeySmNop;
	pBroadcastKey->recvSuccess = broadcastKeyNone_distribute;

	pBroadcastKey->currentState = 0;

	return TI_OK;
}


/**
*
* broadcastKeyNone_start
*
* \b Description: 
*
* report the main key SM of broadcast complete, whithout wating for keys.
*
* \b ARGS:
*
*  I   - pBroadcastKey - context  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/
TI_STATUS broadcastKeyNone_start(struct _broadcastKey_t *pBroadcastKey)
{
	TI_STATUS	status=TI_NOK;

	if (pBroadcastKey->pParent->reportBcastStatus!=NULL)
    {
		status = pBroadcastKey->pParent->reportBcastStatus(pBroadcastKey->pParent, TI_OK);
    }

	return status;
}

/**
*
* broadcastKeyNone_distribute
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
TI_STATUS broadcastKeyNone_distribute(struct _broadcastKey_t *pBroadcastKey, encodedKeyMaterial_t *pEncodedKeyMaterial)
{
	TI_STATUS  status=TI_NOK;
	
	if (pBroadcastKey->pKeyDerive->derive!=NULL)
    {
        status = pBroadcastKey->pKeyDerive->derive(pBroadcastKey->pKeyDerive, 
                                                   pEncodedKeyMaterial);
    }
	if (status != TI_OK)
	{
		return TI_NOK;
	}

	if (pBroadcastKey->pParent->reportBcastStatus!=NULL)
    {
        status = pBroadcastKey->pParent->reportBcastStatus(pBroadcastKey->pParent, TI_OK);
    }

	return status;
}

