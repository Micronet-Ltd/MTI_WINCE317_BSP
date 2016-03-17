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

if not "%2" == "" if not exist %2\. (
  @echo.
  @echo %2\ does not appear to exist!  Continue anyway???
  @echo.
  rem pause
)

call prjbuild /nopause %1
if errorlevel 1 goto Failed

@echo #################################################
@echo #########  SKIPPING FILE COPY 
@echo #################################################

goto Fini


:Failed
@echo #################################################
@echo #########  FAILED: ERRORLEVEL=%ERRORLEVEL% 
@echo #################################################
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
