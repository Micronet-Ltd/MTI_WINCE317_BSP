// All rights reserved ADENEO EMBEDDED 2010
/*
==============================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
==============================================================
*/
//
//  File:  smartreflexpolicy.cpp
//


//******************************************************************************
//  #INCLUDES
//******************************************************************************
#include "omap.h"
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "omap_dvfs.h"
#include <ceddkex.h>

#include "omap3530_clocks.h"
#include "omap3530_prcm.h"

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
//  Set debug zones names and initial setting for driver
//
#ifndef SHIP_BUILD

#ifndef ZONE_ERROR
#define ZONE_ERROR          DEBUGZONE(0)
#endif

#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"ProxyDriver", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined" ,  L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    0x0000
};

#endif

//------------------------------------------------------------------------------
//  Local Definitions

#define POLICY_CONTEXT_COOKIE       'strx'
#define CONSTRAINT_ID_DVFS          L"DVFS"
#define SENSING_PERIOD              (1000)
#define SENSING_DELAY               (1000)

#define PRM_VP1_VOLTAGE             0x483072c0
#define PRM_VP2_VOLTAGE             0x483072e0

#define STATE_INACTIVE              0
#define STATE_WAITING               1
#define STATE_SENSING               2

//-----------------------------------------------------------------------------
//  Local structures

typedef struct
{
    DWORD                              dwActiveDevices;
    DWORD                              dwSensingPeriod;
    DWORD                              dwSensingDelay;
    BOOL                               bDisplayLPROn;
    BOOL                               bSmartReflexState;
    DWORD                              nSensingState;
    BOOL                               bExitThread;
    HANDLE                             hDvfsConstraint;
    HANDLE                             hDvfsCallback;
    HANDLE                             hSmartReflexSensingEvent;
    HANDLE                             hSmartReflexSensingThread;
    BOOL                               bLprModeState;
    DWORD                              dwLprOffOpm;
} SmartReflexPolicyInfo_t;

typedef struct
{
    void                              *pPrcmRegistryAddress;
    UINT                               optimizedVoltage;
} VddOptimizationInfo_t;

typedef struct
{
    DWORD                           cookie;
} SmartReflexPolicyContext_t;

//------------------------------------------------------------------------------
//  policy registry parameters
//
static const DEVICE_REGISTRY_PARAM g_policyRegParams[] = {
    {
        L"SensingPeriod", PARAM_DWORD, FALSE, offset(SmartReflexPolicyInfo_t, dwSensingPeriod),
        fieldsize(SmartReflexPolicyInfo_t, dwSensingPeriod), (void*)SENSING_PERIOD
    }, {
        L"SensingDelay", PARAM_DWORD, FALSE, offset(SmartReflexPolicyInfo_t, dwSensingDelay),
        fieldsize(SmartReflexPolicyInfo_t, dwSensingDelay), (void*)SENSING_DELAY
    }, {
        L"LprOffOpm", PARAM_DWORD, FALSE, offset(SmartReflexPolicyInfo_t, dwLprOffOpm),
        fieldsize(SmartReflexPolicyInfo_t, dwLprOffOpm), (void*)kOpm2
    }
};

//-----------------------------------------------------------------------------
//  local variables
//
static SmartReflexPolicyInfo_t          s_SmartReflexLoadPolicyInfo;
static IOCTL_RETENTION_VOLTAGES         s_KernIoCtlInfo;
static DWORD                            s_currentOpm;
static VddOptimizationInfo_t            s_VddOptimizationInfo[2];
//-----------------------------------------------------------------------------
//
//  Function:  EnableSmartReflex
//
//  Function that sends SmartReflex KernelIOCTL
//
VOID
EnableSmartReflex(
    BOOL bEnable
    )
{
    KernelIoControl(
        IOCTL_SMARTREFLEX_CONTROL,
        &bEnable,
        sizeof(BOOL),
        NULL,
        0,
        NULL
        );
     RETAILMSG(ZONE_INFO,(L"SmartReflexPolicy: EnableSmartReflex(%d)",bEnable));
}

//-----------------------------------------------------------------------------
//
//  Function:  SendRetentionVoltages
//
//  Function that sends new retention voltages via KernelIOCTL
//
VOID
SendRetentionVoltages()
{
    KernelIoControl(
        IOCTL_UPDATE_RETENTION_VOLTAGES,
        &s_KernIoCtlInfo,
        sizeof(IOCTL_RETENTION_VOLTAGES),
        NULL,
        0,
        NULL
        );
}

