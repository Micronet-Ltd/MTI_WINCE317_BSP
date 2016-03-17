@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to initialize a ToolSet.  The following variables
REM will be initialized:
REM
REM B_TOOLBIN  - The name of the bin directory relative to P_TOOLROOT
REM B_TOOLINC  - The name of the include directory relative to P_TOOLROOT
REM B_TOOLLIB  - The name of the lib directory relative to P_TOOLROOT
REM B_CC       - The C compiler to use
REM B_ASM      - The Assembler to use
REM B_LIB      - The Librarian to use
REM B_LIBEXT   - The filename extension to use for libraries
REM B_LINK     - The Linker to use
REM B_MAKE     - The make program to use (typically full path to GMAKEW32.EXE)
REM
REM If B_INPATH is set to any value, the CC, ASM, LIB, and LINK settings
REM will simply be the binary name.  If B_INPATH is null, these respective
REM settings will be the full path to the tools.
REM
REM This batch file may do other validity checks to ensure that the specified
REM prjinfo.bat settings are appropriate for this ToolSet.
REM
REM This batch file may initialize other ToolSet specific settings that are
REM necessary to build the project (such as things required in toolset.mak).
REM
REM If the /info command-line switch is used, the batch file must display a
REM simple line describing the tools this ToolSet supports.
REM
REM If the /newproj command-line switch is used, the OS initialization batch
REM file will not have been run, and the P_OSROOT and P_OSVER settings will
REM not be available.  Whenever the /newproj switch is used, toolinit.bat
REM should simply do a validity check to ensure that the CPU/OS/TOOLSET
REM combination is valid.
REM
REM On exit B_ERROR will be null on success, or set to an error description
REM on failure.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: toolinit.bat $
REM Revision 1.6  2009/04/16 00:54:03Z  garyp
REM Noted that this ToolSet is deprecated.
REM Revision 1.5  2006/12/07 23:08:09Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.4  2006/11/08 00:50:54Z  Garyp
REM Added support for MSVS8.
REM Revision 1.3  2006/05/14 05:02:00Z  Keithg
REM Added quotes to support paths with space(s)
REM Revision 1.2  2006/03/11 00:25:00Z  KeithG
REM Added quotes to support paths with spaces in the name.
REM Revision 1.1  2005/06/13 01:22:52Z  Pauli
REM Initial revision
REM Revision 1.4  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.3  2005/04/26 18:39:52Z  GaryP
REM Added support for the B_BYTEORDER setting.
REM Revision 1.2  2005/03/18 22:05:43Z  GaryP
REM Documentation update.
REM Revision 1.1  2005/01/11 19:20:36Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Initial Settings
REM -----------------------------------------------

REM Set the prefix for the DCL libraries
set B_DCLLIBPREFIX=

REM Set the filename extension to use for libraries
set B_LIBEXT=lib

if "%B_BYTEORDER%" == "" set B_BYTEORDER=Auto

set B_TOOLVER=VS6
if exist %P_TOOLROOT%\bin\rc.exe set B_TOOLVER=VS8

REM These two variables are used as parameters to SHOWERR.EXE in the event
REM that additional triggers must be used to parse the output log file to
REM find warnings and errors.  For ToolSets where the standard trigger
REM values are sufficient, these two variable may be left blank.
REM
set B_TOOLERRORS=
set B_TOOLWARNINGS=


REM -----------------------------------------------
REM     Determine the Build Type
REM -----------------------------------------------


REM -----------------------------------------------
REM     Validate the CPU
REM -----------------------------------------------

REM The MSVC toolset CPU support is simple -- long live the 386!

if "%P_CPUTYPE%" == "386" goto _386
goto BadCPUType

:_386
set B_CC=cl
set B_ASM=
goto SetPath


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPath
set B_TOOLBIN=bin
set B_TOOLINC=include
set B_TOOLLIB=lib

if not exist "%P_TOOLROOT%\%B_TOOLBIN%\*.*" goto BadToolRoot
if not exist "%P_TOOLROOT%\%B_TOOLLIB%\*.*" goto BadToolRoot


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

set B_LIB=lib
set B_LOC=
set B_LINK=link

REM If the tools are not "inpath" set to the full path

if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM="%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%"
if "%B_INPATH%" == "" set B_CC="%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%"
if "%B_INPATH%" == "" set B_LINK="%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%"
if "%B_INPATH%" == "" set B_LIB="%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%"

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "msvc6"      - Works with Microsoft Visual Studio 6, 7, and 8 (deprecated)
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized.
echo               The CPU name must be lowercase.
echo.
set B_ERROR=TOOLINIT_BadCPUType
goto Fini

:BadToolRoot
echo.
echo toolinit.bat: The ToolRoot "%P_TOOLROOT%" is not valid.
echo.
set B_ERROR=TOOLINIT_BadToolRoot
goto Fini


REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini

