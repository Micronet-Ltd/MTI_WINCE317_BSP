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
//  File:  base.c
//
//  This file implements CE DDK DLL entry function.
//
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <storemgr.h>
#include <fatutil.h>
#include <oal.h>
#include <oalex.h>
#include <args.h>
#include <gpio.h>



//------------------------------------------------------------------------------
//
BOOL HalQueryFormatPartition(VOID *pOutBuffer, UINT32 outSize)
{
	void *hs, *hp;
	BOOL ret = 0;

	hs = OpenStore(TEXT("NFD"));
	if((HANDLE)-1 != hs)
	{
		RETAILMSG(1, (L"Open 'Part01' partition\r\n"));
		hp = OpenPartition(hs, TEXT("Part01"));
		if((HANDLE)-1 != hs)
		{
			ret = DeviceIoControl(hp, IOCTL_FMD_QUERY_FORMAT, 0, 0, pOutBuffer, outSize, &ret, 0);
			CloseHandle(hp);
		}
		CloseHandle(hs);
	}

	return ret;
}

//------------------------------------------------------------------------------
//
BOOL Set5VValue(BOOL bEnable)
{
	HANDLE hGpio;
	DWORD rc     = FALSE;
	DWORD gpio5V;

	RETAILMSG(0, (L"+%S, 5V Enable = %d\r\n",__FUNCTION__, bEnable)); 

  {
	card_ver mbver;
	UINT32 in = OAL_ARGS_QUERY_MAIN_CFG;
	BOOL ret = 0;
	HANDLE hProxy = NULL;

	mbver.card_addr	= 0;
	mbver.ver		= 0;
	mbver.rev		= 0;
	mbver.config	= 0;
	hProxy = CreateFile(L"PXY1:", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

    if(hProxy != INVALID_HANDLE_VALUE)
	{
		ret = DeviceIoControl(hProxy, IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &mbver, sizeof(mbver), 0, 0 );
		RETAILMSG(0, (L"HalQueryHwMainCfg ret %x\r\n", ret));
	}
	else
	{
      RETAILMSG(1, (L"%S: Proxy Error.., err = %d\r\n",__FUNCTION__, GetLastError() ));
      return(FALSE); 
	}

	CloseHandle(hProxy);

	if( !ret )
	     {
          RETAILMSG(1, (L"%S: IOCTL_HAL_QUERY_CARD_CFG calling failure.., err = %d\r\n",__FUNCTION__, GetLastError() ));
	      return(FALSE);
	     }
		else
		 {
          RETAILMSG(0, (L"%S, board ver = %d, rev = %d\r\n",__FUNCTION__, mbver.ver, mbver.rev + 1)); 
		 }

	if(mbver.rev < 2)
	{
		gpio5V = GPIO_58;
	}
	else
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return 0;
	}
  }
	

	// open handle to gpio driver
    hGpio = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);

    if (hGpio != INVALID_HANDLE_VALUE)
    {
		RETAILMSG(0, (L"%S, 5V On = %d\r\n",__FUNCTION__, bEnable));
		if (bEnable)
		{
			rc = DeviceIoControl(hGpio, IOCTL_GPIO_SETBIT, &gpio5V,sizeof(gpio5V), 0, 0, NULL, NULL );

		}
		else
		{
			rc = DeviceIoControl(hGpio, IOCTL_GPIO_CLRBIT, &gpio5V,sizeof(gpio5V), 0, 0, NULL, NULL );
		}
    }

	
    CloseHandle(hGpio);

	RETAILMSG(0, (L"-%S, ret = %d\r\n",__FUNCTION__, rc)); 
	return rc;
}

//------------------------------------------------------------------------------
//
BOOL HalQuerySwUpdateFlags(VOID *pInBuffer, UINT32 InSize, VOID *pOutBuffer, UINT32 outSize, UINT32* RetSize )
{
	BOOL ret = 0;
	HANDLE hProxy = NULL;
		
	RETAILMSG(0, (L"HalQuerySwUpdateFlags: %x  %x  %x  %x  %x\r\n", pInBuffer, InSize, pOutBuffer, outSize, RetSize));

	hProxy = CreateFile(L"PXY1:", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

    if(hProxy != INVALID_HANDLE_VALUE)
	{
		ret = DeviceIoControl(hProxy, IOCTL_HAL_SW_UPDATE_FLAG,pInBuffer, InSize, pOutBuffer, outSize, RetSize, 0 );
		RETAILMSG(0, (L"HalQuerySwUpdateFlags ret %x\r\n", ret));
	}

	CloseHandle(hProxy);

	return ret;
}
//------------------------------------------------------------------------------
//
BOOL HalQueryParamBlock(VOID *pInBuffer, UINT32 InSize, VOID *pOutBuffer, UINT32 outSize, UINT32* RetSize )
{
	BOOL ret = 0;
	HANDLE hProxy = NULL;
		
	RETAILMSG(0, (L"HalQueryParamBlock: %x  %x  %x  %x  %x\r\n", pInBuffer, InSize, pOutBuffer, outSize, RetSize));

	hProxy = CreateFile(L"PXY1:", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

    if(hProxy != INVALID_HANDLE_VALUE)
	{
		ret = DeviceIoControl(hProxy, IOCTL_HAL_PARAM_BLOCK, pInBuffer, InSize, pOutBuffer, outSize, RetSize, 0 );
		RETAILMSG(0, (L"HalQueryParamBlock ret %x\r\n", ret));
	}

	CloseHandle(hProxy);

	return ret;
}
//------------------------------------------------------------------------------
//
BOOL HalQueryHwBootStat(VOID *pInBuffer, UINT32 InSize, VOID *pOutBuffer, UINT32 outSize, UINT32* RetSize )
{
	BOOL ret = 0;
	HANDLE hProxy = NULL;
		
	RETAILMSG(0, (L"HalQueryHwBootStat: %x  %x  %x  %x  %x\r\n", pInBuffer, InSize, pOutBuffer, outSize, RetSize));

	hProxy = CreateFile(L"PXY1:", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

    if(hProxy != INVALID_HANDLE_VALUE)
	{
		ret = DeviceIoControl(hProxy, IOCTL_HAL_GET_HW_BOOT_STAT, pInBuffer, InSize, pOutBuffer, outSize, RetSize, 0 );
		RETAILMSG(0, (L"HalQueryHwBootStat ret %x\r\n", ret));
	}

	CloseHandle(hProxy);

	return ret;
}

//------------------------------------------------------------------------------
//


//------------------------------------------------------------------------------
// 
//  Function:  DllEntry
//
//  The DDL entry function
//
BOOL WINAPI
//DllEntry(
DllMain(
    HANDLE hInstance,
    DWORD reason,
    LPVOID pvReserved
    )
{
    if (reason == DLL_PROCESS_ATTACH)
        {
        DisableThreadLibraryCalls((HMODULE)hInstance);
        }
    return TRUE;
}

//------------------------------------------------------------------------------
