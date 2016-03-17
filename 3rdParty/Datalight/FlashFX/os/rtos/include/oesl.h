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

    FlashFX header for the RTOS porting kit.

    This header contains the default settings for the OS.  Most of these
    settings can be overriden in the ffxconf.h for the project.

    This header file is included after ffxconf.h, but before fxmacros.h,
    therefore it may expect that only those settings which are required
    in ffxconf.h for the given OS are set.  Settings which are not required
    in the ffxconf.h, and are set to defaults by fxmacros.h will NOT be
    usable in this header file.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: oesl.h $
    Revision 1.12  2009/07/17 20:26:01Z  garyp
    Added support for the FFXCONF_POWERSUSPENDRESUME feature.
    Revision 1.11  2008/03/26 19:30:55Z  Garyp
    Updated to default to using background compaction only if thread support and
    allocator support is enabled, and to use synchronous (none) if it is not.
    Revision 1.10  2008/01/31 21:08:44Z  Garyp
    Removed the obsolete and unused FFX_IO_MAPPED setting and IOPORTADDRESS
    type.
    Revision 1.9  2008/01/17 04:08:01Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.8  2007/11/03 23:50:02Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2006/10/16 20:58:23Z  Garyp
    Refactored how MBR settings are specified.  Removed some obsolete settings.
    Revision 1.6  2006/10/11 22:20:20Z  Garyp
    Use refactored configuration settings.
    Revision 1.5  2006/10/06 00:22:09Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.4  2006/02/09 19:22:29Z  Pauli
    Updated comments and default settings.
    Revision 1.3  2006/01/12 02:22:17Z  Garyp
    Documentation updated.
    Revision 1.2  2005/12/31 03:06:56Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.1  2005/12/07 06:44:36Z  Pauli
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



                /********************************\
                 *                              *
                 *      Constant settings       *
                 *      for this OS             *
                 *                              *
                \********************************/


/*-------------------------------------------------------------------
    Enable FAT Monitor support so long as FAT support is enabled.
-------------------------------------------------------------------*/
#define FFXCONF_FATMONITORSUPPORT   FFXCONF_FATSUPPORT





#endif /* OESL_H_INCLUDED */
