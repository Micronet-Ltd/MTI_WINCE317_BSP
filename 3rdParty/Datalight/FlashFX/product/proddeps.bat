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
REM Revision 1.7.1.3  2011/02/03 23:23:37Z  garyp
REM Don't allow building on AUGUSTUS since it does not have TwinText.
REM Revision 1.7.1.2  2010/12/14 18:17:52Z  garyp
REM Link to the DCL v3.1 branch.
REM Revision 1.7  2010/06/12 04:55:42Z  garyp
REM Added SQA as a sub-project.
REM Revision 1.6  2010/05/03 02:51:14Z  garyp
REM Removed the "doc" sub-product dependency.
REM Revision 1.5  2010/05/02 22:38:18Z  garyp
REM Added PRODMACHINE and PRODDEPS.
REM Revision 1.4  2007/05/18 23:02:17Z  garyp
REM Updated to use docs as a sub-product.
REM Revision 1.3  2007/02/15 23:56:21Z  Garyp
REM Added the PERF project dependency settings.
REM Revision 1.2  2006/09/19 14:21:27Z  Garyp
REM Added the BRANCHDL setting.
REM Revision 1.1  2005/02/22 02:27:52Z  Pauli
REM Initial revision
REM -------------------------------------------------------------------------

@echo %ECHO%

REM Specify the list of build machines it is acceptable to build on, in
REM priority order.  "ANY" indicates that any build machine is OK.
REM Reminder that we can't build on AUGUSTUS because it does not have
REM TwinText.
REM
set PRODMACHINE=gluteus maximus maximillian

REM List products this one is dependent on.  This must exactly match the
REM list of product branches and revisions below.
REM
set PRODDEPS=dcl perf sqa

REM Specify the sub-product branches to use
set BRANCHDL=v3_1_Release
set BRANCHPRF=.
set BRANCHSQA=.

REM Specify the sub-product revisions to use
set REVDL=alpha
set REVPRF=alpha
set REVSQA=alpha


