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

#ifndef _UNICAST_KEY_802_1X_H
#define _UNICAST_KEY_802_1X_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

#include "keyDerive.h"
#include "unicastKeySM.h"

/* Constants */
/** number of states in the state machine */
#define	UCAST_KEY_802_1X_NUM_STATES		3

/** number of events in the state machine */
#define	UCAST_KEY_802_1X_NUM_EVENTS		4

/* Enumerations */

/** state machine states */
typedef enum 
{
	UCAST_KEY_802_1X_STATE_IDLE                  = 0,
	UCAST_KEY_802_1X_STATE_START                 = 1,
	UCAST_KEY_802_1X_STATE_COMPLETE              = 2
} unicastKey802_1x_smStates;

/** State machine events */
typedef enum 
{
	UCAST_KEY_802_1X_EVENT_START                  = 0,
	UCAST_KEY_802_1X_EVENT_STOP                   = 1,
	UCAST_KEY_802_1X_EVENT_SUCCESS                = 2,
	UCAST_KEY_802_1X_EVENT_FAILURE				  = 3
} unicastKey802_1x_smEvents;

/* Enumerations */

/* Typedefs */

/* Structures */


/* External data definitions */

/* External functions definitions */

/* Function prototypes */


TI_STATUS unicastKey802_1x_config(struct _unicastKey_t *pUnicastKey);


#endif /*  _UNICAST_KEY_802_1X_H*/
