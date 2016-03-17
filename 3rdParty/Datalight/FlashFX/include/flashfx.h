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

    This is the master FlashFX header that is included by all FlashFX C code.

    Note that this file should typically be include after any ToolSet or OS
    specific includes such as STRING.H and WINDOWS.H respectively.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: flashfx.h $
    Revision 1.8  2009/10/06 21:39:11Z  garyp
    Eliminated references to the obsolete ffxapi.h.
    Revision 1.7  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/01/23 18:16:10Z  Garyp
    Re-added the error check removed in the previous rev.
    Revision 1.5  2007/01/20 01:09:05Z  pauli
    Removed a duplicate set of error checking code.
    Revision 1.4  2007/01/14 02:21:51Z  Garyp
    Removed fxapi.h.
    Revision 1.3  2007/01/12 23:00:56Z  Garyp
    Updated to include fxapi.h and cleaned up the documentation.
    Revision 1.2  2006/01/28 22:41:41Z  Garyp
    Updated to include fxparam.h.
    Revision 1.1  2005/10/09 21:32:16Z  Pauli
    Initial revision
    Revision 1.2  2005/08/21 10:41:58Z  garyp
    Updated to use ffxconf.h rather than oemconf.h.
    Revision 1.1  2005/08/03 19:40:28Z  pauli
    Initial revision
    Revision 1.2  2005/08/03 19:40:28Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.1  2005/07/05 08:59:02Z  pauli
    Initial revision
    Revision 1.15  2005/06/12 21:19:54Z  PaulI
    Removed definition of TRUE and FALSE.
    Revision 1.14  2005/02/14 01:39:25Z  GaryP
    Updated to include dlapi.h.
    Revision 1.13  2005/01/26 01:56:38Z  GaryP
    Updated to use the new DCL headers and functions.
    Revision 1.12  2004/12/30 23:08:29Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.11  2004/07/08 20:46:14Z  garys
    moved FFXOPTION enumeration from oeslapi.h to fxoption.h
    Revision 1.10  2004/01/16 19:12:54  garys
    Merge from FlashFXMT
    Revision 1.7.1.4  2004/01/16 19:12:54  garyp
    Added protections in case flashfx.h and ffxapi.h are both included.
    Revision 1.7.1.3  2003/11/14 21:12:20Z  garyp
    Changed to use FLASHFX_H_INCLUDED.
    Revision 1.7.1.2  2003/11/03 04:32:48Z  garyp
    Re-checked into variant sandbox.
    Revision 1.8  2003/11/03 04:32:48Z  garyp
    Updated to include FXSTATUS.H.
    Revision 1.7  2003/05/29 18:00:38Z  garyp
    Modified to always include STDDEF.H, except under CE3.
    Revision 1.6  2003/05/29 01:57:50Z  garyp
    Refrain from including STDDEF.H on CE3.
    Revision 1.5  2003/04/21 20:58:26Z  garyp
    Updated to include STDDEF.H if needed.
    Revision 1.4  2003/04/15 21:21:54Z  garyp
    Stop including CPUNUM.H and FXVER.H by default.
    Revision 1.3  2003/04/06 19:51:42Z  garyp
    Updated to include CPUNUM.H.
    Revision 1.2  2002/11/07 21:36:08Z  garyp
    Modified to define TRUE and FALSE if they are not already defined.
    09/26/02 de  Reordered NVBF and ALLOC_UNIT_OFFSET_CACHE cache declarations.
                 The former now turns on the latter by default.
    09/05/02 gjs Disabled NVBF and ALLOC_UNIT_OFFSET_CACHE by default
                 Non-extended devices (NOR) don't support NVBF
    07/30/02 PKG Added ALLOC_UNIT_OFFSET_CACHE option
    07/16/02 DE  Added NVBF option for NAND VBF.
    07/12/02 gp  Created from FLASHFX.H
---------------------------------------------------------------------------*/

#ifndef FLASHFX_H_INCLUDED
#define FLASHFX_H_INCLUDED


/*-------------------------------------------------------------------
    If both flashfx.h and fxapi.h are necessary (rare), flashfx.h
    must be first.
-------------------------------------------------------------------*/
#ifdef FXAPI_H_INCLUDED
#error "flashfx.h must be included before fxapi.h"
#endif


/*-------------------------------------------------------------------
    NOTE:  The product locale number must match the values found in
           the various prodinfo.bat files for each of the products
           (see dlprod.h for a further description)
-------------------------------------------------------------------*/
#ifndef D_PRODUCTLOCALE
#define D_PRODUCTLOCALE     (2)     /* FlashFX */
#endif

/*  Note that the order in which these items are included is critical
*/
#include <dcl.h>
#include <ffxconf.h>
#include <oesl.h>
#include <fxstatus.h>
#include <fxtypes.h>
#include <fxmacros.h>

/*  Standard prototypes to include everywhere
*/
#include <fxoption.h>
#include <fxparam.h>
#include <oeslapi.h>
#include <fxutil.h>
#include <fxtrace.h>


#endif /* FLASHFX_H_INCLUDED */
