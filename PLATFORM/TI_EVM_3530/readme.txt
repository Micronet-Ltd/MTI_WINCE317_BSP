*************************************************
Notes
*************************************************

In this BSP, EVM or EVM1 refers to the TMDXEVM3503 platform and EVM2
refers to the TMDSEVM3530 platform.

When using TMDSEVM3530 (EVM2), OMAP EVM MAIN BOARD REV G is required.

BSP configuration is handled by the ti_evm_3530.bat file. Note that this
file also contains a lot of information about the BSP.

*************************************************
Release History
*************************************************

<8/27/2010> 6.15.00

- Improved support for the SYSGEN_PM_PDA power manager model. This
feature is unsupported and is provided as a starting point for customers
who want to use the PDA power management model. Note that when using
SYSGEN_PM_PDA, all wake sources (except the PWR_ON button) will cause
the system to resume to the "Unattended" mode.

- Fixed problem with gpio API (gpio.h) that prevented inline functions
from working for processes other than the kernel process.

- Added code to disable/enable all GPIO FCLKs and ICLKs during
suspend/resume. Added code to restore GPIO debounce registers when OAL
GPIO context is restored. This fixes a problem with GPIO pins that use
the GPIO hardware debounce feature after suspend/resume.

- Fixed typo that swapped CM_CLKEN_DSS1 and CM_CLKEN_DSS2 in the _fclk_DSS1 
and _fclk_DSS2 definitions.

- Fixed problem with TWL_INTR_USB_PRES interrupt mask control.

- Fixed short packet handling in CQTD::ReadDataIN function of OTG driver.

- Modified the retries manner to leave the while loop when the notification 
handle is created successfully.

- Fixed the VDD1/VDD2 don't drop to 0 in OFF mode issue.

- Fixed the AUTO_OFF and AUTO_RET are both set in OFF mode contrary to 
OMAP hw reference manual issue.

- Added the SMSC Ethernet driver packaging.

- Fixed EHCI - suspend/resume - intermittently, resume takes effect after 
30 seconds issue.

<6/16/2010> 6.14.01.01

- Fixed problem with VFP/NEON save/restore of FPEXC register during
suspend/resume.

- Added workaround for problem with VFP/NEON register save/restore
during thread switching.

- New version of TI_Multimedia_readme file.

- Fixed problem in SDHC driver that caused "ERROR: Power Handler
function yield to low priority thread" during suspend/resume without a
card in the socket.

- Improved suspend/resume behavior for OTG port. Enabled high speed mode
by default for both OTG host and function modes.

- Merged USB20 changes from Windows CE QFE into EHCI and OTG host
(musbhcd) drivers to improve behavior when suspend/resume occurs during
device enumeration/configuration. Requires the March 2010 CE6R3 QFE.

- Improved power management subsystem suspend/resume behavior, changed
suspend state to CHIP OFF mode and added save/restore for VFP/NEON
registers. Note that in CHIP OFF mode the VDD1 and VDD2 power supplies
will remain on due to an EVM1/EVM2 power module hardware design issue.

- Renamed ti_systemstatepolicy DLL to ti_systemstatepolicy_omap35xx_tps659xx_ti_v1 
to conform to BSP convention. Added missing platform.bib entry for same.
Added retries to RequestPowerNotifications during system state policy
driver init in case PM subsystem is not ready when system state policy
adapter starts. 

- Fixed suspend/resume clock management issue in EHCI driver. Added
registry entry to enable/disable ECHI root port bus suspend mode
(disabled by default) to work around EHCI PHY compatibility issue. See
the known hardware issues section for more information.

- Fixed problem in wave driver configuration that caused audio recording
to be mono only.

- Fixed OALIoCtlHalDumpRegisters return code handling (code used for
debugging only, no effect on default BSP configuration).

- Fixed typo in sdbus driver SDHC2 support (no effect on default BSP
configuration).

- Corrected error in SPISetSlaveMode function.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- Ethernet driver fails some CETK tests.

- If the VRFB is enabled (it is enabled by default), the video capture
driver (camera_mdc) and the sample video capture application will
malfunction.

- The do.exe command line utility may not able to perform some types of
CE shell commands, use the Platfrom Builder Target->Target Control shell
with a KITL link to execute commands that do not work using do.exe.

- BSP configuration using the platform builder catalog is not supported.
Configure the BSP by editing the ti_evm_3530.bat file.

- The USB OTG port is sensitive to the type/length of USB cable used.
The cause of this issue is not known. Recommended cable is GoldX USB 5
in 1 (GXQU-06) available from www.amazon.com.

- The MDC/EVM2 camera (video capture) driver requires a large amount of
free program memory (~30 MB) when used with the video capture demo
program. Use the control panel, system icon, memory tab to adjust the
memory allocation. After running a video capture demo program, a large
memory leak was observed. It is not known if this is due to a problem
with the demo program or with the camera driver.

- If a USB device is inserted in the EHCI port during system suspend,
the suspend current will be higher. This is because the root hub port
cannot be placed into suspend mode due to a OMAP3 hardware design
incompatibility with the EHCI PHY chip used on the MDC and EVM2. See the
known hardware issues section for more information.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or EVM2, plugging a low/full speed device into the USB
port on the daughter card will cause the port to become non-functional
until a reboot. See Known Hardware Issues for more information. To work
around this issue, use a hub between the USB port and the low/full speed
device. This behavior is by design (OMAP3xxx chip), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the software based video codecs provided by Microsoft. No
fix is expected. If the software based video codecs are needed, edit the
ti_evm_3530.bat file to set BSP_NOVRFB=1. No fix is expected.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- EVM1 only. When the BSP_DSPLINK environment variable is set, a large
amount of RAM is allocated to the DSP subsystem and debug images can
become too large to boot. To work around this problem either disable the
CODEC Engine support components when doing a debug build or remove
components from the OS configuration that are not needed for the debug
session.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up. No fix is
expected.

- EVM1 only. The RTC emulation code does not support alarms.

<4/20/2010> 6.14.00

- Fixed problem with VAUX2 voltage (used by EHCI transceiver) which was
being set to 2.8V instead of 1.8V.

- Added work around to MUSBOTG driver for problem with VBUS error
handling.

- Changed SMSC Ethernet driver to include source code.

- Modified the SMSC Ethernet driver to pass CETK tests. Note that this
modification may reduce network performance slightly and should be
reviewed by the end user. See TI_EVM_3530\SRC\DRIVERS\SMSC911X\lan9118.c 
[line 95, NdisStallExecution(100UL)].

- Updated DVSDK (DSP CODEC engine) components. Added AAC decoder, 3GPP
demux support, and MP3 decoder DSHOW wrapper. Also moved DVSDK .bib and
.reg file entries to separate include files.

- Changed tick timer interval for work around problem with CETK
scheduler test failure.

- Changed camera_mdc driver to remove STILL Pin function. Modified
sample video capture application to work with this change.

- Fixed thread priority issue that caused problems with USBFN RNDIS
class driver.

- The default configuration of the BSP (ti_evm_3530.bat) is now set up
for the EVM3530 (EVM2). Note that the default configuration for previous
BSP releases was for EVM3503 (EVM1).

- EVM2 only: VRFB (Virtual Rotated Frame Buffer) support is now enabled
by default in all images and platform configuration .bat files. Note
that the software based video decoders supplied with Windows CE will not
operate correctly with VRFB enabled. If any software based video
decoders are needed, edit the ti_evm_3530.bat file to set BSP_NOVRFB=1.
Note that EVM1 boards that use ES2.x silicon cannot use the VRFB due to
a silicon issue.

- The SYSGEN_DSHOW_GENERICSTREAMER variable is now set in all of the
sample OS designs. Note that this variable does not have a Platform
Builder catalog entry associated with it and must be either manually
added to the .pbxml file or set in the ti_evm_3530.bat file (both have
been done for the TI_EVM_3530 BSP).

- Cleaned up errors when building with WINCESHIP set.

- Changed USB RNDIS KITL driver to use OAL PRCM to manage clocks.

- Changed custom PM.DLL sources file to honor SYSGEN_PM_PDA variable.
Note that this option is untested and unsupported.

- Fixed problem that caused incorrect RTC time after suspend/resume
(EVM2 only).

- Fixed syntax errors in tsp659xx MUSB OTG transceiver support. Note
that this feature is untested and unsupported.

- Changed RTC intialization to avoid losing time unless PMIC chip is
reset or has its power turned off. Time now correct after OMAP_RST
button is used to reset the EVM.

