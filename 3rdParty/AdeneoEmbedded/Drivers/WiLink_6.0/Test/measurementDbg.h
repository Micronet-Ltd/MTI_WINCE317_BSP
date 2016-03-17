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

#ifndef __MEASUREMENT_DBG_H__
#define __MEASUREMENT_DBG_H__

/* definitions for ctrl dbg */

#define DBG_MEASUREMENT_PRINT_HELP		    	0
#define DBG_MEASUREMENT_PRINT_STATUS			1
#define	DBG_MEASUREMENT_CHANNEL_LOAD_START  	2 
#define	DBG_MEASUREMENT_CHANNEL_LOAD_STOP	    3 
#define DBG_MEASUREMENT_SEND_FRAME_REQUEST      4 
#define DBG_MEASUREMENT_START_NOISE_HIST        5 
#define DBG_MEASUREMENT_STOP_NOISE_HIST         6 
#define DBG_MEASUREMENT_GET_NOISE_HIST_RESULTS  7 
#define DBG_MEASUREMENT_SEND_CHANNEL_LOAD_FRAME 8 
#define DBG_MEASUREMENT_SEND_BEACON_TABLE_FRAME 9
#define DBG_MEASUREMENT_SEND_NOISE_HIST_1_FRAME 10
#define DBG_MEASUREMENT_SEND_NOISE_HIST_2_FRAME 11
#define DBG_MEASUREMENT_SET_TRAFFIC_THRSLD      12
#define DBG_SC_PRINT_STATUS					    30
#define DBG_SC_SET_SWITCH_CHANNEL_NUM			31
#define DBG_SC_SET_SWITCH_CHANNEL_TBTT			32
#define DBG_SC_SET_SWITCH_CHANNEL_MODE         33
#define DBG_SC_SET_CHANNEL_AS_VALID			    34
#define DBG_SC_SET_CHANNEL_AS_INVALID			35
#define DBG_SC_SWITCH_CHANNEL_CMD				36
#define DBG_SC_CANCEL_SWITCH_CHANNEL_CMD		37

#define DBG_REG_DOMAIN_PRINT_VALID_CHANNELS		50




void measurementDebugFunction(TI_HANDLE hMeasurementMgr, TI_HANDLE hSwitchChannel, TI_HANDLE hRegulatoryDomain, TI_UINT32 funcType, void *pParam);

void measurement_channelLoadCallBackDbg(TI_HANDLE hMeasurementMgr, TI_STATUS status, 
                                        TI_UINT8* CB_buf);

void measurement_noiseHistCallBackDbg(TI_HANDLE hMeasurementMgr, TI_STATUS status, 
                                      TI_UINT8* CB_buf);

#endif /* __MEASUREMENT_DBG_H__*/
