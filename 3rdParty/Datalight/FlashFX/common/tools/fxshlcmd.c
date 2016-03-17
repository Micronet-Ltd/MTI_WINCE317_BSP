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

    This module creates a FlashFX command shell using the DCL shell
    functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxshlcmd.c $
    Revision 1.29  2010/12/12 06:42:45Z  garyp
    Updated for the new FXDUMP calling convention.
    Revision 1.28  2010/07/11 17:47:54Z  garyp
    Eliiminiated the "remount" command.  Updated FfxShellAddCommands()
    to support using a default shell system name and prefix.
    Revision 1.27  2010/07/05 22:08:17Z  garyp
    Added support for chip serial numbers.
    Revision 1.26  2010/01/27 04:27:34Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.25  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.24  2009/12/08 23:38:20Z  garyp
    Updated the ParamGet command to handle the ECCCORRECTED and
    ECCUNCORRECTED symbols.
    Revision 1.23  2009/12/01 07:29:16Z  garyp
    Cleaned up the CHIPID display to be more readable.
    Revision 1.22  2009/11/11 00:55:41Z  garyp
    Updated to condition the TESTS and TOOLS on FlashFX specific
    settings, rather than DCL settings.
    Revision 1.21  2009/07/21 22:16:32Z  garyp
    Merged from the v4.0 branch.  Updated the "ParamGet" command to support the
    "CHIPID" type, and fixed to not try to enumerate invalid param values.
    Revision 1.20  2009/04/01 15:02:58Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.19  2009/02/17 06:17:41Z  keithg
    Added explicit void to unused function parameters.
    Revision 1.18  2009/02/09 22:40:33Z  billr
    Make StressMT command disappear completely in a single-threaded build.
    Revision 1.17  2009/02/09 02:47:34Z  garyp
    Merged from the v4.0 branch.  Moved the "FXDevice" and "FXDisk" commands
    into the module fxshlshare.c.  Updated the FXDEVICE command to display 
    whether BBM is loaded for the DEVICE or not.  Added some typecasts to
    mollify a picky compiler.  Updated the FXDISK command to display whether
    the allocator is loaded or not.
    Revision 1.16  2008/06/04 23:04:17Z  billr
    Make StressMT command disappear in a single-threaded build.
    Revision 1.15  2008/05/16 18:29:00Z  garyp
    Merged from the WinMobile branch.
    Revision 1.14.1.2  2008/05/16 18:29:00Z  garyp
    Fixed the FXDISK command to cleanly handle cases where the allocator
    is not included.
    Revision 1.14  2008/03/24 19:38:14Z  Garyp
    Updated to work properly when allocator support is disabled.  Added the
    FXDEVICE and FXDISK commands.  Modified the TRACEMASKGET/SWAP commands to
    perform their actions via the ParameterGet/Set() interface, which uses the
    external API, allowing those commands to affect the trace state of the 
    installed driver, rather than the external tool.
    Revision 1.13  2008/01/13 04:01:59Z  keithg
    Function header updates to support autodoc.
    Revision 1.12  2008/01/07 04:54:50Z  garyp
    Minor syntax fix for a help command.
    Revision 1.11  2007/12/17 07:56:13Z  Garyp
    Modified the FXDebug command to automatically choose the FlashFX
    DISK to use.
    Revision 1.10  2007/12/15 01:38:38Z  Garyp
    Updated to use the new DclShellAddMultipleCommands() function, and to
    call a port specific function to add OS specific commands.
    Revision 1.9  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/10/05 18:19:22Z  Garyp
    Corrected to remove some typecasts per code review.
    Revision 1.7  2007/10/03 20:51:20Z  Garyp
    Added a typecast to avoid a compiler warning.
    Revision 1.6  2007/09/12 00:37:48Z  Garyp
    Added the "Debug" command.  Modified the trace commands so they can
    be initialized separately.
    Revision 1.5  2007/04/08 00:10:47Z  Garyp
    Modified so that the latency parameter get/set command are conditional
    on FFXCONF_LATENCYREDUCTIONENABLED.  Added the ability to control the
    DevMgr mutex cycling.
    Revision 1.4  2007/04/07 03:23:07Z  Garyp
    Added the "ParamGet" and "ParamSet" functions.
    Revision 1.3  2007/04/02 21:04:06Z  Garyp
    Modified to no longer use the tool wrapper function which prevented the
    command prefix stuff from working properly.
    Revision 1.2  2007/03/19 17:20:40Z  Garyp
    Added commands to get and set the FlashFX trace mask.
    Revision 1.1  2007/02/08 22:35:40Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxtools.h>
