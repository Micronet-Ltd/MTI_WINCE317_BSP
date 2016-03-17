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

#define CESHELL_DATA_TRANSFER TEXT("SHELL_DATA_TRANSFER")

class ShellDataTransfer
{
public:
   enum OperationType { COPY, MOVE, LINK, DELE };

protected:
   struct TransferToData
   {
      ShellDataTransfer * m_pShellDataTransfer;
      OperationType m_op;
      LPWSTR m_pwszTo;
   };
   static DWORD WINAPI TransferToThread(LPVOID lpParameter);

public:
   DWORD m_dwFlags;
   HWND m_hwndOwnerLV;
   LPCITEMIDLIST m_pidlFolder;
   UINT m_uItems;
   LPCITEMIDLIST * m_ppidlItems;

   static const DWORD CONTAINS_NAMESPACE_OBJECT;
   static const DWORD CONTAINS_COPY_DATA;
   static const DWORD CONTAINS_CUT_DATA;
   static const DWORD RENAME_ON_COLLISION;

   ShellDataTransfer();
   ~ShellDataTransfer() {}
   void Free();
   ShellDataTransfer * Clone();

   BOOL TransferTo(OperationType op, LPCWSTR pwszTo);

   // Packed as :
   //   <DWORD flags>
   //   <HWND owner listview>
   //   <fully qualified pidl path to folder>
   //   <UINT count of items>
   //   <single pidl array of items>

   static HGLOBAL Pack(ShellDataTransfer * pDataTrans);
   static BOOL Unpack(HGLOBAL hData, ShellDataTransfer * pDataTrans);
};

class CShellDataObject :
   public IDataObject
{
protected:
   DWORD m_ObjRefCount;
   HGLOBAL m_hData;

public:
   CShellDataObject();
   ~CShellDataObject();

   //IUnknown methods
   STDMETHOD (QueryInterface) (REFIID riid, LPVOID * ppvObj);
   STDMETHOD_ (ULONG, AddRef) (VOID);
   STDMETHOD_ (ULONG, Release) (VOID);

   //IDataObject methods
   STDMETHOD (GetData) (FORMATETC *, STGMEDIUM *);
   STDMETHOD (GetDataHere) (FORMATETC *, STGMEDIUM *);
   STDMETHOD (QueryGetData) (FORMATETC *);
   STDMETHOD (GetCanonicalFormatEtc) (FORMATETC *, FORMATETC *);
   STDMETHOD (SetData) (FORMATETC *, STGMEDIUM *, BOOL);
   STDMETHOD (EnumFormatEtc) (DWORD, IEnumFORMATETC **);
   STDMETHOD (DAdvise) (FORMATETC *, DWORD, IAdviseSink *, DWORD *);
   STDMETHOD (DUnadvise) (DWORD);
   STDMETHOD (EnumDAdvise) (IEnumSTATDATA **);
};

class CShellClipboard
{
protected:
   static void ClearClipboardData();
   static DWORD WINAPI FileOperationThread(LPVOID lpParameter);
   static BOOL SetClipboardData(HWND hwndLV, IShellFolder * pShellFolder,
                                UINT uItemCount, LPCITEMIDLIST * aPidls,
                                DWORD dwFlags);
   static void SetClipboardData2();

public:
   static BOOL Cut(HWND hwndLV, IShellFolder * pShellFolder,
                   UINT uItemCount, LPCITEMIDLIST * aPidls);
   static BOOL Copy(HWND hwndLV, IShellFolder * pShellFolder,
                    UINT uItemCount, LPCITEMIDLIST * aPidls);
   static BOOL Paste(HWND hwndLV, LPCITEMIDLIST pidlToFQ,
                     BOOL fPasteAsShortcut = FALSE);
   static DWORD GetContentType();
   static void HandleEscape(HWND hwndLV);
   static BOOL HasData();
   static BOOL ShouldHiliteAsCut(HWND hwndLV, LPCITEMIDLIST pidl);
};

