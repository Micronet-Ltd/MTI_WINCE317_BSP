@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file builds the product for Win32 hosted environments.
REM
REM In addition to the environment variables described in the syntax, the
REM following variables will be modified or set during the course of the
REM build process:
REM
REM B_DEBUG    - This will be set to the level specified on the command
REM              line, if any.
REM B_LOG      - The full filespec of the build output log
REM B_OBJEXT   - The filename extension for object code files (OBD or OBR)
REM B_RELDIR   - The location where the generated drivers will be placed
REM B_MAKE     - The full name of the make program to use
REM B_TOOLBIN  - The dir where the ToolSet binaries reside (P_TOOLROOT relative)
REM B_TOOLINC  - The dir where the ToolSet include files reside (P_TOOLROOT relative)
REM B_TOOLLIB  - The dir where the ToolSet libraries reside (P_TOOLROOT relative)
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: makeall.bat $
REM Revision 1.19  2010/11/30 21:28:30Z  garyp
REM Added a /nogui command-line option to avoid using the GUI file viewer
REM to display errors.
REM Revision 1.18  2010/05/20 21:51:20Z  garyp
REM Concatenate the ignore.txt files with the /A switch.
REM Revision 1.17  2010/05/07 23:09:33Z  garyp
REM Updated SHOWERR to use the new /Q option.
REM Revision 1.16  2010/05/07 21:33:32Z  garyp
REM Updated to concatenate the IGNORE.TXT files from the main product,
REM DCL, and the ToolSet.
REM Revision 1.15  2009/11/23 13:49:53Z  garyp
REM Updated so that changing B_CFLAGS will cause lastproj.bat to 
REM invalidate a build.
REM Revision 1.14  2009/06/28 01:08:51Z  garyp
REM Updated to use the new "view" utility to display errors.  Eliminated the
REM SHREDIR logic, as it is no longer necessary with the new version of 
REM GNU Make.
REM Revision 1.13  2009/05/17 17:45:41Z  garyp
REM Minor error handling adjustments.
REM Revision 1.12  2009/03/10 22:35:57Z  keithg
REM Added quotes to P_TOOLROOT evaluation missed in a prior checkin.
REM Revision 1.11  2009/02/08 00:18:05Z  garyp
REM Merged from the v4.0 branch.  Improved the tests which determine the proper
REM value for the B_CPUCODE setting.  Updated to display B_DRIVERLIBS.  Updated
REM so that B_CFLAGS is handled as a standard project flag.  Updated to call
REM prjcopy.bat automatically if it exists in the project, and if the build was
REM successful.
REM Revision 1.10  2007/08/18 21:46:10Z  garyp
REM Updated to work with the new version number format.
REM Revision 1.9  2007/07/18 21:29:02Z  keithg
REM Added quotes to P_TOOLROOT evaluation to work with the default MSVC
REM installation path.
REM Revision 1.8  2007/05/02 21:22:11Z  Garyp
REM Updated makeall.bat to call detect.bat if the P_TOOLROOT value is not set.
REM Revision 1.7  2007/01/20 14:34:41Z  Garyp
REM Improved logic which decides when to automatically clean first to better
REM detect situations where trees with pre-existing object code are being
REM copied around.
REM Revision 1.6  2007/01/02 22:20:31Z  Garyp
REM Modified so the error log will be displayed even when running in
REM "NoPause" mode.
REM Revision 1.5  2006/12/08 01:39:40Z  Garyp
REM Updated to allow a given ToolSet to have its own ignore.txt which will get
REM appended to the product specific ignore.txt.
REM Revision 1.4  2006/10/16 17:24:20Z  Garyp
REM Improved error handling.
REM Revision 1.3  2006/10/08 03:13:32Z  Garyp
REM Updated to work with the new SHOWERR.EXE v2.3, which returns a specific
REM errorlevel if the data contains warnings only.  In this event, if all other
REM aspects of the build were successful, the B_ERROR value returned by MAKEALL
REM will be "BuildWarnings".  This can then be detected and specially handled
REM by higher level processes if need be.
REM Revision 1.2  2006/08/26 00:09:12Z  Garyp
REM Updated to dynamically build shredir.tmp in the Project Directory.
REM Revision 1.1  2005/10/01 13:28:32Z  Pauli
REM Initial revision
REM Revision 1.4  2005/06/12 22:19:26Z  PaulI
REM Cleared product/subproduct numbers and prefix.
REM Revision 1.3  2005/04/26 18:36:06Z  GaryP
REM Added support for the B_BYTEORDER setting.
REM Revision 1.2  2005/03/17 18:28:38Z  GaryP
REM Added support for P_OSROOT and P_OSVER.  Now do the OS initialize
REM before the ToolSet initialization.
REM Revision 1.1  2005/01/18 15:11:21Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "?"  goto Syntax
if "%1" == "/?"  goto Syntax
if "%1" == "-?"  goto Syntax

