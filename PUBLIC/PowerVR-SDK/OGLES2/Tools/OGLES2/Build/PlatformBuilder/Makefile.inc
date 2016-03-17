APITOOLSROOT=$(SDKROOT)\Tools\$(API)
TOOLSROOT	=$(SDKROOT)\Tools
	
# API specific

$(OBJDIR)\PVRTPrint3DAPI.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTPrint3DAPI.cpp

$(OBJDIR)\PVRTgles2Ext.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTgles2Ext.cpp
	
$(OBJDIR)\PVRTTextureAPI.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTTextureAPI.cpp

$(OBJDIR)\PVRTBackground.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTBackground.cpp

$(OBJDIR)\PVRTPFXParserAPI.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTPFXParserAPI.cpp

$(OBJDIR)\PVRTShader.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APITOOLSROOT)\PVRTShader.cpp
	
# Tools

$(OBJDIR)\PVRTFixedPoint.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTFixedPoint.cpp

$(OBJDIR)\PVRTMatrixF.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTMatrixF.cpp
	
$(OBJDIR)\PVRTQuaternionF.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTQuaternionF.cpp

$(OBJDIR)\PVRTMisc.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTMisc.cpp

$(OBJDIR)\PVRTTrans.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTTrans.cpp

$(OBJDIR)\PVRTVertex.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTVertex.cpp

$(OBJDIR)\PVRTModelPOD.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTModelPOD.cpp

$(OBJDIR)\PVRTDecompress.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTDecompress.cpp

$(OBJDIR)\PVRTTriStrip.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTTriStrip.cpp
		
$(OBJDIR)\PVRTTexture.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTTexture.cpp	

$(OBJDIR)\PVRTPrint3D.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTPrint3D.cpp

$(OBJDIR)\PVRTResourceFile.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTResourceFile.cpp

$(OBJDIR)\PVRTString.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTString.cpp
	
$(OBJDIR)\PVRTPFXParser.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTPFXParser.cpp

$(OBJDIR)\PVRTShadowVol.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTShadowVol.cpp
	
$(OBJDIR)\PVRTVector.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTVector.cpp		

$(OBJDIR)\PVRTError.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(TOOLSROOT)\PVRTError.cpp
	
