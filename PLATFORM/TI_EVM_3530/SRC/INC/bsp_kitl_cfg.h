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
//
//  File:  bsp_kitl_cfg.h
//
#ifndef _BSP_KITL_CFG_H
#define _BSP_KITL_CFG_H

//------------------------------------------------------------------------------
//
//  Define:  BSP_IRQ_ETHER_KITL
//
//  irq of ethernet kitl
//
#define BSP_IRQ_ETHER_KITL             (IRQ_GPIO_95)
#define BSP_ETHER_GPIO_PA              (OMAP_GPIO3_REGS_PA)
//------------------------------------------------------------------------------
//
//  Define:  BSP_RESET_ETHER_KITL
//
//  GPIO pin that drives ethernet kitl reset input
//
#define BSP_RESET_ETHER_KITL_GPIO_PA    (OMAP_GPIO2_REGS_PA)
#define BSP_RESET_ETHER_KITL_GPIO       (61)


//------------------------------------------------------------------------------
#endif
