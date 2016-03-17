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
/** \file mainSecSm.c
 *  \brief 802.1X finite state machine header file
 *
 *  \see mainSecSm.h
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	mainSecSm.c											   	   */
/*    PURPOSE:	Main Security State Machine API					   		   */
/*																	 	   */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_38
#include "osApi.h"
#include "paramOut.h"
#include "report.h"
#include "DataCtrl_Api.h"
#include "smeApi.h"
#include "rsn.h"
#include "rsnApi.h"
#include "mainSecSm.h"
#include "mainSecNull.h"

/* Constants */

/* Enumerations */

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Global variables */

/* Local function prototypes */

/* functions */

/**
*
* rsn_mainSecSmFullInit
*
* \b Description: 
*
* Init main security state machine state machine
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSecSmNull_config(mainSec_t *pMainSec, 
                            TRsnPaeConfig *pPaeConfig)
{
    pMainSec->start = (mainSecSmStart_t)mainSecSmNull_start;
	pMainSec->stop = mainSecSmNull_stop;
	pMainSec->reportAuthStatus = NULL;
	pMainSec->reportKeysStatus = mainSecNull_reportKeysStatus;
	pMainSec->reportReAuthenticate = NULL;
	pMainSec->getAuthIdentity  = NULL;
	pMainSec->setAuthIdentity  = (mainSecSm_getAuthIdentity_t)mainSecNull_setAuthIdentity;
    pMainSec->getAuthState = mainSecNull_getAuthState;
    pMainSec->reportAuthFailure  = (mainSecSm_reportAuthFailure_t)mainSecNull_reportAuthFailure;
    return TI_OK;
}

/**
*
* mainSecSmNull_Start
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSecSmNull_start(mainSec_t *pMainSec)
{
    TI_STATUS status;

    status = rsn_reportStatus(pMainSec->pParent, TI_OK); 
    
    return status;
}

/**
*
* mainSecSmNull_Stop
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSecSmNull_stop(mainSec_t *pMainSec)
{
    return TI_OK;
}

/**
*
* mainSecNull_reportKeysStatus
*
* \b Description: 
*
* Start the NULL main security SM. Reports success to the rsn module immediately.
*
* \b ARGS:
*
*  none
*
* \b RETURNS:
*
*  TI_OK on success, TI_NOK otherwise.
*
* \sa 
*/
TI_STATUS mainSecNull_reportKeysStatus(mainSec_t *pMainSec, TI_STATUS keysStatus)
{

	return TI_OK;
} 
/**
*
* mainSecKeysOnly_getAuthState:  \n 
*
* \b Description: 
*
* Get authentication state from supp1x SM.
*
* \b ARGS:
*
*  I   - pMainSec - pMainSec SM context  \n
*  I   - authIdentity - pointer to authentication state \n
*
* \b RETURNS:
*
*  TI_OK if successful, TI_NOK otherwise.
*
* \sa 
*/

TI_STATUS mainSecNull_getAuthState(mainSec_t *pMainSec, TIWLN_SECURITY_STATE *secState)
{
	*secState = eSecurityStateHalted; 
	return TI_OK;

} /*mainSecKeysOnly_getAuthState*/


TI_STATUS mainSecSmNull_nop(mainSec_t *pMainSec)
{
	return TI_OK;

} /*mainSecKeysOnly_getAuthState*/


TI_STATUS mainSecNull_reportAuthFailure(mainSec_t *pMainSec, EAuthStatus authStatus) 
{
    return TI_OK;
}


TI_STATUS mainSecNull_setAuthIdentity(mainSec_t *pMainSec, authIdentity_t *authIdentity)
{

	return TI_OK;
}
