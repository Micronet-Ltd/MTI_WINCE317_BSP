
        Datalight FlashFX Tera SDK for Microsoft Windows CE
        Flash Media Manager
        Copyright (c) 1993-2012 Datalight, Inc.
        All Rights Reserved Worldwide.
        Version 2.1.1 Build 2128DF created on 04/24/2012
        --------------------------------------------------------------


        Welcome to the Datalight FlashFX Tera Software Developer's Kit,
        for Win32 hosts.

        The FlashFX Tera documentation is available in electronic format
        in the "doc" directory of the FlashFX Tera tree.


---[ Introducing FlashFX Tera ]----------------

        FlashFX Tera v2.1.1 is a release for all ports.  See the the OS
        specific portion of this readme.txt file for more details,
        as well as the respective FlashFX Tera and DCL history.txt files
        for a more comprehensive change list.

        -----------------------------------------
        FlashFX Tera v2.1 is a release for all ports.  See the the OS
        specific portion of this readme.txt file for more details,
        as well as the respective FlashFX Tera and DCL history.txt files
        for a more comprehensive change list.

        This version introduces support for Micron ClearNAND devices,
        the i.MX51 controller, the MTD FIM (See FlashFX Tera documentation
        for details), as well as support for Windows Embedded Compact
        7, VxWorks 6.9, and a number of bug fixes.

        For important notices about ClearNAND support and other issues,
        please see the "Known Issues" section at the end of this file.

        -----------------------------------------
        FlashFX Tera v2.0.1 is an update to the FlashFX Tera family.  See
        the OS specific portion of this readme.txt file for more details,
        as well as the respective FlashFX Tera and DCL history.txt files
        for a more comprehensive change list.

        This version fixes an issue where disk scrubbing recommendations
        were not getting properly returned up the call stack, resulting
        in passive scrubbing rather than active scrubbing.

        -----------------------------------------
        FlashFX Tera v2.0 is a major update to the FlashFX Tera family.
        Updates and additions are too numerous to list here; refer to
        the "history.txt" file for details.  Updates fall into the
        following categories:

        - Disks may now span Devices.  Historically, FlashFX Tera did not
          allow such configuration.  Refer to the FlashFX Tera documentation
          for details.

        - FlashFX Tera v2.0 introduces the QuickMount feature which permits
          VBF-enabled disks to save state information when unmounting
          disks.  When these disks are remounted, this state information
          is used rather than scanning the partition.  This can boost
          mounting speed by an order of magnitude or more, particularly
          for larger disks.

        - All ports have been updated to support the latest or nearly-
          latest release of their respective OS.  See the "readme.txt"
          files for the individual ports for details.

        - There have been numerous bug fixes and enhancements.  Refer
          to the "history.txt" file for details.

        - Several NTMs and the NAND FIM have been enhanced and updated.

        -----------------------------------------
        FlashFX Tera v1.2.3 is an update to v1.2.2.  See the OS specific
        portion of this readme.txt file for more details, as well as the
        respective FlashFX Tera and DCL history.txt files for a more
        comprehensive change list.

        This release contains the following general enhancements over the
        previous version:

        - Support for the Micron 34nm "M60" product line has been added.

        - A variety of issues surrounding multi-chip configurations were 
          fixed. Most of these issues were in FMSLTEST and some in the
          BBM module. These issues affect the use of raw FML disks in
          FlashFX Tera- see the "readme-issues.txt" file for more
          information.

        - The FXImage tool has been simplified so that it can be built
          without linking FlashFX Core libraries.  The tool now only 
          works in "VBF" mode, and creates images that only contain
          user data and FlashFX tags.  Raw and FS modes are no longer
          available.  Note that this change applies only to the v1.x
          product line.

        -----------------------------------------
        FlashFX Tera v1.2.2 is an update to v1.2.1.  See the OS specific
        portion of this readme.txt file for more details, as well as the
        respective FlashFX Tera and DCL history.txt files for a more
        comprehensive change list.

        This release contains the following general enhancements over the
        previous version:

        - Added support for Linux kernel 2.6.34.

        - Added support for block I/O discard requests as implemented in
          Linux kernel 2.6.28 and later.

        - Updated a variety of modules to build cleanly with the TI Code
          Composer Studio tool chain v4.0.

        -----------------------------------------
        FlashFX Tera v1.2.1 is an update to v1.2.  See the OS specific
        portion of this readme.txt file for more details, as well as the
        respective FlashFX Tera and DCL history.txt files for a more
        comprehensive change list.
 
        - Added support for Linux kernel 2.6.33.

        - The sample Linux boot loader project has been updated to be fully
          functional.  It now uses the Reliance Nitro Reader (available
          separately).  Using this boot loader, it is possible to run
          Linux on a LogicPD MX31-lite entirely from NAND flash managed
          by FlashFX Tera and formatted with Reliance Nitro as the root
          file system.

        -----------------------------------------
        FlashFX Tera v1.2 is an update to v1.1, and it contains the
        following significant enhancements.  See the history.txt file for
        an exhaustive list.
 
        - Fixed several issues in BBM and the Device Manager pertaining
          to correctly using reserved space and range checking.

        - Fixed a number of issues in the NOR FIM (norfim.c).

        - Refactored a number of headers and source files to support the
          various proprietary, public, and dual-licenses.  For non-Linux
          SDKs, all the code in the SDK is covered under the standard
          Datalight proprietary license, or the proprietary half of the
          dual license.  For Linux SDKs, some code is provided under a
          BSD license, which is compatible with GPL v2.


