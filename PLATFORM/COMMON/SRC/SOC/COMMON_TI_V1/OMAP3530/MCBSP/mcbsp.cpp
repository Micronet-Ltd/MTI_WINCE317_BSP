// All rights reserved ADENEO EMBEDDED 2010

#include <windows.h>
#include <ceddk.h>

#include <omap.h>
#include <mcbsptypes.h>
#include "omap3530.h"

void 
SocMcbspDevConf(
		McBSPDevice_t* pDevice
	 )
{
	OMAP_SYSC_GENERAL_REGS* pSyscRegs = NULL;
	PHYSICAL_ADDRESS pa; 
	DWORD val		 = 0;
	UINT32 *pAddress = NULL;
	
	// Map System Control Registers
	pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
	pSyscRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(
        pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE
        );

	if (pSyscRegs == NULL)
	{
		return;
	}

    switch (pDevice->deviceID)
    {
        case OMAP_DEVICE_MCBSP1:
        case OMAP_DEVICE_MCBSP2:
            pAddress = (UINT32*)&pSyscRegs->CONTROL_DEVCONF0;
            break;

        case OMAP_DEVICE_MCBSP3:
        case OMAP_DEVICE_MCBSP4:
        case OMAP_DEVICE_MCBSP5:
            pAddress = (UINT32*)&pSyscRegs->CONTROL_DEVCONF1;
            break;

        default:
            goto cleanUp;
            break;
    }

    if (pDevice->clksPinSrc)
	{
        val = INREG32(pAddress);

        switch (pDevice->deviceID)
        {
        case OMAP_DEVICE_MCBSP1:
            val |= DEVCONF0_MCBSP1_CLKS;
            break;
        case OMAP_DEVICE_MCBSP2:
            val |= DEVCONF0_MCBSP2_CLKS;
            break;
        case OMAP_DEVICE_MCBSP3:
            val |= DEVCONF1_MCBSP3_CLKS;
            break;
        case OMAP_DEVICE_MCBSP4:
            val |= DEVCONF1_MCBSP4_CLKS;
            break;
        case OMAP_DEVICE_MCBSP5:
            val |= DEVCONF1_MCBSP5_CLKS;
            break;
        }

        OUTREG32(pAddress, val);
	}
    else
	{
        val = INREG32(pAddress);

        switch (pDevice->deviceID)
        {
        case OMAP_DEVICE_MCBSP1:
            val &= ~DEVCONF0_MCBSP1_CLKS;
            break;
        case OMAP_DEVICE_MCBSP2:
            val &= ~DEVCONF0_MCBSP2_CLKS;
            break;
        case OMAP_DEVICE_MCBSP3:
            val &= ~DEVCONF1_MCBSP3_CLKS;
            break;
        case OMAP_DEVICE_MCBSP4:
            val &= ~DEVCONF1_MCBSP4_CLKS;
            break;
        case OMAP_DEVICE_MCBSP5:
            val &= ~DEVCONF1_MCBSP5_CLKS;
            break;
        }

        OUTREG32(pAddress, val);
	}

cleanUp:

	MmUnmapIoSpace((PVOID)pSyscRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
}