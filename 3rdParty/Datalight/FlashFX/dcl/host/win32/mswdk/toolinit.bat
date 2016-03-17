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
REM Revision 1.2  2009/09/10 21:13:10Z  jeremys
REM The path to rc.exe was wrong, fixed.
REM Revision 1.1  2008/02/12 00:09:06Z  jeremys
REM Initial revision
REM Revision 1.4  2006/12/07 23:08:08Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.3  2006/11/08 00:50:30Z  Garyp
REM Added support for the /Info option.
REM Revision 1.2  2006/03/08 18:57:35Z  joshuab
REM Remove excess include path.
REM Revision 1.1  2005/06/16 18:34:50Z  Pauli
REM Initial revision
REM Revision 1.3  2005/06/16 19:34:49Z  PaulI
REM Merged with latest revision from XP variant branch.
REM Now (if the IFSKIT is being used) requires that the B_IFSKITTARG
REM variable be set to wxp or w2k make sure whatever you pick matches
REM Revision 1.2  2005/06/13 02:22:52Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.1  2005/03/31 21:06:52Z  garyp
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

REM CE's directory structure and required flags perfectly match
REM neither of our P_CPUTYPE nor P_CPUFAMILY settings so calc
REM the values needed for CE.

if "%P_CPUTYPE%" == "386" goto _386
goto BadCPUType

:_386
set B_CC=cl
set B_RC=rc
set B_ASM=
goto SetPath


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

:SetPath
REM If we were simply running NEWPROJ, quit now since we don't have
REM the P_OSVER environment variable.  If we've gotten this far,
REM we have done all the validation we can.

if "%1" == "/newproj" goto Success

if not "%P_OSVER%" == "ntddk" goto InitIFSKIT
set B_TOOLBIN=bin
set B_TOOLINC=include
set B_TOOLLIB=lib
REM For the NTDDK, the standard C library headers are handled by Visual C
set B_CLIBINC=
if     "%B_DEBUG%" == "0" set B_OSLIBPATH=%P_TOOLROOT%\libfre\i386
if not "%B_DEBUG%" == "0" set B_OSLIBPATH=%P_TOOLROOT%\libchk\i386
goto CheckPath

:InitIFSKIT
if not "%P_OSVER%" == "ifskit" goto BadOSVer
if "%B_IFSKITTARG%" == "" goto BadIFSKITTARG
set B_TOOLBIN=bin\x86\x86
set B_TOOLINC=inc\crt
set B_TOOLLIB=lib\%B_IFSKITTARG%\i386
set B_CLIBINC=
set B_OSLIBPATH=%P_TOOLROOT%\lib\%B_IFSKITTARG%\i386
set B_MFCINC=$(P_TOOLROOT)\inc\mfc42
set B_WININC=$(P_TOOLROOT)\inc\api

:CheckPath
if not exist %P_TOOLROOT%\%B_TOOLBIN%\*.* goto BadToolRoot
if not exist %P_TOOLROOT%\%B_TOOLLIB%\*.* goto BadToolRoot


REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

set B_LIB=link /lib
set B_LOC=
set B_LINK=link

REM If the tools are not "inpath" set to the full path

if "%B_INPATH%" == "" if not "%B_ASM%" == "" set B_ASM=%P_TOOLROOT%\%B_TOOLBIN%\%B_ASM%
if "%B_INPATH%" == "" set B_CC=%P_TOOLROOT%\%B_TOOLBIN%\%B_CC%
if "%B_INPATH%" == "" set B_RC=%P_TOOLROOT%\%B_TOOLBIN%\..\%B_RC%
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
echo   "mswinddk"   - Works with the Microsoft Windows DDK
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

:BadOSVer
echo.
echo toolinit.bat: The P_OSVER environment variable must be set to "ifskit"
echo               if the Windows IFS Kit is being used, or "ntddk" if the
echo               Windows DDK and MS Visual C are being used.
echo.
set B_ERROR=TOOLINIT_BadOSVer
goto Fini

:BadIFSKITTARG
echo.
echo toolinit.bat: The B_IFSKITTARG environment variable must be set to "wxp"
echo               or "w2k" to specify whether WinXP or Win2K libraries should
echo               linked in.
echo.
set B_ERROR=TOOLINIT_BadIFSKITTARG
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
