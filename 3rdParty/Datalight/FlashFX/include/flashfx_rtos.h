/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

  U.S. Government Restricted Rights:  Use, duplication, reproduction, or
  transfer of this commercial product and accompanying documentation is
  restricted in accordance with FAR 12.212 and DFARS 227.7202 and by a
  License Agreement.

  IN ADDITION TO COPYRIGHT AND PATENT LAW, THIS SOFTWARE IS PROTECTED UNDER
  CONTRACT(S) BETWEEN DATALIGHT, INC. AND THE LICENSEE ("BINDING AGREEMENTS").
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

    This header defines the RTOS function interface to FlashFX.

    This header is designed to be included by code that is built <outside>
    the Datalight standardized build process.

    To keep the requirements for accomplishing this as simple as possible,
    this header should only include other headers that exist in the same
    directory as this header file -- hence the use of "" for the included
    Datalight headers, rather than <>.

    Using this header MAY require the D_TOOLSETNUM value to be defined
    prior to including this header.  If the dcl_rtos.h header does NOT have
    enough information to derive the D_TOOLSETNUM value, define it prior to
    including this header, using the recognized values defined in dltlset.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: flashfx_rtos.h $
    Revision 1.11  2011/02/10 02:17:48Z  garyp
    Eliminated the use of unsigned long and short data types.  Now use only
    our standard abstracted types, as well as plain unsigned, to avoid
    compatibility issues with some toolchains and platforms.
    Revision 1.10  2009/08/07 23:21:15Z  garyp
    Removed the DCLFUNC definition and fxstatus.h include since they are handled
    at lower levels.  Fixed the dcl_osname.h inclusion to work properly in a 4GR
    build tree, when code is being built outside the DL build process.
    Revision 1.9  2009/04/02 15:56:44Z  thomd
    Updated to include fxstatus.h
    Revision 1.8  2009/02/09 02:28:30Z  garyp
    Merged from the v4.0 branch.  Updated to initialize D_PRODUCTNUM
    if it is not already initialized.
    Revision 1.7  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2007/02/10 21:33:31Z  Garyp
    Updated preprocessor checks to work properly in a 4GR environment.
    Revision 1.5  2007/01/12 23:06:54Z  Garyp
    Updated to include fxapi.h and cleaned up the documentation.
    Revision 1.4  2007/01/11 21:24:12Z  Garyp
    Updated to include the appropriate DCL headers, as well as fxstatus.h
    Revision 1.3  2006/08/21 21:58:00Z  Garyp
    Renamed FlashFXDiskInfo to just FlashFXInfo.
    Revision 1.2  2006/05/06 00:32:26Z  Pauli
    Updated prototype list with all tests and tools.
    Revision 1.1  2006/01/27 00:25:16Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FLASHFX_RTOS_H_INCLUDED
#define FLASHFX_RTOS_H_INCLUDED


/*-------------------------------------------------------------------
    If both flashfx_rtos.h and fxapi.h are necessary, the former
    must be first.
-------------------------------------------------------------------*/
#if !defined(DL4GR_H_INCLUDED) && !defined(FLASHFX_H_INCLUDED) && defined(FXAPI_H_INCLUDED)
#error "flashfx_rtos.h must be included before fxapi.h"
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
    If neither dcl.h nor dcl_rtos.h have already been included
    by higher level headers, we <may> be here because we're being
    included by externally built software which will not have
    the DCL include path.  Therefore use a path relative to the
    current directory.
-------------------------------------------------------------------*/
#ifndef DCL_H_INCLUDED
  #ifndef DCL_RTOS_H_INCLUDED
    #if defined(D_PRODUCTNUM) && defined(D_SUBPRODUCTNUM)
      /*  In the event that FlashFX is a sub-product, and we're being
          built by our standard build process, the DCL include directory
          is one level deeper.
      */
      #include "../../dcl/include/dcl_rtos.h"
    #else
      /*  Not a subproduct, or not being built by the Datalight build
          process, so the DCL include directory should be located in
          the FlashFX tree.
      */
      #include "../dcl/include/dcl_rtos.h"
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


/*  Driver Framework
*/
FFXSTATUS   FlashFXDeviceOpen( unsigned nDiskNum);
FFXSTATUS   FlashFXDeviceClose(unsigned nDiskNum);
FFXSTATUS   FlashFXDeviceIO(   unsigned nDiskNum, D_UINT32 ulSectorNum, void *pClientBuffer, D_UINT16 uSectorCount, D_BOOL fReading);
FFXSTATUS   FlashFXDeviceIoctl(unsigned nDiskNum, D_UINT16 uCommand, void *pBuffer);

/*  Tools
*/
int         FlashFXCheck(const char *pszCmdLine);
int         FlashFXInfo(const char *pszCmdLine);
int         FlashFXDump(const char *pszCmdLine);
int         FlashFXImage(const char *pszCmdLine);
int         FlashFXReclaim(const char *pszCmdLine);
int         FlashFXRemount(const char *pszCmdLine);

/*  Tests
*/
int         FlashFXStressMT(const char *pszCmdLine);
int         FlashFXTestFMSL(const char *pszCmdLine);
int         FlashFXTestVBF(const char *pszCmdLine);


#endif /* FLASHFX_RTOS_H_INCLUDED */

