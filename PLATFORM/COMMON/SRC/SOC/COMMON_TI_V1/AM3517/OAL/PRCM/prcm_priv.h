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
//  File: prcm_priv.h
//
#include "oal_prcm.h"

#pragma warning (push)
#pragma warning (disable:4200)

//-----------------------------------------------------------------------------
#define MAX_IDLESTATUS_LOOP         (0x1000)
#define cm_offset(field)            ((LONG)&(((OMAP_CM_REGS*)0)->field))
#define prm_offset(field)           ((LONG)&(((OMAP_PRM_REGS*)0)->field))
#define AUTOIDLE_DECL(a, x, y, z)   ClockOffsetInfo_3 a = {x, y, z}
#define IDLESTAT_DECL(a, x, y)      ClockOffsetInfo_1 a = {x, y}
#define ICLK_DECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, FALSE}
#define ICLK_VDECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, TRUE}
#define FCLK_DECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, FALSE}
#define FCLK_VDECL(a, x, y, z)       ClockOffsetInfo_2 a = {x, y, z, TRUE}
#define WKEN_DECL(a, x, y)          ClockOffsetInfo_1 a = {x, y}

//-----------------------------------------------------------------------------
// common device structures

typedef struct {
    UINT                    mask;
    UINT                    offset;
} ClockOffsetInfo_1;

typedef struct {
    LONG                    refCount;    
    UINT                    mask;
    UINT                    offset;
    BOOL                    bVirtual;
} ClockOffsetInfo_2;

typedef struct {
    UINT                    enabled;
    UINT                    mask;
    UINT                    offset;
} ClockOffsetInfo_3;

typedef struct {
    DWORD                   size;
    SourceClock_e           rgSourceClocks[];    
} SourceDeviceClocks_t;

typedef struct {
    PowerDomain_e           powerDomain;
    ClockOffsetInfo_2      *pfclk;
    ClockOffsetInfo_2      *piclk;
    ClockOffsetInfo_1      *pwken;    
    ClockOffsetInfo_1      *pidlestatus;
    ClockOffsetInfo_3      *pautoidle;
    SourceDeviceClocks_t   *pSrcClocks;
} DeviceLookupEntry;

// Note: This structure definition must match the duplicate definition in src\oal\oallib\oem_prcm_device.c
typedef struct {
    ClockDomain_e           clockDomain;
    DWORD                   requiredWakeupDependency;
    DWORD                   requiredSleepDependency;
} DomainDependencyRequirement;

//------------------------------------------------------------------------------
// Power domain definition

typedef struct {
    ClockDomain_e           clockDomain;
    UINT                    clockState;
    UINT                    clockShift;
} ClockDomainState_t;

typedef struct {
    UINT                    count;
    ClockDomainState_t      rgClockDomains[];
} ClockDomainInfo_t;

typedef struct {
    UINT                    powerState;
    UINT                    logicState;
    UINT                    sleepDependency;
    UINT                    wakeDependency;     
} PowerDomainState_t;

typedef struct {
    LONG                    refCount;
    UINT                    ffValidationMask;
    PowerDomainState_t     *pDomainState;
    ClockDomainInfo_t      *pClockStates;
    UINT                    rgDeviceContextState[3];
} PowerDomainInfo_t; 

typedef PowerDomainInfo_t   DomainMap[POWERDOMAIN_COUNT];

//------------------------------------------------------------------------------
// Voltage domain definition

typedef int                 VddRefCountTable[kVDD_COUNT];

//------------------------------------------------------------------------------
// DPLL domain definition

typedef struct {
    UINT                    lowPowerEnabled;
    UINT                    rampTime;    
    UINT                    freqSelection;
    UINT                    driftGuard;
    UINT                    dpllMode;              // off, bypass, locked
    UINT                    sourceDivisor;
    UINT                    multiplier;
    UINT                    divisor;
    UINT                    dpllAutoidleState;
    UINT                    outputDivisor;
} DpllState_t;

