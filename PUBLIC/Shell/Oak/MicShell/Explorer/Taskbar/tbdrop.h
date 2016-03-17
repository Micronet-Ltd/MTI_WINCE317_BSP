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

#include <windows.h>
#include <commctrl.h>
#include "taskbar.hxx"
#include "list.hpp"
#include "resource.h"

IDropTarget * CTaskbarDropTarget_Create(HWND hwndTB, CList * plistTaskBar);

class CTaskbarDropTarget : public IDropTarget
{
protected:
   static const DWORD TIMER_THRESHOLD;

   DWORD m_dwRefs;
   HWND m_hwndTB;
   CList * m_plist;
   UINT m_uTarget;
   DWORD m_dwClockTimer;

public:
   CTaskbarDropTarget(HWND hwndTB, CList *plistTaskBar);
    
   // IUnknown
   STDMETHOD(QueryInterface)(REFIID, LPVOID *);
   STDMETHOD_(ULONG,AddRef)(VOID) ;
   STDMETHOD_(ULONG,Release)(VOID);

   // IDropTarget
   STDMETHOD(DragEnter)(IDataObject*, DWORD, POINTL, DWORD*);
   STDMETHOD(DragOver)(DWORD, POINTL, DWORD*);
   STDMETHOD(DragLeave)();
   STDMETHOD(Drop)(IDataObject*, DWORD, POINTL, DWORD*);
};

