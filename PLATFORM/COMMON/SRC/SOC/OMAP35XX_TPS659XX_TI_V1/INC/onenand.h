/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File: onenand.h
//

//
//  This file contains information about OneNAND flash supported by FMD driver.
//
#ifndef __ONE_NAND_H
#define __ONE_NAND_H


//------------------------------------------------------------------------------
//  OneNAND Id and geometry
//
//  Following constats must be specified for each supported NAND chip:
//  1. manufacturer ID (UINT8), 2. device Id (UINT8), 
//  3. number of block (UINT32), 4. sectors per block (UINT32),
//  5. sector data size (UINT32), 6. extended address (BOOL),
//  7. 16-bit interface (BOOL)

//  Note that for some OneNAND parts, the driver treats a 2KB page as a sector for
//  better performance with the MSFLASH FAL

#define ONENAND_INFO_DATA                           \
    { 0xEC, 0x30, 1024, 256, 512, FALSE, TRUE  },   \
    { 0xEC, 0x48, 2048, 256, 512, TRUE, TRUE   },   \
    { 0xEC, 0x40, 2048, 64, 2048, FALSE, TRUE  },   \
    { 0xEC, 0x58, 4096, 64, 2048, TRUE, TRUE  },    \


//------------------------------------------------------------------------------
//  Boot Partition buffer
//
//  Buffer size is calculated as blockSize(0x20000) + sectorSize(0x800) + (sectors/block)(0x40) * 8
//  Value used must be sufficient for all supported NANDs.

#define ONENAND_BPART_BUFFER_SIZE      0x20A00



//------------------------------------------------------------------------------
// OneNAND Register Address Definitions                                     
//

#define ONENAND_REGS_OFFSET            0x0001E000

typedef volatile struct {
    UINT16 ONLD_REG_MANUF_ID;       // 0000
    UINT16 ONLD_REG_DEV_ID;         // 0002
    UINT16 ONLD_REG_VER_ID;         // 0004
    UINT16 ONLD_REG_DATABUF_SIZE;   // 0006
    UINT16 ONLD_REG_BOOTBUF_SIZE;   // 0008
    UINT16 ONLD_REG_BUF_AMOUNT;     // 000A
    UINT16 ONLD_REG_TECH;           // 000C
    UINT16 RESERVED_000E[249];      // 000E - 01FE
    UINT16 ONLD_REG_START_ADDR1;    // 0200
    UINT16 ONLD_REG_START_ADDR2;    // 0202
    UINT16 ONLD_REG_START_ADDR3;    // 0204
    UINT16 ONLD_REG_START_ADDR4;    // 0206
    UINT16 ONLD_REG_START_ADDR5;    // 0208
    UINT16 ONLD_REG_START_ADDR6;    // 020A
    UINT16 ONLD_REG_START_ADDR7;    // 020C
    UINT16 ONLD_REG_START_ADDR8;    // 020E
    UINT16 RESERVED_0210[248];      // 0210 - 03FE
    UINT16 ONLD_REG_START_BUF;      // 0400
    UINT16 RESERVED_0402[7];        // 0402 - 040E
    UINT16 RESERVED_0410[24];       // 0410 - 043E
    UINT16 ONLD_REG_CMD;            // 0440
    UINT16 ONLD_REG_SYS_CONF1;      // 0442
    UINT16 ONLD_REG_SYS_CONF2;      // 0444
    UINT16 RESERVED_0446[13];       // 0446 - 045E
    UINT16 RESERVED_0460[16];       // 0460 - 047E
    UINT16 ONLD_REG_CTRL_STAT;      // 0480
    UINT16 ONLD_REG_INT;            // 0482
    UINT16 RESERVED_0484[10];       // 0484 - 0496
    UINT16 ONLD_REG_WR_PROTECT_START;   // 0498
    UINT16 RESERVED_049A;           // 049A
    UINT16 ONLD_REG_WR_PROTECT_STAT;// 049C
    UINT16 RESERVED_049E[3249];     // 049E - 1DFE
    UINT16 ONLD_REG_ECC_STAT;       // 1E00
    UINT16 ONLD_REG_ECC_RSLT_MB0;   // 1E02
    UINT16 ONLD_REG_ECC_RSLT_SB0;   // 1E04
    UINT16 ONLD_REG_ECC_RSLT_MB1;   // 1E06
    UINT16 ONLD_REG_ECC_RSLT_SB1;   // 1E08
    UINT16 ONLD_REG_ECC_RSLT_MB2;   // 1E0A
    UINT16 ONLD_REG_ECC_RSLT_SB2;   // 1E0C
    UINT16 ONLD_REG_ECC_RSLT_MB3;   // 1E0E
    UINT16 ONLD_REG_ECC_RSLT_SB3;   // 1E10
} ONE_NAND_REGS;
//  Define:  OneNAND Main Buffer Address
#define ONLD_BT_MB0_ADDR            0x00000000
#define ONLD_BT_MB1_ADDR            0x00000200
#define ONLD_DT_MB00_ADDR           0x00000400
#define ONLD_DT_MB01_ADDR           0x00000600
#define ONLD_DT_MB02_ADDR           0x00000800
#define ONLD_DT_MB03_ADDR           0x00000A00
#define ONLD_DT_MB10_ADDR           0x00000C00
#define ONLD_DT_MB11_ADDR           0x00000E00
#define ONLD_DT_MB12_ADDR           0x00001000
#define ONLD_DT_MB13_ADDR           0x00001200

