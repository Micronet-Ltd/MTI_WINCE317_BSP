/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
#include <bsp.h>
#include <bsp_version.h>
#include <bldver.h>
#include <i2c.h>
#include <oal_i2c.h>
#include "..\\oal_adds.h"
//------------------------------------------------------------------------------
//
//  Global:  g_oalIoCtlPlatformType/OEM    
//
//  Platform Type/OEM
//

    LPCWSTR g_oalIoCtlPlatformType = L"EVM3703                                         ";
	LPCWSTR g_oalIoCtlPlatformOEM  = L"Micronet CE-317";

//------------------------------------------------------------------------------
//
// Global: g_oalIoctlPlatformManufacturer/Name
//
//

LPCWSTR g_oalIoCtlPlatformManufacturer = L"Micronet Ltd.";
LPCWSTR g_oalIoCtlPlatformName = L"Mobile Data Terminal";

//------------------------------------------------------------------------------
//
//  Global:  g_oalIoCtlProcessorVendor/Name/Core
//
//  Processor information
//

LPCWSTR g_oalIoCtlProcessorVendor = L"Texas Instruments";
LPCWSTR g_oalIoCtlProcessorName   = L"DM3730/15";
LPCWSTR g_oalIoCtlProcessorCore   = L"Cortex-A8";

//------------------------------------------------------------------------------
//
//  Global:  g_oalIoctlInstructionSet/ClockSpeed
//
//  Processor instruction set identifier and maximal CPU speed
//
UINT32 g_oalIoCtlInstructionSet = PROCESSOR_FLOATINGPOINT;
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	UINT32 g_oalIoCtlClockSpeed = BSP_SPEED_CPUMHZ_600;
#else
	UINT32 g_oalIoCtlClockSpeed = BSP_SPEED_CPUMHZ;
#endif


//------------------------------------------------------------------------------
//
//  Global:  g_Versions
//
//  Version globals marked by pattern to be able to find by EBOOT



VersionPatternExt g_VersionPattern = {MARKER_LOW,BSP_VERSION_MAJOR,BSP_VERSION_MINOR,BSP_VERSION_INCREMENTAL,__DATE__,BSP_VERSION_STRING,MARKER_HIGH,BSP_VERSION_BUILD,MARKER_EXT };



//------------------------------------------------------------------------------
//
//  Function:  BSPIoCtlHalInitRegistry
//
//  Implements the IOCTL_HAL_INITREGISTRY handler.

