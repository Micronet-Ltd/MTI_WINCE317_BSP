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

Abstract: Defns of ALL registry keys accessed by CPLs
          except Owner CPL's regkeys are defined in oak\inc\OWNER.H
**/

//
// Common to several CPLs
//
// in HKLM
#define RK_CONTROLPANEL         TEXT("ControlPanel")
#define RV_DEVICENAME           TEXT("DeviceName")
#define RV_NODRAG               TEXT("NoDrag")     // make CPL windows not be draggable
#define RV_FULLSCREEN           TEXT("FullScreen") // to make the CPLs always full-screen. (Note this only looks good if your screen is approx the same size as the dlgs. E.g. PsPCs).
#define RV_RECENTERFORSIP       TEXT("RecenterForSIP") // to make CPLs re-center when SIP goes up. (Mutually exclusive with FullScreen)
#define RV_RAISELOWERSIP        TEXT("RaiseLowerSIP") // to make CPLs that need text input raise SIP on entry & restore on exit
#define RK_SYSTEM_GWE_BUTTON    TEXT("SYSTEM\\GWE\\Button")
#define RV_FONTWEIGHT           TEXT("FontWeight")
#define RV_INPUTCONFIG          TEXT("InputConfig")// Bit0=have keybd, Bit1=have touch screen Bit2==have HW buttons
//#define RV_UNLOADCLOSEAPPS    TEXT("UnloadCloseApps")

//
// System CPL
//
// in HKLM
#define RK_IDENT                TEXT("Ident")
#define RV_NAME                 TEXT("Name")
#define RV_DESC                 TEXT("Desc")
// in HKLM
#define RK_DRIVERS_PCMCIA       TEXT("Drivers\\PCMCIA")
#define RV_FRIENDLYNAME         TEXT("FriendlyName")


//
// COMM CPL
//
// in HKCU
#define RK_CONTROLPANEL_COMM    TEXT("ControlPanel\\Comm")
#define RV_CNCT                 TEXT("Cnct")
#define RV_AUTOCNCT             TEXT("AutoCnct")

//
// Keyboard CPL
//
// in HKCU
#define RK_CONTROLPANEL_KEYBD   TEXT("ControlPanel\\Keybd")
#define RV_DISPDLY              TEXT("DispDly")
#define RV_INITIALDELAY         TEXT("InitialDelay")
#define RV_REPEATRATE           TEXT("RepeatRate")

//
// Owner CPL
//
// in HKCU
#define RK_NETIDENT             TEXT("COMM\\Ident")
#define RV_PASSWORD             TEXT("LMPW")
#define RV_DOMAIN               TEXT("ComputerDomain")

//
// Power CPL
//
// in HKLM
#define RK_SYSTEM_CCS_CONTROL_POWER        TEXT("SYSTEM\\CurrentControlSet\\Control\\Power")
#define RV_BATTPOWEROFF         TEXT("BattPowerOff")
#define RV_EXTPOWEROFF          TEXT("ExtPowerOff")

//
// Screen CPL
//
// in HKCU
#define RK_CONTROLPANEL_BACKLIGHT    TEXT("ControlPanel\\BackLight")
#define RV_BATTERYTIMEOUT            TEXT("BatteryTimeout")
#define RV_OLD_BATTERYTIMEOUT        TEXT("OldBatteryTimeout")
#define RV_USEBATTERY                TEXT("UseBattery")
#define RV_ACTIMEOUT                 TEXT("ACTimeout")
#define RV_OLD_ACTIMEOUT             TEXT("OldACTimeout")
#define RV_USEAC                     TEXT("UseExt")
#define RV_ADVANCEDCPL               TEXT("AdvancedCPL")
#define RK_CONTROLPANEL_DESKTOP      TEXT("ControlPanel\\Desktop")
#define RV_TILE                      TEXT("Tile")
#define RV_WALLPAPER                 TEXT("Wallpaper")

// Color schemes
#define RK_COLORSCHEMES              TEXT("ControlPanel\\Appearance\\Schemes")
#define RK_APPEARANCE                TEXT("ControlPanel\\Appearance")
#define RV_CURRENT                   TEXT("Current")
#define RV_FULLCONTROL               TEXT("FullControl")

// Backlight
//This is the entry point we look for in the CPL pointed to by RV_ADVANCEDCPL
#define FUNCNAME_BACKLIGHTADVAPPLET        TEXT("BacklightAdvApplet")
// BacklightAdvApplet
//This is the event we signal when the settings changed under FUNCNAME_BACKLIGHTADVAPPLET.
#define EVENTNAME_ADVBACKLIGHTEVENT        TEXT("BackLightChangeEvent")

//This is the event we signal when the backlight settings change.
#define EVENTNAME_BACKLIGHTCHANGEEVENT     TEXT("BackLightChangeEvent")
#define DEF_BATTERYTIMEOUT	15
#define DEF_EXTTIMEOUT      60

//
// Sound scheme CPL. These values are read by the WaveAPI driver
//
// in HKLM
#define RK_SND_SCHEME           TEXT("Snd\\Scheme")
#define RK_SND_EVENT            TEXT("Snd\\Event")
#define RV_SCHEME               TEXT(".Scheme")
#define RV_EVENTMASK            TEXT("EventMask")
#define REGVALUE_DEFAULTSOUNDS  TEXT(".DefaultSounds")
#define REGVALUE_NOSOUNDS       TEXT(".NoSounds")
#define REGVALUE_ALLSOUNDS      TEXT(".AllSounds")

