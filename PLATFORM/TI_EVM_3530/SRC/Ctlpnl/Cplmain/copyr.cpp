//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

**/
#include "cplpch.h"

extern "C" BOOL CALLBACK CopyrightsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define BUFFER_SIZE 1023

/************************************************************************************
Routine Description:
    The dialog procedure for processing the Sound accessibility tab.
   
Arguments:
    Regular DlgProc
    
Return Value:
    BOOL value based on processed message 
************************************************************************************/
extern "C" BOOL CALLBACK CopyrightsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{    
	//TODO: 
	//Assumption: Copyright file is under windows folder.
	//Change the file name when the name is availbale.

	TCHAR	szFileName[]	= TEXT("\\windows\\copyrts.txt");
	LPTSTR	lpszFileName	= szFileName;
	HANDLE	hFile;
	DWORD	dwBytesToRead = 0, dwReadBytes = 0, dwFileSize = 0;
	PBYTE	pbBuffer		= NULL;
	WCHAR	*lpszConvert	= NULL;
	HWND	hEditCtl;
	
    switch (message)     
    {
        case WM_INITDIALOG:
		{	
			hFile = CreateFile(lpszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			
			if(INVALID_HANDLE_VALUE == hFile)
			{
				RETAILMSG(1, (L"CreateFile(%s) fails", szFileName));				
			}
			else 
			{
				dwFileSize = GetFileSize(hFile, NULL);

				// Make sure that a file size is valid.
				if (0xFFFFFFFF == dwFileSize) 
				{
					DEBUGMSG(ZONE_SYSTEM, (L"Error %u trying to get size of \"%s\".", GetLastError(), szFileName));					
				}
				else 
				{
					pbBuffer = (PBYTE)LocalAlloc(LMEM_ZEROINIT, BUFFER_SIZE + 1);
					hEditCtl = GetDlgItem(hDlg, IDC_COPYR);

					if(pbBuffer)
					{
						while (dwFileSize > 0) 
						{
							// Get a data block size to read.
							dwBytesToRead = (dwFileSize < BUFFER_SIZE) ? dwFileSize : BUFFER_SIZE;

							// Read the data block.
							if (!ReadFile(hFile, pbBuffer, dwBytesToRead, &(dwReadBytes = 0), NULL) ||
								(dwBytesToRead != dwReadBytes))
							{
								RETAILMSG(1, (L"Error %u reading from \"%s\".", GetLastError(), szFileName));
								break;
							}
							else 
							{
								// Transfer the data to the edit control.
								lpszConvert = (WCHAR *)LocalAlloc(LPTR, (dwBytesToRead + 1) * sizeof(WCHAR));

								if(lpszConvert) {
									MultiByteToWideChar ( CP_ACP, 0, (LPCSTR)pbBuffer, dwBytesToRead,
														lpszConvert, dwBytesToRead + 1);

									// Be sure the file string is null terminated.								
									lpszConvert[dwBytesToRead] = 0;	

									SendMessage(hEditCtl, EM_SETSEL, (WPARAM) -1, (LPARAM) -1);
									SendMessage(hEditCtl, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM)(LPTSTR)lpszConvert);

									// Calculate a "bytes left to read".
									dwFileSize -= dwBytesToRead;								

									LocalFree(lpszConvert);
									lpszConvert = NULL;
								}
							}
						}
					}

					if(pbBuffer)
						LocalFree(pbBuffer);
				}

				CloseHandle(hFile);
			}
			
			hFile = NULL;

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );

			return TRUE;
		}
		
		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDOK:				
					return TRUE;
			}			
	}

	return FALSE;
}
