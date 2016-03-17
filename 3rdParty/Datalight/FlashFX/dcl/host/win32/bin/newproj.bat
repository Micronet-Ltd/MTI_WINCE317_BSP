@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to create a new project using the specified OS,
REM CPU, and ToolSet.
REM
REM ToDo:
REM   -	The CPU setting should really be processed after the ToolSet setting
REM	since the selection of CPUs depends on the ToolSet.  To do it properly
REM	the command line order ought to be changed so that CPU is specified
REM	after the ToolSet.
REM   - If the P_OSVER variable exists prior to the execution of this batch
REM     file, it will be placed in the prjinfo.bat file indiscriminately --
REM     even if it is completely wrong.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: newproj.bat $
REM Revision 1.11  2011/01/13 02:32:40Z  garyp
REM Updated to set ERROR and ERRORLEVEL.  Updated to allow quotes
REM in the ToolRoot value.  Now initialize P_OSVER to null.
REM Revision 1.10  2009/02/08 00:19:44Z  garyp
REM Merged from the v4.0 branch.  Modified to initialize B_DRIVERLIBS and
REM B_CFLAGS settings.
REM Revision 1.9  2007/10/11 21:59:42Z  Garyp
REM Fixed the other half of the RECURSE quietly issue addressed in rev 1.6.
REM Revision 1.8  2007/10/09 02:14:12Z  Garyp
REM Improved error handling.
REM Revision 1.7  2007/10/05 22:10:02Z  Garyp
REM Updated so that NEWPROJ can be used to create a DCL project.
REM Revision 1.6  2007/04/06 17:48:11Z  Garyp
REM Updated RECURSE options to be silent about exit codes.
REM Revision 1.5  2006/11/08 00:39:04Z  Garyp
REM Enhanced the mechanism by which ToolSet information is displayed.
REM Revision 1.4  2006/08/14 00:08:06Z  Garyp
REM Updated so that if a prjinfo.bat exists in the dcl\os\osname\newproj
REM directory when a new project is created, it will be treated as a suffix
REM to the standard list of environment variable generated on the fly by
REM newproj.bat.
REM Revision 1.3  2006/02/03 01:00:18Z  brandont
REM Changed generation of projinfo.bat to not explicitly destroy the
REM P_OSVER environment variable if it was already set.
REM Revision 1.2  2006/01/19 01:33:03Z  Garyp
REM Fixed some broken parameter validation logic.
REM Revision 1.1  2005/12/01 01:56:58Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 01:56:58Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.8  2005/11/09 17:04:08Z  Pauli
REM Changed PAUSE to B_PAUSE for consistency with the build process.
REM Revision 1.7  2005/11/08 20:31:23Z  Pauli
REM Added default values for P_OSROOT and P_OSVER.
REM Revision 1.6  2005/05/19 20:21:47Z  garyp
REM Improved error handling.
REM Revision 1.5  2005/04/24 18:29:22Z  GaryP
REM Updated to skip the CPU tree check if the product is Reliance.
REM Revision 1.4  2005/03/18 21:53:54Z  GaryP
REM Fixed a syntax problem.  Changed to not call the OS init batch file prior
REM to calling TOOLINIT.BAT.
REM Revision 1.3  2005/03/17 18:28:38Z  GaryP
REM Updated to call the OS init batch file prior to calling TOOLINIT.BAT.  Use
REM a /newproj parameter for the TOOLINIT call, so it does not do checks that
REM would not be applicable until buld time.
REM Revision 1.2  2005/03/14 23:34:11Z  PaulI
REM Added a "nopause" option.  This allows for automated project creation.
REM Revision 1.1  2005/01/18 07:41:54Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

REM Init to something so our error messages work
set P_PRODUCT=product

if "%1" == "" goto help
if "%1" == "?" goto help
if "%1" == "/?" goto help
if "%1" == "-?" goto help
if "%1" == "help" goto help

set B_PAUSE=pause
if /I "%1" == "/nopause" shift && set B_PAUSE=


REM ------------------------------------------------------------------
REM     Parameter Checks and Initialization
REM ------------------------------------------------------------------

if "%1" == "" goto BadRoot
if not exist %1\product\prodinfo.bat goto BadRoot

REM If there is a "dcl" subtree within the product tree,
REM set P_DCLROOT to that tree regardless of what it may
REM have been previously.  Handle the case where it is DCL
REM itself for which a project is being created.

set P_DCLROOT=
if exist %1\dcl\product\*.* set P_DCLROOT=%1\dcl
if "%P_DCLROOT%" == "" if exist %1\host\win32\bin\*.* set P_DCLROOT=%1
if "%P_DCLROOT%" == "" goto BadDCLRoot

set P_ROOT=%1

call %1\product\prodinfo.bat

if "%2" == "" goto BadProjDir
if "%3" == "" goto BadOS

if not exist %1\os\%3\newproj\*.* goto BadOS
if not exist %P_DCLROOT%\host\win32\os\%3.bat goto BadOS


REM ------------------------------------------------------------------
REM     Validate and create the Project Directory
REM ------------------------------------------------------------------

