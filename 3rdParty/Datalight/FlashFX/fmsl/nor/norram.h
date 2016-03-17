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

    This header defines emulation targets for use with the NORRAM FIM.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: norram.h $
    Revision 1.9  2010/09/20 02:26:51Z  garyp
    Adjusted the FFX_EMUL_VERYSMALLPAGES page and block sizes to
    accommodate the new "QAMT" tests.
    Revision 1.8  2008/05/27 16:36:15Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.7  2008/02/06 16:53:23Z  keithg
    turned off emulated timmings to increase build/test cycle.
    Revision 1.6  2007/11/03 05:26:04Z  Garyp
    Added the "VERYSMALLPAGES" emulation target.
    Revision 1.5  2007/05/16 21:58:37Z  Garyp
    Documentation updates.
    Revision 1.4  2007/05/14 17:30:54Z  keithg
    Minor updates to the FFX_NORRAM_EMULATION_TARGET
    Revision 1.3  2007/04/27 19:09:45Z  Garyp
    Removed the obsolete and unused EMUL_CHIPCOUNT setting.  Changed the
    default emulation target to use an auto-scaling erase block size and an
    erase block number that by default causes it to appear as if there is two
    chips.
    Revision 1.2  2007/04/26 19:06:07Z  keithg
    Changed the behavior to prevent the creation of disks with erase block
    sizes that were not power of two.  Added EFFICIENCY target emulation.
    Revision 1.1  2006/12/17 00:33:14Z  Garyp
    Initial revision
---------------------------------------------------------------------------*/

#define FFX_EMUL_DEFAULT        (0)
#define FFX_EMUL_DBAU1500       (1)
#define FFX_EMUL_EFFICENCY      (2)
#define FFX_EMUL_VERYSMALLPAGES (3)

/*-------------------------------------------------------------------
    Default Emulation Target

    This target is defined to ensure that the following goals are
    met:
    1) The total amount of RAM required defaults to using the device
       settings values from ffxconf.h.
    2) The erase zone size automatically scales with the maximum
       device size.
    3) Timing emulation is disabled so things run fast.
    4) Boot block emulation is enabled to ensure the code is
       exercised during testing.
    5) There are at least two chips supported to ensure testing
       coverage.
-------------------------------------------------------------------*/
#if FFX_NORRAM_EMULATION_TARGET == FFX_EMUL_DEFAULT
  #define EMUL_TOTALSIZE     (D_UINT32_MAX) /* special flag to use settings from ffxconf.h */
  #define EMUL_ERASEZONESIZE (D_UINT32_MAX) /* Use D_UINT32_MAX to use 1/32nd of the device size */
  #define EMUL_DEVFLAGS         DEV_NOT_MLC /* device flags */
  #define EMUL_BLOCKS_PER_CHIP         (16) /* number of erase blocks per chip */
  #define EMUL_PAGE_SIZE                (0) /* zero means to use the default */
  #define EMUL_BOOT_BLOCKS             TRUE /* should be TRUE for checkin */
  #define EMUL_TIMING                 FALSE /* should be FALSE for checkin */
  #if EMUL_BOOT_BLOCKS
    #define EMUL_BOOTBLOCK_DIVISOR      (8) /* a regular block contains n boot blocks */
    #define EMUL_BOOTBLOCKS_LOW         (1) /* normal blocks at the start which are boot blocks (1 max) */
    #define EMUL_BOOTBLOCKS_HIGH        (1) /* normal blocks at the end which are boot blocks (1 max) */
  #endif
  #if EMUL_TIMING
    #define EMUL_DELAY_READ_US      (100UL) /* microseconds to _sysdelay() per page */
    #define EMUL_DELAY_WRITE_US    (1500UL) /* microseconds to _sysdelay() per page */
    #define EMUL_SLEEP_ERASE_US (1000000UL) /* microseconds to sleep() per block */
  #endif

