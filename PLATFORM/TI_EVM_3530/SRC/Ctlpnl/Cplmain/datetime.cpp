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

#include "cplpch.h"
#include <notify.h>

#define T_SECONDS 1

/***********************************************************************************************
 TimeZone conversion structures/macros
************************************************************************************************/

// FILETIME (100-ns intervals) to minutes (10 x 1000 x 1000 x 60)
#define FILETIME_TO_MINUTES ((__int64)600000000L)

typedef struct tagTZREG {
    LONG    Bias;
    LONG    StandardBias;
    LONG    DaylightBias;
    SYSTEMTIME StandardDate;
    SYSTEMTIME DaylightDate;
} TZREG;

#define ZoneHasDaylightTime(tzi)    (tzi.DaylightDate.wMonth && tzi.DaylightBias)

/***********************************************************************************************
Date and Time CPL Class
************************************************************************************************/
#define M_DI(id)    GetDlgItem(m_hDlg, id)

extern HINSTANCE g_hInst; // From cplmain
static HWND     g_hToolTip = NULL;
static WNDPROC  g_wndProcTT = NULL;

class CDateTime
{
private:
    TIME_ZONE_INFORMATION m_tziCurrent, m_tziSelected;
    HWND    m_hDlg;
    BOOL    m_fTimerOn;
    BOOL    m_fInDST;
    BOOL    m_fTZChanged;
    BOOL    m_fTimeChanged;

public:

    void TimerOn()
    {
        if(!m_fTimerOn)
        {
            SetTimer(m_hDlg, T_SECONDS, 1000, NULL);
            m_fTimerOn = TRUE;
        }
    }

    void TimerOff()
    {
        if(m_fTimerOn)
        {
            KillTimer(m_hDlg, T_SECONDS);
            m_fTimerOn = FALSE;
        }
    }



    CDateTime()  { ZEROMEM(this); }
    ~CDateTime() { TimerOff(); }

    void Init(HWND hDlg)
    {
        m_hDlg = hDlg;

        // disable the apply button until a change occurs
        EnableWindow(DI(IDC_TIME_APPLY),FALSE);

        // get the current time zone
        DWORD tzid = GetTimeZoneInformation(&m_tziCurrent);

        // Does the kernel think we're currently in DST?
        m_fInDST = (TIME_ZONE_ID_DAYLIGHT == tzid );

        //do we want to autoDST?
        CReg reg(HKEY_LOCAL_MACHINE, RK_CLOCK);
        Button_SetCheck(DI(IDC_TIME_DLST), reg.ValueDW(RV_AUTODST,0));

        // If the timezone doesn't support Daylight time at all,
        // GRAY the box
        EnableWindow(DI(IDC_TIME_DLST), ZoneHasDaylightTime(m_tziCurrent));

        // now we need to load the combo box
        HWND hwndCB = DI(IDC_TIME_ZONE);

        // enumerate the keys to fill the combo box
        CReg   Reg1(HKEY_LOCAL_MACHINE, RV_TIMEZONES), Reg2;
        TCHAR  szTimeZone[64];
        TCHAR  szZoneCurr[70];
        while(Reg1.EnumKey(szTimeZone, ARRAYSIZEOF(szTimeZone)))
        {
            LPTSTR szTemp;
            Reg2.Reset();

            if(Reg2.Open(Reg1,szTimeZone))
            {
                // use the display name rather than the standard name
                szTemp = (LPTSTR)Reg2.ValueSZ(RV_DISPLAY);
                Reg2.ValueSZ(RV_STD, szTimeZone, ARRAYSIZEOF(szTimeZone));
                if(szTemp && !lstrcmp(szTimeZone, m_tziCurrent.StandardName))
                {
                    lstrcpy(szZoneCurr,szTemp);
                }
                ComboBox_AddString(hwndCB,szTemp);
            }
        }
        ComboBox_SelectString(hwndCB,0,szZoneCurr);
        SetFocus(hwndCB);

        // selected timezone is same as current
        m_tziSelected = m_tziCurrent;

        // set 'today' explicitly
        SYSTEMTIME st;

        MonthCal_GetCurSel(DI(IDC_TIME_DATE), (LPARAM)&st);
        MonthCal_SetToday(DI(IDC_TIME_DATE), (LPARAM)&st);
    }

