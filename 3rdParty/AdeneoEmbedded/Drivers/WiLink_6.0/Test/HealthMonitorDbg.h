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

#ifndef __HEALTH_MONITOR_DBG_H__
#define __HEALTH_MONITOR_DBG_H__

/*
 ***********************************************************************
 *	Constant definitions.
 ***********************************************************************
 */

/* debug functions */
#define DBG_HM_PRINT_HELP					  0 	
#define DBG_HM_RECOVERY_NO_SCAN_COMPLETE	  1   
#define DBG_HM_RECOVERY_MBOX_FAILURE		  2   
#define DBG_HM_RECOVERY_HW_AWAKE_FAILURE	  3   
#define DBG_HM_RECOVERY_TX_STUCK			  4   
#define DBG_HM_DISCONNECT_TIMEOUT			  5   
#define DBG_HM_RECOVERY_POWER_SAVE_FAILURE	  6   
#define DBG_HM_RECOVERY_MEASUREMENT_FAILURE	  7   
#define DBG_HM_RECOVERY_BUS_FAILURE	          8   
#define DBG_HM_RECOVERY_FROM_CLI              9
#define DBG_HM_RECOVERY_FROM_HW_WD_EXPIRE     10
#define DBG_HM_RECOVERY_RX_XFER_FAILURE       11
                                               
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
 * \param pStadHandles - modules handles list.\n
 * \param funcType - the specific debug function.\n
 * \param pParam - parameters for the debug function.\n
 */
void healthMonitorDebugFunction (TStadHandlesList *pStadHandles, TI_UINT32 funcType, void *pParam);

/**
 * \\n
 * \date 14-Feb-2005\n
 * \brief Prints scan concentrator debug menu
 *
 * Function Scope \e Public.\n
 */
void printHealthMonitorDbgFunctions(void);



#endif /* __HEALTH_MONITOR_DBG_H__ */
