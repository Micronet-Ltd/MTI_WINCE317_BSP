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

//------------------------------------------------------------------------------
//
//  File:  omap35xx_wdog.h
//
//  This file contains offset addresses for WatchDog registers.

#ifndef __OMAP35XX_WDOG_H
#define __OMAP35XX_WDOG_H

//------------------------------------------------------------------------------
//
// Watchdog Timer
//
typedef volatile struct {

    UINT32 WIDR;              // offset 0x0000, WIDR
    UINT32 zzzReserved01[3];
    
    UINT32 WD_SYSCONFIG;      // offset 0x0010, WD_SYSCONFIG
    UINT32 WD_SYSSTATUS;      // offset 0x0014, WD_SYSSTATUS
    UINT32 WISR;              // offset 0x0018, WISR
    UINT32 WIER;              // offset 0x001C, WIER
    UINT32 zzzReserved02[1];
    
    UINT32 WCLR;              // offset 0x0024, WCLR
    UINT32 WCRR;              // offset 0x0028, WCRR
    UINT32 WLDR;              // offset 0x002C, WLDR
    UINT32 WTGR;              // offset 0x0030, WTGR
    UINT32 WWPS;              // offset 0x0034, WWPS   
    UINT32 zzzReserved03[4];
    
    UINT32 WSPR;              // offset 0x0048, WSPR   
    
} OMAP_WDOG_REGS;

//------------------------------------------------------------------------------

#define WDOG_DISABLE_SEQ1       0x0000AAAA
#define WDOG_DISABLE_SEQ2       0x00005555

#define WDOG_ENABLE_SEQ1        0x0000BBBB
#define WDOG_ENABLE_SEQ2        0x00004444

//------------------------------------------------------------------------------

#endif
