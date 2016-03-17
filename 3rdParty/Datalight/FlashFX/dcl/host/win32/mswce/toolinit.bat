@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to initialize a ToolSet.  The following variables
REM will be initialized:
REM
REM B_TOOLBIN  - The name of the bin directory relative to P_TOOLROOT
REM B_TOOLINC  - The name of the include directory relative to P_TOOLROOT
REM B_TOOLLIB  - The name of the lib directory relative to P_TOOLROOT (CE 4.x)
REM B_CESDKDIR - The full path to the %_PROJECTROOT%\cesysgen\sdk\lib directory
REM B_CEOAKDIR - The full path to the %_PROJECTROOT%\cesysgen\oak\lib directory
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
REM Revision 1.4  2011/06/02 18:27:18Z  johnb
REM Changed logic to skip adding 386 specific compiler optiopn 
REM if CE version is 6.00 or 7.00.
REM Revision 1.3  2011/06/02 18:08:07Z  johnb
REM Added missing WEC7 compiler macro for 386
REM Added missing WEC7 compiler and linker macros for SH
REM Revision 1.2  2011/04/27 16:59:45Z  johnb
REM Added Windws Embedded Compact 7 support
REM Added ARM7 CPU support
REM Revision 1.1  2008/06/20 03:03:22Z  garyp
REM Initial revision
REM Revision 1.7.1.2  2008/05/28 02:00:16Z  garyp
REM Added the /QRxscale switch.  Added an apparently missing 'I" to the
REM ARMV4 designation.
REM Revision 1.7  2008/03/12 21:29:02Z  Garyp
REM Added commented out thumb compilation options.
REM Revision 1.6  2007/11/24 18:10:01Z  Garyp
REM Added support for CE 5.02 -- AKA WinMobile 6.
REM Revision 1.5  2007/05/25 18:51:56Z  Garyp
REM Fixed a compiler flags issue when using CE6 for x86 targets.  Removed
REM remnants of CE3 support.
REM Revision 1.4  2007/03/01 21:17:23Z  Garyp
REM Added CE 6.0 support.  Removed CE 3.0 support.
REM Revision 1.3  2006/12/07 23:08:08Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.2  2006/11/08 00:50:49Z  Garyp
REM Added support for the /Info option.
REM Revision 1.1  2005/12/01 02:13:34Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/01 02:13:33Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.11  2005/11/09 17:04:09Z  Pauli
REM Updated to use B_PAUSE so as to work with the "nopause" option.
REM Revision 1.10  2005/08/26 21:17:01Z  tonyq
REM Added support for the ARMV4I CPU type to the MSWCE4 toolset
REM Revision 1.9  2005/08/26 19:09:43Z  tonyq
REM Added support for the ARMV4I CPU type to the MSWCE4 toolset
REM Revision 1.8  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.7  2005/04/26 18:39:52Z  GaryP
REM Added support for the B_BYTEORDER setting.
REM Revision 1.6  2005/03/18 22:05:45Z  GaryP
REM Documentation update.
REM Revision 1.5  2005/03/18 02:56:55Z  GaryP
REM Updated to use P_OSROOT and P_OSVER.  Honor the /newproj switch so
REM we don't fail if the CE target libraries are not built yet.
REM Revision 1.4  2005/02/27 05:23:38Z  GaryP
REM Normalized settings to work better with CE 5.
REM Revision 1.3  2005/02/27 01:38:33Z  GaryP
REM Fixed so the CECLDIR only applies to CE 5.
REM Revision 1.2  2005/02/26 02:00:29Z  GaryP
REM Modified so the B_INPATH setting does not have to be used for CE 5.
REM Revision 1.1  2005/01/11 19:20:36Z  GaryP
REM Initial revision
REM Revision 1.9  2004/12/21 04:23:33Z  GaryP
REM Added the B_CEOAKDIR value.
REM Revision 1.8  2004/12/20 00:06:03Z  jeremys
REM Changed MIPSII to mipsii.
REM Revision 1.5  2004/10/16 07:17:22Z  garys
REM Added MIPSII support in CE 4.2 and 5.0
REM Revision 1.4  2004/09/29 18:23:14  garys
REM Added /QRimplicit-import- for armv4 and armv4i to avoid link errors
REM Revision 1.3  2004/09/27 18:06:44  tonyq
REM Updated for Windows CE 5.0 Platform builder (command line build)
REM Revision 1.1  2004/07/06 20:24:34  GaryP
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
REM     Branch to the appropriate CPU section
REM -----------------------------------------------

set CECLDIR=
set CELKDIR=

REM CE's directory structure and required flags perfectly match
REM neither of our P_CPUTYPE nor P_CPUFAMILY settings so calc
REM the values needed for CE.

if "%P_CPUTYPE%" == "386" goto _386
if "%P_CPUTYPE%" == "arm" goto _armv4
if "%P_CPUTYPE%" == "armv4" goto _armv4
if "%P_CPUTYPE%" == "armv4i" goto _armv4i
if "%P_CPUTYPE%" == "xscale" goto _xscale
if "%P_CPUTYPE%" == "sh3" goto _sh3
if "%P_CPUTYPE%" == "sh4" goto _sh4
if "%P_CPUTYPE%" == "mipsii" goto _mipsii
if "%P_CPUTYPE%" == "armv7" goto _armv7
goto BadCPUType

