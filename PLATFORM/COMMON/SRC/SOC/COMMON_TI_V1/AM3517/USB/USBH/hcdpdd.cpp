// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) MPC Data Limited 2007.  All rights reserved.
//
//------------------------------------------------------------------------------
//
//  File:  hcd_pdd.cpp
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright 1999,2000 by Texas Instruments Incorporated. All rights reserved.
// Property of Texas Instruments Incorporated. Restricted rights to use,
// duplicate or disclose this code are granted through contract.
//
////////////////////////////////////////////////////////////////////////////////
//
// Platform dependant part of the USB Open Host Controller Driver (OHCD).
//
// Functions:
//
//  HcdPdd_DllMain
//  HcdPdd_Init
//  HcdPdd_CheckConfigPower
//  HcdPdd_PowerUp
//  HcdPdd_PowerDown
//  HcdPdd_Deinit
//  HcdPdd_Open
//  HcdPdd_Close
//  HcdPdd_Read
//  HcdPdd_Write
//  HcdPdd_Seek
//  HcdPdd_IOControl
//
////////////////////////////////////////////////////////////////////////////////

#pragma warning(push)
#pragma warning(disable: 4510 4512 4610)
#include <windows.h>
#include <types.h>
#include <memory.h>
#include <ceddk.h>
#include <ohcdddsi.h>
#include <nkintr.h>

#include "am3517.h"
#include "am3517_usb.h"
#include "oal_clock.h"
#include "sdk_padcfg.h"
#include "drvcommon.h"
#include "testmode.h"
#pragma warning(pop)

#pragma warning(disable:4068 6320)

/* Lock or unlock the Chip Cfg MMR registers. */
void ChipCfgLock(OMAP_SYSC_GENERAL_REGS* pSysConfRegs, BOOL lock)
{
	UNREFERENCED_PARAMETER(lock);
	UNREFERENCED_PARAMETER(pSysConfRegs);

	// Not available on AM3517

	/*
    if (lock)
    {
        pSysConfRegs->KICK0R = BOOT_CFG_KICK0_LOCK;
        pSysConfRegs->KICK1R = BOOT_CFG_KICK1_LOCK;
    }
    else
    {
        pSysConfRegs->KICK0R = BOOT_CFG_KICK0_UNLOCK;
        pSysConfRegs->KICK1R = BOOT_CFG_KICK1_UNLOCK;
    }
	*/
}

/* HcdPdd_DllMain
 *
 *  DLL Entry point.
 *
 * Return Value:
 */

extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DEBUGMSG(ZONE_INIT, (TEXT("USBH: HcdPdd_DllMain DLL_PROCESS_ATTACH\r\n")));
        break;
    case DLL_PROCESS_DETACH:
        DEBUGMSG(ZONE_INIT, (TEXT("USBH: HcdPdd_DllMain DLL_PROCESS_DETACH\r\n")));
        break;
    }

    return TRUE;
}

 /* HcdPdd_SetDevicePower
 *   Updates device power state according the
 *   SOhcdPdd::CurrentDx field.
 *
 * Return Value:
 *   Win error code.
 */

