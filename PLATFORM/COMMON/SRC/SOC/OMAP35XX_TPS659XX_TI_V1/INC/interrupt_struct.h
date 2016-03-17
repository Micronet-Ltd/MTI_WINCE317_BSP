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
//  file: interrupt_struct.h
//
#ifndef __INTERRUPT_STRUCT_H
#define __INTERRUPT_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define: OMAP_INTR_CONTEXT
//
typedef struct {
    OMAP_INTC_MPU_REGS *pICLRegs;
    OMAP_GPIO_REGS  *pGPIORegs[OMAP_GPIO_BANK_COUNT];   
} OMAP_INTR_CONTEXT;

#ifdef __cplusplus
}
#endif

#endif


