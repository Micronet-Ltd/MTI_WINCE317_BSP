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

    This header contains macros to manipulate the OMAP 35x GPMC registers,
    for both chips selects and for 8 or 16-bit flash.

    Note that the name of this header might imply that it would go with an
    "ntomap3xx" NTM, however that is not the case.  This header may be used
    an any NTM's hooks module as appropriate, hence the "fh" prefix.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fhomap3xx.h $
    Revision 1.1  2010/07/05 20:06:04Z  garyp
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FHOMAP3XX_H_INCLUDED
#define FHOMAP3XX_H_INCLUDED

/*  These macros require a level of indirection to allow the symbols
    to resolve properly with the correct register size component.

    Note the distinction between the "OMAP35X" and "OMAP3XX" below.
    The former are specified in the hwomap35x.h file, while the 
    latter are used in this header.
*/

#if NAND_DATA_WIDTH_BYTES == 1
  /*  Don't use parens around these numbers or the
      macros below will not resolve correctly.
  */        
  #define OMAP3XX_FLASH_WIDTH      8
#elif NAND_DATA_WIDTH_BYTES == 2
  #define OMAP3XX_FLASH_WIDTH     16
#else
  #error "FFX: fhomap3xx.h: Unsupported NAND_DATA_WIDTH_BYTES value"
#endif


#define GET_OMAP3XX_GPMC_NAND_REG_CS(reg, siz, cs, base, val)                   \
   ((!cs) ? OMAP35X_GPMC_NAND##siz##_##reg##_0(base)  /* "val" is not used */   \
          : OMAP35X_GPMC_NAND##siz##_##reg##_1(base))

#define PUT_OMAP3XX_GPMC_NAND_REG_CS(reg, siz, cs, base, val)                   \
   if(!cs) (OMAP35X_GPMC_NAND##siz##_##reg##_0(base) = (D_UINT##siz##)val);     \
      else (OMAP35X_GPMC_NAND##siz##_##reg##_1(base) = (D_UINT##siz##)val);

#define  OP_OMAP3XX_GPMC_NAND_REG(op, reg, siz, cs, base, val)                  \
       op##_OMAP3XX_GPMC_NAND_REG_CS( reg, siz, cs, base, val)

#define     OMAP3XX_GPMC_NAND_GET(reg, cs, base)                                \
         OP_OMAP3XX_GPMC_NAND_REG(GET, reg, OMAP3XX_FLASH_WIDTH, cs, base, 0)     

#define     OMAP3XX_GPMC_NAND_PUT(reg, cs, base, val)                           \
         OP_OMAP3XX_GPMC_NAND_REG(PUT, reg, OMAP3XX_FLASH_WIDTH, cs, base, val) 



#endif  /* FHOMAP3XX_H_INCLUDED */

