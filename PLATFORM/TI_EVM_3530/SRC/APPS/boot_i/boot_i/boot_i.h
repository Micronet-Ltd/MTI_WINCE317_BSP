#pragma once

#include "resource.h"
#define BUFFER_SIZE   1024
typedef struct tag_socket_param
{
	SOCKET s;
	HWND w;
	unsigned short Port;
	TCHAR TxTotal[25];
	TCHAR RvTotal[25];
	TCHAR StatusTx[256];
	TCHAR StatusRv[256];
}srv_param;
extern srv_param sp;
extern "C" unsigned long __stdcall socket_srv(void *pv_param);
