#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX tools.
#
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tools.mak $
#	Revision 1.3  2006/05/06 00:07:10Z  Pauli
#	Added FlashFX command shell module.
#	Revision 1.2  2006/03/06 01:32:38Z  Garyp
#	Eliminated FXRDIMG/FXWRIMG and replaced with FXIMAGE.
#	Revision 1.1  2006/01/25 17:25:56Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

TARGFILE = fxoetool.$(B_LIBEXT)
TARGNAME = $(B_RELDIR)\lib\$(TARGFILE)

# Tell the ToolSet init	code that we need the OS includes
B_NEEDSOSHEADERS = 1

OETOOLHEADERS =	$(FXHEADERS)
OETOOLINCLUDES =

OETOOLOBJ =	toolfw.$(B_OBJEXT)	\
		fxchk.$(B_OBJEXT)	\
		fxdump.$(B_OBJEXT) 	\
		fximage.$(B_OBJEXT)	\
		fxinfo.$(B_OBJEXT)	\
 		fxreclm.$(B_OBJEXT)	\
		fxremnt.$(B_OBJEXT)	\
		fxshell.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

OETOOLTARGETS :	OETOOLDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

OETOOLDIR :
	@echo Processing os\$(P_OS)\Tools...

$(TARGNAME) : tools.mak $(OETOOLOBJ)
	if exist $(TARGFILE) del $(TARGFILE)
	$(B_BUILDLIB) $(TARGFILE) *.$(B_OBJEXT)
	$(B_COPY) $(TARGFILE) $(TARGNAME)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

toolfw.$(B_OBJEXT)  : $(OETOOLHEADERS) toolfw.c
fxchk.$(B_OBJEXT)   : $(OETOOLHEADERS) fxchk.c
fxdump.$(B_OBJEXT)  : $(OETOOLHEADERS) fxdump.c
fximage.$(B_OBJEXT) : $(OETOOLHEADERS) fximage.c
fxinfo.$(B_OBJEXT)  : $(OETOOLHEADERS) fxinfo.c
fxreclm.$(B_OBJEXT) : $(OETOOLHEADERS) fxreclm.c
fxremnt.$(B_OBJEXT) : $(OETOOLHEADERS) fxremnt.c
fxshell.$(B_OBJEXT) : $(OETOOLHEADERS) fxshell.c


