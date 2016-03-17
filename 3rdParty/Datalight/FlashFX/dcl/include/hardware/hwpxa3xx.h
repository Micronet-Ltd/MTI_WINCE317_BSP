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

    This module contains types and macros for interfacing with the Marvell
    PXA3xx platform.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: hwpxa3xx.h $
    Revision 1.2  2009/12/03 02:55:50Z  garyp
    Added a number of DFC specific register definitions.
    Revision 1.1  2008/12/11 19:23:50Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef HWPXA3XX_H_INCLUDED
#define HWPXA3XX_H_INCLUDED


/*---------------------------------------------------------
    Timers
---------------------------------------------------------*/
#define PXA3XX_TIMER_BASE               (0x40A00000)


/*---------------------------------------------------------
    PXA3xx DFC Registers and Settings
---------------------------------------------------------*/
#define PXA3XX_DFC_REGS_OFFSET          (0x43100000UL)
#define PXA3XX_DFC_REGS_EXTENT          (0x00010000UL)

#define PXA3XX_DFCREG(base, offs)       (*(volatile D_UINT32*)((char*)(base) + (offs)))

#define PXA3XX_DFC_MINIMUM_IO_SIZE      (8)

#define BIT(n)                          (1U << (n))
#define BITS(high, low)                 ((BIT(high) | (BIT(high) - 1)) & ~(BIT(low) - 1))


/*---------------------------------------------------------
    NDCR -- NAND Controller Control Register
---------------------------------------------------------*/
#define PXA3XX_NDCR(base)               PXA3XX_DFCREG(base, 0x00)
#define PXA3XX_NDCR_SPARE_EN            BIT(31)
#define PXA3XX_NDCR_ECC_EN              BIT(30)
#define PXA3XX_NDCR_DMA_EN              BIT(29)
#define PXA3XX_NDCR_ND_RUN              BIT(28)
#define PXA3XX_NDCR_DWIDTH_C            BIT(27)
#define PXA3XX_NDCR_DWIDTH_M            BIT(26)
#define PXA3XX_NDCR_PAGE_SZ_MASK        BITS(25, 24)
#define PXA3XX_NDCR_PAGE_SZ_512         (0)
#define PXA3XX_NDCR_PAGE_SZ_2048        BIT(24)
#define PXA3XX_NDCR_NCSX                BIT(23)
#define PXA3XX_NDCR_ND_STOP             BIT(22)
#define PXA3XX_NDCR_CLR_PG_CNT          BIT(20)
#define PXA3XX_NDCR_CLR_ECC             BIT(19)
#define PXA3XX_NDCR_RD_ID_CNT_MASK      BITS(18, 16)
#define PXA3XX_NDCR_RD_ID_CNT(c)        (((c) & 7) << 16)
#define PXA3XX_NDCR_RA_START            BIT(15)
#define PXA3XX_NDCR_PG_PER_BLK          BIT(14)
#define PXA3XX_NDCR_PG_PER_BLK_32       (0)
#define PXA3XX_NDCR_PG_PER_BLK_64       PXA3XX_NDCR_PG_PER_BLK
#define PXA3XX_NDCR_ND_ARB_EN           BIT(12)
#define PXA3XX_NDCR_RDYM                BIT(11)
#define PXA3XX_NDCR_CS0_PAGEDM          BIT(10)
#define PXA3XX_NDCR_CS1_PAGEDM          BIT(9)
#define PXA3XX_NDCR_CS0_CMDDM           BIT(8)
#define PXA3XX_NDCR_CS1_CMDDM           BIT(7)
#define PXA3XX_NDCR_CS0_BBDM            BIT(6)
#define PXA3XX_NDCR_CS1_BBDM            BIT(5)
#define PXA3XX_NDCR_DBERRM              BIT(4)
#define PXA3XX_NDCR_SBERRM              BIT(3)
#define PXA3XX_NDCR_WRDREQM             BIT(2)
#define PXA3XX_NDCR_RDDREQM             BIT(1)
#define PXA3XX_NDCR_WRCMDREQM           BIT(0)
#define PXA3XX_NDCR_INTERRUPT_MASK_ALL  BITS(11, 0)


/*---------------------------------------------------------
    NDTR0CS0 -- NAND Controller Timing Register 0

    Maximum permitted value of all times is 76 nsec.

    Note that despite the implications of the name of this
    register, these timings apply to both CS0 and CS1.

    Note that there's an extended bit for tRP that is not
    implemented here because it's 1) inconvenient; and 2)
    not presently needed.

    Note that the actual macro for configuring this 
    register is located in hwpxa310.h or hwpxa320.h.
---------------------------------------------------------*/
#define PXA3XX_NDTR0CS0(base)           PXA3XX_DFCREG(base, 0x04)


