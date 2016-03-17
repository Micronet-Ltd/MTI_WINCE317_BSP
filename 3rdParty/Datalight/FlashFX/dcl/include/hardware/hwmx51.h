/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2012 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation currently located
  at:
        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is a trade secret and the confidential property of Datalight, 
  Inc., protected under the copyright laws of the United States and other
  jurisdictions.  Patents may be pending.

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

    This module contains types and macros for interfacing with the FreeScale
    MX51 platform.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: hwmx51.h $
    Revision 1.3  2012/02/09 21:53:11Z  Glenns
    Fix compiler warnings caused by shifting a bit in a literal constant
    (which are type "int" by default) into the sign bit.
    Revision 1.2  2011/12/05 17:25:22Z  glenns
    Remove static structures that don't belong in header files.
    Revision 1.1  2011/11/22 22:09:02Z  jimmb
    Initial revision
---------------------------------------------------------------------------*/
#ifndef HWMX51_H_INCLUDED
#define HWMX51_H_INCLUDED
/******************************************************************************
*******************************************************************************
    Physical NAND Operations functions.

    This implementation reflects the fact that the Freescale chips
    incorporating these related controllers all have exactly one controller.

    Some machine dependencies (like number of bits in an int) are
    allowed, as this is machine-dependent code.

*******************************************************************************
******************************************************************************/

/*
   NAND CHIP Definations Configurable  
*/
#define STATUSBUSYBIT                             6 /*In Samsung NAND flash, bit 6 of the status register is the Ready/Busy indication.*/
#define STATUSERRORBIT                            0 /*In Samsung NAND flash, bit 0 of the status register is the Pass/Fail indication.*/
#define CHIP_ADDR_CYCLES_NUM                      5
#define NUM_OF_BLOCK_CYCLES                       3
#define CMDREADSTATUS                             0x70

/* Bitfield macros that use rely on bitfield width/shift information*/

#define DLBITFMASK(bit) (((1U << (bit ## _WID)) - 1) << (bit ## _LSH))
#define DLBITFVAL(bit, val) ((val) << (bit ## _LSH))


/*  Eight sections.
    The number of bits that were detected/corrected for every
    page section of 528/538 bytes in the main and spare area as a result
    of BCH ECC check.
*/
#define NANDFC_ECC_STATUS_RESULT_NOSER1_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER2_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER3_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER4_4SB_ERR         0x4000
#define NANDFC_ECC_STATUS_RESULT_NOSER5_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER6_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER7_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER8_4SB_ERR         0x4000

 
 
/*  For 512B 2 K 4K byte page the Legal values of RBA(Ram Buffer Address) are
    0 
*/ 
#define XFER_BUFFER_NUMBER      (0)

/*  The column address of 2K Byte page-area of 2KB page.
*/
#define MX51_PAGE_COLUMN        (0)

/*  ECC Result Error Codes
*/
#define ECC_STATUS_NO_ERROR     (0)

/*  Base Address Register Offset
*/
#define BA_REG_OFFSET           (0x1E00)
#define IP_REG_OFFSET           (0x3000)

/*  These are the column Index of each 16 byte portion of the 64 byte spare
    area for a 2KB page.
*/
#define MX51_SPARE_COLUMN0 (0*NAND512_SPARE)
#define MX51_SPARE_COLUMN1 (1*NAND512_SPARE)
#define MX51_SPARE_COLUMN2 (2*NAND512_SPARE)
#define MX51_SPARE_COLUMN3 (3*NAND512_SPARE)

/*  This is the column Index of the tag for a 2KB page.
*/
#define TAG_SPARE_COLUMN      MX51_SPARE_COLUMN3




/*  Controller registers and bit fields
*/

#define NFCREG(offset)   ((D_UINT32 *) ((char *) NFC.pBase + BA_REG_OFFSET + offset))
#define IPREG(offset)    ((D_UINT32 *) ((char *) NFC_IP.pIPBase + IP_REG_OFFSET + offset))

/*  Definition for AXI registers 
*/

#define NFC_FLASH_CMD                   NFCREG(0x00)
#define NAND_ADD_LOW                    NFCREG(0x04)
#define NAND_ADD_HIGH                   NFCREG(0x24)
#define NFC_CONFIGURATION1              NFCREG(0x34)
#define ECC_STATUS_RESULT               NFCREG(0x38)
#define ECC_STATUS_SUM                  NFCREG(0x3C)
#define LAUNCH_NFC                      NFCREG(0x40)

