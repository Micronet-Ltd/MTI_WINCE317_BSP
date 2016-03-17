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
//  File: light_sense.h
//

#ifndef __LIGHTSENS_H_
#define __LIGHTSENS_H_

#define LIGHTE_SENSE_SAMPLES	20
#define LIGHT_SENSE_ACCURACY	25	//mV
#define LIGHTSENSE_CONFIG		L"Update Light Sense Config"
#define LIGHTSENSE_CONFIG_LEN	64			// bytes
#define LIGHT_SENSE_REG_KEY     L"Drivers\\BuiltIn\\ltsr"
#define LIGHT_SENSE_REG_HL      L"HighLight"
#define LIGHT_SENSE_REG_LL      L"LowLight"

//------------------------------------------------------------------------------

#define IOCTL_LTSR_GETQUEUENAME CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)

#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
	AMBIENT_LIGHT_UNSPEC,
	AMBIENT_LIGHT_LOW,
	AMBIENT_LIGHT_NORMAL,
	AMBIENT_LIGHT_HIGH,
	AMBIENT_LIGHT_INVALID
}AMBIENT_LIGHT;

//------------------------------------------------------------------------------
//
//  Functions: LightSense
//
__inline HANDLE LightSenseOpen(void *QueueName, UINT32 len)
{
    HANDLE hDevice;

	hDevice = CreateFile(L"LTS1:", 0, 0, 0, 0, 0, 0);
    if(hDevice == INVALID_HANDLE_VALUE)
		return 0;


    // Get Queue Name
    if(!DeviceIoControl(hDevice, IOCTL_LTSR_GETQUEUENAME, hDevice, sizeof(hDevice), QueueName, len, 0, 0))
	{
        CloseHandle(hDevice);
		return 0;
	}

    return hDevice;
}

__inline VOID LightSenseClose(HANDLE hContext)
{
    CloseHandle(hContext);
}
#ifdef __cplusplus
}
#endif

#endif //__LIGHTSENS_H_
//-----------------------------------------------------------------------------

