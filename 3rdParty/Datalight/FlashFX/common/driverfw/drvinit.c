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

    This module contains helper routines that are used by the OS level
    device driver.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvinit.c $
    Revision 1.18  2010/09/23 05:55:23Z  garyp
    Updated documentation and debug messages -- no functional changes.
    Revision 1.17  2010/08/18 21:12:30Z  garyp
    Fixed documentation/comment typos.
    Revision 1.16  2010/07/16 14:57:04Z  garyp
    Fixed the destroy function to accommodate reducing the DCL instance
    usage count, rather than destroying the instance.
    Revision 1.15  2010/01/10 20:31:33Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.14  2009/07/22 18:01:59Z  garyp
    Corrected return values in FfxDriverDestroy().
    Revision 1.13  2009/07/21 21:14:57Z  garyp
    Merged from th ev 4.0 branch.  Eliminated a deprecated header.  Updated
    FfxDriverDestroy() to return an FFXSTATUS value rather than void.  Updated
    to create a DCL instance on the fly, at driver creation time, if one is not 
    passed in.
    Revision 1.12  2009/03/31 19:16:02Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.11  2009/02/03 05:33:38Z  keithg
    Removed asserts and added error handling.  Moved debug messages
    to debug level two to reduce spam.
    Revision 1.10  2008/03/24 17:04:02  Garyp
    Modified FfxDriverCreate() to take a driver number rather than a name.
    Modified FfxDriverHandle() to take a driver number parameter.
    Revision 1.9  2008/01/13 07:26:26Z  keithg
    Function header updates to support autodoc.
    Revision 1.8  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2006/10/04 00:32:55Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.6  2006/05/28 22:34:26Z  Garyp
    Tweaked a debug level.
    Revision 1.5  2006/05/07 23:42:06Z  Garyp
    Added FfxDriverHandle().
    Revision 1.4  2006/02/13 09:02:26Z  Garyp
    Updated debugging code.
    Revision 1.3  2006/02/12 01:15:08Z  Garyp
    Updated to call FfxHookDriverCreate/Destroy().
    Revision 1.2  2006/02/10 07:56:19Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.1  2005/10/02 02:10:56Z  Pauli
    Initial revision
    Revision 1.11  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.10  2004/11/19 20:23:48Z  GaryP
    Modified to save a global pointer to the FFXDRIVERINFO structure.
    Revision 1.9  2004/08/15 04:16:39Z  GaryP
    Minor function renaming exersize
    Revision 1.8  2004/07/21 21:18:47Z  GaryP
    Added debug code.
    Revision 1.7  2003/12/05 10:18:46Z  garys
    Merge from FlashFXMT
    Revision 1.4.1.3  2003/12/05 10:18:46  garyp
    Moved the oemmount/unmount logic to drvdev.c.
    Revision 1.4.1.2  2003/11/01 02:04:58Z  garyp
    Re-checked into variant sandbox.
    Revision 1.5  2003/11/01 02:04:58Z  garyp
    Reworked the driver initialization process to work with FFX/MT needs.
    Revision 1.4  2003/08/07 23:03:54Z  garyp
    Documentation updates.
    Revision 1.3  2003/08/05 23:30:16Z  garyp
    Updated the documentation.
    Revision 1.2  2003/05/14 00:16:10Z  garyp
    Fixed a faulty diagnostic message.
    Revision 1.1  2003/04/15 17:49:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>

static FFXDRIVERINFO *pFFXGlobalFDI = NULL;


