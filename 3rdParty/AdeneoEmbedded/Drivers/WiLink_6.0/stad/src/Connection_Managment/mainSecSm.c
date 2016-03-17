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
/** \file mainSecSm.c
 *  \brief 802.1X finite state machine header file
 *
 *  \see mainSecSm.h
 */


/***************************************************************************/
/*                                                                         */
/*      MODULE: mainSecSm.c                                                */
/*    PURPOSE:  Main Security State Machine API                            */
/*                                                                         */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_39
#include "osApi.h"
#include "paramOut.h"
#include "report.h"
#include "DataCtrl_Api.h"
#include "smeApi.h"
#include "rsn.h"
#include "rsnApi.h"
#include "mainSecSm.h"
#include "mainSecNull.h"
#include "mainSecKeysOnly.h"
#include "mainKeysSm.h"
#include "externalSec.h"

/* Constants */

/** number of events in the state machine */
#define MAIN_SEC_MAX_NUM_EVENTS     7

/** number of states in the state machine */
#define MAIN_SEC_MAX_NUM_STATES     6

/* Enumerations */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Global variables */

/* Local function prototypes */

TI_STATUS mainSec_setKey(struct _mainSec_t *pMainSec, TSecurityKeys *pKey);
TI_STATUS mainSec_removeKey(struct _mainSec_t *pMainSec, TSecurityKeys *pKey);
TI_STATUS mainSec_setDefaultKeyId(struct _mainSec_t *pMainSec, TI_UINT8 keyId);

/* functions */

/**
*
* mainSec_create
*
* \b Description: 
*
* Allocate memory for the main security context, and create all the rest of the needed contexts.
*
* \b ARGS:
*
*  I - hOs - OS handle for OS operations.
*
* \b RETURNS:
*
*  pointer to main security context. If failed, returns NULL.
*
* \sa 
*/
mainSec_t* mainSec_create(TI_HANDLE hOs)
{
    mainSec_t   *pHandle;
    TI_STATUS       status;

    /* allocate association context memory */
    pHandle = (mainSec_t*)os_memoryAlloc(hOs, sizeof(mainSec_t));
    if (pHandle == NULL)
    {
        return NULL;
    }

    os_memoryZero(hOs, pHandle, sizeof(mainSec_t));

    /* allocate memory for association state machine */
    status = fsm_Create(hOs, &pHandle->pMainSecSm, MAIN_SEC_MAX_NUM_STATES, MAIN_SEC_MAX_NUM_EVENTS);
    if (status != TI_OK)
    {
        os_memoryFree(hOs, pHandle, sizeof(mainSec_t));
        return NULL;
    }

    pHandle->pMainKeys = mainKeys_create(hOs);
    if (pHandle->pMainKeys == NULL)
    {
        fsm_Unload(hOs, pHandle->pMainSecSm);
        os_memoryFree(hOs, pHandle, sizeof(mainSec_t));
        return NULL;
    }

    pHandle->pKeyParser = pHandle->pMainKeys->pKeyParser;
    pHandle->hOs = hOs;
    
    /* created only for external security mode */
    pHandle->pExternalSec = externalSec_create(hOs);

    if (pHandle->pExternalSec == NULL)
    {
        fsm_Unload(hOs, pHandle->pMainSecSm);
        mainKeys_unload(pHandle->pMainKeys);
        os_memoryFree(hOs, pHandle, sizeof(mainSec_t));
        return NULL;
    }

    return pHandle;
}

