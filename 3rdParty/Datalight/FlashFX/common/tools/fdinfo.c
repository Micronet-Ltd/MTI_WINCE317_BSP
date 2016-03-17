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

    This module implements the "FXINFO" functionality, which reports driver-
    compile time settings, statistics, device, and disk information.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fdinfo.c $
    Revision 1.61  2010/04/18 23:20:23Z  garyp
    Updated to use some refactored DCL system info functionality.
    Revision 1.60  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.59  2009/12/02 21:23:10Z  garyp
    Refactored large portions of the display functionality into the module
    fxinfohelp.c.
    Revision 1.58  2009/11/19 22:18:07Z  garyp
    Updated to display some bitflags by name to make the output more readable.
    Revision 1.57  2009/11/05 03:49:57Z  garyp
    Minor datatype updates -- no functional changes.
    Revision 1.56  2009/07/30 02:55:10Z  garyp
    Merged from the v4.0 branch.  Updated the detailed unit display to identify
    the active unit for each region, and to summarize recoverable space in 
    terms of that in active an inactive units.  Added a region dump capability
    to the /Usage option.  Updated to display the composite unit dirtiness.  
    Updated to display the Device chip ID bytes, if available.  Eliminated the
    undocumented /X switch and folded that functionality into the documented
    /V switch.  Display more detailed information about Disks which span
    Devices.  Display emulated geometry separately.  Updated to use some renamed
    status codes.  Updated to display the number of Devices a Disk uses.  Added
    OTP support.  Added a wrapper around the DclScaleKB() function.  Moved the
    ScaleKB() function into DCL.  Updated to display the chip count and the high
    reserved space in the Device info.
    Revision 1.55  2009/04/09 02:58:27Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.54  2009/04/01 14:40:35Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.53  2009/03/25 01:10:41Z  keithg
    Added unconditional initialization to placate picky compilers.
    Revision 1.52  2009/03/09 19:38:23Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.51  2009/02/09 01:38:06Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FFXTOOLPARAMS
    structure.
    Revision 1.50  2008/06/24 18:15:26Z  johnb
    [BUG 2016] Backed out previous change and modified failure text.
    Revision 1.49  2008/06/24 16:51:27Z  johnb
    [Bug 2016] Updated code in ReportFlashFXConfigInfo() to check return
    code to determine if specific interface is enabled and to display
    an appropriate message.
    Revision 1.48  2008/06/02 16:45:26Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.47  2008/05/13 16:03:17Z  thomd
    Correct bug in display for large numbers of units
    Revision 1.46  2008/05/08 15:56:06Z  garyp
    Fixed the FML stats query to use the proper interface.
    Revision 1.45  2008/05/06 16:05:07Z  garyp
    Updated to work with the new format of the FML block statistics.
    Revision 1.44  2008/05/03 20:22:53Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.43  2008/03/24 18:29:22Z  Garyp
    Updated to compile when allocator support is disabled.
    Revision 1.42  2008/01/17 03:29:13Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.41  2008/01/13 07:17:31Z  keithg
    Function header updates to support autodoc.
    Revision 1.40  2008/01/08 00:51:37Z  Garyp
    Updated some messages to be consistent -- no functional changes.
    Revision 1.39  2008/01/06 16:21:57Z  garyp
    Fixed to free resources in reverse order of allocation.
    Revision 1.38  2007/12/14 23:06:25Z  Garyp
    Updated to support displaying FML erase counts.
    Revision 1.37  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.36  2007/10/30 22:09:14Z  Garyp
    Added an undocumented /X option to display extensive media usage
    information.
    Revision 1.35  2007/10/05 01:34:35Z  Garyp
    Fixed so the system stats always display, even if the /Stats flag is not
    used.
    Revision 1.34  2007/10/01 22:43:13Z  pauli
    Resolved Bug 355: Display the alignment requirements of the device.
    Revision 1.33  2007/09/11 20:16:40Z  pauli
    Resolved Bug 1116: refactored how large values are scaled up to MB or GB.
    Added scaling for displaying large page counts.  General cleanup to only
    display information relevant to the flash type.  Restructured the VBF
    output to provide additional information.
    Revision 1.32  2007/08/31 21:14:42Z  pauli
    Resolved Bug 1402: Changed byte counts in VBFUNITMETRICS to page counts.
    Revision 1.31  2007/08/29 21:14:46Z  thomd
    Changed dev and disk math to reduce overflow.  Display values in KB and
    parenthetically MB or GB.
    Revision 1.30  2007/08/02 18:19:35Z  timothyj
    Updated some output to be in units of KB and MB.
    Revision 1.29  2007/06/24 04:33:50Z  Garyp
    Modified so that dumping stats to PerfLog is independent from the standard
    stats display functions.
    Revision 1.28  2007/04/15 17:51:58Z  Garyp
    Updated to use the DclRatio() function to display percentages.  Updated
    to display BBM statistics, if available.
    Revision 1.27  2007/04/08 23:27:25Z  Garyp
    Updated so that stats are displayed only if the /Stats option is used.
    Changed so that media usage information is shown only if the /Usage
    option is used, and eliminated the /NoUsage flag.
    Revision 1.26  2007/04/08 00:10:46Z  Garyp
    Modified to display additional latency information and to make it
    conditional on the value of FFXCONF_LATENCYREDUCTIONENABLED.
    Revision 1.25  2007/04/06 03:16:33Z  Garyp
    Modified the use of boolean types so we build cleanly in CE.
    Revision 1.24  2007/04/06 01:44:18Z  Garyp
    Added a /PerfLog command line option.  Updated to use an instance data
    structure rather than passing a zillion arguments to the various internal
    functions.
    Revision 1.23  2007/04/02 02:03:21Z  Garyp
    Now use real % signs.
    Revision 1.22  2007/03/30 18:46:43Z  Garyp
    Updated to support mutex and read/write semaphore statistics.  Updated
    to display the device latency settings.
    Revision 1.21  2007/01/10 01:46:58Z  Garyp
    Minor function renaming exercise.
    Revision 1.20  2006/11/13 18:40:31Z  Garyp
    Updated to display the compaction model.
    Revision 1.19  2006/11/10 19:16:50Z  Garyp
    Added the /NoUsage command-line option.  Updated to use the new
    method of querying statistics.
    Revision 1.18  2006/10/27 00:26:19Z  Garyp
    Documentation cleanup.
    Revision 1.17  2006/10/16 19:31:14Z  Garyp
    Added support for new configuration flags.
    Revision 1.16  2006/10/13 01:22:27Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.15  2006/09/16 20:23:20Z  Garyp
    Improved error handling.
    Revision 1.14  2006/08/20 00:18:29Z  Garyp
    Tweaked the BBM overhead display.
    Revision 1.13  2006/08/07 03:16:50Z  Garyp
    Updated to display the BBM and VBF media overhead percentages.
    Revision 1.12  2006/06/19 16:25:29Z  Garyp
    Updated to report region tracking information.
    Revision 1.11  2006/05/28 23:44:02Z  Garyp
    Fixed a uninitialized buffer which was causing bogus configuration options
    to be displayed.  Minor output fixes.
    Revision 1.10  2006/05/19 00:44:14Z  Garyp
    Updated to display boot block and reserved space information.  Fixed the
    device type display to handle Sibley.
    Revision 1.9  2006/05/08 01:47:22Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.8  2006/03/07 02:39:12Z  Garyp
    Updated to work with FfxFmlDeviceInfo().  Fixed to work properly even
    if VBF is not loaded.
    Revision 1.7  2006/02/21 02:02:43Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.6  2006/02/15 19:23:27Z  Garyp
    Fixed a couple messages.
    Revision 1.5  2006/02/12 18:49:42Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a
    single device/disk at a time.
    Revision 1.4  2006/02/07 02:54:17Z  Garyp
    Updated to use the new FML interface.
    Revision 1.3  2006/01/02 06:57:26Z  Garyp
    Updated to use new "stats" functions.
    Revision 1.2  2005/12/17 19:30:16Z  garyp
    Removed some obsolete settings.
    Revision 1.1  2005/12/04 21:07:06Z  Pauli
    Initial revision
    Revision 1.5  2005/12/04 21:07:06Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.4  2005/11/13 04:51:33Z  Garyp
    Continued general cleanup of the output.
    Revision 1.3  2005/11/09 17:10:34Z  Garyp
    Cleaned up the output.  Added the /F and /R options.
    Revision 1.2  2005/10/20 03:29:47Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/06 06:50:56Z  Garyp
    Initial revision
    Revision 1.16  2004/12/30 17:32:44Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.15  2004/11/29 20:19:32Z  GaryP
    Added support for displaying compile-time configuration information.
    Revision 1.14  2004/02/07 23:32:04Z  garys
    Merge from FlashFXMT
    Revision 1.11.1.5  2004/02/07 23:32:04  garyp
    Cleaned up the output.
    Revision 1.11.1.4  2004/01/03 18:09:34Z  garyp
    Changed to use FfxStrICmp().
    Revision 1.11.1.3  2003/11/22 03:44:08Z  garyp
    Fixed to work properly if vbfgetunitinfo() fails.  Reformatted for
    readability.
    Revision 1.11.1.2  2003/11/03 04:47:36Z  garyp
    Re-checked into variant sandbox.
    Revision 1.12  2003/11/03 04:47:36Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.11  2003/06/10 23:58:25Z  billr
    Comment change only.
    Revision 1.10  2003/05/01 01:42:35Z  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h.  core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.
    Revision 1.9  2003/04/29 21:30:42Z  garyp
    Minor fixes to mollify the VxWorks/GNU compiler.
    Revision 1.8  2003/04/28 17:19:01Z  billr
    Correct reporting of erase count when spare units are unformatted.
    Revision 1.7  2003/04/25 00:59:04Z  billr
    Show correct region size.
    Revision 1.6  2003/04/23 23:06:56Z  billr
    Fix FXINFO for VBF4. Change the UnitInformation structure and add
    vbfgetpartitioninfo() to support this.
    Revision 1.5  2003/04/16 16:12:39Z  billr
    Merge from VBF4 branch.
    Revision 1.4  2003/04/08 21:52:28Z  garyp
    Updated to use FFX internal string routines.  Eliminated use of __D_FAR
    where possible.
    Revision 1.3  2003/03/23 03:14:16Z  garyp
    Eliminated all OESL specific code, along with any code related to
    the external API or oem/vbf initialization.  Revamped to use the new
    TOOLS and TESTS framework to deal with these issues.
    03/09/00 DE  Changed "no format" error message to say you probrably
                 WON'T have a VBF format on a RAM disk.
    06/29/00 HDS Changed polarity of allocation start block MSBit
                 (from client block MSBit) to maintain backward compatibility.
    06/28/00 HDS Changed method of determining number of logical units
                 from structure reference to header reference.
    03/30/00 HDS Increased size of working buffer to handle 256KB unit size.
    01/11/00 HDS Added constants and macros to utilize the MSB of the
                 allocation client block for expanding the erase zone size
                 to 256 KB.
    11/03/99 TWQ Updated for Linux
    02/09/99 HDS Flagged all printfs to make them easier to find.
    01/25/99 HDS Changed K to bytes in AND/NAND erase zone size display and
                 changed to prevent double display of AND/NAND erase zone size.
    11/19/98 PKG Made changes to clearly identify some information about the
                 location and length of the VBF partition
    11/16/98 HDS Made minor change to display information.
    10/28/98 HDS Changed display to show FXINFO instead of FDINFO.
    10/27/98 HDS Modified ReportMediaUsage to prevent a divide by zero.
    10/26/98 HDS Changed display format to make more readable and to include
                 option indicators.
    10/26/98 HDS Removed the "Estimated Disk Life Time Used" calculation and
                 display (didn't apply to all types).
    10/26/98 PKG Changed all allocation list related calculations to be in
                 terms of the allocation block size instead of the
                 VBF_BLOCK_SIZE.
    10/26/98 HDS Changed lpcScratchData to be defined in terms of
                 VBF_BLOCK_SIZE.
    08/10/98 HDS Changed the recoverable space computation to round up to
                 prevent reporting 0 when a small value exists.
    08/10/98 HDS Fixed the FXINFO tool to report the correct information for
                 the AND flash memory.
    12/10/97 HDS Fixed command names.
    12/08/97 HDS Corrected minor far pointer problems.
    11/20/97 PKG Removed use of .pt files to make it directory independant and
                 re-ordered the function calls to be self proto-typed.  This
                 allows this source file to be compiled from anywhere within
                 the project.
    11/20/97 HDS Made the inclusion of the main function and all printf
                 operations conditional.
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    09/09/97 PKG Re-ordered the include files so the VBF.H is included before
                 the vbfapi.pt file since it uses types defined there.
    08/26/97 PKG Now uses new Datalight specific types
    07/28/97 PKG Reorganized source into single, smaller, simpler modules and
                 removed the library files.
    06/17/97 PKG Made _sys functions to avoid linking in C libraries.
    05/29/97 PKG Added use of the DEV_MASK to isolate device types
    06/12/97 DM  The MediaCheck() routine was pulled out of this module and
                 added to a library.
    03/25/97 DM  Created
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <fxtools.h>
#include <fxinfohelp.h>
#include <fxconfig.h>
#include <fxstats.h>
#include <fxperflog.h>
#include <dlperflog.h>

#if FFXCONF_VBFSUPPORT
#include <vbf.h>
#endif

typedef struct
{
    FFXDISKHANDLE   hDisk;
    FFXFMLHANDLE    hFML;
    char            szPerfLogSuffix[PERFLOG_MAX_SUFFIX_LEN];
    unsigned        fVerbose      : 1;
    unsigned        fStats        : 1;
    unsigned        fReset        : 1;
    unsigned        fMediaUsage   : 1;
    unsigned        fPerfLog      : 1;
} FITESTINFO;

/*  Prototypes for internal routines
*/
static void         ShowUsage(FFXTOOLPARAMS *pTP);
static D_INT16      DisplayFlashFXInfo(FITESTINFO *pTI);
static void         ReportFlashFXConfigInfo(FITESTINFO *pTI);

#define ARGBUFFLEN           (128)
#define MAX_REGIONS          (512)
#define MAX_MUTEXES           (32)
#define MAX_SEMAPHORES         (8)
#define MAX_RWSEMAPHORES       (8)


/*-------------------------------------------------------------------
    Protected: FfxDiskInfo()

    This function displays Disk information.

    Parameters:
        pTP  - The FFXTOOLPARAMS structure to use

    Return Value:
        Returns 0 if successful, or a line number otherwise.
-------------------------------------------------------------------*/
D_INT16 FfxDiskInfo(
    FFXTOOLPARAMS  *pTP)
{
    char            achArgBuff[ARGBUFFLEN];
    FITESTINFO      ti = {0};       /* All fields zero'd */
    D_UINT16        uArgNum = 1;

    DclPrintf("\nFlashFX Information\n");
    FfxSignOn(FALSE);
    DclPrintf("\n");

     while(DclArgRetrieve(pTP->dtp.pszCmdLine, uArgNum, ARGBUFFLEN, achArgBuff))
     {

        if(DclStrICmp(achArgBuff, "/?") == 0)
        {
            ShowUsage(pTP);

            return 1;
        }

        if(DclStrICmp(achArgBuff, "/R") == 0)
        {
            ti.fReset = TRUE;

            uArgNum++;

            continue;
        }

        if(DclStrICmp(achArgBuff, "/V") == 0)
        {
            ti.fVerbose = TRUE;

            uArgNum++;

            continue;
        }

        if(DclStrICmp(achArgBuff, "/Stats") == 0)
        {
            ti.fStats = TRUE;

            uArgNum++;

            continue;
        }

        if(DclStrICmp(achArgBuff, "/Usage") == 0)
        {
            ti.fMediaUsage = TRUE;

            uArgNum++;

            continue;
        }

        if(DclStrNICmp(achArgBuff, "/PerfLog", 8) == 0)
        {
            ti.fPerfLog = TRUE;
            ti.fStats = TRUE;
            ti.fVerbose = TRUE;

            if(achArgBuff[8] == ':')
            {
                DclStrNCpy(ti.szPerfLogSuffix, &achArgBuff[9], sizeof(ti.szPerfLogSuffix));
                ti.szPerfLogSuffix[sizeof(ti.szPerfLogSuffix)-1] = 0;
            }

            uArgNum++;

            continue;
        }

        DclPrintf("%s: Unknown option '%s'\n", pTP->dtp.pszCmdName, achArgBuff);

        return 2;
    }

    ti.hDisk = FfxDriverDiskHandle(NULL, pTP->nDiskNum);
    if(!ti.hDisk)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    ti.hFML = FfxFmlHandle(pTP->nDiskNum);
    if(!ti.hFML)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    /*  Call media check. It will return 0 if there are no errors!
    */
    return DisplayFlashFXInfo(&ti);
}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    This function displays the command usage.

    Parameters:
        pTP - The FFXTOOLPARAMS structure to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nSyntax:  %s disk [options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where:\n");
    DclPrintf("           disk  The FlashFX Disk to use, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("         /Stats  Display statistics information where available.\n");
    DclPrintf("             /R  Reset statistics counts where possible.\n");
    DclPrintf("             /V  Display more detailed and Verbose information where possible.\n");
    DclPrintf("         /Usage  Display media usage summary information.  Use with the /V\n");
    DclPrintf("                 option to display detailed media usage information.  This\n");
    DclPrintf("                 option is valid only for Disks which are using an allocator.\n");
    DclPrintf(" /PerfLog[:sfx]  Output the statistics information in CSV form, in addition to\n");
    DclPrintf("                 the standard output.  This option implies the /V option and\n");
    DclPrintf("                 /Stats options.  The optional 'sfx' value is a text suffix to\n");
    DclPrintf("                 append to the build number in the PerfLog CSV output.\n");
    DclPrintf("Note that using the /Usage option will skew <any and all> statistics information\n");
    DclPrintf("which is being gathered.\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: DisplayFlashFXInfo()

    This function is used to call the functions that do the actual
    work, to retrieve the media info in the correct order.  This
    way a program can call just this routine instead of calling
    five others to get the media info.

    Parameters:
        pTI - A pointer to the FITESTINFO structure to use.

    Return Value:
        Zero if successful, or a line number otherwise
-------------------------------------------------------------------*/
static D_INT16 DisplayFlashFXInfo(
    FITESTINFO     *pTI)
{
    FFXSTATUS       ffxStat;
    FFXFMLINFO      FmlInfo;
  #if FFXCONF_VBFSUPPORT
    VBFHANDLE       hVBF;
  #endif

    ReportFlashFXConfigInfo(pTI);

    ffxStat = FfxFmlDiskInfo(pTI->hFML, &FmlInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
        return __LINE__;

    ffxStat = FfxDisplayDiskInfo(pTI->hFML, TRUE);
    if(ffxStat != FFXSTAT_SUCCESS)
        return __LINE__;

    DclPrintf("\n");

  #if FFXCONF_VBFSUPPORT
 
    hVBF = FfxVbfHandle(FmlInfo.nDiskNum);
    if(hVBF)
    {
        ffxStat = FfxDisplayVbfDiskInfo(hVBF, pTI->fVerbose);
        if(ffxStat != FFXSTAT_SUCCESS)
            return __LINE__;

        if(pTI->fMediaUsage)
        {
            DclPrintf("\n");
            
            ffxStat = FfxDisplayVbfMediaUsageInfo(hVBF, pTI->fVerbose);
            if(ffxStat != FFXSTAT_SUCCESS)
                return __LINE__;
        }

        DclPrintf("\n");
    }
    else
    {
        DclPrintf("No VBF information found.\n\n");
    }
    
  #else
  
    DclPrintf("The VBF allocator is not built into this driver.\n\n");
  
  #endif

    ffxStat = FfxDisplayEmulatedDiskInfo(pTI->hFML);
    if(ffxStat != FFXSTAT_SUCCESS)
        return __LINE__;

  #if DCLCONF_OUTPUT_ENABLED
    if(pTI->fStats)
    {
      #if FFXCONF_VBFSUPPORT
        VBFREGIONSTATS          RegionStats;
        VBFCOMPSTATS            VbfCompStats;
      #endif
        FFXFMLSTATS             FmlStats;
        DRIVERIOSTATS           DrvStats;
        FFXCOMPSTATS            CompStats;
        FFXBBMSTATS             BbmStats;
        FFXSTATUS               ffxStat;

        DclPrintf("\n");

        /*------------------------------
            Driver I/O Stats
        ------------------------------*/
        ffxStat = FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_DRIVERIO, &DrvStats, sizeof(DrvStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxDriverDiskIOStatsDisplay(&DrvStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteDiskIOStats(&DrvStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("Driver I/O statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying driver statistics\n\n", ffxStat);
            }
        }

        /*------------------------------
            Driver Compaction Stats
        ------------------------------*/
        ffxStat = FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_DRIVERCOMPACTION, &CompStats, sizeof(CompStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxDriverDiskCompStatsDisplay(&CompStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteDiskCompactionStats(&CompStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("Driver compaction statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying driver compaction statistics\n\n", ffxStat);
            }
        }


      #if FFXCONF_VBFSUPPORT
        /*------------------------------
            VBF Compaction Stats
        ------------------------------*/
        ffxStat = FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_VBFCOMPACTION, &VbfCompStats, sizeof(VbfCompStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxVbfCompactionStatsDisplay(&VbfCompStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteVbfCompactionStats(&VbfCompStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("VBF compaction statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying VBF compaction statistics\n\n", ffxStat);
            }
        }

        /*------------------------------
            VBF Region Stats
        ------------------------------*/
        DclMemSet(&RegionStats, 0, sizeof(RegionStats));

      #if MAX_REGIONS
        if(hVBF && pTI->fVerbose)
        {
            RegionStats.pMountData = DclMemAlloc(sizeof(*RegionStats.pMountData) * MAX_REGIONS);

            if(RegionStats.pMountData)
                RegionStats.nMountDataCount = MAX_REGIONS;
            else
                DclPrintf("Can't allocate memory to collect detailed region mount stats!\n\n");
        }
      #endif

        ffxStat = FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_VBFREGION, &RegionStats, sizeof(RegionStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxVbfRegionStatsDisplay(&RegionStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteVbfRegionStats(&RegionStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("VBF region statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying VBF region statistics\n\n", ffxStat);
            }
        }

        if(RegionStats.pMountData)
            DclMemFree(RegionStats.pMountData);
      #endif

        /*------------------------------
            FML I/O Stats
        ------------------------------*/
        DclMemSet(&FmlStats, 0, sizeof(FmlStats));

        FmlStats.pBlockStats =
            DclMemAlloc(sizeof(*FmlStats.pBlockStats) * FmlInfo.ulTotalBlocks);

        if(FmlStats.pBlockStats)
            FmlStats.ulBlockStatsCount = FmlInfo.ulTotalBlocks;
        else
            DclPrintf("Can't allocate enough memory to collect block statistics!\n\n");

        ffxStat = FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_FML, &FmlStats, sizeof(FmlStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxFmlStatsDisplay(&FmlStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteFmlIOStats(&FmlStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("FML Statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying FML statistics\n\n", ffxStat);
            }
        }

        if(FmlStats.pBlockStats)
            DclMemFree(FmlStats.pBlockStats);

        /*------------------------------
            BBM Stats
        ------------------------------*/
        ffxStat = FfxFmlParameterGet(pTI->hFML, FFXPARAM_STATS_BBM, &BbmStats, sizeof(BbmStats));
        if(ffxStat == FFXSTAT_SUCCESS)
        {
            FfxBbmStatsDisplay(&BbmStats, pTI->fVerbose);
            if(pTI->fPerfLog)
                FfxPerfLogWriteBbmStats(&BbmStats, NULL, NULL, pTI->szPerfLogSuffix);
        }
        else
        {
            if(pTI->fVerbose)
            {
                if(ffxStat == FFXSTAT_CATEGORYDISABLED)
                    DclPrintf("BBM Statistics are not enabled\n\n");
                else
                    DclPrintf("Error %lX querying BBM statistics\n\n", ffxStat);
            }
        }

        DclPrintf("'*' - Since last statistics reset\n\n");
    }
  #endif

    /*  Reset the stats if so instructed...
    */
    if(pTI->fReset)
        FfxDriverDiskParameterGet(pTI->hDisk, FFXPARAM_STATS_RESETALL, NULL, 0);

    return 0;
}


/*-------------------------------------------------------------------
    Local: ReportFlashFXConfigInfo()

    This function displays FlashFX configuration information.

    Parameters:
        pTI - A pointer to the FITESTINFO structure to use.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ReportFlashFXConfigInfo(
    FITESTINFO     *pTI)
{
    FFXCONFIGINFO   fci = {sizeof(fci)};
 
    fci.SysInfo.nStrucLen = sizeof(fci.SysInfo);
    fci.MemStats.uStrucLen = sizeof(fci.MemStats);

    fci.MutStats.nStrucLen = sizeof(fci.MutStats);
    fci.MutStats.pDMI = DclMemAlloc(sizeof(DCLMUTEXINFO) * MAX_MUTEXES);
    if(fci.MutStats.pDMI)
        fci.MutStats.nSupplied = MAX_MUTEXES;
    else
        fci.MutStats.nSupplied = 0;

    fci.SemStats.nStrucLen = sizeof(fci.SemStats);
    fci.SemStats.pDSI = DclMemAlloc(sizeof(DCLSEMAPHOREINFO) * MAX_SEMAPHORES);
    if(fci.SemStats.pDSI)
        fci.SemStats.nSupplied = MAX_SEMAPHORES;
    else
        fci.SemStats.nSupplied = 0;

    fci.RWSemStats.nStrucLen = sizeof(fci.RWSemStats);
    fci.RWSemStats.pRWI = DclMemAlloc(sizeof(DCLRDWRSEMAPHOREINFO) * MAX_RWSEMAPHORES);
    if(fci.RWSemStats.pRWI)
        fci.RWSemStats.nSupplied = MAX_SEMAPHORES;
    else
        fci.RWSemStats.nSupplied = 0;

    if(FfxConfigurationInfo(&fci, (D_BOOL)pTI->fVerbose, (D_BOOL)pTI->fReset) != FFXSTAT_SUCCESS)
        return;

    /*  Always display general system stats, even if the /Stats
        option is not used.
    */
    if(DclSystemInfoDisplay(&fci.SysInfo) != DCLSTAT_SUCCESS)
        DclPrintf("Error retrieving system stats\n");

    if(pTI->fStats)
    {
        if(DclMemStatsDisplay(&fci.MemStats) != DCLSTAT_SUCCESS)
            DclPrintf("Memory tracking stats disabled or not available\n");

        if(DclMutexStatsDisplay(&fci.MutStats) != DCLSTAT_SUCCESS)
            DclPrintf("Mutex tracking stats disabled or not available\n");

        if(DclSemaphoreStatsDisplay(&fci.SemStats) != DCLSTAT_SUCCESS)
            DclPrintf("Semaphore tracking stats disabled or not available\n");

        if(DclSemaphoreRdWrStatsDisplay(&fci.RWSemStats) != DCLSTAT_SUCCESS)
            DclPrintf("Read/write semaphore tracking stats disabled or not available\n");
    }

    if(fci.RWSemStats.pRWI)
        DclMemFree(fci.RWSemStats.pRWI);

    if(fci.SemStats.pDSI)
        DclMemFree(fci.SemStats.pDSI);

    if(fci.MutStats.pDMI)
        DclMemFree(fci.MutStats.pDMI);

    DclPrintf("FlashFX Compile-Time Configuration Options:\n");

    DclPrintf("  FFX_MAX_DEVICES:                               %U\n", fci.uMaxFmlDevices);
    DclPrintf("  FFX_MAX_DISKS:                                 %U\n", fci.uMaxVbfDrives);

    DclPrintf("  MBR support:                                 %s\n",
        (fci.ulConfBits & FFXCONFBIT_MBRSUPPORT         ? "Yes" : " No"));

    DclPrintf("  Reliance Support:                            %s\n",
        (fci.ulConfBits & FFXCONFBIT_RELIANCESUPPORT    ? "Yes" : " No"));

    DclPrintf("  FAT Support:                                 %s\n",
        (fci.ulConfBits & FFXCONFBIT_FATSUPPORT         ? "Yes" : " No"));

    DclPrintf("    FAT Monitor functionality:                 %s\n",
        (fci.ulConfBits & FFXCONFBIT_FATMONITORSUPPORT  ? "Yes" : " No"));

    DclPrintf("  Format functionality:                        %s\n",
        (fci.ulConfBits & FFXCONFBIT_FORMATSUPPORT      ? "Yes" : " No"));

    DclPrintf("    Device driver auto-format logic:           %s\n",
        (fci.ulConfBits & FFXCONFBIT_DRIVERAUTOFORMAT   ? "Yes" : " No"));

    DclPrintf("    MBR format functionality:                  %s\n",
        (fci.ulConfBits & FFXCONFBIT_MBRFORMAT          ? "Yes" : " No"));

    DclPrintf("    Internal FAT format function:              %s\n",
        (fci.ulConfBits & FFXCONFBIT_FATFORMATSUPPORT   ? "Yes" : " No"));

    DclPrintf("    BBM format functionality:                  %s\n",
        (fci.ulConfBits & FFXCONFBIT_BBMFORMAT          ? "Yes" : " No"));

    DclPrintf("  Force Aligned I/O:                           %s\n",
        (fci.ulConfBits & FFXCONFBIT_FORCEALIGNEDIO     ? "Yes" : " No"));

    if(fci.uCompactionModel == FFX_COMPACT_SYNCHRONOUS)
    {
        DclPrintf("  Compaction Model:                    Synchronous\n");
    }
    else if(fci.uCompactionModel == FFX_COMPACT_BACKGROUNDIDLE)
    {
        DclPrintf("  Compaction Model:                Background Idle\n");
    }
    else
    {
        DclAssert(fci.uCompactionModel == FFX_COMPACT_BACKGROUNDTHREAD);
        DclPrintf("  Compaction Model:              Background Thread\n");
    }
    DclPrintf("  Run-time latency configuration:              %s\n",
        (fci.ulConfBits & FFXCONFBIT_LATENCYRUNTIME     ? "Yes" : " No"));
    DclPrintf("    Latency auto-tune:                         %s\n",
        (fci.ulConfBits & FFXCONFBIT_LATENCYAUTOTUNE    ? "Yes" : " No"));
    DclPrintf("    Erase-suspend supported:                   %s\n",
        (fci.ulConfBits & FFXCONFBIT_LATENCYERASESUSPEND ? "Yes" : " No"));

    DclPrintf("\n");

#if 0

   /*----------------------------------------------------------------*/

   /*----------------------------------------------------------------*/
    FfxPrintf
        ("\nFlashFX Run-Time Configuration Options (for the specified drive):\n");

   /*----------------------------------------------------------------*/

   /*----------------------------------------------------------------*/


    FfxPrintf("    ------> General Options <------\n");

    if(FfxHookOptionGet(FFXOPT_SECTORLENGTH, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Emulated sector length:               %lu\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Emulated sector length:               Unsupported Option!\n");

#if FFXCONF_FATMONITORSUPPORT
    if(FfxHookOptionGet(FFXOPT_USEFATMONITOR, pDX, &fTrue, sizeof fTrue))
    {
        if(fTrue)
            FfxPrintf
                ("      FAT Monitor:                          Enabled\n");
        else
            FfxPrintf
                ("      FAT Monitor:                          Disabled\n");
    }
    else
    {
        FfxPrintf
            ("      FAT Monitor:                          Unsupported Option!\n");
    }
#endif


    FfxPrintf("    ------> Formatting Options <------\n");

    if(FfxHookOptionGet(FFXOPT_DEFAULTFS, pDX, &uTemp, sizeof uTemp))
    {
        if(uTemp == FFX_FILESYS_FAT)
            FfxPrintf("      Default file system:                  FAT\n");
        else if(uTemp == FFX_FILESYS_RELIANCE)
            FfxPrintf
                ("      Default file system:                  Reliance\n");
        else if(uTemp == FFX_FILESYS_UNKNOWN)
            FfxPrintf("      Default file system:                  None\n");
        else
            FfxPrintf
                ("      Default file system:                  Invalid! (%u)\n",
                 uTemp);
    }
    else
    {
        FfxPrintf
            ("      Default file system:                  Unsupported Option!\n");
    }

#if FFXCONF_DRIVERAUTOFORMAT
    if(FfxHookOptionGet(FFXOPT_DISK_FORMATSTATE, pDX, &uTemp, sizeof uTemp))
    {
        if(uTemp == FFX_FORMAT_NEVER)
            FfxPrintf("      Auto-Format State:                    Never\n");
        else if(uTemp == FFX_FORMAT_ONCE)
            FfxPrintf("      Auto-Format State:                    Once\n");
        else if(uTemp == FFX_FORMAT_ALWAYS)
            FfxPrintf("      Auto-Format State:                    Always\n");
        else
            FfxPrintf
                ("      Auto-Format State:                    Invalid! (%u)\n",
                 uTemp);
    }
    else
    {
        FfxPrintf
            ("      Auto-Format State:                    Unsupported Option!\n");
    }
#endif

    if(FfxHookOptionGet(FFXOPT_FORMATBBMSTATE, pDX, &uTemp, sizeof uTemp))
    {
        if(uTemp == FFX_FORMAT_NEVER)
            FfxPrintf("      BBM Auto-Format State:                Never\n");
        else if(uTemp == FFX_FORMAT_ONCE)
            FfxPrintf("      BBM Auto-Format State:                Once\n");
        else if(uTemp == FFX_FORMAT_ALWAYS)
            FfxPrintf("      BBM Auto-Format State:                Always\n");
        else
            FfxPrintf
                ("      BBM Auto-Format State:                Invalid! (%u)\n",
                 uTemp);
    }
    else
    {
        FfxPrintf
            ("      BBM Auto-Format State:                Unsupported Option!\n");
    }

    if(FfxHookOptionGet(FFXOPT_USEMBR, pDX, &fTrue, sizeof fTrue))
    {
        if(fTrue)
            FfxPrintf
                ("      Write an MBR:                         Enabled\n");
        else
            FfxPrintf
                ("      Write an MBR:                         Disabled\n");
    }
    else
    {
        FfxPrintf
            ("      Write an MBR:                         Unsupported Option!\n");
    }

    if(FfxHookOptionGet(FFXOPT_FORMATTOTALSIZE, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Allocator total size:                 0x%08lx\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Allocator total size:                 Unsupported Option!\n");

    if(FfxHookOptionGet(FFXOPT_CLIENTTOTALSIZE, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Client total size:                    0x%08lx\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Client total size:                    Unsupported Option!\n");


#if FFXCONF_FATFORMATSUPPORT
    if(FfxHookOptionGet(FFXOPT_FATROOTENTRIES, pDX, &uTemp, sizeof uTemp))
        FfxPrintf("      FAT root entries:                         0x%04x\n",
                  uTemp);
    else
        FfxPrintf
            ("      FAT root entries:                     Unsupported Option!\n");
    if(FfxHookOptionGet(FFXOPT_FATCOUNT, pDX, &uTemp, sizeof uTemp))
        FfxPrintf("      Number of FATs:                           0x%04x\n",
                  uTemp);
    else
        FfxPrintf
            ("      Number of FATs:                       Unsupported Option!\n");
#endif


#if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    FfxPrintf("    ------> Background Compaction <------\n");
    if(FfxHookOptionGet(FFXOPT_COMPACTIONPRIORITY, pDX, &uTemp, sizeof uTemp))
        FfxPrintf("      Priority                              %u\n", uTemp);
    else
        FfxPrintf
            ("      Priority                              Unsupported Option!\n");
    if(FfxHookOptionGet
       (FFXOPT_COMPACTIONREADIDLEMS, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Read idle milliseconds                %lu\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Read idle milliseconds                Unsupported Option!\n");
    if(FfxHookOptionGet
       (FFXOPT_COMPACTIONWRITEIDLEMS, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Write idle milliseconds               %lu\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Write idle milliseconds               Unsupported Option!\n");
    if(FfxHookOptionGet
       (FFXOPT_COMPACTIONSLEEPMS, pDX, &ulTemp, sizeof ulTemp))
        FfxPrintf("      Sleep milliseconds                    %lu\n",
                  ulTemp);
    else
        FfxPrintf
            ("      Sleep milliseconds                    Unsupported Option!\n");
#endif

#endif

    return;
}
 
