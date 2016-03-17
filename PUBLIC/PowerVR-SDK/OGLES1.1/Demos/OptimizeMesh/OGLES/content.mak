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
# $Revision: 1.1 $
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
	model_texture.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Sphere_fixed.cpp \
	$(CONTENTDIR)/Sphere_float.cpp \
	$(CONTENTDIR)/SphereOpt_fixed.cpp \
	$(CONTENTDIR)/SphereOpt_float.cpp \
	$(CONTENTDIR)/model_texture.cpp

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

model_texture.pvr: $(MEDIAPATH)/model_texture.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/model_texture.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Sphere_fixed.cpp: Sphere_fixed.pod
	$(FILEWRAP)  -o $@ Sphere_fixed.pod

$(CONTENTDIR)/Sphere_float.cpp: Sphere_float.pod
	$(FILEWRAP)  -o $@ Sphere_float.pod

$(CONTENTDIR)/SphereOpt_fixed.cpp: SphereOpt_fixed.pod
	$(FILEWRAP)  -o $@ SphereOpt_fixed.pod

$(CONTENTDIR)/SphereOpt_float.cpp: SphereOpt_float.pod
	$(FILEWRAP)  -o $@ SphereOpt_float.pod

$(CONTENTDIR)/model_texture.cpp: model_texture.pvr
	$(FILEWRAP)  -o $@ model_texture.pvr


############################################################################
# End of file (content.mak)
############################################################################