/**
*
* mainSec_config
*
* \b Description: 
*
* Init main security state machine state machine
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSec_config (mainSec_t *pMainSec, 
                          mainSecInitData_t *pInitData, 
                          void *pParent,
                          TI_HANDLE hReport,
                          TI_HANDLE hOs,
                          TI_HANDLE hCtrlData,
                          TI_HANDLE hEvHandler,
                          TI_HANDLE hConn,
                          TI_HANDLE hTimer)
{
    TI_STATUS               status;

    pMainSec->setKey = mainSec_setKey;
    pMainSec->removeKey = mainSec_removeKey;
    pMainSec->setDefaultKeyId = mainSec_setDefaultKeyId;

    pMainSec->pParent = pParent;
    pMainSec->hReport = hReport;
    pMainSec->hOs = hOs;

    TRACE4(pMainSec->hReport, REPORT_SEVERITY_SM, "MainSec SM: config, authProtocol = %d, keyExchangeProtocol=%d, unicastSuite=%d, broadcastSuite=%d\n", pInitData->pPaeConfig->authProtocol, pInitData->pPaeConfig->keyExchangeProtocol, pInitData->pPaeConfig->unicastSuite, pInitData->pPaeConfig->broadcastSuite); 

    if (TI_TRUE == pMainSec->pParent->bRsnExternalMode)
    {
            status = externalSec_config(pMainSec);
    }
    else
    {
         switch (pInitData->pPaeConfig->keyExchangeProtocol)
         {
            case RSN_KEY_MNG_NONE:
                status = mainSecSmNull_config(pMainSec, pInitData->pPaeConfig);
                break;
            case RSN_KEY_MNG_802_1X:
                status = mainSecKeysOnly_config(pMainSec, pInitData->pPaeConfig);
                break;
            default:
                status = mainSecSmNull_config(pMainSec, pInitData->pPaeConfig);
                break;
         }
    }
  
    status  = mainKeys_config (pMainSec->pMainKeys, 
                               pInitData->pPaeConfig, 
                               pMainSec, 
                               pMainSec->hReport, 
                               pMainSec->hOs, 
                               hCtrlData, 
                               hEvHandler, 
                               hConn, 
                               pMainSec->pParent,
                               hTimer);
    if (status != TI_OK)
    {
        TRACE0(pMainSec->hReport, REPORT_SEVERITY_ERROR, "MAIN_SEC_SM: error in configuring mainKeys SM\n");
        return status;
    }

    TRACE0(pMainSec->hReport, REPORT_SEVERITY_SM, "MAIN_SEC_SM: successful configuration SM\n");

    return status;
}

/**
*
* mainSec_config
*
* \b Description: 
*
* Init main security state machine state machine
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSec_unload(mainSec_t *pMainSec)
{
    TI_STATUS   status;

    if (pMainSec == NULL)
    {
        return TI_NOK;
    }

    status = mainKeys_unload(pMainSec->pMainKeys);
    if (status != TI_OK)
    {
        /* report failure but don't stop... */
        TRACE0(pMainSec->hReport, REPORT_SEVERITY_ERROR, "MAIN_SEC_SM: Error releasing Main Keys SM memory \n");
    }

    status = fsm_Unload(pMainSec->hOs, pMainSec->pMainSecSm);
    if (status != TI_OK)
    {
        /* report failure but don't stop... */
        TRACE0(pMainSec->hReport, REPORT_SEVERITY_ERROR, "MAIN_SEC_SM: Error releasing FSM memory \n");
    }

    status = externalSec_Destroy (pMainSec->pExternalSec);
    if (status != TI_OK)
    {
        /* report failure but don't stop... */
        TRACE0(pMainSec->hReport, REPORT_SEVERITY_ERROR, "MAIN_SEC_SM: Error releasing External Security SM memory \n");
    }

    os_memoryFree(pMainSec->hOs, pMainSec, sizeof(mainSec_t));

    return TI_OK;
}

/**
*
* mainSec_setKey
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSec_setKey(struct _mainSec_t *pMainSec, TSecurityKeys *pKey)
{
    TI_STATUS               status = TI_OK;
    
    if ((pMainSec == NULL) || (pKey == NULL))
    {
        return TI_NOK;
    }

    if (pKey->keyType != KEY_NULL)
    {
        TRACE6(pMainSec->hReport, REPORT_SEVERITY_INFORMATION, "MAIN_SEC_SM: setting key #%d, value = 0x%X 0x%X 0x%X 0x%X 0x%X\n", pKey->keyIndex, (TI_UINT8)pKey->encKey[0], (TI_UINT8)pKey->encKey[1], (TI_UINT8)pKey->encKey[2], (TI_UINT8)pKey->encKey[3], (TI_UINT8)pKey->encKey[4]);

        status = pMainSec->pParent->setKey(pMainSec->pParent, pKey);
    }
    
    return status;
}

/**
*
* mainSec_removeKey
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSec_removeKey(struct _mainSec_t *pMainSec, TSecurityKeys *pKey)
{
    TI_STATUS               status = TI_OK;
    
    if ((pMainSec == NULL) || (pKey == NULL))
    {
        return TI_NOK;
    }

    if (pKey->keyType != KEY_NULL)
    {
        TRACE1(pMainSec->hReport, REPORT_SEVERITY_INFORMATION, "MAIN_SEC_SM: removing key #%d, \n", pKey->keyIndex);

        status = pMainSec->pParent->removeKey(pMainSec->pParent, pKey);
    }
    
    return status;
}

/**
*
* mainSec_setDefaultKeyId
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSec_setDefaultKeyId(struct _mainSec_t *pMainSec, TI_UINT8 keyId)
{
    TI_STATUS               status = TI_OK;

    if (pMainSec == NULL)
    {
        return TI_NOK;
    }

    status = pMainSec->pParent->setDefaultKeyId(pMainSec->pParent, keyId);
    
    return status;
}


