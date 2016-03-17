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
  jurisdictions.  Patents may be pending.

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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlsys.c $
    Revision 1.11  2010/04/18 21:12:17Z  garyp
    Did some refactoring of the DCL system info functions to make them
    more useful.
    Revision 1.10  2007/12/18 03:51:04Z  brandont
    Updated function headers.
    Revision 1.9  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.8  2007/08/03 01:02:27Z  garyp
    Modified version numbers to be 32-bit values
    Revision 1.7  2007/04/03 19:18:21Z  Garyp
    Added support for mutex tracking.
    Revision 1.6  2006/12/16 21:34:34Z  Garyp
    Added more compile time project information.
    Revision 1.5  2006/12/11 22:04:15Z  Garyp
    Updated to support DCL_OSFEATURE_CONSOLEINPUT.
    Revision 1.4  2006/12/08 00:05:19Z  Garyp
    Updated to work with the D_CPUTYPE value which is a string rather than
    a number.
    Revision 1.3  2006/09/16 19:48:14Z  Garyp
    Modified to accommodate build numbers which are now strings rather than
    numbers.
    Revision 1.2  2006/08/18 19:38:23Z  Garyp
    Added support for the processor alignment boundary.
    Revision 1.1  2006/01/02 02:37:02Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlstats.h>
#include <dlver.h>


/*-------------------------------------------------------------------
    Public: DclSystemInfoQuery()

    Fill the supplied DCLSYSTEMINFO structure.

    Parameters:
        pDSS     - A pointer to the DCLSYSTEMINFO structure to fill.
                   On entry, the uStrucLen field must be set to the
                   size of the structure.

    Return Value:
        Returns a DCLSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
DCLSTATUS DclSystemInfoQuery(
    DCLSYSTEMINFO  *pDSS)
{
    DclAssertWritePtr(pDSS, sizeof(*pDSS));

    if(pDSS->nStrucLen != sizeof(*pDSS))
        return DCLSTAT_BADSTRUCLEN;

    DclMemSet(pDSS, 0, sizeof(*pDSS));

    pDSS->nStrucLen = sizeof(*pDSS);

    DclStrNCpy(pDSS->szProductName, D_PRODUCTNAME, sizeof(pDSS->szProductName)-1);
    DclStrNCpy(pDSS->szProductBuild, (char *)DCLSTRINGIZE(D_PRODUCTBUILD), sizeof(pDSS->szProductBuild)-1);
    DclStrNCpy(pDSS->szSubProductBuild, DCLBUILDNUM, sizeof(pDSS->szSubProductBuild)-1);
    DclStrNCpy(pDSS->szOSName, DCL_OSNAME, sizeof(pDSS->szOSName)-1);
    DclStrNCpy(pDSS->szOSVer, (char *)DCLSTRINGIZE(D_OSVER), sizeof(pDSS->szOSVer)-1);
    DclStrNCpy(pDSS->szCPUType, (char *)DCLSTRINGIZE(D_CPUTYPE), sizeof(pDSS->szCPUType)-1);

    pDSS->ulProductVer              = D_PRODUCTVER;
    pDSS->nProductNum               = D_PRODUCTNUM;
    pDSS->ulSubProductVer           = VERSIONVAL;
    pDSS->nSubProductNum            = PRODUCTNUM_DCL;

    pDSS->nDebugLevel               = D_DEBUG;
    pDSS->nToolSetNum               = D_TOOLSETNUM;
    pDSS->nAlignment                = DCL_ALIGNSIZE;
    pDSS->nProcessorAlignment       = DCL_CPU_ALIGNSIZE;
    pDSS->ulDebugTraceMask          = DCLTRACEMASK;

    pDSS->fTraceAutoEnable          = DCLTRACE_AUTOENABLE;
    pDSS->fProfiler                 = DCLCONF_PROFILERENABLED;
    pDSS->fHighResTimeStamp         = DCLCONF_HIGHRESTIMESTAMP;
    pDSS->fOutputEnabled            = DCLCONF_OUTPUT_ENABLED;

  #if DCLCONF_PROFILERENABLED
    pDSS->fProfilerAutoEnable       = DCLPROF_AUTOENABLE;
  #endif

    pDSS->fMemoryTracking           = DCLCONF_MEMORYTRACKING;
    pDSS->fMutexTracking            = DCLCONF_MUTEXTRACKING;
    pDSS->fSemaphoreTracking        = DCLCONF_SEMAPHORETRACKING;

    pDSS->fOSFeatureUnicode         = DCL_OSFEATURE_UNICODE;
    pDSS->fOSFeatureThreads         = DCL_OSFEATURE_THREADS;
    pDSS->fOSFeatureConsoleInput    = DCL_OSFEATURE_CONSOLEINPUT;

    pDSS->fBigEndian                = DCL_BIG_ENDIAN;
    pDSS->fNative64BitSupport       = DCL_NATIVE_64BIT_SUPPORT;

    return DCLSTAT_SUCCESS;
}

