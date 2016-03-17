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
/** \file broadcastKeyNone.h
 *  \brief station broadcast key None API
 *
 *  \see broadcastKeySM.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  station broadcast key SM	                                	*
 *   PURPOSE: station broadcast key SM API		                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _BROADCAST_KEY_NONE_H
#define _BROADCAST_KEY_NONE_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

#include "broadcastKeySM.h"

/* Constants */

/* Enumerations */

/* Typedefs */


/* Structures */


/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS broadcastKeyNone_config(broadcastKey_t *pBroadcastKey);


#endif /*  _BROADCAST_KEY_NONE_H*/
