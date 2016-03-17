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

    Formats the media with the low level structures needed, and may place
    a FAT format on the media.

    ToDo:
      - Modify this utility to return standard status values.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: format.c $
    Revision 1.33  2010/07/13 00:03:28Z  garyp
    Cleaned up the syntax display.
    Revision 1.32  2010/01/27 04:27:33Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.31  2009/08/02 16:57:25Z  garyp
    Merged from the v4.0 branch.  Updated so that VBF instances are created
    in compaction suspend mode and have to be explicitly resumed.  Modified
    the shutdown processes to take a mode parameter.  Updated to use some
    reorganized FAT functionality.  Updated to use the revised FfxSignOn() 
    function, which now takes an fQuiet parameter.  Condition the FAT code on
    FFXCONF_FATFORMATSUPPORT rather than FFXCONF_FATSUPPORT.
    Now query the FAT count and serial number via the options interface.
    Revision 1.30  2009/04/01 14:42:40Z  davidh
    Function hearders updated for AutoDoc.
    Revision 1.29  2009/03/09 19:40:06Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.28  2009/02/17 06:13:28Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.27  2009/02/09 01:35:41Z  garyp
    Merged from the v4.0 branch.  Modified so the test's "main" function still
    compiles, even if allocator support is disabled.  Updated to use the new
    FFXTOOLPARAMS structure.
    Revision 1.26  2008/12/09 21:32:10Z  keithg
    No longer pass a serial number to VbfFormat() - it is handled internally.
    Revision 1.25  2008/05/23 17:16:40Z  thomd
    Added fFormatNoErase
    Revision 1.24  2008/03/22 23:54:13Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.23  2008/01/13 07:26:59Z  keithg
    Function header updates to support autodoc.
    Revision 1.22  2007/12/01 03:32:55Z  Garyp
    Modified FfxFatFormat() to return a meaningful status code rather than
    D_BOOL, and adjusted the calling code to make use of it as appropriate.
    Revision 1.21  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.20  2007/09/27 00:02:10Z  jeremys
    Renamed a header file.
    Revision 1.19  2007/08/30 22:35:51Z  pauli
    Resolved Bugs 458, 1268, 1398 and 1400.  The cushion parameter is now
    specified in 10ths of percent.  Eliminated the use of byte counts to
    represent the size of the disk.  Removed the one spare unit limitation.
    Revision 1.18  2007/08/16 04:55:51Z  garyp
    Modified to never return negative values.
    Revision 1.17  2007/06/08 23:50:45Z  rickc
    Fixed WriteMBR() to properly return D_BOOL.
    Revision 1.16  2007/06/08 19:01:29Z  rickc
    Cast to remove compiler warning.
    Revision 1.15  2007/06/07 20:13:59Z  rickc
    Replaced deprecated vbfwrite() calls with FfxVbfWritePages()
    Revision 1.14  2006/12/16 00:44:27Z  Garyp
    Modified to no longer use the C library rand() function.
    Revision 1.13  2006/11/01 01:02:27Z  Garyp
    Updated to eliminate compiler warnings with the Green Hills tools.
    Revision 1.12  2006/10/16 20:58:22Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.11  2006/10/13 01:22:27Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.10  2006/10/04 00:35:03Z  Garyp
    Updated to use DclMemAllocZero().
    Revision 1.9  2006/06/12 11:59:19Z  Garyp
    Eliminated the use of the deprecated vbfclientsize().
    Revision 1.8  2006/03/05 01:57:22Z  Garyp
    Modified the usage of FfxFmlDiskInfo() and FfxFmlDeviceInfo().
    Revision 1.7  2006/02/21 02:02:42Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.6  2006/02/12 20:07:23Z  Garyp
    Eliminated obsolete settings.  Removed dead code.
    Revision 1.5  2006/02/08 00:22:30Z  Garyp
    Updated to use the new FML interface.
    Revision 1.4  2006/01/12 04:38:27Z  Garyp
    Updated to handle (or gracefully fail) in cases where non-standard
    block sizes are being used.
    Revision 1.3  2006/01/05 03:36:27Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.2  2005/12/08 23:02:42Z  Garyp
    Eliminated the use of FFX_RESERVED_SPACE.
    Revision 1.1  2005/12/04 21:07:10Z  Pauli
    Initial revision
    Revision 1.3  2005/12/04 21:07:10Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.2  2005/10/20 03:30:54Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/03 20:37:22Z  Garyp
    Initial revision
    Revision 1.18  2005/05/17 22:03:29Z  garyp
    Modified to avoid attempting to suspend compaction if the VBF handle is NULL.
    Revision 1.17  2004/12/30 17:32:44Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.16  2004/12/21 07:20:54Z  garys
    Fixed /T and /R options to recognize M for MB
    Revision 1.15  2004/12/21 04:22:18  GaryP
    Fixed a bug in the command-line parser.  Fixed to build cleanly with GNU
    tools.
    Revision 1.14  2004/12/19 21:05:53Z  GaryP
    Eliminated an unused parameter.
    Revision 1.13  2004/12/19 04:09:20Z  GaryP
    Completely revamped to support writing FAT or Reliance MBRs independently
    from the high level format (if any).  Parameters updated to be more
    consistent with other utilities.
    Revision 1.12  2004/12/16 08:11:28Z  garys
    Added TOUPPER and TOLOWER macros to allow typecasting for gnu.
    Revision 1.11  2004/12/14 03:30:39  jeremys
    Fixed WriteMBR default fs, used to use DEFAULTFS_FAT, now uses
    FFX_DEFAULTFS.  Was broken for Reliance.
    Revision 1.10  2004/11/20 00:54:46Z  GaryP
    Modified to suspend/restore background compaction around the format
    operation.
    Revision 1.9  2004/09/29 21:16:27Z  garys
    Initialize hidden sectors to avoid bug when FAT formatting without MBR
    Revision 1.8  2004/09/25 04:48:20  GaryP
    Fixed to only write an MBR if the VBF_BLOCK_SIZE value is less than or
    equal to DSK_SECTORLEN.  Moved some FAT specific code into conditional
    blocks.
    Revision 1.7  2004/09/23 06:01:21Z  GaryP
    Changed to use DSK_SECTORLEN and FAT_SECTORLEN.
    Revision 1.6  2004/09/17 02:27:25Z  GaryP
    Minor function renaming exercise.
    Revision 1.5  2004/08/09 21:56:06Z  GaryP
    Modified so that all FAT logic is conditional on FFXCONF_FATSUPPORT.  Added
    write MBR code since that is no longer an integral part of FfxFatFormat().
    Revision 1.4  2004/07/20 01:29:24Z  GaryP
    Minor include file updates.
    Revision 1.3  2004/05/06 01:22:54Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.2  2004/03/15 22:55:02Z  garys
    Eliminate compiler warning about dangling "else".
    Revision 1.1  2004/02/02 19:52:58Z  garyp
    Initial revision
    Revision 1.17  2003/09/18 19:27:05Z  billr
    Change test for reserved space to eliminate a warning from some
    compilers when FFX_RESERVED_SPACE was zero.
    Revision 1.16  2003/09/12 17:27:36Z  garys
    updated usage for /P with new Min and Max values
    Revision 1.15  2003/09/11 20:55:50  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.14  2003/06/13 11:57:04  garyp
    Eliminated the direct inclusion of VBF internal header files.
    Revision 1.13  2003/05/28 17:39:34Z  brandont
    Added a variable size default number of root directory entries that is
    based on the size of the media but only for media smaller than 1.44Mb in
    order to make better use of the limited space on smaller flash disks.
    Revision 1.12  2003/05/08 18:39:48  garyp
    Minor output formatting changes.
    Revision 1.11  2003/05/06 21:04:43Z  garys
    completely removed bSkipErase flag (f and F case, and commeted
    out code), and added /U to the usage screen.
    Revision 1.10  2003/05/01 16:33:30  billr
    Reorganize headers: include/vbf.h is now nothing but the public interface.
    VBF internal information has moved to core/vbf/_vbf.h.  core/vbf/_vbflowl.h
    is obsolete.  A new header include/vbfconf.h contains definitions the OEM
    may want to change.  There were a few changes to types, and to names of
    manifest constants to accommodate this.
    Revision 1.9  2003/04/21 19:33:48Z  garys
    bSkipErase was removed from vbffmt.c in the vbf4 branch.  We are still
    deciding whether we want to remove it here or add it back into vbffmt.c.
    Revision 1.8  2003/04/10 19:07:30  garyp
    Updated to use the new fatformat() calling sequence.
    Revision 1.7  2003/03/27 23:09:02Z  garyp
    Fixed some DclPrintf() issues.
    Revision 1.6  2003/03/27 20:31:48Z  garyp
    Modified to use standard FlashFX utility functions where possible.  Fixed
    to cast string pointer to char * as needed when calling DclPrintf().
    Revision 1.5  2002/12/04 07:42:52Z  garyp
    Changed _sysputc() to _dosputchar().
    02/07/02 DE  Updated copyright notice for 2002.
    02/09/00 TWQ Updated DisplayStatistics to correctly display information
                 for RAM drives under FlashFX.  Removed an invalid
                 DclProductionAssert() that was showing a fatal error when
                 we tried to format a RAM disk.
    12/15/99 GJS Corrected address to write DPB rebuild flag (FF).
    03/16/99 TWQ Added undocumented /U option to unconditionaly format the
                 media as if it had never been formatted before.  It will
                 ignore previous reserved areas of the flash overwriting them.
    03/10/99 TWQ Added code to recognize a disk change.
    03/10/99 HDS Returned max preserve space message to original value.
    03/02/99 HDS Updated message (max preserve space value) to match new
                 MAX_UNIT_SIZE.
    02/09/99 HDS Flagged all printfs to make them easier to find for
                 modification.
    01/05/99 TWQ Added /F (Fast) Format switch (No Pre-Erase if not needed)
    11/18/98 PKG Made signon same as other tools
    11/16/98 HDS Made minor change to display information.
    09/01/98 PKG Changed lpcScratchData to be in terms of VBF_SCRATCH_BLOCK_SIZE
    04/28/98 PKG Removed call to oemunmount.  This will cause the format to
                 fail for flash memory such as NAND.
    12/10/97 PKG Fixed code that was performing toupper in SetSwitches that
                 was causing the incorrect drive number to be accessed before
                 and after format.  Also had side effect of accessing the
                 floppy drive during DBP checks (IsBiosDrive)
    12/10/97 HDS Fixed command name.
    12/03/97 PKG Removed use of C library getche() it was almost trippling the
                 size of the binary.
    12/01/97 PKG Removed putc and getc these are now part of the API
    11/24/97 PKG Fixed cushion validity checks to use the defines set in the
                 vbf.h header file.
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses extended Datalight specific types.
    08/26/97 PKG Now uses new Datalight specific types
    08/07/97 PKG Now use FAT_SECTORLEN instead of the BLOCK_SIZE, Added FAT
                 format error checking.
    07/28/97 PKG Added /q switch to control the cushion space allocated in the
                 flash format.
    06/18/97 PKG Fixed /r switch to work when no drive letter or number is
                 given on the command line.
    06/17/97 PKG Now uses _sysgetc() for user interaction
    06/16/97 PKG Added oemunmount() call
    06/11/97 PKG Removed use of GetUnitNum(), this functionality is now covered
                 by the ApiInstallCheck()  Added unit number and BIOS number
                 checks in ParseCommandLine().
    05/29/97 PKG Added DEV_MASK to identify NAND flash
    05/22/97 PKG Now uses our own _dl_dos_absread, Borlands is buggy
    04/17/97 PKG Removed use of DOS.H and randomize()
    04/03/97 PKG Further updates to the summary output
    03/25/97 PKG Added multiple spare unit support and new summary
    03/24/97 PKG Added low level DOS I/O to allow redirection by DOS
    02/24/97 PKG Original revision from CardTrick v3.01
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <dlerrlev.h>

