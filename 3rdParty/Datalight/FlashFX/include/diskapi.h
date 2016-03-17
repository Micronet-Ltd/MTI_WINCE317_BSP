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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header contains the prototypes, structures, and types necessary
    to use the DISK related API functions in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: diskapi.h $
    Revision 1.16  2011/02/16 18:33:42Z  garyp
    Added a duplicate definition of DCLMBR_PARTITION_BOOTABLE to avoid
    issue with shared-license headers (still could use some refactoring).
    Revision 1.15  2011/02/09 00:57:55Z  garyp
    Modified to use some refactored disk partition manipulation code which
    has moved from FlashFX into DCL.  Not functionally changed otherwise.
    Revision 1.14  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.13  2009/07/22 00:17:50Z  garyp
    Merged from the v4.0 branch.  Added FfxMBRDisplayPartitionTable().  Added a
    missing const declaration to FfxMBRDisplayPTE().  Minor structure updates.
    Eliminated an unused parameter for FfxMBRLoadPartitions().   
    Revision 1.12  2009/02/09 00:46:33Z  garyp
    Removed partition ID stuff which now resides in DCL.
    Revision 1.11  2008/05/04 19:04:46Z  garyp
    Added a DISKMBR structure.
    Revision 1.10  2008/03/23 02:26:16Z  Garyp
    Added more MBR definitions.
    Revision 1.9  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/06/26 19:33:07Z  timothyj
    Increased CHS max values to the full limit imposed by the MBR on-media
    structure.
    Revision 1.7  2006/10/18 04:27:33Z  Garyp
    Modified so MBR support can be stripped out of the product.
    Revision 1.6  2006/06/12 11:59:18Z  Garyp
    Minor type changes.
    Revision 1.5  2006/03/17 20:13:52Z  Garyp
    Modified to allow FAT boot records for non-512-byte disks to be recognized.
    Revision 1.4  2006/02/21 03:28:33Z  Garyp
    Minor type changes.
    Revision 1.3  2006/01/08 14:46:28Z  Garyp
    Minor prototype tweaks.
    Revision 1.2  2005/12/10 16:58:37Z  garyp
    Fixed the Reliance partition ID to match that used in Reliance 2.0.
    Revision 1.1  2005/10/02 01:58:22Z  Pauli
    Initial revision
    Revision 1.12  2004/12/30 21:38:43Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.11  2004/12/19 00:12:51Z  GaryP
    Modified to accommodate MBRs that manage disks with sector sizes other than
    512 bytes.  Added a FlashFX proprietary field into the MBR to denote the
    sector size.
    Revision 1.10  2004/09/22 17:52:19Z  GaryP
    Updated to use DSK_SECTORLEN.
    Revision 1.9  2004/09/17 02:27:25Z  GaryP
    Minor function renaming exercise.
    Revision 1.8  2004/08/07 02:31:45Z  GaryP
    Miscellaneous changes to support isoating the file system specific support
    from generic FlashFX functionality.
    Revision 1.7  2004/07/23 23:43:32Z  GaryP
    Added support for specifying a default file system.
    Revision 1.6  2004/05/06 00:42:06Z  garyp
    Updated to eliminate passing sector buffers around.
    Revision 1.5  2004/01/13 04:16:08Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.4  2004/01/13 04:16:08  garyp
    Eliminated some obsolete prototypes.
    Revision 1.2.1.3  2003/11/29 01:57:42Z  garyp
    Added the Reliance file system ID.
    Revision 1.2.1.2  2003/11/05 03:52:56Z  garyp
    Re-checked into variant sandbox.
    Revision 1.3  2003/11/05 03:52:56Z  garyp
    Updated to use the new FlashFX/MT interface.
    Revision 1.2  2003/04/16 06:15:40Z  garyp
    Variable renamed.
    Revision 1.1  2003/03/26 21:15:24Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DISKAPI_H_INCLUDED
#define DISKAPI_H_INCLUDED


/*-------------------------------------------------------------------
-------------                                             -----------
-------------  Structures and Types for Public Functions  -----------
-------------                                             -----------
-------------------------------------------------------------------*/

#define DSK_SECTORLEN     (512) /* Standard disk sector size     */

#define MAX_SPT            (63)
#define MAX_SIDES         (255)
#define MAX_TRACKS       (1024)

#if FFXCONF_MBRSUPPORT
    /*  MBR specific stuff
    */
    #include <dlmbr.h>
    
    #define MBR_OFFSET_FFXSIG  0x01B6   /* offset of FFX MBR signature            */
    #define MBR_OFFSET_FFXBPS  0x01BA   /* offset of FFX MBR BytesPerSector value */
    #define MBR_FFX_SIGNATURE  "FFXP"   /* FlashFX MBR signature                  */
    #define MBR_TWOBYTEJUMP     0xEB    /* First byte of an MBR is one  */
    #define MBR_THREEBYTEJUMP   0xE9    /* of these two values.         */
#else
    /*  Indicates that a given partition is bootable.  This is a
        duplication of the definition in dlmbr.h, done to avoid
        including that header when it is otherwise not needed.
    */     
    #define DCLMBR_PARTITION_BOOTABLE  0x80 
#endif

/*  Define this even if MBR support is turned off, because we use this
    structure to store internal emulated physical characteristics.
*/
typedef struct
{
    D_UINT32            ulStartSector;
    D_UINT32            ulSectorCount;
    D_BYTE              bFSID;
    D_BYTE              bBootable;
} FFXPARTITIONINFO;

typedef struct
{
    D_BOOL              fUsingMBR;
    D_BOOL              fIsFFXMBR;
    D_BOOL              fFoundValidPartitions;
    D_UINT16            uSectorsPerTrack;
    D_UINT16            uHeads;
    D_UINT16            uCylinders;
    D_UINT32            ulBytesPerSector;
    D_UINT32            ulTotalSectors;
  #if FFXCONF_MBRSUPPORT
    FFXPARTITIONINFO    fpi[DCLMBR_PARTITION_COUNT];
  #else
    FFXPARTITIONINFO    fpi[1];
  #endif
} FFXPHYSICALDISKINFO;



/*-------------------------------------------------------------------
-------------                                            ------------
-------------       Prototypes for Public Functions      ------------
-------------                                            ------------
-------------------------------------------------------------------*/

D_BOOL          FfxGetPhysicalDiskParams(FFXPHYSICALDISKINFO *pPDI, const D_BUFFER *pBuffer, D_UINT32 ulSectorLen);
D_BOOL          FfxCalcPhysicalDiskParams(FFXPHYSICALDISKINFO *pPDI, D_UINT32 ulSectors);
D_BOOL          FfxCalculateCHS(D_UINT32 *pulTotalSectors, D_UINT16 *puSides, D_UINT16 *puSpt, D_UINT16 *puCyls);

#if FFXCONF_MBRSUPPORT
    D_UINT32    FfxMBRLoadPartitions(FFXPHYSICALDISKINFO *pPDI, const D_BUFFER *pData);
  #if FFXCONF_MBRFORMAT
    D_UINT32    FfxMBRBuild(D_BUFFER *pSector, D_UINT16 uSides, D_UINT16 uSpt, D_UINT16 uCylinders, D_UINT32 ulSectorLen, D_UINT16 uFileSys);
  #endif
#endif



#endif /* DISKAPI_H_INCLUDED */


