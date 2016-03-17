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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//

//
// This module contains an MDD for the battery API.  It relies on the OEM
// to provide PDD routines customized for the platform's battery hardware
// interface.
//

#include <battimpl.h>

// The last time the batteries were changed, in real time
FILETIME v_ftLastChange; 

// The last time the cpu turned on
DWORD v_imsStart;      

// How long the current batteries have been used, not counting the 
// current power cycle
DWORD v_cmsUsed;        

// How long the previous batteries lasted
DWORD v_cmsPreviousCpuUsage;

// Are we currently on external power.
BOOL v_fOnExternalPower;

// Is the stopwatch running?
BOOL v_fStopwatchRunning;

/*
BatteryAPIStopwatch

This function keeps track of how long the batteries have been in use while
the cpu is awake.  This function gets called just before power goes off and just 
after it comes back on.  See GwesPowerOff.

*/
void
BatteryAPIStopwatch (
                 BOOL fStart,      // start / stop
                 BOOL fReset     // rememeber how long the stopwatch has been on, then reset
                 )
{
    DWORD cmsT;
    if ( fReset ) {
        cmsT = v_cmsPreviousCpuUsage;
        v_cmsPreviousCpuUsage = v_cmsUsed;
        if ( v_fStopwatchRunning ) {
            v_cmsPreviousCpuUsage += GetTickCount() - v_imsStart;
        }
        if ( 0 == v_cmsPreviousCpuUsage ) {
            // If user ran just on AC, don't destroy his previous battery time.
            v_cmsPreviousCpuUsage = cmsT;
        }
        v_cmsUsed = 0;
    }
    if ( fStart && (!v_fStopwatchRunning || fReset) ) {
        v_imsStart = GetTickCount();
        v_fStopwatchRunning = TRUE;
    } else if ( !fStart && v_fStopwatchRunning ) {
        v_cmsUsed += GetTickCount() - v_imsStart;
        v_fStopwatchRunning = FALSE;
    }
}



