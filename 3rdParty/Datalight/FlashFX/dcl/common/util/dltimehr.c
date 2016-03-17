/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

  In addition to civil penalties for infringement of copyright under appli-
  cable U.S. law, 17 U.S.C. 1204 provides criminal penalties for violation
  of (a) the restrictions on circumvention of copyright protection systems
  found in 17 U.S.C. 1201 and (b) the protections for the integrity of
  copyright management information found in 17 U.S.C. 1202.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  A SOURCE CODE AGREEMENT, NON-DISCLOSURE AGREEMENT, AND/OR SIMILAR BINDING
  CONTRACT BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
  IF YOU ARE A LICENSEE, YOUR RIGHT, IF ANY, TO COPY, PUBLISH, MODIFY, OR
  OTHERWISE USE THE SOFTWARE, IS SUBJECT TO THE TERMS AND CONDITIONS OF THE
  BINDING AGREEMENTS.  BY USING THE SOFTWARE IN ANY MANNER, IN WHOLE OR IN
  PART, YOU AGREE TO BE BOUND BY THE TERMS OF THE BINDING AGREEMENTS.

  IF YOU ARE NOT A DATALIGHT LICENSEE, ANY USE MAY RESULT IN CIVIL AND
  CRIMINAL ACTION AGAINST YOU.  CONTACT DATALIGHT, INC. AT THE ADDRESS
  SET FORTH ABOVE IF YOU OBTAINED THIS SOFTWARE IN ERROR.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This module contain high resolution timing services.

    NOTE:  The accuracy of these services depends on the OS Services Layer
           implementation of a high resolution ticker.  This may not exist
           in all environments.  For those environments, the OS Services
           Layer code will typically use the standard system tick, and
           using the high resolution timestamps will buy nothing, other
           than adding the overhead of doing 64-bit math.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltimehr.c $
    Revision 1.12  2010/01/04 22:12:15Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.11  2009/11/16 03:17:17Z  garyp
    Added some missing typecasts.
    Revision 1.10  2009/11/09 14:56:12Z  garyp
    Updated so the whole module is conditional on DCLCONF_HIGHRESTIMESTAMP.
    Removed those 64-bit macros which are no longer necessary.
    Revision 1.9  2009/06/28 00:46:03Z  garyp
    Updated to use this feature as a service.
    Revision 1.8  2009/02/08 00:30:03Z  garyp
    Merged from the v4.0 branch.  Added DclHighResTimePassedUS().
    Revision 1.7  2008/06/03 21:43:09Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.6  2007/12/18 20:21:51Z  brandont
    Updated function headers.
    Revision 1.5  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.4  2006/12/16 21:34:30Z  Garyp
    Documentation fixes.
    Revision 1.3  2006/10/22 01:50:24Z  Garyp
    Added DclHighResTimePassedMS().
    Revision 1.2  2006/10/05 23:44:45Z  Garyp
    Modified to use a corrected macro name.
    Revision 1.1  2006/06/26 23:07:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlservice.h>
#include <dlinstance.h>

#if DCLCONF_HIGHRESTIMESTAMP

static D_BOOL   fNanos;             /* Resolution is in nanoseconds per tick */
static D_UINT32 ulTickResolution;   /* pico/nanoseconds per tick             */
static D_UINT64 ullTickModulus;     /* tick count rolls over; or 0           */


/*-------------------------------------------------------------------
    Public: DclHighResTimeServiceInit()

    This function initializes the HighResTime subsystem.  Typically
    this is called one time, very early in the startup process.  This
    service does not allocate any system resources, and therefore
    does not require any shutdown operations.

    Parameters:
        hDclInst - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS DclHighResTimeServiceInit(
    DCLINSTANCEHANDLE       hDclInst)
{
    static DCLSERVICEHEADER SrvHRTime[DCL_MAX_INSTANCES];
    DCLSERVICEHEADER       *pSrv;
    DCLSTATUS               dclStat;
    D_UINT64                ullTickResolution;

    /*  REMINDER! Because some environments are capable of restarting,
                  without reinitializing static data, this function must
                  explicitly ensure that any static data is manually
                  zero'd, rather than just expecting it to already be in
                  that state.
    */

    /*  The "resolution" field is the only field for which the above
        REMINDER matters...
    */
    ulTickResolution = 0;

    if(!hDclInst)
        return DCLSTAT_SERVICE_BADHANDLE;

    /*  There are potential thread safety issues with the following
        initialization process, however this function should be
        called early before any such issues can occur.
    */
    ullTickModulus = DclOsHighResTickModulus();
    ullTickResolution = DclOsHighResTickResolution();   /* picoseconds per tick */

    if(ullTickResolution > D_UINT32_MAX)
    {
        DclUint64DivUint32(&ullTickResolution, 1000UL);
        fNanos = TRUE;
    }
    else
    {
        fNanos = FALSE;
    }

    if(ullTickResolution > D_UINT32_MAX)
         return DCLSTAT_TIME_HRTICKRESOLUTION;

    ulTickResolution = (D_UINT32)ullTickResolution;

    if(ulTickResolution == 0)
        return DCLSTAT_TIME_HRTICKRESOLUTION;

    pSrv = &SrvHRTime[DclInstanceNumber(hDclInst)];
    dclStat = DclServiceHeaderInit(&pSrv, "DLHRTIME", DCLSERVICE_HRTIME, NULL, NULL, 0);
    if(dclStat == DCLSTAT_SUCCESS)
        dclStat = DclServiceCreate(hDclInst, pSrv);

    return dclStat;
}


