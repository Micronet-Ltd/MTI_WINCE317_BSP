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
VGPCOMPILER	= ..\..\..\Utilities\VGPCompiler\Windows\VGPCompiler.exe

MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

TEXTURES = \
	skyline.pvr \
	Wall_diffuse_baked.pvr \
	Tang_space_BodyMap.pvr \
	Tang_space_LegsMap.pvr \
	Tang_space_BeltMap.pvr \
	FinalChameleonManLegs.pvr \
	FinalChameleonManHeadBody.pvr \
	lamp.pvr \
	ChameleonBelt.pvr


VERTEX_PROGRAMS = \
	DOT3_Skinning.h \
	VertexLit_Skinning.h
	

RESOURCES = \
	$(CONTENTDIR)/ChameleonScene.cpp \
	$(CONTENTDIR)/ChameleonScene_Fixed.cpp \
	$(CONTENTDIR)/skyline.cpp \
	$(CONTENTDIR)/Wall_diffuse_baked.cpp \
	$(CONTENTDIR)/Tang_space_BodyMap.cpp \
	$(CONTENTDIR)/Tang_space_LegsMap.cpp \
	$(CONTENTDIR)/Tang_space_BeltMap.cpp \
	$(CONTENTDIR)/FinalChameleonManLegs.cpp \
	$(CONTENTDIR)/FinalChameleonManHeadBody.cpp \
	$(CONTENTDIR)/lamp.cpp \
	$(CONTENTDIR)/ChameleonBelt.cpp

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

skyline.pvr: $(MEDIAPATH)/skyline.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/skyline.bmp -o$@

Wall_diffuse_baked.pvr: $(MEDIAPATH)/Wall_diffuse_baked.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Wall_diffuse_baked.bmp -o$@

Tang_space_BodyMap.pvr: $(MEDIAPATH)/Tang_space_BodyMap.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Tang_space_BodyMap.tga -o$@

Tang_space_LegsMap.pvr: $(MEDIAPATH)/Tang_space_LegsMap.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Tang_space_LegsMap.tga -o$@

Tang_space_BeltMap.pvr: $(MEDIAPATH)/Tang_space_BeltMap.tga
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/Tang_space_BeltMap.tga -o$@

FinalChameleonManLegs.pvr: $(MEDIAPATH)/FinalChameleonManLegs.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/FinalChameleonManLegs.bmp -o$@

FinalChameleonManHeadBody.pvr: $(MEDIAPATH)/FinalChameleonManHeadBody.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC4 -i$(MEDIAPATH)/FinalChameleonManHeadBody.bmp -o$@

lamp.pvr: $(MEDIAPATH)/lamp.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC2 -i$(MEDIAPATH)/lamp.bmp -o$@

ChameleonBelt.pvr: $(MEDIAPATH)/ChameleonBelt.bmp
	$(PVRTEXTOOL) -m -fOGLPVRTC2 -i$(MEDIAPATH)/ChameleonBelt.bmp -o$@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/ChameleonScene.cpp: ChameleonScene.pod
	$(FILEWRAP)  -o $@ ChameleonScene.pod

$(CONTENTDIR)/ChameleonScene_Fixed.cpp: ChameleonScene_Fixed.pod
	$(FILEWRAP)  -o $@ ChameleonScene_Fixed.pod

$(CONTENTDIR)/skyline.cpp: skyline.pvr
	$(FILEWRAP)  -o $@ skyline.pvr

$(CONTENTDIR)/Wall_diffuse_baked.cpp: Wall_diffuse_baked.pvr
	$(FILEWRAP)  -o $@ Wall_diffuse_baked.pvr

$(CONTENTDIR)/Tang_space_BodyMap.cpp: Tang_space_BodyMap.pvr
	$(FILEWRAP)  -o $@ Tang_space_BodyMap.pvr

$(CONTENTDIR)/Tang_space_LegsMap.cpp: Tang_space_LegsMap.pvr
	$(FILEWRAP)  -o $@ Tang_space_LegsMap.pvr

$(CONTENTDIR)/Tang_space_BeltMap.cpp: Tang_space_BeltMap.pvr
	$(FILEWRAP)  -o $@ Tang_space_BeltMap.pvr

$(CONTENTDIR)/FinalChameleonManLegs.cpp: FinalChameleonManLegs.pvr
	$(FILEWRAP)  -o $@ FinalChameleonManLegs.pvr

$(CONTENTDIR)/FinalChameleonManHeadBody.cpp: FinalChameleonManHeadBody.pvr
	$(FILEWRAP)  -o $@ FinalChameleonManHeadBody.pvr

$(CONTENTDIR)/lamp.cpp: lamp.pvr
	$(FILEWRAP)  -o $@ lamp.pvr

$(CONTENTDIR)/ChameleonBelt.cpp: ChameleonBelt.pvr
	$(FILEWRAP)  -o $@ ChameleonBelt.pvr

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
