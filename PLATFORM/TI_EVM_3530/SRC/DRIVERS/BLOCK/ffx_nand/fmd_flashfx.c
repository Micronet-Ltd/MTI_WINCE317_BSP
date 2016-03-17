/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================

 Copyright (c) 2009, Micronet Corporation. All rights reserved.

 2010 - 2011
 Vladimir Zatulovsky	Multiple flash devices support
 2012
 Vladimir Zatulovsky	4-bit ECC for CPU revisions above 1.2
 Vladimir Zatulovsky	Lock/unlock by #WP fix
 Vladimir Zatulovsky	on-die 4-bit ECC support starting, not use the spare
 						area layout should be redesigned 
Vladimir Zatulovsky		4-bit ecc for datalight support
Vladimir Zatulovsky		on-die ecc for datalight support
================================================================================
*/
//
//  File: fmd.c
//
//  This file implements NAND flash media PDD.
//
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <omap35xx.h>
#include <fmd.h>
#include <nand.h>
#include "gpmc_ecc.h"
#include <bsp_base_regs.h>
#include <omap_cpuver.h>
#include <oalex.h>
//#include "..\\nand\\nand_MT29F2G16.h"
#include "..\\nand\\bsp_fmd.h"

#define NAND_FEATURE_ENABLE_ECC 8

#define WORD_ACCESS_SIZE			(2)     // Accesses 2 bytes of data
#define SECTOR_DATA_SIZE			(2048)  // 2048 bytes per sector/page
#define SECTORS_PER_BLOCK			(64)    // 64 sectors/page per block
#define NUMBER_OF_BLOCKS_MT29F2G16			(2048)  // 2048 blocks
#define NUMBER_OF_BLOCKS_MT29C4G48			(4096)  // 4096 blocks
#define NUMBER_OF_BLOCKS_MT29F8G16			(8192)  // 8192 blocks
#define NUMBER_OF_BLOCKS_02GR3B2D			(4096)  // 4096 blocks
#define NUMBER_OF_BLOCKS_H8KDS0UN0MER_46M	(4096)  // 4096 blocks

//-----------------------------------------------------------------------------
// nand id
#define MANUFACTURER_ID_MT29F2G16			(0x2C)
#define MANUFACTURER_ID_MT29F8G16			(0x2C)
#define MANUFACTURER_ID_MT29C4G48			(0x2C)
#define MANUFACTURER_ID_02GR3B2D			(0x20)
#define MANUFACTURER_ID_H8KDS0UN0MER_46M	(0xAD)
#define NAND_DEVICE_ID_MT29F2G16			(0xBA)
#define NAND_DEVICE_ID_MT29F8G16			(0xB3)
#define NAND_DEVICE_ID_MT29C4G48			(0xBC)
#define NAND_DEVICE_ID_02GR3B2D				(0xBC)
#define NAND_DEVICE_ID_H8KDS0UN0MER_46M		(0xBC)

//-----------------------------------------------------------------------------
// nand access definitions
#define WRITE_NAND(x,y)         OUTREG16(x,y)   // 16-bit access
#define READ_NAND(x)            INREG16(x)      // 16-bit access
typedef UINT16                  NANDREG;        // 16-bit access

//-----------------------------------------------------------------------------
#define FIFO_THRESHOLD          (64)            // pre-fetch fifo config

//-----------------------------------------------------------------------------
// nand pin connection information
#define GPMC_NAND_CS            (0)             // NAND is on CHIP SELECT 0
#define GPMC_IRQ_WAIT_EDGE      (GPMC_IRQENABLE_WAIT0_EDGEDETECT)

//-----------------------------------------------------------------------------
// nand ecc parameters
#define GPMC_ECC_TOP_SECTOR		(0x3<<4) // page is 4 of 512 B sectors (2048)
#define GPMC_ECC_CONFIG         (GPMC_ECC_CONFIG_16BIT|GPMC_NAND_CS|GPMC_ECC_TOP_SECTOR)
#define ECC_PARITY_ARRAY_SIZE   ((SECTOR_DATA_SIZE >> 9) * 3)

typedef struct
{
    UCHAR hwBadBlock[2];		// Hardware bad block flag
    UCHAR ecc[ECC_BYTES];		// BootROM expect ECC starts from the 3rd byte when bus width is 16
    UCHAR reserved1[4];         // Reserved - used by FAL
    UCHAR reserved2[2];			// Reserved - used by FAL
    UCHAR swBadBlock;         	// Software bad block flag
    UCHAR oemReserved;          // For use by OEM
    UCHAR unused[2];			// Unused
}NAND_SPARE_AREA;

//-----------------------------------------------------------------------------
// typedefs and enums
typedef enum {
    kPrefetchOff,
    kPrefetchRead,
    kPrefetchWrite,
} PrefetchMode_e;

typedef struct {
    CRITICAL_SECTION        cs;
    BOOL                    bNandLockMode;
    PrefetchMode_e          prefetchMode;
    volatile NANDREG       *pNandCmd;
    volatile NANDREG       *pNandAddress;
    volatile NANDREG       *pNandData;
    volatile NANDREG       *pFifo;
    OMAP_GPMC_REGS         *pGpmcRegs;
    DWORD                   memBase[2];
    DWORD                   memLen[2];
    DWORD                   timeout;
    NAND_INFO               nandInfo;
    EccType_e               ECCtype;
    DWORD                   ECCsize; /* number of bytes */
} NandDevice_t;

//-----------------------------------------------------------------------------
// global variables

static NandDevice_t s_Device;
static HANDLE s_hNand = 0;

pfnFMD_ReadSectorNotify  g_fnFMD_ReadSectorNotify = 0;

__inline OMAP_GPMC_REGS *NAND_GetGpmcRegs(HANDLE hNand);