//
// Volume CPL. These values are read by the WaveAPI driver
//
// in HKCU
#define RK_CONTROLPANEL_VOLUME  TEXT("ControlPanel\\Volume")
#define RV_KEY                  TEXT("Key")        // LOWORD==Keyclick volume (0==off, 1==soft 2==loud)
#define RV_SCREEN               TEXT("Screen")    // LOWORD==ScreenTap volume (0==off, 1==soft 2==loud)
#define RV_MUTE                 TEXT("Mute")    // Events: 0==mute, 1==play. Bit0=Notif, Bit1==Apps, Bit2==Events
#define RV_VOLUME               TEXT("Volume")    // Volume: 0==off, 0xFFFFFFFF=max. loword & hiword must match (L & R channels)

//
// Stylus & Mouse CPLs
//
// in HKCU
#define RK_CONTROLPANEL_PEN     TEXT("ControlPanel\\Pen")
#define RV_DBLTAPTIME           TEXT("DblTapTime")
#define RV_DBLTAPDIST           TEXT("DblTapDist")

//
// SIP CPL
//
#define RK_CONTROLPANEL_SIP     TEXT("ControlPanel\\Sip")
#define RV_ALLOWCHANGE          TEXT("AllowChange")
//#define RV_SUGGESTWORDS[]     TEXT("SuggWords");
//#define RV_SUGGESTCHARS[]     TEXT("SuggChars");
//#define RV_SUGGESTWEIGHT[]    TEXT("SuggWeight");
//#define RV_SUGGESTADDSPACE[]  TEXT("SuggSpace");

#define RK_CLSID                TEXT("CLSID")
#define RK_ISSIPINPUTMETHOD     TEXT("IsSIPInputMethod")
#define RK_DEFAULTICON          TEXT("DefaultIcon")
#define RV_ISIM                 TEXT("1")

//
// Remove-programs CPL
//
#define RK_SOFTWAREAPPS         TEXT("SOFTWARE\\Apps")
#define RV_INSTALLED            TEXT("Instl")
#define RV_INSTALLDIR           TEXT("InstlDir")
#define RV_INSTALLDIRCOUNT      TEXT("InstlDirCnt")
#define RV_ISVFILE              TEXT("IsvFile")
#define RK_APPSSHARED           TEXT("SOFTWARE\\Apps\\Shared")
#define RV_PID                  TEXT("PID")
//
// Date-Time CPL
//
#define RK_CLOCK                TEXT("Software\\Microsoft\\Clock")
#define RV_INDST                TEXT("HomeDST")                        // are we currently in DST
#define RV_AUTODST              TEXT("AutoDST")
#define RV_DSTUI                TEXT("ShowDSTUI") 
#define RV_TIMEZONES            TEXT("Time Zones")
#define RV_DISPLAY              TEXT("Display")
#define RV_TZI                  TEXT("TZI")
#define RV_DLT                  TEXT("Dlt")
#define RV_STD                  TEXT("Std")

//
// Accessibility CPL
//
#define RK_ACCESSIB             TEXT("ControlPanel\\Accessibility")        
#define RK_ACCESSIBHC           TEXT("ControlPanel\\Accessibility\\HighContrast")

#define RV_HCSCHEME             TEXT("High Contrast Scheme")

//Registry values for General tab
#define RV_SOUNDONACTIV         TEXT("Sound on Activation")

/////////////////////////////////////////////////////
// Non registry, non-localizable constants
////////////////////////////////////////////////////

#define PATH_WINDOWS            TEXT("\\Windows")
#define PATH_WINDOWS_SLASH      TEXT("\\Windows\\")
#define CURR0                   TEXT("Curr0")
#define SPACE_COMMA             TEXT(" ,")
#define BACKSLASH               TEXT("\\")
#define CTLPNL_CLASS            TEXT("CTLPNL_Class") // class name of our hidden window
#define WELCOME_CLASS           TEXT("Welcome")
#define DIALOG_CLASS            TEXT("Dialog")
#define PEGHELP_EXE             TEXT("\\Windows\\peghelp.exe")
#define UNLOAD_SCRIPT           TEXT("\\Windows\\%s.Unload")

//Power CPL///////////////////////
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

#define	PM_EVENT_NAME			TEXT("PowerManager/ReloadActivityTimeouts")

#define	PM_MAX_BATT_TIMEOUT		5*60 
#define PM_TRANSITION_NEVER_DW	0
/////////////////////////////////
//Ignition CPL///////////////////////
#define RK_FULL_PSC						PWRMGR_REG_KEY TEXT("\\PSC")	
#define RK_IGN_ENABLE					TEXT("IgnitionSwitchControlEnable")
#define RK_PSC_IGN_TIME					TEXT("IgnStableTime")
#define RK_PSC_IGN_ACT					TEXT("PerformAction")
#define RK_PSC_IGN_ACT_DELAY			TEXT("PerformActionDelay")
#define RK_PSC_IGN_SHUTDOWN_ABSOLUTE	TEXT("ShutDownAbsolute")

#define	RESTART_STR			TEXT("You must restart your system ")\
							TEXT("for the configuration changes to take effect. ")\
							TEXT("Click Yes to restart now or No if you plan to restart later.")
 
#define	DISABLE_IGNI_WARN	TEXT("One or more Power Management timeout parameters collide with the ")\
							TEXT("ignition switch control settings. Do you want to continue? If you ")\
							TEXT("select YES, ignition switch control will be disabled.")
#define PWR_TIMEOUTS_WARN	TEXT("One or more Power Management timeout parameters collide with the ")\
							TEXT("ignition switch control settings. Do you want to continue? If you ")\
							TEXT("select YES, power management timeouts will be adjusted automatically.")
