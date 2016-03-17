#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <twl.h>
#include <tps659xx.h>
#include <gpio.h>
#include <initguid.h>
#include <madc.h>
#include <pm.h>
#include "tempSensor.h"

#if (BSP_TEMPSENSOR == 1)

#define LOCK	EnterCriticalSection(&tempSensor->cs);
#define UNLOCK	LeaveCriticalSection(&tempSensor->cs);

UINT16 tVoltages[] =    { 1186, 1128, 1064, 997, 927, 856, 785, 716, 648, 584, 524, 468, 416, 370, 328, 291, 258, 228, 202, 179, 159, 141, 126, 112, 100,  46,   0 };
INT16 tTemperatures[] = {  -10,   -5,    0,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95, 100, 105, 110, 115, 120 };
 // 0 = 1064..1128 mV

DWORD MonitorThread(VOID *pParam)
{
	DWORD interval = 0, nullCounter = TEMPSENSOR_NULL_COUNTER;
	UINT16 mV;
	UINT8 backupReg;
	TempSensor_t *tempSensor = (TempSensor_t *)pParam;

	RETAILMSG(1, (L"Temperature sensor : MonitorThread\r\n", __FUNCTION__));

	TEMPSENSOR_DATA msg;
	msg.LimiterTemperature = tempSensor->MaximalTemperature;
	
	HANDLE hMutex = CreateMutex(NULL, FALSE, L"ADCIN0_Mutex");
	if (!hMutex)
	{
		RETAILMSG(1, (L"Temperature sensor : %S, CreateMutex failure\r\n", __FUNCTION__));
		goto CleanUp;
	}

	tempSensor->hA2D = MADCOpen();	
	if (!tempSensor->hA2D)
	{
		RETAILMSG(1, (L"Temperature sensor : %S, MADCOpen failure\r\n", __FUNCTION__));
		goto CleanUp;
	}
	
	//Signal Init Done Event
	SetEvent(tempSensor->hInitDoneEvent);

	while (1)
	{
		DWORD res = WaitForSingleObject(tempSensor->hWaitInterruptEvent, interval);
		if (tempSensor->bNeedExit)
			break;

		if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0)
		{
			ReleaseMutex(hMutex);
			continue;
		}
		DWORD rc = MADCReadValue(tempSensor->hA2D, MADC_CHANNEL_0, &(tempSensor->RawData), 1);
		ReleaseMutex(hMutex);
        if (rc)
		{
			if(tempSensor->bGetRawData)
			{
				tempSensor->bGetRawData = FALSE;
				SetEvent(tempSensor->hGetRawDataEvent);
				continue;
			}

			mV = (UINT16)((TEMPSENSOR_CONVERSION_VALUE * tempSensor->RawData) >> 10);
			RETAILMSG(0, (L"Temperature sensor : rawData = %d, mV = %d\r\n", tempSensor->RawData, mV));
		}

		if (mV < TEMPSENSOR_NULL_THRESHOLD) // board w/o temperature sensor
		{
			if (!nullCounter)
				break;

			interval = TEMPSENSOR_NULL_INTERVAL;
			nullCounter--;
			continue;
		}

		nullCounter = TEMPSENSOR_NULL_COUNTER;

		int i = 0;
		while (mV < tVoltages[i])
			i++;

		if (tempSensor->bOverheatShutdown)
		{
			if (tTemperatures[i] >= tempSensor->MaximalTemperature)
			{
				RETAILMSG(1, (L"Temperature sensor : Overheat shutdown!!!\r\n"));

				TWLReadRegs(tempSensor->hTWL, TWL_BACKUP_REG_G, &backupReg, 1);
				backupReg |= 1;
				TWLWriteRegs(tempSensor->hTWL, TWL_BACKUP_REG_G, &backupReg, 1);
				SetSystemPowerState(NULL, POWER_STATE_OFF, POWER_FORCE);
			}
		}

		if ((tTemperatures[i] != tempSensor->CurrentTemperature) || tempSensor->bForceEnqueue)
		{
			tempSensor->CurrentTemperature = tTemperatures[i];

			if (tempSensor->bForceEnqueue)
			{
				SetEvent(tempSensor->hGetEvent);
				tempSensor->bForceEnqueue = FALSE;
			}

			if (tempSensor->CurrentTemperature >= (tempSensor->MaximalTemperature - 5) && !tempSensor->bForceEnqueue)
			{
				if (tempSensor->hRegKey)
				{
					DWORD dw = ++(tempSensor->CriticalTemperatureCounter);
					RegSetValueEx (tempSensor->hRegKey, L"CriticalTemperatureCounter", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
					dw = tempSensor->MaximalTemperature;
					RegSetValueEx (tempSensor->hRegKey, L"LastLimiter", 0, REG_DWORD, (BYTE *)&dw, sizeof(dw));
					RegFlushKey(tempSensor->hRegKey);
				}
			}

			LOCK
			if (tempSensor->pFirstQueue) 
			{
				msg.CurrentTemperature = tempSensor->CurrentTemperature;
				QueueInfo_t *queue = (QueueInfo_t *)(tempSensor->pFirstQueue);
				RETAILMSG(1, (L"\r\nSending %d'C (%d mV) to queues:\r\n", tempSensor->CurrentTemperature, mV));
				do
				{
					if (queue->hQueue)
					{
						RETAILMSG(1, (L"To %s, prev: %X, next: %X\r\n", queue->QueueName, queue->pPrevious, queue->pNext));
						WriteMsgQueue(queue->hQueue, &msg, sizeof(msg), 0, 0);
					}
				}
				while (queue = (QueueInfo_t *)(queue->pNext));
			}
			UNLOCK
		}

		if (tTemperatures[i] > TEMPSENSOR_NORMAL_TEMPERATURE)
			interval = TEMPSENSOR_HIGH_INTERVAL;
		else
			interval = TEMPSENSOR_NORMAL_INTERVAL;
	}

CleanUp:;
	if (tempSensor->hA2D)
	{
		MADCClose(tempSensor->hA2D);
		tempSensor->hA2D = NULL;
	}

	if (hMutex)
		CloseHandle(hMutex);

	// release resources
	CloseHandle(tempSensor->hWaitInterruptEvent);
	CloseHandle(tempSensor->hGetEvent);
	CloseHandle(tempSensor->hGetRawDataEvent);
	RegCloseKey(tempSensor->hRegKey);
	DeleteCriticalSection(&tempSensor->cs);

	RETAILMSG(1, (L"Temperature sensor : Monitor thread down (nullCounter = %d of %d, interval = %d ms).\r\n", nullCounter, TEMPSENSOR_NULL_COUNTER, TEMPSENSOR_NULL_INTERVAL));
	return 0;
}

