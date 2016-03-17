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
/*																		   */
/*	  MODULE:	TrafficMonitor.h												       */
/*    PURPOSE:	TrafficMonitor module Header file		 							   */
/*																		   */
/***************************************************************************/
#ifndef _GENERALUTIL_API_H_
#define _GENERALUTIL_API_H_

#include "tidef.h"
#include "paramOut.h" /* check tis include*/
/**/
/* call back functions prototype.*/
/**/
typedef void (*GeneralEventCall_t)(TI_HANDLE Context,int EventCount,TI_UINT16 Mask,TI_UINT32 Cookie);
               

TI_HANDLE DistributorMgr_Create(TI_HANDLE hOs , int MaxNotifReqElment); 
TI_STATUS DistributorMgr_Destroy(TI_HANDLE hDistributorMgr);
TI_HANDLE DistributorMgr_Reg(TI_HANDLE hDistributorMgr,TI_UINT16 Mask,TI_HANDLE CallBack,TI_HANDLE Context,TI_UINT32 Cookie);
TI_STATUS DistributorMgr_ReReg(TI_HANDLE hDistributorMgr,TI_HANDLE ReqElmenth ,TI_UINT16 Mask,TI_HANDLE CallBack,TI_HANDLE Context,TI_UINT32 Cookie);
TI_STATUS DistributorMgr_AddToMask(TI_HANDLE hDistributorMgr,TI_HANDLE ReqElmenth,TI_UINT16 Mask);
TI_STATUS DistributorMgr_UnReg(TI_HANDLE hDistributorMgr,TI_HANDLE RegEventHandle);
void DistributorMgr_HaltNotif(TI_HANDLE ReqElmenth);
void DistributorMgr_RestartNotif(TI_HANDLE ReqElmenth);
void DistributorMgr_EventCall(TI_HANDLE hDistributorMgr,TI_UINT16 Mask,int EventCount); 


TI_HANDLE List_create(TI_HANDLE hOs,int MaxNumOfElements,int ContainerSize);
TI_STATUS List_Destroy(TI_HANDLE hList)	;
TI_HANDLE List_AllocElement(TI_HANDLE hList);
TI_STATUS List_FreeElement(TI_HANDLE hList,TI_HANDLE Container);
TI_HANDLE List_GetFirst(TI_HANDLE List);
TI_HANDLE List_GetNext(TI_HANDLE List);


#endif
