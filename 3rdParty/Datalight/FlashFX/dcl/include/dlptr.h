/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2008 Datalight, Inc.
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

    This header defines the API to the Datalight Common Libraries.

    This header is included by dcl.h, and therefore does not need to be
    included directly.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlptr.h $
    Revision 1.4  2008/05/08 21:59:16Z  garyp
    Added some missing parens around macro arguments.
    Revision 1.3  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.2  2006/04/04 01:28:31Z  brandont
    Added DclPtrDiff macro.
    Revision 1.1  2005/12/02 16:25:26Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPTR_H_INCLUDED
#define DLPTR_H_INCLUDED


/*-------------------------------------------------------------------
    The following macros are used to perform pointer arithmetic
-------------------------------------------------------------------*/
#define DclPtrInc(ptr)          (void *)(((D_UINT8 *)(ptr)) +  sizeof(*(ptr)))
#define DclPtrDec(ptr)          (void *)(((D_UINT8 *)(ptr)) -  sizeof(*(ptr)))
#define DclPtrAdd(ptr, val)     (void *)(((D_UINT8 *)(ptr)) + (sizeof(*(ptr)) * (val)))
#define DclPtrSub(ptr, val)     (void *)(((D_UINT8 *)(ptr)) - (sizeof(*(ptr)) * (val)))
#define DclPtrAddByte(ptr, val) (void *)(((D_UINT8 *)(ptr)) + (val))
#define DclPtrSubByte(ptr, val) (void *)(((D_UINT8 *)(ptr)) - (val))
#define DclPtrMask(ptr, mask)   (void *)(((D_UINT32)(ptr)) & (mask))
#define DclPtrDiff(ptr1, ptr2)  ((D_UINT32)(((D_UINT8 *)(ptr1)) - ((D_UINT8 *)(ptr2))))


#endif /* DLPTR_H_INCLUDED */
