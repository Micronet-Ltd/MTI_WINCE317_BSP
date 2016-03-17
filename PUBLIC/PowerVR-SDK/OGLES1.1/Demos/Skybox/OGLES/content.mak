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


MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

TEXTURES = \
	balloon.pvr \
	skybox1.pvr \
	skybox2.pvr \
	skybox3.pvr \
	skybox4.pvr \
	skybox5.pvr \
	skybox6.pvr

	

RESOURCES = \
	$(CONTENTDIR)/Balloon_fixed.cpp \
	$(CONTENTDIR)/Balloon_float.cpp \
	$(CONTENTDIR)/balloon.cpp \
	$(CONTENTDIR)/skybox1.cpp \
	$(CONTENTDIR)/skybox2.cpp \
	$(CONTENTDIR)/skybox3.cpp \
	$(CONTENTDIR)/skybox4.cpp \
	$(CONTENTDIR)/skybox5.cpp \
	$(CONTENTDIR)/skybox6.cpp

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

balloon.pvr: $(MEDIAPATH)/balloon.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/balloon.bmp -o$@

skybox1.pvr: $(MEDIAPATH)/skybox1.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox1.bmp -o$@

skybox2.pvr: $(MEDIAPATH)/skybox2.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox2.bmp -o$@

skybox3.pvr: $(MEDIAPATH)/skybox3.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox3.bmp -o$@

skybox4.pvr: $(MEDIAPATH)/skybox4.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox4.bmp -o$@

skybox5.pvr: $(MEDIAPATH)/skybox5.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox5.bmp -o$@

skybox6.pvr: $(MEDIAPATH)/skybox6.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skybox6.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Balloon_fixed.cpp: Balloon_fixed.pod
	$(FILEWRAP)  -o $@ Balloon_fixed.pod

$(CONTENTDIR)/Balloon_float.cpp: Balloon_float.pod
	$(FILEWRAP)  -o $@ Balloon_float.pod

$(CONTENTDIR)/balloon.cpp: balloon.pvr
	$(FILEWRAP)  -o $@ balloon.pvr

$(CONTENTDIR)/skybox1.cpp: skybox1.pvr
	$(FILEWRAP)  -o $@ skybox1.pvr

$(CONTENTDIR)/skybox2.cpp: skybox2.pvr
	$(FILEWRAP)  -o $@ skybox2.pvr

$(CONTENTDIR)/skybox3.cpp: skybox3.pvr
	$(FILEWRAP)  -o $@ skybox3.pvr

$(CONTENTDIR)/skybox4.cpp: skybox4.pvr
	$(FILEWRAP)  -o $@ skybox4.pvr

$(CONTENTDIR)/skybox5.cpp: skybox5.pvr
	$(FILEWRAP)  -o $@ skybox5.pvr

$(CONTENTDIR)/skybox6.cpp: skybox6.pvr
	$(FILEWRAP)  -o $@ skybox6.pvr


############################################################################
# End of file (content.mak)
############################################################################