//-----------------------------------------------------------------------------
__inline NAND_INFO const *NAND_GetGeometry(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_LockBlocks(HANDLE hNand, UINT blockStart, UINT blockEnd, BOOL bLock);

//-----------------------------------------------------------------------------
void NAND_Uninitialize(HANDLE hNand);

//-----------------------------------------------------------------------------
HANDLE NAND_Initialize(LPCTSTR szContext, PCI_REG_INFO *pRegIn, PCI_REG_INFO *pRegOut);

//-----------------------------------------------------------------------------
BOOL NAND_Seek(HANDLE hNand, SECTOR_ADDR sector, UINT offset);

//-----------------------------------------------------------------------------
BOOL NAND_Read(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc);

//-----------------------------------------------------------------------------
BOOL NAND_Write(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc);

//-----------------------------------------------------------------------------
UINT16 NAND_GetStatus(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_EraseBlock(HANDLE hNand, BLOCK_ID blockId);

//-----------------------------------------------------------------------------
BOOL NAND_Enable(HANDLE hNand, BOOL bEnable);

//-----------------------------------------------------------------------------
UINT NAND_MutexEnter(HANDLE hNand);

//-----------------------------------------------------------------------------
UINT NAND_MutexExit(HANDLE hNand);

//-----------------------------------------------------------------------------
BOOL NAND_SendCommand(HANDLE hNand, UINT cmd);

//-----------------------------------------------------------------------------
BOOL NAND_ConfigurePrefetch(HANDLE hNand, UINT accessType);

//-----------------------------------------------------------------------------

void SetFMDNotify(pfnFMD_ReadSectorNotify fnFMD_ReadSectorNotify)
{
	g_fnFMD_ReadSectorNotify = fnFMD_ReadSectorNotify;
}

//-----------------------------------------------------------------------------
__inline void SectorAccess(volatile NANDREG *pReg, SECTOR_ADDR sector, UINT offset)
{
	// Offset is provided to this function in bytes; NAND device requires words
	offset >>= 1;
    WRITE_NAND(pReg, (offset & 0xFF));
    WRITE_NAND(pReg, ((offset >> 8) & 0xFF));
    WRITE_NAND(pReg, (sector & 0xFF));
    WRITE_NAND(pReg, ((sector >> 8) & 0xFF));
    WRITE_NAND(pReg, ((sector >> 16) & 0xFF));
}

//-----------------------------------------------------------------------------
__inline void BlockAccess(volatile NANDREG *pReg, BLOCK_ID blockId)
{
    blockId *= SECTORS_PER_BLOCK;
    WRITE_NAND(pReg, (blockId & 0xFF));
    WRITE_NAND(pReg, ((blockId >> 8) & 0xFF));
    WRITE_NAND(pReg, ((blockId >> 16) & 0xFF));
}

//-----------------------------------------------------------------------------
__inline void WaitOnEmptyWriteBufferStatus(NandDevice_t *pDevice)
{
    UINT status;
    do
    {
        status = INREG32(&pDevice->pGpmcRegs->GPMC_STATUS);
    } while ((status & GPMC_STATUS_EMPTYWRITEBUFFER) == 0);
}

//------------------------------------------------------------------------------
// Waits until the NAND status reads "ready"
// Note : a timout could be added but shouldn't be required
//
__inline void WaitForReadyStatus(HANDLE hNand)
{
    //  Wait for a Ready status
    while((NAND_GetStatus(hNand) & NAND_STATUS_READY) == 0); 
}


//-----------------------------------------------------------------------------
BOOL InitializePointers(LPCTSTR szContext, NandDevice_t *pDevice)
{
    UNREFERENCED_PARAMETER(szContext);
    UNREFERENCED_PARAMETER(pDevice);
	return 1;
}

//-----------------------------------------------------------------------------
__inline OMAP_GPMC_REGS *NAND_GetGpmcRegs(HANDLE hNand)
{
    return ((NandDevice_t*)hNand)->pGpmcRegs;
}


//-----------------------------------------------------------------------------
__inline NAND_INFO const *NAND_GetGeometry(HANDLE hNand)
{
    return &(((NandDevice_t*)hNand)->nandInfo);
}

//-----------------------------------------------------------------------------
BOOL NAND_LockBlocks(HANDLE hNand, UINT blockStart, UINT blockEnd, BOOL bLock)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    UNREFERENCED_PARAMETER(blockEnd);
    UNREFERENCED_PARAMETER(blockStart);

    if(!pDevice)
		return 0;

    if(bLock)
	{
        CLRREG32(&pDevice->pGpmcRegs->GPMC_CONFIG, GPMC_CONFIG_WRITEPROTECT);        

		do
		{
		}while(NAND_GetStatus(pDevice) & NAND_STATUS_NWP);
	}
    else
	{
        SETREG32(&pDevice->pGpmcRegs->GPMC_CONFIG, GPMC_CONFIG_WRITEPROTECT);

		do
		{
		}while((NAND_GetStatus(pDevice) & NAND_STATUS_NWP) == 0);
	}

	OALMSG(0, (L"NAND_LockBlocks(all %x).\r\n", bLock));
    
    return 1;
}

BOOL NAND_SetFeature(HANDLE hNand, UINT32 fe_om)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

	OALMSG(0, (L"NAND_set_feature: %d\r\n", fe_om));

	//if(pDevice->ECCtype == HW4bit)
	{
		WaitForReadyStatus(s_hNand);
		// Send Read Parameter Page Command
		NAND_SendCommand(pDevice, NAND_CMD_SET_FEATURE);

		// Send Address 90h
		WRITE_NAND(pDevice->pNandAddress, 0x90);

		WRITE_NAND(pDevice->pNandData, fe_om);
		fe_om = 0;
		WRITE_NAND(pDevice->pNandData, fe_om);
		WRITE_NAND(pDevice->pNandData, fe_om);
		WRITE_NAND(pDevice->pNandData, fe_om);

		WaitForReadyStatus(s_hNand);
	}

	return 0;
}

VOID NAND_SelectECCtype(EccType_e ecc)
{
	UINT32 fe_om;
	s_Device.ECCtype = ecc;
	s_Device.ECCsize = (s_Device.ECCtype == ONDIE4bit)? 0:
					   (s_Device.ECCtype == Hamming1bit)? ECC_BYTES_HAMMING:
					   (s_Device.ECCtype == BCH4bit)? ECC_BYTES_BCH4: ECC_BYTES_BCH8;

	fe_om = (s_Device.ECCtype == ONDIE4bit)?NAND_FEATURE_ENABLE_ECC:0;

	NAND_SendCommand(&s_Device, NAND_CMD_RESET);

    // Wait for NAND
    while((NAND_GetStatus(&s_Device) & NAND_STATUS_READY) == 0);
	NAND_SetFeature(&s_Device, fe_om);
}

//-----------------------------------------------------------------------------
void NAND_Uninitialize(HANDLE hNand)
{
    UNREFERENCED_PARAMETER(hNand);
}

