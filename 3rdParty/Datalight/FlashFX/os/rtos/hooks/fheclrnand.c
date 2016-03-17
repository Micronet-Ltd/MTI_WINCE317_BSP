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

       FfxHookNTECLRNandCreate()
       FfxHookNTECLRNandDestroy()
       FfxHookNTECLRNandSetChipSelect()
       FfxHookNTECLRNandReadyWait()
       FfxHookNTECLRNandSetChipCommand()
       FfxHookNTECLRNandSetChipCommandNoWait()
       FfxHookNTECLRNandDataIn()
       FfxHookNTECLRNandDataOut()
       FfxHookNTECLRNandStatusIn()
       FfxHookNTECLRNandAddressOut()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    NOTE:  This is a sample NAND Control Module for use with the nteclrnand
           NTM. This code provides a non-functional template for developing a
           NAND Control Module for new hardware.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fheclrnand.c $
    Revision 1.1  2011/12/14 21:28:44Z  glenns
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>
#include <nandcmd.h>
#include <fheclrnand.h>

typedef struct nand_ctl
{
    /*  Declare the NANDCTL structure here.
    */
    unsigned    placeholder;
} NANDCTL;


/*-------------------------------------------------------------------
    Public: FfxHookNTECLRNandCreate()

    This function creates a NAND Control Module instance which
    is associated with the nteclrnand NTM.

    Parameters:
        hDev - The Device handle

    Return Value:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
NANDCTL * FfxHookNTECLRNandCreate(
    FFXDEVHANDLE        hDev)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTECLRNandCreate()\n"));

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
    Public: FfxHookNTECLRNandDestroy()

    This function destroys a NAND Control Module instance.

    Parameters:
        pNC - A pointer to the NANDCTL structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTECLRNandDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandDestroy() pNC=%P\n", pNC));

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
    Public: FfxHookNTECLRNandSetChipSelect()

    This function selects a particular NAND chip.  Any control
    bits are set to zero and all other chips are deselected.

    Parameters:
        pNC    - A pointer to the NANDCTL structure for the device
        ulChip - The chip number (high bits of linear address)

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTECLRNandSetChipSelect(
    NANDCTL            *pNC,
    D_UINT32            ulChip,
    CHIP_SELECT_MODE    mode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandSetChipSelect() pNC=%P Chip=%lU Mode=%s\n",
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
    Public: FfxHookNTECLRNandReadyWait()

    Poll the status indications of the Enhanced ClearNAND chip
    until it indicates that the device is in the state requested
    or timeout limit is reached.  Enhanced ClearNAND has multiple
    status indications and multiple commands for reading these
    indications so the command to get status and what status
    to look for must be passed as parameters to this function.

    Note that if one of the status commands is used to check if
    the device is ready when performing read operations, it
    may be necessary to reissue the read command that was used
    in order to get the chip out of the read status mode and
    back into the read data mode. Consult your data sheet for
    details.

    The timeout limit is specified by NAND_TIME_OUT in
    nandconf.h.

    Parameters:
        pNC         - A pointer to the NANDCTL structure for the
                      device
        ucWaitFor   - Status code to look for
        ucCommand   - command to be used to access the status
                      register containing the status codes.

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTECLRNandReadyWait(
    NANDCTL    *pNC,
    D_UCHAR     ucWaitFor,
    D_UCHAR     ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandReadyWait() pNC=%P\n", pNC));

    DclAssert(pNC);

    /*  Avoid compiler warnings
    */
    (void)pNC;
    (void)ucWaitFor;
    (void)ucCommand;

    /*  Replace this with real code...
    */
    DclProductionError();

#if 0
    /*  Outline of typical implementation:
    */
    {
        DCLTIMER    t;
        FFXSTATUS   retval = FFXSTAT_FIMTIMEOUT;
        D_UCHAR     temp;

        DclTimerSet(&t, NAND_TIME_OUT);

        while(!DclTimerExpired(&t))
        {
            FfxHookNTECLRNandSetChipCommandNoWait(pNC, ucCommand);
            temp = FfxHookNTECLRNandStatusIn(pNC);

            if ((((D_UCHAR)temp) & ucWaitFor) == ucWaitFor)
            {
                retval = FFXSTAT_SUCCESS;
                break;
            }
        }

        /*  We MUST have one more check after exiting the while() loop
            because in a multithreaded system we could have timed out
            due to other threads being scheduled, but still completed
            the operation successfully.
       */
        if (retval != FFXSTAT_SUCCESS)
        {
            temp = FfxHookNTECLRNandStatusIn(pNC);
            if ((((D_UCHAR)temp) & ucWaitFor) == ucWaitFor)
                retval = FFXSTAT_SUCCESS;
        }
        if (retval == FFXSTAT_SUCCESS)
        {
            /*  If we read a status register during a page read or
                random read operation, we have to eissue the read
                command to resume it: 
            */

            if (lastCommand == CMD_READ_MAIN ||
                lastCommand == CMD_READ_SPARE)
                *(pNC->pNANDChipCommandLatch) = lastCommand;
        }
        
        return retval;
    }
    /*  End of typical implementation
    */
#endif

    return FFXSTAT_SUCCESS;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTECLRNandSetChipCommandNoWait()

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
FFXSTATUS FfxHookNTECLRNandSetChipCommandNoWait(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandSetChipCommandNoWait() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

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
    Public: FfxHookNTECLRNandSetChipCommand()

    Waits for the chip to be ready and then sends a command using
    FfxHookNTECLRNandSetChipCommandNoWait().

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTECLRNandSetChipCommand(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXSTATUS   ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandSetChipCommand() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    ffxStat = FfxHookNTECLRNandReadyWait(pNC, DEVICE_READY, CMD_DEV_STATUS);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTECLRNandSetChipCommandNoWait(pNC, ucCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTECLRNandDataIn()

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
void FfxHookNTECLRNandDataIn(
    NANDCTL        *pNC,
    NAND_IO_TYPE   *pBuffer,
    D_UINT16        uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTECLRNandDataIn() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

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
    Public: FfxHookNTECLRNandDataOut()

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
void FfxHookNTECLRNandDataOut(
    NANDCTL            *pNC,
    const NAND_IO_TYPE *pBuffer,
    D_UINT16            uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTECLRNandDataOut() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

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
    Public: FfxHookNTECLRNandStatusIn()

    Performs a single platform-dependent input operation from the
    NAND data port, for a status read, and returns the result.

    Parameters:
        pNC - A pointer to the NANDCTL structure for the device

    Return Value:
        The status data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTECLRNandStatusIn(
    NANDCTL    *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTECLRNandStatusIn() pNC=%P\n", pNC));

    DclAssert(pNC);

    (void)pNC;                  /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

    return 0;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTECLRNandAddressOut()

    Performs a single platform-dependent output operation to the
    NAND address port, storing the provided address value.

    Parameters:
        pNC     - A pointer to the NANDCTL structure for the device
        address - the data to store to the address port

    Return Value:
        None
-------------------------------------------------------------------*/
void FfxHookNTECLRNandAddressOut(
    NANDCTL    *pNC,
    D_UCHAR     address)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTECLRNandAddressOut() pNC=%P Addr=%02X\n", pNC, (D_UINT16)address));

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




