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
/** \file  smePrivate.h
 *  \brief SME private declaratnions
 *
 *  \see   sme.c, sme.h, smeSm.c
 */


#ifndef __SME_PRIVATE_H__
#define __SME_PRIVATE_H__

#include "tidef.h"
#include "TWDriver.h"
#include "sme.h"
#include "scanResultTable.h"

typedef struct
{
    mgmtStatus_e    eMgmtStatus;     /* Contains the last DisAssociation reason towards upper layer                  */
    TI_UINT32       uStatusCode;     /* Extra information to the reason. i.e. packet status code or roaming trigger  */
} TDisAssocReason;

typedef struct
{
    /* Handles to other modules */
    TI_HANDLE       hOS;
    TI_HANDLE       hReport;
    TI_HANDLE       hScanCncn;
    TI_HANDLE       hApConn;
    TI_HANDLE       hConn;
    TI_HANDLE       hScr;
    TI_HANDLE       hRegDomain;
    TI_HANDLE       hEvHandler;
    TI_HANDLE       hSiteMgr;
    TI_HANDLE       hRsn;
    TI_HANDLE       hScanResultTable;          /* Working table - points to one of the next two tables */
    TI_HANDLE       hSmeScanResultTable;       /* Sme local table */
    TI_HANDLE       hScanCncnScanResulTable;   /* Scan Cncn table - table used by the application */
    TI_HANDLE       hSmeSm;
    TI_HANDLE       hDrvMain;
    TI_HANDLE       hTwd;

    /* parameters */
    TI_BOOL         bRadioOn;
    EConnectMode    eConnectMode;
    ScanBssType_e   eBssType;
    TMacAddr        tBssid;
    ESsidType       eSsidType;
    TSsid           tSsid;

    /* internal state-machine variables */
    TI_BOOL         bConnectRequired;
    TI_BOOL         bRunning;
    EConnectMode    eLastConnectMode;
    TI_BOOL         bAuthSent;
    TI_UINT32       uScanCount;
    TI_BOOL         bReselect;      /* for SG Avalanche */

    TI_BOOL         bConstantScan;  /* scan constantly, for WSC PB mode */
    TSiteEntry      *pCandidate;
	TSiteEntry      tCandidate;     /* used to store the selected entry of sme_Select*/
    TDisAssocReason tDisAssoc;

    TSmeInitParams  tInitParams;
    TPeriodicScanParams	tScanParams; /* temporary storage for scan command */

} TSme;

TSiteEntry *sme_Select (TI_HANDLE hSme);


#endif /* __SME_PRIVATE_H__ */
