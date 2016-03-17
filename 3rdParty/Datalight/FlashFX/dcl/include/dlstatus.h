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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header file contains the standard status codes used throughout
    the Datalight Common Libraries, and products that use them.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlstatus.h $
    Revision 1.51  2011/05/19 14:41:37Z  garyp
    Added a status code.
    Revision 1.50  2011/02/09 00:41:39Z  garyp
    Added status codes.
    Revision 1.49  2010/12/01 23:13:32Z  garyp
    Added status codes.
    Revision 1.48  2010/10/11 18:01:31Z  garyp
    Added status codes.
    Revision 1.47  2010/09/18 02:21:31Z  garyp
    Added status codes.
    Revision 1.46  2010/08/28 22:32:30Z  garyp
    Added status codes.
    Revision 1.45  2010/04/10 18:12:47Z  garyp
    Added status codes.
    Revision 1.44  2010/02/13 20:37:41Z  garyp
    Updated to use the correct license wording.
    Revision 1.43  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.42  2010/01/23 16:36:30Z  garyp
    Added a status code.
    Revision 1.41  2010/01/07 02:38:00Z  garyp
    Added status codes.
    Revision 1.40  2009/11/26 01:25:07Z  garyp
    Added status codes.
    Revision 1.39  2009/11/06 22:46:19Z  garyp
    Added some status codes.
    Revision 1.38  2009/11/03 15:37:46Z  garyp
    Added status codes.
    Revision 1.37  2009/09/24 21:52:15Z  garyp
    Added a status code.
    Revision 1.36  2009/09/14 22:30:03Z  garyp
    Added a status code.
    Revision 1.35  2009/09/09 22:57:03Z  garyp
    Added several new status codes.
    Revision 1.34  2009/07/13 13:27:34Z  garyp
    Added status codes.
    Revision 1.33  2009/06/30 02:52:23Z  garyp
    Added a status code.
    Revision 1.32  2009/06/27 00:35:37Z  garyp
    Added numerous status codes.
    Revision 1.31  2009/06/12 02:04:51Z  garyp
    Added numerous file system related status codes.
    Revision 1.30  2009/05/24 17:06:53Z  garyp
    Added status codes.
    Revision 1.29  2009/04/09 21:39:36Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.28  2009/03/25 01:31:18Z  glenns
    Fix Bugzilla #2464: Added DCLSTAT_INTERNAL_ERROR, a code to be used in
    locations where we know an "impossible" code path has been reached.
    Revision 1.27  2009/02/10 02:18:53Z  garyp
    Added a status code.
    Revision 1.26  2009/02/08 02:49:28Z  garyp
    Merged from the v4.0 branch.  Added numerous status codes.
    Revision 1.25  2009/01/26 21:32:15Z  johnb
    Added new status code to manage bad multibyte character
    sequences, DCLSTAT_BADMULTIBYTECHAR.
    Revision 1.24  2008/06/21 00:14:16Z  brandont
    Added a couple of error codes for the stats interface.
    Revision 1.23  2008/05/23 02:15:52Z  garyp
    Merged from the WinMobile branch.
    Revision 1.22.1.2  2008/05/23 02:15:52Z  garyp
    Added a status code.
    Revision 1.22  2008/05/04 17:29:55Z  garyp
    Added a status code.
    Revision 1.21  2008/04/16 23:45:32Z  brandont
    Added a new class of errors for the DCL file system services.
    Revision 1.20  2007/12/13 03:27:24Z  Garyp
    Added status codes.
    Revision 1.19  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.18  2007/10/14 16:11:43Z  Garyp
    Added status codes.
    Revision 1.17  2007/10/05 01:29:32Z  pauli
    Added DCLSTAT_MEMUNDERRUN.
    Revision 1.16  2007/10/03 22:00:22Z  brandont
    Added DCLLOCALE_LOADER.
    Revision 1.15  2007/08/05 22:09:00Z  garyp
    Refactored the "general" status locale so the values more readily reflect
    traditional DOS/Win32 error values, which can then be mapped onto
    errorlevels.  See dlerrlev.c/h for more information.
    Revision 1.14  2007/06/22 16:24:31Z  Garyp
    Added a number of status codes (work-in-progress).
    Revision 1.13  2007/05/14 17:31:08Z  garyp
    New/corrected status codes.
    Revision 1.12  2007/04/07 02:52:03Z  Garyp
    Added the ability to set and extract a 20-bit value from a DCLSTATUS type.
    Revision 1.11  2007/04/01 21:31:46Z  Garyp
    Fixed a typo.
    Revision 1.10  2007/03/30 18:57:48Z  Garyp
    Added status codes.
    Revision 1.9  2007/01/16 22:15:36Z  Garyp
    New status codes.
    Revision 1.8  2007/01/12 02:40:25Z  Garyp
    Added shell status codes.
    Revision 1.7  2006/12/20 23:40:35Z  billr
    Add a new status.
    Revision 1.6  2006/10/11 22:20:19Z  Garyp
    Fixed to ensure that status codes are generated with the proper codebase
    value.
    Revision 1.5  2006/10/07 02:49:14Z  Garyp
    Added status codes.
    Revision 1.4  2006/09/19 20:27:34Z  brandont
    Added error codes for thread suspend and resume.
    Revision 1.3  2006/02/28 23:12:08Z  Garyp
    Added status codes.
    Revision 1.2  2006/01/02 02:21:56Z  Garyp
    Added status codes.
    Revision 1.1  2005/12/05 17:37:40Z  Pauli
    Initial revision
    Revision 1.2  2005/12/05 17:37:39Z  Garyp
    Added status codes.
    Revision 1.1  2005/10/04 22:57:00Z  Garyp
    Initial revision
    Revision 1.2  2005/09/29 01:24:26Z  Garyp
    Added new types and status codes.
    Revision 1.1  2005/07/16 03:20:36Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLSTATUS_H_INCLUDED
#define DLSTATUS_H_INCLUDED