//-----------------------------------------------------------------------------
HANDLE NAND_Initialize(LPCTSTR szContext, PCI_REG_INFO *pRegIn, PCI_REG_INFO *pRegOut)
{
    HANDLE hDevice = 0;
    UINT ffPrefetchMode = 0;
    UINT8 manufacturer, device;
    NandDevice_t *pDevice = &s_Device;
	UINT32 CPU_Rev;

    // initialize structure
    memset(pDevice, 0, sizeof(NandDevice_t));

	CPU_Rev = Get_CPUVersion();
//	CPU_Id  = CPU_ID(CPU_Rev);
	CPU_Rev = CPU_REVISION(CPU_Rev);

    pDevice->pGpmcRegs = (OMAP_GPMC_REGS*)OALPAtoUA(OMAP_GPMC_REGS_PA);
    pDevice->pFifo = (NANDREG*)OALPAtoUA(pRegIn->MemBase.Reg[0]);

    pDevice->pNandCmd = (volatile NANDREG*)((UINT32)pDevice->pGpmcRegs + offset(OMAP_GPMC_REGS, GPMC_NAND_COMMAND_0));
    pDevice->pNandAddress= (volatile NANDREG*)((UINT32)pDevice->pGpmcRegs + offset(OMAP_GPMC_REGS, GPMC_NAND_ADDRESS_0));
    pDevice->pNandData= (volatile NANDREG*)((UINT32)pDevice->pGpmcRegs + offset(OMAP_GPMC_REGS, GPMC_NAND_DATA_0));

    // Enable GPMC wait-to-nowait edge detection mechanism on NAND R/B pin
    NAND_Enable(pDevice, 1);

    // Write RESET command
    // (a reset aborts any current READ, WRITE (PROGRAM) or ERASE operation)
    NAND_SendCommand(pDevice, NAND_CMD_RESET);

    // Wait for NAND
    while((NAND_GetStatus(pDevice) & NAND_STATUS_READY) == 0);

    // Send Read ID Command
    NAND_SendCommand(pDevice, NAND_CMD_READID);

    // Send Address 00h
    WRITE_NAND(pDevice->pNandAddress, 0);

    // Read the manufacturer ID & device code
    manufacturer = (UINT8)READ_NAND(pDevice->pNandData);
    device = (UINT8)READ_NAND(pDevice->pNandData);

	OALMSG(1, (L"NAND_Initialize: CPU ver (%x) NAND(%d, %x)\r\n", CPU_Rev, manufacturer, device));

	switch(manufacturer)
	{
		case MANUFACTURER_ID_MT29F2G16:
//		case MANUFACTURER_ID_MT29C4G48:
//		case MANUFACTURER_ID_MT29F8G16:
			if(device == NAND_DEVICE_ID_MT29F2G16)
				break;
			if(device == NAND_DEVICE_ID_MT29C4G48)
				break;
			if(device == NAND_DEVICE_ID_MT29F8G16)
				break;
		case MANUFACTURER_ID_02GR3B2D:
			if(device == NAND_DEVICE_ID_02GR3B2D)
				break;
		case MANUFACTURER_ID_H8KDS0UN0MER_46M:
			if(device == NAND_DEVICE_ID_H8KDS0UN0MER_46M)
				break;
		default:
		{
			OALMSG(1, (L"Unknown NAND device(%x, %x).\r\n", manufacturer, device));
			return 0;
		}
	}
    
    //  Enable and reset ECC engine (workaround for engine giving 0s first time)
//	NAND_SelectECCtype(Hamming1bit);
	s_Device.ECCtype = Hamming1bit;
	s_Device.ECCsize = ECC_BYTES_HAMMING;
    ECC_Init(pDevice->pGpmcRegs, GPMC_ECC_CONFIG, pDevice->ECCtype, NAND_ECC_READ);
    ECC_Reset(pDevice->pGpmcRegs);

    //  Only enable during NAND read/write/erase operations
    NAND_Enable(pDevice, 0);

    // configure the prefetch engine
    pDevice->prefetchMode = kPrefetchOff;
    OUTREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONTROL, 0);
    
    // set prefetch mask
    ffPrefetchMode = GPMC_PREFETCH_CONFIG_SYNCHROMODE |
                     GPMC_PREFETCH_CONFIG_PFPWENROUNDROBIN |
                     GPMC_PREFETCH_CONFIG_ENABLEOPTIMIZEDACCESS |
                     GPMC_PREFETCH_CONFIG_WAITPINSELECTOR(GPMC_NAND_CS) |
                     GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD(FIFO_THRESHOLD) |
                     GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR(GPMC_NAND_CS);

    OUTREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONFIG1, ffPrefetchMode);

    // configure prefetch engine
    OUTREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONFIG2, SECTOR_DATA_SIZE);
    SETREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONFIG1, GPMC_PREFETCH_CONFIG_ENABLEENGINE);

    pDevice->nandInfo.deviceId = device;	//NAND_DEVICE_ID;
    pDevice->nandInfo.manufacturerId = manufacturer;//MANUFACTURER_ID;

    pDevice->nandInfo.blocks = (pDevice->nandInfo.deviceId == NAND_DEVICE_ID_MT29F2G16)? NUMBER_OF_BLOCKS_MT29F2G16:
							   (pDevice->nandInfo.deviceId == NAND_DEVICE_ID_MT29C4G48)? NUMBER_OF_BLOCKS_MT29C4G48:
							   (pDevice->nandInfo.deviceId == NAND_DEVICE_ID_MT29F8G16)? NUMBER_OF_BLOCKS_MT29F8G16:
							   (pDevice->nandInfo.deviceId == NAND_DEVICE_ID_02GR3B2D)? NUMBER_OF_BLOCKS_02GR3B2D:
							   (pDevice->nandInfo.deviceId == NAND_DEVICE_ID_H8KDS0UN0MER_46M)? NAND_DEVICE_ID_H8KDS0UN0MER_46M:
							   NUMBER_OF_BLOCKS_MT29C4G48;
    pDevice->nandInfo.sectorSize = SECTOR_DATA_SIZE;
    pDevice->nandInfo.sectorsPerBlock = SECTORS_PER_BLOCK;
    pDevice->nandInfo.wordData = WORD_ACCESS_SIZE;
    
    // We are done
    hDevice = pDevice;

    return hDevice;
}

//-----------------------------------------------------------------------------
BOOL NAND_Seek(HANDLE hNand, SECTOR_ADDR sector, UINT offset)
{
    // seek to address
    NandDevice_t *pDevice = (NandDevice_t*)hNand;
    SectorAccess(pDevice->pNandAddress, sector, offset);

    return 1;
}

