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

    This module contains types and macros for interfacing with the FreeScale
    MX35 platform.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: hwmx35.h $
    Revision 1.1  2011/11/22 22:09:00Z  jimmb
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


/*  State information about the NAND Flash Controller (NFC.  Since the i.MX35
     has exactly one of these, the single structure may be allocated statically
     and referenced directly (no need to pass a pointer around).
 */
static struct
{
    void *pBase;
    unsigned int uConfig1;
    D_UCHAR ucWidthBytes;
    NTMDATA *pNTM;
    D_BOOL fEccEnabled;
    unsigned uAddrsBytes;   /* page # and up (not offset in page) */
    unsigned fResetBeforeProgram : 1;

} NFC;

/* Eight sections 
*/
#define NANDFC_ECC_STATUS_RESULT_NOSER1_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER2_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER3_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER4_4SB_ERR         0x4000
#define NANDFC_ECC_STATUS_RESULT_NOSER5_4SB_ERR         0x0004
#define NANDFC_ECC_STATUS_RESULT_NOSER6_4SB_ERR         0x0040
#define NANDFC_ECC_STATUS_RESULT_NOSER7_4SB_ERR         0x0400
#define NANDFC_ECC_STATUS_RESULT_NOSER8_4SB_ERR         0x4000


 
/*  For 2 K-byte page the Legal values of RBA(Ram Buffer Address) are
    0 or 4
*/ 
#define XFER_BUFFER_NUMBER      (0)

/*  The column address of 2K Byte page-area of 2KB page.
*/
#define MX35_PAGE_COLUMN        (0)

/*  ECC Result Error Codes
*/
#define ECC_STATUS_NO_ERROR     (0)

/*  Base Address Register Offset
*/
#define BA_REG_OFFSET           (0x1E00)

/*  These are the column Index of each 16 byte portion of the 64 byte spare
    area for a 2KB page.
*/
#define MX35_SPARE_COLUMN0 (0*NAND512_SPARE)
#define MX35_SPARE_COLUMN1 (1*NAND512_SPARE)
#define MX35_SPARE_COLUMN2 (2*NAND512_SPARE)
#define MX35_SPARE_COLUMN3 (3*NAND512_SPARE)

/*  This is the column Index of the tag for a 2KB page.
*/
#define TAG_SPARE_COLUMN      MX35_SPARE_COLUMN3




/*  Controller registers and bit fields
*/
#define NFCREG(offset)   ((D_UINT16 *) ((char *) NFC.pBase + BA_REG_OFFSET + offset))
 
#define RAM_BUFFER_ADDRESS     NFCREG(0x04)
#define NAND_FLASH_ADD         NFCREG(0x06)
#define NAND_FLASH_CMD         NFCREG(0x08)
#define NFC_CONFIGURATION      NFCREG(0x0A)
#define BLS                    (0U << 0)
#define BLS_1                  (1U << 1)

#define ECC_STATUS_RESULT_1    NFCREG(0x0C)
#define ECC_STATUS_RESULT_2    NFCREG(0x0E)

#define SPAS                   NFCREG(0x10)
#define SPAS_MASK              (0xFF00)
#define SPAS_SHIFT             (0)
#define NFC_SPAS_64            (32)

#define NF_WR_PROT             NFCREG(0x12)
#define WRPROT_UNLOCK          (4)
#define WRPROT_LOCK            (2)
#define WRPROT_LOCKTIGHT       (1)

#define NAND_FLASH_WR_PR_ST    NFCREG(0x18)

#define NAND_FLASH_CONFIG1     NFCREG(0x1A)
#define FP_INT                 (1U << 11)
#define PPB                    (0U << 10)
#define PPB_1                  (1U << 9)
#define PPB_32                 ((0U << 9) | (0U << 10))
#define PPB_64                 ((1U << 9) | (0U << 10))
#define PPB_128                ((0U << 9) | (1U << 10))
#define PPB_256                ((1U << 9) | (1U << 10))

#define SYM                    (1U << 8)
#define NF_CE                  (0U << 7)
#define NFC_RST                (1U << 6)
#define NF_BIG                 (1U << 5)
#define INT_MASK               (1U << 4)
#define ECC_EN                 (1U << 3)
#define ECC_EN_BYP             (0U << 3)
#define NFC_SP_EN              (1U << 2)
#define DMA_MODE               (1U << 1)
#define NFC_ECC_MODE_4BIT      (1U << 0)
#define NFC_ECC_MODE_8BIT      (0U << 0)
#define NF_LITTLE              (0U << 5)
#define FP_INT_512             (0U << 11)


#define NAND_FLASH_CONFIG2     NFCREG(0x1C)
#define INT                    (1U << 15)
#define FDO_PAGE               (1U <<  3)
#define FDO_ID                 (2U <<  3)
#define FDO_STATUS             (4U <<  3)
#define FDI                    (1U <<  2)
#define FADD                   (1U <<  1)
#define FCMD                   (1U <<  0)

#define UNLOCK_START_BLK_ADD   NFCREG(0x20)
#define UNLOCK_END_BLK_ADD     NFCREG(0x22)

#define NFC_ECC_BIT_UNCORRECTABLE_ERROR_8BIT        0xF
#define NFC_ECC_BIT_UNCORRECTABLE_ERROR_4BIT        7
#define NFC_ECC_SECTION_BITS                        4
#endif /* HWMX51_H_INCLUDED */
