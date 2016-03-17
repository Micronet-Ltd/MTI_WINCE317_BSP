// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
// Copyright (c) 2008 BSQUARE Corporation. All rights reserved.

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


#include "BspInfo.h"
#include <bsp.h>
#include "..\\..\\SDK\\inc\\swupdate_api.h"
// Returns the number of characters in an expression.
#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))

BOOL CALLBACK BspInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static DWORD BspVersionMessageBox(TCHAR* pszVersionString);

HINSTANCE g_hInstance = NULL;

// Note: BSP_VERSION_STRING is defined in bsp_version.h
BOOL CALLBACK BspInfoDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
    {
	    case WM_INITDIALOG:
		{
			TCHAR pszVersionString[1024];
			BspVersionMessageBox(pszVersionString);
			SetDlgItemText(hDlg, IDC_TXT, pszVersionString);

			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_BSPINFO_ICON));
			if(hIcon)
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (WPARAM)hIcon);
		}
	    return 1;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
			}
			break;
		}
		break;
	}
	return 0;
}
extern "C" BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			// Remember the instance
			g_hInstance = (HINSTANCE)hInstance;
			break;

		case DLL_PROCESS_DETACH:
			g_hInstance = NULL;
			break;
    }
    return TRUE;
}

static DWORD BspVersionMessageBox(void);

// When main control panel is loaded, BspInfo.cpl will be loaded, then
// CPL_INIT, CPL_GETCOUNT and CPL_NEWINQUIRE will be called, then
// BspInfo.cpl will be unloaded. When the BspInfo icon is double clicked,
// BspInfo.cpl will be loaded, then CPL_INIT and CPL_DBLCLK will be called.

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_INIT\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_GETCOUNT\n")));
            return 1;

        case CPL_NEWINQUIRE:
		{
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_NEWINQUIRE\n")));

            // this applet supports only 1 action, lParam1 must be 0
            ASSERT(0 == lParam1);
            ASSERT(lParam2);

            NEWCPLINFO* lpNewCplInfo = (NEWCPLINFO *) lParam2;
            if (lpNewCplInfo)
            {
                lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
                lpNewCplInfo->dwFlags = 0;
                lpNewCplInfo->dwHelpContext = 0;
                lpNewCplInfo->lData = IDI_BSPINFO_ICON;
                _tcscpy(lpNewCplInfo->szName, _T(""));
                _tcscpy(lpNewCplInfo->szInfo, _T(""));
				
                // szName displayed below icon
                LoadString(g_hInstance, IDS_BSPINFO_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                //DEBUGMSG(1, (TEXT("BspInfo: szName = \"%s\"\n"), lpNewCplInfo->szName));

                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_BSPINFO_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));
                //DEBUGMSG(1, (TEXT("BspInfo: szInfo = \"%s\"\n"), lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_BSPINFO_ICON));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("BspInfo: CPL_NEWINQUIRE failed\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
			
        case CPL_DBLCLK:
		{
			// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_DBLCLK\n")));
			TCHAR pszTitle[MAX_PATH] = {0};

			LoadString(g_hInstance, IDS_BSPINFO_TITLE, pszTitle, MAX_PATH);
			ASSERT(pszTitle);

			HWND hWnd = FindWindow(L"Dialog", pszTitle);
			if (hWnd)
			{
				SetForegroundWindow(hWnd);
			}
			else
			{
				DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_BSPINFO), hwndCPL, BspInfoDlgProc);
			}
            return 0;
		}	
        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_STOP\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            //DEBUGMSG(1, (TEXT("BspInfo: CPL_EXIT\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}

extern BOOL MIC_GetParamBlock(VOID *, UINT32);
// thread to start messagebox
static DWORD BspVersionMessageBox(TCHAR* pszVersionString)
{
	int i;
	ULARGE_INTEGER lfree = {0};
	ULARGE_INTEGER ltotal = {0};
	ULARGE_INTEGER lavail = {0};
	unsigned long free = 0;
	unsigned long total = 0;
	MEMORYSTATUS ms = {0};
	manufacture_block mb = {0};
//    TCHAR pszVersionString[1024];
    TCHAR szName[32];

	ms.dwLength = sizeof(ms);
	GlobalMemoryStatus(&ms);

	free = ms.dwAvailPhys/1024;
	total = ms.dwTotalPhys/1024;

	GetDiskFreeSpaceEx(L"\\", &lavail, &ltotal, &lfree);
	lavail.QuadPart /= 1024;
	ltotal.QuadPart /= 1024;
	lfree.QuadPart /= 1024;
	
	BspVersionExt ever, xver;
	DWORD size;

	MIC_GetBspSwVersionExt( EBOOT_VERSION, (VOID*)&ever, sizeof(ever), &size );		
	MIC_GetBspSwVersionExt( XLDR_VERSION, (VOID*)&xver, sizeof(xver), &size );
	MIC_GetParamBlock(&mb, sizeof(mb));

	mb.device_serial_number[RECORD_LENGTH - 1] = 0;

	for(i = 0; i < sizeof(mb.device_serial_number); i++)
	{
		if(0 == mb.device_serial_number[i])
			break;
		if(!isxdigit(mb.device_serial_number[i]))
		{
			mb.device_serial_number[i] = 0;
			break;
		}
	}

	if(mb.device_serial_number[0] == 0)
		sprintf((char *)mb.device_serial_number, "not set");

    LoadString(g_hInstance, IDS_BSPINFO_TITLE, szName, LENGTHOF(szName));
	wsprintf(pszVersionString, L"BSP Version:\t\t" BSP_VERSION_STRING L"\nBuild Date:\t\t" TEXT(__DATE__) \
								L"\n\nEboot Version:\t%u.%02u.%03u.%02u\n" \
								L"X-Loader Version:\t%u.%02u.%03u.%02u" \
								L"\n\nMemory:%8u Kb free from %8u Kb" \
								L"\nStorage:%8u Kb free from %8u Kb" \
								L"\n\nDevice serial number: %S", 
								ever.dwVersionMajor, ever.dwVersionMinor, ever.dwVersionIncremental, ever.dwVersionBuild,
								xver.dwVersionMajor, xver.dwVersionMinor, xver.dwVersionIncremental, xver.dwVersionBuild,
								free, total, lfree.LowPart, ltotal.LowPart,
								mb.device_serial_number);
    //DEBUGMSG(1, (TEXT("BspInfo: MessageBoxThread, szName \"%s\", pszVersionString string \"%s\"\n"), szName, pszVersionString));
	//MessageBox(NULL, pszVersionString, szName, MB_OKCANCEL );

	return 0;
}