//-----------------------------------------------------------------------------
BOOL NAND_Read(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc)
{
    UINT32 fifoLevel;
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    // Start ECC if a valid ECC buffer is passed in
    if(pEcc)
        ECC_Init(pDevice->pGpmcRegs, GPMC_ECC_CONFIG, pDevice->ECCtype, NAND_ECC_READ);

    // enable prefetch if it's been properly configured
    if(pDevice->prefetchMode == kPrefetchRead)
	{
        SETREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONTROL, GPMC_PREFETCH_CONTROL_STARTENGINE);

        // start copying data into passed in buffer
        while(size > 0)
		{
            // wait for fifo threshold to be reached
            fifoLevel = 0;
            while(fifoLevel < FIFO_THRESHOLD)
			{
                fifoLevel = INREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_STATUS);
                fifoLevel &= GPMC_PREFETCH_STATUS_FIFOMASK;
                fifoLevel >>= GPMC_PREFETCH_STATUS_FIFOSHIFT;
			}

            // copy data to buffer
            memcpy(pData, (BYTE*)pDevice->pFifo, FIFO_THRESHOLD);
            pData += FIFO_THRESHOLD;
            size -= FIFO_THRESHOLD;
		}

        // NOTE:
        //  Prefetch engine will automatically stop on the completion
        // of data transfer
        pDevice->prefetchMode = kPrefetchOff;
	}
    else
	{
        // NOTE:
        //  Code assumes the entire page is read at once
        while(size >= sizeof(NANDREG))
		{
            *(NANDREG*)pData = READ_NAND(pDevice->pNandData);
            pData += sizeof(NANDREG);
            size -= sizeof(NANDREG);

			// workaround for BCH engine when ECC is not put at the end of OOB area.  
			// the checking is based on puting ECC at the BootROM expected location -
			// with 2 bytes offset from the start of the OOB area

            if(pDevice->ECCtype == BCH4bit || pDevice->ECCtype == BCH8bit)
			{
                if(size == (int)(sizeof(NAND_SPARE_AREA) - ECC_OFFSET - pDevice->ECCsize))
					ECC_Reset(pDevice->pGpmcRegs);
			}
		}
	}

    // get ECC result
    if(pEcc)
	{
        // UNDONE:
        //  should pass in sector size???
        ECC_Result(pDevice->pGpmcRegs, pEcc, pDevice->ECCsize);
	}
	else
	{
        ECC_Reset(pDevice->pGpmcRegs);
	}
    
    return 1;
}


//-----------------------------------------------------------------------------
BOOL NAND_Write(HANDLE hNand, BYTE *pData, int size, BYTE *pEcc)
{
    UINT32 fifoLevel;
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    // Start ECC if a valid ECC buffer is passed in
    if(pEcc)
		ECC_Init(pDevice->pGpmcRegs, GPMC_ECC_CONFIG, pDevice->ECCtype, NAND_ECC_WRITE);

    // enable prefetch if it's been properly configured
    if(pDevice->prefetchMode == kPrefetchWrite )
	{
        SETREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONTROL, GPMC_PREFETCH_CONTROL_STARTENGINE);

        // start copying data into passed in buffer
        while(size)
		{
            // copy data to buffer
            memcpy((BYTE*)pDevice->pFifo, pData, FIFO_THRESHOLD);
            pData += FIFO_THRESHOLD;
            size -= FIFO_THRESHOLD;  
            
            // wait for fifo threshold to be reached
            fifoLevel = 0;
            while(fifoLevel < FIFO_THRESHOLD)
			{
                fifoLevel = INREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_STATUS);
                fifoLevel &= GPMC_PREFETCH_STATUS_FIFOMASK;
                fifoLevel >>= GPMC_PREFETCH_STATUS_FIFOSHIFT;
			}
		}

        // NOTE:
        //  Prefetch engine will automatically stop on the completion
        // of data transfer
        pDevice->prefetchMode = kPrefetchOff;
	}
    else
	{
        int writeCount = 0;
		
        while(size >= sizeof(NANDREG))
		{
            WRITE_NAND(pDevice->pNandData, *(NANDREG*)pData);

            // Workaround Errata 1.53
            // need to check on EMPTYWRITEBUFFERSTATUS on every
            // 255 bytes
            if(++writeCount >= 255)
			{
                WaitOnEmptyWriteBufferStatus(pDevice);
                writeCount = 0;
			}
            
            pData += sizeof(NANDREG);
            size -= sizeof(NANDREG);
		}
	}

    // get ECC result
    if(pEcc)
	{
        ECC_Result(pDevice->pGpmcRegs, pEcc, pDevice->ECCsize);
	}
	else
	{
        ECC_Reset(pDevice->pGpmcRegs);
	}

    return 1;
}

//-----------------------------------------------------------------------------
UINT16 NAND_GetStatus(HANDLE hNand)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    // request status
    WRITE_NAND(pDevice->pNandCmd, NAND_CMD_STATUS);

    return READ_NAND(pDevice->pNandData);
}

//-----------------------------------------------------------------------------
BOOL NAND_EraseBlock(HANDLE hNand, BLOCK_ID blockId)
{    
    NandDevice_t *pDevice = (NandDevice_t*)hNand;
    
    // Calculate the sector number
    NAND_SendCommand(hNand, NAND_CMD_ERASE_SETUP);    
    BlockAccess(pDevice->pNandAddress, blockId);
    NAND_SendCommand(hNand, NAND_CMD_ERASE_CONFIRM);
    
    return 1;
}

//-----------------------------------------------------------------------------
BOOL NAND_Enable(HANDLE hNand, BOOL bEnable)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    if(bEnable)
    {
        // Enable GPMC wait-to-nowait edge detection mechanism on NAND R/B pin
        SETREG32(&pDevice->pGpmcRegs->GPMC_IRQENABLE, GPMC_IRQ_WAIT_EDGE);
        //  Reset IRQ status
        SETREG32(&pDevice->pGpmcRegs->GPMC_IRQSTATUS, GPMC_IRQ_WAIT_EDGE);
    }
    else
    {
        //  Reset IRQ status
        SETREG32(&pDevice->pGpmcRegs->GPMC_IRQSTATUS, GPMC_IRQ_WAIT_EDGE);
        // Disable GPMC wait-to-nowait edge detection mechanism on NAND R/B pin
        CLRREG32(&pDevice->pGpmcRegs->GPMC_IRQENABLE, GPMC_IRQ_WAIT_EDGE);
    }

    return 1;
}

