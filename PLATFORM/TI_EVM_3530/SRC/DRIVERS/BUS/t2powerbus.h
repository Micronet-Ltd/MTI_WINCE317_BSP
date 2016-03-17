/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
#ifndef __T2POWERBUS_H
#define __T2POWERBUS_H


//-----------------------------------------------------------------------------
// T2 power enumerations
//
typedef enum T2_POWER {
    T2_EXTPOWER_VRMMC1,
    T2_EXTPOWER_VRMMC2,
    T2_EXTPOWER_VRUSB,
    T2_EXTPOWER_VRVUSB,
    T2_EXTPOWER_VRI2S,
    T2_EXTPOWER_LCD,
    T2_EXTPOWER_VRCAM,
    T2_EXTPOWER_VDAC,  
    T2_POWER_COUNT,
    T2_POWER_NONE = -1
} T2_POWER;

//-----------------------------------------------------------------------------
//
//  Class:  T2PowerBus_t
//
class T2PowerBus_t : public omapPowerBus_t
{
friend omapPowerBus_t;

    //-------------------------------------------------------------------------
protected:
    HANDLE                  m_hTwl;             // Handle to TWL driver

    //-------------------------------------------------------------------------
public:     
    BOOL 
    InitializePowerBus(
        );
         
    void 
    DeinitializePowerBus(
        );

    virtual 
    DWORD 
    PreDevicePowerStateChange(
        DWORD devId, 
        CEDEVICE_POWER_STATE oldPowerState,
        CEDEVICE_POWER_STATE newPowerState
        );
    
    virtual 
    DWORD 
    PostDevicePowerStateChange(
        DWORD devId, 
        CEDEVICE_POWER_STATE oldPowerState,
        CEDEVICE_POWER_STATE newPowerState
        );

protected:
    T2PowerBus_t(
        );

    BOOL
    WriteTwlReg(
        DWORD address,
        UCHAR value
        );

    BOOL
    ReadTwlReg(
        DWORD address,
        UCHAR& value
        );

    BOOL 
    SendSingularPBMessage(
        UCHAR power_res_id,
        UCHAR res_state
        );
    
    BOOL 
    SetT2USBXvcrActive(
        BOOL fActive
        );

    BOOL 
    UpdateUSBVBusSleepOffState(
        UCHAR state
        );

    BOOL 
    WakeT2USBPhyDPLL(
        );

    BOOL
    UsbFnAttach(
        BOOL state
        );
        
    BOOL 
    SelectPowerResVolt(
        DWORD res_vsel_reg_offset,
        UCHAR reg_data
        );
        
    BOOL 
    SetVmmc1PowerOnOff(
        UCHAR power_res_state
        );
        
    BOOL 
    SetVmmc2PowerOnOff(
        UCHAR power_res_state
        );        
};

//-----------------------------------------------------------------------------
#endif //__T2POWERBUS_H

