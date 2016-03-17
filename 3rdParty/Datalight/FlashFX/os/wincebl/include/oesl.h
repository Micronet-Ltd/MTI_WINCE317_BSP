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

    This header contains information defining the default characteristics of
    the Windows CE Boot Loader OS Layer.  Some aspects of this information
    can be modified by overriding it in ffxconf.h.

    This header file is included after ffxconf.h, but before fxmacros.h,
    therefore it may expect that only those settings which are required
    in ffxconf.h for the given OS are set.  Settings which are not required
    in the ffxconf.h, and are set to defaults by fxmacros.h will NOT be
    usable in this header file.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oesl.h $
    Revision 1.17  2009/07/24 02:37:54Z  garyp
    Merged from the v4.0 branch.  Added a default FFXCONF_USESECTORINFO
    setting.  Added support for the FFXCONF_POWERSUSPENDRESUME feature.
    Revision 1.16  2008/12/17 00:21:11Z  keithg
    Removed structure packing directive. (bug 150)
    Revision 1.15  2008/03/26 19:30:54Z  Garyp
    Modified so FAT Monitor support is always disabled.
    Revision 1.14  2008/01/31 21:08:43Z  Garyp
    Removed the obsolete and unused FFX_IO_MAPPED setting and IOPORTADDRESS
    type.
    Revision 1.13  2008/01/17 04:08:04Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.12  2007/11/03 23:50:17Z  Garyp
    Updated to use the standard module header.
    Revision 1.11  2006/10/16 20:58:22Z  Garyp
    Refactored how MBR settings are specified.  Removed some obsolete settings.
    Revision 1.10  2006/10/12 22:51:09Z  Garyp
    Use refactored configuration settings.
    Revision 1.9  2006/10/06 00:43:35Z  Garyp
    Removed an obsolete setting.
    Revision 1.8  2006/10/06 00:24:34Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.7  2006/08/04 15:28:48Z  timothyj
    Removed FFXCONF_DRIVERAUTOTEST definition (was moved to ffxconf.h)
    Revision 1.6  2006/06/29 20:58:42Z  rickc
    Added ability to override FFXCONF_DRIVERAUTOFORMAT default value
    Revision 1.5  2006/05/17 18:32:15Z  Garyp
    Removed the FFX_USEMBR setting -- this is specified in ffxconf.h.
    Revision 1.4  2006/05/06 22:41:45Z  Garyp
    Removed an obsolete prototype.
    Revision 1.3  2006/01/12 02:22:17Z  Garyp
    Documentation updated.
    Revision 1.2  2005/12/31 03:06:55Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.1  2005/12/07 06:44:36Z  Pauli
    Initial revision
    Revision 1.3  2005/12/07 06:44:36Z  Garyp
    Updated to use new FFX_COMPACTIONMODEL settings.
    Revision 1.2  2005/11/14 20:47:41Z  Garyp
    Eliminated "Single-threaded-FIM" settings.
    Revision 1.1  2005/10/01 17:55:02Z  Garyp
    Initial revision
    Revision 1.12  2005/06/17 21:31:09Z  PaulI
    Removed D_CHAR and D_UCHAR type definitions.  These are now defined in DCL.
    Revision 1.11  2005/06/16 16:43:56Z  PaulI
    Removed definition of D_BOOL.  This is now defined in DCL.
    Revision 1.10  2005/02/02 06:57:25Z  GaryP
    Set FFXCONF_FORCEALIGNEDIO to default to TRUE, but to honor
    any preexisting setting in ffxconf.h.
    Revision 1.9  2005/01/28 21:54:37Z  GaryP
    Eliminated the OS target information.  Now exists in the ostypes.h file in
    the DCL project.
    Revision 1.8  2004/12/31 00:26:37Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.7  2004/12/16 08:48:48Z  garys
    Added TOUPPER and TOLOWER macros to allow typecasting for gnu.
    Revision 1.6  2004/12/11 02:59:40  tonyq
    Added perror to WINCEBL OESL
    Revision 1.5  2004/09/29 02:09:53  GaryP
    General cleanup of configuration settings in preparation for FFX/Pro release.
    Revision 1.4  2004/09/23 06:14:33Z  GaryP
    Refactored for FFX/Pro release.
    Revision 1.3  2004/08/30 18:55:59Z  GaryP
    Added the DCL_OSFEATURE_THREADS setting.
    Revision 1.2  2004/08/25 07:56:00Z  GaryP
    Added the background garbage collection setting.
    Revision 1.1  2004/08/16 19:16:02Z  garyp
    Initial revision
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
    default will be FFX_COMPACT_NONE.
-------------------------------------------------------------------*/
#ifndef FFX_COMPACTIONMODEL
#define FFX_COMPACTIONMODEL         FFX_COMPACT_NONE
#endif


/*-------------------------------------------------------------------
    FFXCONF_POWERSUSPENDRESUME controls whether the API for
    providing notifications for power suspend and resume is
    included or not.
-------------------------------------------------------------------*/
#ifndef FFXCONF_POWERSUSPENDRESUME
/* Default to FALSE for a CE bootloader environment
*/
#define FFXCONF_POWERSUSPENDRESUME  FALSE
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
    No FAT Monitor support in a bootloader environment.
-------------------------------------------------------------------*/
#define FFXCONF_FATMONITORSUPPORT   FALSE


/*-------------------------------------------------------------------
    Misc settings
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
    Force write interruption testing to be disabled regardless
    whether we are doing a debug or release build.  When building
    a boot loader or OAL, the required C library stuff to support
    write interruption testing is not available.
-------------------------------------------------------------------*/
#define VBF_WRITEINT_ENABLED    FALSE



#endif /* OESL_H_INCLUDED */
