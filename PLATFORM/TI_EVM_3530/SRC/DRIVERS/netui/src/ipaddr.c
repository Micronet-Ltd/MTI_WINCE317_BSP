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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

ipaddr.c

Abstract:  

Functions:


Notes: 


--*/

#include "windows.h"
#include "windowsx.h"
#include "resource.h"
#include "netui.h"		/* Global IPAddress definitions */
#include "netuip.h"

#define MAXSTRINGLEN    256     // Maximum output string length
#define MAXMESSAGE      128     // Maximum resource string message

#define ALLOC_SIZE      (2*MAXSTRINGLEN+MAXMESSAGE)


int _cdecl RuiUserMessage(HWND hWnd, HMODULE hMod, UINT fuStyle, UINT idTitle,
                          UINT idMsg, ...)
{
    LPTSTR   pszTitle, pszRes, pszMsg;
    int       iRet;

    // Get the default module if necessary
    if (hMod == NULL)
        hMod = v_hInst;

    // Allocate the string buffer
    if ((pszTitle = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR)*ALLOC_SIZE)) == NULL)
        return IDCANCEL;

    // Fetch the UI title and message
    iRet = LoadString(v_hInst, idTitle ? idTitle : IDS_ERR_TITLE, pszTitle, MAXMESSAGE) + 1;

    pszRes = pszTitle + iRet;
    
    iRet += LoadString(hMod, idMsg, pszRes, ALLOC_SIZE-iRet) + 1;

    // Get the real message
    pszMsg = pszTitle + iRet;
    
    StringCchVPrintf(pszMsg, (ALLOC_SIZE - iRet), pszRes, (va_list)(&idMsg + 1));

    iRet = MessageBox(hWnd, pszMsg, pszTitle, fuStyle);

    LocalFree(pszTitle);
    return iRet;
}

void WINAPI RegisterIPClass(HINSTANCE hInst)
{
    WNDCLASS   ClassStruct;

    /* define class attributes */
    ClassStruct.lpszClassName = (LPTSTR)IPADDRESS_CLASS;
    ClassStruct.hCursor       = NULL;
    ClassStruct.lpszMenuName  = (LPTSTR)NULL;
    ClassStruct.style         = /*CS_HREDRAW|CS_VREDRAW|*/CS_DBLCLKS;
    ClassStruct.lpfnWndProc   = IPAddressWndFn;
    ClassStruct.hInstance     = hInst;
    ClassStruct.hIcon         = NULL;
    ClassStruct.cbWndExtra    = IPADDRESS_EXTRA;
    ClassStruct.cbClsExtra    = 0;
    ClassStruct.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1 );

    /* register IPAddress window class */
    RegisterClass(&ClassStruct);
    return;
}

void WINAPI UnregisterIPClass(HINSTANCE hInst)
{
    UnregisterClass((LPTSTR)IPADDRESS_CLASS, hInst);
    return;
}

/*
    Check an address to see if its valid.
    call
        ip = The address to check.
    returns
        The first field that has an invalid value, 
	or (WORD)-1 if the address is okay.

    returns
        TRUE if the address is okay
	FALSE if it is not
*/
DWORD CheckAddress(DWORD ip)
{
    BYTE b;

    b = HIBYTE(HIWORD(ip));
    if (b < MIN_FIELD1 || b > MAX_FIELD1 || b == 127)    return 0;
    b = LOBYTE(LOWORD(ip));
    if (b > MAX_FIELD3)    return 3;
    return (DWORD)-1;
}


