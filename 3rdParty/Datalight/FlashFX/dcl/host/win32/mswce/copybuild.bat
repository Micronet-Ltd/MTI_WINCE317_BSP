@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file contains the back-end logic for copying binaries from
REM the Project Directory tree into the CE build tree.
REM
REM On exit, B_ERROR will be clear if there were no errors, and set to an
REM error context if there was a problem.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: copybuild.bat $
REM Revision 1.2  2009/04/09 19:38:23Z  keithg
REM Fixed bug 2543, clarified pause condition.
REM Revision 1.1  2008/10/03 00:40:14Z  garyp
REM Initial revision
REM Revision 1.1.1.3  2008/10/03 00:40:14Z  garyp
REM Use more robust syntax on the copy command.
REM Revision 1.1.1.2  2008/07/06 09:27:42Z  garyp
REM Modified to not do a "hard" pause, even after a syntax error.
REM Revision 1.1  2008/06/21 00:29:20Z  garyp
REM Initial revision
REM Revision 1.4  2008/04/02 20:37:18Z  garyp
REM Updated to copy into the TARGETPLATROOT as well, if it exists.
REM Revision 1.3  2008/03/19 19:29:29Z  Garyp
REM Modified per recommendations to copy LIBs into the oak/lib directory, and
REM other executables into the oak/target directory.
REM -------------------------------------------------------------------------

@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

set B_ERROR=

if "%1" == "?" goto Syntax
if "%1" == "/?" goto Syntax

REM set B_PAUSE=pause
REM Leave the statement above commented out so we inherit the B_PAUSE value
REM which was probably initialized in makeall.bat.  Nevertheless, if the
REM option is specified on the command-line, honor it.

if "%1" == "/nopause" set B_PAUSE=
if "%1" == "/nopause" shift

if not "%1" == "" goto Syntax

if "%B_RELDIR%" == "" goto BadEnv
if not exist %B_RELDIR%\*.* goto BadRel
if "%_TARGETPLATROOT%" == "" goto BadCeEnv
if "%_FLATRELEASEDIR%" == "" goto BadCeEnv
if "%_PROJECTROOT%" == "" goto BadCeEnv
if "%_WINCEOSVER%" == "" goto BadCeEnv
if "%WINCEDEBUG%" == "" goto BadCeEnv
if "%_TGTCPU%" == "" goto BadCeEnv

REM CE 4.x uses a different target directory than CE 5.0, though some CE 4.2
REM BSPs seem to have a similar structure as used in CE 5.0.  Handle whichever
REM cases we find...
REM
set DST2=%_PROJECTROOT%\cesysgen\oak
if not exist %DST2%\*.* set DST2=%_PROJECTROOT%\oak

set DSTTARG=
set DSTLIB=
if exist %DST2%\target\*.* set DSTTARG=%DST2%\target\%_TGTCPU%\%WINCEDEBUG%
if exist %DST2%\lib\*.*    set DSTLIB=%DST2%\lib\%_TGTCPU%\%WINCEDEBUG%

set TPRTARG=
set TPRLIB=
if exist %_TARGETPLATROOT%\target\*.* set TPRTARG=%_TARGETPLATROOT%\target\%_TGTCPU%\%WINCEDEBUG%
if exist %_TARGETPLATROOT%\lib\*.*    set TPRLIB=%_TARGETPLATROOT%\lib\%_TGTCPU%\%WINCEDEBUG%

echo.
echo About to copy LIBs:
echo   From: %B_RELDIR%
echo   To:   %DSTLIB%\...
if not "%TPRLIB%" == "" echo   And:  %TPRLIB%\...
echo.
echo About to copy DLLs/EXEs:
echo   From: %B_RELDIR%
echo   To:   %_FLATRELEASEDIR%\...
if not "%DSTTARG%" == "" echo   And:  %DSTTARG%\...
if not "%TPRTARG%" == "" echo   And:  %TPRTARG%\...
echo.

if     "%B_DEBUG%" == "0" if "%WINCEDEBUG%" == "retail" goto SkipWarning
if not "%B_DEBUG%" == "0" if "%WINCEDEBUG%" == "debug"  goto SkipWarning
echo NOTE: The %P_PRODUCT% debug level (B_DEBUG=%B_DEBUG%) does not match
echo       the Windows CE debug setting (WINCEDEBUG=%WINCEDEBUG%).
echo.
:SkipWarning
if "%B_PAUSE%" == "" goto SkipPause
echo Press any key to continue, or press CTRL+C to abort...
%B_PAUSE% >nul
:SkipPause

echo Copying files...
if not "%DSTLIB%" == "" if exist %B_RELDIR%\*.lib copy %B_RELDIR%\*.lib %DSTLIB%\.
if not "%TPRLIB%" == "" if exist %B_RELDIR%\*.lib copy %B_RELDIR%\*.lib %TPRLIB%\.

REM Hide all the LIBs for this next copy...
if exist %B_RELDIR%\*.lib attrib +h %B_RELDIR%\*.lib
if exist %B_RELDIR%\*.dll copy %B_RELDIR%\*.* %_FLATRELEASEDIR%
if not "%DSTTARG%" == "" if exist %B_RELDIR%\*.dll copy %B_RELDIR%\*.* %DSTTARG%\.
if not "%TPRTARG%" == "" if exist %B_RELDIR%\*.dll copy %B_RELDIR%\*.* %TPRTARG%\.
if exist %B_RELDIR%\*.lib attrib -h %B_RELDIR%\*.lib
goto END

:BadEnv
echo.
echo B_RELDIR must point to your project's release directory.  This
echo variable gets set as part of the PRJBUILD process.
echo.
echo Please re-run PRJBUILD, and then run this batch file again.
echo.
set B_ERROR=PRJCOPY_BadEnv
goto DoPause

:BadRel
echo.
echo The product binaries don't exist in the location specified by
echo B_RELDIR (%B_RELDIR%).
echo.
echo Please re-run PRJBUILD, and then run this batch file again.
echo.
set B_ERROR=PRJCOPY_BadRel
goto DoPause

:BadCeEnv
echo.
echo The CE environment variables _FLATRELEASEDIR, _WINCEOSVER, WINCEDEBUG,
echo _TGTCPU, _TARGETPLATROOT, and _PROJECTROOT must all be set.
echo.
echo To set these, this batch file is typically run from a command prompt
echo opened from Platform Builder.  From the "Build" menu, choose "Open
echo Release Directory...", then change to this Project Directory and run
echo the PRJBUILD/PRJCOPY batch files again.
echo.
set B_ERROR=PRJCOPY_BadCeEnv
:DoPause
if not "%B_PAUSE%" == "" %B_PAUSE% >nul
goto END

:Syntax
echo The PRJCOPY command copies the products binaries (DLLs, EXEs, and LIBs) to the
echo appropriate places in the CE directory tree.  The only allowed command-line
echo option is "/nopause", which will cause the operation to happen without user
echo intervention, so long as there are no fatal syntax errors.
set B_ERROR=PRJCOPY_Syntax

:END
set DST2=
set DSTTARG=
set DSTLIB=
set TPRTARG=
set TPRLIB=

if "%B_ERROR%" == "" (exit /b 0) else (exit /b 1)

