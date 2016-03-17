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

#include "ceshpriv.h"

class CShellMalloc :
   public IMalloc
{
public:
   DWORD m_ObjRefCount;

   CShellMalloc();
   ~CShellMalloc();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID FAR* ppobj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IMalloc methods
   STDMETHOD_ (LPVOID, Alloc) (ULONG cb);
   STDMETHOD_ (LPVOID, Realloc) (void *pv, ULONG cb);
   STDMETHOD_ (VOID, Free) (void *pv);
   STDMETHOD_ (ULONG, GetSize) (void *pv);
   STDMETHOD_ (int, DidAlloc) (void *pv);
   STDMETHOD_ (VOID, HeapMinimize) (void);
};