/*-------------------------------------------------------------------
    Public: FfxDriverCreate()

    Create a FlashFX driver instance.  Only one driver instance is
    supported at this time.

    Parameters:
        nDriverNum - The driver instance number (relative to 1).

    Return Value:
        Returns a pointer to the FFXDRIVERINFO structure if
        successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDRIVERINFO *FfxDriverCreate(
    unsigned            nDriverNum,
    DCLINSTANCEHANDLE   hDclInst)
{
    DCLINSTANCEHANDLE   hLocalDclInst = NULL;
    FFXDRIVERINFO      *pFDI;
    D_BOOL              fDriverLocked = FALSE;
    char                szDriverName[8];

    /*  Ideally we should <require> that the DCL Instance handle already
        be set upon entry into this function.  However, because this
        affects so many ports, for now we will automatically initialize
        it if necessary.  We must keep track of the fact that we did the
        init, so at destroy time we know whether we should destroy the
        instance.
    */
    if(!hDclInst)
    {
        /*  Create a DCL Instance before doing <anything> else.  This will
            initialize the default services, as defined in the project.
        */
        if(DclInstanceCreate(0, DCLFLAG_DRIVER, &hLocalDclInst) != DCLSTAT_SUCCESS)
            return NULL;

        if(!hLocalDclInst)
            return NULL;

        hDclInst = hLocalDclInst;
    }

    DclAssert(nDriverNum < 99);
    DclSNPrintf(szDriverName, sizeof(szDriverName), "FXDRV%u", nDriverNum);

    FFXPRINTF(1, ("FfxDriverCreate() DriverName='%s' hDclInst=%P\n", szDriverName, hDclInst));

    DclAssert(pFFXGlobalFDI == NULL);

    pFDI = DclMemAllocZero(sizeof *pFDI);
    if(!pFDI)
    {
        FFXPRINTF(1, ("Unable to allocate memory\n"));

        DclError();

        goto InitFailure;
    }

    pFDI->hDclInst = hDclInst;
    if(hLocalDclInst)
        pFDI->fLocalDcl = TRUE;

  #if D_DEBUG
    DclMemCpy(pFDI->acSignature, FFX_DRIVER_SIGNATURE, sizeof pFDI->acSignature);
  #endif

    pFDI->pMutex = DclMutexCreate(szDriverName);
    if(!pFDI->pMutex)
    {
        FFXPRINTF(1, ("Mutex creation failed\n"));

        DclError();

        goto InitFailure;
    }

    if(!FfxDriverLock(pFDI))
        goto InitFailure;

    fDriverLocked = TRUE;

    if(!FfxHookDriverCreate(pFDI))
        goto InitFailure;

    FfxDriverUnlock(pFDI);

    FFXPRINTF(2, ("FfxDriverCreate() returning %P\n", pFDI));

    pFFXGlobalFDI = pFDI;

    return pFDI;

  InitFailure:
    if(fDriverLocked)
        FfxDriverUnlock(pFDI);

    if(pFDI->pMutex)
        DclMutexDestroy(pFDI->pMutex);

    if(pFDI)
        DclMemFree(pFDI);

    if(hLocalDclInst)
        DclInstanceDestroy(hLocalDclInst);

    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxDriverDestroy()

    Destroy a FlashFX driver instance.

    Parameters:
        pFDI - A pointer to the FFXDRIVERINFO structure.

    Return Value:
        Returns an FFXSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverDestroy(
    FFXDRIVERINFO      *pFDI)
{
    DCLINSTANCEHANDLE   hLocalDclInst = NULL;
    FFXSTATUS           ffxStat;

    FFXPRINTF(2, ("FfxDriverDestroy() pFDI=%P\n", pFDI));

    /*  Fail if the driver is already destroyed or the given
        pointer is not ours.
    */
    if(!pFFXGlobalFDI)
    {
        FFXPRINTF(1, ("Warning: FfxDriverDestroy() called when the driver is already destroyed.\n"));
        return FFXSTAT_BADPARAMETER;
    }

    if(pFDI != pFFXGlobalFDI)
    {
        FFXPRINTF(1, ("Warning: FfxDriverDestroy() given an invalid handle.\n"));
        return FFXSTAT_BADPARAMETER;
    }

    FfxHookDriverDestroy(pFDI);

    if(pFDI->fInFFX)
        FfxDriverUnlock(pFDI);

    DclMutexDestroy(pFDI->pMutex);

    /*  We must destroy the DCL Instance if we were the ones to create
        it.  Use a temporary variable to ensure we are releasing
        resources in reverse order of allocation/creation.
    */
    if(pFDI->fLocalDcl)
        hLocalDclInst = pFDI->hDclInst;

    ffxStat = DclMemFree(pFDI);
    DclAssert(ffxStat == FFXSTAT_SUCCESS);

    pFFXGlobalFDI = NULL;

    if(hLocalDclInst)
    {
        ffxStat = DclInstanceDestroy(hLocalDclInst);
        if(ffxStat == DCLSTAT_INST_USAGECOUNTREDUCED)
            ffxStat = FFXSTAT_SUCCESS;
        
        DclAssert(ffxStat == FFXSTAT_SUCCESS);
    }

    FFXPRINTF(1, ("FfxDriverDestroy() returning %lX\n", ffxStat));

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxDriverHandle()

    Get a handle for the specified FlashFX driver instance number.

    Parameters:
        nDriverNum - The Driver number to use (relative to 1).

    Return Value:
        Returns a pointer to the FFXDRIVERINFO structure if
        successful, otherwise NULL.
-------------------------------------------------------------------*/
FFXDRIVERINFO *FfxDriverHandle(
    unsigned    nDriverNum)
{
    FFXPRINTF(2, ("FfxDriverHandle(%u)\n", nDriverNum));

    if(nDriverNum != 1)
        return NULL;

    return pFFXGlobalFDI;
}



