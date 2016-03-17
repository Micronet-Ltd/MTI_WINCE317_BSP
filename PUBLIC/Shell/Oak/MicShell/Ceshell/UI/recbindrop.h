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

#include "ceshhpc.h"

#pragma once

class CRecBinDropTarget :
   public IDropTarget
{
private:
   DWORD m_ObjRefCount;
   LPWSTR m_pwszPath;
   DWORD m_dwEffect;

public:
   CRecBinDropTarget();
   ~CRecBinDropTarget();

   BOOL Initialize();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID, LPVOID*);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   // IDropTarget methods
   STDMETHOD (DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
   STDMETHOD (DragOver)(DWORD, POINTL, LPDWORD);
   STDMETHOD (DragLeave)(VOID);
   STDMETHOD (Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
};

