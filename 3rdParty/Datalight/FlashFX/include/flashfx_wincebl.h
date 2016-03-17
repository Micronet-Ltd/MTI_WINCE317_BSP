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

    This header defines the Loader function interface to FlashFX.

    This header is designed to be included by code that is built <outside>
    the Datalight standardized build process.

    To keep the requirements for accomplishing this as simple as possible,
    this header should only include other headers that exist in the same
    directory as this header file -- hence the use of "" for the included
    Datalight headers, rather than <>.

    Using this header MAY require the D_TOOLSETNUM value to be defined
    prior to including this header.  If the dcl_loader.h header does NOT have
    enough information to derive the D_TOOLSETNUM value, define it prior to
    including this header, using the recognized values defined in dltlset.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: flashfx_wincebl.h $
    Revision 1.7  2010/07/18 00:48:19Z  garyp
    Updated so the "AltUnlock" functionality is optional, and must be
    explicitly turned on.
    Revision 1.6  2009/12/09 19:51:30Z  garyp
    Added the fAutoUnlock field.
    Revision 1.5  2009/08/07 23:20:21Z  garyp
    Removed the DCLFUNC definition and fxstatus.h include since they are
    handled at lower levels.  Fixed the dcl_osname.h inclusion to work properly
    in a 4GR build tree, when code is being built outside the DL build process.
    Revision 1.4  2009/07/29 18:24:34Z  garyp
    Merged from the v4.0 branch.  Removed support for the CE Loader interface,
    which has been replaced by the FlashFX Loader (include fxloaderapi.h).
    Added IOCTL_FMD_FLASHFX to allow FMD specific clients to talk to FlashFX
    using FMD_OEMIoControl.
    Revision 1.3  2009/04/02 15:56:44Z  thomd
    Updated to include fxstatus.h
    Revision 1.2  2009/02/09 02:28:47Z  garyp
    Merged from the v4.0 branch.  Updated to initialize D_PRODUCTNUM if it
    is not already initialized.  Added prototypes.  Made the FMD functions
    conditional.
    Revision 1.1  2008/03/15 18:49:54Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FLASHFX_WINCEBL_H_INCLUDED
#define FLASHFX_WINCEBL_H_INCLUDED


#ifndef __WINDOWS__
  #error "windows.h must be included before flashfx_wincebl.h"
#endif


/*-------------------------------------------------------------------
    If both flashfx_wincebl.h and fxapi.h are necessary, the former
    must be first.
-------------------------------------------------------------------*/
#if !defined(DL4GR_H_INCLUDED) && !defined(FLASHFX_H_INCLUDED) && defined(FXAPI_H_INCLUDED)
#error "flashfx_wincebl.h must be included before fxapi.h"
#endif


/*-------------------------------------------------------------------
    Define product specific characteristics.  This is handled in an
    #ifndef block for two reasons.  This could be a subproduct, in
    which case the master product has already defined this value.
    Additionally, this code <could> be compiled by the standard
    Datalight build process, where the value is already defined.

    Note that the PRODUCTNUM_* values are defined <later> when
    dlprod.h is included.
-------------------------------------------------------------------*/
#ifndef D_PRODUCTNUM
  #define D_PRODUCTNUM          (PRODUCTNUM_FLASHFX)
#endif


/*-------------------------------------------------------------------
    If neither dcl.h nor dcl_wincebl.h have already been included
    by higher level headers, we <may> be here because we're being
    included by externally built software which will not have
    the DCL include path.  Therefore use a path relative to the
    current directory.
-------------------------------------------------------------------*/
#ifndef DCL_H_INCLUDED
  #ifndef DCL_WINCEBL_H_INCLUDED
    #if defined(D_PRODUCTNUM) && defined(D_SUBPRODUCTNUM)
      /*  In the event that FlashFX is a sub-product, and we're being
          built by our standard build process, the DCL include directory
          is one level deeper.
      */
      #include "../../dcl/include/dcl_wincebl.h"
    #else
      /*  Not a subproduct, or not being built by the Datalight build
          process, so the DCL include directory should be located in
          the FlashFX tree.
      */
      #include "../dcl/include/dcl_wincebl.h"
    #endif
  #endif
