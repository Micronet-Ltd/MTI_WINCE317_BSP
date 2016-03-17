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
REM Revision 1.10  2011/03/12 22:38:39Z  garyp
REM Fixed some messages -- no functional changes.
REM Revision 1.9  2010/09/16 13:57:31Z  jimmb
REM Make sure this toolset does not use quotes around file name paths because
REM we are verifying that all file name paths are in the 8.3 or short version.
REM Revision 1.8  2010/09/13 17:48:41Z  jimmb
REM Updated the toolset to support RVDS version 4.0 and 4.1.
REM Revision 1.7  2006/12/07 23:08:07Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.6  2006/11/08 00:50:25Z  Garyp
REM Added support for the /Info option.
REM Revision 1.5  2006/11/01 03:09:19Z  Garyp
REM Changed the lib extension to .a.  Fixed the rest of the paths to work if
REM installed in a directory with spaces.
REM Revision 1.4  2006/10/21 18:28:46Z  Garyp
REM Fixed a syntax error.
REM Revision 1.3  2006/10/13 20:37:44Z  jeremys
REM Fixed a problem with the toolset residing in a path that contains spaces.
REM Revision 1.2  2006/10/08 23:48:12Z  Garyp
REM First working version of the RVDS30 ToolSet.
REM Revision 1.1  2006/10/07 01:10:32Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if /I "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Initial Settings
REM -----------------------------------------------

REM
set B_NOQUOTEDINCPATHS=1

REM Set the prefix for the DCL libraries
set B_DCLLIBPREFIX=

REM Set the filename extension to use for libraries
set B_LIBEXT=a

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

set B_TOOLBIN=rvct\programs\%ARMVER%\%ARMBUILD%\win_32-pentium
if not exist "%P_TOOLROOT%\%B_TOOLBIN%\*.exe" goto BadToolRoot


REM -----------------------------------------------
REM     Set the Tool Paths
REM -----------------------------------------------

set B_CC=armcc.exe
set B_ASM=armasm.exe
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%


REM -----------------------------------------------
REM     Validate the CPU
REM -----------------------------------------------

set B_RVDSCPU=--arm --cpu=%B_CPUFLAGS% --apcs /interwork
goto SetPath


REM -----------------------------------------------
REM     Set Lib and Inc Path
REM -----------------------------------------------



:SetPath
set B_TOOLINC=rvct\data\%ARMVER%\%ARMBUILD%\include\windows
set B_TOOLLIB=rvct\data\%ARMVER%\%ARMBUILD%\lib\armlib

if not exist "%P_TOOLROOT%\%B_TOOLLIB%\*.*" goto BadCPUType


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

set B_LIB=armar
set B_LINK=armlink
if "%B_INPATH%" == "" set B_LIB="%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%"
if "%B_INPATH%" == "" set B_LINK="%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%"
set B_LOC=

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

if "%1" == "/newproj" goto Success

if "%B_CPUFLAGS%" == "" goto BadCPUFlags

if not "%B_BYTEORDER%" == "little" if not "%B_BYTEORDER%" == "big" goto BadByteOrder

if "%B_BYTEORDER%" == "little" set B_END=-DDCL_BIG_ENDIAN=0
if "%B_BYTEORDER%" == "big" set B_END=-DDCL_BIG_ENDIAN=1

if "%B_BYTEORDER%" == "little" set B_RVDSCPU=%B_RVDSCPU% --littleend
if "%B_BYTEORDER%" == "big" set B_RVDSCPU=%B_RVDSCPU% --bigend

:Success
set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "rvds30"     - Works with the ARM RealView Developer's Suite 3.0, 4.0, and 4.1
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadToolRoot
echo.
echo toolinit.bat: The tool bin directory of "%P_TOOLROOT%\%B_TOOLBIN%"
echo               does not appear to be valid.
echo.
set B_ERROR=TOOLINIT_BadToolRoot
%B_PAUSE%
goto Fini

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized for RVDS.
echo.
set B_ERROR=TOOLINIT_BadCPUType
%B_PAUSE%
goto Fini

:BadByteOrder
echo.
echo toolinit.bat: The B_BYTEORDER value "%B_BYTEORDER%" is not valid.
echo               It must be set to "big" or "little".
echo.
set B_ERROR=TOOLINIT_BadByteOrder
%B_PAUSE%
goto Fini

:BadCPUFlags
echo.
echo toolinit.bat: The environment variable B_CPUFLAGS must be set to the
echo               appropriate "--cpu=" ARMCC compiler options for your
echo               target hardware.  Do not include the "--cpu=" portion
echo               in the B_CPUFLAGS value.
echo.
echo               See the RVDS documentation to determine the correct
echo               options to use.  Once the value is determined, add the
echo               "set B_CPUFLAGS=[value]" command to prjinfo.bat.
echo.
echo               For example, on an ARM7TDMI platform, B_CPUFLAGS should
echo               be set to "ARM7TDMI".  You may also specify extra ARMCC
echo               parameters in this environment variable.
echo.
set B_ERROR=TOOLINIT_BadCPUFlags
goto Fini


REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini
