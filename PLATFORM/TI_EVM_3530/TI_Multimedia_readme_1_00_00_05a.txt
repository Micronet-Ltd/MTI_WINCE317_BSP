TI Multimedia Bundle Release notes
==================================

=================================
DVSDK (Binary) Version 1.00.00.05a
=================================

This is a binary release of the Directshow filters (MPEG2, MPEG4, H264 video decode, 
aac, mp3 audio decode, aac parser filter, mp4/3gp container) 
and several DMAI sample applications. 

==================
Component Versions
==================

Component versions can be found in OMAP3 WinCE DVSDK Software Manfest (1.00.00.05a).pdf

This was verified with WinCE 6.0 R3
The DVSDK was verified on both TMDXEVM3503(EVM1) as well as TMDSEVM3530 (EVM2).

DVSDK Demos and Examples:

DMAI examples for performing file based decode and encode operations.
WMP to exercise avi audio/video playback.

===================
New in this release
===================

- Dshow filters updated to version 1.10.00.05
  - Added AAC parser and decode filters to allow AAC file playback
  - Added MP3 decode filter as an option a customer can enable if they choose to offload MP3 decoding onto DSP
    (mp3 codec is not part of DSP codec server. This needs to be integrated separately)
  - Added support for MP4/3GPP containers for the following formats:
            - H264 + AAC
            - MPEG4 + AAC
  - Improved performance of video filters using SDMA
  - Implemented quality control interface to drop decoded frames when frames are late

===================
New in 1.00.00.05
===================

- GA version of DMAI and DirectShow filters
- Performance improvement of media playback
   - Multimedia performance improvements due to increased display backbuffers as well as 
     higher clk speed (both are set in the BSP)
- Same Codec server can be used for different silicon speeds. DSP BIOS/LINK updates the DSP 
  code with the correct clock information at startup.

=====================
Fixed in 1.00.00.05
=====================

- Rotation support (VRFB) for H.264, MPEG2, MPEG4 codecs with updated directshow filters

=====================
Fixed in 1.00.00.04
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

- Rotation (VRFB) may not work for Video codecs that are part of WinCE 6.0 running on ARM 
  (e.g, MPEG1 video, WMV video). These codecs do not handle the stride information properly. 
  Please contact Microsoft for further details.
- Running IE along with WMP results in audio stutter and loss of AV Sync
- Only AVI, MP4, 3GP containers are supported for WMP.
- AV synchronization is off for video clips at D1 resolution when the bitrate
  is high (e.g. >= 2Mbps) on EVMs with ES2.1 silicon. Performance can be improved by
  using ES 3.1 silicon with multiple block reads enabled for the SDIO driver.
- Media player using directshow filters cannot be run at the same time as DMAI applications, 
  due to lack of an arbitrator of DSPLINK resources.
- MPEG2 decoder related: 
SDOCM00057731: MPEG2 Decoder: Flicker is observed at top of the display while decoding Mpeg2 streams of resolution 352x240 with video decode
SDOCM00062310: MPEG2 Decoder: Generated yuv resolution is not same as the input stream resolution for some MP streams
SDOCM00060533: MPEG2 Decoder: Unable to decode certain clips @ QCIF resolution

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
Sample commands to test selected encode/decode operations
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
s image_decode_io1_omap3530 -c jpegdec -i /Release/data/fruitbasket_352x288_420p.jpg -o /Release/results/fruitbasket_352x288_420p.jpg.uyvy

Perform MPEG2 decode of 50 frames of an mpeg2 elementary stream into UYVY RAW frames:
s video_decode_io2_omap3530 -c mpeg2dec -i /Release/data/davincieffect_cif.m2v -o /Release/results/davincieffect_cif.m2v.uyvy -n 50
 
Perform a speech decode of 1000 speech frames from g.711 alaw format to 16 bit linear PCM format: 
s speech_decode_io1_omap3530 -c g711dec -i /Release/data/davincieffect.g711 -o /Release/results/davincieffect.pcm --compandinglaw alaw -n 1000

Test commands for dmai encode apps
==================================
Note: Some of the encode input files are results of the decode operation above and the appropriate file should be copied to the /Release/data folder.

Perform JPEG encode of a UYVY RAW frame of resolution 352x288 into a jpeg image: 
s image_encode_io1_omap3530 -c jpegenc -i /Release/data/fruitbasket_352x288_420p.jpg.uyvy -o /Release/results/fruitbasket_352x288_420p.jpg.uyvy.jpg -r 352x288 --iColorSpace 3 

Perform H264 encode of 50 UYVY RAW frames of resolution 720x480 into an H264
elementary stream at a bitrate of 1 Mbps: 
s video_encode_io1_omap3530 -c h264enc -i /Release/data/davincieffect_ntsc.264.uyvy -o /Release/results/davincieffect_ntsc.264 -r 720x480 -b 1000000 -n 50

To perform a speech encode of 1000 speech frames from 16bit linear PCM format to g.711 alaw format: 
s speech_encode_io1_omap3530 -c g711enc -i /Release/data/davincieffect.pcm -o /Release/results/davincieffect.alaw --compandinglaw alaw -n 1000

-------------------
Test media files
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
Procedure to test
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
 
----------------------------------------
DMAI sample applications known issues
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
