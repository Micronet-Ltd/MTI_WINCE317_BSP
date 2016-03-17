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

    This module contains helper routines that are used by the OS level
    device driver to do high level formatting.

    ToDo:
        Once the allocator interface is fully abstracted, the code in this
        module should use that interface, and not have any direct knowledge
        about VBF.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: drvfmt.c $
    Revision 1.27  2010/01/27 04:27:30Z  glenns
    Repair issues exposed by turning on a compiler option to warn of 
    possible data loss resulting from implicit typecasts between
    integer data types.
    Revision 1.26  2009/08/02 16:53:45Z  garyp
    Pushed the FAT serial number query from FfxDriverFormatFileSystem()
    down into FatFormat().
    Revision 1.25  2009/07/21 21:12:31Z  garyp
    Merged from the v4.0 branch.  Eliminated a deprecated header.  Updated
    to use some reorganized FAT functionality.
    Revision 1.24  2009/04/09 21:26:37Z  garyp
    Documentation updates only -- no functional changes.
    Revision 1.23  2009/03/31 19:15:15Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.22  2009/02/18 03:07:47Z  keithg
    Added explicit void of unused formal parameter.
    Revision 1.21  2008/12/09 22:23:35Z  keithg
    Serial numbers are now calculated internally.  Added option call to allow
    an OEM to over-ride the default random serial number.
    Revision 1.20  2008/05/27 19:29:54Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.19  2008/03/22 17:38:42Z  Garyp
    Made the code conditional on FFXCONF_VBFSUPPORT.  Minor data type changes.
    Revision 1.18  2008/01/13 07:26:25Z  keithg
    Function header updates to support autodoc.
    Revision 1.17  2007/12/01 03:32:56Z  Garyp
    Modified FfxFatFormat() to return a meaningful status code rather than
    D_BOOL, and adjusted the calling code to make use of it as appropriate.
    Revision 1.16  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.15  2007/09/26 23:48:22Z  jeremys
    Renamed a header file.
    Revision 1.14  2007/01/05 02:30:45Z  pauli
    Resolve Bug 814.  Reduce the total number of sectors by the number
    of hidden sectors if an MBR is used.
    Revision 1.13  2006/12/30 20:22:09Z  Garyp
    Fixed an invalid debug message.
    Revision 1.12  2006/10/16 18:55:26Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.11  2006/10/11 00:34:24Z  Garyp
    Same filename, different functionality.  This module now contains high-level
    formatting code.  The logic that use to be in this module is now refactored
    and mostly resides in drvvbf.c.
    Revision 1.10  2006/06/12 12:17:31Z  Garyp
    Cleaned up some debug messages.
    Revision 1.9  2006/03/06 22:15:22Z  Garyp
    Removed some dead code.
    Revision 1.8  2006/02/21 02:02:41Z  Garyp
    Updated to work with the new VBF API.
    Revision 1.7  2006/02/15 09:49:49Z  Garyp
    Removed obsolete settings.
    Revision 1.6  2006/02/14 02:59:40Z  Garyp
    Corrected to use DiskNum instead of DeviceNum.
    Revision 1.5  2006/02/08 00:23:05Z  Garyp
    Updated to use new device/disk handle model.
    Revision 1.4  2006/01/12 04:06:56Z  Garyp
    Updated to handle (or gracefully fail) in cases where non-standard
    block sizes are being used.
    Revision 1.3  2006/01/08 14:28:06Z  Garyp
    Modified so the uAllocBlockSize and ulMaxTransferSize fields in pDev are
    initialized after VBF formatting and mounting.  Added debugging code.
    Revision 1.2  2005/12/15 01:32:00Z  garyp
    Fixed an invalid return value.
    Revision 1.1  2005/12/04 21:07:08Z  Pauli
    Initial revision
    Revision 1.2  2005/12/04 21:07:07Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.1  2005/10/06 06:50:54Z  Garyp
    Initial revision
    Revision 1.29  2005/05/11 00:56:17Z  garyp
    Updated some debugging code to be more useful.
    Revision 1.28  2005/04/27 02:15:14Z  garyp
    Minor debug code changes.
    Revision 1.27  2005/02/25 21:30:13Z  GaryP
    Minor formatting cleanup -- no functional changes.
    Revision 1.26  2004/12/30 17:32:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.25  2004/12/01 18:51:48Z  PaulI
    Initialized compStat to prevent compiler warning of using an uninitialized
    variable.
    Revision 1.24  2004/11/19 20:23:49Z  GaryP
    Modified to accomodate nested compaction suspends.
    Revision 1.23  2004/11/09 02:46:07Z  GaryP
    Modified to use the updated background compaction API.
    Revision 1.22  2004/10/29 00:57:49Z  GaryP
    Changed garbage collection terminology to compaction.
    Revision 1.21  2004/10/28 19:32:37Z  GaryP
    Fixed to properly disable background compaction prior to doing the format.
    Revision 1.20  2004/09/29 02:38:07Z  GaryP
    Minor cleanup of the messages.
    Revision 1.19  2004/09/25 04:10:32Z  GaryP
    Added error handling to FfxCalculateCHS().
    Revision 1.18  2004/09/24 21:00:06Z  jaredw
    Now sets TotalSectors before calculating CHS values.
    Revision 1.17  2004/09/17 02:44:14Z  GaryP
    Moved the FATMON re-init code to the proper location after reformatting.
    Revision 1.16  2004/09/07 21:37:42Z  GaryP
    Eliminated the FFXOPT_FATFORMAT setting.
    Revision 1.15  2004/08/30 20:31:00Z  GaryP
    Documentation fixes.
    Revision 1.14  2004/08/10 07:29:25Z  garyp
    Added a missing header.
    Revision 1.13  2004/08/09 19:32:25Z  GaryP
    Made all the FAT specific code conditional on the FFXCONF_* FAT related
    settings.  Split the MBR logic out from FfxFatFormat().
    Revision 1.12  2004/07/23 23:43:33Z  GaryP
    Added support for specifying a default file system.
    Revision 1.11  2004/07/19 22:23:00Z  jaredw
    moved pFFP delcration inside #if statement
    Revision 1.10  2004/07/19 18:47:08Z  jaredw
    readded the pFFP declaration
    Revision 1.9  2004/07/12 21:31:23Z  garyp
    Eliminated an unreferenced local.
    Revision 1.8  2004/07/03 02:04:35Z  GaryP
    Updated to use FfxHookOptionGet().
    Revision 1.7  2004/04/30 02:27:40Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.6  2003/11/01 01:57:12Z  garys
    Merge from FlashFXMT
    Revision 1.3.1.2  2003/11/01 01:57:12  garyp
    Re-checked in to variant sandbox.
    Revision 1.4  2003/11/01 01:57:12Z  garyp
    Modified parameters to FfxDriverFormatDevice() to accommodate FFX-MT needs.
    Revision 1.3  2003/09/11 20:49:43Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.2  2003/08/07 23:03:54Z  garyp
    Documentation updates.
    Revision 1.1  2003/04/15 17:49:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

