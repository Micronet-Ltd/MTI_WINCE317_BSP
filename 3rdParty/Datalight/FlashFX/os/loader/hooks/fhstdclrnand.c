/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2011 Datalight, Inc.
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

    This module contains the FlashFX Hooks Layer default implementations for:

       FfxHookNTSTDCLRNandCreate()
       FfxHookNTSTDCLRNandDestroy()
       FfxHookNTSTDCLRNandSetChipSelect()
       FfxHookNTSTDCLRNandReadyWait()
       FfxHookNTSTDCLRNandSetChipCommand()
       FfxHookNTSTDCLRNandSetChipCommandNoWait()
       FfxHookNTSTDCLRNandDataIn()
       FfxHookNTSTDCLRNandDataOut()
       FfxHookNTSTDCLRNandStatusIn()
       FfxHookNTSTDCLRNandAddressOut()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    NOTE:  This is a sample NAND Control Module for use with the ntstdclrnand
           NTM. This code provides a non-functional template for developing a 
           NAND Control Module for new hardware.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhstdclrnand.c $
    Revision 1.2  2011/11/29 00:34:30Z  glenns
    Be sure revision history is in place. No functional changes.
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>
#include <fhstdclrnand.h>

typedef struct nand_ctl
{
    /*  Declare the NANDCTL structure here.
    */
    unsigned    placeholder;
} NANDCTL;


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandCreate()

    This function creates a NAND Control Module instance which
    is associated with the ntstdclrnand NTM.

    Parameters:
        hDev - The Device handle

    Return Value:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
