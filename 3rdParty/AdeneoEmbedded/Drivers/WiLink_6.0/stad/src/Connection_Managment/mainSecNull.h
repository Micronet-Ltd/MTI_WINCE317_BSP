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
/** \file MainSecSm.h
 *  \brief RSN main security SM
 *
 *  \see MainSecSm.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	MainSecSm.h												   */
/*    PURPOSE:	RSN main security SM									   */
/*																	 	   */
/***************************************************************************/

#ifndef _MAIN_SEC_NULL_SM_H
#define _MAIN_SEC_NULL_SM_H

#include "fsm.h"
#include "mainSecSm.h"

/* Constants */

/* Enumerations */

/* Typedefs */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS mainSecSmNull_config(mainSec_t *pMainSec, 
                            TRsnPaeConfig *pPaeConfig);

TI_STATUS mainSecSmNull_start(mainSec_t *pMainSec);

TI_STATUS mainSecSmNull_stop(mainSec_t *pMainSec);

TI_STATUS mainSecNull_reportKeysStatus(mainSec_t *pMainSec, TI_STATUS keysStatus);

TI_STATUS mainSecNull_getAuthState(mainSec_t *pMainSec, TIWLN_SECURITY_STATE *supp1XState);

TI_STATUS mainSecNull_reportAuthFailure(mainSec_t *pMainSec, EAuthStatus authStatus);
TI_STATUS mainSecNull_setAuthIdentity(mainSec_t *pMainSec, authIdentity_t *authIdentity);

#endif

