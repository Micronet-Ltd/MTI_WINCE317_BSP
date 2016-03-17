//
// Copyright (c) Micronet LTD.  All rights reserved.
//
/*********************************************************************
*
*
*	Module Name: LDR.h
*
*	Abstract:
*		This file contains the LDR communication protocol and cmds
*       between bootloader  and desktop computer.
*
*	Author:   Anna R.
*
*	History:  May,2009 Created.
*
*	Note:
*
********************************************************************/
#ifndef LDR_H
#define LDR_H


#include <windows.h>
#include <halether.h>
#include "tftp.h"
#include "dhcp.h"
#include "udp.h"
#include <BootCmd.h>
#include <eboot.h>

//---------------------------------------------------------------------------
//           Loaders Errors
//---------------------------------------------------------------------------

enum LDR_ERRORS_TYPE
{
	LDR_NO_ERROR = 0,
	LDR_GENERAL_ERROR,
	LDR_WRONG_ARGUMENT,

};

// --------------------------------------------------------------------------------------------------
//  LDR protocol related functions
//  LDR protocol is the Logical Layer for CMD mode communications between host and device
//---------------------------------------------------------------------------------------------------

BOOL LDREtherTransport (     EDBG_ADDR *pEdbgAddr,     // IN our eth address
						      LPDWORD pdwSubnetMask,   // IN subnet mask
                              DWORD *pdwDHCPLeaseTime  // INthis parameter is overloaded. pass NULL to indicate static IP
							  );



void LDRSendBootme(EDBG_ADDR *pMyAddr);

BOOL
LDRProcessCmd(
    EDBG_ADDR *pMyAddr,     // IN - Our IP/ethernet address
    BYTE   *pFrameBuffer,   // IN - Pointer to frame buffer (for extracting src IP)
    UINT16 *pwUDPData,      // IN - Recvd UDP data
    UINT16 cwUDPDataLength // IN - Len of recvd data    
   ) ;

BOOL LDRSendBootmeAndWaitForMsg (EDBG_ADDR *pEdbgAddr);

/*
BOOL LDRSendResponse(EDBG_ADDR *pMyAddr,     // IN - Our IP/ethernet address			
					 BYTE   *pFrameBuffer,   // IN - Pointer to frame buffer (for extracting src IP)and sending frame buffer back
					 UINT16 *pwUDPData,       // IN - Recvd UDP data
					 UINT16 cwUDPDataLength , // IN - Len of recvd data,
					 USHORT ack,              // ack value to send back;
					 BYTE*  pLDRResp,         // LDR data to be sent
					 UINT16 cwLDRData         // number bytes to be sent (including LDR header)
					 );
					 */


BOOL LDRSendAck(EDBG_ADDR *pMyAddr,      // IN - Our IP/ethernet address			
				BYTE*   pRxFrameBuffer,  // IN - Pointer to frame buffer (for extracting src IP)
				UINT16* pwRxLDRData,     // IN - pointer to recvd LDR data
                UINT16  cbTxLDRData,     // IN loader responce data size (data + header)
				UINT16  ack              // IN ack value to send back;
				);


//------------------------------------------------------------
// LDR CMDs execution related functions
//-------------------------------------------------------------
DWORD LDRImageDownload(BYTE *pwLDRData,   UINT16 cwLDRDataLength);
BOOL LDRImageFlash();
BOOL LDRReadData(ULONG size, UCHAR *pData);
BOOL LDRInitData();



UINT16 getImageVersion(VersionInfoExt* pVersion);
UINT16 getXLDRVersion(VersionInfoExt* pVersion);
UINT16 getEBOOTVersion(VersionInfoExt* pVersion);
UINT16 GetVersion(UINT16 wVersionType,VersionInfo* pVersionInfo);
UINT16 GetVersionExt(UINT16 wVersionType,VersionInfoExt* pVersionInfo);


BOOL  DownloadBinRAM();
DWORD LDRLaunchImageFlash();


UINT16 GetHWID (HWVersions* pHWVersions);
//void LaunchImageRam();

UINT16 LdrGetPartEntryes( PARTENTRY* pEntryes );
UINT16 LdrGetFlashInfo( FlashInfo* pFlashInfo );
UINT16 LdrReadSector( BYTE* pBuf, UINT32 offset, UINT32 size );

BOOL	LDRBmpFlash();

// Ethernet frame type is in 7th word of buffer
#define FRAMETYPE(x)    ntohs(*(USHORT *)((x) + 6*sizeof(USHORT)))




#endif
