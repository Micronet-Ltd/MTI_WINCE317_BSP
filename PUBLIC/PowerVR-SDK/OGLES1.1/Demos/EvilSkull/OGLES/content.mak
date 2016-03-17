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
# $Revision: 1.3 $
#--------------------------------------------------------------------------

#############################################################################
## Variables
#############################################################################

PVRTEXTOOL 	= ..\..\..\Utilities\PVRTexTool\PVRTexToolCL\Win32\PVRTexTool.exe
FILEWRAP 	= ..\..\..\Utilities\Filewrap\Win32\Filewrap.exe


MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

TEXTURES = \
	Iris.pvr \
	Metal.pvr \
	Fire02.pvr \
	Fire03.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Iris.cpp \
	$(CONTENTDIR)/Metal.cpp \
	$(CONTENTDIR)/Fire02.cpp \
	$(CONTENTDIR)/Fire03.cpp \
	$(CONTENTDIR)/EvilSkull_float.cpp \
	$(CONTENTDIR)/EvilSkull_fixed.cpp

all: resources 
	
help:
	@echo Valid targets are:
	@echo resources, textures, binary_shaders, clean
	@echo PVRTEXTOOL, FILEWRAP and VGPCOMPILER can be used to override the 
	@echo default paths to these utilities.

clean:
	-rm $(TEXTURES)
	-rm $(RESOURCES)
	
	
resources: 		$(CONTENTDIR) $(RESOURCES)
textures: 		$(TEXTURES)


$(CONTENTDIR):
	-mkdir $@

############################################################################
# Create textures
############################################################################

Iris.pvr: $(MEDIAPATH)/Iris.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Iris.tga -o$@

Metal.pvr: $(MEDIAPATH)/Metal.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Metal.tga -o$@

Fire02.pvr: $(MEDIAPATH)/Fire02.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Fire02.tga -o$@

Fire03.pvr: $(MEDIAPATH)/Fire03.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Fire03.tga -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Iris.cpp: Iris.pvr
	$(FILEWRAP)  -o $@ Iris.pvr

$(CONTENTDIR)/Metal.cpp: Metal.pvr
	$(FILEWRAP)  -o $@ Metal.pvr

$(CONTENTDIR)/Fire02.cpp: Fire02.pvr
	$(FILEWRAP)  -o $@ Fire02.pvr

$(CONTENTDIR)/Fire03.cpp: Fire03.pvr
	$(FILEWRAP)  -o $@ Fire03.pvr

$(CONTENTDIR)/EvilSkull_float.cpp: EvilSkull_float.pod
	$(FILEWRAP)  -o $@ EvilSkull_float.pod

$(CONTENTDIR)/EvilSkull_fixed.cpp: EvilSkull_fixed.pod
	$(FILEWRAP)  -o $@ EvilSkull_fixed.pod


############################################################################
# End of file (content.mak)
############################################################################
