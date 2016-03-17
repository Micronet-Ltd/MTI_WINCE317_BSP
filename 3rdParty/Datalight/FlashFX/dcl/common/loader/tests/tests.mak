#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXRELFS library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: tests.mak $
#	Revision 1.7  2008/11/07 06:18:31Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.6  2008/11/06 21:22:53Z  johnb
#	Added default $(B_INCLUDES) in PB section
#	Revision 1.5  2008/11/03 20:21:28Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.4  2008/01/14 21:45:14Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include $(B_DCLLIBPREFIX) before the library name
#	Revision 1.3  2007/10/05 22:06:21Z  brandont
#	Added dltloadertest.c.
#	Revision 1.2  2007/10/05 19:32:26Z  brandont
#	Renamed dltloaderutil to dltreaderutil
#	Revision 1.1  2007/10/05 03:16:18Z  brandont
#	Initial revision
#	Revision 1.1  2007/09/27 01:18:38Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\common\loader\include

TARGNAME = $(B_RELDIR)\lib\dltloader.$(B_LIBEXT)

HEADERS = \
	$(DCLHEADERS) 			\
	$(B_LOCALINC)\dlrelread.h 	\
	$(B_LOCALINC)\dlfatread.h 	\
	$(B_LOCALINC)\dlreader.h 	\
	$(P_ROOT)\include\dlreaderio.h	\
	$(P_ROOT)\include\dltloader.h

SRC = \
	dltreaderioapi.c		\
	dltreaderapi.c			\
	dltreaderutil.c			\
	dltloaderapi.c			\
	dltloadertest.c

OBJ = \
	dltreaderioapi.$(B_OBJEXT)	\
	dltreaderapi.$(B_OBJEXT)	\
	dltreaderutil.$(B_OBJEXT)	\
	dltloaderapi.$(B_OBJEXT)	\
	dltloadertest.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

TARGETS : DIR	TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

DIR :
	@echo Processing Common\Loader\Tests...

$(TARGNAME) : dltloader.$(B_LIBEXT)
	$(B_COPY) dltloader.$(B_LIBEXT) $(TARGNAME)

dltloader.$(B_LIBEXT) : tests.mak	$(OBJ)
	if exist dltloader.$(B_LIBEXT) del dltloader.$(B_LIBEXT)
	$(B_BUILDLIB) dltloader.$(B_LIBEXT) *.$(B_OBJEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

dltreaderioapi.$(B_OBJEXT) : $(HEADERS) dltreaderioapi.c
dltreaderapi.$(B_OBJEXT)   : $(HEADERS) dltreaderapi.c dltreaderutil.h
dltreaderutil.$(B_OBJEXT)  : $(HEADERS) dltreaderutil.c dltreaderutil.h
dltloaderapi.$(B_OBJEXT)   : $(HEADERS) dltloaderapi.c
dltloadertest.$(B_OBJEXT)  : $(HEADERS) dltloadertest.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: DIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLTLOADER		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(SRC)					>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

