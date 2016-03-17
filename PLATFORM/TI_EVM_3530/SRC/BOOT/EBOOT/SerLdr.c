// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Igor Lantsman

#include <eboot.h>
#include <ldr.h>
#include "bootcmd.h"
#include <swupdate.h>
#include <nand.h>
#include <fmd.h>
#include "nand_MT29F2G16.h"

extern BOOL BLWriteSWRights(UINT8 *pSWRights);
extern BOOL BLReadSWRights(UINT8 *pSWRights);
extern DWORD GetUniqueDeviceID();
extern VOID	FlReadDataSetStart(UCHAR *pDataStart);
extern BOOL BLSDCardReadFileData(ULONG MaxSize, UCHAR *pData);
extern BOOL BLWriteBmp( UINT32 address, UINT32 DataLength );
extern BOOL ResetGlobalVariables();
extern BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
extern BOOL OALEraseEntireFlash(UINT32 Param);

VOID BlSerMsgLoop();

#define		ESC_CHAR		0xA5
#define		START_CHAR		0xB8

#define		DOBOOT_MSG_LEN		8
#define		OKDOBOOT_MSG_LEN	6
#define		OKDOBOOT_MSG	"\r\nOK\r\n"
const char DOBOOT_MSG[] =	"\r\nBOOT\r\n";

UINT32 OALGetNandInfo(VOID* pInfo)
{
    HANDLE hFMD = NULL;
    PCI_REG_INFO regInfo;
	NAND_INFO*	ptrInfo;

	regInfo.MemBase.Reg[0] = g_ulFlashBase;
    hFMD = FMD_Init(NULL, &regInfo, NULL);
    if(hFMD == NULL) 
	{
		OALMSG(OAL_ERROR, (L"Can't open FMD driver\r\n"));
		return 0;
	}
	ptrInfo = NAND_GetGeometry(hFMD);
    if(ptrInfo == NULL) 
	{
		OALMSG(OAL_ERROR, (L"Can't get NAND info\r\n"));
		return 0;
	}
	memcpy(pInfo, ptrInfo, sizeof(NAND_INFO));

	FMD_Deinit(hFMD);
	return 1;
}

unsigned long CalcCS( const void *buf, int len)
{
	unsigned long cs=0;
	unsigned char *p=(unsigned char*)buf;
	while(len--)
	{
		cs+=*p++;
	}
	return cs;
}
BOOL CheckRndisMacAddress(UINT8 MacAddress[])
{
	if(MacAddress[0]!= 0x00 || MacAddress[1] != 0x02 )
		return 0;
	return 1;
}
VOID SetDefaultRndisMac(UINT8 RndisMacAddress[])
{
	UINT32 udId = GetUniqueDeviceID();
	RndisMacAddress[0] = 0x00;
	RndisMacAddress[1] = 0x02;

	RndisMacAddress[2] = (UCHAR)(udId);
	RndisMacAddress[3] = (UCHAR)(udId >> 8);
	RndisMacAddress[4] = (UCHAR)(udId >> 16);
	RndisMacAddress[5] = (UCHAR)(udId >> 24);
}    
VOID RecheckRndisMac(UINT8 RndisMacAddress[])
{
	if( !CheckRndisMacAddress(RndisMacAddress) )
		SetDefaultRndisMac(RndisMacAddress);
}

UINT32 BLSerReadData(UINT8* szBuffer, size_t CharCount)
{
    UINT32 count;
    UINT8 key;
    
    count = 0;
    while(count < CharCount) 
	{
        key = (UINT8)OALBLMenuReadKey(TRUE);
        szBuffer[count++] = key;
    }

    return count;   
}
VOID BLSerWriteData(const UINT8* szBuffer, size_t CharCount)
{
    while(CharCount--) 
	{
		OEMWriteDebugByte(*szBuffer++);
	}
}


UINT32 BLSerReadDataEsc(UINT8* szBuffer, size_t CharCount)
{
    UINT32 count;
    UINT8	key;
    
    count = 0;
    while(count < CharCount) 
	{
        key = (UINT8)OALBLMenuReadKey(TRUE);
		if((UINT8)ESC_CHAR == key)
		{
			key = (UINT8)OALBLMenuReadKey(TRUE);
			key += 2;
		}
		szBuffer[count++] = key;
    }

    return count;    
}

