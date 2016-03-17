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

#include "fsdrop.h"
#include "datatrans.h"
#include "idlist.h"
#include "resource.h"
#include "shelldialogs.h"

//////////////////////////////////////////////////
// CFileFolder

CFileSystemDropTarget::CFileSystemDropTarget() :
   m_dwEffect(DROPEFFECT_NONE)
{
   m_ObjRefCount = 1;
}

BOOL CFileSystemDropTarget::Initialize(LPCWSTR pwszPath)
{
   ASSERT(pwszPath);
   if (!pwszPath)
      return FALSE;

   if (FAILED(StringCchCopy(m_wszPath, lengthof(m_wszPath), pwszPath)))
      return FALSE;

   if (!PathFileExists(m_wszPath))
      return FALSE;

   return TRUE;
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CFileSystemDropTarget::QueryInterface(REFIID riid,
                                                   LPVOID FAR * ppobj)
{
   HRESULT hr = E_NOINTERFACE;

   if (!ppobj)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   *ppobj = NULL;

   if(IsEqualIID(riid, IID_IUnknown)) // IUnknown
   {
      *ppobj = this;
   }
   else if(IsEqualIID(riid, IID_IDropTarget)) // IDropTarget
   {
      *ppobj = (IDropTarget *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CFileSystemDropTarget::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CFileSystemDropTarget::Release(VOID)
{
   if (0 == --m_ObjRefCount)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IDropTarget

STDMETHODIMP CFileSystemDropTarget::DragEnter(LPDATAOBJECT pDataObj, 
                                              DWORD dwKeyState, 
                                              POINTL pt, 
                                              LPDWORD pdwEffect)
{
   ASSERT(pDataObj);
   ASSERT(pdwEffect);
   if (!pDataObj || !pdwEffect)
      return E_INVALIDARG;

   DWORD dwAttrib = GetFileAttributes(m_wszPath);

   if ((-1 != dwAttrib) && PathIsLink(m_wszPath))
   {
      if (SHGetShortcutTarget(m_wszPath, m_wszPath, MAX_PATH))
      {
         PathRemoveQuotesAndArgs(m_wszPath);
         dwAttrib = GetFileAttributes(m_wszPath);
      }
      else
      {
         // Can't trace the target.
         dwAttrib = (DWORD) -1;
      }
   }

   if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
   {
      FORMATETC fe = {0};
      STGMEDIUM sm = {0};
      ShellDataTransfer sdt;
      WCHAR wszSrc[MAX_PATH];

      fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
      fe.ptd = NULL;
      fe.dwAspect = DVASPECT_CONTENT;
      fe.lindex = -1;
      fe.tymed = TYMED_HGLOBAL;

      if (SUCCEEDED(pDataObj->GetData(&fe, &sm)) &&
          ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
      {
         if (SHGetPathFromIDList(sdt.m_pidlFolder, wszSrc))
         {
            if (PathIsSameDevice(wszSrc, m_wszPath))
               m_dwEffect = DROPEFFECT_MOVE;
            else
               m_dwEffect = DROPEFFECT_COPY;
         }
         else
         {
            m_dwEffect = DROPEFFECT_NONE;
         }
         sdt.Free();  
      }
      else
      {
         m_dwEffect = DROPEFFECT_NONE;
      }
   }
   else
   {
      m_dwEffect = DROPEFFECT_NONE;
   }

   return NOERROR;
}


STDMETHODIMP CFileSystemDropTarget::DragOver(DWORD dwKeyState,
                                             POINTL pt,
                                             LPDWORD pdwEffect)
{
   ASSERT(pdwEffect);
   if (!pdwEffect)
      return E_INVALIDARG;

   *pdwEffect = m_dwEffect;

   return NOERROR;
}

STDMETHODIMP CFileSystemDropTarget::DragLeave()
{
   m_dwEffect = DROPEFFECT_NONE;

   return NOERROR;
}

STDMETHODIMP CFileSystemDropTarget::Drop(LPDATAOBJECT pDataObj,
                                         DWORD dwKeyState,
                                         POINTL pt,
                                         LPDWORD pdwEffect)
{
   ASSERT(pDataObj);
   ASSERT(pdwEffect);
   if (!pDataObj || !pdwEffect)
      return E_INVALIDARG;

   BOOL fReturn = TRUE;
   HRESULT hr = NOERROR;
   FORMATETC fe = {0};
   STGMEDIUM sm = {0};
   ShellDataTransfer sdt;
   WCHAR wszPathSrc[MAX_PATH];

   fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   fe.ptd = NULL;
   fe.dwAspect = DVASPECT_CONTENT;
   fe.lindex = -1;
   fe.tymed = TYMED_HGLOBAL;

   hr = pDataObj->GetData(&fe, &sm);
   if (FAILED(hr))
      return E_FAIL;

   if (!ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
      return E_FAIL;

   if (SHGetPathFromIDList(sdt.m_pidlFolder, wszPathSrc))
   {
      ASSERT(DROPEFFECT_NONE != *pdwEffect);

      BOOL fIsSameSrcDst = (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                                        wszPathSrc, -1, m_wszPath, -1));

      if (!fIsSameSrcDst || ((DROPEFFECT_COPY | DROPEFFECT_LINK) & *pdwEffect))
      {
         // The src and dst are not the same or they are the same and it's
         // a link or we've been told it's ok to try a same src/dst drop
         if (DROPEFFECT_COPY & *pdwEffect)
         {
            if (fIsSameSrcDst)
               sdt.m_dwFlags |= ShellDataTransfer::RENAME_ON_COLLISION;

            fReturn = sdt.TransferTo(ShellDataTransfer::COPY,
                                     m_wszPath);
         }
         else if (DROPEFFECT_MOVE & *pdwEffect)
         {
            fReturn = sdt.TransferTo(ShellDataTransfer::MOVE,
                                     m_wszPath);
         }
         else if (DROPEFFECT_LINK & *pdwEffect)
         {
            fReturn = sdt.TransferTo(ShellDataTransfer::LINK,
                                     m_wszPath);
         }
         else
         {
            fReturn = FALSE;
         }
      }
      else
      {
         // The src and dst are two different instances of the same folder
         HRESULT hrPrintf = E_FAIL;
         LPTSTR pszTitle = NULL;
         TCHAR szError[128];

         if (DROPEFFECT_MOVE & *pdwEffect)
         {
            pszTitle = MAKEINTRESOURCE(IDS_TITLE_MOVEERROR);
            hrPrintf = ::StringCchPrintf(szError, lengthof(szError), TEXT("%s%s"),
                                         LOAD_STRING(IDS_TITLE_MOVEERROR),
                                         LOAD_STRING(IDS_SHFO_ERR_SRCDSTSAME));
         }
         else
         {
            ASSERT(DROPEFFECT_MOVE & *pdwEffect);
         }

         if (pszTitle && SUCCEEDED(hrPrintf))
         {
            ShellDialogs::ShowFileError(NULL, pszTitle, szError,
                                        NULL, MB_ICONERROR | MB_OK);
         }

         fReturn = FALSE;
      }
   }
   else
   {
      fReturn = FALSE;
   }

   sdt.Free();

   DragLeave();

   return (fReturn ? NOERROR : E_FAIL);
}

