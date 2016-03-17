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
/*---------------------------------------------------------------------------*\
 *  module: drag.cpp
\*---------------------------------------------------------------------------*/
#define _OLE32_

#include "drag.h"

#define c_szDragCaptureWnd TEXT("DragCaptureWnd")

CDropTargetList *g_pDropTargetList;

#define TIMER_INT 100


// NOTE: There can only be one of these at a time, this will always refer
//       to the current IDropTarget we are playing with.
WNDPROC lpfnCurrent;
HWND    hwndCurrent;

typedef struct tagDROPTARGETMARSHALDATA {
   LPDATAOBJECT pDataObj;
   DWORD grfKeyState;
   POINTL pt;
   DWORD *pdwEffect;
   HRESULT hr;
} DROPTARGETMARSHALDATA, *LPDROPTARGETMARSHALDATA; 


#define WM_DROPTARGET_DRAGENTER       (WM_USER + 0x400)
#define WM_DROPTARGET_DRAGOVER        (WM_USER + 0x401)
#define WM_DROPTARGET_DRAGLEAVE       (WM_USER + 0x402)
#define WM_DROPTARGET_DROP            (WM_USER + 0x403)


LRESULT
CALLBACK
DropTarget_SubClassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
/*---------------------------------------------------------------------------*\
 * NOTE: Doing this to allow for thread context switching
\*---------------------------------------------------------------------------*/
{
   LPDROPTARGETINFO lpdti;
   LPDROPTARGET lpdt;
   LPDROPTARGETMARSHALDATA lpdtmd = (LPDROPTARGETMARSHALDATA)lp;
   
   //SHELL_CHECKSTACK(hwnd);

   lpdti = g_pDropTargetList->FindDropTargetByHwnd(hwnd);
   if (!lpdti)
   {
      RETAILMSG(TRUE, (TEXT("DropTarget_SubClassProc: Called without IDropTarget associated with hWnd.\r\n")));
      return 0;
   }
   
   lpdt = lpdti->pDropTarget; 
   if (!lpdt)
   {
      return 0;
   }

   switch (msg)
   {
      case WM_DROPTARGET_DRAGENTER:
         if (!lpdtmd)
         {
            RETAILMSG(TRUE, (TEXT("DropTarget_SubClassProc: Called without DROPTARGETMARSHALDATA.\r\n")));
            return 0;
         }
         RETAILMSG(0, (TEXT("DropTarget_SubClassProc: Calling DragEnter\r\n")));
         
         lpdtmd->hr = lpdt->DragEnter(lpdtmd->pDataObj,lpdtmd->grfKeyState,
                               lpdtmd->pt, lpdtmd->pdwEffect);
         break;

      case WM_DROPTARGET_DRAGOVER:
         if (!lpdtmd)
         {
            RETAILMSG(TRUE, (TEXT("DropTarget_SubClassProc: Called without DROPTARGETMARSHALDATA.\r\n")));
            return 0;
         }
         RETAILMSG(0, (TEXT("DropTarget_SubClassProc: Calling DragOver\r\n")));
         lpdtmd->hr = lpdt->DragOver(lpdtmd->grfKeyState, lpdtmd->pt,
                              lpdtmd->pdwEffect);
         break;

      case WM_DROPTARGET_DRAGLEAVE:
         if (!lpdtmd)
         {
            RETAILMSG(TRUE, (TEXT("DropTarget_SubClassProc: Called without DROPTARGETMARSHALDATA.\r\n")));
            return 0;
         }   
         RETAILMSG(0, (TEXT("DropTarget_SubClassProc: Calling DragLeave\r\n")));
         lpdtmd->hr = lpdt->DragLeave();
         break;

      case WM_DROPTARGET_DROP:
         if (!lpdtmd)
         {
            RETAILMSG(TRUE, (TEXT("DropTarget_SubClassProc: Called without DROPTARGETMARSHALDATA.\r\n")));
            return 0;
         }
         RETAILMSG(0, (TEXT("DropTarget_SubClassProc: Calling Drop\r\n")));
         lpdtmd->hr = lpdt->Drop(lpdtmd->pDataObj,lpdtmd->grfKeyState,
                           lpdtmd->pt, lpdtmd->pdwEffect);
         break;
         
      default:
         RETAILMSG(0, (TEXT("DropTarget_SubClassProc: Calling the WndProc")));
         if (lpfnCurrent)
         {
            return CallWindowProc(lpfnCurrent, hwnd, msg, wp, lp);
         }
   }

   return 0;
      
} /* DropTarget_SubClassProc()
   */


