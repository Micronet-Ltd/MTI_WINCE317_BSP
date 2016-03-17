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

    This header is used to configure project specific settings for the
    Datalight Common Libraries (DCL).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dclconf.h $
    Revision 1.6  2009/07/19 02:25:18Z  garyp
    Merged from the v4.0 branch.  Added the DCL_SERVICES setting.
    Revision 1.5  2007/12/14 23:15:35Z  Garyp
    Added Datalight Shell settings.
    Revision 1.4  2007/11/03 23:50:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/05/29 23:27:22Z  rickc
    Added DCLCONF_MUTEXTRACKING
    Revision 1.2  2007/03/13 00:06:58Z  rickc
    Added DSL output constants
    Revision 1.1  2007/02/21 02:18:28Z  rickc
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DCLCONF_H_INCLUDED
#define DCLCONF_H_INCLUDED


/*-------------------------------------------------------------------
    Messages and Debug Output

    Set DCLCONF_OUTPUT_ENABLED to FALSE to prevent the inclusion of
    any code which is used to display messages (regardless whether
    building in DEBUG or RELEASE mode).  Typically this would only
    be set to FALSE in the final stages of production.

    DCLOUTPUT_BUFFERLEN defines the maximum length of the internal
    buffers used to manipulate messages.  Messages exceeding this
    length may be truncated.  Increasing this value allows longer
    messages to be processed, but increases stack usage.

    If output is enabled, DCLOUTPUT_THREADID and DCLOUTPUT_TIMESTAMP
    control whether each message is marked with the current thread
    ID and time stamp.
-------------------------------------------------------------------*/
#define DCLCONF_OUTPUT_ENABLED          TRUE

#if DCLCONF_OUTPUT_ENABLED
  #define DCLOUTPUT_BUFFERLEN           (256)
  #define DCLOUTPUT_THREADID            FALSE
  #define DCLOUTPUT_TIMESTAMP           FALSE
#endif


/*-------------------------------------------------------------------
    Datalight Shell Settings

    Set these values to TRUE to enable the Datalight interactive
    command shell.

    The "tests" and "tools" settings control whether the shell
    includes the various tests and tools which are available
    both in DCL and in the products which use DCL.
-------------------------------------------------------------------*/
#define DCLCONF_COMMAND_SHELL           TRUE

#if DCLCONF_COMMAND_SHELL
  #define DCLCONF_SHELL_TESTS           TRUE
  #define DCLCONF_SHELL_TOOLS           TRUE
#endif


/*-------------------------------------------------------------------
    Tracking Settings

    Enable the various tracking settings to allow statistical and
    diagnostic output to be gathered and displayed.

    These settings typically default to being disabled when building
    a release version of the product.
-------------------------------------------------------------------*/
#if D_DEBUG == 0
  #define DCLCONF_MEMORYTRACKING        FALSE   /* MUST be FALSE for checkin    */
  #define DCLCONF_MUTEXTRACKING         FALSE   /* MUST be FALSE for checkin    */
  #define DCLCONF_SEMAPHORETRACKING     FALSE   /* MUST be FALSE for checkin    */
#else
  #define DCLCONF_MEMORYTRACKING         TRUE   /* Should be TRUE for checkin   */
  #define DCLCONF_MUTEXTRACKING         FALSE   /* Should be FALSE for checkin  */
  #define DCLCONF_SEMAPHORETRACKING     FALSE   /* Should be FALSE for checkin  */
#endif


/*-------------------------------------------------------------------
    Trace Settings

    DCLTRACEMASK is a bit-mapped mask of DCLTRACEBIT_class values
    that filter debug output created by the DCLTRACEPRINTF() macros
    used throughout DCL.

    For example, to enable tracing for memory management and
    mutexes, you would use the following statement:

        #define DCLTRACEMASK    (DCLTRACEBIT_MEM | DCLTRACEBIT_MUTEX)

    See dltrace.h for a list of the available trace classes.

    The DCLTRACE_AUTOENABLE setting controls whether tracing is
    initially enabled or disabled at startup time.  This setting is
    system-wide, and affects any products that include DCL.
-------------------------------------------------------------------*/
#define DCLTRACEMASK                    DCLTRACEBIT_NONE

#define DCLTRACE_AUTOENABLE             TRUE


/*-------------------------------------------------------------------
    Profiler Settings
-------------------------------------------------------------------*/
#define DCLCONF_PROFILERENABLED         FALSE

#if DCLCONF_PROFILERENABLED
  #define DCLPROF_AUTOENABLE            FALSE
  #define DCLPROF_MAX_RECORDS          (7000)
  #define DCLPROF_MAX_NESTING_LEVEL      (40)
  #define DCLPROF_MAX_THREADS            (10)
#endif


/*-------------------------------------------------------------------
    High-Res Timestamp

    Set this value to TRUE to map the regular millisecond based
    timestamp functions onto the the high-res timestamp functions.

    This may ONLY be done if the high-res timestamp functions are
    implemented (oshrtick.c), and have a period long enough to be
    meaningful (some high-res timers wrap too fast to be useful).
-------------------------------------------------------------------*/
#define DCLCONF_HIGHRESTIMESTAMP        FALSE


/*-------------------------------------------------------------------
    DCL Project Services

    DCL_SERVICES defines the set of services to create when a
    project is initialized.  Each entry is a function pointer to
    the initializer function for a given service.  Services are
    created in the order specified, and at shutdown time, are 
    destroyed in reverse order.

    The predefined macro "DCL_OS_DEFAULT_SERVICES" is defined to
    the default set of services for each supported OS environment,
    and can be found in the appropriate dlosconf.h.

    OEMs can modify this list to add or remove Datalight services,
    or add OEM specific services.
-------------------------------------------------------------------*/
#define DCL_SERVICES                    {DCL_OS_DEFAULT_SERVICES}



#endif  /* DCLCONF_H_INCLUDED */

