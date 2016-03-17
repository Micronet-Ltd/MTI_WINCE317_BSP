// All rights reserved ADENEO EMBEDDED 2010
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
//  File:  powerdomaincontext.cpp
//
//
#include "omap.h"
#include <windows.h>
#include <oal.h>
#include <oalex.h>
#include <ceddk.h>
#include <ceddkex.h>
#include "omap_dvfs.h"
#include <list>
#include <indexlist.h>
#include "_constants.h"
#include "powerdomainconstraint.h"

using namespace std;

//-----------------------------------------------------------------------------
//  registry key names
//
#define REGEDIT_POWERDOMAIN_MPU     (_T("MPU"))
#define REGEDIT_POWERDOMAIN_IVA     (_T("IVA"))
#define REGEDIT_POWERDOMAIN_CORE    (_T("CORE"))
#define REGEDIT_POWERDOMAIN_PERIPHERAL (_T("PERIPHERAL"))
#define REGEDIT_POWERDOMAIN_USBHOST (_T("USBHOST"))
#define REGEDIT_POWERDOMAIN_SGX     (_T("SGX"))
#define REGEDIT_POWERDOMAIN_DSS     (_T("DSS"))
#define REGEDIT_POWERDOMAIN_CAMERA  (_T("CAMERA"))
#define REGEDIT_POWERDOMAIN_NEON    (_T("NEON"))

#define REGEDIT_POWERDOMAIN_FMT     (_T("%s\\%s"))

//-----------------------------------------------------------------------------
//  local structures
typedef struct {
    CRITICAL_SECTION                cs;
    BOOL                            bCallbackMode;
    DWORD                           currentTID;
    PowerDomainConstraint          *rgPowerDomainStates[POWERDOMAIN_COUNT];
    IndexList<DWORD>                IndexList;
} PowerDomainConstraintInfo_t;

typedef struct {
    HANDLE rgConstraints[POWERDOMAIN_COUNT];
} ConstraintContext;

typedef struct {
    DWORD                           idContext;
    ConstraintCallback              fnCallback;
    HANDLE                          hRefContext;
    HANDLE                          hConstraintContext;
    HANDLE                          hCallbackContext;
} PowerDomainCallbackInfo_t;      

//-----------------------------------------------------------------------------
//  local variables
static list<PowerDomainCallbackInfo_t*> s_CallbackList;
static PowerDomainConstraintInfo_t  s_PowerDomainInfo;

//-----------------------------------------------------------------------------
// 
//  Function:  Lock
//
//  enter critical section
//
static
void
Lock()
{
    EnterCriticalSection(&s_PowerDomainInfo.cs);
}

//-----------------------------------------------------------------------------
// 
//  Function:  Unlock
//
//  exit critical section
//
static
void
Unlock()
{
    LeaveCriticalSection(&s_PowerDomainInfo.cs);
}

