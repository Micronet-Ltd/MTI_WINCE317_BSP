
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
    ldrcmds.c
Abstract:  

    This file contains the code to execute Loader CMDs.
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

// pointer to save image into the ram.
static BYTE* g_pLDRData = NULL;


//---------------------------------------------------------------------------

#define START_LDR_DATA       IMAGE_WINCE_CODE_PA



//--------------------------------------------------------------------------
//external functions
//--------------------------------------------------------------------------



extern BOOL DownloadBin       (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
extern BOOL WriteImageToFlash ();
extern BOOL ResetGlobalVariables();
extern BOOL DownloadImage (LPDWORD pdwImageStart, LPDWORD pdwImageLength, LPDWORD pdwLaunchAddr);
extern BL_IMAGE_TYPE GetImageType();

//extern VOID JumpTo(UINT32 address);

extern UINT32 ReadFlashImage(UINT32* p_size);
extern UINT32 ReadFlashNK();
extern BOOL   ReadFlashXLDR (UINT32* p_size);
extern HANDLE GetPartEntry( PPARTENTRY pEntry, BYTE FileSystem, BYTE fActive );
extern	BOOL BLWriteBmp( UINT32 address, UINT32 DataLength );

//extern void OEMPlatformDeinit();
//extern void OEMDebugDeinit();



//--------------------------------------------------------------------------
// Loader functions
//--------------------------------------------------------------------------


    

/*-------------------------------------------------------------------------
 *   Function:    LDRImageDownload
 *
 *   Description: Put the downloaded LDR packet data into the RAM memory
 *-------------------------------------------------------------------------
 */

DWORD LDRImageDownload(BYTE *pwLDRData, UINT16 cwLDRDataLength)
{

	KITL_RETAILMSG(0,("g_pLDRData = %x, pwLDRData =%x cwLDRDataLength = %x \r\n",g_pLDRData ,pwLDRData, cwLDRDataLength));

	memcpy(g_pLDRData,(BYTE*)pwLDRData,cwLDRDataLength);

	g_pLDRData+=cwLDRDataLength;

	
	return ACK_NO_ERROR;
}



/*-------------------------------------------------------------------------
 *   Function:    LDRImageFlash
 *
 *   Description: Get the bin file from the RAM memory, take row data
 *                and put it into another RAM memory place and then burn it.
 *-------------------------------------------------------------------------
 */
BOOL LDRImageFlash()
{
	DWORD rval = FALSE;

	DWORD dwImageStart,dwImageLength,dwLaunchAddr;

	dwImageStart = dwImageLength = dwLaunchAddr = 0;

	ResetGlobalVariables();

	memcpy((BYTE*)IMAGE_WINCE_CODES_PA, (BYTE*)START_LDR_DATA, IMAGE_WINCE_CODE_SIZE);
	memset((BYTE*)START_LDR_DATA, 0xFF, IMAGE_WINCE_CODE_SIZE);
	g_pLDRData = (BYTE*)IMAGE_WINCE_CODES_PA;
	//LDRInitData();


	rval = DownloadImage(&dwImageStart,&dwImageLength, &dwLaunchAddr);


	// Change boot device to rndis (WriteFlashNK function changed it to NAND

    g_bootCfg.bootDevLoc.LogicalLoc = OMAP_USBHS_REGS_PA;

	
    return rval;

}

#if 0
void LaunchImage( DWORD launch)
{

// Print message, flush caches and jump to image
    KITL_RETAILMSG(0,("Launch Windows CE image by jumping to 0x%08x...\r\n\r\n", g_eboot.launchAddress));
	OEMPlatformDeinit();
    OEMDebugDeinit();

	JumpTo(launch); // never back from here


}


/*-------------------------------------------------------------------------
 *   Function:    LaunchImageFlash
 *
 *   Description: Read nk from flash and then Lauch it
 *-------------------------------------------------------------------------
 */
DWORD LaunchImageFlash()
{

	DWORD ret = 1;
	DWORD dwImageSize =0;
//	DWORD imageStartAddress = IMAGE_WINCE_CODE_PA;

	KITL_RETAILMSG(0,("LaunchImageFlash started \r\n"));



	if (ReadFlashNK()==BL_ERROR)
	{
		KITL_RETAILMSG(1,("LaunchImageFlash: Error:  image is not burned\r\n" ));
		return BL_ERROR;
	}

	// Print message, flush caches and jump to image
    KITL_RETAILMSG(1,("Launching Windows CE image from 0x%08x...\r\n\r\n", g_eboot.launchAddress));
	OEMPlatformDeinit();
    OEMDebugDeinit();

	JumpTo(g_eboot.launchAddress); // never back from here

	
	return ret;

}

#endif


/*-------------------------------------------------------------------------
 *   Function:    DownloadBinRAM
 *
 *   Description: read nk.bin from RAM and put into the RAM as row data
 *-------------------------------------------------------------------------
 */
BOOL DownloadBinRAM()
{

	DWORD rval = 1;
	DWORD dwImageType;
	DWORD dwStartAddress,dwImageLength,dwLaunchAddr;

	KITL_RETAILMSG(0,("DownloadBinRAM started \r\n"));


	ResetGlobalVariables();
	LDRInitData();

	 dwImageType = GetImageType();
	 if (dwImageType != BL_IMAGE_TYPE_BIN)
	 {
		 KITL_RETAILMSG(1,("DownloadBinRAM: the image is not bin file \r\n"));
		 return FALSE;
	 }


	 // read bin file from dawnloaded by loader place at RAM
	 // and put row data, ready for running at the right place 
	 // into the RAM for future execution.

	 rval &= DownloadBin( &dwStartAddress, &dwImageLength, &dwLaunchAddr );

	 g_eboot.launchAddress = dwStartAddress;


	 // Print message, flush caches and jump to image
/*	 
    KITL_RETAILMSG(0,("Launch Windows CE image by jumping to 0x%x...\r\n\r\n", g_eboot.launchAddress));

	OEMPlatformDeinit();
    OEMDebugDeinit();

	JumpTo(dwStartAddress); // never back from here
	
*/
	
	return  TRUE;

}


#if 0

BOOL DownloadBinRAM()
{

	DWORD rval = FALSE;

	DWORD dwImageStart,dwImageLength,dwLaunchAddr;

	dwImageStart = dwImageLength = dwLaunchAddr = 0;

	ResetGlobalVariables();
	LDRInitData();


	rval = DownloadImage(&dwImageStart,&dwImageLength, &dwLaunchAddr);


	// Change boot device to rndis (WriteFlashNK function changed it to NAND

    //g_bootCfg.bootDevLoc.LogicalLoc = OMAP_USBHS_REGS_PA;

	 g_eboot.launchAddress = dwImageStart;

	
    return rval;
}




/*-------------------------------------------------------------------------
 *   Function:    LDRLaunchImageFlash
 *
 *   Description: Read nk from flash and then Lauch it
 *-------------------------------------------------------------------------
 */

DWORD LDRLaunchImageFlash()
{

	DWORD ret = 1;
	DWORD dwImageSize =0;
//	DWORD imageStartAddress = IMAGE_WINCE_CODE_PA;

	KITL_RETAILMSG(0,("LDRLaunchImageFlash started \r\n"));


	if (ReadFlashImage(&dwImageSize)==BL_ERROR)
	{
		KITL_RETAILMSG(1,("getImageVersion: Error:  image is not burned\r\n" ));
		return BL_ERROR;
	}


	// Print message, flush caches and jump to image
    KITL_RETAILMSG(1,("Launching Windows CE image from 0x%08x...\r\n", g_eboot.launchAddress));
	OEMPlatformDeinit();
    OEMDebugDeinit();

	JumpTo(g_eboot.launchAddress); // never back from here

	
	return ret;

}

#endif




/*-------------------------------------------------------------------------
 *   Function:    GetVersion
 *
 *   Description: Get Version Info according to the component type 
 *-------------------------------------------------------------------------
 */
UINT16 GetVersionExt(UINT16 wVersionType,VersionInfoExt* pdwVersionInfo)
{
	UINT16 ret = ACK_NO_ERROR;


	KITL_RETAILMSG(0,("GetVersion: wVersionType: %d \r\n", wVersionType));
	switch(wVersionType)
	{
	case VERSION_XLDR:
		ret = getXLDRVersion(pdwVersionInfo);
		break;
	case VERSION_EBOOT:
		ret = getEBOOTVersion(pdwVersionInfo);
		break;

	case VERSION_IMG:
		ret = getImageVersion(pdwVersionInfo);
		break;

	default:
		ret = ACK_WRONG_ARGUMENT;

	};

//	KITL_RETAILMSG(0,("GetVersion: pdwVersionInfo: %d:%d:%d :%s\r\n", 
//		pdwVersionInfo->majorVersion,pdwVersionInfo->minorVersion,pdwVersionInfo->incrementalVersion,pdwVersionInfo->buildDate));

	return ret;
}

UINT16 GetVersion(UINT16 wVersionType,VersionInfo* pdwVersionInfo)
{
	VersionInfoExt ver = {0};
	UINT16 ret;

	ret = GetVersionExt(wVersionType, &ver);
	if(ret == ACK_NO_ERROR)
	{
		memcpy(pdwVersionInfo, &ver.Version, sizeof(VersionInfo));
	}
	return ret;
}

/*-------------------------------------------------------------------------
 *   Function:    getImageVersion
 *
 *   Description: Gets Image Version Info  
 *-------------------------------------------------------------------------
 */
UINT16 getImageVersion(VersionInfoExt* pVersion)
{
	// read image into ram at IMAGE_WINCE_CODE_PA offset
	UINT16 ret = ACK_VERSION_NOT_FOUND;
	DWORD dwImageSize =0;
	DWORD imageStartAddress = IMAGE_WINCE_CODE_PA;
	UINT32* Ptr32 ;
	VersionPatternExt* p_VersionPattern;
	UINT32			VersFlag = 0;

	memset(pVersion,  0, sizeof(VersionInfoExt) );


	if (ReadFlashImage(&dwImageSize)==BL_ERROR)
	{
		KITL_RETAILMSG(1,("getImageVersion: Error:  image is not burned\r\n" ));
		return ACK_IMAGE_NOT_BURNED;
	}

	Ptr32 	= (UINT32*)( imageStartAddress & 0xFFFFFFFC);

	for (;Ptr32< (UINT32*)(( imageStartAddress + dwImageSize ) & 0xFFFFFFFC); Ptr32++ )
	{
		if (*Ptr32!=MARKER_LOW)
			continue;

		KITL_RETAILMSG(0, ("Found low pattern Ptr32 = %x \r\n", (DWORD)Ptr32));

		VersFlag = 1;
		if(*(Ptr32 +( sizeof(VersionPattern)/sizeof(UINT32)) - 1) == MARKER_HIGH)//old version
		{
			VersFlag = 1;
			if(*(Ptr32 +( sizeof(VersionPatternExt)/sizeof(UINT32)) - 1) == MARKER_EXT)//new version 
			{
				OALMSG(1,(L"bsp vers found \r\n"));		
				VersFlag = 2;
			}
		}
		else
			continue;

		KITL_RETAILMSG(0,("Found high pattern Ptr32 +4 = %x \r\n", (DWORD)( Ptr32+( sizeof(VersionPattern)/sizeof(UINT32)) - 1 )));

        p_VersionPattern = (VersionPatternExt*)((BYTE*)(Ptr32));

        pVersion->Version.majorVersion      =  p_VersionPattern->ver.majorVersion;
		pVersion->Version.minorVersion      =  p_VersionPattern->ver.minorVersion;
		pVersion->Version.incrementalVersion=  p_VersionPattern->ver.incrementalVersion;

		if(2 == VersFlag)
		{
			OALMSG(1,(L"bsp prod vers %x\r\n", p_VersionPattern->Build));		
			pVersion->BuildVersion				=  p_VersionPattern->Build;
		}
		else
		{
			pVersion->BuildVersion				 =  0;
		}
		memcpy(pVersion->Version.buildDate,p_VersionPattern->ver.dateString,BUILD_DATE_STR_LENTGH);
        pVersion->Version.buildDate[BUILD_DATE_STR_LENTGH -1] = '\0';

		memcpy(pVersion->Version.versionString, p_VersionPattern->ver.versionString, sizeof(pVersion->Version.versionString));
        pVersion->Version.versionString[VERS_STRING_LEN - 1] = 0;
	
		ret = ACK_NO_ERROR ; // succeeded
		break;

	}

	return ret;
		
}


/*-------------------------------------------------------------------------
 *   Function:    getXLDRVersion
 *
 *   Description: Gets Xloader Version Info  
 *-------------------------------------------------------------------------
 */
UINT16 getXLDRVersion(VersionInfoExt* pVersion)
{
	// read image into ram at IMAGE_WINCE_CODE_PA offset
	UINT16 ret = ACK_VERSION_NOT_FOUND;
	DWORD dwSize =0;
	DWORD startAddress = IMAGE_WINCE_CODE_PA;
	UINT32* Ptr32 ;
	VersionPatternExt* p_VersionPattern;
	UINT32	VersFlag = 0;

	memset(pVersion,  0, sizeof(VersionInfoExt) );


	if (!ReadFlashXLDR(&dwSize))
	{
		KITL_RETAILMSG(1,("getXLDRVersion: Error:  XLDR is not burned\r\n" ));
		return ACK_IMAGE_NOT_BURNED;
	}

	Ptr32 	= (UINT32*)( startAddress & 0xFFFFFFFC);

	for (;Ptr32< (UINT32*)(( startAddress + dwSize ) & 0xFFFFFFFC); Ptr32++ )
	{
		if (*Ptr32 != MARKER_LOW)
			continue;

		if(*(Ptr32 + sizeof(VersionPattern)/sizeof(UINT32) - 1) == MARKER_HIGH)//old version
		{
			VersFlag = 1;
			if(*(Ptr32 + sizeof(VersionPatternExt)/sizeof(UINT32) - 1) == MARKER_EXT)//new version 
			{
				VersFlag = 2;
			}
		}
		else
			continue;

		KITL_RETAILMSG(0,("Found high pattern Ptr32 +4 = %x \r\n", (DWORD)( Ptr32+( sizeof(VersionPatternExt)/sizeof(UINT32)) - 1 )));

        p_VersionPattern = (VersionPatternExt*)((BYTE*)(Ptr32));

        pVersion->Version.majorVersion      =  p_VersionPattern->ver.majorVersion;
		pVersion->Version.minorVersion      =  p_VersionPattern->ver.minorVersion;
		pVersion->Version.incrementalVersion=  p_VersionPattern->ver.incrementalVersion;
		if(2 == VersFlag)
		{
			pVersion->BuildVersion				=  p_VersionPattern->Build;
		}
		else
		{
			pVersion->BuildVersion				 =  0;
		}
		memcpy(pVersion->Version.buildDate, p_VersionPattern->ver.dateString, BUILD_DATE_STR_LENTGH);
        pVersion->Version.buildDate[BUILD_DATE_STR_LENTGH -1] = '\0';

		memcpy(pVersion->Version.versionString, p_VersionPattern->ver.versionString, sizeof(pVersion->Version.versionString));
        pVersion->Version.versionString[VERS_STRING_LEN - 1] = 0;

		ret = ACK_NO_ERROR ; // succeeded
		break;

	}

	return ret;
		
}


/*-------------------------------------------------------------------------
 *   Function:    getEBOOTVersion
 *
 *   Description: Gets Eboot Version Info  
 *-------------------------------------------------------------------------
 */
UINT16 getEBOOTVersion(VersionInfoExt* pdwVersion)
{	
	pdwVersion->Version.majorVersion		= EBOOT_VERSION_MAJOR;
	pdwVersion->Version.minorVersion		= EBOOT_VERSION_MINOR;
	pdwVersion->Version.incrementalVersion	= EBOOT_VERSION_INCREMENTAL;
	pdwVersion->BuildVersion				= EBOOT_VERSION_BUILD;

	memcpy(pdwVersion->Version.buildDate,__DATE__,BUILD_DATE_STR_LENTGH);
	pdwVersion->Version.buildDate[BUILD_DATE_STR_LENTGH -1] = '\0';

	wcsncpy(pdwVersion->Version.versionString, EBOOT_VERSION_STRING, VERS_STRING_LEN);
	pdwVersion->Version.versionString[VERS_STRING_LEN - 1] = 0;
	return ACK_NO_ERROR;
}




/*-------------------------------------------------------------------------
 *   Function:    LDRReadData
 *
 *   Description: Read data from the  pData into the LDR RAM memory place 
 *-------------------------------------------------------------------------
 */
BOOL LDRReadData(ULONG size, UCHAR *pData)
{
	KITL_RETAILMSG(0,( "BLLDRReadData size = %x,g_pLDRData = %x  pData = %x \r\n", size,g_pLDRData,(DWORD)pData));
	memcpy(pData,g_pLDRData,size);
    g_pLDRData +=size;

	return TRUE;
}


/*-------------------------------------------------------------------------
 *   Function:    LDRInitData
 *
 *   Description: Put the pointer to the start LDR RAM memory place 
 *-------------------------------------------------------------------------
 */
BOOL LDRInitData()
{
	g_pLDRData = (BYTE*)START_LDR_DATA;
	return TRUE;
}




/*-------------------------------------------------------------------------
 *   Function:    GetHWID
 *
 *   Description: Get processor and companion chip versions 
 *-------------------------------------------------------------------------
 */
UINT16 GetHWID (HWVersions* pHWVersions)
{
	UINT32 * pIDAddr = OALPAtoUA(OMAP_IDCODE_REGS_PA);
	UINT32 dwIdReg,dwCrc;
	UINT8 version;
	HANDLE hTwl;

	if (!pHWVersions)
		return ACK_WRONG_ARGUMENT;

// Display processor and companion chip versions
	dwIdReg = INREG32(pIDAddr);
	switch (dwIdReg & 0xFFFFF000)
	{
		case 0x0B6D6000:
			version = 0;
		    break;
		case 0x0B7AE000:
			version = 1;
			break;
		case 0x1B7AE000:
			version = 2;
			break;

		case 0x2B7AE000:
			version = 5;
			break;

		case 0x4B7AE000:
			version = 6;
			break;


		default:
			version = 0xFF;
	}
	
	// Some ES2.1 silicon has incorrectly burned fuses indicating a different revision
	// Read the CRC in public ROM area to distinguish between revisions
	dwCrc = INREG32(PUBLIC_ROM_CRC_PA);
	switch (dwCrc)
	{
		case PUBLIC_ROM_CRC_ES2_0:
			// CRC indicates this is ES2.0
			if (version != 1)
				// Id register indicated this was some other rev, indicate ES2.0 with wrong Id
				version = 3;
			break;
		case PUBLIC_ROM_CRC_ES2_1:
			// CRC indicates this is ES2.1
			if (version < 2)
				// Id register indicated this was some other rev, indicate ES2.1 with wrong Id
				version = 4;
			break;
		default:
			break;
	}
		

	pHWVersions->CPUVersion = version;
	
	// Note that T2 accesses must occur after I2C initialization
	hTwl = OALTritonOpen();
    OALTritonRead(hTwl, TWL_IDCODE_31_24, &version);
    OALTritonClose(hTwl);

	
	pHWVersions->CoChipVersion = version;
	if (pHWVersions->CoChipVersion != 0x00 && pHWVersions->CoChipVersion != 0x10 
		&& pHWVersions->CoChipVersion != 0x40 && pHWVersions->CoChipVersion != 0x50)
	{
		pHWVersions->CoChipVersion = 0xFF;
	}

	KITL_RETAILMSG(1,("Omap Version: %x , TWL4030 Version = %x \r\n", pHWVersions->CPUVersion,pHWVersions->CoChipVersion ));

	return ACK_NO_ERROR;
}


UINT16 LdrGetPartEntryes( PARTENTRY* pEntryes )
{
	if( INVALID_HANDLE_VALUE == GetPartEntry( &pEntryes[0], PART_BOOTSECTION, FALSE ) )
	{
		KITL_RETAILMSG(1,("LdrGetPartEntryes error for PART_BOOTSECTION\r\n"));
		return ACK_PARTITION_ERROR;
	}

	GetPartEntry( &pEntryes[1], PART_DOS32, FALSE );
	
	return ACK_NO_ERROR;
}
BOOL LDRBmpFlash()
{
//	UINT32 size =  (UINT32)(g_pLDRData - START_LDR_DATA);
//	g_pLDRData = (BYTE*)START_LDR_DATA;
	
	return BLWriteBmp(START_LDR_DATA, (UINT32)g_pLDRData - START_LDR_DATA );
}