HANDLE TempSensorQueueCreate(TCHAR *name)
{
	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(TEMPSENSOR_DATA);
	msgOptions.bReadAccess	= 0;
	return CreateMsgQueue(name, &msgOptions);
}

QueueInfo_t *TempSensorQueueAlloc(TempSensor_t *tempSensor)
{
	QueueInfo_t *queue, *newQueue;

	newQueue = (QueueInfo_t *)LocalAlloc(LPTR, sizeof(QueueInfo_t));
	if (!newQueue)
	{
		SetLastError(ERROR_OUTOFMEMORY);
		return 0;
	}

	memset(newQueue, 0, sizeof(newQueue));
	
	if (tempSensor->pFirstQueue)
	{
		queue = (QueueInfo_t *)(tempSensor->pFirstQueue);
		while (queue->pNext)
			queue = (QueueInfo_t *)(queue->pNext);

		queue->pNext = newQueue;
		newQueue->pPrevious = queue;
	}
	else
		tempSensor->pFirstQueue = newQueue;

	newQueue->pTempSensor = tempSensor;
	wsprintf(newQueue->QueueName, L"%s%X", TEMPSENSOR_MSGQUEUENAME, newQueue);
	newQueue->dwProcessId = GetProcessId(GetOwnerProcess());
	newQueue->hQueue = NULL;
	RETAILMSG(1, (L"New queue: %s, prev: %X, next: %X\r\n", newQueue->QueueName, newQueue->pPrevious, newQueue->pNext));

	return newQueue;
}

QueueInfo_t *GetOwnerQueue(TempSensor_t *tempSensor)
{
	DWORD dwOwnerProcessId = GetProcessId(GetOwnerProcess());

	QueueInfo_t *pQueue = (QueueInfo_t *)(tempSensor->pFirstQueue);
	while (pQueue)
	{
		if (pQueue->dwProcessId == dwOwnerProcessId)
			break;

		pQueue = (QueueInfo_t *)(pQueue->pNext);
	}
	
	return pQueue;
}