- Fixed problem with OAL RTC code that caused CETK timer drift test
failure.

- Fixed problem with OTG driver that caused OTG port to stop working
after the first time a cable is plugged in.

- Changed pin mux operation order to ensure block functional clock is
enabled prior to switching the pins ownership from GPIO to the block.

- Fixed problem with SDHC driver that caused some SDIO cards to fail.

- Changed SDBOOT to support for 4 bit wide SD memory cards. 

- Attempted to improve support in the SDHC driver for the second MMC
controller (MMC2/SDHC2). Note that this feature is untested and
unsupported.

- Changed custom SDMEMORY card driver to automatically detect need for
multi block read workaround.

- Added kernel IOCTL functions to return CPU family and CPU revision.

- Added bootloader menu entry to enable/disable retail messages (UART3).

- Added custom SDBUS driver that supports high speed SD cards. Modified
SDHC driver to support high speed cards.

- Fixed problem with NAND and ONENAND XLDR being too large.

- Added I2C bus lock/unlock functions to CEDDK API. These functions are
needed only when atomic multiple transaction bus access is required.

- Improved power management subsystem by updating to the latest code
base. Changed the default suspend state to OSWR (CORE+OTHER = OSWR, MPU
= CSWR). Note that OSWR = Off State With Retention and CSWR = Clock
Stopped With Retention. 

- Fixed error in OTG link recovery that used GPIO_186 for ULPI D5
instead of GPIO_189.

- Added environment variable to ti_evm_3530.bat to enable TV-OUT/S-VIDEO
output.

- Configured SYS_BOOT6 pin as GPIO output to drive PROC_ACT LED as NLED.

- Removed one NEON BLT that caused CE6R3 CETK GDI AlphaBlend and
GradientFill test failures.

- Removed all unsupported catalog items, use ti_evm_3530.bat to
configure the BSP.

- Added very preliminary support for EVM2 with DM3730 CPU chip
(EMV3730). Use the ti_evm2_3730.bat file (make a copy and rename the
copy to ti_evm_3530.bat) to select this configuration. The EVM_3503_MDC
sample OS design may be used as a starting point for working with the
EMV3730. Note that the EVM3730 support has had little testing and is
unsupported.

Known EVM3730 limitations: PowerVR, camera_mdc driver, sample video
capture application, shell extentions (for CPU registers), Smart Reflex
and suspend/resume cannot be used with the EVM3730 at this time. The
EVMFlashTool has not been updated to work with the EVM3730, so only SD
BOOT is supported. Note that because the EVMFlashTool cannot be used to
partition the NAND, the NAND file system driver cannot be used and is
disabled by default. Issues with the I2C bus driver have also been
observed.

- Support for UART4 driver (EVM3730 only). Note that the EMV
hardware does not have connections for UART4 and the default GPIO
alternate function configuration is not setup to support UART4.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- Ethernet driver fails some CETK tests. Note that this issue also
exists in earlier releases.

- Using the control panel to set suspend timer does not result in
automatic suspend. Cause is unknown. Note that this issue also exists in
earlier releases.

- EVM1 boards that use ES2.x silicon cannot use the VRFB due to a
silicon issue. No fix is expected.

- If the VRFB is enabled, the video capture driver (camera_mdc) and the
sample video capture application will malfunction.

- If a device is connected to the OTG port during suspend/resume it will
sometimes fail to enumerate. Remove and re-insert the cable connected to
the OTG port to work around this issue.

- The do.exe command line utility may not able to perform some types of
CE shell commands, use the Platfrom Builder Target->Target Control shell
with a KITL link to execute commands that do not work using do.exe.

- BSP configuration using the platform builder catalog is not supported.
Configure the BSP by editing the ti_evm_3530.bat file.

- The USB OTG port is sensitive to the type/length of USB cable used.
The cause of this issue is not known. Recommended cable is GoldX USB 5
in 1 (GXQU-06) available from www.amazon.com.

- The MDC/EVM2 camera (video capture) driver requires a large amount of
free program memory (~30 MB) when used with the video capture demo
program. Use the control panel, system icon, memory tab to adjust the
memory allocation. After running a video capture demo program, a large
memory leak was observed. It is not known if this is due to a problem
with the demo program or with the camera driver.

- Suspend/resume clock management for the HSUSB host EHCI driver is not
working. When this driver is included in the system, the suspend current
will be significantly higher.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or EVM2, plugging a low/full speed device into the USB
port on the daughter card will cause the port to become non-functional
until a reboot. See Known Hardware Issues for more information. To work
around this issue, use a hub between the USB port and the low/full speed
device. This behavior is by design (OMAP3xxx chip), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the software based video codecs provided by Microsoft. No
fix is expected. If the software based video codecs are needed, edit the
ti_evm_3530.bat file to set BSP_NOVRFB=1. No fix is expected.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up. No fix is
expected.

- EVM1 only. The RTC emulation code does not support alarms.

BSP Changes:

<1/15/2010> 6.13.00

BSP Changes:

- A preview of the DVFS (Dynamic Voltage and Frequency Scaling)
subsystem is now available for evaluation (disabled by default).
Enabling this subsystem (unset BSP_NODVFS, BSP_NOCPUPOLICY, and
BSP_NOINTRLAT) will cause the DVFS subsystem to vary the core, CPU and
DSP subsystem voltages and operating frequencies based on system
activity.

- Added custom versions of sample XAMLPERF applications, configured to
rotate the display to landscape mode (if needed and VRFB is enabled),
display frame rate and CPU load statistics.

- Fixed problem with video capture chip reset sequence that caused video
capture to fail after power on reset (it worked if OMAP_RST button was
used).

- The DSP based video decoders have been updated to work with the
display driver VRFB (Virtual Rotated Frame Buffer) mode. Rotated display
performance using VRFB is much better than when using the flat frame
buffer. Note that for large displays, the bandwidth required to scan the
display when it is rotated may be too large for the OMAP3 to suppport
even with VRFB. Note that the video decoders supplied with Windows CE
will still not work with VRFB.

- Improved suspend/resume behavior and reduced power consumption in
suspend state. Most drivers are now properly manageing clocks during
suspend/resume. Notable exceptions are the HS USB host EHCI and OHCI
drivers.

- Changed EHCI driver suspend/resume code to leave HSUSB and USBTLL
clocks enabled during suspend. This is done as a workaround for a
problem that causes the USBTLL clocks to be stuck in idle mode after
resume.

- Changed name of variable that selects bootloader initial operating
mode from BSP_OPP_SELECT to BSP_OPM_SELECT.

- Disabled SD card wakeup, disabled keypad wakeup, disabled EHCI wakeup.

- Changed OHCI/ECHI USB host driver load order to make EHCI driver load
first to avoid USBTLL reset in by EHCI driver from interferring with
OHCI driver TLL block configuration.

- Changed wave driver to restart audio streams after suspend/resume.

- Merged musb driver changes to improve suspend/resume behavior and
possibly improve TWL4030/TPS659xx transceiver compatibility. Note that
TWL4030/TPS659xx transceiver support is untested and unsupported.

- Changed names of sample OS designs to reflect actual EVM1
(TMDXEVM3503) and EVM2 (TMDSEVM3530) platform names. Removed XP skin
from EVM2 sample OS design.

- Added registry entry to specify current that musb OTG port can supply
in host mode. Set to 100mA for EVM1, 500mA for EVM2.

- Added source code for sample video capture application.

- Fixed duplicate IOCTL code definitions for IOCTL_PRCM_SET_SUSPENDSTATE
and IOCTL_PRCM_DOMAIN_SET_CLOCKSTATE.

- Corrected EVM2 TPS65950 PMIC version number display in bootloader.

- Added kernel IOCTL call that returns information about DSP (IVA2)
subsystem. For details, see OMAP35XX_TPS659XX_TI_V1\inc\oalex.h
(IOCTL_HAL_GET_DSP_INFO).

- Added power state clock management to camera_mdc driver.

- Added support for BSP_TI_DVSDK environment variable for use with the
TI DVSDK package. When using the DSP based CODECs from the TI DVSDK
package, unset all of the variables in the CODEC Engine support
components section of the TI_EVM_3530.bat file (BSP_DSPLINK, BSP_CMEM,
BSP_DSHOWFILTER, BSP_H264_DECODE_FILTER, BSP_MPEG4_DECODE_FILTER,
BSP_MPEG2_DECODE_FILTER and BSP_DMAI_SAMPLE_APPLICATIONS) and set
BSP_TI_DVSDK. Note that a clean build is required.

