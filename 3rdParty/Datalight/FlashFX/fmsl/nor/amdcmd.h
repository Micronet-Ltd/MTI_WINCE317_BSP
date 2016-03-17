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

    This header file contain macros for building flash commands for AMD
    flash memory.  It must be included after flashcmd.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: amdcmd.h $
    Revision 1.15  2009/12/03 20:57:54Z  garyp
    Moved manufacturer ID codes into nor.h.  Renamed ID_STMICRO to
    ID_NUMONYX.
    Revision 1.14  2008/09/17 20:21:20Z  Glenns
    Added support for JEDEC continuation code and Spansion Manufacturer ID.
    Revision 1.13  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2006/12/15 22:21:00Z  rickc
    Added Samsung K8P2915UQB
    Revision 1.11  2006/09/07 01:12:03Z  rickc
    Fixed dev IDs
    Revision 1.10  2006/08/31 05:33:00Z  pauli
    Eliminated the AMD_SEND_CMD macro.
    Revision 1.9  2006/07/20 23:47:21Z  Pauli
    Added support for K8A5615ETA and K8C5615EBM.
    Revision 1.8  2006/06/07 21:29:36Z  rickc
    Added Spansion S29PL129J
    Revision 1.7  2006/05/30 18:14:09Z  rickc
    Corrected part name typo
    Revision 1.6  2006/05/20 20:25:29Z  Garyp
    Updated the chip params structure to be more flexible with regard to boot
    blocks.
    Revision 1.5  2006/02/03 19:35:06Z  Garyp
    Modified to use new FfxHookWindowMap/Size() functions.  Updated
    debugging code.
    Revision 1.4  2006/01/23 22:34:24Z  Rickc
    Changed boot block and erase zone size to reflect single chip and not 2x16
    interleave value.
    Revision 1.3  2006/01/13 17:49:53Z  Rickc
    Corrected chip id
    Revision 1.2  2006/01/13 12:21:04Z  Rickc
    Added MLC flag and block sizes to chip params, also added samsung parts
    Revision 1.1  2005/10/06 05:46:20  Pauli
    Initial revision
    Revision 1.3  2005/05/12 04:30:55Z  garyp
    Corrected the 8-bit parts to properly specify "no boot block".
    Revision 1.2  2005/05/10 23:27:46Z  garyp
    Added a standard ID mechanism.
    Revision 1.1  2005/05/08 20:22:04Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef AMDCMD_H_INCLUDED
#define AMDCMD_H_INCLUDED


/*---------------------------------------------------------
    Single-Chip Command Codes and Status
---------------------------------------------------------*/
#define ACMD_ID                 (0x90)
#define ACMD_PROG               (0xA0)
#define ACMD_READMODE           (0xF0)
#define ACMD_ERASE1             (0x80)
#define ACMD_ERASE2             (0x30)
#define ACMD_ERASESUSPEND       (0xB0)
#define ACMD_UNLOCK1            (0xAA)
#define ACMD_UNLOCK2            (0x55)
#define ACMD_UNLOCK_BYPASS      (0x20)
#define ACMD_BYPASS_RESET       (0x00)
#define ACMD_WRITETOBUFFER      (0x25)
#define ACMD_BUFFERTOFLASH      (0x29)

#define ASTAT_OK                (0x80)  /* DQ7 Data# polling                */
#define ASTAT_TOGGLEDONE        (0x40)  /* DQ6 Toggle bit 1                 */
#define ASTAT_FAIL              (0x20)  /* DQ5 exceeded timing limits       */
#define ASTAT_ERASEBEGUN        (0x08)  /* DQ3 sector erase timer           */
#define ASTAT_BUFFERABORT       (0x02)  /* DQ1 Write-to-Buffer Abort (high) */
#define ASTAT_MASK              (0x88)
#define ASTAT_JEDECCONTINUATINON (0x7F)


