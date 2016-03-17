
# PVRShell

$(OBJDIR)\PVRShell.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(SHELLROOT)\PVRShell.cpp
	
$(OBJDIR)\PVRShellAPI.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(SHELLAPIROOT)\PVRShellAPI.cpp
	
$(OBJDIR)\PVRShellOS.obj:
	@echo ------- Creating $@ -------
	$(CCOMPILER) $(CDEFINES) $(CONLY_FLAGS) $(C_COMMAND_LINE_OPTIONS) -Fo$@ $(SHELLOSROOT)\PVRShellOS.cpp