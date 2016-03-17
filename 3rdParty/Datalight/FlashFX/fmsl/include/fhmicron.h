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
  jurisdictions.  The software may be subject to one or more US patents, 
  including without limitation US Patents: US#5860082, US#6260156.  Patents
  may be pending.

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
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines the interface between the Micron NTM and its hooks
    functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmicron.h $
    Revision 1.5  2010/07/16 15:24:27Z  garyp
    Updated so the hooks module provides the maximum number of chips
    count to the NTM.
    Revision 1.4  2010/07/04 19:58:33Z  garyp
    Prototypes updated for the revamped hooks abstraction.  Renamed
    some variables for clarity.
    Revision 1.3  2010/06/25 20:06:31Z  glenns
    Add support for Micron M60 boot blocks
    Revision 1.2  2009/12/19 23:05:18Z  garyp
    Added the nAlignment field to FFXMICRONPARAMS.
    Revision 1.1  2009/12/11 18:50:10Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHMICRON_H_INCLUDED
#define FHMICRON_H_INCLUDED


/*---------------------------------------------------------
    Structure: FFXMICRONPARAMS

    This structure contains the NTMICRON configuration
    parameters.  It is filled in by the hook's Create()
    function, and provides a mechanism for the hook code
    to communicate run-time, project specific information
    to the NTM.
---------------------------------------------------------*/
typedef struct
{
    size_t      nStructLen;
    unsigned    nMaxChips;
    unsigned    nTagOffset;
    unsigned    nAlignment;
    unsigned    nM60EDC4Chip;       /* Chip for which ulM60EDC4Block applies */
    D_UINT32    ulM60EDC4Block;     /* First block on nM60EDC4Chip which uses on-die 4-bit EDC */
    unsigned    fSupportCacheModeReads  : 1;
    unsigned    fSupportCacheModeWrites : 1;
    unsigned    fSupportDualPlanes      : 1;
} FFXMICRONPARAMS;


/*---------------------------------------------------------
    Function prototypes for the Micron NTM hook interface.
---------------------------------------------------------*/
NTMHOOKHANDLE   FfxHookNTMicronCreate(         FFXDEVHANDLE hDev, FFXMICRONPARAMS *pParams);
FFXSTATUS       FfxHookNTMicronDestroy(        NTMHOOKHANDLE hNtmHook);
FFXSTATUS       FfxHookNTMicronConfigure(      NTMHOOKHANDLE hNtmHook, unsigned nChip, const FFXNANDCHIP *pChipInfo, D_BYTE *pabID);
FFXSTATUS       FfxHookNTMicronSetChipSelect(  NTMHOOKHANDLE hNtmHook, unsigned nChip, CHIP_SELECT_MODE mode);
FFXSTATUS       FfxHookNTMicronReadyWait(      NTMHOOKHANDLE hNtmHook);
FFXSTATUS       FfxHookNTMicronSetChipCommand( NTMHOOKHANDLE hNtmHook, D_BYTE bCommand);
FFXSTATUS       FfxHookNTMicronSetChipCommandNoWait(NTMHOOKHANDLE hNtmHook, D_BYTE bCommand);
void            FfxHookNTMicronDataIn(         NTMHOOKHANDLE hNtmHook,       NAND_IO_TYPE *pBuffer, unsigned nCount);
void            FfxHookNTMicronDataOut(        NTMHOOKHANDLE hNtmHook, const NAND_IO_TYPE *pBuffer, unsigned nCount);
FFXSTATUS       FfxHookNTMicronPageRead(       NTMHOOKHANDLE hNtmHook,       NAND_IO_TYPE *pBuffer, unsigned nCount, D_BUFFER *pabECC);
FFXSTATUS       FfxHookNTMicronPageWrite(      NTMHOOKHANDLE hNtmHook, const NAND_IO_TYPE *pBuffer, unsigned nCount, D_BUFFER *pabECC);
NAND_IO_TYPE    FfxHookNTMicronStatusIn(       NTMHOOKHANDLE hNtmHook);
void            FfxHookNTMicronAddressOut(     NTMHOOKHANDLE hNtmHook, D_BYTE bAddress);
D_UINT32        FfxHookNTMicronGetPageStatus(  NTMHOOKHANDLE hNtmHook, const D_BUFFER *pSpare);


#endif  /* FHMICRON_H_INCLUDED */

