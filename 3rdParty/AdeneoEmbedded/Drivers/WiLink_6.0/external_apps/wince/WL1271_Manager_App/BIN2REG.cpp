#include <windows.h>
#include "BIN2REG.h"
#ifdef UNDER_CE
#include <winreg.h>
#endif

#define NVSVALUENAME L"NVSChunk"
#define REGKEYPATH L"Comm\\tiwlnapi1\\Parms\\NVS"
#define HREGKEYROOT "HKEY_LOCAL_MACHINE"
#define HREGKEYPATH "HKEY_LOCAL_MACHINE\\Comm\\tiwlnapi1\\Parms\\NVS"

#define CHUNKDATASIZE 16

CBIN2REG::CBIN2REG(void)
{
}

CBIN2REG::~CBIN2REG(void)
{
}

LRESULT CBIN2REG::BinFileToRegFile (LPCTSTR szSourceFileName, LPCTSTR szRegFileName)
{
	HANDLE hBinFile = INVALID_HANDLE_VALUE;
	HANDLE hRegFile = INVALID_HANDLE_VALUE;

	hBinFile = CreateFile(szSourceFileName,
							GENERIC_READ,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

	if (hBinFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Failed to open Binary file");
		return ERROR_FILE_NOT_FOUND;
	}

	hRegFile = CreateFile(szRegFileName,
							GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

	if (hRegFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Failed to create Registry file");
		CloseHandle(hBinFile);
		return ERROR_FILE_INVALID;
	}

	LRESULT hr = BinFileContentToReg(hBinFile, hRegFile, HREGKEYPATH, CHUNKDATASIZE);

	if (hRegFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hRegFile);
		hRegFile = INVALID_HANDLE_VALUE;
	}
	if (hBinFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hBinFile);
		hBinFile = INVALID_HANDLE_VALUE;
	}

	return hr;
}

LRESULT CBIN2REG::BufferToReg(LPBYTE pBuffer, DWORD dwLen, HANDLE hRegFile, LPSTR szKeyName)
{
	DWORD dwWrittenBytes = 0;
	CHAR szBuffer[100];

	sprintf(szBuffer, "\t\"%s\"=hex:", szKeyName);
	WriteFile(hRegFile, szBuffer, strlen(szBuffer)*sizeof(szBuffer[0]), &dwWrittenBytes, NULL);	

	for (DWORD dwIndex = 0 ; dwIndex < dwLen ; dwIndex++)
	{
		// First and Last element
		if (dwIndex == 0 && dwIndex+1 == dwLen)
		{
			sprintf(szBuffer, "%02X\r\n", pBuffer[dwIndex]);
		}
		// First element
		if (dwIndex == 0)
		{
			sprintf(szBuffer, "%02X", pBuffer[dwIndex]);
		}
		// Last element
		else if (dwIndex+1 == dwLen)
		{
			sprintf(szBuffer, ",%02X\r\n", pBuffer[dwIndex]);
		}
		// Other elements
		else
		{
			sprintf(szBuffer, ",%02X", pBuffer[dwIndex]);
		}
	
		WriteFile(hRegFile, szBuffer, strlen(szBuffer)*sizeof(szBuffer[0]), &dwWrittenBytes, NULL);	
	}
	
	return ERROR_SUCCESS;
}