/*
    IPAddressWndFn() - Main window function for an IPAddress control.

    call
    	hWnd	handle to IPAddress window
	wMsg	message number
	wParam	word parameter
	lParam	long parameter
*/
LONG CALLBACK IPAddressWndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LONG lResult;
    CONTROL *pControl;
    int i;
	SIZE	Size;
    TCHAR szBuf[CHARS_PER_FIELD+1];    
    
    lResult = TRUE;
    
    switch( wMsg )
    {
#if 0
        case WM_GETDLGCODE :
            lResult = DLGC_WANTCHARS;
            break;
#endif

        case WM_CREATE : /* create pallette window */
        {
            HDC hdc;
            UINT uiFieldStart;
            DWORD dwJustification;

            pControl = LocalAlloc(LPTR, sizeof(CONTROL));
            if (pControl)
            {
#define LPCS	((CREATESTRUCT  *)lParam)
                
                pControl->fEnabled = TRUE;
                pControl->fPainted = FALSE;
                pControl->fInMessageBox = FALSE;
                pControl->hwndParent = LPCS->hwndParent;
                pControl->dwStyle = LPCS->style;

                dwJustification = ((pControl->dwStyle & IP_RIGHT) != 0) ?
                    ES_RIGHT : 
                    ((pControl->dwStyle & IP_CENTER) != 0) ?
                    ES_CENTER : ES_LEFT;

                hdc = GetDC(hWnd);
                GetTextExtentExPoint (hdc, SZFILLER, 1, 
                                      0, NULL, NULL, &Size);
                pControl->uiFillerWidth = Size.cx;
                ReleaseDC(hWnd, hdc);

                /* width of field - (margins) - (space for period * num periods)*/
                pControl->uiFieldWidth = (LPCS->cx 
                                          - (LEAD_ROOM * 2)
                                          - pControl->uiFillerWidth
                                          *(NUM_FIELDS-1)) 
                    / NUM_FIELDS;
                uiFieldStart = LEAD_ROOM;

                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    pControl->Children[i].byLow = MIN_FIELD_VALUE;
                    pControl->Children[i].byHigh = MAX_FIELD_VALUE;

                    pControl->Children[i].hWnd = CreateWindow(
		    			TEXT("Edit"),
                        NULL,
                        WS_CHILD | WS_VISIBLE | 
                        /*ES_MULTILINE |*/ ES_NUMBER | dwJustification,
                        uiFieldStart,
                        HEAD_ROOM,
                        pControl->uiFieldWidth,
                        LPCS->cy-(HEAD_ROOM*2),
                        hWnd,
                        (HMENU)i,
                        LPCS->hInstance,
                        NULL);

                    Edit_LimitText(pControl->Children[i].hWnd, CHARS_PER_FIELD);

                    pControl->Children[i].lpfnWndProc = 
                        (FARPROC) GetWindowLong(pControl->Children[i].hWnd,
                                                GWL_WNDPROC);

                    SetWindowLong(pControl->Children[i].hWnd, 
                                  GWL_WNDPROC, (LONG)IPAddressFieldProc);

                    uiFieldStart += pControl->uiFieldWidth 
		    		    + pControl->uiFillerWidth;
                }

                SAVE_CONTROL_HANDLE(hWnd, pControl);

#undef LPCS
            }
            else
                DestroyWindow(hWnd);
        }
        break;

        case WM_PAINT: /* paint control window */
        {
            PAINTSTRUCT Ps;
            RECT rect;
            UINT uiFieldStart;
            COLORREF TextColor;
            HBRUSH hBrush, hOldBrush;
            HPEN hPen, hOldPen;

            BeginPaint(hWnd, (LPPAINTSTRUCT)&Ps);
            GetClientRect(hWnd, &rect);
            pControl = GET_CONTROL_HANDLE(hWnd);

            hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
            hOldBrush = SelectObject(Ps.hdc, hBrush);

			// need to set border pen to white if body of box is black
			#define CLR_WHITE   0x00FFFFFFL
			#define CLR_BLACK   0x00000000L
			hPen = CreatePen(PS_SOLID, 1, ((CLR_BLACK==GetSysColor(COLOR_WINDOW)) ? CLR_WHITE : CLR_BLACK));
			hOldPen = SelectObject(Ps.hdc, hPen);

            Rectangle(Ps.hdc, 0, 0, rect.right, rect.bottom);
            
            SelectObject(Ps.hdc, hOldBrush);
            DeleteObject(hBrush);

            SelectObject(Ps.hdc, hOldPen);
            DeleteObject(hPen);

            if (pControl->fEnabled)
                TextColor = GetSysColor(COLOR_WINDOWTEXT);
            else
                TextColor = GetSysColor(COLOR_GRAYTEXT);

            if (TextColor)
                SetTextColor(Ps.hdc, TextColor);
            SetBkColor(Ps.hdc, GetSysColor(COLOR_WINDOW));

            uiFieldStart = pControl->uiFieldWidth + LEAD_ROOM;
            for (i = 0; i < NUM_FIELDS-1; ++i)
            {
                ExtTextOut(Ps.hdc, uiFieldStart, HEAD_ROOM, 0, NULL,
                           SZFILLER, 1, NULL);
                uiFieldStart +=pControl->uiFieldWidth + pControl->uiFillerWidth;
            }

            pControl->fPainted = TRUE;

            EndPaint(hWnd, &Ps);
        }
        break;

        case WM_SETFONT:
        {
            pControl = GET_CONTROL_HANDLE(hWnd);
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SendMessage (pControl->Children[i].hWnd,
                             WM_SETFONT, wParam, lParam);
            }
        }
        break;

        case WM_SETFOCUS : /* get focus - display caret */
            pControl = GET_CONTROL_HANDLE(hWnd);
            EnterField(&(pControl->Children[0]), 0, CHARS_PER_FIELD);
            break;

        case WM_LBUTTONDOWN : /* left button depressed - fall through */
            SetFocus(hWnd);
            break;

        case WM_ENABLE:
        {
            pControl = GET_CONTROL_HANDLE(hWnd);
            pControl->fEnabled = (BOOL)wParam;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                EnableWindow(pControl->Children[i].hWnd, (BOOL)wParam);
            }

            if (pControl->dwStyle & WS_TABSTOP)
            {
                DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

                if (wParam)
                {
                    dwStyle |= WS_TABSTOP;
                }
                else
                {
                    dwStyle &= ~WS_TABSTOP;
                };
                SetWindowLong(hWnd, GWL_STYLE, dwStyle);
            }

            if (pControl->fPainted)    InvalidateRect(hWnd, NULL, FALSE);
        }
        break;

        case WM_DESTROY :
            pControl = GET_CONTROL_HANDLE(hWnd);