- Updated DSP CODEC subsystem binaries from DVSDK 1.00.00.05.

- Fixed problem with ethernet driver power management that could cause
exceptions during transitions between NDIS power states.

- Increased PWR_ON key debounce time.

- Added support for new operating point - MPU = 720MHz and IVA = 520MHz
at VDD1 = 1.35V.

- Renamed display driver in OS image to be ddi.dll. Fixed problem with
clock management in D3/D4 power states.

- Added environment variables to select NAND flash type, currently
supported are BSP_NAND_MT29F2G16 and BSP_NAND_MT29F4G16.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- The do.exe command line utility may not able to perform some types of
CE shell commands due to running with user privileges, use the Platfrom
Builder Target->Target Control shell with a KITL link to execute
commands that do not work using do.exe.

- Many BSP settings cannot be configured using the Platform Builder
Catalog View. Edit the ti_evm_3530.bat file instead.

- The USB OTG port is sensitive to the type/length of USB cable used.
The cause of this issue is not known. Recommended cable is GoldX USB 5
in 1 (GXQU-06) available from www.amazon.com.

- The MDC/EVM2 camera (video capture) driver requires a large amount of
free program memory (~30 MB) when used with the video capture demo
program. Use the control panel, system icon, memory tab to adjust the
memory allocation. After running a video capture demo program, a large
memory leak was observed. It is not known if this is due to a problem
with the demo program or with the camera driver.

- Suspend/resume clock management for the HSUSB host EHCI driver is not
working. When this driver is included in the system, the suspend current
will be significantly higher.

- Suspend/resume for the unsupported OHCI driver is incomplete. If this
driver is used it may not work after suspend/resume.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or EVM2, plugging a low/full speed device into the USB
port on the daughter card will cause the port to become non-functional
until a reboot. See Known Hardware Issues for more information. To work
around this issue, use a hub between the USB port and the low/full speed
device. This behavior is by design (OMAP3xxx chip), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead. No fix is expected.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

- EVM1 only. The RTC emulation code does not support alarms.

<12/08/2009> 6.12.04

BSP Changes:

- Updated PowerVR to fix issues found in 6.12.03 release.

- Minor change to display driver to make debug zones compatible with
PowerVR driver.

- Changed EVM_3530_MDC sample OS design to include PowerVR driver by
default. Note that if this sample OS design is used, PowerVR from the
BSP release must be installed in the PUBLIC tree in order to build
successfully.

- Changed EVM_3530_MDC sample OS design to include Silverlight, Flash
Lite and XP Skin by default. Note that this sample OS design can only be
used with CE6R3.

- All the sample images included with the BSP have been compiled using
CE6R3.

- Added sample images for EVM2. Note that EMV2 sample images include
PowerVR support, Silverlight and the XAMLPERF application.

- Fixed problem with camera_mdc driver that caused debug builds to fail
if retail build was not done first.

- Updated EVM_3530_MDC sample OS design to add UART1, UART3 and USB OTG
BSP feature variables. Note that UART3 (COM3) is still disabled by
default in the ti_evm_3530.bat file.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- Many BSP settings cannot be configured using the Platform Builder
Catalog View. Edit the ti_evm_3530.bat file instead.

- When PowerVR is included, there are issues unless the XP Skin OS
component is selected.

- The USB OTG port is sensitive to the type/length of USB cable used.
The cause of this issue is not known. Recommended cable is GoldX USB 5
in 1 (GXQU-06) available from www.amazon.com.

- EVM2 only. After using the PWR_ON button to suspend, the system
resumes immediately. It is not yet known if the cause is hardware or
software.

- The MDC/EVM2 camera (video capture) driver has only limited power
management and may not work after suspend/resume. It also requires a
large amount of free program memory (~30 MB) when used with the video
capture demo program. Use the control panel, system icon, memory tab to
adjust the memory allocation. After running a video capture demo
program, a large memory leak was observed. It is not known if this is
due to a problem with the demo program or with the camera driver.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or the EVM2, the HS USB port 2 (EHCI) will not work after
suspend/resume.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or EVM2, plugging a low/full speed device into the USB
port on the daughter card will cause the port to become non-functional
until a reboot. See Known Hardware Issues for more information. To work
around this issue, use a hub between the USB port and the low/full speed
device. This behavior is by design (OMAP3 hardware), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port is limited to low and full speed transfers because
device enumeration fails using high speed transfers. The cause of this
problem is not known and may be due to a hardware issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead. No fix is expected.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

- EVM1 only. The RTC emulation code does not support alarms.

<10/21/2009> 6.12.03

BSP Changes:

- Added sample batch files for EVM1 (evm1_ti_evm_3530.bat), EVM1 with
Multimedia Daughter Card, (evm1_mdc_ti_evm_3530.bat) and EVM2
(evm2_ti_evm_3530.bat). See the comments near the top of these files for
usage.

- Added sample application that can be used to demo the video capture
capability of the EVM1+MDC or EVM2. Requires use of OS configuration
similar to the EVM_3530_MDC sample OS design.

- Added registry entry to enable/disable display driver code that
supports waiting for vertical blanking, disabled by default.

- Removed BSP_SPEED_CPUMHZ variable and added BSP_OPP_SELECT to select
initial VDD1 voltage, CPU speed and IVA speed. See TI_EVM_3530.bat for
supported operating points.

- Fixed problem with CPU and TPS659XX voltage processor init sequence in
bootloader. Unset BSP_NOSMARTREFLEXPOLICY to work around issue causing
incorrect VDD1/VDD2 voltages.

- Changed references to OMAP3430 to OMAP35XX.

- Changed references to TWL4030 to TPS659XX.

- Fixed problem with wave driver failing to stop audio streams during
suspend.

- Removed redundant save/restore of VFP/NEON registers on thread switch.
The CE6 kernel does save/restore of ARM VFPv2 VFP/NEON registers on
thread switch. The OAL now only saves/restores the new ARM VFPv3
VFP/NEON registers.

- New BSP installation instructions.

- Added wave driver registry entry to specify default audio route.

- Remove headset driver from build, driver is still available for use as
example code.

- Increased gain on stereo headset output path.

- Changed hard tabs to spaces in all source code.

- Changed display driver cursor registry entry to enable cursor whenever
sysgen_cursor is included in the OS configuration.

- Disabled CLOCKACTIVITY bit in USBTLL SYSCONFIG register for ECHI
driver.

- Fixed problem with AUX input / stereo headset output routing.

- Documented meaning of wave driver gain registry settings in
platform.reg. Added input gain control for AUX input. Note that the EVM
and EVM2 audio in connector is stereo line input, not a microphone
input.

- Fixed problem with DVI signal timing in some DVI modes.

- Inverted backlight on/off control for EVM2 in backlight driver and
bootloader.

- Changed touch panel driver to extend settling time prior to sampling
to improve performance when touch signal filter capacitors are
installed (EVM2).

- Enabled TPS659XX RTC code for EVM2 only (TPS659XX MSECURE pin no longer
floating, now connected to GPMC_WAIT2/GPIO_64, internal RTC can now be
used). 

- Added registry entry to control high speed mode enable/disable for
musb OTG stack.

- New bootloader logo file.

- Changed SDHC driver to use registry entries to configure 4 bit SD
memory, 4 bit SDIO and low voltage support for slot 1.

- Enabled display driver sDMA BLTs when cursor is enabled.

- Added environment variable to enable PAL TVOUT format. Note that this
option is untested and unsupported.

- Added environment variable to control the IVA (DSP) clock.

- The TPS659XX VDD1 regulator voltage has been changed to 1.35V.

- Added environment variable to select between EVM1 and EVM2.

- Added environment variables to select EVM2 features. See
TI_EVM_3530.bat for details and restrictions.

- Added support for video capture subsystem on EVM2.

- Added support for HSUSB (High Speed USB) host (EHCI) port on EVM2.

- Added support for external VBUS power control on HSUSB (EHCI) port on
EVM2.

- Added support for new ethernet controller (SMSC9220) and new reset
signal on EVM2.

- Fixed bug in display driver DSS clock management that was causing
problems with DVI frame rates.

- Fixed problem with bootloader CPU version messages.

- Changed display driver surface allocation to fail PREMULT surface
allocation in video/overlay memory.

- Added support for UART3 RTS/CTS handshake on EVM2 using registry
entries to enable this feature.

- Added support for RTC in TPS659XX chip for EVM2.

- Added support for OTG port external power switch for EVM2. 

- Added disabled new audio mute signal during startup.

