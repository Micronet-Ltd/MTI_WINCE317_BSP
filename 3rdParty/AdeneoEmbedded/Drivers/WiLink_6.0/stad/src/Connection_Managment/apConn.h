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
/** \file apConn.h
 *  \brief AP Connection Module API
 *
 *  \see apConn.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  AP Connection	    		                                	*
 *   PURPOSE: AP Connection Module API                              		*
 *                                                                          *
 ****************************************************************************/

#ifndef _AP_CONNECTION_H_
#define _AP_CONNECTION_H_

#include "802_11Defs.h"
#include "apConnApi.h"
#include "DrvMainModules.h"

/* Typedefs */
/* This struct is used for ROAMING_TRIGGER_AP_DISCONNECT */
typedef struct  
{
	TI_UINT16 uStatusCode;		/* status code of deauth/disassoc packet				   */
	TI_BOOL   bDeAuthenticate;	/* Whether this packet is DeAuth ( if DisAssoc than TI_FALSE) */
} APDisconnect_t;

typedef union
{
	APDisconnect_t APDisconnect;
	ERate		   rate;
} roamingEventData_u;

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

/* Called by Config Manager */
TI_HANDLE apConn_create(TI_HANDLE hOs);
TI_STATUS apConn_unload(TI_HANDLE hAPConnection);
void      apConn_init (TStadHandlesList *pStadHandles);
TI_STATUS apConn_SetDefaults (TI_HANDLE hAPConnection, apConnParams_t *pApConnParams);

/* Called by SME and Site Manager */
TI_STATUS apConn_start(TI_HANDLE hAPConnection, TI_BOOL roamingEnabled);
TI_STATUS apConn_stop(TI_HANDLE hAPConnection, TI_BOOL removeKeys);

void apConn_printStatistics(TI_HANDLE hAPConnection);


/* Called by Connection SM */
void apConn_ConnCompleteInd(TI_HANDLE hAPConnection, mgmtStatus_e status, TI_UINT32 uStatusCode);

void apConn_DisconnCompleteInd(TI_HANDLE hAPConnection, mgmtStatus_e status, TI_UINT32 uStatusCode);

/* Called by Current BSS, Rate Adaptation, RSN and other modules generating roaming events */
TI_STATUS apConn_reportRoamingEvent(TI_HANDLE hAPConnection,
									apConn_roamingTrigger_e roamingEventType,
									roamingEventData_u *pRoamingEventData);

/* Called by XCC Manager */
void apConn_RoamHandoffFinished(TI_HANDLE hAPConnection);
void apConn_getRoamingStatistics(TI_HANDLE hAPConnection, TI_UINT8 *roamingCount, TI_UINT16 *roamingDelay);
void apConn_resetRoamingStatistics(TI_HANDLE hAPConnection);

void apConn_updateNeighborAPsList(TI_HANDLE hAPConnection, neighborAPList_t *pListOfpriorityAps);

/* Called by Switch Channel */
TI_STATUS apConn_indicateSwitchChannelInProgress(TI_HANDLE hAPConnection);
TI_STATUS apConn_indicateSwitchChannelFinished(TI_HANDLE hAPConnection);

/* Called by Association SM */
TI_STATUS apConn_getVendorSpecificIE(TI_HANDLE hAPConnection, TI_UINT8 *pRequest, TI_UINT32 *len);


TI_BOOL apConn_isPsRequiredBeforeScan(TI_HANDLE hAPConnection);

void apConn_setDeauthPacketReasonCode(TI_HANDLE hAPConnection, TI_UINT8 deauthReasonCode);

#endif /*  _AP_CONNECTION_H_*/