//-----------------------------------------------------------------------------
//
//  Function:  GetOptimizedVoltages
//
//  reads SmartReflex optimized values
//
void
SaveCalibratedVoltages()
{
    UINT idx;
    for (idx = 0 ; idx < 2 ; idx++)
        {
        s_VddOptimizationInfo[idx].optimizedVoltage = INREG32(s_VddOptimizationInfo[idx].pPrcmRegistryAddress);
        }
}

//-----------------------------------------------------------------------------
//
//  Function:  SetOptimizedVoltages
//
//  Sends retention optimized values to the kernel
//
void
FlushCalibratedVoltages()
{
    UINT idx;
    for (idx = 0 ; idx < 2 ; idx++)
        {
        s_KernIoCtlInfo.retentionVoltage[idx] = s_VddOptimizationInfo[idx].optimizedVoltage;
        }

    SendRetentionVoltages();
}

//-----------------------------------------------------------------------------
//
//  Function:  UpdateSmartReflex
//
//  Function that decide wether or not to turn on SmartReflex
//
VOID
UpdateSmartReflex()
{
    if ((s_SmartReflexLoadPolicyInfo.dwActiveDevices > 0 ||
         s_SmartReflexLoadPolicyInfo.nSensingState == STATE_SENSING
         ) &&
        s_SmartReflexLoadPolicyInfo.bSmartReflexState == FALSE
           )
        {
        //enable SR
        s_SmartReflexLoadPolicyInfo.bSmartReflexState = TRUE;
        EnableSmartReflex(s_SmartReflexLoadPolicyInfo.bSmartReflexState);
        }

    else if ((s_SmartReflexLoadPolicyInfo.dwActiveDevices == 0 ||
        s_SmartReflexLoadPolicyInfo.nSensingState == STATE_SENSING
        ) &&
        s_SmartReflexLoadPolicyInfo.bSmartReflexState == TRUE
           )
        {
        //diable SR
        s_SmartReflexLoadPolicyInfo.bSmartReflexState = FALSE;
        EnableSmartReflex(s_SmartReflexLoadPolicyInfo.bSmartReflexState);
        }
}





//-----------------------------------------------------------------------------
//
//  Function:  SmartReflexSensingThread
//
//  thread in charge of changing the optimized voltage values
//
DWORD
WINAPI
SmartReflexSensingThread(
    LPVOID pvParam
    )
{
    DWORD code;
    DWORD dwTimeout;

    UNREFERENCED_PARAMETER(pvParam);

    // set thread priority to lowest to prevent possible thread inversion
    CeSetThreadPriority(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingThread, 255);

    // events
    // -----------------
    //
    // abort - when currently not in OPM0 or
    //         a device is active which causes smartreflex to be enabled
    //
    // signal - currently at opm 0 and
    //          no device active to trigger smartreflex
    //
    // timeout - wait time expired
    //

    // states
    //--------
    //
    // Inactive - not waiting to calibrate or
    //            not currently calibrating
    //
    // Waiting  - currently waiting to calibrate
    //
    // Sensing  - currently calibrating
    //

    // current state   |       abort       |       signal       |       timeout     |
    //--------------------------------------------------------------------------------
    //
    // Inactive               Inactive             Waiting              Inactive
    //
    // Waiting                Inactive             Waiting              Sensing
    //
    // Sensing                Inactive             Waiting              Inactive
    //

    dwTimeout = INFINITE;
    for(;;)
        {
        code = WaitForSingleObject(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingEvent, dwTimeout);

        // check for exit thread
        if (s_SmartReflexLoadPolicyInfo.bExitThread == TRUE) goto cleanUp;

        // identify event
        if (s_currentOpm != kOpm0 || s_SmartReflexLoadPolicyInfo.dwActiveDevices != 0)
            {
            // Event: abort
            dwTimeout = INFINITE;
            switch (s_SmartReflexLoadPolicyInfo.nSensingState)
                {
                case STATE_INACTIVE:
                    break;

                case STATE_WAITING:
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_INACTIVE;
                    break;

                case STATE_SENSING:
                    // check if smartreflex should be disabled
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_INACTIVE;
                    UpdateSmartReflex();
                    break;
                }
            }
        else if (code == WAIT_OBJECT_0)
            {
            // Event: signal
            switch (s_SmartReflexLoadPolicyInfo.nSensingState)
                {
                case STATE_INACTIVE:
                    dwTimeout = s_SmartReflexLoadPolicyInfo.dwSensingDelay;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_WAITING;
                    break;

                case STATE_WAITING:
                    dwTimeout = s_SmartReflexLoadPolicyInfo.dwSensingDelay;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_WAITING;
                    break;

                case STATE_SENSING:
                    dwTimeout = s_SmartReflexLoadPolicyInfo.dwSensingDelay;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_WAITING;
                    UpdateSmartReflex();
                    break;
                }
            }
        else
            {
            // Event: timeout
            switch (s_SmartReflexLoadPolicyInfo.nSensingState)
                {
                case STATE_INACTIVE:
                    dwTimeout = INFINITE;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_INACTIVE;
                    break;

                case STATE_WAITING:
                    dwTimeout = s_SmartReflexLoadPolicyInfo.dwSensingPeriod;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_SENSING;
                    UpdateSmartReflex();
                    break;

                case STATE_SENSING:
                    dwTimeout = INFINITE;
                    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_INACTIVE;
                    SaveCalibratedVoltages();
                    UpdateSmartReflex();
                    FlushCalibratedVoltages();
                    break;
                }
            }
        }

cleanUp:
    return 0;
}

