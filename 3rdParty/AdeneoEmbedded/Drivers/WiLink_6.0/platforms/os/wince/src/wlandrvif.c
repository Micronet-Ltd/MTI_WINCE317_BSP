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

/** \file   WlanDrvIf.c 
 *  \brief  The OS-Dependent interfaces of the WLAN driver with external applications:
 *          - Configuration utilities (including download, configuration and activation)
 *          - Network Stack (Tx and Rx)
 *          - Interrupts
 *          - Events to external applications
 *  
 *  \see    WlanDrvIf.h, Wext.c
 */

#define __FILE_ID__  FILE_ID_138

#include <ndis.h>
#include "WlanDrvIf.h"
#include "DrvMain.h"
#include "osApi.h"
#include "osRgstry.h"
#include "context.h"
#include "TWDriver.h"
#include "Ethernet.h"
#include "DataCtrl_Api.h"
#ifdef TI_DBG
#include "tracebuf_api.h"
#endif
#include "bmtrace_api.h"


#ifdef FPGA1273_STAGE_
	#define FW_FILE_NAME  "Fw1273_FPGA.bin"
#else
	#define FW_FILE_NAME  "Fw1273_CHIP.bin"
#endif

#define NVS_FILE_NAME "nvs_map.bin"

#define PKT_BUFFERS_NUM_LIMIT   10000   /* Treat a packet with more than 10 buffers as invalid */
#define TX_RAW_BUF_HEADROOM     2    /* Copy the packet in 2 bytes offset to have the IP header in aligned address */

static NDIS_PHYSICAL_ADDRESS High = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);

/** 
 * \fn     wlanDrvIf_UpdateDriverState
 * \brief  Update the driver state
 * 
 * The DrvMain uses this function to update the OAL with the driver steady state
 *     that is relevant for the driver users.
 * 
 * \note   
 * \param  hOs          - The driver object handle
 * \param  eDriverState - The new driver state
 * \return void
 * \sa     
 */ 
void wlanDrvIf_UpdateDriverState (TI_HANDLE hOs, EDriverSteadyState eDriverState)
{
    TWlanDrvIfObj *drv = (TWlanDrvIfObj *)hOs;

    PRINT1(DBG_INIT_VERY_LOUD, "wlanDrvIf_UpdateDriverState(): State = %d\n", eDriverState);

    /* Save the new state */
    drv->tCommon.eDriverState = eDriverState;

} /* wlanDrvIf_UpdateDriverState() */

/*--------------------------------------------------------------------------------------*/

TI_STATUS wlanDrvIf_loadFile(NDIS_STRING *pFileName, NDIS_HANDLE *pFileHandle, TI_UINT8 **pBuffer, TI_UINT32 *Length)
{
    NDIS_STATUS status;

    os_printf("wlanDrvIf_loadFile: name=%s, handle=0x%x, buffer=0x%x, len=%d\n", pFileName->Buffer,  *pFileHandle, pBuffer, Length);
    if (*pFileHandle)
    {
        /* file already opened and mapped */
        return TI_OK;
    }

    NdisOpenFile(&status, pFileHandle, Length, pFileName, High);
    if (status != NDIS_STATUS_SUCCESS) 
    {
        os_printf("TIWL: Could not open image file, status=0x%x\n", status );
        return TI_NOK;
    }
    NdisMapFile(&status, (PVOID*)pBuffer, *pFileHandle);
    if(status != NDIS_STATUS_SUCCESS)
    {
        os_printf("TIWL: Could not map image file, status=0x%x\n", status );
        NdisCloseFile (*pFileHandle);
        return TI_NOK;
    }

    return TI_OK;

} /* wlanDrvIf_loadFile() */

/*--------------------------------------------------------------------------------------*/

/** 
 * \fn     wlanDrvIf_GetFile
 * \brief  Provides access to a requested init file
 * 
 * Provide the requested file information and call the requester callback.
 * Note that in Linux the files were previously loaded to driver memory 
 *     by the loader (see wlanDrvIf_LoadFiles).
 *
 * \note   
 * \param  hOs       - The driver object handle
 * \param  pFileInfo - The requested file's properties
 * \return TI_OK
 * \sa     wlanDrvIf_LoadFiles
 */ 
