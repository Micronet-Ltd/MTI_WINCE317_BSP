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

    OneNAND Register Definitions

    ToDo:  Couldn't/shouldn't the contents of this header simply be included
           in onenandctl.h?
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: onenandregs.h $
    Revision 1.6  2009/08/04 03:19:19Z  garyp
    Merged from the v4.0 branch.  Added cache read support.  Documentation
    cleanup, and one register name changed.
    Revision 1.5  2008/04/17 20:30:03Z  Glenns
    Add support for FlexOneNAND Partition Information commands.
    Revision 1.4  2008/01/30 22:27:44Z  Glenns
    Added command to access Partition Info for FlexOneNAND
    Revision 1.3  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/03/09 00:17:12Z  timothyj
    Added the length of the register map, for use in the project hooks’ default
    implementations.
    Revision 1.1  2006/02/03 21:00:20Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#ifndef ONENANDREGS_H_INCLUDED
#define ONENANDREGS_H_INCLUDED

/*  Size (in access-size units - depends on configuration) of
    the OneNAND register map
*/
#define ONENAND_REGS_EXTENT                     0x10000

/*  Register Map (word order)
*/
#define ONENAND_REG_BOOTRAM_MAIN                0x0000
#define ONENAND_REG_DATARAM_MAIN                0x0200
#define ONENAND_REG_BOOTRAM_SPARE               0x8000
#define ONENAND_REG_DATARAM_SPARE               0x8010

#define ONENAND_REG_MANUFACTURER_ID             0xF000
#define ONENAND_REG_DEVICE_ID                   0xF001
#define ONENAND_REG_VERSION_ID                  0xF002
#define ONENAND_REG_DATA_BUF_SIZE               0xF003
#define ONENAND_REG_BOOT_BUF_SIZE               0xF004
#define ONENAND_REG_AMOUNT_OF_BUF               0xF005
#define ONENAND_REG_TECHNOLOGY                  0xF006

#define ONENAND_REG_START_ADDRESS_1             0xF100
#define ONENAND_REG_START_ADDRESS_2             0xF101
#define ONENAND_REG_START_ADDRESS_3             0xF102
#define ONENAND_REG_START_ADDRESS_4             0xF103
#define ONENAND_REG_START_ADDRESS_5             0xF104
#define ONENAND_REG_START_ADDRESS_6             0xF105
#define ONENAND_REG_START_ADDRESS_7             0xF106
#define ONENAND_REG_START_ADDRESS_8             0xF107

#define ONENAND_REG_START_BUFFER                0xF200

#define ONENAND_REG_COMMAND                     0xF220
#define ONENAND_REG_SYSTEM_CONFIG_1             0xF221
#define ONENAND_REG_SYSTEM_CONFIG_2             0xF222

#define ONENAND_REG_CONTROLLER_STATUS           0xF240
#define ONENAND_REG_INTERRUPT                   0xF241

#define ONENAND_REG_START_BLOCK_ADDRESS         0xF24C

#define ONENAND_REG_WRITE_PROTECT_STATUS        0xF24E

#define ONENAND_REG_ECC_STATUS                  0xFF00
#define ONENAND_REG_MAIN_RESULT_1               0xFF01
#define ONENAND_REG_SPARE_RESULT_1              0xFF02
#define ONENAND_REG_MAIN_RESULT_2               0xFF03
#define ONENAND_REG_SPARE_RESULT_2              0xFF04
#define ONENAND_REG_MAIN_RESULT_3               0xFF05
#define ONENAND_REG_SPARE_RESULT_3              0xFF06
#define ONENAND_REG_MAIN_RESULT_4               0xFF07
#define ONENAND_REG_SPARE_RESULT_4              0xFF08

/*  OneNAND Device ID Register bits
*/
#define ONENAND_REG_DEVID_DDP_MASK              0x0008
#define ONENAND_REG_DEVID_DENSITY_MASK          0x0070

#define ONENAND_REG_DEVID_128Mb                 0x0000
#define ONENAND_REG_DEVID_256Mb                 0x0010
#define ONENAND_REG_DEVID_512Mb                 0x0020
#define ONENAND_REG_DEVID_1Gb                   0x0030
#define ONENAND_REG_DEVID_2Gb                   0x0040
#define ONENAND_REG_DEVID_4Gb                   0x0050


/*  OneNAND Flash Write Protection Status Register bits
*/
 #define ONENAND_REG_FWPS_LOCKED_TIGHT          0x0001
 #define ONENAND_REG_FWPS_LOCKED                0x0002
 #define ONENAND_REG_FWPS_UNLOCKED              0x0004

/*  BootRAM Partition command definitions
*/
#define ONENAND_BP_CMD_RESET                    0x00F0
#define ONENAND_BP_CMD_LOAD                     0x00E0
#define ONENAND_BP_CMD_READ_ID                  0x0090

