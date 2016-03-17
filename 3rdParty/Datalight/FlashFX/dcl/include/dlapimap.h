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

    This header provides the default mappings for DCL functions which may be
    remapped either at the OS level in dlosconf.h, or at the project level
    in dclconf.h.

                            DCL Header Hierarchy

    DCL interfaces are classified into separate headers based on their use
    and visibility.  Every source code module in DCL includes either dcl.h
    or dcl_osname.h as the top level header (after any OS-specific headers).
 
    dcl.h        - This is the top-level header for DCL code which is built
                   using the standard Datalight build process.  This header
                   autmatically includes dlapi.h, dlapiint.h, and dlapimap.h
                   (among others).

    dcl_osname.h - This is the OS-specific top-level header for DCL code
                   which may be built outside the standard Datalight build
                   process.  This header automatically includes dlapi.h.

                            Noteworthy Sub-Headers

    dlapi.h      - This header contains the DCL interface definitions for
                   those functions which may be used by code which could be
                   built outside the Datalight build process.  This header
                   MUST not depend on any other headers which reside outside
                   the DCL include directory.  It is automatically included
                   by both dcl.h and dcl_osname.h.

    dlapiint.h   - This header contains the DCL interface definitions for
                   those functions which are used by one or more Datalight
                   products, built using the standard Datalight product
                   build process.  It is automatically include by dcl.h.

    dlapimap.h   - This header contains the DCL interface definitions for
                   those functions which, similar to dlapiint.h, are used
                   internally by Datalight code, however may get remapped
                   by OS specific code.  Typically these functions are C
                   library equivalents.  It is automatically include by
                   dcl.h.

    dlapiprv.h  - This header contains the DCL interface definitions for
                  those functions which are private to DCL, and are not
                  intended for use outside DCL, whether by Datalight code
                  or not.  Typically these functions include low-level
                  primitives which are wrapped by higher level functions
                  which should be used.

    A few discrete headers exist for specific subsystems which must be
    explicitly included if they are to be used (generally they are used
    only internally by DCL).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlapimap.h $
    Revision 1.13  2009/11/10 23:40:23Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.12  2008/08/06 18:15:19Z  keithg
    Updated to map the memory functions to the DCL implementations rather than
    the C library functions.  OS abstractions must complete any explicit C 
    library mappings.
    Revision 1.11  2008/07/30 22:09:36Z  keithg
    Added default prototype for DclMemMove and associated.
    Revision 1.10  2008/06/05 02:39:20Z  brandont
    Added DclStrNChr.
    Revision 1.9  2007/11/10 23:00:42Z  brandont
    Added prototypes for DclStrRChr and DclStrNChr.
    Revision 1.8  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/04/01 17:40:16Z  Garyp
    Added DclStrNCat().
    Revision 1.6  2006/05/28 19:30:02Z  Garyp
    Added DclStrCmp().
    Revision 1.5  2006/02/03 03:15:06Z  brandont
    Added header files for prototypes of the memcpy(), ..., toupper() functions.
    Updated prototypes for some of the DCL standard C library implementations.
    Revision 1.4  2006/01/10 08:21:56Z  Garyp
    Added DclStrCat() support.
    Revision 1.3  2006/01/04 03:02:07Z  Garyp
    Documentation update.
    Revision 1.2  2005/12/28 00:00:59Z  Pauli
    Added DclStrChr.
    Revision 1.1  2005/10/07 02:45:26Z  Pauli
    Initial revision
    Revision 1.2  2005/10/07 03:45:25Z  Garyp
    Reverted back to using size_t types now that we're properly including
    stddef.h elsewhere.
    Revision 1.1  2005/10/02 06:38:56Z  Garyp
    Initial revision
    Revision 1.13  2005/09/28 19:53:26Z  Garyp
    Cleaned up data type and prototype interactions.
    Revision 1.12  2005/09/22 09:19:50Z  garyp
    Added mappings.
    Revision 1.11  2005/09/20 19:09:40Z  pauli
    Added DclOsSleep.
    Revision 1.10  2005/09/15 00:12:55Z  garyp
    Added DclMemStatistics().
    Revision 1.9  2005/08/21 04:40:58Z  garyp
    Eliminated // comments.
    Revision 1.8  2005/08/15 06:54:01Z  garyp
    Added DclProductVersion().
    Revision 1.7  2005/08/05 21:32:38Z  pauli
    Revision 1.5  2005/08/04 17:05:06Z  Garyp
    Added the prototypes from dlapi.h.
    Revision 1.4  2005/08/03 18:39:48Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.3  2005/07/31 02:44:17Z  Garyp
    Added prototypes.
    Revision 1.2  2005/07/31 00:38:08Z  Garyp
    Added support for DclUllToA().
    Revision 1.1  2005/07/18 10:12:42Z  pauli
    Initial revision
    Revision 1.4  2005/06/20 20:11:21Z  Pauli
    Added 64-bit operations.
    Revision 1.3  2005/06/17 18:43:17Z  PaulI
    Added DCL_OsGetChar.
    Revision 1.2  2005/06/13 19:47:04Z  PaulI
    Prototype updates.
    Revision 1.1  2005/04/11 01:52:34Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLAPIMAP_H_INCLUDED
