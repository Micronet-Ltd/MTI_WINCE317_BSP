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
  jurisdictions.  Patents may be pending.

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

    This header defines the interface to the profiler.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprof.h $
    Revision 1.7  2009/11/16 03:48:56Z  garyp
    Updated to declare the label pointer as "const".
    Revision 1.6  2009/11/11 16:08:16Z  garyp
    Updated to define DclProfContextName() even if the profiler is disabled.
    Revision 1.5  2009/11/10 22:52:10Z  garyp
    Updated to initialize the profiler as a service.  
    Revision 1.4  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.3  2006/08/21 23:25:30Z  Garyp
    Added prototypes.
    Revision 1.2  2006/01/04 03:03:25Z  Garyp
    Changed ulProfFlags so that it is not static.  This was not necessary to
    start with, and with the updated profiler code, the address of this
    variable is used to calculate stack depth.
    Revision 1.1  2005/10/02 02:37:02Z  Pauli
    Initial revision
    Revision 1.4  2005/09/16 00:46:07Z  garyp
    Added DclProfToggle().
    Revision 1.3  2005/08/01 23:10:34Z  pauli
    Updated macros to use new profiler enter which takes a ulUserData parameter.
    Revision 1.2  2005/07/31 01:49:45Z  Garyp
    Modified DclProfilerLeave() to take ulUserData parameter which is summed
    and displayed in the summary statistics.
    Revision 1.1  2005/07/25 20:59:06Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPROF_H_INCLUDED
#define DLPROF_H_INCLUDED

#if DCLCONF_PROFILERENABLED

#define         DclProfServiceInit      DCLFUNC(DclProfServiceInit)
#define         DclProfEnter            DCLFUNC(DclProfEnter)
#define         DclProfLeave            DCLFUNC(DclProfLeave)
#define         DclProfContextName      DCLFUNC(DclProfContextName)
#define         DclProfDumpCallTree     DCLFUNC(DclProfDumpCallTree)

DCLSTATUS       DclProfServiceInit(DCLINSTANCEHANDLE hDclInst);
void            DclProfEnter(const char *pszLabel, D_UINT32 *pulProfFlags, D_UINT32 ulUserData);
void            DclProfLeave(const char *pszLabel, D_UINT32 *pulProfFlags, D_UINT32 ulUserData);
const char *    DclProfContextName(void);
void            DclProfDumpCallTree(void);

#define DCLPROF_TOPLEVEL    0x00000001
#define DCLPROF_IGNORE      0x00000002

#define DclProfilerRecord(_pszLabel, flg, usr)          \
    DclProfilerEnter(_pszLabel, flg, usr);              \
    DclProfilerLeave(0);

#define DclProfilerEnter(_pszLabel, flg, usr)           \
    {                                                   \
        D_UINT32            ulProfFlags = (flg);        \
        static const char  *pszLabel = (_pszLabel);     \
        DclProfEnter(pszLabel, &ulProfFlags, (usr));

#define DclProfilerLeave(usr)                           \
        DclProfLeave(pszLabel, &ulProfFlags, (usr));    \
    }

#else

#define DclProfilerRecord(_pszLabel, flg, usr)
#define DclProfilerEnter(_pszLabel, flg, usr)
#define DclProfilerLeave(usr)
#define DclProfContextName()      NULL

#endif


#endif /*  #ifndef DLPROF_H_INCLUDED */