#include <fxdriver.h>
#include <fxfmlapi.h>
#include <dlshell.h>
#include <fxshell.h>
#include <fxver.h>
#include "fxshl.h"

#if DCLCONF_COMMAND_SHELL

#define ARGBUFFLEN  (256)   /* Buffer for concatenating arguments */

#if FFXCONF_SHELL_TOOLS
#if FFXCONF_VBFSUPPORT
static DCLSTATUS    ShellCmdFfxCheck(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxCompact(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxFormat(      DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxImage(       DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS    ShellCmdFfxDebug(       DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxDump(        DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxParamGet(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdFfxParamSet(    DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
#if FFXCONF_SHELL_TESTS
#if FFXCONF_VBFSUPPORT
#if DCL_OSFEATURE_THREADS
static DCLSTATUS    ShellCmdFfxStressMT(    DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS    ShellCmdFfxTestVBF(     DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS    ShellCmdFfxTestFMSL(    DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
static DCLSTATUS    ShellCmdFfxInfo(        DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdTraceMaskGet(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    ShellCmdTraceMaskSet(   DCLSHELLHANDLE hShell, int argc, char **argv);

static D_BOOL       ParseDriveNumber(const char *pszString, D_UINT16 *puDriveNumber, D_UINT16 *puStringLen);
static void         DisplayParamValue(FFXPARAM idParam, const char *pBuffer, D_UINT32 ulLen);


static const DCLSHELLCOMMAND aFfxCmds[] =
{
  #if FFXCONF_SHELL_TOOLS
  #if FFXCONF_VBFSUPPORT
    {NULL,          "%sCheck",    "Check the spare unit on a Disk",                 ShellCmdFfxCheck},
    {NULL,          "%sCompact",  "Compact a flash Disk",                           ShellCmdFfxCompact},
    {NULL,          "%sFormat",   "Format a flash Disk",                            ShellCmdFfxFormat},
    {NULL,          "%sImage",    "Read or write flash images",                     ShellCmdFfxImage},
  #endif
    {NULL,          "%sDebug",    "Invoke the interactive FlashFX Debugger (FXDB)", ShellCmdFfxDebug},
    {NULL,          "%sDump",     "Dump a flash Disk image",                        ShellCmdFfxDump},
    {NULL,          "%sParamGet", "Get a configuration parameter",                  ShellCmdFfxParamGet},
    {NULL,          "%sParamSet", "Set a configuration parameter",                  ShellCmdFfxParamSet},
  #endif
  #if FFXCONF_SHELL_TESTS
  #if FFXCONF_VBFSUPPORT
  #if DCL_OSFEATURE_THREADS
    {NULL,          "%sStressMT", "Run the Multithreaded VBF stress test",          ShellCmdFfxStressMT},
  #endif
    {NULL,          "%sTestVBF",  "Run the VBF unit test",                          ShellCmdFfxTestVBF},
  #endif
    {NULL,          "%sTestFMSL", "Run the FMSL unit test",                         ShellCmdFfxTestFMSL},
  #endif
    {NULL,          "%sDevice"  , "Create, destroy, and enumerate Devices",         FfxShellCmdDevice},
    {NULL,          "%sDisk"    , "Create, destroy, and enumerate Disks",           FfxShellCmdDisk},
    {NULL,          "%sInfo",     "Display detailed Device and Disk information",   ShellCmdFfxInfo}
};

static const DCLSHELLCOMMAND aFfxTraceCmds[] =
{
    {"Debugging",   "Trace%sGet", "Get the flash manager trace mask",               ShellCmdTraceMaskGet},
    {"Debugging",   "Trace%sSet", "Set the flash manager trace mask",               ShellCmdTraceMaskSet},
};


/*-------------------------------------------------------------------
    Public: FfxShellAddCommands()

    Add FlashFX commands to the shell.

    Parameters:
        hShell       - The shell handle.
        pszClassName - A pointer to the null-terminated command class
                       name to use.  May be NULL to use the default
                       product base name.
        pszPrefix    - A pointer to the null-terminated command prefix
                       to use.  May be NULL to use the default prefix.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxShellAddCommands(
    DCLSHELLHANDLE  hShell,
    const char     *pszClassName,
    const char     *pszPrefix)
{
    DCLSTATUS       dclStat;

    if(!hShell)
    {
        DclError();
        return FFXSTAT_BADPARAMETER;
    }

    if(!pszClassName)
        pszClassName = PRODUCTBASENAME;

    if(!pszPrefix)
        pszPrefix = PRODUCTPREFIX;

    dclStat = DclShellAddMultipleCommands(hShell, aFfxCmds, DCLDIMENSIONOF(aFfxCmds), pszClassName, pszPrefix);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    dclStat = FfxShellAddTraceCommands(hShell, pszClassName, pszPrefix);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return FfxShellAddOsCommands(hShell, pszClassName, pszPrefix);
}


/*-------------------------------------------------------------------
    Public: FfxShellAddTraceCommands()

    This function adds FlashFX trace commands to the shell.

    Parameters:
        hShell       - The shell handle.
        pszClassName - A pointer to the null-terminated command
                       class name to use.
        pszPrefix    - A pointer to the null-terminated command
                       prefix to use.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxShellAddTraceCommands(
    DCLSHELLHANDLE  hShell,
    const char     *pszClassName,
    const char     *pszPrefix)
{
    DCLSTATUS       dclStat;

    if(!hShell)
    {
        DclError();
        return FFXSTAT_BADPARAMETER;
    }

    DclAssert(pszClassName);
    DclAssert(pszPrefix);

    dclStat = DclShellAddMultipleCommands(hShell, aFfxTraceCmds, DCLDIMENSIONOF(aFfxTraceCmds), pszClassName, pszPrefix);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    return FFXSTAT_SUCCESS;
}



                    /*------------------------------*\
                     *                              *
                     *  Standard FlashFX Commands   *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Local: ShellCmdTraceMaskGet()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdTraceMaskGet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    FFXSTATUS       ffxStat;
    D_UINT32        ulMask;

    (void) hShell;
    if(argc > 1)
        goto TraceGetSyntax;

    ffxStat = FfxDriverDiskParameterGet(NULL, FFXPARAM_DEBUG_TRACEMASKGET, &ulMask, sizeof(ulMask));
    if(ffxStat != FFXSTAT_SUCCESS)
        DclPrintf("Command failed with status %lX\n", ffxStat);
    else
        DclPrintf("The current flash manager trace mask value is:  %lX\n\n", ulMask);

    return ffxStat;

  TraceGetSyntax:

    DclPrintf("%s displays the current flash manager trace mask value.\n\n", argv[0]);
    DclPrintf("Syntax:  %s\n\n", argv[0]);

    return FFXSTAT_BADPARAMETER;

}


/*-------------------------------------------------------------------
    Local: ShellCmdTraceMaskSet()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdTraceMaskSet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    const char     *pStr;
    D_UINT32        ulMask;
    FFXSTATUS       ffxStat;

    (void) hShell;
    if(argc != 2 || ISHELPREQUEST())
        goto TraceSetSyntax;

    pStr = DclNtoUL(argv[1], &ulMask);

    if(!pStr || *pStr != 0)
    {
        DclPrintf("Syntax error, \"%s\" not recognized as a numeric value\n", argv[1]);
        return FFXSTAT_BADPARAMETER;
    }

    ffxStat = FfxDriverDiskParameterSet(NULL, FFXPARAM_DEBUG_TRACEMASKSWAP, &ulMask, sizeof(ulMask));
    if(ffxStat != FFXSTAT_SUCCESS)
        DclPrintf("Command failed with status %lX\n", ffxStat);
    else
        DclPrintf("New trace mask value set.  The old trace mask value was:  %lX\n", ulMask);

    return ffxStat;

  TraceSetSyntax:

    DclPrintf("%s sets a new current flash manager trace mask value.\n\n", argv[0]);
    DclPrintf("Syntax:  %s  mask\n\n", argv[0]);
    DclPrintf("Where:  mask - is the new trace mask value, expressed as a hex (0x) or\n");
    DclPrintf("               decimal number\n\n");

    return FFXSTAT_BADPARAMETER;

}


#if FFXCONF_SHELL_TOOLS
#if FFXCONF_VBFSUPPORT

/*-------------------------------------------------------------------
    Local: ShellCmdFfxCheck()

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxCheck(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxCheckUnits(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxCheckUnits(&tp);

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: ShellCmdFfxCompact()

    Parameters:

    Return Value:

-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxCompact(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxReclaim(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxReclaim(&tp);

    return FFXSTAT_BADSYNTAX;
}

#endif

/*-------------------------------------------------------------------
    Local: ShellCmdFfxDebug()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxDebug(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSHELLPARAMS  params;

    DclShellParams(hShell, &params);

    if(argc > 1 || ISHELPREQUEST())
        goto DisplayHelp;

    return FfxDebug(UINT_MAX, &params);

  DisplayHelp:

    DclPrintf("This command invokes the interactive FlashFX Debugger (FXDB).\n\n");
    DclPrintf("Syntax:  %s\n", argv[0]);

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: ShellCmdFfxDump()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxDump(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    FFXTOOLPARAMS   tp = {{0}};
    int             ii;

    (void) hShell;

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

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

    return FfxDump(&tp);

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxDump(&tp);

    return FFXSTAT_BADSYNTAX;
}


#if FFXCONF_VBFSUPPORT
/*-------------------------------------------------------------------
    Local: ShellCmdFfxFormat()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxFormat(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxFormat(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxFormat(&tp);

    return FFXSTAT_BADSYNTAX;
}


/*-------------------------------------------------------------------
    Local: ShellCmdFfxImage()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxImage(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxImage(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxImage(&tp);

    return FFXSTAT_BADSYNTAX;
}

#endif


/*-------------------------------------------------------------------
    Local: ShellCmdFfxParamGet()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxParamGet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT16        uDriveNum = 0;
    D_UINT32        ulLen;
    FFXPARAM        idParam;
    FFXDISKHANDLE   hDisk;
    FFXSTATUS       ffxStat;
    char           *pBuffer;

    (void) hShell;
    if(argc != 3 || ISHELPREQUEST())
        goto Syntax;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto Syntax;
    }

    hDisk = FfxDriverDiskHandle(NULL, (unsigned)uDriveNum);
    if(!hDisk)
    {
        DclPrintf("DISK%U not initialized\n", uDriveNum);
        return FFXSTAT_BADPARAMETER;
    }

    if(DclStrICmp(argv[2], "ALL") == 0)
    {
        for(idParam = FFXPARAM_STARTOFLIST+1;
            idParam < FFXPARAM_ENDOFLIST;
            idParam ++)
        {
            ffxStat = FfxDriverDiskParameterGet(hDisk, idParam, NULL, 0);

            /*  Decode the status value to determine the parameter length
            */
            ulLen = DCLSTAT_GETUINT20(ffxStat);
            if(ulLen != D_UINT32_MAX)
            {
                pBuffer = DclMemAllocZero(ulLen);
                if(pBuffer)
                {
                    ffxStat = FfxDriverDiskParameterGet(hDisk, idParam, pBuffer, ulLen);
                    if(ffxStat == FFXSTAT_SUCCESS)
                        DisplayParamValue(idParam, pBuffer, ulLen);
                    else
                        DclPrintf("Error %lX querying ID %x\n", ffxStat, idParam);

                    DclMemFree(pBuffer);
                }
                else
                {
                    DclPrintf("Error allocating %lU bytes of memory for ID=%x\n", ulLen, idParam);
                }
            }
        }

        return FFXSTAT_SUCCESS;
    }

  #if FFXCONF_LATENCYREDUCTIONENABLED
    if(DclStrICmp(argv[2], "DEVREADMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXREADCOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVWRITEMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXWRITECOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVERASEMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXERASECOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVERASEPOLLTIME") == 0)
    {
        idParam = FFXPARAM_DEVMGR_ERASEPOLLINTERVAL;
    }
    else if(DclStrICmp(argv[2], "DEVCYCLEMUTEX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_CYCLEMUTEX;
    }
    else
  #endif
    if(DclStrICmp(argv[2], "CHIPID") == 0)
    {
        idParam = FFXPARAM_FIM_CHIPID;
    }
    else if(DclStrICmp(argv[2], "CHIPSN") == 0)
    {
        idParam = FFXPARAM_FIM_CHIPSN;
    }
    else if(DclStrICmp(argv[2], "ECCCORRECTED") == 0)
    {
        idParam = FFXPARAM_FIM_CORRECTEDERRORS;
    }
    else if(DclStrICmp(argv[2], "ECCUNCORRECTED") == 0)
    {
        idParam = FFXPARAM_FIM_UNCORRECTABLEERRORS;
    }
    else if(DclStrNICmp(argv[2], "0x", 2) == 0)
    {
        const char *pStr;
        D_UINT32    ulID;

        pStr = DclNtoUL(argv[2], &ulID);
        if(!pStr || *pStr)
            goto BadParam;

        idParam = (FFXPARAM)ulID;
    }
    else
    {
      BadParam:
        DclPrintf("Syntax error in \"%s\"\n", argv[2]);
        return FFXSTAT_BADSYNTAX;
    }

    ffxStat = FfxDriverDiskParameterGet(hDisk, idParam, NULL, 0);

    /*  Decode the status value to determine the parameter length
    */
    ulLen = DCLSTAT_GETUINT20(ffxStat);
    if(ulLen != D_UINT32_MAX)
    {
        pBuffer = DclMemAllocZero(ulLen);
        if(pBuffer)
        {
            ffxStat = FfxDriverDiskParameterGet(hDisk, idParam, pBuffer, ulLen);
            if(ffxStat == FFXSTAT_SUCCESS)
                DisplayParamValue(idParam, pBuffer, ulLen);

            DclMemFree(pBuffer);
        }
        else
        {
            DclPrintf("Error allocating memory\n");
            return DCLSTAT_MEMALLOCFAILED;
        }
    }

    if(ulLen == D_UINT32_MAX || ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error %lX querying ID %x\n", ffxStat, idParam);
        return FFXSTAT_FAILURE;
    }

    return FFXSTAT_SUCCESS;

  Syntax:

    DclPrintf("This command gets a configuration parameter and displays it.\n\n");
    DclPrintf("Syntax:  %s  /DISKn {Name | 0x0000}\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("   /DISKn  Is the disk to use\n");
    DclPrintf("     Name  Is the symbolic name of the parameter to retrieve, which must be\n");
    DclPrintf("           one of the following values:\n");
    DclPrintf("             ALL              - Display all queryable parameter values\n");
    DclPrintf("             CHIPID           - Display the flash chip ID values\n");
    DclPrintf("             CHIPSN           - Display the flash chip serial number\n");
  #if FFXCONF_LATENCYREDUCTIONENABLED
    DclPrintf("             DEVREADMAX       - The max pages to read in one operation\n");
    DclPrintf("             DEVWRITEMAX      - The max pages to write in one operation\n");
    DclPrintf("             DEVERASEMAX      - The max blocks to erase in one operation\n");
    DclPrintf("             DEVERASEPOLLTIME - The erase poll interval (ms)\n");
    DclPrintf("             DEVCYCLEMUTEX    - A flag (0 or 1) to indicate that the DEV mutex\n");
    DclPrintf("                                should be cycled during I/O.\n");
  #endif
    DclPrintf("             ECCCORRECTED     - The count of errors corrected with ECC\n");
    DclPrintf("             ECCUNCORRECTED   - The count of uncorrectable ECC errors\n");
    DclPrintf("   0x0000  An explicit parameter ID number.\n\n");
    DclPrintf("The returned parameter values will be displayed in hexadecimal form.\n");

    return FFXSTAT_BADPARAMETER;
}


/*-------------------------------------------------------------------
    Local: ShellCmdFfxParamSet()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxParamSet(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT16        uDriveNum = 0;
    D_UINT32        ulLen;
    D_UINT32        ulValue;
    D_UINT8         u8;
    D_UINT16        u16;
    FFXPARAM        idParam;
    FFXDISKHANDLE   hDisk;
    FFXSTATUS       ffxStat;
    void           *pBuffer;
    const char     *pStr;

    (void) hShell;
    if(argc != 4 || ISHELPREQUEST())
        goto Syntax;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto Syntax;
    }

    hDisk = FfxDriverDiskHandle(NULL, (unsigned)uDriveNum);
    if(!hDisk)
    {
        DclPrintf("DISK%U not initialized\n", uDriveNum);
        return FFXSTAT_BADPARAMETER;
    }

    pStr = DclNtoUL(argv[3], &ulValue);
    if(!pStr || *pStr)
    {
        DclPrintf("The value \"%s\" is not valid\n", argv[3]);
        return FFXSTAT_BADSYNTAX;
    }

  #if FFXCONF_LATENCYREDUCTIONENABLED
    if(DclStrICmp(argv[2], "DEVREADMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXREADCOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVWRITEMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXWRITECOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVERASEMAX") == 0)
    {
        idParam = FFXPARAM_DEVMGR_MAXERASECOUNT;
    }
    else if(DclStrICmp(argv[2], "DEVERASEPOLLTIME") == 0)
    {
        idParam = FFXPARAM_DEVMGR_ERASEPOLLINTERVAL;
    }
    else if(DclStrICmp(argv[2], "DEVCYCLEMUTEX") == 0)
    {
        if(ulValue > 1)
        {
            DclPrintf("Valid settings for this parameter are 0 and 1\n");
            return FFXSTAT_BADSYNTAX;
        }

        idParam = FFXPARAM_DEVMGR_CYCLEMUTEX;
    }
    else
  #endif
    if(DclStrNICmp(argv[2], "0x", 2) == 0)
    {
        D_UINT32    ulID;

        pStr = DclNtoUL(argv[2], &ulID);
        if(!pStr || *pStr)
            goto BadParam;

        idParam = (FFXPARAM)ulID;
    }
    else
    {
      BadParam:
        DclPrintf("Syntax error in \"%s\"\n", argv[2]);
        return FFXSTAT_BADSYNTAX;
    }

    ffxStat = FfxDriverDiskParameterGet(hDisk, idParam, NULL, 0);

    /*  Decode the status value to determine the parameter length
    */
    ulLen = DCLSTAT_GETUINT20(ffxStat);
    if(ulLen == D_UINT32_MAX)
    {
        DclPrintf("Error %lX querying ID %x\n", ffxStat, idParam);
        return FFXSTAT_FAILURE;
    }

    switch(ulLen)
    {
        case 1:
            if(ulValue > D_UINT8_MAX)
            {
                DclPrintf("The value \"%s\" does not fit in an 8-bit variable\n", argv[3]);
                return FFXSTAT_BADSYNTAX;
            }
            u8 = (D_UINT8)ulValue;
            pBuffer = &u8;
            break;

        case 2:
            if(ulValue > D_UINT16_MAX)
            {
                DclPrintf("The value \"%s\" does not fit in a 16-bit variable\n", argv[3]);
                return FFXSTAT_BADSYNTAX;
            }
            u16 = (D_UINT16)ulValue;
            pBuffer = &u16;
            break;

        case 4:
            pBuffer = &ulValue;
            break;

        default:
            DclPrintf("This command only supports setting values up to 32-bit in size\n");
            return FFXSTAT_BADSYNTAX;
    }

    ffxStat = FfxDriverDiskParameterSet(hDisk, idParam, pBuffer, ulLen);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Error %lX setting the parameter value\n", ffxStat);
    }

    return ffxStat;

  Syntax:

    DclPrintf("This command sets a configuration parameter.\n\n");
    DclPrintf("Syntax:  %s  /DISKn {Name | 0x0000} Value\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("   /DISKn  Is the disk to use\n");
    DclPrintf("     Name  Is the symbolic name of the parameter to set, which must be one of\n");
    DclPrintf("           the following values:\n");
  #if FFXCONF_LATENCYREDUCTIONENABLED
    DclPrintf("             DEVREADMAX       - The max pages to read in one operation\n");
    DclPrintf("             DEVWRITEMAX      - The max pages to write in one operation\n");
    DclPrintf("             DEVERASEMAX      - The max blocks to erase in one operation\n");
    DclPrintf("             DEVERASEPOLLTIME - The erase poll interval (ms)\n");
    DclPrintf("             DEVCYCLEMUTEX    - A flag (0 or 1) to indicate that the DEV mutex\n");
    DclPrintf("                                should be cycled during I/O.\n");
  #endif
    DclPrintf("   0x0000  An explicit parameter ID number.\n");
    DclPrintf("    Value  The configuration parameter value, which must (currently) be a\n");
    DclPrintf("           single hex or decimal number.\n\n");
    DclPrintf("WARNING: Some configuration parameters are NOT designed to be changed after\n");
    DclPrintf("         the system is initialized, and others are not designed to be changed\n");
    DclPrintf("         at all.  Those parameters with symbolic names are safe to change at\n");
    DclPrintf("         run-time.\n");

    return FFXSTAT_BADPARAMETER;
}

#endif


/*-------------------------------------------------------------------
    Local: ShellCmdFfxInfo()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
 static DCLSTATUS ShellCmdFfxInfo(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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


#if FFXCONF_SHELL_TESTS
#if FFXCONF_VBFSUPPORT

#if DCL_OSFEATURE_THREADS
/*-------------------------------------------------------------------
    Local: ShellCmdFfxStressMT()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxStressMT(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

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

    if(FfxStressMT(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxStressMT(&tp);

    return FFXSTAT_BADSYNTAX;
}

#endif /* DCL_OSFEATURE_THREADS */


/*-------------------------------------------------------------------
    Local: ShellCmdFfxTestVBF()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxTestVBF(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxTestVBF(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxTestVBF(&tp);

    return FFXSTAT_BADSYNTAX;
}

#endif


/*-------------------------------------------------------------------
    Local: ShellCmdFfxTestFMSL()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static DCLSTATUS ShellCmdFfxTestFMSL(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achBuffer[ARGBUFFLEN];
    D_UINT16        uDriveNum = 0;
    FFXTOOLPARAMS   tp;
    int             ii;

    (void) hShell;
    DclMemSet(&tp, 0, sizeof(tp));

    tp.dtp.pszCmdName   = argv[0];
    tp.dtp.pszCmdLine   = achBuffer;
    tp.pszDriveForms    = "/DISKn";

    if(argc < 2 || ISHELPREQUEST())
        goto DisplayHelp;

    /*  Retrieve the drive number
    */
    if(!ParseDriveNumber(argv[1], &uDriveNum, NULL))
    {
        DclPrintf("%s: Unable to determine the drive number from '%s'.\n", argv[0], argv[1]);

        goto DisplayHelp;
    }

    tp.nDeviceNum = 0;
    tp.nDiskNum = uDriveNum;

    achBuffer[0] = 0;
    for(ii=2; ii<argc; ii++)
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

    if(FfxTestFMSL(&tp))
        return FFXSTAT_FAILURE;
    else
        return FFXSTAT_SUCCESS;

  DisplayHelp:

    DclStrCpy(achBuffer, "/?");

    FfxTestFMSL(&tp);

    return FFXSTAT_BADSYNTAX;
}

#endif



                    /*------------------------------*\
                     *                              *
                     *      Helper Functions        *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
    Local: ParseDriveNumber()

    Determines if the given drive designation is valid.  It may
    be in any one of the following forms, case-insensitive:
        /DISKn

    Parameters:
        pszString     - ASCIIZ string designating the device to use.
                        This is usually DSK1: or DSK2:.
        puDriveNumber - A pointer to where the drive number is
                        returned.
        puStringLen   - A pointer to where the string length is
                        returned.  Supply a null pointer if this
                        value is not needed.

    Return Value:
        Returns TRUE if successful, otherwise FALSE.
        *puDriveNumber - Set to the zero relative drive number or
                         D_UINT16_MAX
        *puStringLen   - Set to the length of the drive number string
-------------------------------------------------------------------*/
static D_BOOL ParseDriveNumber(
    const char *pszString,
    D_UINT16   *puDriveNumber,
    D_UINT16   *puStringLen)
{
    #define     BUFFSIZE    (7)     /* room for "/DISKn" plus a NULL */
    char        cBuffer[BUFFSIZE];

    *puDriveNumber = D_UINT16_MAX;

    DclStrNCpy(cBuffer, pszString, BUFFSIZE);

    if( cBuffer[BUFFSIZE-1] != 0 && cBuffer[BUFFSIZE-1] != ' ' &&
        cBuffer[BUFFSIZE-1] != '/' && cBuffer[BUFFSIZE-1] != '\t')
        return FALSE;

    if((cBuffer[5] < '0') || (cBuffer[5] > '9'))
        return FALSE;

     cBuffer[BUFFSIZE-1] = 0;           /* ensure null termination */

    if(DclStrLen(cBuffer) != BUFFSIZE-1)
        return FALSE;

    if(DclStrNICmp(cBuffer, "/DISK", 5) != 0)
        return FALSE;

    /*  The /DISKn values range from 0 to 9. Range check done earlier,
        OK to typecast:
    */
    *puDriveNumber = (D_UINT16)DclAtoI(&pszString[5]);

    if(puStringLen)
        *puStringLen = 6;

    return TRUE;
}


/*-------------------------------------------------------------------
    Local: DisplayParamValue()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
static void DisplayParamValue(
    FFXPARAM    idParam,
    const char *pBuffer,
    D_UINT32    ulLen)
{
    #define     MAXPARAMNAME    (24)
    char        szParamName[MAXPARAMNAME];
    unsigned    nDefaultWidth = 16;    

    DclAssert(pBuffer);
    DclAssert(ulLen);

    switch(idParam)
    {
        case FFXPARAM_FIM_CHIPID:
            DclPrintf("Parameter CHIPID for %u chip(s):\n", (ulLen+(FFXPARAM_CHIPID_LENGTH-1)) / FFXPARAM_CHIPID_LENGTH);
            DclHexDump(NULL, HEXDUMP_UINT8 | HEXDUMP_NOOFFSET | HEXDUMP_NOASCII, FFXPARAM_CHIPID_LENGTH, ulLen, pBuffer);
            return;

        case FFXPARAM_FIM_CHIPSN:
            DclPrintf("Parameter CHIPSN for %u chip(s):\n", (ulLen+(FFXPARAM_CHIPSN_LENGTH-1)) / FFXPARAM_CHIPSN_LENGTH);
            DclHexDump(NULL, HEXDUMP_UINT8 | HEXDUMP_NOOFFSET | HEXDUMP_NOASCII, FFXPARAM_CHIPSN_LENGTH, ulLen, pBuffer);
            return;

        case FFXPARAM_FIM_CORRECTEDERRORS:
            DclStrNCpy(szParamName, "ECCCORRECTED", sizeof(szParamName));
            break;
            
        case FFXPARAM_FIM_UNCORRECTABLEERRORS:
            DclStrNCpy(szParamName, "ECCUNCORRECTED", sizeof(szParamName));
            break;
            
      #if FFXCONF_LATENCYREDUCTIONENABLED
        case FFXPARAM_DEVMGR_MAXREADCOUNT:
            DclStrNCpy(szParamName, "DEVREADMAX", sizeof(szParamName));
            break;
        case FFXPARAM_DEVMGR_MAXWRITECOUNT:
            DclStrNCpy(szParamName, "DEVWRITEMAX", sizeof(szParamName));
            break;
        case FFXPARAM_DEVMGR_MAXERASECOUNT:
            DclStrNCpy(szParamName, "DEVERASEMAX", sizeof(szParamName));
            break;
        case FFXPARAM_DEVMGR_ERASEPOLLINTERVAL:
            DclStrNCpy(szParamName, "DEVERASEPOLLTIME", sizeof(szParamName));
            break;
        case FFXPARAM_DEVMGR_CYCLEMUTEX:
            DclStrNCpy(szParamName, "DEVCYCLEMUTEX", sizeof(szParamName));
            break;
      #endif
      
        default:
            DclSNPrintf(szParamName, sizeof(szParamName), "ID %x", idParam);
            break;
    }

    DclPrintf("Parameter %-24s:  ", szParamName);

    switch(ulLen)
    {
        case 1:
            DclPrintf("0x%02x\n", (unsigned)*pBuffer);
            break;
        case 2:
            DclPrintf("0x%04x\n", (unsigned)*(D_UINT16*)pBuffer);
            break;
        case 4:
            DclPrintf("%lX\n", *(D_UINT32*)pBuffer);
            break;
        case 8:
            DclPrintf("%llX\n", *(D_UINT64*)pBuffer);
            break;
        default:
            if(ulLen <= 16)
                DclHexDump(NULL, HEXDUMP_UINT8 | HEXDUMP_NOOFFSET, nDefaultWidth, ulLen, pBuffer);
            else
                DclHexDump("\n", HEXDUMP_UINT8, nDefaultWidth, ulLen, pBuffer);
            break;
    }

    return;
}



#endif  /* DCLCONF_COMMAND_SHELL */

