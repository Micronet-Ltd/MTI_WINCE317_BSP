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

/***************************************************************************/
/*                                                                                                                                                 */
/*        MODULE:       TrafficMonitor.h                                                                           */
/*    PURPOSE:  TrafficMonitor module Header file                                              */
/*                                                                                                                                                 */
/***************************************************************************/
#ifndef _TRAFFIC_MONITOR_API_H
#define _TRAFFIC_MONITOR_API_H

#include "DataCtrl_Api.h"
#include "DrvMainModules.h"

/**/
/* call back functions prototype.*/
/**/
typedef void (*TraffEevntCall_t)(TI_HANDLE Context,TI_UINT32 Cookie);


/*
 *      This enum list all the available traffic monitor event 
 *  that a client can register to.
 */
typedef enum
{
    TX_ALL_MSDU_FRAMES =1,                          
    RX_ALL_MSDU_FRAMES,
    TX_RX_DIRECTED_FRAMES,
    TX_RX_ALL_MSDU_FRAMES,
    TX_RX_ALL_MSDU_IN_BYTES,
    TX_RX_DIRECTED_IN_BYTES,
    TX_RX_ALL_802_11_DATA_IN_BYTES,
    TX_RX_ALL_802_11_DATA_FRAMES
} TraffEvntOptNum_t;

 

typedef enum
{
    TRAFF_EDGE = 0,
    TRAFF_LEVEL 
}TraffTrigger_t;

typedef enum
{
    TRAFF_UP = 0,
    TRAFF_DOWN 
}TraffDirection_t;

typedef struct
{
    /*initial param*/
        TraffEevntCall_t    CallBack;
    TI_HANDLE           Context; 
    TI_UINT32              Cookie;    
    TraffDirection_t    Direction;
    TraffTrigger_t      Trigger;
    int                                 Threshold;
    TI_UINT32              TimeIntervalMs;
    TraffEvntOptNum_t   MonitorType;
}TrafficAlertRegParm_t;


/*********************************************************************/


TI_HANDLE TrafficMonitor_create(TI_HANDLE hOs);
void TrafficMonitor_Init (TStadHandlesList *pStadHandles, TI_UINT32 BWwindowMs);
TI_STATUS TrafficMonitor_Destroy(TI_HANDLE hTrafficMonitor);
TI_HANDLE TrafficMonitor_RegEvent(TI_HANDLE hTrafficMonitor, TrafficAlertRegParm_t *TrafficAlertRegParm, TI_BOOL AutoResetCreate);
TI_STATUS TrafficMonitor_SetRstCondition(TI_HANDLE hTrafficMonitor,TI_HANDLE EventHandle,TI_HANDLE ResetEventHandle,TI_BOOL MutualRst);
int TrafficMonitor_GetFrameBandwidth(TI_HANDLE hTrafficMonitor);
void TrafficMonitor_UnregEvent(TI_HANDLE hTrafficMonitor, TI_HANDLE EventHandle);
void TrafficMonitor_Event(TI_HANDLE hTrafficMonitor,int Count,TI_UINT16 Mask,TI_UINT32 MonitorModuleType);
void TrafficMonitor_StopEventNotif(TI_HANDLE hTrafficMonitor,TI_HANDLE EventHandle);
void TrafficMonitor_StartEventNotif(TI_HANDLE hTrafficMonitor,TI_HANDLE EventHandle);
void TrafficMonitor_ResetEvent(TI_HANDLE hTrafficMonitor, TI_HANDLE EventHandle);
TI_STATUS TrafficMonitor_Stop(TI_HANDLE hTrafficMonitor);       
TI_STATUS TrafficMonitor_Start(TI_HANDLE hTrafficMonitor);      
TI_BOOL TrafficMonitor_IsEventOn(TI_HANDLE EventHandle);




#endif


