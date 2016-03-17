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
/** \file roamingMngr_autoSM.h
 *  \brief Internal Roaming Manager API
 *
 *  \see roamingMngr.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Roaming Manager	    		                                *
 *   PURPOSE: Roaming Manager Module API                              		*
 *                                                                          *
 ****************************************************************************/

#ifndef _ROAMING_MNGR_AUTO_SM_H_
#define _ROAMING_MNGR_AUTO_SM_H_

#include "osApi.h"
#include "paramOut.h"
#include "scanMngrApi.h"
#include "bssTypes.h"
#include "DrvMainModules.h"


/*-----------*/
/* Constants */
/*-----------*/

/** state machine states */
typedef enum 
{
/*	0	*/	ROAMING_STATE_IDLE,
/*	1	*/	ROAMING_STATE_WAIT_4_TRIGGER,
/*	2	*/  ROAMING_STATE_WAIT_4_CMD,
/*	3	*/  ROAMING_STATE_SCANNING,
/*	4	*/  ROAMING_STATE_SELECTING,
/*	5	*/  ROAMING_STATE_CONNECTING,
/*	6	*/  ROAMING_STATE_LAST

} roamingMngr_smStates;

/** State machine events */
typedef enum 
{
/*	0	*/	ROAMING_EVENT_START = 0, 			/* CONNECTED */
/*	1	*/	ROAMING_EVENT_STOP, 			/* NOT CONNECTED */
/*	2	*/	ROAMING_EVENT_ROAM_TRIGGER,
/*	3	*/	ROAMING_EVENT_SCAN, 
/*	4	*/	ROAMING_EVENT_SELECT, 
/*	5	*/	ROAMING_EVENT_REQ_HANDOVER, 
/*	6	*/	ROAMING_EVENT_ROAM_SUCCESS, 
/*	7	*/	ROAMING_EVENT_FAILURE, 
/*	8	*/	ROAMING_EVENT_LAST

} roamingMngr_smEvents;

#define ROAMING_MNGR_NUM_STATES     	ROAMING_STATE_LAST   
#define ROAMING_MNGR_NUM_EVENTS     	ROAMING_EVENT_LAST

#define INVALID_CANDIDATE_INDEX     	0xFF
#define CURRENT_AP_INDEX            	0xFE


extern TGenSM_actionCell roamingMngrAuto_matrix[ROAMING_MNGR_NUM_STATES][ROAMING_MNGR_NUM_EVENTS];
extern TI_INT8*  AutoRoamStateDescription[];
extern TI_INT8*  AutoRoamEventDescription[];

/*--------------*/
/* Enumerations */
/*--------------*/

/*----------*/
/* Typedefs */
/*----------*/

/*------------*/
/* Structures */
/*------------*/

/*---------------------------*/
/* External data definitions */
/*---------------------------*/

/*--------------------------------*/
/* External functions definitions */
/*--------------------------------*/

/*----------------------------*/
/* Global Function prototypes */
/*----------------------------*/


#endif /* _ROAMING_MNGR_AUTO_SM_H_ */