/*  Definition for IP registers 
*/

#define NF_WR_PROT                      IPREG(0x00)
#define UNLOCK_BLK_ADD(i)               IPREG(0x04 +(i*4))
#define UNLOCK_BLK_ADD0                 IPREG(0x04)
#define UNLOCK_BLK_ADD1                 IPREG(0x08)
#define NFC_CONFIGURATION2              IPREG(0x24)
#define NFC_CONFIGURATION3              IPREG(0x28)
#define NFC_IPC                         IPREG(0x2C)
#define AXI_ERR_ADD                     IPREG(0x30)
#define NFC_DELAY_LINE                  IPREG(0x34)
/*
  Register Bit Filed Positions (Left Shift)

*/
/* NFC_CONFIGURATION2 */
#define NFC_NFC_CONFIGURATION2_PS_LSH                   0
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES0_LSH      5
#define NFC_NFC_CONFIGURATION2_PPB_LSH                  7
#define NFC_NFC_CONFIGURATION2_NUM_ADR_PHASES1_LSH      12
#define NFC_NFC_CONFIGURATION2_SPAS_LSH                 16
#define NFC_NFC_CONFIGURATION2_ST_CMD_LSH               24

/*NFC_CONFIGURATION3 */
#define NFC_NFC_CONFIGURATION3_SB2R_LSH                 4
#define NFC_NFC_CONFIGURATION3_SBB_LSH                  8
#define NFC_NFC_CONFIGURATION3_NUM_OF_DEVICES_LSH       12

/* NFC_CONFIGURATION1 register */
#define NFC_NFC_CONFIGURATION1_CS_LSH                   12
#define NFC_NFC_CONFIGURATION1_CS_ACTIVE_CS0            0
#define NFC_NFC_CONFIGURATION1_CS_WID                   3
/* UNLOCK_BLK_ADD register*/

#define NFC_UNLOCK_BLK_ADD_USBA_LSH                     0
#define NFC_UNLOCK_BLK_ADD_UEBA_LSH                     16
/* UNLOCK_BLK_ADD0 register */
#define NFC_UNLOCK_BLK_ADD_USBA_WID                     16
#define NFC_UNLOCK_BLK_ADD_UEBA_WID                     16
/* NF_WR_PROT register */

#define NFC_NF_WR_PROT_WPC_LSH                       0
#define NFC_NF_WR_PROT_CS2L_LSH                      3
#define NFC_NF_WR_PROT_BLS_LSH                       6
#define NFC_NF_WR_PROT_LTS0_LSH                      8
#define NFC_NF_WR_PROT_LS0_LSH                       9
#define NFC_NF_WR_PROT_US0_LSH                       10
#define NFC_NF_WR_PROT_LTS1_LSH                      11
#define NFC_NF_WR_PROT_LS1_LSH                       12
#define NFC_NF_WR_PROT_US1_LSH                       13
#define NFC_NF_WR_PROT_LTS2_LSH                      14
#define NFC_NF_WR_PROT_LS2_LSH                       15
#define NFC_NF_WR_PROT_US2_LSH                       16
#define NFC_NF_WR_PROT_LTS3_LSH                      17
#define NFC_NF_WR_PROT_LS3_LSH                       18
#define NFC_NF_WR_PROT_US3_LSH                       19
#define NFC_NF_WR_PROT_LTS4_LSH                      20
#define NFC_NF_WR_PROT_LS4_LSH                       21
#define NFC_NF_WR_PROT_US4_LSH                       22
#define NFC_NF_WR_PROT_LTS5_LSH                      23
#define NFC_NF_WR_PROT_LS5_LSH                       24
#define NFC_NF_WR_PROT_US5_LSH                       25
#define NFC_NF_WR_PROT_LTS6_LSH                      26
#define NFC_NF_WR_PROT_LS6_LSH                       27
#define NFC_NF_WR_PROT_US6_LSH                       28
#define NFC_NF_WR_PROT_LTS7_LSH                      29
#define NFC_NF_WR_PROT_LS7_LSH                       30
#define NFC_NF_WR_PROT_US7_LSH                       31

/*  Bit Definitions for NFC_CONFIGURATION 2 
*/

