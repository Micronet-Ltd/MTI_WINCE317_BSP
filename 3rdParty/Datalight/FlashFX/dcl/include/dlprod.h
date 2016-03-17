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
  jurisdictions. 

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

  Notwithstanding the foregoing, Licensee acknowledges that the software may
  be distributed as part of a package containing, and/or in conjunction with
  other source code files, licensed under so-called "open source" software 
  licenses.  If so, the following license will apply in lieu of the terms set
  forth above:

  Redistribution and use of this software in source and binary forms, with or
  without modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions, and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions, and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  
  THIS SOFTWARE IS PROVIDED BY DATALIGHT "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
  CHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT, ARE
  DISCLAIMED.  IN NO EVENT SHALL DATALIGHT BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEG-
  LIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Description

    This header defines product information.  This is the only code in DCL
    which knows anything about the various Datalight products used with DCL.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: dlprod.h $
    Revision 1.17  2010/09/09 20:14:40Z  garyp
    Updated FlashFX Pro to FlashFX Tera.
    Revision 1.16  2010/05/04 16:28:11Z  billr
    Change DCLFUNC() macro to enable exporting these functions from a
    module.
    Revision 1.15  2010/02/13 20:37:41Z  garyp
    Updated to use the correct license wording.
    Revision 1.14  2010/02/10 23:38:54Z  billr
    Changed the headers to reflect the shared or public license. [jimmb]
    Revision 1.13  2009/11/11 22:23:40Z  garyp
    Restored a comment deleted in the previous revision.
    Revision 1.12  2009/11/11 22:06:13Z  garyp
    Changed the official Reliance name to "Reliance Nitro".
    Revision 1.11  2009/05/23 17:28:23Z  garyp
    Documentation updates -- no functional changes.
    Revision 1.10  2009/04/14 23:51:54Z  garyp
    Reverted the previous checkin.  Updated documentation.
    Revision 1.9  2009/04/14 21:09:34Z  thomd
    Updated to new Product Name
    Revision 1.8  2007/11/03 23:31:21Z  Garyp
    Added the standard module header.
    Revision 1.7  2007/08/19 17:49:59Z  garyp
    Added a max version number string length definition.
    Revision 1.6  2007/07/10 23:43:36Z  keithg
    Added SQA project
    Revision 1.5  2007/02/21 05:14:17Z  brandont
    Updates for the Datalight Automation Framework project.
    Revision 1.4  2006/09/16 19:26:52Z  Garyp
    Added DCL_MAX_BUILDNUM_LENGTH.
    Revision 1.3  2006/08/18 20:18:33Z  Garyp
    Documentation update.  Changed "FlashFX" to be "FlashFX Pro".
    Revision 1.2  2006/02/23 01:04:18Z  Garyp
    Added D_PRODUCTPREFIX.
    Revision 1.1  2005/10/23 02:54:04Z  Pauli
    Initial revision
    Revision 1.3  2005/10/23 03:54:04Z  garyp
    Documentation update.
    Revision 1.2  2005/10/20 04:20:45Z  garyp
    Added product prefix values.
    Revision 1.1  2005/10/02 05:03:00Z  Garyp
    Initial revision
    Revision 1.4  2005/09/29 02:38:19Z  Garyp
    No longer use Mobile4GR.
    Revision 1.3  2005/09/02 00:43:00Z  Garyp
    Changed D_PRODUCTNAME to "4GR".
    Revision 1.2  2005/08/27 04:03:18Z  Garyp
    Added 4GR product number stuff.
    Revision 1.1  2005/07/04 02:48:56Z  pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef DLPROD_H_INCLUDED
#define DLPROD_H_INCLUDED


