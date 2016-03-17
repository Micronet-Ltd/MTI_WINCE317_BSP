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
// Copyright (c) 2007, 2013 Micronet Ltd. All rights reserved.
// by Vladimir Zatulovsky
//
//  File:  kitl.c
//

//-------------------------------------------------------------------------------

#include <bsp.h>
#include <bus.h>
#include <kitlprot.h>
#include <kitl_cfg.h>
#include <devload.h>
#include <oal_kitlex.h>
#include <bus.h>

////------------------------------------------------------------------------------
//  Local definition
//
#ifndef HKEY_LOCAL_MACHINE
#define HKEY_LOCAL_MACHINE          ((HKEY)(ULONG_PTR)0x80000002)
#endif

int s_bKitlActive = 0;

//------------------------------------------------------------------------------
//
UINT32 OEMKitlEnableClocks(BOOL bEnable)
{
    return 1;
}

//------------------------------------------------------------------------------
//Currently only support Ethernet Kitl, and we didn't coniser wake-up
//
BOOL OEMKitlStartup()
{
    OAL_KITL_ARGS *pArgs, args;
    CHAR *szDeviceId;
    OMAP_GPIO_REGS *pGPIORegs = OALPAtoUA(BSP_ETHER_GPIO_PA);


    KITLSetDebug(ZONE_ERROR | ZONE_WARNING | ZONE_INIT | ZONE_KITL_OAL | ZONE_KITL_ETHER | 0);


    KITL_RETAILMSG(ZONE_INIT, ("+OALKitlStart\r\n"));

    // First get boot args and device id
    pArgs = (OAL_KITL_ARGS*)OALArgsQuery(OAL_ARGS_QUERY_KITL);

    // If we don't get kitl arguments use default
    if(!pArgs)
	{
        KITL_RETAILMSG(ZONE_WARNING, ("WARN: Boot arguments not found, use defaults\r\n"));
        memset(&args, 0, sizeof(args));
        args.flags = OAL_KITL_FLAGS_ENABLED; //|OAL_KITL_FLAGS_DHCP;
        args.devLoc.IfcType = Internal;
        args.devLoc.BusNumber = 0;
        args.devLoc.LogicalLoc = OMAP_USBHS_REGS_PA;
        pArgs = &args;
	}

    // We always create device name
    szDeviceId = BSP_DEVICE_PREFIX;
    pArgs->flags |= OAL_KITL_FLAGS_EXTNAME;
    
    // Finally call KITL library
    s_bKitlActive = OALKitlInit(szDeviceId, pArgs, g_kitlDevices);

    // If it failed or KITL is disabled
    if(!s_bKitlActive || !(pArgs->flags & OAL_KITL_FLAGS_ENABLED))
	{
		KITL_RETAILMSG(ZONE_ERROR, ("-OALKitlStart(failure)\r\n"));
		return 0;
	}
    // enable kitl interrupts
    s_bKitlActive = 1;
    OEMKitlEnable(1);
    
    KITL_RETAILMSG(ZONE_KITL_OAL, ("-OALKitlStart(succeed)\r\n"));

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALKitlInitRegistry
//
VOID OALKitlInitRegistry()
{
    DEVICE_LOCATION devLoc;

    // Get KITL device location
    if(!OALKitlGetDevLoc(&devLoc))
		return;

    // Depending on device bus
    switch(devLoc.IfcType)
	{
        case Internal:
            switch(devLoc.LogicalLoc)
			{
                case OMAP_USBHS_REGS_PA:
                    // Disable USB, enable ethernet
                    OEMEthernetDriverEnable(0);
                    OEMUsbDriverEnable(0);
                    break;   
                default:
                    // Enable both USB and ethernet
                    OEMEthernetDriverEnable(1);
                    OEMUsbDriverEnable(1);
                    break;
			}
            break;
        }

    return;
}

//------------------------------------------------------------------------------

DWORD OEMEthGetSecs()
{
    return OALGetTickCount()/1000;
}

//------------------------------------------------------------------------------
//
//  Function:     OEMKitlEnable
//
//  Enables/disables kitl.  Necessary to enable/disable gpio pin for kitl
//  interrupts. 
//
BOOL OEMKitlEnable(BOOL bEnable)
{
    if(!s_bKitlActive)
		return 0;

    if(bEnable)
        OEMKitlEnableClocks(0);
    else
        OEMKitlEnableClocks(1);

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  OALGetTickCount
//
UINT32 OALGetTickCount()
{
    OMAP_GPTIMER_REGS *pGPTimerRegs = OALPAtoUA(OMAP_GPTIMER1_REGS_PA);
    return INREG32(&pGPTimerRegs->TCRR) >> 5;
}

//------------------------------------------------------------------------------
//
//  Function:  OEMKitlIoctl
//
//  This function handles KITL IOCTL codes.
//
//
BOOL OEMKitlIoctl(DWORD code, VOID * pInBuffer, DWORD inSize, VOID * pOutBuffer, DWORD outSize, DWORD * pOutSize)
{
    BOOL fRet = FALSE;
    switch (code) {
    case IOCTL_HAL_INITREGISTRY:
        OALKitlInitRegistry();
        // Leave return code false and set last error to ERROR_NOT_SUPPORTED
        // This allows code to fall through to OEMIoctl so IOCTL_HAL_INITREGISTRY can be 
        // handled there as well.
        NKSetLastError(ERROR_NOT_SUPPORTED);
        break;
    default:
        fRet = OALIoCtlVBridge (code, pInBuffer, inSize, pOutBuffer, outSize, pOutSize);
    }

    return fRet;
}
