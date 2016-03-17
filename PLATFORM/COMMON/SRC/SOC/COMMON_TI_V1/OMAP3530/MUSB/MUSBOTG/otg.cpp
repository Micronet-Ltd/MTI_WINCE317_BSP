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
// Module Name:  
//     OTG.cpp
// 
// Abstract: Provides Library for OTG Module.
// 
// Notes: 
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
#pragma warning(push)
#pragma warning(disable : 4100 4245 4512)
#include <windows.h>
#include <types.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <usbotg.hpp>
#include <initguid.h>
#include "transceiver_musb.hpp"
#include "otg.hpp"
#include <omap_musbcore.h>
#include <omap3530_musbcore.h>
#include <omap3530_musbotg.h>
#include <musbioctl.h>
#include <pm.h>
#include <omap3530.h>
#include <oal.h>
#include <initguid.h>
#include <nkintr.h>
#include <sdk_gpio.h>
#include <sdk_padcfg.h>
#pragma warning(pop)

// map zones used in this driver to actual zones defined by MDD
#define ZONE_OTG_INFO   ZONE_OTG_STATE
#define ZONE_OTG_HNP    ZONE_OTG_STATE

#define STATUS_CONNECT              0x01
#define STATUS_DISCONN_REQUEST      0x02
#define STATUS_SUSPEND              0x04
#define STATUS_DISCONN_COMPLETE     0x08
#define STATUS_HNP_SESSION_INIT     0x10
#define STATUS_HNP_SESSION_PROCESS  0x20
#define STATUS_HNP_SESSION_MASK     0x30
#define STATUS_WAIT_HOST_DISCONN_COMPLETE   0x40
#define STATUS_SESSION_RESTART      0x80
#define STATUS_RETENTION_WAKEUP     0x100

#define DO_SESSCHK_TIMEOUT 5000 // Note that this is required to set such a long time or it cannot handle SRP properly
#define DO_USBCLK_TIMEOUT  3
#define DO_SUSPEND_TIMEOUT 3000 // We need to change to 3 sec as to handle the SmartPhone suspend and then detach case.
                                // If it is too short, it may affect the others e.g. HNP or device mode.
#define DO_USBHOST_TIMEOUT 3
#define DO_DISCONNECT_TIMEOUT 1000
#define DO_INACTIVITY_TIMEOUT 10000
#define DO_TRANSCEIVER_SUSPEND_TIMEOUT 3015
#define REG_VBUS_CHARGE_B_TYPE_CONNECTOR TEXT("BTYPE")
#define REG_USBFN_DRV_PATH TEXT("Drivers\\BuiltIn\\MUSBOTG\\USBFN")

#define REG_USBFN_DEFAULT_FUNC_DRV_PATH TEXT("Drivers\\USB\\FunctionDrivers")
#define REG_USBFN_DEFAULT_FUNC_DRV TEXT("DefaultClientDriver")

#define IDGND_IST_PRIORITY          131
#define OTG_IST_PRIORITY            130
#define DMA_IST_PRIORITY            129

#define ULPI_VENDORID_LOW_R         0

#define SUSPEND_RESUME_DEBUG_ENABLE	FALSE

static TCHAR const* pChargerNameEvt=_T("vbus.power.event");

OMAPMHSUSBOTG *gpHsUsbOtg;

static const DEVICE_REGISTRY_PARAM s_deviceRegParams[] = 
{
    {
        L"startupTimeout", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, startupTimeout),
            fieldsize(MUsbOTGRegCfg_t, startupTimeout), (void *) 5000
    }, 
    {
        L"DisableHighSpeed", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, DisableHighSpeed),
            fieldsize(MUsbOTGRegCfg_t, DisableHighSpeed), (void *) 1
    }, 
    {
        L"USBChargerNotify", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, szChargerNameEvt),
            fieldsize(MUsbOTGRegCfg_t, szChargerNameEvt), (void *) pChargerNameEvt
    }, 
    {
        L"DMAIrq", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, DMAIrq),
            fieldsize(MUsbOTGRegCfg_t, DMAIrq), (void *) 93
    },
    {
        L"VbusDischargeTime", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, vbusDischargeTime),
            fieldsize(MUsbOTGRegCfg_t, vbusDischargeTime), (void *) 100
    },
    {
        L"idgndIstPriority", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, idgndIstPrio),
            fieldsize(MUsbOTGRegCfg_t, idgndIstPrio), (void *) IDGND_IST_PRIORITY
    }, 
    {
        L"otgIstPriority", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, otgIstPrio),
            fieldsize(MUsbOTGRegCfg_t, otgIstPrio), (void *) OTG_IST_PRIORITY
    },
    {
        L"dmaIstPriority", PARAM_DWORD, FALSE, offset(MUsbOTGRegCfg_t, dmaIstPrio),
            fieldsize(MUsbOTGRegCfg_t, dmaIstPrio), (void *) DMA_IST_PRIORITY
    }
    
};

CEDEVICE_POWER_STATE g_busDx = D4;

//------------------------------------------------------------------------------
static VOID UpdateDevicePower(HANDLE hBus, CEDEVICE_POWER_STATE reqDx, VOID *pvoid)
{
	UNREFERENCED_PARAMETER(pvoid);

    if (g_busDx == reqDx)
        return;
    g_busDx = reqDx;
    SetDevicePowerState(hBus, g_busDx, NULL);
}

//------------------------------------------------------------------------------
//
//  Function:  CreateUSBOTGObject
//
//  Routine Description:
//
//      Called by MDD in OTG_Init(). 
//      This function instantiates the USBOTG object.
//      It will be used by OTG_xxxx driver functions.
//
//  Arguments:
//
//      lpActivePath    :   Registry Path
//
//  Return values:
//
//      USBOTG object 
//      NULL if failed.
//
USBOTG *    
CreateUSBOTGObject(
                   LPTSTR lpActivePath
                   ) 
{

    gpHsUsbOtg = new OMAPMHSUSBOTG (lpActivePath) ;
    return gpHsUsbOtg;
}

//------------------------------------------------------------------------------
//
//  Function:  DeleteUSBOTGObject()
// 
//  Routine Description:
//
//      Called by MDD in OTG_Deinit(). 
//
//  Arguments:
//
//      pUsbOtg - USBOTG object to be deleted.
//
//  Return values:
//
//      Nothing
//
void
DeleteUSBOTGObject(
                   USBOTG * pUsbOtg
                   )
{
    if (pUsbOtg)
        delete pUsbOtg;
}

OMAPMHSUSBIdGnd::OMAPMHSUSBIdGnd(OMAPMHSUSBOTG *pUsbOtg)
:   m_pUsbOtg (pUsbOtg)
,   CMiniThread( 0, TRUE)
{
    m_hIdGndIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hIdGndIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("OMAPMHSUSBIdGnd:Failed to create event\r\n")));
        return;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("-OMAPMHSUSBIdGnd\r\n")));

}

OMAPMHSUSBIdGnd::~OMAPMHSUSBIdGnd()
{    
    // Terminationg thread.
    m_fTerminated = TRUE;
    ThreadTerminated(1000);
}

BOOL OMAPMHSUSBIdGnd::Init()
{    
    m_fTerminated = FALSE;
    ThreadStart() ;    
    return TRUE;
    
}

DWORD OMAPMHSUSBIdGnd::ThreadRun()
{
    
    PHSMUSB_T pOTG = m_pUsbOtg->GetHsMusb();    

    m_pUsbOtg->m_pTransceiver->AconnNotifHandle(m_hIdGndIntrEvent);
    
    CeSetThreadPriority(GetCurrentThread(), m_pUsbOtg->m_OTGRegCfg.idgndIstPrio);

    while (!m_fTerminated)
    { 
        DWORD rc = WaitForSingleObject(m_hIdGndIntrEvent, INFINITE);         
        if (rc != WAIT_OBJECT_0)
        {
            DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Something wrong with IdGndIntrEvent\r\n")));
            m_fTerminated = TRUE;
        }
        else
        {                 
            DEBUGMSG(ZONE_OTG_FUNCTION,(L"***### OMAPMHSUSBIdGnd::ThreadRun calling SessionRequest")); 
            m_pUsbOtg->m_dwStatus |= STATUS_RETENTION_WAKEUP;
            SetEvent(pOTG->hSysIntrEvent);
       }
    }
    return 1;
}

OMAPMHSUSBDMA::OMAPMHSUSBDMA(OMAPMHSUSBOTG * pUsbOtg, DWORD dwDMAIrq)
:   m_pUsbOtg (pUsbOtg)
,   CMiniThread( 0, TRUE )
{
    DWORD dwDMASysIntr;

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
           &dwDMAIrq,
           sizeof(dwDMAIrq),
           &dwDMASysIntr,
           sizeof(dwDMASysIntr),
           NULL))
    {
        DEBUGMSG(ZONE_OTG_ERROR, (L"ERROR: OTG_Init: "  L"Failed map OTG controller interrupt\r\n" ));
        return;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("DMAIRQ=0x%x, SYSINTR=0x%x\r\n"), dwDMAIrq, dwDMASysIntr));
    m_dwDMASysIntr = dwDMASysIntr;
    m_dwDMAIrq = dwDMAIrq;

    m_hDMAIntrEvent = CreateEvent(0, FALSE, FALSE, NULL);
    if (m_hDMAIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to create DMA Interrupt Event handle\r\n")));
        // We need to do clean up here
        return;
    }


    if (!InterruptInitialize(m_dwDMASysIntr, m_hDMAIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to initialize the DMA interrupt routine\r\n")));
        // We need to do clean up here
        return;
    }    

    
}

OMAPMHSUSBDMA::~OMAPMHSUSBDMA()
{    
    // Terminationg thread.
    ThreadTerminated(1000);
}
BOOL OMAPMHSUSBDMA::Init()
{
    m_fTerminated = FALSE;
    ThreadStart() ;    
    return TRUE;
    
}

DWORD OMAPMHSUSBDMA::ThreadRun()
{    
    CeSetThreadPriority(GetCurrentThread(), m_pUsbOtg->m_OTGRegCfg.dmaIstPrio);

    while (!m_fTerminated)
    {
        DWORD rc = WaitForSingleObject(m_hDMAIntrEvent, INFINITE);         
        if (rc != WAIT_OBJECT_0)
        {
            DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Something wrong with DMA IntrEvent\r\n")));
            m_fTerminated = TRUE;
        }
        else
        {
            PHSMUSB_T pOTG = m_pUsbOtg->GetHsMusb();
            DWORD reg = INREG32(&pOTG->pUsbDmaRegs->Intr);

            if (pOTG->dwPwrMgmt == MODE_SYSTEM_SUSPEND)
			{
	            RETAILMSG(1, (L"OTG DMA ThreadRun MODE_SYSTEM_SUSPEND\r\n"));
                Sleep(100);
			    continue;
			}

	        if (pOTG->dwPwrMgmt == MODE_SYSTEM_RESUME)
			{
	            RETAILMSG(1, (L"OTG DMA ThreadRun MODE_SYSTEM_RESUME\r\n"));
                Sleep(100);
			    continue;
			}

            int i = 0;
            //Read the Interrupt Register and see
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("DMA Interrupt Register = 0x%x\r\n"),reg));
            while (i < MAX_DMA_CHANNEL)
            {
                if (reg & INTR_CH(i))
                {
                    if (m_pUsbOtg->GetMode() == HOST_MODE)
                        pOTG->pFuncs[HOST_MODE-1]->ProcessDMA(pOTG, (UCHAR)i);
                    else if (m_pUsbOtg->GetMode() == DEVICE_MODE)
                        pOTG->pFuncs[DEVICE_MODE-1]->ProcessDMA(pOTG, (UCHAR)i);
                    else
                    {
                        DEBUGMSG(ZONE_OTG_INFO, (TEXT("Ignore DMA channel %d interrupt\r\n"), i));
                    }
                }
                i++;
            }
        }
        InterruptDone(m_dwDMASysIntr);
    }
    return 1;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG Constructor
//
//  Routine Description:
//
//      This is the constructor for OMAPMHSUSBOTG
//
//  Arguments:
//
//      lpActivePath - Registry path
//
//  Return values:
//
//      nothing
//
OMAPMHSUSBOTG::OMAPMHSUSBOTG(LPCTSTR lpActivePath)
:   USBOTG(lpActivePath)
,   CMiniThread(0, TRUE)
,   m_ActiveKey(HKEY_LOCAL_MACHINE,lpActivePath)
,   m_bSessionDisable(FALSE)
{
    if (GetDeviceRegistryParams(
        lpActivePath, &m_OTGRegCfg, dimof(s_deviceRegParams), s_deviceRegParams
        ) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (L"ERROR: OMAPMHSUSBOTG: "
            L"Failed read registry parameters\r\n"
            ));
    }

    m_hParent = CreateBusAccessHandle(lpActivePath);
    if (m_hParent == NULL)
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to get Bus Handle\r\n")));

    m_ActiveKeyPath = NULL;
    if (lpActivePath) 
    {
        DWORD dwLength = _tcslen(lpActivePath) + 1;
        m_ActiveKeyPath = new TCHAR[dwLength];
        if (m_ActiveKeyPath)
            StringCchCopy(m_ActiveKeyPath, dwLength, lpActivePath);
    }

    InitializeCriticalSection(&m_csUSBClock);
    m_bHNPEnable = FALSE;
    m_pDMA = NULL;
    m_pIdGnd = NULL;
    m_bOTGReady = FALSE;
    m_disconnected = TRUE;
    m_fDelayRequired = TRUE;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG Destructor
//
//  Routine Description:
//
//      This is the destructor for OMAPMHSUSBOTG
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      nothing
//
OMAPMHSUSBOTG::~OMAPMHSUSBOTG()
{
    if (m_ActiveKeyPath)
        delete [] m_ActiveKeyPath;

    if (m_hParent) 
    {
        EnterCriticalSection(&m_pOTG->regCS);
        m_pOTG->bClockStatus=FALSE;
        UpdateDevicePower(m_hParent, D4, NULL);
        LeaveCriticalSection(&m_pOTG->regCS);
        CloseBusAccessHandle(m_hParent);
    }
    
    if (m_pDMA)
        delete m_pDMA;

    if (m_pIdGnd)
        delete m_pIdGnd;

	ReleaseDevicePads(OMAP_DEVICE_HSOTGUSB);
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::Init
//
//  Routine Description:
//
//      This is the initialization for OMAPMHSUSBOTG object
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL OMAPMHSUSBOTG::Init()
{
    int i;    
    BOOL bReturn;
    TCHAR szDefaultClientDriver[MAX_PATH+1]={'R','N','D','I','S'};

    m_pOTG = (PHSMUSB_T)LocalAlloc(LPTR, sizeof(HSMUSB_T));
    if (m_pOTG == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failure to allocate m_pOTG memory\r\n")));
        return FALSE;
    }

    memset(m_pOTG, 0x00, sizeof(HSMUSB_T)); 
    // Other initialization should be done here...    

    m_pOTG->dwPwrMgmt = MODE_SYSTEM_NORMAL;
    for (i = 0; i < 2; i++)
        m_pOTG->hReadyEvents[i] = CreateEvent(0, FALSE, FALSE, NULL);

    if(!MapHardware())
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failure to map hardware\r\n")));
        return FALSE;
    }
    
    InitializeCriticalSection(&m_pOTG->regCS);

    UpdateDevicePower(m_hParent, D0, NULL);
    m_pOTG->bClockStatus = TRUE;
    m_fPowerRequest = FALSE;

	if (!RequestDevicePads(OMAP_DEVICE_HSOTGUSB))
	{
	    DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failure to request pads\r\n")));
        return FALSE;
	}

    m_pTransceiver = CreateHSUSBOTGTransceiver(m_pOTG);
    if (!m_pTransceiver)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failure to initialize OTG Transceiver\r\n")));
        return FALSE;
    }

    // manually reset transceiver (Advisory 3.0.1.144)
    m_pTransceiver->Reset();

    {
        HKEY hkDevice;
        DWORD dwStatus;
        DWORD dwType, dwSize;

        dwStatus = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) REG_USBFN_DEFAULT_FUNC_DRV_PATH, 0, KEY_ALL_ACCESS, &hkDevice);
        if(dwStatus != ERROR_SUCCESS) {
                 DEBUGMSG(ZONE_OTG_ERROR, (_T("UfnPdd_Init: OpenDeviceKey('%s') failed %u\r\n"), REG_USBFN_DRV_PATH, dwStatus));
        }
        else
        {
            dwType = REG_SZ;
            dwSize = sizeof(szDefaultClientDriver);
            dwStatus = ::RegQueryValueEx( hkDevice,
                                        REG_USBFN_DEFAULT_FUNC_DRV,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)&szDefaultClientDriver,
                                        &dwSize);

             if(dwStatus != ERROR_SUCCESS || dwType != REG_SZ) {
                  DEBUGMSG(ZONE_OTG_ERROR, (_T("UFNPDD_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
                          REG_USBFN_DEFAULT_FUNC_DRV_PATH, REG_USBFN_DEFAULT_FUNC_DRV, dwStatus));
             }

             RegCloseKey(hkDevice);
        }
    }

    if(strcmp((const char*)&szDefaultClientDriver,"RNDIS")||strcmp((const char*)&szDefaultClientDriver,"Serial_Class"))
    {
        m_fDelayRequired = TRUE;
    }
    else
    {
        m_fDelayRequired = FALSE;
    }


    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Accessing register with parent (0x%x)\r\n"), m_hParent));

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("FAddr = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->FAddr)));
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Power = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IntrTXE = 0x%x\r\n"), INREG16(&m_pOTG->pUsbGenRegs->IntrTxE)));
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IntrRXE = 0x%x\r\n"), INREG16(&m_pOTG->pUsbGenRegs->IntrRxE)));
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG_Rev = 0x%x\r\n"), INREG32(&m_pOTG->pUsbOtgRegs->OTG_Rev)));
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HW Ver = 0x%x\r\n"), INREG32(&m_pOTG->pUsbGenRegs->HWVers)));

    bReturn = USBOTG::Init();
    
    m_dwUSBUsageCount = 0;  
    
    return bReturn;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::PostInit
//
//  Routine Description:
//
//      This is the post-initialization for OMAPMHSUSBOTG object. And
//      it is called by DefBus.cpp.  It should override the USBOTG::PostInit
//      and notify that is ready to run.
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//

BOOL OMAPMHSUSBOTG::PostInit()
{
    BOOL bReturn = FALSE;
    DEBUGMSG(ZONE_OTG_INIT, (L"OMAPMHSUSBOTG::PostInit(): "
        L"Start.\r\n"
        ));    

    // Default setting to be client mode
    m_UsbOtgState = USBOTG_b_idle;
    m_UsbOtgInput.bit.id = 1;

    if ( GetDeviceHandle()!=NULL &&  IsKeyOpened())
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("PostInit\r\n")));
        bReturn=USBOTG::PostInit();
    }

    UpdateDevicePower(m_hParent, D4, NULL);
    ThreadStart();
    
    return bReturn;
}

