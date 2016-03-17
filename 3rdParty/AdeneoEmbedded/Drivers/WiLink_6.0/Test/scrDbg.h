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
/** \file  scrDbg.h
 *  \brief This file include private definitions for the SCR debug module.
 *
 *  \see   scrDbg.c, scrApi.h
 */

#ifndef __SCRDBG_H__
#define __SCRDBG_H__

#include "scr.h"

/*
 ***********************************************************************
 *  Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_SCR_PRINT_HELP                      0
#define DBG_SCR_CLIENT_REQUEST_SERVING_CHANNEL  1
#define DBG_SCR_CLIENT_RELEASE_SERVING_CHANNEL  2
#define DBG_SCR_CLIENT_REQUEST_PERIODIC_SCAN    3
#define DBG_SCR_CLIENT_RELEASE_PERIODIC_SCAN    4
#define DBG_SCR_SET_GROUP                       5
#define DBG_SCR_PRINT_OBJECT                    6
#define DBG_SCR_SET_MODE                        7

/*
 ***********************************************************************
 *  Enums.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  Typedefs.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  Structure definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  External data definitions.
 ***********************************************************************
 */

/*
 ***********************************************************************
 *  External functions definitions
 ***********************************************************************
 */

/**
 * \\n
 * \date 01-May-2005\n
 * \brief Main SCR debug function
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void scrDebugFunction( TI_HANDLE hScanMngr, TI_UINT32 funcType, void *pParam );

/**
 * \\n
 * \date 01-May-2005\n
 * \brief Prints SCR debug menu
 *
 * Function Scope \e Public.\n
 */
void printScrDbgFunctions(void);

/**
 * \\n
 * \date 29-March-2005\n
 * \brief Request the SCR with a given client ID.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param client - the client to request as.\n\
 * \param eResource - the requested resource.\n
 */
void requestAsClient( TI_HANDLE hScr, EScrClientId client, EScrResourceId eResource );

/**
 * \\n
 * \date 01-May-2005\n
 * \brief Stops continuous scan process.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param client - the client to release as.\n\
 */
void releaseAsClient( TI_HANDLE hScr, EScrClientId client, EScrResourceId eResource );

/**
 * \\n
 * \date 01-May-2005\n
 * \brief Change the SCR group.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param group - the group to change to.\n
 */
void changeGroup( TI_HANDLE hScr, EScrGroupId group );

/**
 * \\n
 * \date 23-Nov-2005\n
 * \brief Change the SCR mode.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 * \param mode - the mode to change to.\n
 */
void changeMode( TI_HANDLE hScr, EScrModeId mode );

/**
 * \\n
 * \date 15-June-2005\n
 * \brief Prints the SCR object.\n
 *
 * Function Scope \e Public.\n
 * \param hScr - handle to the SCR object.\n
 */
void printSCRObject( TI_HANDLE hScr );

#endif /* __SCRDBG_H__ */
