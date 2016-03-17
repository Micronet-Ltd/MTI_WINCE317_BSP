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

/** \file   TwIfDebug.h 
 *  \brief  TwIf testing module API definitions                                  
 *
 *  \see    TwIf.c, TwIf.h
 */


#ifndef __TWIF_DBG_H__
#define __TWIF_DBG_H__


/************************************************************************
 * Defines
 ************************************************************************/
/* debug functions */
#define DBG_TWIF_PRINT_HELP		    	0
#define DBG_TWIF_PRINT_INFO	            1
#define DBG_TWIF_PRINT_WAKE_LOCK_COUNTER 2

/************************************************************************
 * Types
 ************************************************************************/


/************************************************************************
 * Functions
 ************************************************************************/
void twifDebugFunction (TI_HANDLE hTWD, TI_UINT32 uFuncType, void *pParam);



#endif /* __TWIF_DBG_H__ */

