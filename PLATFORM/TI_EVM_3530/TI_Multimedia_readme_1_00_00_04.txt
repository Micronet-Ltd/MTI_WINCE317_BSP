TI Multimedia Bundle Release notes
==================================

=================================
DVSDK (Binary) Version 1.00.00.04
=================================

This is a binary release of the Directshow filters (MPEG2, MPEG4, H264 
decode) and several DMAI sample applications. 

==================
Component Versions
==================
   * Codec Engine 2.24 GA
   * WinCE DSP Link 1.61.04 GA
   * Codec Server CS1OMAP3530 1.00.00 GA with following codecs
        * H264 Decoder      2.01.004 GA
        * H264 Encoder      2.01.012 GA
        * MPEG4 Decoder     2.00.009 GA
        * MPEG4 Encoder     2.01.006 GA
        * MPEG2 Decoder     2.00.008 GA
        * JPEG Decoder      2.00.004 GA
        * JPEG Encoder      2.01.004 GA
        * AAC LC/HE Decoder 1.30.002 GA
        * G711 Encoder      1.12.00.000 GA
        * G711 Decoder      1.12.00.000 GA
   * DMAI 1.24.00.04
   * Direct Show Filters 1.00.00.04

This was verified with WinCE 6.0 R2 - updated with Cumulative Product Update
Rollup Package 2007 and Windows Embedded CE6.0 monthly updates (QFEs) up to
April 2009

DVSDK Demos and Examples:

DMAI examples for performing file based decode and encode operations.
WMP to exercise avi audio/video playback.

===================
New in this release
===================

- Unified codec server for all the codecs
- GA versions of Codec Engine, DSP BIOS/LINK, Codec server, Codecs
- Speech (G.711) codec support
- Performance improvement of media playback
   - Optimized YUV-RGB conversion routine
   - Improvements in DirectShow filters
   - Optimization in codec server

=====================
Fixed in this release
=====================

- Occasional dropped frames are observed. Performance can be improved by using 
ES 3.1 silicon with multiple block reads enabled for the SDIO driver.
- Reducing the window size of the Windows Media Player to a size smaller than
the frame size may result in corrupted output.
- Video display is corrupted when part of the display window lies outside the 
LCD display.
- Directshow filters do not meet real-time and display is corrupted when 
video renderer operates in GDI mode (ie. when Windows Media Player is hidden by
another window). This is because GDI mode requires YUV to RGB conversion of the
DSP codec output frames.
- Due to a problem with the CERuntime_exit() API in Codec Engine, this API is
not called and some resources are not totally freed even after closing 
Windows Media Player. 
- WMP hangs due to exception in timm.dll when unsupported input file is selected

========================
Known issues/limitations
========================

- Rotation through VRFB is not supported by the video display in WMP
- Running IE along with WMP results in audio stutter and loss of AV Sync
- MP3 playback can only be done on the ARM. This results in higher ARM-side CPU
load.
- Only AVI containers are supported for WMP.
- AV synchronization is off for video clips at D1 resolution when the bitrate
is high (e.g. >= 1Mbps). Performance can be improved by using ES 3.1 
silicon with multiple block reads enabled for the SDIO driver.
- Directshow filters cannot be run at the same time as DMAI applications, due to
lack of an arbitrator of DSPLINK resources.

========================
DMAI sample applications
========================
 
The following dmai apps (.exe) are included in the release

- video_decode_io2_omap3530
- video_encode_io1_omap3530
- audio_decode_io1_omap3530
- audio_encode_io1_omap3530
- image_decode_io1_omap3530
- image_encode_io1_omap3530
- speech_decode_io1_omap3530
- speech_encode_io1_omap3530

Each app can be used to exercise one specific type of media (audio, video or 
image).

The DSP combo image cs.x64-is expected to be under the windows folder
of the target device when the device has booted.

------------------------------------------------------------
1. Sample commands to test selected encode/decode operations
------------------------------------------------------------

Each command represents one test using one of the DMAI apps for WinCE. The 
command has to be manually entered (or copy pasted) into the 
Windows CE command prompt window. Please ensure you have KITL enabled in 
your OS design so that the board can access the applications and/or media from 
the release directory.

Test commands for dmai decode apps
==================================

Perform H264 decode of 50 frames of an h264 elementary stream into UYUV RAW 
frames: 
s video_decode_io2_omap3530 -c h264dec -i /Release/data/davincieffect_ntsc.264 -o /Release/results/davincieffect_ntsc.264.uyvy -n 50

Perform AAC decode of 200 frames of an AAC elementary stream into a PCM audio 
file: 
s audio_decode_io1_omap3530 -c aachedec -i /Release/data/davincieffect.aac -o /Release/results/davincieffect.aac.pcm -n 200

Perform JPEG decode of a jpeg image into a UYUV RAW frame:
s image_decode_io1_omap3530 -c jpegdec -i /Release/data/shrek.jpg -o /Release/results/shrek.uyvy

Perform MPEG2 decode of 50 frames of an mpeg2 elementary stream into UYVY RAW 
frames:
s video_decode_io2_omap3530 -c mpeg2dec -i /Release/data/davincieffect_cif.m2v -o /Release/results/davincieffect_cif.m2v.uyvy -n 50
 