- Improved display driver interrupt subsystem.

- Modified BSP directory structure to move platform dependent code to
src\drivers tree.

- Modified platform independent code to compile separately from platform
dependent code.

- Modified BSP directory stucture to move platform independent
code to platform\common\src\soc\omap tree.

- Fixed some PREFAST code analysis warnings.

BSP Notes:

- The BSP contains several drivers that are provided only as examples.
These drivers have not been tested on the EVM and are not supported:

TI_EVM_3530\SRC\DRIVERS\USBHCD
TI_EVM_3530\SRC\DRIVERS\MUSB\USBOTG_TPS659XX
TI_EVM_3530\SRC\DRIVERS\CAMERA
TI_EVM_3530\SRC\DRIVERS\HEADSET
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap\TPS659XX\BCI
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap\TPS659XX\MADC
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap35xx\HDQ

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- EVM2 only. The EVMFlashTool does not work with some EVM2 samples, the
cause is not known. As a work around use of the SD boot is suggested.

- The USB OTG port is sensitive to the type/length of USB cable used.
The cause of this issue is not known. Recommended cable is GoldX USB 5
in 1 (GXQU-06) available from www.amazon.com.

- EVM2 only. After using the PWR_ON button to suspend, the system
resumes immediately. It is not yet known if the cause is hardware or
software.

- The MDC/EVM2 camera (video capture) driver has only limited power
management and may not work after suspend/resume. It also requires a
large amount of free program memory (~30 MB) when used with the video
capture demo program. Use the control panel, system icon, memory tab to
adjust the memory allocation. After running a video capture demo
program, a large memory leak was observed. It is not known if this is
due to a problem with the demo program or with the camera driver.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or the EVM2, the HS USB port 2 (EHCI) will not work after
suspend/resume.

- When using the USB ECHI controller with the EVM1 + TI Multimedia
Daughter Card or EVM2, plugging a low/full speed device into the USB
port on the daughter card will cause the port to become non-functional
until a reboot. See Known Hardware Issues for more information. To work
around this issue, use a hub between the USB port and the low/full speed
device. This behavior is by design (OMAP3 hardware), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port is limited to low and full speed transfers because
device enumeration fails using high speed transfers. The cause of this
problem is not known and may be due to a hardware issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead. No fix is expected.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

- EVM1 only. The RTC emulation code does not support alarms.

<6/20/2009> 6.11.00

BSP Changes:

- Added preview version of camera driver to support video capture using
the TI Multimedia Daughter Card. See the BSP_EXPANSION_TI_MULTIMEDIA
section in TI_EVM_3530.bat. There are some issues with this driver, see
BSP Know Issues for details.

- New version of DSP CODEC subsystem components.

- Increased priority of musbotg driver threads.

- Changed UART driver to stop advertising support for 75, 110 and 150
baud (out of range for baud rate divider).

- Power management changes.

- Display driver now has support for sDMA and NEON BLTs enabled.

- Display driver now supports LPR (Low Power Refresh) depending on power
management subsystem configuration. Note that this feature is untested.

- Fixed some display driver problems that were causing CETK GDI test
failures. 

- Added sample camera driver for customers that use camera input instead
of video capture. Note that this driver is not built by default and is
untested and unsupported.

- Fixed problem in UART driver power state transition code.

- Increased default touch driver sample rate to 200 samples per second.

- Some changes to target control shell extension commands.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- The Preview version of the MDC camera (video capture) driver has only
limited power management and may not work after suspend/resume. It also
requires a large amount of free program memory (~30 MB). Use the control
panel, system icon, memory tab to adjust the memory allocation. After
running a video capture demo program, a large memory leak was observed.
It is not known if this is due to a problem with the demo program or
with the camera driver.

- When using the USB ECHI controller with the TI Multimedia Daughter
Card, the MDC USB port will not work after suspend/resume. This problem
is expected to be fixed in a future release.

- When using the USB ECHI controller with the TI Multimedia Daughter
Card, plugging a low/full speed device into the USB port on the daughter
card will cause the port to become non-functional until a reboot. See
Known Hardware Issues for more information. To work around this issue,
use a hub between the USB port and the low/full speed device. This
behavior is by design (hardware), no fix is expected.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port is limited to low and full speed transfers because
device enumeration fails using high speed transfers. The cause of this
problem is not known and may be due to a hardware issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead. No fix is expected.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

- The RTC emulation code does not support alarms.

<5/28/2009> 6.10.00

BSP Changes:

- The DSP based CODEC subsystem has been improved and DMAI sample
applications are now available.

Codecs on the DSP (i.e. the ones in the DSP server/combo) can be
exercised in one of two ways: either through the DirectShow framework in
Windows Media Player, or by running independent DMAI (Davinci Multimedia
Application Interface) sample applications provided in this release. Not
all codecs are supported by DirectShow filters currently. Here’s a table
that summarizes current support, depending on the codec of interest:

DSP CODEC      DirectShow filter for WMP?    DMAI sample application?
 
MPEG2 decoder  Yes                           Yes
MPEG4 decoder  Yes                           Yes
H.264 decoder  Yes                           Yes
AAC decoder    No                            Yes
JPEG decoder   No                            Yes
MPEG4 encoder  No                            Yes
H.264 encoder  No                            Yes
JPEG encoder   No                            Yes

- The DSP based MP3 decoder has been removed.

- OAL now enables NEON/VFP-Lite coprocessor and performs NEON register
save/restore on thread context switch.

- OAL CPU cache configuration setup changes.

- OAL interrupt support now manages wake from some interrupts.

- McSPI bus driver API now supports 3 SPI bus controllers, improved
slave mode support.

- The display driver makes some changes to display frame buffer cache
management and video/graphics pipeline management.

- Display driver is configurable to support for TV out using composite
video (P6 RCA jack) is improved. When using composite output the CPU
module SW1 position 6 must be OFF and BSP_TVOUT_COMPOSITE must be set in
TI_EVM_3530.bat.

- Display driver support for S-VIDEO out using P14 is improved. When
using S-VIDEO output the CPU module SW1 position 6 must be ON and
BSP_TVOUT_COMPOSITE must be not be set in TI_EVM_3530.bat.

- GPIO driver has improved support for GPIO pin configuration.

- Power management has changes including interrupt latency constraint
management and CPU load policy management.

- McBSP driver has many changes including enhanced registry based
configuration.

- Wave driver has many changes related to audio path management.

- Wave driver now has support for setting some audio path gains using
registry entries.

- SDHC driver SDIO interrupt pin management has been improved.

- MUSBFN driver has bug fixes and DMA mode 0 changes.

- UART driver has improvements, including DMA support.

- The OAL RTC emulation code has been improved. Note that RTC emulation
is needed because the TPS659XX MSEC pin is not connected on the EVM
design, causing the TPS659XX RTC to be unusable.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- When using the USB ECHI controller with the TI Multimedia Daughter
Card, suspend/resume does not work. This problem is expected to be fixed
in the next release.

- The UART driver fails the CETK serial driver power management tests.
This problem is expected to be fixed in the next release.

- The display driver fails some CETK stretchblt and surface creation
tests. This problem is expected to be fixed in the next release.

- When using the USB ECHI controller with the TI Multimedia Daughter
Card, plugging a low/full speed device into the USB port on the daughter
card will cause the port to become non-functional until a reboot. See
Known Hardware Issues for more information. To work around this issue,
use a hub between the USB port and the low/full speed device. This
behavior is by design (hardware), no fix is expected.

- The UART driver will sometimes throw an exception during debug build
bootup. Note that this issue has not been observed recently and may have
been fixed.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port is limited to low and full speed transfers because
device enumeration fails using high speed transfers. The cause of this
problem is not known and may be due to a hardware issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- DSP based MP3 decoder causes system to lock up and should not be used.

- Some CETK ethernet drivers tests lock up.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead.

- The display occasionally fails to come up at boot on some units. Note
that this has not been seen recently and may no longer be an issue.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- Occasional I2C bus lock ups that cause general system instability. The
cause is under investigation. This problem has only been seen when
booting a debug build of NK.bin. Note that this has not been seen
recently and may no longer be an issue.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

- The RTC emulation code does not support alarms.

BSP Notes:

A beta version of the ARM’s Vector Floating Point (VFP) library is
available for customers wanting to enable VFP functions.  Bsquare will
be integrating the final release of this library in a future release of
our BSP.  This will happen once ARM release final version and has been
fully tested and qualified with our BSP for OMAP35x platform. This
library is available at ARM website at:

    http://www.arm.com/products/os/windowsce.html.