/*-------------------------------------------------------------------
    Type: DCLSTATUS

    A structured status code used to return error/status information.

    DCLSTATUS is a 32-bit structured status code upon which all other
    product status codes are based.  The code has the following layout:

    <pre>
    {{+------------------------------------------------------+}}
    {{|                                                      |}}
    {{|             High 16-bits     -     Low 16-bits       |}}
    {{|          rccc rrrp pppp llll - llll ssss ssss ssss   |}}
    {{|          /\ / \ /\    / \         / \            /   |}}
    {{|  Reserved  |   |  \  /  |\       /   \          /|   |}}
    {{|          Class |CodeBase| \     /    Status Value|   |}}
    {{|            Reserved     | Locale                 |   |}}
    {{|                         |                        |   |}}
    {{|                         -------- Line Num --------   |}}
    {{|                         |                        |   |}}
    {{|                         --------  UINT20 ---------   |}}
    {{|                                                      |}}
    {{+------------------------------------------------------+}}
    </pre>

    This bit-mapped status code is laid out to make it reasonably
    easy to decode the hex value at a glance.  This is facilitated
    by ensuring that the major components of the status, the Class,
    CodeBase, Locale, and Status Value, are each aligned so their
    least signficant bits fall on a nybble boundary.

    The "class" field defines the type of status code, which will be
    one of the following:

        DCLSTATUSCLASS_ERROR     -  1
        DCLSTATUSCLASS_INFO      -  2
        DCLSTATUSCLASS_LINENUM   -  3
        DCLSTATUSCLASS_UINT20    -  4

    The "CodeBase" field defines the code base in which the error
    originated.  This value will identical to the product codes
    defined in dlprod.h.

    The "Locale" field defines the location within the CodeBase
    where the error orginated.  The following Locales are defined
    for DCL:

        DCLLOCALE_GENERAL        -  1
        DCLLOCALE_OSSERV         -  2
        DCLLOCALE_INSTANCE       -  3
        DCLLOCALE_MEMORY         -  4
        DCLLOCALE_FILE           -  5
        DCLLOCALE_LOG            -  6
        DCLLOCALE_SHELL          -  7
        DCLLOCALE_ENV            -  8
        DCLLOCALE_DRIVERFW       -  9
        DCLLOCALE_PERFLOG        - 10
        DCLLOCALE_MISC           - 11
        DCLLOCALE_LOADER         - 12
        DCLLOCALE_TIME           - 13
        DCLLOCALE_WIN            - 14

    The "Status Value" is a unique status code within each Locale.
    See dlstatus.h for a detailed list of Locales and Status Values.

    The DCLSTATUSCLASS_LINENUM is a special class of DCLSTATUS values
    where the locale and status bits together are used to indicate a
    line number.

    The DCLSTATUSCLASS_UINT20 is a special class of DCLSTATUS values
    where the locale and status bits together are used to indicate a
    20-bit unsigned integer value.  The macros DCLSTAT_SETUINT20()
    and DCLSTAT_GETUINT20() are used to encode and decode these status
    values.

    Several status codes have special
    definitions:
        DCLSTAT_SUCCESS - This is the standard success status code.
        DCLSTAT_FAILURE - This is the default status code for for
                          failure, when a more specific status code
                          is not defined.  The use of specific status
                          codes is highly preferable.

    Along with DCLSTAT_SUCCESS and DCLSTAT_FAILURE, the entire Locale
    of "General" status codes is designed to be shared across products.
    Any product specific names are for convenience only -- the actual
    definition is the same across products, meaning that the "CodeBase"
    field for a "General" status code cannot be used to differentiate
    an identical status originating on one CodeBase versus another.
-------------------------------------------------------------------*/
#define DCLSTATCLASSMASK        (0x7)      /* 3 class bits */
#define DCLSTATCLASSSHIFT       (28)
#define DCLSTATCODEBASEMASK     (0x1F)     /* 5 code base ID bits */
#define DCLSTATCODEBASESHIFT    (20)
#define DCLSTATLOCMASK          (0xFF)     /* 8 locale bits (locale within the code base) */
#define DCLSTATLOCSHIFT         (12)
#define DCLSTATVALMASK          (0xFFF)    /* 12 status value bits */
#define DCLSTATVALSHIFT         (0)
#define DCLSTATLINENUMMASK      (0xFFFFF)  /* 20 line number bits */
#define DCLSTATLINENUMSHIFT     (0)
#define DCLSTATUINT20MASK       (0xFFFFF)  /* 20 "UINT20" bits */
#define DCLSTATUINT20SHIFT      (0)

#define DCLSTATGETCLASS(stat)   (((DCLSTATUS)(stat) >> (DCLSTATCLASSSHIFT)) & DCLSTATCLASSMASK)
#define DCLSTATSETCLASS(clas)   ((DCLSTATUS)(clas)  << (DCLSTATCLASSSHIFT))

#define DCLSTATGETCODEBASE(stat)(((DCLSTATUS)(stat) >> (DCLSTATCODEBASESHIFT)) & DCLSTATCODEBASEMASK)
#define DCLSTATSETCODEBASE(prod)((DCLSTATUS)(prod)  << (DCLSTATCODEBASESHIFT))

#define DCLSTATGETLOC(stat)     (((DCLSTATUS)(stat) >> (DCLSTATLOCSHIFT)) & DCLSTATLOCMASK)
#define DCLSTATSETLOC(loc)      ((DCLSTATUS)(loc)   << (DCLSTATLOCSHIFT))

#define DCLSTATGETVAL(stat)     (((DCLSTATUS)(stat) >> (DCLSTATVALSHIFT)) & DCLSTATVALMASK)
#define DCLSTATSETVAL(val)      ((DCLSTATUS)(val)   << (DCLSTATVALSHIFT))

#define DCLSTATGETLINENUM(stat) (((DCLSTATUS)(stat) >> (DCLSTATLINENUMSHIFT)) & DCLSTATLINENUMMASK)
#define DCLSTATSETLINENUM(lin)  ((DCLSTATUS)(lin)   << (DCLSTATLINENUMSHIFT))

#define DCLSTATGETUINT20(stat)  (((DCLSTATUS)(stat) >> (DCLSTATUINT20SHIFT)) & DCLSTATUINT20MASK)
#define DCLSTATSETUINT20(u20)   ((DCLSTATUS)(u20)   << (DCLSTATUINT20SHIFT))

/*  Class values are NOT product specific, so declare them here.
*/
#define DCLSTATUSCLASS_ERROR    1
#define DCLSTATUSCLASS_INFO     2
#define DCLSTATUSCLASS_LINENUM  3
#define DCLSTATUSCLASS_UINT20   4

/*  This is a general macro for combining all the elements together to
    make a DCLSTATUS value.
*/
#define DCL_MAKESTATUS(codebase, clas, loc, val)            \
    ((DCLSTATUS)(                                           \
    DCLSTATSETCLASS(clas) |                                 \
    DCLSTATSETCODEBASE(codebase) |                          \
    DCLSTATSETLOC(loc) |                                    \
    DCLSTATSETVAL(val)))

#define DCLSTAT_CURRENTLINE                                 \
    ((DCLSTATUS)(                                           \
    DCLSTATSETCLASS(DCLSTATUSCLASS_LINENUM) |               \
    DCLSTATSETCODEBASE(PRODUCTNUM_DCL) |                    \
    DCLSTATSETLINENUM(__LINE__)))


