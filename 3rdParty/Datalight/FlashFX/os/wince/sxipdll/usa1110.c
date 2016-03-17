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

/*  Platform dependent code for NMI µSA1110 µEngine.
*/

#include <flashfx.h>
#include <sxipplat.h>


/*  The Operating System Count Register (OSCR) is mapped by the BSP
    at A9000010.  This is a static section mapping, which is always in
    the page table and thus never causes a data abort exception.
    Using a constant rather than a variable for the address means that
    the value is loaded from the text section (using a PC-relative
    address), which also won't cause a data abort exception. 
*/

#define OSCR (*(volatile unsigned long *) 0xA9000010)




/*------------------------------------------------------------------------
    Parameters:
       microsec - the number of microseconds to delay.  The maximum
          allowed value is 71111.
    Notes:
       Delays for at least the specified number of microseconds before
       returning.
    Returns:
       Nothing.
------------------------------------------------------------------------*/

void SXIP_delay(
    unsigned long microsec)
{
    /*  OSCR counts at 3.6864 MHz, or 2304 counts per 625 microseconds.
        2304/625 is approximately 30199 >> 13 (so close that it makes
        no difference up to the point where the calculation overflows
        a 32-bit signed value).  Add 2 to the result of the calculation:
        1 for truncation, and another 1 because the starting time could
        be just before the first tick.  
    */

    unsigned long   stop = OSCR + ((30199 * microsec) >> 13) + 2;
    while((signed)(OSCR - stop) < 0)
    {
        /*  empty 
        */
    }
}

unsigned long SXIP_set_timer(
    unsigned long microsec)
{
    return OSCR + ((30199 * microsec) >> 13) + 2;
}

D_BOOL SXIP_timeout(
    unsigned long timer)
{
    return ((signed)(OSCR - timer) >= 0);
}
