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

    This module contains the default functionality for obtaining device or
    disk specific configuration options which are stored in the Driver
    Framework.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvopt.c $
    Revision 1.12  2010/11/22 16:19:50Z  glenns
    Added option for disk allocator option flags. First use is for
    allocator QuickMount feature.
    Revision 1.11  2009/07/18 00:53:23Z  garyp
    Eliminated use of an obsolete header.
    Revision 1.10  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.9  2009/03/31 19:20:00Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.8  2008/03/13 19:12:24Z  Garyp
    Renamed the function to allow the hook interface to be inserted in the
    chain.  Minor datatype updates.
    Revision 1.7  2008/01/13 07:26:31Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/02/01 02:38:41Z  Garyp
    Added support for handling device timing and latency settings.
    Revision 1.4  2006/10/09 19:27:52Z  Garyp
    Added and removed options pertaining for formatting.
    Revision 1.3  2006/02/13 00:53:59Z  Garyp
    Added options.
    Revision 1.2  2006/02/12 00:04:01Z  Garyp
    Added standard compaction settings.
    Revision 1.1  2006/02/09 19:41:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverFWOptionGet()

    Get a configuration option.  This function is normally only
    called from FfxHookOptionGet() to obtain an option that is
    standard for all projects using a given OS.

    Parameters:
        opt       - The option identifier (FFXOPT_*).
        handle    - The FFXDEVHANDLE or FFXDISKHANDLE.
        pBuffer   - A pointer to object to receive the option value.
                    May be NULL.
        ulBuffLen - The size of object to receive the option value.
                    May be zero if pBuffer is NULL.

    Return Value:
        TRUE if the option identifier is valid, the option value is
        available and either pBuffer is NULL or ulBuffLen is the
        appropriate size for the option value, otherwise FALSE.
-------------------------------------------------------------------*/
D_BOOL FfxDriverFWOptionGet(
    FFXOPTION       opt,
    void           *handle,
    void           *pBuffer,
    D_UINT32        ulBuffLen)
{
    D_BOOL          fResult = FALSE;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEINDENT),
        "FfxDriverFWOptionGet() handle=%P Type=%x pBuff=%P Len=%lU\n",
        handle, opt, pBuffer, ulBuffLen));

    switch (opt)
    {
        case FFXOPT_DEVICE_NUMBER:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(unsigned));
                *(unsigned*)pBuffer = pDev->Conf.nDevNum;
            }
            fResult = TRUE;
            break;

        case FFXOPT_DEVICE_SETTINGS:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXDEVSETTINGS));
                *(FFXDEVSETTINGS*)pBuffer = pDev->Conf.DevSettings;
            }
            fResult = TRUE;
            break;

      #if FFXCONF_LATENCYREDUCTIONENABLED
        case FFXOPT_DEVICE_TIMING:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXDEVTIMING));
                *(FFXDEVTIMING*)pBuffer = pDev->Conf.DevTiming;
            }
            fResult = TRUE;
            break;

        case FFXOPT_DEVICE_LATENCY:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXDEVLATENCY));
                *(FFXDEVLATENCY*)pBuffer = pDev->Conf.DevLatency;
            }
            fResult = TRUE;
            break;
      #endif

      #if FFXCONF_FORMATSUPPORT
        case FFXOPT_DISK_FORMATSETTINGS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXFORMATSETTINGS));
                *(FFXFORMATSETTINGS*)pBuffer = pDisk->Conf.FormatSettings;
            }
            fResult = TRUE;
            break;
      #endif

      #if FFXCONF_ALLOCATORSUPPORT
        case FFXOPT_DISK_ALLOCSETTINGS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXALLOCSETTINGS));
                *(FFXALLOCSETTINGS*)pBuffer = pDisk->Conf.AllocSettings;
            }
            fResult = TRUE;
            break;

        case FFXOPT_DISK_ALLOCOPTIONFLAGS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(D_UINT32));
                *(D_UINT32 *)pBuffer = pDisk->Conf.AllocSettings.ulOptionFlags;
            }
            fResult = TRUE;
            break;
      #endif

      #if FFXCONF_BBMSUPPORT
        case FFXOPT_DEVICE_BBMSETTINGS:
            DclAssert(FfxDriverDeviceHandleValidate(handle));
            if(pBuffer)
            {
                FFXDEVINFO *pDev = *(FFXDEVHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(FFXBBMSETTINGS));
                *(FFXBBMSETTINGS*)pBuffer = pDev->Conf.BbmSettings;
            }
            fResult = TRUE;
            break;
      #endif

      #if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS
        case FFXOPT_COMPACTIONREADIDLEMS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(D_UINT32));
                *(D_UINT32*)pBuffer = pDisk->Conf.CompSettings.ulReadIdleMS;
            }
            fResult = TRUE;
            break;
        case FFXOPT_COMPACTIONWRITEIDLEMS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(D_UINT32));
                *(D_UINT32*)pBuffer = pDisk->Conf.CompSettings.ulWriteIdleMS;
            }
            fResult = TRUE;
            break;

      #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
        case FFXOPT_COMPACTIONPRIORITY:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(D_UINT16));
                *(D_UINT16*)pBuffer = pDisk->Conf.CompSettings.uThreadPriority;
            }
            fResult = TRUE;
            break;
        case FFXOPT_COMPACTIONSLEEPMS:
            DclAssert(FfxDriverDiskHandleValidate(handle));
            if(pBuffer)
            {
                FFXDISKINFO *pDisk = *(FFXDISKHANDLE)handle;

                DclAssert(ulBuffLen == sizeof(D_UINT32));
                *(D_UINT32*)pBuffer = pDisk->Conf.CompSettings.ulSleepMS;
            }
            fResult = TRUE;
            break;
      #endif
      #endif

        default:

            /*  Other parameter codes may be recognized by FlashFX.
            */
            fResult = FfxOsOptionGet(opt, handle, pBuffer, ulBuffLen);
            break;
    }

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_CONFIG, 1, TRACEUNDENT),
        "FfxDriverFWOptionGet() returning %U\n", fResult));

    return fResult;
}
