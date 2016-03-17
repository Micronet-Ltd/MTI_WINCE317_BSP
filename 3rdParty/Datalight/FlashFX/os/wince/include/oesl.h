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

    This header contains information defining the default characteristics
    of the Windows CE OS Layer.  Some aspects of this information can be
    modified by overriding it in ffxconf.h.

    This header file is included after ffxconf.h, but before fxmacros.h,
    therefore it may expect that only those settings which are required
    in ffxconf.h for the given OS are set.  Settings which are not required
    in the ffxconf.h, and are set to defaults by fxmacros.h will NOT be
    usable in this header file.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oesl.h $
    Revision 1.14  2009/07/22 01:19:07Z  garyp
    Merged from the v4.0 branch.  Added a default FFXCONF_USESECTORINFO
    setting.  Added support for the FFXCONF_POWERSUSPENDRESUME
    feature.
    Revision 1.13  2008/12/17 00:21:07Z  keithg
    Removed structure packing directive. (bug 150)
    Revision 1.12  2008/03/26 19:30:51Z  Garyp
    Updated to default to using background compaction only if thread support
    and allocator support is enabled, and to use synchronous (none) if it is
    not.  Turn FAT Monitor off since it is not needed on CE 4.2 or later.
    Revision 1.11  2008/01/31 21:08:43Z  Garyp
    Removed the obsolete and unused FFX_IO_MAPPED setting and IOPORTADDRESS
    type.
    Revision 1.10  2008/01/17 04:08:04Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.9  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2006/10/16 20:58:24Z  Garyp
    Refactored how MBR settings are specified.  Removed some obsolete settings.
    Revision 1.7  2006/10/12 22:19:18Z  Garyp
    Use refactored configuration settings.
    Revision 1.6  2006/10/06 00:43:33Z  Garyp
    Removed an obsolete setting.
    Revision 1.5  2006/10/06 00:24:16Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.4  2006/05/06 22:41:45Z  Garyp
    Removed an obsolete prototype.
    Revision 1.3  2006/01/12 02:22:20Z  Garyp
    Documentation updated.
    Revision 1.2  2005/12/31 03:06:55Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.1  2005/12/07 06:44:38Z  Pauli
    Initial revision
    Revision 1.4  2005/12/07 06:44:37Z  Garyp
    Updated to use new FFX_COMPACTIONMODEL settings.
    Revision 1.3  2005/12/04 21:07:12Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.2  2005/11/14 20:47:40Z  Garyp
    Eliminated "Single-threaded-FIM" settings.
    Revision 1.1  2005/10/01 17:55:00Z  Garyp
    Initial revision
    Revision 1.31  2005/06/17 21:30:55Z  PaulI
    Removed D_CHAR and D_UCHAR type definitions.  These are now defined in DCL.
    Revision 1.30  2005/06/16 16:43:13Z  PaulI
    Removed definition of D_BOOL.  This is now defined in DCL.
    Revision 1.29  2005/02/02 06:57:25Z  GaryP
    Set FFXCONF_FORCEALIGNEDIO to default to TRUE, but to honor any preexisting
    setting in ffxconf.h.
    Revision 1.28  2005/01/28 21:54:18Z  GaryP
    Eliminated the OS target information.  Now exists in the ostypes.h file in
    the DCL project.
    Revision 1.27  2004/12/31 00:26:36Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.26  2004/12/16 08:48:48Z  garys
    Added TOUPPER and TOLOWER macros to allow typecasting for gnu.
    Revision 1.25  2004/11/10 20:37:17  GaryP
    Added function prototypes for oecompat.c.
    Revision 1.24  2004/09/29 06:49:52Z  GaryP
    More ffx/pro release changes.
    Revision 1.23  2004/09/29 02:09:53Z  GaryP
    General cleanup of configuration settings in preparation for FFX/Pro release.
    Revision 1.22  2004/09/23 06:14:33Z  GaryP
    Refactored for FFX/Pro release.
    Revision 1.21  2004/08/25 07:09:20Z  GaryP
    Added the background garbage collection setting.
    Revision 1.20  2004/08/11 19:26:52Z  GaryP
    Removed misplaced validation logic.
    Revision 1.19  2004/08/10 18:32:01Z  GaryP
    Reorganized the settings and added validity checks.
    Revision 1.18  2004/08/06 22:14:28Z  GaryP
    Organizational change.
    Revision 1.17  2004/07/26 17:01:00Z  jaredw
    Added DCL_OSFEATURE_THREADS.
    Revision 1.16  2004/07/23 23:43:30Z  GaryP
    Added support for specifying a default file system.
    Revision 1.15  2004/07/22 21:07:02Z  jaredw
    Removed 64bit data type.
    Revision 1.14  2004/04/28 01:38:49Z  garyp
    Added the FFX_THREADNAMELEN setting.
    Revision 1.13  2004/02/28 00:40:40Z  garys
    Merge from FlashFXMT
    Revision 1.8.1.6  2004/02/28 00:40:40  garyp
    Added some missing settings.
    Revision 1.8.1.5  2004/01/24 23:05:08Z  garyp
    Updated to use the new FFXCONF_... style configuration settings.
    Revision 1.8.1.4  2004/01/03 06:32:52Z  garyp
    Added D_INT64 and D_UINT64 data types.
    Revision 1.8.1.3  2003/11/13 01:43:54Z  garyp
    Changed FFX_ALIGNSIZE to FFX_ALIGNTYPE.
    Revision 1.8.1.2  2003/11/03 16:44:06Z  garyp
    Re-checked into variant sandbox.
    Revision 1.9  2003/11/03 16:44:06Z  garyp
    Eliminated the __D_FAR definition.
    Revision 1.8  2003/04/21 01:05:48Z  garyp
    Removed "signed" from the basic types.
    Revision 1.7  2003/04/13 04:33:06Z  garyp
    Eliminated __USE_32BIT and added FFX_ALIGNSIZE.
    Revision 1.6  2003/03/27 18:35:16Z  garyp
    Added the FFX_OSNAME and FFX_IO_MAPPED settings.
    Revision 1.5  2002/11/25 03:15:16Z  garyp
    Changed LITTLE_ENDIAN to FFX_LITTLE_ENDIAN and eliminated the use of #ifdef
    to examine it.
    Revision 1.4  2002/11/22 23:49:50Z  garyp
    Cleaned up and eliminated some obsolete stuff.
    Revision 1.3  2002/11/07 18:29:12Z  garyp
    Fixed FFX_UNICODE_SUPPORT to be defined as 0 or 1 instead of TRUE or FALSE.
    Revision 1.2  2002/11/07 11:22:32Z  garyp
    Added Unicode support.
