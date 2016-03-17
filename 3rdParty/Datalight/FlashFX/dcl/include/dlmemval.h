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

---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlmemval.h $
    Revision 1.1  2009/06/27 00:44:42Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLMEMVAL_H_INCLUDED
#define DLMEMVAL_H_INCLUDED

typedef struct
{
    D_UINT32        ulAutomaticBytes;
    D_UINT32        ulOSMemPoolBytes;
    D_UINT32        ulDLMemMgrBytes;
    D_UINT32        ulExplicitBytes;
    D_UINT32        ulInactiveOSMemPoolBytes;
    D_UINT32        ulInactiveDLMemMgrBytes;
    D_UINT32        ulInactiveExplicitBytes;
    unsigned        nAutomaticBlocks;
    unsigned        nOSMemPoolBlocks;
    unsigned        nDLMemMgrBlocks;
    unsigned        nExplicitBlocks;
    unsigned        nInactiveOSMemPoolBlocks;
    unsigned        nInactiveDLMemMgrBlocks;
    unsigned        nInactiveExplicitBlocks;
} DCLMEMVALSUMMARY;

typedef struct
{
    DCLIOREQUEST    ior;
    unsigned        nNewMode;
    unsigned        nOldMode;
} DCLREQ_MEMVAL_ERRORMODE;

typedef struct
{
    DCLIOREQUEST        ior;
    DCLMEMVALSUMMARY    MVS;
    D_BOOL              fQuiet;
    D_BOOL              fReset;
} DCLREQ_MEMVAL_SUMMARY;

typedef struct
{
    DCLIOREQUEST        ior;
    const void         *pBuffer;
    D_UINT32            ulBuffLen;
    unsigned            nFlags;
} DCLREQ_MEMVAL_BUFFERADD;

typedef struct
{
    DCLIOREQUEST        ior;
    const void         *pBuffer;
} DCLREQ_MEMVAL_BUFFERREMOVE;

typedef struct
{
    DCLIOREQUEST        ior;
    const void         *pBuffer;
    D_UINT32            ulBuffLen;
    unsigned            nFlags;
} DCLREQ_MEMVAL_BUFFERVALIDATE;

#define DCL_MVFLAG_RESERVEDMASK     (0xFFFF0000U)
#define DCL_MVFLAG_READONLY         (0x00008000U)
#define DCL_MVFLAG_TYPEMASK         (0x00000003U)
#define DCL_MVFLAG_TYPEAUTOMATIC    (0x00000000U)
#define DCL_MVFLAG_TYPEOSMEMPOOL    (0x00000001U)
#define DCL_MVFLAG_TYPEDLMEMMGR     (0x00000002U)
#define DCL_MVFLAG_TYPEEXPLICIT     (0x00000003U)


#define DclMemValServiceInit        DCLFUNC(DclMemValServiceInit)
#define DclMemValErrorMode          DCLFUNC(DclMemValErrorMode)
#define DclMemValSummary            DCLFUNC(DclMemValSummary)
#define DclMemValBufferAdd          DCLFUNC(DclMemValBufferAdd)
#define DclMemValBufferRemove       DCLFUNC(DclMemValBufferRemove)
#define DclMemValBufferValidate     DCLFUNC(DclMemValBufferValidate)

DCLSTATUS DclMemValServiceInit(   DCLINSTANCEHANDLE hDclInst);
unsigned  DclMemValErrorMode(     DCLINSTANCEHANDLE hDclInst, unsigned nNewMode);
DCLSTATUS DclMemValSummary(       DCLINSTANCEHANDLE hDclInst, DCLMEMVALSUMMARY *pVal, D_BOOL fQuiet, D_BOOL fReset);
DCLSTATUS DclMemValBufferAdd(     DCLINSTANCEHANDLE hDclInst, const void *pBuffer, D_UINT32 ulBuffLen, unsigned nFlags);
DCLSTATUS DclMemValBufferRemove(  DCLINSTANCEHANDLE hDclInst, const void *pBuffer);
DCLSTATUS DclMemValBufferValidate(DCLINSTANCEHANDLE hDclInst, const void *pBuffer, D_UINT32 ulBuffLen, unsigned nFlags);


#endif  /* DLMEMVAL_H_INCLUDED */

