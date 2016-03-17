// bench2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "bench2.h"
#include <windows.h>
#include <commctrl.h>

#define MAX_C 20
extern "C" void mem_cpy(void *, void *, unsigned long);
extern "C" void mem_cpy_2(void *, void *, unsigned long);
extern "C" void mem_cpy_16(void *, void *, unsigned long);

unsigned long mbpersec(unsigned __int64 size, unsigned long msec)
{
	return (unsigned long)(1000*(size/(1024*1024))/msec);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
 	// TODO: Place code here.
	UINT32 Ticks, Avar;
	DWORD dwMB10s = 0;
	int i = MAX_C;
	int BufSize = 64*1024*1024;


		SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);


	UINT32 *buf=(UINT32 *)VirtualAlloc(0,BufSize,MEM_COMMIT,PAGE_READWRITE);
	if (!buf)
	{ 
		NKDbgPrintfW(L"Cannot allocate memory %d\r\n",BufSize);
//		sprintf(szDebugOut,"Cannot allocate memory %d\r\n",BufSize);
//	    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
		VirtualFree(buf, 0, MEM_RELEASE);
		return 0;
  
	}
	UINT32 *buf2=(UINT32 *)VirtualAlloc(0,BufSize,MEM_COMMIT,PAGE_READWRITE);
	if (!buf2)
	{ 
		NKDbgPrintfW(L"Cannot allocate memory %d\r\n",BufSize);
//		sprintf(szDebugOut,"Cannot allocate memory %d\r\n",BufSize);
//	    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
		VirtualFree(buf, 0, MEM_RELEASE);
		VirtualFree(buf2, 0, MEM_RELEASE);
		return 0;
  
	}
	memset(buf,  0x55, BufSize);
	memset(buf2, 0xAA, BufSize);
/*
	HANDLE hPort =  serial_open(_T("COM3:"), 115200, FLOWCONTROL_OFF);


//	NKDbgPrintfW(L"\r\n\r\nBench External memory> read of %d MB\r\n", MAX_C*BufSize/1024/1024);
	sprintf(szDebugOut,"\r\n\r\nBench External memory of %d MB\r\n", MAX_C*BufSize/1024/1024);
	WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
//	NKDbgPrintfW(L"\r\n=================================================================================================\r\n");
	sprintf(szDebugOut,"\r\n================================================================================\r\n");
	WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
//	NKDbgPrintfW(L"%8s %16s %22s %24s %20s\r\n", L"Type", L"Total time(ms)", L"Performance(MB/s)", L"Code Execution time(ms)", L"Burst read time(ms)");
	sprintf(szDebugOut,"%8s %16s %22s %24s\r\n", "Type", "Total time(ms)", "Performance(MB/s)", "Code Execution time(ms)");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
//	NKDbgPrintfW(L"\r\n-------------------------------------------------------------------------------------------------\r\n");
	sprintf(szDebugOut,"\r\n------------------------------------------------------------------------------\r\n");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);


	Sleep(500);
	DWORD dwPrio= CeGetThreadPriority(GetCurrentThread());
	CeSetThreadPriority(GetCurrentThread(),100);
	int i = MAX_C;
	DWORD dwMB10s =0;
	DWORD dwTime, dwAvar = 0;

	do
	{
	dwTime=GetTickCount();	

	memcpy(buf2, buf, BufSize);

	dwTime=GetTickCount()-dwTime;
	if (!dwTime)
		dwTime=1; 
	
	dwMB10s += mbpersec(BufSize, dwTime);
	dwMB10s >>= 1;
	dwAvar += dwTime;
	dwAvar >>= 1;
	
	
	}while(i--);

	//	NKDbgPrintfW(L"%8s %16d %22d %24s %20s\r\n", L"QWORD", dwTime, dwMB10s, L"N/A", L"N/A");
	sprintf(szDebugOut,"%8s %16d %22d %24s\r\n", "memcpy", dwAvar, dwMB10s, "N/A");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);

	
	i = MAX_C;
	dwMB10s = dwAvar = 0;
	do{
		dwTime=GetTickCount();

		DW_Copy(buf, buf2, BufSize);

		dwTime=GetTickCount()-dwTime;
		if (!dwTime)
			dwTime=1; 
		
		dwMB10s += mbpersec(BufSize, dwTime);
		dwMB10s >>= 1;
		dwAvar += dwTime;
		dwAvar >>= 1;
	}while(i--);
	
//	NKDbgPrintfW(L"%8s %16d %22d %24s %20s\r\n", L"DWORD", dwTime, dwMB10s, L"N/A", L"N/A");
	sprintf(szDebugOut,"%8s %16d %22d %24s\r\n", "DWORD", dwTime, dwMB10s, "N/A");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
  


	i = MAX_C;
	dwMB10s = dwAvar = 0;
	do{
		dwTime=GetTickCount();

		W_Copy((WORD*)buf,(WORD*)buf2, BufSize);

		dwTime=GetTickCount()-dwTime;
		if (!dwTime)
			dwTime=1; 
		
		dwMB10s += mbpersec(BufSize, dwTime);
		dwMB10s >>= 1;
		dwAvar += dwTime;
		dwAvar >>= 1;
	}while(i--);
	
	
//	NKDbgPrintfW(L"%8s %16d %22d %24s %20s\r\n", L"WORD", dwTime, dwMB10s, L"N/A", L"N/A");
	sprintf(szDebugOut,"%8s %16d %22d %24s\r\n", "WORD", dwTime, dwMB10s, "N/A");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);


	i = MAX_C;
	dwMB10s = dwAvar = 0;
	do{
		dwTime=GetTickCount();

		B_Copy((BYTE*)buf,(BYTE*)buf2, BufSize);

		dwTime=GetTickCount()-dwTime;
		if (!dwTime)
			dwTime=1; 
		
		dwMB10s += mbpersec(BufSize, dwTime);
		dwMB10s >>= 1;
		dwAvar += dwTime;
		dwAvar >>= 1;
	}while(i--);
	
	
//	NKDbgPrintfW(L"%8s %16d %22d %24s %20s\r\n", L"BYTE", dwTime, dwMB10s, L"N/A", L"N/A");
	sprintf(szDebugOut,"%8s %16d %22d %24s\r\n", "BYTE", dwTime, dwMB10s, "N/A");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);
//	NKDbgPrintfW(L"\r\n-------------------------------------------------------------------------------------------------\r\n");
	sprintf(szDebugOut,"\r\n--------------------------------------------------------------------------------\r\n");
    WriteFile(hPort, szDebugOut, strlen(szDebugOut), &dwBytesRead, NULL);


	CeSetThreadPriority(GetCurrentThread(),dwPrio);

quit:
	VirtualFree(buf2, 0, MEM_RELEASE);
	}
	CloseHandle(hPort);
	return 0;
*/
	Ticks = GetTickCount();	
