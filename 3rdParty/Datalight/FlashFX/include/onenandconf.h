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

    OneNAND NTM OEM configuration.

    ToDo:  This module should be eliminated, and any necessary project
           configuration settings specified in the project's fh1nand.c
           module, and communicated back to the OneNAND NTM at run-time.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: onenandconf.h $
    Revision 1.8  2009/08/04 03:15:23Z  garyp
    Merged from the v4.0 branch.  Eliminated configuration parameters which
    are now specified in the hook function.  Moved ONENAND_TIME_OUT to
    onenandctl.h.
    Revision 1.7  2009/03/06 23:13:41Z  glenns
    Fix Bugzilla #2471: Added a macro to indicate whether FlashFX should 
    manage the SLC or MLC partition of a FlexOneNAND device.
    Revision 1.6  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2006/10/06 00:21:04Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.4  2006/05/02 17:56:07Z  billr
    DCL timers are milliseconds, not microseconds.
    Revision 1.3  2006/03/09 21:18:55Z  timothyj
    Configured default OneNAND interface for asynchronous mode.
    Revision 1.2  2006/03/07 20:40:53Z  timothyj
    Added definitions for OneNAND-specific settings.
    Revision 1.1  2006/02/03 20:59:36Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

/*  Set this to TRUE if the OneNAND hardware expects data in little
    endian format, FALSE if hardware is big endian.  In almost all
    circumstances this is identical to the platform's byte order.
*/
#define ONENAND_LITTLE_ENDIAN    !DCL_BIG_ENDIAN

/*  ONENAND_DATA_WIDTH_BYTES is the width of the data/control/address
    path in bytes.  Only 8 and 16 bit I/O are currently supported.
*/
#define ONENAND_DATA_WIDTH_BYTES 2									 


/*-------------------------------------------------------------------
    For FlexOneNAND devices, this macro tells the NTM which partition 
    to manage.  It has no effect for standard OneNAND devices.

    Refer to FlexOneNAND datasheets for details.
-------------------------------------------------------------------*/
#define FLEX1NAND_MANAGE_MLC TRUE



