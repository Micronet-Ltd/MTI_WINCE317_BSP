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
/** \file smeDebug.h
 *  \brief This file include private definitions for the SME debug module.
 *  \
 *  \date 13-february-2006
 */

#ifndef __SMEDBG_H__
#define __SMEDBG_H__

#include "sme.h"

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_SME_PRINT_HELP		    	0
#define DBG_SME_PRINT_OBJECT	        1
#define DBG_SME_PRINT_STATS             2
#define DBG_SME_CLEAR_STATS             3
#define DBG_SME_BSSID_LIST              4

/*
 ***********************************************************************
 *	Enums.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Typedefs.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	Structure definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	External data definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *	External functions definitions
 ***********************************************************************
 */

/**
 * \\n
 * \date 13-February-2006\n
 * \brief Main SME debug function
 *
 * Function Scope \e Public.\n
 * \param hSmeSm - handle to the SME SM object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void smeDebugFunction( TI_HANDLE hSmeSm, TI_UINT32 funcType, void *pParam );

/**
 * \\n
 * \date 13-February-2006\n
 * \brief Prints SME debug menu
 *
 * Function Scope \e Public.\n
 */
void printSmeDbgFunctions(void);

#endif /* __SMEDBG_H__ */

