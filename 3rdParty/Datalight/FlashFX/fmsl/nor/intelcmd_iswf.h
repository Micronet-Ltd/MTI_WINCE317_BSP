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

    This header file contain macros for building flash commands for ISW Intel
    flash memory.  It must be included after flashcmd.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: intelcmd_iswf.h $
    Revision 1.6  2009/12/03 20:57:54Z  garyp
    Moved manufacturer ID codes into nor.h.  Renamed ID_STMICRO to
    ID_NUMONYX.
    Revision 1.5  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.4  2006/08/30 00:41:14Z  pauli
    Updated comments.
    Revision 1.3  2006/05/20 21:59:32Z  Garyp
    Updated the chip params structure to be more flexible with regard to boot
    blocks.
    Revision 1.2  2006/01/26 19:12:54Z  Rickc
    Added ST 58PR512J
    Revision 1.1  2006/01/11 19:41:50Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef INTELCMD_ISWF_H_INCLUDED
#define INTELCMD_ISWF_H_INCLUDED


/*---------------------------------------------------------
    Single-Chip Command Codes and Status

    commands that differ from most intel flash
---------------------------------------------------------*/
#define ICMD_PROG                       (0x41)
#define ICMD_WRITEBUFFER                (0xE9)
#define ISTAT_MASK                      (0x02FE) /* All valid status bits. */
/* error - attempted write with Object data to Control Mode region */
/* bit 9 set, bit 8 unset and bit 4 (write fail) set*/
#define ISTAT_OBJ_TO_CTL                (0x0210)
/* error - attempted rewrite to object region */
/* bit 9 unset, bit 8 set and bit 4 (write fail)  set */
#define ISTAT_REWRITE_OBJ               (0x0110)
/* error - write using illegal command */
/* bit 9 set, bit 8 set, and bit 4 (write fail)  set */
#define ISTAT_ILLEGAL_CMD               (0x0310)

/*---------------------------------------------------------
    Multi-Chip Command Codes and Status

    commands that differ from most intel flash
---------------------------------------------------------*/
/* error - attempted write with Object data to Control Mode region */
#define INTLSTAT_OBJ_TO_CTL             MAKEFLASHDUPCMD(ISTAT_OBJ_TO_CTL)
/* error - attempted rewrite to object region */
#define INTLSTAT_REWRITE_OBJ             MAKEFLASHDUPCMD(ISTAT_REWRITE_OBJ)
/* error - write using illegal command */
#define INTLSTAT_ILLEGAL_CMD             MAKEFLASHDUPCMD(ISTAT_ILLEGAL_CMD)

/*------------------------------------------------------------
    Sibley chips - Intel  
-------------------------------------------------------------*/
#define ID_48F128M64   ID_INTEL89, 0x8888, 0x01000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   128Mb 64Mhz*/
#define ID_48F128M108  ID_INTEL89, 0x8886, 0x01000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   128Mb 108Mhz*/
#define ID_48F256M64   ID_INTEL89, 0x8889, 0x02000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   256Mb 64Mhz*/
#define ID_48F256M108  ID_INTEL89, 0x887D, 0x02000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   256Mb 108Mhz*/
#define ID_48F512M64   ID_INTEL89, 0x888A, 0x04000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   512Mb 64Mhz*/
#define ID_48F512M108  ID_INTEL89, 0x887E, 0x04000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   512Mb 108Mhz*/

/*------------------------------------------------------------
    Sibley compatible chips - Numonyx
-------------------------------------------------------------*/
#define ID_58PR512J    ID_NUMONYX, 0x8819, 0x04000000UL, 0, 0, ID_BUFF1024|ID_UNLOCK  /*   512Mb   */



#endif  /* INTELCMD_ISWF_H_INCLUDED */


