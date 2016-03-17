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

    This module defines the array of supported flash devices.  Support for
    each device is controlled at compile time via defines in ffxconf.h.

    Naming conventions:
       x8    8 bit devices in an array
       2x8   8 bit devices odd bytes on one, even on another
       x16   16 bit devices in an array

    Supported devices are listed below.  #define the FIM Device Name in
    ffxconf.h to enable support for a given flash part.  Note that the
    FIM Device Names are case sensitive.

    FIM Device Name   FIM File Name
    ---------------   -------------
    FFXFIM_asux8      asux8.c
    FFXFIM_asu4x8     asu4x8.c
    FFXFIM_ambx16     ambx16.c
    FFXFIM_amb2x16    amb2x16.c
    FFXFIM_asbx16     asbx16.c
    FFXFIM_asb2x16    asb2x16.c
    FFXFIM_iffx8      iffx8.c
    FFXFIM_iffx16     iffx16.c
    FFXFIM_iff2x16    iff2x16.c
    FFXFIM_isfx16     isfx16.c
    FFXFIM_isf2x16    isf2x16.c
    FFXFIM_isf4x8     isf4x8.c
    FFXFIM_iswfx16    iswfx16.c
    FFXFIM_norram     norram.c
    FFXFIM_norfile    norfile.c
    FFXFIM_norfim     norfim.c  (Universal NOR FIM with CFI support)
    FFXFIM_nand       nand.c    (Requires an NTM to be specified as well)

    See the FIM list, available on the Datalight web site, for a complete
    list of supported flash parts.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: devflash.c $
    Revision 1.14  2011/05/24 22:27:51Z  billr
    Add Linux MTD FIM.
    Revision 1.13  2009/12/23 18:49:28Z  keithg
    Increased FFX_MAX_DEVICES check to support the previous checkin.
    Revision 1.12  2009/12/09 23:33:19Z  keithg
    Increased support for FFX_MAX_DEVICES of 8.
    Revision 1.11  2009/04/01 17:18:14Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.10  2008/03/06 18:46:07Z  Glenns
    Add Support for Eclipse FIM
    Revision 1.9  2007/11/07 17:25:25Z  pauli
    Made #error message strings to prevent macro expansion.
    Revision 1.8  2007/11/03 23:49:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/05/25 20:17:24Z  pauli
    Made FFXFIM_norfim available if either NOR or Sibley support is
    enabled.
    Revision 1.6  2007/01/02 22:26:58Z  rickc
    Replaced cfix16 and cfi2x16 with norfim
    Revision 1.5  2006/11/01 22:41:26Z  rickc
    Added cfix16 and cfi2x16 FIMs
    Revision 1.4  2006/08/11 23:29:53Z  rickc
    Added ambx16 FIM
    Revision 1.3  2006/04/04 00:14:08Z  michaelm
    removed mention of fimlist.pdf and instead refer to website
    Revision 1.2  2006/02/10 22:40:19Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.1  2006/02/09 23:50:10Z  Garyp
    Initial revision
    Revision 1.4  2006/01/13 12:30:11Z  Rickc
    Added asbx16 FIM.
    Revision 1.3  2006/01/12 03:57:08Z  Garyp
    Documentation, debug code, and general cleanup -- no functional changes.
    Revision 1.2  2006/01/11 23:04:55Z  billr
    Merge Sibley support from v2.01.
    Revision 1.1  2005/11/16 01:00:28Z  Pauli
    Initial revision
    Revision 1.2  2005/11/16 01:00:28Z  Garyp
    Eliminated single-threaded FIM support.
    Revision 1.1  2005/10/05 17:43:34Z  Garyp
    Initial revision
    Revision 1.142  2005/05/11 20:41:49Z  garyp
    Added asux8.  Removed amdx8.
    Revision 1.141  2005/05/05 18:45:09Z  garyp
    Added iff2x16.c.
    Revision 1.140  2005/04/07 00:18:21Z  tonyq
    Added new 4x8 FIM for WindRiver's PowerQuicII board
    Revision 1.139  2005/02/11 21:33:29Z  billr
    Add NANDRAM.
    Revision 1.138  2004/12/30 23:24:51Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.137  2004/09/24 20:27:52Z  garys
    removed old Intelx16 and IStrtx16 FIMs
    Revision 1.135  2004/08/10 21:41:34  jaredw
    Added isfx16 MT fim.
    Revision 1.134  2004/08/06 19:32:39Z  GaryP
    Replace the ramdisk FIM with the norram FIM.
    Revision 1.133  2004/07/28 23:56:01Z  BillR
    Add file simulation of NOR flash.
    Revision 1.132  2004/07/26 18:53:42Z  jaredw
    Added iffx8, iffx16, asb2x16, amb2x16 MT fims.
    Revision 1.131  2004/07/02 19:40:46Z  jaredw
    Added MT fim asu4x8
    Revision 1.13  2004/01/24 23:10:20Z  garys
    Merge from FlashFXMT
    Revision 1.9.1.11  2004/01/24 23:10:20  garyp
    Modified to use the new style FFXCONF_... settings.
    Revision 1.9.1.10  2004/01/05 00:42:26Z  garyp
    Comment changes only.
    Revision 1.9.1.9  2003/12/13 22:04:56Z  garyp
    Added RAMDISK to the MT FIMs and removed it from the old-style FIMs.
    Revision 1.9.1.8  2003/12/05 22:38:32Z  garyp
    Added some OSE specific FIMs.
    Revision 1.9.1.7  2003/12/03 19:15:01Z  garys
    added extern for AmdMB2x16
    Revision 1.9.1.6  2003/12/03 19:01:51  garys
    added AMD MirrorBit FIM
    Revision 1.9.1.5  2003/11/22 02:01:32  garyp
    Modified to generate a compile time error if a FIM is not defined.
    Revision 1.9.1.4  2003/11/07 07:26:18Z  garyp
    Changed a FIM name to the new standard.
    Revision 1.9.1.3  2003/11/06 20:29:16Z  garyp
    Added a real FIM2 FIM.
    Revision 1.9.1.2  2003/11/03 19:20:04Z  garyp
    Re-checked into variant sandbox.
    Revision 1.10  2003/11/03 19:20:04Z  garyp
    Added a new device list for new style FIMs.
    Revision 1.9  2003/06/13 20:59:46Z  garys
    removed istrt2x8 and amdtb2x16
    Revision 1.8  2003/06/12 23:00:21  garys
    removed FIMs that moved to the extras dir
    Revision 1.7  2003/04/21 06:14:20  garyp
    Eliminated TestRAM.
    Revision 1.6  2003/04/15 20:30:38Z  garyp
    Added RAMDISK.  Eliminated a bunch of obsolete FIMs.
    Revision 1.5  2003/01/10 19:18:06Z  garyp
    Added AMD4X8.
    06/08/02 gjs Added the Nec2x16 FIM.
    02/07/02 DE  Updated copyright notice for 2002.
    06/13/01 gjs Added Intelx16 FIM /w pkg.
    11/10/00 gjs Added BigAmd4x8 and BigAmd2x16 for bam4x8 and bam2x16 FIMs.
    02/26/00 HDS Added support for the xamd016, xamd040, xat040 and sst040 FIMs.
    09/24/99 HDS Added support for the Intel StrataFlash 2x8 mode.
    09/01/99 TWQ Added support for the Intel 28F128 Strata Flash x16
    02/18/99 HDS Added support for the AmdTBx16 and AmTB2x16 device group.
    02/09/99 TWQ Added new UltraNand FIM structure and defines
    11/18/98 DM  Added new FIM, FimFile, structure and defines
    10/21/98 HDS Added Big Real Mode (addresses extended memory directly) test
                 support.
    06/30/98 PKG Added Toshiba/Samsung NAND device support
    04/30/98 HDS Added the Hitachi AND device support.
    11/19/97 HDS Changed include file search to use predefined path.
    10/21/97 HDS Changed amd device name to Amdx8 for byte version and
                 Amd2x8 for interleaved version.
    10/20/97 HDS Changed intel device name to Intelx8 for byte version
                 and Intel2x8 for interleaved version.
    10/13/97 HDS Changed am29F080x8 device name to Amd, which supports
                 both the am29F080x8 and am29F016x16 devices.
    09/26/97 HDS Changed intel device name to Intel2
    07/09/97 PKG Updated supported device list
    12/31/96 PKG Created
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxdevapi.h>
#include <fimdev.h>
#include "devflash.h"