---[ FlashFX Tera Overview ]-----------------------

        FlashFX Tera is a major new addition to the FlashFX product line.

        The product supports MLC flash, as well as flash arrays up to
        2 terabytes in size, exceeding the FlashFX Pro limit of 2GB.
        FlashFX Tera supports all the modern SLC NAND flash parts, as
        well as NOR flash, which have been supported by FlashFX Pro.

        FlashFX Tera uses a new media format which is not compatible with
        FlashFX Pro.  Media formatted with FlashFX Pro must be reformatted
        before it can be used with FlashFX Tera.  Likewise, once media
        has been used with FlashFX Tera, it cannot be used by FlashFX Pro
        without reformatting.  See the section "Migrating Media from an
        Old Version" for more information.

        The following is a short list of some of the new features and
        significant changes.  See the history.txt file for an exhaustive
        list.

        - A new BBM scheme has been added to provide each chip in an FML
          Disk its own BBM table.  This greatly simplifies manufacturing
          issues.
        - Custom and default error management policies are supported.
        - Supports hardware and software ECC, with hooks for OEM customized
          ECC processing.
        - Supports active scrubbing of bit errors.
        - Supports Samsung Flex OneNAND.
        - Reserved space for NAND is no longer handled in the NTM, but
          rather is dealt with in the Device Manager.
        - FlashFX Tera no longer supports NAND with the "AnyBitZero"
          style of marking factory bad blocks.


---[ MLC Parts Supported ]-----------------------

        FlashFX Tera supports a variety of MLC flash parts from Micron,
        Numonyx, and Samsung.  See the "FlashFX Supported Hardware.pdf"
        for detailed information.


---[ Target Hardware Requirements ]--------------

        Flash Hardware Requirements

        FlashFX Tera requires that processors and flash parts operate within
        the defined hardware specifications and cannot safeguard against
        all types of hardware errors.  While the hardware is operating
        within specification it is expected that FlashFX Tera will protect
        and maintain the disk structures.

        FlashFX Tera requires that the flash parts are operated under the
        conditions and limits in their manufacturers' specifications.
        While the hardware is operating within specification FlashFX Tera
        reliably protects and maintains stored data even if an unexpected
        interruption of operations occurs.

        Power Loss Requirements

        A key specification that is sometimes overlooked in hardware
        designs is power during flash operations.  Manufacturers'
        specifications for flash parts generally require power supply
        voltages to remain within specified limits during a program or
        erase operation.  Many modern NAND flash parts recommend against
        or prohibit removing power during these operations.  MLC NAND in
        particular is sensitive to this, as interrupting programming of
        a page may unrecoverably corrupt the contents of another page.

        FlashFX Tera includes features to attempt to mitigate the effects
        of an interrupted program operation on NOR and SLC NAND flash,
        but this can never be entirely reliable.  FlashFX Tera does not
        support recovery from an interrupted program operation on MLC
        NAND flash.

        Interrupted erase operations are detected and handled correctly
        by FlashFX Tera.


