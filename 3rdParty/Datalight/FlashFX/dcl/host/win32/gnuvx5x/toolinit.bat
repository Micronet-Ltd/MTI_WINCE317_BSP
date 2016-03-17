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
REM Revision 1.9  2009/04/03 20:10:43Z  thomd
REM Fix loader problem - bug 1652
REM Revision 1.8  2007/02/08 00:58:38Z  Garyp
REM Updated to better parse for errors during the build, since the compiler is
REM lame, and fixed so that "le" is appended to the library name for MIPS
REM targets (if necessary).
REM Revision 1.7  2006/12/07 23:08:10Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.6  2006/11/08 00:51:04Z  Garyp
REM Added support for the /Info option.
REM Revision 1.5  2006/08/30 21:20:42Z  Garyp
REM Reverted to using the Datalight supplied MAKE program which should be
REM feasible now with the new SHREDIR utility.
REM Revision 1.4  2006/01/23 18:07:42Z  pauli
REM Corrected mcpu and march switches for the pentium 1.
REM Revision 1.3  2006/01/19 18:07:06Z  pauli
REM Updated to set the correct compiler switches all pentium CPUs.
REM Revision 1.2  2005/12/20 01:10:49Z  tonyq
REM Updated to properly support Pentium2, Pentium3, and Pentium4 with GNU tools
REM Revision 1.1  2005/06/13 01:22:52Z  Pauli
REM Initial revision
REM Revision 1.4  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.3  2005/05/04 23:23:59Z  PaulI
REM Defaulted to use vxWorks 5.5 tool names for x86 target.
REM Revision 1.2  2005/05/04 20:52:56Z  garyp
REM Corrected the x86 target to differentiate betwee the tools for 5.4 and 5.5.
REM Revision 1.1  2005/04/26 20:42:22Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not "%P_OS%" == "vxworks" if not "%P_OS%" == "loader" goto BadOS
if not exist %P_TOOLROOT%\host\x86-win32\bin\*.* goto BadOSVer

if "%B_BYTEORDER%" == "" set B_BYTEORDER=Auto

REM These two variables are used as parameters to SHOWERR.EXE in the event
REM that additional triggers must be used to parse the output log file to
REM find warnings and errors.  For ToolSets where the standard trigger
REM values are sufficient, these two variables may be left blank.
REM
set B_TOOLERRORS=/E:".c:"
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
if "%P_CPUFAMILY%" == "mips" goto _mips
if "%P_CPUFAMILY%" == "ppc" goto _ppc
if "%P_CPUFAMILY%" == "simulator" goto _simulator
if "%P_CPUFAMILY%" == "x86" goto _x86
goto BadCPUFam

:_arm
set B_ASM=
set B_CC=ccarm
set B_LIB=ararm
set B_GNUCPU=
if "%P_CPUTYPE%" == "armarch5" goto _arm_arch5
if "%P_CPUTYPE%" == "xscale" goto _arm_xscale
if "%B_GNUCPU%" == "" goto BadCPUType

:_arm_arch5
set VXCPU=ARMARCH5
set B_GNUCPU=-DCPU=%VXCPU% -DCPU_926E -DARMEL
set B_GNUOPT=-DPRJ_BUILD -DARMMMU=ARMMMU_926E -DARMCACHE=ARMCACHE_926E -fvolatile
goto SetPaths

:_arm_xscale
set VXCPU=XSCALE
set B_GNUCPU=-DCPU=%VXCPU% -mcpu=xscale -DARMEL
set B_GNUOPT=-DPRJ_BUILD -mapcs-32 -mlittle-endian -DWRSBCPXA250 -DCPU_XSCALE -DARMMMU=ARMMMU_XSCALE -DARMCACHE=ARM -fvolatile
goto SetPaths

:_mips
set B_ASM=
set B_CC=ccmips
set B_LIB=armips

REM Validate byte order
REM
set T_ORDER=
if "%B_BYTEORDER%" == "big" set T_ORDER=EB
if "%B_BYTEORDER%" == "little" set T_ORDER=EL
if "%B_BYTEORDER%" == "little" set VXBO=le
if "%T_ORDER%" == "" goto BadByteOrder

REM Validate CPU type
REM
if "%P_CPUTYPE%" == "mips32" set VXCPU=MIPS32
if "%P_CPUTYPE%" == "mips64" set VXCPU=MIPS64
if "%VXCPU%" == "" goto BadCPUType

