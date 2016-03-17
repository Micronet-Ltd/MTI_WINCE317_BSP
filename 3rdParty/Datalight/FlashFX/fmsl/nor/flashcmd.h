/*---------------------------------------------------------------------------
               ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 1993 - 2007 Datalight, Inc.
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

    This module contains generic macros for manipulating flash memory in
    a variety of configurations.

    Prior to including this header, the following symbols must be defined:

    FLASH_BUS_WIDTH  - This is the flash data bus with in bits.  Value of
                       8, 16, and 32 are currently supported.
    FLASH_INTERLEAVE - This is the number of flash chips that are inter-
                       leaved to create the flash data elements of the
                       FLASH_BUS_WIDTH bits.  A value of 1 indicates that
                       the flash is not interleaved.

    From these values, the following information is derived:

    FLASH_CHIP_WIDTH - This is the width of each individual flash chip
                       in bits (FLASH_BUS_WIDTH / FLASH_INTERLEAVE).
    FLASH_BUS_BYTES  - This is the width of the flash data bus in bytes
                       (FLASH_BUS_WIDTH / 8).
    FLASHDATA        - This is a data type that is the native flash data 
                       element size.  For FLASH_BUS_WIDTHS of 8, 16, or 32
                       the FLASHDATA type will end up being D_UCHAR, D_UINT16,
                       or D_UINT32, respectively.
    PFLASHDATA       - This is defined as a volatile pointer to the 
                       FLASHDATA type.
    FLASHID          - This is a data type that is used to hold the
                       manufacturer and device ID.  This will be the
                       native flash data element size FOR A GIVEN CHIP.
                       For FLASH_CHIP_WIDTH values of 8, 16, or 32 bits,
                       the FLASHID will end up being a D_UCHAR, D_UINT16,
                       or D_UINT32, respectively.
    FLASHIDCODES     - This is a structure used to hold flash ID information. 
    GETFLASHIDCODES  - This is a macro that is used to retrieve flash ID
                       codes into a FLASHIDCODES structure.
    MAKEFLASHCMD     - This is a macro that is used to generate flash 
                       commands that may vary for individual chips.
    MAKEFLASHDUPCMD  - This is a macro that is used to generate flash 
                       commands that are identical across all chips.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                              Revision History
    $Log: flashcmd.h $
    Revision 1.3  2007/11/03 23:49:37Z  Garyp
    Updated to use the standard module header.
    Revision 1.2  2006/08/31 05:30:40Z  pauli
    Updated comments.
    Revision 1.1  2005/10/02 01:33:20Z  Pauli
    Initial revision
    Revision 1.1  2005/10/02 02:33:20Z  Garyp
    Initial revision
    Revision 1.1  2005/10/02 02:33:20Z  Garyp
    Initial revision
    Revision 1.1  2005/05/10 16:26:18Z  pauli
    Initial revision
    Revision 1.1  2005/05/10 16:26:18Z  pauli
    Initial revision
    Revision 1.2  2005/05/10 16:26:18Z  billr
    Add parentheses around macro parameters to enforce function-like
    evaluation.
    Revision 1.1  2005/05/08 17:45:48Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FLASHCMD_H_INCLUDED
#define FLASHCMD_H_INCLUDED

#ifndef FLASH_BUS_WIDTH
    #error "FFX: FLASH_BUS_WIDTH is not defined"
#endif

#ifndef FLASH_INTERLEAVE
    #error "FFX: FLASH_INTERLEAVE is not defined"
#endif

#define FLASH_CHIP_WIDTH                (FLASH_BUS_WIDTH / FLASH_INTERLEAVE)
#define FLASH_BUS_BYTES                 (FLASH_BUS_WIDTH / 8)

#if FLASH_BUS_WIDTH == 8
    typedef D_UCHAR                     FLASHDATA;
    typedef volatile D_UCHAR *          PFLASHDATA;
#elif FLASH_BUS_WIDTH == 16
    typedef D_UINT16                      FLASHDATA;
    typedef volatile D_UINT16 *           PFLASHDATA;
#elif FLASH_BUS_WIDTH == 32
    typedef D_UINT32                   FLASHDATA;
    typedef volatile D_UINT32 *        PFLASHDATA;
#else
    #error "FFX: Unsupported FLASH_BUS_WIDTH value"
#endif

#if FLASH_CHIP_WIDTH == 8
    typedef D_UCHAR                     FLASHID;
    #define FLASHCHIP_MASK              0xFF
#elif FLASH_CHIP_WIDTH == 16
    typedef D_UINT16                      FLASHID;
    #define FLASHCHIP_MASK              0xFFFF
#elif FLASH_CHIP_WIDTH == 32
    typedef D_UINT32                   FLASHID;
    #define FLASHCHIP_MASK              0xFFFFFFFFUL
#else
    #error "FFX: Unsupported chip width"
#endif


/*---------------------------------------------------------
    Macro primitives for getting and setting values for 
    individual chips in an interleaved flash array.
---------------------------------------------------------*/    
#define GETCHIPN(x, n)           ((((FLASHDATA)(x)) >> (FLASH_CHIP_WIDTH*(n))) & FLASHCHIP_MASK)
#define SETCHIPN(x, n)           ((FLASHDATA)(x) << (FLASH_CHIP_WIDTH*(n)))


