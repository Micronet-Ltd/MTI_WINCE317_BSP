/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This module contains the OS specific Hooks Layer default implementation
    for hooking accesses to the flash.

    Should these functions need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    The FfxHookAccessBegin() and FfxHookAccessEnd() functions provide a method
    for controlling things like Vpp or low power states.  They are called from
    the Device Manager, and may be called quite frequently.

    In some environments, Vpp control may be slow or require a delay to wait
    for Vpp to ramp up.  In this situation an alternate method to control
    Vpp would be to turn it on once and leave it on, or to turn Vpp off
    after some amount of inactivity.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhaccess.c $
    Revision 1.8  2010/09/23 08:29:47Z  garyp
    Documentation and pseudo-code updated only -- no functional changes.
    Revision 1.7  2009/07/17 20:06:16Z  garyp
    Merged from the v4.0 branch.  Added support for ACCESS_SUSPEND and
    ACCESS_RESUME.
    Revision 1.6  2009/04/08 20:32:41Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.5  2008/03/23 03:48:03Z  Garyp
    Minor data type changes.
    Revision 1.4  2007/11/29 23:23:57Z  Garyp
    Updated to maintain a usage counter tracking accesses to the flash.  Added
    pseudo-code to show how to implement a global mutex in the event that the
    flash is shared with another flash manager.
    Revision 1.3  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/10 01:14:24Z  Garyp
    Updated to use device/disk handles rather than the EXTMEDIAINFO structure.
    Revision 1.1  2005/10/06 05:45:44Z  Pauli
    Initial revision
    Revision 1.5  2004/12/30 23:34:35Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/07/23 00:13:34Z  GaryP
    Removed support for obsolete OEM status flags.
    Revision 1.3  2004/06/25 22:41:22Z  BillR
    New interface to multi-threaded FIMs.  New project hooks for
    initialization and configuration.
    Revision 1.2  2004/02/23 05:50:58Z  garys
    Revision 1.1.1.2  2004/02/23 05:50:58  garyp
    Cleaned up the function documentation.
    Revision 1.1  2004/01/07 20:49:56Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>


/*-------------------------------------------------------------------
    Public: FfxHookAccessBegin()

    Notify project code when access to the flash is about to begin.

    The FlashFX Device Manager calls this function before all accesses
    to the physical media.  If necessary, it should enable Vpp or
    perform any other operations to allow updates to the media.

    This code may also be used to serialize access to the flash when
    necessary.  Two possible scenarios when this may be necessary are 
    described below:

    1) If FlashFX is NOT the only code which touches the flash at any
       given moment, accesses to the flash by FlashFX and the "other"
       code must be serialized, otherwise the state of the flash
       hardware is likely to be corrupted.

    2) If FlashFX is configured such that more than one Device
       instance is used for a single piece of flash hardware, the
       accesses must be serialized.  For example, if a FIM or NTM is
       configured such that a single flash chip is partitioned to
       create more than one Device, access must be serialized.  
       Similarly, if there are multiple flash chips with separate
       Device instances, but they are physically managed by a single
       flash controller, access must be serialized (in both cases
       the assumption is that the FIM/NTM does not already contain
       code to serialize the accesses).

    In either of these examples, the use of a binary semaphore, is
    recommended, which which is acquired and released at the points
    shown in the pseudo-code below.  For the first scenario described
    above, the semaphore must be a globally visible object which can
    be seen by both FlashFX and the "other" code.  For the second
    scenario described above, the semaphore need not be globally
    visible, and the standard Datalight semaphore functions -- 
    DclSemaphoreAcquire/Release() are sufficient.

    *Note* -- A binary semaphore is recommended rather than a mutex,
              because in the second scenario, it is possible that
              the thread which is releasing the semaphore will be a 
              different thread from the one which acquired it.

    Parameters:
        hDev  - The Device handle.
        uType - This parameter will be one of the ACCESS_* types
                defined in oeslapi.h.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookAccessBegin(
    FFXDEVHANDLE    hDev,
    D_UINT16        uType)
{
    DclAssert(hDev);

    switch(uType)
    {
        case ACCESS_READS:
        case ACCESS_WRITES:
        case ACCESS_ERASES:
        case ACCESS_MOUNTS:
        case ACCESS_UNMOUNTS:
        case ACCESS_SUSPEND:
        case ACCESS_RESUME:
        {
            FFXDEVINFO *pDev = FfxDriverDevicePtr(hDev);

            DclAssert(pDev);

            /*  (REMINDER, we're already serialized at the Device Manager
                layer, so no special protection is needed for the usage
                counter manipulations which follow.)
            */

            /*  If the flash is shared at run-time with some other flash
                manager, a globally visible semaphore should be acquired
                at this point, if the usage count is about to go to '1',
                as shown in the pseudocode below.  The "other" flash 
                manager must be similarly constructed to acquire and 
                release the same semaphore protecting the flash.
            */


            /*  NOTE -- It is possible and expected that this code will
                        be called in a nested fashion (multiple, nested
                        begin-access calls before any end-access calls).
                        Code which manages the hardware state MUST be 
                        constructed to accommodate this characteristic.
                        The nUsageCount field may be used to assist in
                        tracking the state.
            */
