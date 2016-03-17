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

    This module provides the information necessary to configure a Windows CE
    based FlashFX project.
---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                                Revision History
    $Log: ffxconf.h $
    Revision 1.14.1.2  2010/12/20 22:40:38Z  glenns
    Removed FFXCONF_SOFTLOCKSUPORT, as this feature is not
    supported in this product version.
    Revision 1.14  2010/11/22 16:13:55Z  glenns
    Added FFXCONF_QUICKMOUNTSUPPORT option.
    Revision 1.13  2010/01/23 22:55:54Z  garyp
    Eliminated the FFX_DISK0_FORMAT setting.  Disk format settings
    for Windows CE are now handled entirely in the registry.
    Revision 1.12  2009/07/19 02:53:27Z  garyp
    Merged from the v4.0 branch.  Added a NAND Special Features Section.
    Changed the FFXCONF_FATFORMATSUPPORT default value to FALSE.
    Revision 1.11  2009/04/03 05:12:31Z  keithg
    Work around for bug 2561: Removed FFXCONF_READAHEADENABLED
    Revision 1.10  2008/05/27 16:36:16Z  keithg
    Clarified comments with professional customer facing terms.
    Revision 1.9  2008/03/27 02:56:54Z  Garyp
    Renamed FFX_DEV0_BBMFORMAT to FFX_DEV0_BBM.  Added DISK-level allocator
    configuration settings.  Added the FFX_MAX_PAGESIZE setting.  Added support
    for compiling the allocator feature out of the product entirely.  Reduced
    documentation for timing and latency configuration (see the product docs).
    Revision 1.8  2007/11/03 23:50:35Z  Garyp
    Updated to use the standard module header.
    Revision 1.7  2007/09/13 20:28:54Z  thomd
    default MTSTRESS param /time
    Revision 1.6  2007/09/13 16:20:03Z  thomd
    Updated FFX_MTTESTPARAMS
    Revision 1.5  2007/08/03 14:12:52Z  timothyj
    Changed device and disk settings from byte offsets to units of kilobytes.
    Revision 1.4  2007/02/01 02:27:31Z  Garyp
    Added device timing and latency configuration settings.
    Revision 1.3  2006/10/13 01:47:34Z  Garyp
    Major refactoring of the format related settings to allow any or all of the
    formatting features to be scaled out of the code, as well as to allow each
    disk to have its own format settings (state, file system, MBR, etc).
    Revision 1.2  2006/07/10 22:38:42Z  Garyp
    Renamed NTCADIO to NTCAD.  Fixed some invalid settings due to this
    file originating from Nucleus rather than CE.
    Revision 1.1  2006/05/08 12:04:36Z  timothyj
    Initial revision
    Revision 1.9  2006/04/04 01:23:54Z  michaelm
    removed mention of fimlist.pdf and instead refer to website
    Revision 1.8  2006/03/14 18:27:03Z  Pauli
    Corrected the NAND device size to match the hardware (16MB).
    Revision 1.7  2006/03/11 05:29:39Z  Pauli
    Updated comments.
    Revision 1.6  2006/03/08 20:53:10Z  Garyp
    Changed the NTM init process back to using a list of possible NTMs, rather
    than a single fixed NTM per Device.
    Revision 1.5  2006/03/04 21:39:35Z  Garyp
    Eliminated an unnecessary typecast.
    Revision 1.4  2006/02/16 18:11:01Z  Pauli
    Updated to the device/disk configuration scheme.
    Revision 1.3  2005/12/30 16:48:53Z  Garyp
    Updated to use renamed thread related settings and functions, which are
    now a part of DCL.
    Revision 1.2  2005/12/15 00:09:36Z  Pauli
    Fixed typo in FIM name.
    Revision 1.1  2005/12/13 21:37:36Z  Pauli
    Initial revision
---------------------------------------------------------------------------*/

#ifndef FFXCONF_H_INCLUDED
#define FFXCONF_H_INCLUDED


            /*------------------------------------------*\
             *              Feature Support             *
             *                                          *
             *  FFXCONF_* settings define the overall   *
             *  feature set which will be compiled into *
             *  the FlashFX device driver.  Disable     *
             *  features which are not being used to    *
             *  reduce the size of the device driver.   *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
    Flash Technology Support

    This section defines the flash technologies that the driver
    will support.  At least one of these features must be enabled.
-------------------------------------------------------------------*/
#define FFXCONF_NANDSUPPORT             TRUE
#define FFXCONF_NORSUPPORT              FALSE
#define FFXCONF_ISWFSUPPORT             FALSE


