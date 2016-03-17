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
REM Revision 1.23  2012/04/17 21:08:43Z  jimmb
REM Updated the toolinit to support VxWorks 6.9.2.  This version of 6.9
REM updated the diab compiler version.  Vxworks suppport 2 versions
REM of the diab compiler 5.9.0.0 and 5.9.1.0
REM Revision 1.22  2010/06/14 20:13:42Z  jimmb
REM Additional changes to support VxWorks 6.9
REM Revision 1.21  2010/06/11 17:51:08Z  jimmb
REM Syncronized diab and gnu 
REM Support for VxWorks 6.9
REM Revision 1.20  2010/06/09 16:46:12Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.9
REM Revision 1.19  2009/08/27 17:14:13Z  jimmb
REM Updated and added support for ppc32
REM Revision 1.18  2009/08/20 21:35:17Z  jimmb
REM Updated VxWorks build files to support VxWorks 6.8
REM Revision 1.17  2009/04/08 21:17:50Z  thomd
REM Fix loader problem - bug 1652
REM Revision 1.16  2009/03/30 18:40:41Z  jimmb
REM Updated the toots to suppot the Arm V6 processor and support
REM VxWorks 6.7 VSB builds.
REM Revision 1.15  2008/08/25 19:41:47Z  jimmb
REM Updated for VxWorks 6.7
REM Revision 1.14  2007/10/10 02:09:01Z  Garyp
REM Fixed a syntax error for PPC and SH targets.
REM Revision 1.13  2007/10/06 00:40:22Z  Garyp
REM Updated for VxWorks 6.6.
REM Revision 1.12  2007/09/05 23:01:17Z  jimmb
REM Updated the files to support the Windriver compiler using version 6.6
REM Revision 1.11  2007/09/05 17:53:49Z  jimmb
REM Added VxWorks 6.6 to the diab configuration
REM Revision 1.10  2007/02/07 19:40:12Z  Garyp
REM Corrected tool paths for VxWorks 6.5.
REM Revision 1.9  2007/01/18 19:09:57Z  peterb
REM (Per gp) updated to work with VxWorks 6.5.
REM Revision 1.8  2006/12/07 23:08:11Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the DOS
REM extended version which was necessary to support Win9x builds.
REM Revision 1.7  2006/11/08 00:51:18Z  Garyp
REM Added support for the /Info option.
REM Revision 1.6  2006/08/31 20:58:53Z  Garyp
REM Corrected some error messages.
REM Revision 1.5  2006/08/28 02:00:18Z  Garyp
REM Added VxWorks 6.4 support.  Changed to use the MAKE supplied with the DL
REM kits, which is the same version supplied in all the VxWorks 6.x stuff so
REM far.
REM Revision 1.4  2006/08/07 23:11:33Z  Garyp
REM Updated to properly initialize the VxWorks version portion of the DIAB CPU
REM flags.
REM Revision 1.3  2006/05/26 19:39:46Z  tonyq
REM Updated with VxWorks 6.3 support
REM Revision 1.2  2006/01/20 22:47:30  brandont
REM Updated comment for VxWorks 6.2 support.
REM Revision 1.1  2005/11/12 18:00:34Z  Pauli
REM Initial revision
REM Revision 1.2  2005/11/12 18:00:34Z  Garyp
REM Updated to support VxWorks 6.2.
REM Revision 1.1  2005/06/13 02:22:52Z  Garyp
REM Initial revision
REM Revision 1.2  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.1  2005/04/28 08:20:18Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not "%P_OS%" == "vxworks" if not "%P_OS%" == "loader" goto BadOS
if not exist %P_TOOLROOT%\diab\*.* goto BadOSVer

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

REM Ensure these are not set from a previous run
set VXCPU=
set VXBO=
set VXSFX=

REM -----------------------------------------------
REM     Validate CPU and get settings
REM -----------------------------------------------