/*-------------------------------------------------------------------
    DCLSTAT_GET/SETUINT20()

    These macros are used to store and retrieve an unsigned 20-bit
    integer in a DCLSTATUS value.  This is done by re-using the
    locale and status fields in the DCLSTATUS bitmask, and using
    a special class -- DCLSTATUSCLASS_UINT20 -- which indicates
    that the DCLSTATUS value is special.

    If the value returned by DCLSTAT_GETUINT20() is D_UINT32_MAX,
    then the original DCLSTATUS value was NOT a valid UINT20 value.
-------------------------------------------------------------------*/

#define DCLSTAT_SETUINT20(u20)                              \
    ((DCLSTATUS)(                                           \
    DCLSTATSETCLASS(DCLSTATUSCLASS_UINT20) |                \
    DCLSTATSETCODEBASE(PRODUCTNUM_DCL) |                    \
    DCLSTATSETUINT20(u20)))

#define DCLSTAT_GETUINT20(stat)                             \
    ((D_UINT32)                                             \
    ((DCLSTATGETCLASS(stat) == DCLSTATUSCLASS_UINT20) ?     \
    DCLSTATGETUINT20(stat) : D_UINT32_MAX))


/*-------------------------------------------------------------------
    Now that the foundational stuff is set up, define DCL specific
    locales and status values.
-------------------------------------------------------------------*/

/*  This is a general macro for combining all the elements together to
    make a DCLSTATUS value that is specific to the DCL product.
*/
#define MAKEDCLERR(loc, val) DCL_MAKESTATUS(PRODUCTNUM_DCL, DCLSTATUSCLASS_ERROR, loc, val)

/*  Note that DCLSTAT_SUCCESS must be zero for compatibility
    purposes.
*/
#define DCLSTAT_SUCCESS               0x00000000UL
#define DCLSTAT_CONTINUE              0xFFFFFFFEUL
#define DCLSTAT_FAILURE               0xFFFFFFFFUL

/*  Update the documentation above when adding items to this list
*/
#define DCLLOCALE_GENERAL             1
#define DCLLOCALE_OSSERV              2
#define DCLLOCALE_INSTANCE            3
#define DCLLOCALE_MEMORY              4
#define DCLLOCALE_FILE                5
#define DCLLOCALE_LOG                 6
#define DCLLOCALE_SHELL               7
#define DCLLOCALE_ENV                 8
#define DCLLOCALE_DRIVERFW            9
#define DCLLOCALE_PERFLOG            10
#define DCLLOCALE_MISC               11
#define DCLLOCALE_LOADER             12
#define DCLLOCALE_TIME               13
#define DCLLOCALE_WIN                14
#define DCLLOCALE_BLOCKDEV           15


/*-------------------------------------------------------------------
    General status codes.

      - The first 128 values are set aside to generally match the
        traditional DOS errorcode values.
      - The second 128 values are Datalight specific codes.
      - The first 256 values may be mapped directly onto errorlevel
        values, as implemented in dlerrlev.h.  See that file for
        more details.  Any status value with a code greater than
        255 will not be directly mappable onto an errorlevel.
      - In those rare cases where there are similar status values,
        such as DCLSTAT_OUTOFMEMORY and DCLSTAT_MEMALLOCFAILED, the
        status code from the "general" locale should only be used
        where direct errorlevel mapping is required, otherwise the
        values from the more specific locales should be used.
-------------------------------------------------------------------*/
/*#define DCLSTAT_RESERVED_DONT_USE         (MAKEDCLERR(DCLLOCALE_GENERAL,   0)) */
#define DCLSTAT_BADREQUEST                  (MAKEDCLERR(DCLLOCALE_GENERAL,   1))
#define DCLSTAT_FILENOTFOUND                (MAKEDCLERR(DCLLOCALE_GENERAL,   2))
#define DCLSTAT_PATHNOTFOUND                (MAKEDCLERR(DCLLOCALE_GENERAL,   3))
#define DCLSTAT_OUTOFHANDLES                (MAKEDCLERR(DCLLOCALE_GENERAL,   4))
#define DCLSTAT_ACCESSDENIED                (MAKEDCLERR(DCLLOCALE_GENERAL,   5))
#define DCLSTAT_BADHANDLE                   (MAKEDCLERR(DCLLOCALE_GENERAL,   6))
#define DCLSTAT_OUTOFMEMORY                 (MAKEDCLERR(DCLLOCALE_GENERAL,   8))
#define DCLSTAT_INVALIDDRIVE                (MAKEDCLERR(DCLLOCALE_GENERAL,  15))
#define DCLSTAT_WRITEPROTECT                (MAKEDCLERR(DCLLOCALE_GENERAL,  19))
#define DCLSTAT_NOTREADY                    (MAKEDCLERR(DCLLOCALE_GENERAL,  21))
#define DCLSTAT_BADSTRUCLEN                 (MAKEDCLERR(DCLLOCALE_GENERAL,  24))
#define DCLSTAT_WRITEFAILED                 (MAKEDCLERR(DCLLOCALE_GENERAL,  29))
#define DCLSTAT_READFAILED                  (MAKEDCLERR(DCLLOCALE_GENERAL,  30))
#define DCLSTAT_BADPARAMETER                (MAKEDCLERR(DCLLOCALE_GENERAL,  87))
        /* Datalight specific values... */
