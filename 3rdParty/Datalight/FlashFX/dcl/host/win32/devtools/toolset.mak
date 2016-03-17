#----------------------------------------------------------------------------
#				Description
#
#	This make file provides	the ToolSet specific make settings used
#	throughout the project.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: toolset.mak $
#	Revision 1.1  2005/08/21 08:15:06Z  Pauli
#	Initial revision
#	Revision 1.1  2005/08/21 09:15:06Z  Garyp
#	Initial revision
#	Revision 1.2  2005/08/21 09:15:05Z  garyp
#	Re-abstracted so that the general include paths and general defines are
#	set in dcl.mak.  Enhanced so that multiple include paths are allowed in
#	the same variable.
#	Revision 1.1  2005/01/26 02:56:40Z  pauli
#	Initial revision
#	Revision 1.2  2005/01/26 01:56:40Z  GaryP
#	Documentation update.
#	Revision 1.1  2005/01/16 20:41:36Z  GaryP
#	Initial revision
#----------------------------------------------------------------------------

ifndef TOOLSET_MAK_INCLUDED
TOOLSET_MAK_INCLUDED = 1

#--------------------------------------------------------------------
#	Reality	Checks
#--------------------------------------------------------------------


#--------------------------------------------------------------------
#	Basic Settings
#--------------------------------------------------------------------

# The ToolSet number must be unique for each ToolSet and must
# match	the values found in dltlset.h and dltlset.inc.

B_TOOLSETNUM=1520

B_ARESNAME=a_flags$(B_DEBUG).rsp
B_CRESNAME=c_flags$(B_DEBUG).rsp
B_ASMINCCMD=/I#				Assembler switch for include path
B_ASMDEFCMD=/D#				Assembler switch to define a symbol
B_CINCCMD=-I#				C compiler switch for include path
B_CDEFCMD=-D#				C compiler switch to define a symbol

TOOLSETINIT : ARESFILES	CRESFILES GENERIC_DEFINES GENERIC_INCLUDES

# Use the Borland common rules
include	$(P_DCLROOT)\host\win32\devtools\devtools.mak

endif	# TOOLSET_MAK_INCLUDED

