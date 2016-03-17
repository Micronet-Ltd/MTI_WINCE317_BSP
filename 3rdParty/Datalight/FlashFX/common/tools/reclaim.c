/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module implements a utility function to cause Disk compactions to 
    occur, and allows manipulation of the compaction settings.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: reclaim.c $
    Revision 1.13  2011/04/24 20:06:42Z  garyp
    Cleaned up the documentation and error return values.
    Revision 1.12  2010/07/13 00:03:46Z  garyp
    Cleaned up the syntax display.
    Revision 1.11  2009/07/21 22:37:44Z  garyp
    Merged from the v4.0 branch.  Added support for compaction suspend and resume.
    Revision 1.10  2009/04/01 15:30:48Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.9  2009/03/09 19:44:17Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.8  2009/02/09 15:55:42Z  thomd
    Removed unused variable
    Revision 1.7  2009/02/09 01:40:17Z  garyp
    Merged from the v4.0 branch.  Major cleanup.  Added the /ALL and /NONAGG
    options.  Use the updated FFXTOOPARAMS structure.  Updated to always
    build, even if the allocator is disabled.
    Revision 1.6  2008/02/06 21:36:52Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.5  2008/01/13 07:27:06Z  keithg
    Function header updates to support autodoc.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/02/12 18:48:22Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a
    single device/disk at a time.
    Revision 1.2  2006/01/31 07:18:47Z  Garyp
    Updated to use the new FML interface.
    Revision 1.1  2005/10/20 02:35:46Z  Pauli
    Initial revision
    Revision 1.2  2005/10/20 03:35:46Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/02 03:24:32Z  Garyp
    Initial revision
    Revision 1.2  2005/09/18 05:55:24Z  garyp
    Renamed vbfcompact() to FfxVbfCompact() and added FfxVbfCompactIfIdle().
    Revision 1.1  2005/08/03 19:29:32Z  pauli
    Initial revision
    Revision 1.11  2004/12/30 23:43:19Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.10  2004/09/23 08:44:00Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.9  2004/09/15 23:22:41Z  garys
    fAggressive parameter to FfxVbfCompact() to support background G.C.
    Revision 1.8  2004/01/14 03:27:28  garys
    Merge from FlashFXMT
    Revision 1.5.1.4  2004/01/14 03:27:28  garyp
    Include changes.
    Revision 1.5.1.3  2004/01/03 18:10:10Z  garyp
    Changed to use FfxStrICmp().
    Revision 1.5.1.2  2003/11/03 04:27:16Z  garyp
    Re-checked into variant sandbox.
    Revision 1.6  2003/11/03 04:27:16Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.5  2003/04/08 21:54:46Z  garyp
    Updated to use FFX internal string routines.  Eliminated use of __D_FAR
    where possible.
    Revision 1.4  2003/03/27 23:30:20Z  garyp
    Fixed some command-line parsing problems.
    Revision 1.3  2003/03/23 07:17:36Z  garyp
    Eliminated all OESL specific code, along with any code related to the
    external API or oem/vbf initialization.  Revamped to use the new TOOLS and
    TESTS framework to deal with these issues.
    02/16/01 MJM Return error if vbfmount fails.
    11/03/99 TWQ Restricted number of garbage collections to 1 if don't have
                 all 3 arguments.
    02/09/99 HDS Flagged all printfs to make them easier to find for
                 modification.
    12/10/97 HDS Fixed command names.
    11/20/97 PKG Removed use of .pt files to make it directory independant and
                 re-ordered the function calls to be self proto-typed.  This
                 allows these source file to be compiled from anywhere within
                 the project
    11/19/97 HDS Changed include file search to use predefined path.
    08/26/97 PKG Now uses new Datalight specific types
    07/28/97 PKG Reorganized source into single, smaller, simpler modules and
                 removed the library files.
    06/12/96 DM  The CleanDisk() routine was pulled out of this module and
                 placed into a library.
    03/10/97 DM  Created
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>

static unsigned CleanDisk(VBFHANDLE hVBF, D_UINT32 ulCount, D_BOOL fAggressive);
static void     ShowUsage(FFXTOOLPARAMS *pTP);

#endif


