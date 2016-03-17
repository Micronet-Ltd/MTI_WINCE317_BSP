/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/** \file  osentry.h 
 *  \brief This module defines unified interface to the OS specific sources and services.
 *
 *  \see   osentry.c
 */

#ifndef __OS_ENTRY_H__
#define __OS_ENTRY_H__

#ifdef WM_ZOOM2
/* Check hardware present */
#define CHK_HW_PRESENT
void Get_GPIOPWR_LINE(int);
#endif
NDIS_STATUS WiLinkCardEnable(void);
NDIS_STATUS WiLinkCardDisable(void);
NDIS_STATUS WlanDriverEnable(TWlanDrvIfObjPtr pTWlanDrvIf, TI_BOOL bPowerSet);
void WlanDriverDisable(TWlanDrvIfObjPtr pTWlanDrvIf, TI_BOOL bPowerSet);

void os_closeFirmwareImage( TI_HANDLE OsContext );
void os_closeRadioImage( TI_HANDLE OsContext );

void os_setPowerOfTnetw(TI_BOOL bPowerOn);
void os_hardResetTnetw( void );


#endif 

