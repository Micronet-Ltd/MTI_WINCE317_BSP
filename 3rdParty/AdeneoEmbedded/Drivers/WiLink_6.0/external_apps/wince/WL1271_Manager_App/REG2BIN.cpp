#include "Windows.h"
#include "REG2BIN.h"


CREG2BIN::CREG2BIN(void)
{
}

CREG2BIN::~CREG2BIN(void)
{
}

LRESULT CREG2BIN::RegToBuffer(LPTSTR szRegKey, LPTSTR szValueNamePrefix, LPBYTE &pBuffer, LPDWORD pdwBufferLen)
{
	HKEY hRegKey;
#ifdef UNDER_CE
	LONG lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, 0, &hRegKey);
#else
	LONG lr = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEYPATH, 0, KEY_ALL_ACCESS, &hRegKey);
#endif

	if (lr != ERROR_SUCCESS)
	{
		wprintf(L"Failed to open registry");	
		return lr;
	}

	WCHAR szValueName[100];
	wsprintf(szValueName, L"%sNB", szValueNamePrefix);

	// Identify number of chunks
	DWORD dwChunkNb;
	DWORD dwChunkSize;
	DWORD dwSize = sizeof(dwChunkNb);
	DWORD dwType = REG_DWORD;
	lr = RegQueryValueEx(hRegKey,
				szValueName,
				0,
				&dwType,
				(LPBYTE)&dwChunkNb,
				&dwSize);

	if (lr != ERROR_SUCCESS)
	{
		RegCloseKey(hRegKey);
		return lr;
	}

	// Identify the size of a chunk, assuming that first chunk is the largest one
	wsprintf(szValueName, L"%s%d", szValueNamePrefix, 0);
	dwSize = 0;
	dwType = REG_BINARY;
	lr = RegQueryValueEx(hRegKey,
				szValueName, 
				0,
				&dwType,
				NULL,
				&dwSize);

	if (lr != ERROR_SUCCESS)
	{
		RegCloseKey(hRegKey);
		return lr;
	}

	// Allocate memory
	dwChunkSize = dwSize;
	DWORD dwBufferLen = (dwChunkNb * dwSize);
	pBuffer = (LPBYTE)LocalAlloc(LPTR, dwBufferLen);
	if (pBuffer == NULL)
	{
		wprintf(L"Failed to allocate buffer");
		RegCloseKey(hRegKey);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	LPBYTE pBufferBytes = pBuffer;
	dwBufferLen = 0;
	for (DWORD dwIndex = 0 ; dwIndex < dwChunkNb ; dwIndex++)
	{
		// Create value name
		wsprintf(szValueName, L"%s%d", szValueNamePrefix, dwIndex);

		// Read chunk data
		dwSize = dwChunkSize;
		dwType = REG_BINARY;
		lr = RegQueryValueEx(hRegKey,
			szValueName, 
			0,
			&dwType,
			pBufferBytes,
			&dwSize);

		if (lr != ERROR_SUCCESS)
		{
			wprintf(L"Error Reading chunk data");
			break;
		}

		// Store info
		pBufferBytes += dwSize;
		dwBufferLen += dwSize;

	}
	
	// Store info
	*pdwBufferLen = dwBufferLen;

	// Close registry
	RegCloseKey(hRegKey);

	return lr;
}

LRESULT CREG2BIN::RegToFile(LPTSTR szRegNVSFile, LPTSTR szRegKey)
{
	LPBYTE pBuffer;
	DWORD dwBufferSize;
	HRESULT hr;

	//Read Registry first
	if (ERROR_SUCCESS != (hr = RegToBuffer(szRegKey, NVSVALUENAME, pBuffer, &dwBufferSize)))
	{
		return hr;
	}

	// Open the NVS file
	HANDLE hFile = CreateFile(szRegNVSFile,
								GENERIC_WRITE,
								0,
								0,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LocalFree(pBuffer);
		return ERROR_FILE_INVALID;
	}

	DWORD dwWrittenBytes = 0;
	if (FALSE == WriteFile(hFile, pBuffer, dwBufferSize,&dwWrittenBytes, 0) || dwBufferSize != dwWrittenBytes)
	{
		hr = ERROR_FILE_CORRUPT;
	}
	else
	{
		hr = ERROR_SUCCESS;
	}

	//
	CloseHandle(hFile);

	//
	LocalFree(pBuffer);
	
	return hr;
}