//-----------------------------------------------------------------------------
BOOL NAND_SendCommand(HANDLE hNand, UINT cmd)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    WRITE_NAND(pDevice->pNandCmd, cmd);

	return 1;
}

//-----------------------------------------------------------------------------
BOOL NAND_ConfigurePrefetch(HANDLE hNand, UINT accessType)
{
    UINT ffPrefetchMode = 0;
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    // disable prefetch engine
    pDevice->prefetchMode = kPrefetchOff;
    
    OUTREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONTROL, 0);

    // set prefetch mask
    ffPrefetchMode = GPMC_PREFETCH_CONFIG_PFPWENROUNDROBIN |
                     GPMC_PREFETCH_CONFIG_ENABLEOPTIMIZEDACCESS |
                     GPMC_PREFETCH_CONFIG_WAITPINSELECTOR(GPMC_NAND_CS) |
                     GPMC_PREFETCH_CONFIG_FIFOTHRESHOLD(FIFO_THRESHOLD) |
                     GPMC_PREFETCH_CONFIG_ENGINECSSELECTOR(GPMC_NAND_CS);

    if (accessType == NAND_DATA_WRITE)
	{
        pDevice->prefetchMode = kPrefetchWrite;
        ffPrefetchMode |= GPMC_PREFETCH_CONFIG_WRITEPOST;
	}
    else
        pDevice->prefetchMode = kPrefetchRead;
        
    OUTREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONFIG1, ffPrefetchMode);

    SETREG32(&pDevice->pGpmcRegs->GPMC_PREFETCH_CONFIG1, GPMC_PREFETCH_CONFIG_ENABLEENGINE);

    return 1;
}


//-----------------------------------------------------------------------------
BOOL NAND_CorrectEccData(HANDLE hNand, BYTE *pData, UINT size, BYTE const *pEccOld, BYTE const *pEccNew)
{
    NandDevice_t *pDevice = (NandDevice_t*)hNand;

    // this call assumes the array size of pEccOld and pEccNew are of the 
    // correct size to hold all the parity bits of the given size
    
    if(memcmp(pEccOld, pEccNew,  pDevice->ECCsize) != 0)
	{
        // check if data is correctable
        if(!ECC_CorrectData(pDevice->pGpmcRegs, pData, size, pEccOld, pEccNew))
			return 0;
	}    

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_Init
//
//  This function is called to initialize flash subsystem.
//
VOID *FMD_Init(LPCTSTR szContext, PCI_REG_INFO *pRegIn, PCI_REG_INFO *pRegOut)
{
	OALMSG(0, (L"!!!!!!!!!!!! FMD_Init !!!!!!!!!!!!!!!!!!!!!\r\n"));

    // Map NAND registers
    s_hNand = NAND_Initialize(szContext, pRegIn, pRegOut);

    return s_hNand;
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_Deinit
//
BOOL FMD_Deinit(VOID *pContext)
{
	OALMSG(0, (L"!!!!!!!!!!!! FMD_Deinit !!!!!!!!!!!!!!!!!!!!!\r\n"));

    if(pContext != s_hNand)
		return 0;

    //  Only enable during NAND read/write/erase operations
    NAND_Enable(s_hNand, 1);

    // Wait for NAND    
    // the next command may not work if you remove this wait on the status, 
    // because if the R/B pin is asserted during the reset, its deassertion 
    // isn't guaranteed to mean that the device is ready
    WaitForReadyStatus(s_hNand);   

    // Write RESET command
    // (a reset aborts any current READ, WRITE (PROGRAM) or ERASE operation)
    NAND_SendCommand(s_hNand, NAND_CMD_RESET);

    // Wait for NAND
    WaitForReadyStatus(s_hNand);

    // Clear GPMC wait-to-nowait edge detection mechanism on NAND R/B pin
    NAND_Enable(s_hNand, 0);

    // uninitialize and release allocated resources
    NAND_Uninitialize(s_hNand);
    s_hNand = 0;
    
    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_GetInfo
//
//  This function is call to get flash information
//
BOOL FMD_GetInfo(FlashInfo *pFlashInfo)
{
    // If we don't support NAND, fail...
    if(!s_hNand)
		return 0;

    // Memory type is NAND
    pFlashInfo->flashType = NAND;
    pFlashInfo->dwNumBlocks = NAND_GetGeometry(s_hNand)->blocks;
    pFlashInfo->wSectorsPerBlock = NAND_GetGeometry(s_hNand)->sectorsPerBlock;
    pFlashInfo->wDataBytesPerSector = NAND_GetGeometry(s_hNand)->sectorSize;
    pFlashInfo->dwBytesPerBlock = NAND_GetGeometry(s_hNand)->sectorSize;
    pFlashInfo->dwBytesPerBlock *= NAND_GetGeometry(s_hNand)->sectorsPerBlock;

    return 1;
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_ReadSector
//
//  Read the content of the sector.
//
#define USE_ON_DIE_ECC 1
BOOL NAND_ReadPageAndECC(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo, EccType_e ecc)
{
    UINT32 pageSize;
    NAND_SPARE_AREA sa;
    BYTE rgEcc[ECC_BYTES];

    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;

	NAND_SelectECCtype(ecc);

	// Clear out spare area struct
	memset(&sa, 0xFF, sizeof(NAND_SPARE_AREA));


	// be sure to do this before sending the READ command and not after ! or the
	// status register would remain at status read mode, which would have to be changed
	// before retreiving the data
	WaitForReadyStatus(s_hNand);

	// Read page
	if(Buff)
	{
		// configure prefetch engine for input and start it
		NAND_ConfigurePrefetch(s_hNand, NAND_DATA_READ);
       
		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);

		// Send the address
		NAND_Seek(s_hNand, page, 0);

		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ2);

		// Wait for the action to finish
		WaitForReadyStatus(s_hNand);

		//Force a read here, else we will read the status again
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);
            
		NAND_Read(s_hNand, Buff, pageSize, rgEcc);
	}
    else
	{
        // Send the command
        NAND_SendCommand(s_hNand, NAND_CMD_READ1);

        // Send the address
        NAND_Seek(s_hNand, page, pageSize);

        // Send the command
        NAND_SendCommand(s_hNand, NAND_CMD_READ2);

        // Wait for the action to finish
        WaitForReadyStatus(s_hNand);

        //Force a read here, else we will read the status again
        NAND_SendCommand (s_hNand, NAND_CMD_READ1);
	}

    // read spare area
    NAND_Read(s_hNand, (BYTE*)&sa, sizeof(sa), 0);

	// Make sure of the NAND status
	WaitForReadyStatus(s_hNand);

	// Copy sector info
	if(sectorInfo)
	{
		sectorInfo->bBadBlock    = sa.hwBadBlock[0] & sa.hwBadBlock[1]; // HW bad block check
		sectorInfo->bBadBlock    &= sa.swBadBlock;						// SW bad block flag check
		sectorInfo->bOEMReserved = sa.oemReserved;

		memcpy(&sectorInfo->dwReserved1, sa.reserved1, sizeof(sectorInfo->dwReserved1));
		memcpy(&sectorInfo->wReserved2, sa.reserved2, sizeof(sectorInfo->wReserved2));
	}

	// perform ecc correction and correct data when possible
	if(Buff && (!NAND_CorrectEccData(s_hNand, Buff, pageSize, sa.ecc, rgEcc)))
	{
		UINT count;
		UCHAR *Data = Buff;

		for(count = 0; count < sizeof(sa); count++)
		{
			// Allow OEMReserved byte to be set to reserved/readonly
			if (&(((UINT8*)&sa)[count]) == &sa.oemReserved)
				continue;
			if(((UINT8*)&sa)[count] != 0xFF)
			{
				OALMSG(1, (L"NAND_ReadPageAndECC: impossible to correct SA of page %d\r\n", page));	

				return 0;
			}
		}

		for(count = 0; count < pageSize; count++)
		{
			if(*Data != 0xFF) 
			{
				OALMSG(1, (L"NAND_ReadPageAndECC: impossible to correct page %d\r\n", page));	

				return 0;
			}
			++Data;
		}
	}

    return 1;
}

BOOL NAND_ReadPageUsingOnDieECC(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
    NAND_SPARE_AREA sa;
    UINT32 pageSize;
	UINT8 b[4];
	UINT16 stat;

	//  Change idle mode to no-idle to ensure access to GPMC registers
    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;
	NAND_SelectECCtype(ONDIE4bit);

	memset(&sa, 0xFF, sizeof(NAND_SPARE_AREA));

	// be sure to do this before sending the READ command and not after ! or the
	// status register would remain at status read mode, which would have to be changed
	// before retreiving the data
	WaitForReadyStatus(s_hNand);

	// Read page
	if(Buff)
	{
		// configure prefetch engine for input and start it
		NAND_ConfigurePrefetch(s_hNand, NAND_DATA_READ);
       
		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);

		// Send the address
		NAND_Seek(s_hNand, page, 0);

		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ2);

		// Wait for the action to finish
		WaitForReadyStatus(s_hNand);

		//Force a read here, else we will read the status again
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);
            
		NAND_Read(s_hNand, Buff, pageSize, 0);
	}

	// Make sure of the NAND status
	WaitForReadyStatus(s_hNand);

	// Send the command
	NAND_SendCommand(s_hNand, NAND_CMD_READ1);

	// Send the address
	NAND_Seek(s_hNand, page, pageSize + 4);

	// Send the command
	NAND_SendCommand(s_hNand, NAND_CMD_READ2);

	// Wait for the action to finish
	WaitForReadyStatus(s_hNand);
	stat = NAND_GetStatus(s_hNand);
	if(stat & NAND_STATUS_ERROR)
	{
		OALMSG(1, (L"NAND_ReadPageUsingOnDieECC: Uncorrectable error in page %d\r\n", page));
		return 0;
	}
	if(stat & NAND_STATUS_CORRECTED)
	{
		OALMSG(1, (L"NAND_ReadPageUsingOnDieECC: recoverable error in page %d\r\n", page));
	}

	//Force a read here, else we will read the status again
	NAND_SendCommand(s_hNand, NAND_CMD_READ1);
	NAND_Read(s_hNand, b, 4, 0);


	memcpy(&sa.reserved2, &b[0], sizeof(sa.reserved2));
	sa.swBadBlock = b[2];
	sa.oemReserved = b[3];

	if(sectorInfo)
	{
		sectorInfo->bBadBlock = 0xFF & sa.swBadBlock;
		sectorInfo->bOEMReserved = sa.oemReserved;
		memcpy(&sectorInfo->wReserved2, &sa.reserved2, sizeof(sectorInfo->wReserved2));
	}

	return 1;
}