NANDCTL * FfxHookNTSTDCLRNandCreate(
    FFXDEVHANDLE        hDev)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTSTDCLRNandCreate()\n"));

    DclAssert(hDev);

    (void)hDev;

    /*  Replace this with real code...
    */
    DclProductionError();

    /*  ...typically...
    pNC = DclMemAllocZero(sizeof *pNC);

    ...initialize structure...

    return pNC;
    */

    /*  Aways fail -- this is dummy template code
    */
    return NULL;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandDestroy()

    This function destroys a NAND Control Module instance.

    Parameters:
        pNC - A pointer to the NANDCTL structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandDestroy() pNC=%P\n", pNC));

    DclAssert(pNC);

    (void)pNC;

    /*  Replace this with real code...
    */
    DclProductionError();

    /*  ...typically...
    DclMemFree(pNC);
    */

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandSetChipSelect()

    This function selects a particular NAND chip.  Any control
    bits are set to zero and all other chips are deselected.

    Parameters:
        pNC    - A pointer to the NANDCTL structure for the device
        ulChip - The chip number (high bits of linear address)

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandSetChipSelect(
    NANDCTL            *pNC,
    D_UINT32            ulChip,
    CHIP_SELECT_MODE    mode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandSetChipSelect() pNC=%P Chip=%lU Mode=%s\n",
         pNC, ulChip, mode == MODE_WRITE ? "MODE_WRITE" : "MODE_READ"));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)ulChip;
    (void)mode;

    /*  Replace this with real code...
    */
    DclProductionError();

    /*
    if(all good)
        return FFXSTAT_SUCCESS;
    else
    */
        return FFXSTAT_FIMRANGEINVALID;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandReadyWait()

    Poll the BSY/RDY pin of the NAND chip until it indicates that
    the device is ready or a timeout limit is reached.  This is
    often called before a command is written to the flash to be
    sure the device will listen before the command is sent.  The
    chip selects must/will be set prior to this function call.
    This function could look at the busy pins of the device or
    send a read status command.

    Note that if the read status command is used to check if
    the device is ready when performing read operations, it
    may be necessary to reissue the read command that was used
    in order to get the chip out of the read status mode and
    back into the read data mode. Consult your data sheet for
    details.

    The timeout limit is specified by NAND_TIME_OUT in
    nandconf.h.

    Parameters:
        pNC - A pointer to the NANDCTL structure for the device

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandReadyWait(
    NANDCTL    *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandReadyWait() pNC=%P\n", pNC));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;

    /*  Replace this with real code...
    */
    DclProductionError();

#if 0
    /*  Outline of typical implementation:
    */
    {
        DCLTIMER    t;

        DclTimerSet(&t, NAND_TIME_OUT);

        while(!DclTimerExpired(&t))
        {
            if(chip is ready)
                return FFXSTAT_SUCCESS;
        }

        /*  We MUST have one more check after exiting the while() loop
            because in a multithreaded system we could have timed out
            due to other threads being scheduled, but still completed
            the operation successfully.
        */
        return (chip is still not ready) ? FFXSTAT_FIMTIMEOUT : FFXSTAT_SUCCESS;
    }
    /*  End of typical implementation
    */
#endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandSetChipCommandNoWait()

    Universal "send arbitrary command" helper routine used by all
    higher level chip-specific routines. This routine only takes
    care of making sure that the proper interface pins are
    manipulated so that the command is entered into a previously
    selected chip.  The appropriate chip must already be selected
    before calling this routine and subsequent address,
    confirmation and data bytes must be sent to the chips using
    calls to additional helper routines.

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandSetChipCommandNoWait(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandSetChipCommandNoWait() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)ucCommand;

    /*  Replace this with real code...
    */
    DclProductionError();

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandSetChipCommand()

    Waits for the chip to be ready and then sends a command using
    FfxHookNTSTDCLRNandSetChipCommandNoWait().

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandSetChipCommand(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXSTATUS   ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandSetChipCommand() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    ffxStat = FfxHookNTSTDCLRNandReadyWait(pNC);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTSTDCLRNandSetChipCommandNoWait(pNC, ucCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandSetChipCommandNoWait()

    Universal "send arbitrary command" helper routine used by all
    higher level chip-specific routines. This routine only takes
    care of making sure that the proper interface pins are
    manipulated so that the command is entered into a previously
    selected chip.  The appropriate chip must already be selected
    before calling this routine and subsequent address,
    confirmation and data bytes must be sent to the chips using
    calls to additional helper routines.

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTSTDCLRNandSetChipCommandNoWait(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandSetChipCommandNoWait() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)ucCommand;

    /*  Replace this with real code...
    */
    DclProductionError();

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandDataIn()

    Performs a platform-dependent stream input operation from the
    NAND data port and returns the result.

    Parameters:
        pNC     - A pointer to the NANDCTL structure for the device
        pBuffer - A pointer to a buffer to be filled with the data
                  read from the device
        uCount  - Number of NAND_IO_TYPE elements in the buffer
                  to be read from the device

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookNTSTDCLRNandDataIn(
    NANDCTL        *pNC,
    NAND_IO_TYPE   *pBuffer,
    D_UINT16        uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTSTDCLRNandDataIn() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

    DclAssert(pNC);
    DclAssert(pBuffer);
    DclAssert(uCount);

    (void)pNC;                  /* unused */
    (void)pBuffer;              /* unused */
    (void)uCount;               /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandDataOut()

    Performs a platform-dependent stream output operation to the
    NAND data port, storing the provided data.

    Parameters:
        pNC     - A pointer to the NANDCTL structure for the device
        pBuffer - A pointer to a buffer containing the data to be
                  sent to the device
        uCount  - Number of NAND_IO_TYPE elements in the buffer
                  to be sent to the device

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookNTSTDCLRNandDataOut(
    NANDCTL            *pNC,
    const NAND_IO_TYPE *pBuffer,
    D_UINT16            uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTSTDCLRNandDataOut() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

    DclAssert(pNC);
    DclAssert(pBuffer);
    DclAssert(uCount);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)pBuffer;
    (void)uCount;

    return;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandStatusIn()

    Performs a single platform-dependent input operation from the
    NAND data port, for a status read, and returns the result.

    Parameters:
        pNC - A pointer to the NANDCTL structure for the device

    Return Value:
        The status data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTSTDCLRNandStatusIn(
    NANDCTL    *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTSTDCLRNandStatusIn() pNC=%P\n", pNC));

    DclAssert(pNC);

    (void)pNC;                  /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

    return 0;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTSTDCLRNandAddressOut()

    Performs a single platform-dependent output operation to the
    NAND address port, storing the provided address value.

    Parameters:
        pNC     - A pointer to the NANDCTL structure for the device
        address - the data to store to the address port

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookNTSTDCLRNandAddressOut(
    NANDCTL    *pNC,
    D_UCHAR     address)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTSTDCLRNandAddressOut() pNC=%P Addr=%02X\n", pNC, (D_UINT16)address));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)address;

    /*  Replace this with real code...
    */
    DclProductionError();

    return;
}




