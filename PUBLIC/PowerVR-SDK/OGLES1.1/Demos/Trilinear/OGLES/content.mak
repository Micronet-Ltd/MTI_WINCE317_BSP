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
	Back.pvr \
	Tape.pvr \
	Ball.pvr \
	Info.pvr

	

RESOURCES = \
	$(CONTENTDIR)/o_model_fixed.cpp \
	$(CONTENTDIR)/o_model_float.cpp \
	$(CONTENTDIR)/Back.cpp \
	$(CONTENTDIR)/Tape.cpp \
	$(CONTENTDIR)/Ball.cpp \
	$(CONTENTDIR)/Info.cpp

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

Back.pvr: $(MEDIAPATH)/Back.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Back.bmp -o$@

Tape.pvr: $(MEDIAPATH)/Tape.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Tape.bmp -o$@

Ball.pvr: $(MEDIAPATH)/Ball.bmp $(MEDIAPATH)/tBall.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Ball.bmp -a$(MEDIAPATH)/tBall.bmp -o$@

Info.pvr: $(MEDIAPATH)/Info.bmp $(MEDIAPATH)/tInfo.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Info.bmp -a$(MEDIAPATH)/tInfo.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/o_model_fixed.cpp: o_model_fixed.pod
	$(FILEWRAP)  -o $@ o_model_fixed.pod

$(CONTENTDIR)/o_model_float.cpp: o_model_float.pod
	$(FILEWRAP)  -o $@ o_model_float.pod

$(CONTENTDIR)/Back.cpp: Back.pvr
	$(FILEWRAP)  -o $@ Back.pvr

$(CONTENTDIR)/Tape.cpp: Tape.pvr
	$(FILEWRAP)  -o $@ Tape.pvr

$(CONTENTDIR)/Ball.cpp: Ball.pvr
	$(FILEWRAP)  -o $@ Ball.pvr

$(CONTENTDIR)/Info.cpp: Info.pvr
	$(FILEWRAP)  -o $@ Info.pvr


############################################################################
# End of file (content.mak)
############################################################################
