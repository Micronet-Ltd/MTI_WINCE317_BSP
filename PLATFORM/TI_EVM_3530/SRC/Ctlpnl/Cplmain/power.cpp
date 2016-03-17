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
/**
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Abstract: Code specific to the Power CPL
**/
#include "cplpch.h"
#include <pnp.h>
#include <pm.h>
#include <msgqueue.h>
#include "regcpl.h"

#define WM_USER_NOTIFICATION (WM_USER + 1)

#define STR_SECONDS				_T(" seconds")

const TCHAR	IncompatibilityStr[] = _T("The incompatibility with timeout settings!");

BOOL	NotifyBlight();
void	SetBlReg( DWORD bEnableBatTime, DWORD iBatteryTime, DWORD bEnableExtTime, DWORD iExternalTime );
void	GetBlReg( DWORD& bEnableBatTime, DWORD& iBatteryTime, DWORD& bEnableExtTime, DWORD& iExternalTime );
DWORD	GetIgniFromReg();
BOOL	SetIgniToReg(DWORD);

/////////

// these tabs are not exposed publicly in the rgApplets table -- instead,
// we add them dynamically when the power applet is instantiated.
const CPLTABINFO SleepTab = { 
	IDS_POWER_SLEEPTAB, IDD_SLEEP, L"SleepDlgProc", 0,0,0, L"file:ctpnl.htm#conserve_batteries_with_auto_suspend",
};
const CPLTABINFO PowerDeviceTab = { 
	IDS_POWER_DEVICETAB, IDD_POWER_DEVICE, L"PowerDeviceDlgProc", 0,0,0, L"ctpnl.htm#device_power_status",
};
const CPLTABINFO TimeoutsTab = {	
	IDS_POWER_TIMEOUTSTAB, IDD_POWER_TIMEOUTS, L"PowerTimeoutsDlgProc", 0,0,0, L"ctpnl.htm#power_schemes",
};

void BatteryOnChange(HWND hDlg, const PPOWER_BROADCAST ppb);

const int BatteryTime1[] = 	{				// in seconds
							1*60,
							2*60,
							3*60,
							4*60,
							5*60,
							};

const int BatteryTime2[] = 	{				// in seconds
							1*60,
							2*60,
							5*60,
							10*60,
							15*60,
							30*60,
							};

#define DEFAULT_TIME1       2		// 3 min
#define DEFAULT_TIME2       2		// 5 min**/


#define NUM_BATTERIES 2
#define NUM_BATTERY_LEVELS 3

static const int rgBatteryLevelCtls[NUM_BATTERIES][NUM_BATTERY_LEVELS] = {
	{ IDC_STATIC_GOOD, IDC_STATIC_LOW, IDC_STATIC_VLOW },
	{ IDC_STATIC_GOOD1, IDC_STATIC_LOW1, IDC_STATIC_VLOW1 }
};

static const int rgBatteryLevels[NUM_BATTERY_LEVELS] = 
	{ BATTERY_FLAG_HIGH, BATTERY_FLAG_LOW, BATTERY_FLAG_CRITICAL };

// Indices into iBatteryLevelControls array
#define MAIN_BATTERY 0
#define BACKUP_BATTERY 1

#define LOW_CONTROL 1


HANDLE g_hPowerTerminateEvent; // To tell PowerNotificationThread to exit


void GetSuspendTimes(int &iMinutes1, int& iMinutes2)
{
	CReg reg(HKEY_LOCAL_MACHINE, RK_SYSTEM_CCS_CONTROL_POWER);

	iMinutes1 = reg.ValueDW(RV_BATTPOWEROFF);
	iMinutes2 = reg.ValueDW(RV_EXTPOWEROFF);
}

void SaveSuspendTimes(int iMinutes1, int iMinutes2)
{
	CReg reg;
	
	reg.Create(HKEY_LOCAL_MACHINE, RK_SYSTEM_CCS_CONTROL_POWER);

	reg.SetDW(RV_BATTPOWEROFF, iMinutes1);
	reg.SetDW(RV_EXTPOWEROFF, iMinutes2);

	NotifyWinUserSystem(NWUS_MAX_IDLE_TIME_CHANGED);
}

void UpdateLB(HWND hDlg, int CtlId)
{
	HWND hwndCB;
	int i, iMax;
	TCHAR szBuf[20];
	
	hwndCB = DI(CtlId);
	
	ComboBox_ResetContent(hwndCB);
	if (CtlId == IDC_COMBO2) {
		iMax =  ARRAYSIZEOF(BatteryTime2);
	}else{
		iMax =  ARRAYSIZEOF(BatteryTime1);
	}
	
	for (i = 0; i < iMax ; i++)	 
	{	
		// Should use FormatMessage() but that makes it require the FormatMessage() component
		StringCbPrintf( szBuf, sizeof(szBuf), TEXT("%d %s"), 
				  (CtlId==IDC_COMBO1 ? BatteryTime1[i] : BatteryTime2[i])/60,
				  CELOADSZ(i==0 ? IDS_POWER_MINUTE : IDS_POWER_MINUTES) );
		ComboBox_AddString(hwndCB, szBuf);
	}
}		

int FindCurrentIndex(int iCurrTime, const int rgTimes[], int iMax, int iDefault)
{
	if(iCurrTime)
	{
		for(int i=0; i < iMax; i++)
		{
			if(iCurrTime == rgTimes[i])
				return i;
		}
	}
	return iDefault;
}

void ShowHideCombo2(HWND hDlg, int swshow)
{
	ShowWindow(DI(IDC_STATIC3), swshow);
	ShowWindow(DI(IDC_STATIC4), swshow);
	ShowWindow(DI(IDC_COMBO2), swshow);
}


// The Sleep dialog box is only used when the new power manager features are
// not used.
extern "C" BOOL APIENTRY SleepDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			int m1, m2;
	
			GetSuspendTimes(m1, m2);
			// SendDlgItemMessage(hDlg, IDC_COMBO1, CB_SETEXTENDEDUI, TRUE, 0);
			// SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETEXTENDEDUI, TRUE, 0);
			
			UpdateLB(hDlg, IDC_COMBO1);
			UpdateLB(hDlg, IDC_COMBO2);								    
			
			ComboBox_SetCurSel(DI(IDC_COMBO1), 
				FindCurrentIndex(m1, BatteryTime1, ARRAYSIZEOF(BatteryTime1), DEFAULT_TIME1));
			
			ComboBox_SetCurSel(DI(IDC_COMBO2), 
				FindCurrentIndex(m2, BatteryTime2, ARRAYSIZEOF(BatteryTime2), DEFAULT_TIME2));
			

			Button_SetCheck(DI(IDC_CHECK2), (m2 > 0));
			ShowHideCombo2(hDlg, (m2>0) ? SW_SHOW : SW_HIDE);
			return TRUE; // let gwe set focus
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CHECK2:
					ShowHideCombo2(hDlg, Button_GetCheck(DI(IDC_CHECK2)) ? SW_SHOW : SW_HIDE);
					return TRUE;

				case IDOK:	
				{
					int m1=0, m2=0;
					int iSel;
					iSel = ComboBox_GetCurSel(DI(IDC_COMBO1));
					if(iSel >= 0)
						m1 = BatteryTime1[iSel];

	
					if(Button_GetCheck(DI(IDC_CHECK2)))
					{
						iSel = ComboBox_GetCurSel(DI(IDC_COMBO2));
						if (iSel >= 0)
							m2 = BatteryTime2[iSel];
					}
					SaveSuspendTimes(m1, m2);
				}
				return TRUE;
			}
			break;
		}
	}
	return (FALSE);
}