//-----------------------------------------------------------------------------
//
//  Method: NotifyCallbacks
//
void
NotifyDomainCallbacks(
    UINT powerDomain, 
    UINT state,
    DWORD idContext
    )
{
    POWERDOMAIN_CONSTRAINT_INFO constraintInfo;
    list<PowerDomainCallbackInfo_t*>::iterator iter;

    // Prevent further modification until all clients are notified.
    s_PowerDomainInfo.bCallbackMode = TRUE;
    s_PowerDomainInfo.currentTID = GetCurrentThreadId();

    // initialize data structure
    constraintInfo.state = state;
    constraintInfo.powerDomain = powerDomain;
    constraintInfo.size = sizeof(POWERDOMAIN_CONSTRAINT_INFO);
    
    for (iter = s_CallbackList.begin(); iter != s_CallbackList.end(); ++iter)
        {
        // don't notify the caller who triggered the change
        if ((*iter)->idContext == idContext) continue;

        // notify callbacks
        (*iter)->fnCallback((*iter)->hRefContext, 
            CONSTRAINT_MSG_POWERDOMAIN_UPDATE, 
            (void*)&constraintInfo,
            sizeof(POWERDOMAIN_CONSTRAINT_INFO)
            );
        }
    
    s_PowerDomainInfo.bCallbackMode = FALSE;
}

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_InitConstraint
//
//  Constraint initialization
//
HANDLE
PWRDOM_InitConstraint(
    _TCHAR const *szContext
    )
{
    _TCHAR szBuffer[MAX_PATH];
    
    // Initialize data structure
    memset(&s_PowerDomainInfo.rgPowerDomainStates, 0, sizeof(s_PowerDomainInfo.rgPowerDomainStates));

    // instantiate each power domain which supports constraints

    // mpu
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_MPU);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_MPU] = new MpuPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_MPU] != NULL)
        {
        // Don't need to flush the power state because the cpu load power policy adapter
        // will do it for us
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_MPU]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_MPU]->FlushDomainPowerState(); 
        }

    // core
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_CORE);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CORE] = new CorePowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CORE] != NULL)
        {
        // Depend on external power policy adapter to change the power state of core
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CORE]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CORE]->FlushDomainPowerState(); 
        }

    // iva
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_IVA);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_IVA2] = new IvaPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_IVA2] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_IVA2]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_IVA2]->FlushDomainPowerState();        
        }

    // usbhost
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_USBHOST);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_USBHOST] = new UsbHostPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_USBHOST] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_USBHOST]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_USBHOST]->FlushDomainPowerState();
        }

    // dss
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_DSS);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_DSS] = new DssPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_DSS] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_DSS]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_DSS]->FlushDomainPowerState();
        }

    // sgx
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_SGX);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_SGX] = new SgxPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_SGX] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_SGX]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_SGX]->FlushDomainPowerState();
        }

    // peripheral
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_PERIPHERAL);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_PERIPHERAL] = new PeripheralPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_PERIPHERAL] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_PERIPHERAL]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_PERIPHERAL]->FlushDomainPowerState();
        }

    // camera
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_CAMERA);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CAMERA] = new CameraPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CAMERA] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CAMERA]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_CAMERA]->FlushDomainPowerState();
        }

    // neon
    _stprintf(szBuffer, REGEDIT_POWERDOMAIN_FMT, szContext, REGEDIT_POWERDOMAIN_NEON);
    s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_NEON] = new NeonPowerDomainConstraint();
    if (s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_NEON] != NULL)
        {
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_NEON]->InitializePowerDomain(szBuffer);
        s_PowerDomainInfo.rgPowerDomainStates[POWERDOMAIN_NEON]->FlushDomainPowerState();
        }

    InitializeCriticalSection(&s_PowerDomainInfo.cs);
    
    return (HANDLE)&s_PowerDomainInfo;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_DeinitConstraint
