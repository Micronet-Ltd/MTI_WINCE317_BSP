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

/** \file  osrecv.c 
 *  \brief functions that handle the packet receiving from the os
 *
 *  \see   
 */

#include <ndis.h>
#include "WlanDrvIf.h"
#include "RxBuf.h"
#include "tidef.h"
#ifdef GEM_SUPPORTED
#include "gem_api.h"
#endif


/* 
 * Stop queueing and forward the packets if current packet is shorter than the following length.
 * This prevents short TCP packets with the PUSH flag set from being delayed 
 * (such delay may result in the loss of this packet as seen in Chariot TCP Rx tests) 
 */
#define BURST_PKT_LEN_THRESHOLD     1400


/** 
 * \fn     recvProcessPacketFilter
 * \brief  
 * 
 * \param  pAdapter - handle to the driver adpter
 * \param  pData - pointer to the data buffer
 * \return TI_BOOL
 */
TI_BOOL recvProcessPacketFilter(TWlanDrvIfObjPtr pAdapter, TI_UINT8* pData)
{
	TI_UINT32 i;

	if(MAC_EQUAL(pAdapter->CurrentAddr, (TI_UINT8*)&pData[6])) {
		PRINTF(DBG_RECV_VERY_LOUD, ("TIWLN: PacketFilter. Block. SrcAddr=ours\n"
			"   SrcAddr = %02X-%02X-%02X-%02X-%02X-%02X\n"
			"   DstAddr = %02X-%02X-%02X-%02X-%02X-%02X\n", 
			pData[6], pData[7], pData[8], pData[9], pData[10], pData[11],
			pData[0], pData[1], pData[2], pData[3], pData[4], pData[5])
			);
		return TI_FALSE;
	}

    if(pAdapter->PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS)
		return TI_TRUE;

	if((pData[0] != 0xff)) 
	{
		if(pData[0] & 0x01) 
		{
			if(pAdapter->PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST)
				return TI_TRUE;

			if(pAdapter->PacketFilter & NDIS_PACKET_TYPE_MULTICAST) 
			{
				for(i=0; i<pAdapter->NumMulticastAddresses; i++) 
				{
					if(MAC_EQUAL(pData,&pAdapter->MulticastTable[i][0]))
					{
						return TI_TRUE;
					}
				}
			}
			/*
				PRINTF(DBG_RECV_LOUD, ("TIWLN: PacketFilter. Block. 1\n"
				"   PacketFilter = %X\n"
				"   SrcAddr = %02X-%02X-%02X-%02X-%02X-%02X\n"
				"   DstAddr = %02X-%02X-%02X-%02X-%02X-%02X\n",
				pAdapter->PacketFilter,
				pData[6], pData[7], pData[8], pData[9], pData[10], pData[11],
				pData[0], pData[1], pData[2], pData[3], pData[4], pData[5])
				);
			*/
			return TI_FALSE;
		/* if(pData[0] & 0x01) */
		} 
		if(pAdapter->PacketFilter & NDIS_PACKET_TYPE_DIRECTED) 
		{
			if(MAC_EQUAL(pData, pAdapter->CurrentAddr))
			{
				return TI_TRUE;
			}
		}
	}
	else if(pAdapter->PacketFilter & NDIS_PACKET_TYPE_BROADCAST)
    {
        return TI_TRUE;
    }
	/*
		PRINTF(DBG_RECV_LOUD, ("TIWLN: PacketFilter. Block. 2\n"
		"   PacketFilter = %X\n"
		"   SrcAddr = %02X-%02X-%02X-%02X-%02X-%02X\n"
		"   DstAddr = %02X-%02X-%02X-%02X-%02X-%02X\n", 
		pAdapter->PacketFilter,
		pData[6], pData[7], pData[8], pData[9], pData[10], pData[11],
		pData[0], pData[1], pData[2], pData[3], pData[4], pData[5])
		);
	*/

	return TI_FALSE;

} /* recvProcessPacketFilter() */

void MiniportReturnPacket(NDIS_HANDLE  MiniportAdapterContext, PNDIS_PACKET Packet)
{
	TWlanDrvIfObjPtr    pAdapter = (TWlanDrvIfObjPtr) MiniportAdapterContext;
	void                *pBuffer;
	PNDIS_BUFFER        pCurBuffer, pTempBuffer;
	TI_UINT32           Length;

	pBuffer = (void *) *(TI_UINT32*)&Packet->MiniportReserved;
	NdisQueryPacket(Packet, NULL, NULL, &pCurBuffer, &Length);
	while(pCurBuffer) 
    {
		pTempBuffer = pCurBuffer;
		NdisGetNextBuffer(pCurBuffer, &pCurBuffer);
		NdisFreeBuffer(pTempBuffer);
	}
	NdisFreePacket(Packet);
    RxBufFree(pAdapter, pBuffer);
	PRINT(DBG_RECV_LOUD, "TIWL: recvReturnPacket Out\n");

} /* MiniportReturnPacket() */

