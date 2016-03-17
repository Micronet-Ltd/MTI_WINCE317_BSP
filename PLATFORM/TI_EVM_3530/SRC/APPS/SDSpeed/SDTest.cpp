// SDTest.cpp : Defines the entry point for the application.
//

#include "windows.h"

#define BUFFER_SIZE		64000000

//#define WITH_OVERLOAD_THREAD

#pragma optimize ("", off)

#ifdef WITH_OVERLOAD_THREAD
bool bKeepGoing = true;
static ULONG Thread(void* arg)
{
	volatile int i;
	while (bKeepGoing)
	{
		i++;
	}
	return 0;
}
#endif

#pragma optimize ("", on)


int WINAPI WinMain(	HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR str[255] = {0};
	HANDLE hCurThread = GetCurrentThread();

	// Allocate memory
	char *buf = (char *)VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);
	if (!buf)
	{
		MessageBox(NULL, L"Can't allocate memory", L"Error", 0);
		return 0;
	}

	WCHAR *filePath = _T("\\nandtest\\test.dat");
//	WCHAR *filePath = _T("\\Storage Card\\test.dat");

#ifdef WITH_OVERLOAD_THREAD
	HANDLE hOverloadThread = CreateThread(NULL, 0, &(LPTHREAD_START_ROUTINE )Thread, (LPVOID)0, 0, 0);
	Sleep(1000);
#endif

	int priority = CeGetThreadPriority(hCurThread);
	CeSetThreadPriority(hCurThread, 110);

	// Create/overwrite file for writing
	HANDLE hf = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
	if (hf == INVALID_HANDLE_VALUE)
	{
		wsprintf(str, L"Error creating file. GetLastError() = %d", GetLastError());
		goto end;
	}

	DWORD written;
	DWORD writeIdle = GetIdleTime();
	DWORD writeTicks = GetTickCount();
	if (!WriteFile(hf, buf, BUFFER_SIZE, &written,0))
	{
		wsprintf(str, L"Error writing file. GetLastError() = %d", GetLastError());
		goto end;
	}

	CloseHandle(hf);
	writeTicks = GetTickCount() - writeTicks;
	writeIdle = GetIdleTime() - writeIdle;

	// 
	memset(buf, 0xAA, BUFFER_SIZE); 

	// Open file for reading
	hf = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf == INVALID_HANDLE_VALUE)
	{
		wsprintf(str, L"Error opening file. GetLastError() = %d", GetLastError());
		goto end;
	}

	DWORD readBytes;
	DWORD readIdle = GetIdleTime();
	DWORD readTicks = GetTickCount();
	if (!ReadFile(hf, buf, BUFFER_SIZE, &readBytes, 0))
	{
		wsprintf(str, L"Error reading file. GetLastError() = %d", GetLastError());
		goto end;
	}

	readTicks = GetTickCount() - readTicks;
	readIdle = GetIdleTime() - readIdle;

	double writeSpeed = (BUFFER_SIZE / 1000000) / ((double)writeTicks / 1000);
	double readSpeed = (BUFFER_SIZE / 1000000) / ((double)readTicks / 1000);

	wsprintf(str, L"Write ticks = %ld, idle = %ld, speed = %4.1f MB/s.\r\n\r\nRead ticks = %ld, idle = %ld, speed = %4.1f MB/s.", writeTicks, writeIdle, writeSpeed, readTicks, readIdle, readSpeed);

end:;
	CloseHandle(hf);

	CeSetThreadPriority(hCurThread, priority);

#ifdef WITH_OVERLOAD_THREAD
	bKeepGoing = false;
	WaitForSingleObject(hOverloadThread, INFINITE);
	CloseHandle(hOverloadThread);
#endif

	VirtualFree(buf, BUFFER_SIZE, MEM_DECOMMIT);
	
	MessageBox(NULL, str, L"SD Test", 0);
}