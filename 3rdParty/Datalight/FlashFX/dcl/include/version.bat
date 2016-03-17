@echo off

REM -------------------------------------------------------------------------
REM 				Description
REM
REM The following sequence of lines is used in the automated build process
REM to manage the versions, build numbers, and the build date.  The line
REM following each of the keyword lines ($SIBLD...) contains a value
REM enclosed in the delimeters that immediately follow the keyword.  Do
REM not manually modify these lines unless you know what you are doing.
REM
REM If a parameter is specified on the command-line, this value will be	used
REM as the prefix for the variables set, otherwise "DL" will be used.
REM
REM NOTE: This batch file should be moved to the "Product" directory, and
REM       renamed to prodver.bat, once the AutoProj process is fixed to
REM       allow manipulating version information for files in two different
REM 	  directories.
REM
REM NOTE: No revision history in this file because it is automatically
REM       updated.
REM -------------------------------------------------------------------------

@echo %ECHO%

set T_PREFIX=DL
if not "%1" == "" set T_PREFIX=%1

REM NULLVAR must be set to nothing
set NULLVAR=

REM -----------------------------------------------------------------
REM START OF STUFF THAT IS AUTOMATICALLY CHANGED BY AUTOPROJ
REM -----------------------------------------------------------------

REM $SIBLDNUM=%                 The delimeters are "=" and "%
set %T_PREFIX%_BUILDNUM=1190BH%NULLVAR%

REM $SIBLDDATE=%                The delimeters are "=" and "%"
set %T_PREFIX%_BUILDDATE=04/24/2012%NULLVAR%

REM -----------------------------------------------------------------
REM START OF STUFF THAT IS MANUALLY CHANGED BY THE PRODUCT LEAD
REM -----------------------------------------------------------------

REM NOTE: The DL_VERSION and DL_VERSIONVAL variables
REM       must match the values found in dlver.h.

REM $SIBLDVER=%                 The delimeters are "=" and "%"
set T_VERSION=v3.1.2%NULLVAR%

REM Version number in hex.  The low byte is either zero, or the lowercase
REM hex representation of the alphabetic revision character -- 'a' == 0x61.
REM
set %T_PREFIX%_VERSIONVAL=0x03010200

REM Denote if we are in a Release, Alpha, or Beta state, uncomment the
REM one desired state only.
REM
set T_ALPHABETA=
REM set T_ALPHABETA= (Alpha)
REM set T_ALPHABETA= (Beta)

REM Copyright
REM
set %T_PREFIX%_COPYRIGHT=Copyright (c) 1993-2012 Datalight, Inc.

REM -----------------------------------------------------------------
REM END OF MANUALLY CHANGED STUFF
REM -----------------------------------------------------------------

set %T_PREFIX%_VERSION=%T_VERSION%%T_ALPHABETA%

REM Delete stuff not needed any more...
set T_ALPHABETA=
set T_PREFIX=
set T_VERSION=


