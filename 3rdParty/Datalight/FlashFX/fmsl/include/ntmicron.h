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

    This header contains Micron specific command-codes and ID values.

    Note that while this header might be named "ntmicron", it may be used
    by NTMs other than the Micron NTM, if they happen to support Micron
    parts.

    Note that some of this information is applicable to Numonyx parts.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ntmicron.h $
    Revision 1.1  2010/06/19 17:01:04Z  garyp
    Initial revision
    Revision 1.7  2010/06/19 04:33:48Z  garyp
    Corrected the ID4 plane size mask.
    Revision 1.6  2010/05/27 21:35:32Z  garyp
    Updated to add the "ECC enabled" flag.
    Revision 1.5  2010/05/26 17:56:09Z  glenns
    Add support for Micron M60 34nm family of devices.
    Revision 1.4  2009/11/30 01:19:48Z  garyp
    Filled in missing definitions and updated to compile if Numonyx support
    is enabled.
    Revision 1.3  2009/07/24 23:27:52Z  garyp
    Merged from the v4.0 branch.  Added support for Micron lock commands.
    Added a "MICRON" prefix to Micron specific commands.
    Revision 1.2  2009/03/03 17:47:12Z  glenns
    Fixed Bug 2478.  The offending trigraph grenades (AKA stupid compiler)
    were placeholders for quantities yet to be defined, so replaced them
    with the word "Undefined".
    Revision 1.1  2008/05/06 02:09:36Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef NTMICRON_H_INCLUDED
#define NTMICRON_H_INCLUDED

#if FFXCONF_NANDSUPPORT_MICRON || FFXCONF_NANDSUPPORT_NUMONYX

/*---------------------------------------------------------
    Micron ID Byte 2

    Values which might be an obvious progression are left
    undefined if they were not specified in the immediately
    available specs.
---------------------------------------------------------*/
#define MICRONID2_DIE_PER_CE_MASK           (0x03)
#define MICRONID2_DIE_PER_CE_1              (0x00)
#define MICRONID2_DIE_PER_CE_2              (0x01)
#define MICRONID2_DIE_PER_CE_4              (0x02) /* from Numonyx spec */
#define MICRONID2_DIE_PER_CE_8              (0x03) /* from Numonyx spec */
#define MICRONID2_CELL_TYPE_MASK            (0x0C)
#define MICRONID2_CELL_TYPE_SLC             (0x00)
#define MICRONID2_CELL_TYPE_MLC4            (0x04)
#define MICRONID2_CELL_TYPE_MLC8            (0x08) /* from Numonyx spec */
#define MICRONID2_CELL_TYPE_MLC16           (0x0C) /* from Numonyx spec */
#define MICRONID2_SIMUL_PAGES_MASK          (0x30)
#define MICRONID2_SIMUL_PAGES_1             (0x00)
#define MICRONID2_SIMUL_PAGES_2             (0x10)
#define MICRONID2_SIMUL_PAGES_4             (0x20) /* from Numonyx spec */
#define MICRONID2_SIMUL_PAGES_8             (0x30) /* from Numonyx spec */
#define MICRONID2_DIE_INTERLEAVE            (0x40)
#define MICRONID2_CACHE_PROGRAM             (0x80)

/*---------------------------------------------------------
    Micron ID Byte 3

    Values which might be an obvious progression are left
    undefined if they were not specified in the immediately
    available specs.
---------------------------------------------------------*/
#define MICRONID3_PAGE_SIZE_MASK            (0x03)
#define MICRONID3_PAGE_SIZE_1KB             (0x00) /* from Numonyx spec */
#define MICRONID3_PAGE_SIZE_2KB             (0x01)
#define MICRONID3_PAGE_SIZE_4KB             (0x02) 
#define MICRONID3_PAGE_SIZE_8KB             (0x03) /* from Numonyx spec */
#define MICRONID3_SPARE_SIZE_64B            (0x04) /* same bit for 218B spares! (Numonyx definition differs) */
#define MICRONID3_BLOCK_SIZE_MASK           (0x30)
#define MICRONID3_BLOCK_SIZE_64KB           (0x00) /* from Numonyx spec */
#define MICRONID3_BLOCK_SIZE_128KB          (0x10)
#define MICRONID3_BLOCK_SIZE_256KB          (0x20) /* from Numonyx spec */
#define MICRONID3_BLOCK_SIZE_512KB          (0x30)
#define MICRONID3_INTERFACE_WIDTH_X16       (0x40)
#define MICRONID3_SERIAL_ACCESS_MASK        (0x88)
#define MICRONID3_SERIAL_ACCESS_20NS        (0x08) /* Micron M60a spec has 0xxxxxx0b */
#define MICRONID3_SERIAL_ACCESS_25NS        (0x80) /* Micron M60a spec has 1xxxxxx0b */
#define MICRONID3_SERIAL_ACCESS_35NS        (0x00) /* 30/50 from Numonyx spec */

