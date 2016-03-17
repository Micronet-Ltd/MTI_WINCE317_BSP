// All rights reserved ADENEO EMBEDDED 2010
//
// Copyright (c) MPC Data Limited 2010.  All rights reserved.
//
//
// Module Name: 
//        usbotgpdd.h
//
// Abstract:
//        USB OTG Driver Header.
//
#pragma once

#include <usbotg.hpp>
#include <usbotgxr.hpp>

class CAM35xOTG :public USBOTG{
public:
    CAM35xOTG (LPCTSTR lpActivePath) ;
    ~CAM35xOTG () ;
    BOOL Init() ;
    BOOL PostInit() ;

    // Overwrite 
    virtual BOOL IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut);
    virtual BOOL PowerUp();
    virtual BOOL PowerDown();
    virtual BOOL UpdatePowerState();

    // OTG PDD Function.
    BOOL    SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus);
    BOOL    DischargVBus()  { return TRUE; };
    BOOL    NewStateAction(USBOTG_STATES usbOtgState , USBOTG_OUTPUT usbOtgOutput) ;
    BOOL    IsSE0();
    BOOL    UpdateInput() ;
    //BOOL    StateChangeNotification (USBOTG_TRANSCEIVER_STATUS_CHANGE , USBOTG_TRANSCEIVER_STATUS);
    USBOTG_MODE UsbOtgConfigure(USBOTG_MODE usbOtgMode ) { return usbOtgMode; };
	void	StartUSBModule();

protected:
    HANDLE  m_hParent;
    LPTSTR  m_ActiveKeyPath;
    HANDLE  m_hPollThread;
    OMAP_SYSC_GENERAL_REGS* m_pSysConfRegs;
    CSL_UsbRegs  *m_pUsbRegs;

    static DWORD CALLBACK PollThread(void *data);
    VOID HostMode(BOOL start);
    VOID FunctionMode(BOOL start);
    HANDLE m_hPollThreadExitEvent;
    DWORD  m_PollTimeout;
    BOOL   m_bUSB11Enabled;
    BOOL   m_bEnablePolling;
	BOOL   m_bFunctionMode;
    BOOL   m_InFunctionModeFn;
	BOOL   m_bHostMode;

    void ChipCfgLock(BOOL lock);

    friend DWORD UnloadDrivers(CAM35xOTG* pOTG);
private:
};