The use of this library is subject to customers responsibility and is
not under warranty by Bsquare.

<4/20/2009> 6.09.02

BSP Changes:

- Added support for configuration of the HS USB ports 1 & 2 (MM1 & MM2).
See TI_EVM_3530.bat and FILES\platform.reg for details. Ports using
low/full speed 2/3/4/6 pin interface are configured by the OHCI driver,
ports using 12 pin interface are configured by the EHCI driver. Note
that use of the HS USB ports requires an expansion daughter card.

- Added enironment variables to configure the BSP for use with the
BSQUARE Magnum expansion card and the TI Multimedia Daughter Card. Note
that support for the TI Multimedia card is limited to the USB host
connector which can support only high speed devices due to a hardware
limitation of the OMAP3530 chip. See TI_EVM_3530.bat for details.

- Improved VESA compatibility of 1280x720 60Hz DVI mode.

- Removed obsolete BSP_DVI_ENABLE environment variable.

- Improved behavior of OTG port, hubs work the first time the cable is
inserted. Support for high speed devices is still disabled. OTG Host
Negotiation Protocol (HNP) and Session Request Protocal (SRP) is
untested.

BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- When using the USB ECHI controller with the TI Multimedia Daughter
Card, plugging a low/full speed device into the USB port on the daughter
card will cause the port to become non-functional until a reboot. See
Known Hardware Issues for more information. To work around this issue,
use a hub between the USB port and the low/full speed device.

- The UART driver will sometimes throw an exception during debug build
bootup.

- If a device is connected to the OTG port during boot it will sometimes
fail to enumerate. Remove and re-insert the cable connected to the OTG
port to work around this issue.

- The USB OTG port is limited to low and full speed transfers because
device enumeration fails using high speed transfers. The cause of this
problem is not known and may be due to a hardware issue.

- The USB OTG port will sometimes fail to enumerate devices and with
spurious errors on pipes. It is not known if the cause is due to
hardware or software.

- DSP based MP3 decoder causes system to lock up and should not be used.

- Some CETK ethernet drivers tests lock up.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead.

- The display occasionally fails to come up at boot on some units. Note
that this has not been seen recently and may no longer be an issue.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- Occasional I2C bus lock ups that cause general system instability. The
cause is under investigation. This problem has only been seen when
booting a debug build of NK.bin. Note that this has not been seen
recently and may no longer be an issue.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

BSP Notes:

- The BSP contains several drivers that are provided only as examples.
These drivers have not been tested on the EVM and are not supported:

TI_EVM_3530\SRC\DRIVERS\USBHCD
TI_EVM_3530\SRC\DRIVERS\MUSB\USBOTG_TPS659XX
TI_EVM_3530\SRC\DRIVERS\CAMERA
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap\TPS659XX\BCI
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap\TPS659XX\MADC
PLATFORM\COMMON\SRC\SOC\OMAP35XX_TPS659XX_TI_V1\omap35xx\HDQ

<4/03/2009> 6.09.01

BSP Changes:

- Fixed problem with PowerVR binaries being out of sync with the BSP.
When selecting the PowerVR components from the catalog, be sure to
select the "PowerVR", "PowerVR/3D API/OpenGLES 1.0", "PowerVR/3D
API/OpenGLES 2.0" and "PowerVR/3D API/OpenVG 1.0.1" checkboxes under
"Third Party".

<2/06/2009> 6.09.00

BSP Changes:

- Support for the OMAP3530 ES3.1 silicon using the JW256 Package On
Package with 2 x 128 MB DRAMs. See the BSP_SDRAM_BANK1_ENABLE variable
in TI_EVM_3530.bat for details.

- The drivers for the musb subsystem have been changed to disable high
speed support. This was done as a partial work around for problems with
USB 2.0 hub support.

- An OpenGL subsystem (PowerVR) is available as a separate install not
included in the BSP.

- Fixed syntax errors in src\inc\tps659xx_musb.hpp file. Note that the
TPS659XX OTG transceiver driver is provided as an example only and is not
supported.

- Fixed a problem with the BSP_DVI_800W_600H_56HZ horizontal sync
timing.

- Changed display driver registry entries to enable cursor support when
any of the BSP_DVI_... environment variables is set.

- Added command line utility to allow OMAP shell extensions to be
executed without a KITL connection. For details see TI_EVM_3530.bat,
look for BSP_SHELL_EXTENSIONS_DO_CMD.

- Improved behavior of sample OHCI driver (USBHCD) during
suspend/resume. Note that the USBHCD driver is provided as an example
only and is not supported.

- Removed DSP based MP3 decoder support.

- Improved behavior of UART when break is received when TX DMA is enabled.


BSP Known Issues:

Also see the "Known Hardware Issues" section later in this file.

- The BSP has not be tested with the ES3.1 OMAP3530 silicon.

- The UART driver will sometimes throw an exception during debug build
bootup.

- The OTG driver ULPI register access will sometimes fail during bootup
if a device is connected to the OTG port during boot.

- The musb subsystem will fail the first time a hub is connected. Remove
and re-insert the cable connected to the OTG port to work around this
issue.

- DSP based MP3 decoder causes system to lock up and should not be used.

- The USB OTG port, used in host mode (mini-A to A cable adapter) has
problems enumerating some USB 2.0 hubs. The cause is not known. Support
for high speed operations has been disabled until the problem is fixed.

- When the OTG port is operating in host mode (a USB device or hub is
plugged in), the device will sometimes stop operating. Unplugging and
reinserting the mini-A cable may make the device operate again.

- The USB OTG port ID pin sensing is currently polled. When a USB device
is unplugged from the OTG port, some time may be required (about 1
second) before the same or another device can be plugged in. It is
possible that this issue will be fixed in a future code drop (provided
that a method for generating an ID pin grounded event is found).

- Some CETK ethernet drivers tests lock up.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead.

- The display occasionally fails to come up at boot on some units. Note
that this has not been seen recently and may no longer be an issue.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- Occasional I2C bus lock ups that cause general system instability. The
cause is under investigation. This problem has only been seen when
booting a debug build of NK.bin. Note that this has not been seen
recently and may no longer be an issue.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default. A partial work around has been implemented, but is
not yet validated.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either disable the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation should only be used with the Virtual Rotated Frame
Buffer (VRFB). Rotation using the flat frame buffer is possible, but 
the OMAP3530 memory/bus bandwidth is not sufficient to allow display
scanning without causing display to flicker and/or lock up.

BSP Notes:

- The BSP contains several drivers that are provided only as examples.
These drivers have not been tested on the EVM and are not supported:

DRIVERS\USBHCD
DRIVERS\MUSB\USBOTG_TPS659XX
CSP\OMAP\CAMERA
CSP\OMAP\TPS659XX\BCI
CSP\OMAP\TPS659XX\MADC
CSP\OMAP3430\HDQ

- Simple benchmark results in millions per second:

Cached instruction execution rate:             612
Non-cached non-buffered RAM DWORD read rate:     5.02
Non-cached non-buffered RAM DWORD write rate:    6.27
Non-cached non-buffered RAM 4 DWORD read rate:  10.03
Non-cached non-buffered RAM 4 DWORD write rate: 12.53


<12/30/2008> 6.08.00

BSP Changes:

- Added CodecEngine/DSPLINK Binary, CODEC and DShow Filter support for
MPEG2 Video Decoder.

- Added ISP1504 OTG driver for use by musb block (OTG port on EVM main
PCB). This driver and related changes improve the behavior of the OTG
port when used in host mode. USB Devices that draw 50 mA or less may
work without a powered hub. There are still some problems, see the BSP
known issues section for details.

- Changed DVI display mode timings to better conform with VESA standard.
See src\drivers\lcd\vga\lcd_vga.c for more information about pixel clock
and sync timing settings. Note that some modes have non-VESA sync
timing due to display controller register value range limitations. See
SRC\INC\drivers\lcd\vga\lcd_vga.c for details.

- Changed display driver direct draw capabilities to make video overlay
downscaling support dependent on the pixel clock divider. The overlay
pipeline hardware downscaling requires that the pixel clock be at most
1/4 the functional clock rate. For details, see the OMAP3530 TRM.

- Added 2 new VESA compatible DVI modes that have a pixel clock divider
of 4 allowing 50% video overlay downscaling - BSP_DVI_800W_600H_56HZ and
BSP_DVI_640W_480H_72HZ.