//-------------------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::MapHardware
//
//  Routine Description:
//  
//      This is to perform the MmMapIOSpace for the USB registers.
//      It should be called during Init() state.
//
//
//  Return:
//
//      TRUE - success
//      FALSE - fail
//

BOOL OMAPMHSUSBOTG::MapHardware() 
{    
    PHYSICAL_ADDRESS pa;
    PCSP_MUSB_GEN_REGS pGen;
    PCSP_MUSB_OTG_REGS pOtg;
    PCSP_MUSB_DMA_REGS pDma;
    PCSP_MUSB_CSR_REGS pCsr;
    volatile DWORD *pSysControlRegs;
    DDKISRINFO dii;        
    TCHAR *ptcChargerEvent;
    DWORD dwDMAIrq;
    
    if (GetIsrInfo(&dii) != ERROR_SUCCESS) 
    {
        return FALSE;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IRQ = 0x%x\r\n"), dii.dwIrq));

    pa.QuadPart = OMAP35XX_MUSB_BASE_REG_PA + OMAP35XX_MUSB_GEN_OFFSET;
    pGen = (PCSP_MUSB_GEN_REGS)MmMapIoSpace(pa, sizeof(CSP_MUSB_GEN_REGS), FALSE);

    if (pGen == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to MmMap USB General Registers\r\n")));
        return FALSE;
    }   
    m_pOTG->pUsbGenRegs = pGen;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Sizeof(CSP_MUSB_GEN_REGS)=0x%x, mapped (0x%x)\r\n"), sizeof(CSP_MUSB_GEN_REGS), pGen)); 
 
    pa.QuadPart = OMAP35XX_MUSB_BASE_REG_PA + OMAP35XX_MUSB_CSR_OFFSET;
    pCsr = (PCSP_MUSB_CSR_REGS)MmMapIoSpace(pa, sizeof(CSP_MUSB_CSR_REGS), FALSE);

    if (pCsr == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to MmMap USB CSR Registers\r\n")));
        return FALSE;
    }   
    m_pOTG->pUsbCsrRegs = pCsr;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Sizeof(CSP_MUSB_CSR_REGS)=0x%x, mapped (0x%x)\r\n"), sizeof(CSP_MUSB_CSR_REGS), pCsr)); 


    pa.QuadPart = OMAP35XX_MUSB_BASE_REG_PA + OMAP35XX_MUSB_OTG_OFFSET;    
    pOtg = (PCSP_MUSB_OTG_REGS)MmMapIoSpace(pa, sizeof(CSP_MUSB_OTG_REGS), FALSE);
    if (pOtg == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to MmMap USB OTG Registers\r\n")));
        return FALSE;
    }   
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Sizeof(CSP_MUSB_OTG_REGS)=0x%x, mapped (0x%x)\r\n"), sizeof(CSP_MUSB_OTG_REGS), pOtg)); 
    m_pOTG->pUsbOtgRegs = pOtg;

    pa.QuadPart = OMAP35XX_MUSB_BASE_REG_PA + OMAP35XX_MUSB_DMA_OFFSET;        
    pDma = (PCSP_MUSB_DMA_REGS)MmMapIoSpace(pa, sizeof(CSP_MUSB_DMA_REGS), FALSE);
    if (pDma == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to MmMap USB OTG Registers\r\n")));
        return FALSE;
    }   
    m_pOTG->pUsbDmaRegs = pDma;

    pa.QuadPart = OMAP_SYSC_GENERAL_REGS_PA; //Verify
    pSysControlRegs = (volatile DWORD *)MmMapIoSpace(pa, 1024, FALSE);

    m_pOTG->pSysControlRegs = pSysControlRegs;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Sizeof(CSP_MUSB_DMA_REGS)=0x%x, mapped (0x%x)\r\n"), sizeof(CSP_MUSB_DMA_REGS), pDma)); 

    pa.QuadPart = OMAP_SYSC_PADCONFS_REGS_PA;//Verify
    m_pOTG->pPadControlRegs = (volatile UINT8 *)MmMapIoSpace(pa, 2048, FALSE);

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, 
                &dii.dwIrq, 
                sizeof(dii.dwIrq), 
                &dii.dwSysintr, 
                sizeof(dii.dwSysintr), 
                NULL )) 
    {
        DEBUGMSG(ZONE_OTG_ERROR, (L"ERROR: OTG_Init: "  L"Failed map OTG controller interrupt\r\n" ));
        return FALSE;
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IRQ=0x%x, SYSINTR=0x%x\r\n"), dii.dwIrq, dii.dwSysintr));
    m_dwSysIntr = dii.dwSysintr;
    m_dwIrq = dii.dwIrq;
    
    // Spawn a thread to wait for DMA event
    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("RegOpenKeyEx(%s) \r\n"), m_ActiveKeyPath));

    dwDMAIrq = m_OTGRegCfg.DMAIrq;
    
    m_pDMA = new OMAPMHSUSBDMA(this, dwDMAIrq);
    if (m_pDMA == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Fail to set up DMA and it would be disable\r\n")));            
    }    

    m_pIdGnd = new OMAPMHSUSBIdGnd(this);
    if (m_pIdGnd == NULL)
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("OMAPMHSUSBIdGnd failed to create\r\n")));

    ptcChargerEvent = m_OTGRegCfg.szChargerNameEvt;

    DEBUGMSG(ZONE_OTG_INFO, (TEXT("OTG Charger Event(%s)\r\n"), ptcChargerEvent));
    m_BatteryChargeEvent = CreateEvent(NULL, TRUE, FALSE, ptcChargerEvent);
    if ((GetLastError() == ERROR_SUCCESS) || (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        m_BatteryChargeStatus = 0xff;
        DEBUGMSG(ZONE_OTG_INFO, (TEXT("Battery Charger Event Create\r\n")));
    }
    else
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Battery Charger Event failure\r\n")));
        m_BatteryChargeEvent = NULL;
    }
    return TRUE;
}

//-------------------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::IOControl
//
//  Routine Description:
//  
//      This is to perform the IOControl Request. We have added the following new
//      dwCode on the top of existing USBOTG:
//      IOCTL_USBOTG_REQUEST_SESSION_ENABLE - 
//          This is set the Session Request bit. By setting that and if there is a
//          device attach, it would cause the connection. If nothing attached, it would
//          clear by MUSBMHDRC.
//
//      IOCTL_USBOTG_REQUEST_SESSION_DISABLE - 
//          Clear the Session Request bit.
//
//  Return:
//
//      TRUE - success
//      FALSE - fail
//
BOOL OMAPMHSUSBOTG::IOControl(DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, 
                                 PBYTE pBufOut, DWORD dwLenOut,PDWORD pdwActualOut)
{
    BOOL bReturn = FALSE;
    BOOL rc = FALSE;

    switch (dwCode)
    {
        case IOCTL_POWER_CAPABILITIES:
            if (pBufOut && dwLenOut >= sizeof (POWER_CAPABILITIES) && pdwActualOut)
            {
                __try
                {
                    PPOWER_CAPABILITIES PowerCaps;
                    PowerCaps = (PPOWER_CAPABILITIES)pBufOut;

                    // Only supports D0 (permanently on) and D4(off.
                    memset(PowerCaps, 0, sizeof(*PowerCaps));
                    PowerCaps->DeviceDx = 0x11;   // d0 ~ d4
                    *pdwActualOut = sizeof(*PowerCaps);

                    bReturn = TRUE;
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    RETAILMSG(1, (L"exception in ioctl\r\n"));
                }
            }
            break;
            
        case IOCTL_POWER_SET:
            
            if (pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE))
            {
                __try
                {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pBufOut;

                    m_Dx = ReqDx;
                    m_fPowerRequest = TRUE;

                    DEBUGMSG(1, (TEXT("OTG IOCTL_POWER_SET D%d\r\n"), ReqDx));
                    ResetEvent(m_pOTG->hPowerEvent);
                    SetInterruptEvent(m_dwSysIntr);
                    WaitForSingleObject(m_pOTG->hPowerEvent, INFINITE);

                    *(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;
                    *pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
                    bReturn = TRUE;


                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    RETAILMSG(1, (L"Exception in ioctl\r\n"));
                }
            }
            break;

        case IOCTL_POWER_GET:
            if (pBufOut != NULL && dwLenOut >= sizeof(CEDEVICE_POWER_STATE))
            {
                __try
                {
                    *(PCEDEVICE_POWER_STATE)pBufOut = m_Dx;

                    bReturn = TRUE;

                    DEBUGMSG(ZONE_OTG_INFO, (L"USBOTG: IOCTL_POWER_GET to D%u \r\n", m_Dx));
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    RETAILMSG(1, (L"Exception in ioctl\r\n"));
                }
            }
            break;

        case IOCTL_POWER_QUERY:
            if (pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE))
            {
            // Return a good status on any valid query, since we are
            // always ready to change power states (if asked for state
            // we don't support, we move to next highest, eg D3->D4).
                __try
                {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pBufOut;

                    if (VALID_DX(ReqDx))
                    {
                    // This is a valid Dx state so return a good status.
                        bReturn = TRUE;
                    }

                    DEBUGMSG(ZONE_OTG_INFO, (L"USBOTG: IOCTL_POWER_QUERY %d\r\n", ReqDx));
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    RETAILMSG(1, (L"Exception in ioctl\r\n"));
                }
            }
            break;

        case IOCTL_REGISTER_POWER_RELATIONSHIP:
            bReturn = TRUE;
            break;

        case IOCTL_BUS_USBOTG_HNP_ENABLE:
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HNP Set\r\n")));
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HNP Enable\r\n")));
            if (m_pOTG->operateMode == DEVICE_MODE)
                SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
            else
                CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
            m_bHNPEnable = TRUE;
            bReturn = TRUE;
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_REQUEST_HOST_MODE:
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("HNP Set\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            // if from IOCTL_BUS_USBOTG_HNP_ENABLE, always set it
            if (INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTREQ)
            {
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("HNP Disable\r\n")));
                CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
            }
            else
            {
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("HNP Enable\r\n")));
                SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
            }
            m_bHNPEnable = TRUE;

            bReturn = TRUE;
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_BUS_USBOTG_HNP_DISABLE:    
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("HNP Disable\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);        
            bReturn = TRUE;
            m_bHNPEnable = FALSE;
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_SUSPEND_HOST:
        case IOCTL_BUS_USBOTG_SUSPEND :
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Host Suspend\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            if (INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE)
                SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SUSPENDM/*|POWER_EN_SUSPENDM*/);
            else
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("Device Mode cannot set Suspend Bit\r\n")));
            bReturn = TRUE;
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_RESUME_HOST:
        case IOCTL_BUS_USBOTG_RESUME:
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Host Resume\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            if (INREG8(&m_pOTG->pUsbGenRegs->Power) & POWER_SUSPENDM)
            {
                SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_RESUME);
                Sleep(10);
                CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_RESUME);
            }
            else
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("Device Mode cannot set Resume Bit\r\n")));
            bReturn = TRUE;
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_REQUEST_SESSION_ENABLE:        
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Session Enable\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION) == 0x00)
            {
                SessionRequest(TRUE, TRUE);
                bReturn = TRUE;            
                Sleep(2000); 
                // It is required to sleep 2 sec for SRP as if StopUSBClock again, the DATA3 line would be reconfigured to
                // GPIO IN which would cause the SRP un-success.
            }
            else
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("SessionRequest currently enable, ignore (0x%x)\r\n"), 
                        INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("SessionRequest --- still try to write\r\n")));
                SessionRequest(TRUE, TRUE);
                bReturn = TRUE;
            }                
            LeaveCriticalSection(&m_pOTG->regCS);        
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_REQUEST_SESSION_DISABLE:        
            if (m_pOTG->operateMode != IDLE_MODE)
                m_bSessionDisable = TRUE;

            DEBUGMSG(ZONE_OTG_INFO, (TEXT("IOCTL_USBOTG_REQUEST_SESSION_DISABLE\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION) == DEVCTL_SESSION)
                SessionRequest(FALSE, FALSE);
            LeaveCriticalSection(&m_pOTG->regCS);
            if (m_pOTG->operateMode != IDLE_MODE)
                SetInterruptEvent(m_dwSysIntr);
            bReturn = TRUE;
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_REQUEST_START:
            if (m_bOTGReady)
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IOCTL_USBOTG_REQUEST_START\r\n")));
                rc = StartUSBClock(TRUE);
                EnterCriticalSection(&m_pOTG->regCS);        
                SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);
                SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
                LeaveCriticalSection(&m_pOTG->regCS);
                if (rc == TRUE)
                    StopUSBClock();
            }
            break;

        case IOCTL_USBOTG_REQUEST_STOP:
        {
            rc = StartUSBClock(TRUE);
            UINT8 DevCtl = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("IOCTL_USBOTG_REQUEST_STOP\r\n")));
            EnterCriticalSection(&m_pOTG->regCS);
            m_bExtendOTGSuspend = TRUE;
            if((DevCtl&DEVCTL_B_DEVICE) != DEVCTL_B_DEVICE)
            {
                if ((DevCtl & DEVCTL_SESSION) == DEVCTL_SESSION)
                    SessionRequest(FALSE, FALSE);
            }
            CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);
            LeaveCriticalSection(&m_pOTG->regCS);
            if (rc == TRUE)
                StopUSBClock();
        }
            break;

        case IOCTL_USBOTG_REQUEST_SESSION_QUERY:
        {
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Query:\r\n")));
            rc = StartUSBClock(TRUE);
            EnterCriticalSection(&m_pOTG->regCS);
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("DevCtl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Power  = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
#ifdef DEBUG
            UCHAR devctl = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
			UCHAR vbus = (devctl & DEVCTL_VBUS) >> 3;
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("VBus = 0x%x\r\n"), vbus));
#endif
            LeaveCriticalSection(&m_pOTG->regCS);
            bReturn = TRUE;
            if (rc == TRUE)
                StopUSBClock();
        }
        break;

        case IOCTL_USBOTG_DUMP_ULPI_REG:
            rc = StartUSBClock(TRUE);

            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Power = 0x%x\r\n"), m_pOTG->pUsbGenRegs->Power));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Dump ULPI Reg\r\n")));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("ULPIVBusControl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->ulpi_regs.ULPIVbusControl)));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("ULPICarKitControl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->ulpi_regs.ULPICarKitControl)));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("ULPIIntMask = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->ulpi_regs.ULPIIntMask)));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("ULPIIntSrc = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->ulpi_regs.ULPIIntSrc)));        
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("ULPIRawData = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->ulpi_regs.ULPIRawData)));
            m_pTransceiver->DumpULPIRegs();
            bReturn = TRUE;
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_USBOTG_ULPI_SET_LOW_POWER_MODE:
            rc = StartUSBClock(TRUE);
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Power = 0x%x\r\n"), m_pOTG->pUsbGenRegs->Power));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Set Low Power Mode\r\n")));
            bReturn = m_pTransceiver->SetLowPowerMode();
            if (rc == TRUE)
                StopUSBClock();
            break;

        case IOCTL_CONTEXT_RESTORE:
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("USB OTG context restore\r\n")));
            ContextRestore();
            break;

        default:
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("USBOTG::IOControl (0x%x)\r\n"), dwCode));
            bReturn = USBOTG::IOControl(dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
    }

    return bReturn;
}

