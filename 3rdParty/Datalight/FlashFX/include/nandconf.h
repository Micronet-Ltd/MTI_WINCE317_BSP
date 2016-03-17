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

    NAND FIM OEM configuration.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandconf.h $
    Revision 1.4  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/10/06 00:20:33Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.2  2006/03/13 03:12:27Z  Garyp
    Eliminated an obsolete setting.
    Revision 1.1  2005/11/22 02:31:48Z  Pauli
    Initial revision
    Revision 1.3  2005/11/22 02:31:48Z  Garyp
    Added NANDCONF_LEGACYSPAREFORMAT.
    Revision 1.2  2005/10/30 04:21:36Z  Garyp
    Eliminated obsolete settings.
    Revision 1.1  2004/12/31 02:17:54Z  Garyp
    Initial revision
    Revision 1.3  2004/12/30 23:17:53Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2003/09/11 17:15:33Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.1  2003/08/28 16:52:08Z  billr
    Initial revision
---------------------------------------------------------------------------*/

/*  Define the time-out value in uSeconds.  Default is 20ms for both write
    and erase; that is twice the maximum specified time for any supported NAND
    chip to do anything we could sensibly ask of it...
*/
#define NAND_TIME_OUT         20000L

/*  Set this to TRUE if the NAND hardware expects data in little endian format,
    FALSE if hardware is big endian.  In almost all circumstances this is
    identical to the platform's byte order.
*/
#define NAND_LITTLE_ENDIAN    !DCL_BIG_ENDIAN

/*  NAND_DATA_WIDTH_BYTES is the width of the data/control/address path in bytes.
    Only 8 and 16 bit I/O are currently supported.
    NAND_DATA_WIDTH_POW2 is the exponent in the data width expressed as 2^n bytes.
    (byte=0, 16-bits= 1,...)

    The "pins" port is assumed to always be accessed in bytes.

*/
#define NAND_DATA_WIDTH_BYTES 1
#define NAND_DATA_WIDTH_POW2  0


