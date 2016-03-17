// Copyright (c) 2007, 2013 2008 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    system.c

Abstract:
    Device dependant part of the USB Universal Host Controller Driver (EHCD).

Notes:
--*/

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
//  File:  system.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <devload.h>
#include <giisr.h>
#include <hcdddsi.h>
#include <cebuscfg.h>
#include <omap35xx_config.h>
#include <omap35xx_irq.h>
#include <omap35xx_prcm.h>
#include <omap35xx_base_regs.h>
#include <omap35xx_usbhost.h>
#include <ceddkex.h>
#include <constants.h>
#include <initguid.h>
#include <gpio.h>
#include <omap35xx.h>
#include <bus.h>

#ifdef DEBUG
#define ZONE_CONTROL            DEBUGZONE(5)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_INFO               DEBUGZONE(10)
#define ZONE_PDD                DEBUGZONE(15)

//#undef ZONE_INFO
//#define ZONE_INFO 1
//#undef ZONE_ERROR
//#define ZONE_ERROR 1

// set to non-zero value to cause register dump after configuration

#endif


#define DUMP_REGS       0

#define EHCI_DMA_ALLOC                  0
#define REG_PHYSICAL_PAGE_SIZE      TEXT("PhysicalPageSize")
#define EHCI_PRCM_TIMEOUT           100   //  1000
#define NUM_PORTS                   3

BOOL g_fOmapEhciSuspended = 0;

enum
{
    USB_PORT_NOT_USED = 0,
    USB_PORT_PHY_MODE,
    USB_PORT_TLL_MODE
};


typedef struct _SEHCDPdd
{
    DWORD memBase;
    DWORD memLen;
    DWORD irq;
    DWORD BusSuspendResume;
    LPVOID lpvMemoryObject;
    LPVOID lpvEHCDMddObject;
    PVOID pvVirtualAddress;                 // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;

	OMAP_USB_TLL_REGS           *pOMAP_USB_TLL_REGS;
    OMAP_UHH_REGS               *pUHHRegs;
    OMAP_PRCM_USBHOST_PRM_REGS  * pOMAP_PRCM_USBHOST_PRM_REGS;
    OMAP_PRCM_CORE_PRM_REGS     * pOMAP_PRCM_CORE_PRM_REGS;

    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                 // serializes access to the PDD object
    HANDLE          IsrHandle;
    DWORD dwActualPowerState;
    HANDLE hParentBusHandle;
    DWORD port1Mode;
    DWORD port2Mode;
    DWORD port3Mode;
    DWORD Port1PwrGpio;
    DWORD Port2PwrGpio;
    DWORD Port3PwrGpio;
    DWORD Port1PwrLevel;
    DWORD Port2PwrLevel;
    DWORD Port3PwrLevel;
    HANDLE hGpio;
    HANDLE hRootBus;

    UINT nDVFSOrder;
} SEHCDPdd, *P_SEHCDPdd;

typedef struct _SEHCDOpen
{
	SEHCDPdd *pddObj;
}SEHCDOpen;

//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(SEHCDPdd, memBase),
        fieldsize(SEHCDPdd, memBase), NULL
    }, { 
        L"MemLen", PARAM_DWORD, TRUE, offset(SEHCDPdd, memLen),
        fieldsize(SEHCDPdd, memLen), NULL
    }, { 
        L"Irq", PARAM_DWORD, TRUE, offset(SEHCDPdd, irq),
        fieldsize(SEHCDPdd, irq), NULL
    }, { 
        L"BusSuspendResume", PARAM_DWORD, FALSE, offset(SEHCDPdd, BusSuspendResume),
        fieldsize(SEHCDPdd, BusSuspendResume), (void *)1                 // ; Either 0 (false) or 1 (true)
    }, { 
        L"Port1Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port1Mode),
        fieldsize(SEHCDPdd, port1Mode), (void *)USB_PORT_PHY_MODE                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"Port2Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port2Mode),
        fieldsize(SEHCDPdd, port2Mode), (void *)USB_PORT_PHY_MODE                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"Port3Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port3Mode),
        fieldsize(SEHCDPdd, port3Mode), (void *)USB_PORT_NOT_USED                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"DVFSOrder", PARAM_DWORD, FALSE, offset(SEHCDPdd, nDVFSOrder),
        fieldsize(SEHCDPdd, nDVFSOrder), (void *)150
    } 
};

extern void dumpRegs(void);

// Amount of memory to use for HCD buffer
#define gcTotalAvailablePhysicalMemory (0x20000) // 128K
#define gcHighPriorityPhysicalMemory (gcTotalAvailablePhysicalMemory/4)   // 1/4 as high priority

void PHY_AssertReset(SEHCDPdd * pPddObject)
{
	RETAILMSG(0 ,(TEXT("+PHY_AssertReset\r\n")));

    // platform specific code (if any) would go here...
	
    /* assert reset */
	// HUB nRESET to low
	GPIOClrBit(pPddObject->hGpio , GPIO_34);
	// HUB nRESETB to low
    GPIOClrBit(pPddObject->hGpio , GPIO_65);
	// power off PHY 1.8V
	GPIOSetBit(pPddObject->hGpio , GPIO_111);
        
	StallExecution(10*1000);

	RETAILMSG(0 ,(TEXT("-PHY_AssertReset\r\n")));
}

void PHY_DeassertReset(SEHCDPdd * pPddObject)
{
	RETAILMSG(0 ,(TEXT("+PHY_DeassertReset\r\n")));

    // platform specific code (if any) would go here...
    /* deassert reset */
	// HUB nRESET to high
    //GPIOSetBit(pPddObject->hGpio , GPIO_34);
	//StallExecution(2 * 1000);
	// power on PHY 1.8V
    GPIOClrBit(pPddObject->hGpio , GPIO_111);
	StallExecution(2 * 1000);
	// HUB nRESETB to high
	GPIOSetBit(pPddObject->hGpio , GPIO_65);
	StallExecution(2 * 1000);
        
	RETAILMSG(0 ,(TEXT("-PHY_DeassertReset\r\n")));
}

void TLLDevice_AssertReset(SEHCDPdd * pPddObject)
{
    // platform specific code (if any) would go here...

    return;
}

void TLLDevice_DeassertReset(SEHCDPdd * pPddObject)
{
    // platform specific code (if any) would go here...

    return;
}