/* Restore all the child window procedures before we delete our memory block.*/
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SetWindowLong(pControl->Children[i].hWnd, GWL_WNDPROC,
                              (LONG)pControl->Children[i].lpfnWndProc);
            }


            LocalFree(pControl);
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))                
            {
/* One of the fields lost the focus, see if it lost the focus to another field
   of if we've lost the focus altogether.  If its lost altogether, we must send
   an EN_KILLFOCUS notification on up the ladder. */
                case EN_KILLFOCUS:

                {
                    HWND hFocus;

                    pControl = GET_CONTROL_HANDLE(hWnd);

                    if (!pControl->fInMessageBox)
                    {
                        hFocus = GetFocus();
                        for (i = 0; i < NUM_FIELDS; ++i)
                            if (pControl->Children[i].hWnd == hFocus)    break;

                        if (i >= NUM_FIELDS)
                        {
                            SendMessage(pControl->hwndParent, WM_COMMAND,
                                        MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_KILLFOCUS),
                                        (LPARAM)hWnd);
                            pControl->fHaveFocus = FALSE;
                        }
                    }
                }
                break;

/* One of the fields is getting the focus.  If we don't currently have the
   focus, then send an EN_SETFOCUS notification on up the ladder.*/
                case EN_SETFOCUS:
                    pControl = GET_CONTROL_HANDLE(hWnd);
                    if (!pControl->fHaveFocus)
                    {
                        pControl->fHaveFocus = TRUE;
                        SendMessage(pControl->hwndParent, WM_COMMAND, 
                                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_SETFOCUS),
                                    (LPARAM)hWnd );
                    }
                    break;
            }
            break;

