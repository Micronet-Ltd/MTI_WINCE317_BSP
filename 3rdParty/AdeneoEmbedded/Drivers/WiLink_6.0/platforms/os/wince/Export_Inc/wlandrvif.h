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

/*
 * inc/WlanDrvIf.h
 *
 */

#ifndef WLAN_DRV_IF_H
#define WLAN_DRV_IF_H

#include "osDot11.h"
#include "WlanDrvCommon.h"
#include "osTIType.h"
#include "osApi.h"
#include "osDebug.h"
#include "paramOut.h"
#include "IPCKernel.h"
#include <ndis.h>

#define MAX_MULTICAST_ADDRESSES				32
#define ETH_ADDR_SIZE						6
#define MAX_NDIS_PACKETS                    32


/*
 * TCmdRespUnion is defined for each OS:
 * For Linx and WM that defined is empty.
 * For OSE the new typedef includes all "Done" typedefs in union from EMP code (H files).
 */
typedef struct
{
    TI_UINT32 uDummy;
} TCmdRespUnion;


typedef struct 
{
    TWlanDrvIfCommon        tCommon;        /* The driver object common part */
    NDIS_HANDLE             MiniportHandle;                                         /* NDIS MiniPort handle          */
    NDIS_MINIPORT_INTERRUPT Interrupt;                                              /* NDIS Interrupt                */
    NDIS_HANDLE             ConfigHandle;                                           /* NDIS configuration handle     */
	NDIS_HANDLE		        FirmwareImageHandle;                                    /* Firmware file handle          */
	NDIS_HANDLE		        EepromImageHandle;                                      /* NVS      file handle          */
	NDIS_HANDLE		        PacketPoolHandle;                                       /* NDIS packet pool              */
	NDIS_HANDLE		        BufferPoolHandle;                                       /* NDIS buffer pool              */
    IPC_KERNEL_STRUCT_T     IPC;                                                    /* IPC handle                    */
    HANDLE                  drvThread;                                              /* Driver context task           */
    HANDLE                  drvEvent;                                               /* Driver context task event     */
    BOOL                    bRunDrv;                                                /* Driver context task run flag  */
    TI_UINT32               InitTime;                                               /* OID_GEN_INIT_TIME_MS          */
	DWORD			        powerModeState;                                         /* OID_PNP_SET/QUERY_POWER       */
	TI_UINT8		        MulticastTable[MAX_MULTICAST_ADDRESSES][ETH_ADDR_SIZE]; /* OID_802_3_MULTICAST_LIST      */
	TI_UINT32		        NumMulticastAddresses;                                  /* OID_802_3_MULTICAST_LIST      */
	TI_UINT8		        PermanentAddr[ETH_ADDR_SIZE];                           /* OID_802_3_PERMANENT_ADDRESS   */
    TI_UINT8                CurrentAddr[ETH_ADDR_SIZE];                             /* OID_802_3_CURRENT_ADDRESS     */
    OS_802_11_WEP           DefaultWepKeys[DOT11_MAX_DEFAULT_WEP_KEYS];             /* OID_802_11_RELOAD_DEFAULTS    */
	TI_UINT32               LinkSpeed;                                              /* OID_GEN_LINK_SPEED            */
	TI_UINT32               LinkStatus;                                             /* OID_GEN_MEDIA_CONNECT_STATUS  */
    TI_UINT32               etherMaxPayloadSize;                                    /* OID_GEN_MAXIMUM_LOOKAHEAD     */
	TI_UINT32               PacketFilter;                                           /* OID_GEN_CURRENT_PACKET_FILTER */
	void*					pTxSignalObject;
    PNDIS_PACKET            aNdisPackets[MAX_NDIS_PACKETS];
    TI_UINT8                numOfNdisPackets; /* Holds the actual number of packets to be sent to NS */

} TWlanDrvIfObj, *TWlanDrvIfObjPtr;

TI_STATUS wlanDrvIf_Create      (TWlanDrvIfObjPtr pAdapter);
VOID      wlanDrvIf_Destroy     (TWlanDrvIfObjPtr pAdapter);
TI_STATUS wlanDrvIf_Start       (TWlanDrvIfObjPtr pAdapter);
VOID      wlanDrvIf_Stop        (TWlanDrvIfObjPtr pAdapter);
DWORD     wlanDrvIf_DriverTask  (LPVOID lpParam);

#endif /* WLAN_DRV_IF_H */

