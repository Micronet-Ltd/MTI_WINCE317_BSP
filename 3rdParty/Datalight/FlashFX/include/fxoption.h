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

    This module provide the definitions necessary to use the FXOPTION module.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: fxoption.h $
    Revision 1.21  2010/11/22 05:56:55Z  glenns
    Added FFX option for getting allocator option flags.
    Revision 1.20  2010/02/18 02:57:57Z  billr
    Update license text and copyright date on shared and public files.
    Revision 1.19  2009/12/31 17:24:42Z  billr
    Refactor headers for better separation between 0S-dependent and
    OS-independent code.
    Revision 1.18  2009/12/14 22:23:44Z  keithg
    Added options for the NANDSIM chip ID and NOR device type  and
    NOR chip size for the flash simulators.
    Revision 1.17  2009/12/11 22:19:12Z  garyp
    Added lock related options.
    Revision 1.16  2009/08/02 16:53:44Z  garyp
    Modified the FAT serial number option to be similar to other FAT options.
    Revision 1.15  2009/01/19 05:22:03Z  keithg
    Added NANDSIM_TABLE_GET option used by the NSPTABLE.C.
    Revision 1.14  2008/12/09 21:36:54Z  keithg
    Added option values for over-riding the default random serial number.
    Revision 1.13  2008/03/26 02:56:24Z  Garyp
    Changed BBMFORMAT to BBMSETTINGS.  Added ALLOCATORSETTINGS.
    Revision 1.12  2007/11/03 23:49:38Z  Garyp
    Updated to use the standard module header.
    Revision 1.11  2007/09/13 16:55:06Z  johnb
    backed out project specific option and moved to project code.
    Revision 1.9  2007/02/01 02:42:23Z  Garyp
    Added support for handling device timing and latency settings.
    Revision 1.8  2006/10/09 19:27:58Z  Garyp
    Obsoleted a number of settings, and added a few.
    Revision 1.7  2006/08/11 23:17:38Z  rickc
    Added option for the PISMO PCI card
    Revision 1.6  2006/07/26 17:08:58Z  Pauli
    Added options for NAND Simulator.
    Revision 1.5  2006/05/28 23:08:37Z  Garyp
    Made FAT options conditional on FFXCONF_FATSUPPORT.
    Revision 1.4  2006/03/06 22:15:25Z  Garyp
    Eliminated the use of obsolete options.
    Revision 1.3  2006/02/15 05:21:06Z  Garyp
    Removed obsolete settings.
    Revision 1.2  2006/02/09 19:41:52Z  Garyp
    Added a number of new options.
    Revision 1.1  2005/12/04 21:07:12Z  Pauli
    Initial revision
    Revision 1.3  2005/12/04 21:07:12Z  Garyp
    Modified the compaction model to be specified as a tri-state value, which is
    one of the following FFX_COMPACT_SYNCHRONOUS, FFX_COMPACT_BACKGROUNDIDLE, or
    FFX_COMPACT_BACKGROUNDTHREAD.
    Revision 1.2  2005/11/14 13:44:35Z  Garyp
    Updated to use the FFXCONF_IDLETIMECOMPACTION setting.
    Revision 1.1  2005/10/06 06:51:06Z  Garyp
    Initial revision
    Revision 1.3  2005/09/18 05:26:55Z  garyp
    Modified so that last read/write access times are recorded in VBF rather
    than the driver framework, and are not conditional on the background
    compaction feature being enabled.
    Revision 1.2  2005/08/21 04:41:31Z  garyp
    Eliminated // comments.
    Revision 1.1  2005/08/05 17:40:56Z  pauli
    Initial revision
    Revision 1.2  2005/08/05 17:40:56Z  Garyp
    Updated to use revamped reserved space options which now allow
    reserved space at the top of the array.
    Revision 1.1  2005/07/28 16:15:24Z  pauli
    Initial revision
    Revision 1.13  2005/03/30 04:50:48Z  GaryP
    Added support for the BBM format type, and spare area percentage. both
    of which can be configured through the standard options interface.
    Revision 1.12  2005/02/11 17:55:58Z  billr
    Add options for NANDRAM FIM.
    Revision 1.11  2004/12/30 23:14:02Z  GaryP
    Updated to use indents of 4, hard tabs of 8 (if used), and a standard
    comment style.
    Revision 1.10  2004/09/29 18:48:15Z  billr
    New options to support file FIMs.
    Revision 1.9  2004/09/23 03:36:56Z  GaryP
    Added compaction options.
    Revision 1.8  2004/09/08 01:56:25Z  GaryP
    Eliminated the FFXOPT_FATFORMAT setting.
    Revision 1.7  2004/08/23 17:04:45Z  GaryP
    Added thread priority options.
    Revision 1.6  2004/08/11 00:51:53Z  GaryP
    Fixed a documentation problem.
    Revision 1.5  2004/08/06 23:34:20Z  GaryP
    Added several new FFXOPT_* values.
    Revision 1.4  2004/07/23 23:43:35Z  GaryP
    Added support for specifying a default file system.
    Revision 1.3  2004/07/18 03:30:39Z  GaryP
    Added FFXOPT_FORMATBBMSTATE.  Created some levels in the enumeration
    to ease debugging.
    Revision 1.2  2004/07/12 21:28:20Z  garyp
    Eliminated a trailing comma in an enumeration.
    Revision 1.1  2004/07/08 20:51:22Z  garys
    Initial revision