/* Get the value of the IP Address.  The address is placed in the DWORD pointed
   to by lParam and the number of non-blank fields is returned.*/
        case IP_GETADDRESS:
        {
            int iFieldValue;
            DWORD dwValue;

            pControl = GET_CONTROL_HANDLE(hWnd);

            lResult = 0;
            dwValue = 0;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                iFieldValue = GetFieldValue(&(pControl->Children[i]));
                if (iFieldValue == -1)
                    iFieldValue = 0;
                else
                    ++lResult;
                dwValue = (dwValue << 8) + iFieldValue;
            }
            *((DWORD  *)lParam) = dwValue;

        }
        break;

/* Clear all fields to blanks.*/
        case IP_CLEARADDRESS:
        {
            pControl = GET_CONTROL_HANDLE(hWnd);
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SetWindowText(pControl->Children[i].hWnd,TEXT(""));
            }
        }
        break;

/* Set the value of the IP Address.  The address is in the lParam with the
   first address byte being the high byte, the second being the second byte,
   and so on.  A lParam value of -1 removes the address. */
        case IP_SETADDRESS:
        {

            pControl = GET_CONTROL_HANDLE(hWnd);
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                wsprintf(szBuf, TEXT("%d"), HIBYTE(HIWORD(lParam)));
                SetWindowText(pControl->Children[i].hWnd, szBuf);
                lParam <<= 8;
            }
        }
        break;

        case IP_SETRANGE:
            if (wParam < NUM_FIELDS)
            {
                pControl = GET_CONTROL_HANDLE(hWnd);

                pControl->Children[wParam].byLow = LOBYTE(LOWORD(lParam));
                pControl->Children[wParam].byHigh = HIBYTE(LOWORD(lParam));

            }
            break;

/* Set the focus to this control.
   wParam = the field number to set focus to, or -1 to set the focus to the
   first non-blank field.*/
        case IP_SETFOCUS:
            pControl = GET_CONTROL_HANDLE(hWnd);

            if (wParam >= NUM_FIELDS)
            {
                for (wParam = 0; wParam < NUM_FIELDS; ++wParam)
                    if (GetFieldValue(&(pControl->Children[wParam])) == -1)   break;
                if (wParam >= NUM_FIELDS)    wParam = 0;
            }
            EnterField(&(pControl->Children[wParam]), 0, CHARS_PER_FIELD);

            break;

        default:
            lResult = DefWindowProc( hWnd, wMsg, wParam, lParam );
            break;
    }
    return( lResult );
}




