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

    Interface to low-level OneNAND flash control functions.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: onenandctl.h $
    Revision 1.10  2009/12/13 01:36:33Z  garyp
    Eliminated the definition of NTMHOOKHANDLE.
    Revision 1.9  2009/08/04 03:17:00Z  garyp
    Merged from the v4.0 branch.  Added the FFXONENANDPARAMS structure, and
    updated the interface to allow the Hook module's Create() function to
    communicate configuration parameters back to the OneNAND NTM at run-time.
    Revision 1.8  2008/09/29 19:52:56Z  glenns
    Added support for FlexOneNAND to be sure main page data is blank in the
    page register when doing spare-only writes.
    Revision 1.7  2007/11/03 23:49:39Z  Garyp
    Updated to use the standard module header.
    Revision 1.6  2006/10/06 00:21:25Z  Garyp
    Modified to use the DCL specific byte-ordering macros.
    Revision 1.5  2006/03/09 21:00:10Z  timothyj
    Removed unused unnecessary ONENANDINFO parameter.
    Revision 1.4  2006/03/08 22:49:32Z  timothyj
    Removed extra comment after verifying chip ID size.
    Revision 1.3  2006/03/06 17:25:32Z  Garyp
    Changed a reference from NAND_LITTLE_ENDIAN to be OneNAND specific.
    Revision 1.2  2006/02/10 23:42:46Z  Garyp
    Started the process of factoring things into Devices and Disks.
    Work-in-progress, not completed yet.
    Revision 1.1  2006/02/03 20:59:54Z  timothyj
    Initial revision
---------------------------------------------------------------------------*/

#ifndef ONENANDCTL_H_INCLUDED
#define ONENANDCTL_H_INCLUDED

/*  Define the time-out value in milliseconds.  Default is 20ms for both
    write and erase; that is twice the maximum specified time for any
    supported NAND chip to do anything we could sensibly ask of it...
*/
#define ONENAND_TIME_OUT         20


/*  These macros implement different types of operations based
    on the OneNAND I/O width.
*/
#if (ONENAND_DATA_WIDTH_BYTES == 1)
  #define ONENAND_IO_TYPE           D_UINT8
  #define ONENAND_BLANKDATA         (ONENAND_IO_TYPE)0xFF
  #define BENDSWAP(cmd)             (cmd)
#elif (ONENAND_DATA_WIDTH_BYTES == 2)
  #define ONENAND_IO_TYPE           D_UINT16
  #define ONENAND_BLANKDATA         (ONENAND_IO_TYPE)0xFFFF
  #if ((!DCL_BIG_ENDIAN) ^ ONENAND_LITTLE_ENDIAN)
    #define BENDSWAP(cmd)           BENDSWAP16(cmd)
  #endif
#else
  #error "FFX: Unsupported ONENAND_DATA_WIDTH_BYTES value"
#endif


/*--------------------------------------------------------------
    Structure: FFXONENANDPARAMS

    This structure contains OneNAND configuration parameters.
    It is filled in by the FfxHookNTOneNANDCreate() function.

    The fUseSynchBurstMode field indicates whether synchronous
    burst mode should be used or not.  System performance will
    vary depending on the CPU speed, bus access times, etc.
    This value should be tuned based on actual performance in
    the system under test, and MUST match the capabilities of
    the bus to which the OneNAND device is connected.
--------------------------------------------------------------*/
typedef struct
{
    unsigned    nStructLen;
    unsigned    nTimeoutMS;             /* OneNAND time value in milliseconds */
    unsigned    nIOWidth;               /* I/O element size */
    unsigned    nAlignmentBoundary;     /* Minimum client buffer alignment size (bytes) */
    unsigned    fUseHardwareECC : 1;    /* TRUE or FALSE... */
    unsigned    fUseSynchBurstMode : 1; /* TRUE or FALSE... */
} FFXONENANDPARAMS;


NTMHOOKHANDLE   FfxHookNTOneNANDCreate(       FFXDEVHANDLE hDev, FFXONENANDPARAMS *pONP);
FFXSTATUS       FfxHookNTOneNANDDestroy(      NTMHOOKHANDLE hNtmHook);
FFXSTATUS       FfxHookNTOneNANDReadyWait(    NTMHOOKHANDLE hNtmHook);
/*FFXSTATUS       FfxHookNTOneNANDBusyWait(    NTMHOOKHANDLE hNtmHook);
*/
ONENAND_IO_TYPE FfxHookNTOneNANDReadRegister( NTMHOOKHANDLE hNtmHook, D_UINT16 uOffset);
void            FfxHookNTOneNANDReadBuffer(   NTMHOOKHANDLE hNtmHook, void *pBuffer, D_UINT16 uOffset, D_UINT16 uCount);
void            FfxHookNTOneNANDWriteBuffer(  NTMHOOKHANDLE hNtmHook, const void *pBuffer, D_UINT16 uOffset, D_UINT16 uCount);
void            FfxHookNTOneNANDWriteRegister(NTMHOOKHANDLE hNtmHook, D_UINT16 uOffset, ONENAND_IO_TYPE Value);


#if D_DEBUG
    void FfxNtmOneNANDDumpRegisters(NTMHOOKHANDLE hHook);
#endif


#endif  /* ONENANDCTL_H_INCLUDED */

