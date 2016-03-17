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

    This module provides the means to do 1 and 2 bit error detection, and 1
    bit error correction.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxecc.c $
    Revision 1.17  2009/10/06 21:08:28Z  garyp
    Renamed the functions for consistency with other APIs.  Updated
    FfxEccCorrect() to return the DCL result codes.  Eliminated use of
    the FFXECC structure.
    Revision 1.16  2009/10/03 00:36:52Z  garyp
    Generalized the correction functionality and moved it into DCL.  Updated
    FfxEccCorrectData() to call the DCL function to do the correction.
    Revision 1.15  2009/08/04 18:54:22Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.14  2009/04/01 15:39:56Z  davidh
    Function Headers updated for AutoDoc.
    Revision 1.13  2009/02/19 05:07:49Z  glenns
    - Removed parameterized EDC stuff (which makes direct calls into
      the driver) into a separate file that will be placed in the driver.
      Part of the fix Bugzilla #2391.
    Revision 1.12  2009/02/18 18:34:23Z  glenns
    - Added code to prevent compiler warnings about unused formal
      parameters.
    Revision 1.11  2009/02/02 23:41:45Z  glenns
    - Renamed identifiers to use Datalight standards.
    - Added an interface to retrieve EDC processors by index rather
      than correction capability. Required for testing.
    Revision 1.10  2009/01/29 23:08:12Z  thomd
    Correct array bounds
    Revision 1.9  2009/01/27 23:29:24Z  glenns
    - Reorganized header file inclusion to relieve some obfuscation
      difficulty.
    Revision 1.8  2009/01/23 21:17:19Z  glenns
    - Clarified commentary
    Revision 1.7  2009/01/18 20:15:02Z  glenns
    - Renamed EDC Processor Array global using hungarian notation.
    - Added "static" qualifier to EDC processor and EDC Processor
      Array definitions.
    Revision 1.6  2009/01/17 01:49:35Z  billr
    Declare functions with prototypes, declare local functions static.
    Revision 1.5  2009/01/16 23:50:51Z  glenns
    - Added the "FfxParameterizedEccGenerate" and
      "FfxParameterizedEccCorrectData" functions. These are the
      primary interface between FlashFX and a parameterized ECC
      processor.
    - Added the "FfxDefaultOneBitCalculate" function. This works as
      a wrapper around the existing EDC one-bit EDC processor and
      allows it to be used with the parameterized ECC mechanism.
    - Added the public-domain 4-bit EDC processor and support
      functions.
    - Added two FX_EDC_PROCESSOR static definitions, one
      each for the one-bit and four-bit EDC processors, and included
      an static array to hold pointers to those definitions.
    - Added the function "FfxAcquireEccProcessor" to search the
      array of FX_EDC_PROCESSORs and pick an appropriate
      one based on input parameters.
    - Included the error manager header so that EDC processors can
      encode correctable error information into an FFXIOSTATUS.
    Revision 1.4  2008/07/23 18:07:15Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.3  2008/01/13 07:27:13Z  keithg
    Function header updates to support autodoc.
    Revision 1.2  2007/11/03 23:49:32Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/10/14 21:28:22Z  Garyp
    Initial revision
    Revision 1.4  2007/10/04 02:26:31Z  Garyp
    Documentation updated -- no functional changes.
    Revision 1.3  2007/02/06 21:06:22Z  billr
    Use new DCL ECC function.
    Revision 1.2  2006/02/15 19:23:50Z  thomd
    Moved cECC variables to local; adjusted call to Transform appropriately
    Revision 1.1  2005/10/26 12:54:14Z  Pauli
    Initial revision
    Revision 1.2  2005/10/26 13:54:13Z  Garyp
    Modified to use an FFXECC structure.
    Revision 1.1  2005/10/02 02:58:26Z  Garyp
    Initial revision
    Revision 1.5  2005/08/03 19:30:56Z  pauli
    Replaced DCL_FuncName with DclFuncName.
    Revision 1.3  2005/07/31 01:49:46Z  Garyp
    Updated to use new profiler leave function which now takes a ulUserData
    parameter.
    Revision 1.2  2005/07/30 22:44:26Z  Garyp
    Fixed a bogus message.
    Revision 1.1  2005/07/26 02:34:30Z  pauli
    Initial revision
    Revision 1.10  2005/03/26 23:03:30Z  GaryP
    Standardized the function headers.  General code formatting cleanup.
    Added some debug code.  Changed functions to static where possible.
    Renamed public but non-published functions to reduce name-space pollution.
    Revision 1.9  2004/12/30 23:03:12Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.8  2003/09/12 16:28:38Z  garys
    Minor comment change in ecccorrectdata().
    Revision 1.7  2003/09/11 20:52:11  billr
    Merged from [Integration_2K_NAND] branch.
    Revision 1.6  2003/05/21 21:49:24Z  billr
    Support larger erase zone sizes in BBM. Refactor bbm.h to simplify
    obfuscation.
    Revision 1.5  2002/11/14 07:38:54Z  garyp
    Removed the __COBF__ conditional.
    Revision 1.4  2002/11/14 06:58:56Z  garyp
    Updated the module header.
    Revision 1.3  2002/11/12 22:00:10Z  garyp
    Updated to use the new VBF header files.  Cleaned up the module headers.
    02/07/02 DE  Updated copyright notice for 2002.
    10/01/01 mjm Added BBM patent info.
    03/30/00 HDS Corrected a declaration to prevent an overlay.
    12/28/98 PKG Now includes flashfx.h instead of machine, assert, and utils.pt
    06/17/98 PKG Prototypes updated for more precise description and to ease
                 integration with the FIM.  correctdata now calls the calccode
                 instead of relying on the callee to call it.
    05/20/98 HDS Changed the code to meet Datalight code standards.
    05/04/98 PKG Types changed, defines added, globals declared
    05/04/98 PKG Original derived from Toshiba SmartMedia(TM) ECC Software
                 and Hardware specification.
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <fxiosys.h>
#include <ecc.h>


