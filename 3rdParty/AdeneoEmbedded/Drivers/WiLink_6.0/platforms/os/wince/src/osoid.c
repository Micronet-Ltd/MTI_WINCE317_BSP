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

/** \file  osoid.c 
 *  \brief handling the os oid's
 *
 *  \see   
 */

#include <ndis.h>
#include <ntddndis.h>

#include "osTIType.h"
#include "wlandrvif.h"
#include "CmdHndlr.h"
#include "cmdinterpretoid.h"
#ifdef TI_DBG
#include "tracebuf_api.h"
#endif
#ifdef GEM_SUPPORTED
#include "gem_api.h"
#endif


static TI_STATUS InsertCommand (TWlanDrvIfObj *pAdapter,
                                NDIS_OID       Oid,
                                TI_UINT32      uFlags,
                                void          *pInfoBuffer, 
                                TI_UINT32      uInfoBufferLength,
                                TI_UINT32     *pActualLen,
                                TI_UINT32     *pNeededLen);


/** 
 * \fn     MiniportQueryInformation
 * \brief  process a Query request for NDIS, OIDs that are specific about the Driver.
 * 
 * \param  MiniportAdapterContext - a pointer to the adapter
 * \param  Oid - the NDIS OID to process.
 * \param  InformationBuffer -  a pointer into the NdisRequest->InformationBuffer into which store the result of the query.
 * \param  InformationBufferLength - a pointer to the number of InfoBufferLength left in the InformationBuffer.
 * \param  InfoBufferLengthWritten - a pointer to the number of InfoBufferLength written into the InformationBuffer.
 * \param  InfoBufferLengthNeeded - If there is not enough room in the information buffer then this will contain the number of InfoBufferLength needed to complete the request.
 * \return NDIS_STATUS - status of the operation.
 */