//----------------------------------------------------------
//  
//  Function:   OMAPMHSUSBOTG:: LoadUnloadUSBFN
//
//  Routine description:
//
//      This function is to request to load USB Device Mode and it should set the operateMode=1 or 0.
//
//  Argument:
//
//      fLoad : TRUE - Load the device module (set operateMode = 1)
//              FALSE - Unload the device module (set operateMode = 0 or retain operateMode =2)
//
//  Return values:
//
//      TRUE
//
BOOL OMAPMHSUSBOTG::LoadUnloadUSBFN(BOOL fLoad)
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OMAPMHSUSBOTG: LoadUnloadUSBFN\r\n")));
    BOOL bReturn = USBOTG::LoadUnloadUSBFN(fLoad);
    return bReturn;
}

//----------------------------------------------------------
//  
//  Function:   OMAPMHSUSBOTG:: LoadUnloadHCD
//
//  Routine description:
//
//      This funciton is to request to load USB Host Mode and it should set the operateMode= 2 or 0.
//
//  Argument:
//
//      fLoad : TRUE - Load the host module (set operateMode = 2)
//              FALSE - Unload the device module (set operateMode = 0 or retain operateMode = 1)
//
//  Return values:
//
//      TRUE
//
BOOL OMAPMHSUSBOTG::LoadUnloadHCD(BOOL fLoad)
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("LoadUnloadHCD\r\n")));
    BOOL bReturn = USBOTG::LoadUnloadHCD(fLoad);
    return bReturn;
}

//----------------------------------------------------------
//  
//  Function:   OMAPMHSUSBOTG:: OTGUSBFNNotfyActive
//
//  Routine description:
//
//      Since DeviceFolder is NULL, we need to override the USBOTG::OTGUSBFNNotfyActive
//      This is to set the b_usbfn_active = 1
//
//  Argument:
//
//      lpDeviceName - Not used
//      fActive  - TRUE - as b-device established
//                 FALSE - b-device not active
//
//  Return values:
//
//      TRUE
//
BOOL OMAPMHSUSBOTG::OTGUSBFNNotfyActive(LPCTSTR lpDeviceName, BOOL fActive)  
{ 
    // Work around needs this function. OTG can't set loc_conn before USBFN Ready.
    BOOL bReturn = USBOTG:: OTGUSBFNNotfyActive(lpDeviceName, fActive);
    return bReturn;
}

//----------------------------------------------------------
//  
//  Function:   OMAPMHSUSBOTG:: OTGHCDNotfyAccept
//
//  Routine description:
//
//      Since DeviceFolder is NULL, we need to override the USBOTG::OTGHCDNotfyAccept
//      This is to set the b_hcd_active = 1
//
//  Argument:
//
//      lpDeviceName - Not used
//      fActive  - TRUE - as b-host established
//                 FALSE - b-host not active
//
//  Return values:
//
//      TRUE
//
BOOL OMAPMHSUSBOTG::OTGHCDNotfyAccept(LPCTSTR lpDeviceName, BOOL fActive) 
{
    BOOL bReturn = USBOTG:: OTGHCDNotfyAccept(lpDeviceName, fActive);
    return bReturn;
}

//----------------------------------------------------------
//  
//  Function:   OMAPMHSUSBOTG:: UsbOtgConfigure
//
//  Routine description:
//
//      This is to configure the controller (e.g. device, host) based
//      on the current usbOtgMode.
//
//  Argument:
//
//      usbOtgMode :    The mode to be configured to.
//
//  Return values:
//
//      mode configure.
//
USBOTG_MODE OMAPMHSUSBOTG::UsbOtgConfigure(USBOTG_MODE usbOtgMode )
{
    usbOtgMode = USBOTG_Bidirection;
    return usbOtgMode;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::UpdateInput
//
//  Routine Description:
//
//      This is to update the state machine.
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL    OMAPMHSUSBOTG::UpdateInput() 
{
    BOOL rc = FALSE;

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("USBOTG::UpdateInput called\r\n")));
    rc = StartUSBClock(TRUE);
    UCHAR devctl = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
    UCHAR vbus = (devctl & DEVCTL_VBUS) >> 3;

    // Check the a_conn and b_conn
    //m_UsbOtgInput.bit.a_conn = (m_pOTG->operateMode == HOST_MODE)? 1:0;
    //m_UsbOtgInput.bit.b_conn = (m_pOTG->operateMode == DEVICE_MODE)? 1:0;
    // Read the Power register to determine the valid session
    //DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("VBus value = 0x%x\r\n")));
    // Dual Role B-Device 6.8.2    
    m_UsbOtgInput.bit.b_bus_req = ((devctl & DEVCTL_SESSION)?1:0);
    m_UsbOtgInput.bit.id = ((devctl & DEVCTL_B_DEVICE)? 1:0);    
    m_UsbOtgInput.bit.b_sess_vld = (((devctl & DEVCTL_SESSION) && (m_pOTG->operateMode == DEVICE_MODE))? 1:0);

    // Dual Role A-Device 6.8.2
    m_UsbOtgInput.bit.a_vbus_vld = ((vbus>=0x2)?1:0);
    //m_UsbOtgInput.bit.a_bus_req = 

    if (rc == TRUE)
        StopUSBClock();
    
    return TRUE;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::IsSE0
//
//  Routine Description:
//
//      This is to return if it is at SE0 (D+/D- pull down)
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL    OMAPMHSUSBOTG::IsSE0()
{
    BOOL bIsSE0;
    BOOL rc = FALSE;
    
    rc = StartUSBClock(TRUE);
    bIsSE0 = m_pTransceiver->IsSE0();
    if (rc == TRUE)
        StopUSBClock();

    DEBUGMSG(ZONE_OTG_INFO, (TEXT("IsSE0 returning %d\r\n"), bIsSE0));

    return bIsSE0;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::NewStateAction
//
//  Routine Description:
//
//      This is to update the state machine.
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL    OMAPMHSUSBOTG::NewStateAction(USBOTG_STATES usbOtgState , USBOTG_OUTPUT usbOtgOutput ) 
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("NewStateAction\r\n")));
    BOOL bReturn = USBOTG::NewStateAction(usbOtgState,usbOtgOutput);
    
    // Process extra state action if needed.

    return bReturn;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::EventNotification
//
//  Routine Description:
//
//      This is to update the event notification and perform
//      hardware changes according to state machine change.
//
//  Arguments:
//
//      nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL    OMAPMHSUSBOTG::EventNotification()
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("EventNotification called\r\n")));
    BOOL bReturn = USBOTG::EventNotification();

    return bReturn;
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::SessionRequest
//
//  Routine Description:
//
//      This is call when SessionRequest
//
//  Arguments:
//
//      fPulseLocConn    :  Not used
//      fPulseChrgVBus   :  Charge VBus
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
BOOL    OMAPMHSUSBOTG::SessionRequest(BOOL fPulseLocConn, BOOL fPulseChrgVBus)
{   
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Session Request(%d, %d)\r\n"), fPulseLocConn, fPulseChrgVBus));
    if (fPulseLocConn || fPulseChrgVBus)
    {
        // (workaround) wait and set session bit again.  Otherwise, sometimes USB transceiver VBUS doesn't go to 5V.
        SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);          
        Sleep(100);
        SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
    }
    else
    {
        CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
    }

    return TRUE;
}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::GetMode
//
//  Routine Description:
//
//      Check the DevCtl.D2 and the current operation mode.
//
//  Return values:
//
//      DEVICE_MODE, HOST_MODE or IDLE_MODE
//
//
DWORD OMAPMHSUSBOTG::GetMode()
{
    DWORD mode;

    if (INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE)
    {
        if (m_pOTG->operateMode == HOST_MODE)
            mode = HOST_MODE;
        else
            mode = IDLE_MODE;
    }
    else
    {
        if (m_pOTG->operateMode == DEVICE_MODE)
            mode = DEVICE_MODE;
        else
            mode = IDLE_MODE;
    }

    return mode;

}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::PowerDownDisconnect
//
//  Routine Description:
//
//      This is call for second stage of INTRUSB get interrupt
//      Refer to USB Interrupt Routine for details
//
//  Arguments:
//
//      pContext - Pointer to OTG Context
//
//  Return values:
//
//      Status info
//
//
void OMAPMHSUSBOTG::PowerDownDisconnect()
{
    DWORD dwPrevState;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect detected\r\n")));
    m_bSessionDisable = FALSE;
    m_disconnected = TRUE;

    ResetEndPoints();
    // If we have done the disconnect, don't process anymore.
    if ((m_pOTG->connect_status & CONN_DC) == 0)
        m_dwStatus |= STATUS_DISCONN_REQUEST|STATUS_WAIT_HOST_DISCONN_COMPLETE;
    else
    {
        m_dwStatus &= ~STATUS_WAIT_HOST_DISCONN_COMPLETE;
        m_dwStatus |= STATUS_DISCONN_COMPLETE;
    }

    m_pOTG->operateMode = IDLE_MODE;
    dwPrevState = m_pOTG->connect_status;
    m_pOTG->connect_status &= ~CONN_CCS;

    if (dwPrevState & CONN_CCS)
        m_pOTG->connect_status |= CONN_CSC;
    else
        m_pOTG->connect_status &= ~CONN_CSC;

    m_pTransceiver->SetVBusSource(TRUE);

    if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
        m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);

    SoftResetMUSBController();
}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::OTG_ConfigISR_stage2
//
//  Routine Description:
//
//      This is call for second stage of INTRUSB get interrupt
//      Refer to USB Interrupt Routine for details
//
//  Arguments:
//
//      pContext - Pointer to OTG Context
//
//  Return values:
//
//      Status info
//
//
DWORD    OMAPMHSUSBOTG::OTG_ConfigISR_stage2()
{    
    HKEY hkDevice;
    DWORD dwStatus;
    DWORD dwType, dwSize;
    
    // Step 1: Handling of SOF

    // Step 2: Handling of disconnect
    if ((((m_pOTG->intr_usb & INTRUSB_DISCONN) == INTRUSB_DISCONN)) || m_bSessionDisable)
    {
        DWORD dwPrevState;
        DWORD dwCurMode = m_pOTG->operateMode;              
        DEBUGMSG(1, (TEXT("Host disconnect detected\r\n")));        
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect detected\r\n")));        

        if(!(ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
        {
            SoftResetULPILink();
        }
        m_bSessionDisable = FALSE;
        m_disconnected = TRUE;
        if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
        {
            ResetEndPoints();
            // If we have done the disconnect, don't process anymore.
            if ((m_pOTG->connect_status & CONN_DC) == 0)
                m_dwStatus |= STATUS_DISCONN_REQUEST|STATUS_WAIT_HOST_DISCONN_COMPLETE;
            else                         
            {
                m_dwStatus &= ~STATUS_WAIT_HOST_DISCONN_COMPLETE;
                m_dwStatus |= STATUS_DISCONN_COMPLETE;           
            }
            
            //DEBUGMSG(1, (TEXT("operateMode = IDLE (disconnect, ISR_stage2)\r\n")));
            m_pOTG->operateMode = IDLE_MODE;
            dwPrevState = m_pOTG->connect_status;
            m_pOTG->connect_status &= ~CONN_CCS;
            if (dwPrevState & CONN_CCS) 
                m_pOTG->connect_status |= CONN_CSC;
            else
                m_pOTG->connect_status &= ~CONN_CSC;

            if (dwCurMode == DEVICE_MODE)
            {
                m_pTransceiver->SetVBusSource(TRUE);

                m_dwbTypeConnector = 0;

                dwStatus = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) REG_USBFN_DRV_PATH, 0, KEY_ALL_ACCESS, &hkDevice);
                if(dwStatus != ERROR_SUCCESS)
                {
                    DEBUGMSG(ZONE_OTG_INFO, (_T("UfnPdd_Init: OpenDeviceKey('%s') failed %u\r\n"), REG_USBFN_DRV_PATH, dwStatus));
                    goto cleanUp;
                }

                dwType = REG_DWORD;
                dwSize = sizeof(m_dwbTypeConnector);
                dwStatus = ::RegSetValueEx(hkDevice, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, NULL, dwType, 
                    (LPBYTE) &m_dwbTypeConnector, dwSize);
                if(dwStatus != ERROR_SUCCESS || dwType != REG_DWORD)
                {
                    DEBUGMSG(ZONE_OTG_INFO, (_T("UFNPDD_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
                        REG_USBFN_DRV_PATH, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, dwStatus));
                    RegCloseKey(hkDevice);
                    goto cleanUp;
                }
                RegCloseKey(hkDevice);

                if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                    m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);
                SoftResetMUSBController();

            }
            else if (dwCurMode == HOST_MODE)
            {
                if (m_pOTG->pFuncs[HOST_MODE-1] != NULL)
                    m_pOTG->pFuncs[HOST_MODE-1]->Disconnect((void *)m_pOTG);         

                SessionRequest(FALSE, FALSE);
                SoftResetMUSBController();

            } 
        }

        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect with Sess = 0x%x\r\n"),INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));            
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect with Pwr = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect with connect_status=0x%x\r\n"), INREG8(&m_pOTG->connect_status)));

        // disable suspend interrupt as needed
        if (!m_pTransceiver->SupportsTransceiverWakeWithoutClock())
            CLRREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  
    }

    // Step 3: Handling of suspend
    if ((m_pOTG->intr_usb & INTRUSB_SUSPEND) == INTRUSB_SUSPEND)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Suspend detected with DevCtl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Suspend detected with Power = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Suspend detected with Intr_USB = 0x%x\r\n"), m_pOTG->intr_usb));

        if ((m_pOTG->operateMode == DEVICE_MODE) && (m_pOTG->deviceType == A_DEVICE))
        {
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("################ DEBUG THIS, IT IS DEVICE MODE using A_DEVICE\r\n")));
            DWORD dwPrevState;
            RETAILMSG(1, (TEXT("A_DEVICE:HNP switching and need to disconnect device mode\r\n")));
            m_dwStatus |= STATUS_DISCONN_COMPLETE;
            m_dwStatus &= ~STATUS_WAIT_HOST_DISCONN_COMPLETE;
            //DEBUGMSG(1, (TEXT("operateMode = IDLE (suspend device)\r\n")));
            m_pOTG->operateMode = IDLE_MODE;
            m_pOTG->connect_status |= CONN_DC;
            dwPrevState = m_pOTG->connect_status;
            m_pOTG->connect_status &= ~CONN_CCS;
            if (dwPrevState & CONN_CCS) 
                m_pOTG->connect_status |= CONN_CSC;
            else
                m_pOTG->connect_status &= ~CONN_CSC;

            if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);     
            
            ResetEndPoints();
            // clear any disconnect interrupt
            m_pOTG->intr_usb &= ~INTRUSB_DISCONN;           
            m_dwStatus |= STATUS_SESSION_RESTART | STATUS_SUSPEND;
        }
        else if ((m_pOTG->operateMode == DEVICE_MODE) && (m_pOTG->deviceType == B_DEVICE))
        {
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("################ DEBUG THIS, IT IS DEVICE MODE using A_DEVICE\r\n")));
         
            BYTE devctl = 0, vbuslevel;

            devctl = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
            vbuslevel = devctl & DEVCTL_VBUS;
            if(vbuslevel != DEVCTL_VBUSVALID)
            {
               m_dwStatus |= STATUS_SUSPEND;
               return STATUS_SUSPEND;
            }

            if(m_bHNPEnable)
            {
                DWORD dwPrevState;
                RETAILMSG(1, (TEXT("B_DEVICE:HNP switching and need to disconnect device mode DevCtl:%x\r\n"),INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));

                m_dwStatus |= STATUS_DISCONN_COMPLETE;
                m_dwStatus &= ~STATUS_WAIT_HOST_DISCONN_COMPLETE;
                m_pOTG->operateMode = IDLE_MODE;
                m_pOTG->connect_status |= CONN_DC;
                dwPrevState = m_pOTG->connect_status;
                m_pOTG->connect_status &= ~CONN_CCS;
                if (dwPrevState & CONN_CCS)
                    m_pOTG->connect_status |= CONN_CSC;
                else
                    m_pOTG->connect_status &= ~CONN_CSC;

                if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                    m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);

                ResetEndPoints();

                // clear any disconnect interrupt
                m_pOTG->intr_usb &= ~INTRUSB_DISCONN;
                m_dwStatus |= STATUS_SESSION_RESTART | STATUS_SUSPEND;

            }
            else
            {
                // To Do...
                // Stop the USB clocks and trigger on a USB wakeup interrupt.
                // ensure we are not charging the battery via the USB port

                // for now... do not set STATUS_SUSPEND, because this will force a reset
            }
        }
        else if(m_pOTG->operateMode == DEVICE_MODE)
        {
             // ??? unreachable code, what was intended?
             SessionRequest(FALSE, FALSE);
             //m_disconnected = FALSE;
            m_dwStatus |= STATUS_SUSPEND;
        }

        // disable suspend interrupt as needed
        if (!m_pTransceiver->SupportsTransceiverWakeWithoutClock())
            CLRREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  

    }