REM ----------------------------------------------------------------------
REM  Initial settings
REM ----------------------------------------------------------------------

REM These sub-product settings should be clear when beginning an initial build
set P_SUBPREFIX=
set P_SUBPRODUCT=
set P_SUBPRODUCTNUM=


REM ----------------------------------------------------------------------
REM  Check for arguments
REM ----------------------------------------------------------------------

set B_PAUSE=pause
set B_GUI=1

:NextArg
if "%1" == "no_pause" (set B_PAUSE=) && shift && goto NextArg
if "%1" == "/nopause" (set B_PAUSE=) && shift && goto NextArg
if "%1" == "/nogui" (set B_GUI=0) && shift && goto NextArg


REM ----------------------------------------------------------------------
REM  Validate settings
REM ----------------------------------------------------------------------

if "%P_DCLROOT%" == "" goto BadDTRoot
if "%P_ROOT%" == "" goto BadRoot
if "%P_PROJDIR%" == "" goto BadProj
if "%P_OS%" == "" goto BadOS
if "%P_CPUTYPE%" == "" goto BadCPUType
if "%P_CPUFAMILY%" == "" goto BadCPUFam
if "%P_TOOLSET%" == "" goto BadToolSet

if not exist %P_DCLROOT%\nul goto BadDTRoot
if not exist %P_DCLROOT%\host\win32\%P_TOOLSET%\nul goto BadToolSet

if not exist %P_ROOT%\product\prodinfo.bat goto BadRoot
call %P_ROOT%\product\prodinfo.bat


REM ----------------------------------------------------------------------
REM Determine and/or validate the P_TOOLROOT setting.
REM ----------------------------------------------------------------------
if not "%P_TOOLROOT%" == "" if exist "%P_TOOLROOT%\*" goto GotToolRoot
if not "%P_TOOLROOT%" == "" goto BadToolRoot

if not exist %P_DCLROOT%\host\win32\%P_TOOLSET%\detect.bat goto BadToolRoot
call %P_DCLROOT%\host\win32\%P_TOOLSET%\detect.bat %P_OS%
if "%P_TOOLROOT%" ==  "" goto BadToolRoot
if not exist "%P_TOOLROOT%\*" goto BadToolRoot

:GotToolRoot


REM ----------------------------------------------------------------------
REM Validate the DEBUG level.  If not specified, use the existing
REM value if any, else default to a level of 0.  DEBUG=0 means this
REM is a RELEASE build.
REM ----------------------------------------------------------------------

if "%1" == ""  goto DefLevel
if "%1" == "0" goto SetLevel
if "%1" == "1" goto SetLevel
if "%1" == "2" goto SetLevel
if "%1" == "3" goto SetLevel
goto BadLevel

:SetLevel
set B_DEBUG=%1
shift

:DefLevel
if "%B_DEBUG%" == "" set B_DEBUG=0

set B_BLDOPTS=%1 %2 %3 %4 %5 %6 %7 %8 %9


REM ----------------------------------------------------------------------
REM Validate the P_PROJDIR setting
REM ----------------------------------------------------------------------

if exist testdir.tmp del testdir.tmp
echo test dir >%P_PROJDIR%\testdir.tmp
if not exist testdir.tmp goto BadProjDir
del testdir.tmp


REM ----------------------------------------------------------------------
REM Validate the CPU for this ToolSet
REM ----------------------------------------------------------------------

REM Save the original value
set OLDFAM=%P_CPUFAMILY%

call %P_DCLROOT%\host\win32\%P_TOOLSET%\cpucheck.bat %P_CPUTYPE%

REM Restore the original value
set NEWFAM=%P_CPUFAMILY%
set P_CPUFAMILY=%OLDFAM%
set OLDFAM=

if "%NEWFAM%" == "" goto BadCPUMatch
if not "%NEWFAM%" == "%P_CPUFAMILY%" goto BadCPUMatch

set NEWFAM=


REM ----------------------------------------------------------------------
REM Set the output log file to use, and delete any previously
REM existing file.
REM ----------------------------------------------------------------------

set B_LOG=%P_PROJDIR%\build%B_DEBUG%.log

if exist %B_LOG% del %B_LOG%


REM ----------------------------------------------------------------------
REM Safety check -- see if we ought to clean first
REM ----------------------------------------------------------------------