//-----------------------------------------------------------------------------
//
//  Function:  SignalSensingThread
//
//  Signals SmartReflex sensing thread to either (re)start calibration or
//  abort calibration
//
VOID
SignalSensingThread()
{
    if ((s_currentOpm == kOpm0 && s_SmartReflexLoadPolicyInfo.dwActiveDevices == 0) ||
        s_SmartReflexLoadPolicyInfo.nSensingState != STATE_INACTIVE)
        {
        SetEvent(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingEvent);
        }
}

//-----------------------------------------------------------------------------
//
//  Function:  DvfsConstraintCallback
//
//  Called whenever there is a OPM change
//
BOOL DvfsConstraintCallback(
    HANDLE hRefContext,
    DWORD msg,
    void *pParam,
    UINT size
    )
{
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(hRefContext);

    if (msg == CONSTRAINT_MSG_DVFS_NEWOPM)
        {
        s_currentOpm = (DWORD)pParam;
        // When OPM changes check if smartreflex sensing is active
        // if it is then abort activity
        SignalSensingThread();
        }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_InitPolicy
//
//  Policy  initialization
//

HANDLE
SMARTREFLEX_InitPolicy(
    _TCHAR const *szContext
    )
{
    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_InitPolicy(0x%08x)\r\n",
      szContext
      ));

    PHYSICAL_ADDRESS PhysAddr;
    DWORD dwInitOpm = CONSTRAINT_STATE_NULL;

    // initializt global structure
    memset(&s_SmartReflexLoadPolicyInfo, 0, sizeof(SmartReflexPolicyInfo_t));
    s_SmartReflexLoadPolicyInfo.dwActiveDevices = 0x1; //because LPR starts disabled
    s_SmartReflexLoadPolicyInfo.bSmartReflexState = FALSE;
    s_SmartReflexLoadPolicyInfo.nSensingState = STATE_INACTIVE;
    s_SmartReflexLoadPolicyInfo.bLprModeState = FALSE;

    // Read policy registry params
    if (GetDeviceRegistryParams(
        szContext, &s_SmartReflexLoadPolicyInfo, dimof(g_policyRegParams),
        g_policyRegParams) != ERROR_SUCCESS)
        {
        RETAILMSG(ZONE_ERROR, (L"SMARTREFLEX_InitPolicy: Invalid/Missing "
            L"registry parameters.  Unloading policy\r\n")
            );
        }

    //create DVFS change notification event
    s_SmartReflexLoadPolicyInfo.hSmartReflexSensingEvent = CreateEvent(
                                                               NULL,
                                                               FALSE,
                                                               FALSE,
                                                               NULL
                                                               );

    //start retention optimization thread
    s_SmartReflexLoadPolicyInfo.hSmartReflexSensingThread = CreateThread(
                                                                   NULL,
                                                                   0,
                                                                   SmartReflexSensingThread,
                                                                   NULL,
                                                                   0,
                                                                   NULL
                                                                   );

    // Obtain DVFS constraint handler
    s_SmartReflexLoadPolicyInfo.hDvfsConstraint = PmxSetConstraintById(
                                    CONSTRAINT_ID_DVFS,
                                    CONSTRAINT_MSG_DVFS_REQUEST,
                                    (void*)&dwInitOpm,
                                    sizeof(DWORD)
                                    );

    // register for DVFS change notifications
    s_SmartReflexLoadPolicyInfo.hDvfsCallback = PmxRegisterConstraintCallback(
                                        s_SmartReflexLoadPolicyInfo.hDvfsConstraint,
                                        DvfsConstraintCallback,
                                        NULL,
                                        0,
                                        (HANDLE)&s_SmartReflexLoadPolicyInfo
                                        );

    //map vdd1 voltage register to virtual memory
    PhysAddr.QuadPart = PRM_VP1_VOLTAGE;
    s_VddOptimizationInfo[0].pPrcmRegistryAddress = MmMapIoSpace(
                                                        PhysAddr,
                                                        sizeof(UINT32),
                                                        FALSE
                                                        );

    //map vdd2 voltage register to virtual memory
    PhysAddr.QuadPart = PRM_VP2_VOLTAGE;
    s_VddOptimizationInfo[1].pPrcmRegistryAddress = MmMapIoSpace(
                                                        PhysAddr,
                                                        sizeof(UINT32),
                                                        FALSE
                                                        );


    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_InitPolicy()\r\n"));
    return (HANDLE)&s_SmartReflexLoadPolicyInfo;
}

