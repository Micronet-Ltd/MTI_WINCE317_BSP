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

    This header file contains defines the interface to the "statistics"
    feature in FlashFX.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxperflog.h $
    Revision 1.7  2009/02/09 00:57:50Z  garyp
    Eliminated a header which is no longer necessary.
    Revision 1.6  2008/05/03 20:26:37Z  garyp
    Updated to build cleanly when output is disabled.
    Revision 1.5  2007/12/26 02:29:52Z  Garyp
    Added prototypes.
    Revision 1.4  2007/12/13 03:52:12Z  Garyp
    Prototype updated.
    Revision 1.3  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2007/08/29 23:29:22Z  Garyp
    Changed some D_BUFFER types to char to avoid compiler warnings in
    some environments.
    Revision 1.1  2007/06/22 17:34:04Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXPERFLOG_H_INCLUDED
#define FXPERFLOG_H_INCLUDED

#include <dlperflog.h>

#if DCLCONF_OUTPUT_ENABLED

/*-------------------------------------------------------------------
    FFXSTATS
-------------------------------------------------------------------*/
typedef enum
{
    FFXSTATS_RESERVED           = 0x0000,
    FFXSTATS_DISKIO,
    FFXSTATS_DISKCOMPACTION,
    FFXSTATS_VBFCOMPACTION,
    FFXSTATS_VBFREGION,
    /*  Note that these stats are essentially in hierarchical
        order, and it is important that they remain that way.
        The Driver Framework contains the generalized functions
        to gather the high level stats, and the FML has the
        generalized functions to handle the low level stats.
    */
    FFXSTATS_FMLIO,
    FFXSTATS_FMLIONAND,
    FFXSTATS_BBM,
    FFXSTATS_LIMIT
} FFXSTATS;


/*-------------------------------------------------------------------
    Prototypes for the statistics interface.  Note that these
    functions are structured such that the "query" functions
    reside in libraries which are used when linking together a
    device driver, while these "display" functions are located
    so that they may be discretely linked in by anyone who needs
    to use them.
-------------------------------------------------------------------*/

DCLPERFLOGHANDLE    FfxPerfLogFormatDiskIOStats(        const DRIVERIOSTATS  *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatDiskCompactionStats(const FFXCOMPSTATS   *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatVbfCompactionStats( const VBFCOMPSTATS   *pCS,   DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatVbfRegionStats(     const VBFREGIONSTATS *pRS,   DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatFmlIOStats(         const FFXFMLSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatFmlIONandStats(     const FFXFMLSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
DCLPERFLOGHANDLE    FfxPerfLogFormatBbmStats(           const FFXBBMSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);

FFXSTATUS           FfxPerfLogWriteDiskIOStats(         const DRIVERIOSTATS  *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteDiskCompactionStats( const FFXCOMPSTATS   *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteVbfCompactionStats(  const VBFCOMPSTATS   *pCS,   DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteVbfRegionStats(      const VBFREGIONSTATS *pRS,   DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteFmlIOStats(          const FFXFMLSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteFmlIONandStats(      const FFXFMLSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);
FFXSTATUS           FfxPerfLogWriteBbmStats(            const FFXBBMSTATS    *pStat, DCLLOGHANDLE hLog, const char *pszCatSuffix, const char *pszBldSuffix);

FFXSTATUS           FfxDriverPerfLogDispatch(FFXDISKHANDLE hDisk, DCLREQ_STATS *pReq);
FFXSTATUS           FfxFmlPerfLogDispatch(   FFXFMLHANDLE hFML, DCLREQ_STATS *pReq);

#endif  /* DCLCONF_OUTPUT_ENABLED */
#endif  /* FXPERFLOG_H_INCLUDED */