BOOL NAND_ReadPageFromBootBlock(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
	return NAND_ReadPageAndECC(page, Buff, sectorInfo, Hamming1bit);
}

BOOL NAND_ReadPage(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
#if defined USE_ON_DIE_ECC
	return NAND_ReadPageUsingOnDieECC(page, Buff, sectorInfo);
#else
	return NAND_ReadPageAndECC(page, Buff, sectorInfo, BCH4bit);
#endif
}

BOOL FMD_ReadSector(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo, DWORD pages)
{
    UINT32 oldIdleMode;
    UINT32 pageSize, blockId;

    // Fail if FMD wasn't opened
    if(!s_hNand) 
	    return 0;
    
	//  Change idle mode to no-idle to ensure access to GPMC registers
    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;

    oldIdleMode = INREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG));
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), SYSCONFIG_NOIDLE);

	OALMSG(0, (L"FMD_ReadSector: %d %X, %X, %d\r\n", page, Buff, sectorInfo, pages));	
	
	while(pages > 0)
	{
        // Only enable during NAND read/write/erase operations
        NAND_Enable(s_hNand, 1);

		blockId = page/NAND_GetGeometry(s_hNand)->sectorsPerBlock;
		if(blockId == 0)
			NAND_ReadPageFromBootBlock(page, Buff, sectorInfo);
		else
			NAND_ReadPage(page, Buff, sectorInfo);

		// Only enable during NAND read/write/erase operations
		NAND_Enable(s_hNand, 0);

		// Move to next sector
        page++;
		if(Buff)
			Buff += pageSize;
        sectorInfo++;
        pages--;

		if(Buff && g_fnFMD_ReadSectorNotify) 
			g_fnFMD_ReadSectorNotify();
	}

    // Restore idle mode
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), oldIdleMode);
    
    return (pages == 0);
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_WriteSector
//
BOOL NAND_WritePageAndECC(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo, EccType_e ecc)
{
    UINT32 pageSize;
    NAND_SPARE_AREA sa;

    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;

	NAND_SelectECCtype(ecc);

	// Clear out spare area struct
	memset(&sa, 0xFF, sizeof(NAND_SPARE_AREA));

	if(Buff)
	{
		// enable prefetch
		NAND_ConfigurePrefetch(s_hNand, NAND_DATA_WRITE);

		// send the write command
		NAND_SendCommand(s_hNand, NAND_CMD_WRITE1);

		// send the address to write to
		NAND_Seek(s_hNand, page, 0);

		// send data to flash
		NAND_Write(s_hNand, Buff, pageSize, sa.ecc);
	}
	else
	{
		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);

		// Send the address
		NAND_Seek(s_hNand, page, NAND_GetGeometry(s_hNand)->sectorSize);

		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ2);

		// Wait for the action to finish
		WaitForReadyStatus(s_hNand);

		//Force a read here, else we will read the status again
		NAND_SendCommand (s_hNand, NAND_CMD_READ1);

        // read spare area
        NAND_Read(s_hNand, (BYTE*)&sa, sizeof(sa), 0);

		//send the write command
		NAND_SendCommand(s_hNand, NAND_CMD_WRITE1);

		//send the address to write to
		NAND_Seek(s_hNand, page, pageSize);
	}

	if(sectorInfo)
	{
		// Fill in rest of spare area info (we already have ECC from above)
		sa.swBadBlock     = sectorInfo->bBadBlock;
		sa.oemReserved  = sectorInfo->bOEMReserved;
		memcpy(sa.reserved1, &sectorInfo->dwReserved1, sizeof(sa.reserved1));
		memcpy(sa.reserved2, &sectorInfo->wReserved2, sizeof(sa.reserved2));
	}

	// write new spare info
	NAND_Write(s_hNand, (BYTE *)&sa, sizeof(sa), 0);

	// initiate the data programming process :
	NAND_SendCommand(s_hNand, NAND_CMD_WRITE2);
	OALStall(600);

	// wait until completion of the operation :
	WaitForReadyStatus(s_hNand);

	if(NAND_GetStatus(s_hNand) & NAND_STATUS_ERROR)
	{
		OALMSG(1, (L"FMD_WriteSector: Impossible to program SA of page %d\r\n", page));
		return 0;
	}

    return 1;
}
BOOL NAND_WritePageUsingOnDieECC(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
    UINT32 pageSize;
    NAND_SPARE_AREA sa;

    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;

	NAND_SelectECCtype(ONDIE4bit);

	// Clear out spare area struct
	memset(&sa, 0xFF, sizeof(NAND_SPARE_AREA));

	if(Buff)
	{
		// enable prefetch
		NAND_ConfigurePrefetch(s_hNand, NAND_DATA_WRITE);

		// send the write command
		NAND_SendCommand(s_hNand, NAND_CMD_WRITE1);

		// send the address to write to
		NAND_Seek(s_hNand, page, 0);

		NAND_Write(s_hNand, Buff, pageSize, 0);
	}
	else
	{
		UINT8 b[4];
		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ1);

		// Send the address
		NAND_Seek(s_hNand, page, pageSize + 4);

		// Send the command
		NAND_SendCommand(s_hNand, NAND_CMD_READ2);

		// Wait for the action to finish
		WaitForReadyStatus(s_hNand);

		//Force a read here, else we will read the status again
		NAND_SendCommand (s_hNand, NAND_CMD_READ1);

		// read spare area

		NAND_Read(s_hNand, b, 4, 0);
		
		memcpy(&sa.reserved2, &b[0], sizeof(sa.reserved2));
		sa.swBadBlock = b[2];
		sa.oemReserved = b[3];

		//send the write command
		NAND_SendCommand(s_hNand, NAND_CMD_WRITE1);

		//send the address to write to
		NAND_Seek(s_hNand, page, pageSize);
	}

	if(sectorInfo)
	{
		memcpy(&sa.ecc[2], &sectorInfo->wReserved2, sizeof(sectorInfo->wReserved2));
		sa.ecc[4] = sectorInfo->bBadBlock;
		sa.ecc[5] = sectorInfo->bOEMReserved;
	}

	// write new spare info
	NAND_Write(s_hNand, (BYTE *)&sa, sizeof(sa), 0);

	// initiate the data programming process :
	NAND_SendCommand(s_hNand, NAND_CMD_WRITE2);
	OALStall(600);

	// wait until completion of the operation :
	WaitForReadyStatus(s_hNand);

	if(NAND_GetStatus(s_hNand) & NAND_STATUS_ERROR)
	{
		OALMSG(1, (L"NAND_WritePageUsingOnDieECC: Impossible to program SA of page %d\r\n", page));
		return 0;
	}

    return 1;
}