VOID BLSerWriteDataEsc(const UINT8* szBuffer, size_t CharCount)
{
    while(CharCount--) 
	{
		if( (UINT8)OEM_DEBUG_READ_NODATA	== *szBuffer	||
			(UINT8)OEM_DEBUG_COM_ERROR		== *szBuffer	||
			(UINT8)START_CHAR				== *szBuffer	||
			(UINT8)ESC_CHAR					== *szBuffer	)
		{
			OEMWriteDebugByte(ESC_CHAR);
			OEMWriteDebugByte((*szBuffer) - 2 );
			szBuffer++;
		}
		else
			OEMWriteDebugByte(*szBuffer++);
	}
}


void PostFormHeader( UINT8* buff, UINT32 Len )
{
	ProtocolHeader*	pHeader = (ProtocolHeader*)buff;
	
	pHeader->DataLen	= Len - sizeof(ProtocolHeader);
	pHeader->DataCS		= CalcCS( buff + sizeof(ProtocolHeader), Len - sizeof(ProtocolHeader) );
	pHeader->HeaderCS	= CalcCS( buff, sizeof(ProtocolHeader) - 4 );
}
INT BLSerWaitForChar(UINT8 Key, UINT32 Timeout)
{
	UINT32 time = OALGetTickCount() + Timeout;
	INT		err = -1;
	UINT8	key;
    while(	((key = (UINT8)OALBLMenuReadKey(0)) != Key )			&& 
			((INFINITE == Timeout) || (time > OALGetTickCount()))	);
    
	if( key == Key	) 
	{
		err = 0;
	}

    return err;
}

VOID	BlReset()
{
    OMAP_PRCM_GLOBAL_PRM_REGS * pPrmGlobal = OALPAtoUA(OMAP_PRCM_GLOBAL_PRM_REGS_PA);
	// Do warm reset
	OUTREG32(&pPrmGlobal->PRM_RSTCTRL, /*RSTCTRL_RST_DPLL3|*/ RSTCTRL_RST_GS);
}