/*-------------------------------------------------------------------
    Bad Block Management (BBM)

    Typically BBM is required if NAND flash is being used.
-------------------------------------------------------------------*/
#define FFXCONF_BBMSUPPORT              FFXCONF_NANDSUPPORT


/*-------------------------------------------------------------------
    Flash Allocator Support

    FFXCONF_ALLOCATORSUPPORT defines whether FlashFX has support
    for an allocator built in.
-------------------------------------------------------------------*/
#define FFXCONF_ALLOCATORSUPPORT        TRUE


/*-------------------------------------------------------------------
    Quick Mount Support

    This section defines whether the FlashFX Quick Mount option
    is enabled. This option allows saving allocator state info
    across a power cycle so that it does not have to be totally
    regenerated at every disk mount.
-------------------------------------------------------------------*/
#define FFXCONF_QUICKMOUNTSUPPORT       FALSE
    

/*-------------------------------------------------------------------
    File System Support

    Define the types of file systems about which FlashFX should
    include logic to support.  Setting either of these values to
    FALSE will eliminate support for the respective file system.
-------------------------------------------------------------------*/
#define FFXCONF_RELIANCESUPPORT         TRUE
#define FFXCONF_FATSUPPORT              FALSE


/*-------------------------------------------------------------------
    Format Support

    FFXCONF_FORMATSUPPORT controls whether FlashFX contains any
    formatting code.  If this is FALSE, than all formatting features
    within every level of FlashFX are compiled out.

    FFXCONF_DRIVERAUTOFORMAT controls whether FlashFX has the ability
    to automatically format the disk when the driver is loaded.  If
    this feature is turned off, formatting must be invoked manually.

    FFXCONF_FATFORMATSUPPORT controls whether FlashFX has the ability
    to write a FAT format to the disk.

    FFXCONF_BBMFORMAT controls whether FlashFX has the ability to
    perform a low level BBM format.
-------------------------------------------------------------------*/
#define FFXCONF_FORMATSUPPORT           TRUE

#if FFXCONF_FORMATSUPPORT
  #define FFXCONF_DRIVERAUTOFORMAT      TRUE
  #define FFXCONF_FATFORMATSUPPORT      FALSE
  #define FFXCONF_BBMFORMAT             FFXCONF_BBMSUPPORT
#endif


/*-------------------------------------------------------------------
    Latency Configuration Options

    FFXCONF_LATENCYREDUCTIONENABLED defines whether FlashFX includes
    logic to improve system latency by breaking large I/O requests
    into smaller requests, at the cost of reduced I/O throughput.

    See the FlashFX documentation for detailed instructions on how
    to configure and use this feature.
-------------------------------------------------------------------*/
#define FFXCONF_LATENCYREDUCTIONENABLED TRUE
#if FFXCONF_LATENCYREDUCTIONENABLED
  #define FFXCONF_LATENCYAUTOTUNE       TRUE
  #define FFXCONF_ERASESUSPENDSUPPORT   (FFXCONF_NORSUPPORT || FFXCONF_ISWFSUPPORT)
#endif