---[ Migrating Media from an Old Version ] ------

        FlashFX Tera uses a different metadata format than older versions
        of FlashFX, and there is no "field update" ability or other automatic
        way to migrate any existing data to the new format, short of copying
        the files off the device, and replacing them after it has been
        reformatted.

        Furthermore, the media must be reformatted in a manner that allows
        FlashFX Tera to correctly recognize the original factory marked
        bad blocks, and not misinterpret any old metadata such that it
        thinks that <every> block is bad.

        Reformatting the flash can be accomplished in a couple different
        ways:
        - Use the old version of FlashFX Pro to "unformat" the flash,
          which essentially wipes the flash but does not put any new
          FlashFX format onto the media.
        - Use a JTAG debugger, or some such tool, to wipe the flash (but
          preserve any factory bad block markers, if at all possible).

        After either of these operations, the flash will appear to be "new"
        to FlashFX Tera, and it can use its normal formatting process,
        placing the new style metadata onto the flash.


---[ Activating FlashFX Tera ]-------------------

        FlashFX Tera ships as an evaluation SDK that is limited to 25%
        of the flash part. When an SDK is purchased, you will receive a
        License ID and an Activation Key that must be coded in the
        ffxconf.h file for your project.  This activation procedure
        will remove the 25% limit.


---[ FIM Errata ]--------------------------------

        The AMD/Spansion MirrorBit FIMs implemented in the amb2x16.c and
        ambx16.c files will support the M-series MirrorBit chips.  However
        these particular chips contain a complementary write disturb issue
        which is described in an application note available from AMD/Spansion.
        FlashFX does not address this issue.

        This write disturb problem has been resolved in N-series chips,
        which Datalight supports using the same FIMs.  Datalight does not
        recommend using these FIMs with the M-series chips.

        Additionally, due to the unusual nature by which the status bit
        bit is toggled on these chips, making the code thread-safe is
        problematic.  Therefore, if these FIMs are used, you must ensure
        that you only mount one VBF Disk on the Device, OR, you must modify
        the FIM to disable the EraseSuspend functionality.  Please contact
        Datalight for more information on these issues.



---[ Windows CE Specific Information ]-----------

        FlashFX Tera version 2.1 is a minor upgrade to the FlashFX Tera
        product line.  It includes support for Windows CE 7.  For
        details, refer to the "history.txt" file.

        FlashFX Tera version 2.0 is a major upgrade to the FlashFX Tera
        product line, and contains many significant upgrades and
        additions to the Windows CE port.  For details, refer to 
        the "history.txt" file.  Highlights are:

        - Support for Windows CE up to 6.0R3.    
        - FlashFX_FML, a sample application that demonstrates use
          of the FML interface.
        - Supports Power Suspend and Resume functionality.
        - Fixes remaining CETK test failures.
        - Supports the concept of a store.

        FlashFX Tera version 1.2.3 addresses an issue in which the Windows
        CE block device driver experienced failures in the Windows CE
        Test Kit (CETK) suite of tests.  All CETK tests now pass, except
        4026 and 4027, in which the block driver responds to invalid or
        unsupported IOCTL with an unexpected error code (0x1f).

        FlashFX Tera version 1.2 is a major new addition to the FlashFX
        product family, with significant enhancements to seamlessly
        support Microsoft Windows CE -- completely replacing the native
        flash solution.

        FlashFX Tera version 1.0 is a major new addition to the FlashFX
        product family, with significant enhancements to seamlessly
        support Microsoft Windows CE -- completely replacing the native
        flash solution.

        See the history.txt file for an exhaustive list of enhancements
        for Windows CE.


---[ Development System Requirements ]-----------

        The minimum development system requirements for using the
        FlashFX Tera SDK for Windows CE are:
        - Microsoft Windows CE 5.0 or later
        - Any system capable of running Microsoft Windows CE development
          tools.
        - 60MB of disk space for installation of the FlashFX Tera SDK.
        - Sufficient RAM for development tools.


