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

    This module contains file system related shell commands.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlshellfs.c $
    Revision 1.26  2011/04/17 21:05:54Z  garyp
    Updated the directory listing stats to work better on very large volumes.
    Revision 1.25  2011/03/02 02:15:45Z  garyp
    Properly conditioned the code to build cleanly if threading is turned off.
    Use the VA64BUG() macro where appropriate.  Fixed the "Del" command to
    display an error message if it fails.
    Revision 1.24  2010/04/17 22:37:34Z  garyp
    Fixed the DIR command's /H and /S options to not be case sensitive.
    Enhanced the DIR command's output summary to be more user-friendly.
    Revision 1.23  2010/04/12 03:11:32Z  garyp
    Updated the DirRemove command to support removing entire directory trees.
    Revision 1.22  2009/11/02 20:29:34Z  garyp
    Fixed the FSSTRESSTEST command to properly initialize the DCL instance
    handle.
    Revision 1.21  2009/06/27 23:20:17Z  garyp
    Updated for functions which now take a DCL Instance handle.
    Revision 1.20  2009/06/11 16:51:10Z  garyp
    Added the FS stress test.
    Revision 1.19  2009/05/02 18:57:00Z  garyp
    Corrected a typo.
    Revision 1.18  2009/05/02 18:20:40Z  garyp
    Resolved Bug 2278 where filenames with printf() format specifies displayed
    incorrectly.  Reverted the changes from rev 1.16 as the real problem lies
    at the lower layer, as described in Bug 2260.
    Revision 1.17  2009/05/02 18:16:01Z  garyp
    Updated to use preserve the original status codes and display them where
    possible.  Updated to use correct Hungarian notation.
    Revision 1.16  2009/04/27 22:50:38Z  johnbr
    Improved the EOF handling in the type and copy commands.
    Revision 1.15  2009/04/27 15:55:29Z  garyp
    Changed the DIR command's /R option to /S to be more similar to the
    "standard" DIR command.
    Revision 1.14  2009/04/25 23:15:58Z  brandont
    Added a stand alone hash command.  Added a hash option to the dir command.
    Revision 1.13  2009/04/25 21:51:57Z  brandont
    Added recursive dir support.
    Revision 1.12  2009/02/09 06:11:07Z  garyp
    Removed a reference to an obsolete variable.
    Revision 1.11  2009/02/08 00:49:25Z  garyp
    Merged from the v4.0 branch.  Added error handling and error messages
    that are useful.  No longer throw away the detailed status code and return
    DCLSTAT_FAILURE.
    Revision 1.10  2009/01/26 23:18:56Z  keithg
    Added explicit void of unused formal parameter(s).
    Revision 1.9  2008/04/07 15:54:28  brandont
    Updated to use the new DCL file system services.
    Revision 1.8  2008/04/04 00:10:56Z  brandont
    Changed the uAttributes field of the DCLFSSTAT structure to be 32-bits.
    Revision 1.7  2008/04/03 23:55:32Z  brandont
    Updated all defines and structures used by the DCL file system services to
    use the DCLFS prefix.
    Revision 1.6  2007/12/18 20:32:36Z  brandont
    Updated function headers.
    Revision 1.5  2007/12/05 02:48:42Z  Garyp
    Updated to declare the shell command const.
    Revision 1.4  2007/11/03 23:31:13Z  Garyp
    Added the standard module header.
    Revision 1.3  2007/10/25 16:55:10Z  michaelm
    changed file opens to open in binary mode
    Revision 1.2  2007/10/05 19:51:21Z  brandont
    Corrected uninitialized variable warning in DclShellAddFileSystemCommands.
    Revision 1.1  2007/08/28 17:56:22Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <dcl.h>
#include <dlshell.h>

#if DCLCONF_COMMAND_SHELL

#include <dltools.h>
#include <test/dltfsstress.h>
#include "dlshl.h"


