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
/** \file unicastKeySM.h
 *  \brief station unicast key SM API
 *
 *  \see unicastKeySM.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  station unicast key SM	                                	*
 *   PURPOSE: station unicast key SM API		                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _KEY_DERIVE_H
#define _KEY_DERIVE_H

#include "paramOut.h"
#include "rsnApi.h"
#include "keyTypes.h"

/* Constants */

/* Enumerations */

/* Typedefs */

typedef struct _keyDerive_t    keyDerive_t;

/* Main Sec SM functions */
typedef TI_STATUS (*keyDeriveDerive_t)(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKeyMaterial);
typedef TI_STATUS (*keyDeriveRemove_t)(struct _keyDerive_t *pKeyDerive, encodedKeyMaterial_t *pEncodedKeyMaterial);

/* Structures */

/* State machine associated data structures. */
struct _keyDerive_t
{
	encodedKeyMaterial_t	key;

	struct _mainKeys_t     	*pMainKeys;
	
	TI_HANDLE				hReport;
	TI_HANDLE			    hOs;
    
	keyDeriveDerive_t		derive;
	keyDeriveRemove_t		remove;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

keyDerive_t* keyDerive_create(TI_HANDLE hOs);

TI_STATUS keyDerive_unload(keyDerive_t *pKeyDerive);

TI_STATUS keyDerive_config(keyDerive_t *pKeyDerive, 
						ECipherSuite cipher, 
						struct _mainKeys_t *pParent,
						TI_HANDLE hReport,
						TI_HANDLE hOs);


#endif /*  _KEY_DERIVE_H*/
