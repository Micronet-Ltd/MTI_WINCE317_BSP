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


/** \file   timer.h 
 *  \brief  timer module header file.                                  
 *
 *  \see    timer.c
 */

#ifndef _TIMER_H_
#define _TIMER_H_


/* The callback function type for timer clients */
typedef void (*TTimerCbFunc)(TI_HANDLE hCbHndl, TI_BOOL bTwdInitOccured);


/* External Functions Prototypes */
/* ============================= */
TI_HANDLE tmr_Create (TI_HANDLE hOs);
TI_STATUS tmr_Destroy (TI_HANDLE hTimerModule);
TI_STATUS tmr_Free (TI_HANDLE hTimerModule);
void      tmr_ClearInitQueue (TI_HANDLE hTimerModule);
void      tmr_ClearOperQueue (TI_HANDLE hTimerModule);
void      tmr_Init (TI_HANDLE hTimerModule, TI_HANDLE hOs, TI_HANDLE hReport, TI_HANDLE hContext);
void      tmr_UpdateDriverState (TI_HANDLE hTimerModule, TI_BOOL bOperState);
TI_HANDLE tmr_CreateTimer (TI_HANDLE hTimerModule);
TI_STATUS tmr_DestroyTimer (TI_HANDLE hTimerInfo);
void      tmr_StartTimer (TI_HANDLE     hTimerInfo,
                          TTimerCbFunc  fExpiryCbFunc,
                          TI_HANDLE     hExpiryCbHndl,
                          TI_UINT32     uIntervalMsec,
                          TI_BOOL       bPeriodic);
void      tmr_StopTimer (TI_HANDLE hTimerInfo);
void      tmr_GetExpiry (TI_HANDLE hTimerInfo);
void      tmr_HandleExpiry (TI_HANDLE hTimerModule);

#ifdef TI_DBG
void      tmr_PrintModule (TI_HANDLE hTimerModule);
void      tmr_PrintTimer (TI_HANDLE hTimerInfo);
#endif /* TI_DBG */


#endif  /* _TIMER_H_ */