REM ToDo:
REM Our build process requires that projects reside on the same drive
REM as the product, but not necessarily in the same tree.  We should
REM validate that the specified project is on the same drive here...

if exist %2\*.* goto Already

md %2

REM Make sure we really did create the directory
echo test dir >%2\testdir.tmp

REM Doing these CD commands helps to ensure that the user really did
REM specify a full path to the Project Directory.
cd %P_DCLROOT%\host\win32
cd %2
if exist %2\testdir.tmp goto GoodDir

REM A bad path was specified.  Try to remove the temp files, however
REM we really don't know where we originated from so guess that we
REM were in the product root and try to remove them from there...
cd %1 >nul
if exist %2\testdir.tmp del %2\testdir.tmp
if not exist %2\*.* rd %2
goto BadDir

:GoodDir
del %2\testdir.tmp

REM Change into the Project Directory
cd %2


REM ------------------------------------------------------------------
REM     Validate the ToolSet
REM ------------------------------------------------------------------

if not exist %P_DCLROOT%\host\win32\%5\toolinit.bat goto BadToolSet

set P_TOOLROOT=%~6
if not "%~6" == "" if exist "%~6\*.*" goto GotToolSet

REM Try to automatically determine the ToolRoot if we can

set P_TOOLROOT=
if exist %P_DCLROOT%\host\win32\%5\detect.bat call %P_DCLROOT%\host\win32\%5\detect.bat %3
if "%P_TOOLROOT%" == "" goto BadToolRoot

:GotToolSet


REM ------------------------------------------------------------------
REM     Validate the CPU, and determine the CPU family
REM ------------------------------------------------------------------

REM NOTE: All P_CPUTYPE and P_CPUFAMILY settings must be lowercase

call %P_DCLROOT%\host\win32\%5\cpucheck.bat %4

if "%P_CPUFAMILY%" == "" goto BadCPU

if "%P_CPUFAMILY%" == "auto" goto SkipCheck

if "%P_PRODUCT%" == "Reliance" goto SkipCheck

if not exist %P_DCLROOT%\cpu\*.* goto SkipCheck
if exist %P_DCLROOT%\cpu\%P_CPUFAMILY%\cpubase\*.* goto SkipCheck
echo.
echo NEWPROJ: The CPU family code tree for "%P_CPUFAMILY%" does not
echo          exist.  The product will link correctly without this
echo          code only if the "Cpu" function interface is not used.
echo.
%B_PAUSE%

:SkipCheck


REM ------------------------------------------------------------------
REM     Copy the default files into the Project Directory
REM ------------------------------------------------------------------

for %%a in (%P_TREES%) do if exist %1\%%a\os\%3\newproj\*.* copy %1\%%a\os\%3\newproj\*.* >nul
copy %P_DCLROOT%\host\win32\newproj\*.*                             >nul
copy %1\os\%3\newproj\*.*             				    >nul


REM ------------------------------------------------------------------
REM     Validate that this OS/CPU/TOOLSET combination is valid
REM ------------------------------------------------------------------

set P_ROOT=%1
set P_PROJDIR=%2
set P_OS=%3
set P_CPUTYPE=%4
set P_TOOLSET=%5

call %P_DCLROOT%\host\win32\%P_TOOLSET%\toolinit.bat /newproj
if not "%B_ERROR%" == "" goto BadCombo

if not "%4" == "auto" goto NotAuto
echo.
echo Defaulting to CPU family "%P_CPUFAMILY%" and CPU type "%P_CPUTYPE%".
echo.
:NotAuto


REM ------------------------------------------------------------------
REM     Build PRJINFO.BAT
REM ------------------------------------------------------------------

if exist prjinfo.bat ren prjinfo.bat prjinfo.tmp

echo >prjinfo.bat REM ---------- Product/Project Settings ----------
echo.>>prjinfo.bat
echo >>prjinfo.bat set P_ROOT=%1
echo >>prjinfo.bat set P_PROJDIR=%2
echo >>prjinfo.bat set P_OS=%3
echo >>prjinfo.bat set P_OSROOT=
echo >>prjinfo.bat set P_OSVER=
echo >>prjinfo.bat set P_CPUFAMILY=%P_CPUFAMILY%
echo >>prjinfo.bat set P_CPUTYPE=%P_CPUTYPE%
echo >>prjinfo.bat set P_TOOLSET=%5
echo >>prjinfo.bat set P_TOOLROOT=%P_TOOLROOT%
echo >>prjinfo.bat set P_PRIVATE=1
echo.>>prjinfo.bat
echo >>prjinfo.bat REM --------------- Build Settings ---------------
echo.>>prjinfo.bat
echo >>prjinfo.bat REM Define any product-wide C compiler flags to use
echo >>prjinfo.bat set B_CFLAGS=
echo.>>prjinfo.bat
echo >>prjinfo.bat REM Define any extra libraries required for linking
echo >>prjinfo.bat set B_DRIVERLIBS=

REM If a prjinfo.bat already existed, it was a suffix which is to
REM be appended to the newly generated prjinfo.bat -- do so now.

