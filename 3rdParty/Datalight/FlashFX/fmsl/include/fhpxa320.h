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

    This header defines the interface between the PXA320 NTM and its hooks
    functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhpxa320.h $
    Revision 1.4  2010/06/24 23:00:59Z  garyp
    Corrected a broken comment.
    Revision 1.3  2010/06/19 02:24:43Z  garyp
    Added fields to support the Micron M60.
    Revision 1.2  2009/12/10 18:17:19Z  garyp
    Removed the NTMHOOKHANDLE definition, which now resides in common code.
    Revision 1.1  2009/12/01 20:27:30Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NTPXA320_H_INCLUDED
#define NTPXA320_H_INCLUDED


/*---------------------------------------------------------
    Structure: FFXPXA320PARAMS

    This structure contains the NTPXA320 configuration
    parameters.  It is filled in by the hook's Create()
    function, and provides a mechanism for the hook code
    to communicate run-time, project specific information
    to the NTM.
---------------------------------------------------------*/
typedef struct
{
    size_t              nStructLen;
    volatile D_UINT32  *pulPXA320Base;
    D_UINT32            ulDefaultTR0;
    D_UINT32            ulDefaultTR1;
    D_UINT32            ulM60EDC4Block;     /* First block on nM60EDC4Chip which uses on-die 4-bit EDC */
    unsigned            nM60EDC4Chip;       /* Chip for which ulM60EDC4Block applies */
    unsigned            fSupportCacheMode   : 1;
    unsigned            fSupportCS1         : 1;
    unsigned            fSupportM60OnDieEDC : 1;
} FFXPXA320PARAMS;


/*---------------------------------------------------------
    Function prototypes for the PXA320 hook interface.
---------------------------------------------------------*/
NTMHOOKHANDLE   FfxHookNTPxa320Create(   FFXDEVHANDLE hDev, FFXPXA320PARAMS *pParams);
FFXSTATUS       FfxHookNTPxa320Destroy(  NTMHOOKHANDLE hNtmHook);
FFXSTATUS       FfxHookNTPxa320Configure(NTMHOOKHANDLE hNtmHook, const FFXNANDCHIP *pChipInfo, D_BYTE *pabID);



#endif  /* NTPXA320_H_INCLUDED */

