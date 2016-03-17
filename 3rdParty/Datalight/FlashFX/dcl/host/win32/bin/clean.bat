@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is a helper batch file used when cleaning the project.
REM
REM On exit, B_ERROR will be clear if there were no errors, and set to an
REM error context if there was a problem.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: clean.bat $
REM Revision 1.8  2011/03/07 02:22:41Z  garyp
REM Fixed to initialize B_MAKE if not already initialized.
REM Revision 1.7  2011/03/06 23:25:50Z  garyp
REM Updated to invoke the PROJCLEAN target within any project specific
REM make files.
REM Revision 1.6  2011/01/13 02:25:41Z  garyp
REM Removed legacy DOS box support.
REM Revision 1.5  2009/02/08 00:15:53Z  garyp
REM Merged from the v4.0 branch.  Minor updates to some messages.
REM Revision 1.4  2008/01/31 22:50:28Z  Garyp
REM Updated the "everything" functionality to delete the "kitroot" directory
REM tree.
REM Revision 1.3  2006/08/29 02:56:40Z  Garyp
REM Added messages, and updated to ignore the SDK directory.
REM Revision 1.2  2006/07/30 21:15:04Z  Garyp
REM Modified to use a CMD shell rather than the call command when executing
REM sub-processes.
REM Revision 1.1  2005/02/24 04:04:12Z  Pauli
REM Initial revision
REM Revision 1.2  2005/02/24 02:04:12Z  GaryP
REM Fixed to clean the root of the project.
REM Revision 1.1  2005/01/11 19:20:35Z  GaryP
REM Initial revision
REM Revision 1.2  2004/11/17 02:06:36Z  jeremys
REM Changed CMDPGM environment variable use 32k environment size instead of
REM 4k.  This fixed an "out of environment space" error.
REM Revision 1.1  2004/07/07 00:48:00Z  brandont
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

set B_ERROR=

if "%1" == "?" goto Help
if "%1" == "/?" goto Help

set CLEANALL=0
if /I "%1" == "all" shift && set CLEANALL=1
if /I "%1" == "everything" shift && set CLEANALL=2

if not "%1" == "" goto Help

if "%B_MAKE%" == "" set B_MAKE=%P_DCLROOT%\host\win32\bin\gmakew32

echo.
echo Cleaning the product tree... One moment please...
echo.


REM ----------------------------------------------------------------------
REM Validate settings
REM ----------------------------------------------------------------------

if "%P_ROOT%" == "" goto BadRoot
if not exist %P_ROOT%\product\prodinfo.bat goto BadRoot

call %P_ROOT%\product\prodinfo.bat

REM Confirm that we really are currently sitting in the Project Directory
if exist testdir.tmp del testdir.tmp
echo test dir >%P_PROJDIR%\testdir.tmp
if not exist testdir.tmp goto BadProjDir
del testdir.tmp

 
REM ----------------------------------------------------------------------
REM Clean the Product Tree (excluding the Projects tree)
REM ----------------------------------------------------------------------

cd %P_ROOT%
REM Delete any LASTPROJ.BAT so MAKEALL knows we did a clean
REM
if exist lastproj.bat del lastproj.bat

REM Clean the root of the product
REM
call %P_DCLROOT%\host\win32\bin\cleanhlp.bat

REM Call cleandir for each buildable tree (ignores the PROJECTS tree)
REM
call %P_DCLROOT%\host\win32\bin\cleandir.bat %P_TREES%

REM Clean miscellaneous stuff
REM
if exist %P_ROOT%\build.*       del %P_ROOT%\build.*
if exist %P_ROOT%\sources.cmn   del %P_ROOT%\sources.cmn
if exist %P_ROOT%\projects\dirs del %P_ROOT%\projects\dirs

if not "%CLEANALL%" == "2" goto CleanThisProj


REM ----------------------------------------------------------------------
REM Clean Every Project (the "everything" option)
REM ----------------------------------------------------------------------

if not exist %P_ROOT%\projects\*.* goto CleanThisProj

if exist %P_ROOT%\kitroot\. rd /s/q %P_ROOT%\kitroot

echo Cleaning every Project...

cd %P_ROOT%\projects

%P_DCLROOT%\host\win32\bin\recurse /n /r /eSDK %COMSPEC% /c %P_DCLROOT%\host\win32\bin\cleanhlp.bat /PROJECT

cd %P_ROOT%

REM         .
REM         .
REM         .
REM Fall through to the "CleanThisProj" code so that we will handle the
REM case where the current project is NOT located in the PROJECTS tree.
REM         .
REM         .
REM         .

REM ----------------------------------------------------------------------
REM Clean This Project
REM ----------------------------------------------------------------------

:CleanThisProj

echo Cleaning the current Project...

cd %P_PROJDIR%

call %P_DCLROOT%\host\win32\bin\cleanhlp.bat

if not "%CLEANALL%" == "0" (
  @REM Invoke MAKE with the PROJCLEAN target.
  @REM
  for %%A in (???proj.mak) do call %B_MAKE% -f%%A PROJCLEAN
)

if "%CLEANALL%" == "0" goto Fini

if not "%P_CPUTYPE%" == "" if exist %P_PROJDIR%\%P_CPUTYPE%\. rd /s/q %P_PROJDIR%\%P_CPUTYPE%

goto Fini


REM ----------------------------------------------------------------------
REM Error Handlers
REM ----------------------------------------------------------------------

:Help
echo.
echo CLEAN cleans the product tree of all files generated during the
echo build process, as well as deleting the target binaries for the
echo current project if the "all" parameter is used.
echo.
echo Note that by default CLEAN will not clean project trees other
echo than the one currently in use.  However it can be forced to do so
echo if the "everything" parameter is used.
echo.
set B_ERROR=CLEAN_Help
goto Fini


:BadRoot
echo.
echo clean.bat:  P_ROOT is not set, or is not valid.
echo.
set B_ERROR=CLEAN_BadRoot
goto Fini

:BadProjDir
echo.
echo clean.bat: The P_PROJDIR setting "%P_PROJDIR%"
echo            does not match the current directory from which PRJCLEAN was run.
echo.
set B_ERROR=CLEAN_BadProj
goto Fini


REM ----------------------------------------------------------------------
REM Cleanup
REM ----------------------------------------------------------------------

:Fini
set CLEANALL=
 
