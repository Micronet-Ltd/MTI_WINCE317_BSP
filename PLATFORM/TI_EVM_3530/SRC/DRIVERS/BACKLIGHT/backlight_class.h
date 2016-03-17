//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  File:  backlight_class.h
//
#ifndef __BACKLIGHT_CLASS_H
#define __BACKLIGHT_CLASS_H


#ifndef SHIP_BUILD
#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)
#endif

#include "bl.h"

//------------------------------------------------------------------------------
//  CBacklight class
//
class CBacklightRoot
{
// public methods
//---------------
//
public:

    virtual BOOL Initialize(LPCTSTR szContext)              { return TRUE; }
    virtual BOOL Uninitialize()                             { return TRUE; }
    virtual BOOL SetPowerState(CEDEVICE_POWER_STATE power)  { return TRUE; }
};

//-----------------------------------------------------------------------------
//  backlight object which drives the triton LED
//
class TLED_Backlight : public CBacklightRoot
{
    typedef CBacklightRoot              ParentClass;

// member variables
//---------------
//
public:

    HANDLE          m_hTled;
    DWORD           m_channel;
    DWORD           m_dutyCycles[STATES_QTY];
    DWORD           m_dutyCyclesBatt[STATES_QTY];
    DWORD           m_dutyCyclesAC[STATES_QTY];
	DWORD			m_MaxDutyCyclesDef;
	DWORD			m_MaxDutyCycles7i;
	DWORD			m_minDutyCycle;
	BOOL			m_bIs4inch;

// constructor
//------------
//
public:

    TLED_Backlight() : m_hTled(NULL), m_channel(-1), m_MaxDutyCyclesDef(BKL_DEFAULT_DUTY_CYCLE), m_MaxDutyCycles7i(BKL_7INCH_DUTY_CYCLE), m_bIs4inch(1)     
	{
		m_dutyCycles[0] = BACKLIGHT_HIGH;
		m_dutyCycles[1] = BACKLIGHT_LOW;
		m_dutyCycles[2] = BACKLIGHT_LOW;
		m_dutyCycles[3] = BACKLIGHT_LOW;
		m_dutyCycles[4] = BACKLIGHT_LOW;
	};


// public virtual methods
//-----------------------
//
public:

    virtual BOOL Initialize(LPCTSTR szContext);
    virtual BOOL Uninitialize();
    virtual BOOL SetPowerState(CEDEVICE_POWER_STATE power);

	void	UpdateDutyCycles( DWORD PwrSrc, DWORD LsLevel );
	void	UpdateRegParams();
};


//-----------------------------------------------------------------------------
//  backlight object which drives a GPIO line
//
class GPIO_Backlight : public TLED_Backlight
{
    typedef TLED_Backlight              ParentClass;

// member variables
//---------------
//
public:

    HANDLE          m_hGpio;
    DWORD           m_gpioId;
    DWORD           m_powerMask;


// constructor
//------------
//
public:

    GPIO_Backlight() : m_hGpio(NULL), m_gpioId(-1)      {};


// public virtual methods
//-----------------------
//
public:

    virtual BOOL Initialize(LPCTSTR szContext);
    virtual BOOL Uninitialize();
    virtual BOOL SetPowerState(CEDEVICE_POWER_STATE power);
    virtual UCHAR BacklightGetSupportedStates( );

};


#endif //__BACKLIGHT_H