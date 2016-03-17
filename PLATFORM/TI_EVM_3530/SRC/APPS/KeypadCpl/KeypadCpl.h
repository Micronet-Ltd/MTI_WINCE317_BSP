// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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

#ifndef _KEYPADCPL_H_
#define _KEYPADCPL_H_

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include <Winuser.h>
#include <prsht.h>
#include <cpl.h>
#include <tchar.h>
#include <keypad_ls.h>

#include "resource.h"

extern "C" LONG CALLBACK CPlApplet(HWND hwndCPL, UINT message, LPARAM lParam1, LPARAM lParam2);
extern "C" BOOL DllEntry(HANDLE hInstance, DWORD fdwReason, LPVOID lpvReserved);



////////////////////////////////////////////////////////////
#define LED_MIN_SETTING         KP_LIGHT_MAX
#define LED_MAX_SETTING         KP_LIGHT_MIN
#define LED_NUM_TICKS           KP_LIGHT_MIN
#define LED_TICK_INTERVAL       ((LED_MAX_SETTING - LED_MIN_SETTING) / LED_NUM_TICKS)

#define KP_PW_REG_SD			L"EnableOffKey"
#define KP_PW_REG_SUSP			L"EnableSuspendKey"

#endif  // _KEYPADCPL_H_
