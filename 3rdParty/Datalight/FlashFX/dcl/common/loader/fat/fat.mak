#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXFAT library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: fat.mak $
#	Revision 1.6  2008/11/07 05:03:18Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.5  2008/11/03 20:21:18Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.4  2008/01/14 21:41:13Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include $(B_DCLLIBPREFIX) before the library name
#	Revision 1.3  2007/10/05 19:42:42Z  brandont
#	Updated dependency list.
#	Revision 1.2  2007/10/03 00:45:49Z  brandont
#	Updated the module include file list.  Added the loader module
#	include directory to the search path.
#	Revision 1.1  2007/09/27 01:16:32Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\common\loader\include

TARGNAME = $(B_RELDIR)\lib\dlfatrdr.$(B_LIBEXT)

FATHEADERS = \
	$(DCLHEADERS) 			\
	$(P_ROOT)\include\dlfat.h 	\
	$(P_ROOT)\include\dlfatapi.h 	\
	$(B_LOCALINC)\dlreader.h 	\
	$(B_LOCALINC)\dlfatread.h 	\
	$(P_ROOT)\include\dlpartid.h	\
	$(P_ROOT)\include\dlloader.h	\
	$(P_ROOT)\include\dlreaderio.h

FATSRC = \
	    dlfatread.c

FATOBJ = \
	    dlfatread.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

FATTARGETS : FATDIR	TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

FATDIR :
	@echo Processing Common\Loader\FAT...

$(TARGNAME) : dlfatrdr.$(B_LIBEXT)
	$(B_COPY) dlfatrdr.$(B_LIBEXT) $(TARGNAME)

dlfatrdr.$(B_LIBEXT) : fat.mak	$(FATOBJ)
	if exist dlfatrdr.$(B_LIBEXT) del dlfatrdr.$(B_LIBEXT)
	$(B_BUILDLIB) dlfatrdr.$(B_LIBEXT) *.$(B_OBJEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

dlfatread.$(B_OBJEXT)  : $(FATHEADERS) dlfatread.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: FATDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLFATRDR		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(FATSRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

