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

    This module contains the Windows CE OS Layer default implementations for
    abstracting ECC functionality.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhecc.c $
    Revision 1.3  2010/07/06 01:35:56Z  garyp
    Updated to work with multiple chip selects.
    Revision 1.2  2010/06/25 03:11:02Z  garyp
    Debug code updated -- no functional changes.
    Revision 1.1  2009/12/19 01:50:22Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fxnandapi.h>    
#include <ecc.h>
#include "project.h"

#if USE_HARDWARE_ECC
#include <hardware/hwomap35x.h>
#endif

static D_UINT32    CSSTToCanonical(const D_BUFFER *pabECC);
static void        CanonicalToCSST(D_UINT32 ulECC, D_BUFFER *pabECC);
static FFXIOSTATUS CorrectPage(D_UINT16 uSpareSize, D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC);




/*-------------------------------------------------------------------
    Public: FfxHookEccCalcStart()

    Prepare for the start of ECC calculation before a data transfer.

    The mode argument selects behavior appropriate to read or 
    write operations as noted below.

    Depending on the implementation of the ECC calculation (software
    or various types of hardware) this may cause ECC calculations to
    be performed on the buffer after a read (when FfxHookEccCalcRead()
    is called), or started on the buffer contents before a write
    (useful if the hardware can calculate the ECC concurrently with
    the write).  For hardware ECC this function may prepare the 
    hardware for the data transfer.  For software ECC, this function
    may do nothing.

    Parameters:
        hNtmHook  - The NTM Hook handle to use.  This may be NULL
                    for some NTMs.
        pData     - A pointer to the data for which the ECCs are
                    to be calculated.
        nLength   - The data length.
        pabECC    - A pointer to an array of bytes containing ECC
                    values for a page.
        eccMode   - ECC_MODE_READ or ECC_MODE_WRITE.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookEccCalcStart(
    NTMHOOKHANDLE   hNtmHook,
    const D_BUFFER *pData,
    size_t          nLength,
    D_BUFFER       *pabECC,
    ECCMODE         eccMode)
{
    DclAssert(hNtmHook); 
    DclAssert(pData);
    DclAssert(pabECC);
    DclAssert(nLength);
    DclAssert(nLength % DATA_BYTES_PER_ECC == 0);

    (void)hNtmHook;
    (void)pData;
    (void)nLength;
    (void)pabECC;
    (void)eccMode;

  #if USE_HARDWARE_ECC
    {
        D_UINT32    ulConfigVal = OMAP35X_GPMC_ECC_CONFIG_CHIPSELECT(hNtmHook->nChip);

      #if NAND_DATA_WIDTH_BYTES == 1
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = 0 | ulConfigVal;  
      #else
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_ECC_CONFIG_16BIT | ulConfigVal;  
      #endif
      
        OMAP35X_GPMC_ECC_CONTROL(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_ECC_CONTROL_CLEAR;

        ulConfigVal = OMAP35X_GPMC_ECC_CONTROL(hNtmHook->pulGPMCBase);
        ulConfigVal |= OMAP35X_GPMC_ECC_CONTROL_POINTER(1);
        OMAP35X_GPMC_ECC_CONTROL(hNtmHook->pulGPMCBase) = ulConfigVal;

        ulConfigVal = OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase);
        ulConfigVal |= OMAP35X_GPMC_ECC_CONFIG_ENABLE;
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = ulConfigVal;
    }
  #endif
  
    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookEccCalcRead()

    Obtain the results of the ECC calculation.

    For software ECC, this function typically calculates the ECCs.
    For some hardware ECC generators this function may shut down 
    the ECC calculation hardware.

    Parameters:
        hNtmHook  - The NTM Hook handle to use.  This may be NULL
                    for some NTMs.
        pData     - A pointer to the data on which the ECCs are
                    to be calculated.
        nLength   - The data length
        pabECC    - A pointer to an array of bytes containing ECC
                    values for a page.
        eccMode   - ECC_MODE_READ or ECC_MODE_WRITE.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookEccCalcRead(
    NTMHOOKHANDLE   hNtmHook,
    const D_BUFFER *pData,
    size_t          nLength,
    D_BUFFER       *pabECC,
    ECCMODE         eccMode)
{   
    DclAssert(hNtmHook); 
    DclAssert(pData);
    DclAssert(pabECC);
    DclAssert(nLength);
    DclAssert(nLength % DATA_BYTES_PER_ECC == 0);

    (void)eccMode;

  #if USE_HARDWARE_ECC
    {
        D_UINT32    ulConfigVal;
        unsigned    nRegisterIndex = 0;
        unsigned    nn = 0;
        
        while(nLength)
        {
            /*  Read from the next Hardware ECC Result Register
            */
            ulConfigVal = OMAP35X_GPMC_ECC1_RESULT(((D_UINT32*)hNtmHook->pulGPMCBase) + nRegisterIndex);

            pabECC[nn+0] = (D_UINT8)ulConfigVal;
            pabECC[nn+1] = (D_UINT8)(ulConfigVal >> 16);
            pabECC[nn+2] = (D_UINT8)(((ulConfigVal >> 8) & 0x0F) | ((ulConfigVal >> 20) & 0xF0));

          #if D_DEBUG > 2
            /*  This code validates that the software ECC calculator
                generates the same ECC values as the HW ECC calculator.
            */                    
            {
                D_BUFFER    abTempECC[BYTES_PER_ECC];
                
                CanonicalToCSST(DclEccCalculate(pData, DATA_BYTES_PER_ECC*2), abTempECC);

                FFXPRINTF(1, ("HW=%lX %02x %02x %02x -:- %02x %02x %02x Can=%lX\n", 
                    ulConfigVal,
                    pabECC[nn+0], pabECC[nn+1], pabECC[nn+2], 
                    abTempECC[0], abTempECC[1], abTempECC[2], 
                    DclEccCalculate(pData, DATA_BYTES_PER_ECC*2)));

                DclAssert(abTempECC[0] == pabECC[nn+0]);
                DclAssert(abTempECC[1] == pabECC[nn+1]);
                DclAssert(abTempECC[2] == pabECC[nn+2]);

                pData += DATA_BYTES_PER_ECC*2;
            }
          #endif

            nn += BYTES_PER_ECC;
            nRegisterIndex++;
            nLength -= DATA_BYTES_PER_ECC*2;
        }

        ulConfigVal = OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase);
        ulConfigVal &= ~OMAP35X_GPMC_ECC_CONFIG_ENABLE;
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = ulConfigVal;
    }

  #else

    (void)hNtmHook;

    while(nLength)
    {	   
        D_UINT32    ulECC;

        ulECC = DclEccCalculate(pData, DATA_BYTES_PER_ECC*2);

        CanonicalToCSST(ulECC, pabECC);

        nLength -= DATA_BYTES_PER_ECC*2;
        pData   += DATA_BYTES_PER_ECC*2;
        pabECC  += BYTES_PER_ECC;
    }

  #endif 
 
    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookEccCorrectPage()

    Examine the original ECC and a newly calculated ECC, and perform
    any necessary and possible corrections on a page.  This function
    is typically used in concert with FfxHookEccCalcStart() and 
    FfxHookEccCalcRead(), and allows project-specific, custom ECC
    handling to be implemented.

    Note that the length of the pabECC byte array may vary depending
    on the implementation and strength of the ECC used in the ECC
    calculation functions.

    Parameters:
        hNtmHook  - The NTM Hook handle to use.  This may be NULL
                    for some NTMs.
        pData      - A pointer to page of data read from the flash.
        pSpare     - A pointer to spare area associated with the
                     given page data.
        pabECC     - A pointer to an array of bytes containing ECC
                     values for a page. 
        pChipInfo  - A pointer to the FFXNANDCHIP structure
                     describing the chip being used.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.

        ffxStat will be FFXSTAT_FIMCORRECTABLEDATA if one or more
        bit errors were corrected with ECC.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
