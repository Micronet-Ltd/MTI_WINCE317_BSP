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

#include "SerialPort.h"

CSerialPort::CSerialPort()
{
	m_hPort = INVALID_HANDLE_VALUE;
}

CSerialPort::~CSerialPort()
{
	if (m_hPort != INVALID_HANDLE_VALUE)
	{
		Close();
	}
}

BOOL CSerialPort::Open(DWORD dwIndex)
{
	TCHAR szComName[10];
	
	if (dwIndex > 9)
	{
		return FALSE;
	}

	//
	swprintf(szComName, L"COM%d:", dwIndex);

	//
	m_hPort = CreateFile (szComName,
						GENERIC_READ | GENERIC_WRITE,
						0,
						0,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if (m_hPort == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	// Configure snd/receive timeout
	COMMTIMEOUTS ct;
	ct.ReadIntervalTimeout = 10; // this makes a timeout occur when what is in the buffer has been read
	ct.ReadTotalTimeoutConstant = 0;
	ct.ReadTotalTimeoutMultiplier = 0;
	ct.WriteTotalTimeoutConstant = 10;  // 10 ms added to the timeout of the whole packet
	ct.WriteTotalTimeoutMultiplier = 2;  // 2 ms for each byte sent 
	//ct.ReadIntervalTimeout = 500;
	//ct.ReadTotalTimeoutMultiplier = 100;
	//ct.ReadTotalTimeoutConstant = 100;
	//ct.WriteTotalTimeoutMultiplier = 200;
	//ct.WriteTotalTimeoutConstant = 200;

	SetCommTimeouts(m_hPort, &ct);

	// Set communication mask
	SetCommMask (m_hPort, EV_RXCHAR);

	return TRUE;
}

BOOL CSerialPort::Close()
{
	if (m_hPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hPort);
		m_hPort = INVALID_HANDLE_VALUE;
	}

	return FALSE;
}

BOOL CSerialPort::Read(PBYTE pBuffer, DWORD *pdwBufferSize)
{
	DWORD dwNumberOfBytesRead = 0;
	BOOL bRet = FALSE;
	DWORD dwComEvent = 0;

	if (pBuffer == NULL || pdwBufferSize == NULL || *pdwBufferSize == 0)
	{
		return FALSE;
	}

	// Wait for specific event
	//bRet = WaitCommEvent(m_hPort, &dwComEvent, NULL);
	//if (bRet == FALSE || !(dwComEvent & EV_RXCHAR))
	//{
	//	return FALSE;
	//}

	// Read Data
	bRet = ReadFile(m_hPort, pBuffer, *pdwBufferSize, &dwNumberOfBytesRead, 0);   
	if (bRet == TRUE)
	{
		*pdwBufferSize = dwNumberOfBytesRead;
	}
	else
	{
		*pdwBufferSize = 0;
	}
	return bRet;
}

BOOL CSerialPort::Write(PBYTE pBuffer, DWORD *pdwBufferSize)
{
	DWORD dwNumberOfBytesWritten = 0;
	BOOL bRet = FALSE;

	if (pBuffer == NULL || pdwBufferSize == NULL || *pdwBufferSize == 0)
	{
		return FALSE;
	}

	// Write Data
	bRet = WriteFile(m_hPort, pBuffer, *pdwBufferSize, &dwNumberOfBytesWritten, 0);   
	*pdwBufferSize = dwNumberOfBytesWritten;

	return bRet;
}