cleanUp:
    return m_dwStatus;
}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::OTG_ProcessEPx
//
//  Routine Description:
//
//      Processing the endpoint x interrupt
//
//  Arguments:
//
//      endpoint - ep which receive interrupt
//      isRx -  Rx interrupt => TRUE
//              Tx interrupt => FALSE
//
//  Return values:
//
//      status
//
DWORD OMAPMHSUSBOTG::OTG_ProcessEPx(UCHAR endpoint, BOOL isRx)
{    
    DWORD dwStatus = 0;
    DWORD mode;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("OTG_ProcessEPx for ep %d, isRx %d\r\n"), endpoint, isRx));
    // For testing purpose, just dump the value from FIFO
    if (GetMode() == DEVICE_MODE)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("m_pOTG Device Mode = 0x%x\r\n"), m_pOTG->pFuncs[DEVICE_MODE-1]));
        mode = DEVICE_MODE;
    }
    else if (GetMode() == HOST_MODE)
    {        
        mode = HOST_MODE;
    }
    else
    {
        DEBUGMSG(ZONE_OTG_INFO, (TEXT("Ignore EP %d (%s) interrupt\r\n"), endpoint, isRx?(TEXT("IN")):(TEXT("OUT"))));
        return 0;
    }

    if (m_pOTG->pFuncs[mode-1] != NULL)
    {   
        if (isRx)
            dwStatus = m_pOTG->pFuncs[mode-1]->ProcessEPx_RX((void *)m_pOTG, endpoint);
        else
            dwStatus = m_pOTG->pFuncs[mode-1]->ProcessEPx_TX((void *)m_pOTG, endpoint);
    }
    return dwStatus;
}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::OTG_ProcessEP0
//
//  Routine Description:
//
//      Processing the endpoint 0 interrupt
//
//  Arguments:
//
//      pContext - Pointer to OTG context
//
//  Return values:
//
//      status
//
DWORD OMAPMHSUSBOTG::OTG_ProcessEP0()
{    
    DWORD dwStatus = 0;
    DWORD mode;

    DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("OTG_ProcessEP0\r\n")));
    // For testing purpose, just dump the value from FIFO    
    if (GetMode() == DEVICE_MODE)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG::CSR0 = 0x%x (device)\r\n"), INREG16(&m_pOTG->pUsbCsrRegs->ep[0].CSR.CSR0)));        
        mode = DEVICE_MODE;
    }
    else if (GetMode() == HOST_MODE)
    {        
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG::CSR0 = 0x%x (host)\r\n"), INREG16(&m_pOTG->pUsbCsrRegs->ep[0].CSR.CSR0)));      
        mode = HOST_MODE;
    }
    else
    {
        DEBUGMSG(ZONE_OTG_INFO, (TEXT("EP0 interrupt ignore due to incorrect mode\r\n")));
        return 0;
    }

    if (m_pOTG->pFuncs[mode-1] != NULL)
        dwStatus = m_pOTG->pFuncs[mode-1]->ProcessEP0((void *)m_pOTG);

    return dwStatus;
}

//------------------------------------------------------------
//
//  Function:   OMAPMHSUSBOTG::OTG_ConfigISR_stage1
//
//  Routine Description:
//
//      This is call for first stage of INTRUSB get interrupt
//      Refer to USB Interrupt Routine for details
//
//  Arguments:
//
//      pHmHSUSB - Pointer to OTG Context
//
//  Return values:
//
//      status     
//
DWORD    OMAPMHSUSBOTG::OTG_ConfigISR_stage1()
{
    HKEY hkDevice;
    DWORD dwStatus;
    DWORD dwType, dwSize;
    
    // Step 1. Resume Interrupt
    if ((m_pOTG->intr_usb & INTRUSB_RESUME) == INTRUSB_RESUME)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Resume Interrupt received\r\n")));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Power = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("DevCtl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("INTR_USB = 0x%x\r\n"), m_pOTG->intr_usb));
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("INTRRX = 0x%x, INTRTX = 0x%x\r\n"), m_pOTG->intr_rx, m_pOTG->intr_tx));
        // Try to clear it and see if problem solved
        DEBUGMSG(1, (TEXT("Clear the Resume interrupt\r\n")));
        CLRREG8(&m_pOTG->pUsbGenRegs->IntrUSB, INTRUSB_RESUME);

        // renable suspend interrupt as needed
        if (!m_pTransceiver->SupportsTransceiverWakeWithoutClock())
            SETREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  
    }

    // Step 2. Session Request Interrupt on A Device?
    if ((m_pOTG->intr_usb & INTRUSB_SESSREQ) == INTRUSB_SESSREQ)
    {
        DEBUGMSG(ZONE_OTG_INFO, (TEXT("Session Request\r\n")));
        SessionRequest(TRUE, TRUE);
    }

    // Step 3. VBus Interrupt on A Device?
    if ((m_pOTG->intr_usb & INTRUSB_VBUSERR) == INTRUSB_VBUSERR)
    {
        // This handles VBUS error for the case "VBUS error is generated while connecting flash drive"
        RETAILMSG(1, (TEXT("VBus Error - OTG_HandleVBusError to be implemented\r\n")));
                // Suppose to call OTG_HandleVBusError
        m_handleVBUSError = TRUE;
    }
    
    // Step 4: Host Mode connection checking
    if (((m_pOTG->intr_usb & INTRUSB_CONN) == INTRUSB_CONN) && 
        ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE) == DEVCTL_HOSTMODE))
    {
        DWORD dwPrevState;
        RETAILMSG(1, (TEXT("USB Connection for Host Side\r\n")));
        // It may be HNP and perform role switching
        m_disconnected = FALSE;

        if (m_pOTG->operateMode == DEVICE_MODE)
        {
            RETAILMSG(1, (TEXT("HNP switching and need to disconnect device mode\r\n")));
            m_dwStatus |= STATUS_DISCONN_COMPLETE;
            m_dwStatus &= ~STATUS_WAIT_HOST_DISCONN_COMPLETE;
            //DEBUGMSG(1, (TEXT("operateMode = IDLE (HNP device to host)\r\n")));
            m_pOTG->operateMode = IDLE_MODE;
            m_pOTG->connect_status |= CONN_DC;
            dwPrevState = m_pOTG->connect_status;
            m_pOTG->connect_status &= ~CONN_CCS;
            if (dwPrevState & CONN_CCS) 
                m_pOTG->connect_status |= CONN_CSC;
            else
                m_pOTG->connect_status &= ~CONN_CSC;

            m_pTransceiver->SetVBusSource(TRUE);
            //UpdateBatteryCharger(BATTERY_USBHOST_DISCONNECT);
            if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);                  
            ResetEndPoints();
            // clear any disconnect interrupt
            m_pOTG->intr_usb  &= ~INTRUSB_DISCONN;           
            m_dwStatus |= STATUS_HNP_SESSION_INIT;
        }
        else
        {
            //DEBUGMSG(1, (TEXT("operateMode = HOST_MODE\r\n")));
            EnterCriticalSection(&m_pOTG->regCS);
            if(m_pOTG->dwPwrMgmt !=  MODE_SYSTEM_SUSPEND)
            {
                m_pOTG->operateMode = HOST_MODE;
                m_pOTG->deviceType = (INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_B_DEVICE)? B_DEVICE: A_DEVICE;

                DEBUGMSG(ZONE_OTG_HNP, (TEXT("Connect: DeviceType [%s]\r\n"),
                    ((m_pOTG->deviceType & B_DEVICE)?(TEXT("B_DEVICE")):(TEXT("A_DEVICE"))) ));

                m_dwStatus |= STATUS_CONNECT;
                // Set the current connect status
                dwPrevState = m_pOTG->connect_status;
                m_pOTG->connect_status |= CONN_CCS;
                if (dwPrevState & CONN_CCS)
                    m_pOTG->connect_status &= ~CONN_CSC;
                else
                    m_pOTG->connect_status |= CONN_CSC;

                // Send the message to Host to do Host_Connect
                if (m_pOTG->pFuncs[HOST_MODE-1] != NULL)
                    m_pOTG->pFuncs[HOST_MODE-1]->Connect((void *)m_pOTG);
                LeaveCriticalSection(&m_pOTG->regCS);
            }
            else
            {
                LeaveCriticalSection(&m_pOTG->regCS);
            }
        }

        // renable suspend interrupt as needed
        if (!m_pTransceiver->SupportsTransceiverWakeWithoutClock())
            SETREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  
    }
    else if (m_pOTG->intr_usb & INTRUSB_CONN)
    {
        DEBUGMSG(ZONE_OTG_INFO, (TEXT("Invalid Connection Detect with DevCtl = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
    }
    
    // Step 5: Reset IRQ
    if ((m_pOTG->intr_usb & INTRUSB_RESET) == INTRUSB_RESET)
    {
        if (((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE) == 0x00) 
            && ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00))// Device Mode
        {
            DWORD dwPrevState;
            
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("USB Reset detected, it would be connection!!\r\n")));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("INTR_USB = 0x%x\r\n"), m_pOTG->intr_usb));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("DEVCTL = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("INTRRX = 0x%x, INTRTX = 0x%x\r\n"), m_pOTG->intr_rx, m_pOTG->intr_tx));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Power Mode = 0x%x\r\n"), INREG8(&m_pOTG->pUsbGenRegs->Power)));
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("DMA INTR = 0x%x\r\n"), INREG32(&m_pOTG->pUsbDmaRegs->Intr)));

            m_pOTG->deviceType = ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_B_DEVICE)? B_DEVICE: A_DEVICE);
            DEBUGMSG(ZONE_OTG_INFO, (TEXT("Reset: DeviceType [%s]\r\n"), 
                ((m_pOTG->deviceType & B_DEVICE)?(TEXT("B_DEVICE")):(TEXT("A_DEVICE"))) ));

            if (m_pOTG->operateMode != HOST_MODE)
            {       
                //DEBUGMSG(1, (TEXT("operateMode = DEVICE_MODE\r\n")));
                m_pOTG->operateMode = DEVICE_MODE;            
                m_dwStatus |= STATUS_CONNECT;
                m_dwStatus &= ~(STATUS_DISCONN_REQUEST|STATUS_WAIT_HOST_DISCONN_COMPLETE|STATUS_DISCONN_COMPLETE);
                dwPrevState = m_pOTG->connect_status;
                m_pOTG->connect_status |= CONN_CCS;
                if (dwPrevState & CONN_CCS) 
                    m_pOTG->connect_status &= ~CONN_CSC;
                else
                    m_pOTG->connect_status |= CONN_CSC;

                if (m_pOTG->deviceType & B_DEVICE && (INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION))
                {
                    m_pTransceiver->SetVBusSource(TRUE);
                    m_dwbTypeConnector = 1;
                }
                else
                {
                    m_dwbTypeConnector = 0;
                }

                dwStatus = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) REG_USBFN_DRV_PATH, 0, KEY_ALL_ACCESS, &hkDevice);
                if(dwStatus != ERROR_SUCCESS)
                {
                    DEBUGMSG(ZONE_OTG_WARN, (_T("UfnPdd_Init: OpenDeviceKey('%s') failed %u\r\n"), REG_USBFN_DRV_PATH, dwStatus));
                    goto cleanUp;
                }

                dwType = REG_DWORD;
                dwSize = sizeof(m_dwbTypeConnector);
                dwStatus = ::RegSetValueEx(hkDevice, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, NULL, dwType, 
                    (LPBYTE) &m_dwbTypeConnector, dwSize);
                if(dwStatus != ERROR_SUCCESS || dwType != REG_DWORD)
                {
                    DEBUGMSG(ZONE_OTG_WARN, (_T("UFNPDD_Init: RegQueryValueEx('%s', '%s') failed %u\r\n"),
                        REG_USBFN_DRV_PATH, REG_VBUS_CHARGE_B_TYPE_CONNECTOR, dwStatus));
                    RegCloseKey(hkDevice);
                    goto cleanUp;
                }
                RegCloseKey(hkDevice);

                m_disconnected = FALSE;
                if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                    m_pOTG->pFuncs[DEVICE_MODE-1]->ResetIRQ((void *)m_pOTG);
                /* Clear bus suspend if there are any, because after bus reset, 
                immediately bus suspend is received during host->device HNP */
                if(m_pOTG->intr_usb & INTRUSB_SUSPEND)
                   m_pOTG->intr_usb &= ~INTRUSB_SUSPEND;
            }
            else
            {                
                DEBUGMSG(ZONE_OTG_HNP, (TEXT("Get HNP role switch to device\r\n")));                    
                ResetEndPoints();
                //DEBUGMSG(1, (TEXT("operateMode = IDLE (HNP host to device)\r\n")));
                m_pOTG->operateMode = IDLE_MODE;
                m_dwStatus |= (STATUS_DISCONN_REQUEST|STATUS_WAIT_HOST_DISCONN_COMPLETE|STATUS_HNP_SESSION_INIT);
                dwPrevState = m_pOTG->connect_status;
                m_pOTG->connect_status &= ~CONN_CCS;
                if (dwPrevState & CONN_CCS) 
                    m_pOTG->connect_status |= CONN_CSC;
                else
                    m_pOTG->connect_status &= ~CONN_CSC;

                if (m_pOTG->pFuncs[HOST_MODE-1] != NULL)
                    m_pOTG->pFuncs[HOST_MODE-1]->Disconnect((void *)m_pOTG);                                
                m_pOTG->intr_usb &= ~INTRUSB_DISCONN;

                /* Clear bus suspend if there are any, because after bus reset, 
                immediately bus suspend is received during host->device HNP */
                if(m_pOTG->intr_usb & INTRUSB_SUSPEND)
                   m_pOTG->intr_usb &= ~INTRUSB_SUSPEND;

            }
        }
        else
        {
            if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("USB Babble:DevCtl;0x%x\r\n"),INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
            else
                DEBUGMSG(ZONE_OTG_INFO, (TEXT("Wait for disconnect complete\r\n")));
        }

        // renable suspend interrupt as needed
        if (!m_pTransceiver->SupportsTransceiverWakeWithoutClock())
            SETREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  
    }
cleanUp:
    return m_dwStatus;
}

//-------------------------------------------------------------------------------
//
//  Function:: CreateFunctionDeviceFolder
//  
//  Routine Description:
//
//      Create Function Device Folder object
//
//  
DeviceFolder * 
OMAPMHSUSBOTG::CreateFunctionDeviceFolder(
    LPCTSTR lpBusName,
    LPCTSTR lpTemplateRegPath,
    DWORD dwBusType, 
    DWORD BusNumber,
    DWORD DeviceNumber,
    DWORD FunctionNumber,
    HANDLE hParent,
    DWORD dwMaxInitReg,
    LPCTSTR lpDeviceBusName 
    ) 
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("CreateFunctionDeviceFolder\r\n")));
    return new OMAPMHSUsbClientDeviceFolder(OMAP_USBHS_DEV_REGS_PA,
        lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber,FunctionNumber, hParent, dwMaxInitReg, lpDeviceBusName);
}

//-------------------------------------------------------------------------------
//
//  Function:: CreateHostDeviceFolder
//  
//  Routine Description:
//
//      Create Host Device Folder object
//
//  

DeviceFolder * 
OMAPMHSUSBOTG::CreateHostDeviceFolder(
    LPCTSTR lpBusName,
    LPCTSTR lpTemplateRegPath,
    DWORD dwBusType, 
    DWORD BusNumber,
    DWORD DeviceNumber,
    DWORD FunctionNumber,
    HANDLE hParent,
    DWORD dwMaxInitReg,
    LPCTSTR lpDeviceBusName
    ) 
{
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("CreateHostDeviceFolder\r\n")));
    return new OMAPMHSUsbClientDeviceFolder(OMAP_USBHS_HOST_REGS_PA,
        lpBusName, lpTemplateRegPath, dwBusType, BusNumber, DeviceNumber,FunctionNumber, hParent, dwMaxInitReg, lpDeviceBusName);
}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::EnableSuspend
// 
//  Routine Description:
//
//      Enable Suspend mode or not
//
//  Parameter:
//      TRUE : Enable
//      FALSE: Disable
//
BOOL OMAPMHSUSBOTG::EnableSuspend(BOOL fEnable)
{
    if (fEnable)
    {
        if (!m_bEnableSuspend)
        { 
		  //PCSP_MUSB_GEN_REGS pGen = m_pOTG->pUsbGenRegs;
          //Keep the transceiver on during system suspend and when a USB device is attached
          //SETREG8(&pGen->Power, POWER_EN_SUSPENDM);       
        }
        m_bEnableSuspend = TRUE;
    }
    else
    {
        if (m_bEnableSuspend)
        {
          CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_EN_SUSPENDM);
        }
        m_bEnableSuspend = FALSE;
    }
    return TRUE;
}



