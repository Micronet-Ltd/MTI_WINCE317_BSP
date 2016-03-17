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

class CShellClassFactory :
   public IClassFactory
{
protected:
   DWORD m_ObjRefCount;
   CLSID m_clsid;

public:
   CShellClassFactory(REFCLSID rclsid);
   ~CShellClassFactory();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID, LPVOID*);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IClassFactory methods
   STDMETHOD (CreateInstance) (IUnknown*, REFIID, void**);
   STDMETHOD (LockServer) (BOOL);
};

