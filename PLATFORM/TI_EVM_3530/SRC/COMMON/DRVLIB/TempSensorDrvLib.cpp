#include <tempsensor.h>

HANDLE TemperatureSensorOpen()
{
	HANDLE hDevice;
	TEMPSENSOR_DEVICE_CONTEXT *pContext = NULL; 
	RETAILMSG(1, (L"+TemperatureSensorOpen\r\n"));

	hDevice = CreateFile(PWRKEY_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) 
		return INVALID_HANDLE_VALUE;
	
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1, (L"TemperatureSensorOpen: Error opening TempSensor Event Handle. Error= \r\n", GetLastError()));
		return INVALID_HANDLE_VALUE;
	}

	if (!DeviceIoControl(hDevice, IOCTL_TEMPSENSOR_OPENCONTEXT, 0, 0, 0, 0, 0, 0))
	{
		CloseHandle(hDevice);
		return INVALID_HANDLE_VALUE;
	}

	pContext = (TEMPSENSOR_DEVICE_CONTEXT *)LocalAlloc(LPTR, sizeof(TEMPSENSOR_DEVICE_CONTEXT));
	RETAILMSG(1, (L"TemperatureSensorOpen : pContext=0x%x\r\n", pContext));
	if (pContext == NULL)
	{
		CloseHandle(hDevice);
		return INVALID_HANDLE_VALUE;
	}

	pContext->hDevice = hDevice;
	pContext->Cookie = TEMPSENSOR_DEVICE_COOKIE;
	pContext->isRegistered = FALSE;

	RETAILMSG(1, (L"-TemperatureSensorOpen\r\n"));
	return pContext;
}

BOOL TemperatureSensorClose(HANDLE hTempSensor)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;
	BOOL ret;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	ret = CloseHandle(pContext->hDevice);
	LocalFree(pContext);
	return ret;
}

UINT32 TemperatureSensorRegister(HANDLE hTempSensor, TCHAR *pQueueName, UINT32 *pOutBufferLen)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE || !pQueueName || !pOutBufferLen)
		return TEMPSENSOR_ERROR_INVALID_PARAMETER;

	if (pContext->isRegistered)
		return TEMPSENSOR_ERROR_ALREADY_REGISTERED;

	if (!DeviceIoControl(pContext->hDevice, IOCTL_TEMPSENSOR_REGISTERQUEUE, 0, 0, pQueueName, *pOutBufferLen, (DWORD *)pOutBufferLen, 0))
	{
		DWORD err = GetLastError();

		if (err == ERROR_BUFFER_OVERFLOW)
			return TEMPSENSOR_ERROR_BUFFER_OVERFLOW;
		else if (err == ERROR_ALREADY_REGISTERED)
			return TEMPSENSOR_ERROR_ALREADY_REGISTERED;

		return TEMPSENSOR_ERROR_DRIVER_ERROR;
	}

	pContext->isRegistered = TRUE;
	return TEMPSENSOR_OK;
}

UINT32 TemperatureSensorDeregister(HANDLE hTempSensor)
{
	TEMPSENSOR_DEVICE_CONTEXT *pContext = (TEMPSENSOR_DEVICE_CONTEXT *)hTempSensor;

	if (pContext == INVALID_HANDLE_VALUE || pContext->Cookie != TEMPSENSOR_DEVICE_COOKIE)
		return TEMPSENSOR_ERROR_INVALID_PARAMETER;

	if (!pContext->isRegistered)
		return TEMPSENSOR_ERROR_NOT_REGISTERED;

	if (!DeviceIoControl(pContext->hDevice, IOCTL_TEMPSENSOR_DEREGISTERQUEUE, 0, 0, 0, 0, 0, 0))
	{
		DWORD err = GetLastError();

		if (err == ERROR_SERVICE_NOT_FOUND)
			return TEMPSENSOR_ERROR_NOT_REGISTERED;

		return TEMPSENSOR_ERROR_DRIVER_ERROR;
	}

	pContext->isRegistered = FALSE;
	return TEMPSENSOR_OK;
}