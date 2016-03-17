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

    Register settings for OMAP35x.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: hwomap35x.h $
    Revision 1.6  2010/08/04 00:40:54Z  garyp
    Added the OMAP base and GPMC register offsets.  Updated the way
    timer1 is handled.
    Revision 1.5  2010/01/23 15:16:47Z  garyp
    Corrected some CS1 and CS2 config register definitions.
    Revision 1.4  2009/12/14 04:23:47Z  garyp
    Added/clarified some registers.
    Revision 1.3  2009/11/26 02:22:56Z  glenns
    Add macros for manipulating OMAP35xx idle mode. Needed for operating 
    Prefetch and HW EDC simultaneously.
    Revision 1.2  2009/10/13 20:41:27Z  garyp
    Added IRQ status and enable register settings.
    Revision 1.1  2009/09/23 22:06:00Z  garyp
    Initial revision
---------------------------------------------------------------------------*/


#define OMAP35X_REG(base, offs)   (*(volatile D_UINT32*)((char*)(base) + (offs)))
#define OMAP35X_REG16(base, offs) (*(volatile D_UINT16*)((char*)(base) + (offs)))
#define OMAP35X_REG8(base, offs)  (*(volatile D_UINT8*) ((char*)(base) + (offs)))


/*---------------------------------------------------------
    OMAP35X Base and GPMC Registers, Offset and Size
---------------------------------------------------------*/
#define OMAP35X_PHYSICAL_OFFSET                 (0x48000000UL)
#define OMAP35X_PHYSICAL_SIZE                   (0x08000000UL)

#define OMAP35X_GPMC_PHYSICAL_OFFSET            (0x6E000000UL)
#define OMAP35X_GPMC_PHYSICAL_SIZE              (0x00100000UL)


/*---------------------------------------------------------
    OMAP35X Timer Registers
---------------------------------------------------------*/
#define OMAP35X_TIMER_GP1_TCRR(base)            OMAP35X_REG(base, 0x318028UL)

#define OMAP35X_TICK_DIVISOR                    (32768)


/*---------------------------------------------------------
    OMAP35X GPMC Registers
---------------------------------------------------------*/
#define OMAP35X_GPMC_REVISION(base)             OMAP35X_REG(base, 0x0000UL)
#define OMAP35X_GPMC_SYSCONFIG(base)            OMAP35X_REG(base, 0x0010UL)
#define OMAP35X_GPMC_SYSSTATUS(base)            OMAP35X_REG(base, 0x0014UL)
#define OMAP35X_GPMC_IRQSTATUS(base)            OMAP35X_REG(base, 0x0018UL)
#define OMAP35X_GPMC_IRQENABLE(base)            OMAP35X_REG(base, 0x001CUL)
#define OMAP35X_GPMC_TIMEOUT_CONTROL(base)      OMAP35X_REG(base, 0x0040UL)
#define OMAP35X_GPMC_ERR_ADDRESS(base)          OMAP35X_REG(base, 0x0044UL)
#define OMAP35X_GPMC_ERR_TYPE(base)             OMAP35X_REG(base, 0x0048UL)
#define OMAP35X_GPMC_CONFIG(base)               OMAP35X_REG(base, 0x0050UL)
#define OMAP35X_GPMC_STATUS(base)               OMAP35X_REG(base, 0x0054UL)

#define OMAP35X_GPMC_CS0_CONFIG1(base)          OMAP35X_REG(base, 0x0060UL)
#define OMAP35X_GPMC_CS0_CONFIG2(base)          OMAP35X_REG(base, 0x0064UL)
#define OMAP35X_GPMC_CS0_CONFIG3(base)          OMAP35X_REG(base, 0x0068UL)
#define OMAP35X_GPMC_CS0_CONFIG4(base)          OMAP35X_REG(base, 0x006CUL)
#define OMAP35X_GPMC_CS0_CONFIG5(base)          OMAP35X_REG(base, 0x0070UL)
#define OMAP35X_GPMC_CS0_CONFIG6(base)          OMAP35X_REG(base, 0x0074UL)
#define OMAP35X_GPMC_CS0_CONFIG7(base)          OMAP35X_REG(base, 0x0078UL)