/*-------------------------------------------------------------------
    Public: DclHighResTimeStamp()

    Get a timestamp representing the current time suitable for use as
    an argument to DclHighResTimePassed(), DclHighResTimePassedMS(),
    or DclHighResTimePassedUS().

    Parameters:
        None.

    Return Value:
        Returns a DCLHRTIMESTAMP value.
-------------------------------------------------------------------*/
DCLHRTIMESTAMP DclHighResTimeStamp(void)
{
    return DclOsHighResTickCount();
}


/*-------------------------------------------------------------------
    Public: DclHighResTimePassed()

    Determine the elapsed time in nanoseconds since a high-res
    timestamp was obtained with DclHighResTimeStamp().

    The maximum number of nanoseconds that can be measured is
    never greater than the equivalent of ~584 years, however it
    is also dependent on the system tick modulus value, which
    is platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    Parameters:
        tTimestamp - A timestamp value returned by
                     DclHighResTimeStamp()

    Return Value:
        Returns the number of nanoseconds which have elapsed since
        the call to DclHighResTimeStamp().

        Returns D_UINT64_MAX if the HighResTime service has not been
        initialized or if the HighResTime functionality is not
        available.

        If the timer has exceeded the maximum period which it is
        capable of timing, the return value is undefined.
-------------------------------------------------------------------*/
D_UINT64 DclHighResTimePassed(
    DCLHRTIMESTAMP  tTimestamp)
{
    DCLHRTIMESTAMP  tNow = DclOsHighResTickCount();
    D_UINT64        ullElapsedTicks = tNow;

    /*  Return an error code if the service has not been initialized
    */
    if(!ulTickResolution)
    {
        D_UINT64    ullHRMax = DCLINIT64MAX;
        return ullHRMax;
    }

    ullElapsedTicks -= tTimestamp;

    /*  Check for reaching the tick modulus.  Note that there is no way
        to know whether it has actually been reached multiple times.
    */
    if(tNow < tTimestamp)
        ullElapsedTicks += ullTickModulus;

    DclUint64MulUint32(&ullElapsedTicks, ulTickResolution);

    if(!fNanos)
        DclUint64DivUint32(&ullElapsedTicks, 1000UL);

    return ullElapsedTicks;
}


/*-------------------------------------------------------------------
    Public: DclHighResTimePassedMS()

    Determine the elapsed time in milliseconds since a high-res
    timestamp was obtained with DclHighResTimeStamp().

    The maximum number of milliseconds that can be measured is
    never greater than the equivalent of ~49.7 days, however it
    is also dependent on the system tick modulus value, which is 
    platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    This function is normally only used when the standard timers
    which are millisecond based are mapped so that the high-res
    timers are used instead.

    Parameters:
        tTimestamp - A timestamp value returned by
                     DclHighResTimeStamp()

    Return Value:
        Returns the number of milliseconds which have elapsed since
        the call to DclHighResTimeStamp().

        Returns D_UINT32_MAX if the HighResTime service has not been
        initialized or if the HighResTime functionality is not
        available.

        If the timer has exceeded the maximum period which it is
        capable of timing, the return value is undefined.
-------------------------------------------------------------------*/
D_UINT32 DclHighResTimePassedMS(
    DCLHRTIMESTAMP  tTimestamp)
{
    D_UINT64        ullNanos;
    D_UINT64        ullCompare = DCLINIT64MAX;

    ullNanos = DclHighResTimePassed(tTimestamp);

    /*  Return an error code if the service has not been initialized
    */
    if(ullNanos == ullCompare)
        return D_UINT32_MAX;

    /*  Convert nanoseconds to milliseconds rounding as best we can
    */
    ullNanos += 500000UL;
    DclUint64DivUint32(&ullNanos, 1000000UL);

    return (D_UINT32)ullNanos;
}


/*-------------------------------------------------------------------
    Public: DclHighResTimePassedUS()

    Determines the elapsed time in microseconds since the timestamp
    was marked.

    The maximum number of microseconds that can be measured is
    never greater than the equivalent of ~71.5 minutes, however
    it is also dependent on the system tick modulus value, which
    is platform dependent.

    If tTimestamp is zero, this function will effectively return
    the elapsed time since system startup, with the same caveats
    regarding the maximum timeable period.

    Parameters:
        tTimestamp - A timestamp value returned by DclTimeStamp()

    Return Value:
        Returns the number of microseconds that have elapsed.  If
        the time period is longer than the system tick modulus, the
        return value is undefined.
-------------------------------------------------------------------*/
D_UINT32 DclHighResTimePassedUS(
    DCLHRTIMESTAMP    tTimestamp)
{
    D_UINT64        ullNanos;

    ullNanos = DclHighResTimePassed(tTimestamp);

    /*  Convert nanoseconds to microseconds rounding as best we can
    */
    ullNanos += 500UL;
    DclUint64DivUint32(&ullNanos, 1000UL);

    return (D_UINT32)ullNanos;
}


#endif  /* DCLCONF_HIGHRESTIMESTAMP */