/*
    IPAddressFieldProc() - Edit field window procedure

    This function sub-classes each edit field.
*/
LONG CALLBACK IPAddressFieldProc(HWND hWnd,
                                 UINT wMsg, 
                                 WPARAM wParam, 
                                 LPARAM lParam)
{
    CONTROL  *pControl;
    FIELD  *pField;
    HWND hControlWindow;
    DWORD dwChildID;
    DWORD dwResult;
    static fInComposition = FALSE;

    hControlWindow = GetParent(hWnd);
    if (!hControlWindow)
        return 0;

    pControl = GET_CONTROL_HANDLE(hControlWindow);
    dwChildID = GetWindowLong(hWnd, GWL_ID);
    pField = &(pControl->Children[dwChildID]);
    if (pField->hWnd != hWnd)    return 0;

    switch (wMsg)
    {

        case WM_GETDLGCODE :
            return DLGC_WANTCHARS | DLGC_WANTARROWS;
            break;

        case WM_CHAR:

/* Typing in the last digit in a field, skips to the next field.*/
#ifdef	FAREAST
			// Patch up the character on J devices
            if (wParam >= 0xff10 && wParam <= 0xff19) {
                wParam -= 0xfee0;
			}
#endif	//FAREAST
            if (wParam >= '0' && wParam <= '9')
            {

                dwResult = CallWindowProc((WNDPROC)pControl->Children[dwChildID].lpfnWndProc,
                                          hWnd, 
                                          wMsg, 
                                          wParam, 
                                          lParam);
                dwResult = Edit_GetSel(hWnd);

                if (dwResult == MAKELPARAM(CHARS_PER_FIELD, CHARS_PER_FIELD)
                    && ExitField(pControl, dwChildID)
                    && dwChildID < NUM_FIELDS-1)
                {
                    EnterField(&(pControl->Children[dwChildID+1]),
                               0, CHARS_PER_FIELD);
                }
                return dwResult;
            }

/* spaces and periods fills out the current field and then if possible,
   goes to the next field.*/
            else if (wParam == FILLER || wParam == SPACE)
            {
                dwResult = Edit_GetSel(hWnd);
                if (dwResult != 0L && HIWORD(dwResult) == LOWORD(dwResult)
                    && ExitField(pControl, dwChildID))
                {
                    if (dwChildID >= NUM_FIELDS-1)
                        MessageBeep((UINT)-1);
                    else
                    {
                        EnterField(&(pControl->Children[dwChildID+1]),
                                   0, CHARS_PER_FIELD);
                    }
                }
                return 0;
            }

/* Backspaces go to the previous field if at the beginning of the current field.
   Also, if the focus shifts to the previous field, the backspace must be
   processed by that field.*/
            else if (wParam == BACK_SPACE)
            {
                if (dwChildID > 0 && Edit_GetSel(hWnd) == 0L)
                {
                    if (SwitchFields(pControl, dwChildID, dwChildID-1,
                                     CHARS_PER_FIELD, CHARS_PER_FIELD)
                        && Edit_LineLength(pControl->Children[dwChildID-1].hWnd, 0)
                        != 0L)
                    {
                        SendMessage(pControl->Children[dwChildID-1].hWnd,
                                    wMsg, wParam, lParam);
                    }
                    return 0;
                }
            }

/* Any other printable characters are not allowed.*/
            else if (wParam > SPACE)
            {
                MessageBeep((UINT)-1);
                return 0;
            }
            break;

        case WM_KEYDOWN:
            switch (wParam)
            {

/* Arrow keys move between fields when the end of a field is reached.*/
                case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                    if (GetKeyState(VK_CONTROL) < 0)
                    {
                        if ((wParam == VK_LEFT || wParam == VK_UP) && dwChildID > 0)
                        {
                            SwitchFields(pControl, dwChildID, dwChildID-1,
                                         0, CHARS_PER_FIELD);
                            return 0;
                        }
                        else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                                 && dwChildID < NUM_FIELDS-1)
                        {
                            SwitchFields(pControl, dwChildID, dwChildID+1,
                                         0, CHARS_PER_FIELD);
                            return 0;
                        }
                    }
                    else
                    {
                        DWORD dwStart, dwEnd;

                        dwResult = Edit_GetSel(hWnd);
                        dwStart = LOWORD(dwResult);
                        dwEnd = HIWORD(dwResult);
                        if (dwStart == dwEnd)
                        {
                            if ((wParam == VK_LEFT || wParam == VK_UP)
                                && dwStart == 0
                                && dwChildID > 0)
                            {
                                SwitchFields(pControl, dwChildID, dwChildID-1,
                                             CHARS_PER_FIELD, CHARS_PER_FIELD);
                                return 0;
                            }
                            else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                                     && dwChildID < NUM_FIELDS-1)
                            {
                                dwResult = Edit_LineLength(hWnd, 0);
                                if (dwStart >= dwResult)
                                {
                                    SwitchFields(pControl, dwChildID, dwChildID+1, 0, 0);
                                    return 0;
                                }
                            }
                        }

                        /* If we make it to this point, we're going to pass the key
                           on to the standard processing except that we want VK_UP
                           to act like VK_LEFT and VK_DOWN to act like VK_RIGHT*/
                        if (wParam == VK_UP)
                            wParam = VK_LEFT;
                        else if (wParam == VK_DOWN)
                            wParam = VK_RIGHT;
                    }
                    break;

/* Home jumps back to the beginning of the first field.*/
                case VK_HOME:
                    if (dwChildID > 0)
                    {
                        SwitchFields(pControl, dwChildID, 0, 0, 0);
                        return 0;
                    }
                    break;

/* End scoots to the end of the last field.*/
                case VK_END:
                    if (dwChildID < NUM_FIELDS-1)
                    {
                        SwitchFields(pControl, dwChildID, NUM_FIELDS-1,
                                     CHARS_PER_FIELD, CHARS_PER_FIELD);
                        return 0;
                    }
                    break;
	
	
            } /* switch (wParam)*/

            break;

		case WM_PASTE :
			dwResult = CallWindowProc((WNDPROC)pControl->Children[dwChildID].lpfnWndProc,
									  hWnd, wMsg, wParam, lParam);
			ExitField(pControl, dwChildID);
			return dwResult;
			
        case WM_IME_STARTCOMPOSITION:
            fInComposition = TRUE;      // Mark now in composition phase.
            break;

        case WM_IME_ENDCOMPOSITION:
            fInComposition = FALSE;     // Mark now not in composition phase.
            break;

        case WM_KILLFOCUS:
            PadField(pControl, dwChildID);

            // Flush composition string in IME
            if (fInComposition == TRUE) {
                HIMC hIMC = ImmGetContext(hWnd);
                ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
            }
            break;

    } /* switch (wMsg)*/

    return CallWindowProc((WNDPROC)pControl->Children[dwChildID].lpfnWndProc,
                          hWnd, wMsg, wParam, lParam);
}