#define DCLSTAT_HELPREQUEST                 (MAKEDCLERR(DCLLOCALE_GENERAL, 128))
#define DCLSTAT_VERSIONMISMATCH             (MAKEDCLERR(DCLLOCALE_GENERAL, 129))
#define DCLSTAT_UNSUPPORTEDCOMMAND          (MAKEDCLERR(DCLLOCALE_GENERAL, 130))
#define DCLSTAT_UNSUPPORTEDFUNCTION         (MAKEDCLERR(DCLLOCALE_GENERAL, 131))
#define DCLSTAT_UNSUPPORTEDREQUEST          (MAKEDCLERR(DCLLOCALE_GENERAL, 132))
#define DCLSTAT_LIMITREACHED                (MAKEDCLERR(DCLLOCALE_GENERAL, 133))
#define DCLSTAT_DISKNUMBERINVALID           (MAKEDCLERR(DCLLOCALE_GENERAL, 134))
#define DCLSTAT_FEATUREDISABLED             (MAKEDCLERR(DCLLOCALE_GENERAL, 135))
#define DCLSTAT_BADSYNTAX                   (MAKEDCLERR(DCLLOCALE_GENERAL, 136))
#define DCLSTAT_DRIVERLOCKFAILED            (MAKEDCLERR(DCLLOCALE_GENERAL, 137))
#define DCLSTAT_INVALIDDEVICE               (MAKEDCLERR(DCLLOCALE_GENERAL, 138))
#define DCLSTAT_OUTOFRANGE                  (MAKEDCLERR(DCLLOCALE_GENERAL, 139))
#define DCLSTAT_UNKNOWNDEVICE               (MAKEDCLERR(DCLLOCALE_GENERAL, 140))
#define DCLSTAT_BUFFERTOOSMALL              (MAKEDCLERR(DCLLOCALE_GENERAL, 141))
#define DCLSTAT_BADINSTANCENUMBER           (MAKEDCLERR(DCLLOCALE_GENERAL, 142))
#define DCLSTAT_BADINSTANCEHANDLE           (MAKEDCLERR(DCLLOCALE_GENERAL, 143))
#define DCLSTAT_BADPOINTER                  (MAKEDCLERR(DCLLOCALE_GENERAL, 144))
#define DCLSTAT_INSTANCECREATIONFAILED      (MAKEDCLERR(DCLLOCALE_GENERAL, 145))
#define DCLSTAT_OUTPUTDISABLED              (MAKEDCLERR(DCLLOCALE_GENERAL, 146))
#define DCLSTAT_UNSUPPORTEDVERSION          (MAKEDCLERR(DCLLOCALE_GENERAL, 147))
#define DCLSTAT_ARGUMENTSTOOLONG            (MAKEDCLERR(DCLLOCALE_GENERAL, 148))
#define DCLSTAT_BADPARAMETERLEN             (MAKEDCLERR(DCLLOCALE_GENERAL, 149))
#define DCLSTAT_CATEGORYDISABLED            (MAKEDCLERR(DCLLOCALE_GENERAL, 150))
#define DCLSTAT_SUBTYPECOMPLETE             (MAKEDCLERR(DCLLOCALE_GENERAL, 151))
#define DCLSTAT_SUBTYPEUNUSED               (MAKEDCLERR(DCLLOCALE_GENERAL, 152))
#define DCLSTAT_BUSY                        (MAKEDCLERR(DCLLOCALE_GENERAL, 153))
#define DCLSTAT_NOTENABLED                  (MAKEDCLERR(DCLLOCALE_GENERAL, 154))
#define DCLSTAT_UNSUPPORTEDFEATURE          (MAKEDCLERR(DCLLOCALE_GENERAL, 155))
#define DCLSTAT_INTERNAL_ERROR              (MAKEDCLERR(DCLLOCALE_GENERAL, 156))
#define DCLSTAT_USERABORT                   (MAKEDCLERR(DCLLOCALE_GENERAL, 157))
#define DCLSTAT_DATAVERIFYERROR             (MAKEDCLERR(DCLLOCALE_GENERAL, 158))
#define DCLSTAT_DISCARDFAILED               (MAKEDCLERR(DCLLOCALE_GENERAL, 159))
#define DCLSTAT_UNSUPPORTED                 (MAKEDCLERR(DCLLOCALE_GENERAL, 160))
#define DCLSTAT_TIMEOUT                     (MAKEDCLERR(DCLLOCALE_GENERAL, 161))
/*#define DCLSTAT_RESERVED_DONT_USE         (MAKEDCLERR(DCLLOCALE_GENERAL, 255)) */

/*  OS Service status codes
*/
#define DCLSTAT_MUTEXCREATEFAILED           (MAKEDCLERR(DCLLOCALE_OSSERV, 10))
#define DCLSTAT_MUTEXACQUIREFAILED          (MAKEDCLERR(DCLLOCALE_OSSERV, 11))
#define DCLSTAT_MUTEXDESTROYFAILED          (MAKEDCLERR(DCLLOCALE_OSSERV, 12))
#define DCLSTAT_MUTEXRELEASEFAILED          (MAKEDCLERR(DCLLOCALE_OSSERV, 13))
#define DCLSTAT_MUTEXTRACKINGDISABLED       (MAKEDCLERR(DCLLOCALE_OSSERV, 14))
#define DCLSTAT_MUTEXSTATSERROR             (MAKEDCLERR(DCLLOCALE_OSSERV, 15))
#define DCLSTAT_MUTEX_STATICOSMUTEXINUSE    (MAKEDCLERR(DCLLOCALE_OSSERV, 16))
#define DCLSTAT_MUTEX_STATICOSMUTEXNOTINUSE (MAKEDCLERR(DCLLOCALE_OSSERV, 17))
#define DCLSTAT_MUTEX_TIMEOUT               (MAKEDCLERR(DCLLOCALE_OSSERV, 18))

#define DCLSTAT_SEMAPHORECREATEFAILED       (MAKEDCLERR(DCLLOCALE_OSSERV, 20))
#define DCLSTAT_SEMAPHOREACQUIREFAILED      (MAKEDCLERR(DCLLOCALE_OSSERV, 21))
#define DCLSTAT_SEMAPHOREDESTROYFAILED      (MAKEDCLERR(DCLLOCALE_OSSERV, 22))
#define DCLSTAT_SEMAPHORERELEASEFAILED      (MAKEDCLERR(DCLLOCALE_OSSERV, 23))
#define DCLSTAT_SEMAPHORETRACKINGDISABLED   (MAKEDCLERR(DCLLOCALE_OSSERV, 24))
#define DCLSTAT_SEMAPHORESTATSERROR         (MAKEDCLERR(DCLLOCALE_OSSERV, 25))
#define DCLSTAT_SEMAPHOREALREADYEXISTS      (MAKEDCLERR(DCLLOCALE_OSSERV, 26))

#define DCLSTAT_THREADCREATEFAILED          (MAKEDCLERR(DCLLOCALE_OSSERV, 30))
#define DCLSTAT_THREADBADPRIORITY           (MAKEDCLERR(DCLLOCALE_OSSERV, 31))
#define DCLSTAT_THREADDESTROYFAILED         (MAKEDCLERR(DCLLOCALE_OSSERV, 32))
#define DCLSTAT_THREADTERMINATEFAILED       (MAKEDCLERR(DCLLOCALE_OSSERV, 33))
#define DCLSTAT_THREADWAITFAILED            (MAKEDCLERR(DCLLOCALE_OSSERV, 34))
#define DCLSTAT_THREADWAITTIMEOUT           (MAKEDCLERR(DCLLOCALE_OSSERV, 35))
#define DCLSTAT_THREADATTRCREATEFAILED      (MAKEDCLERR(DCLLOCALE_OSSERV, 36))
#define DCLSTAT_THREADSUSPENDFAILED         (MAKEDCLERR(DCLLOCALE_OSSERV, 37))
#define DCLSTAT_THREADRESUMEFAILED          (MAKEDCLERR(DCLLOCALE_OSSERV, 38))
#define DCLSTAT_THREADBADATTRIBUTES         (MAKEDCLERR(DCLLOCALE_OSSERV, 39))
#define DCLSTAT_THREADINVALIDID             (MAKEDCLERR(DCLLOCALE_OSSERV, 40))