//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::StopUSBClock
//
//  Routine Description:
//
//      Stop the USB Interface Clock
//
BOOL OMAPMHSUSBOTG::StopUSBClock()
{
    PCSP_MUSB_OTG_REGS pOtg;    

    //DWORD *pSysControlRegs;
    // Configure the rest of the stuff:    
    EnterCriticalSection(&m_csUSBClock);        

    if (m_bUSBClockEnable == FALSE)
    {
        LeaveCriticalSection(&m_csUSBClock);
        return FALSE;
    }

        (m_dwUSBUsageCount == 0)? m_dwUSBUsageCount:(m_dwUSBUsageCount--);
        if (m_dwUSBUsageCount > 0)
        {            
            LeaveCriticalSection(&m_csUSBClock);
            return FALSE;
        }
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+StopUSBClock\r\n")));
    EnableSuspend(TRUE);

        pOtg = m_pOTG->pUsbOtgRegs;        
 
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+StopUSBClock\r\n")));

    // According to TI Spec TRM Version H 25.12.3.1
    // Master Interface Management - force standy mode
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_MASK);
    // Side Interface Management - force standby mode
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_MASK);
    // Enable Auto Idle
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);
    // Enable Wakeup
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_ENABLEWAKEUP);
    // Enable Force Standby
    SETREG32(&pOtg->OTG_FORCESTDBY, OTG_FORCESTDY_ENABLEFORCE);

    DEBUGMSG(1, (TEXT("OTG SysConfig = 0x%x\r\n"), INREG32(&pOtg->OTG_SYSCONFIG)));

    if(m_pOTG->operateMode == IDLE_MODE)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("***###StopUSBClock:  EnableWakeupInterrupt(TRUE)\r\n")));
        m_pTransceiver->EnableWakeupInterrupt(TRUE);
    }

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("-StopUSBClock\r\n")));
    m_bUSBClockEnable = FALSE;
    LeaveCriticalSection(&m_csUSBClock);
    return TRUE;

}

//------------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::StartUSBClock
//
//  Routine Description:
//
//      Start the USB Interface Clock
//
BOOL OMAPMHSUSBOTG::StartUSBClock(BOOL fInc)
{
    PCSP_MUSB_OTG_REGS pOtg;

    // Configure the rest of the stuff:
    EnterCriticalSection(&m_csUSBClock);

    if (m_bUSBClockEnable == TRUE)
    {
        LeaveCriticalSection(&m_csUSBClock);
        return FALSE;
    }

    if(m_pOTG->operateMode == IDLE_MODE)
    {
        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("***###StartUSBClock:  EnableWakeupInterrupt(FALSE)\r\n")));
        m_pTransceiver->EnableWakeupInterrupt(FALSE);
    }
    pOtg = m_pOTG->pUsbOtgRegs;


    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+StartUSBClock\r\n")));

    EnterCriticalSection(&m_pOTG->regCS);
    UpdateDevicePower(m_hParent, D0, NULL);
    LeaveCriticalSection(&m_pOTG->regCS);

    EnableSuspend(FALSE);

    // TI spec TRM V.H 25.12.3.2 & 25.12.3.3
    // Clear the EnableForce bit
    CLRREG32(&pOtg->OTG_FORCESTDBY, OTG_FORCESTDY_ENABLEFORCE);

#if 1
    // Enable Wakeup
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_ENABLEWAKEUP);

    // Set the MIDLEMODE to SmartStandy
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_MASK);
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_SMARTSTDBY);

    // Set the SIDLEMODE to SmartIdle
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_MASK);
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_SMARTIDLE);

    // Clear the AutoIdle mode
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Set back the AutoIdle mode to 1
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);
#else
    // Set the MIDLEMODE to NoStandy.  For smartfon, if use SmartStandy, system doesn't wake up from retention.
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_MASK);
    //SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_SMARTSTDBY);

    // Set the SIDLEMODE to SmartIdle
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_MASK);
    //SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_SMARTIDLE);

    // Clear the AutoIdle mode
    //CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Set back the AutoIdle mode to 1
    //SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);
#endif
	
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("-StartUSBClock\r\n")));

    if (fInc)
        m_dwUSBUsageCount++;

    m_bUSBClockEnable = TRUE;
    m_pOTG->bClockStatus = TRUE;
    LeaveCriticalSection(&m_csUSBClock);

    return TRUE;

}

OMAPMHSUSBOTG::SetPowerState(CEDEVICE_POWER_STATE reqDx)
{
    PCSP_MUSB_GEN_REGS pGen = m_pOTG->pUsbGenRegs;

    switch (reqDx)
    {
        case D0:
        case D1:
        case D2:
        {
            BOOL a_device_present;
            BOOL b_device_present;

            if (m_pTransceiver->SupportsTransceiverWakeWithoutClock())
			{
                // *** Transceiver in TWL4030/TPS659xx ***

                a_device_present = m_pTransceiver->IsADeviceConnected();
                b_device_present = m_pTransceiver->IsBDeviceConnected();

                m_bIncCount = FALSE;
                m_pOTG->dwPwrMgmt = MODE_SYSTEM_RESUME;

                StartUSBClock(TRUE);

                if(m_disconnected)
                {
                    if(a_device_present)
                    {
                        OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);
                        OUTREG16(&pGen->IntrTxE, 0xffff);
                        OUTREG16(&pGen->IntrRxE, 0xfffe);

                        if (m_OTGRegCfg.DisableHighSpeed)
						{
                            CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_HSENABLE);
                            SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);
						}
                        else
                            SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_HSENABLE|POWER_SOFTCONN);
                    }
                    else if(b_device_present)
                    {
                        OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);
                        OUTREG16(&pGen->IntrTxE, 0xffff);
                        OUTREG16(&pGen->IntrRxE, 0xfffe);

                        m_dwStatus = STATUS_SESSION_RESTART;
                        CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SUSPENDM);
                        m_pTransceiver->SetVBusSource(TRUE);
                    }
                    else
                    {
                        StopUSBClock();

                        m_pOTG->bClockStatus = FALSE;
                        UpdateDevicePower(m_hParent, D4, NULL);
                        m_pTransceiver->EnableWakeupInterrupt(TRUE);
                    }
                }
                else
                {
                    StopUSBClock();

                    CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SUSPENDM);
                    m_pTransceiver->SetVBusSource(TRUE);

                    m_pOTG->bClockStatus = FALSE;
                    UpdateDevicePower(m_hParent, D4, NULL);

                    m_pTransceiver->EnableWakeupInterrupt(TRUE);
                }
			}
			else
			{
                // *** Transceiver is pure ULPI interface ***

				// clock control moved to PowerUp

				// resume transceiver configuration (may restore external VBUS control)
				m_pTransceiver->Resume();

                OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);
                OUTREG16(&pGen->IntrTxE, 0xffff);
                OUTREG16(&pGen->IntrRxE, 0xfffe);

                m_pOTG->dwPwrMgmt = MODE_SYSTEM_RESUME;
				
                SetInterruptEvent(m_dwSysIntr);
			}
        }
        break;

        case D3:
        case D4:
        {
            pGen = m_pOTG->pUsbGenRegs;
            if (!m_disconnected)
            {
                m_pOTG->dwPwrMgmt = MODE_SYSTEM_SUSPEND;
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG Power Down\r\n")));

                if(m_pOTG->bClockStatus != TRUE)
                    goto cleanUp;

                if (m_pOTG->pUsbGenRegs->DevCtl & DEVCTL_HOSTMODE)
                {
                    DEBUGMSG(ZONE_OTG_INFO, (TEXT("OTG Power Down SetDevicePowerState D3\r\n")));

				    CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
					
//					Sleep(100);

                    SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_EN_SUSPENDM);
                    SETREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SUSPENDM);

					Sleep(100);

					// clock control moved to PowerDown
                }
                else
                {
                    DEBUGMSG(ZONE_OTG_INFO, (TEXT("OTG Power Down SetDevicePowerState D4\r\n")));

                    OUTREG8(&pGen->IntrUSBE, 0);
                    OUTREG16(&pGen->IntrTxE, 0);
                    OUTREG16(&pGen->IntrRxE, 0);

                    // Simulate disconnect.
                    CLRREG8(&m_pOTG->pUsbGenRegs->Power, POWER_SOFTCONN);

                    // Read Interrupt status registers to clear outstanding interrupts
                    INREG8(&pGen->IntrUSB);
                    INREG16(&pGen->IntrTx);
                    INREG16(&pGen->IntrRx);

                    SessionRequest(FALSE, FALSE);

                    PowerDownDisconnect();

					Sleep(100);

					// clock control moved to PowerDown
                }
            }
            else
            {
                if(m_pOTG->bClockStatus == TRUE)
                {
	                m_pOTG->dwPwrMgmt = MODE_SYSTEM_SUSPEND;
					// clock control moved to PowerDown
                }

                m_pOTG->dwPwrMgmt = MODE_SYSTEM_SUSPEND;
            }
            m_pTransceiver->EnableWakeupInterrupt(TRUE);
        }
        break;
    }
cleanUp:
    return TRUE;
}


//-----------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::PowerUp
//
//  Routine Description:
//
//      System Power Up the device when resume
//
//  Arguments:
//
//      Nothing
//
//  Return:
//
//      TRUE - success, FALSE - failure
BOOL OMAPMHSUSBOTG::PowerUp()
{
	m_bIncCount = FALSE;
				
    StartUSBClock(TRUE);
    UpdateDevicePower(m_hParent, D0, NULL);
    return TRUE;
}


//-----------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::PowerDown
//
//  Routine Description:
//
//      System Power Down the device when suspend
//
//  Arguments:
//
//      Nothing
//
//  Return:
//
//      TRUE - success, FALSE - failure
BOOL OMAPMHSUSBOTG::PowerDown()
{
    m_dwUSBUsageCount = 0;
    StopUSBClock();

    m_pOTG->bClockStatus = FALSE;
    UpdateDevicePower(m_hParent, D4, NULL);
     return TRUE;
}

