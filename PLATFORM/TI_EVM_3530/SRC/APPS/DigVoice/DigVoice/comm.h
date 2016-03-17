#ifndef _COMM_H_
#define _COMM_H_

#include <windows.h>

#define MAX_SIZE_TO_READ 80

#define XON_CHAR     0x11
#define XOFF_CHAR    0x13

//extern bool    g_bUseXonXoff;

#ifdef __cplusplus
	extern "C" {
#endif

HANDLE WINAPI __OpenComPort(LPCTSTR pszPortName);
//HANDLE OpenComPort(LPCTSTR pszPortName);
//BOOL WaitIncoming(HANDLE hCom);
//BOOL sendATCmd(HANDLE hCom, const char *cmdStr, char *respStr);

BOOL sendATCmdApp(HANDLE hCom, const char *cmdStr, char *respStr);

HANDLE WINAPI __OpenComPortTest(LPCTSTR pszPortName);

	#ifdef __cplusplus
		}
	#endif	

#endif //_COMM_H_