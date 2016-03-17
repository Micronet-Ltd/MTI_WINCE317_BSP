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
//  File:  stub.c
//
#include <windows.h>
#include <oal.h>
#include <oalex.h>

//------------------------------------------------------------------------------
//
//  Function:  NKCreateStaticMapping
//
void* 
NKCreateStaticMapping(
    DWORD phBase, 
    DWORD size
    )
{
    return OALPAtoUA(phBase << 8);
}

//------------------------------------------------------------------------------

void
OALPowerEnableCoreClock(
    UINT clock
    )
{
}

//------------------------------------------------------------------------------

void
OALPowerDisableCoreClock(
    UINT clock
    )
{
}

//------------------------------------------------------------------------------

