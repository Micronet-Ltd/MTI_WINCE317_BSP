/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    Command codes and status flags for NAND flash chips. 

    This header should contain only those commands and flags which are
    considered to be generic, and apply across a broad spectrum of parts
    from different flash manufacturers (or at least more than one).
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandcmd.h $
    Revision 1.16  2011/11/21 17:40:54Z  glenns
    Add command set for Micron Enhanced ClearNAND.
    Revision 1.15  2011/02/10 18:04:47Z  glenns
    Corrected ONFI READ_PARAM command.
    Revision 1.14  2010/12/03 22:20:34Z  glenns
    Add and ONFI 2.3 command per CSS request.
    Revision 1.13  2010/09/29 19:46:23Z  glenns
    Back out previous change pending product announcement.
    Revision 1.12  2010/08/05 23:17:22Z  glenns
    Add [TRADE NAME TBA] bit-mask.
    Revision 1.11  2010/07/06 02:24:17Z  garyp
    Eliminated some deprecated symbols.
    Revision 1.10  2010/07/05 23:24:14Z  garyp
    Updated the NAND status flags.
    Revision 1.9  2010/06/19 17:01:02Z  garyp
    Moved Micron specific commands to ntmicron.h.
    Revision 1.8  2010/06/19 00:24:29Z  garyp
    Added some Micron commands.
    Revision 1.7  2010/05/26 18:00:09Z  glenns
    Add support for Micron M60 34nm family of devices.
    Revision 1.6  2008/05/06 02:09:34Z  garyp
    Moved Micron specific commands to ntmicron.h.
    Revision 1.5  2008/01/30 22:26:46Z  Glenns
    Added commands to unlock blocks (ostensibly for STMicro parts,
    but others may use them as well)
    Revision 1.4  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/09/12 22:33:05Z  Garyp
    Added some Micron specific commands.
    Revision 1.2  2007/06/27 22:50:28Z  timothyj
    Added ORNAND-specific partial page automatic seek command.
    Revision 1.1  2006/02/22 19:06:54Z  billr
    Initial revision
    Based on ntbyteio.c
---------------------------------------------------------------------------*/

#ifndef NANDCMD_H_INCLUDED
#define NANDCMD_H_INCLUDED

/*  Commands for the NAND flash device
    See specification sheets for NAND parts for detailed
    information.
*/

/*  Read commands.
*/
#define CMD_READ_MAIN                   0x00
#define CMD_READ_MAIN_START             0x30
#define CMD_READ_SEGMENT_START          0x31    /* ORNAND-specific command */
#define CMD_READ_BACK                   0x00
#define CMD_READ_BACK_START             0x35
#define CMD_READ_PAGE_RANDOM            0x05
#define CMD_READ_PAGE_RANDOM_START      0xE0
#define CMD_READ_SPARE                  0x50    /* no confirmation byte */

/*  Programming commands, all potentially have a confirmation byte
*/
#define CMD_PROGRAM                     0x80
#define CMD_WRITE_PAGE_RANDOM           0x85
#define CMD_PROGRAM_START               0x10
#define CMD_WRITE_BACK                  0x85
#define CMD_WRITE_BACK_START            0x10
#define CMD_BLOCK_UNLOCK_START			0x23	/* Some Numonyx parts */
#define CMD_BLOCK_UNLOCK_END			0x24	/* Some Numonyx parts */

/*  Erase commands
*/
#define CMD_ERASE_BLOCK                 0x60
#define CMD_ERASE_BLOCK_START           0xD0

/*  Single byte commands
*/
#define CMD_STATUS                      0x70
#define CMD_READ_ID                     0x90
#define CMD_READ_SN                     0xED
#define CMD_READ_PARAM                  0xEC    /* ONFI 2.3 */
#define CMD_RESET                       0xFF

/*  Special Read command for AMD Ultra-Nand
*/
#define CMD_READ_GAPLESS                0x02

/*  UNUSED
*/
#define CMD_RESUME                      0xD0
#define CMD_SUSPEND                     0xB0
#define CMD_REGISTER_RD                 0xE0


/*---------------------------------------------------------
    Status Register Bits

    NANDSTAT_ERROR is only valid when NANDSTAT_READY
    is set.
---------------------------------------------------------*/
#define NANDSTAT_ERROR                  0x01
#define NANDSTAT_DATA_CORRECTED         0x08    /* Micron */
#define NANDSTAT_CACHE_READY            0x20    /* Micron and Numonyx, at least */
#define NANDSTAT_READY                  0x40
#define NANDSTAT_WRITE_ENABLE           0x80

/*  COMMANDS for MICRON Clear NAND Part
*/
#define CMD_LUN_STATUS                  0x71
#define CMD_DEV_STATUS                  0x72
#define CMD_GET_NEXT_OPERATION_STATUS   0x77
#define CMD_OPERATION_STATUS            0x7D
#define CMD_READ_PARAM_PAGE             0xEC
#define CMD_RESET_LUN                   0xFA
#define CMD_QUEUED_READ                 0x07
#define CMD_QUEUED_READ_START           0x37
#define CMD_QUEUED_WRITE                0x80
#define CMD_QUEUED_WRITE_START          0x11




/*---------------------------------------------------------
    Micron enhanced clear NAND Device Status Register Bits

    NANDSTAT_ERROR is only valid when NANDSTAT_READY
    is set.
---------------------------------------------------------*/
#define NAND_DEV_STAT_PENDING_ERROR                 0x01
#define NAND_DEV_STAT_READY_READ_ERASE              0x02    
#define NAND_DEV_STAT_OPSTAT_AVAIL                  0x04    
#define NAND_DEV_STAT_READY_PROG_OP                 0x08
#define NAND_DEV_STAT_NO_PENDING_COMMANDS           0x20
#define NAND_DEV_STAT_READY                         0x40

/*---------------------------------------------------------
    Operation Status Bits to Wait On.
---------------------------------------------------------*/
#define OP_STAT_OPERATION_ERROR          (0x01)
#define OP_STAT_REFRESH_BLOCK            (0x02)
#define OP_STAT_RETIRE_BLOCK             (0x04)
#define OP_STAT_PAGE_NOT_PROGRAMMED      (0x08)
#define OP_STAT_VALID                    (0x40)


#endif  /* NANDCMD_H_INCLUDED */


