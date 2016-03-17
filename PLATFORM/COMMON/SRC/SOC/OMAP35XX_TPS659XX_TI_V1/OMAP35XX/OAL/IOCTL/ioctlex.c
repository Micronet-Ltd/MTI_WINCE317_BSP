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
//  File:  ioctlex.c
//
//  This file This file implements additional OAL calls.
//
#include <windows.h>
#include <nkintr.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include <args.h>

#include <bsp_version.h>

//------------------------------------------------------------------------------
extern UINT32 OALGetSiliconIdCode();

//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHALPostInit
//
//  
BOOL OALIoCtlHALPostInit(
    UINT32 code, 
    VOID *pInBuffer,
    UINT32 inSize, 
    VOID *ppOutBuffer, 
    UINT32 outSize, 
    UINT32 *pOutSize
    )
{
	manufacture_block* pMBlock;
	card_ver *lcd_ver;
	unsigned short lcd = '4';
	unsigned short *p = (unsigned short *)g_oalIoCtlPlatformOEM;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHALPostInit\r\n"));
    
    OALPowerPostInit();
	
	if(lcd_ver = (card_ver *)OALArgsQuery(OAL_ARGS_QUERY_LCD_CFG))
	{
		if(lcd_ver->config == 'B')
			lcd = '7';
		else
			lcd = '4';
		while(*p)
			p++;
		p--;
		*p = lcd;
	}
	if(pMBlock = (manufacture_block*)OALArgsQuery(OAL_ARGS_QUERY_MUNUFACT))
	{
		UINT32 len = 0;
		if( pMBlock->device_serial_number[0] > 0x20 && pMBlock->device_serial_number[0] < 0x80 )
		{
			while( len <= RECORD_LENGTH && pMBlock->device_serial_number[len]  )
			{
				((wchar_t*)g_oalIoCtlPlatformType)[len] = pMBlock->device_serial_number[len];
				len++;
			}
			((wchar_t*)g_oalIoCtlPlatformType)[len] = 0;
		}
		else
			len = 7;//L"EVM3530" without spaces
		((wchar_t*)g_oalIoCtlPlatformType)[len] = 0;
	}

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"-OALIoCtlHALPostInit(rc = %d)\r\n", TRUE));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalIrq2Sysintr
//
//
BOOL OALIoCtlHalIrq2Sysintr(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalIrq2Sysintr\r\n"));

    // Check input parameters
    if (
        pInpBuffer == NULL || inpSize < sizeof(UINT32) ||
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_IRQ2SYSINTR invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    // Call function itself
    *(UINT32*)pOutBuffer = OALIntrTranslateIrq(*(UINT32*)pInpBuffer);
    if (pOutSize != NULL) *pOutSize = sizeof(UINT32);
   rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalIrq2Sysintr(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetCpuID
//
//
BOOL OALIoCtlHalGetCpuID(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetCpuID\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_CPUID invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU Silicon ID value
    ((DWORD*)pOutBuffer)[0] = OALGetSiliconIdCode();

    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetCpuID(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetDieID
//
//
BOOL OALIoCtlHalGetDieID(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL                        rc = FALSE;
    OMAP_IDCORE_REGS*           pIdRegs;    
    IOCTL_HAL_GET_DIEID_OUT*    pDieID;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetDieID\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(IOCTL_HAL_GET_DIEID_OUT)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_DIEID invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU DIE ID values
    pIdRegs = OALPAtoUA(OMAP_IDCORE_REGS_PA);

    //  Copy to passed in struct
    pDieID = (IOCTL_HAL_GET_DIEID_OUT*)pOutBuffer;
    
    pDieID->dwIdCode    = INREG32(&pIdRegs->IDCODE);
    pDieID->dwProdID_0  = INREG32(&pIdRegs->PRODUCTION_ID_0);
    pDieID->dwProdID_1  = INREG32(&pIdRegs->PRODUCTION_ID_1);
    pDieID->dwProdID_2  = INREG32(&pIdRegs->PRODUCTION_ID_2);
    pDieID->dwProdID_3  = INREG32(&pIdRegs->PRODUCTION_ID_3);
    pDieID->dwDieID_0   = INREG32(&pIdRegs->DIE_ID_0);
    pDieID->dwDieID_1   = INREG32(&pIdRegs->DIE_ID_1);
    pDieID->dwDieID_2   = INREG32(&pIdRegs->DIE_ID_2);
    pDieID->dwDieID_3   = INREG32(&pIdRegs->DIE_ID_3);
    
    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetDieID(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetCpuFamily
//
//
BOOL OALIoCtlHalGetCpuFamily(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetCpuFamily\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_CPUFAMILY invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU family
    ((DWORD*)pOutBuffer)[0] = g_dwCpuFamily;

    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetCpuFamily(rc = %d)\r\n", rc
    ));
    return rc;

}

//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetCpuRevision
//
//
BOOL OALIoCtlHalGetCpuRevision(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetCpuRevision\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(UINT32)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_CPUREVISION invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Get CPU revision
    ((DWORD*)pOutBuffer)[0] = g_dwCpuRevision;

    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL&&OAL_FUNC, (
        L"-OALIoCtlHalGetCpuRevision(rc = %d)\r\n", rc
    ));
    return rc;

}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalI2CMode
//  
//  *pInpBuffer is a pointer to DWORD which has the following values per byte
//  0 = ignore
//  1 = SS mode
//  2 = FS mode
//  3 = HS mode
//  
//
//  | 31-25(i2c4) | 24-16(i2c3) | 15-8(i2c2) | 7-0(i2c1) |
//
BOOL OALIoCtlHalI2CMode(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function: OALIoCtlIgnore
//
BOOL OALIoCtlIgnore(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL  rc = FALSE;

    return rc;
}
//------------------------------------------------------------------------------
#if 0
//------------------------------------------------------------------------------
//
//  Function: OALIoCtlHalGetBspVersion
//
//
BOOL OALIoCtlHalGetBspVersion(UINT32 code, VOID *pInpBuffer,
    UINT32 inpSize, VOID *pOutBuffer, UINT32 outSize, UINT32 *pOutSize)
{
    BOOL                        rc = FALSE;
    IOCTL_HAL_GET_BSP_VERSION_OUT*    pBspVersion;
    
    OALMSG(OAL_IOCTL&&OAL_FUNC, (L"+OALIoCtlHalGetBspVersion\r\n"));

    // Check input parameters
    if (
        pOutBuffer == NULL || outSize < sizeof(IOCTL_HAL_GET_BSP_VERSION_OUT)
    ) {
        NKSetLastError(ERROR_INVALID_PARAMETER);
        OALMSG(OAL_WARN, (
            L"WARN: IOCTL_HAL_GET_BSP_VERSION_OUT invalid parameters\r\n"
        ));
        goto cleanUp;
    }

    //  Copy to passed in struct
    pBspVersion = (IOCTL_HAL_GET_BSP_VERSION_OUT*)pOutBuffer;
    
    pBspVersion->dwVersionMajor    = BSP_VERSION_MAJOR;
    pBspVersion->dwVersionMinor    = BSP_VERSION_MINOR;
    pBspVersion->dwVersionIncremental = BSP_VERSION_INCREMENTAL;
    
    rc = TRUE;

cleanUp:
    OALMSG(OAL_IOCTL && OAL_FUNC, (
        L"-OALIoCtlHalGetBspVersion(rc = %d)\r\n", rc
    ));
    return rc;

}
#endif