/*
    @func DWORD | BatteryAPIGetSystemPowerStatusEx2 | Retrieves the power status of the 
    system.

    @parm PSYSTEM_POWER_STATUS_EX2 | pstatus | buffer to be filled in
    @parm DWORD | wLen | Length of pstatus buffer
    @parm BOOL | fUpdate | Get the latest information from the device driver,
            as opposed to cached information that may be out-of-date by several 
            seconds
*/
DWORD WINAPI
BatteryAPIGetSystemPowerStatusEx2(
                       PSYSTEM_POWER_STATUS_EX2 pstatus,
                       DWORD dwLen,
                       BOOL fUpdate
                       )
{
    SYSTEM_POWER_STATUS_EX2 *pCachedStatus, *pTempCachedStatus;
    BOOL fBatteriesChangedSinceLastCall;
    FILETIME ftNow;
    SYSTEMTIME st;

    ASSERT( gpPddBuffer != NULL && gpPddBufferTemp != NULL );
    if( gpPddBuffer == NULL || gpPddBufferTemp == NULL )
        return 0;

    pCachedStatus = (SYSTEM_POWER_STATUS_EX2 *)gpPddBuffer;
    pTempCachedStatus = (SYSTEM_POWER_STATUS_EX2 *)gpPddBufferTemp;

    if ( ! pstatus )
        return( 0 );

    if ( fUpdate ) {
        LOCKBATTERY();
        try {
            // get latest information
            BatteryPDDGetStatus (pTempCachedStatus, &fBatteriesChangedSinceLastCall);

            if ( pTempCachedStatus->ACLineStatus != AC_LINE_OFFLINE ) {
                if ( !v_fOnExternalPower ) {
                    v_fOnExternalPower = TRUE;
                    BatteryAPIStopwatch (FALSE, FALSE); // stop
                }
            } else {
                if ( v_fOnExternalPower ) {
                    v_fOnExternalPower = FALSE;
                    BatteryAPIStopwatch (TRUE, FALSE); // start
                }
            }

            // if BATTERY_STATUS.fBatteriesChangedSinceLastCall or if we just 
            // booted and have no last change time
            if ( fBatteriesChangedSinceLastCall || 
                 (0 == v_ftLastChange.dwHighDateTime && 
                  0 == v_ftLastChange.dwLowDateTime) ) {
                GetLocalTime (&st);
                SystemTimeToFileTime (&st, &ftNow);
                v_ftLastChange = ftNow;
                BatteryAPIStopwatch (!v_fOnExternalPower, /*fReset*/TRUE);
            }
            DEBUGMSG( 0, (TEXT("fBatteriesChangedSinceLastCall %X\r\n"), (WORD)fBatteriesChangedSinceLastCall));        
        }except(EXCEPTION_EXECUTE_HANDLER) {
            UNLOCKBATTERY();
            SetLastError(ERROR_GEN_FAILURE);
            return 0;
        }

        // to avoid blocking threads that called BatteryAPIGetSystemPowerStatusEx2 
        // with the fUpdate=FALSE, we called the battery PDD functions passing a temporary status buffer.
        // now we will copy the contents of that buffer into the real status buffer.  This way we are 
        // reducing the time that other threads may have to wait blocked.
        LOCKPDDBUFFER();
        memcpy( pCachedStatus, pTempCachedStatus, gdwPddBufferSize );
        UNLOCKPDDBUFFER();
        UNLOCKBATTERY();
    }

    LOCKPDDBUFFER();
    try {
        // Copy data to users buffer
        if ( dwLen > gdwPddBufferSize )
            dwLen = gdwPddBufferSize;
        memcpy( pstatus, pCachedStatus, dwLen );

        DEBUGMSG( 0, (TEXT("ACLineStatus %X\r\n"), pCachedStatus->ACLineStatus));
        DEBUGMSG( 0, (TEXT("BatteryFlag %X\r\n"), pCachedStatus->BatteryFlag));
        DEBUGMSG( 0, (TEXT("BatteryLifePercent %X\r\n"), pCachedStatus->BatteryLifePercent));
        DEBUGMSG( 0, (TEXT("BatteryLifeTime %X\r\n"), pCachedStatus->BatteryLifeTime));
        DEBUGMSG( 0, (TEXT("BatteryFullLifeTime %X\r\n"), pCachedStatus->BatteryFullLifeTime));
        DEBUGMSG( 0, (TEXT("BackupBatteryFlag %X\r\n"), pCachedStatus->BackupBatteryFlag));
        DEBUGMSG( 0, (TEXT("BackupBatteryLifePercent %X\r\n"), pCachedStatus->BackupBatteryLifePercent));
        DEBUGMSG( 0, (TEXT("BackupBatteryLifeTime %X\r\n"), pCachedStatus->BackupBatteryLifeTime));
        DEBUGMSG( 0, (TEXT("BackupBatteryFullLifeTime %X\r\n"), pCachedStatus->BackupBatteryFullLifeTime));
        DEBUGMSG( 0, (TEXT("BatteryVoltage %X\r\n"), pCachedStatus->BatteryVoltage));
        DEBUGMSG( 0, (TEXT("BatteryCurrent %X\r\n"), pCachedStatus->BatteryCurrent));
        DEBUGMSG( 0, (TEXT("BatteryAverageCurrent %X\r\n"), pCachedStatus->BatteryAverageCurrent));
        DEBUGMSG( 0, (TEXT("BatteryAverageInterval %X\r\n"), pCachedStatus->BatteryAverageInterval));
        DEBUGMSG( 0, (TEXT("BatterymAHourConsumed %X\r\n"), pCachedStatus->BatterymAHourConsumed));
        DEBUGMSG( 0, (TEXT("BatteryTemperature %X\r\n"), pCachedStatus->BatteryTemperature));
        DEBUGMSG( 0, (TEXT("BackupBatteryVoltage %X\r\n"), pCachedStatus->BackupBatteryVoltage));
        DEBUGMSG( 0, (TEXT("BatteryChemistry %X\r\n"), pCachedStatus->BatteryChemistry));
    }except(EXCEPTION_EXECUTE_HANDLER) {
        dwLen = 0;
        SetLastError(ERROR_GEN_FAILURE);
    }
    UNLOCKPDDBUFFER();

    return (dwLen);
}


