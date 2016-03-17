// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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

#ifndef __TOUCH_H
#define __TOUCH_H


//------------------------------------------------------------------------------
// Sample rate during polling.
#define DEFAULT_SAMPLE_RATE_HZ          200
#define TOUCHPANEL_SAMPLE_RATE_LOW      DEFAULT_SAMPLE_RATE_HZ
#define TOUCHPANEL_SAMPLE_RATE_HIGH     DEFAULT_SAMPLE_RATE_HZ

// Number of samples to discard when pen is initially down
#define DEFAULT_INITIAL_SAMPLES_DROPPED 1

#define DIFF_X_MAX            10
#define DIFF_Y_MAX            10

#define FILTEREDSAMPLESIZE      3
#define SAMPLESIZE              1

// Internal functions.
BOOL PddGetTouchIntPinState( VOID );
BOOL PddGetTouchData(UINT32 * pxPos, UINT32 * pyPos);
BOOL PddGetRegistrySettings( PDWORD );
BOOL PddInitializeHardware( VOID );
VOID PddDeinitializeHardware( VOID );

#endif