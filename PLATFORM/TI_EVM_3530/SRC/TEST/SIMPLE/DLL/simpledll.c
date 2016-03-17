//===================================================================
//
//	Module Name:	SIMPLEDLL.DLL
//
//	File Name:		simpledll.c
//
//	Description:	Support driver for simple benchmark
//
//===================================================================
// Copyright (c) 2007- 2009 BSQUARE Corporation. All rights reserved.
//===================================================================

//----------------------------------------------------------------------------
//
// Includes
//
//----------------------------------------------------------------------------

#include <windows.h>
#include <ceddk.h>

#include "..\simpledll.h"

//----------------------------------------------------------------------------
//
// Externals
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Globals
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Function Prototypes
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// DLL entry point
//
//----------------------------------------------------------------------------

BOOL DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE) hinstDll);
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
//
// Standard stream interface driver functions
//
//----------------------------------------------------------------------------

DWORD SID_Init(DWORD dwContext)
{
	return 1;
}

DWORD SID_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	return 1;
}

BOOL SID_Close (DWORD hOpenContext)
{
	return TRUE;
}

BOOL SID_Deinit (DWORD hDeviceContext)
{
	return TRUE;
}

BOOL SID_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    static LPVOID pVirtualMemoryAddress = NULL;
    LPVOID pVirtualMemoryAddressCaller = NULL;
    DWORD PhysicalAddress = 0;

	switch (dwCode)
	{
		case SIMPLEDLL_IOCTL_ALLOCATE_PHYSICAL_MEMORY:
		{
            DWORD RequestedSize = *(DWORD *)pBufIn;
            pVirtualMemoryAddress = AllocPhysMem(RequestedSize, PAGE_READWRITE|PAGE_NOCACHE, 0, 0, &PhysicalAddress);
            if (pVirtualMemoryAddress == NULL)
			    return FALSE;
				
			pVirtualMemoryAddressCaller = VirtualAllocCopyEx(GetCurrentProcess(), GetOwnerProcess(), (LPVOID)pVirtualMemoryAddress, RequestedSize, PAGE_READWRITE|PAGE_NOCACHE);	
            if (pVirtualMemoryAddressCaller == NULL)
			    return FALSE;

            *(DWORD *)pBufOut = (DWORD)pVirtualMemoryAddressCaller;
            return TRUE;
			break;
        }
		
		case SIMPLEDLL_IOCTL_FREE_PHYSICAL_MEMORY:
            FreePhysMem(pVirtualMemoryAddress);
            return TRUE;
			break;
    }
	
	return FALSE;
}

void SID_PowerDown(DWORD hDeviceContext)
{
}

void SID_PowerUp(DWORD hDeviceContext)
{
}

DWORD SID_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	return 0;
}

DWORD SID_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
	return -1;
}

DWORD SID_Seek(DWORD hOpenContext, long Amount, WORD Type)
{
	return -1;
}
