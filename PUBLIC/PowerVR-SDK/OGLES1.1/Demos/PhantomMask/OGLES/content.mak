#--------------------------------------------------------------------------
# Name         : content.mak
# Title        : Makefile to build content files
# Author       : Auto-generated
#
# Copyright    : 2007 by Imagination Technologies.  All rights reserved.
#              : No part of this software, either material or conceptual 
#              : may be copied or distributed, transmitted, transcribed,
#              : stored in a retrieval system or translated into any 
#              : human or computer language in any form by any means,
#              : electronic, mechanical, manual or other-wise, or 
#              : disclosed to third parties without the express written
#              : permission of VideoLogic Limited, Unit 8, HomePark
#              : Industrial Estate, King's Langley, Hertfordshire,
#              : WD4 8LZ, U.K.
#
# Description  : Makefile to build content files for demos in the PowerVR SDK
#
# Platform     :
#
# $Revision: 1.2 $
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################

PVRTEXTOOL 	= ..\..\..\Utilities\PVRTexTool\PVRTexToolCL\Win32\PVRTexTool.exe
FILEWRAP 	= ..\..\..\Utilities\Filewrap\Win32\Filewrap.exe
VGPCOMPILER	= ..\..\..\Utilities\VGPCompiler\Windows\VGPCompiler.exe

MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

TEXTURES = \
	MaskMain.pvr \
	RoomStill.pvr


VERTEX_PROGRAMS = \
	DIF.h \
	SHL.h
	

RESOURCES = \
	$(CONTENTDIR)/PhantomMask_float.cpp \
	$(CONTENTDIR)/PhantomMask_fixed.cpp \
	$(CONTENTDIR)/MaskMain.cpp \
	$(CONTENTDIR)/RoomStill.cpp

all: resources vertexprograms
	
help:
	@echo Valid targets are:
	@echo resources, textures, binary_shaders, clean
	@echo PVRTEXTOOL, FILEWRAP and VGPCOMPILER can be used to override the 
	@echo default paths to these utilities.

clean:
	-rm $(TEXTURES)
	-rm $(RESOURCES)
	-rm $(VERTEX_PROGRAMS)
	
resources: 		$(CONTENTDIR) $(RESOURCES)
textures: 		$(TEXTURES)
vertexprograms:	$(VERTEX_PROGRAMS)

$(CONTENTDIR):
	-mkdir $@

############################################################################
# Create textures
############################################################################

MaskMain.pvr: $(MEDIAPATH)/MaskMain.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/MaskMain.bmp -o$@

RoomStill.pvr: $(MEDIAPATH)/RoomStill.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/RoomStill.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/PhantomMask_float.cpp: PhantomMask_float.pod
	$(FILEWRAP)  -o $@ PhantomMask_float.pod

$(CONTENTDIR)/PhantomMask_fixed.cpp: PhantomMask_fixed.pod
	$(FILEWRAP)  -o $@ PhantomMask_fixed.pod

$(CONTENTDIR)/MaskMain.cpp: MaskMain.pvr
	$(FILEWRAP)  -o $@ MaskMain.pvr

$(CONTENTDIR)/RoomStill.cpp: RoomStill.pvr
	$(FILEWRAP)  -o $@ RoomStill.pvr

############################################################################
# Compile VGP code
############################################################################

$(VERTEX_PROGRAMS): $(@:.h=.vp)
	-$(VGPCOMPILER) -target=generic -i$(@:.h=.vp) -o$(@:.h=)_VGP.h
	-$(VGPCOMPILER) -target=generic -vgplite -i$(@:.h=.vp) -o$(@:.h=)_VGPLITE.h
	-$(VGPCOMPILER) -target=armvp -i$(@:.h=.vp) -o$(@:.h=)_VGPARMVP.h

############################################################################
# End of file (content.mak)
############################################################################
