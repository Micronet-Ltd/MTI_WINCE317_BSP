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

    This header contains information defining the default characteristics
    of the Loader interface.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlosconf.h $
    Revision 1.6  2009/06/25 00:59:43Z  garyp
    Added the DCL_OS_DEFAULT_SERVICES setting.
    Revision 1.5  2009/05/23 17:31:46Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.4  2009/02/12 20:27:50Z  garyp
    Minor cleanup of how the OS requestor characteristics are defined.
    Revision 1.3  2008/08/20 01:02:37Z  keithg
    Added description of how to map DCL C library functions to the
    native implementations if desired.
    Revision 1.2  2007/11/03 23:31:25Z  Garyp
    Added the standard module header.
    Revision 1.1  2007/10/10 03:07:16Z  brandont
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLOSCONF_H_INCLUDED
#define DLOSCONF_H_INCLUDED

#include <string.h>             /* memcpy(), etc */


/*-------------------------------------------------------------------
    Port specific string and memory function mappings

    For the loader environment, all DCL string and memory
    functions are left to their DCL default mappings.
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    If any DCL C library replacements need to be overriden for
    performance or compatibility reasons, do that here.

    The general form to use follows:

    #ifndef DclMemCmp
    #define DclMemCmp(_p1, _p2, _len)   memcmp(_p1, _p2, _len)
    #endif
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Define the target OS name and features.
-------------------------------------------------------------------*/
#define DCL_OSTARGET_LOADER             (1)
#define DCL_OSNAME                      "Loader"

#define DCL_OSFEATURE_UNICODE           FALSE


/*-------------------------------------------------------------------
    The threads feature is used to determine whether this particular
    RTOS has a threading model or not, and the default value specified
    here may be preempted by defining it in dclconf.h.
-------------------------------------------------------------------*/
#ifndef DCL_OSFEATURE_THREADS
#define DCL_OSFEATURE_THREADS           FALSE
#endif

#ifndef DCL_OSFEATURE_CONSOLEINPUT
#define DCL_OSFEATURE_CONSOLEINPUT      TRUE
#endif

#if DCL_OSFEATURE_UNICODE
typedef unsigned short D_WCHAR;
#endif


/*-------------------------------------------------------------------
    Default Services

    This defines the default set of services to automatically
    initialize for a project based on this OS.  This value is
    typically used in dclconf.h to avoid having to duplicate
    this list in numerous projects.

    Note that even for features which are disabled at compile-time,
    the "init" function still may get called.
-------------------------------------------------------------------*/
#define DCL_OS_DEFAULT_SERVICES         DCL_DEFAULT_SERVICES


/*-------------------------------------------------------------------
    Set the default DCL_ALIGNTYPE/SIZE values if they were not
    already initialized in dclconf.h.

    These values define data alignment boundaries as required by
    the operating system and enforced by the compiler.  This value
    is used by Datalight software as the minimum boundary on which
    declared structures and allocated memory must be aligned.

    DCL_ALIGNSIZE may not be greater than the alignment enforced by
    the compiler when allotting space for the maximum size native
    type (D_UINT32 or D_UINT64).  There is no practical reason for
    this value to ever be less than 4 since all the modern compilers
    Datalight currently deals with ensure that 32-bit values are
    always aligned, even when using processor such as x86 which does
    not require it.

    Note that we can't use sizeof to calculate DCL_ALIGNSIZE because
    we must be able to use this value at preprocessor time.
-------------------------------------------------------------------*/
#ifndef DCL_ALIGNSIZE
    #if DCL_NATIVE_64BIT_SUPPORT && DCL_CPU_ALIGNSIZE > 4
        #define DCL_ALIGNSIZE           (8)
        typedef D_UINT64                DCL_ALIGNTYPE;
    #else
        #define DCL_ALIGNSIZE           (4)
        typedef D_UINT32                DCL_ALIGNTYPE;
    #endif
#endif


/*-------------------------------------------------------------------
    Object name lengths
-------------------------------------------------------------------*/
#define DCL_MUTEXNAMELEN                (12)
#define DCL_SEMAPHORENAMELEN            (12)
#define DCL_THREADNAMELEN               (12)


/*-------------------------------------------------------------------
    Define the default path separator character to use.  This does
    not imply that other path separator characters are not legal,
    just that this will be the default character used.
-------------------------------------------------------------------*/
#define DCL_PATHSEPCHAR                 '\\'
#define DCL_PATHSEPSTR                  "\\"


/*-------------------------------------------------------------------
    DCLTHREADID abstraction.
-------------------------------------------------------------------*/
typedef D_UINT32                        DCLTHREADID;


/*-------------------------------------------------------------------
    External Requestor Interface

    DCL_IOCTL_MAX_REQUESTS - Specifies the maximum number of devices
    which may be opened via the external requestor interface.  This
    is typically set to zero in environments which do not support a
    requestor interface.  The default value for this setting is
    specified in the OS-specific dlosconf.h file, but may be
    superseded at the project level by defining it in dclconf.h.

    DCL_IOCTL_FILESYS - The OS specific IOCTL number used to send
    requests to a file system.  The default value for this setting
    is specified in the OS-specific dlosconf.h file, but may be
    superseded at the project level by defining it in dclconf.h.

    DCL_IOCTL_BLOCKDEV - The OS specific IOCTL number used to send
    requests to a block device.  The default value for this setting
    is specified in the OS-specific dlosconf.h file, but may be
    superseded at the project level by defining it in dclconf.h.
-------------------------------------------------------------------*/
#ifndef DCL_IOCTL_MAX_REQUESTS
#define DCL_IOCTL_MAX_REQUESTS          (0)     /* Disabled for this OS */
#endif

#ifndef DCL_IOCTL_FILESYS
#define DCL_IOCTL_FILESYS               (0)
#endif

#ifndef DCL_IOCTL_BLOCKDEV
#define DCL_IOCTL_BLOCKDEV              (0)
#endif


#endif  /* DLOSCONF_H_INCLUDED */
