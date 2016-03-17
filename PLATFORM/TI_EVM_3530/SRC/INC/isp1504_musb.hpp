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
//------------------------------------------------------------------------------
//
//  File: isp1504_musb.hpp
//

#include "transceiver_musb.hpp"

class HSUSBOTGTransceiverIsp1504 : public HSUSBOTGTransceiver
{
public:
    HSUSBOTGTransceiverIsp1504(PHSMUSB_T pOTG);
    ~HSUSBOTGTransceiverIsp1504();
    void EnableVBusDischarge(BOOL fDischarge) {};
    void SetVBusSource(BOOL fBat) {};
    void EnableWakeupInterrupt(BOOL fEnable) {};
    void AconnNotifHandle(HANDLE hAconnEvent);
    BOOL UpdateUSBVBusSleepOffState(BOOL fActive) {return TRUE;};
    BOOL SupportsTransceiverWakeWithoutClock() {return FALSE;};
    BOOL SetLowPowerMode();
    BOOL IsSE0();
    void DumpULPIRegs();
    void Reset();
    void Resume();
    BOOL ResetPHY() {return TRUE;};
    BOOL IsADeviceConnected() {return FALSE;};
    BOOL IsBDeviceConnected() {return FALSE;};
  
private:
    HANDLE m_hAconnEvent;
    PHSMUSB_T m_pOTG;
};
