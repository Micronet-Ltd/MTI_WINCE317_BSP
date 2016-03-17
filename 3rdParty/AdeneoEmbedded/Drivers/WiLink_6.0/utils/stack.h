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
/** \file stack.h
 *  \brief Seport module API
 *
 *  \see stack.c
 */

/***************************************************************************/
/*                                                                         */
/*    MODULE:   stack.h                                                    */
/*    PURPOSE:  Stack module internal header API                           */
/*                                                                         */
/***************************************************************************/


#ifndef STACK_H
#define STACK_H


typedef struct _Stack_t
{
    TI_HANDLE   hOs;
    void       *pBuf;
    unsigned    uPtr;
    unsigned    uDep;
    unsigned    uElemSize;
    TI_BOOL        bBuf;

    void      (*fCpy) (TI_HANDLE, void*, void*, unsigned);

} Stack_t;



unsigned stackInit    (Stack_t *pStack, TI_HANDLE hOs, unsigned uElemSize, unsigned uDep, void *pBuf, void (*fCpy) (TI_HANDLE, void*, void*, unsigned));
unsigned stackDestroy (Stack_t *pStack);
unsigned stackPush    (Stack_t *pStack, void *pVal);
unsigned stackPop     (Stack_t *pStack, void *pVal);


#endif