//------------------------------------------------------------
//  Function: OMAPMHSUSBOTG::DelayByRegisterRead
//
//
void OMAPMHSUSBOTG::DelayByRegisterRead()
{
    PCSP_MUSB_OTG_REGS pOtg;
    DWORD i;

    pOtg = m_pOTG->pUsbOtgRegs;

    for(i=0;i<12;i++)
    INREG32(&pOtg->OTG_Rev);
}
//------------------------------------------------------------
//  Function: OMAPMHSUSBOTG::ContextRestore
//
//
BOOL OMAPMHSUSBOTG::ContextRestore(void)
{
    // We do nothing here. Everything necessary is moved to SetPowerState(D2)
    return TRUE;
}
//------------------------------------------------------------
//  Function: OMAPMHSUSBOTG::LinkRecoveryProcedure1
//
//
BOOL OMAPMHSUSBOTG::LinkRecoveryProcedure1(void)
{
    OMAP_SYSC_PADCONFS_REGS   *pConfig=NULL;
    HANDLE hGpio=NULL;
    BOOL ret = FALSE;

    pConfig = (OMAP_SYSC_PADCONFS_REGS*)m_pOTG->pPadControlRegs;
    hGpio = GPIOOpen();

    RETAILMSG(1, (TEXT("Assign ULPI signals to GPIO\r\n")));
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_STP, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));          /*HSUSB0_STP*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA0, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA0*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA1, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA1*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA2, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA2*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA3, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA3*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA4, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA4*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA5, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA5*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA6, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA6*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA7, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA7*/

    // Do the recover stuff here!
    GPIOSetMode(hGpio, 121, GPIO_DIR_OUTPUT); // STP
    GPIOSetMode(hGpio, 125, GPIO_DIR_OUTPUT); // D0
    GPIOSetMode(hGpio, 130, GPIO_DIR_OUTPUT); // D1
    GPIOSetMode(hGpio, 131, GPIO_DIR_OUTPUT); // D2
    GPIOSetMode(hGpio, 169, GPIO_DIR_OUTPUT); // D3
    GPIOSetMode(hGpio, 188, GPIO_DIR_OUTPUT); // D4
    GPIOSetMode(hGpio, 189, GPIO_DIR_OUTPUT); // D5
    GPIOSetMode(hGpio, 190, GPIO_DIR_OUTPUT); // D6
    GPIOSetMode(hGpio, 191, GPIO_DIR_OUTPUT); // D7

    // Attempt recovery procedure 1
    //RETAILMSG(1, (TEXT("Attempt Recovery Procedure 1\r\n")));

    GPIOSetBit(hGpio, 121); // Set   STP
    GPIOSetBit(hGpio, 190); // Set   D6
    GPIOClrBit(hGpio, 191); // Clear D7
    GPIOClrBit(hGpio, 125); // Clear D0
    GPIOClrBit(hGpio, 130); // Clear D1
    GPIOClrBit(hGpio, 131); // Clear D2
    GPIOClrBit(hGpio, 169); // Clear D3
    GPIOClrBit(hGpio, 188); // Clear D4
    GPIOClrBit(hGpio, 189); // Clear D5

    DelayByRegisterRead();

    GPIOClrBit(hGpio, 190); // Clear D6
    DelayByRegisterRead();
    GPIOClrBit(hGpio, 121); // Clear STP

    GPIOClose(hGpio);

    RETAILMSG(1, (TEXT("Restore ULPI Signals\r\n")));
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_STP, (OFF_OUTPUT_PULL_INACTIVE | INPUT_DISABLE | PULL_UP | MUX_MODE_0));          /*HSUSB0_STP*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA0, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA0*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA1, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA1*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA2, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA2*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA3, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA3*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA4, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA4*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA5, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA5*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA6, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA6*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA7, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA7*/

    if((ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
    {
        //RETAILMSG(1, (TEXT("Recovery Procedure 1. ULPI Link Functional\r\n")));
        ret = TRUE;
    }

    return ret;
}

//------------------------------------------------------------
//  Function: OMAPMHSUSBOTG::LinkRecoveryProcedure2
//
//
BOOL OMAPMHSUSBOTG::LinkRecoveryProcedure2(void)
{
    OMAP_SYSC_PADCONFS_REGS   *pConfig=NULL;
    HANDLE hGpio=NULL;
    BOOL ret = FALSE;

    pConfig = (OMAP_SYSC_PADCONFS_REGS*)m_pOTG->pPadControlRegs;
    hGpio = GPIOOpen();

    RETAILMSG(1, (TEXT("Assign ULPI signals to GPIO\r\n")));
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_STP, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));          /*HSUSB0_STP*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA0, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA0*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA1, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA1*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA2, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA2*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA3, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA3*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA4, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA4*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA5, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA5*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA6, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA6*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA7, (INPUT_DISABLE | PULL_INACTIVE | MUX_MODE_4));        /*HSUSB0_DATA7*/

    // Do the recover stuff here!
    GPIOSetMode(hGpio, 121, GPIO_DIR_OUTPUT); // STP
    GPIOSetMode(hGpio, 125, GPIO_DIR_OUTPUT); // D0
    GPIOSetMode(hGpio, 130, GPIO_DIR_OUTPUT); // D1
    GPIOSetMode(hGpio, 131, GPIO_DIR_OUTPUT); // D2
    GPIOSetMode(hGpio, 169, GPIO_DIR_OUTPUT); // D3
    GPIOSetMode(hGpio, 188, GPIO_DIR_OUTPUT); // D4
    GPIOSetMode(hGpio, 189, GPIO_DIR_OUTPUT); // D5
    GPIOSetMode(hGpio, 190, GPIO_DIR_OUTPUT); // D6
    GPIOSetMode(hGpio, 191, GPIO_DIR_OUTPUT); // D7

    // Attempt recovery procedure 1
    //RETAILMSG(1, (TEXT("Attempt Recovery Procedure 2\r\n")));

    GPIOSetBit(hGpio, 190); // Set   D6
    GPIOClrBit(hGpio, 191); // Clear D7
    GPIOClrBit(hGpio, 125); // Clear D0
    GPIOClrBit(hGpio, 130); // Clear D1
    GPIOClrBit(hGpio, 131); // Clear D2
    GPIOClrBit(hGpio, 169); // Clear D3
    GPIOClrBit(hGpio, 188); // Clear D4
    GPIOClrBit(hGpio, 189); // Clear D5

    DelayByRegisterRead();

    GPIOSetBit(hGpio, 121); // Set   STP


    DelayByRegisterRead();

    GPIOClrBit(hGpio, 190); // Clear D6
    DelayByRegisterRead();
    GPIOClrBit(hGpio, 121); // Clear STP

    GPIOClose(hGpio);


    RETAILMSG(1, (TEXT("Restore ULPI Signals\r\n")));
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_STP, (OFF_OUTPUT_PULL_INACTIVE | INPUT_DISABLE | PULL_UP | MUX_MODE_0));          /*HSUSB0_STP*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA0, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA0*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA1, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA1*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA2, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA2*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA3, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA3*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA4, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA4*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA5, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA5*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA6, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA6*/
    OUTREG16(&pConfig->CONTROL_PADCONF_HSUSB0_DATA7, (OFF_INPUT_PULL_DOWN | INPUT_ENABLE | PULL_INACTIVE | MUX_MODE_0));        /*HSUSB0_DATA7*/

    if((ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
    {
        //RETAILMSG(1, (TEXT("Recovery Procedure 2. ULPI Link Functional\r\n")));
        ret = TRUE;
    }

    return ret;
}


BOOL OMAPMHSUSBOTG::SoftResetULPILink(void)
{
    HANDLE hGpio=NULL;
    DWORD j=5;

    hGpio = GPIOOpen();

    do
    {

        // Attempt recovery procedure 1
        RETAILMSG(1, (TEXT("Attempt Recovery Procedure 1\r\n")));
        if(!LinkRecoveryProcedure1())
        {
            // Attempt recovery procedure 2
            RETAILMSG(1, (TEXT("Attempt Recovery Procedure 2\r\n")));
            if(!LinkRecoveryProcedure2())
            {
                // Attempt recovery procedure 3
                RETAILMSG(1, (TEXT("Attempt Recovery Procedure 3! Reset PHY\r\n")));
                m_pTransceiver->ResetPHY();

                // Check whether LINK is functional. Otherwise restart
                // Recovery Procedures.
                if((ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
                {
                    RETAILMSG(1, (TEXT("Recovery Procedure 3. ULPI Link Functional\r\n")));
                    break;
                }
            }
            else
            {
                RETAILMSG(1, (TEXT("Recovery Procedure 2. ULPI Link Functional\r\n")));
                break;
            }
        }
        else
        {
            RETAILMSG(1, (TEXT("Recovery Procedure 1. ULPI Link Functional\r\n")));
            break;
        }


        if (j == 0)
            RETAILMSG(1, (TEXT("ULPI Link recovery attempted. Exit recovery process\r\n")));

    }while(j--);

    GPIOClose(hGpio);
    return TRUE;
}


//------------------------------------------------------------
//  Function: OMAPMHSUSBOTG::ResetEndPoints
//
//
void OMAPMHSUSBOTG::ResetEndPoints(void)
{
    PCSP_MUSB_GEN_REGS pGen;
    PCSP_MUSB_CSR_REGS pCsr;

    pGen = m_pOTG->pUsbGenRegs;
    pCsr = m_pOTG->pUsbCsrRegs;

    DEBUGMSG(ZONE_OTG_INIT || ZONE_OTG_FUNCTION, (TEXT("ResetEndPoints\r\n")));    
    int i = 0;

    for (i = 0; i < 15; i++)
    {
        OUTREG8(&pGen->Index, i);

        if (i == 0)
        {
            SETREG16(&pCsr->ep[0].CSR.CSR0, CSR0_P_FLUSHFIFO);
            while (INREG16(&pCsr->ep[0].CSR.CSR0) & CSR0_P_FLUSHFIFO);
            OUTREG16(&pCsr->ep[0].CSR.CSR0, 0);
        }
        else
        {
            OUTREG16(&pCsr->ep[i].CSR.TxCSR, 0);
            OUTREG16(&pCsr->ep[i].RxCSR, 0);
            OUTREG16(&pCsr->ep[i].TxMaxP, 0);
            OUTREG16(&pCsr->ep[i].RxMaxP, 0);
        }        
    }

    return;
}

//-----------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::UpdateBatteryCharger
//
//  Routine Description:
//
//      This is to update the status of the battery charger and send the event
//      to Battery Charger driver accordingly.
//
//  Parameter:
//
//      dwCurValue - the status of the battery charger.
//------------------------------------------------------------
void    OMAPMHSUSBOTG::UpdateBatteryCharger(DWORD dwCurValue)
{
    if ((m_BatteryChargeStatus == dwCurValue) || (m_BatteryChargeEvent == NULL))
        return;

    m_BatteryChargeStatus = dwCurValue;
    SetEventData(m_BatteryChargeEvent, m_BatteryChargeStatus);
    SetEvent(m_BatteryChargeEvent);
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OMAPMHSUSBOTG::UpdateBatteryCharger to %d\r\n"), m_BatteryChargeStatus));
    return;
}

//-----------------------------------------------------------
//
//  Function: OMAPMHSUSBOTG::SoftResetMUSBController
//
//  Routine Description:
//
//      Softreset the MUSB controller 
//      Initially, this was implmented to recover from USB babble errors.
//      It can be used for other purposes as well.
//
//------------------------------------------------------------
BOOL OMAPMHSUSBOTG::SoftResetMUSBController(BOOL bCalledFromPowerThread)
{
    PCSP_MUSB_OTG_REGS pOtg = m_pOTG->pUsbOtgRegs;      
    PCSP_MUSB_GEN_REGS pGen = m_pOTG->pUsbGenRegs;

	UNREFERENCED_PARAMETER(bCalledFromPowerThread);

    DEBUGMSG(ZONE_OTG_INIT || ZONE_OTG_FUNCTION, (TEXT("SoftResetMUSBController\r\n")));    

    m_timeout = m_OTGRegCfg.startupTimeout;
    m_bRequestSession = FALSE;        
    m_dwStatus = 0;
    m_disconnected = TRUE;
    m_bSuspendTransceiver = FALSE;
    m_handleVBUSError = FALSE;

    // Softreset the MUSB controller
    EnterCriticalSection(&m_pOTG->regCS);
    DWORD dwPriority = CeGetThreadPriority(GetCurrentThread());
    CeSetThreadPriority(GetCurrentThread(), 1);
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_SOFTRESET);
    // wait for reset done
    while ((INREG32(&pOtg->OTG_SYSSTATUS) & OTG_SYSSTATUS_RESETDONE) == 0)
	{
	}

    EnterCriticalSection(&m_csUSBClock);        
	m_dwUSBUsageCount = 0;
    m_bUSBClockEnable = FALSE;
    StartUSBClock(TRUE);
    LeaveCriticalSection(&m_csUSBClock);        

    if (m_OTGRegCfg.DisableHighSpeed)
        CLRREG8(&pGen->Power, POWER_HSENABLE);
    CeSetThreadPriority(GetCurrentThread(), dwPriority);

    ResetEndPoints();
    LeaveCriticalSection(&m_pOTG->regCS);

    // manually reset transceiver (Advisory 3.0.1.144)
    m_pTransceiver->Reset();

    Sleep(100);

    EnterCriticalSection(&m_pOTG->regCS);

    // TI spec TRM V.H 25.12.3.2 & 25.12.3.3

    // Clear the EnableForce bit 
    CLRREG32(&pOtg->OTG_FORCESTDBY, OTG_FORCESTDY_ENABLEFORCE);

    // Set the MIDLEMODE to NoStandy.  For smartfon, if use SmartStandy, system doesn't wake up from retention.
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_MASK);
    //SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_SMARTSTDBY);

    // Set the SIDLEMODE to SmartIdle
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_MASK);
    //SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_SMARTIDLE);

    // Clear the AutoIdle mode
    //CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Set back the AutoIdle mode to 1
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Configure to 12-pin, 8 bit ULPI    
    OUTREG32(&pOtg->OTG_INTERFSEL, OTG_INTERFSEL_12_PIN_ULPI);

    // Enable all interrupts except SOF.
    OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);  
    OUTREG16(&pGen->IntrTxE, 0xffff);
    OUTREG16(&pGen->IntrRxE, 0xfffe);

    OUTREG8(&pGen->Testmode,0);    

    ResetEndPoints();

    if (m_OTGRegCfg.DisableHighSpeed)
	{
        CLRREG8(&pGen->Power, POWER_HSENABLE);
        SETREG8(&pGen->Power, POWER_SOFTCONN);
	}
    else
	{
        SETREG8(&pGen->Power, POWER_SOFTCONN | POWER_HSENABLE);
	}
        
    CLRREG8(&pGen->Power, POWER_SUSPENDM|POWER_EN_SUSPENDM);

    Sleep(10);
    
    LeaveCriticalSection(&m_pOTG->regCS);

    return TRUE;
}

