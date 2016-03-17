// All rights reserved ADENEO EMBEDDED 2010

#include <windows.h>
#include <ceddk.h>

#include <omap.h>
#include <mcbsptypes.h>
#include "am3517.h"

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

	val = INREG32(pAddress);

    if (pDevice->clksPinSrc)
	{
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
	}
    else
	{
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
	}

	// Special case for McBSP1
	if (pDevice->deviceID == OMAP_DEVICE_MCBSP1)
	{
		if (pDevice->clockModeRx)
		{
			val |= DEVCONF0_MCBSP1_CLKR;			
		}
		else
		{
			val &= ~DEVCONF0_MCBSP1_CLKR;
		}

		if (pDevice->frameSyncSourceRx)
		{
			val |= DEVCONF0_MCBSP1_FSR;
		}
		else
		{
			val &= ~DEVCONF0_MCBSP1_FSR;
		}
	}

    OUTREG32(pAddress, val);

cleanUp:

	MmUnmapIoSpace((PVOID)pSyscRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
}

/*
{
	OMAP_SYSC_GENERAL_REGS* pSyscRegs = NULL;
	PHYSICAL_ADDRESS pa; 
	DWORD mask	= 0;
	DWORD val	= 0;
	
	UNREFERENCED_PARAMETER(bEnable);

	// On AM3517, only valid for MCBSP1 and MCBSP2
	if ((pDevice->deviceID != OMAP_DEVICE_MCBSP1) && 
		(pDevice->deviceID != OMAP_DEVICE_MCBSP2)  )
	{
		return;
	}

	// Map System Control Registers
	pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA;
	pSyscRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(
        pa, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE
        );

	if (pSyscRegs == NULL)
	{
		return;
	}

	switch(pDevice->deviceID)
	{
	case OMAP_DEVICE_MCBSP1:
		{
			if (pDevice->clockModeRx)
			{
				val |= DEVCONF0_MCBSP1_CLKR;
			}
			if (pDevice->frameSyncSourceRx)
			{
				val |= DEVCONF0_MCBSP1_FSR;
			}
			if (pDevice->pConfigInfo->SRGClkSrc == kMcBSP_SRG_SRC_CPU_CLK ||
				pDevice->pConfigInfo->SRGClkSrc == kMcBSP_SRG_SRC_CLKX_PIN)
			{
				val |= DEVCONF0_MCBSP1_CLKS;
			}

			mask = DEVCONF0_MCBSP1_CLKR | DEVCONF0_MCBSP1_FSR | DEVCONF0_MCBSP1_CLKS;
		}
		break;
	case OMAP_DEVICE_MCBSP2:
		{
			if (pDevice->pConfigInfo->SRGClkSrc == kMcBSP_SRG_SRC_CPU_CLK ||
				pDevice->pConfigInfo->SRGClkSrc == kMcBSP_SRG_SRC_CLKX_PIN)
			{
				val |= DEVCONF0_MCBSP2_CLKS;
			}

			mask = DEVCONF0_MCBSP2_CLKS;
		}
		break;
	default:
		{
			goto cleanUp;
		}
		break;
	}

	MASKREG32(&pSyscRegs->CONTROL_DEVCONF0, mask, val);

cleanUp:

	MmUnmapIoSpace((PVOID)pSyscRegs, sizeof(OMAP_SYSC_GENERAL_REGS));
}
*/

