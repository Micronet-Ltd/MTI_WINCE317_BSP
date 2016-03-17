/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998-2010 Texas Instruments Incorporated           |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**          
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**     
**+----------------------------------------------------------------------+**
***************************************************************************/

#include <windows.h>
#include "Checksum.h"

#define CRC32_POLYNOMIAL	0xEDB88320L
#define HEXTONUM(x)	( x>='0' && x <= '9') ? x - '0' : ( (tolower(x) >= 'a' && tolower(x) <= 'f')? tolower(x) - 'a' + 10 : 0 )

static DWORD CRC32Table[256];
T_CHECKSUM_INSTANCE g_checksumInstance;

//-----------------------------------------------------------------------------
//! \brief	This function intialize the table used by ComputeCRC32 (fast version)
//!
//-----------------------------------------------------------------------------

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	BOOL bRet = TRUE;

	Checksum_VALUE Checksum;
	Checksum_STRINGVALUE ChecksumString;
	_TCHAR msg[512];

	bRet = Checksum_Initialize(&g_checksumInstance);
	if (bRet == FALSE)
	{
		RETAILMSG(1, (TEXT("\tFailed to initialize checksum engine.\r\n")));
		return FALSE;
	}

	// Compute checksum on file
	RETAILMSG(1, (TEXT("\tChecksum_AddDataFromFile %s.\r\n"), lpCmdLine));
	bRet = Checksum_AddDataFromFile(&g_checksumInstance, (LPCTSTR) lpCmdLine);
	if (bRet == FALSE)
	{
		RETAILMSG(1, (TEXT("\tFailed to compute checksum on file %s.\r\n"), lpCmdLine));
		return FALSE;
	}

	bRet = Checksum_GetCurrentChecksum (&g_checksumInstance, Checksum);
	if (bRet == FALSE)
	{
		RETAILMSG(1, (TEXT("\tFailed to get checksum on file %s.\r\n"), lpCmdLine));
	}

	Checksum_ChecksumToString(Checksum, ChecksumString);

	memset(msg,0, sizeof(msg));
	swprintf(msg, L"%s checksum value : \n0x%s", lpCmdLine, ChecksumString);

	MessageBox(NULL, msg, L"Checksum", MB_OK);

	return 0;
}

static void InitCRC32Table()
{
	DWORD i,j;
	DWORD dwValue;
	
	for ( i = 0; i <= 0xFF ; i++ ) 
	{
		dwValue = i;
		for ( j = 8 ; j > 0; j-- ) 
		{
			if ( dwValue & 0x1 )
			{
				dwValue = ( dwValue >> 0x1 ) ^ CRC32_POLYNOMIAL;
			}
			else
			{
				dwValue >>= 1;
			}
		}
		CRC32Table[ i ] = dwValue;
	}
}


//-----------------------------------------------------------------------------
//! \brief	This function computes the CRC32 of a given buffer (Fast version using tables)
//!
//! \param	buffer Pointer on memory you want compute a crc
//!
//! \param  len Length of zone which you want compute a crc
//!
//! \retun  The CRC32 value
//-----------------------------------------------------------------------------
static DWORD ComputeCRC32(unsigned char *buffer, int len, DWORD dwResult)
{
	int i;	
	unsigned char c;
	
	for(i = 0; i < len; i++)
	{
		c = buffer[i];
		dwResult = (dwResult >> 8) ^ CRC32Table[(dwResult ^ c) & 0xff];
	}
	
	return dwResult;
}

BOOL Checksum_Initialize(LPT_CHECKSUM_INSTANCE checksumInstance)
{
    InitCRC32Table();
	checksumInstance->currentChecksum = 0xFFFFFFFFL;
	return TRUE;
}

BOOL Checksum_Deinitialize(LPT_CHECKSUM_INSTANCE checksumInstance)
{
	return TRUE;
}

BOOL Checksum_Reset(LPT_CHECKSUM_INSTANCE checksumInstance)
{
	checksumInstance->currentChecksum = 0xFFFFFFFFL;
	return TRUE;
}

BOOL Checksum_AddData(const LPT_CHECKSUM_INSTANCE checksumInstance, const LPBYTE lpBuffer, DWORD dwLenBuffer)
{
	// Test Parameters
	if (lpBuffer == NULL)
	{
		return FALSE;
	}

    checksumInstance->currentChecksum = ComputeCRC32(lpBuffer, dwLenBuffer, checksumInstance->currentChecksum);
	return TRUE;
}

BOOL Checksum_AddDataFromFile(const LPT_CHECKSUM_INSTANCE checksumInstance, LPCTSTR szfilePath)
{
	BOOL bRet = FALSE;
	BYTE lpBuffer[2048];

	// Open the file
	HANDLE hFile = CreateFile(szfilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwRead;

		bRet = TRUE;

		// read it KB by KB
		while (ReadFile(hFile, lpBuffer, sizeof(lpBuffer), &dwRead, NULL))
		{
			// End of file ?
			if (dwRead == 0)
			{
				break;
			}

			Checksum_AddData(checksumInstance, lpBuffer, dwRead);
		}
		CloseHandle(hFile);
	}

	return bRet;
}

BOOL Checksum_GetCurrentChecksum(const LPT_CHECKSUM_INSTANCE checksumInstance, Checksum_VALUE checksum)
{
	memcpy(checksum, &checksumInstance->currentChecksum, sizeof(Checksum_VALUE));
	return TRUE;
}

BOOL Checksum_ChecksumToString(const Checksum_VALUE checksum, Checksum_STRINGVALUE checksumString)
{
	BOOL bReturn = FALSE;
	TCHAR tmp[3];
	int i;

	memset(checksumString, 0, sizeof(Checksum_STRINGVALUE));

	for(i = 0; i < CHECKSUM_LEN; ++i)
	{
		wsprintf(tmp, L"%02x", checksum[i]);
		checksumString[i*2] = tmp[0];
		checksumString[i*2 + 1] = tmp[1];
	}
	return bReturn;
}

BOOL Checksum_IsChecksumCorrect(const LPT_CHECKSUM_INSTANCE checksumInstance, const Checksum_VALUE checksumValue)
{
	Checksum_VALUE currentChecksumValue;
	memset(currentChecksumValue, 0, sizeof(Checksum_VALUE));

    if (Checksum_GetCurrentChecksum(checksumInstance, currentChecksumValue) == FALSE)
	{
		return FALSE;
	}

	return (memcmp(checksumValue, currentChecksumValue, sizeof(Checksum_VALUE)) == 0);
}

BOOL Checksum_IsChecksumIdentical(const Checksum_VALUE checksumValue1, const Checksum_VALUE checksumValue2)
{
    return (memcmp(checksumValue1, checksumValue2, sizeof(Checksum_VALUE)) == 0);
}

BOOL Checksum_StringToChecksum(const Checksum_STRINGVALUE checksumString, Checksum_VALUE checksum)
{
	int i;

	memset(checksum, 0, sizeof(Checksum_VALUE));

	for (i = 0; i < CHECKSUM_LEN * 2; i += 2)
	{
		checksum[i/2] = ((HEXTONUM(checksumString[i])) << 4);
		checksum[i/2] |= HEXTONUM(checksumString[i+1]);
	}

	return TRUE;
}