#define NFC_CONFIGURATION2_PS_2K        (1 << 0)
#define NFC_CONFIGURATION2_SYM          (1 << 2)
#define NFC_CONFIGURATION2_ECC_EN       (1 << 3)
#define NFC_CONFIGURATION2_ECC_BY       (0 << 3)
#define NFC_CONFIG2_NUM_CMD_PHASES      (1 << 4)
#define NFC_CONFIG2_NUM_ADR_PHASES0     (1 << 5)
#define NFC_CONFIG2_ECC_4_BIT_MODE      (0 << 6)
#define NFC_CONFIG2_ECC_8_BIT_MODE      (1 << 6)
#define NFC_CONFIG2_64_PPB              (1 << 7)
#define NFC_CONFIGURATION2_EDC          (7 << 9)
#define NFC_CONFIG2_NUM_ADR_PHASES1     (2 << 12)
#define NFC_CONFIG2_ATO_PRG_DNE_MSK     (0 << 14)
#define NFC_CONFIGURATION2_INT_MSK      (0 << 15)
#define NFC_CONFIGURATION2_SPAS         (0x20 << 16)
#define NFC_CONFIGURATION2_ST_CMD       (0x70 << 24)

/*  Bit Definitions of NFC_CONFIGURATION1 register 
*/

#define NFC_CONFIG1_SP_EN_MAIN_SPARE    (0 << 0)
#define NFC_CONFIGURATION1_NF_CE        (0 << 1)
#define NFC_CONFIGURATION1_NFC_RST      (1 << 2)
#define NFC_CONFIGURATION1_RBA_FIRST    (0 << 4) 
#define NFC_CONFIGURATION1_NUM_OF_ITE   (0 << 8)
#define NFC_CONFIGURATION1_CS_0         (0 << 12)

/*  Bit Definitions for UNLOCK_BLK_ADD Register 
*/

#define NFC_UNLOCK_BLK_ADD_USBA_END     (0 << 0)
#define NFC_UNLOCK_BLK_ADD_USBA_START   (0xFFFF << 16)

/*  Bit Definitions for NFC_LAUNCH 
*/

#define FCMD                            (1U <<  0)
#define FADD                            (1U <<  1)
#define FDI                             (1U <<  2)
#define FDO_PAGE                        (1U <<  3)
#define FDO_ID                          (2U <<  3)
#define FDO_STATUS                      (4U <<  3)
#define AUTO_PROG                       (1U <<  6)
#define AUTO_READ                       (1U <<  7) 
#define AUTO_ERASE                      (1U <<  9)
#define AUTO_STATUS                     (1U <<  12)  

/*  Bit Definitions for NFC_IPC
*/

#define NFC_OPS_STAT_INT                (1UL << 31)
#define NFC_OPS_STAT_INT_CLEAR          (0 << 31)
#define NFC_OP_DONE                     (1 << 30)
#define NFC_IPC_RB_B                    (1 << 28)
#define NFC_PS_WIDTH                    (2)
#define NFC_PS_SHIFT                    (0)
#define NFC_PS_512                      (0)
#define NFC_PS_2K                       (1)
#define NFC_PS_4K                       (2)
#define NFC_NFC_IPC_CACK                (1 << 1)
#define NFC_IPC_CREQ                    (1 << 0)
#define NFC_IPC_NO_CREQ                 (0 << 0)

/*  Bit Definitions for NFC_CONFIGURATION3
*/

#define NFC_CONFIGURATION3_ADD_OP       (0 << 0)
#define NFC_CONFIGURATION3_TOO          (0 << 2)
#define NFC_CONFIGURATION3_FW_8BIT      (1 << 3) 
#define NFC_CONFIGURATION3_FW_16BIT     (0 << 3) 
#define NFC_CONFIGURATION3_SB2R         (0 << 4) /*In Samsung NAND flash, bit 0 of the status register is the Pass/Fail indication.*/
#define NFC_CONFIGURATION3_NF_BIG       (0 << 7)
#define NFC_CONFIGURATION3_SBB          (6 << 8) /*In Samsung NAND flash, bit 6 of the status register is the Ready/Busy indication. */ 
#define NFC_CONFIGURATION3_DMA_MODE     (0 << 11)
#define NFC_CONFIG3_NUM_OF_DEVICES      (0 << 12)
#define NFC_CONFIGURATION3_RBB_MODE     (1 << 15) 
#define NFC_CONFIGURATION3_FMP          (0 << 16)
#define NFC_CONFIGURATION3_FMP_1        (0 << 17)
#define NFC_CONFIGURATION3_NO_SDMA      (1 << 20)

