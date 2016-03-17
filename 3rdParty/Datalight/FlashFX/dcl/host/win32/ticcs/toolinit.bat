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
REM Revision 1.5  2011/03/01 21:56:22Z  garyp
REM Updated for the more generic way that P_TOOLROOT is defined.
REM Moved target specific settings from toolset.mak into this file.
REM Revision 1.4  2010/10/21 13:23:05Z  jimmb
REM Fixed the syntax error that prohibited ticcs from displaying info.
REM Revision 1.3  2010/08/11 20:12:23Z  johnb
REM Added ARM9 support.
REM Revision 1.2  2010/05/24 18:32:47Z  garyp
REM Changed the default library extension to .LIB rather than .A.
REM Revision 1.1  2010/04/27 15:40:36Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM -----------------------------------------------
REM     Validation
REM -----------------------------------------------

if not exist %P_TOOLROOT%\. goto BadToolRoot

if "%B_BYTEORDER%" == "" set B_BYTEORDER=little

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
set B_LIBEXT=lib


REM -----------------------------------------------
REM     Validate CPU and get settings
REM -----------------------------------------------

if "%P_CPUFAMILY%" == "c6000" goto _c6000
if "%P_CPUFAMILY%" == "arm" goto _arm
goto BadCPUFam

REM  ------ C6000 Family ------

:_c6000
set B_CPUDIRNAME=c6000
set B_LOC=
set B_LINK=lnk6x
set B_ASM=asm6x
set B_CC=cl6x
set B_LIB=ar6x

if "%P_CPUTYPE%" == "c6748" goto _c6748
goto BadCPUType

:_c6748
set B_TSCPU=--silicon_version=6400+
set B_TSOPT=
if "%B_BYTEORDER%" == "big" set B_TSOPT=-me
set B_TSOPT=%B_TSOPT% -D=DCL_CPU_ALIGNSIZE=8

if "%B_BYTEORDER%" == "big" set B_TSEND=-DDCL_BIG_ENDIAN=1
if "%B_BYTEORDER%" == "little" set B_TSEND=-DDCL_BIG_ENDIAN=0

goto SetPaths


REM  ------ ARM Family ------

:_arm
set B_CPUDIRNAME=tms470
set B_LOC=
set B_LINK=cl470 --run_linker
set B_ASM=cl470
set B_CC=cl470
set B_LIB=ar470

if "%P_CPUTYPE%" == "arm926ejs" goto _arm926ejs
goto BadCPUType

:_arm926ejs
set B_TSCPU=
set B_TSOPT=--endian=%B_BYTEORDER% -D=DCL_CPU_ALIGNSIZE=4

if "%B_BYTEORDER%" == "big" set B_TSEND=-DDCL_BIG_ENDIAN=1
if "%B_BYTEORDER%" == "little" set B_TSEND=-DDCL_BIG_ENDIAN=0

goto SetPaths


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPaths

if "%1" == "/newproj" goto Success

set B_TOOLLIB=%B_CPUDIRNAME%\lib
set B_TOOLINC=%B_CPUDIRNAME%\include
set B_TOOLBIN=%B_CPUDIRNAME%\bin

if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot


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
echo   "ticcs"      - Works with the TI Code Composer Studio tools
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

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
set B_CPUDIRNAME=