- Video overlay downscaling to 50% is now supported for
BSP_DVI_640W_480H_72HZ, BSP_DVI_800W_600H_56HZ and the default on-board
LCD 480W by 640H resolutions.

- Added range checks for display controller sync timing register values.

- Added workaround for crash during OAL PRCM subsystem initialization
caused by enabling OAL_FUNC zone for OALMSG.

- Added code to prevent use of critical sections and OALMSGs in OAL PRCM
subsystem when called while kernel is single threaded (OEMPowerOff and
OEMIdle).

BSP Known Issues:

- The USB OTG port, used in host mode (mini-A to A cable adapter) has
problems recognizing a few hubs. The cause is not known.

- When the OTG port is operating in host mode (a USB device or hub is
plugged in), the device will sometimes stop operating. Unplugging and
reinserting the mini-A cable may make the device operate again.

- The USB OTG port ID pin sensing is currently polled. When a USB device
is unplugged from the OTG port, some time may be required (about 1
second) before the same or another device can be plugged in. It is
possible that this issue will be fixed in a future code drop (provided
that a method for generating an ID pin grounded event is found).

- Some CETK ethernet drivers                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
- Warm reset using switch and IOCTL_HAL_REBOOT function cause a cold
boot.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft. The default build
settings use the normal flat frame buffer instead.

- The display occasionally fails to come up at boot on some units. Note
that this has not been seen recently and may no longer be an issue.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- Occasional I2C bus lock ups that cause general system instability. The
cause is under investigation. This problem has only been seen when
booting a debug build of NK.bin.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue. The UART TX DMA is
disabled by default.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either remove the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

- When playing videos and playback window is moved to be partially off
screen, the video playback may show artifacts.

- Display rotation is only supported when using the Virtual Rotated
Frame Buffer (VRFB).

BSP Notes:

Simple benchmark results in millions per second:

Cached instruction execution rate:             612
Non-cached non-buffered RAM DWORD read rate:     5.02
Non-cached non-buffered RAM DWORD write rate:    6.27
Non-cached non-buffered RAM 4 DWORD read rate:  10.03
Non-cached non-buffered RAM 4 DWORD write rate: 12.53

------------------------------------------------------------------------------

<11/26/2008> 6.07.00

BSP Changes:

- Changed 720P DVI (1280x720) to be 60Hz, 72 MHz pixel clock.

- Changed DEFAULT_PIXELTYPE to DISPC_PIXELFORMAT_RGB16 for all display
formats. Use of DISPC_PIXELFORMAT_RGB32 for DEFAULT_PIXELTYPE results in
a dramatic drop in performance in the GDI BLTs, cause is not known.

- Disabled TX DMA for all UART channels. This works around a bug that
caused the TX DMA to stop after a line break condition is received.

- Added support for software based mouse cursor to display driver,
disabled by default, enabled using display driver registry entry.

- Increased touch panel sample rate to 125 points per second.

- Added error message requesting that all QFEs be installed if
DDSCAPS_OWNDC flag is not defined.

- Modified display driver to fix problem with menus appearing behind
video overlay.

- Fixed typo in PRCM clock management code that was causing DPLL5
frequency updates to update DPLL4 instead.

- Added missing critical sections for some DMA driver functions.

- Added CESYSGEN IF CE_MODULES_USBHOST around EHCI usb dirs file entry.

- Fixed a bug in wave driver that cause stereo sounds to play back in
only one channel.

- Changed config.bib FSRAMPERCENT to allow large debug images to boot
without out of memory issues.

- Prebuilt images for ONENAND support are no longer included on the BSP
release CD. Images with ONENAND support can still be created using the
BSP, see the ONENAND environment variable in the TI_EVM_3530.bat file.

BSP Known Issues:

- Some CETK ethernet drivers tests lock up.

- Warm reset using switch is not reliable.

- The IOCTL_HAL_REBOOT function is not reliable.

- The USB OTG port has problems recognizing USB devices and switching to
host mode. The cause is probably due to an incorrect OTG driver, the
plan is to fix this in a future release.

- The Virtual Rotated Frame Buffer (VRFB) module is not compatible with
the some of the video codecs provided by Microsoft.  The default build
settings use the normal flat frame buffer instead.

- The display occasionally fails to come up at boot on some units. Note
that this has not been seen recently and may no longer be an issue.

- When playing media files with the CODEC engine based H.264
decompression on a display configured to use the VRFB and a rotated
display, the display will sometimes malfunction (go white).  A reboot of
the device is required to recover.

- Occasional I2C bus lock ups that cause general system instability. The
cause is under investigation. This problem has only been seen when
booting a debug build of NK.bin.

- When UART driver TX DMA is enabled (registry TxDmaRequest entry) the
UART stops sending after a line break is received. It is not known if
the cause is due to a hardware or software issue.

- Video playback may cause display flicker when using large format DVI
display output (800x600 and up). The cause is not known, but suspected
to be due to exceeding the available bus bandwidth.

- Problems have been reported when playing WMV videos with a size
greater than 320x240.

- When the BSP_DSPLINK environment variable is set, a large amount of
RAM is allocated to the DSP subsystem and debug images can become too
large to boot. To work around this problem either remove the CODEC
Engine support components when doing a debug build or remove components
from the OS configuration that are not needed for the debug session.

------------------------------------------------------------------------------

<10/14/08   6.06.01>

- Removed flash demo from default image.

- Added  environment variable to control inclusion of flash player.

- Added environment variable to include flash player demo and link on
desktop to start the demo.

- Added CodecEngine/DSPLINK Binaries, Codecs and DShow Filter support
for MP3, H264 Video Decoder, MPEG4 Video Decoder.

- Added environment variables to enable CODEC engine support components.
See TI_EVM_3530.bat for details.

------------------------------------------------------------------------------

<10/14/08   6.06.0x>

- Improved DVI support, added environment variables to enable DVI only
display output at 640x480, 800x480, 800x600, 1024x768 and 1280x720. See
TI_EVM_3530.bat for more information. Removed obsolete DVI registry entry.

- Added null battery driver, reports default values for system with no battery.

- Added option to enable BQ27000 battery driver.

- Added example registry entry for touch panel SampleRate.

- Changed boot menu to clarify items related to ethernet MAC address.

- Added driver for TPS659XX PWRON key.

- Changed RTC emulation code to use 32KHz sync counter instead of
GPTIMER1. Changed suspend/resume code to stop GPTIMER1 before suspend,
enable during resume. This addresses problems caused by tick timer
running during suspend.

- Fixed problem with USB clock source control in OAL PRCM module.

- Added binary NDIS miniport driver for SMSC9115 ethernet chip.

- Changed serial driver to remove RTS/CTS capability flag.

- Added environment variable to include flash player, flash demo and
link on desktop to start the demo.

------------------------------------------------------------------------------

<9/10/08    6.05.00>

- Added BSP_DVI variable to cause display driver to build with the
default display output directed to DVI port instead of LCD panel. Note
that a display registry entry is also available to enable/disable DVI
output. Note that the DVI display size is the the same as the LCD size
(480 wide by 640 high) and will appear horizontally stretched on the DVI
monitor.

- Fixed problem with keypad (S4-S18) driver.

- Modified wave driver to support AuxHeadset mode (AUX in, headset out).
Changed default mode (set in headset driver) to AuxHeadset mode.

- Added "dvi" command to the OMAP CE target shell extensions to allow
switching display output between the LCD and DVI. Note that this option
is only usable if LCD and DVI resolution is the same (480W x 640H).

- Fixed exception in default bus driver during resume.

- Note that the HDQ and CAMERA drivers are included as examples, BSP but
are not supported on the EVM3530.

------------------------------------------------------------------------------

<8/25/08    6.04.00>

- Updated flash driver to support MT29F2G16 NAND contained in the Micron
POP package.

- Updated XLDR and EBOOT to support boot from MT29F2G16 contained in the
Micron POP package.

- Changed icon for BSP information control panel applet.

- Added TI specific jpg file as default desktop wallpaper.

- Changed default CPU core clock speed to 600 MHz.

- Changed default FMD block driver to NAND (was ONENAND).

- Improved bootloader default boot device selection based on boot select
switches.

------------------------------------------------------------------------------

<8/1/08     6.03.00>

- Added PMExt driver to image, added custom PM.DLL to load and
initialize PMExt driver.

- Improved TPS659XX audio path control to reduce pop noises on systems
supporting external mute. Note that the EVM3530 does not support
external mute.

- Changed EBOOT default boot behavior to be select boot device based on
boot switch settings.

- Improved SD boot documentation.