/** 
 * \fn     recvNewPacket
 * \brief  
 * 
 * \param  pAdapter - handle to the driver adpter
 * \param  Msdu - pointer to the data buffer
 * \param  Length - packet length
 * \return NDIS_STATUS
 */
NDIS_STATUS recvNewPacket(TWlanDrvIfObjPtr pAdapter, RxIfDescriptor_t* pRxDesc, void *pWlanBuffer, USHORT Length)
{
	OS_PACKET_8021Q_INFO    NdisPacket8021qInfo;
	PNDIS_PACKET            pPacket;
	PNDIS_BUFFER            pBuffer;
	NDIS_STATUS             Status;

	PRINTF(DBG_RECV_LOUD, ("TIWL: Received packet. Length - %d\n", Length));
	NdisAllocatePacket(&Status, &pPacket, pAdapter->PacketPoolHandle);
	if(Status != NDIS_STATUS_SUCCESS) 
    {
		PRINTF(DBG_RECV_ERROR, ("  Could not allocate NDIS packet. Status-%X\n",Status));
        /* free Buffer */
        RxBufFree(pAdapter, pWlanBuffer); 
		return Status;
	}
	NDIS_SET_PACKET_HEADER_SIZE(pPacket, 14);
    NdisAllocateBuffer(&Status, &pBuffer, pAdapter->BufferPoolHandle, (TI_UINT8 *)RX_ETH_PKT_DATA(pWlanBuffer), RX_ETH_PKT_LEN(pWlanBuffer));
    if(Status != NDIS_STATUS_SUCCESS)
    {
        PRINT(DBG_RECV_ERROR, "  Could not allocate NDIS buffer\n");
        /* free Buffer */
        RxBufFree(pAdapter, pWlanBuffer); 
        NdisFreePacket(pPacket);
        return Status;
    }
    NdisChainBufferAtBack(pPacket, pBuffer);
	*(TI_UINT32*)&pPacket->MiniportReserved = (TI_UINT32)pWlanBuffer;
	NDIS_SET_PACKET_STATUS(pPacket, NDIS_STATUS_SUCCESS);

    pAdapter->aNdisPackets[pAdapter->numOfNdisPackets++] = pPacket;

	if(recvProcessPacketFilter(pAdapter, RX_ETH_PKT_DATA(pWlanBuffer))) 
    {
		NdisPacket8021qInfo.u.TagHeader.UserPriority = 0;
		NdisPacket8021qInfo.u.TagHeader.CanonicalFormatId = 0;
		NdisPacket8021qInfo.u.TagHeader.Reserved = 0;        
		NdisPacket8021qInfo.u.TagHeader.VlanId = 0;  
		NDIS_PER_PACKET_INFO_FROM_PACKET(pPacket, Ieee8021pPriority) = NdisPacket8021qInfo.u.Value;
		if(NdisPacket8021qInfo.u.TagHeader.UserPriority != 0)
        {
			PRINTF(DBG_RECV_LOUD,("**************** Recev NdisPacket8021qInfo.u.TagHeader.UserPriority=%x*****************\n",NdisPacket8021qInfo.u.TagHeader.UserPriority));
        }

#ifdef GEM_SUPPORTED
		GEM_PacketIndicateHandler( pAdapter->MiniportHandle,
									&pPacket,
									1 );
#endif

        /* 
         * If current burst ended, forward the buffered Rx packets to the network stack.
         *
         * Note: Forward also if last packet is shorter than regular traffic, to prevent 
         *       short TCP packets with the PUSH flag set from being delayed (such delay may 
         *       result in the loss of this packet as seen in Chariot TCP Rx tests) 
         */
        if ((pRxDesc->driverFlags & DRV_RX_FLAG_END_OF_BURST) || 
            (pAdapter->numOfNdisPackets == MAX_NDIS_PACKETS)  || 
            (Length < BURST_PKT_LEN_THRESHOLD))
        {
            NdisMIndicateReceivePacket(pAdapter->MiniportHandle,pAdapter->aNdisPackets, pAdapter->numOfNdisPackets);
            pAdapter->numOfNdisPackets = 0;
        }
	}
	else
    {
        pAdapter->numOfNdisPackets--;
        MiniportReturnPacket(pAdapter, pAdapter->aNdisPackets[pAdapter->numOfNdisPackets]);        
	}

	return NDIS_STATUS_SUCCESS;

 } /* recvNewPacket() */



