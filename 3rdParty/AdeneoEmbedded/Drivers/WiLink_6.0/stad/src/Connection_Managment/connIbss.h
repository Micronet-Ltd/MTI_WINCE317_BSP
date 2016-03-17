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
/** \file connIbss.h
 *  \brief IBSS connection header file
 *
 *  \see connIbss.c
 */

/***************************************************************************/
/*																									*/
/*	  MODULE:	ibssConn.h																*/
/*    PURPOSE:	IBSS connection header file			 								*/
/*																									*/
/***************************************************************************/
#ifndef __CONN_IBSS_H__
#define __CONN_IBSS_H__

#include "tidef.h"
#include "paramOut.h"
#include "conn.h"

/* IBSS connection SM events */
typedef enum 
{
	CONN_IBSS_CREATE   		= 0,  /* Sent when establishing new IBSS. */ 
	CONN_IBSS_CONNECT		  ,   /* Sent when trying to join existing IBSS. */
	CONN_IBSS_DISCONNECT	  ,	  /* Stops to connection */
	CONN_IBSS_RSN_SUCC		  ,   /* RSN keys are set to the HW */
	CONN_IBSS_STA_JOINED	  ,   /* Event sent when other STA joined our self IBSS */
	CONN_IBSS_MERGE			  ,
	CONN_IBSS_DISCONN_COMPLETE,   /* Called when disconncet event is done */ 
    CONN_IBSS_NUM_EVENTS            
} connIbssEvent_e;

/* IBSS connection states */
typedef enum
{
	STATE_CONN_IBSS_IDLE 		=0, 
	STATE_CONN_IBSS_SELF_WAIT	,
	STATE_CONN_IBSS_RSN_WAIT	,
	STATE_CONN_IBSS_CONNECTED 	,
	STATE_CONN_IBSS_WAIT_DISCONN_CMPLT,
    CONN_IBSS_NUM_STATES    
} conn_ibss_state_e;


TI_STATUS conn_ibssConfig(conn_t *pConn);

TI_STATUS conn_ibssSMEvent(TI_UINT8 *currentState, TI_UINT8 event, TI_HANDLE hConn);

void IbssConnSM_ScrCB( TI_HANDLE hConn, EScrClientRequestStatus requestStatus, EScrResourceId eResource, EScePendReason pendReason);

void connIbss_DisconnectComplete (conn_t *pConn, TI_UINT8  *data, TI_UINT8   dataLength);

#endif /* __CONN_IBSS_H__ */
