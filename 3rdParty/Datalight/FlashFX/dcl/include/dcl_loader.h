/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
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

    This header defines the function interface to the Datalight Common
    Libraries.

    This header is designed to be included by code that is built <outside>
    the Datalight standardized build process.

    To keep the requirements for accomplishing this as simple as possible,
    this header should only include other headers that exist in the same
    directory as this header file -- hence the use of "" for the included
    Datalight headers, rather than <>.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dcl_loader.h $
    Revision 1.5  2010/01/07 02:32:45Z  garyp
    Updated to include dltypes.h and dlinstance.h, which are now necessary
    due to refactoring.
    Revision 1.4  2009/10/09 01:19:08Z  garyp
    Moved headers common to all OS abstractions into dlapi.h.
    Revision 1.3  2009/03/05 17:58:38Z  thomd
    Updated to include dlprod.h and dliosys.h.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/10 02:47:38Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DCL_LOADER_H_INCLUDED
#define DCL_LOADER_H_INCLUDED


/*-------------------------------------------------------------------
    Include the basic necessities to use ANSI C
-------------------------------------------------------------------*/
#include <stddef.h>

#ifndef TRUE
  #define TRUE                      (1)
  #define FALSE                     (0)
#endif

/*-------------------------------------------------------------------
    Define product specific characteristics.  These are handled in
    #ifndef blocks for two reasons.  This could be a subproduct,
    in which case the master product has already defined these
    values.  Additionally, this code <could> be compiled by the
    standard Datalight build process, where the value is already
    defined.

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
  #ifdef _MSC_VER
    /*  If "_MSC_VER" is defined, then we know we are using MS Visual C tools
    */
    #define D_TOOLSETNUM TS_MSVC6
  #endif
#endif

#ifndef D_TOOLSETNUM
  #error "Unable to automatically determine the D_TOOLSETNUM value"
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


#define DclTest         DCLFUNC(DclTest)
#define DclTestFSIO     DCLFUNC(DclTestFSIO)

int DclTest(const char *pszCmdLine);
int DclTestFSIO(const char *pszCmdLine);



#endif  /* DCL_LOADER_H_INCLUDED */

