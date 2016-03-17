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
  jurisdictions.  Patents may be pending.

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

    This header defines the Windows CE function interface to the Datalight
    Common Libraries.

    This header is designed to be included by code that is built <outside>
    the Datalight standardized build process.

    To keep the requirements for accomplishing this as simple as possible,
    this header should only include other headers that exist in the same
    directory as this header file -- hence the use of "" for the included
    Datalight headers, rather than <>.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dcl_wince.h $
    Revision 1.5  2011/04/22 19:57:55Z  garyp
    Updated to initialize D_PRODUCTNUM if necessary.
    Revision 1.4  2010/01/07 02:32:45Z  garyp
    Updated to include dltypes.h and dlinstance.h, which are now necessary
    due to refactoring.
    Revision 1.3  2009/10/09 01:27:17Z  garyp
    Moved headers common to all OS abstractions into dlapi.h.
    Revision 1.2  2009/02/08 00:58:13Z  garyp
    Merged from the v4.0 branch.  Updated to include dlprod.h and dliosys.h.
    Updated to use the MSWCE ToolSet rather than MSWCE4.
    Revision 1.1  2007/11/03 23:31:22Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DCL_WINCE_H_INCLUDED
#define DCL_WINCE_H_INCLUDED


/*-------------------------------------------------------------------
    Include the basic necessities to use ANSI C
-------------------------------------------------------------------*/
#include <stddef.h>

#ifndef TRUE
  #define TRUE                      (1)
  #define FALSE                     (0)
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
  #define D_PRODUCTNUM          (PRODUCTNUM_DCL)
#endif


/*-------------------------------------------------------------------
    Externally compiled code which calls Datalight functions should
    only require the D_TOOLSETNUM symbol.  Verify that we have the
    D_TOOLSETNUM value, or try to figure it out if we have enough
    information to do so.
-------------------------------------------------------------------*/
#ifndef D_TOOLSETNUM
  /*  For CE, we are OK using the TS_MSWCE value below regardless
      whether we are using PB or the command-line tools.
  */
  #define D_TOOLSETNUM TS_MSWCE
#endif


/*-------------------------------------------------------------------
    Include the correct ToolSet header.
-------------------------------------------------------------------*/
#include "dltlset.h"
#include "dltlinit.h"
#include "dlprod.h"


/*-------------------------------------------------------------------
    Only define DCLFUNC() if it is not already defined at a higher
    level.
-------------------------------------------------------------------*/
#ifndef DCLFUNC
  #define DCLFUNC(fn) (fn)
#endif


/*-------------------------------------------------------------------
    Note the use of quotes rather than angle brackets.  Because this
    code may be built outside the Datalight build process, this
    header may only include Datalight headers which can be located
    relative to the current directory in which this header resides.
-------------------------------------------------------------------*/
#include "dltypes.h"
#include "dlapi.h"
#include "dlinstance.h"



#endif  /* DCL_WINCE_H_INCLUDED */