TI_STATUS wlanDrvIf_GetFile (TI_HANDLE hOs, TFileInfo *pFileInfo)
{
    TWlanDrvIfObjPtr drv = (TWlanDrvIfObjPtr)hOs;
    NDIS_STRING      FileName;
    TI_STATUS        Status = TI_OK;

    /* Future option for getting the FW image part by part */ 
    pFileInfo->hOsFileDesc = NULL;

    //os_printf("wlanDrvIf_loadFile: type=%d\n", pFileInfo->eFileType);
    /* Fill the file's location and size in the file's info structure */
    switch (pFileInfo->eFileType) 
    {
    case FILE_TYPE_INI: 
        Status = TI_OK;
        break;
    case FILE_TYPE_NVS:     
        NdisInitializeString(&FileName,NVS_FILE_NAME);
        Status = wlanDrvIf_loadFile(&FileName,&(drv->EepromImageHandle), &(TI_UINT8 *)drv->tCommon.tNvsImage.pImage, &drv->tCommon.tNvsImage.uSize);
        pFileInfo->pBuffer = (TI_UINT8 *)drv->tCommon.tNvsImage.pImage; 
        pFileInfo->uLength = drv->tCommon.tNvsImage.uSize;
        Status = TI_OK;
        break;
    case FILE_TYPE_FW:  
        NdisInitializeString(&FileName,FW_FILE_NAME);
        Status = wlanDrvIf_loadFile(&FileName,&(drv->FirmwareImageHandle), &(TI_UINT8 *)drv->tCommon.tFwImage.pImage, &drv->tCommon.tFwImage.uSize);
        pFileInfo->pBuffer = (TI_UINT8 *)drv->tCommon.tFwImage.pImage; 
        pFileInfo->bLast = TI_FALSE;
        pFileInfo->uLength = 0;
        pFileInfo->uOffset = 0;
        pFileInfo->uChunkBytesLeft = 0;
        
        pFileInfo->uChunksLeft = BYTE_SWAP_LONG( *((TI_UINT32*)(pFileInfo->pBuffer)) );
		
    	/* check uChunksLeft's Validity */
    	if (( pFileInfo->uChunksLeft == 0 ) || ( pFileInfo->uChunksLeft > MAX_CHUNKS_IN_FILE ))
    	{
            os_printf("wlanDrvIf_GetFile: invalid chunk\n");
    		return TI_NOK;
    	}
    	pFileInfo->pBuffer += DRV_ADDRESS_SIZE;
    
    /* Fall through */
    case FILE_TYPE_FW_NEXT:    
            /* check dec. validity */
        if ( pFileInfo->uChunkBytesLeft >= pFileInfo->uLength )
    	{
    		pFileInfo->uChunkBytesLeft -= pFileInfo->uLength;
    	}
    	/* invalid Dec. */
    	else
    	{
            os_printf("wlanDrvIf_GetFile: No. of Bytes Left < File Length\n");
    		return TI_NOK;
    	}
    	pFileInfo->pBuffer += pFileInfo->uLength; 
    
    	/* Finished reading all Previous Chunk */
    	if ( pFileInfo->uChunkBytesLeft == 0 )
    	{
    		/* check dec. validity */
    		if ( pFileInfo->uChunksLeft > 0 )
    		{
    			pFileInfo->uChunksLeft--;
    		}
    		/* invalid Dec. */
    		else
    		{
                os_printf("wlanDrvIf_GetFile: o. of Bytes Left = 0 and Chunks Left <= 0\n" );
                return TI_NOK;
    		}
    		/* read Chunk's address */
    		pFileInfo->uAddress = BYTE_SWAP_LONG( *((TI_UINT32*)(pFileInfo->pBuffer)));
    		pFileInfo->pBuffer += DRV_ADDRESS_SIZE;
    		/* read Portion's length */
    		pFileInfo->uChunkBytesLeft = BYTE_SWAP_LONG( *((TI_UINT32*)(pFileInfo->pBuffer)) );
    		pFileInfo->pBuffer += DRV_ADDRESS_SIZE;
    	}
    	/* Reading Chunk is NOT complete */
    	else
    	{
    		pFileInfo->uAddress += pFileInfo->uLength;
    	}
    		
    	if ( pFileInfo->uChunkBytesLeft < OS_SPECIFIC_RAM_ALLOC_LIMIT )
    	{
    		pFileInfo->uLength = pFileInfo->uChunkBytesLeft;
    	}
    	else
    	{
    		pFileInfo->uLength = OS_SPECIFIC_RAM_ALLOC_LIMIT;
    	}
    
    	/* If last chunk to download */
    	if (( pFileInfo->uChunksLeft == 0 ) &&
			( pFileInfo->uLength == pFileInfo->uChunkBytesLeft ))
    	{
    		pFileInfo->bLast = TI_TRUE;
    	}
            /*FILE_TYPE_FW_NEXT*/
        break;

    }/* switch */

    /* Call the requester callback */
    if (pFileInfo->fCbFunc)
    {
        pFileInfo->fCbFunc (pFileInfo->hCbHndl);
    }
    //os_printf("wlanDrvIf_loadFile: done %x\n", Status);

    return Status;

} /* wlanDrvIf_GetFile() */

