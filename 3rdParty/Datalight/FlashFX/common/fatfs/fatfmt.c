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

    This module implements FAT format functionality on a FlashFX Disk.
    ToDo:
      - Update to work with sector sizes other than 512 bytes.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fatfmt.c $
    Revision 1.20  2009/07/22 18:17:06Z  garyp
    Removed an unused variable reference.
    Revision 1.19  2009/07/21 21:55:27Z  garyp
    Merged from the v4.0 branch.   Updated to use BPB and boot record 
    creation functions which now reside in DCL.
    Revision 1.18  2009/03/31 19:49:06Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.17  2009/02/18 04:46:57Z  garyp
    Void an unused parameter.
    Revision 1.16  2009/02/13 20:42:03Z  garyp
    Partial merge from the v4.0 branches.  Resolved bug 2350 -- Fixed to not
    depend on structure packing.  Fixed an endian issue in the time and date
    fields of the volume label.  Modified to no longer create a sector buffer in
    automatic storage.
    Revision 1.15  2008/02/06 20:14:59Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.14  2008/01/13 07:26:45Z  keithg
    Function header updates to support autodoc.
    Revision 1.13  2007/12/01 03:32:56Z  Garyp
    Modified FfxFatFormat() to return a meaningful status code rather than
    D_BOOL, and adjusted the calling code to make use of it as appropriate.
    Revision 1.12  2007/11/03 23:49:31Z  Garyp
    Updated to use the standard module header.
    Revision 1.11  2007/09/26 23:59:18Z  jeremys
    Renamed a header file.
    Revision 1.10  2007/06/07 21:34:39Z  rickc
    Replace deprecated vbfwrite() calls with FfxVbfWritePages()
    Revision 1.9  2006/10/16 19:49:01Z  Garyp
    Minor type changes.
    Revision 1.8  2006/10/11 18:28:48Z  Garyp
    Updated to use some renamed symbols.
    Revision 1.7  2006/10/06 00:14:34Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.6  2006/02/21 23:18:16Z  Garyp
    Minor type changes.
    Revision 1.5  2006/01/31 01:58:06Z  Garyp
    Updated to use the new FML interface.
    Revision 1.4  2006/01/12 02:23:43Z  Garyp
    Updated to handle (or gracefully fail) in cases where non-standard
    block sizes are being used.
    Revision 1.3  2006/01/11 02:39:48Z  Garyp
    Changed a preprocessor condition to match similar code.
    Revision 1.2  2006/01/05 03:36:28Z  Garyp
    Changes per Bill and GP.  Updated to eliminate the fixed compile-time
    VBF allocation block size setting.
    Revision 1.1  2005/10/03 19:37:20Z  Pauli
    Initial revision
    Revision 1.13  2004/12/30 17:32:43Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.12  2004/09/23 05:49:16Z  GaryP
    Updated to use the FAT_SECTORLEN value.
    Revision 1.11  2004/08/30 20:26:46Z  GaryP
    Changed the file number.
    Revision 1.10  2004/08/15 19:16:11Z  GaryP
    Minor variable renaming exersize.
    Revision 1.9  2004/08/09 20:35:57Z  GaryP
    Renamed the function, and updated to no longer support writing an MBR.
    Revision 1.8  2004/07/23 23:49:31Z  GaryP
    Added support for specifying a default file system.
    Revision 1.7  2004/05/06 01:21:28Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.6  2003/11/05 03:24:40Z  garys
    Merge from FlashFXMT
    Revision 1.3.1.2  2003/11/05 03:24:40  garyp
    Re-checked into variant sandbox.
    Revision 1.4  2003/11/05 03:24:40Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.3  2003/05/14 00:34:28Z  garyp
    Added debugging code.
    Revision 1.2  2003/04/16 06:20:54Z  garyp
    Updated to use the new FfxBuildBPB().
    Revision 1.1  2003/04/15 17:49:10Z  garyp
    Initial revision
    Revision 1.5  2003/03/26 01:29:48Z  garyp
    Moved BuildMBR() and BuildBPB() to other modules and renamed
    them along the way.  Eliminated use of the __D data types.
    Revision 1.4  2003/02/01 09:39:40Z  garyp
    Modified to use new types and symbols from FAT.H.
    Revision 1.3  2003/01/10 22:07:22Z  garyp
    Eliminated a number of C++ style comments.
    08/02/02 gjs Fixed assert in BuildMBR() by casting D_UINTs to D_UINT32.
    02/06/02 gp  Fixed some minor documentation problems, and added a typecast
                 to an assert.
    01/03/01 gp  Added the RESERVEMBRTRACK compile time option to allow
                 the first track to be reserved for the MBR rather than
                 just the first sector.
                 Fixed a bug in BuildMBR() where the ending cylinder value
                 in the partition table was always off by one (too high).
                 Modified BuildMBR() take a cylinders parameter rather than
                 recalculating it.  Modified the routine to return the number
                 of hidden sectors reserved for the MBR.
                 Modified BuildBPB() to take a hidden sector count value for
                 inclusion in the BPB.
    11/22/99 GJS Removed an invalid assert for sectors per track.
    08/04/99 DM  If volume label passed to FatFormat is NULL then we do not
                 write the first directory entry with a blank label. This
                 fixes VMAPI so that it can be booted from MSDOS with no
                 intervention.
    10/20/98 PKG Now handles disks larger than 128MB
    01/12/98 PKG Now 100% portable, uses _ENDIAN macros.
    11/19/97 HDS Changed include file search to use predefined path.
    10/27/97 HDS Now uses new extended Datalight specific types.
    08/26/97 PKG Now uses new Datalight specific types
    08/07/97 PKG No longer uses BLOCK_SIZE since it is not available.  Perform
                 some error checking to avoid formatting a very small disk.
    06/06/97 DM  Now uses the CHS generator in a seperate module
    05/14/97 PKG All pointers passed as arguments are now type casted
                 to the types they already are.  This will allow the data
                 segment and the stack segment to be in seperate segments.
    04/30/97 DM  Added type casts to avoid lint warnings
    03/24/97 PKG Added a default time date stamp to the volume label
    02/19/97 PKG Created
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_ALLOCATORSUPPORT
#if FFXCONF_VBFSUPPORT

