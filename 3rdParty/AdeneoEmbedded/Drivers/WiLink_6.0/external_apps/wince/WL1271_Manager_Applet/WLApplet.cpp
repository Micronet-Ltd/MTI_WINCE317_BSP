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
 
/** \file   WLApplet.cpp 
 *  \brief  Control Panel applet for WL1271_Manager_Applet appliction launching
 *
 *  \see   WL1271_Manager_Applet.cpp  
 */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <tchar.h>

#include "resource.h"

#define LENGTHOF(exp) ((sizeof((exp)))/sizeof((*(exp))))

HINSTANCE g_hInstance = NULL;

BOOL WINAPI DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved)
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

LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    switch (message)
    {
        case CPL_INIT:
            // Perform global initializations, especially memory  allocations, here.
            // Return 1 for success or 0 for failure.
            // Control Panel does not load if failure is returned.
            DEBUGMSG(1, (TEXT("CPlApplet: CPL_INIT\n")));
            return 1;

        case CPL_GETCOUNT:
            // The number of actions supported by this applet
            DEBUGMSG(1, (TEXT("CPlApplet: CPL_GETCOUNT\n")));
            return 1;

        case CPL_NEWINQUIRE:
        {
            // This message is sent once for each dialog box, as
            // determined by the value returned from CPL_GETCOUNT.
            // lParam1 is the 0-based index of the dialog box.
            // lParam2 is a pointer to the NEWCPLINFO structure.
            //DEBUGMSG(1, (TEXT("CPlApplet: CPL_NEWINQUIRE\n")));

            // this applet supports only 1 action, lParam1 must be 0
            //ASSERT(0 == lParam1);
            //ASSERT(lParam2);

            NEWCPLINFO* lpNewCplInfo = (NEWCPLINFO *) lParam2;
            if (lpNewCplInfo)
            {
                lpNewCplInfo->dwSize = sizeof(NEWCPLINFO);
                lpNewCplInfo->dwFlags = 0;
                lpNewCplInfo->dwHelpContext = 0;
                lpNewCplInfo->lData = IDI_WLAPPLET_ICON;
                _tcscpy(lpNewCplInfo->szName, _T(""));
                _tcscpy(lpNewCplInfo->szInfo, _T(""));
                
                // szName displayed below icon
                LoadString(g_hInstance, IDS_WLAPPLET_TITLE, lpNewCplInfo->szName, LENGTHOF(lpNewCplInfo->szName));
                DEBUGMSG(1, (TEXT("CPlApplet: szName = \"%s\"\n"), lpNewCplInfo->szName));

                // szInfo displayed when icon is selected
                LoadString(g_hInstance, IDS_WLAPPLET_DESCRIPTION, lpNewCplInfo->szInfo, LENGTHOF(lpNewCplInfo->szInfo));
                DEBUGMSG(1, (TEXT("CPlApplet: szInfo = \"%s\"\n"), lpNewCplInfo->szInfo));

                // The large icon for this application. Do not free this 
                // HICON; it is freed by the Control Panel infrastructure.
                lpNewCplInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_WLAPPLET_ICON));

                // szHelpFile is unused
                _tcscpy(lpNewCplInfo->szHelpFile, _T(""));
                return 0;
            }
            DEBUGMSG(1, (TEXT("CPlApplet: CPL_NEWINQUIRE failed\n")));
            return 1;  // Nonzero value means CPlApplet failed.
        }
            
        case CPL_DBLCLK:
			{
				// The user has double-clicked the icon for the dialog box in lParam1 (zero-based).
				DEBUGMSG(1, (TEXT("CPlApplet: CPL_DBLCLK\n")));
				PROCESS_INFORMATION pi = {0};
				if (CreateProcess(_T("\\Windows\\WL1271_Manager_App.exe"), NULL, NULL,
					NULL, FALSE, 0, NULL, NULL, NULL, &pi))
			 {
				 CloseHandle(pi.hThread);
				 CloseHandle(pi.hProcess);
				 return 0;
			 }

				return 1;
			}

        case CPL_STOP:
            // Called once for each dialog box. Used for cleanup.
            DEBUGMSG(1, (TEXT("CPlApplet: CPL_STOP\n")));
            return 0;

        case CPL_EXIT:
            // Called only once for the application. Used for cleanup.
            DEBUGMSG(1, (TEXT("CPlApplet: CPL_EXIT\n")));
            return 0;

        default:
            return 0;
    }

    return 1;  // CPlApplet failed.
}