#define DCLSTAT_REQUESTOR_BADHANDLE         (MAKEDCLERR(DCLLOCALE_OSSERV, 50))
#define DCLSTAT_REQUESTOR_BADDEVICE         (MAKEDCLERR(DCLLOCALE_OSSERV, 51))
#define DCLSTAT_REQUESTOR_NOTOPEN           (MAKEDCLERR(DCLLOCALE_OSSERV, 52))
#define DCLSTAT_REQUESTOR_REQUESTFAILED     (MAKEDCLERR(DCLLOCALE_OSSERV, 53))
#define DCLSTAT_REQUESTOR_INTERFACEFAILED   (MAKEDCLERR(DCLLOCALE_OSSERV, 54))
#define DCLSTAT_REQUESTOR_OPENFAILED        (MAKEDCLERR(DCLLOCALE_OSSERV, 55))
#define DCLSTAT_REQUESTOR_CLOSEFAILED       (MAKEDCLERR(DCLLOCALE_OSSERV, 56))
#define DCLSTAT_REQUESTOR_ALREADYOPEN       (MAKEDCLERR(DCLLOCALE_OSSERV, 57))
#define DCLSTAT_REQUESTOR_UNSUPPORTEDDEVICE (MAKEDCLERR(DCLLOCALE_OSSERV, 58))
#define DCLSTAT_REQUESTOR_OUTOFHANDLES      (MAKEDCLERR(DCLLOCALE_OSSERV, 59))
#define DCLSTAT_REQUESTOR_BADNAME           (MAKEDCLERR(DCLLOCALE_OSSERV, 60))

#define DCLSTAT_MEM_VIRTUALMAPFAILED1       (MAKEDCLERR(DCLLOCALE_OSSERV, 70))
#define DCLSTAT_MEM_VIRTUALMAPFAILED2       (MAKEDCLERR(DCLLOCALE_OSSERV, 71))
#define DCLSTAT_MEM_VIRTUALUNMAPFAILED      (MAKEDCLERR(DCLLOCALE_OSSERV, 72))

/*  DCL Instance/Project/Service status codes
*/
#define DCLSTAT_INST_OUTOFRANGE             (MAKEDCLERR(DCLLOCALE_INSTANCE,   1))
#define DCLSTAT_INST_BADHANDLE              (MAKEDCLERR(DCLLOCALE_INSTANCE,   2))
#define DCLSTAT_INST_NOTINITIALIZED         (MAKEDCLERR(DCLLOCALE_INSTANCE,   3))
#define DCLSTAT_INST_ALREADYINITIALIZED     (MAKEDCLERR(DCLLOCALE_INSTANCE,   4))
#define DCLSTAT_INST_CORRUPTED              (MAKEDCLERR(DCLLOCALE_INSTANCE,   5))
#define DCLSTAT_INST_DESTROYFAILED          (MAKEDCLERR(DCLLOCALE_INSTANCE,   6))
#define DCLSTAT_INST_PROJECTNOTINITIALIZED  (MAKEDCLERR(DCLLOCALE_INSTANCE,   7))
#define DCLSTAT_INST_SYSMUTEXNOTINITIALIZED (MAKEDCLERR(DCLLOCALE_INSTANCE,   8))
#define DCLSTAT_INST_SERVICESALREADYCREATED (MAKEDCLERR(DCLLOCALE_INSTANCE,   9))
#define DCLSTAT_INST_SERVICESNOTCREATED     (MAKEDCLERR(DCLLOCALE_INSTANCE,  10))
#define DCLSTAT_INST_RECREATEFAILED         (MAKEDCLERR(DCLLOCALE_INSTANCE,  11))
#define DCLSTAT_INST_USAGECOUNTREDUCED      (MAKEDCLERR(DCLLOCALE_INSTANCE,  12))
#define DCLSTAT_INST_HANDLEMISMATCH         (MAKEDCLERR(DCLLOCALE_INSTANCE,  13))
#define DCLSTAT_PROJ_BADHANDLE              (MAKEDCLERR(DCLLOCALE_INSTANCE, 100))
#define DCLSTAT_PROJ_BADINSTANCEHANDLE      (MAKEDCLERR(DCLLOCALE_INSTANCE, 101))
#define DCLSTAT_PROJ_NOTINITIALIZED         (MAKEDCLERR(DCLLOCALE_INSTANCE, 102))
#define DCLSTAT_PROJ_ALREADYINITIALIZED     (MAKEDCLERR(DCLLOCALE_INSTANCE, 103))
#define DCLSTAT_PROJ_CORRUPTED              (MAKEDCLERR(DCLLOCALE_INSTANCE, 104))
#define DCLSTAT_PROJ_INVALID                (MAKEDCLERR(DCLLOCALE_INSTANCE, 105))
#define DCLSTAT_PROJ_INITCANCELLED          (MAKEDCLERR(DCLLOCALE_INSTANCE, 106))
#define DCLSTAT_SERVICE_NOTINITIALIZED      (MAKEDCLERR(DCLLOCALE_INSTANCE, 200))
#define DCLSTAT_SERVICE_ALREADYINITIALIZED  (MAKEDCLERR(DCLLOCALE_INSTANCE, 201))
#define DCLSTAT_SERVICE_BADHANDLE           (MAKEDCLERR(DCLLOCALE_INSTANCE, 202))
#define DCLSTAT_SERVICE_NUMBERINVALID       (MAKEDCLERR(DCLLOCALE_INSTANCE, 203))
#define DCLSTAT_SERVICE_DESTROYFAILED       (MAKEDCLERR(DCLLOCALE_INSTANCE, 204))
#define DCLSTAT_SERVICE_CORRUPTED           (MAKEDCLERR(DCLLOCALE_INSTANCE, 205))
#define DCLSTAT_SERVICE_ALREADYREGISTERED   (MAKEDCLERR(DCLLOCALE_INSTANCE, 206))
#define DCLSTAT_SERVICE_NOTREGISTERED       (MAKEDCLERR(DCLLOCALE_INSTANCE, 207))
#define DCLSTAT_SERVICE_DEREGISTERFAILED    (MAKEDCLERR(DCLLOCALE_INSTANCE, 208))
#define DCLSTAT_SERVICE_NOIOCTLINTERFACE    (MAKEDCLERR(DCLLOCALE_INSTANCE, 209))
#define DCLSTAT_SERVICE_BADREQUEST          (MAKEDCLERR(DCLLOCALE_INSTANCE, 210))
#define DCLSTAT_SERVICE_BADSUBREQUEST       (MAKEDCLERR(DCLLOCALE_INSTANCE, 211))
#define DCLSTAT_SERVICE_UNSUPPORTEDREQUEST  (MAKEDCLERR(DCLLOCALE_INSTANCE, 212))
#define DCLSTAT_SERVICE_UNHANDLEDREQUEST    (MAKEDCLERR(DCLLOCALE_INSTANCE, 213))
#define DCLSTAT_SERVICE_FLAGSBAD            (MAKEDCLERR(DCLLOCALE_INSTANCE, 214))
#define DCLSTAT_SERVICE_RECURSEFAILED       (MAKEDCLERR(DCLLOCALE_INSTANCE, 215))
#define DCLSTAT_SERVICE_NOPREVIOUSSERVICE   (MAKEDCLERR(DCLLOCALE_INSTANCE, 216))

