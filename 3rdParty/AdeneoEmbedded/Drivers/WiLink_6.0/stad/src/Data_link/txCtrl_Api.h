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
/*    MODULE:   txCtrl_Api.h                                               */
/*    PURPOSE:  TxCtrl module API Header file                              */
/*                                                                         */
/***************************************************************************/
#ifndef _TX_CTRL_API_H_
#define _TX_CTRL_API_H_

#include "GeneralUtilApi.h"
#include "DrvMainModules.h"

/* TxCtrl Xmit results */
typedef enum
{
    STATUS_XMIT_SUCCESS,
    STATUS_XMIT_BUSY,
    STATUS_XMIT_ERROR
} EStatusXmit;


typedef struct 
{
	TI_BOOL    bHtEnable;	                        /* current flag of HT Capabilities enabled */
    TI_UINT32  uTxCtrlHtControl;        	        /* The HT Control Field for futur use. for now empty and the FW set it */
} TtxCtrlHtControl;


/* Build the buffers descriptor of a typical two buffers (header & data) Tx packet */
#define BUILD_TX_TWO_BUF_PKT_BDL(pPktCtrlBlk, pHdrBuf, uHdrLen, pDataBuf, uDataLen)  \
    pPktCtrlBlk->tTxnStruct.aBuf[0]   = (TI_UINT8 *) (pHdrBuf);    \
    pPktCtrlBlk->tTxnStruct.aLen[0]   = (TI_UINT16 ) (uHdrLen);    \
    pPktCtrlBlk->tTxnStruct.aBuf[1]   = (TI_UINT8 *) (pDataBuf);   \
    pPktCtrlBlk->tTxnStruct.aLen[1]   = (TI_UINT16 ) (uDataLen);   \
    pPktCtrlBlk->tTxnStruct.aLen[2]   = 0;                         \
    pPktCtrlBlk->tTxDescriptor.length = (TI_UINT16)((uHdrLen) + (uDataLen));


/****************************************************************/
/*                  MODULE  PUBLIC  FUNCTIONS                   */
/****************************************************************/ 

/* 
 *  The TxCtrl MAIN public functions (in txCtrl.c):
 */
TI_HANDLE txCtrl_Create (TI_HANDLE hOs);
void      txCtrl_Init (TStadHandlesList *pStadHandles);
TI_STATUS txCtrl_SetDefaults (TI_HANDLE hTxCtrl, txDataInitParams_t *txDataInitParams);
TI_STATUS txCtrl_Unload (TI_HANDLE hTxCtrl);
EStatusXmit txCtrl_XmitData (TI_HANDLE hTxCtrl, TTxCtrlBlk *pPktCtrlBlk);
TI_STATUS txCtrl_XmitMgmt (TI_HANDLE hTxCtrl, TTxCtrlBlk *pPktCtrlBlk);
void      txCtrl_UpdateQueuesMapping (TI_HANDLE hTxCtrl);
void *    txCtrl_AllocPacketBuffer (TI_HANDLE hTxCtrl, TTxCtrlBlk *pPktCtrlBlk, TI_UINT32 uPacketLen);
void      txCtrl_FreePacket (TI_HANDLE hTxCtrl, TTxCtrlBlk *pPktCtrlBlk, TI_STATUS eStatus);
TI_STATUS txCtrl_NotifyFwReset(TI_HANDLE hTxCtrl);
TI_STATUS txCtrl_CheckForTxStuck(TI_HANDLE hTxCtrl);
TI_UINT32 txCtrl_BuildDataPktHdr (TI_HANDLE hTxCtrl, TTxCtrlBlk *pPktCtrlBlk, AckPolicy_e ackPolicy);


/* 
 *  The txCtrlParams.c sub-module public functions:
 */
void      txCtrlParams_resetCounters(TI_HANDLE hTxCtrl);
TI_HANDLE txCtrlParams_RegNotif(TI_HANDLE hTxCtrl, 
                                TI_UINT16 EventMask, 
                                GeneralEventCall_t CallBack,
                                TI_HANDLE context, 
                                TI_UINT32 Cookie);
