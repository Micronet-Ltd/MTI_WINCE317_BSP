// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
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

#include <bsp.h>
#include <omap_cpuver.h>

// The following constants are used to distinguish between ES2.0 and ES2.1 CPU 
// revisions.  This mechanism is necessary because the ID fuses are incorrectly
// set to ES2.0 in some ES2.1 devices.
#define PUBLIC_ROM_CRC_PA       0x14020
#define PUBLIC_ROM_CRC_ES2_0    0x5a540331
#define PUBLIC_ROM_CRC_ES2_1    0x6880d8d6

//------------------------------------------------------------------------------
//  External functions

VOID OEMDeinitDebugSerial();

extern DWORD GetCp15ControlRegister(void);
extern DWORD GetCp15AuxiliaryControlRegister(void);
extern DWORD OALGetL2Aux(void);

//------------------------------------------------------------------------------
//  Global FixUp variables
//
//  Note: This is workaround for makeimg limitation - no fixup on variables
//        initialized to zero, they must also be const.
//
const volatile DWORD dwOEMFailPowerPaging  = 1;
const volatile DWORD dwOEMDrWatsonSize     = 0x0004B000;
const volatile DWORD dwOEMTargetProject    = OEM_TARGET_PROJECT_CEBASE;
const volatile DWORD dwOEMHighSecurity     = OEM_HIGH_SECURITY_GP;

//-----------------------------------------------------------------------------
//
//  Global:  g_CpuFamily
//
//  Set during OEMInit to indicate CPU family.
//
    DWORD g_dwCpuFamily = CPU_FAMILY_37XX;
	
//-----------------------------------------------------------------------------
//
//  Global:  g_CpuFamily
//
//  Set during OEMInit to indicate CPU family.
//

DWORD g_dwCpuRevision = CPU_REVISION_UNKNOWN;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMSRAMStartOffset
//
//  offset to start of SRAM where SRAM routines will be copied to. 
//  Reinitialized in config.bib (FIXUPVAR)
//
DWORD dwOEMSRAMStartOffset = 0x00008000;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMVModeSetupTime
//
//  Setup time for DVS transitions. Reinitialized in config.bib (FIXUPVAR)
//
DWORD dwOEMVModeSetupTime = 2;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMPRCMCLKSSetupTime
//
//  Timethe PRCM waits for system clock stabilization. 
//  Reinitialized in config.bib (FIXUPVAR)
//
const volatile DWORD dwOEMPRCMCLKSSetupTime = 0x140;//0x2;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMMPUContextRestore
//
//  location to store context restore information from off mode (PA)
//
const volatile DWORD dwOEMMPUContextRestore = 0x84000100;

//------------------------------------------------------------------------------
//
//  Global:  dwOEMMaxIdlePeriod
//
//  maximum idle period during OS Idle in milliseconds
//
DWORD dwOEMMaxIdlePeriod = 1000;

//------------------------------------------------------------------------------

//extern DWORD gdwFailPowerPaging;
//extern DWORD cbNKPagingPoolSize;

//------------------------------------------------------------------------------
//
//  Global:  g_oalKitlEnabled
//
//  Save kitl state
//
//
DWORD g_oalKitlEnabled;
//------------------------------------------------------------------------------
const volatile DWORD dwDRAMEXT = 0x90000000;
const volatile DWORD dwEXTLEN  = 0x0E700000;
//const volatile DWORD dwEXTLEN  = 0x00000000;
BOOL OEMGetExtensionDRAM(LPDWORD lpMemStart, LPDWORD lpMemLen) 
{
	*lpMemStart = dwDRAMEXT;
	*lpMemLen	= dwEXTLEN;

	OALMSGS(1, (L"OEMGetExtensionDRAM: %d bytes from VA(%X)\r\n", dwEXTLEN, dwDRAMEXT));

	return 1;
}
#if (BSP_OPM_SELECT == A) || (BSP_CDS == 1)
	extern UINT32 g_oalIoCtlClockSpeed;
#endif
//-----------------------------------------------------------------------------
//
//  Global:  g_oalRetailMsgEnable
//
//  Used to enable retail messages
//
BOOL   g_oalRetailMsgEnable = FALSE;

//-----------------------------------------------------------------------------
//
//  Global:  g_ResumeRTC
//
//  Used to inform RTC code that a resume occured
//
BOOL g_ResumeRTC = FALSE;

//------------------------------------------------------------------------------
//
//  Function:  OEMInit
//
//  This is Windows CE OAL initialization function. It is called from kernel
//  after basic initialization is made.
//
VOID
OEMInit(
    )
{
    BOOL           *pColdBoot;
    BOOL           *pRetailMsgEnable;

    OALMSGS(1, (L"+OEMInit\r\n"));

    //----------------------------------------------------------------------
    // Initialize OAL log zones
    //----------------------------------------------------------------------

    OALLogSetZones( 
    //           (1<<OAL_LOG_VERBOSE)  |
    //           (1<<OAL_LOG_INFO)     |
               (1<<OAL_LOG_ERROR)    |
               (1<<OAL_LOG_WARN)     |
    //           (1<<OAL_LOG_IOCTL)    | 
    //           (1<<OAL_LOG_FUNC)     |
    //           (1<<OAL_LOG_INTR)     |
               0);


    //----------------------------------------------------------------------
    // Determion CPU revison
    //----------------------------------------------------------------------

	g_dwCpuRevision = Get_CPUVersion();
	g_dwCpuRevision = CPU_REVISION(g_dwCpuRevision);
	
    OALMSG(1, (L"OAL: CPU revision 0x%x\r\n", g_dwCpuRevision));

    OALMSG(1, (L"OAL: CPU L2 Aux register 0x%x\r\n", OALGetL2Aux()));
	
    //----------------------------------------------------------------------
    // Update platform specific variables
    //----------------------------------------------------------------------
#if (BSP_OPM_SELECT == A)
	{
		UINT32 *pOpm = OALPAtoUA(IMAGE_SHARE_OPM);

#if (BSP_CDS == 1)
		{
			UINT16*  CDSParams;
			CDSParams = OALArgsQuery(OAL_ARGS_QUERY_CDSPARAMS);
			*pOpm = CDSParams[5];
		}
#endif
		if(4 == *pOpm)
			g_oalIoCtlClockSpeed = BSP_SPEED_CPUMHZ_1000;
		else if(3 == *pOpm)
			g_oalIoCtlClockSpeed = BSP_SPEED_CPUMHZ_800;
		else
			g_oalIoCtlClockSpeed = BSP_SPEED_CPUMHZ_600;

		OALMSG(1, (L"OAL: MPU Speed %d MHz\r\n",g_oalIoCtlClockSpeed));
	}
#else
	OALMSG(1, (L"OAL: MPU Speed %d MHz\r\n",BSP_SPEED_CPUMHZ));
#endif

    //----------------------------------------------------------------------
    // Update kernel variables
    //----------------------------------------------------------------------

    dwNKDrWatsonSize = dwOEMDrWatsonSize;
  //  gdwFailPowerPaging = dwOEMFailPowerPaging;
  //  cbNKPagingPoolSize = (dwOEMPagingPoolSize == -1) ? 0 : dwOEMPagingPoolSize;
    
    // Alarm has resolution 10 seconds (actually has 1 second resolution, 
	// but setting alarm too close to suspend will cause problems).
    dwNKAlarmResolutionMSec = 10000;

    // Set extension functions
    pOEMIsProcessorFeaturePresent = OALIsProcessorFeaturePresent;
    pfnOEMSetMemoryAttributes     = OALSetMemoryAttributes;

    // Profiling support
    g_pOemGlobal->pfnProfileTimerEnable  = OEMProfileTimerEnable;
    g_pOemGlobal->pfnProfileTimerDisable = OEMProfileTimerDisable;
	g_pOemGlobal->pfnGetExtensionDRAM	 = OEMGetExtensionDRAM;
    //----------------------------------------------------------------------
    // Windows Mobile backward compatibility issue...
    //----------------------------------------------------------------------

    switch (dwOEMTargetProject)
        {
        case OEM_TARGET_PROJECT_SMARTFON:
        case OEM_TARGET_PROJECT_WPC:
		default:
			CEProcessorType = PROCESSOR_STRONGARM;
            break;
        }

    //----------------------------------------------------------------------
    // Initialize cache globals
    //----------------------------------------------------------------------

    OALCacheGlobalsInit();
	
    #if 1 //def DEBUG
        OALMSG(1, (L"CPU CP15 Control Register = 0x%x\r\n", GetCp15ControlRegister()));
        OALMSG(1, (L"CPU CP15 Auxiliary Control Register = 0x%x\r\n", GetCp15AuxiliaryControlRegister()));
    #endif
		
    //----------------------------------------------------------------------
    // Initialize Power Domains
    //----------------------------------------------------------------------
    
	OALMSG(0, (L"Init power\r\n"));
    OALPowerInit();

    //----------------------------------------------------------------------
    // Initialize Vector Floating Point co-processor
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init Vector Floating Point co-processor\r\n"));
    OALVFPInitialize(g_pOemGlobal);

    //----------------------------------------------------------------------
    // Initialize interrupt
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init interrupts\r\n"));
    if (!OALIntrInit())
        {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInit: failed to initialize interrupts\r\n"
            ));
        goto cleanUp;
        }

    //----------------------------------------------------------------------
    // Initialize system clock
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init timer\r\n"));
    if (!OALTimerInit(1, 0, 0))
        {
        OALMSG(OAL_ERROR, (
            L"ERROR: OEMInit: Failed to initialize system clock\r\n"
            ));
        goto cleanUp;
        }    

    //----------------------------------------------------------------------
    // Initialize SRAM Functions
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init SRAM funcions\r\n"));
    OALSRAMFnInit();

    //----------------------------------------------------------------------
    // Initialize high performance counter
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init performance timer\r\n"));
    OALPerformanceTimerInit(0, 0);

    //----------------------------------------------------------------------
    // Initialize modem
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init modem\r\n"));
    OALModemInit();

    //----------------------------------------------------------------------
    // Initialize the KITL
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init KITLE\r\n"));
    g_oalKitlEnabled = KITLIoctl(IOCTL_KITL_STARTUP, NULL, 0, NULL, 0, NULL);

    //----------------------------------------------------------------------
    // Initialize the watchdog
    //----------------------------------------------------------------------

	OALMSG(0, (L"Init WD\r\n"));
    OALWatchdogInit(g_pOemGlobal);

    //----------------------------------------------------------------------
    // Check for retail messages enabled
    //----------------------------------------------------------------------