CDropTargetList::CDropTargetList()
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
} /* CDropTargetList()
   */


CDropTargetList::~CDropTargetList()
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   LPDROPTARGETINFO lpdti = NULL;
   LPDROPTARGETINFO lpdtinext = NULL;

   m_list.Lock(TRUE);
   lpdti = (LPDROPTARGETINFO)m_list.Head();
   while(lpdti)
   {
      lpdtinext = (LPDROPTARGETINFO)m_list.Next((LPLISTOBJ) lpdti);
      m_list.Disconnect((LPLISTOBJ) lpdti);
      __try
      {
         // It is possible that this will crash if the owner of the IDropTarget has already been unloaded
         lpdti->pDropTarget->Release();
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
         DEBUGMSG(ZONE_WARNING, (TEXT("RegisterDragDrop was called without an corresponding RevokeDragDrop.")));
      }
      LocalFree(lpdti);
      lpdti = lpdtinext;
   }
   m_list.Lock(FALSE);
} /* ~CDropTargetList()
   */


BOOL CDropTargetList::AddDropTarget(HWND hwndTarget, 
                           IDropTarget *pDropTarget)
/*---------------------------------------------------------------------------*\
 * NOTE: Assumes that pDropTarget is not NULL!
\*---------------------------------------------------------------------------*/
{
   LPDROPTARGETINFO lpdti;

   lpdti = FindDropTargetByHwnd(hwndTarget);
   if(lpdti)
   {
      // hwndTarget was already registered.
      return FALSE;
   }
   
   lpdti = (LPDROPTARGETINFO)LocalAlloc(LMEM_FIXED, sizeof(DROPTARGETINFO));
   if(lpdti == NULL)
   {
      return FALSE;
   }
   lpdti->hwnd = hwndTarget;
   lpdti->pDropTarget = pDropTarget;
   pDropTarget->AddRef();

   m_list.AddTail(&lpdti->lpObj);
   return TRUE;

} /* AddDropTarget() 
   */


BOOL CDropTargetList::RemoveDropTarget(HWND hwndTarget)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   LPDROPTARGETINFO lpdti;

   lpdti = FindDropTargetByHwnd(hwndTarget);
   if(lpdti)
   {
      m_list.Disconnect((LPLISTOBJ) lpdti);
      lpdti->pDropTarget->Release();
      LocalFree(lpdti);
      return TRUE;
   }
   
   return FALSE;

} /* RemoveDropTarget() 
   */


LPDROPTARGETINFO 
CDropTargetList::FindDropTargetByHwnd(HWND hwndTarget)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   LPDROPTARGETINFO lpdti;

   lpdti = (LPDROPTARGETINFO)m_list.Head();
   while (lpdti != NULL)
   {
      if (hwndTarget == lpdti->hwnd)
      {
         return lpdti;
      }

      lpdti = (LPDROPTARGETINFO)m_list.Next(&lpdti->lpObj);
   }
   return NULL;
   
} /* FindDropTargetByHwnd()
   */

#define VK_ALT VK_MENU

static const struct {
        int     keyCode;
        WPARAM  keyFlag;
    } vKeyMap [] = {
        { VK_LBUTTON, MK_LBUTTON },
        { VK_RBUTTON, MK_RBUTTON },
        { VK_MBUTTON, MK_MBUTTON },
        { VK_ALT    , MK_ALT     },
        { VK_SHIFT  , MK_SHIFT   },
        { VK_CONTROL, MK_CONTROL }
   };

//+-------------------------------------------------------------------------
//
//  Function:    GetControlKeysState
//
//  Synopsis:   queries the current status of the control keys
//
//  Arguments:  [fAll]   -- if true, the just query the keys, not mouse
//            buttons too
//
//  Returns:    the MK flags for each key pressed
//
//  Algorithm:   Get key state either for all keys and mouse buttons in
//            the vKeyMap table or simply for the key portion of the table
//            and translate it to the WPARAM form as returned in mouse
//            messages.
//
//  History:    dd-mmm-yy Author    Comment
//            06-Dec-93 alexgo    32bit port
//
//--------------------------------------------------------------------------

WORD GetControlKeysState(BOOL fAll)
{
    WORD grfKeyState = 0;

    int i = (fAll) ? 0 : 3;
   
    for (; i < sizeof(vKeyMap) / sizeof(vKeyMap[0]); i++)
    {
      if (GetAsyncKeyState(vKeyMap[i].keyCode) < 0) // Key down
      {
         grfKeyState |= vKeyMap[i].keyFlag;
      }
    }

    return grfKeyState;
}

