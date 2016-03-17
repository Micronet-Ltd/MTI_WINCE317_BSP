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
REM Revision 1.5  2010/06/18 13:19:00Z  jimmb
REM Fixed label to correct message when running newproj
REM Revision 1.4  2007/10/12 00:24:20Z  Garyp
REM Minor information message fix.
REM Revision 1.3  2007/10/01 21:26:00Z  pauli
REM Enabled additional warnings and treat warnings as errors.
REM Revision 1.2  2007/07/23 23:38:03Z  pauli
REM Added new descriptive error message if the tool root is invalid.
REM Revision 1.1  2007/06/15 01:21:16Z  pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM
REM     This toolset is designed to be used with
REM     Nucleus and RTOS configurations.
REM -----------------------------------------------

if "%P_OS%" == "nucleus" goto GoodOS
if "%P_OS%" == "rtos" goto GoodOS
goto BadOS

:GoodOS


REM -----------------------------------------------
REM     Initial Settings
REM -----------------------------------------------

REM Set the prefix for the DCL libraries
set B_DCLLIBPREFIX=

REM Set the filename extension to use for libraries
if "%P_OS%" == "nucleus" set B_LIBEXT=lib
if "%P_OS%" == "rtos" set B_LIBEXT=a

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
REM     Validate the CPU
REM -----------------------------------------------

REM If the "auto" CPU was specified, default to "arm" and "arm9"
if not "%P_CPUFAMILY%" == "auto" goto NotAuto
set P_CPUFAMILY=arm
set P_CPUTYPE=arm9
:NotAuto

if "%P_CPUFAMILY%" == "arm" goto _arm
goto BadCPUFam

:_arm

set B_TOOLBIN=\bin
set B_ASM=
set B_CC=arm-none-eabi-gcc
set B_LIB=arm-none-eabi-ar
set B_LINK=arm-none-eabi-ld
set B_TOOLINC=\include
set B_TOOLLIB=\lib
set B_GNUCPU=
if "%P_CPUTYPE%" == "arm9" goto _arm_9
if "%P_CPUTYPE%" == "arm11" goto _arm_11
goto BadCPUType

:_arm_9
set B_GNUCPU=-mcpu=arm926ej-s
goto _arm_common

:_arm_11
set B_GNUCPU=-mcpu=arm1136jf-s
goto _arm_common


:_arm_common
set B_GNUOPT=-Wall -Werror -Wsign-compare -Wundef
REM -Wconversion -Wwrite-strings -Wcast-align -Wunreachable-code -Wextra -Wno-unused-parameter
if not "%B_DEBUG%"=="0" set B_GNUOPT=%B_GNUOPT% -O0 -gdwarf-2
if     "%B_DEBUG%"=="0" set B_GNUOPT=%B_GNUOPT% -Os -fno-strict-aliasing
set B_GNUOPT=%B_GNUOPT% -m%B_BYTEORDER%-endian


REM -----------------------------------------------
REM     Set Paths
REM -----------------------------------------------
:SetPaths

if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_LIB=%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot

set B_LOC=

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

if "%1" == "/newproj" goto Success

REM Set the endianness
REM
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
echo   "gnucs"      - Works with the Code Sourcery ARM GNU toolchain
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadOS
echo.
echo toolinit.bat: This ToolSet is specifically tailored for the Code Sourcery ARM GNU tools
echo.
set B_ERROR=TOOLINIT_BadOS
%B_PAUSE%
goto Fini

:BadToolRoot
echo.
echo toolinit.bat: The tools cannot be found at the specified tool root directory:
echo                   "%P_TOOLROOT%"
echo               For this toolset, the P_TOOLROOT should be set to the root
echo               directory of the Code Sourcery tool chain.  To auto detect set
echo               P_TOOLROOT to null in prjinfo.bat.
echo.
set B_ERROR=TOOLINIT_BadToolRoot
%B_PAUSE%
goto Fini

:BadToolVer
echo.
echo toolinit.bat: This ToolSet is specifically tailored for the Code Sourcery
echo               ARM GNU 3.4.2 Tools
echo.
set B_ERROR=TOOLINIT_BadToolVer
%B_PAUSE%
goto Fini

:BadCPUFam
echo.
echo toolinit.bat: The CPU Family "%P_CPUFAMILY%" is not recognized for gnucs.
echo.
set B_ERROR=TOOLINIT_BadCPUFamily
%B_PAUSE%
goto Fini

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized for gnucs.
echo.
set B_ERROR=TOOLINIT_BadCPUType
%B_PAUSE%
goto Fini

:BadByteOrder
echo.
echo toolinit.bat: The B_BYTEORDER value "%B_BYTEORDER%" is not valid.  It must
echo               be set to "big" or "little".
echo.
set B_ERROR=TOOLINIT_BadByteOrder
%B_PAUSE%
goto Fini

:Fini
