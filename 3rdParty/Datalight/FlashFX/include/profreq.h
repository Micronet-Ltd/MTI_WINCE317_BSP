/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This header defines the interface used to build profiler requests to be
    processed through the external API.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: profreq.h $
    Revision 1.3  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/02/13 09:15:38Z  Garyp
    Updated to new external API interface.
    Revision 1.1  2006/01/01 06:00:36Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef PROFREQ_H_INCLUDED
#define PROFREQ_H_INCLUDED

#include <fxiosys.h>

/*---------------------------------------------------------
    NOTE! All the REQ_... structures <MUST> have a REQHDR
          structure as the first field!.
---------------------------------------------------------*/
typedef struct
{
    FFXIOREQUEST    req;
} REQ_PROFENABLE;

typedef struct
{
    FFXIOREQUEST    req;
} REQ_PROFDISABLE;

typedef struct
{
    FFXIOREQUEST    req;
    D_BOOL          fReset;
} REQ_PROFSUMMARY;

typedef struct
{
    FFXIOREQUEST    req;
    D_BOOL          fSummaryReset;
} REQ_PROFTOGGLE;


#endif /* PROFREQ_H_INCLUDED */