typedef struct {
    int                     vddDomain;
    LONG                    refCount;
    DpllState_t            *pDpllInfo;
} Dpll_Entry_t;

typedef Dpll_Entry_t        DpllMap[kDPLL_COUNT];

//------------------------------------------------------------------------------
// Root clk src definition

typedef struct {
    int                     dpllDomain;
    LONG                    refCount;
} DpllClkOut_Entry_t;

typedef DpllClkOut_Entry_t  DpllClkOutMap[kDPLL_CLKOUT_COUNT];

//------------------------------------------------------------------------------
// secondary clk src definition.  

typedef struct {
    UINT                    id;
    UINT                    divisor;
} SrcClockDivisor_Entry_t;

typedef struct {
    UINT                    count;
    SrcClockDivisor_Entry_t SourceClock[];
} SrcClockDivisorTable_t;

typedef struct {
    int                     parentClk;
    LONG                    refCount;
    BOOL                    bIsDpllSrcClk;
    SrcClockDivisorTable_t *pDivisors;
    int                     thisClk;
} SrcClock_Entry_t;

typedef SrcClock_Entry_t    SrcClockMap[kSOURCE_CLOCK_COUNT];

//------------------------------------------------------------------------------
//
//  Global:  g_PrcmPrm
//
//  Reference to all PRCM-PRM registers. Initialized in OALPowerInit
//
extern OMAP_PRCM_PRM       *g_pPrcmPrm;

//------------------------------------------------------------------------------
//
//  Global:  g_PrcmCm
//
//  Reference to all PRCM-CM registers. Initialized in OALPowerInit
//
extern OMAP_PRCM_CM        *g_pPrcmCm;

//------------------------------------------------------------------------------
//
//  Global:  g_PrcmPostInit
//
//  Indicates if prcm library has been fully initialized. 
//  Initialized in OALPowerPostInit
//
extern BOOL                 g_PrcmPostInit;

//------------------------------------------------------------------------------
//
//  Global:  g_PrcmMutex
//
//  Contains a list of CRITICAL_SECTIONS used for synchronized access to 
//  PRCM registers
//
typedef enum
{
    Mutex_DeviceClock = 0,
    Mutex_Clock,
    Mutex_Domain,
    Mutex_Voltage,
    Mutex_Power,
    Mutex_Intr,
    Mutex_Count
} PRCMMutex_Id;

extern CRITICAL_SECTION     g_rgPrcmMutex[Mutex_Count];

//------------------------------------------------------------------------------
//
//  Global:  g_bSingleThreaded
//
//  Indicated that the OAL PRCM functions are being called while the kernel is 
//  single threaded (OEMIdle, OEMPoweroff)
//
extern BOOL g_bSingleThreaded;

//------------------------------------------------------------------------------
// internal/helper routines

BOOL
ClockInitialize();

BOOL
ClockUpdateParentClock(
    int srcClkId,
    BOOL bEnable
    );

BOOL
DomainInitialize();

BOOL
ResetInitialize();

BOOL
DeviceInitialize();

OMAP_CM_REGS*
GetCmRegisterSet(
    UINT powerDomain
    );

OMAP_PRM_REGS*
GetPrmRegisterSet(
    UINT powerDomain
    );

BOOL
DomainGetDeviceContextState(
    UINT                powerDomain,
    ClockOffsetInfo_2  *pInfo,
    BOOL                bSet
    );

//------------------------------------------------------------------------------
// synchronization routines
__inline void 
Lock(
    PRCMMutex_Id mutexId
    )
{
    if (g_PrcmPostInit && !g_bSingleThreaded) 
        {
        EnterCriticalSection(&g_rgPrcmMutex[mutexId]);
        }
}

__inline void 
Unlock(
    PRCMMutex_Id mutexId
    )
{
    if (g_PrcmPostInit && !g_bSingleThreaded)
        {
        LeaveCriticalSection(&g_rgPrcmMutex[mutexId]);
        }
}

#pragma warning (pop)

//------------------------------------------------------------------------------