/*  NF_WR_PROT register 
*/

#define NFC_NF_WR_PROT_WPC_UNLOCK       (1 << 2)
#define NFC_NF_WR_PROT_WPC_LOCK_ALL     (1 << 1)
#define NFC_NF_WR_PROT_WPC_LOCK_TIGHT   (1 << 0)

#define NFC_NF_WR_PROT_BLS_LOCKED       (1)
#define NFC_NF_WR_PROT_BLS_UNLOCKED     (1 << 7)

#define NFC_NF_WR_PROT_CS2L_CS0LOCK_STATUS   (0)
#define NFC_WPC_RESET                       ~(7)
#define NFC_WPC_UNLOCK                      (1 << 2)

#define NFC_NF_WR_PROT_CS2L_CS0            (0 << 3)
#define NFC_NF_WR_PROT_CS2L_CS1            (1 << 3)
#define NFC_NF_WR_PROT_CS2L_CS2            (2 << 3)
#define NFC_NF_WR_PROT_CS2L_CS3            (3 << 3)
#define NFC_NF_WR_PROT_CS2L_CS(i)          (i << 3)
#define NAND_STATUS_ERROR_BIT               (0)         /* Status Bit0 indicates error*/
#define NAND_STATUS_BUSY_BIT                (6)         /* Status Bit6 indicates busy */
#define NFC_SET_WPC(val) ((DCLMEMGET32(NF_WR_PROT) & NFC_WPC_RESET) | val )

#define NFC_UNLOCK_END_ADDR_SHIFT           (16)

#define UNLOCK_ADDR(start_addr,end_addr)     \
        DCLMEMPUT32(UNLOCK_BLK_ADD0 ,start_addr | \
        (end_addr << NFC_UNLOCK_END_ADDR_SHIFT));

#define UNLOCK_ADDR1(start_addr,end_addr)        \
        DCLMEMPUT32(UNLOCK_BLK_ADD1 ,start_addr | \
        (end_addr << NFC_UNLOCK_END_ADDR_SHIFT));

#define NFC_NFC_IPC_INT_LSH                         31
#define NFC_NFC_IPC_INT_WID                         1
#define NFC_NFC_IPC_CREQ_LSH                        0
#define NFC_NFC_IPC_CACK_LSH                        1

#define NFC_NFC_IPC_CREQ_WID                        1
#define NFC_NFC_IPC_CACK_WID                        1

#define NFC_NFC_IPC_CREQ_NO_REQUEST                     0
#define NFC_NFC_IPC_CREQ_REQUEST                        1


#define CLRREG32(x, y)      DCLMEMPUT32(x, DCLMEMGET32(x)&~(y))
#define DCLREG32(addr, mask, val) DCLMEMPUT32(addr, ((DCLMEMGET32(addr)&(~(mask))) | val))
#define DCLREG32BF(addr, bit, val) (DCLREG32(addr, DLBITFMASK(bit), DLBITFVAL(bit, val)))

#define NFC_ECC_SECTION_BITS                        4
#define NFC_ECC_BIT_UNCORRECTABLE_ERROR_8BIT        0xF
#define NFC_ECC_BIT_UNCORRECTABLE_ERROR_4BIT        7
#define NFC_ECC_BIT_UNCORRECTABLE_ERROR_SPARE       0xF0000000
#define NFC_ECC_BIT_UNCORRECTABLE_ERROR             0xF
/* ECC_STATUS_RESULT register */
#define NFC_ECC_STATUS_RESULT_NOSER_NO_ERR             0
#define NFC_ECC_STATUS_RESULT_NOSER_1SB_ERR            1
#define NFC_ECC_STATUS_RESULT_NOSER_2SB_ERR            2
#define NFC_ECC_STATUS_RESULT_NOSER_3SB_ERR            3
#define NFC_ECC_STATUS_RESULT_NOSER_4SB_ERR            4
#define NFC_ECC_STATUS_RESULT_NOSER_5SB_ERR            5
#define NFC_ECC_STATUS_RESULT_NOSER_6SB_ERR            6
#define NFC_ECC_STATUS_RESULT_NOSER_7SB_ERR            7
#define NFC_ECC_STATUS_RESULT_NOSER_8SB_ERR            8

#endif /* HWMX51_H_INCLUDED */