/*-------------------------------------------------------------------
    Public: FfxEccCalculate()

    Generate an error correction code (ECC) for 256 bytes of data.
    The ECC consists of 3 bytes of calculated parity data:

    BIT POSITION     7     6     5     4     3     2     1     0
    LINE PARITY    LP07  LP06  LP05  LP04  LP03  LP02  LP01  LP00
    LINE PARITY    LP15  LP14  LP13  LP12  LP11  LP10  LP09  LP08
    COLUMN PARITY  CP5   CP4   CP3   CP2   CP1   CP0    1     1

    Parameters:
        pData  - An array of data for which the ECC is to be computed.
        pabECC - A pointer to an array of ECC bytes, BYTES_PER_ECC
                 in length.

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxEccCalculate(
    const D_BUFFER *pData,
    D_BUFFER       *pabECC)
{
    D_UINT32        ulEcc;

    DclAssertReadPtr(pData, DATA_BYTES_PER_ECC);
    DclAssertReadPtr(pabECC, BYTES_PER_ECC);

    ulEcc = DclEccCalculate(pData, DATA_BYTES_PER_ECC);

    FFXECC_CANONICALTOSSFDC(ulEcc, pabECC);

    return;
}


/*-------------------------------------------------------------------
    Public: FfxEccCorrect()

    Compare a calculated ECC with the stored ECC.  If any differences
    exist, it is determined whether it is a correctable data error
    (i.e., all parity bit pairs have 1 error and 1 match), a
    correctable code error (i.e., only one bit is different) or an
    uncorrectable error.  If it is one of the correctable types,
    then the error is corrected.

    Parameters:
        pData      - an array of the ECC data to be checked
        pabReadEcc - a pointer to the location from which the
                     saved ECC is to be read
        pabCalcEcc - A pointer to an array of ECC bytes, BYTES_PER_ECC
                     in length.

    Return Value:
        Returns a DCLECCRESULT value which will be one of the
        following values:
          DCLECC_VALID         - The data and old ECC value had no
                                 errors.
          DCLECC_DATACORRECTED - The data was corrected.
          DCLECC_ECCCORRECTED  - The old ECC value was corrected.
          DCLECC_UNCORRECTABLE - The data and/or old ECC has
                                 uncorrectable errors.
-------------------------------------------------------------------*/
DCLECCRESULT FfxEccCorrect(
    D_BUFFER       *pData,
    D_BUFFER       *pabReadEcc,
    const D_BUFFER *pabCalcEcc)
{
    D_UINT32        ulOldECC;
    D_UINT32        ulNewECC;
    DCLECCRESULT    iResult;

    DclAssertWritePtr(pData, DATA_BYTES_PER_ECC);
    DclAssertWritePtr(pabReadEcc, BYTES_PER_ECC);
    DclAssertReadPtr(pabCalcEcc, BYTES_PER_ECC);

    ulOldECC = FFXECC_SSFDCTOCANONICAL(pabReadEcc);
    ulNewECC = FFXECC_SSFDCTOCANONICAL(pabCalcEcc);

    iResult = DclEccCorrect(pData, DATA_BYTES_PER_ECC, &ulOldECC, ulNewECC);

    if(iResult == DCLECC_ECCCORRECTED)
        DclMemCpy(pabReadEcc, pabCalcEcc, BYTES_PER_ECC);

    return iResult;
}


