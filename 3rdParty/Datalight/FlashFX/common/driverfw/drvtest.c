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

    This module contains functions for implementing the auto-test
    funtionality in the Driver Framework.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvtest.c $
    Revision 1.16  2010/01/07 23:55:07Z  garyp
    Split out the DCL test so it can be separately invoked at the proper
    point in time.  Updated to take a DCL instance handle, or query for
    it if necessary.
    Revision 1.15  2009/11/06 21:00:44Z  garyp
    Updated so VBFTEST and MTSTRESS are conditioned out if allocator
    support is not included.
    Revision 1.14  2009/07/21 21:28:11Z  garyp
    Merged from the v4.0 branch.  Eliminated a deprecated header.  Removed
    a spurious structure member initialization.
    Revision 1.13  2009/03/31 19:23:03Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.12  2009/03/24 20:51:40Z  thomd
    Correct dtp within FFX_DCLTESTPARAMS condition
    Revision 1.11  2009/02/09 18:53:17Z  thomd
    Corrected affected members of FFXTOOLPARAMS
    Revision 1.10  2008/03/25 19:46:49Z  Garyp
    Minor datatype changes.
    Revision 1.9  2008/01/13 07:26:35Z  keithg
    Function header updates to support autodoc.
    Revision 1.8  2007/11/07 17:35:10Z  pauli
    Corrected type usage.
    Revision 1.7  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/09/14 21:52:32Z  pauli
    Added descriptive error messages and now cause the system to halt if a unit
    test fails.
    Revision 1.5  2007/09/13 21:33:47Z  pauli
    Resolved Bug 1426: The disk number is now added to the parameter string
    before calling FfxStressMT().
    Revision 1.4  2006/10/03 23:50:31Z  Garyp
    Updated to use the new style printf macros and functions.
    Revision 1.3  2006/02/15 05:51:37Z  Garyp
    Fixed to build properly.
    Revision 1.2  2006/02/09 01:49:03Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.1  2005/10/02 02:10:56Z  Pauli
    Initial revision
    Revision 1.4  2005/08/21 10:40:17Z  garyp
    Documentation update.
    Revision 1.3  2005/08/18 02:01:57Z  garyp
    Updated to build cleanly when some tests are disabled.
    Revision 1.2  2005/08/15 04:08:53Z  garyp
    Added support for auto-testing DCL.
    Revision 1.1  2004/12/30 19:32:42Z  pauli
    Initial revision
    Revision 1.5  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.4  2004/11/19 20:23:51Z  GaryP
    Updated to use refactored header files.
    Revision 1.3  2004/09/29 02:24:27Z  GaryP
    General cleanup of configuration settings in preparation for FFX/Pro release.
    Revision 1.2  2004/07/20 00:11:56Z  GaryP
    Minor include file updates.
    Revision 1.1  2004/04/28 02:09:00Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>

#if FFXCONF_DRIVERAUTOTEST

#include <dltools.h>
#include <fxtools.h>


