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

    FlashFX header for the Loader porting kit.

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
    Revision 1.6  2009/07/17 20:26:00Z  garyp
    Added support for the FFXCONF_POWERSUSPENDRESUME feature.
    Revision 1.5  2008/03/26 19:30:51Z  Garyp
    Modified so write interruption testing support, and FAT Monitor are always
    disabled in a loader environment.
    Revision 1.4  2008/01/31 21:08:45Z  Garyp
    Removed the obsolete and unused FFX_IO_MAPPED setting and IOPORTADDRESS
    type.
    Revision 1.3  2008/01/17 04:07:59Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/10/10 05:20:36Z  brandont
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
#define FFXCONF_FORCEALIGNEDIO       TRUE
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
/* Default to FALSE for a loader environment
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


                /********************************\
                 *                              *
                 *      Constant settings       *
                 *      for this OS             *
                 *                              *
                \********************************/


/*-------------------------------------------------------------------
    Disable FAT Monitor support in a loader environment.
-------------------------------------------------------------------*/
#define FFXCONF_FATMONITORSUPPORT   FALSE


/*-------------------------------------------------------------------
    Force write interruption testing to be disabled regardless
    whether we are doing a debug or release build.  When building
    in primitive environments, the required C library stuff to
    support write interruption testing may not be not available.
-------------------------------------------------------------------*/
#define VBF_WRITEINT_ENABLED        FALSE




#endif /* OESL_H_INCLUDED */
