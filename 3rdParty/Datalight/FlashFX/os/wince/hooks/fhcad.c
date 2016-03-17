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

    This module contains the FlashFX Hooks Layer default implementations for:

       FfxHookNTCADCreate()
       FfxHookNTCADDestroy()
       FfxHookNTCADSetChipSelect()
       FfxHookNTCADReadyWait()
       FfxHookNTCADSetChipCommand()
       FfxHookNTCADSetChipCommandNoWait()
       FfxHookNTCADDataIn()
       FfxHookNTCADDataOut()
       FfxHookNTCADStatusIn()
       FfxHookNTCADAddressOut()

    Should these routines need customization for your project, copy this
    module into your Project Directory, make your changes, and modify
    ffxproj.mak to build the new module.

    NOTE:  This is a sample NAND Control Module for use with the ntcad NTM.
           This code provides a non-functional template for developing a NAND
           Control Module for new hardware.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhcad.c $
    Revision 1.6  2009/10/14 01:23:25Z  keithg
    Removed now obsolete fxio.h include file.
    Revision 1.5  2009/07/19 04:40:53Z  garyp
    Merged from the v4.0 branch.  Documentation updates only.
    Revision 1.4  2009/04/08 20:25:10Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2009/02/24 02:53:03Z  glenns
    - Added clarifying comment regarding use of status register rather
      than the READY/~BUSY line for determining device status.
    Revision 1.2  2007/11/03 23:50:11Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2006/07/09 02:39:16Z  Garyp
    Initial revision
    Revision 1.4  2006/07/09 02:39:16Z  Garyp
    Fixed to build cleanly (hopefully).
    Revision 1.3  2006/07/06 02:00:16Z  Garyp
    Updated to use the Create/Destroy metaphor.  Change the return values to
    use standard FFXSTATUS values.
    Revision 1.2  2006/05/08 22:47:35Z  timothyj
    Removed extra return
    Revision 1.1  2006/05/08 18:23:06Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/
#include <flashfx.h>
#include <nandconf.h>
#include <nandctl.h>

typedef struct nand_ctl
{
    /*  Declare the NANDCTL structure here.
    */
    unsigned    placeholder;
} NANDCTL;


/*-------------------------------------------------------------------
    Public: FfxHookNTCADCreate()

    Create a NAND Control Module instance which is associated with
    the ntcad NTM.

    Parameters:
        hDev        - The Device handle

    Return Value:
        Returns a pointer to the NANDCTL structure to use if
        successful, otherwise NULL.
 -------------------------------------------------------------------*/
NANDCTL * FfxHookNTCADCreate(
    FFXDEVHANDLE        hDev)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0), "FfxHookNTCADCreate()\n"));

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
    Public: FfxHookNTCADDestroy()

    Destroy a NAND Control Module instance.

    Parameters:
        pNC         - A pointer to the NANDCTL structure

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADDestroy(
    NANDCTL        *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADDestroy() pNC=%P\n", pNC));

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
    Public: FfxHookNTCADSetChipSelect()

    Select a particular NAND chip.  Any control bits are set to
    zero and all other chips are deselected.

    Parameters:
        pNC      - A pointer to the NANDCTL structure for the device
        ulChip   - The chip number (high bits of linear address)

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADSetChipSelect(
    NANDCTL            *pNC,
    D_UINT32            ulChip,
    CHIP_SELECT_MODE    mode)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipSelect() pNC=%P Chip=%lU Mode=%s\n",
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
    Public: FfxHookNTCADReadyWait()

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
        pNC      - A pointer to the NANDCTL structure for the device

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADReadyWait(
    NANDCTL    *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADReadyWait() pNC=%P\n", pNC));

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
    Public: FfxHookNTCADSetChipCommand()

    Wait for the chip to be ready and then sends a command using
    FfxHookNTCADSetChipCommandNoWait().

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        ucCommand - the command to send

    Return Value:
        Returns an FFXSTATUS value indicating success or failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTCADSetChipCommand(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXSTATUS   ffxStat;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipCommand() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

    DclAssert(pNC);

    ffxStat = FfxHookNTCADReadyWait(pNC);

    if(ffxStat == FFXSTAT_SUCCESS)
        ffxStat = FfxHookNTCADSetChipCommandNoWait(pNC, ucCommand);

    return ffxStat;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTCADSetChipCommandNoWait()

    Send a command to the chip without waiting for status.

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
FFXSTATUS FfxHookNTCADSetChipCommandNoWait(
    NANDCTL    *pNC,
    D_UCHAR     ucCommand)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADSetChipCommandNoWait() pNC=%P Cmd=%02X\n", pNC, (D_UINT16)ucCommand));

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
    Public: FfxHookNTCADDataIn()

    Perform a platform-dependent stream input operation from the
    NAND data port and return the result.

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        pBuffer   - A pointer to a buffer to be filled with the data
                    read from the device
        uCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be read from the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTCADDataIn(
    NANDCTL        *pNC,
    NAND_IO_TYPE   *pBuffer,
    D_UINT16        uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADDataIn() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

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
    Public: FfxHookNTCADDataOut()

    Perform a platform-dependent stream output operation to the
    NAND data port, storing the provided data.

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device
        pBuffer   - A pointer to a buffer containing the data to be
                    sent to the device
        uCount    - Number of NAND_IO_TYPE elements in the buffer
                    to be sent to the device

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTCADDataOut(
    NANDCTL            *pNC,
    const NAND_IO_TYPE *pBuffer,
    D_UINT16            uCount)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADDataOut() pNC=%P pBuff=%P Count=%U\n", pNC, pBuffer, uCount));

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
    Public: FfxHookNTCADStatusIn()

    Perform a single platform-dependent input operation from the
    NAND data port, for a status read, and return the result.

    Parameters:
        pNC       - A pointer to the NANDCTL structure for the device

    Return Value:
        The status data read from the chip.
-------------------------------------------------------------------*/
NAND_IO_TYPE FfxHookNTCADStatusIn(
    NANDCTL    *pNC)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 3, 0),
        "FfxHookNTCADStatusIn() pNC=%P\n", pNC));

    DclAssert(pNC);

    (void)pNC;                  /* unused */

    /*  Replace this with real code...
    */
    DclProductionError();

    return 0;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTCADAddressOut()

    Perform a single platform-dependent output operation to the
    NAND address port, storing the provided address value.

    Parameters:
        pNC      - A pointer to the NANDCTL structure for the device
        address  - the data to store to the address port

    Return Value:
        None.
-------------------------------------------------------------------*/
void FfxHookNTCADAddressOut(
    NANDCTL    *pNC,
    D_UCHAR     address)
{
    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, 0),
        "FfxHookNTCADAddressOut() pNC=%P Addr=%02X\n", pNC, (D_UINT16)address));

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