/*-------------------------------------------------------------------
    Public: FfxDriverUnitTest()

    Invoke the generic device driver framework logic for running the
    FlashFX unit tests from a FlashFX driver.  This function is only 
    compiled if FFXCONF_DRIVERAUTOTEST is set to TRUE and one or more
    of the FFX_FMSLTESTPARAMS, FFX_VBFTESTPARAMS, or FFX_MTTESTPARAMS,
    sybole are defined symbol (typically done in ffxconf.h).  See the
    individual tests for a description of the options.

    Parameters:
        hDclInst    - The DCL instance handle.
        hDisk       - The FFXDISKHANDLE to use

    Return Value:
        Returns an FFXSTATUS code indicating the results.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverUnitTest(
    DCLINSTANCEHANDLE   hDclInst,
    FFXDISKHANDLE       hDisk)
{
  #if defined(FFX_FMSLTESTPARAMS) || (FFXCONF_ALLOCATORSUPPORT && (defined(FFX_VBFTESTPARAMS) || defined(FFX_MTTESTPARAMS)))
    D_INT16             iResult;
    FFXTOOLPARAMS       ftp = {{0}};

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
        {
            DclPrintf("DCL Instance not yet created... aborting test\n");
            return DCLSTAT_INST_NOTINITIALIZED;
        }
    }
    
    ftp.dtp.hDclInst    = hDclInst;
    ftp.nDeviceNum      = (*hDisk)->Conf.DiskSettings.nDevNum;
    ftp.nDiskNum        = (*hDisk)->Conf.nDiskNum;
  #endif

  #ifdef FFX_FMSLTESTPARAMS
    {
        ftp.dtp.pszCmdName = "FfxTestFMSL";
        ftp.dtp.pszCmdLine = FFX_FMSLTESTPARAMS;

        iResult = FfxTestFMSL(&ftp);
        if(iResult)
        {
            DclPrintf("FFX: FMSL unit test failed, Error Code = %D\n", iResult);
            DclError();
            return DCLSTAT_CURRENTLINE;
        }
    }
  #endif

  #if FFXCONF_ALLOCATORSUPPORT
  #ifdef FFX_VBFTESTPARAMS
    {

        ftp.dtp.pszCmdName = "FfxTestVBF";
        ftp.dtp.pszCmdLine = FFX_VBFTESTPARAMS;

        iResult = FfxTestVBF(&ftp);
        if(iResult)
        {
            DclPrintf("FFX: VBF unit test failed, Error Code = %D\n", iResult);
            DclError();
            return DCLSTAT_CURRENTLINE;
        }
    }
  #endif

  #ifdef FFX_MTTESTPARAMS
    {
        /*  The casts involving uLen are required due to inconsistent types
            among the various DCL API used, see bug 1438.  The length of the
            string is expected to be considerably shorter than the maximum
            length that can be represented with an int.
        */
        char       *pszParams;
        size_t      nLen =  DclStrLen(FFX_MTTESTPARAMS) + 9;

        DclAssert(nLen <= D_INT16_MAX);

        /*  Allocate a string long enough for the parameters.
        */
        pszParams = DclMemAllocZero((D_UINT32)nLen);
        if(!pszParams)
        {
            DclPrintf("FFX: Failed to allocate memory for test parameters\n");
            DclError();
            return DCLSTAT_CURRENTLINE;
        }

        /*  Format the string to include the disk number and the parameters.
        */
        DclAssert(ftp.nDiskNum < 100);
        DclSNPrintf(pszParams, (int)nLen, "/disk%u %s", ftp.nDiskNum, FFX_MTTESTPARAMS);

        ftp.dtp.pszCmdName = "FfxStressMT";
        ftp.dtp.pszCmdLine = pszParams;

        iResult = FfxStressMT(&ftp);

        DclMemFree(pszParams);

        if(iResult)
        {
            DclPrintf("FFX: MTSTRESS test failed, Error Code = %D\n", iResult);
            DclError();
            return DCLSTAT_CURRENTLINE;
        }
    }
  #endif
  #endif

    /*  In the event that no tests are defined...
    */
    (void)hDclInst;
    (void)hDisk;
    
    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxDriverUnitTestDCL()

    Invoke the generic device driver framework logic for running
    the DCL unit test from a FlashFX driver.  This function is only 
    compiled if FFXCONF_DRIVERAUTOTEST is set to TRUE and the test
    is only run if FFX_DCLTESTPARAMS is a defined symbol (typically
    done in ffxconf.h).  See the individual tests for a description
    of the options.

    Parameters:
        hDclInst    - The DCL instance handle.

    Return Value:
        Returns a DCLSTATUS code indicating the results.
-------------------------------------------------------------------*/
DCLSTATUS FfxDriverUnitTestDCL(
    DCLINSTANCEHANDLE   hDclInst)
{
  #ifdef FFX_DCLTESTPARAMS
    DCLTOOLPARAMS       dtp = {0};
    DCLSTATUS           dclStat;

    if(!hDclInst)
    {
        hDclInst = DclInstanceHandle(0);
        if(!hDclInst)
        {
            DclPrintf("DCL Instance not yet created... aborting test\n");
            return DCLSTAT_INST_NOTINITIALIZED;
        }
    }

    dtp.hDclInst    = hDclInst;
    dtp.pszCmdName  = "FfxTestDCL";
    dtp.pszCmdLine  = FFX_DCLTESTPARAMS;

    dclStat = DclTestMain(&dtp);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("FFX: DCL unit test failed, Error Code = %lX\n", dclStat);
        DclError();
    }

    return dclStat;
    
  #else  

    (void)hDclInst;
    return DCLSTAT_SUCCESS;
  
  #endif
}





#endif  /* FFXCONF_DRIVERAUTOTEST */


