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
REM Revision 1.6  2009/04/08 21:18:07Z  thomd
REM Fix loader problem - bug 1652
REM Revision 1.5  2006/12/07 23:08:11Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.4  2006/11/08 00:51:23Z  Garyp
REM Added support for the /Info option.
REM Revision 1.3  2006/08/30 21:20:43Z  Garyp
REM Reverted to using the Datalight supplied MAKE program which should be
REM feasible now with the new SHREDIR utility.
REM Revision 1.2  2006/08/02 22:40:51Z  Garyp
REM Updated to properly initialize the VxWorks version portion of the DIAB
REM CPU flags.
REM Revision 1.1  2005/06/13 01:22:54Z  Pauli
REM Initial revision
REM Revision 1.2  2005/06/13 02:22:54Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.1  2005/04/26 18:39:54Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not "%P_OS%" == "vxworks" if not "%P_OS%" == "loader" goto BadOS
if not "%P_OSVER%" == "5.x" goto BadOSVer

if not exist %P_TOOLROOT%\host\diab\win32\bin\*.* goto BadOSVer

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

REM WARNING: The following VXVER setting may be incorrect for VxWorks 5.4!
set VXVER=
if "%P_OSVER%" == "5.x" set VXVER=55
if "%VXVER%" == "" goto BadOSVer


REM -----------------------------------------------
REM     Validate CPU and get settings
REM -----------------------------------------------

if "%P_CPUFAMILY%" == "arm" goto _arm
if "%P_CPUFAMILY%" == "mips" goto _mips
if "%P_CPUFAMILY%" == "ppc" goto _ppc
if "%P_CPUFAMILY%" == "simulator" goto _simulator
if "%P_CPUFAMILY%" == "x86" goto _x86
goto BadCPUFam

:_arm
if not "%P_CPUTYPE%" == "armarch5" goto BadCPUType
set VXCPU=ARMARCH5
set B_DIABCPU=-DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER% -DCPU_926E
set B_DIABOPTS1=-W:c:,-Xclib-optim-off -Xansi -Xlocal-data-area-static-only
set B_DIABOPTS2=-Xsize-opt -Wa,-Xlit-auto-off
set B_DIABOPTS3=-DARMMMU=ARMMMU_926E -DARMCACHE=ARMCACHE_926E -Xpointers-volatile
goto SetPaths

:_mips
REM Validate byte order
REM
set T_ORDER=
if "%B_BYTEORDER%" == "little" set T_ORDER=EL
if "%B_BYTEORDER%" == "big" set T_ORDER=EB
if "%T_ORDER%" == "" goto BadByteOrder

REM Validate CPU type
REM
if "%P_CPUTYPE%" == "mips32" set VXCPU=MIPS32
if "%P_CPUTYPE%" == "mips64" set VXCPU=MIPS64
if "%VXCPU%" == "" goto BadCPUType

set B_DIABCPU=-DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER% -DMIPS%T_ORDER%
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths

:_ppc
if "%P_CPUTYPE%" == "ppc403" set VXCPU=PPC403
if "%P_CPUTYPE%" == "ppc405" set VXCPU=PPC405
if "%P_CPUTYPE%" == "ppc440" set VXCPU=PPC440
if "%P_CPUTYPE%" == "ppc603" set VXCPU=PPC603
if "%P_CPUTYPE%" == "ppc604" set VXCPU=PPC604
if "%P_CPUTYPE%" == "ppc860" set VXCPU=PPC860
if "%P_CPUTYPE%" == "ppc85xx" set VXCPU=PPC85XX
if "%VXCPU%" == "" goto BadCPUType
set B_DIABCPU=-DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER%
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths

:_simulator
if not "%P_CPUTYPE%" == "simnt" goto BadCPUType
set VXCPU=SIMNT
set DIABCPUFLAGS=-tX86LH
set B_DIABCPU=-DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER%
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths

:_x86
if "%P_CPUTYPE%" == "pentium"  set VXCPU=PENTIUM
if "%P_CPUTYPE%" == "pentium2" set VXCPU=PENTIUM2
if "%P_CPUTYPE%" == "pentium3" set VXCPU=PENTIUM3
if "%P_CPUTYPE%" == "pentium4" set VXCPU=PENTIUM4
if "%VXCPU%" == "" goto BadCPUType
set B_DIABCPU=-DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER%
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPaths
if "%VXCPU%" == "" goto BadCPUType
if "%DIABCPUFLAGS%" == "" goto BadCPUFlags

REM if "%1" == "/newproj" goto Success

REM TODO: Why do we set DIABLIB?  It is not used by TOOLSET.MAK.
REM       Is it a required setting for the tools themselves???

REM Set variables assuming VxWorks 5.4 through 5.5.1
REM
set B_TOOLBIN=host\diab\win32\bin
set B_TOOLLIB=host\diab\win32\lib
set B_TOOLINC=target\h
set DIABLIB=%P_TOOLROOT%\host\diab

if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot

set B_VXLIBSUFFIX=%VXCPU%diab


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

REM set B_ASM=das
set B_CC=dcc
set B_LIB=dar
set B_LOC=
set B_LINK=dld

REM If the tools are not "inpath" set to the full path
REM
if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_LIB=%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%

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
echo   "diabvx5x"   - Works with the VxWorks DIAB Tools v5.x
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadOS
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks
echo.
set B_ERROR=TOOLINIT_BadOS
goto Fini

:BadOSVer
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks version 5.x.
echo.
set B_ERROR=TOOLINIT_BadOSVer
goto Fini

:BadCPUFlags
echo.
echo toolinit.bat: The environment variable DIABCPUFLAGS must be set to the
echo               appropriate "-t" compiler options for your target hardware.
echo               See the DIAB documentation to determine the correct "-t"
echo               options to use.  Once the value is determined, add the
echo               "set DIABCPUFLAGS=[value]" command to prjinfo.bat.
echo.
echo               For example, on a PPC403 platform with ELF big-endian object
echo               code, software floating point, DIABCPUFLAGS should be set to
echo               "-tPPC403ES".
echo
echo               Do not include a trailing ":vxworks55" suffix, as this is
echo               automatically added by the build process.
echo.
set B_ERROR=TOOLINIT_BadCPUFlags
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
set DIABBASE=
set T_ORDER=
set VXCPU=
set VXVER=

