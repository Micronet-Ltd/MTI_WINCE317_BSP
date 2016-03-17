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


HANDLE OpenComPort(LPCTSTR pszPortName);
BOOL WaitIncoming(HANDLE hCom);
BOOL sendATCmd(HANDLE hCom, const char *cmdStr, char *respStr);
BOOL sendATCmdNoResp(HANDLE hCom, const char *cmdStr);


	#ifdef __cplusplus
		}
	#endif	

#endif //_COMM_H_