/*--------------------------------------------------------------------------------------*/

void wlanDrvIf_SetMacAddress (TI_HANDLE hOs, TI_UINT8 *pMacAddr)
{
    TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)hOs;

    /* Copy STA MAC address to the network interface structure */
    NdisMoveMemory(pAdapter->CurrentAddr, pMacAddr, ETH_ADDR_SIZE);
	/* LTK Fix  */
	NdisMoveMemory(pAdapter->PermanentAddr, pMacAddr, ETH_ADDR_SIZE);

} /* wlanDrvIf_SetMacAddress() */

/*--------------------------------------------------------------------------------------*/

TI_STATUS osInitTable_IniFile (TI_HANDLE hOs, TInitTable *InitTable, char *file_buf, int file_length)
{
    
    regFillInitTable(hOs, InitTable);
   
    return TI_OK;

} /* osInitTable_IniFile() */

/*--------------------------------------------------------------------------------------*/

DWORD wlanDrvIf_DriverTask(LPVOID lpParam)
{
    /*wlanDrvIf_DriverTask */
	TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)lpParam;

    SetProcPermissions(0xffffffff);

    while (TRUE)
    {
        if (WaitForSingleObject(pAdapter->drvEvent, INFINITE) != WAIT_OBJECT_0) 
        {
             PRINT(DBG_INIT_FATAL_ERROR, "drvThread() WaitForSingleObject FAILED  !!!\n");
            break;
        }
        if (!pAdapter->bRunDrv)
        {
            break;
        }
        context_DriverTask (pAdapter->tCommon.hContext);
    }

    return (0);

} /* wlanDrvIf_DriverTask() */

/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_Start
 * \brief  Start driver
 * 
 * \sa     wlanDrvIf_Stop
 */ 
/*--------------------------------------------------------------------------------------*/

TI_STATUS wlanDrvIf_Start (TWlanDrvIfObjPtr pAdapter)
{
    PRINT(DBG_INIT_LOUD, "wlanDrvIf_Start()\n");
    if (!pAdapter->tCommon.hDrvMain)
    {
        PRINT(DBG_INIT_FATAL_ERROR, "wlanDrvIf_Start() Driver not created!\n");
        return TI_NOK;
    }
    /* 
     *  Insert Start command in DrvMain action queue, request driver scheduling 
     *      and wait for action completion (all init process).
     */
    return drvMain_InsertAction (pAdapter->tCommon.hDrvMain, ACTION_TYPE_START);
} /* wlanDrvIf_Start() */

/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_Stop
 * \brief  Stop driver
 * 
 * \sa     wlanDrvIf_Start
 */ 
/*--------------------------------------------------------------------------------------*/

VOID wlanDrvIf_Stop (TWlanDrvIfObjPtr pAdapter)
{
    PRINT(DBG_INIT_LOUD, "wlanDrvIf_Stop()\n");
    /* 
     *  Insert Stop command in DrvMain action queue, request driver scheduling 
     *      and wait for Stop process completion.
     */
    drvMain_InsertAction (pAdapter->tCommon.hDrvMain, ACTION_TYPE_STOP);

} /* wlanDrvIf_Stop() */

