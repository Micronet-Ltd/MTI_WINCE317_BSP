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

#ifndef _UNICAST_KEY_SM_H
#define _UNICAST_KEY_SM_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"

/* Constants */

/* Enumerations */

/* Typedefs */

typedef struct _unicastKey_t    unicastKey_t;

/* Main Sec SM functions */
typedef TI_STATUS (*unicastKeySmStart_t)(struct _unicastKey_t *pUnicastKey);
typedef TI_STATUS (*unicastKeySmStop_t)(struct _unicastKey_t *pUnicastKey);
typedef TI_STATUS (*unicastKeySmRecvSuccess_t)(struct _unicastKey_t *pUnicastKey, encodedKeyMaterial_t *pEncodedKeyMaterial);
typedef TI_STATUS (*unicastKeySmRecvFailure_t)(struct _unicastKey_t *pUnicastKey);

/* Structures */

/* State machine associated data structures. */
typedef struct
{
	encodedKeyMaterial_t    *pEncodedKeyMaterial;
} unicastKeyData_t;

struct _unicastKey_t
{
	TI_UINT8                               currentState;
	fsm_stateMachine_t	                *pUcastKeySm;
    unicastKeyData_t					data;

    struct _mainKeys_t                 	*pParent;
	keyDerive_t							*pKeyDerive;
	
	TI_HANDLE			                hReport;
	TI_HANDLE			                hOs;
    
    unicastKeySmStart_t               	start;
    unicastKeySmStop_t                	stop;
    unicastKeySmRecvSuccess_t			recvSuccess;
    unicastKeySmRecvFailure_t			recvFailure;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

unicastKey_t* unicastKey_create(TI_HANDLE hOs);

TI_STATUS unicastKey_unload(unicastKey_t *punicastKey);

TI_STATUS unicastKey_config(struct _unicastKey_t *pUnicastKey, 
						   TRsnPaeConfig *pPaeConfig, 
						   struct _mainKeys_t *pParent,
						   TI_HANDLE hReport,
						   TI_HANDLE hOs);

TI_STATUS unicastKeySmUnexpected(struct _unicastKey_t *pUnicastKey);

TI_STATUS unicastKeySmNop(struct _unicastKey_t *pUnicastKey);

#endif /*  _UNICAST_KEY_SM_H*/