#if FFXCONF_VBFSUPPORT

#include <fxdriver.h>
#include <fxfmlapi.h>
#include <diskapi.h>
#include <vbf.h>

#if FFXCONF_FATFORMATSUPPORT
#include <dlfat.h>
#include <fxfatapi.h>

#define LARGE_DISK_DIR_ENTRIES      512
#define SMALL_DISK_DIR_ENTRIES      240
#define COMPUTE_OPTIMAL_ENTRIES     0
#endif

#define ARGBUFFLEN                  128

typedef struct
{
    D_BOOL      fMBRPartFAT;
    D_BOOL      fMBRPartReliance;
    D_BOOL      fFormatFAT;
    D_UINT32    ulNumSpareUnits;
    D_UINT32    ulCushionSpace;
  #if FFXCONF_FATFORMATSUPPORT
    D_UINT16    uNumDirEntries;
    D_CHAR      szLabel[12];
  #endif
    D_BOOL      fFormatNoErase;
} FMTPARAMS;


/*-------------------------------------------------------------------
               Prototypes for internal functions
-------------------------------------------------------------------*/
static void     ShowUsage(FFXTOOLPARAMS *pTP);
static D_INT16  FormatDisk(FFXFMLHANDLE hFML, FMTPARAMS *pFP);
static void     DisplayStatistics(FFXFMLHANDLE hFML, VBFHANDLE hVBF, FMTPARAMS *pFP);
static D_INT16  SetSwitches(FFXTOOLPARAMS *pTP, FMTPARAMS *pFP);
#if FFXCONF_MBRFORMAT
static D_BOOL   WriteMBR(VBFHANDLE hVBF, D_UINT32 ulSectorLen, D_UINT16 uFS, D_UINT32 *pulTotalSectors, D_UINT32 *pulHiddenSectors, D_UINT16 *puHeads, D_UINT16 *puSPT, D_UINT16 *puCylinders);
#endif

