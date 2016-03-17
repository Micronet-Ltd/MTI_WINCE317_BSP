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

/** \file  osmemapi.c 
 *  \brief api to some memory handling functionw that are os dependent
 *
 *  \see   
 */

#include "tidef.h"
#include "osApi.h"
#include <ndis.h>

#define TIWLN_MEM_TAG						(*(TI_UINT32*)"LWIT")

/****************************************************************************************
 *                        																*
 *						OS Memory API													*       
 *																						*
 ****************************************************************************************/


/** 
 * \fn     os_memoryAlloc
 * \brief  Allocates resident (nonpaged) system-space memory.
 * 
 * \note  With the call to vmalloc it is assumed that this function will
 * \note  never be called in an interrupt context. vmalloc has the potential to
 * \note  sleep the caller while waiting for memory to become available.
 * \param  OsContext - a handle to the OS object
 * \param  Size	- Specifies the size, in bytes, to be allocated.
 * \return void* - Pointer to the allocated memory;  NULL if there is insufficient memory available. 
 */ 
void* os_memoryAlloc(TI_HANDLE OsContext, TI_UINT32 Size)
{
	PVOID pVirtualAddress;
	NDIS_STATUS Status;

	#if (TIWLN_MAJOR_VERSION >= 5)
		Status = NdisAllocateMemoryWithTag(&pVirtualAddress,Size, TIWLN_MEM_TAG);
	#else
		Status = NdisAllocateMemory(&pVirtualAddress, Size, 0, High);
	#endif

    if (Status != NDIS_STATUS_SUCCESS)
    {
		return NULL;
	}

	return pVirtualAddress;
}


/** 
 * \fn     os_memoryCAlloc
 * \brief  Allocates an array in memory with elements initialized to 0.
 * 
 * \param  OsContext - our adapter context.
 * \param  Number - Number of elements
 * \param  Size	- Length in bytes of each element
 * \return void* - Pointer to the allocated memory;  NULL if there is insufficient memory available. 
 */ 
void* os_memoryCAlloc(TI_HANDLE OsContext, TI_UINT32 Number, TI_UINT32 Size)
{
	PVOID pAllocatedMem;
	ULONG MemSize;

	MemSize = Number * Size;
	pAllocatedMem = os_memoryAlloc(OsContext, MemSize);

    if (!pAllocatedMem)
    {
		return NULL;
	}
   
	NdisZeroMemory(pAllocatedMem, MemSize);

	return pAllocatedMem;
}


/** 
 * \fn     os_memoryFree
 * \brief  This function releases a block of memory previously allocated with the os_memoryAlloc function.
 * 
 * \param  OsContext - our adapter context.
 * \param  pMemPtr - Pointer to the base virtual address of the allocated memory; This address was returned by the os_memoryAlloc function.
 * \param  Size - Specifies the size, in bytes, of the memory block to be released; This parameter must be identical to the Length that was passed to os_memoryAlloc.
 * \return void
 */ 
void os_memoryFree(TI_HANDLE OsContext, PVOID pMemPtr, UINT32 Size)
{
    NdisFreeMemory(pMemPtr, Size, 0);
}


/** 
 * \fn     os_memorySet
 * \brief  This function fills a block of memory with given value.
 * 
 * \param  OsContext - our adapter context.
 * \param  pMemPtr - Specifies the base address of a block of memory.
 * \param  Value - Specifies the value to set
 * \param  Length - Specifies the size, in bytes, to copy.
 * \return void
 */ 
void os_memorySet(TI_HANDLE OsContext, void* pMemPtr, TI_INT32 Value, TI_UINT32 Length)
{
   NdisFillMemory(pMemPtr, Length, Value);
}


/** 
 * \fn     os_memorySet
 * \brief  This function fills a block of memory with 0s.
 * 
 * \param  OsContext - our adapter context.
 * \param  pMemPtr - Specifies the base address of a block of memory.
 * \param  Length - Specifies how many bytes to fill with 0s.
 * \return void
 */ 
void os_memoryZero(TI_HANDLE OsContext, PVOID pMemPtr, TI_UINT32 Length)
{
    NdisZeroMemory(pMemPtr, Length);
}


/** 
 * \fn     os_memoryCopy
 * \brief  This function copies a specified number of bytes from one caller-supplied location to another.
 * 
 * \param  OsContext - our adapter context.
 * \param  pDstPtr - Destination buffer
 * \param  pSrcPtr - Source buffer
 * \param  Size - Specifies the size, in bytes, to copy.
 * \return void
 */ 
void os_memoryCopy(TI_HANDLE OsContext, void* pDstPtr, void* pSrcPtr, TI_UINT32 Size)
{
    NdisMoveMemory(pDstPtr, pSrcPtr, Size);
}


/** 
 * \fn     os_memoryCompare
 * \brief  This function copies a specified number of bytes from one caller-supplied location to another.
 * 
 * \param  OsContext - our adapter context.
 * \param  Buf1 - First buffer
 * \param  Buf2 - Second buffer
 * \param  Count - Number of characters
 * \return The return value indicates the relationship between the buffers:
 *         < 0 Buf1 less than Buf2
 *         0 Buf1 identical to Buf2
 *         > 0 Buf1 greater than Buf2
 */ 
TI_INT32 os_memoryCompare(TI_HANDLE OsContext, TI_UINT8* Buf1, TI_UINT8* Buf2, TI_INT32 Count)
{
    return memcmp(Buf1, Buf2, Count);
}


/** 
 * \fn     os_memoryCopyFromUser
 * \brief  This function copies a specified number of bytes from one caller-supplied location to another. 
 *
 * \note  source buffer is in USER-MODE address space
 * \param  OsContext - our adapter context.
 * \param  pDstPtr - Destination buffer
 * \param  pSrcPtr - Source buffer
 * \param  Size - Specifies the size, in bytes, to copy.
 * \return void
 */ 
int os_memoryCopyFromUser(TI_HANDLE OsContext, void* pDstPtr, void* pSrcPtr, TI_UINT32 Size)
{
	/* drivers in WinMobile ver<=6.0 are in USER-MODE so we don't need a different copy */
	os_memoryCopy(OsContext, pDstPtr, pSrcPtr, Size);

    return TI_OK;
}


/** 
 * \fn     os_memoryCopyToUser
 * \brief  This function copies a specified number of bytes from one caller-supplied location to another. 
 *
 * \note   desination buffer is in USER-MODE address space
 * \param  OsContext - our adapter context.
 * \param  pDstPtr - Destination buffer
 * \param  pSrcPtr - Source buffer
 * \param  Size - Specifies the size, in bytes, to copy.
 * \return void
 */ 
int
os_memoryCopyToUser(
    TI_HANDLE OsContext,
    void* pDstPtr,
    void* pSrcPtr,
    TI_UINT32 Size
    )
{
	/* drivers in WinMobile ver<=6.0 are in USER-MODE so we don't need a different copy */
   os_memoryCopy(OsContext, pDstPtr, pSrcPtr, Size);

   return TI_OK;
}

