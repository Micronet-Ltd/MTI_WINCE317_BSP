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
/*    MODULE:   txCtrl.h                                                   */
/*    PURPOSE:  txCtrl module Header file                                  */
/*                                                                         */
/***************************************************************************/
#ifndef _TX_CTRL_H_
#define _TX_CTRL_H_


#include "paramOut.h"
#include "DataCtrl_Api.h"


extern void wlanDrvIf_FreeTxPacket (TI_HANDLE hOs, TTxCtrlBlk *pPktCtrlBlk, TI_STATUS eStatus);


#define DEF_TX_PORT_STATUS              CLOSE
#define DEF_CURRENT_PRIVACY_MODE        TI_FALSE
#define DEF_EAPOL_ENCRYPTION_STATUS     TI_FALSE
#define HEADER_PAD_SIZE                 2       /* 2-byte pad before header with QoS, for 4-byte alignment */
#define MGMT_PKT_LIFETIME_TU            2000    /* Mgmt pkts lifetime in TUs (1024 usec). */

/* defined in qosMngr.c - standard WMM translation from TID to AC. */
extern int WMEQosTagToACTable[MAX_NUM_OF_802_1d_TAGS];
extern const TI_UINT8 WMEQosAcToTid[MAX_NUM_OF_AC];

/* The TX delay histogram ranges start and end in uSec. */
static const TI_UINT32 txDelayRangeStart[TX_DELAY_RANGES_NUM] = {    0,  1000, 10000, 20000, 40000, 60000,  80000, 100000, 200000 };
static const TI_UINT32 txDelayRangeEnd  [TX_DELAY_RANGES_NUM] = { 1000, 10000, 20000, 40000, 60000, 80000, 100000, 200000, 0xFFFFFFFF };

/* BE is ordered here above BK for priority sensitive functions (BE is 0 but has higher priority than BK). */
static const EAcTrfcType priorityOrderedAc[] = {QOS_AC_BK, QOS_AC_BE, QOS_AC_VI, QOS_AC_VO};

typedef struct 
{
    TI_UINT32  dbgNumPktsSent[MAX_NUM_OF_AC];       /* Pkts sent by data-queue or mgmt-queue. */
    TI_UINT32  dbgNumPktsBackpressure[MAX_NUM_OF_AC];/* Pkts for which backpressure was set by HW-Q */
    TI_UINT32  dbgNumPktsBusy[MAX_NUM_OF_AC];      /* Pkts for which busy was received from HW-Q */
    TI_UINT32  dbgNumPktsXfered[MAX_NUM_OF_AC];    /* Pkts sent to Xfer */
    TI_UINT32  dbgNumPktsSuccess[MAX_NUM_OF_AC];   /* Pkts for which success was received from Xfer */
    TI_UINT32  dbgNumPktsPending[MAX_NUM_OF_AC];   /* Pkts for which pending was received from Xfer */
    TI_UINT32  dbgNumPktsError[MAX_NUM_OF_AC];     /* Pkts for which error was received from Xfer */
    TI_UINT32  dbgNumTxCmplt[MAX_NUM_OF_AC];        /* Pkts that reached complete CB */
    TI_UINT32  dbgNumTxCmpltOk[MAX_NUM_OF_AC];     /* Pkts that reached complete CB with status TI_OK */
    TI_UINT32  dbgNumTxCmpltError[MAX_NUM_OF_AC];  /* Pkts that reached complete CB with status TI_NOK */
    TI_UINT32  dbgNumTxCmpltOkBytes[MAX_NUM_OF_AC];/* Acknowledged bytes (complete status TI_OK) */
    TI_UINT32  dbgNumXferCmplt;                    /* Number of Xfer-Complete events (after pending). */
} txDataDbgCounters_t;



/* 
 *  Module object structure. 
 */
