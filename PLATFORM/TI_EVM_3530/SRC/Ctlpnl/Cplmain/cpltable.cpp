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

Abstract: This is the const data that the whole CPL is driven of
**/

const CPLTABINFO CommRasTab = { 
	IDS_COMM_CONNECTTAB, IDD_RAS, L"CommRASDlgProc",0,0,0,L"file:ctpnl.htm#adjust_pc_connection_settings",
};

const CPLTABINFO DialingTab = { 
	0, 0, L"ShowLineTranslateDlg",0,0,0,0, 
};

const CPLTABINFO KeybdTab = { 
	IDS_KEYBD_TAB, IDD_KEYBD, L"KeybdDlgProc",0,0,0,L"file:ctpnl.htm#adjusting_keyboard_settings",
};

const CPLTABINFO PasswdTab = { 
	IDS_PASSWD_TAB, IDD_PASSWD, L"PasswdDlgProc", 0, 0, 1, L"file:ctpnl.htm#set_or_change_the_password",
};

const CPLTABINFO OwnerTab = { 
    IDS_PROFILE_IDTAB, IDD_PROFILE, L"OwnerDlgProc", 0, 0, 1, L"file:ctpnl.htm#add_or_change_owner_identification",
};

const CPLTABINFO NotesTab = { 
    IDS_PROFILE_NOTESTAB, IDD_NOTES, L"NotesDlgProc", 0, 0, 1, L"file:ctpnl.htm#add_or_change_owner_identification",
};

const CPLTABINFO NetIdentTab = { 
	IDS_PROFILE_NETIDENT, IDD_NETIDENT, L"NetIdentDlgProc",0,0,1,L"file:ctpnl.htm#network_identification",
};

// Power
const int rgBatteryBold[] = { IDC_STATIC_BACK, IDC_STATIC_MAIN, IDC_STATIC_CHARGE, IDC_STATIC_EXTERNAL,
						IDC_STATIC_PWR, IDC_STATIC_MBS };

const CPLTABINFO BatteryTab = { 
	IDS_POWER_BATTERYTAB, IDD_BATTERY, L"BatteryDlgProc", rgBatteryBold, ARRAYSIZEOF(rgBatteryBold), 0, L"file:ctpnl.htm#battery",
};

const int rgSystemBold[] = { IDC_STATIC_COMPUTER, IDC_STATIC_SYSTEM };

const CPLTABINFO SystemTab = { 
	IDS_SYSTEM_SYSTAB, IDD_SYSTEM, L"SystemDlgProc", rgSystemBold, ARRAYSIZEOF(rgSystemBold), 0, L"file:ctpnl.htm#adjusting_memory_and_power_settings",
};

const CPLTABINFO SystemIdentTab = { 
	IDS_SYSTEM_IDTAB, IDD_COMPUTER, L"SystemIdentDlgProc",0,0,1,L"file:ctpnl.htm#change_the_device_name",
};

const int rgMemoryBold[] = { IDC_STATIC_ALLOC_LEFT, IDC_STATIC_ALLOC_RIGHT, 
					 IDC_STATIC_IN_USE_LEFT,IDC_STATIC_IN_USE_RIGHT,
					 IDC_STATIC_STORAGE, IDC_STATIC_PROGRAM, 
					 IDC_STATIC_MEMORY1, IDC_STATIC_MEMORY2 };

const CPLTABINFO MemoryTab = { 
	IDS_SYSTEM_MEMTAB, IDD_MEMORY, L"MemoryDlgProc", rgMemoryBold, ARRAYSIZEOF(rgMemoryBold), 0, L"file:ctpnl.htm#adjust_the_memory_to_power_ratio",
};

// Copyrights tab
const CPLTABINFO CopyrTab = {
    IDS_SYSTEM_COPYRTAB, IDD_COPYRIGHTS, L"CopyrightsDlgProc",0,0,0,L"file:ctpnl.htm#adjusting_memory_and_power_settings",	//System help section
};
//

const CPLTABINFO BackgndTab = { 
	IDS_SCREEN_BACKGNDTAB, IDD_BACKGROUND, L"BackgroundDlgProc", 0, 0, 0, L"file:ctpnl.htm#Changing_the_Display",
};

