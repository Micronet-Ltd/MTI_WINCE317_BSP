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


#ifndef CMDINTERPRET_H
#define CMDINTERPRET_H

/*  Command interpreter header file */

TI_HANDLE cmdInterpret_Create (TI_HANDLE hOs);

TI_STATUS cmdInterpret_Destroy (TI_HANDLE hCmdInterpret, TI_HANDLE hEvHandler);

void cmdInterpret_Init (TI_HANDLE hCmdInterpret, TStadHandlesList *pStadHandles);

int cmdInterpret_convertAndExecute (TI_HANDLE hCmdInterpret, TConfigCommand *cmdObj);

int cmdInterpret_ServiceCompleteCB (TI_HANDLE hCmdInterpret, int status, void *buffer);

void cmdInterpret_convertToOs (TI_HANDLE hCmdInterpret, TConfigCommand *cmdPtr);

void *cmdInterpret_GetStat (TI_HANDLE hCmdInterpret);

#endif
