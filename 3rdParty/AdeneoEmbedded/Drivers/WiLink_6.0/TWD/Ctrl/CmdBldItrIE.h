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

#ifndef CMDBLDITRIE_H
#define CMDBLDITRIE_H


TI_STATUS cmdBld_ItrIeMemoryMap                	(TI_HANDLE hCmdBld, MemoryMap_t *apMap, void *fCb, TI_HANDLE hCb);
TI_STATUS cmdBld_ItrIeRoamimgStatisitics       	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void * pCb);
TI_STATUS cmdBld_ItrIeErrorCnt                 	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void * pCb);
TI_STATUS cmdBld_ItrIeRSSI                     	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void * pCb);
TI_STATUS cmdBld_ItrIeSg                       	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void * pCb);
TI_STATUS cmdBld_ItrIeStatistics               	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void *pCb);
TI_STATUS cmdBld_ItrIeDataFilterStatistics     	(TI_HANDLE hCmdBld, void * fCb, TI_HANDLE hCb, void *pCb);
TI_STATUS cmdBld_ItrIeMediumOccupancy          	(TI_HANDLE hCmdBld, TInterrogateCmdCbParams interogateCmdCBParams);
TI_STATUS cmdBld_ItrIeTfsDtim                  	(TI_HANDLE hCmdBld, TInterrogateCmdCbParams interogateCmdCBParams);
TI_STATUS cmdBld_ItrIeNoiseHistogramResults    	(TI_HANDLE hCmdBld, TInterrogateCmdCbParams noiseHistCBParams);
TI_STATUS cmdBld_ItrIePowerConsumptionstat      (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb);
TI_STATUS cmdBld_ItrIeRateParams                (TI_HANDLE hCmdBld, void *fCb, TI_HANDLE hCb, void* pCb);

#endif

