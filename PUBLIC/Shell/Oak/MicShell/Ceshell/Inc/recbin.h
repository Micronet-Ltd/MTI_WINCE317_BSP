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

class CRecycleBin :
   public IEnumIDList
{
   static const TCHAR BITBUCKET[];
   static const TCHAR EXPLORER[];
   static const TCHAR DBVOLUMENAME[];
   static const TCHAR RECYCLEINFO[];
   static const TCHAR RECYCLE_BIN_ENABLEDBFILE[];
   static const TCHAR RECYCLE_BIN_FLUSH[];

   static BOOL m_fcsInit;
   static BOOL m_fFlushVolume;
   static BOOL m_fMounted; 
   static CEGUID m_guid;
   static CEOID m_oid;
   static CRITICAL_SECTION m_cs;
   static HWND m_hwndDesktop;
   static HANDLE m_hDB;
   static TCHAR m_szCSIDLProfile[MAX_PATH];
   static TCHAR m_szFindFile[MAX_PATH];
   static UINT m_dwPercentage;
   
   DWORD m_dwInitMountTimeout;
   DWORD m_ObjRefCount;
   HANDLE m_hfind;   // INVALID_HANLDE_VALUE = uninitialized; NULL = enum done
   SHCONTF m_grfFlags;         
   WIN32_FIND_DATA m_FindData;

   UINT AddRecord(LPCTSTR, LPCTSTR);
   BOOL CanAddFile(HANDLE hFile, ULONGLONG * ullFileSize);
   UINT DeleteDatabase();
   HRESULT GetDBType();
   BOOL GetDBVolume (CEGUID * pguid);
   DWORD GetDWORD(HKEY hkeyRoot, LPCTSTR pszPath, LPCTSTR pszKey, DWORD defVal);
   CEOID GetFileInfo(LPCTSTR, LPTSTR, const size_t, LPFILETIME);
   UINT GetTempFileName(DWORD, LPCTSTR, LPTSTR*);
   HRESULT GetTypeAndInit();
   DWORD GetUsedRecycleBinSize(ULONGLONG * ullUsed);
   BOOL MeetsFindCriteria();
   BOOL Notify();
   UINT OpenDatabase();   
public:
   static UINT m_iNotObjectStore;
   static TCHAR m_szBitBucketPath[MAX_PATH];

   CRecycleBin(SHCONTF grfFlags = 0);
   ~CRecycleBin();

   HRESULT AddFileFolder(LPCWSTR pwszFullPath, LPITEMIDLIST * ppidl);
   void BeginRecycle(SHCONTF grfFlags = 0);
   void EndRecycle();
   HRESULT DeleteAll(BOOL fConfirmDel);
   HRESULT DeleteFileFolder(LPCITEMIDLIST pidl);
   BOOL GetFileDateDel(LPCITEMIDLIST pidl, FILETIME* pftDateDel);
   LPOLESTR GetFileOrigLoc(LPCITEMIDLIST pidl);
   LPOLESTR GetFileOrigName(LPCITEMIDLIST pidl);   
   BOOL GetFileSize(LPCITEMIDLIST pidl, ULONGLONG * pullSize);
   UINT GetPercentage();
   static void GlobalInit();
   static void GlobalCleanup();
   void Initialize();         //Enumeration
   BOOL IsFull();
   BOOL IsEmpty();
   HRESULT RestoreFile(LPCITEMIDLIST pidl);
   HRESULT RestoreAll();
   VOID SetDesktopWindow(HWND hwndDesktop);
   BOOL SetPercentage(UINT);
   ULONGLONG StoreSize();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IEnumIDList methods
   STDMETHOD (Next) (ULONG, LPITEMIDLIST*, ULONG*);
   STDMETHOD (Skip) (ULONG);
   STDMETHOD (Reset) (VOID);
   STDMETHOD (Clone) (IEnumIDList**);
};
