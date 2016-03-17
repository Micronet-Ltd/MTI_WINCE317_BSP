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
#include <windows.h>
#ifdef WM_ZOOM2
#include <windev.h>
#endif
#include "ipc_sta.h"

#include "TWDriver.h"
#include "convert.h"

#include "privcmd.h"


THandle CuOs_Create(THandle hIpcSta)
{
	return (THandle) 1;
}


VOID CuOs_Destroy(THandle hCuWext)
{
}


S32 CuOs_Get_SSID(THandle hCuWext, OS_802_11_SSID* ssid)
{
	U32 returned_bytes, rc;

	rc = IpcSta_Send_Oid(OID_802_11_SSID, (PVOID)ssid, sizeof(OS_802_11_SSID), (PVOID)ssid, 
					sizeof(OS_802_11_SSID), &returned_bytes);

	if(rc != ERROR_SUCCESS) {
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}
		
    return OK;
}


S32 CuOs_Get_BSSID(THandle hCuOs, TMacAddr bssid)
{
	U32 returned_bytes, rc;

	rc = IpcSta_Send_Oid(OID_802_11_BSSID, (PVOID)bssid, 0, (PVOID)bssid, 
					sizeof(TMacAddr), &returned_bytes);

	if(rc != ERROR_SUCCESS) {
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

    return OK;

}


S32 CuOs_GetCurrentChannel(THandle hCuOs, U32* channel)
{
	U32 rc;
	U8 temp_chan = 0;

	rc = IPC_STA_Private_Send(NULL, TIWLN_802_11_CHANNEL_GET, NULL, 0, 
						 &temp_chan, sizeof(U8));

	if(rc != ERROR_SUCCESS)
		*channel = 0;
	else
	*channel = (U32)temp_chan;

	return OK;
}

S32 CuOs_GetDriverThreadId(THandle hCuOs, U32* threadid)
{
	U32 rc;

	rc = IPC_STA_Private_Send(NULL, DRIVER_THREADID_PARAM, NULL, 0, threadid, sizeof(U32));

	if(rc != ERROR_SUCCESS)
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;

	return OK;
}

S32 CuOs_Start_Scan(THandle hCuOs)
{
	U32 returned_bytes, rc;

	rc = IpcSta_Send_Oid(OID_802_11_BSSID_LIST_SCAN, NULL, 0, NULL, 
					0, &returned_bytes);

	if(rc != ERROR_SUCCESS) {
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	return OK;
}


S32 CuOs_GetBssidList(THandle hCuWext, OS_802_11_BSSID_LIST_EX *bssidList)
{
	U32 returned_bytes, rc;

	rc = IpcSta_Send_Oid(OID_802_11_BSSID_LIST, (PVOID)bssidList, 10000, (PVOID)bssidList, 
					10000, &returned_bytes);

	if(rc != ERROR_SUCCESS) {
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	return OK;
}


S32 CuOs_Set_BSSID(THandle hCuWext, TMacAddr bssid)
{
	U32  rc;

	rc = IPC_STA_Private_Send(NULL, OID_802_11_BSSID, (PVOID)bssid,
							  sizeof(TMacAddr), NULL, 0);

	if(rc != ERROR_SUCCESS) {
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	return OK;
}


S32 CuOs_Set_ESSID(THandle hCuWext, OS_802_11_SSID* ssid)
{
	U32 rc;


	rc = IPC_STA_Private_Send(NULL, OID_802_11_SSID, (PVOID)ssid,
							  sizeof(OS_802_11_SSID), NULL, 0);


	return OK;
}


S32 CuOs_GetTxPowerLevel(THandle hCuWext, S32* pTxPowerLevel)
{
	U32  rc;
	S8 pl;

	rc = IPC_STA_Private_Send(NULL, TIWLN_802_11_TX_POWER_DBM_GET, (PVOID)NULL, 
					  0, (PVOID)&pl, sizeof(S8));

	if(rc != ERROR_SUCCESS)
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;

	*pTxPowerLevel = (S32)pl;

	return OK;
}


S32 CuOs_SetTxPowerLevel(THandle hCuWext, S32 txPowerLevel)
{
	U32 rc;
	S8 pl;

	pl = (S8)txPowerLevel;

	rc = IPC_STA_Private_Send(NULL, TIWLN_802_11_TX_POWER_DBM_GET, (PVOID)&pl, 
							  sizeof(S8), (PVOID)NULL, 0);

	if(rc != ERROR_SUCCESS){
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	return OK;
}


S32 CuOs_GetRtsTh(THandle hCuWext, PS32 pRtsTh)
{
	U32 rc;
	U16 rts;

	*pRtsTh = 0;

	rc = IPC_STA_Private_Send(NULL, TWD_RTS_THRESHOLD_PARAM, (PVOID)NULL, 
							  0, (PVOID)&rts, sizeof(U16));

	if(rc != ERROR_SUCCESS){
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	*pRtsTh = (S32)rts;

    return OK;
}


S32 CuOs_SetRtsTh(THandle hCuWext, S32 RtsTh)
{
	U32 rc;
	U16 rts = (U16)RtsTh;

	rc = IPC_STA_Private_Send(NULL, TWD_RTS_THRESHOLD_PARAM, (PVOID)&rts, 
							  sizeof(U16), (PVOID)NULL, 0);

	if(rc != ERROR_SUCCESS){
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

    return OK;
}



S32 CuOs_GetFragTh(THandle hCuWext, PS32 pFragTh)
{
	U32 rc;
	U16 frag;

	*pFragTh = 0;

	rc = IPC_STA_Private_Send(NULL, TWD_FRAG_THRESHOLD_PARAM, (PVOID)NULL, 
							  0, (PVOID)&frag, sizeof(U16));

	if(rc != ERROR_SUCCESS){
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

	*pFragTh = (S32)frag;

    return OK;
}

S32 CuOs_SetFragTh(THandle hCuWext, S32 FragTh)
{
	U32 rc;
	U16 frag = (U16)FragTh;

	rc = IPC_STA_Private_Send(NULL, TWD_FRAG_THRESHOLD_PARAM, (PVOID)&frag, 
							  sizeof(U16), (PVOID)NULL, 0);

	if(rc != ERROR_SUCCESS){
		os_error_printf(CU_MSG_ERROR, "%s failed. rc = %d(0x%X)\n", __FUNCTION__, rc, rc);
		return EOALERR_IPC_STA_ERROR_SENDING_WEXT;
	}

    return OK;
}


S32 CuOs_RedirectDrvOutputToTicon(THandle hCuOs)
{
	U32 val, rc;

	rc = IPC_STA_Private_Send(NULL, TIWLN_REDIR_TO_TICON, &val, 
						  sizeof(U32), (PVOID)NULL, 0);

	return rc;
}