DWORD HcdPdd_SetDevicePower(SOhcdPdd *pPddObject)
{
//    DWORD nPhyCtl;
	DWORD dwRegVal = 0;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_SetDevicePower: State %d\r\n"),
        pPddObject->CurrentDx));

	if (!pPddObject)
        return ERROR_INVALID_PARAMETER;

    if(pPddObject->dwDisablePowerManagement != 0)
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_SetDevicePower: Power management disabled\r\n")));
        return ERROR_SUCCESS;
    }

    switch (pPddObject->CurrentDx)
    {
    case D0: // Power on

        // Determine if USB 1.1 is in use
        pPddObject->fUSB11Enabled = FALSE;
		/*
        pPddObject->fUSB11Enabled =
            (pPddObject->pSysConfReg->CFGCHIP2 & CFGCHIP2_USB1SUSPENDM) ? TRUE : FALSE;

		// Enable Clocks
		EnableDeviceClocks(OMAP_DEVICE_HSOTGUSB, TRUE);

		// Request pads
		RequestDevicePads(OMAP_DEVICE_HSOTGUSB);

        // Unlock USBPHY_CTL reg
        ChipCfgLock(pPddObject->pSysConfReg, FALSE);

		nPhyCtl = pPddObject->pSysConfReg->CONTROL_DEVCONF2;
        DEBUGMSG(0, (L"UsbPowerModule: 1 CFGCHIP2 = 0x%08X\r\n", nPhyCtl));

        // take Phy out of reset
        nPhyCtl &= ~(DEVCONF2_USBOTG_PHY_RESET);
        pPddObject->pSysConfReg->CONTROL_DEVCONF2 = nPhyCtl;

        // start on-chip PHY and PLL's
        nPhyCtl |= (DEVCONF2_USBOTG_PHY_PLLON	 |
                    DEVCONF2_USBOTG_SESSENDEN	 |
                    DEVCONF2_USBOTG_VBUSDETECTEN |
                    DEVCONF2_USBOTG_DATAPOLARITY |
					DEVCONF2_USBOTG_REFFREQ_13MHZ|
                    DEVCONF2_USBOTG_OTGMODE_HOST );
        nPhyCtl &= ~(DEVCONF2_USBOTG_PHY_PD		 |
                     DEVCONF2_USBOTG_POWERDOWNOTG);
        pPddObject->pSysConfReg->CONTROL_DEVCONF2 = nPhyCtl;
        Sleep(1);

        // wait until ready
        while ((pPddObject->pSysConfReg->CONTROL_DEVCONF2 & DEVCONF2_USBOTG_PWR_CLKGOOD) != DEVCONF2_USBOTG_PWR_CLKGOOD)
            Sleep(5);

        // Lock USBPHY_CTL reg
        ChipCfgLock(pPddObject->pSysConfReg, TRUE);
*/
        dwRegVal = READ_PORT_ULONG(pPddObject->ioPortBase+USB_DEVCTL_REG_OFFSET);
        dwRegVal |= BIT0;
        WRITE_PORT_ULONG(pPddObject->ioPortBase+USB_DEVCTL_REG_OFFSET, dwRegVal);

        DEBUGMSG(0, (L"UsbPowerModule: 2 CFGCHIP2 = 0x%08X\r\n", pPddObject->pSysConfReg->CONTROL_DEVCONF2));
        break;

    case D3: // Sleep
    case D4: // Power off

        dwRegVal = READ_PORT_ULONG(pPddObject->ioPortBase+USB_DEVCTL_REG_OFFSET);
        dwRegVal &= ~BIT0;
        WRITE_PORT_ULONG(pPddObject->ioPortBase+USB_DEVCTL_REG_OFFSET, dwRegVal);

/*
        // Unlock USBPHY_CTL reg
        ChipCfgLock(pPddObject->pSysConfReg, FALSE);

        // Power down the PHY
        nPhyCtl = pPddObject->pSysConfReg->CONTROL_DEVCONF2;

        // Only power down the USB2.0 PHY if USB1.1 PHY not in use
        if (!pPddObject->fUSB11Enabled)
        {
            nPhyCtl |= DEVCONF2_USBOTG_PHY_PD;
        }

        nPhyCtl |= DEVCONF2_USBOTG_POWERDOWNOTG;
        pPddObject->pSysConfReg->CONTROL_DEVCONF2 = nPhyCtl;

        // Lock USBPHY_CTL reg
        ChipCfgLock(pPddObject->pSysConfReg, TRUE);

		// Disable Clocks
		EnableDeviceClocks(OMAP_DEVICE_HSOTGUSB, FALSE);

		// Release pads
		ReleaseDevicePads(OMAP_DEVICE_HSOTGUSB);
*/
        break;

    default:

        return ERROR_INVALID_PARAMETER;
    }

    return ERROR_SUCCESS;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure OHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD HcdPdd_Init(DWORD dwContext)
{
    SOhcdPdd *pPddObject = NULL;
    BOOL fRet = FALSE;
    PHYSICAL_ADDRESS PhysAddr = {0};
    DWORD IrqVal = IRQ_USB0_INT;
    DWORD BytesRet;
    HKEY hKey = NULL;
 
    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH:+HcdPdd_Init:\r\n")));

    // Initialise platform PDD
    if (!USBHPDD_Init())
    {
        DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Init: Failed to initialise platform USBH PDD\r\n"));
        goto _clean;
    }
    // Start with VBUS power off
    USBHPDD_PowerVBUS(FALSE);

    // PDD context
    pPddObject = new SOhcdPdd;

    if (!pPddObject)
    {
        DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Init: Failed to alloc PDD context!\r\n"));
        goto _clean;
    }

    memset(pPddObject, 0, sizeof(*pPddObject));

    // Map registers

    // map the HC register space to VM
    PhysAddr.LowPart = AM3517_USB0_REGS_PA;
    pPddObject->ioPortBase = (PUCHAR) MmMapIoSpace (PhysAddr, sizeof(CSL_UsbRegs), FALSE);

    // Map System Configuration register space to VM
    PhysAddr.LowPart = OMAP_SYSC_GENERAL_REGS_PA;
    pPddObject->pSysConfReg = (OMAP_SYSC_GENERAL_REGS*)MmMapIoSpace(PhysAddr, sizeof(OMAP_SYSC_GENERAL_REGS), FALSE);

    // Map the CPPI register space to VM
    PhysAddr.LowPart = AM3517_CPPI_REGS_PA;
    pPddObject->ioCppiBase = (PUCHAR) MmMapIoSpace(PhysAddr, sizeof(CSL_CppiRegs), FALSE);

    if ((NULL == pPddObject->ioPortBase) ||
        (NULL == pPddObject->ioCppiBase) ||
        (NULL == pPddObject->pSysConfReg))
    {
        DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Init: Failed to map registers!\r\n"));
        goto _clean;
    }

    // Request SYSINTR ID
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,  &IrqVal, sizeof(DWORD),
            &pPddObject->dwSysIntr, sizeof(DWORD), &BytesRet))
    {
        DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Init: Failed to request SYSINTR for IRQ%d!\r\n",
            IrqVal));
        goto _clean;
    }

    // Save reg path
    _tcsncpy(pPddObject->szDriverRegKey, (LPCTSTR)dwContext, MAX_PATH);

    // Read registry settings
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)dwContext, 0, 0, &hKey) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Init: Failed to open registry key\r\n"));
        goto _clean;
    }
    else
    {
        // Read descriptor count value
        DWORD cb = sizeof(pPddObject->dwDescriptorCount);
        if (RegQueryValueEx(
            hKey,
            _T("DescriptorCount"),
            NULL,
            NULL,
            (LPBYTE)&pPddObject->dwDescriptorCount,
            &cb) != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Init: Failed to read DescriptorCount from registry\r\n")));
            goto _clean;
        }

        // Read power management disable flag
        DWORD dpm = sizeof(pPddObject->dwDisablePowerManagement);
        if (RegQueryValueEx(
            hKey,
            _T("DisablePowerManagement"),
            NULL,
            NULL,
            (LPBYTE)&pPddObject->dwDisablePowerManagement,
            &dpm) != ERROR_SUCCESS)
        {
            // Enable power management by default
            pPddObject->dwDisablePowerManagement = 0;
        }
    }