if exist %P_ROOT%\lastproj.bat goto RunLastProj

REM If lastproj.bat does not exist, the project should already be clean.
REM If it appears not to be, clean it anyway...
if exist %P_DCLROOT%\common\clib\*.ob? goto CleanProject
if exist %P_ROOT%\common\tools\*.ob? goto CleanProject
goto GenLastProj

:RunLastProj
call %P_ROOT%\lastproj.bat
if "%LASTPROJ%" == "%P_PROJDIR%-%P_TOOLSET%-%P_CPUTYPE%-%B_CFLAGS%" goto GenLastProj

REM At least one of the above characteristics of the project
REM has changed, so clean first...

:CleanProject
echo.
echo Automatically cleaning the %P_PRODUCT% installation...
echo.
call %P_DCLROOT%\host\win32\bin\clean

:GenLastProj
echo set LASTPROJ=%P_PROJDIR%-%P_TOOLSET%-%P_CPUTYPE%-%B_CFLAGS%>%P_ROOT%\lastproj.bat


REM ----------------------------------------------------------------------
REM Display the product version
REM ----------------------------------------------------------------------

echo.
echo Datalight %P_PRODUCT%, %P_PRODDESC%
echo %P_VERSION% Build %P_BUILDNUM% dated %P_BUILDDATE%


REM ----------------------------------------------------------------------
REM Validate and initialize the OS Layer to use
REM ----------------------------------------------------------------------

echo.
echo Initializing OS build info for "%P_OS%"

if not exist %P_DCLROOT%\host\win32\os\%P_OS%.bat goto BadOSLayer

set B_ERROR=
call %P_DCLROOT%\host\win32\os\%P_OS%.bat
if not "%B_ERROR%" == "" goto GenError


REM ----------------------------------------------------------------------
REM Validate and initialize the ToolSet to use
REM ----------------------------------------------------------------------

echo Initializing ToolSet info for "%P_TOOLSET%"

set B_TSDIR=%P_DCLROOT%\host\win32\%P_TOOLSET%

if not exist %B_TSDIR%\toolinit.bat goto BadTools
if not exist %B_TSDIR%\toolset.* goto BadTools

set B_ERROR=
call %B_TSDIR%\toolinit.bat
if not "%B_ERROR%" == "" goto GenError


REM ----------------------------------------------------------------------
REM Determine the CPU libraries to build.  The B_CPUCODE variable is
REM used by dcl.mak to determine which CPU specific libraries (assembly
REM language code) should be included in the standard list of libraries
REM with which to link.
REM ----------------------------------------------------------------------

REM If no assember is defined, or if there is no "cpubase"
REM directory, then skip the CPU tree entirely.
REM
set B_CPUCODE=0
if "%B_ASM%" == "" goto CPUDone
if not exist %P_DCLROOT%\cpu\%P_CPUFAMILY%\cpubase\cpubase.mak goto CPUDone

set B_CPUCODE=1

REM If the CPU "type" and "family" are identical, then skip the
REM "cputype" directory.
REM
if "%P_CPUFAMILY%" == "%P_CPUTYPE%" goto CPUDone

REM If the "cputype" tree and makefile exist, set B_CPUCODE to indicate
REM that both the "cpubase" and "cputype" directories are to be built.
REM
if exist %P_DCLROOT%\cpu\%P_CPUFAMILY%\%P_CPUTYPE%\%P_CPUTYPE%.mak set B_CPUCODE=2

:CPUDone


REM ----------------------------------------------------------------------
REM Set object code filename extensions and target directories
REM ----------------------------------------------------------------------

REM Set defaults to RELEASE settings
set B_OBJEXT=obr
set B_RELDIR=%P_PROJDIR%\%P_CPUTYPE%\release

if "%B_DEBUG%" == "0" goto MakeTree

REM Set DEBUG settings
set B_OBJEXT=obd
set B_RELDIR=%P_PROJDIR%\%P_CPUTYPE%\debug

:MakeTree

if exist %B_RELDIR%\lib\*.* goto SkipTree

if not exist %P_PROJDIR%\%P_CPUTYPE%\nul md %P_PROJDIR%\%P_CPUTYPE%
if not exist %B_RELDIR%\nul md %B_RELDIR%
if not exist %B_RELDIR%\lib\nul md %B_RELDIR%\lib

:SkipTree


REM ----------------------------------------------------------------------
REM Choose the right ToolSet components
REM ----------------------------------------------------------------------