TI_STATUS txCtrlParams_AddToNotifMask(TI_HANDLE hTxCtrl, TI_HANDLE Notifh, TI_UINT16 EventMask);
TI_STATUS txCtrlParams_UnRegNotif(TI_HANDLE hTxCtrl, TI_HANDLE RegEventHandle);
TI_STATUS txCtrlParams_setAdmissionCtrlParams(TI_HANDLE hTxCtrl, 
                                              TI_UINT8 acId, 
                                              TI_UINT16 mediumTime, 
                                              TI_UINT32 minimumPHYRate, 
                                              TI_BOOL admFlag);
TI_STATUS txCtrlParams_getParam(TI_HANDLE hTxCtrl, paramInfo_t *pParamInfo);    
TI_STATUS txCtrlParams_setParam(TI_HANDLE hTxCtrl, paramInfo_t *pParamInfo);
TI_STATUS txCtrlParams_SetHtControl (TI_HANDLE hTxCtrl, TtxCtrlHtControl *pHtControl);
void txCtrlParams_setBssId (TI_HANDLE hTxCtrl, TMacAddr *pCurrBssId);
void txCtrlParams_setBssType (TI_HANDLE hTxCtrl, ScanBssType_e currBssType);
void txCtrlParams_setQosHeaderConverMode (TI_HANDLE hTxCtrl, EHeaderConvertMode  headerConverMode);
void txCtrlParams_setCurrentPrivacyInvokedMode (TI_HANDLE hTxCtrl, TI_BOOL currentPrivacyInvokedMode);
void txCtrlParams_setEapolEncryptionStatus (TI_HANDLE hTxCtrl, TI_BOOL eapolEncryptionStatus);
void txCtrlParams_setEncryptionFieldSizes (TI_HANDLE hTxCtrl, TI_UINT8 encryptionFieldSize);
void txCtrlParams_getCurrentEncryptionInfo (TI_HANDLE hTxCtrl, 
                                            TI_BOOL    *pCurrentPrivacyInvokedMode,
                                            TI_UINT8   *pEncryptionFieldSize);
ERate txCtrlParams_GetTxRate (TI_HANDLE hTxCtrl);
void txCtrlParams_setAcAdmissionStatus (TI_HANDLE hTxCtrl, 
                                        TI_UINT8 ac, 
                                        EAdmissionState admissionRequired,
                                        ETrafficAdmState admissionState);
void txCtrlParams_setAcMsduLifeTime (TI_HANDLE hTxCtrl, TI_UINT8 ac, TI_UINT32 msduLifeTime);
void txCtrlParams_setAcAckPolicy (TI_HANDLE hTxCtrl, TI_UINT8 ac, AckPolicy_e ackPolicy);
void txCtrlParams_updateMgmtRateAttributes(TI_HANDLE hTxCtrl, TI_UINT8 ratePolicyId, TI_UINT8 ac);
void txCtrlParams_updateDataRateAttributes(TI_HANDLE hTxCtrl, TI_UINT8 ratePolicyId, TI_UINT8 ac);
void txCtrlParams_updateTxSessionCount(TI_HANDLE hTxCtrl, TI_UINT16 txSessionCount);
#ifdef TI_DBG
void txCtrlParams_printInfo(TI_HANDLE hTxCtrl);
void txCtrlParams_printDebugCounters(TI_HANDLE hTxCtrl);
void txCtrlParams_resetDbgCounters(TI_HANDLE hTxCtrl);
#endif /* TI_DBG */


/* 
 *  The txCtrlServ.c sub-module public functions:
 */
TI_STATUS txCtrlServ_buildNullFrame(TI_HANDLE hTxCtrl, TI_UINT8* pFrame, TI_UINT32* pLength);
TI_STATUS txCtrlServ_buildWlanHeader(TI_HANDLE hTxCtrl, TI_UINT8* pFrame, TI_UINT32* pLength);

#endif /* _TX_CTRL_API_H_ */
