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
/** \file spectrumMngmntMgr.h
 *  \brief dot11h spectrum Management Meneger module interface header file
 *
 *  \see spectrumMngmntMgr.c
 */

/***************************************************************************/
/*																			*/
/*	  MODULE:	spectrumMngmntMgr.h											*/
/*    PURPOSE:	dot11h spectrum Management Meneger module interface         */
/*              header file                                        			*/
/*																			*/
/***************************************************************************/
#ifndef __SPECTRUMMNGMNTMGR_H__
#define __SPECTRUMMNGMNTMGR_H__

#include "paramOut.h"
#include "measurementMgr.h"
#include "requestHandler.h"



TI_STATUS measurementMgr_receiveQuietIE(TI_HANDLE hMeasurementMgr,
								  TI_UINT8		quietCount,
								  TI_UINT8		quietPeriod,
								  TI_UINT16	quietDuration,
								  TI_UINT16	quietOffset);


TI_STATUS measurementMgr_receiveTPCRequest(TI_HANDLE hMeasurementMgr,
										TI_UINT8		dataLen,
										TI_UINT8		*pData);

TI_STATUS measurementMgr_dot11hParseFrameReq(TI_HANDLE hMeasurementMgr, 
                                           TI_UINT8 *pData, TI_INT32 dataLen,
                                           TMeasurementFrameRequest *frameReq);

TI_STATUS measurementMgr_dot11hParseRequestIEHdr(TI_UINT8 *pData, TI_UINT16 *reqestHdrLen,
                                               TI_UINT16 *measurementToken);

TI_BOOL      measurementMgr_dot11hIsTypeValid(TI_HANDLE hMeasurementMgr, 
                                              EMeasurementType type, 
                                              EMeasurementScanMode scanMode);

TI_STATUS measurementMgr_dot11hBuildReport(TI_HANDLE hMeasurementMgr, MeasurementRequest_t request, TMeasurementTypeReply * reply);

TI_STATUS measurementMgr_dot11hSendReportAndCleanObject(TI_HANDLE hMeasurementMgr);

TI_STATUS measurementMgr_dot11hBuildRejectReport(TI_HANDLE hMeasurementMgr,
											 MeasurementRequest_t *pRequestArr[],
											 TI_UINT8		numOfRequestsInParallel,
											 EMeasurementRejectReason	rejectReason);


/* The following function uses features from the old Measurement module. */
/* It will have to be adapted to using the new Measurement Manager. */
#if 0

TI_STATUS measurementMgr_getBasicMeasurementParam(TI_HANDLE hMeasurementMgr,
										  acxStatisitcs_t*	pAcxStatisitics,
										  mediumOccupancy_t* pMediumOccupancy);
#endif /* 0 */



#endif /* __SPECTRUMMNGMNTMGR_H__ */