//+-------------------------------------------------------------------------
//
//  Function:   GetControlKeysStateOfParam
//
//  Synopsis:   gets the key/button state of wparam (used with mouse messages)
//
//  Arguments:  [wParam]   -- the wParam to parse apart
//
//  Returns:    the key's set in wParam
//
//  Algorithm:   First determine if keys we are interested in are set
//            in the wParam message. Then go check the state of the
//            ALT key and record that in the key state. We then return
//            that to the caller.
//
//  History:    dd-mmm-yy Author    Comment
//            06-Dec-93 alexgo    32bit port
//
//--------------------------------------------------------------------------

WORD GetControlKeysStateOfParam(WPARAM wParam)
{
    // Check all the buttons we are interested in at once.
    WORD grfKeyState = (WORD) (wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_SHIFT | MK_CONTROL));

    // get the alt key
    if (GetAsyncKeyState(VK_ALT) < 0) // Key down
    {
      grfKeyState |= MK_ALT;
    }

    return grfKeyState;
}


LRESULT CALLBACK DragDrop_WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
/*---------------------------------------------------------------------------*\
 *
\*---------------------------------------------------------------------------*/
{
   switch (msg)
   {
      case WM_CANCELMODE:
         break;
         
      default:
         return DefWindowProc(hwnd, msg, wp, lp);
   }
   
   return 0;

} /* DragDrop_WndProc()
   */