#ifdef FIM_SPECIFIED
#undef FIM_SPECIFIED
#endif

                    /*------------------*\
                     *                  *
                     *       FIMs       *
                     *                  *
                    \*------------------*/

#if FFXCONF_NORSUPPORT
    extern FIMDEVICE FFXFIM_asux8;
    extern FIMDEVICE FFXFIM_asu4x8;
    extern FIMDEVICE FFXFIM_ambx16;
    extern FIMDEVICE FFXFIM_amb2x16;
    extern FIMDEVICE FFXFIM_asbx16;
    extern FIMDEVICE FFXFIM_asb2x16;
    extern FIMDEVICE FFXFIM_iffx8;
    extern FIMDEVICE FFXFIM_iffx16;
    extern FIMDEVICE FFXFIM_iff2x16;
    extern FIMDEVICE FFXFIM_isfx16;
    extern FIMDEVICE FFXFIM_isf2x16;
    extern FIMDEVICE FFXFIM_isf4x8;
    extern FIMDEVICE FFXFIM_norram;
    extern FIMDEVICE FFXFIM_norfile;
#endif
#if FFXCONF_NANDSUPPORT
    extern FIMDEVICE FFXFIM_nand;
    extern FIMDEVICE FFXFIM_MTD;
#endif
#if FFXCONF_ISWFSUPPORT
    extern FIMDEVICE FFXFIM_iswfx16;
    extern FIMDEVICE FFXFIM_eclipsex16;