/*-------------------------------------------------------------------
    NOTE:  These product number must match the values found in the
           various prodinfo.bat files for each of the products.
-------------------------------------------------------------------*/
#define PRODUCTNUM_RELIANCE     (1)
#define PRODUCTNUM_FLASHFX      (2)
#define PRODUCTNUM_DCL          (3)
#define PRODUCTNUM_4GR          (4)
#define PRODUCTNUM_DAF          (5)
#define PRODUCTNUM_SQA          (6)


/*-------------------------------------------------------------------
    These prefixes must be uppercase and exactly 3 characters long.
-------------------------------------------------------------------*/
#define RELPREFIX               "REL"
#define FFXPREFIX               "FFX"
#define DL4GRPREFIX             "4GR"
#define DCLPREFIX               "DCL"
#define DAFPREFIX               "DAF"
#define SQAPREFIX               "SQA"


/*-------------------------------------------------------------------
    Symbol: D_PRODUCTNUM

    The Datalight product number.  This is used to differentiate
    various Datalight code bases.  Typically this number is defined
    in the prodinfo.bat or prodinfo.sh file for the product.

    Symbol: D_PRODUCTNAME

    The user-friendly product name, and should be no more than 15
    characters long, not including terminating NULL.

    Symbol: D_PRODUCTPREFIX

    The product prefix, which must be uppercase and be exactly 2
    characters long.
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
    Macro: DCLFUNC()

    Define a DCL function name using a product prefix from the
    master product.  This allows it to be possible (however
    undesirable) to include separate instances of DCL are
    necessary multiple Datalight products are used in a
    monolithic environment.
-------------------------------------------------------------------*/

    /*--------------------------------------------------------------*\
     * NOTE! The text in the strings below may be automatically     *
     *       changed by the kit building process (see bldcomp.bat), *
     *       and modifying these strings without making adjustments *
     *       elsewhere may break things.                            *
    \*--------------------------------------------------------------*/

#if D_PRODUCTNUM == PRODUCTNUM_DCL
    #define D_PRODUCTNAME           "DCL"
    #define D_PRODUCTPREFIX         "DL"
    #define DCLFUNC(fn)             fn
#elif D_PRODUCTNUM == PRODUCTNUM_RELIANCE
    #define D_PRODUCTNAME           "Reliance Nitro"
    #define D_PRODUCTPREFIX         "RL"
    #define DCLFUNC(fn)             Rel##fn
#elif D_PRODUCTNUM == PRODUCTNUM_FLASHFX
    #define D_PRODUCTNAME           "FlashFX Tera"
    #define D_PRODUCTPREFIX         "FX"
    #define DCLFUNC(fn)             Ffx##fn
#elif D_PRODUCTNUM == PRODUCTNUM_4GR
    #define D_PRODUCTNAME           "4GR"
    #define D_PRODUCTPREFIX         "4G"
    #define DCLFUNC(fn)             DL4GR##fn
#elif D_PRODUCTNUM == PRODUCTNUM_DAF
    #define D_PRODUCTNAME           "Automation Framework"
    #define D_PRODUCTPREFIX         "DF"
    #define DCLFUNC(fn)             Daf##fn
#elif D_PRODUCTNUM == PRODUCTNUM_SQA
    #define D_PRODUCTNAME           "Quality Assurance"
    #define D_PRODUCTPREFIX         "QA"
    #define DCLFUNC(fn)             Sqa##fn
#else
    #error "DCL: dlprod.h: D_PRODUCTNUM is not valid"
#endif


/*-------------------------------------------------------------------
    This defines the maximum length of build number and version
    strings, excluding any terminating NULLs.  A full version
    string includes the build number string.

    Note that both build and version "numbers" actually may be
    alphanumeric sequences.

    These default to odd values so that they may be placed in
    evenly aligned buffers including a null-terminator.
-------------------------------------------------------------------*/
#define DCL_MAX_BUILDNUM_LENGTH     (15)
#define DCL_MAX_VERSION_LENGTH      (DCL_MAX_BUILDNUM_LENGTH + 16)


#endif  /* DLPROD_H_INCLUDED */


