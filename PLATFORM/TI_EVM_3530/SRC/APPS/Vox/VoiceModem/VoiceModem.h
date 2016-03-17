#ifndef _VMODEM_H_
#define _VMODEM_H_


#pragma once
#include "resource.h"

#include <windows.h>
//#include "MuxMdm.h"

typedef struct _VMODEM_CONTEXT
{
 HANDLE      hCom;
 HANDLE      hAgHS;
 DWORD       destThreadId;
 HWND        hDlg;
 BOOL        bMux; 

}VMODEM_CONTEXT_s;


#ifdef __cplusplus
	extern "C" {
#endif

  DWORD WINAPI VoiceModemThread(LPVOID lpParam);


 #ifdef __cplusplus
		}
 #endif	


#endif //_VMODEM_H_