// Report power/battery changes to the battery dialog
static
DWORD
PowerNotificationThread (PVOID lpParam)
{
	DEBUGCHK(lpParam);
	DEBUGCHK(g_hPowerTerminateEvent);

	BYTE pbMsgBuf[sizeof(POWER_BROADCAST) + sizeof(POWER_BROADCAST_POWER_INFO)];
	MSGQUEUEOPTIONS msgopts;

    HWND hDlg = (HWND)lpParam;
	HANDLE rghWaits[2] = { NULL };
	HANDLE hReq = NULL;
	DWORD dwRet = 1; // 1 is error

	// Create our message queue
	memset(&msgopts, 0, sizeof(msgopts));
    msgopts.dwSize = sizeof(msgopts);
    msgopts.dwFlags = 0;
    msgopts.dwMaxMessages = 0;
    msgopts.cbMaxMessage = sizeof(pbMsgBuf);
    msgopts.bReadAccess = TRUE;

	rghWaits[0] = CreateMsgQueue(NULL, &msgopts);
	if (!rghWaits[0]) {
		ERRORMSG(1, (TEXT("Could not create power message queue\r\n")));
		goto EXIT;
	}

	// Request notifications
	hReq = RequestPowerNotifications(rghWaits[0], PBT_POWERINFOCHANGE);
	if (!hReq) {
		ERRORMSG(1, (TEXT("RequestPowerNotifications() failed\r\n")));
		goto EXIT;
	}

	rghWaits[1] = g_hPowerTerminateEvent;

    while(TRUE)
    {
        DWORD dwWaitCode = WaitForMultipleObjects(2, rghWaits, FALSE, INFINITE);
        if(dwWaitCode == WAIT_OBJECT_0)
        {
			DWORD dwSize, dwFlags;

            if (ReadMsgQueue(rghWaits[0], pbMsgBuf, sizeof(pbMsgBuf), &dwSize, 0, &dwFlags)) {
                DEBUGMSG(ZONE_POWER, (TEXT("Received power notification\r\n")));
				SendMessage(hDlg, WM_USER_NOTIFICATION, 0, (LPARAM) pbMsgBuf);
            }
			else {
				DEBUGCHK(FALSE); // We should never get here
			}
        }
        else {
            // got terminate signal
            break;
        }
    }

	dwRet = 0; // Success!

EXIT:
	// Clean up
	if (hReq) StopPowerNotifications(hReq);
	if (rghWaits[0]) CloseHandle(rghWaits[0]);

	DEBUGMSG(ZONE_POWER, (TEXT("PowerNotificationThread exiting\r\n")));

    return dwRet;
}



extern "C" BOOL APIENTRY BatteryDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static HANDLE hThread = NULL;

    switch (message)
    {
      case WM_INITDIALOG:
	  {
		  // Set up the dialog		  
		  int iTemp = BatteryDrvrGetLevels();
		  int rgActualLevels[NUM_BATTERIES];
		  rgActualLevels[MAIN_BATTERY]   = (int)LOWORD(iTemp);
		  rgActualLevels[BACKUP_BATTERY] = (int)HIWORD(iTemp);
		  
		  DEBUGMSG(ZONE_POWER, (L"MainBatteryLevel=%x BackupLevel=%x\r\n", 
			  rgActualLevels[MAIN_BATTERY], rgActualLevels[BACKUP_BATTERY]));
		  
		  for (int iBattery = 0; iBattery < NUM_BATTERIES; ++iBattery) {
			  for (int iLevel = 0; iLevel < rgActualLevels[iBattery]; ++iLevel) {
				  ShowWindow(DI(rgBatteryLevelCtls[iBattery][iLevel]), SW_SHOW);
			  }
			  
			  if (rgActualLevels[iBattery] == 2) {
				  SetDlgItemText(hDlg, rgBatteryLevelCtls[iBattery][LOW_CONTROL],
					  CELOADSZ(IDS_POWER_VERYLOW));
			  }
		  }

		  // Set up the power notification thread
		  g_hPowerTerminateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
		  if (g_hPowerTerminateEvent) {
			  hThread = CreateThread(NULL, 0, PowerNotificationThread, (LPVOID)hDlg, 0, NULL);
			  
			  if (!hThread) {
				  DEBUGMSG(ZONE_ERROR, 
					  (TEXT("Error: Could not start notification thread\r\n")));
			  }
		  }
		  else {
			  DEBUGMSG(ZONE_ERROR, (TEXT("Error: Unable to create event\r\n")));
		  }
		  
		  return TRUE; // let GWE set focus
	  }

	  case WM_USER_NOTIFICATION:
		  BatteryOnChange(hDlg, (PPOWER_BROADCAST) lParam);
		  return TRUE;

      case WM_COMMAND:
      {
	    switch (LOWORD(wParam))
	    {
	      case IDCANCEL:
		  case IDOK:
			if (g_hPowerTerminateEvent) {
				// Close our thread.
				if (hThread) {
					SetEvent(g_hPowerTerminateEvent);
					WaitForSingleObject(hThread, INFINITE);
					CloseHandle(hThread);
					hThread = NULL;
				}

				// Close our event
				CloseHandle(g_hPowerTerminateEvent);
				g_hPowerTerminateEvent = NULL;
			}
			else {
				DEBUGCHK(!hThread); // hThread exists only if the event is good
			}
			return TRUE;
		}
		break;
	  }
	}

	return (FALSE);
}

const int rgOptionalCtls1[] = { IDC_BAT1, IDC_BAT2,IDC_BAT1_STATIC, IDC_BAT2_STATIC };
const int rgOptionalCtls2[] = { IDC_BAT3, IDC_BAT3_STATIC };
const int rgOneOfCtls[] = { IDC_STATIC_CHARGE, IDC_STATIC_EXTERNAL, IDC_STATIC_MAIN };

// if text has changed, update it
#define UpdateCtl(x) \
	GetDlgItemText(hDlg, x, szTemp2, CCHSIZEOF(szTemp2)); \
	if (lstrcmp(szTemp1, szTemp2)) { SetDlgItemText(hDlg, x, szTemp1); }

