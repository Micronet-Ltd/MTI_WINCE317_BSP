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

    This file provides the Error Policy Hook needed with the FlashFX Error
    Manager. This file is the one that would be modified for individual
    projects to customize error policy.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: errpolicyhook.c $
    Revision 1.6  2009/04/15 23:42:35Z  garyp
    Documentation updates and general code cleanup -- no functional
    changes.
    Revision 1.5  2009/04/01 20:13:59Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.4  2009/02/27 23:28:29Z  billr
    Some work on bug 2454: default Error Manager policy recommends
    retiring blocks with read errors
    Revision 1.3  2009/01/27 23:00:54Z  glenns
    - Reorganized header file inclusion to relieve some obfuscation difficulty.
    Revision 1.2  2009/01/20 23:34:51Z  glenns
    - Added revision history block.
    - Gave timeout errors their own policy recommendation.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <errmanager.h>

/*-------------------------------------------------------------------
    Public: FfxErrorMgrPolicyHook()

    The Error Policy Hook procedure. It is the default implementation
    of the FlashFX Error Policy. It adheres to the following basic
    rules:

    1. It supports only Flash-specific error conditions.  Any error
       outside this set of conditions will cause an Abandon
       Operations code to be returned.
    2. Any flash error other than a correctable bit error will
       stimulate a recommendation to retire the block.
    3. A corrected error whose number of corrected bits is equal to
       the maximum number of correctable errors will result in a
       recommendation of Scrub Now.
    4. A corrected error whose number of corrected bits is one less
       than the maximum number of correctable errors will result in
       a recommendation of Relocate Page.
    5. Corrected errors with lower numbers of corrected bits will
       result in a policy recommendation of No Action.

    Parameters:
        error   - Indication of which flash error has taken place
        pIoStat - A pointer to an FFXIOSTATUS structure into which
                  the number of corrected bits has been encoded
                  (for correctable errors)
        hFimDev - Handle for a FIM Device

    Return Value:
        An element of the FFX_ERRORPOLICY, indicating a recommended
        course of action.
-------------------------------------------------------------------*/
FFX_ERRORPOLICY FfxErrorMgrPolicyHook(
    FFX_FLASHERROR  error,
    FFXIOSTATUS    *pIoStat,
    FFXFIMDEVHANDLE hFimDev)
{
    switch (error)
    {
        /*  Erase and Write failures result from status indications
            coming from the flash chip.  NAND chip specifications
            uniformly recommend retiring the erase block when the
            chip reports error status on a program or erase.
        */
        case FFXERRTYPE_WRITE_FAILURE:
        case FFXERRTYPE_ERASE_FAILURE:
            return FFXERRPOLICY_RETIREBLOCK;

        /*  Timeout errors are indicative of misdesigned or broken
            hardware.  Recommendations in this case are not meaningful.
            Uncorrectable errors come from ECC verifications.
        */
        case FFXERRTYPE_TIMEOUT:
        case FFXERRTYPE_UNCORRECTABLE:
        case FFXERRTYPE_OF_NO_INTEREST:
            return FFXERRPOLICY_NO_RECOMMENDATION;

        /*  Correctable errors require supplementary processing.
        */
        case FFXERRTYPE_CORRECTABLE:
            {
                D_UINT32 ulCorrectedBits;
                D_UINT32 ulMaxCorrectable;

                ulMaxCorrectable = FfxErrMgrGetMaxCorrectableError(hFimDev);
                ulCorrectedBits = FfxErrMgrDecodeCorrectedBits(pIoStat);

                /*  This is the algorithm described above. Modify this code
                    if you wish a different algorithm for correctable errors.

                    Note that it is possible for some EDC algorithms to
                    correct more bits in some circumstances than they are
                    rated for. Take that into account.
                */
                if (ulCorrectedBits > ulMaxCorrectable)
                    return FFXERRPOLICY_SCRUBNOW;
                switch (ulMaxCorrectable - ulCorrectedBits)
                {
                    case 0:
                        return FFXERRPOLICY_SCRUBNOW;
                    case 1:
                        return FFXERRPOLICY_RELOCATEPAGE;
                    default:
                        return FFXERRPOLICY_NO_ACTION;
                }
            }

        /*  This switch should have a case for every possible value of
            FFX_FLASHERROR, so "default:" is an error.
        */
        default:
            DclError();
            /*  Fall through */

        /*  Readback/Verify errors occur when data read back from the
            chip does not match what we think we wrote.  About the only
            way this can happen on NAND is for a page to be so garbled
            that the error can't be detected, much less corrected.
            "Read Failure" is currently unused.
        */
        case FFXERRTYPE_READBACK_VERIFY:
        case FFXERRTYPE_READ_FAILURE:
        case FFXERRTYPE_UNRECOVERABLE:
            return FFXERRPOLICY_ABANDONOPERATIONS;
    }
}