/*
    Switch the focus from one field to another.
    call
        pControl = Pointer to the CONTROL structure.
	iOld = Field we're leaving.
	iNew = Field we're entering.
	hNew = Window of field to goto
	wStart = First character selected
	wEnd = Last character selected + 1
    returns
        TRUE on success, FALSE on failure.

    Only switches fields if the current field can be validated.
*/
BOOL SwitchFields(CONTROL  *pControl, int iOld, int iNew, DWORD dwStart, DWORD dwEnd)
{
    if (!ExitField(pControl, iOld))    return FALSE;
    EnterField(&(pControl->Children[iNew]), dwStart, dwEnd);
	return TRUE;
}



/*
    Set the focus to a specific field's window.
    call
	pField = pointer to field structure for the field.
	wStart = First character selected
	wEnd = Last character selected + 1
*/
void EnterField(FIELD  *pField, DWORD dwStart, DWORD dwEnd)
{
    TCHAR szBuf[CHARS_PER_FIELD+1];
    DWORD dwLength;

    SetFocus(pField->hWnd);
    Edit_SetSel(pField->hWnd, dwStart, dwEnd);
    *(DWORD *)szBuf = CHARS_PER_FIELD;
    dwLength = (DWORD)SendMessage(pField->hWnd,EM_GETLINE,0,(DWORD)(LPSTR)szBuf);
    ASSERT(dwLength <= CHARS_PER_FIELD);
    if (dwLength != 0 && dwLength <= CHARS_PER_FIELD) {
        szBuf[dwLength] = '\0';
        pField->dwOrigVal = (DWORD)_ttoi(szBuf);
    }
    else
        pField->dwOrigVal = 0;
}


