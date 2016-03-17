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

#include "datatrans.h"
#include "idlist.h"
#include "resource.h"
#include "shelldialogs.h"
#include "usersettings.h"

const DWORD ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT = 0x00000001;
const DWORD ShellDataTransfer::CONTAINS_COPY_DATA        = 0x00000002;
const DWORD ShellDataTransfer::CONTAINS_CUT_DATA         = 0x00000004;
const DWORD ShellDataTransfer::RENAME_ON_COLLISION       = 0x00000008;

//////////////////////////////////////////////////
// ShellDataTransfer

ShellDataTransfer::ShellDataTransfer() :
   m_dwFlags(0),
   m_hwndOwnerLV(NULL),
   m_pidlFolder(NULL),
   m_uItems(NULL),
   m_ppidlItems(NULL)
{
}

DWORD WINAPI ShellDataTransfer::TransferToThread(LPVOID lpParameter)
{
   PREFAST_ASSERT(lpParameter);
   TransferToData * pTransferToData = (TransferToData *) lpParameter;
   WCHAR wszFrom[MAX_PATH+2]; // For the "" necessary for SHCreateShortcutEx
   LPITEMIDLIST pidlFQ = NULL;
   BOOL fReturn = TRUE;

   if (LINK == pTransferToData->m_op)
   {
      HRESULT hr;
      LPWSTR pwszFrom = wszFrom+1;
      WCHAR wszShortcut[MAX_PATH];
      BOOL fFoundName = TRUE;

      wszFrom[0] = L'\"';
      for (UINT i = 0; i < pTransferToData->m_pShellDataTransfer->m_uItems; i++)
      {
         pidlFQ = ILConcatenate(pTransferToData->m_pShellDataTransfer->m_pidlFolder,
                                pTransferToData->m_pShellDataTransfer->m_ppidlItems[i]);
         if (!pidlFQ)
         {
            continue;
         }

         if (ILIsNameSpacePidl(ILFindLast(pidlFQ)))
         {
            hr = ::StringCchCopy(pwszFrom,
                                  (lengthof(wszFrom)-1),
                                  ILParsingName(ILFindLast(pidlFQ)));
            fFoundName = SUCCEEDED(hr);
         }
         else
         {
            fFoundName = SHGetPathFromIDList(pidlFQ, pwszFrom);
         }

         if (fFoundName)
         {
            hr = NOERROR;
            size_t cchTo = ::wcslen(pTransferToData->m_pwszTo);
            if (L'\\' == pTransferToData->m_pwszTo[cchTo])
            {
               hr = ::StringCchPrintf(wszShortcut,
                                     lengthof(wszShortcut),
                                     L"%s%s",
                                     pTransferToData->m_pwszTo,
                                     PathFindFileName(pwszFrom));
            }
            else
            {
               hr = ::StringCchPrintf(wszShortcut,
                                     lengthof(wszShortcut),
                                     L"%s\\%s",
                                     pTransferToData->m_pwszTo,
                                     PathFindFileName(pwszFrom));
            }

            if (SUCCEEDED(hr))
            {
               hr = ::StringCchCat(wszFrom, lengthof(wszFrom), L"\"");
            }

            if (FAILED(hr) || !SHCreateShortcutEx(wszShortcut, wszFrom, NULL, NULL))
            {
               ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(IDS_TITLE_SHORTCUT),
                                           MAKEINTRESOURCE(IDS_SHORTCUT_ERR),
                                           wszFrom, MB_ICONERROR | MB_OK);
            }
         }
         else
         {
            // The source file is missing, notify the user
            WCHAR wszError[256] = L"";

            ::StringCchPrintfEx(wszError,
                              lengthof(wszError),
                              NULL,
                              NULL,
                              STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                              LOAD_STRING(IDS_SHORTCUT_ERR),
                              ILDisplayName(ILFindLast(pidlFQ)));

            if (SUCCEEDED(StringCchCat(wszError, lengthof(wszError),
                                       LOAD_STRING(IDS_SHFO_ERR_FILENOTFOUND))))
            {
               ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(IDS_TITLE_SHORTCUT),
                                           wszError, NULL, MB_ICONERROR | MB_OK);
            }
            else
            {
               ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(IDS_TITLE_SHORTCUT),
                                           MAKEINTRESOURCE(IDS_SHFO_ERR_FILENOTFOUND),
                                           NULL, MB_ICONERROR | MB_OK);
            }
         }

         ILFree(pidlFQ);
      }
   }
   else
   {
      LPWSTR pwszFrom = NULL;
      DWORD dwFromLength = 0;
      DWORD dwNewFromLength = 0;
      SHFILEOPSTRUCT shfos = {0};

      // Build the list of source strings (double null terminated)
      for (UINT i = 0; i < pTransferToData->m_pShellDataTransfer->m_uItems; i++)
      {
         ASSERT(!ILIsNameSpacePidl(pTransferToData->m_pShellDataTransfer->m_ppidlItems[i]));
         pidlFQ = ILConcatenate(pTransferToData->m_pShellDataTransfer->m_pidlFolder,
                                pTransferToData->m_pShellDataTransfer->m_ppidlItems[i]);
         if (!pidlFQ)
         {
            continue;
         }

         if (SHGetPathFromIDList(pidlFQ, wszFrom))
         {
            dwNewFromLength += (wcslen(wszFrom)+1);
            pwszFrom = (LPWSTR) g_pShellMalloc->Alloc((dwNewFromLength+1)*sizeof(WCHAR));
            if (pwszFrom)
            {
               if (dwFromLength)
               {
                  memcpy(pwszFrom, shfos.pFrom, dwFromLength*sizeof(WCHAR));
               }

               if ((dwNewFromLength-dwFromLength) <= lengthof(wszFrom))
               {
                  memcpy(pwszFrom+dwFromLength, wszFrom, (dwNewFromLength-dwFromLength)*sizeof(WCHAR));
               }
               pwszFrom[dwNewFromLength] = L'\0'; // Double null terminate

               if (shfos.pFrom)
               {
                  g_pShellMalloc->Free((VOID *) shfos.pFrom);
               }
               shfos.pFrom = (LPCWSTR) pwszFrom;
               dwFromLength = dwNewFromLength;
            }
         }
         else
         {
            // The source file is missing, notify the user
            UINT uErrorTitle = 0;
            WCHAR wszError[256] = L"";

            switch (pTransferToData->m_op)
            {
               case COPY:
                  uErrorTitle = IDS_TITLE_COPYERROR;
                  ::StringCchPrintfEx(wszError,
                                    lengthof(wszError),
                                    NULL,
                                    NULL,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    LOAD_STRING(IDS_SHFO_ERR_HDR_COPY),
                                    ILDisplayName(ILFindLast(pidlFQ)));
               break;
 
               case MOVE:
                  uErrorTitle = IDS_TITLE_MOVEERROR;
                  ::StringCchPrintfEx(wszError,
                                    lengthof(wszError),
                                    NULL,
                                    NULL,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    LOAD_STRING(IDS_SHFO_ERR_HDR_MOVE),
                                    ILDisplayName(ILFindLast(pidlFQ)));
               break;

               case DELE:
                  uErrorTitle = IDS_TITLE_DELETEERROR;
                  ::StringCchPrintfEx(wszError,
                                    lengthof(wszError),
                                    NULL,
                                    NULL,
                                    STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                    LOAD_STRING(IDS_SHFO_ERR_HDR_DELETE),
                                    ILDisplayName(ILFindLast(pidlFQ)));
               break;
            }

            if (SUCCEEDED(StringCchCat(wszError, lengthof(wszError),
                                       LOAD_STRING(IDS_SHFO_ERR_FILENOTFOUND))))
            {
               ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(uErrorTitle),
                                           wszError, NULL, MB_ICONERROR | MB_OK);
            }
            else
            {
               ShellDialogs::ShowFileError(NULL, MAKEINTRESOURCE(uErrorTitle),
                                           MAKEINTRESOURCE(IDS_SHFO_ERR_FILENOTFOUND),
                                           NULL, MB_ICONERROR | MB_OK);
            }
         }

         ILFree(pidlFQ);
      }

      if (shfos.pFrom)
      {
         // Determine which operation the user wants to perform
         switch (pTransferToData->m_op)
         {
            case COPY:
               shfos.wFunc = FO_COPY;
            break;

            case MOVE:
               shfos.wFunc = FO_MOVE;
            break;

            case DELE:
               shfos.wFunc = FO_DELETE;
            break;

            default: 
               ASSERT((COPY == pTransferToData->m_op) ||
                  (MOVE == pTransferToData->m_op) ||
                  (DELETE == pTransferToData->m_op));
         }

         // No parent window in case the source folder goes away during the operation
         shfos.hwnd = NULL;

         // Set up the appropriate flags
         if (FO_DELETE == shfos.wFunc)
         {
            if (UserSettings::GetUseRecycleBin())
            {
               shfos.fFlags |= FOF_ALLOWUNDO;
            }
         }
         else
         {
            shfos.fFlags |= FOF_ALLOWUNDO;
         }

         // Allow for copy and paste in the same directory
         if (pTransferToData->m_pShellDataTransfer->m_dwFlags & RENAME_ON_COLLISION)
         {
            shfos.fFlags |= FOF_RENAMEONCOLLISION;
         }

         // Set the destination
         shfos.pTo = pTransferToData->m_pwszTo;

         // Do the file operation
         fReturn = (0 == SHFileOperation(&shfos));

         g_pShellMalloc->Free((VOID *) shfos.pFrom);
      }
   }

   // Clean up the TransferTo data
   if (pTransferToData->m_pwszTo)
   {
      g_pShellMalloc->Free(pTransferToData->m_pwszTo);
   }
   pTransferToData->m_pShellDataTransfer->Free();
   delete pTransferToData->m_pShellDataTransfer;
   delete pTransferToData;

   return fReturn;
}

