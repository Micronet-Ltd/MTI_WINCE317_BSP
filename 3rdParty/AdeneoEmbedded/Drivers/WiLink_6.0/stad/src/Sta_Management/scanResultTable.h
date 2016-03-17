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
/** \file  scanResultTable.h
 *  \brief declarations for a table holding scan results, by BSSID
 *
 *  \see   scanResultTable.c
 */


#ifndef __SCAN_RESULT_TABLE_H__
#define __SCAN_RESULT_TABLE_H__

#include "osTIType.h"
#include "tidef.h"
#include "ScanCncn.h"
#include "DrvMainModules.h"


/* site types */
typedef enum
{
    SITE_PRIMARY        = 0,
    SITE_SELF           = 1,
    SITE_REGULAR        = 2,
    SITE_NULL           = 3
} siteType_e;

typedef struct
{
    /* The following fields are used for entry management at the SiteMng */
    TI_UINT8                   index;
    siteType_e                 siteType;
    TI_UINT32                  localTimeStamp;
    /* end of fields  are used for entry management at the SiteMng */

    TI_BOOL                    bConsideredForSelect;
    ERadioBand                 eBand;   
    TI_UINT8                   tsfTimeStamp[ TIME_STAMP_LEN ];

    /* The following fields are used for the selection */
    TI_BOOL                    probeRecv;
    TI_BOOL                    beaconRecv;
    TMacAddr                   bssid;
    TSsid                      ssid;
    ScanBssType_e              bssType;
    rateMask_t                 rateMask;
    ERate                      maxBasicRate;
    ERate                      maxActiveRate;
    EModulationType            beaconModulation;
    EModulationType            probeModulation;
    EPreamble                  currentPreambleType;
    EPreamble                  preambleAssRspCap;
    EPreamble                  barkerPreambleType;
    ESlotTime                  currentSlotTime;
    ESlotTime                  newSlotTime;
    TI_BOOL                    useProtection;
    TI_BOOL                    NonErpPresent;
    TI_UINT8                   channel;
    TI_BOOL                    privacy;
    TI_BOOL                    agility;
    TI_UINT16                  capabilities;
    TI_UINT16                  beaconInterval;
    TI_UINT8                   dtimPeriod;
    TI_INT8                    snr;
    ERate                      rxRate;
    TI_INT32                   rssi;

    /* HT capabilites */
	Tdot11HtCapabilitiesUnparse tHtCapabilities;
	/* HT information */
    TI_BOOL                     bHtInfoUpdate;
	Tdot11HtInformationUnparse  tHtInformation;

    /* QOS */
    TI_BOOL                    WMESupported;
    dot11_ACParameters_t       WMEParameters;
    TI_UINT8                   lastWMEParameterCnt;

    /* Power Constraint */
    TI_UINT8                   powerConstraint;

    /* AP Tx Power obtained from TPC Report */
    TI_UINT8                   APTxPower;

    /* UPSD */
    TI_BOOL                    APSDSupport;

    /* WiFi Simple Config */
    TIWLN_SIMPLE_CONFIG_MODE   WSCSiteMode; /* indicates the current WiFi Simple Config mode of the specific site*/

    TI_UINT16                  atimWindow;
    dot11_RSN_t                pRsnIe[MAX_RSN_IE];
    TI_UINT8                   rsnIeLen;

    /* 80211h beacon  - Switch Channel IE included */
    TI_BOOL                    bChannelSwitchAnnoncIEFound;

	TI_UINT8                   pUnknownIe[MAX_BEACON_BODY_LENGTH];
    TI_UINT16                  unknownIeLen;

    mgmtStatus_e               failStatus;
    TI_BOOL                    prioritySite;
    TI_UINT8                   probeRespBuffer[ MAX_BEACON_BODY_LENGTH ];
    TI_UINT16                  probeRespLength;
    TI_UINT8                   beaconBuffer[ MAX_BEACON_BODY_LENGTH ];
    TI_UINT16                  beaconLength;

} TSiteEntry;


typedef enum 
{
    SCAN_RESULT_TABLE_DONT_CLEAR,
    SCAN_RESULT_TABLE_CLEAR

} EScanResultTableClear;


TI_HANDLE   scanResultTable_Create (TI_HANDLE hOS, TI_UINT32 uEntriesNumber);
void        scanResultTable_Init (TI_HANDLE hScanResultTable, TStadHandlesList *pStadHandles, EScanResultTableClear eClearTable);
void        scanResultTable_Destroy (TI_HANDLE hScanResultTable);
TI_STATUS   scanResultTable_UpdateEntry (TI_HANDLE hScanResultTable, TMacAddr *pBssid, TScanFrameInfo* pFrame);
void        scanResultTable_SetStableState (TI_HANDLE hScanResultTable);
TSiteEntry  *scanResultTable_GetFirst (TI_HANDLE hScanResultTable);
TSiteEntry  *scanResultTable_GetNext (TI_HANDLE hScanResultTable);
TSiteEntry  *scanResultTable_GetBySsidBssidPair (TI_HANDLE hScanResultTable, TSsid *pSsid, TMacAddr *pBssid);
TI_UINT32   scanResultTable_CalculateBssidListSize (TI_HANDLE hScanResultTable, TI_BOOL bAllVarIes);
TI_UINT32 scanResultTable_GetNumOfBSSIDInTheList (TI_HANDLE hScanResultTable);
TI_STATUS   scanResultTable_GetBssidList (TI_HANDLE hScanResultTable, OS_802_11_BSSID_LIST_EX *pBssidList, 
                                          TI_UINT32 *pLength, TI_BOOL bAllVarIes);
TI_STATUS scanResultTable_GetBssidSupportedRatesList (TI_HANDLE hScanResultTable, OS_802_11_N_RATES *pRateList, TI_UINT32 *pLength);

void        scanResultTable_PerformAging(TI_HANDLE hScanResultTable);
void        scanResultTable_SetSraThreshold(TI_HANDLE hScanResultTable, TI_UINT32 uSraThreshold);

#endif /* __SCAN_RESULT_TABLE_H__ */

