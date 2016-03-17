
$(OBJDIR)\$(TARGETNAME).obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\$(TARGETNAME).cpp

$(OBJDIR)\Balloon_fixed.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Balloon_fixed.cpp

$(OBJDIR)\Balloon_float.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\Balloon_float.cpp

$(OBJDIR)\balloon.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\balloon.cpp

$(OBJDIR)\skybox1.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox1.cpp

$(OBJDIR)\skybox2.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox2.cpp

$(OBJDIR)\skybox3.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox3.cpp

$(OBJDIR)\skybox4.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox4.cpp

$(OBJDIR)\skybox5.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox5.cpp

$(OBJDIR)\skybox6.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(APPROOT)\Content\skybox6.cpp


!INCLUDE $(SDKROOT)\builds\PlatformBuilder\demo.inc