void ShellDataTransfer::Free()
{
   if (m_ppidlItems)
   {
      for (UINT i = 0; i < m_uItems; i++)
      {
         if (m_ppidlItems[i])
         {
            ILFree(m_ppidlItems[i]);
            m_ppidlItems[i] = NULL;
         }
      }
      g_pShellMalloc->Free(m_ppidlItems);
      m_ppidlItems = NULL;
   }

   if (m_pidlFolder)
   {
      ILFree(m_pidlFolder);
      m_pidlFolder = NULL;
   }
}

ShellDataTransfer * ShellDataTransfer::Clone()
{
   ShellDataTransfer * pShellDataTransfer = new ShellDataTransfer();
   if (!pShellDataTransfer)
   {
      return NULL;
   }

   pShellDataTransfer->m_dwFlags = m_dwFlags;
   pShellDataTransfer->m_hwndOwnerLV = m_hwndOwnerLV;
   pShellDataTransfer->m_pidlFolder = NULL;
   pShellDataTransfer->m_uItems = m_uItems;
   pShellDataTransfer->m_ppidlItems = NULL;

   if (m_pidlFolder)
   {
      pShellDataTransfer->m_pidlFolder = ILCopy(m_pidlFolder, IL_ALL);
      ASSERT(pShellDataTransfer->m_pidlFolder);
   }

   if (m_uItems)
   {
      pShellDataTransfer->m_ppidlItems = (LPCITEMIDLIST *) g_pShellMalloc->Alloc(m_uItems * sizeof(LPITEMIDLIST));
      ASSERT(pShellDataTransfer->m_ppidlItems);
      if (pShellDataTransfer->m_ppidlItems)
      {
         for (UINT i = 0; i < m_uItems; i++)
         {
            if (m_ppidlItems[i])
            {
               pShellDataTransfer->m_ppidlItems[i] = ILCopy(m_ppidlItems[i], IL_ALL);
               ASSERT(pShellDataTransfer->m_ppidlItems[i]);
            }
         }
      }
   }

   return pShellDataTransfer;
}

