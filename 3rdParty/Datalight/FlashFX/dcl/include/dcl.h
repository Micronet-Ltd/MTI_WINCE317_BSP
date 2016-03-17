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

    This header defines the interface to the Datalight Common Libraries.

    This is the master header for DCL and must be included in DCL specific
    code before any other Datalight headers are included.

    For other Datalight code bases, this header must the first thing
    included by the master header for that code base.

    For external code, or code which is NOT being built by the standard
    Datalight build processes, yet still needs to use DCL functionality,
    this header is NOT used.  Rather the OS specific header is used
    (dcl_vxworks.h and dcl_nucleus.h for example).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dcl.h $
    Revision 1.13  2010/01/13 19:23:00Z  billr
    Update copyright date.
    Revision 1.12  2010/01/12 00:20:33Z  billr
    Remove use of dcldefs.h.
    Revision 1.11  2009/12/31 17:24:18Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.10  2009/10/09 02:11:11Z  garyp
    Added a check to ensure that dcl.h is included ahead of dlapi.h.
    Revision 1.9  2009/06/28 01:47:15Z  garyp
    Added DCL_MAX_INSTANCES.
    Revision 1.8  2009/02/08 02:43:13Z  garyp
    Merged from the v4.0 branch.  Updated to include dliosys.h automatically.
    Revision 1.7  2007/11/02 03:02:05Z  Garyp
    Updated to always include limits.h.
    Revision 1.6  2007/03/06 03:46:50Z  Garyp
    Removed a temporary setting.
    Revision 1.5  2006/12/08 02:36:11Z  Garyp
    Eliminated dlcpunum.h.
    Revision 1.4  2006/10/06 00:05:01Z  Garyp
    Added a temporary default setting for DCLCONF_OUTPUT_ENABLED.
    Revision 1.3  2006/08/15 19:52:35Z  Garyp
    Minor refactoring -- updated to include dlcpunum.h and dlenv.h.
    Revision 1.2  2006/01/02 12:17:38Z  Garyp
    Refactored dlapi.h and moved internal DCL functions (those called only
    by other Datalight code) into dlapiint.h.
    Revision 1.1  2005/12/05 17:45:34Z  Pauli
    Initial revision
    Revision 1.2  2005/12/05 17:45:34Z  Pauli
    Merge with 2.0 product line, build 173.
    Revision 1.1  2005/10/03 02:13:30Z  Garyp
    Initial revision
    Revision 1.12  2005/09/29 03:43:21Z  Garyp
    Cleaned up data type and prototype interactions.
    Revision 1.11  2005/09/21 07:18:58Z  garyp
    Added prototypes.
    Revision 1.10  2005/09/20 19:09:40Z  pauli
    Added DclOsSleep.
    Revision 1.9  2005/09/15 00:12:57Z  garyp
    Added DclMemStatistics().
    Revision 1.8  2005/08/25 07:09:52Z  Garyp
    Minor cleanup -- no functional changes.
    Revision 1.7  2005/08/17 09:22:47Z  garyp
    Fixed a duplicated declaration.
    Revision 1.6  2005/08/15 06:54:02Z  garyp
    Added DclProductVersion().
    Revision 1.5  2005/08/04 17:05:06Z  Garyp
    Moved prototypes for the redefinable functions into dlapimap.h.
    Revision 1.4  2005/08/03 18:03:40Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.3  2005/07/31 02:44:17Z  Garyp
    Added prototypes.
    Revision 1.2  2005/07/31 00:38:11Z  Garyp
    Added support for DclUllToA().
    Revision 1.1  2005/07/18 09:25:06Z  pauli
    Initial revision
    Revision 1.7  2005/06/20 20:04:46Z  Pauli
    Included dl64bit.h for 64-bit operations.
    Revision 1.6  2005/06/17 21:40:47Z  PaulI
    Added prototype for DclOsGetChar.
    Revision 1.5  2005/06/15 02:42:32Z  PaulI
    Added condition to avoid including this header multiple times.
    Revision 1.4  2005/06/12 22:52:40Z  PaulI
    Added product numbers.  Defined DCLFUNC based on product being built.
    Revision 1.3  2005/04/16 23:14:11Z  PaulI
    XP Merge
    Revision 1.2.1.5  2005/04/16 23:14:11Z  GaryP
    Made the definition of TRUE and FALSE conditional.
    Revision 1.2.1.4  2005/04/16 10:24:46Z  PaulI
    Prototype update.
    Revision 1.2.1.3  2005/04/12 20:23:44Z  garyp
    Prototype updates.
    Revision 1.2.1.2  2005/04/10 22:30:11Z  garyp
    Added bunches of new stuff to support the newly implemented functions.
    Revision 1.2  2005/02/14 01:39:24Z  GaryP
    Updated to include dltlset.h and dltypes.h.
    Revision 1.1  2005/01/10 08:18:02Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DCL_H_INCLUDED
