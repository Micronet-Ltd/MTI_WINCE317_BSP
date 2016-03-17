@echo off

REM -------------------------------------------------------------------------
REM				Description
REM
REM This batch file builds the Bootloader version of FlashFX (FXBOOT.LIB)
REM and then builds the various WinMobile bootloader components which use
REM it.  It will optionally copy the final bootloader binaries to the 
REM specified target location.
REM
REM This batch file requires the standard WinMobile "WPC" environment.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM				Revision History
REM $Log: BuildBL.bat $
REM Revision 1.3  2010/07/18 00:24:34Z  garyp
REM Ignore white space for the file comparison.
REM Revision 1.2  2010/07/17 00:30:25Z  garyp
REM Updated to check that fmd_flashfx.c is unchanged.
REM Revision 1.1  2010/07/04 17:59:58Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

if "%1" == "?" goto Syntax
if "%1" == "/?" goto Syntax

if /i not "%_TGTPROJ%" == "wpc" goto BadProj


if not "%2" == "" if not exist %2\. (
  @echo.
  @echo %2\ does not appear to exist!  Continue anyway???
  @echo.
  rem pause
)

fc /w fmd_flashfx.c %_TARGETPLATROOT%\src\drivers\block\nand\fmd_flashfx.c >NUL
if errorlevel 1 (
  echo.
  echo WARNING: The fmd_flashfx.c in this directory does not match that which
  echo          resides in the WinMobile tree!  Continue anyway....???
  echo.
  rem pause
)

call prjbuild /nopause %1
if errorlevel 1 goto Failed

pushd %_TARGETPLATROOT%\src\drivers\block\nand

build -c
if errorlevel 1 goto Failed

cd %_TARGETPLATROOT%\src\boot

build -c
if errorlevel 1 goto Failed

popd


pushd %_FLATRELEASEDIR%

call postmakeimg.bat
if errorlevel 1 goto Failed

if not "%2" == "" if exist %2\. (
  REM Prompt for each one of these since the process will build all of them
  REM however some will only work when FlashFX is in release mode, due to
  REM the size of the binary.
  REM
  @echo.
  dir %2\
  @echo.
  copy /-Y mlo                   %2\.
  copy /-Y xldr_signed.bin       %2\xldr.bin
  copy /-Y eboot_dev_signed.bin  %2\eboot.bin
  copy /-Y ebootsd.raw           %2\.
  copy /-Y ipl_signed.bin        %2\ipl.bin
  REM copy /-Y flash_signed.bin  %2\flash.bin
) else (
  @echo #################################################
  @echo #########  SKIPPING FILE COPY 
  @echo #################################################
)

popd
goto Fini


:Failed
@echo #################################################
@echo #########  FAILED: ERRORLEVEL=%ERRORLEVEL% 
@echo #################################################
goto Fini

:BadProj
@echo.
@echo This batch file is normally run from a "WPC" WinMobile environment.
@echo.
goto Fini

:Syntax
@echo.
@echo This batch file builds FlashFX and the WinMobile bootloader components and
@echo optionally copies them to a target location.  
@echo.
@echo The first parameter is the FlashFX DEBUG level, 0 to 3.
@echo.
@echo The optional second parameter is the path of the target location.  If this
@echo option is not specified, the bootloader components will not be copied.
@echo.


:Fini
