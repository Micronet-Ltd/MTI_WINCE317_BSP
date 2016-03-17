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

#include "recbindrop.h"
#include "recbin.h"
#include "datatrans.h"
#include "idlist.h"

//////////////////////////////////////////////////
// CFileFolder

CRecBinDropTarget::CRecBinDropTarget() :
   m_pwszPath(NULL),
   m_dwEffect(DROPEFFECT_NONE)
{
   m_ObjRefCount = 1;
}

CRecBinDropTarget::~CRecBinDropTarget()
{
   if (m_pwszPath)
   {
      g_pShellMalloc->Free(m_pwszPath);
   }
}

BOOL CRecBinDropTarget::Initialize()
{
   size_t cb = 0;
   
   HRESULT hr = StringCbLength(CRecycleBin::m_szBitBucketPath, MAX_PATH * sizeof(TCHAR), &cb);  
   if (SUCCEEDED(hr))
   {
      cb += sizeof(WCHAR); // '\0'
      m_pwszPath = (LPWSTR) g_pShellMalloc->Alloc(cb);

      if (m_pwszPath)
      {         
         memcpy(m_pwszPath, CRecycleBin::m_szBitBucketPath, cb);
         if (!PathFileExists(m_pwszPath))
         {
            g_pShellMalloc->Free(m_pwszPath);
            m_pwszPath = NULL;
            hr = E_FAIL; 
         }
      }
      else
      {
         hr = E_OUTOFMEMORY;
      }
   }

   return SUCCEEDED(hr);
}


//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CRecBinDropTarget::QueryInterface(REFIID riid,
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

STDMETHODIMP_(ULONG) CRecBinDropTarget::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CRecBinDropTarget::Release(VOID)
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
//DoDragDrop calls DragEnter when the mouse has moved over a window that is registered as a drag target.
//DragEnter determines whether a drop can be accepted and its effect if it is accepted.
STDMETHODIMP CRecBinDropTarget::DragEnter(LPDATAOBJECT pDataObj, 
                                              DWORD dwKeyState, 
                                              POINTL pt, 
                                              LPDWORD pdwEffect)
{
   HRESULT hr = E_UNEXPECTED;
   WCHAR wszDest[MAX_PATH];
   DWORD dwAttrib;

   if (!pDataObj || !pdwEffect)
   {
      ASSERT(0);
      hr = E_INVALIDARG;
      goto leave;
   }

   if (!m_pwszPath)
   {
      goto leave;
   }

   if (CSTR_EQUAL != CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                                   CRecycleBin::m_szBitBucketPath, -1, m_pwszPath, -1))
   {
      goto leave;
   }

   if (FAILED(StringCchCopy(wszDest, lengthof(wszDest), m_pwszPath)))
   {
      goto leave;
   }

   dwAttrib = GetFileAttributes(wszDest);

   if ((-1 != dwAttrib) && (FILE_ATTRIBUTE_DIRECTORY & dwAttrib))
   {
      FORMATETC fe = {0};
      STGMEDIUM sm = {0};
      ShellDataTransfer sdt;

      fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
      fe.ptd = NULL;
      fe.dwAspect = DVASPECT_CONTENT;
      fe.lindex = -1;
      fe.tymed = TYMED_HGLOBAL;

      if (SUCCEEDED(pDataObj->GetData(&fe, &sm)) &&
          ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
      {  
         m_dwEffect = DROPEFFECT_MOVE;
         if(ShellDataTransfer::CONTAINS_NAMESPACE_OBJECT && sdt.m_dwFlags)
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

   hr = NOERROR;

leave:
   return hr;
}

//The DoDragDrop function calls this method each time the user moves the mouse across a given target window. 
//DoDragDrop exits the loop if the drag-and-drop operation is canceled, if the user drags the mouse 
//out of the target window, or if the drop is completed. 
STDMETHODIMP CRecBinDropTarget::DragOver(DWORD dwKeyState,
                                             POINTL pt,
                                             LPDWORD pdwEffect)
{
   ASSERT(pdwEffect);
   if (!pdwEffect)
   {
      return E_INVALIDARG;
   }

   *pdwEffect = m_dwEffect;

   return NOERROR;
}

//The DoDragDrop function calls this method in either of the following cases: 
//When the user drags the cursor out of a given target window. 
//or
//When the user cancels the current drag-and-drop operation.
//DragLeave causes the drop target to suspend its feedback actions.
STDMETHODIMP CRecBinDropTarget::DragLeave()
{
   m_dwEffect = DROPEFFECT_NONE;

   return NOERROR;
}
//The DoDragDrop function calls Drop method when the user completes the drag-and-drop operation.
//Drop incorporates the source data into the target window, removes target feedback, and releases the data object.  
STDMETHODIMP CRecBinDropTarget::Drop(LPDATAOBJECT pDataObj,
                                         DWORD dwKeyState,
                                         POINTL pt,
                                         LPDWORD pdwEffect)
{
   ASSERT(pDataObj);
   ASSERT(pdwEffect);
   if (!pDataObj || !pdwEffect)
   {
      return E_INVALIDARG;
   }

   BOOL fReturn = TRUE;
   HRESULT hr = NOERROR;
   FORMATETC fe = {0};
   STGMEDIUM sm = {0};
   ShellDataTransfer sdt;

   fe.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CESHELL_DATA_TRANSFER);
   fe.ptd = NULL;
   fe.dwAspect = DVASPECT_CONTENT;
   fe.lindex = -1;
   fe.tymed = TYMED_HGLOBAL;

   hr = pDataObj->GetData(&fe, &sm);
   if (FAILED(hr))
   {
      return E_FAIL;
   }

   if (!ShellDataTransfer::Unpack(sm.hGlobal, &sdt))
   {
      return E_FAIL;
   }

   if (DROPEFFECT_MOVE & *pdwEffect)
   {
      fReturn = sdt.TransferTo(ShellDataTransfer::DELE, NULL);
   }
   else
   {
      fReturn = FALSE;
   }

   sdt.Free();

   DragLeave();

   return (fReturn ? NOERROR : E_FAIL);
}

