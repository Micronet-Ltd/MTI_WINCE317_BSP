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
//  File: SDHC.CPP
//  SDHC controller driver implementation

#include "omap.h"
#include "am3517.h"

OMAP_SYSC_GENERAL_REGS  *g_pSysConfRegs = NULL;

static BOOL validate_pointer()
{
    if (g_pSysConfRegs == NULL)
    {
#ifndef BOOT_MODE
        PHYSICAL_ADDRESS pa;
        /* Map the sys config registers */
        pa.QuadPart = (LONGLONG)OMAP_SYSC_GENERAL_REGS_PA;
        g_pSysConfRegs = (OMAP_SYSC_GENERAL_REGS*) MmMapIoSpace(pa,
            sizeof (OMAP_SYSC_GENERAL_REGS),
            FALSE);
#else
		g_pSysConfRegs = OALPAtoUA(OMAP_SYSC_GENERAL_REGS_PA);
#endif
    }
    return (g_pSysConfRegs != NULL) ? TRUE : FALSE;
}

void SocResetEmac()
{
    if (validate_pointer())
    {
		g_pSysConfRegs->CONTROL_IP_SW_RESET |= CPGMACSS_SW_RST;

#ifndef BOOT_MODE
		{
			DWORD dwStart = 0;

			// Wait for a few milliseconds
			dwStart = GetTickCount();
			while((GetTickCount() - dwStart) < 10);
		}
#else
		OALStall(10);
#endif
        g_pSysConfRegs->CONTROL_IP_SW_RESET &= ~CPGMACSS_SW_RST;
    }
}


void SocAckInterrupt(DWORD flag)
{
    if (validate_pointer())
    {
        g_pSysConfRegs->CONTROL_LVL_INTR_CLEAR = flag & 0xF;
    }
}

