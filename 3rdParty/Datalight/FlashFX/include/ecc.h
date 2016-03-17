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

    Public interface to ECC generation and checking.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ecc.h $
    Revision 1.12  2009/10/06 06:08:38Z  garyp
    Prototypes updated.  Eliminated the obsolete ECC_RESULT enum.
    Revision 1.11  2009/10/02 23:17:51Z  garyp
    Added macros to support ECC conversions.
    Revision 1.10  2009/03/25 18:01:14Z  glenns
    - Default ALLOW_INFERIOR_EDC to FALSE.
    Revision 1.9  2009/03/24 20:01:56Z  glenns
    - Add a flag macro that allows users to configure FlashFX to use
      EDC mechanisms that are weaker than the chip requirements for
      testing or one-off purposes.
    Revision 1.8  2009/02/18 23:22:02Z  glenns
    - Updated function prototypes for EDC Processor "Init" and
      "Destroy" to take an EdcProcessor instance as a parameter, so
      that dynamically-allocated resources needed by the EDC
      algorithms can be associated with the instance.
    - Changed data type to a more meaningfull name.
    Revision 1.7  2009/02/03 00:08:59Z  glenns
    - Changed function pointer typedefs to more accurately represent
      Datalight coding standards.
    - Added interface to access EDC processor list with an index.
      Useful for testing.
    Revision 1.6  2009/01/17 01:54:41Z  billr
    Remove unnecessary typedef that caused a compiler warning.
    Declare functions with prototypes; but the prototypes may not yet
    be correct.
    Revision 1.5  2009/01/17 00:02:26Z  glenns
    - Added typedefs and function declarations to support
      parameterized EDC.
    - Added commentary to distinguish between "traditional" EDC
      and parameterized EDC.
    Revision 1.4  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.3  2007/10/14 21:12:26Z  Garyp
    Renamed the ECC functions.
    Revision 1.2  2005/12/15 02:17:31Z  garyp
    Fixed data type issues to keep CE happy.
    Revision 1.1  2005/12/02 01:20:14Z  Pauli
    Initial revision
    Revision 1.3  2005/12/02 01:20:14Z  Garyp
    Added the MAX_ECCS_PER_PAGE setting.
    Revision 1.2  2005/10/26 15:13:17Z  Garyp
    Prototype update.
    Revision 1.1  2005/10/02 02:58:28Z  Garyp
    Initial revision
    Revision 1.4  2005/03/28 01:39:46Z  GaryP
    Minor cleanup.
    Revision 1.3  2004/12/30 23:03:11Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.2  2003/08/21 22:02:18Z  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.1  2003/08/21 22:02:18Z  billr
    Initial revision
---------------------------------------------------------------------------*/

#ifndef ECC_H_INCLUDED
#define ECC_H_INCLUDED


/*---------------------------------------------------------------------------

                SECTION 1: SLC EDC/ECC Functionality

    This section of this interface defines the interface for the
    traditional one-bit-correction two-bit-detection EDC implementation
    that exists in DCL. It uses macros for ECC parameters and fixes
    the correction algorithm to use only the aforementioned implementation.
    MLC NAND flash requires heavier-duty ECC and is available through
    the second section of this interface.

    This section is being left here intact to ease the transition to the
    newer method of ECC management; existing ECC management will continue
    to function unchanged. However, perhaps moving forward into Tera
    Phase II, this may soon become deprecated.

---------------------------------------------------------------------------*/

/*  Size of a single ECC code in bytes and the sector it describes
*/
#define BYTES_PER_ECC           (3)
#define DATA_BYTES_PER_ECC      (256)
#define MAX_ECCS_PER_PAGE       (FFX_NAND_MAXPAGESIZE / DATA_BYTES_PER_ECC)
#define MAX_ECC_BYTES_PER_PAGE  (MAX_ECCS_PER_PAGE * BYTES_PER_ECC)

#define FFXECC_CANONICALTOSSFDC(ulECC, pSSFDC)              \
    pSSFDC[0] = (D_UINT8)(ulECC >> 6);                      \
    pSSFDC[1] = (D_UINT8)(ulECC >> 14);                     \
    pSSFDC[2] = (D_UINT8)(ulECC << 2) | 3;

