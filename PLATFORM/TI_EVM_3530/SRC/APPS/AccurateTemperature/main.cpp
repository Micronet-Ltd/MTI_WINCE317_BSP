#include <windows.h>
#include <commctrl.h>
#include "MicUserSdk.h"
#include "console.h"

#define TEMPSENSOR_CONVERSION_VALUE		1501	// ((1.5 / 1023) << 10) * 1000

void Log(TCHAR *s, ...);
UINT32 MIC_TemperatureSensorGetRawData(HANDLE hTempSensor, DWORD *data);

BOOL IsConsole()
{
	DWORD mode;
	DWORD bytesReturned;
	return DeviceIoControl(_fileno(stdout), IOCTL_CONSOLE_GETMODE, 0, 0, &mode, sizeof(mode), &bytesReturned, 0);
}

int interval_sec = 60; // every minute

int _tmain(int argc, _TCHAR* argv[])
{
	Log(L"Usage: AccurateTemperature.exe <interval in sec>");
	Log(L"Log output to COM2");
	
	if (argc > 1)
	{
		int arg1 = _wtoi(argv[1]);
		if (arg1 < 2)
			Log(L"Invalid interval parameter. Must be 2 sec or longer" , interval_sec);
		else
			interval_sec = arg1;
	}

	Log(L"Interval = %d sec." , interval_sec);

	HANDLE hTempSensor = MIC_TemperatureSensorOpen();
	if (hTempSensor == INVALID_HANDLE_VALUE)
	{
		Log(L"Can't open temperature sensor device. Error = %d", GetLastError());
		goto error;
	}

	while (IsConsole())
	{
		DWORD data;
		if (MIC_TemperatureSensorGetRawData(hTempSensor, &data) == TEMPSENSOR_OK)
		{
			DWORD mV = (DWORD)((TEMPSENSOR_CONVERSION_VALUE * data) >> 10);
			double Vo = ((double)mV) / 1000;
			double B = 3380;
			double Ro = 10000;
			double To = 25 + 273.15;
			double Vref = (double)1.5;
			double Rt = Ro * (Vo / (Vref - Vo));
			double Tk = (To * B) / (To * log(Rt / Ro) + B);
			double Tc = Tk - (double)273.15;

			Log(L"Raw data is %d, mV = %d, t(C) = %f" , data, mV, Tc);
		}

		DWORD delay = GetTickCount() + interval_sec * 1000;
		while (IsConsole() && (delay > GetTickCount()))
			Sleep(500);
	}

	goto clean_up;
	/*
	TEMPSENSOR_DATA data;
	UINT32 ret = MIC_TemperatureSensorGet(hTempSensor, &data);
	if (ret != TEMPSENSOR_OK)
		DebugBreak();
	*/

error:;
	while (IsConsole())
		Sleep(500);

clean_up:;
	if (hTempSensor != INVALID_HANDLE_VALUE)
		MIC_TemperatureSensorClose(hTempSensor);

	Log(NULL);
	return 0;
}