#ifdef ENABLE_TESTMODE_SUPPORT
    InitUsbTestMode(pPddObject->ioPortBase);
#endif

    // Set the initial power state to Full On
    pPddObject->CurrentDx = D0;
    fRet = (HcdPdd_SetDevicePower(pPddObject) == ERROR_SUCCESS);
    if (fRet)
    {
        // Create the memory and hcd object's
        pPddObject->lpvMemoryObject = HcdMdd_CreateMemoryObject(gcTotalAvailablePhysicalMemory,
                                                                gcHighPriorityPhysicalMemory, NULL, NULL);

        pPddObject->lpvOhcdMddObject = HcdMdd_CreateHcdObject(pPddObject,
                                                              pPddObject->lpvMemoryObject,
                                                              pPddObject->szDriverRegKey,
                                                              pPddObject->ioPortBase,
                                                              pPddObject->dwSysIntr);

        if (!pPddObject->lpvOhcdMddObject || !pPddObject->lpvMemoryObject)
        {
            DEBUGCHK(FALSE);
            fRet = FALSE;
        }
    }
    else
    {
        DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Init: HcdPdd_SetDevicePower failed\r\n")));
    }

_clean:

    if (hKey)
        RegCloseKey(hKey);

    if (!fRet)
    {
        // Cleanup

        if (pPddObject)
        {
            if (pPddObject->pSysConfReg)
                MmUnmapIoSpace(pPddObject->pSysConfReg, sizeof(OMAP_SYSC_GENERAL_REGS));

            if (pPddObject->ioCppiBase)
                MmUnmapIoSpace(pPddObject->ioCppiBase, sizeof(CSL_CppiRegs));

            if (pPddObject->ioPortBase)
                MmUnmapIoSpace(pPddObject->ioPortBase, sizeof(CSL_UsbRegs));

            delete pPddObject;
            pPddObject = NULL;
        }
    }

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH:-HcdPdd_Init:(0x%X)\r\n"), pPddObject));
    return (DWORD)pPddObject;
}

