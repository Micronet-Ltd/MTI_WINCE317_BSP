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
/** \file keyDeriveAes.h
 *  \brief AES key derivation API
 *
 *  \see keyDeriveAes.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:	AES encryption key derivation                               *
 *   PURPOSE:   AES encryption key derivation API                           *
 *                                                                          *
 ****************************************************************************/

#ifndef _KEY_DERIVE_AES_H
#define _KEY_DERIVE_AES_H

#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"

#define DERIVE_AES_KEY_LEN 16   /* 128 bit key  */


typedef struct
{
    TI_UINT8   macAddress[MAC_ADDR_LEN];
    TI_UINT8   keyRSC[KEY_RSC_LEN]; 
    TI_UINT8   encKey[DERIVE_AES_KEY_LEN];
} keyMaterialAes_t;

/* AES key derivation init function. */
TI_STATUS keyDeriveAes_config(struct _keyDerive_t *pKeyDerive);

TI_STATUS keyDeriveAes_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);
TI_STATUS keyDeriveAes_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

#endif /*  _KEY_DERIVE_AES_H  */
