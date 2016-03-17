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

    This header defines the interface between the Standard Clear-NAND NTM and 
    its hooks functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhstdclrnand.h $
    Revision 1.5  2011/12/12 22:12:38Z  jimmb
    Removed the added API modification
    Revision 1.4  2011/12/12 21:45:12Z  jimmb
    Removed board specific register definitions
    Revision 1.3  2011/12/12 20:13:12Z  jimmb
    Panda project and Standard ClearNAND update from GL
    Revision 1.2  2011/12/08 11:16:22Z  jimmb
    Added GPMC register memory mappings
    Revision 1.1  2011/10/15 01:07:52Z  glenns
    Initial revision
    Revision 1.1  2011/10/04
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHSTDCLRNAND_H_INCLUDED
#define FHSTDCLRNAND_H_INCLUDED

#define MAX_CHIPS                 (1)   /* Max chips supported -- CS0 to CS(MAX_CHIPS-1) */

/*----------------------------------------------------------------
 Function prototypes for the Standard ClearNAND NTM hook interface.
------------------------------------------------------------------*/
PNANDCTL        FfxHookNTSTDCLRNandCreate(                  FFXDEVHANDLE hDev);
FFXSTATUS       FfxHookNTSTDCLRNandDestroy(                 PNANDCTL pNC);
FFXSTATUS       FfxHookNTSTDCLRNandSetChipSelect(           PNANDCTL pNC, D_UINT32 ulAddress, CHIP_SELECT_MODE mode);
FFXSTATUS       FfxHookNTSTDCLRNandReadyWait(               PNANDCTL pNC);
FFXSTATUS       FfxHookNTSTDCLRNandSetChipCommand(          PNANDCTL pNC, D_UCHAR ucCommand);
FFXSTATUS       FfxHookNTSTDCLRNandSetChipCommandNoWait(    PNANDCTL pNC, D_UCHAR ucCommand);
void            FfxHookNTSTDCLRNandDataIn(                  PNANDCTL pNC, NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
void            FfxHookNTSTDCLRNandDataOut(                 PNANDCTL pNC, const NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
NAND_IO_TYPE    FfxHookNTSTDCLRNandStatusIn(                PNANDCTL pNC);
void            FfxHookNTSTDCLRNandAddressOut(              PNANDCTL pNC, D_UCHAR address);

#endif  /* FHSTDCLRNAND_H_INCLUDED */