/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_Create
 * \brief  Create the driver instance
 * 
 * Create and link all driver modules.
 *
 * \note   
 * \param  void
 * \return 0 - OK, else - failure
 * \sa     wlanDrvIf_Destroy
 */ 
/*--------------------------------------------------------------------------------------*/

TI_STATUS wlanDrvIf_Create (TWlanDrvIfObjPtr pAdapter)
{
    pAdapter->tCommon.eDriverState = DRV_STATE_IDLE;
	/*Tx Context*/
	pAdapter->pTxSignalObject = os_SignalObjectCreate (pAdapter); /* initialize "Tx Stop/Resume context" */

    if (pAdapter->pTxSignalObject == NULL)
    {
        return TI_NOK;
    }

    /* Create all driver modules and link their handles */
    return drvMain_Create (pAdapter, 
                           &pAdapter->tCommon.hDrvMain, 
                           &pAdapter->tCommon.hCmdHndlr, 
                           &pAdapter->tCommon.hContext, 
                           &pAdapter->tCommon.hTxDataQ,
                           &pAdapter->tCommon.hTxMgmtQ,
                           &pAdapter->tCommon.hTxCtrl,
                           &pAdapter->tCommon.hTWD,
                           &pAdapter->tCommon.hEvHandler,
                           &pAdapter->tCommon.hCmdDispatch,
                           &pAdapter->tCommon.hReport);

} /* wlanDrvIf_Create() */

/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_Destroy
 * \brief  Destroy the driver instance
 * 
 * Destroy all driver modules.
 *
 * \note   
 * \param  drv - The driver object handle
 * \return void
 * \sa     wlanDrvIf_Create
 */ 
/*--------------------------------------------------------------------------------------*/
VOID wlanDrvIf_Destroy (TWlanDrvIfObjPtr pAdapter)
{

    /* Free signalling object */
	os_SignalObjectFree (pAdapter, pAdapter->pTxSignalObject);
    /* Destroy all driver modules */
    if (pAdapter->tCommon.hDrvMain)
    {
        drvMain_Destroy (pAdapter->tCommon.hDrvMain);
    }

} /* wlanDrvIf_Destroy() */


/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_FreeTxPacket
 * \brief  Free the OS Tx packet
 * 
 * Free the OS Tx packet after driver processing is finished.
 *
 * \note   
 * \param  hOs          - The OAL object handle
 * \param  pPktCtrlBlk  - The packet CtrlBlk
 * \param  eStatus      - The packet transmission status (OK/NOK)
 * \return void
 * \sa     
 */ 
/*--------------------------------------------------------------------------------------*/
void wlanDrvIf_FreeTxPacket (TI_HANDLE hOs, TTxCtrlBlk *pPktCtrlBlk, TI_STATUS eStatus)
{
	TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)hOs;
    PNDIS_PACKET     pPacket  = (PNDIS_PACKET)(pPktCtrlBlk->tTxPktParams.pInputPkt);

	if (eStatus == TI_OK) 
    {
		NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_SUCCESS);
		NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_SUCCESS);
	}
    else 
    {
        /* Report NDIS_STATUS_RESOURCES when an error occured. */
		PRINTF(DBG_SEND_ERROR, ("wlanDrvIf_FreeTxPacket() sending NDIS_STATUS_RESOURCES\n"));
		NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_RESOURCES);
		NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_RESOURCES);
	}
}


/*--------------------------------------------------------------------------------------*/
/** 
 * \fn     wlanDrvIf_SendPacket
 * \brief  Send Tx packet from OS to the driver
 * 
 *
 * \note   
 * \param  
 * \return 
 * \sa     
 */ 