//
//  Constraint initialization
//
BOOL
PWRDOM_DeinitConstraint(
    HANDLE hConstraintAdapter
    )
{
    BOOL rc = FALSE;

    // validate parameters
    if (hConstraintAdapter != (HANDLE)&s_PowerDomainInfo) goto cleanUp;

    // don't do anything as this constraint adapter is meant to be loaded
    // all the time.

    rc = TRUE;

cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_CreateConstraint
//
//  Constraint uninitialization
//
HANDLE
PWRDOM_CreateConstraint(
    HANDLE hConstraintAdapter
    )
{
    BOOL b;
    DWORD id;
    DWORD *pDataNode;
    HANDLE rc = NULL;
    ConstraintContext *pConstraintContext;
    
    // validate parameters
    if (hConstraintAdapter != (HANDLE)&s_PowerDomainInfo) goto cleanUp;

    // create structure to maintain constraints for this handle
    pConstraintContext = (ConstraintContext*)LocalAlloc(LPTR, sizeof(ConstraintContext));
    if (pConstraintContext == NULL) goto cleanUp;
    memset(pConstraintContext->rgConstraints, 0, sizeof(pConstraintContext->rgConstraints));
    
    // get new index
    Lock();
    b = s_PowerDomainInfo.IndexList.NewIndex(&pDataNode, &id);
    Unlock();
    
    if (b == FALSE)
        {
        LocalFree(pConstraintContext);
        goto cleanUp;
        }

    // initialize values
    rc = (HANDLE)(id + 1);
    *pDataNode = (DWORD)pConstraintContext;

cleanUp:    
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_UpdateConstraint
//
//  Constraint update
//
BOOL
PWRDOM_UpdateConstraint(
    HANDLE hConstraintContext, 
    DWORD msg, 
    void *pParam, 
    UINT  size
    )
{       
    BOOL rc = FALSE;    
    PowerDomainConstraint *pPowerDomain; 
    ConstraintContext *pConstraintContext;
    POWERDOMAIN_CONSTRAINT_INFO *pConstraintInfo; 
    
    DWORD id = (DWORD)hConstraintContext - 1;
    
    // validate parameters
    if (size != sizeof(POWERDOMAIN_CONSTRAINT_INFO)) goto cleanUp;
    if (id > s_PowerDomainInfo.IndexList.MaxIndex()) goto cleanUp;

    // don't allow power domain updates on the thread the callback 
    // notification is occurring on or else some general synchronous issues
    // will occurr
    if (s_PowerDomainInfo.bCallbackMode == TRUE &&
        s_PowerDomainInfo.currentTID == GetCurrentThreadId())
        {
        goto cleanUp;
        }

    // validate structure
    pConstraintInfo = (POWERDOMAIN_CONSTRAINT_INFO*)pParam;
    if (pConstraintInfo->size != sizeof(POWERDOMAIN_CONSTRAINT_INFO)) goto cleanUp;
    if (pConstraintInfo->powerDomain >= POWERDOMAIN_COUNT) goto cleanUp;
    pPowerDomain = s_PowerDomainInfo.rgPowerDomainStates[pConstraintInfo->powerDomain];
    if (pPowerDomain == NULL) goto cleanUp;

    // get constraint list
    pConstraintContext = *(ConstraintContext**)s_PowerDomainInfo.IndexList.GetIndex(id);
    if (pConstraintContext == NULL) goto cleanUp;
    
    if (pConstraintContext->rgConstraints[pConstraintInfo->powerDomain] == NULL)
        {
        pConstraintContext->rgConstraints[pConstraintInfo->powerDomain] = 
            pPowerDomain->CreatePowerDomainConstraint();
        }

    // apply constraint
    Lock();
    rc = pPowerDomain->UpdatePowerDomainConstraint(
            pConstraintContext->rgConstraints[pConstraintInfo->powerDomain],
            msg,
            pConstraintInfo->state,
            sizeof(pConstraintInfo->state),
            id
            );      
    Unlock();
    
    
cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_CloseConstraint
//
//  Constraint releasing constraint handle
//
BOOL
PWRDOM_CloseConstraint(
    HANDLE hConstraintContext
    )
{
    BOOL rc = FALSE;
    ConstraintContext *pConstraintContext;
    DWORD id = (DWORD)hConstraintContext - 1;
    
    // validate
    if (id > s_PowerDomainInfo.IndexList.MaxIndex()) goto cleanUp;

    // get data node
    pConstraintContext = *(ConstraintContext**)s_PowerDomainInfo.IndexList.GetIndex(id);
    if (pConstraintContext == NULL) goto cleanUp;

    Lock();    
    for (int i = 0; i < POWERDOMAIN_COUNT; ++i)
        {
        if (pConstraintContext->rgConstraints[i] != NULL)
            {
            s_PowerDomainInfo.rgPowerDomainStates[i]->ClosePowerDomainContraint(
                pConstraintContext->rgConstraints[i]
                );
            }
        }

    LocalFree(pConstraintContext);    
    s_PowerDomainInfo.IndexList.DeleteIndex(id);  
    Unlock();
    
    rc = TRUE;
cleanUp:    
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_CreateConstraintCallback
//
//  Constraint callback registeration
//
HANDLE
PWRDOM_InsertConstraintCallback(
    HANDLE hConstraintContext, 
    void *pCallback, 
    void *pParam, 
    UINT  size,
    HANDLE hRefContext
    )
{
    HANDLE rc = NULL;
    DWORD *pDataNode;
    DWORD id = (DWORD)hConstraintContext - 1;
    PowerDomainCallbackInfo_t *pCallbackInfo;

    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(pParam);

    // validate
    if (id >s_PowerDomainInfo.IndexList.MaxIndex()) goto cleanUp;

    // get data node
    pDataNode = s_PowerDomainInfo.IndexList.GetIndex(id);
    if (pDataNode == NULL) goto cleanUp;

    // allocate callback structure
    pCallbackInfo = (PowerDomainCallbackInfo_t*)LocalAlloc(LPTR, sizeof(PowerDomainCallbackInfo_t));
    if (pCallbackInfo == NULL) goto cleanUp;

    // initialize structure
    pCallbackInfo->idContext = id;
    pCallbackInfo->fnCallback = (ConstraintCallback)pCallback;
    pCallbackInfo->hConstraintContext = hConstraintContext;
    pCallbackInfo->hCallbackContext = pCallbackInfo;
    pCallbackInfo->hRefContext = hRefContext;
    
    // insert into callback list
    Lock();
    s_CallbackList.push_back(pCallbackInfo);
    Unlock();
    
cleanUp:
    return rc;
} 

//-----------------------------------------------------------------------------
// 
//  Function:  PWRDOM_RemoveConstraintCallback
//
//  Constraint callback unregisterations
//
BOOL
PWRDOM_RemoveConstraintCallback(
    HANDLE hConstraintCallback
    )
{
    BOOL rc = FALSE;
    list<PowerDomainCallbackInfo_t*>::iterator iter;
    
    // find entry with matching callback info
    Lock();
    for (iter = s_CallbackList.begin(); iter != s_CallbackList.end(); ++iter)
        {
        if ((*iter)->hCallbackContext == hConstraintCallback)
            {
            // free resources
            LocalFree(*iter);
            s_CallbackList.erase(iter);
            rc = TRUE;
            break;
            }
        }
    Unlock();

    return rc;
} 

//------------------------------------------------------------------------------
