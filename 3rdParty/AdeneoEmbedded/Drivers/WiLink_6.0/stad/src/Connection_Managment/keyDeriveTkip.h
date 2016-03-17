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
/** \file keyDeriveTkip.h
 *  \brief TKIP key derivation API
 *
 *  \see keyDeriveTkip.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:	WEP broadcast key derivation                                *
 *   PURPOSE:   WEP broadcast key derivation API                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _KEY_DERIVE_TKIP_H
#define _KEY_DERIVE_TKIP_H

#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"

#define KEY_DERIVE_TKIP_ENC_LEN 16
#define KEY_DERIVE_TKIP_MIC_LEN 8

typedef struct
{
    TI_UINT8   macAddress[MAC_ADDR_LEN];    
    TI_UINT8   keyRSC[KEY_RSC_LEN]; 
    TI_UINT8   encKey[KEY_DERIVE_TKIP_ENC_LEN];
    TI_UINT8   micRxKey[KEY_DERIVE_TKIP_MIC_LEN];
    TI_UINT8   micTxKey[KEY_DERIVE_TKIP_MIC_LEN];
} keyMaterialTkip_t;

/* WEP broadcast key derivation init function. */
TI_STATUS keyDeriveTkip_config(struct _keyDerive_t *pKeyDerive);

TI_STATUS keyDeriveTkip_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);
TI_STATUS keyDeriveTkip_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

#endif /*  __INCLUDE_WEP_BROADCAST_KEY_DERIVATION_H*/
