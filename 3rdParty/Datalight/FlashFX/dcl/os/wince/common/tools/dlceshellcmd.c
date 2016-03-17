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

    This module implements CE OS specific shell commands.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlceshellcmd.c $
    Revision 1.7  2011/04/22 19:59:09Z  garyp
    Added support for the FileSpy functionality.
    Revision 1.6  2010/12/10 03:09:26Z  garyp
    Removed an unused prototype.
    Revision 1.5  2010/12/10 02:58:18Z  garyp
    Added the "Vol" command.
    Revision 1.4  2010/09/23 18:36:02Z  garyp
    Updated to build cleanly with CE 5.0.
    Revision 1.3  2010/09/23 05:47:34Z  garyp
    Added the SetPowerState and WarmBoot commands.
    Revision 1.2  2009/02/08 01:28:09Z  garyp
    Merged from the v4.0 branch.  Updated to add "OS" commands rather than
    "FileSystem" commands.  The former in turn will add the latter.
    Revision 1.1  2007/12/14 02:12:18Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pkfuncs.h>
#include <pm.h>

#include <dcl.h>

#if DCLCONF_COMMAND_SHELL

#include <dltools.h>
#include <dlerrlev.h>
#include <dlshell.h>
#include <dlcetools.h>
#include <dlwinutil.h>
#include <dlapireq.h>

#define MAX_COMMAND_LINE    (256)