void ExternalDeviceAssertReset(SEHCDPdd * pPddObject)
{
    if(pPddObject->port1Mode == USB_PORT_PHY_MODE || pPddObject->port2Mode == USB_PORT_PHY_MODE || pPddObject->port3Mode == USB_PORT_PHY_MODE)
       PHY_AssertReset(pPddObject);
    else if(pPddObject->port1Mode == USB_PORT_TLL_MODE || pPddObject->port2Mode == USB_PORT_TLL_MODE || pPddObject->port3Mode == USB_PORT_TLL_MODE)
		TLLDevice_AssertReset(pPddObject);
}

void ExternalDeviceDeassertReset(SEHCDPdd * pPddObject)
{
    if(pPddObject->port1Mode == USB_PORT_PHY_MODE || pPddObject->port2Mode == USB_PORT_PHY_MODE || pPddObject->port3Mode == USB_PORT_PHY_MODE)
		PHY_DeassertReset(pPddObject);
	else if(pPddObject->port1Mode == USB_PORT_TLL_MODE || pPddObject->port2Mode == USB_PORT_TLL_MODE || pPddObject->port3Mode == USB_PORT_TLL_MODE)
		TLLDevice_DeassertReset(pPddObject);
}

static BOOL Configure_EHCI(SEHCDPdd * pPddObject)
{
	BOOL  bPortTLLmode[NUM_PORTS];
    DWORD dwStartTickCount = 0, dwCnt, dwHostConfig = 0;
    
	RETAILMSG(0 ,(TEXT("+Configure_EHCI\r\n")));
    RETAILMSG(1, (TEXT("*****###  Working TLL code with PHY support.  %s   %s\r\n"), TEXT(__DATE__), TEXT(__TIME__)));

	RETAILMSG(0 ,(TEXT("EHCI -  UHHRegs = 0x%X, &pUHHRegs->SYSCONFIG = 0x%X\r\n"), pPddObject->pUHHRegs, &pPddObject->pUHHRegs->SYSCONFIG));
	RETAILMSG(0 ,(TEXT("EHCI -  USB_TLL_REGS = 0x%X, &USBTLL_SYSCONFIG = 0x%X\r\n"), pPddObject->pOMAP_USB_TLL_REGS, &pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG));

    
    // enable USB host save and restore mechanism
    pPddObject->pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST |= (0x01 << 4); //SAVEANDRESTORE

    // enable USBTLL save and restore mechanism
    pPddObject->pOMAP_PRCM_CORE_PRM_REGS->PM_PWSTCTRL_CORE |= (0x01 << 4); //SAVEANDRESTORE

    // put external USB devices in reset before enabling EHCI controller.  
    // Some USB transceivers require to be held in reset before USB host supplies ULPI clock.
    ExternalDeviceAssertReset(pPddObject);
	RETAILMSG(0 ,(TEXT("EHCI -  Phy reset asserted %d\r\n"), GetTickCount()));
    
    /* start a soft reset on usb tll*/
	pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG = USBTLL_SYSCONFIG_SOFTRESET | USBTLL_SYSCONFIG_ENAWAKEUP | USBTLL_SYSCONFIG_SIDLEMODE(SIDLE_IGNORE)| USBTLL_SYSCONFIG_CACTIVITY;
    dwStartTickCount = GetTickCount() + EHCI_PRCM_TIMEOUT;
	while(!(pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS & USBTLL_SYSSTATUS_RESETDONE))
	{
		if(GetTickCount() > dwStartTickCount)
		{
			RETAILMSG(1, (TEXT("EHCI -  USB_TLL reset Timeout 0x%x\r\n"), pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
            return(FALSE);
		}
	}

	RETAILMSG(0 ,(TEXT("EHCI -  USB_TLL ready %d\r\n"), GetTickCount()));

	pPddObject->pUHHRegs->SYSCONFIG |= UHH_SYSCONFIG_AUTOIDLE | UHH_SYSCONFIG_SOFTRESET;

    dwStartTickCount = GetTickCount() + EHCI_PRCM_TIMEOUT;
	while(!(pPddObject->pUHHRegs->SYSSTATUS & UHH_SYSSTATUS_RESETDONE))
	{
		if(GetTickCount() > dwStartTickCount)
		{
			RETAILMSG(1, (TEXT("EHCI -  UHH reset Timeout 0x%x\r\n"), pPddObject->pUHHRegs->SYSSTATUS));
			return(FALSE);
		}
	}
    pPddObject->pUHHRegs->SYSCONFIG &= ~UHH_SYSCONFIG_SOFTRESET; 
	RETAILMSG(1 ,(TEXT("EHCI -  UHH SYSSTATUS 0x%x, ResetDone delay = %d ticks\r\n"), pPddObject->pUHHRegs->SYSSTATUS, GetTickCount()));
    
	pPddObject->pUHHRegs->SYSCONFIG = UHH_SYSCONFIG_ENAWAKEUP | UHH_SYSCONFIG_SIDLEMODE(SIDLE_SMART) | UHH_SYSCONFIG_MIDLEMODE(MIDLE_SMART);
        
        for(dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            bPortTLLmode[dwCnt] = FALSE;

        if (pPddObject->port1Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P1_ULPI_BYPASS;
            bPortTLLmode[0] = TRUE;
        }
        
        // bypass bits were added for ports 2 and 3
        if (pPddObject->port2Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P2_ULPI_BYPASS;
            bPortTLLmode[1] = TRUE;
        }
        
        if (pPddObject->port3Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P3_ULPI_BYPASS;
            bPortTLLmode[2] = TRUE;
        }

        dwHostConfig |= UHH_HOSTCONFIG_ENA_INCR4 | UHH_HOSTCONFIG_ENA_INCR8 | UHH_HOSTCONFIG_ENA_INCR16;
        
        pPddObject->pUHHRegs->HOSTCONFIG = dwHostConfig;
        
        // Not sure what this polling loop is for...
		dwStartTickCount = GetTickCount() + EHCI_PRCM_TIMEOUT;
        if( pPddObject->port1Mode == USB_PORT_PHY_MODE )
        {
            while(pPddObject->pUHHRegs->HOSTCONFIG & UHH_HOSTCONFIG_P1_ULPI_BYPASS)
			{
				if(GetTickCount() > dwStartTickCount)
				{
					RETAILMSG(1, (TEXT("EHCI -  HOSTCONFIG Timeout 0x%x\r\n"), pPddObject->pUHHRegs->HOSTCONFIG));
					return 0;
				}
			}
            
            RETAILMSG(1, (TEXT("EHCI: PHY Mode Polling done, ticks = %d\r\n"), GetTickCount()));
        }
        
        if ((pPddObject->port1Mode == USB_PORT_TLL_MODE) ||
            (pPddObject->port2Mode == USB_PORT_TLL_MODE) ||
            (pPddObject->port3Mode == USB_PORT_TLL_MODE) 
            )
        {
            RETAILMSG(0, (TEXT("EHCI: TLL Mode Polling start...\r\n")));
            RETAILMSG(0, (TEXT("##########USBTLL_SYSSTATUS =0x%x\r\n"), pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
            
            for (dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            {
                if (bPortTLLmode[dwCnt] == TRUE)
                    pPddObject->pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[dwCnt] &= 
                             ~(USBTLL_CHANNEL_CONF_i_ULPIDDRMODE | 
                             USBTLL_CHANNEL_CONF_i_UTMIAUTOIDLE | 
                             USBTLL_CHANNEL_CONF_i_ULPIAUTOIDLE);
            }
            
			RETAILMSG(0, (TEXT("##########USBTLL_SYSSTATUS =0x%x\r\n"), pPddObject->pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
            pPddObject->pOMAP_USB_TLL_REGS->TLL_SHARED_CONF |= USBTLL_SHARED_CONF_FCLK_IS_ON | USBTLL_SHARED_CONF_USB_DIVRATIO(1);
            
            for (dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            {
                if (bPortTLLmode[dwCnt] == TRUE)
                    pPddObject->pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[dwCnt] |= 
                        USBTLL_CHANNEL_CONF_i_CHANEN | USBTLL_CHANNEL_CONF_i_ULPINOBITSTUFF;
            }
        }
        
        if (pPddObject->port1Mode == USB_PORT_PHY_MODE)
        {
            pPddObject->pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

        if (pPddObject->port2Mode == USB_PORT_PHY_MODE)
        {
            pPddObject->pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

        if (pPddObject->port3Mode == USB_PORT_PHY_MODE)
        {
            pPddObject->pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

	#if DUMP_REGS
        //dumpRegs();
    #endif
    {
		RETAILMSG(1, (TEXT("########## Deassert Reset %d ... "), GetTickCount()));
		ExternalDeviceDeassertReset(pPddObject);
		RETAILMSG(1, (TEXT("%d done\r\n"), GetTickCount()));

		StallExecution(100 * 1000);

    }

	RETAILMSG(0 ,(TEXT("-Configure_EHCI\r\n")));
    return TRUE;
}

/* HcdPdd_DllMain
 *
 *  DLL Entry point.
 *
 * Return Value:
 */
extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    return 1;
}

/* GetRegistryPhysicalMemSize
 * Return Value:
 */
static BOOL GetRegistryPhysicalMemSize(LPCWSTR RegKeyPath, DWORD * lpdwPhyscialMemSize)
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwRet;
    
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if(dwRet == ERROR_SUCCESS)
	{
		// Read base address, range from registry and determine IOSpace
		dwSize = sizeof(dwData);
		dwRet = RegQueryValueEx(hKey, REG_PHYSICAL_PAGE_SIZE, 0, &dwType, (PUCHAR)&dwData, &dwSize);
		RegCloseKey(hKey);
		if(dwRet == ERROR_SUCCESS)
		{
			if (lpdwPhyscialMemSize)
			{
				*lpdwPhyscialMemSize = dwData;
				return 1;
			}
		}
    }
	DEBUGMSG(ZONE_ERROR,(TEXT("EHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"), RegKeyPath, dwRet));
	return 0;
}



/* InitializeEHCI
 *
 *  Configure and initialize EHCI card
 *
 *  Return Value:
 *  Return TRUE if card could be located and configured, otherwise FALSE
 */
static BOOL InitializeEHCI(SEHCDPdd * pPddObject, LPCWSTR szDriverRegKey)
{
    BOOL    fResult = FALSE;
    LPVOID  pobMem = NULL;
    LPVOID  pobEHCD = NULL;
    DWORD   dwHPPhysicalMemSize;
    HKEY    hKey;
    DWORD   dwIrq = IRQ_EHCI;
    PHYSICAL_ADDRESS pa;
    
	if(!pPddObject)
	{
		RETAILMSG(1, (TEXT("InitializeEHCI: pPddObject null pointer\r\n")));
		return 0;
	}
    
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDriverRegKey, 0, 0, &hKey)!= ERROR_SUCCESS)
    {
        RETAILMSG(1, (TEXT("InitializeEHCI:GetRegistryConfig RegOpenKeyEx(%s) failed\r\n"), szDriverRegKey));
        return 0;
    }
    
    // Map register base to virtual address
    pa.QuadPart = OMAP35XX_EHCI_REGS_PA;
    
    pPddObject->ioPortBase  =(UCHAR *)MmMapIoSpace(pa, OMAP35XX_EHCI_REGS_SIZE, FALSE);
    if(!pPddObject->ioPortBase)
    {
		RETAILMSG(1, (TEXT("EHCI: Failed to map 0x%x...\r\n"), OMAP35XX_EHCI_REGS_PA));
        return 0;
    }
    
    pa.QuadPart = OMAP35XX_UHH_REGS_PA;
    pPddObject->pUHHRegs = (OMAP_UHH_REGS *)MmMapIoSpace(pa, sizeof(OMAP_UHH_REGS), FALSE);

    pa.QuadPart = OMAP35XX_USBTLL_REGS_PA;
    pPddObject->pOMAP_USB_TLL_REGS = (OMAP_USB_TLL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_USB_TLL_REGS), FALSE);

	pa.QuadPart = OMAP_PRCM_USBHOST_PRM_REGS_PA;
    pPddObject->pOMAP_PRCM_USBHOST_PRM_REGS = (OMAP_PRCM_USBHOST_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);
    
    pa.QuadPart = OMAP_PRCM_CORE_PRM_REGS_PA;
    pPddObject->pOMAP_PRCM_CORE_PRM_REGS = (OMAP_PRCM_CORE_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_PRM_REGS), FALSE);
    

	if(!Configure_EHCI(pPddObject))
		goto InitializeEHCI_Error;

    // Request SYSINTR for interrupt
    if(!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(dwIrq), &pPddObject->dwSysIntr, sizeof(pPddObject->dwSysIntr), 0))
    {
		RETAILMSG(1, (TEXT("EHCI: Failed to aquire IRQ_USB_HGEN")));
        return 0;
    }
    
	RETAILMSG(0, (L"EHCI: SYSINTR %d\r\n", pPddObject->dwSysIntr));

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // The PDD can supply a buffer of contiguous physical memory here, or can let the
    // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
    // API to allocate the memory and bus controller physical addresses and pass this information
    // into the MDD.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    if (GetRegistryPhysicalMemSize(szDriverRegKey, &pPddObject->dwPhysicalMemSize))
    {
        // A quarter for High priority Memory.
        dwHPPhysicalMemSize = pPddObject->dwPhysicalMemSize/4;
        // Align with page size.
        pPddObject->dwPhysicalMemSize =(pPddObject->dwPhysicalMemSize + PAGE_SIZE -1) & ~(PAGE_SIZE -1);
        dwHPPhysicalMemSize =((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
    }
    else
    {
        pPddObject->dwPhysicalMemSize = 0;
    }
    
    if (pPddObject->dwPhysicalMemSize < gcTotalAvailablePhysicalMemory)
    {
        pPddObject->dwPhysicalMemSize = gcTotalAvailablePhysicalMemory;
        dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
    }
    
#if (EHCI_DMA_ALLOC)
    // Fill out the DMA adapter descriptor.  The bus address and type is irrelevant
    pPddObject->AdapterObject.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);
    pPddObject->AdapterObject.InterfaceType = PCIBus;
    pPddObject->AdapterObject.BusNumber     = 0;
    
    
    if ((pPddObject->pvVirtualAddress = HalAllocateCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, &pPddObject->LogicalAddress, FALSE)) == NULL)
    {
		RETAILMSG(1, (TEXT("EHCI - HalAllocateCommonBuffer Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, (PUCHAR) pPddObject->pvVirtualAddress, (PUCHAR) pPddObject->LogicalAddress.LowPart)))
#endif
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, NULL, NULL)))
    {
		RETAILMSG(1, (TEXT("EHCI - HcdMdd_CreateMemoryObject Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    if (!(pobEHCD = HcdMdd_CreateHcdObject(pPddObject, pobMem, szDriverRegKey, pPddObject->ioPortBase, pPddObject->dwSysIntr)))
    {
		RETAILMSG(1, (TEXT("EHCI - HcdMdd_CreateHcdObject Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    pPddObject->lpvMemoryObject  = pobMem;
    pPddObject->lpvEHCDMddObject = pobEHCD;
    
    _tcsncpy(pPddObject->szDriverRegKey, szDriverRegKey, MAX_PATH);
    
    if (hKey != NULL)
    {
        DWORD dwCapability;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_CAPABILITY_VALNAME, 0, &dwType, (PUCHAR)&dwCapability, &dwLength) == ERROR_SUCCESS)
        {
            HcdMdd_SetCapability(pobEHCD, dwCapability);
        }
        RegCloseKey(hKey);
    }
    
    //ExternalDeviceDeassertReset(pPddObject);
    return TRUE;
    
InitializeEHCI_Error:
    if (pPddObject->IsrHandle)
    {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }
    
    if(pPddObject->ioPortBase != NULL )
       MmUnmapIoSpace((PVOID)pPddObject->ioPortBase, OMAP35XX_EHCI_REGS_SIZE);       

	if(pPddObject->pUHHRegs != NULL )
      MmUnmapIoSpace((PVOID)pPddObject->pUHHRegs, sizeof(OMAP_UHH_REGS)); 

	if(pPddObject->pOMAP_USB_TLL_REGS != NULL )
      MmUnmapIoSpace((PVOID)pPddObject->pOMAP_USB_TLL_REGS, sizeof(OMAP_USB_TLL_REGS)); 

    if (pobEHCD)
        HcdMdd_DestroyHcdObject(pobEHCD);
    if (pobMem)
        HcdMdd_DestroyMemoryObject(pobMem);
    if (pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);
    
    pPddObject->lpvMemoryObject = NULL;
    pPddObject->lpvEHCDMddObject = NULL;
    pPddObject->pvVirtualAddress = NULL;
    if (hKey != NULL)
        RegCloseKey(hKey);
    
	RETAILMSG(1, (TEXT("-InitializeEHCI fail\r\n")));
    return FALSE;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure EHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD HcdPdd_Init(DWORD dwContext)
{
    //SEHCDPdd *  pPddObject = malloc(sizeof(SEHCDPdd));
	SEHCDPdd *  pPddObject =  (SEHCDPdd *)LocalAlloc(LPTR, sizeof(SEHCDPdd));
    BOOL        fRet = FALSE;
    extern DWORD g_dwContext;
    DWORD dwClock = OMAP_DEVICE_USBTLL;

	RETAILMSG(0, (L"EHCI USB Host: HcdPdd_Init+\r\n"));

    if (pPddObject)
    {
        pPddObject->pvVirtualAddress = NULL;
        InitializeCriticalSection(&pPddObject->csPdd);
        pPddObject->IsrHandle = NULL;

        // Read device parameters
        if(GetDeviceRegistryParams((LPCWSTR)dwContext, pPddObject, dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) 
        {
			RETAILMSG(1, (L"ERROR: HcdPdd_Init: Failed read registry parameters\r\n"));
            goto cleanUp;
        }

        pPddObject->hParentBusHandle = CreateBusAccessHandle( (LPCTSTR) g_dwContext ) ;
        if(pPddObject->hParentBusHandle == NULL)
		{
            DEBUGMSG(ZONE_ERROR, (TEXT("EHCI: Failure to create bus access handle.  g_dwContext = %s\r\n"), (char *)g_dwContext));
			RETAILMSG(1, (TEXT("EHCI: Failure to create bus access handle.  g_dwContext = %s\r\n"), (char *)g_dwContext));
		}

        SetDevicePowerState(pPddObject->hParentBusHandle, D0, NULL);

        // Open GPIO driver
        pPddObject->hGpio = GPIOOpen();
        if (pPddObject->hGpio == INVALID_HANDLE_VALUE) 
            goto cleanUp;

		// Set Line for output
		GPIOSetMode(pPddObject->hGpio , 111, GPIO_DIR_OUTPUT);
		GPIOSetMode(pPddObject->hGpio , 65, GPIO_DIR_OUTPUT);
		GPIOSetMode(pPddObject->hGpio , 54, GPIO_DIR_OUTPUT);

        // open root bus driver
        pPddObject->hRootBus = CreateFile(L"BUS1:", 0, 0, NULL, 0, 0, 0 );
        if (pPddObject->hRootBus == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_Init: Failed open root bus driver handle\r\n"));
			RETAILMSG(1, (L"ERROR: HcdPdd_Init: Failed open root bus driver handle\r\n"));
            goto cleanUp;
        }

        // enable USBTLL clocks
        if (!DeviceIoControl(pPddObject->hRootBus, IOCTL_BUS_REQUEST_CLOCK, &dwClock, sizeof(dwClock), NULL, 0, NULL, NULL))
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_Init: IOCTL_BUS_REQUEST_CLOCK failed\r\n"));
			RETAILMSG(1, (L"ERROR: HcdPdd_Init: IOCTL_BUS_REQUEST_CLOCK failed\r\n"));
            goto cleanUp;
        }
        Sleep(10);

        fRet = InitializeEHCI(pPddObject, (LPCWSTR)dwContext);

        if (fRet)
		{
			if (pPddObject->BusSuspendResume)
	            HcdMdd_SetCapability(pPddObject->lpvEHCDMddObject, HCD_SUSPEND_RESUME | HCD_ROOT_HUB_INTERRUPT);
    	    else
        	    HcdMdd_SetCapability(pPddObject->lpvEHCDMddObject, HCD_ROOT_HUB_INTERRUPT);
		}
			
        if(!fRet)
        {
            DeleteCriticalSection(&pPddObject->csPdd);
            free(pPddObject);
            pPddObject = NULL;
        }
    }

cleanUp:
    if(!fRet && pPddObject) 
    {
        if(pPddObject->lpvEHCDMddObject) 
			HcdMdd_DestroyHcdObject(pPddObject->lpvEHCDMddObject);
        if(pPddObject->lpvMemoryObject) 
			HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
        if(pPddObject->dwSysIntr) 
			KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, (PVOID)&pPddObject->dwSysIntr, sizeof(&pPddObject->dwSysIntr), NULL, 0, NULL);
        CloseBusAccessHandle(pPddObject->hParentBusHandle);
        free(pPddObject);
        pPddObject = 0;
    }

	GPIOSetBit(pPddObject->hGpio , GPIO_34);
	StallExecution(20 * 1000);

	RETAILMSG(0, (TEXT("EHCI USB Host: HcdPdd_Init -\r\n")));

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
extern BOOL HcdPdd_CheckConfigPower(UCHAR bPort, DWORD dwCfgPower, DWORD dwTotalPower)
{
    return ((dwCfgPower + dwTotalPower) > 500) ? 0 : 1;
}

extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
	RETAILMSG(0, (TEXT("HcdPdd_PowerUp\r\n")));

    return;
}

extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
	RETAILMSG(0, (TEXT("HcdPdd_PowerDown\r\n")));

	return;
}


extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;

    if(pPddObject->lpvEHCDMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvEHCDMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    if(pPddObject->ioPortBase != NULL )
       MmUnmapIoSpace((PVOID)pPddObject->ioPortBase, OMAP35XX_EHCI_REGS_SIZE);       

	if(pPddObject->pUHHRegs != NULL )
      MmUnmapIoSpace((PVOID)pPddObject->pUHHRegs, sizeof(OMAP_UHH_REGS)); 

	if(pPddObject->pOMAP_USB_TLL_REGS != NULL )
      MmUnmapIoSpace((PVOID)pPddObject->pOMAP_USB_TLL_REGS, sizeof(OMAP_USB_TLL_REGS)); 

    SetDevicePowerState(pPddObject->hParentBusHandle, D4, NULL);
    if (pPddObject->hParentBusHandle)
        CloseBusAccessHandle(pPddObject->hParentBusHandle);

    LocalFree(pPddObject);
    return 1;
}


extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	SEHCDPdd  *pPddObject	  = (SEHCDPdd  *)hDeviceContext;
	SEHCDOpen *pOpenPddObject = (SEHCDOpen *)LocalAlloc(LPTR, sizeof(SEHCDOpen));

	RETAILMSG(1, (L"EHCI:+%S, pPddObject = 0x%X\r\n",__FUNCTION__, pPddObject));

	if(pOpenPddObject)
		pOpenPddObject->pddObj = pPddObject;

#ifdef USB_IF_ELECTRICAL_TEST_MODE
    return (DWORD) (((SEHCDPdd*)hDeviceContext)->lpvEHCDMddObject);
#endif //#ifdef USB_IF_ELECTRICAL_TEST_MODE

	RETAILMSG(1, (L"EHCI:-%S, pOpenPddObject = 0x%X\r\n",__FUNCTION__, pOpenPddObject));

    return((DWORD)pOpenPddObject); // we can be opened, but only once!
}


extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
	SEHCDOpen * pPddObject = (SEHCDOpen *)hOpenContext;

	RETAILMSG(1, (L"EHCI:+%S, pPddObject = 0x%X\r\n",__FUNCTION__, pPddObject));

	LocalFree(pPddObject);

	RETAILMSG(1, (L"EHCI:-%S\r\n",__FUNCTION__));
    return TRUE;
}


extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    return (DWORD)-1; // an error occured
}

extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes, DWORD NumberOfBytes)
{
    return (DWORD)-1;
}


extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    return (DWORD)-1;
}


extern BOOL HcdPdd_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	BOOL rc = FALSE;
    SEHCDPdd *pPddObject;
    DWORD dwClock = OMAP_DEVICE_USBTLL;
	
    DEBUGMSG(ZONE_CONTROL, (L"+HcdPdd_IOControl(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n", hOpenContext, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut));

    // Check if we get correct context
    if(!(SEHCDOpen *)hOpenContext)
	{
        DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_IOControl: Incorrect context paramer\r\n"));
        goto cleanUp;
	}

	pPddObject = ((SEHCDOpen *)hOpenContext)->pddObj;

    switch(dwCode)
	{
		case IOCTL_GET_USB_DEVICE_EXT_INFO:
		{
			if (pBufIn && dwLenIn >= sizeof(UCHAR) && pBufOut && dwLenOut >= sizeof(USB_DEVICE_EXT_INFO))
			{
				USB_DEVICE_EXT_INFO *info = (USB_DEVICE_EXT_INFO *)pBufOut;
				if (rc = HcdMdd_GetDeviceInfo(pPddObject->lpvEHCDMddObject, pBufIn[0], &(info->Port), &(info->Tier)))
				{
					if (pdwActualOut)
						*pdwActualOut = sizeof(USB_DEVICE_EXT_INFO);
				}
			}

			break;
		}
        case IOCTL_POWER_CAPABILITIES:
		{
            DEBUGMSG(ZONE_INFO, (L"EHCI: Received IOCTL_POWER_CAPABILITIES\r\n"));
            if(pBufOut && dwLenOut >= sizeof (POWER_CAPABILITIES) && pdwActualOut)
			{
				__try
                {
					PPOWER_CAPABILITIES PowerCaps;
                    PowerCaps = (PPOWER_CAPABILITIES)pBufOut;

                    // Only supports D0 (permanently on) and D4(off).
                    memset(PowerCaps, 0, sizeof(*PowerCaps));
                    PowerCaps->DeviceDx = (DX_MASK(D0) | DX_MASK(D4));
					*pdwActualOut = sizeof(*PowerCaps);
                    rc = TRUE;
				}
                __except(EXCEPTION_EXECUTE_HANDLER)
				{
					DEBUGMSG(ZONE_ERROR, (L"exception in ioctl\r\n"));
				}
			}
            break;
		}
        // deprecated
        case IOCTL_POWER_QUERY:
		{
            rc = TRUE;
            DEBUGMSG(ZONE_INFO,(L"EHCI: Received IOCTL_POWER_QUERY\r\n"));
            break;
		}
        // requests a change from one device power state to another
        case IOCTL_POWER_SET:
		{
            DEBUGMSG(ZONE_INFO,(L"EHCI: Received IOCTL_POWER_SET\r\n"));
			if(pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE))
			{
                CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pBufOut;
                
                switch(ReqDx)
				{
                    case D0:
                    case D1:
                    case D2:
					{
						if(pPddObject->dwActualPowerState > D2)
						{
							__try
							{
								Configure_EHCI(pPddObject);

								HcdMdd_PowerUp(pPddObject->lpvEHCDMddObject);
								StallExecution(100 * 1000);
								
								GPIOSetBit(pPddObject->hGpio , GPIO_34);
								StallExecution(20 * 1000);
							}
							__except(EXCEPTION_EXECUTE_HANDLER)
							{
								RETAILMSG(1, (L"EHCI: Exception in IOCTL_POWER_SET(D0)\r\n"));
								GPIOClrBit(pPddObject->hGpio , GPIO_34);
								StallExecution(200 * 1000);

								HcdMdd_PowerDown(pPddObject->lpvEHCDMddObject);

								// ##### Michael Set USB devices in RESET state on Power Down
								ExternalDeviceAssertReset(pPddObject);
								StallExecution(10 * 1000);
							}

							#if DUMP_REGS
								dumpRegs();
							#endif


							StallExecution(20 * 1000);
							g_fOmapEhciSuspended = 0; // Instead it Igor wants event ???
						}

                        break;
					}
                    case D3:
                    case D4:
					{
						if(pPddObject->dwActualPowerState < D3)
						{
							__try
							{
								GPIOSetBit(pPddObject->hGpio , GPIO_34);
								StallExecution(200 * 1000);

								HcdMdd_PowerDown(pPddObject->lpvEHCDMddObject);

								// ##### Michael Set USB devices in RESET state on Power Down
								ExternalDeviceAssertReset(pPddObject);
								StallExecution(10 * 1000);
							}
							__except(EXCEPTION_EXECUTE_HANDLER)
							{
								RETAILMSG(1, (L"EHCI: Exception in IOCTL_POWER_SET(D4)\r\n"));
							}
							#if DUMP_REGS
								dumpRegs();
							#endif

							g_fOmapEhciSuspended = TRUE;
						}

						break;
					}
				}
				pPddObject->dwActualPowerState = ReqDx;
                *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

                DEBUGMSG(ZONE_INFO, (L"EHCI: IOCTL_POWER_SET to D%u \r\n", pPddObject->dwActualPowerState));

                rc = 1;
			}
            break;
		}
        // gets the current device power state
        case IOCTL_POWER_GET:
		{
            DEBUGMSG(ZONE_INFO, (L"EHCI: Received IOCTL_POWER_GET\r\n"));
            if(pBufOut != NULL && dwLenOut >= sizeof(CEDEVICE_POWER_STATE))
			{
                __try
				{
                    *(PCEDEVICE_POWER_STATE)pBufOut = pPddObject->dwActualPowerState;

                    rc = TRUE;

                    DEBUGMSG(ZONE_INFO, (L"EHCI: IOCTL_POWER_GET to D%u \r\n", pPddObject->dwActualPowerState));
				}
                __except(EXCEPTION_EXECUTE_HANDLER)
				{
                    DEBUGMSG(ZONE_ERROR, (L"Exception in ioctl\r\n"));
				}
			}
            break;
		}
        default:
            DEBUGMSG(ZONE_CONTROL, (L"EHCI: Not handled IOCTL 0x%X", dwCode));
            break;
	}

cleanUp:
    DEBUGMSG(ZONE_CONTROL, (L"-HcdPdd_IOControl(rc = %d)\r\n", rc));
    return rc;

}            

// Manage WinCE suspend/resume events
// This gets called by the MDD's IST when it detects a power resume.
// By default it has nothing to do.
extern void HcdPdd_InitiatePowerUp(DWORD hDeviceContext)
{
    return;
}

#if DUMP_REGS
void dumpRegs(void)
{
    PHYSICAL_ADDRESS pa;
    static OMAP_PRCM_USBHOST_CM_REGS        *pOMAP_USBHOST_CM;
    static OMAP_PRCM_CORE_CM_REGS           *pOMAP_PRCM_CORE_CM_REGS;
    static OMAP_PRCM_USBHOST_PRM_REGS       *pOMAP_PRCM_USBHOST_PRM_REGS;
    static OMAP_PRCM_CLOCK_CONTROL_CM_REGS  *pOMAP_PRCM_CLOCK_CONTROL_CM_REGS;
    static OMAP_USB_TLL_REGS                *pOMAP_USB_TLL_REGS;
    static OMAP_UHH_REGS                    *pUHHRegs;
    static OMAP_SYSC_PADCONFS_REGS          *pConfig;
    static volatile UINT8                   *pbTLL_ULPI, *pReg8;
    UINT32 addr_phy, byte_offset;

/*---------------------------------- CODE ------------------------------------*/
    RETAILMSG(1,(TEXT("EHCI: dumpRegs Called\r\n")));
	RETAILMSG(1,(TEXT("#############################################\r\n")));

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;

    RETAILMSG(1,(TEXT("\r\nTLL module:\r\n")));

    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x14;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x18;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x1c;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x44;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x48;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    pa.QuadPart      = OMAP35XX_UHH_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;

    RETAILMSG(1,(TEXT("\r\nUHH_config module:\r\n")));

    byte_offset = 0x0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x14;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x44;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    pa.QuadPart      = OMAP35XX_EHCI_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;
    RETAILMSG(1,(TEXT("\r\nEHCI module:\r\n")));

    for (byte_offset=0; byte_offset <= 0x28; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    for (byte_offset=0x50; byte_offset <= 0x5c; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x90; byte_offset <= 0xa0; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    RETAILMSG(1,(TEXT("\r\nPRCM module:\r\n")));

    pa.QuadPart      = 0x48002000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x274;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x2d8;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    MmUnmapIoSpace((PVOID)pReg8, 0x1000);

    
    pa.QuadPart  = 0x48004000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x0;

    for (byte_offset=0x0; byte_offset <= 0x4; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x40; byte_offset <= 0x44; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x904; byte_offset <= 0x904; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x940; byte_offset <= 0x944; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    for (byte_offset=0xa00; byte_offset <= 0xa08; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0xa10; byte_offset <= 0xa18; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0xa30; byte_offset <= 0xa38; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    byte_offset = 0xa40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd04;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    for (byte_offset=0xd40; byte_offset <= 0xd4c; byte_offset+=4)
    {
        RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    byte_offset = 0xd50;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd70;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  = 0x48005000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x0;

    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x140;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x400;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x410;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x430;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  =  0x48306000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0xae0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd70;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  =  0x48007000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0x270;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x4e0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);



    RETAILMSG(1,(TEXT("\r\nSDRC module:\r\n")));

    pa.QuadPart  = 0x6d000000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0xa0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xa4;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);



    RETAILMSG(1,(TEXT("\r\nPAD Conf:\r\n")));

    pa.QuadPart  = 0x48002000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0x16c;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x170;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x184;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x188;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x164;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x168;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x180;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)\r\n"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));


    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    //TLLDevice_AssertReset();
    //  TLLDevice_DeassertReset();

    pa.QuadPart = OMAP35XX_UHH_REGS_PA;
    pUHHRegs    = (OMAP_UHH_REGS *)MmMapIoSpace(pa, OMAP35XX_UHH_REGS_SIZE, FALSE);

    pa.QuadPart      = OMAP_PRCM_USBHOST_CM_REGS_PA;
    pOMAP_USBHOST_CM = (OMAP_PRCM_USBHOST_CM_REGS *)MmMapIoSpace(pa, OMAP_PRCM_USBHOST_CM_REGS_SIZE, FALSE);

    pa.QuadPart      = OMAP_PRCM_CORE_CM_REGS_PA;
    pOMAP_PRCM_CORE_CM_REGS = (OMAP_PRCM_CORE_CM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_CM_REGS), FALSE);

    pa.QuadPart      = OMAP_PRCM_USBHOST_PRM_REGS_PA;
    pOMAP_PRCM_USBHOST_PRM_REGS = (OMAP_PRCM_USBHOST_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);

    pa.QuadPart      = OMAP_PRCM_CLOCK_CONTROL_CM_REGS_PA;
    pOMAP_PRCM_CLOCK_CONTROL_CM_REGS = (OMAP_PRCM_CLOCK_CONTROL_CM_REGS*)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CLOCK_CONTROL_CM_REGS), FALSE);

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    pOMAP_USB_TLL_REGS = (OMAP_USB_TLL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_USB_TLL_REGS), FALSE);

    pa.QuadPart   = OMAP_SYSC_PADCONFS_REGS_PA;
    pConfig       = (OMAP_SYSC_PADCONFS_REGS *)MmMapIoSpace(pa, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    pbTLL_ULPI = (volatile char*)MmMapIoSpace(pa, 0x1000, FALSE);

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT5        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MMC2_DAT5,  pConfig->CONTROL_PADCONF_MMC2_DAT5  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT6        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MMC2_DAT6,  pConfig->CONTROL_PADCONF_MMC2_DAT6  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT7        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MMC2_DAT7,  pConfig->CONTROL_PADCONF_MMC2_DAT7  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_DX        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP3_DX,  pConfig->CONTROL_PADCONF_MCBSP3_DX  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_DR        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP3_DR,  pConfig->CONTROL_PADCONF_MCBSP3_DR  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_CLKX      (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP3_CLKX,pConfig->CONTROL_PADCONF_MCBSP3_CLKX));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_FSX       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP3_FSX, pConfig->CONTROL_PADCONF_MCBSP3_FSX ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_UART1_CTS        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_UART1_CTS,  pConfig->CONTROL_PADCONF_UART1_CTS  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_CLKX      (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP4_CLKX,pConfig->CONTROL_PADCONF_MCBSP4_CLKX));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_DR        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP4_DR,  pConfig->CONTROL_PADCONF_MCBSP4_DR  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_DX        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP4_DX,  pConfig->CONTROL_PADCONF_MCBSP4_DX  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_FSX       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCBSP4_FSX, pConfig->CONTROL_PADCONF_MCBSP4_FSX ));

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D10          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D10     ,pConfig->CONTROL_PADCONF_ETK_D10   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D11          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D11     ,pConfig->CONTROL_PADCONF_ETK_D11   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D12          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D12     ,pConfig->CONTROL_PADCONF_ETK_D12   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D13          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D13     ,pConfig->CONTROL_PADCONF_ETK_D13   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D14          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D14     ,pConfig->CONTROL_PADCONF_ETK_D14   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D15          (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_ETK_D15     ,pConfig->CONTROL_PADCONF_ETK_D15   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI1_CS3       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI1_CS3  ,pConfig->CONTROL_PADCONF_MCSPI1_CS3  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CS1       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI2_CS1  ,pConfig->CONTROL_PADCONF_MCSPI2_CS1  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_SIMO      (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI2_SIMO ,pConfig->CONTROL_PADCONF_MCSPI2_SIMO ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_SOMI      (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI2_SOMI ,pConfig->CONTROL_PADCONF_MCSPI2_SOMI ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CS0       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI2_CS0  ,pConfig->CONTROL_PADCONF_MCSPI2_CS0  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CLK       (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_MCSPI2_CLK  ,pConfig->CONTROL_PADCONF_MCSPI2_CLK  ));

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_UART1_RTS        (0x%x) 0x%x\r\n"),&pConfig->CONTROL_PADCONF_UART1_RTS,pConfig->CONTROL_PADCONF_UART1_RTS));


    RETAILMSG(1,(TEXT("CM_FCLKEN3_CORE          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_FCLKEN3_CORE,   pOMAP_PRCM_CORE_CM_REGS->CM_FCLKEN3_CORE ));
    RETAILMSG(1,(TEXT("CM_ICLKEN3_CORE          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_ICLKEN3_CORE,   pOMAP_PRCM_CORE_CM_REGS->CM_ICLKEN3_CORE ));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE3_CORE        (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_AUTOIDLE3_CORE, pOMAP_PRCM_CORE_CM_REGS->CM_AUTOIDLE3_CORE));
    RETAILMSG(1,(TEXT("CM_FCLKEN_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_FCLKEN_USBHOST, pOMAP_USBHOST_CM->CM_FCLKEN_USBHOST ));
    RETAILMSG(1,(TEXT("CM_ICLKEN_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_ICLKEN_USBHOST, pOMAP_USBHOST_CM->CM_ICLKEN_USBHOST ));
    RETAILMSG(1,(TEXT("CM_IDLEST_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_IDLEST_USBHOST, pOMAP_USBHOST_CM->CM_IDLEST_USBHOST ));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_AUTOIDLE_USBHOST, pOMAP_USBHOST_CM->CM_AUTOIDLE_USBHOST ));
    RETAILMSG(1,(TEXT("CM_SLEEPDEP_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_SLEEPDEP_USBHOST, pOMAP_USBHOST_CM->CM_SLEEPDEP_USBHOST ));
    RETAILMSG(1,(TEXT("CM_CLKSTCTRL_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_CLKSTCTRL_USBHOST, pOMAP_USBHOST_CM->CM_CLKSTCTRL_USBHOST ));
    RETAILMSG(1,(TEXT("CM_CLKSTST_USBHOST       (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_CLKSTST_USBHOST, pOMAP_USBHOST_CM->CM_CLKSTST_USBHOST ));


    RETAILMSG(1,(TEXT("RM_RSTST_USBHOST         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->RM_RSTST_USBHOST     , pOMAP_PRCM_USBHOST_PRM_REGS->RM_RSTST_USBHOST       ));
    RETAILMSG(1,(TEXT("PM_WKEN_USBHOST          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKEN_USBHOST      , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKEN_USBHOST        ));
    RETAILMSG(1,(TEXT("PM_MPUGRPSEL_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_MPUGRPSEL_USBHOST , pOMAP_PRCM_USBHOST_PRM_REGS->PM_MPUGRPSEL_USBHOST   ));
    RETAILMSG(1,(TEXT("PM_IVA2GRPSEL_USBHOST    (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_IVA2GRPSEL_USBHOST, pOMAP_PRCM_USBHOST_PRM_REGS->PM_IVA2GRPSEL_USBHOST ));
    RETAILMSG(1,(TEXT("PM_WKST_USBHOST          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKST_USBHOST      , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKST_USBHOST        ));
    RETAILMSG(1,(TEXT("PM_WKDEP_USBHOST         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKDEP_USBHOST     , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKDEP_USBHOST       ));
    RETAILMSG(1,(TEXT("PM_PWSTCTRL_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST  , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST    ));
    RETAILMSG(1,(TEXT("PM_PWSTST_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTST_USBHOST    , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTST_USBHOST  ));
    RETAILMSG(1,(TEXT("PM_PREPWSTST_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PREPWSTST_USBHOST , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PREPWSTST_USBHOST   ));

    RETAILMSG(1,(TEXT("CM_CLKEN_PLL             (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN_PLL        , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN_PLL));
    RETAILMSG(1,(TEXT("CM_CLKEN2_PLL            (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN2_PLL       , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN2_PLL));
    RETAILMSG(1,(TEXT("CM_IDLEST_CKGEN          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST_CKGEN     , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST_CKGEN));
    RETAILMSG(1,(TEXT("CM_IDLEST2_CKGEN         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST2_CKGEN    , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST2_CKGEN));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE_PLL          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE_PLL     , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE_PLL));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE2_PLL         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE2_PLL    , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE2_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL1_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL1_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL1_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL2_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL2_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL2_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL3_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL3_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL3_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL4_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL4_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL4_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL5_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL5_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL5_PLL));
    RETAILMSG(1,(TEXT("CM_CLKOUT_CTRL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKOUT_CTRL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKOUT_CTRL));


    RETAILMSG(1,(TEXT("UHH SYSCONFIG  (0x%x) 0x%x\r\n"),&pUHHRegs->SYSCONFIG, pUHHRegs->SYSCONFIG));
    RETAILMSG(1,(TEXT("UHH SYSSTATUS  (0x%x) 0x%x\r\n"),&pUHHRegs->SYSSTATUS, pUHHRegs->SYSSTATUS));
    RETAILMSG(1,(TEXT("UHH HOSTCONFIG (0x%x) 0x%x\r\n"),&pUHHRegs->HOSTCONFIG,pUHHRegs->HOSTCONFIG));


    RETAILMSG(1,(TEXT("TLL_SHARED_CONF (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_SHARED_CONF,pOMAP_USB_TLL_REGS->TLL_SHARED_CONF));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[0] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0]));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[1] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1]));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[2] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2]));

    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_0 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*0)),*(pbTLL_ULPI+0x800+(0x100*0))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_1 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*1)),*(pbTLL_ULPI+0x800+(0x100*1))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_2 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*2)),*(pbTLL_ULPI+0x800+(0x100*2))));

    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_0 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*0)),*(pbTLL_ULPI+0x801+(0x100*0))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_1 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*1)),*(pbTLL_ULPI+0x801+(0x100*1))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_2 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*2)),*(pbTLL_ULPI+0x801+(0x100*2))));



    RETAILMSG(1,(TEXT("USBTLL_SYSSTATUS (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS,pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
    RETAILMSG(1,(TEXT("USBTLL_SYSCONFIG (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG,pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG));
    RETAILMSG(1,(TEXT("USBTLL_IRQSTATUS (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_IRQSTATUS,pOMAP_USB_TLL_REGS->USBTLL_IRQSTATUS));

    RETAILMSG(1,(TEXT("EHCI: dumpRegs ended\r\n")));
	RETAILMSG(1,(TEXT("#############################################\r\n")));


    MmUnmapIoSpace((PVOID)pUHHRegs, OMAP35XX_UHH_REGS_SIZE);
    MmUnmapIoSpace((PVOID)pOMAP_USBHOST_CM, OMAP_PRCM_USBHOST_CM_REGS_SIZE);
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_CORE_CM_REGS, sizeof(OMAP_PRCM_CORE_CM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_USBHOST_PRM_REGS, sizeof(OMAP_PRCM_USBHOST_PRM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_CLOCK_CONTROL_CM_REGS, sizeof(OMAP_PRCM_CLOCK_CONTROL_CM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_USB_TLL_REGS, sizeof(OMAP_USB_TLL_REGS));
    MmUnmapIoSpace((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));
    MmUnmapIoSpace((PVOID)pbTLL_ULPI, 0x1000);
}
#endif // #if DUMP_REGS