const CPLTABINFO ColSchemeTab = {
	IDS_SCREEN_COLSCHEMETAB, IDD_COLSCHEME, L"ColSchemeDlgProc", 0, 0, 0, L"file:ctpnl.htm#appearance",
};
const LPCTSTR pszSaveColSchemeHelp = L"file:ctpnl.htm#customcolor";

const CPLTABINFO BcklitTab = { 
	IDS_SCREEN_BACKLIGHTTAB, IDD_BACKLIGHT, L"BacklightDlgProc", 0, 0, 0, L"file:ctpnl.htm#display_backlight_settings",
};

const CPLTABINFO DblClkTab = { 
	IDS_MOUSE_DBLCLICKTAB, IDD_DBLTAP, L"DblClickDlgProc", 0, 0, 0, L"file:ctpnl.htm#mousesettings",
};

const CPLTABINFO DblTapTab = { 
	IDS_STYLUS_DBLTAPTAB, IDD_DBLTAP, L"DblTapDlgProc", 0, 0, 0, L"file:ctpnl.htm#adjust_the_stylus_double_tap_rate",
};

const CPLTABINFO CalibratTab = { 
	IDS_STYLUS_CALIBTAB, IDD_CALIBRATE, L"CalibrateDlgProc", 0, 0, 0, L"file:ctpnl.htm#recalibrate_the_stylus",
};

const int rgVolumeBold[] = { IDC_STATIC1, IDC_ENABLECLICKTAP };

const CPLTABINFO VolumeTab = { 
	IDS_VOLUMETAB, IDD_VOLUME, L"VolumeDlgProc", rgVolumeBold, ARRAYSIZEOF(rgVolumeBold), 0, L"file:ctpnl.htm#adjust_volume_and_turn_sounds_on_or_off",
};

const int rgSndSchemeBold[] = { IDC_STATIC_EVENTNAME, IDC_STATIC_EVENTSOUND };

const CPLTABINFO SndSchemeTab = { 
	IDS_SNDSCHEMETAB, IDD_SNDSCHEME, L"SndSchemeDlgProc", rgSndSchemeBold, ARRAYSIZEOF(rgSndSchemeBold), 0, L"file:ctpnl.htm#change_event_sounds",
};

const CPLTABINFO SipTab = { 
	IDS_SIP_TABTITLE, IDD_INPUTMETHOD, L"SipDlgProc", 0, 0, 0, L"file:ctpnl.htm#sip_info",
};

const CPLTABINFO RemoveTab = { 
	IDS_REMOVE_TITLE, IDD_REMOVE, L"RemoveDlgProc", 0, 0, 0, L"file:ctpnl.htm#removing_programs",
};


// Date/Time help topics
const CPLTABINFO DateTimeTab = {
    IDS_DATETIME_TAB, IDD_DATETIME, L"DateTimeDlgProc",0,0,0,L"file:ctpnl.htm#setting_the_date_and_time",
};

// Root Certificate Store
const CPLTABINFO CertRootsTab = {
	IDS_CERTSTORES_TAB,IDD_CERTROOTS_TAB, L"CertRootDlgProc",0,0,0,L"file:ctpnl.htm#managing_certificate_stores",
};

// Accessibility
const CPLTABINFO AccKeybdTab = {
    IDS_ACC_KEYTAB, IDD_ACC_KEYBD, L"AccessKeybdDlgProc",0,0,0,L"file:ctpnl.htm#stickyKeys_settings",
};

const CPLTABINFO AccSoundTab = {
    IDS_ACC_SOUNDTAB, IDD_ACC_SOUND, L"AccessSoundDlgProc",0,0,0,L"file:ctpnl.htm#soundsentry_settings",
};

const CPLTABINFO AccDispTab = {
    IDS_ACC_DISPTAB, IDD_ACC_DISP, L"AccessDisplayDlgProc",0,0,0,L"file:ctpnl.htm#highcontrast_settings",
};

const CPLTABINFO AccMouseTab = {
    IDS_ACC_MOUSETAB, IDD_ACC_MOUSE, L"AccessMouseDlgProc",0,0,0,L"file:ctpnl.htm#mousekeys_settings",
};

const CPLTABINFO AccGeneralTab = {
    IDS_ACC_GENTAB, IDD_ACC_GEN, L"AccessGeneralDlgProc",0,0,0,L"file:ctpnl.htm#accessibility_settings",
};

// Extern callback functions
BOOL PowerCallback(CPLAPPLETINFO*);

