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

/***************************************************************************/
/*																		   */
/*		MODULE:	rx.h													   */
/*    	PURPOSE:	Rx module functions header file						   */
/*																		   */
/***************************************************************************/
#ifndef _RX_DATA_H_
#define _RX_DATA_H_

#include "paramOut.h"
#include "TWDriver.h"

#define DEF_EXCLUDE_UNENCYPTED				TI_FALSE
#define DEF_EAPOL_DESTINATION				OS_ABS_LAYER
#define DEF_RX_PORT_STATUS					CLOSE

#define FILTER_TYPE_ACTIVE	1
#define FILTER_TYPE_SUSPEND 2

#define MAX_ACTIVE_FILTERS 	(MAX_DATA_FILTERS-1)
typedef struct 
{
	TI_UINT32		excludedFrameCounter;	
	TI_UINT32		rxDroppedDueToVLANIncludedCnt;    
    TI_UINT32		rxWrongBssTypeCounter;
	TI_UINT32		rxWrongBssIdCounter;
    TI_UINT32      rcvUnicastFrameInOpenNotify;
}rxDataDbgCounters_t;


/*                         |                           |                         |
 31 30 29 28 | 27 26 25 24 | 23 22 21 20 | 19 18 17 16 | 15 14 13 12 | 11 10 9 8 | 7 6 5 4 | 3 2 1 0
                           |                           |                         |
*/                                           


typedef enum
{
	DATA_IAPP_PACKET  = 0,
	DATA_EAPOL_PACKET = 1,
	DATA_DATA_PACKET  = 2,
    DATA_VLAN_PACKET  = 3,
	MAX_NUM_OF_RX_DATA_TYPES
}rxDataPacketType_e;



typedef void (*rxData_pBufferDispatchert) (TI_HANDLE hRxData , void *pBuffer, TRxAttr *pRxAttr);


typedef struct 
{
	/* Handles */
	TI_HANDLE	 		hCtrlData;
	TI_HANDLE	 		hTWD;
	TI_HANDLE			hMlme;
	TI_HANDLE			hOs;
	TI_HANDLE			hRsn;
	TI_HANDLE			hReport;
	TI_HANDLE			hSiteMgr;
	TI_HANDLE			hXCCMgr;
    TI_HANDLE           hEvHandler;
    TI_HANDLE           hTimer;
    TI_HANDLE           RxEventDistributor;
	TI_HANDLE           hThroughputTimer;
	TI_HANDLE			hPowerMgr;
    TI_BOOL             rxThroughputTimerEnable;
	TI_BOOL             rxDataExcludeUnencrypted;
    TI_BOOL             rxDataExludeBroadcastUnencrypted;
	eapolDestination_e 	rxDataEapolDestination;

	portStatus_e  		rxDataPortStatus;
	
    /* Rx Data Filters */
    filter_e            filteringDefaultAction;
    TI_BOOL             filteringEnabled;
    TI_BOOL             isFilterSet[MAX_DATA_FILTERS];
    TRxDataFilterRequest filterRequests[MAX_DATA_FILTERS];

	/* Counters */
	rxDataCounters_t	rxDataCounters;
	rxDataDbgCounters_t	rxDataDbgCounters;

	rxData_pBufferDispatchert rxData_dispatchBuffer[MAX_NUM_OF_RX_PORT_STATUS][MAX_NUM_OF_RX_DATA_TYPES];

	TI_INT32				prevSeqNum;

 	TI_UINT32           uLastDataPktRate;  /* save Rx packet rate for statistics */

	TI_BOOL			    reAuthInProgress;
	TI_HANDLE			reAuthActiveTimer;
	TI_UINT32			reAuthActiveTimeout;


    /* Generic Ethertype support */
    TI_UINT16           genericEthertype;
}rxData_t;

#endif
