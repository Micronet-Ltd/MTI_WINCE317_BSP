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

#include "ceshhpc.h"

class AutoScroll_t
{
protected:
   static const int s_ScrollInterval;
   static const int s_MinScrollVelocity;

   enum { s_CntSamples = 3 };
   enum Direction_t { s_Up, s_Down, s_Left, s_Right };

   int m_IdxNextSample;
   DWORD m_TimeLastScroll;
   POINTL m_PointSamples[s_CntSamples];
   DWORD m_TimeSamples[s_CntSamples];

   bool AddSample(const POINTL * ppt, DWORD dwTime);
   int CurrentVelocity();
   static bool CanScroll(HWND hwnd, Direction_t Direction);
   static void DragScrollDirection(HWND hwnd, const POINTL * ppt,
                                   bool * pUp, bool * pDown,
                                   bool * pLeft, bool * pRight);


public:
   void Init();
   bool AutoScroll(HWND hwnd, const POINTL * pptNow);
};

class CShellListView
{
protected:
   struct DragData
   {
      HIMAGELIST m_himlDragMask;
      HWND m_hwndSourceLV;
      POINT m_ptDragOrigin;
      int * m_pExcludeList;
   };

   HWND m_hwndLV;
   DWORD m_dwColumnCount;
   int m_iTarget;
   LPITEMIDLIST m_pidlMakeEditableOnAdd;
   AutoScroll_t m_AutoScroll;
   static DragData * m_pDragData;

public:
   CShellListView();
   ~CShellListView();
   operator HWND() const { return m_hwndLV; } 

   BOOL AppendColumn(LPCTSTR psz, int width, int alignment);
   void Arrange() const;
   BOOL BeginDrag(NMLISTVIEW * pnmListView);
   BOOL Clear();
   BOOL Create(HWND hwndParent = NULL, UINT fFlags = 0);
   void Destroy();
   void DragEnter(POINTL pt);
   void DragLeave();
   BOOL DragOver(POINTL pt, BOOL * pfScrolled);
   void DropTo(POINTL pt);
   void EndDrag();
   int FindItemFromPidl(LPCITEMIDLIST pidl) const;
   LPARAM GetContextMenuPoint() const;
   LPCITEMIDLIST GetItemPidl(int iItem) const;
   LPCITEMIDLIST GetItemPidl(LPCITEMIDLIST pidlItem) const;
   LPCITEMIDLIST GetPidlAt(POINTL pt) const;
   LPCITEMIDLIST * GetSelected(UINT * puCount) const;
   BOOL GiveFeedback(DWORD dwEffect);
   void HandleEscape();
   void HiliteCurrentTarget();
   void InvalidateItemColumn(LPCITEMIDLIST pidl, int iColumn);
   BOOL IsAutoArrangeEnabled() const;
   void MakeEditableOnAdd(LPCITEMIDLIST pidl);
   void OnSetFocus();
   BOOL Populate(LPENUMIDLIST pEnumIDList);
   BOOL ReflectAdd(LPCITEMIDLIST  pidl);
   BOOL ReflectRemove(LPCITEMIDLIST pidl);
   void Resize(DWORD width, DWORD height);
   void RenameSelected();
   void SelectAll();
   void SetColors(COLORREF rgbBkColor, COLORREF rgbTextColor, COLORREF rgbTextBkColor);
   void SetRedraw(BOOL fRedraw);
   UINT SetViewMode(UINT ViewMode);
   void ShowSelectedAsCut();
   void SnapToGrid() const;
   void Sort(PFNLVCOMPARE pfnCompare, LPARAM lParamSort);
   void ToggleAutoArrange(BOOL fAutoArrange);
   BOOL UpdateItemPidl(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew);
};