/*-------------------------------------------------------------------
    Protected: FfxReclaim()

    Compact a Disk, or change compaction settings.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use.

    Return Value:
        Returns 0 if successful, -1 if there was nothing to do, and
        non-zero on error.
-------------------------------------------------------------------*/
D_INT16 FfxReclaim(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT
    #define         ARGBUFFLEN   (128)
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;
    D_UINT16        uArgNum;
    D_UINT32        ulCount = 1;
    VBFHANDLE       hVBF;
    D_BOOL          fAggressive = TRUE;
    unsigned        nNumCompacts;
  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    D_BOOL          fSuspend = FALSE;
    D_BOOL          fResume = FALSE;
  #endif

    DclPrintf("\nFlashFX Compact\n");
    FfxSignOn(FALSE);

    uArgCount = DclArgCount(pTP->dtp.pszCmdLine);

    for(uArgNum = 1; uArgNum <= uArgCount; uArgNum++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, uArgNum, ARGBUFFLEN, achArgBuff))
        {
            DclError();
            return DCLERRORLEVEL_INTERNALERROR;
        }

        if(DclStrICmp(achArgBuff, "/?") == 0)
        {
            ShowUsage(pTP);
            return DCLERRORLEVEL_HELPREQUEST;
        }

        if(DclStrICmp(achArgBuff, "/All") == 0)
        {
            ulCount = D_UINT32_MAX;
            continue;
        }

      #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
        if(DclStrICmp(achArgBuff, "/Suspend") == 0)
        {
            fSuspend = TRUE;
            continue;
        }

        if(DclStrICmp(achArgBuff, "/Resume") == 0)
        {
            fResume = TRUE;
            continue;
        }
      #endif

        if(DclStrICmp(achArgBuff, "/NonAgg") == 0)
        {
            fAggressive = FALSE;
            continue;
        }

        if(ulCount == 1)
        {
            ulCount = (D_UINT32)DclAtoL(achArgBuff);
            if(ulCount)
                continue;
        }

        DclPrintf("Invalid argument '%s'\n", achArgBuff);
        return DCLERRORLEVEL_BADSYNTAX;
    }

    hVBF = FfxVbfHandle(pTP->nDiskNum);
    if(!hVBF)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return DCLERRORLEVEL_INVALIDDRIVE;
    }

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    if(fSuspend)
    {
        D_UINT32 ulCount = FfxVbfCompactIdleSuspend(hVBF);

        if(ulCount != D_UINT32_MAX)
            DclPrintf("Idle-time compaction suspend count incremented to %lU\n", ulCount);
        else
            DclPrintf("Unable to increment the suspend count\n");

        return 0;
    }

    if(fResume)
    {
        D_UINT32 ulCount = FfxVbfCompactIdleResume(hVBF);

        if(ulCount != D_UINT32_MAX)
            DclPrintf("Idle-time compaction suspend count decremented to %lU\n", ulCount);
        else
            DclPrintf("Unable to decrement the suspend count\n");

        return 0;
    }
  #endif

    /*  Clean the entire disk by forcing compactions
    */
    nNumCompacts = CleanDisk(hVBF, ulCount, fAggressive);
    if(nNumCompacts)
        return 0;
    else
        return -1;

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ShowUsage()

    Parameters:

    Return Value:  
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nThis command compacts a Disk to recover discarded space.  It also allows\n");
    DclPrintf("compaction related settings to be changed.\n\n");
    DclPrintf("Syntax: %s Disk [Count] [Options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where:\n");
    DclPrintf("  Disk          The FlashFX Disk designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  Count         Specifies the maximum number of compactions to perform.\n");
    DclPrintf("                Defaults to 1 if not specified and \"/All\" is not used.\n");
    DclPrintf("And [Options] are:\n");
    DclPrintf("  /?            This help information.\n");
    DclPrintf("  /All          Compact the Disk until there is nothing more which can be\n");
    DclPrintf("                recovered.  Supersedes the \"Count\" value.\n");
    DclPrintf("  /NonAgg       Specifies that non-aggressive compactions are to be performed,\n");
    DclPrintf("                rather than the default aggressive compactions.\n");
  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    DclPrintf("  /Suspend      Suspend background compaction operations.\n");
    DclPrintf("  /Resume       Resume background compaction operations.\n");
  #endif
    DclPrintf("\nReturns 0 if successful, or -1 if there was nothing to compact.  Any other\n");
    DclPrintf("value indicates an error condition.\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: CleanDisk()

    Clean the Disk by performing up to the specified number of 
    compactions.

    Parameters:
       hVBF        - A handle denoting the VBF instance to use
       ulCount     - The maximum number of compactions to perform.
       fAggressive - A flag indicating whether aggressive
                     compactions should be used or not.

    Return Value:
        Returns the number of compactions performed.
-------------------------------------------------------------------*/
static unsigned CleanDisk(
    VBFHANDLE   hVBF,
    D_UINT32    ulCount,
    D_BOOL      fAggressive)
{
    unsigned    nNumCompactions = 0;
    D_UINT32    ulTotalPages = 0;

    DclAssert(hVBF);
    DclAssert(ulCount);

    DclPrintf("Compacting -- one moment please...\n");

    while(ulCount--)
    {
        FFXIOSTATUS ioStat;

        /*  If we cannot find another dirty unit, then we are done.
        */
        ioStat = FfxVbfCompact(hVBF, fAggressive);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            break;

        DclPrintf("Compacting unit freed %lU pages\n", ioStat.ulCount);

        /*  Count the number of compactions
        */
        nNumCompactions++;
        ulTotalPages += ioStat.ulCount;
    }

    DclPrintf("\nCompacted %u units recovering %lU pages.\n", nNumCompactions, ulTotalPages);

    return nNumCompactions;
}


#endif  /* FFXCONF_VBFSUPPORT */