BOOL ShellDataTransfer::TransferTo(OperationType op, LPCWSTR pwszTo)
{
   TransferToData * pTransferToData = NULL;
   HANDLE hThread = NULL;

   // Validate parameters
   if (DELE == op)
   {
      ASSERT(!pwszTo);
   }
   else
   {
      ASSERT(pwszTo);
      if (!pwszTo)
      {
         return FALSE;
      }
   }

   // Setup the data to fire off to the thread
   pTransferToData = new TransferToData();
   if (!pTransferToData)
   {
      goto TransferTo_error;
   }

   pTransferToData->m_pShellDataTransfer = NULL;
   pTransferToData->m_op = op;
   pTransferToData->m_pwszTo = NULL;

   pTransferToData->m_pShellDataTransfer = this->Clone();
   if (!pTransferToData->m_pShellDataTransfer)
   {
      goto TransferTo_error;
   }

   if (pwszTo)
   {
      size_t cch = 0;
      if (FAILED(StringCchLength(pwszTo, MAX_PATH, &cch)))
      {
         goto TransferTo_error;
      }

      pTransferToData->m_pwszTo = (LPWSTR) g_pShellMalloc->Alloc((cch+1)*sizeof(WCHAR));
      if (!pTransferToData->m_pwszTo)
      {
         goto TransferTo_error;
      }

      ::wcscpy(pTransferToData->m_pwszTo, pwszTo);
   }

   hThread = ::CreateThread(NULL, 0, TransferToThread, pTransferToData, 0, NULL);
   if (hThread)
   {
      ::CloseHandle(hThread);
      return TRUE;
   }

TransferTo_error:

   if (pTransferToData)
   {
      if (pTransferToData->m_pwszTo)
      {
         g_pShellMalloc->Free(pTransferToData->m_pwszTo);
      }

      if (pTransferToData->m_pShellDataTransfer)
      {
         pTransferToData->m_pShellDataTransfer->Free();
         delete pTransferToData->m_pShellDataTransfer;
      }

      delete pTransferToData;
   }

   return FALSE;
}