#define OMAP35X_GPMC_CS1_CONFIG1(base)          OMAP35X_REG(base, 0x0090UL)
#define OMAP35X_GPMC_CS1_CONFIG2(base)          OMAP35X_REG(base, 0x0094UL)
#define OMAP35X_GPMC_CS1_CONFIG3(base)          OMAP35X_REG(base, 0x0098UL)
#define OMAP35X_GPMC_CS1_CONFIG4(base)          OMAP35X_REG(base, 0x009CUL)
#define OMAP35X_GPMC_CS1_CONFIG5(base)          OMAP35X_REG(base, 0x00A0UL)
#define OMAP35X_GPMC_CS1_CONFIG6(base)          OMAP35X_REG(base, 0x00A4UL)
#define OMAP35X_GPMC_CS1_CONFIG7(base)          OMAP35X_REG(base, 0x00A8UL)

#define OMAP35X_GPMC_CS2_CONFIG1(base)          OMAP35X_REG(base, 0x00C0UL)
#define OMAP35X_GPMC_CS2_CONFIG2(base)          OMAP35X_REG(base, 0x00C4UL)
#define OMAP35X_GPMC_CS2_CONFIG3(base)          OMAP35X_REG(base, 0x00C8UL)
#define OMAP35X_GPMC_CS2_CONFIG4(base)          OMAP35X_REG(base, 0x00CCUL)
#define OMAP35X_GPMC_CS2_CONFIG5(base)          OMAP35X_REG(base, 0x00D0UL)
#define OMAP35X_GPMC_CS2_CONFIG6(base)          OMAP35X_REG(base, 0x00D4UL)
#define OMAP35X_GPMC_CS2_CONFIG7(base)          OMAP35X_REG(base, 0x00D8UL)

#define OMAP35X_GPMC_NAND8_COMMAND_0(base)      OMAP35X_REG8(base, 0x007CUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_0(base)      OMAP35X_REG8(base, 0x0080UL)
#define OMAP35X_GPMC_NAND8_DATA_0(base)         OMAP35X_REG8(base, 0x0084UL)
#define OMAP35X_GPMC_NAND8_COMMAND_1(base)      OMAP35X_REG8(base, 0x00ACUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_1(base)      OMAP35X_REG8(base, 0x00B0UL)
#define OMAP35X_GPMC_NAND8_DATA_1(base)         OMAP35X_REG8(base, 0x00B4UL)
#define OMAP35X_GPMC_NAND8_COMMAND_2(base)      OMAP35X_REG8(base, 0x00DCUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_2(base)      OMAP35X_REG8(base, 0x00E0UL)
#define OMAP35X_GPMC_NAND8_DATA_2(base)         OMAP35X_REG8(base, 0x00E4UL)
#define OMAP35X_GPMC_NAND8_COMMAND_3(base)      OMAP35X_REG8(base, 0x010CUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_3(base)      OMAP35X_REG8(base, 0x0110UL)
#define OMAP35X_GPMC_NAND8_DATA_3(base)         OMAP35X_REG8(base, 0x0114UL)
#define OMAP35X_GPMC_NAND8_COMMAND_4(base)      OMAP35X_REG8(base, 0x013CUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_4(base)      OMAP35X_REG8(base, 0x0140UL)
#define OMAP35X_GPMC_NAND8_DATA_4(base)         OMAP35X_REG8(base, 0x0144UL)
#define OMAP35X_GPMC_NAND8_COMMAND_5(base)      OMAP35X_REG8(base, 0x016CUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_5(base)      OMAP35X_REG8(base, 0x0170UL)
#define OMAP35X_GPMC_NAND8_DATA_5(base)         OMAP35X_REG8(base, 0x0174UL)
#define OMAP35X_GPMC_NAND8_COMMAND_6(base)      OMAP35X_REG8(base, 0x019CUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_6(base)      OMAP35X_REG8(base, 0x01A0UL)
#define OMAP35X_GPMC_NAND8_DATA_6(base)         OMAP35X_REG8(base, 0x01A4UL)
#define OMAP35X_GPMC_NAND8_COMMAND_7(base)      OMAP35X_REG8(base, 0x01CCUL)
#define OMAP35X_GPMC_NAND8_ADDRESS_7(base)      OMAP35X_REG8(base, 0x01D0UL)
#define OMAP35X_GPMC_NAND8_DATA_7(base)         OMAP35X_REG8(base, 0x01D4UL)

