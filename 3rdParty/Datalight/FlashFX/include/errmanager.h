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

    This interface file provides the structure and function declarations
    needed to use the FlashFX Error Manager.
---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
                                Revision History
    $Log: errmanager.h $
    Revision 1.5  2010/06/08 17:31:26Z  glenns
    Add support for devices whose hardware can recommend page
    scrubs.
    Revision 1.4  2009/01/27 23:20:41Z  glenns
    - Reorganized header file inclusion to relieve some obfuscation
      difficulty.
    Revision 1.3  2009/01/27 20:46:56Z  glenns
    - Added "of no interest" to list of flash error indications so that posting
      an error event does not cause use on uninitialized value for errors
      not managed by the error manager.
    Revision 1.2  2009/01/20 23:36:38Z  glenns
    - Added revision history block
    - Added support for marking an FFXIOSTATUS structure with the
      location of a correctable error when one is found (main page,
      spare area, main and spare, or can't tell).
---------------------------------------------------------------------------*/

#ifndef ERRMANAGER_H_INCLUDED
#define ERRMANAGER_H_INCLUDED

#define MAX_ERR_DEPTH_COUNT 10
#define HW_REQUESTS_SCRUB 0xFF

typedef enum _e_FfxFlashError
{
    FFXERRTYPE_OF_NO_INTEREST = 0,
    FFXERRTYPE_WRITE_FAILURE,
    FFXERRTYPE_ERASE_FAILURE,
    FFXERRTYPE_READ_FAILURE,
    FFXERRTYPE_TIMEOUT,
    FFXERRTYPE_CORRECTABLE,
    FFXERRTYPE_UNCORRECTABLE,
    FFXERRTYPE_READBACK_VERIFY,
    FFXERRTYPE_UNRECOVERABLE
} FFX_FLASHERROR;

typedef enum _e_FfxCorrectionLocale
{
    FFXCORRECTIONLOC_CANTTELL = 0,
    FFXCORRECTIONLOC_MAINPAGE,
    FFXCORRECTIONLOC_SPARE,
    FFXCORRECTIONLOC_MAINANDSPARE
} FFX_CORRECTIONLOCALE;

typedef enum _e_FfxErrorPolicyRecommendation
{
    FFXERRPOLICY_NO_ACTION = 0,
    FFXERRPOLICY_SCRUBLATER,
    FFXERRPOLICY_RELOCATEPAGE,
    FFXERRPOLICY_SCRUBNOW,
    FFXERRPOLICY_RETIREBLOCK,
    FFXERRPOLICY_ABANDONOPERATIONS,
    FFXERRPOLICY_NO_RECOMMENDATION
} FFX_ERRORPOLICY;

struct tagErrorMgrInstance;
typedef struct tagErrorMgrInstance ERRMANAGERINSTANCE;

D_UINT32 FfxErrMgrGetMaxCorrectableError(FFXFIMDEVHANDLE hFimDev);

D_UINT32 FfxErrMgrDecodeCorrectedBits(FFXIOSTATUS *pIoStat);

void FfxErrMgrEncodeCorrectedBits(D_UINT32 ulCorrectedBits, FFXIOSTATUS *pIoStat);

FFX_ERRORPOLICY FfxErrMgrDecodeRecommendation(FFXIOSTATUS *pIoStat);

void FfxErrMgrEncodeRecommendation(FFX_ERRORPOLICY policy, FFXIOSTATUS *pIoStat);

void FfxErrMgrEncodeCorrectionLocale(FFX_CORRECTIONLOCALE locale, FFXIOSTATUS *pIoStat);

FFX_CORRECTIONLOCALE FfxErrMgrDecodeCorrectionLocale(FFXIOSTATUS *pIoStat);

void FfxErrMgrReportError(
    FFXFIMDEVHANDLE  hFimDev,
    ERRMANAGERINSTANCE *instance,
    FFXIOSTATUS *pIoStat,
    D_UINT32 ulBlockNum,
    D_UINT32 ulPageOffset);

void FfxErrMgrKillError(FFXIOSTATUS *pIoStat);

ERRMANAGERINSTANCE *FfxErrMgrCreate(FFXFIMDEVHANDLE hFimDev);

void FfxErrMgrDestroy(ERRMANAGERINSTANCE *instance);

FFX_ERRORPOLICY FfxErrorMgrPolicyHook(
    FFX_FLASHERROR error,
    FFXIOSTATUS *pIoStat,
    FFXFIMDEVHANDLE hFimDev);

#endif /* ERRMANAGER_H_INCLUDED */
