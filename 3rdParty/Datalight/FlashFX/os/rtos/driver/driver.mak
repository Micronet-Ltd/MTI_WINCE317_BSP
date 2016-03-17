#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FlashFX device
#	driver for your	OS.
#
#	It is designed for use with GNU	Make or	compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: driver.mak $
#	Revision 1.6  2007/01/11 03:34:07Z  Garyp
#	Removed some obsolete comments.
#	Revision 1.5  2006/08/28 03:15:05Z  Garyp
#	Enclosed a copy command in quotes so the silly GNU MAKE does not
#	massage the string improperly when it things there is an SH in the system.
#	Revision 1.4  2006/08/25 22:37:32Z  Garyp
#	Fixed to use the proper copy command abstraction.
#	Revision 1.3  2006/03/02 19:43:41Z  Pauli
#	Add a file system include path to the list of OS include paths.  This
#	is used to put the Reliance include directory in to the path list.
#	Revision 1.2  2006/02/09 18:48:14Z  Pauli
#	Updated to be consistent with the current process.
#	Revision 1.1  2005/10/14 02:08:30Z  Pauli
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

TARGNAME = $(B_RELDIR)\flashfx.$(B_LIBEXT)

# Tell the ToolSet init	code that we need the OE includes
B_NEEDSOSHEADERS = 1

# Add the file system include path to the list of OS include paths
B_OSINC += $(B_FSINC)

DRIVERHEADERS =	$(FXHEADERS) ..\include\ffxdrv.h

DRIVEROBJ =	ffxdrv.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

DRIVERTARGETS :	DRIVERINIT TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\flashfx.mak


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

DRIVERINIT :
	@echo Processing os\$(P_OS)\Driver...
	$(B_COPY) "$(B_RELDIR)\lib\*.$(B_LIBEXT)"
	if exist flashfx.$(B_LIBEXT) del flashfx.$(B_LIBEXT)

# This allows us to conditionally adds the DCL libs to the PRODLIBS list.
# If we are building as a subproduct, the DCL libs will be added by the
# "master" product.
ifndef P_SUBPRODUCT
DRVDCLLIBS =	$(DCLLIBS)
else
DRVDCLLIBS =
endif

# This lists every library to be included in the all-inclusive
# FlashFX library.  This definition MUST appear physically
# in the MAKE file before it is referenced.
PRODLIBS =	$(FXDRVLIBS)		\
		$(DRVDCLLIBS)		\
		fxdrvr.$(B_LIBEXT)	\
		fxoecmn.$(B_LIBEXT)	\
		fxoetest.$(B_LIBEXT) 	\
		fxoetool.$(B_LIBEXT)

.PHONY: $(PRODLIBS)

$(PRODLIBS):
	$(B_ADDLIBS) flashfx.$(B_LIBEXT) $(@F)

$(TARGNAME) : fxdriver.$(B_LIBEXT) $(PRODLIBS)
	$(B_COPY) flashfx.$(B_LIBEXT) $(B_RELDIR)\.

# Make a temporary name for fxdriver.a to be used in the PRODLIBS
# step.  Can't be the same name or MAKE complains.
fxdriver.$(B_LIBEXT) : driver.mak $(DRIVEROBJ)
	if exist fxdriver.$(B_LIBEXT) del fxdriver.$(B_LIBEXT)
	$(B_BUILDLIB) fxdriver.$(B_LIBEXT) *.$(B_OBJEXT)
	$(B_COPY) fxdriver.$(B_LIBEXT) $(B_RELDIR)\lib\.
	$(B_COPY) fxdriver.$(B_LIBEXT) fxdrvr.$(B_LIBEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

ffxdrv.$(B_OBJEXT)	: $(DRIVERHEADERS) ffxdrv.c

