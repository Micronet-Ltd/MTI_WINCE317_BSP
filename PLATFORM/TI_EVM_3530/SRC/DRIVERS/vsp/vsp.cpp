// vsp.cpp : Defines the entry point for the DLL application.
//

#include "vsp.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

HANDLE COM_Init(ULONG Identifier, LPCVOID lpvBusContext)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S\r\n", __FUNCTION__));
	InitContext *initContext = new InitContext();
	if (initContext && !initContext->Init(Identifier, lpvBusContext))
	{
		delete(initContext);
		initContext = NULL;
	}
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, initContext = %p\r\n", __FUNCTION__, initContext));
	return initContext;
}

BOOL COM_Deinit(InitContext *initContext)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, initContext = %p\r\n", __FUNCTION__, initContext));
	delete(initContext);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S\r\n", __FUNCTION__));
	return TRUE;
}
	
HANDLE COM_Open(InitContext *initContext, DWORD AccessCode, DWORD ShareMode)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, initContext = %p\r\n", __FUNCTION__, initContext));
	VirtualSerialPort *vsp = initContext->Open(AccessCode, ShareMode);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, vsp = %p\r\n", __FUNCTION__, vsp));
	return vsp;
}

BOOL COM_Close(VirtualSerialPort *vsp)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, vsp = %p\r\n", __FUNCTION__, vsp));
	InitContext *initContext = vsp->GetInitContext();
	BOOL res = initContext->Close(vsp);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, res = %d\r\n", __FUNCTION__, res));
	return res;
}

ULONG COM_Read(VirtualSerialPort *vsp, PUCHAR pTargetBuffer, ULONG BufferLength)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, vsp = %p\r\n", __FUNCTION__, vsp));
	ULONG res = vsp->Read(pTargetBuffer, BufferLength);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, res = %d\r\n", __FUNCTION__, res));
	return res;
}
	
ULONG COM_Write(VirtualSerialPort *vsp, PUCHAR pSourceBytes, ULONG NumberOfBytes)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, vsp = %p\r\n", __FUNCTION__, vsp));
	ULONG res = vsp->Write(pSourceBytes, NumberOfBytes);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, res = %d\r\n", __FUNCTION__, res));
	return res;
}
	
ULONG COM_Seek(VirtualSerialPort *vsp, LONG Position,	DWORD Type)
{
	return (ULONG)-1;
}

BOOL COM_PowerDown(VirtualSerialPort *vsp)
{
	return FALSE;
}

BOOL COM_PowerUp(VirtualSerialPort *vsp)
{
	return FALSE;
}

BOOL COM_IOControl(VirtualSerialPort *vsp,
		DWORD dwCode, 
		PBYTE pBufIn,
		DWORD dwLenIn, 
		PBYTE pBufOut, 
		DWORD dwLenOut,
		PDWORD pdwActualOut)
{
	RETAILMSG(VSP_RETAILS, (L"VSP:+%S, vsp = %p\r\n", __FUNCTION__, vsp));
	BOOL res = vsp->IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	RETAILMSG(VSP_RETAILS, (L"VSP:-%S, res = %d\r\n", __FUNCTION__, res));
	return res;
}