VOID BlSerMsgLoop()
{
	UINT8*			pBuff;
	ProtocolHeader*	pHead;
	UINT8*			pData;
	UINT32			OutLen;
	char			fExit	= 1;
	const UINT8		Start	= START_CHAR;
	UINT32	Timeout = 0, i = 0;
	ProtocolHeader	hdr = {0};
	
	if(fExit)//connect
	{
		pBuff = (UINT8*)DOBOOT_MSG;
		Timeout = OALGetTickCount() + 200; 
		
		while(i < DOBOOT_MSG_LEN)
		{
			if(OALGetTickCount() > Timeout)
				return;
			if(0 == BLSerWaitForChar(pBuff[i], 20))
				i++;
		}
		BLSerWriteData(&Start, 1 );
		BLSerWriteData(OKDOBOOT_MSG, OKDOBOOT_MSG_LEN );
	}
	fExit = 0;

	g_eboot.type	= DOWNLOAD_TYPE_RAM;
	pBuff			= (UINT8*)IMAGE_DISPLAY_BUF_PA + (IMAGE_DISPLAY_BUF_SIZE>>1);
	pHead			= (ProtocolHeader*)pBuff;
	pData			= pBuff + sizeof(ProtocolHeader);


	Timeout = 400;
	while(!fExit)
	{
		OutLen = sizeof(ProtocolHeader);
		
		if(BLSerWaitForChar( Start, Timeout ))
			return;
		Timeout = INFINITE;

		BLSerReadDataEsc(pBuff, sizeof(ProtocolHeader));

		//check
		if( sizeof(ProtocolHeader)	!= pHead->HeaderSize	||
			pHead->HeaderCS			!= CalcCS( pHead, sizeof(ProtocolHeader) - 4 )	)
		{
			memset(pHead,0,sizeof(ProtocolHeader));
			pHead->Ack = ACK_CORRUPTED_CMD;

			BLSerWriteData(&Start, 1);
			PostFormHeader(pBuff, OutLen);
			BLSerWriteDataEsc(pBuff, OutLen);
			continue;
		}
		if(pHead->DataLen)
			BLSerReadDataEsc(pData, pHead->DataLen);

		switch(pHead->Cmd)
		{
			case CMD_SET_MANUFACTURE:
			{
				if(sizeof(manufacture_block) != pHead->DataLen )
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}
				
				if( pHead->DataCS	!= CalcCS( pData, sizeof(manufacture_block) ) ||
					!BLWriteManufactureCfg(pData) )//error
				{
					pHead->Ack = ACK_GENERAL_ERROR;
				}
			}
			break;
			case CMD_GET_MANUFACTURE:
			{
				if( !BLReadManufactureCfg(pData) )//error
				{
					pHead->Ack = ACK_GENERAL_ERROR;
					memset(pData, 0, sizeof(manufacture_block));
				}
				OutLen = sizeof(ProtocolHeader) + sizeof(manufacture_block);
			}
			break;
			case CMD_FORMAT_FLASH:
			{	
				BSP_ARGS *pArgs			= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				pArgs->coldBoot			= TRUE;
				if( !BLConfigureFlashPartitions(TRUE) )
					pHead->Ack = ACK_PARTITION_ERROR;
			}
			break;
			case CMD_FORMAT_PART:
			{
				BSP_ARGS *pArgs			= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				pArgs->bFormatPartFlag	= TRUE;
				pArgs->coldBoot			= TRUE;
			}
			break;
			case CMD_GET_ETHER_MAC:
			{
				memcpy((UINT16*)pData, g_bootCfg.EthMacAddress, sizeof(g_bootCfg.EthMacAddress));
				OutLen = sizeof(ProtocolHeader) + sizeof(UINT16) * 3;
			}
			break;
			case CMD_SET_ETHER_MAC:
			{
				BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				UINT16* mac		= (UINT16*)pData;
				if( sizeof(UINT16) * 3 != pHead->DataLen )
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}

				if( ((mac[0]== 0xFFFF) && (mac[1] == 0xFFFF	) && (mac[2] == 0xFFFF	)) || 
					((mac[0]== 0x0	 ) && (mac[1] == 0x0	) && (mac[2] == 0x0		))	)
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}
				memcpy((void*)g_bootCfg.EthMacAddress, mac, sizeof(pArgs->EthMacAddress));
				BLWriteBootCfg(&g_bootCfg);
				memcpy((void*)pArgs->EthMacAddress, mac, sizeof(pArgs->EthMacAddress));
			}
			break;
			case CMD_SET_WIFI_MAC:
			{
				BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				UINT16* mac		= (UINT16*)pData;
				if( sizeof(UINT16) * 3 != pHead->DataLen )
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}

				if( ((mac[0]== 0xFFFF) && (mac[1] == 0xFFFF	) && (mac[2] == 0xFFFF	)) || 
					((mac[0]== 0x0	 ) && (mac[1] == 0x0	) && (mac[2] == 0x0		))	)
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}

				memcpy((void*)g_bootCfg.WiFiMacAddress, mac, sizeof(pArgs->WiFiMacAddress));
				BLWriteBootCfg(&g_bootCfg);
				memcpy((void*)pArgs->WiFiMacAddress, mac, sizeof(pArgs->WiFiMacAddress));
			}
			break;
			case CMD_GET_WIFI_MAC:
			{
				memcpy((UINT16*)pData, g_bootCfg.WiFiMacAddress, sizeof(g_bootCfg.WiFiMacAddress));
				OutLen = sizeof(ProtocolHeader) + sizeof(UINT16) * 3;
			}
			break;
			case CMD_GET_RNDIS_MAC:
			{
				BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				RecheckRndisMac(pArgs->RndisMacAddress);
			
				memcpy(pData, (void*)pArgs->RndisMacAddress, sizeof(pArgs->RndisMacAddress));
                OutLen = sizeof(pArgs->RndisMacAddress) + sizeof(ProtocolHeader);
			}
			break;
			case CMD_SET_RNDIS_MAC:
			{
				BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				if( sizeof(pArgs->RndisMacAddress) != pHead->DataLen	||
					!CheckRndisMacAddress((UINT8*)pData)						)
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}
				memcpy((void*)g_bootCfg.RndisMacAddress, pData, sizeof(pArgs->RndisMacAddress));
				BLWriteBootCfg(&g_bootCfg);
				memcpy((void*)pArgs->RndisMacAddress, pData, sizeof(pArgs->RndisMacAddress));
			}
			break;
			case CMD_GET_VERSION:
			{
				VersionInfoExt versionInfo = {0};
				UINT16 param = (UINT16)pHead->Param;

				pHead->Ack = GetVersionExt(param, &versionInfo);

				memcpy(pData, &versionInfo, sizeof(VersionInfo));
				OutLen = sizeof(VersionInfo) + sizeof(ProtocolHeader);
			}
			break;
			case CMD_GET_VERSION_EXT:
			{
				VersionInfoExt versionInfo = {0};

				pHead->Ack = GetVersionExt((UINT16)pHead->Param, &versionInfo);

				memcpy(pData, &versionInfo, sizeof(VersionInfoExt));
				OutLen = sizeof(VersionInfoExt) + sizeof(ProtocolHeader);
			}
			break;
			case CMD_GET_HW_VERSION:
			{
				HWVersions hwVersions = {0};

				pHead->Ack = GetHWID(&hwVersions);

				memcpy(pData, &hwVersions, sizeof(HWVersions));
			    OutLen = sizeof(HWVersions) + sizeof(ProtocolHeader);
			}
			break;
			case	CMD_RESET_DEVICE:
			{
				BlReset();
				while(1);
			}
			break;
			case	CMD_IMG_FLASH_LAUNCH: 
			{
				fExit = 1;
			}
			break;
			case CMD_ERASE_REGISTRY:
			{
				BSP_ARGS *pArgs = (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				pArgs->bHiveCleanFlag	= TRUE;
				pArgs->coldBoot			= TRUE;
			}
			break;
			case CMD_GET_SW_RIGHTS:
			{
				if( !BLReadSWRights(pData) )
					pHead->Ack = ACK_ERROR_FLASH;
			    OutLen = SW_RIGHTS_BLOCK_SIZE + sizeof(ProtocolHeader);
			}
			break;
			case CMD_SET_SW_RIGHTS:
			{
				if( SW_RIGHTS_BLOCK_SIZE != pHead->DataLen )
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}
				if(!BLWriteSWRights(pData))
					pHead->Ack = ACK_ERROR_FLASH;
			}
			break;
			case CMD_GET_RAM_TEST_RESULT:
			{
				BSP_ARGS *pArgs = OALPAtoUA(IMAGE_SHARE_ARGS_PA);
				memcpy( pData,	(void*)&pArgs->ram_test_res, sizeof(ram_tst_result) );
			    OutLen = sizeof(ram_tst_result) + sizeof(ProtocolHeader);
			}
			break;
			case CMD_GET_FLASH_TEST:
			{
				pHead->Ack = ACK_NOT_IMPLEMENTED;
			}
			break;
			case CMD_BURN_SD_FILE:
			{
				BSP_ARGS *pArgs			= OALPAtoUA(IMAGE_SHARE_ARGS_PA);
				UINT32 MaxSize = 0, size = 0, tmp = 0;
				UINT8*	ptr = (UINT8*)IMAGE_WINCE_CODES_PA;	
				switch((UINT16)pHead->Param)
				{
					case sw_type_xldr:
						wcscpy(g_bootCfg.filename, L"XLDRNAND.BIN");
						MaxSize = IMAGE_XLDR_CODE_SIZE;
						ptr = (UINT8*)IMAGE_WINCE_CODE_PA;
					break;
					case sw_type_eboot:
						wcscpy(g_bootCfg.filename, L"EBNAND.BIN");
						MaxSize = IMAGE_EBOOT_CODE_SIZE;
						ptr = (UINT8*)IMAGE_WINCE_CODE_PA;
					break;
					case sw_type_mnfr:
						wcscpy(g_bootCfg.filename, L"MNFR.BIN");
						MaxSize = sizeof(manufacture_block);
					break;
					case sw_type_logo:
						wcscpy(g_bootCfg.filename, L"LOGO.BMP");
						MaxSize = IMAGE_BOOTLOADER_BITMAP_SIZE;
					break;
					case sw_type_nk:
						wcscpy(g_bootCfg.filename, L"NK.BIN");
						MaxSize = IMAGE_WINCE_CODE_SIZE;
					break;
				}
				if(0 == MaxSize)
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}
				if(	(BL_ERROR == BLSDCardDownload(&g_bootCfg, 0))	||
					(0 == (size = BLSDCardReadFileData(MaxSize, ptr))))
				{
					pHead->Ack = ACK_NOT_EXIST;
					break;
				}
				if(size > MaxSize) 
				{
					pHead->Ack = ACK_WRONG_SIZE;
					break;
				}

				if(sw_type_logo == (UINT16)pHead->Param)//sw_type_data
				{
				
					if(!BLWriteBmp((UINT32)ptr, size))
					{
						pHead->Ack = ACK_ERROR_FLASH;
					}
					else
					{
						memcpy(&hdr, pHead, sizeof(ProtocolHeader));
						BLShowLogo();
						memcpy(pHead, &hdr, sizeof(ProtocolHeader));
					}
				}
				else if(sw_type_mnfr == (UINT16)pHead->Param)
				{
					if( !BLWriteManufactureCfg(ptr) )
						pHead->Ack = ACK_ERROR_FLASH;
				}
				else//bin files
				{
					g_eboot.bootDeviceType			= OAL_KITL_TYPE_FLASH;
					g_bootCfg.bootDevLoc.LogicalLoc = BSP_NAND_REGS_PA;
				    FlReadDataSetStart(ptr);

					ResetGlobalVariables();
					if(!DownloadImage(&tmp, &size, &MaxSize))
						pHead->Ack = ACK_ERROR_FLASH;
					pArgs->coldBoot	= TRUE;
				}
			}
			break;
			case	CMD_ERASE_FLASH:
				{
					BSP_ARGS *pArgs			= OALPAtoUA(IMAGE_SHARE_ARGS_PA);
					pArgs->coldBoot			= TRUE;
					if( !OALEraseEntireFlash((UINT32)pHead->Param) )
						pHead->Ack = ACK_ERROR_FLASH;
				}
			break;
			case CMD_SET_MAX_FREQ:
			{
				BSP_ARGS *pArgs	= (BSP_ARGS *)(IMAGE_SHARE_ARGS_PA);
				UINT16* freq		= (UINT16*)pData;
				if( sizeof(UINT16) != pHead->DataLen )
				{
					pHead->Ack = ACK_WRONG_ARGUMENT;
					break;
				}

				memcpy((void*)&(g_bootCfg.CdsParams[4]), freq, sizeof(UINT16));
				BLWriteBootCfg(&g_bootCfg);
				memcpy((void*)&(pArgs->CdsParams[4]), freq, sizeof(UINT16));
			}
			break;
			case CMD_GET_MAX_FREQ:
			{
				memcpy((UINT16*)pData, &(g_bootCfg.CdsParams[4]), sizeof(UINT16));
				OutLen = sizeof(ProtocolHeader) + sizeof(UINT16);
			}
			break;
			case	CMD_GET_NAND_INFO:
			{
				NAND_INFO Info = {0};
				if(!OALGetNandInfo(&Info))
					pHead->Ack = ACK_ERROR_FLASH;

				memcpy(pData, &Info, sizeof(Info));
			    OutLen = sizeof(Info) + sizeof(ProtocolHeader);
			}
			break;
			default:
				pHead->Ack = ACK_NOT_IMPLEMENTED;
				break;
		}

		BLSerWriteData(&Start, 1 );
		PostFormHeader(pBuff, OutLen);
		BLSerWriteDataEsc(pBuff, OutLen );
	}
}
/*
INT BLSerReadWithTimeout(UINT8* pKey, UINT32 Timeout)
{
	UINT32 time = OALGetTickCount() + Timeout;
	INT		err = 0;
	UINT8	key;
    while( (	(key = (UINT8)OEMReadDebugByte()) == (UINT8)OEM_DEBUG_READ_NODATA ) && 
				( time > OALGetTickCount() )							);
    
	if( key == (UINT8)OEM_DEBUG_READ_NODATA	||
		key == (UINT8)OEM_DEBUG_COM_ERROR	) 
	{
		err = *pKey;
	}
	else
		*pKey = key;

    return err;
}

*/