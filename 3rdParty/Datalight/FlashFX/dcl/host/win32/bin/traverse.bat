@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This is the default batch file for traversing a directory structure
REM and building the specified directories.  It is used internally by the
REM product build process and is not run directly.
REM
REM On entry, the first parameter must be the name of the directory being
REM processed -- relative to P_ROOT.  Subsequent parameters must be the
REM subdirectories to process.  The subdirectories are processed in the
REM order specified.
REM
REM As each subdirectory is processed, if it contains a .MAK file with the
REM same name as the directory, it will be processed using B_BLDCMD.
REM
REM If instead the subdirectory contains a .BAT file with the same name as
REM the directory, it will be called, continuing the traversal.
REM
REM If neither a .MAK nor a .BAT file are found, but the directory contains
REM a "product" subdirectory with a prodinfo.bat file, the tree will be 
REM built as a subproduct.  The subproduct must build using the exact same
REM CPU and ToolSet settings as the parent product.  This product nesting is
REM only allowed to one subproduct level.
REM
REM "." is a special case subdirectory name and cannot be used with any
REM other subdirectory.  This flag indicates that the first parameter is
REM the "project" directory and that ???proj.mak should be processed.  Note
REM that in this case the "project" directory MUST be the full path spec-
REM ification rather than being relative to P_ROOT.  This must be handled
REM in this fashion because the project directory may not be in the product
REm tree and therefore cannot be traversed to using the normal process.
REM
REM All the various B_... variables must be set, including B_LOG.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: traverse.bat $
REM Revision 1.2  2007/07/06 20:08:35Z  pauli
REM Added logic to cleanup and restore env variables if the sub product
REM project fails to build.
REM Revision 1.1  2005/10/06 20:54:14Z  Pauli
REM Initial revision
REM Revision 1.2  2005/10/06 21:54:13Z  Garyp
REM Tweaked the error handling code.
REM Revision 1.1  2005/10/03 04:38:30Z  Garyp
REM Initial revision
REM Revision 1.6  2005/08/30 22:32:17Z  Garyp
REM Modified so that the sub-product variables are set inside the various prodinfo.bat
REM files rather than in traverse.bat.
REM Revision 1.5  2005/08/21 11:27:09Z  garyp
REM Updated to work with new project make files.
REM Revision 1.4  2005/08/18 02:34:58Z  garyp
REM Improved error handling.
REM Revision 1.3  2005/08/14 18:29:54Z  garyp
REM Hacked to check for ffxproj.mak instead of project.mak (now will fail
REM for a standalone Reliance project).
REM Revision 1.2  2005/08/06 01:32:36Z  pauli
REM Revision 1.1  2005/06/12 23:32:22Z  pauli
REM Initial revision
REM Revision 1.2  2005/06/12 23:32:22Z  PaulI
REM Updated to save and restore product info when building a subproduct.
REM Revision 1.1  2005/01/20 18:22:26Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

if "%1" == "" goto Syntax

if not "%2" == "." goto NormalDir

REM ---------------------------------------------------------------
REM Special case the project code since it may not be in the product
REM tree at all and we can't rely on traversing to it normally.
REM ---------------------------------------------------------------

echo Processing "%1"
cd %1

REM The project tree is special in that it must be a MAK process and not
REM another traversal, and the MAK file will always be named ???proj.mak,
REM where "???" is the P_PREFIX value for the product.

shift
if not "%2" == "" goto Syntax
if not exist %P_PREFIX%proj.mak goto MakeFileMissing

call %B_BLDCMD% %P_PREFIX%proj

REM Return to the original directory from which we invoked the
REM project code (even if it resides outside the product tree).

cd %P_ROOT%\projects

if "%B_ERROR%" == "" goto Fini
goto BldError


REM ---------------------------------------------------------------
REM Normal traverse process.  If there is a MAK file, process it.
REM ---------------------------------------------------------------

:NormalDir
echo Processing "%P_ROOT%\%1"
cd %P_ROOT%\%1