static DCLSTATUS    FileSpy( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    LoadDrv( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    Part(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    SetPower(DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS    WarmBoot(DCLSHELLHANDLE hShell, int argc, char **argv);
#if _WIN32_WCE >= 500 
static DCLSTATUS    Vol(     DCLSHELLHANDLE hShell, int argc, char **argv);
#endif

static const DCLSHELLCOMMAND aCECmds[] =
{
    {"OS", "%sFileSpy",       "Get/Set DCL File Spy settings",                FileSpy},
    {"OS", "%sLoadDriver",    "Load a device driver and activate it",         LoadDrv},
    {"OS", "%sPart",          "Display and manipulate stores and partitions", Part},
    {"OS", "%sSetPowerState", "Set the system power state",                   SetPower},
    {"OS", "%sWarmBoot",      "Warm boot the system",                         WarmBoot},
  #if _WIN32_WCE >= 500 
    {"OS", "%sVol",           "Display and manipulate volumes",               Vol}
  #endif
};



/*-------------------------------------------------------------------
    Protected: DclShellAddOsCommands()

    Add OS-specific commands to the specified shell instance.

    Parameters:
        hShell       - The shell handle.
        pszClassName - A pointer to the null-terminated command
                       class name to use.
        pszPrefix    - A pointer to the null-terminated command
                       prefix to use.

    Return Value:
        Returns a DCLSTATUS value indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclShellAddOsCommands(
    DCLSHELLHANDLE  hShell,
    const char     *pszClassName,
    const char     *pszPrefix)
{
    DCLSTATUS       dclStat;

    if(!hShell)
    {
        DclError();
        return DCLSTAT_BADPARAMETER;
    }

    DclAssert(pszClassName);
    DclAssert(pszPrefix);

    dclStat = DclShellAddMultipleCommands(hShell, aCECmds, DCLDIMENSIONOF(aCECmds), pszClassName, pszPrefix);
    if(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Add the generic "win" commands
        */
        dclStat = DclWinShellAddCommands(hShell, pszClassName, pszPrefix);
        if(dclStat == DCLSTAT_SUCCESS)
        {
            /*  For some bizarre reason that only MS knows, in the CE Target
                Control window, the word "Exit" is parsed before we even get
                a chance to see it.  Even worse, CE just displays an error,
                and doesn't seem to actually do anything.  This makes it
                impossible to exit the shell using that command.

                Therefore, for CE, rename the EXIT command to QUIT.
            */
            dclStat = DclShellRenameCommand(hShell, "Exit", "Quit");
            DclAssert(dclStat == DCLSTAT_SUCCESS);
        }
    }

    return dclStat;
}



                    /*------------------------------*\
                     *                              *
                     *  CE Specific Shell Commands  *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
 static DCLSTATUS FileSpy(
    DCLSHELLHANDLE          hShell,
    int                     argc,
    char                  **argv)
{
    DCLOSREQHANDLE          hReq;
    DCLSTATUS               dclStat;
    DCLDECLAREREQUESTPACKET (FILESPY, TRACEMASK, SpyMask);  /* DCLREQ_FILESPY_TRACEMASK */

    if(argc < 2 || argc > 3 || (argc >= 2 && ((DclStrCmp(argv[1], "/?") == 0) || (DclStrCmp(argv[1], "?") == 0))))
        goto Syntax;

    dclStat = DclOsRequestorOpen(argv[1], DCLIODEVICE_FILESYSTEM, &hReq);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error %lX attempting to open file system device \"%s\".\n", dclStat, argv[1]);
        return dclStat;
    }

    if(argc == 2)
    {
        /*  Set a flag that we are simply querying the existing setting
        */
        SpyMask.ulNewTraceMask = D_UINT32_MAX;
    }
    else
    {
        const char *pStr = argv[2];

        pStr = DclNtoUL(pStr, &SpyMask.ulNewTraceMask);
        if(!pStr || *pStr)
        {
            DclPrintf("Invalid option \"%s\"\n", argv[2]);
            DclOsRequestorClose(hReq);
            return DCLSTAT_BADSYNTAX;
        }
    }

    dclStat = DclOsRequestorDispatch(hReq, &SpyMask.ior);
    if(dclStat == DCLSTAT_SUCCESS && SpyMask.dclStat == DCLSTAT_SUCCESS)
    {
        if(argc == 2)
            DclPrintf("DLFILESPY mask flags value is %lX\n", SpyMask.ulOldTraceMask);
        else
            DclPrintf("DLFILESPY mask flags value was %lX, now set to %lX\n", SpyMask.ulOldTraceMask, SpyMask.ulNewTraceMask);
    }
    else
    {
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = SpyMask.dclStat;
        
        DclPrintf("Error %lX attempting to process request.\n", dclStat);
    }

    DclOsRequestorClose(hReq);

    return dclStat;

  Syntax:

    DclPrintf("This command is used to get/set the event flags mask for the DCL File\n");
    DclPrintf("Spy Filter.  The filter must already be loaded.\n\n");
    DclPrintf("Syntax:  %s VolumeName [EventMask]\n\n", argv[0]);
    DclPrintf("Where:\n");
    DclPrintf("  VolumeName - The name of the volume to which the filter applies.\n");
    DclPrintf("   EventMask - The event flags mask.  If this argument is not used,\n");
    DclPrintf("               the command will simply display the current settings.\n");

    return DCLSTAT_HELPREQUEST;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS LoadDrv(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achCmdLine[MAX_COMMAND_LINE];
    int             ii;

    achCmdLine[0] = 0;

    for(ii=1; ii<argc; ii++)
    {
        DclStrCat(achCmdLine, argv[ii]);
        DclStrCat(achCmdLine, " ");
    }

    return DclCeDriverLoad(argv[0], achCmdLine);
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS Part(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achCmdLine[MAX_COMMAND_LINE];
    int             ii;

    achCmdLine[0] = 0;

    for(ii=1; ii<argc; ii++)
    {
        DclStrCat(achCmdLine, argv[ii]);
        DclStrCat(achCmdLine, " ");
    }

    return DclCePartition(argv[0], achCmdLine);
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS SetPower(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DWORD           dwState;
    DWORD           dwResult;

    if(argc != 2 || ((DclStrCmp(argv[1], "/?") == 0) || (DclStrCmp(argv[1], "?") == 0)))
    {
        DclPrintf("Set the system power state to one of the following values:\n");
        DclPrintf("  ON, OFF, CRITICAL, BOOT, IDLE, SUSPEND, RESET, PASSWORD,\n");
      #if _WIN32_WCE >= 502
        DclPrintf("  UNATTENDED, USERIDLE, BACKLIGHTON\n");
      #endif

        return DCLSTAT_HELPREQUEST;
    }

    if(DclStrICmp(argv[1], "ON") == 0)
        dwState = POWER_STATE_ON;
    else if(DclStrICmp(argv[1], "OFF") == 0)
        dwState = POWER_STATE_OFF;
    else if(DclStrICmp(argv[1], "CRITICAL") == 0)
        dwState = POWER_STATE_CRITICAL;
    else if(DclStrICmp(argv[1], "BOOT") == 0)
        dwState = POWER_STATE_BOOT;
    else if(DclStrICmp(argv[1], "IDLE") == 0)
        dwState = POWER_STATE_IDLE;
    else if(DclStrICmp(argv[1], "SUSPEND") == 0)
        dwState = POWER_STATE_SUSPEND;
    else if(DclStrICmp(argv[1], "RESET") == 0)
        dwState = POWER_STATE_RESET;
    else if(DclStrICmp(argv[1], "PASSWORD") == 0)
        dwState = POWER_STATE_PASSWORD;
  #if _WIN32_WCE >= 502
    else if(DclStrICmp(argv[1], "UNATTENDED") == 0)
          dwState = POWER_STATE_UNATTENDED;
    else if(DclStrICmp(argv[1], "USERIDLE") == 0)
        dwState = POWER_STATE_USERIDLE;
    else if(DclStrICmp(argv[1], "BACKLIGHTON") == 0)
        dwState = POWER_STATE_BACKLIGHTON;
  #endif  
    else
    {
        DclPrintf("Unrecognized power state \"%s\"\n", argv[1]);
        return DCLSTAT_BADSYNTAX;
    }

    dwResult = SetSystemPowerState(NULL, dwState, POWER_FORCE);
    if(dwResult != ERROR_SUCCESS)
    {
        DclPrintf("SetSystemPowerState() failed with error %lU\n", dwResult);
        return DCLSTAT_FAILURE;
    }

    return DCLSTAT_SUCCESS;
}


#if _WIN32_WCE >= 500 /* CeGetVolumeInfo() was introduced in 5.0 */

/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS Vol(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    char            achCmdLine[MAX_COMMAND_LINE];
    int             ii;

    achCmdLine[0] = 0;

    for(ii=1; ii<argc; ii++)
    {
        DclStrCat(achCmdLine, argv[ii]);
        DclStrCat(achCmdLine, " ");
    }

    return DclCeVolume(argv[0], achCmdLine);
}

#endif


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
 static DCLSTATUS WarmBoot(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DWORD           dwBytesReturned = 0;

    if(argc == 2 && ((DclStrCmp(argv[1], "/?") == 0) || (DclStrCmp(argv[1], "?") == 0)))
    {
        DclPrintf("Delay 5 seconds and then perform a warm system restart\n");

        return DCLSTAT_HELPREQUEST;
    }

    DclPrintf("System attempting a warm boot in 5 seconds...\n");

    DclOsSleep(5000);
    
    KernelIoControl(IOCTL_HAL_REBOOT, NULL, 0, NULL, 0, &dwBytesReturned);

    return DCLSTAT_FAILURE;
}




#endif

