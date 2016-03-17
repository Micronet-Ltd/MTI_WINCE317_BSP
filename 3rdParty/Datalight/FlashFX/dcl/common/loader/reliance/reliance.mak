#----------------------------------------------------------------------------
#				Description
#
#	This make file controls	the generation of the FXRELFS library.  It is
#	designed for use with GNU Make or compatibles.
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#				Revision History
#	$Log: reliance.mak $
#	Revision 1.6  2008/11/07 05:03:26Z  johnb
#	Cleaned up whitespace
#	Cleaned up use of B_INCLUDES
#	Revision 1.5  2008/11/03 20:21:23Z  johnb
#	Added RELEASETYPE=PLATFORM 
#	Revision 1.4  2008/01/14 21:44:45Z  johnb
#	Modified Platform Builder section, specifically TARGETNAME to include $(B_DCLLIBPREFIX) before the library name
#	Revision 1.3  2007/10/05 19:46:28Z  brandont
#	Updated dependency list.
#	Revision 1.2  2007/10/03 00:45:16Z  brandont
#	Updated the module include file list.  Added the loader module
#	include directory to the search path.
#	Revision 1.1  2007/09/27 01:18:38Z  jeremys
#	Initial revision
#----------------------------------------------------------------------------

#-----------------------------------------------------------
#	Settings
#-----------------------------------------------------------

# Define an extra general include path
B_LOCALINC = $(P_ROOT)\common\loader\include

TARGNAME = $(B_RELDIR)\lib\dlrelrdr.$(B_LIBEXT)

RELFSHEADERS = \
	$(DCLHEADERS) \
	$(P_ROOT)\include\dlloader.h 	\
	$(P_ROOT)\include\dlreaderio.h 	\
	$(P_ROOT)\include\dlrelfs.h 	\
	$(P_ROOT)\include\dlrelapi.h 	\
	$(B_LOCALINC)\dlreader.h	\
	$(B_LOCALINC)\dlrelread.h	\
	dlrelbase.h			\
	dlrelcore.h			\
	dlrelmacro.h			\
	dlrelopts.h			\
	dlrelrdhlp.h			\
	dlreltypes.h

RELFSSRC = \
		dlrelread.c		\
		dlrelrdhlp.c

RELFSOBJ = \
		dlrelread.$(B_OBJEXT)	\
		dlrelrdhlp.$(B_OBJEXT)


#-----------------------------------------------------------
#	Targets
#-----------------------------------------------------------

RELFSTARGETS : RELFSDIR	TOOLSETINIT $(TARGNAME)


#-----------------------------------------------------------
#	Default	Project	Rules
#-----------------------------------------------------------

include	$(P_ROOT)\product\dcl.mak


#-----------------------------------------------------------
#	Build Commands
#-----------------------------------------------------------

RELFSDIR :
	@echo Processing Common\Loader\Reliance...

$(TARGNAME) : dlrelrdr.$(B_LIBEXT)
	$(B_COPY) dlrelrdr.$(B_LIBEXT) $(TARGNAME)

dlrelrdr.$(B_LIBEXT) : reliance.mak	$(RELFSOBJ)
	if exist dlrelrdr.$(B_LIBEXT) del dlrelrdr.$(B_LIBEXT)
	$(B_BUILDLIB) dlrelrdr.$(B_LIBEXT) *.$(B_OBJEXT)


#-----------------------------------------------------------
#	Dependencies
#-----------------------------------------------------------

dlrelread.$(B_OBJEXT)  : $(RELFSHEADERS) dlrelread.c
dlrelrdhlp.$(B_OBJEXT) : $(RELFSHEADERS) dlrelrdhlp.c


#-----------------------------------------------------------
#	Special	Targets	for Windows CE Platform	Builder	only
#-----------------------------------------------------------

mswcepb	: RELFSDIR TOOLSETINIT
	@echo #							 >sources
	@echo #This is an auto-generated file -- DO NOT	EDIT	>>sources
	@echo #							>>sources
	@echo RELEASETYPE=PLATFORM				>>sources
	@echo TARGETNAME=$(B_DCLLIBPREFIX)DLRELRDR		>>sources
	@echo TARGETTYPE=LIBRARY				>>sources
	@echo TARGETLIBS=					>>sources
	@echo EXEENTRY=						>>sources
	@echo SOURCES=$(RELFSSRC)				>>sources
	@echo B_INCLUDES=$(B_LOCALINC);				>>sources

