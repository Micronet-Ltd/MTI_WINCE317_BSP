//===================================================================
//
//  Module Name:    Simple.exe
//  
//  File Name:      Simple.c
//  
//  Description:    Measures number of simple instructions executed per second.
//
//===================================================================
// Copyright (c) 2007- 2009 BSQUARE Corporation. All rights reserved.
//===================================================================

//-----------------------------------------------------------------------------
//
// Windows CE header files
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <types.h>
#include <windev.h>
#include <pwingdi.h>
#include <ceddk.h>
#include "..\simpledll.h"

//-----------------------------------------------------------------------------
//
// External Functions
//
//-----------------------------------------------------------------------------

extern DWORD InstructionTestLoop(int Iterations);
extern DWORD MemoryReadTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);
extern DWORD MemoryWriteTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);
extern DWORD MultipleMemoryReadTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);
extern DWORD MultipleMemoryWriteTestLoop(DWORD ToggleCount, PDWORD MemoryAddress);

//-----------------------------------------------------------------------------
//
// Defines
//
//-----------------------------------------------------------------------------

#define NUMBER_OF_INSTRUCTION_LOOP_ITERATIONS           100000

#define NUMBER_OF_MEMORY_READ_LOOP_ITERATIONS           20000
#define NUMBER_OF_MEMORY_WRITE_LOOP_ITERATIONS          20000

#define NUMBER_OF_MULTIPLE_MEMORY_READ_LOOP_ITERATIONS  20000
#define NUMBER_OF_MULTIPLE_MEMORY_WRITE_LOOP_ITERATIONS 20000

//-----------------------------------------------------------------------------
// Time Functions
//-----------------------------------------------------------------------------

LARGE_INTEGER PerformanceFrequency;

void InitTimeStamp(void)
{
    QueryPerformanceFrequency(&PerformanceFrequency);
}

void GetTimeStamp(LARGE_INTEGER * pTimeStamp)
{
    QueryPerformanceCounter(pTimeStamp);
}

DWORD GetTimeDeltaUs(LARGE_INTEGER * pOldTimeStamp)
{
    LARGE_INTEGER NewTimeStamp;
    LARGE_INTEGER Result;

    GetTimeStamp(&NewTimeStamp);    

    Result.QuadPart = NewTimeStamp.QuadPart - pOldTimeStamp->QuadPart;
    
    Result.QuadPart = (Result.QuadPart * 1000000) / PerformanceFrequency.QuadPart;

    return Result.LowPart;
}

double ComputeRate(double EventsPerIteration, double Iterations, double TimeInUs)
{
    double Rate = EventsPerIteration * Iterations;
    
    Rate = Rate / TimeInUs;

    return Rate;
}

// used for in-cache memory testing
#define BUFF_SIZE   (256 * 1024)
int pbuf1[BUFF_SIZE];
int pbuf2[BUFF_SIZE];

TCHAR ResultString[4096];
TCHAR FinalResultString[8128];