/*---------------------------------------------------------
    Multi-Chip Command Codes and Status
---------------------------------------------------------*/
#define AMDCMD_IDENTIFY                MAKEFLASHDUPCMD(ACMD_ID)
#define AMDCMD_PROGRAM                 MAKEFLASHDUPCMD(ACMD_PROG)
#define AMDCMD_READ_MODE               MAKEFLASHDUPCMD(ACMD_READMODE)
#define AMDCMD_ERASE1                  MAKEFLASHDUPCMD(ACMD_ERASE1)
#define AMDCMD_ERASE2                  MAKEFLASHDUPCMD(ACMD_ERASE2)
#define AMDCMD_ERASE_SUSPEND           MAKEFLASHDUPCMD(ACMD_ERASESUSPEND)
#define AMDCMD_UNLOCK1                 MAKEFLASHDUPCMD(ACMD_UNLOCK1)
#define AMDCMD_UNLOCK2                 MAKEFLASHDUPCMD(ACMD_UNLOCK2)
#define AMDCMD_UNLOCK_BYPASS           MAKEFLASHDUPCMD(ACMD_UNLOCK_BYPASS)
#define AMDCMD_BYPASS_RESET            MAKEFLASHDUPCMD(ACMD_BYPASS_RESET)
#define AMDCMD_WRITETOBUFFER           MAKEFLASHDUPCMD(ACMD_WRITETOBUFFER)
#define AMDCMD_BUFFERTOFLASH           MAKEFLASHDUPCMD(ACMD_BUFFERTOFLASH)

#define AMDSTAT_DONE                   MAKEFLASHDUPCMD(ASTAT_OK)
#define AMDSTAT_FAIL                   MAKEFLASHDUPCMD(ASTAT_FAIL)
#define AMDSTAT_TOGGLEDONE             MAKEFLASHDUPCMD(ASTAT_TOGGLEDONE)
#define AMDSTAT_ERASEBEGUN             MAKEFLASHDUPCMD(ASTAT_ERASEBEGUN)
#define AMDSTAT_MASK                   MAKEFLASHDUPCMD(ASTAT_MASK)
#define AMDSTAT_JEDECCONTINUATION      MAKEFLASHDUPCMD(ASTAT_JEDECCONTINUATINON)


/*---------------------------------------------------------
    FIM ID structure and flags
---------------------------------------------------------*/
typedef struct AMDCHIPPARAMS_TAG
{
    FLASHID     idMfg;
    FLASHID     idDev;
    D_UINT32    ulChipSize;
    D_UINT32    ulBlockSize;
    D_UINT32    ulBootBlockSize;
    D_UINT16    uLowBootBlocks;     /* count of full blocks divided into boot blocks */
    D_UINT16    uHighBootBlocks;    /* count of full blocks divided into boot blocks */
    D_UINT16    uDevNotMlc;
    D_UINT16    uIDFlags;
} AMDCHIPPARAMS;
 
#define IDFLAG_RESERVED0        0x0001  /* Low 8 bits are reserved for      */
#define IDFLAG_RESERVED1        0x0002  /* general cross-FIM use            */
#define IDFLAG_RESERVED2        0x0004
#define IDFLAG_RESERVED3        0x0008
#define IDFLAG_RESERVED4        0x0010
#define IDFLAG_RESERVED5        0x0020
#define IDFLAG_RESERVED6        0x0040
#define IDFLAG_RESERVED7        0x0080
#define IDFLAG_FIM0             0x0100  /* High 8 bits are available for    */
#define IDFLAG_FIM1             0x0200  /* use in individual FIMs           */
#define IDFLAG_FIM2             0x0400
#define IDFLAG_FIM3             0x0800
#define IDFLAG_FIM4             0x1000
#define IDFLAG_FIM5             0x2000
#define IDFLAG_FIM6             0x4000
#define IDFLAG_FIM7             0x8000
#define IDFLAG_ENDOFLIST        0xFFFF  /* End-of-List marker               */

/*---------------------------------------------------------
    AMD 8-bit parts, and clones
    
    ulBlockSize, ulBootBlockSize, and uDevNotMlc
    currently not used in asux8.c and asu4x8.c
---------------------------------------------------------*/
#define ID_AM29F040(fl)          ID_AMD,       0xA4, 0x00080000UL, 0, 0, 0, 0, 0, (fl) /*  4Mb  */
#define ID_AM29F080(fl)          ID_AMD,       0xD5, 0x00100000UL, 0, 0, 0, 0, 0, (fl) /*  8Mb  */
#define ID_AM29F016(fl)          ID_AMD,       0xAD, 0x00200000UL, 0, 0, 0, 0, 0, (fl) /* 16Mb  */
#define ID_AM29F032B(fl)         ID_AMD,       0x41, 0x00400000UL, 0, 0, 0, 0, 0, (fl) /* 32Mb  */
#define ID_AM29LV081(fl)         ID_AMD,       0x38, 0x00100000UL, 0, 0, 0, 0, 0, (fl) /*  8Mb  */
#define ID_AM29LV017B(fl)        ID_AMD,       0xC8, 0x00200000UL, 0, 0, 0, 0, 0, (fl) /* 16Mb  */
#define ID_AM29LV033C(fl)        ID_AMD,       0xA3, 0x00400000UL, 0, 0, 0, 0, 0, (fl) /* 32Mb  */
#define ID_AM29LV065D(fl)        ID_AMD,       0x93, 0x00800000UL, 0, 0, 0, 0, 0, (fl) /* 64Mb  */
#define ID_AMC0XXDFLKA(fl)       ID_AMD,       0x3D, 0x00200000UL, 0, 0, 0, 0, 0, (fl) /* 16Mb  */

