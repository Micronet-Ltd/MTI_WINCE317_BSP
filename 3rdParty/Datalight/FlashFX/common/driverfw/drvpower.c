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

    This module contains Driver Framework functions for managing power
    suspend/resume.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvpower.c $
    Revision 1.4  2010/11/09 14:14:21Z  garyp
    Renamed some functions for interface consistency -- no functional
    changes.
    Revision 1.3  2010/07/21 16:15:47Z  garyp
    Documentation and debug code updated.
    Revision 1.2  2009/03/09 02:57:54Z  garyp
    Merged from the v4.0 branch.
    Revision 1.1.1.5  2009/03/09 02:57:54Z  garyp
    Eliminated a deprecated header.
    Revision 1.1.1.4  2008/12/17 20:23:37Z  garyp
    Fixed to handle another status code explicitly, made necessary by changes
    at a lower level.
    Revision 1.1.1.3  2008/11/29 17:09:16Z  garyp
    Updated to pass suspend/resume requests through VBF if it is being used.
    Revision 1.1.1.2  2008/07/06 18:49:58Z  garyp
    Made the code conditional on FFXCONF_POWERSUSPENDRESUME.
    Revision 1.1  2008/07/06 16:55:44Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif

#if FFXCONF_POWERSUSPENDRESUME

/*-------------------------------------------------------------------
    Public: FfxDriverDiskPowerSuspend()

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
        hDisk       - The Disk handle
        nPowerState - The power state

    Return Value:
        Returns a status code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDiskPowerSuspend(
    FFXDISKHANDLE   hDisk,
    unsigned        nPowerState)
{
    FFXDISKINFO    *pDisk;
    FFXSTATUS       ffxStat;

    FFXPRINTF(1, ("FfxDriverDiskPowerSuspend() hDisk=%lX PowerState=%u\n", hDisk, nPowerState));

    DclAssert(hDisk);

    pDisk = *hDisk;

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    /*  Disable any background compaction operations.  This also
        ensures that any in-progress compactions are completed.
    */
    FFXPRINTF(1, ("FfxDriverDiskPowerSuspend() Suspend compacter\n"));
    ffxStat = FfxDriverDiskCompactSuspend(pDisk);
    if( ffxStat != FFXSTAT_SUCCESS &&
        ffxStat != FFXSTAT_DRVCOMPACTALREADYSUSPENDED &&
        ffxStat != FFXSTAT_DRVCOMPACTNOTENABLED)
    {
        DclError();
        return ffxStat;
    }
  #endif

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    FFXPRINTF(1, ("FfxDriverDiskPowerSuspend() VBF\n"));
    if(pDisk->hVBF)
        ffxStat = FfxVbfPowerSuspend(pDisk->hVBF, nPowerState);
    else

  #endif
	{
		FFXPRINTF(1, ("FfxDriverDiskPowerSuspend() Suspend FML\n"));
        ffxStat = FfxFmlPowerSuspend(pDisk->hFML, nPowerState);
	}
    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDiskPowerResume()

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
FFXSTATUS FfxDriverDiskPowerResume(
    FFXDISKHANDLE   hDisk,
    unsigned        nPowerState)
{
    FFXDISKINFO    *pDisk;
    FFXSTATUS       ffxStat;

    FFXPRINTF(1, ("FfxDriverDiskPowerResume() hDisk=%lX PowerState=%u\n", hDisk, nPowerState));

    DclAssert(hDisk);

    pDisk = *hDisk;

  #if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
    if(pDisk->hVBF)
        ffxStat = FfxVbfPowerResume(pDisk->hVBF, nPowerState);
    else

  #endif

        ffxStat = FfxFmlPowerResume(pDisk->hFML, nPowerState);

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        /*  Re-enable any background compaction operations
        */
        FfxDriverDiskCompactResume(pDisk);
    }
  #endif

    return ffxStat;
}


#endif  /* FFXCONF_POWERSUSPENDRESUME */