NDIS_STATUS 
MiniportQueryInformation(
	IN NDIS_HANDLE  MiniportAdapterContext, 
	IN NDIS_OID     Oid,
	IN PVOID        pInfoBuffer, 
	IN ULONG        InfoBufferLength,
	OUT PULONG      pcbWritten, 
	OUT PULONG      pcbNeeded
	)
{

    ti_private_cmd_t* pcmd;
    NDIS_OID pOid = Oid;
    BOOLEAN ProprietarySet = FALSE;
    TI_STATUS      status;
    TWlanDrvIfObj *pAdapter = (TWlanDrvIfObj *)MiniportAdapterContext;
    TI_UINT32      uCommand;

#ifdef GEM_SUPPORTED
	status = GEM_QueryInformationHandler_Pre( MiniportAdapterContext,
											   Oid,
											   pInfoBuffer,
											   InfoBufferLength,
											   pcbWritten,
											   pcbNeeded );
	if ( status != NDIS_STATUS_NOT_RECOGNIZED ) {
			return status;
	}
#endif

	RETAILMSG(0, (TEXT("WIFI_W2: MiniportQInf+ \r\n")));

    if (Oid == OID_TI_WILINK_IOCTL)
    {
    	RETAILMSG(0, (TEXT("WIFI_W2: MinipQInf OID_TI_WILINK_IOCTL \r\n")));
        uCommand = *((TI_UINT32 *)pInfoBuffer);

        if (IS_PARAM_FOR_MODULE(uCommand, DRIVER_MODULE_PARAM))
        {
            switch (uCommand)
            {
            case DRIVER_START_PARAM:
				RETAILMSG(1, (TEXT("WIFI_W2: MinipQInf DRIVER_START_PARAM \r\n")));
                wlanDrvIf_Start (pAdapter);
                return NDIS_STATUS_SUCCESS;
    
            case DRIVER_STOP_PARAM:
				RETAILMSG(1, (TEXT("WIFI_W2: MinipQInf DRIVER_STOP_PARAM \r\n")));
                wlanDrvIf_Stop (pAdapter);
                return NDIS_STATUS_SUCCESS;

            case DRIVER_STATUS_PARAM:
				RETAILMSG(1, (TEXT("WIFI_W2: MinipQInf DRIVER_STATUS_PARAM \r\n")));
                *(TI_UINT32 *)pInfoBuffer = 
                    (pAdapter->tCommon.eDriverState == DRV_STATE_RUNNING) ? TI_TRUE : TI_FALSE;
                *pcbWritten = sizeof(TI_UINT32);
                return NDIS_STATUS_SUCCESS;

            case DRIVER_THREADID_PARAM:
				RETAILMSG(1, (TEXT("WIFI_W2: MinipQInf DRIVER_THREADID_PARAM \r\n")));
                *(TI_UINT32 *)pInfoBuffer = (TI_UINT32)pAdapter->drvThread;
                *pcbWritten = sizeof(TI_UINT32);
                return NDIS_STATUS_SUCCESS;
            }
        }
    }

	RETAILMSG(0, (TEXT("WIFI_W2: MinipQInf Oid 0x%08x \r\n"), Oid));

    switch (Oid)
    {		
    case OID_GEN_LINK_SPEED:		

        if (InfoBufferLength >= sizeof(TI_UINT32))
        {
            if (!pAdapter->LinkSpeed)
            {
                *(TI_INT32*)pInfoBuffer = LINK_SPEED;
            }
            else
            {
                *(TI_UINT32*)pInfoBuffer = pAdapter->LinkSpeed;
            }
            status      = NDIS_STATUS_SUCCESS;
            *pcbWritten = sizeof(TI_UINT32);
            PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: OID_GEN_LINK_SPEED %d\n", *(TI_UINT32 *)pInfoBuffer));
        }
        else
        {
            status      = NDIS_STATUS_INVALID_LENGTH;
            *pcbWritten = 0;
            *pcbNeeded  = sizeof(TI_UINT32);
            PRINTF(DBG_NDIS_OIDS_VERY_LOUD, ("cmdInterpret: Wrong buffer size in OID_GEN_LINK_SPEED\n"));
        }
		break;


    case 0xffffff:
				
        status      = NDIS_STATUS_SUCCESS;
        *pcbWritten = sizeof(TI_UINT32);
		break;

    default:

		if(Oid == OID_TI_WILINK_IOCTL) {

			pcmd = (ti_private_cmd_t*) pInfoBuffer;

			if(pcmd->in_buffer_len) {

				switch(pcmd->cmd) {

				case OID_802_11_SSID:
				case OID_802_11_BSSID:
					RETAILMSG(1, (TEXT("WIFI_W2: MinipQInf OID_802_11_SSID OID_802_11_BSSID  \r\n")));
					pOid = pcmd->cmd;
					ProprietarySet = TRUE;
					break;

				}

			}
		}

		if(pAdapter->tCommon.eDriverState != DRV_STATE_RUNNING) //== DRV_STATE_FAILED)
 		    return NDIS_STATUS_ADAPTER_NOT_READY;
		/* If the driver is in stat of stoped, return NDIS_STATUS_ADAPTER_NOT_READY */
		if (pAdapter->powerModeState == NdisDeviceStateD3)
		{
 		    return NDIS_STATUS_ADAPTER_NOT_READY;
		}

		if(!ProprietarySet) 
        {
			RETAILMSG(0, (TEXT("WIFI_W2: MinipQInf !ProprietarySet 0x%08x \r\n"), Oid));
            /* Insert the quary command to the WLAN driver */
            status = InsertCommand (pAdapter, 
                                    Oid, 
                                    OS_OID_GET_FLAG, 
                                    pInfoBuffer, 
                                    InfoBufferLength, 
                                    pcbWritten, 
                                    pcbNeeded);
		}
		else 
        {
			RETAILMSG(0, (TEXT("WIFI_W2: MinipQInf ProprietarySet 0x%08x \r\n"), Oid));
            /* Insert proprietary set command to the WLAN driver */
            status = InsertCommand (pAdapter, 
                                    pOid, 
                                    OS_OID_SET_FLAG, 
                                    (PUCHAR)pInfoBuffer + sizeof(ti_private_cmd_t), 
                                    pcmd->in_buffer_len, 
                                    pcbWritten, 
                                    pcbNeeded);
		}

        break;
    }

#ifdef GEM_SUPPORTED
	(VOID)GEM_QueryInformationHandler_Post( MiniportAdapterContext,
											 Oid,
											 pInfoBuffer,
											 InfoBufferLength,
											 pcbWritten,
											 pcbNeeded );
#endif

	RETAILMSG(0, (TEXT("WIFI_W2: MinipQInf- Status %d \r\n"), status));
    /* tb_sprintf("QueryI Oid=0x%x buf=0x%x exit\n",Oid,pInfoBuffer); */
    return (NDIS_STATUS)status;

} /* MiniportQueryInformation() */


