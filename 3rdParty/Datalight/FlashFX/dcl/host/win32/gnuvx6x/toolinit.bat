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
REM Revision 1.28  2010/06/14 20:05:07Z  jimmb
REM Reverted back to 1.26 bacuse of an incorrect update to this
REM toolset abstraction
REM Revision 1.26  2010/06/14 20:05:07Z  jimmb
REM Additional changes to support VxWorks 6.9
REM Revision 1.25  2010/06/11 14:00:31Z  jimmb
REM Syncronized diab and gnu 
REM Support for VxWorks 6.9
REM Revision 1.24  2010/06/09 16:29:03Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.9
REM Revision 1.23  2009/08/27 17:15:36Z  jimmb
REM Updated and added support for ppc32
REM Revision 1.22  2009/08/21 20:01:51Z  jimmb
REM Update Vxtools for correct _arm_cortexa8 vxtool type
REM Revision 1.21  2009/08/20 21:45:34Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.8
REM Revision 1.20  2009/06/13 18:30:22Z  jimmb
REM Added support for the Omap 3530
REM Revision 1.19  2009/04/06 18:13:43Z  thomd
REM Fix loader problem - bug 1652
REM Revision 1.18  2009/01/30 17:55:25Z  jimmb
REM Updating the build files for correct implementation of VxWorks 6.7
REM Revision 1.17  2008/08/25 19:44:21Z  jimmb
REM Updated for VxWorks 6.7
REM Revision 1.16  2008/08/06 20:42:29Z  jimmb
REM Updated the toolinit to reflect the deprecated options
REM Revision 1.15  2008/04/29 16:51:23Z  jimmb
REM Added support for the ARM v6 processor
REM Revision 1.14  2007/10/05 23:05:15Z  Garyp
REM Updated for VxWorks 6.6.
REM Revision 1.13  2007/08/29 21:52:21Z  jimmb
REM updated the BAT files to support the new VxWorks 6.6 and the GNU compiler
REM v4.1.2
REM Revision 1.12  2007/02/07 19:40:12Z  Garyp
REM Corrected tool paths for VxWorks 6.5.
REM Revision 1.11  2007/01/18 18:25:07Z  peterb
REM (Per gp) updated to work with VxWorks 6.5.
REM Revision 1.10  2006/12/07 23:08:09Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the DOS
REM extended version which was necessary to support Win9x builds.
REM Revision 1.9  2006/11/08 00:51:00Z  Garyp
REM Added support for the /Info option.
REM Revision 1.8  2006/08/31 20:58:53Z  Garyp
REM Corrected some error messages.
REM Revision 1.7  2006/08/28 02:01:28Z  Garyp
REM Added VxWorks 6.4 support.  Changed to use the MAKE supplied with the DL
REM kits, which is the same version supplied in all the VxWorks 6.x stuff so
REM far.
REM Revision 1.6  2006/05/26 19:34:08Z  tonyq
REM Updated with VxWorks 6.3 support
REM Revision 1.5  2006/01/23 18:07:28  pauli
REM Corrected mcpu and march switches for the pentium 1.
REM Revision 1.4  2006/01/20 23:18:10Z  brandont
REM Updated comment about VxWorks 6.2 support
REM Revision 1.3  2006/01/19 18:30:38Z  Pauli
REM Updated to set the compiler switches for all pentium CPUs to match those
REM used by WindRiver WorkBench.
REM Revision 1.2  2005/12/20 01:04:59Z  tonyq
REM Updated to properly support Pentium2, Pentium3, and Pentium4 with GNU tools
REM Revision 1.1  2005/12/04 00:23:14Z  Pauli
REM Initial revision
REM Revision 1.3  2005/12/04 00:23:14Z  Garyp
REM Removed the definition of the PRJ_BUILD symbol from some of the targets as
REM this was causing builds in 6.2 to fail.
REM Revision 1.2  2005/11/12 18:00:35Z  Garyp
REM Updated to support VxWorks 6.2.
REM Revision 1.1  2005/06/13 02:22:52Z  Garyp
REM Initial revision
REM Revision 1.3  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.2  2005/05/20 18:46:25Z  PaulI
REM Updated compiler flags for 64-bit MIPS processor.
REM Revision 1.1  2005/04/28 10:08:34Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not "%P_OS%" == "vxworks" if not "%P_OS%" == "loader" goto BadOS
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
set B_ASM=
set B_CC=ccarm
set B_LIB=ararm
if "%P_CPUTYPE%" == "armarch5" goto _arm_arch5
if "%P_CPUTYPE%" == "armarch6" goto _arm_arch6
if "%P_CPUTYPE%" == "cortexa8" goto _arm_cortexa8
if "%P_CPUTYPE%" == "xscale" goto _arm_xscale