#define OMAP35X_GPMC_NAND16_COMMAND_0(base)     OMAP35X_REG16(base, 0x007CUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_0(base)     OMAP35X_REG16(base, 0x0080UL)
#define OMAP35X_GPMC_NAND16_DATA_0(base)        OMAP35X_REG16(base, 0x0084UL)
#define OMAP35X_GPMC_NAND16_COMMAND_1(base)     OMAP35X_REG16(base, 0x00ACUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_1(base)     OMAP35X_REG16(base, 0x00B0UL)
#define OMAP35X_GPMC_NAND16_DATA_1(base)        OMAP35X_REG16(base, 0x00B4UL)
#define OMAP35X_GPMC_NAND16_COMMAND_2(base)     OMAP35X_REG16(base, 0x00DCUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_2(base)     OMAP35X_REG16(base, 0x00E0UL)
#define OMAP35X_GPMC_NAND16_DATA_2(base)        OMAP35X_REG16(base, 0x00E4UL)
#define OMAP35X_GPMC_NAND16_COMMAND_3(base)     OMAP35X_REG16(base, 0x010CUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_3(base)     OMAP35X_REG16(base, 0x0110UL)
#define OMAP35X_GPMC_NAND16_DATA_3(base)        OMAP35X_REG16(base, 0x0114UL)
#define OMAP35X_GPMC_NAND16_COMMAND_4(base)     OMAP35X_REG16(base, 0x013CUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_4(base)     OMAP35X_REG16(base, 0x0140UL)
#define OMAP35X_GPMC_NAND16_DATA_4(base)        OMAP35X_REG16(base, 0x0144UL)
#define OMAP35X_GPMC_NAND16_COMMAND_5(base)     OMAP35X_REG16(base, 0x016CUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_5(base)     OMAP35X_REG16(base, 0x0170UL)
#define OMAP35X_GPMC_NAND16_DATA_5(base)        OMAP35X_REG16(base, 0x0174UL)
#define OMAP35X_GPMC_NAND16_COMMAND_6(base)     OMAP35X_REG16(base, 0x019CUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_6(base)     OMAP35X_REG16(base, 0x01A0UL)
#define OMAP35X_GPMC_NAND16_DATA_6(base)        OMAP35X_REG16(base, 0x01A4UL)
#define OMAP35X_GPMC_NAND16_COMMAND_7(base)     OMAP35X_REG16(base, 0x01CCUL)
#define OMAP35X_GPMC_NAND16_ADDRESS_7(base)     OMAP35X_REG16(base, 0x01D0UL)
#define OMAP35X_GPMC_NAND16_DATA_7(base)        OMAP35X_REG16(base, 0x01D4UL)

#define OMAP35X_GPMC_PREFETCH_CONFIG1(base)     OMAP35X_REG(base, 0x01E0UL)
#define OMAP35X_GPMC_PREFETCH_CONFIG2(base)     OMAP35X_REG(base, 0x01E4UL)
#define OMAP35X_GPMC_PREFETCH_CONTROL(base)     OMAP35X_REG(base, 0x01ECUL)
#define OMAP35X_GPMC_PREFETCH_STATUS(base)      OMAP35X_REG(base, 0x01F0UL)

#define OMAP35X_GPMC_ECC_CONFIG(base)           OMAP35X_REG(base, 0x01F4UL)
#define OMAP35X_GPMC_ECC_CONTROL(base)          OMAP35X_REG(base, 0x01F8UL)
#define OMAP35X_GPMC_ECC_SIZE(base)             OMAP35X_REG(base, 0x01FCUL)

#define OMAP35X_GPMC_ECC1_RESULT(base)          OMAP35X_REG(base, 0x0200UL)
#define OMAP35X_GPMC_ECC2_RESULT(base)          OMAP35X_REG(base, 0x0204UL)
#define OMAP35X_GPMC_ECC3_RESULT(base)          OMAP35X_REG(base, 0x0208UL)
#define OMAP35X_GPMC_ECC4_RESULT(base)          OMAP35X_REG(base, 0x020CUL)
#define OMAP35X_GPMC_ECC5_RESULT(base)          OMAP35X_REG(base, 0x0210UL)
#define OMAP35X_GPMC_ECC6_RESULT(base)          OMAP35X_REG(base, 0x0214UL)
#define OMAP35X_GPMC_ECC7_RESULT(base)          OMAP35X_REG(base, 0x0218UL)
#define OMAP35X_GPMC_ECC8_RESULT(base)          OMAP35X_REG(base, 0x021CUL)
#define OMAP35X_GPMC_ECC9_RESULT(base)          OMAP35X_REG(base, 0x0220UL)


