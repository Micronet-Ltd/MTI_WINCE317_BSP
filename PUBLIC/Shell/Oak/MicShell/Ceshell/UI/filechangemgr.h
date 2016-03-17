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

#pragma once

#include "ceshhpc.h"

class FileChangeManager
{
protected:
   static const DWORD NOTIFY_FILTER;
   static const DWORD NOTIFY_BUFFER_MAX;

   HANDLE m_hThread;
   CRITICAL_SECTION m_cs;
   CRITICAL_SECTION m_cs2;
   BOOL m_fShutdown;
   DWORD m_dwWaitObjects;
   HANDLE m_waitObjects[32];
   IShellChangeNotify * m_ShellChangeNotify[31];

   static DWORD WINAPI ThreadProc(LPVOID lpParameter);
   void OnChange(HANDLE hNotificationInfo, IShellChangeNotify * pShellChangeNotify);
   void ProcessEvents();

public:
   FileChangeManager();
   ~FileChangeManager();

   UINT AddWatch(LPCWSTR pwszPath, IShellChangeNotify * pShellChangeNotify);
   void RefreshAllViews();
   BOOL RemoveWatch(UINT uWatchID);
   void Shutdown();
};