//+-------------------------------------------------------------------------
//
//  Function:   CDragOperation::CDragOperation
//
//  Synopsis:   Initialize the object to start the operation
//
//  Arguments:   [pDataObject] - pointer to data object to drop
//            [pDropSource] - pointer to source for drop operation
//            [dwOKEffects] - effects allowed in drag operation
//            [pdwEffect] - how operation affected source data
//            [hr] - whether constructor succeeded
//
//  Algorithm:   Initialize data in object. Make sure that static data
//               is initialized. Wait for first mouse message to begin.
//
//  History:   dd-mmm-yy Author    Comment
//            20-Oct-94 alexgo    added support for Win31 drag drop
//            04-Apr-94 Ricksa    Created
//
//--------------------------------------------------------------------------
CDragOperation::CDragOperation(
    LPDATAOBJECT pDataObject,
    LPDROPSOURCE pDropSource,
    DWORD dwOKEffects,
    DWORD FAR *pdwEffect,
    HRESULT& hr)
   :
       m_pDataObject(pDataObject),
       m_pDropSource(pDropSource),
       m_dwOKEffects(dwOKEffects),
       m_pdwEffect(pdwEffect),
       m_hwndLast((HWND) NULL),
       m_pDropTarget(NULL),
       m_fEscapePressed(FALSE),
       m_hrDragResult(S_OK),
       m_fReleasedCapture(FALSE),
       m_pdtNoTarget(NULL),
       m_hwndDropTarget((HWND) NULL)
{
   // NOTE: We need to create a window to the focus to, this wndproc
   //       need only respond to WM_CANCELMODE, that way we know what is up
   //       with the capture.
         
   m_hwndHasDragCapture = CreateWindow(c_szDragCaptureWnd, NULL, WS_POPUP, 0, 0,
                                       1, 1, NULL, NULL, HINST_CESHELL, NULL);

   ASSERT(m_hwndHasDragCapture);
   SetCapture(m_hwndHasDragCapture);

   m_pdtNoTarget = new CNullDropTarget;
   ASSERT(m_pdtNoTarget != NULL);

   MSG msg;

    // Busy wait until a mouse or escape message is in the queue
    while (!PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
      ;

    // get mouse pos and key state
   m_ptlLastPoint.x = LOWORD(msg.lParam);
   m_ptlLastPoint.y = HIWORD(msg.lParam);
   m_grfKeyState = (msg.message == WM_LBUTTONUP) ? 0 : MK_LBUTTON;
//   m_grfKeyState = GetControlKeysStateOfParam(msg.wParam);
}

//+-------------------------------------------------------------------------
//
//  Function:   ~CDragOperation
//
//  Synopsis:   Clean up object
//
//  Algorithm:   Release mouse capture. 
//      formats.
//
//  History:   dd-mmm-yy Author    Comment
//            04-Apr-94 Ricksa    Created
//
//--------------------------------------------------------------------------
CDragOperation::~CDragOperation(void)
{
    // Stop the mouse capture
    ReleaseCapture();
   DestroyWindow(m_hwndHasDragCapture);
   m_pdtNoTarget->Release();
}

BOOL 
CDragOperation::HandleMessages(
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
    MSG msg;
    BOOL fResult = TRUE;
    BOOL fMsg;
    UINT uTimer = SetTimer(NULL, 0, TIMER_INT, NULL);

    do 
   {
      fMsg = FALSE;

      // Note: the order of peek is important - further messages can show up
      // in the last peek
        // If we looked for mouse messages first, we might never pick up
        // WM_QUIT or keyboard messages (because by the time we finished
        // processing the mouse message another might be on the queue).
        // So, we check for WM_QUIT and keyboard messages first.

      if (PeekMessage(&msg, 0, WM_QUIT, WM_QUIT, PM_REMOVE | PM_NOYIELD) ||
          PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE | PM_NOYIELD) ||
          PeekMessage(&msg, 0, WM_SYSKEYDOWN, WM_SYSKEYUP, PM_REMOVE | PM_NOYIELD) ||
          PeekMessage(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
          PeekMessage(&msg, 0, 0, 0, PM_REMOVE | PM_NOYIELD)) 
      {
         fMsg = TRUE;

         if (msg.message == WM_QUIT) 
         {
            PostQuitMessage(msg.wParam);
            m_hrDragResult = ResultFromScode(E_FAIL);
            fResult = FALSE;
         }
         else
         {
            if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
               (msg.message >= WM_SYSKEYDOWN && msg.message <= WM_SYSKEYUP))
            {
               do
               {
                  if ((msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN)
                     && msg.wParam == VK_ESCAPE)
                  {
                     m_fEscapePressed = TRUE;
                  }
               } while (PeekMessage(&msg, 0, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE | PM_NOYIELD) ||
                      PeekMessage(&msg, 0, WM_SYSKEYDOWN, WM_SYSKEYUP, PM_REMOVE | PM_NOYIELD));

               DWORD grfKeyState; 

               grfKeyState = GetControlKeysState(FALSE) | 
                           (m_grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

               if ((grfKeyState == m_grfKeyState) && !m_fEscapePressed)
               {
                  fMsg = FALSE;
               }
               else
               {
                  m_grfKeyState = grfKeyState;
               }
            }
            else
            {
               if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) 
               {
                  if( GetAsyncKeyState(VK_ESCAPE) < 0 )
                  {
                     m_fEscapePressed = TRUE;
                  }

                  if (WM_MOUSEMOVE == msg.message)
                  {
                     MSG msg2;
                     while(PeekMessage(&msg2, 0, WM_MOUSEMOVE, WM_MOUSEMOVE,   PM_REMOVE))
                     {
                        msg = msg2;
                     }
                  }

                  m_ptlLastPoint.x = LOWORD(msg.lParam);
                  m_ptlLastPoint.y = HIWORD(msg.lParam);

                  RETAILMSG(0, (TEXT("Raw points: x = %d  y = %d\r\n"), m_ptlLastPoint.x, m_ptlLastPoint.y));
#if 0
                  if(msg.message != WM_MOUSEMOVE)
                  {
                     m_grfKeyState = GetControlKeysStateOfParam(msg.wParam);
                  }
#else
                  if(msg.message == WM_LBUTTONUP)
                  {
                     m_grfKeyState &= ~MK_LBUTTON;
                  }
#endif
               }
               else
               {
                  if ((msg.message == WM_TIMER) && (msg.wParam == uTimer))
                  {
                     m_grfKeyState = GetControlKeysState(FALSE) | (m_grfKeyState &
                           (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

                     if( GetAsyncKeyState(VK_ESCAPE) < 0 )
                     {
                        m_fEscapePressed = TRUE;
                     }
                  }
                  else
                  {
                     DispatchMessage(&msg);
                     fMsg = FALSE;
                  }
               }
            }
         }
/*      } else {
         WaitMessage(); */  // Note this loop will be VERY ineffecient now, 
                        // eating tons of CPU cycles while it's waiting
                        // for a message, but since during a drag-drop
                        // we're probably going to be getting a lot of
                        // messages anyway, this is probably ok.
      }
    } while (!fMsg);

    KillTimer(NULL, uTimer);

    return fResult;
} /* CDragOperation::HandleMessage()
   */


//+-------------------------------------------------------------------------
//
//  Function:   CDragOperation::HandleFeedBack
//
//  Synopsis:   Handle feedback and update of cursor
//
//  Arguments:  [hr] - hresult from previous operation on drop target.
//
//  Returns:   TRUE - continue drag operation
//      FALSE - error
//
//  Algorithm:  If previous operation on the target failed, map this to a
//              disallowed drop. Then ask the source for feedback. If it
//              so requests, then update the cursor. If an unexpected
//              error occurs, let caller know that loop should break.
//
//  History:   dd-mmm-yy Author    Comment
//            19-Apr-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CDragOperation::HandleFeedBack(HRESULT hr)
{
    BOOL fResult = TRUE;

    if (hr != NOERROR)
    {
      // target not responding for some reason; treat
      // as if drop not possible, but don't preserve
      // the reason why.
      *m_pdwEffect = DROPEFFECT_NONE;
    }

    // If bogus return from drag over, then make sure results are appropriate.
   *m_pdwEffect &= (m_dwOKEffects | DROPEFFECT_SCROLL);

   hr = m_pDropSource->GiveFeedback(*m_pdwEffect);

    if(hr != NOERROR)
    {
        // Either we want to change the cursor or some unexpected
      // error has occurred.
      if (DRAGDROP_S_USEDEFAULTCURSORS == GetScode(hr))
      {
//         SetDragCursor(*m_pdwEffect);
//         _pcddcDefault->SetCursor(*m_pdwEffect);
      }
      else
      {
         fResult = FALSE;
         m_hrDragResult = hr;
      }
    }

    return fResult;
}


//+-------------------------------------------------------------------------
//
//  Function:   CDragOperation::UpdateTarget
//
//  Synopsis:   Update the target window based on mouse location
//
//  Returns:   TRUE - continue drag operation
//            FALSE - error or time to drop
//
//  Algorithm:   First, we query the source to see if it wants to continue
//      with the drop. If so, we get current window for mouse. If
//      it is different than the previous window check to see whether
//      the targets are different. If they are different, then notify
//      the current target that we are leaving and then notify the
//      new target that we have arrived.
//
//  History:   dd-mmm-yy Author    Comment
//            04-Apr-94 Ricksa    Created
//            10-Jul-94 AlexT     Allow same IDropTarget on different HWNDs
//
//--------------------------------------------------------------------------
BOOL CDragOperation::UpdateTarget(void)
{
   BOOL fResult = TRUE;
   HRESULT hr;
   LPDROPTARGETINFO lpdti;
   LPDROPTARGET lpCurDropTarget = m_pdtNoTarget;
   HWND hWndTemp = NULL;
   DROPTARGETMARSHALDATA dtmd;
         
   HWND hwndCur = WindowFromPoint(POINTfromPOINTL(m_ptlLastPoint));

   // Query continue can return telling us one of four things:
   // (1) Keep going (S_OK), (2) Drop operation should occur
   // (DRAGDROP_S_DROP), (3) Drop operation is canceled
   // (DRAGDROP_S_CANCEL) or (4) An unexpected error has occurred.

   RETAILMSG(0, (TEXT("CDragOperation::UpdateTarget: Before QueryContinueDrag; m_grfKeyState = %d\r\n"),m_grfKeyState));
   HRESULT hrQuery = m_pDropSource->QueryContinueDrag(m_fEscapePressed, m_grfKeyState);
   RETAILMSG(0, (TEXT("CDragOperation::UpdateTarget: hrQuery = %08x\r\n"), hrQuery));

   if (hrQuery != S_OK)
   {
      m_hrDragResult = hrQuery;
      fResult = FALSE;
      goto UpdateTarget_exit;
   }

   if (hwndCur != m_hwndLast)
   {
/*      DWORD dwCurrentProcessId = 0;
      DWORD dwTempProcessID; */

      RETAILMSG(0, (TEXT("CDragOperation::UpdateTarget: Different hwnd...\r\n")));
      hWndTemp = hwndCur;

      while (hWndTemp && lpCurDropTarget == m_pdtNoTarget)
      {
         lpdti = g_pDropTargetList->FindDropTargetByHwnd(hWndTemp);
         if (lpdti)
         {
            lpCurDropTarget = lpdti->pDropTarget;
         }
         m_hwndDropTarget = hWndTemp;

         hWndTemp = GetParent(hWndTemp);
      } 

      PREFAST_ASSERT(lpCurDropTarget);

      m_hwndLast = hwndCur;

      if ( (lpCurDropTarget != m_pDropTarget) || (m_pDropTarget == NULL))
      {
         if (m_pDropTarget != NULL)
         {
            if (m_pdtNoTarget == m_pDropTarget)
            {
               m_pDropTarget->DragLeave();
            }
            else
            {
               // Tell the drop target we are leaving & release it
               SendMessage(hwndCurrent, WM_DROPTARGET_DRAGLEAVE, 0, (LPARAM)&dtmd);
               SetWindowLong(hwndCurrent, GWL_WNDPROC, (LONG)lpfnCurrent);

               lpfnCurrent = NULL;
               hwndCurrent = NULL;
            }

            m_pDropTarget->Release();
            m_pDropTarget = NULL;
         }

         // Set up effects for query of target
         *m_pdwEffect = m_dwOKEffects;

         m_pDropTarget = lpCurDropTarget;
         m_pDropTarget->AddRef();

         // BUGBUG: Our implementation of CNullDropTarget doesn't
         //           know which window, and the point it needs is actually
         //           screen coordinates so we will convert here !

         if (m_pdtNoTarget == m_pDropTarget)
         {
            hr = m_pDropTarget->DragEnter(m_pDataObject, m_grfKeyState,
                                          m_ptlLastPoint, m_pdwEffect);
         }
         else
         {
            if (lpfnCurrent)
            {
               RETAILMSG(TRUE, (TEXT("DROPTARGET wasn't cleared, huge mistake!\r\n")));
            }
            
            hwndCurrent = m_hwndDropTarget;
            lpfnCurrent = (WNDPROC)SetWindowLong(hwndCurrent, GWL_WNDPROC,
               (LONG)DropTarget_SubClassProc);
            
            dtmd.pDataObj    = m_pDataObject;
            dtmd.grfKeyState = m_grfKeyState;
            dtmd.pt          = m_ptlLastPoint;
            dtmd.pdwEffect   = m_pdwEffect;

            SendMessage(hwndCurrent, WM_DROPTARGET_DRAGENTER, 0, (LPARAM)&dtmd);
            hr = dtmd.hr;
         }

         // Errors from this call are ignored. We interpret them
         // as the drop being disallowed. Since we don't really
         // use this information here but in the DragOver call
         // we make shortly, we just use this call to notify
         // the application that we are beginning a drag operation.

         if (!HandleFeedBack(hr))
         {
            goto UpdateTarget_exit;
         }
      }
   }


   if (hrQuery != NOERROR)
   {
      // Query asked for a drop
      fResult = FALSE;
      m_hrDragResult = hrQuery;
   }

UpdateTarget_exit:

   return fResult;
}


//+-------------------------------------------------------------------------
//
//  Function:   CDragOperation::DragOver
//
//  Synopsis:   Tell the target we are dragging over and process the result
//
//  Returns:   TRUE - continue drag operation
//      FALSE - error or time to drop
//
//  Algorithm:   Call the target's drag over if there is one and then
//      get the sources feedback to update the cursor accordingly.
//
//  History:   dd-mmm-yy Author    Comment
//            04-Apr-94 Ricksa    Created
//
//--------------------------------------------------------------------------
BOOL CDragOperation::DragOver(void)
{
   BOOL fResult = TRUE;
   DROPTARGETMARSHALDATA dtmd;

   if (m_pDropTarget != NULL)
   {
      *m_pdwEffect = m_dwOKEffects;

      if (m_pdtNoTarget == m_pDropTarget)
      {
         dtmd.hr = m_pDropTarget->DragOver(m_grfKeyState, m_ptlLastPoint, m_pdwEffect);
         fResult = HandleFeedBack(dtmd.hr);
      }
      else
      {
         dtmd.grfKeyState = m_grfKeyState;
         dtmd.pt          = m_ptlLastPoint;
         dtmd.pdwEffect   = m_pdwEffect;
         
         SendMessage(hwndCurrent, WM_DROPTARGET_DRAGOVER, 0, (LPARAM)&dtmd);         
         fResult = HandleFeedBack(dtmd.hr);
      }
   }
   else
   {
      ImageList_DragMove(m_ptlLastPoint.x, m_ptlLastPoint.y);
   }

   return fResult;
}


//+-------------------------------------------------------------------------
//
//  Function:   CDragOperation::CompleteDrop
//
//  Synopsis:   Complete the drag/drop operation
//
//  Returns:   Result of operation
//
//  Algorithm:   If there is a target and we have decided to drop, then
//      drop. Otherwise, release the target and return whatever
//      the other result of the operation was.
//
//  History:   dd-mmm-yy Author    Comment
//            04-Apr-94 Ricksa    Created
//
//--------------------------------------------------------------------------
HRESULT CDragOperation::CompleteDrop(void)
{
   DROPTARGETMARSHALDATA dtmd;
   
   // Stop the mouse capture in case a dialog box is thrown up.
   ReleaseCapture();

   if (m_pDropTarget != NULL)
   {
      // Caller is Drag/Drop aware
      // and indicated it might accept drop

      // The drop source replies DRAG_S_DROP if the user has
      // released the left mouse button.  However, we may be over
      // a drop target which has refused a drop (via the feedback
      // DROPEFFECT_NONE).  Thus, both the drop source and drop
      // target need to agree before we commit the drop.

      if (m_pdtNoTarget == m_pDropTarget)
      {
         if ((DRAGDROP_S_DROP == GetScode(m_hrDragResult)) && (*m_pdwEffect != DROPEFFECT_NONE))
         {
            // We are going to try to drop
            *m_pdwEffect = m_dwOKEffects;
            
            dtmd.hr = m_pDropTarget->Drop(m_pDataObject, m_grfKeyState,
                                          m_ptlLastPoint, m_pdwEffect);
            if (dtmd.hr != S_OK)
            {
               // If drop actually failed in the last stage, let the
               // caller know that this happened.
               m_hrDragResult = dtmd.hr;
            }

         }
         else
         {
            m_pDropTarget->DragLeave();
         }
      }
      else
      {
         if ((DRAGDROP_S_DROP == GetScode(m_hrDragResult)) && (*m_pdwEffect != DROPEFFECT_NONE))
         {
            // We are going to try to drop
            *m_pdwEffect = m_dwOKEffects;
            
            dtmd.pDataObj    = m_pDataObject;
            dtmd.grfKeyState = m_grfKeyState;
            dtmd.pt          = m_ptlLastPoint;
            dtmd.pdwEffect   = m_pdwEffect;
            
            SendMessage(hwndCurrent, WM_DROPTARGET_DROP, 0, (LPARAM)&dtmd);
            
            if (dtmd.hr != S_OK)
            {
               // If drop actually failed in the last stage, let the
               // caller know that this happened.
               m_hrDragResult = dtmd.hr;
            }
         }
         else
         {
            *m_pdwEffect = DROPEFFECT_NONE;
            SendMessage(hwndCurrent, WM_DROPTARGET_DRAGLEAVE, 0, (LPARAM)&dtmd);
         }
         
         SetWindowLong(hwndCurrent, GWL_WNDPROC, (LONG)lpfnCurrent);

         lpfnCurrent = NULL;
         hwndCurrent = NULL;
      }
      
      m_pDropTarget->Release();
      m_pDropTarget = NULL;

   }
   else
   {
      *m_pdwEffect = DROPEFFECT_NONE;
   }

   return m_hrDragResult;
}

STDMETHODIMP CNullDropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppobj)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   *ppobj = NULL;
     
   if ((riid == IID_IUnknown) || (riid == IID_IDropTarget))
   {
      ++m_dwRefs;
      *ppobj = (LPVOID)this;

      return NOERROR;
   }

   return E_NOINTERFACE;

} /* CNullDropTarget::QueryInterface()
   */


