
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <omap3430.h>

#include <bt_buffer.h>
#include <bt_hcip.h>
#include <bt_os.h>
#include <bt_debug.h>
#include <bt_tdbg.h>

static OMAP_GPIO_REGS * pGpio5 = NULL;
static OMAP_SYSC_PADCONFS_REGS * pPadConfig = NULL;

int PDD_StartHardware(void)
{
	/*
	PHYSICAL_ADDRESS pa;
	
    if (pPadConfig == NULL) {
        pa.QuadPart = OMAP2430_SYSC1_PADCONFS1_REGS_PA;
		pPadConfig = (OMAP2430_SYSC_PADCONFS1_REGS *)MmMapIoSpace(pa, sizeof(OMAP2430_SYSC_PADCONFS1_REGS), FALSE);
		if(pPadConfig == NULL) {
                RETAILMSG(1, (L" OMAP2430 BRF6300! OMAP2430_PADCONFS1_REGS:"
                    L"Failed to map PADCONFS1 registers (pa = 0x%08x)\r\n", pa.LowPart
                ));
                return FALSE;   
            }
    }
    MASKREG32(&(pPadConfig->ulCONTROL_SYS_CLKREQ), BIT28|BIT27|BIT26|BIT25|BIT24, BIT27); // Mode 0, Pulldown, enabled
    
    if (pGpio5 == NULL) {
        pa.QuadPart = OMAP2430_GPIO5_REGS_PA;
		pGpio5 = (OMAP_GPIO_REGS *)MmMapIoSpace(pa, sizeof(OMAP_GPIO_REGS), FALSE);
		if(pGpio5 == NULL) {
                RETAILMSG(1, (L" OMAP2430 BRF6300! OMAP2430_GPIO5_REGS:"
                    L"Failed to map GPIO5 registers (pa = 0x%08x)\r\n", pa.LowPart
                ));
                return FALSE;   
            }
    }
    
    // Reset the hardware
    //Peter, OMAP2430 GPIO133 is used as toggle pin
    CLRREG32(&pGpio5->OE, (1<<5));				//output
    SETREG32(&pGpio5->CLEARDATAOUT, (1<<5));	//output low
    StallExecution(10000);
    SETREG32(&pGpio5->SETDATAOUT, (1<<5));		//output high
    //StallExecution(10000); 
    */ 
    return TRUE;
}

int PDD_StopHardware(void)
{
	/*
    if (pGpio5) {
        CLRREG32(&pGpio5->OE, (1<<5));
        SETREG32(&pGpio5->CLEARDATAOUT, (1<<5));      
    }
    if(pGpio5) MmUnmapIoSpace((VOID *)pGpio5, sizeof(OMAP_GPIO_REGS));

    if(pPadConfig) MmUnmapIoSpace((VOID *)pPadConfig, sizeof(OMAP2430_SYSC_PADCONFS1_REGS));
    */
    return TRUE;
}

