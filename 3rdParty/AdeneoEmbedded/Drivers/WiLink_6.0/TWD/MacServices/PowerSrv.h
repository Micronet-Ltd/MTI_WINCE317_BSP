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
/** \file powerSrv.h
 *  \brief This is the Power Manager module private (internal).
 *  \
 *  \date 27-Apr-2004
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Power Manager                                                 *
 *   PURPOSE: Power Manager Module private                                      *
 *                                                                          *
 ****************************************************************************/

#ifndef _POWER_SRV_H_
#define _POWER_SRV_H_

#include "TWDriverInternal.h"
#include "PowerSrv_API.h"

/*#include "PowerSrvSM.h"*/

/*****************************************************************************
 **         Constants                                                       **
 *****************************************************************************/


/*****************************************************************************
 **         Enumerations                                                    **
 *****************************************************************************/





/*new Power*/


typedef enum 
{
	DRIVER_MODE,
	USER_MODE
}powerSrvMode_e;  

typedef enum 
{
	NEW_REQUEST,
	PENDING_REQUEST,
	RUNNING_REQUEST,
	HANDLED_REQUEST
}powerSrvRequestState_e;


/*****************************************************************************
 **         Typedefs                                                        **
 *****************************************************************************/




/*****************************************************************************
 **         Structures                                                      **
 *****************************************************************************/



typedef struct
{
	powerSrvRequestState_e 		requestState;
	powerSrvMode_e				requestMode;
	E80211PsMode	            psMode;
	TI_BOOL 						sendNullDataOnExit;
	void* 						powerSaveCBObject;
	powerSaveCmpltCB_t    		powerSrvCompleteCB;
	powerSaveCmdResponseCB_t	powerSaveCmdResponseCB;
}	powerSrvRequest_t;   

/** \struct powerSrv_t
 * this structure contain the data of the PowerSrv object.
 */


typedef struct
{
	TI_HANDLE   			hPowerSrvSM;            /**<
                                             					* Hnadle to the Power Server State Machine object.
                                            			 		*/

	TI_HANDLE   			hOS;                        	/**<
                                             					* Handle to the OS object.
                                             					*/

    TI_HANDLE   			hReport;                    	/**<
                                            	 				* Handle to the Report module.
                                             					*/

	TI_HANDLE   			hEventMbox;         		/**< Handle to the power controller object via the WhalCtrl.
											 * Need for configure the desired power mode policy in the system.
											 */
    TI_HANDLE               hCmdBld;
											 
	powerSrvMode_e 		currentMode; 		/**<
											*holds the current mode of the PSS - driver or user...
											*/

	powerSrvMode_e 		userLastRequestMode;
											/**<
											*
											*/

	powerSrvRequest_t 	userRequest;		/**<
											*user request struct.
											*/

	powerSrvRequest_t 	driverRequest;		/**<
											*driver request struct
											*/

	powerSrvRequest_t*  	pCurrentRequest;	/**<
											*pointer to the current request - user/driver request
											*/
	
	TFailureEventCb		failureEventFunc;	/**<
											* upper layer Failure Event CB.
											* called when the scan command has been Timer Expiry
											*/
	TI_HANDLE			failureEventObj;		/**<
											* object parameter passed to the failureEventFunc
											* when it is called 
											*/
	
} powerSrv_t;



/*****************************************************************************
 **         External data definitions                                       **
 *****************************************************************************/


/*****************************************************************************
 **         External functions definitions                                  **
 *****************************************************************************/





/*****************************************************************************
 **         Public Function prototypes                                      **
 *****************************************************************************/

/*****************************************************************************
 **         Private Function prototypes                                     **
 *****************************************************************************/

/*****************************************************************************
 **         Private Function prototypes                                     **
 *****************************************************************************/

#endif /*  _POWER_SRV_H_  */

