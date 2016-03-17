
//
// Copyright (c) Micronet LTD.  All rights reserved.
//
// History of Changes:
// Anna Rayer:  May 2009.   Created

/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    ldrtransp.c
Abstract:  

    This contains the code to implement Loader Transport Communications with HOST server.
--*/


#include "ldr.h"
#include "BootCmd.h"
#include <image_cfg.h>
#include <eboot.h>
#include <boot_args.h>
#include <omap35xx.h>
#include <blcommon.h>
#include <bsp_version.h>
#include <args.h>


//--------------------------------------------------------------------------
// global variables
//--------------------------------------------------------------------------


// buffer to get any eth buffer
static BYTE gFrameBuffer[ETHER_MTU];

// buffer to send LDR data MAX_LDR_MSG_LENGTH + UDP_DATA_FRAME_OFFSET = ETHER_MTU
static BYTE gLDRFrameBuffer[ETHER_MTU];

// TRUE if loader got connection established cmd (CMD_STOP_BOOTME) from the host
static BOOL g_bConnected;

static BOOL g_bDownloaded = FALSE;

static BYTE g_Buffer[2048];
//---------------------------------------------------------------------------

//#define START_LDR_DATA       IMAGE_WINCE_CODE_PA

#define MAX_LDRBOOTME_CNT  20
#define LDRBOOTME_INTERVAL 3


//--------------------------------------------------------------------------
//external functions
//--------------------------------------------------------------------------



extern BOOL EbootCheckIP (EDBG_ADDR *pEdbgAddr);
extern BOOL EbootGetDHCPAddr (EDBG_ADDR *pEdbgAddr, DWORD *pdwSubnetMask, DWORD *pdwDHCPLeaseTime);

extern BOOL BLWriteSWRights(BYTE *pSWRights);
extern BOOL BLReadSWRights(BYTE *pSWRights);

extern BOOL BLReadManufactureCfg( BYTE *pManufCfg);
extern BOOL BLWriteManufactureCfg(BYTE *pManufCfg);


extern UINT32 ReadFlashNK();

extern	BOOL BLGetFlashInfo( FlashInformation* pFlashInfo );
extern	BOOL BLReadSector( BYTE* pBuf, DumpParams* params );


//--------------------------------------------------------------------------
// Loader functions
//--------------------------------------------------------------------------


/*-------------------------------------------------------------------------
 *   Function:    LDRSendBootme
 *
 *   Description: Send a broadcast UDP packet to identify 
 *                us as available to receive cmds
 *-------------------------------------------------------------------------
 */

void
LDRSendBootme(EDBG_ADDR *pMyAddr)
{
    EDBG_ADDR DestAddr;
    UCHAR BootmeBuf[sizeof(tCmdHeader)+UDP_DATA_FRAME_OFFSET];
	tCmdHeader *pHdr = (tCmdHeader *)(BootmeBuf + UDP_DATA_FRAME_OFFSET);

	
	memset(pHdr,0,sizeof(tCmdHeader));
    

    // Format BOOTME message

	pHdr->ProtID = LDR_PROTOCOL_ID;
	pHdr->Cmd=CMD_SEND_BOOTME;
	pHdr->CmdAck=NO_ACK;
	pHdr->SeqNum =1;

 //   KITLOutputDebugString("Sent BOOTME 2\r\n");
    // Use local broadcast for now
    memset(&DestAddr.dwIP, 0xFF, sizeof(DestAddr.dwIP));

    memset(&DestAddr.wMAC, 0xFF, sizeof(DestAddr.wMAC));
    DestAddr.wPort = htons(LDR_PORT);

    pMyAddr->wPort = htons(LDR_PORT);
//	KITLOutputDebugString("Sent BOOTME 3\r\n");
    
    if (!EbootSendUDP(BootmeBuf, &DestAddr, pMyAddr, (BYTE *)pHdr, sizeof(tCmdHeader)))
        KITLOutputDebugString("SendBootme()::Error on SendUDP() call\r\n");
    else
        KITLOutputDebugString("Sent BOOTME to %s\r\n", inet_ntoa(DestAddr.dwIP));


}




/*-------------------------------------------------------------------------
 *   Function:    LDRProcessCmd
 *
 *   Description: This functions gets UDP data, checks if it's 
 *                LDR protocol cmd. If yes, it processes and executes
 *                the LDR cmd.
 *-------------------------------------------------------------------------
 */
