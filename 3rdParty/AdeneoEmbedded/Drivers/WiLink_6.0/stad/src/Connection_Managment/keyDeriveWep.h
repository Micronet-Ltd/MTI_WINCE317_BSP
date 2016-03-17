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
/** \file wepBroadcastKeyDerivation.h
 *  \brief WEP broadcast key derivation API
 *
 *  \see wepBroadcastKeyDerivation.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:	WEP broadcast key derivation                                *
 *   PURPOSE:   WEP broadcast key derivation API                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _KEY_DERIVE_WEP_H
#define _KEY_DERIVE_WEP_H

#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"


#define DERIVE_WEP_KEY_LEN_40	 5  /* 40 bit (5 byte) key */
#define DERIVE_WEP_KEY_LEN_104	 13 /* 104 bit (13 byte) key */
#define DERIVE_WEP_KEY_LEN_232	 29 /* 232 bit (29 byte) key */


/* WEP broadcast key derivation init function. */
TI_STATUS keyDeriveWep_config(struct _keyDerive_t *pKeyDerive);

TI_STATUS keyDeriveWep_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

TI_STATUS keyDeriveWep_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

TI_STATUS keyDeriveNone_config(struct _keyDerive_t *pKeyDerive);

TI_STATUS keyDeriveNone_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

TI_STATUS keyDeriveNone_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

#endif /*  __INCLUDE_WEP_BROADCAST_KEY_DERIVATION_H*/
