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
/** \file ScanMngrDbg.h
 *  \brief This file include private definitions for the scan manager debug module.
 *  \
 *  \date 29-March-2005
 */

#ifndef __SCANMNGRDBG_H__
#define __SCANMNGRDBG_H__

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_SCAN_MNGR_PRINT_HELP		    	0
#define DBG_SCAN_MNGR_START_CONT_SCAN           1
#define DBG_SCAN_MNGR_STOP_CONT_SCAN            2
#define DBG_SCAN_MNGR_START_IMMED_SCAN          3
#define DBG_SCAN_MNGR_STOP_IMMED_SCAN           4
#define DBG_SCAN_MNGR_PRINT_TRACK_LIST          5
#define DBG_SCAN_MNGR_PRINT_STATS               6
#define DBG_SCAN_MNGR_RESET_STATS               7
#define DBG_SCAN_MNGR_PIRNT_NEIGHBOR_APS        8
#define DBG_SCAN_MNGR_PRINT_POLICY              9
#define DBG_SCAN_MNGR_PRINT_OBJECT              10

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
 * \date 29-March-2005\n
 * \brief Main scan manager debug function
 *
 * Function Scope \e Public.\n
 * \param hScanMngr - handle to the scan manager object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 * \param hSiteMgr - handle to th esite manager object.\n
 * \param hCtrlData - handle to the data ctrl object.\n
 */
void scanMngrDebugFunction( TI_HANDLE hScanMngr, TI_UINT32 funcType, void *pParam, TI_HANDLE hSiteMgr, TI_HANDLE hCtrlData );

/**
 * \\n
 * \date 29-March-2005\n
 * \brief Prints scan manager debug menu
 *
 * Function Scope \e Public.\n
 */
void printScanMngrDbgFunctions(void);

/**
 * \\n
 * \date 29-March-2005\n
 * \brief Starts continuous scan process.\n
 *
 * Function Scope \e Public.\n
 * \param hScanMngr - handle to the scan manager object.\n
 * \param hSiteMgr - handle to the site manager object.\n\
 * \param hCtrlData - handle to the data ctrl object.\n
 */
void startContScan( TI_HANDLE hScanMngr, TI_HANDLE hSiteMgr, TI_HANDLE hCtrlData );

#endif /* __SCANMNGRDBG_H__ */
