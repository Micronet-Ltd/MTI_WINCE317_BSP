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
/** \file  scr.h
 *  \brief This file includes internal (private) definitions to the SCR module
 *
 *  \see   scrApi.h, scr.c
 */


#ifndef __SCR_H__
#define __SCR_H__

#include "scrApi.h"

/*
 ***********************************************************************
 *  Constant definitions.
 ***********************************************************************
 */

 /*
 ***********************************************************************
 *  Enums.
 ***********************************************************************
 */

/** \enum EScrClientState 
 * \brief enumerates the different states a client may be in .\n
 */
typedef enum
{
    SCR_CS_IDLE = 0,    /**< client is idle */
    SCR_CS_PENDING,     /**< client is pending to use the channel */
    SCR_CS_RUNNING,     /**< client is using the channel */
    SCR_CS_ABORTING     /**< 
                         * client was using the channel, but was aborted, 
                         * and complete notification is expected.
                         */
} EScrClientState;


/*
 ***********************************************************************
 *  Typedefs.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  Structure definitions.
 ***********************************************************************
 */

/** \struct TScrClient
 * \brief This structure contains information for a specific client
 */
typedef struct
{
    EScrClientState     state[ SCR_RESOURCE_NUM_OF_RESOURCES ];     /**< the client current state, per resource */
    TScrCB              clientRequestCB;                            /**< the client's callback function */
    TI_HANDLE           ClientRequestCBObj;                         /**< the client's object */
    EScePendReason      currentPendingReason[ SCR_RESOURCE_NUM_OF_RESOURCES ];
                                                                    /**< 
                                                                     * the reason why this client is pending
                                                                     * (if at all)
                                                                     */
} TScrClient;

/** \struct TScr
 * \brief This structure contains the SCR object data
 */
typedef struct
{
    TI_HANDLE               hOS;                                    /**< a handle to the OS object */
    TI_HANDLE               hReport;                                /**< a handle to the report object */
    TI_BOOL                 statusNotficationPending;               /**< 
                                                                     * whether the SCR is in the process of  
                                                                     * notifying a status change to a client
                                                                     * (used to solve re-entrance problem)
                                                                     */
    EScrClientId            runningClient[ SCR_RESOURCE_NUM_OF_RESOURCES ];
                                                                    /**< 
                                                                     * The index of the current running client 
                                                                     * (SCR_CID_NO_CLIENT if none), per resource
                                                                     */
    EScrGroupId             currentGroup;                           /**< the current group */
    EScrModeId              currentMode;                            /**< the current mode */
    TScrClient              clientArray[ SCR_CID_NUM_OF_CLIENTS ];  /**< array holding all clients' info */
} TScr;


/*
 ***********************************************************************
 *  External functions definitions
 ***********************************************************************
 */
/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Searches the client database for a client with matching state, from startFrom to endAt\n
 *
 * Function Scope \e Private.\n
 * \param hScr - handle to the SCR object.\n
 * \param requiredState - the state to match.\n
 * \param eResource - the resource to macth.\n
 * \param startFrom - the highest priority to begin searching from.\n
 * \param endAt - the lowest priority to include in the search.\n
 * \return the client ID if found, SCR_CID_NO_CLIENT if not found.\n
 */
EScrClientId scrFindHighest( TI_HANDLE hScr,
                             EScrClientState requiredState,
                             EScrResourceId eResource,
                             TI_UINT32 startFrom,
                             TI_UINT32 endAt );

#endif /* __SCR_H__ */
