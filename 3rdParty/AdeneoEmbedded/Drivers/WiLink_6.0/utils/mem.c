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
#define __FILE_ID__  FILE_ID_129
#include "tidef.h"
#include "osApi.h"


#define MEM_BLOCK_START  (('m'<<24) | ('e'<<16) | ('m'<<8) | 's')
#define MEM_BLOCK_END    (('m'<<24) | ('e'<<16) | ('m'<<8) | 'e')


typedef struct
{
    TI_HANDLE hOs;
    TI_UINT32 uMaxAllocated;
    TI_UINT32 uCurAllocated;

} TMemMng;


typedef struct
{
    TI_UINT32 size;
    TI_UINT32 signature;

} TMemBlock;



/****************************************************************************************
 *                                                                                      *
 *                      OS Memory API                                                   *       
 *                                                                                      *
 ****************************************************************************************/
TI_HANDLE mem_Create (TI_HANDLE hOs)
{
    TMemMng *pMemMng = (TMemMng *)os_memoryAlloc (hOs, sizeof(TMemMng));

    if (pMemMng != NULL) 
    {
        pMemMng->hOs = hOs;
        pMemMng->uCurAllocated = 0;
        pMemMng->uMaxAllocated = 0;
    }

    return (TI_HANDLE)pMemMng;
}


TI_STATUS mem_Destroy (TI_HANDLE hMem)
{
    TMemMng *pMemMng = (TMemMng *)hMem;

    if (pMemMng != NULL) 
    {
        os_memoryFree (pMemMng->hOs, (void *)pMemMng, sizeof(TMemMng));
    }

    return TI_OK;
}



/****************************************************************************************
 *                        os_memoryAlloc()                                 
 ****************************************************************************************
DESCRIPTION:    Allocates resident (nonpaged) system-space memory.

ARGUMENTS:      OsContext   - our adapter context.
                Size        - Specifies the size, in bytes, to be allocated.

RETURN:         Pointer to the allocated memory.
                NULL if there is insufficient memory available.

NOTES:          With the call to vmalloc it is assumed that this function will
                never be called in an interrupt context. vmalloc has the potential to
                sleep the caller while waiting for memory to become available.

*****************************************************************************************/
void* mem_Alloc (TI_HANDLE hMem, TI_UINT32 size)
{
    TMemMng *pMemMng = (TMemMng *)hMem;
    TMemBlock *pMemBlock;
    TI_UINT32 total = size + sizeof(TMemBlock) + sizeof(TI_UINT32);

  #ifdef TI_MEM_ALLOC_TRACE
    os_printf ("mem_Alloc(0x%p, %lu) : %u\n", hMem, size, uTotalSize);
  #endif

    pMemBlock = (TMemBlock *) os_memoryAlloc (pMemMng->hOs, total);
    pMemBlock->size = size;
    pMemBlock->signature = MEM_BLOCK_START;
    *(TI_UINT32 *)((TI_UINT8 *)pMemBlock + total - sizeof(TI_UINT32)) = MEM_BLOCK_END;

    pMemMng->uCurAllocated += total;
    if (pMemMng->uMaxAllocated < pMemMng->uCurAllocated) 
    {
        pMemMng->uMaxAllocated = pMemMng->uCurAllocated;
    }

    return (void*)((TI_UINT8 *)pMemBlock + sizeof(TMemBlock));
}


/****************************************************************************************
 *                        os_memoryCAlloc()                                 
 ****************************************************************************************
DESCRIPTION:    Allocates an array in memory with elements initialized to 0.

ARGUMENTS:      OsContext   -   our adapter context.
                Number      -   Number of elements
                Size        -   Length in bytes of each element

RETURN:         None

NOTES:          
*****************************************************************************************/
void* mem_Calloc (TI_HANDLE hMem, TI_UINT32 number, TI_UINT32 size)
{
    TMemMng *pMemMng = (TMemMng *)hMem;
    void *ptr;
    TI_UINT32 total;

    total = number * size;

 #ifdef TI_MEM_ALLOC_TRACE
    os_printf ("os_memoryCAlloc(%u, %u) : %u\n", number, size, total);
 #endif

    ptr = mem_Alloc (hMem, total);

    if (ptr != NULL)
    {
        os_memorySet (pMemMng->hOs, ptr, 0, total);
    }

    return ptr;
}



/****************************************************************************************
 *                        os_memoryFree()                                 
 ****************************************************************************************
DESCRIPTION:    This function releases a block of memory previously allocated with the
                os_memoryAlloc function.


ARGUMENTS:      OsContext   -   our adapter context.
                pMemPtr     -   Pointer to the base virtual address of the allocated memory.
                                This address was returned by the os_memoryAlloc function.
                Size        -   Specifies the size, in bytes, of the memory block to be released.
                                This parameter must be identical to the Length that was passed to
                                os_memoryAlloc.

RETURN:         None

NOTES:          
*****************************************************************************************/
void mem_Free (TI_HANDLE hMem, void* ptr, TI_UINT32 size)
{
    TMemMng *pMemMng = (TMemMng *)hMem;
    TMemBlock *pMemBlock = (TMemBlock *)((TI_UINT8 *)ptr - sizeof(TMemBlock));
   
  #ifdef TI_MEM_ALLOC_TRACE
    os_printf ("os_memoryFree(%p, %u)\n", ptr, size);
  #endif

    if (pMemBlock->signature != MEM_BLOCK_START)
    {
      #ifdef TI_MEM_ALLOC_TRACE
        os_printf ("os_memoryFree: memory block signature is incorrect - 0x%x\n", pMemBlock->signature);
      #endif
    }

    else
    {
        *(TI_UINT8 *)(&pMemBlock->signature) = '~';
        if (*(TI_UINT32 *)((TI_UINT8 *)pMemBlock + pMemBlock->size + sizeof(TMemBlock)) != MEM_BLOCK_END)
        {
          #ifdef TI_MEM_ALLOC_TRACE
            os_printf ("os_memoryFree: memory block corruption, size - %u\n", pMemBlock->size);
          #endif
        }

        os_memoryFree (pMemMng->hOs, pMemBlock, pMemBlock->signature + sizeof(TMemBlock) + sizeof(TI_UINT32));

        pMemMng->uCurAllocated -= size + sizeof(TMemBlock) + sizeof(TI_UINT32);

        if ((int)pMemMng->uCurAllocated < 0) 
        {
          #ifdef TI_MEM_ALLOC_TRACE
            os_printf ("os_memoryFree: memory heap allocation calculation corrupted, size=%u, current=%u\n",
                       size, drv->cur_heap_bytes_allocated);
          #endif
            pMemMng->uCurAllocated = 0;
        }
    }
}