#define ONLD_BT_SB0_ADDR            0x00010000
#define ONLD_BT_SB1_ADDR            0x00010010
#define ONLD_DT_SB00_ADDR           0x00010020
#define ONLD_DT_SB01_ADDR           0x00010030
#define ONLD_DT_SB02_ADDR           0x00010040
#define ONLD_DT_SB03_ADDR           0x00010050
#define ONLD_DT_SB10_ADDR           0x00010060
#define ONLD_DT_SB11_ADDR           0x00010070
#define ONLD_DT_SB12_ADDR           0x00010080
#define ONLD_DT_SB13_ADDR           0x00010090

//  Define:  OneNAND Register Masking values                                          
#define MASK_DFS                0x8000
#define MASK_FBA                0x01FF
#define MASK_DBS                0x8000
#define MASK_FCBA               0x01FF
#define MASK_FCPA               0x00FC
#define MASK_FCSA               0x0003
#define MASK_FPA                0x00FC
#define MASK_FSA                0x0003
#define MASK_BSA                0x0F00
#define MASK_BSC                0x0003

//  Define:  OneNAND Start Buffer Register Setting values                             
#define BSA_BT_BUF0             0x0000
#define BSA_BT_BUF1             0x0100
#define BSA_DT_BUF00                0x0800
#define BSA_DT_BUF01                0x0900
#define BSA_DT_BUF02                0x0A00
#define BSA_DT_BUF03                0x0B00
#define BSA_DT_BUF10                0x0C00
#define BSA_DT_BUF11                0x0D00
#define BSA_DT_BUF12                0x0E00
#define BSA_DT_BUF13                0x0F00
                       
#define BSC_1_SECTOR                0x0001
#define BSC_2_SECTOR                0x0002
#define BSC_3_SECTOR                0x0003
#define BSC_4_SECTOR                0x0000
#define DBS_DDP_0                       0x0000
#define DBS_DDP_1                       0x8000
#define DFS_DDP_0                       0x0000
#define DFS_DDP_1                       0x8000