    void TimeZoneChanged(void)
    {
        // get display name of selected timezone
        TCHAR   szZoneCurr[70];
        HWND    hwndCB = M_DI(IDC_TIME_ZONE);
        int     index = ComboBox_GetCurSel(hwndCB);
        ComboBox_GetLBText(hwndCB,index, szZoneCurr);

        // Iterate over all timezones. Init new timezone to current just in case
        TCHAR   szTimeZone[64];
        CReg    Reg1(HKEY_LOCAL_MACHINE, RV_TIMEZONES), Reg2;
        while(Reg1.EnumKey(szTimeZone, ARRAYSIZEOF(szTimeZone)))
        {
            Reg2.Reset();
            if(Reg2.Open(Reg1,szTimeZone))
            {
                // if the display name matches the one in the combobox get the TZI data.
                if(!lstrcmp(szZoneCurr, Reg2.ValueSZ(RV_DISPLAY)))
                {
                    TZREG  *pTZR = (TZREG*)Reg2.ValueBinary(RV_TZI);
                    if(pTZR)
                    {
                        m_tziSelected.Bias = pTZR->Bias;
                        m_tziSelected.StandardBias = pTZR->StandardBias;
                        m_tziSelected.DaylightBias = pTZR->DaylightBias;
                        m_tziSelected.StandardDate = pTZR->StandardDate;
                        m_tziSelected.DaylightDate = pTZR->DaylightDate;

                        Reg2.ValueSZ(RV_DLT, m_tziSelected.DaylightName, 32);
                        Reg2.ValueSZ(RV_STD, m_tziSelected.StandardName, 32);

                        DEBUGMSG(ZONE_DATETIME, (L"Changed to TZ(%s, %s) StdBias=%d DaylightBias=%d\r\n",
                            m_tziSelected.StandardName, m_tziSelected.DaylightName, m_tziSelected.Bias, m_tziSelected.DaylightBias));
                        break;
                    }
                }
            }
        }
        // if zone doesnt have a daylight-savings time then disable the checkbox
        EnableWindow(M_DI(IDC_TIME_DLST), ZoneHasDaylightTime(m_tziSelected));

        EnableWindow(M_DI(IDC_TIME_APPLY),TRUE);
        m_fTZChanged = TRUE;
    }

    void CalendarIncrementDay(HWND hwndMC)
    {
        // add a day to the monthcal
        SYSTEMTIME mcSysTime;
        //Prefix
        __int64   mcFileTime = 0;

        // set each of the dates in the monthcal

         // update "today"
        MonthCal_GetToday(hwndMC, (LPARAM)&mcSysTime);
        SystemTimeToFileTime(&mcSysTime, (FILETIME *)&mcFileTime);
        // add 24 hours
        mcFileTime += (((__int64)(24*60)) * FILETIME_TO_MINUTES);
        FileTimeToSystemTime((FILETIME *)&mcFileTime, &mcSysTime);
        MonthCal_SetToday(hwndMC,(LPARAM)&mcSysTime);

        // update the selected time
        MonthCal_GetCurSel(hwndMC, (LPARAM)&mcSysTime);
        SystemTimeToFileTime(&mcSysTime, (FILETIME *)&mcFileTime);
        // add 24 hours
        mcFileTime += (((__int64)(24*60)) * FILETIME_TO_MINUTES);
        FileTimeToSystemTime((FILETIME *)&mcFileTime, &mcSysTime);
        MonthCal_SetCurSel(hwndMC,(LPARAM)&mcSysTime);

    }

