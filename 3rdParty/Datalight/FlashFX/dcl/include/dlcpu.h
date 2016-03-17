/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This header contains CPU family identifies for use with the D_CPUFAMILY
    symbol which is passed in on the command-line using the standard build
    process.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                 Revision History
    $Log: dlcpu.h $
    Revision 1.5  2011/08/31 20:18:07Z  billr
    Resolve bug 3243: Checks to detect GPL violations give false positives.
    Revision 1.4  2010/03/10 18:08:02Z  jimmb
    Added LInux specific CPU_FAMILY types for build
    i3296 and powerc projects.
    Revision 1.3  2009/02/08 01:05:07Z  garyp
    Protected against nested inclusion.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2006/12/08 03:09:24Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLCPU_H_INCLUDED
#define DLCPU_H_INCLUDED

/*  The lowercase processor family suffix on these types must exactly
    match the CPU family as specified in the prjinfo.bat file, and
    passed  into this code on the command line.
*/
#define D_CPUFAMILY_x86            (1)
#define D_CPUFAMILY_i386           (1)
#define D_CPUFAMILY_arm            (2)
#define D_CPUFAMILY_ppc            (3)
#define D_CPUFAMILY_powerpc        (3)
#define D_CPUFAMILY_mips           (4)
#define D_CPUFAMILY_coldfire       (5)
#define D_CPUFAMILY_tricore        (6)
#define D_CPUFAMILY_sh             (7)
#define D_CPUFAMILY_simulator      (8)
#define D_CPUFAMILY_68k            (9)

#endif  /* DLCPU_H_INCLUDED */

