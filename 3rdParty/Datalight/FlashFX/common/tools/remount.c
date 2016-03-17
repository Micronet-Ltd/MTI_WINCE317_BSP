/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2009 Datalight, Inc.
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

    Remounts a flash disk.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: remount.c $
    Revision 1.11  2009/07/22 00:01:43Z  garyp
    Merged from the v4.0 branch.  Updated so that VBF instances are created
    in compaction suspend mode and have to be explicitly resumed.  Modified
    the shutdown processes to take a mode parameter.  Updated to use the
    revised FfxSignOn() function, which now takes an fQuiet parameter.
    Revision 1.10  2009/04/01 15:31:23Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.9  2009/03/09 19:45:21Z  thomd
    Cast parameter of FfxFmlHandle().
    Revision 1.8  2009/02/09 01:38:08Z  garyp
    Merged from the v4.0 branch.  Updated to use the new FFXTOOLPARAMS
    structure.
    Revision 1.7  2008/03/22 23:57:34Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.6  2008/01/13 07:27:07Z  keithg
    Function header updates to support autodoc.
    Revision 1.5  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2006/02/20 06:15:38Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.3  2006/02/12 18:48:22Z  Garyp
    Modified to use the new FFXTOOLPARAMS structure, and deal with a
    single device/disk at a time.
    Revision 1.2  2006/01/31 07:21:16Z  Garyp
    Updated to use the new FML interface.
    Revision 1.1  2005/10/20 02:36:30Z  Pauli
    Initial revision
    Revision 1.2  2005/10/20 03:36:30Z  garyp
    Changed some D_CHAR buffers to be plain old char.
    Revision 1.1  2005/10/02 03:24:32Z  Garyp
    Initial revision
    Revision 1.9  2004/12/30 23:43:18Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2004/01/14 03:27:36Z  garys
    Merge from FlashFXMT
    Revision 1.5.1.4  2004/01/14 03:27:36  garyp
    Include changes.
    Revision 1.5.1.3  2004/01/03 18:10:24Z  garyp
    Changed to use FfxStrICmp().
    Revision 1.5.1.2  2003/11/03 04:49:30Z  garyp
    Re-checked into variant sandbox.
    Revision 1.6  2003/11/03 04:49:30Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.5  2003/04/08 21:54:58Z  garyp
    Updated to use FFX internal string routines.  Eliminated use of __D_FAR
    where possible.
    Revision 1.4  2003/03/27 23:30:20Z  garyp
    Fixed some command-line parsing problems.
    Revision 1.3  2003/03/26 17:48:08Z  garyp
    Eliminated all OESL specific code, along with any code related to
    the external API or oem/vbf initialization.  Revamped to use the new
    TOOLS and TESTS framework to deal with these issues.
    02/09/99 HDS Flagged all printfs to make them easier to find for
                 modification.
    11/16/98 HDS Made minor change to display information.
    12/10/97 HDS Fixed command names.
    11/20/97 PKG Removed use of .pt files to make it directory independant
                 and re-ordered the function calls to be self prototyped.
                 This allows these source file to be compiled from anywhere
                 within the project.
    11/19/97 HDS Changed include file search to use predefined path.
    08/26/97 PKG Now uses new Datalight specific types
    07/28/97 PKG Reorganized source into single, smaller, simpler modules
                 and removed the library files.
    06/12/97 DM  The  RemountDisk() routine was pulled out of this module and
                 placed into a library.
    03/10/97 DM  Created
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <fxfmlapi.h>
#include <fxtools.h>

static D_INT16  RemountDisk(VBFHANDLE hVBF);
static void     ShowUsage(FFXTOOLPARAMS *pTP);

#define ARGBUFFLEN   128


/*-------------------------------------------------------------------
    Protected: FfxRemount()

    Parameters:

    Return Value:
-------------------------------------------------------------------*/
D_INT16 FfxRemount(
    FFXTOOLPARAMS  *pTP)
{
    char            achArgBuff[ARGBUFFLEN];
    D_UINT16        uArgCount;
    D_INT16         iErrorLevel;
    VBFHANDLE       hVBF;

    DclPrintf("\nFlashFX Remount\n");
    FfxSignOn(FALSE);

    uArgCount = DclArgCount(pTP->dtp.pszCmdLine);

    if(uArgCount)
    {
        if(!DclArgRetrieve(pTP->dtp.pszCmdLine, 1, ARGBUFFLEN, achArgBuff))
        {
            DclError();
            return 3;
        }

        if(DclStrICmp(achArgBuff, "/?") == 0)
        {
            ShowUsage(pTP);

            return 1;
        }

        DclPrintf("Invalid argument '%s'\n", (const char *) achArgBuff);

        return 2;
    }

    hVBF = FfxVbfHandle(pTP->nDiskNum);
    if(!hVBF)
    {
        DclPrintf("DISK%u not initialized\n", pTP->nDiskNum);
        return __LINE__;
    }

    /*  If resetting the disk fails, then return a non-zero error code to
        dos and print a message out to the user!
    */
    iErrorLevel = RemountDisk(hVBF);
    if(iErrorLevel != 0)
    {
        DclPrintf("\n  Disk Remount Failed!\n");

        return iErrorLevel;
    }

    DclPrintf("\nDisk Remounted\n");

    /*  Return an error level of 0 to DOS - this means no errors occured
    */
    return 0;
}


/*-------------------------------------------------------------------
    Local: ShowUsage()

    Parameters:

    Return Value:	 
        None.
-------------------------------------------------------------------*/
static void ShowUsage(
    FFXTOOLPARAMS  *pTP)
{
    DclPrintf("\nThis command remounts a FlashFX disk.\n\n");
    DclPrintf("Usage: %s drive\n", pTP->dtp.pszCmdName);
    DclPrintf("  drive  The drive designation, in the form: %s\n", pTP->pszDriveForms);
    DclPrintf("  /?     This help information\n");

    if(pTP->dtp.pszExtraHelp)
        DclPrintf(pTP->dtp.pszExtraHelp);

    return;
}


/*-------------------------------------------------------------------
    Local: RemountDisk()

    This function remounts a flash disk.

    Parameters:
        hFML - A handle denoting the FML instance to use

    Return Value:
        This function will return an error level of 0 if successful.
        It will return an error level of non-zero if an error occurs.
-------------------------------------------------------------------*/
static D_INT16 RemountDisk(
    VBFHANDLE       hVBF)
{
    VBFDISKINFO     DiskInfo;
    FFXSTATUS       ffxStat;
    VBFHANDLE       hNewVBF;

    if(FfxVbfDiskInfo(hVBF, &DiskInfo) != FFXSTAT_SUCCESS)
        return __LINE__;

    ffxStat = FfxVbfDestroy(hVBF, FFX_SHUTDOWNFLAGS_NORMAL);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("Unable to dismount DISK%u, Status=%lX\n", DiskInfo.nDiskNum, ffxStat);
        return __LINE__;
    }

    hNewVBF = FfxVbfCreate(DiskInfo.hFML);
    if(!hNewVBF)
    {
        DclPrintf("Unable to mount DISK%u\n", DiskInfo.nDiskNum);
        return __LINE__;
    }

  #if FFX_COMPACTIONMODEL != FFX_COMPACT_NONE
    /*  The VBF instance is created with idle-time compaction
        suspended.  It must be explicitly enabled.
    */
    FfxVbfCompactIdleResume(hNewVBF);
  #endif

    /*  If there is no error, then return an error level of zero.
    */
    return 0;
}



#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */



