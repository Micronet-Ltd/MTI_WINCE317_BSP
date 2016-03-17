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
//------------------------------------------------------------------------------
//
//  File:  map.c
//
//  This file contains DDK library implementation for IO space mapping
//  functions. For OMAP we choose to use one common mapping for all
//  on chip devices to reduce number of mapping (usually each device
//  will create its own mapping which cause unnecessary TLB missises).
//  
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <oalex.h>
#include <omap35xx.h>
#include "map.h"


//------------------------------------------------------------------------------
//
//  Global:  g_pRegs
//
//  This global variable contains a table containing the physical address,
//  length of the physical address, and virtual pointer of OMAP registers
//  All must be on a minimum of 1MB boundary
//
#pragma data_seg(".SHARED")

static MEMORY_REGISTER_ENTRY _pRegEntries[] = {
    { OMAP_CONFIG_REGS_PA,          OMAP_CONFIG_REGS_SIZE,      NULL },      // 0x48000000 - 16MB
    { OMAP_CTRL_REGS_PA,            OMAP_CTRL_REGS_SIZE,        NULL },      // 0x68000000 - 16MB
    { OMAP_SMS_REGS_PA,             OMAP_SMS_REGS_SIZE,         NULL },      // 0x6C000000 -  1MB
    { OMAP_SDRC_REGS_PA,            OMAP_SDRC_REGS_SIZE,        NULL },      // 0x6D000000 -  1MB
    { OMAP_GPMC_REGS_PA,            OMAP_GPMC_REGS_SIZE,        NULL },      // 0x6E000000 -  1MB
    { OMAP_PERI_CONFIG_REGS_PA,     OMAP_PERI_CONFIG_REGS_SIZE, NULL },      // 0x49000000 -  1MB
    { OMAP_SGX_REGS_PA,             OMAP_SGX_REGS_SIZE,         NULL },      // 0x50000000 -  1MB
    { OMAP_WKUP_CONFIG_REGS_PA,     OMAP_WKUP_CONFIG_REGS_SIZE, NULL},
    { 0,                            0,                          NULL}
};

#pragma data_seg()
#pragma comment(linker,"/section:.SHARED,RWS")


//------------------------------------------------------------------------------

VOID* 
MmMapIoSpace(
    PHYSICAL_ADDRESS PhysAddr, 
    ULONG size, 
    BOOLEAN cacheEnable
    )
{
    MEMORY_REGISTER_ENTRY *pRegEntry = _pRegEntries;
    VOID *pAddress = NULL;
    UINT64 phSource;
    UINT32 sourceSize, offset;
    BOOL rc;

    // Check if we can use common mapping for device registers (test is
    // simplified as long as we know that we support only 32 bit addressing).
    //
    if (cacheEnable == FALSE && PhysAddr.HighPart == 0)
        {
        while (pRegEntry->dwSize > 0)
            {
            if (pRegEntry->dwStart <= PhysAddr.LowPart &&
                (pRegEntry->dwStart + pRegEntry->dwSize) > PhysAddr.LowPart)
                {

                // check if memory is already mapped to the current process space
                rc = TRUE;
                if (pRegEntry->pv == NULL)
                    {
                    // reserve virtual memory and map it to a physical address
                    //
                    pRegEntry->pv = VirtualAlloc(
                                        0, pRegEntry->dwSize, MEM_RESERVE, 
                                        PAGE_NOACCESS
                                        );

                    if (pRegEntry->pv == NULL)
                        {
                        DEBUGMSG(TRUE, (
                            L"ERROR: MmMapIoSpace failed reserve registers memory\r\n"
                            ));
                        goto cleanUp;
                        }

                    rc = VirtualCopy(
                            pRegEntry->pv, (PVOID)(pRegEntry->dwStart >> 8), 
                            pRegEntry->dwSize, 
                            PAGE_PHYSICAL|PAGE_READWRITE|PAGE_NOCACHE
                            );
                    }
                
                if (!rc)
                    {
                    DEBUGMSG(TRUE, (
                        L"ERROR: MmMapIoSpace failed allocate registers memory\r\n"
                        ));
                    VirtualFree(pRegEntry->pv, 0, MEM_RELEASE);
                    pRegEntry->pv = NULL;
                    goto cleanUp;
                    }

                // Calculate offset
                if (pRegEntry->pv != NULL)
                    {
                    offset = PhysAddr.LowPart - pRegEntry->dwStart;
                    (UINT32)pAddress = (UINT32)pRegEntry->pv + offset;
                    }
                break;
                }

            // check next register map entry
            //
            ++pRegEntry;
            }
        }

    if (pAddress == NULL)
        {
        phSource = PhysAddr.QuadPart & ~(PAGE_SIZE - 1);
        sourceSize = size + (PhysAddr.LowPart & (PAGE_SIZE - 1));

        pAddress = VirtualAlloc(0, sourceSize, MEM_RESERVE, PAGE_NOACCESS);
        if (pAddress == NULL)
            {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace failed reserve memory\r\n"
                ));
            goto cleanUp;
            }   
        
        rc = VirtualCopy(
                pAddress, (PVOID)(phSource >> 8), sourceSize,
                PAGE_PHYSICAL|PAGE_READWRITE|(cacheEnable ? 0 : PAGE_NOCACHE)
                );

        if (!rc)
            {
            DEBUGMSG(TRUE, (
                L"ERROR: MmMapIoSpace failed allocate memory\r\n"
                ));
            VirtualFree(pAddress, 0, MEM_RELEASE);
            pAddress = NULL;
            goto cleanUp;
            }
        (UINT32)pAddress += PhysAddr.LowPart & (PAGE_SIZE - 1);
        }

