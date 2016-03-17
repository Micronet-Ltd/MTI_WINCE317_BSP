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

    This header defines the Windows CE function interface to FlashFX.

    This header is designed to be included by code that is built <outside>
    the Datalight standardized build process.

    To keep the requirements for accomplishing this as simple as possible,
    this header should only include other headers that exist in the same
    directory as this header file -- hence the use of "" for the included
    Datalight headers, rather than <>.

    Using this header MAY require the D_TOOLSETNUM value to be defined prior
    to including this header.  If the dcl_wince.h header does NOT have
    enough information to derive the D_TOOLSETNUM value, define it prior to
    including this header, using the recognized values defined in dltlset.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: flashfx_wince.h $
    Revision 1.3  2009/08/07 23:20:29Z  garyp
    Removed the DCLFUNC definition and fxstatus.h include since they are
    handled at lower levels.  Fixed the dcl_osname.h inclusion to work properly
    in a 4GR build tree, when code is being built outside the DL build process.
    Revision 1.2  2009/04/02 21:19:51Z  thomd
    Updated to include fxstatus.h
    Revision 1.1  2008/07/12 20:14:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FLASHFX_WINCE_H_INCLUDED
#define FLASHFX_WINCE_H_INCLUDED


#ifndef __WINDOWS__
  #error "windows.h must be included before flashfx_wince.h"
#endif


/*-------------------------------------------------------------------
    If both flashfx_wince.h and fxapi.h are necessary, the former
    must be first.
-------------------------------------------------------------------*/
#if !defined(DL4GR_H_INCLUDED) && !defined(FLASHFX_H_INCLUDED) && defined(FXAPI_H_INCLUDED)
  #error "flashfx_wince.h must be included before fxapi.h"
#endif


/*-------------------------------------------------------------------
    Define product specific characteristics.  This is handled in an
    #ifndef block for two reasons.  This could be a subproduct, in
    which case the master product has already defined this value.
    Additionally, this code <could> be compiled by the standard
    Datalight build process, where the value is already defined.

    Note that the PRODUCTNUM_* values are defined <later> when
    dlprod.h is included.
-------------------------------------------------------------------*/
#ifndef D_PRODUCTNUM
  #define D_PRODUCTNUM          (PRODUCTNUM_FLASHFX)
#endif


/*-------------------------------------------------------------------
    If neither dcl.h nor dcl_wince.h have already been included
    by higher level headers, we <may> be here because we're being
    included by externally built software which will not have
    the DCL include path.  Therefore use a path relative to the
    current directory.
-------------------------------------------------------------------*/
#ifndef DCL_H_INCLUDED
  #ifndef DCL_WINCE_H_INCLUDED
    #if defined(D_PRODUCTNUM) && defined(D_SUBPRODUCTNUM)
      /*  In the event that FlashFX is a sub-product, and we're being
          built by our standard build process, the DCL include directory
          is one level deeper.
      */
      #include "../../dcl/include/dcl_wince.h"
    #else
      /*  Not a subproduct, or not being built by the Datalight build
          process, so the DCL include directory should be located in
          the FlashFX tree.
      */
      #include "../dcl/include/dcl_wince.h"
    #endif
  #endif
#endif


/*-------------------------------------------------------------------
    Include FlashFX headers which provide access to the portions of
    the publicly exposed FlashFX API which are not OS-specific.

    Note the use of quotes rather than angle brackets.  Because this
    code may be built outside the Datalight build process, this
    header may only include Datalight headers which can be located
    relative to the current directory in which this header resides.
-------------------------------------------------------------------*/
#include "fxapi.h"


/*-------------------------------------------------------------------
    Initialization routines

    These are called automatically during the Wince initialization
    as configured in the component settings of the Wince project.

    These can also be called manually by an application if discrete
    control of device initialization is needed or desired.
-------------------------------------------------------------------*/



#endif /* FLASHFX_WINCE_H_INCLUDED */

