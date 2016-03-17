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

#define __FILE_ID__  FILE_ID_31
#include "osApi.h"
#include "report.h"
#include "rsnApi.h"

#include "keyDerive.h"
#include "keyDeriveTkip.h"

#include "mainKeysSm.h"

/**
*
* keyDeriveTkip_config
*
* \b Description: 
*
* TKIP key derivation init function: 
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

TI_STATUS keyDeriveTkip_config(struct _keyDerive_t *pKeyDerive)
{
    pKeyDerive->derive = keyDeriveTkip_derive;
    pKeyDerive->remove = keyDeriveTkip_remove;

    return TI_OK;
}


/**
*
* keyDeriveTkip_derive
*
* \b Description: 
*
* TKIP key derivation function: 
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

TI_STATUS keyDeriveTkip_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
    TI_STATUS           status;
    TSecurityKeys      key;
    keyMaterialTkip_t   *keyMaterialTkip;
    
    if (pEncodedKey==NULL)
    {
        return TI_NOK;
    }

    key.keyType = KEY_TKIP;
    key.keyIndex = (TI_UINT8)pEncodedKey->keyId;
    key.encLen = KEY_DERIVE_TKIP_ENC_LEN;

    /* Note: Reduce 2 bytes from the size of keyMaterialTkip_t in the following check, 
	         because it is added as padding at the end due to the OS_PACKED removal. */
    if ( pEncodedKey->keyLen < (sizeof(keyMaterialTkip_t) - 2) )
    {
        TRACE1(pKeyDerive->hReport, REPORT_SEVERITY_ERROR, "KEY_DERIVE_TKIP: ERROR: wrong key length %d !!!\n", pEncodedKey->keyLen);
        return TI_NOK;
    }

    keyMaterialTkip = (keyMaterialTkip_t*)pEncodedKey->pData;
    /* Copy encryption key */
    os_memoryCopy(pKeyDerive->hOs, (void *)key.encKey, (void *)keyMaterialTkip->encKey, KEY_DERIVE_TKIP_ENC_LEN);
    if (pEncodedKey->keyId & 0x10000000)
    {   /* Copy MIC RX */
        os_memoryCopy(pKeyDerive->hOs, (void *)key.micTxKey, (void *)keyMaterialTkip->micRxKey, KEY_DERIVE_TKIP_MIC_LEN);
        /* Copy MIC RX */
        os_memoryCopy(pKeyDerive->hOs, (void *)key.micRxKey, (void *)keyMaterialTkip->micTxKey, KEY_DERIVE_TKIP_MIC_LEN);
    }
    else
    {  /* Copy MIC RX */
        os_memoryCopy(pKeyDerive->hOs, (void *)key.micRxKey, (void *)keyMaterialTkip->micRxKey, KEY_DERIVE_TKIP_MIC_LEN);
        /* Copy MIC RX */
        os_memoryCopy(pKeyDerive->hOs, (void *)key.micTxKey, (void *)keyMaterialTkip->micTxKey, KEY_DERIVE_TKIP_MIC_LEN);
    }
    
    /* Copy MAC address key */
    MAC_COPY (key.macAddress, keyMaterialTkip->macAddress);
    /* Copy RSC */
    os_memoryCopy(pKeyDerive->hOs, (void *)key.keyRsc, (void *)keyMaterialTkip->keyRSC, KEY_RSC_LEN);


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

TI_STATUS keyDeriveTkip_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey)
{
    TI_STATUS status;
    TSecurityKeys  key;

    os_memoryZero(pKeyDerive->hOs, &key, sizeof(TSecurityKeys));
    key.keyType = KEY_TKIP;
    key.keyIndex = (TI_UINT8)pEncodedKey->keyId;
    key.encLen = KEY_DERIVE_TKIP_ENC_LEN;
	MAC_COPY (key.macAddress, pEncodedKey->pData);

    status = pKeyDerive->pMainKeys->removeKey(pKeyDerive->pMainKeys, &key);
    if (status == TI_OK)
    {
        os_memoryZero(pKeyDerive->hOs, &pKeyDerive->key, sizeof(encodedKeyMaterial_t));
    }

    return status;
}

