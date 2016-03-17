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
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
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

#ifndef __WIFI_H
#define __WIFI_H

#include "tidef.h"
#include "TWDriverScan.h"
#include "AS_Global.h"
#include "AS_Channel.h"
#include "Conf_Global.h"
#include "DTI_Channel.h"
#include "DTI_Modes.h"
#include "roamingMngrTypes.h"

// ------ Global Wifi Defines -------
#define NB_ETEVENTS		4
#define NB_PRRATES		14
#define NB_SCANTYPES	6

//#define NB_BANDINDEX	3	-- Dual Mode Not supported in current version
#define NB_BANDINDEX	2

// --- End of Global Wifi Defines ---


typedef struct {
	WCHAR*	EvtName;
	UINT	EvtIndex;
} EvtDesc_t;
typedef EvtDesc_t ETEventNamesList[NB_ETEVENTS];
static const ETEventNamesList ETEventNames = 
{
	{L"ET Disabled",SCAN_ET_COND_DISABLE},
	{L"ET on beacons only",SCAN_ET_COND_BEACON},
	{L"ET on PR only",SCAN_ET_COND_PROBE_RESP},
	{L"ET on both",SCAN_ET_COND_ANY_FRAME}
};

typedef struct {
	WCHAR*	PRRateName;
	UINT	PRRateIndex;
} PRRateDesc_t;
typedef PRRateDesc_t PRRateDescList[NB_PRRATES];
static const PRRateDescList PRRateDesc = 
{
	{L"Auto",DRV_RATE_MASK_AUTO},
	{L"1 Mbps",DRV_RATE_MASK_1_BARKER},
	{L"2 Mbps",DRV_RATE_MASK_2_BARKER},
	{L"5.5 Mbps",DRV_RATE_MASK_5_5_CCK},
	{L"11 Mbps",DRV_RATE_MASK_11_CCK},
	{L"22 Mbps",DRV_RATE_MASK_22_PBCC},
	{L"6 Mbps",DRV_RATE_MASK_6_OFDM},
	{L"9 Mbps",DRV_RATE_MASK_9_OFDM},
	{L"12 Mbps",DRV_RATE_MASK_12_OFDM},
	{L"18 Mbps",DRV_RATE_MASK_18_OFDM},
	{L"24 Mbps",DRV_RATE_MASK_24_OFDM},
	{L"36 Mbps",DRV_RATE_MASK_36_OFDM},
	{L"48 Mbps",DRV_RATE_MASK_48_OFDM},
	{L"54 Mbps",DRV_RATE_MASK_54_OFDM}
};

typedef WCHAR* ScanTypeDesc_t;
typedef ScanTypeDesc_t ScanTypeDescList[NB_SCANTYPES];
static const ScanTypeDescList ScanTypeDesc =
{L"0",L"1",L"2",L"3",L"4",L"5"};


typedef WCHAR* BandIndex_t;
typedef BandIndex_t BandIndexList_t[NB_BANDINDEX];
static const BandIndexList_t BandIndexList =
{
	L"2.4 GHz",
	L"5 GHz",
//	L"Dual" : Dual mode not supported in current version
};


BOOL WL_Init(void);
BOOL WL_GetStatus(void);
BOOL WL_SaveStartupState(int);
BOOL WL_TurnOn(void);
BOOL WL_TurnOff(void);
BOOL WL_DeInit(void);
BOOL WL_BIPCalibration (BOOL bSB1_14, 
						BOOL bSB1_4,
						BOOL bSB8_16,
						BOOL bSB34_48,
						BOOL bSB52_64,
						BOOL bSB100_116,
						BOOL bSB120_140,
						BOOL bSB149_165);
BOOL WL_GetMACAddress (LPTSTR szMACAddress, DWORD dwSize);
BOOL WL_SetMACAddress (LPCTSTR szMACAddress);
BOOL WL_GetRoamingStatus(BOOL &bRoamingEnable);
BOOL WL_RoamingDisable ();
BOOL WL_RoamingEnable ();
BOOL WL_SetScanAppDiscParameters(TScanAppData* pScanAppData);
BOOL WL_SetScanAppTrackParameters(TScanAppData* pScanAppData);
BOOL WL_SetScanAppImmScanParameters(TScanAppData* pScanAppData);

BOOL WL_GetScanAppDiscParameters(UINT bandIndex,TScanAppData* pScanAppData);
BOOL WL_GetScanAppTrackParameters(UINT bandIndex,TScanAppData* pScanAppData);
BOOL WL_GetScanAppImmScanParameters(UINT bandIndex,TScanAppData* pScanAppData);
VOID WL_StartScan(void);
VOID WL_StopScan(void);

BOOL WL_SetASChannelParameters(const AS_Channel_t* pAsChannelData);
BOOL WL_GetASChannelParameters(TI_UINT8 ChannelIndex, AS_Channel_t* pAsChannelData);
VOID WL_GetGlobalParameters(Conf_Global_t* pGlobalData);
VOID WL_SetGlobalParameters(const Conf_Global_t* pGlobalData);
VOID WL_GetDTIChannelParameters(TI_UINT8 bandNumber,DTI_Channel_t* pDtiChannelData);
BOOL WL_SetDTIChannelParameters(const DTI_Channel_t* pDtiChannelData);
VOID WL_GetASGlobalParameters(AS_Global_t* pASGlobalData);
BOOL WL_SetASGlobalParameters(const AS_Global_t* pAsGlobalData);

BOOL WL_GetRoamingParameters(roamingMngrConfigParams_t &roamingMngrConfigParams);
VOID WL_SetRoamingParameters(const roamingMngrConfigParams_t roamingMngrConfigParams);
#endif