FFXIOSTATUS FfxHookEccCorrectPage(
    NTMHOOKHANDLE       hNtmHook,
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC,
    const FFXNANDCHIP  *pChipInfo) 
{   	
    DclAssert(DCLISALIGNED((D_UINTPTR)pSpare, sizeof(D_UINT32)));
    DclAssert(pChipInfo->pChipClass->uSpareSize == 64);

    return CorrectPage(pChipInfo->pChipClass->uSpareSize, pData, pSpare, pabECC);    
}


/*-------------------------------------------------------------------
    Local: CorrectPage()

    This function examines an "Offset Zero" style page, checking
    the page flags and potentially the ECC to determine if the
    page is OK.  If a failure is found, the ECC will attempt to
    correct the problem and return a status which will indicate
    the state of the data.

    Parameters:
        pData      - A pointer to page of data read from the flash.
        pSpare     - A pointer to spare area associated with the
                     given data sector.
        pabECC     - A pointer to an array of bytes containing ECC
                     values for a page.

    Return Value:
        Returns an FFXIOSTATUS structure indicating the state of
        the operation.

        ffxStat will be FFXSTAT_FIMCORRECTABLEDATA if one or more
        bit errors were corrected with ECC.

        If ffxStat == FFXSTAT_FIMUNCORRECTABLEDATA, the ulCount
        field will indicate the number of correct segments that
        were found.
-------------------------------------------------------------------*/
static FFXIOSTATUS CorrectPage(
    D_UINT16            uSpareSize,
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC)
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxHookEcc:CorrectPage() SpareSize=%U\n", uSpareSize));

    DclProfilerEnter("FfxHookEcc:CorrectPage", 0, 0);

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(uSpareSize);
    DclAssert(uSpareSize % NAND512_SPARE == 0);
    
    if(ISWRITTENWITHECC(pSpare[FLAGS_OFFSET]) || !FfxNtmHelpIsRangeErased1Bit(&pSpare[ECC_OFFSET], ECC_LEN))
    {
        unsigned        nCount = 0;
        D_BOOL          fUncorrectableError = FALSE;
        unsigned        nIndex = ECC_OFFSET;
        D_UINT32        ulOldECC;
        D_UINT32        ulNewECC;
        DCLECCRESULT    iResult;

        while(uSpareSize)
        {
            ulOldECC = CSSTToCanonical(&pSpare[nIndex]);
            ulNewECC = CSSTToCanonical(pabECC);

            iResult = DclEccCorrect(pData, DATA_BYTES_PER_ECC*2, &ulOldECC, ulNewECC);
            if(iResult == DCLECC_UNCORRECTABLE)
            {
                fUncorrectableError = TRUE;
            }
            else
            {
                if(iResult == DCLECC_DATACORRECTED)
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                }
                else if(iResult == DCLECC_ECCCORRECTED)
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                    DclMemCpy(&pSpare[nIndex], pabECC, BYTES_PER_ECC);
                }
                else
                {
                    DclAssert(iResult == DCLECC_VALID);
                }

                /*  Increment the count so long as we have NOT encountered an
                    uncorrectable error.
                */
                ioStat.ulCount++;
            }

            nCount++;
            pData   += DATA_BYTES_PER_ECC*2;
            pabECC  += BYTES_PER_ECC;
            nIndex  += BYTES_PER_ECC;

            /*  Even if we found errors, attempt to correct errors in
                any subsequent areas.
            */
            uSpareSize  -= NAND512_SPARE;
        }

        /*  Return FFXSTAT_SUCCESS if none of the segments had uncorrectable
            errors.
        */
        if(!fUncorrectableError)
        {
            ioStat.ffxStat = FFXSTAT_SUCCESS;
        }
        else
        {
            DCLPRINTF(1, ("FfxHookEcc:CorrectPage() Uncorrectable ECC error in segment %u\n", nCount));
            
          #if D_DEBUG > 2
            DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, nCount * DATA_BYTES_PER_ECC,
                       pData - (nCount * DATA_BYTES_PER_ECC));
            DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, nCount * 8,
                       pSpare - ((nCount / 2) * NAND512_SPARE));
          #endif
        }
    }
    else
    {
        DclAssert(ISUNWRITTEN(pSpare[FLAGS_OFFSET]));
        ioStat.ulCount = 1;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxHookEcc:CorrectPage() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


/*-------------------------------------------------------------------
    Public: CSSTToCanonical()

    Convert an ECC in the format provided by the OMAP 3530 hardware,
    into the Datalight canonical 32-bit format.  See dlecc.c and the
    3530 specification for details.

    Note that the hardware ECC for data which is all zeros or all
    ones is all zeros, which is opposite of what the Datalight 
    canonical format uses.

    Parameters:
        pabECC     - A pointer to the three ECC bytes provided by
                     the hardware.

    Return Value:
        Returns the ECC in the Datalight canonical format.
-------------------------------------------------------------------*/
static D_UINT32 CSSTToCanonical(
    const D_BUFFER *pabECC)
{
    D_UINT32        ulEven;
    D_UINT32        ulOdd;
    D_UINT32        ulECC = 0;
    unsigned        nIndex = 0;

    DclAssertReadPtr(pabECC, ECC_BYTES);

    /*  Get the even and odd parity bits out of the 3 bytes
    */
    ulEven = pabECC[0] | (((D_UINT32)(pabECC[2] & 0x0F)) << 8);
    ulOdd =  pabECC[1] | (((D_UINT32)(pabECC[2] & 0xF0)) << 4);

    FFXPRINTF(3, ("Even %lX Odd %lX\n", ulEven, ulOdd)); 

    /*  The bits are all inverted...
    */
    ulEven = ~ulEven;
    ulOdd = ~ulOdd;

    while(nIndex < 24)
    {
        ulECC |= (ulEven & 1) << nIndex;
        ulECC |= ((ulOdd & 1) << (nIndex+1));
        ulEven >>= 1;
        ulOdd >>= 1;
        nIndex += 2;
    }

    FFXPRINTF(3, ("CSSTToCanonical: %02x %02x %02x can=%lX \n", pabECC[0], pabECC[1], pabECC[2], ulECC));

    return ulECC;
}


/*-------------------------------------------------------------------
    Public: CanonicalToCSST()

    Convert an ECC in the Datalight canonical format to the format
    used by the OMAP 3530 hardware.  See dlecc.c and the 3530
    specification for details.

    Parameters:
        ulECC       - The ECC in Datalight canonical format.
        pabECC      - A pointer to a three byte buffer to receive
                      the results.

    Return Value:
        None.
-------------------------------------------------------------------*/
static void CanonicalToCSST(
    D_UINT32        ulECC,
    D_BUFFER       *pabECC)
{
    D_UINT32        ulEven = 0;
    D_UINT32        ulOdd = 0;
    unsigned        nIndex = 0;
    D_UINT32 ulOrig = ulECC;

    DclAssertWritePtr(pabECC, ECC_BYTES);

    /*  Get the even and odd parity bits out of our interleaved format
        and into separate variables.
    */                
    while(nIndex < 12)
    {
        ulEven |= (ulECC & 1) << nIndex;
        ulECC >>= 1;
        ulOdd |= (ulECC & 1) << nIndex;
        ulECC >>= 1;
        nIndex++;
    }

    /*  The bits are all inverted...
    */
    ulEven = ~ulEven;
    ulOdd = ~ulOdd;

    /*  Get the right bits into the correct bytes.
    */
    pabECC[0] = (D_BYTE)ulEven;
    pabECC[1] = (D_BYTE)ulOdd;
    pabECC[2] = (D_BYTE)(((ulOdd >> 4) & 0xF0) | ((ulEven >> 8) & 0x0F));

    FFXPRINTF(3, ("CanonicalToCSST: can=%lX %02x %02x %02x\n", ulOrig, pabECC[0], pabECC[1], pabECC[2]));

    DclAssert(CSSTToCanonical(pabECC) == ulOrig);

    return;
}




 
#endif  /* FFXCONF_NANDSUPPORT */

