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
//  Header:  omap35xx_gpio.h
//
#ifndef __OMAP35XX_GPIO_H
#define __OMAP35XX_GPIO_H

//------------------------------------------------------------------------------

typedef volatile struct {
    UINT32 REVISION;                // 0x0000
    UINT32 zzzReserved01[3];
    UINT32 SYSCONFIG;               // 0x0010
    UINT32 SYSSTATUS;               // 0x0014
    UINT32 IRQSTATUS1;              // 0x0018
    UINT32 IRQENABLE1;              // 0x001C
    UINT32 WAKEUPENABLE;            // 0x0020
    UINT32 RESERVED_0024;
    UINT32 IRQSTATUS2;              // 0x0028
    UINT32 IRQENABLE2;              // 0x002C
    UINT32 CTRL;                    // 0x0030
    UINT32 OE;                      // 0x0034
    UINT32 DATAIN;                  // 0x0038
    UINT32 DATAOUT;                 // 0x003C
    UINT32 LEVELDETECT0;            // 0x0040
    UINT32 LEVELDETECT1;            // 0x0044
    UINT32 RISINGDETECT;            // 0x0048
    UINT32 FALLINGDETECT;           // 0x004C
    UINT32 DEBOUNCENABLE;           // 0x0050
    UINT32 DEBOUNCINGTIME;          // 0x0054
    UINT32 zzzReserved02[2];
    UINT32 CLEARIRQENABLE1;         // 0x0060
    UINT32 SETIRQENABLE1;           // 0x0064
    UINT32 zzzReserved03[2];
    UINT32 CLEARIRQENABLE2;         // 0x0070
    UINT32 SETIRQENABLE2;           // 0x0074
    UINT32 zzzReserved04[2];
    UINT32 CLEARWAKEUPENA;          // 0x0080
    UINT32 SETWAKEUPENA;            // 0x0084
    UINT32 zzzReserved05[2];
    UINT32 CLEARDATAOUT;            // 0x0090
    UINT32 SETDATAOUT;              // 0x0094
    UINT32 zzzReserved06[2];
} OMAP_GPIO_REGS;


//------------------------------------------------------------------------------

#endif // __OMAP35XX_GPIO_H

