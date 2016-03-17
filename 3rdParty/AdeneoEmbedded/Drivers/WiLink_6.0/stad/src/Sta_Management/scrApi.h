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
/** \file ScrApi.h
 *  \brief This file include public definitions for the SCR module, comprising its API.
 *  \
 *  \date 01-Dec-2004
 */

#ifndef __SCRAPI_H__
#define __SCRAPI_H__

#include "DrvMainModules.h"

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
/** \enum EScrModeId 
 * \brief enumerates the different modes available in the system    
 */
typedef enum
{
    SCR_MID_NORMAL = 0,         /**< Normal mode      */
    SCR_MID_SG,                 /**< Soft Gemini mode */
    SCR_MID_NUM_OF_MODES
} EScrModeId;

/** \enum EScrGroupId 
 * \brief enumerates the different groups available in the system
 */
typedef enum
{
    SCR_GID_IDLE = 0,           /**< STA is idle */
    SCR_GID_DRV_SCAN,           /**< STA is disconnected, SME scans */
    SCR_GID_APP_SCAN,           /**< STA is disconnected, application scans */
    SCR_GID_CONNECT,            /**< STA is trying to conenct */
    SCR_GID_CONNECTED,          /**< STA is connected */
    SCR_GID_ROAMING,            /**< STA is performing roaming to another AP */
    SCR_GID_NUM_OF_GROUPS
} EScrGroupId;

/** \enum EScrResourceId 
 * \brief enumerates the different resources controlled by the SCR
 */
typedef enum
{
    SCR_RESOURCE_SERVING_CHANNEL = 0,
    SCR_RESOURCE_PERIODIC_SCAN,
    SCR_RESOURCE_NUM_OF_RESOURCES
} EScrResourceId;

/** \enum EScrClientId 
 * \brief enumerates the different clients available in the system
 */
typedef enum
{
    SCR_CID_APP_SCAN = 0,    /* lowest priority */
    SCR_CID_DRIVER_FG_SCAN,
    SCR_CID_CONT_SCAN,
    SCR_CID_XCC_MEASURE,
    SCR_CID_BASIC_MEASURE,
    SCR_CID_CONNECT,
    SCR_CID_IMMED_SCAN,
    SCR_CID_SWITCH_CHANNEL,     /* highest priority */
    SCR_CID_NUM_OF_CLIENTS,
    SCR_CID_NO_CLIENT
} EScrClientId;

/** \enum EScrClientRequestStatus 
 * \brief enumerates the status reports the client may receive
 */
typedef enum 
{
    SCR_CRS_RUN = 0,            /**< the client can use the channel */
    SCR_CRS_PEND,               /**< the channel is in use, The client may wait for it. */
    SCR_CRS_ABORT,              /**< client should abort it's use of the channel */
    SCR_CRS_FW_RESET            /**< Notification of recovery (client should elect what to do) */
} EScrClientRequestStatus;

/** \enum EScePendReason 
 * \brief enumerates the different reasons which can cause a client request to return with pend status.
 */
typedef enum
{
    SCR_PR_OTHER_CLIENT_ABORTING = 0,       /**< 
                                             * The requesting client is waiting for a client with lower priority  
                                             * to abort operation
                                             */
    SCR_PR_OTHER_CLIENT_RUNNING,            /**<
                                             * The requesting client is waiting for a client that cannot be aborted
                                             * to finish using the channel.
                                             */
    SCR_PR_DIFFERENT_GROUP_RUNNING,         /**< The current SCR group is different than the client's group */
    SCR_PR_NONE                             /**< The client is not pending */
} EScePendReason;

/*
 ***********************************************************************
 *  Typedefs.
 ***********************************************************************
 */

 /** \typedef scr_abortReason_e
  * \brief Defines the function prototype a client should register as callback.
  */
typedef void (*TScrCB)( TI_HANDLE hClient, 
                        EScrClientRequestStatus requestStatus,
                        EScrResourceId eResource,
                        EScePendReason pendReason );

/*
 ***********************************************************************
 *  Structure definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  External data definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  External functions definitions
 ***********************************************************************
 */

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Creates the SCR object
 *
 * Function Scope \e Public.\n
 * \param hOS - handle to the OS object.\n
 * \return a handle to the SCR object.\n
 */
TI_HANDLE scr_create( TI_HANDLE hOS );

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Finalizes the SCR object (freeing memory)
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 */
void scr_release( TI_HANDLE hScr );

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Initializes the SCR object
 *
 * \param  pStadHandles  - The driver modules handles
 * \return void  
 */
void scr_init (TStadHandlesList *pStadHandles);

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Registers the callback function to be used per client.
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param client - the client ID.\n
 * \param callbackFunc - the address of the callback function to use.\n
 * \param callbackObj - the handle of the object to pass to the callback function (the client object).\n
 */
void scr_registerClientCB( TI_HANDLE hScr, 
                           EScrClientId client,
                           TScrCB callbackFunc, 
                           TI_HANDLE callbackObj );

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Notifies the running process upon a firmware reset.
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 */
void scr_notifyFWReset( TI_HANDLE hScr );

/**
 * \\n
 * \date 27-April-2005\n
 * \brief Changes the current SCR group.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param newGroup - the new group to use.\n
 */
void scr_setGroup( TI_HANDLE hScr, EScrGroupId newGroup );

/**
 * \\n
 * \date 23-1l-2005\n
 * \brief Changes the current SCR mode. This function is called from Soft Gemini module only \n
 *        when changing mode - clients that are not valid in the new group/mode are aborted  \n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param newMode - the new mode to use.\n
 */
void scr_setMode( TI_HANDLE hScr, EScrModeId newMode );

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Request the channel use by a client
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param client - the client ID requesting the channel.\n
 * \param eResource - the requested resource.\n
 * \param pPendReason - the reason for a pend reply.\n
 * \return The request status.\n
 * \retval SCR_CRS_REJECT the channel cannot be allocated to this client.
 * \retval SCR_CRS_PEND the channel is currently busy, and this client had been placed on the waiting list.
 * \retval SCR_CRS_RUN the channel is allocated to this client.
 */
EScrClientRequestStatus scr_clientRequest( TI_HANDLE hScr, EScrClientId client,
                                           EScrResourceId eResource,
                                           EScePendReason* pPendReason );

/**
 * \\n
 * \date 01-Dec-2004\n
 * \brief Notifies the SCR that the client doe not require the channel any longer
 *
 * This function can be called both by clients that are in possession of the channel, and by
 * clients that are pending to use the channel.\n
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param client - the client releasing the channel.\n
 * \param eResource - the resource being released.\n
 * \return TI_OK if successful, TI_NOK otherwise.\n
 */
void scr_clientComplete( TI_HANDLE hScr, EScrClientId client, EScrResourceId eResource );

#endif /* __SCRAPI_H__ */