/*-------------------------------------------------------------------
    NAND Special Features

    FFXCONF_OTPSUPPORT defines whether any OTP features which might
    be available in the NTM are enabled or not.

    "LEGACY", "SSFDC", and "OFFSETZERO" denote the different styles
    of NAND bad block marking which may be supported.
-------------------------------------------------------------------*/
#define FFXCONF_OTPSUPPORT              FFXCONF_NANDSUPPORT
#define FFXCONF_NANDSUPPORT_LEGACY      FFXCONF_NANDSUPPORT
#define FFXCONF_NANDSUPPORT_SSFDC       FFXCONF_NANDSUPPORT
#define FFXCONF_NANDSUPPORT_OFFSETZERO  FFXCONF_NANDSUPPORT


            /*------------------------------------------*\
             *           Device Configuration           *
             *                                          *
             *  Devices are identified as DEV0 to DEVn  *
             *  and correspond to physical flash arrays *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
                     Device Configuration Settings

    FFX_DEVn_FIMS

        Each device has its own list of Flash Interface Modules
        (FIMs), defined in FFX_DEVn_FIMS, using the form shown below.

    FFX_DEVn_NTMS

        If the NAND FIM is specified for a given device, a list of
        NAND Technology Modules (NTMs) must be specified in the form
        shown below.  If the device is NOR, this setting should be
        set to {NULL}.

        NOTE: The FIM and NTM lists are case sensitive and must be
              null-terminated.

    FFX_DEVn_SETTINGS

        This defines the basic configuration information for each
        device, in the form:

          {UncachedAddress, ReservedLoKB, ReservedHiKB, MaxDevSizeKB}

        UnchachedAddress is the base address of the flash array.
        For flash types such as NAND, or the NOR "ram" or "file"
        FIMS, where virtual address mapping is not required, this
        value is not used and is typically set to FFX_BADADDRESS.

        ReservedLoKB and ReservedHiKB specify the amount of reserved
        flash at the beginning and end of the flash array in KB,
        which will not be touched by FlashFX.

        MaxDevSizeKB is the maximum amount of flash in KB, to use for
        the device, AFTER removing any reserved space.  For NAND,
        the actual usable amount of flash will be less than that
        specified here, due to BBM overhead.

    FFX_DEVn_TIMING and FFX_DEVn_LATENCY

        These settings define the timing characteristics for the
        physical device, and the desired system latency settings.
        These settings are only used used when the latency reduction
        features are enabled using FFXCONF_LATENCYREDUCTIONENABLED.

    FFX_DEVn_BBM

        This defines the BBM settings for the device.

    See the FlashFX documentation for detailed information about
    configuring these settings.  Additionally, www.datalight.com
    has a complete list of supported FIMs, NTMs, flash parts, and
    NAND controllers.
-------------------------------------------------------------------*/
#define FFX_MAX_DEVICES         (1)     /* Max Devices Supported */

#define FFX_DEV0_FIMS           {&FFXFIM_nand, NULL}
#define FFX_DEV0_NTMS           {&FFXNTM_cad, NULL}
#define FFX_DEV0_SETTINGS       {FFX_BADADDRESS, 0, 0, 0x00004000UL}
#define FFX_DEV0_TIMING         {FFX_DEFAULT_TIMING_SETTINGS}
#define FFX_DEV0_LATENCY        {FFX_DEFAULT_LATENCY_SETTINGS}
#define FFX_DEV0_BBM            {FFX_DEFAULT_BBM_SETTINGS}

/* Replicate these settings to configure additional devices... */



            /*------------------------------------------*\
             *            Disk Configuration            *
             *                                          *
             *  Disks are identified as DISK0 to DISKn  *
             *  and are specified as an offset/length   *
             *  on a given device.                      *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
                      Disk Configuration Settings

        The following is a brief description of the Disk Config-
        uration settings.  See the FlashFX documentation for a
        complete description of these options.

    Basic Settings

        For Windows CE, basic Disk settings, such as which Device a
        Disk resides on, the starting offset and length to use, and
        the format state, are specified in the registry.  See the 
        FlashFX documentation for information on configuring these
        settings.

    FFX_DISKn_ALLOCATOR

        This defines the allocator settings for the Disk.  See the
        FlashFX documentation for a complete description of how to
        customize this setting.

    FFX_DISKn_COMPACTION

        This defines the compaction settings for the Disk.  See the
        FlashFX documentation for a complete description of how to
        customize this setting.
-------------------------------------------------------------------*/
#define FFX_MAX_DISKS           (1)     /* Max Disks Supported */

#define FFX_DISK0_ALLOCATOR     {FFX_DEFAULT_ALLOCATOR_SETTINGS}
#define FFX_DISK0_COMPACTION    {FFX_DEFAULT_COMPACTION_SETTINGS}


/* Replicate these settings to configure additional disks... */



            /*------------------------------------------*\
             *                                          *
             *      Testing and Debugging Settings      *
             *                                          *
            \*------------------------------------------*/



/*-------------------------------------------------------------------
    Set to TRUE to cause the FlashFX tests to be built into the
    device driver, and automatically run at driver initialization
    time.

    NOTE: These tests will destroy all the data on the disk, and
          the disk must be formatted once these tests are complete.

    Set the FFX_xxxTESTPARAMS symbols to the test parameters to be
    used, per the test documentation.  If one of these symbols is
    left undefined, the corresponding test will be skipped.
-------------------------------------------------------------------*/
#define FFXCONF_DRIVERAUTOTEST  TRUE

