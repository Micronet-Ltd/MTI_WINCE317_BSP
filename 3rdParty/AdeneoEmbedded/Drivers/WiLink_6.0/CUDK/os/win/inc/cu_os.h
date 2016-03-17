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
/*    MODULE:   CuWext.h                                                    */
/*    PURPOSE:                                                              */
/*                                                                          */
/****************************************************************************/
#ifndef _CU_WEXT_H_
#define _CU_WEXT_H_

/* defines */
/***********/
#define MAX_SSID_LEN 32

/* types */
/*********/

/* functions */
/*************/
THandle CuOs_Create(THandle hIpcSta);
VOID CuOs_Destroy(THandle hCuWext);

S32 CuOs_Get_SSID(THandle hCuWext, OS_802_11_SSID* ssid);
S32 CuOs_Get_BSSID(THandle hCuWext, TMacAddr bssid);
S32 CuOs_GetCurrentChannel(THandle hCuWext, U32* channel);
S32 CuOs_GetDriverThreadId(THandle hCuWext, U32* threadid);
S32 CuOs_Start_Scan(THandle hCuWext);
S32 CuOs_GetBssidList(THandle hCuWext, OS_802_11_BSSID_LIST_EX *bssidList);
S32 CuOs_Set_BSSID(THandle hCuWext, TMacAddr bssid);
S32 CuOs_Set_ESSID(THandle hCuWext, OS_802_11_SSID* ssid);
S32 CuOs_GetTxPowerLevel(THandle hCuWext, S32* pTxPowerLevel);
S32 CuOs_SetTxPowerLevel(THandle hCuWext, S32 txPowerLevel);
S32 CuOs_GetRtsTh(THandle hCuWext, PS32 pRtsTh);
S32 CuOs_SetRtsTh(THandle hCuWext, S32 RtsTh);
S32 CuOs_GetFragTh(THandle hCuWext, PS32 pFragTh);
S32 CuOs_SetFragTh(THandle hCuWext, S32 FragTh);
S32 CuOs_RedirectDrvOutputToTicon(THandle hCuOs);

#endif  /* _CU_WEXT_H_ */
        
