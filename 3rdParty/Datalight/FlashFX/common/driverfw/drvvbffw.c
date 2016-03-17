/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This is a VBF framework that allows All-In-One versions of the
    TOOLS and TESTS to be run, without having to mess with the details
    of FMSL and VBF initialization and shutdown.  It also allows the
    TOOLS/TESTS to be run on a range of drives if desired.

    This framework is for apps that use all of FlashFX (FMSL and VBF).
    If you are using only the FMSL, use the FMSL specific framework,
    rather than this one.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvvbffw.c $
    Revision 1.12  2009/07/21 21:49:44Z  garyp
    Merged from the v4.0 branch.  Modified the shutdown processes to take a
    mode parameter.  Updated to use the revised FfxSignOn() function, which
    now takes an fQuiet parameter.
    Revision 1.11  2009/04/09 21:26:36Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.10  2009/02/09 06:46:40Z  davidh
    Function Hearders updated for AutoDoc.
    Revision 1.9  2009/02/09 06:46:40Z  garyp
    Merged with the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.8  2008/03/25 00:04:01Z  Garyp
    Updated to compile when allocator support is disabled.
    Revision 1.7  2008/01/13 07:26:37Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/02/10 01:10:34Z  timothyj
    Added call to FfxProjMain() in initialization.
    Revision 1.4  2006/10/11 00:34:17Z  Garyp
    Modified to use new allocator create/destroy functions.
    Revision 1.3  2006/10/03 23:51:05Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.2  2006/07/01 18:57:54Z  Garyp
    Updated to use the new FfxDriverVbfCreate() function.
    Revision 1.1  2006/03/15 22:24:46Z  Garyp
    Initial revision
    Revision 1.4  2006/03/07 20:36:27Z  Garyp
    Modified the framework to continue even if the VBF instance creation fails,
    since some tools will function without VBF being loaded.
    Revision 1.3  2006/02/12 18:54:38Z  Garyp
    Modified to only deal with one device/disk.
    Revision 1.2  2006/02/06 20:10:00Z  Garyp
    Updated to use the new model for creating devices and disks.
    Revision 1.1  2005/10/06 04:50:30Z  Garyp
    Initial revision
    Revision 1.10  2004/12/30 17:32:47Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.9  2004/11/20 04:27:59Z  GaryP
    Updated to use refactored header files.
    Revision 1.8  2004/08/15 04:16:44Z  GaryP
    Minor function renaming exersize
    Revision 1.7  2004/07/20 01:31:15Z  GaryP
    Eliminated support for the "AllDrives" and "Quiet" options.  Modified to
    pass the FFXDISKDATA to FfxHookDiskCreate().
    Revision 1.6  2004/07/03 02:08:02Z  GaryP
    Updated to use the Driver Framework.
    Revision 1.5  2004/02/22 19:04:00Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.5  2004/02/22 19:04:00  garyp
    Modified to no longer use oemchanged().
    Revision 1.2.1.4  2004/01/19 03:40:56Z  garyp
    Modified the error message to display the number of drives.
    Revision 1.2.1.3  2003/12/04 22:56:34Z  garyp
    Modified to use the new oemmount() function.
    Revision 1.2.1.2  2003/11/03 05:58:56Z  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/11/03 05:58:56Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/05/02 17:04:20Z  garyp
    Fixed an out-of-date comment.
    Revision 1.1  2003/03/23 05:40:06Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlerrlev.h>
#include <fxdriver.h>
#include <fxaiof.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverVbfAllInOneFramework()

    Parameters:

    Return Value:
        The D_INT16 value returned from the TEST/TOOL or -1 if
        an AIOF specific error occurs.
-------------------------------------------------------------------*/
D_INT16 FfxDriverVbfAllInOneFramework(
    PFNFRAMEWORKCMD     pfnFWCommand,
    FFXTOOLPARAMS      *pTP,
    FFXDEVINITDATA     *pDeviceData,
    FFXDISKINITDATA    *pDiskData)
{
  #if FFXCONF_VBFSUPPORT
    D_INT16             iReturn = -1;
    FFXDRIVERINFO      *pDI = NULL;
    FFXSTATUS           ffxStat;
    FFXDEVHANDLE        hDev = NULL;
    FFXDISKHANDLE       hDisk = NULL;

    DclAssert(pfnFWCommand);
    DclAssert(pTP);

    if(!FfxProjMain())
    {
        DclPrintf("\n%s: init cancelled by FfxProjMain\n", pTP->dtp.pszCmdName);
        goto Cleanup;
    }

    FfxSignOn(FALSE);

    pDI = FfxDriverCreate(1, pTP->dtp.hDclInst);
    if(!pDI)
    {
        DclPrintf("\n%s: Initialization failed\n", pTP->dtp.pszCmdName);
        goto Cleanup;
    }

    hDev = FfxDriverDeviceCreate(pDI, pDeviceData);
    if(!hDev)
    {
        DclPrintf("\n%s failed, DEV%u could not be initialized\n", pTP->dtp.pszCmdName, pTP->nDeviceNum);
        goto Cleanup;
    }

    hDisk = FfxDriverDiskCreate(pDI, pDiskData);
    if(!hDisk)
    {
        DclPrintf("\n%s failed, DISK%u could not be initialized\n", pTP->dtp.pszCmdName, pTP->nDiskNum);
        goto Cleanup;
    }

    ffxStat = FfxDriverAllocatorCreate(hDisk);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("FFX: VBF instance creation failed for DISK%u\n", pTP->nDiskNum);
    }

    /*  Call the tool function
    */
    iReturn = (*pfnFWCommand)(pTP);

    if(iReturn)
        DclPrintf("\n%s failed, return code %D\n", pTP->dtp.pszCmdName, iReturn);
    else
        DclPrintf("\n%s completed successfully\n", pTP->dtp.pszCmdName);

  Cleanup:

    /*  Destroy objects in reverse order of creation
    */
    if(hDisk)
    {
        ffxStat = FfxDriverAllocatorDestroy(hDisk, FFX_SHUTDOWNFLAGS_NORMAL);
        if(ffxStat != FFXSTAT_SUCCESS)
            DclPrintf("Error destroying VBF instance for DISK%u, Status=%lX\n", pTP->nDiskNum, ffxStat);

        ffxStat = FfxDriverDiskDestroy(hDisk);
        if(ffxStat != FFXSTAT_SUCCESS)
            DclPrintf("Error destroying DISK%u, Status=%lX\n", pTP->nDiskNum, ffxStat);
    }

    if(hDev)
    {
        ffxStat = FfxDriverDeviceDestroy(hDev);
        if(ffxStat != FFXSTAT_SUCCESS)
            DclPrintf("Error destroying DEV%u, Status=%lX\n", pTP->nDeviceNum, ffxStat);
    }

    if(pDI)
        FfxDriverDestroy(pDI);

    return iReturn;

  #else

    (void)pfnFWCommand;
    (void)pTP;
    (void)pDeviceData;
    (void)pDiskData;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}