/*---------------------------------------------------------
    NDTR1CS0 -- NAND Controller Timing Register 1

    Note that despite the implications of the name of this
    register, these timings apply to both CS0 and CS1.

    Maximum permitted value of tR is 630163 nsec.  Maximum
    permitted value of tWHR and tAR is 153 nsec.  Minimum
    permitted value of all fields is 10 nsec.

    Note that the actual macro for configuring this 
    register is located in hwpxa310.h or hwpxa320.h.
---------------------------------------------------------*/
#define PXA3XX_NDTR1CS0(base)           PXA3XX_DFCREG(base, 0x0C)


/*---------------------------------------------------------
    NDSR -- NAND Controller Status Register
---------------------------------------------------------*/
#define PXA3XX_NDSR(base)               PXA3XX_DFCREG(base, 0x14)
#define PXA3XX_NDSR_RDY                 BIT(11)
#define PXA3XX_NDSR_CS0_PAGED           BIT(10)
#define PXA3XX_NDSR_CS1_PAGED           BIT(9)
#define PXA3XX_NDSR_CS0_CMDD            BIT(8)
#define PXA3XX_NDSR_CS1_CMDD            BIT(7)
#define PXA3XX_NDSR_CS0_BBD             BIT(6)
#define PXA3XX_NDSR_CS1_BBD             BIT(5)
#define PXA3XX_NDSR_DBERR               BIT(4)
#define PXA3XX_NDSR_SBERR               BIT(3)
#define PXA3XX_NDSR_WRDREQ              BIT(2)
#define PXA3XX_NDSR_RDDREQ              BIT(1)
#define PXA3XX_NDSR_WRCMDREQ            BIT(0)


/*---------------------------------------------------------
    NDPCR -- NAND Controller Page Count Register
---------------------------------------------------------*/
#define PXA3XX_NDPCR(base)              PXA3XX_DFCREG(base, 0x18)


/*---------------------------------------------------------
    NDBBR0/1 -- NAND Controller Bad Block Registers
---------------------------------------------------------*/
#define PXA3XX_NDBBR0(base)             PXA3XX_DFCREG(base, 0x1C)
#define PXA3XX_NDBBR1(base)             PXA3XX_DFCREG(base, 0x20)


/*---------------------------------------------------------
    NDDB -- NAND Controller Data Buffer Register
---------------------------------------------------------*/
#define PXA3XX_NDDB(base)               PXA3XX_DFCREG(base, 0x40)


/*---------------------------------------------------------
    NDCB0/1/2 -- NAND Controller Command Buffers 0, 1, & 2
---------------------------------------------------------*/
#define PXA3XX_NDCB0(base)              PXA3XX_DFCREG(base, 0x48)
#define PXA3XX_NDCB1(base)              PXA3XX_DFCREG(base, 0x4C)
#define PXA3XX_NDCB2(base)              PXA3XX_DFCREG(base, 0x50)
#define PXA3XX_NDCB_AUTO_RS             BIT(25)
#define PXA3XX_NDCB_CSEL                BIT(24)
#define PXA3XX_NDCB_CMD_TYPE_MASK       BITS(23, 21)
#define PXA3XX_NDCB_CMD_TYPE_READ       (0U)
#define PXA3XX_NDCB_CMD_TYPE_PROGRAM    (1 << 21)
#define PXA3XX_NDCB_CMD_TYPE_ERASE      (2 << 21)
#define PXA3XX_NDCB_CMD_TYPE_READ_ID    (3 << 21)
#define PXA3XX_NDCB_CMD_TYPE_STATUS_READ (4 << 21)
#define PXA3XX_NDCB_CMD_TYPE_RESET      (5 << 21)
#define PXA3XX_NDCB_NC                  BIT(20)
#define PXA3XX_NDCB_DBC                 BIT(19)
#define PXA3XX_NDCB_ADDR_CYC_MASK       BITS(18, 16)
#define PXA3XX_NDCB_ADDR_CYC(n)         ((n) << 16)
#define PXA3XX_NDCB_CMD2_MASK           BITS(15, 8)
#define PXA3XX_NDCB_CMD2(cmd)           (((cmd) << 8) & PXA3XX_NDCB_CMD2_MASK)
#define PXA3XX_NDCB_CMD1_MASK           BITS(7, 0)
#define PXA3XX_NDCB_CMD1(cmd)           ((cmd) & PXA3XX_NDCB_CMD1_MASK)




#endif  /* HWPXA3XX_H_INCLUDED */

