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

class CNetworkFolder :
   public IShellFolder,
   public IPersistFolder
{
protected:
   DWORD m_ObjRefCount;

   void ShowNetworkError(HRESULT hr, LPWSTR pwsz);

public:
   CNetworkFolder();
   ~CNetworkFolder();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID, LPVOID*);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IPersist methods
   STDMETHOD (GetClassID) (LPCLSID);

   //IPersistFolder methods
   STDMETHOD (Initialize) (LPCITEMIDLIST);

   //IShellFolder methods
   STDMETHOD (BindToObject) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (BindToStorage) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (CompareIDs) (LPARAM, LPCITEMIDLIST, LPCITEMIDLIST);
   STDMETHOD (CreateViewObject) (HWND, REFIID, LPVOID* );
   STDMETHOD (EnumObjects) (HWND, DWORD, LPENUMIDLIST*);
   STDMETHOD (GetAttributesOf) (UINT, LPCITEMIDLIST*, LPDWORD);
   STDMETHOD (GetDisplayNameOf) (LPCITEMIDLIST, DWORD, LPSTRRET);
   STDMETHOD (GetUIObjectOf) (HWND, UINT, LPCITEMIDLIST*, REFIID, LPUINT, LPVOID*);
   STDMETHOD (ParseDisplayName) (HWND, LPBC, LPOLESTR, LPDWORD, LPITEMIDLIST*, LPDWORD);
   STDMETHOD (SetNameOf) (HWND, LPCITEMIDLIST, LPCOLESTR, DWORD, LPITEMIDLIST*);
};

