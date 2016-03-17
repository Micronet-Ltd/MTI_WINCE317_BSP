// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Igor Lantsman
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

#ifndef _IGNITIONCPL_H_
#define _IGNITIONCPL_H_

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

#define CONF_BYTE			18
#define BIT_ENABLE			3
#define	MASK_EXISTS			(1<<0 | 1<<1 | 1<<2)

#define SUSPEND				0
#define SHUTDOWN			1

#endif  // _IGNITIONCPL_H_
