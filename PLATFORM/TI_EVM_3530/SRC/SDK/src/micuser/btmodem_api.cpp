//-----------------------------------------------------------------------------
// Copyright 2011 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  btmodem_api.cpp
//
//  This file contains btmodem component API code.
//
//  Created by Vitaly Ilyasov    2011
//-----------------------------------------------------------------------------



#include <MicUserSdk.h>
#include <service.h>
#include <btmodem.h>

#ifdef SYSGEN_BTH_MODEM

UINT32 MIC_BTModemNotifyRegister(HANDLE hService)
{
	if (hService == INVALID_HANDLE_VALUE)
		return BTMODEM_ERROR_INVALID_PARAMETER;

	DWORD status = BTMCOMMAND_CREATE_MSG_QUEUE;
	if (DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &status, sizeof(status), NULL, 0, 0, 0))
		return BTMODEM_ERROR_SUCCESS;

	if (GetLastError() == ERROR_ALREADY_REGISTERED)
		return BTMODEM_ERROR_ALREADY_REGISTERED;

	return BTMODEM_ERROR_REGISTRATION_ERROR;
}

UINT32 MIC_BTModemDeactivate(HANDLE hService)
{
	if (hService == INVALID_HANDLE_VALUE)
		return BTMODEM_ERROR_INVALID_PARAMETER;

	UINT32 ret = BTMODEM_ERROR_SUCCESS;

	DWORD status = BTMCOMMAND_DEACTIVATE;
	if (!DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &status, sizeof(status), NULL, 0, 0, 0))
	{
		status = BTMCOMMAND_GET_EXTENDED_ERROR;
		if (!DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &status, sizeof(status), &ret, sizeof(ret), 0, 0))
			return BTMODEM_ERROR_DRIVER_ERROR;
	}

	return ret;
}

UINT32 MIC_BTModemActivate(HANDLE hService, TCHAR *modemName)
{
	UINT32 ret = BTMODEM_ERROR_SUCCESS;
	DWORD err = 0, status = SERVICE_STATE_UNKNOWN;
	if (hService == INVALID_HANDLE_VALUE || modemName == NULL)
		return BTMODEM_ERROR_INVALID_PARAMETER;

	BTMODEM_CONTEXT btContext = {0};
	DWORD cmd = BTMCOMMAND_GET_CONTEXT;
	if (!DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &cmd, sizeof(cmd), &btContext, sizeof(btContext), 0, 0))
		return BTMODEM_ERROR_DRIVER_ERROR;

	wcsncpy(btContext.ModemName, modemName, _tcslen(modemName) + 1);

	cmd = BTMCOMMAND_SET_CONTEXT;
	if (!DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &cmd, sizeof(cmd), &btContext, sizeof(btContext), 0, 0))
		return BTMODEM_ERROR_DRIVER_ERROR;

	cmd = BTMCOMMAND_ACTIVATE;
	if (DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &cmd, sizeof(cmd), NULL, 0, 0, 0))
	{
		DWORD i = 0;
		do
		{
			if (!DeviceIoControl(hService, IOCTL_SERVICE_STATUS, 0, 0, &status, sizeof(status), 0, 0))
			{
				ret = BTMODEM_ERROR_ACTIVATING_ERROR;
				goto error;
			}

			Sleep(SERVICE_UP_TIMEOUT_VALUE);
		}
		while (status == SERVICE_STATE_STARTING_UP && i++ < SERVICE_UP_TIMEOUT_COUNTER);
	}

	if (status != SERVICE_STATE_ON)
	{
		cmd = BTMCOMMAND_GET_EXTENDED_ERROR;
		if (!DeviceIoControl(hService, IOCTL_SERVICE_CONTROL, &cmd, sizeof(cmd), &err, sizeof(err), 0, 0))
		{
			ret = BTMODEM_ERROR_UNKNOWN_ERROR;
			goto error;
		}

		ret = (UINT32)err;
		goto error;
	}

	return BTMODEM_ERROR_SUCCESS;

error:;
	MIC_BTModemDeactivate(hService);
	return ret;
}

#else
UINT32 MIC_BTModemNotifyRegister(HANDLE hService) { return BTMODEM_ERROR_DRIVER_ERROR; }
UINT32 MIC_BTModemDeactivate(HANDLE hService) { return BTMODEM_ERROR_DRIVER_ERROR; }
UINT32 MIC_BTModemActivate(HANDLE hService, TCHAR *modemName) { return BTMODEM_ERROR_DRIVER_ERROR; }
#endif // SYSGEN_BTH_MODEM
