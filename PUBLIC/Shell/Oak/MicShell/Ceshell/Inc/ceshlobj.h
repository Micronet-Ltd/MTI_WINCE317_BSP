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

#include <shlobj.h>


// ceshell private interfaces


// {1CB6BFA9-6809-4d35-9279-D4809265F2EC}
//DEFINE_GUID(IID_IEnumFilter, 0x1cb6bfa9, 0x6809, 0x4d35, 0x92, 0x79, 0xd4, 0x80, 0x92, 0x65, 0xf2, 0xec);

// {3A303AC0-9C68-434e-AE44-FAD991C80CD7}
//DEFINE_GUID(IID_IShellListView, 0x3a303ac0, 0x9c68, 0x434e, 0xae, 0x44, 0xfa, 0xd9, 0x91, 0xc8, 0xc, 0xd7);


// IEnumFilter interface
//
//
// [Member functions]
//
// IEnumFilter::SetFilterString(pszFilter)
//   This function sets a filter pattern to be used by an enumerator. i.e. IEnumIDList
//
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IEnumFilter

DECLARE_INTERFACE_(IEnumFilter, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IEnumFilter specific methods ***
    STDMETHOD(SetFilterString) (THIS_ LPCWSTR pszFilter) PURE;
};


// IShellListView interface
//
//
// [Member functions]
//
// IShellListView::EnablePersistentViewMode(fEnable)
//   This function enables or disables ViewMode persistency via registry.
//
// IShellListView::GetSelectedItems(paPidls, puCount)
//   This function returns an array and number of selected items in the current view.
//
// IShellListView::SetColumnWidths(pWidths, cItems)
//   This function sets the column widths used for details view.
//
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IShellListView

DECLARE_INTERFACE_(IShellListView, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IShellListView specific methods ***
    STDMETHOD(EnablePersistentViewMode) (THIS_ BOOL  fEnable) PURE;
    STDMETHOD(GetSelectedItems) (THIS_ LPCITEMIDLIST** paPidls, UINT* puCount) PURE;
    STDMETHOD(SetColumnWidths) (THIS_ int* pWidths, int cItems) PURE;
};

