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
//------------------------------------------------------------------------------
//
//  File: otgtransceiver_musb.h
//

#ifndef __OTGTRANSCEIVER_MUSB_H
#define __OTGTRANSCEIVER_MUSB_H

#include <omap_musbotg.h>

class HSUSBOTGTransceiver
{
public:
    HSUSBOTGTransceiver() {};
    ~HSUSBOTGTransceiver() {};
    virtual void EnableVBusDischarge(BOOL fDischarge) = 0;
    virtual void SetVBusSource(BOOL fBat) = 0;
    virtual void EnableWakeupInterrupt(BOOL fEnable) = 0;
    virtual void AconnNotifHandle(HANDLE hAconnEvent) = 0;
	virtual BOOL ProcessInterrupts(BYTE* pUSBInt, USHORT* pTxInt, USHORT* pRxInt) = 0;
	virtual void EnableInterrupts(DWORD dwEPMask) = 0;
	virtual void DisableInterrupts(DWORD dwEPMask) = 0;
	virtual void ClearInterrupts() = 0;
	virtual void SetVBUSPower(BOOL bOn) = 0;
	virtual BOOL ProcessInterrupts() = 0;
    virtual BOOL UpdateUSBVBusSleepOffState(BOOL fActive) = 0;
    virtual BOOL SupportsTransceiverWakeWithoutClock() = 0;
    virtual BOOL SetLowPowerMode() = 0;
    virtual BOOL IsSE0() = 0;
    virtual void DumpULPIRegs() = 0;
    virtual void Reset() = 0;
    virtual BOOL ResetPHY() = 0;
    virtual BOOL IsADeviceConnected() = 0;
    virtual BOOL IsBDeviceConnected() = 0;
};

EXTERN_C HSUSBOTGTransceiver * CreateHSUSBOTGTransceiver(PHSMUSB_T pOTG);

#endif // __OTGTRANSCEIVER_MUSB_H