if exist prjinfo.tmp copy prjinfo.bat + prjinfo.tmp prjinfo.bat >nul
if exist prjinfo.tmp del prjinfo.tmp


REM ------------------------------------------------------------------
REM     Success!
REM ------------------------------------------------------------------

echo.
echo A new %P_PRODUCT% project for %3 was created in "%2"
echo.
echo The file prjinfo.bat contains the default project and build settings.
echo Use prjbuild.bat and prjclean.bat to build and clean the project.
echo.

set ERROR=
goto Fini


REM ------------------------------------------------------------------
REM     Error Handlers
REM ------------------------------------------------------------------

:BadRoot
set ERROR=NEWPROJ_BadRoot
echo.
echo NEWPROJ:  %P_PRODUCT% not found in "%1"
echo.
goto Fini

:BadDCLRoot
set ERROR=NEWPROJ_BadDCLRoot
echo.
echo NEWPROJ:  The product tree does not contain a DCL sub-tree.
echo.
goto Fini

:BadDir
set ERROR=NEWPROJ_BadDir
echo.
echo NEWPROJ:  The Project Directory "%2" cannot be created, is not a
echo           full path specification from the root, or is not on the
echo           same drive as the product tree.
echo.
goto Fini

:BadProjDir
set ERROR=NEWPROJ_BadProjDir
echo.
echo NEWPROJ:  A Project Directory must be specified.
echo.
goto Fini

:Already
set ERROR=NEWPROJ_BadProj
echo.
echo NEWPROJ:  The Project Directory already exists.  New project not created.
echo.
goto Fini

:BadOS
set ERROR=NEWPROJ_BadOS
echo.
echo NEWPROJ:  The OS "%3" is not valid.
echo.
echo Valid OS names are:
echo.
dir /b %1\os\*.
echo.
goto Fini

:BadToolSet
set ERROR=NEWPROJ_BadToolSet
echo.
echo NEWPROJ:  The ToolSet "%5" does not appear to be valid
echo.
echo The following ToolSet abstractions are available:
%P_DCLROOT%\host\win32\bin\recurse /n /q /s%P_DCLROOT%\host\win32 cmd /c if exist toolinit.bat call toolinit.bat /info
echo.
echo ToolSet names must be specified in all lowercase.
echo.
goto ErrCleanup

:BadToolRoot
set ERROR=NEWPROJ_BadToolRoot
echo.
echo NEWPROJ:  The ToolRoot "%~6" is not valid or could not be automatically
echo           determined.
echo.
goto ErrCleanup

:BadCPU
set ERROR=NEWPROJ_BadCPU
echo.
echo NEWPROJ:  The CPU "%4" does not appear to be valid for the given
echo           ToolSet.  CPU names must be specified in all lowercase.
echo.
goto ErrCleanup

:BadCombo
set ERROR=NEWPROJ_%B_ERROR%
echo.
echo NEWPROJ:  The specified OS/CPU/ToolSet combination is not valid.
echo.
echo           The error designation is "%B_ERROR%".
echo.
goto ErrCleanup

:ErrCleanup
cd %1
rd /s/q %2
goto Fini


REM ------------------------------------------------------------------
REM     Syntax
REM ------------------------------------------------------------------

:Help
echo.
echo NEWPROJ creates a new project using the specified characteristics.
echo.
echo Syntax:    NEWPROJ ProdRoot ProjDir OS CPU ToolSet [ToolRoot]
echo.
echo Where: ProdRoot - The root of the %P_PRODUCT% installation directory
echo         ProjDir - The name of the Project Directory to use.  This must be on
echo                   the same drive as the %P_PRODUCT% directory.  This must be a
echo                   fully qualified directory name that does not already exist.
echo              OS - The operating system to target
echo             CPU - The CPU to target.  "auto" is allowed for some ToolSets.
echo         ToolSet - The name of the ToolSet abstraction to use
echo      [ToolRoot] - The optional root of the development tools to use.  If
echo                   this value is not specified, an attempt will be made to
echo                   automatically determine the tool root.
echo.
echo Run this command with the OS, CPU, or ToolSet names blank to get a list of
echo supported options.
echo.
echo NOTE: The OS, CPU, and ToolSet values MUST be specified in lowercase.
echo.
echo Should an error occur, the ERRORLEVEL will be non-zero, and the ERROR variable
echo will be set.
echo.
if "%P_DCLROOT%" == "" if not exist dcl\host\win32\bin\*.* goto Fini
set RUUT=%P_DCLROOT%
if "%RUUT%" == "" set RUUT=dcl
echo The following ToolSet abstractions are available:
%RUUT%\host\win32\bin\recurse /n /q /s%RUUT%\host\win32 cmd /c if exist toolinit.bat call toolinit.bat /info
set RUUT=
echo.
set ERROR=NEWPROJ_Syntax
goto Fini


REM ------------------------------------------------------------------
REM     Cleanup
REM ------------------------------------------------------------------

:Fini
set B_PAUSE=

if "%ERROR%" == "" (exit /b 0) else (exit /b 1)

