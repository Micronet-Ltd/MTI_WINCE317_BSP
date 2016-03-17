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
REM Revision 1.3  2010/06/28 20:18:58Z  jimmb
REM Fixed syntax to correctly add the Pentium and Pentium4 CPU to
REM the Intel abstraction.
REM Revision 1.2  2010/06/28 16:54:58Z  jimmb
REM Added pentium and pentium4 to supported CPU's
REM Revision 1.1  2010/06/16 21:24:42Z  jimmb
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not "%P_OS%" == "vxworks" if not "%P_OS%" == "loader" goto BadOS
if not exist %P_TOOLROOT%\intel\*.* goto BadOSVer
if not exist %P_TOOLROOT%\gnu\*.* goto BadOSVer


if "%B_BYTEORDER%" == "" set B_BYTEORDER=Auto

REM These two variables are used as parameters to SHOWERR.EXE in the event
REM that additional triggers must be used to parse the output log file to
REM find warnings and errors.  For ToolSets where the standard trigger
REM values are sufficient, these two variable may be left blank.
REM
set B_TOOLERRORS=
set B_TOOLWARNINGS=


REM -----------------------------------------------
REM     Initial Settings
REM -----------------------------------------------

REM Set the prefix for the DCL libraries
set B_DCLLIBPREFIX=

REM Set the filename extension to use for libraries
set B_LIBEXT=a

REM Ensure this is not set from a previous run
set VXCPU=
set VXBO=


REM -----------------------------------------------
REM     Validate CPU and get settings
REM -----------------------------------------------

set B_LOC=
set B_LINK=link

if "%P_CPUFAMILY%" == "arm" goto _arm
if "%P_CPUFAMILY%" == "ppc" goto _ppc
if "%P_CPUFAMILY%" == "mips" goto _mips
if "%P_CPUFAMILY%" == "sh" goto _sh
if "%P_CPUFAMILY%" == "x86" goto _x86
if "%P_CPUFAMILY%" == "simulator" goto _simulator

goto BadCPUFam

:_arm
goto BadCPUType

:_mips
goto BadCPUType

:_ppc
goto BadCPUType

:_sh
goto BadCPUType

:_simulator
REM We only support the NT simulator at this time
if not "%P_CPUTYPE%" == "simnt" goto BadCPUType
set B_ASM=
set B_CC=icc
set B_LIB=arpentium
set VXCPU=SIMNT
set B_GNUCPU=-DCPU=%VXCPU% -march=i486 -mtune=i486
set B_GNUOPT=-fno-defer-pop

goto SetPaths

:_x86
if "%P_CPUTYPE%" == "pentium"  goto _pentium
if "%P_CPUTYPE%" == "pentium4" goto _pentium4

goto BadCPUType

:_pentium
set VXCPU=PENTIUM
set B_GNUCPU=-DCPU=PENTIUM -march=pentium

goto _x86_done

:_pentium4
set VXCPU=PENTIUM4
set B_GNUCPU=-DCPU=PENTIUM4 -march=pentium4

:_x86_done
REM Tool names for VxWorks 6.x
set B_ASM=
set B_CC=icc
set B_LIB=arpentium


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPaths
if "%VXCPU%" == "" goto BadCPUType

if "%1" == "/newproj" goto Success

set B_TOOLLIB=.
set B_TOOLINC=vxworks-%P_OSVER%\target\h

set B_TOOLBIN=
set B_TOOLBIN1=
if "%P_OSVER%" == "6.9" set B_TOOLBIN=intel\20100316-vxworks-6.9\x86-win32\bin\ia32
if "%P_OSVER%" == "6.9" set B_TOOLBIN1=gnu\4.3.3-vxworks-6.9\x86-win32\bin

if "%B_TOOLBIN%" == "" goto BadOSVer
if "%B_TOOLBIN1%" == "" goto BadOSVer

if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLBIN1%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot

set B_VXLIBSUFFIX=%VXCPU%icc%VXBO%


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

REM If the tools are not "inpath" set to the full path
REM
if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN1%\%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN1%\%B_LINK%
if "%B_INPATH%" == "" set B_LIB=%P_TOOLROOT%\%B_TOOLBIN1%\%B_LIB%

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

:Success
set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "iccvx6x"    - Works with the VxWorks Intel IA32 tools

goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadOS
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks 6.x
echo.
set B_ERROR=TOOLINIT_BadOS

goto Fini

:BadOSVer
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks
echo               versions 6.9.
echo.
echo               P_TOOLROOT is not valid, or this version of VxWorks
echo               is not supported.
echo.
set B_ERROR=TOOLINIT_BadOSVer

goto Fini

:BadCPUFam
echo.
echo toolinit.bat: The CPU "%P_CPUFAMILY%" is not implemented for this ToolSet.
echo.
set B_ERROR=TOOLINIT_BadCPUFamily

goto Fini

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized for this ToolSet.
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

:BadByteOrder
echo.
echo toolinit.bat: The byte order setting B_BYTEORDER must be set
echo               to "big" or "little".
echo.
set B_ERROR=TOOLINIT_BadByteOrder

goto Fini


REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini
set T_ORDER=
set VXCPU=
set VXBO=