BOOL BSPIoCtlHalInitRegistry(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	HKEY hk;
	if(ERROR_SUCCESS == NKRegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Bluetooth\\Transports\\BuiltIn\\1", 0, 0, &hk))
	{
        RETAILMSG(1, (L"IOCTL_HAL_INITREGISTRY disable BT serial driver\r\n"));
		//NKRegSetValueEx(hk, L"flags", 0, REG_DWORD, (UCHAR*)&flags, sizeof(DWORD));
		NKRegSetValueEx(hk, L"driver", 0, REG_SZ, (UINT8 *)L"\0", sizeof(L"\0"));
		NKRegCloseKey(hk);
	}
    if(!g_oalKitlEnabled)
    {
        // If KITL isn't enabled, ensure that USB and Ethernet drivers are not
        // blocked.  This logic prevents a persistent registry from inadvertently blocking
        // these drivers when KITL has been removed from an image.
        OEMEthernetDriverEnable(1);
        OEMUsbDriverEnable(1);
    }
        
    // call RTC init
    OALIoCtlHalInitRTC(IOCTL_HAL_INIT_RTC, NULL, 0, NULL, 0, NULL);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalHiveCleanFlag
//
//  Implements the IOCTL_HAL_GET_HIVE_CLEAN_FLAG handler.

BOOL OALIoCtlHalHiveCleanFlag(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	UINT32 *pdwFlags = (UINT32 *)pInpBuffer;
	INT32  *pfClean;

	if(!pInpBuffer || (inpSize != sizeof(UINT32)) || !pOutBuffer || (outSize != sizeof(INT32)))
	{
		NKSetLastError(ERROR_INVALID_PARAMETER);

		return 0;
	}
	else
	{
		*(INT32 *)pOutBuffer = 0;

		pfClean = OALArgsQuery(OAL_ARGS_QUERY_HIVECLEAN);

		if(pfClean && *pfClean)
		{
			if(*pdwFlags == HIVECLEANFLAG_SYSTEM)
			{
				*(INT32 *)pOutBuffer = 1;

				OALMSG(1, (TEXT("OEM: %s cleaning system hive\r\n"), (*pfClean)?L" ":L"Not"));
			}

			if(*pdwFlags == HIVECLEANFLAG_USERS)
			{
				*(INT32 *)pOutBuffer = 1;

				OALMSG(1, (TEXT("OEM: %s cleaning user profiles\r\n"), (*pfClean)?L" ":L"Not"));
			}
		}
	}

	return 1;
}

//------------------------------------------------------------------------------
//
//  Global:  g_oalIoCtlTable[]    
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs  
//  defined in the IOCTL configuration file. This global array is exported 
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] = {
#include "ioctl_tab.h"
};

//-----------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalI2CCopyFnTable
//
//  returns OAL i2c routines to be called directly from drivers
//
BOOL 
OALIoCtlHalI2CCopyFnTable(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *pOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
    BOOL rc = FALSE;
    OAL_IFC_I2C *pIn;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalI2CCopyFnTable\r\n"));
    
    if (pInBuffer == NULL || inSize != sizeof(OAL_IFC_I2C))
        {
        goto cleanUp;
        }

    // update info and call appropriate routine
    //
    if (pOutSize != NULL) *pOutSize = sizeof(OAL_IFC_I2C);
    pIn = (OAL_IFC_I2C*)pInBuffer;

    pIn->fnI2CLock = OALI2CLock;
    pIn->fnI2CUnlock = OALI2CUnlock;
    pIn->fnI2COpen = OALI2COpen;
    pIn->fnI2CClose = OALI2CClose;
    pIn->fnI2CWrite = OALI2CWrite;
    pIn->fnI2CRead = OALI2CRead;
    rc = TRUE;
    
cleanUp:
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-OALIoCtlHalI2CCopyFnTable(rc = %d)\r\n", rc));
    return rc;
}

#include "..\..\sdk\inc\swupdate_api.h"

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetBspVersion
//
//
BOOL OALIoCtlHalGetBspVersion(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL                        rc = FALSE;
    BspVersionExt*    pBspVersion;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetBspVersion\r\n"));

    // Check input parameters
    if ( !pInpBuffer || inpSize < sizeof(UINT32)	||
        pOutBuffer == NULL || outSize < sizeof(BspVersion)) 
	{
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_BSP_VERSION_OUT invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Copy to passed in struct
    pBspVersion = (BspVersionExt*)pOutBuffer;
 
	switch( *(UINT32*)pInpBuffer )
	{
		case BSP_VERSION:
		{
			pBspVersion->dwVersionMajor			= BSP_VERSION_MAJOR;
			pBspVersion->dwVersionMinor			= BSP_VERSION_MINOR;
			pBspVersion->dwVersionIncremental	= BSP_VERSION_INCREMENTAL;
			if(outSize >= sizeof(BspVersionExt))
			{
				pBspVersion->dwVersionBuild			= BSP_VERSION_BUILD;
			}
			rc = TRUE;
		}
		break;
		case EBOOT_VERSION:
		{
			UINT32* pTmpVer = (UINT32*)OALArgsQuery(OAL_ARGS_QUERY_EBOOT_VERS);
			if(pTmpVer)
			{
				RETAILMSG(1, (L"%x.%x.%x\r\n", ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMajor, 
												((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMinor,
												((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionIncremental));
				if(((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionIncremental / 100)
				{
					pBspVersion->dwVersionMajor			= ((Sw_Version*)pTmpVer)->Version.dwVersionMajor;
					pBspVersion->dwVersionMinor			= ((Sw_Version*)pTmpVer)->Version.dwVersionMinor;			
					pBspVersion->dwVersionIncremental	= ((Sw_Version*)pTmpVer)->Version.dwVersionIncremental;
					if(outSize >= sizeof(BspVersionExt))
					{
						pBspVersion->dwVersionBuild			= ((Sw_Version*)pTmpVer)->BuildVersion;
					}
				}
				else// old
				{	
					pBspVersion->dwVersionMajor			= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMajor;
					pBspVersion->dwVersionMinor			= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMinor;			
					pBspVersion->dwVersionIncremental	= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionIncremental;
					if(outSize >= sizeof(BspVersionExt))
					{
						pBspVersion->dwVersionBuild			= 0;
					}
				}
				rc = TRUE;
			}

		}
		break;
		case XLDR_VERSION:
		{
			UINT32* pTmpVer = (UINT32*)OALArgsQuery(OAL_ARGS_QUERY_XLDR_VERS);
			if(pTmpVer)
			{
				if(((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionIncremental / 100)
				{
					pBspVersion->dwVersionMajor			= ((Sw_Version*)pTmpVer)->Version.dwVersionMajor;
					pBspVersion->dwVersionMinor			= ((Sw_Version*)pTmpVer)->Version.dwVersionMinor;			
					pBspVersion->dwVersionIncremental	= ((Sw_Version*)pTmpVer)->Version.dwVersionIncremental;
					if(outSize >= sizeof(BspVersionExt))
					{
						pBspVersion->dwVersionBuild			= ((Sw_Version*)pTmpVer)->BuildVersion;
					}
				}
				else// old
				{	
					pBspVersion->dwVersionMajor			= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMajor;
					pBspVersion->dwVersionMinor			= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionMinor;			
					pBspVersion->dwVersionIncremental	= ((IOCTL_HAL_GET_BSP_VERSION_OUT*)pTmpVer)->dwVersionIncremental;
					if(outSize >= sizeof(IOCTL_HAL_GET_BSP_VERSION_OUT))
					{
						pBspVersion->dwVersionBuild			= 0;
					}
				}
				rc = TRUE;
			}
		}
		break;
		default:
		{
			NKSetLastError(ERROR_INVALID_PARAMETER);
		}
		break;
	}

    
	if(rc && pOutSize)
	{
		if(outSize >= sizeof(BspVersionExt))
			*pOutSize = sizeof(BspVersionExt);
		else
			*pOutSize = sizeof(BspVersion);
	}
cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (
        L"-OALIoCtlHalGetBspVersion(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoctlHalGetNeonStats 
//
//
BOOL OALIoctlHalGetNeonStats( 
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize
)
{  
    
    BOOL        rc = FALSE;
    IOCTL_HAL_GET_NEON_STAT_S*   pVfpStat;

    UNREFERENCED_PARAMETER(code);//
    UNREFERENCED_PARAMETER(inpSize);//
    UNREFERENCED_PARAMETER(pOutSize);//
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoctlHalGetNeonStats\r\n"));

    // Check parameters
    if ((pOutBuffer==NULL) && (outSize < sizeof(IOCTL_HAL_GET_NEON_STAT_S)))
    {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_NEON_STATS invalid parameters\r\n"
        ));
        goto cleanUp;
    }
    //  Copy to passed in struct
    pVfpStat = (IOCTL_HAL_GET_NEON_STAT_S*)pOutBuffer;    
    memcpy(pVfpStat,&g_oalNeonStat,sizeof(IOCTL_HAL_GET_NEON_STAT_S));
   
    if ((pInpBuffer!=NULL) && (wcscmp((LPWSTR)pInpBuffer, L"clear")==0))
    {
        /* User wants us to clear the stats */
        memset(&g_oalNeonStat,0,sizeof(IOCTL_HAL_GET_NEON_STAT_S));
    } 

    rc = TRUE;
            
cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (
        L"-OALIoctlHalGetNeonStats(rc = %d)\r\n", rc
    ));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetDspInfo
//
//
BOOL OALIoCtlHalGetDspInfo(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL                        rc = FALSE;
    IOCTL_HAL_GET_DSP_INFO_OUT* pDspInfo;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetDspInfo\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(IOCTL_HAL_GET_DSP_INFO)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_DSP_INFO_OUT invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Copy to passed in struct
    pDspInfo = (IOCTL_HAL_GET_DSP_INFO_OUT*)pOutBuffer;
    
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	{
		UINT32 *pOpm = OALPAtoUA(IMAGE_SHARE_OPM);

		if(4 == *pOpm)
			pDspInfo->dwSpeedMHz = BSP_IVA2_DPLL_MULT_874;
		else if(3 == *pOpm)
			pDspInfo->dwSpeedMHz = BSP_IVA2_DPLL_MULT_660;
		else
			pDspInfo->dwSpeedMHz = BSP_IVA2_DPLL_MULT_520;

		OALMSG(1, (L"OALIoCtlHalGetDspInfo: DSP Speed 0x%x MHz\r\n",pDspInfo->dwSpeedMHz));
	}
#else
    pDspInfo->dwSpeedMHz = BSP_SPEED_IVAMHZ;
#endif
    
    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (
        L"-OALIoCtlHalGetDspInfo(rc = %d)\r\n", rc
    ));
    return rc;

}

extern void DumpPrcmRegs();

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalDumpRegisters
//
//
BOOL OALIoCtlHalDumpRegisters(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalDumpRegisters\r\n"));

    // Check input parameters
    if (pInpBuffer == NULL || inpSize != sizeof(DWORD))
        {
        goto cleanUp;
        }

    // could use input parameter to select device to dump (not implemented)
    switch (*(DWORD *)pInpBuffer)
	{
        case IOCTL_HAL_DUMP_REGISTERS_PRCM:
		    DumpPrcmRegs();
                rc = TRUE;
			break;
	}

cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (
        L"-OALIoCtlHalDumpRegisters(rc = %d)\r\n", rc
    ));
    return rc;

}
//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetEthMacAddr
//
//  Implements the IOCTL_HAL_GET_ETH_MACADDR handler.
//
BOOL OALIoCtlHalGetEthMacAddr(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32  size = 6*sizeof(UINT8);
    UINT8   *pMAC = NULL;


    // Check buffer size
    if(pOutSize)
		*pOutSize = size;

    if(!pOutBuffer || outSize < size)
	{
        OALMSG(OAL_WARN, (L"ERR: OALIoCtlHalGetEthMacAddr: Buffer too small\r\n"));
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);

        return 0;
    }

    // Get a MAC address for RNDIS
    pMAC = OALArgsQuery(OAL_ARGS_QUERY_ETHMAC);
    if(pMAC) // Copy pattern to output buffer
        memcpy(pOutBuffer, pMAC, size);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetRNdisMacAddr\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetWiFiMacAddr
//
//  Implements the IOCTL_HAL_GET_WIFI_MACADDR handler.
//
BOOL OALIoCtlHalGetWiFiMacAddr(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32  size = 6*sizeof(UINT8);
    UINT8   *pMAC = NULL;


    // Check buffer size
    if(pOutSize)
		*pOutSize = size;

    if(!pOutBuffer || outSize < size)
	{
        OALMSG(OAL_WARN, (L"ERR: OALIoCtlHalGetWiFiMacAddr: Buffer too small\r\n"));
        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);

        return 0;
    }

    // Get a MAC address for RNDIS
    pMAC = OALArgsQuery(OAL_ARGS_QUERY_WIFIMAC);
    if(pMAC) // Copy pattern to output buffer
        memcpy(pOutBuffer, pMAC, size);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetWiFiMacAddr\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetCdsParams
//
//  Implements the IOCTL_HAL_GET_CDS_PARAMS handler.
//
BOOL OALIoCtlHalGetCdsParams(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    UINT32  size = 8*sizeof(UINT16); //Gets Both temperatures and frequencies
    DWORD*  pCDSParams = NULL;


    // Check buffer size
    if(pOutSize)
		*pOutSize = size;

    if(!pOutBuffer || outSize < size)
	{
        OALMSG(OAL_WARN, (L"ERR: OALIoCtlHalGetCdsParams: Buffer too small\r\n"));

        RETAILMSG(1,  (L"ERR: OALIoCtlHalGetCdsParams: Buffer too small\r\n"));

        NKSetLastError(ERROR_INSUFFICIENT_BUFFER);

        return 0;
    }

    pCDSParams = OALArgsQuery(OAL_ARGS_QUERY_CDSPARAMS);
    if(pCDSParams) // Copy pattern to output buffer
        memcpy(pOutBuffer, pCDSParams, size);

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetCdsParams\r\n"));
    RETAILMSG(1, (L"-OALIoCtlHalGetCdsParams\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalSetMpuSpeed
//
//  Implements the IOCTL_HAL_SET_MPU_SPEED handler.
//
BOOL OALIoCtlHalSetMpuSpeed(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	if (pInpBuffer == NULL) {
		OALMSG(OAL_IOCTL&&OAL_FUNC, (L"Error: OALIoCtlHalSetMpuSpeed Null input buffer\r\n"));
		return 0;
	}

	g_oalIoCtlClockSpeed = *(UINT32*)pInpBuffer;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalSetMpuSpeed\r\n"));

    return 1;
}



//  Function: OALIoCtlHalQueryCardConfig
//
//
BOOL OALIoCtlHalQueryCardConfig(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL     rc = 0;
    card_ver *pcv;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalQueryCardConfig\r\n"));

    // Check input parameters
    if(!pInpBuffer || inpSize < sizeof(UINT32) || pOutBuffer == NULL || outSize < sizeof(card_ver))
	{
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(1, (L"WARN: IOCTL_HAL_QUERY_CARD_CFG invalid parameters\r\n"));
        return 0;
    }

    //  Copy to passed in struct
    pcv = (card_ver *)pOutBuffer;
 
	switch(*(UINT32*)pInpBuffer)
	{
		case OAL_ARGS_QUERY_MAIN_CFG:
		case OAL_ARGS_QUERY_LCD_CFG:
		case OAL_ARGS_QUERY_WLAN_CFG:
		case OAL_ARGS_QUERY_CAN_CFG:
		case OAL_ARGS_QUERY_GPS_CFG:
		case OAL_ARGS_QUERY_MODEM_CFG:
		case OAL_ARGS_QUERY_EXTAUDIO_CFG:
		{
			card_ver *pVer = (card_ver *)OALArgsQuery(*(UINT32*)pInpBuffer);
			if(pVer)
			{
				memcpy(pcv, pVer, sizeof(card_ver));
				rc = 1;
			}
			break;
		}
		default:
		{
			NKSetLastError(ERROR_INVALID_PARAMETER);
		}
		break;
	}

    OALMSG(OAL_IOCTL && OAL_FUNC, (L"-OALIoCtlHalQueryCardConfig(rc = %d)\r\n", rc));

    return rc;

}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalParamBlock
//
//  Implements the IOCTL_HAL_UPDATE_FLAG handler.

BOOL OALIoCtlHalParamBlock(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	BOOL	ret = 0;
	void*	pBlock;

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalParamBlock\r\n"));

	if( pBlock = OALArgsQuery(OAL_ARGS_QUERY_MUNUFACT) )
	{
		if(pOutBuffer && (outSize == sizeof(manufacture_block)) && pOutSize )
		{
			memcpy( pOutBuffer,	pBlock, sizeof(manufacture_block));
			*pOutSize				= sizeof(UINT32);
			ret = 1;
		}
	}
	OALMSG(0, (L"OALIoCtlHalParamBlock pfFlags %x \r\n", pBlock));

	if( !ret )
		NKSetLastError(ERROR_INVALID_PARAMETER);

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalParamBlock ret %x\r\n", ret));
	return ret;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlSwUpdateFlag
//
//  Implements the IOCTL_HAL_UPDATE_FLAG handler.

BOOL OALIoCtlHalSwUpdateFlag(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	BOOL	ret = 0;
	INT32*	pfFlags;

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalSwUpdateFlag\r\n"));
//	OALMSG(1, (L"+OALIoCtlHalSwUpdateFlag %x %x %x %x %x \r\n", pInpBuffer, inpSize, pOutBuffer, outSize, pOutSize));

	if( pfFlags = OALArgsQuery(OAL_ARGS_QUERY_SW_UPDATE) )
	{
		if( pInpBuffer && ( inpSize == sizeof(UINT32) ) )
		{
			*pfFlags = *((INT32*)pInpBuffer);
			ret = 1;
		}

		if(pOutBuffer && (outSize == sizeof(UINT32)) && pOutSize )
		{
			*((INT32*)pOutBuffer)	= *pfFlags;
			*pOutSize				= sizeof(UINT32);
			ret = 1;
		}
	}
	OALMSG(0, (L"OALIoCtlHalSwUpdateFlag pfFlags %x \r\n", pfFlags));

	if( !ret )
		NKSetLastError(ERROR_INVALID_PARAMETER);

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalSwUpdateFlag ret %x\r\n", ret));
//	OALMSG(1, (L"-OALIoCtlHalSwUpdateFlag ret %x\r\n", ret));
	return ret;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHwBootStat
//
//  Implements the IOCTL_HAL_UPDATE_FLAG handler.

BOOL OALIoCtlHalGetHwBootStat(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	BOOL	ret = 0;
	void*	pStat;

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetHwBootStat\r\n"));

	if( pStat = OALArgsQuery(OAL_ARGS_QUARY_HW_BOOT_STAT) )
	{
		if(pOutBuffer && (outSize == sizeof(hw_boot_status)) && pOutSize )
		{
			memcpy(pOutBuffer, pStat, sizeof(hw_boot_status));
			*pOutSize				= sizeof(hw_boot_status);
			ret = 1;
		}
	}
	OALMSG(1, (L"OALIoCtlHalGetHwBootStat pStat %x \r\n", *(UINT32 *)pStat));

	if( !ret )
		NKSetLastError(ERROR_INVALID_PARAMETER);

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHwBootStat ret %x\r\n", ret));
	return ret;
}

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalGetHwBootStatEx
//
//  Implements the IOCTL_HAL_UPDATE_FLAG handler.

BOOL OALIoCtlHalGetHwBootStatEx(UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
	BOOL	ret = 0;
	void*	pStat;
	hw_boot_status_ex * pHwBootStatusEx = pOutBuffer;

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetHwBootStatEx\r\n"));

	if(pOutBuffer && (outSize == sizeof(hw_boot_status_ex)))
	{
		if( pStat = OALArgsQuery(OAL_ARGS_QUARY_HW_BOOT_STAT) )
		{
			{
				memcpy(&(pHwBootStatusEx->HwBootStatus), pStat, sizeof(hw_boot_status));
				OALMSG(1, (L"OALIoCtlHalGetHwBootStatEx OAL_ARGS_QUARY_HW_BOOT_STAT pStat %x \r\n", *(UINT32 *)pStat));

				if( pStat = OALArgsQuery(OAL_ARGS_QUERY_THERMALSHUTDOWN) )
				{
					memcpy(&(pHwBootStatusEx->ThermalShutdownStatus), pStat, sizeof(UINT32));
					*pOutSize = sizeof(hw_boot_status_ex);
					ret = 1;
				}

				OALMSG(1, (L"OALIoCtlHalGetHwBootStatEx OAL_ARGS_QUERY_THERMALSHUTDOWN pStat %x \r\n", *(UINT32 *)pStat));
			}
		}
	}

	if( !ret )
		NKSetLastError(ERROR_INVALID_PARAMETER);

	OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHalGetHwBootStatEx ret %x\r\n", ret));
	return ret;
}
