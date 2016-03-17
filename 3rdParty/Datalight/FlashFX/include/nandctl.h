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

    Interface to low-level NAND flash control functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandctl.h $
    Revision 1.22  2011/11/22 14:21:14Z  jimmb
    Removed duplicate prototype declaration for the mx51 and mx35
    Revision 1.21  2011/04/06 20:10:35Z  jimmb
    Updated for support of the Tegra II NTM and Nand controller.
    Revision 1.20  2011/02/07 21:24:08Z  jimmb
    Added nVidia Tegra 2
    Revision 1.19  2009/12/11 22:19:58Z  garyp
    Moved the Micron specific stuff to fhmicron.h.
    Revision 1.18  2009/12/03 00:51:00Z  garyp
    Moved PXA320 stuff to fhpxa320.h.
    Revision 1.17  2009/10/06 15:26:27Z  garyp
    Added new NTMICRON hooks function prototypes.  Updated to 
    include fxnandapi.h.
    Revision 1.16  2009/03/12 21:42:51Z  keithg
    Removed trailing comma from enum lists to placate a number of
    compilers that complain.
    Revision 1.15  2009/02/26 17:15:41Z  glenns
    Moved LFA-related addressable-unit macros from "nand.h", as that interface
    file is really purposed for different sorts of things.
    Revision 1.14  2009/02/06 08:36:13Z  keithg
    Updated to reflect new location of NAND header files and macros.
    Revision 1.13  2008/06/02 14:07:42  keithg
    Removed unused and deprecated FIM_xxx error values.
    Revision 1.12  2008/05/03 03:32:01Z  garyp
    Removed some obsolete prototypes.
    Revision 1.11  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/09/07 21:20:09Z  Garyp
    Added prototypes for the Micron NTM hooks.
    Revision 1.9  2007/05/05 01:24:50Z  timothyj
    Added prototypes for new MX31 project hooks.
    Revision 1.8  2007/03/14 21:13:49Z  rickc
    Added hooks prototypes for ntpxa320 NTM.
    Revision 1.7  2007/02/13 23:38:22Z  timothyj
    Changed 'ulPage' parameter to some NTM functions to 'ulStartPage' for
    consistency.  Changed D_UINT16 uCount parameters to D_UIN32 ulCount, to
    allow the call tree all the way up through the IoRequest to avoid having
    to range check (and/or split) requests.  Removed corresponding casts.
    Revision 1.6  2006/10/06 00:20:46Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.5  2006/06/27 22:12:46Z  Garyp
    Updated the NTCAD hooks interface.
    Revision 1.4  2006/05/08 20:59:02Z  rickc
    Define BENDSWAP on 16bit NAND when not swapping
    Revision 1.3  2006/05/08 18:22:58Z  timothyj
    Added cadio NTM function hooks
    Revision 1.2  2006/02/10 10:38:22Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/11/25 20:27:00Z  Pauli
    Initial revision
    Revision 1.4  2005/11/25 20:26:59Z  Garyp
    Documentation update.
    Revision 1.3  2005/11/02 05:38:02Z  Garyp
    Added/updated prototypes for various NAND control interfaces.
    Revision 1.2  2005/10/22 13:19:40Z  garyp
    Added prototypes.
    Revision 1.1  2005/10/06 06:53:22Z  Garyp
    Initial revision
    Revision 1.4  2004/12/30 23:17:52Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.3  2004/08/12 01:20:03Z  GaryP
    Updated to use the renamed "nandctl" functions which are now part of the
    Project Hooks interface.
    Revision 1.2  2003/09/11 17:18:41Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.1  2003/08/19 22:54:02Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NANDCTL_H_INCLUDED
#define NANDCTL_H_INCLUDED

#include <fxdevice.h>
#include <nandid.h>

/*  FIM Status codes, abbreviated fr.

    NOTE: This type is deprecated and has been replaced with FFXSTATUS
          codes for most uses.  The only remaining place where this is
          still currently being used in the interface between the
          fhbyteio and ntbyteio modules.
*/
typedef enum
{
    FIM_OK = 0,                 /* worked */
    FIM_ERANGE,                 /* bad args */
    FIM_EWRITE_PROT,            /* flash chip is write protected */
    FIM_ETIMEOUT                /* timed out waiting for the flash chip */
} FIMResult_t;


/*  These macros augment different types operations based on the NAND I/O width.
*/
#if (NAND_DATA_WIDTH_BYTES == 1)
  #define NAND_IO_TYPE          D_UCHAR
  #define BENDSWAP(cmd)         (cmd)
#elif (NAND_DATA_WIDTH_BYTES == 2)
  #define NAND_IO_TYPE          D_UINT16
  #if ((!DCL_BIG_ENDIAN) ^ NAND_LITTLE_ENDIAN)
    #define BENDSWAP(cmd)       BENDSWAP16(cmd)
  #else
    #define BENDSWAP(cmd)       (cmd)
  #endif
#else
  #error "FFX: Unsupported NAND_DATA_WIDTH_BYTES value"
#endif

/*  These macros define the smallest addressable unit that NTMs that have
    been modified for LFA are capable of accessing. In most cases it is
    512 bytes, which is the size of a small-block page. If finer-grained
    addressing is required, an NTM that has not been modified for LFA must
    be used, or an LFA-enabled NTM must be modified to perform addressing
    a separate identifier for unit address and local offset.

    REFACTOR: The truly optimal solution for this feature is to use
    64-bit addressing, but since many of the platforms we must support
    are not capable of 64-bit addressing or arithmetic, we are currently
    relegated to addressing NAND chips in terms of "addressable units".
*/
#define ADDR_LINES_PER_ADDRESSABLE_UNIT (9)
#define BYTES_PER_ADDRESSABLE_UNIT (512UL)
#define BYTES_PER_KILOBYTE  (1024UL)