Perform a speech decode of 1000 speech frames from g.711 alaw format to 16 bit linear PCM format: 
s speech_decode_io1_omap3530 -c g711dec -i /Release/data/davincieffect.g711 -o /Release/results/davincieffect.pcm --compandinglaw alaw -n 1000

Test commands for dmai encode apps
==================================

Perform JPEG encode of a UYVY RAW frame of resolution 720x480 into a jpeg image: 
s image_encode_io1_omap3530 -c jpegenc -i /Release/data/shrek.uyvy -o /Release/results/shrek.uyvy.jpg -r 720x480 --iColorSpace 3

Perform H264 encode of 50 UYVY RAW frames of resolution 720x480 into an H264
elementary stream at a bitrate of 1 Mbps: 
s video_encode_io1_omap3530 -c h264enc -i /Release/data/davincieffect_ntsc.264.uyvy -o /Release/results/davincieffect_ntsc.264 -r 720x480 -b 1000000 -n 50

To perform a speech encode of 1000 speech frames from 16bit linear PCM format to g.711 alaw format: 
s speech_encode_io1_omap3530 -c g711enc -i /Release/data/davincieffect.pcm -o /Release/results/davincieffect.alaw --compandinglaw alaw -n 1000

-------------------
2. Test media files
-------------------

The commands given above assume that media files are located in the flat release
directory, e.g. in /Release/data. Note that the media files are not provided as
part of the BSP, and must be downloaded separately.

Similarly the output files will be located in the flat release directory, e.g.
/Release/results

If using the exact same syntax as given in section 1, please make sure that the 
directories and files mentioned for input and the directories for output are 
available. Otherwise the applications will fail.

The media files can be placed in other locations such as MMC card/SD card. The 
command syntax shown above needs to change to reflect the correct path.

--------------------
3. Procedure to test
--------------------

- Copy the app ".exe"s to the target platform's flat release directory (for 
e.g, C:\WINCE600\OSDesigns\EVM_3530\RelDir\TI_EVM_3530_ARMV4I_Release)

- Once the WinCE image has booted and windows desktop is visible on the LCD 
screen, from Visual Studio project select Target->Target Control. 

- You should see Windows CE command prompt
 
    Windows CE Command Prompt
        <command>: Shell commands ('?' for shell help)
        '.<command>': Debugger commands ('.?' for help)
        '!<command>': Debugger extension commands
        Ctrl-Q: Abort pending command
        Ctrl-L: Clear all
        Ctrl-A: Select all
        Ctrl-F: Find (F4: Search forward, Shift-F4: Search backwards)
    
    Windows CE>

At the command prompt enter one of the commands. For e.g image decode.

    Windows CE> s image_decode_io1_omap3530 -c jpegdec -i /Release/data/pics/shrek.jpg -o /Release/results/pics/shrek.uyvy

The command will return immediately. The test execution status is displayed on 
the target LCD screen in a console window. When the test finishes you should 
see the output file generated in the specified location.

If there is an error you will see it in the target LCD screen in the console 
window. Unfortunately the console window closes when the application exits.

If the media files are located on MMC/SD card (i.e, "Storage Card" folder  
on the target device) then the syntax would change to 

    Windows CE> s image_decode_io1_omap3530 -c jpegdec -i "/Storage Card/shrek.jpg" -o "/Storage Card/shrek.uyvy"

The path needs to be in quotes to handle the space in directory name.
 
--------------------------------------------------------
4. DMAI sample applications issues fixed in this release
--------------------------------------------------------

- Currently it is not possible to switch between decode apps and encode apps. 
For that a fresh boot is required. But any number of EITHER decode apps OR 
encode apps can be run one after the other. This is because this release is 
based on the EA2 release of Codec Engine that does not have LPM, so the cache 
state is not reset when reloading a different server, possibly leading to cache
coherency issues.
- Apps command line option "--benchmark" is not implemented and test will fail 
if used with that option.
- video_encode_io1_omap3530 has a problem where it only encodes the first file 
(length of the clip does not matter) correctly. A second encode call will result
in the encode going through but the resulting encoded file content is not 
correct. 

----------------------------------------
5. DMAI sample applications known issues
----------------------------------------
- Cannot exercise audio_encode_io1_omap3530.exe dmai app since there is no audio
  encoder in the codec server
- DMAI application does not support 411p and 422i as possible chroma formats
- When performing jpeg image encode using DMAI application of an image with 
  dimension (height or width) that is not a multiple of 16, leads to distorted
  output. This is a JPEG encoder known limitation.
- When performing jpeg image encode using DMAI application, it was observed that
  using qfactor above 96 leads to misalignment in output image.
- When performing AAC audio decode using DMAI application, Mono files are decoded
  as dualmono during aac decode operations.
- The default CMEM configuration allows JPEG encode and decode of images with
  resolutions upto 1280x720 (1MPixel) with YUV444, YUV422, YUV420 formats. 
  Images of higher resolution can be encoded/decoded with appropriate increase
  in CMEM buffer configuration within limits of available memory
