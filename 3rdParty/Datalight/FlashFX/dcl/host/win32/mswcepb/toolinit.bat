@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to initialize a ToolSet.  This process is
REM dramatically different for THIS ToolSet because we are simply preparing
REM the SOURCES and DIRS files so that the Platform Builder BUILD utility
REM can do the real work.
REM
REM To see a more typical implementation of TOOLINIT.BAT see one of the
REM other ones -- don't use this one as a pattern for implementing a new
REM ToolSet.
REM
REM If the /info command-line switch is used, the batch file must display a
REM simple line describing the tools this ToolSet supports.
REM
REM On exit B_ERROR will be null on success, or set to an error description
REM on failure.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: toolinit.bat $
REM Revision 1.11  2012/04/13 15:00:18Z  johnb
REM Echo current value of B_INCLUDES to sources.cmn as well as
REM adding the B_INCLUDES macro to sources.cmn.
REM Revision 1.10  2012/02/28 18:38:50Z  johnb
REM Changed -WIN32_WCE to _WIN32_WCE.
REM Revision 1.9  2012/02/24 22:57:33Z  johnb
REM Added definition for _WIN32_WCE
REM Revision 1.8  2007/04/24 18:57:41Z  Garyp
REM Fixed the other half of the bug fixed in the previous revision, in P_CPUFAMILY.
REM The previous fix was to P_CPUTYPE rather than P_CPUNUM.
REM Revision 1.7  2007/04/23 21:13:15Z  jeremys
REM Changed B_CPUNUM to P_CPUNUM (B_CPUNUM doesn't exist).
REM Revision 1.6  2006/12/08 03:09:20Z  Garyp
REM Updated to allow ToolSet specific triggers for detecting errors and warnings
REM in the build logs.  Modified to no longer use "CPUNUM" functionality.
REM Updated to use gmakew32.exe exclusively and no longer support the
REM DOS extended version which was necessary to support Win9x builds.
REM Revision 1.5  2006/11/08 00:50:44Z  Garyp
REM Added support for the /Info option.
REM Revision 1.4  2006/09/20 00:07:50Z  Garyp
REM Modified to accommodate build numbers which are now strings rather than
REM numbers.
REM Revision 1.3  2006/01/01 16:56:02Z  Garyp
REM Fixed so the D_OSVER symbol is initialized.
REM Revision 1.2  2005/12/15 01:23:56Z  garyp
REM Fixed to buil properly with Platform Builder.
REM Revision 1.1  2005/10/01 13:56:30Z  Pauli
REM Initial revision
REM Revision 1.5  2005/06/13 03:12:32Z  PaulI
REM Added support for DCL lib prefix.
REM Revision 1.4  2005/04/26 18:39:52Z  GaryP
REM Added support for the B_BYTEORDER setting.
REM Revision 1.3  2005/03/11 23:27:15Z  GaryP
REM Modified to initialize B_LIBEXT.
REM Revision 1.2  2005/01/26 01:56:39Z  GaryP
REM Documentation update.
REM Revision 1.1  2005/01/11 19:20:37Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "/info" goto DisplayInfo

REM The ToolSet number MUST match those found in DLTLSET.H/INC!
set B_TOOLSETNUM=2401

REM Normalize the CPU values
call %P_DCLROOT%\host\win32\mswcepb\cecpu.bat

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

REM Set the prefix for the DCL libraries.  This is typically only done
REM for the MSWCEPB ToolSet, and then only if the master project being
REM built is something other than DCL itself.
if not "%P_PREFIX%" == "DCL" set B_DCLLIBPREFIX=%P_PREFIX%

REM Set the filename extension to use for libraries
set B_LIBEXT=lib


REM -----------------------------------------------
REM Build the SOURCES.CMN file
REM -----------------------------------------------

cd %P_ROOT%

echo # This is an auto-generated file.  Any changes manually made to this   >sources.cmn
echo # file will be lost when the next build is done.  Edit the real 	   >>sources.cmn
echo # version of this file in the %P_ROOT%\product directory.             >>sources.cmn
echo #                                                                     >>sources.cmn

if     "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -PD "D_PRODUCTNUM SETA %P_PRODUCTNUM%" >>sources.cmn
if     "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -PD "D_PRODUCTVER SETA %P_VERSIONVAL%" >>sources.cmn
if     "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -PD "D_TOOLSETNUM SETA %B_TOOLSETNUM%" >>sources.cmn
if not "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -DD_PRODUCTNUM=%P_PRODUCTNUM%          >>sources.cmn
if not "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -DD_PRODUCTVER=%P_VERSIONVAL%          >>sources.cmn
if not "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -DD_TOOLSETNUM=%B_TOOLSETNUM%          >>sources.cmn

REM These are no longer supported for assembly code at this time
REM if     "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -PD "D_PRODUCTBUILD SETA %P_BUILDNUM%" >>sources.cmn
REM if     "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -PD "D_CPUNUM SETA %B_CPUNUM%"         >>sources.cmn
REM if not "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -DD_PRODUCTBUILD=%P_BUILDNUM%          >>sources.cmn
REM if not "%CPUFAM%" == "arm" echo ADEFINES=$(ADEFINES) -DD_CPUNUM=%B_CPUNUM%                  >>sources.cmn


echo CDEFINES=-DD_PRODUCTNUM=%P_PRODUCTNUM% -DD_PRODUCTVER=%P_VERSIONVAL% -DD_PRODUCTBUILD=%P_BUILDNUM% -DD_CPUFAMILY=D_CPUFAMILY_%P_CPUFAMILY% -DD_CPUTYPE=%P_CPUTYPE% -DD_TOOLSETNUM=%B_TOOLSETNUM% -DD_OSVER=%P_OSVER% -D_WIN32_WCE=%_WINCEOSVER% >>sources.cmn

copy /a sources.cmn + product\sources.cmn sources.cmn >nul
echo %B_INCLUDES% \>>sources.cmn
echo $(B_INCLUDES) >>sources.cmn
cd %P_PROJDIR%

REM Regardless what ToolSet or toolchain is being used we always use
REM the Win32 version of GNU Make.
set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

REM When using Platform Builder, the B_ASM value is not important other than
REM that it must be set to something so that CPU.BAT will choose to traverse
REM the CPU tree.  Therefore, just set it to "auto".
set B_ASM=auto


REM -----------------------------------------------
REM The DIRS file for the projects tree needs to be
REM specially handled so generate it now.
REM -----------------------------------------------

echo DIRS = %P_PROJNAME%                     >%P_ROOT%\projects\dirs

set B_ERROR=
goto Fini


REM -----------------------------------------------
REM     Display Supported Versions
REM -----------------------------------------------
:DisplayInfo
echo   "mswcepb"    - Works with Windows CE 4.x Platform Builder
goto Fini





REM -----------------------------------------------
REM     Cleanup
REM -----------------------------------------------

:Fini