/*  An opaque platform-specific structure holding the information
    needed to perform physical I/O to the NAND chip(s).
*/
typedef struct nand_ctl *PNANDCTL;


/*  Read/write mode argument for NANDSetChipSelect()
*/
typedef enum
{
    MODE_READ = 'R',
    MODE_WRITE = 'W'
} CHIP_SELECT_MODE;


/*-------------------------------------------------------------------
    NTCAD NTM Hook interface routines.

    Universal routines for all NAND chips that require software
    control of the chip interface control lines.  The address lines
    greater than the MSB of the chip are shifted right and used as
    numerical chips selects.

    The production code assumes the classical AMD multiplexing
    scheme.  If the pins are mapped differently or have different
    polarity then these routines will have to be modified to account
    for that.
-------------------------------------------------------------------*/
PNANDCTL        FfxHookNTCADCreate(         FFXDEVHANDLE hDev);
FFXSTATUS       FfxHookNTCADDestroy(        PNANDCTL pNC);
FFXSTATUS       FfxHookNTCADSetChipSelect(  PNANDCTL pNC, D_UINT32 ulAddress, CHIP_SELECT_MODE mode);
FFXSTATUS       FfxHookNTCADReadyWait(      PNANDCTL pNC);
FFXSTATUS       FfxHookNTCADSetChipCommand( PNANDCTL pNC, D_UCHAR ucCommand);
FFXSTATUS       FfxHookNTCADSetChipCommandNoWait(PNANDCTL pNC, D_UCHAR ucCommand);
void            FfxHookNTCADDataIn(         PNANDCTL pNC, NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
void            FfxHookNTCADDataOut(        PNANDCTL pNC, const NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
NAND_IO_TYPE    FfxHookNTCADStatusIn(       PNANDCTL pNC);
void            FfxHookNTCADAddressOut(     PNANDCTL pNC, D_UCHAR address);


/*-------------------------------------------------------------------
    NTPAGEIO NTM Hook interface routines.
-------------------------------------------------------------------*/
typedef struct NANDFLASHINFO
{
    D_BUFFER    abID[NAND_ID_SIZE];
    D_UINT16    uDevices;               /* number of devices (chips) found */
    D_UINT16    uPadding;
} NANDFLASHINFO;

PNANDCTL        FfxHookNTPageCreate(       FFXDEVHANDLE hDev, NANDFLASHINFO *pNFI, FFXFIMBOUNDS *pBounds);
FFXSTATUS       FfxHookNTPageDestroy(      PNANDCTL pNC);
FFXIOSTATUS     FfxHookNTPageRead(         PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT16 uCount, D_BUFFER *pPageBuff, D_BUFFER *pSpareBuff);
FFXSTATUS       FfxHookNTPageReadStart(    PNANDCTL pNC, D_UINT32 ulPage);
FFXIOSTATUS     FfxHookNTPageReadComplete( PNANDCTL pNC, D_BUFFER *pPageBuff, D_BUFFER *pSpareBuff);
FFXIOSTATUS     FfxHookNTPageReadHidden(   PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT32 ulCount, D_UINT16 uScale, D_BUFFER *pBuffer);
FFXIOSTATUS     FfxHookNTPageWrite(        PNANDCTL pNC, D_UINT32 ulStartPage, D_UINT32 ulCount, const D_BUFFER *pPageBuff, const D_BUFFER *pSpareBuff);
FFXSTATUS       FfxHookNTPageWriteStart(   PNANDCTL pNC, D_UINT32 ulPage, const D_BUFFER *pPageBuff, const D_BUFFER *pSpareBuff);
FFXIOSTATUS     FfxHookNTPageWriteComplete(PNANDCTL pNC);
FFXIOSTATUS     FfxHookNTPageEraseBlock(   PNANDCTL pNC, D_UINT32 ulBlock);


/*-------------------------------------------------------------------
    NTMX31 NTM Hook interface routines.
-------------------------------------------------------------------*/
PNANDCTL        FfxHookNTMX31Create(        FFXDEVHANDLE hDev);
FFXSTATUS       FfxHookNTMX31Destroy(       PNANDCTL pNC);
D_BOOL          FfxHookNTMX31SetParameters( PNANDCTL pNC, D_UCHAR ucByteWidth, D_UINT16 ucPageSize);

/*-------------------------------------------------------------------
    NTTegra2 NTM Hook interface routines.
-------------------------------------------------------------------*/
PNANDCTL        FfxHookNTMTegra2Create(        FFXDEVHANDLE hDev);
FFXSTATUS       FfxHookNTMTegra2Destroy(       PNANDCTL pNC);
D_BOOL          FfxHookNTMTegra2Parameters( PNANDCTL pNC, D_UCHAR ucByteWidth, D_UINT16 ucPageSize);
void * FfxHookHwDmaAlloc(void *ptr,unsigned long size);
void FfxHookHwDmaFree(D_UINT32 dma_addr,unsigned long size,void *cpuAddr);

/*  It is necessary to temporarily place this at the bottom of this
    header to avoid having to modify every one of the hooks modules.
    Once the headers are properly factored, and fxnandapi.h is the
    top-level header for NAND stuff which is visible outside the FMSL
    tree, nandctl.h will probably go away entirely.
*/    
#include <fxnandapi.h>

#endif  /* NANDCTL_H_INCLUDED */