#define DLAPIMAP_H_INCLUDED


/*-------------------------------------------------------------------
    This list of mappings may be superseded by the OS layer (defined
    in dlosconf.h) or overridden in the project code (dclconf.h).
    Therefore, define these mappings only if they are not already
    set up.

    Note that in most cases, we default to using DCL specific
    functions, but in a few others, particularly the mem* functions,
    we default to using the C Library versions.
-------------------------------------------------------------------*/
#ifndef DclAtoI
#define DclAtoI                     DCLFUNC(DclAtoI)
int     DclAtoI(const char * pach);
#endif

#ifndef DclAtoL
#define DclAtoL                     DCLFUNC(DclAtoL)
long    DclAtoL(const char * pach);
#endif

#ifndef DclIsDigit
#define DclIsDigit(c)               (c >= '0' && c <= '9')
#endif

#ifndef DclMemCpyAligned
#define DclMemCpyAligned            DCLFUNC(DclMemCpyAligned)
void *  DclMemCpyAligned(void * pDst, const void * pSrc, size_t nLen);
#endif

#ifndef DclMemSetAligned
#define DclMemSetAligned            DCLFUNC(DclMemSetAligned)
void *  DclMemSetAligned(void * pDst, int chr, size_t nLen);
#endif

#ifndef DclMemCmp
#define DclMemCmp                   DCLFUNC(DclMemCmp)
int DclMemCmp(const void * pBuff1, const void * pBuff2, size_t nLen);
#endif

#ifndef DclMemCpy
#define DclMemCpy                   DCLFUNC(DclMemCpy)
void * DclMemCpy(void *pDst, const void *pSrc, size_t nLen);
#endif

#ifndef DclMemMove
#define DclMemMove                  DCLFUNC(DclMemMove)
void * DclMemMove(void * pDst, const void * pSrc, size_t nLen);
#endif

#ifndef DclMemSet
#define DclMemSet                   DCLFUNC(DclMemSet)
void * DclMemSet(void *pDst, int chr, size_t nLen);
#endif

#ifndef DclStrCat
#define DclStrCat                   DCLFUNC(DclStrCat)
char *  DclStrCat(char *pszDst, const char *pszSrc);
#endif

#ifndef DclStrNCat
#define DclStrNCat                  DCLFUNC(DclStrNCat)
char *  DclStrNCat(char *pszDst, const char *pszSrc, size_t nCount);
#endif

#ifndef DclStrChr
#define DclStrChr                   DCLFUNC(DclStrChr)
char *  DclStrChr(const char * pszStr, int c);
#endif

#ifndef DclStrNChr
#define DclStrNChr                  DCLFUNC(DclStrNChr)
char * DclStrNChr(const char * pszStr, int iLen, int c);
#endif

#ifndef DclStrRChr
#define DclStrRChr                  DCLFUNC(DclStrRChr)
char *  DclStrRChr(const char * pszStr, int c);
#endif

#ifndef DclStrNChr
#define DclStrNChr                   DCLFUNC(DclStrNChr)
char *  DclStrNChr(const char * pszStr, int c, int len);
#endif

#ifndef DclStrCmp
#define DclStrCmp                   DCLFUNC(DclStrCmp)
int     DclStrCmp(const char *psz1, const char *psz2);
#endif

#ifndef DclStrICmp
#define DclStrICmp                  DCLFUNC(DclStrICmp)
int     DclStrICmp(const char * psz1, const char * psz2);
#endif

#ifndef DclStrNCmp
#define DclStrNCmp                  DCLFUNC(DclStrNCmp)
int     DclStrNCmp(const char * psz1, const char * psz2, size_t nLen);
#endif

#ifndef DclStrNICmp
#define DclStrNICmp                 DCLFUNC(DclStrNICmp)
int     DclStrNICmp(const char * psz1, const char * psz2, size_t nLen);
#endif

#ifndef DclStrCpy
#define DclStrCpy                   DCLFUNC(DclStrCpy)
char *  DclStrCpy(char * pszDst, const char * pszSrc);
#endif

#ifndef DclStrNCpy
#define DclStrNCpy                  DCLFUNC(DclStrNCpy)
char *  DclStrNCpy(char * pszDst, const char * pszSrc, size_t nLen);
#endif

#ifndef DclStrLen
#define DclStrLen                   DCLFUNC(DclStrLen)
size_t     DclStrLen(const char * pszStr);
#endif

#ifndef DclToLower
#include <ctype.h>
#define DclToLower(chr)             tolower(chr)
#endif

#ifndef DclToUpper
#include <ctype.h>
#define DclToUpper(chr)             toupper(chr)
#endif


#endif  /* DLAPIMAP_H_INCLUDED */