---[ Preconfigured Projects ]--------------------

        This FlashFX Tera SDK ships with preconfigured projects for some
        common reference platforms.  These projects can be found under
        the [projects] directory, in the FlashFX Tera installation.

        Review the README files contained in each of the various
        [projects] directories for more information about using
        FlashFX Tera on that platform.

        The following Windows CE specific projects are included:

        beagleboard     - BeagleBoard platform
        bsquarePXA320   - BSQUARE PXA320 reference platform
        dbau1500        - AMD Alchemy DBAu1500 reference platform
        Mistral_OMAP35x - Mistral OMAP35x reference platform
        MX31BL1NAND     - MX31 Bootloader project using OneNAND
        MX31CE1NAND     - MX31 Driver project using OneNAND
        MX31CENAND      - MX31 Driver project using NAND
        OSK5912CE1NAND  - OMAP5912 Driver project using OneNAND
        OSK5912CENAND   - OMAP5912 Driver project using NAND
        pxa250          - Intel DBPXA250/255 reference platform
        pxa270          - Driver project for the Intel DBPXA270
        pxa270bl        - Bootloader project for the Intel DBPXA270
        sandgate3_ce    - Sophia Systems PXA320
        TI_SDP3430      - Driver project for the TI SDP3430
        TI_SDP3430BL    - Bootloader project for the TI SDP3430


---[ Known Issues in this SDK ]------------------

        - When using the FXFMT command-line format utility, it is highly
          recommended that the /Part:fs option be used to specify that
          an MBR be written to the disk.

          If this option is not used, upon reboot the FlashFX Tera drive will
          either fail to load or be reformatted, depending on the "Format"
          registry entry.

        - Persistent Registry Storage (non-hive based) functionality from
          previous versions of FlashFX has not been updated to use the new
          Datalight Loader abstraction located in DCL.  This functionality,
          located in regread.c in the WINCEBL abstraction has been disabled.

          Standard hive-based registry storage is recommended.  Please
          contact Datalight Support for questions about the old style
          Persistent Registry Storage.

        - The FlashFX Tera command-line build process requires environment
          variables that are typically set up by Platform Builder.  To gain
          access to these variables with a minimal amount of hassle, from
          within Platform Builder, choose "Open Release Directory" from the
          "Build OS" menu.  This will give you a command-line prompt that
          has the necessary variables set.  You can then conveniently use
          the FlashFX Tera build batch files described in the developer's
          guide.

        - While the Platform Builder 4.2 build capability from within the
          GUI still exists, it is not documented in this release.  Datalight
          highly recommends building FlashFX using the command-line and the
          MSWCE ToolSet abstraction.  If you must build within the CE 4.2
          IDE, please contact Datalight support.

        - The newproj.bat file syntax incorrectly lists "x86" as a valid CPU
          type.  Use "386" instead (Bug 2826).

        - FML Raw Page/Block Status

          The FML "Raw" block and page interfaces do little or no range
          checking. It is very important to use these interfaces with 
          extra caution to sure they are placed properly within the range
          spanned by an FML disk. This is especially true in multi-chip
          configurations. Corruption of BBM tables could result otherwise.
          If in doubt, please contact Datalight customer service.

        - Micron ClearNAND Support

          The support for Micron ClearNAND comes in the form of two
          NTMs, one for Standard ClearNAND and one for Enhanced
          ClearNAND. There are a couple of important issues to be
          discussed about this support.
      
          - FlashFX support for ClearNAND was developed using sample
            devices. These sample devices had a known issue in which
            the spare area size was smaller than published in the
            datasheet. Because of this, each NTM has a macro defined,
            "<DEVTYPE>_DEVICE_ESAMPLE", that allows compensation for
            this to be switched on or off.
          - Support for Enhanced ClearNAND treats the device as a
            standard NAND device. Enhanced ClearNAND contains many
            performance-improvement features such as multiple
            volumes and a pipelined command architecture that allows
            software to run multiple devices each containing 
            multiple LUNs concurrently. FlashFX does *not* currently
            take advantage of those features.

        - MX35 NTM

          This release has an NTM in it that supports the i.MX35
          controller. It is functional but not qualified by
          Datalight, and should be considered experimental.
---[ History File ]-------------------------                    

        Please refer to the history.txt file for more details on 
        changes to FlashFX Tera since previous releases.          


---[ Contacting Datalight ]-------------------------------------------

        Datalight can be contacted at:

        Datalight, Inc              Phone:  425-951-8086
        21520 30th Drive SE                 800-221-6630
        Suite 110                   Fax:    425-951-8094
        Bothell, WA  98021          Web:    http://www.datalight.com

