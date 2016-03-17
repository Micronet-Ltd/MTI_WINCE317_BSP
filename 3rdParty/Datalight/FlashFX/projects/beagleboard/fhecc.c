/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2010 Datalight, Inc.
                       All Rights Reserved Worldwide.

  Datalight, Incorporated is a Washington State corporation located at:

        21520 30th Dr SE, Suite 110,      Tel:  425-951-8086
        Bothell, WA  98021                Fax:  425-951-8094
        USA                               Web:  http://www.datalight.com

  This software, including without limitation all source code and documen-
  tation, is the confidential, trade secret property of Datalight, Inc.
  and is protected under the copyright laws of the United States and other
  jurisdictions.  The software may be subject to one or more of these US
  patents: US#5860082, US#6260156.  Patents may be pending.

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

    This module contains the Windows CE OS Layer implementation of ECC 
    handling for the Beagleboard.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhecc.c $
    Revision 1.4  2010/01/07 03:17:48Z  garyp
    Updated debug code -- no functional changes.
    Revision 1.3  2009/12/11 03:12:03Z  garyp
    Updated to use an NTMHOOKHANDLE rather than a PNANDCTL.
    Revision 1.2  2009/10/07 17:30:42Z  garyp
    Fixed to build cleanly with hardware ECC turned on.
    Revision 1.1  2009/10/06 21:41:52Z  garyp
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fxnandapi.h>
#include <ecc.h>
#include "project.h"

#if USE_HARDWARE_ECC
#include <hardware/hwomap35x.h>

FFXIOSTATUS CorrectPage(D_BUFFER *pData, D_BUFFER *pSpare, const D_BUFFER *pabECC, const FFXNANDCHIP *pChipInfo);

#endif


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
        D_UINT32    ulConfigVal;

        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = OMAP35X_GPMC_ECC_CONFIG_16BIT; 
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

/*          FFXPRINTF(1, ("HWECC=%lX\n", ulConfigVal)); */

            pabECC[nn++] = (D_UINT8)ulConfigVal;
            pabECC[nn++] = (D_UINT8)(((ulConfigVal >> 8) & 0xF) | ((ulConfigVal & 0xF0000) >> 12));
            pabECC[nn++] = (D_UINT8)(ulConfigVal >> 20);

            /*  Since we are doing ECCs per every 512 bytes, just fill in
                the second half with 0xFFs.  Of course the logic in the
                "correct" function must expect this.
            */                
            pabECC[nn++] = 0xFF;
            pabECC[nn++] = 0xFF;
            pabECC[nn++] = 0xFF;

            nRegisterIndex++;
            nLength -= DATA_BYTES_PER_ECC*2;
        }

        ulConfigVal = OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase);
        ulConfigVal &= ~OMAP35X_GPMC_ECC_CONFIG_ENABLE;
        OMAP35X_GPMC_ECC_CONFIG(hNtmHook->pulGPMCBase) = ulConfigVal;
    }

  #else

    /* NOTE! If using the format of the ECC data is different for the HW 
             and SW solutions.  If you are switching back and forth, the
             media will <NOT> be compatible.
    */

    (void)hNtmHook;

    while(nLength)
    {
        FfxEccCalculate(pData, pabECC);

        nLength -= DATA_BYTES_PER_ECC;
        pData   += DATA_BYTES_PER_ECC;
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
        pData     - A pointer to page of data read from the flash.
        pSpare    - A pointer to spare area associated with the
                    given page data.
        pabECC    - A pointer to an array of bytes containing ECC
                    values for a page.
        pChipInfo - A pointer to the FFXNANDCHIP structure
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
    DclAssert(hNtmHook); 
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(pChipInfo);

    (void)hNtmHook;
    
  #if USE_HARDWARE_ECC
    return CorrectPage(pData, pSpare, pabECC, pChipInfo);       
  #else
    return FfxNtmHelpCorrectPage(pData, pSpare, pabECC, pChipInfo);      
  #endif
}


