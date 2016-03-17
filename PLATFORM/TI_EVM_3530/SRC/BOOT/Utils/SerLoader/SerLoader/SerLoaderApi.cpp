#include "stdafx.h"
#include "SerTransport.h"
#include "Protocol.h"
#include "bootcmd.h"
#include "SerLoaderApi.h"

//extern CProtocol*		pProt;

unsigned long SERLOADER_API LdrGetWaitTimeout(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;

	CProtocol*	pProt	= (CProtocol*)(hPort);
	return pProt->GetWaitTimeout();
}
void SERLOADER_API LdrSetWaitTimeout(HANDLE hPort, DWORD Timeout)
{
	if((HANDLE)-1 == hPort)
		return;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	pProt->SetWaitTimeout(Timeout);
}
unsigned long SERLOADER_API LdrOpen(unsigned int PortNum, HANDLE* phPort )
{
	if(!phPort)
		return FERROR_BAD_PARAMETERS;

	CSerTransport*	pSerTr	= new CSerTransport;	
	CProtocol*		pProt	= new CProtocol(pSerTr);
	
	DWORD err;
	TCHAR Name[64];
	
	if( PortNum < 10 )
		wsprintf(Name, _T("COM%d:"), PortNum);
	else
		wsprintf(Name, _T("\\\\.\\COM%d"), PortNum);

	HANDLE port;
	if( err = pProt->Open( Name, port ) )
	{
		delete pSerTr;
		delete pProt;
	}
	else
	{
		//*phPort = port;
		*phPort = (HANDLE)pProt;
	}
	return err;
}
unsigned long SERLOADER_API LdrClose(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	unsigned long err(-1);

	CProtocol*	pProt	= (CProtocol*)(hPort);
	err = pProt->Close(pProt->m_hPort);
	if(!err)
	{
		delete pProt->m_pTransport;
		delete pProt;
	}
	return err;
}
unsigned long SERLOADER_API LdrConnect(HANDLE hPort, DWORD Param)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	
	pProt->ClearReceive(pProt->m_hPort);
	if(Param)
		return pProt->ConnectAuto(pProt->m_hPort);
	return pProt->ConnectOld(pProt->m_hPort);
}
//
unsigned long SERLOADER_API LdrGetParamBlock(HANDLE hPort, manufacture_block* manuf_block)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen, err;

	pProt->ClearReceive(pProt->m_hPort);

	err = pProt->Get(pProt->m_hPort, CMD_GET_MANUFACTURE, (BYTE*)manuf_block, sizeof(manufacture_block), OutLen );

	return err;
}
unsigned long SERLOADER_API LdrSetParamBlock(HANDLE hPort, manufacture_block* manuf_block)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_MANUFACTURE, (BYTE*)manuf_block, sizeof(manufacture_block));
}
//
unsigned long SERLOADER_API LdrGetVersion(HANDLE hPort, VersionInfo* pVersInfo, VersionType type)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_VERSION, (BYTE*)pVersInfo, sizeof(VersionInfo), OutLen, type);
}
unsigned long SERLOADER_API LdrGetVersionExt(HANDLE hPort, VOID* pVersInfo, DWORD Size, VersionType type)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_VERSION_EXT, (BYTE*)pVersInfo, Size, OutLen, type);
}
unsigned long SERLOADER_API LdrGetHWVersion(HANDLE hPort, HWVersions* pVersInfo)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_HW_VERSION, (BYTE*)pVersInfo, sizeof(HWVersions), OutLen);
}
//
unsigned long SERLOADER_API LdrGetEtherMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_ETHER_MAC, (BYTE*)mac, sizeof(USHORT) * 3, OutLen);
}
unsigned long SERLOADER_API LdrSetEtherMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_ETHER_MAC, (BYTE*)mac, sizeof(USHORT) * 3);
}
//
//
unsigned long SERLOADER_API LdrGetWiFiMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_WIFI_MAC, (BYTE*)mac, sizeof(USHORT) * 3, OutLen);
}
unsigned long SERLOADER_API LdrSetWiFiMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_WIFI_MAC, (BYTE*)mac, sizeof(USHORT) * 3);
}
unsigned long SERLOADER_API LdrGetRndisMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_RNDIS_MAC, (BYTE*)mac, sizeof(USHORT) * 3, OutLen);
}
unsigned long SERLOADER_API LdrSetRndisMac(HANDLE hPort, unsigned short mac[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_RNDIS_MAC, (BYTE*)mac, sizeof(USHORT) * 3);
}
//
unsigned long SERLOADER_API LdrGetSwRights(HANDLE hPort, unsigned char sr[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_SW_RIGHTS, (BYTE*)sr, SW_RIGHTS_BLOCK_SIZE, OutLen);
}
unsigned long SERLOADER_API LdrSetSwRights(HANDLE hPort, unsigned char sr[] )
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_SW_RIGHTS, (BYTE*)sr, SW_RIGHTS_BLOCK_SIZE);
}
//
unsigned long SERLOADER_API LdrFormatFlash(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_FORMAT_FLASH, 0, 0);
}
unsigned long SERLOADER_API LdrFormatDataPartition(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_FORMAT_PART, 0, 0);
}
unsigned long SERLOADER_API LdrCleanRegistry(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_ERASE_REGISTRY, 0, 0);
}
//
unsigned long SERLOADER_API LdrReset(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->PutCmd(pProt->m_hPort, CMD_RESET_DEVICE, 0);
}
unsigned long SERLOADER_API LdrExitLoaderMode(HANDLE hPort)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_IMG_FLASH_LAUNCH, 0, 0);
}

unsigned long SERLOADER_API LdrGetRamTestResult(HANDLE hPort, ram_tst_result* tr)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_RAM_TEST_RESULT, (BYTE*)tr, sizeof(ram_tst_result), OutLen);
}

unsigned long SERLOADER_API LdrBurnSDFile(HANDLE hPort, sw_type_data type)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_BURN_SD_FILE, 0, 0, type);
}
unsigned long SERLOADER_API LdrEraseFlash(HANDLE hPort, unsigned int Param)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_ERASE_FLASH, 0, 0, Param);
}

unsigned long SERLOADER_API LdrGetMaxFreq(HANDLE hPort, UINT16 * freq)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_MAX_FREQ, (BYTE*)freq, sizeof(UINT16), OutLen);
}

unsigned long SERLOADER_API LdrSetMaxFreq(HANDLE hPort, UINT16 * freq)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Put(pProt->m_hPort, CMD_SET_MAX_FREQ, (BYTE*)freq, sizeof(UINT16));
}

unsigned long SERLOADER_API LdrGetNandInfo(HANDLE hPort, NAND_INFO* pInfo)
{
	if((HANDLE)-1 == hPort)
		return -1;
	CProtocol*	pProt	= (CProtocol*)(hPort);
	DWORD OutLen;

	pProt->ClearReceive(pProt->m_hPort);

	return pProt->Get(pProt->m_hPort, CMD_GET_NAND_INFO, (BYTE*)pInfo, sizeof(NAND_INFO), OutLen);
}