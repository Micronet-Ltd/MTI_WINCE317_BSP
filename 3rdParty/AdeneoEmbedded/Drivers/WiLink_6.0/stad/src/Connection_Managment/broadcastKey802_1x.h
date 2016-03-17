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
/** \file broadcastKeySM.h
 *  \brief station broadcast key SM API
 *
 *  \see broadcastKeySM.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  station broadcast key SM	                                	*
 *   PURPOSE: station broadcast key SM API		                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _BROADCAST_KEY_802_1X_H
#define _BROADCAST_KEY_802_1X_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

#include "broadcastKeySM.h"

/* Constants */
/** number of states in the state machine */
#define	BCAST_KEY_802_1X_NUM_STATES		3

/** number of events in the state machine */
#define	BCAST_KEY_802_1X_NUM_EVENTS		4

/* Enumerations */

/** state machine states */
typedef enum 
{
	BCAST_KEY_802_1X_STATE_IDLE                  = 0,
	BCAST_KEY_802_1X_STATE_START                 = 1,
	BCAST_KEY_802_1X_STATE_COMPLETE              = 2
} broadcastKey802_1x_smStates;

/** State machine events */
typedef enum 
{
	BCAST_KEY_802_1X_EVENT_START                  = 0,
	BCAST_KEY_802_1X_EVENT_STOP                   = 1,
	BCAST_KEY_802_1X_EVENT_SUCCESS                = 2,
	BCAST_KEY_802_1X_EVENT_FAILURE				  = 3
} broadcastKey802_1x_smEvents;

/* Enumerations */

/* Typedefs */


/* Structures */


/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS broadcastKey802_1x_config(broadcastKey_t *pBroadcastKey);


#endif /*  _BROADCAST_KEY_802_1X_H*/
