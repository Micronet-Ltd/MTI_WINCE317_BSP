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
  jurisdictions.  Patents may be pending.

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

    This header contains prototypes for the CPU layer of the Datalight
    Common Libraries.

    Note that the CPU layer is almost never directly accessed, but rather
    is used indirectly through the OS Services routines.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                 Revision History
    $Log: dlcpuapi.h $
    Revision 1.5  2009/10/17 23:17:07Z  garyp
    Added and removed prototypes.
    Revision 1.4  2009/09/25 17:21:28Z  garyp
    Added prototypes.
    Revision 1.3  2009/02/08 01:05:44Z  garyp
    Added prototypes.
    Revision 1.2  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.1  2005/10/02 02:04:36Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

void            DclCpuFlushICache(void);
D_UINT32        DclCpuGetTickCount(void);
D_UINT32        DclCpuGetTickModulus(void);
D_UINT32        DclCpuGetTickResolution(void);
D_UINT32        DclCpuInterruptDisable(void);
void            DclCpuInterruptRestore(D_UINT32);
unsigned        DclCpuIsMmuOn(void);
D_INT16         DclCpuMemCmp16(const void *pM1, const void *pM2, D_UINT16 uLen);
void *          DclCpuMemCpy16(void *pDest, const void *pSrc, D_UINT16 uLen);
void *          DclCpuMemSet16(void *pMem, D_UCHAR cVal, D_UINT16 uLen);
void            DclCpuMemCpyAligned32_32(void *pDest, const void *pSource, D_UINT32 ulChunks);
D_UCHAR         DclCpuSerialGetChar(D_UINT16 uIOPort);
void            DclCpuSerialPutChar(D_UINT16 uIOPort, D_UCHAR uChar);
DCLSTATUS       DclCpuSerialInit(   D_UINT16 uIOPort, D_UINT16 uBaudRateDivisor);


/*  Note that the following functions are used for diagnostics and
    debugging.  The take parameters in registers and are typically
    only used for debugging other assembly language code.
*/
void            DclCpuDisplayChar(void);
void            DclCpuDisplayString(void);
void            DclCpuDisplayHexByte(void);
void            DclCpuDisplayHexWord(void);

