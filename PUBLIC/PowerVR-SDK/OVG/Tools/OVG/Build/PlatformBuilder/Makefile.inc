APITOOLSROOT=$(SDKROOT)\Tools\$(API)
TOOLSROOT	=$(SDKROOT)\Tools
	
# API specific

$(OBJDIR)\PVRTLoadPVG.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTLoadPVG.cpp

$(OBJDIR)\PVRTPrintVG.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTPrintVG.cpp
	
$(OBJDIR)\PVRTSVGObject.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTSVGObject.cpp

$(OBJDIR)\PVRTSVGParser.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTSVGParser.cpp

$(OBJDIR)\PVRTImage.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTImage.cpp
	
# Tools

$(OBJDIR)\PVRTString.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTString.cpp

$(OBJDIR)\PVRTResourceFile.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTResourceFile.cpp
	
$(OBJDIR)\PVRTVector.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTVector.cpp

$(OBJDIR)\PVRTMatrixF.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTMatrixF.cpp

$(OBJDIR)\PVRTError.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTError.cpp
	
$(OBJDIR)\PVRTTexture.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTTexture.cpp
	