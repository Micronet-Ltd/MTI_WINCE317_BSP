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

    This header describes the interface to the Block Device API as abstracted
    in the OS Services layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosblockdev.h $
    Revision 1.1  2010/08/04 01:11:20Z  garyp
    Initial revision
    Revision 1.2  2010/03/18 22:13:39Z  glenns
    Add "fAlignedIO" flag for block device abstractions that require IO
    to be aligned to sector size.
    Revision 1.1  2009/09/10 16:59:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLOSBLOCKDEV_H_INCLUDED
#define DLOSBLOCKDEV_H_INCLUDED

#include <dliosys.h>

/*  A block device must be opened using one of the following modes.
*/
typedef enum
{
    DCLOSBLOCKDEVACCESS_LOWLIMIT = 0,
    DCLOSBLOCKDEVACCESS_READ,
    DCLOSBLOCKDEVACCESS_WRITE,
    DCLOSBLOCKDEVACCESS_READWRITE,
    DCLOSBLOCKDEVACCESS_HIGHLIMIT
} DCLOSBLOCKDEVACCESS;


typedef enum
{
    DCLBLOCKDEVTYPE_LOWLIMIT = 0,
    DCLBLOCKDEVTYPE_GENERIC,        /* Generic block device         */  
    DCLBLOCKDEVTYPE_FILTER,         /* Generic filter               */
    DCLBLOCKDEVTYPE_DLRAMDEV,       /* Standard DCL RAM device      */
    DCLBLOCKDEVTYPE_DLFILEDEV,      /* Standard DCL FILE device     */
    DCLBLOCKDEVTYPE_DLLOGFILTER,    /* Standard DCL LOG filter      */
    DCLBLOCKDEVTYPE_DLCACHEFILTER,  /* Standard DCL CACHE filter    */
    DCLBLOCKDEVTYPE_DLFLASHFX,      /* FlashFX device               */
    DCLBLOCKDEVTYPE_DLRELIANCE,     /* Legacy Reliance device       */
    DCLBLOCKDEVTYPE_ATA,
    DCLBLOCKDEVTYPE_ATAPI,
    DCLBLOCKDEVTYPE_CDROM,
    DCLBLOCKDEVTYPE_DVD,
    DCLBLOCKDEVTYPE_COMPACTFLASH,
    DCLBLOCKDEVTYPE_DISKONCHIP,
    DCLBLOCKDEVTYPE_FLASH,          /* Something other than FlashFX */
    DCLBLOCKDEVTYPE_PCCARD,
    DCLBLOCKDEVTYPE_PCIIDE,
    DCLBLOCKDEVTYPE_USB,
    DCLBLOCKDEVTYPE_HIGHLIMIT
} DCLBLOCKDEVTYPE;


/*  Possible values for DclOsBlockDevCreate()'s ulFlags field.
*/
#define DCLOSBLOCKDEVFLAG_CACHED    0x0001

/*  The handle is an opaque pointer to the internal control structure
*/
typedef struct sDCLOSBLOCKDEVCONTROL *DCLOSBLOCKDEVHANDLE;

/*  I/O is done through function pointers
*/
typedef DCLIOSTATUS (FNDCLOSBLOCKDEVREAD)    (DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, D_BUFFER *pBuffer);
typedef DCLIOSTATUS (FNDCLOSBLOCKDEVWRITE)   (DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount, const D_BUFFER *pBuffer);
typedef DCLIOSTATUS (FNDCLOSBLOCKDEVDISCARD) (DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount);
typedef DCLIOSTATUS (FNDCLOSBLOCKDEVFLUSH)   (DCLOSBLOCKDEVHANDLE hDev, D_UINT64 ullStart, D_UINT32 ulCount);

/*  Upon creating an instance, a simple structure is filled in which
    contains the basic geometry and function pointers for doing I/O.
*/
typedef struct
{
    D_UINT64                ullTotalSectors;
    D_UINT64                ullCylinders;
    D_UINT64                ullHeads;
    D_UINT64                ullSectorsPerTrack;
    D_UINT32                ulBytesPerSector;
    DCLBLOCKDEVTYPE         nDeviceType;
    FNDCLOSBLOCKDEVREAD    *pfnRead;
    FNDCLOSBLOCKDEVWRITE   *pfnWrite;
    FNDCLOSBLOCKDEVDISCARD *pfnDiscard;
    FNDCLOSBLOCKDEVFLUSH   *pfnFlush;
    unsigned                fAlignedIo      :1;
} DCLOSBLOCKDEVINFO;


#define DclOsBlockDevCreate           DCLFUNC(DclOsBlockDevCreate)
#define DclOsBlockDevDestroy          DCLFUNC(DclOsBlockDevDestroy)

DCLSTATUS DclOsBlockDevCreate(const char *pszDevName, DCLOSBLOCKDEVACCESS nMode, D_UINT32 ulFlags, DCLOSBLOCKDEVHANDLE *phDev, DCLOSBLOCKDEVINFO *pDevInfo);
DCLSTATUS DclOsBlockDevDestroy(DCLOSBLOCKDEVHANDLE hDev);



#endif  /* DLOSBLOCKDEV_H_INCLUDED */


