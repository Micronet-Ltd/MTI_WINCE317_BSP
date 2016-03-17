// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vitaly Iliasov
//
#ifndef __TEMPSENSOR_H
#define __TEMPSENSOR_H

#include <windows.h>
#include "tempsensor_api.h"

#define TEMPSENSOR_MSGQUEUENAME			L"TempSensorMsgQueue"
#define PWRKEY_DEVICE_NAME					L"PKD1:"

#define TEMPSENSOR_NORMAL_INTERVAL		1000
#define TEMPSENSOR_NORMAL_TEMPERATURE	75
#define TEMPSENSOR_HIGH_INTERVAL		200
#define TEMPSENSOR_HIGH_TEMPERATURE		80
#define TEMPSENSOR_CRITICAL_TEMPERATURE	110
#define TEMPSENSOR_REG_KEY				L"\\Drivers\\BuiltIn\\pwrkey"
#define TEMPSENSOR_NULL_COUNTER			1000
#define TEMPSENSOR_NULL_INTERVAL		0
#define TEMPSENSOR_NULL_THRESHOLD		10

#define TEMPSENSOR_DEVICE_COOKIE        'tsen'

#define TEMPSENSOR_CONVERSION_VALUE		1501	// ((1.5 / 1023) << 10) * 1000

#define IOCTL_TEMPSENSOR_REGISTERQUEUE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TEMPSENSOR_DEREGISTERQUEUE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TEMPSENSOR_GETDATA			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TEMPSENSOR_FORCEENQUEUE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TEMPSENSOR_OPENCONTEXT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TEMPSENSOR_GETRAWDATA			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)

//Static library functions serve both external API and kernel drivers
HANDLE TemperatureSensorOpen();
BOOL   TemperatureSensorClose(HANDLE hTempSensor);
UINT32 TemperatureSensorRegister(HANDLE hTempSensor, TCHAR *pQueueName, UINT32 *pOutBufferLen);
UINT32 TemperatureSensorDeregister(HANDLE hTempSensor);

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct 
{
	DWORD	Cookie;
    HANDLE  hDevice;
	BOOL	isRegistered;

} TEMPSENSOR_DEVICE_CONTEXT;

typedef struct _TempSensor_t
{
	CRITICAL_SECTION cs;
	HANDLE		hMonitorThread;
	HANDLE		hA2D;
	HANDLE		hWaitInterruptEvent;
	HANDLE		hGetEvent;
	HANDLE		hGetRawDataEvent;
	HANDLE		hTWL;
	HANDLE		hInitDoneEvent;
	HKEY		hRegKey;
	BOOL		bNeedExit;
	BOOL		bOverheatShutdown;
	BOOL		bForceEnqueue;
	BOOL		bGetRawData;
	INT16		MaximalTemperature;
	INT16		CurrentTemperature;
	DWORD		RawData;
	DWORD		CriticalTemperatureCounter;
	void		*pFirstQueue;
} TempSensor_t;

typedef struct _QueueInfo_t
{
	DWORD			dwProcessId;
	HANDLE			hQueue;
	TCHAR			QueueName[64];
	TempSensor_t	*pTempSensor;
	void			*pPrevious;
	void			*pNext;
} QueueInfo_t;

void TempSensorInit(TempSensor_t *pTempSensor, HANDLE hTriton);
void TempSensorDeinit(TempSensor_t *pTempSensor);
void TempSensorClose(TempSensor_t *tempSensor);
BOOL TempSensorIOControl(TempSensor_t *tempSensor, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize);

#ifdef __cplusplus
}
#endif

#endif