#endif


/*-------------------------------------------------------------------
    Include FlashFX headers which provide access to the portions of
    the publicly exposed FlashFX API which are not OS-specific.

    Note the use of quotes rather than angle brackets.  Because this
    code may be built outside the Datalight build process, this
    header may only include Datalight headers which can be located
    relative to the current directory in which this header resides.
-------------------------------------------------------------------*/
#include "fxapi.h"


/*-------------------------------------------------------------------
    Structures and Types for Public Functions
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Prototypes for Public Functions
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    FMD-To-FlashFX Interface

    This interface requires that the Microsoft header "fmd.h" already
    be included.  If it is not, leave the interface undefined.
-------------------------------------------------------------------*/
#ifdef _FMD_H_

typedef struct
{
    unsigned        nDeviceNum;
    unsigned        nDiskNum;
    unsigned long   ulExtraReservedLowKB;
    unsigned long   ulExtraReservedHighKB;
    unsigned long   ulMaxArraySizeKB;
    unsigned        nRemapTableSize;
    UINT16         *puRemapTable;
    unsigned        fUseBBM : 1;
    unsigned        fUseAllocator : 1;
    unsigned        fSpanDevices : 1;
    unsigned        fAutoUnlock : 1;
    unsigned        fAltUnlock : 1;
} FFXWMFMDRANGE;

PVOID   FfxWM_FMD_Init(LPCTSTR pActive, PPCI_REG_INFO pRegIn, PPCI_REG_INFO pRegOut);
BOOL    FfxWM_FMD_InitRanges(VOID *pContext, FFXWMFMDRANGE *pWMFMDRange1, FFXWMFMDRANGE *pWMFMDRange2);
BOOL    FfxWM_FMD_Deinit(VOID *pContext);
BOOL    FfxWM_FMD_GetInfo(FlashInfo *pFlashInfo);
BOOL    FfxWM_FMD_ReadSector(SECTOR_ADDR ulStartSector, UCHAR *pClientBuffer, SectorInfo *pSectorInfo, DWORD ulCount);
BOOL    FfxWM_FMD_WriteSector(SECTOR_ADDR ulStartSector, UCHAR *pClientBuffer, SectorInfo *pSectorInfo, DWORD ulCount);
DWORD   FfxWM_FMD_GetBlockStatus(BLOCK_ID blockId);
BOOL    FfxWM_FMD_SetBlockStatus(BLOCK_ID blockId, DWORD status);
BOOL    FfxWM_FMD_EraseBlock(BLOCK_ID blockId);
VOID    FfxWM_FMD_PowerUp(VOID);
VOID    FfxWM_FMD_PowerDown(VOID);
BOOL    FfxWM_FMD_OEMIoControl(DWORD dwIoControlCode, UCHAR *pInBuf, DWORD dwInBufSize, UCHAR *pOutBuf, DWORD dwOutBufSize, DWORD *pBytesReturned);

/*  The IOCTL_DISK_USER() macro is defined in diskio.h, and sets aside
    32 function numbers.  The first 12 or so are defined in fmd.h for
    use by FMD specific functions.  We arbitrarily appropriate one to
    allow FlashFX functionality to be accessed through the FMD IOCTL
    interface.

    No easy way to tell for certain if this number will conflict with
    some other software, however it should only be used in situations
    when FlashFX is known to be in the system and hooked into the
    FMD interface, and therefore FlashFX will get first crack at
    handling the request.
*/
#define IOCTL_FMD_FLASHFX   IOCTL_DISK_USER(30)


#endif


#endif /* FLASHFX_WINCEBL_H_INCLUDED */

