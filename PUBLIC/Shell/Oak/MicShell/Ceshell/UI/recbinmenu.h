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

class CRecycleBinMenu :
   public IContextMenu
{
private:
   UINT m_idAddCmd;
   DWORD m_ObjRefCount;
   
public:
   CRecycleBinMenu();
   ~CRecycleBinMenu();

   BOOL Initialize(LPCWSTR pwszPath);

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IContextMenu methods
   STDMETHOD (GetCommandString) (UINT_PTR, UINT, UINT *, LPSTR, UINT);
   STDMETHOD (InvokeCommand) (LPCMINVOKECOMMANDINFO);
   STDMETHOD (QueryContextMenu) (HMENU hmenu, UINT, UINT, UINT, UINT);
};

