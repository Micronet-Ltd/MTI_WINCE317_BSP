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

    This module contains the implementations for:

       FfxHookNTMX31Create()
       FfxHookNTMX31Destroy()
       FfxHookNTMX31SetParameters()
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhmx31.c $
    Revision 1.5  2009/10/14 22:44:58Z  keithg
    Removed now obsolete fxio.h include file; Changed MEM_PUT...
    macros to the new ones DCLMEM... macros.
    Revision 1.4  2009/04/08 19:43:54Z  davidh
    Function headers updated for AutoDoc.
    Revision 1.3  2009/03/26 22:44:16Z  billr
    Fix unbalanced trace indent/undent.
    Revision 1.2  2007/11/03 23:49:54Z  Garyp
    Updated to use the standard module header.
    Revision 1.1  2007/05/07 23:58:12Z  brandont
    Initial revision
    Revision 1.1  2007/05/07 23:58:12Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#include <flashfx.h>
#include <oecommon.h>
#include <nandconf.h>
#include <nandctl.h>


/*  Private constants
*/

/*  CCM register physical base addresss
*/
#define CCM_REGS_PHYS_BASE      (0x53F80000)
#define CCM_REGS_EXTENT         (0x64)

/*  Private data - there is only one NFC in the MX31, so using a static/global
    is not only reasonable, but preferred.
*/
/* Note this is currently initialized to NULL, because it is not known
   whether the project-specific system on which this code is running
   requires an MMU, and therefore the DclAsserts() below will fire.
*/
static void *pCCMBase = NULL;

/* CCM registers and bit fields
*/
#define CCMREG(offset)   ((D_UINT32 volatile *) ((char *) pCCMBase + offset))

#define CCM_RCSR                CCMREG(0x0C)
#define RCSR_NF16B              (1U << 31)
#define RCSR_NFMS               (1U << 30)



/*-------------------------------------------------------------------
    Public: FfxHookNTMX31Create()

    Allocate (if necessary) and inialize the private control
    structure.  Typically this will involve mapping the
    MX31 CCM registers into the address space of the
    calling process.

    Parameters:
        hDev - The device handle.

    Return Value:
        Returns a non-NULL opaque private pointer for this module,
        or NULL on failure.
-------------------------------------------------------------------*/
PNANDCTL FfxHookNTMX31Create(
    FFXDEVHANDLE       hDev)
{

    (void)hDev;

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEINDENT),
                    "FfxHookNTMX31Create()\n"));

    /*  This would be a good location to map the pointer pCCMBase
        to CCM_REGS_PHYS_BASE, if required, or assign it, if mapping
        is not required.

        If mapping is required (ie. if your system has an MMU), add the
        code to do so and remove the assertion below.

        If mapping is NOT required, you can probably replace the
        assertion below with the following line of code:


        pCCMBase = (void *)CCM_REGS_PHYS_BASE;


    */
    DclError();

    FFXTRACEPRINTF((MAKETRACEFLAGS(FFXTRACE_NTMHOOK, 1, TRACEUNDENT),
        "FfxHookNTMX31Create() returning MX31 Control: %lX\n", pCCMBase));

    return (PNANDCTL)pCCMBase;
}


/*-------------------------------------------------------------------
    Public: FfxHookNTMX31Destroy()

    Deallocate (if necessary) and deinialize the
    MX31 control structure and any relevant hardware shutdown.
    Typically this will involve unmapping the CCM registers.

    Parameters:
        pMX31Ctl - pointer to the MX31 control structure, private
                   to this module.

    Return Value:
        An FFXSTATUS value indicating success or the nature of
        any failure.
-------------------------------------------------------------------*/
FFXSTATUS FfxHookNTMX31Destroy(PNANDCTL pNC)
{
    DclAssert(pNC);
    DclAssert(pCCMBase);
    DclAssert(pNC == pCCMBase);

    /*  This would be a good location to un-map the pointer pCCMBase
    */

    return FFXSTAT_SUCCESS;
}

/*-------------------------------------------------------------------
    Public: FfxHookNTMX31SetParameters()

    Configure the MX31 with the chip parameters.  Typically
    this only involves programming the RCSR register in
    a fashion that is synchronized with other system
    components that may be accessing the RCSR register.

    NOTE!   There is a timing hole here that cannot
            be closed solely within the module that calls this
            function.

            System software MUST ensure that this module synchronizes
            access to the RCSR register with any other component that
            may be accessing the RCSR register.

    Parameters:
        pNC          - A pointer to the private NANDCTL structure
        ucWidthBytes - number of bytes of width of the NAND device
                       data bus (1 - x8, 2 - x16)
        uPageSize    - page size of the NAND device

    Return Value:
        Returns TRUE if the system is configured for the width
        and page size specified.
-------------------------------------------------------------------*/
D_BOOL FfxHookNTMX31SetParameters(
    PNANDCTL        pNC,
    D_UCHAR         ucByteWidth,
    D_UINT16        ucPageSize)
{
    D_UINT32    ulRCSRVal;

    DclAssert(pNC == pCCMBase);

    ulRCSRVal = DCLMEMGET32(CCM_RCSR);

    /*  set or clear the bit indicating 16b wide data bus
        for the NFC.
    */
    if (ucByteWidth == 1)
    {
        ulRCSRVal &= ~RCSR_NF16B;
    }
    else if (ucByteWidth == 2)
    {
        ulRCSRVal |= RCSR_NF16B;
    }
    else
    {
        DclError();
        return FALSE;
    }

    if (ucPageSize == 512)
    {
        ulRCSRVal &= ~RCSR_NFMS;
    }
    else if (ucPageSize == 2048)
    {
        ulRCSRVal |= RCSR_NFMS;
    }
    else
    {
        DclError();
        return FALSE;
    }

    DCLMEMPUT32(CCM_RCSR, ulRCSRVal);

    return TRUE;
}


