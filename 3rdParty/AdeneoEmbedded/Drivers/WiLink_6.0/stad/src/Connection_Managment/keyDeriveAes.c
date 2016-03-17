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
/** \file keyDeriveAes.c
 * \brief AES encryption key derivation implementation.
 *
 * \see aesBroadcastKeyDerivation.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	AES broadcast key derivation                                *
 *   PURPOSE:   AES broadcast key derivation                                *
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_29
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"
#include "keyDeriveAes.h"

#include "mainKeysSm.h"

/**
*
* keyDeriveAes_config
*
* \b Description: 
*
* AES broadcast key derivation configuration function: 
*			- Initializes the derive & remove callback functions
* \b ARGS: 
*
*  None
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveAes_config(struct _keyDerive_t *pKeyDerive)
{
	pKeyDerive->derive = keyDeriveAes_derive;
	pKeyDerive->remove = keyDeriveAes_remove;

	return TI_OK;
}


/**
*
* keyDeriveAes_derive
*
* \b Description: 
*
* AES key derivation function: 
*					- Decodes the key material.
*					- Distribute the decoded key material to the driver.
*
* \b ARGS: 
*
*  I - p - Pointer to the encoded key material.
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveAes_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
	TI_STATUS status;
	TSecurityKeys	key;
	keyMaterialAes_t   *keyMaterialAes = NULL;
	
	/* Small verification */
	if ((pEncodedKey==NULL) || (pKeyDerive == NULL))
	{
		return TI_NOK;
	}
	
    /* Note: Reduce 2 bytes from the size of keyMaterialAes_t in the following check, 
	         because it is added as padding at the end due to the OS_PACKED removal. */
    if ( pEncodedKey->keyLen < (sizeof(keyMaterialAes_t) - 2) )
	{
TRACE1(pKeyDerive->hReport, REPORT_SEVERITY_ERROR, "KEY_DERIVE_AES: ERROR: wrong key length %d !!!\n",						pEncodedKey->keyLen);
		return TI_NOK;
	}

	keyMaterialAes = (keyMaterialAes_t*)pEncodedKey->pData;

	
	/* Fill security key structure */
	os_memoryZero(pKeyDerive->hOs, &key, sizeof(TSecurityKeys));

	key.keyType   = KEY_AES;
	key.keyIndex  = (TI_UINT8)pEncodedKey->keyId;
	key.encLen    = DERIVE_AES_KEY_LEN;
	os_memoryCopy(pKeyDerive->hOs, (void *)key.encKey, pEncodedKey->pData + MAC_ADDR_LEN+KEY_RSC_LEN, 
		          DERIVE_AES_KEY_LEN);

	/* Copy MAC address key */
	MAC_COPY (key.macAddress, keyMaterialAes->macAddress);
	/* Copy RSC */
	os_memoryCopy(pKeyDerive->hOs, (void *)key.keyRsc, (void *)keyMaterialAes->keyRSC, KEY_RSC_LEN);

	status = pKeyDerive->pMainKeys->setKey(pKeyDerive->pMainKeys, &key);
	if (status == TI_OK)
	{
		os_memoryCopy(pKeyDerive->hOs, &pKeyDerive->key, pEncodedKey, sizeof(encodedKeyMaterial_t));
	}
	
	return status;
}

/**
*
* keyDeriveAes_remove
*
* \b Description: 
*
* AES key remove function: 
*			- Remove the key material from the driver.
*
* \b ARGS: 
*
*  None.
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveAes_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
	TI_STATUS status;
	TSecurityKeys	key;
	
	if ((pEncodedKey==NULL) || (pKeyDerive == NULL))
	{
		return TI_NOK;
	}
	
	os_memoryZero(pKeyDerive->hOs, &key, sizeof(TSecurityKeys));
	key.keyType  = KEY_AES;
	key.keyIndex = (TI_UINT8)pEncodedKey->keyId;
	key.encLen    = DERIVE_AES_KEY_LEN;
	MAC_COPY (key.macAddress, pEncodedKey->pData);

	status = pKeyDerive->pMainKeys->removeKey(pKeyDerive->pMainKeys, &key);
	if (status == TI_OK)
	{
		os_memoryZero(pKeyDerive->hOs, &pKeyDerive->key, sizeof(encodedKeyMaterial_t));
	}
	
	return status;
}