:_386
set B_CC=cl
set B_ASM=ml
set B_RC=rc
if "%P_OSVER%" == "5.00" set CECLDIR=x86\
if "%P_OSVER%" == "5.02" set CECLDIR=x86\
if "%P_OSVER%" == "6.00" set CECLDIR=x86\
if "%P_OSVER%" == "6.00" set CELKDIR=x86\
if "%P_OSVER%" == "7.00" set CECLDIR=x86\
if "%P_OSVER%" == "7.00" set CELKDIR=x86\
set CECPUDIR=x86
set B_CECPUC=/Dx86 /D_X86_

if "%P_OSVER%" == "6.00" goto _386_no_ifdiv
if "%P_OSVER%" == "7.00" goto _386_no_ifdiv
set B_CECPUC=%B_CECPUC% -QIfdiv-

:_386_no_ifdiv
set B_CECPUL=X86
set B_CELFLAGS=
goto SetPath

:_armv4
set B_CC=clarm
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.00" set CECLDIR=arm\
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "5.02" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "6.00" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set CELKDIR=arm\
if "%P_OSVER%" == "7.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set CECLDIR=arm\
if "%P_OSVER%" == "7.00" set CELKDIR=arm\
set B_ASM=armasm
set B_RC=rc
set B_CECPUL=ARM
set CECPUDIR=armv4
set B_CECPUC=/DARM /DARMV4 /D_ARM_ /QRimplicit-import-
set B_CELFLAGS=-merge:.astart=.text -align:4096
goto SetPath

:_armv4i
set B_CC=clarm
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.00" set CECLDIR=arm\
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "5.02" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "6.00" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set CELKDIR=arm\
if "%P_OSVER%" == "7.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set CECLDIR=arm\
if "%P_OSVER%" == "7.00" set CELKDIR=arm\
set B_ASM=armasm
set B_RC=rc
set B_CECPUL=thumb
set CECPUDIR=armv4i
set B_CECPUC=/DARM /DARMV4I /D_ARM_ /QRimplicit-import- /QRxscale
set B_CELFLAGS=-merge:.astart=.text -align:4096
goto SetPath

:_armv7
set B_CC=clarm
if "%P_OSVER%" == "7.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set CECLDIR=arm\
if "%P_OSVER%" == "7.00" set CELKDIR=arm\
set B_ASM=armasm
set B_RC=rc
set B_CECPUL=thumb
set CECPUDIR=armv7
set B_CECPUC=/DARM /DARMV7 /D_ARM_ /QRimplicit-import- /QRxscale
set B_CELFLAGS=-merge:.astart=.text -align:4096
goto SetPath

:_xscale
set CECPUDIR=armv4i
set B_CECPUC=/DARM /DARMV4I /D_ARM_ /DTHUMBSUPPORT /QRarch4T /QRinterwork-return /QRimplicit-import-
set B_CECPUL=thumb
set B_CELFLAGS=-merge:.astart=.text -align:4096
set B_CC=clarm
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.00" set CECLDIR=arm\
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "5.02" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "6.00" set CECLDIR=arm\
if "%P_OSVER%" == "6.00" set CELKDIR=arm\
if "%P_OSVER%" == "7.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set CECLDIR=arm\
if "%P_OSVER%" == "7.00" set CELKDIR=arm\
set B_ASM=armasm
set B_RC=rc
goto SetPath

:_sh3
set B_ASM=shasm
set B_RC=rc
set B_CECPUL=SH3
set CECPUDIR=sh3
set B_CECPUC=/DSH /D_SHX_ /DSH3 /DSH3_CACHE=1 /DSHx
set B_CELFLAGS=-align:4096
if "%P_OSVER%" == "4.00" set B_CELFLAGS=
set B_CC=clsh
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.00" set CECLDIR=sh\
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "5.02" set CECLDIR=sh\
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "6.00" set CECLDIR=sh\
if "%P_OSVER%" == "6.00" set CELKDIR=sh\
if "%P_OSVER%" == "7.00" set CECLDIR=sh\
if "%P_OSVER%" == "7.00" set CELKDIR=sh\
goto SetPath

:_sh4
set B_ASM=shasm
set B_RC=rc
set B_CECPUL=SH4
set B_CELFLAGS=-align:4096
if "%P_OSVER%" == "5.00" set CECLDIR=sh\
if "%P_OSVER%" == "5.02" set CECLDIR=sh\
if "%P_OSVER%" == "6.00" set CECLDIR=sh\
if "%P_OSVER%" == "6.00" set CELKDIR=sh\
if "%P_OSVER%" == "7.00" set CECLDIR=sh\
if "%P_OSVER%" == "7.00" set CELKDIR=sh\
set CECPUDIR=sh4
REM SH3_CACHE on the line below is identical to what PB sets
set B_CECPUC=/DSH /DSH4 /D_SHX_ /DSHx /DSH3_CACHE=1 /Qsh4r5
set B_CC=clsh
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set B_CC=cl
goto SetPath