------------------------------------------------------------------------------

<7/14/08    6.02.01>
- Fixed syntax error building debug version of SD bootloader.

------------------------------------------------------------------------------

<7/03/08    6.02.00>
- Enable warm reset logic
- Added control panel application to display BSP version information
- Add support for booting from MMC/SD memory card

------------------------------------------------------------------------------

<6/06/08        6.01.00>
- Added custom SD memory card driver (workaround
 for multi-block read errata)
- Fixed USBOTG cable attach failure
- Added optional hardware watchdog support

------------------------------------------------------------------------------

<5/2/08     6.00.00>
- Initial Release

***********************************
Known Hardware Issues:
***********************************

See the TI Silicon Errata document for the latest list of known CPU
issues.

- EVM1 boards that use ES2.x silicon cannot use the VRFB due to a
silicon issue. No fix is expected.

- The EVM2 and MDC ECHI port uses a transceiver that has problems with
the EHCI  controller PHY interface, it will fail after USB bus
suspend/resume. New designs should choose an EHCI transceiver that is
compatible with the EHCI PHY interface.

- (EVM2 REVG only) OTG Port, VBUS Fault Detection Feature:
	
The VBUS Fault detection input of ISP1507 (U36) on the EVM Main board
can be connected to two sources with resistor installation options.

1. Resistor R830 connects the Fault detection input to the Over Current
detection output of an OTG VBUS switch.

2. Resistor R845 connects the Fault detection input to ground to disable
the ISP1507 Fault feature when not used (Same as REV-D & E Main board).

Currently, the EVM main board has both optional resistors installed on
the Main board which disables the Fault detection input feature of the
ISP1507, and disables the fault input to U152.
	
The Fault detection feature does not have a functional impact on the OTG
port if the USE_EXT_VBUS_IND bit in the ISP1507 is set low. This feature
is currently disabled on the main board, like previous EVM platforms.
There is a board level work around to enable this feature. 
	
- (EVM2 REVG only) OTG Port, VBUS Decoupling Capacitor option
	
The current design allows the VBUS power source on the USB OTG port to
be switched between standard host (500mA) and OTG mode. The default is
standard host mode which is capable of sourcing 500mA of current. A
100uF (C552) decoupling capacitor is connected to VBUS.  The USB
specification requires a 120uF or greater capacitor connected to VBUS
when operating in standard host mode and the USB OTG specification
requires the VBUS capacitance to be in the range of 1 - 6.5uF for OTG
mode. The board should have been designed with a 4.7uF capacitor
connected to VBUS with a jumper option that allows an additional 120uF
to be connected to VBUS when operating in standard host mode.
	
It may be necessary to replace the 100uF capacitor installed in location
C552 with a 120uF capacitor if the voltage on VBUS drops below the
specified level when a USB device is attached when operating in standard
host mode. It may be necessary to replace the 100uF capacitor installed
in location C552 with a 4.7uF capacitor if operating in OTG mode. 

- (EVM2 REVG only) HSUSB HOST INTERFACE ID-Pin for A-devices
	
In the current design ID pin of USB3320C-EZK is grounded through 10K,
but for A-devices, ID pin has to grounded through 0ohm Resistor.
	
For supporting A-Devices, install 0ohm at resistor R639 instead of 10K
ohm.
	
- (EVM power module REVD only) LOW POWER MODE of EVM
	
In normal mode, SYS_CLKREQ is high. In low power mode, SYS_CLKREQ goes
to high impedance. But In the current design SYS_CLKREQ is pulled high
and this keeps it high, and so the PMIC never shuts down voltages in OFF
mode.
	
If Low power mode needs to be supported on this board, the resistor at
R24 could be removed. 

- The OCHI and EHCI controllers cannot be used at the same time due to
silicon issues.

- There are limitations to the OMAP3 display controller's abilities.
Example configurations that may cause the display subsystem to
malfunction include:

Use of flat frame buffer display rotation.
Use of VRFB display rotation with large resolution displays.
Use of video pipelines with display rotation.
Use of video pipelines with large resolution displays.
Use of video scaling with large resolution displays. 

Everthing that the display subsystem does requires some hardware
resources, such as bandwidth from the internal bus and DDR SDRAM memory
subsystem.
 
- Larger displays require more resources than smaller displays.

- Using a video pipeline requires more resources than when not using a
video pipelines.

- Video scaling (down scaling in particular) requires more resources
then videos that do not require scaling.
 
Eventually, the increased demand for resources from display subsystem
overwhelms the hardware's capabilities.
 
This will occur when using a large (720p) display and doing video
playback that requires resizing the video.
 
It is not possible to predict when this type of problem will occur,
customers will need to test their own use cases to see if they will
work.

- The power supply (on the TPS659XX) used for the MMC/SD/SDIO slot can
only supply about 220 mA. Cards that require more power (such as many
WiFi cards) will not operate properly.

- On some EVM2 samples, the EEPROM (Microchip 93AA46B-I/SN) connected to
the EVM2 SMSC9220 Ethernet controller is the wrong part. The SMSC9220
requires an EEPROM with 128x8 organization, but this EEPROM is 64x16
organization and cannot be use to store the MAC address. The correct
part would be Microchip 93AA46A-I/SN.

- When using the USB ECHI controller, for instance with the TI
Multimedia Daughter Card or the EVM2 HSUSB host port, plugging a
low/full speed device into the USB port on the daughter card will cause
the port to become non-functional and a reboot will be required to
recover. This problem is due to the fact that the EHCI controller hands
off low/full speed devices to the OHCI controller, but the OHCI
controller cannot talk to the USB port through a high speed (12 pin) PHY
due to a design issue with the OMAP3530. To work around this issue, use
a hub between the USB port and the low/full speed device. See the
OMAP3530 documentation for details.

- TPS659XX MSEC pin is floating, keeping chip in secure mode. The TPS659XX
Real Time Clock (RTC) cannot be enabled in secure mode. This issue has
been fixed on the EVM2.

- Because the TPS659XX RTC cannot be enabled, RTC is emulated using CPU
counter. This issue has been fixed on the EVM2.

- Because the TPS659XX RTC cannot be enabled, the RTC alarm cannot wake
system. This causes some CETK notification tests to fail. This issue has
been fixed on the EVM2.

- Because the TPS659XX RTC cannot be enabled, the RTC is emulated using
32 bit counter running at 32768 Hz, counter rolls over every 36.4 hours.
This issue has been fixed on the EVM2.

- The audio input jack is a stereo line level input, not a microphone
input.

- Some devices (EVM1: Ethernet, DVI, MMC LEDs, ETM interface, MUSB OTG;
EVM2: Ethernet, DVI, MMC LEDs, ETM interface, JTAG, MUSB OTG, video
decoder, camera) are not powered when the battery power or charger power
inputs are used.

- The EVM DB9 UART1/2 and UART3 serial port hardware design only support
RXD and TXD, there are no modem handshake/status signals available on
the ports. The EVM2 has support for RTS/CTS handshake for UART3.

- The EVM USB OTG port implementation is missing an external VBUS power
switch needed to use the OTG port as a USB Host for devices drawing more
than 100mA from the USB OTG port. To work around this issue, an external
powered USB hub must be used. This issue has been addressed on EVM2.

- The LED connected to GPIO8 (used as the Notification LED) is very dim
due to a hardware issue. This issue has been fixed on the EVM2. 

- The external transistor required by the TPS659XX to mute the audio
amplifier during enable/disable transitions is missing from the EVM
design. This causes noise to be heard when the audio amplifier is
enabled/disabled at the start/end of audio playback. This issue has been
fixed on the EVM2.

*************************************************
Installation
*************************************************

This BSP must be used in conjunction with Windows Embedded CE6.0 R2.
The following tools must be installed in the following order to use this
BSP:

    1) Microsoft Visual Studio 2005 (VS2005)
    2) Microsoft Visual Studio 2005 Service Pack 1
    3) Windows Embedded CE6.0 (Plug in for VS2005)
    4) Windows Embedded CE6.0 Service Pack 1 (SP1)
    5) Windows Embedded CE6.0 R2 Update
    6) Windows Embedded CE6.0 Cumulative Product Update Rollup Package 2007 (QFEs)
       and additional cumulative updates, included CE6R3 if desired.	
    7) Remaining Windows Embedded CE6.0 monthly updates (QFEs) if desired.

Once the tools are installed in the correct sequence, the TI_EVM_3530
BSP  platform dependent and platform independent code must be installed.
Follow the instructions in the _Installation.txt file located in the BSP
release directory.

