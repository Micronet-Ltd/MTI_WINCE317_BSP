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
/** \file GenSM.h
 *  \brief Generic state machine declarations
 *
 *  \see GenSM.c
 */


#ifndef __GENSM_H__
#define __GENSM_H__

#include "tidef.h"


/* action function type definition */
typedef void (*TGenSM_action) (void *pData);


/* State/Event cell */
typedef  struct
{
    TI_UINT32       uNextState; /**< next state in transition */
    TGenSM_action   fAction;    /**< action function */
} TGenSM_actionCell;


/* 
 * matrix type 
 * Although the state-machine matrix is actually a two-dimensional array, it is treated as a single 
 * dimension array, since the size of each dimeansion is only known in run-time
 */
typedef TGenSM_actionCell *TGenSM_matrix;


/* generic state machine object structure */
typedef struct
{
    TI_HANDLE       hOS;               /**< OS handle */ 
    TI_HANDLE       hReport;           /**< report handle */ 
    TGenSM_matrix   tMatrix;           /**< next state/action matrix */
    TI_UINT32       uStateNum;         /**< Number of states in the matrix */
    TI_UINT32       uEventNum;         /**< Number of events in the matrix */
    TI_UINT32       uCurrentState;     /**< Current state */
    TI_UINT32       uEvent;            /**< Last event sent */
    void            *pData;            /**< Last event data */
    TI_BOOL         bEventPending;     /**< Event pending indicator */
    TI_BOOL         bInAction;         /**< Evenet execution indicator */
    TI_UINT32       uModuleLogIndex;   /**< Module index to use for printouts */
    TI_INT8         *pGenSMName;       /**< state machine name */
    TI_INT8         **pStateDesc;      /**< State description strings */
    TI_INT8         **pEventDesc;      /**< Event description strings */
} TGenSM;



TI_HANDLE   genSM_Create (TI_HANDLE hOS);
void        genSM_Unload (TI_HANDLE hGenSM);
void        genSM_Init (TI_HANDLE hGenSM, TI_HANDLE hReport);
void        genSM_SetDefaults (TI_HANDLE hGenSM, TI_UINT32 uStateNum, TI_UINT32 uEventNum,
                        TGenSM_matrix pMatrix, TI_UINT32 uInitialState, TI_INT8 *pGenSMName, 
                        TI_INT8 **pStateDesc, TI_INT8 **pEventDesc, TI_UINT32 uModuleLogIndex);
void        genSM_Event (TI_HANDLE hGenSM, TI_UINT32 uEvent, void *pData);
TI_UINT32   genSM_GetCurrentState (TI_HANDLE hGenSM);

#endif /* __GENSM_H__ */