if "%P_CPUFAMILY%" == "arm" goto _arm
if "%P_CPUFAMILY%" == "mips" goto _mips
if "%P_CPUFAMILY%" == "ppc" goto _ppc
if "%P_CPUFAMILY%" == "sh" goto _sh
if "%P_CPUFAMILY%" == "simulator" goto _simulator
if "%P_CPUFAMILY%" == "x86" goto _x86
goto BadCPUFam

:_arm
if "%P_CPUTYPE%" == "armarch5" goto _arm_arch5
if "%P_CPUTYPE%" == "armarch6" goto _arm_arch6
if "%P_CPUTYPE%" == "cortexa8" goto _arm_cortexa8
if "%P_CPUTYPE%" == "xscale" goto _arm_xscale
goto BadCPUType

:_arm_arch5
set VXCPU=ARMARCH5
set B_DIABCPU=
set B_DIABOPTS1=-W:c:,-Xmismatch-warning=2
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths

:_arm_arch6	

set VXCPU=ARMARCH6
set B_DIABCPU=
set B_DIABOPTS1=-W:c:,-Xmismatch-warning=2
set B_DIABOPTS2=
goto SetPaths

:_arm_cortexa8
set VXCPU=ARMARCH7
set B_DIABCPU=
set B_DIABOPTS1=-W:c:,-Xmismatch-warning=2
set B_DIABOPTS2=
set B_DIABOPTS3=

goto SetPaths

:_arm_xscale
set VXCPU=XSCALE

REM Validate byte order
REM
set T_ORDER=
if "%B_BYTEORDER%" == "little" set T_ORDER=EL
if "%B_BYTEORDER%" == "big" set T_ORDER=EB
if "%B_BYTEORDER%" == "big" set VXBO=be
if "%T_ORDER%" == "" goto BadByteOrder

set B_DIABCPU=
set B_DIABOPTS1=-DARM%T_ORDER%
set B_DIABOPTS2=
set B_DIABOPTS3=
goto SetPaths

:_mips
REM Validate byte order
REM
set T_ORDER=
if "%B_BYTEORDER%" == "little" set VXBO=le
if "%B_BYTEORDER%" == "little" set T_ORDER=EL
if "%B_BYTEORDER%" == "big" set T_ORDER=EB
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

set B_DIABCPU=-DMIPS%T_ORDER%
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=

REM Add a suffix for MIPSI2/MIPS32 only
if "%P_CPUTYPE%" == "mipsi2"  set VXSFX=MIPSI2sf
if "%P_CPUTYPE%" == "mips32"  set VXSFX=MIPS32sf
if "%P_CPUTYPE%" == "mipsi32" set VXSFX=MIPSI32sf
if "%P_CPUTYPE%" == "mipsi64" set VXSFX=MIPSI64sf
goto SetPaths

:_ppc
if "%P_CPUTYPE%" == "ppc403"    set VXCPU=PPC403
if "%P_CPUTYPE%" == "ppc405"    set VXCPU=PPC405
if "%P_CPUTYPE%" == "ppc440"    set VXCPU=PPC440
if "%P_CPUTYPE%" == "ppc603"    set VXCPU=PPC603
if "%P_CPUTYPE%" == "ppc604"    set VXCPU=PPC604
if "%P_CPUTYPE%" == "ppc860"    set VXCPU=PPC860
if "%P_CPUTYPE%" == "ppc85xx"   set VXCPU=PPC85XX
if "%P_CPUTYPE%" == "ppc32"     set VXCPU=PPC32

set B_DIABCPU=
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=

REM Special case for the PPC32
if "%P_CPUTYPE%" == "ppc32"     set B_DIABCPU=-DPPC32_fp60x -DCPU_VARIANT=_83xx

REM Sepcial case for the PPC85xx
if "%P_CPUTYPE%" == "ppc85xx"   set VXSFX=PPC85XXsf

if "%VXCPU%" == "" goto BadCPUType
goto SetPaths