void BatteryOnChange(HWND hDlg, const PPOWER_BROADCAST ppb)
{
	int         rgBatteryFlags[NUM_BATTERIES];
	int         iBattery;
	int         idcShow;
	
	#define	LEN_TEMP	256
	TCHAR		szTemp1[LEN_TEMP];
	TCHAR		szTemp2[LEN_TEMP];

	PPOWER_BROADCAST_POWER_INFO ppbpi = (PPOWER_BROADCAST_POWER_INFO) ppb->SystemPowerState;

	// Optional stuff at bottom of prop-page

	// if supported by driver, get lifetime info
	szTemp1[0]=0;
	if(!BatteryDrvrSupportsChangeNotification())
	{
		// Hide these battery indicators if we don't support it (if not already hidden)
	    if(IsWindowVisible(DI(rgOptionalCtls1[0])))
			ShowHide(hDlg, rgOptionalCtls1, ARRAYSIZEOF(rgOptionalCtls1), SW_HIDE);
	}
	else
	{
		SYSTEMTIME	stLastChange;
		DWORD       dwCpuUsage, dwPrevCpuUsage;
		int 		iLen;
		
		// Show these battery indicators if we support them (if not already visible)
	    if(!IsWindowVisible(DI(rgOptionalCtls1[0])))
			ShowHide(hDlg, rgOptionalCtls1, ARRAYSIZEOF(rgOptionalCtls1), SW_SHOW);
		
		BatteryGetLifeTimeInfo(&stLastChange, &dwCpuUsage, &dwPrevCpuUsage);
		
		// format LastChange as string
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &stLastChange, NULL, szTemp1, CCHSIZEOF(szTemp1));
		iLen = lstrlen(szTemp1);
		if(iLen+2 < CCHSIZEOF(szTemp1))
		{
			szTemp1[iLen++] = L' ';
			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &stLastChange, NULL, szTemp1+iLen, CCHSIZEOF(szTemp1)-iLen);
		}
		UpdateCtl(IDC_BAT1);
			
		DEBUGMSG(ZONE_POWER, (L"1 Battery CpuUsage=%d PrevCpuUsage=%d (Text=%s)\r\n", 
			dwCpuUsage, dwPrevCpuUsage, szTemp1));
	
		// format CpuUsage as string
		dwCpuUsage /= (1000*60);  // convert to minutes
		StringCbPrintf(szTemp1, sizeof(szTemp1), TEXT("%d:%02d"), dwCpuUsage/60, dwCpuUsage%60);
		UpdateCtl(IDC_BAT2);

		DEBUGMSG(ZONE_POWER, (L"1 Battery CpuUsage=%s\r\n", szTemp1));
	}

	DEBUGMSG(ZONE_POWER, (L"Battery status: Main=%x Backup=%x AC=%x \r\n", 
		ppbpi->bBatteryFlag, ppbpi->bBackupBatteryFlag, ppbpi->bACLineStatus));

	// update battery percentage 
	if(ppbpi->bBatteryLifePercent < 0 || ppbpi->bBatteryLifePercent > 100)
	{
	   // Hide these battery indicators if we don't support it (if not already hidden)
	   if(IsWindowVisible(DI(rgOptionalCtls2[0])))
		   ShowHide(hDlg, rgOptionalCtls2, ARRAYSIZEOF(rgOptionalCtls2), SW_HIDE);
	}
	else
	{
	   wsprintf(szTemp1, TEXT("%d%%"), ppbpi->bBatteryLifePercent);
	   UpdateCtl(IDC_BAT3);
      
	   if(!IsWindowVisible(DI(rgOptionalCtls2[0])))
		   ShowHide(hDlg, rgOptionalCtls2, ARRAYSIZEOF(rgOptionalCtls2), SW_SHOW);
	}

	// Show exactly ONE of "Charging", "External" or "Main Battery"
	if(ppbpi->bBatteryFlag & BATTERY_FLAG_CHARGING) idcShow = IDC_STATIC_CHARGE;
	else if(ppbpi->bACLineStatus == AC_LINE_ONLINE) idcShow = IDC_STATIC_EXTERNAL;
	else idcShow = IDC_STATIC_MAIN;

	for (int i=0; i<ARRAYSIZEOF(rgOneOfCtls); ++i) {
		ShowWindow(DI(rgOneOfCtls[i]), (idcShow==rgOneOfCtls[i]) ? SW_SHOW :SW_HIDE);
	}

	rgBatteryFlags[MAIN_BATTERY]   = ppbpi->bBatteryFlag;
	rgBatteryFlags[BACKUP_BATTERY] = ppbpi->bBackupBatteryFlag;

	// Clear any battery flag set to "unknown"
	for (iBattery = 0; iBattery < NUM_BATTERIES; ++iBattery) {
		if (rgBatteryFlags[iBattery] == BATTERY_FLAG_UNKNOWN) {
			rgBatteryFlags[iBattery] = 0;
		}
	}

	// Highlight batteries' status as appropriate
	for (iBattery = 0; iBattery < NUM_BATTERIES; ++iBattery) {
		for (int iLevel = 0; iLevel < NUM_BATTERY_LEVELS; ++iLevel) {
			EnableWindow(DI(rgBatteryLevelCtls[iBattery][iLevel]), 
				(rgBatteryFlags[iBattery] & rgBatteryLevels[iLevel]));
		}
	}	
}



//
// The following is all for the new power manager's power applet
//


// The Device Power Tab

static HANDLE g_hDeviceTerminateEvent; // To tell DeviceNotificationThread to exit

#define PWRMGR_STATE_KEY PWRMGR_REG_KEY TEXT("\\State")
#define PWRMGR_INTERFACES_KEY PWRMGR_REG_KEY TEXT("\\Interfaces")


// this routine converts a text string to a GUID if possible
static
BOOL 
ConvertStringToGuid (LPCTSTR pszGuid, GUID *pGuid)
{
    UINT Data4[8];
    int  Count;
    BOOL fOk = FALSE;
    TCHAR *pszGuidFormat = _T("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

    DEBUGCHK(pGuid != NULL && pszGuid != NULL);
    __try {
        if (_stscanf(pszGuid, pszGuidFormat, &pGuid->Data1, 
            &pGuid->Data2, &pGuid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], 
            &Data4[4], &Data4[5], &Data4[6], &Data4[7]) != 11) 
        {
            DEBUGMSG(ZONE_WARNING, (_T("Couldn't parse '%s'\r\n"), pszGuid));
        } else {
            for(Count = 0; Count < sizeof(Data4)/sizeof(Data4[0]); Count++) {
                pGuid->Data4[Count] = (UCHAR) Data4[Count];
            }
        }
        fOk = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DEBUGMSG(ZONE_WARNING, (_T("Error parsing guid\r\n")));
    }

    return fOk;
}


// Returns the count of queues in *pcActualHandles. Also returns FALSE on error. 
// Pass in NULL for rghMsgQues and rghReqs and 0 for cHandles to simply get the count.
// cHandles is the number of HANDLEs allocated in the arrays.
static
BOOL
GetPowerManagedMsgQueues(HANDLE *rghMsgQueues, HANDLE *rghReqs, DWORD cHandles, DWORD cbMaxMessage, DWORD *pcActualHandles)
{
	DEBUGCHK((rghMsgQueues != NULL && rghReqs != NULL && cHandles >= 0) || 
		     (rghMsgQueues == NULL && rghReqs == NULL && cHandles == 0));
    DEBUGCHK(pcActualHandles);

    MSGQUEUEOPTIONS msgopts;
	CReg reg(HKEY_LOCAL_MACHINE, PWRMGR_INTERFACES_KEY);
	DWORD cActualHandles = 0; // How many did we find in the registry
	const int cchGUID = MAX_PATH;
	TCHAR szGUID[cchGUID + 1];
	LPTSTR szTmp = NULL;
	szGUID[cchGUID] = '\0';
    GUID guid;
    BOOL fRet = FALSE;

	if (!reg.IsOK()) {
		DEBUGMSG(ZONE_WARNING, (TEXT("Could not open key %s\r\n"), PWRMGR_INTERFACES_KEY));
		goto EXIT;
	}

	memset(&msgopts, 0, sizeof(msgopts));
    msgopts.dwSize = sizeof(msgopts);
    msgopts.dwFlags = 0;
    msgopts.dwMaxMessages = 0;
    msgopts.cbMaxMessage = cbMaxMessage;
    msgopts.bReadAccess = TRUE;
    
	// If the user has sent us a place to put the handles run through them and create all the queues we have room for.
	if (rghMsgQueues && rghReqs && cHandles > 0)
	{
		while ((cActualHandles < cHandles) && reg.EnumValue(szGUID, cchGUID, szTmp, 0))
		{
			if (ConvertStringToGuid(szGUID, &guid))
			{
				// Then the user wants us to perform the creations
				rghMsgQueues[cActualHandles] = CreateMsgQueue(NULL, &msgopts);
				if (!rghMsgQueues[cActualHandles]) {
					DEBUGMSG(ZONE_ERROR, (TEXT("Unable to create MsgQueue\r\n")));
					goto EXIT;
				}

				rghReqs[cActualHandles] = 
					RequestDeviceNotifications(&guid, rghMsgQueues[cActualHandles], TRUE);
				if (!rghReqs[cActualHandles]) {
					DEBUGMSG(ZONE_ERROR, (TEXT("Unable to RequestDeviceNotifications\r\n")));
					goto EXIT;
				}
				
				++cActualHandles;
			}
		}
	}

	// now run through any remaining to get the total actual
	while (reg.EnumValue(szGUID, cchGUID, szTmp, 0))
	{
		if (ConvertStringToGuid(szGUID, &guid))
		{
			++cActualHandles;
		}
	}


    *pcActualHandles = cActualHandles;
    fRet = TRUE;

EXIT:
	return fRet;
}