#endif


/*-------------------------------------------------------------------
    Public: FfxFormat()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxFormat(
    FFXTOOLPARAMS  *pTP)
{
  #if FFXCONF_VBFSUPPORT
    D_INT16         iErrorLevel;
    FMTPARAMS       fp;
    FFXFMLHANDLE    hFML;
    VBFHANDLE       hVBF;
    FFXSTATUS       ffxStat;

    DclPrintf("\nFlashFX Disk Format\n");
    FfxSignOn(FALSE);

    DclMemSet(&fp, 0, sizeof(FMTPARAMS));

  #if FFXCONF_FATFORMATSUPPORT
    fp.uNumDirEntries = COMPUTE_OPTIMAL_ENTRIES;
    DclStrCpy(fp.szLabel, "NO NAME    ");
  #endif
    fp.ulNumSpareUnits = FFX_USE_DEFAULT;
    fp.ulCushionSpace = FFX_USE_DEFAULT;

    iErrorLevel = SetSwitches(pTP, &fp);
    if(iErrorLevel)
        return iErrorLevel;

    hFML = FfxFmlHandle(pTP->nDiskNum);
    if(!hFML)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    hVBF = FfxVbfHandle(pTP->nDiskNum);
    if(hVBF)
    {
        ffxStat = FfxVbfDestroy(hVBF, FFX_SHUTDOWNFLAGS_TIMECRITICAL);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Error %lX unloading VBF for DISK%u\n", ffxStat, pTP->nDiskNum);
            return __LINE__;
        }
    }

    return FormatDisk(hFML, &fp);

  #else

    (void)pTP;

    DclPrintf("FlashFX is configured with Allocator support disabled\n");

    return DCLERRORLEVEL_FEATUREDISABLED;

  #endif
}


#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ShowUsage()

    Displays the usage information.

    Parameters:
        pTP - A pointer to the FFXTOOLPARAMS structure to use

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nThis command performs a low-level FlashFX format on the specified Disk,\n");
    DclPrintf("and can optionally write an MBR.\n\n");
    DclPrintf("Syntax: %s Disk [Options]\n\n", pTP->dtp.pszCmdName);
    DclPrintf("Where:\n");
    DclPrintf("  Disk      The FlashFX Disk designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  /?        This help information\n");
    DclPrintf("  /S:n      The number of spare units, Min=%U, Max=%U, Default=%U\n", VBF_MIN_SPARE, VBF_MAX_SPARE, VBF_DEFAULT_SPARE);
    DclPrintf("  /Q:n      The cushion in 10ths of percent, Min=%U, Max=%U, Default=%U\n", VBF_MIN_CUSHION, VBF_MAX_CUSHION, VBF_DEFAULT_CUSHION);
    DclPrintf("  /FNE      Perform a Factory Fast format (no erase)\n");
  #if FFXCONF_FATFORMATSUPPORT
    DclPrintf("  /FAT      Place a FAT format on the disk\n");
    DclPrintf("  /V:\"vol\"  FAT: Place volume label \"vol\" on disk\n");
    DclPrintf("  /D:n      FAT: 'n' specifies root dir entries, Min=16, Max=512, Default=240\n");
  #endif
  #if FFXCONF_MBRFORMAT
    DclPrintf("  /Part:fs  Create a FAT or Reliance partition where fs='fat' or fs='rel'\n\n");
    DclPrintf("An MBR will be written only if the /Part option is used.\n");
  #endif

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: FormatDisk()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_INT16 FormatDisk(
    FFXFMLHANDLE    hFML,
    FMTPARAMS      *pFP)
{
    VBFHANDLE       hVBF;
    FFXFMLINFO      FmlInfo;
    D_UINT32        ulTotalSectors;
    D_UINT16        uHeads;
    D_UINT16        uSPT;
    D_UINT16        uCylinders;
    VBFDISKINFO     di;
    FFXSTATUS       ffxStat;
  #if FFXCONF_MBRFORMAT || FFXCONF_FATFORMATSUPPORT
    D_UINT32        ulHiddenSectors = 0;
  #endif

    /*  Get some info about the media, be sure it's up to date
    */
    FfxFmlDiskInfo(hFML, &FmlInfo);
    if(FmlInfo.ulTotalBlocks == 0L)
    {
        DclPrintf("No media found to format\n");
        return __LINE__;
    }

    DclAssert(FmlInfo.ulBlockSize);

    DclPrintf("Formatting Flash...\n");
    ffxStat = FfxVbfFormat(hFML, pFP->ulCushionSpace, pFP->ulNumSpareUnits, pFP->fFormatNoErase);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("VBF format failed, Status=%lX\n", ffxStat);
        return __LINE__;
    }

    /*  Make sure the VBF format is recognized
    */
    hVBF = FfxVbfCreate(hFML);
    if(!hVBF)
    {
        DclPrintf("VBF mount failed!\n");
        return __LINE__;
    }

    if(FfxVbfDiskInfo(hVBF, &di) != FFXSTAT_SUCCESS)
        return __LINE__;

  #if FFXCONF_FATFORMATSUPPORT
    if(pFP->fFormatFAT && (di.uPageSize != FAT_SECTORLEN))
    {
        DclPrintf("Cannot FAT format unless the sector size is 512\n");
        return __LINE__;
    }
  #endif

    /*  Record the total number of sectors
    */
    ulTotalSectors = di.ulTotalPages;

    /*  If we are writing either kind of partition, or if we are using no
        MBR, but still writing a FAT format, we need to do this to calculate
        the proper CHS values.
    */
    if(pFP->fMBRPartFAT || pFP->fMBRPartReliance || pFP->fFormatFAT)
    {
        D_UINT16    uFS = FFX_FILESYS_UNKNOWN;

        if(pFP->fMBRPartReliance)
        {
            uFS = FFX_FILESYS_RELIANCE;
        }
      #if FFXCONF_FATFORMATSUPPORT
        else
        {
            uFS = FFX_FILESYS_FAT;
        }
      #endif

        if(!FfxCalculateCHS(&ulTotalSectors, &uHeads, &uSPT, &uCylinders))
            return __LINE__;

        if(pFP->fMBRPartFAT || pFP->fMBRPartReliance)
        {
          #if FFXCONF_MBRFORMAT
            if(!WriteMBR(hVBF, di.uPageSize, uFS, &ulTotalSectors,
                         &ulHiddenSectors, &uHeads, &uSPT, &uCylinders))
                return __LINE__;
          #else
            (void)uFS;
            DclProductionError();
          #endif
        }
    }


  #if FFXCONF_FATFORMATSUPPORT
    if(pFP->fFormatFAT)
    {
        DCLFATPARAMS ffp = {0};

        ffp.uBytesPerSector = di.uPageSize;

        /*  Determine if a number of root directory entries has
            been specified
        */
        if(pFP->uNumDirEntries == COMPUTE_OPTIMAL_ENTRIES)
        {
            if(ulTotalSectors <= ((1024UL * 1024UL) / ffp.uBytesPerSector))
                pFP->uNumDirEntries = SMALL_DISK_DIR_ENTRIES;
            else
                pFP->uNumDirEntries = LARGE_DISK_DIR_ENTRIES;
        }

        FfxHookOptionGet(FFXOPT_FATCOUNT, FmlInfo.hDisk, &ffp.uNumFATs, sizeof ffp.uNumFATs);
        DclAssert(ffp.uNumFATs);

        if(!FfxHookOptionGet(FFXOPT_FATSERIALNUM, FmlInfo.hDisk, &ffp.ulSerialNumber, sizeof ffp.ulSerialNumber))
        {
            ffp.ulSerialNumber = (D_UINT32)DclRand(NULL);
        }

        ffp.ulTotalSectors = ulTotalSectors;
        ffp.ulHiddenSectors = ulHiddenSectors;
        ffp.uHeads = uHeads;
        ffp.uSPT = uSPT;
        ffp.uCylinders = uCylinders;
        ffp.uRootEntries = pFP->uNumDirEntries;
        DclStrCpy(ffp.szVolumeLabel, pFP->szLabel);

        /*  Format the media with a FAT file system
        */
        ffxStat = FfxFatFormat(hVBF, &ffp);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("FAT format failed, Status=%lX\n", ffxStat);
            return __LINE__;
        }
    }
  #endif

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    /*  The VBF instance is created with idle-time compaction
        suspended.  It must be explicitly enabled.
    */
    FfxVbfCompactIdleResume(hVBF);
  #endif

    /*  Show the user what happened
    */
    DisplayStatistics(hFML, hVBF, pFP);

    return 0;
}