#define FFXECC_SSFDCTOCANONICAL(pSSFDC)                     \
    ((((D_UINT32)pSSFDC[0]) << 6) |                         \
     (((D_UINT32)pSSFDC[1]) << 14) |                        \
     (((D_UINT32)pSSFDC[2]) >> 2))

/*  Software ECC functions
*/
void         FfxEccCalculate(const D_BUFFER *pcData, D_BUFFER *pabECC);
DCLECCRESULT FfxEccCorrect(D_BUFFER *pcData, D_BUFFER *pcReadEcc, const D_BUFFER *pabECC);


/*---------------------------------------------------------------------------

           SECTION 2: Generalized MLC EDC/ECC Functionality

    In this section, EDC functionality and parameterization is done through
    data structures that are passed up and down the call chain. By
    appropriately initializing and populating these structures, EDC/ECC
    is fully parameterized and no macros are necessary to complete ECC
    generation and evaluation.

    REFACTOR: The following typedefs, structure elements and function pointer
    declarations require some explanation/discussion in comment blocks.

---------------------------------------------------------------------------*/

/*  This macro allows the system to be configured to use the best available
    EDC mechanism to satisfy the Acquire procedure rather than insisting on
    full protection.

    WARNING: This macro is provided mainly for testing and for one-off
    configurations in which full EDC protection is not required. It is
    *highly* recommended to set this FALSE for any production distributions,
    as failing to protect devices with full recommended EDC will cause
    FlashFX to prematurely retire blocks and seriously reduce the device's
    apparent usable life.
*/
#define ALLOW_INFERIOR_EDC FALSE

typedef enum eECCMCD
{
    ECC_CMD_ENCODE = 0,
    ECC_CMD_VERIFY
} FX_ECC_CMD;

typedef struct tagBufferDesc
{
    D_BUFFER *buffer;
    D_UINT32  uCount;
    D_UINT32  uOffset;
} FX_BUFFER_DESC;

typedef struct tagEccParams
{
    FX_BUFFER_DESC  data_buffer;
    FX_BUFFER_DESC  metadata_buffer;
    D_BUFFER       *ecc_buffer;
    D_BUFFER       *reference_ecc_buffer;
    D_UINT32        ulECCBufferByteCount;
    FX_ECC_CMD      cmd;
    FFXIOSTATUS    *pIoStat;
    struct tagEDCProcessor *pEccProcessor;
} FX_ECC_PARAMS;

struct tagEdcCalcParams;
typedef struct tagEdcCalcParams EDCCALCPARAMS;

typedef void (*PFNCALCULATEECC)(FX_ECC_PARAMS *params);

typedef D_BOOL (*PFNEDCINIT)(struct tagEDCProcessor *pEdcProcessor);

typedef void (*PFNEDCSTART)(void);

typedef void (*PFNEDCDESTROY)(struct tagEDCProcessor *pEdcProcessor);

typedef struct tagEDCProcessor
{
    PFNCALCULATEECC Calculate;
    PFNEDCINIT      Init;
    PFNEDCSTART     Start;
    PFNEDCDESTROY   Destroy;
    D_UINT16        ulCorrectionCapability;
    D_UINT16        ulDetectionCapability;
    D_UINT32        ulNumDataBytesEvaluated;
    D_UINT32        ulNumMetaBytesEvaluated;
    D_UINT32        ulNumBytesForECC;
    D_BOOL          fRequiresRecalculatedECC;
    EDCCALCPARAMS  *pEdcCalcParams;
} FX_EDC_PROCESSOR;

void FfxParameterizedEccGenerate(FX_ECC_PARAMS *params);

void FfxParameterizedEccCorrectData(FX_ECC_PARAMS *params);

FX_EDC_PROCESSOR *FfxAcquireEdcProcessor(D_UINT32 ulBitsOfCorrection);

FX_EDC_PROCESSOR *FfxGetIndexedECCProcessor(D_UINT32 ulIndex);



#endif  /*  ECC_H_INCLUDED */