union BufferPlusDevDetail
{
	DEVDETAIL DevDetail;
	BYTE byBuffer[sizeof(DEVDETAIL) + MAX_DEVCLASS_NAMELEN * sizeof(TCHAR)];
};
	

// Report power-managed devices to the device power dialog
static
DWORD
DeviceNotificationThread (PVOID lpParam)
{
	DEBUGCHK(lpParam);
	DEBUGCHK(g_hDeviceTerminateEvent);

    HWND hDlg = (HWND)lpParam;
	DWORD dwSize, dwFlags;
	
	BufferPlusDevDetail Buff={0};
    
    HANDLE *rghWaits = NULL;
	HANDLE *rghReqs = NULL;
	DWORD cHandles = 0;
    DWORD cHandlesOriginal;
	DWORD dwRet = 1; // 1 is error

    if (!GetPowerManagedMsgQueues(NULL, NULL, 0, sizeof(Buff), &cHandles)) {
		goto EXIT;
	}

	rghWaits = MyRgAlloc(HANDLE, cHandles + 1); // +1 for terminate event
	rghReqs  = MyRgAlloc(HANDLE, cHandles);

#ifdef DEBUG
    {
        // Verify that the arrays are initialized to 0. Error clean up will
        // fail if they are not 0.
        DWORD dwDebug;

        if (rghWaits) {
            for (dwDebug = 0; dwDebug < cHandles; ++dwDebug) {
                DEBUGCHK(rghWaits[dwDebug] == NULL);
            }
            DEBUGCHK(rghWaits[cHandles] == NULL);
        }

        if (rghReqs) {
            for (dwDebug = 0; dwDebug < cHandles; ++dwDebug) {
                DEBUGCHK(rghReqs[dwDebug] == NULL);
            }
        }
    }
#endif

	if (!rghWaits || !rghReqs) {
		RETAILMSG(1, (TEXT("Power: Out of memory\r\n")));
		goto EXIT;
	}

    cHandlesOriginal = cHandles;

    if (!GetPowerManagedMsgQueues(rghWaits, rghReqs, cHandlesOriginal, sizeof(Buff), &cHandles)) {
		goto EXIT;
	}

    // The count may have changed. Update cHandles to the minimum.
	cHandles = min(cHandlesOriginal, cHandles); 

    if (cHandles == 0) {
        // Nothing to do.
        goto EXIT;
    }

	rghWaits[cHandles] = g_hDeviceTerminateEvent;

    while(TRUE)
    {
        DWORD dwWaitCode = WaitForMultipleObjects(cHandles + 1, rghWaits, FALSE, INFINITE);
        if(dwWaitCode >= WAIT_OBJECT_0 && dwWaitCode < (WAIT_OBJECT_0 + cHandles))
        {
			DWORD dwIdx = dwWaitCode - WAIT_OBJECT_0;
            DEBUGCHK(dwIdx < cHandles);

            if (ReadMsgQueue(rghWaits[dwIdx], Buff.byBuffer, sizeof(Buff), &dwSize, 0, &dwFlags)) {
                DEBUGMSG(ZONE_POWER, (TEXT("Received %s notification\r\n"), Buff.DevDetail.szName));
				SendMessage(hDlg, WM_USER_NOTIFICATION, 0, (LPARAM) Buff.byBuffer);
            }
			else {
				DEBUGCHK(FALSE); // We should never get here
			}
        }
        else
        {
            // got terminate signal
            break;
        }
    }

	dwRet = 0; // Success!

EXIT:
	// Clean up

	for (DWORD i = 0; i < cHandles; ++i) {
		if (rghReqs && rghReqs[i])
			StopDeviceNotifications(rghReqs[i]);
		if (rghWaits && rghWaits[i])
			CloseHandle(rghWaits[i]);
	}

    // Don't close g_hDeviceTerminateEvent (rghWaits[cHandles]). It is
    // closed in the DlgProc.

	MyFree(rghWaits);
	MyFree(rghReqs);

	DEBUGMSG(ZONE_POWER, (TEXT("DeviceNotificationThread exiting\r\n")));

    return dwRet;
}


// Is this power state valid?
static BOOL CePowerIsValid(CEDEVICE_POWER_STATE cepower)
{
	return cepower > PwrDeviceUnspecified && cepower < PwrDeviceMaximum;
}


// Sets up the list view with the given column headers.
static BOOL InitListView(HWND hwndListView, const TCHAR **rgszHeaders, int cszHeaders)
{
	DEBUGCHK(hwndListView);
	DEBUGCHK(rgszHeaders);
	DEBUGCHK(cszHeaders > 0);
	
	int iIndex;
	LVCOLUMN lvColumn;
	RECT rc;
	int cx;
	int cxTotal;

	// Exit if cszHeaders is <= 0 to avoid division by 0
	if (cszHeaders <= 0) {
		DEBUGMSG(ZONE_ERROR, (TEXT("Error: Got header count of %i\r\n"), 
			cszHeaders));
		return FALSE;
	}

	
	// Empty the list in list view.
	ListView_DeleteAllItems(hwndListView);
	
	// Initialize the columns in the list view.
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;

	GetClientRect(hwndListView, &rc);
	cxTotal = rc.right - rc.left;
	cx = cxTotal / cszHeaders;
	
	// Insert the five columns in the list view.
	for (iIndex = 0; iIndex < cszHeaders; ++iIndex)
	{
		if (iIndex == 0) {
			// Give the 0 column any leftover
			lvColumn.cx = cx + (cxTotal % cszHeaders);
		}
		else {
			lvColumn.cx = cx;
		}

		lvColumn.pszText = (TCHAR*) rgszHeaders[iIndex];
		if (ListView_InsertColumn(hwndListView, iIndex, &lvColumn) == -1) {
			DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not insert column %i"), 
				iIndex));
			return FALSE;
		}
	}
	
	return TRUE;
}


static BOOL InitDevicePowerListView(HWND hwndListView)
{
	DEBUGCHK(hwndListView);

	const TCHAR *rgszHeaders[] = { CELOADSZ(IDS_POWER_DEVICE), 
								   CELOADSZ(IDS_POWER_STATUS) };
	
	return InitListView(hwndListView, rgszHeaders, ARRAYSIZEOF(rgszHeaders));
}


