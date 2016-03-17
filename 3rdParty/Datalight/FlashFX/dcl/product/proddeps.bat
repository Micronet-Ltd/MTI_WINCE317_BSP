@echo off

REM -------------------------------------------------------------------------
REM                             Description
REM
REM This batch file is used to control what versions of the dependent
REM products are used.  The value "alpha" indicates the latest build,
REM while "beta" and "release" indicate older -- and (possibly) more
REM stable versions.  Explicit build numbers can be specified if needed.
REM
REM Note that the words "alpha", "beta", and "release" MUST be lower case.
REM
REM This file is typically updated only by the product/engineering manager.
REM -------------------------------------------------------------------------

REM -------------------------------------------------------------------------
REM                             Revision History
REM $Log: proddeps.bat $
REM Revision 1.7  2011/02/06 02:35:42Z  garyp
REM Don't allow building on AUGUSTUS since we could be doing AutoDoc.
REM Revision 1.6  2011/02/03 20:06:26Z  garyp
REM Set the preferred build machine to AUGUSTUS.
REM Revision 1.5  2010/05/02 23:24:08Z  garyp
REM Updated so maximillian is the least preferred machine to build this product.
REM Revision 1.4  2010/05/02 18:02:06Z  garyp
REM Syntax fix.
REM Revision 1.3  2010/05/02 16:57:36Z  garyp
REM Don't use a '*' to indicate that any build machine is OK since that will
REM cause wild card matching in our batch logic.
REM Revision 1.2  2010/05/02 00:37:48Z  garyp
REM Updated to specify PRODMACHINE and PRODDEPS.
REM Revision 1.1  2010/05/01 18:19:24Z  garyp
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Specify the list of build machines it is acceptable to build on, in
REM priority order.  "ANY" indicates that any build machine is OK.
REM
REM Reminder that we can't build DCL on AUGUSTUS because it does not have
REM TwinText (even though AutoDoc is usually off for DCL).
REM
set PRODMACHINE=gluteus maximus maximillian

REM List products this one is dependent on.  This must exactly match the
REM list of product branches and revisions below.
REM
set PRODDEPS=tpt

REM Specify the sub-product branches to use
REM
set BRANCHTPT=.

REM Specify the sub-product revisions to use
REM
set REVTPT=alpha

