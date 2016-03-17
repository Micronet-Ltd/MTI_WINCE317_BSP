//-----------------------------------------------------------------------------
// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  swupdate_api.c
//
//  This file contains access to sw update flags.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <swupdate_api.h>
#include <light_sense.h>
#include <bsp_version.h>
#include <args.h>
#include <swupdate.h>
//------------------------------------------------------------------------------
// Function name	: MIC_GetSwUpdateFlags
// Description	    : Gets SwUpdate flags. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : pointer to Flags 
//                   : Size of Flags 
//------------------------------------------------------------------------------
BOOL MIC_GetSwUpdateFlags( VOID* pFlags, UINT32 Size )
{
	UINT32 RetSize = 0;
	return HalQuerySwUpdateFlags( 0, 0, pFlags, Size, &RetSize );
}


//------------------------------------------------------------------------------
// Function name	: MIC_SetSwUpdateFlags
// Description	    : Set SwUpdate flags. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : pointer to Flags 
//                   : Size of Flags 
//------------------------------------------------------------------------------
BOOL MIC_SetSwUpdateFlags( VOID* pFlags, UINT32 Size )
{
	if( Size < sizeof(sw_update_state))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	sw_update_state Flags = *(sw_update_state*)pFlags;
	
	Flags.bits &= 0x0000FFFF;//low word

	if(	Size					<	MIN_SECUR_SIZE		||
		((UINT32*)pFlags)[1]	!=	SECUR_PASS			)
	{
		Flags.data_type &= (sw_type_logo | sw_type_nk);//if password is not entered - only nk or logo 
	}
	if(!Flags.data_type)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	return HalQuerySwUpdateFlags( (VOID*)&Flags, sizeof(sw_update_state), 0, 0, 0 );
}

//---------------------------------------------------------------------------------------------
BOOL	MIC_GetBspSwVerstion( UINT SwType, BspVersion* pVersion )
{
	DWORD size;

	return KernelIoControl(IOCTL_HAL_GET_BSP_VERSION, &SwType, sizeof(SwType), pVersion, sizeof(BspVersion), &size);
}
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
BOOL	MIC_GetBspSwVersionExt( UINT SwType, VOID* pVersion, DWORD Size, DWORD* pOutSize )
{
	return KernelIoControl(IOCTL_HAL_GET_BSP_VERSION, &SwType, sizeof(SwType), pVersion, Size, pOutSize);
}
//---------------------------------------------------------------------------------------------

BOOL MIC_GetParamBlock( VOID* pBlock, UINT32 Size )
{
	UINT32 RetSize = 0;
	return HalQueryParamBlock( 0, 0, pBlock, Size, &RetSize );
}

//---------------------------------------------------------------------------------------------
HANDLE MIC_LightSenseOpen(void *QueueName, UINT32 len)
{
	if( !QueueName || len < LIGHTSENSE_CONFIG_LEN)
		return INVALID_HANDLE_VALUE;

	return LightSenseOpen(QueueName, len);
}
//---------------------------------------------------------------------------------------------
void	MIC_LightSenseClose(HANDLE hDevice)
{
	if(hDevice != INVALID_HANDLE_VALUE)
		LightSenseClose(hDevice);
}
//---------------------------------------------------------------------------------------------
DWORD	MIC_GetBspVersString(WCHAR* pVersionString, DWORD MaxLength )
{
	if(VERS_STRING_LEN > MaxLength)
		return VERS_STRING_LEN;

	wcscpy_s(pVersionString, VERS_STRING_LEN, BSP_VERSION_STRING);
	pVersionString[VERS_STRING_LEN - 1] = 0;
	return 0;
}



//---------------------------------------------------------------------------------------------
BOOL	MIC_5VSetValue( BOOL bEnable )
{
	return Set5VValue(bEnable);
}


//---------------------------------------------------------------------------------------------
BOOL MIC_IsHWExist(HW_CONFIG_QUERY CfgType, void* pBuff, DWORD OutSize )
{
	DWORD Type = 0, Code = 0, ret = 0, size, Out = 0;
	
	if(!CfgType || !pBuff || !OutSize)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}

	switch(CfgType)
	{
		case QUERY_MAIN_CFG:
			Type = OAL_ARGS_QUERY_MAIN_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_LCD_CFG:
			Type = OAL_ARGS_QUERY_LCD_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_WLAN_CFG:
			Type = OAL_ARGS_QUERY_WLAN_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_CAN_CFG:
			Type = OAL_ARGS_QUERY_CAN_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_GPS_CFG:
			Type = OAL_ARGS_QUERY_GPS_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_MODEM_CFG:
			Type = OAL_ARGS_QUERY_MODEM_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		case QUERY_HW_BOOT_STATUS:
			Type = OAL_ARGS_QUARY_HW_BOOT_STAT;
			Code = IOCTL_HAL_GET_HW_BOOT_STAT;
			size = sizeof(hw_boot_status);
			break;
		case QUERY_EXTAUDIO_CFG:
			Type = OAL_ARGS_QUERY_EXTAUDIO_CFG;
			Code = IOCTL_HAL_QUERY_CARD_CFG;
			size = sizeof(card_ver);
			break;
		default:
			break;
	}
	if(0 == Type || size > OutSize)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return ret;
	}
	HANDLE hProxy = CreateFile(L"PXY1:", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hProxy == INVALID_HANDLE_VALUE)
		return ret;
	
	ret = DeviceIoControl(hProxy, Code, &Type, sizeof(Type), pBuff, size, &Out, 0);
	CloseHandle(hProxy);

	return ret;
}