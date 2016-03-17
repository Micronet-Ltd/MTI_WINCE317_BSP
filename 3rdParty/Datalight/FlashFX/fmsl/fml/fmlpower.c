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

    This function handle power suspend/resume requests at the FML Disk
    level.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fmlpower.c $
    Revision 1.3  2010/07/21 16:14:24Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.2  2009/02/18 04:27:11Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.5  2009/02/18 04:27:11Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.1.1.4  2008/12/17 21:29:46Z  garyp
    Fixed a polarity problem introduced in the previous rev.
    Revision 1.1.1.3  2008/12/16 21:02:25Z  garyp
    Major update to support the concept of a Disk which spans multiple Devices.
    Revision 1.1.1.2  2008/07/06 18:57:56Z  garyp
    Removed an assert.
    Revision 1.1  2008/07/06 17:22:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxfmlapi.h>
#include <fxdevapi.h>
#include "fml.h"


#if FFXCONF_POWERSUSPENDRESUME


static D_BOOL IsDeviceInUse(FFXFMLHANDLE hFML, unsigned nDevNum);


/*-------------------------------------------------------------------
    Protected: FfxFmlPowerSuspend()

    Prepare the specified Disk for a power suspend operation.  Note
    that if this Disk resides on a Device which is shared with other
    Disks, the actual call down the stack to the FIM/NTM's suspend
    function will *NOT* be issued until the last Disk receives the
    suspend request.

    All I/O requests to the Disk must be completed, and no new
    requests issued, once this call has been made.

    The nPowerState value is currently unused by the FlashFX call
    stack, but is provided for OEM use in the FIM or NTM as needed.
    Nevertheless it should be used with caution, as Datalight may
    use this value in the future.

    Parameters:
        hFML        - The FML handle
        nPowerState - The power state

    Return Value:
        Returns a status code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlPowerSuspend(
    FFXFMLHANDLE                hFML,
    unsigned                    nPowerState)
{
    FFXFMLLAYOUT               *pLayout;
    FFXSTATUS                   ffxStat = FFXSTAT_SUCCESS;
    FFXIOSTATUS                 ioStat;
    FFXIOR_DEV_POWER_SUSPEND    DevReq;
    unsigned                    nDevNum;

    DclAssert(hFML);

    FFXPRINTF(1, ("FfxFmlPowerSuspend() hFML=%lX PowerState=%u OldSuspendCount=%lU\n",
        hFML, nPowerState, DclOsAtomic32Retrieve(&hFML->ulSuspendCount)));

    /*  Mark this Disk as being suspended
    */
    if(DclOsAtomic32Increment(&hFML->ulSuspendCount) != 1)
        return FFXSTAT_FML_ALREADYSUSPENDED;

    /*  We've confirmed that any Disks using this Device have all been
        marked as suspended.  Now tell the DevMgr about it, for each
        Device this Disk uses.
    */

    DclMemSet(&DevReq, 0, sizeof(DevReq));

    DevReq.ior.ioFunc   = FXIOFUNC_DEV_POWER_SUSPEND;
    DevReq.ior.ulReqLen = sizeof DevReq;
    DevReq.nPowerState  = nPowerState;

    pLayout = hFML->pLayout;

    nDevNum = hFML->nStartDevNum;

    while(pLayout)
    {
        if(IsDeviceInUse(hFML, nDevNum))
        {
            ffxStat = FFXSTAT_FML_SUSPENDPARTIAL;
        }
        else
        {
            ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
            if(ioStat.ffxStat != FFXSTAT_SUCCESS)
                break;
        }

        pLayout = pLayout->pNext;
        nDevNum++;
    }

    if(ioStat.ffxStat == FFXSTAT_SUCCESS)
        return ffxStat;
    else
        return ioStat.ffxStat;

    /*  ToDo: We should consider whether a failure to suspend at the
              DevMgr level means we should decrement our Disk suspend
              count, so it is not suspended.  The issue is moot at this
              point because the OS code currently used to test this,
              absolutely expects suspend to work every time.
    */
}


/*-------------------------------------------------------------------
    Protected: FfxFmlPowerResume()

    Indicate to the specified Disk that a power resume operation has
    occurred.

    The nPowerState value is currently unused by the FlashFX call
    stack, but is provided for OEM use in the FIM or NTM as needed.
    Nevertheless it should be used with caution, as Datalight may
    use this value in the future.

    Parameters:
        hDisk       - The Disk handle
        nPowerState - The power state

    Return Value:
        Returns a status code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFmlPowerResume(
    FFXFMLHANDLE            hFML,
    unsigned                nPowerState)
{
    FFXFMLLAYOUT           *pLayout;
    FFXIOSTATUS             ioStat;
    FFXIOR_DEV_POWER_RESUME DevReq;

    DclAssert(hFML);

    FFXPRINTF(1, ("FfxFmlPowerResume() hFML=%lX PowerState=%u OldSuspendCount=%lU\n",
        hFML, nPowerState, DclOsAtomic32Retrieve(&hFML->ulSuspendCount)));

    /*  Reduce the suspend count
    */
    if(DclOsAtomic32Decrement(&hFML->ulSuspendCount) != 0)
        return FFXSTAT_FML_STILLSUSPENDED;

    DclMemSet(&DevReq, 0, sizeof(DevReq));

    DevReq.ior.ioFunc   = FXIOFUNC_DEV_POWER_RESUME;
    DevReq.ior.ulReqLen = sizeof DevReq;
    DevReq.nPowerState  = nPowerState;

    pLayout = hFML->pLayout;

    while(pLayout)
    {
        ioStat = FfxDevIORequest(pLayout->hFimDev, &DevReq.ior);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        pLayout = pLayout->pNext;
    }

    return ioStat.ffxStat;

    /*  ToDo: We should consider whether a failure to resume at the
              DevMgr level means we should increment our Disk suspend
              count, so it is still suspended.  The issue is moot at
              this point because the OS code currently used to test
              this, absolutely expects suspend to work every time.
    */
}


/*-------------------------------------------------------------------
    Local: IsDeviceInUse()

    Iterate through all the Disks (excepting the one denoted by hFML)
    and make sure there are no other Disks mapped to the specified
    Device, which are not yet suspended.

    Parameters:
        hFML        - The FML handle
        nDevNum     - The Device to check

    Return Value:
        Returns TRUE if the Device is still in use, or FALSE
        otherwise.
 -------------------------------------------------------------------*/
static D_BOOL IsDeviceInUse(
    FFXFMLHANDLE    hFML,
    unsigned        nDevNum)
{
    unsigned        nn;

    for(nn=0; nn < FFX_MAX_DISKS; nn++)
    {
        FFXFMLHANDLE hFMLTemp = FfxFmlHandle(nn);

        if(!hFMLTemp || (hFMLTemp == hFML))
            continue;

        if( (nDevNum >= hFMLTemp->nStartDevNum) &&
            (nDevNum <= hFMLTemp->nStartDevNum + hFMLTemp->nDevCount - 1) )
        {
            if(DclOsAtomic32Retrieve(&hFMLTemp->ulSuspendCount) == 0)
                return TRUE;
        }
    }

    return FALSE;
}




#endif  /* FFXCONF_POWERSUSPENDRESUME */


