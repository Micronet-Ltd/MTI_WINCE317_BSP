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
//  File:  bsp_bootstubs.c
//

// stub routines which exist in full images but not in bootloader images

//-----------------------------------------------------------------------------

#include <windows.h>
#include <bsp.h>

//-----------------------------------------------------------------------------
BOOL
INTERRUPTS_STATUS()
{
    return FALSE;
}

//-----------------------------------------------------------------------------
void 
WINAPI 
EnterCriticalSection(
    LPCRITICAL_SECTION lpcs
    )  
{
}

//-----------------------------------------------------------------------------
void 
WINAPI 
LeaveCriticalSection(
    LPCRITICAL_SECTION lpcs
    )  
{
}

//-----------------------------------------------------------------------------
void 
WINAPI 
InitializeCriticalSection(
    LPCRITICAL_SECTION lpcs
    )  
{
}
//-----------------------------------------------------------------------------
void WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection)
{}
//-----------------------------------------------------------------------------
HANDLE
WINAPI   
SC_CreateMutex(
	LPSECURITY_ATTRIBUTES lpsa, 
	BOOL bInitialOwner, 
	LPCTSTR lpName
	)
{
	return NULL;
}

//-----------------------------------------------------------------------------
DWORD
WINAPI   
SC_WaitForMultiple(
	DWORD cObjects, 
	CONST HANDLE *lphObjects, 
	BOOL fWaitAll, 
	DWORD dwTimeout
	)
{
	return 0;
}

//-----------------------------------------------------------------------------
BOOL
WINAPI
SC_ReleaseMutex(
	HANDLE hMutex)
{
	return TRUE;
}

//------------------------------------------------------------------------------
BOOL
PrcmDeviceGetContextState(
    UINT devId, 
    BOOL bSet
    )
{
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
PrcmDeviceEnableClocks(
    UINT devId, 
    BOOL bEnable
    )
{
    OMAP_CM_REGS* pCmRegs;

    switch (devId)
        {
/*
        case OMAP_DEVICE_GPTIMER1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_WKUP_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPT1);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPT1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPT1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_GPT1);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_GPT1);
                }
            break;
*/            
        case OMAP_DEVICE_I2C1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C1);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_I2C1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C1);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C1);
                }
                
            break;

        case OMAP_DEVICE_I2C2:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C2);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C2);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_I2C2);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C2);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C2);
                }
            break;

        case OMAP_DEVICE_I2C3:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C3);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C3);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_I2C3);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_I2C3);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_I2C3);
                }
            break;
		case OMAP_DEVICE_HDQ:
			OALMSG(0, (L"+PrcmDeviceEnableClocks : OMAP_DEVICE_HDQ() time = %x , enable = %x \r\n",OALGetTickCount(),bEnable));
			pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_HDQ);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_HDQ);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_HDQ);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_HDQ);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_HDQ);
                }
            break;

        case OMAP_DEVICE_MCSPI1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_MCSPI1);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_MCSPI1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_MCSPI1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_MCSPI1);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_MCSPI1);
                }
            break;
        case OMAP_DEVICE_MMC1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_MMC1);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_MMC1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_MMC1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_MMC1);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_MMC1);
                }
            break;
/*
        case OMAP_DEVICE_UART1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_UART1);
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_UART1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_UART1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN1_xxx, CM_CLKEN_UART1);
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_UART1);
                }
            break;

        case OMAP_DEVICE_UART3:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_UART3);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_UART3);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_UART3);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_UART3);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_UART3);
                }
            break;

        case OMAP_DEVICE_GPIO1:
            pCmRegs = OALPAtoUA(OMAP_PRCM_WKUP_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO1);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO1);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO1);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO1);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO1);
                }
            break;

        case OMAP_DEVICE_GPIO2:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO2);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO2);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO2);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO2);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO2);
                }
            break;

        case OMAP_DEVICE_GPIO3:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO3);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO3);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO3);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO3);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO3);
                }
            break;

        case OMAP_DEVICE_GPIO4:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO4);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO4);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO4);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO4);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO4);
                }
            break;

        case OMAP_DEVICE_GPIO5:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO5);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO5);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO5);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO5);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO5);
                }
            break;

        case OMAP_DEVICE_GPIO6:
            pCmRegs = OALPAtoUA(OMAP_PRCM_PER_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO6);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO6);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_GPIO6);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_GPIO6);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_GPIO6);
                }
            break;

        case OMAP_DEVICE_DSS:
            pCmRegs = OALPAtoUA(OMAP_PRCM_DSS_CM_REGS_PA);
            if (bEnable)
                {                
                SETREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_DSS1 | CM_CLKEN_DSS2 | CM_CLKEN_TV);
                SETREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_DSS);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_FCLKEN_xxx, CM_CLKEN_DSS1 | CM_CLKEN_DSS2);
                CLRREG32(&pCmRegs->CM_ICLKEN_xxx, CM_CLKEN_DSS);
                }
            break;

        case OMAP_DEVICE_HSOTGUSB:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_HSOTGUSB);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_HSOTGUSB_IDLE);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_HSOTGUSB);
                }
            break;

        case OMAP_DEVICE_D2D:
            pCmRegs = OALPAtoUA(OMAP_PRCM_CORE_CM_REGS_PA);
            if (bEnable)
                {
                SETREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_D2D);
                while (INREG32(&pCmRegs->CM_IDLEST1_xxx) & CM_IDLEST_ST_D2D);
                }
            else
                {
                CLRREG32(&pCmRegs->CM_ICLKEN1_xxx, CM_CLKEN_D2D);
                }
            break;
*/    
        }
    return TRUE;
}

//-----------------------------------------------------------------------------