set B_GNUCPU=-DCPU=%VXCPU% -DMIPS%T_ORDER%
if "%P_CPUTYPE%" == "mips32" set B_GNUCPU=%B_GNUCPU% -mips2 -DSOFT_FLOAT -msoft-float
if "%P_CPUTYPE%" == "mips64" set B_GNUCPU=%B_GNUCPU% -mips3
set B_GNUOPT=-DPRJ_BUILD -mno-branch-likely -G 0 -%T_ORDER%

REM Add a suffix for MIPS32 only
if "%P_CPUTYPE%" == "mips32" set VXCPU=MIPS32sf
goto SetPaths

:_ppc
set B_ASM=
set B_CC=ccppc
set B_LIB=arppc
if "%P_CPUTYPE%" == "ppc403" set VXCPU=403
if "%P_CPUTYPE%" == "ppc405" set VXCPU=405
if "%P_CPUTYPE%" == "ppc440" set VXCPU=440
if "%P_CPUTYPE%" == "ppc603" set VXCPU=603
if "%P_CPUTYPE%" == "ppc604" set VXCPU=604
if "%P_CPUTYPE%" == "ppc860" set VXCPU=860
set B_GNUCPU=-DCPU=PPC%VXCPU% -mcpu=%VXCPU%
set VXCPU=PPC%VXCPU%
set B_GNUOPT=-mstrict-align -O2 -fstrength-reduce
goto SetPaths

:_simulator
REM We only support the NT simulator at this time
if not "%P_CPUTYPE%" == "simnt" goto BadCPUType
set B_ASM=
set B_CC=ccsimpc
set B_LIB=arsimpc
set VXCPU=SIMNT
set B_GNUCPU=-DCPU=%VXCPU% -mpentium
set B_GNUOPT=-fno-defer-pop
goto SetPaths

:_x86
if "%P_CPUTYPE%" == "pentium"  goto _pentium
if "%P_CPUTYPE%" == "pentium2" goto _pentium2
if "%P_CPUTYPE%" == "pentium3" goto _pentium3
if "%P_CPUTYPE%" == "pentium4" goto _pentium4
goto BadCPUType

:_pentium
set VXCPU=PENTIUM
set B_GNUCPU=-DCPU=PENTIUM -mcpu=pentium -march=pentium
goto _x86_done

:_pentium2
set VXCPU=PENTIUM2
set B_GNUCPU=-DCPU=PENTIUM2 -mcpu=pentiumpro -march=pentiumpro
goto _x86_done

:_pentium3
set VXCPU=PENTIUM3
set B_GNUCPU=-DCPU=PENTIUM3 -mcpu=pentiumiii -march=p3
goto _x86_done

:_pentium4
set VXCPU=PENTIUM4
REM In VxWorks, p4 REALLY is treated as p3.
set B_GNUCPU=-DCPU=PENTIUM4 -mcpu=pentiumiii -march=p3
goto _x86_done

:_x86_done

REM Tool names for VxWorks 5.5
set B_ASM=
set B_CC=ccpentium
set B_LIB=arpentium
REM Different tool names for VxWorks 5.4 (would be preferable to use
REM P_OSVER to make this distinction, however VXWORKS.BAT is currently
REM written to set P_OSVER equal to "5.x").
REM
if exist "%P_TOOLROOT%\host\x86-win32\bin\cc386.exe" set B_CC=cc386
if exist "%P_TOOLROOT%\host\x86-win32\bin\ar386.exe" set B_LIB=ar386
set B_GNUOPT=-nostdlib -fno-defer-pop
goto SetPaths


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPaths
if "%VXCPU%" == "" goto BadCPUType

REM if "%1" == "/newproj" goto Success

REM Set variables assuming VxWorks 5.4 through 5.5.1
REM
set B_TOOLBIN=host\x86-win32\bin
set B_TOOLLIB=host\x86-win32\lib
set B_TOOLINC=target\h
if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot

set B_VXLIBSUFFIX=%VXCPU%gnu%VXBO%


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

REM If the tools are not "inpath" set to the full path
REM
if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN%\%B_LINK%
if "%B_INPATH%" == "" set B_LIB=%P_TOOLROOT%\%B_TOOLBIN%\%B_LIB%

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
echo   "gnuvx5x"    - Works with the VxWorks GNU v5.x tools
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadOS
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks 5.x
echo.
set B_ERROR=TOOLINIT_BadOS
goto Fini

:BadOSVer
echo.
echo toolinit.bat: This ToolSet is specifically tailored for VxWorks
echo               version 5.x.
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