#if FFXCONF_MBRFORMAT

/*-------------------------------------------------------------------
    Local: WriteMBR()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL WriteMBR(
    VBFHANDLE   hVBF,
    D_UINT32    ulSectorLen,
    D_UINT16    uFS,
    D_UINT32   *pulTotalSectors,
    D_UINT32   *pulHiddenSectors,
    D_UINT16   *puHeads,
    D_UINT16   *puSPT,
    D_UINT16   *puCylinders)
{
    D_BUFFER   *pSector;
    FFXIOSTATUS ioStat;

    DclAssert(*pulTotalSectors);

    if(*pulTotalSectors < 32)
        return FALSE;

    pSector = DclMemAllocZero(ulSectorLen);
    if(!pSector)
        return FALSE;

    DclAssert(ulSectorLen >= DSK_SECTORLEN);

    /*  Hypothetically we could have a VBF allocation block size that is
        larger than a standard MBR.  In this event we will simply fill
        in the first DSK_SECTORLEN bytes of the MBR, and the remainder
        will be filled with NULLs.
    */
    *pulHiddenSectors = FfxMBRBuild(pSector, *puHeads, *puSPT,
                                    *puCylinders, ulSectorLen, uFS);

    *pulTotalSectors -= *pulHiddenSectors;

    ioStat = FfxVbfWritePages(hVBF, 0, 1, pSector);

    DclMemFree(pSector);

    if(IOSUCCESS(ioStat, 1))
        return TRUE;
    else
        return FALSE;
}

