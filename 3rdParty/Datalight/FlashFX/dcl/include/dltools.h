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

    This module provides the prototypes and symbols that may be required to
    use the DCL tests and tools.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dltools.h $
    Revision 1.23  2011/05/18 22:39:52Z  garyp
    Moved the DclTestFSStressMain() prototype from dltfsstress.h to dltools.h.
    Revision 1.22  2010/03/09 21:38:46Z  billr
    Resolve bug 3060: Enabling unit tests causes build failures.
    Revision 1.21  2009/09/10 01:21:03Z  garyp
    Added support for DEVIOTEST.
    Revision 1.20  2009/06/25 00:33:34Z  garyp
    Moved the misplaced tests prototypes to dltests.h.
    Revision 1.19  2009/05/08 02:19:17Z  garyp
    Added UTF-8 tests.
    Revision 1.18  2009/04/08 19:17:57Z  garyp
    Updates for AutoDoc -- no functional changes.
    Revision 1.17  2009/02/21 02:04:33Z  brandont
    Renamed Testudivdi3 to DclTestudivdi3.  Added DCLFUNC for DclTestCompilerBugs
    and DclTestudivdi3.
    Revision 1.16  2009/02/08 20:00:44Z  garyp
    Removed an unused field.
    Revision 1.15  2009/01/19 22:00:21Z  johnb
    Added prototype for Multibyte/Wide char test, DclTestMBWCConv.
    Revision 1.14  2008/11/05 15:45:42Z  jimmb
    Added prototype for DCLSTATUS DclTestudivdi3(void);
    Revision 1.13  2007/12/18 05:17:22Z  brandont
    Updated comments.
    Revision 1.12  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.11  2007/10/05 20:42:54Z  pauli
    Added memory tracking tests.
    Revision 1.10  2007/07/31 19:47:46Z  Garyp
    Clarified an ambiguous function name.
    Revision 1.9  2007/05/13 16:51:54Z  garyp
    Added a prototype.
    Revision 1.8  2007/04/03 19:15:26Z  Garyp
    Updated to support a PerfLog build number suffix.
    Revision 1.7  2007/01/28 00:45:38Z  Garyp
    Added PerfLog support.
    Revision 1.6  2006/07/03 18:07:50Z  Pauli
    Added DclStats().
    Revision 1.5  2006/05/06 22:06:01Z  Garyp
    Minor prototype changes.
    Revision 1.4  2006/03/14 23:59:08Z  Pauli
    Added path test.
    Revision 1.3  2006/02/03 00:29:32Z  Pauli
    Added prototype for date/time tests.
    Revision 1.2  2005/12/27 23:00:24Z  Pauli
    Updated prototypes, adding 64-bit math and byte order tests.
    Revision 1.1  2005/10/02 02:10:54Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/
#ifndef DLTOOLS_H_INCLUDED
#define DLTOOLS_H_INCLUDED


/*-------------------------------------------------------------------
    Type: DCLTOOLPARAMS

    A structure used to pass parameters to tests and tools.
-------------------------------------------------------------------*/
typedef struct
{
    const char         *pszCmdName;
    const char         *pszCmdLine;
    const char         *pszExtraHelp;
    const char         *pszDriveForms;
    DCLINSTANCEHANDLE   hDclInst;
} DCLTOOLPARAMS;


/*-------------------------------------------------------------------
    Prototypes for TESTS as called by the OS-specific wrappers
-------------------------------------------------------------------*/
#define DclTestMain                 DCLFUNC(DclTestMain)
#define DclTestDevIOMain            DCLFUNC(DclTestDevIOMain)
#define DclTestFSIOMain             DCLFUNC(DclTestFSIOMain)
#define DclTestFSStressMain         DCLFUNC(DclTestFSStressMain)
#define DclStats                    DCLFUNC(DclStats)

DCLSTATUS DclTestMain(          DCLTOOLPARAMS *pTP);
DCLSTATUS DclTestDevIOMain(     DCLTOOLPARAMS *pTP);
DCLSTATUS DclTestFSIOMain(      DCLTOOLPARAMS *pTP);
DCLSTATUS DclTestFSStressMain(  DCLTOOLPARAMS *pTP);
DCLSTATUS DclStats(             DCLTOOLPARAMS *pTP);



#endif /* DLTOOLS_H_INCLUDED */