/*
            if(pDev->nUsageCount == 0)
                AcquireBinarySemaphore(pSem);
*/
            /*  Provide some protection to ensure that "begins" and
                "ends" are properly matched.  Reminder that even though
                the semaphore used above may be global in scope, this
                counter is specific to the Device instance.
            */                
            pDev->nUsageCount++;

            /*  Realistically the counter should never overflow unless
                something is amiss somewhere.
            */
            DclAssert(pDev->nUsageCount != 0);

            break;
        }

        default:
            DclError();

            return FFXSTAT_PROJBADACCESSTYPE;
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookAccessEnd()

    Notify project code when access to the flash has completed.

    The FlashFX Device Manager calls this function after access to
    the physical media.  If necessary, it should disable Vpp or 
    perform any other operations that might need to be performed.

    *NOTE* -- See the comments for FfxHookAccessBegin().

    Parameters:
        hDev  - The Device handle.
        uType - This parameter will be one of the ACCESS_* types
                defined in oeslapi.h.

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookAccessEnd(
    FFXDEVHANDLE    hDev,
    D_UINT16        uType)
{
    DclAssert(hDev);

    switch(uType)
    {
        case ACCESS_READS:
        case ACCESS_WRITES:
        case ACCESS_ERASES:
        case ACCESS_MOUNTS:
        case ACCESS_UNMOUNTS:
        case ACCESS_SUSPEND:
        case ACCESS_RESUME:
        {
            FFXDEVINFO *pDev = FfxDriverDevicePtr(hDev);

            DclAssert(pDev);

            /*  (REMINDER, we're already serialized at the Device Manager
                layer, so no special protection is needed for the usage
                counter manipulations which follow.)
            */

            /*  Provide some protection to ensure that "begins" and
                "ends" are properly matched.  Reminder that even though
                the semaphore used above may be global in scope, this
                counter is specific to the Device instance.
            */                
            pDev->nUsageCount--;

            /*  Realistically the counter should never underflow unless
                something is amiss somewhere.
            */
            DclAssert(pDev->nUsageCount != UINT_MAX);

            /*  If the flash is shared at run-time with some other flash
                manager, a globally visible semaphore should be released
                at this point, if the usage count has gone to '0', as 
                shown in the pseudocode below.  The "other" flash manager
                must be similarly constructed to acquire and release the
                same semaphore protecting the flash.
            */
/*
                if(pDev->nUsageCount == 0)
                    ReleaseBinarySemaphore(pSem);
*/
            break;
        }

        default:
            DclError();

            return FFXSTAT_PROJBADACCESSTYPE;
    }

    return FFXSTAT_SUCCESS;
}



