// All rights reserved ADENEO EMBEDDED 2010
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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
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
//  File: omap_usbotg.hpp
//

#include <omap3530_musbotg.h>
#include <batt.h>
#pragma once

typedef struct {
    DWORD DisableHighSpeed;
    DWORD startupTimeout;
    TCHAR *szChargerNameEvt;
    DWORD DMAIrq;
    DWORD dwActiveSyncDelay;
    DWORD vbusDischargeTime;
    DWORD idgndIstPrio;
    DWORD otgIstPrio;
    DWORD dmaIstPrio;
} MUsbOTGRegCfg_t;

class OMAPMHSUSBOTG;

class OMAPMHSUSBIdGnd : public CLockObject, public CMiniThread {
public:
    OMAPMHSUSBIdGnd(OMAPMHSUSBOTG *pUsbOtg);
    virtual ~OMAPMHSUSBIdGnd();
    virtual BOOL Init();
private:
    virtual DWORD ThreadRun();
    OMAPMHSUSBOTG * const  m_pUsbOtg;    
    HANDLE m_hIdGndIntrEvent;
    BOOL    m_fTerminated;

public:

};

class OMAPMHSUSBDMA : public CLockObject, public  CMiniThread {
public:
    OMAPMHSUSBDMA(OMAPMHSUSBOTG * pUsbOtg, DWORD dwIrq);
    virtual ~OMAPMHSUSBDMA();
    virtual BOOL    Init() ;
private:
    virtual DWORD   ThreadRun(); // User have to implement this function.
    BOOL    m_fTerminated;
    OMAPMHSUSBOTG * const  m_pUsbOtg;
    HANDLE  m_hDMAIntrEvent;      // Interrupt Event due to DMA
    DWORD   m_dwDMASysIntr;       // SysInterrupt of DMA
    DWORD   m_dwDMAIrq;           // Interrupt for DMA

};

class OMAPMHSUSBOTG : public USBOTG,  public  CMiniThread 
{
public:
    OMAPMHSUSBOTG (LPCTSTR lpActivePath) ;
    ~OMAPMHSUSBOTG () ;

    // virtual functions from USBOTG
    virtual BOOL    Init();
    virtual BOOL    PostInit() ;
    virtual DeviceFolder * CreateFunctionDeviceFolder(LPCTSTR lpBusName,LPCTSTR lpTemplateRegPath,DWORD dwBusType, DWORD BusNumber,DWORD DeviceNumber,DWORD FunctionNumber,HANDLE hParent,DWORD dwMaxInitReg=MAX_INIT_REG,LPCTSTR lpDeviceBusName = NULL) ;
    virtual DeviceFolder * CreateHostDeviceFolder(LPCTSTR lpBusName,LPCTSTR lpTemplateRegPath,DWORD dwBusType, DWORD BusNumber,DWORD DeviceNumber,DWORD FunctionNumber,HANDLE hParent,DWORD dwMaxInitReg=MAX_INIT_REG,LPCTSTR lpDeviceBusName = NULL) ;
    virtual BOOL    LoadUnloadUSBFN(BOOL fLoad);
    virtual BOOL    LoadUnloadHCD(BOOL fLoad);
    virtual USBOTG_MODE UsbOtgConfigure(USBOTG_MODE usbOtgMode );
    virtual BOOL    EventNotification();
    
    // State Machine was implememted by Hardware.
    virtual BOOL    NewStateAction(USBOTG_STATES /*usbOtgState*/ , USBOTG_OUTPUT /*usbOtgOutput*/) ;
    virtual BOOL    UpdateInput() ;
    virtual BOOL    SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus);
    virtual BOOL    DischargVBus()  { return TRUE; };
    virtual BOOL    IsSE0();

    virtual BOOL    OTGUSBFNNotfyActive(LPCTSTR lpDeviceName, BOOL fActive);
    virtual BOOL    OTGHCDNotfyAccept(LPCTSTR lpDeviceName, BOOL fActive);
    virtual DWORD   OTG_ConfigISR_stage1();
    virtual DWORD   OTG_ConfigISR_stage2();
    virtual void PowerDownDisconnect();
    virtual DWORD   OTG_ProcessEP0();
    virtual DWORD   OTG_ProcessEPx(UCHAR endpoint, BOOL IsRx);
    virtual DWORD   GetMode();
    virtual BOOL    IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
                        PDWORD pdwActualOut);
    PHSMUSB_T GetHsMusb() { return m_pOTG;};
    virtual BOOL    StartUSBClock(BOOL fInc);
    virtual BOOL    StopUSBClock();
    virtual BOOL    PowerUp();
    virtual BOOL    PowerDown();
    virtual void    ResetEndPoints();
    virtual BOOL    EnableSuspend(BOOL fSet);
    virtual void    UpdateBatteryCharger(DWORD dwCurValue);
    virtual BOOL    SoftResetMUSBController(BOOL bCalledFromPowerThread = FALSE);
    void    DelayByRegisterRead();
    BOOL    LinkRecoveryProcedure1();
    BOOL    LinkRecoveryProcedure2();
    virtual BOOL    SoftResetULPILink();
    virtual BOOL    ContextRestore();
    virtual BOOL    SetPowerState(CEDEVICE_POWER_STATE reqDx);
    CEDEVICE_POWER_STATE	m_Dx;
    DWORD   m_dwStatus;
    HSUSBOTGTransceiver *m_pTransceiver;
    MUsbOTGRegCfg_t m_OTGRegCfg;

    //virtual BOOL    PowerUp() { if (!m_IsThisPowerManaged) OtgActive() ; return TRUE; };
    //virtual BOOL    PowerDown() { if (!m_IsThisPowerManaged) OtgIdle(); return TRUE; };
    //virtual BOOL    UpdatePowerState();

    UCHAR ReadULPIReg(PHSMUSB_T pOTG, UCHAR idx);

    CRITICAL_SECTION m_csUSBClock;

