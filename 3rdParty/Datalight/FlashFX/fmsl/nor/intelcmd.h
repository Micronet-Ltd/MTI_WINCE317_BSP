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

    This header file contain macros for building flash commands for Intel
    flash memory.  It must be included after flashcmd.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: intelcmd.h $
    Revision 1.8  2009/12/03 20:57:54Z  garyp
    Moved manufacturer ID codes into nor.h.  Renamed ID_STMICRO to
    ID_NUMONYX.
    Revision 1.7  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2006/08/30 00:41:04Z  pauli
    Updated comments.
    Revision 1.5  2006/07/31 20:03:29Z  timothyj
    Added P30 family
    Revision 1.4  2006/05/21 18:23:04Z  Garyp
    Updated the chip params structure to be more flexible with regard to boot
    blocks.
    Revision 1.3  2006/05/13 00:00:09Z  Garyp
    Added boot block information for the "Advanced+" parts.
    Revision 1.2  2006/01/11 19:57:48Z  billr
    Merge Sibley support from v2.01.  Add ID entry for Intel 28F256J3.
    Revision 1.1  2005/10/02 01:33:36Z  Pauli
    Initial revision
    Revision 1.5  2005/08/26 01:42:05Z  Cheryl
    Add #ifndef around conflicting defines for program/status/buffer write
    commands.  Not all intel flash types use the same any longer.
    Revision 1.4  2005/08/05 21:39:23Z  tonyq
    Added support for 28F256J3 Intel Strataflash
    Revision 1.3  2005/05/13 02:36:03Z  garyp
    Further abstraction of the manufacturer names.
    Revision 1.2  2005/05/12 02:16:45Z  garyp
    Corrected bad K3 and L18 device ID values.  Added missing K3/K18 parts.
    Revision 1.1  2005/05/08 17:47:36Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef INTELCMD_H_INCLUDED
#define INTELCMD_H_INCLUDED


/*---------------------------------------------------------
    Single-Chip Command Codes and Status
---------------------------------------------------------*/
#define ICMD_ID                         (0x90)

#ifndef ICMD_PROG   /* not all flash use same value */
#define ICMD_PROG                       (0x40)
#endif

#define ICMD_CLEARSTAT                  (0x50)
#define ICMD_READSTAT                   (0x70)

#ifndef ICMD_WRITEBUFFER    /* not all flash use same value */
#define ICMD_WRITEBUFFER                (0xE8)
#endif

#define ICMD_READMODE                   (0xFF)
#define ICMD_ERASESTART                 (0x20)
#define ICMD_ERASESUSPEND               (0xB0)
#define ICMD_ERASERESUME                (0xD0)
#define ICMD_CONFIRM                    (0xD0)
#define ICMD_LOCKUNLOCK                 (0x60)
#define ICMD_LOCKCONFIRM                (0x01)

#define ISTAT_WRITEFAIL                 (0x10)
#define ISTAT_ERASEFAIL                 (0x20)
#define ISTAT_OK                        (0x80)
#define ISTAT_ERASESUSPENDED            (0xC0)
#ifndef ISTAT_MASK              /* not all flash use same value */
#define ISTAT_MASK                      (0xFE) /* All valid status bits. */
#endif


/*---------------------------------------------------------
    Multi-Chip Command Codes and Status
---------------------------------------------------------*/
#define INTLCMD_IDENTIFY                MAKEFLASHDUPCMD(ICMD_ID)
#define INTLCMD_PROGRAM                 MAKEFLASHDUPCMD(ICMD_PROG)
#define INTLCMD_CLEAR_STATUS            MAKEFLASHDUPCMD(ICMD_CLEARSTAT)
#define INTLCMD_READ_STATUS             MAKEFLASHDUPCMD(ICMD_READSTAT)
#define INTLCMD_BUFFERED_WRITE          MAKEFLASHDUPCMD(ICMD_WRITEBUFFER)
#define INTLCMD_READ_MODE               MAKEFLASHDUPCMD(ICMD_READMODE)
#define INTLCMD_ERASE_START             MAKEFLASHDUPCMD(ICMD_ERASESTART)
#define INTLCMD_ERASE_SUSPEND           MAKEFLASHDUPCMD(ICMD_ERASESUSPEND)
#define INTLCMD_ERASE_RESUME            MAKEFLASHDUPCMD(ICMD_ERASERESUME)
#define INTLCMD_CONFIRM                 MAKEFLASHDUPCMD(ICMD_CONFIRM)
#define INTLCMD_LOCKUNLOCK              MAKEFLASHDUPCMD(ICMD_LOCKUNLOCK)
#define INTLCMD_LOCKCONFIRM             MAKEFLASHDUPCMD(ICMD_LOCKCONFIRM)

