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

    This header file contains standard data types and handles used throughout
    FlashFX.  These are mostly opaque types -- the actual definitions are
    located where they are used.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxtypes.h $
    Revision 1.25  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.24  2009/07/29 18:39:46Z  garyp
    Merged from the v4.0 branch.  Minor datatype and documentation changes.
    Revision 1.23  2009/05/07 16:01:29Z  keithg
    Removed compile time configuration defines to allow this file to
    be used in application level includes.
    Revision 1.22  2009/04/08 19:17:57Z  garyp
    Updated for AutoDoc -- no functional changes.
    Revision 1.21  2009/03/27 07:45:41Z  keithg
    Added FFXREQHANDLE from the fxapi.h module.
    Revision 1.20  2008/11/19 23:20:39Z  keithg
    Moved default macro definitions into fxmacros.h
    Revision 1.19  2008/10/08 20:14:08Z  keithg
    Added BBM v5 default format settings.
    Revision 1.18  2008/05/27 16:36:15Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.17  2008/03/21 20:26:36Z  Garyp
    Added default configuration values for DEVICE, DISK, ALLOCATOR, and FORMAT
    settings.  Modified BBM settings to include a value to indicate whether BBM
    should be used at all.
    Revision 1.16  2008/01/31 03:49:47Z  Garyp
    Updated so FFX_NAND_TAGSIZE is always defined.
    Revision 1.15  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.14  2007/08/01 15:25:02Z  timothyj
    Removed compile-time limiting of large flash array support.  Removed the
    obsolete FFX_FLASHOFFSET.
    Revision 1.13  2007/06/28 17:45:52Z  timothyj
    Defined FFX_REMAINING as FFX_FLASHOFFSET_MAX, which can be either 64 or
    32 bits wide, depending on whether FFXCONF_LARGE_FLASH is defined.
    Revision 1.12  2007/03/30 23:36:57Z  timothyj
    Changed FFXCONF_LFA to FFXCONF_LARGE_FLASH for clarity (BZ/IR #953).
    Revision 1.11  2007/03/01 00:28:51Z  timothyj
    Added FFX_FLASHOFFSET_MAX type for linear flash offsets that resolves to
    D_UINT64 or D_UINT32 depending on whether the build is configured for LFA
    support.  Currently only D_UINT32 is supported.
    Revision 1.10  2007/02/01 02:42:54Z  Garyp
    Added some default settings values.
    Revision 1.9  2006/11/10 20:36:09Z  Garyp
    Removed an obsolete type.
    Revision 1.8  2006/05/08 01:35:24Z  Garyp
    Finalized the statistics interfaces.
    Revision 1.7  2006/03/04 04:54:10Z  Garyp
    Changed FFX_BADADDRESS to automatically cast to void*.
    Revision 1.6  2006/02/27 05:33:27Z  Garyp
    Added default NAND and NOR tag size settings.  Removed dead code.
    Revision 1.5  2006/02/11 23:25:46Z  Garyp
    Tweaked to build cleanly.
    Revision 1.4  2006/02/11 03:51:39Z  Garyp
    Added FFX_REMAINING and FFX_BADADDRESS.
    Revision 1.3  2006/02/10 22:40:19Z  Garyp
    Added some default FlashFX settings.
    Revision 1.2  2006/02/10 10:35:47Z  Garyp
    Updated to use the FFXDEVHANDLE and FFXDISKHANDLE model.
    Revision 1.1  2005/11/25 20:56:46Z  Pauli
    Initial revision
    Revision 1.3  2005/11/25 20:56:46Z  Garyp
    Removed FFXIOSTATUS.
    Revision 1.2  2005/10/10 04:44:14Z  Garyp
    Added the FFXIOSTATUS and FFXALLOC structures.
    Revision 1.1  2005/10/06 06:49:22Z  Garyp
    Initial revision
    Revision 1.20  2005/02/13 23:17:56Z  GaryP
    Eliminated an unused type.
    Revision 1.19  2004/12/30 21:38:42Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.18  2004/11/20 04:58:47Z  GaryP
    Removed some misplaced types.
    Revision 1.17  2004/08/30 21:43:38Z  GaryP
    Added PFFXMUTEX and PFFXSEMAPHORE type declarations from oeslapi.h.
    Revision 1.16  2004/08/11 19:26:51Z  GaryP
    Moved the PFATMONINST definition to fxdriver.h.
    Revision 1.15  2004/08/06 03:28:43Z  GaryP
    Added an opaque type for PFATMONINST.
    Revision 1.14  2004/07/27 15:52:22Z  BillR
    Add an implementation of _sysdelay() that uses a calibrated loop for use
    on platforms that lack a microsecond-resolution time reference,
    Revision 1.13  2004/07/20 01:09:11Z  GaryP
    Moved the stuff related to the tools, tests, and All-in-one-framework to
    different headers.  Added opaque types for FFXDRIVERINFO and FFXDISKDATA.
    Revision 1.12  2004/06/18 22:51:51Z  BillR
    New project hooks for initialization and configuration.
    Revision 1.11  2004/04/23 09:05:33Z  garyp
    Updated to define CHAR_BIT if not already defined.
    Revision 1.10  2004/03/12 06:01:05Z  garys
    Merge from FlashFXMT
    Revision 1.7.1.8  2004/03/12 06:01:05  garyp
    Fixed the PEXTMEDIAINFO declaration.
    Revision 1.7.1.7  2004/01/24 22:21:12Z  garyp
    Modified VBFHANDLE to be a far pointer (among other minor changes).
    Revision 1.7.1.6  2003/12/30 23:16:20Z  billr
    Comment changes only.
    Revision 1.7.1.5  2003/12/05 22:40:20Z  garyp
    Added the DEVEXTRA type.
    Revision 1.7.1.4  2003/11/26 21:07:26Z  billr
    Changes for thread safety. Compiles (Borland/DOS), not yet tested.
    Revision 1.7.1.3  2003/11/11 17:13:44Z  garyp
    Fixed an invalid structure declaration.
    Revision 1.7.1.2  2003/11/02 05:44:32Z  garyp
    Re-checked into variant sandbox.
    Revision 1.8  2003/11/02 05:44:32Z  garyp
    Added the FMLHANDLE and VBFHANDLE types.
    Revision 1.7  2003/04/15 23:14:40Z  garyp
    Removed PFFXMUTEX type.
    Revision 1.6  2003/04/07 22:29:46Z  garyp
    Added the PFFXMUTEX type.
    Revision 1.5  2003/03/26 09:19:40Z  garyp
    Eliminated the __D_* data types.
    Revision 1.4  2002/12/03 21:52:18Z  dennis
    Added NVBF MGM image support to rdimg and wrimg conditional on the MGM
    command line switch.  Added D_BOOL * and __D_PBOOL types to FXTYPES.H.
    Revision 1.3  2002/11/14 11:22:08  garyp
    Added the D_PCCHAR types.
    Revision 1.2  2002/11/07 11:07:46Z  garyp
    Added typedefs for Unicode support.
---------------------------------------------------------------------------*/
#ifndef FXTYPES_H_INCLUDED
#define FXTYPES_H_INCLUDED

/*-------------------------------------------------------------------
    Define the standard tag sizes for NAND and NOR.
-------------------------------------------------------------------*/
/*  Define this value even when NAND support is disabled -- some
    of the tools require it even if NAND support is turned off.
*/
#define FFX_NAND_TAGSIZE  (2)
#define FFX_NOR_TAGSIZE   (4)


/*-------------------------------------------------------------------
                     Standard handles used most everywhere
-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
   VBFHANDLE

   For the time being, a VBFHANDLE is a pointer to a pointer to
   a VBFDATA structure.

   ToDo: Investigate relocating this to somewhere in VBF.  Seems
         like it should not need to be visible at this level.
-------------------------------------------------------------------*/
typedef struct sVBFDATA           **VBFHANDLE;


/*-------------------------------------------------------------------
    Type: FFXFMLHANDLE

    A handle to an FML instance created with FfxFmlCreate().

    Type: FFXDEVHANDLE

    A handle to a Device created with FfxDriverDeviceCreate() or
    FfxDriverDeviceCreateParam().

    Type: FFXDISKHANDLE

    A handle to a Disk created with FfxDriverDiskCreate() or
    FfxDriverDiskCreateParam().
-------------------------------------------------------------------*/
typedef DCLOSREQHANDLE              FFXREQHANDLE;
typedef struct tagFFXFIMDEVICE     *FFXFIMDEVHANDLE;
typedef struct sFFXFMLDISK         *FFXFMLHANDLE;

typedef struct tagFFXDRIVERINFO     FFXDRIVERINFO;  /* Master driver structure  */

typedef struct tagFFXDEVINITDATA    FFXDEVINITDATA; /* OS specific init-time info */
typedef struct tagFFXDEVCONFIG      FFXDEVCONFIG;   /* Driver Framework info    */
typedef struct tagFFXDEVHOOK        FFXDEVHOOK;     /* project specific info    */
typedef struct tagFFXDEVINFO        FFXDEVINFO;     /* Driver Framework info    */
typedef struct tagFFXDEVINFO      **FFXDEVHANDLE;   /* General Device handle    */

typedef struct tagFFXDISKINITDATA   FFXDISKINITDATA;/* OS specific init-time info */
typedef struct tagFFXDISKCONFIG     FFXDISKCONFIG;  /* Driver Framework info    */
typedef struct tagFFXDISKHOOK       FFXDISKHOOK;    /* project specific info    */
typedef struct tagFFXDISKINFO       FFXDISKINFO;    /* Driver Framework info    */
typedef struct tagFFXDISKINFO     **FFXDISKHANDLE;  /* General Disk handle      */

typedef struct tagFFXIOSTATUS       FFXIOSTATUS;




#endif /* FXTYPES_H_INCLUDED */