/*
    @func BOOL | BatteryAPIGetSystemPowerStatusEx | Retrieves the power status of the 
    system.

    @parm PSYSTEM_POWER_STATUS_EX | pstatus | buffer to be filled in
    @parm BOOL | fUpdate | Get the latest information from the device driver,
            as opposed to cached information that may be out-of-date by several 
            seconds
*/
BOOL WINAPI
BatteryAPIGetSystemPowerStatusEx(
                      PSYSTEM_POWER_STATUS_EX pstatus,
                      BOOL fUpdate
                      )
{
    DWORD dwRetLen;

    if ( ! pstatus )
        return( FALSE );

    // Just call the new Ex2 version and tell it we have a smaller structure
    dwRetLen = BatteryAPIGetSystemPowerStatusEx2( (PSYSTEM_POWER_STATUS_EX2)pstatus, sizeof(SYSTEM_POWER_STATUS_EX), fUpdate );
    DEBUGMSG( 0, (TEXT("GetSystemPowerStatusEx Len %d\r\n"), dwRetLen));
    DEBUGMSG( 0, (TEXT("ACLineStatus %X\r\n"), pstatus->ACLineStatus));
    DEBUGMSG( 0, (TEXT("BatteryFlag %X\r\n"), pstatus->BatteryFlag));
    DEBUGMSG( 0, (TEXT("BatteryLifePercent %X\r\n"), pstatus->BatteryLifePercent));
    DEBUGMSG( 0, (TEXT("BatteryLifeTime %X\r\n"), pstatus->BatteryLifeTime));
    DEBUGMSG( 0, (TEXT("BatteryFullLifeTime %X\r\n"), pstatus->BatteryFullLifeTime));
    DEBUGMSG( 0, (TEXT("BackupBatteryFlag %X\r\n"), pstatus->BackupBatteryFlag));
    DEBUGMSG( 0, (TEXT("BackupBatteryLifePercent %X\r\n"), pstatus->BackupBatteryLifePercent));
    DEBUGMSG( 0, (TEXT("BackupBatteryLifeTime %X\r\n"), pstatus->BackupBatteryLifeTime));
    DEBUGMSG( 0, (TEXT("BackupBatteryFullLifeTime %X\r\n"), pstatus->BackupBatteryFullLifeTime));

    if ( dwRetLen ==  sizeof(SYSTEM_POWER_STATUS_EX) )
        return( TRUE );
    else
        return( FALSE );
}


/*
    @func void | BatteryAPIGetLifeTimeInfo | Retrieves information on how long
    the batteries have lasted.

    @parm LPSYSTEMTIME | pstLastChange | The time (in local time) when the
    batteries were last changed.

    @parm DWORD * | pcmsCpuUsage | The amount of time the current batteries
    have been in active use.
    
    @parm DWORD * | pcmsPreviousCpuUsage | The amount of time the previous
    batteries were in active use before being replaced, or zero if unknown.
    
*/
void WINAPI
BatteryAPIGetLifeTimeInfo (
                       LPSYSTEMTIME pstLastChange,
                       DWORD * pcmsCpuUsage,
                       DWORD * pcmsPreviousCpuUsage
                       )
{
    try {
        if ( pstLastChange )
            FileTimeToSystemTime (&v_ftLastChange, pstLastChange);

        if ( pcmsCpuUsage ) {
            *pcmsCpuUsage = v_cmsUsed;
            if ( v_fStopwatchRunning ) {
                *pcmsCpuUsage += GetTickCount() - v_imsStart;
            }
        }

        if ( pcmsPreviousCpuUsage )
            *pcmsPreviousCpuUsage = v_cmsPreviousCpuUsage;
    }except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
}

void add64_64_64(const FILETIME *lpnum1, LPFILETIME lpnum2, LPFILETIME lpres) {
    __int64 num1, num2;
    num1 = (((__int64)lpnum1->dwHighDateTime)<<32)+(__int64)lpnum1->dwLowDateTime;
    num2 = (((__int64)lpnum2->dwHighDateTime)<<32)+(__int64)lpnum2->dwLowDateTime;
    num1 += num2;
    lpres->dwHighDateTime = (DWORD)(num1>>32);
    lpres->dwLowDateTime = (DWORD)(num1&0xffffffff);
}

void sub64_64_64(const FILETIME *lpnum1, LPFILETIME lpnum2, LPFILETIME lpres) {
    __int64 num1, num2;
    num1 = (((__int64)lpnum1->dwHighDateTime)<<32)+(__int64)lpnum1->dwLowDateTime;
    num2 = (((__int64)lpnum2->dwHighDateTime)<<32)+(__int64)lpnum2->dwLowDateTime;
    num1 -= num2;
    lpres->dwHighDateTime = (DWORD)(num1>>32);
    lpres->dwLowDateTime = (DWORD)(num1&0xffffffff);
}


/*
    @func void | BatteryAPINotifyOfTimeChange | Adjust times to account for the user
    changing the real (clock) time.

    @parm BOOL | fForward | Time has jumped forward

    @parm FILETIME * | pftDelta | The amount the time has changed
    
*/
void WINAPI
BatteryAPINotifyOfTimeChange (
                          BOOL fForward,
                          FILETIME * pftDelta
                          )
{
    try {
        if ( pftDelta ) {
            if ( fForward ) {
                add64_64_64 (&v_ftLastChange, pftDelta, &v_ftLastChange);
            } else {
                sub64_64_64 (&v_ftLastChange, pftDelta, &v_ftLastChange);
            }
        }
    }except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
}   