HGLOBAL ShellDataTransfer::Pack(ShellDataTransfer * pDataTrans)
{
   PREFAST_ASSERT(pDataTrans);
   ASSERT(pDataTrans->m_pidlFolder);
   ASSERT(pDataTrans->m_ppidlItems);
   if (!pDataTrans ||!pDataTrans->m_pidlFolder || !pDataTrans->m_ppidlItems)
   {
      return NULL;
   }

   PBYTE pData = NULL;
   DWORD cbFolder = 0;
   DWORD cbItems = 0;
   DWORD cbData = 0;

   cbFolder = ILSize(pDataTrans->m_pidlFolder, IL_ALL);
   ASSERT(0 != cbFolder);

   for (UINT i = 0; i < pDataTrans->m_uItems; i++)
   {
      ASSERT(1 == ILCount(pDataTrans->m_ppidlItems[i]));
      cbItems += ILSize(pDataTrans->m_ppidlItems[i], 1);
   }
   ASSERT(0 != cbItems);

   pData = (PBYTE) LocalAlloc(LPTR, sizeof(DWORD)+sizeof(HWND)+cbFolder+sizeof(UINT)+cbItems);
   if (!pData)
   {
      return NULL;
   }

   // <DWORD flags>
   memcpy(pData, &pDataTrans->m_dwFlags, sizeof(DWORD));
   cbData += sizeof(DWORD);

   // <HWND owner>
   memcpy(pData+cbData, &pDataTrans->m_hwndOwnerLV, sizeof(HWND));
   cbData += sizeof(HWND);

   // <fully qualified pidl path to folder>
   memcpy(pData+cbData, pDataTrans->m_pidlFolder, cbFolder);
   cbData += cbFolder;

   // <UINT count of items>
   memcpy(pData+cbData, &pDataTrans->m_uItems, sizeof(UINT));
   cbData += sizeof(UINT);

   // <single pidl array of items>
   for (UINT j = 0; j < pDataTrans->m_uItems; j++)
   {
      memcpy(pData+cbData,
             pDataTrans->m_ppidlItems[j],
             ILSize(pDataTrans->m_ppidlItems[j], 1));
      cbData += ILSize(pDataTrans->m_ppidlItems[j], 1);
   }

   ASSERT(sizeof(DWORD)+sizeof(HWND)+cbFolder+sizeof(UINT)+cbItems == cbData);
   return pData;
}

BOOL ShellDataTransfer::Unpack(HGLOBAL hData, ShellDataTransfer * pDataTrans)
{
   ASSERT(hData);
   ASSERT(pDataTrans);
   if (!hData || !pDataTrans)
   {
      return FALSE;
   }

   PBYTE pData = (PBYTE) hData;

   // <DWORD flags>
   memcpy(&pDataTrans->m_dwFlags, pData, sizeof(DWORD));
   pData += sizeof(DWORD);

   // <HWND owner>
   memcpy(&pDataTrans->m_hwndOwnerLV, pData, sizeof(HWND));
   pData += sizeof(HWND);

   // <fully qualified pidl path to folder>
   pDataTrans->m_pidlFolder = ILCopy((LPCITEMIDLIST) pData, IL_ALL);
   ASSERT(pDataTrans->m_pidlFolder);
   pData += ILSize(pDataTrans->m_pidlFolder, IL_ALL);

   // <UINT count of items>
   memcpy(&pDataTrans->m_uItems, pData, sizeof(UINT));
   ASSERT(0 != pDataTrans->m_uItems);
   pData += sizeof(DWORD);

   // <single pidl array of items>
   pDataTrans->m_ppidlItems = (LPCITEMIDLIST *) g_pShellMalloc->Alloc(pDataTrans->m_uItems * sizeof(LPITEMIDLIST));
   ASSERT(pDataTrans->m_ppidlItems);
   for (UINT i = 0; i < pDataTrans->m_uItems; i++)
   {
      pDataTrans->m_ppidlItems[i] = ILCopy((LPCITEMIDLIST) pData, 1);
      pData += ILSize(pDataTrans->m_ppidlItems[i], 1);
   }

   return TRUE;
}


//////////////////////////////////////////////////
// CShellDataObject

CShellDataObject::CShellDataObject() :
   m_ObjRefCount(0),
   m_hData(NULL)
{
   m_ObjRefCount = 1;
}
 
