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

    This module defines the interface to the multithreaded FIM layer.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nor.h $
    Revision 1.7  2009/12/03 22:01:18Z  garyp
    Migrated all NOR manufacturer IDs into this header from various places.
    Revision 1.6  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/04/03 20:11:20Z  Garyp
    Eliminated an unsed structure.
    Revision 1.4  2006/08/21 22:32:12Z  Garyp
    Added function prototypes.
    Revision 1.3  2006/02/11 00:13:00Z  Garyp
    Renamed the FIMDEVICE structure instantiation for clarity.
    Revision 1.2  2006/02/03 20:19:42Z  Garyp
    Added miscellaneous settings needed for norwrap.c.
    Revision 1.1  2005/10/14 02:08:32Z  Pauli
    Initial revision
    Revision 1.23  2005/05/08 06:39:13Z  garyp
    Factored out some prototypes to different headers.
    Revision 1.22  2004/12/30 23:08:31Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.21  2004/07/06 17:45:34Z  jaredw
    added ID_4_CYCLE struct for amb2x16 fim
    Revision 1.2  2003/12/18 02:08:14Z  garys
    Revision 1.1.1.2  2003/12/18 02:08:14  garyp
    Eliminated the prototypes for standard functions.
    Revision 1.1  2003/12/17 18:37:12Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#ifndef NOR_H_INCLUDED
#define NOR_H_INCLUDED

#include "norread.h"

/*  NOR mnufacturer codes, in numerical order.
*/
#define ID_NA       0x00
#define ID_AMD      0x01
#define ID_FUJITSU  0x04
#define ID_NEC      0x10
#define ID_NUMONYX  0x20
#define ID_MICRON   0x2C
#define ID_SPANSION 0x67
#define ID_INTEL89  0x89
#define ID_INTELB0  0xB0
#define ID_MXIC     0xC2
#define ID_SAMSUNG  0xEC


/*  Possible device types returned in the MediaInfo structure
*/
#define DEVTYPE_MASK    (0x00FF)
#define DEVFLAGS_MASK   (0xFF00)
#define DEV_NOR         DEVTYPE_NOR
#define DEV_ISWF        DEVTYPE_ISWF


/*  Error return value from ErasePoll()
*/
#define FIMMT_ERASE_FAILED ((D_UINT32) -1)

/*-------------------------------------------------------------------
   The largest array supported is MAX_ARRAY bytes.  This
   must be synced with VBF.H's maximum partition size.
-------------------------------------------------------------------*/
#define MAX_ARRAY       (2047UL * 1024UL * 1024UL)


/*  Used for ID codes with 3 words of Device IDs
*/
typedef struct
{
    D_UINT16        uManufCode;
    D_UINT16        uDeviceCode1;
    D_UINT16        uDeviceCode2;
    D_UINT16        uDeviceCode3;
} ID_4_CYCLE;


/*-----------------------------------------------
    Handy-dandy macro to declare a NOR FIM Device
-----------------------------------------------*/
#if 0                           /* Enable this code when we need it */

/* NOTE that this macro works nicely, however the ARM ADS tools
   complain when trying to compile it.
*/
#define DECLARE_NOR_FIMDEVICE(nam)                  \
FIMDEVICE FFXFIM_##nam =                         \
{                                                   \
    Mount,                                          \
    Unmount,                                        \
    Read,                                           \
    Write,                                          \
    EraseStart,                                     \
    ErasePoll,                                      \
    EraseSuspend,                                   \
    EraseResume                                     \
};
#endif


/*-----------------------------------------------
    Prototypes for NOR helper functions.
-----------------------------------------------*/
D_UINT32 FfxFimNorWindowCreate(FFXDEVHANDLE hDev, D_UINT32 ulOffset, const FFXFIMBOUNDS *pBounds, volatile void **ppMedia);
D_UINT32 FfxFimNorWindowMap(   FFXDEVHANDLE hDev, D_UINT32 ulOffset, volatile void **ppMedia);



#endif /* NOR_H_INCLUDED */

