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

class CFileSystemEnum :
   public IEnumIDList,
   public IEnumFilter
{
private:
   DWORD m_ObjRefCount;
   LPCITEMIDLIST m_pidlFQ;
   HANDLE m_hfind; // INVALID_HANLDE_VALUE = uninitialized; NULL = enum done
   WIN32_FIND_DATA m_FindData;
   SHCONTF m_grfFlags;
   LPWSTR m_pszFilter;
   LPWSTR m_pszRecycleBin;

   BOOL MeetsFindCriteria();

public:
   CFileSystemEnum(SHCONTF grfFlags = 0);
   ~CFileSystemEnum();

   BOOL Initialize(LPCITEMIDLIST pidlFQ);

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