BOOL
LDRProcessCmd(
    EDBG_ADDR *pMyAddr,     // IN - Our IP/ethernet address
							
    BYTE   *pFrameBuffer,   // IN - Pointer to frame buffer (for extracting src IP)
    UINT16 *pwUDPData,      // IN - Recvd UDP data
    UINT16  cwUDPDataLength  // IN - Len of recvd data  
   ) 
{
    tCmdHeader* pRxHdr = (tCmdHeader *)((BYTE*)pwUDPData);
	tCmdHeader *pTxHdr = (tCmdHeader *)(gLDRFrameBuffer+UDP_DATA_FRAME_OFFSET);
	BYTE* pLDRTxData =   pTxHdr->Data;
	UINT16 cwTxLDRData = sizeof (tCmdHeader); // by default response data is the only header
	UINT16 ack = ACK_NO_ERROR;
	UINT16 cwDataLentgh;
    BYTE* p_data;
	DWORD launchAddress = 0;
//	BOOL ret;


//	BYTE* pLDRResponse = NULL;
//	UINT16 cwLDRData = sizeof (tCmdHeader); // by default response data is the only header
//	BYTE buffer[MANUFACTURE_BLOCK_SIZE];


	

//	int i;

   // EDBG_ADDR SrcAddr;
//	KITLOutputDebugString("LDRProcessCmd started 0. %d ,%x\r\n",cwUDPDataLength,(DWORD)pwUDPData);

	if (!pwUDPData )
	{
		KITLOutputDebugString("Wrong size of receive msg %d.\r\n",cwUDPDataLength);
		return FALSE;
	}

	
/*	

	for (i =0; i<cwUDPDataLength; i++)
	{
		KITLOutputDebugString("%x ",*((BYTE*)pwUDPData +i));
	}
	KITLOutputDebugString("LDRProcessCmd started 02 \r\n");
	*/
	

 //   KITLOutputDebugString("LDRProcessCmd started 03. %d \r\n",cwUDPDataLength); 

	if (pRxHdr->ProtID != LDR_PROTOCOL_ID)
	{
		KITLOutputDebugString("Not LDR cmd\r\n");
        return FALSE;
	}
	

//	KITLOutputDebugString("LDRProcessCmd started1.\r\n");

	// Check and see if anyone has started a download.  If so, don't accept
    // any commands from different hosts.
  //  SrcAddrFromFrame(&SrcAddr,pFrameBuffer);

	switch (pRxHdr->Cmd)
    {
        case CMD_STOP_BOOTME:
            KITLOutputDebugString("Got CMD_STOP_BOOTME\r\n");
            g_bConnected = TRUE;
			break;
            
		case CMD_BIN_DOWNLOAD:
			//KITLOutputDebugString("Got CMD_IMG_DOWNLOAD %x, %x \r\n", (DWORD)pHdr->Data,cwUDPDataLength - sizeof(tCmdHeader)+ sizeof(pHdr->Data));
            if (pRxHdr->SeqNum == 1)
			{
				LDRInitData();
				KITLOutputDebugString("First Download package is received \r\n");
			}
			
			cwDataLentgh = cwUDPDataLength - sizeof(tCmdHeader)+ sizeof(pRxHdr->Data);
			p_data = &pRxHdr->Data[0];

			ack = (UINT16) LDRImageDownload(p_data,cwDataLentgh);
			break;

		case CMD_FLASH_BIN:
			KITLOutputDebugString("Got CMD_FLASH_IMAGE cmd. \r\n");
			ack = (LDRImageFlash()==TRUE) ? ACK_NO_ERROR:ACK_ERROR_FLASH;
			break;

		case CMD_SET_MANUFACTURE:
			{
				KITLOutputDebugString("Got CMD_SET_MANUFACTURE cmd. \r\n");

				//cwDataLentgh = cwUDPDataLength - sizeof(tCmdHeader)+ sizeof(pRxHdr->Data);
				p_data = &pRxHdr->Data[0];

				ack = (BLWriteManufactureCfg(p_data)==TRUE)? ACK_NO_ERROR : ACK_ERROR_FLASH;
			}

			break;

		case CMD_GET_MANUFACTURE:
			{

				KITLOutputDebugString("Got CMD_GET_MANUFACTURE cmd. \r\n");

				ack = (BLReadManufactureCfg(pLDRTxData)==TRUE)? ACK_NO_ERROR : ACK_ERROR_FLASH;
				cwTxLDRData = MANUFACTURE_BLOCK_SIZE + sizeof (tCmdHeader)- sizeof (pRxHdr->Data);
			}

			break;


		case CMD_SET_SW_RIGHTS:
			{
				KITLOutputDebugString("Got CMD_SET_SW_RIGHTS cmd. \r\n");

				//cwDataLentgh = cwUDPDataLength - sizeof(tCmdHeader)+ sizeof(pRxHdr->Data);
				p_data = &pRxHdr->Data[0];

				ack = (BLWriteSWRights(p_data)==TRUE)? ACK_NO_ERROR : ACK_ERROR_FLASH;
			}

			break;

		case CMD_GET_SW_RIGHTS:
			{

				KITLOutputDebugString("Got CMD_GET_SW_RIGHTS cmd. \r\n");

				ack = (BLReadSWRights(pLDRTxData)==TRUE)? ACK_NO_ERROR : ACK_ERROR_FLASH;
				cwTxLDRData = SW_RIGHTS_BLOCK_SIZE + sizeof (tCmdHeader)- sizeof (pRxHdr->Data);
			}

			break;

		case CMD_GET_VERSION:
			{
				VersionInfo versionInfo;
				USHORT wVersionType;

				KITLOutputDebugString("Got CMD_GET_VERSION cmd. \r\n");

				if (pRxHdr->Data)
				{
					wVersionType = *((UINT16*)pRxHdr->Data);

					ack = GetVersion(wVersionType,&versionInfo);

					memcpy(pLDRTxData,&versionInfo,sizeof(versionInfo));

				}
				else
				{
					KITLOutputDebugString("Wrong VersionInfo data. \r\n");
					memset(pLDRTxData,0,sizeof(versionInfo));
					ack = ACK_WRONG_ARGUMENT;
				}
                cwTxLDRData = sizeof(versionInfo) + sizeof (tCmdHeader)- sizeof (pRxHdr->Data);
			}

			break;
		case CMD_GET_VERSION_EXT:
			{
				VersionInfoExt versionInfo;
				USHORT wVersionType;

				KITLOutputDebugString("Got CMD_GET_VERSION_EXT cmd. \r\n");

				if (pRxHdr->Data)
				{
					wVersionType = *((UINT16*)pRxHdr->Data);

					ack = GetVersionExt(wVersionType,&versionInfo);

					memcpy(pLDRTxData,&versionInfo,sizeof(versionInfo));

				}
				else
				{
					KITLOutputDebugString("Wrong VersionInfoExt data. \r\n");
					memset(pLDRTxData,0,sizeof(versionInfo));
					ack = ACK_WRONG_ARGUMENT;
				}
                cwTxLDRData = sizeof(versionInfo) + sizeof (tCmdHeader)- sizeof (pRxHdr->Data);
			}

			break;
		case CMD_GET_HW_VERSION:
			{
				HWVersions hwVersions;
				KITLOutputDebugString("Got CMD_GET_HW_VERSION cmd. \r\n");

				ack = GetHWID (&hwVersions);

				memcpy(pLDRTxData,&hwVersions,sizeof(HWVersions));

			    cwTxLDRData = sizeof(HWVersions) + sizeof (tCmdHeader)- sizeof (pTxHdr->Data);

				KITLOutputDebugString("Got CMD_GET_HW_VERSION cmd. cwTxLDRData = %x ,ack = %x \r\n",cwTxLDRData,ack);
			}
			break;

		case CMD_FORMAT_PART:
			{
				BSP_ARGS *pArgs = (BSP_ARGS *)IMAGE_SHARE_ARGS_PA;

				KITLOutputDebugString("Got CMD_FORMAT_PART cmd. \r\n");

				pArgs->bFormatPartFlag = TRUE;
				ack = ACK_NO_ERROR;
			}
			break;

		case CMD_ERASE_REGISTRY:
			{
				BSP_ARGS *pArgs = (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				KITLOutputDebugString("Got CMD_ERASE_REGISTRY cmd. \r\n");
				pArgs->bHiveCleanFlag = TRUE;
				ack = ACK_NO_ERROR;
			}
			break;
            
		case CMD_IMG_RAM_LAUNCH:
			{
				KITLOutputDebugString("Got CMD_IMG_RAM_LAUNCH cmd \r\n");
				if (DownloadBinRAM()==TRUE)
				{
					ack = ACK_NO_ERROR;
					g_bDownloaded = TRUE;
				}
				else
				{ 
					KITLOutputDebugString("CMD_IMG_RAM_LAUNCH: Error:  the Image is not Dawnloaded\r\n" );
					ack = ACK_IMAGE_NOT_BURNED;

				}
			}

			break;
		case CMD_IMG_FLASH_LAUNCH:
			{
				KITLOutputDebugString("Got CMD_IMG_FLASH_LAUNCH cmd \r\n");

				if (ReadFlashNK()==BL_ERROR)
				{
					KITLOutputDebugString("getImageVersion: Error:  image is not burned\r\n" );
					ack = ACK_IMAGE_NOT_BURNED;
				}
				else
				{
					ack = ACK_NO_ERROR;
					g_bDownloaded = TRUE;
				}
			}
	
			break;
		case CMD_GET_FLASH_INFO:
		{
			FlashInformation	finfo;
			KITLOutputDebugString("Got CMD_GET_FLASH_INFO cmd \r\n");
			

			if( BLGetFlashInfo( &finfo ) )
			{
				memcpy(pLDRTxData, &finfo, sizeof(FlashInformation));
				ack = ACK_NO_ERROR;
			}
			else
			{
				memset(pLDRTxData, 0, sizeof(FlashInformation));
				ack = ACK_ERROR_FLASH;
			}
			cwTxLDRData = sizeof(FlashInformation) + sizeof (tCmdHeader)- sizeof (pTxHdr->Data);

		}
		break;
		case CMD_GET_PART_INFO:
		{
			PARTENTRY	pinfo[NUM_PARTS] = {0};
			KITLOutputDebugString("Got CMD_GET_PART_INFO cmd \r\n");
			
			if(ACK_NO_ERROR == (ack = LdrGetPartEntryes(pinfo)) )
				memcpy(pLDRTxData, pinfo, sizeof(pinfo));
			else
				memset(pLDRTxData, 0, sizeof(pinfo));

			cwTxLDRData = sizeof(pinfo) + sizeof (tCmdHeader)- sizeof (pTxHdr->Data);
		}
		break;
		case	CMD_GET_STORE_DUMP:
		{
			UINT32	size = 0, TotalLength = 0;
			DumpParams params;

			//KITLOutputDebugString("Got CMD_GET_STORE_DUMP cmd\r\n" );

			memcpy( (BYTE*)&params, pRxHdr->Data, sizeof(DumpParams) );

			//KITLOutputDebugString("CMD_GET_STORE_DUMP: offset %X, size %X\r\n", params.DumpOffset, params.DumpSize);
			
			ack = ACK_NO_ERROR;
	////////////////////////////////////temp!!!!!!! for the test loop only /////
	//		KITLOutputDebugString("DP: offset %X\r\n", params.DumpOffset);
	//size = MAX_LDR_DATA_LENGTH;
	//memcpy( pLDRTxData, g_Buffer + TotalLength, size );
	//cwTxLDRData		= size + sizeof(tCmdHeader) - sizeof(pTxHdr->Data);					
	//break;		
	////////////////////////////////////temp!!!!!!!/////////////////////////////
			if(pRxHdr->SeqNum	== 0)
			{
				if( !BLReadSector(g_Buffer, &params) )
				{
					KITLOutputDebugString("CMD_GET_STORE_DUMP: BLReadSector failed, offset %X, size %X\r\n", params.DumpOffset, params.DumpSize);
					ack = ACK_ERROR_FLASH;
					memset( pLDRTxData, 0, min(params.DumpSize, MAX_LDR_DATA_LENGTH) );
					cwTxLDRData	= min(params.DumpSize, MAX_LDR_DATA_LENGTH) + sizeof(tCmdHeader) - sizeof(pTxHdr->Data);					
					break;
				}
			}
			//KITLOutputDebugString("CMD_GET_STORE_DUMP: LdrReadSector ack = %d \r\n", ack);

			TotalLength = MAX_LDR_DATA_LENGTH * pRxHdr->SeqNum;
			
			if( TotalLength < params.DumpSize )
			{
				size = min( MAX_LDR_DATA_LENGTH, ( params.DumpSize - TotalLength ) );
				memcpy( pLDRTxData, g_Buffer + TotalLength, size );
				cwTxLDRData		= size + sizeof(tCmdHeader) - sizeof(pTxHdr->Data);					
			}
			else
			{
				ack = ACK_WRONG_ARGUMENT;//out of range
			}
		}
		break;
		case CMD_BURN_BITMAP:
		{
			if( LDRBmpFlash() )
				ack = ACK_NO_ERROR;
			else
				ack = ACK_IMAGE_NOT_BURNED;
		}
		break;
		default:
            KITLOutputDebugString("Unrecognized LDR cmd: 0x%X\r\n",pRxHdr->Cmd);
			ack = ACK_UNKNOWN_CMD;
    }

	// return LDRSendResponse(pMyAddr,pFrameBuffer, pwUDPData,cwUDPDataLength ,ack, pLDRResponse,cwLDRData);

	  return LDRSendAck(pMyAddr,pFrameBuffer, pwUDPData, cwTxLDRData,ack);
	  

}


/*-------------------------------------------------------------------------
 *   Function:    LDRSendAck
 *
 *   Description: Sends ack to the host computer with the requested data
 *-------------------------------------------------------------------------
 */
BOOL LDRSendAck(EDBG_ADDR *pMyAddr,      // IN - Our IP/ethernet address			
				BYTE*   pRxFrameBuffer,  // IN - Pointer to frame buffer (for extracting src IP)
				UINT16* pwRxLDRData,     // IN - pointer to recvd LDR data
                UINT16  cbTxLDRData,     // IN loader responce data size (data + header)
				UINT16  ack              // IN ack value to send back;
				)

{
	tCmdHeader* pRxHdr = (tCmdHeader *)((BYTE*)pwRxLDRData);
	tCmdHeader *pTxHdr = (tCmdHeader *)(gLDRFrameBuffer+UDP_DATA_FRAME_OFFSET);
    
    EDBG_ADDR HostAddr;
	BOOL ret = TRUE;


	SrcAddrFromFrame(&HostAddr, pRxFrameBuffer);

	pMyAddr->wPort = HostAddr.wPort;


    pTxHdr->ProtID = pRxHdr->ProtID;
    pTxHdr->Cmd    = pRxHdr->Cmd;
    pTxHdr->SeqNum = pRxHdr->SeqNum;
	pTxHdr->CmdAck = ack;
    
	if (!EbootSendUDP(gLDRFrameBuffer, &HostAddr, pMyAddr, (BYTE *)pTxHdr, cbTxLDRData))
	{
		KITLOutputDebugString("Error in SendUDP\r\n");
		ret = FALSE;
	}

	return ret;
}



#if 0

/*-------------------------------------------------------------------------
 *   Function:    LDRSendResponse
 *
 *   Description: Sends response to the host computer with the requested data
 *-------------------------------------------------------------------------
 */
BOOL LDRSendResponse(EDBG_ADDR *pMyAddr,     // IN - Our IP/ethernet address			
					 BYTE   *pFrameBuffer,   // IN - Pointer to frame buffer (for extracting src IP)
					 UINT16 *pwUDPData,       // IN - Recvd UDP data
					 UINT16 cwUDPDataLength , // IN - Len of recvd data,
					 USHORT ack,              // ack value to send back;
					 BYTE*  pLDRResp,         // LDR data to be sent (not including LDR header)
					 UINT16 cwLDRData         // number bytes to be sent (including LDR header)
					 )

{

    tCmdHeader *pHdr = (tCmdHeader *)(gLDRFrameBuffer+UDP_DATA_FRAME_OFFSET);
    EDBG_ADDR HostAddr;
	BOOL ret = TRUE;

	KITLOutputDebugString("LDRSendResponse UDP_DATA_FRAME_OFFSET = %x, data\r\n",UDP_DATA_FRAME_OFFSET - sizeof (tCmdHeader) + 2 );

    
    memcpy(pHdr, pwUDPData, sizeof(tCmdHeader));


    // Copy the address from the received frame
    SrcAddrFromFrame(&HostAddr, pFrameBuffer);
    pMyAddr->wPort = HostAddr.wPort;
	pHdr->CmdAck = ack;

	if (pLDRResp)
	{
		memcpy(pHdr->Data,pLDRResp,cwLDRData - sizeof (tCmdHeader) + 2);
		KITLOutputDebugString("Copied %x data\r\n",cwLDRData - sizeof (tCmdHeader) + 2 );
	}
    
    if (!EbootSendUDP(gLDRFrameBuffer, &HostAddr, pMyAddr, (BYTE *)pHdr, cwLDRData))
	{
		KITLOutputDebugString("Error in SendUDP1 %x\r\n" , cwLDRData);
		ret = FALSE;
	}


	return ret;
}

#endif


/*-------------------------------------------------------------------------
 *   Function:    LDRSendBootmeAndWaitForMsg
 *
 *   Description: ITs the main loop of Loader protocol.
 *                Its wait for any ethernet packet and checks if its
 *                LDR packet to execute a request
 *-------------------------------------------------------------------------
 */
BOOL LDRSendBootmeAndWaitForMsg (EDBG_ADDR *pEdbgAddr)
{
    DWORD dwCurSec = OEMKitlGetSecs () - LDRBOOTME_INTERVAL;
    USHORT wLen, wDestPort, wSrcPort, wUDPDataLen, *pwUDPData;
    int nRetries = 0;
	BOOL bJumpToImage = FALSE;

    KITLOutputDebugString ("+LDRSendBootmeAndWaitForMsg 1\r\n"); 
	g_bConnected = FALSE;
	while((nRetries < MAX_LDRBOOTME_CNT || g_bConnected) && !g_bDownloaded)
	{
		if (OEMKitlGetSecs () - dwCurSec >= LDRBOOTME_INTERVAL && !g_bConnected)
		{
			nRetries ++;
			dwCurSec += LDRBOOTME_INTERVAL;
			// send a bootme message
			LDRSendBootme (pEdbgAddr);
			KITLOutputDebugString ("LDRSendBootme started\r\n"); 
		}

        
        // get another frame and pass it to TFTP handler
        wLen = sizeof (gFrameBuffer);
        if (OEMEthGetFrame (gFrameBuffer, &wLen)) {       // frame available?
		//	KITLOutputDebugString ("Got Eth packet %d type: %x \r\n",wLen,FRAMETYPE (gFrameBuffer));
            switch (FRAMETYPE (gFrameBuffer)) {
            case 0x0800:    // IP packet
                if (!EbootCheckUDP(pEdbgAddr, gFrameBuffer, &wDestPort, &wSrcPort, &pwUDPData, &wUDPDataLen)) {  // UDP?

					//KITLOutputDebugString ("UDP packet.\r\n");
					if (!LDRProcessCmd(pEdbgAddr, gFrameBuffer, pwUDPData, wUDPDataLen)){
						//KITLOutputDebugString ("LDR cmd.\r\n");
					}
				}

                break;
            case 0x0806:    // ARP packet
                if (EbootProcessARP (pEdbgAddr, gFrameBuffer) == PROCESS_ARP_RESPONSE) {
                    KITLOutputDebugString( "Some other station has IP Address: %s !!! Aborting.\r\n", inet_ntoa(pEdbgAddr->dwIP));
                    return FALSE;
                }
                break;
            default:
				KITLOutputDebugString ("Undefined protocol.\r\n");
                break;
            }
        }
    }
    KITLOutputDebugString ("-LDRSendBootmeAndWaitForMsg\r\n"); 
    return TRUE;
}

    

/*-------------------------------------------------------------------------
 *   Function:    LDREtherTransport
 *
 *   Description: Starts loader communication with desktop.
 *-------------------------------------------------------------------------
 */

BOOL LDREtherTransport (     EDBG_ADDR *pEdbgAddr,     // IN our eth address
						      LPDWORD pdwSubnetMask,   // IN subnet mask
                              DWORD *pdwDHCPLeaseTime  // INthis parameter is overloaded. pass NULL to indicate static IP
							  )
                              
{
    // simple check on arguments
    if (!pEdbgAddr || !pdwSubnetMask ) {
        return FALSE;
    }

    // find out IP address and verify it
    if (pdwDHCPLeaseTime) {
        if (!EbootGetDHCPAddr (pEdbgAddr, pdwSubnetMask, pdwDHCPLeaseTime)) {
            return FALSE;
        }
    } else {
        if (!EbootCheckIP (pEdbgAddr)) {
            KITLOutputDebugString ("Some other station has IP Address: %s !!! Aborting.\r\n", inet_ntoa(pEdbgAddr->dwIP));
            return FALSE;
        }
        // This will tell CheckUDP() to only accept datagrams for our IP address
        ClearPromiscuousIP();
    }

       // enters into LDR communications loop here
	if (!LDRSendBootmeAndWaitForMsg (pEdbgAddr))
        return FALSE;

    return TRUE;
}