:NextDir
shift
if "%1" == "" goto Fini

if not exist %1\%1.mak goto TryBat
cd %1
call %B_BLDCMD% %1
cd ..

if "%B_ERROR%" == "" goto NextDir
goto BldError


REM ---------------------------------------------------------------
REM Didn't find the MAK file, so look for a BAT file.  If found
REM process it, continuing the traverse.
REM ---------------------------------------------------------------

:TryBat
if not exist %1\%1.bat goto TrySubProd

cd %1
call %1.bat
cd ..

if "%B_ERROR%" == "" goto NextDir
goto BldError


REM ---------------------------------------------------------------
REM Didn't find a MAK or BAT file.  See if there is a SubProduct
REM and process it if so.
REM ---------------------------------------------------------------
:TrySubProd
if not exist %1\product\prodinfo.bat goto BadLayer

REM Save values we will trash while building the sub-product
set B_OLDROOT=%P_ROOT%
set B_OLDTREES=%P_TREES%

set P_ROOT=%P_ROOT%\%1
call %1\product\prodinfo.bat /SUBPRODUCT

set P_TREES=%P_SUBTREES%

echo.
echo         ------[ Building %P_SUBPRODDESC% (%P_SUBBUILDNUM%) ]------
echo.
echo.                                                                  >>%B_LOG%
echo         ------[ Building %P_SUBPRODDESC% (%P_SUBBUILDNUM%) ]------>>%B_LOG%
echo.                                                                  >>%B_LOG%

REM We must process the project specific code for the sub-product first.
REM This code resides in the same overall Project Directory as the main
REM project however it will build separately, using its own makefile.

if not exist %P_PROJDIR%\%P_SUBPREFIX%proj.mak goto SkipProj
echo Processing "%P_PROJDIR%"
cd %P_PROJDIR%
call %B_BLDCMD% %P_SUBPREFIX%proj
cd %P_ROOT%\..
if not "%B_ERROR%" == "" goto FinishSubProd
:SkipProj

REM Now process the all the directories of the sub product.

cd %1
call %B_TRAVERSE% . %P_TREES%
cd ..

:FinishSubProd
echo.
echo         ------[ Finished %P_SUBPRODDESC% (%P_SUBBUILDNUM%) ]------
echo.
echo.                                                                  >>%B_LOG%
echo         ------[ Finished %P_SUBPRODDESC% (%P_SUBBUILDNUM%) ]------>>%B_LOG%
echo.                                                                  >>%B_LOG%

REM Restore original values that we trashed
set P_ROOT=%B_OLDROOT%
set P_TREES=%B_OLDTREES%

REM Clean up environment variables
set B_OLDROOT=
set B_OLDTREES=

set P_SUBPRODDESC=
set P_SUBPRODUCT=
set P_SUBPRODUCT8=
set P_SUBPREFIX=
set P_SUBPRODUCTNUM=
set P_SUBTREES=
set P_SUBBUILDNUM=
set P_SUBBUILDDATE=
set P_SUBVERSION=
 
if "%B_ERROR%" == "" goto NextDir
goto BldError

:BadLayer
echo TRAVERSE: Cannot traverse the layer "%1", skipping...
echo TRAVERSE: Cannot traverse the layer "%1", skipping...>>%B_LOG%
goto NextDir


REM ---------------------------------------------------------------
REM Error handlers
REM ---------------------------------------------------------------

:BldError
echo TRAVERSE: Error, MAKE failed for %1, "%B_ERROR%"
echo TRAVERSE: Error, MAKE failed for %1, "%B_ERROR%">>%B_LOG%
goto Fini

:Syntax
echo TRAVERSE: Syntax Error!
set B_ERROR=TRAVERSE_Syntax
goto Fini

:MakeFileMissing
echo TRAVERSE: Error -- %P_PREFIX%proj.mak is missing
set B_ERROR=TRAVERSE_MakeFileMissing
goto Fini


REM ---------------------------------------------------------------
REM Cleanup
REM ---------------------------------------------------------------

:Fini
