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
/** \file KeyParserWep.c
 * \brief Wep key parser implementation.
 *
 * \see keyParser.h
*/

/****************************************************************************
 *                                                                          *
 *   MODULE:	Wep Key Parser                                             *
 *   PURPOSE:   EAP parser implementation                                   *
 *                                                                          *
 ****************************************************************************/

#define __FILE_ID__  FILE_ID_35
#include "tidef.h"
#include "osApi.h"
#include "report.h"

#include "keyTypes.h"

#include "keyParser.h"
#include "keyParserWep.h"
#include "mainKeysSm.h"

#include "unicastKeySM.h"
#include "broadcastKeySM.h"


TI_STATUS keyParserWep_recv(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen);
TI_STATUS keyParserWep_remove(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen);

/**
*
* Function  - Init KEY Parser module.
*
* \b Description: 
*
* Called by RSN Manager. 
* Registers the function 'rsn_keyParserRecv()' at the distributor to receive KEY frames upon receiving a KEY_RECV event.
*
* \b ARGS:
*
*  
* \b RETURNS:
*
*  TI_STATUS - 0 on success, any other value on failure. 
*
*/

TI_STATUS keyParserWep_config(struct _keyParser_t *pKeyParser)
{
	pKeyParser->recv = keyParserWep_recv;
	pKeyParser->replayReset = keyParser_nop;
	pKeyParser->remove = keyParserWep_remove;
	return TI_OK;
}


/**
*
* keyParserWep_recv
*
* \b Description: 
*
* WEP key Parser receive function:
*							- Called by the utility or NDIS (Windows)  upon receiving a WEP Key.
*							- Filters the following keys:								
*								- Per Client Keys
*								- Keys with size different than 40-bit, 104-bit and 232-bit
*								- Keys with invalid key index
*
* \b ARGS:
*
*  I   - pKeyParser - Pointer to the keyParser context  \n
*  I   - pKeyData - A pointer to the Key Data. \n
*  I   - keyDataLen - The Key Data length. \n
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
*/
TI_STATUS keyParserWep_recv(struct _keyParser_t *pKeyParser,
						  TI_UINT8 *pKeyData, TI_UINT32 keyDataLen)
{
	TI_STATUS				        status;
	OS_802_11_KEY 	                *pKeyDesc;
    TSecurityKeys                  securityKey;
	
	if (pKeyData == NULL)
	{
TRACE0(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "WEP_KEY_PARSER: ERROR: NULL KEY Data\n");
		return TI_NOK;
	}
	
	pKeyDesc = (OS_802_11_KEY*)pKeyData; 

	if ((pKeyDesc->KeyLength < MIN_KEY_LEN ) || (pKeyDesc->KeyLength >= MAX_KEY_LEN ))
    {
        TRACE1(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "WEP_KEY_PARSER: ERROR: Key Length out of bounds=%d\n", pKeyDesc->KeyLength);
        return TI_NOK;
    }

    if (pKeyDesc->KeyIndex & WEP_KEY_REMAIN_BITS_MASK)
    {  /* the reamining bits in the key index are not 0 (when they should be) */
TRACE0(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "WEP_KEY_PARSER: ERROR: Key index bits 8-29 should be 0 !!!\n");
		return TI_NOK;
    }

    securityKey.keyType = KEY_WEP;
    securityKey.encLen = (TI_UINT16)pKeyDesc->KeyLength;
    securityKey.keyIndex = pKeyDesc->KeyIndex;
    os_memoryCopy(pKeyParser->hOs, (void *)securityKey.encKey, pKeyDesc->KeyMaterial, pKeyDesc->KeyLength);
	
    TRACE2(pKeyParser->hReport, REPORT_SEVERITY_INFORMATION, "WEP_KEY_PARSER: Key received keyId=%x, keyLen=%d\n",						    pKeyDesc->KeyIndex, pKeyDesc->KeyLength);


    /* We accept only 40, 104 or 232 -bit WEP keys*/
    if (!((securityKey.encLen == WEP_KEY_LEN_40) || (securityKey.encLen == WEP_KEY_LEN_104) 
          || (securityKey.encLen == WEP_KEY_LEN_232)))
    {	/*Invalid key length*/
        TRACE1(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "WEP_KEY_PARSER: ERROR: Invalid Key length: %d !!!\n", securityKey.encLen);
        return TI_NOK;
    }
    /* configure key for Tx and Rx */
    if (pKeyDesc->KeyIndex & WEP_KEY_TRANSMIT_MASK)
    {	/* configure default key for Tx - unicast */
        status = pKeyParser->pParent->setDefaultKeyId(pKeyParser->pParent, (TI_UINT8)securityKey.keyIndex);
        if (status!=TI_OK)
        {
            return status;
        }        
    }
    /* configure key for Tx - unicast, and Rx - broadcast*/
    status = pKeyParser->pParent->setKey(pKeyParser->pParent, &securityKey);

	return status;
}



TI_STATUS keyParserWep_remove(struct _keyParser_t *pKeyParser, TI_UINT8 *pKeyData, TI_UINT32 keyDataLen)
{
	TI_STATUS				status;
	OS_802_11_KEY			*pKeyDesc;
	encodedKeyMaterial_t    encodedKeyMaterial;
    TI_UINT8                keyBuffer[MAC_ADDR_LEN+KEY_RSC_LEN+MAX_WEP_KEY_DATA_LENGTH];

	if (pKeyData == NULL)
	{
TRACE0(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "EXT_KEY_PARSER: ERROR: NULL KEY Data\n");
		return TI_NOK;
	}
	
	pKeyDesc = (OS_802_11_KEY*)pKeyData;

    if (pKeyDesc->KeyIndex & WEP_KEY_TRANSMIT_MASK)
	{	/* Bit 31 should always be zero */
TRACE0(pKeyParser->hReport, REPORT_SEVERITY_ERROR, "WEP_KEY_PARSER: ERROR: Remove TX key index\n");
		return TI_NOK;
	}

	encodedKeyMaterial.keyId = pKeyDesc->KeyIndex;
	encodedKeyMaterial.keyLen = 0;
    encodedKeyMaterial.pData = (char *) keyBuffer;
    MAC_COPY (keyBuffer, pKeyDesc->BSSID);

	/* which should we delete ????*/
    status =  pKeyParser->pBcastKey->pKeyDerive->remove(pKeyParser->pUcastKey->pKeyDerive, &encodedKeyMaterial);
	return status;

}
