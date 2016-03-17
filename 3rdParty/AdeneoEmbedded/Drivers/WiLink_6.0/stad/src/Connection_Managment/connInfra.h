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
/** \file connInfra.h
 *  \brief Infra connection header file
 *
 *  \see connInfra.c
 */

/***************************************************************************/
/*																									*/
/*	  MODULE:	infraConn.h																*/
/*    PURPOSE:	Infrastructure connection header file			 								*/
/*																									*/
/***************************************************************************/
#ifndef __CONN_INFRA_H__
#define __CONN_INFRA_H__

#include "tidef.h"
#include "paramOut.h"
#include "conn.h"

/* Infra connection SM events */
typedef enum 
{
	CONN_INFRA_CONNECT				= 0,
	CONN_INFRA_SCR_SUCC ,
    CONN_INFRA_JOIN_CMD_CMPLT ,
	CONN_INFRA_DISCONNECT ,
	CONN_INFRA_MLME_SUCC ,
	CONN_INFRA_RSN_SUCC	,
	CONN_INFRA_HW_CONFIGURED ,
	CONN_INFRA_DISCONN_COMPLETE ,
    CONN_INFRA_NUM_EVENTS			
} connInfraEvent_e;

/* Infra connection states */
typedef enum
{
	STATE_CONN_INFRA_IDLE	                 = 0,
	STATE_CONN_INFRA_SCR_WAIT_CONN			 = 1,
    STATE_CONN_INFRA_WAIT_JOIN_CMPLT         = 2,
	STATE_CONN_INFRA_MLME_WAIT			     = 3,
	STATE_CONN_INFRA_RSN_WAIT			     = 4,
	STATE_CONN_INFRA_CONFIG_HW 		         = 5,
    STATE_CONN_INFRA_CONNECTED			     = 6,
    STATE_CONN_INFRA_SCR_WAIT_DISCONN   	 = 7,
    STATE_CONN_INFRA_WAIT_DISCONNECT         = 8,
    CONN_INFRA_NUM_STATES
} infra_state_e;

TI_STATUS conn_infraConfig(conn_t *pConn);

TI_STATUS conn_infraSMEvent(TI_UINT8 *currentState, TI_UINT8 event, TI_HANDLE hConn);

TI_STATUS connInfra_JoinCmpltNotification(TI_HANDLE hconn);

void connInfra_DisconnectComplete (conn_t *pConn, TI_UINT8  *data, TI_UINT8   dataLength);

#endif /* __CONN_INFRA_H__ */
