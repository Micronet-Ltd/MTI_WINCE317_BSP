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
/** \file currBssApi.h
 *  \brief Current BSS module API
 *
 *  \see currBss.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Current BSS                                               *
 *   PURPOSE: Current BSS Module API                                    *
 *                                                                          *
 ****************************************************************************/

#ifndef _CURR_BSS_API_H_
#define _CURR_BSS_API_H_

#include "siteMgrApi.h"
#include "roamingMngrTypes.h"
#include "paramOut.h"

/* Constants */

/* Enumerations */

/* Structures */

/**
* Current BSS control block 
* Following structure defines parameters that can be configured externally,
* internal variables, and handlers of other modules used by Current BSS module
*/

TI_STATUS currBss_registerBssLossEvent(TI_HANDLE hCurrBSS,TI_UINT32  uNumOfBeacons, TI_UINT16 uClientID);
TI_STATUS currBss_registerTxRetryEvent(TI_HANDLE hCurrBSS,TI_UINT8   uMaxTxRetryThreshold, TI_UINT16 uClientID);
TI_INT8 currBSS_RegisterTriggerEvent (TI_HANDLE hCurrBSS, TI_UINT8 triggerID,TI_UINT16 clientID, void* fCB, TI_HANDLE hCB);
TI_STATUS currBSS_setParam(TI_HANDLE hCurrBSS, paramInfo_t *pParam);
TI_STATUS currBSS_getParam(TI_HANDLE hCurrBSS, paramInfo_t *pParam);

#endif /*  _CURR_BSS_API_H_*/

