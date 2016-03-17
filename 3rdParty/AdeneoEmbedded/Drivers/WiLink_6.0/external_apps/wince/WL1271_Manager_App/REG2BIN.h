#pragma once

#define NVSVALUENAME L"NVSChunk"
#define REGKEYPATH L"Comm\\tiwlnapi1\\Parms\\NVS"

class CREG2BIN
{
public:
	CREG2BIN(void);
	~CREG2BIN(void);

	LRESULT RegToBuffer(LPTSTR szRegKey, LPTSTR szValueNamePrefix, LPBYTE &pBuffer, LPDWORD pdwBufferLen);
	LRESULT RegToFile(LPTSTR szRegNVSFile, LPTSTR szRegKey);

};