#endif
#if FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT
    extern FIMDEVICE FFXFIM_norfim;
#endif

                    /*------------------*\
                     *                  *
                     *   Device List    *
                     *                  *
                    \*------------------*/

#if FFX_MAX_DEVICES > 8
    #error "FFX: FFX_MAX_DEVICES is larger than what devflash.c supports"

    /*  If you get this particular build error and you need to support
        more than 8 flash devices, add the appropriate logic to the
        code below, and increase the value used in this error check
        accordingly.
    */
#endif


#if FFX_MAX_DEVICES > 0 && defined(FFX_DEV0_FIMS)
    static FIMDEVICE *pDevList_0[] = FFX_DEV0_FIMS;
    #define FIM_SPECIFIED
#endif
#if FFX_MAX_DEVICES > 1 && defined(FFX_DEV1_FIMS)
    static FIMDEVICE *pDevList_1[] = FFX_DEV1_FIMS;
    #define FIM_SPECIFIED
#endif
#if FFX_MAX_DEVICES > 2 && defined(FFX_DEV2_FIMS)
    static FIMDEVICE *pDevList_2[] = FFX_DEV2_FIMS;
    #define FIM_SPECIFIED
#endif
#if FFX_MAX_DEVICES > 3 && defined(FFX_DEV3_FIMS)
    static FIMDEVICE *pDevList_3[] = FFX_DEV3_FIMS;
    #define FIM_SPECIFIED
#endif
#if FFX_MAX_DEVICES > 4 && defined(FFX_DEV4_FIMS)
    static FIMDEVICE *pDevList_4[] = FFX_DEV4_FIMS;
    #define FIM_SPECIFIED
#endif

#if FFX_MAX_DEVICES > 5 && defined(FFX_DEV5_FIMS)
    static FIMDEVICE *pDevList_5[] = FFX_DEV5_FIMS;
    #define FIM_SPECIFIED
#endif

#if FFX_MAX_DEVICES > 6 && defined(FFX_DEV6_FIMS)
    static FIMDEVICE *pDevList_6[] = FFX_DEV6_FIMS;
    #define FIM_SPECIFIED
#endif

#if FFX_MAX_DEVICES > 7 && defined(FFX_DEV7_FIMS)
    static FIMDEVICE *pDevList_7[] = FFX_DEV7_FIMS;
    #define FIM_SPECIFIED
#endif


FIMDEVICE  **ppFIMDevList[FFX_MAX_DEVICES+1] =
{
  #if FFX_MAX_DEVICES > 0
  #ifdef FFX_DEV0_FIMS
    &pDevList_0[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 1
  #ifdef FFX_DEV1_FIMS
    &pDevList_1[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 2
  #ifdef FFX_DEV2_FIMS
    &pDevList_2[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 3
  #ifdef FFX_DEV3_FIMS
    &pDevList_3[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 4
  #ifdef FFX_DEV4_FIMS
    &pDevList_4[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 5
  #ifdef FFX_DEV5_FIMS
    &pDevList_5[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 6
  #ifdef FFX_DEV6_FIMS
    &pDevList_6[0],
  #else
    NULL,
  #endif
  #endif
  #if FFX_MAX_DEVICES > 7
  #ifdef FFX_DEV7_FIMS
    &pDevList_7[0],
  #else
    NULL,
  #endif
  #endif
    NULL
};


#ifndef FIM_SPECIFIED
    #error "FFX: A FIM must be specified"
#endif
