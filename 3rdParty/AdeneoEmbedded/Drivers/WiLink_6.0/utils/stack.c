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
/** \file stack.c
 *  \brief Seport module API
 *
 *  \see stack.h
 */

/***************************************************************************/
/*                                                                         */
/*    MODULE:   stack.c                                                    */
/*    PURPOSE:  Stack module implementation                                */
/*                                                                         */
/***************************************************************************/


#define __FILE_ID__  FILE_ID_133
#include "tidef.h"
#include "osApi.h"
#include "stack.h"


/**
 * \date 30-May-2006\n
 * \brief initialize stack object
 *
 * Function Scope \e Public.\n
 * \param pStack    - pointer to the Stack_t structure\n
 * \param hOS       - handle to the OS object\n
 * \param uElemSize - size of a one stack element\n
 * \param uDep      - stack depth\n
 * \param pBuf      - pointer to the stack buffer; if NULL a memory for the stack buffer will be dynamically allocated\n
 * \param fCpy      - pointer to function copying the stack element; if NULL a default copy function will be used\n
 * \return 0 - on success, -1 - on failure\n
 */
unsigned stackInit 
(
    Stack_t   *pStack, 
    TI_HANDLE  hOs,
    unsigned   uElemSize, 
    unsigned   uDep, 
    void      *pBuf, 
    void     (*fCpy) (TI_HANDLE, void*, void*, unsigned)
)
{
    pStack->hOs       = hOs;
    pStack->uPtr      = 0;
    pStack->uElemSize = uElemSize;
    pStack->uDep      = uDep * uElemSize;

    if (pBuf)
    {
        pStack->pBuf  = pBuf;
        pStack->bBuf  = 0;
    }

    else
    {
        pStack->pBuf  = _os_memoryAlloc (hOs, pStack->uDep);
        pStack->bBuf  = TI_TRUE;
    }

    if (fCpy)
        pStack->fCpy  = fCpy;
    else
        pStack->fCpy  = os_memoryCopy; 

    return 0; 
}


/**
 * \date 30-May-2006\n
 * \brief destroy stack object
 *
 * Function Scope \e Public.\n
 * \param pStack    - pointer to the Stack_t structure\n
 * \return 0 - on success, -1 - on failure\n
 */
unsigned stackDestroy (Stack_t *pStack)
{
    if (pStack->bBuf)
        _os_memoryFree (pStack->hOs, pStack->pBuf, pStack->uDep);

    return 0;
}


/**
 * \date 30-May-2006\n
 * \brief destroy stack object
 *
 * Function Scope \e Public.\n
 * \param pStack    - pointer to the Stack_t structure\n
 * \param pVal      - the pointer to the pushed value\n
 * \return 0 - on success, -1 - on failure\n
 */
unsigned stackPush (Stack_t *pStack, void *pVal)
{
    if (pStack->uPtr < pStack->uDep)
    {
        pStack->fCpy (pStack->hOs, (unsigned char*)pStack->pBuf + pStack->uPtr, pVal, pStack->uElemSize);
        pStack->uPtr += pStack->uElemSize;

        return 0;
    }

    return -1;
}


/**
 * \date 30-May-2006\n
 * \brief destroy stack object
 *
 * Function Scope \e Public.\n
 * \param pStack    - pointer to the Stack_t structure\n
 * \param pVal      - the pointer to the popped value\n
 * \return 0 - on success, -1 - on failure\n
 */
unsigned stackPop (Stack_t *pStack, void *pVal)
{
    if (pStack->uPtr > 0)
    {
        pStack->uPtr -= pStack->uElemSize;
        pStack->fCpy (pStack->hOs, pVal, (unsigned char*)pStack->pBuf + pStack->uPtr, pStack->uElemSize);

        return 0;
    }

    return -1;
}