// Adds the device listed in pd to the list with its details.
static BOOL AddDeviceToList(HWND hwndList, DEVDETAIL *pd)
{
	DEBUGCHK(hwndList);
	DEBUGCHK(pd);

	static const UINT rguCEPowerIDs[] = { 
		IDS_POWER_HIGH_D0, IDS_POWER_MEDIUM_D1, IDS_POWER_LOW_D2, 
		IDS_POWER_OFF_D3, IDS_POWER_OFF_D4 
	};

	DEBUGCHK(ARRAYSIZEOF(rguCEPowerIDs) == PwrDeviceMaximum);

	CEDEVICE_POWER_STATE cepower;
	const int cchFullName = MAX_PATH;
	TCHAR szFullName[cchFullName + 1];
	szFullName[cchFullName] = '\0';
	const GUID &guid = pd->guidDevClass;

	_sntprintf(szFullName, cchFullName, 
		TEXT("{%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x}\\%s"),
		guid.Data1, guid.Data2, guid.Data3, (guid.Data4[0] << 8) + guid.Data4[1], 
		guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], 
		guid.Data4[6], guid.Data4[7], 
		pd->szName);

	if (GetDevicePower(szFullName, POWER_NAME, &cepower) == ERROR_SUCCESS && CePowerIsValid(cepower))
	{
		DEBUGMSG(ZONE_POWER, (TEXT("Power state for %s is %u\n"), szFullName, cepower));

		LVITEM lvitem;
		memset(&lvitem, 0, sizeof(lvitem));
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = 0;
		lvitem.iSubItem = 0;
		lvitem.pszText = pd->szName;
		int iIndex = ListView_InsertItem(hwndList, &lvitem);
		if (iIndex == -1) {
			DEBUGMSG(ZONE_ERROR, (TEXT("Could not insert %s into list\n"), pd->szName));
			return FALSE;
		}
	
		ListView_SetItemText(hwndList, iIndex, 1, (TCHAR*) CELOADSZ(rguCEPowerIDs[cepower]));
	}
	else {
		DEBUGMSG(ZONE_POWER, (TEXT("GetDevicePower(\"%s\") was unsuccessful\n"),
			szFullName));
		return FALSE;
	}

	return TRUE;
}


// Removes the device listed in pd from the list.
static BOOL RemoveDeviceFromList(HWND hwndList, DEVDETAIL *pd)
{
	DEBUGCHK(hwndList);
	DEBUGCHK(pd);

	LVFINDINFO lvfindinfo;
	lvfindinfo.flags = LVFI_STRING;
	lvfindinfo.psz = pd->szName;

	int iIndex = ListView_FindItem(hwndList, -1, &lvfindinfo);
	if (iIndex == -1) {
		DEBUGMSG(ZONE_ERROR, (TEXT("Could not find %s in device list\r\n"), pd->szName));
		return FALSE;
	}

	BOOL fRet = ListView_DeleteItem(hwndList, iIndex);
	DEBUGCHK(fRet);

	return TRUE;
}


extern "C" BOOL APIENTRY PowerDeviceDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static HANDLE hThread = NULL;

    switch (message)
    {
	case WM_INITDIALOG:
		{
			InitDevicePowerListView(DI(IDC_LIST_POWER_DEVICE));

			g_hDeviceTerminateEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
            if (g_hDeviceTerminateEvent) {
                hThread = CreateThread(NULL, 0, DeviceNotificationThread, (LPVOID)hDlg, 0, NULL);

				if (!hThread) {
					DEBUGMSG(ZONE_ERROR, 
						(TEXT("Error: Could not start notification thread\r\n")));
				}
            }
			else {
				DEBUGMSG(ZONE_ERROR, (TEXT("Error: Unable to create event\r\n")));
			}
			
			return TRUE; // let GWE set focus
		}
		
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				if (g_hDeviceTerminateEvent) {
					// Close our thread.
					if (hThread) {
						SetEvent(g_hDeviceTerminateEvent);
						WaitForSingleObject(hThread, INFINITE);
						CloseHandle(hThread);
						hThread = NULL;
					}

					// Close our event
					CloseHandle(g_hDeviceTerminateEvent);
					g_hDeviceTerminateEvent = NULL;
				}
				else {
					DEBUGCHK(!hThread); // hThread exists only if the event is good
				}
				return TRUE;
			}

			break;
		}

	case WM_USER_NOTIFICATION:
		{
			// We got a notification. Process it.
			DEVDETAIL *pd = (DEVDETAIL*) lParam;
			HWND hwndList = DI(IDC_LIST_POWER_DEVICE);
			
			if (pd->fAttached == TRUE) {
				AddDeviceToList(hwndList, pd);
			}
			else {
				RemoveDeviceFromList(hwndList, pd);
			}
			return TRUE;
		}
	}


	return FALSE;
}


// The Power Schemes tab

/*//moved to regcpl.h
#define FULL_TIMEOUTS_KEY_SZ	PWRMGR_REG_KEY TEXT("\\Timeouts")

#define PM_AC_USER_IDLE_SZ		TEXT("ACUserIdle")
#define PM_AC_SYSTEM_IDLE_SZ	TEXT("ACSystemIdle")
#define PM_AC_SUSPEND_SZ		TEXT("ACSuspend")
#define PM_AC_OFF_SZ			TEXT("ACOff")
#define PM_DEF_AC_OFF_SZ		TEXT("DefACOff")

#define PM_BATT_USER_IDLE_SZ	TEXT("BattUserIdle")
#define PM_BATT_SYSTEM_IDLE_SZ	TEXT("BattSystemIdle")
#define PM_BATT_SUSPEND_SZ		TEXT("BattSuspend")
#define PM_BATT_OFF_SZ			TEXT("BattOff")
#define PM_DEF_BATT_OFF_SZ		TEXT("DefBattOff")
*/

enum PowerTransitionIdx {
	PM_TO_USER_IDLE_IDX = 0,
	PM_TO_SYSTEM_IDLE_IDX,
	PM_TO_SUSPEND_IDX,
	PM_TRANSITION_COUNT
};

//moved to regcpl.h #define PM_TRANSITION_NEVER_DW 0


static const DWORD g_rgdwBattTimes[] = { 5, 10, 15, 30, 1*60, 2*60, 3*60, 4*60, PM_MAX_BATT_TIMEOUT, PM_TRANSITION_NEVER_DW };
static const DWORD g_rgdwACTimes[]   = { 30, 1*60, 2*60, 5*60, 10*60, 15*60, 30*60, PM_TRANSITION_NEVER_DW };

static const UINT g_rguTimeoutComboIDs[PM_TRANSITION_COUNT] = { IDC_COMBO_TO_USER_IDLE, IDC_COMBO_TO_SYSTEM_IDLE, IDC_COMBO_TO_SUSPEND };

struct PowerSourceData {
	UINT uNameID; // ID of the string name of this power scheme
	UINT uIconID; // ID of the icon to display for this power scheme
	const TCHAR *rgszValues[PM_TRANSITION_COUNT]; // Registry value names
	const DWORD *rgdwTimes; // Timeout options
	int cTimes; // Count of timeout options
	int rgiDefaultTimeIdx[PM_TRANSITION_COUNT]; // Default timeout indices
};

static const PowerSourceData g_psdBattery = {
	IDS_POWER_BATTERY,
	IDI_POWER_BAT,
	{ PM_BATT_USER_IDLE_SZ, PM_BATT_SYSTEM_IDLE_SZ, PM_BATT_SUSPEND_SZ },
	g_rgdwBattTimes,
	ARRAYSIZEOF(g_rgdwBattTimes),
	{ 2, 3, 4 } // These are indices
};

static const PowerSourceData g_psdAC = {
	IDS_POWER_AC,
	IDI_POWER_AC,
	{ PM_AC_USER_IDLE_SZ, PM_AC_SYSTEM_IDLE_SZ, PM_AC_SUSPEND_SZ }, 
	g_rgdwACTimes,
	ARRAYSIZEOF(g_rgdwACTimes),
	{ 5, 5, 6 } // These are indices
};

