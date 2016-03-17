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
/** \file unicastKeyNone.c
 * \brief station unicast key None implementation
 *
 * \see unicastKeyNone.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	station unicast key None		                            *
 *   PURPOSE:   station unicast key None implementation						*
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_45
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "unicastKeyNone.h"
#include "mainKeysSm.h"


TI_STATUS unicastKeyNone_start(struct _unicastKey_t *pUnicastKey);
TI_STATUS unicastKeyNone_distribute(struct _unicastKey_t *pUnicastKey, encodedKeyMaterial_t *pEncodedKeyMaterial);



/**
*
* Function  - Config KEY Parser module.
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

TI_STATUS unicastKeyNone_config(struct _unicastKey_t *pUnicastKey)
{

	pUnicastKey->start = unicastKeyNone_start;
	pUnicastKey->stop = unicastKeySmNop;
	pUnicastKey->recvFailure = unicastKeySmNop;
	pUnicastKey->recvSuccess = unicastKeyNone_distribute;

	pUnicastKey->currentState = 0;

	
	return TI_OK;
}

/**
*
* unicastKeyNone_start
*
* \b Description: 
*
* report the main key SM of unicast complete, whithout wating for keys.
*
* \b ARGS:
*
*  I   - pUnicastKey - context  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/
TI_STATUS unicastKeyNone_start(struct _unicastKey_t *pUnicastKey)
{
	TI_STATUS	status=TI_NOK;

	if (pUnicastKey->pParent->reportUcastStatus!=NULL)
    {
		status = pUnicastKey->pParent->reportUcastStatus(pUnicastKey->pParent, TI_OK);
    }

	return status;
}

/**
*
* unicastKeyNone_distribute
*
* \b Description: 
*
* Distribute unicast key material to the driver and report the main key SM on unicast complete.
*
* \b ARGS:
*
*  I   - pData - Encoded key material  \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/
TI_STATUS unicastKeyNone_distribute(struct _unicastKey_t *pUnicastKey, encodedKeyMaterial_t *pEncodedKeyMaterial)
{
	TI_STATUS  status=TI_NOK;
	
	if ((pUnicastKey==NULL) || (pEncodedKeyMaterial==NULL))
    {
        return TI_NOK;
    }
    
    if (pUnicastKey->pKeyDerive->derive!=NULL)
    {
        status = pUnicastKey->pKeyDerive->derive(pUnicastKey->pKeyDerive, 
                                                       pEncodedKeyMaterial);
    }
	if (status != TI_OK)
	{
		return TI_NOK;
	}

	if (pUnicastKey->pParent->setDefaultKeyId!=NULL)
    {
        status = pUnicastKey->pParent->setDefaultKeyId(pUnicastKey->pParent,
                                                       (TI_UINT8)pEncodedKeyMaterial->keyId);
    }
	if (status != TI_OK)
	{
		return status;
	}

	if (pUnicastKey->pParent->reportUcastStatus!=NULL)
    {
        status = pUnicastKey->pParent->reportUcastStatus(pUnicastKey->pParent, TI_OK);
    }

	return status;
}