/*  Command register definitions
*/
#define ONENAND_REG_CMD_LOAD_DATA               0x0000
#define ONENAND_REG_CMD_LOAD_SPARE              0x0013
#define ONENAND_REG_CMD_PROGRAM_DATA            0x0080
#define ONENAND_REG_CMD_PROGRAM_SPARE           0x001A
#define ONENAND_REG_CMD_COPY_BACK_PGM           0x001B
#define ONENAND_REG_CMD_UNLOCK_BLOCK            0x0023
#define ONENAND_REG_CMD_LOCK_BLOCK              0x002A
#define ONENAND_REG_CMD_LOCK_TIGHT_BLOCK        0x002C
#define ONENAND_REG_CMD_ERASE_VERIFY_READ       0x0071
#define ONENAND_REG_CMD_BLOCK_ERASE             0x0094
#define ONENAND_REG_CMD_MULTI_BLOCK_ERASE       0x0095
#define ONENAND_REG_CMD_ERASE_SUSPEND           0x00B0
#define ONENAND_REG_CMD_ERASE_RESUME            0x0030
#define ONENAND_REG_CMD_RESET_NAND_CORE         0x00F0
#define ONENAND_REG_CMD_RESET_ONENAND           0x00F3
#define ONENAND_REG_CMD_OTP_ACCESS              0x0065
#define ONENAND_REG_CMD_PI_ACCESS				0x0066
#define ONENAND_REG_CMD_PI_UPDATE               0x0005
#define ONENAND_REG_CMD_CACHE_READ              0x000E
#define ONENAND_REG_CMD_FINISH_CACHE_READ       0x000C
#define ONENAND_REG_CMD_SYNC_BURST_BLOCK_READ   0x000A

/*  System Configuration Register 1 definitions
*/
#define ONENAND_REG_SCR1_BWPS                   0x0001
#define ONENAND_REG_SCR1_RDY_CONF               0x0010
#define ONENAND_REG_SCR1_IOBE                   0x0020
#define ONENAND_REG_SCR1_INT_POL                0x0040
#define ONENAND_REG_SCR1_RDY_POL                0x0080
#define ONENAND_REG_SCR1_ECC                    0x0100
#define ONENAND_REG_SCR1_MASK_BL                0x0E00
#define ONENAND_REG_SCR1_MASK_BRL               0x7000
#define ONENAND_REG_SCR1_RM                     0x8000

/*  Status register definitions
*/
#define ONENAND_REG_STATUS_TO                   0x0000
#define ONENAND_REG_STATUS_OTP                  0x0040
#define ONENAND_REG_STATUS_RSTB                 0x0080
#define ONENAND_REG_STATUS_SUS                  0x0200
#define ONENAND_REG_STATUS_ERROR                0x0400
#define ONENAND_REG_STATUS_ERASE                0x0800
#define ONENAND_REG_STATUS_PROG                 0x1000
#define ONENAND_REG_STATUS_LOAD                 0x2000
#define ONENAND_REG_STATUS_LOCK                 0x4000
#define ONENAND_REG_STATUS_ONGO                 0x8000

/*  Interrupt Status register definitions
*/
#define ONENAND_REG_INTSTATUS_RSTI              0x0010
#define ONENAND_REG_INTSTATUS_EI                0x0020
#define ONENAND_REG_INTSTATUS_WI                0x0040
#define ONENAND_REG_INTSTATUS_RI                0x0080
#define ONENAND_REG_INTSTATUS_INT               0x8000

/*  BufferRAM BSA value definitions (NOT bits w/in a register unless shifted)
*/
#define ONENAND_VAL_BSA_DATARAM0_0				0x8
#define ONENAND_VAL_BSA_DATARAM0_1				0x9
#define ONENAND_VAL_BSA_DATARAM0_2				0xA
#define ONENAND_VAL_BSA_DATARAM0_3				0xB

#define ONENAND_VAL_BSA_DATARAM1_0				0xC
#define ONENAND_VAL_BSA_DATARAM1_1				0xD
#define ONENAND_VAL_BSA_DATARAM1_2				0xE
#define ONENAND_VAL_BSA_DATARAM1_3				0xF

/*  BufferRAM BSC value definitions (NOT bits w/in a register unless shifted)
*/
#define ONENAND_VAL_BSC_1						0x1
#define ONENAND_VAL_BSC_2						0x2
#define ONENAND_VAL_BSC_3						0x3
#define ONENAND_VAL_BSC_4						0x0

/*  System Configuration 1 Register Burst Read Latency value definitions
    (NOT bits w/in a register unless shifted)
*/
#define ONENAND_VAL_SCR1_BRL_8                  0x0
#define ONENAND_VAL_SCR1_BRL_9                  0x1
#define ONENAND_VAL_SCR1_BRL_10                 0x2
#define ONENAND_VAL_SCR1_BRL_3                  0x3
#define ONENAND_VAL_SCR1_BRL_4                  0x4
#define ONENAND_VAL_SCR1_BRL_5                  0x5
#define ONENAND_VAL_SCR1_BRL_6                  0x6
#define ONENAND_VAL_SCR1_BRL_7                  0x7

#endif  /* ONENANDREGS_H_INCLUDED */