/** 
 * \fn     MiniportSetInformation
 * \brief  process a set request from NDIS, OIDs that are specific about the Driver.
 * 
 * \param  MiniportAdapterHandle - a handle to the adapter
 * \param  Oid - the NDIS OID to process.
 * \param  pInfoBuffer -  a pointer into the NdisRequest->InformationBuffer from which we take the value to set.
 * \param  cbInfoBuffer - 
 * \param  pcbRead - 
 * \param  pcbNeeded - 
 * \return NDIS_STATUS - status of the operation.
 */
NDIS_STATUS
MiniportSetInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID     Oid,
    IN PVOID        pInfoBuffer, 
    IN ULONG        InfoBufferLength,
    OUT PULONG      pcbRead,
    OUT PULONG      pcbNeeded
                      ) 
{
    TI_STATUS   status;
    TI_UINT32   uCommand;
    TWlanDrvIfObj *pAdapter = (TWlanDrvIfObj *)MiniportAdapterContext;

	RETAILMSG(0, (TEXT("WIFI_W2: MiniportSetInf+ \r\n")));

#ifdef GEM_SUPPORTED
	if ( GEM_SetInformationHandler( MiniportAdapterContext,
									 Oid,
									 pInfoBuffer,
									 InfoBufferLength,
									 pcbRead,
									 pcbNeeded ) != NDIS_STATUS_SUCCESS ) {
			/* discard information setting */
			return NDIS_STATUS_SUCCESS;
	}
#endif

    switch (Oid)
    {

    case OID_PNP_SET_POWER:

		RETAILMSG(1, (TEXT("WIFI_W2: MiniportSetInf OID_PNP_SET_POWER \r\n")));

        uCommand = *((TI_UINT32 *)pInfoBuffer);
        WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER %d\r\n", uCommand)); 

        switch (uCommand) 
        {
            case NdisDeviceStateD0:
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET POWER_MODE_ACTIVE (NdisDeviceStateD0)\n"));
                if (pAdapter->powerModeState!=NdisDeviceStateD0)
                {
                    //pAdapter->powerModeState = NdisDeviceStateD0;
					WLAN_OS_REPORT(("TIWL: NdisDeviceStateD0 Start\r\n")); 
                    if(0 == wlanDrvIf_Start(pAdapter))
					{
						pAdapter->powerModeState = NdisDeviceStateD0;
						status = NDIS_STATUS_SUCCESS;
					}
					else
 						status = NDIS_STATUS_FAILURE;
					WLAN_OS_REPORT(("TIWL: NdisDeviceStateD0 %d\r\n", status)); 
                }
                WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER adapter started \r\n"));    
                break;

            case NdisDeviceStateD1:
                WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER NdisDeviceStateD1 \n"));    
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET OID_PNP_SET_POWER (NdisDeviceStateD1)\n"));
                status = NDIS_STATUS_NOT_SUPPORTED;
                break;

            case NdisDeviceStateD2:
                WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER NdisDeviceStateD2 \n"));     
                PRINTF(DBG_NDIS_OIDS_LOUD, ("SET OID_PNP_SET_POWER (NdisDeviceStateD2)\n"));
                status = NDIS_STATUS_NOT_SUPPORTED;
                break;

            case NdisDeviceStateD3:
                WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER NdisDeviceStateD3 \r\n"));    
                if(pAdapter->powerModeState!=NdisDeviceStateD3)
                {
                    pAdapter->powerModeState = NdisDeviceStateD3;
                    wlanDrvIf_Stop(pAdapter);
                    status = NDIS_STATUS_SUCCESS;
                }
				else
						status = NDIS_STATUS_FAILURE;
                WLAN_OS_REPORT(("TIWL: (SET)OID_PNP_SET_POWER adapter stopped \r\n"));    
                break;

            default:
                PRINTF(DBG_NDIS_OIDS_LOUD, ("POWER_MODE unknown\n"));
                break;
        }
        PRINTF(DBG_NDIS_OIDS_LOUD, ("TIWL: (SET) OID_PNP_SET_POWER % d\n", uCommand));

        break;

    default:

		RETAILMSG(0, (TEXT("WIFI_W2: MiniportSetInf OID 0x%08x \r\n"), Oid));

		if(pAdapter->tCommon.eDriverState != DRV_STATE_RUNNING) //== DRV_STATE_FAILED)
 		    return NDIS_STATUS_ADAPTER_NOT_READY;
        /* If the driver is in stat of stoped, return NDIS_STATUS_ADAPTER_NOT_READY */
		if (pAdapter->powerModeState == NdisDeviceStateD3)
		{
 		    return NDIS_STATUS_ADAPTER_NOT_READY;
		}

		RETAILMSG(0, (TEXT("WIFI_W2: MiniportSetInf Set OID 0x%08x \r\n"), Oid));

        /* Insert the set command to the WLAN driver */
        status = InsertCommand (pAdapter, 
                                Oid, 
                                OS_OID_SET_FLAG, 
                                pInfoBuffer, 
                                InfoBufferLength, 
                                pcbRead, 
                                pcbNeeded);

        break;
    }

	RETAILMSG(0, (TEXT("WIFI_W2: MiniportSetInf- Status %d\r\n"), status));
    /* tb_sprintf("SetI Oid=0x%x buf=0x%x EXIT\n",Oid,pInfoBuffer); */
    return (NDIS_STATUS)status;

} /* MiniportSetInformation() */