/*---------------------------------------------------------
    SysConfig Register Values
---------------------------------------------------------*/
#define OMAP35X_GPMC_SYSCONFIG_AUTOIDLE                     (1 << 0)
#define OMAP35X_GPMC_SYSCONFIG_SOFTRESET                    (1 << 1)
#define OMAP35X_GPMC_SYSCONFIG_NO_IDLE                      (0x8)
#define OMAP35X_GPMC_SYSCONFIG_SMART_IDLE                   (0x10)


/*
#define OMAP35X_GPMC_SYSCONFIG_IDLEMODE_SHIFT               (3)
#define OMAP35X_GPMC_SYSCONFIG_IDLEMODE_MASK                (0x03 << OMAP35X_GPMC_SYSCONFIG_IDLEMODE_SHIFT)
#define OMAP35X_GPMC_SYSCONFIG_IDLEMODE(n)                  ((n <<   OMAP35X_GPMC_SYSCONFIG_IDLEMODE_SHIFT) & \
                                                                     OMAP35X_GPMC_SYSCONFIG_IDLEMODE_MASK)
#define OMAP35X_GPMC_SYSCONFIG_IDLE_FORCE                   (0)
#define OMAP35X_GPMC_SYSCONFIG_IDLE_NONE                    (1)
#define OMAP35X_GPMC_SYSCONFIG_IDLE_SMART                   (2)
#define OMAP35X_GPMC_SYSCONFIG_IDLE_DISABLED                (3)
*/


/*---------------------------------------------------------
    General Status Register Values
---------------------------------------------------------*/
#define OMAP35X_GPMC_STATUS_WRITEBUFFEREMPTY                (1 <<  0)
#define OMAP35X_GPMC_STATUS_WAIT0_READY                     (1 <<  8)
#define OMAP35X_GPMC_STATUS_WAIT1_READY                     (1 <<  9)
#define OMAP35X_GPMC_STATUS_WAIT2_READY                     (1 << 10)
#define OMAP35X_GPMC_STATUS_WAIT3_READY                     (1 << 11)


/*---------------------------------------------------------
    IRQ Status and Enable
---------------------------------------------------------*/
#define OMAP35X_GPMC_IRQ_FIFOEVENT                          (1 <<   0)
#define OMAP35X_GPMC_IRQ_TERMINALCOUNT                      (1 <<   1)
#define OMAP35X_GPMC_IRQ_WAIT_0_EDGEDETECT                  (1 <<   8)
#define OMAP35X_GPMC_IRQ_WAIT_1_EDGEDETECT                  (1 <<   9)
#define OMAP35X_GPMC_IRQ_WAIT_2_EDGEDETECT                  (1 <<  10)
#define OMAP35X_GPMC_IRQ_WAIT_3_EDGEDETECT                  (1 <<  11)


/*---------------------------------------------------------
    Prefetch Configuration and Status
---------------------------------------------------------*/
/*  CONFIG1
*/
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ACCESSWRITEPOST       (1 << 0)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_DMAENABLED            (1 << 2)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_SYNCHROMODE           (1 << 3)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ENABLEENGINE          (1 << 7)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWENROUNDROBIN      (1 << 23)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ENABLEOPTIMIZEDACCESS (1 << 27)

#define OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR_SHIFT (4)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR_MASK  (0x03 << OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR_SHIFT)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR(n)    ((n <<   OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_CONFIG1_WAITPINSELECTOR_MASK)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD_SHIFT   (8)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD_MASK    (0x7F << OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD_SHIFT)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD(n)      ((n <<   OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_CONFIG1_FIFOTHRESHOLD_MASK)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO_SHIFT (16)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO_MASK (0x0F << OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO_SHIFT)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO(n)   ((n <<   OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_CONFIG1_PFPWWEIGHTEDPRIO_MASK)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT_SHIFT (24)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT_MASK (0x07 << OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT_SHIFT)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT(n)   ((n <<   OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_CONFIG1_ENGINECHIPSELECT_MASK)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE_SHIFT   (28)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE_MASK    (0x07 << OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE_SHIFT)
#define OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE(n)      ((n <<   OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_CONFIG1_CYCLEOPTIMIZE_MASK)
/*  CONTROL
*/
#define OMAP35X_GPMC_PREFETCH_CONTROL_STARTENGINE (1 << 0)

/*  STATUS
*/
#define OMAP35X_GPMC_PREFETCH_STATUS_FIFOTHRESHOLD          (1 << 16)

#define OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING_SHIFT   (0)
#define OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING_MASK    (0x3FFF << OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING_SHIFT)
#define OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING(n)      ((n <<     OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING_SHIFT) & \
                                                                       OMAP35X_GPMC_PREFETCH_STATUS_COUNTREMAINING_MASK)
