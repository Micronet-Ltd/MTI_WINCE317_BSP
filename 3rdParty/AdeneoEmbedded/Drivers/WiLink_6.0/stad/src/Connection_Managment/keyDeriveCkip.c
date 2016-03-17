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
/** \file wepBroadcastKeyDerivation.c
 * \brief WEP broadcast key derivation implementation.
 *
 * \see wepBroadcastKeyDerivation.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	WEP broadcast key derivation                                *
 *   PURPOSE:   WEP broadcast key derivation                                *
 *                                                                          *
 ****************************************************************************/

#ifdef XCC_MODULE_INCLUDED
#define __FILE_ID__  FILE_ID_30
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"


#include "keyDerive.h"
#include "keyDeriveCkip.h"

#include "mainKeysSm.h"
#include "mainSecSm.h"
#include "admCtrl.h"

/**
*
* keyDeriveCkip_config
*
* \b Description: 
*
* CKIP key derivation init function: 
*							- Initializes the derive & remove callback functions
*							- Resets the key material in the system control block								
*
* \b ARGS: 
*
*  None
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveCkip_config(struct _keyDerive_t *pKeyDerive)
{
    pKeyDerive->derive = keyDeriveCkip_derive;
    pKeyDerive->remove = keyDeriveCkip_remove;

    return TI_OK;
}


/**
*
* keyDeriveCkip_derive
*
* \b Description: 
*
* CKIP key derivation function: 
*							- Decodes the key material.
*							- Distribute the decoded key material to the driver.
*
* \b ARGS: 
*
*  I - p - Pointer to the encoded key material.
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveCkip_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
    TI_STATUS           status;
    TSecurityKeys      key;
    TI_UINT8               ckipIndex, keyIndex;
    TI_UINT8               ckipKey[KEY_DERIVE_CKIP_ENC_LEN];

    key.keyType = KEY_XCC;
    key.keyIndex = (TI_UINT8)pEncodedKey->keyId;

    if (pEncodedKey->keyLen != KEY_DERIVE_CKIP_ENC_LEN)
    {
        if ((pEncodedKey->keyLen != KEY_DERIVE_CKIP_5_LEN) && (pEncodedKey->keyLen != KEY_DERIVE_CKIP_13_LEN))
        {
            TRACE1(pKeyDerive->hReport, REPORT_SEVERITY_ERROR, "KEY_DERIVE_CKIP: ERROR: wrong key length %d !!!\n", pEncodedKey->keyLen);
            return TI_NOK;
        }

        keyIndex=0;
        for (ckipIndex=0; ckipIndex<KEY_DERIVE_CKIP_ENC_LEN; ckipIndex++)
        {
           ckipKey[ckipIndex]= pEncodedKey->pData[keyIndex];
		   keyIndex++;
		   if (keyIndex >= pEncodedKey->keyLen)
		   {
				keyIndex = 0;
		   }
           /*keyIndex = ((keyIndex+1) <pEncodedKey->keyLen) ? keyIndex+1 : 0;*/
        }
    }
    else 
    {
        for (ckipIndex=0; ckipIndex<KEY_DERIVE_CKIP_ENC_LEN; ckipIndex++)
        {
           ckipKey[ckipIndex]= pEncodedKey->pData[ckipIndex];
        }
    }

    if (pKeyDerive->pMainKeys->pParent->pParent->pAdmCtrl->encrInSw)
    {
        key.encLen = KEY_DERIVE_CKIP_ENC_LEN;
    }
    else
    {
        key.encLen = pEncodedKey->keyLen;
    }
    
    /* Copy encryption key - not expand */
    os_memoryCopy(pKeyDerive->hOs, (void*)key.encKey, ckipKey, key.encLen);
    /* Copy the MIC keys */
    os_memoryCopy(pKeyDerive->hOs, (void*)key.micRxKey, ckipKey, KEY_DERIVE_CKIP_ENC_LEN);
    os_memoryCopy(pKeyDerive->hOs, (void*)key.micTxKey, ckipKey, KEY_DERIVE_CKIP_ENC_LEN);

    status = pKeyDerive->pMainKeys->setKey(pKeyDerive->pMainKeys, &key);
    if (status == TI_OK)
    {
        os_memoryCopy(pKeyDerive->hOs, &pKeyDerive->key, pEncodedKey, sizeof(encodedKeyMaterial_t));
    }

    return status;
}
 
/**
*
* wepBroadcastKeyDerivationRemove
*
* \b Description: 
*
* WEP broadcast key removal function: 
*							- Remove the key material from the driver.
*
* \b ARGS: 
*
*  None.
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*/

TI_STATUS keyDeriveCkip_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
    TI_STATUS status;
    TSecurityKeys  key;

    os_memoryZero(pKeyDerive->hOs, &key, sizeof(TSecurityKeys));
    key.keyType = KEY_XCC;
    key.keyIndex = (TI_UINT8)pEncodedKey->keyId;
    key.encLen = KEY_DERIVE_CKIP_ENC_LEN;
	MAC_COPY (key.macAddress, pEncodedKey->pData);

    status = pKeyDerive->pMainKeys->removeKey(pKeyDerive->pMainKeys, &key);
    if (status == TI_OK)
    {
        os_memoryZero(pKeyDerive->hOs, &pKeyDerive->key, sizeof(encodedKeyMaterial_t));
    }

    return status;
}

#endif /* XCC_MODULE_INCLUDED */