REM Set to use the default ToolSet components located in host\win32\bin
set B_BLDCMD=%P_DCLROOT%\host\win32\bin\bldlayer.bat
set B_BUILDLIB=%P_DCLROOT%\host\win32\bin\makelib.bat
set B_ADDLIBS=%P_DCLROOT%\host\win32\bin\addlibs.bat
set B_TRAVERSE=%P_DCLROOT%\host\win32\bin\traverse.bat

REM However, if ToolSet specific components exist, use them instead
if exist %B_TSDIR%\bldlayer.bat set B_BLDCMD=%B_TSDIR%\bldlayer.bat
if exist %B_TSDIR%\makelib.bat  set B_BUILDLIB=%B_TSDIR%\makelib.bat
if exist %B_TSDIR%\addlibs.bat  set B_ADDLIBS=%B_TSDIR%\addlibs.bat
if exist %B_TSDIR%\traverse.bat set B_TRAVERSE=%B_TSDIR%\traverse.bat

set B_TSDIR=


REM ----------------------------------------------------------------------
REM Inform the user
REM ----------------------------------------------------------------------

echo.
echo About to build %P_PRODUCT% %P_VERSION% using the following settings:
echo.
echo Project Settings:
echo   DL Common Libs (P_DCLROOT):    %P_DCLROOT%
echo     Product Root (P_ROOT):       %P_ROOT%
echo     Project Root (P_PROJDIR):    %P_PROJDIR%
echo        Target OS (P_OS):         %P_OS%
echo          OS Root (P_OSROOT):     %P_OSROOT%
echo       OS Version (P_OSVER):      %P_OSVER%
echo       CPU Family (P_CPUFAMILY):  %P_CPUFAMILY%
echo         CPU Type (P_CPUTYPE):    %P_CPUTYPE%
echo         Tool Set (P_TOOLSET):    %P_TOOLSET%
echo        Tool Root (P_TOOLROOT):   %P_TOOLROOT%
echo.
echo Build Settings:
echo      Debug Level (B_DEBUG):      %B_DEBUG%
echo       Byte Order (B_BYTEORDER):  %B_BYTEORDER%
echo   Global C Flags (B_CFLAGS):     %B_CFLAGS%
echo  Extra Libraries (B_DRIVERLIBS): %B_DRIVERLIBS%
echo.
%B_PAUSE%


REM ----------------------------------------------------------------------
REM Build the product
REM ----------------------------------------------------------------------

echo.
echo Building %P_PRODUCT%...
echo.

if not "%P_TOOLSET%" == "mswcepb" echo Building %P_PRODUCT%...>%B_LOG%

set B_ERROR=
call %B_TRAVERSE% . %P_TREES%

cd %P_PROJDIR%


REM ----------------------------------------------------------------------
REM Display results
REM ----------------------------------------------------------------------

REM Create IGNORE.TMP, which is a concatenation of the DCL, product, and 
REM ToolSet IGNORE.TXT files.  Note that if the product being built is 
REM DCL only, the data will get duplicated, but that is no big deal.
REM
copy %P_DCLROOT%\product\ignore.txt %P_PROJDIR%\ignore.tmp >nul
if exist %P_ROOT%\product\ignore.txt                   copy /A %P_PROJDIR%\ignore.tmp + %P_ROOT%\product\ignore.txt                   %P_PROJDIR%\ignore.tmp >nul
if exist %P_DCLROOT%\host\win32\%P_TOOLSET%\ignore.txt copy /A %P_PROJDIR%\ignore.tmp + %P_DCLROOT%\host\win32\%P_TOOLSET%\ignore.txt %P_PROJDIR%\ignore.tmp >nul

%P_DCLROOT%\host\win32\bin\showerr %B_LOG% /Q %B_TOOLERRORS% %B_TOOLWARNINGS% %P_PROJDIR%\ignore.tmp >error%B_DEBUG%.log
if not errorlevel 1 goto PostBuild
if errorlevel 250 if "%B_ERROR%" == "" set B_ERROR=MAKEALL_BuildWarnings

if "%B_PAUSE%" == "" (
  if "%B_GUI%" == "1" (
    start %P_DCLROOT%\host\win32\bin\view /sound:3 /title:"%P_PRODUCT% build errors! -- Viewing error%B_DEBUG%.log" error%B_DEBUG%.log
  ) else (
    type error%B_DEBUG%.log
  )
) else (
  if "%B_GUI%" == "1" (
    echo.
    echo ---[ Viewing the build errors.  Close the viewer window to continue... ]---
    echo.
    start /wait %P_DCLROOT%\host\win32\bin\view /sound:3 /top /title:"%P_PRODUCT% build errors! -- Viewing error%B_DEBUG%.log" error%B_DEBUG%.log
  ) else (
    more error%B_DEBUG%.log
    echo.
    pause
  )
)