#if USE_HARDWARE_ECC

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
static FFXIOSTATUS CorrectPage(
    D_BUFFER           *pData,
    D_BUFFER           *pSpare,
    const D_BUFFER     *pabECC,
    const FFXNANDCHIP  *pChipInfo) 
{
    FFXIOSTATUS         ioStat = INITIAL_PAGEIO_STATUS(FFXSTAT_FIMUNCORRECTABLEDATA);
    D_UINT16            uSpareSize;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 3, TRACEINDENT),
        "FfxHookOmap:CorrectPage()\n"));

    DclProfilerEnter("FfxHookOmap:CorrectPage", 0, 0);

    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(pChipInfo);

    uSpareSize = pChipInfo->pChipClass->uSpareSize;

    DclAssert(uSpareSize % NAND512_SPARE == 0);

    /*  Check the data block status, if it's clean, then the block is good
    */
    if(ISUNWRITTEN(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        /*  TODO: For consistency, if we are using 2K pages, we really
            should be validating the bFlags fields for the full array of
            LEGACYNANDSPARE structures, as we do in the next clause.
        */
        ioStat.ulCount = 1;
        ioStat.ffxStat = FFXSTAT_SUCCESS;
    }
    else if(ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
    {
        D_UINT16    uCount = 0;
        D_BOOL      fUncorrectableError = FALSE;

        /*  Note that it is possible for a single-bit error to corrupt
            the status value.  Double-check it by testing whether it
            differs by a single bit.
        */
        while(uSpareSize)
        {
            /*  If dealing with multiple ECCs in a 2K page, all the
                block status values must be identical (except for the
                possible single-bit error).
            */
            if(!ISWRITTENWITHECC(pSpare[NSOFFSETZERO_FLAGS_OFFSET]))
            {
                fUncorrectableError = TRUE;
            }

            if(DclMemCmp(&pSpare[NSOFFSETZERO_ECC1_OFFSET], pabECC, BYTES_PER_ECC) != 0)
            {
                D_UINT32        ulOldECC;
                D_UINT32        ulNewECC;
                DCLECCRESULT    iResult;

                ulOldECC = DclEccOmap35xToCanonical(&pSpare[NSOFFSETZERO_ECC1_OFFSET]);
                ulNewECC = DclEccOmap35xToCanonical(pabECC);

                /*  The mem compare above already proved that the two ECCs
                    do not match, so they better still not match...
                */    
                DclAssert(ulOldECC != ulNewECC);
  
                iResult = DclEccCorrect(pData, DATA_BYTES_PER_ECC*2, &ulOldECC, ulNewECC);

                if(iResult == DCLECC_UNCORRECTABLE)
                {
                    fUncorrectableError = TRUE;
                }
                else if(iResult == DCLECC_DATACORRECTED)
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;
                }
                else if(iResult == DCLECC_ECCCORRECTED)
                {
                    ioStat.op.ulPageStatus |= PAGESTATUS_DATACORRECTED;

                    /*  If the ECC was corrected, then "old" ECC should have
                        been set equal to "new" ECC.
                    */                        
                    DclAssert(ulOldECC == ulNewECC);

                    DclEccCanonicalToOmap35x(ulOldECC, (D_BUFFER*)&pSpare[NSOFFSETZERO_ECC1_OFFSET]);

                    /*  Debugging logic to ensure the conversion functions
                        are working properly.
                    */    
                    DclAssert(DclEccOmap35xToCanonical(&pSpare[NSOFFSETZERO_ECC1_OFFSET]) == ulOldECC);
                }
                else
                {
                    DclError();
                }
            }

            uCount++;
            pData += DATA_BYTES_PER_ECC*2;

            /*  Even if we found errors, attempt to correct errors in
                any subsequent areas.
            */
            uSpareSize  -= NAND512_SPARE;
            pSpare      += NAND512_SPARE;
            pabECC      += BYTES_PER_ECC*2;
        }

        /*  Return FFXSTAT_SUCCESS if none of the segments had uncorrectable
            errors.
        */
        if(!fUncorrectableError)
        { 
            if(ioStat.op.ulPageStatus & PAGESTATUS_DATACORRECTED)
                ioStat.ffxStat = FFXSTAT_FIMCORRECTABLEDATA;
            else
                ioStat.ffxStat = FFXSTAT_SUCCESS;  
        }
      #if D_DEBUG > 2
        else
        {
            DclHexDump("Failing page:\n", HEXDUMP_UINT8, 16, uCount * DATA_BYTES_PER_ECC,
                       pData - (uCount * DATA_BYTES_PER_ECC));
            DclHexDump("Spare:\n", HEXDUMP_UINT8, 16, uCount * 8,
                       pSpare - ((uCount / 2) * NAND512_SPARE));
        }
      #endif
    }
    else
    {
        /*  We should never be checking for ECC on a page that is written
            without ECC.  BBM writes pages without ECC.  This condition can
            happen for other pages if more than a single bit error occurs in
            the flags field.
        */
        FFXPRINTF(2, ("Attempting to correct a page not written with ECC\n"));
    }

    DclProfilerLeave(0UL);

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTM, 2, TRACEUNDENT),
        "FfxHookOmap:CorrectPage() returning %s\n", FfxDecodeIOStatus(&ioStat)));

    return ioStat;
}


#endif  /* USE_HARDWARE_ECC */

#endif  /* FFXCONF_NANDSUPPORT */

