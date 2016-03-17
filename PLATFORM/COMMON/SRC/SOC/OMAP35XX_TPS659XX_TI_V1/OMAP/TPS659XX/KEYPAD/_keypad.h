/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//
//  File: _keypad.h
//

#ifndef __KEYPAD_H_
#define __KEYPAD_H_

#include <light_sense.h>
#include <tled.h>
#include <madc.h>

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)
#define ZONE_IST            DEBUGZONE(4)

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define KPD_DEVICE_COOKIE           'kpdD'

#define MATRIX_SIZE                 KEYPAD_ROWS
#define VK_KEYS                     256
#define DWORD_BITS                  32

#define MATRIX_SIZE_EXT             MATRIX_SIZE
//------------------------------------------------------------------------------
//  Local Structures

typedef struct {
    DWORD cookie;
    DWORD priority256;
    DWORD enableWake;
    DWORD clockDivider;
    DWORD debounceCount;
    DWORD samplePeriod;
    DWORD firstRepeat;
    DWORD nextRepeat;
    HANDLE hIntrEventKeypad;
    HANDLE hIntrThreadKeypad;
    HANDLE hIntrEventPower;
    HANDLE hIntrThreadPower;
    BOOL intrThreadExit;
    DWORD powerMask;
    CEDEVICE_POWER_STATE powerState;

    HANDLE hTWL;

    DWORD bEnableOffKey;
    DWORD bDisablePwronKey;
	DWORD isSWMode;
    CRITICAL_SECTION csKeymap;
	UINT32 long_pressDelay;
  
    HANDLE m_hLed;
    HANDLE m_hLight;
    HANDLE hLightThread;
    HANDLE hKeypressEvent;
    UINT32 KpLedNum;
    UINT32 KpLedIntensityHigh;
    UINT32 KpLedIntensityNormal;
    UINT32 KpLedIntensityLow;
    INT32  KpLightEnabled;
    UINT32 KpLightDepend;
	AMBIENT_LIGHT currentAmbientLight;
    CRITICAL_SECTION    cs;

	DWORD	KpPwrNotif;
	HANDLE	hGpio;
    DWORD bEnableSuspendKey;
} KeypadDevice_t;

typedef struct {
    BOOL pending;
    BOOL remapped;
    DWORD time;
    DWORD state;
} KeypadRemapState_t;

typedef struct {
    BOOL pending;
    DWORD time;
    BOOL blocked;
} KeypadRepeatState_t;

typedef struct {
    BOOL pending;
    DWORD time;
} KeypadLongPressState_t;

//-----------------------------------------------------------------------------
BOOL
KPD_Deinit(
    DWORD context
    );

DWORD KPD_IntrThread(
    VOID *pContext
    );

DWORD PWRON_IntrThread(
    VOID *pContext
    );

VOID 
SendKeyPadEvent(
    const KeypadDevice_t *pDevice,
    BYTE bVk,
    BYTE bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo
    );

VOID
PressedReleasedKeys(
    KeypadDevice_t *pDevice,
    const DWORD vkState[],
    const DWORD vkNewState[]
    );

DWORD KPD_LightThread(VOID *pContext);

void reset_int_mode(HANDLE hGpio, BOOL fSuspend );

#endif //__KEYPAD_H_
//-----------------------------------------------------------------------------

