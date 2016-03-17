// All rights reserved ADENEO EMBEDDED 2010
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
//  File:  soccfg.c
//
//  Configuration file for the device list
//
#include "omap3530.h"
#include "soc_cfg.h"

DWORD SOCGetGPIODeviceByBank(DWORD index)
{
    switch (index)
    {
    case 1: return OMAP_DEVICE_GPIO1;
    case 2: return OMAP_DEVICE_GPIO2;
    case 3: return OMAP_DEVICE_GPIO3;
    case 4: return OMAP_DEVICE_GPIO4;
    case 5: return OMAP_DEVICE_GPIO5;
    case 6: return OMAP_DEVICE_GPIO6;
    default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetSDHCDeviceBySlot(DWORD index)
{
    switch (index)
    {
    case 1: return OMAP_DEVICE_MMC1;
    case 2: return OMAP_DEVICE_MMC2;
    default: return OMAP_DEVICE_NONE;   
    }
}

DWORD SOCGetMCSPIDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return OMAP_DEVICE_MCSPI1;
		case 2:	return OMAP_DEVICE_MCSPI2;
		case 3:	return OMAP_DEVICE_MCSPI3;
		case 4:	return OMAP_DEVICE_MCSPI4;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetMCBSPDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return OMAP_DEVICE_MCBSP1;
		case 2:	return OMAP_DEVICE_MCBSP2;
		case 3:	return OMAP_DEVICE_MCBSP3;
		case 4:	return OMAP_DEVICE_MCBSP4;
		case 5:	return OMAP_DEVICE_MCBSP5;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetMCBSPSidetoneAddress(OMAP_DEVICE_ID deviceID)
{
	switch(deviceID)
	{
	case OMAP_DEVICE_MCBSP2 : return OMAP_MCBSP2_SIDETONE_REGS_PA;
	case OMAP_DEVICE_MCBSP3 : return OMAP_MCBSP3_SIDETONE_REGS_PA;
	case OMAP_DEVICE_MCBSP1 : 
	case OMAP_DEVICE_MCBSP4 : 
	case OMAP_DEVICE_MCBSP5 : return (DWORD)NULL;
	}
	return (DWORD)NULL;
}

DWORD SOCGetI2CDeviceByBus(DWORD index)
{
	switch(index)
	{
		case 1:	return OMAP_DEVICE_I2C1;
		case 2:	return OMAP_DEVICE_I2C2;
		case 3:	return OMAP_DEVICE_I2C3;
	}
	return OMAP_DEVICE_NONE;
}

DWORD SOCGetUartDeviceByIndex(DWORD index)
{
	switch(index)
	{
		case 1:	return OMAP_DEVICE_UART1;
		case 2:	return OMAP_DEVICE_UART2;
		case 3:	return OMAP_DEVICE_UART3;
		case 4:	return OMAP_DEVICE_UART4;
		
	}
	return OMAP_DEVICE_NONE;
}


DWORD SOCGetGPMCAddress(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_GPMC_REGS_PA;
}

DWORD SOCGetIntCtrlAddr()
{
    return OMAP_INTC_MPU_REGS_PA;
}

void SOCGetDSSInfo(DSS_INFO* pInfo)
{
    pInfo->DSSDevice = OMAP_DEVICE_DSS;
    pInfo->TVEncoderDevice = OMAP_DEVICE_TVOUT;
    pInfo->DSS1_REGS_PA     = OMAP_DSS1_REGS_PA;
    pInfo->DISC1_REGS_PA    = OMAP_DISC1_REGS_PA; 
    pInfo->VENC1_REGS_PA    = OMAP_VENC1_REGS_PA;
    pInfo->DSI_REGS_PA      = OMAP_DSI_REGS_PA;
    pInfo->DSI_PLL_REGS_PA  = OMAP_DSI_PLL_REGS_PA;
}

DWORD SOCGetDMADevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_DEVICE_SDMA;
}

DWORD SOCGetVRFBDevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
    return OMAP_DEVICE_VRFB;
}

void SOCGetUSBHInfo(USBH_INFO* pInfo)
{
	pInfo->EHCIDevice				= OMAP_DEVICE_HSUSB2; //OMAP_DEVICE_EHCI; TODO: Verify this value is correct
	pInfo->TLLDevice				= OMAP_DEVICE_USBTLL;
	pInfo->Host1Device				= OMAP_DEVICE_USBHOST1;
	pInfo->Host2Device				= OMAP_DEVICE_USBHOST2;
	pInfo->Host3Device				= OMAP_DEVICE_USBHOST3;
	pInfo->UHH_REGS_PA				= OMAP_USBUHH_REGS_PA;
	pInfo->USBTLL_REGS_PA			= OMAP_USBTLL_REGS_PA;
	pInfo->EHCI_REGS_PA				= OMAP_USBEHCI_REGS_PA;
	pInfo->PRCM_USBHOST_REGS_PA	    = OMAP_PRCM_USBHOST_PRM_REGS_PA;
	pInfo->PRCM_CORE_REGS_PA		= OMAP_PRCM_CORE_PRM_REGS_PA;
}

DWORD SOCGetUSBOTGAddress()
{
	return OMAP_USBHS_REGS_PA;
}

DWORD SOCGetIDCodeAddress()
{
	return OMAP_IDCODE_REGS_PA;
}

DWORD SOCGetHDQDevice(DWORD index)
{
    UNREFERENCED_PARAMETER(index);
	return OMAP_DEVICE_HDQ;
}
