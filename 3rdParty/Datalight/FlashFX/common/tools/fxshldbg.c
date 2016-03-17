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

    This module creates a FlashFX command shell instance which contains
    commands for debugging flash.

    ToDo: The Block Status command (BStatus) should/could be generalized to
          apply to all types of flash, rather than just NAND.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshldbg.c $
    Revision 1.34  2011/04/23 00:35:42Z  garyp
    Fixed some parameter parsing problems.
    Revision 1.33  2011/02/09 00:57:54Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.32  2010/07/25 21:24:02Z  garyp
    Tweaked some comparison logic to avoid warnings with the Code
    Sourcery tools.  Corrected some bad Hungarian.
    Revision 1.31  2010/07/12 23:32:52Z  garyp
    Fixed a cut-and-paste error.
    Revision 1.30  2010/07/05 22:09:05Z  garyp
    Added a verify option to the FML write command.
    Revision 1.29  2010/04/29 00:04:21Z  garyp
    Updated to build cleanly with the TICCS ToolSet abstraction.
    Revision 1.28  2010/01/27 04:27:34Z  glenns
    Repair issues exposed by turning on a compiler option to warn of  possible 
    data loss resulting from implicit typecasts between integer data types.
    Revision 1.27  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.26  2009/12/11 20:23:11Z  garyp
    Renamed some commands to better match the main shell.
    Revision 1.25  2009/12/03 22:32:18Z  garyp
    Cleaned up some messages -- no functional changes.
    Revision 1.24  2009/11/25 23:12:37Z  garyp
    Renamed the BSTATUS command to BRAWSTATUS, and added a new BSTATUS command
    which operates on logical blocks.
    Revision 1.23  2009/11/17 20:17:44Z  garyp
    Enhanced the ECC command to display OMAP35x style ECCs, if possible.
    Revision 1.22  2009/10/06 18:20:10Z  garyp
    Renamed the "ecc" command to "eccssfdc" and added a new "ecc" command which
    operates using the Datalight canonical ECC form.  Fixed a parsing bug in 
    the "CompareBytes" command.
    Revision 1.21  2009/07/20 20:44:26Z  garyp
    Merged from the v4.0 branch.  Added the block lock/unlock, and the "data"
    commands.  Documentation updated.
    Revision 1.20  2009/04/09 02:58:24Z  garyp
    Renamed a structure to avoid AutoDoc naming collisions.
    Revision 1.19  2009/04/01 15:08:22Z  davidh
    Function Headers Updated for AutoDoc.
    Revision 1.18  2009/03/18 17:32:27Z  glenns
    Fix Bugzilla #2370: Removed references to obsolete block status values.
    Revision 1.17  2009/03/09 19:43:31Z  thomd
    Cast parameter of FfxFmlHandle()
    Revision 1.16  2009/02/17 06:18:00Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.15  2009/02/09 02:48:43Z  garyp
    Merged from the v4.0 branch.  Renamed the "Disk" command to "ChgDisk".  
    Added the Disk and Device commands which operate identically to those same
    commands in the main FlashFX Shell.  Updated to allow the debug shell to 
    function even when there are no Disks defined in the system.  Major update
    to support 8 page and spare buffers, rather than 2, to allow multi-page 
    commands to be issued.
    Revision 1.14  2008/12/19 07:14:17Z  keithg
    Replaced get/set block status capabilities; Removed status types that
    are no longer used.
    Revision 1.13  2008/10/21 00:07:57Z  keithg
    Conditioned obsolescent block status functionality on BBM v5.
    Revision 1.12  2008/05/20 22:28:31Z  billr
    Fix warning that ulBlockStatus may be used uninitialized.
    Revision 1.11  2008/05/06 16:14:35Z  keithg
    o use generic FFXSTAT_xxx codes.
    Revision 1.10  2008/05/03 03:46:22Z  garyp
    Added the /V and /I options for the BlockStatus command.  Fixed a broken
    syntax display for BlockStatus.  Fixed broken parsing of the /TAGS option
    for the FMLRead/Write commands.
    Revision 1.9  2008/03/17 17:05:46Z  Garyp
    Added the BBMINFO and CB (CompareBytes) commands.  Updated to work when
    allocator support is disabled.  Modified command which use tags to allow a
    tag width to be specified.
    Revision 1.8  2008/01/13 07:27:05Z  keithg
    Function header updates to support autodoc.
    Revision 1.7  2008/01/06 16:26:28Z  garyp
    Fixed a potential memory corruption issue.  Updated to release resources
    in reverse order of allocation.
    Revision 1.6  2007/12/17 07:58:45Z  Garyp
    Modified the FXDebug command to automatically choose the FlashFX
    DISK to use.  Rename the "exit" command to "quit" to avoid conflicts with
    the CE Target Control window.
    Revision 1.5  2007/12/05 02:48:44Z  Garyp
    Updated to use the new DclShellAddMultipleCommands() function.  Fixed
    to restore the original prompt on exit, if any.
    Revision 1.4  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/10/14 21:12:35Z  Garyp
    Modified to use a renamed function.
    Revision 1.2  2007/09/13 00:20:43Z  Garyp
    Updated to build properly when NAND support is disabled.  Fixed
    miscellaneous compiler warnings.
    Revision 1.1  2007/09/12 19:34:20Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <dlfatapi.h>
#include <fxtools.h>
#include <fxfmlapi.h>
#include <fxstats.h>
#include <fxdevapi.h>
#include <fxtrace.h>
#include <dlshell.h>
#include <fxshell.h>
#include <dlpartid.h>
#include <diskapi.h>
#include <ecc.h>
#if FFXCONF_ALLOCATORSUPPORT
#include <vbf.h>
#endif
#include "fxshl.h"

#if DCLCONF_COMMAND_SHELL

#define MAX_BUFFERS           (8)   /* Must be a single decimal digit */
#define ARGBUFFLEN          (256)   /* Buffer for concatenating arguments */
#define DEFAULT_BYTE_WIDTH   (32)
#define MAX_PROMPT_LEN       (16)
#define MAX_DESCRIPTION_LEN  (16)

typedef struct _FXDB
{
    FFXFMLHANDLE        hFML;
  #if FFXCONF_ALLOCATORSUPPORT
    VBFHANDLE           hVBF;
    VBFDISKMETRICS      VbfInfo;
  #endif
    D_BUFFER           *pPB[MAX_BUFFERS];
    D_BUFFER           *pSB[MAX_BUFFERS];
    FFXFMLINFO          ffi;
    FFXFMLDEVINFO       DevInfo;
    unsigned            nDiskNum;
    char                szPrompt[MAX_PROMPT_LEN];
} FXDB;