/** 
 * \fn     InsertCommand
 * \brief  Forward the command to the WLAN driver in a local buffer
 * 
 * \param  pAdapter          - A handle to the adapter
 * \param  Oid               - The NDIS OID to process.
 * \param  uFlags            - Indicate if Set or Get operation.
 * \param  pInfoBuffer       - A pointer to the buffer that contains the command info.
 * \param  uInfoBufferLength - The length of the command data pointed by pInfoBuffer
 * \param  pActualLen        - The actual length of data copied back by the buffer
 * \param  pNeededLen        - The required buffer length in case it was too short
 * 
 * \return NDIS_STATUS       - status of the operation.
 */
static TI_STATUS InsertCommand (TWlanDrvIfObj *pAdapter,
                                NDIS_OID       Oid,
                                TI_UINT32      uFlags,
                                void          *pInfoBuffer, 
                                TI_UINT32      uInfoBufferLength,
                                TI_UINT32     *pActualLen,
                                TI_UINT32     *pNeededLen)
{
    TI_STATUS  eStatus;
    void      *pLocalBuffer;
    TI_UINT32  uActualLen = 0;
    TI_UINT32  uNeededLen = 0;

	RETAILMSG(0, (TEXT("WIFI_W2: InsertCommand+ \r\n")));

    /* Allocate a local buffer to avoid access to user memory from driver context */
    pLocalBuffer = os_memoryAlloc (pAdapter, uInfoBufferLength);
    if (pLocalBuffer == NULL)
    {
        WLAN_OS_REPORT(("TIWL: InsertCommand() failed to allocate local buffer!!  OID = 0x%x, Flags = 0x%x, Length = %d\n", Oid, uFlags, uInfoBufferLength));    
        return TI_NOK;
    }

    /* Copy user input buffer to local buffer */
    os_memoryCopyFromUser(pAdapter, pLocalBuffer, pInfoBuffer, uInfoBufferLength);

	RETAILMSG(0, (TEXT("WIFI_W2: InsertCommand Oid 0x%08x \r\n"), Oid));

    /* Insert the command to the WLAN driver */
    eStatus = cmdHndlr_InsertCommand (pAdapter->tCommon.hCmdHndlr, 
                                      Oid, 
                                      uFlags, 
                                      pLocalBuffer, 
                                      uInfoBufferLength, 
                                      NULL, 
                                      0, 
                                      &uActualLen, 
                                      &uNeededLen);

    /* Now we have driver data ready in the local buffer so copy to user buffer */
    os_memoryCopyToUser (pAdapter, pInfoBuffer, pLocalBuffer, uInfoBufferLength);

    /* If pointer is not NULL, update the read data length in case length was enough */
    if (pActualLen) 
    {
        os_memoryCopyToUser (pAdapter, pActualLen, &uActualLen, sizeof(TI_UINT32));
    }

    /* If pointer is not NULL, update the needed data length in case length wasn't enough */
    if (pNeededLen)
    {
        os_memoryCopyToUser (pAdapter, pNeededLen, &uNeededLen, sizeof(TI_UINT32));
    }

    /* Free the local buffer */
    os_memoryFree (pAdapter, pLocalBuffer, uInfoBufferLength);

	RETAILMSG(0, (TEXT("WIFI_W2: InsertCommand Status %d \r\n"), eStatus));

    return eStatus;
}
