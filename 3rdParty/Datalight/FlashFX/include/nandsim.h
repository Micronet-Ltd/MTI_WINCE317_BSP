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

    This header defines internal interfaces used by the NAND Simulator.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: nandsim.h $
    Revision 1.5  2010/02/25 02:23:16Z  billr
    Refactor so the storage layer can be in an OS Services module.
    Revision 1.4  2009/04/08 01:43:06Z  garyp
    Removed incorrectly placed function headers.  Documentation updates
    only -- no functional changes.
    Revision 1.3  2009/02/19 05:18:13Z  keithg
    Changed the behavior of the table based policy to limit the number of
    errors thown for a given element.  See NsOperationElement.
    Revision 1.2  2009/02/06 17:27:33Z  billr
    Add new NAND Simulator error policy.
    Revision 1.1  2009/02/06 02:23:18Z  keithg
    Initial revision
    Revision 1.18  2009/01/29 11:23:30  keithg
    Moved table policy specific structure to a private include.
    Revision 1.17  2009/01/22 19:03:02  keithg
    Added NAND simulator configuration structure and conditional definition
    of the NAND ID string.
    Revision 1.16  2009/01/18 21:20:56  keithg
    Table policy is now public information move here from nsptable.c.
    Revision 1.15  2008/02/03 02:15:32Z  keithg
    comment updates to support autodoc
    Revision 1.14  2007/12/27 00:24:15Z  Garyp
    Eliminated the use of FFXCONF_NAND_SIMULATOR.  The NAND simulator
    code is now conditional on FFXCONF_NANDSUPPORT only.
    Revision 1.13  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.12  2007/10/05 00:01:41Z  billr
    Add capability to simulate partial programming.
    Revision 1.11  2007/09/20 01:22:18Z  pauli
    Added the ability for the simulation mechanism to report to the NTM
    that the flash is "new."
    Revision 1.10  2007/08/22 19:35:46Z  pauli
    Added a name field to the policy and storage interfaces.
    Revision 1.9  2007/08/01 17:24:15Z  timothyj
    Converted some 64-bit types back to 32-bit and used 64-bit DCL macros for
    all 64-bit operations, to facilitate use in environments where the emulated
    64-bit types and environment-specific 64-bit libraries are used. (BZ #512
    and BZ #803)
    Revision 1.8  2007/06/26 19:10:37Z  timothyj
    Added Erase method, for more accurate simulation purposes and to enable
    storage mechanism optimizations.
    Revision 1.7  2007/03/01 22:02:17Z  timothyj
    Changed pages per block computations to be D_UINT16, to avoid warnings or
    casts for working with the representations of this value in the NTMINFO and
    FIMINFO structs.
    Revision 1.6  2006/12/06 21:33:38Z  DeanG
    Add new table-driven error policy module
    Revision 1.5  2006/10/20 00:50:31Z  DeanG
    Forward-port simulator persistence enhancement
    Revision 1.4.1.2  2006/10/20 00:50:31Z  DeanG
    Separate out device dimensions into separate structure.
    Revision 1.4  2006/08/26 00:30:04Z  Pauli
    Added compiler conditions to display an appropriate error message if the
    NAND simulator requirements are not met and to disable it by default.
    Revision 1.3  2006/07/29 01:24:45Z  DeanG
    Changes to accomodate enhanced random policy module
    Revision 1.2  2006/07/27 23:36:25Z  DeanG
    Changes to add "random" policy module
    Revision 1.1  2006/07/26 17:38:42Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#if FFXCONF_NANDSUPPORT

#include <nsstorage.h>
#include <nandid.h>

/*  ===========================================================================
    Error policy layer:
*/

/*  NsPolicyHandle is an internal reference to private data associated with
    an instance of an error policy.  It is returned by the policy layer Create
    call.  It contains any internal variables that layer desires.
*/
typedef struct tagNsPolicyData *NsPolicyHandle;
typedef struct tagNsPolicyData  NsPolicyData;

/* NsDevDimensions contains information about the "dimensions" (sizes)
   of various aspects of the device.
*/
typedef struct
{
    D_UINT32 pageDataSize;   /* in bytes */
    D_UINT32 pageSpareSize;  /* in bytes */
    D_UINT32 pageTotalSize;  /* in bytes */
    D_UINT16 pagesPerBlock;
    D_UINT32 totalBlocks;
} NsDevDimensions;

/*  NsPolicyParam contains the information about read/write requests.
*/
typedef struct
{
    D_UINT32         ulPageNumber;
    D_BUFFER const * pData;
    D_UINT32         ulSize;       /* size in bytes of spare or data area */
    D_UINT16         uPagesPerBlock;
} NsPolicyPageParam;

/*  NsPolicyBlockParam contains the information about block requests.
*/
typedef struct
{
    D_UINT32 ulBlockNumber;
    D_UINT32 ulSize;       /* size in bytes of data area */
} NsPolicyBlockParam;

/*  NsPolicyError enumerates the error types.
*/
typedef enum
{
    NsPolicyError_None,
    NsPolicyError_Timeout,
    NsPolicyError_IO,
    NsPolicyError_FactoryBadBlock,
    NsPolicyError_PageBit,    /*  error applies to every page in the block */
    NsPolicyError_BlockBit,   /*  error applies to explicit bits within the entire block */
    NsPolicyError_PartialProgram, /* interrupted programming, just a few/most but not all bits */
    NsPolicyError_Count       /* count of the number of items in the enum */
} NsPolicyError;

/*  NsPolicyBitState enumerates the type of bit change.
*/
typedef enum
{
    NsPolicyBitState_On,
    NsPolicyBitState_Off,
    NsPolicyBitState_Toggle,
    NsPolicyBitState_Count   /* count of the number of items in the enum */
} NsPolicyBitState;

/*  NsPolicyBitError contains the information to communicate a single bit
    error.  Multiple bit errors are communicated using an array of single
    bit errors.

    If the error is NsPolicyError_PageBit, the offset is a bit offset within a page.
    If the error is NsPolicyError_BlockBit, the offset is a bit offset within the block.

    To add the ability to specific "stuck" bits, we would add a flag to
    this struct to inidicate stuck bits.  This is not currently implemented.
*/
typedef struct
{
    NsPolicyBitState state;
    D_UINT32         ulOffset;
} NsPolicyBitError;

/*  NsPolicyAction contains the action the policy layer is requesting
    of the caller.
*/
typedef struct
{
    NsPolicyError            error;
    NsPolicyBitError const * pBitErrors;
    D_UINT32                 ulNumBitErrors;
} NsPolicyAction;


/*  The Table based Error Policy uses the NsOperationElement and
    NsOperationList to inject error conditions after certain conditions
    are met.  The following structure is the key to the use of the table
    based error policy module.

    For each type of operation (read/write/erase), there is a list of
    NsOperationElements.  The appropriate list is sequentially scanned on
    each operation.  A list element "matches" when the list element "ulMatch"
    value is equal to either of the following:
        *   The page/block number of the operation.
        *   -1 (ie, any page/block addresses match).

    When a match occurs:

     1. The list element "ulCount" value is incremented.
     2. The list element "errorType" is examined:
         *  If it is NOT equal to "NsPolicyError_None",
            AND the "ulCount" value exceeds the "ulThreshold" value,
            AND the "ulCount" value is less than "ulThreshold + ulMaxErrors",
            THEN the designated error is generated, using the "errorBits" struct,
            AND a debug message is displayed.
         *  If it IS equal to "NsPolicyError_None",
            AND the "ulThreshold" value modulo the "ulCount" value is zero,
            THEN a debug message is displayed.  This feature, in optional
            conjunction with the "-1" (ie, all page/block addresses match)
            feature, allows for convenient tracking of operations.

    A pointer to the NsPolicyTableList is obtained via a call to the
    FfxHookOptionGet()
*/
typedef struct
{
    D_CHAR const   * const pTestLabel;
    NsPolicyError    const errorType;
    D_UINT32         const ulMatch;      /* -1 matches any page/block number */
    D_UINT32         const ulThreshold;  /* the number of SUCCESSFUL operations allowed */
    D_UINT32         const ulMaxErrors;  /* Maximum number of failures to report */
    NsPolicyBitError const errorBits;
    D_UINT32               ulCount;      /* defaults to initially 0 by "C" rules */
} NsOperationElement;
typedef struct
{
    D_CHAR const       * const pListName;
    D_CHAR const       * const pUnitName;
    NsOperationElement * const pOpElement;  /* pointer to array */
    D_UINT32             const ulCount;
} NsOperationList;

typedef NsPolicyHandle NsPolicyCreate(FFXDEVHANDLE hDev, NsDevDimensions *pDevDim);
typedef void           NsPolicyDestroy(   NsPolicyHandle hPolicy);
typedef NsPolicyAction NsPolicyBlockInit( NsPolicyHandle hPolicy, NsPolicyBlockParam const *pParams);
typedef NsPolicyAction NsPolicyReadPage(  NsPolicyHandle hPolicy, NsPolicyPageParam  const *pParams);
typedef NsPolicyAction NsPolicyWritePage( NsPolicyHandle hPolicy, NsPolicyPageParam  const *pParams);
typedef NsPolicyAction NsPolicyReadSpare( NsPolicyHandle hPolicy, NsPolicyPageParam  const *pParams);
typedef NsPolicyAction NsPolicyWriteSpare(NsPolicyHandle hPolicy, NsPolicyPageParam  const *pParams);
typedef NsPolicyAction NsPolicyEraseBlock(NsPolicyHandle hPolicy, NsPolicyBlockParam const *pParams);


/*  This structure contains the "V-table" of all the policy layer functions.
*/
typedef struct
{
    NsPolicyCreate     * const Create;
    NsPolicyDestroy    * const Destroy;
    NsPolicyBlockInit  * const BlockInit;
    NsPolicyReadPage   * const ReadPage;
    NsPolicyWritePage  * const WritePage;
    NsPolicyReadSpare  * const ReadSpare;
    NsPolicyWriteSpare * const WriteSpare;
    NsPolicyEraseBlock * const EraseBlock;
    char               * const szName;
} NsPolicyInterfaces;


typedef struct tagNsConfiguration_s {
    D_UINT32            ulNumDevices;
    const D_UINT8       acNandId[NAND_ID_SIZE];
    NsStorageInterfaces *pStorage;
    NsStorageData       *pStorageData;
    NsPolicyInterfaces  *pPolicy;
    NsPolicyData        *pPolicyData;
} NsConfiguration;


/*-------------------------------------------------------------------
    Configurable Interface Layers
-------------------------------------------------------------------*/


/*  Error Policy Interfaces
*/
extern NsPolicyInterfaces NANDSIM_NullPolicy;
extern NsPolicyInterfaces NANDSIM_RandomPolicy;
extern NsPolicyInterfaces NANDSIM_TablePolicy;
extern NsPolicyInterfaces NANDSIM_RealModelPolicy;


/*-------------------------------------------------------------------
    Error Mechanism layer:

    NsMechContext is an internal structure that is created and
    returned by the mechanism layer Create call.  It contains
    any internal variables that layer desires.
-------------------------------------------------------------------*/
typedef struct tagNsMechanismData *NsMechanismHandle;
typedef struct tagNsMechanismData  NsMechanismData;

NsMechanismHandle NsMechanismCreate(FFXDEVHANDLE hDev, D_BOOL *pfNewFlash);
FFXSTATUS         NsMechanismDestroy(      NsMechanismHandle hMech);
FFXSTATUS         NsMechanismEraseBlock(   NsMechanismHandle hMech, D_UINT32 ulBlock);
FFXSTATUS         NsMechanismReadPage(     NsMechanismHandle hMech, D_UINT32 ulPage, D_BUFFER       *pBuffer);
FFXSTATUS         NsMechanismWritePage(    NsMechanismHandle hMech, D_UINT32 ulPage, D_BUFFER const *pBuffer);
FFXSTATUS         NsMechanismReadSpare(    NsMechanismHandle hMech, D_UINT32 ulPage, D_BUFFER       *pBuffer);
FFXSTATUS         NsMechanismWriteSpare(   NsMechanismHandle hMech, D_UINT32 ulPage, D_BUFFER const *pBuffer);
FFXSTATUS         NsMechanismGetParameters(NsMechanismHandle hMech, D_BUFFER *pID, D_UINT32 *pulNumChips);


#endif /* FFXCONF_NANDSUPPORT */

