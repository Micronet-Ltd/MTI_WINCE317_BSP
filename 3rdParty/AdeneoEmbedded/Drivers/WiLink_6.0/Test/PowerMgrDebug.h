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
/** \file PowerMgrDebug.h
 *  \brief This is the Power Manager module private (internal).
 *  \
 *  \date 13-Jun-2004
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  Power Manager Debug                                                *
 *   PURPOSE: Power Manager Debug Module private                                      *
 *                                                                          *
 ****************************************************************************/

#ifndef _POWER_MGR_DEBUG_H_
#define _POWER_MGR_DEBUG_H_

#include "tidef.h"

/*****************************************************************************
 **         Constants                                                       **
 *****************************************************************************/


/*****************************************************************************
 **         Enumerations                                                    **
 *****************************************************************************/

enum
{
    POWER_MGR_DEBUG_PRINT_ALL_COMMANDS,
    POWER_MGR_DEBUG_START_PS,
    POWER_MGR_DEBUG_STOP_PS,
    POWER_MGR_PRINT_OBJECTS,
    POWER_MGR_DEBUG_MAX_COMMANDS
};

/*****************************************************************************
 **         Typedefs                                                        **
 *****************************************************************************/


/*****************************************************************************
 **         Structures                                                      **
 *****************************************************************************/


/*****************************************************************************
 **         External data definitions                                       **
 *****************************************************************************/


/*****************************************************************************
 **         External functions definitions                                  **
 *****************************************************************************/


/*****************************************************************************
 **         Public Function prototypes                                      **
 *****************************************************************************/

void powerMgrDebugFunction(TI_HANDLE thePowerMgrHandle,
                           TI_UINT32 theDebugFunction,
                           void* theParameter);

#endif /*  _POWER_MGR_DEBUG_H_  */

