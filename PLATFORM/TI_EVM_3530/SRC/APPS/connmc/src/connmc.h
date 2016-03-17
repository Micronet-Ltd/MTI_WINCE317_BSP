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
/******************************************************************************

connmc.h

******************************************************************************/

#ifndef CONNMC_H__
#define CONNMC_H__

#pragma warning (disable:4514)
#pragma warning (push, 3)
#include <windows.h>
#include <windowsx.h>
#pragma warning (pop)
#include <dbt.h> // For WM_DEVICECHANGE
#include <commctrl.h>
#include <wcommctl.h>
#include <tchar.h>

#ifdef USE_SIP
#include <sipapi.h>
#endif // USE_SIP

#include "resource.h"
#include "newconninfo.h"
#include "rnaconninfo.h"
#include "dccconninfo.h"
#include "vpnconninfo.h"
#include "lanconninfo.h"
#include "pppoeconninfo.h"

#ifdef USE_SIP
typedef BOOL (WINAPI* LPFNSIP)(SIPINFO*);
typedef DWORD (WINAPI* LPFNSIPSTATUS)();
#define SIP_UP    0
#define SIP_DOWN  1
#endif // USE_SIP

// Messages For communicating from the logic to the UI
#define RNM_SHOWERROR                WM_USER + 1

// The number of colums in this list view strings are IDS_LV_COLUMN_X
#define LV_COLUMN_COUNT              4

// Globals
extern BOOL g_fPortrait;

void ValidateName(TCHAR * pszNewName, UINT * pError);

#define ZONE_ERROR		0x8000

#define CCHSIZEOF(x)	(sizeof(x) / sizeof(TCHAR))

#endif // CONNMC_H__
