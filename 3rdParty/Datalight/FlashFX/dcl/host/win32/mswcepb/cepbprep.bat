@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to prepare a project to be built with Windows
REM CE Platform Builder.  We generate the prjinfo.bat file on the fly,
REM using settings in the current environment.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: cepbprep.bat $
REM Revision 1.6  2012/04/13 15:00:08Z  johnb
REM Update to add B_INCLUDES to PRJINFO.BAT 
REM Revision 1.5  2012/04/12 20:15:16Z  johnb
REM Updates for MSWCEPB toolset.
REM Revision 1.4  2009/04/29 18:01:16Z  johnb
REM Added environment variable, P_SOURCESCMN, to indiicate 
REM location of SOURCES.CMN in the product root.
REM Revision 1.3  2009/02/08 00:22:55Z  garyp
REM Documentation cleanup.
REM Revision 1.2  2005/12/15 01:08:08Z  garyp
REM Fixed to build properly with Platform Builder.
REM Revision 1.1  2005/08/21 10:43:00Z  Pauli
REM Initial revision
REM Revision 1.2  2005/08/21 11:43:00Z  garyp
REM Updated to work with new project make files.
REM Revision 1.1  2005/01/20 18:58:14Z  pauli
REM Initial revision
REM -------------------------------------------------------------------------

@if "%ECHO%" == "" set ECHO=off
@echo %ECHO%

if "%3" == "" goto BadParam
if "%4" == "" goto BadParam
if "%4" == "wince" goto setvals
if "%4" == "wincebl" goto setvals
goto BadParam

:setvals
set P_ROOT=%1
set P_PROJNAME=%2
set B_DEBUG=%3
set P_DCLROOT=%1\dcl
set P_SOURCESCMN=%1\sources.cmn
set P_OS=%4

if "%P_ROOT%" == "" goto BadRoot
if exist %P_ROOT%\cepbprep.tmp del %P_ROOT%\cepbprep.tmp
echo test dir >cepbprep.tmp
if not exist %P_ROOT%\cepbprep.tmp goto BadRoot
del cepbprep.tmp

if not exist %P_ROOT%\product\prodinfo.bat goto BadRoot
call %P_ROOT%\product\prodinfo.bat

if "%P_PROJNAME%" == "" goto BadProj
if not exist %P_ROOT%\projects\%P_PROJNAME%\???main.c goto BadProj

REM Use the CE _TGTCPU... environment settings to determine the
REM proper CPU family and type values for the build environment.
REM On exit the CPUFAM and CPUTYP vars will be set to project
REM standard names.
call %P_DCLROOT%\host\win32\mswcepb\cecpu.bat

REM Build an appropriate PRJINFO.BAT based on the current settings.

echo  >%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_ROOT=%P_ROOT%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_PROJDIR=%P_ROOT%\projects\%P_PROJNAME%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set B_DEBUG=%B_DEBUG%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_OS=%P_OS%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_TOOLSET=mswcepb
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_TOOLROOT=%_WINCEROOT%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_CPUFAMILY=%CPUFAM%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set P_CPUTYPE=%CPUTYP%
echo >>%P_ROOT%\projects\%P_PROJNAME%\prjinfo.bat set B_INCLUDES=$(FX_ROOT)\os\%P_OS%\include;$(FX_ROOT)\dcl\os\%P_OS%\include;$(FX_ROOT)\dcl\os\win\include;
 
echo.
echo ----------------------------------------------------------------------------------
echo Preparing %P_PRODUCT% DIRS, SOURCES, and MAKEFILEs for Windows CE
echo ----------------------------------------------------------------------------------
echo.

cd %P_ROOT%\projects\%P_PROJNAME%

call prjbuild.bat /nopause

goto Fini

:BadParam
echo.
echo Error - CEPBPREP.BAT:  Parameter error: P_ROOT="%1" 
echo P_PROJNAME="%2" P_DEBUG="%3" os="%4"
echo.
echo CEPPBREP.BAT {FFXROOT} {PROJNAME} {DEBUG} {OS}
echo.
echo FFXROOT  -- root directory of FlashFX Tree
echo PROJNAME -- name of project to use
echo DEBUG    -- debug level 0 - 3
echo OS       -- wince or wincebl
echo.
pause
goto Fini

:BadRoot
echo.
echo Error - CEPBPREP.BAT:  The P_ROOT value "%P_ROOT%" is invalid  
echo or non-existant.  You must be located in the directory specified by
echo P_ROOT when CEPBPREP is run.
echo.
pause
goto Fini

:BadProj
echo
echo Error - %P_PRODUCT%:CEPBPREP.BAT:  The P_PROJNAME setting "%P_PROJNAME%"
echo does not name a valid project that exists in the %P_ROOT%\PROJECTS tree.
echo.
pause


:Fini