/*	i = 0;
	do{
		ramread4t(buf, BufSize);
	}while(++i < MAX_C);

	dwcTime=GetTickCount()-dwcTime;
	if (!dwcTime)
		dwcTime=1; 
*/
//	memcpy(buf, buf2, BufSize);
	i = MAX_C;
	Avar = dwMB10s = 0;
	do{
		Ticks = GetTickCount();	
		
		mem_cpy_16(buf, buf2, BufSize);
		
		Ticks = GetTickCount()- Ticks;
		if(!Ticks)
			Ticks=1; 

		dwMB10s += mbpersec(BufSize, Ticks);
		dwMB10s >>= 1;
		Avar += Ticks;
		Avar >>= 1;
	}while(--i);

	
	NKDbgPrintfW(L"%8s %16d %22d\r\n", L"4*DWORD", Avar, dwMB10s);

	i = MAX_C;
	Avar = dwMB10s = 0;
	do{
		Ticks = GetTickCount();	
		
		mem_cpy_2(buf, buf2, BufSize);
		
		Ticks = GetTickCount()- Ticks;
		if(!Ticks)
			Ticks=1; 

		dwMB10s += mbpersec(BufSize, Ticks);
		dwMB10s >>= 1;
		Avar += Ticks;
		Avar >>= 1;
	}while(--i);

	
	NKDbgPrintfW(L"%8s %16d %22d\r\n", L"WORD", Avar, dwMB10s);

	i = MAX_C;
	Avar = dwMB10s = 0;
	do{
		Ticks = GetTickCount();	
		
		mem_cpy(buf, buf2, BufSize);
		
		Ticks = GetTickCount()- Ticks;
		if(!Ticks)
			Ticks=1; 

		dwMB10s += mbpersec(BufSize, Ticks);
		dwMB10s >>= 1;
		Avar += Ticks;
		Avar >>= 1;
	}while(--i);

	
	NKDbgPrintfW(L"%8s %16d %22d\r\n", L"BYTE", Avar, dwMB10s);

	VirtualFree(buf, 0, MEM_RELEASE);
	VirtualFree(buf2, 0, MEM_RELEASE);

	return 0;
}
