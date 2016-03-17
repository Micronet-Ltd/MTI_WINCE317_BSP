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
/** \file  ScanCncnPrivate.h
 *  \brief Scan concentartor module private definitions
 *
 *  \see   ScanCncn.c, ScanCncn.h, ScanCncnApp.c
 */

#ifndef __SCAN_CNCN_PRIVATE_H__
#define __SCAN_CNCN_PRIVATE_H__

#include "osTIType.h"
#include "TWDriver.h"
#include "scrApi.h"
#include "ScanCncnSm.h"

/** \enum EConnectionStatus
 * \brief enumerates the different connection statuses
 */
typedef enum
{
    STA_CONNECTED = 0,              /**< the station is connected to an infrastructure BSS */
    STA_NOT_CONNECTED,              /**< the station is not connected to an infrastructure BSS */
    STA_IBSS                        /**< the station is participating in an IBSS */
} EConnectionStatus;

typedef struct
{
    /* handles to other modules */
    TI_HANDLE               hOS;
    TI_HANDLE               hTWD;
    TI_HANDLE               hReport;
    TI_HANDLE               hRegulatoryDomain;
    TI_HANDLE               hSiteManager;
    TI_HANDLE               hSCR;
    TI_HANDLE               hAPConn;
    TI_HANDLE               hEvHandler;
    TI_HANDLE               hMlme;
    TI_HANDLE               hHealthMonitor;
    TI_HANDLE               hSme;

    /* client specific information */
    TScanCncnClient         *pScanClients[ SCAN_SCC_NUM_OF_CLIENTS ];

    /* SG Flags */
    TI_BOOL                 bUseSGParams;
    TI_UINT32               uSGcompensationPercent;
    TI_UINT32               uSGcompensationMaxTime;
    TI_UINT8                uSGprobeRequestPercent;   
        
    /* connection status */
    EConnectionStatus       eConnectionStatus;
    TScanCncnInitParams     tInitParams;

    /* scan concentrator application sub-module data */
    TI_HANDLE               hScanResultTable; /* application scan result table */
    TI_HANDLE               hOSScanSm; /* OS scan state machine */
    EScanCncnClient         eCurrentRunningAppScanClient; /* to disallow both one-shot and periodic app */
    TI_UINT32               uOSScanLastTimeStamp;
    TI_BOOL                 bOSScanRunning;
    TScanParams             tOsScanParams;

} TScanCncn;

#endif /* __SCAN_CNCN_PRIVATE_H__ */

