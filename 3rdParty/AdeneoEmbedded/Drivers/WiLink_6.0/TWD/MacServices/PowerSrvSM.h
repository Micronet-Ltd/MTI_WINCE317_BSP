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
/** \file PowerSrvSM.h
 *  \brief This is the PowerSrv module API.
 *  \
 *  \date 6-Oct-2005
 */

/****************************************************************************
 *                                                                                                        *
 *   MODULE:  PowerSrv                                                                              *
 *   PURPOSE: Power Server State machine API                                                    *
 *                                                                                                              *
 ****************************************************************************/

#ifndef _POWER_SRV_SM_H_
#define _POWER_SRV_SM_H_

#include "PowerSrv_API.h"
#include "PowerSrv.h"



/*****************************************************************************
 **         Constants                                                       **
 *****************************************************************************/

#define POWER_SAVE_GUARD_TIME_MS            5000       /* The gaurd time used to protect from FW stuck */

/*****************************************************************************
 **         Enumerations                                                    **
 *****************************************************************************/

/** \enum PowerSrvSMEvents_e */
typedef enum 
{
        POWER_SRV_EVENT_REQUEST_ACTIVE , 
    POWER_SRV_EVENT_REQUEST_PS ,
    POWER_SRV_EVENT_SUCCESS,
    POWER_SRV_EVENT_FAIL ,
    POWER_SRV_SM_EVENT_NUM
}PowerSrvSMEvents_e;

/** \enum PowerSrvSMStates_e */
typedef enum 
{
    POWER_SRV_STATE_ACTIVE = 0,
    POWER_SRV_STATE_PEND_PS ,
    POWER_SRV_STATE_PS ,
    POWER_SRV_STATE_PEND_ACTIVE ,
    POWER_SRV_STATE_ERROR_ACTIVE,
    POWER_SRV_SM_STATE_NUM
}PowerSrvSMStates_e;



/*****************************************************************************
 **         Structures                                                      **
 *****************************************************************************/

/** \struct PowerSrvSM_t */
typedef struct
{
    TI_HANDLE               hCmdBld;                    /**< 
                                                         * Handle to the power controller object via the command builder.
                                                         * Need for configure the desired power mode policy in the system.
                                                         */

    TI_HANDLE               hOS;                        /**< Handle to the OS object. */

    TI_HANDLE               hReport;                    /**< Handle to the Report module. */

    TI_HANDLE               hFSM;                       /**< Handle to the State machine module. */

    TI_HANDLE               hTimer;                     /**< Handle to the Timer module. */

    TI_HANDLE               hPwrSrvSmTimer;             /**< Guard timer for PS commands sent to the FW */

    PowerSrvSMStates_e      currentState;               /**< the current state of the state machine. */

    powerSrvRequest_t*      pSmRequest;                 /**< pointer to the relevant request in the power server. */

    TI_UINT8                hangOverPeriod;             /**< parameter for the FW */

    TI_UINT8                numNullPktRetries;          /**< parameter for the FW */
    
    EHwRateBitFiled         NullPktRateModulation;      /**< parameter for the FW */

    TFailureEventCb         failureEventCB;             /**< Failure event callback */

    TI_HANDLE               hFailureEventObj;           /**< Failure event object (supplied to the above callback) */
} PowerSrvSM_t;







/*****************************************************************************
 **         External data definitions                                       **
 *****************************************************************************/


/*****************************************************************************
 **         External functions definitions                                  **
 *****************************************************************************/


/*****************************************************************************
 **         Public Function prototypes                                      **
 *****************************************************************************/

/**
 * \
 * \date 6-Oct-2005\n
 * \brief Creates the object of the PowerSrv.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the OS.\n
 * Return Value: TI_HANDLE - handle to the PowerSrv object.\n
 */
TI_HANDLE powerSrvSM_create(TI_HANDLE hOsHandle);

/**
 * \
 * \date 6-Oct-2005\n
 * \brief Destroy the object of the PowerSrvSM.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrv object.\n
 * Return Value: TI_STATUS - TI_OK on success else TI_NOK.\n
 */
TI_STATUS powerSrvSM_destroy(TI_HANDLE thePowerSrvSMHandle);

/**
 * \
 * \date 6-Oct-2005\n
 * \brief Initialize the PowerSrvSM module.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * 2) TI_HANDLE - handle to the Report object.
 * 3) TI_HANDLE - handle to the Command Builder object.
 * 4) TI_HANDLE - handle to the Timer module object.
 * Return Value: TI_STATUS - TI_OK on success else TI_NOK.\n
 */
TI_STATUS powerSrvSM_init (TI_HANDLE hPowerSrvSM,
                           TI_HANDLE hReport,
                           TI_HANDLE hCmdBld,
                           TI_HANDLE hTimer);

TI_STATUS powerSrvSM_config(TI_HANDLE hPowerSrvSM,
                            TPowerSrvInitParams *pPowerSrvInitParams);
/**
 * \
 * \date 6-Oct-2005\n
 * \brief return the component version.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * Return Value: TI_UINT32 - component version code.\n
 */

TI_STATUS powerSrvSM_SMApi(TI_HANDLE hPowerSrvSM,
                                            PowerSrvSMEvents_e theSMEvent);


/**
 * \
 * \date 020-Oct-2005\n
 * \brief This function sets the current SM working request.\n
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * 2) powerSrvRequest_t* pSmRequest - desierd request./n
 * Return Value: TI_STATUS -  TI_OK.\n
 */
TI_STATUS powerSrvSm_setSmRequest(TI_HANDLE hPowerSrvSM,powerSrvRequest_t* pSmRequest);



/**
 * \
 * \date 09-Jun-2004\n
 * \brief get the current state of the state machine.
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * Return Value: PowerCtrlSMStates_e.\n
 */
PowerSrvSMStates_e powerSrvSM_getCurrentState(TI_HANDLE hPowerSrvSM);


/**
 * \
 * \date 20-July-2004\n
 * \brief sets rate modulation
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * 2) rate_e rate modulation
 * Return Value: void.\n
 */
void powerSrvSM_setRateModulation(TI_HANDLE hPowerSrvSM, TI_UINT16 rateModulation);

/**
 * \brief sets rate modulation
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n* 
 * Return Value: TI_UINT16 Rate.\n
 */
TI_UINT32 powerSrvSM_getRateModulation(TI_HANDLE hPowerSrvSM);

/**
 * \
 * \date 20-July-2004\n
 * \brief print configuration of the PowerSrvSM object - use for debug!
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * 1) TI_HANDLE - handle to the PowerSrvSM object.\n
 * Return Value: void.\n
 */
void powerSrvSM_printObject(TI_HANDLE hPowerSrvSM);

/**
 * \
 * \date 21-August-2006\n
 * \brief Registers a failure event callback for power save error notifications (timer expiry).\n
 *
 * Function Scope \e Public.\n
 * Parameters:\n
 * - hPowerSrvSM      - handle to the PowerSrv object.        
 * - failureEventCB     - the failure event callback function.
 * - hFailureEventObj   - handle to the object passed to the failure event callback function.
*/
void powerSrvSM_RegisterFailureEventCB( TI_HANDLE hPowerSrvSM, 
                                        void* failureEventCB, TI_HANDLE hFailureEventObj );
#endif /*  _POWER_SRV_SM_H_  */

