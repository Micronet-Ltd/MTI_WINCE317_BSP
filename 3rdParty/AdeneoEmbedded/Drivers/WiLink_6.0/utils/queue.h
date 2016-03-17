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


/** \file   queue.h 
 *  \brief  queue module header file.                                  
 *
 *  \see    queue.c
 */


#ifndef _QUEUE_H_
#define _QUEUE_H_


#define QUE_UNLIMITED_SIZE      0xFFFFFFFF

/* A queue node header structure */                        
typedef struct _TQueNodeHdr 
{
    struct _TQueNodeHdr *pNext;
    struct _TQueNodeHdr *pPrev;
} TQueNodeHdr;



/* External Functions Prototypes */
/* ============================= */
TI_HANDLE que_Create  (TI_HANDLE hOs, TI_HANDLE hReport, TI_UINT32 uLimit, TI_UINT32 uNodeHeaderOffset);
TI_STATUS que_Destroy (TI_HANDLE hQue);
TI_STATUS que_Enqueue (TI_HANDLE hQue, TI_HANDLE hItem);
TI_HANDLE que_Dequeue (TI_HANDLE hQue);
TI_STATUS que_Requeue (TI_HANDLE hQue, TI_HANDLE hItem);
TI_UINT32 que_Size    (TI_HANDLE hQue);

#ifdef TI_DBG
void      que_Print   (TI_HANDLE hQue);
#endif /* TI_DBG */



#endif  /* _QUEUE_H_ */