/*---------------------------------------------------------
    16-bit parts, various manufacturers
---------------------------------------------------------*/
/*                               idMfg       idDev   ChipSize      BlockSize     BootBlockSize L  H  uDevNotMlc   uIDFlags */
#define ID_AM29LV641D(fl)        ID_AMD,     0x22D7, 0x00800000UL, 0x00010000UL, 0x00002000UL, 0, 0, DEV_NOT_MLC, (fl) /* uniform sector */
#define ID_S29PL129J(fl)         ID_AMD,     0x227E, 0x01000000UL, 0x00010000UL, 0x00002000UL, 1, 1, DEV_NOT_MLC, (fl) /* top and bottom */
#define ID_MBM29LV160(fl)        ID_FUJITSU, 0x22C4, 0x00200000UL, 0x00010000UL, 0x00002000UL, 0, 1, DEV_NOT_MLC, (fl) /* top only */
#define ID_MBM29LV650_651UE(fl)  ID_FUJITSU, 0x22D7, 0x00800000UL, 0x00010000UL, 0x00002000UL, 0, 0, DEV_NOT_MLC, (fl) /* uniform sector */
#define ID_UPD29F032203ALT(fl)   ID_NEC,     0x2250, 0x00400000UL, 0x00010000UL, 0x00002000UL, 0, 1, DEV_NOT_MLC, (fl) /* top only */
#define ID_UPD29F032203ALB(fl)   ID_NEC,     0x2253, 0x00400000UL, 0x00010000UL, 0x00002000UL, 1, 0, DEV_NOT_MLC, (fl) /* bottom only */
#define ID_UPD29F032116(fl)      ID_NEC,     0x221B, 0x00400000UL, 0x00010000UL, 0x00002000UL, 1, 1, DEV_NOT_MLC, (fl) /* top and bottom */
#define ID_UPD29F064115(fl)      ID_NEC,     0x221C, 0x00800000UL, 0x00010000UL, 0x00002000UL, 1, 1, DEV_NOT_MLC, (fl) /* top and bottom */
#define ID_M29W641D(fl)          ID_NUMONYX, 0x22C7, 0x00800000UL, 0x00010000UL, 0x00002000UL, 0, 0, DEV_NOT_MLC, (fl) /* uniform sector */
#define ID_K8A5615EBA(fl)        ID_SAMSUNG, 0x22FD, 0x02000000UL, 0x00010000UL, 0x00002000UL, 1, 0, DEV_NOT_MLC, (fl) /* bottom only */
#define ID_K8A5615ETA(fl)        ID_SAMSUNG, 0x22FC, 0x02000000UL, 0x00010000UL, 0x00002000UL, 0, 1, DEV_NOT_MLC, (fl) /* top only */
#define ID_K8C5615EBM(fl)        ID_SAMSUNG, 0x2207, 0x02000000UL, 0x00020000UL, 0x00008000UL, 1, 0, 0          , (fl) /* bottom only */
#define ID_K8C5615ETM(fl)        ID_SAMSUNG, 0x2206, 0x02000000UL, 0x00020000UL, 0x00008000UL, 0, 1, 0          , (fl) /* top only */
#define ID_K8P2915UQB(fl)        ID_SAMSUNG, 0x257E, 0x01000000UL, 0x00010000UL, 0x00002000UL, 1, 1, DEV_NOT_MLC, (fl) /* top and bottom */
#define ID_ENDOFLIST                    0x0,    0x0,          0x0,          0x0,          0x0, 0, 0, DEV_NOT_MLC, IDFLAG_ENDOFLIST



#endif  /* AMDCMD_H_INCLUDED */


