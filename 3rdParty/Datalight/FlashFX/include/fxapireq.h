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

    This header defines the general structure used to provide the external
    API functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxapireq.h $
    Revision 1.14  2010/07/31 22:50:37Z  garyp
    Updated to use the DCLREQ_VERSIONCHECK structure defined in DCL.
    Revision 1.13  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.12  2009/03/27 08:12:46Z  keithg
    Removed obsolete include of DCL requestor include; temporarily
    removed fxapi.h include to get obfuscation working.
    Revision 1.11  2009/02/09 03:34:50Z  garyp
    Merged from the v4.0 branch.  Include fxapi.h.  Prototypes updated.  Define
    FFXREQHANDLE to be the same as DCLOSREQHANDLE.
    Revision 1.10  2008/03/23 02:39:34Z  Garyp
    Made allocator/VBF code conditional on FFXCONF_ALLOCATORSUPPORT and
    FFXCONF_VBFSUPPORT, respectively.
    Revision 1.9  2007/12/26 02:03:58Z  Garyp
    Added prototypes.
    Revision 1.8  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/08/03 02:50:12Z  garyp
    Updated to use tje new 32-bit version number.
    Revision 1.6  2007/06/21 23:34:26Z  Garyp
    Updated to use the generic external requestor logic now implemented in DCL.
    Revision 1.5  2006/09/16 19:26:53Z  Garyp
    Modified to handle build numbers which are now strings rather than numbers.
    Revision 1.4  2006/02/13 09:14:54Z  Garyp
    Updated to new external API interface.
    Revision 1.3  2006/02/07 07:32:16Z  Garyp
    Renamed FFXDEVHANDLE to FFXREQHANDLE to avoid conflicts with the
    new meaning of FFXDEVHANDLE.
    Revision 1.2  2006/01/01 06:00:36Z  Garyp
    Added API requestor support for the profiler.
    Revision 1.1  2005/05/02 17:38:08Z  Pauli
    Initial revision
    Revision 1.3  2005/05/02 18:38:08Z  Garyp
    Added external API support for FfxFmlWritePages.
    Revision 1.2  2005/11/06 09:05:23Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/10/02 02:33:24Z  Garyp
    Initial revision
    Revision 1.6  2004/12/30 17:31:14Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/11/29 18:51:39Z  GaryP
    Added support for a Driver Framework class of external API functions.
    Revision 1.4  2004/11/20 04:46:33Z  GaryP
    Added external API support for compaction suspend/restore.
    Revision 1.3  2004/09/23 08:31:25Z  GaryP
    Semantic change from "garbage collection" to "compaction".
    Revision 1.2  2004/02/22 19:15:42Z  garys
    Revision 1.1.1.5  2004/02/22 19:15:42  garyp
    Eliminated support for oemchanged().
    Revision 1.1.1.4  2004/02/02 05:38:40Z  garyp
    Added the DEBUG class of external API functions.
    Revision 1.1.1.3  2003/11/26 20:45:14Z  billr
    Fix warning on 16-bit platforms.
    Revision 1.1.1.2  2003/11/24 01:39:12Z  garyp
    Removed an unnecessary comma.
    Revision 1.1  2003/11/22 03:32:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FXAPIREQ_H_INCLUDED
#define FXAPIREQ_H_INCLUDED

#include <fxiosys.h>


#define FFXAPI_REQUESTSIGNATURE 0xCCDD55AAUL
#define FFXAPI_REPLYSIGNATURE   0x55AACCDDUL

/*---------------------------------------------------------
    Function prototypes
---------------------------------------------------------*/
FFXREQHANDLE    FlashFX_DevOpen(unsigned nDiskNum);
FFXSTATUS       FlashFX_DevRequest(             FFXREQHANDLE hDev, FFXIOREQUEST *pReq);
FFXSTATUS       FlashFX_DevClose(               FFXREQHANDLE hDev);
FFXSTATUS       FfxDriverIoctlMasterDispatch(   FFXIOREQUEST *pReq);
FFXSTATUS       FfxDriverIoctlDispatch(         FFXIOREQUEST *pReq);
FFXSTATUS       FfxDriverGenericIoctlDispatch(  FFXIOREQUEST *pReq);
FFXSTATUS       FfxFmlIoctlDispatch(            FFXIOREQUEST *pReq);

#if FFXCONF_ALLOCATORSUPPORT && FFXCONF_VBFSUPPORT
FFXSTATUS       FfxVbfIoctlDispatch(            FFXIOREQUEST *pReq);
#endif



#endif /* FXAPIREQ_H_INCLUDED */