#include <vbf.h>
#include <diskapi.h>
#include <fxdriver.h>

#if FFXCONF_FORMATSUPPORT

#if FFXCONF_FATFORMATSUPPORT
#include <fxfatapi.h>
static FFXSTATUS FatFormat(FFXDISKHANDLE hDisk, D_BOOL fUsedMBR, D_UINT32 ulHidden, D_UINT16 uSides, D_UINT16 uSPT, D_UINT16 uCylinders, D_UINT32 ulClientSize);
#endif


/*-------------------------------------------------------------------
    Protected: FfxDriverFormatFileSystem()

    This function writes file system level information to a
    disk.  It only knows how to do two things:
      1) Write an MBR to the disk, if the FS is FAT or Reliance
      2) Write a FAT format to the disk.

    If the system is configured such that this function cannot
    do at at least one of those two things, the status code
    FFXSTAT_DRV_NOFILESYSTOFORMAT will be returned.

    Parameters:
        hDisk - The handle of the disk to use
        pFS   - A pointer to the FFXFORMATSETTINGS structure to use.
                If this value is NULL, then the default format
                settings for the disk will be used.

    Return Value:
        Returns an FFXSTATUS code indicating the state of the
        operation.
-------------------------------------------------------------------*/
FFXSTATUS FfxDriverFormatFileSystem(
    FFXDISKHANDLE       hDisk,
    FFXFORMATSETTINGS  *pFS)
{
    FFXDISKINFO        *pDsk;
    FFXSTATUS           ffxStat;
    VBFDISKINFO         di;
    FFXFORMATSETTINGS   fs;

    FFXPRINTF(1, ("FfxDriverFormatFileSystem() FormatState=%U FileSys=%U UseMBR=%U\n",
        pFS->uFormatState, pFS->uFileSystem, pFS->fUseMBR));

    DclAssert(hDisk);

    pDsk = *hDisk;

    if(!pFS)
    {
        /*  Use the default settings if none were specified
        */
        if(!FfxHookOptionGet(FFXOPT_DISK_FORMATSETTINGS, hDisk, &fs, sizeof(fs)))
            return FFXSTAT_DRV_NOFORMATSETTINGS;

        pFS = &fs;
    }

    ffxStat = FfxVbfDiskInfo(pDsk->hVBF, &di);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        FFXPRINTF(1, ("FfxVbfDiskInfo failed!\n"));

        return ffxStat;
    }

  #if !FFXCONF_MBRFORMAT && !FFXCONF_FATFORMATSUPPORT
    {
        DclAssert(pFS->fUseMBR == FALSE);

        return FFXSTAT_DRV_NOFILESYSTOFORMAT;
    }
  #else
    {
        D_UINT32            ulVBFClientSize;
        D_UINT32            ulHidden = 0;   /* Init to avoid compile warning */
        D_UINT16            uSides = 0;     /* Init to avoid compile warning */
        D_UINT16            uSPT = 0;       /* Init to avoid compile warning */
        D_UINT16            uCylinders = 0; /* Init to avoid compile warning */

      #if D_DEBUG
        {
            #define BUFFLEN (16)

            char szFileSys[BUFFLEN];

            switch(pFS->uFileSystem)
            {
                case FFX_FILESYS_UNKNOWN:
                    DclStrCpy(szFileSys, "Unknown");
                    break;
                case FFX_FILESYS_RELIANCE:
                    DclStrCpy(szFileSys, "Reliance");
                    break;
                case FFX_FILESYS_FAT:
                    DclStrCpy(szFileSys, "FAT");
                    break;
                default:
                    DclStrCpy(szFileSys, "Other");
                    DclError();
                    break;
            }

            FFXPRINTF(1, ("Driver Format Params: fUseMBR=%U Type=%s\n", pFS->fUseMBR, szFileSys));
        }
      #endif

        if(!pFS->fUseMBR && (pFS->uFileSystem != FFX_FILESYS_FAT))
        {
            /*  If this routine can't do anything, return a code indicating
                as such.
            */
            return FFXSTAT_DRV_NOFILESYSTOFORMAT;
        }

        ulVBFClientSize = di.ulTotalPages * di.uPageSize;

      #if FFXCONF_MBRFORMAT
        if(pFS->fUseMBR)
        {
            if(!FfxDriverWriteMBR(hDisk, ulVBFClientSize, &ulHidden, &uSides, &uSPT, &uCylinders))
            {
                DclPrintf("FlashFX: MBR write failed!\n");

                return FFXSTAT_WRITEFAILED;
            }
        }
      #endif

      #if FFXCONF_FATFORMATSUPPORT
        if(pFS->uFileSystem == FFX_FILESYS_FAT)
            
            /*  Note: if fUseMBR is false, some parameters to this function will
                not be initialized to valid values, but this doesn't matter
                becase FatFormat examines fUseMBR and if it is false, it will
                compute its own CHS calues and won't use those parameters
                anyway.
            */
            ffxStat = FatFormat(hDisk, pFS->fUseMBR, ulHidden, uSides, uSPT, uCylinders, ulVBFClientSize);
      #endif

        return ffxStat;
    }
  #endif
}