//
//  Function: OMAPMHSUSBOTG::ThreadRun
//
//  Routine Description:
//
//      This is the interrupt thread waiting for signal
//
//  Arguments:
//
//      Nothing
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
DWORD OMAPMHSUSBOTG::ThreadRun()
{
    DWORD   dwEvent;
    DWORD   dwReady = 0x00;
    BOOL    fTerminated = FALSE;
    PCSP_MUSB_GEN_REGS pGen;
    PCSP_MUSB_OTG_REGS pOtg;        
    DWORD dwSysConfigValue;
    int i;
    BOOL  bStratupWaitDone = FALSE;
    DWORD dwStartTime;
    pGen = m_pOTG->pUsbGenRegs;

    CeSetThreadPriority(GetCurrentThread(), m_OTGRegCfg.otgIstPrio);

    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+OMAPMHSUSBOTG::ThreadRun\r\n")));        
    // Wait for both musbhost and musbfn drivers to load
    while ((dwEvent = WaitForMultipleObjects(2, m_pOTG->hReadyEvents, FALSE, INFINITE))!= WAIT_FAILED)
    {
        if (dwEvent == WAIT_OBJECT_0) // Device
            dwReady |= 0x01;
        else if (dwEvent == WAIT_OBJECT_0+1) // Host
            dwReady |= 0x02;

        if (dwReady == 0x03)
            break;
    }

    // wait for shell API
    WaitForAPIReady(SH_SHELL, 45000);
	
    m_bIncCount = FALSE;    
    m_bExtendOTGSuspend = FALSE;
    // Here to update the state to idle state
    m_UsbOtgState = USBOTG_b_idle;
    m_UsbOtgInput.bit.id = 1;
    EventNotification();

    m_disconnected = TRUE;
    m_bSuspendTransceiver = FALSE;
    m_handleVBUSError = FALSE;
    dwStartTime = GetTickCount();

    // Now you have all drivers loading and hence can do the register on the bus...
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OMAPMHSUSBOTG::ThreadRun: Enable Interrupt waiting for connection!!!\r\n")));

    StartUSBClock(TRUE);
    // Configure the rest of the stuff:
    pOtg = m_pOTG->pUsbOtgRegs;

    if (m_OTGRegCfg.DisableHighSpeed)
        CLRREG8(&pGen->Power, POWER_HSENABLE);
    
    // Configuring according to TI spec TRM V.H 25.12.3.2 & 25.12.3.3
    // Clear the EnableForce bit 
    CLRREG32(&pOtg->OTG_FORCESTDBY, OTG_FORCESTDY_ENABLEFORCE);

    // Set the MIDLEMODE to SmartStandy
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_MASK);
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_M_SMARTSTDBY);

    // Set the SIDLEMODE to SmartIdle
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_MASK);
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_S_SMARTIDLE);

    // Clear the AutoIdle mode
    CLRREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Set back the AutoIdle mode to 1
    SETREG32(&pOtg->OTG_SYSCONFIG, OTG_SYSCONF_AUTOIDLE);

    // Configure to 12-pin, 8 bit ULPI    
    OUTREG32(&pOtg->OTG_INTERFSEL, OTG_INTERFSEL_12_PIN_ULPI);

    m_bRequestSession = FALSE;        

    m_dwStatus = 0;
    
    m_pOTG->hPowerEvent = CreateEvent(0, TRUE, FALSE, NULL);
    if (m_pOTG->hPowerEvent == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to create Power Event handle\r\n")));
        return FALSE;
    }

    m_pOTG->hResumeEvent = CreateEvent(0, TRUE, FALSE, NULL);
    if (m_pOTG->hResumeEvent == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to create Resume Event handle\r\n")));
        return FALSE;
    }

    // Now we can set the interrupt routine
    m_hIntrEvent = CreateEvent(0, FALSE, FALSE, NULL);
    if (m_hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to create Interrupt Event handle\r\n")));
        return FALSE;
    }

    // Register the interrupt routine now
    // Initialize interrupt
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("ThreadRun::Initialize Interrupt with sysintr = 0x%x\r\n"), m_dwSysIntr));
    if (!InterruptInitialize(m_dwSysIntr, m_hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_OTG_ERROR, (TEXT("Failed to initialize the interrupt routine\r\n")));
        // We need to do clean up here
        return FALSE;
    }

    m_pOTG->dwSysIntr = m_dwSysIntr;    
    m_pOTG->hSysIntrEvent = m_hIntrEvent;
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("OTG -> Set SYSINTR = 0x%x\r\n"), m_pOTG->dwSysIntr));
    // Ready to run the DMA thread
    m_pIdGnd->Init();
    m_pDMA->Init();

    // Enable the interrupt now
    OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);  // Enable all interrupts except SOF.
    OUTREG16(&pGen->IntrTxE, 0xffff);
    OUTREG16(&pGen->IntrRxE, 0xfffe);

    OUTREG8(&pGen->Testmode,0);    

    // Set connection bit here, ready to process

    // Disable the session request bit for devctl
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("DevCtl = 0x%x\r\n"), INREG8(&pGen->DevCtl)));
    CLRREG8(&pGen->DevCtl, DEVCTL_SESSION);
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("Waiting ...\r\n")));
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("DevCtl = 0x%x\r\n"), INREG8(&pGen->DevCtl)));
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("IntrTxE = 0x%x\r\n"), INREG16(&pGen->IntrTxE)));
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("IntrRxE = 0x%x\r\n"), INREG16(&pGen->IntrRxE)));
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("IntrUSBE = 0x%x\r\n"), INREG8(&pGen->IntrUSBE)));
    DEBUGMSG(ZONE_OTG_INFO, (TEXT("Power = 0x%x\r\n"), INREG8(&pGen->Power)));    

    ResetEndPoints();

    m_timeout = m_OTGRegCfg.startupTimeout;

    if (m_OTGRegCfg.DisableHighSpeed)
    {
        CLRREG8(&pGen->Power, POWER_HSENABLE);
        SETREG8(&pGen->Power, POWER_SOFTCONN);
    }
    else
    {
        SETREG8(&pGen->Power, POWER_SOFTCONN|POWER_HSENABLE);
    }
        
    CLRREG8(&pGen->Power, POWER_EN_SUSPENDM);    
    
    m_pTransceiver->SetVBusSource(TRUE);

    m_bOTGReady = TRUE;

    while (!fTerminated)
    {                        
        DWORD rc;

        if (m_pTransceiver->SupportsTransceiverWakeWithoutClock())
		{
		    DWORD delta;

            // *** Using transceiver in TWL4030/TPS65xxx ***
            rc = WaitForSingleObject(m_hIntrEvent, m_timeout);

            if (m_fPowerRequest)
            {
                m_fPowerRequest = FALSE;
                SetPowerState(m_Dx);
                if (m_Dx == D4)
                {
                    ResetEvent(m_hIntrEvent);
                    m_timeout = INFINITE;
                    SetEvent(m_pOTG->hPowerEvent);
                    continue;
                }
                SetEvent(m_pOTG->hPowerEvent);
            }

            if (rc == WAIT_TIMEOUT)
            {
                m_pTransceiver->EnableWakeupInterrupt(TRUE);
                if(m_timeout == DO_INACTIVITY_TIMEOUT || m_timeout == DO_DISCONNECT_TIMEOUT)
                {
                    if(!(ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
                    {
                        SoftResetULPILink();
                    }
                }
                if(m_bSuspendTransceiver)
                {
                    if (m_pTransceiver->IsADeviceConnected())
                    {
                        // Enable to SOFTCONN bit for PC to see the connection
                        CLRREG8(&pGen->Power, POWER_SOFTCONN);

                        // Wait for 45 sec or until shell is ready which ever happens first
                        do{
                        delta = GetTickCount() - dwStartTime;
                        // Check wheather
                        if(bStratupWaitDone || delta >= m_OTGRegCfg.dwActiveSyncDelay )
                        break;
                        Sleep(1000);
                        }while (IsAPIReady(SH_SHELL) == FALSE);

                        bStratupWaitDone = TRUE;

                        // Enable to SOFTCONN bit for PC to see the connection
                        if (m_OTGRegCfg.DisableHighSpeed)
						{
                            CLRREG8(&pGen->Power, POWER_HSENABLE);
                            SETREG8(&pGen->Power, POWER_SOFTCONN);
						}
                        else
                            SETREG8(&pGen->Power, POWER_SOFTCONN|POWER_HSENABLE);

                        // This is to put MUSB Controller functional if it is in host mode
                        CLRREG8(&pGen->Power, POWER_EN_SUSPENDM);
                        CLRREG8(&pGen->Power, POWER_SUSPENDM);
                        continue;
                    }
                    else if(m_pTransceiver->IsBDeviceConnected())
                    {
                        RETAILMSG(1, (TEXT("Calling Session Request%d\r\n"),__LINE__));
                        SessionRequest(FALSE, FALSE);
                        CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
                    }

                    m_dwUSBUsageCount = 0;

                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+(m_timeout == DO_TRANSCEIVER_SUSPEND_TIMEOUT)\r\n")));

                    StopUSBClock();
                    EnterCriticalSection(&m_pOTG->regCS);
                    m_pOTG->bClockStatus = FALSE;
                    UpdateDevicePower(m_hParent, D4, NULL);
                    LeaveCriticalSection(&m_pOTG->regCS);

                    m_timeout = INFINITE;
                    m_bSuspendTransceiver = FALSE;
                    m_pOTG->dwPwrMgmt = MODE_SYSTEM_SUSPEND;

                    continue;
                }

                m_timeout = INFINITE;

                if (m_pOTG->operateMode == HOST_MODE)
                {
                    //EnableSuspend(TRUE);
                }
                else if (m_pOTG->operateMode == DEVICE_MODE)
                {
                }
                else
                {
                    // If you are here, it can be either IDLE_MODE or SUSPEND_MODE
                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("StopUSBClock with operateMode=%d, m_dwStatus=0x%x\r\n"),
                        m_pOTG->operateMode, m_dwStatus));

                    if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0)
                    {

                        if ((m_dwStatus & STATUS_SESSION_RESTART)
                            && ((m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00)
                            && (m_pOTG->operateMode == IDLE_MODE))
                        {

                            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Try session restart\r\n")));
                            if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                                m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);

                            SessionRequest(FALSE, FALSE);
                            CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
                            SessionRequest(TRUE, TRUE);
                            m_dwStatus &= ~STATUS_SESSION_RESTART;
                            m_timeout = DO_SESSCHK_TIMEOUT;
                        }
                        else if ((m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00)
                        {
                            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+(m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00\r\n")));
                            if(m_disconnected || ((INREG8(&pGen->DevCtl) & DEVCTL_SESSION) == 0))
                            {
                                ResetEndPoints();
                                m_pTransceiver->EnableWakeupInterrupt(TRUE);

                                m_timeout = DO_TRANSCEIVER_SUSPEND_TIMEOUT;
                                m_bIncCount = TRUE;
                                //m_disconnected = FALSE;
                                m_bSuspendTransceiver = TRUE;
                            }
                        }
                    }
                }

                continue;
            }
            else
            {
                if ((m_dwStatus & STATUS_SESSION_RESTART)
                    && ((m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00)
                    && (m_pOTG->operateMode == IDLE_MODE))
                {

                    if (m_pOTG->pFuncs[DEVICE_MODE-1] != NULL)
                        m_pOTG->pFuncs[DEVICE_MODE-1]->Disconnect((void *)m_pOTG);

                    m_dwStatus &= ~STATUS_SESSION_RESTART;
                }

                StartUSBClock(m_bIncCount);
                m_bIncCount = FALSE;
                if (m_dwStatus & STATUS_SESSION_RESTART)
                    m_timeout = DO_SUSPEND_TIMEOUT;
                else
                    m_timeout = INFINITE;

            }

            m_pOTG->intr_usb = INREG8(&m_pOTG->pUsbGenRegs->IntrUSB);
            m_pOTG->intr_tx  |= INREG16(&m_pOTG->pUsbGenRegs->IntrTx);
            m_pOTG->intr_rx  |= INREG16(&m_pOTG->pUsbGenRegs->IntrRx);

            DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("OTG Interrupts(0x%x) intr_tx(0x%x) intr_rx(0x%x)\n"), m_pOTG->intr_usb, m_pOTG->intr_tx, m_pOTG->intr_rx ));

            if (m_pOTG->dwPwrMgmt == MODE_SYSTEM_RESUME)
            {
                if (m_pOTG->operateMode == HOST_MODE)
                {
                    DWORD dwPrevState = 0;

                    DEBUGMSG(1, (TEXT("Resume on Host Mode\r\n")));
                    if (INREG8(&pGen->Power) & POWER_RESUME)
                        CLRREG8(&pGen->Power, POWER_RESUME);

                    if ( ((INREG8(&pGen->DevCtl) & DEVCTL_SESSION) == 0) || ((m_pOTG->intr_usb & INTRUSB_SUSPEND) != 0) )
                    {
                        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Resume Host Mode with request session again\r\n")));
                        ResetEndPoints();
                        m_bRequestSession = TRUE;
                        m_pOTG->operateMode = IDLE_MODE;
                        m_dwStatus |= STATUS_DISCONN_REQUEST;
                        dwPrevState = m_pOTG->connect_status;
                        m_pOTG->connect_status &= ~CONN_CCS;
                        if (dwPrevState & CONN_CCS)
                            m_pOTG->connect_status |= CONN_CSC;
                        else
                            m_pOTG->connect_status &= ~CONN_CSC;

                        if (m_pOTG->pFuncs[HOST_MODE-1] != NULL)
                            m_pOTG->pFuncs[HOST_MODE-1]->Disconnect((void *)m_pOTG);

                        SessionRequest(FALSE, FALSE);

                        m_disconnected = TRUE;
                        m_timeout = DO_SUSPEND_TIMEOUT;
                        Sleep(200);
                        // Softreset the MUSB controller to recover from
                        // babble errors when Activesync is connected
                        SoftResetMUSBController();

		                m_pOTG->dwPwrMgmt = MODE_SYSTEM_NORMAL;
                        continue;
                    }
                    else
                    {
                        SoftResetMUSBController();
                        m_timeout = DO_USBHOST_TIMEOUT;
                    }
                }
                else
                {
                    BYTE  devCtl = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);

                    // This condition occur either on wakeup from suspend (no devices connected before suspend)
                    // or an USB_PRES interrupt detected

                    // Identify if a A or B device is connected and set the session bit only when there is a
                    // device connected.
                    // Setting a session bit without any device connected is causing USB_PRES interrupt

                    // VBUS state changes  immediately from VBUSVALID to "VBUS Above AValid, below VBusValid" on
                    // cable removal so check for VBUSVALID instead of T2 TWL_STS_VBUS
                    SoftResetMUSBController();
                    if (m_pTransceiver->IsBDeviceConnected() ||
                        ((devCtl & DEVCTL_VBUS) == DEVCTL_VBUSVALID))
                        {
                        SessionRequest(TRUE, TRUE);
                        m_timeout = DO_USBCLK_TIMEOUT;
                        }
                    else
                        {
                        // No Device attached
                        DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("NO Device Connected\n")));

                        // wait for interrupt
                        m_timeout = DO_USBCLK_TIMEOUT;
                        m_pOTG->dwPwrMgmt = MODE_SYSTEM_NORMAL;
                        continue;
                        }
                }

                m_pOTG->dwPwrMgmt = MODE_SYSTEM_NORMAL;
            }

            DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("Interrupt Variable INTRUSB(0x%x) Devctl(0x%x)\n"), m_pOTG->intr_usb, INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
            if((!m_pOTG->intr_usb) && (!m_pOTG->intr_tx) && (!m_pOTG->intr_rx))
            {
                if(m_timeout == INFINITE)
                {
                    m_timeout = DO_SUSPEND_TIMEOUT;
                }
                DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("Interrupt Variable INTRUSB No interrupt at all\n")));
            }

            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Interrupt Variable INTRUSB(0x%x), TX(0x%x), RX(0x%x), DevCtl(0x%x), m_dwStatus(0x%x)\r\n"),
                m_pOTG->intr_usb, m_pOTG->intr_tx, m_pOTG->intr_rx, INREG8(&m_pOTG->pUsbGenRegs->DevCtl), m_dwStatus));

            if (m_dwStatus & STATUS_RETENTION_WAKEUP)
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Status Retention with DevCtl Session = 0x%x\r\n"),
                    INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));

                if(!(ReadULPIReg( m_pOTG, ULPI_VENDORID_LOW_R)))
                {
                    SoftResetULPILink();
                }
                if (m_pTransceiver->IsADeviceConnected())
                {
                    // Enable to SOFTCONN bit for PC to see the connection
                    CLRREG8(&pGen->Power, POWER_SOFTCONN);

                    // Wait for 45 sec or until shell is ready which ever happens first
                    do{
                        delta = GetTickCount() - dwStartTime;
                        // Check wheather
                        if(bStratupWaitDone || delta >= m_OTGRegCfg.dwActiveSyncDelay )
                            break;
                        Sleep(1000);
                    }while (IsAPIReady(SH_SHELL) == FALSE);


                    bStratupWaitDone = TRUE;
                    // Enable to SOFTCONN bit for PC to see the connection
                    if (m_OTGRegCfg.DisableHighSpeed)
					{
                        CLRREG8(&pGen->Power, POWER_HSENABLE);
                        SETREG8(&pGen->Power, POWER_SOFTCONN);
					}
                    else
                        SETREG8(&pGen->Power, POWER_SOFTCONN|POWER_HSENABLE);

                    // This is to put MUSB Controller functional if it is in host mode
                    CLRREG8(&pGen->Power, POWER_EN_SUSPENDM);
                    CLRREG8(&pGen->Power, POWER_SUSPENDM);
                }
                else
                {
                    // Enable to SOFTCONN bit for PC to see the connection
                    if (m_OTGRegCfg.DisableHighSpeed)
					{
                        CLRREG8(&pGen->Power, POWER_HSENABLE);
                        SETREG8(&pGen->Power, POWER_SOFTCONN);
					}
                    else
                        SETREG8(&pGen->Power, POWER_SOFTCONN|POWER_HSENABLE);

                    // This is to put MUSB Controller functional if it is in host mode
                    CLRREG8(&pGen->Power, POWER_EN_SUSPENDM);
                    CLRREG8(&pGen->Power, POWER_SUSPENDM);

                    if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION) == 0)
                    {
                        SessionRequest(TRUE, TRUE);
                    }
                }

                m_pOTG->dwPwrMgmt = MODE_SYSTEM_RESUME;

                m_bSuspendTransceiver = FALSE;
                m_dwStatus &= ~STATUS_RETENTION_WAKEUP;
                m_timeout = DO_SESSCHK_TIMEOUT;
            }

            if (m_dwStatus & STATUS_HNP_SESSION_PROCESS)
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("We can process role switch loading now\r\n")));
                if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE) == 0x00)
                    m_pOTG->intr_usb |= INTRUSB_RESET;
                else
                    m_pOTG->intr_usb |= INTRUSB_CONN;
                m_dwStatus &= ~STATUS_HNP_SESSION_PROCESS;
            }

            // Implement according to USB Interrupt Service Routine
            // USBOTGHS Functional Spec Rev 0.1 Figure 8.1 (89/174)

            if (m_pOTG->intr_usb)
            {
                OTG_ConfigISR_stage1();
            }

            if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
            {
                // Here is the handling of Tx/Rx
                // First handling of Endpoint 0 interrupt first.
                if (m_pOTG->intr_tx & INTR_EP(0))
                {
                    OTG_ProcessEP0();
                }

                // It may be a bit odd of why not doing all the things in same loop
                // but this is according to the functional spec. Reference: Inventra MUSBMHDRC
                // Programmer's Guide pg 46
                // Second now handling of Rx 1-15
                i = 1;
                while (i <= 15)
                {
                    if (m_pOTG->intr_rx & INTR_EP(i))
                        OTG_ProcessEPx((UCHAR)i, TRUE);
                    i++;
                }

                // Third now handling of Tx 1-15
                i = 1;
                if (!(m_dwStatus & (STATUS_DISCONN_COMPLETE|STATUS_DISCONN_REQUEST)))
                {
                    while (i <= 15)
                    {
                        if (m_pOTG->intr_tx & INTR_EP(i))
                            OTG_ProcessEPx((UCHAR)i, FALSE);
                        i++;
                    }
                }
            }

            if (m_pOTG->intr_usb)
            {
                OTG_ConfigISR_stage2();
            }

            if(m_handleVBUSError)
            {
                SessionRequest(TRUE, TRUE);
			    m_timeout = m_OTGRegCfg.startupTimeout;
                m_handleVBUSError = FALSE;
            }

            // We can clear the intr_usb, intr_tx & intr_rx
            if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
            {
                m_pOTG->intr_rx = 0;
                m_pOTG->intr_tx = 0;
            }
            m_pOTG->intr_usb = 0;

            if (m_dwStatus & STATUS_CONNECT)
            {
                m_dwStatus &= ~STATUS_CONNECT;
                if (m_pOTG->operateMode == HOST_MODE)
                    m_timeout = DO_USBHOST_TIMEOUT;
                else
                    m_timeout = INFINITE;
            }

            if ((m_dwStatus & STATUS_DISCONN_COMPLETE) && (m_pOTG->connect_status & CONN_DC))
            {
                m_dwStatus &= ~(STATUS_DISCONN_COMPLETE|STATUS_WAIT_HOST_DISCONN_COMPLETE);
                m_pOTG->connect_status &= ~CONN_DC;
                if (m_bRequestSession)
                {
                    m_bRequestSession = FALSE;
                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Request Session Again\r\n")));
                    SessionRequest(TRUE, TRUE);
                }
                else
                {
                    if (m_bExtendOTGSuspend)
                    {
                        m_bExtendOTGSuspend = FALSE;
                        m_timeout = DO_SESSCHK_TIMEOUT;
                    }
                    else
                    {
                        m_timeout = DO_USBCLK_TIMEOUT;
                    }
                    // Perform reset of all the endpoints, we need to do that.
                    if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
                        ResetEndPoints();
                    m_pTransceiver->EnableWakeupInterrupt(TRUE);

                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect Complete and Set IOCTRL again\r\n")));
                }

                if (m_dwStatus & STATUS_HNP_SESSION_INIT)
                {
                    // Check whether it is host mode connection or device mode connection
                    SetEvent(m_hIntrEvent);
                    m_dwStatus |= STATUS_HNP_SESSION_PROCESS;
                    m_dwStatus &= ~STATUS_HNP_SESSION_INIT;
                    DEBUGMSG(ZONE_OTG_HNP, (TEXT("m_bHNPSession set interrupt\r\n")));
                    m_timeout = INFINITE;
                }
            }

            if (m_dwStatus & STATUS_DISCONN_REQUEST)
            {
                m_dwStatus &= ~STATUS_DISCONN_REQUEST;
                m_dwStatus |= STATUS_DISCONN_COMPLETE;
                m_timeout = INFINITE;
            }

            if (m_dwStatus & STATUS_SUSPEND)
            {
                m_dwStatus &= ~STATUS_SUSPEND;
                // Need to set the infinite and wait for the disconnect signal complete
                if (m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE)
                    m_timeout = INFINITE;
                else
                    m_timeout = DO_SUSPEND_TIMEOUT;
            }
            if(m_disconnected)
            {
                m_timeout = DO_DISCONNECT_TIMEOUT;
            }
            else
            {
                m_timeout = DO_INACTIVITY_TIMEOUT;
            }

            InterruptDone(m_dwSysIntr);
        }
		else
        {
            // *** Using transceiver with pure ULPI interface ***

            rc = WaitForSingleObject(m_hIntrEvent, m_timeout);

            if (m_fPowerRequest)
            {
                DEBUGMSG(1, (TEXT("OTG m_fPowerRequest D%d\r\n"), m_Dx));
                m_fPowerRequest = FALSE;
                SetPowerState(m_Dx);
                if (m_Dx == D4)
                {
                    ResetEvent(m_hIntrEvent);
                    m_timeout = INFINITE;
                    SetEvent(m_pOTG->hPowerEvent);
                    continue;
                }
                SetEvent(m_pOTG->hPowerEvent);
            }

            if (m_pOTG->dwPwrMgmt == MODE_SYSTEM_SUSPEND)
			{
                Sleep(100);
                if (rc != WAIT_TIMEOUT)
                	InterruptDone(m_dwSysIntr);
			    continue;
			}

			#if SUSPEND_RESUME_DEBUG_ENABLE
	            if (m_pOTG->dwPwrMgmt == MODE_SYSTEM_RESUME)
				{
	                RETAILMSG(1, (L"OTG ThreadRun MODE_SYSTEM_RESUME\r\n"));
	                BYTE DevCtlValue = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
	                dwSysConfigValue = INREG32(&m_pOTG->pUsbOtgRegs->OTG_SYSCONFIG);
	                BYTE dwPowerValue = INREG8(&m_pOTG->pUsbGenRegs->Power);

	                RETAILMSG(1, (TEXT("OTG: SYSCONFIG(0x%x), POWER(0x%x), DEVCTL(0x%x)\r\n"),
	                    dwSysConfigValue, dwPowerValue, DevCtlValue));
						
	                RETAILMSG(1, (TEXT("OTG: operateMode %s\r\n"), m_pOTG->operateMode == HOST_MODE ? L"host" : m_pOTG->operateMode == DEVICE_MODE ? L"device" : L"idle"));
				}
			#endif
				
            // OTG port ID pin changes will only be detected if session bit in DEVCTL register is set
            if (m_pOTG->dwPwrMgmt == MODE_SYSTEM_NORMAL && !(INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION))
			{
                SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);          

				Sleep(1);
				
				#if SUSPEND_RESUME_DEBUG_ENABLE
		            BYTE DevCtlValue = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
		            dwSysConfigValue = INREG32(&m_pOTG->pUsbOtgRegs->OTG_SYSCONFIG);
		            BYTE dwPowerValue = INREG8(&m_pOTG->pUsbGenRegs->Power);

		            RETAILMSG(1, (TEXT("OTG: SYSCONFIG(0x%x), POWER(0x%x), DEVCTL(0x%x), CCS %d, CSC %d\r\n"),
		                      dwSysConfigValue, dwPowerValue, DevCtlValue, m_pOTG->connect_status & CONN_CCS ? 1 : 0, m_pOTG->connect_status & CONN_CSC ? 1 : 0));
				#endif
							  
				m_timeout = 100;

                if (rc == WAIT_TIMEOUT)
    				continue;
            }

            OUTREG8(&pGen->IntrUSBE, INTRUSB_ALL&~INTRUSB_SOF);