:_sh
if not "%P_CPUTYPE%" == "sh7751" goto BadCPUType
set VXCPU=SH7750
set B_DIABCPU=
set B_DIABOPTS1=
set B_DIABOPTS2=
set B_DIABOPTS3=
if "%B_BYTEORDER%" == "little" set VXBO=le
goto SetPaths

:_simulator
if not "%P_CPUTYPE%" == "simnt" goto BadCPUType
set VXCPU=SIMNT
set DIABCPUFLAGS=-tX86LH
set B_DIABCPU=
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
set B_DIABCPU=
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

if "%1" == "/newproj" goto Success

set DIABBASE=
set VXVER=

if "%P_OSVER%" == "6.0" set DIABBASE=diab\5.2.2.0
if "%P_OSVER%" == "6.1" set DIABBASE=diab\5.2.3.0
if "%P_OSVER%" == "6.2" set DIABBASE=diab\5.3.1.0
if "%P_OSVER%" == "6.3" set DIABBASE=diab\5.4.0.0
if "%P_OSVER%" == "6.4" set DIABBASE=diab\5.5.0.0
if "%P_OSVER%" == "6.5" set DIABBASE=diab\5.5.1.0
if "%P_OSVER%" == "6.6" set DIABBASE=diab\5.6.0.0
if "%P_OSVER%" == "6.7" set DIABBASE=diab\5.7.0.0
if "%P_OSVER%" == "6.8" set DIABBASE=diab\5.8.0.0
if "%P_OSVER%" == "6.9" set DIABBASE=diab\5.9.1.0
if "%DIABBASE%" == "" goto BadOSVer

REM This is NOT a typo below where VXVER for 6.1 is set to 60, and 6.5 is set
REM to 64.  These match the values used in WorkBench when building with DIAB
REM tools for these versions.
if "%P_OSVER%" == "6.0" set VXVER=60
if "%P_OSVER%" == "6.1" set VXVER=60
if "%P_OSVER%" == "6.2" set VXVER=62
if "%P_OSVER%" == "6.3" set VXVER=63
if "%P_OSVER%" == "6.4" set VXVER=64
if "%P_OSVER%" == "6.5" set VXVER=64
if "%P_OSVER%" == "6.6" set VXVER=66
if "%P_OSVER%" == "6.7" set VXVER=67
if "%P_OSVER%" == "6.8" set VXVER=68
if "%P_OSVER%" == "6.9" set VXVER=69
if "%VXVER%" == "" goto BadOSVer

set B_DIABCPU=%B_DIABCPU% -DCPU=%VXCPU% %DIABCPUFLAGS%:vxworks%VXVER%

REM Vxworks 6.9 has two possible diab compilers
REM Check the updated one first then back up to the original

set B_TOOLBIN=%DIABBASE%\win32\bin
if exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto Setdiab

if "%P_OSVER%" == "6.9" set DIABBASE=diab\5.9.0.0
set B_TOOLBIN=%DIABBASE%\win32\bin


:Setdiab
set B_TOOLLIB=%DIABBASE%\win32\lib
set B_TOOLINC=vxworks-%P_OSVER%\target\h

REM TODO: Why do we set DIABLIB?  It is not used by TOOLSET.MAK.
REM       Is it a required setting for the tools themselves???
REM
set DIABLIB=%P_TOOLROOT%\%DIABBASE%

if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot

if "%VXSFX%" == "" set VXSFX=%VXCPU%

set B_VXLIBSUFFIX=%VXSFX%diab%VXBO%


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
echo   "diabvx6x"   - Works with the VxWorks DIAB Tools v6.x
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
echo               versions 6.0 through 6.8.
echo.
echo               P_TOOLROOT is not valid, or this version of VxWorks
echo               is not supported.
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
echo               Do not include a trailing ":vxworks6x" suffix, as this is
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
set VXBO=
set VXCPU=
set VXSFX=
set VXVER=

