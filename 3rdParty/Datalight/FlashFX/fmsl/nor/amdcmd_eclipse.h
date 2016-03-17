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

    This header file contain macros for building flash commands for
    Spansion Eclipse flash memory.  It must be included after flashcmd.h.
    
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: amdcmd_eclipse.h $
    Revision 1.5  2009/02/13 00:47:54Z  billr
    Update copyright date.
    Revision 1.4  2009/02/13 00:45:16Z  billr
    Fix warning about missing braces in initializer when compiling
    eclipsex16.c.
    Revision 1.3  2008/06/03 22:36:54Z  Glenns
    Changed Eclipse device parameters to use Spansion manufacturer
    ID.
    Revision 1.2  2008/05/01 22:02:40Z  Glenns
    - Fixed bug id device ID's
    - Added support for Engineering Sample devices
    Revision 1.1  2008/02/20 23:23:00Z  Glenns
    Initial revision
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

#ifndef AMDCMD_ECLIPSE_H_INCLUDED
#define AMDCMD_ECLIPSE_H_INCLUDED

/*---------------------------------------------------------
    Single-Chip Command Codes and Status commands
    that differ from most AMD/Spansion flash
---------------------------------------------------------*/
#define ECLIPSE_DEVRDY          (0x0080)
#define ECLIPSE_ERASE_SUSPEND   (0x0040)
#define ECLIPSE_ERASE_STATUS    (0x0020)
#define ECLIPSE_PROGRAM_STATUS  (0x0010)
#define ECLIPSE_RESERVED        (0x0080)
#define ECLIPSE_PROGRAM_SUSPEND (0x0004)
#define ECLIPSE_SECTLOCK_STATUS (0x0002)
#define ECLIPSE_BANKLOCK_STATUS (0x0001)

#define ECMD_CLEAR_STATUS       (0x71)
#define ECMD_READ_STATUS        (0x70)
#define ECMD_BITFIELD_LOAD      (0xBF)
#define ESTAT_MASK              (0xFF)

/*---------------------------------------------------------
    Multi-Chip Command Codes and Status commands
    that differ from most AMD/Spansion flash
---------------------------------------------------------*/
#define ECLIPSESTAT_DEVRDY              MAKEFLASHDUPCMD(ECLIPSE_DEVRDY)          
#define ECLIPSESTAT_ERASE_SUSPEND       MAKEFLASHDUPCMD(ECLIPSE_ERASE_SUSPEND)   
#define ECLIPSESTAT_ERASE_STATUS        MAKEFLASHDUPCMD(ECLIPSE_ERASE_STATUS)    
#define ECLIPSESTAT_PROGRAM_STATUS      MAKEFLASHDUPCMD(ECLIPSE_PROGRAM_STATUS)  
#define ECLIPSESTAT_RESERVED            MAKEFLASHDUPCMD(ECLIPSE_RESERVED)        
#define ECLIPSESTAT_PROGRAM_SUSPEND     MAKEFLASHDUPCMD(ECLIPSE_PROGRAM_SUSPEND) 
#define ECLIPSESTAT_SECTLOCK_STATUS     MAKEFLASHDUPCMD(ECLIPSE_SECTLOCK_STATUS) 
#define ECLIPSESTAT_BANKLOCK_STATUS     MAKEFLASHDUPCMD(ECLIPSE_BANKLOCK_STATUS)

#define ECLIPSECMD_CLEAR_STATUS         MAKEFLASHDUPCMD(ECMD_CLEAR_STATUS)
#define ECLIPSECMD_READ_STATUS          MAKEFLASHDUPCMD(ECMD_READ_STATUS)
#define ECLIPSECMD_BITFIELD_LOAD        MAKEFLASHDUPCMD(ECMD_BITFIELD_LOAD)
#define ECLIPSESTAT_MASK                MAKEFLASHDUPCMD(ESTAT_MASK)

