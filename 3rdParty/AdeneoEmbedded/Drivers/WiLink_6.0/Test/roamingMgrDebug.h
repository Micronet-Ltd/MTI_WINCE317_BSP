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
#ifndef __ROAMIN_MGR_DEBUG_H__
#define __ROAMIN_MGR_DEBUG_H__

#include "tidef.h"
#include "paramOut.h"

#define ROAMING_MGR_DEBUG_HELP_MENU 			0
#define PRINT_ROAMING_STATISTICS				1
#define RESET_ROAMING_STATISTICS				2
#define PRINT_ROAMING_CURRENT_STATUS			3
#define PRINT_ROAMING_CANDIDATE_TABLE			4
#define TRIGGER_ROAMING_LOW_QUALITY_EVENT		5
#define TRIGGER_ROAMING_BSS_LOSS_EVENT			6 
#define TRIGGER_ROAMING_SWITCH_CHANNEL_EVENT	7 
#define TRIGGER_ROAMING_AP_DISCONNECT_EVENT		8 
#define TRIGGER_ROAMING_CONNECT_EVENT			9 	
#define TRIGGER_ROAMING_NOT_CONNECTED_EVENT		10
#define TRIGGER_ROAMING_HANDOVER_SUCCESS_EVENT	11
#define TRIGGER_ROAMING_HANDOVER_FAILURE_EVENT	12

/* Added for EMP project */
#define ROAMING_REGISTER_BSS_LOSS_EVENT         13
#define ROAMING_START_IMMEDIATE_SCAN            14
#define ROAMING_CONNECT                         15
#define ROAMING_START_CONT_SCAN_BY_APP          16
#define ROAMING_STOP_CONT_SCAN_BY_APP           17
#define RAOMING_SET_DEFAULT_SCAN_POLICY         18
#define ROAMING_PRINT_MANUAL_MODE               19

void roamingMgrDebugFunction(TI_HANDLE hRoamingMngr, 
					   TI_UINT32	funcType, 
					   void		*pParam);


#endif /* __ROAMIN_MGR_DEBUG_H__ */
