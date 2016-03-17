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
#include <windows.h>
#include <tchar.h>
#include <cpl.h>
#include "resource.h"
#include "stgui.h"

#define DIALOG_CLASS    TEXT("Dialog")
#define ERRLEN          32

HINSTANCE g_hInstance  = NULL;              // Library instance
UINT      g_cInitCount = 0;

#define ARRAYSIZE(a)            (sizeof(a)/sizeof(a[0])) 
#define LOADSTRING(id, buf)     LoadString(g_hInstance, id, buf, ARRAYSIZE(buf))

// --------------------------------------------------------------------
BOOL Size2String(LONGLONG llBytes, LPTSTR pszSize, DWORD cLen)
// --------------------------------------------------------------------
{
    double bytes = (double)llBytes;
    DWORD cIter = 0;    
    LPCTSTR pszUnits[] = { _T("B"), _T("KB"), _T("MB"), _T("GB"), _T("TB") };
    DWORD cUnits = sizeof(pszUnits) / sizeof(pszUnits[0]);

    // move from bytes to kb, to mb, to gb and so on diving by 1024
    while(bytes >= 1024 && cIter < (cUnits-1))
    {
        bytes /= 1024;
        cIter++;
    }
    _sntprintf(pszSize, cLen, _T("%.2f %s\n"), bytes, pszUnits[cIter]);
    return TRUE;
}

// --------------------------------------------------------
VOID ErrBox(HWND hWnd, UINT idString, DWORD dwErr)
// --------------------------------------------------------
{
    TCHAR szTitle[MAX_PATH] = _T("");
    LPTSTR szError = NULL;
    INT cBytes = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        dwErr,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (TCHAR *)&szError,
        500,
        NULL );

    if(szError && cBytes)
        szError[cBytes] = _T('\0');
	else
		szError = _T("");

    LoadString(g_hInstance, idString, szTitle, MAX_PATH);

    MessageBox(hWnd, szError, szTitle, MB_OK);

    if(szError && cBytes)
        LocalFree(szError);
        
}

// --------------------------------------------------------
VOID CenterWindow(HWND hWnd)
// --------------------------------------------------------
{
    int x, y;
    RECT r = {0};
    HDC hdc = GetDC(hWnd);
    if(hdc)
    {
        x = GetDeviceCaps(hdc, HORZRES);
        y = GetDeviceCaps(hdc, VERTRES);
        GetWindowRect(hWnd, &r);
        MoveWindow(hWnd, (x-(r.right-r.left))/2, (y-(r.bottom-r.top))/2,
            (r.right-r.left), (r.bottom-r.top), TRUE);
        ReleaseDC(hWnd, hdc);
    }
}

// --------------------------------------------------------
BOOL StartApplet(VOID)
// --------------------------------------------------------
{
    PROPSHEETPAGE psp[1];
    PROPSHEETHEADER psh;

    // see if a window of DIALOG_CLASS with this title already exists.
    HWND hPrevWnd = FindWindow(DIALOG_CLASS, 
        (LPCTSTR)LoadString(g_hInstance, IDS_PROP, NULL, 0));
    if (hPrevWnd)
    {
       // if the window already exists, switch to it because it is almost 
       // guaranteed to be a previous instance of this applet
       SetForegroundWindow((HWND)((DWORD)hPrevWnd | 0x00000001));
       return FALSE;
    }

    // single tab property sheet
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[0].hInstance = g_hInstance;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MAIN);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = MainDlgProc;
    psp[0].pszTitle = (LPCTSTR)(LoadString(g_hInstance, IDS_TITLE, NULL, 0));
    psp[0].pfnCallback = NULL;
    psp[0].lParam = 0;

    // property sheet header
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_HASHELP | PSH_USECALLBACK;
    psh.hwndParent = NULL;
    psh.hInstance = g_hInstance;
    psh.pszIcon = MAKEINTRESOURCE(IDI_PARTMAN);
    psh.pszCaption = (LPCTSTR)(LoadString(g_hInstance, IDS_PROP, NULL,0));
    psh.nPages = 1;
    psh.nStartPage = 0;
    psh.ppsp = psp;
    psh.pfnCallback = PropSheetCallback;

    return PropertySheet(&psh);
}

// --------------------------------------------------------
extern "C" BOOL WINAPI DllEntry(PVOID hmod, ULONG ulReason, PCONTEXT pctx OPTIONAL)
// --------------------------------------------------------
{
   if (ulReason == DLL_PROCESS_ATTACH){
        g_hInstance = (HINSTANCE)hmod;
   }

   return TRUE;

   UNREFERENCED_PARAMETER(pctx);
}

// --------------------------------------------------------
extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2)
// --------------------------------------------------------
{
   static         int iInitCount = 0;
   LPNEWCPLINFO   lpncpli;

   switch (message) {
   case CPL_INIT:
      // nothing to do on init
      if(0 == g_cInitCount)
      {
        g_cInitCount++;
        return TRUE;
      }
      else
        return FALSE;

   case CPL_GETCOUNT:
      // only one applet in this .CPL
      return (LONG)1;  

   case CPL_NEWINQUIRE:        // third message, sent once per app

      ASSERT(0 == lParam1);
      
      lpncpli = (LPNEWCPLINFO) lParam2;

      lpncpli->dwSize = (DWORD) sizeof(NEWCPLINFO);
      lpncpli->dwFlags = 0;
      lpncpli->dwHelpContext = 0;
      lpncpli->lData = IDI_PARTMAN;
      lpncpli->hIcon = LoadIcon (g_hInstance,
         (LPCTSTR) MAKEINTRESOURCE(IDI_PARTMAN));
      lpncpli->szHelpFile[0] = '\0';

      LOADSTRING (IDS_TITLE, lpncpli->szName);
      LOADSTRING (IDS_INFO, lpncpli->szInfo);

      break;

   case CPL_DBLCLK:
      // create the prop sheet
      StartApplet();
      break;

   case CPL_STOP:

   case CPL_EXIT:
      // close hwnd
      g_cInitCount--;
      break;

   default:
      break;
   }

   return 0;
}  // CPlApplet

