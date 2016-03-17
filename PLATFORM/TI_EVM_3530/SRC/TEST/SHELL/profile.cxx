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
#include <windows.h>
#include <bsp.h>
#include <twl.h>
#include <bus.h>
#include <display.h>

#include <omap35xx_guid.h>
#include "utils.h"
#include <i2cproxy.h>
#include <proxyapi.h>
#include <tps659xx_bci.h>
#include <omap_prof.h>

//-----------------------------------------------------------------------------
typedef VOID (*PFN_FmtPuts)(WCHAR *s, ...);
typedef WCHAR* (*PFN_Gets)(WCHAR *s, int cch);

//-----------------------------------------------------------------------------
extern HANDLE GetProxyDriverHandle();

//-----------------------------------------------------------------------------
BOOL
ProfileInterruptLatency(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    // get option parameters
    int nTargetSamples = GetOptionLong((int*)&argc, args, 100, L"s");
    
    DeviceIoControl(
            GetProxyDriverHandle(), 
            IOCTL_PROFILE_INTERRUPTLATENCY, 
            &nTargetSamples, 
            sizeof(int), 
            NULL, 
            0, 
            NULL, 
            NULL
            );
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
ProfileDvfs(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    DVFS_STRESS_TEST_PARAMETERS     stressParam;

    int nTargetSamples = GetOptionLong((int*)&argc, args, 100, L"s");

    // get frequency of dvfs change
    stressParam._period = GetOptionLong((int*)&argc, args, 500, L"t");
    if (stressParam._period == NULL) goto cleanUp;

    // get duration of stress test
    stressParam._duration = GetOptionLong((int*)&argc, args, 30, L"d");
    if (stressParam._duration == NULL) goto cleanUp;

    // get test mode (random or between two operating points)
    stressParam._random = GetOptionFlag((int*)&argc, args, L"r") == TRUE;

    if(stressParam._random == FALSE )
    {
        // identify operating points if between two modes
        stressParam._hiopm = GetOptionLong((int*)&argc, args, 1, L"h");
        if (stressParam._hiopm < 1 || stressParam._hiopm > 5) goto cleanUp;
    
        stressParam._lowopm = GetOptionLong((int*)&argc, args, 0, L"l");
        if (stressParam._lowopm < 0 || stressParam._lowopm > 4 ||
            stressParam._lowopm >= stressParam._hiopm) 
            {
            goto cleanUp;
            }   

        //stressParam._hiopm--;
        //stressParam._lowopm--;
    }

    stressParam._duration *= 1000;  //convert seconds to mili seconds
    
    rc = DeviceIoControl(
            GetProxyDriverHandle(), 
            IOCTL_PROFILE_DVFS, 
            &stressParam, 
            sizeof(DVFS_STRESS_TEST_PARAMETERS), 
            NULL, 
            0, 
            NULL, 
            NULL
            );

cleanUp:
    
    return rc;
}

//-----------------------------------------------------------------------------
BOOL
ProfileSdrcStall(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    // get option parameters
    int nTargetSamples = GetOptionLong((int*)&argc, args, 100, L"s");
    
    DeviceIoControl(
            GetProxyDriverHandle(), 
            IOCTL_PROFILE_DVFS_CORE, 
            &nTargetSamples, 
            sizeof(int), 
            NULL, 
            0, 
            NULL, 
            NULL
            );
        
    return TRUE;
}

//-----------------------------------------------------------------------------
BOOL
ProfileWakeupAccuracy(
    ULONG argc,
    LPWSTR args[],
    PFN_FmtPuts pfnFmtPuts
    )
{
    BOOL rc = FALSE;
    WAKEUPACCURACY_TEST_PARAMETERS    testParam;

    // get option parameters
    testParam._sleepPeriod = GetOptionLong((int*)&argc, args, 200, L"t");;
    testParam._numberOfSamples = GetOptionLong((int*)&argc, args, 100, L"s");
        
    DeviceIoControl(
            GetProxyDriverHandle(), 
            IOCTL_PROFILE_WAKEUPACCURACY, 
            &testParam, 
            sizeof(WAKEUPACCURACY_TEST_PARAMETERS), 
            NULL, 
            0, 
            NULL, 
            NULL
            );
        
    return TRUE;
}

//-----------------------------------------------------------------------------
