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


/** \file   StaCap.h 
 *  \brief  StaCap module header file.                                  
 *
 *  \see    StaCap.c
 */

#ifndef _STA_CAP_H_
#define _STA_CAP_H_

#include "TWDriver.h"
#include "DrvMainModules.h"

/* The main StaCap module structure */
typedef struct 
{
	TI_HANDLE   hOs;
	TI_HANDLE   hReport;
    TI_HANDLE   hTWD;
    TI_HANDLE   hQosMngr;
    TI_HANDLE   hSme;

} TStaCap;	

/* The Supported MCS Set field structure acording 802.11n SPEC*/
typedef struct 
{
    TI_UINT8    aRxMscBitmask[RX_TX_MCS_BITMASK_SIZE];
	TI_UINT16   uHighestSupportedDataRate;
	TI_UINT8    uTxRxSetting;
	TI_UINT8    aReserved[3];
} TStaCapSuppMcsSet;	

/* The HT Capabilities element structure acording 802.11n SPEC*/
typedef struct 
{
    TI_UINT16	        uHtCapabilitiesInfo;
    TI_UINT8	        uAMpduParam;
    TStaCapSuppMcsSet   tSuppMcsSet;
    TI_UINT16	        uExteCapabilities;
    TI_UINT32	        uTxBfCapabilities;
    TI_UINT8	        uAselCapabilities;
} TStaCapHtCapabilities ;	


/* External Functions Prototypes */
/* ============================= */
TI_HANDLE StaCap_Create (TI_HANDLE hOs);
TI_STATUS StaCap_Destroy (TI_HANDLE hStaCap);
TI_STATUS StaCap_Init (TStadHandlesList *pStadHandles);
TI_STATUS StaCap_GetHtCapabilitiesIe (TI_HANDLE hStaCap, TI_UINT8 *pRequest, TI_UINT32 *pLen);
void StaCap_IsHtEnable (TI_HANDLE hStaCap, TI_BOOL *b11nEnable);
#endif  /* _STA_CAP_H_ */


