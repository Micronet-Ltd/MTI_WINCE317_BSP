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

class CDesktopFolder :
   public IShellFolder,
   public IShellDetails,
   public IPersistFolder
{
private:
   DWORD m_ObjRefCount;
   LPITEMIDLIST m_pidl;

   static LPITEMIDLIST CreateRootPidl();
   STDMETHODIMP GetDesktop(LPITEMIDLIST * ppidlDesktop,  IShellFolder ** ppDesktopFolder);

public:
   CDesktopFolder();
   ~CDesktopFolder();

   // IUnknown methods
   STDMETHOD (QueryInterface) (REFIID, LPVOID*);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   // IShellFolder methods
   STDMETHOD (ParseDisplayName) (HWND, LPBC, LPOLESTR, LPDWORD, LPITEMIDLIST*, LPDWORD);
   STDMETHOD (EnumObjects) (HWND, DWORD, LPENUMIDLIST*);
   STDMETHOD (BindToObject) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (BindToStorage) (LPCITEMIDLIST, LPBC, REFIID, LPVOID*);
   STDMETHOD (CompareIDs) (LPARAM, LPCITEMIDLIST, LPCITEMIDLIST);
   STDMETHOD (CreateViewObject) (HWND, REFIID, LPVOID*);
   STDMETHOD (GetAttributesOf) (UINT, LPCITEMIDLIST*, LPDWORD);
   STDMETHOD (GetUIObjectOf) (HWND, UINT, LPCITEMIDLIST*, REFIID, LPUINT, LPVOID*);
   STDMETHOD (GetDisplayNameOf) (LPCITEMIDLIST, DWORD, LPSTRRET);
   STDMETHOD (SetNameOf) (HWND, LPCITEMIDLIST, LPCOLESTR, DWORD, LPITEMIDLIST*);

   // IShellDetails methods
   STDMETHOD (ColumnClick) (UINT);
   STDMETHOD (GetDetailsOf) (LPCITEMIDLIST, UINT, LPSHELLDETAILS);

   // IPersist methods
   STDMETHOD (GetClassID) (LPCLSID);

   // IPersistFolder methods
   STDMETHOD (Initialize) (LPCITEMIDLIST);
};

