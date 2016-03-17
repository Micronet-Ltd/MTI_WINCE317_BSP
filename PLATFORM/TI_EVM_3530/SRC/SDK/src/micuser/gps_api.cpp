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
#include <gps_api.h>
#include <gps.h>
#include <devload.h>

//---------------------------------------------------------------------------------------------
INT32 MIC_GPSPower(INT32 power)
{
	INT32	ret = 0;
	HANDLE	hDevice;
	DWORD	PwrState = D4;
	DWORD	OutLen;

	RETAILMSG(0, (L"+MIC_GPSPower\r\n"));
	
	hDevice = CreateFile(GPS_PORT_NAME, DEVACCESS_BUSNAMESPACE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if((HANDLE)-1 == hDevice)
	{
		RETAILMSG(1, (L"MIC_GPSPower: CreateFile(GPS_DEVNAME) failed\r\n"));
		return ret;
	}
	if(power)
		PwrState = D0;

	if(!(ret = DeviceIoControl (hDevice, IOCTL_POWER_SET, (LPVOID)&power, sizeof(power), (LPVOID)&PwrState, sizeof(PwrState), &OutLen, NULL )))
	{
		PwrState = D4;
		power	 = 0;
		DeviceIoControl (hDevice, IOCTL_POWER_SET, (LPVOID)&power, sizeof(power), (LPVOID)&PwrState, sizeof(PwrState), &OutLen, NULL );
		RETAILMSG(1, (L"MIC_GPSPower: MIC_GPSPower failed err %d\r\n", GetLastError()));
	}

	CloseHandle(hDevice);

	RETAILMSG(0, (L"-MIC_GPSPower ret %d\r\n", ret));
	return ret;
}
//---------------------------------------------------------------------------------------------
INT32 MIC_GPSGetPowerStatus(INT32* pPower)
{
	INT32	ret = 0;
	INT32	power = 0;
	HANDLE	hDevice;
	DWORD   BufIn = 1;
	
	hDevice = CreateFile(GPS_PORT_NAME, DEVACCESS_BUSNAMESPACE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if((HANDLE)-1 == hDevice)
	{
		RETAILMSG(0, (L"MIC_GPSGetPowerStatus: CreateFile(GPS_DEVNAME) failed\r\n"));
		return ret;
	}

	ret = DeviceIoControl(hDevice, IOCTL_POWER_GET, (LPVOID)&BufIn, sizeof(BufIn), (LPVOID)&power, sizeof(power), 0, NULL ); 

	CloseHandle(hDevice);
	
	if(ret)
	{
		switch(power)
		{
			case	D0:	
			case	D1:	
			case	D2:	
				*pPower = 1;
			break;
			case	D3:	
			case	D4:	
				*pPower = 0;
			break;
			default:
				ret = 0;
			break;
		}
	}			
	
	return ret;
}