goto BadBuild


REM ----------------------------------------------------------------------
REM Do any post-build processing we might need
REM ----------------------------------------------------------------------

:PostBuild

if "%P_TOOLSET%" == "mswcepb" goto Fini

echo.
echo -----{ %P_PRODUCT% built successfully }-----
echo.
echo Results can be found in %B_RELDIR%.
echo.
set B_ERROR=

REM ---------------------------------------------
REM Run prjcopy.bat if it exists in the project
REM ---------------------------------------------

REM Note that prjcopy.bat is designed to honor the B_PAUSE
REM environment variable if it exists, so there is no need
REM to pass a /nopause switch to prjcopy.
REM
if exist %P_PROJDIR%\prjcopy.bat call %P_PROJDIR%\prjcopy.bat
goto Fini


REM ----------------------------------------------------------------------
REM Error handling
REM ----------------------------------------------------------------------

:BadBuild
if "%B_ERROR%" == "" set B_ERROR=MAKEALL_BuildFailed
echo.
echo.
echo -----{ %P_PRODUCT% build failed ("%B_ERROR%") }-----
echo.
echo See BUILD%B_DEBUG%.LOG and ERROR%B_DEBUG%.LOG for details.
echo.
goto Fini

:BadLevel
echo makeall.bat: The debug level parameter must be 0-3
set B_ERROR=MAKEALL_DebugLevel
goto Fini

:BadTools
echo makeall.bat: The ToolSet %P_TOOLSET% is missing or incomplete
set B_ERROR=MAKEALL_ToolSetBad
goto Fini

:GenError
echo makeall.bat: Failed due to "%B_ERROR%"
goto Fini

:BadDTRoot
echo makeall.bat: P_DCLROOT is not valid
set B_ERROR=MAKEALL_DclRoot
goto Fini

:BadRoot
echo makeall.bat: P_ROOT is not valid
set B_ERROR=MAKEALL_ProductRoot
goto Fini

:BadProj
echo makeall.bat: P_PROJDIR is not valid
set B_ERROR=MAKEALL_ProjectDir
goto Fini

:BadProjDir
echo.
echo makeall.bat: The P_PROJDIR setting "%P_PROJDIR%"
echo              does not match the directory from which prjbuild.bat was run.
echo.
set B_ERROR=MAKEALL_CurrentDir
goto fini

:BadOS
echo makeall.bat: P_OS is not valid
set B_ERROR=MAKEALL_OS
goto Fini

:BadOSLayer
echo makeall.bat: The OS Layer is incomplete or invalid
set B_ERROR=MAKEALL_OSLayer
goto Fini

:BadCPUMatch
echo makeall.bat: The combination of P_CPUFAMILY and P_CPUTYPE is not
echo              valid for this ToolSet.
set B_ERROR=MAKEALL_CPUMatch
goto Fini

:BadCPUType
echo makeall.bat: P_CPUTYPE is not valid
set B_ERROR=MAKEALL_CPUType
goto Fini

:BadCPUFam
echo makeall.bat: P_CPUFAMILY is not valid
set B_ERROR=MAKEALL_CPUFamily
goto Fini

:BadToolSet
echo makeall.bat: P_TOOLSET is not valid
set B_ERROR=MAKEALL_ToolSet
goto Fini

:BadToolRoot
echo makeall.bat: P_TOOLROOT for the ToolSet "%P_TOOLSET%" and OS "%P_OS%"
echo              is not valid, or could not be automatically determined.
echo              P_TOOLROOT = "%P_TOOLROOT%"
set B_ERROR=MAKEALL_ToolRoot
goto Fini

:Syntax
echo.
echo makeall.bat is called by prjbuild.bat to compile the entire %P_PRODUCT%
echo source tree.
echo.
echo Syntax:  MAKEALL [/nopause] [/nogui] Debug_Level
echo.
echo Where:
echo     /nopause - Prevent pauses during the build when errors occur.
echo       /nogui - Dislpay errors as console text rather than in a GUI window. 
echo  Debug_Level - The DEBUG level to use, which must be a value from 0 to 3.
echo.
echo On exit, the B_ERROR environment variable will be clear if the build
echo was successful, or set to an error message if it was not.  If the
echo value is "MAKEALL_BuildWarnings", then the build was fully successful
echo except for the fact that the code generated warnings (however if the
echo ToolSet is configured to treat warnings as errors, then not all the
echo code will have been built).

:Fini
set B_GUI=
