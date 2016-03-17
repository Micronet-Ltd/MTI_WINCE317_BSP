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
/** \file SoftGeminiDbg.h
 *  \brief This file include private definitions for the soft Gemini debug module.
 *  \
 *  \date 30-Dec-2004
 */

#ifndef __SOFT_GEMINI_DBG_H__
#define __SOFT_GEMINI_DBG_H__

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_SG_PRINT_HELP					  	 0
#define DBG_SG_PRINT_PARAMS						 1
#define DBG_SG_SENSE_MODE						 2
#define DBG_SG_PROTECTIVE_MODE					 3

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
 * \date 14-Feb-2005\n
 * \brief Main soft Geminirdebug function
 *
 * Function Scope \e Public.\n
 * \param hSoftGemini - handle to the scan concentrator object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void SoftGeminiDebugFunction( TI_HANDLE hSoftGemini, TI_UINT32 funcType, void *pParam );

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Prints scan concentrator debug menu
 *
 * Function Scope \e Public.\n
 */
void printSoftGeminiDbgFunctions(void);

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Prints SoftGemini params.\n
 *
 * Function Scope \e Public.\n
 * \param hSoftGemini - handle to the SoftGemini object.\n
 */
void printSoftGeminiParams( TI_HANDLE hSoftGemini );


#endif /* __SOFT_GEMINI_DBG_H__ */