BOOL NAND_WritePage2BootBlock(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
	return NAND_WritePageAndECC(page, Buff, sectorInfo, Hamming1bit);
}

BOOL NAND_WritePage(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo)
{
#if defined USE_ON_DIE_ECC
	return NAND_WritePageUsingOnDieECC(page, Buff, sectorInfo);
#else
	return NAND_WritePageAndECC(page, Buff, sectorInfo, BCH4bit);
#endif
}

BOOL FMD_WriteSector(SECTOR_ADDR page, UCHAR *Buff, SectorInfo *sectorInfo, DWORD pages)
{
    UINT32 oldIdleMode;
    UINT32 pageSize, blockId;
    SECTOR_ADDR startPage = page;
    SECTOR_ADDR endPage = page + pages;

    // Fail if FMD wasn't opened
    if(!s_hNand)
		return 0;

    pageSize = NAND_GetGeometry(s_hNand)->sectorSize;

    //  Change idle mode to no-idle to ensure access to GPMC registers
    oldIdleMode = INREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG));
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), SYSCONFIG_NOIDLE);

    //  Only enable during NAND read/write/erase operations    
    NAND_Enable(s_hNand, 1);
    NAND_LockBlocks(s_hNand, startPage, endPage, 0);

	OALMSG(0, (L"FMD_WriteSector: %d %X, %X, %d\r\n", page, Buff, sectorInfo, pages));	
	while(pages)
    {
		blockId = page/NAND_GetGeometry(s_hNand)->sectorsPerBlock;

		if(blockId == 0)
			NAND_WritePage2BootBlock(page, Buff, sectorInfo);
		else
			NAND_WritePage(page, Buff, sectorInfo);

		// Move to next sector
		page++;
		if(Buff)
			Buff += pageSize;
		sectorInfo++;
		pages--;

		if (g_fnFMD_ReadSectorNotify) 
			g_fnFMD_ReadSectorNotify();		
	}

    //  Only enable during NAND read/write/erase operations    
    NAND_LockBlocks(s_hNand, startPage, endPage, 1);
    NAND_Enable(s_hNand, 0);

    // Restore idle mode
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), oldIdleMode);

    return (pages == 0);
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_EraseBlock
//
//  Erase the given block
//
BOOL FMD_EraseBlock(BLOCK_ID blockId)
{
    UINT32 oldIdleMode;
	BOOL erased;

    // Check if we know flash geometry
    if(!s_hNand)
		return 0;

    //  Change idle mode to no-idle to ensure access to GPMC registers
    oldIdleMode = INREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG));
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), SYSCONFIG_NOIDLE);

    //  Only enable during NAND read/write/erase operations    
    NAND_Enable(s_hNand, 1);
    NAND_LockBlocks(s_hNand, blockId, blockId, 0);
    
    // send block id to erase
    NAND_EraseBlock(s_hNand, blockId);

    // wait for completion    
    WaitForReadyStatus(s_hNand);

    //Verify there wasn't any error by checking the NAND status register :
	erased = ((NAND_GetStatus(s_hNand) & NAND_STATUS_ERROR) == 0);

	OALMSG(!erased, (L"FMD_EraseBlock: Cannot erase block (%d)\r\n", blockId));

	//  Only enable during NAND read/write/erase operations    
    NAND_LockBlocks(s_hNand, blockId, blockId, TRUE);
    NAND_Enable(s_hNand, FALSE);

    //  Change idle mode back
    OUTREG32(&(NAND_GetGpmcRegs(s_hNand)->GPMC_SYSCONFIG), oldIdleMode);

    return erased;
}