/*--------------------------------------------------------------------------------------*/
static TI_STATUS wlanDrvIf_SendPacket (TWlanDrvIfObjPtr pAdapter, PNDIS_PACKET pPacket, TI_UINT32 uPacketQoSTag)
{
	PNDIS_BUFFER  pFirstBuf;
	PNDIS_BUFFER  pCurBuf;
	PUCHAR        pCurBufAddr;
	ULONG         uCurBufLen;
	ULONG         uPacketLen;
	ULONG         uNumBuffers;
	ULONG         i;
	TI_STATUS     eStatus;
	TTxCtrlBlk *  pPktCtrlBlk;
    TI_BOOL       bCopyPacket = TI_FALSE;
    CL_TRACE_START_L1();

    /* Get packet and first buffer information */
    NdisQueryPacket (pPacket, NULL, &uNumBuffers, &pFirstBuf, &uPacketLen);

#ifdef TI_DBG
	if ((uPacketLen == 0) || (uNumBuffers == 0) || (pFirstBuf == NULL)) 
    {
		PRINTF(DBG_SEND_ERROR, ("wlanDrvIf_SendPacket() BAD NDIS Packet: PktLength=%d, NumBufs=%d, FirstBufAddr=0x%x\n",
                                uPacketLen, uNumBuffers, pFirstBuf));
        NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_FAILURE);
        NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_FAILURE);
        CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
		return TI_NOK;
	}
