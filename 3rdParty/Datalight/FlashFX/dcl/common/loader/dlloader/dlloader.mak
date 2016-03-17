#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXLOADER library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: dlloader.mak $
#	Revision 1.5  2008/11/07 05:03:08Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.4  2008/11/03 20:21:12Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.3  2008/01/14 21:41:00Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include $(B_DCLLIBPREFIX) before the library name
#	Revision 1.2  2007/10/02 23:36:26Z  brandont
#	Updated the module header file list.  Added the loader module 
#	include directory to the search path.
#	Revision 1.1  2007/09/27 01:16:42Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\common\loader\include

TARGNAME = $(B_RELDIR)\lib\dlloader.$(B_LIBEXT)

LOADERHEADERS = \
	$(DCLHEADERS)			\
	$(P_ROOT)\include\dlloader.h 	\
	$(B_LOCALINC)\dlfatread.h	\
	$(B_LOCALINC)\dlrelread.h	\

LOADERSRC =  \
	dlloader.c

LOADEROBJ =  \
	dlloader.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

LOADERTARGETS : LOADERDIR TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

LOADERDIR :
	@echo Processing Common\Loader\DLLoader...

$(TARGNAME) : dlloader.$(B_LIBEXT)
	$(B_COPY) dlloader.$(B_LIBEXT) $(TARGNAME)

dlloader.$(B_LIBEXT) : dlloader.mak	$(LOADEROBJ)
	if exist dlloader.$(B_LIBEXT) del dlloader.$(B_LIBEXT)
	$(B_BUILDLIB) dlloader.$(B_LIBEXT) *.$(B_OBJEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

dlloader.$(B_OBJEXT)   : $(LOADERHEADERS) dlloader.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: LOADERDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLLOADER		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(LOADERSRC)				>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