static const PowerSourceData *g_rgppsd[] = { &g_psdBattery, &g_psdAC };
#define POWER_SOURCE_COUNT	ARRAYSIZEOF(g_rgppsd)

BOOL GetPowerOff(DWORD& Off)
{
	DWORD BtTimeout = 0, AcTimeout = 0;
	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}
	BtTimeout = reg.ValueDW(PM_BATT_OFF_SZ, 0);
	AcTimeout = reg.ValueDW(PM_AC_OFF_SZ, 0);

	if( BtTimeout || AcTimeout )
		Off = 1;
	else
		Off = 0;

	return TRUE;
}
BOOL SetPowerOff(DWORD Off)
{
	DWORD	BtTimeout	= 0; 
	DWORD	AcTimeout	= 0;

	CReg reg(HKEY_LOCAL_MACHINE, FULL_TIMEOUTS_KEY_SZ);

	if (!reg.IsOK()) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Could not open power key %s\r\n"), FULL_TIMEOUTS_KEY_SZ));
		return FALSE;
	}

	if( Off )
	{
		BtTimeout = reg.ValueDW(PM_DEF_BATT_OFF_SZ, 3600);
		AcTimeout = reg.ValueDW(PM_DEF_AC_OFF_SZ, 3600);
	}
	if(!reg.SetDW(PM_BATT_OFF_SZ, BtTimeout)) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to set value %s\r\n"), PM_BATT_OFF_SZ));
		return FALSE;
	}
	if(!reg.SetDW(PM_AC_OFF_SZ, AcTimeout)) 
	{
		DEBUGMSG(ZONE_ERROR, (TEXT("Unable to set value %s\r\n"), PM_AC_OFF_SZ));
		return FALSE;
	}
	
	return 1;
}

// Retrieves the registry values in rgszValues from the key szKey and 
// places them in the rgdwTimeouts array which must be at least of size 
// PM_TRANSITION_COUNT. Returns FALSE on any error.
static BOOL GetRegTimeoutValues(const TCHAR *szKey, const TCHAR * const rgszValues[PM_TRANSITION_COUNT], DWORD *rgdwTimeouts)
{
	DEBUGCHK(szKey);
	DEBUGCHK(rgszValues);
	DEBUGCHK(rgdwTimeouts);

	static const DWORD dwNoValue = 0xbeef;

	CReg reg(HKEY_LOCAL_MACHINE, szKey);

	if (!reg.IsOK()) {
		DEBUGMSG(ZONE_POWER, (TEXT("Could not open power key %s\r\n"), szKey));
		return FALSE;
	}

	for (int idx = 0; idx < PM_TRANSITION_COUNT; ++idx)
	{
		rgdwTimeouts[idx] = reg.ValueDW(rgszValues[idx], dwNoValue);

		if (rgdwTimeouts[idx] == dwNoValue) {
			DEBUGMSG(ZONE_POWER, (TEXT("Unable to get value %s\\%s\r\n"), 
				szKey, rgszValues[idx]));
			return FALSE;
		}
	}

	return TRUE;
}


// Sets the registry values in rgszValues from the key szKey to the values in 
// the rgdwTimeouts array which must be at least of size PM_TRANSITION_COUNT.
// Returns FALSE on any error.
static BOOL SetRegTimeoutValues(const TCHAR *szKey, const TCHAR * const rgszValues[PM_TRANSITION_COUNT], const DWORD *rgdwTimeouts)
{
	DEBUGCHK(szKey);
	DEBUGCHK(rgszValues);
	DEBUGCHK(rgdwTimeouts);

	CReg reg(HKEY_LOCAL_MACHINE, szKey);

	if (!reg.IsOK()) {
		DEBUGMSG(ZONE_POWER, (TEXT("Could not open power key %s\r\n"), szKey));
		return FALSE;
	}

	for (int idx = 0; idx < PM_TRANSITION_COUNT; ++idx)
	{
		if (!reg.SetDW(rgszValues[idx], rgdwTimeouts[idx])) {
			DEBUGMSG(ZONE_POWER, (TEXT("Unable to set value %s\\%s\r\n"), 
				szKey, rgszValues[idx]));
			return FALSE;
		}
	}

	return TRUE;
}


// Retrieves the selected values from each combo box.
static BOOL GetSelectedTimeoutValues(HWND hDlg, const UINT rguComboIDs[PM_TRANSITION_COUNT], DWORD *rgdwTimeouts, const DWORD *rgdwTimes, int cTimes)
{
	DEBUGCHK(hDlg);		
	DEBUGCHK(rguComboIDs);
	DEBUGCHK(rgdwTimeouts);
	DEBUGCHK(rgdwTimes);
	DEBUGCHK(cTimes > 0);

	for (int idx = 0; idx < PM_TRANSITION_COUNT; ++idx)
	{
		HWND hwndCB = DI(rguComboIDs[idx]);
		int iSel = ComboBox_GetCurSel(hwndCB);

		DEBUGCHK(iSel >= 0);
		DEBUGCHK(iSel < cTimes);

		rgdwTimeouts[idx] = rgdwTimes[iSel];
	}

	return TRUE;
}


// Sets up the combo boxes with the possible timeout choices.
static BOOL UpdateListBoxes(HWND hDlg, const UINT rguComboIDs[PM_TRANSITION_COUNT], const DWORD *rgdwTimes, int cTimes)
{
	DEBUGCHK(hDlg);
	DEBUGCHK(rguComboIDs);
	DEBUGCHK(rgdwTimes);
	DEBUGCHK(cTimes > 0);

	const int iBufSize = 60;
	TCHAR szBuf[iBufSize + 1];
	szBuf[iBufSize] = '\0';

	for (int iTransition = 0; iTransition < PM_TRANSITION_COUNT; ++iTransition)
	{
		HWND hwndCB = DI(rguComboIDs[iTransition]);
		DEBUGCHK(hwndCB);

		ComboBox_ResetContent(hwndCB);

		for (int iTime = 0; iTime < cTimes; ++iTime)
		{
			DWORD dwTime = rgdwTimes[iTime];
			
			if (dwTime == PM_TRANSITION_NEVER_DW) {
				_sntprintf(szBuf, iBufSize, TEXT("%s"), CELOADSZ(IDS_POWER_NEVER));
			}
			else if( 60 > dwTime )
			{
				//_sntprintf(szBuf, iBufSize, TEXT("%s%u %s"), CELOADSZ(IDS_POWER_AFTER), dwTime, CELOADSZ(IDS_POWER_SECONDS));
				const TCHAR*	ptr = 0;
				const TCHAR *szSec_s = CELOADSZ(IDS_15SECS);// string "15 seconds"
				if( szSec_s )
					ptr = _tcschr( szSec_s, _T('5') );
				
				if( ptr )
				{
					if( *(ptr + 1) != _T('1') )//if it's not previous char
						ptr++;
					else
						ptr--;
				}
				else
					ptr = STR_SECONDS;

				_sntprintf(szBuf, iBufSize, TEXT("%s%u%s"), CELOADSZ(IDS_POWER_AFTER), dwTime, ptr);
			}
			else { 
				dwTime /= 60;
				const TCHAR *szMinute_s = dwTime == 1 ? CELOADSZ(IDS_POWER_MINUTE) : CELOADSZ(IDS_POWER_MINUTES);
				_sntprintf(szBuf, iBufSize, TEXT("%s%u %s"), CELOADSZ(IDS_POWER_AFTER), dwTime, szMinute_s);
			}
			
			int iErr = ComboBox_AddString(hwndCB, szBuf);
			if (iErr == CB_ERR || iErr == CB_ERRSPACE) {
				DEBUGMSG(ZONE_ERROR, (TEXT("Could not add %s to combo box\r\n"),
					szBuf));
				return FALSE;
			}
		}
	}

	return TRUE;
}


