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
/** \file connApi.h
 *  \brief connection module API
 *
 *  \see conn.c
 */

/***************************************************************************/
/*																									*/
/*	  MODULE:	connApi.h																*/
/*    PURPOSE:	connection module API			 								*/
/*																									*/
/***************************************************************************/
#ifndef __CONN_API_H__
#define __CONN_API_H__

#include "tidef.h"
#include "paramOut.h"
#include "802_11Defs.h"
#include "mlmeApi.h"
#include "DrvMainModules.h"

/*
 * Connection type enum
 */
typedef enum
{
	CONN_TYPE_FIRST_CONN = 0,  /* Standart 802.11 association */
	CONN_TYPE_ROAM             /* Perform roaming connection. (Re Association) */ 

} EConnType;


typedef enum
{
    CONN_TYPE_PARAM         = 0x01

} EConnParam;


/* 
	Prototype for connection status announcment, this function is called upon connection
   lost or connection establishment.
*/
typedef void (*conn_status_callback_t)( TI_HANDLE hObj, mgmtStatus_e status, TI_UINT32 uStatusCode);

/* Connection interface functions prototypes */

TI_HANDLE conn_create (TI_HANDLE hOs);

void      conn_init (TStadHandlesList *pStadHandles);

TI_STATUS conn_SetDefaults (TI_HANDLE hConn, connInitParams_t *pConnInitParams);

TI_STATUS conn_unLoad (TI_HANDLE hConn);

TI_STATUS conn_setParam (TI_HANDLE hConn, paramInfo_t *pParam);

TI_STATUS conn_getParam (TI_HANDLE hConn, paramInfo_t *pParam);

TI_STATUS conn_start(TI_HANDLE hConn, 
                     EConnType connType,
                     conn_status_callback_t  pConnStatusCB,
                     TI_HANDLE connStatCbObj,
                     TI_BOOL disConEraseKeys,
                     TI_BOOL reNegotiateTspec);

void connInfraJoinCmdCmpltNotification(TI_HANDLE CB_handle);

TI_STATUS conn_stop (TI_HANDLE               hConn, 
                     DisconnectType_e       disConnType,
				     mgmtStatus_e 			reason,
				     TI_BOOL					disConEraseKeys,
				     conn_status_callback_t  pConnStatusCB,
				     TI_HANDLE				connStatCbObj );

void conn_timeout (TI_HANDLE hConn, TI_BOOL bTwdInitOccured);

TI_STATUS conn_ibssStaJoined (TI_HANDLE hConn);
TI_STATUS conn_ibssMerge (TI_HANDLE hConn);

TI_STATUS conn_reportMlmeStatus(TI_HANDLE			hConn, 
							mgmtStatus_e		status,
							TI_UINT16				uStatusCode);

TI_STATUS conn_reportRsnStatus(TI_HANDLE			hConn, 
							mgmtStatus_e		status);

#ifdef REPORT_LOG
void conn_ibssPrintStatistics (TI_HANDLE hConn);
#endif /*TI_DBG*/

#endif /* __CONN_API_H__ */