static DCLSTATUS    DebugBBMInfo(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugChangeDisk(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDevice(        DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDisk(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugECC(           DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugECCSSFDC(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugInfo(          DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugCompareBytes(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDisplayBytes(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDisplayWords(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDisplayDWords( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugDisplayStruct( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFillBytes(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFillWords(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFillDWords(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLBlockLock(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLBlockUnlock(DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLBlockStatus(DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLBlockRawStatus( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLErase(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLRead(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLWrite(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLSpareRead(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLSpareWrite( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLTagRead(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugFMLTagWrite(   DCLSHELLHANDLE hShell, int argc, char **argv);
#if FFXCONF_ALLOCATORSUPPORT
static DCLSTATUS    DebugAllocRead(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugAllocWrite(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    DebugAllocDiscard(  DCLSHELLHANDLE hShell, int argc, char **argv);
#endif

static const DCLSHELLCOMMAND aDebugCmds[] =
{
    {"FXDB General",   "%sBBMInfo",   "Display Bad Block Manager information",        DebugBBMInfo},
    {"FXDB General",   "%sDisk",      "Change the current Disk",                      DebugChangeDisk},
    {"FXDB General",   "%sECC",       "Calculate ECCs in Datalight canonical form",   DebugECC},
    {"FXDB General",   "%sECCSSFDC",  "Calculate ECCs in SSFDC form",                 DebugECCSSFDC},
    {"FXDB General",   "%sFXDevice",  "Create, destroy, and enumerate Devices",       DebugDevice},
    {"FXDB General",   "%sFXDisk",    "Create, destroy, and enumerate Disks",         DebugDisk},
    {"FXDB General",   "%sFXInfo",    "Display detailed Device and Disk information", DebugInfo},
    {"FXDB Data",      "%sCB",        "Compare data in byte format",                  DebugCompareBytes},
    {"FXDB Data",      "%sData",      "Display data as a structure",                  DebugDisplayStruct},
    {"FXDB Data",      "%sDB",        "Display data in byte format",                  DebugDisplayBytes},
    {"FXDB Data",      "%sDW",        "Display data in word format",                  DebugDisplayWords},
    {"FXDB Data",      "%sDD",        "Display data in dword format",                 DebugDisplayDWords},
    {"FXDB Data",      "%sFB",        "Fill an area with bytes",                      DebugFillBytes},
    {"FXDB Data",      "%sFW",        "Fill an area with words",                      DebugFillWords},
    {"FXDB Data",      "%sFD",        "Fill an area with dwords",                     DebugFillDWords},
  #if FFXCONF_ALLOCATORSUPPORT
    {"FXDB Allocator", "%sARead",     "Read allocator pages from flash",              DebugAllocRead},
    {"FXDB Allocator", "%sAWrite",    "Write allocator pages to flash",               DebugAllocWrite},
    {"FXDB Allocator", "%sADiscard",  "Discard allocator pages",                      DebugAllocDiscard},
  #endif
    {"FXDB FML",       "%sBErase",    "Erase one or more blocks",                     DebugFMLErase},
    {"FXDB FML",       "%sBLock",     "Lock one or more physical flash blocks",       DebugFMLBlockLock},
    {"FXDB FML",       "%sBUnlock",   "Unlock one or more physical flash blocks",     DebugFMLBlockUnlock},
    {"FXDB FML",       "%sBStatus",   "Get the status for a NAND block",              DebugFMLBlockStatus},
    {"FXDB FML",       "%sBRawStatus","Get/Set the status for a physical NAND block", DebugFMLBlockRawStatus},
    {"FXDB FML",       "%sPRead",     "Read pages from flash",                        DebugFMLRead},
    {"FXDB FML",       "%sPWrite",    "Write pages to flash",                         DebugFMLWrite},
    {"FXDB FML",       "%sSRead",     "Read spare area data from NAND flash",         DebugFMLSpareRead},
    {"FXDB FML",       "%sSWrite",    "Write spare area data to NAND flash",          DebugFMLSpareWrite},
    {"FXDB FML",       "%sTRead",     "Read tags from NAND flash",                    DebugFMLTagRead},
    {"FXDB FML",       "%sTWrite",    "Write tags to NAND flash",                     DebugFMLTagWrite}
};


static FFXSTATUS InitDisk(DCLSHELLHANDLE hShell, FXDB *pFXDB, unsigned nDiskNum);
static D_BOOL    ParseDisplayArgs(  FXDB *pFXDB, int argc, char **argv, D_UINT32 *pulOffset, D_UINT32 *pulCount, D_UINT16 *puWidth, D_BUFFER **ppBuff, char *pszDescription, unsigned nScale, unsigned nConsoleWidth);
static D_BOOL    ParseBufferName(   FXDB *pFXDB, const char *pszName, D_BUFFER **ppBuff, char *pszDescription, D_UINT16 *puWidth, unsigned *pnElements);
static D_BOOL    ParsePageBuffName( FXDB *pFXDB, const char *pszName, D_BUFFER **ppBuff, char *pszDescription, unsigned *pnElements, unsigned *pnBufferNum, unsigned *pfFoundArg);
static D_BOOL    ParseSpareBuffName(FXDB *pFXDB, const char *pszName, D_BUFFER **ppBuff, char *pszDescription, unsigned *pnElements, unsigned *pfFoundArg);
static unsigned  CalculateHexDumpWidth(unsigned nConsoleWidth, unsigned nScale);
static D_BOOL    RevalidateDisk(DCLSHELLHANDLE hShell);
static D_BOOL    IsValidDisk(DCLSHELLHANDLE hShell, FXDB *pFXDB);


/*-------------------------------------------------------------------
    Protected: FfxDebug()

    Invoke the FlashFX Debugger (FXDB) using an instance of the
    Datalight Shell.

    Parameters:

    Return Value:
        Returns an FFXSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
FFXSTATUS FfxDebug(
    unsigned        nDiskNum,
    DCLSHELLPARAMS *pSP)
{
    #define         MAX_OLD_PROMPT_LEN (64)
    char            szOldPrompt[MAX_OLD_PROMPT_LEN];
    FXDB           *pFXDB;
    DCLSHELLHANDLE  hShell;
    FFXSTATUS       ffxStat;
    DCLSTATUS       dclStat = DCLSTAT_SHELLCREATEFAILED;
    char            szWidthCmd[24];

    DclPrintf("\nFlashFX Debugger\n");
    FfxSignOn(FALSE);
    DclPrintf("\n");

    DclAssert(pSP);

    pFXDB = DclMemAllocZero(sizeof(*pFXDB));
    if(!pFXDB)
        return DCLSTAT_OUTOFMEMORY;

    ffxStat = InitDisk(NULL, pFXDB, nDiskNum);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclMemFree(pFXDB);
        return ffxStat;
    }

    /*  If an environment is supplied, save off the old PROMPT value so it
        can be restored on exit.
    */
    if(pSP->hEnv)
    {
        const char *pszOldPrompt;

        pszOldPrompt = DclEnvVarLookup(pSP->hEnv, "PROMPT");
        if(pszOldPrompt)
            DclStrNCpy(szOldPrompt, pszOldPrompt, sizeof(szOldPrompt));
        else
            szOldPrompt[0] = 0;
    }

    /*  Use the console width parameter from the parent shell.  We don't
        have a handy API function to set this, but we're not using the
        script, so generate a single line script command to set the
        console width to the same value as the parent shell.
    */
    DclSNPrintf(szWidthCmd, sizeof(szWidthCmd), "width %u\n", pSP->nWidth);

    hShell = DclShellCreate(pSP->hDclInst, "FXDB", szWidthCmd, pSP->hEnv, TRUE, pSP->fEcho, pFXDB);
    if(hShell)
    {
        dclStat = DclShellAddMultipleCommands(hShell, aDebugCmds, DCLDIMENSIONOF(aDebugCmds), "FXDB", "");
        if(dclStat == DCLSTAT_SUCCESS)
        {
            /*  For some bizarre reason that only MS knows, in the CE Target
                Control window, the word "Exit" is parsed before we even get
                a chance to see it.  Even worse, CE just displays an error,
                and doesn't seem to actually do anything.  This makes it
                impossible to exit the shell using that command.

                Therefore, even though it only necessary for CE, and even then
                only when running within the target control window, always
                rename the EXIT command to QUIT.
            */
            dclStat = DclShellRenameCommand(hShell, "Exit", "Quit");
            DclAssert(dclStat == DCLSTAT_SUCCESS);

            /*  For the most part we don't want access to the general purpose
                FFX shell commands, however the trace commands are useful to
                have in this interface, so add them.
            */
            dclStat = FfxShellAddTraceCommands(hShell, "Debugging", "");
            if(dclStat == FFXSTAT_SUCCESS)
            {
                DCLSHELLPARAMS  params;

                DclShellParams(hShell, &params);

                DclEnvVarAdd(params.hEnv, "PROMPT", pFXDB->szPrompt);

                dclStat = DclShellRun(hShell);
            }
        }

        DclShellDestroy(hShell);
    }

    if(pFXDB->pSB[0])
        DclMemFree(pFXDB->pSB[0]);
    DclMemFree(pFXDB->pPB[0]);
    DclMemFree(pFXDB);

    /*  Restore the original prompt value.
    */
    if(pSP->hEnv)
        DclEnvVarAdd(pSP->hEnv, "PROMPT", szOldPrompt);

    return dclStat;
}


                    /*------------------------------*\
                     *                              *
                     *         FXDB Commands        *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Local: DebugBBMInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugBBMInfo(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;
    FFXBBMSTATS     BbmStats;
    FFXSTATUS       ffxStat;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc > 1 || ISHELPREQUEST())
    {
        DclPrintf("This command displays information about the FlashFX Bad Block\n");
        DclPrintf("Manager (BBM)\n\n");

        return FFXSTAT_BADSYNTAX;
    }

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    ffxStat = FfxFmlParameterGet(pFXDB->hFML, FFXPARAM_STATS_BBM, &BbmStats, sizeof(BbmStats));
    if(ffxStat == FFXSTAT_SUCCESS)
    {
        FfxBbmStatsDisplay(&BbmStats, TRUE);
    }
    else
    {
        if(ffxStat == FFXSTAT_CATEGORYDISABLED)
            DclPrintf("BBM Statistics are not enabled\n\n");
        else
            DclPrintf("Error %lX querying BBM statistics\n\n", ffxStat);
    }

    return ffxStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS DebugChangeDisk(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;
    DCLSTATUS       dclStat;
    const char     *pStr;
    D_UINT32        ulDisk;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc != 2 || ISHELPREQUEST())
        goto DisplayHelp;

    pStr = DclNtoUL(argv[1], &ulDisk);
    if(!pStr || *pStr != 0)
    {
        DclPrintf("Syntax error in \"%s\"\n\n", argv[1]);
        return FFXSTAT_BADSYNTAX;
    }

    if(ulDisk >= FFX_MAX_DISKS)
    {
        DclPrintf("The Disk number is not valid, must range from 0 to %u\n", FFX_MAX_DISKS-1);
        return FFXSTAT_BADSYNTAX;
    }

    if(ulDisk == pFXDB->nDiskNum)
    {
        DclPrintf("DISK%lU is already the current Disk\n", ulDisk);
        return FFXSTAT_BADSYNTAX;
    }

    dclStat = InitDisk(hShell, pFXDB, (D_UINT16)ulDisk);

    return dclStat;

  DisplayHelp:

    DclPrintf("This command changes the current Disk for the debugger.\n\n");
    DclPrintf("Syntax:  %s DiskNum\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  DiskNum    - The Disk number to make current, relative to 0.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS DebugDisk(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    dclStat = FfxShellCmdDisk(hShell, argc, argv);

    /*  A Disk could have disappeared underneath us, or a new one
        might have appeared.
    */
    RevalidateDisk(hShell);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS DebugDevice(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    dclStat = FfxShellCmdDevice(hShell, argc, argv);

    /*  A Disk could have disappeared underneath us, or a new one
        might have appeared.
    */
    RevalidateDisk(hShell);

    return dclStat;
}


/*-------------------------------------------------------------------
    Local: DebugECC()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugECC(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_BUFFER       *pBuff;
    D_BUFFER       *pWorkBuff;
    DCLSHELLPARAMS  params;
    int             ii;
    D_UINT16        uLen;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 3 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!ParsePageBuffName(pFXDB, argv[1], &pBuff, NULL, NULL, NULL, NULL))
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[1]);
        goto DisplayHelp;
    }

    if(argc == 3)
    {
        int iLen;
        
        if(DclStrNICmp(argv[2], "/len:", 5) != 0)
        {
            DclPrintf("Unrecognized option: \"%s\"\n\n", argv[2]);
            goto DisplayHelp;
        }

        iLen = DclAtoI(&argv[2][5]);

        if(iLen < 1 || iLen > D_INT16_MAX)
        {
            DclPrintf("The specified length of %U is not valid\n\n", iLen);
            goto DisplayHelp;
        }
        else
        {
            /*  Range check done, typecast OK:
            */
            uLen = (D_UINT16)iLen;
        }
        
        if(!uLen || uLen > pFXDB->ffi.uPageSize || pFXDB->ffi.uPageSize % uLen)
        {
            DclPrintf("The specified length of %U is not valid\n\n", uLen);
            goto DisplayHelp;
        }
    }
    else
    {
        uLen = pFXDB->ffi.uPageSize;
    }

    pWorkBuff = pBuff;
    for(ii=0; ii<pFXDB->ffi.uPageSize/uLen; ii++)
    {
        D_UINT32    ulECC;

        ulECC = DclEccCalculate(pWorkBuff, uLen);

        DclPrintf("Canonical ECCs for the %U bytes at segment %d:  %lX\n", uLen, ii, ulECC);

        pWorkBuff += uLen;
    }

    if(uLen == 512)
    {
        pWorkBuff = pBuff;
        for(ii=0; ii<pFXDB->ffi.uPageSize/uLen; ii++)
        {
             D_BUFFER    abECC[3];

            DclEccCanonicalToOmap35x(DclEccCalculate(pWorkBuff, uLen), abECC);

            DclPrintf("OMAP35x ECCs for the 512 bytes at segment %d:  %02x %02x %02x\n", ii, abECC[0], abECC[1], abECC[2]);

            pWorkBuff += uLen;
        }
    }
    else
    {
        DclPrintf("OMAP35x ECCs can only be calculated for 512 byte lengths\n");
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command calculates ECC codes in Datalight canonical form, for the\n");
    DclPrintf("data in the specified Page Buffer.\n\n");
    DclPrintf("Syntax:  %s Buffer [/Len:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The Page Buffer (PBn) for which ECCs will be calculated.\n");
    DclPrintf("  /Len:n     - Specifies an alternate length to use if the ECC should be\n");
    DclPrintf("               calculated on a length less than the size of a page.  In\n");
    DclPrintf("               this case 'n' should evenly divide the page size.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugECCSSFDC()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugECCSSFDC(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_BUFFER       *pBuff;
    DCLSHELLPARAMS  params;
    int             ii;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc != 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!ParsePageBuffName(pFXDB, argv[1], &pBuff, NULL, NULL, NULL, NULL))
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[1]);
        goto DisplayHelp;
    }

    for(ii=0; ii<pFXDB->ffi.uPageSize/512; ii++)
    {
        D_BUFFER    abECC1[BYTES_PER_ECC];
        D_BUFFER    abECC2[BYTES_PER_ECC];

        FfxEccCalculate(&pBuff[(ii*512) + 0], abECC1);
        FfxEccCalculate(&pBuff[(ii*512) + DATA_BYTES_PER_ECC], abECC2);

        DclPrintf("ECCs for 512 bytes at segment %u:  %02x %02x %02x -- %02x %02x %02x\n",
            ii, abECC1[0], abECC1[1], abECC1[2], abECC2[0], abECC2[1], abECC2[2]);
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command calculates SSFDC style ECC codes for the data in the specified\n");
    DclPrintf("Page Buffer.\n\n");
    DclPrintf("Syntax:  %s Buffer\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The Page Buffer (PBn) for which ECCs will be calculated.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugInfo(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    FFXTOOLPARAMS   tp;
    int             ii;
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 1 || ISHELPREQUEST())
        goto DisplayHelp;

    tp.nDeviceNum = 0;
    tp.nDiskNum = pFXDB->nDiskNum;

    achBuffer[0] = 0;
    for(ii=1; ii<argc; ii++)
    {
        size_t  nLen = sizeof(achBuffer) - (DclStrLen(achBuffer) + 2);

        if(DclStrLen(argv[ii]) > nLen)
        {
            DclPrintf("Command arguments too long\n");
            return FFXSTAT_ARGUMENTSTOOLONG;
        }

        DclStrCat(achBuffer, " ");
        DclStrNCat(achBuffer, argv[ii], nLen);
    }

    if(FfxDiskInfo(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxDiskInfo(&tp);

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugCompareBytes()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugCompareBytes(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset;
    D_UINT32        ulCount;
    D_BUFFER       *pSrc;
    D_BUFFER       *pDest;
    char            szSrcDesc[MAX_DESCRIPTION_LEN];
    char            szDestDesc[MAX_DESCRIPTION_LEN];
    unsigned        nSrcElements;
    unsigned        nDestElements;
    D_UINT16        uSrcMaxWidth;
    D_UINT16        uDestMaxWidth;
    D_UINT32        ulTotalBytesDiff = 0;
    D_UINT32        ulTotalBitsDiff = 0;
    DCLSHELLPARAMS  params;
    unsigned        ii;
    int             iArgNum;
    D_UINT16        uMaxWidth;
    unsigned        nMaxElements;
    unsigned        fVerbose = FALSE;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 3 || argc > 5 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!ParseBufferName(pFXDB, argv[1], &pSrc, szSrcDesc, &uSrcMaxWidth, &nSrcElements))
    {
        DclPrintf("Bad syntax:  \"%s\" is not a valid buffer\n", argv[1]);
        goto DisplayHelp;
    }

    if(!ParseBufferName(pFXDB, argv[2], &pDest, szDestDesc, &uDestMaxWidth, &nDestElements))
    {
        DclPrintf("Bad syntax:  \"%s\" is not a valid buffer\n", argv[2]);
        goto DisplayHelp;
    }

    ulOffset = 0;
    uMaxWidth = DCLMIN(uSrcMaxWidth, uDestMaxWidth);
    nMaxElements = DCLMIN(nSrcElements, nDestElements);
    ulCount = uMaxWidth;

    /*  Loop through all remaining arguments and process them
    */
    for(iArgNum=3; iArgNum<argc; iArgNum++)
    {
        const char     *pStr;

        if(DclStrICmp(argv[iArgNum], "/v") == 0)
        {
            fVerbose = TRUE;
            continue;
        }

        pStr = DclNtoUL(argv[iArgNum], &ulOffset);
        if(pStr && *pStr == ':')
        {
            if(ulOffset >= uMaxWidth)
            {
                DclPrintf("The specified offset (%lU) is out of range\n", ulOffset);
                goto DisplayHelp;
            }

            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(ulOffset + ulCount > uMaxWidth * nMaxElements)
            {
                DclPrintf("The specified offset+count (%lU+%lU) is out of range\n", ulOffset, ulCount);
                goto DisplayHelp;
            }

            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n\n", argv[iArgNum]);
        return FALSE;
    }

    DclPrintf("Comparing %s to %s at offset %lU for %lU bytes\n",
        szSrcDesc, szDestDesc, ulOffset, ulCount);

    for(ii=0; ii < ulCount; ii++)
    {
        D_UINT8 bSrc = pSrc[ulOffset+ii];
        D_UINT8 bDest = pDest[ulOffset+ii];

        if(bSrc != bDest)
        {
            unsigned    nDiff = DclBitCount(bSrc ^ bDest);

            if(fVerbose)
            {
                DclPrintf("Mismatch at relative offset %u:  %02X - %02X  (%u bits different)\n",
                    ii, bSrc, bDest, nDiff);
            }

            ulTotalBytesDiff++;
            ulTotalBitsDiff += nDiff;
        }
    }

    if(ulTotalBytesDiff)
        DclPrintf("%lU byte(s) were different, containing %lU bit(s) flipped\n", ulTotalBytesDiff, ulTotalBitsDiff);
    else
        DclPrintf("No differences encountered\n");

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command compares data in byte format.\n\n");
    DclPrintf("Syntax:  %s Buff1 Buff2 [Offset[:Count]] [/V]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buff1      - The first buffer to use PBn, or SBn.\n");
    DclPrintf("  Buff2      - The second buffer to use PBn, or SBn.\n");
    DclPrintf("  Offset     - The absolute byte offset into the buffer at which to start the\n");
    DclPrintf("               compare operation.  Defaults to 0.\n");
    DclPrintf("  Count      - The total number of bytes to compare.  Defaults to the full amount\n");
    DclPrintf("             - of the remaining length of the shortest buffer.  Multiple buffers\n");
    DclPrintf("               may be compared if Count exceeds the remaining space.\n");
    DclPrintf("  /V         - Provide verbose information about any differences\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugDisplayBytes()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugDisplayBytes(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset;
    D_UINT32        ulCount;
    D_UINT16        uWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc > 4 || ISHELPREQUEST())
        goto DisplayHelp;

    if(ParseDisplayArgs(pFXDB, argc, argv, &ulOffset, &ulCount, &uWidth, &pBuff, szDescription, 1, params.nWidth))
    {
        DclPrintf("Displaying %lU bytes at absolute offset %lU into %s:\n", ulCount, ulOffset, szDescription);
        DclHexDump(NULL, HEXDUMP_UINT8, uWidth, ulCount, pBuff+ulOffset);

        return FFXSTAT_SUCCESS;
    }

  DisplayHelp:

    DclPrintf("This command displays data in byte format.\n\n");
    DclPrintf("Syntax:  %s [Source] [Offset[:Count]] [/Width:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Source     - The source data to display, either PBn or SBn.  PB1 is the\n");
    DclPrintf("               default.\n");
    DclPrintf("  Offset     - The absolute byte offset into the source data at which to start\n");
    DclPrintf("               displaying.\n");
    DclPrintf("  Count      - The total number of bytes to display.\n");
    DclPrintf("  /Width:n   - The number of bytes per line, from 1 to 32, default scales to the\n");
    DclPrintf("               console width.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugDisplayWords()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugDisplayWords(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset;
    D_UINT32        ulCount;
    D_UINT16        uWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc > 4 || ISHELPREQUEST())
        goto DisplayHelp;

    if(ParseDisplayArgs(pFXDB, argc, argv, &ulOffset, &ulCount, &uWidth, &pBuff, szDescription, 2, params.nWidth))
    {
        DclPrintf("Displaying %lU words at absolute offset %lU into %s:\n", ulCount, ulOffset, szDescription);
        DclHexDump(NULL, HEXDUMP_UINT16, uWidth, ulCount, pBuff+ulOffset);

        return FFXSTAT_SUCCESS;
    }

  DisplayHelp:

    DclPrintf("This command displays data in word (2-byte) format.\n\n");
    DclPrintf("Syntax:  %s [Source] [Offset[:Count]] [/Width:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Source     - The source data to display, either PBn or SBn.  PB1 is the\n");
    DclPrintf("               default.\n");
    DclPrintf("  Offset     - The absolute byte offset into the source data at which to start\n");
    DclPrintf("               displaying.\n");
    DclPrintf("  Count      - The total number of words to display.\n");
    DclPrintf("  /Width:n   - The number of bytes per line, from 1 to 32, default scales to the\n");
    DclPrintf("               console width.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugDisplayDWords()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugDisplayDWords(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset;
    D_UINT32        ulCount;
    D_UINT16        uWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc > 4 || ISHELPREQUEST())
        goto DisplayHelp;

    if(ParseDisplayArgs(pFXDB, argc, argv, &ulOffset, &ulCount, &uWidth, &pBuff, szDescription, 4, params.nWidth))
    {
        DclPrintf("Displaying %lU dwords at absolute offset %lU into %s:\n", ulCount, ulOffset, szDescription);
        DclHexDump(NULL, HEXDUMP_UINT32, uWidth, ulCount, pBuff+ulOffset);

        return FFXSTAT_SUCCESS;
    }

  DisplayHelp:

    DclPrintf("This command displays data in dword (4-byte) format.\n\n");
    DclPrintf("Syntax:  %s [Source] [Offset[:Count]] [/Width:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Source     - The source data to display, either PBn or SBn.  PB1 is the\n");
    DclPrintf("               default.\n");
    DclPrintf("  Offset     - The absolute byte offset into the source data at which to start\n");
    DclPrintf("               displaying.\n");
    DclPrintf("  Count      - The total number of dwords to display.\n");
    DclPrintf("  /Width:n   - The number of bytes per line, from 1 to 32, default scales to the\n");
    DclPrintf("               console width.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugDisplayStruct

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugDisplayStruct(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_BUFFER       *pBuff;
    DCLSHELLPARAMS  params;
    unsigned        fGotArg;
    int             iArgNum;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 4 || ISHELPREQUEST())
        goto DisplayHelp;

    iArgNum = 1;

    if(!ParsePageBuffName(pFXDB, argv[iArgNum], &pBuff, NULL, NULL, NULL, &fGotArg))
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[1]);
        goto DisplayHelp;
    }

    if(fGotArg)
        iArgNum++;

    if(DclStrICmp(argv[iArgNum], "AS") == 0)
        iArgNum++;

    if(iArgNum != argc-1)
        goto DisplayHelp;

    if(DclStrICmp(argv[iArgNum], "FATBR") == 0)
    {
        DCLFATBPB   bpb;

        if(DclFatBPBMove(&bpb, pBuff) != FSID_NONE)
        {
            DclFatBPBDisplay(&bpb);
            return DCLSTAT_SUCCESS;
        }
        else
        {
            DclPrintf("The data was not a recognized FAT BPB format\n");
            return DCLSTAT_FAT_UNRECOGNIZEDBPB;
        }
    }
  #if FFXCONF_MBRSUPPORT
    else if(DclStrICmp(argv[iArgNum], "MBR") == 0)
    {
        DclMBRPartitionDisplayTable(pBuff, TRUE);

        return DCLSTAT_SUCCESS;
    }
  #endif

  DisplayHelp:

    DclPrintf("This command displays data as one of the following structures:\n\n");
    DclPrintf("Syntax:  %s [Source] [as] StructType\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("      Source - The Page Buffer to use.  PB1 is the default.\n");
    DclPrintf("  StructType - Is one of the following:\n");
    DclPrintf("                 FATBR - Display the data as a FAT boot record.\n");
  #if FFXCONF_MBRSUPPORT
    DclPrintf("                 MBR   - Display the data as an MBR.\n");
  #endif

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFillBytes

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFillBytes(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset = 0;
    D_UINT32        ulCount = D_UINT32_MAX;
    D_UINT32        ulFill;
    D_UINT16        uMaxWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    int             ii;
    unsigned        xx;
    const char     *pStr;
    DCLSHELLPARAMS  params;
    unsigned        fRandom = FALSE;
    unsigned        nElements;
    unsigned        fFoundBuff = FALSE;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 4 || ISHELPREQUEST())
        goto FillHelp;

    if(DclStrICmp(argv[1], "RANDOM") == 0)
    {
        fRandom = TRUE;
    }
    else
    {
        pStr = DclNtoUL(argv[1], &ulFill);
        if(!pStr || *pStr != 0)
        {
            DclPrintf("Bad Syntax in \"%s\"\n\n", argv[1]);
            goto FillHelp;
        }

        if(ulFill > 255)
        {
            DclPrintf("Illegal fill value -- must range from 0 to 255\n\n", argv[1]);
            goto FillHelp;
        }
    }

    /*  If the event that there are no more arguments, call this function
        with a NULL pointer will initialize the default arguments.
    */
    if(argc <= 2)
    {
        ParsePageBuffName(pFXDB, NULL, &pBuff, szDescription, &nElements, NULL, NULL);
        uMaxWidth = pFXDB->ffi.uPageSize;
    }

    /*  Loop through the remaining optional arguments and process them
    */
    for(ii=2; ii<argc; ii++)
    {
        if(!fFoundBuff && ParseBufferName(pFXDB, argv[ii], &pBuff, szDescription, &uMaxWidth, &nElements))
        {
            fFoundBuff = TRUE;
            continue;
        }

        pStr = DclNtoUL(argv[ii], &ulOffset);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
            {
                DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
                goto FillHelp;
            }

            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
        goto FillHelp;
    }

    if(ulOffset >= uMaxWidth)
    {
        DclPrintf("The offset is out of range -- %lU >= %U\n\n", ulOffset, uMaxWidth);
        goto FillHelp;
    }

    if(ulCount == D_UINT32_MAX)
        ulCount = uMaxWidth - ulOffset;

    /*  Range check- must have uMaxWidth * nElements to fit in D_UINT16 
        data type.
    */
    DclAssert((((D_UINT32)uMaxWidth) * nElements) <= D_UINT16_MAX);
    uMaxWidth *= (D_UINT16)nElements;

    ulCount = DCLMIN(ulCount, uMaxWidth - ulOffset);

    for(xx=0; xx<ulCount; xx++)
    {
        if(fRandom)
            ulFill = DclRand(NULL) % 255;

        pBuff[ulOffset+xx] = (D_UINT8)ulFill;
    }

    if(fRandom)
    {
        DclPrintf("Filled %lU bytes at offset %lU in %s with a random fill value\n",
            ulCount, ulOffset, szDescription);
    }
    else
    {
        DclPrintf("Filled %lU bytes at offset %lU in %s with fill byte 0x%02lX\n",
            ulCount, ulOffset, szDescription, ulFill);
    }

    return FFXSTAT_SUCCESS;

  FillHelp:

    DclPrintf("This command fills Page/Spare Buffer(s) with the specified byte value.\n\n");
    DclPrintf("Syntax:  %s Value [Dest] [Offset[:Count]]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Value      - The byte value with which to fill.  This may be set to the\n");
    DclPrintf("               keyword RANDOM to cause a random value to be used\n");
    DclPrintf("  Dest       - The destination buffer to fill, either PBn or SBn.  Defaults to\n");
    DclPrintf("               PB1 if not specified.\n");
    DclPrintf("  Offset     - The absolute byte offset into the destination buffer at which to\n");
    DclPrintf("               start the fill operation.  Defaults to 0.\n");
    DclPrintf("  Count      - The total number of bytes to fill.  Defaults to the remaining\n");
    DclPrintf("               space within Dest, relative to Offset.  The count may be used\n");
    DclPrintf("               to fill multiple buffers, from the specified starting point\n");
    DclPrintf("               through the last Page or Spare buffer.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFillWords()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFillWords(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset = 0;
    D_UINT32        ulCount = 0;
    D_UINT32        ulFill;
    D_UINT16        uMaxWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    int             ii;
    unsigned        xx;
    const char     *pStr;
    DCLSHELLPARAMS  params;
    unsigned        fRandom = FALSE;
    unsigned        nElements = 0;  /* Avoid compiler warning */
    unsigned        fFoundBuff = FALSE;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 4 || ISHELPREQUEST())
        goto FillHelp;

    if(DclStrICmp(argv[1], "RANDOM") == 0)
    {
        fRandom = TRUE;
    }
    else
    {
        pStr = DclNtoUL(argv[1], &ulFill);
        if(!pStr || *pStr != 0)
        {
            DclPrintf("Bad Syntax in \"%s\"\n\n", argv[1]);
            goto FillHelp;
        }

        if(ulFill > 0xFFFF)
        {
            DclPrintf("Illegal fill value -- must range from 0 to 0xFFFF\n\n", argv[1]);
            goto FillHelp;
        }
    }

    /*  If the event that there are no more arguments, call this function
        with a NULL pointer will initialize the default arguments.
    */
    if(argc <= 2)
    {
        ParsePageBuffName(pFXDB, NULL, &pBuff, szDescription, &nElements, NULL, NULL);
        uMaxWidth = pFXDB->ffi.uPageSize;
    }

    /*  Loop through the remaining optional arguments and process them
    */
    for(ii=2; ii<argc; ii++)
    {
        if(!fFoundBuff && ParseBufferName(pFXDB, argv[ii], &pBuff, szDescription, &uMaxWidth, &nElements))
        {
            fFoundBuff = TRUE;
            continue;
        }

        pStr = DclNtoUL(argv[ii], &ulOffset);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
            {
                DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
                goto FillHelp;
            }

            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
        goto FillHelp;
    }

    if(ulOffset >= uMaxWidth)
    {
        DclPrintf("The offset is out of range -- %lU >= %U\n\n", ulOffset, uMaxWidth);
        goto FillHelp;
    }

    if(ulCount == D_UINT32_MAX)
        ulCount = (uMaxWidth - ulOffset) / 2;

    /*  Range check- must have uMaxWidth * nElements to fit in D_UINT16 
        data type.
    */
    DclAssert((((D_UINT32)uMaxWidth) * nElements) <= D_UINT16_MAX);
    uMaxWidth *= (D_UINT16)nElements;

    ulCount = DCLMIN(ulCount, (uMaxWidth - ulOffset) / 2);


/*
    if(!ulCount)
        ulCount = uMaxWidth;

    uMaxWidth *= nElements;

    if(ulOffset >= uMaxWidth)
    {
        DclPrintf("The offset is out of range -- %lU >= %U\n\n", ulOffset, uMaxWidth);
        goto FillHelp;
    }

    if(ulCount == D_UINT32_MAX)
        ulCount = (uMaxWidth - ulOffset) / 2;

    if(!ulCount || (ulOffset + (ulCount*2) > uMaxWidth))
    {
        DclPrintf("The offset+count is out of range -- %lU+%lU*2 >= %U-2\n\n", ulOffset, ulCount, uMaxWidth);
        goto FillHelp;
    }
*/
    for(xx=0; xx<ulCount; xx++)
    {
        if(fRandom)
            ulFill = DclRand(NULL) % 0xFFFF;

        pBuff[ulOffset+(xx*2)+0] = (D_UINT8)(ulFill >> 8);
        pBuff[ulOffset+(xx*2)+1] = (D_UINT8)(ulFill);
    }

    if(fRandom)
    {
        DclPrintf("Filled %lU words at offset %lU in %s with a random fill value\n",
            ulCount, ulOffset, szDescription);
    }
    else
    {
        DclPrintf("Filled %lU words at offset %lU in %s with fill value 0x%04lX\n",
            ulCount, ulOffset, szDescription, ulFill);
    }

    return FFXSTAT_SUCCESS;

  FillHelp:

    DclPrintf("This command fills Page/Spare Buffer(s) with the specified word value.\n\n");
    DclPrintf("Syntax:  %s Value [Dest] [Offset[:Count]]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Value      - The word value with which to fill.  This may be set to the\n");
    DclPrintf("               keyword RANDOM to cause a random value to be used\n");
    DclPrintf("  Dest       - The destination buffer to fill, either PBn or SBn.  Defaults to\n");
    DclPrintf("               PB1 if not specified.\n");
    DclPrintf("  Offset     - The absolute byte offset into the destination buffer at which to\n");
    DclPrintf("               start the fill operation.  Defaults to 0.\n");
    DclPrintf("  Count      - The total number of words to fill.  Defaults to the remaining\n");
    DclPrintf("               space within Dest, relative to Offset.  The count may be used\n");
    DclPrintf("               to fill multiple buffers, from the specified starting point\n");
    DclPrintf("               through the last Page or Spare buffer.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFillDWords()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFillDWords(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    D_UINT32        ulOffset = 0;
    D_UINT32        ulCount = 0;
    D_UINT32        ulFill;
    D_UINT16        uMaxWidth;
    D_BUFFER       *pBuff;
    char            szDescription[MAX_DESCRIPTION_LEN];
    int             ii;
    unsigned        xx;
    const char     *pStr;
    DCLSHELLPARAMS  params;
    unsigned        fRandom = FALSE;
    unsigned        nElements;
    unsigned        fFoundBuff = FALSE;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 4 || ISHELPREQUEST())
        goto FillHelp;

    if(DclStrICmp(argv[1], "RANDOM") == 0)
    {
        fRandom = TRUE;
    }
    else
    {
        pStr = DclNtoUL(argv[1], &ulFill);
        if(!pStr || *pStr != 0)
        {
            DclPrintf("Bad Syntax in \"%s\"\n\n", argv[1]);
            goto FillHelp;
        }
    }

    /*  If the event that there are no more arguments, call this function
        with a NULL pointer will initialize the default arguments.
    */
    if(argc <= 2)
    {
        ParsePageBuffName(pFXDB, NULL, &pBuff, szDescription, &nElements, NULL, NULL);
        uMaxWidth = pFXDB->ffi.uPageSize;
    }

    /*  Loop through the remaining optional arguments and process them
    */
    for(ii=2; ii<argc; ii++)
    {
        if(!fFoundBuff && ParseBufferName(pFXDB, argv[ii], &pBuff, szDescription, &uMaxWidth, &nElements))
        {
            fFoundBuff = TRUE;
            continue;
        }

        pStr = DclNtoUL(argv[ii], &ulOffset);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
            {
                DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
                goto FillHelp;
            }

            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
        goto FillHelp;
    }

    if(ulOffset >= uMaxWidth)
    {
        DclPrintf("The offset is out of range -- %lU >= %U\n\n", ulOffset, uMaxWidth);
        goto FillHelp;
    }

    if(ulCount == D_UINT32_MAX)
        ulCount = (uMaxWidth - ulOffset) / 4;

    /*  Range check- must have uMaxWidth * nElements to fit in D_UINT16 
        data type.
    */
    DclAssert((((D_UINT32)uMaxWidth) * nElements) <= D_UINT16_MAX);
    uMaxWidth *= (D_UINT16)nElements;

    ulCount = DCLMIN(ulCount, (uMaxWidth - ulOffset) / 4);

/*
    if(!ulCount)
        ulCount = uMaxWidth;

    uMaxWidth *= nElements;

    if(ulOffset >= uMaxWidth)
    {
        DclPrintf("The offset is out of range -- %lU >= %U\n\n", ulOffset, uMaxWidth);
        goto FillHelp;
    }

    if(ulCount == D_UINT32_MAX)
        ulCount = (uMaxWidth - ulOffset) / 4;

    if(!ulCount || (ulOffset + (ulCount*4) > uMaxWidth))
    {
        DclPrintf("The offset+count is out of range -- %lU+%lU*4 >= %U-4\n\n", ulOffset, ulCount, uMaxWidth);
        goto FillHelp;
    }
*/
    for(xx=0; xx<ulCount; xx++)
    {
        if(fRandom)
            ulFill = DclRand(NULL);

        pBuff[ulOffset+(xx*4)+0] = (D_UINT8)(ulFill >> 24);
        pBuff[ulOffset+(xx*4)+1] = (D_UINT8)(ulFill >> 16);
        pBuff[ulOffset+(xx*4)+2] = (D_UINT8)(ulFill >> 8);
        pBuff[ulOffset+(xx*4)+3] = (D_UINT8)(ulFill);
    }

    if(fRandom)
    {
        DclPrintf("Filled %lU dwords at offset %lU in %s with a random fill value\n",
            ulCount, ulOffset, szDescription);
    }
    else
    {
        DclPrintf("Filled %lU dwords at offset %lU in %s with fill value %lX\n",
            ulCount, ulOffset, szDescription, ulFill);
    }

    return FFXSTAT_SUCCESS;

  FillHelp:

    DclPrintf("This command fills Page/Spare Buffer(s) with the specified dword value.\n\n");
    DclPrintf("Syntax:  %s Value [Dest] [Offset[:Count]]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Value      - The dword value with which to fill.  This may be set to the\n");
    DclPrintf("               keyword RANDOM to cause a random value to be used\n");
    DclPrintf("  Dest       - The destination buffer to fill, either PBn or SBn.  Defaults to\n");
    DclPrintf("               PB1 if not specified.\n");
    DclPrintf("  Offset     - The absolute byte offset into the destination buffer at which to\n");
    DclPrintf("               start the fill operation.  Defaults to 0.\n");
    DclPrintf("  Count      - The total number of dwords to fill.  Defaults to the remaining\n");
    DclPrintf("               space within Dest, relative to Offset.  The count may be used\n");
    DclPrintf("               to fill multiple buffers, from the specified starting point\n");
    DclPrintf("               through the last Page or Spare buffer.\n");

    return FFXSTAT_BADSYNTAX;
}  


/*-------------------------------------------------------------------
    Local: DebugFMLBlockLock()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLBlockLock(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulBlockNum;
    D_UINT32        ulCount = 1;
    FFXIOSTATUS     ioStat;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 3 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(!(pFXDB->DevInfo.uDeviceFlags & DEV_LOCKABLE))
    {
        DclPrintf("The Device does not support lock/unlock commands\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

    if(DclStrICmp(argv[1], "/All") == 0)
    {
        ulBlockNum = 0;
        ulCount = pFXDB->ffi.ulTotalBlocks;
    }
    else
    {
        pStr = DclNtoUL(argv[1], &ulBlockNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }
    }

    if(argc > 2 && DclStrICmp(argv[2], "/Freeze") == 0)
    {
        if(!(pFXDB->DevInfo.nLockFlags & FFXLOCKFLAGS_LOCKFREEZE))
        {
            DclPrintf("The Device does not support the /Freeze option\n\n");
            return FFXSTAT_UNSUPPORTEDREQUEST;
        }

        FMLLOCK_FREEZE(pFXDB->hFML, ulBlockNum, ulCount, ioStat);

        /*  Note that we are not checking the exact count, because that
            may be 'n' blocks if individual blocks can be frozen, or
            1 device, if the operation is chip-wide.
        */
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Error freezing blocks, %s\n", FfxDecodeIOStatus(&ioStat));
        }
    }
    else
    {
        if(argc > 2)
        {
            DclPrintf("Unrecognized option \"%s\"\n", argv[2]);
            return DCLSTAT_BADPARAMETER;
        }
        
        if(pFXDB->DevInfo.nLockFlags & FFXLOCKFLAGS_LOCKALLBLOCKS)
        {
            DclPrintf("CAUTION!  This type of flash only supports locking all the blocks in a\n");
            DclPrintf("          flash chip.  This means that blocks in reserved areas, and\n");
            DclPrintf("          potentially blocks which are mapped as to other Disks will be\n");
            DclPrintf("          affected.  This command is NOT BBM aware.\n\n");
        }

        FMLLOCK_BLOCKS(pFXDB->hFML, ulBlockNum, ulCount, ioStat);
        if(ioStat.ffxStat != FFXSTAT_SUCCESS)
            DclPrintf("Error locking blocks, %s\n", FfxDecodeIOStatus(&ioStat));
        else
            DclPrintf("Successfully locked %lU blocks\n", ioStat.ulCount);
    }

    return ioStat.ffxStat;

  DisplayHelp:

    DclPrintf("This command locks one or more blocks of flash.\n\n");
    DclPrintf("Syntax:  %s /All | StartBlock[:Count] [/Freeze]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /All       - Lock all the blocks in the chip.  If the flash characteristics\n");
    DclPrintf("               indicate that locking all blocks is the only mode supported,\n");
    DclPrintf("               then this option is required, rather than specifying a range.\n");
    DclPrintf("  StartBlock - The starting block number, relative to 0, within the Disk.\n");
    DclPrintf("  Count      - The optional number of blocks to lock.  Defaults to 1 if not\n");
    DclPrintf("               specified.\n");
    DclPrintf("  /Freeze    - This flag indicates that the current locked/unlocked state for\n");
    DclPrintf("               the blocks should be frozen.  This option is only available if\n");
    DclPrintf("               the flash characterics define it as a capability.\n\n");
    DclPrintf("NOTE:  This command is NOT BBM aware.\n\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLBlockUnlock()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLBlockUnlock(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulBlockNum;
    D_UINT32        ulCount = 1;
    FFXIOSTATUS     ioStat;
    unsigned        fInvert = FALSE;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || argc > 3 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(!(pFXDB->DevInfo.uDeviceFlags & DEV_LOCKABLE))
    {
        DclPrintf("The Device does not support lock/unlock commands\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

    if(DclStrICmp(argv[1], "/All") == 0)
    {
        ulBlockNum = 0;
        ulCount = pFXDB->ffi.ulTotalBlocks;
    }
    else
    {
        pStr = DclNtoUL(argv[1], &ulBlockNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }
    }

    if(argc == 3)
    {
        if(DclStrICmp(argv[2], "/Invert") == 0)
        {
            if(!(pFXDB->DevInfo.nLockFlags & FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
            {
                DclPrintf("The Device does not support the /Invert option\n\n");
                return FFXSTAT_UNSUPPORTEDREQUEST;
            }

            fInvert = TRUE;
        }
        else
        {
            DclPrintf("Unrecognized option \"%s\"\n\n", argv[2]);
            return FFXSTAT_UNSUPPORTEDREQUEST;
        }
    }

    if((pFXDB->DevInfo.nLockFlags & FFXLOCKFLAGS_UNLOCKSINGLERANGE) ||
        (pFXDB->DevInfo.nLockFlags & FFXLOCKFLAGS_UNLOCKINVERTEDRANGE))
    {
        DclPrintf("CAUTION!  This type of flash only supports unlocking a single or inverted\n");
        DclPrintf("          range of flash.  This means that any other unlocked blocks will\n");
        DclPrintf("          become locked, including blocks in reserved areas, and potentially\n");
        DclPrintf("          blocks which are mapped as to other Disks.  This command is NOT\n");
        DclPrintf("          BBM aware.\n\n");
    }

    FMLUNLOCK_BLOCKS(pFXDB->hFML, ulBlockNum, ulCount, fInvert, ioStat);
    if(ioStat.ffxStat != FFXSTAT_SUCCESS)
    {
        /*  Note that we did NOT use the IOSUCCESS() macro to check for
            error sinces this operation may legitimately unlock more
            blocks than were requested if the block locking operations
            have to be chip-wide, rather than Disk-wide.
        */
        DclPrintf("Error unlocking blocks, %s\n", FfxDecodeIOStatus(&ioStat));
    }

    return ioStat.ffxStat;

  DisplayHelp:

    DclPrintf("This command unlocks one or more blocks of flash.\n\n");
    DclPrintf("Syntax:  %s /All | StartBlock[:Count] [/Invert]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  /All       - Unlock all the blocks in the chip.\n");
    DclPrintf("  StartBlock - The starting block number, relative to 0, within the Disk.\n");
    DclPrintf("  Count      - The optional number of blocks to unlock.  Defaults to 1 if not\n");
    DclPrintf("               specified.\n");
    DclPrintf("  /Invert    - This flag indicates that all blocks <except> the specified range\n");
    DclPrintf("               should be unlocked.  This option is only available if the flash\n");
    DclPrintf("               characterics define it as a capability.  This option may not be\n");
    DclPrintf("               used in combination with the /All option.\n");
    DclPrintf("NOTE:  This command is NOT BBM aware.\n\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLBlockStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLBlockStatus(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        const char     *pStr;
        D_UINT32        ulBlockNum;
        D_UINT32        ulCount = 1;
        D_UINT32        ulProcessed = 0;
        FFXIOSTATUS     ioStat;
        D_BOOL          fVerbose = FALSE;
        D_BOOL          fIgnoreErrors = FALSE;
        int             argnum;
        D_UINT32        ulScannedGood = 0;
        D_UINT32        ulScannedBad = 0;
        D_UINT32        ulScannedError = 0;
        D_UINT32        ulTotalDevLocked = 0;
        D_UINT32        ulTotalSoftLocked = 0;

        pStr = DclNtoUL(argv[1], &ulBlockNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        for(argnum=2; argnum<argc; argnum++)
        {
            if(DclStrICmp(argv[argnum], "/V") == 0)
            {
                fVerbose = TRUE;

                continue;
            }
            else if(DclStrICmp(argv[argnum], "/I") == 0)
            {
                fIgnoreErrors = TRUE;

                continue;
            }
            else
            {
                DclPrintf("Unrecognized option: \"%s\"\n", argv[argnum]);
                return FFXSTAT_BADSYNTAX;
            }
        }

        for(; ulCount; ulBlockNum++, ulCount--)
        {
            char        szType[24];
            unsigned    fDevLocked;
            unsigned    fSoftLocked;

            ulProcessed++;

            FML_GET_BLOCK_STATUS(pFXDB->hFML, ulBlockNum, ioStat);
            if(!IOSUCCESS(ioStat, 1))
            {
                ulScannedError++;

                DclPrintf("Failed to get status for block %lU, %s\n", ulBlockNum, FfxDecodeIOStatus(&ioStat));

                if(fIgnoreErrors)
                    continue;
                else
                    break;
            }

            fDevLocked = ioStat.op.ulBlockStatus & BLOCKSTATUS_DEV_LOCKED;
            if(fDevLocked)
                ulTotalDevLocked++;

            fSoftLocked = ioStat.op.ulBlockStatus & BLOCKSTATUS_SOFT_LOCKED;
            if(fSoftLocked)
                ulTotalSoftLocked++;

            switch(ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE)
            {
                case BLOCKSTATUS_NOTBAD:
                    ulScannedGood++;
                    if(fVerbose)
                        DclStrCpy(szType, "Good");
                    else
                        DclStrCpy(szType, "");
                    break;
                case BLOCKSTATUS_FACTORYBAD:
                    ulScannedBad++;
                    DclStrCpy(szType, "FactoryBad");
                    break;
                case BLOCKSTATUS_LEGACYNOTBAD:
                    ulScannedBad++;
                    DclStrCpy(szType, "LegacyNotBad");
                    break;
                default:
                    DclError();
                    DclStrCpy(szType, "Unrecognized");
                    break;
            }

            if(szType[0])
            {
                DclPrintf("Block Status for %4lU: %16s - %s%s(%lX)\n",
                    ulBlockNum, szType,
                    fDevLocked ? "DEVLOCKED " : " ",
                    fSoftLocked ? "SOFTLOCKED " : " ",
                    ioStat.op.ulBlockStatus);
            }
        }

        DclPrintf("Processed %lU blocks:  Good=%lU Bad=%lU DevLocked=%lU SoftLocked=%lU Error=%lU\n",
            ulProcessed, ulScannedGood, ulScannedBad, ulTotalDevLocked, ulTotalSoftLocked, ulScannedError);

        ffxStat = ioStat.ffxStat;
    }
  #endif

    return ffxStat;

  DisplayHelp:

    DclPrintf("This is a NAND specific command gets the block status for one or more erase\n");
    DclPrintf("blocks.\n\n");
    DclPrintf("Syntax:  %s StartBlock[:Count] [/V] [/I]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartBlock - The starting block number, relative to 0, within the disk.\n");
    DclPrintf("  Count      - The optional number of blocks to process.  Defaults to 1 if not\n");
    DclPrintf("               specified.\n");
    DclPrintf("  /I         - This optional switch causes any errors to be ignored, and the\n");
    DclPrintf("               subsequent block, if any, is processed.\n");
    DclPrintf("  /V         - This optional switch causes the block status to be displayed for\n");
    DclPrintf("               every block in the range.  If this option is not used, the block\n");
    DclPrintf("               status is display only for those blocks which are NOT good.\n\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLBlockRawStatus()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLBlockRawStatus(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;
    FFXSTATUS       ffxStat = FFXSTAT_SUCCESS;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        const char     *pStr;
        D_UINT32        ulBlockNum;
        D_UINT32        ulCount = 1;
        D_UINT32        ulProcessed = 0;

        /*  The logic around fSet and ulBlockStatus is sufficiently confusing
            to some compilers (gcc 4.1.2, at least) that they will report
            that ulBlockStatus may be used uninitialized.  To placate the
            compiler yet avoid creating a booby trap, initialize this to
            something the block status cannot be legitimately set to.  It
            might be preferable to rearrange the logic so the compiler can
            prove the variable is always initialized instead of just
            silencing the warning.
        */
        D_UINT32        ulBlockStatus = D_UINT32_MAX;
        FFXIOSTATUS     ioStat;
        D_BOOL          fSet = FALSE;
        D_BOOL          fVerbose = FALSE;
        D_BOOL          fIgnoreErrors = FALSE;
        int             argnum;
        D_UINT32        ulScannedGood = 0;
        D_UINT32        ulScannedBad = 0;
        D_UINT32        ulScannedError = 0;
        D_UINT32        ulSetGood = 0;
        D_UINT32        ulSetBad = 0;
        D_UINT32        ulSetError = 0;
        D_UINT32        ulTotalDevLocked = 0;
        D_UINT32        ulTotalSoftLocked = 0;

        pStr = DclNtoUL(argv[1], &ulBlockNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        for(argnum=2; argnum<argc; argnum++)
        {
            if(DclStrICmp(argv[argnum], "/Set:Good") == 0)
            {
                ulBlockStatus = BLOCKSTATUS_NOTBAD;
                fSet = TRUE;

                continue;
            }
            else if(DclStrICmp(argv[argnum], "/Set:FactoryBad") == 0)
            {
                ulBlockStatus = BLOCKSTATUS_FACTORYBAD;
                fSet = TRUE;

                continue;
            }
            else if(DclStrICmp(argv[argnum], "/V") == 0)
            {
                fVerbose = TRUE;

                continue;
            }
            else if(DclStrICmp(argv[argnum], "/I") == 0)
            {
                fIgnoreErrors = TRUE;

                continue;
            }
            else
            {
                DclPrintf("Unrecognized option: \"%s\"\n", argv[argnum]);
                return FFXSTAT_BADSYNTAX;
            }
        }

        for(; ulCount; ulBlockNum++, ulCount--)
        {
            char        szType[24];
            unsigned    fDevLocked;
            unsigned    fSoftLocked;

            ulProcessed++;

            FML_GET_RAW_BLOCK_STATUS(pFXDB->hFML, ulBlockNum, ioStat);
            if(!IOSUCCESS(ioStat, 1))
            {
                ulScannedError++;

                DclPrintf("Failed to get status for block %lU, %s\n", ulBlockNum, FfxDecodeIOStatus(&ioStat));

                if(fIgnoreErrors)
                    continue;
                else
                    break;
            }

            fDevLocked = ioStat.op.ulBlockStatus & BLOCKSTATUS_DEV_LOCKED;
            if(fDevLocked)
                ulTotalDevLocked++;

            fSoftLocked = ioStat.op.ulBlockStatus & BLOCKSTATUS_SOFT_LOCKED;
            if(fSoftLocked)
                ulTotalSoftLocked++;

            switch(ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE)
            {
                case BLOCKSTATUS_NOTBAD:
                    ulScannedGood++;
                    if(fVerbose)
                        DclStrCpy(szType, "Good");
                    else
                        DclStrCpy(szType, "");
                    break;
                 case BLOCKSTATUS_FACTORYBAD:
                    ulScannedBad++;
                    DclStrCpy(szType, "FactoryBad");
                    break;
                case BLOCKSTATUS_LEGACYNOTBAD:
                    ulScannedBad++;
                    DclStrCpy(szType, "LegacyNotBad");
                    break;
                 default:
                    DclError();
                    DclStrCpy(szType, "Unrecognized");
                    break;
            }

            if(szType[0])
            {
                DclPrintf("Block Status for %4lU: %16s - %s%s(%lX)\n",
                    ulBlockNum, szType,
                    fDevLocked ? "DEVLOCKED " : " ",
                    fSoftLocked ? "SOFTLOCKED " : " ",
                    ioStat.op.ulBlockStatus);
            }

            if(fSet)
            {
                if((ioStat.op.ulBlockStatus & BLOCKSTATUS_MASKTYPE) != ulBlockStatus)
                {
                    FML_SET_RAW_BLOCK_STATUS(pFXDB->hFML, ulBlockNum, ulBlockStatus, ioStat);
                    if(!IOSUCCESS(ioStat, 1))
                    {
                        ulSetError++;

                        DclPrintf("Failed to set status for block %lU, %s\n", ulBlockNum, FfxDecodeIOStatus(&ioStat));

                        if(fIgnoreErrors)
                            continue;
                        else
                            break;
                     }

                    switch(ulBlockStatus)
                    {
                        case BLOCKSTATUS_NOTBAD:
                            ulSetGood++;
                            DclStrCpy(szType, "Good");
                            break;
                        case BLOCKSTATUS_FACTORYBAD:
                            ulSetBad++;
                            DclStrCpy(szType, "FactoryBad");
                            break;
                        default:
                            DclError();
                            DclStrCpy(szType, "Unrecognized");
                            break;
                    }

                    DclPrintf("           Changed to: %16s (%lU)\n", szType, ulBlockStatus);
                }
            }
        }

        DclPrintf("Processed %lU blocks, original status:  Good=%lU Bad=%lU DevLocked=%lU SoftLocked=%lU Error=%lU\n",
            ulProcessed, ulScannedGood, ulScannedBad, ulTotalDevLocked, ulTotalSoftLocked, ulScannedError);

        if(fSet)
        {
            DclPrintf("New status:  Good=%lU Bad=%lU Error=%lU\n",
                ulSetGood, ulSetBad, ulSetError);
        }

        ffxStat = ioStat.ffxStat;
    }
  #endif

    return ffxStat;

  DisplayHelp:

    DclPrintf("This is a NAND specific command gets or sets the block status for one or more\n");
    DclPrintf("physical erase blocks.\n\n");
    DclPrintf("Syntax:  %s StartBlock[:Count] [/V] [/I] [/Set:type]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartBlock - The starting block number, relative to 0, within the disk.\n");
    DclPrintf("  Count      - The optional number of blocks to process.  Defaults to 1 if not\n");
    DclPrintf("               specified.\n");
    DclPrintf("  /I         - This optional switch causes any errors to be ignored, and the\n");
    DclPrintf("               subsequent block, if any, is processed.\n");
    DclPrintf("  /V         - This optional switch causes the block status to be displayed for\n");
    DclPrintf("               every block in the range.  If this option is not used, the block\n");
    DclPrintf("               status is display only for those blocks which are NOT good.\n");
    DclPrintf("  /Set:type  - This optional switch (powerful and DANGEROUS) can be used to\n");
    DclPrintf("               change the bad-block status for a range of blocks.  This option\n");
    DclPrintf("               MUST be used with EXTREME caution, as it may destroy any data in\n");
    DclPrintf("               the flash, and could violate the NAND manufacturer's programming\n");
    DclPrintf("               guidelines.  After using this option, a BBM format will need to\n");
    DclPrintf("               be performed to cause BBM to repopulate its bad block table.\n");
    DclPrintf("               Some NTMs may not support all the functionality described here,\n");
    DclPrintf("               and will return an error in that event.  The following 'type'\n");
    DclPrintf("               values are available:\n");
    DclPrintf("            GOOD - Mark the blocks as being good.  To accomplish this, it may\n");
    DclPrintf("                   be necessary to erase the entire block!  Use with caution!\n");
    DclPrintf("      FACTORYBAD - Mark the blocks so they appear to be original factory-bad\n");
    DclPrintf("                   blocks.\n");
    DclPrintf("NOTE: This function operates on physical erase blocks, not logical erase blocks\n");
    DclPrintf("      as remapped by BBM.  If FlashFX is using a 'raw' Disk mapping, which may\n");
    DclPrintf("      include the BBM area, then this function will be able to see and touch\n");
    DclPrintf("      even those blocks which are being managed by BBM.\n\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLErase()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLErase(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    const char     *pStr;
    D_UINT32        ulBlockNum;
    D_UINT32        ulCount = 1;
    FFXIOSTATUS     ioStat;
    DCLSHELLPARAMS  params;
    FXDB           *pFXDB;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc != 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    pStr = DclNtoUL(argv[1], &ulBlockNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    FMLERASE_BLOCKS(pFXDB->hFML, ulBlockNum, ulCount, ioStat);
    if(!IOSUCCESS(ioStat, ulCount))
    {
        DclPrintf("Erased %lU blocks, %s\n", ioStat.ulCount, FfxDecodeIOStatus(&ioStat));
        return ioStat.ffxStat;
    }
    else
    {
        DclPrintf("Erase successful\n");
        return FFXSTAT_SUCCESS;
    }

  DisplayHelp:

    DclPrintf("This command erases one or more flash blocks.\n\n");
    DclPrintf("Syntax:  %s StartBlock[:Count]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartBlock - The starting block number, relative to 0, within the\n");
    DclPrintf("               disk to erase.\n");
    DclPrintf("  Count      - The optional number of blocks to erase.  Defaults\n");
    DclPrintf("               to 1 if not specified.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLRead()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLRead(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulPageNum;
    D_UINT32        ulCount = 1;
    D_UINT32        ulTagWidth;
    char            szDescription[MAX_DESCRIPTION_LEN];
    FFXIOSTATUS     ioStat;
    D_BUFFER       *pBuff;
    D_BUFFER       *pSpare;
    int             argnum;
    unsigned        fRaw = FALSE;
    unsigned        fSpares = FALSE;
    unsigned        fTags = FALSE;
    DCLSHELLPARAMS  params;
    unsigned        fGotArg;
    unsigned        nElements;
    unsigned        nBufferNum;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    argnum = 1;

    pStr = DclNtoUL(argv[argnum], &ulPageNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    argnum++;

    if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
    {
        argnum++;
    }

    if(!ParsePageBuffName(pFXDB, argnum < argc ? argv[argnum] : NULL, &pBuff, 
                          szDescription, &nElements, &nBufferNum, &fGotArg))
    {
        DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
        return FFXSTAT_BADSYNTAX;
    }

    if(fGotArg)
        argnum++;

    /*  Default buffers
    */
    pSpare = pFXDB->pSB[nBufferNum];

    /*  Loop through any remaining optional arguments and process them
    */
    for(; argnum<argc; argnum++)
    {
        if(DclStrNICmp(argv[argnum], "/TAGS", 5) == 0)
        {
            fTags = TRUE;

            if(argv[argnum][5] == ':')
            {
                pStr = DclNtoUL(&argv[argnum][6], &ulTagWidth);
                if(!pStr || *pStr != 0)
                {
                    DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
                    return FFXSTAT_BADSYNTAX;
                }
            }
            else
            {
                DclAssert(argv[argnum][5] == 0);
                ulTagWidth = FFX_NAND_TAGSIZE;
            }

            if((ulTagWidth > pFXDB->DevInfo.uMetaSize) ||
               ((ulTagWidth != pFXDB->DevInfo.uMetaSize) &&
               !(pFXDB->DevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)))
            {
                DclPrintf("The tag width value (%lU) is invalid or out of range\n", ulTagWidth);
                return FFXSTAT_BADSYNTAX;
            }

            continue;
        }

        if(DclStrICmp(argv[argnum], "/SPARES") == 0)
        {
            fSpares = TRUE;
            continue;
        }

        if(DclStrICmp(argv[argnum], "/RAW") == 0)
        {
            fRaw = TRUE;
            continue;
        }

        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
        goto DisplayHelp;
    }

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND && (fRaw || fTags || fSpares))
    {
        DclPrintf("The /TAGS, /RAW, and /SPARE options are only valid when using NAND\n\n");
        goto DisplayHelp;
    }

    if(fRaw + fTags > 1)
    {
        DclPrintf("Only one of the /TAGS and /RAW options may be used at a time\n\n");
        goto DisplayHelp;
    }

    while(ulCount)
    {
        D_BUFFER   *pTmpBuff;
        D_BUFFER   *pTmpSpare;
        D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

        DclPrintf("Reading %lU pages starting at flash page %lU into %s\n",
            ulThisCount, ulPageNum, szDescription);

      #if FFXCONF_NANDSUPPORT
        if(fTags)
        {
            FMLREAD_TAGGEDPAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, pSpare, (unsigned)ulTagWidth, ioStat);
        }
        else if(fRaw)
        {
            if(!fSpares)
                pSpare = NULL;

            FMLREAD_RAWPAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, pSpare, ioStat);
        }
        else
      #endif
        {
          #if FFXCONF_NANDSUPPORT
            if(fSpares)
            {
                FMLREAD_SPARES(pFXDB->hFML, ulPageNum, ulThisCount, pSpare, ioStat);
                if(!IOSUCCESS(ioStat, 1))
                {
                    DclPrintf("Failed to read spare area for page %lU, %s\n", ulPageNum, FfxDecodeIOStatus(&ioStat));
                    break;
                }
            }
            else
            {
                pSpare = NULL;
            }
          #endif

            FMLREAD_PAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, ioStat);
        }

        if(!IOSUCCESS(ioStat, ulThisCount))
        {
            DclPrintf("Failed to read %lU pages, starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
            break;
        }

        /*  Display on the console...
        */
        pTmpBuff = pBuff;
        pTmpSpare = pSpare;

        while(ulThisCount)
        {
            DclPrintf("Page %lU:\n", ulPageNum);
            DclHexDump(NULL, HEXDUMP_UINT8,
                (D_UINT16)CalculateHexDumpWidth(params.nWidth, 1), pFXDB->ffi.uPageSize, pTmpBuff);

            if(fTags)
            {
                DclHexDump("Tag:  ", HEXDUMP_UINT8|HEXDUMP_NOOFFSET, 16, ulTagWidth, pTmpSpare);
                pTmpSpare += ulTagWidth;
            }
            else if(pSpare)
            {
                DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, pFXDB->ffi.uSpareSize, pTmpSpare);
                pTmpSpare += pFXDB->ffi.uSpareSize;
            }

            pTmpBuff += pFXDB->ffi.uPageSize;
            ulPageNum++;
            ulThisCount--;
            ulCount--;
        }
        /*  Display the I/O status only if something unusual happened.
        */
        if(ioStat.op.ulPageStatus != 0)
            DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command reads one or more pages from flash, using FML interfaces.\n\n");
    DclPrintf("Syntax:  %s StartPage[:Count] [to] [Buffer] [/TAGS] [/SPARES] [/RAW]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartPage  - The starting page number to read within the disk, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of pages to read.  If this is greater than 1,\n");
    DclPrintf("               then sequential Page Buffers will used.  If the count exceeds the\n");
    DclPrintf("               count of remaining Page Buffers relative to the start, the reads\n");
    DclPrintf("               will wrap back to the specified starting buffer.  Defaults to 1.\n");
    DclPrintf("  Buffer     - The Page Buffer (PBn) into which data will be read.  Defaults to\n");
    DclPrintf("               PB1.\n");
    DclPrintf("  NAND Only Options:\n");
    DclPrintf("  /TAGS[:n]  - Reads the tag data as well.  If 'n' is not specified, the default\n");
    DclPrintf("               tag width of %u will be used.  Note that non-default values for\n", FFX_NAND_TAGSIZE);
    DclPrintf("               'n' are supported only by select NTMs.\n");
    DclPrintf("  /SPARES    - Reads the spare area data as well\n");
    DclPrintf("  /RAW       - Reads raw, uncorrected main page and spare area data.  This\n");
    DclPrintf("               functionality may not be supported by all NTMs\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLWrite()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLWrite(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulPageNum;
    D_UINT32        ulCount = 1;
    D_UINT32        ulTagWidth;
    char            szDescription[MAX_DESCRIPTION_LEN];
    FFXIOSTATUS     ioStat;
    D_BUFFER       *pBuff;
  #if FFXCONF_NANDSUPPORT
    D_BUFFER       *pSpare;
  #endif
    int             argnum;
    unsigned        fRaw = FALSE;
    unsigned        fSpares = FALSE;
    unsigned        fTags = FALSE;
    DCLSHELLPARAMS  params;
    unsigned        fGotArg;
    unsigned        nElements;
    unsigned        nBufferNum;
    D_BUFFER       *pVerify = NULL;
    DCLSTATUS       dclStat;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    argnum = 1;

    if(!ParsePageBuffName(pFXDB, argv[argnum], &pBuff, szDescription, &nElements, &nBufferNum, &fGotArg))
    {
        DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
        return FFXSTAT_BADSYNTAX;
    }

    if(fGotArg)
        argnum++;

  #if FFXCONF_NANDSUPPORT
    /*  Default buffers
    */
    pSpare = pFXDB->pSB[nBufferNum];
  #endif
  
    if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
    {
        argnum++;
    }

    if(argnum >= argc)
    {
        DclPrintf("Syntax error!\n\n");
        goto DisplayHelp;
    }

    pStr = DclNtoUL(argv[argnum], &ulPageNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    argnum++;

    /*  Loop through any remaining optional arguments and process them
    */
    for(; argnum<argc; argnum++)
    {
        if(DclStrNICmp(argv[argnum], "/TAGS", 5) == 0)
        {
            fTags = TRUE;

            if(argv[argnum][5] == ':')
            {
                pStr = DclNtoUL(&argv[argnum][6], &ulTagWidth);
                if(!pStr || *pStr != 0)
                {
                    DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
                    return FFXSTAT_BADSYNTAX;
                }
            }
            else
            {
                DclAssert(argv[argnum][5] == 0);
                ulTagWidth = FFX_NAND_TAGSIZE;
            }

            if((ulTagWidth > pFXDB->DevInfo.uMetaSize) ||
               ((ulTagWidth != pFXDB->DevInfo.uMetaSize) &&
               !(pFXDB->DevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)))
            {
                DclPrintf("The tag width value (%lU) is invalid or out of range\n", ulTagWidth);
                return FFXSTAT_BADSYNTAX;
            }

            continue;
        }

        if(DclStrICmp(argv[argnum], "/SPARES") == 0)
        {
             fSpares = TRUE;
             continue;
        }

        if(DclStrICmp(argv[argnum], "/RAW") == 0)
        {
             fRaw = TRUE;
             continue;
        }

        if(DclStrICmp(argv[argnum], "/V") == 0)
        {
            pVerify = DclMemAlloc(pFXDB->ffi.uPageSize);
            continue;
        }

        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
        goto DisplayHelp;
    }

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND && (fRaw || fTags || fSpares))
    {
        DclPrintf("The /TAGS, /RAW, and /SPARES options are only valid when using NAND\n\n");
        goto DisplayHelp;
    }

    if(fRaw + fTags > 1)
    {
        DclPrintf("Only one of the /TAGS and /RAW options may be used at a time\n\n");
        goto DisplayHelp;
    }

    if((fSpares && fTags) || (fSpares && !fRaw))
    {
        DclPrintf("The /SPARES option may only be used with the /RAW option\n\n");
        goto DisplayHelp;
    }

    while(ulCount)
    {
        D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

        DclPrintf("Writing %lU pages starting with %s to flash page %lU\n",
            ulThisCount, szDescription, ulPageNum);

      #if FFXCONF_NANDSUPPORT
        if(fRaw)
        {
            if(!fSpares)
                pSpare = NULL;

            FMLWRITE_RAWPAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, pSpare, ioStat);
        }
        else
        {
            if(fTags)
            {
                FMLWRITE_TAGGEDPAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, pSpare, (unsigned)ulTagWidth, ioStat);
            }
            else
            {
                FMLWRITE_PAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, ioStat);
            }
        }
      #else
        {
            FMLWRITE_PAGES(pFXDB->hFML, ulPageNum, ulThisCount, pBuff, ioStat);
        }
      #endif

        if(!IOSUCCESS(ioStat, ulThisCount))
        {
            DclPrintf("Failed to write %lU pages, starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
            break;
        }

        /*  Display the I/O status only if something unusual happened.
        */
        if(ioStat.op.ulPageStatus != 0)
            DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));

        if(pVerify)
        {
            if(ulThisCount > 1)
            {
                DclPrintf("Data verification is only allowed when writing one page at a time\n");
            }
            else
            {
                FFXIOSTATUS ioRead;

              #if FFXCONF_NANDSUPPORT
                if(fRaw)
                {
                    FMLREAD_RAWPAGES(pFXDB->hFML, ulPageNum, 1, pVerify, NULL, ioRead);
                }
                else
              #endif
                {
                    FMLREAD_PAGES(pFXDB->hFML, ulPageNum, 1, pVerify, ioRead);
                }

                DclPrintf("Verification IOStatus was %s\n", FfxDecodeIOStatus(&ioRead));

                if(DclMemCmp(pBuff, pVerify, pFXDB->ffi.uPageSize) != 0)
                    DclPrintf("Read-back verify failed!\n");
            }
        }       

        ulPageNum += ulThisCount;
        ulCount -= ulThisCount;
    }

    dclStat = FFXSTAT_SUCCESS;

  FmlWriteCleanup:

    if(pVerify)
        DclMemFree(pVerify);

    return dclStat;

  DisplayHelp:
    DclPrintf("This command writes one or more pages to flash, using FML interfaces.\n\n");
    DclPrintf("Syntax:  %s [Buffer] [to] StartPage[:Count] [options]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The starting Page Buffer (PBn) to write.  If not specified, PB1\n");
    DclPrintf("               will be used.\n");
    DclPrintf("  StartPage  - The starting page number to write to the disk, relative to zero.\n");
    DclPrintf("  Count      - The optional number of pages to write.  If this is greater than 1,\n");
    DclPrintf("               then sequential Page Buffers will used.  If the count exceeds the\n");
    DclPrintf("               count of remaining Page Buffers relative to the start, the writes\n");
    DclPrintf("               will wrap back to the specified starting buffer.  Defaults to 1.\n");
    DclPrintf("  /V         - Verify the main page data by reading it back.  This option is only\n");
    DclPrintf("               functional when Count is 1.\n");
    DclPrintf("  NAND Only Options:\n");
    DclPrintf("  /TAGS[:n]  - Writes a tag as well.  The tag data will be obtained from the\n");
    DclPrintf("               first 'n' bytes of the corresponding Spare Buffer.  If 'n' is not\n");
    DclPrintf("               specified, the default tag width of %u will be used.  Note that\n", FFX_NAND_TAGSIZE);
    DclPrintf("               non-default values for 'n' are supported only by select NTMs.\n");
    DclPrintf("  /RAW       - Writes raw, uncorrected main page data.  This functionality may\n");
    DclPrintf("               not be supported by all NTMs.\n");
    DclPrintf("  /SPARES    - Causes the /RAW option to write the corresponding Spare Buffer\n");
    DclPrintf("               data as well.\n");

    dclStat = FFXSTAT_BADSYNTAX;
    
    goto FmlWriteCleanup;
}


/*-------------------------------------------------------------------
    Local: DebugFMLSpareRead()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLSpareRead(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        unsigned        nElements;
        unsigned        fGotArg;
        const char     *pStr;
        D_UINT32        ulPageNum;
        D_UINT32        ulCount = 1;
        FFXIOSTATUS     ioStat;
        D_BUFFER       *pSpare;
        int             argnum = 1;

        pStr = DclNtoUL(argv[argnum], &ulPageNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        argnum++;

        if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
        {
            argnum++;
        }

        if(!ParseSpareBuffName(pFXDB, argv[argnum], &pSpare, NULL, &nElements, &fGotArg))
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
            return FFXSTAT_BADSYNTAX;
        }

        if(fGotArg)
            argnum++;

        if(argnum < argc)
        {
            DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
            goto DisplayHelp;
        }

        while(ulCount)
        {
            D_BUFFER   *pTmpSpare;
            D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

            FMLREAD_SPARES(pFXDB->hFML, ulPageNum, ulThisCount, pSpare, ioStat);
            if(!IOSUCCESS(ioStat, ulThisCount))
            {
                DclPrintf("Failed to read %lU spare areas starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
                break;
            }

            pTmpSpare = pSpare;

            while(ulThisCount)
            {
                /*  Display on the console...
                */
                DclPrintf("Spare Area for Page %lU:\n", ulPageNum);
                DclHexDump(NULL, HEXDUMP_UINT8, 16, pFXDB->ffi.uSpareSize, pTmpSpare);

                pTmpSpare += pFXDB->ffi.uSpareSize;
                ulThisCount--;
                ulCount--;
                ulPageNum++;
            }

            /*  Display the I/O status only if something unusual happened.
            */
            if(ioStat.op.ulPageStatus != 0)
                DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));
        }

        return FFXSTAT_SUCCESS;
    }
  #endif

  DisplayHelp:

    DclPrintf("This is a NAND specific command reads one or more spare areas from flash, using\n");
    DclPrintf("FML interfaces.\n\n");
    DclPrintf("Syntax:  %s StartPage[:Count] [to] [Buffer]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartPage  - The starting page number to read within the disk, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of spares to read.  If this is greater than 1, then\n");
    DclPrintf("               sequential Spare Buffers will be used.  If count exceeds the number of\n");
    DclPrintf("               buffers remaining, the specified starting buffer will be re-used.\n");
    DclPrintf("               Defaults to 1 if not specified.\n");
    DclPrintf("  Buffer     - The Spare Buffer (SBn) into which data will be read.  Defaults to SB1.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLSpareWrite()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLSpareWrite(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        unsigned        nElements;
        unsigned        fGotArg;
        const char     *pStr;
        D_UINT32        ulPageNum;
        D_UINT32        ulCount = 1;
        FFXIOSTATUS     ioStat;
        D_BUFFER       *pSpare;
        int             argnum = 1;

        if(!ParseSpareBuffName(pFXDB, argv[argnum], &pSpare, NULL, &nElements, &fGotArg))
        {
            DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
            return FFXSTAT_BADSYNTAX;
        }

        if(fGotArg)
            argnum++;

        if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
        {
            argnum++;
        }

        if(argnum >= argc)
        {
            DclPrintf("Syntax error!\n\n");
            goto DisplayHelp;
        }

        pStr = DclNtoUL(argv[argnum], &ulPageNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        argnum++;

        if(argnum < argc)
        {
            DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
            goto DisplayHelp;
        }

        while(ulCount)
        {
            D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

            FMLWRITE_SPARES(pFXDB->hFML, ulPageNum, ulThisCount, pSpare, ioStat);
            if(!IOSUCCESS(ioStat, ulThisCount))
            {
                DclPrintf("Failed to write %lU spare areas starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
                break;
            }

            /*  Display the I/O status only if something unusual happened.
            */
            if(ioStat.op.ulPageStatus != 0)
                DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));

            ulPageNum += ulThisCount;
            ulCount -= ulThisCount;
        }

        return FFXSTAT_SUCCESS;
    }
  #endif

  DisplayHelp:

    DclPrintf("This is a NAND specific command writes one or more spare areas to flash, using\n");
    DclPrintf("FML interfaces.\n\n");
    DclPrintf("Syntax:  %s [Buffer] [to] StartPage[:Count]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The starting Spare Buffer (SBn) from which data will be written.\n");
    DclPrintf("               Defaults to SB1.\n");
    DclPrintf("  StartPage  - The starting page number to write to the disk, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of spares to write.  If this is greater than 1, then\n");
    DclPrintf("               sequential Spare Buffers will be used.  If count exceeds the number of\n");
    DclPrintf("               buffers remaining, the specified starting buffer will be re-used.\n");
    DclPrintf("               Defaults to 1 if not specified.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLTagRead()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLTagRead(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        const char     *pStr;
        char            szDescription[MAX_DESCRIPTION_LEN];
        D_UINT32        ulPageNum;
        D_UINT32        ulCount = 1;
        D_UINT32        ulTagWidth = FFX_NAND_TAGSIZE;
        FFXIOSTATUS     ioStat;
        D_BUFFER       *pBuff;
        int             argnum = 1;
        D_UINT16        uMaxWidth;

        pStr = DclNtoUL(argv[argnum], &ulPageNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        argnum++;

        if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
        {
            argnum++;
        }

        if(argnum < argc)
        {
            if(!ParseBufferName(pFXDB, argv[argnum], &pBuff, szDescription, &uMaxWidth, NULL))
            {
                DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
                goto DisplayHelp;
            }

            argnum++;
        }
        else
        {
            /*  If the event that there are no more arguments, call this function
                with a NULL pointer will initialize the default arguments.
            */
            ParseSpareBuffName(pFXDB, NULL, &pBuff, szDescription, NULL, NULL);
            uMaxWidth = pFXDB->ffi.uSpareSize;
        }

        if(argnum < argc)
        {
            if(DclStrNICmp(argv[argnum], "/SIZE:", 6) == 0)
            {
                pStr = DclNtoUL(&argv[argnum][6], &ulTagWidth);
                if(!pStr || *pStr != 0)
                {
                    DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
                    return FFXSTAT_BADSYNTAX;
                }
            }
            else
            {
                DclPrintf("Unrecognized option \"%s\"\n\n", argv[argnum]);
                return FFXSTAT_BADSYNTAX;
            }
        }

        if((ulTagWidth > pFXDB->DevInfo.uMetaSize) ||
           ((ulTagWidth != pFXDB->DevInfo.uMetaSize) &&
           !(pFXDB->DevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)))
        {
            DclPrintf("The tag width value (%lU) is invalid or out of range\n", ulTagWidth);
            return FFXSTAT_BADSYNTAX;
        }

        if(ulCount > (D_UINT32)(uMaxWidth / ulTagWidth))
        {
            DclPrintf("Truncating the count to the maximum number of tags which will fit in the specified buffer\n");
            ulCount = uMaxWidth / ulTagWidth;
        }

        FMLREAD_TAGS(pFXDB->hFML, ulPageNum, ulCount, pBuff, (unsigned)ulTagWidth, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("Tag read failed, %s\n", FfxDecodeIOStatus(&ioStat));
            return FFXSTAT_FAILURE;
        }

        DclPrintf("Successfully read %lU tag(s) into %s\n", ulCount, szDescription);

        while(ulCount)
        {
            if(ulTagWidth == 2)
            {
                DclPrintf("    %X\n", *(D_UINT16*)pBuff);
            }
            else if(ulTagWidth == 4)
            {
                DclPrintf("    %lX\n", *(D_UINT32*)pBuff);
            }
            else
            {
                DclHexDump("    ", HEXDUMP_UINT8 | HEXDUMP_NOOFFSET | HEXDUMP_NOASCII,
                    (D_UINT16)ulTagWidth, ulTagWidth, pBuff);
            }

            pBuff += ulTagWidth;

            ulCount--;
        }

        return FFXSTAT_SUCCESS;
    }
  #endif

  DisplayHelp:

    DclPrintf("This is a NAND specific command which reads one or more tags from flash, using\n");
    DclPrintf("the FML interface.\n\n");
    DclPrintf("Syntax:  %s StartPage[:Count] [to] [Buffer] [/Size:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartPage  - The starting page number from which to read the tag, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of tags to read.  Depending on the target\n");
    DclPrintf("               buffer specified, this value may be truncated if necessary to\n");
    DclPrintf("               avoid overflow.  Defaults to 1 if not specified.\n");
    DclPrintf("  Buffer     - The Buffer into which data will be read, either PBn or SBn.  If\n");
    DclPrintf("               not specified, SB1 will be used.\n");
    DclPrintf("  /Size:n    - The tag size to use.  If not specified, the default tag size\n");
    DclPrintf("               of %u will be used.  Note that non-default values for 'n' are\n", FFX_NAND_TAGSIZE);
    DclPrintf("               supported only by select NTMs.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugFMLTagWrite()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugFMLTagWrite(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(pFXDB->ffi.uDeviceType != DEVTYPE_NAND)
    {
        DclPrintf("This command is only valid on NAND flash\n\n");
        return FFXSTAT_UNSUPPORTEDREQUEST;
    }

  #if FFXCONF_NANDSUPPORT
    {
        const char     *pStr;
        char            szDescription[MAX_DESCRIPTION_LEN];
        D_UINT32        ulPageNum;
        D_UINT32        ulCount = 1;
        D_UINT32        ulTagWidth = FFX_NAND_TAGSIZE;
        FFXIOSTATUS     ioStat;
        D_BUFFER       *pBuff;
        int             argnum = 1;
        D_UINT16        uMaxWidth;

        if(ParseBufferName(pFXDB, argv[argnum], &pBuff, szDescription, &uMaxWidth, NULL))
        {
            argnum++;
        }
        else
        {
            /*  If the event that there are no more arguments, call this function
                with a NULL pointer will initialize the default arguments.
            */
            ParseSpareBuffName(pFXDB, NULL, &pBuff, szDescription, NULL, NULL);
            uMaxWidth = pFXDB->ffi.uSpareSize;
        }

        if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
        {
            argnum++;
        }

        if(argnum >= argc)
        {
            DclPrintf("Syntax error!\n\n");
            goto DisplayHelp;
        }

        pStr = DclNtoUL(argv[argnum], &ulPageNum);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, &ulCount);
            if(!pStr || *pStr != 0)
                goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
        }
        else
        {
            if(!pStr || *pStr != 0)
                goto DisplayHelp;
        }

        argnum++;

        if(argnum < argc)
        {
            if(DclStrNICmp(argv[argnum], "/SIZE:", 6) == 0)
            {
                pStr = DclNtoUL(&argv[argnum][6], &ulTagWidth);
                if(!pStr || *pStr != 0)
                {
                    DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
                    return FFXSTAT_BADSYNTAX;
                }
            }
            else
            {
                DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
                goto DisplayHelp;
            }
        }

        if((ulTagWidth > pFXDB->DevInfo.uMetaSize) ||
           ((ulTagWidth != pFXDB->DevInfo.uMetaSize) &&
           !(pFXDB->DevInfo.uDeviceFlags & DEV_VARIABLE_LENGTH_TAGS)))
        {
            DclPrintf("The tag width value (%lU) is invalid or out of range\n", ulTagWidth);
            return FFXSTAT_BADSYNTAX;
        }

        if(ulCount > (D_UINT32)(uMaxWidth / ulTagWidth))
        {
            DclPrintf("Truncating the count to the maximum number of tags which will fit in the specified buffer\n");
            ulCount = uMaxWidth / ulTagWidth;
        }

        FMLWRITE_TAGS(pFXDB->hFML, ulPageNum, ulCount, pBuff, (unsigned)ulTagWidth, ioStat);
        if(!IOSUCCESS(ioStat, ulCount))
        {
            DclPrintf("Tag write failed, %s\n", FfxDecodeIOStatus(&ioStat));
            return FFXSTAT_FAILURE;
        }

        DclPrintf("Successfully wrote %lU tag(s) from %s\n", ulCount, szDescription);

        return FFXSTAT_SUCCESS;
    }
  #endif

  DisplayHelp:

    DclPrintf("This is a NAND specific command which writes one or more tags to flash, using\n");
    DclPrintf("the FML interface.\n\n");
    DclPrintf("Syntax:  %s [Buffer] [to] StartPage[:Count] [/Size:n]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The Buffer which contains the tag data to write, either PBn or SBn.\n");
    DclPrintf("               If not specified, SB1 will be used.\n");
    DclPrintf("  StartPage  - The starting page number which will be modified to contain the tag,\n");
    DclPrintf("               relative to zero.\n");
    DclPrintf("  Count      - The optional number of tags to write.  Depending on the target\n");
    DclPrintf("               buffer specified, this value may be truncated if necessary to\n");
    DclPrintf("               avoid overflow.  Defaults to 1 if not specified.\n");
    DclPrintf("  /Size:n    - The tag size to use.  If not specified, the default tag size\n");
    DclPrintf("               of %u will be used.  Note that non-default values for 'n' are\n", FFX_NAND_TAGSIZE);
    DclPrintf("               supported only by select NTMs.\n");

    return FFXSTAT_BADSYNTAX;
}


#if FFXCONF_ALLOCATORSUPPORT

/*-------------------------------------------------------------------
    Local: DebugAllocRead()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugAllocRead(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulPageNum;
    D_UINT32        ulCount = 1;
    char            szDescription[MAX_DESCRIPTION_LEN];
    FFXIOSTATUS     ioStat;
    D_BUFFER       *pBuff;
    int             argnum;
    DCLSHELLPARAMS  params;
    unsigned        fGotArg;
    unsigned        nElements;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(!pFXDB->hVBF)
    {
        DclPrintf("The allocator is not initialized for this disk\n");
        return FFXSTAT_BADHANDLE;
    }

    argnum = 1;

    pStr = DclNtoUL(argv[argnum], &ulPageNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    argnum++;

    if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
    {
        argnum++;
    }

    if(!ParsePageBuffName(pFXDB, argnum < argc ? argv[argnum] : NULL, &pBuff, 
                          szDescription, &nElements, NULL, &fGotArg))
    {
        DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
        return FFXSTAT_BADSYNTAX;
    }

    if(fGotArg)
        argnum++;

    if(argnum < argc)
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
        goto DisplayHelp;
    }

    while(ulCount)
    {
        D_BUFFER   *pTmpBuff;
        D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

        DclPrintf("Reading %lU pages starting at flash page %lU into %s\n",
            ulThisCount, ulPageNum, szDescription);

        ioStat = FfxVbfReadPages(pFXDB->hVBF, ulPageNum, ulThisCount, pBuff);
        if(!IOSUCCESS(ioStat, ulThisCount))
        {
            DclPrintf("Failed to read %lU pages starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
            break;
        }

        pTmpBuff = pBuff;

        while(ulThisCount)
        {
            /*  Display on the console...
            */
            DclPrintf("Page %lU:\n", ulPageNum);
            DclHexDump(NULL, HEXDUMP_UINT8,
                (D_UINT16)CalculateHexDumpWidth(params.nWidth, 1), pFXDB->VbfInfo.uPageSize, pTmpBuff);

            pTmpBuff += pFXDB->VbfInfo.uPageSize;
            ulThisCount--;
            ulCount--;
            ulPageNum++;
        }

        /*  Display the I/O status only if something unusual happened.
        */
        if(ioStat.op.ulPageStatus != 0)
            DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command reads one or more pages from flash, using allocator interfaces.\n\n");
    DclPrintf("Syntax:  %s StartPage[:Count] [to] [Buffer]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartPage  - The starting page number to read within the disk, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of pages to read.  If this is greater than 1,\n");
    DclPrintf("               then sequential Page Buffers will used.  If the count exceeds the\n");
    DclPrintf("               count of remaining Page Buffers relative to the start, the reads\n");
    DclPrintf("               will wrap back to the specified starting buffer.  Defaults to 1.\n");
    DclPrintf("  Buffer     - The Page Buffer (PBn) into which data will be read.  Defaults to\n");
    DclPrintf("               PB1.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugAllocWrite()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugAllocWrite(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulPageNum;
    D_UINT32        ulCount = 1;
    char            szDescription[MAX_DESCRIPTION_LEN];
    FFXIOSTATUS     ioStat;
    D_BUFFER       *pBuff;
    int             argnum;
    DCLSHELLPARAMS  params;
    unsigned        fGotArg;
    unsigned        nElements;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(!pFXDB->hVBF)
    {
        DclPrintf("The allocator is not initialized for this disk\n");
        return FFXSTAT_BADHANDLE;
    }

    argnum = 1;

    if(!ParsePageBuffName(pFXDB, argv[argnum], &pBuff, szDescription, &nElements, NULL, &fGotArg))
    {
        DclPrintf("Syntax error in \"%s\"\n", argv[argnum]);
        return FFXSTAT_BADSYNTAX;
    }

    if(fGotArg)
        argnum++;

    if(argnum < argc && DclStrICmp(argv[argnum], "TO") == 0)
    {
        argnum++;
    }

    if(argnum >= argc)
    {
        DclPrintf("Syntax error!\n\n");
        goto DisplayHelp;
    }

    pStr = DclNtoUL(argv[argnum], &ulPageNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    argnum++;

    if(argnum < argc)
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
        goto DisplayHelp;
    }

    while(ulCount)
    {
        D_UINT32    ulThisCount = DCLMIN(ulCount, nElements);

        DclPrintf("Writing %lU pages starting with %s to flash page %lU\n",
            ulThisCount, szDescription, ulPageNum);

        ioStat = FfxVbfWritePages(pFXDB->hVBF, ulPageNum, ulThisCount, pBuff);
        if(!IOSUCCESS(ioStat, ulThisCount))
        {
            DclPrintf("Failed to write %lU pages, starting with page %lU, %s\n", ulThisCount, ulPageNum, FfxDecodeIOStatus(&ioStat));
            break;
        }

        /*  Display the I/O status only if something unusual happened.
        */
        if(ioStat.op.ulPageStatus != 0)
            DclPrintf("IOStatus was %s\n", FfxDecodeIOStatus(&ioStat));

        ulPageNum += ulThisCount;
        ulCount -= ulThisCount;
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command writes one or more pages to flash, using allocator interfaces.\n\n");
    DclPrintf("Syntax:  %s [Buffer] [to] StartPage[:Count]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  Buffer     - The starting Page Buffer (PBn) to write.  If not specified, PB1\n");
    DclPrintf("               will be used.\n");
    DclPrintf("  StartPage  - The starting page number to write to the disk, relative to zero.\n");
    DclPrintf("  Count      - The optional number of pages to write.  If this is greater than 1,\n");
    DclPrintf("               then sequential Page Buffers will used.  If the count exceeds the\n");
    DclPrintf("               count of remaining Page Buffers relative to the start, the writes\n");
    DclPrintf("               will wrap back to the specified starting buffer.  Defaults to 1.\n");

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: DebugAllocDiscard()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS DebugAllocDiscard(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FXDB           *pFXDB;
    const char     *pStr;
    D_UINT32        ulPageNum;
    D_UINT32        ulCount = 1;
    FFXIOSTATUS     ioStat;
    int             argnum;
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(argc != 2 || ISHELPREQUEST())
        goto DisplayHelp;

    if(!IsValidDisk(hShell, pFXDB))
        return FFXSTAT_SHELL_NOCURRENTDISK;

    if(!pFXDB->hVBF)
    {
        DclPrintf("The allocator is not initialized for this disk\n");
        return FFXSTAT_BADHANDLE;
    }

    argnum = 1;

    pStr = DclNtoUL(argv[argnum], &ulPageNum);
    if(pStr && *pStr == ':')
    {
        pStr = DclNtoUL(pStr+1, &ulCount);
        if(!pStr || *pStr != 0)
            goto DisplayHelp;

            if(!ulCount)
            {
                DclPrintf("A valid count must be specified\n\n");
                goto DisplayHelp;
            }
    }
    else
    {
        if(!pStr || *pStr != 0)
            goto DisplayHelp;
    }

    argnum++;

    if(argnum < argc)
    {
        DclPrintf("Unrecognized option: \"%s\"\n\n", argv[argnum]);
        goto DisplayHelp;
    }

    while(ulCount)
    {
        ioStat = FfxVbfDiscardPages(pFXDB->hVBF, ulPageNum, 1);
        if(!IOSUCCESS(ioStat, 1))
        {
            DclPrintf("Failed to discard page %lU, %s\n", ulPageNum, FfxDecodeIOStatus(&ioStat));
            break;
        }

        /*  Display the I/O status only if something unusual happened.
        */
        if(ioStat.op.ulPageStatus != 0)
            DclPrintf("Read returned %s\n", FfxDecodeIOStatus(&ioStat));

        ulPageNum++;
        ulCount--;
    }

    return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclPrintf("This command discards one or more allocator pages.\n\n");
    DclPrintf("Syntax:  %s StartPage[:Count]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  StartPage  - The starting page number to discard within the disk, relative\n");
    DclPrintf("               to zero.\n");
    DclPrintf("  Count      - The optional number of pages to discard.  Defaults to 1 if not\n");
    DclPrintf("               specified.\n");

    return FFXSTAT_BADSYNTAX;
}

#endif


                    /*------------------------------*\
                     *                              *
                     *      Helper Functions        *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Local: InitDisk()

    This function configures FXDB for the specified DISKn.  Any
    previous configuration for a different disk is destroyed.

    Parameters:

    Return Value:
        Returns an FFXSTATUS value.
-------------------------------------------------------------------*/
static FFXSTATUS InitDisk(
    DCLSHELLHANDLE  hShell,
    FXDB           *pFXDB,
    unsigned        nDiskNum)
{
    FXDB            fxdb;
    FFXSTATUS       ffxStat;

    DclMemSet(&fxdb, 0, sizeof(fxdb));

    if(nDiskNum == UINT_MAX)
    {
        /*  Use the first valid disk we can find
        */
        for(nDiskNum=0; nDiskNum<FFX_MAX_DISKS; nDiskNum++)
        {
            fxdb.hFML = FfxFmlHandle(nDiskNum);
            if(fxdb.hFML)
                break;
        }

        if(!fxdb.hFML)
        {
            DclPrintf("There are no initialized FlashFX Disks\n");
            return FFXSTAT_DISKNUMBERINVALID;
        }
    }
    else
    {
        fxdb.hFML = FfxFmlHandle(nDiskNum);
        if(!fxdb.hFML)
        {
            DclPrintf("DISK%u is not valid\n", nDiskNum);
            return FFXSTAT_DISKNUMBERINVALID;
        }
    }

    fxdb.nDiskNum = nDiskNum;

  #if FFXCONF_ALLOCATORSUPPORT
    fxdb.hVBF = FfxVbfHandle(nDiskNum);
    if(fxdb.hVBF)
    {
        ffxStat = FfxVbfDiskMetrics(fxdb.hVBF, &fxdb.VbfInfo);
        if(ffxStat != FFXSTAT_SUCCESS)
        {
            DclPrintf("Error retrieving VBF disk info\n");
            return ffxStat;
        }
    }
  #endif

    ffxStat = FfxFmlDiskInfo(fxdb.hFML, &fxdb.ffi);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error retrieving FML disk info\n");
        return ffxStat;
    }

    ffxStat = FfxFmlDeviceInfo(fxdb.ffi.nDeviceNum, &fxdb.DevInfo);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error retrieving FML device info\n");
        return ffxStat;
    }

    /*  If any old and new page size information is the same, no need
        to release and reallocate the buffers.
    */
    if(pFXDB->ffi.uPageSize == fxdb.ffi.uPageSize)
    {
        DclAssert(pFXDB->pPB[0]);
        DclAssert(pFXDB->pSB[0]);
        DclAssert(pFXDB->ffi.uSpareSize == fxdb.ffi.uSpareSize);

        DclMemCpy(&fxdb.pPB[0], &pFXDB->pPB[0], sizeof(fxdb.pPB[0]) * MAX_BUFFERS);
        DclMemCpy(&fxdb.pSB[0], &pFXDB->pSB[0], sizeof(fxdb.pSB[0]) * MAX_BUFFERS);
    }
    else
    {
        unsigned    nn;

        /*  Free any old buffers we might have
        */
        if(pFXDB->pSB[0])
            DclMemFree(pFXDB->pSB[0]);

        if(pFXDB->pPB[0])
            DclMemFree(pFXDB->pPB[0]);

        /*  Allocate memory for our various page buffers.  Perform a single
            allocation operation then update pointers accordingly.
        */
        fxdb.pPB[0] = DclMemAlloc(fxdb.ffi.uPageSize * MAX_BUFFERS);
        if(!fxdb.pPB[0])
            return DCLSTAT_OUTOFMEMORY;

        for(nn=1; nn<MAX_BUFFERS; nn++)
            fxdb.pPB[nn] = fxdb.pPB[nn-1] + fxdb.ffi.uPageSize;

        /*  Allocate memory for our various spare buffers (if NAND).  Perform
            a single allocation operation then update pointers accordingly.
        */
        if(fxdb.ffi.uSpareSize)
        {
            fxdb.pSB[0] = DclMemAlloc(fxdb.ffi.uSpareSize * MAX_BUFFERS);
            if(!fxdb.pSB[0])
            {
                DclMemFree(fxdb.pPB[0]);
                return DCLSTAT_OUTOFMEMORY;
            }

            for(nn=1; nn<MAX_BUFFERS; nn++)
                fxdb.pSB[nn] = fxdb.pSB[nn-1] + fxdb.ffi.uSpareSize;
         }
    }

    /*  Replace the original structure contents with the new
    */
    *pFXDB = fxdb;

    DclSNPrintf(pFXDB->szPrompt, sizeof(pFXDB->szPrompt), "FXDB-DISK%U>", pFXDB->nDiskNum);

    if(hShell)
    {
        DCLSHELLPARAMS  params;

        DclShellParams(hShell, &params);

        DclEnvVarAdd(params.hEnv, "PROMPT", pFXDB->szPrompt);

        /*  Ignore any possible error from this...
        */
    }

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Local: RevalidateDisk()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL RevalidateDisk(
    DCLSHELLHANDLE  hShell)
{
    FXDB           *pFXDB;
    DCLSHELLPARAMS  params;

    DclAssert(hShell);

    DclShellParams(hShell, &params);
    pFXDB = (FXDB*)params.pUserData;
    DclAssert(pFXDB);

    if(pFXDB->hFML == NULL)
    {
        DCLSTATUS   dclStat;

        /*  If we did not have a valid Disk to start with, try to
            see if we have one now.  If possible find one with the
            same original Disk number, but if that is not available,
            search for any Disk.
        */
        if(FfxFmlHandle(pFXDB->nDiskNum))
            dclStat = InitDisk(hShell, pFXDB, pFXDB->nDiskNum);
        else
            dclStat = InitDisk(hShell, pFXDB, UINT_MAX);

        /*  If that all worked, then we're good...
        */
        if(dclStat == DCLSTAT_SUCCESS)
            return TRUE;
    }
    else
    {
        /*  If we had a Disk to start with, see if the handle is
            still valid.  If so, we're all good.
        */
        if(FfxFmlHandle(pFXDB->nDiskNum) == pFXDB->hFML)
            return TRUE;

        /*  The Disk we were using apparently went away.  Try to
            find another Disk to switch to.
        */
        if(InitDisk(hShell, pFXDB, UINT_MAX) == DCLSTAT_SUCCESS)
            return TRUE;
    }

    /*  The hFML field being NULL is the indicator to the various shell
        commands that there is no "current" Disk which can be operated
        upon.
    */
    pFXDB->hFML = NULL;

    DclStrNCpy(pFXDB->szPrompt, "FXDB->NoDisk>", sizeof(pFXDB->szPrompt));

    /*  Ignore any possible error from this...
    */
    DclEnvVarAdd(params.hEnv, "PROMPT", pFXDB->szPrompt);

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: IsValidDisk()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static D_BOOL IsValidDisk(
    DCLSHELLHANDLE  hShell,
    FXDB           *pFXDB)
{
    (void) hShell;

    if(pFXDB->hFML)
        return TRUE;

    DclPrintf("This command cannot be used until a Disk is created\n");

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: ParseDisplayArgs()

    This function parses the arguments used for the various
    display commands.

    Parameters:

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseDisplayArgs(
    FXDB           *pFXDB,
    int             argc,
    char          **argv,
    D_UINT32       *pulOffset,
    D_UINT32       *pulCount,
    D_UINT16       *puWidth,
    D_BUFFER      **ppBuff,
    char           *pszDescription,
    unsigned        nScale,
    unsigned        nConsoleWidth)
{
    const char     *pStr;
    int             ii;
    D_UINT16        uMaxWidth;
    unsigned        nElements;
    unsigned        fFoundBuff = FALSE;

    DclAssert(argc);
    DclAssert(pFXDB);
    DclAssert(pulOffset);
    DclAssert(pulCount);
    DclAssert(puWidth);
    DclAssert(ppBuff);
    DclAssert(pszDescription);
    DclAssert(nScale == 1 || nScale == 2 || nScale == 4);
    DclAssert(nConsoleWidth);

    /*  Initialize default values
    */
    *pulOffset = 0;
    *pulCount = D_UINT32_MAX / nScale;          /* (magic large value)  */
    *puWidth = D_UINT16_MAX;

    /*  If the event that there are no arguments, call this function
        with a NULL pointer will initialize the default arguments.
    */
    ParsePageBuffName(pFXDB, NULL, ppBuff, pszDescription, &nElements, NULL, NULL);
    uMaxWidth = pFXDB->ffi.uPageSize;

    /*  Loop through all arguments and process them
    */
    for(ii=1; ii<argc; ii++)
    {
        if(!fFoundBuff && ParseBufferName(pFXDB, argv[ii], ppBuff, pszDescription, &uMaxWidth, &nElements))
        {
            fFoundBuff = TRUE;
            continue;
        }

        if(DclStrNICmp(argv[ii], "/Width:", 7) == 0)
        {
            D_UINT32    ulWidth;

            pStr = DclNtoUL(&argv[ii][7], &ulWidth);
            if(!pStr || *pStr != 0)
            {
                DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
                return FALSE;
            }

            if(ulWidth && ulWidth <= DEFAULT_BYTE_WIDTH)
                *puWidth = (D_UINT16)ulWidth;
            else
                DclPrintf("Invalid /Width value -- using default\n\n");

            continue;
        }

        pStr = DclNtoUL(argv[ii], pulOffset);
        if(pStr && *pStr == ':')
        {
            pStr = DclNtoUL(pStr+1, pulCount);
            if(!pStr || *pStr != 0)
            {
                DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
                return FALSE;
            }

            continue;
        }

        DclPrintf("Syntax error in \"%s\"\n\n", argv[ii]);
        return FALSE;
    }

    if(*pulOffset >= uMaxWidth)
    {
        DclPrintf("The starting offset is out of range, %lU >= %U\n\n", *pulOffset, uMaxWidth);
        return FALSE;
    }

    if(*pulCount == D_UINT32_MAX / nScale)
        *pulCount = (uMaxWidth - *pulOffset) / nScale;

    /*  Range check- must have uMaxWidth * nElements to fit in D_UINT16 
        data type.
    */
    DclAssert((((D_UINT32)uMaxWidth) * nElements) <= D_UINT16_MAX);
    uMaxWidth *= (D_UINT16)nElements;

    *pulCount = DCLMIN(*pulCount, (uMaxWidth - *pulOffset) / nScale);

    /*  If an explicit /width value was not specified, choose a
        default width appropriate for the console width.
    */
    if(*puWidth == D_UINT16_MAX)
    {
        unsigned pulWidth = CalculateHexDumpWidth(nConsoleWidth, nScale);
        
        /*  Range check- must have CalculateHexDumpWidth returning a value
            that fits in D_UINT16.
        */
        DclAssert(pulWidth <= D_UINT16_MAX);
        *puWidth = (D_UINT16)pulWidth;
    }

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ParseBufferName()

    This function parses the specified string for a recognized
    buffer name, and returns the buffer characteristics.

    Parameters:

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseBufferName(
    FXDB           *pFXDB,
    const char     *pszName,
    D_BUFFER      **ppBuff,
    char           *pszDescription,
    D_UINT16       *puWidth,
    unsigned       *pnElements)
{
    unsigned        fFoundArg = FALSE;

    DclAssert(pFXDB);
    DclAssert(pszName);
    DclAssert(ppBuff);

    if(!ParseSpareBuffName(pFXDB, pszName, ppBuff, pszDescription, pnElements, &fFoundArg))
        return FALSE;

    if(fFoundArg)
    {
        if(puWidth)
            *puWidth = pFXDB->ffi.uSpareSize;

        return TRUE;
    }

    /*  Do page buff last so if neither PB nor SB are found, the default
        values will be filled in for page buffers.
    */
    if(ParsePageBuffName(pFXDB, pszName, ppBuff, pszDescription, pnElements, NULL, &fFoundArg))
    {
        if(fFoundArg)
        {
            if(puWidth)
                *puWidth = pFXDB->ffi.uPageSize;

            return TRUE;
        }
    }

    return FALSE;
}


/*-------------------------------------------------------------------
    Local: ParsePageBuffName()

    This function parses the specified string for a recognized
    buffer name, and returns the buffer characteristics.

    Parameters:

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParsePageBuffName(
    FXDB           *pFXDB,
    const char     *pszName,
    D_BUFFER      **ppBuff,
    char           *pszDescription,
    unsigned       *pnElements,
    unsigned       *pnBufferNum,
    unsigned       *pfFoundArg)
{
    D_UINT32        ulNum = 1;

    DclAssert(pFXDB);
    DclAssert(ppBuff);

    if(pfFoundArg)
        *pfFoundArg = FALSE;

    if(pszName && DclStrNICmp(pszName, "PB", 2) == 0)
    {
        const char *    pStr;

        pStr = DclNtoUL(&pszName[2], &ulNum);
        if(!pStr || *pStr)
        {
            DclPrintf("Bad syntax:  \"%s\"\n", pszName);
            return FALSE;
        }

        if(!ulNum || ulNum > MAX_BUFFERS)
        {
            DclPrintf("Page buffer number %lU is out of range, must range from 1 to %u\n",
                ulNum, MAX_BUFFERS);

            return FALSE;
        }

        if(pfFoundArg)
            *pfFoundArg = TRUE;
    }

    *ppBuff = pFXDB->pPB[ulNum-1];

    if(pszDescription)
        DclSNPrintf(pszDescription, MAX_DESCRIPTION_LEN, "Page Buffer %lU", ulNum);

    if(pnBufferNum)
        *pnBufferNum = (unsigned)ulNum-1;

    if(pnElements)
        *pnElements = MAX_BUFFERS - (ulNum-1);

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: ParseSpareBuffName()

    This function parses the specified string for a recognized
    buffer name, and returns the buffer characteristics.

    Parameters:

    Return Value:
        Returns TRUE if successful, or FALSE otherwise.
-------------------------------------------------------------------*/
static D_BOOL ParseSpareBuffName(
    FXDB           *pFXDB,
    const char     *pszName,
    D_BUFFER      **ppBuff,
    char           *pszDescription,
    unsigned       *pnElements,
    unsigned       *pfFoundArg)
{
    D_UINT32        ulNum = 1;

    DclAssert(pFXDB);
    DclAssert(ppBuff);

    if(pfFoundArg)
        *pfFoundArg = FALSE;

    if(pszName && DclStrNICmp(pszName, "SB", 2) == 0)
    {
        const char *    pStr;

        pStr = DclNtoUL(&pszName[2], &ulNum);
        if(!pStr || *pStr)
        {
            DclPrintf("Bad syntax:  \"%s\"\n", pszName);
            return FALSE;
        }

        if(!ulNum || ulNum > MAX_BUFFERS)
        {
            DclPrintf("Spare buffer number %lU is out of range, must range from 1 to %u\n",
                ulNum, MAX_BUFFERS);
            return FALSE;
        }

        if(pfFoundArg)
            *pfFoundArg = TRUE;
    }

    *ppBuff = pFXDB->pSB[ulNum-1];

    if(pszDescription)
        DclSNPrintf(pszDescription, MAX_DESCRIPTION_LEN, "Spare Buffer %lU", ulNum);

    if(pnElements)
        *pnElements = MAX_BUFFERS - (ulNum-1);

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: CalculateHexDumpWidth()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static unsigned CalculateHexDumpWidth(
    unsigned    nConsoleWidth,
    unsigned    nScale)
{
    unsigned    nWidth;

    DclAssert(nConsoleWidth);
    DclAssert(nScale == 1 || nScale == 2 || nScale == 4);

    nWidth = DEFAULT_BYTE_WIDTH / nScale;

    switch(nScale)
    {
        /*  Magic numbers are approximations based on our carnal
            knowledge of the DclHexDump() output format -- doesn't
            scale well across different element sizes...
        */
        case 1:
            if(nConsoleWidth < 40)
                nWidth /= 8;
            else if(nConsoleWidth < 75)
                nWidth /= 4;
            else if(nConsoleWidth < 140)
                nWidth /= 2;

            break;

        case 2:
            if(nConsoleWidth < 30)
                nWidth /= 8;
            else if(nConsoleWidth < 50)
                nWidth /= 4;
            else if(nConsoleWidth < 90)
                nWidth /= 2;

            break;

        case 4:
            if(nConsoleWidth < 25)
                nWidth /= 8;
            else if(nConsoleWidth < 45)
                nWidth /= 4;
            else if(nConsoleWidth < 80)
                nWidth /= 2;

            break;

        default:
            DclProductionError();
    }

    return nWidth;
}



#endif  /* DCLCONF_COMMAND_SHELL */

