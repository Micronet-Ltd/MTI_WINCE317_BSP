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
  jurisdictions.  Patents may be pending.

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

    This module contains types and macros for interfacing with the Marvell
    PXA310 platform.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: hwpxa310.h $
    Revision 1.2  2009/12/03 02:54:18Z  garyp
    Corrected to use quoted includes rather than angle brackets.
    Revision 1.1  2008/12/11 19:23:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef HWPXA310_H_INCLUDED
#define HWPXA310_H_INCLUDED


/*  Include definitions which are 310/320 independent.
*/
#include "hwpxa3xx.h"


/*---------------------------------------------------------
    Timers
---------------------------------------------------------*/
#define PXA310_TICK_DIVISOR         (????)


/*---------------------------------------------------------
    Timings are in terms of cycles of a 156 MHz clock,
    which is 39 cycles per 250 nanoseconds.  The value
    in the register field is one less than the desired
    clock count.  All times must be non-zero; maximum
    permitted values are noted below.
---------------------------------------------------------*/
#define PXA310_NSEC(nsec) (((((nsec) * 39) + 249) / 250) - 1)


/*---------------------------------------------------------
    NAND Interface Timing Parameter 0 Register (NDTR0CS0)
    physical address 0x43100004.

    Maximum permitted value of all times is 76 nsec.

    Note that there's an extended bit for tRP that is not
    implemented here because it's 1) inconvenient; and 2)
    not presently needed.
---------------------------------------------------------*/
#define PXA310_tCH_CS_WH_WP_RH_RP(tCH, tCS, tWH, tWP, tRH, tRP)     \
    ( (PXA310_NSEC(tCH) << 19)                                      \
    | (PXA310_NSEC(tCS) << 16)                                      \
    | (PXA310_NSEC(tWH) << 11)                                      \
    | (PXA310_NSEC(tWP) << 8)                                       \
    | (PXA310_NSEC(tRH) << 3)                                       \
    |  PXA310_NSEC(tRP) )


/*---------------------------------------------------------
    NAND Interface Timing Parameter 1 Register (NDTR1CS0)
    physical address 0x4310000C.

    Maximum permitted value of tR is 630163 nsec.  Maximum
    permitted value of tWHR and tAR is 153 nsec.  Minimum
    permitted value of all fields is 10 nsec.
---------------------------------------------------------*/
#define PXA310_tR_WHR_AR(tR, tWHR, tAR)                             \
    ( (PXA310_NSEC(tR) << 16)                                       \
    | (PXA310_NSEC(tWHR) << 4)                                      \
    |  PXA310_NSEC(tAR) )


#endif  /* HWPXA310_H_INCLUDED */


