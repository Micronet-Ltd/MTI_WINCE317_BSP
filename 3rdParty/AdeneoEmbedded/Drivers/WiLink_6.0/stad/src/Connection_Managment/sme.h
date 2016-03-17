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
/** \file  sme.h
 *  \brief SME API declarations
 *
 *  \see   sme.c, smeSm.c, smePrivate.h
 */


#ifndef __SME_H__
#define __SME_H__

#include "tidef.h"
#include "DrvMainModules.h"
#include "ScanCncn.h"
#include "paramOut.h"

#define SME_SCAN_TABLE_ENTRIES 32

typedef enum
{
    SSID_TYPE_ANY = 0,
    SSID_TYPE_SPECIFIC,
    SSID_TYPE_INVALID
} ESsidType;

typedef enum
{
    SME_RESTART_REASON_EXTERNAL_PARAM_CHANGED = 0,
    SME_RESTART_REASON_WSC_PB_MODE
} ESmeRestartReason;

typedef void (*TSmeSmCbFunc)(TI_HANDLE hCbHndl); 

TI_HANDLE   sme_Create (TI_HANDLE hOS);
void        sme_Init (TStadHandlesList *pStadHandles);
void        sme_SetDefaults (TI_HANDLE hSme, TSmeModifiedInitParams *pModifiedInitParams, TSmeInitParams *pInitParams);
void        sme_Destroy (TI_HANDLE hSme);
void        sme_Start (TI_HANDLE hSme);
void        sme_Stop (TI_HANDLE hSme);
void        sme_Restart (TI_HANDLE hSme);
void        sme_ScanResultCB (TI_HANDLE hSme, EScanCncnResultStatus eStatus,
                              TScanFrameInfo* pFrameInfo, TI_UINT16 uSPSStatus);
void        sme_MeansurementScanResult (TI_HANDLE hSme, EScanCncnResultStatus eStatus, TScanFrameInfo* pFrameInfo);
void        sme_ReportConnStatus (TI_HANDLE hSme, mgmtStatus_e eStatusType, TI_UINT32 uStatusCode);
void        sme_ReportApConnStatus (TI_HANDLE hSme, mgmtStatus_e eStatusType, TI_UINT32 uStatusCode);

void        sme_ConnectScanReport (TI_HANDLE hSme, TI_HANDLE *hScanResultTable);
void        sme_MeasureScanReport (TI_HANDLE hSme, TI_HANDLE *hScanResultTable);


#endif /* __SME_H__ */