//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_DeinitPolicy
//
//  Policy uninitialization
//
BOOL
SMARTREFLEX_DeinitPolicy(
    HANDLE hPolicyAdapter
    )
{
    BOOL rc = FALSE;

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_DeinitPolicy(0x%08x)\r\n",
      hPolicyAdapter
      ));

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_DeinitPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    //unmap vdd1 voltage register to virtual memory
    MmUnmapIoSpace(s_VddOptimizationInfo[0].pPrcmRegistryAddress,sizeof(UINT32));

    //unmap vdd2 voltage register to virtual memory
    MmUnmapIoSpace(s_VddOptimizationInfo[1].pPrcmRegistryAddress,sizeof(UINT32));

    //unregisted DVFS change callback function
    PmxUnregisterConstraintCallback(s_SmartReflexLoadPolicyInfo.hDvfsConstraint,
                                    DvfsConstraintCallback
                                    );

    //release DVFS constraint handler
    PmxReleaseConstraint(s_SmartReflexLoadPolicyInfo.hDvfsConstraint);

    //stop thread gracefuly
    if (s_SmartReflexLoadPolicyInfo.hSmartReflexSensingThread != NULL)
        {
        s_SmartReflexLoadPolicyInfo.bExitThread = TRUE;
        SetEvent(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingEvent);
        WaitForSingleObject(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingThread, INFINITE);
        CloseHandle(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingThread);
        }

    //close sync event hanle
    CloseHandle(s_SmartReflexLoadPolicyInfo.hSmartReflexSensingEvent);

    // clear main policy structure
    memset(&s_SmartReflexLoadPolicyInfo, 0, sizeof(SmartReflexPolicyInfo_t));

    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_DeinitPolicy()\r\n"));
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_OpenPolicy
//
//  Called when someone request to open a communication handle to the power
//  policy adapter
//
HANDLE
SMARTREFLEX_OpenPolicy(
    HANDLE hPolicyAdapter
    )
{
    HANDLE                  rc = NULL;
    SmartReflexPolicyContext_t    *pContext;

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_OpenPolicy(0x%08x)\r\n",
      hPolicyAdapter
      ));

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_OpenPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    // Create a policy context handle and return handle
    pContext = (SmartReflexPolicyContext_t*)LocalAlloc(LPTR, sizeof(SmartReflexPolicyContext_t));
    if (pContext == NULL)
        {
        RETAILMSG (ZONE_WARN, (L"WARNING: SMARTREFLEX_OpenPolicy:"
            L"Policy resources allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // initialize variables
    pContext->cookie = POLICY_CONTEXT_COOKIE;

    // add to list of collection
    rc = (HANDLE)pContext;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_OpenPolicy()\r\n"));
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_ClosePolicy
//
//  device state change handler
//
BOOL
SMARTREFLEX_ClosePolicy(
    HANDLE hPolicyContext,
    UINT dev,
    UINT oldState,
    UINT newState
    )
{
    SmartReflexPolicyContext_t  *pContext = (SmartReflexPolicyContext_t*)hPolicyContext;

	UNREFERENCED_PARAMETER(dev);
	UNREFERENCED_PARAMETER(oldState);
	UNREFERENCED_PARAMETER(newState);

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_ClosePolicy(0x%08x,0xd,0x%02x,0x%02x )\r\n",
      hPolicyContext,
      dev,
      oldState,
      newState
      ));

    // validate parameters
    if (pContext == NULL || pContext->cookie != POLICY_CONTEXT_COOKIE)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_ClosePolicy:"
            L"Incorrect context parameter\r\n"
            ));
        return FALSE;
        }

    // free resources
    LocalFree(pContext);

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_ClosePolicy()\r\n"));
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_NotifyPolicy
//
//  captures messages sent to the power policy adapter
//
BOOL
SMARTREFLEX_NotifyPolicy(
    HANDLE hPolicyContext,
    DWORD msg,
    void *pParam,
    UINT size
    )
{
    BOOL rc = FALSE;
    static BOOL * pEnable = NULL;
    DWORD       dwOpm;
    SmartReflexPolicyContext_t    *pContext = (SmartReflexPolicyContext_t*)hPolicyContext;
    pEnable = (BOOL*)pParam;

	UNREFERENCED_PARAMETER(size);

    RETAILMSG(ZONE_FUNCTION, (
        L"+SMARTREFLEX_NotifyPolicy(0x%08x,0x%08x,0x%08x,%d )\r\n",
        hPolicyContext,
        msg,
        pParam,
        size
        ));

    // validate parameters
    if (pContext == NULL || pContext->cookie != POLICY_CONTEXT_COOKIE)
        {
        RETAILMSG (ZONE_ERROR, (L"ERROR: SMARTREFLEX_NotifyPolicy:"
            L"Incorrect context parameter\r\n"
            ));
        goto cleanUp;
        }

    if (pEnable == NULL)
        {
        RETAILMSG (ZONE_WARN, (L"WARNING: SMARTREFLEX_NotifyPolicy:"
            L"Incorrect input buffer context\r\n"
            ));
        goto cleanUp;
        }

    switch (msg)
        {
        case SMARTREFLEX_LPR_MODE:
            if (*pEnable)
                {

                if (s_SmartReflexLoadPolicyInfo.bLprModeState == FALSE)
                    {
                    RETAILMSG(ZONE_INFO, (L"SMARTREFLEX_NotifyPolicy: LPR mode ON\r\n"));

                    //  Enabling LPR, so drop DVFS constraint
                    dwOpm = CONSTRAINT_STATE_NULL;

                    PmxUpdateConstraint(
                        s_SmartReflexLoadPolicyInfo.hDvfsConstraint,
                        CONSTRAINT_MSG_DVFS_REQUEST,
                        (void*)&dwOpm,
                        sizeof(DWORD)
                        );

                    //  Update SmartReflex
                    s_SmartReflexLoadPolicyInfo.bLprModeState = TRUE;
                    s_SmartReflexLoadPolicyInfo.dwActiveDevices--;
                    UpdateSmartReflex();
                    }
                }
            else
                {
                if (s_SmartReflexLoadPolicyInfo.bLprModeState == TRUE)
                    {
                    RETAILMSG(ZONE_INFO, (L"SMARTREFLEX_NotifyPolicy: LPR mode OFF\r\n"));

                    //  Disabling LPR, so set DVFS constraint for minimum memory bandwidth
                    dwOpm = s_SmartReflexLoadPolicyInfo.dwLprOffOpm;

                    PmxUpdateConstraint(
                        s_SmartReflexLoadPolicyInfo.hDvfsConstraint,
                        CONSTRAINT_MSG_DVFS_REQUEST,
                        (void*)&dwOpm,
                        sizeof(DWORD)
                        );

                    //  Update SmartReflex
                    s_SmartReflexLoadPolicyInfo.bLprModeState = FALSE;
                    s_SmartReflexLoadPolicyInfo.dwActiveDevices++;
                    UpdateSmartReflex();
                    }
                }
            break;
        }
    // exit routine
    rc = TRUE;

cleanUp:
    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_NotifyPolicy()\r\n"));
    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_PreDeviceStateChange
//
//  device state change handler
//
BOOL
SMARTREFLEX_PreDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev,
    UINT oldState,
    UINT newState
    )
{
    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_PreDeviceStateChange(0x%08x,0x%02x,0x%02x,0x%02x )\r\n",
      hPolicyAdapter,
      dev,
      oldState,
      newState
      ));

    BOOL bCoreDevice = FALSE;

    // validate parameters
    if (hPolicyAdapter != (HANDLE)&s_SmartReflexLoadPolicyInfo) return FALSE;

    // record the new device state for the device
    if (dev >= OMAP_DEVICE_GENERIC) return TRUE;

    switch (dev)
        {
        case OMAP_DEVICE_I2C1:
        case OMAP_DEVICE_I2C2:
        case OMAP_DEVICE_I2C3:
        case OMAP_DEVICE_MMC1:
        case OMAP_DEVICE_MMC2:
        case OMAP_DEVICE_MMC3:
        case OMAP_DEVICE_USBTLL:
        case OMAP_DEVICE_HDQ:
        case OMAP_DEVICE_MCBSP1:
        case OMAP_DEVICE_MCBSP5:
        case OMAP_DEVICE_MCSPI1:
        case OMAP_DEVICE_MCSPI2:
        case OMAP_DEVICE_MCSPI3:
        case OMAP_DEVICE_MCSPI4:
        case OMAP_DEVICE_UART1:
        case OMAP_DEVICE_UART2:
        case OMAP_DEVICE_TS:
        case OMAP_DEVICE_GPTIMER10:
        case OMAP_DEVICE_GPTIMER11:
        case OMAP_DEVICE_MSPRO:
        case OMAP_DEVICE_EFUSE:

            // Core devices can effect the load on vdd2.  So
            // a recalibration of vdd2 may be required of currently in
            // opm0.
            bCoreDevice = true;

            // fall-through
        case OMAP_DEVICE_MCBSP2:
        case OMAP_DEVICE_MCBSP3:
        case OMAP_DEVICE_MCBSP4:
            // PER devices

            // fall-through
        case OMAP_DEVICE_SGX:
            if (newState < (UINT)D3 && oldState >= (UINT)D3)
                {
                //Add an Active device
                s_SmartReflexLoadPolicyInfo.dwActiveDevices++;
                }
            else if (newState >= (UINT)D3 && oldState < (UINT)D3)
                {
                //Remove an Active device
                s_SmartReflexLoadPolicyInfo.dwActiveDevices--;
                }

            UpdateSmartReflex();

            RETAILMSG(ZONE_INFO, (L"SmartReflexPolicy: CORE Active Devices: %d\r\n",
                s_SmartReflexLoadPolicyInfo.dwActiveDevices
                ));

            break;
        }

    // recalibrate new voltage levels if SmartReflex is disabled and we're at opm0
    if (bCoreDevice == 0)
        {
        SignalSensingThread();
        }

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_PreDeviceStateChange()\r\n"));
    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function:  SMARTREFLEX_PostDeviceStateChange
//
//  device state change handler
//
BOOL
SMARTREFLEX_PostDeviceStateChange(
    HANDLE hPolicyAdapter,
    UINT dev,
    UINT oldState,
    UINT newState
    )
{
	UNREFERENCED_PARAMETER(hPolicyAdapter);
	UNREFERENCED_PARAMETER(dev);
	UNREFERENCED_PARAMETER(oldState);
	UNREFERENCED_PARAMETER(newState);

    RETAILMSG(ZONE_FUNCTION, (
      L"+SMARTREFLEX_PostDeviceStateChange(0x%08x,0x%02x,0x%02x,0x%02x )\r\n",
      hPolicyAdapter,
      dev,
      oldState,
      newState
      ));

    RETAILMSG(ZONE_FUNCTION, (L"-SMARTREFLEX_PostDeviceStateChange()\r\n"));
    return TRUE;
}
//-----------------------------------------------------------------------------