/*---------------------------------------------------------
    Micron ID Byte 4

    Values which might be an obvious progression are left
    undefined if they were not specified in the immediately
    available specs.
---------------------------------------------------------*/
#define MICRONID4_ONDIE_EDC_MASK            (0x03)
#define MICRONID4_ONDIE_EDC_4BIT            (0x02)
#define MICRONID4_PLANES_PER_CE_MASK        (0x0C)
#define MICRONID4_PLANES_PER_CE_1           (0x00)
#define MICRONID4_PLANES_PER_CE_2           (0x04)
#define MICRONID4_PLANES_PER_CE_4           (0x08)
#define MICRONID4_PLANES_PER_CE_8           (0x0C) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_MASK           (0x70)
#define MICRONID4_PLANE_SIZE_64MB           (0x00) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_128MB          (0x10) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_256MB          (0x20) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_512MB          (0x30) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_1GB            (0x40) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_2GB            (0x50)
#define MICRONID4_PLANE_SIZE_4GB            (0x60) /* from Numonyx spec */
#define MICRONID4_PLANE_SIZE_8GB            (0x70) /* from Numonyx spec */
#define MICRONID4_ECC_ENABLED               (0x80) /* M60 on-die ECC enabled */

/*---------------------------------------------------------
    Micron Specific Commands

    At whatever point we might determine that these
    commands apply to a larger audience than just
    Micron NAND, we will probably migrate them into
    nandcmd.h.
---------------------------------------------------------*/
#define MICRON_CMD_READ_LOCK_STATUS             (0x7A)
#define MICRON_CMD_READ_CACHE_MODE              (0x31)
#define MICRON_CMD_READ_CACHE_MODE_LAST         (0x3F)
#define MICRON_CMD_READ_PAGE_DUALPLANE_RANDOM   (0x06)
#define MICRON_CMD_PROGRAM_NEXTPLANE            (0x11)
#define MICRON_CMD_PROGRAM_CACHE_MODE           (0x15)
#define MICRON_CMD_UNLOCK1                      (0x23)
#define MICRON_CMD_UNLOCK2                      (0x24)
#define MICRON_CMD_LOCK                         (0x2A)
#define MICRON_CMD_LOCKTIGHT                    (0x2C)
#define MICRON_CMD_GET_FEATURE                  (0xEE)
#define MICRON_CMD_SET_FEATURE                  (0xEF)

/*---------------------------------------------------------
    Get/Set Feature Details
---------------------------------------------------------*/
#define MICRON_FEATURE_ADDRESS_ARRAY_OP         (0x90)
#define MICRON_FEATURE_ENABLE_ECC               (0x08)
#define MICRON_FEATURE_DISABLE_ECC              (0x00)

/*---------------------------------------------------------
    Micron lock status bits, as returned by the
    MICRON_CMD_READ_LOCK_STATUS command.
---------------------------------------------------------*/
#define MICRON_LOCKSTAT_MASK                    (0x07)
#define MICRON_LOCKSTAT_NOT_SUPPORTED           (0x00) /* LOCK pin is low */
#define MICRON_LOCKSTAT_LOCKED_TIGHT            (0x01)
#define MICRON_LOCKSTAT_LOCKED                  (0x02)
#define MICRON_LOCKSTAT_UNLOCKED_TIGHT          (0x05)
#define MICRON_LOCKSTAT_UNLOCKED                (0x06)
#define MICRON_LOCKSTAT_TIGHT_BIT               (0x01)
#define MICRON_LOCKSTAT_UNLOCKED_BIT            (0x04)


#endif  /* FFXCONF_NANDSUPPORT_MICRON */
#endif  /* NTMICRON_H_INCLUDED */