/*  Note that this code is conditional on FFXCONF_FATSUPPORT rather than
    FFXCONF_FATFORMATSUPPORT because it may be linked in when building
    external tools.  Therefore the same condition must be used here as
    is used in format.c.
*/
#if FFXCONF_FATSUPPORT

#include <dlfat.h>
#include <fxfatapi.h>
#include <vbf.h>
#include <fxfmlapi.h>


/*-------------------------------------------------------------------
    Public: FfxFatFormat()

    Formats media with a FAT12 or FAT16 file system.

    Parameters:
        hVBF - The VBF handle to use
        pFFP - FAT information to use

    Return Value:
        Returns an FFXSTATUS value indicating the result.
-------------------------------------------------------------------*/
FFXSTATUS FfxFatFormat(
    VBFHANDLE           hVBF,
    DCLFATPARAMS       *pFFP)
{
    unsigned            nSector;
    unsigned            nFat;
    unsigned            nNumFats;
    D_UINT16            uSecPerFat;
    D_UCHAR             cMediaID;
    D_UINT32            ulPageOffset;
    D_UINT32            ulNumSectors;
    DCLFATBPB           bpb;
    D_BUFFER           *pSectorBuff = NULL;
    FFXIOSTATUS         ioStat;

    DclAssert(pFFP);

    FFXPRINTF(1, ("FfxFatFormat() BPS=%U HiddenSecs=%lU TotalSecs=%lU RootSize=%U NumFATs=%U\n",
        pFFP->uBytesPerSector, pFFP->ulHiddenSectors, pFFP->ulTotalSectors,
        pFFP->uRootEntries, pFFP->uNumFATs));

    if(pFFP->uBytesPerSector != FAT_SECTORLEN)
    {
        DclPrintf("FlashFX: The FAT format function only works on disks with 512 byte sectors\n");
        return FFXSTAT_FATFMT_UNSUPPORTEDSECTORSIZE;
    }

    DclAssert(hVBF);
    DclAssert(pFFP->uBytesPerSector);
    DclAssert(pFFP->uRootEntries);
    DclAssert(pFFP->uRootEntries % 16 == 0);
    DclAssert(pFFP->uNumFATs);
    DclAssert(pFFP->ulTotalSectors);

    /*  Make sure there is enough space in the media
    */
    if(pFFP->ulTotalSectors < 32)
        return FFXSTAT_FATFMT_INVALIDGEOMETRY;

    ulPageOffset = pFFP->ulHiddenSectors;

    if(!DclFatBPBBuild(&bpb, pFFP))
        return FFXSTAT_FATFMT_BPBCREATIONFAILED;

    pSectorBuff = DclMemAllocZero(pFFP->uBytesPerSector);
    if(!pSectorBuff)
        return DCLSTAT_MEMALLOCFAILED;

    ioStat.ffxStat = DclFatBootRecordBuild(pSectorBuff, &bpb);
    if(ioStat.ffxStat != DCLSTAT_SUCCESS)
        goto Cleanup;

    ioStat = FfxVbfWritePages(hVBF, ulPageOffset, 1, pSectorBuff);
    if(!IOSUCCESS(ioStat, 1))
        goto Cleanup;

    /*  Next sector
    */
    ulPageOffset++;

    /*  Get some useful information for so that we can write the
        rest of the FAT sectors and root directory out to the media.
    */
    nNumFats = (unsigned)pSectorBuff[BT_NFAT_OFF];

    DCLLE2NE((D_BUFFER*)&uSecPerFat, &pSectorBuff[BT_SPF_OFF], sizeof(uSecPerFat));

    cMediaID = pSectorBuff[BT_MDID_OFF];

    /*  For each FAT copy
    */
    for(nFat = 0; nFat < nNumFats; nFat++)
    {
        for(nSector = 0; nSector < uSecPerFat; nSector++)
        {
            DclMemSet(pSectorBuff, 0, pFFP->uBytesPerSector);

            /*  Write out the first FAT sector (FAT ID)
            */
            if(nSector == 0)
            {
                pSectorBuff[0] = cMediaID;
                pSectorBuff[1] = 0xFF;
                pSectorBuff[2] = 0xFF;
                if(pFFP->ulTotalSectors > FAT12_MAXSECTORS)
                    pSectorBuff[3] = 0xFF;
            }

            ioStat = FfxVbfWritePages(hVBF, ulPageOffset, 1, pSectorBuff);
            if(!IOSUCCESS(ioStat, 1))
                goto Cleanup;

            ulPageOffset ++;
        }
    }

    /*  Determine the number of sectors required to hold the root directory
    */
    ulNumSectors = DCLNEXTPOW2BOUNDARY(pFFP->uRootEntries * FAT_DIRENTRYSIZE,
                                       pFFP->uBytesPerSector) / pFFP->uBytesPerSector;

    /*  Now write out the root directory.  We must zero the entire
        root directory.
    */
    for(nSector = 0; nSector < ulNumSectors; nSector++)
    {
        if(nSector == 0)
        {
            D_BUFFER    abDirEntry[FAT_DIRENTRYSIZE];
            D_TIME      tim;
            DCLDATETIME DateTime;
            D_UINT16    uDate;
            D_UINT16    uTime;

            /*  For the first sector, zero the sector buffer, and write a
                volume label entry.
            */
            DclMemSet(pSectorBuff, 0, pFFP->uBytesPerSector);

            DclMemSet(abDirEntry, 0, sizeof(abDirEntry));

            if(pFFP->szVolumeLabel[0])
                DclMemCpy(&abDirEntry[DIR_NAME_OFF], pFFP->szVolumeLabel, DIR_NAME_LEN+DIR_EXT_LEN);
            else
                DclMemCpy(&abDirEntry[DIR_NAME_OFF], "FLASHFXDISK", DIR_NAME_LEN+DIR_EXT_LEN);

            abDirEntry[DIR_ATTR_OFF] = DIR_ATTR_ARCHIVE + DIR_ATTR_VOLUME;

            /*  Try to use the current date and time if we can.
            */
            if(!(DclOsGetDateTime(&tim) && DclDateTimeDecode(&tim, &DateTime)))
            {
                /*  If not, default to legacy values.
                */
                DclMemSet(&DateTime, 0, sizeof(DateTime));

                DateTime.uYear = 1997;
                DateTime.uDay = 1;

                /*  Other values are all zero'd by the DclMemSet()...
                */
            }

            uTime = ((DateTime.uHour << FAT_DIRHOURSHIFT) |
                    (DateTime.uMinute << FAT_DIRMINUTESHIFT) |
                    ((DateTime.uSecond / 2) << FAT_DIRSECONDSHIFT));

            uDate = (((DateTime.uYear - 1980) << FAT_DIRYEARSHIFT) |
                    ((DateTime.uMonth + 1) << FAT_DIRMONTHSHIFT) |
                    (DateTime.uDay << FAT_DIRDAYSHIFT));

            DCLNE2LE(&abDirEntry[DIR_TIME_OFF], &uTime, sizeof(uTime));
            DCLNE2LE(&abDirEntry[DIR_DATE_OFF], &uDate, sizeof(uDate));
        }
        else if(nSector == 1)
        {
            /*  On the second sector, zero the buffer again so the volume
                label entry is wiped out from the first sector.  Once we've
                done this, there is no reason to do it again.
        */
            DclMemSet(pSectorBuff, 0, pFFP->uBytesPerSector);
        }

        /*  Write out each sector
        */
        ioStat = FfxVbfWritePages(hVBF, ulPageOffset, 1, pSectorBuff);
        if(!IOSUCCESS(ioStat, 1))
            goto Cleanup;

        /*  Next sector
        */
        ulPageOffset++;
    }

  Cleanup:

    if(pSectorBuff)
        DclMemFree(pSectorBuff);

    return ioStat.ffxStat;
}



#endif  /* FFXCONF_FATSUPPORT */

#endif  /* FFXCONF_VBFSUPPORT */
#endif  /* FFXCONF_ALLOCATORSUPPORT */


