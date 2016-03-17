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
	FloorToon.pvr \
	MouseToon.pvr \
	Toon.pvr \
	WallToon.pvr


VERTEX_PROGRAMS = \
	CSH.h
	

RESOURCES = \
	$(CONTENTDIR)/Mouse_float.cpp \
	$(CONTENTDIR)/Mouse_fixed.cpp \
	$(CONTENTDIR)/FloorToon.cpp \
	$(CONTENTDIR)/MouseToon.cpp \
	$(CONTENTDIR)/Toon.cpp \
	$(CONTENTDIR)/WallToon.cpp

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

FloorToon.pvr: $(MEDIAPATH)/FloorToon.bmp
	$(PVRTEXTOOL) -nt -fOGL565 -i$(MEDIAPATH)/FloorToon.bmp -o$@

MouseToon.pvr: $(MEDIAPATH)/MouseToon.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/MouseToon.bmp -o$@

Toon.pvr: $(MEDIAPATH)/Toon.bmp
	$(PVRTEXTOOL) -nt -m -fOGL565 -i$(MEDIAPATH)/Toon.bmp -o$@

WallToon.pvr: $(MEDIAPATH)/WallToon.bmp
	$(PVRTEXTOOL) -nt -m -fOGL565 -i$(MEDIAPATH)/WallToon.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Mouse_float.cpp: Mouse_float.pod
	$(FILEWRAP)  -o $@ Mouse_float.pod

$(CONTENTDIR)/Mouse_fixed.cpp: Mouse_fixed.pod
	$(FILEWRAP)  -o $@ Mouse_fixed.pod

$(CONTENTDIR)/FloorToon.cpp: FloorToon.pvr
	$(FILEWRAP)  -o $@ FloorToon.pvr

$(CONTENTDIR)/MouseToon.cpp: MouseToon.pvr
	$(FILEWRAP)  -o $@ MouseToon.pvr

$(CONTENTDIR)/Toon.cpp: Toon.pvr
	$(FILEWRAP)  -o $@ Toon.pvr

$(CONTENTDIR)/WallToon.cpp: WallToon.pvr
	$(FILEWRAP)  -o $@ WallToon.pvr

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
