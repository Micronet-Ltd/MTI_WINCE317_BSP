#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.1  2005/08/21 04:11:20Z  Pauli
#	Initial revision
#	Revision 1.1  2005/08/21 05:11:20Z  Garyp
#	Initial revision
#	Revision 1.2  2005/08/21 05:11:20Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/03/12 00:27:16Z  pauli
#	Initial revision
#	Revision 1.3  2005/03/11 23:27:15Z  GaryP
#	Updated to delete prodlibs.lst at ToolSet init time.
#	Revision 1.2  2005/02/22 18:31:30Z  GaryP
#	Minor syntax cleanup.
#	Revision 1.1  2005/01/16 20:41:35Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

ifndef TOOLSET_MAK_INCLUDED
TOOLSET_MAK_INCLUDED = 1

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------


#--------------------------------------------------------------------
#	ToolSet	Initialization
#--------------------------------------------------------------------

TOOLSETINIT : MASTERMAKEFILES
	@if exist prodlibs.lst del prodlibs.lst

MASTERMAKEFILES	:
	@echo #							 >makefile
	@echo # This is an auto-generated file -- DO NOT EDIT	>>makefile
	@echo #							>>makefile
	@echo !INCLUDE $$(_MAKEENVROOT)\makefile.def		>>makefile


endif	# TOOLSET_MAK_INCLUDED