BOOL TempSensorQueueDealloc(TempSensor_t *tempSensor)
{
	DWORD dwOwnerProcessId = GetProcessId(GetOwnerProcess());

	QueueInfo_t *pQueue = GetOwnerQueue(tempSensor);
	
	if (!pQueue)
		return FALSE;

	if (pQueue->pPrevious || pQueue->pNext)
	{
		if (pQueue->pPrevious)
			((QueueInfo_t *)(pQueue->pPrevious))->pNext = pQueue->pNext;
		else
			tempSensor->pFirstQueue = pQueue->pNext;

		if (pQueue->pNext)
			((QueueInfo_t *)(pQueue->pNext))->pPrevious = pQueue->pPrevious;
	}
	else
		tempSensor->pFirstQueue = NULL;

	if (pQueue->hQueue)
		CloseMsgQueue(pQueue->hQueue);

	LocalFree(pQueue);
	return TRUE;
}

void TempSensorClose(TempSensor_t *tempSensor)
{
	LOCK;
	TempSensorQueueDealloc(tempSensor);
	UNLOCK;
}

void TempSensorInit(TempSensor_t *tempSensor, HANDLE hTriton)
{
    WCHAR szEvevntName[MAX_PATH];

	if (!tempSensor->hMonitorThread)
	{
		tempSensor->bOverheatShutdown = TRUE;
		tempSensor->MaximalTemperature = TEMPSENSOR_CRITICAL_TEMPERATURE;
		tempSensor->CriticalTemperatureCounter = 0;

		tempSensor->hRegKey = NULL;
		LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEMPSENSOR_REG_KEY, 0, 0, &tempSensor->hRegKey);
		if (ERROR_SUCCESS == lStatus)
		{	
			DWORD dwValue;
			DWORD dwSize = sizeof(DWORD);
			DWORD dwValType = REG_DWORD;

			lStatus = RegQueryValueEx(tempSensor->hRegKey, TEXT("LimiterTemperature"), NULL, &dwValType, (LPBYTE)&dwValue, &dwSize);
			if (ERROR_SUCCESS == lStatus)
			{
				if (dwValue >= TEMPSENSOR_HIGH_TEMPERATURE && dwValue <= TEMPSENSOR_CRITICAL_TEMPERATURE)
					tempSensor->MaximalTemperature = (INT16)dwValue;
			}

			lStatus = RegQueryValueEx(tempSensor->hRegKey, TEXT("OverheatShutdown"), NULL, &dwValType, (LPBYTE)&dwValue, &dwSize);
			if (ERROR_SUCCESS == lStatus)
			{
				if (dwValue == 0)
					tempSensor->bOverheatShutdown = FALSE;
			}

			lStatus = RegQueryValueEx(tempSensor->hRegKey, TEXT("CriticalTemperatureCounter"), NULL, &dwValType, (LPBYTE)&dwValue, &dwSize);
			if (ERROR_SUCCESS == lStatus)
			{
				tempSensor->CriticalTemperatureCounter = dwValue;
			}

			dwSize = sizeof(szEvevntName);
			lStatus = RegQueryValueEx(tempSensor->hRegKey, TEXT("InitDoneEventName"), NULL, NULL, (LPBYTE)szEvevntName, &dwSize);
			if (ERROR_SUCCESS != lStatus)
			{
				RETAILMSG(0, (L"Temperature sensor Error acquiring: InitDoneEventName\r\n"));
			}
		}

		tempSensor->CurrentTemperature = -111;
		tempSensor->bNeedExit = FALSE;
		tempSensor->bForceEnqueue = FALSE;
		tempSensor->bGetRawData = FALSE;
		InitializeCriticalSection(&tempSensor->cs);
		tempSensor->hWaitInterruptEvent = CreateEvent(0, 0, 0, 0);
		tempSensor->hGetEvent = CreateEvent(0, 0, 0, 0);
		tempSensor->hGetRawDataEvent = CreateEvent(0, 0, 0, 0);
		tempSensor->hTWL = hTriton;
		//Create Init Done Event
		tempSensor->hInitDoneEvent = CreateEvent(NULL, TRUE, FALSE, szEvevntName);

		tempSensor->hMonitorThread = CreateThread(0, 0, MonitorThread, tempSensor, 0, 0);
	}
}



