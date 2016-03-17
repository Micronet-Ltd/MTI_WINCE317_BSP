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

    This header defines the interface between the Enhanced Clear-NAND NTM and 
    its hooks functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fheclrnand.h $
    Revision 1.1  2011/11/17 20:14:22Z  glenns
    Initial revision
    Revision 1.1  2011/03/10
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHECLRNAND_H_INCLUDED
#define FHECLRNAND_H_INCLUDED

/*---------------------------------------------------------
    Different Device Status to wait on.
---------------------------------------------------------*/
#define RESET_WAIT                  (0x20)

#define DEVICE_READY_READ_ERASE     (0x02)
#define DEVICE_OPSTAT_AVAIL         (0x04)
#define DEVICE_READY_PROG           (0x08)
#define DEVICE_READY                (0x40)

/*---------------------------------------------------------
    LUN Status Bits to Wait On.
---------------------------------------------------------*/
#define LUN_STATUS_VALID            (0x40)

#define LUN_COMMAND_IDLE            (LUN_STATUS_VALID | 0x00)
#define LUN_COMMAND_ACTIVE          (LUN_STATUS_VALID | 0x80)
#define LUN_COMMAND_COMPLETE        (LUN_STATUS_VALID | 0x20)

#define LUN_PROGRAM_READY           (LUN_COMMAND_IDLE | 0x04)
#define LUN_READ_ERASE_READY        (LUN_COMMAND_IDLE | 0x02)

/*---------------------------------------------------------
    Function prototypes for the Enhanced ClearNAND NTM hook interface.
---------------------------------------------------------*/
PNANDCTL        FfxHookNTECLRNandCreate(                  FFXDEVHANDLE hDev);
FFXSTATUS       FfxHookNTECLRNandDestroy(                 PNANDCTL pNC);
FFXSTATUS       FfxHookNTECLRNandSetChipSelect(           PNANDCTL pNC, D_UINT32 ulAddress, CHIP_SELECT_MODE mode);
FFXSTATUS       FfxHookNTECLRNandReadyWait(               PNANDCTL pNC, D_UCHAR ucWaitFor, D_UCHAR ucCommand);
FFXSTATUS       FfxHookNTECLRNandSetChipCommand(          PNANDCTL pNC, D_UCHAR ucCommand);
FFXSTATUS       FfxHookNTECLRNandSetChipCommandNoWait(    PNANDCTL pNC, D_UCHAR ucCommand);
void            FfxHookNTECLRNandDataIn(                  PNANDCTL pNC, NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
void            FfxHookNTECLRNandDataOut(                 PNANDCTL pNC, const NAND_IO_TYPE *pBuffer, D_UINT16 uCount);
NAND_IO_TYPE    FfxHookNTECLRNandStatusIn(                PNANDCTL pNC);
void            FfxHookNTECLRNandAddressOut(              PNANDCTL pNC, D_UCHAR address);

#endif  /* FHECLRNAND_H_INCLUDED */

