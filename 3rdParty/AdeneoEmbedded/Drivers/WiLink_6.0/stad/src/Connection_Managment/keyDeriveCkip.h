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
/** \file keyDeriveCkip.h
 *  \brief CKIP key derivation API
 *
 *  \see keyDeriveCkip.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:	WEP broadcast key derivation                                *
 *   PURPOSE:   WEP broadcast key derivation API                            *
 *                                                                          *
 ****************************************************************************/

#ifdef XCC_MODULE_INCLUDED
#ifndef _KEY_DERIVE_CKIP_H
#define _KEY_DERIVE_CKIP_H

#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"

#define KEY_DERIVE_CKIP_ENC_LEN 16
#define KEY_DERIVE_CKIP_5_LEN   5
#define KEY_DERIVE_CKIP_13_LEN  13


/* WEP broadcast key derivation init function. */
TI_STATUS keyDeriveCkip_config(struct _keyDerive_t *pKeyDerive);

TI_STATUS keyDeriveCkip_derive(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);
TI_STATUS keyDeriveCkip_remove(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKey);

#endif /*  __INCLUDE_WEP_BROADCAST_KEY_DERIVATION_H */
#endif /* XCC_MODULE_INCLUDED */
