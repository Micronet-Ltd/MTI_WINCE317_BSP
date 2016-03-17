REM
REM              Texas Instruments OMAP(TM) Platform Software
REM  (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
REM 
REM  Use of this software is controlled by the terms and conditions found
REM  in the license agreement under which this software has been supplied.
REM
REM              Micronet ltd. CE317 Platform Software based on OMAP(TM)
REM  (c) Copyright Micronet ltd. All Rights Reserved.
REM 

REM --------------------------------------------------------------------------
REM This batch file is configured for the CE317.
REM
REM This setup should be used with an OS configuration similar to that of 
REM the AM(DM)37XX TI processors.
REM --------------------------------------------------------------------------
REM
REM --------------------------------------------------------------------------
REM Notes
REM --------------------------------------------------------------------------

REM After making changes to this file, recommended build procedure is:
REM
REM Use Platform builder menu: Build, Open Release Directory in Build Window
REM enter command: cd "%_TARGETPLATROOT%"
REM enter command: build -c
REM close build window
REM Use platform builder menu: Build, Build Current BSP and Subprojects
REM
REM Note that a sysgen must have been completed before using the above procedure. 

REM --------------------------------------------------------------------------
REM Build Environment
REM --------------------------------------------------------------------------

REM Always copy binaries to flat release directory
set WINCEREL=1
REM Generate .cod, .lst files
set WINCECOD=1

REM ----OS SPECIFIC VERSION SETTINGS----------

if "%SG_OUTPUT_ROOT%" == "" (set SG_OUTPUT_ROOT=%_PROJECTROOT%\cesysgen) 

set _PLATCOMMONLIB=%_PLATFORMROOT%\common\lib
set _TARGETPLATLIB=%_TARGETPLATROOT%\lib
set _EBOOTLIBS=%SG_OUTPUT_ROOT%\oak\lib
set _KITLLIBS=%SG_OUTPUT_ROOT%\oak\lib
set _RAWFILETGT=%_TARGETPLATROOT%\target\%_TGTCPU%

set BSP_WCE=1

REM --------------------------------------------------------------------------
REM NAND Selection
REM --------------------------------------------------------------------------
set BSP_NAND_FMD=

REM --------------------------------------------------------------------------
REM Initial Operating Mode - VDD1 voltage, MPU (CPU) and IVA speeds
REM --------------------------------------------------------------------------

REM Select initial operating point for VDD1 voltage, CPU speed and IVA (DSP) speed.
REM Note that this controls the operating point selected by the bootloader.
REM If the power management subsystem is enabled, the initial operating point 
REM it uses is controlled by registry entries.
REM Use 4 for MPU[1000Mhz @ 1.3125V], IVA2[875Mhz @ 1.3125V], CORE[400Mhz @ 1.1375V] (OPMTM)
REM Use 3 for MPU[800Mhz  @ 1.2625V], IVA2[660Mhz @ 1.2625V], CORE[400Mhz @ 1.1375V] (OPM120)
REM Use 2 for MPU[600Mhz  @ 1.1000V], IVA2[520Mhz @ 1.1000V], CORE[400Mhz @ 1.1375V] (OPM100)
REM Use 1 for MPU[300Mhz  @ 0.9375V], IVA2[260Mhz @ 0.9375V], CORE[400Mhz @ 1.1375V] (OPM50)
REM Use A for MPU auto config
set BSP_OPM_SELECT=A
set BSP_CDS=1

REM Note that the initial MPU speed is what will be reported by IOCTL_PROCESSOR_INFORMATION.

REM --------------------------------------------------------------------------
REM Power Management Subsystem Settings
REM --------------------------------------------------------------------------

REM These should always be unset
set BSP_NOPMEXT=
set BSP_NOPMXPROXY=

REM There are 3 supported advanced power management configurations:

REM DVFS off, Smart Reflex off (default)
set BSP_NODVFS=1
set BSP_NOCPUPOLICY=1
set BSP_NOINTRLAT=1
set BSP_NOSYSSTATEPOLICY=1
set BSP_NOSMARTREFLEXPOLICY=1

REM DVFS on, Smart Reflex off
REM set BSP_NODVFS=
REM set BSP_NOCPUPOLICY=
REM set BSP_NOINTRLAT=
REM set BSP_NOSYSSTATEPOLICY=
REM set BSP_NOSMARTREFLEXPOLICY=1

REM DVFS on, Smart Reflex on
REM set BSP_NODVFS=
REM set BSP_NOCPUPOLICY=
REM set BSP_NOINTRLAT=
REM set BSP_NOSYSSTATEPOLICY=
REM set BSP_NOSMARTREFLEXPOLICY=

REM --------------------------------------------------------------------------
REM Other Settings
REM --------------------------------------------------------------------------

REM Set the following if the MMC1 controller should be configured in low voltage mode (1.8V instead of 3V)
set MMCHS1_LOW_VOLTAGE=
set MMCHS2_LOW_VOLTAGE=1

REM Set to select the BQ2700 battery driver, comment out to select NULL battery driver.
REM Note that the BQ27000 battery driver requires the HDQ driver - comment out "set BSP_NOHDQ=1" below.
REM Note that the BQ27000 battery driver is untested and unsupported.
REM CE_300
set BSP_BATTERY_BQ27000=
set BSP_CHARGER_BQ24030=

if EXIST %_TARGETPLATROOT%\SRC\TEST\UTILS\MAKE.BAT set PATH=%PATH%;%_TARGETPLATROOT%\SRC\TEST\UTILS

REM ----BOARD CONFIGURATION----------
REM Set the following to eliminate unsupported or undesired features
REM These variables can not be overridden within the Platform Builder IDE
rem No driver support for the following features
set BSP_NOMADC=
set BSP_NOHDQ=
REM controls inclusion of the unsupported camera driver
set BSP_NOD3DM=1
set BSP_NOPCIBUS=1
set BSP_NOUSBSER=1
set BSP_NOBLUETOOTH=
set BSP_NOIRDA=1
set BSP_NOHEADSET=

REM Enables sample watchdog timer, see SRC\OAL\OALWATCHDOG for source code.
rem set BSP_WATCHDOG_ENABLE=

REM --------------------------------------------------------------------------
REM Display Configuration
REM --------------------------------------------------------------------------

REM Set to enable composite/S-VIDEO (P14) output by default.
set BSP_TVOUT_ENABLE=

REM Set to enable composite video output (P6 RCA jack) , S-VIDEO (P14) is the default.
REM When using composite output CPU module SW1 position 6 must be OFF.
set BSP_TVOUT_COMPOSITE=

REM Set to enable PAL format video output (COMPOSITE on P6 or S-VIDEO on P14).
REM Note that this option is provided as an example only, it is untested and unsupported.
set BSP_TVOUT_PAL=

REM If the Virtual Rotated Frame Buffer is enabled, then the display can
REM be rotated using DM3730 hardware support with less performance penalty
REM than when using the flat frame buffer. The DSP based video CODECs supplied 
REM with the BSP will work with VRFB enabled, but the video CODECs supplied 
REM with Windows CE will not work  due to the large stride used by the VRFB.
set BSP_NOVRFB=1

REM --------------------------------------------------------------------------
REM These variables control drivers for the DM3730 musb block
REM --------------------------------------------------------------------------

REM if this not set, then all musb drivers will be removed the image
set BSP_USBOTG=1

REM Note that all of these drivers are required for any OTG port functionality.
set BSP_NOUSBHSOTG=
set BSP_NOUSBHSFN=
set BSP_NOUSBHSHCD=

REM These variables select the type of OTG tranceiver used to connect the musb block to the EVM OTG port.
REM Set only one variable, both are ignored if BSP_NOUSBHSOTG is set. 
REM Note that default resistor options on the EVM connect the REM musb block to the ISP1504 tranceiver, 
REM so BSP_USBHSOTG_ISP1504 is set by default. Select BSP_USBHSOTG_TPS659XX only if the resistor options 
REM are changed. See the EVM schematics for details. Note that EVM TPS659XX tranceiver support is untested.
REM If these variables are changed, a clean build is required.
set BSP_USBHSOTG_ISP1504=
REM Support for the TPS659XX OTG transceiver is untested and unsupported.
set BSP_USBHSOTG_TPS659XX=1

REM --------------------------------------------------------------------------
REM HSUSB USB host port configuration
REM --------------------------------------------------------------------------

REM Note: If the HSUSB port variables are changed, a clean build and XLDR/EBOOT update are required.
REM Note: There may be restrictions for both the MM1 and MM2 ports, see the OMAP documentation/errata.
REM Note: Configuration of the HS USB ports is done by XLDR/EBOOT and EHCI driver (12 pin mode).

REM EVM EHCI driver can only support the DM3730 USB host port1 (mm1) or host port2 (mm2) using an expansion board.
REM Set BSP_NOUSBEHCI to disable USB EHCI driver, clear/comment out to enable.
set BSP_NOUSBEHCI=
set BSP_NOUSBOHCI=1
set BSP_NOUSBAUDIOCLASS=


REM --------------------------------------------------------------------------
REM UART Driver Configuration
REM --------------------------------------------------------------------------

REM Set to remove UART driver.
set BSP_NOCOM=

set BSP_NOCOM1=
set BSP_NOCOM2=1
REM Disable COM2/Uart3 when used for debug messages
set BSP_DEBUG_SERIAL=1
set BSP_NOCOM3=
REM The 37xx has a 4th UART, but the EVM does not have connections for it.
set BSP_NOCOM4=1

REM --------------------------------------------------------------------------
REM The following SPI drivers are available in the BSP but disabled on EVM3530
REM Note that the pins needed for these SPI ports may not be configured by the BSP,
REM check SRC\BOOT\XLDR\platform.c to verify pin configuration for your hardware design.
REM --------------------------------------------------------------------------

set BSP_NOSPI2=1
set BSP_NOSPI3=1
set BSP_NOSPI4=1

set BSP_NOMCBSP1=1
set BSP_NOMCBSP3=1

REM --------------------------------------------------------------------------
REM These variables control drivers for the DM3730 MMC/SD/SDIO controller
REM --------------------------------------------------------------------------

REM The EVM BSP supports one MMC/SD/SDIO slot using the MMC1 controller
set BSP_NOSDHC1=
REM Enables high speed SD card support for SDHC1 (MMC1).
REM Note that careful board layout is required to support this option.
set BSP_SDHIGHSPEEDSUPPORT_SDHC1=1

REM The EVM BSP does not support the MMC2 (SDHC2) controller
REM Note that this option is untested and unsupported
set BSP_NOSDHC2=
REM Enables high speed SD card support for SDHC2 (MMC2).
REM Note that careful board layout is required to support this option.
set BSP_SDHIGHSPEEDSUPPORT_SDHC2=1
set BSP_EMMCFEATURE=1

REM Audio --------------------------------------------------------------------
REM Set Audio configuration here independent of BSP_TPS_65950 
REM
set BSP_VOICE_PATH=

REM --------------------------------------------------------------------------
REM The following variables can be used to disable drivers on the EVM3530
REM --------------------------------------------------------------------------
set BSP_NOSPI1=
set BSP_NONAND=
set BSP_NONLED=1
set BSP_NODMA=
set BSP_NOGPIO=
set BSP_NOTLED=
set BSP_NOSPI=
set BSP_NOTWL=
set BSP_NORTC=
set BSP_NOMCBSP=
set BSP_NOBACKLIGHT=
set BSP_NOTOUCH=
set BSP_NOKEYBD=
set BSP_NOAUDIO=
set BSP_NOSDHC=
set BSP_NODISPLAY=
set BSP_NOPWRKEY=
set BSP_NOBATTERY=
set BSP_NOUSB=

REM Disables control panel applet that displays BSP version and build date
set BSP_NO_BSPINFO=

REM BSP_SDMAPROXY should be always set.
set BSP_SDMAPROXY=1

REM --------------------------------------------------------------------------
REM Misc. settings
REM --------------------------------------------------------------------------

REM disables VMINI (supports ethernet sharing of KITL connection)
set BSP_NOSHAREETH=1

REM TI OMAP shell extensions command line utility (do.exe)
REM Usage: Open a command prompt from the start menu.
REM        Type "do" followed by the command you want to use. 
REM        Example: "do ?" will show the help and available commands.
REM Note: If a KITL connection is available, the TI OMAP shell extensions can be used from the target shell.
set BSP_SHELL_EXTENSIONS_DO_CMD=1

REM --------------------------------------------------------------------------

REM TI BSP builds its own ceddk.dll. Setting this IMG var excludes default CEDDK from the OS image.
set IMGNODFLTDDK=1

set IMGENABLEHEAPSENTINEL=1

set BSP_XRPLUGIN_DDRAW=1
set BSP_XRPLUGIN_OPENGL=
set SYSGEN_SAMPLEXAMLPERF=

set sysgen_msmq_srmp=1
set sysgen_msmq_visadm=1
set SYSGEN_PRINTING=1
set SYSGEN_PCL=1

set BSP_OEM_POOL=1
set BSP_NO_IGNITION=
:EXIT
