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

    This module provide the definitions necessary to use the various get/set
    param functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxparam.h $
    Revision 1.26  2010/07/08 03:43:26Z  garyp
    Added more error injection options.
    Revision 1.25  2010/07/06 18:20:15Z  garyp
    Added support for error injection.
    Revision 1.24  2010/07/01 17:44:45Z  garyp
    Added support for chip serial numbers.
    Revision 1.23  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.22  2009/12/08 22:29:41Z  garyp
    Added IDs for querying the FIM/NTM for correctable and uncorrectable
    error counts.
    Revision 1.21  2009/11/29 22:01:27Z  garyp
    Added FFXPARAM_CHIPID_LENGTH.
    Revision 1.20  2009/07/29 18:37:57Z  garyp
    Merged from the 4.0 branch.  Added FFXPARAM_FIM_CHIPID.  Renamed a symbol.
    Revision 1.19  2009/05/07 16:02:21Z  keithg
    Removed compile time configuration defines to allow this file to
    be used in application level includes.
    Revision 1.18  2009/02/08 20:29:17Z  garyp
    Merged from the v4.0 branch.  Updated to use some renamed variables
    -- no functional changes.
    Revision 1.17  2009/01/19 05:25:13Z  keithg
    Added support for the NTSIM_CHECK parameter (corrected name).
    Revision 1.16  2009/01/19 05:22:57Z  keithg
    Added a debug check for the NAND simulator.
    Revision 1.15  2008/10/02 22:37:56Z  keithg
    Added an invalid value parameter for validation use.
    Revision 1.14  2008/03/26 02:55:14Z  Garyp
    Removed FFXPARAM_TAG_LENGTH.  Added parameter codes for manipulating the
    trace mask.
    Revision 1.13  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2007/06/20 01:34:27Z  Garyp
    Factored out the stats reset logic so that it can be independently invoked.
    Revision 1.11  2007/04/15 17:51:55Z  Garyp
    Renamed the BBM stats parameter value to match the other stats related
    parameters.
    Revision 1.10  2007/04/07 23:01:49Z  Garyp
    Made the latency parameters condiitional on the latency reduction features
    being enabled.
    Revision 1.9  2007/04/07 00:04:46Z  Garyp
    Reduced the scale so the parameters can be enumerated faster.
    Revision 1.8  2007/03/24 22:33:09Z  Garyp
    Added an "ErasePollInterval" parameter ID.
    Revision 1.7  2007/01/31 23:41:37Z  Garyp
    Renamed some parameter names.
    Revision 1.6  2006/11/17 04:37:25Z  Garyp
    Modified to allow the read/write/erase latency settings to be queried or
    set.
    Revision 1.5  2006/11/14 17:54:31Z  billr
    Add support for new parameter to get BBM information.
    Revision 1.4  2006/11/10 03:25:48Z  Garyp
    Standardized on a naming convention for the various "stats" interfaces.
    Modified the stats query to use the generic ParameterGet() ability now
    implemented through most all layers of the code.
    Revision 1.3  2006/10/18 17:55:05Z  billr
    Revert changes made in revision 1.2.
    Revision 1.1  2006/02/01 21:59:36Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXPARAM_H_INCLUDED
#define FXPARAM_H_INCLUDED

#define FFXPARAM_CHIPID_LENGTH          (8)
#define FFXPARAM_CHIPSN_LENGTH          (16)


typedef enum
{
    FFXPARAM_STARTOFLIST                = 0x0000,
    FFXPARAM_PAGE_SIZE                  = 0x0001,

    FFXPARAM_STATS_FML                  = 0x0100,
    FFXPARAM_STATS_FMLRESET,
    FFXPARAM_STATS_DRIVERIO,
    FFXPARAM_STATS_DRIVERIORESET,
    FFXPARAM_STATS_DRIVERCOMPACTION,
    FFXPARAM_STATS_DRIVERCOMPACTIONRESET,
    FFXPARAM_STATS_VBFREGION,
    FFXPARAM_STATS_VBFREGIONRESET,
    FFXPARAM_STATS_VBFCOMPACTION,
    FFXPARAM_STATS_VBFCOMPACTIONRESET,
    FFXPARAM_STATS_BBM,
    FFXPARAM_STATS_RESETALL,

    FFXPARAM_DEVMGR_MAXREADCOUNT        = 0x0200,
    FFXPARAM_DEVMGR_MAXWRITECOUNT,
    FFXPARAM_DEVMGR_MAXERASECOUNT,
    FFXPARAM_DEVMGR_ERASEPOLLINTERVAL,
    FFXPARAM_DEVMGR_CYCLEMUTEX,

    FFXPARAM_DEBUG_TRACEMASKGET         = 0x0300,
    FFXPARAM_DEBUG_TRACEMASKSWAP,

    FFXPARAM_FIM_CHIPID                 = 0x0400,
    FFXPARAM_FIM_CHIPSN,
    FFXPARAM_FIM_CORRECTEDERRORS,
    FFXPARAM_FIM_UNCORRECTABLEERRORS,

    FFXPARAM_FIM_ERRINJECT_MIN          = 0x04C0,
    FFXPARAM_FIM_ERRINJECT_DISABLE      = 0x04C0,
    FFXPARAM_FIM_ERRINJECT_READIO,
    FFXPARAM_FIM_ERRINJECT_WRITEIO,
    FFXPARAM_FIM_ERRINJECT_ERASEIO,
/*  FFXPARAM_FIM_ERRINJECT_WRITETIMEOUT, */
    FFXPARAM_FIM_ERRINJECT_MAX,

    FFXPARAM_NTM_SIMCHECK               = 0x0500,

    /*  End of the enumeration.
    */
    FFXPARAM_ENDOFLIST
} FFXPARAM;


#endif /* FXPARAM_H_INCLUDED */


