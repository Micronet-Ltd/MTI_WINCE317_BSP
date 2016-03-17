#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the DCL tests.
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tests.mak $
#	Revision 1.1  2006/01/24 21:03:40Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

TARGFILE = dlostest.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OSTESTHEADERS =	$(DCLHEADERS)
OSTESTINCLUDES =

OSTESTOBJ = dcltest.$(B_OBJEXT)		\
	    fsiotest.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

OSTESTTARGETS :	OSTESTDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

OSTESTDIR : 
	@echo Processing os\$(P_OS)\Tests...

$(TARGNAME) : tests.mak $(OSTESTOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)
 

#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

dcltest.$(B_OBJEXT)   : $(OSTESTHEADERS) dcltest.c
fsiotest.$(B_OBJEXT)  : $(OSTESTHEADERS) fsiotest.c
 