goto BadCPUType

:_arm_arch5
set VXCPU=ARMARCH5
set B_GNUCPU=-DCPU=%VXCPU% -DCPU_926E -DARMEL
set B_GNUOPT=-DARMMMU=ARMMMU_926E -DARMCACHE=ARMCACHE_926E -fvolatile

goto SetPaths

:_arm_arch6
set VXCPU=ARMARCH6
set B_GNUCPU=-DCPU=%VXCPU% -DCPU_1136JF -DARMEL
set B_GNUOPT=-DARMMMU=ARMMMU_1136JF -DARMCACHE=ARMCACHE_1136JF

goto SetPaths

:_arm_cortexa8
set VXCPU=ARMARCH7
set B_GNUCPU=-DCPU=ARMARCH7 -DCPU_CORTEXA8 -DARMEL
set B_GNUOPT=-DARMMMU=ARMMMU_CORTEXA8 -DARMCACHE=ARMCACHE_CORTEXA8

goto SetPaths

:_arm_xscale
set VXCPU=XSCALE
set B_GNUCPU=-DCPU=%VXCPU%
set B_GNUOPT=
if "%B_BYTEORDER%" == "little" set B_GNUOPT=-txscale -DARMEL
if "%B_BYTEORDER%" == "big" set B_GNUOPT=-txscalebe -Wa,-EB -DARMEB
if "%B_BYTEORDER%" == "big" set VXBO=be
if "%B_GNUOPT%" == "" goto BadByteOrder

goto SetPaths

:_mips
set B_ASM=
set B_CC=ccmips
set B_LIB=armips

REM Validate byte order
REM
set T_ORDER=
if "%B_BYTEORDER%" == "little" set VXBO=le
if "%B_BYTEORDER%" == "big" set T_ORDER=EB
if "%B_BYTEORDER%" == "little" set T_ORDER=EL
if "%T_ORDER%" == "" goto BadByteOrder

REM Validate CPU type
REM
if "%P_CPUTYPE%" == "mipsi2"   set VXCPU=MIPSI2
if "%P_CPUTYPE%" == "mipsi3"   set VXCPU=MIPSI3
if "%P_CPUTYPE%" == "mips32"   set VXCPU=MIPS32
if "%P_CPUTYPE%" == "mipsi32"  set VXCPU=MIPSI32
if "%P_CPUTYPE%" == "mips64"   set VXCPU=MIPS64
if "%P_CPUTYPE%" == "mipsi64"  set VXCPU=MIPSI64
if "%VXCPU%" == "" goto BadCPUType

set B_GNUCPU=-DCPU=%VXCPU% -DMIPS%T_ORDER%
if "%P_CPUTYPE%" == "mipsi2"  set B_GNUCPU=%B_GNUCPU% -mips2 -mgp32 -DSOFT_FLOAT -msoft-float
if "%P_CPUTYPE%" == "mipsi32" set B_GNUCPU=%B_GNUCPU% -mips32 -mgp32 -DSOFT_FLOAT -msoft-float
if "%P_CPUTYPE%" == "mips32"  set B_GNUCPU=%B_GNUCPU% -mips2 -DSOFT_FLOAT -msoft-float
if "%P_CPUTYPE%" == "mipsi3"  set B_GNUCPU=%B_GNUCPU% -mips3 -mgp64 -mabi=n32
if "%P_CPUTYPE%" == "mipsi64" set B_GNUCPU=%B_GNUCPU% -mips64 -mgp64 -mabi=n32
if "%P_CPUTYPE%" == "mips64"  set B_GNUCPU=%B_GNUCPU% -mips3 -mgp64 -mabi=o64
set B_GNUOPT=-mno-branch-likely -G 0 -%T_ORDER%

REM Add a suffix for MIPSI2/MIPS32 only
if "%P_CPUTYPE%" == "mipsi2"  set VXCPU=MIPSI2sf
if "%P_CPUTYPE%" == "mips32"  set VXCPU=MIPS32sf
if "%P_CPUTYPE%" == "mipsi32" set VXCPU=MIPSI32sf
if "%P_CPUTYPE%" == "mipsi64" set VXCPU=MIPSI64sf

