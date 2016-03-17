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
/** \file externalSec.h
 *  \brief station externalSec API
 *
 *  \see externalSec.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  externalSec	                                	            *
 *   PURPOSE: station external security		                                *
 *                                                                          *
 ****************************************************************************/

#ifndef _EXTERNAL_SEC_H
#define _EXTERNAL_SEC_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

/* Constants */
/** number of states in the state machine */
#define	EXTERNAL_SEC_NUM_STATES		2

/** number of events in the state machine */
#define  EXTERNAL_SEC_NUM_EVENTS	3

/* Enumerations */

/** state machine states */
typedef enum 
{
	EXTERNAL_SEC_STATE_IDLE        = 0,
	EXTERNAL_SEC_STATE_WAIT        = 1
} externalSec_smStates;


/** State machine events */
typedef enum 
{
	EXTERNAL_SEC_EVENT_START       = 0,
	EXTERNAL_SEC_EVENT_COMPLETE    = 1,
	EXTERNAL_SEC_EVENT_STOP	       = 2
} externalSec_smSEvents;

/* Typedefs */

/* Structures */
struct externalSec_t
{
    TI_UINT8            currentState;
    fsm_stateMachine_t  *pExternalSecSm;
    mainSec_t           *pParent;
    TI_HANDLE           hOs; 
    TI_HANDLE           hReport;
    TI_BOOL             bPortStatus;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS externalSec_config(mainSec_t *pMainSec);
struct externalSec_t* externalSec_create(TI_HANDLE hOs);
TI_STATUS externalSec_Destroy(struct externalSec_t *pExternalSec);
TI_STATUS externalSec_event(struct externalSec_t *pExternalSec, TI_UINT8 event, void *pData);
TI_STATUS externalSec_rsnComplete(struct externalSec_t *pExternalSec);
#endif /* _EXTERNAL_SEC_H*/
