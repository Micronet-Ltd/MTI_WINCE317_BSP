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
#include "ceshlobj.h"

class CNameSpaceEnum :
   public IEnumIDList,
   public IEnumFilter
{
private:
   DWORD m_ObjRefCount;
   CLSID m_clsid;
   HKEY m_hkey;
   DWORD m_dwIndex;
   LPITEMIDLIST m_pidlFS;
   IEnumIDList * m_pEnumFS;
   SHCONTF m_grfFlags;
   bool m_bUsingFilter;

public:
   CNameSpaceEnum(SHCONTF grfFlags = 0);
   ~CNameSpaceEnum();

   BOOL Initialize(REFCLSID clsid, LPCITEMIDLIST pidlFS = NULL);
   void Close();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IEnumIDList methods
   STDMETHOD (Next) (ULONG, LPITEMIDLIST*, ULONG*);
   STDMETHOD (Skip) (ULONG);
   STDMETHOD (Reset) (VOID);
   STDMETHOD (Clone) (IEnumIDList**);

   // IEnumFilter
   STDMETHOD (SetFilterString)(LPCWSTR);
};