// Compares dwCurrTime against the possible values and returns its
// index in the array, or iDefault if it was not found.
static int FindCurrentIndex(DWORD dwCurrTime, const DWORD *rgdwTimes, int cTimes, int iDefault)
{
	DEBUGCHK(rgdwTimes);
	DEBUGCHK(cTimes > 0);
	DEBUGCHK(iDefault >= 0 && iDefault < cTimes);

	int iRet = iDefault;

	for (int i = 0; i < cTimes; ++i)
	{
		if(dwCurrTime == rgdwTimes[i]) {
			iRet = i;
			break;
		}
	}
	
	return iRet;
}


// Sets the list box selections to the values in rgdwCurrTimeouts.
static BOOL	SetListBoxSelection(HWND hDlg, const UINT rguComboIDs[PM_TRANSITION_COUNT], const int rgiDefaultIdx[PM_TRANSITION_COUNT], const DWORD *rgdwCurrTimeouts, const DWORD *rgdwTimes, int cTimes)
{
	DEBUGCHK(hDlg);
	DEBUGCHK(rguComboIDs);
	DEBUGCHK(rgiDefaultIdx);
	DEBUGCHK(rgdwCurrTimeouts);
	DEBUGCHK(rgdwTimes);
	DEBUGCHK(cTimes > 0);

	for (int i = 0; i < PM_TRANSITION_COUNT; ++i) 
	{
		int idx = FindCurrentIndex(rgdwCurrTimeouts[i], rgdwTimes, cTimes, rgiDefaultIdx[i]);
		DEBUGCHK(idx >= 0 && idx < cTimes);

		int iErr = ComboBox_SetCurSel(DI(rguComboIDs[i]), idx);
		DEBUGCHK(iErr != CB_ERR);
	}

	return TRUE;
}


// See if the new power manager registry settings exist.
static BOOL NewRegValuesExist()
{
	DWORD rgTemp[PM_TRANSITION_COUNT];

	for (int i = 0; i < POWER_SOURCE_COUNT; ++i) {
		if (!GetRegTimeoutValues(FULL_TIMEOUTS_KEY_SZ, g_rgppsd[i]->rgszValues, rgTemp)) {
			return FALSE;
		}
	}

	return TRUE;
}


static BOOL UpdateSheet(HWND hDlg, const PowerSourceData *ppsd, const DWORD rgdwCurrTimes[PM_TRANSITION_COUNT])
{
	DEBUGCHK(ppsd);
	DEBUGCHK(rgdwCurrTimes);

	HICON hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(ppsd->uIconID), IMAGE_ICON, 0, 0, 0);
	if (!hIcon) {
		DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not load icon\r\n")));
	}
	else {
		if (!SendMessage(DI(IDC_STATIC1), STM_SETIMAGE, IMAGE_ICON, (LONG) hIcon)) {
			DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not update icon\r\n")));
		}
		DestroyIcon(hIcon);
	}

	if (!UpdateListBoxes(hDlg, g_rguTimeoutComboIDs, ppsd->rgdwTimes, ppsd->cTimes)) {
		DEBUGMSG(ZONE_ERROR, (TEXT("Error: Problem setting up combo boxes!\r\n")));
		return FALSE;
	}

	if (!SetListBoxSelection(hDlg, g_rguTimeoutComboIDs, ppsd->rgiDefaultTimeIdx, 
			rgdwCurrTimes, ppsd->rgdwTimes, ppsd->cTimes)) {
		DEBUGMSG(ZONE_ERROR, (TEXT("Error: Problem setting combo box selections!\r\n")));
		return FALSE;
	}

	return TRUE;
}


extern "C" BOOL APIENTRY PowerTimeoutsDlgProc (HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static DWORD rgdwCurrTimes[POWER_SOURCE_COUNT][PM_TRANSITION_COUNT];

	static UINT uCurIdx = 0; // Default to first (battery)

	static	DWORD OldOff = 0;
	int i;

	DEBUGCHK(uCurIdx < ARRAYSIZEOF(g_rgppsd));
	DEBUGCHK(g_rgppsd[uCurIdx]);
	DEBUGCHK(NewRegValuesExist());

    switch (message)
    {
		case WM_INITDIALOG:
		{
			DEBUGCHK(ARRAYSIZEOF(g_rguTimeoutComboIDs) == PM_TRANSITION_COUNT);

			// Set up the power schemes combo box
			HWND hwndCBPowerSource = DI(IDC_COMBO_POWER_SOURCE);
			DEBUGCHK(hwndCBPowerSource);

			for (i = 0; i < POWER_SOURCE_COUNT; ++i) {
				LPCTSTR szPowerSource = CELOADSZ(g_rgppsd[i]->uNameID);
				DEBUGCHK(szPowerSource);
				int iErr = ComboBox_AddString(hwndCBPowerSource, szPowerSource);
				if (iErr == CB_ERR || iErr == CB_ERRSPACE) {
					DEBUGMSG(ZONE_ERROR, (TEXT("Could not add %s to combo box\r\n"),
						szPowerSource));
				}
			}

			// See which power scheme to start with
			SYSTEM_POWER_STATUS_EX spse;
			if (GetSystemPowerStatusEx(&spse, TRUE)) {
				if (spse.ACLineStatus == AC_LINE_ONLINE) {
					// Start with the AC scheme since we are on AC power
					uCurIdx = 1;
				}
			}

			// Set the current power scheme
			DEBUGCHK(uCurIdx < ComboBox_GetCount(hwndCBPowerSource));
			if (ComboBox_SetCurSel(hwndCBPowerSource, uCurIdx) == CB_ERR) {
				DEBUGMSG(ZONE_ERROR, (TEXT("Problem setting current list selection\r\n")));
			}

			// Get all the timeout values from the registry
			for (i = 0; i < POWER_SOURCE_COUNT; ++i) {
				if (!GetRegTimeoutValues(FULL_TIMEOUTS_KEY_SZ, g_rgppsd[i]->rgszValues, rgdwCurrTimes[i])) {
					DEBUGMSG(ZONE_ERROR, (TEXT("Error: Problem reading registry values for %s!\r\n"),
						CELOADSZ(g_rgppsd[i]->uNameID)));
				}
			}

			UpdateSheet(hDlg, g_rgppsd[uCurIdx], rgdwCurrTimes[uCurIdx]);
			
			///update power off checkbox
			DWORD OffFlag = 0;
			GetPowerOff(OffFlag);
			OldOff = OffFlag;
			Button_SetCheck(DI(IDC_TURNOFF), OffFlag);
			///

			return TRUE; // let GWE set focus
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_COMBO_POWER_SOURCE:
				{
					switch(HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						{
							// Save the on-screen timeouts
							if (!GetSelectedTimeoutValues(hDlg, g_rguTimeoutComboIDs, rgdwCurrTimes[uCurIdx],
									g_rgppsd[uCurIdx]->rgdwTimes, g_rgppsd[uCurIdx]->cTimes)) {
								DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not get selected timeout values!\r\n")));
							}

							// Update our selection
							int iCurSel = ComboBox_GetCurSel((HWND) lParam);
							if (iCurSel == CB_ERR) {
								DEBUGCHK(FALSE); // Why wasn't anything selected?
							}
							else {
								DEBUGCHK(iCurSel < POWER_SOURCE_COUNT);
								uCurIdx = iCurSel;
							}

							UpdateSheet(hDlg, g_rgppsd[uCurIdx], rgdwCurrTimes[uCurIdx]);

							return TRUE;
						}

					}

					break;
				}

				case IDOK:
				{
					BOOL ResetFlag = 0;
					// Get the timeouts currently displayed
					if (!GetSelectedTimeoutValues(hDlg, g_rguTimeoutComboIDs, rgdwCurrTimes[uCurIdx], 
							g_rgppsd[uCurIdx]->rgdwTimes, g_rgppsd[uCurIdx]->cTimes)) {
						DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not get selected timeout values!\r\n")));
					}

					// Write all the timeout values to the registry
					for (i = 0; i < POWER_SOURCE_COUNT; ++i) {
						if (!SetRegTimeoutValues(FULL_TIMEOUTS_KEY_SZ, g_rgppsd[i]->rgszValues, rgdwCurrTimes[i])) {
							DEBUGMSG(ZONE_ERROR, (TEXT("Error: Problem reading registry values for %s!\r\n"), 
								CELOADSZ(g_rgppsd[i]->uNameID)));
						}
					}
					
					//check incompatibility with ignition reg
					if(	0 == rgdwCurrTimes[0][0]	|| ///batt timeout == never
						0 == rgdwCurrTimes[0][1]	||
						0 == rgdwCurrTimes[0][2]	||
						0 != rgdwCurrTimes[1][2]	)//ac suspend timeout != never
					{
						if( GetIgniFromReg() )//ignition enabled, but can't work
						{
							if(IDYES != MessageBox(hDlg, DISABLE_IGNI_WARN, _T("Schemes"), MB_ICONWARNING | MB_YESNO))
								break;
							SetIgniToReg(0);
							if(IDYES == MessageBox(hDlg, RESTART_STR, _T("Schemes"), MB_ICONWARNING | MB_YESNO))
							{
								ResetFlag = 1;
							}
						}
					}
					//power off
					DWORD off = Button_GetCheck(DI(IDC_TURNOFF));
					if( OldOff != off )//don't update shutdown timeouts if the flag is not changed
						SetPowerOff(off);	

					//Synchronize backlight timeouts//////////////////////////////////
					DWORD bEnableBatTime, iBatteryTime, bEnableExtTime, iExternalTime;
					GetBlReg( bEnableBatTime, iBatteryTime, bEnableExtTime, iExternalTime );

					if( PM_TRANSITION_NEVER_DW == rgdwCurrTimes[0][0] )
					{
						bEnableBatTime = PM_TRANSITION_NEVER_DW;
					}
					else
					{
						if(bEnableBatTime)
							iBatteryTime = rgdwCurrTimes[0][0];
					}

					if( PM_TRANSITION_NEVER_DW == rgdwCurrTimes[1][0] )
					{
						bEnableExtTime = PM_TRANSITION_NEVER_DW;
					}
					else
					{
						if(bEnableExtTime)
							iExternalTime = rgdwCurrTimes[1][0];
					}
					SetBlReg( bEnableBatTime, iBatteryTime, bEnableExtTime, iExternalTime );
					NotifyBlight();
					///////////////////////////////////////////////////////////////////////////
					// Tell power manager to reload the registry timeouts
					HANDLE hevReloadActivityTimeouts = 
						OpenEvent(EVENT_ALL_ACCESS, FALSE, PM_EVENT_NAME);
					if (hevReloadActivityTimeouts) {
						SetEvent(hevReloadActivityTimeouts);
						CloseHandle(hevReloadActivityTimeouts);
					}
					else {
						DEBUGMSG(ZONE_ERROR, (TEXT("Error: Could not open PM reload event\r\n")));
					}

					if(ResetFlag)
						SetSystemPowerState(0, POWER_STATE_RESET, POWER_FORCE);
					else
						RegFlushKey(HKEY_LOCAL_MACHINE);
				}
				// fall through
				case IDCANCEL:
					return TRUE;
			}		
			break;
		}
	}

	return FALSE;
}


