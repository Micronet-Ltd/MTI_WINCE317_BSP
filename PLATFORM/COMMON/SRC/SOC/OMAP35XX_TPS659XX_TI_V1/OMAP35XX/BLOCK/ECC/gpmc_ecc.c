/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/

//-----------------------------------------------------------------------------
//
//  File:  gpmc_ecc.c
//
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <oal.h>
#include <omap35xx.h>
#include <gpmc_ecc.h>


#define DATA_BLOCK_LEN  512 // Ecc calculations is done in multiple of 512 B.

#define ECC_BUFF_LEN    3   // 24-bit ECC

// Count of bits set in xor determines the type of error
#define NO_ERRORS                0
#define ECC_ERROR                1
#define CORRECTABLE_ERROR       12
#define ERASED_SECTOR           24



//-----------------------------------------------------------------------------
// Function:       CountNumberOfOnes()
//
// Description:    Counts the number of bits that are "1" in a byte.
//
// Returns:        Number of bits that are "1".
//
static __inline 
UCHAR 
CountNumberOfOnes(
    DWORD num
    )
{
    UCHAR count = 0;
    while(num)
        {
        num=num&(num-1);
        count++;
        }

    return count;
}


//-----------------------------------------------------------------------------
VOID
ECC_Init(
    OMAP_GPMC_REGS *pGpmcRegs,
    UINT configMask
    )
{
    //  Configure ECC calculator engine for NAND part
    OUTREG32(&pGpmcRegs->GPMC_ECC_CONFIG, configMask);

    //  Set ECC field sizes
    OUTREG32(&pGpmcRegs->GPMC_ECC_SIZE_CONFIG, 0x3fcff000);

    //  Select result reg 1 and clear results
    OUTREG32(&pGpmcRegs->GPMC_ECC_CONTROL, GPMC_ECC_CONTROL_CLEAR);
    OUTREG32(&pGpmcRegs->GPMC_ECC_CONTROL, GPMC_ECC_CONTROL_POINTER1);

    //  Enable ECC engine
    SETREG32(&pGpmcRegs->GPMC_ECC_CONFIG, GPMC_ECC_CONFIG_ENABLE);
}

//-----------------------------------------------------------------------------
VOID
ECC_Result(
    OMAP_GPMC_REGS *pGpmcRegs,
    BYTE *pEcc,
    int size
    )
{
    UINT    regIndex = 0;
    UINT8   eccIndex;
    UINT32  regVal;

    // the ecc engine is setup encode 512 bytes at a time
    // so reading a sectore of 2048 bytes will require 4 sets of encoded
    // groups
    
    for (eccIndex=0; eccIndex < size;)
        {
        regVal = INREG32(((UINT32*)&pGpmcRegs->GPMC_ECC1_RESULT) + regIndex);

        // ECC-x[0] where x is from A-D
        pEcc[eccIndex++] = ECC_P1_128_E(regVal);

        // ECC-x[1] where x is from A-D
        pEcc[eccIndex++] = ECC_P1_128_O(regVal);

        // ECC-x[2] where x is from A-D
        pEcc[eccIndex++] = ECC_P512_2048_E(regVal)|ECC_P512_2048_O(regVal)<<4;

        // read next ecc register
        regIndex++;
        }

    return;
}

//-----------------------------------------------------------------------------
VOID
ECC_Reset(
    OMAP_GPMC_REGS *pGpmcRegs
    )
{
    //  Disable ECC engine
    CLRREG32(&pGpmcRegs->GPMC_ECC_CONFIG, GPMC_ECC_CONFIG_ENABLE);
}

//-----------------------------------------------------------------------------
// Function:    ECC_CorrectData()
//
// Description: Call to correct errors (if possible) in the specified data.
//
// Notes:       This implemention uses 3 bytes of ECC info for every 512 bytes
//              of data.  Furthermore, only single bit errors can be corrected
//              for every 512 bytes of data.
// 
//              Based off of algorithm described at www.samsung.com
//              http://www.samsung.com/global/business/semiconductor/products/flash/FlashApplicationNote.html
//
// Returns:     Boolean indicating if the data was corrected.
//
BOOL ECC_CorrectData(
    OMAP_GPMC_REGS *pGpmcRegs,  // GPMC register
    BYTE *pData,                // Data buffer
    int sizeData,               // Count of bytes in data buffer
    BYTE const *pEccOld,        // Pointer to the ECC on flash
    BYTE const *pEccNew         // Pointer to the ECC the caller calculated
    )
{
    BOOL rc = FALSE;
    int   i;
    UCHAR numOnes;
    DWORD ECCxor[ECC_BUFF_LEN];
    UCHAR mask;
    DWORD byteLocation;
    DWORD bitLocation;
    UCHAR count;
    BOOL  bCorrect;

    //  ECC calculated for every 512 bytes of data
    UINT numberOfSectors = (sizeData/DATA_BLOCK_LEN);

    UNREFERENCED_PARAMETER(pGpmcRegs);


    //----- 1. Check passed parameters -----
    for(count=0; count < numberOfSectors; count++ )
        {
        //----- 2. XOR the existing ECC info with the new ECC info -----
        for(i = 0; i < ECC_BUFF_LEN; i++)
            {
                ECCxor[i] = *(pEccNew+i) ^ *(pEccOld+i);
            }

        //----- 3. Determine if this is a single-bit error that can be corrected -----
        //         NOTE: The total number of bits equal to '1' in the XORed Hamming
        //               Codes determines if the error can be corrected.
        numOnes = 0;
        for(i = 0; i < ECC_BUFF_LEN; i++)
            {
            numOnes += CountNumberOfOnes(ECCxor[i]);
            }

        switch( numOnes )
            {
            case NO_ERRORS:
            case ECC_ERROR:
            case ERASED_SECTOR:
                //  No error in the data
                bCorrect = FALSE;
                break;
            
            case CORRECTABLE_ERROR:
                //  Single bit error; correctable
                bCorrect = TRUE;
                break;
            
            default:
                //  More than 1 bit error
                rc = FALSE;
                goto cleanUp;
                break;
            }
            
            
        //----- 4. Compute the location of the single-bit error -----
        if( bCorrect )
            {
            // Note: This is how the ECC is layed out in the ECC buffers.
            // ECCxor[0] = P128e  P64e   P32e   P16e   P8e    P4e    P2e    P1e
            // ECCxor[1] = P128o  P64o   P32o   P16o   P8o    P4o    P2o    P1o
            // ECCxor[2] = P2048o P1024o P512o  P256o  P2048e P1024e P512e  P256e

            // Combine the 'o' xor'ed values to get row and column
            byteLocation = ((ECCxor[2] & 0xF0) << 1) | (ECCxor[1] >> 3);
            bitLocation = ECCxor[1] & 0x7;

            //----- 5. Correct the single-bit error (set the bit to its complementary value) -----
            mask = (0x01 << bitLocation);
            if(pData[byteLocation] & mask)
                {
                pData[byteLocation] &= ~mask;       // 0->1 error, set bit to 0
                }
            else
                {
                pData[byteLocation] |= mask;        // 1->0 error, set bit to 1
                }
            }

            
        //  Advance pointers
        pEccOld += ECC_BUFF_LEN;                // Pointer to the ECC on flash
        pEccNew += ECC_BUFF_LEN;                // Pointer to the ECC the caller calculated
        pData += DATA_BLOCK_LEN;
        }

    rc = TRUE;
    
cleanUp:
    return rc;
}



//-----------------------------------------------------------------------------
