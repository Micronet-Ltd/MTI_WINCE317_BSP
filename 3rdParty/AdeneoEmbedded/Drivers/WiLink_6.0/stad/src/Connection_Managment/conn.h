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
/** \file conn.h
 *  \brief connection module internal header file
 *
 *  \see conn.c
 */

/***************************************************************************/
/*																			*/
/*	  MODULE:	conn.h														*/
/*    PURPOSE:	connection module internal header file						*/
/*																			*/
/***************************************************************************/
#ifndef __CONN_H__
#define __CONN_H__

#include "tidef.h"
#include "paramOut.h"
#include "fsm.h"
#include "802_11Defs.h"
#include "connApi.h"
#include "scrApi.h"

typedef struct
{
	mgmtStatus_e			disAssocEventReason;		/* Disassoc reason to be used for upper layer	*/
	TI_UINT32					disAssocEventStatusCode;	/* Extra information for disConnEventReason		*/
} connSmContext_t;


/*
 *	ibssRandomSsidGen - in IBSS, that flag indicates if the ibss must be randommaly changed. 
 *	TI_FALSE - do not generate random ibss.
 *	TI_TRUE -	generate random ibss.
 *	1.	at Start up that flag is TI_FALSE
 *  2.	after Connection fail in connIbss or connSelf SM, set the flag to TI_TRUE in order to 
 *		generate a new random ibss.
 */

/* Connection handle */
typedef struct 
{
	TI_UINT8				state;	
	connSmContext_t			smContext;
	connectionType_e		currentConnType;
	TI_UINT32				timeout;
	TI_HANDLE               hConnTimer;         /* This timer is used both by IBSS and BSS */
	fsm_stateMachine_t		*ibss_pFsm;
    fsm_stateMachine_t		*infra_pFsm;
	EConnType				connType;
	DisconnectType_e		disConnType;
	mgmtStatus_e			disConnReasonToAP;
	TI_BOOL					disConEraseKeys;
    TI_UINT8                buf[40];            /* For dummy interrogate data (to flush cmdQueue) */
	conn_status_callback_t  pConnStatusCB;
	TI_HANDLE				connStatCbObj;
	TI_BOOL 				scrRequested[ SCR_RESOURCE_NUM_OF_RESOURCES ]; /* wether SCR was requested for the two resources */
    TI_BOOL                 bScrAcquired[ SCR_RESOURCE_NUM_OF_RESOURCES ]; /* wether SCR was acquired for the two resources */
	TI_UINT32               ibssDisconnectCount;

    /* Other modules handles */
	TI_HANDLE				hSiteMgr;
	TI_HANDLE				hSmeSm;
	TI_HANDLE				hMlmeSm;
	TI_HANDLE				hRsn;
	TI_HANDLE				hReport;
	TI_HANDLE				hOs;
	TI_HANDLE				hRxData;
	TI_HANDLE 				hPwrMngr;
	TI_HANDLE				hCtrlData;
	TI_HANDLE 				hQosMngr;
	TI_HANDLE				hTWD;
	TI_HANDLE				hMeasurementMgr;
	TI_HANDLE				hScr;
	TI_HANDLE				hTrafficMonitor;
	TI_HANDLE				hXCCMngr;
    TI_HANDLE               hScanCncn;
	TI_HANDLE				hCurrBss;
	TI_HANDLE				hSwitchChannel;
	TI_HANDLE				hEvHandler;
	TI_HANDLE				hHealthMonitor;
	TI_HANDLE				hTxMgmtQ;
    TI_HANDLE               hRegulatoryDomain;
    TI_HANDLE    			hSoftGemini;
    TI_HANDLE	            hTxCtrl;
    TI_HANDLE	            hTimer;
} conn_t;

#endif /* __CONN_H__*/