#if FFXCONF_DRIVERAUTOTEST
  #define  FFX_DCLTESTPARAMS    ""
  #define  FFX_FMSLTESTPARAMS   "/p /e"
  #define  FFX_VBFTESTPARAMS    "/p /e"
  #define  FFX_MTTESTPARAMS     "/time:5"
#endif


/*-------------------------------------------------------------------
    Error Injection Tests

    See the FlashFX documentation for more information about
    configuring and using the error injection tests.
-------------------------------------------------------------------*/
#define FFXCONF_ERRORINJECTIONTESTS  FALSE

#if FFXCONF_ERRORINJECTIONTESTS
  #define FFX_ERRINJRATE_READ   (10000UL)  /* bit errors per million reads  */
  #define FFX_ERRINJRATE_WRITE    (100UL)  /* I/O errors per million writes */
  #define FFX_ERRINJRATE_ERASE   (1000UL)  /* I/O errors per million erases */
#endif


/*-------------------------------------------------------------------
    FFX_TRACEMASK is used to specify the classes of debug messages
    for which tracing will be enabled.  See fxtrace.h for more
    information.
-------------------------------------------------------------------*/
#define FFX_TRACEMASK                       ( 0                         \
/*                                        |   FFXTRACEBIT_DRIVER     /**/ \
/*                                        |   FFXTRACEBIT_DRIVERFW   /**/ \
/*                                        |   FFXTRACEBIT_VBF        /**/ \
/*                                        |   FFXTRACEBIT_COMPACTION /**/ \
/*                                        |   FFXTRACEBIT_MEDIAMGR   /**/ \
/*                                        |   FFXTRACEBIT_BBM        /**/ \
/*                                        |   FFXTRACEBIT_NAND       /**/    \
/*                                        |   FFXTRACEBIT_NTM        /**/     \
/*                                        |   FFXTRACEBIT_NTMHOOK    /**/     \
/*                                        |   FFXTRACEBIT_CONFIG     /**/     \
/*                                        |   FFXTRACEBIT_EXTAPI     /**/   \
                                            )


/*-------------------------------------------------------------------
    Set the various "ReadBackVerify" settings to TRUE to enable
    read-back-verify functionality at the desired level.  Note
    that this will severely impact write performance.
-------------------------------------------------------------------*/
#define FFX_DEVMGR_READBACKVERIFY       FALSE
#define FFX_MEDIAMGR_READBACKVERIFY     FALSE



            /*------------------------------------------*\
             *                                          *
             *          Miscellaneous Settings          *
             *                                          *
            \*------------------------------------------*/


/*-------------------------------------------------------------------
    FFX_MAX_PAGESIZE is a power-of-two value which defines the
    maximum size page supported in the driver.  Reducing this
    value to the same size page used by the hardware will result
    in more efficient memory usage.
-------------------------------------------------------------------*/
#define FFX_MAX_PAGESIZE                (2048)


/*-------------------------------------------------------------------
    Optional _sysdelay() Timing Parameters

    These optional timing parameters are for systems that lack a
    microsecond-resolution time reference for implementing the
    _sysdelay() function.  They define the characteristics of a
    delay loop that is used to control timing of FIM operations.

    The _sysdelay() functionality is only required by the NOR and
    Sibley FIMs.

    These values may be automatically calculated at initialization
    time by calling FfxDelayCalibrate(), typically from the
    FfxHookDriverCreate() function in fhinit.c.  The resulting
    numbers can then be plugged in here to avoid the overhead of
    automatic calculation at run-time.

    See the FlashFX documentation for information on customizing
    this feature.
-------------------------------------------------------------------*/
/*  #define FFX_DELAY_SHIFT    (11)                         */
/*  #define FFX_DELAY_MICROSEC (1UL << FFX_DELAY_SHIFT)     */
/*  #define FFX_DELAY_COUNT    (300UL * FFX_DELAY_MICROSEC) */



/*#define FFX_COMPACTIONMODEL FFX_COMPACT_SYNCHRONOUS*/


#endif /* FFXCONF_H_INCLUDED */