/*  Memory status codes
*/
#define DCLSTAT_MEMALLOCFAILED              (MAKEDCLERR(DCLLOCALE_MEMORY,  1))
#define DCLSTAT_MEMPOOLCORRUPTED            (MAKEDCLERR(DCLLOCALE_MEMORY,  3))
#define DCLSTAT_MEMUNDERRUN                 (MAKEDCLERR(DCLLOCALE_MEMORY,  4))
#define DCLSTAT_MEMOVERRUN                  (MAKEDCLERR(DCLLOCALE_MEMORY,  5))
#define DCLSTAT_MEMFREEFAILED               (MAKEDCLERR(DCLLOCALE_MEMORY,  6))
#define DCLSTAT_MEM_NULLPOINTER             (MAKEDCLERR(DCLLOCALE_MEMORY,  7))
#define DCLSTAT_MEM_INITFAILURE             (MAKEDCLERR(DCLLOCALE_MEMORY,  8))

#define DCLSTAT_MEMVAL_OUTOFENTRIES         (MAKEDCLERR(DCLLOCALE_MEMORY, 50))
#define DCLSTAT_MEMVAL_INVALIDBUFFER        (MAKEDCLERR(DCLLOCALE_MEMORY, 51))
#define DCLSTAT_MEMVAL_INVALIDLENGTH        (MAKEDCLERR(DCLLOCALE_MEMORY, 52))
#define DCLSTAT_MEMVAL_NOTINITIALIZED       (MAKEDCLERR(DCLLOCALE_MEMORY, 53))
#define DCLSTAT_MEMVAL_READONLY             (MAKEDCLERR(DCLLOCALE_MEMORY, 54))
#define DCLSTAT_MEMVAL_READPOINTERNULL      (MAKEDCLERR(DCLLOCALE_MEMORY, 55))
#define DCLSTAT_MEMVAL_READLENGTHBAD        (MAKEDCLERR(DCLLOCALE_MEMORY, 56))
#define DCLSTAT_MEMVAL_READBUFFER           (MAKEDCLERR(DCLLOCALE_MEMORY, 57))
#define DCLSTAT_MEMVAL_WRITEPOINTERNULL     (MAKEDCLERR(DCLLOCALE_MEMORY, 58))
#define DCLSTAT_MEMVAL_WRITELENGTHBAD       (MAKEDCLERR(DCLLOCALE_MEMORY, 59))
#define DCLSTAT_MEMVAL_WRITEBUFFER          (MAKEDCLERR(DCLLOCALE_MEMORY, 60))


/*  File System operation codes
*/
#define DCLSTAT_FS_CREATEFAILED             (MAKEDCLERR(DCLLOCALE_FILE, 1))
#define DCLSTAT_FS_IOERROR                  (MAKEDCLERR(DCLLOCALE_FILE, 2))
#define DCLSTAT_FS_UNDEFINED                (MAKEDCLERR(DCLLOCALE_FILE, 3))
#define DCLSTAT_FS_DENIED                   (MAKEDCLERR(DCLLOCALE_FILE, 4))
#define DCLSTAT_FS_PATHTOOLONG              (MAKEDCLERR(DCLLOCALE_FILE, 5))
#define DCLSTAT_FS_BADPATH                  (MAKEDCLERR(DCLLOCALE_FILE, 6))
#define DCLSTAT_FS_NOTFOUND                 (MAKEDCLERR(DCLLOCALE_FILE, 7))
#define DCLSTAT_FS_BADHANDLE                (MAKEDCLERR(DCLLOCALE_FILE, 8))
#define DCLSTAT_FS_NOHANDLES                (MAKEDCLERR(DCLLOCALE_FILE, 9))
#define DCLSTAT_FS_HANDLESHARE              (MAKEDCLERR(DCLLOCALE_FILE, 10))
#define DCLSTAT_FS_EOF                      (MAKEDCLERR(DCLLOCALE_FILE, 11))
#define DCLSTAT_FS_FULL                     (MAKEDCLERR(DCLLOCALE_FILE, 12))
#define DCLSTAT_FS_EXISTS                   (MAKEDCLERR(DCLLOCALE_FILE, 13))
#define DCLSTAT_FS_OPENFAILED               (MAKEDCLERR(DCLLOCALE_FILE, 14))
#define DCLSTAT_FS_WRITETOOLONG             (MAKEDCLERR(DCLLOCALE_FILE, 15))
#define DCLSTAT_FS_LASTENTRY                (MAKEDCLERR(DCLLOCALE_FILE, 16))
#define DCLSTAT_FS_TOOMANY                  (MAKEDCLERR(DCLLOCALE_FILE, 17))
#define DCLSTAT_FS_BADCMD                   (MAKEDCLERR(DCLLOCALE_FILE, 18))
#define DCLSTAT_FS_UNMOUNTED                (MAKEDCLERR(DCLLOCALE_FILE, 19))
#define DCLSTAT_FS_UNFORMATTED              (MAKEDCLERR(DCLLOCALE_FILE, 20))
#define DCLSTAT_FS_BLOCKSIZE                (MAKEDCLERR(DCLLOCALE_FILE, 21))
#define DCLSTAT_FS_NAMELENGTH               (MAKEDCLERR(DCLLOCALE_FILE, 22))
#define DCLSTAT_FS_NOPROCESS                (MAKEDCLERR(DCLLOCALE_FILE, 23))
#define DCLSTAT_FS_READFAILED               (MAKEDCLERR(DCLLOCALE_FILE, 24))
#define DCLSTAT_FS_WRITEFAILED              (MAKEDCLERR(DCLLOCALE_FILE, 25))
#define DCLSTAT_FS_CLOSEFAILED              (MAKEDCLERR(DCLLOCALE_FILE, 26))
#define DCLSTAT_FS_DELETEFAILED             (MAKEDCLERR(DCLLOCALE_FILE, 27))
#define DCLSTAT_FS_SEEKFAILED               (MAKEDCLERR(DCLLOCALE_FILE, 28))
#define DCLSTAT_FS_STATFAILED               (MAKEDCLERR(DCLLOCALE_FILE, 29))
#define DCLSTAT_FS_STATFSFAILED             (MAKEDCLERR(DCLLOCALE_FILE, 30))
#define DCLSTAT_FS_RENAMEFAILED             (MAKEDCLERR(DCLLOCALE_FILE, 31))
#define DCLSTAT_FS_TRUNCATEFAILED           (MAKEDCLERR(DCLLOCALE_FILE, 32))
#define DCLSTAT_FS_READONLY                 (MAKEDCLERR(DCLLOCALE_FILE, 33))
#define DCLSTAT_FS_BADORIGIN                (MAKEDCLERR(DCLLOCALE_FILE, 34))