    void Update(void)
    {
        SYSTEMTIME cpltime, tmptime;
        // to allow the user to get good feedback about the current timezone
        // daylight savings time etc, we actually update the system on the fly
        // for these items the changes will be reflected in the time of each date
        // control

        // load the 'clock'
        DateTime_GetSystemtime(M_DI(IDC_TIME_TIME),(LPARAM)&cpltime);
        // load the 'date'
        MonthCal_GetCurSel(M_DI(IDC_TIME_DATE), (LPARAM)&tmptime);
        // copy the dates into cpltime
        cpltime.wYear = tmptime.wYear;
        cpltime.wDayOfWeek = tmptime.wDayOfWeek;
        cpltime.wDay = tmptime.wDay;
        cpltime.wMonth = tmptime.wMonth;

        if ( !UpdateTimeZone(&cpltime) && m_fTimeChanged)
        {
            // Need to change system time but didn't do it in UpdateTimeZone
            SetLocalTime(&cpltime);
        }

        // Update the clock & calendar display, since things may have changed
        GetLocalTime(&cpltime);
        DateTime_SetSystemtime(M_DI(IDC_TIME_TIME), GDT_VALID, &cpltime);
        MonthCal_SetToday(M_DI(IDC_TIME_DATE), &cpltime);
        MonthCal_SetCurSel(M_DI(IDC_TIME_DATE), &cpltime);

        m_fTimeChanged = FALSE;
    }
    void UpdateTime()   { m_fTimeChanged = TRUE; }

    //  Returns TRUE if it did change the system time, FALSE otherwise
    BOOL UpdateTimeZone(SYSTEMTIME *pstOld)
    {
        BOOL bTimeChanged = FALSE;

        /*
            Get current state of the "DST-active" checkbox
            If new timezone doesnt support DST, ignore the checkbox
            Otherwise, DST is ready to go
        */
        BOOL fUseDST = Button_GetCheck(M_DI(IDC_TIME_DLST)) && ZoneHasDaylightTime(m_tziSelected);

        //need to update the time if the timezone changed before updating for DST
        if (m_fTZChanged )
        {
            // calculate difference between Bias of previous & new timezones
            LONG OldBias = m_tziCurrent.Bias + (m_fInDST? m_tziCurrent.DaylightBias : m_tziCurrent.StandardBias);
            LONG NewBias = m_tziSelected.Bias + (m_fInDST ? m_tziSelected.DaylightBias : m_tziSelected.StandardBias);

            // Since UTC=Local+Bias, NewLocal=OldLocal+(OldBias-NewBias)
            SYSTEMTIME  stNew;

            __int64     ftOld = 0;
            __int64     ftNew = 0;

            SystemTimeToFileTime(pstOld, (FILETIME*)&ftOld);

            DEBUGMSG(ZONE_DATETIME, (L"TZ Change: Updating clock. Old Bias=%d New=%d, Diff=%d\r\n", OldBias, NewBias, OldBias-NewBias));
            ftNew = ftOld + (((__int64)(OldBias-NewBias)) * FILETIME_TO_MINUTES);
            FileTimeToSystemTime((FILETIME*)&ftNew, &stNew);

            // now update the clock
            SetLocalTime(&stNew);
            // update our notion of "current" timezone
            SetTimeZoneInformation(&m_tziSelected);

            m_tziCurrent = m_tziSelected;
            m_fTZChanged = FALSE;

            if (ZoneHasDaylightTime(m_tziSelected))
            {
                m_fInDST = fUseDST;
               }
               else
               {
                   m_fInDST = fUseDST = FALSE;
               }

            bTimeChanged = TRUE;
        }

        CReg reg(HKEY_LOCAL_MACHINE, RK_CLOCK);

        reg.SetDW(RV_AUTODST, fUseDST);

        /* if the auto adjust for DST box is checked, we need to reset the DST event.
            the time or date may have been adjusted enough to make us miss any currently
            scheduled events, so the best solution is to refresh the timezone information
            so that the DST event is rescheduled based on the current date/dimezone settings
        */
        //if (fUseDST )
        {
            DEBUGMSG(ZONE_DATETIME, (_T("DateTime:  Resetting Timezone Info")));
            SetTimeZoneInformation(&m_tziSelected);
        }

        return bTimeChanged;
    }

};

