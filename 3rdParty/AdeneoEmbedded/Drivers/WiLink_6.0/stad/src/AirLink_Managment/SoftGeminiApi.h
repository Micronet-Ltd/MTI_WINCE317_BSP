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
/** \file SoftGeminiApi.h
 *  \brief BlueTooth-Wlan coexistence module interface header file
 *
 *  \see SoftGemini.c & SoftGemini.h
 */

/***************************************************************************/
/*																			*/
/*	  MODULE:	SoftGeminiApi.h												*/
/*    PURPOSE:	BlueTooth-Wlan coexistence module interface header file		*/
/*																			*/
/***************************************************************************/
#ifndef __SOFT_GEMINI_API_H__
#define __SOFT_GEMINI_API_H__

#include "paramOut.h"
#include "DrvMainModules.h"


/* The Soft-Gemini module API functions */
TI_HANDLE SoftGemini_create(TI_HANDLE hOs);
void      SoftGemini_init (TStadHandlesList *pStadHandles);
TI_STATUS SoftGemini_SetDefaults (TI_HANDLE hSoftGemini, SoftGeminiInitParams_t *pSoftGeminiInitParams);
TI_STATUS SoftGemini_destroy(TI_HANDLE hSoftGemini);
TI_STATUS SoftGemini_setParam(TI_HANDLE hSoftGemini, paramInfo_t *pParam);
TI_STATUS SoftGemini_getParam(TI_HANDLE hSoftGemini, paramInfo_t *pParam);
void      SoftGemini_printParams(TI_HANDLE hSoftGemini);
void      SoftGemini_SenseIndicationCB( TI_HANDLE hSoftGemini, char* str, TI_UINT32 strLen );
void      SoftGemini_ProtectiveIndicationCB( TI_HANDLE hSoftGemini, char* str, TI_UINT32 strLen );
void      SoftGemini_startPsPollFailure(TI_HANDLE hSoftGemini);
void      SoftGemini_endPsPollFailure(TI_HANDLE hSoftGemini);
void SoftGemini_SetPSmode(TI_HANDLE hSoftGemini);
void SoftGemini_unSetPSmode(TI_HANDLE hSoftGemini);
ESoftGeminiEnableModes SoftGemini_getSGMode(TI_HANDLE hSoftGemini);

#endif /* __SOFT_GEMINI_API_H__ */