#define DCL_H_INCLUDED


/*-------------------------------------------------------------------
    If both dcl.h and dlapi.h are necessary (rare), dcl.h
    must be first.
-------------------------------------------------------------------*/
#ifdef DLAPI_H_INCLUDED
#error "dcl.h must be included before dlapi.h"
#endif


/*-------------------------------------------------------------------
    Include the basic necessities to use ANSI C
-------------------------------------------------------------------*/
#include <limits.h>
#include <stddef.h>


/*-------------------------------------------------------------------
    Symbol: D_PRODUCTLOCALE

    The D_PRODUCTLOCALE value indicates the code base to which a
    given piece of code belongs.

    The product locale number must match the values found in the
    the various prodinfo.bat files for each of the products (see
    dlprod.h for a further description).
-------------------------------------------------------------------*/
#ifndef D_PRODUCTLOCALE
#define D_PRODUCTLOCALE     (3)     /* DCL */
#endif

#ifndef TRUE
#define TRUE                (1)
#define FALSE               (0)
#endif


/*-------------------------------------------------------------------
    DCL_MAX_INSTANCES (undocumented)

    This setting defines the maximum number of DCL Driver Framework
    instances that may exist at any given time.  Eventually this may
    be initialized in the project's dclconf.h, however at this time
    there is no reason it would ever be more than one (1).
-------------------------------------------------------------------*/
#define DCL_MAX_INSTANCES   (1)


/*-------------------------------------------------------------------
    Headers that define environment and configuration information.

    Note that the order in which these headers are included is
    critical.
-------------------------------------------------------------------*/
#include <dltlset.h>        /* ToolSet numbers                      */
#include <dltlinit.h>       /* ToolSet initialization               */
#include <dlenv.h>          /* Compilation environment validation   */
#include <dlprod.h>         /* Product information                  */
#include <dclconf.h>        /* Project configuration                */
#include <dlosconf.h>       /* OS environment configuration         */


/*-------------------------------------------------------------------
    Headers that define interface characteristics.
-------------------------------------------------------------------*/
#include <dltypes.h>        /* Types used throughout DCL            */
#include <dlapi.h>          /* Public DCL API                       */
#include <dliosys.h>        /* I/O request system                   */
#include <dlapiint.h>       /* Internal DCL API                     */
#include <dlapimap.h>       /* Remappable API definitions           */
#include <dlassert.h>       /* DCL assert functionality             */
#include <dltrace.h>        /* Trace facility                       */
#include <dlprof.h>         /* Profiler                             */
#include <dlend.h>          /* Byte ordering                        */
#include <dl64bit.h>        /* 64-bit math interface                */
#include <dlmemval.h>       /* Memory validation service            */


/*-------------------------------------------------------------------
    Headers that define macros.
-------------------------------------------------------------------*/
#include <dlmacros.h>       /* Generic helper macros                */
#include <dlstatus.h>       /* DCLSTATUS layout                     */
#include <dlptr.h>          /* Pointer math                         */


/*-------------------------------------------------------------------
    DCL_MAX_INSTANCES (undocumented)

    This setting defines the maximum number of DCL Driver Framework
    instances that may exist at any given time.  Eventually this may
    be initialized in the project's dclconf.h, however at this time
    there is no reason it would ever be more than one (1).
-------------------------------------------------------------------*/
#define DCL_MAX_INSTANCES   (1)


#endif  /* DCL_H_INCLUDED */

