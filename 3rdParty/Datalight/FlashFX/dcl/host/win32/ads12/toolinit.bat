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
REM PRJINFO.BAT settings are appropriate for this ToolSet.
REM
REM This batch file may initialize other ToolSet specific settings that are
REM necessary to build the project (such as things required in TOOLSET.MAK).
REM
REM If the /info command-line switch is used, the batch file must display a
REM simple line describing the tools this ToolSet supports.
REM
REM If the /newproj command-line switch is used, the OS initialization batch
REM file will not have been run, and the P_OSROOT and P_OSVER settings will
REM not be available.  Whenever the /newproj switch is used, TOOLINIT.BAT
REM should simply do a validity check to ensure that the CPU/OS/TOOLSET
REM combination is valid.
REM
REM On exit B_ERROR will be null on success, or set to an error description
REM on failure.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: toolinit.bat $
REM Revision 1.3  2006/12/07 23:08:12Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.2  2006/11/08 00:53:03Z  Garyp
REM Added support for the /Info option.
REM Revision 1.1  2005/12/01 01:51:52Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 01:51:52Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.6  2005/11/09 17:04:09Z  Pauli
REM Changed PAUSE to B_PAUSE for consistency with the build process.
REM Revision 1.5  2005/11/09 16:39:16Z  Pauli
REM Updated to correctly adhere to the "/nopause" option.
REM Revision 1.4  2005/06/13 02:22:54Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.3  2005/04/26 18:39:54Z  GaryP
REM Added support for the B_BYTEORDER setting.
REM Revision 1.2  2005/03/18 22:05:45Z  GaryP
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
REM     Set Bin Dir
REM -----------------------------------------------

set B_TOOLBIN=bin


REM -----------------------------------------------
REM     Validate the CPU
REM -----------------------------------------------

REM If the "auto" CPU was specified, default to "arm" and "xscale"
if not "%P_CPUFAMILY%" == "auto" goto NotAuto
set P_CPUFAMILY=arm
set P_CPUTYPE=xscale
:NotAuto

set B_CC=armcc
set B_ASM=armasm
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%

if "%P_CPUTYPE%" == "xscale" goto _xscale
if "%P_CPUTYPE%" == "arm9" goto _arm9
if "%P_CPUTYPE%" == "arm926ejs" goto _arm926ejs
goto BadCPUType

:_xscale
set B_ADSCPU=
goto SetPath

:_arm9
set B_ADSCPU=-apcs -noswstackcheck -interwork -cpu 4T
goto SetPath

:_arm926ejs
set B_ADSCPU=-apcs -noswstackcheck -interwork -cpu ARM926EJ-S -fa
goto SetPath


REM -----------------------------------------------
REM     Set Lib and Inc Path
REM -----------------------------------------------

:SetPath
set B_TOOLINC=include
set B_TOOLLIB=lib\armlib

if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadCPUType


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

set B_LIB=armar
set B_LINK=armlink
if "%B_INPATH%" == "" set B_LIB=%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%
set B_LOC=

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

if "%1" == "/newproj" goto Success

REM Note that this setting is temporary to accomodate OSE's driver code.
if not "%B_BYTEORDER%" == "little" if not "%B_BYTEORDER%" == "big" goto BadByteOrder
if "%B_BYTEORDER%" == "little" set B_END=-DLITTLE_ENDIAN
if "%B_BYTEORDER%" == "big" set B_END=

:Success
set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "ads12"      - Works with the ARM Developer Suite v1.2
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized for ADS.
echo.
set B_ERROR=TOOLINIT_BAT_BadCPUType
%B_PAUSE%
goto Fini

:BadByteOrder
echo.
echo toolinit.bat: The B_BYTEORDER value "%B_BYTEORDER%" is not valid.  It
echo               must be set to "big" or "little".
echo.
set B_ERROR=TOOLINIT_BAT_BadByteOrder
%B_PAUSE%
goto Fini


REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini
