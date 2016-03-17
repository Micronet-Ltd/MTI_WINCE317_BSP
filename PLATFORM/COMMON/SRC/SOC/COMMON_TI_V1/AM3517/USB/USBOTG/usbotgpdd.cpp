// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) MPC Data Limited 2010.  All rights reserved.
//
//
// Module Name: 
//        usbotgpdd.cpp
//
// Abstract:
//        USB OTG Driver
//

#pragma warning(push)
#pragma warning(disable: 4100 4245 4512 6287 6258)
#include <windows.h>
#include <types.h>
#include <nkintr.h>

#include "am3517.h"
#include "am3517_usb.h"
#include "oal_clock.h"
#include "sdk_padcfg.h"
#include "usbotgpdd.h"
#pragma warning(pop)

//#ifdef DEBUG
LPCTSTR g_cppOtgStateString[]    = {
    TEXT("USBOTG_states_unknown"),
// A Port States
    TEXT("USBOTG_a_idle"),
    TEXT("USBOTG_a_wait_vrise"),
    TEXT("USBOTG_a_wait_bcon"),
    TEXT("USBOTG_a_host"),
    TEXT("USBOTG_a_suspend"),
    TEXT("USBOTG_a_peripheral"),
    TEXT("USBOTG_a_wait_vfall"),
    TEXT("USBOTG_a_vbus_err"),
// B Port States
    TEXT("USBOTG_b_idle"),
    TEXT("USBOTG_b_srp_init"),
    TEXT("USBOTG_b_peripheral"),
    TEXT("USBOTG_b_wait_acon"),
    TEXT("USBOTG_b_host")
    };
//#endif

//========================================================================
//!  \fn    ChipCfgLock(
//!           PSYSCONFREGS pSysConfRegs, BOOL lock
//!         )
//!  \brief Lock or unlock the Chip Cfg MMR registers.
//=======================================================================
void CAM35xOTG::ChipCfgLock(BOOL lock)
{
	UNREFERENCED_PARAMETER(lock);

	// Not available on AM3517
}

void CAM35xOTG::StartUSBModule(void)
{
    DWORD nPhyCtl = 0;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::StartUSBModule()\r\n")));

    nPhyCtl = m_pSysConfRegs->CONTROL_DEVCONF2;
        
	// Global Reset 
	SETREG32(&m_pSysConfRegs->CONTROL_IP_SW_RESET, USB20OTGSS_SW_RST);
	Sleep(10);
	CLRREG32(&m_pSysConfRegs->CONTROL_IP_SW_RESET, USB20OTGSS_SW_RST);

	// Reset the controller
	SETREG32(&m_pUsbRegs->CTRLR, OTG_CONTROL_RESET);

	Sleep(10);

    // Unlock USBPHY_CTL reg
    ChipCfgLock(FALSE);

    // Take Phy out of reset
    nPhyCtl &= ~(DEVCONF2_USBOTG_PHY_RESET);
    m_pSysConfRegs->CONTROL_DEVCONF2 = nPhyCtl;

    // configure normal USB operation
    nPhyCtl &= ~(DEVCONF2_USBOTG_OTGMODE);
        
    // Start on-chip PHY and PLL's
    nPhyCtl |= (DEVCONF2_USBOTG_PHY_PLLON    |
				DEVCONF2_USBOTG_SESSENDEN    |
				DEVCONF2_USBOTG_VBUSDETECTEN |
				DEVCONF2_USBOTG_DATAPOLARITY ) ;

    nPhyCtl &= ~(DEVCONF2_USBOTG_PHY_PD		 |
				 DEVCONF2_USBOTG_POWERDOWNOTG);

    m_pSysConfRegs->CONTROL_DEVCONF2 = nPhyCtl;
    Sleep(1);

    // wait until ready
    while ((m_pSysConfRegs->CONTROL_DEVCONF2 & DEVCONF2_USBOTG_PWR_CLKGOOD) != DEVCONF2_USBOTG_PWR_CLKGOOD)
        Sleep(5);

    // Lock USBPHY_CTL reg
    ChipCfgLock(TRUE);

    // Enable USB state polling
    m_bEnablePolling = TRUE;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::StartUSBModule()\r\n")));
}

CAM35xOTG::CAM35xOTG(LPCTSTR lpActivePath)
:   USBOTG(lpActivePath)
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::CAM35xOTG()\r\n")));

    m_hParent = CreateBusAccessHandle(lpActivePath);

    // Initialize hardware register pointers
    m_pUsbRegs = NULL;
    m_pSysConfRegs = NULL;
	m_bFunctionMode = FALSE;
	m_bHostMode = FALSE;

    // Initialize polling parameters
    m_hPollThreadExitEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    m_PollTimeout = INFINITE; // Initialise poll period to safe value

    if (lpActivePath) 
    {
        DWORD dwLength = _tcslen(lpActivePath) + 1;
        m_ActiveKeyPath = new TCHAR [ dwLength ] ;
        if (m_ActiveKeyPath)
            StringCchCopy( m_ActiveKeyPath, dwLength, lpActivePath );
    }

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::CAM35xOTG()\r\n")));
}

BOOL CAM35xOTG::Init()
{
    PHYSICAL_ADDRESS PA;
    BOOL rc = FALSE;
    DWORD dwType, dwLength;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::Init()\r\n")));

    // Map USB registers
    PA.QuadPart = AM3517_USB0_REGS_PA;
    m_pUsbRegs = (CSL_UsbRegs *)MmMapIoSpace (PA, sizeof(CSL_UsbRegs), FALSE);

    // Map System Configuration registers
    PA.QuadPart = (LONGLONG)OMAP_SYSC_GENERAL_REGS_PA;
    m_pSysConfRegs = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(PA, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

    // Check registers mapped OK
    if ((m_pUsbRegs == NULL)||
        (m_pSysConfRegs == NULL))
    {
        DEBUGMSG(ZONE_OTG_ERROR,(TEXT("CAM35xOTG::Init: ERROR: failed to map hardware registers\r\n")));
        goto cleanUp;
    }

    if ((m_ActiveKeyPath != NULL)
      &&(USBOTG::Init()))
    {
        // Create thread to poll USB status
        m_hPollThread = CreateThread(NULL, 0, PollThread, this, 0, NULL);
        if (m_hPollThread == NULL)
        {
            DEBUGMSG(ZONE_OTG_ERROR,(TEXT("CAM35xOTG::Init: ERROR: failed to create poll thread\r\n")));
            goto cleanUp;
        }

        // Determine if USB 1.1 is in use
		m_bUSB11Enabled = FALSE; // Not available on AM3517

        // Read poll timeout from registry
        if (!RegQueryValueEx((L"PollTimeout"),&dwType,(PBYTE)&m_PollTimeout,&dwLength)) 
        {
            DEBUGMSG(ZONE_OTG_ERROR, (TEXT("CAM35xOTG::Init: Failed to read PollTimeout from registry\r\n")));
            goto cleanUp;
        }
    }
    else
    {
        DEBUGMSG(ZONE_OTG_ERROR,(TEXT("CAM35xOTG::Init: ERROR: NULL ActiveKeyPath or base Init() failed\r\n")));
        goto cleanUp;
    }

	m_RequestedPowerState = D0;

	// Start the USB module
    StartUSBModule();

    rc = TRUE;

cleanUp:
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::Init(%d)\r\n"), rc));
    return rc;
}

BOOL CAM35xOTG::PostInit()
{
    BOOL rc = FALSE;
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::PostInit()\r\n")));

    // Default setting to be client mode
    m_UsbOtgState = USBOTG_b_idle;
    m_UsbOtgInput.bit.id = 1;
    rc = USBOTG::PostInit();

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::PostInit(%d)\r\n"), rc));
    return rc;
}

CAM35xOTG::~CAM35xOTG() 
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::~CAM35xOTG()\r\n")));

    // Terminate polling thread
    SetEvent(m_hPollThreadExitEvent);
    if(WaitForSingleObject(m_hPollThread, m_PollTimeout) == WAIT_TIMEOUT)
    {
        // force thread termination
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::~CAM35xOTG(): terminating poll thread...\r\n")));
#pragma warning(push)
#pragma warning(disable: 6258)
        TerminateThread(m_hPollThread, 0);
#pragma warning(push)
    }

    // Clean up event handle
    CloseHandle(m_hPollThreadExitEvent);
    CloseHandle(m_hPollThread);

    // Deallocate hardware register pointers
    if (m_pUsbRegs)
        MmUnmapIoSpace(m_pUsbRegs, sizeof(CSL_UsbRegs));
    if (m_pSysConfRegs)
        MmUnmapIoSpace(m_pSysConfRegs, sizeof(OMAP_SYSC_GENERAL_REGS));

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::~CAM35xOTG()\r\n")));
}

BOOL CAM35xOTG::PowerUp()
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::PowerUp()\r\n")));

	DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::PowerUp()\r\n")));
    return TRUE;
}

BOOL CAM35xOTG::PowerDown()
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::PowerDown()\r\n")));
	
	DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::PowerDown()\r\n")));
    return TRUE;
}
DWORD UnloadDrivers(CAM35xOTG* pOTG)
{
    pOTG->HostMode(FALSE);
    pOTG->FunctionMode(FALSE);
    return 0;
}

BOOL CAM35xOTG::UpdatePowerState()
{
    BOOL rc = FALSE;
	UINT32 nPhyCtl = 0;
    
	DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::UpdatePowerState()\r\n")));

    // Apply the requested power state
    switch(m_RequestedPowerState)
    {
    case D0:

	    // Global Reset 
		SETREG32(&m_pSysConfRegs->CONTROL_IP_SW_RESET, USB20OTGSS_SW_RST);
		Sleep(10);
		CLRREG32(&m_pSysConfRegs->CONTROL_IP_SW_RESET, USB20OTGSS_SW_RST);
     	
		// Enable Clocks
		EnableDeviceClocks(OMAP_DEVICE_HSOTGUSB, TRUE);

		// Request pads
		if (!RequestDevicePads(OMAP_DEVICE_HSOTGUSB))
		{
			ERRORMSG(ZONE_ERROR,
				(L" CAM35xOTG::PowerUp: RequestDevicePads failed\r\n"));
		}

	    // Reset the controller
		SETREG32(&m_pUsbRegs->CTRLR, OTG_CONTROL_RESET);

        Sleep(10);

        // Unlock USBPHY_CTL reg
        ChipCfgLock(FALSE);

		MASKREG32(&m_pSysConfRegs->CONTROL_DEVCONF2, DEVCONF2_USBOTG_REFFREQ, DEVCONF2_USBOTG_REFFREQ_13MHZ);

		SETREG32(&m_pSysConfRegs->CONTROL_DEVCONF2, DEVCONF2_USBOTG_PHY_PLLON		|
													DEVCONF2_USBOTG_SESSENDEN		|
													DEVCONF2_USBOTG_VBUSDETECTEN	|
													DEVCONF2_USBOTG_REFFREQ_13MHZ	|
													DEVCONF2_USBOTG_DATAPOLARITY	);

		CLRREG32(&m_pSysConfRegs->CONTROL_DEVCONF2, DEVCONF2_USBOTG_PHY_RESET		|
													DEVCONF2_USBOTG_OTGMODE			|
													DEVCONF2_USBOTG_POWERDOWNOTG	|
													DEVCONF2_USBPHY_GPIO_MODE		|
													DEVCONF2_USBOTG_PHY_PD			);
		Sleep(1);

        // wait until ready
        while ((m_pSysConfRegs->CONTROL_DEVCONF2 & DEVCONF2_USBOTG_PWR_CLKGOOD) != DEVCONF2_USBOTG_PWR_CLKGOOD)
            Sleep(5);

        // Lock USBPHY_CTL reg
        ChipCfgLock(TRUE);

        if(m_bFunctionMode && (m_pUsbRegs->DEVCTL & CSL_USB_DEVCTL_BDEVICE_MASK))
        {
            //SOFTCONN
            m_pUsbRegs->POWER |= CSL_USB_POWER_SOFTCONN_MASK;
        }

        // Enable USB state polling
        m_bEnablePolling = TRUE;

        rc = TRUE;
        break;
    case D3:
    case D4:

        // Disable USB state polling
        m_bEnablePolling = FALSE;

        if (WaitForSingleObject(
            CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)UnloadDrivers,this,0,NULL)
            ,2000) != WAIT_OBJECT_0)
        {
            RETAILMSG(1,(TEXT("driver unloading timed out. maybe because the suspend order came from the USB device (power key of a keybaord)\r\n")));
        }
        
        nPhyCtl = INREG32(&m_pSysConfRegs->CONTROL_DEVCONF2);

        // End the session for host        
        m_pUsbRegs->DEVCTL &= ~CSL_USB_DEVCTL_SESSION_MASK;

        // Disconnect peripheral
        m_pUsbRegs->POWER &= ~CSL_USB_POWER_SOFTCONN_MASK;
        

        // Unlock USBPHY_CTL reg
        ChipCfgLock(FALSE);

		// Only power down the USB2.0 PHY if USB1.1 PHY not in use
		if (!m_bUSB11Enabled)
		{
			nPhyCtl |= DEVCONF2_USBOTG_PHY_PD;
		}

		// Power down the OTG
		nPhyCtl |= DEVCONF2_USBOTG_POWERDOWNOTG;
		OUTREG32(&m_pSysConfRegs->CONTROL_DEVCONF2, nPhyCtl);

        // Lock USBPHY_CTL reg
        ChipCfgLock(TRUE);

		// Release pads
		if (!ReleaseDevicePads(OMAP_DEVICE_HSOTGUSB))
		{
			ERRORMSG(ZONE_ERROR,
				(L" CAM35xOTG::PowerDown: ReleaseDevicePads failed\r\n"));
		}

        // Clocks off (leave enabled if USB1.1 in use)
        if (!m_bUSB11Enabled)
			EnableDeviceClocks(OMAP_DEVICE_HSOTGUSB, FALSE);
        else
			EnableDeviceClocks(OMAP_DEVICE_HSOTGUSB, TRUE);


        rc = TRUE;    
        break;
    default:
        DEBUGMSG(ZONE_OTG_ERROR,(TEXT("COMAPL13xOTG::UpdatePowerState: do not support power state %d\r\n"), m_RequestedPowerState));
        break;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-COMAPL13xOTG::UpdatePowerState(%d)\r\n"), rc));

    return rc;
}

BOOL CAM35xOTG::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    BOOL bReturn = FALSE;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::IoControl(%d)\r\n"), dwCode));

    switch (dwCode) 
    {
        // Reimplemented IOCTL_POWER_SET in PDD layer, as the code
        // in the MDD has a bug (it always returns FALSE!)
        case IOCTL_POWER_SET:
            DEBUGMSG(ZONE_OTG_STATE,(TEXT("CAM35xOTG::IOControl: IOCTL_POWER_SET\r\n")));
            if (!pBufOut || dwLenOut < sizeof(CEDEVICE_POWER_STATE)) {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
            } else {
                m_RequestedPowerState = *(PCEDEVICE_POWER_STATE) pBufOut;
                m_IsThisPowerManaged = TRUE;
                DEBUGMSG(ZONE_OTG_STATE, (TEXT("CAM35xOTG::IOControl:IOCTL_POWER_SET: D%d\r\n"), m_RequestedPowerState));
                bReturn = UpdatePowerState();
                // did we set the device power?
                if (pdwActualOut) {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }
            }
            break;
        // Implemented IOCTL_POWER_GET in PDD layer, as the MDD
        // layer does not implement this IOCTL
        case IOCTL_POWER_GET: // gets the current device power state
            DEBUGMSG(ZONE_OTG_STATE,(TEXT("CAM35xOTG::IOControl: IOCTL_POWER_GET\r\n")));
            if (!pBufOut || dwLenOut < sizeof(CEDEVICE_POWER_STATE)) {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
            } else {
                *(PCEDEVICE_POWER_STATE) pBufOut = m_RequestedPowerState;
                if (pdwActualOut) {
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                }                
                bReturn = TRUE;
            }
            break;
        // Implemented IOCTL_BUS_SET_POWER_STATE in PDD layer, as the MDD
        // layer uses default bus driver with wrong parameters
		case IOCTL_BUS_SET_POWER_STATE:
			DEBUGMSG(ZONE_OTG_STATE,(TEXT("CAM35xOTG::IOControl: IOCTL_BUS_SET_POWER_STATE\r\n")));
            if ((pBufIn == NULL) || (dwLenIn < sizeof(CE_BUS_POWER_STATE))) {
                SetLastError(ERROR_INVALID_PARAMETER);
                bReturn = FALSE;
			} else {
				CE_BUS_POWER_STATE* state = (CE_BUS_POWER_STATE *)pBufIn;
				CEDDK_BUS_POWER_STATE ceddkPowerState;

				ceddkPowerState.DevicePowerState = *(state->lpceDevicePowerState);
				ceddkPowerState.lpReserved = state->lpReserved;

				bReturn = DefaultBusDriver::IOControl(IOCTL_BUS_SET_POWER_STATE, 
											(PBYTE)state->lpDeviceBusName, sizeof(state->lpDeviceBusName), 
											(PBYTE)&ceddkPowerState, sizeof(CEDDK_BUS_POWER_STATE),
											pdwActualOut) ;
			}

			break;
        default:    
            bReturn = USBOTG::IOControl(dwCode,pBufIn,dwLenIn,pBufOut,dwLenOut,pdwActualOut);
            break;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::IoControl(%d)\r\n"), bReturn));
    return bReturn;
}

// OTG PDD Functions
BOOL CAM35xOTG::SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus)
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::SessionRequest(%d,%d)\r\n"), fPulseLocConn, fPulseChrgVBus));

    if (fPulseLocConn || fPulseChrgVBus)
    {
        // Set session bit
        m_pUsbRegs->DEVCTL = BIT0;
    }
    else
    {
        // Clear session bit
        m_pUsbRegs->DEVCTL = 0;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::SessionRequest()\r\n")));
    return TRUE;
}

BOOL CAM35xOTG::NewStateAction(USBOTG_STATES usbOtgState , USBOTG_OUTPUT usbOtgOutput) 
{
	UNREFERENCED_PARAMETER(usbOtgOutput);

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+CAM35xOTG::NewStateAction()\r\n")));

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("USBOTG state: %s\n\r"), g_cppOtgStateString[usbOtgState]));

    // Load correct driver for current state
    switch(usbOtgState)
    {        
        case USBOTG_a_host:
        // in host mode
        if (m_UsbOtgInput.bit.b_conn != 0)
        {
            HostMode(TRUE);            
        }
        break;
    case USBOTG_b_peripheral:
        // in function mode
        FunctionMode(TRUE);
        break;
    case USBOTG_a_wait_vfall:
        // Clear session bit
        m_pUsbRegs->DEVCTL &= ~CSL_USB_DEVCTL_SESSION_MASK;
        // Deliberate fallthrough
    default:
        // In neither host nor function,
        // unload both drivers
        HostMode(FALSE);
        FunctionMode(FALSE);
        break;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::NewStateAction()\r\n")));

    // Always returning FALSE means the MDD progresses through the USBOTG state machine 
    // more slowly. This helps the case where a host cable is attached with no device.
    return FALSE;
}

BOOL CAM35xOTG::IsSE0()
{
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::IsSE0()\r\n")));
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::IsSE0()\r\n")));
    return FALSE;
}

BOOL CAM35xOTG::UpdateInput() 
{
    UINT8 devctl = 0;
    UINT8 vbus = 0;
    UINT32 cfgchip2 = 0;
    UINT32 statr = 0;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::UpdateInput()\r\n")));

    // read USB controller registers
    devctl = m_pUsbRegs->DEVCTL;
	cfgchip2 = m_pSysConfRegs->CONTROL_DEVCONF2;
    statr = m_pUsbRegs->STATR;
    DEBUGMSG(ZONE_OTG_FUNCTION,
        (TEXT("DEVCTL 0x%x, CFGCHIP2 0x%x, STATR 0x%x\r\n"), devctl, cfgchip2, statr));

    // fix some bits of MDD input to ensure
    // correct movement about the state machine
    m_UsbOtgInput.bit.a_bus_drop = 0;
    m_UsbOtgInput.bit.a_bus_req = 1;

    // check to see if a session is active
    if(devctl & CSL_USB_DEVCTL_SESSION_MASK)
    {
        // Session valid
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    Session Valid\r\n")));

        // Determine A or B device
        if((devctl & CSL_USB_DEVCTL_BDEVICE_MASK) == 0)
        {
            // 'A' device
            DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    'A' device\r\n")));
            m_UsbOtgInput.bit.id = 0;

            if((devctl & CSL_USB_DEVCTL_HOSTMODE_MASK) == 0)
            {
                // The controller has dropped host mode, 
                // so no device is attached
                m_UsbOtgInput.bit.b_conn = 0;
                DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    host mode OFF\r\n")));
           }
            else
            {
                // Operating as host, B device connected
                m_UsbOtgInput.bit.b_conn = 1;
                DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    host mode ON\r\n")));
            }
        }
        else
        {
            // 'B' device
            DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    'B' device\r\n")));
            m_UsbOtgInput.bit.id = 1;
        }
    }
    else
    {
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    Session End\r\n")));

        // no session active, set the session bit 
        // to identify USB mode
        m_pUsbRegs->DEVCTL |= CSL_USB_DEVCTL_SESSION_MASK;
        Sleep(100);
    }

    // Vbus
    m_UsbOtgInput.bit.a_vbus_vld = 0;
    m_UsbOtgInput.bit.b_sess_vld = 0;
    vbus = (devctl & CSL_USB_DEVCTL_VBUS_MASK) >> CSL_USB_DEVCTL_VBUS_SHIFT;
    switch(vbus)
    {
    case 0:
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    VBus: Below Session End\r\n")));
        break;
    case 1:
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    VBus: Above Session End, below AValid\r\n")));
        break;
    case 2:
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    VBus: Above AValid, below VBusValid\r\n")));
        break;
    case 3:
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    VBus: Above VBusValid\r\n")));
        // The bus voltage is good. 
        // In 'A' mode, this means the host is driving the bus OK (Vbus valid)
        // In 'B' mode, this means a connected device is driving the bus OK. 
        m_UsbOtgInput.bit.a_vbus_vld = 1;
        m_UsbOtgInput.bit.b_sess_vld = 1;
        break;
    default:
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("    VBus: ERROR, invalid state (%d)\r\n"), vbus));
        break;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, 
        (L"CAM35xOTG::UpdateInput:m_UsbOtgInput.ul=%x\r\n",m_UsbOtgInput.ul));
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::UpdateInput()\r\n")));
    return TRUE;
}

DWORD CAM35xOTG::PollThread(void *data)
{
    BOOL bTerminate = FALSE;
    CAM35xOTG *context = (CAM35xOTG *)data;
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("+CAM35xOTG::PollThread()\r\n")));
       
    if(context != NULL)
    {
        while (!bTerminate) 
        {
            switch (WaitForSingleObject(context->m_hPollThreadExitEvent, context->m_PollTimeout))
            {
            case WAIT_OBJECT_0:
                DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::PollThread(): Exit event signalled\r\n")));
                bTerminate = TRUE;
                break;
            case WAIT_TIMEOUT:
                DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::PollThread(): Polling...\r\n")));
                // Call MDD to query system state. 
                // (This is turn calls UpdateInput, which 
                // sets the bits of m_UsbOtgInput to expose
                // the USB state)
                if(context->m_bEnablePolling)
                {
                    context->UpdateInput();
                    context->EventNotification();
                }
                break;
            default:
                // error!
                bTerminate = TRUE;
                break;
            }
        }
    }
    else
        DEBUGMSG(ZONE_OTG_ERROR,(TEXT("CAM35xOTG::PollThread(): ERROR: invalid context parameter\r\n")));

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("-CAM35xOTG::PollThread()\r\n")));
    return 0;
}

VOID CAM35xOTG::HostMode(BOOL start)
{
    // only need to load/unload if required
    // state is different from current one
    if(m_bHostMode != start)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::HostMode(): %s host mode\r\n"), start? (L"entering"):(L"leaving")));
        RETAILMSG(TRUE,(TEXT("CAM35xOTG::HostMode(): %s host mode\r\n"), start? (L"entering"):(L"leaving")));
		m_bHostMode = start;
        LoadUnloadHCD(start);
    }
}

VOID CAM35xOTG::FunctionMode(BOOL start)
{    
    if (m_InFunctionModeFn)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::FunctionMode() recursion prevented\r\n")));
        return;
    }
    m_InFunctionModeFn = TRUE;
    // only need to load/unload if required
    // state is different from current one
    if(m_bFunctionMode != start)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("CAM35xOTG::FunctionMode(): %s function mode\r\n"), start? (L"entering"):(L"leaving")));
		RETAILMSG(TRUE,(TEXT("CAM35xOTG::FunctionMode(): %s function mode\r\n"), start? (L"entering"):(L"leaving")));
        m_bFunctionMode = start;
        LoadUnloadUSBFN(start);
    }
	m_InFunctionModeFn = FALSE;
}

// Class Factory.
USBOTG *CreateUSBOTGObject(LPTSTR lpActivePath)
{
    return new CAM35xOTG(lpActivePath);
}
void DeleteUSBOTGObject(USBOTG *pUsbOtg)
{
    if (pUsbOtg)
        delete pUsbOtg;
}