/*  Command execution prototypes
*/
static DCLSTATUS CmdDirChange(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdDirCreate(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdDirList(     DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdDirRename(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdDirRemove(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileAppend(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileCompare( DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileCopy(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileCreate(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileDelete(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileRename(  DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileType(    DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileWrite(   DCLSHELLHANDLE hShell, int argc, char **argv);
static DCLSTATUS CmdFileHash(    DCLSHELLHANDLE hShell, int argc, char **argv);
#if DCLCONF_SHELL_TESTS
static DCLSTATUS CmdFsioTest(    DCLSHELLHANDLE hShell, int argc, char **argv);
#if DCL_OSFEATURE_THREADS
static DCLSTATUS CmdFsStressTest(DCLSHELLHANDLE hShell, int argc, char **argv);
#endif
#endif

/*  Helper functions
*/
static DCLSTATUS WriteInputToFile(DCLSHELLHANDLE hShell, DCLFSFILEHANDLE hFile, D_UINT32 ulSize);
static DCLSTATUS FsCreateFile(    DCLSHELLHANDLE hShell, const char *pszName, D_UINT32 ulSize, D_UINT8 uData);
static DCLSTATUS FsCopyFile(      DCLSHELLHANDLE hShell, const char *pszSrc, const char *pszDest);
static DCLSTATUS FileHashCompute( DCLSHELLHANDLE hShell, char *pszPath, D_UINT32 *pulCrc32);
static DCLSTATUS DirRecurse(      DCLSHELLHANDLE hShell, D_BOOL fRecurse, D_BOOL fShowFileHash, char *pszPath, size_t nDimOfPath, char *pszName, size_t nDimOfName, D_UINT64 *pullAccumNumFiles, D_UINT64 *pullAccumNumDirs, D_UINT64 *pullAccumTotalBytes);


static const DCLSHELLCOMMAND aFileSysCmds[] =
{
    {"Directory", "Dir",          "Display a directory listing",                    CmdDirList},
    {"Directory", "CD",           "Change the current directory",                   CmdDirChange},
    {"Directory", "MD",           "Make a directory",                               CmdDirCreate},
    {"Directory", "RD",           "Remove a directory",                             CmdDirRemove},
    {"Directory", "RenDir",       "Rename a directory",                             CmdDirRename},
  #if DCLCONF_SHELL_TESTS
    {"DCL",       "FSIOTest",     "Run the File System I/O tests",                  CmdFsioTest},
  #if DCL_OSFEATURE_THREADS
    {"DCL",       "FSStressTest", "Run the File System Multithreaded Stress tests", CmdFsStressTest},
  #endif
  #endif
    {"File",      "Append",       "Append data from the console to a file",         CmdFileAppend},
    {"File",      "Compare",      "Compare the contents of two files",              CmdFileCompare},
    {"File",      "Hash",         "Compute a hash on the contents of a file",       CmdFileHash},
    {"File",      "Copy",         "Copy a file",                                    CmdFileCopy},
    {"File",      "Create",       "Create a file",                                  CmdFileCreate},
    {"File",      "Del",          "Delete a file",                                  CmdFileDelete},
    {"File",      "Ren",          "Rename a file",                                  CmdFileRename},
    {"File",      "Type",         "Display the contents of a file on the console",  CmdFileType},
    {"File",      "Write",        "Write to a file using console input",            CmdFileWrite}
};


/*-------------------------------------------------------------------
    Protected: DclShellAddFileSystemCommands()

    Add file system specific commands to the specified shell instance.

    Parameters:
        hShell      - The handle of the shell instance to use.

    Return Value:
        Returns a DCLSTATUS code indicating the operation result.
-------------------------------------------------------------------*/
DCLSTATUS DclShellAddFileSystemCommands(
    DCLSHELLHANDLE  hShell)
{
    unsigned        nn;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    DclAssert(hShell);

    for(nn = 0; nn < DCLDIMENSIONOF(aFileSysCmds); nn++)
    {
        dclStat = DclShellAddCommand(hShell, &aFileSysCmds[nn]);
        if(dclStat != DCLSTAT_SUCCESS)
            break;
     }

    return dclStat;
}


                    /*------------------------------*\
                     *                              *
                     *     File System Commands     *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdDirList(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLFSSTATFS     sStatFs;
    char            szPath[DLS_PATHLEN+1];
    char            szName[DLS_PATHLEN+1];
    D_UINT64        ullNumFiles = 0;
    D_UINT64        ullNumDirs = 0;
    D_UINT64        ullUsedBytes = 0;
    D_BOOL          fRecurse = FALSE;
    D_BOOL          fShowHash = FALSE;
    int             nSwitches = 0;
    DCLSTATUS       dclStat;

    while(argc > 1 + nSwitches)
    {
        if(DclStrICmp(argv[1 + nSwitches], "/S") == 0)
        {
            fRecurse = TRUE;
            nSwitches++;
        }
        else if(DclStrICmp(argv[1 + nSwitches], "/H") == 0)
        {
            fShowHash = TRUE;
            nSwitches++;
        }
        else
        {
            break;
        }
    }

    /*  Display help screen
    */
    if(argc > 2 + nSwitches || ISHELPREQUEST())
    {
        DclPrintf("This command lists the contents of a directory.\n\n");
        DclPrintf("Syntax:  %s [/S] [/H] [Path]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("    /S - Recursing into subdirectories.\n");
        DclPrintf("    /H - Display a hash for each file.\n");
        DclPrintf("  Path - Specifies the name/path of directory to list.  If path is omitted,\n");
        DclPrintf("         the current working directory is used.\n\n");
        DclPrintf("Examples:  %s dir1\n", argv[0]);
        DclPrintf("           %s /S dir1\n", argv[0]);
        DclPrintf("           %s /H dir1\n", argv[0]);
        DclPrintf("           %s /S /H dir1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    if(argc > 1 + nSwitches)
    {
        /*  Use the specified dir
        */
        DclStrCpy(szPath, argv[1 + nSwitches]);
    }
    else
    {
        szPath[0] = 0;

        /*  Use the current dir
        */
        if(hShell->fCurDirSupport)
            DclFsDirGetWorking(szPath, DCLDIMENSIONOF(szPath));
    }

    dclStat = DclFsStatFs(szPath, &sStatFs);

    /*  Display the contents of this directory and recurse into
        subdirectories if requested.
    */
    dclStat = DirRecurse(hShell,
                         fRecurse,
                         fShowHash,
                         szPath,
                         DCLDIMENSIONOF(szPath),
                         szName,
                         DCLDIMENSIONOF(szName),
                         &ullNumFiles,
                         &ullNumDirs,
                         &ullUsedBytes);

    /*  Show disk usage
    */
    dclStat = DclFsStatFs(szPath, &sStatFs);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Command failed, Status=%lX\n", dclStat);
    }
    else
    {
        char        szSizeBuff[16];
        D_UINT64    ullTotalBytes;
        D_UINT64    ullFreeBytes;

        ullTotalBytes  = sStatFs.ulTotalBlocks;
        ullTotalBytes *= sStatFs.ulBlockSize;
        
        ullFreeBytes  = sStatFs.ulFreeBlocks;
        ullFreeBytes *= sStatFs.ulBlockSize;
        
        if(fRecurse)
        {
            DclPrintf("Totals: %10llU File(s) %14llU Bytes -- %11s\n", 
                VA64BUG(ullNumFiles), VA64BUG(ullUsedBytes),
                DclScaleItems(ullUsedBytes, 1, szSizeBuff, sizeof(szSizeBuff)));
            DclPrintf("        %10llU Directories\n", VA64BUG(ullNumDirs+1));
        }

        DclPrintf("\nVolume Information:\n");
        if(sStatFs.szDeviceName[0])
            DclPrintf("  Resides on Block Device: \"%10s\"\n", sStatFs.szDeviceName);
        DclPrintf("  File System Block Size:    %6lU Bytes\n", sStatFs.ulBlockSize);
        DclPrintf("  Total Logical Blocks:  %10lU -> Total Size: %11s\n", 
            sStatFs.ulTotalBlocks, DclScaleItems(ullTotalBytes, 1, szSizeBuff, sizeof(szSizeBuff)));
        DclPrintf("  Free Logical Blocks:   %10lU -> Free Space: %11s\n", 
            sStatFs.ulFreeBlocks, DclScaleItems(ullFreeBytes, 1, szSizeBuff, sizeof(szSizeBuff)));
    }
    
    DclPrintf("\n");

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdDirChange(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command changes the current working directory (CWD).\n\n");
        DclPrintf("Syntax:  %s Path\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  Path - Is the Path/Name of new CWD.  To change devices, the device name\n");
        DclPrintf("         must be included in the path.\n\n");
        DclPrintf("Examples:  %s dir1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    if(hShell->fCurDirSupport)
    {
        DCLSTATUS   dclStat;

        /*  Change the current directory
        */
        dclStat = DclFsDirSetWorking(argv[1]);
        if(dclStat != DCLSTAT_SUCCESS)
            DclPrintf("Directory change failed, Status=%lX\n", dclStat);

        return dclStat;
    }
    else
    {
        DclPrintf("This OS abstraction does not support setting the CWD\n");
        return DCLSTAT_FAILURE;
    }
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdDirCreate(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /* Explicitly unused parameter */
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command creates (makes) a directory.\n\n");
        DclPrintf("Syntax:  %s DirName\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  DirName - Is the Path/Name of directory to create\n\n");
        DclPrintf("Examples:  %s dir1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    dclStat = DclFsDirCreate(argv[1]);
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("Directory creation failed, Status=%lX\n", dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdDirRename(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /* Explicitly unused parameter */
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 3 || ISHELPREQUEST())
    {
        DclPrintf("This command renames a directory.\n\n");
        DclPrintf("Syntax:  %s OldDirName NewDirName\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  OldDirName - Is the name of directory to rename\n");
        DclPrintf("  NewDirName - Is the new name of the directory\n\n");
        DclPrintf("Examples:  %s dir1 dir2\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  Check the arg count.
    */
    if(argc < 3)
         return DCLSTAT_BADSYNTAX;

    dclStat = DclFsDirRename(argv[1], argv[2]);
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("Directory rename failed, Status=%lX\n", dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdDirRemove(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;
    D_BOOL          fSubDirs = FALSE;
    D_BOOL          fHiddenSystem = FALSE;
    D_BOOL          fReadonly = FALSE;
    D_BOOL          fQuiet = FALSE;
    D_BOOL          fContinueOnError = FALSE;
    int             iArgNum;

    /*  Explicitly unused parameter
    */
    (void)hShell;

    /*  Display help screen
    */
    if(argc < 2 || argc > 6 || ISHELPREQUEST())
    {
        DclPrintf("This command removes a directory, and optionally can remove an entire\n");
        DclPrintf("directory tree.\n\n");
        DclPrintf("Syntax:  %s DirName [/S [/H] [/R] [/C] [/Q]]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  DirName - The Path/Name of directory to remove\n");
        DclPrintf("       /S - Delete the specified directory tree, including all files and\n");
        DclPrintf("            subdirectories.\n");
        DclPrintf("       /H - Process Hidden and System files/directories as well.\n");
        DclPrintf("       /R - Delete Readonly files/directories.\n");
        DclPrintf("       /C - Continue even if errors occur.\n");
        DclPrintf("       /Q - Operate Quietly, with summary and error information only.\n\n");
        DclPrintf("NOTE: The /H, /R, /C, and /Q options are only valid when used with the\n");
        DclPrintf("      /S option, and are ignored otherwise.\n\n");
        DclPrintf("Examples:  %s DirName\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    iArgNum = 2;
    while(iArgNum < argc)
    {
        if(DclStrICmp(argv[iArgNum], "/S") == 0)
            fSubDirs = TRUE;
        else if(DclStrICmp(argv[iArgNum], "/H") == 0)
            fHiddenSystem = TRUE;
        else if(DclStrICmp(argv[iArgNum], "/R") == 0)
            fReadonly = TRUE;
        else if(DclStrICmp(argv[iArgNum], "/C") == 0)
            fContinueOnError = TRUE;
        else if(DclStrICmp(argv[iArgNum], "/Q") == 0)
            fQuiet = TRUE;
        else
            goto BadOptionr;

        iArgNum++;
    }

    if(fSubDirs)
    {
        dclStat = DclFsDirTreeDelete(argv[1], 
            fQuiet ? DCL_VERBOSE_NORMAL : DCL_VERBOSE_LOUD,
            fHiddenSystem, fReadonly, fContinueOnError);
    }
    else
    {
        dclStat = DclFsDirRemove(argv[1]);
    }

    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("Directory remove failed, Status=%lX\n", dclStat);
   
    return dclStat;

  BadOptionr:
    DclPrintf("Unrecognized option \"%s\"\n", argv[iArgNum]);
    return DCLSTAT_BADPARAMETER;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileAppend(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv )
{
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    D_UINT32        ulCount = D_UINT32_MAX;

    /*  Display help screen
    */
    if(argc < 2 || argc > 3 || ISHELPREQUEST())
    {
        DclPrintf("This command appends data from the console to a file.\n\n");
        DclPrintf("Syntax:  %s FileSpec [Count]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec - Is the file specification for the file to append to\n");
        DclPrintf("  Count    - Is the number of bytes to append.  If count is not specified,\n");
        DclPrintf("             then one whole line of console input will be appended.\n\n");
        DclPrintf("Examples:  %s file1 5\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    if(argc == 3)
        ulCount = DclAtoL(argv[2]);

    /*  open/create the destination file
    */
    dclStat = DclFsFileOpen(argv[1], "r+b", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error opening file, Status=%lX\n", dclStat);
        return dclStat;
    }

    /*  Write to the file
    */
    DclOsFileSeek(hFile, 0, DCLFSFILESEEK_END);
    dclStat = WriteInputToFile(hShell, hFile, ulCount);

    /*  Close the file
    */
    DclFsFileClose(hFile);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileCreate(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT32        ulSize = 0;
    D_UINT8         cData = 'A';
    DCLSTATUS       dclStat;

    /*  Display help screen
    */
    if(argc < 2 || ISHELPREQUEST())
    {
        DclPrintf("This command creates a file of the specified length and contents.\n");
        DclPrintf("Syntax:  %s FileSpec [Size] [Data]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec - Is the file specification (optional path and name) of the new\n");
        DclPrintf("             file.\n");
        DclPrintf("  Size     - The optional size of the file in bytes.  If this is not specified,\n");
        DclPrintf("             a zero length file will be created\n");
        DclPrintf("  Data     - The optional ASCII character to write to file.  If this not\n");
        DclPrintf("             specified, the letter 'A' will be used.\n\n");
        DclPrintf("Examples:  %s file1 1024 Z\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  Get optional parameters
    */
    if(argc >= 3)
    {
        ulSize = DclAtoL(argv[2]);
    }
    if(argc >= 4)
    {
        cData = argv[3][0];
    }

    dclStat =  FsCreateFile(hShell, argv[1], ulSize, cData);
    if (dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to create file '%s', Status=%lX\n", argv[1], dclStat);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS FileHashCompute(
    DCLSHELLHANDLE  hShell,
    char           *pszPath,
    D_UINT32       *pulCrc32)
{
    #define         CRC_BUFFER_SIZE (16 * 1024)
    DCLFSFILEHANDLE hFile1;
    D_UINT32        ulBytesRead;
    void           *pData1;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    /* Explicitly unused parameter */
    ((void)hShell);

    DclAssert(pszPath);
    DclAssert(pulCrc32);
    *pulCrc32= 0;
    pData1 = DclMemAlloc(CRC_BUFFER_SIZE);
    if(!pData1)
    {
        DclPrintf("Insufficient memory\n");
        dclStat = DCLSTAT_MEMALLOCFAILED;
    }
    else
    {
        /*  open file1
        */
        dclStat = DclFsFileOpen(pszPath, "rb", &hFile1);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Unable to open file '%s', Status=%lX\n", pszPath, dclStat);
        }
        else
        {
            /*  Read from the file and compute the CRC.
            */
            while(dclStat == DCLSTAT_SUCCESS)
            {
                /*  Read from file1.
                */
                dclStat = DclFsFileRead(
                        hFile1,
                        pData1,
                        CRC_BUFFER_SIZE,
                        &ulBytesRead);
                if(dclStat == DCLSTAT_FS_EOF)
                {
                    dclStat = DCLSTAT_SUCCESS;
                    break;
                }
                if((dclStat != DCLSTAT_SUCCESS) || (ulBytesRead == 0))
                {
                    DclPrintf("Unexpected error reading file '%s', Status=%lX Read=%lU\n",
                            pszPath, dclStat, ulBytesRead);
                    break;
                }

                /*  Compare the buffers.
                */
                *pulCrc32 = DclCrc32Update(
                        *pulCrc32,
                        pData1,
                        ulBytesRead);

                if(ulBytesRead < CRC_BUFFER_SIZE)
                {
                    break;
                }
            }

            /*  Close the file
            */
            DclFsFileClose(hFile1);
        }

        DclMemFree(pData1);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileHash(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    D_UINT32        ulCrc32 = 0;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    /* Explicitly unused parameter */
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command computes a hash (32-bit CRC) on the contents of a file.\n\n");
        DclPrintf("Syntax:  %s FileSpec1\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec1 - Is the file specification of the first file to compute a hash\n");
        DclPrintf("Examples:  %s file1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  Compute the file hash
    */
    dclStat = FileHashCompute(
            hShell,
            argv[1],
            &ulCrc32);

    /*  Display the result.
    */
    if(dclStat == DCLSTAT_SUCCESS)
    {
        DclPrintf("File hash is %lX\n", ulCrc32);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileCompare(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLFSFILEHANDLE hFile1;
    DCLFSFILEHANDLE hFile2;
    size_t          nBytesRead;
    char            pData1[40];
    char            pData2[40];
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;

    /* Explicitly unused parameter */
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 3 || ISHELPREQUEST())
    {
        DclPrintf("This command compares the contents of two files.\n\n");
        DclPrintf("Syntax:  %s FileSpec1 FileSpec2\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec1 - Is the file specification of the first file to compare\n");
        DclPrintf("  FileSpec2 - Is the file specification of the second file to compare\n\n");
        DclPrintf("Examples:  %s file1 file2\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  open file1
    */
    dclStat = DclFsFileOpen(argv[1], "rb", &hFile1);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to open file '%s', Status=%lX\n", argv[1], dclStat);
        return dclStat;
    }

    /*  open file1
    */
    dclStat = DclFsFileOpen(argv[2], "rb", &hFile2);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to open file '%s', Status=%lX\n", argv[2], dclStat);
        return dclStat;
    }

    /*  Read from both files and compare the bytes.
    */
    dclStat = DCLSTAT_SUCCESS;
    while(dclStat == DCLSTAT_SUCCESS)
    {
        /*  Clear both buffers.
        */
        DclMemSet(pData1, 0, sizeof(pData1));
        DclMemSet(pData2, 0, sizeof(pData2));

        /*  Read from file1.
        */
        nBytesRead = DclOsFileRead(pData1, 1, sizeof(pData1), hFile1);
        if(nBytesRead == 0)
        {
            if(DclOsFileRead(pData2, 1, 1, hFile2) == 1)
            {
                DclPrintf("File2 is longer than File1\n");
                dclStat = DCLSTAT_FAILURE;
            }
            break;
        }

        /*  Read from file2, only read the number of bytes read from file1.
            This allows us to do partial file compares.
        */
        if(DclOsFileRead(pData2, 1, nBytesRead, hFile2) != nBytesRead)
        {
            dclStat = DCLSTAT_FS_READFAILED;
            break;
        }

        /*  Compare the buffers.
        */
        if(DclMemCmp(pData1, pData2, nBytesRead) != 0)
        {
            dclStat = DCLSTAT_FAILURE;
            break;
        }
    }

    /*  Close both files
    */
    DclFsFileClose(hFile1);
    DclFsFileClose(hFile2);

    /*  Determine the result.
    */
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Files are different!\n");
    }
    else
    {
        DclPrintf("No differences encountered\n");
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileCopy(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /*  Display help screen
    */
    if(argc != 3 || ISHELPREQUEST())
    {
        DclPrintf("This command copies a file.\n\n");
        DclPrintf("Syntax:  %s SourceFileSpec DestFileSpec\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  SourceFileSpec - Is the source file to copy\n");
        DclPrintf("  DestFileSpec   - Is the destination file name\n\n");
        DclPrintf("NOTE: This command does not support wildcards\n");

        return DCLSTAT_SUCCESS;
    }

    /*  Check arg count.
    */
    if(argc < 3)
    {
        return DCLSTAT_FAILURE;
    }

	dclStat = FsCopyFile(hShell, argv[1], argv[2]);
    if (dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to copy file, Status=%lX\n", dclStat);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileDelete(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;
    
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command deletes a file.\n\n");
        DclPrintf("Syntax:  %s FileSpec\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec  - Is the Path/Name of file to delete\n\n");
        DclPrintf("Examples:  %s file1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    dclStat = DclFsFileDelete(argv[1]);
    if(dclStat != DCLSTAT_SUCCESS)
        DclPrintf("The \"%s\" command failed with status code %lX\n", argv[0], dclStat);

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileRename(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLSTATUS       dclStat;

    /* Explicitly unused parameter */
    ((void)hShell);

    /*  Display help screen
    */
    if(argc != 3 || ISHELPREQUEST())
    {
        DclPrintf("This command renames a file.\n\n");
        DclPrintf("Syntax:  %s OldFileSpec NewFileSpec\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  OldFileSpec - Is the file specification of the existing file to rename\n");
        DclPrintf("  NewFileSpec - Is the new file specification\n\n");
        DclPrintf("Examples:  %s file1 file2\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  Check the arg count.
    */
    if(argc < 3)
    {
        return DCLSTAT_BADSYNTAX;
    }

    dclStat = DclFsFileRename(argv[1], argv[2]);
    if (dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to rename '%s' to '%s', Status=%lX\n", argv[1], argv[2], dclStat);
    }

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileType(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLFSFILEHANDLE hFile;
    size_t          nBytesRead;
    DCLSTATUS       dclStat;

    /*  Display help screen
    */
    if(argc != 2 || ISHELPREQUEST())
    {
        DclPrintf("This command types the contents of a file to the console.\n\n");
        DclPrintf("Syntax:  %s FileSpec\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec - Is the file specification (optional path and name) of the file\n");
        DclPrintf("             to display.\n");
        DclPrintf("Examples:  %s file1\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    /*  Open the file.
    */
    dclStat = DclFsFileOpen(argv[1], "rb", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
        return dclStat;

    /*  Read and output the contents of the file.
    */
    while(TRUE)
    {
        /*  Read from the file.
        */
        nBytesRead = DclOsFileRead(hShell->pWorkBuff, 1, WORKBUFFSIZE - 1, hFile);
        if(nBytesRead == 0)
            break;

        hShell->pWorkBuff[nBytesRead] = 0;

        /*  Display the contents.  Use DclOutputString() rather than
            DclPrintf() because the former does not have any inherent
            internal length limit, whereas DclPrintf() will truncate
            after DCLOUTPUT_BUFFERLEN length.
        */
        DclOutputString(hShell->params.hDclInst, (const char *)hShell->pWorkBuff);
    }

    /*  Close the file.
    */
    DclFsFileClose(hFile);

    DclPrintf("\n");

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS CmdFileWrite(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv )
{
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    D_UINT32        ulCount = D_UINT32_MAX;

    /*  Display help screen
    */
    if(argc < 2 || argc > 3 || ISHELPREQUEST())
    {
        DclPrintf("This command writes data from the console a file.  If the file does not\n");
        DclPrintf("exist it will be created.  If the file already exists, the first \"Count\"\n");
        DclPrintf("bytes of the file will be overwritten.\n\n");
        DclPrintf("Syntax:  %s FileSpec [Count]\n\n", argv[0]);
        DclPrintf("Where:\n");
        DclPrintf("  FileSpec - Is the file specification to write to\n");
        DclPrintf("  Count    - Is the number of bytes to write.  If count is not specified,\n");
        DclPrintf("             then one whole line of console input will be written.\n\n");
        DclPrintf("Examples:  %s file1 5\n", argv[0]);

        return DCLSTAT_SUCCESS;
    }

    if(argc == 3)
        ulCount = DclAtoL(argv[2]);

    /*  open/create the destination file
    */
    dclStat = DclFsFileOpen(argv[1], "w+b", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Error opening file, Status=%lX\n", dclStat);
        return dclStat;
    }

    /*  Write to the file
    */
    dclStat = WriteInputToFile(hShell, hFile, ulCount);

    /*  Close the file
    */
    DclFsFileClose(hFile);

    return dclStat;
}


#if DCLCONF_SHELL_TESTS

/*-------------------------------------------------------------------
    Run the DCL FSIO Test
-------------------------------------------------------------------*/
static DCLSTATUS CmdFsioTest(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLTOOLPARAMS   tp = {0};
    char            szCmdLine[256];
    int             ii;
    DCLSTATUS       dclStat;

    /* Explicitly unused parameter
    */
    ((void)hShell);

    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(ii = 1; ii < argc; ii++)
    {
        DclStrCat(szCmdLine, argv[ii]);
        DclStrCat(szCmdLine, " ");
    }

    /*  Run the test
    */
    tp.hDclInst     = hShell->params.hDclInst;
    tp.pszCmdName = argv[0];
    tp.pszCmdLine = szCmdLine;

    dclStat = DclTestFSIOMain(&tp);
    if (dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("File System I/O Test failed with status %lX\n", dclStat);
    }

    return dclStat;
}


#if DCL_OSFEATURE_THREADS

/*-------------------------------------------------------------------
    Run the DCL FS Stress Test
-------------------------------------------------------------------*/
static DCLSTATUS CmdFsStressTest(
    DCLSHELLHANDLE  hShell,
    int             argc,
    char          **argv)
{
    DCLTOOLPARAMS   tp = {0};
    char            szCmdLine[256];
    int             ii;
    DCLSTATUS       dclStat;

    /*  Explicitly unused parameter
    */
    ((void)hShell);

    /*  Combine the args to a single string.
    */
    szCmdLine[0] = 0;
    for(ii = 1; ii < argc; ii++)
    {
        DclStrCat(szCmdLine, argv[ii]);
        DclStrCat(szCmdLine, " ");
    }

    /*  Run the test
    */
    tp.hDclInst   = hShell->params.hDclInst;
    tp.pszCmdName = argv[0];
    tp.pszCmdLine = szCmdLine;

    dclStat = DclTestFSStressMain(&tp);
    if (dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("File System Stress Test failed with status %lX\n", dclStat);
    }

    return dclStat;
}

#endif
#endif



                    /*------------------------------*\
                     *                              *
                     *      Helper Functions        *
                     *                              *
                    \*------------------------------*/


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS FsCopyFile(
    DCLSHELLHANDLE  hShell,
    const char     *pszSrc,
    const char     *pszDest)
{
    DCLFSFILEHANDLE hSrc;
    DCLFSFILEHANDLE hDest;
    size_t          nBytesRead;
    DCLTIMESTAMP    ts ;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    DCLFSSTAT       stat;
    D_UINT32        ulCopied = 0;

    DclPrintf("Copying: %s to %s\n", pszSrc, pszDest);

    ts = DclTimeStamp();

    dclStat = DclFsStat(pszSrc, &stat);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to obtain information for \"%s\", Status=%lX\n", pszSrc, dclStat);
        return dclStat;
    }

    /*  Open the source file.
    */
    dclStat = DclFsFileOpen(pszSrc, "rb", &hSrc);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to open source file \"%s\", Status=%lX\n", pszSrc, dclStat);
        return dclStat;
    }

    /*  Create the destination file.
    */
    dclStat = DclFsFileOpen(pszDest, "wb", &hDest);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to create destination file \"%s\", Status=%lX\n", pszDest, dclStat);

        DclFsFileClose(hSrc);
        return dclStat;
    }

    /*  Copy the source to the destination.
    */
    while(TRUE)
    {
        /*  Read from the source file.
        */
        nBytesRead = DclOsFileRead(hShell->pWorkBuff, 1, WORKBUFFSIZE, hSrc);
        if(nBytesRead == 0)
        {
            break;
        }

        /*  Write to the dest file.
        */
        if(DclOsFileWrite(hShell->pWorkBuff, 1, nBytesRead, hDest) != nBytesRead)
        {
            DclPrintf("Error writing to the destination file\n");
            dclStat = DCLSTAT_FS_WRITEFAILED;
            break;
        }

        ulCopied += nBytesRead;
    }

    if(ulCopied != stat.ulSize)
    {
        DclPrintf("Copy failed, only copied %lU of %lU bytes\n", ulCopied, stat.ulSize);
        if(dclStat == DCLSTAT_SUCCESS)
            dclStat = DCLSTAT_FS_READFAILED;
    }

    /*  Close both files.
    */
    DclFsFileClose(hSrc);
    DclFsFileClose(hDest);

    /*  Display the time.
    */
    DclPrintf("Time: %lU ms\n", DclTimePassed(ts));

    return dclStat;
}


/*-------------------------------------------------------------------
-------------------------------------------------------------------*/
static DCLSTATUS FsCreateFile(
    DCLSHELLHANDLE  hShell,
    const char     *pszName,
    D_UINT32        ulSize,
    D_UINT8         uData)
{
    DCLFSFILEHANDLE hFile;
    DCLSTATUS       dclStat = DCLSTAT_SUCCESS;
    D_UINT32        ulBytesLeft;
    D_UINT32        ulBytesToWrite;
    DCLTIMESTAMP    ts;

    DclPrintf("Creating: %s, %lU bytes\n", pszName, ulSize);

    ts = DclTimeStamp();

    /*  Open the file.
    */
    dclStat = DclFsFileOpen(pszName, "wb", &hFile);
    if(dclStat != DCLSTAT_SUCCESS)
    {
        DclPrintf("Unable to create file \"%s\", Status=%lX\n", pszName, dclStat);
        return dclStat;
    }

    /*  Write to the file.
    */
    DclMemSet(hShell->pWorkBuff, uData, WORKBUFFSIZE);
    ulBytesLeft = ulSize;
    while(ulBytesLeft)
    {
        ulBytesToWrite = WORKBUFFSIZE;
        if(ulBytesLeft < ulBytesToWrite)
        {
            ulBytesToWrite = ulBytesLeft;
        }

        if(DclOsFileWrite(hShell->pWorkBuff, 1, ulBytesToWrite, hFile) != ulBytesToWrite)
        {
            DclPrintf("Error writing to file\n");
            dclStat = DCLSTAT_FS_WRITEFAILED;
            break;
        }
        ulBytesLeft -= ulBytesToWrite;
    }

    /*  Close the file.
    */
    DclFsFileClose(hFile);

    /*  Display elapsed time.
    */
    DclPrintf("Time: %lU ms\n", DclTimePassed(ts));

    return dclStat;
}


/*-------------------------------------------------------------------
    NOTE: If ulSize is D_UINT32_MAX, exactly one line of console
          input will be written.
-------------------------------------------------------------------*/
static DCLSTATUS WriteInputToFile(
    DCLSHELLHANDLE  hShell,
    DCLFSFILEHANDLE hFile,
    D_UINT32        ulSize)
{
    D_UINT32        ulRemaining = ulSize;
    D_UINT32        ulBytesRead;

    while(ulRemaining)
    {
        /*  Get user input
        */
        ulBytesRead = DclShellConsoleInput(hShell, (char *)hShell->pWorkBuff, WORKBUFFSIZE);
        if(ulBytesRead == D_UINT32_MAX)
        {
            DclPrintf("Input error.\n");
            return DCLSTAT_FAILURE;
        }

        if(ulBytesRead > ulRemaining)
        {
            ulBytesRead = ulRemaining;
        }

        /*  Write this chunk of data to the file.
        */
        if(DclOsFileWrite(hShell->pWorkBuff, 1, ulBytesRead, hFile) != ulBytesRead)
        {
            DclPrintf("Error writing to file.\n");
            return DCLSTAT_FS_WRITEFAILED;
        }

        if(ulRemaining == D_UINT32_MAX)
        {
            DclOsFileWrite("\n", 1, 1, hFile);
            return DCLSTAT_SUCCESS;
        }

        ulRemaining -= ulBytesRead;
    }

    return DCLSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Display the contents of a directory.
-------------------------------------------------------------------*/
static DCLSTATUS DirRecurse(
    DCLSHELLHANDLE  hShell,
    D_BOOL          fRecurse,
    D_BOOL          fShowFileHash,
    char           *pszPath,
    size_t          nDimOfPath,
    char           *pszName,
    size_t          nDimOfName,
    D_UINT64       *pullAccumNumFiles,
    D_UINT64       *pullAccumNumDirs,
    D_UINT64       *pullAccumTotalBytes)
{
    DCLFSDIRHANDLE  hDir;
    DCLFSSTAT       sStat;
    DCLDATETIME     sDateTime;
    D_UINT32        ulNumFiles = 0;
    D_UINT32        ulNumDirs = 0;
    size_t          nPathLength;
    D_UINT64        ullUsedBytes = 0;
    DCLSTATUS       dclStat;
    char            szSizeBuff[16];

    /*  Open the directory
    */
    nPathLength = DclStrLen(pszPath);
    dclStat = DclFsDirOpen(pszPath, &hDir);
    if(dclStat != DCLSTAT_SUCCESS)
	{
        DclPrintf("Unable to open directory %s, Status=%lX\n", pszPath, dclStat);
        return dclStat;
	}
    
    /*  Enumerate the directory contents
    */
    DclPrintf("Directory Listing of \"%s\":\n", pszPath);

    do
    {
        char    szAttrib[5];

        dclStat = DclFsDirRead(hDir, pszName, nDimOfName, &sStat);
        if(dclStat != DCLSTAT_SUCCESS)
             break;

        /*  Time stamp
        */
        DclDateTimeDecode(&sStat.tModify, &sDateTime);
        DclPrintf("  %04u-%02u-%02u %02u:%02u:%02u.%03u ", sDateTime.uYear,
                  sDateTime.uMonth + 1, sDateTime.uDay, sDateTime.uHour,
                  sDateTime.uMinute, sDateTime.uSecond, sDateTime.uMillisecond);

        if(fShowFileHash)
        {
            if(sStat.ulAttributes & DCLFSATTR_DIRECTORY)
            {
                DclPrintf("         ");
            }
            else
            {
                D_UINT32 ulCrc32;

                pszPath[nPathLength] = 0;
                if(pszPath[nPathLength - 1] == DCL_PATHSEPCHAR)
                    DclSNPrintf(&pszPath[nPathLength], nDimOfPath - nPathLength, "%s", pszName);
                else
                    DclSNPrintf(&pszPath[nPathLength], nDimOfPath - nPathLength, "%c%s", DCL_PATHSEPCHAR, pszName);
                
                FileHashCompute(hShell, pszPath, &ulCrc32);
                pszPath[nPathLength] = 0;
                DclPrintf("%08lX ", ulCrc32);
            }
        }

        DclMemSet(szAttrib, ' ', sizeof(szAttrib));
        szAttrib[4] = 0;

        if(sStat.ulAttributes & DCLFSATTR_READONLY)
            szAttrib[0] = 'R';
        if(sStat.ulAttributes & DCLFSATTR_HIDDEN)
            szAttrib[1] = 'H';
        if(sStat.ulAttributes & DCLFSATTR_SYSTEM)
            szAttrib[2] = 'S';
        if(sStat.ulAttributes & DCLFSATTR_ARCHIVE)
            szAttrib[3] = 'A';

        DclPrintf(szAttrib);

        /*  Type/Size and the Name
        */
		if(sStat.ulAttributes & DCLFSATTR_DIRECTORY)
		{
			DclPrintf(" <DIR>      %s\n", pszName);

            /*  Display a '.' or '..' directory, but don't count them
                in the total.
            */    
            if(*pszName != '.')
    		    ulNumDirs++;
		}
		else
		{
            DclPrintf(" %10lU %s\n", sStat.ulSize, pszName);

		    ulNumFiles++;
            ullUsedBytes += sStat.ulSize;
		}
    } while(TRUE);
    
    DclFsDirClose(hDir);

    /*  Show number of files and directories
    */
    DclPrintf("        %10lU File(s) %14llU Bytes -- %11s\n", 
        ulNumFiles, VA64BUG(ullUsedBytes),
        DclScaleItems(ullUsedBytes, 1, szSizeBuff, sizeof(szSizeBuff)));

    DclPrintf("        %10lU Subdirectories\n", ulNumDirs);
    *pullAccumNumFiles += ulNumFiles;
    *pullAccumTotalBytes += ullUsedBytes;
    *pullAccumNumDirs += ulNumDirs;

    /*  Enumerate the directory again if we are recursing into subdirectories
    */
    if(fRecurse)
    {
        DclPrintf("\n");
    
        dclStat = DclFsDirOpen(pszPath, &hDir);
        if(dclStat != DCLSTAT_SUCCESS)
        {
            DclPrintf("Unable to open directory \"%s\", Status=%lX\n", pszPath, dclStat);
        }
        else
        {
            do
            {
                dclStat = DclFsDirRead(hDir, pszName, nDimOfName, &sStat);
                if(dclStat != DCLSTAT_SUCCESS)
                     break;

                if(sStat.ulAttributes & DCLFSATTR_DIRECTORY)
                {
                    /*  Skip the DOT and DOT DOT entries
                    */
                    if(pszName[0] == '.')
                    {
                        if(pszName[1] == 0)
                        {
                            continue;
                        }
                        else if(pszName[1] == '.')
                        {
                            if(pszName[2] == 0)
                                continue;
                        }
                    }

                    /*  Build path for this entry and recurse
                        into subdirectories
                    */
                    pszPath[nPathLength] = 0;
                    if(pszPath[nPathLength - 1] == DCL_PATHSEPCHAR)
                        DclSNPrintf(&pszPath[nPathLength], nDimOfPath - nPathLength, "%s", pszName);
                    else
                        DclSNPrintf(&pszPath[nPathLength], nDimOfPath - nPathLength, "%c%s", DCL_PATHSEPCHAR, pszName);
                    
                    dclStat = DirRecurse(hShell,
                                         fRecurse,
                                         fShowFileHash,
                                         pszPath,
                                         nDimOfPath,
                                         pszName,
                                         nDimOfName,
                                         pullAccumNumFiles,
                                         pullAccumNumDirs,
                                         pullAccumTotalBytes);
                }
            } while(dclStat == DCLSTAT_SUCCESS);
            
            if((dclStat == DCLSTAT_FS_LASTENTRY) || (dclStat == DCLSTAT_FS_NOTFOUND))
                dclStat = DCLSTAT_SUCCESS;

            DclFsDirClose(hDir);
        }
    }

    pszPath[nPathLength] = 0;
    
    return dclStat;
}




#endif

