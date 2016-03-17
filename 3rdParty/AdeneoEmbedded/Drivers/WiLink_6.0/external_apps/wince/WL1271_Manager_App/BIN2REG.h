#pragma once

#define NVSCHUNCKSIZE	16

class CBIN2REG
{
public:
	CBIN2REG(void);
	virtual ~CBIN2REG(void);

	LRESULT BinFileToRegFile (LPCTSTR szSourceFileName, LPCTSTR szRegFileName);
	LRESULT BinFileToReg (LPCTSTR szSourceFileName, LPCTSTR szRegKeyPath);
	LRESULT BufferToReg(LPBYTE pBuffer, DWORD dwBufferLen, LPTSTR szRegKey, LPTSTR szValueNamePrefix, DWORD dwChunkSize);

private:
	LRESULT BinFileContentToReg(HANDLE hBinFile, HANDLE hRegFile, LPCSTR szRegKey, DWORD dwChunkSize);
	LRESULT BufferToReg(LPBYTE pBuffer, DWORD dwLen, HANDLE hRegFile, LPSTR szKeyName);


};
