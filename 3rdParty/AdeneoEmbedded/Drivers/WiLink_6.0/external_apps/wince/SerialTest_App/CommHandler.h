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

#include "SerialPort.h"

class CCommHandler
{

private:
	HWND m_hWnd;
	DWORD m_dwIDRead;
	CSerialPort m_serialPort;
	HANDLE m_hReadThread;
	HANDLE m_hTreadEvent;

public:
	CCommHandler(HWND hWnd, DWORD dwIDRead);
	~CCommHandler();

	BOOL Start (DWORD dwSerialPortIndex);
	BOOL Stop ();
	BOOL IsStarted();

	static DWORD WINAPI ReadThreadProc(LPVOID lpParameter);
	BOOL WriteData(LPCTSTR szData);

private:
	BOOL ReadProc();

};