//  Define:  OneNAND Command Set                                                      
#define ONLD_CMD_READ_PAGE              0x0000  //  Load Single/Multiple sector data unit into buffer
#define ONLD_CMD_READ_SPARE             0x0013  //  Load Single/Multiple spare sector into buffer
#define ONLD_CMD_WRITE_PAGE             0x0080  //  Program Single/Multiple sector data unit from buffer
#define ONLD_CMD_WRITE_SPARE            0x001A  //  Program spare area
#define ONLD_CMD_CPBACK_PRGM            0x001B  //  Copy back operation
#define ONLD_CMD_ULOCK_NAND             0x0023  //  Unlock NAND array a block
#define ONLD_CMD_LOCK_NAND              0x002A  //  Lock NAND array a block
#define ONLD_CMD_LOCK_TIGHT             0x002C  //  Lock-tight NAND array a block
#define ONLD_CMD_ERASE_VERIFY           0x0071  //  Erase verify read
#define ONLD_CMD_ERASE_BLK              0x0094  //  Erase block
#define ONLD_CMD_ERASE_MULTI_BLOCK      0x0095  //  Erase multi block
#define ONLD_CMD_SUSPEND_ERASE          0x00B0  //  Suspend erase
#define ONLD_CMD_RESUME_ERASE           0x0030  //  Resume erase
#define ONLD_CMD_RST_NF_CORE            0x00F0  //  Reset NAND Flash Core
#define ONLD_CMD_RESET                  0x00F3  //  Reset OneNAND
#define ONLD_CMD_OPT_ACCESS             0x0065  //  OTP Access
#define ONLD_CMD_CACHE_READ             0x000E  //  Cache Read
#define ONLD_CMD_CACHE_READ_FINISH      0x000C  //  Finish Cache Read

//  Define:  OneNAND System Configureation1 Register Values                           
#define SYNC_READ_MODE              0x8000
#define ASYNC_READ_MODE             0x0000
                       
#define BST_RD_LATENCY_8            0x0000
#define BST_RD_LATENCY_9            0x1000
#define BST_RD_LATENCY_10           0x2000
#define BST_RD_LATENCY_3            0x3000  // min   
#define BST_RD_LATENCY_4            0x4000  // default 
#define BST_RD_LATENCY_5            0x5000      
#define BST_RD_LATENCY_6            0x6000
#define BST_RD_LATENCY_7            0x7000
                       
#define BST_LENGTH_CONT             0x0000  // default 
#define BST_LENGTH_4WD              0x0200
#define BST_LENGTH_8WD              0x0400
#define BST_LENGTH_16WD             0x0600
#define BST_LENGTH_32WD             0x0800  // N/A on spare 
#define CONF1_ECC_ON                0xFEFF
#define CONF1_ECC_OFF               0x0100  // (~CONF1_ECC_ON)   0x0100
                       
#define RDY_POLAR                   0x0080
#define INT_POLAR                   0x0040
#define IOBE_ENABLE                 0x0020

#define HF_ENABLE                   0x0004                       
#define SYNC_WRITE_MODE             0x0002
#define ASYNC_WRITE_MODE            0x0000
                    
                            
//  Define:  OneNAND Controller Status Register Values                                
#define CTRL_BUSY               0x8000
#define LOCK_CHK                0x4000
#define READ_BUSY               0x2000
#define WRITE_BUSY              0x1000
                       
#define ERASE_BUSY              0x0800
#define FAULT_CHK               0x0400
#define ERASE_SUSPEND               0x0200

#define RESET_BUSY              0x0080
#define OTP_LOCK                0x0040
#define TIME_OUT                0x0001
                       
#define CUR_PROGRAM_FAIL            0x0400  // 0x0080
#define TIME_OUT                0x0001
                            
//  Define:  OneNAND Interrupt Status Register Values                                 
#define INT_MASTER              0x8000
#define INT_CLEAR               0x0000
                       
#define INT_READ                0x0080
#define INT_WRITE               0x0040
#define INT_ERASE               0x0020
#define INT_RESET               0x0010
                       
#define PEND_INT                (INT_MASTER)
#define PEND_READ               (INT_MASTER + INT_READ)
#define PEND_WRITE              (INT_MASTER + INT_WRITE)
#define PEND_ERASE              (INT_MASTER + INT_ERASE)
#define PEND_RESET              (INT_MASTER + INT_RESET)