goto SetPaths

:_ppc
REM PPC stuff under VxWorks is always big-endian
set B_BYTEORDER=big
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
set B_GNUOPT=-mstrict-align -msoft-float

REM Special case for the 85xx
if "%P_CPUTYPE%" == "ppc85xx" set B_GNUCPU=-DCPU=PPC85XX
if "%P_CPUTYPE%" == "ppc85xx" set VXCPU=PPC85XXsf

REM Special case for the PPC32
if "%P_CPUTYPE%" == "ppc32" set VXCPU=PPC32
if "%P_CPUTYPE%" == "ppc32" set B_GNUCPU=-DCPU=PPC603 -DPPC32_fp60x -DCPU_VARIANT=_83xx
if "%P_CPUTYPE%" == "ppc32" set B_GNUOPT=-mstrict-align -mhard-float

goto SetPaths

:_sh
if not "%P_CPUTYPE%" == "sh7751" goto BadCPUType
set T_ORDER=
if "%B_BYTEORDER%" == "little" set T_ORDER=-ml
if "%B_BYTEORDER%" == "little" set VXBO=le
set B_ASM=
set B_CC=ccsh
set B_LIB=arsh
set VXCPU=SH7750
set B_GNUCPU=-DCPU=%VXCPU%
set B_GNUOPT=-m4 -DMS7751SE01 -fvolatile %T_ORDER%

goto SetPaths

:_simulator
REM We only support the NT simulator at this time
if not "%P_CPUTYPE%" == "simnt" goto BadCPUType
set B_ASM=
set B_CC=ccpentium
set B_LIB=arpentium
set VXCPU=SIMNT
set B_GNUCPU=-DCPU=%VXCPU% -march=i486
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
set B_GNUCPU=-DCPU=PENTIUM -march=pentium

goto _x86_done

:_pentium2
set VXCPU=PENTIUM2
set B_GNUCPU=-DCPU=PENTIUM2 -march=pentiumpro

goto _x86_done

:_pentium3
set VXCPU=PENTIUM3
set B_GNUCPU=-DCPU=PENTIUM3 -march=pentium3

goto _x86_done

:_pentium4
set VXCPU=PENTIUM4
set B_GNUCPU=-DCPU=PENTIUM4 -march=pentium4

goto _x86_done

:_x86_done

REM Tool names for VxWorks 6.x
set B_ASM=
set B_CC=ccpentium
set B_LIB=arpentium

set B_GNUOPT=-nostdlib -fno-defer-pop

goto SetPaths


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPaths
if "%VXCPU%" == "" goto BadCPUType

if "%1" == "/newproj" goto Success

set B_TOOLLIB=.
set B_TOOLINC=vxworks-%P_OSVER%\target\h

set B_TOOLBIN=
if "%P_OSVER%" == "6.0" set B_TOOLBIN=gnu\3.3.2-vxworks60\x86-win32\bin
if "%P_OSVER%" == "6.1" set B_TOOLBIN=gnu\3.3.2-vxworks61\x86-win32\bin
if "%P_OSVER%" == "6.2" set B_TOOLBIN=gnu\3.3.2-vxworks-6.2\x86-win32\bin
if "%P_OSVER%" == "6.3" set B_TOOLBIN=gnu\3.4.4-vxworks-6.3\x86-win32\bin
if "%P_OSVER%" == "6.4" set B_TOOLBIN=gnu\3.4.4-vxworks-6.4\x86-win32\bin
if "%P_OSVER%" == "6.5" set B_TOOLBIN=gnu\3.4.4-vxworks-6.5\x86-win32\bin
if "%P_OSVER%" == "6.6" set B_TOOLBIN=gnu\4.1.2-vxworks-6.6\x86-win32\bin
if "%P_OSVER%" == "6.7" set B_TOOLBIN=gnu\4.1.2-vxworks-6.7\x86-win32\bin
if "%P_OSVER%" == "6.8" set B_TOOLBIN=gnu\4.1.2-vxworks-6.8\x86-win32\bin
if "%P_OSVER%" == "6.9" set B_TOOLBIN=gnu\4.3.3-vxworks-6.9\x86-win32\bin

if "%B_TOOLBIN%" == "" goto BadOSVer

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
echo   "gnuvx6x"    - Works with the VxWorks GNU v6.x tools

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
echo               versions 6.0 through 6.9.
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