#endif

	PRINTF(DBG_SEND_INFO, ("wlanDrvIf_SendPacket: NumBufs = %d, PktLen = %d\n", uNumBuffers, uPacketLen));

	/* Allocate a TxCtrlBlk for the Tx packet and save timestamp and length */
    pPktCtrlBlk = TWD_txCtrlBlk_Alloc (pAdapter->tCommon.hTWD);

    if (pPktCtrlBlk == NULL) 
    {
		PRINTF(DBG_SEND_ERROR, ("TWD_txCtrlBlkAlloc() returned NULL\n"));
        NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_FAILURE);
        NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_FAILURE);
        CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
        return TI_NOK;
    }

    pPktCtrlBlk->tTxDescriptor.startTime = os_timeStampMs (pAdapter);
    pPktCtrlBlk->tTxDescriptor.length    = (TI_UINT16)uPacketLen;
    pPktCtrlBlk->tTxPktParams.pInputPkt  = pPacket;

    /* If buffers number is not above BDL limit */
    if (uNumBuffers <= MAX_XFER_BUFS)
    {
        /* 
         * Loop over the pakcet buffers and fill the BDL.
         * If buffers format is not supported, 
         *     indicate that we need to copy all to a new buffer and exit loop.
         */
        pCurBuf = pFirstBuf;
        for (i = 0; i < uNumBuffers; i++) 
        {
            NdisQueryBufferSafe (pCurBuf, (PVOID *)&pCurBufAddr, &uCurBufLen, NormalPagePriority);		

			/*In LTK there are cases that a zero buffer is in the middele of the buffer array so we will copy it as well*/
			if (uCurBufLen == 0)
            {
                bCopyPacket = TI_TRUE;
                break;
            }
            /* If first buffer doesn't contain the Ethernet header, exit (copy needed) */ 
            if (i == 0)
            {
                if (uCurBufLen != ETHERNET_HDR_LEN)
                {
                    bCopyPacket = TI_TRUE;
                    break;
                }
            }
            /* If second buffer and address is not 4-bytes aligned, exit (IP header not aligned, copy needed) */ 
            else if ((i == 1) && ((ULONG)pCurBufAddr & 3))
            {
                bCopyPacket = TI_TRUE;
                break;
            }

            /* Save buffer pointer and length in the BDL */
            pPktCtrlBlk->tTxnStruct.aBuf[i] = pCurBufAddr;
            pPktCtrlBlk->tTxnStruct.aLen[i] = (TI_UINT16)uCurBufLen;

            /* Get next buffer */
            NdisGetNextBuffer (pCurBuf, &pCurBuf);

            /* If no more buffers exit the loop (the BDL is ready and packet copy is NOT needed!) */
            if (!pCurBuf) 
            {
                if (i < (MAX_XFER_BUFS - 1)) 
                {
                    pPktCtrlBlk->tTxnStruct.aLen[i + 1] = 0;
                }
                break;
            }
        }
    }
    /* More buffers than supported by BDL, so copy is needed */
    else 
    {
        bCopyPacket = TI_TRUE;
    }

    /* 
     * For all cases not supported by the BDL, copy the whole packet to a new buffer 
     */
    if (bCopyPacket)
    {
        PUCHAR  pRawBuf;
        PUCHAR  pCopyDestAddr;
        CL_TRACE_START_L2();

        /* Allocate buffer with headroom for getting the IP header in a 4-byte aligned address */
        pRawBuf = txCtrl_AllocPacketBuffer (pAdapter->tCommon.hTxCtrl, pPktCtrlBlk, uPacketLen + TX_RAW_BUF_HEADROOM);

		if (pRawBuf == NULL) 
        {
            PRINTF(DBG_SEND_ERROR, ("wlanDrvIf_SendPacket(): txCtrl_AllocPacketBuffer() returned NULL\n"));
            txCtrl_FreePacket (pAdapter->tCommon.hTxCtrl, pPktCtrlBlk, TI_NOK);
            CL_TRACE_END_L2("tiwlan_drv.ko", "NDIS", "TX", "CopyPacket");
            CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
			return TI_NOK;
		}

        /* Copy the packet to the buffer (2 bytes offset to have the IP header in an aligned address) */
        pCurBuf = pFirstBuf;
        pCopyDestAddr = pRawBuf + TX_RAW_BUF_HEADROOM;  
        for (i = 0; i < PKT_BUFFERS_NUM_LIMIT; i++) 
        {
            NdisQueryBufferSafe (pCurBuf, (PVOID *)&pCurBufAddr, &uCurBufLen, NormalPagePriority);		

			if (uCurBufLen != 0) 
            {
				os_memoryCopy (pAdapter, pCopyDestAddr, pCurBufAddr, uCurBufLen);
				pCopyDestAddr += uCurBufLen;
			}

            NdisGetNextBuffer (pCurBuf, &pCurBuf);
            if (!pCurBuf) 
            {
                break;
            }
        }

#ifdef TI_DBG
        if (i == PKT_BUFFERS_NUM_LIMIT)
        {
            PRINTF(DBG_SEND_ERROR, ("wlanDrvIf_SendPacket() Too many buffers in packet!!  NumBufs=%d\n", uNumBuffers));
            txCtrl_FreePacket (pAdapter->tCommon.hTxCtrl, pPktCtrlBlk, TI_NOK);
            CL_TRACE_END_L2("tiwlan_drv.ko", "NDIS", "TX", "CopyPacket");
            CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
            return TI_NOK;
        }
#endif

        /* Point the first BDL buffer to the Ethernet header, and the second buffer to the rest of the packet */
        BUILD_TX_TWO_BUF_PKT_BDL (pPktCtrlBlk, pRawBuf + TX_RAW_BUF_HEADROOM, ETHERNET_HDR_LEN, 
                                  pRawBuf + TX_RAW_BUF_HEADROOM + ETHERNET_HDR_LEN, uPacketLen - ETHERNET_HDR_LEN)

        CL_TRACE_END_L2("tiwlan_drv.ko", "NDIS", "TX", "CopyPacket");
    }

    /* Send the packet to the driver */
	PRINTF(DBG_SEND_LOUD, ("TIWL: Sending... OsCtx:%X  Packet:%X  Length:%d\n", pAdapter, pPacket, uPacketLen));
    eStatus = txDataQ_InsertPacket (pAdapter->tCommon.hTxDataQ, pPktCtrlBlk, (UINT8)uPacketQoSTag);
	PRINTF(DBG_SEND_LOUD, ("TIWL: Send returned %X\n", eStatus));

    /* Indicate the OS the packet status */
    if (bCopyPacket) 
    {
		/* Since we've copied the packet, we can send a response immediately. */
		if (eStatus == TI_OK) 
		{
            NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_SUCCESS);
            NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_SUCCESS);
        }
        else 
        {
            NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_RESOURCES);
            NdisMSendComplete(pAdapter->MiniportHandle, pPacket, NDIS_STATUS_RESOURCES);
            CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
            return TI_NOK;
        }
    } 
    else
    {
		if (eStatus == TI_OK) 
		{
			/* Packet state is pending. When sent, the state will be updated by wlanDrvIf_FreeTxPacket(). */
	       	NDIS_SET_PACKET_STATUS (pPacket, NDIS_STATUS_PENDING);
        }
		/* For failed case, wlanDrvIf_FreeTxPacket() will have been called and already notify the status. */
	}

    CL_TRACE_END_L1("tiwlan_drv.ko", "NDIS", "TX", "");
    return TI_OK;

} /* SendPacket() */