#define DCLSTAT_FS_DIRNOTEMPTY              (MAKEDCLERR(DCLLOCALE_FILE, 80))
#define DCLSTAT_FS_DIRCREATEFAILED          (MAKEDCLERR(DCLLOCALE_FILE, 81))
#define DCLSTAT_FS_DIRREMOVEFAILED          (MAKEDCLERR(DCLLOCALE_FILE, 82))
#define DCLSTAT_FS_DIRCHANGEFAILED          (MAKEDCLERR(DCLLOCALE_FILE, 83))
#define DCLSTAT_FS_DIROPENFAILED            (MAKEDCLERR(DCLLOCALE_FILE, 84))
#define DCLSTAT_FS_DIRCLOSEFAILED           (MAKEDCLERR(DCLLOCALE_FILE, 85))
#define DCLSTAT_FS_DIRREADFAILED            (MAKEDCLERR(DCLLOCALE_FILE, 86))
#define DCLSTAT_FS_DIRRENAMEFAILED          (MAKEDCLERR(DCLLOCALE_FILE, 87))
#define DCLSTAT_FS_DIRREADONLY              (MAKEDCLERR(DCLLOCALE_FILE, 88))

/*  Log operation codes
*/
#define DCLSTAT_LOG_OPENFAILED              (MAKEDCLERR(DCLLOCALE_LOG, 1))
#define DCLSTAT_LOG_NOWRITEDATA             (MAKEDCLERR(DCLLOCALE_LOG, 2))
#define DCLSTAT_LOG_NOTFOUND                (MAKEDCLERR(DCLLOCALE_LOG, 3))
#define DCLSTAT_LOG_BADHANDLE               (MAKEDCLERR(DCLLOCALE_LOG, 4))
#define DCLSTAT_LOG_BADPARAMS               (MAKEDCLERR(DCLLOCALE_LOG, 5))
#define DCLSTAT_LOG_NOWRITELENGTH           (MAKEDCLERR(DCLLOCALE_LOG, 6))

/*  Shell status codes
*/
#define DCLSTAT_SHELLDISABLED               (MAKEDCLERR(DCLLOCALE_SHELL, 1))
#define DCLSTAT_SHELLCREATEFAILED           (MAKEDCLERR(DCLLOCALE_SHELL, 2))
#define DCLSTAT_SHELLCOMMANDNOTFOUND        (MAKEDCLERR(DCLLOCALE_SHELL, 3))
#define DCLSTAT_SHELLINPUTFAILURE           (MAKEDCLERR(DCLLOCALE_SHELL, 4))
#define DCLSTAT_SHELLSYNTAXERROR            (MAKEDCLERR(DCLLOCALE_SHELL, 5))

/*  Environment status codes
*/
#define DCLSTAT_ENV_SUBSTSYNTAX             (MAKEDCLERR(DCLLOCALE_ENV, 1))
#define DCLSTAT_ENV_SUBSTOVERFLOW           (MAKEDCLERR(DCLLOCALE_ENV, 2))
#define DCLSTAT_ENV_CREATEFAILED            (MAKEDCLERR(DCLLOCALE_ENV, 3))
#define DCLSTAT_ENV_SUBSTPARAMS             (MAKEDCLERR(DCLLOCALE_ENV, 4))

/*  Driver Framework status codes
*/
#define DCLSTAT_DRVFW_NULLPOINTER           (MAKEDCLERR(DCLLOCALE_DRIVERFW,  1))
#define DCLSTAT_DRVFW_NULLHANDLE            (MAKEDCLERR(DCLLOCALE_DRIVERFW,  2))
#define DCLSTAT_DRVFW_BADHANDLE             (MAKEDCLERR(DCLLOCALE_DRIVERFW,  3))
#define DCLSTAT_DRVFW_INSTANCEOUTOFRANGE    (MAKEDCLERR(DCLLOCALE_DRIVERFW,  4))
#define DCLSTAT_DRVFW_INSTANCEALREADYINITED (MAKEDCLERR(DCLLOCALE_DRIVERFW,  5))
#define DCLSTAT_DRVFW_SERVICEOUTOFRANGE     (MAKEDCLERR(DCLLOCALE_DRIVERFW,  6))
#define DCLSTAT_DRVFW_SERVICEALREADYCREATED (MAKEDCLERR(DCLLOCALE_DRIVERFW,  7))
#define DCLSTAT_DRVFW_SERVICENOTREGISTERED  (MAKEDCLERR(DCLLOCALE_DRIVERFW,  8))
#define DCLSTAT_DRVFW_SERVICENOTLINKED      (MAKEDCLERR(DCLLOCALE_DRIVERFW,  9))
#define DCLSTAT_DRVFW_SERVICEHEADERBAD      (MAKEDCLERR(DCLLOCALE_DRIVERFW, 10))
#define DCLSTAT_DRVFW_SERVICELIMITEXCEEDED  (MAKEDCLERR(DCLLOCALE_DRIVERFW, 11))
#define DCLSTAT_DRVFW_SERVICECREATEBAD      (MAKEDCLERR(DCLLOCALE_DRIVERFW, 12))
#define DCLSTAT_DRVFW_SERVICECHAINBAD       (MAKEDCLERR(DCLLOCALE_DRIVERFW, 13))

/*  PerfLog status codes
*/
#define DCLSTAT_PERFLOG_TRUNCATED           (MAKEDCLERR(DCLLOCALE_PERFLOG,  1))
#define DCLSTAT_PERFLOG_OPENFAILED          (MAKEDCLERR(DCLLOCALE_PERFLOG,  2))