---------------------------------------------------------------------------*/
#ifndef FXOPTION_H_INCLUDED
#define FXOPTION_H_INCLUDED


/*  Option specifier for FfxHookOptionGet.
*/
typedef enum
{
    /*  Option codes that must always be defined even if not supplied or
        used by the OS or project.
    */
    FFXOPT_FLASH_START          = 0x0000,
  #if FFXCONF_FATSUPPORT
  #if FFXCONF_FATFORMATSUPPORT              /* If using the FlashFX FAT format  */
    FFXOPT_FATROOTENTRIES       = 0x0400,   /* Entries in the FAT root          */
    FFXOPT_FATCOUNT,                        /* Number of FATs to use            */
    FFXOPT_FATSERIALNUM,
  #endif
  #if FFXCONF_FATMONITORSUPPORT
    FFXOPT_USEFATMONITOR        = 0x0600,   /* Is FAT Monitor enabled?          */
  #endif
  #endif
  #if FFX_COMPACTIONMODEL != FFX_COMPACT_SYNCHRONOUS
    FFXOPT_COMPACTIONREADIDLEMS = 0x0A00,   /* wait n ms after the last read    */
    FFXOPT_COMPACTIONWRITEIDLEMS,           /* wait n ms after the last write   */
  #if FFX_COMPACTIONMODEL == FFX_COMPACT_BACKGROUNDTHREAD
    FFXOPT_COMPACTIONPRIORITY,              /* thread priority                  */
    FFXOPT_COMPACTIONSLEEPMS,               /* ms to sleep when nothing to do   */
  #endif
  #endif

    /*  Device specific settings
    */
    FFXOPT_DEVICE_NUMBER        = 0x2000,
    FFXOPT_DEVICE_SETTINGS,
    FFXOPT_DEVICE_BLOCKSIZE,
    FFXOPT_DEVICE_PAGESIZE,
  #if FFXCONF_LATENCYREDUCTIONENABLED
    FFXOPT_DEVICE_TIMING,
    FFXOPT_DEVICE_LATENCY,
  #endif
  #if FFXCONF_BBMSUPPORT
    FFXOPT_DEVICE_BBMSETTINGS,
  #endif
    FFXOPT_DEVICE_SOFTLOCKFLAGS,
    FFXOPT_DEVICE_SOFTLOCKSTATE,
    FFXOPT_DEVICE_AUTOUNLOCK,

    /*  Disk specific settings
    */
    FFXOPT_DISK_DEVICENUM       = 0x3000,   /* The device number for a disk     */
    FFXOPT_DISK_OFFSET,                     /* The disk offset in blocks        */
    FFXOPT_DISK_SIZE,                       /* The disk size in blocks          */
    FFXOPT_DEVICE_TYPE,                     /* Determine if flash is Sibley     */
    FFXOPT_DEVICE_CHIPSIZE,                 /* Size of a physical flash chip    */
  #if FFXCONF_ALLOCATORSUPPORT
    FFXOPT_DISK_ALLOCSETTINGS,              /* The disk allocator settings      */
    FFXOPT_DISK_ALLOCOPTIONFLAGS,           /* Allocator option flags           */
  #endif
  #if FFXCONF_FORMATSUPPORT
    FFXOPT_DISK_FORMATSETTINGS,             /* The disk format settings         */
  #endif

    /*  Option codes that are used within a particular OS Layer.
    */

    /*  = 0x4000
    */

    /*  Option codes that are used within particular FIMs and NTMs.
    */
    FFXOPT_FIM_TESTMEMORY       = 0x6000,
    FFXOPT_FIM_FILENAME,
/*  FFXOPT_FIM_FILESIZE, */
    FFXOPT_FIM_NANDRAM_MAIN,    /* (D_UCHAR *) pointer to space for main array */
    FFXOPT_FIM_NANDRAM_SPARE,   /* (D_UCHAR *) pointer to space for spare array */

    /*  Option codes used by the NAND simulator.
    */
    FFXOPT_NANDSIM_ID           = 0x7000,   /* Flash ID                 */
    FFXOPT_NANDSIM_NUM_CHIPS,               /* Number of chips          */
    FFXOPT_NANDSIM_POLICY,                  /* Error policy to use      */
    FFXOPT_NANDSIM_STORAGE,                 /* Storage mechanism to use */
    FFXOPT_NANDSIM_RAM,                     /* Memory for RAM storage mechanism */
    FFXOPT_NANDSIM_FILE,                    /* File name/path FILE storage mechanism */
    FFXOPT_NANDSIM_GET_TABLE,               /* Table based error policy */
    FFXOPT_NANDSIM_DECODEID,                /* Used to decode unknown chips */

    /*  Option codes used only by the project.
    */

    /*  Option code for the PISMO PCI card
    */
    FFXOPT_PLX_LCR_BASE       = 0x8000,

    /*  Format parameter settings
    */
    FFXOPT_BBM_FMT_SERIALNUM  = 0x9000,
    FFXOPT_VBF_FMT_SERIALNUM,

    /*  End of the enumeration.
    */
    FFXOPT_ENDOFLIST
} FFXOPTION;

D_BOOL          FfxOptionGet(FFXOPTION opt, void *pBuffer, D_UINT32 ulBuffLen);

#endif /* FXOPTION_H_INCLUDED */

