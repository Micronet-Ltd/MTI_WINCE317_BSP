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

/*--------------------------------------------------------------------------*/
/* Module:		TI_Results.h*/
/**/
/* Purpose:		*/
/**/
/*--------------------------------------------------------------------------*/
#ifndef _TI_RESULTS_H
#define _TI_RESULTS_H

#define TI_RESULT_UNBOUND               0x00000001

#define TI_RESULT_OK                0
#define TI_RESULT_FAILED            0xFFFFFFFF  /* -1*/
#define TI_RESULT_INVALIDE_HANDLE   0xFFFFFFFE  /* -2*/
#define TI_RESULT_SM_NOT_FOUND      0xFFFFFFFD  /* -3*/
#define TI_RESULT_INVALID_PARAMETER     0xFFFFFFFC  /* -4*/
#define TI_RESULT_REGISTRY_FAILED       0xFFFFFFFB  /* -5*/
#define TI_RESULT_NOT_ENOUGH_MEMORY     0xFFFFFFFA  /* -6*/
#define TI_RESULT_DRIVER_ERROR          0xFFFFFFF9  /* -7*/
#define TI_RESULT_IPC_ERROR             0xFFFFFFF8  /* -8*/


#define TI_SUCCEEDED(Status)    (Status == TI_RESULT_OK)
#define TI_FAILED(Status)       (Status != TI_RESULT_OK)

#endif