//-----------------------------------------------------------------------------
//
// Main
//
//-----------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    //BOOL bKMode;
    LARGE_INTEGER TimeStampStart;

    DWORD OperationsPerIteration;
    
    // memory addresses
    PDWORD pVirtualMemoryAddress = NULL;
    DWORD MemorySize;
    DWORD BytesReturned;
    
    DWORD TimeMemoryWriteTestUs = 0;
    DWORD MemoryWritesPerIteration = 0;

    DWORD TimeMemoryReadTestUs = 0;
    DWORD MemoryReadsPerIteration = 0;
    
    double InstructionTestResult;

    double MemoryWriteTestResult;
    double MemoryReadTestResult;

    double MultipleMemoryWriteTestResult;
    double MultipleMemoryReadTestResult;
    
    double InCacheMemTestResult;
    
    HANDLE hFile;
    HANDLE hSimpleDll;
    DWORD dwBytesWritten;

    ResultString[0] = 0;
    FinalResultString[0] = 0;

    // wait for sounds to finish
    Sleep(1000);

    //------------------------------------------------------------------------
    // Get timestamp functions for use
    //------------------------------------------------------------------------

    InitTimeStamp();

    //------------------------------------------------------------------------
    // Get a chunk of non-cached non-buffered RAM
    //------------------------------------------------------------------------

    // open SID1:
    hSimpleDll = CreateFile(TEXT("SID1:"), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (hSimpleDll != INVALID_HANDLE_VALUE)
    {
        // allocate physical memory
        MemorySize = 4096;
        if (!DeviceIoControl(hSimpleDll, SIMPLEDLL_IOCTL_ALLOCATE_PHYSICAL_MEMORY, &MemorySize, sizeof(MemorySize), &pVirtualMemoryAddress, sizeof(pVirtualMemoryAddress), &BytesReturned, NULL))
            RETAILMSG(1, (TEXT("Simple: SIMPLEDLL_IOCTL_ALLOCATE_PHYSICAL_MEMORY failed\r\n")));
    }
    else
    {
        RETAILMSG(1, (TEXT("Simple: Can't open SID1:\r\n")));
    }
    
    //------------------------------------------------------------------------
    // Switch to kernel mode, disable interrupts
    //------------------------------------------------------------------------

    //bKMode = SetKMode(TRUE);
    //SetInterruptState(FALSE);

    //------------------------------------------------------------------------
    // Simple instruction MIPS test
    //------------------------------------------------------------------------

    RETAILMSG(1, (TEXT("Simple: MIPS test\r\n")));

    GetTimeStamp(&TimeStampStart);
    OperationsPerIteration = InstructionTestLoop(NUMBER_OF_INSTRUCTION_LOOP_ITERATIONS);
    InstructionTestResult = ComputeRate(OperationsPerIteration, NUMBER_OF_INSTRUCTION_LOOP_ITERATIONS, GetTimeDeltaUs(&TimeStampStart));

    if (pVirtualMemoryAddress != NULL)
    {
        //------------------------------------------------------------------------
        // Simple single DWORD at a time non-cached, non-buffered RAM read test
        //------------------------------------------------------------------------

        RETAILMSG(1, (TEXT("Simple: DWORD non-cached, non-buffered RAM read test\r\n")));
        GetTimeStamp(&TimeStampStart);
        OperationsPerIteration = MemoryReadTestLoop(NUMBER_OF_MEMORY_READ_LOOP_ITERATIONS, pVirtualMemoryAddress);
        MemoryReadTestResult = ComputeRate(OperationsPerIteration, NUMBER_OF_MEMORY_READ_LOOP_ITERATIONS, GetTimeDeltaUs(&TimeStampStart));

        //------------------------------------------------------------------------
        // Simple single DWORD at a time non-cached, non-buffered RAM write test
        //------------------------------------------------------------------------

        RETAILMSG(1, (TEXT("Simple: DWORD non-cached, non-buffered RAM write test\r\n")));
        GetTimeStamp(&TimeStampStart);
        OperationsPerIteration = MemoryWriteTestLoop(NUMBER_OF_MEMORY_WRITE_LOOP_ITERATIONS, pVirtualMemoryAddress);
        MemoryWriteTestResult = ComputeRate(OperationsPerIteration, NUMBER_OF_MEMORY_WRITE_LOOP_ITERATIONS, GetTimeDeltaUs(&TimeStampStart));

        //------------------------------------------------------------------------
        // Simple quad DWORD at a time non-cached, non-buffered RAM read test
        //------------------------------------------------------------------------

        RETAILMSG(1, (TEXT("Simple: 4-DWORD non-cached, non-buffered RAM read test\r\n")));
        GetTimeStamp(&TimeStampStart);
        OperationsPerIteration = MultipleMemoryReadTestLoop(NUMBER_OF_MULTIPLE_MEMORY_READ_LOOP_ITERATIONS, pVirtualMemoryAddress);
        MultipleMemoryReadTestResult = ComputeRate(OperationsPerIteration, NUMBER_OF_MULTIPLE_MEMORY_READ_LOOP_ITERATIONS, GetTimeDeltaUs(&TimeStampStart));

        //------------------------------------------------------------------------
        // Simple quad DWORD at a time non-cached, non-buffered RAM write test
        //------------------------------------------------------------------------

        RETAILMSG(1, (TEXT("Simple: 4-DWORD non-cached, non-buffered RAM write test\r\n")));
        GetTimeStamp(&TimeStampStart);
        OperationsPerIteration = MultipleMemoryWriteTestLoop(NUMBER_OF_MULTIPLE_MEMORY_WRITE_LOOP_ITERATIONS, pVirtualMemoryAddress);
        MultipleMemoryWriteTestResult = ComputeRate(OperationsPerIteration, NUMBER_OF_MULTIPLE_MEMORY_WRITE_LOOP_ITERATIONS, GetTimeDeltaUs(&TimeStampStart));
    }
        
    // simple memory to memory copy using DWORD pointers
    {
        int i, j;
        const int ITER = 100;

        for (i = 0; i < BUFF_SIZE; i++)
        {
            pbuf2[i] = i;
        }

        GetTimeStamp(&TimeStampStart);

        for (j = 0; j < ITER; j++)
        {
            for (i = 0; i < BUFF_SIZE; i++)
            {
                pbuf1[i] = pbuf2[i];
            }
        }

        InCacheMemTestResult = ComputeRate(BUFF_SIZE, ITER, GetTimeDeltaUs(&TimeStampStart));
    }
    
    //------------------------------------------------------------------------
    // Enable interrupts and return to normal mode
    //------------------------------------------------------------------------

    //SetInterruptState(TRUE);
    //SetKMode(bKMode);

    //------------------------------------------------------------------------
    // Show results
    //------------------------------------------------------------------------

    //printf("Basic execution rate: %0.2f\n", InstructionTestResult);
    swprintf(ResultString, L"execution rate: %0.2f\n", InstructionTestResult);
    wcscat(FinalResultString, ResultString);

    swprintf(ResultString, L"Cached results:\n");
    wcscat(FinalResultString, ResultString);
    swprintf(ResultString, L"RAM DWORD copy rate: %0.2f\n", InCacheMemTestResult);
    wcscat(FinalResultString, ResultString);

    if (pVirtualMemoryAddress != NULL)
    {
        swprintf(ResultString, L"Non-cached/buffered results:\n");
        wcscat(FinalResultString, ResultString);

        //printf("DWORD RAM read rate: %0.2f\n", MemoryReadTestResult);
        swprintf(ResultString, L"RAM read rate:  %0.2f (ldr)\n", MemoryReadTestResult);
        wcscat(FinalResultString, ResultString);

        //printf("DWORD RAM write rate: %0.2f\n", MemoryWriteTestResult);
        swprintf(ResultString, L"RAM write rate: %0.2f (str)\n", MemoryWriteTestResult);
        wcscat(FinalResultString, ResultString);

        //printf("DWORD RAM read rate: %0.2f\n", MultipleMemoryReadTestResult);
        swprintf(ResultString, L"RAM read rate:  %0.2f (ldm)\n", MultipleMemoryReadTestResult);
        wcscat(FinalResultString, ResultString);

        //printf("DWORD RAM write rate: %0.2f\n", MultipleMemoryWriteTestResult);
        swprintf(ResultString, L"RAM write rate: %0.2f (stm)\n", MultipleMemoryWriteTestResult);
        wcscat(FinalResultString, ResultString);
    }
        
    // Add some notes to the output

    swprintf(ResultString, L"Rates in millions per second\n");
    wcscat(FinalResultString, ResultString);
    swprintf(ResultString, L"Multipy by 4 for MB/second\n");
    wcscat(FinalResultString, ResultString);
    
    MessageBox(GetForegroundWindow(), FinalResultString, TEXT("Test Results"), MB_OK | /* MB_ICONINFORMATION | */ MB_DEFBUTTON1 | MB_SETFOREGROUND);

    // create the file
    hFile = CreateFile(L"\\Windows\\simple.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        WriteFile(hFile, FinalResultString, (wcslen(FinalResultString) + 1) * sizeof(TCHAR), &dwBytesWritten, NULL);
        CloseHandle(hFile);
    }
    
    if (hSimpleDll != INVALID_HANDLE_VALUE)
    {
        DeviceIoControl(hSimpleDll, SIMPLEDLL_IOCTL_FREE_PHYSICAL_MEMORY, &pVirtualMemoryAddress, sizeof(pVirtualMemoryAddress), NULL, 0, NULL, NULL);
        CloseHandle(hSimpleDll);
    }

    return 0;
}