#endif


/*-------------------------------------------------------------------
    Local: DisplayStatistics()

    Displays information about the statistics of the media just
    formatted.

    Parameters:

    Return Value:
        None.
-------------------------------------------------------------------*/
static void DisplayStatistics(
    FFXFMLHANDLE    hFML,
    VBFHANDLE       hVBF,
    FMTPARAMS      *pFP)
{
    FFXFMLINFO      FmlInfo;
    VBFDISKINFO     di;
    D_UINT32        ulFmlPages;

    (void) pFP;

    FfxFmlDiskInfo(hFML, &FmlInfo);

    ulFmlPages = FmlInfo.ulTotalBlocks *
                 (FmlInfo.ulBlockSize / FmlInfo.uPageSize);

    FfxVbfDiskInfo(hVBF, &di);

    DclPrintf("Format Complete\n");
    DclPrintf("    Page size:    %10lU Bytes\n", di.uPageSize);
    DclPrintf("    Total:        %10lU Pages\n", ulFmlPages);
    DclPrintf("    Formatted:    %10lU Pages\n", di.ulTotalPages);
    DclPrintf("    VBF Overhead: %10lU Pages\n", ulFmlPages - di.ulTotalPages);

    return;
}


/*-------------------------------------------------------------------
    Local: SetSwitches()

    Parses the command line arguments and sets any globals needed
    for the tests.  Also, shows help via ShowUsage(), if they need
    it.

    Parameters:

    Return Value:
        Error level non-zero, if any problems are encountered.
        Zero, if the command line arguments are valid and recorded.
-------------------------------------------------------------------*/
static D_INT16 SetSwitches(
    FFXTOOLPARAMS  *pTP,
    FMTPARAMS      *pFP)
{
    D_INT16         i, j, k;
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        argc;
    D_UINT16        uIndex;

    argc = DclArgCount(pTP->dtp.pszCmdLine);

    for(i = 1; i <= argc; i++)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, i, ARGBUFFLEN, achArgBuff))
        {
            DclPrintf("\nBad argument!\n");
            return __LINE__;
        }

        j = 0;

        /*  Allow for multiple switches in a string.
        */
        while(achArgBuff[j])
        {
            /*  Save the index value of the switch...
            */
            k = j;

            if(achArgBuff[j] == '/')
            {

                /*  Go to the next character to test.
                */
                ++j;

                /*  Information specfic for each switch
                */
                switch (achArgBuff[j])
                {
                    case '?':
                        ShowUsage(pTP);
                        return 1;

                  #if FFXCONF_MBRFORMAT
                    case 'p':
                    case 'P':
                        ++j;
                        if(DclStrNICmp(&achArgBuff[j - 1], "part:", 5) != 0)
                            goto bad_form;

                        if(DclStrNICmp(&achArgBuff[j + 4], "fat", 3) == 0)
                            pFP->fMBRPartFAT = TRUE;
                        else if(DclStrNICmp(&achArgBuff[j + 4], "rel", 3) == 0)
                            pFP->fMBRPartReliance = TRUE;
                        else
                            goto bad_form;

                        j += 7;
                        break;
                  #endif

                    case 'f':
                    case 'F':
                        ++j;
                        if(DclStrNICmp(&achArgBuff[j - 1], "fne", 3) == 0)
                        {
                            pFP->fFormatNoErase = TRUE;
                        }
                  #if !FFXCONF_FATFORMATSUPPORT
                        else
                            goto bad_form;

                        j += 2;
                        break;
                  #else
                        else if(DclStrNICmp(&achArgBuff[j - 1], "fat", 3) == 0)
                        {
                            pFP->fFormatFAT = TRUE;
                        }
                        else
                            goto bad_form;

                        j += 2;
                        break;

                    case 'v':
                    case 'V':
                    {
                        D_UINT16          uLength;

                        ++j;
                        if(achArgBuff[j] != ':')
                            goto bad_form;
                        ++j;
                        if(achArgBuff[j] != '"')
                            goto bad_form;
                        ++j;

                        uLength = DclStrLen(&achArgBuff[j]) - 1;
                        if(uLength > 11)
                            uLength = 11;
                        DclMemCpy(pFP->szLabel, &achArgBuff[j], uLength);

                        /*  Pad with spaces.
                        */
                        for(uIndex = uLength; uIndex < 11; ++uIndex)
                            pFP->szLabel[uIndex] = ' ';

                        /*  Skip the rest of this option.
                        */
                        j += uLength + 1;
                        break;
                    }

                    case 'd':
                    case 'D':
                    {
                        int ulNumDirEntries;
                        
                        ++j;
                        if(achArgBuff[j] != ':')
                            goto bad_form;
                        ++j;
                        uIndex = 0;

                        /*  Check for invalid characters.
                        */
                        while(achArgBuff[j + uIndex])
                        {
                            if(achArgBuff[j + uIndex] < '0'
                               || achArgBuff[j + uIndex] > '9')
                            {
                                if(achArgBuff[j + uIndex] != '/')
                                    goto bad_form;
                                else
                                    break;
                            }
                            ++uIndex;
                        }

                        /*  Get the number of entries from the command line.
                            Remember DclAtoI returns int, not D_UINT16.
                        */
                        ulNumDirEntries = DclAtoI(&achArgBuff[j]);

                        /*  Ensure its valid.
                        */
                        if(ulNumDirEntries < 16
                           || ulNumDirEntries > 512)
                        {
                            --j;
                            goto bad_form;
                        }

                        /*  Type conversion OK, we know ulNumDirEntries is
                            between 16 and 512.
                        */
                        pFP->uNumDirEntries = (D_UINT16)ulNumDirEntries;
                        
                        /*  Make it modulo 16.
                        */
                        if(pFP->uNumDirEntries % 16 != 0)
                            pFP->uNumDirEntries +=
                                (16 - (pFP->uNumDirEntries % 16));

                        /*  Next option
                        */
                        j += uIndex;
                        break;
                    }
                  #endif

                    case 's':
                    case 'S':
                        ++j;
                        if(achArgBuff[j] != ':')
                            goto bad_form;
                        ++j;
                        uIndex = 0;

                        /*  Check for invalid characters.
                        */
                        while(achArgBuff[j + uIndex])
                        {
                            if(achArgBuff[j + uIndex] < '0'
                               || achArgBuff[j + uIndex] > '9')
                            {
                                if(achArgBuff[j + uIndex] != '/')
                                    goto bad_form;
                                else
                                    break;
                            }
                            ++uIndex;
                        }

                        /*  Get the number of spare units from the command line.
                        */
                        pFP->ulNumSpareUnits = DclAtoL(&achArgBuff[j]);
                        if(pFP->ulNumSpareUnits < VBF_MIN_SPARE
                           || pFP->ulNumSpareUnits > VBF_MAX_SPARE)
                            goto bad_form;

                        /*  Next option
                        */
                        j += uIndex;
                        break;

                    case 'q':
                    case 'Q':
                        ++j;
                        if(achArgBuff[j] != ':')
                            goto bad_form;
                        ++j;
                        uIndex = 0;

                        /*  Check for invalid characters.
                        */
                        while(achArgBuff[j + uIndex])
                        {
                            if(achArgBuff[j + uIndex] < '0'
                               || achArgBuff[j + uIndex] > '9')
                            {
                                if(achArgBuff[j + uIndex] != '/')
                                    goto bad_form;
                                else
                                    break;
                            }
                            ++uIndex;
                        }

                        /*  Get the cushion percentage from the command line.
                        */
                        pFP->ulCushionSpace = DclAtoL(&achArgBuff[j]);
                        if(pFP->ulCushionSpace < VBF_MIN_CUSHION
                           || pFP->ulCushionSpace > VBF_MAX_CUSHION)
                            goto bad_form;

                        /*  Next option
                        */
                        j += uIndex;
                        break;

                        /*  Unrecognized switch
                        */
                      bad_form:
                    default:
                        DclPrintf("\n  Bad option: %s\n", &achArgBuff[k]);
                        ShowUsage(pTP);
                        return 10;
                }
            }
            else
            {
                goto bad_form;
            }
        }
    }


   /*------------------------------------------------------------
                        Validity checking....
   ------------------------------------------------------------*/

    if(pFP->fMBRPartFAT && pFP->fMBRPartReliance)
    {
        DclPrintf("\nCannot specify that both FAT and Reliance partitions be written.\n");
        return __LINE__;
    }

  #if FFXCONF_FATFORMATSUPPORT
    if(pFP->fFormatFAT && pFP->fMBRPartReliance)
    {
        DclPrintf("\nCannot specify to format as FAT with a Reliance partition.\n");
        return __LINE__;
    }

  #else
    if(pFP->fMBRPartFAT || pFP->fFormatFAT)
    {
        DclPrintf("\nThe driver is not configured to support writing a FAT format.\n");
        return __LINE__;
    }
  #endif

  #if !FFXCONF_RELIANCESUPPORT
    if(pFP->fMBRPartReliance)
    {
        DclPrintf("\nThe driver is not configured to support writing a Reliance partition.\n");
        return __LINE__;
    }
  #endif

    /*  All worked fine
    */
    return 0;
}



#endif  /* FFXCONF_VBFSUPPORT */