STDMETHODIMP_(ULONG) CNullDropTarget::AddRef(VOID)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   return ++m_dwRefs;
} /* CNullDropTarget::AddRef()
   */


STDMETHODIMP_(ULONG) CNullDropTarget::Release(VOID)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   if (--m_dwRefs == 0)
   {
      delete this;
      return 0;
  }

  return m_dwRefs;

} /* CNullDropTarget::Release()
   */


HRESULT 
CNullDropTarget::DragEnter(
   IDataObject *pDataObj, 
   DWORD grfKeyState, 
   POINTL pt,
   DWORD *pdwEffect
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   RETAILMSG(0, (TEXT("CNullDropTarget::DragEnter: pt.x = %d pt.y = %d\r\n"), pt.x, pt.y));
   ImageList_DragEnter(NULL, pt.x, pt.y);
   *pdwEffect = DROPEFFECT_NONE;

   return S_OK;
} /* CNullDropTarget::DragEnter()
   */


HRESULT 
CNullDropTarget::DragOver(
   DWORD grfKeyState, 
   POINTL pt, 
   DWORD *pdwEffect
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   RETAILMSG(0, (TEXT("CNullDropTarget::DragOver: pt.x = %d pt.y = %d\r\n"), pt.x, pt.y));
   ImageList_DragMove(pt.x, pt.y);
   *pdwEffect = DROPEFFECT_NONE;

   return S_OK;
} /* CNullDropTarget::DragOver()
   */


