@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM !!  This batch file overrides the default TRAVERSE.BAT found in the 
REM ..\bin directory.
REM
REM It is used for traversing a directory structure and building the
REM specified directories.  It is used internally by the product build
REM process and is not run directly.
REM
REM On entry, the first parameter must be the name of the directory being
REM processed -- relative to P_ROOT.  Subsequent parameters must be the
REM subdirectories to process.  The subdirectories are processed in the
REM order specified.
REM
REM As each subdirectory is processed, if it contains a .MAK file with the
REM same name as the directory it will be processed using B_BLDCMD.
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
REM Revision 1.3  2012/02/28 18:21:51Z  johnb
REM Platform Builder update.  Updated to create a subdirectory for the 
REM main product and for any sub projects.  The generated sources
REM amd makefile files are moved into the appropriate subdirectory.
REM Added ability to allow for more than 9 directories to exist wthin
REM any specific source directory.
REM Revision 1.2  2009/04/29 15:07:14Z  johnb
REM Added code to concatenate SOURES.CMN and each SOURCES file.  
REM The concatenation will not occur unless the Platform Builder project 
REM has defined the variable B_INC_SOURCESCMN to 1 and a SOURCES 
REM file exists in the current directory.
REM Revision 1.1  2005/12/01 02:17:48Z  Pauli
REM Initial revision
REM Revision 1.3  2005/12/01 02:17:48Z  Pauli
REM Merge with 2.0 product line, build 173.
REM Revision 1.2  2005/10/06 21:54:13Z  Garyp
REM Tweaked the error handling code.
REM Revision 1.1  2005/10/03 04:38:30Z  Garyp
REM Initial revision
REM Revision 1.3  2005/08/30 22:37:11Z  Garyp
REM Modified so that the sub-product variables are set inside the various prodinfo.bat
REM files rather than in traverse.bat.
REM Revision 1.2  2005/08/21 11:27:08Z  garyp
REM Re-abstracted so that the general include paths and general defines are
REM set in dcl.mak.  Enhanced so that multiple include paths are allowed in
REM the same variable.
REM Revision 1.1  2005/06/12 23:32:22Z  pauli
REM Initial revision
REM Revision 1.2  2005/06/12 23:32:22Z  PaulI
REM Updated to save and restore product info when building a subproduct.
REM Revision 1.1  2005/01/20 18:27:37Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM We use this batch file instead of the default because we must build
REM the DIRS and SOURCES files for Platform Builder, in addition to doing
REM the standard traversal.  Note how when B_BLDCMD is called, we always
REM use the "mswcepb" command line option to tell the make files to 
REM generate the SOURCES files (the DIRS files are generated directly by
REM this batch file).

if "%1" == "" goto Syntax

REM
REM See if we need to combine sources.cmn into sources
REM
if not "%B_INC_SOURCESCMN%" == "1" goto CheckNormal
if not exist %P_SOURCESCMN% goto SourcesCmnMissing

:CheckNormal
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

call %B_BLDCMD% %P_PREFIX%proj mswcepb

REM
REM See if we need to combine sources.cmn into sources
REM
if not "%B_INC_SOURCESCMN%" == "1" goto DoNotCombine
if not exist sources goto DoNotCombine

ren sources sources.tmp >nul
copy /A %P_SOURCESCMN% + sources.tmp sources >nul
del sources.tmp >nul


:DoNotCombine

if not exist %P_PREFIX%\nul md %P_PREFIX%
if exist sources move sources %P_PREFIX%\sources >nul
if exist makefile move makefile %P_PREFIX%\makefile >nul

REM Return to the original directory from which we invoked the
REM project code (even if it resides outside the product tree).

cd %P_ROOT%\projects

shift
if "%B_ERROR%" == "" goto Fini
goto BldError


REM ---------------------------------------------------------------
REM Normal traverse process. 
REM ---------------------------------------------------------------

:NormalDir
echo Processing "%P_ROOT%\%1"
cd %P_ROOT%\%1


echo # This is an auto-generated file that will be recreated when each   >dirs
echo # build is done.  Edit the associated batch file in this directory >>dirs
echo # to make changes to the DIRS settings.				>>dirs
echo #									>>dirs
echo DIRS = \ >>dirs
for %%x in (%*) do if not "%%x" == "%1" echo %%x \>>dirs

REM Shift unused parameters off
shift
shift

goto NextSkip

REM ---------------------------------------------------------------
REM If there is a MAK file, process it.
REM ---------------------------------------------------------------

:NextDir
shift
:NextSkip
if "%0" == "" goto Fini

if not exist %0\%0.mak goto TryBat

cd %0
call %B_BLDCMD% %0 mswcepb

REM
REM See if we need to combine sources.cmn into sources
REM
if not "%B_INC_SOURCESCMN%" == "1" goto DoNotCombineNormal
if not exist sources goto DoNotCombineNormal

ren sources sources.tmp >nul
copy /A %P_SOURCESCMN% + sources.tmp sources >nul
del sources.tmp >nul

:DoNotCombineNormal
cd ..

if "%B_ERROR%" == "" goto NextDir
goto BldError


REM ---------------------------------------------------------------
REM Didn't find the MAK file, so look for a BAT file.  If found
REM process it, continuing the traverse.
REM ---------------------------------------------------------------

:TryBat
if not exist %0\%0.bat goto TrySubProd

cd %0
call %0.bat

REM
REM See if we need to combine sources.cmn into sources
REM
if not "%B_INC_SOURCESCMN%" == "1" goto DoNotCombineBat
if not exist sources goto DoNotCombineBat

ren sources sources.tmp >nul
copy /A %P_SOURCESCMN% + sources.tmp sources >nul
del sources.tmp >nul

:DoNotCombineBat
cd ..

if "%B_ERROR%" == "" goto NextDir
goto BldError


REM ---------------------------------------------------------------
REM Didn't find a MAK or BAT file.  See if there is a SubProduct
REM and process it if so.
REM ---------------------------------------------------------------
:TrySubProd
if not exist %0\product\prodinfo.bat goto BadLayer

REM Save values we will trash while building the sub-product
set B_OLDROOT=%P_ROOT%
set B_OLDTREES=%P_TREES%

set P_ROOT=%P_ROOT%\%0
call %0\product\prodinfo.bat /SUBPRODUCT

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

cd %P_PROJDIR%
call %B_BLDCMD% %P_SUBPREFIX%proj mswcepb

if not exist %P_SUBPREFIX%\nul md %P_SUBPREFIX%
if exist sources move sources %P_SUBPREFIX%\sources >nul
if exist makefile move makefile %P_SUBPREFIX%\makefile >nul
echo DIRS=%P_SUBPREFIX% %P_PREFIX% >dirs

:SkipProj

cd %0
call %B_TRAVERSE% . %P_TREES%
cd ..

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
echo TRAVERSE: Cannot traverse the layer "%0", skipping...
echo TRAVERSE: Cannot traverse the layer "%0", skipping...>>%B_LOG%
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
%B_PAUSE%
goto Fini

:MakeFileMissing
echo TRAVERSE: Error -- %P_PREFIX%proj.mak is missing
set B_ERROR=TRAVERSE_MakeFileMissing
%B_PAUSE%
goto Fini

:SourcesCmnMissing
echo TRAVERSE: Error -- %P_SOURCESCMN% is missing
set B_ERROR=TRAVERSE_SourcesCmnMissing
%B_PAUSE%
goto Fini


REM ---------------------------------------------------------------
REM Cleanup
REM ---------------------------------------------------------------

:Fini
