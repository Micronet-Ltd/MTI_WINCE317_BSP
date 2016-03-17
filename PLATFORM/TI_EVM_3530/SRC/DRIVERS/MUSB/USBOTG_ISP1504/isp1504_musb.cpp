//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
// File Name:  
//     isp1504_musb.cpp
// 
// Abstract: EVM1: Provides ISP1504 transceiver support library for musbotg driver.
//           EVM2: Provides ISP1507 transceiver support library for musbotg driver.
// 
// Notes: 
//
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <isp1504_musb.hpp>
#include <omap35xx_isp1504.h>
#include <initguid.h>
#include <twl.h>
#include <batt.h>
#include <oal.h>


#ifndef DEBUG
#define ZONE_OTG_ERROR DEBUGZONE(15)
#define ZONE_OTG_WARN DEBUGZONE(1)
#define ZONE_OTG_FUNCTION DEBUGZONE(2)
#define ZONE_OTG_INIT DEBUGZONE(3)
#define ZONE_OTG_INFO DEBUGZONE(4)
#define ZONE_OTG_THREAD DEBUGZONE(8)
#define ZONE_OTG_HNP DEBUGZONE(9)
#else
#define ZONE_OTG_ERROR DEBUGZONE(15)
#define ZONE_OTG_FUNCTION DEBUGZONE(2)
#define ZONE_OTG_INFO DEBUGZONE(4)
#define ZONE_OTG_HNP DEBUGZONE(9)
#endif

EXTERN_C HSUSBOTGTransceiver * CreateHSUSBOTGTransceiver(PHSMUSB_T pOTG)
{
    return new HSUSBOTGTransceiverIsp1504(pOTG);
}

void    
HSUSBOTGTransceiverIsp1504::AconnNotifHandle(HANDLE hAconnEvent)
{
    m_hAconnEvent = hAconnEvent;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HSUSBOTGTransceiver::AconnNotifHandle (0x%x)\r\n"), hAconnEvent));
}

HSUSBOTGTransceiverIsp1504::HSUSBOTGTransceiverIsp1504(PHSMUSB_T pOTG)
{
    m_pOTG = pOTG;
    
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HSUSBOTGTransceiver called\r\n")));
}


HSUSBOTGTransceiverIsp1504::~HSUSBOTGTransceiverIsp1504()
{
}

//-------------------------------------------------------------------------------------------------

BOOL HSUSBOTGTransceiverIsp1504::SetLowPowerMode()
{
    // nothing to do
    return TRUE;
    
    return ISP1504_SetLowPowerMode(m_pOTG);
}

BOOL HSUSBOTGTransceiverIsp1504::IsSE0()
{
    UCHAR debug_r;
        
    return FALSE;

    // USB clock will be running when this function is called
    debug_r = ISP1504_ReadReg(m_pOTG, ISP1504_DEBUG_R);
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("IsSE0 called with line state = 0x%x\r\n"), (debug_r & 0x3)));

    return (((debug_r & 0x3) == 0x00) ? TRUE: FALSE);
}

void HSUSBOTGTransceiverIsp1504::DumpULPIRegs()
{
    //DumpIsp1504Regs(m_pOTG);
}

// used to reset transceiver
void HSUSBOTGTransceiverIsp1504::Reset()
{
    // reset the PHY
    //ISP1504_WriteReg(m_pOTG, ISP1504_FUNCTION_CTRL_S, 0x20);
    //Sleep(100);

}

// used to reset transceiver
void HSUSBOTGTransceiverIsp1504::Resume()
{
}