//------------------------------------------------------------------------------
//
//  Function: FMD_GetBlockStatus
//
DWORD FMD_GetBlockStatus(BLOCK_ID blockId)
{
    SECTOR_ADDR page;
	DWORD marked;
    SectorInfo sectorInfo[2];

    // Check if we know flash geometry
    if(!s_hNand)
		return 0;

    // Calculate sector
    page = blockId * NAND_GetGeometry(s_hNand)->sectorsPerBlock;
	marked = 0;

    if(!FMD_ReadSector(page, 0, sectorInfo, 2))
	{
		OALMSG(1, (L"FMD_GetBlockStatus: unknown block (%d)\r\n", blockId));
        return BLOCK_STATUS_UNKNOWN;
	}

    if((sectorInfo[0].bBadBlock != 0xFF) || (sectorInfo[1].bBadBlock != 0xFF))
	{
		OALMSG(1, (L"FMD_GetBlockStatus: Bad block (%d)\r\n", blockId));
        marked |= BLOCK_STATUS_BAD;
	}

    if(0 == (sectorInfo[0].bOEMReserved & OEM_BLOCK_READONLY))
	{
		OALMSG(0, (L"FMD_GetBlockStatus: ro block (%d)\r\n", blockId));
        marked |= BLOCK_STATUS_READONLY;
	}

    if(0 == (sectorInfo[0].bOEMReserved & OEM_BLOCK_RESERVED))
	{
		OALMSG(0, (L"FMD_GetBlockStatus: reserved block (%d)\r\n", blockId));
        marked |= BLOCK_STATUS_RESERVED;
	}


    return marked;
}

//------------------------------------------------------------------------------
//
//  Function: FMD_SetBlockStatus
//
BOOL FMD_SetBlockStatus(BLOCK_ID blockId, DWORD status)
{
    SECTOR_ADDR page;
    SectorInfo sectorInfo;

    // Check if we know flash geometry
    if(!s_hNand)
		return 0;

	OALMSG(0, (L"FMD_SetBlockStatus: (%d, %x)\r\n", blockId, status));

    // Calculate sector
    page = blockId * NAND_GetGeometry(s_hNand)->sectorsPerBlock;

	if(FMD_ReadSector(page, 0, &sectorInfo, 1))
	{
		sectorInfo.bBadBlock	&= (status & BLOCK_STATUS_BAD)?0:0xFF;
		sectorInfo.bOEMReserved &= (status & BLOCK_STATUS_READONLY)? ~(OEM_BLOCK_READONLY) : 0xFF;
		sectorInfo.bOEMReserved &= (status & BLOCK_STATUS_RESERVED)? ~(OEM_BLOCK_RESERVED) : 0xFF;

        if(FMD_WriteSector(page, 0, &sectorInfo, 1))
		{
			OALMSG(1, (L"FMD_SetBlockStatus: set (%d, %x)\r\n", blockId, status));
			return 1;
		}
	}

    return 0;
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_PowerUp
//
VOID FMD_PowerUp(VOID)
{
    // exit if FMD wasn't opened
    if(!s_hNand)
		return;
    
    //  Only enable during NAND read/write/erase operations
    NAND_Enable(s_hNand, 1);

    // Wait for NAND
    WaitForReadyStatus(s_hNand);   

    // Write the reset command
    NAND_SendCommand(s_hNand, NAND_CMD_RESET);

    // Wait for NAND
    WaitForReadyStatus(s_hNand);

    //  Only enable during NAND read/write/erase operations
    NAND_Enable(s_hNand, 0);

	RETAILMSG(1, (L"FMD_PowerUp\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_PowerDown
//
VOID FMD_PowerDown(VOID)
{
	RETAILMSG(1, (L"FMD_PowerDown\r\n"));

    // exit if FMD wasn't opened
    if(!s_hNand)
		return;
    
    //  Only enable during NAND read/write/erase operations
    NAND_Enable(s_hNand, 1);

    // Wait for NAND
    WaitForReadyStatus(s_hNand);   

    // Write the reset command
    NAND_SendCommand(s_hNand, NAND_CMD_RESET);

    // Wait for NAND
    WaitForReadyStatus(s_hNand);

    //  Only enable during NAND read/write/erase operations
    NAND_Enable(s_hNand, 0);
}

//------------------------------------------------------------------------------
//
//  Function:  FMD_OEMIoControl
//
BOOL FMD_OEMIoControl(DWORD code, UCHAR *pInBuffer, DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize)
{
    UNREFERENCED_PARAMETER(code);

    UNREFERENCED_PARAMETER(pInBuffer);
    UNREFERENCED_PARAMETER(inSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

	return 0;
}