#define OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT      (24)
#define OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK       (0x7F << OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT)
#define OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER(n)         ((n <<   OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_SHIFT) & \
                                                                     OMAP35X_GPMC_PREFETCH_STATUS_FIFOPOINTER_MASK)

#define OMAP35X_GPMC_PREFETCH_FIFO_BYTES                    (64)


/*---------------------------------------------------------
    ECC Configuration and Status
---------------------------------------------------------*/
/*  CONFIG
*/
#define OMAP35X_GPMC_ECC_CONFIG_ENABLE                      (1 << 0)
#define OMAP35X_GPMC_ECC_CONFIG_16BIT                       (1 << 7)
#define OMAP35X_GPMC_ECC_CONFIG_BCH8BIT                     (1 << 12)
#define OMAP35X_GPMC_ECC_CONFIG_BCHENABLE                   (1 << 16)

#define OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT_SHIFT            (1)
#define OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT_MASK             (0x7 << OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT_SHIFT)
#define OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT(n)               ((n <<  OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT_SHIFT) & \
                                                                    OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT_MASK)
#define OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS_SHIFT            (4)
#define OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS_MASK             (0x7 << OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS_SHIFT)
#define OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS(n)               ((n <<  OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS_SHIFT) & \
                                                                    OMAP35X_GPMC_ECC_CONFIG_BCHSECTORS_MASK)
#define OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE_SHIFT           (8)
#define OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE_MASK            (0xF << OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE_SHIFT)
#define OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE(n)              ((n <<  OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE_SHIFT) & \
                                                                    OMAP35X_GPMC_ECC_CONFIG_BCHWRAPMODE_MASK)
/*  CONTROL
*/
#define OMAP35X_GPMC_ECC_CONTROL_CLEAR                      (1 << 8)

#define OMAP35X_GPMC_ECC_CONTROL_POINTER_SHIFT              (0)
#define OMAP35X_GPMC_ECC_CONTROL_POINTER_MASK               (0xF << OMAP35X_GPMC_ECC_CONTROL_POINTER_SHIFT)
#define OMAP35X_GPMC_ECC_CONTROL_POINTER(n)                 ((n <<  OMAP35X_GPMC_ECC_CONTROL_POINTER_SHIFT) & \
                                                                    OMAP35X_GPMC_ECC_CONTROL_POINTER_MASK)
/*  SIZE
*/
#define OMAP35X_GPMC_ECC_SIZE_RESULT1_SIZE1                 (1 << 0)
#define OMAP35X_GPMC_ECC_SIZE_RESULT2_SIZE1                 (1 << 1)
#define OMAP35X_GPMC_ECC_SIZE_RESULT3_SIZE1                 (1 << 2)
#define OMAP35X_GPMC_ECC_SIZE_RESULT4_SIZE1                 (1 << 3)
#define OMAP35X_GPMC_ECC_SIZE_RESULT5_SIZE1                 (1 << 4)
#define OMAP35X_GPMC_ECC_SIZE_RESULT6_SIZE1                 (1 << 5)
#define OMAP35X_GPMC_ECC_SIZE_RESULT7_SIZE1                 (1 << 6)
#define OMAP35X_GPMC_ECC_SIZE_RESULT8_SIZE1                 (1 << 7)
#define OMAP35X_GPMC_ECC_SIZE_RESULT9_SIZE1                 (1 << 8)

#define OMAP35X_GPMC_ECC_SIZE_0_SHIFT                       (12)
#define OMAP35X_GPMC_ECC_SIZE_0_MASK                        (0xFF << OMAP35X_GPMC_ECC_SIZE_0_SHIFT)
#define OMAP35X_GPMC_ECC_SIZE_0(n)                          ((n <<   OMAP35X_GPMC_ECC_SIZE_0_SHIFT) & \
                                                                     OMAP35X_GPMC_ECC_SIZE_0_MASK)
#define OMAP35X_GPMC_ECC_SIZE_1_SHIFT                       (22)
#define OMAP35X_GPMC_ECC_SIZE_1_MASK                        (0xFF << OMAP35X_GPMC_ECC_SIZE_1_SHIFT)
#define OMAP35X_GPMC_ECC_SIZE_1(n)                          ((n <<   OMAP35X_GPMC_ECC_SIZE_1_SHIFT) & \
                                                                     OMAP35X_GPMC_ECC_SIZE_1_MASK)