#define INTLSTAT_WRITE_FAIL             MAKEFLASHDUPCMD(ISTAT_WRITEFAIL)
#define INTLSTAT_ERASE_FAIL             MAKEFLASHDUPCMD(ISTAT_ERASEFAIL)
#define INTLSTAT_DONE                   MAKEFLASHDUPCMD(ISTAT_OK)
#define INTLSTAT_ERASE_SUSPENDED        MAKEFLASHDUPCMD(ISTAT_ERASESUSPENDED)
#define INTLSTAT_STATUS_MASK            MAKEFLASHDUPCMD(ISTAT_MASK)


/*---------------------------------------------------------
    FIM ID structure and flags
---------------------------------------------------------*/
typedef struct INTELCHIPPARAMS_TAG
{
    FLASHID     idMfg;
    FLASHID     idDev;
    D_UINT32    ulChipSize;
    D_UINT16    uLowBootBlocks;     /* count of full blocks divided into boot blocks */
    D_UINT16    uHighBootBlocks;    /* count of full blocks divided into boot blocks */
    D_UINT16    uFlags;
} INTELCHIPPARAMS;


#define ID_BUFF64       0x80
#define ID_BUFF1024     0x400
#define ID_UNLOCK       0x08
 

/*  x8 Intel "FlashFile" parts, and clones
*/
#define ID_28F008SAL  ID_INTEL89,   0xA2, 0x00100000UL, 0, 0, 0x0 /*  8Mb    */
#define ID_28F004S5   ID_INTEL89,   0xA7, 0x00080000UL, 0, 0, 0x0 /*  4Mb 5V */
#define ID_28F008S5   ID_INTEL89,   0xA6, 0x00100000UL, 0, 0, 0x0 /*  8Mb 5V */
#define ID_28F016S5   ID_INTEL89,   0xAA, 0x00200000UL, 0, 0, 0x0 /* 16Mb 5V */

/*  x8/x16 Intel "FlashFile" parts, and clones
*/
#define ID_28F160S3   ID_INTELB0,   0xD0, 0x00200000UL, 0, 0, 0x0 /* 16Mb 3V */
#define ID_28F320S3   ID_INTELB0,   0xD4, 0x00400000UL, 0, 0, 0x0 /* 32Mb 3V */

/*  x8/x16-Bit Intel "StrataFlash" parts, and clones
*/
#define ID_28F320J5   ID_INTEL89,   0x14, 0x00400000UL, 0, 0, 0x0 /*  32Mb 5V */
#define ID_28F640J5   ID_INTEL89,   0x15, 0x00800000UL, 0, 0, 0x0 /*  64Mb 5V */
#define ID_28F320J3   ID_INTEL89,   0x16, 0x00400000UL, 0, 0, 0x0 /*  32Mb 3V */
#define ID_28F640J3   ID_INTEL89,   0x17, 0x00800000UL, 0, 0, 0x0 /*  64Mb 3V */
#define ID_28F128J3   ID_INTEL89,   0x18, 0x01000000UL, 0, 0, 0x0 /* 128Mb 3V */
#define ID_28F256J3   ID_INTEL89,   0x1D, 0x02000000UL, 0, 0, 0x0 /* 256Mb 3V */
#define ID_MX26F640   ID_MXIC,      0xAE, 0x00800000UL, 0, 0, 0x0 /*  64Mb 3V */
#define ID_MX26F128   ID_MXIC,      0x74, 0x01000000UL, 0, 0, 0x0 /* 128Mb 3V */
#define ID_M58LW064D  ID_NUMONYX,   0x17, 0x00800000UL, 0, 0, 0x0 /*  64Mb 3V */
#define ID_MT28F320   ID_MICRON,    0x16, 0x00400000UL, 0, 0, 0x0 /*  32Mb 3V */
#define ID_MT28F640   ID_MICRON,    0x17, 0x00800000UL, 0, 0, 0x0 /*  64Mb 3V */
#define ID_MT28F128   ID_MICRON,    0x18, 0x01000000UL, 0, 0, 0x0 /* 128Mb 3V */

/*  x16 Intel "StrataFlash" parts, and clones
*/
#define ID_28F640K3   ID_INTEL89, 0x8801, 0x00800000UL, 0, 0, ID_BUFF64|ID_UNLOCK /*  64Mb   3V */
#define ID_28F128K3   ID_INTEL89, 0x8802, 0x01000000UL, 0, 0, ID_BUFF64|ID_UNLOCK /* 128Mb   3V */
#define ID_28F256K3   ID_INTEL89, 0x8803, 0x02000000UL, 0, 0, ID_BUFF64|ID_UNLOCK /* 256Mb   3V */
#define ID_28F640K18  ID_INTEL89, 0x8805, 0x00800000UL, 0, 0, ID_BUFF64|ID_UNLOCK /*  64Mb 1.8V */
#define ID_28F128K18  ID_INTEL89, 0x8806, 0x01000000UL, 0, 0, ID_BUFF64|ID_UNLOCK /* 128Mb 1.8V */
#define ID_28F256K18  ID_INTEL89, 0x8807, 0x02000000UL, 0, 0, ID_BUFF64|ID_UNLOCK /* 256Mb 1.8V */
#define ID_28F640L18T ID_INTEL89, 0x880B, 0x00800000UL, 0, 1, ID_BUFF64|ID_UNLOCK /*  64Mb 1.8V */
#define ID_28F128L18T ID_INTEL89, 0x880C, 0x01000000UL, 0, 1, ID_BUFF64|ID_UNLOCK /* 128Mb 1.8V */
#define ID_28F256L18T ID_INTEL89, 0x880D, 0x02000000UL, 0, 1, ID_BUFF64|ID_UNLOCK /* 256Mb 1.8V */
#define ID_28F640L18B ID_INTEL89, 0x880E, 0x00800000UL, 1, 0, ID_BUFF64|ID_UNLOCK /*  64Mb 1.8V */
#define ID_28F128L18B ID_INTEL89, 0x880F, 0x01000000UL, 1, 0, ID_BUFF64|ID_UNLOCK /* 128Mb 1.8V */
#define ID_28F256L18B ID_INTEL89, 0x8810, 0x02000000UL, 1, 0, ID_BUFF64|ID_UNLOCK /* 256Mb 1.8V */
#define ID_28F640P30T ID_INTEL89, 0x8817, 0x00800000UL, 0, 1, ID_BUFF64|ID_UNLOCK /*  64Mb 1.8V */
#define ID_28F128P30T ID_INTEL89, 0x8818, 0x01000000UL, 0, 1, ID_BUFF64|ID_UNLOCK /* 128Mb 1.8V */
#define ID_28F256P30T ID_INTEL89, 0x8919, 0x02000000UL, 0, 1, ID_BUFF64|ID_UNLOCK /* 256Mb 1.8V */
#define ID_28F640P30B ID_INTEL89, 0x881A, 0x00800000UL, 1, 0, ID_BUFF64|ID_UNLOCK /*  64Mb 1.8V */
#define ID_28F128P30B ID_INTEL89, 0x881B, 0x01000000UL, 1, 0, ID_BUFF64|ID_UNLOCK /* 128Mb 1.8V */
#define ID_28F256P30B ID_INTEL89, 0x891C, 0x02000000UL, 1, 0, ID_BUFF64|ID_UNLOCK /* 256Mb 1.8V */

/*  x16 Intel "Advanced+" parts, and clones
*/
#define ID_28F800C3T  ID_INTEL89, 0x88C0, 0x00100000UL, 0, 1, 0x0 /*  8Mb */
#define ID_28F160C3T  ID_INTEL89, 0x88C2, 0x00200000UL, 0, 1, 0x0 /* 16Mb */
#define ID_28F320C3T  ID_INTEL89, 0x88C4, 0x00400000UL, 0, 1, 0x0 /* 32Mb */
#define ID_28F640C3T  ID_INTEL89, 0x88CC, 0x00800000UL, 0, 1, 0x0 /* 64Mb */
#define ID_28F800C3B  ID_INTEL89, 0x88C1, 0x00100000UL, 1, 0, 0x0 /*  8Mb */
#define ID_28F160C3B  ID_INTEL89, 0x88C3, 0x00200000UL, 1, 0, 0x0 /* 16Mb */
#define ID_28F320C3B  ID_INTEL89, 0x88C5, 0x00400000UL, 1, 0, 0x0 /* 32Mb */
#define ID_28F640C3B  ID_INTEL89, 0x88CD, 0x00800000UL, 1, 0, 0x0 /* 64Mb */

/*  What the heck is this part?
*/
#define ID_INTEL66A0  ID_INTEL89, 0x66A0, 0x00200000UL, 0, 0, 0x0  /*   16Mb */

#define ID_ENDOFLIST  0x0,        0x0,    D_UINT32_MAX, 0, 0, 0x0


#endif  /* INTELCMD_H_INCLUDED */