typedef struct 
{
    /* Handles */
    TI_HANDLE           hOs;
    TI_HANDLE           hReport;
    TI_HANDLE           hCtrlData;
    TI_HANDLE           hTWD;
    TI_HANDLE           hTxDataQ;
    TI_HANDLE           hTxMgmtQ;
    TI_HANDLE           hEvHandler;
    TI_HANDLE           TxEventDistributor;
    TI_HANDLE           hHealthMonitor;
    TI_HANDLE           hTimer;
    TI_HANDLE           hStaCap;
    TI_HANDLE           hXCCMngr;
    TI_HANDLE           hQosMngr;
    TI_HANDLE           hRxData;

    TI_HANDLE           hCreditTimer;   /* The medium-usage credit timer handle */

    /* External parameters */
    EHeaderConvertMode  headerConverMode;  /* QoS header needed for data or not. */
    TI_BOOL             currentPrivacyInvokedMode;
    TI_BOOL             eapolEncryptionStatus;
    TI_UINT8            encryptionFieldSize;  /* size to reserve in WLAN header for encryption */
    ScanBssType_e       currBssType;
    TMacAddr            currBssId;
    TI_UINT16           aMsduLifeTimeTu[MAX_NUM_OF_AC];
    AckPolicy_e         ackPolicy[MAX_NUM_OF_AC];
    TtxCtrlHtControl    tTxCtrlHtControl;
	TI_UINT16           genericEthertype;

    /* ACs admission and busy mapping */
    TI_UINT32           busyAcBitmap;   /* Current bitmap of busy ACs (in HW-Q backpressure format). */
    TI_UINT32           busyTidBitmap;  /* Current bitmap of busy TIDs reflected from admitted ACs. */
    TI_UINT32           admittedAcToTidMap[MAX_NUM_OF_AC]; /* From HW-AC to bitmap of TIDs that currently use it. */
    EAcTrfcType         highestAdmittedAc[MAX_NUM_OF_AC]; /* Provide highest admitted AC equal or below given AC. */
    ETrafficAdmState    admissionState[MAX_NUM_OF_AC];    /* AC is allowed to transmit or not. */
    EAdmissionState     admissionRequired[MAX_NUM_OF_AC]; /* AC requires AP's admission or not. */

    /* Tx Attributes */
    TI_UINT32           mgmtRatePolicy[MAX_NUM_OF_AC];  /* Current rate policy for mgmt packets per AC. */
    TI_UINT32           dataRatePolicy[MAX_NUM_OF_AC];  /* Current rate policy for data packets per AC. */
    TI_UINT16           txSessionCount;     /* Current Tx-Session index as configured to FW in last Join command. */
    TI_UINT16           dataPktDescAttrib;  /* A prototype of Tx-desc attrib bitmap for data pkts. */
    TI_UINT8            dbgPktSeqNum;       /* Increment every tx-pkt, insert in descriptor for debug. */

    /* Counters */
    TTxDataCounters     txDataCounters[MAX_NUM_OF_AC]; /* Save Tx statistics per Tx-queue. */
    TI_UINT32           SumTotalDelayUs[MAX_NUM_OF_AC]; /* Store pkt delay sum in Usecs to avoid divide per 
                                                            pkt, and covert to msec on user request. */
    TI_UINT32           currentConsecutiveRetryFail; /* current consecutive number of tx failures due to max retry */
    ERate               eCurrentTxRate;                 /* Save last data Tx rate for applications' query */

    /* credit calculation parameters */
	TI_BOOL				bCreditCalcTimerEnabled;        /* credit timer is enabled from registry */
	TI_BOOL				bCreditCalcTimerRunning;        /* credit calculation timer is running */
    TI_UINT32           creditCalculationTimeout;
    TI_INT32            lowMediumUsageThreshold[MAX_NUM_OF_AC];
    TI_INT32            highMediumUsageThreshold[MAX_NUM_OF_AC];
    TI_UINT32           lastCreditCalcTimeStamp[MAX_NUM_OF_AC];
    TI_BOOL             useAdmissionAlgo[MAX_NUM_OF_AC];
    TI_INT32            credit[MAX_NUM_OF_AC];
    TI_UINT32           mediumTime[MAX_NUM_OF_AC];
    TI_UINT32           totalUsedTime[MAX_NUM_OF_AC];

#ifdef TI_DBG
    txDataDbgCounters_t dbgCounters;    /* debug counters */
#endif

} txCtrl_t;



#endif  /* _TX_CTRL_H_ */
