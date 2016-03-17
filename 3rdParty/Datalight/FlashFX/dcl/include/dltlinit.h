/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header examines the D_TOOLSETNUM value and includes the proper
    ToolSet header.

    The D_TOOLSETNUM is typically specified on the command-line by the
    ToolSet abstraction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltlinit.h $
    Revision 1.14  2010/06/24 14:17:47Z  jimmb
    Name police - Bad programmer - no donut - renamed to reflect GNU
    Revision 1.13  2010/06/21 15:19:53Z  jimmb
    Added the GNU linux tool abstraction for RTOS
    Revision 1.12  2010/06/18 15:04:17Z  jimmb
    Completed adding the Intel compiler descriptions for DCL
    Revision 1.11  2010/04/28 23:30:36Z  garyp
    Added support for the TICCS ToolSet abstraction.
    Revision 1.10  2009/02/08 01:13:10Z  garyp
    Merged from the v4.0 branch.  Added support for the MARVELLSDT and
    MSWCE ToolSet abstractions.
    Revision 1.9  2008/03/11 21:56:01Z  jeremys
    Added WDK support.
    Revision 1.8  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/06/15 01:03:10Z  pauli
    Replaced the poorly named gnunu toolset with gnucs.
    Revision 1.6  2007/03/18 18:08:25Z  Garyp
    Added the "msvs" ToolSet.
    Revision 1.5  2006/12/07 18:33:16Z  Garyp
    Added Microtec ToolSet support.
    Revision 1.4  2006/11/02 19:54:08Z  Garyp
    Renamed the Green Hills toolset from ghsarm36 to ghs.
    Revision 1.3  2006/10/07 01:03:25Z  Garyp
    Added support for the RVDS30 ToolSet abstraction.
    Revision 1.2  2006/08/31 21:43:12Z  Garyp
    Documentation fixes.
    Revision 1.1  2005/10/02 00:42:52Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLTLINIT_H_INCLUDED
#define DLTLINIT_H_INCLUDED

#if ((D_TOOLSETNUM == TS_BC4X) || (D_TOOLSETNUM == TS_DEVTOOLS))
    #include "toolset/devtools.h"
#elif ((D_TOOLSETNUM == TS_MSWCE4) || (D_TOOLSETNUM == TS_MSWCEPB) || (D_TOOLSETNUM == TS_MSWCE))
    #include "toolset/mswce.h"
#elif D_TOOLSETNUM == TS_MSVC6
    #include "toolset/msvc6.h"
#elif D_TOOLSETNUM == TS_MSVS
    #include "toolset/msvs.h"
#elif D_TOOLSETNUM == TS_MSWINDDK
    #include "toolset/mswinddk.h"
#elif D_TOOLSETNUM == TS_MSWDK
    #include "toolset/mswdk.h"
#elif D_TOOLSETNUM == TS_ADS12
    #include "toolset/ads12.h"
#elif D_TOOLSETNUM == TS_RVDS30
    #include "toolset/rvds30.h"
#elif D_TOOLSETNUM == TS_DIABVX5X
    #include "toolset/diabvx5x.h"
#elif D_TOOLSETNUM == TS_DIABVX6X
    #include "toolset/diabvx6x.h"
#elif D_TOOLSETNUM == TS_GNUVX5X
    #include "toolset/gnuvx5x.h"
#elif D_TOOLSETNUM == TS_GNUVX6X
    #include "toolset/gnuvx6x.h"
#elif D_TOOLSETNUM == TS_ICCVX6X
    #include "toolset/iccvx6x.h"
#elif D_TOOLSETNUM == TS_GNURTOS
    #include "toolset/gnurtos.h"
#elif D_TOOLSETNUM == TS_GCC3X
    #include "toolset/gcc3x.h"
#elif D_TOOLSETNUM == TS_GHS
    #include "toolset/ghs.h"
#elif D_TOOLSETNUM == TS_TASKING
    #include "toolset/tasking.h"
#elif D_TOOLSETNUM == TS_GNUCS
    #include "toolset/gnucs.h"
#elif D_TOOLSETNUM == TS_MICROTEC
    #include "toolset/microtec.h"
#elif D_TOOLSETNUM == TS_MARVELLSDT
    #include "toolset/marvellsdt.h"
#elif D_TOOLSETNUM == TS_TICCS
    #include "toolset/ticcs.h"
#else
    #error "DCL: dltlinit.h: D_TOOLSETNUM is not recognized, or not defined"
#endif

#endif  /* DLTLINIT_H_INCLUDED */


