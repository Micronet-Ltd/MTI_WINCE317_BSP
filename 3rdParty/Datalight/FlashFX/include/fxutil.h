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

    This header contains prototypes for a variety of standard utility
    functions in FlashFX.  It is automatically included by flashfx.h.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxutil.h $
    Revision 1.18  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.17  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.16  2009/11/28 19:17:57Z  garyp
    Added prototypes.
    Revision 1.15  2009/10/09 02:24:51Z  garyp
    Added prototypes for internal shell functions (were in fxtools.h).
    Revision 1.14  2009/07/18 01:14:38Z  garyp
    Merged from the v4.0 branch.  Updated the prototype for FfxSignon().
    Revision 1.13  2009/02/09 00:59:05Z  garyp
    Removed some prototypes for obsolete functions.
    Revision 1.12  2008/04/03 23:56:23Z  brandont
    Updated all defines and structures used by the DCL file system
    services to use the DCLFS prefix.
    Revision 1.11  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.10  2007/10/27 16:43:57Z  Garyp
    Removed obsolete prototypes.
    Revision 1.9  2007/10/05 20:15:13Z  Garyp
    Removed obsolete prototypes.
    Revision 1.8  2007/02/20 17:53:36Z  Garyp
    Removed obsolete and/or dead code.
    Revision 1.7  2006/10/06 00:48:24Z  Garyp
    Removed prototypes for obsolete functions.
    Revision 1.6  2006/04/24 17:06:35Z  Garyp
    Prototype updates.
    Revision 1.5  2006/02/21 23:03:53Z  Garyp
    Minor type changes.
    Revision 1.4  2006/02/02 00:49:15Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.3  2006/01/11 02:17:35Z  Garyp
    Removed obsolete prototypes.
    Revision 1.2  2006/01/01 13:27:33Z  Garyp
    Added prototypes for profiler functions.
    Revision 1.1  2005/11/25 23:02:10Z  Pauli
    Initial revision
    Revision 1.2  2005/09/17 23:57:14Z  garyp
    Modified so that _sysdelay() and related code is only built if
    FFXCONF_NORSUPPORT is TRUE.
    Revision 1.1  2005/07/06 04:41:28Z  pauli
    Initial revision
    Revision 1.23  2004/12/30 23:13:58Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.22  2004/10/21 19:08:28Z  jaredw
    Added FfxAtoLHex prototype.
    Revision 1.21  2004/09/28 20:23:40Z  billr
    Replace FfxGetMillisecondCount() with FfxTimestamp() and FfxTimePassed().
    Revision 1.20  2004/09/17 04:07:54Z  GaryP
    Added FfxGetMillisecondCount().
    Revision 1.19  2004/09/10 22:22:32Z  GaryP
    Modified FfxStrLen() to return a D_UINT16 type.
    Revision 1.18  2004/08/30 20:22:34Z  GaryP
    Added support for stbmutex.c and stbsem.c.
    Revision 1.17  2004/08/29 20:50:39Z  GaryP
    Added/tweaked prototypes.
    Revision 1.16  2004/08/21 22:16:33Z  GaryP
    Modified timer pointers to be declared with D_FAR.
    Revision 1.15  2004/08/17 16:39:13Z  garyp
    Modified FfxOptionGet() to no longer take an hDrive parameter.
    Revision 1.14  2004/07/19 23:37:18Z  BillR
    Add an implementation of _sysdelay() that uses a calibrated loop for use
    on platforms that lack a microsecond-resolution time reference,
    Revision 1.13  2004/07/19 20:32:23Z  BillR
    Change FindHighestBit() to FfxFindHighestBit() and move it out of BBM.
    Revision 1.12  2004/07/03 02:42:33Z  GaryP
    Added a prototype for FfxOptionGet().
    Revision 1.11  2004/05/05 22:06:01Z  billr
    Thread-safe timers based on system clock tick.
    Revision 1.10  2004/01/03 18:05:44Z  garys
    Merge from FlashFXMT
    Revision 1.9.1.3  2004/01/03 18:05:44  garyp
    Added FfxRand() and FfxStrNICmp().  Eliminated FfxStrCmp().
    Revision 1.9.1.2  2003/11/29 21:33:06Z  garyp
    Added a prototype for FfxStrICmp().
    Revision 1.9  2001/09/10 21:34:56Z  garys
    changed parameter in FfxStrNCpy(), parameter in DclArgRetrieve(),
    and return value from FfxStrLen() to unsigned int.
    This solves some compiler warnings under CE
    Revision 1.8  2003/04/21 11:05:58  garyp
    Added prototypes.
    Revision 1.7  2003/04/15 17:49:12Z  garyp
    Added prototypes for new string routines.
    Revision 1.6  2003/03/26 21:15:14Z  garyp
    Eliminated the __D_* data types.  Routines are renamed to be
    prefixed with "Ffx".
    Revision 1.5  2002/12/04 01:06:26Z  garyp
    Moved DOS specific prototype functions to OESL.H.
---------------------------------------------------------------------------*/
#ifndef FXUTIL_H_INCLUDED
#define FXUTIL_H_INCLUDED


/*  signon.c
*/
const char *    FfxSignOn(D_BOOL fQuiet);


#endif /* FXUTIL_H_INCLUDED */