void TempSensorDeinit(TempSensor_t *tempSensor)
{
	if (tempSensor->hMonitorThread != NULL)
	{
		tempSensor->bNeedExit = TRUE;
		SetEvent(tempSensor->hWaitInterruptEvent);
		WaitForSingleObject(tempSensor->hMonitorThread, INFINITE);
		CloseHandle(tempSensor->hMonitorThread);
		tempSensor->hMonitorThread = NULL;
	}
}

void TempSensorGetCurrent(TempSensor_t *tempSensor, TEMPSENSOR_DATA *data)
{
	tempSensor->bForceEnqueue = TRUE;
	SetEvent(tempSensor->hWaitInterruptEvent);
	WaitForSingleObject(tempSensor->hGetEvent, 1000);
	data->CurrentTemperature = tempSensor->CurrentTemperature;
	data->LimiterTemperature = tempSensor->MaximalTemperature;
}

BOOL TempSensorGetRawData(TempSensor_t *tempSensor, DWORD *rawData)
{
	tempSensor->bGetRawData = TRUE;
	SetEvent(tempSensor->hWaitInterruptEvent);
	if (WaitForSingleObject(tempSensor->hGetRawDataEvent, TEMPSENSOR_NORMAL_INTERVAL) != WAIT_OBJECT_0)
		return FALSE;

	*rawData = tempSensor->RawData;
	return TRUE;
}

void TempSensorForceEnqueue(TempSensor_t *tempSensor)
{
	tempSensor->bForceEnqueue = TRUE;
	SetEvent(tempSensor->hWaitInterruptEvent);
}

BOOL TempSensorIOControl(TempSensor_t *tempSensor, DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
	BOOL rc = FALSE;
	QueueInfo_t *pQueue;

	switch (code)
	{
		case IOCTL_TEMPSENSOR_REGISTERQUEUE:
		{
			LOCK;
			pQueue = GetOwnerQueue(tempSensor);

			if (pQueue)
			{
				UNLOCK;
				SetLastError(ERROR_ALREADY_REGISTERED);
				break;
			}

			DWORD len = sizeof(pQueue->QueueName);
			RETAILMSG(0, (L"Temperature sensor : %S : len=%d\r\n", __FUNCTION__, len));
			if (pOutSize)
				*pOutSize = len;

			if (outSize < len)
			{
				UNLOCK;
				SetLastError(ERROR_BUFFER_OVERFLOW);
				break;
			}

			if (!pOutBuffer)
			{
				UNLOCK;
				SetLastError(ERROR_INVALID_PARAMETER);
				break;
			}

			pQueue = TempSensorQueueAlloc(tempSensor);
			rc = CeSafeCopyMemory(pOutBuffer, pQueue->QueueName, len);
			pQueue->hQueue = TempSensorQueueCreate(pQueue->QueueName);
			TempSensorForceEnqueue(tempSensor);
			UNLOCK;
		}
		break;

		case IOCTL_TEMPSENSOR_DEREGISTERQUEUE:
		{
			LOCK;
			rc = TempSensorQueueDealloc(tempSensor);
			UNLOCK;

			if (!rc)
				SetLastError(ERROR_SERVICE_NOT_FOUND);
		}
		break;

		case IOCTL_TEMPSENSOR_GETDATA:
		{
			if (!pOutBuffer || outSize < sizeof(TEMPSENSOR_DATA))
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				break;
			}

			TempSensorGetCurrent(tempSensor, (TEMPSENSOR_DATA *)pOutBuffer);
			rc = TRUE;
		}
		break;

		case IOCTL_TEMPSENSOR_GETRAWDATA:
		{
			if (!pOutBuffer || outSize < sizeof(DWORD))
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				break;
			}

			rc = TempSensorGetRawData(tempSensor, (DWORD *)pOutBuffer);
		}
		break;

		case IOCTL_TEMPSENSOR_FORCEENQUEUE:
		{
			TempSensorForceEnqueue(tempSensor);
			rc = TRUE;
		}
		break;

		case IOCTL_TEMPSENSOR_OPENCONTEXT:
		{
			if (!tempSensor->hA2D)
			{
				SetLastError(ERROR_OPEN_FAILED);
				break;
			}
			
			rc = TRUE;
		}
		break;
	}

	return rc;
}
#endif