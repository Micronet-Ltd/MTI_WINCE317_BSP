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

    This header defines the interface to SXIP kernel IOCTLs.

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                           Revision History
    $Log: sxip.h $
    Revision 1.2  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2005/10/02 01:35:00Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 02:35:00Z  Garyp
    Initial revision
    Revision 1.1  2004/12/31 01:48:18Z  pauli
    Initial revision
    Revision 1.1  2004/12/31 00:48:18Z  pauli
    Initial revision
    Revision 1.2  2004/12/30 23:48:17Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.1  2002/11/14 00:18:28Z  qa
    Initial revision
---------------------------------------------------------------------------*/


#define KERNEL_IOCTL_SXIP_PROBE 1
#define KERNEL_IOCTL_SXIP_WRITE 2
#define KERNEL_IOCTL_SXIP_ERASE 3


typedef struct probe_input
{
    void           *pFlashBase;
    D_UINT32       windowSize;
} PROBE_INPUT;

typedef struct write_input
{
    void           *pFlash;
    void           *pData;
    D_UINT32       length;
} WRITE_INPUT;

typedef struct erase_input
{
    void           *pFlash;
} ERASE_INPUT;