*************************************************
Downloading WinCE Image Using PlatformBuilder
*************************************************

The bootloader supports image download using PlatformBuilder controlled
via the bootloader serial user interface.  The following procedure can
be used to download the WinCE image onto the device.

1) Configure TIEVM hardware
    a) Connect serial cable to UART3
    b) Connect ethernet cable to ethernet jack
        Note: The EVM must be on the same subnet as host PC running PlatformBuilder.

2) Open Terminal program on host PC (115200, N, 8, 1)

3) Power up device
    a) Status messages will be displayed on serial port
    b) 4 color boot screen will appear
    c) On first boot only the flash will be reformatted.  This can be a lengthy
        operation, be patient.

4) Launch Platform Builder and open the OS Design project containing the EVM3530 BSP.  

5) Configure PlatformBuilder Target Connectivity Options
    a) From Target menu select Connectivity Options
    b) Select Ethernet as Download transport
    c) Click Settings button to display Ethernet settings
    d) Start download from bootloader serial menu.  This will happen automatically
        if the ethernet cable is plugged in and a MAC address is programmed.  Otherwise
        program MAC address using menu option 4,9.  Once MAC address is 
        programmed exit from menu which wil restart download.
    e) Select the name string that will appear in Ethernet Settings dialog.  It
        will be in the format EVM3530-xxxxx.
    f) Close Target Connectivity Options dialog box

6) Select Attach Device from the Target menu in Platform Builder to initiate the download

The OS image will be downloaded, programmed into flash if applicable,
and launched.  Note that the image will only be programmed into flash if
it was built with the IMGFLASH  environment variable set.

*************************************************
Programming Bootloader Into NAND/ONENAND
*************************************************

The bootloader can be programmed into flash using EVMFlash tool. This
tool is must be used when there is no functional bootloader already
located in ONENAND or NAND flash.

Note that the SW4 settings may need to be changed to select a boot
device compatible with the cable used by EVMFlash tool to communicate
with the EVM.

Please see the documentation provided with that tool for details on its use.

Note that there are 2 versions of the EVM one using Micron NAND and one
using Samsung ONENAND and that there are 2 versions of the EVMFlash tool.

*************************************************
Booting from ONENAND
*************************************************

Note: When switching between NAND and ONENAND builds, you must do a clean build.

Note: To boot the NK.BIN image from the ONENAND, two things must happen:

1. The image must be built to target FLASH (IMGFLASH enabled). This is
controlled by the Project->EVM_3530 Properties..., Configuration
Properties, Build Options "Write run-time image to flash memory
(IMGFLASH=1)" check box. 

2. The image must be downloaded to the system using EBOOT and platform
builder. After downloading the image the bootloader will write the
nk.bin image into the NAND.

To select ONENAND as the 1st boot device, change SW4 to one of the
following:

    position 1 ON,  2 ON,  3 ON,  4 ON,  5 ON,  6 ON,  7 OFF, 8 OFF
    position 1 ON,  2 OFF, 3 ON,  4 ON,  5 ON,  6 ON,  7 OFF, 8 OFF
    position 1 ON,  2 ON,  3 OFF, 4 ON,  5 ON,  6 ON,  7 OFF, 8 OFF
    position 1 ON,  2 ON,  3 ON,  4 ON,  5 OFF, 6 ON,  7 OFF, 8 OFF
    position 1 ON,  2 OFF, 3 OFF, 4 ON,  5 OFF, 6 ON,  7 OFF, 8 OFF

For serial cable EVMFlash tool, select UART as the 1st boot device and ONENAND as
the 2nd boot device, change SW4 to:

    position 1=ON,  2=OFF, 3=OFF, 4=ON,  5=OFF, 6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the ONENAND.

For USB cable EVMFlash tool select USB as the 1st boot device and ONENAND as the
2nd boot device, change SW4 to:

    position 1=ON,  2=ON,  3=OFF, 4=ON,  5=ON,  6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the ONENAND.

*************************************************
Booting from NAND
*************************************************

Note: When switching between NAND and ONENAND builds, you must do a clean build.

Note: To boot the NK.BIN image from the NAND, two things must happen:

1. The image must be built to target FLASH (IMGFLASH enabled). This is
controlled by the Project->EVM_3530 Properties..., Configuration
Properties, Build Options "Write run-time image to flash memory
(IMGFLASH=1)" check box. 

2. The image must be downloaded to the system using EBOOT and platform
builder. After downloading the image the bootloader will write the
nk.bin image into the NAND.

Change SW4 to select NAND as the 1st boot device, use one of the following:

    position 1=OFF, 2=ON,  3=ON,  4=ON,  5=ON,  6=ON, 7=OFF, 8=OFF (NAND, SSI, USB)
    position 1=ON,  2=ON,  3=ON,  4=OFF, 5=ON,  6=ON, 7=OFF, 8=OFF (NAND, SSI, USB, MMC1)
    position 1=OFF, 2=OFF, 3=OFF, 4=OFF, 5=ON,  6=ON, 7=OFF, 8=OFF (NAND, USB, UART, MMC1)
    position 1=OFF, 2=ON,  3=OFF, 4=ON,  5=OFF, 6=ON, 7=OFF, 8=OFF (NAND, UART)
    position 1=OFF, 2=OFF, 3=ON,  4=OFF, 5=OFF, 6=ON, 7=OFF, 8=OFF (NAND, USB)

When using EVMFlash tool with serial cable interface, select UART as the 1st boot
device and NAND as the 2nd boot device, change SW4 to:

    position 1=OFF, 2=ON,  3=OFF,  4=ON, 5=OFF, 6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the NAND.

When using EVMFlash tool with USB cable interface, select USB as the 1st boot
device and NAND as the 2nd boot device, change SW4 to:

    position 1=OFF, 2=OFF, 3=ON,  4=OFF, 5=OFF, 6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the NAND.

*************************************************
Booting from SD memory card (SD boot)
*************************************************

Note: The NK.BIN image must be built to target RAM (IMGFLASH disabled).
This is controlled by the Project->EVM_3530 Properties..., Configuration
Properties, Build Options "Write run-time image to flash memory
(IMGFLASH=1)" check box.

Note: Some older MMC/SD memory cards are not recognized by the CPU
internal boot ROM. If you cannot get SD boot to work, be sure to try
with other cards. This is an issue with the CPU internal boot ROM and
cannot be fixed in the BSP code.

Note: When switching between NAND and ONENAND builds, you must do a clean build.

1. Format the SD Card under WinXP.
 
2. Make the SD Card bootable using Roadkil's sector editor or another
utility: 

- Open the SD card physical sector 0

- Write 0x80 at offset 0x1BE (1st partition entry) 

- Save sector 

- Exit Sector Editor application 

3. Copy ONLY the MLO file to the SD card. Note that the MLO file is not
created during a debug build.

4. Copy both the EBOOTSD.NB0 and the NK.BIN files to the SD card.

5. Configure the EVM SW4 switch to select MMC1 as the 1st boot device, use
one of the following:

    position 1=ON,  2=OFF, 3=OFF, 4=ON,  5=ON,  6=ON, 7=OFF, 8=OFF
    position 1=ON,  2=OFF, 3=ON,  4=ON,  5=OFF, 6=ON, 7=OFF, 8=OFF
    position 1=ON,  2=ON,  3=ON,  4=OFF, 5=OFF, 6=ON, 7=OFF, 8=OFF

Alternately, when using EVMFlash tool with serial cable interface, configure the
EVM SW4 switch to select UART as the 1st boot device and MMC1 as the 2nd
boot device:

    position 1=ON,  2=ON,  3=ON, 4=OFF, 5=OFF, 6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the SD card.

Alternately, when using EVMFlash tool with USB cable interface, configure the EVM
SW4 switch to select USB as the 1st boot device and MMC1 as the 2nd boot
device:

    position 1=ON, 2=OFF, 3=OFF, 4=ON,  5=ON,  6=OFF, 7=OFF, 8=OFF

Note that when using the above switch setting, the bootloader will not
automatically boot from the SD card.
    
6. Plug the SD Card into the EVM. 

7. Power on the EVM -> it should boot from SD. 

The CPU internal ROM will load the SD XLDR (MLO), the SD XLDR will load
SD EBOOT (ebootsd.nb0). After the SD EBOOT bootloader is running, use
the menu to select the SD card as the boot device, then select continue
to boot the NK.BIN file. Note that the MMC/SD boot currently takes about
2 seconds per MB of NK.BIN file size.