CPLAPPLETINFO rgApplets[] = {
// mutex-name, pwd-protect, icon, name, description, commctrl flags CPLTABINFO pointers
{L"CPL_Comm",    NULL, 0, IDI_COMM,   IDS_COMM_NAME,   IDS_COMM_DESC,   IDS_COMM_TITLE,0,   &CommRasTab, 0, 0, 0, 0 },
{L"CPL_Dialing", NULL, 0, IDI_DIALING,IDS_DIALING_NAME,IDS_DIALING_DESC,IDS_DIALING_TITLE,0,&DialingTab,0, 0, 0, 0 },
{L"CPL_Keyboard",NULL, 0, IDI_KEYBD,  IDS_KEYBD_NAME,  IDS_KEYBD_DESC,  IDS_KEYBD_TITLE,0,  &KeybdTab,	 0, 0, 0, 0 }, 
{L"CPL_Password",NULL, 1, IDI_PASSWD, IDS_PASSWD_NAME, IDS_PASSWD_DESC, IDS_PASSWD_TITLE,0, &PasswdTab, 0, 0, 0, 0 },	
{L"CPL_Owner",   NULL, 1, IDI_PROFILE,IDS_PROFILE_NAME,IDS_PROFILE_DESC,IDS_PROFILE_TITLE,0,&OwnerTab,  &NotesTab, &NetIdentTab, 0, 0 },	
{L"CPL_Power",   _T("PowerCallback"), 0, IDI_POWER,  IDS_POWER_NAME,  IDS_POWER_DESC,  IDS_POWER_TITLE, ICC_LISTVIEW_CLASSES,  &BatteryTab, 0, 0, 0, 0 },
{L"CPL_System",  NULL, 0, IDI_SYSTEM, IDS_SYSTEM_NAME, IDS_SYSTEM_DESC, IDS_SYSTEM_TITLE,0, &SystemTab, &MemoryTab, &SystemIdentTab, &CopyrTab, 0 }, 
{L"CPL_Screen",	 NULL, 0, IDI_SCREEN, IDS_SCREEN_NAME, IDS_SCREEN_DESC, IDS_SCREEN_TITLE,0, &BackgndTab,&ColSchemeTab, &BcklitTab, 0, 0 }, 
{L"CPL_Mouse",   NULL, 0, IDI_MOUSE,  IDS_MOUSE_NAME,  IDS_MOUSE_DESC,  IDS_MOUSE_TITLE,0,  &DblClkTab, 0, 0, 0, 0 },
{L"CPL_Stylus",  NULL, 0, IDI_STYLUS, IDS_STYLUS_NAME, IDS_STYLUS_DESC, IDS_STYLUS_TITLE,0, &DblTapTab, &CalibratTab, 0, 0, 0 },
{L"CPL_Sounds",  NULL, 0, IDI_SOUNDS, IDS_SOUND_NAME,  IDS_SOUND_DESC,  IDS_SOUND_TITLE,0,  &VolumeTab, &SndSchemeTab, 0, 0, 0 },
{L"CPL_SIP",     NULL, 0, IDI_SIP,    IDS_SIP_NAME,    IDS_SIP_DESC,    IDS_SIP_TITLE,0,    &SipTab, 0, 0, 0, 0 },
{L"CPL_Remove",  NULL, 0, IDI_REMOVE, IDS_REMOVE_NAME, IDS_REMOVE_DESC, IDS_REMOVE_TITLE,0, &RemoveTab, 0, 0, 0, 0 },
{L"CPL_DateTime",NULL, 0, IDI_TIME,   IDS_DATETIME_NAME, IDS_DATETIME_DESC, IDS_DATETIME_TITLE,ICC_DATE_CLASSES, &DateTimeTab, 0, 0, 0, 0 },
{L"CPL_Certs",	 NULL, 0, IDI_CERT,   IDS_CERTCPL_NAME,IDS_CERTCPL_DESC,IDS_CERTCPL_TITLE,0,&CertRootsTab, 0, 0, 0, 0 },
{L"CPL_Accessib",NULL, 0, IDI_ACCESS, IDS_ACCESS_NAME, IDS_ACCESS_DESC, IDS_ACCESS_TITLE,0, &AccKeybdTab, &AccSoundTab, &AccDispTab, &AccMouseTab, &AccGeneralTab},
};
 
