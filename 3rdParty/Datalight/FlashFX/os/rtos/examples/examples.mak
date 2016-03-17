#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX example
#	applications.
#
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: examples.mak $
#	Revision 1.2  2009/01/16 03:11:29Z  garyp
#	Merged from the v4.0 branch.
#	Revision 1.1.1.2  2009/01/16 03:11:29Z  garyp
#	Fixed a broken name.
#	Revision 1.1  2009/01/14 00:47:08Z  garyp
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

TARGFILE = fxexample.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(FXHEADERS)
OSTESTINCLUDES =

OSTESTOBJ = FlashFX_FML.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

OSTESTTARGETS :	OSTESTDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

OSTESTDIR :
	@echo Processing os\$(P_OS)\examples...

$(TARGNAME) : examples.mak $(OSTESTOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

FlashFX_FML.$(B_OBJEXT) : $(OSTESTHEADERS) FlashFX_FML.c