HRESULT 
CNullDropTarget::DragLeave()
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   RETAILMSG(0, (TEXT("CNullDropTarget::DragLeave\r\n")));
   ImageList_DragLeave(NULL);
   return S_OK;
} /* CNullDropTarget::DragLeave()
   */


HRESULT 
CNullDropTarget::Drop(
   IDataObject *pDataObj, 
   DWORD grfKeyState, 
   POINTL pt,
   DWORD *pdwEffect
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   return E_UNEXPECTED;
} /* CNullDropTarget::Drop()
   */


/*---------------------------------------------------------------------------*\
    @doc BOTH INTERNAL PEGSHELL
    
    @func LRESULT | DoDragDrop | 
         
    @comm Currently in proc for the shell only.
\*---------------------------------------------------------------------------*/
LRESULT
WINAPI 
DoDragDrop(
    LPDATAOBJECT pDataObject,   // @parm the object to drag
    LPDROPSOURCE pDropSource,   // @parm the drop source
    DWORD dwOKEffects,         // @parm effects flags (stuff to draw)
    DWORD FAR *pdwEffect      // @parm what actually happened in the drag 
                        //       drop attempt
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   HRESULT hr = E_FAIL;
   WNDCLASS wc = {0};

   if (!GetClassInfo(HINST_CESHELL, c_szDragCaptureWnd, &wc))
   {
      wc.lpfnWndProc = (WNDPROC)DragDrop_WndProc;
      wc.hInstance = HINST_CESHELL;
      wc.hCursor = LoadCursor(NULL, IDC_ARROW);
      wc.lpszClassName = c_szDragCaptureWnd;

      if (!RegisterClass(&wc))
         return HRESULT_FROM_WIN32_I(GetLastError());
   }

   //BUGBUG: 14845 Wince AEG, this only prevents a second drag/drop
   //        operation from starting, it does not fix the main problem
   //        which is to not be using global variables!
   if (lpfnCurrent)
   {
      return hr;
   }
   
   CDragOperation drgop(pDataObject, pDropSource, dwOKEffects, pdwEffect, hr);

   for (;;)
   {
      if (!drgop.UpdateTarget() || 
         !drgop.DragOver() || 
         !drgop.HandleMessages())
      {
         break;
      }
   } 

   hr = drgop.CompleteDrop();

    return hr;

} /* DoDragDrop()
   */


