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

    This header provides the prototypes and any other macros and symbols
    that may be required to use the FlashFX tools.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxtools.h $
    Revision 1.19  2010/12/12 06:42:45Z  garyp
    Updated for the new FXDUMP calling convention.
    Revision 1.18  2010/05/08 00:21:37Z  garyp
    Reverted the previous checkin to allow things to build in a 4GR style
    environment.
    Revision 1.17  2010/05/04 21:47:54Z  billr
    Refactor headers for Linux build process.
    Revision 1.16  2010/03/09 21:35:47Z  billr
    Resolve bug 3060: Enabling unit tests causes build failures.
    Revision 1.15  2009/10/09 02:24:36Z  garyp
    Removed the inclusion of dltools.h, as well as prototypes which are only
    available to internally compiled code.
    Revision 1.14  2009/09/10 16:59:19Z  garyp
    Documentation updated -- no functional changes.
    Revision 1.13  2009/02/09 03:42:35Z  garyp
    Merged from the v4.0 branch.  Updated FFXTOOLPARAMS to contain an embedded
    DCLTOOLPARAMS structure.  Changed some fields from D_UINT16 to unsigned.
    Revision 1.12  2008/05/19 05:15:42Z  keithg
    Added prototype for the BBM unit test
    Revision 1.11  2008/03/26 02:40:46Z  Garyp
    Prototype updates.
    Revision 1.10  2007/12/01 18:21:32Z  Garyp
    Added a prototype.
    Revision 1.9  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.8  2007/08/30 01:27:22Z  Garyp
    Added prototypes.
    Revision 1.7  2007/02/08 21:23:25Z  Garyp
    Added prototypes.
    Revision 1.6  2006/06/15 23:57:06Z  Pauli
    Made FfxShell prototype conditional on if the command shell is enabled.
    Revision 1.5  2006/05/28 22:01:59Z  Garyp
    Added FfxShell().
    Revision 1.4  2006/03/06 01:47:10Z  Garyp
    Eliminated FXRDIMG/FXWRIMG and replaced with FXIMAGE.
    Revision 1.3  2006/02/12 18:15:14Z  Garyp
    Changed from a mask to a device/disk number.
    Revision 1.2  2006/02/10 10:19:02Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/10/02 23:51:48Z  Pauli
    Initial revision
    Revision 1.7  2005/06/11 23:27:54Z  PaulI
    Reformatted prototypes.
    Revision 1.6  2004/12/30 23:13:59Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.5  2004/10/22 18:35:04Z  jaredw
    Added fxdump prototype.
    Revision 1.4  2004/07/20 01:25:35Z  GaryP
    Added the FFXTOOLPARAMS structure and the prototypes that used to
    be in fxtests.h.
    Revision 1.3  2004/01/18 23:36:26Z  garys
    Merge from FlashFXMT
    Revision 1.2.1.2  2004/01/18 23:36:26  garyp
    Added prototypes.
    Revision 1.2  2003/03/21 19:43:02Z  garyp
    Updated to use the new TOOLS and TESTS architecture.
---------------------------------------------------------------------------*/

#ifndef FXTOOLS_H_INCLUDED
#define FXTOOLS_H_INCLUDED


/*-------------------------------------------------------------------
                     TOOLS and TESTS framework
-------------------------------------------------------------------*/
typedef struct
{
    DCLTOOLPARAMS   dtp;
    const char     *pszDriveForms; /* (To be migrated into DCLTOOLPARAMS) */
    unsigned        nDeviceNum;
    unsigned        nDiskNum;
} FFXTOOLPARAMS;


/*-------------------------------------------------------------------
    Prototypes for TOOLS as called by the OS-specific wrappers
-------------------------------------------------------------------*/
D_INT16     FfxCheckUnits(FFXTOOLPARAMS *pTP);
D_INT16     FfxDiskInfo(  FFXTOOLPARAMS *pTP);
FFXSTATUS   FfxDump(      FFXTOOLPARAMS *pTP);
D_INT16     FfxFormat(    FFXTOOLPARAMS *pTP);
D_INT16     FfxImage(     FFXTOOLPARAMS *pTP);
D_INT16     FfxReclaim(   FFXTOOLPARAMS *pTP);
D_INT16     FfxRemount(   FFXTOOLPARAMS *pTP);
D_INT16     FfxShell(     FFXTOOLPARAMS *pTP);


/*-------------------------------------------------------------------
    Prototypes for TESTS as called by the OS-specific wrappers
-------------------------------------------------------------------*/
D_INT16     FfxTestFMSL(  FFXTOOLPARAMS *pTP);
D_INT16     FfxTestVBF(   FFXTOOLPARAMS *pTP);
D_INT16     FfxTestFSIO(  FFXTOOLPARAMS *pTP);
D_INT16     FfxStressMT(  FFXTOOLPARAMS *pTP);
D_INT16     FfxTestBBM(   FFXTOOLPARAMS *pTP);



#endif /* FXTOOLS_H_INCLUDED */

