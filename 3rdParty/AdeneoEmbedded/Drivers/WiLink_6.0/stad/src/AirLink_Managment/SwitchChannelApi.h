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
/** \file SwitchChannel.h
 *  \brief SwitchChannel module interface header file
 *
 *  \see SwitchChannel.c
 */

/***************************************************************************/
/*																			*/
/*	  MODULE:	SwitchChannel.h														*/
/*    PURPOSE:	SwitchChannel module inteerface header file							*/
/*																			*/
/***************************************************************************/
#ifndef __SWITCH_CHANNEL_H__
#define __SWITCH_CHANNEL_H__

#include "paramOut.h"
#include "DrvMainModules.h"


TI_HANDLE switchChannel_create(TI_HANDLE hOs);

void      switchChannel_init (TStadHandlesList *pStadHandles);

TI_STATUS switchChannel_SetDefaults (TI_HANDLE hSwitchChannel, SwitchChannelInitParams_t *SwitchChannelInitParams);

TI_STATUS switchChannel_stop(TI_HANDLE hSwitchChannel);

TI_STATUS switchChannel_start(TI_HANDLE hSwitchChannel);

TI_STATUS switchChannel_unload(TI_HANDLE hSwitchChannel);

void switchChannel_recvCmd(TI_HANDLE hSwitchChannel, dot11_CHANNEL_SWITCH_t *channelSwitch, TI_UINT8 channel);

void switchChannel_enableDisableSpectrumMngmt(TI_HANDLE hSwitchChannel, TI_BOOL enableDisable);



/* for debug */
typedef enum
{
	SC_SWITCH_CHANNEL_NUM,
	SC_SWITCH_CHANNEL_TBTT,
	SC_SWITCH_CHANNEL_MODE
} SC_switchChannelCmdParam_e;

#ifdef TI_DBG

void switchChannelDebug_switchChannelCmdTest(TI_HANDLE hSwitchChannel, TI_BOOL BeaconPacket);
void switchChannelDebug_CancelSwitchChannelCmdTest(TI_HANDLE hSwitchChannel, TI_BOOL BeaconPacket);
void switchChannelDebug_setChannelValidity(TI_HANDLE hSwitchChannel, TI_UINT8 channelNum, TI_BOOL validity);
void switchChannelDebug_printStatus(TI_HANDLE hSwitchChannel);
void switchChannelDebug_setCmdParams(TI_HANDLE hSwitchChannel, SC_switchChannelCmdParam_e switchChannelCmdParam, TI_UINT8 param);
void switchChannelDebug_SwitchChannelCmdTest(TI_HANDLE hSwitchChannel, TI_BOOL BeaconPacket);
#endif

#endif /* __SWITCH_CHANNEL_H__*/
