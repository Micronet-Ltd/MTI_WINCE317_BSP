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
/** \file fsm.h
 *  \brief finite state machine header file
 *
 *  \see fsm.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	fsm.h													   */
/*    PURPOSE:	Finite State Machine API								   */
/*																	 	   */
/***************************************************************************/

#ifndef __FSM_H__
#define __FSM_H__

#include "tidef.h"
#include "commonTypes.h"

/* Constants */
#define	MAX_DESC_STRING_LEN		64		


/* Enumerations */

/* Typedefs */

/** state transition function */
typedef	TI_STATUS (*fsm_eventActivation_t)(TI_UINT8 *currState, TI_UINT8 event, void* data);

/** action function type definition */
typedef TI_STATUS (*fsm_Action_t)(void* pData);

/* Structures */

/* State\Event cell */
typedef  struct
{
	TI_UINT8			nextState;		/**< next state in transition */
	fsm_Action_t	actionFunc;		/**< action function */
} fsm_actionCell_t;

/** matrix type */
typedef	fsm_actionCell_t*		fsm_Matrix_t;

/** general FSM structure */
typedef struct
{
	fsm_Matrix_t			stateEventMatrix;		/**< State\Event matrix */
	TI_UINT8					MaxNoOfStates;			/**< Max Number of states in the matrix */
	TI_UINT8					MaxNoOfEvents;			/**< Max Number of events in the matrix */
	TI_UINT8					ActiveNoOfStates;		/**< Active Number of states in the matrix */
	TI_UINT8					ActiveNoOfEvents;		/**< Active Number of events in the matrix */
	fsm_eventActivation_t	transitionFunc;			/**< State transition function */
} fsm_stateMachine_t;

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS fsm_Create(TI_HANDLE				hOs,
				fsm_stateMachine_t		**pFsm,
				TI_UINT8					MaxNoOfStates,
				TI_UINT8					MaxNoOfEvents);

TI_STATUS fsm_Unload(TI_HANDLE				hOs,
				fsm_stateMachine_t		*pFsm);

TI_STATUS fsm_Config(fsm_stateMachine_t	*pFsm,
				  fsm_Matrix_t			pMatrix,
				  TI_UINT8					ActiveNoOfStates,
				  TI_UINT8					ActiveNoOfEvents,
				  fsm_eventActivation_t	transFunc,
				  TI_HANDLE				hOs);

TI_STATUS fsm_Event(fsm_stateMachine_t		*pFsm,
				 TI_UINT8					*currentState,
				 TI_UINT8					event,
				 void					*pData);

TI_STATUS fsm_GetNextState(fsm_stateMachine_t		*pFsm,
						TI_UINT8					currentState,
						TI_UINT8					event,
						TI_UINT8					*nextState);


TI_STATUS action_nop(void *pData);


#endif /* __FSM_H__ */