#ifndef BSP_DEBUG_SERIAL
    pRetailMsgEnable = OALArgsQuery(OAL_ARGS_QUERY_OALFLAGS);
    if (pRetailMsgEnable && (*pRetailMsgEnable & OAL_ARGS_OALFLAGS_RETAILMSG_ENABLE))
        g_oalRetailMsgEnable = TRUE;

    //----------------------------------------------------------------------
    // Deinitialize serial debug
    //----------------------------------------------------------------------

    // Could make this conditional on an OALArgsQuery and add bootloader menu item to enable retail messages...
    if (!g_oalRetailMsgEnable)
        OEMDeinitDebugSerial();
#else
	g_oalRetailMsgEnable = TRUE;
#endif
// not available under CE6
#if 0
    //----------------------------------------------------------------------
    // Make Page Tables walk L2 cacheable. There are 2 new fields in OEMGLOBAL
    // that we need to update:
    // dwTTBRCacheBits - the bits to set for TTBR to change page table walk
    //                   to be L2 cacheable. (Cortex-A8 TRM, section 3.2.31)
    //                   Set this to be "Outer Write-Back, Write-Allocate".
    // dwPageTableCacheBits - bits to indicate cacheability to access Level
    //                   L2 page table. We need to set it to "inner no cache,
    //                   outer write-back, write-allocate. i.e.
    //                      TEX = 0b101, and C=B=0.
    //                   (ARM1176 TRM, section 6.11.2, figure 6.7, small (4k) page)
    //----------------------------------------------------------------------
    g_pOemGlobal->dwTTBRCacheBits = 0x8;            // TTBR RGN set to 0b01 - outer write back, write-allocate
    g_pOemGlobal->dwPageTableCacheBits = 0x140;     // Page table cacheability uses 1BB/AA format, where AA = 0b00 (inner non-cached)
#endif

    //----------------------------------------------------------------------
    // Check for a clean boot of device
    //----------------------------------------------------------------------

    pColdBoot = OALArgsQuery(OAL_ARGS_QUERY_COLDBOOT);
    if ((pColdBoot == NULL)|| ((pColdBoot != NULL) && *pColdBoot))
        NKForceCleanBoot();

cleanUp:
    OALMSG(1, (L"-OEMInit\r\n"));
}
//------------------------------------------------------------------------------
