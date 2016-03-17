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

    This is an FML framework that allows FML specific, All-In-One versions of
    the TOOLS and TESTS to be run, without having to mess with the details of
    FML initialization and shutdown.  It also allows the TOOLS/TESTS to be
    run on a range of drives if desired.

    This framework is only for apps that use the FMSL without VBF.  VBF has
    a similar framework for apps that use FMSL and VBF (most apps).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvfmlfw.c $
    Revision 1.12  2010/01/09 19:03:09Z  garyp
    Message and documentation updates -- no functional changes.
    Revision 1.11  2009/07/22 17:53:53Z  garyp
    Corrected the call to FfxSignOn().
    Revision 1.10  2009/07/21 21:10:53Z  garyp
    Merged from the v4.0 branch.  Updated to the new calling convention for
    FfxDriverCreate().
    Revision 1.9  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.8  2009/03/31 18:07:08Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.7  2009/03/05 06:38:34Z  keithg
    Added call to SignOn() and FfxProjMain() in initialization.
    Revision 1.6  2009/02/09 07:15:59Z  garyp
    Merged from the v4.0 branch.  Use the new FFXTOOLPARAMS structure.
    Revision 1.5  2008/03/25 00:04:02Z  Garyp
    Updated to use some slightly modified driver initialization functions.
    Revision 1.4  2008/01/13 07:26:24Z  keithg
    Function header updates to support autodoc.
    Revision 1.3  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/10/03 23:49:49Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.1  2006/03/15 22:26:32Z  Garyp
    Initial revision
    Revision 1.3  2006/02/12 18:56:58Z  Garyp
    Modified to only deal with one device/disk.
    Revision 1.2  2006/02/06 20:10:00Z  Garyp
    Refactored such that the FML is literally just the flash mapping layer,
    other functionality is moved into the Device Manager layer.
    Revision 1.1  2005/10/06 05:50:22Z  Pauli
    Initial revision
    Revision 1.7  2004/12/30 17:32:47Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.6  2004/11/20 04:27:58Z  GaryP
    Updated to use refactored header files.
    Revision 1.5  2004/08/15 04:16:40Z  GaryP
    Minor function renaming exersize
    Revision 1.4  2004/07/20 01:31:15Z  GaryP
    Eliminated support for the "AllDrives" and "Quiet" options.  Modified to
    pass the FFXDISKDATA to FfxHookDiskCreate().
    Revision 1.3  2004/07/02 20:59:42Z  GaryP
    Updated to use the Driver Framework.
    Revision 1.2  2004/02/22 19:12:44Z  garys
    Revision 1.1.1.4  2004/02/22 19:12:44  garyp
    Modified to no longer use oemchanged().
    Revision 1.1.1.3  2004/01/19 03:40:56Z  garyp
    Modified the error message to display the number of drives.
    Revision 1.1.1.2  2003/12/04 22:56:34Z  garyp
    Modified to use the new oemmount() function.
    Revision 1.1  2003/11/17 22:19:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxtools.h>
#include <fxaiof.h>


/*-------------------------------------------------------------------
    Protected: FfxDriverFmlAllInOneFramework()

    Parameters:

    Return Value:
        The D_INT16 value returned from the TEST/TOOL, or -1 if a
        framework specific error occurred.
-------------------------------------------------------------------*/
D_INT16 FfxDriverFmlAllInOneFramework(
    PFNFRAMEWORKCMD     pfnFWCommand,
    FFXTOOLPARAMS      *pTP,
    FFXDEVINITDATA     *pDeviceData,
    FFXDISKINITDATA    *pDiskData)
{
    D_INT16             iReturn = -1;
    FFXDRIVERINFO      *pDI = NULL;
    FFXSTATUS           ffxStat;
    FFXDEVHANDLE        hDev = NULL;
    FFXDISKHANDLE       hDisk = NULL;

    DclAssert(pfnFWCommand);
    DclAssert(pTP);

    if(!FfxProjMain())
    {
        DclPrintf("\n%s: Initialization cancelled by FfxProjMain\n", pTP->dtp.pszCmdName);
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
}


