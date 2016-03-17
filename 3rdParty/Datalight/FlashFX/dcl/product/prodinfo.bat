@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This file defines product specific information for the generalized build
REM process.  This batch file is called automatically during the product
REM build process.  P_ROOT is the only variable that must be defined prior to
REM calling this batch file.
REM
REM The only possible parameter is "/SUBPRODUCT" which means that this product
REM is being built as a sub-product of another, and all of the environment
REM variables listed below will have a "SUB" prefix, such as P_SUBPRODUCT.
REM When /SUBPRODUCT is used, the P_TREES variable will not include the DCL
REM or PROJECTS directories.
REM
REM On exit, the following variables must be set:
REM   P_PRODDESC    The product description
REM   P_PRODUCT     The user-friendly product name (mixed case)
REM   P_PRODUCT8    The max 8 character product name
REM   P_PREFIX      A two or three character product prefix (upper case)
REM   P_PRODUCTNUM  The product # which MUST match the value in dlprod.h
REM   P_PRODTITLE   The kit title
REM   P_TREES       The subdirectories to process when building
REM   P_VERSION     The product version string, as in "v2.1.1" or "Codename"
REM   P_VERSIONVAL  The product version number value in hex, v2.1.1 = 0x02010100
REM   P_BUILDDATE   The product build date string
REM   P_BUILDNUM    The product build number
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: prodinfo.bat $
REM Revision 1.6  2007/08/18 21:46:08Z  garyp
REM Documentation updated.
REM Revision 1.5  2006/12/26 20:01:43Z  Garyp
REM Restored the P_PRODUCT8 setting.
REM Revision 1.4  2006/12/23 21:05:40Z  Garyp
REM Updated so the product description can be changed during the kit building
REM process.
REM Revision 1.3  2006/12/20 01:41:54Z  Garyp
REM Added P_PRODTITLE and eliminated P_PRODUCT8.
REM Revision 1.2  2006/07/30 21:18:22Z  Garyp
REM Eliminated the now obsolete "CLEANEXCLUDE" functionality.
REM Revision 1.1  2005/12/07 05:03:08Z  Pauli
REM Initial revision
REM Revision 1.2  2005/12/07 05:03:07Z  Garyp
REM Modified to support per-product exclude directories when cleaning.
REM Revision 1.1  2005/10/03 13:42:16Z  Garyp
REM Initial revision
REM Revision 1.3  2005/08/30 22:32:18Z  Garyp
REM Modified so that the sub-product variables are set inside the various
REM prodinfo.bat files rather than in traverse.bat.
REM Revision 1.2  2005/08/05 23:21:46Z  pauli
REM Revision 1.1  2005/06/11 23:36:10Z  pauli
REM Initial revision
REM Revision 1.2  2005/06/11 23:36:10Z  PaulI
REM Set product number.
REM Revision 1.1  2005/01/16 22:43:03Z  GaryP
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Determine which extra trees to process, or not
set EXTRA_TREES=
if not "%1" == "/SUBPRODUCT"  set EXTRA_TREES=projects

REM If this is a sub-product, prefix the variables with "SUB"
set SUB=
if "%1" == "/SUBPRODUCT" set SUB=SUB

REM Note that text in the P_PRODUCT, P_PRODTITLE, and P_PRODDESC strings
REM below is automatically changed by the kit building process (see
REM bldcomp.bat), and modifying these strings without making appropriate
REM changes to the kitting process will break things.

set P_%SUB%PRODUCT=DLCommonLibs
set P_%SUB%PRODUCT8=DCL
set P_%SUB%PRODTITLE=SDK
set P_%SUB%PRODDESC=Datalight Common Libraries
set P_%SUB%PREFIX=DCL
set P_%SUB%PRODUCTNUM=3
set P_%SUB%TREES=%EXTRA_TREES% common cpu os

REM Note: The OS directory is listed last in P_TREES because it must
REM       be built last since that is where any linking might happen.

REM Get the version information
call %P_ROOT%\include\version.bat

set P_%SUB%VERSION=%DL_VERSION%
set P_%SUB%VERSIONVAL=%DL_VERSIONVAL%
set P_%SUB%BUILDDATE=%DL_BUILDDATE%
set P_%SUB%BUILDNUM=%DL_BUILDNUM%

set DL_VERSION=
set DL_VERSIONVAL=
set DL_BUILDDATE=
set DL_BUILDNUM=
set EXTRA_TREES=
set SUB=

:Fini
