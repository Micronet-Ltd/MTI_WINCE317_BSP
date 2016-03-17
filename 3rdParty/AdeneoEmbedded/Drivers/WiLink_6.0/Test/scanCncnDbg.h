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
/** \file ScanCncn.h
 *  \brief This file include private definitions for the scan concentrator debug module.
 *  \
 *  \date 30-Dec-2004
 */

#ifndef __SCANCNCNDBG_H__
#define __SCANCNCNDBG_H__

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_SCAN_CNCN_PRINT_HELP		    	0
#define DBG_SCAN_SRV_PRINT_STATUS               1

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
 * \brief Main scan concentrator debug function
 *
 * Function Scope \e Public.\n
 * \param hScanCncn - handle to the scan concentrator object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void scanConcentratorDebugFunction( TI_HANDLE hScanCncn, TI_HANDLE hScanSrv, TI_UINT32 funcType, void *pParam );

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Prints scan concentrator debug menu
 *
 * Function Scope \e Public.\n
 */
void printScanConcentratorDbgFunctions(void);

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Performs a driver scan from within an app scan.\n
 *
 * Function Scope \e Public.\n
 * \param hScanCncn - handle to the scan concentrator object.\n
 */
void appDriverScan( TI_HANDLE hScanCncn );

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Performs an app scan from within a driver scan.\n
 *
 * Function Scope \e Public.\n
 * \param hScanCncn - handle to the scan concentrator object.\n
 */
void driverAppScan( TI_HANDLE hScanCncn );

#endif /* __SCANCNCNDBG_H__ */
