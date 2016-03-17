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
REM Revision 1.2  2009/06/28 01:05:05Z  garyp
REM Updated to properly support the /newproj switch.
REM Revision 1.1  2008/07/26 19:14:52Z  garyp
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

REM The SDT toolset CPU support is simple...

if "%P_CPUTYPE%" == "xscale" goto _xscale
goto BadCPUType

:_xscale
set B_CC=ccxsc.exe
set B_ASM=asxsc.exe
set B_LIB=libxsc.exe
set B_LINK=ldxsc.exe
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


REM If the tools are not "inpath" set to the full path

if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM="%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%"
if "%B_INPATH%" == "" set B_CC="%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%"
if "%B_INPATH%" == "" set B_LINK="%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%"
if "%B_INPATH%" == "" set B_LIB="%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%"

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

if "%1" == "/newproj" goto Success

if not "%B_BYTEORDER%" == "little" if not "%B_BYTEORDER%" == "big" goto BadByteOrder

if "%B_BYTEORDER%" == "little" set B_END=-DDCL_BIG_ENDIAN=0
if "%B_BYTEORDER%" == "big" set B_END=-DDCL_BIG_ENDIAN=1

:Success
set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "marvellsdt" - Works with the Marvell SDT v2.1
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

:BadByteOrder
echo.
echo toolinit.bat: The B_BYTEORDER value "%B_BYTEORDER%" is not valid.
echo               It must be set to "big" or "little".
echo.
set B_ERROR=TOOLINIT_BadByteOrder
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

