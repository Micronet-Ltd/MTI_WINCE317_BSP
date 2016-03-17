/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This header contains definitions for Intel 3 Volt StrataFlash Memory 
    28F128J3A, 28F640J3A, 28F320J3A.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                           Revision History
    $Log: ij3a.h $
    Revision 1.2  2007/11/03 23:50:12Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2004/12/31 02:17:56Z  Pauli
    Initial revision
    Revision 1.1  2004/12/31 02:17:56Z  Garyp
    Initial revision
    Revision 1.1  2004/12/31 01:17:56Z  pauli
    Initial revision
    Revision 1.1  2004/12/31 00:17:56Z  pauli
    Initial revision
    Revision 1.2  2004/12/30 23:17:56Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2002/11/14 00:22:14Z  qa
    Initial revision
---------------------------------------------------------------------------*/


/*  Status Register bits. 
*/

#define SR_WSMS  (1 << 7)       /* SR7: Write State Machine Status */
#define SR_DONE  SR_WSMS        /* SR7: (a more sensible name for it) */
#define SR_ESS   (1 << 6)       /* SR6: Erase Suspend Status */
#define SR_ECLBS (1 << 5)       /* SR5: Erase and Clear Lock-Bits Status */
#define SR_PSLBS (1 << 4)       /* SR4: Program and Set Lock-Bits Status */
#define SR_VPENS (1 << 3)       /* SR3: Programming Voltage Status */
#define SR_PSS   (1 << 2)       /* SR2: Program Suspend Status */
#define SR_DPS   (1 << 1)       /* SR1: Device Protect Status */
#define SR_VALID (0xFE)         /* mask for all valid status bits SR[7:1] */

/*  Extended Status Register bits. 
*/

#define XSR_WBS  (1 << 7)       /* XSR7: Write Buffer Status */

/*  Chip commands as defined by Intel.  Note that these have to be present
    in the correct bit positions for both chips.  
*/

#define READ_ARRAY (0xFF)
#define READ_ID    (0x90)
#define READ_STATUS (0x70)
#define CLEAR_STATUS (0x50)
#define BLOCK_ERASE  (0x20)
#define BLOCK_ERASE_CONFIRM (0xD0)
#define BLOCK_ERASE_RESUME  (0xD0)
#define BLOCK_ERASE_SUSPEND (0xB0)
#define WRITE_BUFFER (0xE8)
#define WRITE_BUFFER_CONFIRM (0xD0)
#define CLEAR_LOCK_BITS (0x60)
#define CLEAR_LOCK_BITS_CONFIRM (0xD0)
#define LOCK_BLOCK (0x60)
#define LOCK_BLOCK_CONFIRM (0x01)

#define BUFFER_BYTES (32)       /* size of write buffer, per chip */

/*  Timing parameters 
*/

/*  According to Intel's data sheet (290667-011, April 2002), the STS
    pin is not valid until 500ns after the command is latched.
    Although the data sheet does not say so, experience suggests that
    SR7, Write State Machine Status (which indicates the same thing as
    STS), has similar timing.  Every time a command is issued, there
    must be a delay before status is polled to allow the WSM to
    indicate it is busy.  
*/

#define WSM_DELAY (1)           /* _sysdelay() argument, microseconds. */

/*  Maximum times, in microseconds.
    From Specification Update 298130-015, April 2002 
*/

#define tWRITE_BUFFER_BYTE (654)
#define tWRITE_BYTE        (630)
#define tBLOCK_ERASE       (5000000)    /* 5 seconds */
#define tCLEAR_LOCK_BITS   (1400000)    /* 1.4 seconds (-40 C.) */
#define tPROGRAM_SUSPEND   (90)
#define tERASE_SUSPEND     (40)

/*  Capacity in bytes of single chips, per type 
*/

#define CHIP_SIZE_28F128  0x1000000L    /* 16Mb */
#define CHIP_SIZE_28F640   0x800000L    /* 8Mb */
#define CHIP_SIZE_28F320   0x400000L    /* 4Mb */

/*  Erase block size in bytes of a single chip (all types) 
*/

#define ERASE_BLOCK_SIZE (128 * 1024L)  /* 128K */
