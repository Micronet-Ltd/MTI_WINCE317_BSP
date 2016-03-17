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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxconfig.h $
    Revision 1.14  2010/04/18 23:08:07Z  garyp
    Updated to use some refactored DCL system info functionality.
    Revision 1.13  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.12  2008/01/17 04:07:57Z  Garyp
    Eliminated the obsolete and unused FFXCONF_ENABLEEXTERNALAPI setting.
    Revision 1.11  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/04/07 23:01:45Z  Garyp
    Added configuration bit definitions.
    Revision 1.9  2007/03/29 23:31:38Z  Garyp
    Updated to support mutex and read/write semaphore statistics.
    Revision 1.8  2007/01/05 21:10:40Z  Garyp
    Updated so FfxConfigurationInfo() structures and prototypes are self-
    contained in this header.
    Revision 1.7  2006/11/13 18:28:07Z  Garyp
    Minor structure update to include the compaction model.
    Revision 1.6  2006/10/16 19:31:14Z  Garyp
    Added support for new configuration flags.
    Revision 1.5  2006/10/13 02:55:48Z  Garyp
    Various settings renamed.
    Revision 1.4  2006/04/20 18:50:14Z  Garyp
    Eliminated the obsolete reserved space field.
    Revision 1.3  2006/02/20 22:39:28Z  Garyp
    Minor type changes.
    Revision 1.2  2006/01/02 02:04:10Z  Garyp
    Added more information to FFXCONFIGINFO.
    Revision 1.1  2005/11/06 03:31:36Z  Pauli
    Initial revision
    Revision 1.2  2005/11/06 03:31:35Z  Garyp
    Updated to support the new statistics interface.
    Revision 1.1  2005/10/02 02:33:26Z  Garyp
    Initial revision
    Revision 1.3  2004/12/30 17:31:14Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2004/12/01 19:05:18Z  PaulI
    Removed duplicate typedef of PFFXCONFIGINFO.
    This is defined in fxdriver.h.
    Revision 1.1  2004/11/29 19:14:22Z  GaryP
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXCONFIG_H_INCLUDED
#define FXCONFIG_H_INCLUDED

#include <dlstats.h>


#define FFXCONFBIT_FATSUPPORT             0x00000001
#define FFXCONFBIT_RELIANCESUPPORT        0x00000002
#define FFXCONFBIT_DRIVERAUTOFORMAT       0x00000004
#define FFXCONFBIT_FATFORMATSUPPORT       0x00000008
#define FFXCONFBIT_FATMONITORSUPPORT      0x00000010
#define FFXCONFBIT_FORMATSUPPORT          0x00000020
#define FFXCONFBIT_RESERVED               0x00000040
#define FFXCONFBIT_FORCEALIGNEDIO         0x00000080
#define FFXCONFBIT_MBRSUPPORT             0x00000100
#define FFXCONFBIT_MBRFORMAT              0x00000200
#define FFXCONFBIT_BBMFORMAT              0x00000400
#define FFXCONFBIT_LATENCYRUNTIME         0x00000800
#define FFXCONFBIT_LATENCYAUTOTUNE        0x00001000
#define FFXCONFBIT_LATENCYERASESUSPEND    0x00002000


/*-------------------------------------------------------------------
    FFXCONFIGINFO
-------------------------------------------------------------------*/
struct tagFFXCONFIGINFO
{
    D_UINT16                uStrucLen;
    D_UINT16                uCompactionModel;
    D_UINT32                ulConfBits;
    D_UINT16                uMaxFmlDevices;
    D_UINT16                uMaxVbfDrives;
    DCLSYSTEMINFO           SysInfo;
    DCLMEMSTATS             MemStats;
    DCLMUTEXSTATS           MutStats;
    DCLSEMAPHORESTATS       SemStats;
    DCLRDWRSEMAPHORESTATS   RWSemStats;
};

typedef struct tagFFXCONFIGINFO FFXCONFIGINFO;

FFXSTATUS FfxConfigurationInfo(FFXCONFIGINFO *pCI, D_BOOL fVerbose, D_BOOL fReset);




#endif /* FXCONFIG_H_INCLUDED */
