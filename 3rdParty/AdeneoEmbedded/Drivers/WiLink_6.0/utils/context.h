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


/** \file   context.h 
 *  \brief  context module header file.                                  
 *
 *  \see    context.c
 */

#ifndef _CONTEXT_H_
#define _CONTEXT_H_



/* The callback function type for context clients */
typedef void (*TContextCbFunc)(TI_HANDLE hCbHndl);

/* The context init parameters */
typedef struct
{
    /* Indicate if the driver should switch to its own context or not before handling events */
    TI_BOOL   bContextSwitchRequired;  
} TContextInitParams;



/* External Functions Prototypes */
/* ============================= */
TI_HANDLE context_Create          (TI_HANDLE hOs);
TI_STATUS context_Destroy         (TI_HANDLE hContext);
void      context_Init            (TI_HANDLE hContext, TI_HANDLE hOs, TI_HANDLE hReport);
TI_STATUS context_SetDefaults     (TI_HANDLE hContext, TContextInitParams *pContextInitParams);

TI_UINT32 context_RegisterClient (TI_HANDLE       hContext,
                                  TContextCbFunc  fCbFunc,
                                  TI_HANDLE       hCbHndl,
                                  TI_BOOL         bEnable,
                                  char           *sName,
                                  TI_UINT32       uNameSize);

void      context_RequestSchedule (TI_HANDLE hContext, TI_UINT32 uClientId);
void      context_DriverTask      (TI_HANDLE hContext);
void      context_EnableClient    (TI_HANDLE hContext, TI_UINT32 uClientId);
void      context_DisableClient   (TI_HANDLE hContext, TI_UINT32 uClientId);

void      context_EnterCriticalSection (TI_HANDLE hContext);
void      context_LeaveCriticalSection (TI_HANDLE hContext);
void      context_DisableClient   (TI_HANDLE hContext, TI_UINT32 uClientId);
void      context_EnableClient    (TI_HANDLE hContext, TI_UINT32 uClientId);
#ifdef TI_DBG
void      context_Print           (TI_HANDLE hContext);
#endif /* TI_DBG */



#endif  /* _CONTEXT_H_ */


