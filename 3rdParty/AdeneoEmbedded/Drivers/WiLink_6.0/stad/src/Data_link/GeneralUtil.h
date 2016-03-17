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
#ifndef _GENERALUTIL_H_
#define _GENERALUTIL_H_


#include "tidef.h"
#include "GeneralUtilApi.h"




/*******************************************************************************************/
typedef struct
{
	TI_HANDLE			Container;
    TI_BOOL                Inuse; 
}ListElement_t;


typedef struct
{
    int                 CurrentIndex; 
    int                 MaxNumOfElements;      
    int                 ContainerSize;    
    ListElement_t	    *ElementList;
    TI_HANDLE           hOs; 
}List_t;


/*******************************************************************************************/


/*******************************************************************************************/
typedef struct{
    List_t *DistributionList;
    TI_HANDLE hOs; 
}DistributorMgr_t;   


typedef struct{
    TI_UINT16              Mask;
    TI_BOOL                HaltReq;
    GeneralEventCall_t  CallBack;
    TI_HANDLE           Context;
    TI_UINT32              Cookie;
}NotifReqElment_t;

/*******************************************************************************************/


#endif