//  Define:  OneNAND Write Protection Status Register Values                          
#define UNLOCKED_STAT               0x0004
#define LOCKED_STAT             0x0002
#define LOCK_TIGHTEN_STAT           0x0001

//  Define:  OneNAND ECC Status Register Valuies                                      
#define ECC_SB0_NO_ERR              0x0000
#define ECC_SB0_1BIT_ERR            0x0001
#define ECC_SB0_2BIT_ERR            0x0002
#define ECC_MB0_NO_ERR              0x0000
#define ECC_MB0_1BIT_ERR            0x0004
#define ECC_MB0_2BIT_ERR            0x0008
                       
#define ECC_SB1_NO_ERR              0x0000
#define ECC_SB1_1BIT_ERR            0x0010
#define ECC_SB1_2BIT_ERR            0x0020
#define ECC_MB1_NO_ERR              0x0000
#define ECC_MB1_1BIT_ERR            0x0040
#define ECC_MB1_2BIT_ERR            0x0080
                       
#define ECC_SB2_NO_ERR              0x0000
#define ECC_SB2_1BIT_ERR            0x0100
#define ECC_SB2_2BIT_ERR            0x0200
#define ECC_MB2_NO_ERR              0x0000
#define ECC_MB2_1BIT_ERR            0x0400
#define ECC_MB2_2BIT_ERR            0x0800

#define ECC_SB3_NO_ERR              0x0000
#define ECC_SB3_1BIT_ERR            0x1000
#define ECC_SB3_2BIT_ERR            0x2000
#define ECC_MB3_NO_ERR              0x0000
#define ECC_MB3_1BIT_ERR            0x4000
#define ECC_MB3_2BIT_ERR            0x8000
                       
#define ECC_ANY_2BIT_ERR            0xAAAA
#define ECC_ANY_BIT_ERR             0xFFFF
#define ECC_MAIN_BIT_ERR            0xCCCC
#define ECC_SPARE_BIT_ERR           0x3333
#define ECC_REG_CLEAR               0x0000

//  Define:  OneNAND Misc Values                                                      
#define SECTOR0                 0x0000
#define SECTOR1                 0x0001
#define SECTOR2                 0x0002
#define SECTOR3                 0x0003
                       
#define DATA_BUF0               0x0000
#define DATA_BUF1               0x0001
                       
#define SECTOR0_OFFSET              0x0000
#define SECTOR1_OFFSET              0x0200
#define SECTOR2_OFFSET              0x0400
#define SECTOR3_OFFSET              0x0600


//  OneNAND's ECC spare area layout
#pragma pack( push, 1 )
typedef struct {
    BYTE  badBlock[2];          // Indicates if block is BAD
    BYTE  int_ecc_sector[3];        // ECC Logical Sector number (managed by internal ECC logic)
    BYTE  reserved1_2;          // dwReserved1 - used by FAL (bits 23-16)
    WORD  reserved1_1_0;        // dwReserved1 - used by FAL (bits 15-0)
    BYTE  int_ecc_main[3];      // ECC values for main area (managed by internal ECC logic) - R/O
    BYTE  int_ecc_spare[2];     // ECC values for spare area (managed by internal ECC logic) - R/O
    BYTE  oemReserved;          // For use by OEM
    BYTE  reserved1_3;          // dwReserved1 - used by FAL (bits 31-24)
    BYTE  reserved2;            // Reserved - used by FAL
} ONENAND_SPARE_AREA;
#pragma pack( pop )

//------------------------------------------------------------------------------
//  Geometry info structure

typedef struct {
    UINT8  manufacturerId;
    UINT8  deviceId;
    UINT32 blocks;
    UINT32 sectorsPerBlock;
    UINT32 sectorSize;
    BOOL   extendedAddress;
    BOOL   wordData;
} NAND_INFO;

//------------------------------------------------------------------------------

#endif // __ONE_NAND_H
