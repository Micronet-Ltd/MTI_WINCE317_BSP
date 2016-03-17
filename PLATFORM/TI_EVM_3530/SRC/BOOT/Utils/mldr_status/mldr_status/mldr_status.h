#ifndef __MLDR_H
#define __MLDR_H

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_SIZE_TO_READ  1024
#define MAX_SIZE_TO_STORE 2048


typedef DWORD (*pfnHandleData)(char* buffer,BOOL* process_completed); 

DWORD ComOperationsThread (pfnHandleData fnHandleData);
DWORD OpenPort (TCHAR*);
DWORD ClosePort();


DWORD OperateTransport (pfnHandleData fnHandleData);
DWORD InitTransport (TCHAR*);
DWORD CloseTransport();

#define KEY_PRESSED 99


//#define pattern_str "DBG_BOOT:"

//#define pattern_str "Sent BOOTME"
#define pattern_str              "D:\\>"
#define process_complete_pattern "UPDATE_COMPLETED"
#define progress_str             "PROGRESS"

#ifdef __cplusplus
}
#endif

#endif
