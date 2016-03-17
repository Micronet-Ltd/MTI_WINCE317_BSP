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
/** \file qosMngr_API.h
 *  \brief QOS manager module external header file
 *
 *  \see qosMngr.c
 */

/***************************************************************************/
/*                                                                         */
/*    MODULE:   qosMgr_API.h                                               */
/*    PURPOSE:  QOS manager module external header file                    */
/*                                                                         */
/***************************************************************************/
#ifndef __QOS_MNGR_API_H__
#define __QOS_MNGR_API_H__

#include "mlmeApi.h"
#include "tiQosTypes.h"
#include "DrvMainModules.h"

#define USER_PRIORITY_4 (WMEQosTagToACTable[4])
#define USER_PRIORITY_6 (WMEQosTagToACTable[6])

/* Used for TSPEC nominal fixed size */
#define FIXED_NOMINAL_MSDU_SIZE_MASK 0x8000

/* User priority is in second byte of tsInfoField (that follows 8 bytes from beginning of the TSPEC IE) */ 
#define GET_USER_PRIORITY_FROM_WME_TSPEC_IE(pTspecIe)   \
    (( *(((TI_UINT8 *)pData) + 9) & TS_INFO_1_USER_PRIORITY_MASK) >> USER_PRIORITY_SHIFT)



typedef struct 
{
    EAcTrfcType         AC;
    TI_UINT8            tid;
    TI_UINT8            userPriority;
    TI_UINT16           nominalMsduSize; /* in bytes */
    TI_UINT32           meanDataRate;    /* bits per second */
    TI_UINT16           surplausBwAllowance;
    TI_INT32            minimumPHYRate;
    TI_UINT16           mediumTime;
    TI_BOOL             UPSDFlag;
    TI_UINT32           uMinimumServiceInterval;
    TI_UINT32           uMaximumServiceInterval;
    EStreamDirection    streamDirection;
    ETrafficAdmState    trafficAdmState;
    TI_UINT8            statusCode;
}tspecInfo_t;

typedef enum 
{
    STATUS_TRAFFIC_ADM_REQUEST_ACCEPT       = 0,
    STATUS_TRAFFIC_ADM_REQUEST_REJECT       = 1,
    STATUS_TRAFFIC_ADM_REQUEST_TIMEOUT      = 2
} trafficAdmRequestStatus_e;


TI_HANDLE qosMngr_create(TI_HANDLE hOs);

TI_STATUS qosMngr_destroy(TI_HANDLE hQosMngr);

void      qosMngr_init (TStadHandlesList *pStadHandles);

TI_STATUS qosMngr_SetDefaults (TI_HANDLE hQosMngr, QosMngrInitParams_t *pQosMngrInitParams);

TI_STATUS qosMngr_disconnect (TI_HANDLE hQosMngr, TI_BOOL bDisconnect);

TI_STATUS qosMngr_connect(TI_HANDLE hQosMngr);

void qosMngr_SetBaPolicies(TI_HANDLE hQosMngr);

TI_STATUS qosMngr_setParams(TI_HANDLE  hQosMngr,paramInfo_t *pParamInfo);

TI_STATUS qosMngr_getParamsActiveProtocol(TI_HANDLE hQosMngr, EQosProtocol *actProt);

TI_STATUS qosMngr_getParams(TI_HANDLE  hQosMngr,paramInfo_t *pParamInfo);

TI_STATUS qosMngr_assocReqBuild(TI_HANDLE  hQosMngr, TI_UINT8 *pQosIe, TI_UINT32 *pLen);

TI_STATUS qosMngr_setSite(TI_HANDLE hQosMngr, assocRsp_t *assocRsp);

void      qosMngr_updateIEinfo(TI_HANDLE hQosMngr, TI_UINT8 *pQosIeParams, EQosProtocol qosSetProtocol);

TI_UINT8 qosMngr_evalSite(TI_HANDLE hQosMngr, TI_BOOL siteAPSDSupport);

TI_STATUS qosMngr_getQosCapabiltyInfeElement(TI_HANDLE  hQosMngr, TI_UINT8 *pQosIe, TI_UINT32 *pLen);

TI_STATUS qosMngr_requestAdmission(TI_HANDLE            hQosMngr, 
                                   OS_802_11_QOS_TSPEC_PARAMS *addTspecParams);

TI_STATUS qosMngr_deleteAdmission(TI_HANDLE hQosMngr, OS_802_11_QOS_DELETE_TSPEC_PARAMS *delAdmissionParams);

TI_STATUS qosMngr_selectActiveProtocol(TI_HANDLE  hQosMngr);

TI_STATUS qosMngr_setAcPsDeliveryMode(TI_HANDLE  hQosMngr);

TI_STATUS qosMngr_sendUnexpectedTSPECResponseEvent(TI_HANDLE    hQosMngr,
                                   tspecInfo_t  *pTspecInfo);

TI_STATUS qosMngr_setAdmissionInfo(TI_HANDLE    hQosMngr, 
                                   TI_UINT8     acID,
                                   tspecInfo_t  *pTspecInfo,
                                   trafficAdmRequestStatus_e trafficAdmRequestStatus);



TI_STATUS QosMngr_receiveActionFrames(TI_HANDLE hQosMngr, TI_UINT8* pData, TI_UINT8 action, TI_UINT32 bodyLen);

void qosMngr_checkTspecRenegResults(TI_HANDLE hQosMngr, assocRsp_t *assocRsp);

TI_UINT32 qosMngr_buildTSPec(TI_HANDLE hQosMngr, TI_UINT32 user_priority, TI_UINT8 *pQosIe);

TI_STATUS qosMngr_GetWmeEnableFlag(TI_HANDLE hQosMngr, TI_BOOL *bWmeEnable);
void qosMngr_UpdatePsTraffic   (TI_HANDLE hQosMngr,TI_BOOL bPsTrafficOn );

#endif /* __QOS_MNGR_API_H__ */