/*---------------------------------------------------------
    This macro changes a single command for a given chip 
    in a multi-chip command, without affecting the other
    commands.
---------------------------------------------------------*/
#define CHANGECHIPCMD(x, n, new) ((((FLASHDATA)(x)) & (~(((FLASHDATA)FLASHCHIP_MASK) << (FLASH_CHIP_WIDTH*(n))))) | SETCHIPN((new), (n)))


/*---------------------------------------------------------
    Macros for building commands/status masks for one or
    more chips.
---------------------------------------------------------*/
#if FLASH_INTERLEAVE == 1
    #define GETCHIP0(x)                  GETCHIPN((x), 0)
    #define SETCHIP0(x)                  SETCHIPN((x), 0)
    #define MAKEMASK(x)                  (SETCHIP0(x))
    #define MAKEFLASHCMD(x)              MAKEMASK(x)
    #define MAKEFLASHDUPCMD(x)           MAKEMASK(x)
    #define TRANSFORMSTATUS(pX, old, new)                                   \
    do                                                                      \
    {                                                                       \
        if(GETCHIP0(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 0, (new)); \
    } while(0)    
    
    #define ISCOMPLETETRANSFORMATION(x, cmd1, cmd2)                         \
        (GETCHIP0(x) == (cmd1) || GETCHIP0(x) == (cmd2))
    
#elif FLASH_INTERLEAVE == 2
    #define GETCHIP0(x)                  GETCHIPN((x), 0)
    #define GETCHIP1(x)                  GETCHIPN((x), 1)
    #define SETCHIP0(x)                  SETCHIPN((x), 0)
    #define SETCHIP1(x)                  SETCHIPN((x), 1)
    #define MAKEMASK(c1, c0)             (SETCHIP1(c1) | SETCHIP0(c0))
    #define MAKEFLASHCMD(c1, c0)         MAKEMASK(c1, c0)
    #define MAKEFLASHDUPCMD(x)           MAKEMASK(x, x)
    #define TRANSFORMSTATUS(pX, old, new)                                \
    do                                                                   \
    {                                                                    \
        if(GETCHIP0(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 0, (new)); \
        if(GETCHIP1(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 1, (new)); \
    } while(0)

    #define ISCOMPLETETRANSFORMATION(x, cmd1, cmd2)                 \
        (((GETCHIP0(x) == (cmd1)) || (GETCHIP0(x) == (cmd2))) &&  \
        ((GETCHIP1(x) == (cmd1)) || (GETCHIP1(x) == (cmd2))))

#elif FLASH_INTERLEAVE == 4
    #define GETCHIP0(x)                  GETCHIPN((x), 0)
    #define GETCHIP1(x)                  GETCHIPN((x), 1)
    #define GETCHIP2(x)                  GETCHIPN((x), 2)
    #define GETCHIP3(x)                  GETCHIPN((x), 3)
    #define SETCHIP0(x)                  SETCHIPN((x), 0)
    #define SETCHIP1(x)                  SETCHIPN((x), 1)
    #define SETCHIP2(x)                  SETCHIPN((x), 2)
    #define SETCHIP3(x)                  SETCHIPN((x), 3)
    #define MAKEMASK(c3, c2, c1, c0)     (SETCHIP3(c3) | SETCHIP2(c2) | SETCHIP1(c1) | SETCHIP0(c0))
    #define MAKEFLASHCMD(c3, c2, c1, c0) MAKEMASK(c3, c2, c1, c0)
    #define MAKEFLASHDUPCMD(x)           MAKEMASK(x, x, x, x)
    #define TRANSFORMSTATUS(pX, old, new)                                \
    do                                                                   \
    {                                                                    \
        if(GETCHIP0(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 0, (new)); \
        if(GETCHIP1(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 1, (new)); \
        if(GETCHIP2(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 2, (new)); \
        if(GETCHIP3(*(pX)) == (old)) *(pX) = CHANGECHIPCMD(*(pX), 3, (new)); \
    } while(0)    

    #define ISCOMPLETETRANSFORMATION(x, cmd1, cmd2)                   \
        ((GETCHIP0(x) == (cmd1) || GETCHIP0(x) == (cmd2)) &&          \
        (GETCHIP1(x) == (cmd1) || GETCHIP1(x) == (cmd2)) &&           \
        (GETCHIP2(x) == (cmd1) || GETCHIP2(x) == (cmd2)) &&           \
        (GETCHIP3(x) == (cmd1) || GETCHIP3(x) == (cmd2)))
#else
    #error "FFX: Unsupported FLASH_INTERLEAVE value"
#endif


typedef struct
{
    FLASHID     idMfg;
    FLASHID     idDev;
    FLASHDATA   data0;
    FLASHDATA   data1;
} FLASHIDCODES, * PFLASHIDCODES;

#define GETFLASHIDCODES(pID, pvM)                                           \
do                                                                          \
{                                                                           \
    ((PFLASHIDCODES)pID)->data0 = *pvM;                                     \
    ((PFLASHIDCODES)pID)->data1 = *(pvM+1);                                 \
    ((PFLASHIDCODES)pID)->idMfg = (FLASHID)((PFLASHIDCODES)pID)->data0;     \
    ((PFLASHIDCODES)pID)->idDev = (FLASHID)((PFLASHIDCODES)pID)->data1;     \
} while(0)

/*---------------------------------------------------------
    The following macros assume that on any given flash 
    chip, the manufacturer's ID and the device ID will
    never happen to be identical.
---------------------------------------------------------*/
#if FLASH_INTERLEAVE == 1

    #define ISVALIDFLASHIDCODE(pID)                                          \
        (((PFLASHIDCODES)pID)->idMfg != ((PFLASHIDCODES)pID)->idDev) 

#elif FLASH_INTERLEAVE == 2

    #define ISVALIDFLASHIDCODE(pID)                                                 \
        ((((PFLASHIDCODES)pID)->idMfg != ((PFLASHIDCODES)pID)->idDev) &&         \
        (GETCHIP1(((PFLASHIDCODES)pID)->data0) == ((PFLASHIDCODES)pID)->idMfg) && \
        (GETCHIP1(((PFLASHIDCODES)pID)->data1) == ((PFLASHIDCODES)pID)->idDev))

#elif FLASH_INTERLEAVE == 4

    #define ISVALIDFLASHIDCODE(pID)                                                 \
        ((((PFLASHIDCODES)pID)->idMfg != ((PFLASHIDCODES)pID)->idDev) &&         \
        (GETCHIP1(((PFLASHIDCODES)pID)->data0) == ((PFLASHIDCODES)pID)->idMfg) && \
        (GETCHIP2(((PFLASHIDCODES)pID)->data0) == ((PFLASHIDCODES)pID)->idMfg) && \
        (GETCHIP3(((PFLASHIDCODES)pID)->data0) == ((PFLASHIDCODES)pID)->idMfg) && \
        (GETCHIP1(((PFLASHIDCODES)pID)->data1) == ((PFLASHIDCODES)pID)->idDev) && \
        (GETCHIP2(((PFLASHIDCODES)pID)->data1) == ((PFLASHIDCODES)pID)->idDev) && \
        (GETCHIP3(((PFLASHIDCODES)pID)->data1) == ((PFLASHIDCODES)pID)->idDev))
 #endif
    
 
#endif  /* FLASHCMD_H_INCLUDED */

