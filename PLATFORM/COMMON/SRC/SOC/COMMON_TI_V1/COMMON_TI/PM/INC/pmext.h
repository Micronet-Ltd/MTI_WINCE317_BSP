// All rights reserved ADENEO EMBEDDED 2010
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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//

//
// This module contains enum needed to compile the power manger extention.
//

// This typedef describes the system activity states.  These are independent of
// factors such as AC power vs. battery, in cradle or not, etc.  OEMs may choose
// to add their own activity states if they customize this module.

// This typedef describes activity events such as user activity or inactivity,
// power status changes, etc.  OEMs may choose to factor other events into their
// system power state transition decisions.

#pragma once
#include <pm.h>

typedef enum {
    NoActivity=0,
    UserActivity,
    UserInactivity,
    SystemActivity,
    SystemInactivity,
    EnterUnattendedModeRequest,
    LeaveUnattendedModeRequest,
    Timeout,
    RestartTimeouts,
    PowerSourceChange,
    Resume,
    SystemPowerStateChange,
    SystemPowerStateAPI,
    PmShutDown,
    PmReloadActivityTimeouts,
    PowerButtonPressed,
    AppButtonPressed, 
    BootPhaseChanged,
    PowerManagerExt = 0x800,
    ExternedEvent = 0x1000
} PLATFORM_ACTIVITY_EVENT, *PPLATFORM_ACTIVITY_EVENT;

typedef DWORD   (WINAPI * PFN_PMExt_Init)(HKEY hKey, LPCTSTR lpRegistryPath);
typedef VOID    (WINAPI * PFN_PMExt_DeInit) (DWORD dwExtContext );

#define PMExt_Init_NAME TEXT("PMExt_Init")
#define PMExt_DeInit_NAME TEXT("PMExt_DeInit")

#define PMExt_Registry_Root TEXT("SYSTEM\\CurrentControlSet\\Control\\Power\\Extension")
