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
/*                                                                         */
/*    MODULE:   tx.h                                                       */
/*    PURPOSE:  Tx module Header file                                      */
/*                                                                         */
/***************************************************************************/
#ifndef _CTRL_H_
#define _CTRL_H_

#include "tidef.h"
#include "paramOut.h"
#include "rxXfer_api.h"
#include "DataCtrl_Api.h"


#define     DEF_CURRENT_PREAMBLE                        PREAMBLE_LONG
#define     ALL_RATES_AVAILABLE                         0xFFFFFFFF                      

typedef enum
{
    CTRL_DATA_TRAFFIC_INTENSITY_HIGH_CROSSED_ABOVE,
    CTRL_DATA_TRAFFIC_INTENSITY_HIGH_CROSSED_BELOW,
    CTRL_DATA_TRAFFIC_INTENSITY_LOW_CROSSED_ABOVE,
    CTRL_DATA_TRAFFIC_INTENSITY_LOW_CROSSED_BELOW,
    CTRL_DATA_TRAFFIC_INTENSITY_MAX_EVENTS
} ctrlData_trafficIntensityEvents_e;


#define TS_EXCEEDS(currTime,expTime) (currTime > expTime)
#define TS_ADVANCE(currTime,expTime,delta) (expTime = currTime + (delta))

typedef struct 
{
    TI_UINT32 supportedRatesMask;
    TI_UINT32 policyClassRateMask;
    TI_UINT32 fwPolicyID;
}tsrsParameters_t;

typedef struct
{
    TI_HANDLE               hSiteMgr;
    TI_HANDLE               hTxCtrl;
    TI_HANDLE               hRxData;
    TI_HANDLE               hTWD;  
    TI_HANDLE               hOs;
    TI_HANDLE               hReport;
    TI_HANDLE               hAPConn;
    TI_HANDLE               hEvHandler;
    TI_HANDLE               hTrafficMonitor;
    TI_HANDLE               hTxDataQ;
    TI_HANDLE               hStaCap;
    TI_HANDLE               hRsn;

    TMacAddr                ctrlDataCurrentBSSID; 
    ScanBssType_e           ctrlDataCurrentBssType; 
    TI_UINT32               ctrlDataCurrentRateMask;
    EPreamble               ctrlDataCurrentPreambleType; 
    TMacAddr                ctrlDataDeviceMacAddress; 
    TI_BOOL                 ctrlDataProtectionEnabled;
    RtsCtsStatus_e          ctrlDataRtsCtsStatus;
    erpProtectionType_e     ctrlDataIbssProtectionType;
    erpProtectionType_e     ctrlDataDesiredIbssProtection; /* 0 = CTS protaction disable ; 1 = Standard CTS protaction */
    
    /*
     * txRatePolicy section
     */

    /* txRatePolicies - here we store the policy and set it to the FW */
    TTxRatePolicy           ctrlDataTxRatePolicy;

    /* number of retries for each rate in each class in the policy that we set to the FW */ 
	TI_UINT32               policyEnabledRatesMaskCck;
	TI_UINT32               policyEnabledRatesMaskOfdm;
	TI_UINT32               policyEnabledRatesMaskOfdmA;
	TI_UINT32               policyEnabledRatesMaskOfdmN;

    TI_UINT32               uCurrPolicyEnabledRatesMask;    /* holds the current used En/Dis Rates Mask */
    TI_UINT32               uMgmtPolicyId;                  /* the management packets policy ID */

#ifdef XCC_MODULE_INCLUDED
    /* Callback for update retries in Link Test */
    retriesCB_t             retriesUpdateCBFunc;
    TI_HANDLE               retriesUpdateCBObj;
#endif

    /* Flag to indicate whether traffic intensity events should be sent or not */
    TI_BOOL                 ctrlDataTrafficIntensityEventsEnabled;
    OS_802_11_TRAFFIC_INTENSITY_THRESHOLD_PARAMS ctrlDataTrafficIntensityThresholds;
    TI_HANDLE               ctrlDataTrafficThresholdEvents[CTRL_DATA_TRAFFIC_INTENSITY_MAX_EVENTS];

    tsrsParameters_t        tsrsParameters[MAX_NUM_OF_AC];

    /* holds last fragmentation threshold */
    TI_UINT16               lastFragmentThreshold;

} ctrlData_t;


#endif