/*-------------------------------------------------------------------
    The DBAU1500 emulation target is defined to match the
    characteristics of the AMD Alchemy DBAU1500 reference
    platform.
-------------------------------------------------------------------*/
#elif FFX_NORRAM_EMULATION_TARGET == FFX_EMUL_DBAU1500
  #define EMUL_TOTALSIZE   (16*1024*1024UL) /* total physical array size */
  #define EMUL_ERASEZONESIZE   (128*1024UL)
  #define EMUL_DEVFLAGS    (DEV_MERGE_WRITES | DEV_NOT_MLC)
  #define EMUL_BLOCKS_PER_CHIP (EMUL_TOTALSIZE / EMUL_ERASEZONESIZE) /* number of erase blocks per chip */
  #define EMUL_PAGE_SIZE                (0) /* zero means to use the default */
  #define EMUL_BOOT_BLOCKS            FALSE
  #define EMUL_TIMING                 FALSE
  #if EMUL_BOOT_BLOCKS
    #define EMUL_BOOTBLOCK_DIVISOR      (0) /* a regular block contains n boot blocks */
    #define EMUL_BOOTBLOCKS_LOW         (0) /* normal blocks at the start which are boot blocks (1 max) */
    #define EMUL_BOOTBLOCKS_HIGH        (0) /* normal blocks at the end which are boot blocks (1 max) */
  #endif
  #if EMUL_TIMING
    /*  These timing characteristics were determined using FlashFX
        Build 1380 on 12/16/2006.  FMSLTEST and VBFTEST performance
        numbers were compared on the real hardware, using WinCE 4.2,
        and using the Win32 port using the NORRAM FIM.
    */
    #define EMUL_DELAY_READ_US      (130UL) /* microseconds to _sysdelay() per page */
    #define EMUL_DELAY_WRITE_US    (1500UL) /* microseconds to _sysdelay() per page */
    #define EMUL_SLEEP_ERASE_US (1300000UL) /* microseconds to sleep() per block */
  #endif

#elif FFX_NORRAM_EMULATION_TARGET == FFX_EMUL_EFFICENCY
  #define EMUL_TOTALSIZE     (D_UINT32_MAX) /* special flag to use settings from ffxconf.h */
  #define EMUL_ERASEZONESIZE   (128*1024UL) /* Use D_UINT32_MAX to use 1/32nd of the device size */
  #define EMUL_DEVFLAGS         DEV_NOT_MLC /* device flags */
  #define EMUL_BLOCKS_PER_CHIP         (32)
  #define EMUL_PAGE_SIZE                (0) /* zero means to use the default */
  #define EMUL_BOOT_BLOCKS            FALSE
  #define EMUL_TIMING                  TRUE
  #if EMUL_TIMING
  /*  These timing characteristics are based on the Intel StrataFlash
      K3/18 series specifications.  They are scaled by a factor of X
      so it will execute that much faster for testing purposes.  Choose
      this value to keep the relative timings between read/write/erase
      somewhat accurate.  Timings much over 50 will adversely affect
      the relative timing characteristics.
  */
    #ifndef EMUL_TIMING_FACTOR
      #define EMUL_TIMING_FACTOR 50
    #endif
    #define EMUL_DELAY_READ_US      (120UL/EMUL_TIMING_FACTOR)
    #define EMUL_DELAY_WRITE_US    (1500UL/EMUL_TIMING_FACTOR)
    #define EMUL_SLEEP_ERASE_US  (750000UL/EMUL_TIMING_FACTOR)
  #endif

#elif FFX_NORRAM_EMULATION_TARGET == FFX_EMUL_VERYSMALLPAGES
  #define EMUL_TOTALSIZE     (D_UINT32_MAX) /* special flag to use settings from ffxconf.h */
  #define EMUL_ERASEZONESIZE   (8*1024UL)   /* 8KB is as small as we can practically make it right now */
  #define EMUL_DEVFLAGS         DEV_NOT_MLC /* device flags */
  #define EMUL_BLOCKS_PER_CHIP         (32)
  #define EMUL_PAGE_SIZE               (256) /* zero means to use the default */
  #define EMUL_BOOT_BLOCKS            FALSE
  #define EMUL_TIMING                 FALSE /* should be FALSE for checkin */
  #if EMUL_BOOT_BLOCKS
    #define EMUL_BOOTBLOCK_DIVISOR      (8) /* a regular block contains n boot blocks */
    #define EMUL_BOOTBLOCKS_LOW         (1) /* normal blocks at the start which are boot blocks (1 max) */
    #define EMUL_BOOTBLOCKS_HIGH        (1) /* normal blocks at the end which are boot blocks (1 max) */
  #endif
  #if EMUL_TIMING
    #define EMUL_DELAY_READ_US      (100UL) /* microseconds to _sysdelay() per page */
    #define EMUL_DELAY_WRITE_US    (1500UL) /* microseconds to _sysdelay() per page */
    #define EMUL_SLEEP_ERASE_US (1000000UL) /* microseconds to sleep() per block */
  #endif

/*-------------------------------------------------------------------
    An invalid emulation target was defined.
-------------------------------------------------------------------*/
#else
  #error "FFX: norram.h -- unrecognized emulation target"
#endif


