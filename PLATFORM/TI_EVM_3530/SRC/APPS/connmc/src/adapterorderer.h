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
#ifndef ADAPTER_ORDERER_H__
#define ADAPTER_ORDERER_H__

#include "connmc.h"

class AdapterOrderer
{
   protected:
      struct AOItem
      {
         LanConnInfo * pLanConnInfo;
         DWORD dwAdjustedRouteMetric;
         AOItem() : pLanConnInfo(NULL), dwAdjustedRouteMetric(0) {}
         ~AOItem() { if (pLanConnInfo) delete pLanConnInfo; }
      };

      HINSTANCE m_hInstance;
      HANDLE m_hThread;
      HANDLE m_hInitEvt;
      HANDLE m_hKillEvt;
      HWND m_hParent;
      HWND m_hDialog;
      WNDPROC m_wndprocButton;
      BOOL m_fVisible;

      static DWORD WINAPI AdapterThreadProc(LPVOID lpParameter);
      static BOOL CALLBACK AdapterOrdererProc(HWND hwnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);
      void init();
      void handleSelectionChange();
      void handleSelectionUp();
      void handleSelectionDown();
      BOOL addAdapter(LanConnInfo * pLanConnInfo);
      void writeChanges();
      BOOL ItemIsInListView(LanConnInfo * pLanConnInfo);
      BOOL IsSameAdapter(LanConnInfo *a, LanConnInfo *b);

      static int CALLBACK SortCallback(LPARAM lParam1, LPARAM lParam2,
                                       LPARAM lParamSort);

      static BOOL CALLBACK UpDownButtonSubcass(HWND hwnd, UINT msg,
                                               WPARAM wParam, LPARAM lParam);

   public:
      AdapterOrderer(HINSTANCE hInstance = NULL, HWND hParent = NULL);
      ~AdapterOrderer();

      BOOL show();
      BOOL hide();
      void refresh();
};

#endif // ADAPTER_ORDERER_H__