---------------------------------------------------------------------------*/
#ifndef OESL_H_INCLUDED
#define OESL_H_INCLUDED


                /********************************\
                 *                              *
                 *   Default settings (may be   *
                 *   overridden in ffxconf.h)   *
                 *                              *
                \********************************/


/*-------------------------------------------------------------------
    Set this value to FALSE in the ffxconf.h file to disable
    requiring that I/O be aligned on DCL_ALIGNSIZE boundaries.
    Only disable this when using a FIM and processor architecture
    that does not require alignment.
-------------------------------------------------------------------*/
#ifndef FFXCONF_FORCEALIGNEDIO
#define FFXCONF_FORCEALIGNEDIO      TRUE
#endif


/*-------------------------------------------------------------------
    FFX_COMPACTIONMODEL defines the type of compaction model the
    system is configured to use.  Legal values are:

    FFX_COMPACT_NONE             - The default model where there are
                                   no parallel compaction operations
                                   (they are performed synchronously
                                   with writes).
    FFX_COMPACT_BACKGROUNDIDLE   - Compaction is performed in the
                                   background by explicit calls to
                                   FfxDriverCompactIfIdle().
    FFX_COMPACT_BACKGROUNDTHREAD - Compaction is performed in the
                                   background by a separate thread.
                                   Requires DCL_OSFEATURE_THREADS to
                                   be TRUE.

    If no compaction model is specified in the ffxconf.h file, the
    default will be FFX_COMPACT_BACKGROUNDTHREAD if allocator and
    thread support are enabled, and FFX_COMPACT_NONE if not.
-------------------------------------------------------------------*/
#ifndef FFX_COMPACTIONMODEL
  #if FFXCONF_ALLOCATORSUPPORT && DCL_OSFEATURE_THREADS
    #define FFX_COMPACTIONMODEL     FFX_COMPACT_BACKGROUNDTHREAD
  #else
    #define FFX_COMPACTIONMODEL     FFX_COMPACT_NONE
  #endif
#endif


/*-------------------------------------------------------------------
    FFXCONF_POWERSUSPENDRESUME controls whether the API for
    providing notifications for power suspend and resume is
    included or not.
-------------------------------------------------------------------*/
#ifndef FFXCONF_POWERSUSPENDRESUME
#define FFXCONF_POWERSUSPENDRESUME  TRUE
#endif


/*-------------------------------------------------------------------
    Define whether FlashFX has any knowledge of what an MBR is.
    Set this here if it is not already initialized in ffxconf.h.
-------------------------------------------------------------------*/
#ifndef FFXCONF_MBRSUPPORT
#define FFXCONF_MBRSUPPORT          TRUE
#endif

#if FFXCONF_MBRSUPPORT && !defined(FFXCONF_MBRFORMAT)
#define FFXCONF_MBRFORMAT           FFXCONF_FORMATSUPPORT
#endif


/*-------------------------------------------------------------------
    Set FFXCONF_USESECTORINFO to TRUE or FALSE to enable or disable
    support for the use of SectorInfo in the FMD and the FlashFX DLL.
    Note that this setting must be the same for both the Bootloader
    and the OS driver projects.  (This setting does not normally
    need to be changed, as SectorInfo is not required.)
-------------------------------------------------------------------*/
#ifndef FFXCONF_USESECTORINFO
#define FFXCONF_USESECTORINFO       FALSE
#endif




                /********************************\
                 *                              *
                 *      Constant settings       *
                 *      for this OS             *
                 *                              *
                \********************************/



/*-------------------------------------------------------------------
    FAT Monitor support is only needed for CE versions prior to
    version 4.2, which don't officially support any more.
-------------------------------------------------------------------*/
#define FFXCONF_FATMONITORSUPPORT   FALSE


/*-------------------------------------------------------------------
    Misc settings
-------------------------------------------------------------------*/


#endif /* OESL_H_INCLUDED */


