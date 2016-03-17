/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  map.c
//
#include <eboot.h>

//------------------------------------------------------------------------------

void* 
OALPAtoVA(
    UINT32 pa, 
    BOOL cached
    )
{
    return (VOID*)pa;
}

//------------------------------------------------------------------------------

UINT32 
OALVAtoPA(
    VOID *va
    )
{
    return (UINT32)va;
}

//------------------------------------------------------------------------------

UINT32 BLVAtoPA(UINT32 address)
{
    OAL_ADDRESS_TABLE *pTable = g_oalAddressTable;
    UINT32 va = address;
    UINT32 pa = 0;

    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"+OALVAtoPA(0x%08x)\r\n", address));

    // Virtual address must be in CACHED or UNCACHED regions.
    if (va < 0x80000000 || va >= 0xC0000000) 
        {
        OALMSG(OAL_ERROR, (
            L"ERROR:OALVAtoPA: invalid virtual address 0x%08x\r\n", address
            ));
        goto cleanUp;
        }

    // Address must be cached, as entries in OEMAddressTable are cached address.
    va = va&~OAL_MEMORY_CACHE_BIT;

    // Search the table for address range
    while (pTable->size != 0) 
        {
        if (va >= pTable->CA && va <= pTable->CA + (pTable->size << 20) - 1) 
            {
            break;
            }
        pTable++;
        }

    // If address table entry is valid compute the PA
    if (pTable->size != 0) pa = pTable->PA + va - pTable->CA;

cleanUp:
    // Indicate physical address 
    OALMSG(OAL_MEMORY&&OAL_FUNC, (L"-OALVAtoPA(pa = 0x%x)\r\n", pa));
    return pa;
}