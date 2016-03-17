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
//
//  File:  omap_pinmux_stubs.c
//
#include <windows.h>

//------------------------------------------------------------------------------
//
//  Function:  OALMux_InitMuxTable
//
//  initialization for pinmux on device state change.
//
void
OALMux_InitMuxTable(
    )
{
}

//------------------------------------------------------------------------------
//
//  Function:  OALMux_UpdateOnDeviceStateChange
//
//  called before and after a device state change.
//
DWORD
OALMux_UpdateOnDeviceStateChange(
    UINT devId,
    UINT oldState,
    UINT newState,
    BOOL bPreStateChange
    )
{
    return -1;
}
//------------------------------------------------------------------------------

