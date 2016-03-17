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
REM Revision 1.7  2009/04/16 14:48:32Z  keithg
REM Added support for specifing the endian order.  Note this is specifically
REM for use in simulation tests and using the endian order switches has no
REM effect on the native endian order.
REM Revision 1.6  2009/04/16 00:53:16Z  garyp
REM Corrected not to use an '&' operation in an echo'd string.
REM Revision 1.5  2009/02/08 00:20:32Z  garyp
REM Merged from the v4.0 branch.  Added support for RC.
REM Revision 1.4  2008/07/22 21:16:34Z  keithg
REM Added support for VS 9.0 and broadened CPU support (x64)
REM Revision 1.3  2007/10/11 22:01:22Z  Garyp
REM Minor information message fix.
REM Revision 1.2  2007/04/05 02:16:24Z  Garyp
REM Fixed up to work with P_TOOLROOT paths which include spaces.
REM Revision 1.1  2007/03/20 20:32:32Z  Garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo


REM -----------------------------------------------
REM     Get Compiler Specific data
REM -----------------------------------------------

call %P_DCLROOT%\host\win32\%P_TOOLSET%\msvsinfo.bat
if not defined B_MSVS_VER goto BadTools
if not defined B_MSVS_TGT goto BadTools
if not defined B_MSVS_DSC goto BadTools
if not defined B_MSVS_DIR goto BadTools


REM -----------------------------------------------
REM     Initial Settings
REM -----------------------------------------------

REM Set the prefix for the DCL libraries
set B_DCLLIBPREFIX=

REM Set the filename extension to use for libraries
set B_LIBEXT=lib

if "%B_BYTEORDER%" == "" set B_BYTEORDER=little

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

REM Nothing to do here...


REM -----------------------------------------------
REM     Validate the CPU
REM -----------------------------------------------

call %P_DCLROOT%\host\win32\%P_TOOLSET%\cpucheck.bat %P_CPUTYPE%
if "%P_CPUFAMILY%"=="" goto BadCPUType

REM Ensure the development environment matches the tools found
REM
if not "%B_MSVS_TGT%"=="%P_CPUTYPE%" goto BadToolsTgt

REM Validate the tools do indeed support the given CPU
REM


REM -----------------------------------------------
REM     Record the names of the tools
REM -----------------------------------------------

set B_CC=cl
set B_LIB=lib
set B_LINK=link
set B_ASM=
set B_LOC=
set B_RC=rc


REM -----------------------------------------------
REM     Set BIN, INC, and LIB paths
REM -----------------------------------------------

REM This toolset requires that the user have set up the environment
REM all tools and paths and private toolset environment is setup.
REM This is necessary since the setup may include different paths
REM for different configurations. (amd64 vs. i386 for example)
REM whereas the tools and libraries may look identical otherwise.

:SetPath
set B_TOOLBIN=
set B_TOOLINC=
set B_TOOLLIB=

REM -----------------------------------------------
REM     Set Misc Common Tools
REM -----------------------------------------------

REM Indicate to other DL tools that the toolset binaries are in the path
REM and already setup.  This will avoid use of full paths.
set B_INPATH=1


REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

if not "%B_BYTEORDER%" == "little" if not "%B_BYTEORDER%" == "big" goto BadByteOrder
if "%B_BYTEORDER%" == "little" set B_END=-DDCL_BIG_ENDIAN=0
if "%B_BYTEORDER%" == "big" set B_END=-DDCL_BIG_ENDIAN=1

set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "msvs"       - Works with Microsoft Visual Studio 6, 7, 8, and 9
goto Fini


REM -----------------------------------------------
REM     Error Handlers
REM -----------------------------------------------

:BadToolsTgt
echo.
echo toolinit.bat: The tools target CPU (%B_MSVS_TGT%) does not match the project (%P_CPUTYPE%).
echo               Ensure the environment is fully configured.
echo.
set B_ERROR=TOOLINIT_BadToolsTgt
goto Fini


:BadTools
echo.
echo toolinit.bat: Tools version information cannot be determined.
echo               Ensure the environment is fully configured.
echo.
set B_ERROR=TOOLINIT_BadTools
goto Fini

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