CShellDataObject::~CShellDataObject()
{
   if (m_hData)
   {
      LocalFree(m_hData);
   }
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CShellDataObject::QueryInterface(REFIID riid,
                                              LPVOID FAR * ppobj)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppobj)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   if(IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppobj = this;
   }
   else if(IsEqualIID(riid, IID_IDataObject)) // IDataObject
   {
      *ppobj = (IDataObject *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CShellDataObject::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CShellDataObject::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IDataObject

STDMETHODIMP CShellDataObject::GetData(FORMATETC * pFormatetc,
                                       STGMEDIUM * pmedium)
{
   ASSERT(pFormatetc);
   ASSERT(pmedium);
   if (!pFormatetc || !pmedium)
   {
      return E_INVALIDARG;
   }

   HRESULT hr = QueryGetData(pFormatetc);
   if (SUCCEEDED(hr))
   {
      pmedium->tymed = TYMED_HGLOBAL;
      pmedium->hGlobal = m_hData;
      pmedium->pUnkForRelease = NULL;
   }

   return hr;
}

STDMETHODIMP CShellDataObject::GetDataHere(FORMATETC * pFormatetc,
                                           STGMEDIUM * pmedium)
{
   return E_NOTIMPL;
}

STDMETHODIMP CShellDataObject::QueryGetData(FORMATETC * pFormatetc)
{
   if (!pFormatetc)
   {
      return E_INVALIDARG;
   }

   if (RegisterClipboardFormat(CESHELL_DATA_TRANSFER) != pFormatetc->cfFormat)
   {
      return DV_E_FORMATETC;
   }

   if (NULL != pFormatetc->ptd)
   {
      return DV_E_FORMATETC;
   }

   if (DVASPECT_CONTENT != pFormatetc->dwAspect)
   {
      return DV_E_DVASPECT;
   }

   if (-1 != pFormatetc->lindex)
   {
      return DV_E_LINDEX;
   }

   if (TYMED_HGLOBAL != pFormatetc->tymed)
   {
      return DV_E_TYMED;
   }

   return NOERROR;
}

STDMETHODIMP CShellDataObject::GetCanonicalFormatEtc(FORMATETC * pFormatetcIn,
                                                     FORMATETC * pFormatetcOut)
{
   return E_NOTIMPL;
}

STDMETHODIMP CShellDataObject::SetData(FORMATETC * pFormatetc,
                                       STGMEDIUM * pmedium,
                                       BOOL fRelease)
{
   ASSERT(pFormatetc);
   ASSERT(pmedium);
   if (!pFormatetc || !pmedium)
   {
      return E_INVALIDARG;
   }

   HRESULT hr = QueryGetData(pFormatetc);
   if (SUCCEEDED(hr))
   {
      if (pmedium->hGlobal)
      {
         ASSERT(TYMED_HGLOBAL == pmedium->tymed);
         if (fRelease)
         {
            m_hData = pmedium->hGlobal;
         }
         else
         {
            size_t cb = GlobalSize(pmedium->hGlobal);
            m_hData = GlobalAlloc(LPTR, cb);
            if (m_hData)
            {
               memcpy(m_hData, pmedium->hGlobal, cb);
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
         }
      }
      else
      {
         hr = E_FAIL;
      }
   }

   return hr;
}

STDMETHODIMP CShellDataObject::EnumFormatEtc(DWORD dwDirection,
                                             IEnumFORMATETC ** ppenumFormatetc)
{
   return E_NOTIMPL;
}

STDMETHODIMP CShellDataObject::DAdvise(FORMATETC * pFormatetc,
                                       DWORD advf,
                                       IAdviseSink * pAdvSink,
                                       DWORD * pdwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CShellDataObject::DUnadvise(DWORD dwConnection)
{
   return E_NOTIMPL;
}

STDMETHODIMP CShellDataObject::EnumDAdvise(IEnumSTATDATA ** ppenumAdvise)
{
   return E_NOTIMPL;
}


//////////////////////////////////////////////////
// CShellClipboard

void CShellClipboard::ClearClipboardData()
{
   // Note that this assumes the the clipboard has already been opened
   UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   HGLOBAL hData = ::GetClipboardData(uFormat);

   if (hData)
   {
      ShellDataTransfer sdt;
      if (ShellDataTransfer::Unpack(hData, &sdt))
      {
         // Unhilite any previous selections and clear the clipboard
         ListView_SetItemState(sdt.m_hwndOwnerLV, -1, 0, LVIS_CUT);
         sdt.Free();
      }
   }

   ::EmptyClipboard();
}

DWORD WINAPI CShellClipboard::FileOperationThread(LPVOID lpParameter)
{
   PREFAST_ASSERT(lpParameter);
   SHFILEOPSTRUCT * pshfos = (SHFILEOPSTRUCT *) lpParameter;

   SHFileOperation(pshfos);

   g_pShellMalloc->Free((PVOID)pshfos->pFrom);
   g_pShellMalloc->Free((PVOID)pshfos->pTo);
   g_pShellMalloc->Free((PVOID)pshfos);

   return 0;
}

BOOL CShellClipboard::SetClipboardData(HWND hwndLV,
                                       IShellFolder * pShellFolder,
                                       UINT uItemCount, 
                                       LPCITEMIDLIST * aPidls,
                                       DWORD dwFlags)
{
   PREFAST_ASSERT(pShellFolder);
   ASSERT(aPidls);

   BOOL fReturn = FALSE;

   IDataObject * pDataObject = NULL;
   if (SUCCEEDED(pShellFolder->GetUIObjectOf(hwndLV, uItemCount, aPidls, 
                                             IID_IDataObject, NULL, 
                                             (LPVOID *) &pDataObject)))
   {
      FORMATETC fe = {0};
      STGMEDIUM sm = {0};
      UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);

      fe.cfFormat = (CLIPFORMAT) uFormat;
      fe.ptd = NULL;
      fe.dwAspect = DVASPECT_CONTENT;
      fe.lindex = -1;
      fe.tymed = TYMED_HGLOBAL;

      if (SUCCEEDED(pDataObject->GetData(&fe, &sm)))
      {
         if (::OpenClipboard(NULL))
         {
            HGLOBAL hData = NULL;
            ShellDataTransfer sdt;

            ClearClipboardData();

            // Add the copy flag into the data and set into the clipboard
            if (ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
            {
               sdt.m_dwFlags |= dwFlags;
               hData = ShellDataTransfer::Pack(&sdt);
               if (hData)
               {
                  fReturn = (BOOL) ::SetClipboardData(uFormat, hData);
               }
               sdt.Free();
            }

            if (fReturn && UserSettings::GetUse2ndClipboard())
            {
               SetClipboardData2();
            }

            ::CloseClipboard();
         }
      }

      pDataObject->Release();
   }

   return fReturn;
}

void CShellClipboard::SetClipboardData2()
{
   // Note that this assumes the the clipboard has already been opened
   UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   HGLOBAL hData = NULL;
   ShellDataTransfer sdt;
   BOOL fFreeShellDataTransfer = FALSE;
   IShellFolder * psfDesktop = NULL;
   IShellFolder * psfParent = NULL;
   UINT uItem = 0;
   STRRET str = {0};
   WCHAR sz[MAX_PATH];
   size_t cch = 0;
   PBYTE pData = NULL;
   BOOL fFreeAlloc = FALSE;
   LPDROPFILES pDropFiles = NULL;

   // Get the current data from the clipboard
   hData = ::GetClipboardData(uFormat);
   if (!hData)
   {
      goto SetClipboardDataEx_done;
   }

   // Read the data into a useable format
   if (!ShellDataTransfer::Unpack(hData, &sdt))
   {
      goto SetClipboardDataEx_done;
   }
   fFreeShellDataTransfer = TRUE;

   // Get the desktop folder to bind down to the parent folder
   if (FAILED(SHGetDesktopFolder(&psfDesktop)))
   {
      goto SetClipboardDataEx_done;
   }

   // Bind to the parent folder
   if (FAILED(psfDesktop->BindToObject(sdt.m_pidlFolder, NULL,
                                       IID_IShellFolder,
                                       (PVOID*) &psfParent)))
   {
      goto SetClipboardDataEx_done;
   }

   // Figure out how much memory we will need to allocate
   for (uItem = 0; uItem < sdt.m_uItems; uItem++)
   {
      if (FAILED(psfParent->GetDisplayNameOf(sdt.m_ppidlItems[uItem],
                                             SHGDN_NORMAL | SHGDN_FORPARSING,
                                             &str)) ||
          FAILED(StrRetToBuf(&str, sdt.m_ppidlItems[uItem], sz, lengthof(sz))))
      {
         goto SetClipboardDataEx_done;
      }

      cch += (::wcslen(sz)+1);
   }
 
   // Allocate the memory (DROPFILES + all of the strings + null termiator)
   pData = (PBYTE) ::LocalAlloc(LPTR, sizeof(DROPFILES)+(cch*sizeof(WCHAR))+sizeof(WCHAR));
   if (!pData)
   {
      goto SetClipboardDataEx_done;
   }
   fFreeAlloc = TRUE;

   // Fill in the DROPFILES structure
   pDropFiles = (LPDROPFILES) pData;
   pDropFiles->pFiles = sizeof(DROPFILES);
   pDropFiles->fWide = TRUE;
   pData += sizeof(DROPFILES);

   // Fill in the strings
   for (uItem = 0; uItem < sdt.m_uItems; uItem++)
   {
      if (FAILED(psfParent->GetDisplayNameOf(sdt.m_ppidlItems[uItem],
                                             SHGDN_NORMAL | SHGDN_FORPARSING,
                                             &str)) ||
          FAILED(StrRetToBuf(&str, sdt.m_ppidlItems[uItem], sz, lengthof(sz))))
      {
         goto SetClipboardDataEx_done;
      }

      cch = (::wcslen(sz)+1);
      memcpy(pData, sz, (cch*sizeof(WCHAR)));
      pData += (cch*sizeof(WCHAR));
   }

   if (::SetClipboardData(CF_HDROP, pDropFiles))
   {
      fFreeAlloc = FALSE;
   }


SetClipboardDataEx_done:

   if (fFreeAlloc)
   {
      ::LocalFree(pData);
   }

   if (psfParent)
   {
      psfParent->Release();
   }

   if (psfDesktop)
   {
      psfDesktop->Release();
   }

   if (fFreeShellDataTransfer)
   {
      sdt.Free();
   }
}


BOOL CShellClipboard::Cut(HWND hwndLV,
                          IShellFolder * pShellFolder,
                          UINT uItemCount, 
                          LPCITEMIDLIST * aPidls)
{
   ASSERT(pShellFolder);
   ASSERT(aPidls);
   if (!pShellFolder || !aPidls)
   {
      return FALSE;
   }

   return SetClipboardData(hwndLV, pShellFolder, uItemCount, aPidls,
                           ShellDataTransfer::CONTAINS_CUT_DATA);
}

BOOL CShellClipboard::Copy(HWND hwndLV,
                           IShellFolder * pShellFolder,
                           UINT uItemCount, 
                           LPCITEMIDLIST * aPidls)
{
   ASSERT(pShellFolder);
   ASSERT(aPidls);
   if (!pShellFolder || !aPidls)
   {
      return FALSE;
   }

   return SetClipboardData(hwndLV, pShellFolder, uItemCount, aPidls,
                           ShellDataTransfer::CONTAINS_COPY_DATA);
}

BOOL CShellClipboard::Paste(HWND hwndLV,
                            LPCITEMIDLIST pidlToFQ,
                            BOOL fPasteAsShortcut)
{
   if (!::OpenClipboard(NULL))
   {
      return FALSE;
   }

   BOOL fSuccess = FALSE;
   STGMEDIUM sm = {0};
   UINT uFormat = RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   sm.hGlobal = ::GetClipboardDataAlloc(uFormat);

   if (sm.hGlobal)
   {
      IShellFolder * pFolder = NULL;
      LPCITEMIDLIST pidlLast = NULL;

      if (SUCCEEDED(SHBindToParent(pidlToFQ, IID_IShellFolder,
                                   (VOID **) &pFolder, &pidlLast)))
      {
         IDropTarget * pDropTarget = NULL;
         if (SUCCEEDED(pFolder->GetUIObjectOf(hwndLV, 1, &pidlLast,
                                              IID_IDropTarget, NULL,
                                              (VOID **) &pDropTarget)))
         {
            CShellDataObject * pDataObject = new CShellDataObject();
            if (pDataObject)
            {
               ShellDataTransfer sdt;
               FORMATETC fe = {0};
               fe.cfFormat = (CLIPFORMAT) uFormat;
               fe.ptd = NULL;
               fe.dwAspect = DVASPECT_CONTENT;
               fe.lindex = -1;
               fe.tymed = sm.tymed = TYMED_HGLOBAL;

               if (SUCCEEDED(pDataObject->SetData(&fe, &sm, FALSE)) &&
                   ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
               {
                  POINTL pt = { -1, -1 };
                  DWORD dwEffect = DROPEFFECT_NONE;

                  if (!fPasteAsShortcut)
                  {
                     if (ShellDataTransfer::CONTAINS_CUT_DATA & sdt.m_dwFlags)
                     {
                        dwEffect = DROPEFFECT_MOVE;
                     }

                     if (ShellDataTransfer::CONTAINS_COPY_DATA & sdt.m_dwFlags)
                     {
                        dwEffect = DROPEFFECT_COPY;
                     }

                     if (ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT & sdt.m_dwFlags)
                     {
                        dwEffect = DROPEFFECT_LINK;
                     }
                  }
                  else
                  {
                     dwEffect = DROPEFFECT_LINK;
                  }

                  fSuccess = SUCCEEDED(pDropTarget->Drop(pDataObject, 0,
                                                         pt, &dwEffect));

                  if (!fPasteAsShortcut)
                  {
                     if (ShellDataTransfer::CONTAINS_CUT_DATA & sdt.m_dwFlags)
                     {
                        ClearClipboardData();
                     }
                  }

                  sdt.Free();
               }

               pDataObject->Release();
            }

            pDropTarget->Release();
         }

         pFolder->Release();
         ILFree(pidlLast);
      }

      ::GlobalFree(sm.hGlobal);
   }
   else
   {
      if (UserSettings::GetUse2ndClipboard())
      {
         LPDROPFILES pDropFiles = (LPDROPFILES) ::GetClipboardData(CF_HDROP);
         WCHAR szTo[MAX_PATH];
         size_t cb = sizeof(SHFILEOPSTRUCT);
         SHFILEOPSTRUCT * pshfos = (SHFILEOPSTRUCT *) g_pShellMalloc->Alloc(cb);

         if (pDropFiles && SHGetPathFromIDList(pidlToFQ, szTo) && pshfos)
         {
            HANDLE hThread = NULL;
            LPWSTR pFrom = (LPWSTR) ((LPBYTE) pDropFiles + pDropFiles->pFiles);
            LPWSTR psz = (LPWSTR) pFrom;

            ASSERT(sizeof(DROPFILES) == pDropFiles->pFiles);

            pshfos->hwnd = NULL;
            pshfos->wFunc = FO_COPY;
            pshfos->fFlags = FOF_ALLOWUNDO;
            pshfos->fAnyOperationsAborted = FALSE;
            pshfos->hNameMappings = NULL;
            pshfos->lpszProgressTitle = NULL;

            while (*psz)
            {
               psz += (_tcslen(psz)+1);
            }
            cb = ((psz-pFrom+1) * sizeof(WCHAR));

            pshfos->pFrom = (LPCWSTR) g_pShellMalloc->Alloc(cb);
            if (pshfos->pFrom)
            {
               ::memcpy((PVOID)pshfos->pFrom, pFrom, cb);

               cb = ((_tcslen(szTo)+1) * sizeof(WCHAR));

               pshfos->pTo = (LPCWSTR) g_pShellMalloc->Alloc(cb);
               if (pshfos->pTo)
               {
                  ::memcpy((PVOID)pshfos->pTo, szTo, cb);

                  hThread = ::CreateThread(NULL, 0, FileOperationThread,
                                           pshfos, 0, NULL);
                  if (hThread)
                  {
                     ::CloseHandle(hThread);
                     fSuccess = TRUE;
                  }
               }
            }

            if (!fSuccess)
            {
               g_pShellMalloc->Free((PVOID)pshfos->pFrom);
               g_pShellMalloc->Free((PVOID)pshfos->pTo);
               g_pShellMalloc->Free((PVOID)pshfos);
            }
         }
      }
   }

   ::CloseClipboard();
   return fSuccess;
}

DWORD CShellClipboard::GetContentType()
{
   DWORD dwType = 0;
   UINT uFormat;
   HGLOBAL hData;

   if (!::OpenClipboard(NULL))
   {
      goto leave;
   }

   uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   hData = ::GetClipboardData(uFormat);

   if (hData)
   {
      ShellDataTransfer sdt;
      if (ShellDataTransfer::Unpack(hData, &sdt))
      {
         dwType = (sdt.m_dwFlags &
            (ShellDataTransfer::CONTAINS_CUT_DATA |
            ShellDataTransfer::CONTAINS_COPY_DATA |
            ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT));

         sdt.Free();
      }
   }

   ::CloseClipboard();

leave:
   return dwType;
}

void CShellClipboard::HandleEscape(HWND hwndLV)
{
   if (!::OpenClipboard(NULL))
   {
      return;
   }

   UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   HGLOBAL hData = ::GetClipboardData(uFormat);

   if (hData)
   {
      ShellDataTransfer sdt;
      if (ShellDataTransfer::Unpack(hData, &sdt))
      {
         if ((hwndLV == sdt.m_hwndOwnerLV) &&
             (ShellDataTransfer::CONTAINS_CUT_DATA & sdt.m_dwFlags))
         {
            ClearClipboardData();
         }

         sdt.Free();
      }
   }

   ::CloseClipboard();
}

BOOL CShellClipboard::HasData()
{
   if (!::OpenClipboard(NULL))
   {
      return FALSE;
   }

   UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   UINT uEnum = ::EnumClipboardFormats(0);
   BOOL fHasData = FALSE;

   while (0 != uEnum)
   {
      if (uFormat == uEnum)
      {
         fHasData = TRUE;
         break;
      }

      if ((CF_HDROP == uEnum) && UserSettings::GetUse2ndClipboard())
      {
         fHasData = TRUE;
         break;
      }

      uEnum = ::EnumClipboardFormats(uEnum);
   }

   ::CloseClipboard();
   return fHasData;
}

BOOL CShellClipboard::ShouldHiliteAsCut(HWND hwndLV, LPCITEMIDLIST pidl)
{
   if (!::OpenClipboard(NULL))
   {
      return FALSE;
   }

   BOOL fShouldHiliteAsCut = FALSE;
   UINT uFormat = ::RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   HGLOBAL hData = ::GetClipboardData(uFormat);

   if (hData)
   {
      ShellDataTransfer sdt;
      if (ShellDataTransfer::Unpack(hData, &sdt))
      {
         if ((hwndLV == sdt.m_hwndOwnerLV) &&
             (ShellDataTransfer::CONTAINS_CUT_DATA & sdt.m_dwFlags))
         {
            for (UINT i = 0; i < sdt.m_uItems; i++)
            {
               if (ILIsEqualParsingNames(pidl, sdt.m_ppidlItems[i]))
               {
                  FILETIME ft1, ft2;
                  if (ILGetDateCached(pidl, &ft1) &&
                    ILGetDateCached(sdt.m_ppidlItems[i], &ft2) &&
                    (0 != CompareFileTime(&ft1, &ft2)))
                  {
                     // Compare time stamp if possible and make sure we are
                     // talking about the same guy. Bail if not equal.
                     break;
                  }
                  fShouldHiliteAsCut = TRUE;
                  break;
               }
            }
         }

         sdt.Free();
      }
   }

   ::CloseClipboard();
   return fShouldHiliteAsCut;
}

