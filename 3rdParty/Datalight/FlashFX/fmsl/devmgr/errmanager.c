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

    This interface file provides the structure and function definitions
    needed to use the FlashFX Error Manager.

    ToDo:
     - Update this code to use Datalight standard Hungarian notation.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: errmanager.c $
    Revision 1.17  2010/09/19 03:18:53Z  garyp
    Clarified some debug messages.
    Revision 1.16  2009/04/16 00:28:40Z  garyp
    Documentation updates and general code cleanup -- no functional changes.
    Revision 1.15  2009/04/02 17:06:19Z  glenns
    - Add parens to function name in header comments to satisfy Autodoc.
    Revision 1.14  2009/04/02 16:04:58Z  glenns
    - Fix Bugzilla #2385: Add and amend function header comment
      blocks as necessary.
    Revision 1.13  2009/04/01 20:13:12Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.12  2009/03/26 06:34:55Z  glenns
    - Fix Bugzilla #2464: Added processing paths for new status codes
      FFXSTAT_FIM_UNEXPECTEDHWSTATE and FFXSTAT_INTERNAL_ERROR.
    Revision 1.11  2009/03/18 16:17:34Z  glenns
    - Fix Bugzilla #2525: Added a case to handle the new FFXSTAT_FIMUNCORRECTED
      error code, with a policy of FFXERRPOLICY_NO_ACTION
    Revision 1.10  2009/02/27 22:59:27Z  billr
    Some work on bug 2454: default Error Manager policy recommends
    retiring blocks with read errors
    Revision 1.9  2009/02/26 00:08:33Z  billr
    Resolve bug 2454: default Error Manager policy recommends retiring blocks
    with read errors.  Resolve bug 2455: NAND program failure may not cause
    block replacement.  Eliminate deprecated status FFXSTAT_FIMERASEFAILED.
    Revision 1.8  2009/01/27 22:58:33Z  glenns
    - Reorganized header file inclusion to relieve some obfuscation difficulty.
    Revision 1.7  2009/01/27 20:47:18Z  glenns
    - Fixed uninitialized use of a variable in the case that an error has been
      reported that is not managed by the error manager.
    Revision 1.6  2009/01/27 04:42:11Z  glenns
    - Modified to use improved event manager interfaces.
    - Modified to accomodate structure variable names that were
      updated to Datalight coding standards.
    Revision 1.5  2009/01/21 19:07:06Z  glenns
    - moved "report.policy" assignment out of conditional compile to
      prevent possible uninitialized variable use.
    Revision 1.4  2009/01/20 22:45:06Z  glenns
    - Added revision history block.
    - Removed obsolete comment
    - Added support for encoding and decoding bit error correction
      locations (main page, spare area, both or can't tell) into an
      FFXIOSTATUS structure.
    - Gave FFXSTAT_TIMEOUT indications a separate processing
      block in the ReportError function.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fimdev.h>
#include "devmgr.h"
#include <errmanager.h>
#include <eventmgr.h>


#define CORRECTEDBITS_SHIFT         (12)
#define CORRECTIONLOCALE_SHIFT      (10)
#define POLICY_REC_SHIFT            (4)

typedef struct tagErrorReport
{
    FFX_FLASHERROR error;
    FFX_ERRORPOLICY policy;
} FFX_ERROR_REPORT;

struct tagErrorMgrInstance
{
    D_BOOL bOperationTerminated;
    void *pErrorMgrInstanceID;
};

#if FFX_USE_EVENT_MANAGER
static void ErrMgrEventHandlerProc(FFX_EVENT *event);
#endif


            /*------------------------------------------*\
             *                                          *
             *             Public Interface             *
             *                                          *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
    Public: FfxErrMgrEncodeCorrectedBits()

    Encode a number of corrected bits into an FFXIOSTATUS structure.
    Any value currently encoded in the FFXIOSTATUS structure will be
    overwritten.

    This is normally used by an NTM when it detects corrected errors.

    Parameters:
        ulCorrectedBits - The number of corrected bits
        pIoStat         - A pointer to the FFXIOSTATUS structure into
                          which we are to encode the bit correction
                          number

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxErrMgrEncodeCorrectedBits(
    D_UINT32        ulCorrectedBits,
    FFXIOSTATUS    *pIoStat)
{
    pIoStat->op.ulPageStatus &= ~PAGESTATUS_NUMCORRECTEDBITS;
    pIoStat->op.ulPageStatus |=
        ((ulCorrectedBits << CORRECTEDBITS_SHIFT) & PAGESTATUS_NUMCORRECTEDBITS);

    return;
}


/*-------------------------------------------------------------------
    Public: FfxErrMgrDecodeCorrectedBits()

    Decode the number of corrected bits encoded in an FFXIOSTATUS
    structure.

    This function is used to extract the number of corrected bits
    that may be indicated within an FFXIOSTATUS structure.  Use of
    this function presumes that the lower-level hardware interfaces
    have determined that correctable bit error has occurred, and has
    encoded that value into its FFXIOSTATUS return value.  Use of
    this function on an FFXIOSTATUS structure that has not been so
    encoded will return a value of zero, even if a correctable error
    condition exists. It will also return zero for other types of
    errors that may occur.

    Parameters:
        pIoStat - A pointer to the FFXIOSTATUS structure from
                  which we are to decode the bit correction
                  number

    Return Value:
        A D_UINT32 containing the number of corrected bits.
-------------------------------------------------------------------*/
D_UINT32 FfxErrMgrDecodeCorrectedBits(
    FFXIOSTATUS    *pIoStat)
{
    D_UINT32        ulCorrectedBits;

    ulCorrectedBits = pIoStat->op.ulPageStatus & PAGESTATUS_NUMCORRECTEDBITS;
    ulCorrectedBits = ulCorrectedBits >> CORRECTEDBITS_SHIFT; /* NumCorrectedBits field offset */

    /*  Why are we casting this to a D_UINT16 when we are returning
        a D_UINT32 in the first place?
    */
    return (D_UINT16)ulCorrectedBits;
}


            /*------------------------------------------*\
             *                                          *
             *          Protected Interface             *
             *                                          *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
    Protected: FfxErrMgrGetMaxCorrectableError()

    Get the rated maximum number of bits that can be corrected by a
    FIM’s underlying correction mechanism.  Note that a given
    correction mechanism may correct more than the rated number of
    bits; this figure indicates the number of bits it is guaranteed
    to correct.

    Parameters:
        hFimDev - Handle for a FIM Device

    Return Value:
        A D_UINT16 indicating the correction rating.
-------------------------------------------------------------------*/
D_UINT32 FfxErrMgrGetMaxCorrectableError(
    FFXFIMDEVHANDLE hFimDev)
{
    (void)hFimDev;

    return hFimDev->pFimInfo->uEdcCapability;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrEncodeRecommendation()

    Encode an error policy recommendation into an FFXIOSTATUS structure.

    Parameters:
        policy  - An erro policy recommendation of type
                  FFX_ERRORPOLICY
        pIoStat - A pointer to the FFXIOSTATUS structure into
                  which we are to encode the recommendation

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxErrMgrEncodeRecommendation(
    FFX_ERRORPOLICY policy,
    FFXIOSTATUS    *pIoStat)
{
    D_INT32         ulPolicy = (D_UINT32)policy;

    pIoStat->op.ulPageStatus &= ~PAGESTATUS_ERRPOLICY;
    pIoStat->op.ulPageStatus |=
        ((ulPolicy << POLICY_REC_SHIFT) & PAGESTATUS_ERRPOLICY);

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrDecodeRecommendation()

    Given an FFXIOSTATUS structure, get the an Error Manager
    recommendation on how to process the current error indication.

    This procedure may be used by a client to decode the type of
    Error Recommendation that may be indicated within an FFXIOSTATUS
    structure.  Use of this procedure presumes that the lower-level
    hardware interfaces have determined that some sort of error has
    occurred, and that this error has been reported to the Error
    Manager, which happens within the Device Manager. Use of this
    procedure on an FFXIOSTATUS structure that has not been processed
    by the Error Manager may produce undesirable results.

    Parameters:
        pIoStat    - a pointer to and FFXIOSTATUS that may have and
                     Error Manager recommendation in it.

    Return Value:
        A value of type FFX_ERRORPOLICY, representing an Error
        Manager recommendation.
-------------------------------------------------------------------*/
FFX_ERRORPOLICY FfxErrMgrDecodeRecommendation(
    FFXIOSTATUS    *pIoStat)
{
    D_UINT32        ulPolicy;

    ulPolicy = pIoStat->op.ulPageStatus & PAGESTATUS_ERRPOLICY;
    ulPolicy = ulPolicy >> POLICY_REC_SHIFT; /* Policy Recommendation field offset */

    return (FFX_ERRORPOLICY)ulPolicy;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrEncodeCorrectionLocale()

    Some NTMs have the ability to tell where corrected bit errors occur,
    whether they be in the main page data area or the spare area. For
    those NTMs that have that capability, this procedure allows them to
    encode that data into the ulPageStatus field of the FFXIOSTATUS
    structure that will be returned.

    Parameters:
        locale      - The location of a corrected bit error
        pIoStat     - The FXIOSTATUS structure into which the
                      locale should be encoded.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxErrMgrEncodeCorrectionLocale(
    FFX_CORRECTIONLOCALE    locale,
    FFXIOSTATUS            *pIoStat)
{
    pIoStat->op.ulPageStatus &= ~PAGESTATUS_CORRECTIONLOCALE;
    pIoStat->op.ulPageStatus |=
        ((locale << CORRECTIONLOCALE_SHIFT) & PAGESTATUS_CORRECTIONLOCALE);

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrDecodeCorrectionLocale()

    Some NTMs have the ability to tell where corrected bit errors occur,
    whether they be in the main page data area or the spare area.
    This procedure allows a client to retrieve that information from
    the FFXIOSTATUS structure that was returned to it.

    Parameters:
        pIoStat     - The FFXIOSTATUS structure that may contain an
                      encoded location of a corrected bit error. If
                      no encoding took place, will return CANTTELL.
    Return Value:
        The location of the corrected bit error.
-------------------------------------------------------------------*/
FFX_CORRECTIONLOCALE FfxErrMgrDecodeCorrectionLocale(
    FFXIOSTATUS    *pIoStat)
{
    D_UINT32        ulLocale;

    ulLocale = pIoStat->op.ulPageStatus & PAGESTATUS_CORRECTIONLOCALE;
    ulLocale = ulLocale >> CORRECTIONLOCALE_SHIFT; /* Policy Recommendation field offset */

    return (FFX_CORRECTIONLOCALE)ulLocale;
}


/*  The following procedure is the primary error reporting mechanism.
*/


/*-------------------------------------------------------------------
    Protected: FfxErrMgrReportError()

    Report an error to the Error Manager and encode a policy
    recommendation in the pIoStat parameter.

    This function is called when a new error is detected.  The Error
    Manager feeds the reported error into the Policy Hook, which
    generates a Policy Recommendation.  This function then records
    the Flash Error itself and its associated Policy Recommendation
    in the FFXIOSTATUS structure provided.

    This function is used exclusively by the Device Manager.

    Parameters:
        hFimDev         - Fim Device handle
        instance        - Error Manager Instance
        ulBlockNum      - Block number where the reported error was
                          found
        ulPageOffset    - Page offset into the block where the
                          reported error was found

    Return Value:
        Error policy recommendation encoded in the pIoStat parameter.
-------------------------------------------------------------------*/
void FfxErrMgrReportError(
    FFXFIMDEVHANDLE     hFimDev,
    ERRMANAGERINSTANCE *instance,
    FFXIOSTATUS        *pIoStat,
    D_UINT32            ulBlockNum,
    D_UINT32            ulPageOffset)
{
    FFX_ERROR_REPORT    report;

#if FFX_USE_EVENT_MANAGER
DECLARE_EVENT_OBJECT(event,FFX_EVENT_REPORT_FLASHERROR);
FLASH_ERROR_METADATA *metadata;
#endif

    DclAssert(hFimDev);
    DclAssert(instance);
    DclAssert(pIoStat);

    /*  Check to see if operation has been terminated. Note that if
        operation has been terminated, the head of the error list will
        indicate an unrecoverable condition with a policy recommendation
        of Abandon Operations. Note also that the only way out of this
        condition is to reboot FlashFX; once an unrecoverable condition
        is encountered, the Error Manager will continue to issue policy
        recommendations of Abandon Operations until reset, regardless
        of what else happens.
    */
    if (!(instance->bOperationTerminated))
    {
        /*  Decode the error information:
        */
        switch (pIoStat->ffxStat)
        {
            case FFXSTAT_FIMREADSTARTFAILED:
            case FFXSTAT_FIMREADCOMPLETEFAILED:
            case FFXSTAT_FIMWRITESTARTFAILED:
            case FFXSTAT_FIMWRITECOMPLETEFAILED:
                FFXPRINTF(1, ("ErrMgr: Deprecated error %lX is unrecoverable.\n", pIoStat->ffxStat));
                /*  fall through */

            case FFXSTAT_FIMMOUNTFAILED:
            case FFXSTAT_FIMBOUNDSINVALID:
            case FFXSTAT_FIMIOERROR:
            case FFXSTAT_INTERNAL_ERROR:
            {
                /*  This class of errors are considered generally unrecoverable
                    to FlashFX, and FlashFX will be told to abandon operations.
                */
                report.error = FFXERRTYPE_UNRECOVERABLE;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIMTIMEOUT:
            {
                /*  Timeout errors:
                */
                report.error = FFXERRTYPE_TIMEOUT;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIM_WRITEFAILED:
            {
                /*  This class of errors are considered write failures.
                */
                report.error = FFXERRTYPE_WRITE_FAILURE;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_DEVMGR_READBACKVERIFYFAILED:
            {
                /*  This class of errors are readback-verify failures.
                */
                report.error = FFXERRTYPE_READBACK_VERIFY;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIM_ERASEFAILED:
            case FFXSTAT_FIM_ERASESUSPENDFAILED:
            case FFXSTAT_FIM_ERASERESUMEFAILED:
            {
                /*  This class of errors are considered erase failures.
                */
                report.error = FFXERRTYPE_ERASE_FAILURE;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIMUNCORRECTABLEDATA:
            {
                /*  This class of errors constitute uncorrectible read errors.
                    They are distinguished from other read failures because these
                    may occur in software with no indication from the flash
                    device that anything went wrong.
                */
                report.error = FFXERRTYPE_UNCORRECTABLE;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIMCORRECTABLEDATA:
            {
                /*  This class of errors constitute correctible read errors.
                */
                report.error = FFXERRTYPE_CORRECTABLE;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            case FFXSTAT_FIMUNCORRECTED:
            case FFXSTAT_FIM_UNEXPECTEDHWSTATE:
            {
                report.error = FFXERRTYPE_OF_NO_INTEREST;
                report.policy = FfxErrorMgrPolicyHook(report.error, pIoStat, hFimDev);
                break;
            }
            default:
            {
                /*  An error indication of no interest to the Flash Error Manager.
                */
                FFXPRINTF(1, ("ErrMgr: Unexpected error %lX is ignored.\n", pIoStat->ffxStat));

                report.error = FFXERRTYPE_OF_NO_INTEREST;
                report.policy = FFXERRPOLICY_NO_ACTION;
                break;
            }
        }

      #if FFX_USE_EVENT_MANAGER
        metadata = &(event.event_metadata.error_metadata);
        metadata->ulBlockNum = ulBlockNum;
        metadata->ulPageOffset = ulPageOffset;
        metadata->ulMaxCorrectible = FfxErrMgrGetMaxCorrectableError(hFimDev);
        metadata->ulNumBitErrors = FfxErrMgrDecodeCorrectedBits(pIoStat);
        metadata->ulFlashErrorType = (D_UINT32)report.error;
        FfxEventMgrPostEvent(hFimDev->eventMgrInstance, &event);
      #endif
    }
    else
    {
        report.error = FFXERRTYPE_UNRECOVERABLE;
        report.policy = FFXERRPOLICY_ABANDONOPERATIONS;
    }

    FfxErrMgrEncodeRecommendation(report.policy, pIoStat);

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrKillError()

    Remove the error indications from an FFXIOSTATUS structure.  It
    should be called when a client of the Error Manager has determined
    that an error it has noted has been successfully dealt with, and
    the indication is no longer required.

    The function will clear the current correction condition, the
    current recommendation, and set the current FXSTATUS indication
    to FFXSTAT_SUCCESS.

    Parameters:
        pIoStat - A pointer to the FFXIOSTATUS structure
                  containing an error indication

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxErrMgrKillError(
    FFXIOSTATUS    *pIoStat)
{
    DclAssert(pIoStat);

    FfxErrMgrEncodeRecommendation(FFXERRPOLICY_NO_ACTION, pIoStat);
    FfxErrMgrEncodeCorrectedBits(0, pIoStat);

    pIoStat->ffxStat = FFXSTAT_SUCCESS;

    return;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrCreate()

    Create an Error Manager instance.

    Parameters:
        hFimDev - A handle to a FIM Device with which this
                  instance is to be associated.

    Return Value:
        An error manager instance pointer. Set to NULL if an instance
        could not be created.
-------------------------------------------------------------------*/
ERRMANAGERINSTANCE *FfxErrMgrCreate(
    FFXFIMDEVHANDLE         hFimDev)
{
    ERRMANAGERINSTANCE     *errManager;
  #if FFX_USE_EVENT_MANAGER
    EVENTMGRINSTANCE       *evtManager;
  #endif

    if (hFimDev == NULL)
        return NULL;

    errManager = DclMemAlloc(sizeof(*errManager));
    if (!errManager)
        return NULL;

    errManager->bOperationTerminated = FALSE;
    errManager->pErrorMgrInstanceID = (void *)&errManager;

  #if FFX_USE_EVENT_MANAGER
    evtManager = FfxDevGetEventMgrInstance(hFimDev);
    FfxEventMgrRegisterEventHandler(
        evtManager,
        FFX_EVENT_REPORT_FLASHERROR,
        ErrMgrEventHandlerProc);
  #endif

    /*  Return the error manager instance:
    */
    return errManager;
}


/*-------------------------------------------------------------------
    Protected: FfxErrMgrDestroy()

    Destroy an Error Manager instance.

    Parameters:
        instance - A pointer to an error manager instance that
                   is to be destroyed.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxErrMgrDestroy(
    ERRMANAGERINSTANCE *instance)
{
    if (instance == NULL)
        return;

    /*  Free the instance:
    */
    DclMemFree(instance);

    return;
}


            /*------------------------------------------*\
             *                                          *
             *            Helper Functions              *
             *                                          *
            \*------------------------------------------*/


#if FFX_USE_EVENT_MANAGER

/*-------------------------------------------------------------------
    Local: ErrMgrEventHandlerProc()

    To aid in debugging and data logging, the Error Manager uses a
    rudimentary event manager (see eventmgr.h). This procedure is
    a callback the Error Manager registers with the event manager
    for the event manager to call when an error event is posted.

    Parameters:
        event - A pointer to the error event that was
                posted

    Return Value:
        None.
-------------------------------------------------------------------*/
static void ErrMgrEventHandlerProc(
    FFX_EVENT              *event)
{
    #define                 PRINTERROREVENT_DEBUGLEVEL  (2)
    FLASH_ERROR_METADATA   *metadata;
    FFX_FLASHERROR          error;

    DclAssert(event->event == FFX_EVENT_REPORT_FLASHERROR);

    metadata = &(event->event_metadata.error_metadata);
    error = metadata->ulFlashErrorType;

    FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, ("FfxErrorManager: Received Error Report\n"));
    FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, (" Error Type: %d\n", error));
    FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, (" Block: %x\n", metadata->ulBlockNum));
    FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, (" Page Offset: %x\n", metadata->ulPageOffset));
    if (error == FFXERRTYPE_CORRECTABLE)
    {
        FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, (" Corrected bits: %d\n", metadata->ulNumBitErrors));
        FFXPRINTF(PRINTERROREVENT_DEBUGLEVEL, (" Max Correctable: %d\n", metadata->ulMaxCorrectible));
    }

    return;
}

#endif