typedef struct _EclipseChipID
{
    AMDCHIPPARAMS chipParams;
    FLASHID DevID2;
    FLASHID DevID3;
} ECLIPSECHIPID;

/* Macros for chip size, erase zone size, etc */

#define SIZE_32KB 0x00008000UL
#define SIZE_64KB 0x00010000UL
#define SIZE_128KB 0x00020000UL
#define SIZE_256KB 0x00040000UL
#define SIZE_64MB 0x00800000UL
#define SIZE_128MB 0x08000000UL
#define SIZE_256MB 0x10000000UL
#define SIZE_512MB 0x20000000UL
#define SIZE_1GB   0x40000000UL

/* Eclipse chips - Spansion */

#define S29XS512S_AMDPARAMS {ID_SPANSION, 0x007E, SIZE_64MB, SIZE_128KB, 0, 0, 0, 0, 0}
#define S29XS01GS_AMDPARAMS {ID_SPANSION, 0x007E, SIZE_128MB, SIZE_128KB, 0, 0, 0, 0, 0}
#define S29XS02GS_AMDPARAMS {ID_SPANSION, 0x007E, SIZE_256MB, SIZE_128KB, 0, 0, 0, 0, 0}

/* 
 * If you want to use the compiler to establish boot-block info
 * instead of CFI...
 */

#define S29XS512S_AMDPARAMS_HI {ID_SPANSION, 0x007E, SIZE_64MB, SIZE_128KB, SIZE_32KB, 0, 4, 0, 0}
#define S29XS01GS_AMDPARAMS_HI {ID_SPANSION, 0x007E, SIZE_128MB, SIZE_128KB, SIZE_32KB, 0, 4, 0, 0}
#define S29XS02GS_AMDPARAMS_HI {ID_SPANSION, 0x007E, SIZE_256MB, SIZE_128KB, SIZE_32KB, 0, 4, 0, 0}

#define S29XS512S_AMDPARAMS_LOW {ID_SPANSION, 0x007E, SIZE_64MB, SIZE_128KB, SIZE_32KB, 4, 0, 0, 0}
#define S29XS01GS_AMDPARAMS_LOW {ID_SPANSION, 0x007E, SIZE_128MB, SIZE_128KB, SIZE_32KB, 4, 0, 0, 0}
#define S29XS02GS_AMDPARAMS_LOW {ID_SPANSION, 0x007E, SIZE_256MB, SIZE_128KB, SIZE_32KB, 4, 0, 0, 0}

#define S29XS512S_AMDPARAMS_BOTH {ID_SPANSION, 0x007E, SIZE_64MB, SIZE_128KB, SIZE_32KB, 4, 4, 0, 0}
#define S29XS01GS_AMDPARAMS_BOTH {ID_SPANSION, 0x007E, SIZE_128MB, SIZE_128KB, SIZE_32KB, 4, 4, 0, 0}
#define S29XS02GS_AMDPARAMS_BOTH {ID_SPANSION, 0x007E, SIZE_256MB, SIZE_128KB, SIZE_32KB, 4, 4, 0, 0}

#define ID_S29XS512S S29XS512S_AMDPARAMS, 0x0057, 0x0001 /* 64MB  */
#define ID_S29XS01GS S29XS01GS_AMDPARAMS, 0x0052, 0x0001 /* 128MB */
#define ID_S29XS02GS S29XS02GS_AMDPARAMS, 0x004F, 0x0001 /* 256MB */

/* Eclipse prototype chip, used for development work: */

#define ID_S29XS02GS_PROTOTYPE S29XS01GS_AMDPARAMS, 0x3715, 0x3704
#define ID_S29XS02GS_PROTOTYPE2 S29XS01GS_AMDPARAMS, 0x004f, 0x0001

#define ID_ENDOFECLIPSELIST { ID_ENDOFLIST }

#endif  /* AMDCMD_ECLIPSE_H_INCLUDED */


