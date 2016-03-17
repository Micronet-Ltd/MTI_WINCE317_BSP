#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXOECMN library.	It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: common.mak $
#	Revision 1.2  2007/12/15 01:59:45Z  Garyp
#	Added fxshellcmd.c.
#	Revision 1.1  2006/01/25 17:06:18Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#--------------------------------------------------------------------
#	Settings
#--------------------------------------------------------------------

TARGFILE = fxoecmn.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSCOMMONHEADERS	= $(FXHEADERS)
OSCOMMONINCLUDES =


#--------------------------------------------------------------------
#	Build a list of all the C source files, except those in the
#	EXCLUDE list, and from that generate the object file list.
#--------------------------------------------------------------------

ALLSRC      	:= $(wildcard *.c)
EXCLUDE     	:=
OSCOMMONSRC    	:= $(filter-out $(EXCLUDE), $(ALLSRC))
OSCOMMONOBJ	:= $(OSCOMMONSRC:.c=.$(B_OBJEXT))


#--------------------------------------------------------------------
#	Targets
#--------------------------------------------------------------------

OSCOMMONTARGETS	: OSCOMMONDIR TOOLSETINIT $(TARGNAME)


#--------------------------------------------------------------------
#	Default	Project	Rules
#--------------------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#--------------------------------------------------------------------
#	Build Commands
#--------------------------------------------------------------------

OSCOMMONDIR :
	@echo Processing os\$(P_OS)\Common...

$(TARGNAME) : common.mak $(OSCOMMONOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#--------------------------------------------------------------------
#	Dependencies
#--------------------------------------------------------------------

fxrtosshellcmd.$(B_OBJEXT)   	: $(OSCOMMONHEADERS) fxrtosshellcmd.c
option.$(B_OBJEXT)    		: $(OSCOMMONHEADERS) option.c
parsedrv.$(B_OBJEXT)   		: $(OSCOMMONHEADERS) parsedrv.c