private:
    // virtual function from CMiniThread
    virtual DWORD   ThreadRun(); // User have to implement this function.

    BOOL    MapHardware() ;
    
    // OTG Register.

    CRegistryEdit m_ActiveKey;
    LPTSTR  m_ActiveKeyPath;
    HANDLE  m_hParent;
    DWORD   m_dwSysIntr;          // Sysinterrupt of HS_USB_MC_NINT
    DWORD   m_dwIrq;              // HS_USB_MC_NINT
    HANDLE  m_hIntrEvent;         // Interrupt Event due to HS_USB_MC_NINIT
    PHSMUSB_T    m_pOTG;          // Pointer to PHSMUSB_T
    OMAPMHSUSBDMA    *m_pDMA;  // Object pointer to OMAPMHSUSBDMA
    OMAPMHSUSBIdGnd *m_pIdGnd;
    BOOL    m_bUSBClockEnable;
    DWORD   m_dwUSBUsageCount;
    BOOL    m_bRequestSession;    
    BOOL    m_bEnableSuspend;
    
    BOOL    m_bHNPEnable;
    DWORD   m_timeout;
    HANDLE  m_BatteryChargeEvent;
    DWORD   m_BatteryChargeStatus;
    BOOL    m_bSessionDisable;
    BOOL    m_bIncCount;
    BOOL    m_bExtendOTGSuspend;
    BOOL    m_bOTGReady;
    BOOL    m_disconnected;

    BOOL    m_bSuspendTransceiver;
    BOOL    m_handleVBUSError;
    DWORD   m_dwbTypeConnector;
    BOOL    m_fDelayRequired;
    BOOL    m_fPowerRequest;
};

class OMAPMHSUsbClientDeviceFolder : public DeviceFolder 
{
public:
    OMAPMHSUsbClientDeviceFolder(DWORD dwLocation, 
                                    LPCTSTR lpBusName,
                                    LPCTSTR lpTemplateRegPath,
                                    DWORD dwBusType, 
                                    DWORD BusNumber, 
                                    DWORD DeviceNumber, 
                                    DWORD FunctionNumber, 
                                    HANDLE hParent, 
                                    DWORD dwMaxInitReg = MAX_INIT_REG, 
                                    LPCTSTR lpDeviceBusName = NULL
                                    )
            
    :   DeviceFolder(lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber,FunctionNumber, hParent, dwMaxInitReg, lpDeviceBusName),
        m_dwDevicePhysAddr(dwLocation)
    {
        DEBUGMSG(1, (TEXT("OMAPMHSUsbClientDeviceFolder(%s, %s, 0x%x, %d, %d, %d ...\r\n"),
            lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber, FunctionNumber));
    };
    
    virtual ~OMAPMHSUsbClientDeviceFolder() 
    {
        SetPowerState(D4);
    };
    
    virtual BOOL LoadDevice() 
    {
        SetPowerState(D0);
        BOOL bReturn = DeviceFolder::LoadDevice();
        if (!bReturn) 
        { 
            // Loading Fails
            SetPowerState(D4);
        }
        return bReturn;
    }
    
    virtual BOOL UnloadDevice() 
    {
        BOOL bReturn = DeviceFolder::UnloadDevice();
        if (bReturn) 
        { 
            // Unloading Success.
            SetPowerState(D4);
        }
        return bReturn;
    }

    virtual BOOL SetPowerState(CEDEVICE_POWER_STATE newPowerState) 
    {
        Lock();
        m_CurPowerState = newPowerState;
        // Should put device into idle according to m_dwDevicePhysAddr
        switch (m_CurPowerState) {
        case D0:
        case D1:
        case D2:
            // We may want to disable host or function idle.
            break;
        }

        switch (m_CurPowerState) {
        case D3:
        case D4:
            // We may want to enable USB function idle.
            break;
        }
        Unlock();
        return TRUE;
    }
    
protected:
    DWORD   m_dwDevicePhysAddr ;
};



