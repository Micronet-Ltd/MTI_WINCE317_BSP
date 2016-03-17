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
/** \file roamingMngr_manualSM.h
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

#ifndef _ROAMING_MNGR_MANUAL_SM_H_
#define _ROAMING_MNGR_MANUAL_SM_H_

/*-----------*/
/* Constants */
/*-----------*/

/** state machine states */
typedef enum 
{
/*	0	*/  ROAMING_MANUAL_STATE_IDLE = 0,
/*	1	*/  ROAMING_MANUAL_STATE_CONNECTED,
/*	2	*/  ROAMING_MANUAL_STATE_SCAN,
/*	3	*/  ROAMING_MANUAL_STATE_HANDOVER,
/*	4	*/  ROAMING_MANUAL_STATE_LAST   
} ERoamManual_smStates;

/** State machine events */
typedef enum 
{
/*	0	*/	ROAMING_MANUAL_EVENT_START = 0,
/*	1	*/	ROAMING_MANUAL_EVENT_SCAN,
/*	2	*/	ROAMING_MANUAL_EVENT_CONNECT,
/*	3	*/	ROAMING_MANUAL_EVENT_STOP, 
/*	4	*/	ROAMING_MANUAL_EVENT_REJECT, 
/*	5	*/	ROAMING_MANUAL_EVENT_SUCCESS,
/*	6	*/	ROAMING_MANUAL_EVENT_FAIL,
/*	7	*/	ROAMING_MANUAL_EVENT_COMPLETE,
/*	8	*/	ROAMING_MANUAL_EVENT_LAST,
} ERoamManual_smEvents;


#define ROAMING_MANUAL_NUM_STATES    ROAMING_MANUAL_STATE_LAST   
#define ROAMING_MANUAL_NUM_EVENTS    ROAMING_MANUAL_EVENT_LAST


extern TGenSM_actionCell roamingMngrManual_matrix[ROAMING_MANUAL_NUM_STATES][ROAMING_MANUAL_NUM_EVENTS];
extern TI_INT8*  ManualRoamStateDescription[];
extern TI_INT8*  ManualRoamEventDescription[];

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


#endif /* _ROAMING_MNGR_MANUAL_SM_H_ */

