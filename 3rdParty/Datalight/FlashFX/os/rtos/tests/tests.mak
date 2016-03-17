#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tests.mak $
#	Revision 1.1  2006/01/25 17:46:42Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

TARGFILE = fxoetest.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OE includes
B_NEEDSOSHEADERS = 1

OETESTHEADERS =	$(FXHEADERS)
OETESTINCLUDES =

OETESTOBJ = fmsltest.$(B_OBJEXT)\
	    vbftest.$(B_OBJEXT)	\
	    mtstress.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

OETESTTARGETS :	OETESTDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

OETESTDIR :
	@echo Processing os\$(P_OS)\Tests...

$(TARGNAME) : tests.mak $(OETESTOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

fmsltest.$(B_OBJEXT)  :	$(OETESTHEADERS) fmsltest.c
vbftest.$(B_OBJEXT)   :	$(OETESTHEADERS) vbftest.c
mtstress.$(B_OBJEXT)  :	$(OETESTHEADERS) mtstress.c

