#ifndef _GPSCPL_H_
#define _GPSCPL_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <tchar.h>

#include "resource.h"

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2);
extern "C" BOOL DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved);

#endif  // _GPSCPL_H_
