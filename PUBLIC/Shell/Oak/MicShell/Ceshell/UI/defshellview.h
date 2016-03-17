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
#include "shelllistview.h"

#define TOOLTIPS 6

#define STATUS_PART_TEXT 0
#define STATUS_PART_ICON 4

typedef struct tagINSMENUDATA {
    UINT uPosition;
    UINT uFlags;
    UINT uIDMNewItem;
    UINT idszMenuItem;
}INSMENUDATA, FAR *LPINSMENUDATA;



class CDefShellView :
   public IShellView,
   public IOleCommandTarget,
   public IOleInPlaceActiveObject,
   public IShellChangeNotify,
   public IDropSource,
   public IDropTarget,
   public IEnumFilter,
   public IShellListView
{
protected:
   struct PopupMenuInfo
   {
      DWORD offset;
      LPCITEMIDLIST m_pidlSelected;
      UINT m_uPidlCount;
      LPCITEMIDLIST * m_aPidls;
      IContextMenu * m_pContextMenu;
   };

   struct DragData
   {
      DWORD m_dwDropEffect;
      IDropTarget * m_pCurrentDropTarget;
      LPDATAOBJECT m_pDataObject;
      HWND m_hwndSource;
      BOOL m_fContainsNamespace;
      BOOL m_fOverItem;
   };

   struct ChangeNotifyEventData
   {
      LONG lEvent;
      LPCITEMIDLIST pidl1;
      LPCITEMIDLIST pidl2;

      ChangeNotifyEventData();
      ~ChangeNotifyEventData();
   };

   DWORD m_ObjRefCount;
   LPCITEMIDLIST m_pidlFQ;
   HWND m_hwnd;
   HICON m_hIcon;
   HMENU m_hMenu;
   HACCEL m_hAccels;
   LPWSTR m_pszFilter;
   short m_cColumns;
   short* m_pColumnWidths;
   CShellListView * m_pShellListView;
   BYTE m_ArrangeBy;
   short m_Fmt;
   UINT m_uWatchID;
   UINT m_uFocus;
   UINT m_uState;
   UINT m_fFlags; // FOLDERSETTINGS
   UINT m_ViewMode; // FOLDERSETTINGS
   UINT m_UndoStr;
   DragData * m_pDragData;
   IShellFolder * m_pShellFolder;
   IShellBrowser * m_pShellBrowser;
   IShellBrowser * m_pTopShellBrowser;
   IOleInPlaceUIWindow* m_pOleInPlaceUIWindow;

   static UINT s_CurrentUndoStr;
   static short s_ColumnWidths[4];
   static DWORD s_TTRefCount;
   static LPWSTR s_pszTooltips[TOOLTIPS];
   static DWORD s_FSRefCount;
   static LPWSTR s_pszDRootName;
   static HICON s_hDRootIcon;

   static const WCHAR WC_DEFSHELLVIEW[];
   static const UINT IN_LABEL_APPLY;
   static const UINT IN_LABEL_EDIT;
   static const UINT IN_REFRESH;
   static const UINT BACK_ENABLED;
   static const UINT FWD_ENABLED;
   static const UINT SORT_DECENDING;
   static const UINT PERSIST_VIEWMODE;
   static const UINT SELECTION_UPDATE_NEEDED;

   static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
   static int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

   static DWORD TTAddRef();
   static DWORD TTRelease();
   static DWORD FSAddRef();
   static DWORD FSRelease();

   virtual LRESULT ApplyLabelEdit(NMLVDISPINFO * lpdi);
   virtual LRESULT BeginLabelEdit(NMLVDISPINFO * lpdi);
   virtual LRESULT Create(HWND hwnd, DWORD width, DWORD height);
   virtual void Destroy();
   virtual void DoContextMenu(POINTL pt);
   virtual void DoDragDrop(NMLISTVIEW * pnmListView);
   virtual void DoNewFolder();
   virtual void DoProperties();
   virtual void GetDisplayInfo(NMLVDISPINFO * lpdi);
   virtual HRESULT HandleChangeNotify(ChangeNotifyEventData * pChangeNotifyEventData);
   virtual LRESULT HandleCommand(DWORD dwCmd);
   virtual LRESULT HandleInitMenuPopup(HMENU hmenu, PopupMenuInfo * pPopupMenuInfo);
   virtual LRESULT HandleNotify(LPNMHDR lpnmh);
   virtual LRESULT HandleSettingChange(DWORD flag, LPCTSTR pszSectionName);
   virtual void HandleSysColorChange();
   virtual void LoadColumnHeaders();
   virtual HMENU LoadPopupMenuTemplate();
   virtual BOOL MergeSubMenu(HMENU hmenuSub, LPINSMENUDATA lpInsMenuData, int nItems, BOOL fActivateFocus);
   virtual BOOL OnDefviewEditCommand();
   virtual HRESULT PerformAction(OLECMDID cmd);
   virtual void Resize(DWORD width, DWORD height);
   virtual BOOL UpdateBrowserMenus(UINT uState);
   virtual BOOL UpdateBrowserToolbar(UINT uState);
   virtual void UpdateCurrentDropTarget(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect);
   virtual void UpdateStatusBar(LPCITEMIDLIST *aPidls , UINT uItemCount, BOOL fInitialize);
   virtual void UpdateUndoMenuItem(HMENU hmenu);
   void OnSelectionChanged();
   void UpdateSelectionMenu();
 
public:
   CDefShellView(IShellFolder * pFolder, LPCITEMIDLIST pidlFQ);
   ~CDefShellView();

   inline void SetWatchID(UINT uWatchID) { m_uWatchID = uWatchID; }

   // IUnknown methods
   STDMETHOD (QueryInterface)(REFIID, LPVOID*);
   STDMETHOD_ (DWORD, AddRef)();
   STDMETHOD_ (DWORD, Release)();
   
   // IOleWindow methods
   STDMETHOD (ContextSensitiveHelp) (BOOL);
   STDMETHOD (GetWindow) (HWND*);

   // IShellView methods
   STDMETHOD (AddPropertySheetPages) (DWORD, LPFNADDPROPSHEETPAGE, LPARAM);
   STDMETHOD (CreateViewWindow) (LPSHELLVIEW, LPCFOLDERSETTINGS, LPSHELLBROWSER, LPRECT, HWND*);
   STDMETHOD (DestroyViewWindow) (VOID);
   STDMETHOD (EnableModeless) (BOOL);
   STDMETHOD (GetCurrentInfo) (LPFOLDERSETTINGS);
   STDMETHOD (GetItemObject) (UINT, REFIID, LPVOID*);
   STDMETHOD (Refresh) (VOID);
   STDMETHOD (SaveViewState) (VOID);
   STDMETHOD (SelectItem) (LPCITEMIDLIST, UINT);
   STDMETHOD (TranslateAccelerator) (LPMSG);
   STDMETHOD (UIActivate) (UINT);

   // IOleCommandTarget
   STDMETHOD (QueryStatus)(const GUID*, ULONG, OLECMD[], OLECMDTEXT*);
   STDMETHOD (Exec)(const GUID*, DWORD, DWORD, VARIANTARG*, VARIANTARG*);

   // IOleInPlaceActiveObject
   // Our frame was activated, better do the activation thing.
   STDMETHOD (OnFrameWindowActivate)(BOOL fActivate);
   STDMETHOD (OnDocWindowActivate)(BOOL fActivate);
   STDMETHOD (ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);

   // IShellChangeNotify methods
   STDMETHOD (OnChange)(LONG, LPCITEMIDLIST, LPCITEMIDLIST);

   // IDropSource methods
   STDMETHOD (QueryContinueDrag)(BOOL, DWORD);
   STDMETHOD (GiveFeedback)(DWORD);

   // IDropTarget methods
   STDMETHOD (DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
   STDMETHOD (DragOver)(DWORD, POINTL, LPDWORD);
   STDMETHOD (DragLeave)(VOID);
   STDMETHOD (Drop)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);

   // IEnumFilter
   STDMETHOD (SetFilterString)(LPCWSTR);

   // IShellListView
   STDMETHOD (EnablePersistentViewMode)(BOOL);
   STDMETHOD (GetSelectedItems)(LPCITEMIDLIST**, UINT*);
   STDMETHOD (SetColumnWidths)(int*, int);
};