#ifdef DEBUG
            BYTE DevCtlValue = INREG8(&m_pOTG->pUsbGenRegs->DevCtl);
            BYTE dwPowerValue = INREG8(&m_pOTG->pUsbGenRegs->Power);
#endif
            dwSysConfigValue = INREG32(&m_pOTG->pUsbOtgRegs->OTG_SYSCONFIG);

            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG: SYSCONFIG(0x%x), POWER(0x%x), DEVCTL(0x%x)\r\n"),
                    dwSysConfigValue, dwPowerValue, DevCtlValue));

            if (rc == WAIT_TIMEOUT)
            {                        
                m_timeout = INFINITE;

                if (m_pOTG->operateMode == HOST_MODE)
                {   
                    //EnableSuspend(TRUE);
                }
                else
                {
                    // If you are here, it can be either IDLE_MODE or SUSPEND_MODE
                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("StopUSBClock with operateMode=%d, m_dwStatus=0x%x\r\n"), 
                        m_pOTG->operateMode, m_dwStatus));

                    if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0)
                    {

                        if ((m_dwStatus & STATUS_SESSION_RESTART) 
                            && ((m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00) 
                            && (m_pOTG->operateMode == IDLE_MODE))
                        {

                            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Try session restart\r\n")));                        
                            SessionRequest(FALSE, FALSE);
                            CLRREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_HOSTREQ);
                            SessionRequest(TRUE, TRUE);
                            m_dwStatus &= ~STATUS_SESSION_RESTART;
                            m_timeout = DO_SESSCHK_TIMEOUT;
                        }
                        else if ((m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00)
                        {              
                            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("+(m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE) == 0x00\r\n")));
                            if(m_disconnected || ((INREG8(&pGen->DevCtl) & DEVCTL_SESSION) == 0))
                            {
                                // avoid continous stream of suspend interrupts
                                CLRREG8(&m_pOTG->pUsbGenRegs->IntrUSBE, INTRUSB_SUSPEND);  
                                // in effect, we must continuously set the session bit to get host mode to work
                                m_timeout = 1000;
                            }
                        }
                    }
                }

                continue;
            }
            else
            {   
                StartUSBClock(m_bIncCount);         
                m_bIncCount = FALSE;            
                if (m_dwStatus & STATUS_SESSION_RESTART)
                    m_timeout = DO_SUSPEND_TIMEOUT;
                else
                    m_timeout = INFINITE;
            }

            m_pOTG->intr_usb = INREG8(&m_pOTG->pUsbGenRegs->IntrUSB);
            m_pOTG->intr_tx  |= INREG16(&m_pOTG->pUsbGenRegs->IntrTx);
            m_pOTG->intr_rx  |= INREG16(&m_pOTG->pUsbGenRegs->IntrRx);

			#if SUSPEND_RESUME_DEBUG_ENABLE
	            if (m_pOTG->intr_usb & (INTRUSB_VBUSERR | INTRUSB_SESSREQ | INTRUSB_DISCONN  | INTRUSB_CONN | INTRUSB_RESET | INTRUSB_BABBLE | INTRUSB_RESUME | INTRUSB_SUSPEND))
	            {
	                RETAILMSG(1, (TEXT("OTG INTRUSB(0x%x), TX(0x%x), RX(0x%x), DevCtl(0x%x), m_dwStatus(0x%x)\r\n"),
	                    m_pOTG->intr_usb, m_pOTG->intr_tx, m_pOTG->intr_rx, INREG8(&m_pOTG->pUsbGenRegs->DevCtl), m_dwStatus));
	            }
			#endif
				
            //DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("OTG Interrupts(0x%x) intr_tx(0x%x) intr_rx(0x%x)\n"), m_pOTG->intr_usb, m_pOTG->intr_tx, m_pOTG->intr_rx ));
            DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTG Interrupt INTRUSB(0x%x), TX(0x%x), RX(0x%x), DevCtl(0x%x), m_dwStatus(0x%x)\r\n"),
                m_pOTG->intr_usb, m_pOTG->intr_tx, m_pOTG->intr_rx, INREG8(&m_pOTG->pUsbGenRegs->DevCtl), m_dwStatus));

            if (m_pOTG->dwPwrMgmt == MODE_SYSTEM_RESUME)
            {
                DEBUGMSG(1, (TEXT("MODE_SYSTEM_RESUME\r\n")));
                if (m_pOTG->operateMode == HOST_MODE)
                {
                    DWORD dwPrevState = 0;

                    DEBUGMSG(1, (TEXT("Resume on Host Mode\r\n")));
                    if (INREG8(&pGen->Power) & POWER_RESUME)
                        CLRREG8(&pGen->Power, POWER_RESUME);                                

                    if ( ((INREG8(&pGen->DevCtl) & DEVCTL_SESSION) == 0) || ((m_pOTG->intr_usb & INTRUSB_SUSPEND) != 0) )
                    {
                        DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Resume Host Mode with request session again\r\n")));
                        ResetEndPoints();
                        m_bRequestSession = TRUE;
                        //DEBUGMSG(1, (TEXT("operateMode = IDLE (resume host)\r\n")));
                        m_pOTG->operateMode = IDLE_MODE;
                        m_dwStatus |= STATUS_DISCONN_REQUEST/*|STATUS_WAIT_HOST_DISCONN_COMPLETE*/;
                        dwPrevState = m_pOTG->connect_status;
                        m_pOTG->connect_status &= ~CONN_CCS;
                        if (dwPrevState & CONN_CCS) 
                            m_pOTG->connect_status |= CONN_CSC;
                        else
                            m_pOTG->connect_status &= ~CONN_CSC;

                        if (m_pOTG->pFuncs[HOST_MODE-1] != NULL)
                           m_pOTG->pFuncs[HOST_MODE-1]->Disconnect((void *)m_pOTG);       

                        SessionRequest(FALSE, FALSE);

                        m_disconnected = TRUE;
                        m_timeout = DO_SUSPEND_TIMEOUT;
                        Sleep(200);
                        // Softreset the MUSB controller to recover from
                        // babble errors when Activesync is connected
                        SoftResetMUSBController();

                        // ??? enable the session bit in OTG devctl register
                        SessionRequest(TRUE, TRUE);

                        if (rc != WAIT_TIMEOUT)
                            InterruptDone(m_dwSysIntr);

                        continue;
                    }
                    else
                        m_timeout = DO_USBHOST_TIMEOUT;
                }            
                else
                {
                    SessionRequest(TRUE, TRUE);
                    m_timeout = DO_USBCLK_TIMEOUT;
                }

                m_pOTG->dwPwrMgmt = MODE_SYSTEM_NORMAL;
            }
        
            //DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("Interrupt Variable INTRUSB(0x%x) Devctl(0x%x)\n"), m_pOTG->intr_usb, INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));
            if((!m_pOTG->intr_usb) && (!m_pOTG->intr_tx) && (!m_pOTG->intr_rx))
            {
               if(m_timeout == INFINITE)
               {
                  m_timeout = DO_SUSPEND_TIMEOUT;
               }
               DEBUGMSG(ZONE_OTG_FUNCTION,(TEXT("Interrupt Variable INTRUSB No interrupt at all\n")));
            }

            // STATUS_RETENTION_WAKEUP is set when IDGND event is signaled
            if (m_dwStatus & STATUS_RETENTION_WAKEUP)
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Status Retention with DevCtl Session = 0x%x\r\n"), 
                    INREG8(&m_pOTG->pUsbGenRegs->DevCtl)));

                if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_SESSION) == 0)
                {
                    SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);

                    // (workaround) wait and set session bit again.  Otherwise, sometimes USB transceiver VBUS doesn't go to 5V.
                    Sleep(100);
                    SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);
                }
            
                m_dwStatus &= ~STATUS_RETENTION_WAKEUP;
                m_timeout = DO_SESSCHK_TIMEOUT;            
            }

            if (m_dwStatus & STATUS_HNP_SESSION_PROCESS)
            {
                DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("We can process role switch loading now\r\n")));
                if ((INREG8(&m_pOTG->pUsbGenRegs->DevCtl) & DEVCTL_HOSTMODE) == 0x00)
                    m_pOTG->intr_usb |= INTRUSB_RESET;
                else
                    m_pOTG->intr_usb |= INTRUSB_CONN;
                m_dwStatus &= ~STATUS_HNP_SESSION_PROCESS;
            }
     
            // Implement according to USB Interrupt Service Routine
            // USBOTGHS Functional Spec Rev 0.1 Figure 8.1 (89/174)

            if (m_pOTG->intr_usb)
            {            
                OTG_ConfigISR_stage1();         
            }

            if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
            {
                // Here is the handling of Tx/Rx
                // First handling of Endpoint 0 interrupt first.
                if (m_pOTG->intr_tx & INTR_EP(0))
                {
                    OTG_ProcessEP0();
                }

                // It may be a bit odd of why not doing all the things in same loop
                // but this is according to the functional spec. Reference: Inventra MUSBMHDRC 
                // Programmer's Guide pg 46
                // Second now handling of Rx 1-15 
                i = 1;
                while (i <= 15)
                {
                    if (m_pOTG->intr_rx & INTR_EP(i))
                        OTG_ProcessEPx((UCHAR)i, TRUE);
                    i++;
                }

                // Third now handling of Tx 1-15
                i = 1;
                if (!(m_dwStatus & (STATUS_DISCONN_COMPLETE|STATUS_DISCONN_REQUEST)))
                {
                    while (i <= 15)
                    {
                        if (m_pOTG->intr_tx & INTR_EP(i))
                            OTG_ProcessEPx((UCHAR)i, FALSE);
                        i++;
                    }
                }
            }

            if (m_pOTG->intr_usb)
            {            
                OTG_ConfigISR_stage2();         
            }

            if(m_handleVBUSError)
            {
                SessionRequest(FALSE, FALSE);
                m_handleVBUSError = FALSE;
			    m_timeout = m_OTGRegCfg.startupTimeout;
                continue;
            }

            // We can clear the intr_usb, intr_tx & intr_rx
            if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
            {
                m_pOTG->intr_rx = 0;
                m_pOTG->intr_tx = 0;
            }
            m_pOTG->intr_usb = 0;
        
            if (m_dwStatus & STATUS_CONNECT)
            {
                m_dwStatus &= ~STATUS_CONNECT;
                if (m_pOTG->operateMode == HOST_MODE)
                    m_timeout = DO_USBHOST_TIMEOUT;
                else
                    m_timeout = INFINITE;            
            }


            if ((m_dwStatus & STATUS_DISCONN_COMPLETE) && (m_pOTG->connect_status & CONN_DC)) 
            {                        
                m_dwStatus &= ~(STATUS_DISCONN_COMPLETE|STATUS_WAIT_HOST_DISCONN_COMPLETE);                       
                m_pOTG->connect_status &= ~CONN_DC;
                if (m_bRequestSession)
                {
                    m_bRequestSession = FALSE;                                
                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Request Session Again\r\n")));
                    SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);  

                    // (workaround) wait and set session bit again.  Otherwise, sometimes USB transceiver VBUS doesn't go to 5V.
                    Sleep(100);
                    SETREG8(&m_pOTG->pUsbGenRegs->DevCtl, DEVCTL_SESSION);      

                }
                else
                {
                    if (m_bExtendOTGSuspend)
                    {
                        m_bExtendOTGSuspend = FALSE;
                        m_timeout = DO_SESSCHK_TIMEOUT;
                    }
                    else
                        m_timeout = DO_USBCLK_TIMEOUT;
                    // Perform reset of all the endpoints, we need to do that.                                
                    if ((m_dwStatus & STATUS_HNP_SESSION_MASK) == 0x00)
                        ResetEndPoints();
                    m_pTransceiver->EnableWakeupInterrupt(TRUE);
                
                    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("Disconnect Complete and Set IOCTRL again\r\n")));                
                }

                if (m_dwStatus & STATUS_HNP_SESSION_INIT)
                {
                    // Check whether it is host mode connection or device mode connection
                    SetEvent(m_hIntrEvent);
                    m_dwStatus |= STATUS_HNP_SESSION_PROCESS;
                    m_dwStatus &= ~STATUS_HNP_SESSION_INIT;
                    DEBUGMSG(ZONE_OTG_HNP, (TEXT("m_bHNPSession set interrupt\r\n")));
                    m_timeout = INFINITE;
                }
            }

            if (m_dwStatus & STATUS_DISCONN_REQUEST)
            {
                m_dwStatus &= ~STATUS_DISCONN_REQUEST;
                m_dwStatus |= STATUS_DISCONN_COMPLETE;
                m_timeout = INFINITE;
            }


            if (m_dwStatus & STATUS_SUSPEND)
            {
                m_dwStatus &= ~STATUS_SUSPEND;
                // Need to set the infinite and wait for the disconnect signal complete
                if (m_dwStatus & STATUS_WAIT_HOST_DISCONN_COMPLETE)
                    m_timeout = INFINITE;
                else
                {
                    m_timeout = DO_SUSPEND_TIMEOUT;
                    OUTREG8(&pGen->IntrUSBE, 0xF6/*INTRUSB_ALL&~INTRUSB_SOF&~INTRSUSPEND*/);
                }
            }
            if(m_disconnected)
            {
               m_timeout = DO_DISCONNECT_TIMEOUT;
            }
            else
            {
               m_timeout = DO_INACTIVITY_TIMEOUT;
            }

            InterruptDone(m_dwSysIntr);
        }
    }
    
    CloseHandle(m_hIntrEvent);
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("ThreadRun exit\r\n")));
    return 1;
}

UCHAR OMAPMHSUSBOTG::ReadULPIReg(PHSMUSB_T pOTG, UCHAR idx)
{
    DWORD dwCount = 100;
    UCHAR ucData = 0x00;
    pOTG->pUsbGenRegs->ulpi_regs.ULPIRegAddr = idx;
    pOTG->pUsbGenRegs->ulpi_regs.ULPIRegControl = ULPI_REG_REQ | ULPI_RD_N_WR;

    while ((pOTG->pUsbGenRegs->ulpi_regs.ULPIRegControl & ULPI_REG_CMPLT) == 0)
    {
        Sleep(20);
        dwCount--;
        if (dwCount == 0)
        {
            RETAILMSG(1, (TEXT("####### FAIL to read ULPI Reg, Control = 0x%x\r\n"), 
                pOTG->pUsbGenRegs->ulpi_regs.ULPIRegControl));
            return 0x00;
        }
    }
    ucData = pOTG->pUsbGenRegs->ulpi_regs.ULPIRegData;

    pOTG->pUsbGenRegs->ulpi_regs.ULPIRegControl = 0;

    return ucData;
}

//------------------------------------------------------------
//
//  Function: OTGAttach
//
//  Routine Description:
//
//      This is call by host dll and device dll to register the
//      calling procedure between host/device with OTG
//
//  Arguments:
//
//      pFuncs           :  [IN] Functions to be called by OTG to host/device.
//                               refer to OMAPM_musb.h for detail
//      pParam           :  [IN] data to be pasted onto the HSUSB structure
//      mode             :  [IN] host or device to register
//                               DEVICE_MODE or HOST_MODE
//      lppvContext      :  [OUT] pointer to pointer of master OTG handle.
//
//  Return values:
//
//      TRUE - success
//      FALSE - failed
//
extern "C" BOOL OTGAttach(PMUSB_FUNCS pFuncs, int mode, LPLPVOID lppvContext)
{
    BOOL bRet = FALSE;
    PHSMUSB_T pOTG = NULL;
    DEBUGMSG(ZONE_OTG_FUNCTION, (TEXT("OTGAttach mode=%d\r\n"), mode));
    
    // Set the corresponding MUSB_FUNCS.
    if ((mode >= DEVICE_MODE) && ( mode <= HOST_MODE))
    {
        pOTG = gpHsUsbOtg->GetHsMusb();
        pOTG->pFuncs[mode-1] = pFuncs; 
        bRet = TRUE;       
    }

    // Pass the gmHsUsb context back to device or host driver       
    *lppvContext = (LPVOID)pOTG;
    
    return bRet;
}

//------------------------------------------------------------------------------
//
//  Function: OTGUSBClock
//
//  Routine Description:
//      
//      Start or Stop USB Clock
//
//  Parameters:
//      
//      fStart - TRUE : start USB clock
//               FALSE: stop USB clock
extern "C" BOOL OTGUSBClock(BOOL fStart)
{
    BOOL bReturn;

    DEBUGMSG(1, (TEXT("OTGUSBClock(%d) called\r\n"), fStart));
    if (gpHsUsbOtg)
    {
        if (fStart)
            bReturn = gpHsUsbOtg->StartUSBClock(TRUE);
        else
            bReturn = gpHsUsbOtg->StopUSBClock();
    }
    else
    {
        DEBUGMSG(1, (TEXT("OTGUSBClock failed to set\r\n")));
        bReturn = FALSE;
    }
    return bReturn;
}