/*-----------------------------------------------------------------------------
Routine Name:

	MiniportMultipleSend

Routine Description:
Arguments:
Return Value:	
-----------------------------------------------------------------------------*/
void MiniportMultipleSend (NDIS_HANDLE MiniportAdapterContext, PPNDIS_PACKET PacketArray, TI_UINT32 NumberOfPackets)
{
	TWlanDrvIfObjPtr        pAdapter = (TWlanDrvIfObjPtr)MiniportAdapterContext;
   	OS_PACKET_8021Q_INFO    NdisPacket8021qInfo;
	TI_UINT32               i;
	TI_STATUS               eStatus;

	PRINTF(DBG_SEND_LOUD, ("TIWL: sendMultipleSend In. Packets - %d\n", NumberOfPackets));
    for (i = 0; i < NumberOfPackets; i++) 
    {
        NdisPacket8021qInfo.u.Value = NDIS_PER_PACKET_INFO_FROM_PACKET (PacketArray[i], Ieee8021pPriority);

		eStatus = wlanDrvIf_SendPacket (pAdapter, PacketArray[i], NdisPacket8021qInfo.u.TagHeader.UserPriority);
		if (eStatus != TI_OK)
		{
			/* out of resources so abort all the following packets in this send request */
			for (i = i + 1; i < NumberOfPackets; i++)
			{
                PRINTF(DBG_SEND_ERROR, ("MiniportMultipleSend(): No resources so abort further packets sending\n"));
                NDIS_SET_PACKET_STATUS (PacketArray[i], NDIS_STATUS_RESOURCES);
				NdisMSendComplete (pAdapter->MiniportHandle, PacketArray[i], NDIS_STATUS_RESOURCES);
			}
		}
    }
	PRINT (DBG_SEND_LOUD, "TIWL: sendMultipleSend Out\n");

} /* sendMultipleSend()*/


/** 
 * \fn     wlanDrvIf_CommandDone
 * \brief  Free current command semaphore.
 * 
 * This routine is called whenever a command has finished executing and Free current command semaphore.
 *
 * \note   
 * \param  hOs           - The driver object handle
 * \param  pSignalObject - handle to complete mechanism per OS
 * \param  CmdResp_p     - respond structure (TCmdRespUnion) for OSE OS only 
 * \return 0 - OK, else - failure
 * \sa     wlanDrvIf_Destroy
 */ 
void wlanDrvIf_CommandDone (TI_HANDLE hOs, void *pSignalObject, TI_UINT8 *CmdResp_p)
{
    /* Free semaphore */
    os_SignalObjectSet (hOs, pSignalObject);
}

/** 
 * \fn     wlanDrvIf_StopTx
 * \brief  block Tx thread until wlanDrvIf_ResumeTx called .
 * 
 * This routine is called whenever we need to stop the network stack to send us pakets since one of our Q's is full.
 *
 * \note   
 * \param  hOs           - The driver object handle
* \return 
 * \sa     wlanDrvIf_StopTx
 */ 
void wlanDrvIf_StopTx (TI_HANDLE hOs)
{
	TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)hOs;
    os_SignalObjectWait (hOs, pAdapter->pTxSignalObject);
}

/** 
 * \fn     wlanDrvIf_ResumeTx
 * \brief  Resume Tx thread .
 * 
 * This routine is called whenever we need to resume the network stack to send us pakets since our Q's are empty.
 *
 * \note   
 * \param  hOs           - The driver object handle
 * \return 
 * \sa     wlanDrvIf_ResumeTx
 */ 
void wlanDrvIf_ResumeTx (TI_HANDLE hOs)
{
	TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr)hOs;
    os_SignalObjectSet (hOs, pAdapter->pTxSignalObject);
}