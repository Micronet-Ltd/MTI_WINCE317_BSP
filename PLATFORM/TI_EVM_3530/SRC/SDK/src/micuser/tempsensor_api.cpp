//-----------------------------------------------------------------------------
// Copyright 2011 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  tempsensor_api.cpp
//
//  This file contains temperature sensor component API code.
//
//  Created by Vitaly Ilyasov    2011
//-----------------------------------------------------------------------------

#include <MicUserSdk.h>
#include "tempsensor.h"

#ifdef BSP_TEMPSENSOR


HANDLE MIC_TemperatureSensorOpen()
{
	return TemperatureSensorOpen();
}

BOOL MIC_TemperatureSensorClose(HANDLE hTempSensor)
{
	return TemperatureSensorClose(hTempSensor);
}

UINT32 MIC_TemperatureSensorGet(HANDLE hTempSensor, TEMPSENSOR_DATA *data)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE || !data)
	{
		return TEMPSENSOR_ERROR_INVALID_PARAMETER;
	}

	if (pContext->isRegistered)
		return TEMPSENSOR_ERROR_QUEUE_REGISTERED;

	if (!DeviceIoControl(pContext->hDevice, IOCTL_TEMPSENSOR_GETDATA, 0, 0, data, sizeof(data), 0, 0))
		return TEMPSENSOR_ERROR_DRIVER_ERROR;

	return TEMPSENSOR_OK;
}

UINT32 MIC_TemperatureSensorGetRawData(HANDLE hTempSensor, DWORD *data)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE || !data)
	{
		return TEMPSENSOR_ERROR_INVALID_PARAMETER;
	}

	if (!DeviceIoControl(pContext->hDevice, IOCTL_TEMPSENSOR_GETRAWDATA, 0, 0, data, sizeof(data), 0, 0))
		return TEMPSENSOR_ERROR_DRIVER_ERROR;

	return TEMPSENSOR_OK;
}

UINT32 MIC_TemperatureSensorForceEnqueue(HANDLE hTempSensor)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE)
		return TEMPSENSOR_ERROR_INVALID_PARAMETER;

	if (!DeviceIoControl(pContext->hDevice, IOCTL_TEMPSENSOR_FORCEENQUEUE, 0, 0, 0, 0, 0, 0))
		return TEMPSENSOR_ERROR_DRIVER_ERROR;

	return TEMPSENSOR_OK;
}

UINT32 MIC_TemperatureSensorRegister(HANDLE hTempSensor, TCHAR *pQueueName, UINT32 *pOutBufferLen)
{
	return TemperatureSensorRegister(hTempSensor, pQueueName, pOutBufferLen);
}

UINT32 MIC_TemperatureSensorDeregister(HANDLE hTempSensor)
{
	return TemperatureSensorDeregister(hTempSensor);
}

#else
HANDLE MIC_TemperatureSensorOpen() { return INVALID_HANDLE_VALUE; }
BOOL MIC_TemperatureSensorClose(HANDLE hTempSensor) { return FALSE; }
UINT32 MIC_TemperatureSensorGet(HANDLE hTempSensor, TEMPSENSOR_DATA *data) { return TEMPSENSOR_ERROR_DRIVER_ERROR; }
UINT32 MIC_TemperatureSensorGetRawData(HANDLE hTempSensor, DWORD *data) { return TEMPSENSOR_ERROR_DRIVER_ERROR; }
UINT32 MIC_TemperatureSensorRegister(HANDLE hTempSensor, TCHAR *pQueueName, UINT32 *pOutBufferLen) { return TEMPSENSOR_ERROR_DRIVER_ERROR; }
UINT32 MIC_TemperatureSensorDeregister(HANDLE hTempSensor) { return TEMPSENSOR_ERROR_DRIVER_ERROR; }
UINT32 MIC_TemperatureSensorForceEnqueue(HANDLE hTempSensor) { return TEMPSENSOR_ERROR_DRIVER_ERROR; }
#endif // BSP_TEMPSENSOR
