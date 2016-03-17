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

    This header defines the interface between the MX51 NTM and its hooks
    functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmx51.h $
    Revision 1.1  2011/11/22 14:35:30Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHMX51_H_INCLUDED
#define FHMX51_H_INCLUDED


/*---------------------------------------------------------
    Structure: FFXMX51PARAMS

    This structure contains the NTMX51 configuration
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
    unsigned    fSupportCacheModeReads  : 1;
    unsigned    fSupportCacheModeWrites : 1;
    unsigned    fSupportDualPlanes      : 1;
} FFXMX51PARAMS;

/*--------------------------------------------------------
    Function prototype for the NTMX51 hook interface
--------------------------------------------------------*/
PNANDCTL        FfxHookNTMX51Create(        FFXDEVHANDLE hDev,FFXMX51PARAMS *pParams);
FFXSTATUS       FfxHookNTMX51Destroy(       PNANDCTL pNC);
D_BOOL          FfxHookNTMX51GetIPBaseAdr( PNANDCTL pNC, void **pBuffer);

#endif  /* FHMX51_H_INCLUDED */