LRESULT CALLBACK DT_ToolTipProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (TTM_WINDOWFROMPOINT == uMsg)
    {
        ASSERT(lParam);
        POINT pt = (*((POINT FAR *)lParam));
        HWND hDlg = (HWND) GetWindowLong(hwnd, GWL_USERDATA);
        MapWindowPoints(NULL, hDlg, &pt, 1);
        return ((LRESULT) ChildWindowFromPoint(hDlg, pt));
    }

    ASSERT(g_wndProcTT);
    return CallWindowProc(g_wndProcTT, hwnd, uMsg, wParam, lParam);
}



extern "C" BOOL APIENTRY DateTimeDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    CDateTime *pDateTime = (CDateTime *)GetWindowLong(hDlg,DWL_USER);
    CReg reg(HKEY_LOCAL_MACHINE, RK_CLOCK);


    if(message != WM_INITDIALOG && pDateTime == NULL)
    {
        DEBUGCHK(FALSE);
        return (FALSE);
    }

    switch(message)
    {
        case WM_INITDIALOG:
            DEBUGCHK(pDateTime == NULL);
            pDateTime = new CDateTime;

            //Prefix
            if(pDateTime) {
                // set the window long DWL_USER, (GWL_USERDATA is CRunningTab)
                SetWindowLong(hDlg, DWL_USER, (LONG)pDateTime);
                // setup the window
                pDateTime->Init(hDlg);
                // set a timer to update the clock's seconds
                pDateTime->TimerOn();

                // Create the tooltip window for long strings
                g_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                                 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 hDlg, NULL, g_hInst, NULL);
                ASSERT(g_hToolTip);

                if (g_hToolTip)
                {
                    TOOLINFO ti = {0};
                    ti.cbSize = sizeof(ti);
                    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_TRANSPARENT;
                    ti.hwnd = hDlg;
                    ti.lpszText = LPSTR_TEXTCALLBACK;
                    ti.uId = (UINT) DI(IDC_TIME_ZONE);
                    SendMessage(g_hToolTip, TTM_ADDTOOL, NULL, (LPARAM) &ti);

                    g_wndProcTT = (WNDPROC) SetWindowLong(g_hToolTip, GWL_WNDPROC,
                                                          (LONG) DT_ToolTipProc);
                    ASSERT(g_wndProcTT);
                    SetWindowLong(g_hToolTip, GWL_USERDATA, (LONG) hDlg);
                }

                //IME DISABLE
                ImmAssociateContext(GetDlgItem(hDlg, IDC_TIME_TIME), (HIMC)NULL);

            }

            AygInitDialog( hDlg, SHIDIF_SIPDOWN );
            return 1;

        case WM_DESTROY:
            delete pDateTime;
            pDateTime = NULL;
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_TIME_ZONE:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                        pDateTime->TimeZoneChanged();
                       return TRUE;

                case IDOK:
                    // tell the dialog to save settings
                    pDateTime->Update();
                    return TRUE;

                case IDC_TIME_APPLY:
                    // reset the timer to update the clock's seconds
                    pDateTime->Update();
                    // restart the timer if it has been stopped
                    pDateTime->TimerOn();

                    // disable the apply button until another change occurs
                    SetFocus(DI(IDC_TIME_DATE));
                    EnableWindow(DI(IDC_TIME_APPLY), FALSE);
                    return (TRUE);

                case IDC_TIME_DLST:
                       // enable Apply button
                       EnableWindow(DI(IDC_TIME_APPLY), TRUE);
                    return (TRUE);
            }
            break;

         case WM_TIMER:
            if(wParam == T_SECONDS)
            {
                   SYSTEMTIME stime;
                GetLocalTime(&stime);
                if(stime.wHour == 0 && stime.wMinute == 0 && stime.wSecond == 0)
                    pDateTime->CalendarIncrementDay(DI(IDC_TIME_DATE));
                DateTime_SetSystemtime(DI(IDC_TIME_TIME), GDT_VALID, &stime);
                return (TRUE);
            }
            break;

         case WM_NOTIFY:
            switch (((LPNMHDR)lParam)->code)
            {
                case DTN_DATETIMECHANGE:
                    // kill our timer since we don't want the user to get confused
                    pDateTime->TimerOff();
                    // set the apply button
                    EnableWindow(DI(IDC_TIME_APPLY),TRUE);

                    pDateTime->UpdateTime();
                    return TRUE;

                case MCN_SELCHANGE:
                    pDateTime->UpdateTime();
                    EnableWindow(DI(IDC_TIME_APPLY),TRUE);
                    return TRUE;


                case TTN_GETDISPINFO:
                {
                    ASSERT(TTF_IDISHWND & ((LPNMTTDISPINFO) lParam)->uFlags);

                    COMBOBOXINFO cbi = {0};
                    HDC hdc = NULL;
                    int textLen = 0;
                    TCHAR * pszText = NULL;
                    SIZE size = {0};

                    cbi.cbSize = sizeof(cbi);
                    SendMessage((HWND) ((LPNMHDR) lParam)->idFrom,
                                CB_GETCOMBOBOXINFO, NULL, (LPARAM) &cbi);

                    if (hdc = GetDC(cbi.hwndItem))
                    {
                        textLen = SendMessage(cbi.hwndItem, WM_GETTEXTLENGTH, NULL, NULL);
                        if (textLen && (pszText = new TCHAR[textLen+1]))
                        {

                            SendMessage(cbi.hwndItem, WM_GETTEXT, textLen+1, (LPARAM) pszText);

                            if (GetTextExtentPoint(hdc, pszText, textLen, &size) &&
                                (size.cx >= (cbi.rcItem.right-cbi.rcItem.left)))
                            {
                                LPNMTTDISPINFO lpnmttdi = (LPNMTTDISPINFO) lParam;
                                ZeroMemory(lpnmttdi->szText, sizeof(lpnmttdi->szText));
                                _tcsncpy(lpnmttdi->szText, pszText, 79); // See NMTTDISPINFO in MSDN
                            }
                            delete [] pszText;
                        }
                        ReleaseDC(cbi.hwndItem, hdc);
                    }
                    return TRUE;
                }

                case TTN_SHOW:
                {
                    HWND hCombo = (HWND) ((LPNMHDR) lParam)->idFrom;
                    ASSERT(hCombo);
                    COMBOBOXINFO cbi = {0};
                    cbi.cbSize = sizeof(cbi);
                    SendMessage(hCombo, CB_GETCOMBOBOXINFO, NULL, (LPARAM) &cbi);

                    RECT rcCombo = {0};
                    GetWindowRect(hCombo, &rcCombo);

                    RECT rcToolTip = {0};
                    GetWindowRect(g_hToolTip, &rcToolTip);
                    LONG lToolTipWidth = rcToolTip.right - rcToolTip.left;

                    HMONITOR hMonitor = MonitorFromWindow(hCombo, MONITOR_DEFAULTTONEAREST);
                    ASSERT(hMonitor);
                    if (hMonitor)
                    {
                        MONITORINFO mi = {0};
                        mi.cbSize = sizeof(mi);
                        GetMonitorInfo(hMonitor, &mi);

                        rcCombo.top += cbi.rcItem.top;
                        rcCombo.left += cbi.rcItem.left;
                        if (mi.rcWork.right < (rcCombo.left+lToolTipWidth))
                        {
                            rcCombo.left -= cbi.rcItem.left;
                            rcCombo.left -= (rcCombo.left+lToolTipWidth)-mi.rcWork.right;
                        }
                    }

                    // TTM_ADJUSTRECT is not supported on CE
                    // SendMessage(g_hToolTip, TTM_ADJUSTRECT, TRUE, (LPARAM) &rc);
                    SetWindowPos(g_hToolTip, NULL, rcCombo.left, rcCombo.top,
                                 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
                    SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
                    return TRUE;
                }
            }
            break;

         case WM_SYSCOLORCHANGE:
         case WM_WININICHANGE:
            SendMessage(DI(IDC_TIME_TIME),message,wParam,lParam);
            SendMessage(DI(IDC_TIME_DATE),message,wParam,lParam);
            return (TRUE);
    }
    return (FALSE);
}

