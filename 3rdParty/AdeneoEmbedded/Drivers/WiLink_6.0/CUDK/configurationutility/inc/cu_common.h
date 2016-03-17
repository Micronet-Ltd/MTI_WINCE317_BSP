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

/****************************************************************************/
/*                                                                          */
/*    MODULE:   CuCommon.h                                                  */
/*    PURPOSE:                                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _CU_COMMON_H_
#define _CU_COMMON_H_

/* defines */
/***********/

/* types */
/*********/
typedef struct 
{
    U32 temp; /* the place of the IE's id and length */
    U32 unmatchedPacketsCount;                  /* number of packets didn't match any filter (when the feature was enabled). */
    U32 matchedPacketsCount[MAX_DATA_FILTERS];  /* number of packets matching each of the filters */
} TCuCommon_RxDataFilteringStatistics;


/* functions */
/*************/
THandle CuCommon_Create(THandle *pIpcSta, const PS8 device_name);
VOID CuCommon_Destroy(THandle hCuCommon);

S32 CuCommon_SetU32(THandle hCuCommon, U32 PrivateIoctlId, U32 Data);
S32 CuCommon_GetU32(THandle hCuCommon, U32 PrivateIoctlId, PU32 pData);
S32 CuCommon_SetU16(THandle hCuCommon, U32 PrivateIoctlId, U16 Data);
S32 CuCommon_SetU8(THandle hCuCommon, U32 PrivateIoctlId, U8 Data);
S32 CuCommon_GetU8(THandle hCuCommon, U32 PrivateIoctlId, PU8 pData);
S32 CuCommon_SetBuffer(THandle hCuCommon, U32 PrivateIoctlId, PVOID pBuffer, U32 len);
S32 CuCommon_GetBuffer(THandle hCuCommon, U32 PrivateIoctlId, PVOID pBuffer, U32 len);
S32 CuCommon_GetSetBuffer(THandle hCuCommon, U32 PrivateIoctlId, PVOID pBuffer, U32 len);

S32 CuCommon_Start_Scan(THandle hCuCommon, PVOID scanParams, U32 sizeOfScanParams);
S32 CuCommon_Get_BssidList_Size(THandle hCuCommon, PU32 pSizeOfBssiList);
S32 CuCommon_GetRssi(THandle hCuCommon, PS8 pdRssi, PS8 pbRssi);
S32 CuCommon_GetSnr(THandle hCuCommon, PU32 pdSnr, PU32 pbSnr);
S32 CuCommon_GetdesiredPreambleType(THandle hCuCommon, PU32 pDesiredPreambleType);
S32 CuCommon_GetTxStatistics(THandle hCuCommon, TIWLN_TX_STATISTICS* pTxCounters, U32 doReset); 

S32 CuCommon_Radio_Test(THandle hCuCommon,TTestCmd* data);

S32 CuCommon_AddKey(THandle hCuCommon, OS_802_11_WEP* pKey);
S32 CuCommon_RemoveKey(THandle hCuCommon, U32 KeyIndex);
S32 CuCommon_GetDfsChannels(THandle hCuCommon, PU16 pMinDfsChannel, PU16 pMaxDfsChannel);
S32 CuCommon_SetDfsChannels(THandle hCuCommon, U16 MinDfsChannel, U16 MaxDfsChannel);

S32 CuCommon_PrintDriverDebug(THandle hCuCommon, PVOID pParams, U32 param_size);
S32 CuCommon_PrintDriverDebugBuffer(THandle hCuCommon, U32 func_id, U32 opt_param);

S32 CuCommon_GetRxDataFiltersStatistics(THandle hCuCommon, PU32 pUnmatchedPacketsCount, PU32 pMatchedPacketsCount);
S32 CuCommon_GetPowerConsumptionStat(THandle hCuCommon, ACXPowerConsumptionTimeStat_t *pPowerstat);


#endif  /* _CU_COMMON_H_ */
        