cleanUp:
    return pAddress;
}


//------------------------------------------------------------------------------
//
//  Function:  MmUnmapIoSpace
//
//  Unmap a specified range of physical addresses previously mapped by
//  MmMapIoSpace
//
VOID
MmUnmapIoSpace(
    VOID *pAddress, 
    ULONG size
    )
{
    BOOL                   bFree = TRUE;
    MEMORY_REGISTER_ENTRY *pRegEntry = _pRegEntries;

    // We unmap only in case that memory wasn't mapped from common
    // mapping.
    //
    while (pRegEntry->dwSize > 0)
        {
        if (pRegEntry->pv &&
            (DWORD)pAddress >= (DWORD)pRegEntry->pv &&
            (DWORD)pAddress < ((DWORD)pRegEntry->pv + pRegEntry->dwSize))
            {
            bFree = FALSE;
            break;
            }
        ++pRegEntry;
        }

    if (bFree == TRUE)
        {
        VirtualFree(
            (VOID*)((UINT32)pAddress & ~(PAGE_SIZE - 1)), 0, MEM_RELEASE
            );
        }
}


//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL
TransBusAddrToVirtual(
    INTERFACE_TYPE ifcType,
    ULONG busNumber,
    PHYSICAL_ADDRESS busAddress,
    ULONG length, 
    ULONG *pAddressSpace,
    VOID **ppMappedAddress
    )
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS PhysAddr; 

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &PhysAddr
        ))
        {
        goto cleanUp;
        }

    switch (*pAddressSpace)
        {
        case 0:
            // Memory-mapped I/O, get virtual address for translated address
            *ppMappedAddress = MmMapIoSpace(PhysAddr, length, FALSE);
            if (*ppMappedAddress != NULL) rc = TRUE;
            break;
        case 1:        
            // I/O port
            *ppMappedAddress = (VOID*)PhysAddr.LowPart;
            rc = TRUE;
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  TransBusAddrToVirtual
//
BOOL
TransBusAddrToStatic(
    INTERFACE_TYPE ifcType,
    ULONG busNumber,
    PHYSICAL_ADDRESS busAddress,
    ULONG length,
    ULONG *pAddressSpace,
    VOID **ppAddress
    )
{
    BOOL rc = FALSE;
    PHYSICAL_ADDRESS PhysAddr;
    UINT64 address;
    UINT32 size;

    if (!HalTranslateBusAddress(
        ifcType, busNumber, busAddress, pAddressSpace, &PhysAddr
        ))
        {
        goto cleanUp;
        }
    switch (*pAddressSpace)
        {
        case 0:        
            // Memory-mapped I/O, get statically-mapped virtual address
            // for translated physical address
            address = PhysAddr.QuadPart & ~(PAGE_SIZE - 1);
            size = length + (PhysAddr.LowPart & (PAGE_SIZE - 1));
            *ppAddress = CreateStaticMapping((UINT32)(address >> 8), size);
            if (*ppAddress != NULL)
                {
                rc = TRUE;
                // Adjust with offset from page
                (UINT32)*ppAddress += PhysAddr.LowPart & (PAGE_SIZE - 1);
                }
            break;
        case 1:        
            // I/O port
            *ppAddress = (VOID*)PhysAddr.LowPart;
            rc = TRUE;
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------
