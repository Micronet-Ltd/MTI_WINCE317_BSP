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

    "Write Interuption Defines And Types"
    This header file contains a few things.
    1. The define for the write interruption tests
    2. An enumeration of variables which are used for each
       interruption point
    3. A macro that performs the write interruption request.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: tstwint.h $
    Revision 1.7  2009/07/21 20:41:49Z  garyp
    Merged from the v4.0 branch.  Added interruption points to support deferred
    discards.  Minor datatype changes from D_UINT16 to unsigned. 
    Revision 1.6  2009/03/27 23:42:26Z  keithg
    Removed previous prototype change made in error.
    Revision 1.5  2009/03/27 16:51:05Z  keithg
    Changed FfxMediaMgrIssueWriteInterrupt prototype to match definition.
    Revision 1.4  2007/11/03 23:49:29Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2006/05/08 09:07:25Z  Garyp
    Updated to use the new write interruption test interfaces.
    Revision 1.2  2006/03/13 02:57:37Z  Garyp
    Eliminated an unnecessary extern.
    Revision 1.1  2005/12/08 03:07:18Z  Garyp
    Initial revision
    Revision 1.4  2005/01/28 22:00:16Z  GaryP
    Updated to use new DCL symbols.
    Revision 1.3  2004/12/30 23:48:13Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2003/06/11 00:31:29Z  dennis
    Revision 1.1  2003/05/21 01:56:48  garyp
    Initial revision
    Revision 1.7  2003/05/02 17:41:16Z  garyp
    Made write interruption testing also dependent on the FX_OESLMONO setting.
    Revision 1.6  2003/04/29 21:23:54Z  garyp
    Minor fixes to mollify the VxWorks/GNU compiler.
    Revision 1.5  2003/04/23 21:33:28Z  dennis
    Final version of write interruption tests.  Fail all calls when the
    region cannot be mounted in vbfxxx().
    09/26/02 de  Write interruption tests now use setjmp/longjmp and remount
                 the disk following an interruption.  This more faithfully
                 simulates a power loss.
    10/01/01 mjm Added VBF patent info.
    11/19/98 PKG Corrected use of DEBUG to D_DEBUG.  Missed during previous
                 updates of DEBUG
    03/09/98 PKG Made the write interruption tests off by default
    08/26/97 PKG Now uses new Datalight specific types
    09/04/96 DM  Created
---------------------------------------------------------------------------*/
#ifndef TSTWINT_H_INCLUDED
#define TSTWINT_H_INCLUDED

#if VBF_WRITEINT_ENABLED

/*  Need setjmp/longjmp declarations for the write interruption tests.
*/

#ifdef DCL_OSTARGET_VXWORKS
#include <types/vxTypesOld.h>
#endif

#include <setjmp.h>

enum
{
    WRITE_INT_VBFSUP1 = 0,  /* "Write interruption: After write, before discard" */
    WRITE_INT_VBFSUP2,      /* "Write interruption: After discard, before error handler" */
    DISCARD_INT_VBFSUP1,    /* "Discard interruption: Before discard" */
    DISCARD_INT_VBFSUP2,    /* "Discard interruption: After discard, before writing metadata" */
    DISCARD_INT_VBFSUP3,    /* "Discard interruption: Discard finished." */
    COMPACT_INT_MEDIAMGR1,  /* "Compact MM interruption: Before Formatting Spare LNU" */
    COMPACT_INT_MEDIAMGR2,  /* "Compact MM interruption: After Invalidating Spare LNU" */
    COMPACT_INT_MEDIAMGR3,  /* "Compact MM interruption: After Erasing Spare LNU" */
    COMPACT_INT_VBFSUP0,    /* "Compact interruption: After initial metadata write" */
    COMPACT_INT_VBFSUP1,    /* "Compact interruption: After Formatting Spare LNU" */
    COMPACT_INT_VBFSUP2,    /* "Compact interruption: After metadata copy" */
    COMPACT_INT_VBFSUP3,    /* "Compact interruption: During copy" */
    COMPACT_INT_VBFSUP4,    /* "Compact interruption: Copy complete, after NOR discard" */
    COMPACT_INT_VBFSUP5,    /* "Compact interruption: Compaction complete" */
    COMPACT_INT_VBFSUP6,    /* "Compact interruption: After NAND metadata" */
    WRITE_INT_MEDIAMGR1,    /* "MM interruption: after first alloc write" */
    WRITE_INT_MEDIAMGR2,    /* "MM interruption: after writing data" */
    WRITE_INT_MEDIAMGR3,    /* "MM interruption: after final alloc write" */
    WRITE_INT_MEDIAMGR4,    /* "MM interruption: discard, after first alloc write" */
    WRITE_INT_MEDIAMGR5,    /* "MM interruption: discard, after second alloc write" */
    VBF_INTERRUPTION_POINTS
};


void FfxVbfIssueWriteInterrupt(unsigned nDiskNum, D_UINT16 uInterruption);
void FfxMediaMgrIssueWriteInterrupt(FFXFMLHANDLE hFML, D_UINT16 uInterruption);

#define FFX_ISSUE_WRITE_INTERRUPTION(d, x)   (FfxVbfIssueWriteInterrupt((d), (x)))
#define FFXMM_ISSUE_WRITE_INTERRUPTION(h, x) (FfxMediaMgrIssueWriteInterrupt((h), (x)))

#else

#define FFX_ISSUE_WRITE_INTERRUPTION(d, x)
#define FFXMM_ISSUE_WRITE_INTERRUPTION(h, x)

#endif /* VBF_WRITEINT_ENABLED */


#endif /* TSTWINT_H_INCLUDED */

