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

    This module contains the default implementations for abstracting ECC
    functionality.

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    These functions provide a default implementation that uses software ECCs,
    which serves as a template for supporting hardware ECC generation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhecc.c $
    Revision 1.13  2009/12/11 03:06:07Z  garyp
    Updated to use an NTMHOOKHANDLE rather than a PNANDCTL.
    Revision 1.12  2009/10/20 14:05:56Z  garyp
    Removed some incorrect asserts.
    Revision 1.11  2009/10/06 21:21:50Z  garyp
    Removed an incorrect "const" modifier.
    Revision 1.10  2009/10/06 18:36:32Z  garyp
    Re-abstracted these functions to allow more flexibility with different
    hardware ECC models.  The ECC correction code path now flows through this
    module.  Use a NANDCTL context rather than a device context to facilitate
    shared information at the project hook level.  Eliminated use of the FFXECC
    structure.

    Revision 1.9  2009/04/08 15:00:22Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.8  2009/02/18 21:41:02Z  glenns
    Added code to prevent compiler warnings about unused formal parameters.
    Revision 1.7  2008/01/13 07:29:17Z  keithg
    Function header updates to support autodoc.
    Revision 1.6  2007/11/03 23:50:01Z  Garyp
    Updated to use the standard module header.
    Revision 1.5  2007/10/14 21:12:31Z  Garyp
    Modified to use a renamed function.
    Revision 1.4  2006/02/17 22:16:09Z  Garyp
    Modified the ECC hook functions to take an hDev parameter.
    Revision 1.3  2006/02/10 02:15:30Z  Garyp
    Updated to use device/disk handles rather than the EXTMEDIAINFO structure.
    Revision 1.2  2006/02/09 19:05:52Z  Pauli
    Updated comments.
    Revision 1.1  2005/11/07 17:46:08Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>

#if FFXCONF_NANDSUPPORT

#include <fxnandapi.h>
#include <ecc.h>


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
    DclAssert(pData);
    DclAssert(pabECC);
    DclAssert(nLength);
    DclAssert(nLength % DATA_BYTES_PER_ECC == 0);

    (void)hNtmHook;
    (void)pData;
    (void)nLength;
    (void)pabECC;
    (void)eccMode;
  
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
    DclAssert(pData);
    DclAssert(pabECC);
    DclAssert(nLength);
    DclAssert(nLength % DATA_BYTES_PER_ECC == 0);
    
    (void)hNtmHook;
    (void)eccMode;

    while(nLength)
    {
        FfxEccCalculate(pData, pabECC);

        nLength -= DATA_BYTES_PER_ECC;
        pData   += DATA_BYTES_PER_ECC;
        pabECC  += BYTES_PER_ECC;
    }
 
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
    DclAssert(pData);
    DclAssert(pSpare);
    DclAssert(pabECC);
    DclAssert(pChipInfo);

    (void)hNtmHook;

    /*  Default to calling the standard correction function for known
        spare area formats.
    */                
    return FfxNtmHelpCorrectPage(pData, pSpare, pabECC, pChipInfo);      
}



#endif  /* FFXCONF_NANDSUPPORT */

