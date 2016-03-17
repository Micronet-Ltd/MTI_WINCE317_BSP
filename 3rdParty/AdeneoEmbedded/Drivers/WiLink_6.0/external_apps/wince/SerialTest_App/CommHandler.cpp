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
#include "CommHandler.h"

CCommHandler::CCommHandler(HWND hWnd, DWORD dwIDRead)
:m_hWnd(hWnd), m_dwIDRead(dwIDRead), m_hReadThread(NULL)
{

}

CCommHandler::~CCommHandler()
{
	if (m_hReadThread != NULL)
	{
		Stop();
	}
}

BOOL CCommHandler::Start (DWORD dwSerialPortIndex)
{
	BOOL bRet = m_serialPort.Open(dwSerialPortIndex);

	if (bRet == FALSE)
	{
		return bRet;
	}

	m_hTreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Create Thread
	m_hReadThread =  CreateThread (
						NULL,
						0,
						CCommHandler::ReadThreadProc,
						(LPVOID)this,
						CREATE_SUSPENDED,
						NULL);

	if (m_hReadThread == NULL)
	{
		m_serialPort.Close();
		return FALSE;
	}

	// Resume suspended thread
	ResumeThread(m_hReadThread);
	
	return TRUE;
}

BOOL CCommHandler::Stop ()
{
	if (m_hReadThread == NULL)
		return TRUE;
	else
	{
		// Notify thread to stop 
		SetEvent(m_hTreadEvent);

		//
		WaitForSingleObject(m_hReadThread, INFINITE);

		CloseHandle(m_hReadThread);
		m_hReadThread = NULL;
		
		CloseHandle(m_hTreadEvent);
	}
	return TRUE;
}

BOOL CCommHandler::IsStarted()
{
	return m_hReadThread != NULL;
}

DWORD WINAPI CCommHandler::ReadThreadProc(LPVOID lpParameter)
{
	if (lpParameter == NULL)
	{
		return -1;
	}

	((CCommHandler *)lpParameter)->ReadProc();
	return 0;
}

BOOL CCommHandler::ReadProc()
{
	BOOL bStop = FALSE;
	BYTE pBuffer[255];
	DWORD dwBufferSize;
	BOOL bReadStatus;

	while (bStop == FALSE)
	{
		// Configure buffer size
		dwBufferSize = (sizeof(pBuffer)/sizeof(pBuffer[0]))-1;

		// Read Data
		bReadStatus = m_serialPort.Read(pBuffer, &dwBufferSize);

		// Check Thread End
		if (WaitForSingleObject(m_hTreadEvent, 0) == WAIT_OBJECT_0)
		{
			bStop = TRUE;
			continue;
		}

		// Show Data on GUI
		if (bReadStatus == TRUE && dwBufferSize != 0)
		{
			WCHAR szWindowText[1024];
			BOOL bRet = FALSE;
			WCHAR szBufferData[1024];

			// Get Current
			GetWindowText(GetDlgItem(m_hWnd, m_dwIDRead),
							szWindowText,
							sizeof (szWindowText)/sizeof(szWindowText[0]));

			// Convert received data into WCHAR
			pBuffer[dwBufferSize] = '\0';
			mbstowcs(szBufferData, (char *)pBuffer, dwBufferSize);

			// Append new
			wcscat_s(szWindowText, 
					sizeof (szWindowText)/sizeof(szWindowText[0]),
					szBufferData);

			// Set New
			SetWindowText(GetDlgItem(m_hWnd, m_dwIDRead),
							szWindowText);
			
		}

		Sleep(100);
	}

	return TRUE;
}

BOOL CCommHandler::WriteData(LPCTSTR szData)
{
	CHAR szDataMbs[1024];
	DWORD dwData;
	size_t stDataSize;
	BOOL bRet = FALSE;

	// Convert 
	stDataSize = wcstombs(szDataMbs, szData, sizeof (szDataMbs)/sizeof(szDataMbs[0]));
	dwData = stDataSize;
	
	//
	if (dwData > 0)
	{
		bRet = m_serialPort.Write((PBYTE)szDataMbs, &dwData);
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}