#if FFXCONF_FATFORMATSUPPORT

/*-------------------------------------------------------------------
    Local: FatFormat()

    This function places a FAT format on a disk.

    Parameters:

    Return Value:
        Returns an FFXSTATUS code indicating the state of the
        operation.
-------------------------------------------------------------------*/
static FFXSTATUS FatFormat(
    FFXDISKHANDLE   hDisk,
    D_BOOL          fUsedMBR,
    D_UINT32        ulHidden,
    D_UINT16        uSides,
    D_UINT16        uSPT,
    D_UINT16        uCylinders,
    D_UINT32        ulClientSize)
{
    DCLFATPARAMS    ffp = {0};
    FFXDISKINFO    *pDsk = *hDisk;
    FFXSTATUS       ffxStat;

    DclAssert(pDsk->ulSectorLength < D_UINT16_MAX);

    ffp.uBytesPerSector = (D_UINT16)pDsk->ulSectorLength;

    if(fUsedMBR)
    {
        /*  If we just wrote an MBR, use the disk characteristics that
            we calculated for that process.
        */
        ffp.uHeads = uSides;
        ffp.uSPT = uSPT;
        ffp.uCylinders = uCylinders;
        ffp.ulHiddenSectors = ulHidden;
        ffp.ulTotalSectors = (D_UINT32) uSides * (D_UINT32)uSPT * (D_UINT32)uCylinders;
        ffp.ulTotalSectors -= ulHidden;
    }
    else
    {
        ffp.ulTotalSectors = ulClientSize / ffp.uBytesPerSector;

        /*  Determine the number of heads, SPT, sides, and cylinders.
            This will update the number of sectors to fit into the
            calculated CHS.
        */
        if(!FfxCalculateCHS(&ffp.ulTotalSectors, &ffp.uHeads, &ffp.uSPT, &ffp.uCylinders))
            return FFXSTAT_BADDISKGEOMETRY;

        /*  Hidden sectors will always be zero if an MBR is not being used.
        */
        ffp.ulHiddenSectors = 0;
    }

    FfxHookOptionGet(FFXOPT_FATROOTENTRIES, hDisk, &ffp.uRootEntries, sizeof ffp.uRootEntries);
    FfxHookOptionGet(FFXOPT_FATCOUNT, hDisk, &ffp.uNumFATs, sizeof ffp.uNumFATs);

    DclAssert(ffp.uRootEntries);
    DclAssert(ffp.uNumFATs);

    if(!FfxHookOptionGet(FFXOPT_FATSERIALNUM, hDisk, &ffp.ulSerialNumber, sizeof ffp.ulSerialNumber))
    {
        ffp.ulSerialNumber = (D_UINT32)DclRand(NULL);
    }

    DclSNPrintf(ffp.szVolumeLabel, sizeof(ffp.szVolumeLabel), "FLASHFX_%03u", pDsk->Conf.nDiskNum);

    /*  Format the FAT file system
    */
    ffxStat = FfxFatFormat(pDsk->hVBF, &ffp);
    if(ffxStat != FFXSTAT_SUCCESS)
    {
        DclPrintf("FlashFX: FAT format failed with status code %lX\n", ffxStat);
    }

    return ffxStat;
}



#endif  /* FFXCONF_FATFORMATSUPPORT */
#endif  /* FFXCONF_FORMATSUPPORT */
#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */



