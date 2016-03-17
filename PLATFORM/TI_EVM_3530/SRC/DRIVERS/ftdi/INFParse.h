#ifndef __INFPARSE_H__
#define __INFPARSE_H__
#ifdef __cplusplus
extern "C" {
#endif
BOOL GetVIDPID(char * filename, DWORD * pdwVID, DWORD * pdwPID);
BOOL GetNextVIDPID(char * filename, DWORD * pdwVID, DWORD * pdwPID);
void RegisterINFValues(char * filename, TCHAR * pgszDriverPrefix, TCHAR * pszStreamDriverKey);
BOOL GetStreamDriverKey(TCHAR * pszStreamDriverKey);
//BOOL GetStreamDriverKey(TCHAR * pszStreamDriverKey, int size, int PID, BOOL bGotPID);
//BOOL GetCurrentStreamDriverKey(TCHAR * pszStreamDriverKey, int PID);
BOOL GetDevicePrefix(TCHAR * pszStreamDriverKey, TCHAR * pszDriverPrefix, int size);
HKEY OpenClientRegistryKey(LPCWSTR szUniqueDriverId);
#ifdef __cplusplus
}
#endif
#endif