/*---------------------------------------------------------------------------*\
    @doc BOTH INTERNAL PEGSHELL
    
    @func LRESULT | RegisterDragDrop | 
         
    @comm Currently in proc for the shell only.
\*---------------------------------------------------------------------------*/
LRESULT
WINAPI 
RegisterDragDrop(
    HWND hwnd,               // @parm HWND to add to drop target list
    IDropTarget *pDropTarget   // @parm the drop target
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   if(!IsWindow(hwnd))
   {
      return DRAGDROP_E_INVALIDHWND;
   }

   if(!pDropTarget)
   {
      return E_INVALIDARG;
   }

   if (g_pDropTargetList == NULL)
   {
      if ((g_pDropTargetList = new CDropTargetList()) == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }

   if(g_pDropTargetList->AddDropTarget(hwnd, pDropTarget))
   {
      return S_OK;
   }
   else
   {
      return DRAGDROP_E_ALREADYREGISTERED;
   }

} /* RegisterDragDrop()
   */


/*---------------------------------------------------------------------------*\
    @doc BOTH INTERNAL PEGSHELL
    
    @func LRESULT | RevokeDragDrop | 
         
    @comm Currently in proc for the shell only.
\*---------------------------------------------------------------------------*/
LRESULT
WINAPI 
RevokeDragDrop(
    HWND hwnd               // @parm HWND to remove from drop target list
)
/*---------------------------------------------------------------------------*\
 * 
\*---------------------------------------------------------------------------*/
{
   if (g_pDropTargetList == NULL)
   {
      if ((g_pDropTargetList = new CDropTargetList()) == NULL)
      {
         return E_OUTOFMEMORY;
      }
   }

   if(g_pDropTargetList->RemoveDropTarget(hwnd))
   {
      return S_OK;
   }
   else
   {
      return DRAGDROP_E_NOTREGISTERED;
   }

} /* RevokeDragDrop()
   */