// If the new power management registry keys exist, use the new power applet.
extern "C" BOOL APIENTRY PowerCallback(CPLAPPLETINFO *cplAppletInfo)
{
	DEBUGCHK(cplAppletInfo);

	// cplAppletInfo->rgptab[0] is always &BatteryTab

	if (NewRegValuesExist()) {
		DEBUGMSG(ZONE_POWER, (TEXT("Using new power applet\r\n")));
		cplAppletInfo->rgptab[1] = &TimeoutsTab;
		cplAppletInfo->rgptab[2] = &PowerDeviceTab;
	}
	else {
		DEBUGMSG(ZONE_POWER, (TEXT("Using old power applet\r\n")));
		cplAppletInfo->rgptab[1] = &SleepTab;
	}

	return TRUE;
}
////////////////////

void GetBlReg( DWORD& bEnableBatTime, DWORD& iBatteryTime, DWORD& bEnableExtTime, DWORD& iExternalTime )
{
	CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_BACKLIGHT);

	bEnableBatTime = reg.ValueDW(RV_USEBATTERY, TRUE);
	bEnableExtTime = reg.ValueDW(RV_USEAC, TRUE);

	if (bEnableBatTime)
	{
		iBatteryTime = reg.ValueDW(RV_BATTERYTIMEOUT, DEF_BATTERYTIMEOUT);
	}
	else 	
	{
		iBatteryTime = reg.ValueDW(RV_OLD_BATTERYTIMEOUT, DEF_BATTERYTIMEOUT);
	}

	if (bEnableExtTime)
	{
		iExternalTime = reg.ValueDW(RV_ACTIMEOUT, DEF_EXTTIMEOUT);
	}
	else
	{
		iExternalTime = reg.ValueDW(RV_OLD_ACTIMEOUT, DEF_EXTTIMEOUT);
	}
}
void SetBlReg( DWORD bEnableBatTime, DWORD iBatteryTime, DWORD bEnableExtTime, DWORD iExternalTime )
{
	CReg reg(HKEY_CURRENT_USER, RK_CONTROLPANEL_BACKLIGHT);
	
	reg.SetDW(RV_USEBATTERY, bEnableBatTime);
	if(bEnableBatTime)
	{
		reg.SetDW(RV_BATTERYTIMEOUT, iBatteryTime);
	}    
	else
	{
		reg.DeleteValue(RV_BATTERYTIMEOUT);
		reg.SetDW(RV_OLD_BATTERYTIMEOUT, iBatteryTime);
	}
			      
	reg.SetDW(RV_USEAC, bEnableExtTime);
	if(bEnableExtTime)
    {
		reg.SetDW(RV_ACTIMEOUT, iExternalTime);
	}
	else
	{
		reg.DeleteValue(RV_ACTIMEOUT);
		reg.SetDW(RV_OLD_ACTIMEOUT, iExternalTime);
	}
}
//////////////////////////////////////
BOOL	NotifyBlight()
{
	// signal the driver to update
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTNAME_BACKLIGHTCHANGEEVENT);
	if(!hEvent)
		return FALSE;

	SetEvent(hEvent);
	CloseHandle(hEvent);

	return TRUE;
}
////////////////////////////
DWORD GetIgniFromReg()
{
	DWORD fEnable = 0;
	CReg reg;
	if( reg.Open( HKEY_LOCAL_MACHINE, RK_FULL_PSC ) )
		fEnable	= reg.ValueDW(RK_IGN_ENABLE, 0	);

	return fEnable;
}
BOOL SetIgniToReg( DWORD fEnable )
{
	CReg reg;
	if( reg.Open( HKEY_LOCAL_MACHINE, RK_FULL_PSC ) )
	{
		reg.SetDW(RK_IGN_ENABLE, fEnable	);
		return TRUE;
	}
	return FALSE;
}	