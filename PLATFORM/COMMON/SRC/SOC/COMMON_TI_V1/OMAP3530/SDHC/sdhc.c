// All rights reserved ADENEO EMBEDDED 2010
// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

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
//  File: sdhc.c
//  SDHC controller SOC specific driver implementation

#pragma warning(push)
#pragma warning(disable: 4115 4201 4214)
#include <windows.h>
#include <ceddk.h>
#include <oal.h>
#include <oalex.h>
#include <omap.h>
#include "omap3530.h"
#include "omap_cpuver.h"
#pragma warning(pop)

#define DEFAULT_PBIAS_VALUE  (PBIASLITEVMODE0|PBIASLITEPWRDNZ0)

#ifdef BOOT_MODE
extern DWORD m_dwCPURev;
#endif

void SocSdhcDevconf(DWORD dwSlot)
{
    OMAP_SYSC_GENERAL_REGS* pSyscGeneralRegs = NULL;
	DWORD dwCPURev = (DWORD)CPU_REVISION_UNKNOWN;

#ifdef BOOT_MODE
	dwCPURev = m_dwCPURev;
    pSyscGeneralRegs = OALPAtoUA(OMAP_SYSC_GENERAL_REGS_PA);
#else
	
	PHYSICAL_ADDRESS pa;
	pa.QuadPart = (LONGLONG)OMAP_SYSC_GENERAL_REGS_PA;

	// Retrieve CPU revision
	KernelIoControl(IOCTL_HAL_GET_CPUREVISION, NULL, 0, &dwCPURev, sizeof(dwCPURev), NULL);

	// Map the System Control Module registers
	pSyscGeneralRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);
	if (pSyscGeneralRegs == NULL)
	{
		return;
	}
#endif
 
    // prepare power change
    if (dwSlot == MMCSLOT_1)
    {
        // Make sure VDDS stable bit is cleared before enabling the power for slot1
		CLRREG32(&pSyscGeneralRegs->CONTROL_PBIAS_LITE, (PBIASLITEVMODE0|PBIASLITEPWRDNZ0));
    } 
    else if (dwSlot == MMCSLOT_2)
    {
		CLRREG32(&pSyscGeneralRegs->CONTROL_PBIAS_LITE, PBIASLITEPWRDNZ0);
		CLRREG32(&pSyscGeneralRegs->CONTROL_DEVCONF1, DEVCONF1_MMCSDIO2ADPCLKISEL);
    }

    // post power change    
    if (dwSlot == MMCSLOT_1)
    {
        UINT32 dwPBiasValue = DEFAULT_PBIAS_VALUE;

        if (dwCPURev == 1)   // ES 1.0
		{
            dwPBiasValue = (PBIASLITEVMODE0|PBIASLITEPWRDNZ0);
		}
        else if (dwCPURev == 2) // ES 2.0
		{
            dwPBiasValue = PBIASLITEPWRDNZ0;
		}
        else if (dwCPURev == 3) // ES 2.1
		{
#ifdef MMCHS1_LOW_VOLTAGE
            dwPBiasValue = PBIASLITEPWRDNZ0;
#else
			dwPBiasValue = (PBIASLITEVMODE0|PBIASLITEPWRDNZ0);
#endif
		}
        else
		{
            dwPBiasValue = DEFAULT_PBIAS_VALUE;
		}

#ifdef BOOT_MODE
        OALStall(100 * 1000);
#else
		Sleep(100);	
#endif
        // Workaround to make the MMC slot 1 work
		SETREG32(&pSyscGeneralRegs->CONTROL_PBIAS_LITE, dwPBiasValue);
		SETREG32(&pSyscGeneralRegs->CONTROL_DEVCONF1, (1 << 24));	// Undocumented bit in the datasheet
    }
    else if (dwSlot == MMCSLOT_2)
    {
#ifdef BOOT_MODE
        OALStall(100 * 1000);
#else
		Sleep(100);	
#endif
		SETREG32(&pSyscGeneralRegs->CONTROL_PBIAS_LITE, PBIASLITEPWRDNZ0);
		SETREG32(&pSyscGeneralRegs->CONTROL_DEVCONF1, DEVCONF1_MMCSDIO2ADPCLKISEL);
    }

#ifndef BOOT_MODE
    MmUnmapIoSpace((PVOID)pSyscGeneralRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
#endif
}
