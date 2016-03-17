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

FILEWRAP 	= ..\..\..\Utilities\Filewrap\Win32\Filewrap.exe

MEDIAPATH = ../Media
CONTENTDIR = Content

#############################################################################
## Instructions
#############################################################################

RESOURCES = \
	$(CONTENTDIR)/Example.cpp

all: resources
	
help:
	@echo Valid targets are:
	@echo resources clean
	@echo FILEWRAP can be used to override the 
	@echo default paths to these utilities.

clean:
	-rm $(RESOURCES)
	
resources: 		$(CONTENTDIR) $(RESOURCES)

$(CONTENTDIR):
	-mkdir $@

############################################################################
# Create content files
############################################################################

$(CONTENTDIR)/Example.cpp: Example.pvg
	$(FILEWRAP)  -o $@ Example.pvg

############################################################################
# End of file (content.mak)
############################################################################