/* HcdPdd_CheckConfigPower
 *
 *    Check power required by specific device configuration and return whether it
 *    can be supported on this platform.  For CEPC, this is trivial, just limit to
 *    the 500mA requirement of USB.  For battery powered devices, this could be
 *    more sophisticated, taking into account current battery status or other info.
 *
 * Return Value:
 *    Return TRUE if configuration can be supported, FALSE if not.
 */
extern BOOL HcdPdd_CheckConfigPower(
                                   UCHAR bPort,         // IN - Port number
                                   DWORD dwCfgPower,    // IN - Power required by configuration
                                   DWORD dwTotalPower)  // IN - Total power currently in use on port
{
	UNREFERENCED_PARAMETER(bPort);
	UNREFERENCED_PARAMETER(dwCfgPower);
	UNREFERENCED_PARAMETER(dwTotalPower);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_CheckConfigPower:\r\n")));
    return TRUE;
}

extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SOhcdPdd *pPddObject = (SOhcdPdd *)hDeviceContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_PowerUp:\r\n")));
    DEBUGCHK(pPddObject);

    HcdMdd_PowerUp(pPddObject->lpvOhcdMddObject);

    return;
}

extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
    SOhcdPdd *pPddObject = (SOhcdPdd *)hDeviceContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_PowerDown:\r\n")));
    DEBUGCHK(pPddObject);

    HcdMdd_PowerDown(pPddObject->lpvOhcdMddObject);

    return;
}


extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Deinit:\r\n")));
    DEBUGCHK(pPddObject);


    if (pPddObject->lpvOhcdMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvOhcdMddObject);
    if (pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);

    if (pPddObject)
    {

        // Release SYSINTR
        if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pPddObject->dwSysIntr, sizeof(DWORD),
            NULL, 0, NULL))
        {
            DEBUGMSG(ZONE_ERROR, (L"HcdPdd_Deinit: Failed to release SYSINTR for IRQ %d!\r\n",
                IRQ_USB0_INT));
        }       

        if (pPddObject->pSysConfReg)
            MmUnmapIoSpace(pPddObject->pSysConfReg, sizeof(OMAP_SYSC_GENERAL_REGS));

        if (pPddObject->ioCppiBase)
            MmUnmapIoSpace(pPddObject->ioCppiBase, sizeof(CSL_CppiRegs));

        if (pPddObject->ioPortBase)
            MmUnmapIoSpace(pPddObject->ioPortBase, sizeof(CSL_UsbRegs));

        delete pPddObject;
        pPddObject = NULL;
    }

    return TRUE;
}


extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
                         DWORD ShareMode)
{
	UNREFERENCED_PARAMETER(AccessCode);
	UNREFERENCED_PARAMETER(ShareMode);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Open:\r\n")));
    return hDeviceContext;
}

extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
	UNREFERENCED_PARAMETER(hOpenContext);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Close:\r\n")));
    return TRUE;
}

extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(pBuffer);
	UNREFERENCED_PARAMETER(Count);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Read:\r\n")));
    return (DWORD)-1;
}

extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
                          DWORD NumberOfBytes)
{
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(pSourceBytes);
	UNREFERENCED_PARAMETER(NumberOfBytes);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Write:\r\n")));
    return (DWORD)-1;
}

extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
	UNREFERENCED_PARAMETER(hOpenContext);
	UNREFERENCED_PARAMETER(Amount);
	UNREFERENCED_PARAMETER(Type);

    DEBUGMSG(ZONE_FUNCTION, (TEXT("USBH: HcdPdd_Seek:\r\n")));
    return(DWORD)-1;
}

extern BOOL HcdPdd_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
                             DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    SOhcdPdd *pPddObject = (SOhcdPdd *)hOpenContext;
    DWORD dwError = ERROR_INVALID_PARAMETER;

	UNREFERENCED_PARAMETER(dwLenIn);
	UNREFERENCED_PARAMETER(pBufIn);

    DEBUGMSG(ZONE_FUNCTION, (_T("USBH: HcdPdd_IOControl: IOCTL:0x%x, InBuf:0x%x, InBufLen:%d, OutBuf:0x%x, OutBufLen:0x%x\r\n"),
        dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut));

    if (!pdwActualOut)
        goto Exit;

    switch (dwCode)
    {
    case IOCTL_POWER_CAPABILITIES:

        if ((pBufOut != NULL) && (dwLenOut >= sizeof(POWER_CAPABILITIES)))
        {
            __try
            {
                PPOWER_CAPABILITIES pPC = (PPOWER_CAPABILITIES) pBufOut;

                //  set power consumption ( in mW)
                pPC->Power[D0] = 0;
                pPC->Power[D1] = (DWORD)PwrDeviceUnspecified;
                pPC->Power[D2] = (DWORD)PwrDeviceUnspecified;
                pPC->Power[D3] = 0;
                pPC->Power[D4] = 0;

                //  set latency ( time to return to D0 in ms )
                pPC->Latency[D0] = 0;
                pPC->Latency[D1] = (DWORD)PwrDeviceUnspecified;
                pPC->Latency[D2] = (DWORD)PwrDeviceUnspecified;
                pPC->Latency[D3] = 20;
                pPC->Latency[D4] = 100;

                //  set device wake caps (BITMASK)
                pPC->WakeFromDx = 0;

                //  set inrush (BITMASK)
                pPC->InrushDx = 0;

                //  set supported device states (BITMASK)
                pPC->DeviceDx = 0x19;   // support D0, D3, D4 (no D1, D2)

                // set flags
                pPC->Flags = 0;

                if (pdwActualOut)
                    (*pdwActualOut) = sizeof(*pPC);

                dwError = ERROR_SUCCESS;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (_T("HcdPdd_IOControl: IOCTL_POWER_CAPABILITIES: exception in ioctl\r\n")));
            }
        }

        break;

    case IOCTL_POWER_GET:

        if ((pBufOut != NULL) && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)))
        {
            __try
            {
                (*(PCEDEVICE_POWER_STATE)pBufOut) = pPddObject->CurrentDx;

                if (pdwActualOut)
                    (*pdwActualOut) = sizeof(CEDEVICE_POWER_STATE);

                dwError = ERROR_SUCCESS;

                DEBUGMSG(ZONE_VERBOSE, (_T("HcdPdd_IOControl: IOCTL_POWER_GET %s; passing back %u\r\n"),
                    dwError == ERROR_SUCCESS ? _T("succeeded") : _T("failed"), pPddObject->CurrentDx));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (_T("HcdPdd_IOControl: IOCTL_POWER_GET: exception in ioctl\r\n")));
            }
        }

        break;

    case IOCTL_POWER_QUERY:

        if ((pBufOut != NULL) && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)))
        {
            __try
            {
                CEDEVICE_POWER_STATE NewDx = (*(PCEDEVICE_POWER_STATE)pBufOut);

                switch(NewDx)
                {
                case D0:
                case D3:
                case D4:
                    break;

                default:
                    (*(PCEDEVICE_POWER_STATE)pBufOut) = PwrDeviceUnspecified;
                }

                if (pdwActualOut)
                    (*pdwActualOut) = sizeof(CEDEVICE_POWER_STATE);

                dwError = ERROR_SUCCESS;

                DEBUGMSG(ZONE_VERBOSE, (_T("HcdPdd_IOControl: IOCTL_POWER_QUERY %u %s\r\n"),
                    NewDx, dwError == ERROR_SUCCESS ? _T("succeeded") : _T("failed")));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (_T("HcdPdd_IOControl: IOCTL_POWER_QUERY: exception in ioctl\r\n")));
            }
        }

        break;

    case IOCTL_POWER_SET:

        if ((pBufOut != NULL) && (dwLenOut >= sizeof(CEDEVICE_POWER_STATE)))
        {
            __try
            {
                CEDEVICE_POWER_STATE NewDx = (*(PCEDEVICE_POWER_STATE)pBufOut);

                if (VALID_DX(NewDx))
                {
                    switch(NewDx)
                    {
                    case D0:
                    case D4:
                        pPddObject->CurrentDx = NewDx;
                        break;

                    case D1:
                    case D2:
                    case D3:
                        pPddObject->CurrentDx = D3;
                        break;
                    }

                    if (pPddObject->CurrentDx == D3 || pPddObject->CurrentDx == D4)
                    {
                        HcdPdd_PowerDown((DWORD)pPddObject);
                    }

                    if ((dwError = HcdPdd_SetDevicePower(pPddObject)) == ERROR_SUCCESS)
                    {
                        if (pPddObject->CurrentDx == D0)
                        {
                            HcdPdd_PowerUp((DWORD)pPddObject);
                        }

                        *(PCEDEVICE_POWER_STATE)pBufOut = pPddObject->CurrentDx;

                        if (pdwActualOut)
                            (*pdwActualOut) = sizeof(CEDEVICE_POWER_STATE);
                    }
                }

                DEBUGMSG(ZONE_VERBOSE, (_T("HcdPdd_IOControl: IOCTL_POWER_SET %u %s; passing back %u\r\n"),
                    NewDx, dwError == ERROR_SUCCESS ? _T("succeeded") : _T("failed"), NewDx));
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                DEBUGMSG(ZONE_ERROR, (_T("HcdPdd_IOControl: exception in ioctl\r\n")));
            }
        }

        break;

    default:
        DEBUGMSG(ZONE_WARNING, (_T("USBH: HcdPdd_IOControl: Unsupported IOCTL code %u\r\n"), dwCode));
        dwError = ERROR_NOT_SUPPORTED;
    }

Exit:
    // Pass back appropriate response codes
    SetLastError(dwError);

    return (dwError == ERROR_SUCCESS);
}

// This gets called by the MDD's IST when it detects a power resume.
// By default it has nothing to do.
extern void HcdPdd_InitiatePowerUp(void)
{
    DEBUGMSG(ZONE_FUNCTION, (_T("USBH: HcdPdd_InitiatePowerUp:\r\n")));
    return;
}