/*  Miscellaneous status codes
*/
#define DCLSTAT_BADMULTIBYTECHAR            (MAKEDCLERR(DCLLOCALE_MISC,    1))
#define DCLSTAT_SLEEPLOCK_TIMEOUT           (MAKEDCLERR(DCLLOCALE_MISC,    2))
#define DCLSTAT_INPUT_DISABLED              (MAKEDCLERR(DCLLOCALE_MISC,   50))
#define DCLSTAT_INPUT_IDLE                  (MAKEDCLERR(DCLLOCALE_MISC,   51))
#define DCLSTAT_INPUT_TERMINATE             (MAKEDCLERR(DCLLOCALE_MISC,   52))
#define DCLSTAT_TLS_DATAPOINTERINVALID      (MAKEDCLERR(DCLLOCALE_MISC,  100))
#define DCLSTAT_FAT_UNRECOGNIZEDSYSID       (MAKEDCLERR(DCLLOCALE_MISC,  200))
#define DCLSTAT_FAT_UNRECOGNIZEDBPB         (MAKEDCLERR(DCLLOCALE_MISC,  201))
#define DCLSTAT_FAT_UNSUPPORTEDBYTESPERSEC  (MAKEDCLERR(DCLLOCALE_MISC,  202))
#define DCLSTAT_CMDLINE_BADSYNTAX           (MAKEDCLERR(DCLLOCALE_MISC,  300))
#define DCLSTAT_CMDLINE_BADOPTION           (MAKEDCLERR(DCLLOCALE_MISC,  301))
#define DCLSTAT_CMDLINE_OPTIONDUPLICATED    (MAKEDCLERR(DCLLOCALE_MISC,  302))
#define DCLSTAT_CMDLINE_OPTIONMISSING       (MAKEDCLERR(DCLLOCALE_MISC,  303))
#define DCLSTAT_CMDLINE_OPTIONMALFORMED     (MAKEDCLERR(DCLLOCALE_MISC,  304))
#define DCLSTAT_CMDLINE_OUTOFRANGE          (MAKEDCLERR(DCLLOCALE_MISC,  305))
#define DCLSTAT_CMDLINE_OUTOFRANGEPOW2      (MAKEDCLERR(DCLLOCALE_MISC,  306))
#define DCLSTAT_CMDLINE_MISMATCHEDQUOTES    (MAKEDCLERR(DCLLOCALE_MISC,  307))
#define DCLSTAT_CMDLINE_STRINGEMPTY         (MAKEDCLERR(DCLLOCALE_MISC,  308))
#define DCLSTAT_CMDLINE_STRINGTOOLONG       (MAKEDCLERR(DCLLOCALE_MISC,  309))
#define DCLSTAT_CMDLINE_BADDEFINITION       (MAKEDCLERR(DCLLOCALE_MISC,  310))
#define DCLSTAT_SYNC_WRITEINTERRUPTEDWRITE  (MAKEDCLERR(DCLLOCALE_MISC,  350))
#define DCLSTAT_SYNC_WRITEINTERRUPTEDREAD   (MAKEDCLERR(DCLLOCALE_MISC,  351))
#define DCLSTAT_SYNC_READINTERRUPTEDWRITE   (MAKEDCLERR(DCLLOCALE_MISC,  352))
#define DCLSTAT_SYNC_READENDCRCINVALID      (MAKEDCLERR(DCLLOCALE_MISC,  353))
#define DCLSTAT_MBR_SIGNATUREINVALID        (MAKEDCLERR(DCLLOCALE_MISC,  370))

/*  Tick, Time, and Timestamp status codes
*/
#define DCLSTAT_TIME_HRTICKRESOLUTION       (MAKEDCLERR(DCLLOCALE_TIME,    1))
#define DCLSTAT_TIME_HRTICKNONFUNCTIONAL    (MAKEDCLERR(DCLLOCALE_TIME,    2))
#define DCLSTAT_TIMER_INSTANCEHANDLEFAILED  (MAKEDCLERR(DCLLOCALE_TIME,  100))
#define DCLSTAT_TIMER_SERVICEINITFAILED     (MAKEDCLERR(DCLLOCALE_TIME,  101))

/*  Windows status codes
*/
#define DCLSTAT_WIN_CLOSEHANDLEFAILED         (MAKEDCLERR(DCLLOCALE_WIN,    1))
#define DCLSTAT_WIN_COMMCLOSEFAILED           (MAKEDCLERR(DCLLOCALE_WIN,  100))
#define DCLSTAT_WIN_COMMGETPARAMSFAILED       (MAKEDCLERR(DCLLOCALE_WIN,  101))
#define DCLSTAT_WIN_REGKEYNOTFOUND            (MAKEDCLERR(DCLLOCALE_WIN,  201))
#define DCLSTAT_WIN_REGBADDEFAULTVALUE        (MAKEDCLERR(DCLLOCALE_WIN,  202))
#define DCLSTAT_WIN_REGBADSETTINGSVALUE       (MAKEDCLERR(DCLLOCALE_WIN,  203))
#define DCLSTAT_WIN_REGINVALIDTYPE            (MAKEDCLERR(DCLLOCALE_WIN,  204))
#define DCLSTAT_WIN_REGSETVALUEFAILED         (MAKEDCLERR(DCLLOCALE_WIN,  205))
#define DCLSTAT_WINGUI_REGISTERCLASSFAILED    (MAKEDCLERR(DCLLOCALE_WIN,  501))
#define DCLSTAT_WINGUI_CREATEWINDOWFAILED     (MAKEDCLERR(DCLLOCALE_WIN,  502))
#define DCLSTAT_WINGUI_LOADCMNCONTROLSAILED   (MAKEDCLERR(DCLLOCALE_WIN,  503))
#define DCLSTAT_WINGUI_GETPROCADDRESSFAILED   (MAKEDCLERR(DCLLOCALE_WIN,  504))
#define DCLSTAT_WINGUI_CMDBARCREATEFAILED     (MAKEDCLERR(DCLLOCALE_WIN,  505))
#define DCLSTAT_WINGUI_CMDBARMENUINSERTFAILED (MAKEDCLERR(DCLLOCALE_WIN,  506))
#define DCLSTAT_WINGUI_CMDBARADORNMENTSFAILED (MAKEDCLERR(DCLLOCALE_WIN,  507))
#define DCLSTAT_WINGUI_LOADMENUFAILED         (MAKEDCLERR(DCLLOCALE_WIN,  508))

/*  Block Device
*/
#define DCLSTAT_BLOCKDEV_BADACCESS              (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 1))
#define DCLSTAT_BLOCKDEV_UNKNOWNDEVICE          (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 2))
#define DCLSTAT_BLOCKDEV_NOTOPEN                (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 3))
#define DCLSTAT_BLOCKDEV_NOMOREDEVICES          (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 4))
#define DCLSTAT_BLOCKDEV_ACCESSDENIED           (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 5))
#define DCLSTAT_BLOCKDEV_UNSUPPORTEDREQUEST     (MAKEDCLERR(DCLLOCALE_BLOCKDEV, 6))
 
#endif  /* DLSTATUS_H_INCLUDED */
