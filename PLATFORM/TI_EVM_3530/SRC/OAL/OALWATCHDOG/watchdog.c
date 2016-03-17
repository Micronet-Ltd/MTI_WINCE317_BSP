// Copyright (c) 2007, 2008 BSQUARE Corporation. All rights reserved.

#include <windows.h>
#include <oal.h>
#include <bsp.h>
#include <oal_prcm.h>
#include <bsp_cfg.h>

static BOOL g_WatchdogInit = FALSE;

//------------------------------------------------------------------------------
//
//  Global: g_pWatchogTimerRegs
//
//  This is global instance of watchdog timer registers
//
static OMAP_WDOG_REGS*      g_pWatchogTimerRegs;



static void WatchdogRefresh(void)
{
	if (g_WatchdogInit == FALSE)
	{
		// Initialize watchdog hardware
//    	g_pWatchogTimerRegs = OALPAtoUA(OMAP_WDOG2_REGS_PA);
		
		// Make sure interface/functional clocks are running
    	PrcmDeviceEnableClocks(OMAP_DEVICE_WDT2, TRUE);

		
		// Ensure the timer is stopped
		// Note - writes are posted; must ensure they have completed before 
		// writing to the same register again.
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_DISABLE_SEQ1);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_DISABLE_SEQ2);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		
    	// Set prescaler 
		OUTREG32(&g_pWatchogTimerRegs->WCLR, BSP_WATCHDOG_WCLR);
		
		// Set reload value in both the reload register and base counter register
		OUTREG32(&g_pWatchogTimerRegs->WLDR, BSP_WATCHDOG_WLDR);
		OUTREG32(&g_pWatchogTimerRegs->WCRR, BSP_WATCHDOG_WLDR);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		
		// Start the watchdog timer
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_ENABLE_SEQ1);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_ENABLE_SEQ2);
		
		g_WatchdogInit = TRUE;
	}
	
	// Refresh the watchdog timer
   	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
	OUTREG32(&g_pWatchogTimerRegs->WTGR, INREG32(&g_pWatchogTimerRegs->WTGR) + 1);
}

void OALWatchdogInit(OEMGLOBAL* pOemGlobal)
{
#if OAL_WATCHDOG_ENABLED
	// Initialize watchdog hardware
   	g_pWatchogTimerRegs = OALPAtoUA(OMAP_WDOG2_REGS_PA);
		
	pOemGlobal->pfnRefreshWatchDog = WatchdogRefresh;
	pOemGlobal->dwWatchDogPeriod = BSP_WATCHDOG_REFRESH_PERIOD_MILLISECONDS;
	pOemGlobal->dwWatchDogThreadPriority = BSP_WATCHDOG_THREAD_PRIORITY;
#endif
}

// Called from OEMPowerOff - no system calls, critical sections, OALMSG, etc., are allowed
//------------------------------------------------------------------------------
// WARNING: This function is called from OEMPowerOff - no system calls, critical 
// sections, OALMSG, etc., may be used by this function or any function that it calls.
//------------------------------------------------------------------------------
void OALWatchdogEnable(BOOL bEnable)
{
#if OAL_WATCHDOG_ENABLED
	if (bEnable == TRUE)
	{
    	// Enable clock
		PrcmDeviceEnableClocks(OMAP_DEVICE_WDT2, TRUE);
		
		// Refresh the watchdog timer
	   	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		OUTREG32(&g_pWatchogTimerRegs->WTGR, INREG32(&g_pWatchogTimerRegs->WTGR) + 1);
		
		// Start Watchdog
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_ENABLE_SEQ1);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_ENABLE_SEQ2);
	}
	else
	{
		// Ensure the timer is stopped
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_DISABLE_SEQ1);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		OUTREG32(&g_pWatchogTimerRegs->WSPR, WDOG_DISABLE_SEQ2);
    	while( INREG32(&g_pWatchogTimerRegs->WWPS) );
		
		// Disable clock
		PrcmDeviceEnableClocks(OMAP_DEVICE_WDT2, FALSE);
	}
#endif
}