:_mipsii
set B_ASM=mipsasm
set B_RC=rc
set B_CECPUL=MIPS
set B_CELFLAGS=-MERGE:.rdata=.text -align:4096
set CECPUDIR=mipsii
set B_CECPUC=-DMIPS -DMIPSII -D_MIPS_ -DR4000
set B_CC=clmips
if "%P_OSVER%" == "5.00" set B_CC=cl
if "%P_OSVER%" == "5.00" set CECLDIR=mips\
if "%P_OSVER%" == "5.02" set B_CC=cl
if "%P_OSVER%" == "5.02" set CECLDIR=mips\
if "%P_OSVER%" == "6.00" set B_CC=cl
if "%P_OSVER%" == "6.00" set CECLDIR=mips\
if "%P_OSVER%" == "6.00" set CELKDIR=mips\
if "%P_OSVER%" == "7.00" set B_CC=cl
if "%P_OSVER%" == "7.00" set CECLDIR=mips\
if "%P_OSVER%" == "7.00" set CELKDIR=mips\
goto SetPath


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPath
set B_TOOLBIN=sdk\bin\i386
if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\public\common\sdk\lib\%CECPUDIR%\*.* goto BadCPUDir

REM If we were simply running NEWPROJ, quit now since we may not have any
REM of the CE specific environment variables.  If we've gotten this far,
REM we have done all the validation we can.

if "%1" == "/newproj" goto Success

set B_TOOLINC=public\common\sdk\inc
set B_TOOLLIB=public\common\sdk\lib\%CECPUDIR%\%WINCEDEBUG%

REM -----------------------------------------------
REM     Mix and match debug and release?
REM -----------------------------------------------
if "%P_OSVER%" == "7.00" goto mixnmatch_WEC7
set B_CESDKDIR=%_PROJECTROOT%\cesysgen\sdk\lib\%CECPUDIR%\%WINCEDEBUG%
set B_CEOAKDIR=%_PROJECTROOT%\cesysgen\oak\lib\%CECPUDIR%\%WINCEDEBUG%
goto mixnmatch_check

:mixnmatch_WEC7
set B_CESDKDIR=%SG_OUTPUT_ROOT%\sdk\lib\%CECPUDIR%\%WINCEDEBUG%
set B_CEOAKDIR=%SG_OUTPUT_ROOT%\oak\lib\%CECPUDIR%\%WINCEDEBUG%

:mixnmatch_check
if     "%B_DEBUG%" == "0" if "%WINCEDEBUG%" == "retail" goto CheckPath
if not "%B_DEBUG%" == "0" if "%WINCEDEBUG%" == "debug"  goto CheckPath

echo.
echo TOOLINIT:  The debug level for %P_PRODUCT% does not match the debug
echo            setting for Windows CE:
echo.
echo                     B_DEBUG=%B_DEBUG%
echo                     WINCEDEBUG=%WINCEDEBUG%
echo.
if "%B_PAUSE%" == "" goto CheckPath
echo            Press any key to continue building, or CTRL+C to break...
echo.
%B_PAUSE% >nul

:CheckPath
if not exist %B_CESDKDIR%\*.* goto BuildCEFirst
if not exist %B_CEOAKDIR%\*.* goto BuildCEFirst


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

set B_LIB=lib
set B_LOC=
set B_LINK=link

REM If the tools are not "inpath" set to the full path

if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%CECLDIR%%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%CECLDIR%%B_CC%
if "%B_INPATH%" == "" set B_RC=%P_TOOLROOT%\%B_TOOLBIN%\%B_RC%
if "%B_INPATH%" == "" set B_LINK=%P_TOOLROOT%\%B_TOOLBIN%\%CELKDIR%%B_LINK%
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
echo   "mswce"      - Works with Windows CE 4.x and later (and WinMobile)
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadCPUType
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not recognized for Windows CE.
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

:BadCPUDir
echo.
echo toolinit.bat: The CPU "%P_CPUTYPE%" is not installed for Windows CE.
echo           The CPU name must be lowercase.
echo.
set B_ERROR=TOOLINIT_BadCPUDir
echo.
echo Installed CPU types are:
echo.
dir /b %P_TOOLROOT%\public\common\sdk\lib\*.*
echo.
%B_PAUSE%
goto Fini

:BuildCEFirst
echo.
echo toolinit.bat: Before building %P_PRODUCT%, you must first build
echo               a Windows CE image so that the libraries in the
echo               "%B_CESDKDIR%"
echo               directory are created.
echo.
echo               Additionally you must be using a command prompt
echo               created with the "Open Release Directory..." option
echo               on the "Build OS" menu in Platform Builder.  This
echo               ensures that all the environment variables are set.
echo.
set B_ERROR=TOOLINIT_BuildCEFirst
echo.
goto Fini


REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini
set CECLDIR=
set CELKDIR=
set CECPUDIR=
