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

#include "recbin.h"
#include "recbinmenu.h"
#include "resource.h"

//////////////////////////////////////////////////
// CRecycleBinMenu

CRecycleBinMenu::CRecycleBinMenu() :
   m_ObjRefCount(0),
   m_idAddCmd(0)
{
   m_ObjRefCount = 1;
}
 
CRecycleBinMenu::~CRecycleBinMenu()
{
}

//////////////////////////////////////////////////
// IUnknown

STDMETHODIMP CRecycleBinMenu::QueryInterface(REFIID riid,
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
   else if(IsEqualIID(riid, IID_IContextMenu)) // IContextMenu
   {
      *ppobj = (IContextMenu *) this;
   }

   if(*ppobj)
   {
      (*(LPUNKNOWN*)ppobj)->AddRef();
      hr = S_OK;
   }

leave:
   return hr;
}

STDMETHODIMP_(ULONG) CRecycleBinMenu::AddRef(VOID)
{
   return ++m_ObjRefCount;
}

STDMETHODIMP_(ULONG) CRecycleBinMenu::Release(VOID)
{
   if (--m_ObjRefCount == 0)
   {
      delete this;
      return 0;
   }
   
   return m_ObjRefCount;
}


//////////////////////////////////////////////////
// IContextMenu 

STDMETHODIMP CRecycleBinMenu::GetCommandString(UINT_PTR idCmd,
                                               UINT uFlags,
                                               UINT * pwReserved,
                                               LPSTR pszName,
                                               UINT cchMax)
{
   return E_NOTIMPL;
}

STDMETHODIMP CRecycleBinMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
   HRESULT hr = NOERROR;
   ASSERT(pici);
   if (!pici || sizeof(CMINVOKECOMMANDINFO) != pici->cbSize)
      return E_FAIL;
   
   //lpVerb: 32-bit value that contains zero in the high-order word and 
   //a menu-identifier offset of the command to carry out in the low-order word.
   if (pici->lpVerb && !HIWORD(pici->lpVerb))
   {
      if (m_idAddCmd == LOWORD(pici->lpVerb))
      {
         BOOL fConfirmDel = TRUE;

         g_pRecBin->BeginRecycle();         
         g_pRecBin->DeleteAll(fConfirmDel);
         g_pRecBin->EndRecycle();
      }
   }
   
   return hr;
}

STDMETHODIMP CRecycleBinMenu::QueryContextMenu(HMENU hmenu,
                                               UINT indexMenu,
                                               UINT idCmdFirst,
                                               UINT idCmdLast,
                                               UINT uFlags)
{
   HRESULT hr = NOERROR;
   ASSERT(CMF_NORMAL == uFlags);

   LPTSTR pszMenuItem = LOAD_STRING(IDS_MENU_FILE_EMPTYRECBIN);
   if (!pszMenuItem)
      return E_FAIL;

   if (::InsertMenu(hmenu, indexMenu, MF_BYPOSITION | MF_STRING,
                    idCmdFirst, pszMenuItem))
   {   
      //Save added menu item identifier
      m_idAddCmd = idCmdFirst;

      g_pRecBin->BeginRecycle();
      if (g_pRecBin->IsEmpty())
         ::EnableMenuItem(hmenu, idCmdFirst, MF_BYCOMMAND | MF_GRAYED);

      g_pRecBin->EndRecycle();
      
      hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
   }
   else
   {
      hr = E_FAIL;
   }

   return hr;
}