LRESULT CBIN2REG::BinFileContentToReg(HANDLE hBinFile, HANDLE hRegFile, LPCSTR szRegKey, DWORD dwChunkSize)
{
	DWORD dwBinFileSizeHigh = 0;
	DWORD dwBinFileSize = GetFileSize(hBinFile, &dwBinFileSizeHigh);

	DWORD dwChunkNb = (dwBinFileSize/dwChunkSize) + ((dwBinFileSize%dwChunkSize!=0) ? 1 : 0);

	// Allocate work buffer
	LPBYTE pBuffer = (LPBYTE)GlobalAlloc(GPTR, dwChunkSize);
	if (pBuffer == NULL)
	{
		wprintf(L"Failed to allocate memory");
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	// Create Reg Key
	CHAR strBuffer[200];
	sprintf(strBuffer, "\r\n[%s]\r\n", szRegKey);

	DWORD dwWrittenBytes = 0;
	WriteFile(hRegFile, strBuffer, strlen(strBuffer)*sizeof(strBuffer[0]), &dwWrittenBytes, NULL);

	for (DWORD dwIndex = 0 ; dwIndex < dwChunkNb ; dwIndex++)
	{
		CHAR szKeyName[20];
		sprintf(szKeyName, "NVSChunk%d", dwIndex);

		// Read Chunk Data
		DWORD dwDataLength = 0;
		ReadFile(hBinFile, pBuffer, dwChunkSize, &dwDataLength, NULL);

		// Write the Chunk
		if (ERROR_SUCCESS != BufferToReg(pBuffer, dwDataLength, hRegFile, szKeyName))
		{
			wprintf(L"Failes to write buffer");
		}
	}
	sprintf(strBuffer, "\t\"NVSChunkNB\"=dword:%X\r\n", dwChunkNb);
	WriteFile(hRegFile, strBuffer, strlen(strBuffer)*sizeof(strBuffer[0]), &dwWrittenBytes, NULL);


	// Free Memory
	if (pBuffer != NULL)
	{
		GlobalFree(pBuffer);
		pBuffer = NULL;
	}

	return ERROR_SUCCESS;
}

LRESULT CBIN2REG::BinFileToReg (LPCTSTR szSourceFileName, LPCTSTR szRegKeyPath)
{
	HANDLE hBinFile = INVALID_HANDLE_VALUE;

	hBinFile = CreateFile(szSourceFileName,
							GENERIC_READ,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

	if (hBinFile == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Failed to open Binary file");
		return ERROR_FILE_NOT_FOUND;
	}

	// Read File content in one shot
	DWORD dwBinFileSizeHigh = 0;
	DWORD dwBinFileSize = GetFileSize(hBinFile, &dwBinFileSizeHigh);

	// Allocate work buffer
	LPBYTE pBuffer = (LPBYTE)LocalAlloc(LPTR, dwBinFileSize);

	if (pBuffer == NULL)
	{
		wprintf(L"Failed to allocate memory");
		CloseHandle(hBinFile);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	DWORD dwReadBytes;
	ReadFile(hBinFile, pBuffer, dwBinFileSize, &dwReadBytes, NULL);

	LRESULT hr = BufferToReg(pBuffer, dwReadBytes,  REGKEYPATH, NVSVALUENAME, NVSCHUNCKSIZE);

	// Free allocated memory
	if (pBuffer != NULL)
	{
		LocalFree(pBuffer);
	}

	if (hBinFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hBinFile);
		hBinFile = INVALID_HANDLE_VALUE;
	}

	return hr;

}

LRESULT CBIN2REG::BufferToReg(LPBYTE pBuffer, DWORD dwBufferLen, LPTSTR szRegKey, LPTSTR szValueNamePrefix, DWORD dwChunkSize)
{
	// Open Registry
	HKEY hRegKey;
#if UNDER_CE
	LONG lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEYPATH, 0, 0, &hRegKey);
#else
	LONG lr = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEYPATH, 0, KEY_ALL_ACCESS, &hRegKey);
#endif

	if (lr == ERROR_FILE_NOT_FOUND)
	{
#ifdef UNDER_CE
		lr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGKEYPATH, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hRegKey, NULL);
#else
		lr = RegCreateKey(HKEY_CURRENT_USER, REGKEYPATH, &hRegKey);
#endif
	}
	
	if (lr != ERROR_SUCCESS)
	{
		return lr;
	}

	// Write chunks
	DWORD dwChunkNb = (dwBufferLen/dwChunkSize) + ((dwBufferLen%dwChunkSize!=0) ? 1 : 0);
	for (DWORD dwIndex = 0 ; dwIndex < dwChunkNb ; dwIndex++)
	{
		// Create Value Name
		WCHAR szValueName[100];
		wsprintf(szValueName, L"%s%d", szValueNamePrefix, dwIndex);

		// Prepare buffers
		LPBYTE pBytes = pBuffer + (dwIndex*dwChunkSize);
		DWORD dwBytesLen = (dwIndex+1 == dwChunkNb)? (dwBufferLen%dwChunkSize): dwChunkSize;
		
		// Store data
#ifdef UNDER_CE
		LONG lr = RegSetValueEx (hRegKey, 
								szValueName,
								0,
								REG_BINARY,
								pBytes,
								dwBytesLen);
#else
		LONG lr = RegSetValueEx (hRegKey, 
								szValueName,
								0,
								REG_BINARY,
								pBytes,
								dwBytesLen);
#endif

		if (lr != ERROR_SUCCESS)
		{
			wprintf(L"Failed to write the registry");
		}			
	}

	// Create Chunks NB Value Name
	WCHAR szValueName[100];
	wsprintf(szValueName, L"%sNB", szValueNamePrefix);

	DWORD dwBytesLen = sizeof(DWORD);

	// Store Chunks NB data
#ifdef UNDER_CE
	lr = RegSetValueEx (hRegKey, 
							szValueName,
							0,
							REG_DWORD,
							(BYTE*)&dwChunkNb,
							dwBytesLen);
#else
	lr = RegSetValueEx(hRegKey, 
						szValueName,
						0,
						REG_DWORD,
						(LPBYTE)&dwChunkNb,
						dwBytesLen);
#endif

	if (lr != ERROR_SUCCESS)
	{
		wprintf(L"Failed to write the registry");
	}	

	// Save data
	RegFlushKey(hRegKey);

	// Close registry access
	RegCloseKey(hRegKey);

	return ERROR_SUCCESS;
}