/*
    Exit a field.
    call
        pControl = pointer to CONTROL structure.
	iField = field number being exited.
    returns
        TRUE if the user may exit the field.
	FALSE if he may not.
*/
BOOL ExitField(CONTROL  *pControl, int iField)
{
    HWND hControlWnd;
    HWND hDialog;
    DWORD dwLength;
    FIELD  *pField;
    TCHAR szBuf[CHARS_PER_FIELD+1];
    int i;

    pField = &(pControl->Children[iField]);
    *(DWORD *)szBuf = CHARS_PER_FIELD;
    dwLength = (DWORD)SendMessage(pField->hWnd,EM_GETLINE,0,(DWORD)(LPSTR)szBuf);
    ASSERT(dwLength <= CHARS_PER_FIELD);

    if (dwLength != 0 && dwLength <= CHARS_PER_FIELD)
    {
        szBuf[dwLength] = '\0';
        i = _ttoi(szBuf);
        if (i < (int)(UINT)pField->byLow || i > (int)(UINT)pField->byHigh)
        {
            if ((hControlWnd = GetParent(pField->hWnd)) != NULL
                && (hDialog = GetParent(hControlWnd)) != NULL)
            {
                pControl->fInMessageBox = TRUE;
                RuiUserMessage(hDialog, NULL, MB_ICONEXCLAMATION, 0,
                               IDS_IPBAD_FIELD_VALUE, i,
                               pField->byLow, pField->byHigh);
                pControl->fInMessageBox = FALSE;
                // Restore original value
                wsprintf(szBuf, TEXT("%d"), pField->dwOrigVal);
                SetWindowText(pField->hWnd, szBuf);
                Edit_SetSel(pField->hWnd, 0, CHARS_PER_FIELD);
                return FALSE;
            }
			// couldn't show warning, restore original value
			wsprintf(szBuf, TEXT("%d"), pField->dwOrigVal);
			SetWindowText(pField->hWnd, szBuf);
			
        } else {
			// This will clean up the field.
			wsprintf(szBuf, TEXT("%d"), i);
			SetWindowText(pField->hWnd, szBuf);
		}

		// Should we pad the field?
        if (dwLength < CHARS_PER_FIELD)
        {
            PadField(pControl, iField);
        }
    }
    return TRUE;
}


/*
    Pad a field.
    call
        hWnd = Field to be padded.

    If the text in the field is shorter than the maximum length, then it
    is padded with 0's on the left.
*/
void PadField(CONTROL  *pControl, int iField)
{
    TCHAR szBuf[CHARS_PER_FIELD+1];
    DWORD dwLength;
    int to, from;
    FIELD  *pField;

    if ((pControl->dwStyle & IP_ZERO) == 0)
        return;

    pField = &(pControl->Children[iField]);
    *(DWORD *)szBuf = CHARS_PER_FIELD;
    dwLength = (DWORD)SendMessage(pField->hWnd,EM_GETLINE,0,(DWORD)(LPSTR)szBuf);
    if (dwLength > 0 && dwLength < CHARS_PER_FIELD)
    {
        szBuf[dwLength] = '\0';
        for (to = CHARS_PER_FIELD-1, from = dwLength-1; dwLength; --dwLength)
            szBuf[to--] = szBuf[from--];
        while (to >= 0)
            szBuf[to--] = '0';
        szBuf[CHARS_PER_FIELD] = '\0';
        SetWindowText(pField->hWnd, szBuf);
    }
}



/*
    Get the value stored in a field.
    call
        pField = pointer to the FIELD structure for the field.
    returns
        The value (0..255) or -1 if the field has not value.
*/
int GetFieldValue(FIELD  *pField)
{
    DWORD dwLength;
    TCHAR szTemp[CHARS_PER_FIELD+1];
    TCHAR szBuf[CHARS_PER_FIELD+1]={0};

    *(WORD *)szTemp = CHARS_PER_FIELD;
    dwLength = (DWORD)SendMessage(pField->hWnd,EM_GETLINE,0,(DWORD)(LPSTR)szTemp);
	LCMapString (LOCALE_SYSTEM_DEFAULT, LCMAP_HALFWIDTH, 
				 szTemp, _tcslen(szTemp)+1, 
				 szBuf, CHARS_PER_FIELD+1);

    if (dwLength > 0 && dwLength <= CHARS_PER_FIELD) 
    {
        szBuf[dwLength] = '\0';
        return _ttoi(szBuf);
    }
    else
        return -1;
}
