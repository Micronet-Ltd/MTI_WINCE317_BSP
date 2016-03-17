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
/*    MODULE:   measurementMgrApi.h                                        */
/*    PURPOSE:  Measurement Manager module interface header file           */
/*                                                                         */
/***************************************************************************/





#ifndef __MEASUREMENTMGR_API_H__
#define __MEASUREMENTMGR_API_H__


#include "scrApi.h"
#include "mlmeApi.h"
#include "DrvMainModules.h"


typedef enum
{
    MEASUREMENT_TRAFFIC_THRESHOLD_PARAM = 0x01,
    MEASUREMENT_GET_STATUS_PARAM        = 0x02

} EMeasurementParam;


TI_HANDLE measurementMgr_create(TI_HANDLE hOs);

void      measurementMgr_init (TStadHandlesList *pStadHandles);

TI_STATUS measurementMgr_SetDefaults (TI_HANDLE hMeasurementMgr, measurementInitParams_t * pMeasurementInitParams);

TI_STATUS measurementMgr_destroy(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_setParam(TI_HANDLE hMeasurementMgr, paramInfo_t * pParam);

TI_STATUS measurementMgr_getParam(TI_HANDLE hMeasurementMgr, paramInfo_t * pParam);

TI_STATUS measurementMgr_connected(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_disconnected(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_enable(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_disable(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_setMeasurementMode(TI_HANDLE hMeasurementMgr, TI_UINT16 capabilities, 
                                            TI_UINT8 * pIeBuffer, TI_UINT16 length);

TI_STATUS measurementMgr_receiveFrameRequest(TI_HANDLE hMeasurementMgr, EMeasurementFrameType frameType,
                                            TI_INT32 dataLen, TI_UINT8 * pData);

void measurementMgr_rejectPendingRequests(TI_HANDLE hMeasurementMgr, EMeasurementRejectReason rejectReason);

void measurementMgr_MeasurementCompleteCB(TI_HANDLE clientObj, TMeasurementReply * msrReply);

void measurementMgr_scrResponseCB(TI_HANDLE hClient, EScrClientRequestStatus requestStatus,
                                  EScrResourceId eResource, EScePendReason pendReason);

void measurementMgr_mlmeResultCB(TI_HANDLE hMeasurementMgr, TMacAddr * bssid, mlmeFrameInfo_t * frameInfo, 
                                 TRxAttr * pRxAttr, TI_UINT8 * buffer, TI_UINT16 bufferLength);


#endif  /* __MEASUREMENTMGR_API_H__ */
